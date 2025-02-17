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

#ifndef __NETWORK_H
#define __NETWORK_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define NETWORK_DBUS_SERVICE "org.bluez"
#define NETWORK_DBUS_INTERFACE "org.bluez.Network1"

/*
 * Type macros
 */
#define NETWORK_TYPE				(network_get_type())
#define NETWORK(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), NETWORK_TYPE, Network))
#define NETWORK_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), NETWORK_TYPE))
#define NETWORK_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), NETWORK_TYPE, NetworkClass))
#define NETWORK_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), NETWORK_TYPE))
#define NETWORK_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), NETWORK_TYPE, NetworkClass))

typedef struct _Network Network;
typedef struct _NetworkClass NetworkClass;
typedef struct _NetworkPrivate NetworkPrivate;

struct _Network {
	GObject parent_instance;

	/*< private >*/
	NetworkPrivate *priv;
};

struct _NetworkClass {
	GObjectClass parent_class;
};

/* used by NETWORK_TYPE */
GType network_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
Network *network_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *network_get_dbus_object_path(Network *self);

const gchar *network_connect(Network *self, const gchar *uuid, GError **error);
void network_disconnect(Network *self, GError **error);

GVariant *network_get_properties(Network *self, GError **error);
void network_set_property(Network *self, const gchar *name, const GVariant *value, GError **error);

gboolean network_get_connected(Network *self, GError **error);
const gchar *network_get_interface(Network *self, GError **error);
const gchar *network_get_uuid(Network *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __NETWORK_H */

