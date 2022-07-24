/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
dprintf(int fd, const char *fmt, ...)
{
	char buf[BUFSIZ];
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (r > 0 && r < sizeof(buf))
		write(fd, buf, r);
	va_end(ap);
	return r;
}

#if 0
void dprintf(const char *fmt, ...)
{
	va_list ap;
    static int fd = -1;
	char buf[LINE_MAX];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
    if (fd < 0) {
        fd = open("/dev/console", 1);
    }
	if (fd >= 0) {
        write(fd, buf, strlen(buf));
    }
}
#endif
