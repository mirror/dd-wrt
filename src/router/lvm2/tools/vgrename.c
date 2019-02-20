/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
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

#include "tools.h"

struct vgrename_params {
	const char *vg_name_old;
	const char *vg_name_new;
	unsigned int old_name_is_uuid : 1;
	unsigned int lock_vg_old_first : 1;
	unsigned int unlock_new_name: 1;
};

static int _lock_new_vg_for_rename(struct cmd_context *cmd,
				   const char *vg_name_new)
{
	if (!lock_vol(cmd, vg_name_new, LCK_VG_WRITE, NULL)) {
		log_error("Can't get lock for %s", vg_name_new);
		return 0;
	}

	return 1;
}

static int _vgrename_single(struct cmd_context *cmd, const char *vg_name,
			    struct volume_group *vg, struct processing_handle *handle)
{
	struct vgrename_params *vp = (struct vgrename_params *) handle->custom_handle;
	char old_path[PATH_MAX];
	char new_path[PATH_MAX];
	struct id id;
	const char *name;
	char *dev_dir;

	/*
	 * vg_name_old may be a UUID which process_each_vg
	 * replaced with the real VG name.  In that case,
	 * vp->vg_name_old will be the UUID and vg_name will be
	 * the actual VG name.  Check again if the old and new
	 * names match, using the real names.
	 */
	if (vp->old_name_is_uuid && !strcmp(vp->vg_name_new, vg_name)) {
		log_error("New VG name must differ from the old VG name.");
		return ECMD_FAILED;
	}

	/*
	 * Check if a VG already exists with the new VG name.
	 *
	 * (FIXME: We could look for the new name in the list of all
	 * VGs that process_each_vg created, but we don't have access
	 * to that list here, so we have to look in lvmcache.)
	 */

	if (lvmcache_vginfo_from_vgname(vp->vg_name_new, NULL)) {
		log_error("New VG name \"%s\" already exists", vp->vg_name_new);
		return ECMD_FAILED;
	}

	if (id_read_format_try(&id, vp->vg_name_new) &&
	    (name = lvmcache_vgname_from_vgid(cmd->mem, (const char *)&id))) {
		log_error("New VG name \"%s\" matches the UUID of existing VG %s", vp->vg_name_new, name);
		return ECMD_FAILED;
	}

	/*
	 * Lock the old VG name first:
	 * . The old VG name has already been locked by process_each_vg.
	 * . Now lock the new VG name here, second.
	 *
	 * Lock the new VG name first:
	 * . The new VG name has already been pre-locked below,
	 *   before process_each_vg was called.
	 * . process_each_vg then locked the old VG name second.
	 * . Nothing to do here.
	 *
	 * Special case when the old VG name is a uuid:
	 * . The old VG's real name wasn't known before process_each_vg,
	 *   so the correct lock ordering wasn't known beforehand,
	 *   so no pre-locking was done.
	 * . The old VG's real name has been locked by process_each_vg.
	 * . Now lock the new VG name here, second.
	 * . Suppress lock ordering checks because the lock order may
	 *   have wanted the new name first, which wasn't possible in
	 *   this uuid-for-name case.
	 */
	if (vp->lock_vg_old_first || vp->old_name_is_uuid) {
		if (!_lock_new_vg_for_rename(cmd, vp->vg_name_new))
			return ECMD_FAILED;
	}

	dev_dir = cmd->dev_dir;

	if (!archive(vg))
		goto error;

	if (!lockd_rename_vg_before(cmd, vg)) {
		stack;
		goto error;
	}

	/* Change the volume group name */
	vg_rename(cmd, vg, vp->vg_name_new);

	/* store it on disks */
	log_verbose("Writing out updated volume group");
	if (!vg_write(vg) || !vg_commit(vg)) {
		goto error;
	}

	if ((dm_snprintf(old_path, sizeof(old_path), "%s%s", dev_dir, vg_name) < 0) ||
	    (dm_snprintf(new_path, sizeof(new_path), "%s%s", dev_dir, vp->vg_name_new) < 0)) {
		log_error("Renaming path is too long %s/%s  %s/%s",
			  dev_dir, vg_name, dev_dir, vp->vg_name_new);
		goto error;
	}

	if (activation() && dir_exists(old_path)) {
		log_verbose("Renaming \"%s\" to \"%s\"", old_path, new_path);

		if (test_mode())
			log_verbose("Test mode: Skipping rename.");

		else if (lvs_in_vg_activated(vg)) {
			if (!vg_refresh_visible(cmd, vg)) {
				log_error("Renaming \"%s\" to \"%s\" failed", 
					old_path, new_path);
				goto error;
			}
		}
	}

	lockd_rename_vg_final(cmd, vg, 1);

	if (!backup(vg))
		stack;
	if (!backup_remove(cmd, vg_name))
		stack;

	unlock_vg(cmd, vg, vp->vg_name_new);
	vp->unlock_new_name = 0;

	log_print_unless_silent("Volume group \"%s\" successfully renamed to \"%s\"",
				vp->vg_name_old, vp->vg_name_new);
	return 1;

 error:
	unlock_vg(cmd, vg, vp->vg_name_new);
	vp->unlock_new_name = 0;

	lockd_rename_vg_final(cmd, vg, 0);

	return 0;
}

int vgrename(struct cmd_context *cmd, int argc, char **argv)
{
	struct vgrename_params vp = { 0 };
	struct processing_handle *handle;
	const char *vg_name_new;
	const char *vg_name_old;
	struct id id;
	int ret;

	if (argc != 2) {
		log_error("Old and new volume group names need specifying");
		return EINVALID_CMD_LINE;
	}

	vg_name_old = skip_dev_dir(cmd, argv[0], NULL);
	vg_name_new = skip_dev_dir(cmd, argv[1], NULL);

	if (!validate_vg_rename_params(cmd, vg_name_old, vg_name_new))
		return_0;

	if (!(vp.vg_name_old = dm_pool_strdup(cmd->mem, vg_name_old)))
		return_ECMD_FAILED;

	if (!(vp.vg_name_new = dm_pool_strdup(cmd->mem, vg_name_new)))
		return_ECMD_FAILED;

	/* Needed change the global VG namespace. */
	if (!lockd_gl(cmd, "ex", LDGL_UPDATE_NAMES))
		return_ECMD_FAILED;

	/*
	 * Special case where vg_name_old may be a UUID:
	 * If vg_name_old is a UUID, then process_each may
	 * translate it to an actual VG name that we don't
	 * yet know.  The lock ordering, and pre-locking,
	 * needs to be done based on VG names.  When
	 * vg_name_old is a UUID, do not do any pre-locking
	 * based on it, since it's likely to be wrong, and
	 * defer all the locking to the _single function.
	 *
	 * When it's not a UUID, we know the two VG names,
	 * and we can pre-lock the new VG name if the lock
	 * ordering wants it locked before the old VG name
	 * which will be locked by process_each.  If lock
	 * ordering wants the old name locked first, then
	 * the _single function will lock the new VG name.
	 */
	if (!(vp.old_name_is_uuid = id_read_format_try(&id, vg_name_old))) {
		if (strcmp(vg_name_new, vg_name_old) < 0) {
			vp.lock_vg_old_first = 0;
			vp.unlock_new_name = 1;

			if (!_lock_new_vg_for_rename(cmd, vg_name_new))
				return ECMD_FAILED;
		} else {
			/* The old VG is locked by process_each_vg. */
			vp.lock_vg_old_first = 1;
		}
	}

	if (!(handle = init_processing_handle(cmd, NULL))) {
		log_error("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	handle->custom_handle = &vp;

	ret = process_each_vg(cmd, 0, NULL, vg_name_old, NULL,
			      READ_FOR_UPDATE | READ_ALLOW_EXPORTED,
			      0, handle, _vgrename_single);

	/* Needed if process_each_vg returns error before calling _single. */
	if (vp.unlock_new_name)
		unlock_vg(cmd, NULL, vg_name_new);

	destroy_processing_handle(cmd, handle);
	return ret;
}

