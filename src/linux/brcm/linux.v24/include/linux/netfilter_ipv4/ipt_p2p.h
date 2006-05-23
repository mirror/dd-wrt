/*
 * $Id: ipt_p2p.h,v 1.9 2004/03/07 01:26:57 liquidk Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef __IPT_P2P_H
#define __IPT_P2P_H

#define IPT_P2P_VERSION "0.3.0a"

/*****************************************************************************/

#define IPT_P2P_PROTO_FASTTRACK        (0x01)  /* Minimum value for proto */
#define IPT_P2P_PROTO_GNUTELLA         (0x02)
#define IPT_P2P_PROTO_EDONKEY          (0x04)
#define IPT_P2P_PROTO_DIRECT_CONNECT   (0x08)
#define IPT_P2P_PROTO_BITTORRENT       (0x10)
#define IPT_P2P_PROTO_OPENFT           (0x20)
#define IPT_P2P_PROTO_ALL              (0xffff) /* Maximum value for proto */

/* Structure shared between the iptables_match module and the kernel's
   ipt_match module. */
struct ipt_p2p_info
{
	/* Application-layer peer-to-peer protocol(s) to match. */
	int proto;
};

/*****************************************************************************/

#endif /* __IPT_P2P_H */
