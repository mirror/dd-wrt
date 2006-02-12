/*
 * $Id: libbridge_misc.c,v 1.1 2005/09/28 11:53:38 seg Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <asm/param.h>
#include "libbridge.h"
#include "libbridge_private.h"

unsigned long __tv_to_jiffies(struct timeval *tv)
{
	unsigned long jif;

	jif = 1000000 * tv->tv_sec + tv->tv_usec;

	return (HZ*jif)/1000000;
}

void __jiffies_to_tv(struct timeval *tv, unsigned long jiffies)
{
	unsigned long tvusec;

	tvusec = (1000000*jiffies)/HZ;
	tv->tv_sec = tvusec/1000000;
	tv->tv_usec = tvusec - 1000000 * tv->tv_sec;
}

static char *state_names[5] = {"disabled", "listening", "learning", "forwarding", "blocking"};

char *br_get_state_name(int state)
{
	if (state >= 0 && state <= 4)
		return state_names[state];

	return "<INVALID STATE>";
}

struct bridge *br_find_bridge(char *brname)
{
	struct bridge *b;

	b = bridge_list;
	while (b != NULL) {
		if (!strcmp(b->ifname, brname))
			return b;

		b = b->next;
	}

	return NULL;
}

struct port *br_find_port(struct bridge *br, char *portname)
{
	char index;
	struct port *p;

	if (!(index = if_nametoindex(portname)))
		return NULL;

	p = br->firstport;
	while (p != NULL) {
		if (p->ifindex == index)
			return p;

		p = p->next;
	}

	return NULL;
}
