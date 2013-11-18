/*
 * This file is Copyright (c)2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>

/* 
 * @@Cj - receiver ID
 * @@Be 0 - almanac dump
 */

static int moto_send(int , char *, char *);
static char moto_gen_checksum(char *, int);
char *gpsd_hexdump(char *, size_t);
int gpsd_hexpack(char *, char *, int);
int hex2bin(char *s);

#define BSIZ 64
int main(int argc, char **argv) {
	int speed, l, fd, n;
	struct termios term;
	char buf[BSIZ];
	time_t s, t;

	if (argc != 5){
		fprintf(stderr, "usage: motosend <speed> <port> msgtype moto-body-hex\n");
		return 1;
	}

	if ((l = strlen(argv[4])) > 2*USHRT_MAX){
		fprintf(stderr, "oversized message\n");
		return 1;
	}

	if (l % 2) {
		fprintf(stderr, "body must have an even number of hex digits\n");
		return 1;
	}

	speed = atoi(argv[1]);
	switch (speed) {
	case 230400:
	case 115200:
	case 57600:
	case 38400:
	case 28800:
	case 14400:
	case 9600:
	case 4800:
		break;
	default:
		fprintf(stderr, "invalid speed\n");
		return 1;
	}

	if ((fd = open(argv[2], O_RDWR | O_NONBLOCK | O_NOCTTY, 0644)) == -1)
		err(1, "open");

	tcgetattr(fd, &term);
	cfmakeraw(&term);
	cfsetospeed(&term, speed);
	cfsetispeed(&term, speed);
	term.c_cc[VMIN] = 8;
	term.c_cc[VTIME] = 1;
	term.c_cflag &= ~(PARENB | PARODD | CRTSCTS);
	term.c_cflag |= CREAD | CLOCAL;
	term.c_iflag = term.c_oflag = term.c_lflag = (tcflag_t) 0;

	if (tcsetattr(fd, TCSANOW | TCSAFLUSH, &term) == -1)
		err(1, "tcsetattr");

	tcflush(fd, TCIOFLUSH);
	t = 0; n = 0;
	while (1){
		usleep(1000);
		bzero(buf, BSIZ);
		if ((l = read(fd, buf, BSIZ)) == -1)
			if (!(EINTR == errno || EAGAIN == errno))
				err(1, "read");

		if (l > 0){
			printf("%s", gpsd_hexdump(buf, l));
			fflush(stdout);
		}
		/* allow for up to "n" resends, once per second */
		if (((s = time(NULL)) > t) && (n < 1)){
			t = s;
			n++;
			moto_send(fd, argv[3], argv[4]);
		}
	}
	return 0;
}

char moto_gen_checksum(char *buf, int len){
	int n;
	char ck = '\0';

	for (n = 0; n < len; n++)
		ck ^= buf[n];
	return ck;

}

static int moto_send(int fd, char *type, char *body ) {
	size_t status;
	char *buf;
	unsigned short l;

	l = strlen(body) / 2;
	if ((buf = malloc(l+7)) == NULL)
		return -1;

	bzero(buf, l+7);
	buf[0] = '@'; buf[1] = '@';
	buf[2] = type[0]; buf[3] = type[1];

	if (l)
		if (gpsd_hexpack(body, buf+4, l) == -1){
			free(buf);
			return -1;
		}

	buf[l+4] = moto_gen_checksum(buf+2, l+2);
	buf[l+5] = '\r'; buf[l+6] = '\n';

	status = write(fd, buf, l+7);
	if (status == -1)
		perror("moto_send");
	return (int)status;
}

static char last;
char *gpsd_hexdump(char *binbuf, size_t binbuflen)
{
	static char hexbuf[USHRT_MAX*2+10+2];
	size_t i, j = 0;
	size_t len = (size_t)binbuflen;
	const char *ibuf = (const char *)binbuf;
	const char *hexchar = "0123456789abcdef";

	for (i = 0; i < len; i++) {
		if (ibuf[i] == '@' && (ibuf[i+1] == '@' || last == '@')){
			hexbuf[j++] = '\n';
			hexbuf[j++] = ibuf[i++];
			hexbuf[j++] = ibuf[i++];
			hexbuf[j++] = ibuf[i++];
			hexbuf[j++] = ibuf[i++];
		} else {
			hexbuf[j++] = hexchar[ (ibuf[i]&0xf0)>>4 ];
			hexbuf[j++] = hexchar[ ibuf[i]&0x0f ];
		}
		last = ibuf[i];
	}
	hexbuf[j] ='\0';
	return hexbuf;
}

int gpsd_hexpack(char *src, char *dst, int len){
	int i, k, l;

	l = (int)(strlen(src) / 2);
	if ((l < 1) || (l > len))
		return -1;

	bzero(dst, len);
	for (i = 0; i < l; i++)
		if ((k = hex2bin(src+i*2)) != -1)
			dst[i] = (char)(k & 0xff);
		else
			return -1;
	return l;
}

int hex2bin(char *s)
{
	int a, b;

	a = s[0] & 0xff;
	b = s[1] & 0xff;

	if ((a >= 'a') && (a <= 'f'))
		a = a + 10 - 'a';
	else if ((a >= 'A') && (a <= 'F'))
		a = a + 10 - 'A';
	else if ((a >= '0') && (a <= '9'))
		a -= '0';
	else
		return -1;

	if ((b >= 'a') && (b <= 'f'))
		b = b + 10 - 'a';
	else if ((b >= 'A') && (b <= 'F'))
		b = b + 10 - 'A';
	else if ((b >= '0') && (b <= '9'))
		b -= '0';
	else
		return -1;

	return ((a<<4) + b);
}
