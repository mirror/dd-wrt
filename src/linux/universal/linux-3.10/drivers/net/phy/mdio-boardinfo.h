/*
 * mdio-boardinfo.h - boardinfo interface internal to the mdio_bus component
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/mutex.h>

struct mdio_board_entry {
	struct list_head	list;
	struct mdio_board_info	board_info;
};

/* __mdio_board_lock protects __mdio_board_list
 * only mdio_bus components are allowed to use these symbols.
 */
extern struct mutex __mdio_board_lock;
extern struct list_head __mdio_board_list;
