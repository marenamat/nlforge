#define DEBUG
#define DEBUGWIDTH 60
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fib_rules.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/limits.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <ncurses.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SPACE_8   "        "
#define SPACE_32  SPACE_8  SPACE_8  SPACE_8  SPACE_8
#define SPACE_128 SPACE_32 SPACE_32 SPACE_32 SPACE_32

#define SPACES(x) (&(SPACE_128)[128 - (x)])

#define BUFSIZESTEP	16384
#define BUFSIZEPADDING	4096

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
static u8 *data;
static u64 datasize = 0;
static u64 bufsize = 0;

static void data_realloc(void) {
  if (datasize + BUFSIZEPADDING < bufsize)
    return;

  u8 *tmp = malloc(bufsize * 2);
  memcpy(tmp, data, bufsize);
  free(data);
  bufsize *= 2;
  data = tmp;
}

#define ENUM_INFO_ALLOW_MULTIPLE 0x1

static struct {
  u16 max_index;
  u8 size;
  u8 flags;
  const char * const *strings;
  union {
    u8  *pu8;
    u16 *pu16;
    u32 *pu32;
    u64 *pu64;
    void *data;
  };
} enum_info;

struct rta_parsed {
  struct rtattr *a;
  u64 begin, payload, end;
  union {
    u8  *pu8;
    u16 *pu16;
    u32 *pu32;
    u64 *pu64;
    void *data;
  };
};

static struct nlmsg_parsed {
  struct nlmsghdr *h;
  u64 begin, payload, end;
  union {
    struct nlmsgerr *err; // NLMSG_ERROR
    struct ifinfomsg *link; // RTM_*LINK
    struct ifaddrmsg *addr; // RTM_*ADDR
    struct rtmsg *rt; // RTM_*ROUTE
    struct ndmsg *neigh; // RTM_*NEIGH
    struct fib_rule_hdr *rule; // RTM_*RULE
    struct ndtmst *ndt; // RTM_*NEIGHTB
    void *data;
  };
} *parsed;

#define PARSED_ALLOC_STEP 64

int parsed_cnt = 0, parsed_max = 0;

#include "nlstrings.h"

static WINDOW *hexwin;
static WINDOW *parsewin;
static WINDOW *statuswin;
static WINDOW *menuwin;
static WINDOW *debugwin;

#define MY_COLORS \
  C(DEFAULT, COLOR_WHITE, COLOR_BLACK, 1000, 1000, 1000, 0, 0, 0) \
  C(STATUS, COLOR_BLACK, COLOR_WHITE, 0, 0, 0, 700, 700, 700) \
  C(CURSOR, COLOR_BLACK, COLOR_YELLOW, 0, 0, 0, 1000, 1000, 700) \
  C(PHF, COLOR_BLUE, COLOR_WHITE, 0, 700, 0, 1000, 1000, 1000) \
  C(PH1, COLOR_BLACK, COLOR_GREEN, 0, 0, 0, 700, 1000, 700) \
  C(PH2, COLOR_WHITE, COLOR_BLUE, 0, 0, 0, 700, 700, 1000) \
  C(MENU, COLOR_BLACK, COLOR_WHITE, 0, 0, 0, 700, 700, 700) \
  C(ENUM_NORMAL, COLOR_BLACK, COLOR_WHITE, 0, 0, 0, 700, 700, 1000) \
  C(ENUM_CHOSEN, COLOR_BLACK, COLOR_YELLOW, 0, 0, 0, 400, 400, 1000) \
  C(DEBUG, COLOR_WHITE, COLOR_RED, 0, 0, 300, 1000, 700, 700) \


#define C(id, f, b, fr, fg, fb, br, bg, bb) id##_COLOR_PAIR,
enum {
  UNDEFINED_COLOR_PAIR = 64,
  MY_COLORS
  MAX_MY_COLOR_PAIR
};
#undef C

#define C(id, f, b, fr, fg, fb, br, bg, bb) id##_COLOR_FOREGROUND, id##_COLOR_BACKGROUND,
enum {
  UNDEFINED_MY_COLOR = 64,
  MY_COLORS
  MAX_MY_COLOR
};
#undef C


static void color_init()
{
  if (!has_colors())
    return;

  if (can_change_color()) {
#define C(id, f, b, fr, fg, fb, br, bg, bb) \
    init_color(id##_COLOR_FOREGROUND, fr, fg, fb); \
    init_color(id##_COLOR_BACKGROUND, br, bg, bb); \
    init_pair(id##_COLOR_PAIR, id##_COLOR_FOREGROUND, id##_COLOR_BACKGROUND);
    MY_COLORS
#undef C
  } else {
#define C(id, f, b, fr, fg, fb, br, bg, bb) init_pair(id##_COLOR_PAIR, f, b);
    MY_COLORS
#undef C
  }
}

#define status(...) do { werase(statuswin); wprintw(statuswin, __VA_ARGS__); wrefresh(statuswin); } while(0)
#ifdef DEBUG
#define debug(...) do { wprintw(debugwin, __VA_ARGS__); wprintw(debugwin, "\n"); wrefresh(debugwin); } while(0)
#else
#define debug(...) do { __VA_ARGS__; } while (0)
#endif

static void status_init()
{
  statuswin = newwin(1, COLS, LINES-1, 0);
  if (has_colors())
    wcolor_set(statuswin, STATUS_COLOR_PAIR, NULL);
  wrefresh(statuswin);
}

static int hexwin_offset = 0;
static int hexwin_cursor = 0;

#define winpos	(hexwin_cursor - hexwin_offset*BYTES_PER_LINE)
#define winy	(winpos / BYTES_PER_LINE + 1)
#define winx_	(winpos % BYTES_PER_LINE)
#define winx	(winx_*3+1 + winx_/BYTES_GROUP_BY)

#define BYTES_PER_LINE		16
#define BYTES_GROUP_BY		4
#define HEXWIN_LINES		(LINES-5)
#define HEXWIN_COLS		(BYTES_PER_LINE*3 + BYTES_PER_LINE/BYTES_GROUP_BY)
#define PGSKIP			16
#define BYTES_PER_SCREEN	(BYTES_PER_LINE*HEXWIN_LINES)

static void hexwin_init(void)
{
  hexwin = newwin(HEXWIN_LINES+2, HEXWIN_COLS, 2, 0);
  box(hexwin, 0, 0);
  wrefresh(hexwin);
}

static void hexwin_redraw(void)
{
  werase(hexwin);
  box(hexwin, 0, 0);
  int y = 0;
  for(int i=hexwin_offset; i<datasize/BYTES_PER_LINE+1; i++, y++) {
    if (y >= HEXWIN_LINES)
      break;
    for (int j=0; j<BYTES_PER_LINE; j++) {
      int rp = i*BYTES_PER_LINE + j;
      if (rp < datasize)
	mvwprintw(hexwin, y+1, j*3+1 + j/BYTES_GROUP_BY, "%02x", data[i*BYTES_PER_LINE + j]);
      if (rp == hexwin_cursor)
	mvwchgat(hexwin, y+1, j*3+1 + j/BYTES_GROUP_BY, 2, 0, CURSOR_COLOR_PAIR, NULL);
      if (rp == datasize)
	break;
    }
  }

  wrefresh(hexwin);
}

static void hexwin_scroll_to_cursor(void) {
  if (!datasize) {
    hexwin_cursor = 0;
    hexwin_offset = 0;
    return;
  }
  while (hexwin_offset*BYTES_PER_LINE > hexwin_cursor)
    hexwin_offset--;
  while (hexwin_cursor - hexwin_offset*BYTES_PER_LINE >= BYTES_PER_SCREEN)
    hexwin_offset++;
}

static void hexwin_cursor_to_scroll(void) {
  if (!datasize) {
    hexwin_cursor = 0;
    hexwin_offset = 0;
    return;
  }
  while (hexwin_offset*BYTES_PER_LINE > hexwin_cursor)
    hexwin_cursor += BYTES_PER_LINE;
  while (hexwin_cursor - hexwin_offset*BYTES_PER_LINE >= BYTES_PER_SCREEN)
    hexwin_cursor -= BYTES_PER_LINE;
}

static void parse_init(void) {
#ifdef DEBUG
  parsewin = newwin(LINES-3, COLS - (HEXWIN_COLS + 4 + DEBUGWIDTH), 3, (HEXWIN_COLS + 3));
#else
  parsewin = newwin(LINES-3, COLS - (HEXWIN_COLS + 4), 3, (HEXWIN_COLS + 3));
#endif
  wrefresh(parsewin);
}

static const char * const addrtostr(int af, int len, const char * const data) {
  static char buf[128];
  int pos = 0;

  switch (af) {
    case AF_INET:
    case AF_INET6:
      if (inet_ntop(af, data, buf, 128) == NULL) {
	status("inet_ntop: %m");
	return NULL;
      }
      return buf;
    case AF_MPLS:
      for (int i=0; i<len; i += 4) {
	if (i)
	  pos += sprintf(buf + pos, " / ");
	else
	  pos += sprintf(buf + pos, "MPLS label stack ");

	u32 lbl = ntohl(*((u32*)(data + i)));
	pos += sprintf(buf + pos, "%u", lbl >> 12);

	if (lbl & 0x100)
	  pos += sprintf(buf + pos, " (bos)");
	if (lbl & 0xff)
	  pos += sprintf(buf + pos, " (lsb %02x)");
      }
      return buf;
    default:
      sprintf(buf, "Display od %s addresses not implemented yet.", af_string[af]);
      return buf;
  }
}

#define isat(begin, type, field) \
     ((hexwin_cursor >= (begin + offsetof(type, field))) \
   && (hexwin_cursor < begin + offsetof(type, field) + sizeof(((type *) NULL)->field)))

#define papr(begin, type, field, format, ...) \
  do { \
    int overflow = (begin + offsetof(type, field) + sizeof(((type *) NULL)->field) - datasize); \
    if (isat(begin, type, field)) { \
      wcolor_set(parsewin, PHF_COLOR_PAIR, NULL); \
      if (overflow > 0) \
	memset(data + datasize, 0, overflow); \
      wprintw(parsewin, #field " = " format, __VA_ARGS__); \
      if (overflow > 0) \
	wprintw(parsewin, " (truncated)"); \
      wcolor_set(parsewin, header_color, NULL); \
    } else if (overflow > 0) { \
      wprintw(parsewin, "\nPremature end of data\n"); \
      debug("premature"); \
      goto parsend; \
    } else \
      wprintw(parsewin, #field " = " format, __VA_ARGS__); \
  } while (0)
#define papru(begin, parent, field) papr(begin, typeof(*(parent)), field, "%u", parent->field)
#define paprx(begin, parent, field) papr(begin, typeof(*(parent)), field, "0x%x", parent->field)
#define papren(begin, parent, field) \
  do { \
    papr(begin, typeof(*(parent)), field, "%u (%s)", parent->field, parent->field >= (sizeof(field##_string)/sizeof(*field##_string)) ? "out of scope" : field##_string[parent->field]); \
    if (isat(begin, typeof(*(parent)), field)) { \
      enum_info.size = sizeof(parent->field); \
      enum_info.max_index = (sizeof(field##_string)/sizeof(*field##_string)); \
      enum_info.strings = field##_string; \
      enum_info.data = &(parent->field); \
    } \
  } while (0)

#define paprf(begin, parent, field) \
  do { \
    papr(begin, typeof(*(parent)), field, "%s", strflags(parent->field, field##_string)); \
    if (isat(begin, typeof(*(parent)), field)) { \
      enum_info.size = sizeof(parent->field); \
      enum_info.max_index = (sizeof(field##_string)/sizeof(*field##_string)); \
      enum_info.strings = field##_string; \
      enum_info.data = &(parent->field); \
      enum_info.flags = ENUM_INFO_ALLOW_MULTIPLE; \
    } \
  } while (0)


#define paprc wprintw(parsewin, ", ")
#define papre wprintw(parsewin, "\n")

#define paprindent(n) for (int i=0; i<2*n; i++) waddch(parsewin, ' ');

static int parse_rtattr(int indent, struct rtattr *a, int alen, int af) {
  struct rta_parsed as;
  int rta_cnt = 0;

  short header_color = DEFAULT_COLOR_PAIR;

  for ( ; RTA_OK(a, alen); a = RTA_NEXT(a, alen)) {
    struct rta_parsed *ap = &as;
    bzero(ap, sizeof(*ap));
    ap->a = a;
    ap->data = RTA_DATA(a);
    ap->begin = ((u8*)a - data);
    ap->payload = ((u8*)ap->data - data);
    ap->end = ap->payload + RTA_PAYLOAD(a);

    paprindent(indent);
    papru(ap->begin, a, rta_len);
    paprc;
    papren(ap->begin, a, rta_type);
    papre;

    paprindent(indent+1);
    wprintw(parsewin, "Value: ");
    switch (a->rta_type) {
      case RTA_UNSPEC:
	break;
      case RTA_DST:
      case RTA_SRC:
      case RTA_GATEWAY:
      case RTA_PREFSRC:
      case RTA_NEWDST: // MLPS label stack
	{
	  const char * const addr = addrtostr(af, RTA_PAYLOAD(a), ap->data);
	  if (addr == NULL)
	    goto parsend;
	  wprintw(parsewin, "%s", addr);
	  break;
	}
      case RTA_IIF:
      case RTA_OIF:
      case RTA_PRIORITY:
      case RTA_TABLE:
      case RTA_MARK:
      case RTA_FLOW:
      case RTA_PREF:
      case RTA_ENCAP_TYPE:
      case RTA_EXPIRES:
	switch (RTA_PAYLOAD(a)) {
	  case 1:
	    wprintw(parsewin, "%u (8bit)", *ap->pu8);
	    break;
	  case 2:
	    wprintw(parsewin, "%u (16bit)", *ap->pu16);
	    break;
	  case 4:
	    wprintw(parsewin, "%u (32bit)", *ap->pu32);
	    break;
	  case 8:
	    wprintw(parsewin, "%llu (64bit)", *ap->pu64);
	    break;
	  default:
	    wprintw(parsewin, "?? (payload length %d)", RTA_PAYLOAD(a));
	}
	break;
      case RTA_METRICS: // nested
	wprintw(parsewin, "not supported yet");
	break;
      case RTA_MULTIPATH: // struct rtnexthop
	papre;
	struct rtnexthop *nh = (void *)(data + ap->payload);
	for ( ; RTNH_OK(nh, (ap->end - ((u8*)nh - data))); nh = RTNH_NEXT(nh)) {
	  paprindent(indent+2);
	  int nhpos = ((u8*)nh) - data;
	  papru(nhpos, nh, rtnh_len);
	  paprc;
	  paprf(nhpos, nh, rtnh_flags);
	  paprc;
	  papru(nhpos, nh, rtnh_hops);
	  paprc;
	  papru(nhpos, nh, rtnh_ifindex);
	  papre;

	  if (!parse_rtattr(indent + 3, RTNH_DATA(nh), nh->rtnh_len - RTNH_LENGTH(0), af))
	    goto parsend;
	}
	papre;
	break;
      case RTA_CACHEINFO: // struct rta_cacheinfo
      case RTA_MFC_STATS: // struct rta_mfc_stats
	wprintw(parsewin, "not supported yet");
	break;
      case RTA_VIA: // struct rtvia
	{
	  struct rtvia *rtvia = (void *)(data + ap->payload);
	  papren(ap->payload, rtvia, rtvia_family);
	  const char * const addr = addrtostr(rtvia->rtvia_family, RTA_PAYLOAD(a) - sizeof(rtvia->rtvia_family), rtvia->rtvia_addr);
	  paprc;
	  if (addr == NULL)
	    goto parsend;
	  wprintw(parsewin, "%s", addr);
	  break;
	}
	break;
      case RTA_ENCAP: // nested
	wprintw(parsewin, "not supported yet");
	break;
      case RTA_PROTOINFO: // no longer used
      case RTA_SESSION: // no longer used
      case RTA_MP_ALGO: // no longer used
	wprintw(parsewin, "no longer used");
	break;
    }
    papre;
    papre;
  }

  return 1;
parsend:
  papre;
  return 0;
}

static void parse(void) {
  free(parsed);
  parsed = malloc(sizeof(*parsed) * PARSED_ALLOC_STEP);
  parsed_cnt = 0;
  parsed_max = PARSED_ALLOC_STEP;

  bzero(&enum_info, sizeof(enum_info));

  werase(parsewin);

  for (int i = 0; i < datasize; parsed_cnt++) {
    parsed[parsed_cnt].h = (void *)(data + i);
    struct nlmsg_parsed *p = &(parsed[parsed_cnt]);
    p->begin = i;
    p->payload = p->begin + ((u8 *)NLMSG_DATA(p->h) - (data + i));
    p->end = p->begin + NLMSG_ALIGN(p->h->nlmsg_len);

    if (p->end <= p->begin + sizeof(struct nlmsghdr) + sizeof(struct rtgenmsg)) {
      wprintw(parsewin, "nlmsghdr len too short");
      goto parsend;
    }

    if (p->h->nlmsg_len != NLMSG_ALIGN(p->h->nlmsg_len)) {
      wprintw(parsewin, "unaligned size, given 0x%x, should be 0x%x", p->h->nlmsg_len, NLMSG_ALIGN(p->h->nlmsg_len));
      goto parsend;
    }

    int in_header = (hexwin_cursor < p->payload) && (hexwin_cursor >= p->begin);
    int in_data = (hexwin_cursor < p->end) && (hexwin_cursor >= p->payload);

    short header_color = DEFAULT_COLOR_PAIR;

    if (in_header || in_data)
      header_color = PH1_COLOR_PAIR;

    wcolor_set(parsewin, header_color, NULL);
    wprintw(parsewin, "nlmsghdr: ");
    papru(p->begin, p->h, nlmsg_len);
    paprc;
    papren(p->begin, p->h, nlmsg_type);
    paprc;

    if (p->h->nlmsg_type & 0x1)
#define nlmsg_flags_string nlmsg_flags_string_basic
      paprf(p->begin, p->h, nlmsg_flags);
#undef nlmsg_flags_string

    else if (p->h->nlmsg_type & 0x2)
#define nlmsg_flags_string nlmsg_flags_string_get
      paprf(p->begin, p->h, nlmsg_flags);
#undef nlmsg_flags_string

    else
#define nlmsg_flags_string nlmsg_flags_string_new
      paprf(p->begin, p->h, nlmsg_flags);
#undef nlmsg_flags_string

    papre;
    paprx(p->begin, p->h, nlmsg_seq);
    paprc;
    papru(p->begin, p->h, nlmsg_pid);
    papre;

    header_color = DEFAULT_COLOR_PAIR;
    wcolor_set(parsewin, DEFAULT_COLOR_PAIR, NULL);

    if (in_data) {
      p->data = NLMSG_DATA(p->h);
      switch (p->h->nlmsg_type) {
	case NLMSG_NOOP:
	  break;
	case NLMSG_ERROR:
	  {
	    paprindent(1);
	    papr(p->payload, struct nlmsgerr, error, "%u (%s)", p->err->error, strerror(p->err->error));
	    papre;
	  }
	case NLMSG_DONE:
	case NLMSG_OVERRUN:
	case RTM_NEWLINK:
	case RTM_DELLINK:
	case RTM_GETLINK:
	case RTM_SETLINK:
	case RTM_NEWADDR:
	case RTM_DELADDR:
	case RTM_GETADDR:
	  break;
	case RTM_NEWROUTE:
	case RTM_DELROUTE:
	case RTM_GETROUTE:
	  {
	    struct rtattr *a = RTM_RTA(p->rt);
	    int apos = ((u8*)a - data);
	    int alen = p->end - apos;
	    int in_header = ((hexwin_cursor < apos) && (hexwin_cursor >= p->payload));
	    int in_data = ((hexwin_cursor >= apos) && (hexwin_cursor < p->end));

	    paprindent(1);
	    papren(p->payload, p->rt, rtm_family);
	    paprc;
	    papru(p->payload, p->rt, rtm_dst_len);
	    paprc;
	    papru(p->payload, p->rt, rtm_src_len);
	    paprc;
	    papru(p->payload, p->rt, rtm_tos);
	    paprc;
	    papre;
	    paprindent(1);
	    papru(p->payload, p->rt, rtm_table);
	    paprc;
	    papren(p->payload, p->rt, rtm_protocol);
	    paprc;
	    papren(p->payload, p->rt, rtm_scope);
	    paprc;
	    papren(p->payload, p->rt, rtm_type);
	    papre;
	    paprindent(1);
	    paprf(p->payload, p->rt, rtm_flags);
	    papre;

	    if (!parse_rtattr(2, a, alen, p->rt->rtm_family))
	      goto parsend;
	    break;
	  }
	case RTM_NEWNEIGH:
	case RTM_DELNEIGH:
	case RTM_GETNEIGH:
	case RTM_NEWRULE:
	case RTM_DELRULE:
	case RTM_GETRULE:
	case RTM_NEWQDISC:
	case RTM_DELQDISC:
	case RTM_GETQDISC:
	case RTM_NEWTCLASS:
	case RTM_DELTCLASS:
	case RTM_GETTCLASS:
	case RTM_NEWTFILTER:
	case RTM_DELTFILTER:
	case RTM_GETTFILTER:
	case RTM_NEWACTION:
	case RTM_DELACTION:
	case RTM_GETACTION:
	case RTM_NEWPREFIX:
	case RTM_GETMULTICAST:
	case RTM_GETANYCAST:
	case RTM_NEWNEIGHTBL:
	case RTM_GETNEIGHTBL:
	case RTM_SETNEIGHTBL:
	case RTM_NEWNDUSEROPT:
	case RTM_NEWADDRLABEL:
	case RTM_DELADDRLABEL:
	case RTM_GETADDRLABEL:
	case RTM_GETDCB:
	case RTM_SETDCB:
	case RTM_NEWNETCONF:
	case RTM_GETNETCONF:
	case RTM_NEWMDB:
	case RTM_DELMDB:
	case RTM_GETMDB:
	case RTM_NEWNSID:
	case RTM_DELNSID:
	case RTM_GETNSID:
	  break;
	default:
	  debug("Unknown nlmsg type: %u", p->h->nlmsg_type);
      }
    }

    i = p->end;
  }

parsend:
  wrefresh(parsewin);
}

static void menu_init(void) {
  menuwin = newwin(1, COLS, 0, 0);
  if (has_colors())
    wcolor_set(menuwin, MENU_COLOR_PAIR, NULL);

  wprintw(menuwin, "F1 new file    F2 save file    F5 enum choose    F7 send to kernel    F8 intercept ip    F10/q quit");
  wrefresh(menuwin);
}


static void debug_init(void) {
  debugwin = newwin(LINES-2, DEBUGWIDTH, 1, COLS-DEBUGWIDTH);
  scrollok(debugwin, TRUE);
  idlok(debugwin, TRUE);
  if (has_colors())
    wcolor_set(debugwin, DEBUG_COLOR_PAIR, NULL);
  wrefresh(debugwin);
}

static int xgetch(void) {
  int ch = getch();
  debug("Got char 0x%x", ch);
  return ch;
}

static char *status_write(const char *msg) {
  status(msg);
  static char buf[256] = {};
  int pos = 0;
  curs_set(1);

  while (1) {
    int ch = xgetch();
    if (ch == KEY_ENTER || ch == 0xa || ch == 0xd)  {
      if (pos == 0) {
	break;
      }

      buf[pos] = 0;
      return buf;
    }

    if (ch == KEY_BACKSPACE) {
      int x, y;
      getyx(statuswin, y, x);
      mvwdelch(statuswin, y, x-1);
      wmove(statuswin, y, x-1);
      wrefresh(statuswin);
      pos--;
    }

    if (ch >= 0x7f)
      continue;

    waddch(statuswin, ch);
    wrefresh(statuswin);
    buf[pos++] = ch;
  }

  curs_set(0);
  return "";
}

static void save_data(void) {
  char resbuf[256];
  const char *res = NULL;
  const char *filename = status_write("File name: ");

  if (filename[0] == '\0') {
    res = "Empty name";
    goto fin;
  }

  int fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd < 0) {
    res = resbuf; sprintf(resbuf, "open: %m");
    goto fin;
  }

  int amount = write(fd, data, datasize);
  if (amount < 0) {
    res = resbuf; sprintf(resbuf, "write: %m");
    goto fin;
  }

  if (amount < datasize) {
    res = "Write incomplete.";
    goto fin;
  }

fin:
  if (res)
    status("File not saved: %s", res);
  else
    status("File saved as \"%s\"", filename);
}

#define ENUM_CHOOSER_SIZE   10

static void enum_choose(void) {
  if (enum_info.size == 0) {
    status("No enum at cursor");
    return;
  }

  u64 current_value = 0;
  switch (enum_info.size) {
    case 1:
      current_value = *(enum_info.pu8);
      break;
    case 2:
      current_value = *(enum_info.pu16);
      break;
    case 4:
      current_value = *(enum_info.pu32);
      break;
    case 8:
      current_value = *(enum_info.pu64);
      break;
    default:
      status("Strange size of enum");
      return;
  }

  int wl, wr, wt, wb;

  int enum_height = 0;
  int enum_width = 0;
  int enum_num = 0;
  int enum_pos = 0;
  int enum_scroll = 0;
  bool enum_mul = !!(enum_info.flags & ENUM_INFO_ALLOW_MULTIPLE);

  for (int i=0; i<enum_info.max_index; i++)
    if (enum_info.strings[i]) {
      int l = strlen(enum_info.strings[i]);
      if (l > enum_width)
	enum_width = l;

      if ((i == current_value) && !enum_mul)
	enum_pos = enum_num;
      enum_num++;
    }

#define ENUM_FLAGFIELD_ADDWIDTH	4

  enum_width += 2;

  if (enum_mul)
    enum_width += ENUM_FLAGFIELD_ADDWIDTH;

  if (enum_num >= ENUM_CHOOSER_SIZE) {
    enum_height = ENUM_CHOOSER_SIZE;
    enum_scroll = (ENUM_CHOOSER_SIZE/2) * ((enum_pos - (ENUM_CHOOSER_SIZE/4)) / (ENUM_CHOOSER_SIZE/2));
  } else
    enum_height = enum_num;

  if (winy + 3 + enum_height >= LINES) {
    wb = winy + 2;
    wt = wb - enum_height;
  } else {
    wt = winy + 3;
    wb = wt + enum_height;
  }

  u32 enum_val[enum_num];

  for (int i=0, j=0; i<enum_info.max_index; i++)
    if (enum_info.strings[i])
      enum_val[j++] = i;

  if (winx < enum_width/2) {
    wl = 0;
    wr = enum_width;
  } else if (winx > HEXWIN_COLS - enum_width/2) {
    wr = HEXWIN_COLS-1;
    wl = wr - enum_width;
  } else {
    wl = winx - enum_width/2;
    wr = wl + enum_width;
  }

  WINDOW *enumwin = newwin((wb - wt), (wr - wl), wt, wl);

  while (1) {
    werase(enumwin);
    for (int i=0; i<enum_height; i++) {
      if (has_colors())
	if (enum_pos == enum_scroll + i)
	  wcolor_set(enumwin, ENUM_CHOSEN_COLOR_PAIR, NULL);
	else
	  wcolor_set(enumwin, ENUM_NORMAL_COLOR_PAIR, NULL);

      if (enum_mul)
	if (current_value & (1ULL << enum_val[i + enum_scroll]))
	  wprintw(enumwin, "[X] ");
	else
	  wprintw(enumwin, "[ ] ");

      wprintw(enumwin, "%s", enum_info.strings[enum_val[i + enum_scroll]]);
      if (enum_mul)
	wprintw(enumwin, "%s\n", SPACES(enum_width - ENUM_FLAGFIELD_ADDWIDTH - strlen(enum_info.strings[enum_val[i + enum_scroll]]) - 1));
      else
	wprintw(enumwin, "%s\n", SPACES(enum_width - strlen(enum_info.strings[enum_val[i + enum_scroll]]) - 1));
    }

    wrefresh(enumwin);
    int ch = xgetch();

    if (ch == KEY_ENTER || ch == 0xa || ch == 0xd) {
      switch (enum_info.size) {
	case 1:
	  *(enum_info.pu8) = current_value;
	  break;
	case 2:
	  *(enum_info.pu16) = current_value;
	  break;
	case 4:
	  *(enum_info.pu32) = current_value;
	  break;
	case 8:
	  *(enum_info.pu64) = current_value;
	  break;
      }
      if (datasize < (enum_info.pu8 - data) + enum_info.size) {
	data_realloc();
	datasize = (enum_info.pu8 - data) + enum_info.size;
      }
      delwin(enumwin);
      return;
    }

    switch (ch) {
      case KEY_UP:
	if (enum_pos > 0)
	  enum_pos--;
	if ((enum_scroll > 0) && (enum_pos - enum_scroll < enum_height/4))
	{
	  enum_scroll -= enum_height/4;
	  if (enum_scroll < 0)
	    enum_scroll = 0;
	}
	if (!enum_mul)
	  current_value = enum_val[enum_pos];
	break;
      case KEY_DOWN:
	if (enum_pos < enum_num-1)
	  enum_pos++;
	if ((enum_scroll + enum_height < enum_num) && (enum_pos - enum_scroll + enum_height/4 >= enum_height))
	{
	  enum_scroll += enum_height/4;
	  if (enum_scroll + enum_height > enum_num)
	    enum_scroll = enum_num - enum_height;
	}
	if (!enum_mul)
	  current_value = enum_val[enum_pos];
	break;
      case ' ':
	if (enum_mul)
	  current_value ^= (1 << enum_val[enum_pos]);
	break;
    }
  }
}

static void send_to_kernel(void) {
  int sk = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sk < 0) {
    status("socket: %m");
    return;
  }

  for (int pos = 0; pos < datasize; ) {
    ssize_t sz = send(sk, data + pos, datasize - pos, 0);
    if (sz < 0) {
      status("send: %m");
      return;
    }
    pos += sz;
  }

  data_realloc();

  ssize_t sz = recv(sk, data + datasize, bufsize - datasize, 0);
  if (sz < 0) {
    status("recv: %m");
    return;
  }
  datasize += sz;
}

static void intercept_ip(void) {
  char *cmd = status_write("Command: ");

  if (cmd[0] == '\0') {
    status("Empty command");
    return;
  }

  int fd_netlink[2];
  int fd_output[2];
  int fd_err[2];

  if (pipe(fd_netlink) < 0) {
    status("pipe: %m");
    return;
  }

  if (pipe(fd_output) < 0) {
    status("pipe: %m");
    return;
  }

  pid_t pid = fork();
  if (pid < 0) {
    status("fork: %m");
    return;
  }

  if (pid == 0) { // Child
    close(fd_netlink[0]);
    close(fd_output[0]);

    char buf[256];

    if (dup2(fd_netlink[1], 3) != 3) {
      int n = sprintf(buf, "dup2: %m");
      write(fd_output[1], buf, n);
      exit(-1);
    }

    if (dup2(fd_output[1], 2) != 2) {
      int n = sprintf(buf, "dup2: %m");
      write(fd_output[1], buf, n);
      exit(-1);
    }

    if (dup2(fd_output[1], 1) != 1) {
      int n = sprintf(buf, "dup2: %m");
      write(fd_output[1], buf, n);
      exit(-1);
    }

    close(fd_netlink[1]);
    close(fd_output[1]);

#ifdef DEBUG
    write(1, cmd, strlen(cmd));
    write(1, "\n", 1);
#endif

    int N = 1;
    for (const char *p = cmd; *p; p++)
      if (*p == ' ')
	for (N++; *p == ' '; p++);

    char *argv[N+1];
    argv[0] = cmd;
    int nn = 1;

    for (char *p = cmd; *p; p++)
      if (*p == ' ') {
	while (*p == ' ')
	  *(p++) = 0;
	argv[nn++] = p;
      }

#ifdef DEBUG
    sprintf(buf, "arg num: %d\n", N);
    write(1, buf, strlen(buf));
    for (int i=0; i<N; i++) {
      write(1, "arg: ", 5);
      write(1, argv[i], strlen(argv[i]));
      write(1, "ea\n", 3);
    }
#endif

    argv[N] = NULL;
    char ldp[PATH_MAX+30];
    strcpy(ldp, "LD_PRELOAD=");
    if (getcwd(ldp + strlen(ldp), PATH_MAX) == NULL) {
      int n = sprintf(buf, "getcwd: %m");
      write(1, buf, n);
      exit(-1);
    }

    strcpy(ldp + strlen(ldp), "/interceptor.so");

    if (putenv(ldp) != 0) {
      int n = sprintf(buf, "putenv: %m");
      write(1, buf, n);
      exit(-1);
    }

    execvp(argv[0], argv);
    int n = sprintf(buf, "exec: %m");
    write(1, buf, n);
    exit(-1);
  }

  close(fd_netlink[1]);
  close(fd_output[1]);

  struct pollfd pfd[2] = {
    { .fd = fd_netlink[0], .events = POLLIN },
    { .fd = fd_output[0], .events = POLLIN }
  };

  datasize = 0;

  while (1) {
    int e = poll(pfd, 2, -1);
    if (e < 0) {
      if (errno == EINTR)
	continue;

      status("poll: %m");
      return;
    }

    if (e == 0)
      continue;

    if (pfd[0].revents & POLLIN) {
      int n = read(fd_netlink[0], data + datasize, bufsize - datasize);
      if (n < 0) {
	status("read: %m");
	return;
      }

      debug("read netlink: %d", n);
      datasize += n;
      data_realloc();
    }

    if (pfd[1].revents & POLLIN) {
      char buf[4096];
      int n = read(fd_output[0], buf, 4096);
      debug("read output: %d", n);
      debug("output: %*s", n, buf);
      if (n < 0) {
	status("read: %m");
	return;
      }
    }

    if ((pfd[0].revents & ~POLLIN) && (pfd[1].revents & ~POLLIN)) {
      debug("revents: %x %x", pfd[0].revents, pfd[1].revents);
      break;
    }
  }

  status("Waiting for command to finish");
  int wstatus;
  while (waitpid(pid, &wstatus, 0) < 0);

  status("Command done.");
  hexwin_redraw();
}

int main(int argc, char **argv)
{
  if (argc > 1) {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
      perror("open");
      return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
      perror("stat");
      return 1;
    }

    u8 *tmp;
    if ((tmp = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
      perror("mmap");
      return 1;
    }

    datasize = st.st_size;
    bufsize = (datasize + BUFSIZESTEP - 1) & ~(BUFSIZESTEP - 1);
    data = malloc(bufsize);
    memcpy(data, tmp, datasize);

  } else {
    datasize = 0;
    bufsize = BUFSIZESTEP;
    data = malloc(bufsize);
  }

  initscr();
  start_color();
  curs_set(0);
  cbreak();
  noecho();

  keypad(stdscr, TRUE);
  refresh();

  color_init();

#ifdef DEBUG
  debug_init();
#endif
  menu_init();
  status_init();
  hexwin_init();
  parse_init();

  while (1) {
    hexwin_redraw();
    parse();
    int ch = xgetch();
//    status("Hit key: %x", ch);
control:
    switch(ch) {
      case 'q':
      case KEY_F(10):
	endwin();
	return 0;
      case KEY_PPAGE:
	hexwin_offset -= PGSKIP;
	if (hexwin_offset < 0)
	  hexwin_offset = 0;
	hexwin_cursor_to_scroll();
	break;
      case KEY_NPAGE:
	hexwin_offset += PGSKIP;
	if (hexwin_offset >= datasize/BYTES_PER_LINE)
	  hexwin_offset = datasize/BYTES_PER_LINE - 1;
	hexwin_cursor_to_scroll();
	break;
      case KEY_LEFT:
	hexwin_cursor--;
	if (hexwin_cursor < 0)
	  hexwin_cursor = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_RIGHT:
	hexwin_cursor++;
	if (hexwin_cursor > datasize)
	  hexwin_cursor = datasize;
	hexwin_scroll_to_cursor();
	break;
      case KEY_UP:
	hexwin_cursor -= BYTES_PER_LINE;
	if (hexwin_cursor < 0)
	  hexwin_cursor = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_DOWN:
	hexwin_cursor += BYTES_PER_LINE;
	if (hexwin_cursor > datasize)
	  hexwin_cursor = datasize;
	hexwin_scroll_to_cursor();
	break;
      case KEY_DC:
	if (hexwin_cursor >= datasize)
	  break;

	if (hexwin_cursor < datasize-1)
	  memmove(data + hexwin_cursor, data + hexwin_cursor + 1, datasize - hexwin_cursor - 1);
	datasize--;
	break;
      case KEY_BACKSPACE:
	if (hexwin_cursor == 0)
	  break;

	if (hexwin_cursor < datasize)
	  memmove(data + hexwin_cursor - 1, data + hexwin_cursor, datasize - hexwin_cursor);
	datasize--;
	hexwin_cursor--;
	break;
      case KEY_SDC:
	if (!datasize || !parsed_cnt)
	  break;
	if (hexwin_cursor >= parsed[parsed_cnt-1].end) {
	  hexwin_cursor = parsed[parsed_cnt-1].end;
	  datasize = hexwin_cursor;
	} else {
	  int c;
	  if (hexwin_cursor < parsed[0].end)
	    c = 0;
	  else if (hexwin_cursor >= parsed[parsed_cnt-1].begin)
	    c = parsed_cnt - 1;
	  else {
	    int l = 0, r = parsed_cnt-1;
	    while (1) {
	      c = (l + r)/2;
	      if ((hexwin_cursor >= parsed[c].begin) && (hexwin_cursor < parsed[c].end))
		break;
	      else {
		if (l == r) {
		  status("Something strange in parser, couldn't find the right nlmsg to delete.");
		  break;
		}
		if (hexwin_cursor < parsed[c].begin)
		  r = c;
		if (hexwin_cursor >= parsed[c].end)
		  l = c;
	      }
	    }
	  }
	  debug("moving %d bytes to (data + %d) from (data + %d)", datasize - parsed[c].end, parsed[c].begin, parsed[c].end);
	  memmove(data + parsed[c].begin, data + parsed[c].end, datasize - parsed[c].end);
	  datasize -= (parsed[c].end - parsed[c].begin);
	  hexwin_cursor = parsed[c].begin;
	}
	hexwin_scroll_to_cursor();
	break;
      case KEY_F(1):
	datasize = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_F(2):
	save_data();
	break;
      case KEY_F(5):
	enum_choose();
	break;
      case KEY_F(7):
	send_to_kernel();
	break;
      case KEY_F(8):
	intercept_ip();
	break;
      default:
	debug("Unknown key 0%o (%s)", ch, keyname(ch));
    }

    if (((ch >= '0') && (ch <= '9'))
     || ((ch >= 'A') && (ch <= 'F'))
     || ((ch >= 'a') && (ch <= 'f'))) {

      mvwaddch(hexwin, winy, winx, ch | COLOR_PAIR(CURSOR_COLOR_PAIR) | A_BOLD);
      wrefresh(hexwin);

      int val = 0;
      if (ch <= '9')
	val = (ch - '0') << 4;
      else
	val = ((ch & 0x1f) + 9) << 4;

      debug("val is 0x%x", val);

      ch = xgetch();

      int go = 1;
      if ((ch >= '0') && (ch <= '9')) {
	val += (ch - '0');
	go = 0;
      } else if (((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f'))) {
	val += (ch & 0x1f) + 9;
	go = 0;
      }

      debug("val is 0x%x, go is %d, datasize is %d", val, go, datasize);

      data_realloc();

      if (hexwin_cursor == datasize)
	datasize++;

      data[hexwin_cursor++] = val;

      if (go)
	goto control;
    }
  }
}
