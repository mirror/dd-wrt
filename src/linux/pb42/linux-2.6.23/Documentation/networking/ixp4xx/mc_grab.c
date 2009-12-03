/*
 * mc_grab.c  - grabs IXP4XX microcode from a binary datastream
 * e.g. The redboot bootloader....
 *
 * usage: mc_grab 1010200 2010200 < /dev/mtd/0 > /dev/misc/npe
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define MAX_IMG 6

static void print_mc_info(unsigned id, int siz)
{
	unsigned char buf[sizeof(unsigned)];
	*(unsigned*)buf = id;
	unsigned idx;
	const char *names[] = { "IXP425", "IXP465", "unknown" };

	idx = (buf[0] >> 4) < 2 ? (buf[0] >> 4) : 2;

	fprintf(stderr, "Device: %s:NPE_%c Func: %2x Rev: %02x.%02x "
		"Size: %5d bytes ID:%08x\n", names[idx], (buf[0] & 0xf)+'A',
		buf[1], buf[2], buf[3], siz*4, ntohl(id));
}

int main(int argc, char *argv[])
{
	int i,j;
	unsigned char buf[sizeof(unsigned)];
	unsigned magic = htonl(0xfeedf00d);
	unsigned id, my_ids[MAX_IMG+1], siz, sizbe;
	int ret=1, verbose=0;

	for (i=0, j=0; i<argc-1 && j<MAX_IMG; i++) {
		if (!strcmp(argv[i+1], "-v"))
			verbose = 1;
		else
			my_ids[j++] = htonl(strtoul(argv[i+1], NULL, 16));
	}
	my_ids[j] = 0;
	if (my_ids[0] == 0 && !verbose) {
		fprintf(stderr, "Usage: %s <-v> [ID1] [ID2] [IDn]\n", argv[0]);
		return 1;
	}

	while ((ret=read(0, buf, sizeof(unsigned))) == sizeof(unsigned)) {
		if (*(unsigned*)buf != magic)
			continue;
		if ((ret=read(0, buf, sizeof(unsigned))) != sizeof(unsigned) )
			break;
		id = *(unsigned*)buf;

		if (read(0, buf, sizeof(siz)) != sizeof(siz) )
			break;
		sizbe = *(unsigned*)buf;
		siz = ntohl(sizbe);

		if (verbose)
			print_mc_info(id, siz);

		for(i=0; my_ids[i]; i++)
			if (id == my_ids[i])
				break;
		if (!my_ids[i])
			continue;

		if (!verbose)
			print_mc_info(id, siz);

		write(1, &magic, sizeof(magic));
		write(1, &id, sizeof(id));
		write(1, &sizbe, sizeof(sizbe));
		for (i=0; i<siz; i++) {
			if (read(0, buf, sizeof(unsigned)) != sizeof(unsigned))
				break;
			write(1, buf, sizeof(unsigned));
		}
		if (i != siz)
			break;
	}
	if (ret)
		fprintf(stderr, "Error reading  Microcode\n");
	return ret;
}
