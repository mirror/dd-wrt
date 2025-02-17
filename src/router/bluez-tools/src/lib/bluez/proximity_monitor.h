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

#ifndef __PROXIMITY_MONITOR_H
#define __PROXIMITY_MONITOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define PROXIMITY_MONITOR_DBUS_SERVICE "org.bluez"
#define PROXIMITY_MONITOR_DBUS_INTERFACE "org.bluez.ProximityMonitor1"

/*
 * Type macros
 */
#define PROXIMITY_MONITOR_TYPE				(proximity_monitor_get_type())
#define PROXIMITY_MONITOR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), PROXIMITY_MONITOR_TYPE, ProximityMonitor))
#define PROXIMITY_MONITOR_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), PROXIMITY_MONITOR_TYPE))
#define PROXIMITY_MONITOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), PROXIMITY_MONITOR_TYPE, ProximityMonitorClass))
#define PROXIMITY_MONITOR_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), PROXIMITY_MONITOR_TYPE))
#define PROXIMITY_MONITOR_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), PROXIMITY_MONITOR_TYPE, ProximityMonitorClass))

typedef struct _ProximityMonitor ProximityMonitor;
typedef struct _ProximityMonitorClass ProximityMonitorClass;
typedef struct _ProximityMonitorPrivate ProximityMonitorPrivate;

struct _ProximityMonitor {
	GObject parent_instance;

	/*< private >*/
	ProximityMonitorPrivate *priv;
};

struct _ProximityMonitorClass {
	GObjectClass parent_class;
};

/* used by PROXIMITY_MONITOR_TYPE */
GType proximity_monitor_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ProximityMonitor *proximity_monitor_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *proximity_monitor_get_dbus_object_path(ProximityMonitor *self);

GVariant *proximity_monitor_get_properties(ProximityMonitor *self, GError **error);
void proximity_monitor_set_property(ProximityMonitor *self, const gchar *name, const GVariant *value, GError **error);

const gchar *proximity_monitor_get_immediate_alert_level(ProximityMonitor *self, GError **error);
void proximity_monitor_set_immediate_alert_level(ProximityMonitor *self, const gchar *value, GError **error);
const gchar *proximity_monitor_get_link_loss_alert_level(ProximityMonitor *self, GError **error);
void proximity_monitor_set_link_loss_alert_level(ProximityMonitor *self, const gchar *value, GError **error);
const gchar *proximity_monitor_get_signal_level(ProximityMonitor *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __PROXIMITY_MONITOR_H */

