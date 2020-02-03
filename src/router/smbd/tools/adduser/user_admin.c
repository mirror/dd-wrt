// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include <config_parser.h>
#include <ksmbdtools.h>

#include <md4_hash.h>
#include <user_admin.h>
#include <management/user.h>
#include <management/share.h>

#include <linux/ksmbd_server.h>

#define MAX_NT_PWD_LEN 129

static char *arg_account;
static char *arg_password;
static int conf_fd = -1;
static char wbuf[2 * MAX_NT_PWD_LEN + 2 * USMBD_REQ_MAX_ACCOUNT_NAME_SZ];

static int __opendb_file(char *pwddb)
{
	conf_fd = open(pwddb, O_WRONLY);
	if (conf_fd == -1) {
		pr_err("%s %s\n", strerr(errno), pwddb);
		return -EINVAL;
	}

	if (ftruncate(conf_fd, 0)) {
		pr_err("%s %s\n", strerr(errno), pwddb);
		close(conf_fd);
		return -EINVAL;
	}

	return 0;
}

static void term_toggle_echo(int on_off)
{
	struct termios term;

	tcgetattr(STDIN_FILENO, &term);

	if (on_off)
		term.c_lflag |= ECHO;
	else
		term.c_lflag &= ~ECHO;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

static char *__prompt_password_stdin(size_t *sz)
{
	char *pswd1 = calloc(1, MAX_NT_PWD_LEN + 1);
	char *pswd2 = calloc(1, MAX_NT_PWD_LEN + 1);
	size_t len = 0;
	int i;

	if (!pswd1 || !pswd2) {
		free(pswd1);
		free(pswd2);
		pr_err("Out of memory\n");
		return NULL;
	}

again:
	printf("New password:\n");
	term_toggle_echo(0);
	if (fgets(pswd1, MAX_NT_PWD_LEN, stdin) == NULL) {
		term_toggle_echo(1);
		pr_err("Fatal error: %s\n", strerr(errno));
		free(pswd1);
		free(pswd2);
		return NULL;
	}

	printf("Retype new password:\n");
	if (fgets(pswd2, MAX_NT_PWD_LEN, stdin) == NULL) {
		term_toggle_echo(1);
		pr_err("Fatal error: %s\n", strerr(errno));
		free(pswd1);
		free(pswd2);
		return NULL;
	}
	term_toggle_echo(1);

	len = strlen(pswd1);
	for (i = 0; i < len; i++)
		if (pswd1[i] == '\n')
			pswd1[i] = 0x00;

	len = strlen(pswd2);
	for (i = 0; i < len; i++)
		if (pswd2[i] == '\n')
			pswd2[i] = 0x00;

	if (memcmp(pswd1, pswd2, MAX_NT_PWD_LEN + 1)) {
		pr_err("Passwords don't match\n");
		goto again;
	}

	len = strlen(pswd1);
	if (len <= 1) {
		pr_err("No password was provided\n");
		goto again;
	}

	*sz = len;
	free(pswd2);
	return pswd1;
}

static char *prompt_password(size_t *sz)
{
	if (!arg_password)
		return __prompt_password_stdin(sz);

	*sz = strlen(arg_password);
	return arg_password;
}

static char *get_utf8_password(long *len)
{
	size_t raw_sz;
	char *pswd_raw, *pswd_converted;
	size_t bytes_read = 0;
	size_t bytes_written = 0;

	pswd_raw = prompt_password(&raw_sz);
	if (!pswd_raw)
		return NULL;

	pswd_converted = usmbd_gconvert(pswd_raw,
					raw_sz,
					USMBD_CHARSET_UTF16LE,
					USMBD_CHARSET_DEFAULT,
					&bytes_read,
					&bytes_written);
	if (!pswd_converted) {
		free(pswd_raw);
		return NULL;
	}

	*len = bytes_written;
	free(pswd_raw);
	return pswd_converted;
}

static void __sanity_check(char *pswd_hash, char *pswd_b64)
{
	size_t pass_sz;
	char *pass = base64_decode(pswd_b64, &pass_sz);

	if (!pass) {
		pr_err("Unable to decode NT hash\n");
		exit(EXIT_FAILURE);
	}

	if (memcmp(pass, pswd_hash, pass_sz)) {
		pr_err("NT hash encoding error\n");
		exit(EXIT_FAILURE);
	}
	free(pass);
}

static char *get_hashed_b64_password(void)
{
	struct md4_ctx mctx;
	long len;
	char *pswd_plain, *pswd_hash, *pswd_b64;

	pswd_plain = get_utf8_password(&len);
	if (!pswd_plain)
		return NULL;

	pswd_hash = calloc(1, sizeof(mctx.hash) + 1);
	if (!pswd_hash) {
		free(pswd_plain);
		pr_err("Out of memory\n");
		return NULL;
	}

	md4_init(&mctx);
	md4_update(&mctx, pswd_plain, len);
	md4_final(&mctx, pswd_hash);

	pswd_b64 = base64_encode(pswd_hash,
				 MD4_HASH_WORDS * sizeof(unsigned int));

	__sanity_check(pswd_hash, pswd_b64);
	free(pswd_plain);
	free(pswd_hash);
	return pswd_b64;
}

static void write_user(struct usmbd_user *user)
{
	char *data;
	int ret, nr = 0;
	size_t wsz;

	if (test_user_flag(user, USMBD_USER_FLAG_GUEST_ACCOUNT))
		return;

	wsz = snprintf(wbuf, sizeof(wbuf), "%s:%s\n", user->name,
			user->pass_b64);
	if (wsz > sizeof(wbuf)) {
		pr_err("Entry size is above the limit: %zu > %zu\n",
			wsz,
			sizeof(wbuf));
		exit(EXIT_FAILURE);
	}

	while (wsz && (ret = write(conf_fd, wbuf + nr, wsz)) != 0) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			pr_err("%s\n", strerr(errno));
			exit(EXIT_FAILURE);
		}

		nr += ret;
		wsz -= ret;
	}
}

static void write_user_cb(void *value, unsigned long long id, void *user_data)
{
	struct usmbd_user *user = (struct usmbd_user *)value;

	write_user(user);
}

static void write_remove_user_cb(void *value,
				 unsigned long long key,
				 void *user_data)
{
	struct usmbd_user *user = (struct usmbd_user *)value;

	if (!strcasecmp(user->name, arg_account)) {
		pr_info("User '%s' removed\n", user->name);
		return;
	}

	write_user_cb(value, key, user_data);
}

static void lookup_can_del_user(void *value,
				unsigned long long key,
				void *user_data)
{
	struct usmbd_share *share = (struct usmbd_share *)value;
	int ret = 0;
	int *abort_del_user = (int *)user_data;

	if (*abort_del_user)
		return;

	ret = shm_lookup_users_map(share,
				   USMBD_SHARE_ADMIN_USERS_MAP,
				   arg_account);
	if (ret == 0)
		goto conflict;

	ret = shm_lookup_users_map(share,
				   USMBD_SHARE_WRITE_LIST_MAP,
				   arg_account);
	if (ret == 0)
		goto conflict;

	ret = shm_lookup_users_map(share,
				   USMBD_SHARE_VALID_USERS_MAP,
				   arg_account);
	if (ret == 0)
		goto conflict;

	*abort_del_user = 0;
	return;

conflict:
	pr_err("Share %s requires user %s to exist\n",
		share->name, arg_account);
	*abort_del_user = 1;
}

int command_add_user(char *pwddb, char *account, char *password)
{
	struct usmbd_user *user;
	char *pswd;

	arg_account = account;
	arg_password = password;

	user = usm_lookup_user(arg_account);
	if (user) {
		put_usmbd_user(user);
		pr_err("Account `%s' already exists\n", arg_account);
		return -EEXIST;
	}

	pswd = get_hashed_b64_password();
	if (!pswd) {
		pr_err("Out of memory\n");
		return -EINVAL;
	}

	/* pswd is already g_strdup-ed */
	if (usm_add_new_user(arg_account, pswd)) {
		pr_err("Could not add new account\n");
		return -EINVAL;
	}

	pr_info("User '%s' added\n", arg_account);
	if (__opendb_file(pwddb))
		return -EINVAL;

	foreach_usmbd_user(write_user_cb, NULL);
	close(conf_fd);
	return 0;
}

int command_update_user(char *pwddb, char *account, char *password)
{
	struct usmbd_user *user;
	char *pswd;

	arg_password = password;
	arg_account = account;

	user = usm_lookup_user(arg_account);
	if (!user) {
		pr_err("Unknown account\n");
		return -EINVAL;
	}

	pswd = get_hashed_b64_password();
	if (!pswd) {
		pr_err("Out of memory\n");
		put_usmbd_user(user);
		return -EINVAL;
	}

	if (usm_update_user_password(user, pswd)) {
		pr_err("Out of memory\n");
		put_usmbd_user(user);
		return -ENOMEM;
	}

	pr_info("User '%s' updated\n", account);
	put_usmbd_user(user);
	free(pswd);

	if (__opendb_file(pwddb))
		return -EINVAL;

	foreach_usmbd_user(write_user_cb, NULL);
	close(conf_fd);
	return 0;
}

int command_del_user(char *pwddb, char *account)
{
	int abort_del_user = 0;

	arg_account = account;
	if (!cp_key_cmp(global_conf.guest_account, arg_account)) {
		pr_err("User %s is a global guest account. Abort deletion.\n",
				arg_account);
		return -EINVAL;
	}

	foreach_usmbd_share(lookup_can_del_user, &abort_del_user);

	if (abort_del_user) {
		pr_err("Aborting user deletion\n");
		return -EINVAL;
	}

	if (__opendb_file(pwddb))
		return -EINVAL;

	foreach_usmbd_user(write_remove_user_cb, NULL);
	close(conf_fd);
	return 0;
}
