/*
 * netlink/netlink-compat.h	Netlink Compatability
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef NETLINK_COMPAT_H_
#define NETLINK_COMPAT_H_

#if !defined _LINUX_SOCKET_H && !defined _BITS_SOCKADDR_H
typedef unsigned short  sa_family_t;
#endif

#ifndef IFNAMSIZ 
/** Maximum length of a interface name */
#define IFNAMSIZ 16
#endif

/* patch 2.4.x if_arp */
#ifndef ARPHRD_INFINIBAND
#define ARPHRD_INFINIBAND 32
#endif

/* patch 2.4.x eth header file */
#ifndef ETH_P_MPLS_UC
#define ETH_P_MPLS_UC  0x8847 
#endif

#ifndef ETH_P_MPLS_MC
#define ETH_P_MPLS_MC   0x8848
#endif

#ifndef  ETH_P_EDP2
#define ETH_P_EDP2      0x88A2
#endif

#ifndef ETH_P_HDLC
#define ETH_P_HDLC      0x0019 
#endif

#ifndef AF_LLC
#define AF_LLC		26
#endif

#endif
