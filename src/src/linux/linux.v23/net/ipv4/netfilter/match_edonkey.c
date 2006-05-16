/*
 * $Id: match_edonkey.c,v 1.4 2003/12/07 06:53:31 jasta Exp $
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

#define get_u8(X,O)   (*(__u8  *)(X + O))
#define get_u16(X,O)  (*(__u16 *)(X + O))
#define get_u32(X,O)  (*(__u32 *)(X + O))

#define EDONKEY_PACKET   (0xe3)

#define TYPE_HELLO       (0x01)
#define TYPE_HELLOANSWER (0x4c)

#define TYPE_HASH        (1)
#define TYPE_STRING      (2)
#define TYPE_DWORD       (3)
#define TYPE_FLOAT       (4)
#define TYPE_BOOL        (5)
#define TYPE_BOOLARRAY   (6)
#define TYPE_BLOB        (7)

#define POS_MAGIC        (0)
#define POS_LEN          (1)
#define POS_TYPE         (5)
#define POS_TAGCOUNT     (27)
#define POS_FIRSTTAG     (31)

#define SIZE_MIN         (30)
#define SIZE_MAX         (400)

int
match_edonkey(const unsigned char *data,
              const unsigned char *end)
{
	int packet_len;
	int tag_count;
	int type;

	if (end - data < POS_FIRSTTAG) return 0;
	if (get_u8(data, POS_MAGIC) != EDONKEY_PACKET) return 0;
	packet_len = get_u32(data, POS_LEN);

	if (packet_len < SIZE_MIN || packet_len > SIZE_MAX) return 0;

	type = get_u8(data, POS_TYPE);

	if (get_u8(data, POS_TYPE) == TYPE_HELLO)
		data++; /* Skip hash size */
	else if (get_u8(data, POS_TYPE) != TYPE_HELLOANSWER)
		return 0;

	tag_count = get_u32(data, POS_TAGCOUNT);
	if (tag_count < 2 || tag_count >= 6) {
		data++;
		tag_count = get_u32(data, POS_TAGCOUNT);
		if(tag_count < 2 || tag_count >= 6) return 0;
	}

	data += POS_FIRSTTAG;

	while(tag_count--) {
		int tag_type = get_u8(data, 0);
		int tag_len = get_u16(data, 1);
		data += 3 + tag_len;

		if (data > end) return 0;

		switch(tag_type) {
		 case TYPE_STRING:
			data += 2 + get_u16(data, 0);
			break;
		 case TYPE_DWORD:
		 case TYPE_FLOAT:
			data += 4;
			if (data > end) return 0;
			break;
		 default:
			return 0;
		}

		if (data > end) return 0;
	}

	return 1;
}
