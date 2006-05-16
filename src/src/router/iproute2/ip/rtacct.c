/*
 * rtacct.c		Applet printing /proc/net/rt_acct.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <string.h>

#include "rt_names.h"

int main(int argc, char **argv)
{
	int i;
	int fd;
	__u32 buffer[256*4];
	char b1[64];
	int rlist[argc];
	int rhwm = 0;

	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-' || strcmp(argv[i], "help") == 0) {
			fprintf(stderr, "Usage: rtacct [ LISTofREALMS ]\n");
			exit(1);
		}
		if (rtnl_rtrealm_a2n(rlist+rhwm, argv[i]))
			fprintf(stderr, "Warning: realm \"%s\" does not exist. Ignored.\n", argv[i]);
		else
			rhwm++;
	}
	if (argc>1 && rhwm==0) {
		fprintf(stderr, "Usage: rtacct [ LISTofREALMS ]\n");
		exit(1);
	}

	fd = open("/proc/net/rt_acct", O_RDONLY);
	if (fd < 0) {
		perror("open /proc/net/rt_acct");
		exit(-1);
	}
	if ((i = read(fd, buffer, sizeof(buffer))) != sizeof(buffer)) {
		fprintf(stderr, "read only %d bytes of /proc/net/rt_acct\n", i);
		exit(-1);
	}
	printf(
"Realm      "
"BytesTo    "
"PktsTo     "
"BytesFrom  "
"PktsFrom   "
"\n"
	       );

	if (rhwm == 0) {
		for (i=0; i<256; i++) {
			__u32 *p = &buffer[i*4];

			if (!p[0] && !p[1] && !p[2] && !p[3])
				continue;
			printf(
"%-10s "
"%-10u "
"%-10u "
"%-10u "
"%-10u "
"\n"
		       , rtnl_rtrealm_n2a(i, b1, sizeof(b1)),
					  p[0], p[1], p[2], p[3]);
		}
	} else {
		int k;

		for (k=0; k<rhwm; k++) {
			__u32 *p;

			i = rlist[k];
			p = &buffer[i*4];

		printf(
"%-10s "
"%-10u "
"%-10u "
"%-10u "
"%-10u "
"\n"
		       , rtnl_rtrealm_n2a(i, b1, sizeof(b1)),
					  p[0], p[1], p[2], p[3]);
		}
	}
	exit(0);
}
