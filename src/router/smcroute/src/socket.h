/* Helper functions
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef SMCROUTE_SOCKET_H_
#define SMCROUTE_SOCKET_H_

#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

int socket_register(int sd, void (*cb)(int, void *), void *arg);
int socket_create  (int domain, int type, int proto, void (*cb)(int, void *), void *arg);
int socket_close   (int sd);
int socket_poll    (struct timeval *timeout);

#endif /* SMCROUTE_SOCKET_H_ */
