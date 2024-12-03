// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <glib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ksmbd_server.h>

#include <config_parser.h>
#include <tools.h>
#include <management/user.h>
#include <management/share.h>

struct smbconf_global global_conf;
struct smbconf_parser parser;

typedef int process_entry_fn(char *entry);
static process_entry_fn process_smbconf_entry,
			process_pwddb_entry,
			process_subauth_entry,
			process_lock_entry;

unsigned long long cp_memparse(char *v)
{
	char *cp;
	unsigned long long ull = strtoull(v, &cp, 0);

	switch (*cp) {
	case 'E':
	case 'e':
		ull <<= 10;
		/* Fall through */
	case 'P':
	case 'p':
		ull <<= 10;
		/* Fall through */
	case 'T':
	case 't':
		ull <<= 10;
		/* Fall through */
	case 'G':
	case 'g':
		ull <<= 10;
		/* Fall through */
	case 'M':
	case 'm':
		ull <<= 10;
		/* Fall through */
	case 'K':
	case 'k':
		ull <<= 10;
	}

	return ull;
}

static int is_ascii_space_tab(char c)
{
	return c == ' ' || c == '\t';
}

static int is_a_group(char *entry)
{
	char *delim;
	int is_group;

	is_group = *entry == '[';
	if (!is_group)
		goto out;
	entry++;
	delim = strchr(entry, ']');
	is_group = shm_share_name(entry, delim);
	if (!is_group)
		goto out;
	entry = cp_ltrim(delim + 1);
	is_group = cp_smbconf_eol(entry);
	if (!is_group) {
		pr_debug("Group contains `%c' [0x%.2X]\n",
			 *entry,
			 (unsigned char)*entry);
		goto out;
	}
	*entry = 0x00;
out:
	return is_group;
}

static int is_a_key_value(char *entry)
{
	char *delim;
	int is_key_value;

	delim = strchr(entry, '=');
	is_key_value = delim > entry;
	if (!is_key_value)
		goto out;
	for (; entry < delim; entry++) {
		is_key_value = cp_printable(entry) && !cp_smbconf_eol(entry);
		if (!is_key_value) {
			pr_debug("Key contains `%c' [0x%.2X]\n",
				 *entry,
				 (unsigned char)*entry);
			goto out;
		}
	}
	is_key_value = !!parser.current;
	if (!is_key_value) {
		pr_debug("Key has no group\n");
		goto out;
	}
	entry = cp_ltrim(entry + 1);
	for (; !cp_smbconf_eol(entry); entry++) {
		is_key_value = cp_printable(entry);
		if (!is_key_value) {
			pr_debug("Value contains `%c' [0x%.2X]\n",
				 *entry,
				 (unsigned char)*entry);
			goto out;
		}
	}
	*entry = 0x00;
out:
	return is_key_value;
}

static unsigned int key_hash(const char *k)
{
	g_autofree char *ck = g_ascii_strdown(k, -1);

	return g_str_hash(ck);
}

static int key_equal(const char *lk, const char *rk)
{
	return !cp_key_cmp(lk, rk);
}

static void add_group(const char *entry)
{
	g_autofree char *name =
		g_strndup(entry + 1, strchr(entry, ']') - entry - 1);
	struct smbconf_group *g =
		g_hash_table_lookup(parser.groups, name);

	if (g) {
		parser.current = g;
		return;
	}

	g = g_malloc(sizeof(struct smbconf_group));
	g->name = name;
	g->kv = g_hash_table_new_full((GHashFunc)key_hash,
				      (GEqualFunc)key_equal,
				      g_free,
				      g_free);
	g_hash_table_insert(parser.groups, name, g);
	name = NULL;

	parser.current = g;
	if (!parser.global && shm_share_name_equal(g->name, "global"))
		parser.global = g;
	else if (!parser.ipc && shm_share_name_equal(g->name, "ipc$"))
		parser.ipc = g;
}

static void add_group_key_value(const char *entry)
{
	const char *delim = strchr(entry, '=');
	g_autofree char *k =
		g_strndup(entry, cp_rtrim(entry, delim - 1) + 1 - entry);
	g_autofree char *v =
		g_strdup(cp_ltrim(delim + 1));

	if (cp_smbconf_eol(v) || g_hash_table_lookup(parser.current->kv, k))
		return;

	g_hash_table_insert(parser.current->kv, k, v);
	k = v = NULL;
}

static int process_smbconf_entry(char *entry)
{
	entry = cp_ltrim(entry);

	if (cp_smbconf_eol(entry))
		return 0;

	if (is_a_group(entry)) {
		add_group(entry);
		return 0;
	}

	if (is_a_key_value(entry)) {
		add_group_key_value(entry);
		return 0;
	}

	pr_err("Invalid smbconf entry `%s'\n", entry);
	return -EINVAL;
}

static int __mmap_parse_file(const char *path, process_entry_fn *process_entry)
{
	GError *error = NULL;
	GMappedFile *file;
	char *contents, *delim;
	size_t len;
	int fd, ret;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		ret = -errno;
		pr_debug("Can't open `%s': %m\n", path);
		goto out;
	}

	file = g_mapped_file_new_from_fd(fd, 0, &error);
	if (error) {
		pr_err("%s\n", error->message);
		g_error_free(error);
		ret = -EINVAL;
		goto out_close;
	}

	contents = g_mapped_file_get_contents(file);
	if (!contents) {
		g_autofree char *entry = g_strdup("");

		ret = process_entry(entry);
		goto out_unref;
	}

	for (len = g_mapped_file_get_length(file);
	     len > 0 && len != (size_t)-1;
	     len -= delim - contents + 1, contents = delim + 1) {
		g_autofree char *entry = NULL;

		delim = memchr(contents, '\n', len) ?: contents + len;
		entry = g_strndup(contents, delim - contents);
		ret = process_entry(entry);
		if (ret ||
		    process_entry == process_subauth_entry ||
		    process_entry == process_lock_entry)
			goto out_unref;
	}

out_unref:
	g_mapped_file_unref(file);
out_close:
	close(fd);
out:
	return ret;
}

static void free_group(struct smbconf_group *g)
{
	g_hash_table_destroy(g->kv);
	g_free(g->name);
	g_free(g);
}

char *cp_ltrim(const char *v)
{
	while (is_ascii_space_tab(*v))
		v++;
	return (char *)v;
}

char *cp_rtrim(const char *v, const char *p)
{
	while (p != v && is_ascii_space_tab(*p))
		p--;
	return (char *)p;
}

int cp_key_cmp(const char *lk, const char *rk)
{
	return g_ascii_strncasecmp(lk, rk, strlen(rk));
}

char *cp_get_group_kv_string(char *v)
{
	return g_strdup(v);
}

int cp_get_group_kv_bool(char *v)
{
	return !cp_key_cmp(v, "yes") ||
	       !cp_key_cmp(v, "1") ||
	       !cp_key_cmp(v, "true") ||
	       !cp_key_cmp(v, "enable");
}

int cp_get_group_kv_config_opt(char *v)
{
	if (!cp_key_cmp(v, "enabled"))
		return KSMBD_CONFIG_OPT_ENABLED;
	if (!cp_key_cmp(v, "auto"))
		return KSMBD_CONFIG_OPT_AUTO;
	if (!cp_key_cmp(v, "mandatory"))
		return KSMBD_CONFIG_OPT_MANDATORY;
	return KSMBD_CONFIG_OPT_DISABLED;
}

unsigned long cp_get_group_kv_long_base(char *v, int base)
{
	return strtoul(v, NULL, base);
}

unsigned long cp_get_group_kv_long(char *v)
{
	return cp_get_group_kv_long_base(v, 10);
}

char **cp_get_group_kv_list(char *v)
{
	return g_strsplit_set(v, "\t ,", -1);
}

void cp_group_kv_list_free(char **list)
{
	g_strfreev(list);
}

int cp_group_kv_steal(GHashTable *kv, const char *lookup, char **k, char **v)
{
	g_free(*k);
	g_free(*v);

	return g_hash_table_steal_extended(kv,
					   lookup,
					   (gpointer *)k,
					   (gpointer *)v);
}

static int group_kv_steal(GHashTable *kv,
			  const char *lookup,
			  char **k,
			  char **v)
{
	int is_steal = cp_group_kv_steal(kv, lookup, k, v) &&
		       !(ksmbd_health_status & KSMBD_SHOULD_RELOAD_CONFIG);

	if (!is_steal) {
		g_free(*k);
		g_free(*v);
		*k = NULL;
		*v = NULL;
	}
	return is_steal;
}

static int process_global_conf_kv(GHashTable *kv)
{
	g_autofree char *k = NULL, *v = NULL;

	if (group_kv_steal(kv, "server string", &k, &v)) {
		global_conf.server_string = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "workgroup", &k, &v)) {
		global_conf.work_group = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "netbios name", &k, &v)) {
		global_conf.netbios_name = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "server min protocol", &k, &v)) {
		global_conf.server_min_protocol = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "server signing", &k, &v)) {
		global_conf.server_signing = cp_get_group_kv_config_opt(v);
	}

	if (group_kv_steal(kv, "server max protocol", &k, &v)) {
		global_conf.server_max_protocol = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "guest account", &k, &v)) {
		global_conf.guest_account = cp_get_group_kv_string(v);
	}

	/* must also happen on config reload */
	if (usm_add_guest_account(global_conf.guest_account))
		return -EINVAL;

	if (group_kv_steal(kv, "max active sessions", &k, &v)) {
		global_conf.sessions_cap = cp_get_group_kv_long(v);
		if (global_conf.sessions_cap <= 0 ||
		    global_conf.sessions_cap > KSMBD_CONF_MAX_ACTIVE_SESSIONS)
			global_conf.sessions_cap =
				KSMBD_CONF_MAX_ACTIVE_SESSIONS;
	}

	if (group_kv_steal(kv, "tcp port", &k, &v)) {
		/* mountd option has precedence */
		if (!global_conf.tcp_port)
			global_conf.tcp_port = cp_get_group_kv_long(v);
	}

	if (group_kv_steal(kv, "ipc timeout", &k, &v)) {
		global_conf.ipc_timeout = cp_get_group_kv_long(v);
	}

	if (group_kv_steal(kv, "max open files", &k, &v)) {
		global_conf.file_max = cp_get_group_kv_long(v);
		if (!global_conf.file_max ||
		    global_conf.file_max > KSMBD_CONF_MAX_OPEN_FILES)
			global_conf.file_max = KSMBD_CONF_MAX_OPEN_FILES;
	}

	if (group_kv_steal(kv, "restrict anonymous", &k, &v)) {
		global_conf.restrict_anon = cp_get_group_kv_long(v);
		if (global_conf.restrict_anon != KSMBD_RESTRICT_ANON_TYPE_1 &&
		    global_conf.restrict_anon != KSMBD_RESTRICT_ANON_TYPE_2)
			global_conf.restrict_anon = 0;
	}

	if (group_kv_steal(kv, "map to guest", &k, &v)) {
		if (!cp_key_cmp(v, "bad user"))
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_BAD_USER;
/* broken */
#if 0
		else if (!cp_key_cmp(v, "bad password"))
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_BAD_PASSWORD;
		else if (!cp_key_cmp(v, "bad uid"))
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_BAD_UID;
#endif
		else
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_NEVER;
	}

	if (group_kv_steal(kv, "bind interfaces only", &k, &v)) {
		global_conf.bind_interfaces_only = cp_get_group_kv_bool(v);
	}

	if (group_kv_steal(kv, "interfaces", &k, &v)) {
		global_conf.interfaces = cp_get_group_kv_list(v);
	}

	if (group_kv_steal(kv, "deadtime", &k, &v)) {
		global_conf.deadtime = cp_get_group_kv_long(v);
	}

	if (group_kv_steal(kv, "smb2 leases", &k, &v)) {
		if (cp_get_group_kv_bool(v))
			global_conf.flags |= KSMBD_GLOBAL_FLAG_SMB2_LEASES;
		else
			global_conf.flags &= ~KSMBD_GLOBAL_FLAG_SMB2_LEASES;
	}

	if (group_kv_steal(kv, "root directory", &k, &v)) {
		global_conf.root_dir = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "smb2 max read", &k, &v)) {
		global_conf.smb2_max_read = cp_memparse(v);
	}

	if (group_kv_steal(kv, "smb2 max write", &k, &v)) {
		global_conf.smb2_max_write = cp_memparse(v);
	}

	if (group_kv_steal(kv, "smb2 max trans", &k, &v)) {
		global_conf.smb2_max_trans = cp_memparse(v);
	}

	if (group_kv_steal(kv, "smb3 encryption", &k, &v)) {
		switch (cp_get_group_kv_config_opt(v)) {
		case KSMBD_CONFIG_OPT_DISABLED:
			global_conf.flags |=
				KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION_OFF;
			global_conf.flags &=
				~KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;
			break;
		case KSMBD_CONFIG_OPT_MANDATORY:
			global_conf.flags |=
				KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;
			global_conf.flags &=
				~KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION_OFF;
			break;
		default:
			global_conf.flags &=
				~KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;
			global_conf.flags &=
				~KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION_OFF;
			break;
		}
	}

	if (group_kv_steal(kv, "share:fake_fscaps", &k, &v)) {
		global_conf.share_fake_fscaps = cp_get_group_kv_long(v);
	}

	if (group_kv_steal(kv, "kerberos service name", &k, &v)) {
		global_conf.krb5_service_name = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "kerberos keytab file", &k, &v)) {
		global_conf.krb5_keytab_file = cp_get_group_kv_string(v);
	}

	if (group_kv_steal(kv, "kerberos support", &k, &v)) {
		global_conf.krb5_support = cp_get_group_kv_bool(v);
	}

	if (group_kv_steal(kv, "server multi channel support", &k, &v)) {
		if (cp_get_group_kv_bool(v))
			global_conf.flags |=
				KSMBD_GLOBAL_FLAG_SMB3_MULTICHANNEL;
		else
			global_conf.flags &=
				~KSMBD_GLOBAL_FLAG_SMB3_MULTICHANNEL;
	}

	if (group_kv_steal(kv, "smb2 max credits", &k, &v)) {
		global_conf.smb2_max_credits = cp_memparse(v);
	}

	if (group_kv_steal(kv, "smbd max io size", &k, &v)) {
		global_conf.smbd_max_io_size = cp_memparse(v);
	}

	if (group_kv_steal(kv, "max connections", &k, &v)) {
		global_conf.max_connections = cp_memparse(v);
		if (!global_conf.max_connections ||
		    global_conf.max_connections > KSMBD_CONF_MAX_CONNECTIONS)
			global_conf.max_connections =
				KSMBD_CONF_MAX_CONNECTIONS;
	}

	if (group_kv_steal(kv, "durable handles", &k, &v)) {
		if (cp_get_group_kv_bool(v))
			global_conf.flags |=
				KSMBD_GLOBAL_FLAG_DURABLE_HANDLES;
		else
			global_conf.flags &=
				~KSMBD_GLOBAL_FLAG_DURABLE_HANDLES;
	}

	return 0;
}

static void add_group_global_conf(void)
{
	if (ksmbd_health_status & KSMBD_SHOULD_RELOAD_CONFIG)
		return;

	add_group_key_value("guest account = nobody");
	add_group_key_value("max active sessions = 1024");
	add_group_key_value("max connections = 128");
	add_group_key_value("max open files = 10000");
	add_group_key_value("netbios name = KSMBD SERVER");
	add_group_key_value("server string = SMB SERVER");
	add_group_key_value("share:fake_fscaps = 64"); /* sparse files */
	add_group_key_value("tcp port = 445");
	add_group_key_value("workgroup = WORKGROUP");
}

static void steal_global_share_conf_kv(GHashTable *kv)
{
	enum KSMBD_SHARE_CONF c;

	for (c = 0; c < KSMBD_SHARE_CONF_MAX; c++) {
		g_autofree char *k = NULL, *v = NULL, *entry = NULL;
		GHashTableIter iter;

		if (KSMBD_SHARE_CONF_IS_BROKEN(c))
			continue;

		if (!KSMBD_SHARE_CONF_IS_GLOBAL(c))
			cp_group_kv_steal(kv, KSMBD_SHARE_CONF[c], &k, &v);

		entry = g_strdup_printf("%s = %s",
					k ?: KSMBD_SHARE_CONF[c],
					v ?: KSMBD_SHARE_DEFCONF[c]);

		ghash_for_each(parser.current, parser.groups, iter) {
			if (parser.current == parser.global)
				continue;

			add_group_key_value(entry);
		}
	}
}

static void add_group_ipc_share_conf(void)
{
	add_group_key_value("comment = IPC share");
	add_group_key_value("guest ok = yes");
}

static void ignore_group_kv(struct smbconf_group *group)
{
	char *k, *v;
	GHashTableIter iter;

	if (!g_hash_table_size(group->kv))
		return;

	pr_debug("Ignored key-values in group `%s'\n", group->name);

	ghash_for_each_steal(k, v, group->kv, iter) {
		pr_debug("... `%s = %s'\n", k, v);

		g_free(k);
		g_free(v);
	}
}

static int finalize_smbconf_parser(void)
{
	int ret;
	GHashTableIter iter;

	add_group("[global]");
	add_group_global_conf();

	ret = process_global_conf_kv(parser.global->kv);
	if (ret)
		goto out;

	add_group("[ipc$]");
	add_group_ipc_share_conf();

	steal_global_share_conf_kv(parser.global->kv);

	ghash_for_each_steal(parser.current, parser.groups, iter) {
		if (parser.current == parser.global)
			continue;

		if (shm_add_new_share(parser.current))
			ret = -EINVAL;

		ignore_group_kv(parser.current);
		free_group(parser.current);
	}

	ignore_group_kv(parser.global);
	free_group(parser.global);
out:
	cp_smbconf_parser_destroy();
	return ret;
}

void cp_smbconf_parser_init(void)
{
	if (!parser.groups)
		parser.groups = g_hash_table_new_full(
			(GHashFunc)shm_share_name_hash,
			(GEqualFunc)shm_share_name_equal,
			NULL,
			(GDestroyNotify)free_group);
}

void cp_smbconf_parser_destroy(void)
{
	if (parser.groups) {
		g_hash_table_destroy(parser.groups);
		parser.groups = NULL;
	}

	parser.current = parser.global = parser.ipc = NULL;
}

int cp_parse_smbconf(char *smbconf)
{
	int is_owner = !parser.groups, ret;

	cp_smbconf_parser_init();
	ret = __mmap_parse_file(smbconf, process_smbconf_entry);
	if (ret == -ENOENT) {
		if (TOOL_IS_ADDSHARE) {
			ret = set_conf_contents(smbconf, "");
		} else if (TOOL_IS_MOUNTD) {
			pr_err("No configuration file\n");
		} else {
			ret = 0;
			pr_info("No configuration file\n");
		}
	}
	if (ret) {
		if (is_owner)
			cp_smbconf_parser_destroy();
		return ret;
	}

	if (!is_owner) {
		struct smbconf_parser backup = parser;

		parser = (struct smbconf_parser){0};
		ret = cp_parse_smbconf(smbconf);
		parser = backup;
		return ret;
	}

	return finalize_smbconf_parser();
}

static int is_a_user_password(char *entry)
{
	char *delim;
	int is_user_password;

	delim = strchr(entry, ':');
	is_user_password = usm_user_name(entry, delim);
	if (!is_user_password)
		goto out;
	entry = delim + 1;
	for (; !cp_pwddb_eol(delim); delim++)
		;
	*delim = 0x00;
	for (; delim > entry; delim--)
		if (delim[-1] != '=')
			break;
	is_user_password = delim > entry;
	if (!is_user_password) {
		pr_debug("Password is missing\n");
		goto out;
	}
	for (; entry < delim; entry++) {
		is_user_password =
			(*entry >= '0' && *entry <= '9') ||
			(*entry >= 'A' && *entry <= 'Z') ||
			(*entry >= 'a' && *entry <= 'z') ||
			(*entry == '+') ||
			(*entry == '/');
		if (!is_user_password) {
			pr_debug("Password contains `%c' [0x%.2X]\n",
				 *entry,
				 (unsigned char)*entry);
			goto out;
		}
	}
out:
	return is_user_password;
}

static void add_user_password(const char *entry)
{
	const char *delim = strchr(entry, ':');
	g_autofree char *name = g_strndup(entry, delim - entry);
	g_autofree char *pwd = g_strdup(delim + 1);
	struct ksmbd_user *user = usm_lookup_user(name);

	if (user) {
		usm_update_user_password(user, pwd);
		put_ksmbd_user(user);
		return;
	}

	usm_add_new_user(name, pwd);
	name = pwd = NULL;
}

static int process_pwddb_entry(char *entry)
{
	if (cp_pwddb_eol(entry))
		return 0;

	if (is_a_user_password(entry)) {
		add_user_password(entry);
		return 0;
	}

	pr_err("Invalid pwddb entry `%s'\n", entry);
	return -EINVAL;
}

int cp_parse_pwddb(char *pwddb)
{
	int ret = __mmap_parse_file(pwddb, process_pwddb_entry);

	if (ret == -ENOENT) {
		if (TOOL_IS_ADDUSER) {
			ret = set_conf_contents(pwddb, "");
		} else {
			ret = 0;
			pr_info("No user database\n");
		}
	}
	return ret;
}

static int is_a_subauth(char *entry)
{
	int num_subauth = ARRAY_SIZE(global_conf.gen_subauth), is_subauth = 0;
	int i;

	for (i = 0; i < num_subauth; i++) {
		char *delim = strchr(entry, i + 1 < num_subauth ? ':' : 0x00);

		is_subauth = delim > entry;
		if (!is_subauth) {
			pr_debug("Subauth is missing\n");
			goto out;
		}
		for (; entry < delim; entry++) {
			is_subauth = *entry >= '0' && *entry <= '9';
			if (!is_subauth) {
				pr_debug("Subauth contains `%c' [0x%.2X]\n",
					 *entry,
					 (unsigned char)*entry);
				goto out;
			}
		}
		entry++;
	}
out:
	return is_subauth;
}

static void add_subauth(const char *entry)
{
	int num_subauth = ARRAY_SIZE(global_conf.gen_subauth);
	int i;

	for (i = 0; i < num_subauth; i++) {
		const char *delim =
			strchr(entry, i + 1 < num_subauth ? ':' : 0x00);

		global_conf.gen_subauth[i] = 0;
		for (; entry < delim; entry++) {
			global_conf.gen_subauth[i] *= 10;
			global_conf.gen_subauth[i] += *entry - '0';
		}
		entry++;
	}
}

static int process_subauth_entry(char *entry)
{
	if (is_a_subauth(entry)) {
		add_subauth(entry);
		return 0;
	}

	pr_err("Invalid subauth entry `%s'\n", entry);
	return -EINVAL;
}

int cp_parse_subauth(void)
{
	int ret = __mmap_parse_file(PATH_SUBAUTH, process_subauth_entry);

	if (!TOOL_IS_MOUNTD)
		return ret;

	if (ret) {
		g_autofree char *contents = NULL;
		g_autoptr(GRand) rand = g_rand_new();
		int num_subauth = ARRAY_SIZE(global_conf.gen_subauth);
		int i;

		for (i = 0; i < num_subauth; i++) {
			char *new_contents = g_strdup_printf(
				"%s%u%c",
				contents ?: "",
				g_rand_int(rand),
				i + 1 < num_subauth ? ':' : '\n');

			g_free(contents);
			contents = new_contents;
		}

		if (ret == -ENOENT)
			ret = set_conf_contents(PATH_SUBAUTH, contents);

		*strchr(contents, '\n') = 0x00;
		add_subauth(contents);
	}
	return ret;
}

static int is_a_lock(char *entry)
{
	char *delim = strchr(entry, 0x00);
	int is_lock;
	pid_t pid;

	is_lock = delim > entry;
	if (!is_lock) {
		pr_debug("Lock is missing\n");
		goto out;
	}
	pid = 0;
	for (; entry < delim; entry++) {
		is_lock = *entry >= '0' && *entry <= '9';
		if (!is_lock) {
			pr_debug("Lock contains `%c' [0x%.2X]\n",
				 *entry,
				 (unsigned char)*entry);
			goto out;
		}
		pid *= 10;
		pid += *entry - '0';
	}
	is_lock = pid > 1;
	if (!is_lock) {
		pr_debug("Lock has invalid PID\n");
		goto out;
	}
	is_lock = !kill(pid, 0);
	if (!is_lock)
		pr_debug("Lock has orphaned PID\n");
out:
	return is_lock;
}

static void add_lock(const char *entry)
{
	global_conf.pid = 0;
	for (; *entry; entry++) {
		global_conf.pid *= 10;
		global_conf.pid += *entry - '0';
	}
}

static int process_lock_entry(char *entry)
{
	if (is_a_lock(entry)) {
		if (!TOOL_IS_MOUNTD) {
			add_lock(entry);
			return 0;
		}

		pr_err("Reserved lock entry `%s'\n", entry);
		return -EAGAIN;
	}

	pr_err("Invalid lock entry `%s'\n", entry);
	return -EINVAL;
}

int cp_parse_lock(void)
{
	int ret = __mmap_parse_file(PATH_LOCK, process_lock_entry);

	if (!TOOL_IS_MOUNTD)
		return ret;

	if (ret) {
		g_autofree char *contents =
			g_strdup_printf("%d\n", getpid());

		if (ret == -ENOENT || ret == -EINVAL)
			ret = set_conf_contents(PATH_LOCK, contents);

		*strchr(contents, '\n') = 0x00;
		add_lock(contents);
	}
	return ret;
}

void cp_parse_external_smbconf_group(char *name, char **options)
{
	g_autofree char *group_name = g_strdup_printf("[%s]", name);
	int is_global;

	add_group(group_name);
	is_global = parser.current == parser.global;

	for (; *options; options++) {
		char *option = cp_ltrim(*options);

		if (cp_smbconf_eol(option))
			continue;

		if (is_a_key_value(option)) {
			enum KSMBD_SHARE_CONF c;

			for (c = 0; c < KSMBD_SHARE_CONF_MAX; c++)
				if ((!is_global ||
				     !KSMBD_SHARE_CONF_IS_GLOBAL(c)) &&
				    shm_share_config(option, c))
					break;

			if (c < KSMBD_SHARE_CONF_MAX) {
				g_hash_table_remove(parser.current->kv,
						    KSMBD_SHARE_CONF[c]);
				add_group_key_value(option);
				continue;
			}
		}

		pr_info("Ignored option `%s'\n", option);
	}
}
