/*
 *
 *  bluez-tools - a set of tools to manage bluetooth devices for linux
 *
 *  Copyright (C) 2010  Alexander Orlenko <zxteam@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __AGENT_HELPER_H
#define __AGENT_HELPER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "bluez-api.h"

#define AGENT_DBUS_INTERFACE "org.bluez.Agent1"
#define AGENT_PATH "/org/blueztools"

extern gboolean agent_need_unregister;

void register_agent_callbacks(gboolean interactive_console, GHashTable *pin_dictonary, gpointer main_loop_object, GError **error);
void unregister_agent_callbacks(GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __AGENT_HELPER_H */
