/*
 * lib/data.c		Abstract Data
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
 * @ingroup nl
 * @defgroup data Abstract Data
 * An abstract and generic way to handle arbitary data.
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <linux/socket.h>

/**
 * Allocate a new abstract data
 * @arg buf		data buffer containing the new data
 * @arg len		data buffer length
 * @arg data		destination abstract data
 *
 * Allocates a new abstract data and copies the specified data
 * buffer into the new handle.
 * 
 * @return 0 on success or a negative error code
 */
int nl_alloc_data(void *buf, size_t len, struct nl_data *data)
{
	if (len > 0) {
		data->d_size = len;
		data->d_data = calloc(1, len);

		if (data->d_data == NULL)
			return nl_error(ENOMEM, "Out of memory");

		if (buf)
			memcpy(data->d_data, buf, len);
	}

	return 0;
}

/**
 * Free up a abstract data handle
 * @arg data		abstract data
 */
void nl_free_data(struct nl_data *data)
{
	if (data->d_data) {
		free(data->d_data);
		data->d_data = NULL;
	}
}

/** @} */
