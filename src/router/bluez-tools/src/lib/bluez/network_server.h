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

#ifndef __NETWORK_SERVER_H
#define __NETWORK_SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define NETWORK_SERVER_DBUS_SERVICE "org.bluez"
#define NETWORK_SERVER_DBUS_INTERFACE "org.bluez.NetworkServer1"

/*
 * Type macros
 */
#define NETWORK_SERVER_TYPE				(network_server_get_type())
#define NETWORK_SERVER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), NETWORK_SERVER_TYPE, NetworkServer))
#define NETWORK_SERVER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), NETWORK_SERVER_TYPE))
#define NETWORK_SERVER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), NETWORK_SERVER_TYPE, NetworkServerClass))
#define NETWORK_SERVER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), NETWORK_SERVER_TYPE))
#define NETWORK_SERVER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), NETWORK_SERVER_TYPE, NetworkServerClass))

typedef struct _NetworkServer NetworkServer;
typedef struct _NetworkServerClass NetworkServerClass;
typedef struct _NetworkServerPrivate NetworkServerPrivate;

struct _NetworkServer {
	GObject parent_instance;

	/*< private >*/
	NetworkServerPrivate *priv;
};

struct _NetworkServerClass {
	GObjectClass parent_class;
};

/* used by NETWORK_SERVER_TYPE */
GType network_server_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
NetworkServer *network_server_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *network_server_get_dbus_object_path(NetworkServer *self);

void network_server_register(NetworkServer *self, const gchar *uuid, const gchar *bridge, GError **error);
void network_server_unregister(NetworkServer *self, const gchar *uuid, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __NETWORK_SERVER_H */

