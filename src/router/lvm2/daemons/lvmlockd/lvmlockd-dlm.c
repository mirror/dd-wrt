/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#define _XOPEN_SOURCE 500  /* pthread */
#define _ISOC99_SOURCE

#include "tools/tool.h"

#include "daemon-server.h"
#include "lib/mm/xlate.h"

#include "lvmlockd-internal.h"
#include "daemons/lvmlockd/lvmlockd-client.h"

/*
 * Using synchronous _wait dlm apis so do not define _REENTRANT and
 * link with non-threaded version of library, libdlm_lt.
 */
#include "libdlm.h"

#include <stddef.h>
#include <poll.h>
#include <errno.h>
#include <endian.h>
#include <fcntl.h>
#include <byteswap.h>
#include <syslog.h>
#include <dirent.h>

struct lm_dlm {
	dlm_lshandle_t *dh;
};

struct rd_dlm {
	struct dlm_lksb lksb;
	struct val_blk *vb;
};

int lm_data_size_dlm(void)
{
	return sizeof(struct rd_dlm);
}

/*
 * lock_args format
 *
 * vg_lock_args format for dlm is
 * vg_version_string:undefined:cluster_name
 *
 * lv_lock_args are not used for dlm
 *
 * version_string is MAJOR.MINOR.PATCH
 * undefined may contain ":"
 */

#define VG_LOCK_ARGS_MAJOR 1
#define VG_LOCK_ARGS_MINOR 0
#define VG_LOCK_ARGS_PATCH 0

static int dlm_has_lvb_bug;

static int cluster_name_from_args(char *vg_args, char *clustername)
{
	return last_string_from_args(vg_args, clustername);
}

static int check_args_version(char *vg_args)
{
	unsigned int major = 0;
	int rv;

	rv = version_from_args(vg_args, &major, NULL, NULL);
	if (rv < 0) {
		log_error("check_args_version %s error %d", vg_args, rv);
		return rv;
	}

	if (major > VG_LOCK_ARGS_MAJOR) {
		log_error("check_args_version %s major %d %d", vg_args, major, VG_LOCK_ARGS_MAJOR);
		return -1;
	}

	return 0;
}

/* This will be set after dlm_controld is started. */
#define DLM_CLUSTER_NAME_PATH "/sys/kernel/config/dlm/cluster/cluster_name"

static int read_cluster_name(char *clustername)
{
	static const char close_error_msg[] = "read_cluster_name: close_error %d";
	char *n;
	int fd;
	int rv;

	if (daemon_test) {
		sprintf(clustername, "%s", "test");
		return 0;
	}

	fd = open(DLM_CLUSTER_NAME_PATH, O_RDONLY);
	if (fd < 0) {
		log_debug("read_cluster_name: open error %d, check dlm_controld", fd);
		return fd;
	}

	rv = read(fd, clustername, MAX_ARGS);
	if (rv < 0) {
		log_error("read_cluster_name: cluster name read error %d, check dlm_controld", fd);
		if (close(fd))
			log_error(close_error_msg, fd);
		return rv;
	}

	n = strstr(clustername, "\n");
	if (n)
		*n = '\0';
	if (close(fd))
		log_error(close_error_msg, fd);
	return 0;
}

int lm_init_vg_dlm(char *ls_name, char *vg_name, uint32_t flags, char *vg_args)
{
	char clustername[MAX_ARGS+1];
	char lock_args_version[MAX_ARGS+1];
	int rv;

	memset(clustername, 0, sizeof(clustername));
	memset(lock_args_version, 0, sizeof(lock_args_version));

	snprintf(lock_args_version, MAX_ARGS, "%u.%u.%u",
		 VG_LOCK_ARGS_MAJOR, VG_LOCK_ARGS_MINOR, VG_LOCK_ARGS_PATCH);

	rv = read_cluster_name(clustername);
	if (rv < 0)
		return -EMANAGER;

	if (strlen(clustername) + strlen(lock_args_version) + 2 > MAX_ARGS) {
		log_error("init_vg_dlm args too long");
		return -EARGS;
	}

	snprintf(vg_args, MAX_ARGS, "%s:%s", lock_args_version, clustername);
	rv = 0;

	log_debug("init_vg_dlm done %s vg_args %s", ls_name, vg_args);
	return rv;
}

int lm_prepare_lockspace_dlm(struct lockspace *ls)
{
	char sys_clustername[MAX_ARGS+1];
	char arg_clustername[MAX_ARGS+1];
	uint32_t major = 0, minor = 0, patch = 0;
	struct lm_dlm *lmd;
	int rv;

	if (daemon_test)
		goto skip_args;

	memset(sys_clustername, 0, sizeof(sys_clustername));
	memset(arg_clustername, 0, sizeof(arg_clustername));

	rv = read_cluster_name(sys_clustername);
	if (rv < 0)
		return -EMANAGER;

	rv = dlm_kernel_version(&major, &minor, &patch);
	if (rv < 0) {
		log_error("prepare_lockspace_dlm kernel_version not detected %d", rv);
		dlm_has_lvb_bug = 1;
	}

	if ((major == 6) && (minor == 0) && (patch == 1)) {
		log_debug("dlm kernel version %u.%u.%u has lvb bug", major, minor, patch);
		dlm_has_lvb_bug = 1;
	}

	if (!ls->vg_args[0]) {
		/* global lockspace has no vg args */
		goto skip_args;
	}

	rv = check_args_version(ls->vg_args);
	if (rv < 0)
		return -EARGS;

	rv = cluster_name_from_args(ls->vg_args, arg_clustername);
	if (rv < 0) {
		log_error("prepare_lockspace_dlm %s no cluster name from args %s", ls->name, ls->vg_args);
		return -EARGS;
	}

	if (strcmp(sys_clustername, arg_clustername)) {
		log_error("prepare_lockspace_dlm %s mismatching cluster names sys %s arg %s",
			  ls->name, sys_clustername, arg_clustername);
		return -EARGS;
	}

 skip_args:
	lmd = malloc(sizeof(struct lm_dlm));
	if (!lmd)
		return -ENOMEM;

	ls->lm_data = lmd;
	return 0;
}

int lm_add_lockspace_dlm(struct lockspace *ls, int adopt)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;

	if (daemon_test)
		return 0;

	if (adopt)
		lmd->dh = dlm_open_lockspace(ls->name);
	else
		lmd->dh = dlm_new_lockspace(ls->name, 0600, DLM_LSFL_NEWEXCL);

	if (!lmd->dh) {
		log_error("add_lockspace_dlm %s adopt %d error", ls->name, adopt);
		free(lmd);
		ls->lm_data = NULL;
		return -1;
	}

	return 0;
}

int lm_rem_lockspace_dlm(struct lockspace *ls, int free_vg)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	int rv;

	if (daemon_test)
		goto out;

	/*
	 * If free_vg is set, it means we are doing vgremove, and we may want
	 * to tell any other nodes to leave the lockspace.  This is not really
	 * necessary since there should be no harm in having an unused
	 * lockspace sitting around.  A new "notification lock" would need to
	 * be added with a callback to signal this. 
	 */

	rv = dlm_release_lockspace(ls->name, lmd->dh, 1);
	if (rv < 0) {
		log_error("rem_lockspace_dlm error %d", rv);
		return rv;
	}
 out:
	free(lmd);
	ls->lm_data = NULL;
	return 0;
}

static int lm_add_resource_dlm(struct lockspace *ls, struct resource *r, int with_lock_nl)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	struct rd_dlm *rdd = (struct rd_dlm *)r->lm_data;
	uint32_t flags = 0;
	char *buf;
	int rv;

	if (r->type == LD_RT_GL || r->type == LD_RT_VG) {
		buf = malloc(sizeof(struct val_blk) + DLM_LVB_LEN);
		if (!buf)
			return -ENOMEM;
		memset(buf, 0, sizeof(struct val_blk) + DLM_LVB_LEN);

		rdd->vb = (struct val_blk *)buf;
		rdd->lksb.sb_lvbptr = buf + sizeof(struct val_blk);

		flags |= LKF_VALBLK;
	}

	if (!with_lock_nl)
		goto out;

	/* because this is a new NL lock request */
	flags |= LKF_EXPEDITE;

	if (daemon_test)
		goto out;

	rv = dlm_ls_lock_wait(lmd->dh, LKM_NLMODE, &rdd->lksb, flags,
			      r->name, strlen(r->name),
			      0, NULL, NULL, NULL);
	if (rv < 0) {
		log_error("S %s R %s add_resource_dlm lock error %d", ls->name, r->name, rv);
		return rv;
	}
 out:
	return 0;
}

int lm_rem_resource_dlm(struct lockspace *ls, struct resource *r)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	struct rd_dlm *rdd = (struct rd_dlm *)r->lm_data;
	struct dlm_lksb *lksb;
	int rv = 0;

	if (daemon_test)
		goto out;

	lksb = &rdd->lksb;

	if (!lksb->sb_lkid)
		goto out;

	rv = dlm_ls_unlock_wait(lmd->dh, lksb->sb_lkid, 0, lksb);
	if (rv < 0) {
		log_error("S %s R %s rem_resource_dlm unlock error %d", ls->name, r->name, rv);
	}
 out:
	if (rdd->vb)
		free(rdd->vb);

	memset(rdd, 0, sizeof(struct rd_dlm));
	r->lm_init = 0;
	return rv;
}

static int to_dlm_mode(int ld_mode)
{
	switch (ld_mode) {
	case LD_LK_EX:
		return LKM_EXMODE;
	case LD_LK_SH:
		return LKM_PRMODE;
	};
	return -1;
}

static int lm_adopt_dlm(struct lockspace *ls, struct resource *r, int ld_mode,
			struct val_blk *vb_out)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	struct rd_dlm *rdd = (struct rd_dlm *)r->lm_data;
	struct dlm_lksb *lksb;
	uint32_t flags = 0;
	int mode;
	int rv;

	memset(vb_out, 0, sizeof(struct val_blk));

	if (!r->lm_init) {
		rv = lm_add_resource_dlm(ls, r, 0);
		if (rv < 0)
			return rv;
		r->lm_init = 1;
	}

	lksb = &rdd->lksb;

	flags |= LKF_PERSISTENT;
	flags |= LKF_ORPHAN;

	if (rdd->vb)
		flags |= LKF_VALBLK;

	mode = to_dlm_mode(ld_mode);
	if (mode < 0) {
		log_error("adopt_dlm invalid mode %d", ld_mode);
		rv = -EINVAL;
		goto fail;
	}

	log_debug("S %s R %s adopt_dlm", ls->name, r->name);

	if (daemon_test)
		return 0;

	/*
	 * dlm returns 0 for success, -EAGAIN if an orphan is
	 * found with another mode, and -ENOENT if no orphan.
	 *
	 * cast/bast/param are (void *)1 because the kernel
	 * returns errors if some are null.
	 */

	rv = dlm_ls_lockx(lmd->dh, mode, lksb, flags,
			  r->name, strlen(r->name), 0,
			  (void *)1, (void *)1, (void *)1,
			  NULL, NULL);

	if (rv == -1 && errno == -EAGAIN) {
		log_debug("S %s R %s adopt_dlm adopt mode %d try other mode",
			  ls->name, r->name, ld_mode);
		rv = -EUCLEAN;
		goto fail;
	}
	if (rv < 0) {
		log_debug("S %s R %s adopt_dlm mode %d flags %x error %d errno %d",
			  ls->name, r->name, mode, flags, rv, errno);
		goto fail;
	}

	/*
	 * FIXME: For GL/VG locks we probably want to read the lvb,
	 * especially if adopting an ex lock, because when we
	 * release this adopted ex lock we may want to write new
	 * lvb values based on the current lvb values (at lease
	 * in the GL case where we increment the current values.)
	 *
	 * It should be possible to read the lvb by requesting
	 * this lock in the same mode it's already in.
	 */

	return rv;

 fail:
	lm_rem_resource_dlm(ls, r);
	return rv;
}

/*
 * Use PERSISTENT so that if lvmlockd exits while holding locks,
 * the locks will remain orphaned in the dlm, still protecting what
 * they were acquired to protect.
 */

int lm_lock_dlm(struct lockspace *ls, struct resource *r, int ld_mode,
		struct val_blk *vb_out, int adopt)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	struct rd_dlm *rdd = (struct rd_dlm *)r->lm_data;
	struct dlm_lksb *lksb;
	struct val_blk vb;
	uint32_t flags = 0;
	int mode;
	int rv;

	if (adopt) {
		/* When adopting, we don't follow the normal method
		   of acquiring a NL lock then converting it to the
		   desired mode. */
		return lm_adopt_dlm(ls, r, ld_mode, vb_out);
	}

	if (!r->lm_init) {
		rv = lm_add_resource_dlm(ls, r, 1);
		if (rv < 0)
			return rv;
		r->lm_init = 1;
	}

	lksb = &rdd->lksb;

	flags |= LKF_CONVERT;
	flags |= LKF_NOQUEUE;
	flags |= LKF_PERSISTENT;

	if (rdd->vb)
		flags |= LKF_VALBLK;

	mode = to_dlm_mode(ld_mode);
	if (mode < 0) {
		log_error("lock_dlm invalid mode %d", ld_mode);
		return -EINVAL;
	}

	log_debug("S %s R %s lock_dlm", ls->name, r->name);

	if (daemon_test) {
		if (rdd->vb) {
			vb_out->version = le16_to_cpu(rdd->vb->version);
			vb_out->flags = le16_to_cpu(rdd->vb->flags);
			vb_out->r_version = le32_to_cpu(rdd->vb->r_version);
		}
		return 0;
	}

	/*
	 * The dlm lvb bug means that converting NL->EX will not return 
	 * the latest lvb, so we have to convert NL->PR->EX to reread it.
	 */
	if (dlm_has_lvb_bug && (ld_mode == LD_LK_EX)) {
		rv = dlm_ls_lock_wait(lmd->dh, LKM_PRMODE, lksb, flags,
				      r->name, strlen(r->name),
				      0, NULL, NULL, NULL);
		if (rv == -1) {
			log_debug("S %s R %s lock_dlm acquire mode PR for %d rv %d",
				  ls->name, r->name, mode, rv);
			goto lockrv;
		}

		/* Fall through to request EX. */
	}

	rv = dlm_ls_lock_wait(lmd->dh, mode, lksb, flags,
			      r->name, strlen(r->name),
			      0, NULL, NULL, NULL);
lockrv:
	if (rv == -1 && errno == EAGAIN) {
		log_debug("S %s R %s lock_dlm acquire mode %d rv EAGAIN", ls->name, r->name, mode);
		return -EAGAIN;
	}
	if (rv < 0) {
		log_error("S %s R %s lock_dlm acquire error %d errno %d", ls->name, r->name, rv, errno);
		return -ELMERR;
	}

	if (rdd->vb) {
		if (lksb->sb_flags & DLM_SBF_VALNOTVALID) {
			log_debug("S %s R %s lock_dlm VALNOTVALID", ls->name, r->name);
			memset(rdd->vb, 0, sizeof(struct val_blk));
			memset(vb_out, 0, sizeof(struct val_blk));
			goto out;
		}

		/*
		 * 'vb' contains disk endian values, not host endian.
		 * It is copied directly to rdd->vb which is also kept
		 * in disk endian form.
		 * vb_out is returned to the caller in host endian form.
		 */
		memcpy(&vb, lksb->sb_lvbptr, sizeof(struct val_blk));
		memcpy(rdd->vb, &vb, sizeof(vb));

		vb_out->version = le16_to_cpu(vb.version);
		vb_out->flags = le16_to_cpu(vb.flags);
		vb_out->r_version = le32_to_cpu(vb.r_version);
	}
out:
	return 0;
}

int lm_convert_dlm(struct lockspace *ls, struct resource *r,
		   int ld_mode, uint32_t r_version)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	struct rd_dlm *rdd = (struct rd_dlm *)r->lm_data;
	struct dlm_lksb *lksb = &rdd->lksb;
	uint32_t mode;
	uint32_t flags = 0;
	int rv;

	log_debug("S %s R %s convert_dlm", ls->name, r->name);

	flags |= LKF_CONVERT;
	flags |= LKF_NOQUEUE;
	flags |= LKF_PERSISTENT;

	if (rdd->vb && r_version && (r->mode == LD_LK_EX)) {
		if (!rdd->vb->version) {
			/* first time vb has been written */
			rdd->vb->version = cpu_to_le16(VAL_BLK_VERSION);
		}
		rdd->vb->r_version = cpu_to_le32(r_version);
		memcpy(lksb->sb_lvbptr, rdd->vb, sizeof(struct val_blk));

		log_debug("S %s R %s convert_dlm set r_version %u",
			  ls->name, r->name, r_version);

		flags |= LKF_VALBLK;
	}

	mode = to_dlm_mode(ld_mode);

	if (daemon_test)
		return 0;

	rv = dlm_ls_lock_wait(lmd->dh, mode, lksb, flags,
			      r->name, strlen(r->name),
			      0, NULL, NULL, NULL);
	if (rv == -1 && errno == EAGAIN) {
		/* FIXME: When does this happen?  Should something different be done? */
		log_error("S %s R %s convert_dlm mode %d rv EAGAIN", ls->name, r->name, mode);
		return -EAGAIN;
	}
	if (rv < 0) {
		log_error("S %s R %s convert_dlm error %d", ls->name, r->name, rv);
		rv = -ELMERR;
	}
	return rv;
}

int lm_unlock_dlm(struct lockspace *ls, struct resource *r,
		  uint32_t r_version, uint32_t lmu_flags)
{
	struct lm_dlm *lmd = (struct lm_dlm *)ls->lm_data;
	struct rd_dlm *rdd = (struct rd_dlm *)r->lm_data;
	struct dlm_lksb *lksb = &rdd->lksb;
	struct val_blk vb_prev;
	struct val_blk vb_next;
	uint32_t flags = 0;
	int new_vb = 0;
	int rv;

	/*
	 * Do not set PERSISTENT, because we don't need an orphan
	 * NL lock to protect anything.
	 */

	flags |= LKF_CONVERT;

	if (rdd->vb && (r->mode == LD_LK_EX)) {

		/* vb_prev and vb_next are in disk endian form */
		memcpy(&vb_prev, rdd->vb, sizeof(struct val_blk));
		memcpy(&vb_next, rdd->vb, sizeof(struct val_blk));

		if (!vb_prev.version) {
			vb_next.version = cpu_to_le16(VAL_BLK_VERSION);
			new_vb = 1;
		}

		if ((lmu_flags & LMUF_FREE_VG) && (r->type == LD_RT_VG)) {
			vb_next.flags = cpu_to_le16(VBF_REMOVED);
			new_vb = 1;
		}

		if (r_version) {
			vb_next.r_version = cpu_to_le32(r_version);
			new_vb = 1;
		}

		if (new_vb) {
			memcpy(rdd->vb, &vb_next, sizeof(struct val_blk));
			memcpy(lksb->sb_lvbptr, &vb_next, sizeof(struct val_blk));

			log_debug("S %s R %s unlock_dlm vb old %x %x %u new %x %x %u",
				  ls->name, r->name,
				  le16_to_cpu(vb_prev.version),
				  le16_to_cpu(vb_prev.flags),
				  le32_to_cpu(vb_prev.r_version),
				  le16_to_cpu(vb_next.version),
				  le16_to_cpu(vb_next.flags),
				  le32_to_cpu(vb_next.r_version));
		} else {
			log_debug("S %s R %s unlock_dlm vb unchanged", ls->name, r->name);
		}

		flags |= LKF_VALBLK;
	} else {
		log_debug("S %s R %s unlock_dlm", ls->name, r->name);
	}

	if (daemon_test)
		return 0;

	rv = dlm_ls_lock_wait(lmd->dh, LKM_NLMODE, lksb, flags,
			      r->name, strlen(r->name),
			      0, NULL, NULL, NULL);
	if (rv < 0) {
		log_error("S %s R %s unlock_dlm error %d", ls->name, r->name, rv);
		rv = -ELMERR;
	}

	return rv;
}

/*
 * This list could be read from dlm_controld via libdlmcontrol,
 * but it's simpler to get it from sysfs.
 */

#define DLM_LOCKSPACES_PATH "/sys/kernel/config/dlm/cluster/spaces"

/*
 * FIXME: this should be implemented differently.
 * It's not nice to use an aspect of the dlm clustering
 * implementation, which could change.  It would be
 * better to do something like use a special lock in the
 * lockspace that was held PR by all nodes, and then an
 * EX request on it could check if it's started (and
 * possibly also notify others to stop it automatically).
 * Or, possibly an enhancement to libdlm that would give
 * info about lockspace members.
 *
 * (We could let the VG be removed while others still
 * have the lockspace running, which largely works, but
 * introduces problems if another VG with the same name is
 * recreated while others still have the lockspace running
 * for the previous VG.  We'd also want a way to clean up
 * the stale lockspaces on the others eventually.)
 */

int lm_hosts_dlm(struct lockspace *ls, int notify)
{
	static const char closedir_err_msg[] = "lm_hosts_dlm: closedir failed";
	char ls_nodes_path[PATH_MAX];
	struct dirent *de;
	DIR *ls_dir;
	int count = 0;

	if (daemon_test)
		return 0;

	memset(ls_nodes_path, 0, sizeof(ls_nodes_path));
	snprintf(ls_nodes_path, PATH_MAX, "%s/%s/nodes",
		 DLM_LOCKSPACES_PATH, ls->name);

	if (!(ls_dir = opendir(ls_nodes_path)))
		return -ECONNREFUSED;

	while ((de = readdir(ls_dir))) {
		if (de->d_name[0] == '.')
			continue;
		count++;
	}

	if (closedir(ls_dir))
		log_error(closedir_err_msg);

	if (!count) {
		log_error("lm_hosts_dlm found no nodes in %s", ls_nodes_path);
		return 0;
	}

	/*
	 * Assume that a count of one node represents ourself,
	 * and any value over one represents other nodes.
	 */

	return count - 1;
}

int lm_get_lockspaces_dlm(struct list_head *ls_rejoin)
{
	static const char closedir_err_msg[] = "lm_get_lockspace_dlm: closedir failed";
	struct lockspace *ls;
	struct dirent *de;
	DIR *ls_dir;

	if (!(ls_dir = opendir(DLM_LOCKSPACES_PATH)))
		return -ECONNREFUSED;

	while ((de = readdir(ls_dir))) {
		if (de->d_name[0] == '.')
			continue;

		if (strncmp(de->d_name, LVM_LS_PREFIX, strlen(LVM_LS_PREFIX)))
			continue;

		if (!(ls = alloc_lockspace())) {
			if (closedir(ls_dir))
				log_error(closedir_err_msg);
			return -ENOMEM;
		}

		ls->lm_type = LD_LM_DLM;
		strncpy(ls->name, de->d_name, MAX_NAME);
		strncpy(ls->vg_name, ls->name + strlen(LVM_LS_PREFIX), MAX_NAME);
		list_add_tail(&ls->list, ls_rejoin);
	}

	if (closedir(ls_dir))
		log_error(closedir_err_msg);
	return 0;
}

int lm_is_running_dlm(void)
{
	char sys_clustername[MAX_ARGS+1];
	int rv;

	if (daemon_test)
		return gl_use_dlm;

	memset(sys_clustername, 0, sizeof(sys_clustername));

	rv = read_cluster_name(sys_clustername);
	if (rv < 0)
		return 0;
	return 1;
}

