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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libubox/avl-cmp.h>

#include "ubusmsg.h"
#include "ubusd_id.h"

static int random_fd = -1;

static int ubus_cmp_id(const void *k1, const void *k2, void *ptr)
{
	const uint32_t *id1 = k1, *id2 = k2;

	if (*id1 < *id2)
		return -1;
	else
		return *id1 > *id2;
}

void ubus_init_string_tree(struct avl_tree *tree, bool dup)
{
	avl_init(tree, avl_strcmp, dup, NULL);
}

void ubus_init_id_tree(struct avl_tree *tree)
{
	if (random_fd < 0) {
		random_fd = open("/dev/urandom", O_RDONLY);
		if (random_fd < 0) {
			perror("open");
			exit(1);
		}
	}

	avl_init(tree, ubus_cmp_id, false, NULL);
}

bool ubus_alloc_id(struct avl_tree *tree, struct ubus_id *id, uint32_t val)
{
	id->avl.key = &id->id;
	if (val) {
		id->id = val;
		return avl_insert(tree, &id->avl) == 0;
	}

	do {
		if (read(random_fd, &id->id, sizeof(id->id)) != sizeof(id->id))
			return false;

		if (id->id < UBUS_SYSTEM_OBJECT_MAX)
			continue;
	} while (avl_insert(tree, &id->avl) != 0);

	return true;
}

