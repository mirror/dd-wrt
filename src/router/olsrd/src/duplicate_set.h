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
 * $Id: duplicate_set.h,v 1.10 2005/02/20 18:52:18 kattemat Exp $
 */

#ifndef _OLSR_DUP_TABLE
#define _OLSR_DUP_TABLE

#include "olsr_types.h"

#define UNKNOWN_MESSAGE 0

struct dup_entry
{
  union olsr_ip_addr     addr;      /* IP address of originator */
  olsr_u16_t             seqno;     /* Seqno of message */
  clock_t                timer;	    /* Holding time */
  struct dup_iface       *ifaces;   /* Interfaces this message was recieved on */
  olsr_u8_t              forwarded; /* If this message was forwarded or not */
  struct dup_entry       *next;     /* Next entry */
  struct dup_entry       *prev;     /* Prev entry */
};

struct dup_iface
{
  union olsr_ip_addr     addr;      /* Addess of the interface */
  struct dup_iface       *next;     /* Next in line */
};


void
olsr_init_duplicate_table(void);

void
olsr_time_out_duplicate_table(void *);

int
olsr_check_dup_table_proc(union olsr_ip_addr *, olsr_u16_t);

int
olsr_check_dup_table_fwd(union olsr_ip_addr *, olsr_u16_t, union olsr_ip_addr *);

void
olsr_del_dup_entry(struct dup_entry *);

void
olsr_print_duplicate_table(void);

struct dup_entry *
olsr_add_dup_entry(union olsr_ip_addr *, olsr_u16_t);

int
olsr_update_dup_entry(union olsr_ip_addr *, olsr_u16_t, union olsr_ip_addr *);

int
olsr_set_dup_forward(union olsr_ip_addr *, olsr_u16_t);

int
olsr_check_dup_forward(union olsr_ip_addr *, olsr_u16_t);

#endif
