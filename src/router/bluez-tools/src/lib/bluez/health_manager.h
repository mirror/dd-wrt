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

#ifndef __HEALTH_MANAGER_H
#define __HEALTH_MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define HEALTH_MANAGER_DBUS_SERVICE "org.bluez"
#define HEALTH_MANAGER_DBUS_INTERFACE "org.bluez.HealthManager1"
#define HEALTH_MANAGER_DBUS_PATH "/org/bluez/"

/*
 * Type macros
 */
#define HEALTH_MANAGER_TYPE				(health_manager_get_type())
#define HEALTH_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), HEALTH_MANAGER_TYPE, HealthManager))
#define HEALTH_MANAGER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), HEALTH_MANAGER_TYPE))
#define HEALTH_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), HEALTH_MANAGER_TYPE, HealthManagerClass))
#define HEALTH_MANAGER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), HEALTH_MANAGER_TYPE))
#define HEALTH_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), HEALTH_MANAGER_TYPE, HealthManagerClass))

typedef struct _HealthManager HealthManager;
typedef struct _HealthManagerClass HealthManagerClass;
typedef struct _HealthManagerPrivate HealthManagerPrivate;

struct _HealthManager {
	GObject parent_instance;

	/*< private >*/
	HealthManagerPrivate *priv;
};

struct _HealthManagerClass {
	GObjectClass parent_class;
};

/* used by HEALTH_MANAGER_TYPE */
GType health_manager_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
HealthManager *health_manager_new();

/*
 * Method definitions
 */
const gchar *health_manager_create_application(HealthManager *self, const GVariant *config, GError **error);
void health_manager_destroy_application(HealthManager *self, const gchar *application, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __HEALTH_MANAGER_H */

