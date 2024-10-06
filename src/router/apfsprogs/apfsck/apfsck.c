/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "apfsck.h"
#include "super.h"
#include "version.h"

int fd;
unsigned int options;
static bool weird_state;
static char *progname;

/**
 * usage - Print usage information and exit
 */
static void usage(void)
{
	fprintf(stderr, "usage: %s [-cuvw] device\n", progname);
	exit(1);
}

/**
 * version - Print version information and exit
 */
static void version(void)
{
	if (*GIT_COMMIT)
		printf("apfsck %s\n", GIT_COMMIT);
	else
		printf("apfsck - unknown git commit id\n");
	exit(1);
}

/**
 * system_error - Print a system error message and exit
 */
__attribute__((noreturn)) void system_error(void)
{
	perror(progname);
	exit(1);
}

/**
 * report - Report the issue discovered and exit
 * @context: structure where corruption was found (can be NULL)
 * @message: format string with a short explanation
 */
__attribute__((noreturn, format(printf, 2, 3)))	void report(const char *context,
							    const char *message,
							    ...)
{
	char buf[128];
	va_list args;

	va_start(args, message);
	vsnprintf(buf, sizeof(buf), message, args);
	va_end(args);

	if (context)
		printf("%s: %s\n", context, buf);
	else
		printf("%s\n", buf);

	exit(1);
}

/**
 * report_crash - Report that a crash was discovered and exit
 * @context: structure with signs of a crash
 *
 * Does nothing unless the -c cli option was used.
 */
void report_crash(const char *context)
{
	if (options & OPT_REPORT_CRASH)
		report(context, "the filesystem was not unmounted cleanly.");
}

/**
 * report_unknown - Report the presence of unknown features and exit
 * @feature: the unsupported feature
 *
 * Does nothing unless the -u cli option was used.
 */
void report_unknown(const char *feature)
{
	if (options & OPT_REPORT_UNKNOWN)
		report(feature, "not supported.");
}

/**
 * report_weird - Report unexplained inconsistencies
 * @context: structure where the inconsistency was found
 *
 * Does nothing unless the -w cli option was used.  This function should
 * be called when the specification, and common sense, appear to be in
 * contradiction with the behaviour of actual filesystems.
 */
void report_weird(const char *context)
{
	if (!(options & OPT_REPORT_WEIRD))
		return;

	/*
	 * Several of my test images have 'weird' issues, so don't exit right
	 * away.  Remember that an issue was found, for the exit code.
	 */
	printf("%s: odd inconsistency (may not be corruption).\n", context);
	weird_state = true;
}

int main(int argc, char *argv[])
{
	char *filename;

	progname = argv[0];
	while (1) {
		int opt = getopt(argc, argv, "cuvw");

		if (opt == -1)
			break;

		switch (opt) {
		case 'c':
			options |= OPT_REPORT_CRASH;
			break;
		case 'u':
			options |= OPT_REPORT_UNKNOWN;
			break;
		case 'w':
			options |= OPT_REPORT_WEIRD;
			break;
		case 'v':
			version();
		default:
			usage();
		}
	}

	if (optind != argc - 1)
		usage();
	filename = argv[optind];

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		system_error();

	parse_filesystem();
	if (weird_state)
		return 1;
	return 0;
}
