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

#ifndef __HEALTH_DEVICE_H
#define __HEALTH_DEVICE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define HEALTH_DEVICE_DBUS_SERVICE "org.bluez"
#define HEALTH_DEVICE_DBUS_INTERFACE "org.bluez.HealthDevice1"

/*
 * Type macros
 */
#define HEALTH_DEVICE_TYPE				(health_device_get_type())
#define HEALTH_DEVICE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), HEALTH_DEVICE_TYPE, HealthDevice))
#define HEALTH_DEVICE_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), HEALTH_DEVICE_TYPE))
#define HEALTH_DEVICE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), HEALTH_DEVICE_TYPE, HealthDeviceClass))
#define HEALTH_DEVICE_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), HEALTH_DEVICE_TYPE))
#define HEALTH_DEVICE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), HEALTH_DEVICE_TYPE, HealthDeviceClass))

typedef struct _HealthDevice HealthDevice;
typedef struct _HealthDeviceClass HealthDeviceClass;
typedef struct _HealthDevicePrivate HealthDevicePrivate;

struct _HealthDevice {
	GObject parent_instance;

	/*< private >*/
	HealthDevicePrivate *priv;
};

struct _HealthDeviceClass {
	GObjectClass parent_class;
};

/* used by HEALTH_DEVICE_TYPE */
GType health_device_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
HealthDevice *health_device_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *health_device_get_dbus_object_path(HealthDevice *self);

const gchar *health_device_create_channel(HealthDevice *self, const gchar *application, const gchar *configuration, GError **error);
void health_device_destroy_channel(HealthDevice *self, const gchar *channel, GError **error);
gboolean health_device_echo(HealthDevice *self, GError **error);

GVariant *health_device_get_properties(HealthDevice *self, GError **error);
void health_device_set_property(HealthDevice *self, const gchar *name, const GVariant *value, GError **error);

const gchar *health_device_get_main_channel(HealthDevice *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __HEALTH_DEVICE_H */

