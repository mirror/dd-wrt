/*
 * Copyright (C) 2007-2011 B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <lindner_marek@yahoo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include <netpacket/packet.h>
#include "list-batman.h"

#define DUMP_TYPE_BATOGM 1
#define DUMP_TYPE_BATICMP 2
#define DUMP_TYPE_BATUCAST 4
#define DUMP_TYPE_BATBCAST 8
#define DUMP_TYPE_BATVIS 16
#define DUMP_TYPE_BATFRAG 32
#define DUMP_TYPE_NONBAT 64

struct dump_if {
	struct list_head list;
	char *dev;
	int32_t raw_sock;
	struct sockaddr_ll addr;
};

struct vlanhdr {
	unsigned short vid;
	u_int16_t ether_type;
} __attribute__ ((packed));

int tcpdump(int argc, char **argv);
