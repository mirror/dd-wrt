/**
 * Main for the `jool` and `jool_siit` userspace applications.
 * Handles the first arguments (often "mode" and "operation") and multiplexes
 * the rest of the work to the corresponding .c's.
 */

#include <argp.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "common/config.h"
#include "common/xlat.h"
#include "usr/util/str_utils.h"
#include "usr/nl/file.h"
#include "usr/argp/command.h"
#include "usr/argp/log.h"
#include "usr/argp/xlator_type.h"
#include "usr/argp/wargp/address.h"
#include "usr/argp/wargp/bib.h"
#include "usr/argp/wargp/denylist4.h"
#include "usr/argp/wargp/eamt.h"
#include "usr/argp/wargp/file.h"
#include "usr/argp/wargp/global.h"
#include "usr/argp/wargp/instance.h"
#include "usr/argp/wargp/joold.h"
#include "usr/argp/wargp/pool4.h"
#include "usr/argp/wargp/session.h"
#include "usr/argp/wargp/stats.h"

#define DISPLAY "display"
#define ADD "add"
#define UPDATE "update"
#define REMOVE "remove"
#define FLUSH "flush"

static int handle_autocomplete(char *junk, int argc, char **argv, void const *arg);

static struct cmd_option instance_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_ANY,
			.handler = handle_instance_display,
			.handle_autocomplete = autocomplete_instance_display,
		}, {
			.label = ADD,
			.xt = XT_ANY,
			.handler = handle_instance_add,
			.handle_autocomplete = autocomplete_instance_add,
		}, {
			.label = REMOVE,
			.xt = XT_ANY,
			.handler = handle_instance_remove,
			.handle_autocomplete = autocomplete_instance_remove,
		}, {
			.label = FLUSH,
			.xt = XT_ANY,
			.handler = handle_instance_flush,
			.handle_autocomplete = autocomplete_instance_flush,
		}, {
			.label = "status",
			.xt = XT_ANY,
			.handler = handle_instance_status,
			.handle_autocomplete = autocomplete_instance_status,
		},
		{ 0 },
};

static struct cmd_option stats_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_ANY,
			.handler = handle_stats_display,
			.handle_autocomplete = autocomplete_stats_display,
		},
		{ 0 },
};

static struct cmd_option address_ops[] = {
		{
			.label = "query",
			.xt = XT_SIIT,
			.handler = handle_address_query,
			.handle_autocomplete = autocomplete_address_query,
		},
		{ 0 },
};

static struct cmd_option global_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_ANY,
			.handler = handle_global_display,
			.handle_autocomplete = autocomplete_global_display,
		}, {
			.label = UPDATE,
			.xt = XT_ANY,
			.child_builder = build_global_update_children,
		},
		{ 0 },
};

static struct cmd_option eamt_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_SIIT,
			.handler = handle_eamt_display,
			.handle_autocomplete = autocomplete_eamt_display,
		}, {
			.label = ADD,
			.xt = XT_SIIT,
			.handler = handle_eamt_add,
			.handle_autocomplete = autocomplete_eamt_add,
		}, {
			.label = REMOVE,
			.xt = XT_SIIT,
			.handler = handle_eamt_remove,
			.handle_autocomplete = autocomplete_eamt_remove,
		}, {
			.label = FLUSH,
			.xt = XT_SIIT,
			.handler = handle_eamt_flush,
			.handle_autocomplete = autocomplete_eamt_flush,
		},
		{ 0 },
};

static struct cmd_option blacklist4_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_SIIT,
			.handler = handle_blacklist4_display,
			.handle_autocomplete = autocomplete_denylist4_display,
		}, {
			.label = ADD,
			.xt = XT_SIIT,
			.handler = handle_blacklist4_add,
			.handle_autocomplete = autocomplete_denylist4_add,
		}, {
			.label = REMOVE,
			.xt = XT_SIIT,
			.handler = handle_blacklist4_remove,
			.handle_autocomplete = autocomplete_denylist4_remove,
		}, {
			.label = FLUSH,
			.xt = XT_SIIT,
			.handler = handle_blacklist4_flush,
			.handle_autocomplete = autocomplete_denylist4_flush,
		},
		{ 0 },
};

static struct cmd_option denylist4_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_SIIT,
			.handler = handle_denylist4_display,
			.handle_autocomplete = autocomplete_denylist4_display,
		}, {
			.label = ADD,
			.xt = XT_SIIT,
			.handler = handle_denylist4_add,
			.handle_autocomplete = autocomplete_denylist4_add,
		}, {
			.label = REMOVE,
			.xt = XT_SIIT,
			.handler = handle_denylist4_remove,
			.handle_autocomplete = autocomplete_denylist4_remove,
		}, {
			.label = FLUSH,
			.xt = XT_SIIT,
			.handler = handle_denylist4_flush,
			.handle_autocomplete = autocomplete_denylist4_flush,
		},
		{ 0 },
};

struct cmd_option pool4_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_NAT64,
			.handler = handle_pool4_display,
			.handle_autocomplete = autocomplete_pool4_display,
		}, {
			.label = ADD,
			.xt = XT_NAT64,
			.handler = handle_pool4_add,
			.handle_autocomplete = autocomplete_pool4_add,
		}, {
			.label = REMOVE,
			.xt = XT_NAT64,
			.handler = handle_pool4_remove,
			.handle_autocomplete = autocomplete_pool4_remove,
		}, {
			.label = FLUSH,
			.xt = XT_NAT64,
			.handler = handle_pool4_flush,
			.handle_autocomplete = autocomplete_pool4_flush,
		},
		{ 0 },
};

static struct cmd_option bib_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_NAT64,
			.handler = handle_bib_display,
			.handle_autocomplete = autocomplete_bib_display,
		}, {
			.label = ADD,
			.xt = XT_NAT64,
			.handler = handle_bib_add,
			.handle_autocomplete = autocomplete_bib_add,
		}, {
			.label = REMOVE,
			.xt = XT_NAT64,
			.handler = handle_bib_remove,
			.handle_autocomplete = autocomplete_bib_remove,
		},
		{ 0 },
};

static struct cmd_option session_ops[] = {
		{
			.label = DISPLAY,
			.xt = XT_NAT64,
			.handler = handle_session_display,
			.handle_autocomplete = autocomplete_session_display,
		},
		{ 0 },
};

static struct cmd_option file_ops[] = {
		{
			.label = "handle",
			.xt = XT_ANY,
			.handler = handle_file_update,
			.handle_autocomplete = autocomplete_file_update,
		},
		{ 0 },
};

static struct cmd_option joold_ops[] = {
		{
			.label = "advertise",
			.xt = XT_NAT64,
			.handler = handle_joold_advertise,
			.handle_autocomplete = autocomplete_joold_advertise,
		},
		{ 0 },
};

struct cmd_option tree[] = {
		{
			.label = "instance",
			.xt = XT_ANY,
			.children = instance_ops,
		}, {
			.label = "stats",
			.xt = XT_ANY,
			.children = stats_ops,
		}, {
			.label = "address",
			.xt = XT_SIIT,
			.children = address_ops,
		}, {
			.label = "global",
			.xt = XT_ANY,
			.children = global_ops,
		}, {
			.label = "eamt",
			.xt = XT_SIIT,
			.children = eamt_ops,
		}, {
			.label = "blacklist4", /* Deprecated */
			.xt = XT_SIIT,
			.children = blacklist4_ops,
		}, {
			.label = "denylist4",
			.xt = XT_SIIT,
			.children = denylist4_ops,
		}, {
			.label = "pool4",
			.xt = XT_NAT64,
			.children = pool4_ops,
		}, {
			.label = "bib",
			.xt = XT_NAT64,
			.children = bib_ops,
		}, {
			.label = "session",
			.xt = XT_NAT64,
			.children = session_ops,
		}, {
			.label = "file",
			.xt = XT_ANY,
			.children = file_ops,
		}, {
			.label = "joold",
			.xt = XT_NAT64,
			.children = joold_ops,
		}, {
			/* See files jool.bash and jool_siit.bash. */
			.label = "autocomplete",
			.xt = XT_ANY,
			.hidden = true,
			.handler  = handle_autocomplete,
		},
		{ 0 },
};

static int init_cmd_option_array(struct cmd_option *layer)
{
	struct cmd_option *node;
	int error;

	if (!layer)
		return 0;

	for (node = layer; node->label; node++) {
		if (node->child_builder) {
			node->children = node->child_builder();
			if (!node->children)
				return -ENOMEM;
		}

		error = init_cmd_option_array(node->children);
		if (error)
			return error;
	}

	return 0;
}

static void teardown_cmd_option_array(struct cmd_option *layer)
{
	struct cmd_option *node;

	if (!layer)
		return;

	for (node = layer; node->label; node++) {
		teardown_cmd_option_array(node->children);
		if (node->child_builder)
			free(node->children);
	}
}

/**
 * Returns the nodes from the @options array whose label start with @prefix.
 * (They will be chained via result->next.)
 *
 * Special cases:
 * - If there is a node whose entire label is @prefix, it returns that one only.
 * - If a node is hidden, it will have to match perfectly.
 */
static struct cmd_option *find_matches(struct cmd_option *options, char *prefix)
{
	struct cmd_option *option;
	struct cmd_option *first = NULL;
	struct cmd_option *last = NULL;

	if (!options)
		return NULL;

	for (option = options; option->label; option++) {
		if (!(xt_get() & option->xt))
			continue;

		if (option->hidden) {
			if (strcmp(option->label, prefix) == 0)
				return option;
			continue;
		}

		if (memcmp(option->label, prefix, strlen(prefix)) == 0) {
			/*
			 * The labels never overlap like this so this isn't
			 * really useful right now.
			 * I'm including this only for the sake of correctness.
			 */
			if (strcmp(option->label, prefix) == 0)
				return option;

			if (first)
				last->next = option;
			else
				first = option;
			last = option;
			last->next = NULL;
		}
	}

	return first;
}

static int unexpected_token(struct cmd_option *nodes, char *token)
{
	fprintf(stderr, "Unexpected token: '%s'\n", token);
	fprintf(stderr, "Available options: ");
	for (; nodes->label; nodes++) {
		if (!cmdopt_is_hidden(nodes))
			fprintf(stderr, "%s ", nodes->label);
	}
	fprintf(stderr, "\n");
	return -EINVAL;
}

static int ambiguous_token(struct cmd_option *nodes, char *token)
{
	fprintf(stderr, "Ambiguous token: '%s'\n", token);
	fprintf(stderr, "Available options: ");
	for (; nodes; nodes = nodes->next) {
		if (!cmdopt_is_hidden(nodes))
			fprintf(stderr, "%s ", nodes->label);
	}
	fprintf(stderr, "\n");
	return -EINVAL;
}

static int more_args_expected(struct cmd_option *nodes)
{
	fprintf(stderr, "More arguments expected.\n");
	fprintf(stderr, "Possible follow-ups: ");
	for (; nodes->label; nodes++) {
		if (!cmdopt_is_hidden(nodes))
			fprintf(stderr, "%s ", nodes->label);
	}
	fprintf(stderr, "\n");
	return -EINVAL;
}

static int __handle(char *iname, int argc, char **argv)
{
	struct cmd_option *nodes = &tree[0];
	struct cmd_option *node = NULL;
	int i;

	if (argc == 0)
		return more_args_expected(nodes);

	for (i = 0; i < argc; i++) {
		node = find_matches(nodes, argv[i]);
		if (!node)
			return unexpected_token(nodes, argv[i]);
		if (node->next)
			return ambiguous_token(node, argv[i]);

		if (node->handler) {
			return node->handler(iname, argc - i, &argv[i],
					node->args);
		}

		nodes = node->children;
	}

	return more_args_expected(node->children);
}

static int handle(char *iname, int argc, char **argv)
{
	int error;

	error = init_cmd_option_array(tree);
	if (error)
		return error;

	error = __handle(iname, argc, argv);

	teardown_cmd_option_array(tree);
	return error;
}

/**
 * Never fails because there's no point yet.
 */
static int handle_autocomplete(char *junk, int argc, char **argv, void const *arg)
{
	struct cmd_option *node = &tree[0];
	long int depth;
	int i;

	/*
	 * Case 1: Suppose the user typed `jool global update m<tab>`:
	 * Bash will query `jool autocomplete 3 global update m`. (`3` being the
	 * index of the incomplete token in the original command.)
	 * At this point, argc = 5, argv = { "autocomplete", "3", "global",
	 * "update", "m" }.
	 *
	 * Case 2: Suppose the command is `jool global update <tab>`:
	 * Bash will query `jool autocomplete 3 global update`.
	 * argc = 4, argv = { "autocomplete", "3", "global", "update" }.
	 *
	 * Case 3: Suppose the command is `jool <tab>`:
	 * Bash will query `jool autocomplete 1`.
	 * argc = 2, argv = { "autocomplete", "1" }.
	 */

	if (argc < 2)
		return 0;

	errno = 0;
	depth = strtol(argv[1], NULL, 10) - 1;
	if (errno)
		return 0;
	argc -= 2;
	argv += 2;
	/*
	 * depth can be argc when the user is tabbing the next token,
	 * without having actually typed any characters.
	 * (Eg. `jool global update <tab>`)
	 */
	if (depth != (argc - 1) && depth != argc)
		return 0;

	/*
	 * At this point,
	 * Case 1: argc = 3, argv = { "global", "update", "m" }, depth = 2.
	 * Case 2: argc = 2, argv = { "global", "update" }, depth = 2.
	 * Case 3: argc = 0, argv = {}, depth = 0.
	 */

	/*
	 * Traverse the intermediate keywords.
	 * (Cases 1 & 2: "global" and "update", Case 3: None.)
	 */
	for (i = 0; i < depth; i++) {
		node = find_matches(node, argv[i]);
		if (!node)
			return 0; /* Prefix does not exist. */
		if (node->next)
			return 0; /* Ambiguous prefix. */

		if (!node->children) {
			node->handle_autocomplete(node->args);
			return 0;
		}

		node = node->children;
	}

	/* Finally print the candidates for the last token. */
	node = find_matches(node, (i < argc) ? argv[i] : "");
	for (; node; node = node->next)
		printf("%s ", node->label);

	return 0;
}

static const struct option OPTIONS[] = {
	{
		.name = "help",
		.has_arg = no_argument,
		.val = '?',
	}, {
		.name = "version",
		.has_arg = no_argument,
		.val = 'V',
	}, {
		.name = "usage",
		.has_arg = no_argument,
		.val = 1000,
	}, {
		.name = "instance",
		.has_arg = required_argument,
		.val = 'i',
	}, {
		.name = "file",
		.has_arg = required_argument,
		.val = 'f',
	},
	{ 0 },
};

static int show_usage(char *program_name)
{
	printf("%s (\n", program_name);
	printf("        [<ARGP1>] <MODE> <OPERATION> [<ARGP2>]\n");
	printf("        | [-h|--help]\n");
	printf("        | (-V|--version)\n");
	printf("        | --usage\n");
	printf(")\n");
	return 0;
}

static int show_help(char *program_name)
{
	struct cmd_option *mode;
	struct cmd_option *op;

	printf("Usage\n");
	printf("=====\n");
	show_usage(program_name);
	printf("\n");

	printf("<ARGP1>\n");
	printf("=======\n");
	printf("Either (--instance|-i) <INSTANCE> or (--file|-f) <FILE>.\n");
	printf("- <INSTANCE> is the instance name\n");
	printf("- <FILE> is a path to a JSON file that contains the instance name\n");
	printf("\n");

	printf("<MODE>s -> <OPERATION>s\n");
	printf("=======================\n");
	for (mode = tree; mode && mode->label; mode++) {
		if (cmdopt_is_hidden(mode))
			continue;

		printf("- %s -> ", mode->label);
		for (op = mode->children; op && op->label; op++) {
			if (!cmdopt_is_hidden(op))
				printf("%s ", op->label);
		}
		printf("\n");
	}
	printf("\n");

	printf("<ARGP2>\n");
	printf("======\n");
	printf("Depends on <MODE> and <OPERATION>. Normally, see respective --help for details.\n");
	printf("(Example: %s instance add --help)\n", program_name);
	printf("\n");

	printf("Report bugs to %s.", argp_program_bug_address);
	printf("\n");
	return 0;
}

static int show_version(void)
{
	printf(JOOL_VERSION_STR "\n");
	return 0;
}

int jool_main(int argc, char **argv)
{
	int opt;
	char *iname = NULL;
	struct jool_result result;

	if (argc == 1)
		return show_help(argv[0]);

	while ((opt = getopt_long(argc, argv, "+?Vi:f:", OPTIONS, NULL)) != -1) {
		switch (opt) {
		case '?':
			return show_help(argv[0]);
		case 'V':
			return show_version();
		case 1000:
			return show_usage(argv[0]);
		case 'i':
			iname = optarg;
			break;
		case 'f':
			result = joolnl_file_get_iname(optarg, &iname);
			if (result.error)
				return pr_result(&result);
			break;
		}
	}

	return handle(iname, argc - optind, argv + optind);
}
