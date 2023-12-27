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
#include "exfat_fs.h"

static void usage(void)
{
	fprintf(stderr, "Usage: tune.exfat\n");
	fprintf(stderr, "\t-l | --print-label                    Print volume label\n");
	fprintf(stderr, "\t-L | --set-label=label                Set volume label\n");
	fprintf(stderr, "\t-u | --print-guid                     Print volume GUID\n");
	fprintf(stderr, "\t-U | --set-guid=guid                  Set volume GUID\n");
	fprintf(stderr, "\t-i | --print-serial                   Print volume serial\n");
	fprintf(stderr, "\t-I | --set-serial=value               Set volume serial\n");
	fprintf(stderr, "\t-V | --version                        Show version\n");
	fprintf(stderr, "\t-v | --verbose                        Print debug\n");
	fprintf(stderr, "\t-h | --help                           Show help\n");

	exit(EXIT_FAILURE);
}

static struct option opts[] = {
	{"print-label",		no_argument,		NULL,	'l' },
	{"set-label",		required_argument,	NULL,	'L' },
	{"print-guid",		no_argument,		NULL,	'u' },
	{"set-guid",		required_argument,	NULL,	'U' },
	{"print-serial",	no_argument,		NULL,	'i' },
	{"set-serial",		required_argument,	NULL,	'I' },
	{"version",		no_argument,		NULL,	'V' },
	{"verbose",		no_argument,		NULL,	'v' },
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
	int flags = 0;
	char label_input[VOLUME_LABEL_BUFFER_SIZE];
	struct exfat *exfat = NULL;
	struct pbr *bs;

	init_user_input(&ui);

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "I:iL:lU:uVvh", opts, NULL)) != EOF)
		switch (c) {
		case 'l':
			flags = EXFAT_GET_VOLUME_LABEL;
			break;
		case 'L':
			snprintf(label_input, sizeof(label_input), "%s",
					optarg);
			flags = EXFAT_SET_VOLUME_LABEL;
			break;
		case 'u':
			flags = EXFAT_GET_VOLUME_GUID;
			break;
		case 'U':
			if (*optarg != '\0' && *optarg != '\r')
				ui.guid = optarg;
			flags = EXFAT_SET_VOLUME_GUID;
			break;
		case 'i':
			flags = EXFAT_GET_VOLUME_SERIAL;
			break;
		case 'I':
			ui.volume_serial = strtoul(optarg, NULL, 0);
			flags = EXFAT_SET_VOLUME_SERIAL;
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

	/* Mode to change or display volume serial */
	if (flags == EXFAT_GET_VOLUME_SERIAL) {
		ret = exfat_show_volume_serial(bd.dev_fd);
		goto close_fd_out;
	} else if (flags == EXFAT_SET_VOLUME_SERIAL) {
		ret = exfat_set_volume_serial(&bd, &ui);
		goto close_fd_out;
	}

	ret = read_boot_sect(&bd, &bs);
	if (ret)
		goto close_fd_out;

	exfat = exfat_alloc_exfat(&bd, bs);
	if (!exfat) {
		free(bs);
		ret = -ENOMEM;
		goto close_fd_out;
	}

	exfat->root = exfat_alloc_inode(ATTR_SUBDIR);
	if (!exfat->root) {
		ret = -ENOMEM;
		goto close_fd_out;
	}

	exfat->root->first_clus = le32_to_cpu(exfat->bs->bsx.root_cluster);
	if (exfat_root_clus_count(exfat)) {
		exfat_err("failed to follow the cluster chain of root\n");
		exfat_free_inode(exfat->root);
		ret = -EINVAL;
		goto close_fd_out;
	}

	if (flags == EXFAT_GET_VOLUME_LABEL)
		ret = exfat_read_volume_label(exfat);
	else if (flags == EXFAT_SET_VOLUME_LABEL)
		ret = exfat_set_volume_label(exfat, label_input);
	else if (flags == EXFAT_GET_VOLUME_GUID)
		ret = exfat_read_volume_guid(exfat);
	else if (flags == EXFAT_SET_VOLUME_GUID)
		ret = exfat_set_volume_guid(exfat, ui.guid);

close_fd_out:
	close(bd.dev_fd);
	if (exfat)
		exfat_free_exfat(exfat);
out:
	return ret;
}
