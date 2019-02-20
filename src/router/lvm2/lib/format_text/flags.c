/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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
#include "lib/metadata/metadata.h"
#include "import-export.h"
#include "lib/misc/lvm-string.h"

/*
 * Bitsets held in the 'status' flags get
 * converted into arrays of strings.
 */
struct flag {
	const uint64_t mask;
	const char *description;
	int kind;
};

static const struct flag _vg_flags[] = {
	{EXPORTED_VG, "EXPORTED", STATUS_FLAG},
	{RESIZEABLE_VG, "RESIZEABLE", STATUS_FLAG},
	{PVMOVE, "PVMOVE", STATUS_FLAG},
	{LVM_READ, "READ", STATUS_FLAG},
	{LVM_WRITE, "WRITE", STATUS_FLAG},
	{LVM_WRITE_LOCKED, "WRITE_LOCKED", COMPATIBLE_FLAG},
	{CLUSTERED, "CLUSTERED", STATUS_FLAG},
	{SHARED, "SHARED", STATUS_FLAG},
	{PARTIAL_VG, NULL, 0},
	{PRECOMMITTED, NULL, 0},
	{ARCHIVED_VG, NULL, 0},
	{0, NULL, 0}
};

static const struct flag _pv_flags[] = {
	{ALLOCATABLE_PV, "ALLOCATABLE", STATUS_FLAG},
	{EXPORTED_VG, "EXPORTED", STATUS_FLAG},
	{MISSING_PV, "MISSING", COMPATIBLE_FLAG},
	{MISSING_PV, "MISSING", STATUS_FLAG},
	{PV_MOVED_VG, NULL, 0},
	{UNLABELLED_PV, NULL, 0},
	{0, NULL, 0}
};

static const struct flag _lv_flags[] = {
	{LVM_READ, "READ", STATUS_FLAG},
	{LVM_WRITE, "WRITE", STATUS_FLAG},
	{LVM_WRITE_LOCKED, "WRITE_LOCKED", COMPATIBLE_FLAG},
	{FIXED_MINOR, "FIXED_MINOR", STATUS_FLAG},
	{VISIBLE_LV, "VISIBLE", STATUS_FLAG},
	{PVMOVE, "PVMOVE", STATUS_FLAG},
	{LOCKED, "LOCKED", STATUS_FLAG},
	{LV_NOTSYNCED, "NOTSYNCED", STATUS_FLAG},
	{LV_REBUILD, "REBUILD", STATUS_FLAG},
	{LV_RESHAPE, "RESHAPE", SEGTYPE_FLAG},
	{LV_RESHAPE_DATA_OFFSET, "RESHAPE_DATA_OFFSET", SEGTYPE_FLAG},
	{LV_RESHAPE_DELTA_DISKS_PLUS, "RESHAPE_DELTA_DISKS_PLUS", SEGTYPE_FLAG},
	{LV_RESHAPE_DELTA_DISKS_MINUS, "RESHAPE_DELTA_DISKS_MINUS", SEGTYPE_FLAG},
	{LV_REMOVE_AFTER_RESHAPE, "REMOVE_AFTER_RESHAPE", SEGTYPE_FLAG},
	{LV_WRITEMOSTLY, "WRITEMOSTLY", STATUS_FLAG},
	{LV_ACTIVATION_SKIP, "ACTIVATION_SKIP", COMPATIBLE_FLAG},
	{LV_ERROR_WHEN_FULL, "ERROR_WHEN_FULL", COMPATIBLE_FLAG},
	{LV_METADATA_FORMAT, "METADATA_FORMAT", SEGTYPE_FLAG},
	{LV_NOSCAN, NULL, 0},
	{LV_TEMPORARY, NULL, 0},
	{POOL_METADATA_SPARE, NULL, 0},
	{LOCKD_SANLOCK_LV, NULL, 0},
	{RAID, NULL, 0},
	{RAID_META, NULL, 0},
	{RAID_IMAGE, NULL, 0},
	{MIRROR, NULL, 0},
	{MIRROR_IMAGE, NULL, 0},
	{MIRROR_LOG, NULL, 0},
	{MIRRORED, NULL, 0},
	{VIRTUAL, NULL, 0},
	{SNAPSHOT, NULL, 0},
	{MERGING, NULL, 0},
	{CONVERTING, NULL, 0},
	{PARTIAL_LV, NULL, 0},
	{POSTORDER_FLAG, NULL, 0},
	{VIRTUAL_ORIGIN, NULL, 0},
	{THIN_VOLUME, NULL, 0},
	{THIN_POOL, NULL, 0},
	{THIN_POOL_DATA, NULL, 0},
	{THIN_POOL_METADATA, NULL, 0},
	{CACHE, NULL, 0},
	{CACHE_POOL, NULL, 0},
	{CACHE_POOL_DATA, NULL, 0},
	{CACHE_POOL_METADATA, NULL, 0},
	{LV_VDO, NULL, 0},
	{LV_VDO_POOL, NULL, 0},
	{LV_VDO_POOL_DATA, NULL, 0},
	{LV_PENDING_DELETE, NULL, 0}, /* FIXME Display like COMPATIBLE_FLAG */
	{LV_REMOVED, NULL, 0},
	{0, NULL, 0}
};

static const struct flag *_get_flags(enum pv_vg_lv_e type)
{
	switch (type) {
	case VG_FLAGS:
		return _vg_flags;

	case PV_FLAGS:
		return _pv_flags;

	case LV_FLAGS:
		return _lv_flags;
	}

	log_error(INTERNAL_ERROR "Unknown flag set requested.");
	return NULL;
}

/*
 * Converts a bitset to an array of string values,
 * using one of the tables defined at the top of
 * the file.
 */
int print_flags(char *buffer, size_t size, enum pv_vg_lv_e type, int mask, uint64_t status)
{
	int f, first = 1;
	const struct flag *flags;

	if (!(flags = _get_flags(type)))
		return_0;

	if (!emit_to_buffer(&buffer, &size, "["))
		return_0;

	for (f = 0; flags[f].mask; f++) {
		if (status & flags[f].mask) {
			status &= ~flags[f].mask;

			if (mask != flags[f].kind)
				continue;

			/* Internal-only flag? */
			if (!flags[f].description)
				continue;

			if (!first) {
				if (!emit_to_buffer(&buffer, &size, ", "))
					return_0;
			} else
				first = 0;
	
			if (!emit_to_buffer(&buffer, &size, "\"%s\"",
					    flags[f].description))
				return_0;
		}
	}

	if (!emit_to_buffer(&buffer, &size, "]"))
		return_0;

	if (status)
		log_warn(INTERNAL_ERROR "Metadata inconsistency: "
			 "Not all flags successfully exported.");

	return 1;
}

int read_flags(uint64_t *status, enum pv_vg_lv_e type, int mask, const struct dm_config_value *cv)
{
	unsigned f;
	uint64_t s = UINT64_C(0);
	const struct flag *flags;

	if (!(flags = _get_flags(type)))
		return_0;

	if (cv->type == DM_CFG_EMPTY_ARRAY)
		goto out;

	while (cv) {
		if (cv->type != DM_CFG_STRING) {
			log_error("Status value is not a string.");
			return 0;
		}

		for (f = 0; flags[f].description; f++)
			if ((flags[f].kind & mask) &&
			    !strcmp(flags[f].description, cv->v.str)) {
				s |= flags[f].mask;
				break;
			}

		if (type == VG_FLAGS && !strcmp(cv->v.str, "PARTIAL")) {
			/*
			 * Exception: We no longer write this flag out, but it
			 * might be encountered in old backup files, so restore
			 * it in that case. It is never part of live metadata
			 * though, so only vgcfgrestore needs to be concerned
			 * by this case.
			 */
			s |= PARTIAL_VG;
		} else if (!flags[f].description && (mask & STATUS_FLAG)) {
			log_error("Unknown status flag '%s'.", cv->v.str);
			return 0;
		}

		cv = cv->next;
	}

      out:
	*status |= s;
	return 1;
}

/*
 * Parse extra status flags from segment "type" string.
 * These flags are seen as INCOMPATIBLE by any older lvm2 code.
 * All flags separated by '+' are trimmed from passed string.
 * All UNKNOWN flags will again cause the "UNKNOWN" segtype.
 *
 * Note: using these segtype status flags instead of actual
 * status flags ensures wanted incompatiblity.
 */
int read_segtype_lvflags(uint64_t *status, char *segtype_str)
{
	unsigned i;
	const struct flag *flags = _lv_flags;
	char *delim;
	char *flag, *buffer, *str;

	if (!(str = strchr(segtype_str, '+')))
		return 1; /* No flags */

	if (!(buffer = strdup(str + 1))) {
		log_error("Cannot duplicate segment string.");
		return 0;
	}

	delim = buffer;

	do {
		flag = delim;
		if ((delim = strchr(delim, '+')))
			*delim++ = '\0';

		for (i = 0; flags[i].description; i++)
			if ((flags[i].kind & SEGTYPE_FLAG) &&
			    !strcmp(flags[i].description, flag)) {
				*status |= flags[i].mask;
				break;
			}

	} while (delim && flags[i].description); /* Till no more flags in type appear */

	if (!flags[i].description)
		/* Unknown flag is incompatible - returns unmodified segtype_str */
		log_warn("WARNING: Unrecognised flag %s in segment type %s.",
			 flag, segtype_str);
	else
		*str = '\0'; /* Cut away 1st. '+' */

	free(buffer);

	return 1;
}

int print_segtype_lvflags(char *buffer, size_t size, uint64_t status)
{
	unsigned i;
	const struct flag *flags = _lv_flags;

	buffer[0] = 0;
	for (i = 0; flags[i].mask; i++)
		if ((flags[i].kind & SEGTYPE_FLAG) &&
		    (status & flags[i].mask) &&
		    !emit_to_buffer(&buffer, &size, "+%s",
				    flags[i].description))
			return 0;

	return 1;
}
