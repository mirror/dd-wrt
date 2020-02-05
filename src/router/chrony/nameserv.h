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

  Module header for nameserver functions
  */


#ifndef GOT_NAMESERV_H
#define GOT_NAMESERV_H

#include "addressing.h"

typedef enum {
  DNS_Success,
  DNS_TryAgain,
  DNS_Failure
} DNS_Status;

/* Resolve names only to selected address family */
extern void DNS_SetAddressFamily(int family);

/* Maximum number of addresses returned by DNS_Name2IPAddress */
#define DNS_MAX_ADDRESSES 16

extern DNS_Status DNS_Name2IPAddress(const char *name, IPAddr *ip_addrs, int max_addrs);

extern int DNS_IPAddress2Name(IPAddr *ip_addr, char *name, int len);

extern void DNS_Reload(void);

#endif /* GOT_NAMESERV_H */

