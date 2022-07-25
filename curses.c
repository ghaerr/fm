/*
 * curses library workaround
 *
 * Jul 2022 Greg Haerr
 */
#include "curses.h"
#include "unikey.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

int LINES = 25;
int COLS = 80;
void *stdscr;

static struct termios oldterm;
static struct termios t;
#define WRITE(FD, SLIT)        write(FD, SLIT, strlen(SLIT))
#define ENABLE_SAFE_PASTE      "\e[?2004h"
// 1003 for mouse move all events
#define ENABLE_MOUSE_TRACKING  "\e[?1000;1002;1015;1006h"
#define DISABLE_MOUSE_TRACKING "\e[?1000;1002;1015;1006l"
#define RESET_VIDEO            "\e[1;0;0m\e[?25h"
#define PROBE_DISPLAY_SIZE     "\e7\e[9979;9979H\e[6n\e8"

static void onkilled(int sig) {
    exit(1);
}

static void restoretty(void) {
  WRITE(1, DISABLE_MOUSE_TRACKING);
  WRITE(1, RESET_VIDEO);
  tcsetattr(1, TCSADRAIN, &oldterm);
}

static void rawtty(void) {
  t.c_cc[VMIN] = 1;         /* requires ESC ESC ESC for ESC */
  t.c_cc[VTIME] = 1;
  t.c_iflag &= ~(INPCK | ISTRIP | PARMRK | INLCR | IGNCR | ICRNL | IXON |
                 IGNBRK | BRKINT);
  t.c_lflag &= ~(IEXTEN | ICANON | ECHO | ECHONL /*| ISIG*/);
  t.c_cflag &= ~(CSIZE | PARENB);
  //t.c_oflag &= ~OPOST;
  t.c_cflag |= CS8;
  //t.c_iflag |= IUTF8;         /* correct kernel backspace behaviour */
  tcsetattr(1, TCSADRAIN, &t);
  //WRITE(1, ENABLE_SAFE_PASTE);
  WRITE(1, ENABLE_MOUSE_TRACKING);
  //WRITE(1, PROBE_DISPLAY_SIZE);
}

static int rawmode(void) {
  static int once;
  if (!once) {
    if (tcgetattr(1, &oldterm) != -1) {
      atexit(restoretty);
      signal(SIGTERM, onkilled);
      signal(SIGINT, onkilled);
    } else {
      return -1;
    }
    once = 1;
  }
  memcpy(&t, &oldterm, sizeof(t));
  rawtty();
  return 0;
}

static int setunbuffered(FILE * fp)
{
#if ELKS
   fflush(fp);
   fp->mode &= ~__MODE_BUF;
   //fp->mode |= _IOFBF;
   fp->bufpos = fp->bufread = fp->bufwrite = fp->bufstart;
#endif
   return 0;
}

#if ELKS
#undef setlinebuf
static int setlinebuf(FILE * fp)
{
   fflush(fp);
   fp->mode &= ~__MODE_BUF;
   fp->mode |= _IOLBF;
   return 0;
}
#endif

void *initscr()
{
    rawmode();
    setunbuffered(stdout);
    return stdout;
}

void endwin()
{
    restoretty();
    setlinebuf(stdout);
}

int has_colors()
{
    return 1;
}

void cbreak()
{
}

void noecho()
{
}

void nonl()
{
}

void intrflush(void *win, int bf)
{
}

void keypad(void *win, int bf)
{
}

void echo()
{
}

void timeout(int t)
{
}

/* cursor on/off */
void curs_set(int visibility)
{
    printf("\e[?25%c", visibility? 'h': 'l');
}

/* clear screen */
void erase()
{
    printf("\e[H\e[2J");
}

void move(int y, int x)
{
    printf("\e[%d;%dH", y, x);
}

void clrtoeol(void)
{
    printf("\e[0K");
}

void printw(char *fmt, ...)
{
    va_list ptr;

    va_start(ptr, fmt);
    vfprintf(stdout,fmt,ptr);
    va_end(ptr);
}

int getch()
{
    int e, n;
    int mx, my, modkeys, status;
    char buf[32];

    fflush(stdout);
    if ((n = readansi(0, buf, sizeof(buf))) < 0)
        return -1;
    if ((e = ansi_to_unikey(buf, n)) != -1) {   // FIXME UTF-8 unicode != -1
        //printf("KBD %x (%d)\n", e, n);
        return e;
    }
    if ((n = ansi_to_unimouse(buf, n, &mx, &my, &modkeys, &status)) != -1) {
        switch (n) {
        case kMouseWheelDown:
        case kMouseWheelUp:
            return n;
        }
    }
    return -1;
}

void wgetnstr(void *win, char *str, int n)
{
    restoretty();
    if (fgets(str, n, stdin))
        str[strlen(str)-1] = '\0';
    else str[0] = '\0';
    rawtty();
}

void start_color()
{
}

void use_default_colors()
{
}

struct cp {
    int fg;
    int bg;
};

static struct cp attrs[17] = {
    { -1, -1 },     /* entry 0 is default attribute */
};

void init_pair(int ndx, int fg, int bg)
{
    if (ndx >= 1 && ndx <= 16) {
        attrs[ndx].fg = fg;
        attrs[ndx].bg = bg;
    }
}

/*                                           blk blu grn cyn red mag yel wht */
static const unsigned char ansi_colors[8] = {30, 34, 32, 36, 31, 35, 33, 37 };

void attron(int a)
{
    int fg = attrs[a & 0x0F].fg;
    int bg = attrs[a & 0x0F].bg;

    if (fg == -1) fg = 39; else fg = ansi_colors[fg];
    if (bg == -1) bg = 39; else bg = ansi_colors[bg];
    if (bg == 39)
        printf("\e[%dm", fg);
    else printf("\e[%d;%dm", fg, bg+10);
}

void attroff(int a)
{
    printf("\e[1;0;0m");
}
