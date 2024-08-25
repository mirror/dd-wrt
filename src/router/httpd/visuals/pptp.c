/*
 * pptp.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <ctype.h>

#include <broadcom.h>

EJ_VISIBLE void ej_dumppptp(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/pptp_connected", "rb");
	if (!in)
		return;
	char pid[32];
	char ifname[32];
	char local[32];
	char remote[32];
	char peer[64];
	int count = 0;
	while (fscanf(in, "%s %s %s %s %s", pid, ifname, local, remote, peer) == 5) {
		websWrite(wp, "%c\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"", count ? ',' : ' ', ifname, peer, local, remote, pid);
		count++;
		if (feof(in))
			break;
	}

	fclose(in);
	return;
}
