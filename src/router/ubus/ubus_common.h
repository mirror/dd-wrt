/*
 * Copyright (C) 2011 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __UBUS_COMMON_H
#define __UBUS_COMMON_H

#include <stdbool.h>

#define UBUS_SIGNATURE_METHOD	(BLOBMSG_TYPE_LAST + 1)
#define UBUS_SIGNATURE_END		(BLOBMSG_TYPE_LAST + 2)

static inline bool ubus_strmatch_len(const char *s1, const char *s2, int *len)
{
	for (*len = 0; s1[*len] == s2[*len]; (*len)++)
		if (!s1[*len])
			return true;

	return false;
}

#endif
