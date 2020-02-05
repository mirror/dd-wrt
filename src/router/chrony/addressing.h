/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Types used for addressing sources etc
  */

#ifndef GOT_ADDRESSING_H
#define GOT_ADDRESSING_H

#include "sysincl.h"

/* This type is used to represent an IPv4 address or IPv6 address.
   All parts are in HOST order, NOT network order. */

#define IPADDR_UNSPEC 0
#define IPADDR_INET4 1
#define IPADDR_INET6 2

typedef struct {
  union { 
    uint32_t in4;
    uint8_t in6[16];
  } addr;
  uint16_t family;
  uint16_t _pad;
} IPAddr;

typedef struct {
  IPAddr ip_addr;
  unsigned short port;
} NTP_Remote_Address;

#define INVALID_IF_INDEX -1

typedef struct {
  IPAddr ip_addr;
  int if_index;
  int sock_fd;
} NTP_Local_Address;

#endif /* GOT_ADDRESSING_H */

