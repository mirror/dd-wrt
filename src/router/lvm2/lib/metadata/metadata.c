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
#include "lib/device/device.h"
#include "lib/metadata/metadata.h"
#include "lib/commands/toolcontext.h"
#include "lib/misc/lvm-string.h"
#include "lib/misc/lvm-file.h"
#include "lib/cache/lvmcache.h"
#include "lib/mm/memlock.h"
#include "lib/datastruct/str_list.h"
#include "lib/metadata/pv_alloc.h"
#include "lib/metadata/segtype.h"
#include "lib/activate/activate.h"
#include "lib/display/display.h"
#include "lib/locking/locking.h"
#include "lib/format_text/archiver.h"
#include "lib/config/defaults.h"
#include "lib/locking/lvmlockd.h"
#include "time.h"
#include "lib/notify/lvmnotify.h"

#include <math.h>
#include <sys/param.h>

static struct physical_volume *_pv_read(struct cmd_context *cmd,
					const struct format_type *fmt,
					struct volume_group *vg,
					struct lvmcache_info *info);

static int _alignment_overrides_default(unsigned long data_alignment,
					unsigned long default_pe_align)
{
	return data_alignment && (default_pe_align % data_alignment);
}

unsigned long set_pe_align(struct physical_volume *pv, unsigned long data_alignment)
{
	unsigned long default_pe_align, temp_pe_align;

	if (pv->pe_align)
		goto out;

	if (data_alignment) {
		/* Always use specified data_alignment */
		pv->pe_align = data_alignment;
		goto out;
	}

	default_pe_align = find_config_tree_int(pv->fmt->cmd, devices_default_data_alignment_CFG, NULL);

	if (default_pe_align)
		/* align on 1 MiB multiple */
		default_pe_align *= DEFAULT_PE_ALIGN;
	else
		/* align on 64 KiB multiple (old default) */
		default_pe_align = DEFAULT_PE_ALIGN_OLD;

	pv->pe_align = MAX((default_pe_align << SECTOR_SHIFT),
			   lvm_getpagesize()) >> SECTOR_SHIFT;

	if (!pv->dev)
		goto out;

	/*
	 * Align to stripe-width of underlying md device if present
	 */
	if (find_config_tree_bool(pv->fmt->cmd, devices_md_chunk_alignment_CFG, NULL)) {
		temp_pe_align = dev_md_stripe_width(pv->fmt->cmd->dev_types, pv->dev);
		if (_alignment_overrides_default(temp_pe_align, default_pe_align))
			pv->pe_align = temp_pe_align;
	}

	/*
	 * Align to topology's minimum_io_size or optimal_io_size if present
	 * - minimum_io_size - the smallest request the device can perform
	 *   w/o incurring a read-modify-write penalty (e.g. MD's chunk size)
	 * - optimal_io_size - the device's preferred unit of receiving I/O
	 *   (e.g. MD's stripe width)
	 */
	if (find_config_tree_bool(pv->fmt->cmd, devices_data_alignment_detection_CFG, NULL)) {
		temp_pe_align = dev_minimum_io_size(pv->fmt->cmd->dev_types, pv->dev);
		if (_alignment_overrides_default(temp_pe_align, default_pe_align))
			pv->pe_align = temp_pe_align;

		temp_pe_align = dev_optimal_io_size(pv->fmt->cmd->dev_types, pv->dev);
		if (_alignment_overrides_default(temp_pe_align, default_pe_align))
			pv->pe_align = temp_pe_align;
	}

out:
	log_very_verbose("%s: Setting PE alignment to %lu sectors.",
			 dev_name(pv->dev), pv->pe_align);

	return pv->pe_align;
}

unsigned long set_pe_align_offset(struct physical_volume *pv,
				  unsigned long data_alignment_offset)
{
	if (pv->pe_align_offset)
		goto out;

	if (data_alignment_offset) {
		/* Always use specified data_alignment_offset */
		pv->pe_align_offset = data_alignment_offset;
		goto out;
	}

	if (!pv->dev)
		goto out;

	if (find_config_tree_bool(pv->fmt->cmd, devices_data_alignment_offset_detection_CFG, NULL)) {
		int align_offset = dev_alignment_offset(pv->fmt->cmd->dev_types, pv->dev);
		/* must handle a -1 alignment_offset; means dev is misaligned */
		if (align_offset < 0)
			align_offset = 0;
		pv->pe_align_offset = MAX(pv->pe_align_offset, align_offset);
	}

out:
	log_very_verbose("%s: Setting PE alignment offset to %lu sectors.",
			 dev_name(pv->dev), pv->pe_align_offset);

	return pv->pe_align_offset;
}

void add_pvl_to_vgs(struct volume_group *vg, struct pv_list *pvl)
{
	dm_list_add(&vg->pvs, &pvl->list);
	vg->pv_count++;
	pvl->pv->vg = vg;
	pv_set_fid(pvl->pv, vg->fid);
}

void del_pvl_from_vgs(struct volume_group *vg, struct pv_list *pvl)
{
	struct lvmcache_info *info;

	vg->pv_count--;
	dm_list_del(&pvl->list);

	pvl->pv->vg = vg->fid->fmt->orphan_vg; /* orphan */
	if ((info = lvmcache_info_from_pvid((const char *) &pvl->pv->id, pvl->pv->dev, 0)))
		lvmcache_fid_add_mdas(info, vg->fid->fmt->orphan_vg->fid,
				      (const char *) &pvl->pv->id, ID_LEN);
	pv_set_fid(pvl->pv, vg->fid->fmt->orphan_vg->fid);
}

/**
 * add_pv_to_vg - Add a physical volume to a volume group
 * @vg - volume group to add to
 * @pv_name - name of the pv (to be removed)
 * @pv - physical volume to add to volume group
 *
 * Returns:
 *  0 - failure
 *  1 - success
 * FIXME: remove pv_name - obtain safely from pv
 */
int add_pv_to_vg(struct volume_group *vg, const char *pv_name,
		 struct physical_volume *pv, int new_pv)
{
	struct pv_list *pvl;
	struct format_instance *fid = vg->fid;
	struct dm_pool *mem = vg->vgmem;
	char uuid[64] __attribute__((aligned(8)));
	int used;

	log_verbose("Adding physical volume '%s' to volume group '%s'",
		    pv_name, vg->name);

	if (!(pvl = dm_pool_zalloc(mem, sizeof(*pvl)))) {
		log_error("pv_list allocation for '%s' failed", pv_name);
		return 0;
	}

	if (!is_orphan_vg(pv->vg_name)) {
		log_error("Physical volume '%s' is already in volume group "
			  "'%s'", pv_name, pv->vg_name);
		return 0;
	}

	if (!new_pv) {
		if ((used = is_used_pv(pv)) < 0)
			return_0;

		if (used) {
			log_error("PV %s is used by a VG but its metadata is missing.", pv_name);
			return 0;
		}
	}

	if (pv->fmt != fid->fmt) {
		log_error("Physical volume %s is of different format type (%s)",
			  pv_name, pv->fmt->name);
		return 0;
	}

	/* Ensure PV doesn't depend on another PV already in the VG */
	if (pv_uses_vg(pv, vg)) {
		log_error("Physical volume %s might be constructed from same "
			  "volume group %s", pv_name, vg->name);
		return 0;
	}

	if (!(pv->vg_name = dm_pool_strdup(mem, vg->name))) {
		log_error("vg->name allocation failed for '%s'", pv_name);
		return 0;
	}

	memcpy(&pv->vgid, &vg->id, sizeof(vg->id));

	/* Units of 512-byte sectors */
	pv->pe_size = vg->extent_size;

	/*
	 * pe_count must always be calculated by pv_setup
	 */
	pv->pe_alloc_count = 0;

	/* LVM1 stores this outside a VG; LVM2 only stores it inside */
	/* FIXME Default from config file? vgextend cmdline flag? */
	pv->status |= ALLOCATABLE_PV;

	if (!fid->fmt->ops->pv_setup(fid->fmt, pv, vg)) {
		log_error("Format-specific setup of physical volume '%s' "
			  "failed.", pv_name);
		return 0;
	}

	if (find_pv_in_vg(vg, pv_name) ||
	    find_pv_in_vg_by_uuid(vg, &pv->id)) {
		if (!id_write_format(&pv->id, uuid, sizeof(uuid))) {
			stack;
			uuid[0] = '\0';
		}
		log_error("Physical volume '%s (%s)' already in the VG.",
			  pv_name, uuid);
		return 0;
	}

	if (vg->pv_count && (vg->pv_count == vg->max_pv)) {
		log_error("No space for '%s' - volume group '%s' "
			  "holds max %d physical volume(s).", pv_name,
			  vg->name, vg->max_pv);
		return 0;
	}

	if (!alloc_pv_segment_whole_pv(mem, pv))
		return_0;

	if ((uint64_t) vg->extent_count + pv->pe_count > MAX_EXTENT_COUNT) {
		log_error("Unable to add %s to %s: new extent count (%"
			  PRIu64 ") exceeds limit (%" PRIu32 ").",
			  pv_name, vg->name,
			  (uint64_t) vg->extent_count + pv->pe_count,
			  MAX_EXTENT_COUNT);
		return 0;
	}

	pvl->pv = pv;
	add_pvl_to_vgs(vg, pvl);
	vg->extent_count += pv->pe_count;
	vg->free_count += pv->pe_count;

	dm_list_iterate_items(pvl, &fid->fmt->orphan_vg->pvs)
		if (pv == pvl->pv) { /* unlink from orphan */
			dm_list_del(&pvl->list);
			break;
		}

	return 1;
}

static int _copy_pv(struct dm_pool *pvmem,
		    struct physical_volume *pv_to,
		    struct physical_volume *pv_from)
{
	memcpy(pv_to, pv_from, sizeof(*pv_to));

	/* We must use pv_set_fid here to update the reference counter! */
	pv_to->fid = NULL;
	pv_set_fid(pv_to, pv_from->fid);

	if (!(pv_to->vg_name = dm_pool_strdup(pvmem, pv_from->vg_name)))
		return_0;

	if (!str_list_dup(pvmem, &pv_to->tags, &pv_from->tags))
		return_0;

	if (!peg_dup(pvmem, &pv_to->segments, &pv_from->segments))
		return_0;

	return 1;
}

static struct pv_list *_copy_pvl(struct dm_pool *pvmem, struct pv_list *pvl_from)
{
	struct pv_list *pvl_to = NULL;

	if (!(pvl_to = dm_pool_zalloc(pvmem, sizeof(*pvl_to))))
		return_NULL;

	if (!(pvl_to->pv = dm_pool_alloc(pvmem, sizeof(*pvl_to->pv))))
		goto_bad;

	if (!_copy_pv(pvmem, pvl_to->pv, pvl_from->pv))
		goto_bad;

	return pvl_to;

bad:
	dm_pool_free(pvmem, pvl_to);
	return NULL;
}

static int _move_pv(struct volume_group *vg_from, struct volume_group *vg_to,
		    const char *pv_name, int enforce_pv_from_source)
{
	struct physical_volume *pv;
	struct pv_list *pvl;

	/* FIXME: handle tags */
	if (!(pvl = find_pv_in_vg(vg_from, pv_name))) {
		if (!enforce_pv_from_source &&
		    find_pv_in_vg(vg_to, pv_name))
			/*
			 * PV has already been moved.  This can happen if an
			 * LV is being moved that has multiple sub-LVs on the
			 * same PV.
			 */
			return 1;

		log_error("Physical volume %s not in volume group %s",
			  pv_name, vg_from->name);
		return 0;
	}

	if (vg_bad_status_bits(vg_from, RESIZEABLE_VG) ||
	    vg_bad_status_bits(vg_to, RESIZEABLE_VG))
		return 0;

	del_pvl_from_vgs(vg_from, pvl);
	add_pvl_to_vgs(vg_to, pvl);

	pv = pvl->pv;

	vg_from->extent_count -= pv_pe_count(pv);
	vg_to->extent_count += pv_pe_count(pv);

	vg_from->free_count -= pv_pe_count(pv) - pv_pe_alloc_count(pv);
	vg_to->free_count += pv_pe_count(pv) - pv_pe_alloc_count(pv);

	return 1;
}

int move_pv(struct volume_group *vg_from, struct volume_group *vg_to,
	    const char *pv_name)
{
	return _move_pv(vg_from, vg_to, pv_name, 1);
}

int move_pvs_used_by_lv(struct volume_group *vg_from,
			struct volume_group *vg_to,
			const char *lv_name)
{
	struct lv_segment *lvseg;
	unsigned s;
	struct lv_list *lvl;
	struct logical_volume *lv;

	/* FIXME: handle tags */
	if (!(lvl = find_lv_in_vg(vg_from, lv_name))) {
		log_error("Logical volume %s not in volume group %s",
			  lv_name, vg_from->name);
		return 0;
	}

	if (vg_bad_status_bits(vg_from, RESIZEABLE_VG) ||
	    vg_bad_status_bits(vg_to, RESIZEABLE_VG))
		return 0;

	dm_list_iterate_items(lvseg, &lvl->lv->segments) {
		if (lvseg->log_lv)
			if (!move_pvs_used_by_lv(vg_from, vg_to,
						     lvseg->log_lv->name))
				return_0;
		for (s = 0; s < lvseg->area_count; s++) {
			if (seg_type(lvseg, s) == AREA_PV) {
				if (!_move_pv(vg_from, vg_to,
					      pv_dev_name(seg_pv(lvseg, s)), 0))
					return_0;
			} else if (seg_type(lvseg, s) == AREA_LV) {
				lv = seg_lv(lvseg, s);
				if (!move_pvs_used_by_lv(vg_from, vg_to,
							     lv->name))
				    return_0;
			}
		}
	}
	return 1;
}

int validate_new_vg_name(struct cmd_context *cmd, const char *vg_name)
{
	static char vg_path[PATH_MAX];
	name_error_t name_error;

	name_error = validate_name_detailed(vg_name);
	if (NAME_VALID != name_error) {
		display_name_error(name_error);
		log_error("New volume group name \"%s\" is invalid.", vg_name);
		return 0;
	}

	snprintf(vg_path, sizeof(vg_path), "%s%s", cmd->dev_dir, vg_name);
	if (path_exists(vg_path)) {
		log_error("%s: already exists in filesystem", vg_path);
		return 0;
	}

	return 1;
}

int validate_vg_rename_params(struct cmd_context *cmd,
			      const char *vg_name_old,
			      const char *vg_name_new)
{
	unsigned length;
	char *dev_dir;

	dev_dir = cmd->dev_dir;
	length = strlen(dev_dir);

	/* Check sanity of new name */
	if (strlen(vg_name_new) > NAME_LEN - length - 2) {
		log_error("New volume group path exceeds maximum length "
			  "of %d!", NAME_LEN - length - 2);
		return 0;
	}

	if (!validate_new_vg_name(cmd, vg_name_new))
		return_0;

	if (!strcmp(vg_name_old, vg_name_new)) {
		log_error("Old and new volume group names must differ");
		return 0;
	}

	return 1;
}

int vg_rename(struct cmd_context *cmd, struct volume_group *vg,
	      const char *new_name)
{
	struct dm_pool *mem = vg->vgmem;
	struct pv_list *pvl;

	vg->old_name = vg->name;

	if (!(vg->name = dm_pool_strdup(mem, new_name))) {
		log_error("vg->name allocation failed for '%s'", new_name);
		return 0;
	}

	dm_list_iterate_items(pvl, &vg->pvs) {
		/* Skip if VG didn't change e.g. with vgsplit */
		if (pvl->pv->vg_name && !strcmp(new_name, pvl->pv->vg_name))
			continue;

		if (!(pvl->pv->vg_name = dm_pool_strdup(mem, new_name))) {
			log_error("pv->vg_name allocation failed for '%s'",
				  pv_dev_name(pvl->pv));
			return 0;
		}

                /* Mark the PVs that still hold metadata with the old VG name */
		log_debug_metadata("Marking PV %s as moved to VG %s", dev_name(pvl->pv->dev), new_name);
		pvl->pv->status |= PV_MOVED_VG;
	}

	return 1;
}

int vg_remove_check(struct volume_group *vg)
{
	unsigned lv_count;

	if (vg_read_error(vg) || vg_missing_pv_count(vg)) {
		log_error("Volume group \"%s\" not found, is inconsistent "
			  "or has PVs missing.", vg ? vg->name : "");
		log_error("Consider vgreduce --removemissing if metadata "
			  "is inconsistent.");
		return 0;
	}

	if (!vg_check_status(vg, EXPORTED_VG))
		return 0;

	lv_count = vg_visible_lvs(vg);

	if (lv_count) {
		log_error("Volume group \"%s\" still contains %u "
			  "logical volume(s)", vg->name, lv_count);
		return 0;
	}

	if (!archive(vg))
		return 0;

	return 1;
}

void vg_remove_pvs(struct volume_group *vg)
{
	struct pv_list *pvl, *tpvl;

	dm_list_iterate_items_safe(pvl, tpvl, &vg->pvs) {
		del_pvl_from_vgs(vg, pvl);
		dm_list_add(&vg->removed_pvs, &pvl->list);
	}
}

int vg_remove_direct(struct volume_group *vg)
{
	struct physical_volume *pv;
	struct pv_list *pvl;
	int ret = 1;

	if (!vg_remove_mdas(vg)) {
		log_error("vg_remove_mdas %s failed", vg->name);
		return 0;
	}

	/* init physical volumes */
	dm_list_iterate_items(pvl, &vg->removed_pvs) {
		pv = pvl->pv;
		if (is_missing_pv(pv))
			continue;

		log_verbose("Removing physical volume \"%s\" from "
			    "volume group \"%s\"", pv_dev_name(pv), vg->name);
		pv->vg_name = vg->fid->fmt->orphan_vg_name;
		pv->status &= ~ALLOCATABLE_PV;

		if (!dev_get_size(pv_dev(pv), &pv->size)) {
			log_error("%s: Couldn't get size.", pv_dev_name(pv));
			ret = 0;
			continue;
		}

		/* FIXME Write to same sector label was read from */
		if (!pv_write(vg->cmd, pv, 0)) {
			log_error("Failed to remove physical volume \"%s\""
				  " from volume group \"%s\"",
				  pv_dev_name(pv), vg->name);
			ret = 0;
		}
	}

	lockd_vg_update(vg);

	set_vg_notify(vg->cmd);

	if (!backup_remove(vg->cmd, vg->name))
		stack;

	if (ret)
		log_print_unless_silent("Volume group \"%s\" successfully removed", vg->name);
	else
		log_error("Volume group \"%s\" not properly removed", vg->name);

	return ret;
}

int vg_remove(struct volume_group *vg)
{
	int ret;

	ret = vg_remove_direct(vg);

	return ret;
}

int check_dev_block_size_for_vg(struct device *dev, const struct volume_group *vg,
				unsigned int *max_phys_block_size_found)
{
	unsigned int phys_block_size, block_size;

	if (!(dev_get_block_size(dev, &phys_block_size, &block_size)))
		return_0;

	if (phys_block_size > *max_phys_block_size_found)
		*max_phys_block_size_found = phys_block_size;

	if (phys_block_size >> SECTOR_SHIFT > vg->extent_size) {
		log_error("Physical extent size used for volume group %s "
			  "is less than physical block size that %s uses.",
			   vg->name, dev_name(dev));
		return 0;
	}

	return 1;
}

int vg_check_pv_dev_block_sizes(const struct volume_group *vg)
{
	struct pv_list *pvl;
	unsigned int max_phys_block_size_found = 0;

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (!check_dev_block_size_for_vg(pvl->pv->dev, vg, &max_phys_block_size_found))
			return 0;
	}

	return 1;
}

int check_pv_dev_sizes(struct volume_group *vg)
{
	struct pv_list *pvl;
	uint64_t dev_size, size;
	int r = 1;

	if (!vg->cmd->check_pv_dev_sizes ||
	    is_orphan_vg(vg->name))
		return 1;

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (is_missing_pv(pvl->pv))
			continue;
		/*
		 * Don't compare the sizes if we're not able
		 * to determine the real dev_size. This may
		 * happen if the device has gone since we did
		 * VG read.
		 */
		if (!dev_get_size(pvl->pv->dev, &dev_size))
			continue;
		size = pv_size(pvl->pv);

		if (dev_size < size) {
			log_warn("WARNING: Device %s has size of %" PRIu64 " sectors which "
				 "is smaller than corresponding PV size of %" PRIu64
				  " sectors. Was device resized?",
				  pv_dev_name(pvl->pv), dev_size, size);
			r = 0;
		}
	}

	return r;
}

int vg_extend_each_pv(struct volume_group *vg, struct pvcreate_params *pp)
{
	struct pv_list *pvl;
	unsigned int max_phys_block_size = 0;

	log_debug_metadata("Adding PVs to VG %s.", vg->name);

	if (vg_bad_status_bits(vg, RESIZEABLE_VG))
		return_0;

	dm_list_iterate_items(pvl, &pp->pvs) {
		log_debug_metadata("Adding PV %s to VG %s.", pv_dev_name(pvl->pv), vg->name);

		if (!(check_dev_block_size_for_vg(pvl->pv->dev,
						  (const struct volume_group *) vg,
						  &max_phys_block_size))) {
			log_error("PV %s has wrong block size.", pv_dev_name(pvl->pv));
			return 0;
		}

		if (!add_pv_to_vg(vg, pv_dev_name(pvl->pv), pvl->pv, 0)) {
			log_error("PV %s cannot be added to VG %s.",
				  pv_dev_name(pvl->pv), vg->name);
			return 0;
		}
	}

	(void) check_pv_dev_sizes(vg);

	dm_list_splice(&vg->pv_write_list, &pp->pvs);

	return 1;
}

int lv_change_tag(struct logical_volume *lv, const char *tag, int add_tag)
{
	char *tag_new;

	if (!(lv->vg->fid->fmt->features & FMT_TAGS)) {
		log_error("Logical volume %s/%s does not support tags",
			  lv->vg->name, lv->name);
		return 0;
	}

	if (add_tag) {
		if (!(tag_new = dm_pool_strdup(lv->vg->vgmem, tag))) {
			log_error("Failed to duplicate tag %s from %s/%s",
				  tag, lv->vg->name, lv->name);
			return 0;
		}
		if (!str_list_add(lv->vg->vgmem, &lv->tags, tag_new)) {
			log_error("Failed to add tag %s to %s/%s",
				  tag, lv->vg->name, lv->name);
			return 0;
		}
	} else
		str_list_del(&lv->tags, tag);

	return 1;
}

int vg_change_tag(struct volume_group *vg, const char *tag, int add_tag)
{
	char *tag_new;

	if (!(vg->fid->fmt->features & FMT_TAGS)) {
		log_error("Volume group %s does not support tags", vg->name);
		return 0;
	}

	if (add_tag) {
		if (!(tag_new = dm_pool_strdup(vg->vgmem, tag))) {
			log_error("Failed to duplicate tag %s from %s",
				  tag, vg->name);
			return 0;
		}
		if (!str_list_add(vg->vgmem, &vg->tags, tag_new)) {
			log_error("Failed to add tag %s to volume group %s",
				  tag, vg->name);
			return 0;
		}
	} else
		str_list_del(&vg->tags, tag);

	return 1;
}

const char *strip_dir(const char *vg_name, const char *dev_dir)
{
	size_t len = strlen(dev_dir);
	if (!strncmp(vg_name, dev_dir, len))
		vg_name += len;

	return vg_name;
}

/*
 * Validates major and minor numbers.
 * On >2.4 kernel we only support dynamic major number.
 */
int validate_major_minor(const struct cmd_context *cmd,
			 const struct format_type *fmt,
			 int32_t major, int32_t minor)
{
	int r = 1;

	if (!strncmp(cmd->kernel_vsn, "2.4.", 4) ||
	    (fmt->features & FMT_RESTRICTED_LVIDS)) {
		if (major < 0 || major > 255) {
			log_error("Major number %d outside range 0-255.", major);
			r = 0;
		}
		if (minor < 0 || minor > 255) {
			log_error("Minor number %d outside range 0-255.", minor);
			r = 0;
		}
	} else {
		/* 12 bits for major number */
		if ((major != -1) &&
		    (major != cmd->dev_types->device_mapper_major)) {
			/* User supplied some major number */
			if (major < 0 || major > 4095) {
				log_error("Major number %d outside range 0-4095.", major);
				r = 0;
			} else
				log_print_unless_silent("Ignoring supplied major %d number - "
							"kernel assigns major numbers dynamically.",
							major);
		}
		/* 20 bits for minor number */
		if (minor < 0 || minor > 1048575) {
			log_error("Minor number %d outside range 0-1048575.", minor);
			r = 0;
		}
	}

	return r;
}

/*
 * Validate parameters to vg_create() before calling.
 * FIXME: Move inside vg_create library function.
 * FIXME: Change vgcreate_params struct to individual gets/sets
 */
int vgcreate_params_validate(struct cmd_context *cmd,
			     struct vgcreate_params *vp)
{
	if (!validate_new_vg_name(cmd, vp->vg_name))
		return_0;

	if (vp->alloc == ALLOC_INHERIT) {
		log_error("Volume Group allocation policy cannot inherit "
			  "from anything");
		return 0;
	}

	if (!vp->extent_size) {
		log_error("Physical extent size may not be zero");
		return 0;
	}

	if (!(cmd->fmt->features & FMT_UNLIMITED_VOLS)) {
		if (!vp->max_lv)
			vp->max_lv = 255;
		if (!vp->max_pv)
			vp->max_pv = 255;
		if (vp->max_lv > 255 || vp->max_pv > 255) {
			log_error("Number of volumes may not exceed 255");
			return 0;
		}
	}

	return 1;
}

static void _vg_wipe_cached_precommitted(struct volume_group *vg)
{
	release_vg(vg->vg_precommitted);
	vg->vg_precommitted = NULL;
}

static void _vg_move_cached_precommitted_to_committed(struct volume_group *vg)
{
	release_vg(vg->vg_committed);
	vg->vg_committed = vg->vg_precommitted;
	vg->vg_precommitted = NULL;
}

/*
 * Update content of precommitted VG
 *
 * TODO: Optimize in the future, since lvmetad needs similar
 *       config tree processing in lvmetad_vg_update().
 */
static int _vg_update_embedded_copy(struct volume_group *vg, struct volume_group **vg_embedded)
{
	struct dm_config_tree *cft;

	_vg_wipe_cached_precommitted(vg);

	/* Copy the VG using an export followed by import */
	if (!(cft = export_vg_to_config_tree(vg)))
		return_0;

	if (!(*vg_embedded = import_vg_from_config_tree(cft, vg->fid))) {
		dm_config_destroy(cft);
		return_0;
	}

	dm_config_destroy(cft);

	return 1;
}

/*
 * Create a (struct volume_group) volume group handle from a struct volume_group pointer and a
 * possible failure code or zero for success.
 */
static struct volume_group *_vg_make_handle(struct cmd_context *cmd,
					    struct volume_group *vg,
					    uint32_t failure)
{
	/* Never return a cached VG structure for a failure */
	if (vg && vg->vginfo && failure != SUCCESS) {
		release_vg(vg);
		vg = NULL;
	}

	if (!vg && !(vg = alloc_vg("vg_make_handle", cmd, NULL)))
		return_NULL;

	vg->read_status = failure;

	/*
	 * If we hold a write lock and might be changing the VG contents, embed a pristine 
	 * copy of the VG metadata for the activation code to use later
	 */
	if (vg->fid && !dm_pool_locked(vg->vgmem) && !vg->vg_committed && !is_orphan_vg(vg->name))
		if (vg_write_lock_held() && !_vg_update_embedded_copy(vg, &vg->vg_committed))
			vg->read_status |= FAILED_ALLOCATION;

	return vg;
}

int lv_has_unknown_segments(const struct logical_volume *lv)
{
	struct lv_segment *seg;
	/* foreach segment */
	dm_list_iterate_items(seg, &lv->segments)
		if (seg_unknown(seg))
			return 1;
	return 0;
}

int vg_has_unknown_segments(const struct volume_group *vg)
{
	struct lv_list *lvl;

	/* foreach LV */
	dm_list_iterate_items(lvl, &vg->lvs)
		if (lv_has_unknown_segments(lvl->lv))
			return 1;
	return 0;
}

struct volume_group *vg_lock_and_create(struct cmd_context *cmd, const char *vg_name)
{
	uint32_t rc;
	struct volume_group *vg;

	if (!validate_name(vg_name)) {
		log_error("Invalid vg name %s", vg_name);
		/* FIXME: use _vg_make_handle() w/proper error code */
		return NULL;
	}

	rc = vg_lock_newname(cmd, vg_name);
	if (rc != SUCCESS)
		/* NOTE: let caller decide - this may be check for existence */
		return _vg_make_handle(cmd, NULL, rc);

	vg = vg_create(cmd, vg_name);
	if (!vg || vg_read_error(vg))
		unlock_vg(cmd, NULL, vg_name);

	return vg;
}

/*
 * Create a VG with default parameters.
 * Returns:
 * - struct volume_group* with SUCCESS code: VG structure created
 * - NULL or struct volume_group* with FAILED_* code: error creating VG structure
 * Use vg_read_error() to determine success or failure.
 * FIXME: cleanup usage of _vg_make_handle()
 */
struct volume_group *vg_create(struct cmd_context *cmd, const char *vg_name)
{
	struct volume_group *vg;
	struct format_instance_ctx fic = {
		.type = FMT_INSTANCE_MDAS | FMT_INSTANCE_AUX_MDAS,
		.context.vg_ref.vg_name = vg_name
	};
	struct format_instance *fid;

	if (!(vg = alloc_vg("vg_create", cmd, vg_name)))
		goto_bad;

	if (!id_create(&vg->id)) {
		log_error("Couldn't create uuid for volume group '%s'.",
			  vg_name);
		goto bad;
	}

	vg->status = (RESIZEABLE_VG | LVM_READ | LVM_WRITE);
	vg->system_id = NULL;

	vg->extent_size = DEFAULT_EXTENT_SIZE * 2;
	vg->max_lv = DEFAULT_MAX_LV;
	vg->max_pv = DEFAULT_MAX_PV;
	vg->alloc = DEFAULT_ALLOC_POLICY;
	vg->mda_copies = DEFAULT_VGMETADATACOPIES;

	if (!(fid = cmd->fmt->ops->create_instance(cmd->fmt, &fic))) {
		log_error("Failed to create format instance");
		goto bad;
	}
	vg_set_fid(vg, fid);

	if (vg->fid->fmt->ops->vg_setup &&
	    !vg->fid->fmt->ops->vg_setup(vg->fid, vg)) {
		log_error("Format specific setup of volume group '%s' failed.",
			  vg_name);
		goto bad;
	}
	return _vg_make_handle(cmd, vg, SUCCESS);

bad:
	unlock_and_release_vg(cmd, vg, vg_name);
	/* FIXME: use _vg_make_handle() w/proper error code */
	return NULL;
}

/* Rounds up by default */
uint32_t extents_from_size(struct cmd_context *cmd, uint64_t size,
			   uint32_t extent_size)
{
	if (size % extent_size) {
		size += extent_size - size % extent_size;
		log_print_unless_silent("Rounding up size to full physical extent %s",
			  		display_size(cmd, size));
	}

	if (size > (uint64_t) MAX_EXTENT_COUNT * extent_size) {
		log_error("Volume too large (%s) for extent size %s. "
			  "Upper limit is less than %s.",
			  display_size(cmd, size),
			  display_size(cmd, (uint64_t) extent_size),
			  display_size(cmd, (uint64_t) MAX_EXTENT_COUNT *
				       extent_size));
		return 0;
	}

	return (uint32_t) (size / extent_size);
}

/*
 * Converts size according to percentage with specified rounding to extents
 *
 * For PERCENT_NONE size is in standard sector units.
 * For all other percent type is in DM_PERCENT_1 base unit (supports decimal point)
 *
 * Return value of 0 extents is an error.
 */
uint32_t extents_from_percent_size(struct volume_group *vg, const struct dm_list *pvh,
				   uint32_t extents, int roundup,
				   percent_type_t percent, uint64_t size)
{
	uint32_t count;

	switch (percent) {
	case PERCENT_NONE:
		if (!roundup && (size % vg->extent_size)) {
			if (!(size -= size % vg->extent_size)) {
				log_error("Specified size is smaller then physical extent boundary.");
				return 0;
			}
			log_print_unless_silent("Rounding size to boundary between physical extents: %s.",
						display_size(vg->cmd, size));
		}
		return extents_from_size(vg->cmd, size, vg->extent_size);
	case PERCENT_LV:
		break;	/* Base extents already passed in. */
	case PERCENT_VG:
		extents = vg->extent_count;
		break;
	case PERCENT_PVS:
		if (pvh != &vg->pvs) {
			/* Physical volumes are specified on cmdline */
			if (!(extents = pv_list_extents_free(pvh))) {
				log_error("No free extents in the list of physical volumes.");
				return 0;
			}
			break;
		}
		/* fall through to use all PVs in VG like %FREE */
	case PERCENT_FREE:
		if (!(extents = vg->free_count)) {
			log_error("No free extents in Volume group %s.", vg->name);
			return 0;
		}
		break;
	default:
		log_error(INTERNAL_ERROR "Unsupported percent type %u.", percent);
		return 0;
	}

	if (!(count = percent_of_extents(size, extents, roundup)))
		log_error("Converted  %s%%%s into 0 extents.",
			  display_percent(vg->cmd, size), get_percent_string(percent));
	else
		log_verbose("Converted %s%%%s into %" PRIu32 " extents.",
			    display_percent(vg->cmd, size), get_percent_string(percent), count);

	return count;
}

static dm_bitset_t _bitset_with_random_bits(struct dm_pool *mem, uint32_t num_bits,
					    uint32_t num_set_bits, unsigned *seed)
{
	dm_bitset_t bs;
	unsigned bit_selected;
	char buf[32];
	uint32_t i = num_bits - num_set_bits;

	if (!(bs = dm_bitset_create(mem, num_bits))) {
		log_error("Failed to allocate bitset for setting random bits.");
		return NULL;
	}

        if (!dm_pool_begin_object(mem, 512)) {
                log_error("dm_pool_begin_object failed for random list of bits.");
		dm_pool_free(mem, bs);
                return NULL;
        }

	/* Perform loop num_set_bits times, selecting one bit each time */
	while (i++ < num_bits) {
		/* Select a random bit between 0 and (i-1) inclusive. */
		bit_selected = lvm_even_rand(seed, i);

		/*
		 * If the bit was already set, set the new bit that became
		 * choosable for the first time during this pass.
		 * This maintains a uniform probability distribution by compensating
		 * for being unable to select it until this pass.
		 */
		if (dm_bit(bs, bit_selected))
			bit_selected = i - 1;

		dm_bit_set(bs, bit_selected);

		if (dm_snprintf(buf, sizeof(buf), "%u ", bit_selected) < 0) {
			log_error("snprintf random bit failed.");
			dm_pool_free(mem, bs);
                	return NULL;
		}
		if (!dm_pool_grow_object(mem, buf, strlen(buf))) {
			log_error("Failed to generate list of random bits.");
			dm_pool_free(mem, bs);
                	return NULL;
		}
	}

	if (!dm_pool_grow_object(mem, "\0", 1)) {
		log_error("Failed to finish list of random bits.");
		dm_pool_free(mem, bs);
		return NULL;
	}

	log_debug_metadata("Selected %" PRIu32 " random bits from %" PRIu32 ": %s", num_set_bits, num_bits, (char *) dm_pool_end_object(mem));

	return bs;
}

static int _vg_ignore_mdas(struct volume_group *vg, uint32_t num_to_ignore)
{
	struct metadata_area *mda;
	uint32_t mda_used_count = vg_mda_used_count(vg);
	dm_bitset_t mda_to_ignore_bs;
	int r = 1;

	log_debug_metadata("Adjusting ignored mdas for %s: %" PRIu32 " of %" PRIu32 " mdas in use "
			   "but %" PRIu32 " required.  Changing %" PRIu32 " mda.",
			   vg->name, mda_used_count, vg_mda_count(vg), vg_mda_copies(vg), num_to_ignore);

	if (!num_to_ignore)
		return 1;

	if (!(mda_to_ignore_bs = _bitset_with_random_bits(vg->vgmem, mda_used_count,
							  num_to_ignore, &vg->cmd->rand_seed)))
		return_0;

	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use)
		if (!mda_is_ignored(mda) && (--mda_used_count,
		    dm_bit(mda_to_ignore_bs, mda_used_count))) {
			mda_set_ignored(mda, 1);
			if (!--num_to_ignore)
				goto out;
		}

	log_error(INTERNAL_ERROR "Unable to find %"PRIu32" metadata areas to ignore "
		  "on volume group %s", num_to_ignore, vg->name);

	r = 0;

out:
	dm_pool_free(vg->vgmem, mda_to_ignore_bs);
	return r;
}

static int _vg_unignore_mdas(struct volume_group *vg, uint32_t num_to_unignore)
{
	struct metadata_area *mda, *tmda;
	uint32_t mda_used_count = vg_mda_used_count(vg);
	uint32_t mda_count = vg_mda_count(vg);
	uint32_t mda_free_count = mda_count - mda_used_count;
	dm_bitset_t mda_to_unignore_bs;
	int r = 1;

	if (!num_to_unignore)
		return 1;

	log_debug_metadata("Adjusting ignored mdas for %s: %" PRIu32 " of %" PRIu32 " mdas in use "
			   "but %" PRIu32 " required.  Changing %" PRIu32 " mda.",
			   vg->name, mda_used_count, mda_count, vg_mda_copies(vg), num_to_unignore);

	if (!(mda_to_unignore_bs = _bitset_with_random_bits(vg->vgmem, mda_free_count,
							    num_to_unignore, &vg->cmd->rand_seed)))
		return_0;

	dm_list_iterate_items_safe(mda, tmda, &vg->fid->metadata_areas_ignored)
		if (mda_is_ignored(mda) && (--mda_free_count,
		    dm_bit(mda_to_unignore_bs, mda_free_count))) {
			mda_set_ignored(mda, 0);
			dm_list_move(&vg->fid->metadata_areas_in_use,
				     &mda->list);
			if (!--num_to_unignore)
				goto out;
		}

	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use)
		if (mda_is_ignored(mda) && (--mda_free_count,
		    dm_bit(mda_to_unignore_bs, mda_free_count))) {
			mda_set_ignored(mda, 0);
			if (!--num_to_unignore)
				goto out;
		}

	log_error(INTERNAL_ERROR "Unable to find %"PRIu32" metadata areas to unignore "
		 "on volume group %s", num_to_unignore, vg->name);

	r = 0;

out:
	dm_pool_free(vg->vgmem, mda_to_unignore_bs);
	return r;
}

static int _vg_adjust_ignored_mdas(struct volume_group *vg)
{
	uint32_t mda_copies_used = vg_mda_used_count(vg);

	if (vg->mda_copies == VGMETADATACOPIES_UNMANAGED) {
		/* Ensure at least one mda is in use. */
		if (!mda_copies_used && vg_mda_count(vg) && !_vg_unignore_mdas(vg, 1))
			return_0;
		else
			return 1;
	}


	/* Not an error to have vg_mda_count larger than total mdas. */
	if (vg->mda_copies == VGMETADATACOPIES_ALL ||
	    vg->mda_copies >= vg_mda_count(vg)) {
		/* Use all */
		if (!_vg_unignore_mdas(vg, vg_mda_count(vg) - mda_copies_used))
			return_0;
	} else if (mda_copies_used < vg->mda_copies) {
		if (!_vg_unignore_mdas(vg, vg->mda_copies - mda_copies_used))
			return_0;
	} else if (mda_copies_used > vg->mda_copies)
		if (!_vg_ignore_mdas(vg, mda_copies_used - vg->mda_copies))
			return_0;

	/*
	 * The VGMETADATACOPIES_ALL value will never be written disk.
	 * It is a special cmdline value that means 2 things:
	 * 1. clear all ignore bits in all mdas in this vg
	 * 2. set the "unmanaged" policy going forward for metadata balancing
	 */
	if (vg->mda_copies == VGMETADATACOPIES_ALL)
		vg->mda_copies = VGMETADATACOPIES_UNMANAGED;

	return 1;
}

uint64_t find_min_mda_size(struct dm_list *mdas)
{
	uint64_t min_mda_size = UINT64_MAX, mda_size;
	struct metadata_area *mda;

	dm_list_iterate_items(mda, mdas) {
		if (!mda->ops->mda_total_sectors)
			continue;
		mda_size = mda->ops->mda_total_sectors(mda);
		if (mda_size < min_mda_size)
			min_mda_size = mda_size;
	}

	if (min_mda_size == UINT64_MAX)
		min_mda_size = UINT64_C(0);

	return min_mda_size;
}

static int _move_mdas(struct volume_group *vg_from, struct volume_group *vg_to,
		      struct dm_list *mdas_from, struct dm_list *mdas_to)
{
	struct metadata_area *mda, *mda2;
	int common_mda = 0;

	dm_list_iterate_items_safe(mda, mda2, mdas_from) {
		if (!mda->ops->mda_in_vg) {
			common_mda = 1;
			continue;
		}

		if (!mda->ops->mda_in_vg(vg_from->fid, vg_from, mda)) {
			if (is_orphan_vg(vg_to->name))
				dm_list_del(&mda->list);
			else
				dm_list_move(mdas_to, &mda->list);
		}
	}
	return common_mda;
}

/*
 * Separate metadata areas after splitting a VG.
 * Also accepts orphan VG as destination (for vgreduce).
 */
int vg_split_mdas(struct cmd_context *cmd __attribute__((unused)),
		  struct volume_group *vg_from, struct volume_group *vg_to)
{
	struct dm_list *mdas_from_in_use, *mdas_to_in_use;
	struct dm_list *mdas_from_ignored, *mdas_to_ignored;
	int common_mda = 0;

	mdas_from_in_use = &vg_from->fid->metadata_areas_in_use;
	mdas_from_ignored = &vg_from->fid->metadata_areas_ignored;
	mdas_to_in_use = &vg_to->fid->metadata_areas_in_use;
	mdas_to_ignored = &vg_to->fid->metadata_areas_ignored;

	common_mda = _move_mdas(vg_from, vg_to,
				mdas_from_in_use, mdas_to_in_use);
	common_mda = _move_mdas(vg_from, vg_to,
				mdas_from_ignored, mdas_to_ignored);

	if ((dm_list_empty(mdas_from_in_use) &&
	     dm_list_empty(mdas_from_ignored)) ||
	    ((!is_orphan_vg(vg_to->name) &&
	      dm_list_empty(mdas_to_in_use) &&
	      dm_list_empty(mdas_to_ignored))))
		return common_mda;

	return 1;
}

void pvcreate_params_set_defaults(struct pvcreate_params *pp)
{
	memset(pp, 0, sizeof(*pp));

	pp->zero = 1;
	pp->force = PROMPT;
	pp->yes = 0;
	pp->restorefile = NULL;
	pp->uuid_str = NULL;

	pp->pva.size = 0;
	pp->pva.data_alignment = UINT64_C(0);
	pp->pva.data_alignment_offset = UINT64_C(0);
	pp->pva.pvmetadatacopies = DEFAULT_PVMETADATACOPIES;
	pp->pva.pvmetadatasize = DEFAULT_PVMETADATASIZE;
	pp->pva.label_sector = DEFAULT_LABELSECTOR;
	pp->pva.metadataignore = DEFAULT_PVMETADATAIGNORE;
	pp->pva.ba_start = 0;
	pp->pva.ba_size = 0;
	pp->pva.pe_start = PV_PE_START_CALC;
	pp->pva.extent_count = 0;
	pp->pva.extent_size = 0;

	dm_list_init(&pp->prompts);
	dm_list_init(&pp->arg_devices);
	dm_list_init(&pp->arg_process);
	dm_list_init(&pp->arg_confirm);
	dm_list_init(&pp->arg_create);
	dm_list_init(&pp->arg_remove);
	dm_list_init(&pp->arg_fail);
	dm_list_init(&pp->pvs);
}

static struct physical_volume *_alloc_pv(struct dm_pool *mem, struct device *dev)
{
	struct physical_volume *pv;

	if (!(pv = dm_pool_zalloc(mem, sizeof(*pv)))) {
		log_error("Failed to allocate pv structure.");
		return NULL;
	}

	pv->dev = dev;

	dm_list_init(&pv->tags);
	dm_list_init(&pv->segments);

	return pv;
}

/**
 * pv_create - initialize a physical volume for use with a volume group
 * created PV belongs to Orphan VG.
 *
 * Returns:
 *   PV handle - physical volume initialized successfully
 *   NULL - invalid parameter or problem initializing the physical volume
 */

struct physical_volume *pv_create(const struct cmd_context *cmd,
				  struct device *dev,
				  struct pv_create_args *pva)
{
	const struct format_type *fmt = cmd->fmt;
	struct dm_pool *mem = fmt->orphan_vg->vgmem;
	struct physical_volume *pv = _alloc_pv(mem, dev);
	unsigned mda_index;
	struct pv_list *pvl;
	uint64_t size = pva->size;
	unsigned long data_alignment = pva->data_alignment;
	unsigned long data_alignment_offset = pva->data_alignment_offset;
	unsigned pvmetadatacopies = pva->pvmetadatacopies;
	uint64_t pvmetadatasize = pva->pvmetadatasize;
	unsigned metadataignore = pva->metadataignore;

	if (!pv)
		return_NULL;

	if (pva->idp)
		memcpy(&pv->id, pva->idp, sizeof(*pva->idp));
	else if (!id_create(&pv->id)) {
		log_error("Failed to create random uuid for %s.",
			  dev_name(dev));
		goto bad;
	}

	if (!dev_get_size(pv->dev, &pv->size)) {
		log_error("%s: Couldn't get size.", pv_dev_name(pv));
		goto bad;
	}

	if (size) {
		if (size > pv->size)
			log_warn("WARNING: %s: Overriding real size. "
				  "You could lose data.", pv_dev_name(pv));
		log_verbose("%s: Pretending size is %" PRIu64 " sectors.",
			    pv_dev_name(pv), size);
		pv->size = size;
	}

	if (pv->size < pv_min_size()) {
		log_error("%s: Size must exceed minimum of %" PRIu64 " sectors.",
			  pv_dev_name(pv), pv_min_size());
		goto bad;
	}

	if (pv->size < data_alignment + data_alignment_offset) {
		log_error("%s: Data alignment must not exceed device size.",
			  pv_dev_name(pv));
		goto bad;
	}

	if (!(pvl = dm_pool_zalloc(mem, sizeof(*pvl)))) {
		log_error("pv_list allocation in pv_create failed");
		goto bad;
	}

	pvl->pv = pv;
	add_pvl_to_vgs(fmt->orphan_vg, pvl);
	fmt->orphan_vg->extent_count += pv->pe_count;
	fmt->orphan_vg->free_count += pv->pe_count;

	pv->fmt = fmt;
	pv->vg_name = fmt->orphan_vg_name;

	if (!fmt->ops->pv_initialise(fmt, pva, pv)) {
		log_error("Format-specific initialisation of physical "
			  "volume %s failed.", pv_dev_name(pv));
		goto bad;
	}

	for (mda_index = 0; mda_index < pvmetadatacopies; mda_index++) {
		if (pv->fmt->ops->pv_add_metadata_area &&
		    !pv->fmt->ops->pv_add_metadata_area(pv->fmt, pv,
					pva->pe_start != PV_PE_START_CALC,
					mda_index, pvmetadatasize,
					metadataignore)) {
			log_error("Failed to add metadata area for "
				  "new physical volume %s", pv_dev_name(pv));
			goto bad;
		}
	}

	return pv;

      bad:
	// FIXME: detach from orphan in error path
	//free_pv_fid(pv);
	//dm_pool_free(mem, pv);
	return NULL;
}

/* FIXME: liblvm todo - make into function that returns handle */
struct pv_list *find_pv_in_vg(const struct volume_group *vg,
			       const char *pv_name)
{
	struct pv_list *pvl;
	struct device *dev = dev_cache_get(vg->cmd, pv_name, vg->cmd->filter);

	/*
	 * If the device does not exist or is filtered out, don't bother trying
	 * to find it in the list. This also prevents accidentally finding a
	 * non-NULL PV which happens to be missing (i.e. its pv->dev is NULL)
	 * for such devices.
	 */
	if (!dev)
		return NULL;

	dm_list_iterate_items(pvl, &vg->pvs)
		if (pvl->pv->dev == dev)
			return pvl;

	return NULL;
}

struct pv_list *find_pv_in_pv_list(const struct dm_list *pl,
				   const struct physical_volume *pv)
{
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, pl)
		if (pvl->pv == pv)
			return pvl;

	return NULL;
}

int pv_is_in_vg(struct volume_group *vg, struct physical_volume *pv)
{
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, &vg->pvs)
		if (pv == pvl->pv)
			 return 1;

	return 0;
}

/**
 * find_pv_in_vg_by_uuid - Find PV in VG by PV UUID
 * @vg: volume group to search
 * @id: UUID of the PV to match
 *
 * Returns:
 *   struct pv_list within owning struct volume_group - if UUID of PV found in VG
 *   NULL - invalid parameter or UUID of PV not found in VG
 *
 * Note
 *   FIXME - liblvm todo - make into function that takes VG handle
 */
struct pv_list *find_pv_in_vg_by_uuid(const struct volume_group *vg,
				      const struct id *id)
{
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, &vg->pvs)
		if (id_equal(&pvl->pv->id, id))
			return pvl;

	return NULL;
}

struct lv_list *find_lv_in_vg(const struct volume_group *vg,
			      const char *lv_name)
{
	struct lv_list *lvl;
	const char *ptr;

	/* Use last component */
	if ((ptr = strrchr(lv_name, '/')))
		ptr++;
	else
		ptr = lv_name;

	dm_list_iterate_items(lvl, &vg->lvs)
		if (!strcmp(lvl->lv->name, ptr))
			return lvl;

	return NULL;
}

struct lv_list *find_lv_in_lv_list(const struct dm_list *ll,
				   const struct logical_volume *lv)
{
	struct lv_list *lvl;

	dm_list_iterate_items(lvl, ll)
		if (lvl->lv == lv)
			return lvl;

	return NULL;
}

struct logical_volume *find_lv_in_vg_by_lvid(struct volume_group *vg,
					     const union lvid *lvid)
{
	struct lv_list *lvl;

	dm_list_iterate_items(lvl, &vg->lvs)
		if (!strncmp(lvl->lv->lvid.s, lvid->s, sizeof(*lvid)))
			return lvl->lv;

	return NULL;
}

struct logical_volume *find_lv(const struct volume_group *vg,
			       const char *lv_name)
{
	struct lv_list *lvl = find_lv_in_vg(vg, lv_name);
	return lvl ? lvl->lv : NULL;
}

struct generic_logical_volume *find_historical_glv(const struct volume_group *vg,
					       const char *historical_lv_name,
					       int check_removed_list,
					       struct glv_list **glvl_found)
{
	struct glv_list *glvl;
	const char *ptr;
	const struct dm_list *list = check_removed_list ? &vg->removed_historical_lvs
							: &vg->historical_lvs;

	/* Use last component */
	if ((ptr = strrchr(historical_lv_name, '/')))
		ptr++;
	else
		ptr = historical_lv_name;

	dm_list_iterate_items(glvl, list) {
		if (!strcmp(glvl->glv->historical->name, ptr)) {
			if (glvl_found)
				*glvl_found = glvl;
			return glvl->glv;
		}
	}

	if (glvl_found)
		*glvl_found = NULL;
	return NULL;
}

int lv_name_is_used_in_vg(const struct volume_group *vg, const char *name, int *historical)
{
	int found = 0;

	if (find_lv(vg, name)) {
		found = 1;
		if (historical)
			*historical = 0;
	} else if (find_historical_glv(vg, name, 0, NULL)) {
		found = 1;
		if (historical)
			*historical = 1;
	}

	return found;
}

struct physical_volume *find_pv(struct volume_group *vg, struct device *dev)
{
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, &vg->pvs)
		if (dev == pvl->pv->dev)
			return pvl->pv;

	return NULL;
}

/* Find segment at a given logical extent in an LV */
struct lv_segment *find_seg_by_le(const struct logical_volume *lv, uint32_t le)
{
	struct lv_segment *seg;

	dm_list_iterate_items(seg, &lv->segments)
		if (le >= seg->le && le < seg->le + seg->len)
			return seg;

	return NULL;
}

struct lv_segment *first_seg(const struct logical_volume *lv)
{
	struct lv_segment *seg;

	dm_list_iterate_items(seg, &lv->segments)
		return seg;

	return NULL;
}

struct lv_segment *last_seg(const struct logical_volume *lv)
{
	struct lv_segment *seg;

	dm_list_iterate_back_items(seg, &lv->segments)
		return seg;

	return NULL;
}

int vg_remove_mdas(struct volume_group *vg)
{
	struct metadata_area *mda;

	/* FIXME Improve recovery situation? */
	/* Remove each copy of the metadata */
	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use) {
		if (mda->ops->vg_remove &&
		    !mda->ops->vg_remove(vg->fid, vg, mda))
			return_0;
	}

	return 1;
}

/*
 * Determine whether two vgs are compatible for merging.
 */
int vgs_are_compatible(struct cmd_context *cmd __attribute__((unused)),
		       struct volume_group *vg_from,
		       struct volume_group *vg_to)
{
	struct lv_list *lvl1, *lvl2;
	struct pv_list *pvl;
	const char *name1, *name2;

	if (lvs_in_vg_activated(vg_from)) {
		log_error("Logical volumes in \"%s\" must be inactive",
			  vg_from->name);
		return 0;
	}

	/* Check compatibility */
	if (vg_to->extent_size != vg_from->extent_size) {
		log_error("Extent sizes differ: %d (%s) and %d (%s)",
			  vg_to->extent_size, vg_to->name,
			  vg_from->extent_size, vg_from->name);
		return 0;
	}

	if (vg_to->max_pv &&
	    (vg_to->max_pv < vg_to->pv_count + vg_from->pv_count)) {
		log_error("Maximum number of physical volumes (%d) exceeded "
			  " for \"%s\" and \"%s\"", vg_to->max_pv, vg_to->name,
			  vg_from->name);
		return 0;
	}

	if (vg_to->max_lv &&
	    (vg_to->max_lv < vg_visible_lvs(vg_to) + vg_visible_lvs(vg_from))) {
		log_error("Maximum number of logical volumes (%d) exceeded "
			  " for \"%s\" and \"%s\"", vg_to->max_lv, vg_to->name,
			  vg_from->name);
		return 0;
	}

	/* Metadata types must be the same */
	if (vg_to->fid->fmt != vg_from->fid->fmt) {
		log_error("Metadata types differ for \"%s\" and \"%s\"",
			  vg_to->name, vg_from->name);
		return 0;
	}

	/* Check no conflicts with LV names */
	dm_list_iterate_items(lvl1, &vg_to->lvs) {
		name1 = lvl1->lv->name;

		dm_list_iterate_items(lvl2, &vg_from->lvs) {
			name2 = lvl2->lv->name;

			if (!strcmp(name1, name2)) {
				log_error("Duplicate logical volume "
					  "name \"%s\" "
					  "in \"%s\" and \"%s\"",
					  name1, vg_to->name, vg_from->name);
				return 0;
			}
		}
	}

	/* Check no PVs are constructed from either VG */
	dm_list_iterate_items(pvl, &vg_to->pvs) {
		if (pv_uses_vg(pvl->pv, vg_from)) {
			log_error("Physical volume %s might be constructed "
				  "from same volume group %s.",
				  pv_dev_name(pvl->pv), vg_from->name);
			return 0;
		}
	}

	dm_list_iterate_items(pvl, &vg_from->pvs) {
		if (pv_uses_vg(pvl->pv, vg_to)) {
			log_error("Physical volume %s might be constructed "
				  "from same volume group %s.",
				  pv_dev_name(pvl->pv), vg_to->name);
			return 0;
		}
	}

	return 1;
}

struct _lv_postorder_baton {
	int (*fn)(struct logical_volume *lv, void *data);
	void *data;
};

static int _lv_postorder_visit(struct logical_volume *lv,
			       int (*fn)(struct logical_volume *lv, void *data),
			       void *data);

static int _lv_each_dependency(struct logical_volume *lv,
			       int (*fn)(struct logical_volume *lv, void *data),
			       void *data)
{
	unsigned i, s;
	struct lv_segment *lvseg;
	struct dm_list *snh;

	struct logical_volume *deps[] = {
		lv->snapshot ? lv->snapshot->origin : 0,
		lv->snapshot ? lv->snapshot->cow : 0 };
	for (i = 0; i < DM_ARRAY_SIZE(deps); ++i) {
		if (deps[i] && !fn(deps[i], data))
			return_0;
	}

	dm_list_iterate_items(lvseg, &lv->segments) {
		if (lvseg->external_lv && !fn(lvseg->external_lv, data))
			return_0;
		if (lvseg->log_lv && !fn(lvseg->log_lv, data))
			return_0;
		if (lvseg->pool_lv && !fn(lvseg->pool_lv, data))
			return_0;
		if (lvseg->metadata_lv && !fn(lvseg->metadata_lv, data))
			return_0;
		for (s = 0; s < lvseg->area_count; ++s) {
			if (seg_type(lvseg, s) == AREA_LV && !fn(seg_lv(lvseg,s), data))
				return_0;
		}
	}

	if (lv_is_origin(lv))
		dm_list_iterate(snh, &lv->snapshot_segs)
			if (!fn(dm_list_struct_base(snh, struct lv_segment, origin_list)->cow, data))
				return_0;

	return 1;
}

static int _lv_postorder_cleanup(struct logical_volume *lv, void *data)
{
	if (!(lv->status & POSTORDER_FLAG))
		return 1;
	lv->status &= ~POSTORDER_FLAG;

	if (!_lv_each_dependency(lv, _lv_postorder_cleanup, data))
		return_0;
	return 1;
}

static int _lv_postorder_level(struct logical_volume *lv, void *data)
{
	struct _lv_postorder_baton *baton = data;
	return (data) ? _lv_postorder_visit(lv, baton->fn, baton->data) : 0;
};

static int _lv_postorder_visit(struct logical_volume *lv,
			       int (*fn)(struct logical_volume *lv, void *data),
			       void *data)
{
	struct _lv_postorder_baton baton;
	int r;

	if (lv->status & POSTORDER_FLAG)
		return 1;
	if (lv->status & POSTORDER_OPEN_FLAG)
		return 1; // a data structure loop has closed...
	lv->status |= POSTORDER_OPEN_FLAG;

	baton.fn = fn;
	baton.data = data;
	r = _lv_each_dependency(lv, _lv_postorder_level, &baton);

	if (r)
		r = fn(lv, data);

	lv->status &= ~POSTORDER_OPEN_FLAG;
	lv->status |= POSTORDER_FLAG;

	return r;
}

/*
 * This will walk the LV dependency graph in depth-first order and in the
 * postorder, call a callback function "fn". The void *data is passed along all
 * the calls. The callback may return zero to indicate an error and terminate
 * the depth-first walk. The error is propagated to return value of
 * _lv_postorder.
 */
static int _lv_postorder(struct logical_volume *lv,
			       int (*fn)(struct logical_volume *lv, void *data),
			       void *data)
{
	int r;
	int pool_locked = dm_pool_locked(lv->vg->vgmem);

	if (pool_locked && !dm_pool_unlock(lv->vg->vgmem, 0))
		return_0;

	r = _lv_postorder_visit(lv, fn, data);
	_lv_postorder_cleanup(lv, 0);

	if (pool_locked && !dm_pool_lock(lv->vg->vgmem, 0))
		return_0;

	return r;
}

/*
 * Calls _lv_postorder() on each LV from VG. Avoids duplicate transitivity visits.
 * Clears with _lv_postorder_cleanup() when all LVs were visited by postorder.
 */
static int _lv_postorder_vg(struct volume_group *vg,
			    int (*fn)(struct logical_volume *lv, void *data),
			    void *data)
{
	struct lv_list *lvl;
	int r = 1;
	int pool_locked = dm_pool_locked(vg->vgmem);

	if (pool_locked && !dm_pool_unlock(vg->vgmem, 0))
		return_0;

	dm_list_iterate_items(lvl, &vg->lvs)
		if (!_lv_postorder_visit(lvl->lv, fn, data)) {
			stack;
			r = 0;
		}

	dm_list_iterate_items(lvl, &vg->lvs)
		_lv_postorder_cleanup(lvl->lv, 0);

	if (pool_locked && !dm_pool_lock(vg->vgmem, 0))
		return_0;

	return r;
}

struct _lv_mark_if_partial_baton {
	int partial;
};

static int _lv_mark_if_partial_collect(struct logical_volume *lv, void *data)
{
	struct _lv_mark_if_partial_baton *baton = data;

	if (baton && lv_is_partial(lv))
		baton->partial = 1;

	return 1;
}

static int _lv_mark_if_partial_single(struct logical_volume *lv, void *data)
{
	unsigned s;
	struct _lv_mark_if_partial_baton baton = { .partial = 0 };
	struct lv_segment *lvseg;

	dm_list_iterate_items(lvseg, &lv->segments) {
		for (s = 0; s < lvseg->area_count; ++s) {
			if (seg_type(lvseg, s) == AREA_PV) {
				if (is_missing_pv(seg_pv(lvseg, s)))
					lv->status |= PARTIAL_LV;
			}
		}
	}

	if (!_lv_each_dependency(lv, _lv_mark_if_partial_collect, &baton))
		return_0;

	if (baton.partial)
		lv->status |= PARTIAL_LV;

	return 1;
}

/*
 * Mark LVs with missing PVs using PARTIAL_LV status flag. The flag is
 * propagated transitively, so LVs referencing other LVs are marked
 * partial as well, if any of their referenced LVs are marked partial.
 */
int vg_mark_partial_lvs(struct volume_group *vg, int clear)
{
	struct lv_list *lvl;

	if (clear)
		dm_list_iterate_items(lvl, &vg->lvs)
			lvl->lv->status &= ~PARTIAL_LV;

	if (!_lv_postorder_vg(vg, _lv_mark_if_partial_single, NULL))
		return_0;
	return 1;
}

/*
 * Be sure that all PV devices have cached read ahead in dev-cache
 * Currently it takes read_ahead from first PV segment only
 */
static int _lv_read_ahead_single(struct logical_volume *lv, void *data)
{
	struct lv_segment *seg = first_seg(lv);
	uint32_t seg_read_ahead = 0, *read_ahead = data;

	if (!read_ahead) {
		log_error(INTERNAL_ERROR "Read ahead data missing.");
		return 0;
	}

	if (seg && seg->area_count && seg_type(seg, 0) == AREA_PV)
		dev_get_read_ahead(seg_pv(seg, 0)->dev, &seg_read_ahead);

	if (seg_read_ahead > *read_ahead)
		*read_ahead = seg_read_ahead;

	return 1;
}

/*
 * Calculate readahead for logical volume from underlying PV devices.
 * If read_ahead is NULL, only ensure that readahead of PVs are preloaded
 * into PV struct device in dev cache.
 */
void lv_calculate_readahead(const struct logical_volume *lv, uint32_t *read_ahead)
{
	uint32_t _read_ahead = 0;

	if (lv->read_ahead == DM_READ_AHEAD_AUTO)
		_lv_postorder((struct logical_volume *)lv, _lv_read_ahead_single, &_read_ahead);

	if (read_ahead) {
		log_debug_metadata("Calculated readahead of LV %s is %u", lv->name, _read_ahead);
		*read_ahead = _read_ahead;
	}
}

struct validate_hash {
	struct dm_hash_table *lvname;
	struct dm_hash_table *historical_lvname;
	struct dm_hash_table *lvid;
	struct dm_hash_table *historical_lvid;
	struct dm_hash_table *pvid;
	struct dm_hash_table *lv_lock_args;
};

/*
 * Check that an LV and all its PV references are correctly listed in vg->lvs
 * and vg->pvs, respectively. This only looks at a single LV, but *not* at the
 * LVs it is using. To do the latter, you should use _lv_postorder with this
 * function. C.f. vg_validate.
 */
static int _lv_validate_references_single(struct logical_volume *lv, void *data)
{
	struct volume_group *vg = lv->vg;
	struct validate_hash *vhash = data;
	struct lv_segment *lvseg;
	struct physical_volume *pv;
	unsigned s;
	int r = 1;

	if (lv != dm_hash_lookup_binary(vhash->lvid, &lv->lvid.id[1],
					sizeof(lv->lvid.id[1]))) {
		log_error(INTERNAL_ERROR
			  "Referenced LV %s not listed in VG %s.",
			  lv->name, vg->name);
		r = 0;
	}

	dm_list_iterate_items(lvseg, &lv->segments) {
		for (s = 0; s < lvseg->area_count; ++s) {
			if (seg_type(lvseg, s) != AREA_PV)
				continue;
			pv = seg_pv(lvseg, s);
			/* look up the reference in vg->pvs */
			if (pv != dm_hash_lookup_binary(vhash->pvid, &pv->id,
							sizeof(pv->id))) {
				log_error(INTERNAL_ERROR
					  "Referenced PV %s not listed in VG %s.",
					  pv_dev_name(pv), vg->name);
				r = 0;
			}
		}
	}

	return r;
}

/*
 * Format is <version>:<info>
 */
static int _validate_lock_args_chars(const char *lock_args)
{
	unsigned i;
	char c;
	int found_colon = 0;
	int r = 1;

	for (i = 0; i < strlen(lock_args); i++) {
		c = lock_args[i];

		if (!isalnum(c) && c != '.' && c != '_' && c != '-' && c != '+' && c != ':') {
			log_error(INTERNAL_ERROR "Invalid character at index %u of lock_args \"%s\"",
				  i, lock_args);
			r = 0;
		}

		if (c == ':' && found_colon) {
			log_error(INTERNAL_ERROR "Invalid colon at index %u of lock_args \"%s\"",
				  i, lock_args);
			r = 0;
		}

		if (c == ':')
			found_colon = 1;
	}

	return r;
}

static int _validate_vg_lock_args(struct volume_group *vg)
{
	if (!_validate_lock_args_chars(vg->lock_args)) {
		log_error(INTERNAL_ERROR "VG %s has invalid lock_args chars", vg->name);
		return 0;
	}

	return 1;
}

/*
 * For lock_type sanlock, LV lock_args are <version>:<info>
 * For lock_type dlm, LV lock_args are not used, and lock_args is
 * just set to "dlm".
 */
static int _validate_lv_lock_args(struct logical_volume *lv)
{
	int r = 1;

	if (!strcmp(lv->vg->lock_type, "sanlock")) {
		if (!_validate_lock_args_chars(lv->lock_args)) {
			log_error(INTERNAL_ERROR "LV %s/%s has invalid lock_args chars",
				  lv->vg->name, display_lvname(lv));
			return 0;
		}

	} else if (!strcmp(lv->vg->lock_type, "dlm")) {
		if (strcmp(lv->lock_args, "dlm")) {
			log_error(INTERNAL_ERROR "LV %s/%s has invalid lock_args \"%s\"",
				   lv->vg->name, display_lvname(lv), lv->lock_args);
			r = 0;
		}
	}

	return r;
}

int vg_validate(struct volume_group *vg)
{
	struct pv_list *pvl;
	struct lv_list *lvl;
	struct glv_list *glvl;
	struct historical_logical_volume *hlv;
	struct lv_segment *seg;
	struct dm_str_list *sl;
	char uuid[64] __attribute__((aligned(8)));
	char uuid2[64] __attribute__((aligned(8)));
	int r = 1;
	unsigned hidden_lv_count = 0, lv_count = 0, lv_visible_count = 0;
	unsigned pv_count = 0;
	unsigned num_snapshots = 0;
	unsigned spare_count = 0;
	size_t vg_name_len = strlen(vg->name);
	size_t dev_name_len;
	struct validate_hash vhash = { NULL };

	if (vg->alloc == ALLOC_CLING_BY_TAGS) {
		log_error(INTERNAL_ERROR "VG %s allocation policy set to invalid cling_by_tags.",
			  vg->name);
		r = 0;
	}

	if (vg->status & LVM_WRITE_LOCKED) {
		log_error(INTERNAL_ERROR "VG %s has external flag LVM_WRITE_LOCKED set internally.",
			  vg->name);
		r = 0;
	}

	/* FIXME Also check there's no data/metadata overlap */
	if (!(vhash.pvid = dm_hash_create(vg->pv_count))) {
		log_error("Failed to allocate pvid hash.");
		return 0;
	}

	dm_list_iterate_items(sl, &vg->tags)
		if (!validate_tag(sl->str)) {
			log_error(INTERNAL_ERROR "VG %s tag %s has invalid form.",
				  vg->name, sl->str);
			r = 0;
		}

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (++pv_count > vg->pv_count) {
			log_error(INTERNAL_ERROR "PV list corruption detected in VG %s.", vg->name);
			/* FIXME Dump list structure? */
			r = 0;
		}

		if (pvl->pv->vg != vg) {
			log_error(INTERNAL_ERROR "VG %s PV list entry points "
				  "to different VG %s.", vg->name,
				  pvl->pv->vg ? pvl->pv->vg->name : "NULL");
			r = 0;
		}

		if (strcmp(pvl->pv->vg_name, vg->name)) {
			log_error(INTERNAL_ERROR "VG name for PV %s is corrupted.",
				  pv_dev_name(pvl->pv));
			r = 0;
		}

		if (dm_hash_lookup_binary(vhash.pvid, &pvl->pv->id,
					  sizeof(pvl->pv->id))) {
			if (!id_write_format(&pvl->pv->id, uuid,
					     sizeof(uuid)))
				stack;
			log_error(INTERNAL_ERROR "Duplicate PV id "
				  "%s detected for %s in %s.",
				  uuid, pv_dev_name(pvl->pv),
				  vg->name);
			r = 0;
		}

		dm_list_iterate_items(sl, &pvl->pv->tags)
			if (!validate_tag(sl->str)) {
				log_error(INTERNAL_ERROR "PV %s tag %s has invalid form.",
					  pv_dev_name(pvl->pv), sl->str);
				r = 0;
			}

		if (!dm_hash_insert_binary(vhash.pvid, &pvl->pv->id,
					   sizeof(pvl->pv->id), pvl->pv)) {
			log_error("Failed to hash pvid.");
			r = 0;
			break;
		}
	}


	if (!check_pv_segments(vg)) {
		log_error(INTERNAL_ERROR "PV segments corrupted in %s.",
			  vg->name);
		r = 0;
	}

	dm_list_iterate_items(lvl, &vg->removed_lvs) {
		if (!(lvl->lv->status & LV_REMOVED)) {
			log_error(INTERNAL_ERROR "LV %s is not marked as removed while it's part "
				  "of removed LV list for VG %s", lvl->lv->name, vg->name);
			r = 0;
		}
	}

	/*
	 * Count all non-snapshot invisible LVs
	 */
	dm_list_iterate_items(lvl, &vg->lvs) {
		lv_count++;

		if (lvl->lv->status & LV_REMOVED) {
			log_error(INTERNAL_ERROR "LV %s is marked as removed while it's "
				  "still part of the VG %s", lvl->lv->name, vg->name);
			r = 0;
		}

		if (lvl->lv->status & LVM_WRITE_LOCKED) {
			log_error(INTERNAL_ERROR "LV %s has external flag LVM_WRITE_LOCKED set internally.",
				  lvl->lv->name);
			r = 0;
		}

		dev_name_len = strlen(lvl->lv->name) + vg_name_len + 3;
		if (dev_name_len >= NAME_LEN) {
			log_error(INTERNAL_ERROR "LV name \"%s/%s\" length %"
				  PRIsize_t " is not supported.",
				  vg->name, lvl->lv->name, dev_name_len);
			r = 0;
		}

		if (!id_equal(&lvl->lv->lvid.id[0], &lvl->lv->vg->id)) {
			if (!id_write_format(&lvl->lv->lvid.id[0], uuid,
					     sizeof(uuid)))
				stack;
			if (!id_write_format(&lvl->lv->vg->id, uuid2,
					     sizeof(uuid2)))
				stack;
			log_error(INTERNAL_ERROR "LV %s has VG UUID %s but its VG %s has UUID %s",
				  lvl->lv->name, uuid, lvl->lv->vg->name, uuid2);
			r = 0;
		}

		if (lv_is_pool_metadata_spare(lvl->lv)) {
			if (++spare_count > 1) {
				log_error(INTERNAL_ERROR "LV %s is extra pool metadata spare volume. %u found but only 1 allowed.",
					  lvl->lv->name, spare_count);
				r = 0;
			}
			if (vg->pool_metadata_spare_lv != lvl->lv) {
				log_error(INTERNAL_ERROR "LV %s is not the VG's pool metadata spare volume.",
					  lvl->lv->name);
				r = 0;
			}
		}

		if (lv_is_cow(lvl->lv))
			num_snapshots++;

		if (lv_is_visible(lvl->lv))
			lv_visible_count++;

		if (!check_lv_segments(lvl->lv, 0)) {
			log_error(INTERNAL_ERROR "LV segments corrupted in %s.",
				  lvl->lv->name);
			r = 0;
		}

		if (lvl->lv->alloc == ALLOC_CLING_BY_TAGS) {
			log_error(INTERNAL_ERROR "LV %s allocation policy set to invalid cling_by_tags.",
				  lvl->lv->name);
			r = 0;
		}

		if (!validate_name(lvl->lv->name)) {
			log_error(INTERNAL_ERROR "LV name %s has invalid form.", lvl->lv->name);
			r = 0;
		}

		dm_list_iterate_items(sl, &lvl->lv->tags)
			if (!validate_tag(sl->str)) {
				log_error(INTERNAL_ERROR "LV %s tag %s has invalid form.",
					  lvl->lv->name, sl->str);
				r = 0;
			}

		if (lvl->lv->status & VISIBLE_LV)
			continue;

		/* snapshots */
		if (lv_is_cow(lvl->lv))
			continue;

		/* virtual origins are always hidden */
		if (lv_is_origin(lvl->lv) && !lv_is_virtual_origin(lvl->lv))
			continue;

		/* count other non-snapshot invisible volumes */
		hidden_lv_count++;

		/*
		 *  FIXME: add check for unreferenced invisible LVs
		 *   - snapshot cow & origin
		 *   - mirror log & images
		 *   - mirror conversion volumes (_mimagetmp*)
		 */
	}

	/*
	 * all volumes = visible LVs + snapshot_cows + invisible LVs
	 */
	if (lv_count != lv_visible_count + num_snapshots + hidden_lv_count) {
		log_error(INTERNAL_ERROR "#LVs (%u) != #visible LVs (%u) "
			  "+ #snapshots (%u) + #internal LVs (%u) in VG %s",
			  lv_count, lv_visible_count, num_snapshots,
			  hidden_lv_count, vg->name);
		r = 0;
	}

	/* Avoid endless loop if lv->segments list is corrupt */
	if (!r)
		goto out;

	if (!(vhash.lvname = dm_hash_create(lv_count))) {
		log_error("Failed to allocate lv_name hash");
		r = 0;
		goto out;
	}

	if (!(vhash.lvid = dm_hash_create(lv_count))) {
		log_error("Failed to allocate uuid hash");
		r = 0;
		goto out;
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		if (dm_hash_lookup(vhash.lvname, lvl->lv->name)) {
			log_error(INTERNAL_ERROR
				  "Duplicate LV name %s detected in %s.",
				  lvl->lv->name, vg->name);
			r = 0;
		}

		if (dm_hash_lookup_binary(vhash.lvid, &lvl->lv->lvid.id[1],
					  sizeof(lvl->lv->lvid.id[1]))) {
			if (!id_write_format(&lvl->lv->lvid.id[1], uuid,
					     sizeof(uuid)))
				stack;
			log_error(INTERNAL_ERROR "Duplicate LV id "
				  "%s detected for %s in %s.",
				  uuid, lvl->lv->name, vg->name);
			r = 0;
		}

		if (!check_lv_segments(lvl->lv, 1)) {
			log_error(INTERNAL_ERROR "LV segments corrupted in %s.",
				  lvl->lv->name);
			r = 0;
		}

		if (!dm_hash_insert(vhash.lvname, lvl->lv->name, lvl)) {
			log_error("Failed to hash lvname.");
			r = 0;
			break;
		}

		if (!dm_hash_insert_binary(vhash.lvid, &lvl->lv->lvid.id[1],
					   sizeof(lvl->lv->lvid.id[1]), lvl->lv)) {
			log_error("Failed to hash lvid.");
			r = 0;
			break;
		}
	}

	if (!_lv_postorder_vg(vg, _lv_validate_references_single, &vhash)) {
		stack;
		r = 0;
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		if (!lv_is_pvmove(lvl->lv))
			continue;
		dm_list_iterate_items(seg, &lvl->lv->segments) {
			if (seg_is_mirrored(seg)) {
				if (seg->area_count != 2) {
					log_error(INTERNAL_ERROR
						  "Segment in %s is not 2-way.",
						  lvl->lv->name);
					r = 0;
				}
			} else if (seg->area_count != 1) {
				log_error(INTERNAL_ERROR
					  "Segment in %s has wrong number of areas: %d.",
					  lvl->lv->name, seg->area_count);
				r = 0;
			}
		}
	}

	if (!(vg->fid->fmt->features & FMT_UNLIMITED_VOLS) &&
	    (!vg->max_lv || !vg->max_pv)) {
		log_error(INTERNAL_ERROR "Volume group %s has limited PV/LV count"
			  " but limit is not set.", vg->name);
		r = 0;
	}

	if (vg->pool_metadata_spare_lv &&
	    !lv_is_pool_metadata_spare(vg->pool_metadata_spare_lv)) {
		log_error(INTERNAL_ERROR "VG references non pool metadata spare LV %s.",
			  vg->pool_metadata_spare_lv->name);
		r = 0;
	}

	if (vg_max_lv_reached(vg))
		stack;

	if (!(vhash.lv_lock_args = dm_hash_create(lv_count))) {
		log_error("Failed to allocate lv_lock_args hash");
		r = 0;
		goto out;
	}

	if (vg_is_shared(vg)) {
		if (!vg->lock_args) {
			log_error(INTERNAL_ERROR "VG %s with lock_type %s without lock_args",
				  vg->name, vg->lock_type);
			r = 0;
		}

		if (vg_is_clustered(vg)) {
			log_error(INTERNAL_ERROR "VG %s with lock_type %s is clustered",
				  vg->name, vg->lock_type);
			r = 0;
		}

		if (vg->system_id && vg->system_id[0]) {
			log_error(INTERNAL_ERROR "VG %s with lock_type %s has system_id %s",
				  vg->name, vg->lock_type, vg->system_id);
			r = 0;
		}

		if (strcmp(vg->lock_type, "sanlock") && strcmp(vg->lock_type, "dlm")) {
			log_error(INTERNAL_ERROR "VG %s has unknown lock_type %s",
				  vg->name, vg->lock_type);
			r = 0;
		}

		if (!_validate_vg_lock_args(vg))
			r = 0;
	} else {
		if (vg->lock_args) {
			log_error(INTERNAL_ERROR "VG %s has lock_args %s without lock_type",
				  vg->name, vg->lock_args);
			r = 0;
		}
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		if (vg_is_shared(vg)) {
			if (lockd_lv_uses_lock(lvl->lv)) {
				if (vg->skip_validate_lock_args)
					continue;

				/*
				 * FIXME: make missing lock_args an error.
				 * There are at least two cases where this
				 * check doesn't work correctly:
				 *
				 * 1. When creating a cow snapshot,
				 * (lvcreate -s -L1M -n snap1 vg/lv1),
				 * lockd_lv_uses_lock() uses lv_is_cow()
				 * which depends on lv->snapshot being
				 * set, but it's not set at this point,
				 * so lockd_lv_uses_lock() cannot identify
				 * the LV as a cow_lv, and thinks it needs
				 * a lock when it doesn't.  To fix this we
				 * probably need to validate by finding the
				 * origin LV, then finding all its snapshots
				 * which will have no lock_args.
				 *
				 * 2. When converting an LV to a thin pool
				 * without using an existing metadata LV,
				 * (lvconvert --type thin-pool vg/poolX),
				 * there is an intermediate LV created,
				 * probably for the metadata LV, and
				 * validate is called on the VG in this
				 * intermediate state, which finds the
				 * newly created LV which is not yet
				 * identified as a metadata LV, and
				 * does not have any lock_args.  To fix
				 * this we might be able to find the place
				 * where the intermediate LV is created,
				 * and set new variable on it like for vgs,
				 * lv->skip_validate_lock_args.
				 */
				if (!lvl->lv->lock_args) {
					/*
					log_verbose("LV %s/%s missing lock_args",
						    vg->name, lvl->lv->name);
					r = 0;
					*/
					continue;
				}

				if (!_validate_lv_lock_args(lvl->lv)) {
					r = 0;
					continue;
				}

				if (!strcmp(vg->lock_type, "sanlock")) {
					if (dm_hash_lookup(vhash.lv_lock_args, lvl->lv->lock_args)) {
						log_error(INTERNAL_ERROR "LV %s/%s has duplicate lock_args %s.",
							  vg->name, lvl->lv->name, lvl->lv->lock_args);
						r = 0;
					}

					if (!dm_hash_insert(vhash.lv_lock_args, lvl->lv->lock_args, lvl)) {
						log_error("Failed to hash lvname.");
						r = 0;
					}

				}
			} else {
				if (lvl->lv->lock_args) {
					log_error(INTERNAL_ERROR "LV %s/%s shouldn't have lock_args",
						  vg->name, lvl->lv->name);
					r = 0;
				}
			}
		} else {
			if (lvl->lv->lock_args) {
				log_error(INTERNAL_ERROR "LV %s/%s with no lock_type has lock_args %s",
					  vg->name, lvl->lv->name, lvl->lv->lock_args);
				r = 0;
			}
		}
	}

	if (!(vhash.historical_lvname = dm_hash_create(dm_list_size(&vg->historical_lvs)))) {
		log_error("Failed to allocate historical LV name hash");
		r = 0;
		goto out;
	}

        if (!(vhash.historical_lvid = dm_hash_create(dm_list_size(&vg->historical_lvs)))) {
                log_error("Failed to allocate historical LV uuid hash");
                r = 0;
                goto out;
        }

	dm_list_iterate_items(glvl, &vg->historical_lvs) {
		if (!glvl->glv->is_historical) {
			log_error(INTERNAL_ERROR "LV %s/%s appearing in VG's historical list is not a historical LV",
				  vg->name, glvl->glv->live->name);
			r = 0;
			continue;
		}

		hlv = glvl->glv->historical;

		if (hlv->vg != vg) {
			log_error(INTERNAL_ERROR "Historical LV %s points to different VG %s while it is listed in VG %s",
				  hlv->name, hlv->vg->name, vg->name);
			r = 0;
			continue;
		}

		if (!id_equal(&hlv->lvid.id[0], &hlv->vg->id)) {
			if (!id_write_format(&hlv->lvid.id[0], uuid, sizeof(uuid)))
				stack;
			if (!id_write_format(&hlv->vg->id, uuid2, sizeof(uuid2)))
				stack;
			log_error(INTERNAL_ERROR "Historical LV %s has VG UUID %s but its VG %s has UUID %s",
				  hlv->name, uuid, hlv->vg->name, uuid2);
			r = 0;
			continue;
                }

		if (dm_hash_lookup_binary(vhash.historical_lvid, &hlv->lvid.id[1], sizeof(hlv->lvid.id[1]))) {
			if (!id_write_format(&hlv->lvid.id[1], uuid,sizeof(uuid)))
				stack;
			log_error(INTERNAL_ERROR "Duplicate historical LV id %s detected for %s in %s",
				  uuid, hlv->name, vg->name);
                        r = 0;
                }

		if (dm_hash_lookup(vhash.historical_lvname, hlv->name)) {
			log_error(INTERNAL_ERROR "Duplicate historical LV name %s detected in %s", hlv->name, vg->name);
			r = 0;
			continue;
		}

                if (!dm_hash_insert(vhash.historical_lvname, hlv->name, hlv)) {
                        log_error("Failed to hash historical LV name");
                        r = 0;
                        break;
                }

                if (!dm_hash_insert_binary(vhash.historical_lvid, &hlv->lvid.id[1], sizeof(hlv->lvid.id[1]), hlv)) {
                        log_error("Failed to hash historical LV id");
                        r = 0;
                        break;
                }

		if (dm_hash_lookup(vhash.lvname, hlv->name)) {
			log_error(INTERNAL_ERROR "Name %s appears as live and historical LV at the same time in VG %s",
				  hlv->name, vg->name);
			r = 0;
			continue;
		}

		if (!hlv->indirect_origin && !dm_list_size(&hlv->indirect_glvs)) {
			log_error(INTERNAL_ERROR "Historical LV %s is not part of any LV chain in VG %s", hlv->name, vg->name);
			r = 0;
			continue;
		}
	}

out:
	if (vhash.lvid)
		dm_hash_destroy(vhash.lvid);
	if (vhash.lvname)
		dm_hash_destroy(vhash.lvname);
	if (vhash.historical_lvid)
		dm_hash_destroy(vhash.historical_lvid);
	if (vhash.historical_lvname)
		dm_hash_destroy(vhash.historical_lvname);
	if (vhash.pvid)
		dm_hash_destroy(vhash.pvid);
	if (vhash.lv_lock_args)
		dm_hash_destroy(vhash.lv_lock_args);

	return r;
}

static int _pv_in_pv_list(struct physical_volume *pv, struct dm_list *head)
{
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, head) {
		if (pvl->pv == pv)
			return 1;
	}

	return 0;
}

/*
 * Check if any of the PVs in VG still contain old PV headers
 * and if yes, schedule them for PV header update.
 */
static int _vg_update_old_pv_ext_if_needed(struct volume_group *vg)
{
	struct pv_list *pvl, *new_pvl;
	int pv_needs_rewrite;

	if (!(vg->fid->fmt->features & FMT_PV_FLAGS))
		return 1;

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (is_missing_pv(pvl->pv) ||
		    !pvl->pv->fmt->ops->pv_needs_rewrite)
			continue;

		if (_pv_in_pv_list(pvl->pv, &vg->pv_write_list))
			continue;

		if (!pvl->pv->fmt->ops->pv_needs_rewrite(pvl->pv->fmt, pvl->pv,
							 &pv_needs_rewrite))
			return_0;

		if (pv_needs_rewrite) {
			/*
			 * Schedule PV for writing only once!
			 */
			if (_pv_in_pv_list(pvl->pv, &vg->pv_write_list))
				continue;

			if (!(new_pvl = dm_pool_zalloc(vg->vgmem, sizeof(*new_pvl)))) {
				log_error("pv_to_write allocation for '%s' failed", pv_dev_name(pvl->pv));
				return 0;
			}
			new_pvl->pv = pvl->pv;
			dm_list_add(&vg->pv_write_list, &new_pvl->list);
			log_debug("PV %s has old extension header, updating to newest version.",
				  pv_dev_name(pvl->pv));
		}
	}

	if (!dm_list_empty(&vg->pv_write_list) &&
	    (!vg_write(vg) || !vg_commit(vg))) {
		log_error("Failed to update old PV extension headers in VG %s.", vg->name);
		return 0;
	}

	return 1;
}

static int _check_historical_lv_is_valid(struct historical_logical_volume *hlv)
{
	struct glv_list *glvl;

	if (hlv->checked)
		return hlv->valid;

	/*
	 * Historical LV is valid if there is
	 * at least one live LV among ancestors.
	 */
	hlv->valid = 0;
	dm_list_iterate_items(glvl, &hlv->indirect_glvs) {
		if (!glvl->glv->is_historical ||
		    _check_historical_lv_is_valid(glvl->glv->historical)) {
			hlv->valid = 1;
			break;
		}
	}

	hlv->checked = 1;
	return hlv->valid;
}

static int _handle_historical_lvs(struct volume_group *vg)
{
	struct glv_list *glvl, *tglvl;
	time_t current_timestamp = 0;
	struct historical_logical_volume *hlv;
	int valid = 1;

	dm_list_iterate_items(glvl, &vg->historical_lvs)
		glvl->glv->historical->checked = 0;

	dm_list_iterate_items(glvl, &vg->historical_lvs) {
		hlv = glvl->glv->historical;

		valid &= _check_historical_lv_is_valid(hlv);

		if (!hlv->timestamp_removed) {
			if (!current_timestamp)
				current_timestamp = time(NULL);
			hlv->timestamp_removed = (uint64_t) current_timestamp;
		}
	}

	if (valid)
		return 1;

	dm_list_iterate_items_safe(glvl, tglvl, &vg->historical_lvs) {
		hlv = glvl->glv->historical;
		if (hlv->checked && hlv->valid)
			continue;

		log_print_unless_silent("Automatically removing historical "
					"logical volume %s/%s%s.",
					 vg->name, HISTORICAL_LV_PREFIX, hlv->name);
		if (!historical_glv_remove(glvl->glv))
			return_0;
	}

	return 1;
}

/*
 * After vg_write() returns success,
 * caller MUST call either vg_commit() or vg_revert()
 */
int vg_write(struct volume_group *vg)
{
	struct dm_list *mdah;
	struct pv_list *pvl, *pvl_safe;
	struct metadata_area *mda;
	struct lv_list *lvl;
	int revert = 0, wrote = 0;

	if (vg_is_shared(vg)) {
		dm_list_iterate_items(lvl, &vg->lvs) {
			if (lvl->lv->lock_args && !strcmp(lvl->lv->lock_args, "pending")) {
				if (!lockd_init_lv_args(vg->cmd, vg, lvl->lv, vg->lock_type, &lvl->lv->lock_args)) {
					log_error("Cannot allocate lock for new LV.");
					return 0;
				}
				lvl->lv->new_lock_args = 1;
			}
		}
	}

	if (!_handle_historical_lvs(vg)) {
		log_error("Failed to handle historical LVs in VG %s.", vg->name);
		return 0;
	}

	if (!vg_validate(vg))
		return_0;

	if (vg->status & PARTIAL_VG) {
		log_error("Cannot update partial volume group %s.", vg->name);
		return 0;
	}

	if (vg_missing_pv_count(vg) && !vg->cmd->handles_missing_pvs) {
		log_error("Cannot update volume group %s while physical "
			  "volumes are missing.", vg->name);
		return 0;
	}

	if (lvmcache_found_duplicate_pvs() && vg_has_duplicate_pvs(vg) &&
	    !find_config_tree_bool(vg->cmd, devices_allow_changes_with_duplicate_pvs_CFG, NULL)) {
		log_error("Cannot update volume group %s with duplicate PV devices.",
			  vg->name);
		return 0;
	}

	if (vg_has_unknown_segments(vg) && !vg->cmd->handles_unknown_segments) {
		log_error("Cannot update volume group %s with unknown segments in it!",
			  vg->name);
		return 0;
	}

	if (!_vg_adjust_ignored_mdas(vg))
		return_0;

	if (!vg_mda_used_count(vg)) {
		log_error("Aborting vg_write: No metadata areas to write to!");
		return 0;
	}

	if (critical_section())
		log_error(INTERNAL_ERROR
			  "Writing metadata in critical section.");

	/* Unlock memory if possible */
	memlock_unlock(vg->cmd);
	vg->seqno++;

	dm_list_iterate_items_safe(pvl, pvl_safe, &vg->pv_write_list) {
		if (!pv_write(vg->cmd, pvl->pv, 1))
			return_0;
		dm_list_del(&pvl->list);
	}

	/* Write to each copy of the metadata area */
	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use) {
		if (mda->status & MDA_FAILED)
			continue;
		if (!mda->ops->vg_write) {
			log_error("Format does not support writing volume"
				  "group metadata areas");
			revert = 1;
			break;
		}
		if (!mda->ops->vg_write(vg->fid, vg, mda)) {
			if (vg->cmd->handles_missing_pvs) {
				log_warn("WARNING: Failed to write an MDA of VG %s.", vg->name);
				mda->status |= MDA_FAILED;
			} else {
				stack;
				revert = 1;
				break;
			}
		} else
			++ wrote;
	}

	if (revert || !wrote) {
		log_error("Failed to write VG %s.", vg->name);
		dm_list_uniterate(mdah, &vg->fid->metadata_areas_in_use, &mda->list) {
			mda = dm_list_item(mdah, struct metadata_area);

			if (mda->status & MDA_FAILED)
				continue;

			if (mda->ops->vg_revert &&
			    !mda->ops->vg_revert(vg->fid, vg, mda)) {
				stack;
			}
		}
		return 0;
	}

	/* Now pre-commit each copy of the new metadata */
	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use) {
		if (mda->status & MDA_FAILED)
			continue;
		if (mda->ops->vg_precommit &&
		    !mda->ops->vg_precommit(vg->fid, vg, mda)) {
			stack;
			/* Revert */
			dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use) {
				if (mda->status & MDA_FAILED)
					continue;
				if (mda->ops->vg_revert &&
				    !mda->ops->vg_revert(vg->fid, vg, mda)) {
					stack;
				}
			}
			return 0;
		}
	}

	if (!_vg_update_embedded_copy(vg, &vg->vg_precommitted)) /* prepare precommited */
		return_0;

	lockd_vg_update(vg);

	return 1;
}

static int _vg_commit_mdas(struct volume_group *vg)
{
	struct metadata_area *mda, *tmda;
	struct dm_list ignored;
	int failed = 0;
	int cache_updated = 0;

	/* Rearrange the metadata_areas_in_use so ignored mdas come first. */
	dm_list_init(&ignored);
	dm_list_iterate_items_safe(mda, tmda, &vg->fid->metadata_areas_in_use)
		if (mda_is_ignored(mda))
			dm_list_move(&ignored, &mda->list);

	dm_list_iterate_items_safe(mda, tmda, &ignored)
		dm_list_move(&vg->fid->metadata_areas_in_use, &mda->list);

	/* Commit to each copy of the metadata area */
	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use) {
		if (mda->status & MDA_FAILED)
			continue;
		failed = 0;
		if (mda->ops->vg_commit &&
		    !mda->ops->vg_commit(vg->fid, vg, mda)) {
			stack;
			failed = 1;
		}
		/* Update cache first time we succeed */
		if (!failed && !cache_updated) {
			lvmcache_update_vg(vg, 0);
			cache_updated = 1;
		}
	}
	return cache_updated;
}

/* Commit pending changes */
int vg_commit(struct volume_group *vg)
{
	int cache_updated = 0;
	struct pv_list *pvl;

	cache_updated = _vg_commit_mdas(vg);

	set_vg_notify(vg->cmd);

	if (cache_updated) {
		/*
		 * We need to clear old_name after a successful commit.
		 * The volume_group structure could be reused later.
		 */
		vg->old_name = NULL;
	        dm_list_iterate_items(pvl, &vg->pvs)
			pvl->pv->status &= ~PV_MOVED_VG;

		/* This *is* the original now that it's commited. */
		_vg_move_cached_precommitted_to_committed(vg);
	}

	/* If at least one mda commit succeeded, it was committed */
	return cache_updated;
}

/* Don't commit any pending changes */
void vg_revert(struct volume_group *vg)
{
	struct metadata_area *mda;
	struct lv_list *lvl;

	dm_list_iterate_items(lvl, &vg->lvs) {
		if (lvl->lv->new_lock_args) {
			lockd_free_lv(vg->cmd, vg, lvl->lv->name, &lvl->lv->lvid.id[1], lvl->lv->lock_args);
			lvl->lv->new_lock_args = 0;
		}
	}

	_vg_wipe_cached_precommitted(vg); /* VG is no longer needed */

	dm_list_iterate_items(mda, &vg->fid->metadata_areas_in_use) {
		if (mda->ops->vg_revert &&
		    !mda->ops->vg_revert(vg->fid, vg, mda)) {
			stack;
		}
	}
}

static int _check_mda_in_use(struct metadata_area *mda, void *_in_use)
{
	int *in_use = _in_use;
	if (!mda_is_ignored(mda))
		*in_use = 1;
	return 1;
}

struct _vg_read_orphan_baton {
	struct cmd_context *cmd;
	struct volume_group *vg;
	const struct format_type *fmt;
};

/*
 * If we know that the PV is orphan, meaning there's at least one MDA on
 * that PV which does not reference any VG and at the same time there's
 * PV_EXT_USED flag set, we're certainly in an inconsistent state and we
 * need to fix this.
 *
 * For example, such situation can happen during vgremove/vgreduce if we
 * removed/reduced the VG, but we haven't written PV headers yet because
 * vgremove stopped abruptly for whatever reason just before writing new
 * PV headers with updated state, including PV extension flags (and so the
 * PV_EXT_USED flag).
 *
 * However, in case the PV has no MDAs at all, we can't double-check
 * whether the PV_EXT_USED is correct or not - if that PV is marked
 * as used, it's either:
 *  - really used (but other disks with MDAs are missing)
 *  - or the error state as described above is hit
 *
 * User needs to overwrite the PV header directly if it's really clear
 * the PV having no MDAs does not belong to any VG and at the same time
 * it's still marked as being in use (pvcreate -ff <dev_name> will fix this).
 *
 * Note that the above doesn't account for the case where the PV has
 * VG metadata that fails to be parsed.  In that case, the PV looks
 * like an in-use orphan, and is auto-repaired here.  A PV with
 * unparsable metadata should be kept on a special list of devices
 * (like duplicate PVs) that are not auto-repaired, cannot be used
 * by pvcreate, and are displayed with a special flag by 'pvs'.
 */

#if 0
static int _check_or_repair_orphan_pv_ext(struct physical_volume *pv,
					  struct lvmcache_info *info,
					  struct _vg_read_orphan_baton *b)
{
	uint32_t ext_version = lvmcache_ext_version(info);
	uint32_t ext_flags = lvmcache_ext_flags(info);
	int at_least_one_mda_used;

	/*
	 * Nothing to do if PV header extension < 2:
	 *  - version 0 is PV header without any extensions,
	 *  - version 1 has bootloader area support only and
	 *    we're not checking anything for that one here.
	 */
	if (ext_version < 2) {
		b->consistent = 1;
		return 1;
	}

	if (ext_flags & PV_EXT_USED) {
		if (lvmcache_mda_count(info)) {
			at_least_one_mda_used = 0;
			lvmcache_foreach_mda(info, _check_mda_in_use, &at_least_one_mda_used);

			/*
			 * We've found a PV that is marked as used with PV_EXT_USED flag
			 * and it's orphan at the same time while it contains MDAs.
			 * This is incorrect state and it needs to be fixed.
			 * The PV_EXT_USED flag needs to be dropped!
			 */
			if (b->repair) {
				if (at_least_one_mda_used) {
					log_warn("WARNING: Repairing flag incorrectly marking "
						 "Physical Volume %s as used.", pv_dev_name(pv));

					/* pv_write will set correct ext_flags */
					if (!pv_write(b->cmd, pv, 0)) {
						b->consistent = 0;
						log_error("Failed to repair physical volume \"%s\".",
							  pv_dev_name(pv));
						return 0;
					}
				}
				b->consistent = 1;
			} else if (at_least_one_mda_used) {
				/* mark as inconsistent only if there's at least 1 MDA used */
				b->consistent = 0;
			}
		}
	}

	return 1;
}
#endif

static int _vg_read_orphan_pv(struct lvmcache_info *info, void *baton)
{
	struct _vg_read_orphan_baton *b = baton;
	struct physical_volume *pv = NULL;
	struct pv_list *pvl;
	uint32_t ext_version;
	uint32_t ext_flags;

	if (!(pv = _pv_read(b->cmd, b->fmt, b->vg, info))) {
		stack;
		return 1;
	}

	if (!(pvl = dm_pool_zalloc(b->vg->vgmem, sizeof(*pvl)))) {
		log_error("pv_list allocation failed");
		free_pv_fid(pv);
		return 0;
	}
	pvl->pv = pv;
	add_pvl_to_vgs(b->vg, pvl);

	/*
	 * FIXME: this bit of code that does the auto repair is disabled
	 * until we can distinguish cases where the repair should not
	 * happen, i.e. the VG metadata could not be read/parsed.
	 *
	 * A PV holding VG metadata that lvm can't understand
	 * (e.g. damaged, checksum error, unrecognized flag)
	 * will appear as an in-use orphan, and would be cleared
	 * by this repair code.  Disable this repair until the
	 * code can keep track of these problematic PVs, and
	 * distinguish them from actual in-use orphans.
	 */

	/*
	if (!_check_or_repair_orphan_pv_ext(pv, info, baton)) {
		stack;
		return 0;
	}
	*/

	/*
	 * Nothing to do if PV header extension < 2:
	 *  - version 0 is PV header without any extensions,
	 *  - version 1 has bootloader area support only and
	 *    we're not checking anything for that one here.
	 */
	ext_version = lvmcache_ext_version(info);
	ext_flags = lvmcache_ext_flags(info);

	/*
	 * Warn about a PV that has the in-use flag set, but appears in
	 * the orphan VG (no VG was found referencing it.)
	 * There are a number of conditions that could lead to this:
	 *
	 * . The PV was created with no mdas and is used in a VG with
	 * other PVs (with metadata) that have not yet appeared on
	 * the system.  So, no VG metadata is found by lvm which
	 * references the in-use PV with no mdas.
	 *
	 * . vgremove could have failed after clearing mdas but
	 * before clearing the in-use flag.  In this case, the
	 * in-use flag needs to be manually cleared on the PV.
	 *
	 * . The PV may have damanged/unrecognized VG metadata
	 * that lvm could not read.
	 *
	 * . The PV may have no mdas, and the PVs with the metadata
	 * may have damaged/unrecognized metadata.
	 */
	if ((ext_version >= 2) && (ext_flags & PV_EXT_USED)) {
		log_warn("WARNING: PV %s is marked in use but no VG was found using it.", pv_dev_name(pv));
		log_warn("WARNING: PV %s might need repairing.", pv_dev_name(pv));
	}

	return 1;
}

/* Make orphan PVs look like a VG. */
struct volume_group *vg_read_orphans(struct cmd_context *cmd,
					     uint32_t warn_flags,
					     const char *orphan_vgname)
{
	const struct format_type *fmt;
	struct lvmcache_vginfo *vginfo;
	struct volume_group *vg = NULL;
	struct _vg_read_orphan_baton baton;
	struct pv_list *pvl, *tpvl;
	struct pv_list head;

	dm_list_init(&head.list);

	if (!(vginfo = lvmcache_vginfo_from_vgname(orphan_vgname, NULL)))
		return_NULL;

	if (!(fmt = lvmcache_fmt_from_vgname(cmd, orphan_vgname, NULL, 0)))
		return_NULL;

	vg = fmt->orphan_vg;

	dm_list_iterate_items_safe(pvl, tpvl, &vg->pvs)
		if (pvl->pv->status & UNLABELLED_PV )
			dm_list_move(&head.list, &pvl->list);
		else
			pv_set_fid(pvl->pv, NULL);

	dm_list_init(&vg->pvs);
	vg->pv_count = 0;
	vg->extent_count = 0;
	vg->free_count = 0;

	baton.cmd = cmd;
	baton.fmt = fmt;
	baton.vg = vg;

	/*
	 * vg_read for a normal VG will rescan labels for all the devices
	 * in the VG, in case something changed on disk between the initial
	 * label scan and acquiring the VG lock.  We don't rescan labels
	 * here because this is only called in two ways:
	 *
	 * 1. for reporting, in which case it doesn't matter if something
	 *    changed between the label scan and printing the PVs here
	 *
	 * 2. pvcreate_each_device() for pvcreate//vgcreate/vgextend,
	 *    which already does the label rescan after taking the
	 *    orphan lock.
	 */

	while ((pvl = (struct pv_list *) dm_list_first(&head.list))) {
		dm_list_del(&pvl->list);
		add_pvl_to_vgs(vg, pvl);
		vg->extent_count += pvl->pv->pe_count;
		vg->free_count += pvl->pv->pe_count;
	}

	if (!lvmcache_foreach_pv(vginfo, _vg_read_orphan_pv, &baton))
		return_NULL;

	return vg;
}

static int _update_pv_list(struct dm_pool *pvmem, struct dm_list *all_pvs, struct volume_group *vg)
{
	struct pv_list *pvl, *pvl2;

	dm_list_iterate_items(pvl, &vg->pvs) {
		dm_list_iterate_items(pvl2, all_pvs) {
			if (pvl->pv->dev == pvl2->pv->dev)
				goto next_pv;
		}

		/*
		 * PV is not on list so add it.
		 */
		if (!(pvl2 = _copy_pvl(pvmem, pvl))) {
			log_error("pv_list allocation for '%s' failed",
				  pv_dev_name(pvl->pv));
			return 0;
		}
		dm_list_add(all_pvs, &pvl2->list);
  next_pv:
		;
	}

	return 1;
}

static void _free_pv_list(struct dm_list *all_pvs)
{
	struct pv_list *pvl;

	dm_list_iterate_items(pvl, all_pvs)
		pvl->pv->fid->fmt->ops->destroy_instance(pvl->pv->fid);
}

static void _destroy_fid(struct format_instance **fid)
{
	if (*fid) {
		(*fid)->fmt->ops->destroy_instance(*fid);
		*fid = NULL;
	}
}

int vg_missing_pv_count(const struct volume_group *vg)
{
	int ret = 0;
	struct pv_list *pvl;
	dm_list_iterate_items(pvl, &vg->pvs) {
		if (is_missing_pv(pvl->pv))
			++ ret;
	}
	return ret;
}

static int _check_reappeared_pv(struct volume_group *correct_vg,
				struct physical_volume *pv, int act)
{
	struct pv_list *pvl;
	int rv = 0;

        /*
         * Skip these checks in case the tool is going to deal with missing
         * PVs, especially since the resulting messages can be pretty
         * confusing.
         */
        if (correct_vg->cmd->handles_missing_pvs)
		return rv;

	/*
	 * Skip this if there is no underlying device present for this PV.
	 */
	if (!pv->dev)
		return rv;

	dm_list_iterate_items(pvl, &correct_vg->pvs)
		if (pv->dev == pvl->pv->dev && is_missing_pv(pvl->pv)) {
			if (act)
				log_warn("WARNING: Missing device %s reappeared, updating "
					 "metadata for VG %s to version %u.",
					 pv_dev_name(pvl->pv),  pv_vg_name(pvl->pv), 
					 correct_vg->seqno);
			if (pvl->pv->pe_alloc_count == 0) {
				if (act) {
					pv->status &= ~MISSING_PV;
					pvl->pv->status &= ~MISSING_PV;
				}
				++ rv;
			} else if (act)
				log_warn("WARNING: Device %s still marked missing because of allocated data "
					 "on it, remove volumes and consider vgreduce --removemissing.",
					 pv_dev_name(pvl->pv));
		}

	return rv;
}

static int _is_foreign_vg(struct volume_group *vg)
{
	return vg->cmd->system_id && strcmp(vg->system_id, vg->cmd->system_id);
}

static int _repair_inconsistent_vg(struct volume_group *vg, uint32_t lockd_state)
{
	unsigned saved_handles_missing_pvs = vg->cmd->handles_missing_pvs;

	if (lvmcache_found_duplicate_pvs()) {
		log_debug_metadata("Skip metadata repair with duplicates.");
		return 0;
	}

	/* Cannot write foreign VGs, the owner will repair it. */
	if (_is_foreign_vg(vg)) {
		log_verbose("Skip metadata repair for foreign VG.");
		return 0;
	}

	if (vg_is_shared(vg) && !(lockd_state & LDST_EX)) {
		log_verbose("Skip metadata repair for shared VG without exclusive lock.");
		return 0;
	}

	log_warn("WARNING: Inconsistent metadata found for VG %s - updating to use version %u", vg->name, vg->seqno);

	vg->cmd->handles_missing_pvs = 1;
	if (!vg_write(vg)) {
		log_error("Automatic metadata correction failed");
		vg->cmd->handles_missing_pvs = saved_handles_missing_pvs;
		return 0;
	}

	vg->cmd->handles_missing_pvs = saved_handles_missing_pvs;

	if (!vg_commit(vg)) {
		log_error("Automatic metadata correction commit failed");
		return 0;
	}

	return 1;
}

static int _wipe_outdated_pvs(struct cmd_context *cmd, struct volume_group *vg, struct dm_list *to_check, uint32_t lockd_state)
{
	struct pv_list *pvl, *pvl2;
	char uuid[64] __attribute__((aligned(8)));

	if (lvmcache_found_duplicate_pvs()) {
		log_debug_metadata("Skip wiping outdated PVs with duplicates.");
		return 0;
	}

	/*
	 * Cannot write foreign VGs, the owner will repair it.
	 * Also, if another host is updating its VG, we may read
	 * the PVs while some are written but not others, making
	 * some PVs look outdated to us just because we're reading
	 * the VG while it's only partially written out.
	 */
	if (_is_foreign_vg(vg)) {
		log_debug_metadata("Skip wiping outdated PVs for foreign VG.");
		return 0;
	}

	if (vg_is_shared(vg) && !(lockd_state & LDST_EX)) {
		log_verbose("Skip wiping outdated PVs for shared VG without exclusive lock.");
		return 0;
	}

	dm_list_iterate_items(pvl, to_check) {
		dm_list_iterate_items(pvl2, &vg->pvs) {
			if (pvl->pv->dev == pvl2->pv->dev)
				goto next_pv;
		}


		if (!id_write_format(&pvl->pv->id, uuid, sizeof(uuid)))
			return_0;
		log_warn("WARNING: Removing PV %s (%s) that no longer belongs to VG %s",
			 pv_dev_name(pvl->pv), uuid, vg->name);
		if (!pv_write_orphan(cmd, pvl->pv))
			return_0;
next_pv:
		;
	}
	return 1;
}

static int _check_or_repair_pv_ext(struct cmd_context *cmd,
				   struct volume_group *vg,
				   uint32_t lockd_state,
				   int repair, int *inconsistent_pvs)
{
	char uuid[64] __attribute__((aligned(8)));
	struct lvmcache_info *info;
	uint32_t ext_version, ext_flags;
	struct pv_list *pvl;
	unsigned pvs_fixed = 0;
	int r = 0;

	*inconsistent_pvs = 0;

	dm_list_iterate_items(pvl, &vg->pvs) {
		/* Missing PV - nothing to do. */
		if (is_missing_pv(pvl->pv))
			continue;

		if (!pvl->pv->dev) {
			/* is_missing_pv doesn't catch NULL dev */
			memset(&uuid, 0, sizeof(uuid));
			if (!id_write_format(&pvl->pv->id, uuid, sizeof(uuid)))
				goto_out;
			log_warn("WARNING: Not repairing PV %s with missing device.", uuid);
			continue;
		}

		if (!(info = lvmcache_info_from_pvid(pvl->pv->dev->pvid, pvl->pv->dev, 0))) {
			log_error("Failed to find cached info for PV %s.", pv_dev_name(pvl->pv));
			goto out;
		}

		ext_version = lvmcache_ext_version(info);
		if (ext_version < 2)
			continue;

		ext_flags = lvmcache_ext_flags(info);
		if (!(ext_flags & PV_EXT_USED)) {
			if (!repair) {
				*inconsistent_pvs = 1;
				/* we're not repairing now, so no need to
				 * check further PVs - inconsistent_pvs is already
				 * set and that will trigger the repair next time */
				return 1;
			}

			if (_is_foreign_vg(vg)) {
				log_verbose("Skip repair of PV %s that is in foreign "
					    "VG %s but not marked as used.",
					    pv_dev_name(pvl->pv), vg->name);
				*inconsistent_pvs = 1;
			} else if (vg_is_shared(vg) && !(lockd_state & LDST_EX)) {
				log_warn("Skip repair of PV %s that is in shared "
					    "VG %s but not marked as used.",
					    pv_dev_name(pvl->pv), vg->name);
				*inconsistent_pvs = 1;
			} else {
				log_warn("WARNING: Repairing Physical Volume %s that is "
					 "in Volume Group %s but not marked as used.",
					  pv_dev_name(pvl->pv), vg->name);

				/* pv write will set correct ext_flags */
				if (!pv_write(cmd, pvl->pv, 1)) {
					*inconsistent_pvs = 1;
					log_error("Failed to repair physical volume \"%s\".",
						  pv_dev_name(pvl->pv));
					goto out;
				}
				pvs_fixed++;
			}
		}
	}

	r = 1;
out:
	if ((pvs_fixed > 0) && !_repair_inconsistent_vg(vg, lockd_state))
		return_0;

	return r;
}

/* Caller sets consistent to 1 if it's safe for vg_read_internal to correct
 * inconsistent metadata on disk (i.e. the VG write lock is held).
 * This guarantees only consistent metadata is returned.
 * If consistent is 0, caller must check whether consistent == 1 on return
 * and take appropriate action if it isn't (e.g. abort; get write lock
 * and call vg_read_internal again).
 *
 * If precommitted is set, use precommitted metadata if present.
 *
 * Either of vgname or vgid may be NULL.
 *
 * Note: vginfo structs must not be held or used as parameters
 *       across the call to this function.
 */
static struct volume_group *_vg_read(struct cmd_context *cmd,
				     const char *vgname,
				     const char *vgid,
				     uint32_t lockd_state, 
				     uint32_t warn_flags, 
				     int enable_repair,
				     int *mdas_consistent,
				     unsigned precommitted)
{
	struct format_instance *fid = NULL;
	struct format_instance_ctx fic;
	const struct format_type *fmt;
	struct volume_group *vg, *correct_vg = NULL;
	struct metadata_area *mda;
	struct lvmcache_info *info;
	int inconsistent = 0;
	int inconsistent_vgid = 0;
	int inconsistent_pvs = 0;
	int inconsistent_mdas = 0;
	int inconsistent_mda_count = 0;
	int strip_historical_lvs = enable_repair;
	int update_old_pv_ext = enable_repair;
	unsigned use_precommitted = precommitted;
	struct dm_list *pvids;
	struct pv_list *pvl;
	struct dm_list all_pvs;
	char uuid[64] __attribute__((aligned(8)));
	int skipped_rescan = 0;
	struct cached_vg_fmtdata *vg_fmtdata = NULL;	/* Additional format-specific data about the vg */
	unsigned use_previous_vg;

	*mdas_consistent = 1;

	if (is_orphan_vg(vgname)) {
		log_very_verbose("Reading VG %s", vgname);

		if (use_precommitted) {
			log_error(INTERNAL_ERROR "vg_read_internal requires vgname "
				  "with pre-commit.");
			return NULL;
		}
		return vg_read_orphans(cmd, warn_flags, vgname);
	}

	uuid[0] = '\0';
	if (vgid && !id_write_format((const struct id*)vgid, uuid, sizeof(uuid)))
		stack;

	log_very_verbose("Reading VG %s %s", vgname ?: "<no name>", vgid ? uuid : "<no vgid>");

	/*
	 * Rescan the devices that are associated with this vg in lvmcache.
	 * This repeats what was done by the command's initial label scan,
	 * but only the devices associated with this VG.
	 *
	 * The lvmcache info about these devs is from the initial label scan
	 * performed by the command before the vg lock was held.  Now the VG
	 * lock is held, so we rescan all the info from the devs in case
	 * something changed between the initial scan and now that the lock
	 * is held.
	 *
	 * Some commands (e.g. reporting) are fine reporting data read by
	 * the label scan.  It doesn't matter if the devs changed between
	 * the label scan and here, we can report what was seen in the
	 * scan, even though it is the old state, since we will not be
	 * making any modifications.  If the VG was being modified during
	 * the scan, and caused us to see inconsistent metadata on the
	 * different PVs in the VG, then we do want to rescan the devs
	 * here to get a consistent view of the VG.  Note that we don't
	 * know if the scan found all the PVs in the VG at this point.
	 * We don't know that until vg_read looks at the list of PVs in
	 * the metadata and compares that to the devices found by the scan.
	 *
	 * It's possible that a change made to the VG during scan was
	 * adding or removing a PV from the VG.  In this case, the list
	 * of devices associated with the VG in lvmcache would change
	 * due to the rescan.
	 *
	 * The devs in the VG may be persistently inconsistent due to some
	 * previous problem.  In this case, rescanning the labels here will
	 * find the same inconsistency.  The VG repair (mistakenly done by
	 * vg_read below) is supposed to fix that.
	 *
	 * FIXME: sort out the usage of the global lock (which is mixed up
	 * with the orphan lock), and when we can tell that the global
	 * lock is taken prior to the label scan, and still held here,
	 * we can also skip the rescan in that case.
	 */
	if (!cmd->can_use_one_scan || lvmcache_scan_mismatch(cmd, vgname, vgid)) {
		/* the skip rescan special case is for clvmd vg_read_by_vgid */
		/* FIXME: this is not a warn flag, pass this differently */
		if (warn_flags & SKIP_RESCAN)
			goto find_vg;
		skipped_rescan = 0;
		log_debug_metadata("Rescanning devices for %s", vgname);
		lvmcache_label_rescan_vg(cmd, vgname, vgid);
	} else {
		log_debug_metadata("Skipped rescanning devices for %s", vgname);
		skipped_rescan = 1;
	}

 find_vg:

	if (!(fmt = lvmcache_fmt_from_vgname(cmd, vgname, vgid, 0))) {
		log_debug_metadata("Cache did not find fmt for vgname %s", vgname);
		return_NULL;
	}

	/* Now determine the correct vgname if none was supplied */
	if (!vgname && !(vgname = lvmcache_vgname_from_vgid(cmd->mem, vgid))) {
		log_debug_metadata("Cache did not find VG name from vgid %s", uuid);
		return_NULL;
	}

	/* Determine the correct vgid if none was supplied */
	if (!vgid && !(vgid = lvmcache_vgid_from_vgname(cmd, vgname))) {
		log_debug_metadata("Cache did not find VG vgid from name %s", vgname);
		return_NULL;
	}

	if (use_precommitted && !(fmt->features & FMT_PRECOMMIT))
		use_precommitted = 0;

	/*
	 * A "format instance" is an abstraction for a VG location,
	 * i.e. where a VG's metadata exists on disk.
	 *
	 * An fic (format_instance_ctx) is a temporary struct used
	 * to create an fid (format_instance).  The fid hangs around
	 * and is used to create a 'vg' to which it connected (vg->fid).
	 *
	 * The 'fic' describes a VG in terms of fmt/name/id.
	 *
	 * The 'fid' describes a VG in more detail than the fic,
	 * holding information about where to find the VG metadata.
	 *
	 * The 'vg' describes the VG in the most detail representing
	 * all the VG metadata.
	 *
	 * The fic and fid are set up by create_instance() to describe
	 * the VG location.  This happens before the VG metadata is
	 * assembled into the more familiar struct volume_group "vg".
	 *
	 * The fid has one main purpose: to keep track of the metadata
	 * locations for a given VG.  It does this by putting 'mda'
	 * structs on fid->metadata_areas_in_use, which specify where
	 * metadata is located on disk.  It gets this information
	 * (metadata locations for a specific VG) from the command's
	 * initial label scan.  The info is passed indirectly via
	 * lvmcache info/vginfo structs, which are created by the
	 * label scan and then copied into fid by create_instance().
	 */

	/* create format instance with appropriate metadata area */
	fic.type = FMT_INSTANCE_MDAS | FMT_INSTANCE_AUX_MDAS;
	fic.context.vg_ref.vg_name = vgname;
	fic.context.vg_ref.vg_id = vgid;
	if (!(fid = fmt->ops->create_instance(fmt, &fic))) {
		log_error("Failed to create format instance");
		return NULL;
	}

	/* Store pvids for later so we can check if any are missing */
	if (!(pvids = lvmcache_get_pvids(cmd, vgname, vgid))) {
		_destroy_fid(&fid);
		return_NULL;
	}

	/*
	 * We use the fid globally here so prevent the release_vg
	 * call to destroy the fid - we may want to reuse it!
	 */
	fid->ref_count++;
	/* Ensure contents of all metadata areas match - else do recovery */
	inconsistent_mda_count=0;
	dm_list_iterate_items(mda, &fid->metadata_areas_in_use) {
		struct device *mda_dev = mda_get_device(mda);

		use_previous_vg = 0;

		log_debug_metadata("Reading VG %s from %s", vgname, dev_name(mda_dev));

		if ((use_precommitted &&
		     !(vg = mda->ops->vg_read_precommit(fid, vgname, mda, &vg_fmtdata, &use_previous_vg)) && !use_previous_vg) ||
		    (!use_precommitted &&
		     !(vg = mda->ops->vg_read(fid, vgname, mda, &vg_fmtdata, &use_previous_vg)) && !use_previous_vg)) {
			inconsistent = 1;
			vg_fmtdata = NULL;
			continue;
		}

		/* Use previous VG because checksum matches */
		if (!vg) {
			vg = correct_vg;
			continue;
		}

		if (!correct_vg) {
			correct_vg = vg;
			continue;
		}

		/* FIXME Also ensure contents same - checksum compare? */
		if (correct_vg->seqno != vg->seqno) {
			if (cmd->metadata_read_only || skipped_rescan)
				log_warn("Not repairing metadata for VG %s.", vgname);
			else
				inconsistent = 1;

			if (vg->seqno > correct_vg->seqno) {
				release_vg(correct_vg);
				correct_vg = vg;
			} else {
				mda->status |= MDA_INCONSISTENT;
				++inconsistent_mda_count;
			}
		}

		if (vg != correct_vg) {
			release_vg(vg);
			vg_fmtdata = NULL;
		}
	}
	fid->ref_count--;

	/* Ensure every PV in the VG was in the cache */
	if (correct_vg) {
		/*
		 * Update the seqno from the cache, for the benefit of
		 * retro-style metadata formats like LVM1.
		 */
		// correct_vg->seqno = seqno > correct_vg->seqno ? seqno : correct_vg->seqno;

		/*
		 * If the VG has PVs without mdas, or ignored mdas, they may
		 * still be orphans in the cache: update the cache state here,
		 * and update the metadata lists in the vg.
		 */
		if (!inconsistent &&
		    dm_list_size(&correct_vg->pvs) > dm_list_size(pvids)) {
			dm_list_iterate_items(pvl, &correct_vg->pvs) {
				if (!pvl->pv->dev) {
					inconsistent_pvs = 1;
					break;
				}

				if (str_list_match_item(pvids, pvl->pv->dev->pvid))
					continue;

				/*
				 * PV not marked as belonging to this VG in cache.
				 * Check it's an orphan without metadata area
				 * not ignored.
				 */
				if (!(info = lvmcache_info_from_pvid(pvl->pv->dev->pvid, pvl->pv->dev, 1)) ||
				    !lvmcache_is_orphan(info)) {
					inconsistent_pvs = 1;
					break;
				}

				if (lvmcache_mda_count(info)) {
					if (!lvmcache_fid_add_mdas_pv(info, fid)) {
						release_vg(correct_vg);
						return_NULL;
					}

					log_debug_metadata("Empty mda found for VG %s on %s.",
							   vgname, dev_name(pvl->pv->dev));

#if 0
					/*
					 * If we are going to do any repair we have to be using 
					 * the latest metadata on disk, so we have to rescan devs
					 * if we skipped that at the start of the vg_read.  We'll
					 * likely come back through here, but without having
					 * skipped_rescan.
					 *
					 * FIXME: in some cases we don't want to do this.
					 */
					if (skipped_rescan && cmd->can_use_one_scan) {
						log_debug_metadata("Restarting read to rescan devs.");
						cmd->can_use_one_scan = 0;
						release_vg(correct_vg);
						correct_vg = NULL;
						lvmcache_del(info);
						label_read(pvl->pv->dev);
						goto restart_scan;
					}
#endif

					if (inconsistent_mdas)
						continue;

					/*
					 * If any newly-added mdas are in-use then their
					 * metadata needs updating.
					 */
					lvmcache_foreach_mda(info, _check_mda_in_use,
							     &inconsistent_mdas);
				}
			}

			/* If the check passed, let's update VG and recalculate pvids */
			if (!inconsistent_pvs) {
				log_debug_metadata("Updating cache for PVs without mdas "
						   "in VG %s.", vgname);
				/*
				 * If there is no precommitted metadata, committed metadata
				 * is read and stored in the cache even if use_precommitted is set
				 */
				lvmcache_update_vg(correct_vg, correct_vg->status & PRECOMMITTED);

				if (!(pvids = lvmcache_get_pvids(cmd, vgname, vgid))) {
					release_vg(correct_vg);
					return_NULL;
				}
			}
		}

		fid->ref_count++;
		if (dm_list_size(&correct_vg->pvs) !=
		    dm_list_size(pvids) + vg_missing_pv_count(correct_vg)) {
			log_debug_metadata("Cached VG %s had incorrect PV list",
					   vgname);

			if (prioritized_section())
				inconsistent = 1;
			else {
				release_vg(correct_vg);
				correct_vg = NULL;
			}
		} else dm_list_iterate_items(pvl, &correct_vg->pvs) {
			if (is_missing_pv(pvl->pv))
				continue;
			if (!str_list_match_item(pvids, pvl->pv->dev->pvid)) {
				log_debug_metadata("Cached VG %s had incorrect PV list",
						   vgname);
				release_vg(correct_vg);
				correct_vg = NULL;
				break;
			}
		}

		if (correct_vg && inconsistent_mdas) {
			release_vg(correct_vg);
			correct_vg = NULL;
		}
		fid->ref_count--;
	}

	dm_list_init(&all_pvs);

	/* Failed to find VG where we expected it - full scan and retry */
	if (!correct_vg) {
		/*
		 * Free outstanding format instance that remained unassigned
		 * from previous step where we tried to get the "correct_vg",
		 * but we failed to do so (so there's a dangling fid now).
		 */
		_destroy_fid(&fid);
		vg_fmtdata = NULL;

		inconsistent = 0;

		if (!(fmt = lvmcache_fmt_from_vgname(cmd, vgname, vgid, 0)))
			return_NULL;

		if (precommitted && !(fmt->features & FMT_PRECOMMIT))
			use_precommitted = 0;

		/* create format instance with appropriate metadata area */
		fic.type = FMT_INSTANCE_MDAS | FMT_INSTANCE_AUX_MDAS;
		fic.context.vg_ref.vg_name = vgname;
		fic.context.vg_ref.vg_id = vgid;
		if (!(fid = fmt->ops->create_instance(fmt, &fic))) {
			log_error("Failed to create format instance");
			return NULL;
		}

		/*
		 * We use the fid globally here so prevent the release_vg
		 * call to destroy the fid - we may want to reuse it!
		*/
		fid->ref_count++;
		/* Ensure contents of all metadata areas match - else recover */
		inconsistent_mda_count=0;
		dm_list_iterate_items(mda, &fid->metadata_areas_in_use) {
			use_previous_vg = 0;

			if ((use_precommitted &&
			     !(vg = mda->ops->vg_read_precommit(fid, vgname, mda, &vg_fmtdata, &use_previous_vg)) && !use_previous_vg) ||
			    (!use_precommitted &&
			     !(vg = mda->ops->vg_read(fid, vgname, mda, &vg_fmtdata, &use_previous_vg)) && !use_previous_vg)) {
				inconsistent = 1;
				vg_fmtdata = NULL;
				continue;
			}

			/* Use previous VG because checksum matches */
			if (!vg) {
				vg = correct_vg;
				continue;
			}

			if (!correct_vg) {
				correct_vg = vg;
				if (!_update_pv_list(cmd->mem, &all_pvs, correct_vg)) {
					_free_pv_list(&all_pvs);
					fid->ref_count--;
					release_vg(vg);
					return_NULL;
				}
				continue;
			}

			if (!id_equal(&vg->id, &correct_vg->id)) {
				inconsistent = 1;
				inconsistent_vgid = 1;
			}

			/* FIXME Also ensure contents same - checksums same? */
			if (correct_vg->seqno != vg->seqno) {
				/* Ignore inconsistent seqno if told to skip repair logic */
				if (cmd->metadata_read_only || skipped_rescan)
					log_warn("Not repairing metadata for VG %s.", vgname);
				else
					inconsistent = 1;

				if (!_update_pv_list(cmd->mem, &all_pvs, vg)) {
					_free_pv_list(&all_pvs);
					fid->ref_count--;
					release_vg(vg);
					release_vg(correct_vg);
					return_NULL;
				}
				if (vg->seqno > correct_vg->seqno) {
					release_vg(correct_vg);
					correct_vg = vg;
				} else {
					mda->status |= MDA_INCONSISTENT;
					++inconsistent_mda_count;
				}
			}

			if (vg != correct_vg) {
				release_vg(vg);
				vg_fmtdata = NULL;
			}
		}
		fid->ref_count--;

		/* Give up looking */
		if (!correct_vg) {
			_free_pv_list(&all_pvs);
			_destroy_fid(&fid);
			return_NULL;
		}
	}

	/*
	 * If there is no precommitted metadata, committed metadata
	 * is read and stored in the cache even if use_precommitted is set
	 */
	lvmcache_update_vg(correct_vg, (correct_vg->status & PRECOMMITTED));

	if (inconsistent) {
		/* FIXME Test should be if we're *using* precommitted metadata not if we were searching for it */
		if (use_precommitted) {
			log_error("Inconsistent pre-commit metadata copies "
				  "for volume group %s", vgname);

			/*
			 * Check whether all of the inconsistent MDAs were on
			 * MISSING PVs -- in that case, we should be safe.
			 */
			dm_list_iterate_items(mda, &fid->metadata_areas_in_use) {
				if (mda->status & MDA_INCONSISTENT) {
					log_debug_metadata("Checking inconsistent MDA: %s", dev_name(mda_get_device(mda)));
					dm_list_iterate_items(pvl, &correct_vg->pvs) {
						if (mda_get_device(mda) == pvl->pv->dev &&
						    (pvl->pv->status & MISSING_PV))
							--inconsistent_mda_count;
					}
				}
			}

			if (inconsistent_mda_count < 0)
				log_error(INTERNAL_ERROR "Too many inconsistent MDAs.");

			if (!inconsistent_mda_count) {
				_free_pv_list(&all_pvs);
				return correct_vg;
			}
			_free_pv_list(&all_pvs);
			release_vg(correct_vg);
			return NULL;
		}

		if (!enable_repair) {
			_free_pv_list(&all_pvs);
			*mdas_consistent = 0;
			return correct_vg;
		}

		if (skipped_rescan) {
			log_warn("Not repairing metadata for VG %s.", vgname);
			_free_pv_list(&all_pvs);
			release_vg(correct_vg);
			return_NULL;
		}

		/* Don't touch if vgids didn't match */
		if (inconsistent_vgid) {
			log_warn("WARNING: Inconsistent metadata UUIDs found for volume group %s.", vgname);
			_free_pv_list(&all_pvs);
			*mdas_consistent = 0;
			return correct_vg;
		}

		/*
		 * If PV is marked missing but we found it,
		 * update metadata and remove MISSING flag
		 */
		dm_list_iterate_items(pvl, &all_pvs)
			_check_reappeared_pv(correct_vg, pvl->pv, 1);

		if (!_repair_inconsistent_vg(correct_vg, lockd_state)) {
			_free_pv_list(&all_pvs);
			release_vg(correct_vg);
			return NULL;
		}

		if (!_wipe_outdated_pvs(cmd, correct_vg, &all_pvs, lockd_state)) {
			_free_pv_list(&all_pvs);
			release_vg(correct_vg);
			return_NULL;
		}
	}

	_free_pv_list(&all_pvs);

	if (vg_missing_pv_count(correct_vg)) {
		log_verbose("There are %d physical volumes missing.",
			    vg_missing_pv_count(correct_vg));
		vg_mark_partial_lvs(correct_vg, 1);
	}

	if ((correct_vg->status & PVMOVE) && !pvmove_mode()) {
		log_error("Interrupted pvmove detected in volume group %s.",
			  correct_vg->name);
		log_print("Please restore the metadata by running vgcfgrestore.");
		release_vg(correct_vg);
		return NULL;
	}

	/* We have the VG now finally, check if PV ext info is in sync with VG metadata. */
	if (!_check_or_repair_pv_ext(cmd, correct_vg, lockd_state, skipped_rescan ? 0 : enable_repair,
				     &inconsistent_pvs)) {
		release_vg(correct_vg);
		return_NULL;
	}

	if (correct_vg && enable_repair && !skipped_rescan) {
		if (update_old_pv_ext && !_vg_update_old_pv_ext_if_needed(correct_vg)) {
			release_vg(correct_vg);
			return_NULL;
		}

		if (strip_historical_lvs && !vg_strip_outdated_historical_lvs(correct_vg)) {
			release_vg(correct_vg);
			return_NULL;
		}
	}

	if (inconsistent_pvs)
		*mdas_consistent = 0;

	return correct_vg;
}

#define DEV_LIST_DELIM ", "

static int _check_devs_used_correspond_with_lv(struct dm_pool *mem, struct dm_list *list, struct logical_volume *lv)
{
	struct device_list *dl;
	int found_inconsistent = 0;
	struct device *dev;
	struct lv_segment *seg;
	uint32_t s;
	int warned_about_no_dev = 0;
	char *used_devnames = NULL, *assumed_devnames = NULL;

	if (!(list = dev_cache_get_dev_list_for_lvid(lv->lvid.s + ID_LEN)))
		return 1;

	dm_list_iterate_items(dl, list) {
		dev = dl->dev;
		if (!(dev->flags & DEV_ASSUMED_FOR_LV)) {
			if (!found_inconsistent) {
				if (!dm_pool_begin_object(mem, 32))
					return_0;
				found_inconsistent = 1;
			} else {
				if (!dm_pool_grow_object(mem, DEV_LIST_DELIM, sizeof(DEV_LIST_DELIM) - 1))
					return_0;
			}
			if (!dm_pool_grow_object(mem, dev_name(dev), 0))
				return_0;
		}
	}

	if (!found_inconsistent)
		return 1;

	if (!dm_pool_grow_object(mem, "\0", 1))
		return_0;
	used_devnames = dm_pool_end_object(mem);

	found_inconsistent = 0;
	dm_list_iterate_items(seg, &lv->segments) {
		for (s = 0; s < seg->area_count; s++) {
			if (seg_type(seg, s) == AREA_PV) {
				if (!(dev = seg_dev(seg, s))) {
					if (!warned_about_no_dev) {
						log_warn("WARNING: Couldn't find all devices for LV %s "
							 "while checking used and assumed devices.",
							  display_lvname(lv));
						warned_about_no_dev = 1;
					}
					continue;
				}
				if (!(dev->flags & DEV_USED_FOR_LV)) {
					if (!found_inconsistent) {
						if (!dm_pool_begin_object(mem, 32))
                                                        return_0;
						found_inconsistent = 1;
					} else {
						if (!dm_pool_grow_object(mem, DEV_LIST_DELIM, sizeof(DEV_LIST_DELIM) - 1))
							return_0;
					}
					if (!dm_pool_grow_object(mem, dev_name(dev), 0))
						return_0;
				}
			}
		}
	}

	if (found_inconsistent) {
		if (!dm_pool_grow_object(mem, "\0", 1))
			return_0;
		assumed_devnames = dm_pool_end_object(mem);
		log_warn("WARNING: Device mismatch detected for %s which is accessing %s instead of %s.",
			 display_lvname(lv), used_devnames, assumed_devnames);
	}

	return 1;
}

static int _check_devs_used_correspond_with_vg(struct volume_group *vg)
{
	struct dm_pool *mem;
	char vgid[ID_LEN + 1];
	struct pv_list *pvl;
	struct lv_list *lvl;
	struct dm_list *list;
	struct device_list *dl;
	int found_inconsistent = 0;

	if (is_orphan_vg(vg->name))
		return 1;

	strncpy(vgid, (const char *) vg->id.uuid, sizeof(vgid));
	vgid[ID_LEN] = '\0';

	/* Mark all PVs in VG as used. */
	dm_list_iterate_items(pvl, &vg->pvs) {
		/*
		 * FIXME: It's not clear if the meaning
		 * of "missing" should always include the
		 * !pv->dev case, or if "missing" is the
		 * more narrow case where VG metadata has
		 * been written with the MISSING flag.
		 */
		if (!pvl->pv->dev)
			continue;
		if (is_missing_pv(pvl->pv))
			continue;
		pvl->pv->dev->flags |= DEV_ASSUMED_FOR_LV;
	}

	if (!(list = dev_cache_get_dev_list_for_vgid(vgid)))
		return 1;

	dm_list_iterate_items(dl, list) {
		if (!(dl->dev->flags & DEV_OPEN_FAILURE) &&
		    !(dl->dev->flags & DEV_ASSUMED_FOR_LV)) {
			found_inconsistent = 1;
			break;
		}
	}

	if (found_inconsistent) {
		if (!(mem = dm_pool_create("vg_devs_check", 1024)))
			return_0;

		dm_list_iterate_items(lvl, &vg->lvs) {
			if (!_check_devs_used_correspond_with_lv(mem, list, lvl->lv)) {
				dm_pool_destroy(mem);
				return_0;
			}
		}

		dm_pool_destroy(mem);
	}

	return 1;
}

struct volume_group *vg_read_internal(struct cmd_context *cmd,
				      const char *vgname, const char *vgid,
				      uint32_t lockd_state, uint32_t warn_flags,
				      int enable_repair,
				      int *mdas_consistent)
{
	struct volume_group *vg;
	struct lv_list *lvl;

	if (!(vg = _vg_read(cmd, vgname, vgid, lockd_state,
			    warn_flags, enable_repair, mdas_consistent, 0)))
		goto_out;

	if (!check_pv_dev_sizes(vg))
		log_warn("One or more devices used as PVs in VG %s "
			 "have changed sizes.", vg->name);

	if (!check_pv_segments(vg)) {
		log_error(INTERNAL_ERROR "PV segments corrupted in %s.",
			  vg->name);
		release_vg(vg);
		vg = NULL;
		goto out;
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		if (!check_lv_segments(lvl->lv, 0)) {
			log_error(INTERNAL_ERROR "LV segments corrupted in %s.",
				  lvl->lv->name);
			release_vg(vg);
			vg = NULL;
			goto out;
		}
	}

	dm_list_iterate_items(lvl, &vg->lvs) {
		/*
		 * Checks that cross-reference other LVs.
		 */
		if (!check_lv_segments(lvl->lv, 1)) {
			log_error(INTERNAL_ERROR "LV segments corrupted in %s.",
				  lvl->lv->name);
			release_vg(vg);
			vg = NULL;
			goto out;
		}
	}

	(void) _check_devs_used_correspond_with_vg(vg);
out:
	if (!*mdas_consistent && (warn_flags & WARN_INCONSISTENT)) {
		if (is_orphan_vg(vgname))
			log_warn("WARNING: Found inconsistent standalone Physical Volumes.");
		else
			log_warn("WARNING: Volume Group %s is not consistent.", vgname);
	}

	return vg;
}

void free_pv_fid(struct physical_volume *pv)
{
	if (!pv)
		return;

	pv_set_fid(pv, NULL);
}

static struct physical_volume *_pv_read(struct cmd_context *cmd,
					const struct format_type *fmt,
					struct volume_group *vg,
					struct lvmcache_info *info)
{
	struct physical_volume *pv;
	struct device *dev = lvmcache_device(info);

	if (!(pv = _alloc_pv(vg->vgmem, NULL))) {
		log_error("pv allocation failed");
		return NULL;
	}

	if (fmt->ops->pv_read) {
		/* format1 and pool */
		if (!(fmt->ops->pv_read(fmt, dev_name(dev), pv, 0))) {
			log_error("Failed to read existing physical volume '%s'", dev_name(dev));
			goto bad;
		}
	} else {
		/* format text */
		if (!lvmcache_populate_pv_fields(info, vg, pv))
			goto_bad;
	}

	if (!alloc_pv_segment_whole_pv(vg->vgmem, pv))
		goto_bad;

	lvmcache_fid_add_mdas(info, vg->fid, (const char *) &pv->id, ID_LEN);
	pv_set_fid(pv, vg->fid);
	return pv;
bad:
	free_pv_fid(pv);
	dm_pool_free(vg->vgmem, pv);
	return NULL;
}

int get_vgnameids(struct cmd_context *cmd, struct dm_list *vgnameids,
		  const char *only_this_vgname, int include_internal)
{
	struct vgnameid_list *vgnl;

	if (only_this_vgname) {
		if (!(vgnl = dm_pool_alloc(cmd->mem, sizeof(*vgnl)))) {
			log_error("vgnameid_list allocation failed.");
			return 0;
		}

		vgnl->vg_name = dm_pool_strdup(cmd->mem, only_this_vgname);
		vgnl->vgid = NULL;
		dm_list_add(vgnameids, &vgnl->list);
		return 1;
	}

	lvmcache_get_vgnameids(cmd, include_internal, vgnameids);

	return 1;
}

int pv_write(struct cmd_context *cmd,
	     struct physical_volume *pv, int allow_non_orphan)
{
	if (!pv->fmt->ops->pv_write) {
		log_error("Format does not support writing physical volumes");
		return 0;
	}

	/*
	 * FIXME: Try to remove this restriction. This requires checking
	 *        that the PV and the VG are in a consistent state. We need
	 *        to provide some revert mechanism since PV label together
	 *        with VG metadata write is not atomic.
	 */
	if (!allow_non_orphan &&
	    (!is_orphan_vg(pv->vg_name) || pv->pe_alloc_count)) {
		log_error("Assertion failed: can't _pv_write non-orphan PV "
			  "(in VG %s)", pv_vg_name(pv));
		return 0;
	}

	if (!pv->fmt->ops->pv_write(pv->fmt, pv))
		return_0;

	pv->status &= ~UNLABELLED_PV;

	return 1;
}

int pv_write_orphan(struct cmd_context *cmd, struct physical_volume *pv)
{
	const char *old_vg_name = pv->vg_name;

	pv->vg_name = cmd->fmt->orphan_vg_name;
	pv->status = ALLOCATABLE_PV;
	pv->pe_alloc_count = 0;

	if (!dev_get_size(pv->dev, &pv->size)) {
		log_error("%s: Couldn't get size.", pv_dev_name(pv));
		return 0;
	}

	if (!pv_write(cmd, pv, 0)) {
		log_error("Failed to clear metadata from physical "
			  "volume \"%s\" after removal from \"%s\"",
			  pv_dev_name(pv), old_vg_name);
		return 0;
	}

	return 1;
}

int is_global_vg(const char *vg_name)
{
	return (vg_name && !strcmp(vg_name, VG_GLOBAL)) ? 1 : 0;
}

/**
 * is_orphan_vg - Determine whether a vg_name is an orphan
 * @vg_name: pointer to the vg_name
 */
int is_orphan_vg(const char *vg_name)
{
	return (vg_name && !strncmp(vg_name, ORPHAN_PREFIX, sizeof(ORPHAN_PREFIX) - 1)) ? 1 : 0;
}

/*
 * Exclude pseudo VG names used for locking.
 */
int is_real_vg(const char *vg_name)
{
	return (vg_name && *vg_name != '#');
}

static int _analyze_mda(struct metadata_area *mda, void *baton)
{
	const struct format_type *fmt = baton;
	mda->ops->pv_analyze_mda(fmt, mda);
	return 1;
}

/*
 * Returns:
 *  0 - fail
 *  1 - success
 */
int pv_analyze(struct cmd_context *cmd, struct device *dev,
	       uint64_t label_sector)
{
	struct label *label;
	struct lvmcache_info *info;

	if (!(label = lvmcache_get_dev_label(dev))) {
		log_error("Could not find LVM label on %s", dev_name(dev));
		return 0;
	}

	log_print("Found label on %s, sector %"PRIu64", type=%.8s",
		  dev_name(dev), label->sector, label->type);

	/*
	 * Next, loop through metadata areas
	 */
	info = label->info;
	lvmcache_foreach_mda(info, _analyze_mda, (void *)lvmcache_fmt(info));

	return 1;
}

/* FIXME: remove / combine this with locking? */
int vg_check_write_mode(struct volume_group *vg)
{
	if (vg->open_mode != 'w') {
		log_errno(EPERM, "Attempt to modify a read-only VG");
		return 0;
	}
	return 1;
}

/*
 * Return 1 if the VG metadata should be written
 * *without* the LVM_WRITE flag in the status line, and
 * *with* the LVM_WRITE_LOCKED flag in the flags line.
 *
 * If this is done for a VG, it forces previous versions
 * of lvm (before the LVM_WRITE_LOCKED flag was added), to view
 * the VG and its LVs as read-only (because the LVM_WRITE flag
 * is missing).  Versions of lvm that understand the
 * LVM_WRITE_LOCKED flag know to check the other methods of
 * access control for the VG, specifically system_id and lock_type.
 *
 * So, if a VG has a system_id or lock_type, then the
 * system_id and lock_type control access to the VG in
 * addition to its basic writable status.  Because previous
 * lvm versions do not know about system_id or lock_type,
 * VGs depending on either of these should have LVM_WRITE_LOCKED
 * instead of LVM_WRITE to prevent the previous lvm versions from
 * assuming they can write the VG and its LVs.
 */
int vg_flag_write_locked(struct volume_group *vg)
{
	if (vg->system_id && vg->system_id[0])
		return 1;

	if (vg->lock_type && vg->lock_type[0] && strcmp(vg->lock_type, "none"))
		return 1;

	return 0;
}

static int _access_vg_clustered(struct cmd_context *cmd, const struct volume_group *vg)
{
	if (vg_is_clustered(vg)) {
		/*
		 * force_access_clustered is only set when forcibly
		 * converting a clustered vg to lock type none.
		 */
		if (cmd->force_access_clustered) {
			log_debug("Allowing forced access to clustered vg %s", vg->name);
			return 1;
		}

		log_verbose("Skipping clustered VG %s.", vg->name);
		return 0;
	}

	return 1;
}

/*
 * Performs a set of checks against a VG according to bits set in status
 * and returns FAILED_* bits for those that aren't acceptable.
 *
 * FIXME Remove the unnecessary duplicate definitions and return bits directly.
 */
uint32_t vg_bad_status_bits(const struct volume_group *vg, uint64_t status)
{
	uint32_t failure = 0;

	if ((status & CLUSTERED) && !_access_vg_clustered(vg->cmd, vg))
		/* Return because other flags are considered undefined. */
		return FAILED_CLUSTERED;

	if ((status & EXPORTED_VG) &&
	    vg_is_exported(vg)) {
		log_error("Volume group %s is exported", vg->name);
		failure |= FAILED_EXPORTED;
	}

	if ((status & LVM_WRITE) &&
	    !(vg->status & LVM_WRITE)) {
		log_error("Volume group %s is read-only", vg->name);
		failure |= FAILED_READ_ONLY;
	}

	if ((status & RESIZEABLE_VG) &&
	    !vg_is_resizeable(vg)) {
		log_error("Volume group %s is not resizeable.", vg->name);
		failure |= FAILED_RESIZEABLE;
	}

	return failure;
}

/**
 * vg_check_status - check volume group status flags and log error
 * @vg - volume group to check status flags
 * @status - specific status flags to check (e.g. EXPORTED_VG)
 */
int vg_check_status(const struct volume_group *vg, uint64_t status)
{
	return !vg_bad_status_bits(vg, status);
}

/*
 * VG is left unlocked on failure
 */
static struct volume_group *_recover_vg(struct cmd_context *cmd,
			 const char *vg_name, const char *vgid,
			 int is_shared, uint32_t lockd_state)
{
	int mdas_consistent = 0;
	struct volume_group *vg;
	uint32_t state = 0;

	unlock_vg(cmd, NULL, vg_name);

	if (!lock_vol(cmd, vg_name, LCK_VG_WRITE, NULL))
		return_NULL;

	/*
	 * Convert vg lock in lvmlockd from sh to ex.
	 */
	if (is_shared && !(lockd_state & LDST_FAIL) && !(lockd_state & LDST_EX)) {
		log_debug("Upgrade lvmlockd lock to repair vg %s.", vg_name);
		if (!lockd_vg(cmd, vg_name, "ex", 0, &state)) {
			log_warn("Skip repair for shared VG without exclusive lock.");
			return NULL;
		}
		lockd_state |= LDST_EX;
	}

	if (!(vg = vg_read_internal(cmd, vg_name, vgid, lockd_state, 0, 1, &mdas_consistent))) {
		unlock_vg(cmd, NULL, vg_name);
		return_NULL;
	}

	if (!mdas_consistent) {
		release_vg(vg);
		unlock_vg(cmd, NULL, vg_name);
		return_NULL;
	}

	return (struct volume_group *)vg;
}

static int _allow_extra_system_id(struct cmd_context *cmd, const char *system_id)
{
	const struct dm_config_node *cn;
	const struct dm_config_value *cv;
	const char *str;

	if (!(cn = find_config_tree_array(cmd, local_extra_system_ids_CFG, NULL)))
		return 0;

	for (cv = cn->v; cv; cv = cv->next) {
		if (cv->type == DM_CFG_EMPTY_ARRAY)
			break;
		/* Ignore invalid data: Warning message already issued by config.c */
		if (cv->type != DM_CFG_STRING)
			continue;
		str = cv->v.str;
		if (!*str)
			continue;

		if (!strcmp(str, system_id))
			return 1;
	}

	return 0;
}

static int _access_vg_lock_type(struct cmd_context *cmd, struct volume_group *vg,
				uint32_t lockd_state, uint32_t *failure)
{
	if (!is_real_vg(vg->name))
		return 1;

	if (cmd->lockd_vg_disable)
		return 1;

	/*
	 * Local VG requires no lock from lvmlockd.
	 */
	if (!vg_is_shared(vg))
		return 1;

	/*
	 * When lvmlockd is not used, lockd VGs are ignored by lvm
	 * and cannot be used, with two exceptions:
	 *
	 * . The --shared option allows them to be revealed with
	 *   reporting/display commands.
	 *
	 * . If a command asks to operate on one specifically
	 *   by name, then an error is printed.
	 */
	if (!lvmlockd_use()) {
		/*
	 	 * Some reporting/display commands have the --shared option
		 * (like --foreign) to allow them to reveal lockd VGs that
		 * are otherwise ignored.  The --shared option must only be
		 * permitted in commands that read the VG for report or display,
		 * not any that write the VG or activate LVs.
	 	 */
		if (cmd->include_shared_vgs)
			return 1;

		/*
		 * Some commands want the error printed by vg_read, others by ignore_vg.
		 * Those using ignore_vg may choose to skip the error.
		 */
		if (cmd->vg_read_print_access_error) {
			log_error("Cannot access VG %s with lock type %s that requires lvmlockd.",
				  vg->name, vg->lock_type);
		}

		*failure |= FAILED_LOCK_TYPE;
		return 0;
	}

	/*
	 * The lock request from lvmlockd failed.  If the lock was ex,
	 * we cannot continue.  If the lock was sh, we could also fail
	 * to continue but since the lock was sh, it means the VG is
	 * only being read, and it doesn't hurt to allow reading with
	 * no lock.
	 */
	if (lockd_state & LDST_FAIL) {
		if ((lockd_state & LDST_EX) || cmd->lockd_vg_enforce_sh) {
			log_error("Cannot access VG %s due to failed lock.", vg->name);
			*failure |= FAILED_LOCK_MODE;
			return 0;
		}

		log_warn("Reading VG %s without a lock.", vg->name);
		return 1;
	}

	if (test_mode()) {
		log_error("Test mode is not yet supported with lock type %s.", vg->lock_type);
		return 0;
	}

	return 1;
}

int is_system_id_allowed(struct cmd_context *cmd, const char *system_id)
{
	/*
	 * A VG without a system_id can be accessed by anyone.
	 */
	if (!system_id || !system_id[0])
		return 1;

	/*
	 * Allowed if the host and VG system_id's match.
	 */
	if (cmd->system_id && !strcmp(cmd->system_id, system_id))
		return 1;

	/*
	 * Allowed if a host's extra system_id matches.
	 */
	if (cmd->system_id && _allow_extra_system_id(cmd, system_id))
		return 1;

	/*
	 * Not allowed if the host does not have a system_id
	 * and the VG does, or if the host and VG's system_id's
	 * do not match.
	 */

	return 0;
}

static int _access_vg_systemid(struct cmd_context *cmd, struct volume_group *vg)
{
	/*
	 * A few commands allow read-only access to foreign VGs.
	 */
	if (cmd->include_foreign_vgs)
		return 1;

	if (is_system_id_allowed(cmd, vg->system_id))
		return 1;

	/*
	 * Allow VG access if the local host has active LVs in it.
	 */
	if (lvs_in_vg_activated(vg)) {
		log_warn("WARNING: Found LVs active in VG %s with foreign system ID %s.  Possible data corruption.",
			  vg->name, vg->system_id);
		if (cmd->include_active_foreign_vgs)
			return 1;
		return 0;
	}

	/*
	 * Print an error when reading a VG that has a system_id
	 * and the host system_id is unknown.
	 */
	if (!cmd->system_id || cmd->unknown_system_id) {
		log_error("Cannot access VG %s with system ID %s with unknown local system ID.",
			  vg->name, vg->system_id);
		return 0;
	}

	/*
	 * Some commands want the error printed by vg_read, others by ignore_vg.
	 * Those using ignore_vg may choose to skip the error.
	 */
	if (cmd->vg_read_print_access_error) {
		log_error("Cannot access VG %s with system ID %s with local system ID %s.",
			  vg->name, vg->system_id, cmd->system_id);
		return 0;
	}

	/* Silently ignore foreign vgs. */

	return 0;
}

/*
 * FIXME: move vg_bad_status_bits() checks in here.
 */
static int _vg_access_permitted(struct cmd_context *cmd, struct volume_group *vg,
				uint32_t lockd_state, uint32_t *failure)
{
	if (!is_real_vg(vg->name)) {
		return 1;
	}

	if (!_access_vg_clustered(cmd, vg)) {
		*failure |= FAILED_CLUSTERED;
		return 0;
	}

	if (!_access_vg_lock_type(cmd, vg, lockd_state, failure)) {
		/* Either FAILED_LOCK_TYPE or FAILED_LOCK_MODE were set. */
		return 0;
	}

	if (!_access_vg_systemid(cmd, vg)) {
		*failure |= FAILED_SYSTEMID;
		return 0;
	}

	return 1;
}

/*
 * Consolidated locking, reading, and status flag checking.
 *
 * If the metadata is inconsistent, setting READ_ALLOW_INCONSISTENT in
 * read_flags will return it with FAILED_INCONSISTENT set instead of 
 * giving you nothing.
 *
 * Use vg_read_error(vg) to determine the result.  Nonzero means there were
 * problems reading the volume group.
 * Zero value means that the VG is open and appropriate locks are held.
 */
static struct volume_group *_vg_lock_and_read(struct cmd_context *cmd, const char *vg_name,
			       const char *vgid,
			       uint32_t lock_flags,
			       uint64_t status_flags,
			       uint32_t read_flags,
			       uint32_t lockd_state)
{
	struct volume_group *vg = NULL;
	uint32_t failure = 0;
	uint32_t warn_flags = 0;
	int mdas_consistent = 1;
	int enable_repair = 1;
	int is_shared = 0;
	int skip_lock = is_orphan_vg(vg_name) && (read_flags & PROCESS_SKIP_ORPHAN_LOCK);

	if ((read_flags & READ_ALLOW_INCONSISTENT) || (lock_flags != LCK_VG_WRITE)) {
		enable_repair = 0;
		warn_flags |= WARN_INCONSISTENT;
	}

	if (!validate_name(vg_name) && !is_orphan_vg(vg_name)) {
		log_error("Volume group name \"%s\" has invalid characters.",
			  vg_name);
		return NULL;
	}

	if (!skip_lock &&
	    !lock_vol(cmd, vg_name, lock_flags, NULL)) {
		log_error("Can't get lock for %s", vg_name);
		return _vg_make_handle(cmd, vg, FAILED_LOCKING);
	}

	if (skip_lock)
		log_very_verbose("Locking %s already done", vg_name);

	if (is_orphan_vg(vg_name))
		status_flags &= ~LVM_WRITE;

	if (!(vg = vg_read_internal(cmd, vg_name, vgid, lockd_state, warn_flags, enable_repair, &mdas_consistent))) {
		if (!(read_flags & READ_OK_NOTFOUND))
			log_error("Volume group \"%s\" not found", vg_name);
		failure |= FAILED_NOTFOUND;
		goto bad;
	}

	if (!_vg_access_permitted(cmd, vg, lockd_state, &failure))
		goto bad;

	/*
	 * If we called vg_read_internal above without repair enabled,
	 * and the read found inconsistent mdas, then then get a write/ex
	 * lock and call it again with repair enabled so it will fix
	 * the inconsistent mdas.
	 *
	 * FIXME: factor vg repair out of vg_read.  The vg_read caller
	 * should get an error about the vg have problems and then call
	 * a repair-specific function if it wants to.  (NB there are
	 * other kinds of repairs hidden in _vg_read that should be
	 * pulled out in addition to _recover_vg).
	 */
	if (!mdas_consistent && !enable_repair) {
		is_shared = vg_is_shared(vg);
		release_vg(vg);

		if (!(vg = _recover_vg(cmd, vg_name, vgid, is_shared, lockd_state))) {
			if (is_orphan_vg(vg_name))
				log_error("Recovery of standalone physical volumes failed.");
			else
				log_error("Recovery of volume group \"%s\" failed.", vg_name);
			failure |= FAILED_RECOVERY;
			goto bad_no_unlock;
		}
	}

	/*
	 * Check that the tool can handle tricky cases -- missing PVs and
	 * unknown segment types.
	 */

	if (!cmd->handles_missing_pvs && vg_missing_pv_count(vg) &&
	    lock_flags == LCK_VG_WRITE) {
		log_error("Cannot change VG %s while PVs are missing.", vg->name);
		log_error("Consider vgreduce --removemissing.");
		failure |= FAILED_INCONSISTENT; /* FIXME new failure code here? */
		goto bad;
	}

	if (!cmd->handles_unknown_segments && vg_has_unknown_segments(vg) &&
	    lock_flags == LCK_VG_WRITE) {
		log_error("Cannot change VG %s with unknown segments in it!",
			  vg->name);
		failure |= FAILED_INCONSISTENT; /* FIXME new failure code here? */
		goto bad;
	}

	failure |= vg_bad_status_bits(vg, status_flags);
	if (failure)
		goto_bad;

	if (!(vg = _vg_make_handle(cmd, vg, failure)) || vg_read_error(vg))
		if (!skip_lock)
			unlock_vg(cmd, vg, vg_name);

	return vg;

bad:
	if (!skip_lock)
		unlock_vg(cmd, vg, vg_name);

bad_no_unlock:
	return _vg_make_handle(cmd, vg, failure);
}

/*
 * vg_read: High-level volume group metadata read function.
 *
 * vg_read_error() must be used on any handle returned to check for errors.
 *
 *  - metadata inconsistent and automatic correction failed: FAILED_INCONSISTENT
 *  - VG is read-only: FAILED_READ_ONLY
 *  - VG is EXPORTED, unless flags has READ_ALLOW_EXPORTED: FAILED_EXPORTED
 *  - VG is not RESIZEABLE: FAILED_RESIZEABLE
 *  - locking failed: FAILED_LOCKING
 *
 * On failures, all locks are released, unless one of the following applies:
 *  - vgname_is_locked(lock_name) is true
 * FIXME: remove the above 2 conditions if possible and make an error always
 * release the lock.
 *
 * Volume groups are opened read-only unless flags contains READ_FOR_UPDATE.
 *
 * Checking for VG existence:
 *
 * FIXME: We want vg_read to attempt automatic recovery after acquiring a
 * temporary write lock: if that fails, we bail out as usual, with failed &
 * FAILED_INCONSISTENT. If it works, we are good to go. Code that's been in
 * toollib just set lock_flags to LCK_VG_WRITE and called vg_read_internal with
 * *consistent = 1.
 */
struct volume_group *vg_read(struct cmd_context *cmd, const char *vg_name,
			     const char *vgid, uint32_t read_flags, uint32_t lockd_state)
{
	uint64_t status_flags = UINT64_C(0);
	uint32_t lock_flags = LCK_VG_READ;

	if (read_flags & READ_FOR_UPDATE) {
		status_flags |= EXPORTED_VG | LVM_WRITE;
		lock_flags = LCK_VG_WRITE;
	}

	if (read_flags & READ_ALLOW_EXPORTED)
		status_flags &= ~EXPORTED_VG;

	return _vg_lock_and_read(cmd, vg_name, vgid, lock_flags, status_flags, read_flags, lockd_state);
}

/*
 * A high-level volume group metadata reading function. Open a volume group for
 * later update (this means the user code can change the metadata and later
 * request the new metadata to be written and committed).
 */
struct volume_group *vg_read_for_update(struct cmd_context *cmd, const char *vg_name,
			 const char *vgid, uint32_t read_flags, uint32_t lockd_state)
{
	struct volume_group *vg = vg_read(cmd, vg_name, vgid, read_flags | READ_FOR_UPDATE, lockd_state);

	if (!vg || vg_read_error(vg))
		stack;

	return vg;
}

/*
 * Test the validity of a VG handle returned by vg_read() or vg_read_for_update().
 */
uint32_t vg_read_error(struct volume_group *vg_handle)
{
	if (!vg_handle)
		return FAILED_ALLOCATION;

	return vg_handle->read_status;
}

/*
 * Lock a vgname and/or check for existence.
 * Takes a WRITE lock on the vgname before scanning.
 * If scanning fails or vgname found, release the lock.
 * NOTE: If you find the return codes confusing, you might think of this
 * function as similar to an open() call with O_CREAT and O_EXCL flags
 * (open returns fail with -EEXIST if file already exists).
 *
 * Returns:
 * FAILED_LOCKING - Cannot lock name
 * FAILED_EXIST - VG name already exists - cannot reserve
 * SUCCESS - VG name does not exist in system and WRITE lock held
 */
uint32_t vg_lock_newname(struct cmd_context *cmd, const char *vgname)
{
	if (!lock_vol(cmd, vgname, LCK_VG_WRITE, NULL))
		return FAILED_LOCKING;

	/* Find the vgname in the cache */
	/* If it's not there we must do full scan to be completely sure */
	if (!lvmcache_fmt_from_vgname(cmd, vgname, NULL, 1)) {
		lvmcache_label_scan(cmd);
		if (!lvmcache_fmt_from_vgname(cmd, vgname, NULL, 1)) {
			lvmcache_label_scan(cmd);
			if (!lvmcache_fmt_from_vgname(cmd, vgname, NULL, 0))
				return SUCCESS; /* vgname not found after scanning */
		}
	}

	/* Found vgname so cannot reserve. */
	unlock_vg(cmd, NULL, vgname);
	return FAILED_EXIST;
}

struct format_instance *alloc_fid(const struct format_type *fmt,
				  const struct format_instance_ctx *fic)
{
	struct dm_pool *mem;
	struct format_instance *fid;

	if (!(mem = dm_pool_create("format_instance", 1024)))
		return_NULL;

	if (!(fid = dm_pool_zalloc(mem, sizeof(*fid)))) {
		log_error("Couldn't allocate format_instance object.");
		goto bad;
	}

	fid->ref_count = 1;
	fid->mem = mem;
	fid->type = fic->type;
	fid->fmt = fmt;

	dm_list_init(&fid->metadata_areas_in_use);
	dm_list_init(&fid->metadata_areas_ignored);

	return fid;

bad:
	dm_pool_destroy(mem);
	return NULL;
}

void pv_set_fid(struct physical_volume *pv,
		struct format_instance *fid)
{
	if (fid == pv->fid)
		return;

	if (fid)
		fid->ref_count++;

	if (pv->fid)
		pv->fid->fmt->ops->destroy_instance(pv->fid);

	pv->fid = fid;
}

void vg_set_fid(struct volume_group *vg,
		 struct format_instance *fid)
{
	struct pv_list *pvl;

	if (fid == vg->fid)
		return;

	if (fid)
		fid->ref_count++;

	dm_list_iterate_items(pvl, &vg->pvs)
		pv_set_fid(pvl->pv, fid);

	dm_list_iterate_items(pvl, &vg->removed_pvs)
		pv_set_fid(pvl->pv, fid);

	if (vg->fid)
		vg->fid->fmt->ops->destroy_instance(vg->fid);

	vg->fid = fid;
}

static int _convert_key_to_string(const char *key, size_t key_len,
				  unsigned sub_key, char *buf, size_t buf_len)
{
	memcpy(buf, key, key_len);
	buf += key_len;
	buf_len -= key_len;
	if ((dm_snprintf(buf, buf_len, "_%u", sub_key) == -1))
		return_0;

	return 1;
}

int fid_add_mda(struct format_instance *fid, struct metadata_area *mda,
		 const char *key, size_t key_len, const unsigned sub_key)
{
	static char full_key[PATH_MAX];

	dm_list_add(mda_is_ignored(mda) ? &fid->metadata_areas_ignored :
		                          &fid->metadata_areas_in_use, &mda->list);

	/* Return if the mda is not supposed to be indexed. */
	if (!key)
		return 1;

	if (!fid->metadata_areas_index)
		return_0;

	/* Add metadata area to index. */
	if (!_convert_key_to_string(key, key_len, sub_key,
				    full_key, sizeof(full_key)))
		return_0;

	if (!dm_hash_insert(fid->metadata_areas_index,
			    full_key, mda)) {
		log_error("Failed to hash mda.");
		return 0;
	}

	return 1;
}

int fid_add_mdas(struct format_instance *fid, struct dm_list *mdas,
		 const char *key, size_t key_len)
{
	struct metadata_area *mda, *mda_new;
	unsigned mda_index = 0;

	dm_list_iterate_items(mda, mdas) {
		mda_new = mda_copy(fid->mem, mda);
		if (!mda_new)
			return_0;
		fid_remove_mda(fid, NULL, key, key_len, mda_index);
		fid_add_mda(fid, mda_new, key, key_len, mda_index);
		mda_index++;
	}

	return 1;
}

struct metadata_area *fid_get_mda_indexed(struct format_instance *fid,
					  const char *key, size_t key_len,
					  const unsigned sub_key)
{
	static char full_key[PATH_MAX];
	struct metadata_area *mda = NULL;

	if (!fid->metadata_areas_index)
		return_NULL;

	if (!_convert_key_to_string(key, key_len, sub_key,
				    full_key, sizeof(full_key)))
		return_NULL;

	mda = (struct metadata_area *) dm_hash_lookup(fid->metadata_areas_index,
						      full_key);

	return mda;
}

int fid_remove_mda(struct format_instance *fid, struct metadata_area *mda,
		   const char *key, size_t key_len, const unsigned sub_key)
{
	static char full_key[PATH_MAX];
	struct metadata_area *mda_indexed = NULL;

	/* At least one of mda or key must be specified. */
	if (!mda && !key)
		return 1;

	if (key) {
		/*
		 * If both mda and key specified, check given mda
		 * with what we find using the index and return
		 * immediately if these two do not match.
		 */
		if (!(mda_indexed = fid_get_mda_indexed(fid, key, key_len, sub_key)) ||
		     (mda && mda != mda_indexed))
			return 1;

		mda = mda_indexed;

		if (!_convert_key_to_string(key, key_len, sub_key,
				    full_key, sizeof(full_key)))
			return_0;

		dm_hash_remove(fid->metadata_areas_index, full_key);
	}

	dm_list_del(&mda->list);

	return 1;
}

/*
 * Copy constructor for a metadata_area.
 */
struct metadata_area *mda_copy(struct dm_pool *mem,
			       struct metadata_area *mda)
{
	struct metadata_area *mda_new;

	if (!(mda_new = dm_pool_alloc(mem, sizeof(*mda_new)))) {
		log_error("metadata_area allocation failed");
		return NULL;
	}
	memcpy(mda_new, mda, sizeof(*mda));
	if (mda->ops->mda_metadata_locn_copy && mda->metadata_locn) {
		mda_new->metadata_locn =
			mda->ops->mda_metadata_locn_copy(mem, mda->metadata_locn);
		if (!mda_new->metadata_locn) {
			dm_pool_free(mem, mda_new);
			return NULL;
		}
	}

	dm_list_init(&mda_new->list);

	return mda_new;
}
/*
 * This function provides a way to answer the question on a format specific
 * basis - does the format specfic context of these two metadata areas
 * match?
 *
 * A metatdata_area is defined to be independent of the underlying context.
 * This has the benefit that we can use the same abstraction to read disks
 * (see _metadata_text_raw_ops) or files (see _metadata_text_file_ops).
 * However, one downside is there is no format-independent way to determine
 * whether a given metadata_area is attached to a specific device - in fact,
 * it may not be attached to a device at all.
 *
 * Thus, LVM is structured such that an mda is not a member of struct
 * physical_volume.  The location of the mda depends on whether
 * the PV is in a volume group.  A PV not in a VG has an mda on the
 * 'info->mda' list in lvmcache, while a PV in a VG has an mda on
 * the vg->fid->metadata_areas_in_use list.  For further details, see _vg_read(),
 * and the sequence of creating the format_instance with fid->metadata_areas_in_use
 * list, as well as the construction of the VG, with list of PVs (comes
 * after the construction of the fid and list of mdas).
 */
unsigned mda_locns_match(struct metadata_area *mda1, struct metadata_area *mda2)
{
	if (!mda1->ops->mda_locns_match || !mda2->ops->mda_locns_match ||
	    mda1->ops->mda_locns_match != mda2->ops->mda_locns_match)
		return 0;

	return mda1->ops->mda_locns_match(mda1, mda2);
}

struct device *mda_get_device(struct metadata_area *mda)
{
	if (!mda->ops->mda_get_device)
		return NULL;
	return mda->ops->mda_get_device(mda);
}

unsigned mda_is_ignored(struct metadata_area *mda)
{
	return (mda->status & MDA_IGNORED);
}

void mda_set_ignored(struct metadata_area *mda, unsigned mda_ignored)
{
	void *locn = mda->metadata_locn;
	unsigned old_mda_ignored = mda_is_ignored(mda);

	if (mda_ignored && !old_mda_ignored)
		mda->status |= MDA_IGNORED;
	else if (!mda_ignored && old_mda_ignored)
		mda->status &= ~MDA_IGNORED;
	else
		return;	/* No change */

	log_debug_metadata("%s ignored flag for mda %s at offset %" PRIu64 ".",
			   mda_ignored ? "Setting" : "Clearing",
			   mda->ops->mda_metadata_locn_name ? mda->ops->mda_metadata_locn_name(locn) : "",
			   mda->ops->mda_metadata_locn_offset ? mda->ops->mda_metadata_locn_offset(locn) : UINT64_C(0));
}

int mdas_empty_or_ignored(struct dm_list *mdas)
{
	struct metadata_area *mda;

	if (dm_list_empty(mdas))
		return 1;
	dm_list_iterate_items(mda, mdas) {
		if (mda_is_ignored(mda))
			return 1;
	}
	return 0;
}

int pv_change_metadataignore(struct physical_volume *pv, uint32_t mda_ignored)
{
	const char *pv_name = pv_dev_name(pv);

	if (mda_ignored && !pv_mda_used_count(pv)) {
		log_error("Metadata areas on physical volume \"%s\" already "
			  "ignored.", pv_name);
		return 0;
	}

	if (!mda_ignored && (pv_mda_used_count(pv) == pv_mda_count(pv))) {
		log_error("Metadata areas on physical volume \"%s\" already "
			  "marked as in-use.", pv_name);
		return 0;
	}

	if (!pv_mda_count(pv)) {
		log_error("Physical volume \"%s\" has no metadata "
			  "areas.", pv_name);
		return 0;
	}

	log_verbose("Marking metadata areas on physical volume \"%s\" "
		    "as %s.", pv_name, mda_ignored ? "ignored" : "in-use");

	if (!pv_mda_set_ignored(pv, mda_ignored))
		return_0;

	/*
	 * Update vg_mda_copies based on the mdas in this PV.
	 * This is most likely what the user would expect - if they
	 * specify a specific PV to be ignored/un-ignored, they will
	 * most likely not want LVM to turn around and change the
	 * ignore / un-ignore value when it writes the VG to disk.
	 * This does not guarantee this PV's ignore bits will be
	 * preserved in future operations.
	 */
	if (!is_orphan(pv) &&
	    vg_mda_copies(pv->vg) != VGMETADATACOPIES_UNMANAGED) {
		log_warn("WARNING: Changing preferred number of copies of VG %s "
			 "metadata from %"PRIu32" to %"PRIu32, pv_vg_name(pv),
			 vg_mda_copies(pv->vg), vg_mda_used_count(pv->vg));
		vg_set_mda_copies(pv->vg, vg_mda_used_count(pv->vg));
	}

	return 1;
}

char *tags_format_and_copy(struct dm_pool *mem, const struct dm_list *tagsl)
{
	struct dm_str_list *sl;

	if (!dm_pool_begin_object(mem, 256)) {
		log_error("dm_pool_begin_object failed");
		return NULL;
	}

	dm_list_iterate_items(sl, tagsl) {
		if (!dm_pool_grow_object(mem, sl->str, strlen(sl->str)) ||
		    (sl->list.n != tagsl && !dm_pool_grow_object(mem, ",", 1))) {
			log_error("dm_pool_grow_object failed");
			return NULL;
		}
	}

	if (!dm_pool_grow_object(mem, "\0", 1)) {
		log_error("dm_pool_grow_object failed");
		return NULL;
	}
	return dm_pool_end_object(mem);
}

const struct logical_volume *lv_committed(const struct logical_volume *lv)
{
	struct volume_group *vg;
	struct logical_volume *found_lv;

	if (!lv)
		return NULL;

	if (!lv->vg->vg_committed)
		return lv;

	vg = lv->vg->vg_committed;

	if (!(found_lv = find_lv_in_vg_by_lvid(vg, &lv->lvid))) {
		log_error(INTERNAL_ERROR "LV %s (UUID %s) not found in committed metadata.",
			  display_lvname(lv), lv->lvid.s);
		return NULL;
	}

	return found_lv;
}

/*
 * Check if a lock_type uses lvmlockd.
 * If not (none, clvm), return 0.
 * If so (dlm, sanlock), return 1.
 */

int is_lockd_type(const char *lock_type)
{
	if (!lock_type)
		return 0;
	if (!strcmp(lock_type, "dlm"))
		return 1;
	if (!strcmp(lock_type, "sanlock"))
		return 1;
	return 0;
}

int vg_is_shared(const struct volume_group *vg)
{
	return (vg->lock_type && is_lockd_type(vg->lock_type));
}

int vg_strip_outdated_historical_lvs(struct volume_group *vg) {
	struct glv_list *glvl, *tglvl;
	time_t current_time = time(NULL);
	uint64_t threshold = find_config_tree_int(vg->cmd, metadata_lvs_history_retention_time_CFG, NULL);

	if (!threshold)
		return 1;

	dm_list_iterate_items_safe(glvl, tglvl, &vg->historical_lvs) {
		/*
		 * Removal time in the future? Not likely,
		 * but skip this item in any case.
		*/
		if (current_time < (time_t) glvl->glv->historical->timestamp_removed)
			continue;

		if ((current_time - glvl->glv->historical->timestamp_removed) > threshold) {
			if (!historical_glv_remove(glvl->glv)) {
				log_error("Failed to destroy record about historical LV %s/%s.",
					  vg->name, glvl->glv->historical->name);
				return 0;
			}
			log_verbose("Outdated record for historical logical volume \"%s\" "
				    "automatically destroyed.", glvl->glv->historical->name);
		}
	}

	return 1;
}
