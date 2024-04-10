// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#include "config_parser.h"
#include "tools.h"
#include "management/share.h"
#include "management/user.h"
#include "linux/ksmbd_server.h"
#include "share_admin.h"

static void usage(int status)
{
	printf(
		"Usage: ksmbd.addshare [-v] [-C CONF] [-P PWDDB] [-a | -u | -d] [-o OPT]... SHARE\n");

	if (status != EXIT_SUCCESS)
		printf("Try `ksmbd.addshare --help' for more information.\n");
	else
		printf(
			"\n"
			"If neither `-a', `-u', nor `-d' is given, either add or update SHARE.\n"
			"SHARE must be UTF-8 and [1, " STR(KSMBD_REQ_MAX_SHARE_NAME) ") bytes.\n"
			"SHARE is case-insensitive.\n"
			"\n"
			"  -a, --add            add SHARE to configuration file\n"
			"  -u, --update         update SHARE in configuration file\n"
			"  -d, --delete         delete SHARE from configuration file\n"
			"  -o, --option=OPT     use OPT as share parameter instead of prompting;\n"
			"                       this option can be given multiple times\n"
			"  -C, --config=CONF    use CONF as configuration file instead of\n"
			"                       `" PATH_SMBCONF "'\n"
			"  -i, --pwddb=PWDDB    use PWDDB as user database instead of\n"
			"                       `" PATH_PWDDB "'\n"
			"  -v, --verbose        be verbose\n"
			"  -V, --version        output version information and exit\n"
			"  -h, --help           display this help and exit\n"
			"\n"
			"See ksmbd.addshare(8) for more details.\n");
}

static const struct option opts[] = {
	{"add",			no_argument,		NULL,	'a' },
	{"update",		no_argument,		NULL,	'u' },
	{"delete",		no_argument,		NULL,	'd' },
	{"option",		required_argument,	NULL,	'o' },
	{"config",		required_argument,	NULL,	'C' },
	{"pwddb",		required_argument,	NULL,	'P' },
	{"verbose",		no_argument,		NULL,	'v' },
	{"version",		no_argument,		NULL,	'V' },
	{"help",		no_argument,		NULL,	'h' },
	{NULL,			0,			NULL,	 0  }
};

int addshare_main(int argc, char **argv)
{
	int ret = -EINVAL;
	g_autofree char *smbconf = NULL, *name = NULL, *pwddb = NULL;
	g_auto(GStrv) options = NULL;
	g_autoptr(GPtrArray) __options =
		g_ptr_array_new_with_free_func(g_free);
	command_fn *command = NULL;
	int c;

	while ((c = getopt_long(argc, argv, "audo:C:i:vVh", opts, NULL)) != EOF)
		switch (c) {
		case 'a':
			command = command_add_share;
			break;
		case 'u':
			command = command_update_share;
			break;
		case 'd':
			command = command_delete_share;
			break;
		case 'o':
			gptrarray_printf(__options, "%s", optarg);
			break;
		case 'C':
			g_free(smbconf);
			smbconf = g_strdup(optarg);
			break;
		case 'i':
			g_free(pwddb);
			pwddb = g_strdup(optarg);
			break;
		case 'v':
			set_log_level(PR_DEBUG);
			break;
		case 'V':
			ret = show_version();
			goto out;
		case 'h':
			ret = 0;
			/* Fall through */
		case '?':
		default:
			usage(ret ? EXIT_FAILURE : EXIT_SUCCESS);
			goto out;
		}

	options = gptrarray_to_strv(__options);
	__options = NULL;

	if (optind == argc - 1) {
		name = g_strdup(argv[optind]);
	} else {
		usage(ret ? EXIT_FAILURE : EXIT_SUCCESS);
		goto out;
	}

	if (!shm_share_name(name, strchr(name, 0x00))) {
		pr_err("Invalid share name `%s'\n", name);
		goto out;
	}

	if (!pwddb)
		pwddb = g_strdup(PATH_PWDDB);
	if (!smbconf)
		smbconf = g_strdup(PATH_SMBCONF);

	ret = load_config(pwddb, smbconf);
	if (ret)
		goto out;

	if (!command) {
		if (g_hash_table_lookup(parser.groups, name))
			command = command_update_share;
		else
			command = command_add_share;
	}

	ret = command(smbconf, name, options);
	smbconf = name = (char *)(options = NULL);
	if (ret)
		goto out;

	if (cp_parse_lock()) {
		pr_info("Ignored lock file\n");
		goto out;
	}

	if (kill(global_conf.pid, SIGHUP) < 0) {
		pr_debug("Can't send SIGHUP to PID %d: %m\n",
			 global_conf.pid);
		goto out;
	}

	pr_info("Notified mountd\n");
out:
	remove_config();
	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
