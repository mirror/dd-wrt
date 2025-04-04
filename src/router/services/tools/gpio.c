/*
 * gpio.c
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
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils.h>

int gpio_main(int argc, char **argv)
{
	unsigned int gpio;
	unsigned int old_gpio = -1;
	unsigned int pin;

	if (argc != 3) {
		fprintf(stderr, "%s <poll | enable | disable> <pin>\n", argv[0]);
		exit(1);
	}

	pin = atoi(argv[2]);
	if (!strcmp(argv[1], "poll")) {
		while (1) {
			gpio = get_gpio(pin);
			if (gpio != old_gpio)
				fprintf(stdout, "%02X\n", gpio);
			old_gpio = gpio;
		}
	} else if (!strcmp(argv[1], "init")) {
		gpio = get_gpio(pin);
	} else if (!strcmp(argv[1], "enable")) {
		gpio = 1;
		set_gpio(pin, gpio);
	} else if (!strcmp(argv[1], "disable")) {
		gpio = 0;
		set_gpio(pin, gpio);
	}

	return 0;
}
