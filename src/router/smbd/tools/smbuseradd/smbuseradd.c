// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
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

#include <config_parser.h>
#include <usmbdtools.h>

#include <management/user.h>
#include <management/share.h>
#include <user_admin.h>

#include <linux/usmbd_server.h>

static char *arg_account = NULL;
static char *arg_password = NULL;

enum {
	COMMAND_ADD_USER = 1,
	COMMAND_DEL_USER,
	COMMAND_UPDATE_USER,
};

static void usage(void)
{
	fprintf(stderr, "Usage: smbuseradd\n");

	fprintf(stderr, "\t-a | --add-user=login\n");
	fprintf(stderr, "\t-d | --del-user=login\n");
	fprintf(stderr, "\t-u | --update-user=login\n");
	fprintf(stderr, "\t-p | --password=pass\n");

	fprintf(stderr, "\t-i smbpwd.db | --import-users=smbpwd.db\n");
	fprintf(stderr, "\t-V | --version\n");
	fprintf(stderr, "\t-v | --verbose\n");

	exit(EXIT_FAILURE);
}

static void show_version(void)
{
	printf("ksmbd-tools version : %s\n", KSMBD_TOOLS_VERSION);
	exit(EXIT_FAILURE);
}

static int parse_configs(char *pwddb)
{
	int ret;

	ret = test_file_access(pwddb);
	if (ret)
		return ret;

	ret = cp_parse_pwddb(pwddb);
	if (ret)
		return ret;
	return 0;
}

static int sanity_check_user_name_simple(char *uname)
{
	int sz, i;

	if (!uname)
		return -EINVAL;

	sz = strlen(uname);
	if (sz < 1)
		return -EINVAL;
	if (sz >= USMBD_REQ_MAX_ACCOUNT_NAME_SZ)
		return -EINVAL;

	/* 1'; Drop table users -- */
	if (!strcmp(uname, "root"))
		return -EINVAL;

	for (i = 0; i < sz; i++) {
		if (isalnum(uname[i]))
			return 0;
	}
	return -EINVAL;
}

#ifdef MULTICALL
int smbuseradd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int ret = EXIT_FAILURE;
	char *pwddb = PATH_PWDDB;
	int c, cmd = 0;

	set_logger_app_name("smbuseradd");

	opterr = 0;
	while ((c = getopt(argc, argv, "c:i:a:d:u:p:Vvh")) != EOF)
		switch (c) {
		case 'a':
			arg_account = g_strdup(optarg);
			cmd = COMMAND_ADD_USER;
			break;
		case 'd':
			arg_account = g_strdup(optarg);
			cmd = COMMAND_DEL_USER;
			break;
		case 'u':
			arg_account = g_strdup(optarg);
			cmd = COMMAND_UPDATE_USER;
			break;
		case 'p':
			arg_password = g_strdup(optarg);
			break;
		case 'i':
			pwddb = g_strdup(optarg);
			break;
		case 'V':
			show_version();
			break;
		case 'v':
			break;
		case '?':
		case 'h':
		default:
			usage();
	}

	if (sanity_check_user_name_simple(arg_account)) {
		pr_err("User name sanity check failure\n");
		goto out;
	}

	if (!pwddb) {
		pr_err("Out of memory\n");
		goto out;
	}

	ret = usm_init();
	if (ret) {
		pr_err("Failed to init user management\n");
		goto out;
	}

	ret = shm_init();
	if (ret) {
		pr_err("Failed to init net share management\n");
		goto out;
	}

	ret = parse_configs(pwddb);
	if (ret) {
		pr_err("Unable to parse configuration files\n");
		goto out;
	}

	if (cmd == COMMAND_ADD_USER)
		ret = command_add_user(pwddb, arg_account, arg_password);
	if (cmd == COMMAND_DEL_USER)
		ret = command_del_user(pwddb, arg_account);
	if (cmd == COMMAND_UPDATE_USER)
		ret = command_update_user(pwddb, arg_account, arg_password);

	/*
	 * We support only ADD_USER command at this moment
	 */
	if (ret == 0 && cmd == COMMAND_ADD_USER)
		notify_usmbd_daemon();
out:
	shm_destroy();
	usm_destroy();
	return ret;
}
