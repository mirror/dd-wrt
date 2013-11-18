/* This file is Copyright (c)2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <sys/types.h>
#include <err.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

void spinner(int );

int main(int argc, char **argv) {
	int speed, n, l, ifd, ofd;
	struct termios term;
	char buf[BUFSIZ];

	if (argc != 4){
		fprintf(stderr, "usage: binlog <speed> <port> <logfile>\n");
		return 1;
	}

	speed = atoi(argv[1]);
	switch (speed) {
	case 230400:
	case 115200:
	case 57600:
	case 38400:
	case 28800:
	case 19200:
	case 14400:
	case 9600:
	case 4800:
		break;
	default:
		fprintf(stderr, "invalid speed\n");
		return 1;
	}

	if ((ifd = open(argv[2], O_RDWR | O_NONBLOCK | O_NOCTTY, 0644)) == -1)
		err(1, "open");

	if ((ofd = open(argv[3], O_RDWR | O_CREAT | O_APPEND, 0644)) == -1)
		err(1, "open");

	tcgetattr(ifd, &term);
	cfmakeraw(&term);
	cfsetospeed(&term, speed);
	cfsetispeed(&term, speed);
	if (tcsetattr(ifd, TCSANOW | TCSAFLUSH, &term) == -1)
		err(1, "tcsetattr");

	tcflush(ifd, TCIOFLUSH);
	n = 0;
	while (1){
		l = read(ifd, buf, BUFSIZ);
		if (l > 0)
		    assert(write(ofd, buf, l) > 0);
		usleep(1000);
		bzero(buf, BUFSIZ);
		spinner( n++ );
	}
	/* NOTREACHED */
	close(ifd);
	close(ofd);
	return 0;
}

void spinner(int n){
	char *s = "|/-\\";

	if (n % 4)
		return;

	n /= 4;

	fprintf(stderr, "\010\010\010\010\010\010\010\010\010\010\010\010\010");
	fprintf(stderr, "%c %d", s[n%4], n);
	fflush(stderr);
}
