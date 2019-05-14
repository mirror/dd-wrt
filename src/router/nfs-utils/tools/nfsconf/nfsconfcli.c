#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "conffile.h"
#include "xlog.h"

typedef enum {
	MODE_NONE,
	MODE_GET,
	MODE_ISSET,
	MODE_DUMP,
	MODE_SET,
	MODE_UNSET
} confmode_t;

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-v] [--file filename.conf] ...\n", name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, " -v			Increase Verbosity\n");
	fprintf(stderr, " --file filename.conf	Load this config file\n");
	fprintf(stderr, "     (Default config file: " NFS_CONFFILE "\n");
	fprintf(stderr, " --modified \"info\"	Use \"info\" in file modified header\n");
	fprintf(stderr, "Modes:\n");
	fprintf(stderr, "  --dump [outputfile]\n");
	fprintf(stderr, "      Outputs the configuration to the named file\n");
	fprintf(stderr, "  --get [--arg subsection] {section} {tag}\n");
	fprintf(stderr, "      Output one specific config value\n");
	fprintf(stderr, "  --isset [--arg subsection] {section} {tag}\n");
	fprintf(stderr, "      Return code indicates if config value is present\n");
	fprintf(stderr, "  --set [--arg subsection] {section} {tag} {value}\n");
	fprintf(stderr, "      Set and Write a config value\n");
	fprintf(stderr, "  --unset [--arg subsection] {section} {tag}\n");
	fprintf(stderr, "      Remove an existing config value\n");
}

int main(int argc, char **argv)
{
	char * confpath = NFS_CONFFILE;
	char * arg = NULL;
	int verbose=0;
	int ret = 0;
	char * dumpfile = NULL;

	confmode_t mode = MODE_NONE;

	modified_by = "Modified by nfsconf";

	while (1) {
		int c;
		int index = 0;
		struct option long_options[] = {
			{"get",		no_argument, 0, 'g' },
			{"set",		no_argument, 0, 's' },
			{"unset",	no_argument, 0, 'u' },
			{"arg",	  required_argument, 0, 'a' },
			{"isset", 	no_argument, 0, 'i' },
			{"dump",  optional_argument, 0, 'd' },
			{"file",  required_argument, 0, 'f' },
			{"verbose",	no_argument, 0, 'v' },
			{"modified", required_argument, 0, 'm' },
			{NULL,			  0, 0, 0 }
		};

		c = getopt_long(argc, argv, "gsua:id::f:vm:", long_options, &index);
		if (c == -1) break;

		switch (c) {
			case 0:
				break;
			case 'f':
				/* user specified source path for config */
				confpath = optarg;
				break;
			case 'a':
				/* user supplied a sub-section name */
				arg = optarg;
				break;
			case 'v':
				verbose++;
				break;
			case 'g':
				mode = MODE_GET;
				break;
			case 's':
				mode = MODE_SET;
				break;
			case 'u':
				mode = MODE_UNSET;
				break;
			case 'i':
				mode = MODE_ISSET;
				break;
			case 'd':
				/* check if user supplied a filename for dump */
				if (optarg == NULL && argv[optind] != NULL
				    && argv[optind][0] != '-')
					optarg = argv[optind++];
				mode = MODE_DUMP;
				dumpfile = optarg;
				break;
			case 'm':
				if (optarg == NULL || *optarg == 0)
					modified_by = NULL;
				else
					modified_by = optarg;
				break;
			default:
				usage(argv[0]);
				return 1;
		}
	}

	/* configure the logging that conffile.c uses */
	if (verbose)
		xlog_config(D_ALL, 1);
	xlog_stderr(1);
	xlog_syslog(0);
	xlog_open("nfsconf");

	if (mode == MODE_NONE) {
		fprintf(stderr, "Error: No MODE selected.\n");
		usage(argv[0]);
		return 1;
	}

	if (mode != MODE_SET && mode != MODE_UNSET) {
		if (conf_init_file(confpath)) {
			/* config file was missing or had an error, warn about it */
			if (verbose || mode != MODE_ISSET) {
				fprintf(stderr, "Error loading config file %s\n",
					confpath);
			}

			/* this isnt fatal for --isset */
			if (mode != MODE_ISSET)
				return 1;
		}
	}

	/* --dump mode, output the current configuration */
	if (mode == MODE_DUMP) {
		/* default to writing it to stdout */
		FILE *out = stdout;

		/* user specified a file to write to instead */
		if (dumpfile) {
			out = fopen(dumpfile, "w");
			if (out == NULL) {
				fprintf(stderr, "Error opening dumpfile %s: %s\n",
					dumpfile, strerror(errno));
				ret = 2;
				goto cleanup;
			}
			if (verbose)
				printf("Dumping config to %s\n", dumpfile);
		}

		/* output the configuration */
		conf_report(out);

		/* close that user specified file */
		if (dumpfile)
			fclose(out);
	} else
	/* --iset and --get share a lot of code */
	if (mode == MODE_GET || mode == MODE_ISSET) {
		char * section = NULL;
		char * tag = NULL;
		const char * val;

		/* test they supplied section and tag names */
		if (optind+1 >= argc) {
			fprintf(stderr, "Error: insufficient arguments for mode\n");
			usage(argv[0]);
			ret = 2;
			goto cleanup;
		}

		/* now we have a section and tag name */
		section = argv[optind++];
		tag = argv[optind++];

		/* retrieve the specified tags value */
		val = conf_get_section(section, arg, tag);
		if (val != NULL) {
			/* ret=0, success, mode --get wants to output the value as well */
			if (mode == MODE_GET)
				printf("%s\n", val);
		} else {
			/* ret=1, no value found, tell the user if they asked */
			if (mode == MODE_GET && verbose)
				fprintf(stderr, "Tag '%s' not found\n", tag);
			ret = 1;
		}
	} else
	if (mode == MODE_SET || mode == MODE_UNSET) {
		char * section = NULL;
		char * tag = NULL;
		char * val = NULL;
		int need = 2;

		if (mode == MODE_UNSET)
			need = 1;

		/* test they supplied section and tag names */
		if (optind+need >= argc) {
			fprintf(stderr, "Error: insufficient arguments for mode\n");
			usage(argv[0]);
			ret = 2;
			goto cleanup;
		}

		/* now we have a section and tag name */
		section = argv[optind++];
		tag = argv[optind++];
		if (mode == MODE_SET)
			val = argv[optind++];

		/* setting an empty string is same as unsetting */
		if (val!=NULL && *val == '\0') {
			mode = MODE_UNSET;
			val = NULL;
		}

		if (conf_write(confpath, section, arg, tag, val)) {
			if (verbose)
				fprintf(stderr, "Error writing config\n");
			ret = 1;
		}
	} else {
		fprintf(stderr, "Mode not yet implemented.\n");
		ret = 2;
	}

cleanup:
	conf_cleanup();
	return ret;
}
