/*
 * ledtool.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <utils.h>
#include <nvparse.h>

static int ledtool_main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stdout, "invalid argument\n");
		fprintf(stdout, "ledtool blink_times [type]\n");
		fprintf(stdout, "types:\n");
		fprintf(stdout, "1 = led connected (blink 1 time)\n");
		fprintf(stdout, "2 = ses led (blink 2 times)\n");
		fprintf(stdout, "3 = ses error (blink 5 times)\n");
		fprintf(stdout, "4 = beeper (on supported devices)\n");
		fprintf(stdout, "default = diag led (blink 1 time)\n");
		exit(-1);
	}
	/* 
	 * Run it in the background 
	 */
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
	int times = atoi(argv[1]);
	int type = 0;
	int count = 0;
	if (argc > 2)
		type = atoi(argv[2]);

	while (times > 0) {
		switch (type) {
		case 1:
			led_control(LED_CONNECTED, LED_ON);
			usleep(500000);
			led_control(LED_CONNECTED, LED_OFF);
			usleep(500000);
			break;
		case 2: // aoss negotiation
			led_control(LED_SES, LED_ON);
			usleep(200000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(200000);
			led_control(LED_SES, LED_OFF);
			usleep(500000);
			break;
		case 3: // aoss error
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			led_control(LED_SES, LED_ON);
			usleep(100000);
			led_control(LED_SES, LED_OFF);
			usleep(100000);
			break;
		case 4:
			led_control(BEEPER, LED_ON);
			usleep(500000);
			led_control(BEEPER, LED_OFF);
			usleep(500000);
			break;

		default:
			led_control(LED_DIAG, LED_ON);
			usleep(500000);
			led_control(LED_DIAG, LED_OFF);
			usleep(500000);
			if (count && (count % 3) == 0)
				sleep(3);

			break;
		}
		times--;
		count++;
	}
	if (type == 3) {
		eval("restart", "ses_led_control");
	}

	return 0;
} // end main
