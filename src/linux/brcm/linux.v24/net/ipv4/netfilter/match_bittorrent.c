/*
 * $Id: match_bittorrent.c,v 1.3 2004/02/08 17:20:51 liquidk Exp $
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

#define __NO_VERSION__

#include <linux/config.h>
#include <linux/module.h>


#define SIZE_MIN    (20)
#define SIZE_MAX    (500)

const unsigned char bittorrent_string[] =
    "\x13"
    "BitTorrent protocol"
    "\x0\x0\x0\x0\x0\x0\x0\x0";

int
match_bittorrent(const unsigned char *data,
                 const unsigned char *end)
{
	/* Only match if the header is within a certain size range, for
	   efficiency purposes. */
	if (end - data < SIZE_MIN || end - data > SIZE_MAX)
		return 0;

	if (memcmp(data, bittorrent_string, sizeof(bittorrent_string) - 1) == 0)
		return 1;

	return 0;
}
