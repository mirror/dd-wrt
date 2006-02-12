/*
 * $Id: brctl_disp.c,v 1.1 2005/09/28 11:53:38 seg Exp $
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
#include "libbridge.h"
#include "brctl.h"

void br_dump_bridge_id(unsigned char *x)
{
	printf("%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x", x[0], x[1], x[2], x[3],
	       x[4], x[5], x[6], x[7]);
}

void br_show_timer(struct timeval *tv)
{
	printf("%4i.%.2i", (int)tv->tv_sec, (int)tv->tv_usec/10000);
}

void br_dump_port_info(struct port *p)
{
	char ifname[IFNAMSIZ];
	struct port_info *pi;

	pi = &p->info;

	printf("%s (%i)\n", if_indextoname(p->ifindex, ifname), p->index);
	printf(" port id\t\t%.4x\t\t\t", pi->port_id);
	printf("state\t\t\t%s\n", br_get_state_name(pi->state));
	printf(" designated root\t");
	br_dump_bridge_id((unsigned char *)&pi->designated_root);
	printf("\tpath cost\t\t%4i\n", pi->path_cost);

	printf(" designated bridge\t");
	br_dump_bridge_id((unsigned char *)&pi->designated_bridge);
	printf("\tmessage age timer\t");
	br_show_timer(&pi->message_age_timer_value);
	printf("\n designated port\t%.4x", pi->designated_port);
	printf("\t\t\tforward delay timer\t");
	br_show_timer(&pi->forward_delay_timer_value);
	printf("\n designated cost\t%4i", pi->designated_cost);
	printf("\t\t\thold timer\t\t");
	br_show_timer(&pi->hold_timer_value);
	printf("\n flags\t\t\t");
	if (pi->config_pending)
		printf("CONFIG_PENDING ");
	if (pi->top_change_ack)
		printf("TOPOLOGY_CHANGE_ACK ");
	printf("\n");
	printf("\n");
}

void br_dump_info(struct bridge *br)
{
	struct bridge_info *bri;
	struct port *p;

	bri = &br->info;

	printf("%s\n", br->ifname);
	if (!bri->stp_enabled) {
		printf("\tSTP disabled\n");
		return;
	}

	printf(" bridge id\t\t");
	br_dump_bridge_id((unsigned char *)&bri->bridge_id);
	printf("\n designated root\t");
	br_dump_bridge_id((unsigned char *)&bri->designated_root);
	printf("\n root port\t\t%4i\t\t\t", bri->root_port);
	printf("path cost\t\t%4i\n", bri->root_path_cost);
	printf(" max age\t\t");
	br_show_timer(&bri->max_age);
	printf("\t\t\tbridge max age\t\t");
	br_show_timer(&bri->bridge_max_age);
	printf("\n hello time\t\t");
	br_show_timer(&bri->hello_time);
	printf("\t\t\tbridge hello time\t");
	br_show_timer(&bri->bridge_hello_time);
	printf("\n forward delay\t\t");
	br_show_timer(&bri->forward_delay);
	printf("\t\t\tbridge forward delay\t");
	br_show_timer(&bri->bridge_forward_delay);
	printf("\n ageing time\t\t");
	br_show_timer(&bri->ageing_time);
	printf("\t\t\tgc interval\t\t");
	br_show_timer(&bri->gc_interval);
	printf("\n hello timer\t\t");
	br_show_timer(&bri->hello_timer_value);
	printf("\t\t\ttcn timer\t\t");
	br_show_timer(&bri->tcn_timer_value);
	printf("\n topology change timer\t");
	br_show_timer(&bri->topology_change_timer_value);
	printf("\t\t\tgc timer\t\t");
	br_show_timer(&bri->gc_timer_value);
	printf("\n flags\t\t\t");
	if (bri->topology_change)
		printf("TOPOLOGY_CHANGE ");
	if (bri->topology_change_detected)
		printf("TOPOLOGY_CHANGE_DETECTED ");
	printf("\n");
	printf("\n");
	printf("\n");

	p = br->firstport;
	while (p != NULL) {
		br_dump_port_info(p);
		p = p->next;
	}
}
