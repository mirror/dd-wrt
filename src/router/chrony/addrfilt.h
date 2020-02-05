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

  Module for providing an authorisation filter on IP addresses
  */

#ifndef GOT_ADDRFILT_H
#define GOT_ADDRFILT_H

#include "addressing.h"

typedef struct ADF_AuthTableInst *ADF_AuthTable;

typedef enum {
  ADF_SUCCESS,
  ADF_BADSUBNET
} ADF_Status;
  

/* Create a new table.  The default rule is deny for everything */
extern ADF_AuthTable ADF_CreateTable(void);

/* Allow anything in the supplied subnet, EXCEPT for any more specific
   subnets that are already defined */
extern ADF_Status ADF_Allow(ADF_AuthTable table,
                            IPAddr *ip,
                            int subnet_bits);

/* Allow anything in the supplied subnet, overwriting existing
   definitions for any more specific subnets */
extern ADF_Status ADF_AllowAll(ADF_AuthTable table,
                               IPAddr *ip,
                               int subnet_bits);

/* Deny anything in the supplied subnet, EXCEPT for any more specific
   subnets that are already defined */
extern ADF_Status ADF_Deny(ADF_AuthTable table,
                           IPAddr *ip,
                           int subnet_bits);

/* Deny anything in the supplied subnet, overwriting existing
   definitions for any more specific subnets */
extern ADF_Status ADF_DenyAll(ADF_AuthTable table,
                              IPAddr *ip,
                              int subnet_bits);

/* Clear up the table */
extern void ADF_DestroyTable(ADF_AuthTable table);

/* Check whether a given IP address is allowed by the rules in 
   the table */
extern int ADF_IsAllowed(ADF_AuthTable table,
                         IPAddr *ip);

/* Check if at least one address from a given family is allowed by
   the rules in the table */
extern int ADF_IsAnyAllowed(ADF_AuthTable table,
                            int family);

#endif /* GOT_ADDRFILT_H */
