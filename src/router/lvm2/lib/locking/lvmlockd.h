/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#ifndef _LVMLOCKD_H
#define _LVMLOCKD_H

#include "libdaemon/client/config-util.h"
#include "libdaemon/client/daemon-client.h"

#define LOCKD_SANLOCK_LV_NAME "lvmlock"

/* lockd_gl flags */
#define LDGL_UPDATE_NAMES         0x00000001

/* lockd_lv flags */
#define LDLV_MODE_NO_SH           0x00000001
#define LDLV_PERSISTENT           0x00000002

/* lvmlockd result flags */
#define LD_RF_NO_LOCKSPACES     0x00000001
#define LD_RF_NO_GL_LS          0x00000002
#define LD_RF_WARN_GL_REMOVED   0x00000004
#define LD_RF_DUP_GL_LS         0x00000008
#define LD_RF_NO_LM		0x00000010

/* lockd_state flags */
#define LDST_EX			0x00000001
#define LDST_SH			0x00000002
#define LDST_FAIL_REQUEST	0x00000004
#define LDST_FAIL_NOLS		0x00000008
#define LDST_FAIL_STARTING	0x00000010
#define LDST_FAIL_OTHER		0x00000020
#define LDST_FAIL		(LDST_FAIL_REQUEST | LDST_FAIL_NOLS | LDST_FAIL_STARTING | LDST_FAIL_OTHER)

#ifdef LVMLOCKD_SUPPORT

/* lvmlockd connection and communication */

void lvmlockd_set_socket(const char *sock);
void lvmlockd_set_use(int use);
int lvmlockd_use(void);
void lvmlockd_init(struct cmd_context *cmd);
void lvmlockd_connect(void);
void lvmlockd_disconnect(void);

/* vgcreate/vgremove use init/free */

int lockd_init_vg(struct cmd_context *cmd, struct volume_group *vg, const char *lock_type, int lv_lock_count);
int lockd_free_vg_before(struct cmd_context *cmd, struct volume_group *vg, int changing);
void lockd_free_vg_final(struct cmd_context *cmd, struct volume_group *vg);

/* vgrename */

int lockd_rename_vg_before(struct cmd_context *cmd, struct volume_group *vg);
int lockd_rename_vg_final(struct cmd_context *cmd, struct volume_group *vg, int success);

/* start and stop the lockspace for a vg */

int lockd_start_vg(struct cmd_context *cmd, struct volume_group *vg, int start_init);
int lockd_stop_vg(struct cmd_context *cmd, struct volume_group *vg);
int lockd_start_wait(struct cmd_context *cmd);

/* locking */

int lockd_gl_create(struct cmd_context *cmd, const char *def_mode, const char *vg_lock_type);
int lockd_gl(struct cmd_context *cmd, const char *def_mode, uint32_t flags);
int lockd_vg(struct cmd_context *cmd, const char *vg_name, const char *def_mode,
	     uint32_t flags, uint32_t *lockd_state);
int lockd_vg_update(struct volume_group *vg);

int lockd_lv_name(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lv_name, struct id *lv_id,
		  const char *lock_args, const char *def_mode, uint32_t flags);
int lockd_lv(struct cmd_context *cmd, struct logical_volume *lv,
	     const char *def_mode, uint32_t flags);

/* lvcreate/lvremove use init/free */

int lockd_init_lv(struct cmd_context *cmd, struct volume_group *vg, struct logical_volume *lv,
		  struct lvcreate_params *lp);
int lockd_init_lv_args(struct cmd_context *cmd, struct volume_group *vg,
		       struct logical_volume *lv, const char *lock_type, const char **lock_args);
int lockd_free_lv(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lv_name, struct id *lv_id, const char *lock_args);

const char *lockd_running_lock_type(struct cmd_context *cmd, int *found_multiple);

int handle_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg);

int lockd_lv_uses_lock(struct logical_volume *lv);

#else /* LVMLOCKD_SUPPORT */

static inline void lvmlockd_set_socket(const char *sock)
{
}

static inline void lvmlockd_set_use(int use)
{
}

static inline void lvmlockd_init(struct cmd_context *cmd)
{
}

static inline void lvmlockd_disconnect(void)
{
}

static inline void lvmlockd_connect(void)
{
}

static inline int lvmlockd_use(void)
{
	return 0;
}

static inline int lockd_init_vg(struct cmd_context *cmd, struct volume_group *vg, const char *lock_type, int lv_lock_count)
{
	return 1;
}

static inline int lockd_free_vg_before(struct cmd_context *cmd, struct volume_group *vg, int changing)
{
	return 1;
}

static inline void lockd_free_vg_final(struct cmd_context *cmd, struct volume_group *vg)
{
	return;
}

static inline int lockd_rename_vg_before(struct cmd_context *cmd, struct volume_group *vg)
{
	return 1;
}

static inline int lockd_rename_vg_final(struct cmd_context *cmd, struct volume_group *vg, int success)
{
	return 1;
}

static inline int lockd_start_vg(struct cmd_context *cmd, struct volume_group *vg, int start_init)
{
	return 0;
}

static inline int lockd_stop_vg(struct cmd_context *cmd, struct volume_group *vg)
{
	return 0;
}

static inline int lockd_start_wait(struct cmd_context *cmd)
{
	return 0;
}

static inline int lockd_gl_create(struct cmd_context *cmd, const char *def_mode, const char *vg_lock_type)
{
	/*
	 * When lvm is built without lvmlockd support, creating a VG with
	 * a shared lock type should fail.
	 */
	if (is_lockd_type(vg_lock_type)) {
		log_error("Using a shared lock type requires lvmlockd.");
		return 0;
	}
	return 1;
}

static inline int lockd_gl(struct cmd_context *cmd, const char *def_mode, uint32_t flags)
{
	return 1;
}

static inline int lockd_vg(struct cmd_context *cmd, const char *vg_name, const char *def_mode,
	     uint32_t flags, uint32_t *lockd_state)
{
	*lockd_state = 0;
	return 1;
}

static inline int lockd_vg_update(struct volume_group *vg)
{
	return 1;
}

static inline int lockd_lv_name(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lv_name, struct id *lv_id,
		  const char *lock_args, const char *def_mode, uint32_t flags)
{
	return 1;
}

static inline int lockd_lv(struct cmd_context *cmd, struct logical_volume *lv,
	     const char *def_mode, uint32_t flags)
{
	return 1;
}

static inline int lockd_init_lv(struct cmd_context *cmd, struct volume_group *vg,
		  	struct logical_volume *lv, struct lvcreate_params *lp)
{
	return 1;
}

static inline int lockd_init_lv_args(struct cmd_context *cmd, struct volume_group *vg,
		       struct logical_volume *lv, const char *lock_type, const char **lock_args)
{
	return 1;
}

static inline int lockd_free_lv(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lv_name, struct id *lv_id, const char *lock_args)
{
	return 1;
}

static inline const char *lockd_running_lock_type(struct cmd_context *cmd, int *found_multiple)
{
	log_error("Using a shared lock type requires lvmlockd.");
	return NULL;
}

static inline int handle_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg)
{
	return 0;
}

static inline int lockd_lv_uses_lock(struct logical_volume *lv)
{
	return 0;
}

#endif	/* LVMLOCKD_SUPPORT */

#endif	/* _LVMLOCKD_H */
