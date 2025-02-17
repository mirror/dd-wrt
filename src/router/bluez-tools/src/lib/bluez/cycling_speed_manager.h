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

#ifndef __CYCLING_SPEED_MANAGER_H
#define __CYCLING_SPEED_MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define CYCLING_SPEED_MANAGER_DBUS_SERVICE "org.bluez"
#define CYCLING_SPEED_MANAGER_DBUS_INTERFACE "org.bluez.CyclingSpeedManager1"

/*
 * Type macros
 */
#define CYCLING_SPEED_MANAGER_TYPE				(cycling_speed_manager_get_type())
#define CYCLING_SPEED_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), CYCLING_SPEED_MANAGER_TYPE, CyclingSpeedManager))
#define CYCLING_SPEED_MANAGER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), CYCLING_SPEED_MANAGER_TYPE))
#define CYCLING_SPEED_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), CYCLING_SPEED_MANAGER_TYPE, CyclingSpeedManagerClass))
#define CYCLING_SPEED_MANAGER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), CYCLING_SPEED_MANAGER_TYPE))
#define CYCLING_SPEED_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), CYCLING_SPEED_MANAGER_TYPE, CyclingSpeedManagerClass))

typedef struct _CyclingSpeedManager CyclingSpeedManager;
typedef struct _CyclingSpeedManagerClass CyclingSpeedManagerClass;
typedef struct _CyclingSpeedManagerPrivate CyclingSpeedManagerPrivate;

struct _CyclingSpeedManager {
	GObject parent_instance;

	/*< private >*/
	CyclingSpeedManagerPrivate *priv;
};

struct _CyclingSpeedManagerClass {
	GObjectClass parent_class;
};

/* used by CYCLING_SPEED_MANAGER_TYPE */
GType cycling_speed_manager_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
CyclingSpeedManager *cycling_speed_manager_new(const gchar *dbus_object_path);
const gchar *cycling_speed_manager_get_dbus_object_path(CyclingSpeedManager *self);

#ifdef	__cplusplus
}
#endif

#endif /* __CYCLING_SPEED_MANAGER_H */

