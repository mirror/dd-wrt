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
 * $Id: tc_set.h,v 1.15 2005/05/29 12:47:46 br1 Exp $
 */

#ifndef _OLSR_TOP_SET
#define _OLSR_TOP_SET

#include "defs.h"
#include "packet.h"

struct topo_dst
{
  union olsr_ip_addr T_dest_addr;
  clock_t            T_time;
  olsr_u16_t         T_seq;
  struct topo_dst   *next;
  struct topo_dst   *prev;
  double             link_quality;
  double             inverse_link_quality;
  double             saved_link_quality;
  double             saved_inverse_link_quality;
};


struct tc_entry
{
  union olsr_ip_addr T_last_addr;
  struct topo_dst    destinations;
  struct tc_entry   *next;
  struct tc_entry   *prev;
};


/* Queue */
extern struct tc_entry tc_table[HASHSIZE];


int
olsr_init_tc(void);


int
olsr_tc_delete_mprs(struct tc_entry *, struct tc_message *);


struct tc_entry *
olsr_lookup_tc_entry(union olsr_ip_addr *);


struct topo_dst *
olsr_tc_lookup_dst(struct tc_entry *, union olsr_ip_addr *);


int
olsr_tc_delete_entry_if_empty(struct tc_entry *);


struct tc_entry *
olsr_add_tc_entry(union olsr_ip_addr *);


int
olsr_tc_update_mprs(struct tc_entry *, struct tc_message *);

int
olsr_print_tc_table(void);

void
olsr_time_out_tc_set(void);

#endif
