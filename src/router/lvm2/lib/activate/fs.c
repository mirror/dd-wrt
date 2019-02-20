/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "fs.h"
#include "lib/activate/activate.h"
#include "lib/commands/toolcontext.h"
#include "lib/misc/lvm-string.h"
#include "lib/misc/lvm-file.h"
#include "lib/mm/memlock.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>

/*
 * Library cookie to combine multiple fs transactions.
 * Supports to wait for udev device settle only when needed.
 */
static uint32_t _fs_cookie = DM_COOKIE_AUTO_CREATE;
static int _fs_create = 0;

static int _mk_dir(const char *dev_dir, const char *vg_name)
{
	static char vg_path[PATH_MAX];
	mode_t old_umask;

	if (dm_snprintf(vg_path, sizeof(vg_path), "%s%s",
			 dev_dir, vg_name) == -1) {
		log_error("Couldn't construct name of volume "
			  "group directory.");
		return 0;
	}

	if (dir_exists(vg_path))
		return 1;

	log_very_verbose("Creating directory %s", vg_path);

	(void) dm_prepare_selinux_context(vg_path, S_IFDIR);
	old_umask = umask(DM_DEV_DIR_UMASK);
	if (mkdir(vg_path, 0777)) {
		log_sys_error("mkdir", vg_path);
		umask(old_umask);
		(void) dm_prepare_selinux_context(NULL, 0);
		return 0;
	}
	umask(old_umask);
	(void) dm_prepare_selinux_context(NULL, 0);

	return 1;
}

static int _rm_dir(const char *dev_dir, const char *vg_name)
{
	static char vg_path[PATH_MAX];

	if (dm_snprintf(vg_path, sizeof(vg_path), "%s%s",
			 dev_dir, vg_name) == -1) {
		log_error("Couldn't construct name of volume "
			  "group directory.");
		return 0;
	}

	if (dir_exists(vg_path) && is_empty_dir(vg_path)) {
		log_very_verbose("Removing directory %s", vg_path);
		rmdir(vg_path);
	}

	return 1;
}

static void _rm_blks(const char *dir)
{
	const char *name;
	static char path[PATH_MAX];
	struct dirent *dirent;
	struct stat buf;
	DIR *d;

	if (!(d = opendir(dir))) {
		log_sys_error("opendir", dir);
		return;
	}

	while ((dirent = readdir(d))) {
		name = dirent->d_name;

		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;

		if (dm_snprintf(path, sizeof(path), "%s/%s", dir, name) == -1) {
			log_error("Couldn't create path for %s", name);
			continue;
		}

		if (!lstat(path, &buf)) {
			if (!S_ISBLK(buf.st_mode))
				continue;
			log_very_verbose("Removing %s", path);
			if (unlink(path) < 0)
				log_sys_error("unlink", path);
		}
	}

	if (closedir(d))
		log_sys_error("closedir", dir);
}

static int _mk_link(const char *dev_dir, const char *vg_name,
		    const char *lv_name, const char *dev, int check_udev)
{
	static char lv_path[PATH_MAX], link_path[PATH_MAX], lvm1_group_path[PATH_MAX];
	static char vg_path[PATH_MAX];
	struct stat buf, buf_lp;

	if (dm_snprintf(vg_path, sizeof(vg_path), "%s%s",
			 dev_dir, vg_name) == -1) {
		log_error("Couldn't create path for volume group dir %s",
			  vg_name);
		return 0;
	}

	if (dm_snprintf(lv_path, sizeof(lv_path), "%s/%s", vg_path,
			 lv_name) == -1) {
		log_error("Couldn't create source pathname for "
			  "logical volume link %s", lv_name);
		return 0;
	}

	if (dm_snprintf(link_path, sizeof(link_path), "%s/%s",
			 dm_dir(), dev) == -1) {
		log_error("Couldn't create destination pathname for "
			  "logical volume link for %s", lv_name);
		return 0;
	}

	if (dm_snprintf(lvm1_group_path, sizeof(lvm1_group_path), "%s/group",
			 vg_path) == -1) {
		log_error("Couldn't create pathname for LVM1 group file for %s",
			  vg_name);
		return 0;
	}

	/* To reach this point, the VG must have been locked.
	 * As locking fails if the VG is active under LVM1, it's
	 * now safe to remove any LVM1 devices we find here
	 * (as well as any existing LVM2 symlink). */
	if (!lstat(lvm1_group_path, &buf)) {
		if (!S_ISCHR(buf.st_mode)) {
			log_error("Non-LVM1 character device found at %s",
				  lvm1_group_path);
		} else {
			_rm_blks(vg_path);

			log_very_verbose("Removing %s", lvm1_group_path);
			if (unlink(lvm1_group_path) < 0)
				log_sys_error("unlink", lvm1_group_path);
		}
	}

	if (!lstat(lv_path, &buf)) {
		if (!S_ISLNK(buf.st_mode) && !S_ISBLK(buf.st_mode)) {
			log_error("Symbolic link %s not created: file exists",
				  link_path);
			return 0;
		}

		if (dm_udev_get_sync_support() && udev_checking() && check_udev) {
			/* Check udev created the correct link. */
			if (!stat(link_path, &buf_lp) &&
			    !stat(lv_path, &buf)) {
				if (buf_lp.st_rdev == buf.st_rdev)
					return 1;

				log_warn("Symlink %s that should have been "
					 "created by udev does not have "
					 "correct target. Falling back to "
					 "direct link creation", lv_path);
			} else
				log_warn("Symlink %s that should have been "
					 "created by udev could not be checked "
					 "for its correctness. Falling back to "
					 "direct link creation.", lv_path);

		}

		log_very_verbose("Removing %s", lv_path);
		if (unlink(lv_path) < 0) {
			log_sys_error("unlink", lv_path);
			return 0;
		}
	} else if (dm_udev_get_sync_support() && udev_checking() && check_udev)
		log_warn("The link %s should have been created by udev "
			  "but it was not found. Falling back to "
			  "direct link creation.", lv_path);

	log_very_verbose("Linking %s -> %s", lv_path, link_path);

	(void) dm_prepare_selinux_context(lv_path, S_IFLNK);
	if (symlink(link_path, lv_path) < 0) {
		log_sys_error("symlink", lv_path);
		(void) dm_prepare_selinux_context(NULL, 0);
		return 0;
	}
	(void) dm_prepare_selinux_context(NULL, 0);

	return 1;
}

static int _rm_link(const char *dev_dir, const char *vg_name,
		    const char *lv_name, int check_udev)
{
	struct stat buf;
	static char lv_path[PATH_MAX];

	if (dm_snprintf(lv_path, sizeof(lv_path), "%s%s/%s",
			 dev_dir, vg_name, lv_name) == -1) {
		log_error("Couldn't determine link pathname.");
		return 0;
	}

	if (lstat(lv_path, &buf)) {
		if (errno == ENOENT)
			return 1;
		log_sys_error("lstat", lv_path);
		return 0;
	}

	if (dm_udev_get_sync_support() && udev_checking() && check_udev)
		log_warn("The link %s should have been removed by udev "
			 "but it is still present. Falling back to "
			 "direct link removal.", lv_path);

	if (!S_ISLNK(buf.st_mode)) {
		log_error("%s not symbolic link - not removing", lv_path);
		return 0;
	}

	log_very_verbose("Removing link %s", lv_path);
	if (unlink(lv_path) < 0) {
		log_sys_error("unlink", lv_path);
		return 0;
	}

	return 1;
}

typedef enum {
	FS_ADD,
	FS_DEL,
	FS_RENAME,
	NUM_FS_OPS
} fs_op_t;

static int _do_fs_op(fs_op_t type, const char *dev_dir, const char *vg_name,
		     const char *lv_name, const char *dev,
		     const char *old_lv_name, int check_udev)
{
	switch (type) {
	case FS_ADD:
		if (!_mk_dir(dev_dir, vg_name) ||
		    !_mk_link(dev_dir, vg_name, lv_name, dev, check_udev))
			return_0;
		break;
	case FS_DEL:
		if (!_rm_link(dev_dir, vg_name, lv_name, check_udev) ||
		    !_rm_dir(dev_dir, vg_name))
			return_0;
		break;
		/* FIXME Use rename() */
	case FS_RENAME:
		if (old_lv_name && !_rm_link(dev_dir, vg_name, old_lv_name,
					     check_udev))
			stack;

		if (!_mk_link(dev_dir, vg_name, lv_name, dev, check_udev))
			stack;
	default:
		; /* NOTREACHED */
	}

	return 1;
}

static DM_LIST_INIT(_fs_ops);
/*
 * Count number of stacked fs_op_t operations to allow to skip dm_list search.
 * FIXME: handling of FS_RENAME
 */
static int _count_fs_ops[NUM_FS_OPS];

struct fs_op_parms {
	struct dm_list list;
	fs_op_t type;
	int check_udev;
	char *dev_dir;
	char *vg_name;
	char *lv_name;
	char *dev;
	char *old_lv_name;
	char names[0];
};

static void _store_str(char **pos, char **ptr, const char *str)
{
	strcpy(*pos, str);
	*ptr = *pos;
	*pos += strlen(*ptr) + 1;
}

static void _del_fs_op(struct fs_op_parms *fsp)
{
	_count_fs_ops[fsp->type]--;
	dm_list_del(&fsp->list);
	free(fsp);
}

/* Check if there is other the type of fs operation stacked */
static int _other_fs_ops(fs_op_t type)
{
	unsigned i;

	for (i = 0; i < NUM_FS_OPS; i++)
		if (type != i && _count_fs_ops[i])
			return 1;
	return 0;
}

/* Check if udev is supposed to create nodes */
static int _check_udev(int check_udev)
{
    return check_udev && dm_udev_get_sync_support() && dm_udev_get_checking();
}

/* FIXME: duplication of the  code from libdm-common.c */
static int _stack_fs_op(fs_op_t type, const char *dev_dir, const char *vg_name,
			const char *lv_name, const char *dev,
			const char *old_lv_name, int check_udev)
{
	struct dm_list *fsph, *fspht;
	struct fs_op_parms *fsp;
	size_t len = strlen(dev_dir) + strlen(vg_name) + strlen(lv_name) +
	    strlen(dev) + strlen(old_lv_name) + 5;
	char *pos;

	if ((type == FS_DEL) && _other_fs_ops(type))
		/*
		 * Ignore any outstanding operations on the fs_op if deleting it.
		 */
		dm_list_iterate_safe(fsph, fspht, &_fs_ops) {
			fsp = dm_list_item(fsph, struct fs_op_parms);
			if (!strcmp(lv_name, fsp->lv_name) &&
			    !strcmp(vg_name, fsp->vg_name)) {
				_del_fs_op(fsp);
				if (!_other_fs_ops(type))
					break; /* no other non DEL ops */
			}
		}
	else if ((type == FS_ADD) && _count_fs_ops[FS_DEL] && _check_udev(check_udev))
		/*
		 * If udev is running ignore previous DEL operation on added fs_op.
		 * (No other operations for this device then DEL could be stacked here).
		 */
		dm_list_iterate_safe(fsph, fspht, &_fs_ops) {
			fsp = dm_list_item(fsph, struct fs_op_parms);
			if ((fsp->type == FS_DEL) &&
			    !strcmp(lv_name, fsp->lv_name) &&
			    !strcmp(vg_name, fsp->vg_name)) {
				_del_fs_op(fsp);
				break; /* no other DEL ops */
			}
		}
	else if ((type == FS_RENAME) && _check_udev(check_udev))
		/*
		 * If udev is running ignore any outstanding operations if renaming it.
		 *
		 * Currently RENAME operation happens through 'suspend -> resume'.
		 * On 'resume' device is added with read_ahead settings, so it
		 * safe to remove any stacked ADD, RENAME, READ_AHEAD operation
		 * There cannot be any DEL operation on the renamed device.
		 */
		dm_list_iterate_safe(fsph, fspht, &_fs_ops) {
			fsp = dm_list_item(fsph, struct fs_op_parms);
			if (!strcmp(old_lv_name, fsp->lv_name) &&
			    !strcmp(vg_name, fsp->vg_name))
				_del_fs_op(fsp);
		}

	if (!(fsp = malloc(sizeof(*fsp) + len))) {
		log_error("No space to stack fs operation");
		return 0;
	}

	pos = fsp->names;
	fsp->type = type;
	fsp->check_udev = check_udev;

	_store_str(&pos, &fsp->dev_dir, dev_dir);
	_store_str(&pos, &fsp->vg_name, vg_name);
	_store_str(&pos, &fsp->lv_name, lv_name);
	_store_str(&pos, &fsp->dev, dev);
	_store_str(&pos, &fsp->old_lv_name, old_lv_name);

	_count_fs_ops[type]++;
	dm_list_add(&_fs_ops, &fsp->list);

	return 1;
}

static void _pop_fs_ops(void)
{
	struct dm_list *fsph, *fspht;
	struct fs_op_parms *fsp;

	dm_list_iterate_safe(fsph, fspht, &_fs_ops) {
		fsp = dm_list_item(fsph, struct fs_op_parms);
		_do_fs_op(fsp->type, fsp->dev_dir, fsp->vg_name, fsp->lv_name,
			  fsp->dev, fsp->old_lv_name, fsp->check_udev);
		_del_fs_op(fsp);
	}

	_fs_create = 0;
}

static int _fs_op(fs_op_t type, const char *dev_dir, const char *vg_name,
		  const char *lv_name, const char *dev, const char *old_lv_name,
		  int check_udev)
{
	if (prioritized_section()) {
		if (!_stack_fs_op(type, dev_dir, vg_name, lv_name, dev,
				  old_lv_name, check_udev))
			return_0;
		return 1;
	}

	return _do_fs_op(type, dev_dir, vg_name, lv_name, dev,
			 old_lv_name, check_udev);
}

int fs_add_lv(const struct logical_volume *lv, const char *dev)
{
	return _fs_op(FS_ADD, lv->vg->cmd->dev_dir, lv->vg->name, lv->name,
		      dev, "", lv->vg->cmd->current_settings.udev_rules);
}

int fs_del_lv(const struct logical_volume *lv)
{
	return _fs_op(FS_DEL, lv->vg->cmd->dev_dir, lv->vg->name, lv->name,
		      "", "", lv->vg->cmd->current_settings.udev_rules);
}

int fs_del_lv_byname(const char *dev_dir, const char *vg_name,
		     const char *lv_name, int check_udev)
{
	return _fs_op(FS_DEL, dev_dir, vg_name, lv_name, "", "", check_udev);
}

int fs_rename_lv(const struct logical_volume *lv, const char *dev,
		 const char *old_vgname, const char *old_lvname)
{
	if (strcmp(old_vgname, lv->vg->name)) {
		return
			(_fs_op(FS_DEL, lv->vg->cmd->dev_dir, old_vgname,
				old_lvname, "", "", lv->vg->cmd->current_settings.udev_rules) &&
			 _fs_op(FS_ADD, lv->vg->cmd->dev_dir, lv->vg->name,
				lv->name, dev, "", lv->vg->cmd->current_settings.udev_rules));
	}

	return _fs_op(FS_RENAME, lv->vg->cmd->dev_dir, lv->vg->name, lv->name,
		      dev, old_lvname, lv->vg->cmd->current_settings.udev_rules);
}

void fs_unlock(void)
{
	if (!prioritized_section()) {
		log_debug_activation("Syncing device names");
		/* Wait for all processed udev devices */
		if (!dm_udev_wait(_fs_cookie))
			stack;
		_fs_cookie = DM_COOKIE_AUTO_CREATE; /* Reset cookie */
		dm_lib_release();
		_pop_fs_ops();
	}
}

uint32_t fs_get_cookie(void)
{
	return _fs_cookie;
}

void fs_set_cookie(uint32_t cookie)
{
	_fs_cookie = cookie;
}

void fs_set_create(void)
{
	_fs_create = 1;
}

int fs_has_non_delete_ops(void)
{
	return _fs_create || _other_fs_ops(FS_DEL);
}
