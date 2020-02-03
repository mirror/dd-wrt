// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/mman.h>
#include <limits.h>
#include <linux/ksmbd_server.h>

#include <config_parser.h>
#include <ksmbdtools.h>
#include <management/user.h>
#include <management/share.h>

struct smbconf_global global_conf;
struct smbconf_parser parser;

unsigned long long memparse(const char *v)
{
	char *eptr;

	unsigned long long ret = strtoull(v, &eptr, 0);

	switch (*eptr) {
	case 'E':
	case 'e':
		ret <<= 10;
	case 'P':
	case 'p':
		ret <<= 10;
	case 'T':
	case 't':
		ret <<= 10;
	case 'G':
	case 'g':
		ret <<= 10;
	case 'M':
	case 'm':
		ret <<= 10;
	case 'K':
	case 'k':
		ret <<= 10;
	}

	return ret;
}

static void kv_release_cb(void *p, unsigned long long id, void *user_data)
{
	free(p);
}

static int is_ascii_space_tab(char c)
{
	return c == ' ' || c == '\t';
}

static int is_a_comment(char *line)
{
	return (*line == 0x00 || *line == ';' || *line == '\n' || *line == '#');
}

static char *utf8_find_next_char(const char *p, const char *end)
{
	if (end) {
		for (++p; p < end && (*p & 0xc0) == 0x80; ++p)
		;
		return (p >= end) ? NULL : (char *)p;
	}
	for (++p; (*p & 0xc0) == 0x80; ++p)
	;
	return (char *)p;
}

static int is_a_group(char *line)
{
	char *p = line;

	if (*p != '[')
		return 0;
	p++;
	while (*p && *p != ']')
		p = utf8_find_next_char(p, NULL);
	if (*p != ']')
		return 0;
	return 1;
}

static int add_new_group(char *line)
{
	char *begin = line;
	char *end = line;
	char *name = NULL;
	struct smbconf_group *group = NULL;
	struct smbconf_group *lookup;

	while (*end && *end != ']')
		end = utf8_find_next_char(end, NULL);

	name = ascii_strdown(begin + 1, end - begin - 1);
	if (!name)
		goto out_free;

	lookup = list_get(&parser.groups, list_tokey(name));
	if (lookup) {
		parser.current = lookup;
		pr_info("SMB conf: multiple group definitions `%s'\n", name);
		free(name);
		return 0;
	}

	group = malloc(sizeof(struct smbconf_group));
	if (!group)
		goto out_free;

	group->name = name;
	list_init(&group->kv);

	if (!group->kv)
		goto out_free;

	parser.current = group;
	list_add_str(&parser.groups, group, group->name);

	return 0;

out_free:
	free(name);
	if (group && group->kv)
		list_clear(&group->kv);
	free(group);
	return -ENOMEM;
}

static int add_group_key_value(char *line)
{
	char *key, *value;

	key = strchr(line, '=');
	if (!key)
		return -EINVAL;

	value = key;
	*key = 0x00;
	key--;
	value++;

	while (is_ascii_space_tab(*key))
		key--;
	while (is_ascii_space_tab(*value))
		value++;

	if (is_a_comment(value))
		return 0;

	if (list_get(&parser.current->kv, list_tokey(key))) {
		pr_info("SMB conf: multuple key-value [%s] %s\n",
			parser.current->name, key);
		return 0;
	}

	key = strndup(line, key - line + 1);
	value = strdup(value);

	if (!key || !value) {
		free(key);
		free(value);
		return -ENOMEM;
	}

	list_add_str(&parser.current->kv, value, key);
	return 0;
}

static int process_smbconf_entry(char *data)
{
	while (is_ascii_space_tab(*data))
		data++;

	if (is_a_comment(data))
		return 0;

	if (is_a_group(data))
		return add_new_group(data);

	return add_group_key_value(data);
}

static int __mmap_parse_file(const char *fname, int (*callback)(char *d))
{
	char *contents, *buf;
	size_t len;
	char *delim;
	int ret = 0;
	FILE *fd;

	fd = fopen(fname, "rb");
	if (fd == NULL) {
		ret = errno;
		pr_err("Can't open `%s': %s\n", fname, strerr(ret));
		return -ret;
	}
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	rewind(fd);
	buf = contents = malloc(len + 1);
	if (!contents)
		goto out;
	fread(contents, 1, len, fd);
	contents[len] = 0;

	while (len > 0) {
		delim = strchr(contents, '\n');
		if (!delim)
			delim = contents + len - 1;

		if (delim) {
			size_t sz = delim - contents;

			if (delim == contents) {
				contents = delim + 1;
				len--;
				continue;
			}

			if (!sz)
				break;
			char *data = strndup(contents, sz);

			ret = callback(data);
			if (ret) {
				free(data);
				goto out;
			}
			free(data);

			contents = delim + 1;
			len -= (sz + 1);
		}
	}

	ret = 0;
out:
	if (buf)
		free(buf);

	if (fd)
		fclose(fd);

	return ret;
}

static int init_smbconf_parser(void)
{
	if (parser.groups)
		return 0;

	list_init(&parser.groups);
	if (!parser.groups)
		return -ENOMEM;
	return 0;
}

static void release_smbconf_group(void *v, unsigned long long id,
				  void *user_data)
{
	struct smbconf_group *g = v;

	list_foreach(&g->kv, kv_release_cb, NULL);
	list_clear(&g->kv);
	free(g->name);
	free(g);
}

static void release_smbconf_parser(void)
{
	if (!parser.groups)
		return;
	list_foreach(&parser.groups, release_smbconf_group, NULL);
	list_clear(&parser.groups);
	parser.groups = NULL;
}

char *cp_ltrim(char *v)
{
	if (!v)
		return NULL;

	while (*v && *v == ' ')
		v++;
	if (*v == 0x00)
		return NULL;
	return v;
}

int cp_key_cmp(char *k, char *v)
{
	if (!k || !v)
		return -1;
	return strncasecmp(k, v, strlen(v));
}

char *cp_get_group_kv_string(char *v)
{
	return strdup(v);
}

int cp_get_group_kv_bool(char *v)
{
	if (!strncasecmp(v, "yes", 3) ||
	    !strncasecmp(v, "1", 1) ||
	    !strncasecmp(v, "true", 4) || !strncasecmp(v, "enable", 6))
		return 1;
	return 0;
}

int cp_get_group_kv_config_opt(char *v)
{
	if (!strncasecmp(v, "disabled", 8))
		return USMBD_CONFIG_OPT_DISABLED;
	if (!strncasecmp(v, "enabled", 7))
		return USMBD_CONFIG_OPT_ENABLED;
	if (!strncasecmp(v, "auto", 4))
		return USMBD_CONFIG_OPT_AUTO;
	if (!strncasecmp(v, "mandatory", 9))
		return USMBD_CONFIG_OPT_MANDATORY;
	return USMBD_CONFIG_OPT_DISABLED;
}

unsigned long cp_get_group_kv_long_base(char *v, int base)
{
	return strtoul(v, NULL, base);
}

unsigned long cp_get_group_kv_long(char *v)
{
	return cp_get_group_kv_long_base(v, 10);
}

struct SList {
	void *data;
	struct SList *next;
};

struct SList *slist_prepend(struct SList *list, void *data)
{
	struct SList *new_list;

	new_list = malloc(sizeof(*list));
	new_list->data = data;
	new_list->next = list;

	return new_list;
}

static char **strsplit_set(const char *string,
			   const char *delimiters, int max_tokens)
{
	unsigned int delim_table[256];
	struct SList *list, *tokens;
	int n_tokens;
	const char *s;
	const char *current;
	char *token;
	char **result;

	if (!string || !delimiters)
		return NULL;

	if (max_tokens < 1)
		max_tokens = INT_MAX;

	if (*string == '\0') {
		result = malloc(sizeof(char *));
		result[0] = NULL;
		return result;
	}

	memset(delim_table, 0, sizeof(delim_table));
	for (s = delimiters; *s != '\0'; ++s)
		delim_table[*(unsigned char *)s] = 1;

	tokens = NULL;
	n_tokens = 0;

	s = current = string;
	while (*s != '\0') {
		if (delim_table[*(unsigned char *)s]
		    && n_tokens + 1 < max_tokens) {
			token = strndup(current, s - current);
			tokens = slist_prepend(tokens, token);
			++n_tokens;

			current = s + 1;
		}

		++s;
	}

	token = strndup(current, s - current);
	tokens = slist_prepend(tokens, token);
	++n_tokens;

	result = malloc(sizeof(char *) * (n_tokens + 1));

	result[n_tokens] = NULL;
	for (list = tokens; list != NULL; list = list->next)
		result[--n_tokens] = list->data;

	list = tokens;
	while (list) {
		struct SList *old = list;

		list = list->next;
		free(old);
	}
	return result;
}

char **cp_get_group_kv_list(char *v)
{
	/*
	 * SMB conf lists are "tabs, spaces, commas" separated.
	 */
	return strsplit_set(v, "\t ,", -1);
}

void cp_group_kv_list_free(char **list)
{
	if (list) {
		int i;

		for (i = 0; list[i] != NULL; i++)
			free(list[i]);
		free(list);
	}
}

static int cp_add_global_guest_account(void *_v)
{
	struct usmbd_user *user;

	if (usm_add_new_user(cp_get_group_kv_string(_v), strdup("NULL"))) {
		pr_err("Unable to add guest account\n");
		return -ENOMEM;
	}

	user = usm_lookup_user(_v);
	if (!user) {
		pr_err("Fatal error: unable to find `%s' account.\n",
		       (const char *)_v);
		return -EINVAL;
	}

	set_user_flag(user, USMBD_USER_FLAG_GUEST_ACCOUNT);
	put_usmbd_user(user);
	global_conf.guest_account = cp_get_group_kv_string(_v);
	return 0;
}

static void global_group_kv(void *_v, unsigned long long id, void *user_data)
{
	void *_k = list_fromkey(id);

	if (!cp_key_cmp(_k, "server string")) {
		global_conf.server_string = cp_get_group_kv_string(_v);
		return;
	}

	if (!cp_key_cmp(_k, "workgroup")) {
		global_conf.work_group = cp_get_group_kv_string(_v);
		return;
	}

	if (!cp_key_cmp(_k, "netbios name")) {
		global_conf.netbios_name = cp_get_group_kv_string(_v);
		return;
	}

	if (!cp_key_cmp(_k, "server min protocol")) {
		global_conf.server_min_protocol = cp_get_group_kv_string(_v);
		return;
	}

	if (!cp_key_cmp(_k, "server signing")) {
		global_conf.server_signing = cp_get_group_kv_config_opt(_v);
		return;
	}

	if (!cp_key_cmp(_k, "server max protocol")) {
		global_conf.server_max_protocol = cp_get_group_kv_string(_v);
		return;
	}

	if (!cp_key_cmp(_k, "guest account")) {
		cp_add_global_guest_account(_v);
		return;
	}

	if (!cp_key_cmp(_k, "max active sessions")) {
		global_conf.sessions_cap = cp_get_group_kv_long(_v);
		return;
	}

	if (!cp_key_cmp(_k, "tcp port")) {
		if (!global_conf.tcp_port)
			global_conf.tcp_port = cp_get_group_kv_long(_v);
		return;
	}

	if (!cp_key_cmp(_k, "ipc timeout")) {
		global_conf.ipc_timeout = cp_get_group_kv_long(_v);
		return;
	}

	if (!cp_key_cmp(_k, "max open files")) {
		global_conf.file_max = cp_get_group_kv_long(_v);
		return;
	}

	if (!cp_key_cmp(_k, "restrict anonymous")) {
		global_conf.restrict_anon = cp_get_group_kv_long(_v);
		if (global_conf.restrict_anon > USMBD_RESTRICT_ANON_TYPE_2 ||
		    global_conf.restrict_anon < 0) {
			global_conf.restrict_anon = 0;
			pr_err("Invalid restrict anonymous value\n");
		}

		return;
	}

	if (!cp_key_cmp(_k, "map to guest")) {
		global_conf.map_to_guest = USMBD_CONF_MAP_TO_GUEST_NEVER;
		if (!cp_key_cmp(_v, "bad user"))
			global_conf.map_to_guest =
			    USMBD_CONF_MAP_TO_GUEST_BAD_USER;
		if (!cp_key_cmp(_v, "bad password"))
			global_conf.map_to_guest =
			    USMBD_CONF_MAP_TO_GUEST_BAD_PASSWORD;
		if (!cp_key_cmp(_v, "bad uid"))
			global_conf.map_to_guest =
			    USMBD_CONF_MAP_TO_GUEST_BAD_UID;
		return;
	}

	if (!cp_key_cmp(_k, "bind interfaces only")) {
		global_conf.bind_interfaces_only = cp_get_group_kv_bool(_v);
		return;
	}

	if (!cp_key_cmp(_k, "interfaces")) {
		global_conf.interfaces = cp_get_group_kv_list(_v);
		return;
	}

	if (!cp_key_cmp(_k, "deadtime")) {
		global_conf.deadtime = cp_get_group_kv_long(_v);
		return;
	}

	if (!cp_key_cmp(_k, "smb2 leases")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= USMBD_GLOBAL_FLAG_SMB2_LEASES;
		else
			global_conf.flags &= ~USMBD_GLOBAL_FLAG_SMB2_LEASES;

		return;
	}

	if (!cp_key_cmp(_k, "root directory")) {
		global_conf.root_dir = cp_get_group_kv_string(_v);
		return;
	}

	if (!cp_key_cmp(_k, "smb2 max read")) {
		global_conf.smb2_max_read = memparse(_v);
		return;
	}

	if (!cp_key_cmp(_k, "smb2 max write")) {
		global_conf.smb2_max_write = memparse(_v);
		return;
	}

	if (!cp_key_cmp(_k, "smb2 max trans")) {
		global_conf.smb2_max_trans = memparse(_v);
		return;
	}

	if (!cp_key_cmp(_k, "cache trans buffers")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= USMBD_GLOBAL_FLAG_CACHE_TBUF;
		else
			global_conf.flags &= ~USMBD_GLOBAL_FLAG_CACHE_TBUF;
		return;
	}

	if (!cp_key_cmp(_k, "cache read buffers")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= USMBD_GLOBAL_FLAG_CACHE_RBUF;
		else
			global_conf.flags &= ~USMBD_GLOBAL_FLAG_CACHE_RBUF;
		return;
	}

	if (!cp_key_cmp(_k, "smb3 encryption")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= USMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;
		else
			global_conf.flags &= ~USMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;

		return;
	}

	if (!cp_key_cmp(_k, "durable handle")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= USMBD_GLOBAL_FLAG_DURABLE_HANDLE;
		else
			global_conf.flags &= ~USMBD_GLOBAL_FLAG_DURABLE_HANDLE;

		return;
	}
}

static void fixup_missing_global_group(void)
{
	int ret;

	/*
	 * Set default global parameters which were not specified
	 * in smb.conf
	 */
	if (!global_conf.file_max)
		global_conf.file_max = USMBD_CONF_FILE_MAX;
	if (!global_conf.server_string)
		global_conf.server_string =
		    cp_get_group_kv_string(USMBD_CONF_DEFAULT_SERVER_STRING);
	if (!global_conf.netbios_name)
		global_conf.netbios_name =
		    cp_get_group_kv_string(USMBD_CONF_DEFAULT_NETBIOS_NAME);
	if (!global_conf.work_group)
		global_conf.work_group =
		    cp_get_group_kv_string(USMBD_CONF_DEFAULT_WORK_GROUP);
	if (!global_conf.tcp_port)
		global_conf.tcp_port = USMBD_CONF_DEFAULT_TPC_PORT;

	if (global_conf.sessions_cap <= 0)
		global_conf.sessions_cap = USMBD_CONF_DEFAULT_SESS_CAP;

	if (global_conf.guest_account)
		return;

	ret = cp_add_global_guest_account(USMBD_CONF_DEFAULT_GUEST_ACCOUNT);
	if (!ret)
		return;
	ret = cp_add_global_guest_account(USMBD_CONF_FALLBACK_GUEST_ACCOUNT);
	if (ret)
		pr_err("Fatal error: Cannot set a global guest account %d\n",
		       ret);
}

static void default_global_group(void)
{
	global_conf.flags |= USMBD_GLOBAL_FLAG_CACHE_TBUF;
	global_conf.flags |= USMBD_GLOBAL_FLAG_CACHE_RBUF;
}

static void global_group(struct smbconf_group *group)
{
	list_foreach(&group->kv, global_group_kv, NULL);
}

#define GROUPS_CALLBACK_STARTUP_INIT	0x1
#define GROUPS_CALLBACK_REINIT		0x2

static void groups_callback(void *_v, unsigned long long id, void *flags)
{
	void *_k = list_fromkey(id);

	if (strncasecmp(_k, "global", 6)) {
		shm_add_new_share((struct smbconf_group *)_v);
		return;
	}

	if (flags == (void *)GROUPS_CALLBACK_STARTUP_INIT)
		global_group((struct smbconf_group *)_v);
}

static int cp_add_ipc_share(void)
{
	char *comment = NULL;
	int ret = 0;

	if (list_get(&parser.groups, list_tokey("ipc$")))
		return 0;

	comment = strdup("comment = IPC share");
	ret = add_new_group("[IPC$]");
	ret |= add_group_key_value(comment);
	if (ret) {
		pr_err("Unable to add IPC$ share\n");
		ret = -EINVAL;
		goto out;
	}
	return ret;

out:
	free(comment);
	return ret;
}

static int __cp_parse_smbconfig(const char *smbconf, long flags)
{
	int ret;

	default_global_group();

	ret = cp_smbconfig_hash_create(smbconf);
	if (ret)
		return ret;

	ret = cp_add_ipc_share();
	if (!ret) {
		list_foreach(&parser.groups, groups_callback, (void *)flags);
		fixup_missing_global_group();
	}
	cp_smbconfig_destroy();
	return ret;
}

int cp_parse_reload_smbconf(const char *smbconf)
{
	return __cp_parse_smbconfig(smbconf, GROUPS_CALLBACK_REINIT);
}

int cp_parse_smbconf(const char *smbconf)
{
	return __cp_parse_smbconfig(smbconf, GROUPS_CALLBACK_STARTUP_INIT);
}

int cp_parse_pwddb(const char *pwddb)
{
	return __mmap_parse_file(pwddb, usm_add_update_user_from_pwdentry);
}

int cp_smbconfig_hash_create(const char *smbconf)
{
	int ret = init_smbconf_parser();

	if (ret)
		return ret;
	return __mmap_parse_file(smbconf, process_smbconf_entry);
}

void cp_smbconfig_destroy(void)
{
	release_smbconf_parser();
}

int cp_parse_external_smbconf_group(char *name, char *opts)
{
	char *delim = opts;
	char *pos;
	int i, len;

	if (init_smbconf_parser())
		return -EINVAL;

	if (!opts || !name)
		return -EINVAL;

	len = strlen(opts);
	/* fake smb.conf input */
	for (i = 0; i < USMBD_SHARE_CONF_MAX; i++) {
		pos = strstr(opts, USMBD_SHARE_CONF[i]);
		if (!pos)
			continue;
		if (pos != opts)
			*(pos - 1) = '\n';
	}

	if (add_new_group(name))
		goto error;

	/* split input and feed to normal process_smbconf_entry() */
	while (len) {
		char *delim = strchr(opts, '\n');

		if (delim) {
			*delim = 0x00;
			len -= delim - opts;
		} else {
			len = 0;
		}

		process_smbconf_entry(opts);
		if (delim)
			opts = delim + 1;
	}
	return 0;

error:
	cp_smbconfig_destroy();
	return -EINVAL;
}
