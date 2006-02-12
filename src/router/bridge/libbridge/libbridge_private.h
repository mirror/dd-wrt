/*
 * $Id: libbridge_private.h,v 1.1 2005/09/28 11:53:38 seg Exp $
 *
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

#ifndef _LIBBRIDGE_PRIVATE_H
#define _LIBBRIDGE_PRIVATE_H

#include <asm/param.h>

int br_socket_fd;

unsigned long __tv_to_jiffies(struct timeval *tv);
void __jiffies_to_tv(struct timeval *tv, unsigned long jiffies);

int br_device_ioctl(struct bridge *br,
		     unsigned long arg0,
		     unsigned long arg1,
		     unsigned long arg2,
		     unsigned long arg3);
int br_get_version(void);
int br_ioctl(unsigned long arg0, unsigned long arg1, unsigned long arg2);
int br_make_bridge_list(void);
int br_make_port_list(struct bridge *br);
int br_read_info(struct bridge *br);
int br_read_port_info(struct port *p);

#endif
