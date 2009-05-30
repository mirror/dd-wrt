/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include "libbridge.h"
#include "libbridge_private.h"

int br_socket_fd = -1;
struct sysfs_class *br_class_net;

int br_init(void)
{
	if (br_socket_fd == -1) {
		if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			return errno;

		br_class_net = sysfs_open_class("net");
	}
	return 0;
}

void br_shutdown(void)
{
	if (br_socket_fd != -1) {
		sysfs_close_class(br_class_net);
		br_class_net = NULL;
		close(br_socket_fd);
		br_socket_fd = -1;
	}
}
