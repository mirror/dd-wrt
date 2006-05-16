/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: comhandler.h,v 1.3 2005/05/05 14:25:41 jordan Exp $
 */

#ifndef __SHAT_COMMHANDLER_H
#define __SHAT_COMMHANDLER_H

#include "arp.h"
#include "ip2ether.h"
#include "lookup.h"
#include "ctrl.h"

#ifndef DISCARD_CMD_SOCKET

/* communication through the control socket */
typedef
struct _comm {
#ifdef __COMHANDLER_PRIVATE
    time_t started ;
    LOOKUP *lookup ;
    CTRL     *ctrl ;
    ARP       *arp ;
    IP2E     *ip2e ;
    REGISTER  *reg ;
    char   visited ; /* local flag */
#else
    char any ;
#endif
} COMM;

extern COMM *comm_init      (void);
extern COMM *comm_update    (COMM*,LOOKUP*,ARP*,IP2E*,CTRL*,REGISTER*);
extern void  comm_delete_slot (void *,int,const char**);
extern void  comm_delete_ip   (void *,int,const char**);
extern void  comm_find_mac    (void *,int,const char**);
extern void  comm_find_ip     (void *,int,const char**);
extern void  comm_find_slot   (void *,int,const char**);
extern void  comm_list        (void *,int,const char**);
extern void  comm_reg_list    (void *,int,const char**);
extern void  comm_info        (void *,int,const char**);
extern void  comm_ping_slot   (void *,int,const char**);
#endif /* DISCARD_CMD_SOCKET */

#endif /* __SHAT_COMMHANDLER_H */
