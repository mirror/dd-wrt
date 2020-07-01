// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <locale.h>

#include "exfat_ondisk.h"
#include "libexfat.h"

static void usage(void)
{
	fprintf(stderr, "Usage: tune.exfat\n");
	fprintf(stderr, "\t-l | --print-label                    Print volume label\n");
	fprintf(stderr, "\t-L | --volume-label=label             Set volume label\n");
	fprintf(stderr, "\t-V | --version                        Show version\n");
	fprintf(stderr, "\t-v | --verbose                        Print debug\n");
	fprintf(stderr, "\t-h | --help                           Show help\n");

	exit(EXIT_FAILURE);
}

static struct option opts[] = {
	{"print-label",		no_argument,		NULL,	'l' },
	{"set-label",		required_argument,	NULL,	'L' },
	{"version",		no_argument,		NULL,	'V' },
	{"verbose",		no_argument,		NULL,	'v' },
	{"help",		no_argument,		NULL,	'h' },
	{"?",			no_argument,		NULL,	'?' },
	{NULL,			0,			NULL,	 0  }
};

static off_t exfat_get_root_entry_offset(struct exfat_blk_dev *bd)
{
	struct pbr *bs;
	int nbytes;
	unsigned int cluster_size;
	off_t root_clu_off;

	bs = (struct pbr *)malloc(sizeof(struct pbr));
	if (!bs) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	nbytes = exfat_read(bd->dev_fd, bs, sizeof(struct pbr), 0);
	if (nbytes != sizeof(struct pbr)) {
		exfat_err("boot sector read failed: %d\n", errno);
		return -1;
	}

	cluster_size = (1 << bs->bsx.sect_per_clus_bits) * bd->sector_size;
	root_clu_off = le32_to_cpu(bs->bsx.clu_offset) * bd->sector_size +
		le32_to_cpu(bs->bsx.root_cluster - EXFAT_REVERVED_CLUSTERS)
		* cluster_size;
	free(bs);

	return root_clu_off;
}

static int exfat_get_volume_label(struct exfat_blk_dev *bd, off_t root_clu_off)
{
	struct exfat_dentry *vol_entry;
	char volume_label[VOLUME_LABEL_BUFFER_SIZE];
	__le16 disk_label[VOLUME_LABEL_MAX_LEN];
	int nbytes;

	vol_entry = malloc(sizeof(struct exfat_dentry));
	if (!vol_entry) {
		exfat_err("failed to allocate memory\n");
		return -ENOMEM;
	}

	nbytes = exfat_read(bd->dev_fd, vol_entry,
		sizeof(struct exfat_dentry), root_clu_off);
	if (nbytes != sizeof(struct exfat_dentry)) {
		exfat_err("volume entry read failed: %d\n", errno);
		return -1;
	}

	memcpy(disk_label, vol_entry->vol_label, sizeof(disk_label));
	memset(volume_label, 0, sizeof(volume_label));
	if (exfat_utf16_dec(disk_label, vol_entry->vol_char_cnt*2,
		volume_label, sizeof(volume_label)) < 0) {
		exfat_err("failed to decode volume label\n");
		return -1;
	}

	exfat_info("label: %s\n", volume_label);
	return 0;
}

static int exfat_set_volume_label(struct exfat_blk_dev *bd,
		char *label_input, off_t root_clu_off)
{
	struct exfat_dentry vol;
	int nbytes;
	__u16 volume_label[VOLUME_LABEL_MAX_LEN];
	int volume_label_len;

	volume_label_len = exfat_utf16_enc(label_input,
			volume_label, sizeof(volume_label));
	if (volume_label_len < 0) {
		exfat_err("failed to encode volume label\n");
		return -1;
	}

	vol.type = EXFAT_VOLUME;
	memset(vol.vol_label, 0, sizeof(vol.vol_label));
	memcpy(vol.vol_label, volume_label, volume_label_len);
	vol.vol_char_cnt = volume_label_len/2;

	nbytes = exfat_write(bd->dev_fd, &vol, sizeof(struct exfat_dentry),
			root_clu_off);
	if (nbytes != sizeof(struct exfat_dentry)) {
		exfat_err("volume entry write failed: %d\n", errno);
		return -1;
	}
	fsync(bd->dev_fd);

	exfat_info("new label: %s\n", label_input);
	return 0;
}

#define EXFAT_GET_LABEL	0x1
#define EXFAT_SET_LABEL	0x2

int main(int argc, char *argv[])
{
	int c;
	int ret = EXIT_FAILURE;
	struct exfat_blk_dev bd;
	struct exfat_user_input ui;
	bool version_only = false;
	int flags = 0;
	char label_input[VOLUME_LABEL_BUFFER_SIZE];
	off_t root_clu_off;

	init_user_input(&ui);

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "L:lVvh", opts, NULL)) != EOF)
		switch (c) {
		case 'l':
			flags = EXFAT_GET_LABEL;
			break;
		case 'L':
			snprintf(label_input, sizeof(label_input), "%s",
					optarg);
			flags = EXFAT_SET_LABEL;
			break;
		case 'V':
			version_only = true;
			break;
		case 'v':
			print_level = EXFAT_DEBUG;
			break;
		case '?':
		case 'h':
		default:
			usage();
	}

	show_version();
	if (version_only)
		exit(EXIT_FAILURE);

	if (argc < 3)
		usage();

	memset(ui.dev_name, 0, sizeof(ui.dev_name));
	snprintf(ui.dev_name, sizeof(ui.dev_name), "%s", argv[argc - 1]);

	ret = exfat_get_blk_dev_info(&ui, &bd);
	if (ret < 0)
		goto out;

	root_clu_off = exfat_get_root_entry_offset(&bd);
	if (root_clu_off < 0)
		goto out;

	if (flags == EXFAT_GET_LABEL)
		ret = exfat_get_volume_label(&bd, root_clu_off);
	else if (flags == EXFAT_SET_LABEL)
		ret = exfat_set_volume_label(&bd, label_input, root_clu_off);

out:
	return ret;
}
