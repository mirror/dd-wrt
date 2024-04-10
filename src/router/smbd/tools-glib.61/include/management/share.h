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
	KSMBD_SHARE_ADMIN_USERS_MAP = 0,
	/* Valid users */
	KSMBD_SHARE_VALID_USERS_MAP,
	/* Invalid users */
	KSMBD_SHARE_INVALID_USERS_MAP,
	/* Read-only users */
	KSMBD_SHARE_READ_LIST_MAP,
	/* Read/Write access to a read-only share */
	KSMBD_SHARE_WRITE_LIST_MAP,
	KSMBD_SHARE_USERS_MAX,
};

enum share_hosts {
	KSMBD_SHARE_HOSTS_ALLOW_MAP = 0,
	KSMBD_SHARE_HOSTS_DENY_MAP,
	KSMBD_SHARE_HOSTS_MAX,
};

#define KSMBD_SHARE_INVALID_UID		((__u16)-1)
#define KSMBD_SHARE_INVALID_GID		((__u16)-1)

struct ksmbd_share {
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
	int		state;

	char		*veto_list;
	int		veto_list_sz;

	char		*guest_account;

	GHashTable	*maps[KSMBD_SHARE_USERS_MAX];
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
 * KSMBD_SHARE_CONF_MAX. See SHARE_CONF comment.
 *
 */
enum KSMBD_SHARE_CONF {
	KSMBD_SHARE_CONF_COMMENT			= 0,
	KSMBD_SHARE_CONF_PATH,
	KSMBD_SHARE_CONF_GUEST_OK,
	KSMBD_SHARE_CONF_GUEST_ACCOUNT,
	KSMBD_SHARE_CONF_READ_ONLY,
	KSMBD_SHARE_CONF_BROWSEABLE			= 5,
	KSMBD_SHARE_CONF_WRITE_OK,
	KSMBD_SHARE_CONF_WRITEABLE,
	KSMBD_SHARE_CONF_STORE_DOS_ATTRIBUTES,
	KSMBD_SHARE_CONF_OPLOCKS,
	KSMBD_SHARE_CONF_CREATE_MASK			= 10,
	KSMBD_SHARE_CONF_DIRECTORY_MASK,
	KSMBD_SHARE_CONF_FORCE_CREATE_MODE,
	KSMBD_SHARE_CONF_FORCE_DIRECTORY_MODE,
	KSMBD_SHARE_CONF_FORCE_GROUP,
	KSMBD_SHARE_CONF_FORCE_USER			= 15,
	KSMBD_SHARE_CONF_HIDE_DOT_FILES,
	KSMBD_SHARE_CONF_VALID_USERS,
	KSMBD_SHARE_CONF_INVALID_USERS,
	KSMBD_SHARE_CONF_READ_LIST,
	KSMBD_SHARE_CONF_WRITE_LIST			= 20,
	KSMBD_SHARE_CONF_ADMIN_USERS,
	KSMBD_SHARE_CONF_HOSTS_ALLOW,
	KSMBD_SHARE_CONF_HOSTS_DENY,
	KSMBD_SHARE_CONF_MAX_CONNECTIONS,
	KSMBD_SHARE_CONF_VETO_FILES			= 25,
	KSMBD_SHARE_CONF_INHERIT_OWNER,
	KSMBD_SHARE_CONF_FOLLOW_SYMLINKS,
	KSMBD_SHARE_CONF_VFS_OBJECTS,
	KSMBD_SHARE_CONF_WRITABLE,
	KSMBD_SHARE_CONF_CROSSMNT			= 30,
	KSMBD_SHARE_CONF_MAX
};

extern const char *KSMBD_SHARE_CONF[KSMBD_SHARE_CONF_MAX];
extern const char *KSMBD_SHARE_DEFCONF[KSMBD_SHARE_CONF_MAX];

#define KSMBD_SHARE_CONF_IS_GLOBAL(c) \
	((c) == KSMBD_SHARE_CONF_GUEST_ACCOUNT || \
	 (c) == KSMBD_SHARE_CONF_MAX_CONNECTIONS)

#define KSMBD_SHARE_CONF_IS_BROKEN(c) \
	((c) == KSMBD_SHARE_CONF_ADMIN_USERS || \
	 (c) == KSMBD_SHARE_CONF_HOSTS_ALLOW || \
	 (c) == KSMBD_SHARE_CONF_HOSTS_DENY || \
	 (c) == KSMBD_SHARE_CONF_FOLLOW_SYMLINKS)

int shm_share_name(char *name, char *p);
int shm_share_config(const char *k, enum KSMBD_SHARE_CONF c);

static inline void set_share_flag(struct ksmbd_share *share, int flag)
{
	share->flags |= flag;
}

static inline void clear_share_flag(struct ksmbd_share *share, int flag)
{
	share->flags &= ~flag;
}

static inline int test_share_flag(struct ksmbd_share *share, int flag)
{
	return share->flags & flag;
}

struct ksmbd_share *get_ksmbd_share(struct ksmbd_share *share);
void put_ksmbd_share(struct ksmbd_share *share);
struct ksmbd_share *shm_lookup_share(char *name);

struct smbconf_group;
int shm_add_new_share(struct smbconf_group *group);

void shm_remove_all_shares(void);

void shm_destroy(void);
unsigned int shm_share_name_hash(const char *name);
int shm_share_name_equal(const char *lname, const char *rname);
void shm_init(void);

int shm_lookup_users_map(struct ksmbd_share *share,
			  enum share_users map,
			  char *name);

int shm_lookup_hosts_map(struct ksmbd_share *share,
			  enum share_hosts map,
			  char *host);

int shm_open_connection(struct ksmbd_share *share);
int shm_close_connection(struct ksmbd_share *share);

typedef void (*share_cb)(struct ksmbd_share *share, void *data);
void shm_iter_shares(share_cb cb, void *data);

struct ksmbd_share_config_response;

int shm_share_config_payload_size(struct ksmbd_share *share);
int shm_handle_share_config_request(struct ksmbd_share *share,
				    struct ksmbd_share_config_response *resp);

#endif /* __MANAGEMENT_SHARE_H__ */
