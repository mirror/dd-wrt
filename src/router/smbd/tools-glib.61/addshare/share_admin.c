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
#include <termios.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <config_parser.h>
#include <tools.h>

#include <management/share.h>
#include <management/user.h>

#include <linux/ksmbd_server.h>
#include <share_admin.h>

/*
 * WARNING:
 *
 * This must match KSMBD_SHARE_CONF enum 1:1.
 * Each description should be well under 80 characters.
 */
static const char *__defconf_fmt[KSMBD_SHARE_CONF_MAX] = {
/*0*/	"; net view and browse list description [%s]",
	"; directory given access [%s]",
	"; allow passwordless connections with guest accounts [%s]",
	"; name of additional guest account [%s]",
	"; users have read only access [%s]",
/*5*/	"; visible in net view and browse list [%s]",
	"; users have read-write access [%s]",
	"; users have read-write access [%s]",
	"; support dos file attributes with xattr [%s]",
	"; issue oplocks for file open requests [%s]",
/*10*/	"; octal bitmask and'd with mapped file perms [%s]",
	"; octal bitmask and'd with mapped dir perms [%s]",
	"; octal bitmask or'd with final file perms [%s]",
	"; octal bitmask or'd with final dir perms [%s]",
	"; system group to map all users [%s]",
/*15*/	"; system user to map all users [%s]",
	"; files beginning with dot are hidden [%s]",
	"; list of the only users allowed to connect [%s]",
	"; list of users disallowed to connect [%s]",
	"; list of users restricted to read only access [%s]",
/*20*/	"; list of users granted read-write access [%s]",
	"; list of users mapped to the superuser [%s]",
	"; list of the only hosts allowed to connect [%s]",
	"; list of hosts disallowed to connect [%s]",
	"; maximum number of simultaneous connections [%s]",
/*25*/	"; files and dirs made inaccessible, e.g. /file1/dir1/ [%s]",
	"; files and dirs inherit ownership from parent dir [%s]",
	"; follow symlinks in the path directory [%s]",
	"; list of vfs objects to overload io ops with [%s]",
	"; users have read-write access [%s]",
/*30*/	"; path lookup can cross mountpoints [%s]",
};

static char **__get_options(GHashTable *kv, int is_global)
{
	GPtrArray *options = g_ptr_array_new();
	enum KSMBD_SHARE_CONF c;

	for (c = 0; c < KSMBD_SHARE_CONF_MAX; c++) {
		const char *k = KSMBD_SHARE_CONF[c], *v = NULL, *pre;

		if ((is_global && KSMBD_SHARE_CONF_IS_GLOBAL(c)) ||
		    KSMBD_SHARE_CONF_IS_BROKEN(c))
			pre = "; ";
		else
			pre = "";

		if (kv)
			v = g_hash_table_lookup(kv, k);
		if (!v)
			v = "";

		gptrarray_printf(options, "%s%s = %s", pre, k, v);
	}

	return gptrarray_to_strv(options);
}

static void __load_conf(enum KSMBD_SHARE_CONF conf,
			int step,
			char **options,
			char *buf,
			size_t *buflen)
{
	char *option = options[conf];

	if (step) {
		if (cp_smbconf_eol(cp_ltrim(buf))) {
			char *p = cp_ltrim(strchr(option, '=') + 1);
			const char *fmt, *s;

			if (*p == 0x00) {
				fmt = __defconf_fmt[conf];
				s = KSMBD_SHARE_DEFCONF[conf];
			} else {
				fmt = "%s";
				s = p;
			}
			snprintf(buf, LINE_MAX, fmt, s);
			*buflen = strlen(buf);
			*p = 0x00;
		} else {
			options[conf] = g_strdup_printf("%s%s", option, buf);
		}
	}

	printf("\r" "\e[2K" "%s%s" "\e[6n", option, buf);

	if (option != options[conf])
		g_free(option);
}

static enum KSMBD_SHARE_CONF __next_conf(enum KSMBD_SHARE_CONF conf,
					 int step,
					 int is_global,
					 int *is_ready)
{
	if (conf > KSMBD_SHARE_CONF_MAX)
		return 0;

	if (conf == KSMBD_SHARE_CONF_MAX) {
		if (*is_ready)
			return conf;
		*is_ready = !*is_ready;
		printf("\eD" "\r" "\e[1m" "Ready to commit changes!" "\e[m");
		return __next_conf(conf - 1, -1, is_global, is_ready);
	}

	if ((is_global && KSMBD_SHARE_CONF_IS_GLOBAL(conf)) ||
	    KSMBD_SHARE_CONF_IS_BROKEN(conf))
		return __next_conf(conf + step, step, is_global, is_ready);

	if (step > 0)
		printf("\eD");
	else if (step < 0)
		printf("\eM");

	return conf;
}

static GList *clear_ml(GList *ml)
{
	g_list_free_full(ml, g_free);
	return NULL;
}

static GList *next_ml(GList *ml)
{
	GList *first = ml;

	ml = g_list_remove_link(ml, first);
	return g_list_concat(ml, first);
}

static GList *init_ml(GList *ml)
{
	g_free(ml->data);
	return g_list_delete_link(ml, ml);
}

static void __new_user_ml_cb(struct ksmbd_user *user, GList **ml)
{
	if (!strncmp(user->name, (*ml)->data, strlen((*ml)->data)))
		*ml = g_list_insert_sorted(*ml,
					   g_strdup(user->name),
					   (GCompareFunc)strcmp);
}

static GList *new_user_ml(GList *ml, char *p)
{
	if (ml)
		return next_ml(ml);

	ml = g_list_append(ml, g_strdup(p));
	usm_iter_users((user_cb)__new_user_ml_cb, &ml);
	return init_ml(ml);
}

static GList *new_system_group_ml(GList *ml, char *p)
{
	struct group *e;

	if (ml)
		return next_ml(ml);

	ml = g_list_append(ml, g_strdup(p));
	setgrent();
	while ((e = getgrent()))
		if (!strncmp(e->gr_name, ml->data, strlen(ml->data)))
			ml = g_list_insert_sorted(ml,
						  g_strdup(e->gr_name),
						  (GCompareFunc)strcmp);
	endgrent();
	return init_ml(ml);
}

static GList *new_system_user_ml(GList *ml, char *p)
{
	struct passwd *e;

	if (ml)
		return next_ml(ml);

	ml = g_list_append(ml, g_strdup(p));
	setpwent();
	while ((e = getpwent()))
		if (!strncmp(e->pw_name, ml->data, strlen(ml->data)))
			ml = g_list_insert_sorted(ml,
						  g_strdup(e->pw_name),
						  (GCompareFunc)strcmp);
	endpwent();
	return init_ml(ml);
}

static GList *new_path_ml(GList *ml, char *p, char *buf)
{
	g_autofree char *name = NULL;
	DIR *dir;
	struct dirent *e;

	if (ml)
		return next_ml(ml);

	ml = g_list_append(ml, g_strdup(p));
	for (name = buf; name < p; name++)
		if (*name == '/')
			break;
	name = g_strndup(name, p - name);
	if (*name == 0x00)
		return init_ml(ml);

	if (global_conf.root_dir) {
		char *new_name =
			g_strdup_printf("%s%s", global_conf.root_dir, name);

		g_free(name);
		name = new_name;
	}

	dir = opendir(name);
	if (!dir)
		return init_ml(ml);

	while ((e = readdir(dir)))
		if (e->d_type == DT_DIR &&
		    strcmp(e->d_name, ".") &&
		    strcmp(e->d_name, "..") &&
		    !strncmp(e->d_name, ml->data, strlen(ml->data)))
			ml = g_list_insert_sorted(ml,
						  g_strdup(e->d_name),
						  (GCompareFunc)strcmp);
	closedir(dir);
	return init_ml(ml);
}

static GList *new_va_ml(GList *ml, char *p, const char *arg, ...)
{
	va_list args;

	if (ml)
		return next_ml(ml);

	ml = g_list_append(ml, g_strdup(p));
	va_start(args, arg);
	for (; arg; arg = va_arg(args, char *))
		if (!strncmp(arg, ml->data, strlen(ml->data)))
			ml = g_list_append(ml, g_strdup(arg));
	va_end(args);
	return init_ml(ml);
}

static char *__rtrim_path(char *buf, char *p)
{
	for (; p > buf; p--)
		if (p[-1] == '/')
			break;
	return p;
}

static char *__rtrim_list(char *buf, char *p)
{
	for (; p > buf; p--)
		if (p[-1] == '\t' || p[-1] == ' ' || p[-1] == ',')
			break;
	return p;
}

static GList *new_conf_ml(GList *ml,
			  enum KSMBD_SHARE_CONF conf,
			  char *buf,
			  size_t *buflen)
{
	char *p = buf;

	switch (conf) {
	case KSMBD_SHARE_CONF_PATH:
		p = __rtrim_path(buf, p + *buflen);
		ml = new_path_ml(ml, p, buf);
		if (!*buflen) {
			buf[(*buflen)++] = '/';
			buf[(*buflen)] = 0x00;
			ml = clear_ml(ml);
		}
		break;
	case KSMBD_SHARE_CONF_GUEST_OK:
	case KSMBD_SHARE_CONF_READ_ONLY:
	case KSMBD_SHARE_CONF_BROWSEABLE:
	case KSMBD_SHARE_CONF_WRITE_OK:
	case KSMBD_SHARE_CONF_WRITEABLE:
	case KSMBD_SHARE_CONF_STORE_DOS_ATTRIBUTES:
	case KSMBD_SHARE_CONF_OPLOCKS:
	case KSMBD_SHARE_CONF_HIDE_DOT_FILES:
	case KSMBD_SHARE_CONF_INHERIT_OWNER:
	case KSMBD_SHARE_CONF_FOLLOW_SYMLINKS:
	case KSMBD_SHARE_CONF_WRITABLE:
	case KSMBD_SHARE_CONF_CROSSMNT:
		ml = new_va_ml(ml, p, "yes", "no", NULL);
		break;
	case KSMBD_SHARE_CONF_GUEST_ACCOUNT:
		ml = new_user_ml(ml, p);
		break;
	case KSMBD_SHARE_CONF_FORCE_GROUP:
		ml = new_system_group_ml(ml, p);
		break;
	case KSMBD_SHARE_CONF_FORCE_USER:
		ml = new_system_user_ml(ml, p);
		break;
	case KSMBD_SHARE_CONF_VALID_USERS:
	case KSMBD_SHARE_CONF_INVALID_USERS:
	case KSMBD_SHARE_CONF_READ_LIST:
	case KSMBD_SHARE_CONF_WRITE_LIST:
	case KSMBD_SHARE_CONF_ADMIN_USERS:
		p = __rtrim_list(buf, p + *buflen);
		ml = new_user_ml(ml, p);
		break;
	case KSMBD_SHARE_CONF_VFS_OBJECTS:
		p = __rtrim_list(buf, p + *buflen);
		ml = new_va_ml(ml, p, "acl_xattr", "streams_xattr", NULL);
	}

	if (ml) {
		snprintf(p, LINE_MAX - (p - buf), "%s", (char *)ml->data);
		*buflen = p - buf + strlen(p);
	}
	return ml;
}

static int __prompt_options_stdin(char **options, int is_global)
{
	g_autoptr(GList) ml = NULL;
	struct termios term, raw_term;
	enum KSMBD_SHARE_CONF conf;
	int is_ready, step, ccode, icode, licode, row, col;
	char buf[LINE_MAX];
	size_t buflen;

	pr_info("Prompting for options\n");

	tcgetattr(STDIN_FILENO, &term);
	raw_term = term;

	raw_term.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term);

	printf(
		"  <enter> <down> <control-n>    next parameter\n"
		"            <up> <control-p>    previous parameter\n"
		"                 <control-w>    erase value\n"
		"                       <tab>    cycle completions\n"
		"                   <space> ,    list separator\n"
		"                         ; #    comment with [default value]\n"
		"\e[1m" "Share parameters" "\e[m" "\n");

	for (conf = is_ready = 0, step = 1;
	     conf < KSMBD_SHARE_CONF_MAX;
	     conf = __next_conf(conf + step, step, is_global, &is_ready)) {
		int c;

		if (step) {
			buflen = 0;
			buf[buflen] = 0x00;
			ml = clear_ml(ml);
			__load_conf(conf, step, options, buf, &buflen);
			ccode = icode = licode = row = col = 0;
			step = 0;
		}

		if (ccode == 'A') {
			step = -1;
			__load_conf(conf, step, options, buf, &buflen);
			ccode = icode = licode = 0;
			continue;
		}
		if (ccode == 'B') {
			step = 1;
			__load_conf(conf, step, options, buf, &buflen);
			ccode = icode = licode = 0;
			continue;
		}

		if (ccode >= '0' && ccode <= '9') {
			icode = 10 * icode + ccode - '0';
		} else if (ccode == ';') {
			licode = icode;
			icode = 0;
		} else if (ccode == 'R') {
			col = icode;
			row = licode;
			ccode = icode = licode = 0;
		}

		c = getchar();
		if (c == EOF || c == 0x04)
			break;

		if (c == '\e' ||
		    (c == '[' && ccode == '\e') ||
		    (c == 'A' && ccode == '[') ||
		    (c == 'B' && ccode == '[') ||
		    (c >= '0' && c <= '9' && ccode == '[') ||
		    (c >= '0' && c <= '9' && ccode >= '0' && ccode <= '9') ||
		    (c == ';' && ccode >= '0' && ccode <= '9') ||
		    (c >= '0' && c <= '9' && ccode == ';') ||
		    (c == 'R' && ccode >= '0' && ccode <= '9')) {
			ccode = c;
			continue;
		}
		ccode = icode = licode = 0;

		if (c == 0x10) {
			step = -1;
			__load_conf(conf, step, options, buf, &buflen);
			continue;
		}
		if (c == 0x0E || c == '\n') {
			step = 1;
			__load_conf(conf, step, options, buf, &buflen);
			continue;
		}

		if (c == 0x17) {
			buflen = 0;
			buf[buflen] = 0x00;
			ml = clear_ml(ml);
			__load_conf(conf, step, options, buf, &buflen);
			continue;
		}

		if (cp_smbconf_eol(cp_ltrim(buf))) {
			buflen = 0;
			buf[buflen] = 0x00;
			ml = clear_ml(ml);
		}

		if (c == '\t') {
			ml = new_conf_ml(ml, conf, buf, &buflen);
			__load_conf(conf, step, options, buf, &buflen);
			continue;
		}

		if (c == raw_term.c_cc[VERASE]) {
			char *u8c;
			size_t clen;

			u8c = g_utf8_find_prev_char(buf, buf + buflen);
			clen = u8c ? buf + buflen - u8c : 1;
			buflen -= buflen > clen ? clen : buflen;
			buf[buflen] = 0x00;
			ml = clear_ml(ml);
			__load_conf(conf, step, options, buf, &buflen);
			continue;
		}

		if (buflen < LINE_MAX - 1 && col < 80) {
			buf[buflen++] = c;
			buf[buflen] = 0x00;
			ml = clear_ml(ml);
			if (!cp_printable(buf + buflen - 1))
				buf[--buflen] = 0x00;
			__load_conf(conf, step, options, buf, &buflen);
		}
	}

	printf("\eD" "\r" "\e[2K");

	raw_term.c_cc[VMIN] = 0;
	raw_term.c_cc[VTIME] = 3;
	tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);
	while (getchar() != EOF)
		;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

	ml = clear_ml(ml);
	return conf < KSMBD_SHARE_CONF_MAX ? -EINVAL : 0;
}

static int process_options(char ***options, GHashTable *kv, int is_global)
{
	if (!**options) {
		g_free(*options);
		*options = __get_options(kv, is_global);
		return __prompt_options_stdin(*options, is_global);
	}
	return 0;
}

static GList *new_share_nl(void)
{
	GList *nl = g_hash_table_get_keys(parser.groups), *l = NULL;

	nl = g_list_sort(nl, (GCompareFunc)strcmp);
	if (parser.ipc) {
		l = g_list_find(nl, parser.ipc->name);
		nl = g_list_remove_link(nl, l);
	}
	nl = g_list_concat(l, nl);
	if (parser.global) {
		l = g_list_find(nl, parser.global->name);
		nl = g_list_remove_link(nl, l);
	}
	return g_list_concat(l, nl);
}

static GList *new_share_kl(struct smbconf_group *g)
{
	GList *l = g_hash_table_get_keys(g->kv), *kl = NULL;
	int is_global = g == parser.global;
	enum KSMBD_SHARE_CONF c;
	char *k;

	for (c = 0; c < KSMBD_SHARE_CONF_MAX; c++)
		if ((!is_global || !KSMBD_SHARE_CONF_IS_GLOBAL(c)) &&
		    !KSMBD_SHARE_CONF_IS_BROKEN(c) &&
		    g_hash_table_lookup_extended(g->kv,
						 KSMBD_SHARE_CONF[c],
						 (gpointer *)&k,
						 NULL)) {
			l = g_list_remove(l, k);
			kl = g_list_insert_sorted(kl, k, (GCompareFunc)strcmp);
		}
	l = g_list_sort(l, (GCompareFunc)strcmp);
	if (kl)
		kl = g_list_insert(kl, NULL, 0);
	return g_list_concat(l, kl);
}

static void __gptrarray_add_share_kl(GPtrArray *gptrarray,
				     GList *kl,
				     GHashTable *kv,
				     int is_global)
{
	GList *l;

	if (kl && kl->data)
		gptrarray_printf(
			gptrarray,
			"\t" "; " "%s" "parameters\n",
			is_global ? "global " : "");

	for (l = kl; l; l = l->next) {
		char *k, *v;

		if (!l->data) {
			gptrarray_printf(
				gptrarray,
				"%s" "\t" "; " "%s" "share parameters\n",
				kl->data ? "\n" : "",
				is_global ? "default " : "");
			continue;
		}

		k = l->data;
		v = g_hash_table_lookup(kv, k);
		gptrarray_printf(gptrarray, "\t" "%s = %s\n", k, v);
	}
}

static char *get_conf_contents(void)
{
	GPtrArray *lines = g_ptr_array_new();
	g_autoptr(GList) nl = new_share_nl();
	GList *l;

	gptrarray_printf(lines, "; see ksmbd.conf(5) for details\n" "\n");
	for (l = nl; l; l = l->next) {
		struct smbconf_group *g =
			g_hash_table_lookup(parser.groups, l->data);
		g_autoptr(GList) kl = new_share_kl(g);

		gptrarray_printf(lines, "[%s]\n", g->name);
		__gptrarray_add_share_kl(lines, kl, g->kv, g == parser.global);
		gptrarray_printf(lines, "\n");
	}
	return gptrarray_to_str(lines);
}

int command_add_share(char *smbconf, char *name, char **options)
{
	g_autofree char *contents = NULL;
	int ret, is_global;

	if (g_hash_table_lookup(parser.groups, name)) {
		pr_err("Share `%s' already exists\n", name);
		ret = -EEXIST;
		goto out;
	}

	is_global = shm_share_name_equal(name, "global");
	ret = process_options(&options, NULL, is_global);
	if (ret)
		goto out;

	cp_parse_external_smbconf_group(name, options);

	contents = get_conf_contents();
	ret = set_conf_contents(smbconf, contents);
	if (ret)
		goto out;

	pr_info("Added share `%s'\n", name);
out:
	g_free(smbconf);
	g_free(name);
	g_strfreev(options);
	return ret;
}

int command_update_share(char *smbconf, char *name, char **options)
{
	g_autofree char *contents = NULL;
	struct smbconf_group *g;
	int ret, is_global;

	g = g_hash_table_lookup(parser.groups, name);
	if (!g) {
		pr_err("Share `%s' does not exist\n", name);
		ret = -EINVAL;
		goto out;
	}

	is_global = g == parser.global;
	ret = process_options(&options, g->kv, is_global);
	if (ret)
		goto out;

	cp_parse_external_smbconf_group(name, options);

	contents = get_conf_contents();
	ret = set_conf_contents(smbconf, contents);
	if (ret)
		goto out;

	pr_info("Updated share `%s'\n", name);
out:
	g_free(smbconf);
	g_free(name);
	g_strfreev(options);
	return ret;
}

int command_delete_share(char *smbconf, char *name, char **options)
{
	g_autofree char *contents = NULL;
	struct smbconf_group *g;
	int ret, is_global;

	g = g_hash_table_lookup(parser.groups, name);
	if (!g) {
		pr_err("Share `%s' does not exist\n", name);
		ret = -EINVAL;
		goto out;
	}

	is_global = g == parser.global;
	if (is_global) {
		g_strfreev(options);
		options = __get_options(NULL, is_global);
		return command_update_share(smbconf, name, options);
	}

	g_hash_table_remove(parser.groups, name);

	contents = get_conf_contents();
	ret = set_conf_contents(smbconf, contents);
	if (ret)
		goto out;

	pr_info("Deleted share `%s'\n", name);
out:
	g_free(smbconf);
	g_free(name);
	g_strfreev(options);
	return ret;
}
