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
#include <termios.h>

#include <config_parser.h>
#include <tools.h>

#include <md4_hash.h>
#include <user_admin.h>
#include <management/user.h>
#include <management/share.h>

#include <linux/ksmbd_server.h>

static void __prompt_password_stdin(char *password, size_t *sz)
{
	struct termios term, raw_term;
	char buf[LINE_MAX];
	size_t buflen;

	pr_info("Prompting for password\n");

	tcgetattr(STDIN_FILENO, &term);
	raw_term = term;

	raw_term.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term);

	for (*sz = (size_t)-1,
	     buflen = 0, password[buflen] = buf[buflen] = 0x00;;) {
		int c;

		if (!buflen) {
			printf("\r" "\e[2K" "%s password: ",
			       *password == 0x00 ? "New" : "Retype");
			fflush(stdout);
		}

		c = getchar();
		if (c == EOF || c == 0x04)
			break;

		if (c == '\n') {
			if (*password == 0x00 && buflen) {
				strncat(password, buf, MAX_NT_PWD_LEN - 1);
				buflen = 0;
				buf[buflen] = 0x00;
				printf("\eD");
				continue;
			}
			if (strcmp(password, buf)) {
				*password = 0x00;
				buflen = 0;
				buf[buflen] = 0x00;
				printf("\r" "\e[2K"
				       "\e[1m" "Passwords do not match!" "\e[m"
				       "\eM");
				continue;
			}
			if (!g_utf8_validate(password, -1, NULL)) {
				*password = 0x00;
				buflen = 0;
				buf[buflen] = 0x00;
				printf("\r" "\e[2K"
				       "\e[1m" "Password is not UTF-8!" "\e[m"
				       "\eM");
				continue;
			}
			*sz = buflen + 1;
			break;
		}

		if (c == raw_term.c_cc[VERASE]) {
			char *u8c;
			size_t clen;

			u8c = g_utf8_find_prev_char(buf, buf + buflen);
			clen = u8c ? buf + buflen - u8c : 1;
			buflen -= buflen > clen ? clen : buflen;
			buf[buflen] = 0x00;
			continue;
		}

		if (buflen < LINE_MAX - 1) {
			buf[buflen++] = c;
			buf[buflen] = 0x00;
			if (!cp_printable(buf + buflen - 1))
				buf[--buflen] = 0x00;
		}
	}

	printf("\eD" "\r" "\e[2K");

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

static int __is_valid_password_len(size_t len)
{
	int is_valid;

	is_valid = len < MAX_NT_PWD_LEN;
	if (!is_valid) {
		pr_err("Password exceeds %d bytes\n",
		       MAX_NT_PWD_LEN - 1);
		goto out;
	}
	if (!len)
		pr_info("Password is empty\n");
out:
	return is_valid;
}

static void __utf16le_convert(char **password, size_t *sz)
{
	size_t bytes_written;
	char *utf16le;

	utf16le = ksmbd_gconvert(*password,
				 *sz - 1,
				 KSMBD_CHARSET_UTF16LE,
				 KSMBD_CHARSET_DEFAULT,
				 NULL,
				 &bytes_written);
	g_free(*password);

	*sz = !utf16le ? (size_t)-1 : bytes_written;
	*password = utf16le;
}

static void __md4_hash(char **password, size_t *sz)
{
	struct md4_ctx mctx;

	md4_init(&mctx);
	md4_update(&mctx, *password, *sz);
	g_free(*password);

	*sz = sizeof(mctx.hash) + 1;
	*password = g_malloc0(*sz);
	md4_final(&mctx, *password);
}

static void __base64_encode(char **password, size_t *sz)
{
	char *base64;

	base64 = base64_encode(*password, *sz - 1);
	g_free(*password);

	*sz = strlen(base64) + 1;
	*password = base64;
}

static int process_password(char **password)
{
	size_t sz;

	if (!*password) {
		*password = g_malloc(MAX_NT_PWD_LEN);
		__prompt_password_stdin(*password, &sz);
		if (sz == (size_t)-1)
			return -EINVAL;
	} else {
		sz = strlen(*password) + 1;
	}

	if (!__is_valid_password_len(sz - 1))
		return -EINVAL;

	__utf16le_convert(password, &sz);
	if (sz == (size_t)-1)
		return -EINVAL;

	__md4_hash(password, &sz);
	__base64_encode(password, &sz);
	return 0;
}

static void __new_user_nl_cb(struct ksmbd_user *user, GList **nl)
{
	if (!test_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT))
		*nl = g_list_insert_sorted(*nl,
					   user->name,
					   (GCompareFunc)strcmp);
}

static GList *new_user_nl(void)
{
	GList *nl = NULL;

	usm_iter_users((user_cb)__new_user_nl_cb, &nl);
	return nl;
}

static char *get_conf_contents(void)
{
	GPtrArray *lines = g_ptr_array_new();
	g_autoptr(GList) nl = new_user_nl();
	GList *l;

	for (l = nl; l; l = l->next) {
		struct ksmbd_user *user = usm_lookup_user(l->data);

		gptrarray_printf(lines, "%s:%s\n", user->name, user->pass_b64);
		put_ksmbd_user(user);
	}
	return gptrarray_to_str(lines);
}

int command_add_user(char *pwddb, char *name, char *password)
{
	g_autofree char *contents = NULL;
	struct ksmbd_user *user;
	int ret;

	user = usm_lookup_user(name);
	if (user) {
		put_ksmbd_user(user);
		pr_err("User `%s' already exists\n", name);
		ret = -EEXIST;
		goto out;
	}

	ret = process_password(&password);
	if (ret)
		goto out;

	ret = usm_add_new_user(g_strdup(name), g_strdup(password));
	if (ret) {
		pr_err("Failed to add user `%s'\n", name);
		goto out;
	}

	contents = get_conf_contents();
	ret = set_conf_contents(pwddb, contents);
	if (ret)
		goto out;

	pr_info("Added user `%s'\n", name);
out:
	g_free(pwddb);
	g_free(name);
	g_free(password);
	return ret;
}

int command_update_user(char *pwddb, char *name, char *password)
{
	g_autofree char *contents = NULL;
	struct ksmbd_user *user;
	int ret;

	user = usm_lookup_user(name);
	if (!user) {
		pr_err("User `%s' does not exist\n", name);
		ret = -EINVAL;
		goto out;
	}

	ret = process_password(&password);
	if (ret)
		goto out;

	usm_update_user_password(user, password);
	put_ksmbd_user(user);

	contents = get_conf_contents();
	ret = set_conf_contents(pwddb, contents);
	if (ret)
		goto out;

	pr_info("Updated user `%s'\n", name);
out:
	g_free(pwddb);
	g_free(name);
	g_free(password);
	return ret;
}

static void __share_transient_user_cb(struct ksmbd_share *share,
				      char **user_name)
{
	if (!*user_name)
		return;

	if (share->guest_account && !strcmp(*user_name, share->guest_account))
		goto require;

	if (!shm_lookup_users_map(share,
				  KSMBD_SHARE_ADMIN_USERS_MAP,
				  *user_name))
		goto require;

	if (!shm_lookup_users_map(share,
				  KSMBD_SHARE_WRITE_LIST_MAP,
				  *user_name))
		goto require;

	if (!shm_lookup_users_map(share,
				  KSMBD_SHARE_VALID_USERS_MAP,
				  *user_name))
		goto require;

	return;
require:
	pr_err("Share `%s' requires user `%s'\n", share->name, *user_name);
	*user_name = NULL;
}

static int __is_transient_user(char *name)
{
	int is_transient;

	is_transient = !global_conf.guest_account ||
		       strcmp(global_conf.guest_account, name);
	if (!is_transient) {
		pr_err("Server requires user `%s'\n", name);
		goto out;
	}
	shm_iter_shares((share_cb)__share_transient_user_cb, &name);
	is_transient = !!name;
out:
	return is_transient;
}

int command_delete_user(char *pwddb, char *name, char *password)
{
	g_autofree char *contents = NULL;
	struct ksmbd_user *user;
	int ret;

	user = usm_lookup_user(name);
	if (!user) {
		pr_err("User `%s' does not exist\n", name);
		ret = -EINVAL;
		goto out;
	}

	if (!__is_transient_user(name)) {
		ret = -EINVAL;
		goto out;
	}

	usm_remove_user(user);

	contents = get_conf_contents();
	ret = set_conf_contents(pwddb, contents);
	if (ret)
		goto out;

	pr_info("Deleted user `%s'\n", name);
out:
	g_free(pwddb);
	g_free(name);
	g_free(password);
	return ret;
}
