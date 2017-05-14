/**
 * f2fs_format.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * Dual licensed under the GPL or LGPL version 2 licenses.
 */
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <time.h>
#include <uuid/uuid.h>

#include "f2fs_fs.h"
#include "f2fs_format_utils.h"

extern struct f2fs_configuration c;

static void mkfs_usage()
{
	MSG(0, "\nUsage: mkfs.f2fs [options] device [sectors]\n");
	MSG(0, "[options]:\n");
	MSG(0, "  -a heap-based allocation [default:1]\n");
	MSG(0, "  -c [device path] up to 7 devices excepts meta device\n");
	MSG(0, "  -d debug level [default:0]\n");
	MSG(0, "  -e [extension list] e.g. \"mp3,gif,mov\"\n");
	MSG(0, "  -l label\n");
	MSG(0, "  -o overprovision ratio [default:5]\n");
	MSG(0, "  -O set feature\n");
	MSG(0, "  -q quiet mode\n");
	MSG(0, "  -s # of segments per section [default:1]\n");
	MSG(0, "  -z # of sections per zone [default:1]\n");
	MSG(0, "  -t 0: nodiscard, 1: discard [default:1]\n");
	MSG(0, "  -m support zoned block device [default:0]\n");
	MSG(0, "sectors: number of sectors. [default: determined by device size]\n");
	exit(1);
}

static void f2fs_show_info()
{
	MSG(0, "\n\tF2FS-tools: mkfs.f2fs Ver: %s (%s)\n\n",
				F2FS_TOOLS_VERSION,
				F2FS_TOOLS_DATE);
	if (c.heap == 0)
		MSG(0, "Info: Disable heap-based policy\n");

	MSG(0, "Info: Debug level = %d\n", c.dbg_lv);
	if (c.extension_list)
		MSG(0, "Info: Add new extension list\n");

	if (c.vol_label)
		MSG(0, "Info: Label = %s\n", c.vol_label);
	MSG(0, "Info: Trim is %s\n", c.trim ? "enabled": "disabled");
}

static void parse_feature(const char *features)
{
	if (!strcmp(features, "encrypt")) {
		c.feature |= cpu_to_le32(F2FS_FEATURE_ENCRYPT);
	} else {
		MSG(0, "Error: Wrong features\n");
		mkfs_usage();
	}
}

static void f2fs_parse_options(int argc, char *argv[])
{
	static const char *option_string = "qa:c:d:e:l:mo:O:s:z:t:";
	int32_t option=0;

	while ((option = getopt(argc,argv,option_string)) != EOF) {
		switch (option) {
		case 'q':
			c.dbg_lv = -1;
			break;
		case 'a':
			c.heap = atoi(optarg);
			break;
		case 'c':
			if (c.ndevs >= MAX_DEVICES) {
				MSG(0, "Error: Too many devices\n");
				mkfs_usage();
			}

			if (strlen(optarg) > MAX_PATH_LEN) {
				MSG(0, "Error: device path should be less than "
					"%d characters\n", MAX_PATH_LEN);
				mkfs_usage();
			}
			c.devices[c.ndevs++].path = strdup(optarg);
			break;
		case 'd':
			c.dbg_lv = atoi(optarg);
			break;
		case 'e':
			c.extension_list = strdup(optarg);
			break;
		case 'l':		/*v: volume label */
			if (strlen(optarg) > 512) {
				MSG(0, "Error: Volume Label should be less than "
						"512 characters\n");
				mkfs_usage();
			}
			c.vol_label = optarg;
			break;
		case 'm':
			c.zoned_mode = 1;
			break;
		case 'o':
			c.overprovision = atof(optarg);
			break;
		case 'O':
			parse_feature(optarg);
			break;
		case 's':
			c.segs_per_sec = atoi(optarg);
			break;
		case 'z':
			c.secs_per_zone = atoi(optarg);
			break;
		case 't':
			c.trim = atoi(optarg);
			break;
		default:
			MSG(0, "\tError: Unknown option %c\n",option);
			mkfs_usage();
			break;
		}
	}

	if (optind >= argc) {
		MSG(0, "\tError: Device not specified\n");
		mkfs_usage();
	}

	/* [0] : META, [1 to MAX_DEVICES - 1] : NODE/DATA */
	c.devices[0].path = strdup(argv[optind]);

	if ((optind + 1) < argc) {
		if (c.ndevs > 1) {
			MSG(0, "\tError: Not support custom size on multi-devs.\n");
			mkfs_usage();
		}
		c.wanted_total_sectors = atoll(argv[optind+1]);
	}

	if (c.zoned_mode)
		c.feature |= cpu_to_le32(F2FS_FEATURE_BLKZONED);
}

int main(int argc, char *argv[])
{
	f2fs_init_configuration();

	f2fs_parse_options(argc, argv);

	f2fs_show_info();

	if (f2fs_devs_are_umounted() < 0) {
		MSG(0, "\tError: Not available on mounted device!\n");
		return -1;
	}

	if (f2fs_get_device_info() < 0)
		return -1;

	/*
	 * Some options are mandatory for host-managed
	 * zoned block devices.
	 */
	if (c.zoned_model == F2FS_ZONED_HM && !c.zoned_mode) {
		MSG(0, "\tError: zoned block device feature is required\n");
		return -1;
	}

	if (c.zoned_mode && !c.trim) {
		MSG(0, "\tError: Trim is required for zoned block devices\n");
		return -1;
	}

	if (f2fs_format_device() < 0)
		return -1;

	f2fs_finalize_device();

	MSG(0, "Info: format successful\n");

	return 0;
}
