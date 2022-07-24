/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/types.h>

#include "curses.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "arg.h"
#include "util.h"
#include "unikey.h"

#if ELKS
char *realpath(const char *path, char resolved[PATH_MAX]);
#define MAX_NAME    20
#else
#define MAX_NAME    32
#include <locale.h>
#endif

#define ISODD(x) ((x) & 1)
#define CONTROL(c) ((c) ^ 0x40)
#define META(c) ((c) ^ 0x80)

struct cpair {
	int fg;
	int bg;
};

/* Supported actions */
enum action {
	SEL_QUIT = 1,
	SEL_BACK,
	SEL_GOIN,
	SEL_FLTR,
	SEL_NEXT,
	SEL_PREV,
	SEL_PGDN,
	SEL_PGUP,
	SEL_HOME,
	SEL_END,
	SEL_CD,
	SEL_CDHOME,
	SEL_TOGGLEDOT,
	SEL_DSORT,
	SEL_SSIZE,
	SEL_MTIME,
	SEL_ICASE,
	SEL_VERS,
	SEL_REDRAW,
	SEL_RUN,
	SEL_RUNARG,
};

struct key {
	int sym;         /* Key pressed */
	enum action act; /* Action */
	char *run;       /* Program to run */
	char *env;       /* Environment variable override */
};

#include "fm.h"

struct entry {
	char name[PATH_MAX];
	mode_t mode;
	time_t t;
    unsigned long size;
};

/* Global context */
struct entry *dents;
char *argv0;
int ndents, cur;
int idle;

/*
 * Layout:
 * .---------
 * | /mnt/path
 * |
 * |    file0
 * |    file1
 * |  > file2
 * |    file3
 * |    file4
 *      ...
 * |    filen
 * |
 * | Permission denied
 * '------
 */

void info(char *, ...);
void warn(char *, ...);
void fatal(char *, ...);

void *
xrealloc(void *p, size_t size)
{
	p = realloc(p, size);
	if (p == NULL)
		fatal("realloc");
	return p;
}

/* Some implementations of dirname(3) may modify `path' and some
 * return a pointer inside `path'. */
char *
xdirname(const char *path)
{
	static char out[PATH_MAX];
	char tmp[PATH_MAX], *p;

	strlcpy(tmp, path, sizeof(tmp));
	p = dirname(tmp);
	if (p == NULL)
		fatal("dirname");
	strlcpy(out, p, sizeof(out));
	return out;
}

char *
xgetenv(char *name, char *fallback)
{
	char *value;

	if (name == NULL)
		return fallback;
	value = getenv(name);
	return value && value[0] ? value : fallback;
}

/*
 * Routine to see if a text string is matched by a wildcard pattern.
 * Returns TRUE if the text is matched, or FALSE if it is not matched
 * or if the pattern is invalid.
 *  *		matches zero or more characters
 *  ?		matches a single character
 *  [abc]	matches 'a', 'b' or 'c'
 *  \c		quotes character c
 *  Adapted from code written by Ingo Wilken.
 *  Adapted from ELKS sash shell.
 */
static int match(char *text, char *pattern)
{
	char	*retrypat;
	char	*retrytxt;
	int	ch;
	int	found;

	if (!showhidden && text[0] == '.')
		return FALSE;
	if (pattern[0] == '\0')
		return TRUE;

	retrypat = NULL;
	retrytxt = NULL;

	while (*text || *pattern) {
		ch = *pattern++;

		switch (ch) {
			case '*':
				retrypat = pattern;
				retrytxt = text;
				break;

			case '[':
				found = FALSE;
				while ((ch = *pattern++) != ']') {
					if (ch == '\\')
						ch = *pattern++;
					if (ch == '\0')
						return FALSE;
					if (*text == ch)
						found = TRUE;
				}
				if (!found) {
					pattern = retrypat;
					text = ++retrytxt;
				}
				/* fall into next case */

			case '?':
				if (*text++ == '\0')
					return FALSE;
				break;

			case '\\':
				ch = *pattern++;
				if (ch == '\0')
					return FALSE;
				/* fall into next case */

			default:
				if (*text == ch) {
					if (*text)
						text++;
					break;
				}
				if (*text) {
					pattern = retrypat;
					text = ++retrytxt;
					break;
				}
				return FALSE;
		}

		if (pattern == NULL)
			return FALSE;
	}
	return TRUE;
}

void
initfilter(char **ifilter)
{
	*ifilter = "";
}

int
visible(char *filter, char *file)
{
    return match(file, filter);
}

int
dircmp(mode_t a, mode_t b)
{
	if (S_ISDIR(a) && S_ISDIR(b))
		return 0;
	if (!S_ISDIR(a) && !S_ISDIR(b))
		return 0;
	if (S_ISDIR(a))
		return -1;
	else
		return 1;
}

/* return -1/0/1 based on sign of x */
static int sign(long x)
{
    return (x > 0) - (x < 0);
}

int
entrycmp(const void *va, const void *vb)
{
	const struct entry *a = va, *b = vb;

	if (dirorder) {
		if (dircmp(a->mode, b->mode) != 0)
			return dircmp(a->mode, b->mode);
	}

	if (sizeorder)
		return sign(a->size - b->size);
	if (mtimeorder)
		return sign(b->t - a->t);
	if (icaseorder)
		return strcasecmp(a->name, b->name);
	if (versorder)
		return strverscmp(a->name, b->name);
	return strcmp(a->name, b->name);
}

void
initcolor(void)
{
	int i;

	start_color();
	use_default_colors();
	for (i = 1; i < LEN(pairs); i++)
		init_pair(i, pairs[i].fg, pairs[i].bg);
}

void
initcurses(void)
{
	char *term;

	if (initscr() == NULL) {
		term = getenv("TERM");
		if (term != NULL)
			fprintf(stderr, "error opening terminal: %s\n", term);
		else
			fprintf(stderr, "failed to initialize curses\n");
		exit(1);
	}
	if (usecolor && has_colors())
		initcolor();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(FALSE); /* Hide cursor */
	timeout(1000); /* One second */
}

void
exitcurses(void)
{
	endwin(); /* Restore terminal */
}

/* Messages show up at the bottom */
void
info(char *fmt, ...)
{
	char buf[LINE_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	move(LINES - 1, 0);
	printw("%s", buf);
}

/* Display warning as a message */
void
warn(char *fmt, ...)
{
	char buf[LINE_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	move(LINES - 1, 0);
	printw("%s: %s\n", buf, strerror(errno));
}

/* Kill curses and display error before exiting */
void
fatal(char *fmt, ...)
{
	va_list ap;

	exitcurses();
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror(errno));
	va_end(ap);
	exit(1);
}

/* Clear the last line */
void
clearprompt(void)
{
	move(LINES - 1, 0);
    clrtoeol();
}

/* Print prompt on the last line */
void
printprompt(char *str)
{
	clearprompt();
	info("%s", str);
    fflush(stdout);
}

int
xgetch(void)
{
	int c;

	c = getch();
	if (c == -1)
		idle++;
	else
		idle = 0;
	return c;
}

/* Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}) */
int
nextsel(char **run, char **env)
{
	int c, i;

	c = xgetch();
	if (c == 033)
		c = META(xgetch());
	if (c >= 'a' && c <= 'z')
		return c;
	for (i = 0; i < LEN(bindings); i++)
		if (c == bindings[i].sym) {
			*run = bindings[i].run;
			*env = bindings[i].env;
			return bindings[i].act;
		}
	return 0;
}

char *
readln(void)
{
	static char ln[LINE_MAX];

	timeout(-1);
	echo();
	curs_set(TRUE);
	memset(ln, 0, sizeof(ln));
	wgetnstr(stdscr, ln, sizeof(ln) - 1);
	noecho();
	curs_set(FALSE);
	timeout(1000);
	return ln[0] ? ln : NULL;
}

int
canopendir(char *path)
{
	DIR *dirp;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;
	closedir(dirp);
	return 1;
}

char *
mkpath(char *dir, char *name, char *out, size_t n)
{
	/* Handle absolute path */
	if (name[0] == '/') {
		strlcpy(out, name, n);
	} else {
		/* Handle root case */
		if (strcmp(dir, "/") == 0) {
			strlcpy(out, "/", n);
			strlcat(out, name, n);
		} else {
			strlcpy(out, dir, n);
			strlcat(out, "/", n);
			strlcat(out, name, n);
		}
	}
	return out;
}


/*
 * Get the time to be used for a file.
 * This is down to the minute for new files, but only the date for old files.
 * The string is returned from a static buffer, and so is overwritten for
 * each call.
 */
static char *timestring(time_t t)
{
    time_t  now;
    char  *str;
    static char buf[26];

    time(&now);

    str = ctime(&t);

    strcpy(buf, &str[4]);
    buf[12] = '\0';

    if ((t > now) || (t < now - 180*24*60L*60)) {
        buf[7] = ' ';
        strcpy(&buf[8], &str[20]);
        buf[12] = '\0';
    }

    return buf;
}

void
printent(struct entry *ent, int active)
{
	unsigned int len = COLS - strlen(CURSR) - 1;
	char cm = 0;
	int attr = 0;
	char name[PATH_MAX];

	/* Copy name locally */
	strlcpy(name, ent->name, sizeof(name));

	/* No text wrapping in entries */
	if (strlen(name) < len)
		len = strlen(name) + 1;

	if (S_ISDIR(ent->mode)) {
		cm = '/';
		attr |= DIR_ATTR;
	} else if (S_ISLNK(ent->mode)) {
		cm = '@';
		attr |= LINK_ATTR;
	} else if (S_ISSOCK(ent->mode)) {
		cm = '=';
		attr |= SOCK_ATTR;
	} else if (S_ISFIFO(ent->mode)) {
		cm = '|';
		attr |= FIFO_ATTR;
	} else if (ent->mode & S_IXUSR) {
		cm = '*';
		attr |= EXEC_ATTR;
	}

	if (active)
		attr |= CURSR_ATTR;

	if (cm) {
		name[len - 1] = cm;
		name[len] = '\0';
	}

	attron(attr);
    name[MAX_NAME] = '\0';
	printw("%s%*s %9lu  %s\n", active ? CURSR : EMPTY, -MAX_NAME, name, ent->size,
        timestring(ent->t));
	attroff(attr);
}

int
dentfill(char *path, struct entry **dents,
	 int (*filter)(char *, char *), char *filterstring)
{
	char newpath[PATH_MAX];
	DIR *dirp;
	struct dirent *dp;
	struct stat sb;
	int r, n = 0;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;

	while ((dp = readdir(dirp)) != NULL) {
		/* Skip self and parent */
		if (strcmp(dp->d_name, ".") == 0 ||
		    strcmp(dp->d_name, "..") == 0)
			continue;
		if (filter(filterstring, dp->d_name) == 0)
			continue;
		*dents = xrealloc(*dents, (n + 1) * sizeof(**dents));
		strlcpy((*dents)[n].name, dp->d_name, sizeof((*dents)[n].name));
		/* Get mode flags */
		mkpath(path, dp->d_name, newpath, sizeof(newpath));
		r = lstat(newpath, &sb);
		if (r == -1)
			fatal("lstat");
		(*dents)[n].mode = sb.st_mode;
		(*dents)[n].t = sb.st_mtime;
		(*dents)[n].size = sb.st_size;
		n++;
	}

	/* Should never be null */
	r = closedir(dirp);
	if (r == -1)
		fatal("closedir");
	return n;
}

void
dentfree(struct entry *dents)
{
	free(dents);
}

/* Return the position of the matching entry or 0 otherwise */
int
dentfind(struct entry *dents, int n, char *cwd, char *path)
{
	char tmp[PATH_MAX];
	int i;

	if (path == NULL)
		return 0;
	for (i = 0; i < n; i++) {
		mkpath(cwd, dents[i].name, tmp, sizeof(tmp));
		DPRINTF_S(path);
		DPRINTF_S(tmp);
		if (strcmp(tmp, path) == 0)
			return i;
	}
	return 0;
}

int
populate(char *path, char *oldpath, char *fltr)
{
	/* Can fail when permissions change while browsing */
	if (canopendir(path) == 0)
		return -1;

	dentfree(dents);

	ndents = 0;
	dents = NULL;

	ndents = dentfill(path, &dents, visible, fltr);
	if (ndents == 0)
		return 0; /* Empty result */

	qsort(dents, ndents, sizeof(*dents), entrycmp);

	/* Find cur from history */
	cur = dentfind(dents, ndents, path, oldpath);
	return 0;
}

void
redraw(char *path)
{
	char cwd[PATH_MAX], cwdresolved[PATH_MAX];
	size_t ncols;
	int nlines, odd;
	int i;

	nlines = MIN(LINES - 4, ndents);

	/* Clean screen */
	erase();

	/* Strip trailing slashes */
	for (i = strlen(path) - 1; i > 0; i--)
		if (path[i] == '/')
			path[i] = '\0';
		else
			break;

	DPRINTF_D(cur);
	DPRINTF_S(path);

	/* No text wrapping in cwd line */
	ncols = COLS;
	if (ncols > PATH_MAX)
		ncols = PATH_MAX;
	strlcpy(cwd, path, ncols);
	cwd[ncols - strlen(CWD) - 1] = '\0';
	realpath(cwd, cwdresolved);

	printw(CWD "%s\n\n", cwdresolved);

	/* Print listing */
	odd = ISODD(nlines);
	if (cur < nlines / 2) {
		for (i = 0; i < nlines; i++)
			printent(&dents[i], i == cur);
	} else if (cur >= ndents - nlines / 2) {
		for (i = ndents - nlines; i < ndents; i++)
			printent(&dents[i], i == cur);
	} else {
		for (i = cur - nlines / 2;
		     i < cur + nlines / 2 + odd; i++)
			printent(&dents[i], i == cur);
	}
}

int tolower(int c)
{
    return (unsigned) (c - 'A') < 26u ? c + ('a' - 'A') : c;
}

void
browse(char *ipath, char *ifilter)
{
	char path[PATH_MAX], oldpath[PATH_MAX], newpath[PATH_MAX];
	char fltr[LINE_MAX];
	char *dir, *tmp, *run, *env;
	struct stat sb;
	int r, fd, shellscript, c;

	strlcpy(path, ipath, sizeof(path));
	strlcpy(fltr, ifilter, sizeof(fltr));
	oldpath[0] = '\0';
begin:
	r = populate(path, oldpath, fltr);
	if (r == -1) {
		warn("populate");
		goto nochange;
	}

	for (;;) {
		redraw(path);
nochange:
		switch (c = nextsel(&run, &env)) {
		case SEL_QUIT:
			dentfree(dents);
			return;
		case SEL_BACK:
			/* There is no going back */
			if (strcmp(path, "/") == 0 ||
			    strcmp(path, ".") == 0 ||
			    strchr(path, '/') == NULL)
				goto nochange;
			dir = xdirname(path);
			if (canopendir(dir) == 0) {
				warn("canopendir");
				goto nochange;
			}
			/* Save history */
			strlcpy(oldpath, path, sizeof(oldpath));
			strlcpy(path, dir, sizeof(path));
			/* Reset filter */
			strlcpy(fltr, ifilter, sizeof(fltr));
			goto begin;
		case SEL_GOIN:
			/* Cannot descend in empty directories */
			if (ndents == 0)
				goto nochange;

			mkpath(path, dents[cur].name, newpath, sizeof(newpath));
			DPRINTF_S(newpath);

			/* Get path info */
			fd = open(newpath, O_RDONLY /*| O_NONBLOCK*/);
			if (fd == -1) {
				warn("open");
				goto nochange;
			}
			r = fstat(fd, &sb);
			if (r == -1) {
				warn("fstat");
				close(fd);
				goto nochange;
			}
            if (((sb.st_mode & S_IFMT) == S_IFREG) && (sb.st_mode & S_IXUSR)) {
                int n, i;
                char buf[16];
                shellscript = 1;
                n = read(fd, buf, sizeof(buf));
                for (i=0; i < n; i++) {
                    if (buf[i] < 9 || buf[i] >= 127)
                        shellscript = 0;
                }
            } else shellscript = 0;
			close(fd);
			DPRINTF_U(sb.st_mode);

			switch (sb.st_mode & S_IFMT) {
			case S_IFDIR:
				if (canopendir(newpath) == 0) {
					warn("canopendir");
					goto nochange;
				}
				strlcpy(path, newpath, sizeof(path));
				/* Reset filter */
				strlcpy(fltr, ifilter, sizeof(fltr));
				goto begin;
			case S_IFREG:
                if ((sb.st_mode & S_IXUSR) && !shellscript) {
                    info("Executable");
                    goto nochange;
                }
				exitcurses();
				run = xgetenv("NOPEN", NOPEN);
				r = spawnlp(path, run, run, newpath, (void *)0);
				initcurses();
				if (r == -1) {
					info("Failed to execute %s", run);
					goto nochange;
				}
				continue;
			default:
				info("Unsupported file");
				goto nochange;
			}
		case SEL_FLTR:
			/* Read filter */
			printprompt("/");
			tmp = readln();
			if (tmp == NULL)
				tmp = ifilter;
			strlcpy(fltr, tmp, sizeof(fltr));
			DPRINTF_S(fltr);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
        case 'a' ... 'z':
            for (r = 0; r < ndents; r++) {
                if (++cur >= ndents)
                    cur = 0;
                if ((tolower(dents[cur].name[0])) == c)
                    break;
            }
            break;
		case SEL_NEXT:
			if (cur < ndents - 1)
				cur++;
			break;
		case SEL_PREV:
			if (cur > 0)
				cur--;
			break;
		case SEL_PGDN:
			if (cur < ndents - 1)
				cur += MIN((LINES - 4) / 2, ndents - 1 - cur);
			break;
		case SEL_PGUP:
			if (cur > 0)
				cur -= MIN((LINES - 4) / 2, cur);
			break;
		case SEL_HOME:
			cur = 0;
			break;
		case SEL_END:
			cur = ndents - 1;
			break;
		case SEL_CD:
			/* Read target dir */
			printprompt("chdir: ");
			tmp = readln();
            clearprompt();
			if (tmp == NULL) {
				//clearprompt();
				goto nochange;
			}
			mkpath(path, tmp, newpath, sizeof(newpath));
			if (canopendir(newpath) == 0) {
				warn("canopendir");
				goto nochange;
			}
			strlcpy(path, newpath, sizeof(path));
			/* Reset filter */
			strlcpy(fltr, ifilter, sizeof(fltr));
			DPRINTF_S(path);
			goto begin;
		case SEL_CDHOME:
			tmp = getenv("HOME");
			if (tmp == NULL) {
				clearprompt();
				goto nochange;
			}
			if (canopendir(tmp) == 0) {
				warn("canopendir");
				goto nochange;
			}
			strlcpy(path, tmp, sizeof(path));
			/* Reset filter */
			strlcpy(fltr, ifilter, sizeof(fltr));
			DPRINTF_S(path);
			goto begin;
		case SEL_TOGGLEDOT:
			showhidden ^= 1;
			initfilter(&ifilter);
			strlcpy(fltr, ifilter, sizeof(fltr));
			goto begin;
        case SEL_SSIZE:
            sizeorder = !sizeorder;
            mtimeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_MTIME:
			mtimeorder = !mtimeorder;
            sizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_DSORT:
			dirorder = !dirorder;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_ICASE:
			icaseorder = !icaseorder;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_VERS:
			versorder = !versorder;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_REDRAW:
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_RUN:
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			run = xgetenv(env, run);
			exitcurses();
			spawnlp(path, run, run, (void *)0);
			initcurses();
			goto begin;
		case SEL_RUNARG:
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			run = xgetenv(env, run);
			exitcurses();
			spawnlp(path, run, run, dents[cur].name, (void *)0);
			initcurses();
			goto begin;
		}
		/* Screensaver */
		if (idletimeout != 0 && idle == idletimeout) {
			idle = 0;
			exitcurses();
			spawnlp(NULL, idlecmd, idlecmd, (void *)0);
			initcurses();
		}
	}
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [-c] [dir]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char cwd[PATH_MAX], *ipath;
	char *ifilter;

	ARGBEGIN {
	case 'c':
		usecolor = 1;
		break;
	default:
		usage();
	} ARGEND

	if (argc > 1)
		usage();

	/* Confirm we are in a terminal */
	if (!isatty(0) || !isatty(1)) {
		fprintf(stderr, "stdin or stdout is not a tty\n");
		exit(1);
	}

	if (getuid() == 0)
		showhidden = 1;
	initfilter(&ifilter);

	if (argv[0] != NULL) {
		ipath = argv[0];
	} else {
		ipath = getcwd(cwd, sizeof(cwd));
		if (ipath == NULL)
			ipath = "/";
	}

	signal(SIGINT, SIG_IGN);

	/* Test initial path */
	if (canopendir(ipath) == 0) {
		fprintf(stderr, "%s: %s\n", ipath, strerror(errno));
		exit(1);
	}

#if !ELKS
	/* Set locale before curses setup */
	setlocale(LC_ALL, "");
#endif
	initcurses();
	browse(ipath, ifilter);
	exitcurses();
	exit(0);
}
