/* SPDX-License-Identifier: GPL-2.0-or-later */
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
	USMBD_SHARE_ADMIN_USERS_MAP = 0,
	/* Valid users */
	USMBD_SHARE_VALID_USERS_MAP,
	/* Invalid users */
	USMBD_SHARE_INVALID_USERS_MAP,
	/* Read-only users */
	USMBD_SHARE_READ_LIST_MAP,
	/* Read/Write access to a read-only share */
	USMBD_SHARE_WRITE_LIST_MAP,
	USMBD_SHARE_USERS_MAX,
};

enum share_hosts {
	USMBD_SHARE_HOSTS_ALLOW_MAP = 0,
	USMBD_SHARE_HOSTS_DENY_MAP,
	USMBD_SHARE_HOSTS_MAX,
};

#define USMBD_SHARE_DEFAULT_CREATE_MASK	0744
#define USMBD_SHARE_DEFAULT_DIRECTORY_MASK	0755

#define USMBD_SHARE_DEFAULT_UID		0
#define USMBD_SHARE_DEFAULT_GID		0

struct usmbd_share {
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

	GHashTable	*maps[USMBD_SHARE_USERS_MAX];
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

	char		*comment;
};

/*
 * WARNING:
 *
 * Add new entries only before to the bottom, right before
 * USMBD_SHARE_CONF_MAX. See SHARE_CONF comment.
 *
 */
enum USMBD_SHARE_CONF {
	USMBD_SHARE_CONF_COMMENT			= 0,
	USMBD_SHARE_CONF_PATH,
	USMBD_SHARE_CONF_GUEST_OK,
	USMBD_SHARE_CONF_GUEST_ACCOUNT,
	USMBD_SHARE_CONF_READ_ONLY,
	USMBD_SHARE_CONF_BROWSEABLE			= 5,
	USMBD_SHARE_CONF_WRITE_OK,
	USMBD_SHARE_CONF_WRITEABLE,
	USMBD_SHARE_CONF_STORE_DOS_ATTRIBUTES,
	USMBD_SHARE_CONF_OPLOCKS,
	USMBD_SHARE_CONF_CREATE_MASK			= 10,
	USMBD_SHARE_CONF_DIRECTORY_MASK,
	USMBD_SHARE_CONF_FORCE_CREATE_MODE,
	USMBD_SHARE_CONF_FORCE_DIRECTORY_MODE,
	USMBD_SHARE_CONF_FORCE_GROUP,
	USMBD_SHARE_CONF_FORCE_USER			= 15,
	USMBD_SHARE_CONF_HIDE_DOT_FILES,
	USMBD_SHARE_CONF_VALID_USERS,
	USMBD_SHARE_CONF_INVALID_USERS,
	USMBD_SHARE_CONF_READ_LIST,
	USMBD_SHARE_CONF_WRITE_LIST			= 20,
	USMBD_SHARE_CONF_ADMIN_USERS,
	USMBD_SHARE_CONF_HOSTS_ALLOW,
	USMBD_SHARE_CONF_HOSTS_DENY,
	USMBD_SHARE_CONF_MAX_CONNECTIONS,
	USMBD_SHARE_CONF_VETO_FILES			= 25,
	USMBD_SHARE_CONF_INHERIT_SMACK,
	USMBD_SHARE_CONF_INHERIT_OWNER,
	USMBD_SHARE_CONF_STREAMS,
	USMBD_SHARE_CONF_MAX
};

extern char *USMBD_SHARE_CONF[USMBD_SHARE_CONF_MAX];

int shm_share_config(char *k, enum USMBD_SHARE_CONF c);

static inline void set_share_flag(struct usmbd_share *share, int flag)
{
	share->flags |= flag;
}

static inline void clear_share_flag(struct usmbd_share *share, int flag)
{
	share->flags &= ~flag;
}

static inline int test_share_flag(struct usmbd_share *share, int flag)
{
	return share->flags & flag;
}

struct usmbd_share *get_usmbd_share(struct usmbd_share *share);
void put_usmbd_share(struct usmbd_share *share);
struct usmbd_share *shm_lookup_share(char *name);

struct smbconf_group;
int shm_add_new_share(struct smbconf_group *group);

void shm_destroy(void);
int shm_init(void);

int shm_lookup_users_map(struct usmbd_share *share,
			  enum share_users map,
			  char *name);

int shm_lookup_hosts_map(struct usmbd_share *share,
			  enum share_hosts map,
			  char *host);

int shm_open_connection(struct usmbd_share *share);
int shm_close_connection(struct usmbd_share *share);

typedef void (*walk_shares)(gpointer key,
			    gpointer value,
			    gpointer user_data);
void for_each_usmbd_share(walk_shares cb, gpointer user_data);

struct usmbd_share_config_response;

int shm_share_config_payload_size(struct usmbd_share *share);
int shm_handle_share_config_request(struct usmbd_share *share,
				    struct usmbd_share_config_response *resp);

#endif /* __MANAGEMENT_SHARE_H__ */
