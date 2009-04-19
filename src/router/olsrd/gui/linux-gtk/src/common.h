
/*
 * OLSR ad-hoc routing table management protocol GUI front-end
 * Copyright (C) 2003 Andreas Tonnesen (andreto@ifi.uio.no)
 *
 * This file is part of olsr.org.
 *
 * uolsrGUI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * uolsrGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsr.org; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 *Andreas Tonnesen
 */

#ifndef _OLSRD_FORNTEND_COMMON
#define _OLSRD_FORNTEND_COMMON

#include <gtk/gtk.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "olsr_protocol.h"

#define olsrd_version "olsr.org GUI 0.2.7"
#define IPC_INTERVAL 500        //interval for IPC read timeout
#define MAXPACKS 20
#define BUFFSIZE 512

extern int connected;
extern struct timeval hold_time_nodes; /* Timeout for all nodes */
extern struct timeval now;

/* Our address */
union olsr_ip_addr main_addr;
union olsr_ip_addr null_addr;

int ipversion;
int ipsize;
char ipv6_buf[100];                    /* buffer for IPv6 inet_htop */

int nodes_timeout;

int freeze_packets;
int display_dec;
extern int timeouts;

/*
 *Node info
 */

struct mid {
  union olsr_ip_addr alias;
  struct mid *next;
  struct mid *prev;
};

struct hna {
  union olsr_ip_addr net;
  union olsr_ip_addr mask;
  struct hna *next;
  struct hna *prev;
};

struct mpr {
  union olsr_ip_addr addr;
  struct timeval timer;
  struct mpr *next;
  struct mpr *prev;
};

struct node {
  union olsr_ip_addr addr;
  union olsr_ip_addr gw_addr;
  int hopcount;
  int display;
  char dev[5];
  struct mid mid;
  struct hna hna;
  struct mpr mpr;
  struct timeval timer;
  struct node *next;
  struct node *prev;
};

/*
 *Interface public
 */

GtkWidget *create_main_window(void);

void packet_list_add(char *, char *, char *);

void route_list_add(char *, char *, char *, char *);

int route_list_del(char *);

void route_list_update(char *);

void set_net_info(gchar *, int);

void set_net_info_offline();

void update_nodes_list(struct node *);

int remove_nodes_list(union olsr_ip_addr *);

/*
 *IPC public
 */
int ipc_connect();

int ipc_close();

int ipc_read();

int ipc_send();

char *ip_to_string(union olsr_ip_addr *);

int gui_itoa(int, char *);

/*
 *Packet.c public
 */

int add_packet_to_buffer(union olsr_message *, int);

union olsr_message *get_packet(int);

/*
 *Nodes.c public
 */

void init_nodes();

struct node *find_node(char *);

struct node *find_node_t(union olsr_ip_addr *);

int update_timer_node(union olsr_ip_addr *, olsr_u8_t);

int add_hna_node(union olsr_ip_addr *, union olsr_ip_addr *, union olsr_ip_addr *, olsr_u8_t);

int add_mid_node(union olsr_ip_addr *, union olsr_ip_addr *, olsr_u8_t);

void init_timer(olsr_u32_t, struct timeval *);

gint time_out_nodes(gpointer);

int add_node(union olsr_ip_addr *, olsr_u8_t);

int add_mpr(union olsr_ip_addr *, union olsr_ip_addr *, struct timeval *);

int update_timer_mpr(union olsr_ip_addr *, union olsr_ip_addr *, olsr_u8_t);

int time_out_mprs(union olsr_ip_addr *);

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
