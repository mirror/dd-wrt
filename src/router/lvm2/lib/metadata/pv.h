/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2010 Red Hat, Inc. All rights reserved.
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
#ifndef _LVM_PV_H
#define _LVM_PV_H

#include "lib/uuid/uuid.h"
#include "device_mapper/all.h"

struct device;
struct format_type;
struct volume_group;

struct physical_volume {
	struct id id;
	struct id old_id;		/* Set during pvchange -u. */
	struct device *dev;
	const struct format_type *fmt;
	struct format_instance *fid;

	/*
	 * vg_name and vgid are used before the parent VG struct exists.
	 * FIXME: Investigate removal/substitution with 'vg' fields.
	 */
	const char *vg_name;
	struct id vgid;

	/*
	 * 'vg' is set and maintained when the PV belongs to a 'pvs'
	 * list in a parent VG struct.
	 */
	struct volume_group *vg;

	uint64_t status;
	uint64_t size;

	/* bootloader area */
	uint64_t ba_start;
	uint64_t ba_size;

	/* physical extents */
	uint32_t pe_size;
	uint64_t pe_start;
	uint32_t pe_count;
	uint32_t pe_alloc_count;
	unsigned long pe_align;
	unsigned long pe_align_offset;

        /* This is true whenever the represented PV has a label associated. */
        uint64_t is_labelled:1;

        /* NB. label_sector is valid whenever is_labelled is true */
	uint64_t label_sector;

	struct dm_list segments;	/* Ordered pv_segments covering complete PV */
	struct dm_list tags;
};

char *pv_fmt_dup(const struct physical_volume *pv);
char *pv_name_dup(struct dm_pool *mem, const struct physical_volume *pv);
struct device *pv_dev(const struct physical_volume *pv);
const char *pv_vg_name(const struct physical_volume *pv);
char *pv_attr_dup(struct dm_pool *mem, const struct physical_volume *pv);
const char *pv_dev_name(const struct physical_volume *pv);
char *pv_uuid_dup(struct dm_pool *mem, const struct physical_volume *pv);
char *pv_tags_dup(const struct physical_volume *pv);
uint64_t pv_size(const struct physical_volume *pv);
uint64_t pv_size_field(const struct physical_volume *pv);
uint64_t pv_dev_size(const struct physical_volume *pv);
uint64_t pv_free(const struct physical_volume *pv);
uint64_t pv_status(const struct physical_volume *pv);
uint32_t pv_pe_size(const struct physical_volume *pv);
uint64_t pv_pe_start(const struct physical_volume *pv);
uint64_t pv_ba_start(const struct physical_volume *pv);
uint64_t pv_ba_size(const struct physical_volume *pv);
uint32_t pv_pe_count(const struct physical_volume *pv);
uint32_t pv_pe_alloc_count(const struct physical_volume *pv);
uint64_t pv_mda_size(const struct physical_volume *pv);
struct lvmcache_info;
uint64_t lvmcache_info_mda_free(struct lvmcache_info *info);
uint64_t pv_mda_free(const struct physical_volume *pv);
uint64_t pv_used(const struct physical_volume *pv);
uint32_t pv_mda_count(const struct physical_volume *pv);
uint32_t pv_mda_used_count(const struct physical_volume *pv);
unsigned pv_mda_set_ignored(const struct physical_volume *pv, unsigned mda_ignored);
int is_orphan(const struct physical_volume *pv);
int is_missing_pv(const struct physical_volume *pv);
int is_used_pv(const struct physical_volume *pv);
int is_pv(const struct physical_volume *pv);
struct label *pv_label(const struct physical_volume *pv);

#endif /* _LVM_PV_H */
