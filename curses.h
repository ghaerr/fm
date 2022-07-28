/* curses.h workaround - Greg Haerr*/

#define KEY_BACKSPACE   kBackSpace
#define KEY_LEFT        kLeftArrow
#define KEY_ENTER       '\r'
#define KEY_RIGHT       kRightArrow
#define KEY_UP          kUpArrow
#define KEY_DOWN        kDownArrow
#define KEY_HOME        kHome
#define KEY_END         kEnd
#define KEY_NPAGE       kPageDown
#define KEY_PPAGE       kPageUp

#define A_NORMAL        0x0700

#define COLOR_BLACK     0
#define COLOR_BLUE      1
#define COLOR_GREEN     2
#define COLOR_CYAN      3
#define COLOR_RED       4
#define COLOR_MAGENTA   5
#define COLOR_YELLOW    6
#define COLOR_WHITE     7

#define COLOR_PAIR(n)   (n)

#define TRUE            1
#define FALSE           0

extern int LINES;
extern int COLS;
extern void *stdscr;

void start_color();
void use_default_colors();
void init_pair(int ndx, int fg, int bg);
void *initscr();
void endwin();
int has_colors();
void cbreak();
void noecho();
void nonl();
void intrflush(void *, int);
void keypad(void *, int);
void echo();
void erase();

void curs_set(int cursoron);
void timeout(int t);
void move(int y, int x);
void clrnl(void);
void clrtoeos(void);
void clrtoeol(void);
void printw(char *, ...);
void waitformouse(void);
int getch();
void wgetnstr(void *, char *, int);
void attron(int a);
void attroff(int a);
