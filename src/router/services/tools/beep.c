/*
 * beep.c
 *
 * Copyright (C) 2008 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>

void beep(int gpio, int time, int polarity)
{
	set_gpio(gpio, 1 - polarity);
	usleep(time * 1000);
	set_gpio(gpio, 0 + polarity);
	usleep(time * 1000);
}

int beep_main(int argc, char **argv)
{
	unsigned int gpio;
	unsigned int polarity;
	unsigned char assoclist[1024];

	if (argc != 4) {
		fprintf(stderr, "%s <interface> <gpio> <polarity>\n", argv[0]);
		exit(1);
	}
	gpio = atoi(argv[2]);
	polarity = atoi(argv[3]);
	while (1) {
		int cnt = getassoclist(argv[1], assoclist);

		if (cnt == -1) {
			cnt = 0;
		}
		if (!cnt) {
			fprintf(stderr, "not associated, wait 5 seconds\n");
			sleep(5);
			continue;
		}
		unsigned char *pos = assoclist;

		pos += 4;
		int rssi = getRssi(argv[1], pos);
		int noise = getNoise(argv[1], pos);
		int snr = rssi - noise;

		if (snr < 0) {
			fprintf(stderr, "snr is %d, invalid\n", snr);
			continue;
		}
		if (snr > 30) {
			fprintf(stderr, "snr perfect, full beep (%d)\n", snr);
			set_gpio(gpio, 1 - polarity); // snr perfect
			continue;
		}
		int beeptime = 66 * (31 - snr);

		fprintf(stderr, "snr is %d, beep interval %d\n", snr, beeptime);

		beep(gpio, beeptime, polarity);
	}

	return 0;
}
