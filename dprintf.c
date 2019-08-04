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
