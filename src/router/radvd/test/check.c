
#include "config.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif


static void usage(char const *pname);
static void version(void);
Suite * util_suite();
Suite * send_suite();

#ifdef HAVE_GETOPT_LONG

/* *INDENT-OFF* */
static char usage_str[] = {
"\n"
"  -h, --help                Print the help and quit.\n"
"  -m, --mode                The print mode (SILENT, MINIMAL, NORMAL, VERBOSE)\n"
"  -s, --suite=suite         The suites to run.\n"
"  -t, --test=test           The unit tests to run.\n"
"  -v, --version             Print the version and quit.\n"
};

static struct option prog_opt[] = {
	{"help", 0, 0, 'h'},
	{"mode", 1, 0, 'm'},
	{"suite", 1, 0, 's'},
	{"test", 1, 0, 't'},
	{"version", 0, 0, 'v'},
	{NULL, 0, 0, 0}
};

#else

static char usage_str[] = {
"[-hv] [-m mode] [-s suite] [-t test]"
};
/* *INDENT-ON* */

#endif

struct options {
	char * suite;
	char * test;
	int mode;
};

static void process_command_line_args(int argc, char * argv[], struct options * options)
{
	char const *pname = ((pname = strrchr(argv[0], '/')) != NULL) ? pname + 1 : argv[0];
	int c;
	char * suite = 0;
	char * test = 0;
	int mode = CK_VERBOSE;
	
	/* parse args */
#define OPTIONS_STR "s:t:m:vh"
#ifdef HAVE_GETOPT_LONG
	int opt_idx;
	while ((c = getopt_long(argc, argv, OPTIONS_STR, prog_opt, &opt_idx)) > 0)
#else
	while ((c = getopt(argc, argv, OPTIONS_STR)) > 0)
#endif
	{
		switch (c) {
		case 'm':
			if (0 == strcmp(optarg, "SILENT")){
				mode = CK_SILENT;
			}
			else if (0 == strcmp(optarg, "MINIMAL")){
				mode = CK_MINIMAL;
			}
			else if (0 == strcmp(optarg, "NORMAL")){
				mode = CK_NORMAL;
			}
			else if (0 == strcmp(optarg, "VERBOSE")){
				mode = CK_VERBOSE;
			}
			else if (0 == strcmp(optarg, "ENV")){
				mode = CK_ENV;
			}
			else {
				fprintf(stderr, "%s: mode, \"%s\", unknown.\n", pname, optarg);
				exit(1);
			}
			break;
		case 's':
			if (suite)
				free(suite);
			suite = strdup(optarg);
			break;
		case 't':
			if (test)
				free(test);
			test = strdup(optarg);
			break;
		case 'v':
			version();
			break;
		case 'h':
			usage(pname);
#ifdef HAVE_GETOPT_LONG
		case ':':
			fprintf(stderr, "%s: option %s: parameter expected\n", pname, prog_opt[opt_idx].name);
			exit(1);
#endif
		case '?':
			exit(1);
		}
	}

	if (options) {
		options->suite = suite;
		options->test = test;
		options->mode = mode;
	}
}


int main(int argc, char * argv[])
{
	srand((unsigned int)time(NULL));

	struct options options = {0, 0, 0};
	process_command_line_args(argc, argv, &options);

	SRunner * sr = srunner_create(util_suite());
	srunner_add_suite(sr, send_suite());
	srunner_run(sr, options.suite, options.test, options.mode);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void usage(char const *pname)
{
	fprintf(stderr, "usage: %s %s\n", pname, usage_str);
	exit(1);
}

static void version(void)
{
	fprintf(stderr, "Version: %s\n\n", VERSION);
	exit(0);
}

