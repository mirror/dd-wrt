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
 * $Id: mid_set.h,v 1.13 2005/05/29 12:47:45 br1 Exp $
 */


#ifndef _OLSR_MID
#define _OLSR_MID

#include "olsr_types.h"

struct mid_address
{
  union olsr_ip_addr  alias;
  struct mid_entry   *main_entry;

  struct mid_address *next_alias;

  /* These are for the reverse list */
  struct mid_address *prev;
  struct mid_address *next;
};

/*
 *Contains the main addr of a node and a list of aliases
 */
struct mid_entry
{
  union olsr_ip_addr  main_addr;
  struct mid_address *aliases;
  struct mid_entry   *prev;
  struct mid_entry   *next;
  clock_t             ass_timer;  
};


extern struct mid_entry mid_set[HASHSIZE];
extern struct mid_address reverse_mid_set[HASHSIZE];


int
olsr_init_mid_set(void);

void 
insert_mid_tuple(union olsr_ip_addr *, struct mid_address *, float);

void
insert_mid_alias(union olsr_ip_addr *, union olsr_ip_addr *, float);

union olsr_ip_addr *
mid_lookup_main_addr(union olsr_ip_addr *);

struct mid_address *
mid_lookup_aliases(union olsr_ip_addr *);

void
olsr_print_mid_set(void);

void
olsr_time_out_mid_set(void *);

int
olsr_update_mid_table(union olsr_ip_addr *, float);

int
mid_delete_node(struct mid_entry *);

#endif
