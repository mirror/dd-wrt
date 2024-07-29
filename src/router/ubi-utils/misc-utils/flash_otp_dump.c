/*
 * flash_otp_dump.c -- display One-Time-Programm data
 */

#define PROGRAM_NAME "flash_otp_dump"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include <mtd/mtd-user.h>

int main(int argc,char *argv[])
{
	int fd, val, i, ret;
	int offset = 0;
	unsigned char buf[16];

	if (argc < 3 || (strcmp(argv[1], "-f") && strcmp(argv[1], "-u"))) {
		fprintf(stderr,"Usage: %s [ -f | -u ] <device> [<offset>]\n", PROGRAM_NAME);
		return EINVAL;
	}

	fd = open(argv[2], O_RDONLY);
	if (fd < 0) {
		perror(argv[2]);
		return errno;
	}

	val = argv[1][1] == 'f' ? MTD_OTP_FACTORY : MTD_OTP_USER;
	ret = ioctl(fd, OTPSELECT, &val);
	if (ret < 0) {
		perror("OTPSELECT");
		return errno;
	}

	if (argc >= 4) {
		char *p;
		offset = (off_t)strtoull(argv[3], &p, 0);
		if (argv[3][0] == 0 || *p != 0) {
			fprintf(stderr, "%s: bad offset value\n", PROGRAM_NAME);
			return ERANGE;
		}
	}

	if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
		perror("lseek()");
		return errno;
	}

	printf("OTP %s data for %s\n",
			argv[1][1] == 'f' ? "factory" : "user", argv[2]);
	while ((ret = read(fd, buf, sizeof(buf)))) {
		if (ret < 0) {
			perror("read()");
			return errno;
		}
		printf("0x%04x:", offset);
		for (i = 0; i < ret; i++)
			printf(" %02x", buf[i]);
		printf("\n");
		offset += ret;
	}

	close(fd);
	return 0;
}
