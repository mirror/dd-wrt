/* Multicast Router Discovery Protocol, RFC4286 (IPv4 only)
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

#ifndef SMCROUTE_MRDISC_H_
#define SMCROUTE_MRDISC_H_

#include "config.h"
#define MRDISC_INTERVAL_DEFAULT 20

#ifdef ENABLE_MRDISC
int mrdisc_init       (int interval);
int mrdisc_exit       (void);

int mrdisc_register   (char *ifname, short vif);
int mrdisc_deregister (              short vif);

int mrdisc_enable     (short vif);
int mrdisc_disable    (short vif);

void mrdisc_send      (        void *arg);
void mrdisc_recv      (int sd, void *arg);

#else
#define mrdisc_init(interval)
#define mrdisc_exit()

#define mrdisc_register(ifname, vif) 0
#define mrdisc_deregister(vif)       0

#define mrdisc_enable(ifname)
#define mrdisc_disable(ifname)       {}

#define mrdisc_send(arg)
#define mrdisc_recv(sd, arg)
#endif

#endif /* SMCROUTE_MRDISC_H_ */
