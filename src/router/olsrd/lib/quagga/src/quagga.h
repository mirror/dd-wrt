
/***************************************************************************
 projekt              : olsrd-quagga
 file                 : quagga.h
 usage                : header for quagga.c
 copyright            : (C) 2006 by Immo 'FaUl' Wehrenberg
 e-mail               : immo@chaostreff-dortmund.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "routing_table.h"
#define HAVE_SOCKLEN_T

#ifndef ZEBRA_PORT
#define ZEBRA_PORT 2600
#endif

#ifdef ZEBRA_HEADER_MARKER
#ifndef ZSERV_VERSION
#define ZSERV_VERSION 1
#endif
#endif

struct ipv4_route {
  uint8_t type;
  uint8_t flags;
  uint8_t message;
  uint8_t prefixlen;
  uint32_t prefix;
  uint8_t nh_count;
  struct {
    uint8_t type;
    union {
      uint32_t v4;
    } payload;
  } *nexthops;
  uint8_t ind_num;
  uint32_t *index;
  uint32_t metric;
  uint8_t distance;
  struct ipv4_route *next;
};

void init_zebra(void);
void zebra_cleanup(void);
unsigned char zebra_send_command(unsigned char, unsigned char *, int);
int zebra_add_v4_route(const struct ipv4_route r);
int zebra_delete_v4_route(const struct ipv4_route r);
void zebra_check(void *);
int zebra_parse_packet(unsigned char *, ssize_t);
int zebra_redistribute(unsigned char);
int zebra_disable_redistribute(unsigned char);
int add_hna4_route(struct ipv4_route);
int delete_hna4_route(struct ipv4_route);
void *my_realloc(void *, size_t, const char *);
int zebra_add_olsr_v4_route(const struct rt_entry *);
int zebra_del_olsr_v4_route(const struct rt_entry *);
void zebra_olsr_localpref(void);
void zebra_olsr_distance(unsigned char);
void zebra_export_routes(unsigned char);

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
