/* $Id$ */
/*
 * Copyright (C) 2002-2011 Sourcefire, Inc.
 *
 * Author(s):  Andrew R. Baker <andrewb@sourcefire.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __IP_ADDR_SET_H__
#define __IP_ADDR_SET_H__

#include <sys/types.h>
#include "sf_types.h"

#ifdef SUP_IP6
# include "ipv6_port.h"
# include "sfutil/sf_ipvar.h"
#endif

#ifndef SUP_IP6
typedef struct _IpAddrNode
{
    uint32_t ip_addr;   /* IP addr */
    uint32_t netmask;   /* netmask */
    uint8_t  addr_flags; /* flag for normal/exception processing */

    struct _IpAddrNode *next;
} IpAddrNode;

typedef struct _IpAddrSet
{
    IpAddrNode *iplist;
    IpAddrNode *neg_iplist;
    uint32_t id;
} IpAddrSet;
#endif  /* SUP_IP6 */


void IpAddrSetDestroy(IpAddrSet *);
IpAddrSet *IpAddrSetParse(char *);


#ifndef SUP_IP6
/* flags */
#define EXCEPT_IP   0x01

void IpAddrSetPrint(char *prefix, IpAddrSet *);
IpAddrSet *IpAddrSetCopy(IpAddrSet *);
IpAddrSet *IpAddrSetCreate(void);
int IpAddrSetContains(IpAddrSet *, struct in_addr);

/* XXX legacy support function */
int ParseIP(char *paddr, IpAddrSet *, int); 
#endif  /* SUP_IP6 */

#endif  /* __IP_ADDR_SET_H__ */
