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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "count.h"

char *count_to_number(uint32_t num)
{
	uint32_t ptr = 0, size = 0;
	uint32_t written = 0, i;
	int new_line_every_n_numbers = 30;
	char *s;

	for (i=0; i < num; ++i) {
		size += snprintf(NULL, 0, "%u ", i);
		if (i > 0 && i % new_line_every_n_numbers == 0)
			size++;
	}
	size++; /* one for null char */

	s = calloc(size, sizeof(char));
	if (!s)
		goto out;

	for (i=0; i < num; ++i) {
		written = sprintf(&s[ptr], "%u ", i);
		ptr  += written;
		if (i > 0 && i % new_line_every_n_numbers == 0) {
			sprintf(&s[ptr], "\n");
			ptr++;
		}
	}

out:
	return s;
}
