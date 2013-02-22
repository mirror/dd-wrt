/*
 * flash_info.c -- print info about a MTD device
 */

#define PROGRAM_NAME "flash_info"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

#include "common.h"
#include <mtd/mtd-user.h>

static void usage(int status)
{
	fprintf(status ? stderr : stdout,
		"Usage: %s <device> [devices]\n",
		PROGRAM_NAME);
	exit(status);
}

int main(int argc, char *argv[])
{
	int fd, i, regcount;

	warnmsg("this utility is deprecated in favor of `mtdinfo` and will be removed in mtd-utils-1.4.6");

	if (argc < 2)
		usage(1);
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
		usage(0);

	for (i = 1; i < argc; ++i) {
		const char *dev = argv[i];
		int r;
		region_info_t reginfo;

		/* Open and size the device */
		fd = open(dev, O_RDONLY);
		if (fd < 0) {
			sys_errmsg("could not open: %s", dev);
			continue;
		}

		if (ioctl(fd, MEMGETREGIONCOUNT, &regcount))
			continue;

		printf("%s: %d erase regions\n", dev, regcount);
		for (r = 0; r < regcount; ++r) {
			reginfo.regionindex = r;
			if (ioctl(fd, MEMGETREGIONINFO, &reginfo) == 0) {
				printf("Region %d is at 0x%x with size 0x%x and "
						"has 0x%x blocks\n", r, reginfo.offset,
						reginfo.erasesize, reginfo.numblocks);
			} else {
				warnmsg("can not read region %d from a %d region device",
					r, regcount);
			}
		}
	}

	return 0;
}
