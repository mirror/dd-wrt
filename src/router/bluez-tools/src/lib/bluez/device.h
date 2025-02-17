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

#ifndef __DEVICE_H
#define __DEVICE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define DEVICE_DBUS_SERVICE "org.bluez"
#define DEVICE_DBUS_INTERFACE "org.bluez.Device1"

/*
 * Type macros
 */
#define DEVICE_TYPE				(device_get_type())
#define DEVICE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DEVICE_TYPE, Device))
#define DEVICE_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), DEVICE_TYPE))
#define DEVICE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DEVICE_TYPE, DeviceClass))
#define DEVICE_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), DEVICE_TYPE))
#define DEVICE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), DEVICE_TYPE, DeviceClass))

typedef struct _Device Device;
typedef struct _DeviceClass DeviceClass;
typedef struct _DevicePrivate DevicePrivate;

struct _Device {
	GObject parent_instance;

	/*< private >*/
	DevicePrivate *priv;
};

struct _DeviceClass {
	GObjectClass parent_class;
};

/* used by DEVICE_TYPE */
GType device_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
Device *device_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *device_get_dbus_object_path(Device *self);

void device_cancel_pairing(Device *self, GError **error);
void device_connect(Device *self, GError **error);
void device_connect_profile(Device *self, const gchar *uuid, GError **error);
void device_disconnect(Device *self, GError **error);
void device_disconnect_profile(Device *self, const gchar *uuid, GError **error);
void device_pair(Device *self, GError **error);
void device_pair_async(Device *self, GAsyncReadyCallback callback, gpointer user_data);
void device_pair_finish(Device *self, GAsyncResult *res, GError **error);

GVariant *device_get_properties(Device *self, GError **error);
void device_set_property(Device *self, const gchar *name, const GVariant *value, GError **error);

const gchar *device_get_adapter(Device *self, GError **error);
const gchar *device_get_address(Device *self, GError **error);
const gchar *device_get_alias(Device *self, GError **error);
void device_set_alias(Device *self, const gchar *value, GError **error);
guint16 device_get_appearance(Device *self, GError **error);
gboolean device_get_blocked(Device *self, GError **error);
void device_set_blocked(Device *self, const gboolean value, GError **error);
guint32 device_get_class(Device *self, GError **error);
gboolean device_get_connected(Device *self, GError **error);
const gchar *device_get_icon(Device *self, GError **error);
gboolean device_get_legacy_pairing(Device *self, GError **error);
const gchar *device_get_modalias(Device *self, GError **error);
const gchar *device_get_name(Device *self, GError **error);
gboolean device_get_paired(Device *self, GError **error);
gint16 device_get_rssi(Device *self, GError **error);
gboolean device_get_trusted(Device *self, GError **error);
void device_set_trusted(Device *self, const gboolean value, GError **error);
const gchar **device_get_uuids(Device *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __DEVICE_H */

