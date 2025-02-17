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

#ifndef __ALERT_H
#define __ALERT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define ALERT_DBUS_SERVICE "org.bluez"
#define ALERT_DBUS_INTERFACE "org.bluez.Alert1"
#define ALERT_DBUS_PATH "/org/bluez"

/*
 * Type macros
 */
#define ALERT_TYPE				(alert_get_type())
#define ALERT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), ALERT_TYPE, Alert))
#define ALERT_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), ALERT_TYPE))
#define ALERT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), ALERT_TYPE, AlertClass))
#define ALERT_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), ALERT_TYPE))
#define ALERT_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), ALERT_TYPE, AlertClass))

typedef struct _Alert Alert;
typedef struct _AlertClass AlertClass;
typedef struct _AlertPrivate AlertPrivate;

struct _Alert {
	GObject parent_instance;

	/*< private >*/
	AlertPrivate *priv;
};

struct _AlertClass {
	GObjectClass parent_class;
};

/* used by ALERT_TYPE */
GType alert_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
Alert *alert_new();

/*
 * Method definitions
 */
void alert_new_alert(Alert *self, const gchar *category, const guint16 count, const gchar *description, GError **error);
void alert_register_alert(Alert *self, const gchar *category, const gchar *agent, GError **error);
void alert_unread_alert(Alert *self, const gchar *category, const guint16 count, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __ALERT_H */

