// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */


#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "main.h"
#include "sys.h"
#include "debug.h"
#include "functions.h"
#include "netlink.h"

char mesh_dfl_iface[] = "bat0";
char module_ver_path[] = "/sys/module/batman_adv/version";

extern const struct command *__start___command[];
extern const struct command *__stop___command[];

static void print_usage(void)
{
	struct {
		const char *label;
		uint32_t types;
	} type[] = {
		{
			.label = "commands:\n",
			.types = BIT(SUBCOMMAND) |
				 BIT(SUBCOMMAND_MIF) |
				 BIT(SUBCOMMAND_VID) |
				 BIT(SUBCOMMAND_HIF),
		},
		{
			.label = "debug tables:                                   \tdisplay the corresponding debug table\n",
			.types = BIT(DEBUGTABLE),
		},
		{
			.label = "JSON queries:                                   \tdisplay results of netlink query as JSON\n",
			.types = BIT(JSON_MIF) |
				 BIT(JSON_VID) |
				 BIT(JSON_HIF),
		},
	};
	const char *default_prefixes[] = {
		"",
		NULL,
	};
	const char *meshif_prefixes[] = {
		"meshif <netdev> ",
		NULL,
	};
	const char *vlan_prefixes[] = {
		"vlan <vdev> ",
		"meshif <netdev> vid <vid> ",
		NULL,
	};
	const char *hardif_prefixes[] = {
		"hardif <netdev> ",
		NULL,
	};
	const struct command **p;
	const char **prefixes;
	const char **prefix;
	char buf[64];
	size_t i;

	fprintf(stderr, "Usage: batctl [options] command|debug table|debug json [parameters]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, " \t-h print this help (or 'batctl <command|debug table|debug json> -h' for the parameter help)\n");
	fprintf(stderr, " \t-v print version\n");

	for (i = 0; i < sizeof(type) / sizeof(*type); i++) {
		fprintf(stderr, "\n");

		fprintf(stderr, "%s", type[i].label);

		for (p = __start___command; p < __stop___command; p++) {
			const struct command *cmd = *p;

			if (!(BIT(cmd->type) & type[i].types))
				continue;

			if (!cmd->usage)
				continue;

			switch (cmd->type) {
			case DEBUGTABLE:
			case JSON_MIF:
			case SUBCOMMAND_MIF:
				prefixes = meshif_prefixes;
				break;
			case JSON_VID:
			case SUBCOMMAND_VID:
				prefixes = vlan_prefixes;
				break;
			case JSON_HIF:
			case SUBCOMMAND_HIF:
				prefixes = hardif_prefixes;
				break;
			default:
				prefixes = default_prefixes;
				break;
			}

			for (prefix = &prefixes[0]; *prefix; prefix++) {
				if (strcmp(cmd->name, cmd->abbr) == 0)
					snprintf(buf, sizeof(buf), "%s%s",
						 *prefix, cmd->name);
				else
					snprintf(buf, sizeof(buf), "%s%s|%s",
						 *prefix, cmd->name, cmd->abbr);

				fprintf(stderr, " \t%-43s%s\n", buf,
					cmd->usage);
			}
		}
	}
}

static void version(void)
{
	int ret;

	printf("batctl %s [batman-adv: ", SOURCE_VERSION);

	ret = read_file(module_ver_path, USE_READ_BUFF | SILENCE_ERRORS);
	if ((line_ptr) && (line_ptr[strlen(line_ptr) - 1] == '\n'))
		line_ptr[strlen(line_ptr) - 1] = '\0';

	if (ret == EXIT_SUCCESS)
		printf("%s]\n", line_ptr);
	else
		printf("module not loaded]\n");

	free(line_ptr);
	exit(EXIT_SUCCESS);
}

static const struct command *find_command_by_types(uint32_t types,
						   const char *name)
{
	const struct command **p;

	for (p = __start___command; p < __stop___command; p++) {
		const struct command *cmd = *p;

		if (!(BIT(cmd->type) & types))
			continue;

		if (strcmp(cmd->name, name) == 0)
			return cmd;

		if (strcmp(cmd->abbr, name) == 0)
			return cmd;
	}

	return NULL;
}

static const struct command *find_command(struct state *state, const char *name)
{
	uint32_t types = 0;

	switch (state->selector) {
	case SP_NONE_OR_MESHIF:
		types = BIT(SUBCOMMAND);
		/* fall through */
	case SP_MESHIF:
		types |= BIT(SUBCOMMAND_MIF) |
			 BIT(DEBUGTABLE)     |
			 BIT(JSON_MIF);
		break;
	case SP_VLAN:
		types = BIT(JSON_VID) |
			BIT(SUBCOMMAND_VID);
		break;
	case SP_HARDIF:
		types = BIT(JSON_HIF) |
			BIT(SUBCOMMAND_HIF);
		break;
	default:
		return NULL;
	}

	return find_command_by_types(types, name);
}

static int detect_selector_prefix(int argc, char *argv[],
				  enum selector_prefix *selector)
{
	/* not enough remaining arguments to detect anything */
	if (argc < 2)
		return -EINVAL;

	/* only detect selector prefix which identifies meshif */
	if (strcmp(argv[0], "meshif") == 0) {
		*selector = SP_MESHIF;
		return 2;
	} else if (strcmp(argv[0], "vlan") == 0) {
		*selector = SP_VLAN;
		return 2;
	} else if (strcmp(argv[0], "hardif") == 0) {
		*selector = SP_HARDIF;
		return 2;
	}

	return 0;
}

static int guess_selector_prefix(int argc, char *argv[],
				 enum selector_prefix *selector)
{
	int ret;

	/* check if there is a direct hit with full prefix */
	ret = detect_selector_prefix(argc, argv, selector);
	if (ret > 0)
		return ret;

	/* not enough remaining arguments to detect anything */
	if (argc < 1)
		return -EINVAL;

	/* don't try to parse subcommand names as network interface */
	if (find_command_by_types(0xffffffff, argv[0]))
		return -EEXIST;

	/* check if it is a netdev - and if it exists, try to guess what kind */
	ret = guess_netdev_type(argv[0], selector);
	if (ret < 0)
		return ret;

	return 1;
}

static int parse_meshif_args(struct state *state, int argc, char *argv[])
{
	enum selector_prefix selector;
	int parsed_args;
	char *dev_arg;
	int ret;

	parsed_args = guess_selector_prefix(argc, argv, &selector);
	if (parsed_args < 1)
		goto fallback_meshif_vlan;

	dev_arg = argv[parsed_args - 1];

	switch (selector) {
	case SP_MESHIF:
		snprintf(state->mesh_iface, sizeof(state->mesh_iface), "%s",
			 dev_arg);
		state->selector = SP_MESHIF;
		return parsed_args;
	case SP_VLAN:
		ret = translate_vlan_iface(state, dev_arg);
		if (ret < 0) {
			fprintf(stderr, "Error - invalid vlan device %s: %s\n",
				dev_arg, strerror(-ret));
			return ret;
		}
		return parsed_args;
	case SP_HARDIF:
		ret = translate_hard_iface(state, dev_arg);
		if (ret < 0) {
			fprintf(stderr, "Error - invalid hardif %s: %s\n",
				dev_arg, strerror(-ret));
			return ret;
		}

		snprintf(state->hard_iface, sizeof(state->hard_iface), "%s",
			 dev_arg);
		return parsed_args;
	case SP_NONE_OR_MESHIF:
		/* not allowed - see detect_selector_prefix */
		break;
	}

fallback_meshif_vlan:
	/* parse vlan as part of -m parameter or mesh_dfl_iface */
	translate_mesh_iface_vlan(state, state->arg_iface);
	return 0;
}

static int parse_dev_args(struct state *state, int argc, char *argv[])
{
	int dev_arguments;
	int ret;

	/* try to parse selector prefix which can be used to identify meshif */
	dev_arguments = parse_meshif_args(state, argc, argv);
	if (dev_arguments < 0)
		return dev_arguments;

	/* try to parse secondary prefix selectors which cannot be used to
	 * identify the meshif
	 */
	argv += dev_arguments;
	argc -= dev_arguments;

	switch (state->selector) {
	case SP_NONE_OR_MESHIF:
	case SP_MESHIF:
		/* continue below */
		break;
	default:
		return dev_arguments;
	}

	/* enough room for additional selectors? */
	if (argc < 2)
		return dev_arguments;

	if (strcmp(argv[0], "vid") == 0) {
		ret = translate_vid(state, argv[1]);
		if (ret < 0)
			return ret;

		return dev_arguments + 2;
	}

	return dev_arguments;
}

int main(int argc, char **argv)
{
	const struct command *cmd;
	struct state state = {
		.arg_iface = mesh_dfl_iface,
		.selector = SP_NONE_OR_MESHIF,
		.cmd = NULL,
	};
	int dev_arguments;
	int opt;
	int ret;

	while ((opt = getopt(argc, argv, "+hm:v")) != -1) {
		switch (opt) {
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 'm':
			if (state.arg_iface != mesh_dfl_iface) {
				fprintf(stderr,
					"Error - multiple mesh interfaces specified\n");
				goto err;
			}
			fprintf(stderr, "Warning - option -m was deprecated and will be removed in the future\n");

			state.arg_iface = argv[2];
			break;
		case 'v':
			version();
			break;
		default:
			goto err;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "Error - no command specified\n");
		goto err;
	}

	argv += optind;
	argc -= optind;
	optind = 0;

	/* parse arguments to identify vlan, ... */
	dev_arguments = parse_dev_args(&state, argc, argv);
	if (dev_arguments < 0)
		goto err;

	argv += dev_arguments;
	argc -= dev_arguments;

	if (argc == 0) {
		fprintf(stderr, "Error - no command specified\n");
		goto err;
	}

	cmd = find_command(&state, argv[0]);
	if (!cmd) {
		fprintf(stderr,
			"Error - no valid command or debug table/JSON specified: %s\n",
			argv[0]);
		goto err;
	}

	state.cmd = cmd;

	if (cmd->flags & COMMAND_FLAG_MESH_IFACE &&
	    check_mesh_iface(&state) < 0) {
		fprintf(stderr,
			"Error - interface %s is not present or not a batman-adv interface\n",
			state.mesh_iface);
		exit(EXIT_FAILURE);
	}

	if (cmd->flags & COMMAND_FLAG_NETLINK) {
		ret = netlink_create(&state);
		if (ret < 0) {
			fprintf(stderr,
				"Error - failed to connect to batadv\n");
			exit(EXIT_FAILURE);
		}
	}

	ret = cmd->handler(&state, argc, argv);

	if (cmd->flags & COMMAND_FLAG_NETLINK)
		netlink_destroy(&state);

	return ret;

err:
	print_usage();
	exit(EXIT_FAILURE);
}
