/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_LVMPOLLD_DATA_UTILS_H
#define _LVM_LVMPOLLD_DATA_UTILS_H

#include <pthread.h>

struct buffer;
struct lvmpolld_state;

enum poll_type {
	PVMOVE = 0,
	CONVERT,
	MERGE,
	MERGE_THIN,
	POLL_TYPE_MAX
};

struct lvmpolld_cmd_stat {
	int retcode;
	int signal;
};

struct lvmpolld_store {
	pthread_mutex_t lock;
	void *store;
	const char *name;
	unsigned active_polling_count;
};

struct lvmpolld_lv {
	/*
	 * accessing following vars doesn't
	 * require struct lvmpolld_lv lock
	 */
	struct lvmpolld_state *const ls;
	const enum poll_type type;
	const char *const lvid;
	const char *const lvmpolld_id;
	const char *const lvname; /* full vg/lv name */
	const unsigned pdtimeout; /* in seconds */
	const char *const sinterval;
	const char *const lvm_system_dir_env;
	struct lvmpolld_store *const pdst;
	const char *const *cmdargv;
	const char *const *cmdenvp;

	/* only used by write */
	pid_t cmd_pid;
	pthread_t tid;

	pthread_mutex_t lock;

	/* block of shared variables protected by lock */
	struct lvmpolld_cmd_stat cmd_state;
	unsigned init_rq_count; /* for debuging purposes only */
	unsigned polling_finished:1; /* no more updates */
	unsigned error:1; /* unrecoverable error occured in lvmpolld */
};

typedef void (*lvmpolld_parse_output_fn_t) (struct lvmpolld_lv *pdlv, const char *line);

/* TODO: replace with configuration option */
#define MIN_POLLING_TIMEOUT 60

struct lvmpolld_lv_state {
	unsigned error:1;
	unsigned polling_finished:1;
	struct lvmpolld_cmd_stat cmd_state;
};

struct lvmpolld_thread_data {
	char *line;
	size_t line_size;
	int outpipe[2];
	int errpipe[2];
	FILE *fout;
	FILE *ferr;
	char buf[1024];
	struct lvmpolld_lv *pdlv;
};

char *construct_id(const char *sysdir, const char *lvid);

/* LVMPOLLD_LV_T section */

/* only call with appropriate struct lvmpolld_store lock held */
struct lvmpolld_lv *pdlv_create(struct lvmpolld_state *ls, const char *id,
			   const char *vgname, const char *lvname,
			   const char *sysdir, enum poll_type type,
			   const char *sinterval, unsigned pdtimeout,
			   struct lvmpolld_store *pdst);

/* only call with appropriate struct lvmpolld_store lock held */
void pdlv_destroy(struct lvmpolld_lv *pdlv);

static inline void pdlv_lock(struct lvmpolld_lv *pdlv)
{
	pthread_mutex_lock(&pdlv->lock);
}

static inline void pdlv_unlock(struct lvmpolld_lv *pdlv)
{
	pthread_mutex_unlock(&pdlv->lock);
}

/*
 * no struct lvmpolld_lv lock required section
 */
static inline int pdlv_is_type(const struct lvmpolld_lv *pdlv, enum poll_type type)
{
	return pdlv->type == type;
}

static inline unsigned pdlv_get_timeout(const struct lvmpolld_lv *pdlv)
{
	return pdlv->pdtimeout;
}

static inline enum poll_type pdlv_get_type(const struct lvmpolld_lv *pdlv)
{
	return pdlv->type;
}

unsigned pdlv_get_polling_finished(struct lvmpolld_lv *pdlv);
struct lvmpolld_lv_state pdlv_get_status(struct lvmpolld_lv *pdlv);
void pdlv_set_cmd_state(struct lvmpolld_lv *pdlv, const struct lvmpolld_cmd_stat *cmd_state);
void pdlv_set_error(struct lvmpolld_lv *pdlv, unsigned error);
void pdlv_set_polling_finished(struct lvmpolld_lv *pdlv, unsigned finished);

/*
 * struct lvmpolld_lv lock required section
 */
static inline struct lvmpolld_cmd_stat pdlv_locked_cmd_state(const struct lvmpolld_lv *pdlv)
{
	return pdlv->cmd_state;
}

static inline int pdlv_locked_polling_finished(const struct lvmpolld_lv *pdlv)
{
	return pdlv->polling_finished;
}

static inline unsigned pdlv_locked_error(const struct lvmpolld_lv *pdlv)
{
	return pdlv->error;
}

/* struct lvmpolld_store manipulation routines */

struct lvmpolld_store *pdst_init(const char *name);
void pdst_destroy(struct lvmpolld_store *pdst);

void pdst_locked_dump(const struct lvmpolld_store *pdst, struct buffer *buff);
void pdst_locked_lock_all_pdlvs(const struct lvmpolld_store *pdst);
void pdst_locked_unlock_all_pdlvs(const struct lvmpolld_store *pdst);
void pdst_locked_destroy_all_pdlvs(const struct lvmpolld_store *pdst);
void pdst_locked_send_cancel(const struct lvmpolld_store *pdst);

static inline void pdst_lock(struct lvmpolld_store *pdst)
{
	pthread_mutex_lock(&pdst->lock);
}

static inline void pdst_unlock(struct lvmpolld_store *pdst)
{
	pthread_mutex_unlock(&pdst->lock);
}

static inline void pdst_locked_inc(struct lvmpolld_store *pdst)
{
	pdst->active_polling_count++;
}

static inline void pdst_locked_dec(struct lvmpolld_store *pdst)
{
	pdst->active_polling_count--;
}

static inline unsigned pdst_locked_get_active_count(const struct lvmpolld_store *pdst)
{
	return pdst->active_polling_count;
}

static inline int pdst_locked_insert(struct lvmpolld_store *pdst, const char *key, struct lvmpolld_lv *pdlv)
{
	return dm_hash_insert(pdst->store, key, pdlv);
}

static inline struct lvmpolld_lv *pdst_locked_lookup(struct lvmpolld_store *pdst, const char *key)
{
	return dm_hash_lookup(pdst->store, key);
}

static inline void pdst_locked_remove(struct lvmpolld_store *pdst, const char *key)
{
	dm_hash_remove(pdst->store, key);
}

struct lvmpolld_thread_data *lvmpolld_thread_data_constructor(struct lvmpolld_lv *pdlv);
void lvmpolld_thread_data_destroy(void *thread_private);

#endif /* _LVM_LVMPOLLD_DATA_UTILS_H */
