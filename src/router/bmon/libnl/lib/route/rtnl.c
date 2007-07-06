/*
 * rtnl.c           Routing Netlink
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

/**
 * @defgroup rtnl Routing Netlink
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>

/**
 * Send a routing netlink request message
 * @arg handle		netlink handle
 * @arg type		message type (see \ref rtnlmsgtypes)
 * @arg family		address family
 * @arg flags		netlink message flags (see \ref nlmsgflags)
 *
 * Fills out a routing netlink request message and sends it out
 * using nl_send_auto_complete().
 *
 * @return 0 on success or a negative error code (See nl_send_auto_complete())
 */
int nl_rtgen_request(struct nl_handle *handle, int type, int family, int flags)
{
	int err = 0;
	struct nl_msg *m;

	struct nlmsghdr n = {
		.nlmsg_len = NLMSG_LENGTH(0),
		.nlmsg_type = type,
		.nlmsg_flags = flags,
	};
	
	struct rtgenmsg gmsg = {
		.rtgen_family = family,
	};

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &gmsg, sizeof(gmsg));

	err = nl_send_auto_complete(handle, nl_msg_get(m));
	nl_msg_free(m);
	return err;
}

/**
 * @name Routing Type Translations
 * @{
 */

static struct trans_tbl rtntypes[] = {
	__ADD(RTN_UNSPEC,unspec)
	__ADD(RTN_UNICAST,unicast)
	__ADD(RTN_LOCAL,local)
	__ADD(RTN_BROADCAST,broadcast)
	__ADD(RTN_ANYCAST,anycast)
	__ADD(RTN_MULTICAST,multicast)
	__ADD(RTN_BLACKHOLE,blackhole)
	__ADD(RTN_UNREACHABLE,unreachable)
	__ADD(RTN_PROHIBIT,prohibit)
	__ADD(RTN_THROW,throw)
	__ADD(RTN_NAT,nat)
	__ADD(RTN_XRESOLVE,xresolve)
};

/**
 * Convert a routing type to a character string.
 * @arg type		routing type
 *
 * Converts a routing type to a character string and stores it in a
 * static buffer.
 *
 * @return A static buffer or the type encoded in hexidecimal
 *         form if no match was found.
 * @attention This funnction is NOT thread safe.
 */
char * nl_rtntype2str(int type)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(type, buf, sizeof(buf), rtntypes,
	    ARRAY_SIZE(rtntypes));
}

/**
 * Convert a routing type to a character string (Reentrant).
 * @arg type		routing type
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a routing type to a character string and stores it in
 * the specified destination buffer.
 *
 * @return The destination buffer or the type encoded in hexidecimal
 *         form if no match was found.
 */
char * nl_rtntype2str_r(int type, char *buf, size_t len)
{
	return __type2str_r(type, buf, len, rtntypes, ARRAY_SIZE(rtntypes));
}

/**
 * Convert a character string to a routing type
 * @arg name		Name of routing type
 *
 * Converts the provided character string specifying a routing
 * type to the corresponding numeric value.
 *
 * @return Routing type negative value if none was found.
 */
int nl_str2rtntype(const char *name)
{
	return __str2type(name, rtntypes, ARRAY_SIZE(rtntypes));
}

/** @} */
/** @} */
