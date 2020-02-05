/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014
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

  Header for asynchronous nameserver functions
  */


#ifndef GOT_NAMESERV_ASYNC_H
#define GOT_NAMESERV_ASYNC_H

#include "nameserv.h"

/* Function type for callback to process the result */
typedef void (*DNS_NameResolveHandler)(DNS_Status status, int n_addrs, IPAddr *ip_addrs, void *anything);

/* Request resolving of a name to IP address. The handler will be
   called when the result is available. */
extern void DNS_Name2IPAddressAsync(const char *name, DNS_NameResolveHandler handler, void *anything);

#endif
