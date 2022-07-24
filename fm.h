/* See LICENSE file for copyright and license details. */
#define CWD   ""
#define CURSR " > "
#define EMPTY "   "

#define NOPEN "more"

int dirorder    = 0; /* Set to 1 to sort by directory first */
int sizeorder  = 0;  /* Set to 1 to sort by file size */
int mtimeorder  = 0; /* Set to 1 to sort by time modified */
int icaseorder  = 0; /* Set to 1 to sort by ignoring case */
int versorder   = 0; /* Set to 1 to sort by version number */
int idletimeout = 0; /* Screensaver timeout in seconds, 0 to disable */
int showhidden  = 0; /* Set to 1 to show hidden files by default */
int usecolor    = 1; /* Set to 1 to enable color attributes */
char *idlecmd   = "rain"; /* The screensaver program */

/* See curs_attr(3) for valid video attributes */
#define CURSR_ATTR A_NORMAL
#define DIR_ATTR   A_NORMAL | COLOR_PAIR(4)
#define LINK_ATTR  A_NORMAL | COLOR_PAIR(6)
#define SOCK_ATTR  A_NORMAL | COLOR_PAIR(1)
#define FIFO_ATTR  A_NORMAL | COLOR_PAIR(5)
#define EXEC_ATTR  A_NORMAL | COLOR_PAIR(2)

/* Colors to use with COLOR_PAIR(n) as attributes */
struct cpair pairs[] = {
	{ .fg = 0, .bg = 0 },
	/* pairs start at 1 */
	{ COLOR_RED,     -1 },
	{ COLOR_GREEN,   -1 },
	{ COLOR_YELLOW,  -1 },
	{ COLOR_BLUE,    -1 },
	{ COLOR_MAGENTA, -1 },
	{ COLOR_CYAN,    -1 },
};

struct key bindings[] = {
	/* Quit */
	{ 'Z',            SEL_QUIT },
	/* Back */
	{ KEY_BACKSPACE,  SEL_BACK },
	{ 127,            SEL_BACK },
	{ KEY_LEFT,       SEL_BACK },
	{ CONTROL('H'),   SEL_BACK },
	/* Inside */
	{ KEY_ENTER,      SEL_GOIN },
	{ '\r',           SEL_GOIN },
	{ KEY_RIGHT,      SEL_GOIN },
	/* Filter */
	{ '/',            SEL_FLTR },
	/* Next */
	{ KEY_DOWN,       SEL_NEXT },
	{ MOUSE_WHEEL_DOWN,SEL_NEXT },
	{ CONTROL('N'),   SEL_NEXT },
	/* Previous */
	{ KEY_UP,         SEL_PREV },
	{ CONTROL('P'),   SEL_PREV },
	{ MOUSE_WHEEL_UP, SEL_PREV },
	/* Page down */
	{ KEY_NPAGE,      SEL_PGDN },
	{ CONTROL('D'),   SEL_PGDN },
	/* Page up */
	{ KEY_PPAGE,      SEL_PGUP },
	{ CONTROL('U'),   SEL_PGUP },
	/* Home */
	{ KEY_HOME,       SEL_HOME },
	//{ META('<'),      SEL_HOME },
	{ '^',            SEL_HOME },
	{ 'H',            SEL_HOME },
	/* End */
	{ KEY_END,        SEL_END },
	//{ META('>'),      SEL_END },
	{ '$',            SEL_END },
	{ 'B',            SEL_END },
	/* Change dir */
	{ 'C',            SEL_CD },
	{ '~',            SEL_CDHOME },
	/* Toggle hide .dot files */
	{ '.',            SEL_TOGGLEDOT },
	/* Toggle sort by directory first */
	{ 'D',            SEL_DSORT },
	/* Toggle sort by size */
	{ 'S',            SEL_SSIZE },
	/* Toggle sort by time */
	{ 'T',            SEL_MTIME },
	/* Toggle case sensitivity */
	{ 'I',            SEL_ICASE },
	/* Toggle sort by version number */
	{ 'V',            SEL_VERS },
	{ CONTROL('L'),   SEL_REDRAW },
	/* Run command */
	{ '!',            SEL_RUN, "sh", "SHELL" },
	/* Run command with argument */
	{ 'E',            SEL_RUNARG, "vi", "EDITOR" },
	{ 'M',            SEL_RUNARG, "more", "PAGER" },
};
