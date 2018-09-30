#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mount.h>
/*
 * This is a tiny test program to endian-swap
 * the superblock on a given device.
 * We simply read 4k from where the superblock should be
 * do the swap, and write it back
 * Don't use this on a real array, use mdadm.
 */

#define MD_RESERVED_BYTES		(64 * 1024)
#define MD_RESERVED_SECTORS		(MD_RESERVED_BYTES / 512)

#define MD_NEW_SIZE_SECTORS(x)		((x & ~(MD_RESERVED_SECTORS - 1)) - MD_RESERVED_SECTORS)

extern long long lseek64(int, long long, int);

int main(int argc, char *argv[])
{
	int fd, i;
	unsigned long size;
	unsigned long long offset;
	char super[4096];
	if (argc != 2) {
		fprintf(stderr, "Usage: swap_super device\n");
		exit(1);
	}
	fd = open(argv[1], O_RDWR);
	if (fd<0) {
		perror(argv[1]);
		exit(1);
	}
	if (ioctl(fd, BLKGETSIZE, &size)) {
		perror("BLKGETSIZE");
		exit(1);
	}
	offset = MD_NEW_SIZE_SECTORS(size) * 512LL;
	if (lseek64(fd, offset, 0) < 0LL) {
		perror("lseek64");
		exit(1);
	}
	if (read(fd, super, 4096) != 4096) {
		perror("read");
		exit(1);
	}

	for (i=0; i < 4096 ; i+=4) {
		char t = super[i];
		super[i] = super[i+3];
		super[i+3] = t;
		t=super[i+1];
		super[i+1]=super[i+2];
		super[i+2]=t;
	}
	/* swap the u64 events counters */
	for (i=0; i<4; i++) {
		/* events_hi and events_lo */
		char t=super[32*4+7*4 +i];
		super[32*4+7*4 +i] = super[32*4+8*4 +i];
		super[32*4+8*4 +i] = t;

		/* cp_events_hi and cp_events_lo */
		t=super[32*4+9*4 +i];
		super[32*4+9*4 +i] = super[32*4+10*4 +i];
		super[32*4+10*4 +i] = t;
	}

	if (lseek64(fd, offset, 0) < 0LL) {
		perror("lseek64");
		exit(1);
	}
	if (write(fd, super, 4096) != 4096) {
		perror("write");
		exit(1);
	}
	exit(0);

}
