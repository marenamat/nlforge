#include <fcntl.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZESTEP 4096
#define DATA_BUFFER 65536
static char *data;
static int datasize = 0;
static int bufsize = 0;

static WINDOW *hexwin;
static WINDOW *statuswin;
static WINDOW *menuwin;
static WINDOW *debugwin;

#define MY_COLORS \
  C(DEFAULT, COLOR_WHITE, COLOR_BLACK, 1000, 1000, 1000, 0, 0, 0) \
  C(STATUS, COLOR_BLACK, COLOR_WHITE, 0, 0, 0, 700, 700, 700) \
  C(CURSOR, COLOR_BLACK, COLOR_YELLOW, 0, 0, 0, 1000, 1000, 700) \
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
#define debug(...) do { wprintw(debugwin, __VA_ARGS__); wprintw(debugwin, "\n"); wrefresh(debugwin); refresh(); } while(0)

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
  scrollok(hexwin, TRUE);
  idlok(hexwin, TRUE);
  box(hexwin, 0, 0);
  wrefresh(hexwin);
}

static void hexwin_redraw(void)
{
  werase(hexwin);
  box(hexwin, 0, 0);
  int y = 0;
  for(int i=hexwin_offset; i<datasize/BYTES_PER_LINE; i++, y++) {
    if (y >= HEXWIN_LINES)
      break;
    for (int j=0; j<BYTES_PER_LINE; j++) {
      mvwprintw(hexwin, y+1, j*3+1 + j/BYTES_GROUP_BY, "%02x", (unsigned char) data[i*BYTES_PER_LINE + j]);
      if (i*BYTES_PER_LINE + j == hexwin_cursor)
	mvwchgat(hexwin, y+1, j*3+1 + j/BYTES_GROUP_BY, 2, 0, CURSOR_COLOR_PAIR, NULL);
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

static void menu_init(void) {
  menuwin = newwin(1, COLS, 0, 0);
  if (has_colors())
    wcolor_set(menuwin, MENU_COLOR_PAIR, NULL);

  wprintw(menuwin, "F1 new file    F2 save file    F5 send to kernel    F10/q quit");
  wrefresh(menuwin);
}

static void debug_init(void) {
  debugwin = newwin(20, 60, 1, COLS-60);
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

static void save_data(void) {
  status("File name: ");
  char filename[256] = {};
  const char *res = NULL;
  int fnpos = 0;
  curs_set(1);

  while (1) {
    int ch = xgetch();
    if (ch == KEY_ENTER || ch == 0xa || ch == 0xd)  {
      if (fnpos == 0) {
	res = "Empty name";
	break;
      }
      filename[fnpos] = 0;

      // TODO: really save the file
      break;
    }
    if (ch >= 0x7f)
      continue;

    waddch(statuswin, ch);
    filename[fnpos++] = ch;
  }

  curs_set(0);
  if (res)
    status("File not saved: %s", res);
  else
    status("File saved as \"%s\"", filename);
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

    char *tmp;
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

  debug_init();
  menu_init();
  status_init();
  hexwin_init();
  hexwin_redraw();

  while (1) {
    int ch = xgetch();
//    status("Hit key: %x", ch);
    switch(ch) {
      case 'q':
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
	if (hexwin_cursor >= datasize)
	  hexwin_cursor = datasize-1;
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
	if (hexwin_cursor >= datasize)
	  hexwin_cursor = datasize - 1;
	hexwin_scroll_to_cursor();
	break;
      case KEY_F(1):
	datasize = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_F(2):
	save_data();
	break;
    }
    hexwin_redraw();
//    refresh();
  }
}
