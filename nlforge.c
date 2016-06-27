#define DEBUG
#define _GNU_SOURCE
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
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZESTEP 4096

typedef uint8_t u8;
typedef uint32_t u32;
static u8 *data;
static int datasize = 0;
static int bufsize = 0;

static void data_realloc(void) {
  u8 *tmp = malloc(bufsize * 2);
  memcpy(tmp, data, bufsize);
  free(data);
  bufsize *= 2;
  data = tmp;
}

static struct p {
  struct nlmsghdr *h;
  int begin, payload, end;
  union {
    struct { // RTM_*LINK
      struct ifinfomsg *link;
    };
    struct { // RTM_*ADDR
      struct ifaddrmsg *addr;
    };
    struct { // RTM_*ROUTE
      struct rtmsg *rt;
    };
    struct { // RTM_*NEIGH
      struct ndmsg *neigh;
    };
    struct { // RTM_*RULE
      struct fib_rule_hdr *rule;
    };
    struct { // RTM_*NEIGHTBL
      struct ndtmst *ndt;
    };
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
  C(MENU, COLOR_BLACK, COLOR_WHITE, 0, 0, 0, 700, 700, 700) \
  C(DEBUG, COLOR_WHITE, COLOR_RED, 0, 0, 300, 1000, 700, 700) \


#define C(id, f, b, fr, fg, fb, br, bg, bb) id##_COLOR_PAIR,
enum {
  UNDEFINED_COLOR_PAIR = 0,
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

#define status(...) do { werase(statuswin); wprintw(statuswin, __VA_ARGS__); wrefresh(statuswin); refresh(); } while(0)
#ifdef DEBUG
#define debug(...) do { wprintw(debugwin, __VA_ARGS__); wprintw(debugwin, "\n"); wrefresh(debugwin); refresh(); } while(0)
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

#define BYTES_PER_LINE		16
#define BYTES_GROUP_BY		4
#define HEXWIN_LINES		(LINES-5)
#define PGSKIP			16
#define BYTES_PER_SCREEN	(BYTES_PER_LINE*HEXWIN_LINES)

static void hexwin_init(void)
{
  hexwin = newwin(HEXWIN_LINES+2, BYTES_PER_LINE*3 + BYTES_PER_LINE/BYTES_GROUP_BY, 2, 0);
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
  parsewin = newwin(LINES-3, COLS-(BYTES_PER_LINE*3 + BYTES_PER_LINE/BYTES_GROUP_BY + 2), 3, (BYTES_PER_LINE*3 + BYTES_PER_LINE/BYTES_GROUP_BY + 1));
  wrefresh(parsewin);
}

#define isat(begin, type, field) \
     ((hexwin_cursor >= (begin + offsetof(type, field))) \
   && (hexwin_cursor < begin + offsetof(type, field) + sizeof(((type *) NULL)->field)))

#define papr(begin, type, field, format, ...) \
  do { \
    if (isat(begin, type, field)) { \
      wcolor_set(parsewin, PHF_COLOR_PAIR, NULL); \
      wprintw(parsewin, #field " = " format, __VA_ARGS__); \
      wcolor_set(parsewin, header_color, NULL); \
    } else { \
      wprintw(parsewin, #field " = " format, __VA_ARGS__); \
    } \
  } while (0)

#define paprc wprintw(parsewin, ", ")
#define papre wprintw(parsewin, "\n")

static void parse(void) {
  free(parsed);
  parsed = malloc(sizeof(*parsed) * PARSED_ALLOC_STEP);
  parsed_cnt = 0;
  parsed_max = PARSED_ALLOC_STEP;

  werase(parsewin);

  for (int i = 0; i < datasize; ) {
    parsed[parsed_cnt].h = (void *)(data + i);
    struct p *p = &(parsed[parsed_cnt]);
    p->begin = i;
    p->payload = p->begin + ((u8 *)NLMSG_DATA(p->h) - (data + i));
    p->end = p->begin + NLMSG_ALIGN(p->h->nlmsg_len);

    int in_header = (hexwin_cursor < p->payload) && (hexwin_cursor >= p->begin);
    int in_data = (hexwin_cursor < p->end) && (hexwin_cursor >= p->payload);

    short header_color = DEFAULT_COLOR_PAIR;

    if (in_header || in_data)
      header_color = PH1_COLOR_PAIR;

    wcolor_set(parsewin, header_color, NULL);
    wprintw(parsewin, "nlmsghdr: ");

    papr(p->begin, struct nlmsghdr, nlmsg_len, "%u", p->h->nlmsg_len);
    paprc;
    papr(p->begin, struct nlmsghdr, nlmsg_type, "%u (%s)",
      p->h->nlmsg_type, nlmsg_type_string[p->h->nlmsg_type]);
    paprc;
    papr(p->begin, struct nlmsghdr, nlmsg_flags, "%s",
      strflags(p->h->nlmsg_flags, (p->h->nlmsg_type & 0x1) ? nlmsg_flags_string_basic
	: ((p->h->nlmsg_type & 0x2) ? nlmsg_flags_string_get : nlmsg_flags_string_new)));
    papre;

    wcolor_set(parsewin, DEFAULT_COLOR_PAIR, NULL);

    switch (p->h->nlmsg_type) {
      case NLMSG_NOOP:
      case NLMSG_ERROR:
      case NLMSG_DONE:
      case NLMSG_OVERRUN:
	break;
      case RTM_NEWLINK:
      case RTM_DELLINK:
      case RTM_GETLINK:
      case RTM_SETLINK:
	p->link = NLMSG_DATA(p->h);
	break;
      case RTM_NEWADDR:
      case RTM_DELADDR:
      case RTM_GETADDR:
	p->addr = NLMSG_DATA(p->h);
	break;
      case RTM_NEWROUTE:
      case RTM_DELROUTE:
      case RTM_GETROUTE:
	p->rt = NLMSG_DATA(p->h);
	break;
      case RTM_NEWNEIGH:
      case RTM_DELNEIGH:
      case RTM_GETNEIGH:
	p->neigh = NLMSG_DATA(p->h);
	break;
      case RTM_NEWRULE:
      case RTM_DELRULE:
      case RTM_GETRULE:
	p->rule = NLMSG_DATA(p->h);
	break;
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
	break;
      case RTM_NEWNEIGHTBL:
      case RTM_GETNEIGHTBL:
      case RTM_SETNEIGHTBL:
	p->ndt = NLMSG_DATA(p->h);
	break;
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

    i = p->end;
  }
  wrefresh(parsewin);
}

static void menu_init(void) {
  menuwin = newwin(1, COLS, 0, 0);
  if (has_colors())
    wcolor_set(menuwin, MENU_COLOR_PAIR, NULL);

  wprintw(menuwin, "F1 new file    F2 save file    F5 send to kernel    F8 intercept ip    F10/q quit");
  wrefresh(menuwin);
}

static void debug_init(void) {
  debugwin = newwin(LINES-2, 60, 1, COLS-60);
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
      if (datasize == bufsize)
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
      case KEY_F(1):
	datasize = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_F(2):
	save_data();
	break;
      case KEY_F(8):
	intercept_ip();
	break;
    }

    if (((ch >= '0') && (ch <= '9'))
     || ((ch >= 'A') && (ch <= 'F'))
     || ((ch >= 'a') && (ch <= 'f'))) {
      int winpos = hexwin_cursor - hexwin_offset*BYTES_PER_LINE;
      int winy = winpos / BYTES_PER_LINE + 1;
      int winx = winpos % BYTES_PER_LINE; winx = winx*3+1 + winx/BYTES_GROUP_BY;

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

      if (hexwin_cursor == bufsize)
	data_realloc();

      if (hexwin_cursor == datasize)
	datasize++;

      data[hexwin_cursor++] = val;

      if (go)
	goto control;
    }
  }
}
