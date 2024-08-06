// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 Namjae Jeon <linkinjeon@kernel.org>
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
#include "exfat_fs.h"

static void usage(void)
{
	fprintf(stderr, "Usage: exfatlabel\n");
	fprintf(stderr, "\t-i | --volume-serial                  Switch to volume serial mode\n");
	fprintf(stderr, "\t-V | --version                        Show version\n");
	fprintf(stderr, "\t-h | --help                           Show help\n");

	exit(EXIT_FAILURE);
}

static struct option opts[] = {
	{"volume-serial",	no_argument,		NULL,	'i' },
	{"version",		no_argument,		NULL,	'V' },
	{"help",		no_argument,		NULL,	'h' },
	{"?",			no_argument,		NULL,	'?' },
	{NULL,			0,			NULL,	 0  }
};

int main(int argc, char *argv[])
{
	int c;
	int ret = EXIT_FAILURE;
	struct exfat_blk_dev bd;
	struct exfat_user_input ui;
	bool version_only = false;
	int serial_mode = 0;
	int flags = 0;
	unsigned long volume_serial;

	init_user_input(&ui);

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	if (argc == 2)
		flags = EXFAT_GET_VOLUME_LABEL;
	else if (argc == 3)
		flags = EXFAT_SET_VOLUME_LABEL;

	opterr = 0;
	while ((c = getopt_long(argc, argv, "iVh", opts, NULL)) != EOF)
		switch (c) {
		case 'i':
			serial_mode = true;
			if (argc == 3)
				flags = EXFAT_GET_VOLUME_SERIAL;
			else if (argc == 4)
				flags = EXFAT_SET_VOLUME_SERIAL;

			break;
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
	snprintf(ui.dev_name, sizeof(ui.dev_name), "%s", argv[serial_mode + 1]);

	ret = exfat_get_blk_dev_info(&ui, &bd);
	if (ret < 0)
		goto out;

	if (serial_mode) {
		/* Mode to change or display volume serial */
		if (flags == EXFAT_GET_VOLUME_SERIAL) {
			ret = exfat_show_volume_serial(bd.dev_fd);
		} else if (flags == EXFAT_SET_VOLUME_SERIAL) {
			ret = exfat_parse_ulong(argv[3], &volume_serial);
			if (volume_serial > UINT_MAX)
				ret = -ERANGE;


			if (ret < 0) {
				exfat_err("invalid serial number(%s)\n", argv[3]);
				goto close_fd_out;
			}

			ui.volume_serial = volume_serial;
			ret = exfat_set_volume_serial(&bd, &ui);
		}
	} else {
		struct exfat *exfat;
		struct pbr *bs;

		ret = read_boot_sect(&bd, &bs);
		if (ret)
			goto close_fd_out;

		exfat = exfat_alloc_exfat(&bd, bs);
		if (!exfat) {
			ret = -ENOMEM;
			goto close_fd_out;
		}

		exfat->root = exfat_alloc_inode(ATTR_SUBDIR);
		if (!exfat->root) {
			ret = -ENOMEM;
			goto free_exfat;
		}

		exfat->root->first_clus = le32_to_cpu(exfat->bs->bsx.root_cluster);
		if (exfat_root_clus_count(exfat)) {
			exfat_err("failed to follow the cluster chain of root\n");
			exfat_free_inode(exfat->root);
			ret = -EINVAL;
			goto free_exfat;
		}

		/* Mode to change or display volume label */
		if (flags == EXFAT_GET_VOLUME_LABEL)
			ret = exfat_read_volume_label(exfat);
		else if (flags == EXFAT_SET_VOLUME_LABEL)
			ret = exfat_set_volume_label(exfat, argv[2]);

free_exfat:
		if (exfat)
			exfat_free_exfat(exfat);
	}

close_fd_out:
	close(bd.dev_fd);
out:
	return ret;
}
