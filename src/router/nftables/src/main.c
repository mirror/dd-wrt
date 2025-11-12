/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>

#include <nftables/libnftables.h>
#include <utils.h>
#include <cli.h>

static struct nft_ctx *nft;

enum opt_indices {
        /* General options */
	IDX_HELP,
	IDX_VERSION,
	IDX_VERSION_LONG,
        /* Ruleset input handling */
	IDX_FILE,
#define IDX_RULESET_INPUT_START	IDX_FILE
	IDX_DEFINE,
	IDX_INTERACTIVE,
        IDX_INCLUDEPATH,
	IDX_CHECK,
	IDX_OPTIMIZE,
#define IDX_RULESET_INPUT_END	IDX_OPTIMIZE
        /* Ruleset list formatting */
        IDX_HANDLE,
#define IDX_RULESET_LIST_START	IDX_HANDLE
        IDX_STATELESS,
        IDX_TERSE,
        IDX_SERVICE,
        IDX_REVERSEDNS,
        IDX_GUID,
        IDX_NUMERIC,
        IDX_NUMERIC_PRIO,
        IDX_NUMERIC_PROTO,
        IDX_NUMERIC_TIME,
#define IDX_RULESET_LIST_END IDX_NUMERIC_TIME
        /* Command output formatting */
        IDX_ECHO,
#define IDX_CMD_OUTPUT_START	IDX_ECHO
        IDX_JSON,
        IDX_DEBUG,
#define IDX_CMD_OUTPUT_END	IDX_DEBUG
};

enum opt_vals {
	OPT_HELP		= 'h',
	OPT_VERSION		= 'v',
	OPT_VERSION_LONG	= 'V',
	OPT_CHECK		= 'c',
	OPT_FILE		= 'f',
	OPT_DEFINE		= 'D',
	OPT_INTERACTIVE		= 'i',
	OPT_INCLUDEPATH		= 'I',
	OPT_JSON		= 'j',
	OPT_NUMERIC		= 'n',
	OPT_STATELESS		= 's',
	OPT_IP2NAME		= 'N',
	OPT_SERVICE		= 'S',
	OPT_DEBUG		= 'd',
	OPT_HANDLE_OUTPUT	= 'a',
	OPT_ECHO		= 'e',
	OPT_GUID		= 'u',
	OPT_NUMERIC_PRIO	= 'y',
	OPT_NUMERIC_PROTO	= 'p',
	OPT_NUMERIC_TIME	= 'T',
	OPT_TERSE		= 't',
	OPT_OPTIMIZE		= 'o',
	OPT_INVALID		= '?',
};

struct nft_opt {
	const char    *name;
	enum opt_vals  val;
	const char    *arg;
	const char    *help;
};

#define NFT_OPT(n, v, a, h) \
	(struct nft_opt) { .name = n, .val = v, .arg = a, .help = h }

static const struct nft_opt nft_options[] = {
	[IDX_HELP]          = NFT_OPT("help",			OPT_HELP,		NULL,
				     "Show this help"),
	[IDX_VERSION]       = NFT_OPT("version",			OPT_VERSION,		NULL,
				     "Show version information"),
	[IDX_VERSION_LONG]  = NFT_OPT(NULL,			OPT_VERSION_LONG,	NULL,
				     "Show extended version information"),
	[IDX_FILE]	    = NFT_OPT("file",			OPT_FILE,		"<filename>",
				     "Read input from <filename>"),
	[IDX_DEFINE]	    = NFT_OPT("define",			OPT_DEFINE,		"<name=value>",
				     "Define variable, e.g. --define foo=1.2.3.4"),
	[IDX_INTERACTIVE]   = NFT_OPT("interactive",		OPT_INTERACTIVE,	NULL,
				     "Read input from interactive CLI"),
	[IDX_INCLUDEPATH]   = NFT_OPT("includepath",		OPT_INCLUDEPATH,	"<directory>",
				     "Add <directory> to the paths searched for include files. Default is: " DEFAULT_INCLUDE_PATH),
	[IDX_CHECK]	    = NFT_OPT("check",			OPT_CHECK,		NULL,
				     "Check commands validity without actually applying the changes."),
	[IDX_HANDLE]	    = NFT_OPT("handle",			OPT_HANDLE_OUTPUT,	NULL,
				     "Output rule handle."),
	[IDX_STATELESS]     = NFT_OPT("stateless",		OPT_STATELESS,		NULL,
				     "Omit stateful information of ruleset."),
	[IDX_TERSE]	    = NFT_OPT("terse",			OPT_TERSE,		NULL,
				      "Omit contents of sets."),
	[IDX_SERVICE]       = NFT_OPT("service",			OPT_SERVICE,		NULL,
				     "Translate ports to service names as described in /etc/services."),
	[IDX_REVERSEDNS]    = NFT_OPT("reversedns",		OPT_IP2NAME,		NULL,
				     "Translate IP addresses to names."),
	[IDX_GUID]	    = NFT_OPT("guid",			OPT_GUID,		NULL,
				     "Print UID/GID as defined in /etc/passwd and /etc/group."),
	[IDX_NUMERIC]       = NFT_OPT("numeric",			OPT_NUMERIC,		NULL,
				     "Print fully numerical output."),
	[IDX_NUMERIC_PRIO]  = NFT_OPT("numeric-priority",	OPT_NUMERIC_PRIO,	NULL,
				     "Print chain priority numerically."),
	[IDX_NUMERIC_PROTO] = NFT_OPT("numeric-protocol",	OPT_NUMERIC_PROTO,	NULL,
				     "Print layer 4 protocols numerically."),
	[IDX_NUMERIC_TIME]  = NFT_OPT("numeric-time",		OPT_NUMERIC_TIME,	NULL,
				      "Print time values numerically."),
	[IDX_ECHO]	    = NFT_OPT("echo",			OPT_ECHO,		NULL,
				     "Echo what has been added, inserted or replaced."),
	[IDX_JSON]	    = NFT_OPT("json",			OPT_JSON,		NULL,
				     "Format output in JSON"),
	[IDX_DEBUG]	    = NFT_OPT("debug",			OPT_DEBUG,		"<level [,level...]>",
				     "Specify debugging level (scanner, parser, eval, netlink, mnl, proto-ctx, segtree, all)"),
	[IDX_OPTIMIZE]	    = NFT_OPT("optimize",		OPT_OPTIMIZE,		NULL,
				     "Optimize ruleset"),
};

#define NR_NFT_OPTIONS (sizeof(nft_options) / sizeof(nft_options[0]))

static const char *get_optstring(void)
{
	static char optstring[2 * NR_NFT_OPTIONS + 2];

	if (!optstring[0]) {
		size_t i, j;

		optstring[0] = '+';
		for (i = 0, j = 1; i < NR_NFT_OPTIONS && j < sizeof(optstring); i++)
			j += snprintf(optstring + j, sizeof(optstring) - j, "%c%s",
				      nft_options[i].val,
				      nft_options[i].arg ? ":" : "");

		assert(j < sizeof(optstring));
	}
	return optstring;
}

static const struct option *get_options(void)
{
	static struct option options[NR_NFT_OPTIONS + 1];

	if (!options[0].name) {
		size_t i, j;

		for (i = 0, j = 0; i < NR_NFT_OPTIONS; ++i) {
			if (nft_options[i].name) {
				options[j].name    = nft_options[i].name;
				options[j].val     = nft_options[i].val;
				options[j].has_arg = nft_options[i].arg != NULL;
				j++;
			}
		}
	}
	return options;
}

static void print_option(const struct nft_opt *opt)
{
	char optbuf[35] = "";
	int i;

	i = snprintf(optbuf, sizeof(optbuf), "  -%c", opt->val);
	if (opt->name)
		i += snprintf(optbuf + i, sizeof(optbuf) - i, ", --%s",
			      opt->name);
	if (opt->arg)
		i += snprintf(optbuf + i, sizeof(optbuf) - i, " %s", opt->arg);

	printf("%-34s%s\n", optbuf, opt->help);
}

static void show_help(const char *name)
{
	int i;

	printf("Usage: %s [ options ] [ cmds... ]\n"
	       "\n"
	       "Options (general):\n", name);

	print_option(&nft_options[IDX_HELP]);
	print_option(&nft_options[IDX_VERSION]);
	print_option(&nft_options[IDX_VERSION_LONG]);

	printf("\n"
	       "Options (ruleset input handling):"
	       "\n");

	for (i = IDX_RULESET_INPUT_START; i <= IDX_RULESET_INPUT_END; i++)
		print_option(&nft_options[i]);

	printf("\n"
	       "Options (ruleset list formatting):"
	       "\n");

	for (i = IDX_RULESET_LIST_START; i <= IDX_RULESET_LIST_END; i++)
		print_option(&nft_options[i]);

	printf("\n"
	       "Options (command output formatting):"
	       "\n");

	for (i = IDX_CMD_OUTPUT_START; i <= IDX_CMD_OUTPUT_END; i++)
		print_option(&nft_options[i]);

	fputs("\n", stdout);
}

static void show_version(void)
{
	const char *cli, *minigmp, *json, *xt;

#if defined(HAVE_LIBREADLINE)
	cli = "readline";
#elif defined(HAVE_LIBEDIT)
	cli = "editline";
#elif defined(HAVE_LIBLINENOISE)
	cli = "linenoise";
#else
	cli = "no";
#endif

#if defined(HAVE_MINIGMP)
	minigmp = "yes";
#else
	minigmp = "no";
#endif

#if defined(HAVE_JSON)
	json = "yes";
#else
	json = "no";
#endif

#if defined(HAVE_XTABLES)
	xt = "yes";
#else
	xt = "no";
#endif

	printf("%s v%s (%s)\n"
	       "  cli:		%s\n"
	       "  json:		%s\n"
	       "  minigmp:	%s\n"
	       "  libxtables:	%s\n",
	       PACKAGE_NAME, PACKAGE_VERSION, RELEASE_NAME,
	       cli, json, minigmp, xt);
}

static const struct {
	const char		*name;
	enum nft_debug_level	level;
} debug_param[] = {
	{
		.name		= "scanner",
		.level		= NFT_DEBUG_SCANNER,
	},
	{
		.name		= "parser",
		.level		= NFT_DEBUG_PARSER,
	},
	{
		.name		= "eval",
		.level		= NFT_DEBUG_EVALUATION,
	},
	{
		.name		= "netlink",
		.level		= NFT_DEBUG_NETLINK,
	},
	{
		.name		= "mnl",
		.level		= NFT_DEBUG_MNL,
	},
	{
		.name		= "proto-ctx",
		.level		= NFT_DEBUG_PROTO_CTX,
	},
	{
		.name		= "segtree",
		.level		= NFT_DEBUG_SEGTREE,
	},
	{
		.name		= "all",
		.level		= ~0,
	},
};

static void nft_options_error(int argc, char * const argv[], int pos)
{
	int i;

	fprintf(stderr, "Error: syntax error, options must be specified before commands\n");
	for (i = 0; i < argc; i++)
		fprintf(stderr, "%s ", argv[i]);
	printf("\n%4c%*s\n", '^', pos - 2, "~~");
}

static bool nft_options_check(int argc, char * const argv[])
{
	bool skip = false, nonoption = false;
	int pos = 0, i;
	size_t j;

	for (i = 1; i < argc; i++) {
		pos += strlen(argv[i - 1]) + 1;
		if (argv[i][0] == '{') {
			break;
		} else if (skip) {
			skip = false;
			continue;
		} else if (argv[i][0] != '-') {
			nonoption = true;
			continue;
		}
		if (nonoption) {
			nft_options_error(argc, argv, pos);
			return false;
		}
		for (j = 0; j < NR_NFT_OPTIONS; j++) {
			if (nft_options[j].arg &&
			    (argv[i][1] == (char)nft_options[j].val ||
			     (argv[i][1] == '-' &&
			      !strcmp(argv[i] + 2, nft_options[j].name)))) {
				skip = true;
				break;
			}
		}
	}

	return true;
}

int main(int argc, char * const *argv)
{
	const struct option *options = get_options();
	bool interactive = false, define = false;
	const char *optstring = get_optstring();
	unsigned int output_flags = 0;
	int i, val, rc = EXIT_SUCCESS;
	unsigned int debug_mask;
	char *filename = NULL;
	unsigned int len;

	/* nftables cannot be used with setuid in a safe way. */
	if (getuid() != geteuid())
		_exit(111);

	if (!nft_options_check(argc, argv))
		exit(EXIT_FAILURE);

	nft = nft_ctx_new(NFT_CTX_DEFAULT);

	while (1) {
		val = getopt_long(argc, argv, optstring, options, NULL);
		if (val == -1)
			break;

		switch (val) {
		case OPT_HELP:
			show_help(argv[0]);
			goto out;
		case OPT_VERSION:
			printf("%s v%s (%s)\n",
			       PACKAGE_NAME, PACKAGE_VERSION, RELEASE_NAME);
			goto out;
		case OPT_VERSION_LONG:
			show_version();
			goto out;
		case OPT_DEFINE:
			if (nft_ctx_add_var(nft, optarg)) {
				fprintf(stderr,
					"Error: Failed to define variable '%s'\n",
					optarg);
				goto out_fail;
			}
			define = true;
			break;
		case OPT_CHECK:
			nft_ctx_set_dry_run(nft, true);
			break;
		case OPT_FILE:
			if (interactive) {
				fprintf(stderr,
					"Error: -i/--interactive and -f/--file options cannot be combined\n");
				goto out_fail;
			}
			filename = optarg;
			break;
		case OPT_INTERACTIVE:
			if (filename) {
				fprintf(stderr,
					"Error: -i/--interactive and -f/--file options cannot be combined\n");
				goto out_fail;
			}
			interactive = true;
			break;
		case OPT_INCLUDEPATH:
			if (nft_ctx_add_include_path(nft, optarg)) {
				fprintf(stderr,
					"Warning: Cannot include path '%s'\n",
					optarg);
			}
			break;
		case OPT_NUMERIC:
			output_flags |= NFT_CTX_OUTPUT_NUMERIC_ALL;
			break;
		case OPT_STATELESS:
			output_flags |= NFT_CTX_OUTPUT_STATELESS;
			break;
		case OPT_IP2NAME:
			output_flags |= NFT_CTX_OUTPUT_REVERSEDNS;
			break;
		case OPT_SERVICE:
			output_flags |= NFT_CTX_OUTPUT_SERVICE;
			break;
		case OPT_DEBUG:
			debug_mask = nft_ctx_output_get_debug(nft);
			for (;;) {
				unsigned int i;
				char *end;

				end = strchr(optarg, ',');
				if (end)
					*end = '\0';

				for (i = 0; i < array_size(debug_param); i++) {
					if (strcmp(debug_param[i].name, optarg))
						continue;
					debug_mask |= debug_param[i].level;
					break;
				}

				if (i == array_size(debug_param)) {
					fprintf(stderr, "Error: invalid debug parameter `%s'\n",
						optarg);
					goto out_fail;
				}

				if (end == NULL)
					break;
				optarg = end + 1;
			}
			nft_ctx_output_set_debug(nft, debug_mask);
			break;
		case OPT_HANDLE_OUTPUT:
			output_flags |= NFT_CTX_OUTPUT_HANDLE;
			break;
		case OPT_ECHO:
			output_flags |= NFT_CTX_OUTPUT_ECHO;
			break;
		case OPT_JSON:
#ifdef HAVE_LIBJANSSON
			output_flags |= NFT_CTX_OUTPUT_JSON;
#else
			fprintf(stderr, "Error: JSON support not compiled-in\n");
			goto out_fail;
#endif
			break;
		case OPT_GUID:
			output_flags |= NFT_CTX_OUTPUT_GUID;
			break;
		case OPT_NUMERIC_PRIO:
			output_flags |= NFT_CTX_OUTPUT_NUMERIC_PRIO;
			break;
		case OPT_NUMERIC_PROTO:
			output_flags |= NFT_CTX_OUTPUT_NUMERIC_PROTO;
			break;
		case OPT_NUMERIC_TIME:
			output_flags |= NFT_CTX_OUTPUT_NUMERIC_TIME;
			break;
		case OPT_TERSE:
			output_flags |= NFT_CTX_OUTPUT_TERSE;
			break;
		case OPT_OPTIMIZE:
			nft_ctx_set_optimize(nft, 0x1);
			break;
		case OPT_INVALID:
			goto out_fail;
		}
	}

	if (!filename && define) {
		fprintf(stderr, "Error: -D/--define can only be used with -f/--filename\n");
		goto out_fail;
	}

	nft_ctx_output_set_flags(nft, output_flags);

	if (optind != argc) {
		char *buf;

		for (len = 0, i = optind; i < argc; i++)
			len += strlen(argv[i]) + strlen(" ");

		buf = calloc(1, len);
		if (buf == NULL) {
			fprintf(stderr, "%s:%u: Memory allocation failure\n",
				__FILE__, __LINE__);
			goto out_fail;
		}
		for (i = optind; i < argc; i++) {
			strcat(buf, argv[i]);
			if (i + 1 < argc)
				strcat(buf, " ");
		}
		rc = !!nft_run_cmd_from_buffer(nft, buf);
		free(buf);
	} else if (filename != NULL) {
		rc = !!nft_run_cmd_from_filename(nft, filename);
	} else if (interactive) {
		if (cli_init(nft) < 0) {
			fprintf(stderr, "%s: interactive CLI not supported in this build\n",
				argv[0]);
			goto out_fail;
		}
	} else {
		fprintf(stderr, "%s: no command specified\n", argv[0]);
		goto out_fail;
	}

out:
	nft_ctx_free(nft);
	return rc;
out_fail:
	nft_ctx_free(nft);
	return EXIT_FAILURE;
}
