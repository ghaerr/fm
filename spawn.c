/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/wait.h>

#include <stdarg.h>
#include <unistd.h>

#include "util.h"

void
spawnvp(char *dir, char *file, char *argv[])
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (dir != NULL)
			chdir(dir);
		execvp(file, argv);
		_exit(1);
	} else {
		/* Ignore interruptions */
		while (waitpid(pid, &status, 0) == -1)
			;
	}
}

void
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
	spawnvp(dir, file, argv);
}
