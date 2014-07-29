/*
 * ProFTPD - mod_snmp packet routines
 * Copyright (c) 2008-2012 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: packet.h,v 1.1 2013/05/15 15:20:27 castaglia Exp $
 */

#include "mod_snmp.h"
#include "pdu.h"

#ifndef MOD_SNMP_PACKET_H
#define MOD_SNMP_PACKET_H

/* SNMP packets shouldn't be larger than 4K, right? */
#define SNMP_PACKET_MAX_LEN		4096

struct snmp_packet {
  pool *pool;

  pr_netaddr_t *remote_addr;
  pr_class_t *remote_class;

  /* Request packet data */
  unsigned char *req_data;
  size_t req_datalen;

  /* SNMP protocol version */
  long snmp_version;

  /* Community string for outgoing packets */
  char *community;
  unsigned int community_len;

  struct snmp_pdu *req_pdu;

  /* Response packet data */
  unsigned char *resp_data;
  size_t resp_datalen;

  struct snmp_pdu *resp_pdu;
};

struct snmp_packet *snmp_packet_create(pool *p);
int snmp_packet_write(pool *p, int sockfd, struct snmp_packet *pkt);

#endif
