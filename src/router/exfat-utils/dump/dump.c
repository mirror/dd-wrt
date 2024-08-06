// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2021 Namjae Jeon <linkinjeon@kernel.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <locale.h>
#include <inttypes.h>

#include "exfat_ondisk.h"
#include "libexfat.h"

#define EXFAT_MIN_SECT_SIZE_BITS		9
#define EXFAT_MAX_SECT_SIZE_BITS		12
#define BITS_PER_BYTE				8
#define BITS_PER_BYTE_MASK			0x7

static const unsigned char used_bit[] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3,/*  0 ~  19*/
	2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4,/* 20 ~  39*/
	2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5,/* 40 ~  59*/
	4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,/* 60 ~  79*/
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4,/* 80 ~  99*/
	3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,/*100 ~ 119*/
	4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4,/*120 ~ 139*/
	3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,/*140 ~ 159*/
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5,/*160 ~ 179*/
	4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5,/*180 ~ 199*/
	3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6,/*200 ~ 219*/
	5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,/*220 ~ 239*/
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8             /*240 ~ 255*/
};

static void usage(void)
{
	fprintf(stderr, "Usage: dump.exfat\n");
	fprintf(stderr, "\t-V | --version                        Show version\n");
	fprintf(stderr, "\t-h | --help                           Show help\n");

	exit(EXIT_FAILURE);
}

static struct option opts[] = {
	{"version",		no_argument,		NULL,	'V' },
	{"help",		no_argument,		NULL,	'h' },
	{"?",			no_argument,		NULL,	'?' },
	{NULL,			0,			NULL,	 0  }
};

static unsigned int exfat_count_used_clusters(unsigned char *bitmap,
		unsigned long long bitmap_len)
{
	unsigned int count = 0;
	unsigned long long i;

	for (i = 0; i < bitmap_len; i++)
		count += used_bit[bitmap[i]];

	return count;
}

static int exfat_show_ondisk_all_info(struct exfat_blk_dev *bd)
{
	struct pbr *ppbr;
	struct bsx64 *pbsx;
	struct exfat_dentry *ed;
	unsigned int root_clu_off, bitmap_clu_off, bitmap_clu;
	unsigned int total_clus, used_clus, clu_offset, root_clu;
	unsigned long long bitmap_len;
	int ret;
	unsigned char *bitmap;
	char *volume_label;

	ppbr = malloc(EXFAT_MAX_SECTOR_SIZE);
	if (!ppbr) {
		exfat_err("Cannot allocate pbr: out of memory\n");
		return -ENOMEM;
	}

	/* read main boot sector */
	if (exfat_read(bd->dev_fd, (char *)ppbr, EXFAT_MAX_SECTOR_SIZE,
			0) != (ssize_t)EXFAT_MAX_SECTOR_SIZE) {
		exfat_err("main boot sector read failed\n");
		ret = -EIO;
		goto free_ppbr;
	}

	if (memcmp(ppbr->bpb.oem_name, "EXFAT   ", 8) != 0) {
		exfat_err("Bad fs_name in boot sector, which does not describe a valid exfat filesystem\n");
		ret = -EINVAL;
		goto free_ppbr;
	}

	pbsx = &ppbr->bsx;

	if (pbsx->sect_size_bits < EXFAT_MIN_SECT_SIZE_BITS ||
	    pbsx->sect_size_bits > EXFAT_MAX_SECT_SIZE_BITS) {
		exfat_err("bogus sector size bits : %u\n",
				pbsx->sect_size_bits);
		ret = -EINVAL;
		goto free_ppbr;
	}

	if (pbsx->sect_per_clus_bits > 25 - pbsx->sect_size_bits) {
		exfat_err("bogus sectors bits per cluster : %u\n",
				pbsx->sect_per_clus_bits);
		ret = -EINVAL;
		goto free_ppbr;
	}

	bd->sector_size_bits = pbsx->sect_size_bits;
	bd->sector_size = 1 << pbsx->sect_size_bits;

	clu_offset = le32_to_cpu(pbsx->clu_offset);
	total_clus = le32_to_cpu(pbsx->clu_count);
	root_clu = le32_to_cpu(pbsx->root_cluster);

	exfat_info("-------------- Dump Boot sector region --------------\n");
	exfat_info("Volume Length(sectors): \t\t%" PRIu64 "\n",
			le64_to_cpu(pbsx->vol_length));
	exfat_info("FAT Offset(sector offset): \t\t%u\n",
			le32_to_cpu(pbsx->fat_offset));
	exfat_info("FAT Length(sectors): \t\t\t%u\n",
			le32_to_cpu(pbsx->fat_length));
	exfat_info("Cluster Heap Offset (sector offset): \t%u\n", clu_offset);
	exfat_info("Cluster Count: \t\t\t\t%u\n", total_clus);
	exfat_info("Root Cluster (cluster offset): \t\t%u\n", root_clu);
	exfat_info("Volume Serial: \t\t\t\t0x%x\n", le32_to_cpu(pbsx->vol_serial));
	exfat_info("Bytes per Sector: \t\t\t%u\n", 1 << pbsx->sect_size_bits);
	exfat_info("Sectors per Cluster: \t\t\t%u\n\n", 1 << pbsx->sect_per_clus_bits);

	bd->cluster_size =
		1 << (pbsx->sect_per_clus_bits + pbsx->sect_size_bits);
	root_clu_off = exfat_clus_to_blk_dev_off(bd, clu_offset, root_clu);

	ed = malloc(sizeof(struct exfat_dentry)*3);
	if (!ed) {
		exfat_err("failed to allocate memory\n");
		ret = -ENOMEM;
		goto free_ppbr;
	}

	ret = exfat_read(bd->dev_fd, ed, sizeof(struct exfat_dentry)*3,
			root_clu_off);
	if (ret < 0) {
		exfat_err("bitmap entry read failed: %d\n", errno);
		ret = -EIO;
		goto free_entry;
	}

	volume_label = exfat_conv_volume_label(&ed[0]);
	if (!volume_label) {
		ret = -EINVAL;
		goto free_entry;
	}

	bitmap_clu = le32_to_cpu(ed[1].bitmap_start_clu);
	bitmap_clu_off = exfat_clus_to_blk_dev_off(bd, clu_offset,
			bitmap_clu);
	bitmap_len = le64_to_cpu(ed[1].bitmap_size);

	exfat_info("----------------- Dump Root entries -----------------\n");
	exfat_info("Volume entry type: \t\t\t0x%x\n", ed[0].type);
	exfat_info("Volume label: \t\t\t\t%s\n", volume_label);
	exfat_info("Volume label character count: \t\t%u\n", ed[0].vol_char_cnt);

	exfat_info("Bitmap entry type: \t\t\t0x%x\n", ed[1].type);
	exfat_info("Bitmap start cluster: \t\t\t%x\n", bitmap_clu);
	exfat_info("Bitmap size: \t\t\t\t%llu\n", bitmap_len);

	exfat_info("Upcase table entry type: \t\t0x%x\n", ed[2].type);
	exfat_info("Upcase table start cluster: \t\t%x\n",
			le32_to_cpu(ed[2].upcase_start_clu));
	exfat_info("Upcase table size: \t\t\t%" PRIu64 "\n\n",
			le64_to_cpu(ed[2].upcase_size));

	bitmap = malloc(bitmap_len);
	if (!bitmap) {
		exfat_err("bitmap allocation failed\n");
		ret = -ENOMEM;
		goto free_volume_label;
	}

	ret = exfat_read(bd->dev_fd, bitmap, bitmap_len, bitmap_clu_off);
	if (ret < 0) {
		exfat_err("bitmap entry read failed: %d\n", errno);
		ret = -EIO;
		free(bitmap);
		goto free_volume_label;
	}

	total_clus = le32_to_cpu(pbsx->clu_count);
	used_clus = exfat_count_used_clusters(bitmap, bitmap_len);

	exfat_info("---------------- Show the statistics ----------------\n");
	exfat_info("Cluster size:  \t\t\t\t%u\n", bd->cluster_size);
	exfat_info("Total Clusters: \t\t\t%u\n", total_clus);
	exfat_info("Free Clusters: \t\t\t\t%u\n", total_clus-used_clus);
	ret = 0;

	free(bitmap);

free_volume_label:
	free(volume_label);
free_entry:
	free(ed);
free_ppbr:
	free(ppbr);
	return ret;
}

int main(int argc, char *argv[])
{
	int c;
	int ret = EXIT_FAILURE;
	struct exfat_blk_dev bd;
	struct exfat_user_input ui;
	bool version_only = false;

	init_user_input(&ui);
	ui.writeable = false;

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "iVh", opts, NULL)) != EOF)
		switch (c) {
		case 'V':
			version_only = true;
			break;
		case '?':
		case 'h':
		default:
			usage();
	}

	show_version();
	if (version_only)
		exit(EXIT_FAILURE);

	if (argc < 2)
		usage();

	memset(ui.dev_name, 0, sizeof(ui.dev_name));
	snprintf(ui.dev_name, sizeof(ui.dev_name), "%s", argv[1]);

	ret = exfat_get_blk_dev_info(&ui, &bd);
	if (ret < 0)
		goto out;

	ret = exfat_show_ondisk_all_info(&bd);
	close(bd.dev_fd);

out:
	return ret;
}
