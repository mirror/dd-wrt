/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
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
 * $Id: lq_packet.h,v 1.6 2005/02/20 18:52:18 kattemat Exp $
 */

#ifndef _OLSR_LQ_PACKET_H
#define _OLSR_LQ_PACKET_H

#include "olsr_types.h"

#define LQ_HELLO_MESSAGE      201
#define LQ_TC_MESSAGE         202

// deserialized OLSR header

struct olsr_common
{
  olsr_u8_t          type;
  double             vtime;
  olsr_u16_t         size;
  union olsr_ip_addr orig;
  olsr_u8_t          ttl;
  olsr_u8_t          hops;
  olsr_u16_t         seqno;
};

// serialized IPv4 OLSR header

struct olsr_header_v4
{
  olsr_u8_t  type;
  olsr_u8_t  vtime;
  olsr_u16_t size;
  olsr_u32_t orig;
  olsr_u8_t  ttl;
  olsr_u8_t  hops;
  olsr_u16_t seqno;
};

// serialized IPv6 OLSR header

struct olsr_header_v6
{
  olsr_u8_t     type;
  olsr_u8_t     vtime;
  olsr_u16_t    size;
  unsigned char orig[16];
  olsr_u8_t     ttl;
  olsr_u8_t     hops;
  olsr_u16_t    seqno;
};

// deserialized LQ_HELLO

struct lq_hello_neighbor
{
  olsr_u8_t                link_type;
  olsr_u8_t                neigh_type;
  double                   link_quality;
  double                   neigh_link_quality;
  union olsr_ip_addr       addr;
  struct lq_hello_neighbor *next;
};

struct lq_hello_message
{
  struct olsr_common       comm;
  double                   htime;
  olsr_u8_t                will;
  struct lq_hello_neighbor *neigh;
};

// serialized LQ_HELLO

struct lq_hello_info_header
{
  olsr_u8_t  link_code;
  olsr_u8_t  reserved;
  olsr_u16_t size;
};

struct lq_hello_header
{
  olsr_u16_t reserved;
  olsr_u8_t  htime;
  olsr_u8_t  will;
};

// deserialized LQ_TC

struct lq_tc_neighbor
{
  double                link_quality;
  double                neigh_link_quality;
  union olsr_ip_addr    main;
  struct lq_tc_neighbor *next;
};

struct lq_tc_message
{
  struct olsr_common    comm;
  union olsr_ip_addr    from;
  olsr_u16_t            ansn;
  struct lq_tc_neighbor *neigh;
};

// serialized LQ_TC

struct lq_tc_header
{
  olsr_u16_t ansn;
  olsr_u16_t reserved;
};

void olsr_output_lq_hello(void *para);

void olsr_output_lq_tc(void *para);

void olsr_input_lq_hello(union olsr_message *ser, struct interface *inif,
                         union olsr_ip_addr *from);

void olsr_input_lq_tc(union olsr_message *ser, struct interface *inif,
                      union olsr_ip_addr *from);

#endif
