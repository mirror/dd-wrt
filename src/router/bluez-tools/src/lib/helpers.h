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

#ifndef __HELPERS_H
#define __HELPERS_H

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "bluez-api.h"

/* DBus helpers */
gboolean intf_supported(const gchar *dbus_service_name, const gchar *dbus_object_path, const gchar *intf_name);

/* BlueZ helpers */
Adapter *find_adapter(const gchar *name, GError **error);
Device *find_device(Adapter *adapter, const gchar *name, GError **error);

/* Others helpers */
#define exit_if_error(error) G_STMT_START{ \
if (error) { \
	g_printerr("%s: %s\n", (error->domain == G_DBUS_ERROR && g_dbus_error_get_remote_error(error) != NULL && strlen(g_dbus_error_get_remote_error(error)) ? g_dbus_error_get_remote_error(error) : "Error"), error->message); \
	exit(EXIT_FAILURE); \
}; }G_STMT_END

/* Convert hex string to int */
int xtoi(const gchar *str);

/* UUID converters */
const gchar *uuid2name(const gchar *uuid);
const gchar *name2uuid(const gchar *name);

/* FS helpers */
gboolean is_file(const gchar *filename, GError **error);
gboolean is_dir(const gchar *dirname, GError **error);
gboolean read_access(const gchar *path, GError **error);
gboolean write_access(const gchar *path, GError **error);
gchar *get_absolute_path(const gchar *path);

#endif /* __HELPERS_H */
