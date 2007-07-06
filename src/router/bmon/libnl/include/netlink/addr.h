/*
 * netlink/addr.h		Abstract Address
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

#ifndef NETLINK_ADDR_H_
#define NETLINK_ADDR_H_

#include <netlink/netlink.h>

/**
 * Maximum length of addresses
 * @ingroup addr
 */
#define NL_ADDR_MAX		32

/**
 * Netlink address
 * @ingroup addr
 *
 * Represents an address, a family of AF_UNSPEC specifies
 * that the type of address is unknown.
 */
struct nl_addr
{
	/** Address family */
	int                 a_family;
	/** Address data */
	unsigned char       a_addr[NL_ADDR_MAX];
	/** Address length */
	unsigned int        a_len;
	/** Number of prefix bits */
	int                 a_prefix;
};


extern int	nl_addrcmp(struct nl_addr *, struct nl_addr *);
extern int	nl_addr_valid(char *, int);
extern int      nl_addr_guess_family(struct nl_addr *);

extern char *	nl_af2str_r(char, char *, size_t);
extern char *	nl_af2str(char);
extern char	nl_str2af(const char *);

extern char *	nl_addr2str_r(struct nl_addr *, char *, size_t);
extern char *	nl_addr2str(struct nl_addr *);
extern int	nl_str2addr(const char *, struct nl_addr *, int);

#endif
