/*
 *	$Id: bidi.c,v 1.1 2005/09/28 11:53:39 seg Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *names[1024];
int numfds;
struct pollfd pfd[1024];

void rcvd_packet(int index, unsigned char *buf, int len)
{
	int i;
	int dmp;

	printf("%s: %i\n", names[index], len);
	dmp = (len + 15) >> 4;
	if (dmp > 8)
		dmp = 8;

	for (i=0;i<dmp;i++) {
		int j;

		for (j=0;j<16;j++)
			printf("%.2x ", buf[(i<<4)|j]);

		printf("\n");
	}

	printf("\n");
}

void loop()
{
	while (1) {
		int i;

		if (poll(pfd, numfds, -1) < 0) {
			perror("poll");
			break;
		}

		for (i=0;i<numfds;i++) {
			unsigned char buf[2048];
			int j;
			int len;

			if (!(pfd[i].revents & POLLIN))
				continue;

			if ((len = read(pfd[i].fd, buf, 2048)) < 0)
				continue;

			rcvd_packet(i, buf, len);
			for (j=0;j<numfds;j++)
				if (i != j)
					write(pfd[j].fd, buf, len);
		}
	}
}

void openfds(int argc, char *argv[])
{
	int i;

	numfds = 0;
	for (i=1;i<argc;i++) {
		int fd;

		if ((fd = open(argv[i], O_RDWR)) < 0) {
			perror("open");
			continue;
		}

		names[numfds] = argv[i];
		pfd[numfds].fd = fd;
		pfd[numfds].events = POLLIN;
		numfds++;
	}
}

int main(int argc, char *argv[])
{
	openfds(argc, argv);
	loop();

	return 0;
}
