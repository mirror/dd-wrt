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

#ifndef __SIM_ACCESS_H
#define __SIM_ACCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define SIM_ACCESS_DBUS_SERVICE "org.bluez"
#define SIM_ACCESS_DBUS_INTERFACE "org.bluez.SimAccess1"

/*
 * Type macros
 */
#define SIM_ACCESS_TYPE				(sim_access_get_type())
#define SIM_ACCESS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), SIM_ACCESS_TYPE, SimAccess))
#define SIM_ACCESS_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), SIM_ACCESS_TYPE))
#define SIM_ACCESS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), SIM_ACCESS_TYPE, SimAccessClass))
#define SIM_ACCESS_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), SIM_ACCESS_TYPE))
#define SIM_ACCESS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), SIM_ACCESS_TYPE, SimAccessClass))

typedef struct _SimAccess SimAccess;
typedef struct _SimAccessClass SimAccessClass;
typedef struct _SimAccessPrivate SimAccessPrivate;

struct _SimAccess {
	GObject parent_instance;

	/*< private >*/
	SimAccessPrivate *priv;
};

struct _SimAccessClass {
	GObjectClass parent_class;
};

/* used by SIM_ACCESS_TYPE */
GType sim_access_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
SimAccess *sim_access_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *sim_access_get_dbus_object_path(SimAccess *self);

void sim_access_disconnect(SimAccess *self, GError **error);

GVariant *sim_access_get_properties(SimAccess *self, GError **error);
void sim_access_set_property(SimAccess *self, const gchar *name, const GVariant *value, GError **error);

gboolean sim_access_get_connected(SimAccess *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __SIM_ACCESS_H */

