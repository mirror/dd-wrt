/*
 * netlink/data.h	Abstract Data
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

#ifndef NETLINK_DATA_H_
#define NETLINK_DATA_H_

#include <netlink/netlink.h>

/**
 * Random netlink data
 * @ingroup data
 */
struct nl_data
{
	/** Actual data (allocated) */
	void *              d_data;
	/** Size of data */
	size_t              d_size;
};

extern int	nl_alloc_data(void *, size_t, struct nl_data *);
extern void	nl_free_data(struct nl_data *d);


#endif
