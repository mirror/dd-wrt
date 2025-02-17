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

#ifndef __THERMOMETER_MANAGER_H
#define __THERMOMETER_MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define THERMOMETER_MANAGER_DBUS_SERVICE "org.bluez"
#define THERMOMETER_MANAGER_DBUS_INTERFACE "org.bluez.ThermometerManager1"

/*
 * Type macros
 */
#define THERMOMETER_MANAGER_TYPE				(thermometer_manager_get_type())
#define THERMOMETER_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), THERMOMETER_MANAGER_TYPE, ThermometerManager))
#define THERMOMETER_MANAGER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), THERMOMETER_MANAGER_TYPE))
#define THERMOMETER_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), THERMOMETER_MANAGER_TYPE, ThermometerManagerClass))
#define THERMOMETER_MANAGER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), THERMOMETER_MANAGER_TYPE))
#define THERMOMETER_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), THERMOMETER_MANAGER_TYPE, ThermometerManagerClass))

typedef struct _ThermometerManager ThermometerManager;
typedef struct _ThermometerManagerClass ThermometerManagerClass;
typedef struct _ThermometerManagerPrivate ThermometerManagerPrivate;

struct _ThermometerManager {
	GObject parent_instance;

	/*< private >*/
	ThermometerManagerPrivate *priv;
};

struct _ThermometerManagerClass {
	GObjectClass parent_class;
};

/* used by THERMOMETER_MANAGER_TYPE */
GType thermometer_manager_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ThermometerManager *thermometer_manager_new(const gchar *dbus_object_path);
const gchar *thermometer_manager_get_dbus_object_path(ThermometerManager *self);

#ifdef	__cplusplus
}
#endif

#endif /* __THERMOMETER_MANAGER_H */

