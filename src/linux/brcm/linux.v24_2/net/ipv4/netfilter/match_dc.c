/*
 * $Id: match_dc.c,v 1.4 2004/02/08 17:20:51 liquidk Exp $
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

#include <linux/netfilter_ipv4/ip_tables.h>


#define SIZE_MIN    (30)
#define SIZE_MAX    (200)

static const unsigned char *dc_cmd[] =
{
	"MyNick",
	"Lock",
	NULL
};

static const unsigned char *next_cmd(const unsigned char *data,
                                     const unsigned char *end)
{
	while (data <= end)
	{
		if (*data++ == '|')
			return data;
	}

	return NULL;
}

int
match_dc(const unsigned char *data,
         const unsigned char *end)
{
	int count=0;

	if (end - data < SIZE_MIN || end - data > SIZE_MAX) return 0;

	while (dc_cmd[count])
	{
		/* Quick exit. */
		if (*data != '$')
			return 0;

		if (end - data < strlen(dc_cmd[count]))
			return 0;

		if (memcmp(data + 1, dc_cmd[count], strlen(dc_cmd[count])) != 0)
			return 0;

		if (!(data = next_cmd(data, end)))
			return 0;

		count++;
	}

	return 1;
}
