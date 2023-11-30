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

#ifndef __UBUSD_ID_H
#define __UBUSD_ID_H

#include <libubox/avl.h>
#include <stdint.h>

struct ubus_id {
	struct avl_node avl;
	uint32_t id;
};

void ubus_init_id_tree(struct avl_tree *tree);
void ubus_init_string_tree(struct avl_tree *tree, bool dup);
bool ubus_alloc_id(struct avl_tree *tree, struct ubus_id *id, uint32_t val);

static inline void ubus_free_id(struct avl_tree *tree, struct ubus_id *id)
{
	avl_delete(tree, &id->avl);
}

static inline struct ubus_id *ubus_find_id(struct avl_tree *tree, uint32_t id)
{
	struct avl_node *avl;

	avl = avl_find(tree, &id);
	if (!avl)
		return NULL;

	return container_of(avl, struct ubus_id, avl);
}

#endif
