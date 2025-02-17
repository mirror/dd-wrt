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

#ifndef __HEART_RATE_H
#define __HEART_RATE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define HEART_RATE_DBUS_SERVICE "org.bluez"
#define HEART_RATE_DBUS_INTERFACE "org.bluez.HeartRate1"

/*
 * Type macros
 */
#define HEART_RATE_TYPE				(heart_rate_get_type())
#define HEART_RATE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), HEART_RATE_TYPE, HeartRate))
#define HEART_RATE_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), HEART_RATE_TYPE))
#define HEART_RATE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), HEART_RATE_TYPE, HeartRateClass))
#define HEART_RATE_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), HEART_RATE_TYPE))
#define HEART_RATE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), HEART_RATE_TYPE, HeartRateClass))

typedef struct _HeartRate HeartRate;
typedef struct _HeartRateClass HeartRateClass;
typedef struct _HeartRatePrivate HeartRatePrivate;

struct _HeartRate {
	GObject parent_instance;

	/*< private >*/
	HeartRatePrivate *priv;
};

struct _HeartRateClass {
	GObjectClass parent_class;
};

/* used by HEART_RATE_TYPE */
GType heart_rate_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
HeartRate *heart_rate_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *heart_rate_get_dbus_object_path(HeartRate *self);

GVariant *heart_rate_get_properties(HeartRate *self, GError **error);
void heart_rate_set_property(HeartRate *self, const gchar *name, const GVariant *value, GError **error);

gboolean heart_rate_get_reset_supported(HeartRate *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __HEART_RATE_H */

