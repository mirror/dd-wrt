/*
 * ledtracking.c
 *
 * Copyright (C) 2011 Christian Scheele <chris@dd-wrt.com>
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

int ledtracking_main(int argc, char **argv)
{
	int toggle = 0;
	int testsnr = 0;
	int rssi, noise, snr_min, snr_max, polarity, delay, snr, gpio;
	unsigned char assoclist[1024];

	if (argc <= 4) {
		fprintf(stderr, "%s <interfacex> <gpio> <polarity> <snr-max>\n", argv[0]);
		exit(1);
	}
	gpio = atoi(argv[2]);
	polarity = atoi(argv[3]);
	snr_max = atoi(argv[4]);
	snr_min = snr_max / 6;
	if (argc == 6) {
		testsnr = atoi(argv[5]);
		fprintf(stderr, "use testsnr %d\n", testsnr);
	}

	while (1) {
		if (testsnr) {
			snr = testsnr;
		} else {
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
			rssi = getRssi(argv[1], pos);
			noise = getNoise(argv[1], pos);
			snr = rssi - noise;
		}

		if (snr < 0) {
			fprintf(stderr, "snr is %d, invalid\n", snr);
			continue;
		}

		snr -= snr_min;
		if (snr < 0)
			snr = 0;

		if (snr >= snr_max - snr_min) {
			fprintf(stderr, "snr >= snr_max - snr_min\n");
			toggle = polarity;
			delay = 100;
		} else {
			fprintf(stderr, "else\n");
			toggle = !toggle;
			delay = 1000 - snr * (1000 - 125) / (snr_max - snr_min);
		}

		usleep(1000 * delay / 2);
		set_gpio(gpio, 0 + toggle);
		fprintf(stderr, "%d,%d\n", gpio, toggle);
	}
}
