/*
 * mdio-boardinfo.c - collect pre-declarations of PHY devices
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/mutex.h>
#include <linux/phy.h>

#include "mdio-boardinfo.h"

/*
 * These symbols are exported ONLY FOR the mdio_bus component.
 * No other users will be supported.
 */

LIST_HEAD(__mdio_board_list);
EXPORT_SYMBOL_GPL(__mdio_board_list);

DEFINE_MUTEX(__mdio_board_lock);
EXPORT_SYMBOL_GPL(__mdio_board_lock);

/**
 * mdio_register_board_info - register PHY devices for a given board
 * @info: array of chip descriptors
 * @n: how many descriptors are provided
 * Context: can sleep
 *
 * The board info passed can safely be __initdata ... but be careful of
 * any embedded pointers (platform_data, etc), they're copied as-is.
 */
int __init
mdiobus_register_board_info(struct mdio_board_info const *info, unsigned n)
{
	struct mdio_board_entry *be;
	int i;

	be = kzalloc(n * sizeof(*be), GFP_KERNEL);
	if (!be)
		return -ENOMEM;

	for (i = 0; i < n; i++, be++, info++) {
		memcpy(&be->board_info, info, sizeof(*info));
		mutex_lock(&__mdio_board_lock);
		list_add_tail(&be->list, &__mdio_board_list);
		mutex_unlock(&__mdio_board_lock);
	}

	return 0;
}
