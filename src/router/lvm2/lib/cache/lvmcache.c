/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
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

#include "base/memory/zalloc.h"
#include "lib/misc/lib.h"
#include "lib/cache/lvmcache.h"
#include "lib/commands/toolcontext.h"
#include "lib/device/dev-cache.h"
#include "lib/locking/locking.h"
#include "lib/metadata/metadata.h"
#include "lib/mm/memlock.h"
#include "lib/datastruct/str_list.h"
#include "lib/format_text/format-text.h"
#include "lib/config/config.h"

/* One per device */
struct lvmcache_info {
	struct dm_list list;	/* Join VG members together */
	struct dm_list mdas;	/* list head for metadata areas */
	struct dm_list das;	/* list head for data areas */
	struct dm_list bas;	/* list head for bootloader areas */
	struct lvmcache_vginfo *vginfo;	/* NULL == unknown */
	struct label *label;
	const struct format_type *fmt;
	struct device *dev;
	uint64_t device_size;	/* Bytes */
	uint32_t ext_version;   /* Extension version */
	uint32_t ext_flags;	/* Extension flags */
	uint32_t status;
};

/* One per VG */
struct lvmcache_vginfo {
	struct dm_list list;	/* Join these vginfos together */
	struct dm_list infos;	/* List head for lvmcache_infos */
	const struct format_type *fmt;
	char *vgname;		/* "" == orphan */
	uint32_t status;
	char vgid[ID_LEN + 1];
	char _padding[7];
	struct lvmcache_vginfo *next; /* Another VG with same name? */
	char *creation_host;
	char *system_id;
	char *lock_type;
	uint32_t mda_checksum;
	size_t mda_size;
	int seqno;
	int scan_summary_mismatch; /* vgsummary from devs had mismatching seqno or checksum */
};

static struct dm_hash_table *_pvid_hash = NULL;
static struct dm_hash_table *_vgid_hash = NULL;
static struct dm_hash_table *_vgname_hash = NULL;
static DM_LIST_INIT(_vginfos);
static DM_LIST_INIT(_found_duplicate_devs);
static DM_LIST_INIT(_unused_duplicate_devs);
static int _scanning_in_progress = 0;
static int _vgs_locked = 0;
static int _found_duplicate_pvs = 0;	/* If we never see a duplicate PV we can skip checking for them later. */

int lvmcache_init(struct cmd_context *cmd)
{
	/*
	 * FIXME add a proper lvmcache_locking_reset() that
	 * resets the cache so no previous locks are locked
	 */
	_vgs_locked = 0;

	dm_list_init(&_vginfos);
	dm_list_init(&_found_duplicate_devs);
	dm_list_init(&_unused_duplicate_devs);

	if (!(_vgname_hash = dm_hash_create(128)))
		return 0;

	if (!(_vgid_hash = dm_hash_create(128)))
		return 0;

	if (!(_pvid_hash = dm_hash_create(128)))
		return 0;

	return 1;
}

void lvmcache_lock_vgname(const char *vgname, int read_only __attribute__((unused)))
{
	if (strcmp(vgname, VG_GLOBAL))
		_vgs_locked++;
}

void lvmcache_unlock_vgname(const char *vgname)
{
	/* FIXME Do this per-VG */
	if (strcmp(vgname, VG_GLOBAL) && !--_vgs_locked) {
		dev_size_seqno_inc(); /* invalidate all cached dev sizes */
	}
}

int lvmcache_vgs_locked(void)
{
	return _vgs_locked;
}

/*
 * When lvmcache sees a duplicate PV, this is set.
 * process_each_pv() can avoid searching for duplicates
 * by checking this and seeing that no duplicate PVs exist.
 *
 *
 * found_duplicate_pvs tells the process_each_pv code
 * to search the devices list for duplicates, so that
 * devices can be processed together with their
 * duplicates (while processing the VG, rather than
 * reporting pv->dev under the VG, and its duplicate
 * outside the VG context.)
 */
int lvmcache_found_duplicate_pvs(void)
{
	return _found_duplicate_pvs;
}

int lvmcache_get_unused_duplicate_devs(struct cmd_context *cmd, struct dm_list *head)
{
	struct device_list *devl, *devl2;

	dm_list_iterate_items(devl, &_unused_duplicate_devs) {
		if (!(devl2 = dm_pool_alloc(cmd->mem, sizeof(*devl2)))) {
			log_error("device_list element allocation failed");
			return 0;
		}
		devl2->dev = devl->dev;
		dm_list_add(head, &devl2->list);
	}
	return 1;
}

void lvmcache_remove_unchosen_duplicate(struct device *dev)
{
	struct device_list *devl;

	dm_list_iterate_items(devl, &_unused_duplicate_devs) {
		if (devl->dev == dev) {
			dm_list_del(&devl->list);
			return;
		}
	}
}

static void _destroy_duplicate_device_list(struct dm_list *head)
{
	struct device_list *devl, *devl2;

	dm_list_iterate_items_safe(devl, devl2, head) {
		dm_list_del(&devl->list);
		free(devl);
	}
	dm_list_init(head);
}

static void _vginfo_attach_info(struct lvmcache_vginfo *vginfo,
				struct lvmcache_info *info)
{
	if (!vginfo)
		return;

	info->vginfo = vginfo;
	dm_list_add(&vginfo->infos, &info->list);
}

static void _vginfo_detach_info(struct lvmcache_info *info)
{
	if (!dm_list_empty(&info->list)) {
		dm_list_del(&info->list);
		dm_list_init(&info->list);
	}

	info->vginfo = NULL;
}

/* If vgid supplied, require a match. */
struct lvmcache_vginfo *lvmcache_vginfo_from_vgname(const char *vgname, const char *vgid)
{
	struct lvmcache_vginfo *vginfo;

	if (!vgname)
		return lvmcache_vginfo_from_vgid(vgid);

	if (!_vgname_hash) {
		log_debug_cache(INTERNAL_ERROR "Internal lvmcache is no yet initialized.");
		return NULL;
	}

	if (!(vginfo = dm_hash_lookup(_vgname_hash, vgname))) {
		log_debug_cache("lvmcache has no info for vgname \"%s\"%s" FMTVGID ".",
				vgname, (vgid) ? " with VGID " : "", (vgid) ? : "");
		return NULL;
	}

	if (vgid)
		do
			if (!strncmp(vgid, vginfo->vgid, ID_LEN))
				return vginfo;
		while ((vginfo = vginfo->next));

	if  (!vginfo)
		log_debug_cache("lvmcache has not found vgname \"%s\"%s" FMTVGID ".",
				vgname, (vgid) ? " with VGID " : "", (vgid) ? : "");

	return vginfo;
}

const struct format_type *lvmcache_fmt_from_vgname(struct cmd_context *cmd,
						   const char *vgname, const char *vgid,
						   unsigned revalidate_labels)
{
	struct lvmcache_vginfo *vginfo;
	struct lvmcache_info *info;
	struct dm_list *devh, *tmp;
	struct dm_list devs;
	struct device_list *devl;
	char vgid_found[ID_LEN + 1] __attribute__((aligned(8)));

	if (!(vginfo = lvmcache_vginfo_from_vgname(vgname, vgid))) {
		stack;
		return NULL;
	}

	/*
	 * If this function is called repeatedly, only the first one needs to revalidate.
	 */
	if (!revalidate_labels)
		goto out;

	/*
	 * This function is normally called before reading metadata so
 	 * we check cached labels here. Unfortunately vginfo is volatile.
 	 */
	dm_list_init(&devs);
	dm_list_iterate_items(info, &vginfo->infos) {
		if (!(devl = malloc(sizeof(*devl)))) {
			log_error("device_list element allocation failed");
			return NULL;
		}
		devl->dev = info->dev;
		dm_list_add(&devs, &devl->list);
	}

	memcpy(vgid_found, vginfo->vgid, sizeof(vgid_found));

	dm_list_iterate_safe(devh, tmp, &devs) {
		devl = dm_list_item(devh, struct device_list);
		label_read(devl->dev);
		dm_list_del(&devl->list);
		free(devl);
	}

	/* If vginfo changed, caller needs to rescan */
	if (!(vginfo = lvmcache_vginfo_from_vgname(vgname, vgid_found)) ||
	    strncmp(vginfo->vgid, vgid_found, ID_LEN))
		return NULL;

out:
	return vginfo->fmt;
}

struct lvmcache_vginfo *lvmcache_vginfo_from_vgid(const char *vgid)
{
	struct lvmcache_vginfo *vginfo;
	char id[ID_LEN + 1] __attribute__((aligned(8)));

	if (!_vgid_hash || !vgid) {
		log_debug_cache(INTERNAL_ERROR "Internal cache cannot lookup vgid.");
		return NULL;
	}

	/* vgid not necessarily NULL-terminated */
	(void) dm_strncpy(id, vgid, sizeof(id));

	if (!(vginfo = dm_hash_lookup(_vgid_hash, id))) {
		log_debug_cache("lvmcache has no info for vgid \"%s\"", id);
		return NULL;
	}

	return vginfo;
}

const char *lvmcache_vgname_from_vgid(struct dm_pool *mem, const char *vgid)
{
	struct lvmcache_vginfo *vginfo;
	const char *vgname = NULL;

	if ((vginfo = lvmcache_vginfo_from_vgid(vgid)))
		vgname = vginfo->vgname;

	if (mem && vgname)
		return dm_pool_strdup(mem, vgname);

	return vgname;
}

const char *lvmcache_vgid_from_vgname(struct cmd_context *cmd, const char *vgname)
{
	struct lvmcache_vginfo *vginfo;

	if (!(vginfo = dm_hash_lookup(_vgname_hash, vgname)))
		return_NULL;

	if (!vginfo->next)
		return dm_pool_strdup(cmd->mem, vginfo->vgid);

	/*
	 * There are multiple VGs with this name to choose from.
	 * Return an error because we don't know which VG is intended.
	 */
	return NULL;
}

/*
 * If valid_only is set, data will only be returned if the cached data is
 * known still to be valid.
 *
 * When the device being worked with is known, pass that dev as the second arg.
 * This ensures that when duplicates exist, the wrong dev isn't used.
 */
struct lvmcache_info *lvmcache_info_from_pvid(const char *pvid, struct device *dev, int valid_only)
{
	struct lvmcache_info *info;
	char id[ID_LEN + 1] __attribute__((aligned(8)));

	if (!_pvid_hash || !pvid)
		return NULL;

	(void) dm_strncpy(id, pvid, sizeof(id));

	if (!(info = dm_hash_lookup(_pvid_hash, id)))
		return NULL;

	/*
	 * When handling duplicate PVs, more than one device can have this pvid.
	 */
	if (dev && info->dev && (info->dev != dev)) {
		log_debug_cache("Ignoring lvmcache info for dev %s because dev %s was requested for PVID %s.",
				dev_name(info->dev), dev_name(dev), id);
		return NULL;
	}

	return info;
}

const struct format_type *lvmcache_fmt_from_info(struct lvmcache_info *info)
{
	return info->fmt;
}

const char *lvmcache_vgname_from_info(struct lvmcache_info *info)
{
	if (info->vginfo)
		return info->vginfo->vgname;
	return NULL;
}

/*
 * Check if any PVs in vg->pvs have the same PVID as any
 * entries in _unused_duplicate_devices.
 */

int vg_has_duplicate_pvs(struct volume_group *vg)
{
	struct pv_list *pvl;
	struct device_list *devl;

	dm_list_iterate_items(pvl, &vg->pvs) {
		dm_list_iterate_items(devl, &_unused_duplicate_devs) {
			if (id_equal(&pvl->pv->id, (const struct id *)devl->dev->pvid))
				return 1;
		}
	}
	return 0;
}

int dev_in_device_list(struct device *dev, struct dm_list *head)
{
	struct device_list *devl;

	dm_list_iterate_items(devl, head) {
		if (devl->dev == dev)
			return 1;
	}
	return 0;
}

int lvmcache_dev_is_unchosen_duplicate(struct device *dev)
{
	return dev_in_device_list(dev, &_unused_duplicate_devs);
}

/*
 * Treat some duplicate devs as if they were filtered out by filters.
 * The actual filters are evaluated too early, before a complete
 * picture of all PVs is available, to eliminate these duplicates.
 *
 * By removing some duplicates from unused_duplicate_devs here, we remove
 * the restrictions that are placed on using duplicate devs or VGs with
 * duplicate devs.
 *
 * In cases where we know that two duplicates refer to the same underlying
 * storage, and we know which dev path to use, it's best for us to just
 * use that one preferred device path and ignore the others.  It is the cases
 * where we are unsure whether dups refer to the same underlying storage where
 * we need to keep the unused duplicate referenced in the
 * unused_duplicate_devs list, and restrict what we allow done with it.
 *
 * In the case of md components, we usually filter these out in filter-md,
 * but in the special case of md superblock version 1.0 where the superblock
 * is at the end of the device, filter-md doesn't always eliminate them
 * first, so we eliminate them here.
 *
 * There may other kinds of duplicates that we want to eliminate at
 * this point (using the knowledge from the scan) that we couldn't
 * eliminate in the filters prior to the scan.
 */

static void _filter_duplicate_devs(struct cmd_context *cmd)
{
	struct dev_types *dt = cmd->dev_types;
	struct lvmcache_info *info;
	struct device_list *devl, *devl2;

	dm_list_iterate_items_safe(devl, devl2, &_unused_duplicate_devs) {

		if (!(info = lvmcache_info_from_pvid(devl->dev->pvid, NULL, 0)))
			continue;

		if (MAJOR(info->dev->dev) == dt->md_major) {
			log_debug_devs("Ignoring md component duplicate %s", dev_name(devl->dev));
			dm_list_del(&devl->list);
			free(devl);
		}
	}

	if (dm_list_empty(&_unused_duplicate_devs))
		_found_duplicate_pvs = 0;
}

static void _warn_duplicate_devs(struct cmd_context *cmd)
{
	char uuid[64] __attribute__((aligned(8)));
	struct lvmcache_info *info;
	struct device_list *devl, *devl2;

	dm_list_iterate_items_safe(devl, devl2, &_unused_duplicate_devs) {
		if (!id_write_format((const struct id *)devl->dev->pvid, uuid, sizeof(uuid)))
			stack;

		log_warn("WARNING: Not using device %s for PV %s.", dev_name(devl->dev), uuid);
	}

	dm_list_iterate_items_safe(devl, devl2, &_unused_duplicate_devs) {
		/* info for the preferred device that we're actually using */
		if (!(info = lvmcache_info_from_pvid(devl->dev->pvid, NULL, 0)))
			continue;

		if (!id_write_format((const struct id *)info->dev->pvid, uuid, sizeof(uuid)))
			stack;

		log_warn("WARNING: PV %s prefers device %s because %s.",
			 uuid, dev_name(info->dev), info->dev->duplicate_prefer_reason);
	}
}

/*
 * Compare _found_duplicate_devs entries with the corresponding duplicate dev
 * in lvmcache.  There may be multiple duplicates in _found_duplicate_devs for
 * a given pvid.  If a dev from _found_duplicate_devs is preferred over the dev
 * in lvmcache, then drop the dev in lvmcache and rescan the preferred dev to
 * add it to lvmcache.
 *
 * _found_duplicate_devs: duplicate devs found during initial scan.
 * These are compared to lvmcache devs to see if any are preferred.
 *
 * _unused_duplicate_devs: duplicate devs not chosen to be used.
 * These are _found_duplicate_devs entries that were not chosen,
 * or unpreferred lvmcache devs that were dropped.
 *
 * del_cache_devs: devices to drop from lvmcache
 * add_cache_devs: devices to scan to add to lvmcache
 */

static void _choose_preferred_devs(struct cmd_context *cmd,
				   struct dm_list *del_cache_devs,
				   struct dm_list *add_cache_devs)
{
	const char *reason;
	struct dm_list altdevs;
	struct dm_list new_unused;
	struct dev_types *dt = cmd->dev_types;
	struct device_list *devl, *devl_safe, *alt, *del;
	struct lvmcache_info *info;
	struct device *dev1, *dev2;
	uint32_t dev1_major, dev1_minor, dev2_major, dev2_minor;
	uint64_t info_size, dev1_size, dev2_size;
	int in_subsys1, in_subsys2;
	int is_dm1, is_dm2;
	int has_fs1, has_fs2;
	int has_lv1, has_lv2;
	int same_size1, same_size2;
	int prev_unchosen1, prev_unchosen2;
	int change;

	dm_list_init(&new_unused);

	/*
	 * Create a list of all alternate devs for the same pvid: altdevs.
	 */
next:
	dm_list_init(&altdevs);
	alt = NULL;

	dm_list_iterate_items_safe(devl, devl_safe, &_found_duplicate_devs) {
		if (!alt) {
			dm_list_move(&altdevs, &devl->list);
			alt = devl;
		} else {
			if (!strcmp(alt->dev->pvid, devl->dev->pvid))
				dm_list_move(&altdevs, &devl->list);
		}
	}

	if (!alt) {
		_destroy_duplicate_device_list(&_unused_duplicate_devs);
		dm_list_splice(&_unused_duplicate_devs, &new_unused);
		return;
	}

	/*
	 * Find the device for the pvid that's currently in lvmcache.
	 */

	if (!(info = lvmcache_info_from_pvid(alt->dev->pvid, NULL, 0))) {
		/* This shouldn't happen */
		log_warn("WARNING: PV %s on duplicate device %s not found in cache.",
			 alt->dev->pvid, dev_name(alt->dev));
		goto next;
	}

	/*
	 * Compare devices for the given pvid to find one that's preferred.
	 * "dev1" is the currently preferred device, starting with the device
	 * currently in lvmcache.
	 */

	dev1 = info->dev;

	dm_list_iterate_items(devl, &altdevs) {
		dev2 = devl->dev;

		if (dev1 == dev2) {
			/* This shouldn't happen */
			log_warn("Same duplicate device repeated %s", dev_name(dev1));
			continue;
		}

		prev_unchosen1 = dev_in_device_list(dev1, &_unused_duplicate_devs);
		prev_unchosen2 = dev_in_device_list(dev2, &_unused_duplicate_devs);

		if (!prev_unchosen1 && !prev_unchosen2) {
			/*
			 * The cmd list saves the unchosen preference across
			 * lvmcache_destroy.  Sometimes a single command will
			 * fill lvmcache, destroy it, and refill it, and we
			 * want the same duplicate preference to be preserved
			 * in each instance of lvmcache for a single command.
			 */
			prev_unchosen1 = dev_in_device_list(dev1, &cmd->unused_duplicate_devs);
			prev_unchosen2 = dev_in_device_list(dev2, &cmd->unused_duplicate_devs);
		}

		dev1_major = MAJOR(dev1->dev);
		dev1_minor = MINOR(dev1->dev);
		dev2_major = MAJOR(dev2->dev);
		dev2_minor = MINOR(dev2->dev);

		if (!dev_get_size(dev1, &dev1_size))
			dev1_size = 0;
		if (!dev_get_size(dev2, &dev2_size))
			dev2_size = 0;

		has_lv1 = (dev1->flags & DEV_USED_FOR_LV) ? 1 : 0;
		has_lv2 = (dev2->flags & DEV_USED_FOR_LV) ? 1 : 0;

		in_subsys1 = dev_subsystem_part_major(dt, dev1);
		in_subsys2 = dev_subsystem_part_major(dt, dev2);

		is_dm1 = dm_is_dm_major(dev1_major);
		is_dm2 = dm_is_dm_major(dev2_major);

		has_fs1 = dm_device_has_mounted_fs(dev1_major, dev1_minor);
		has_fs2 = dm_device_has_mounted_fs(dev2_major, dev2_minor);

		info_size = info->device_size >> SECTOR_SHIFT;
		same_size1 = (dev1_size == info_size);
		same_size2 = (dev2_size == info_size);

		log_debug_cache("PV %s compare duplicates: %s %u:%u. %s %u:%u.",
				devl->dev->pvid,
				dev_name(dev1), dev1_major, dev1_minor,
				dev_name(dev2), dev2_major, dev2_minor);

		log_debug_cache("PV %s: wants size %llu. %s is %llu. %s is %llu.",
				devl->dev->pvid,
				(unsigned long long)info_size,
				dev_name(dev1), (unsigned long long)dev1_size,
				dev_name(dev2), (unsigned long long)dev2_size);

		log_debug_cache("PV %s: %s was prev %s. %s was prev %s.",
				devl->dev->pvid,
				dev_name(dev1), prev_unchosen1 ? "not chosen" : "<none>",
				dev_name(dev2), prev_unchosen2 ? "not chosen" : "<none>");

		log_debug_cache("PV %s: %s %s subsystem. %s %s subsystem.",
				devl->dev->pvid,
				dev_name(dev1), in_subsys1 ? "is in" : "is not in",
				dev_name(dev2), in_subsys2 ? "is in" : "is not in");

		log_debug_cache("PV %s: %s %s dm. %s %s dm.",
				devl->dev->pvid,
				dev_name(dev1), is_dm1 ? "is" : "is not",
				dev_name(dev2), is_dm2 ? "is" : "is not");

		log_debug_cache("PV %s: %s %s mounted fs. %s %s mounted fs.",
				devl->dev->pvid,
				dev_name(dev1), has_fs1 ? "has" : "has no",
				dev_name(dev2), has_fs2 ? "has" : "has no");

		log_debug_cache("PV %s: %s %s LV. %s %s LV.",
				devl->dev->pvid,
				dev_name(dev1), has_lv1 ? "is used for" : "is not used for",
				dev_name(dev2), has_lv2 ? "is used for" : "is not used for");

		change = 0;

		if (prev_unchosen1 && !prev_unchosen2) {
			/* change to 2 (NB when unchosen is set we unprefer) */
			change = 1;
			reason = "of previous preference";
		} else if (prev_unchosen2 && !prev_unchosen1) {
			/* keep 1 (NB when unchosen is set we unprefer) */
			reason = "of previous preference";
		} else if (has_lv1 && !has_lv2) {
			/* keep 1 */
			reason = "device is used by LV";
		} else if (has_lv2 && !has_lv1) {
			/* change to 2 */
			change = 1;
			reason = "device is used by LV";
		} else if (same_size1 && !same_size2) {
			/* keep 1 */
			reason = "device size is correct";
		} else if (same_size2 && !same_size1) {
			/* change to 2 */
			change = 1;
			reason = "device size is correct";
		} else if (has_fs1 && !has_fs2) {
			/* keep 1 */
			reason = "device has fs mounted";
		} else if (has_fs2 && !has_fs1) {
			/* change to 2 */
			change = 1;
			reason = "device has fs mounted";
		} else if (is_dm1 && !is_dm2) {
			/* keep 1 */
			reason = "device is in dm subsystem";
		} else if (is_dm2 && !is_dm1) {
			/* change to 2 */
			change = 1;
			reason = "device is in dm subsystem";
		} else if (in_subsys1 && !in_subsys2) {
			/* keep 1 */
			reason = "device is in subsystem";
		} else if (in_subsys2 && !in_subsys1) {
			/* change to 2 */
			change = 1;
			reason = "device is in subsystem";
		} else {
			reason = "device was seen first";
		}

		if (change) {
			dev1 = dev2;
			alt = devl;
		}

		dev1->duplicate_prefer_reason = reason;
	}

	if (dev1 != info->dev) {
		log_debug_cache("PV %s: switching to device %s instead of device %s.",
				 dev1->pvid, dev_name(dev1), dev_name(info->dev));
		/*
		 * Move the preferred device from altdevs to add_cache_devs.
		 * Create a del_cache_devs entry for the current lvmcache
		 * device to drop.
		 */

		dm_list_move(add_cache_devs, &alt->list);

		if ((del = zalloc(sizeof(*del)))) {
			del->dev = info->dev;
			dm_list_add(del_cache_devs, &del->list);
		}

	} else {
		log_debug_cache("PV %s: keeping current device %s.", dev1->pvid, dev_name(info->dev));
	}

	/*
	 * alt devs not chosen are moved to _unused_duplicate_devs.
	 * del_cache_devs being dropped are moved to _unused_duplicate_devs
	 * after being dropped.  So, _unused_duplicate_devs represents all
	 * duplicates not being used in lvmcache.
	 */

	dm_list_splice(&new_unused, &altdevs);

	goto next;
}

/*
 * The initial label_scan at the start of the command is done without
 * holding VG locks.  Then for each VG identified during the label_scan,
 * vg_read(vgname) is called while holding the VG lock.  The labels
 * and metadata on this VG's devices could have changed between the
 * initial unlocked label_scan and the current vg_read().  So, we reread
 * the labels/metadata for each device in the VG now that we hold the
 * lock, and use this for processing the VG.
 *
 * A label scan is ultimately creating associations between devices
 * and VGs so that when vg_read wants to get VG metadata, it knows
 * which devices to read.
 *
 * It's possible that a VG is being modified during the first label
 * scan, causing the scan to see inconsistent metadata on different
 * devs in the VG.  It's possible that those modifications are
 * adding/removing devs from the VG, in which case the device/VG
 * associations in lvmcache after the scan are not correct.
 * NB. It's even possible the VG was removed completely between
 * label scan and here, in which case we'd not find the VG in
 * lvmcache after this rescan.
 *
 * A scan will also create in incorrect/incomplete picture of a VG
 * when devices have no metadata areas.  The scan does not use
 * VG metadata to figure out that a dev with no metadata belongs
 * to a particular VG, so a device with no mdas will not be linked
 * to that VG after a scan.
 */

int lvmcache_label_rescan_vg(struct cmd_context *cmd, const char *vgname, const char *vgid)
{
	struct dm_list devs;
	struct device_list *devl, *devl2;
	struct lvmcache_vginfo *vginfo;
	struct lvmcache_info *info;

	dm_list_init(&devs);

	if (!(vginfo = lvmcache_vginfo_from_vgname(vgname, vgid)))
		return_0;

	dm_list_iterate_items(info, &vginfo->infos) {
		if (!(devl = malloc(sizeof(*devl)))) {
			log_error("device_list element allocation failed");
			return 0;
		}
		devl->dev = info->dev;
		dm_list_add(&devs, &devl->list);
	}

	/* Delete info for each dev, deleting the last info will delete vginfo. */
	dm_list_iterate_items(devl, &devs)
		lvmcache_del_dev(devl->dev);

	/* Dropping the last info struct is supposed to drop vginfo. */
	if ((vginfo = lvmcache_vginfo_from_vgname(vgname, vgid)))
		log_warn("VG info not dropped before rescan of %s", vgname);

	/* FIXME: should we also rescan unused_duplicate_devs for devs
	   being rescanned here and then repeat resolving the duplicates? */

	label_scan_devs(cmd, cmd->filter, &devs);

	dm_list_iterate_items_safe(devl, devl2, &devs) {
		dm_list_del(&devl->list);
		free(devl);
	}

	if (!(vginfo = lvmcache_vginfo_from_vgname(vgname, vgid))) {
		log_warn("VG info not found after rescan of %s", vgname);
		return 0;
	}

	return 1;
}

/*
 * Uses label_scan to populate lvmcache with 'vginfo' struct for each VG
 * and associated 'info' structs for those VGs.  Only VG summary information
 * is used to assemble the vginfo/info during the scan, so the resulting
 * representation of VG/PV state is incomplete and even incorrect.
 * Specifically, PVs with no MDAs are considered orphans and placed in the
 * orphan vginfo by lvmcache_label_scan.  This is corrected during the
 * processing phase as each vg_read() uses VG metadata for each VG to correct
 * the lvmcache state, i.e. it moves no-MDA PVs from the orphan vginfo onto
 * the correct vginfo.  Once vg_read() is finished for all VGs, all of the
 * incorrectly placed PVs should have been moved from the orphan vginfo
 * onto their correct vginfo's, and the orphan vginfo should (in theory)
 * represent only real orphan PVs.  (Note: if lvmcache_label_scan is run
 * after vg_read udpates to lvmcache state, then the lvmcache will be
 * incorrect again, so do not run lvmcache_label_scan during the
 * processing phase.)
 *
 * TODO: in this label scan phase, don't stash no-MDA PVs into the
 * orphan VG.  We know that's a fiction, and it can have harmful/damaging
 * results.  Instead, put them into a temporary list where they can be
 * pulled from later when vg_read uses metadata to resolve which VG
 * they actually belong to.
 */

int lvmcache_label_scan(struct cmd_context *cmd)
{
	struct dm_list del_cache_devs;
	struct dm_list add_cache_devs;
	struct lvmcache_info *info;
	struct lvmcache_vginfo *vginfo;
	struct device_list *devl;
	int vginfo_count = 0;

	int r = 0;

	log_debug_cache("Finding VG info");

	/* Avoid recursion when a PVID can't be found! */
	if (_scanning_in_progress)
		return 0;

	_scanning_in_progress = 1;

	/* FIXME: can this happen? */
	if (!cmd->filter) {
		log_error("label scan is missing filter");
		goto out;
	}

	if (!refresh_filters(cmd))
		log_error("Scan failed to refresh device filter.");

	/*
	 * Duplicates found during this label scan are added to _found_duplicate_devs().
	 */
	_destroy_duplicate_device_list(&_found_duplicate_devs);

	/*
	 * Do the actual scanning.  This populates lvmcache
	 * with infos/vginfos based on reading headers from
	 * each device, and a vg summary from each mda.
	 *
	 * Note that this will *skip* scanning a device if
	 * an info struct already exists in lvmcache for
	 * the device.
	 */
	label_scan(cmd);

	/*
	 * _choose_preferred_devs() returns:
	 *
	 * . del_cache_devs: a list of devs currently in lvmcache that should
	 * be removed from lvmcache because they will be replaced with
	 * alternative devs for the same PV.
	 *
	 * . add_cache_devs: a list of devs that are preferred over devs in
	 * lvmcache for the same PV.  These devices should be rescanned to
	 * populate lvmcache from them.
	 *
	 * First remove lvmcache info for the devs to be dropped, then rescan
	 * the devs that are preferred to add them to lvmcache.
	 *
	 * Keep a complete list of all devs that are unused by moving the
	 * del_cache_devs onto _unused_duplicate_devs.
	 */

	if (!dm_list_empty(&_found_duplicate_devs)) {
		dm_list_init(&del_cache_devs);
		dm_list_init(&add_cache_devs);

		log_debug_cache("Resolving duplicate devices");

		_choose_preferred_devs(cmd, &del_cache_devs, &add_cache_devs);

		dm_list_iterate_items(devl, &del_cache_devs) {
			log_debug_cache("Drop duplicate device %s in lvmcache", dev_name(devl->dev));
			if ((info = lvmcache_info_from_pvid(devl->dev->pvid, NULL, 0)))
				lvmcache_del(info);
		}

		dm_list_iterate_items(devl, &add_cache_devs) {
			log_debug_cache("Rescan preferred device %s for lvmcache", dev_name(devl->dev));
			label_read(devl->dev);
		}

		dm_list_splice(&_unused_duplicate_devs, &del_cache_devs);

		/*
		 * This may remove some entries from the unused_duplicates list for
		 * devs that we know are the same underlying dev.
		 */
		_filter_duplicate_devs(cmd);

		/*
		 * Warn about remaining duplicates that may actually be separate copies of
		 * the same device.
		 */
		_warn_duplicate_devs(cmd);
	}

	r = 1;

      out:
	_scanning_in_progress = 0;

	dm_list_iterate_items(vginfo, &_vginfos) {
		if (is_orphan_vg(vginfo->vgname))
			continue;
		vginfo_count++;
	}

	log_debug_cache("Found VG info for %d VGs", vginfo_count);

	return r;
}

/*
 * lvmcache_label_scan() detects duplicates in the basic label_scan(), then
 * filters out some dups, and chooses preferred duplicates to use.
 */

void lvmcache_pvscan_duplicate_check(struct cmd_context *cmd)
{
	struct device_list *devl;

	/* Check if label_scan() detected any dups. */
	if (!_found_duplicate_pvs)
		return;

	/*
	 * Once all the dups are identified, they are moved from the
	 * "found" list to the "unused" list to sort out.
	 */
	dm_list_splice(&_unused_duplicate_devs, &_found_duplicate_devs);

	/*
	 * Remove items from the dups list that we know are the same
	 * underlying dev, e.g. md components, that we want to just ignore.
	 */
	_filter_duplicate_devs(cmd);

	/*
	 * no more dups after ignoring some
	 */
	if (!_found_duplicate_pvs)
		return;

	/* Duplicates are found where we would have to pick one. */

	dm_list_iterate_items(devl, &_unused_duplicate_devs)
		log_warn("WARNING: found device with duplicate %s", dev_name(devl->dev));
}

int lvmcache_get_vgnameids(struct cmd_context *cmd, int include_internal,
			   struct dm_list *vgnameids)
{
	struct vgnameid_list *vgnl;
	struct lvmcache_vginfo *vginfo;

	dm_list_iterate_items(vginfo, &_vginfos) {
		if (!include_internal && is_orphan_vg(vginfo->vgname))
			continue;

		if (!(vgnl = dm_pool_alloc(cmd->mem, sizeof(*vgnl)))) {
			log_error("vgnameid_list allocation failed.");
			return 0;
		}

		vgnl->vgid = dm_pool_strdup(cmd->mem, vginfo->vgid);
		vgnl->vg_name = dm_pool_strdup(cmd->mem, vginfo->vgname);

		if (!vgnl->vgid || !vgnl->vg_name) {
			log_error("vgnameid_list member allocation failed.");
			return 0;
		}

		dm_list_add(vgnameids, &vgnl->list);
	}

	return 1;
}

struct dm_list *lvmcache_get_pvids(struct cmd_context *cmd, const char *vgname,
				const char *vgid)
{
	struct dm_list *pvids;
	struct lvmcache_vginfo *vginfo;
	struct lvmcache_info *info;

	if (!(pvids = str_list_create(cmd->mem))) {
		log_error("pvids list allocation failed");
		return NULL;
	}

	if (!(vginfo = lvmcache_vginfo_from_vgname(vgname, vgid)))
		return pvids;

	dm_list_iterate_items(info, &vginfo->infos) {
		if (!str_list_add(cmd->mem, pvids,
				  dm_pool_strdup(cmd->mem, info->dev->pvid))) {
			log_error("strlist allocation failed");
			return NULL;
		}
	}

	return pvids;
}

int lvmcache_get_vg_devs(struct cmd_context *cmd,
			 struct lvmcache_vginfo *vginfo,
			 struct dm_list *devs)
{
	struct lvmcache_info *info;
	struct device_list *devl;

	dm_list_iterate_items(info, &vginfo->infos) {
		if (!(devl = dm_pool_zalloc(cmd->mem, sizeof(*devl))))
			return_0;

		devl->dev = info->dev;
		dm_list_add(devs, &devl->list);
	}
	return 1;
}

static struct device *_device_from_pvid(const struct id *pvid, uint64_t *label_sector)
{
	struct lvmcache_info *info;

	if ((info = lvmcache_info_from_pvid((const char *) pvid, NULL, 0))) {
		if (info->label && label_sector)
			*label_sector = info->label->sector;
		return info->dev;
	}

	return NULL;
}

struct device *lvmcache_device_from_pvid(struct cmd_context *cmd, const struct id *pvid, uint64_t *label_sector)
{
	struct device *dev;

	dev = _device_from_pvid(pvid, label_sector);
	if (dev)
		return dev;

	log_debug_devs("No device with uuid %s.", (const char *)pvid);
	return NULL;
}

int lvmcache_pvid_in_unchosen_duplicates(const char *pvid)
{
	struct device_list *devl;

	dm_list_iterate_items(devl, &_unused_duplicate_devs) {
		if (!strncmp(devl->dev->pvid, pvid, ID_LEN))
			return 1;
	}
	return 0;
}

static int _free_vginfo(struct lvmcache_vginfo *vginfo)
{
	struct lvmcache_vginfo *primary_vginfo, *vginfo2;
	int r = 1;

	vginfo2 = primary_vginfo = lvmcache_vginfo_from_vgname(vginfo->vgname, NULL);

	if (vginfo == primary_vginfo) {
		dm_hash_remove(_vgname_hash, vginfo->vgname);
		if (vginfo->next && !dm_hash_insert(_vgname_hash, vginfo->vgname,
						    vginfo->next)) {
			log_error("_vgname_hash re-insertion for %s failed",
				  vginfo->vgname);
			r = 0;
		}
	} else
		while (vginfo2) {
			if (vginfo2->next == vginfo) {
				vginfo2->next = vginfo->next;
				break;
			}
			vginfo2 = vginfo2->next;
		}

	free(vginfo->system_id);
	free(vginfo->vgname);
	free(vginfo->creation_host);

	if (*vginfo->vgid && _vgid_hash &&
	    lvmcache_vginfo_from_vgid(vginfo->vgid) == vginfo)
		dm_hash_remove(_vgid_hash, vginfo->vgid);

	dm_list_del(&vginfo->list);

	free(vginfo);

	return r;
}

/*
 * vginfo must be info->vginfo unless info is NULL
 */
static int _drop_vginfo(struct lvmcache_info *info, struct lvmcache_vginfo *vginfo)
{
	if (info)
		_vginfo_detach_info(info);

	/* vginfo still referenced? */
	if (!vginfo || is_orphan_vg(vginfo->vgname) ||
	    !dm_list_empty(&vginfo->infos))
		return 1;

	if (!_free_vginfo(vginfo))
		return_0;

	return 1;
}

void lvmcache_del(struct lvmcache_info *info)
{
	if (info->dev->pvid[0] && _pvid_hash)
		dm_hash_remove(_pvid_hash, info->dev->pvid);

	_drop_vginfo(info, info->vginfo);

	info->label->labeller->ops->destroy_label(info->label->labeller,
						  info->label);
	label_destroy(info->label);
	free(info);
}

void lvmcache_del_dev(struct device *dev)
{
	struct lvmcache_info *info;

	if ((info = lvmcache_info_from_pvid((const char *)dev->pvid, dev, 0)))
		lvmcache_del(info);
}

/*
 * vginfo must be info->vginfo unless info is NULL (orphans)
 */
static int _lvmcache_update_vgid(struct lvmcache_info *info,
				 struct lvmcache_vginfo *vginfo,
				 const char *vgid)
{
	if (!vgid || !vginfo ||
	    !strncmp(vginfo->vgid, vgid, ID_LEN))
		return 1;

	if (vginfo && *vginfo->vgid)
		dm_hash_remove(_vgid_hash, vginfo->vgid);
	if (!vgid) {
		/* FIXME: unreachable code path */
		log_debug_cache("lvmcache: %s: clearing VGID", info ? dev_name(info->dev) : vginfo->vgname);
		return 1;
	}

	(void) dm_strncpy(vginfo->vgid, vgid, sizeof(vginfo->vgid));
	if (!dm_hash_insert(_vgid_hash, vginfo->vgid, vginfo)) {
		log_error("_lvmcache_update: vgid hash insertion failed: %s",
			  vginfo->vgid);
		return 0;
	}

	if (!is_orphan_vg(vginfo->vgname))
		log_debug_cache("lvmcache %s: VG %s: set VGID to " FMTVGID ".",
				(info) ? dev_name(info->dev) : "",
				vginfo->vgname, vginfo->vgid);

	return 1;
}

static int _insert_vginfo(struct lvmcache_vginfo *new_vginfo, const char *vgid,
			  uint32_t vgstatus, const char *creation_host,
			  struct lvmcache_vginfo *primary_vginfo)
{
	struct lvmcache_vginfo *last_vginfo = primary_vginfo;
	char uuid_primary[64] __attribute__((aligned(8)));
	char uuid_new[64] __attribute__((aligned(8)));
	int use_new = 0;

	/* Pre-existing VG takes precedence. Unexported VG takes precedence. */
	if (primary_vginfo) {
		if (!id_write_format((const struct id *)vgid, uuid_new, sizeof(uuid_new)))
			return_0;

		if (!id_write_format((const struct id *)&primary_vginfo->vgid, uuid_primary,
				     sizeof(uuid_primary)))
			return_0;

		/*
		 * vginfo is kept for each VG with the same name.
		 * They are saved with the vginfo->next list.
		 * These checks just decide the ordering of
		 * that list.
		 *
		 * FIXME: it should no longer matter what order
		 * the vginfo's are kept in, so we can probably
		 * remove these comparisons and reordering entirely.
		 *
		 * If   Primary not exported, new exported => keep
		 * Else Primary exported, new not exported => change
		 * Else Primary has hostname for this machine => keep
		 * Else Primary has no hostname, new has one => change
		 * Else New has hostname for this machine => change
		 * Else Keep primary.
		 */
		if (!(primary_vginfo->status & EXPORTED_VG) &&
		    (vgstatus & EXPORTED_VG))
			log_verbose("Cache: Duplicate VG name %s: "
				    "Existing %s takes precedence over "
				    "exported %s", new_vginfo->vgname,
				    uuid_primary, uuid_new);
		else if ((primary_vginfo->status & EXPORTED_VG) &&
			   !(vgstatus & EXPORTED_VG)) {
			log_verbose("Cache: Duplicate VG name %s: "
				    "%s takes precedence over exported %s",
				    new_vginfo->vgname, uuid_new,
				    uuid_primary);
			use_new = 1;
		} else if (primary_vginfo->creation_host &&
			   !strcmp(primary_vginfo->creation_host,
				   primary_vginfo->fmt->cmd->hostname))
			log_verbose("Cache: Duplicate VG name %s: "
				    "Existing %s (created here) takes precedence "
				    "over %s", new_vginfo->vgname, uuid_primary,
				    uuid_new);
		else if (!primary_vginfo->creation_host && creation_host) {
			log_verbose("Cache: Duplicate VG name %s: "
				    "%s (with creation_host) takes precedence over %s",
				    new_vginfo->vgname, uuid_new,
				    uuid_primary);
			use_new = 1;
		} else if (creation_host &&
			   !strcmp(creation_host,
				   primary_vginfo->fmt->cmd->hostname)) {
			log_verbose("Cache: Duplicate VG name %s: "
				    "%s (created here) takes precedence over %s",
				    new_vginfo->vgname, uuid_new,
				    uuid_primary);
			use_new = 1;
		} else {
			log_verbose("Cache: Duplicate VG name %s: "
				    "Prefer existing %s vs new %s",
				    new_vginfo->vgname, uuid_primary, uuid_new);
		}

		if (!use_new) {
			while (last_vginfo->next)
				last_vginfo = last_vginfo->next;
			last_vginfo->next = new_vginfo;
			return 1;
		}

		dm_hash_remove(_vgname_hash, primary_vginfo->vgname);
	}

	if (!dm_hash_insert(_vgname_hash, new_vginfo->vgname, new_vginfo)) {
		log_error("cache_update: vg hash insertion failed: %s",
		  	new_vginfo->vgname);
		return 0;
	}

	if (primary_vginfo)
		new_vginfo->next = primary_vginfo;

	return 1;
}

static int _lvmcache_update_vgname(struct lvmcache_info *info,
				   const char *vgname, const char *vgid,
				   uint32_t vgstatus, const char *creation_host,
				   const struct format_type *fmt)
{
	struct lvmcache_vginfo *vginfo, *primary_vginfo;
	char mdabuf[32];

	if (!vgname || (info && info->vginfo && !strcmp(info->vginfo->vgname, vgname)))
		return 1;

	/* Remove existing vginfo entry */
	if (info)
		_drop_vginfo(info, info->vginfo);

	if (!(vginfo = lvmcache_vginfo_from_vgname(vgname, vgid))) {
		/*
	 	 * Create a vginfo struct for this VG and put the vginfo
	 	 * into the hash table.
	 	 */

		if (!(vginfo = zalloc(sizeof(*vginfo)))) {
			log_error("lvmcache_update_vgname: list alloc failed");
			return 0;
		}
		if (!(vginfo->vgname = strdup(vgname))) {
			free(vginfo);
			log_error("cache vgname alloc failed for %s", vgname);
			return 0;
		}
		dm_list_init(&vginfo->infos);

		/*
		 * A different VG (different uuid) can exist with the same name.
		 * In this case, the two VGs will have separate vginfo structs,
		 * but the second will be linked onto the existing vginfo->next,
		 * not in the hash.
		 */
		primary_vginfo = lvmcache_vginfo_from_vgname(vgname, NULL);

		if (!_insert_vginfo(vginfo, vgid, vgstatus, creation_host, primary_vginfo)) {
			free(vginfo->vgname);
			free(vginfo);
			return 0;
		}

		/* Ensure orphans appear last on list_iterate */
		if (is_orphan_vg(vgname))
			dm_list_add(&_vginfos, &vginfo->list);
		else
			dm_list_add_h(&_vginfos, &vginfo->list);
	}

	if (info)
		_vginfo_attach_info(vginfo, info);
	else if (!_lvmcache_update_vgid(NULL, vginfo, vgid)) /* Orphans */
		return_0;

	/* FIXME Check consistency of list! */
	vginfo->fmt = fmt;

	if (info) {
		if (info->mdas.n)
			sprintf(mdabuf, " with %u mda(s)", dm_list_size(&info->mdas));
		else
			mdabuf[0] = '\0';
		log_debug_cache("lvmcache %s: now in VG %s%s%s%s%s.",
				dev_name(info->dev),
				vgname, vginfo->vgid[0] ? " (" : "",
				vginfo->vgid[0] ? vginfo->vgid : "",
				vginfo->vgid[0] ? ")" : "", mdabuf);
	} else
		log_debug_cache("lvmcache: Initialised VG %s.", vgname);

	return 1;
}

static int _lvmcache_update_vgstatus(struct lvmcache_info *info, uint32_t vgstatus,
				     const char *creation_host, const char *lock_type,
				     const char *system_id)
{
	if (!info || !info->vginfo)
		return 1;

	if ((info->vginfo->status & EXPORTED_VG) != (vgstatus & EXPORTED_VG))
		log_debug_cache("lvmcache %s: VG %s %s exported.",
				dev_name(info->dev), info->vginfo->vgname,
				vgstatus & EXPORTED_VG ? "now" : "no longer");

	info->vginfo->status = vgstatus;

	if (!creation_host)
		goto set_lock_type;

	if (info->vginfo->creation_host && !strcmp(creation_host,
						   info->vginfo->creation_host))
		goto set_lock_type;

	free(info->vginfo->creation_host);

	if (!(info->vginfo->creation_host = strdup(creation_host))) {
		log_error("cache creation host alloc failed for %s.",
			  creation_host);
		return 0;
	}

	log_debug_cache("lvmcache %s: VG %s: set creation host to %s.",
			dev_name(info->dev), info->vginfo->vgname, creation_host);

set_lock_type:

	if (!lock_type)
		goto set_system_id;

	if (info->vginfo->lock_type && !strcmp(lock_type, info->vginfo->lock_type))
		goto set_system_id;

	free(info->vginfo->lock_type);

	if (!(info->vginfo->lock_type = strdup(lock_type))) {
		log_error("cache lock_type alloc failed for %s", lock_type);
		return 0;
	}

	log_debug_cache("lvmcache %s: VG %s: set lock_type to %s.",
			dev_name(info->dev), info->vginfo->vgname, lock_type);

set_system_id:

	if (!system_id)
		goto out;

	if (info->vginfo->system_id && !strcmp(system_id, info->vginfo->system_id))
		goto out;

	free(info->vginfo->system_id);

	if (!(info->vginfo->system_id = strdup(system_id))) {
		log_error("cache system_id alloc failed for %s", system_id);
		return 0;
	}

	log_debug_cache("lvmcache %s: VG %s: set system_id to %s.",
			dev_name(info->dev), info->vginfo->vgname, system_id);

out:
	return 1;
}

int lvmcache_add_orphan_vginfo(const char *vgname, struct format_type *fmt)
{
	return _lvmcache_update_vgname(NULL, vgname, vgname, 0, "", fmt);
}

/*
 * FIXME: get rid of other callers of this function which call it
 * in odd cases to "fix up" some bit of lvmcache state.  Make those
 * callers fix up what they need to directly, and leave this function
 * with one purpose and caller.
 */

int lvmcache_update_vgname_and_id(struct lvmcache_info *info, struct lvmcache_vgsummary *vgsummary)
{
	const char *vgname = vgsummary->vgname;
	const char *vgid = (char *)&vgsummary->vgid;
	struct lvmcache_vginfo *vginfo;

	if (!vgname && !info->vginfo) {
		log_error(INTERNAL_ERROR "NULL vgname handed to cache");
		/* FIXME Remove this */
		vgname = info->fmt->orphan_vg_name;
		vgid = vgname;
	}

	/* If PV without mdas is already in a real VG, don't make it orphan */
	if (is_orphan_vg(vgname) && info->vginfo &&
	    mdas_empty_or_ignored(&info->mdas) &&
	    !is_orphan_vg(info->vginfo->vgname) && critical_section())
		return 1;

	/*
	 * Creates a new vginfo struct for this vgname/vgid if none exists,
	 * and attaches the info struct for the dev to the vginfo.
	 * Puts the vginfo into the vgname hash table.
	 */
	if (!_lvmcache_update_vgname(info, vgname, vgid, vgsummary->vgstatus, vgsummary->creation_host, info->fmt)) {
		log_error("Failed to update VG %s info in lvmcache.", vgname);
		return 0;
	}

	/*
	 * Puts the vginfo into the vgid hash table.
	 */
	if (!_lvmcache_update_vgid(info, info->vginfo, vgid)) {
		log_error("Failed to update VG %s info in lvmcache.", vgname);
		return 0;
	}

	/*
	 * FIXME: identify which case this is and why this is needed, then
	 * change that so it doesn't use this function and we can remove
	 * this special case.
	 * (I think this distinguishes the scan path, where these things
	 * are set from the vg_read path where lvmcache_update_vg() is
	 * called which calls this function without seqno/mda_size/mda_checksum.)
	 */
	if (!vgsummary->seqno && !vgsummary->mda_size && !vgsummary->mda_checksum)
		return 1;

	if (!(vginfo = info->vginfo))
		return 1;

	if (!vginfo->seqno) {
		vginfo->seqno = vgsummary->seqno;

		log_debug_cache("lvmcache %s: VG %s: set seqno to %d",
				dev_name(info->dev), vginfo->vgname, vginfo->seqno);

	} else if (vgsummary->seqno != vginfo->seqno) {
		log_warn("Scan of VG %s from %s found metadata seqno %d vs previous %d.",
			 vgname, dev_name(info->dev), vgsummary->seqno, vginfo->seqno);
		vginfo->scan_summary_mismatch = 1;
		/* If we don't return success, this dev info will be removed from lvmcache,
		   and then we won't be able to rescan it or repair it. */
		return 1;
	}

	if (!vginfo->mda_size) {
		vginfo->mda_checksum = vgsummary->mda_checksum;
		vginfo->mda_size = vgsummary->mda_size;

		log_debug_cache("lvmcache %s: VG %s: set mda_checksum to %x mda_size to %zu",
				dev_name(info->dev), vginfo->vgname,
				vginfo->mda_checksum, vginfo->mda_size);

	} else if ((vginfo->mda_size != vgsummary->mda_size) || (vginfo->mda_checksum != vgsummary->mda_checksum)) {
		log_warn("Scan of VG %s from %s found mda_checksum %x mda_size %zu vs previous %x %zu",
			 vgname, dev_name(info->dev), vgsummary->mda_checksum, vgsummary->mda_size,
			 vginfo->mda_checksum, vginfo->mda_size);
		vginfo->scan_summary_mismatch = 1;
		/* If we don't return success, this dev info will be removed from lvmcache,
		   and then we won't be able to rescan it or repair it. */
		return 1;
	}

	/*
	 * If a dev has an unmatching checksum, ignore the other
	 * info from it, keeping the info we already saved.
	 */
	if (!_lvmcache_update_vgstatus(info, vgsummary->vgstatus, vgsummary->creation_host,
				       vgsummary->lock_type, vgsummary->system_id)) {
		log_error("Failed to update VG %s info in lvmcache.", vgname);
		return 0;
	}

	return 1;
}

int lvmcache_update_vg(struct volume_group *vg, unsigned precommitted)
{
	struct pv_list *pvl;
	struct lvmcache_info *info;
	char pvid_s[ID_LEN + 1] __attribute__((aligned(8)));
	struct lvmcache_vgsummary vgsummary = {
		.vgname = vg->name,
		.vgstatus = vg->status,
		.vgid = vg->id,
		.system_id = vg->system_id,
		.lock_type = vg->lock_type
	};

	dm_list_iterate_items(pvl, &vg->pvs) {
		(void) dm_strncpy(pvid_s, (char *) &pvl->pv->id, sizeof(pvid_s));
		/* FIXME Could pvl->pv->dev->pvid ever be different? */
		if ((info = lvmcache_info_from_pvid(pvid_s, pvl->pv->dev, 0)) &&
		    !lvmcache_update_vgname_and_id(info, &vgsummary))
			return_0;
	}

	return 1;
}

/*
 * We can see multiple different devices with the
 * same pvid, i.e. duplicates.
 *
 * There may be different reasons for seeing two
 * devices with the same pvid:
 * - multipath showing two paths to the same thing
 * - one device copied to another, e.g. with dd,
 *   also referred to as cloned devices.
 * - a "subsystem" taking a device and creating
 *   another device of its own that represents the
 *   underlying device it is using, e.g. using dm
 *   to create an identity mapping of a PV.
 *
 * Given duplicate devices, we have to choose one
 * of them to be the "preferred" dev, i.e. the one
 * that will be referenced in lvmcache, by pv->dev.
 * We can keep the existing dev, that's currently
 * used in lvmcache, or we can replace the existing
 * dev with the new duplicate.
 *
 * Regardless of which device is preferred, we need
 * to print messages explaining which devices were
 * found so that a user can sort out for themselves
 * what has happened if the preferred device is not
 * the one they are interested in.
 *
 * If a user wants to use the non-preferred device,
 * they will need to filter out the device that
 * lvm is preferring.
 *
 * The dev_subsystem calls check if the major number
 * of the dev is part of a subsystem like DM/MD/DRBD.
 * A dev that's part of a subsystem is preferred over a
 * duplicate of that dev that is not part of a
 * subsystem.
 *
 * FIXME: there may be other reasons to prefer one
 * device over another:
 *
 * . are there other use/open counts we could check
 *   beyond the holders?
 *
 * . check if either is bad/usable and prefer
 *   the good one?
 *
 * . prefer the one with smaller minor number?
 *   Might avoid disturbing things due to a new
 *   transient duplicate?
 */

static struct lvmcache_info * _create_info(struct labeller *labeller, struct device *dev)
{
	struct lvmcache_info *info;
	struct label *label;

	if (!(label = label_create(labeller)))
		return_NULL;
	if (!(info = zalloc(sizeof(*info)))) {
		log_error("lvmcache_info allocation failed");
		label_destroy(label);
		return NULL;
	}

	info->dev = dev;
	info->fmt = labeller->fmt;

	label->info = info;
	info->label = label;

	dm_list_init(&info->list);
	lvmcache_del_mdas(info);
	lvmcache_del_das(info);
	lvmcache_del_bas(info);

	return info;
}

struct lvmcache_info *lvmcache_add(struct labeller *labeller,
				   const char *pvid, struct device *dev,
				   const char *vgname, const char *vgid, uint32_t vgstatus)
{
	char pvid_s[ID_LEN + 1] __attribute__((aligned(8)));
	char uuid[64] __attribute__((aligned(8)));
	struct lvmcache_vgsummary vgsummary = { 0 };
	struct lvmcache_info *info;
	struct lvmcache_info *info_lookup;
	struct device_list *devl;
	int created = 0;

	(void) dm_strncpy(pvid_s, pvid, sizeof(pvid_s));

	if (!id_write_format((const struct id *)&pvid_s, uuid, sizeof(uuid)))
		stack;

	/*
	 * Find existing info struct in _pvid_hash or create a new one.
	 *
	 * Don't pass the known "dev" as an arg here.  The mismatching
	 * devs for the duplicate case is checked below.
	 */

	info = lvmcache_info_from_pvid(pvid_s, NULL, 0);

	if (!info)
		info = lvmcache_info_from_pvid(dev->pvid, NULL, 0);

	if (!info) {
		info = _create_info(labeller, dev);
		created = 1;
	}

	if (!info)
		return_NULL;

	/*
	 * If an existing info struct was found, check if any values are new.
	 */
	if (!created) {
		if (info->dev != dev) {
			log_debug_cache("PV %s on %s was already found on %s.",
				        uuid, dev_name(dev), dev_name(info->dev));

			strncpy(dev->pvid, pvid_s, sizeof(dev->pvid));

			/*
			 * Keep the existing PV/dev in lvmcache, and save the
			 * new duplicate in the list of duplicates.  After
			 * scanning is complete, compare the duplicate devs
			 * with those in lvmcache to check if one of the
			 * duplicates is preferred and if so switch lvmcache to
			 * use it.
			 */

			if (!(devl = zalloc(sizeof(*devl))))
				return_NULL;
			devl->dev = dev;

			dm_list_add(&_found_duplicate_devs, &devl->list);
			_found_duplicate_pvs = 1;
			return NULL;
		}

		if (info->dev->pvid[0] && pvid[0] && strcmp(pvid_s, info->dev->pvid)) {
			/* This happens when running pvcreate on an existing PV. */
			log_verbose("Changing pvid on dev %s from %s to %s",
				    dev_name(info->dev), info->dev->pvid, pvid_s);
		}

		if (info->label->labeller != labeller) {
			log_verbose("Changing labeller on dev %s from %s to %s",
				    dev_name(info->dev),
				    info->label->labeller->fmt->name,
				    labeller->fmt->name);
			label_destroy(info->label);
			if (!(info->label = label_create(labeller)))
				return_NULL;
			info->label->info = info;
		}
	}

	/*
	 * Add or update the _pvid_hash mapping, pvid to info.
	 */

	info_lookup = dm_hash_lookup(_pvid_hash, pvid_s);
	if ((info_lookup == info) && !strcmp(info->dev->pvid, pvid_s))
		goto update_vginfo;

	if (info->dev->pvid[0])
		dm_hash_remove(_pvid_hash, info->dev->pvid);

	strncpy(info->dev->pvid, pvid_s, sizeof(info->dev->pvid));

	if (!dm_hash_insert(_pvid_hash, pvid_s, info)) {
		log_error("Adding pvid to hash failed %s", pvid_s);
		return NULL;
	}

update_vginfo:
	vgsummary.vgstatus = vgstatus;
	vgsummary.vgname = vgname;
	if (vgid)
		strncpy((char *)&vgsummary.vgid, vgid, sizeof(vgsummary.vgid));

	if (!lvmcache_update_vgname_and_id(info, &vgsummary)) {
		if (created) {
			dm_hash_remove(_pvid_hash, pvid_s);
			strcpy(info->dev->pvid, "");
			free(info->label);
			free(info);
		}
		return NULL;
	}

	return info;
}

static void _lvmcache_destroy_entry(struct lvmcache_info *info)
{
	_vginfo_detach_info(info);
	info->dev->pvid[0] = 0;
	label_destroy(info->label);
	free(info);
}

static void _lvmcache_destroy_vgnamelist(struct lvmcache_vginfo *vginfo)
{
	struct lvmcache_vginfo *next;

	do {
		next = vginfo->next;
		if (!_free_vginfo(vginfo))
			stack;
	} while ((vginfo = next));
}

void lvmcache_destroy(struct cmd_context *cmd, int retain_orphans, int reset)
{
	log_debug_cache("Dropping VG info");

	if (_vgid_hash) {
		dm_hash_destroy(_vgid_hash);
		_vgid_hash = NULL;
	}

	if (_pvid_hash) {
		dm_hash_iter(_pvid_hash, (dm_hash_iterate_fn) _lvmcache_destroy_entry);
		dm_hash_destroy(_pvid_hash);
		_pvid_hash = NULL;
	}

	if (_vgname_hash) {
		dm_hash_iter(_vgname_hash,
			  (dm_hash_iterate_fn) _lvmcache_destroy_vgnamelist);
		dm_hash_destroy(_vgname_hash);
		_vgname_hash = NULL;
	}

	if (!dm_list_empty(&_vginfos))
		log_error(INTERNAL_ERROR "_vginfos list should be empty");
	dm_list_init(&_vginfos);

	/*
	 * Copy the current _unused_duplicate_devs into a cmd list before
	 * destroying _unused_duplicate_devs.
	 *
	 * One command can init/populate/destroy lvmcache multiple times.  Each
	 * time it will encounter duplicates and choose the preferrred devs.
	 * We want the same preferred devices to be chosen each time, so save
	 * the unpreferred devs here so that _choose_preferred_devs can use
	 * this to make the same choice each time.
	 */
	dm_list_init(&cmd->unused_duplicate_devs);
	lvmcache_get_unused_duplicate_devs(cmd, &cmd->unused_duplicate_devs);
	_destroy_duplicate_device_list(&_unused_duplicate_devs);
	_destroy_duplicate_device_list(&_found_duplicate_devs); /* should be empty anyway */
	_found_duplicate_pvs = 0;

	if (retain_orphans) {
		struct format_type *fmt;

		if (!lvmcache_init(cmd))
			stack;

		dm_list_iterate_items(fmt, &cmd->formats) {
			if (!lvmcache_add_orphan_vginfo(fmt->orphan_vg_name, fmt))
				stack;
		}
	}
}

int lvmcache_fid_add_mdas(struct lvmcache_info *info, struct format_instance *fid,
			  const char *id, int id_len)
{
	return fid_add_mdas(fid, &info->mdas, id, id_len);
}

int lvmcache_fid_add_mdas_pv(struct lvmcache_info *info, struct format_instance *fid)
{
	return lvmcache_fid_add_mdas(info, fid, info->dev->pvid, ID_LEN);
}

int lvmcache_fid_add_mdas_vg(struct lvmcache_vginfo *vginfo, struct format_instance *fid)
{
	struct lvmcache_info *info;
	dm_list_iterate_items(info, &vginfo->infos) {
		if (!lvmcache_fid_add_mdas_pv(info, fid))
			return_0;
	}
	return 1;
}

int lvmcache_populate_pv_fields(struct lvmcache_info *info,
				struct volume_group *vg,
				struct physical_volume *pv)
{
	struct data_area_list *da;
	
	if (!info->label) {
		log_error("No cached label for orphan PV %s", pv_dev_name(pv));
		return 0;
	}

	pv->label_sector = info->label->sector;
	pv->dev = info->dev;
	pv->fmt = info->fmt;
	pv->size = info->device_size >> SECTOR_SHIFT;
	pv->vg_name = FMT_TEXT_ORPHAN_VG_NAME;
	memcpy(&pv->id, &info->dev->pvid, sizeof(pv->id));

	if (!pv->size) {
		log_error("PV %s size is zero.", dev_name(info->dev));
		return 0;
	}

	/* Currently only support exactly one data area */
	if (dm_list_size(&info->das) != 1) {
		log_error("Must be exactly one data area (found %d) on PV %s",
			  dm_list_size(&info->das), dev_name(info->dev));
		return 0;
	}

	/* Currently only support one bootloader area at most */
	if (dm_list_size(&info->bas) > 1) {
		log_error("Must be at most one bootloader area (found %d) on PV %s",
			  dm_list_size(&info->bas), dev_name(info->dev));
		return 0;
	}

	dm_list_iterate_items(da, &info->das)
		pv->pe_start = da->disk_locn.offset >> SECTOR_SHIFT;

	dm_list_iterate_items(da, &info->bas) {
		pv->ba_start = da->disk_locn.offset >> SECTOR_SHIFT;
		pv->ba_size = da->disk_locn.size >> SECTOR_SHIFT;
	}

	return 1;
}

int lvmcache_check_format(struct lvmcache_info *info, const struct format_type *fmt)
{
	if (info->fmt != fmt) {
		log_error("PV %s is a different format (seqno %s)",
			  dev_name(info->dev), info->fmt->name);
		return 0;
	}
	return 1;
}

void lvmcache_del_mdas(struct lvmcache_info *info)
{
	if (info->mdas.n)
		del_mdas(&info->mdas);
	dm_list_init(&info->mdas);
}

void lvmcache_del_das(struct lvmcache_info *info)
{
	if (info->das.n)
		del_das(&info->das);
	dm_list_init(&info->das);
}

void lvmcache_del_bas(struct lvmcache_info *info)
{
	if (info->bas.n)
		del_bas(&info->bas);
	dm_list_init(&info->bas);
}

int lvmcache_add_mda(struct lvmcache_info *info, struct device *dev,
		     uint64_t start, uint64_t size, unsigned ignored)
{
	return add_mda(info->fmt, NULL, &info->mdas, dev, start, size, ignored);
}

int lvmcache_add_da(struct lvmcache_info *info, uint64_t start, uint64_t size)
{
	return add_da(NULL, &info->das, start, size);
}

int lvmcache_add_ba(struct lvmcache_info *info, uint64_t start, uint64_t size)
{
	return add_ba(NULL, &info->bas, start, size);
}

void lvmcache_update_pv(struct lvmcache_info *info, struct physical_volume *pv,
			const struct format_type *fmt)
{
	info->device_size = pv->size << SECTOR_SHIFT;
	info->fmt = fmt;
}

int lvmcache_update_das(struct lvmcache_info *info, struct physical_volume *pv)
{
	struct data_area_list *da;
	if (info->das.n) {
		if (!pv->pe_start)
			dm_list_iterate_items(da, &info->das)
				pv->pe_start = da->disk_locn.offset >> SECTOR_SHIFT;
		del_das(&info->das);
	} else
		dm_list_init(&info->das);

	if (!add_da(NULL, &info->das, pv->pe_start << SECTOR_SHIFT, 0 /*pv->size << SECTOR_SHIFT*/))
		return_0;

	return 1;
}

int lvmcache_update_bas(struct lvmcache_info *info, struct physical_volume *pv)
{
	struct data_area_list *ba;
	if (info->bas.n) {
		if (!pv->ba_start && !pv->ba_size)
			dm_list_iterate_items(ba, &info->bas) {
				pv->ba_start = ba->disk_locn.offset >> SECTOR_SHIFT;
				pv->ba_size = ba->disk_locn.size >> SECTOR_SHIFT;
			}
		del_das(&info->bas);
	} else
		dm_list_init(&info->bas);

	if (!add_ba(NULL, &info->bas, pv->ba_start << SECTOR_SHIFT, pv->ba_size << SECTOR_SHIFT))
		return_0;

	return 1;
}

int lvmcache_foreach_pv(struct lvmcache_vginfo *vginfo,
			int (*fun)(struct lvmcache_info *, void *),
			void *baton)
{
	struct lvmcache_info *info;
	dm_list_iterate_items(info, &vginfo->infos) {
		if (!fun(info, baton))
			return_0;
	}

	return 1;
}

int lvmcache_foreach_mda(struct lvmcache_info *info,
			 int (*fun)(struct metadata_area *, void *),
			 void *baton)
{
	struct metadata_area *mda;
	dm_list_iterate_items(mda, &info->mdas) {
		if (!fun(mda, baton))
			return_0;
	}

	return 1;
}

unsigned lvmcache_mda_count(struct lvmcache_info *info)
{
	return dm_list_size(&info->mdas);
}

int lvmcache_foreach_da(struct lvmcache_info *info,
			int (*fun)(struct disk_locn *, void *),
			void *baton)
{
	struct data_area_list *da;
	dm_list_iterate_items(da, &info->das) {
		if (!fun(&da->disk_locn, baton))
			return_0;
	}

	return 1;
}

int lvmcache_foreach_ba(struct lvmcache_info *info,
			 int (*fun)(struct disk_locn *, void *),
			 void *baton)
{
	struct data_area_list *ba;
	dm_list_iterate_items(ba, &info->bas) {
		if (!fun(&ba->disk_locn, baton))
			return_0;
	}

	return 1;
}

struct label *lvmcache_get_dev_label(struct device *dev)
{
	struct lvmcache_info *info;

	if ((info = lvmcache_info_from_pvid(dev->pvid, NULL, 0))) {
		/* dev would be different for a duplicate */
		if (info->dev == dev)
			return info->label;
	}
	return NULL;
}

int lvmcache_has_dev_info(struct device *dev)
{
	if (lvmcache_info_from_pvid(dev->pvid, NULL, 0))
		return 1;
	return 0;
}

/*
 * The lifetime of the label returned is tied to the lifetime of the
 * lvmcache_info which is the same as lvmcache itself.
 */
struct label *lvmcache_get_label(struct lvmcache_info *info) {
	return info->label;
}

uint64_t lvmcache_device_size(struct lvmcache_info *info) {
	return info->device_size;
}

void lvmcache_set_device_size(struct lvmcache_info *info, uint64_t size) {
	info->device_size = size;
}

struct device *lvmcache_device(struct lvmcache_info *info) {
	return info->dev;
}
void lvmcache_set_ext_version(struct lvmcache_info *info, uint32_t version)
{
	info->ext_version = version;
}

uint32_t lvmcache_ext_version(struct lvmcache_info *info) {
	return info->ext_version;
}

void lvmcache_set_ext_flags(struct lvmcache_info *info, uint32_t flags) {
	info->ext_flags = flags;
}

uint32_t lvmcache_ext_flags(struct lvmcache_info *info) {
	return info->ext_flags;
}

int lvmcache_is_orphan(struct lvmcache_info *info) {
	if (!info->vginfo)
		return 1; /* FIXME? */
	return is_orphan_vg(info->vginfo->vgname);
}

int lvmcache_vgid_is_cached(const char *vgid) {
	struct lvmcache_vginfo *vginfo;

	vginfo = lvmcache_vginfo_from_vgid(vgid);

	if (!vginfo || !vginfo->vgname)
		return 0;

	if (is_orphan_vg(vginfo->vgname))
		return 0;

	return 1;
}

uint64_t lvmcache_smallest_mda_size(struct lvmcache_info *info)
{
	if (!info)
		return UINT64_C(0);

	return find_min_mda_size(&info->mdas);
}

const struct format_type *lvmcache_fmt(struct lvmcache_info *info) {
	return info->fmt;
}

int lvmcache_lookup_mda(struct lvmcache_vgsummary *vgsummary)
{
	struct lvmcache_vginfo *vginfo;

	if (!vgsummary->mda_size)
		return 0;

	/* FIXME Index the checksums */
	dm_list_iterate_items(vginfo, &_vginfos) {
		if (vgsummary->mda_checksum == vginfo->mda_checksum &&
		    vgsummary->mda_size == vginfo->mda_size &&
		    !is_orphan_vg(vginfo->vgname)) {
			vgsummary->vgname = vginfo->vgname;
			vgsummary->creation_host = vginfo->creation_host;
			vgsummary->vgstatus = vginfo->status;
			vgsummary->seqno = vginfo->seqno;
			/* vginfo->vgid has 1 extra byte then vgsummary->vgid */
			memcpy(&vgsummary->vgid, vginfo->vgid, sizeof(vgsummary->vgid));

			return 1;
		}
	}

	return 0;
}

int lvmcache_contains_lock_type_sanlock(struct cmd_context *cmd)
{
	struct lvmcache_vginfo *vginfo;

	dm_list_iterate_items(vginfo, &_vginfos) {
		if (vginfo->lock_type && !strcmp(vginfo->lock_type, "sanlock"))
			return 1;
	}

	return 0;
}

void lvmcache_get_max_name_lengths(struct cmd_context *cmd,
				   unsigned *pv_max_name_len,
				   unsigned *vg_max_name_len)
{
	struct lvmcache_vginfo *vginfo;
	struct lvmcache_info *info;
	unsigned len;

	*vg_max_name_len = 0;
	*pv_max_name_len = 0;

	dm_list_iterate_items(vginfo, &_vginfos) {
		len = strlen(vginfo->vgname);
		if (*vg_max_name_len < len)
			*vg_max_name_len = len;

		dm_list_iterate_items(info, &vginfo->infos) {
			len = strlen(dev_name(info->dev));
			if (*pv_max_name_len < len)
				*pv_max_name_len = len;
		}
	}
}

int lvmcache_vg_is_foreign(struct cmd_context *cmd, const char *vgname, const char *vgid)
{
	struct lvmcache_vginfo *vginfo;
	int ret = 0;

	if ((vginfo = lvmcache_vginfo_from_vgid(vgid)))
		ret = !is_system_id_allowed(cmd, vginfo->system_id);

	return ret;
}

/*
 * Example of reading four devs in sequence from the same VG:
 *
 * dev1:
 *    lvmcache: creates vginfo with initial values
 *
 * dev2: all checksums match.
 *    mda_header checksum matches vginfo from dev1
 *    metadata checksum matches vginfo from dev1
 *    metadata is not parsed, and the vgsummary values copied
 *    from lvmcache from dev1 and passed back to lvmcache for dev2.
 *    lvmcache: attach info for dev2 to existing vginfo
 *
 * dev3: mda_header and metadata have unmatching checksums.
 *    mda_header checksum matches vginfo from dev1
 *    metadata checksum doesn't match vginfo from dev1
 *    produces read error in config.c
 *    lvmcache: info for dev3 is deleted, FIXME: use a defective state
 *
 * dev4: mda_header and metadata have matching checksums, but
 *       does not match checksum in lvmcache from prev dev.
 *    mda_header checksum doesn't match vginfo from dev1
 *    lvmcache_lookup_mda returns 0, no vgname, no checksum_only
 *    lvmcache: update_vgname_and_id sees checksum from dev4 does not
 *    match vginfo from dev1, so vginfo->scan_summary_mismatch is set.
 *    attach info for dev4 to existing vginfo
 *
 * dev5: config parsing error.
 *    lvmcache: info for dev5 is deleted, FIXME: use a defective state
 */

int lvmcache_scan_mismatch(struct cmd_context *cmd, const char *vgname, const char *vgid)
{
	struct lvmcache_vginfo *vginfo;

	if (!vgname || !vgid)
		return 1;

	if ((vginfo = lvmcache_vginfo_from_vgid(vgid)))
		return vginfo->scan_summary_mismatch;

	return 1;
}

