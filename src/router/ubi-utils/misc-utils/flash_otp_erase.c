/*
 * flash_otp_erase.c -- erase area of One-Time-Program data
 */

#define PROGRAM_NAME "flash_otp_erase"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <mtd/mtd-user.h>
#include "common.h"

int main(int argc,char *argv[])
{
	int fd, val, ret, offset, size;
	struct otp_info info;
	char *p;

	if (argc != 5 || strcmp(argv[1], "-u")) {
		fprintf(stderr, "Usage: %s -u <device> <offset> <size>\n", PROGRAM_NAME);
		fprintf(stderr, "offset and size must match on OTP region boundaries\n");
		return EINVAL;
	}

	fd = open(argv[2], O_WRONLY);
	if (fd < 0) {
		perror(argv[2]);
		return errno;
	}

	val = MTD_OTP_USER;
	ret = ioctl(fd, OTPSELECT, &val);
	if (ret < 0) {
		perror("OTPSELECT");
		return errno;
	}

	offset = strtoul(argv[3], &p, 0);
	if (argv[3][0] == 0 || *p != 0) {
		fprintf(stderr, "%s: bad offset value\n", PROGRAM_NAME);
		return ERANGE;
	}

	size = strtoul(argv[4], &p, 0);
	if (argv[4][0] == 0 || *p != 0) {
		fprintf(stderr, "%s: bad size value\n", PROGRAM_NAME);
		return ERANGE;
	}

	info.start = offset;
	info.length = size;
	ret = ioctl(fd, OTPERASE, &info);
	if (ret	< 0) {
		perror("OTPERASE");
		return errno;
	}

	return 0;
}
