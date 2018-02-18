/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/*
 *Andreas Tonnesen
 */

#ifndef _OLSRD_FORNTEND_COMMON
#define _OLSRD_FORNTEND_COMMON

#include "../../../src/olsr_protocol.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
typedef unsigned char olsr_u8_t;
typedef unsigned short olsr_u16_t;
typedef unsigned int olsr_u32_t;
typedef char olsr_8_t;
typedef short olsr_16_t;
typedef int olsr_32_t;
#else /* _WIN32 */
typedef u_int8_t olsr_u8_t;
typedef u_int16_t olsr_u16_t;
typedef u_int32_t olsr_u32_t;
typedef int8_t olsr_8_t;
typedef int16_t olsr_16_t;
typedef int32_t olsr_32_t;
#endif /* _WIN32 */

#define VTIME_SCALE_FACTOR (0.0625)

/*extra: time to delete for non-wireless interfaces */
#define NEIGHB_HOLD_TIME_NW   NEIGHB_HOLD_TIME * 2

#define olsrd_version "olsr.org GUI 0.2.7"
#define IPC_INTERVAL 500        //interval for IPC read timeout
#define MAXPACKS 20
#define BUFFSIZE 512

extern int connected;
extern struct timeval hold_time_nodes; /* Timeout for all nodes */
extern struct timeval now;

/* Our address */
extern union olsr_ip_addr main_addr;
extern union olsr_ip_addr null_addr;

extern int ipversion;
extern int ipsize;
extern char ipv6_buf[100];                    /* buffer for IPv6 inet_htop */

extern int nodes_timeout;

extern int freeze_packets;
extern int display_dec;
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

void set_net_info_offline(void);

void update_nodes_list(struct node *);

int remove_nodes_list(union olsr_ip_addr *);

/*
 *IPC public
 */
int ipc_connect(struct sockaddr_in *pin);

int ipc_close(void);

int ipc_read(void);

int ipc_send(void);

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

void init_nodes(void);

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

#endif /* _OLSRD_FORNTEND_COMMON */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
