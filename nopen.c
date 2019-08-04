/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

struct assoc {
	char *regex; /* Regex to match on filename */
	char *file;
	char *argv[NR_ARGS];
	regex_t regcomp;
};

#include "nopenconf.h"

void
spawnassoc(struct assoc *assoc, char *arg)
{
	char *argv[NR_ARGS];
	int i;

	for (i = 0; assoc->argv[i]; i++) {
		if (strcmp(assoc->argv[i], "{}") == 0) {
			argv[i] = arg;
			continue;
		}
		argv[i] = assoc->argv[i];
	}
	argv[i] = NULL;
	spawnvp(NULL, assoc->file, argv);
}

struct assoc *
openwith(char *file)
{
	int i;

	for (i = 0; i < LEN(assocs); i++) {
		if (regexec(&assocs[i].regcomp, file, 0, NULL, 0) == 0)
			return &assocs[i];
	}

	return NULL;
}

void
initassocs(void)
{
	char errbuf[256];
	int i, r;

	for (i = 0; i < LEN(assocs); i++) {
		r = regcomp(&assocs[i].regcomp, assocs[i].regex,
			    REG_NOSUB | REG_EXTENDED | REG_ICASE);
		if (r != 0) {
			regerror(r, &assocs[i].regcomp, errbuf, sizeof(errbuf));
			fprintf(stderr, "invalid regex assocs[%d]: %s: %s\n",
			        i, assocs[i].regex, errbuf);
			exit(1);
		}
	}
}

void
usage(char *argv0)
{
	fprintf(stderr, "usage: %s file...\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	if (argc == 1)
		usage(argv[0]);
	argc--;
	argv++;
	initassocs();
	for (; *argv != NULL; argv++) {
		struct assoc *assoc;

		if ((assoc = openwith(argv[0])) == NULL)
			continue;
		spawnassoc(assoc, argv[0]);
	}
	return 0;
}
