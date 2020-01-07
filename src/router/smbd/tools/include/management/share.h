// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __MANAGEMENT_SHARE_H__
#define __MANAGEMENT_SHARE_H__

#include <glib.h>


enum share_users {
	/* Admin users */
	SMBD_SHARE_ADMIN_USERS_MAP = 0,
	/* Valid users */
	SMBD_SHARE_VALID_USERS_MAP,
	/* Invalid users */
	SMBD_SHARE_INVALID_USERS_MAP,
	/* Read-only users */
	SMBD_SHARE_READ_LIST_MAP,
	/* Read/Write access to a read-only share */
	SMBD_SHARE_WRITE_LIST_MAP,
	SMBD_SHARE_USERS_MAX,
};

enum share_hosts {
	SMBD_SHARE_HOSTS_ALLOW_MAP = 0,
	SMBD_SHARE_HOSTS_DENY_MAP,
	SMBD_SHARE_HOSTS_MAX,
};

#define SMBD_SHARE_DEFAULT_CREATE_MASK	0744
#define SMBD_SHARE_DEFAULT_DIRECTORY_MASK	0755

#define SMBD_SHARE_DEFAULT_UID		0
#define SMBD_SHARE_DEFAULT_GID		0

struct smbd_share {
	char		*name;
	char		*path;

	int		max_connections;
	int		num_connections;

	GRWLock		update_lock;
	int		ref_count;

	unsigned short	create_mask;
	unsigned short	directory_mask;
	unsigned short	force_create_mode;
	unsigned short	force_directory_mode;
	unsigned short	force_uid;
	unsigned short	force_gid;

	int		flags;

	char		*veto_list;
	int		veto_list_sz;

	char		*guest_account;

	GHashTable	*maps[SMBD_SHARE_USERS_MAX];
	/*
	 * FIXME
	 * We need to support IP ranges, netmasks, etc.
	 * This is just a silly hostname matching, hence
	 * these two are not in ->maps[].
	 */
	GHashTable	*hosts_allow_map;
	/* Deny access */
	GHashTable	*hosts_deny_map;

	/* One lock to rule them all [as of now] */
	GRWLock		maps_lock;

	char*		comment;
};

/*
 * WARNING:
 *
 * Add new entries only before to the bottom, right before
 * SMBD_SHARE_CONF_MAX. See SHARE_CONF comment.
 *
 */
enum SMBD_SHARE_CONF {
	SMBD_SHARE_CONF_COMMENT			= 0,
	SMBD_SHARE_CONF_PATH,
	SMBD_SHARE_CONF_GUEST_OK,
	SMBD_SHARE_CONF_GUEST_ACCOUNT,
	SMBD_SHARE_CONF_READ_ONLY,
	SMBD_SHARE_CONF_BROWSEABLE			= 5,
	SMBD_SHARE_CONF_WRITE_OK,
	SMBD_SHARE_CONF_WRITEABLE,
	SMBD_SHARE_CONF_STORE_DOS_ATTRIBUTES,
	SMBD_SHARE_CONF_OPLOCKS,
	SMBD_SHARE_CONF_CREATE_MASK			= 10,
	SMBD_SHARE_CONF_DIRECTORY_MASK,
	SMBD_SHARE_CONF_FORCE_CREATE_MODE,
	SMBD_SHARE_CONF_FORCE_DIRECTORY_MODE,
	SMBD_SHARE_CONF_FORCE_GROUP,
	SMBD_SHARE_CONF_FORCE_USER			= 15,
	SMBD_SHARE_CONF_HIDE_DOT_FILES,
	SMBD_SHARE_CONF_VALID_USERS,
	SMBD_SHARE_CONF_INVALID_USERS,
	SMBD_SHARE_CONF_READ_LIST,
	SMBD_SHARE_CONF_WRITE_LIST			= 20,
	SMBD_SHARE_CONF_ADMIN_USERS,
	SMBD_SHARE_CONF_HOSTS_ALLOW,
	SMBD_SHARE_CONF_HOSTS_DENY,
	SMBD_SHARE_CONF_MAX_CONNECTIONS,
	SMBD_SHARE_CONF_VETO_FILES			= 25,
	SMBD_SHARE_CONF_INHERIT_SMACK,
	SMBD_SHARE_CONF_INHERIT_OWNER,
	SMBD_SHARE_CONF_STREAMS,
	SMBD_SHARE_CONF_MAX
};

extern char *SMBD_SHARE_CONF[SMBD_SHARE_CONF_MAX];

int shm_share_config(char *k, enum SMBD_SHARE_CONF c);

static inline void set_share_flag(struct smbd_share *share, int flag)
{
	share->flags |= flag;
}

static inline void clear_share_flag(struct smbd_share *share, int flag)
{
	share->flags &= ~flag;
}

static inline int test_share_flag(struct smbd_share *share, int flag)
{
	return share->flags & flag;
}

struct smbd_share *get_smbd_share(struct smbd_share *share);
void put_smbd_share(struct smbd_share *share);
struct smbd_share *shm_lookup_share(char *name);

struct smbconf_group;
int shm_add_new_share(struct smbconf_group *group);

void shm_destroy(void);
int shm_init(void);

int shm_lookup_users_map(struct smbd_share *share,
			  enum share_users map,
			  char *name);

int shm_lookup_hosts_map(struct smbd_share *share,
			  enum share_hosts map,
			  char *host);

int shm_open_connection(struct smbd_share *share);
int shm_close_connection(struct smbd_share *share);

typedef void (*walk_shares)(gpointer key,
			    gpointer value,
			    gpointer user_data);
void for_each_smbd_share(walk_shares cb, gpointer user_data);

struct smbd_share_config_response;

int shm_share_config_payload_size(struct smbd_share *share);
int shm_handle_share_config_request(struct smbd_share *share,
				    struct smbd_share_config_response *resp);

#endif /* __MANAGEMENT_SHARE_H__ */
