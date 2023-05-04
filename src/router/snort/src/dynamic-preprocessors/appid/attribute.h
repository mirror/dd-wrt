/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _IOR_SIZE_MASK  0xFF000000
#define _IOR_TYPE_MASK  0x00FFFFFF
#define _IOR(x, y, z, t)	\
	((((sizeof(t)) & 0xFF) << 24) | (((x) & 0xFF) << 16) | (((y) & 0xFF) << 8) | ((z) & 0xFF))

#define ATTR_SIZE_MASK  _IOR_SIZE_MASK
#define ATTR_TYPE_MASK  _IOR_TYPE_MASK
typedef void *pointer_t;

/* Ethernet */
#define _ATTR_ETH(a, id, t)		_IOR('e', a, id, t)
#define _ATTR_ETH_HDR(id, t)	_ATTR_ETH(1, id, t)

#define ATTR_ETH_HDR_RAW		_ATTR_ETH_HDR(0, pointer_t)
#define ATTR_ETH_HDR_DST		_ATTR_ETH_HDR(1, uint8_t[6])
#define ATTR_ETH_HDR_SRC		_ATTR_ETH_HDR(2, uint8_t[6])
#define ATTR_ETH_HDR_8021Q		_ATTR_ETH_HDR(3, uint32_t)
#define ATTR_ETH_HDR_TYPE		_ATTR_ETH_HDR(4, uint16_t)

/* IPv4 */
#define _ATTR_IP(a, id, t)		_IOR('i', a, id, t)
#define _ATTR_IP_HDR(id, t)		_ATTR_IP(1, id, t)
#define _ATTR_IP_SYS(id, t)		_ATTR_IP(2, id, t)

/* IPv4 Header Attributes */
#define ATTR_IP_HDR_RAW			_ATTR_IP_HDR(0, pointer_t)
#define ATTR_IP_HDR_IHL			_ATTR_IP_HDR(1, uint16_t)
#define ATTR_IP_HDR_LEN			_ATTR_IP_HDR(2, uint16_t)
#define ATTR_IP_HDR_TOS			_ATTR_IP_HDR(3, uint8_t)
#define ATTR_IP_HDR_ID			_ATTR_IP_HDR(4, uint16_t)
#define ATTR_IP_HDR_TTL			_ATTR_IP_HDR(5, uint8_t)
#define ATTR_IP_HDR_PTCL		_ATTR_IP_HDR(6, uint8_t)
#define ATTR_IP_HDR_CSUM		_ATTR_IP_HDR(7, uint16_t)
#define ATTR_IP_HDR_SRC			_ATTR_IP_HDR(8, uint32_t)
#define ATTR_IP_HDR_DST			_ATTR_IP_HDR(9, uint32_t)

/* IPv4 System Attributes */
#define ATTR_IP_SYS_CID			_ATTR_IP_SYS(1, uint32_t)

/* TCP */
#define _ATTR_TCP(a, id, t)		_IOR('t', a, id, t)
#define _ATTR_TCP_HDR(id, t)	_ATTR_TCP(1, id, t)
#define _ATTR_TCP_SYS(id, t)	_ATTR_TCP(2, id, t)
#define _ATTR_TCP_APP(id, t)	_ATTR_TCP(3, id, t)

/* TCP Header Attributes */
#define ATTR_TCP_HDR_RAW		_ATTR_TCP_HDR(0, pointer_t)
#define ATTR_TCP_HDR_SRC		_ATTR_TCP_HDR(1, uint16_t)
#define ATTR_TCP_HDR_DST		_ATTR_TCP_HDR(2, uint16_t)

/* TCP System Attributes */
#define ATTR_TCP_SYS_RETX		_ATTR_TCP_SYS(1, uint8_t)

/* TCP Port-based Application Attribute */
#define ATTR_TCP_APP_ID			_ATTR_TCP_APP(1, uint16_t)

/* UDP */
#define _ATTR_UDP(a, id, t)		_IOR('u', a, id, t)
#define _ATTR_UDP_HDR(id, t)	_ATTR_UDP(1, id, t)
#define _ATTR_UDP_APP(id, t)	_ATTR_UDP(2, id, t)

/* UDP Header Attributes */
#define ATTR_UDP_HDR_RAW		_ATTR_UDP_HDR(0, pointer_t)
#define ATTR_UDP_HDR_SRC		_ATTR_UDP_HDR(1, uint16_t)
#define ATTR_UDP_HDR_DST		_ATTR_UDP_HDR(2, uint16_t)

/* UDP Port-based Application Attribute */
#define ATTR_UDP_APP_ID			_ATTR_UDP_APP(1, uint16_t)

/* HTTP */
#define _ATTR_HTTP(a, t)		_IOR('h', a, 0, t)
#define ATTR_HTTP_HOST			_ATTR_HTTP(0, pointer_t)
#define ATTR_HTTP_CTYPE			_ATTR_HTTP(1, pointer_t)
#define ATTR_HTTP_URL			_ATTR_HTTP(2, pointer_t)

/* FACEBOOK */
#define _ATTR_FACEBOOK(a, t)	_IOR('f', a, 0, t)
#define ATTR_FACEBOOK_APP		_ATTR_FACEBOOK(0, pointer_t)

/* CITRIX */
#define _ATTR_CITRIX(a, t)		_IOR('c', a, 0, t)
#define ATTR_CITRIX_PRIORITY	_ATTR_CITRIX(0, uint8_t)

/* YAHOO */
#define _ATTR_YMSGFILE(a, t)	_IOR('y', a, 0, t)
#define ATTR_YMSGFILE_NAME		_ATTR_YMSGFILE(0, pointer_t)

#ifdef __cplusplus
}
#endif

#endif
