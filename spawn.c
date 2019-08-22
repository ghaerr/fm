/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include "util.h"

int
spawnvp(char *dir, char *file, char *argv[])
{
	pid_t pid;
	int status;

	pid = fork();
	switch (pid) {
	case -1:
		return -1;
	case 0:
		if (dir != NULL)
			chdir(dir);
		execvp(file, argv);
		_exit(1);
	default:
		while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
			;
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			return -1;
	}
	return 0;
}

int
spawnlp(char *dir, char *file, char *argv0, ...)
{
	char *argv[NR_ARGS];
	va_list ap;
	int argc;

	va_start(ap, argv0);
	argv[0] = argv0;
	for (argc = 1; argv[argc] = va_arg(ap, char *); argc++)
		;
	argv[argc] = NULL;
	va_end(ap);
	return spawnvp(dir, file, argv);
}
