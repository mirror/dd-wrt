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

#ifndef __THERMOMETER_H
#define __THERMOMETER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define THERMOMETER_DBUS_SERVICE "org.bluez"
#define THERMOMETER_DBUS_INTERFACE "org.bluez.Thermometer1"

/*
 * Type macros
 */
#define THERMOMETER_TYPE				(thermometer_get_type())
#define THERMOMETER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), THERMOMETER_TYPE, Thermometer))
#define THERMOMETER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), THERMOMETER_TYPE))
#define THERMOMETER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), THERMOMETER_TYPE, ThermometerClass))
#define THERMOMETER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), THERMOMETER_TYPE))
#define THERMOMETER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), THERMOMETER_TYPE, ThermometerClass))

typedef struct _Thermometer Thermometer;
typedef struct _ThermometerClass ThermometerClass;
typedef struct _ThermometerPrivate ThermometerPrivate;

struct _Thermometer {
	GObject parent_instance;

	/*< private >*/
	ThermometerPrivate *priv;
};

struct _ThermometerClass {
	GObjectClass parent_class;
};

/* used by THERMOMETER_TYPE */
GType thermometer_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
Thermometer *thermometer_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *thermometer_get_dbus_object_path(Thermometer *self);

GVariant *thermometer_get_properties(Thermometer *self, GError **error);
void thermometer_set_property(Thermometer *self, const gchar *name, const GVariant *value, GError **error);

gboolean thermometer_get_intermediate(Thermometer *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __THERMOMETER_H */

