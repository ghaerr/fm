/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/wait.h>

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arg.h"
#include "util.h"

struct rule {
	char *regex; /* Regex to match on filename */
	char *file;
	char *argv[NR_ARGS];
	regex_t regcomp;
};

#include "nopenconf.h"

char *argv0;

int
run(struct rule *rule, char *arg)
{
	char *argv[NR_ARGS];
	int i;

	for (i = 0; rule->argv[i]; i++) {
		if (strcmp(rule->argv[i], "{}") == 0) {
			argv[i] = arg;
			continue;
		}
		argv[i] = rule->argv[i];
	}
	argv[i] = NULL;
	return spawnvp(NULL, rule->file, argv);
}

struct rule *
matchrule(char *file)
{
	int i;

	for (i = 0; i < LEN(rules); i++) {
		if (regexec(&rules[i].regcomp, file, 0, NULL, 0) == 0)
			return &rules[i];
	}
	return NULL;
}

void
parserules(void)
{
	char errbuf[256];
	int i, r;

	for (i = 0; i < LEN(rules); i++) {
		r = regcomp(&rules[i].regcomp, rules[i].regex,
			    REG_NOSUB | REG_EXTENDED | REG_ICASE);
		if (r != 0) {
			regerror(r, &rules[i].regcomp, errbuf, sizeof(errbuf));
			fprintf(stderr, "invalid regex rules[%d]: %s: %s\n",
			        i, rules[i].regex, errbuf);
			exit(1);
		}
	}
}

void
usage(void)
{
	fprintf(stderr, "usage: %s file...\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int r;

	ARGBEGIN {
	default:
		usage();
	} ARGEND

	if (argc == 0)
		usage();

	r = 0;
	parserules();
	for (; argv[0] != NULL; argv++) {
		struct rule *rule;

		if ((rule = matchrule(argv[0])) == NULL)
			continue;
		if (run(rule, argv[0]) == -1)
			r = 1;
	}
	return r;
}
