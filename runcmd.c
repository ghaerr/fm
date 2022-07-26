/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

int
runcmd(char *dir, char *cmd, char *file, int shflag)
{
	pid_t pid;
	int status, r;

	pid = fork();
	switch (pid) {
	case -1:
		return -1;
	case 0:
		if (dir != NULL && chdir(dir) == -1)
			exit(1);
		if (shflag) {
			execl("/bin/sh", "sh", "-c", cmd, (char*)0);
		} else {
			execlp(cmd, cmd, file, (char *)0);
		}
		_exit(1);
	default:
		while ((r = waitpid(pid, &status, 0)) == -1 && errno == EINTR)
			continue;
		if (r == -1)
			return -1;
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			return -1;
	}
	return 0;
}
