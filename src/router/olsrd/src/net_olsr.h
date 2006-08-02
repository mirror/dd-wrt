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
 * $Id: net_olsr.h,v 1.1 2005/05/25 20:59:46 kattemat Exp $
 */



#ifndef _NET_OLSR
#define _NET_OLSR

#include "defs.h"
#include "process_routes.h"
#include <arpa/inet.h>
#include <net/if.h>

struct deny_address_entry
{
  union olsr_ip_addr        addr;
  struct deny_address_entry *next;
};


/* Output buffer structure */

struct olsr_netbuf
{
  char *buff;     /* Pointer to the allocated buffer */
  int if_index;
  int bufsize;    /* Size of the buffer */
  int maxsize;    /* Max bytes of payload that can be added to the buffer */
  int pending;    /* How much data is currently pending in the buffer */
  int reserved;   /* Plugins can reserve space in buffers */
};

void
init_net(void);

int
net_add_buffer(struct interface *);

int
net_remove_buffer(struct interface *);

int
net_outbuffer_bytes_left(struct interface *);

olsr_u16_t
net_output_pending(struct interface *);

int
net_reserve_bufspace(struct interface *, int);

int
net_outbuffer_push(struct interface *, olsr_u8_t *, olsr_u16_t);

int
net_outbuffer_push_reserved(struct interface *, olsr_u8_t *, olsr_u16_t);

int
net_output(struct interface*);

int
net_sendroute(struct rt_entry *, struct sockaddr *);

int
olsr_prefix_to_netmask(union olsr_ip_addr *, olsr_u16_t);

olsr_u16_t
olsr_netmask_to_prefix(union olsr_ip_addr *);

char *
sockaddr_to_string(struct sockaddr *);

char *
ip_to_string(olsr_u32_t *);

char *
ip6_to_string(struct in6_addr *);

char *
olsr_ip_to_string(union olsr_ip_addr *);

int
add_ptf(int (*)(char *, int *));

int
del_ptf(int (*f)(char *, int *));

olsr_bool
olsr_validate_address(union olsr_ip_addr *);

void
olsr_add_invalid_address(union olsr_ip_addr *);

#endif
