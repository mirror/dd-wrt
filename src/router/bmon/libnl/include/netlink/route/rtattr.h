/*
 * netlink/route/rtattr.h	Routing Attribute
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

#ifndef NETLINK_RTATTR_H_
#define NETLINK_RTATTR_H_

#include <netlink/netlink.h>
#include <netlink/addr.h>
#include <netlink/data.h>

/**
 * Shortcut for nl_copy_data where sizeof(destination buffer) returns the true size
 * @ingroup rtattr
 * @arg DST		name destination buffer
 * @arg RTA		routing attribute
 */
#define NL_COPY_DATA(DST, RTA) \
	nl_copy_data(&(DST), sizeof((DST)), RTA)
/**
 * Shortcut for nl_parse_rtattr for nested routing attributes
 * @ingroup rtattr
 * @arg _T		tb buffer
 * @arg _M		maximum TLV type
 * @arg _R		routing attribute containing a routing attribute arrays
 */
#define nl_parse_nested(_T,_M,_R) \
	nl_parse_rtattr(_T,_M,RTA_DATA(_R),RTA_PAYLOAD(_R))

/**
 * Required length for a tb buffer
 * @ingroup rtattr
 * @arg a		maximum TLV type
 */
#define NL_TB_LENGTH(a) (sizeof(struct rtattr *) * (a))

/**
 * Number of elements in TLV holding an array
 * @ingroup rtattr
 * @param rta		TLV attribute (struct rtattr *)
 * @param length	Length of element type
 */
#define RTA_ARRAY_ELEMS(rta,length) \
	(RTA_PAYLOAD((rta)) / RTA_LENGTH(NLMSG_ALIGN(length)))


extern int	nl_copy_data(void *dst, size_t len, struct rtattr *rta);
extern void	nl_copy_addr(struct nl_addr *addr, struct rtattr *rta);
extern int	nl_alloc_data_from_rtattr(struct nl_data *d, struct rtattr *rta);
extern int	nl_parse_rtattr(struct rtattr *attr[], size_t max,
                		struct rtattr *rta, size_t len);
extern int 	nl_msg_append_tlv(struct nl_msg *, int, void *, size_t);
extern int	nl_msg_append_nested(struct nl_msg *, int, struct nl_msg *);
extern int	nl_msg_parse_rtattr(struct rtattr **, int, struct nl_msg *);
extern int	nl_msg_parse_rtattr_off(struct rtattr **, int, struct nl_msg *, int);

#endif
