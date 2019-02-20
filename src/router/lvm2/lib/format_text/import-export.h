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

#ifndef _LVM_TEXT_IMPORT_EXPORT_H
#define _LVM_TEXT_IMPORT_EXPORT_H

#include "lib/config/config.h"
#include "lib/metadata/metadata.h"
#include "lib/cache/lvmcache.h"

#include <stdio.h>

/*
 * Constants to identify files this code can parse.
 */
#define CONTENTS_FIELD "contents"
#define CONTENTS_VALUE "Text Format Volume Group"

#define FORMAT_VERSION_FIELD "version"
#define FORMAT_VERSION_VALUE 1

/*
 * VGs, PVs and LVs all have status bitsets, we gather together
 * common code for reading and writing them.
 */
enum pv_vg_lv_e {
	PV_FLAGS = 1,
	VG_FLAGS,
	LV_FLAGS,
};

#define COMPATIBLE_FLAG	0x01
#define STATUS_FLAG	0x02
#define SEGTYPE_FLAG	0x04

struct text_vg_version_ops {
	int (*check_version) (const struct dm_config_tree * cf);
	struct volume_group *(*read_vg) (struct format_instance * fid,
					 const struct dm_config_tree *cf,
					 unsigned allow_lvmetad_extensions);
	void (*read_desc) (struct dm_pool * mem, const struct dm_config_tree *cf,
			   time_t *when, char **desc);
	int (*read_vgsummary) (const struct format_type *fmt,
			       const struct dm_config_tree *cft,
			       struct lvmcache_vgsummary *vgsummary);
};

struct text_vg_version_ops *text_vg_vsn1_init(void);

int print_flags(char *buffer, size_t size, enum pv_vg_lv_e type, int mask, uint64_t status);
int read_flags(uint64_t *status, enum pv_vg_lv_e type, int mask, const struct dm_config_value *cv);

int print_segtype_lvflags(char *buffer, size_t size, uint64_t status);
int read_segtype_lvflags(uint64_t *status, char *segtype_str);

int text_vg_export_file(struct volume_group *vg, const char *desc, FILE *fp);
size_t text_vg_export_raw(struct volume_group *vg, const char *desc, char **buf);
struct volume_group *text_read_metadata_file(struct format_instance *fid,
					 const char *file,
					 time_t *when, char **desc);
struct volume_group *text_read_metadata(struct format_instance *fid,
				       const char *file,
				       struct cached_vg_fmtdata **vg_fmtdata,
				       unsigned *use_previous_vg,
				       struct device *dev, int primary_mda,
				       off_t offset, uint32_t size,
				       off_t offset2, uint32_t size2,
				       checksum_fn_t checksum_fn,
				       uint32_t checksum,
				       time_t *when, char **desc);

int text_read_metadata_summary(const struct format_type *fmt,
		       struct device *dev, dev_io_reason_t reason,
		       off_t offset, uint32_t size,
		       off_t offset2, uint32_t size2,
		       checksum_fn_t checksum_fn,
		       int checksum_only,
		       struct lvmcache_vgsummary *vgsummary);

#endif
