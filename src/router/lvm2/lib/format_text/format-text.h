/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_FORMAT_TEXT_H
#define _LVM_FORMAT_TEXT_H

#include "lib/metadata/metadata.h"

#define FMT_TEXT_NAME "lvm2"
#define FMT_TEXT_ALIAS "text"
#define FMT_TEXT_ORPHAN_VG_NAME ORPHAN_VG_NAME(FMT_TEXT_NAME)
#define FMT_TEXT_MAX_MDAS_PER_PV 2

/*
 * Archives a vg config.  'retain_days' is the minimum number of
 * days that an archive file must be held for.  'min_archives' is
 * the minimum number of archives required to be kept for each
 * volume group.
 */
int archive_vg(struct volume_group *vg,
	       const char *dir,
	       const char *desc, uint32_t retain_days, uint32_t min_archive);

/*
 * Displays a list of vg backups in a particular archive directory.
 */
int archive_list(struct cmd_context *cmd, const char *dir, const char *vgname);
int archive_list_file(struct cmd_context *cmd, const char *file);
int backup_list(struct cmd_context *cmd, const char *dir, const char *vgname);

/*
 * The text format can read and write a volume_group to a file.
 */
struct text_context {
	const char *path_live;	/* Path to file holding live metadata */
	const char *path_edit;	/* Path to file holding edited metadata */
	const char *desc;	/* Description placed inside file */
};
struct format_type *create_text_format(struct cmd_context *cmd);

struct labeller *text_labeller_create(const struct format_type *fmt);

int pvhdr_read(struct device *dev, char *buf);

int add_da(struct dm_pool *mem, struct dm_list *das,
	   uint64_t start, uint64_t size);
void del_das(struct dm_list *das);
int add_ba(struct dm_pool *mem, struct dm_list *eas,
	   uint64_t start, uint64_t size);
void del_bas(struct dm_list *bas);
int add_mda(const struct format_type *fmt, struct dm_pool *mem, struct dm_list *mdas,
	    struct device *dev, uint64_t start, uint64_t size, unsigned ignored);
void del_mdas(struct dm_list *mdas);

/* On disk */
struct disk_locn {
	uint64_t offset;	/* Offset in bytes to start sector */
	uint64_t size;		/* Bytes */
} __attribute__ ((packed));

/* Data areas (holding PEs) */
struct data_area_list {
	struct dm_list list;
	struct disk_locn disk_locn;
};

#endif
