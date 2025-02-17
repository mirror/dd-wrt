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

#ifndef __PROXIMITY_REPORTER_H
#define __PROXIMITY_REPORTER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define PROXIMITY_REPORTER_DBUS_SERVICE "org.bluez"
#define PROXIMITY_REPORTER_DBUS_INTERFACE "org.bluez.ProximityReporter1"

/*
 * Type macros
 */
#define PROXIMITY_REPORTER_TYPE				(proximity_reporter_get_type())
#define PROXIMITY_REPORTER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), PROXIMITY_REPORTER_TYPE, ProximityReporter))
#define PROXIMITY_REPORTER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), PROXIMITY_REPORTER_TYPE))
#define PROXIMITY_REPORTER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), PROXIMITY_REPORTER_TYPE, ProximityReporterClass))
#define PROXIMITY_REPORTER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), PROXIMITY_REPORTER_TYPE))
#define PROXIMITY_REPORTER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), PROXIMITY_REPORTER_TYPE, ProximityReporterClass))

typedef struct _ProximityReporter ProximityReporter;
typedef struct _ProximityReporterClass ProximityReporterClass;
typedef struct _ProximityReporterPrivate ProximityReporterPrivate;

struct _ProximityReporter {
	GObject parent_instance;

	/*< private >*/
	ProximityReporterPrivate *priv;
};

struct _ProximityReporterClass {
	GObjectClass parent_class;
};

/* used by PROXIMITY_REPORTER_TYPE */
GType proximity_reporter_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ProximityReporter *proximity_reporter_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *proximity_reporter_get_dbus_object_path(ProximityReporter *self);

GVariant *proximity_reporter_get_properties(ProximityReporter *self, GError **error);
void proximity_reporter_set_property(ProximityReporter *self, const gchar *name, const GVariant *value, GError **error);

const gchar *proximity_reporter_get_immediate_alert_level(ProximityReporter *self, GError **error);
const gchar *proximity_reporter_get_link_loss_alert_level(ProximityReporter *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __PROXIMITY_REPORTER_H */

