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

#ifndef __HEALTH_CHANNEL_H
#define __HEALTH_CHANNEL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define HEALTH_CHANNEL_DBUS_SERVICE "org.bluez"
#define HEALTH_CHANNEL_DBUS_INTERFACE "org.bluez.HealthChannel1"

/*
 * Type macros
 */
#define HEALTH_CHANNEL_TYPE				(health_channel_get_type())
#define HEALTH_CHANNEL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), HEALTH_CHANNEL_TYPE, HealthChannel))
#define HEALTH_CHANNEL_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), HEALTH_CHANNEL_TYPE))
#define HEALTH_CHANNEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), HEALTH_CHANNEL_TYPE, HealthChannelClass))
#define HEALTH_CHANNEL_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), HEALTH_CHANNEL_TYPE))
#define HEALTH_CHANNEL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), HEALTH_CHANNEL_TYPE, HealthChannelClass))

typedef struct _HealthChannel HealthChannel;
typedef struct _HealthChannelClass HealthChannelClass;
typedef struct _HealthChannelPrivate HealthChannelPrivate;

struct _HealthChannel {
	GObject parent_instance;

	/*< private >*/
	HealthChannelPrivate *priv;
};

struct _HealthChannelClass {
	GObjectClass parent_class;
};

/* used by HEALTH_CHANNEL_TYPE */
GType health_channel_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
HealthChannel *health_channel_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *health_channel_get_dbus_object_path(HealthChannel *self);

guint32 health_channel_acquire(HealthChannel *self, GError **error);
void health_channel_release(HealthChannel *self, GError **error);

GVariant *health_channel_get_properties(HealthChannel *self, GError **error);
void health_channel_set_property(HealthChannel *self, const gchar *name, const GVariant *value, GError **error);

const gchar *health_channel_get_application(HealthChannel *self, GError **error);
const gchar *health_channel_get_device(HealthChannel *self, GError **error);
// This has been renamed because 'health_channel_get_type' is already used by GLib
const gchar *health_channel_get_channel_type(HealthChannel *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __HEALTH_CHANNEL_H */

