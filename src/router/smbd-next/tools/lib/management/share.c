// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <config_parser.h>
#include <linux/ksmbd_server.h>

#include <management/share.h>
#include <management/user.h>
#include <ksmbdtools.h>

#define pthread_rwlock_unlock(a)
#define pthread_rwlock_wrlock(a)
#define pthread_rwlock_rdlock(a)
#define pthread_rwlock_init(a,b)
#define pthread_rwlock_destroy(a)

#define KSMBD_SHARE_STATE_FREEING	1

/*
 * WARNING:
 *
 * This must match KSMBD_SHARE_CONF enum 1:1.
 * Add new entries ONLY to the bottom.
 */
char *KSMBD_SHARE_CONF[KSMBD_SHARE_CONF_MAX] = {
	"comment",		/* 0 */
	"path",
	"guest ok",
	"guest account",
	"read only",
	"browseable",		/* 5 */
	"write ok",
	"writeable",
	"store dos attributes",
	"oplocks",
	"create mask",		/* 10 */
	"directory mask",
	"force create mode",
	"force directory mode",
	"force group",
	"force user",		/* 15 */
	"hide dot files",
	"valid users",
	"invalid users",
	"read list",
	"write list",		/* 20 */
	"admin users",
	"hosts allow",
	"hosts deny",
	"max connections",
	"veto files",		/* 25 */
	"inherit smack",
	"inherit owner",
	"streams",
	"follow symlinks",
};

static struct LIST *shares_table;
static pthread_rwlock_t shares_table_lock;

int shm_share_config(char *k, enum KSMBD_SHARE_CONF c)
{
	if (c >= KSMBD_SHARE_CONF_MAX)
		return 0;

	return !cp_key_cmp(k, KSMBD_SHARE_CONF[c]);
}

static void list_hosts_callback(void *item, unsigned long long id,
				void *user_data)
{
	free(item);
	free(list_fromkey(id));
}

static void free_hosts_map(struct LIST *map)
{
	if (map) {
		list_foreach(&map, list_hosts_callback, NULL);
		list_clear(&map);
	}
}

static void list_user_callback(void *item, unsigned long long id,
			       void *user_data)
{
	put_ksmbd_user((struct ksmbd_user *)item);
}

static void free_user_map(struct LIST *map)
{
	if (map) {
		list_foreach(&map, list_user_callback, NULL);
		list_clear(&map);
	}
}

static void kill_ksmbd_share(struct ksmbd_share *share)
{
	int i;

	pr_debug("Kill share %s\n", share->name);

	for (i = 0; i < KSMBD_SHARE_USERS_MAX; i++)
		free_user_map(share->maps[i]);

	free_hosts_map(share->hosts_allow_map);
	free_hosts_map(share->hosts_deny_map);

	pthread_rwlock_destroy(&share->maps_lock);

	free(share->name);
	free(share->path);
	free(share->comment);
	free(share->veto_list);
	free(share->guest_account);
	pthread_rwlock_destroy(&share->update_lock);
	free(share);
}

static int __shm_remove_share(struct ksmbd_share *share)
{
	int ret = 0;

	if (share->state != KSMBD_SHARE_STATE_FREEING) {
		pthread_rwlock_wrlock(&shares_table_lock);
		if (!list_remove(&shares_table, list_tokey(share->name)))
			ret = -EINVAL;
		pthread_rwlock_unlock(&shares_table_lock);
	}

	if (!ret)
		kill_ksmbd_share(share);
	return ret;
}

struct ksmbd_share *get_ksmbd_share(struct ksmbd_share *share)
{
	pthread_rwlock_wrlock(&share->update_lock);
	if (share->ref_count != 0) {
		share->ref_count++;
		pthread_rwlock_unlock(&share->update_lock);
	} else {
		pthread_rwlock_unlock(&share->update_lock);
		share = NULL;
	}

	return share;
}

void put_ksmbd_share(struct ksmbd_share *share)
{
	int drop;

	if (!share)
		return;

	pthread_rwlock_wrlock(&share->update_lock);
	share->ref_count--;
	drop = !share->ref_count;
	pthread_rwlock_unlock(&share->update_lock);

	if (!drop)
		return;

	__shm_remove_share(share);
}

static void put_share_callback(void *item, unsigned long long id, void *user_data)
{
	struct ksmbd_share *share = (struct ksmbd_share *)item;

	share->state = KSMBD_SHARE_STATE_FREEING;
	put_ksmbd_share(share);
}

void shm_remove_all_shares(void)
{
	pthread_rwlock_wrlock(&shares_table_lock);
	list_foreach(&shares_table, put_share_callback, NULL);
	pthread_rwlock_unlock(&shares_table_lock);
}

static struct ksmbd_share *new_ksmbd_share(void)
{
	struct ksmbd_share *share;
	int i;

	share = calloc(1, sizeof(struct ksmbd_share));
	if (!share)
		return NULL;

	share->ref_count = 1;
	/*
	 * Create maps as needed. NULL maps means that share
	 * does not have a corresponding shmbconf entry.
	 */
	for (i = 0; i < KSMBD_SHARE_USERS_MAX; i++)
		share->maps[i] = NULL;

	share->hosts_allow_map = NULL;
	share->hosts_deny_map = NULL;
	pthread_rwlock_init(&share->maps_lock, NULL);
	pthread_rwlock_init(&share->update_lock, NULL);

	return share;
}

static void free_hash_entry(void *item, unsigned long long id, void *user_data)
{
	kill_ksmbd_share(item);
}

static void shm_clear_shares(void)
{
	list_foreach(&shares_table, free_hash_entry, NULL);
}

void shm_destroy(void)
{
	if (shares_table) {
		shm_clear_shares();
		list_clear(&shares_table);
	}
	pthread_rwlock_destroy(&shares_table_lock);
}

int shm_init(void)
{
	list_init(&shares_table);
	if (!shares_table)
		return -ENOMEM;
	pthread_rwlock_init(&shares_table_lock, NULL);
	return 0;
}

static struct ksmbd_share *__shm_lookup_share(char *name)
{
	return (struct ksmbd_share *)list_get(&shares_table, list_tokey(name));
}

struct ksmbd_share *shm_lookup_share(char *name)
{
	struct ksmbd_share *share, *ret;

	pthread_rwlock_rdlock(&shares_table_lock);
	share = __shm_lookup_share(name);
	if (share) {
		ret = get_ksmbd_share(share);
		if (!ret)
			share = NULL;
	}
	pthread_rwlock_unlock(&shares_table_lock);
	return share;
}

static struct LIST *parse_list(struct LIST *map, char **list)
{
	int i;

	if (!list)
		return map;

	if (!map)
		list_init(&map);
	if (!map)
		return map;

	for (i = 0; list[i] != NULL; i++) {
		struct ksmbd_user *user;
		char *p = list[i];

		p = cp_ltrim(p);
		if (!p)
			continue;

		user = usm_lookup_user(p);
		if (!user) {
			pr_info("Drop non-existing user `%s'\n", p);
			continue;
		}

		if (list_get(&map, list_tokey(user->name))) {
			pr_debug("User already exists in a map: %s\n", p);
			continue;
		}

		list_add_str(&map, user, user->name);
	}

	cp_group_kv_list_free(list);
	return map;
}

static void make_veto_list(struct ksmbd_share *share)
{
	int i;

	for (i = 0; i < share->veto_list_sz; i++) {
		if (share->veto_list[i] == '/')
			share->veto_list[i] = 0x00;
	}
}

static void force_group(struct ksmbd_share *share, char *name)
{
	struct group *grp;

	grp = getgrnam(name);
	if (grp) {
		share->force_gid = grp->gr_gid;
		if (share->force_gid == KSMBD_SHARE_INVALID_GID)
			pr_err("Invalid force gid: %u\n", share->force_gid);
	} else
		pr_err("Unable to lookup up /etc/group entry: %s\n", name);
}

static void force_user(struct ksmbd_share *share, char *name)
{
	struct passwd *passwd;

	passwd = getpwnam(name);
	if (passwd) {
		share->force_uid = passwd->pw_uid;
		/*
		 * smb.conf 'force group' has higher priority than
		 * 'force user'.
		 */
		if (share->force_gid == KSMBD_SHARE_INVALID_GID)
			share->force_gid = passwd->pw_gid;
		if (share->force_uid == KSMBD_SHARE_INVALID_UID ||
				share->force_gid == KSMBD_SHARE_INVALID_GID)
			pr_err("Invalid force uid / gid: %u / %u\n",
					share->force_uid, share->force_gid);
	} else {
		pr_err("Unable to lookup up /etc/passwd entry: %s\n", name);
	}
}

static void process_group_kv(void *_v, unsigned long long _k, void *user_data)
{
	struct ksmbd_share *share = user_data;
	char *k = (char *)list_fromkey(_k);
	char *v = (char *)_v;

	if (shm_share_config(k, KSMBD_SHARE_CONF_COMMENT)) {
		share->comment = cp_get_group_kv_string(v);
		if (share->comment == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_PATH)) {
		share->path = cp_get_group_kv_string(v);
		if (share->path == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_GUEST_OK)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_GUEST_OK);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_GUEST_ACCOUNT)) {
		struct ksmbd_user *user;

		if (usm_add_new_user(cp_get_group_kv_string(_v),
				     strdup("NULL"))) {
			pr_err("Unable to add guest account\n");
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
			return;
		}

		user = usm_lookup_user(_v);
		if (user) {
			set_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT);
			put_ksmbd_user(user);
		}
		share->guest_account = cp_get_group_kv_string(_v);
		if (!share->guest_account)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_READ_ONLY)) {
		if (cp_get_group_kv_bool(v)) {
			set_share_flag(share, KSMBD_SHARE_FLAG_READONLY);
			clear_share_flag(share, KSMBD_SHARE_FLAG_WRITEABLE);
		} else {
			clear_share_flag(share, KSMBD_SHARE_FLAG_READONLY);
			set_share_flag(share, KSMBD_SHARE_FLAG_WRITEABLE);
		}
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_BROWSEABLE)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_BROWSEABLE);
		else
			clear_share_flag(share, KSMBD_SHARE_FLAG_BROWSEABLE);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_WRITE_OK) ||
	    shm_share_config(k, KSMBD_SHARE_CONF_WRITEABLE)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_WRITEABLE);
		else
			clear_share_flag(share, KSMBD_SHARE_FLAG_WRITEABLE);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_STORE_DOS_ATTRIBUTES)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_STORE_DOS_ATTRS);
		else
			clear_share_flag(share,
					 KSMBD_SHARE_FLAG_STORE_DOS_ATTRS);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_OPLOCKS)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_OPLOCKS);
		else
			clear_share_flag(share, KSMBD_SHARE_FLAG_OPLOCKS);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_CREATE_MASK)) {
		share->create_mask = cp_get_group_kv_long_base(v, 8);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_DIRECTORY_MASK)) {
		share->directory_mask = cp_get_group_kv_long_base(v, 8);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_FORCE_CREATE_MODE)) {
		share->force_create_mode = cp_get_group_kv_long_base(v, 8);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_FORCE_DIRECTORY_MODE)) {
		share->force_directory_mode = cp_get_group_kv_long_base(v, 8);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_FORCE_GROUP)) {
		force_group(share, v);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_FORCE_USER)) {
		force_user(share, v);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_HIDE_DOT_FILES)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_HIDE_DOT_FILES);
		else
			clear_share_flag(share,
					 KSMBD_SHARE_FLAG_HIDE_DOT_FILES);
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_VALID_USERS)) {
		share->maps[KSMBD_SHARE_VALID_USERS_MAP] =
		    parse_list(share->maps[KSMBD_SHARE_VALID_USERS_MAP],
			       cp_get_group_kv_list(v));
		if (share->maps[KSMBD_SHARE_VALID_USERS_MAP] == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_INVALID_USERS)) {
		share->maps[KSMBD_SHARE_INVALID_USERS_MAP] =
		    parse_list(share->maps[KSMBD_SHARE_INVALID_USERS_MAP],
			       cp_get_group_kv_list(v));
		if (share->maps[KSMBD_SHARE_INVALID_USERS_MAP] == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_READ_LIST)) {
		share->maps[KSMBD_SHARE_READ_LIST_MAP] =
		    parse_list(share->maps[KSMBD_SHARE_READ_LIST_MAP],
			       cp_get_group_kv_list(v));
		if (share->maps[KSMBD_SHARE_READ_LIST_MAP] == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_WRITE_LIST)) {
		share->maps[KSMBD_SHARE_WRITE_LIST_MAP] =
		    parse_list(share->maps[KSMBD_SHARE_WRITE_LIST_MAP],
			       cp_get_group_kv_list(v));
		if (share->maps[KSMBD_SHARE_WRITE_LIST_MAP] == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_ADMIN_USERS)) {
		share->maps[KSMBD_SHARE_ADMIN_USERS_MAP] =
		    parse_list(share->maps[KSMBD_SHARE_ADMIN_USERS_MAP],
			       cp_get_group_kv_list(v));
		if (share->maps[KSMBD_SHARE_ADMIN_USERS_MAP] == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_HOSTS_ALLOW)) {
		share->hosts_allow_map = parse_list(share->hosts_allow_map,
						    cp_get_group_kv_list(v));
		if (share->hosts_allow_map == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_HOSTS_DENY)) {
		share->hosts_deny_map = parse_list(share->hosts_deny_map,
						   cp_get_group_kv_list(v));
		if (share->hosts_deny_map == NULL)
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_MAX_CONNECTIONS)) {
		share->max_connections = cp_get_group_kv_long_base(v, 10);
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_VETO_FILES)) {
		share->veto_list = cp_get_group_kv_string(v + 1);
		if (share->veto_list == NULL) {
			set_share_flag(share, KSMBD_SHARE_FLAG_INVALID);
		} else {
			share->veto_list_sz = strlen(share->veto_list);
			make_veto_list(share);
		}
		return;
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_INHERIT_SMACK)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_INHERIT_SMACK);
		else
			clear_share_flag(share, KSMBD_SHARE_FLAG_INHERIT_SMACK);
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_INHERIT_OWNER)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_INHERIT_OWNER);
		else
			clear_share_flag(share, KSMBD_SHARE_FLAG_INHERIT_OWNER);
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_STREAMS)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_STREAMS);
		else
			clear_share_flag(share, KSMBD_SHARE_FLAG_STREAMS);
	}

	if (shm_share_config(k, KSMBD_SHARE_CONF_FOLLOW_SYMLINKS)) {
		if (cp_get_group_kv_bool(v))
			set_share_flag(share, KSMBD_SHARE_FLAG_FOLLOW_SYMLINKS);
		else
			clear_share_flag(share,
				KSMBD_SHARE_FLAG_FOLLOW_SYMLINKS);
	}
}

static void init_share_from_group(struct ksmbd_share *share,
				  struct smbconf_group *group)
{
	share->name = strdup(group->name);
	share->create_mask = KSMBD_SHARE_DEFAULT_CREATE_MASK;
	share->directory_mask = KSMBD_SHARE_DEFAULT_DIRECTORY_MASK;
	share->force_create_mode = 0;
	share->force_directory_mode = 0;

	share->force_uid = KSMBD_SHARE_INVALID_UID;
	share->force_gid = KSMBD_SHARE_INVALID_GID;

	set_share_flag(share, KSMBD_SHARE_FLAG_AVAILABLE);
	set_share_flag(share, KSMBD_SHARE_FLAG_BROWSEABLE);
	set_share_flag(share, KSMBD_SHARE_FLAG_READONLY);
	set_share_flag(share, KSMBD_SHARE_FLAG_HIDE_DOT_FILES);
	set_share_flag(share, KSMBD_SHARE_FLAG_OPLOCKS);
	set_share_flag(share, KSMBD_SHARE_FLAG_FOLLOW_SYMLINKS);

	if (!cp_key_cmp(share->name, "IPC$"))
		set_share_flag(share, KSMBD_SHARE_FLAG_PIPE);

	list_foreach(&(group->kv), process_group_kv, share);
}

int shm_add_new_share(struct smbconf_group *group)
{
	int ret = 0;
	struct ksmbd_share *share = new_ksmbd_share();

	if (!share)
		return -ENOMEM;

	init_share_from_group(share, group);
	if (test_share_flag(share, KSMBD_SHARE_FLAG_INVALID)) {
		pr_err("Invalid share %s\n", share->name);
		kill_ksmbd_share(share);
		return 0;
	}

	pthread_rwlock_wrlock(&shares_table_lock);
	if (__shm_lookup_share(share->name)) {
		pthread_rwlock_unlock(&shares_table_lock);
		pr_info("share exists %s\n", share->name);
		kill_ksmbd_share(share);
		return 0;
	}

	if (!list_add_str(&shares_table, share, share->name)) {
		kill_ksmbd_share(share);
		ret = -EINVAL;
	}
	pthread_rwlock_unlock(&shares_table_lock);
	return ret;
}

int shm_lookup_users_map(struct ksmbd_share *share,
			 enum share_users map, char *name)
{
	int ret = -ENOENT;

	if (map >= KSMBD_SHARE_USERS_MAX) {
		pr_err("Invalid users map index: %d\n", map);
		return 0;
	}

	if (!share->maps[map])
		return -EINVAL;

	pthread_rwlock_rdlock(&share->maps_lock);
	if (list_get(&share->maps[map], list_tokey(name)))
		ret = 0;
	pthread_rwlock_unlock(&share->maps_lock);

	return ret;
}

/*
 * FIXME
 * Do a real hosts lookup. IP masks, etc.
 */
int shm_lookup_hosts_map(struct ksmbd_share *share,
			 enum share_hosts map, char *host)
{
	struct LIST *lookup_map;
	int ret = -ENOENT;

	if (map >= KSMBD_SHARE_HOSTS_MAX) {
		pr_err("Invalid hosts map index: %d\n", map);
		return 0;
	}

	if (map == KSMBD_SHARE_HOSTS_ALLOW_MAP)
		lookup_map = share->hosts_allow_map;
	if (map == KSMBD_SHARE_HOSTS_DENY_MAP)
		lookup_map = share->hosts_deny_map;

	if (!lookup_map)
		return -EINVAL;

	pthread_rwlock_rdlock(&share->maps_lock);
	if (list_get(&lookup_map, list_tokey(host)))
		ret = 0;
	pthread_rwlock_unlock(&share->maps_lock);

	return ret;
}

int shm_open_connection(struct ksmbd_share *share)
{
	int ret = 0;

	pthread_rwlock_wrlock(&share->update_lock);
	share->num_connections++;
	if (share->max_connections) {
		if (share->num_connections >= share->max_connections)
			ret = -EINVAL;
	}
	pthread_rwlock_unlock(&share->update_lock);
	return ret;
}

int shm_close_connection(struct ksmbd_share *share)
{
	if (!share)
		return 0;

	pthread_rwlock_wrlock(&share->update_lock);
	share->num_connections--;
	pthread_rwlock_unlock(&share->update_lock);
	return 0;
}

void foreach_ksmbd_share(walk_shares cb, void *user_data)
{
	pthread_rwlock_rdlock(&shares_table_lock);
	list_foreach(&shares_table, cb, user_data);
	pthread_rwlock_unlock(&shares_table_lock);
}

int shm_share_config_payload_size(struct ksmbd_share *share)
{
	int sz = 1;

	if (share && !test_share_flag(share, KSMBD_SHARE_FLAG_PIPE)) {
		if (share->path)
			sz += strlen(share->path);
		if (global_conf.root_dir)
			sz += strlen(global_conf.root_dir) + 1;
		if (share->veto_list_sz)
			sz += share->veto_list_sz + 1;
	}

	return sz;
}

int shm_handle_share_config_request(struct ksmbd_share *share,
				    struct ksmbd_share_config_response *resp)
{
	unsigned char *config_payload;

	if (!share)
		return -EINVAL;

	resp->flags = share->flags;
	resp->create_mask = share->create_mask;
	resp->directory_mask = share->directory_mask;
	resp->force_create_mode = share->force_create_mode;
	resp->force_directory_mode = share->force_directory_mode;
	resp->force_uid = share->force_uid;
	resp->force_gid = share->force_gid;
	resp->veto_list_sz = share->veto_list_sz;

	if (test_share_flag(share, KSMBD_SHARE_FLAG_PIPE))
		return 0;

	if (!share->path)
		return 0;

	config_payload = KSMBD_SHARE_CONFIG_VETO_LIST(resp);
	if (resp->veto_list_sz) {
		memcpy(config_payload, share->veto_list, resp->veto_list_sz);
		config_payload += resp->veto_list_sz + 1;
	}
	if (global_conf.root_dir)
		sprintf(config_payload,
			"%s%s", global_conf.root_dir, share->path);
	else
		sprintf(config_payload, "%s", share->path);
	return 0;
}
