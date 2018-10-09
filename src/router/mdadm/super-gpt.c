/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2010 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neil@brown.name>
 *
 */

/*
 * 'gpt' is a pseudo metadata type for devices which have a
 * GPT partition table.
 *
 * Obviously arrays cannot be created or assembled for this type.
 * It is used to allow a new bare device to have an partition table
 * added so the member partitions can then be included in other
 * arrays as relevant.
 *
 * The meaning operations are:
 * examine_super, but not brief_examine_super or export_examine
 * load_super
 * store_super
 */

#include "mdadm.h"
#include "part.h"

static void free_gpt(struct supertype *st)
{
	free(st->sb);
	st->sb = NULL;
}

static void examine_gpt(struct supertype *st, char *homehost)
{
	struct GPT *gpt = st->sb + 512;
	struct GPT_part_entry *gpe = st->sb + 1024;
	unsigned int i;

	printf("    GPT Magic : %llx\n", (unsigned long long)__le64_to_cpu(gpt->magic));
	printf(" GPT Revision : %ld\n", (long)__le32_to_cpu(gpt->revision));
	for (i = 0; i < __le32_to_cpu(gpt->part_cnt); i++) {
		printf("  Partition[%02d] : %12llu sectors at %12llu\n",
		       i,
		       (unsigned long long)__le64_to_cpu(gpe[i].starting_lba),
		       (unsigned long long)__le64_to_cpu(gpe[i].ending_lba)-
		       (unsigned long long)__le64_to_cpu(gpe[i].starting_lba)
		       +1
			);
	}
}

static int load_gpt(struct supertype *st, int fd, char *devname)
{
	struct MBR *super;
	struct GPT *gpt_head;
	int to_read;
	unsigned int sector_size;

	free_gpt(st);

	if (posix_memalign((void**)&super, 4096, 32*512) != 0) {
		pr_err("could not allocate superblock\n");
		return 1;
	}

	if (!get_dev_sector_size(fd, devname, &sector_size)) {
		free(super);
		return 1;
	}

	lseek(fd, 0, 0);
	if (read(fd, super, sizeof(*super)) != sizeof(*super)) {
	no_read:
		if (devname)
			pr_err("Cannot read partition table on %s\n",
				devname);
		free(super);
		return 1;
	}

	if (super->magic != MBR_SIGNATURE_MAGIC ||
	    super->parts[0].part_type != MBR_GPT_PARTITION_TYPE) {
	not_found:
		if (devname)
			pr_err("No partition table found on %s\n",
				devname);
		free(super);
		return 1;
	}
	/* Set offset to second block (GPT header) */
	lseek(fd, sector_size, SEEK_SET);
	/* Seem to have GPT, load the header */
	gpt_head = (struct GPT*)(super+1);
	if (read(fd, gpt_head, sizeof(*gpt_head)) != sizeof(*gpt_head))
		goto no_read;
	if (gpt_head->magic != GPT_SIGNATURE_MAGIC)
		goto not_found;
	if (__le32_to_cpu(gpt_head->part_cnt) >= 128)
		goto not_found;

	to_read = __le32_to_cpu(gpt_head->part_cnt) * sizeof(struct GPT_part_entry);
	to_read =  ((to_read+511)/512) * 512;
	/* Set offset to third block (GPT entries) */
	lseek(fd, sector_size*2, SEEK_SET);
	if (read(fd, gpt_head+1, to_read) != to_read)
		goto no_read;

	st->sb = super;

	if (st->ss == NULL) {
		st->ss = &gpt;
		st->minor_version = 0;
		st->max_devs = 1;
		st->info = NULL;
	}
	return 0;
}

static int store_gpt(struct supertype *st, int fd)
{
	/* FIXME should I save the boot loader */
	/* need to write two copies! */
	/* FIXME allow for blocks != 512 bytes
	 *etc
	 */
	struct MBR *super = st->sb;
	struct GPT *gpt;
	int to_write;

	gpt = (struct GPT*)(super+1);

	to_write = __le32_to_cpu(gpt->part_cnt) * sizeof(struct GPT_part_entry);
	to_write =  ((to_write+511)/512) * 512;

	lseek(fd, 0, 0);
	if (write(fd, st->sb, to_write) != to_write)
		return 4;

	fsync(fd);
	ioctl(fd, BLKRRPART, 0);
	return 0;
}

static void getinfo_gpt(struct supertype *st, struct mdinfo *info, char *map)
{
	struct GPT *gpt = st->sb + 512;
	struct GPT_part_entry *gpe = st->sb + 1024;
	unsigned int i;

	memset(&info->array, 0, sizeof(info->array));
	memset(&info->disk, 0, sizeof(info->disk));
	strcpy(info->text_version, "gpt");
	strcpy(info->name, "gpt");
	info->component_size = 0;

	for (i = 0; i < __le32_to_cpu(gpt->part_cnt); i++) {
		unsigned long long last =
			(unsigned long long)__le64_to_cpu(gpe[i].ending_lba);
		if (last > info->component_size)
			info->component_size = last;
	}
}

static struct supertype *match_metadata_desc(char *arg)
{
	struct supertype *st = xmalloc(sizeof(*st));

	if (!st)
		return st;
	if (strcmp(arg, "gpt") != 0) {
		free(st);
		return NULL;
	}

	st->ss = &gpt;
	st->info = NULL;
	st->minor_version = 0;
	st->max_devs = 1;
	st->sb = NULL;
	return st;
}

static int validate_geometry(struct supertype *st, int level,
			     int layout, int raiddisks,
			     int *chunk, unsigned long long size,
			     unsigned long long data_offset,
			     char *subdev, unsigned long long *freesize,
			     int consistency_policy, int verbose)
{
	pr_err("gpt metadata cannot be used this way\n");
	return 0;
}

struct superswitch gpt = {
	.examine_super = examine_gpt,
	.validate_geometry = validate_geometry,
	.match_metadata_desc = match_metadata_desc,
	.load_super = load_gpt,
	.store_super = store_gpt,
	.getinfo_super = getinfo_gpt,
	.free_super = free_gpt,
	.name = "gpt",
};
