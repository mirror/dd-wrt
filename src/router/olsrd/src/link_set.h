/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: link_set.h,v 1.28 2005/10/23 20:58:14 tlopatic Exp $
 */


/*
 * Link sensing database for the OLSR routing daemon
 */

#ifndef _LINK_SET_H
#define _LINK_SET_H

#include "packet.h"

#define MID_ALIAS_HACK_VTIME  10.0

struct link_entry
{
  union olsr_ip_addr local_iface_addr;
  union olsr_ip_addr neighbor_iface_addr;
  clock_t SYM_time;
  clock_t ASYM_time;
  clock_t time;
  struct neighbor_entry *neighbor;
  olsr_u8_t prev_status;
  /*
   *Hysteresis
   */
  float L_link_quality;
  int L_link_pending;
  clock_t L_LOST_LINK_time;
  clock_t hello_timeout; /* When we should receive a new HELLO */
  double last_htime;
  olsr_u16_t olsr_seqno;
  olsr_bool olsr_seqno_valid;

  /*
   * packet loss
   */

  double loss_hello_int;
  clock_t loss_timeout;

  olsr_u16_t loss_seqno;
  int loss_seqno_valid;
  int loss_missed_hellos;

  int lost_packets;
  int total_packets;

  double loss_link_quality, loss_link_quality2;
  double loss_link_multiplier;

  int loss_window_size;
  int loss_index;

  unsigned char loss_bitmap[16];

  double neigh_link_quality, neigh_link_quality2;

  double saved_loss_link_quality;
  double saved_neigh_link_quality;

  struct link_entry *next;
};


extern struct link_entry *link_set;


/* Function prototypes */

struct link_entry *
get_link_set(void);

clock_t 
get_hold_time_neighbor(void);

void
olsr_init_link_set(void);

struct link_entry *
get_best_link_to_neighbor(union olsr_ip_addr *);

struct link_entry *
lookup_link_entry(union olsr_ip_addr *, union olsr_ip_addr *);

struct link_entry *
update_link_entry(union olsr_ip_addr *, union olsr_ip_addr *, struct hello_message *, struct interface *);

int
check_neighbor_link(union olsr_ip_addr *);

int
replace_neighbor_link_set(struct neighbor_entry *,
			  struct neighbor_entry *);

int
lookup_link_status(struct link_entry *);

void 
olsr_update_packet_loss_hello_int(struct link_entry *, double);

void 
olsr_update_packet_loss(union olsr_ip_addr *, union olsr_ip_addr *, olsr_u16_t);

void 
olsr_print_link_set(void);

void
olsr_update_dijkstra_link_qualities(void);

#endif
