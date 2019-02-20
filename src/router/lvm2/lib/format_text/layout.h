/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_TEXT_LAYOUT_H
#define _LVM_TEXT_LAYOUT_H

#include "lib/config/config.h"
#include "lib/metadata/metadata.h"
#include "lib/cache/lvmcache.h"
#include "lib/uuid/uuid.h"

/* disk_locn and data_area_list are defined in format-text.h */

/*
 * PV header extension versions:
 *   - version 1: bootloader area support
 *   - version 2: PV_EXT_USED flag support
 */
#define PV_HEADER_EXTENSION_VSN 2

struct pv_header_extension {
	uint32_t version;
	uint32_t flags;
	/* NULL-terminated list of bootloader areas */
	struct disk_locn bootloader_areas_xl[0];
} __attribute__ ((packed));

/* Fields with the suffix _xl should be xlate'd wherever they appear */
/* On disk */
struct pv_header {
	int8_t pv_uuid[ID_LEN];

	/* This size can be overridden if PV belongs to a VG */
	uint64_t device_size_xl;	/* Bytes */

	/* NULL-terminated list of data areas followed by */
	/* NULL-terminated list of metadata area headers */
	struct disk_locn disk_areas_xl[0];	/* Two lists */
} __attribute__ ((packed));

/*
 * Ignore this raw location.  This allows us to
 * ignored metadata areas easily, and thus balance
 * metadata across VGs with many PVs.
 */
#define RAW_LOCN_IGNORED 0x00000001

/* On disk */
struct raw_locn {
	uint64_t offset;	/* Offset in bytes to start sector */
	uint64_t size;		/* Bytes */
	uint32_t checksum;
	uint32_t flags;
} __attribute__ ((packed));

int rlocn_is_ignored(const struct raw_locn *rlocn);
void rlocn_set_ignored(struct raw_locn *rlocn, unsigned mda_ignored);

/* On disk */
/* Structure size limited to one sector */
struct mda_header {
	uint32_t checksum_xl;	/* Checksum of rest of mda_header */
	int8_t magic[16];	/* To aid scans for metadata */
	uint32_t version;
	uint64_t start;		/* Absolute start byte of mda_header */
	uint64_t size;		/* Size of metadata area */

	struct raw_locn raw_locns[0];	/* NULL-terminated list */
} __attribute__ ((packed));

struct mda_header *raw_read_mda_header(const struct format_type *fmt,
				       struct device_area *dev_area, int primary_mda);

struct mda_lists {
	struct metadata_area_ops *file_ops;
	struct metadata_area_ops *raw_ops;
};

struct mda_context {
	struct device_area area;
	uint64_t free_sectors;
	struct raw_locn rlocn;	/* Store inbetween write and commit */
};

/* FIXME Convert this at runtime */
#define FMTT_MAGIC "\040\114\126\115\062\040\170\133\065\101\045\162\060\116\052\076"
#define FMTT_VERSION 1
#define MDA_HEADER_SIZE 512
#define LVM2_LABEL "LVM2 001"
#define MDA_SIZE_MIN (8 * (unsigned) lvm_getpagesize())
#define MDA_ORIGINAL_ALIGNMENT 512	/* Original alignment used for start of VG metadata content */

int read_metadata_location_summary(const struct format_type *fmt, struct mda_header *mdah, int primary_mda, 
		    struct device_area *dev_area, struct lvmcache_vgsummary *vgsummary,
		    uint64_t *mda_free_sectors);

#endif
