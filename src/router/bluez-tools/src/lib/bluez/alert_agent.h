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

#ifndef __ALERT_AGENT_H
#define __ALERT_AGENT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define ALERT_AGENT_DBUS_SERVICE "org.bluez"
#define ALERT_AGENT_DBUS_INTERFACE "org.bluez.AlertAgent1"

/*
 * Type macros
 */
#define ALERT_AGENT_TYPE				(alert_agent_get_type())
#define ALERT_AGENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), ALERT_AGENT_TYPE, AlertAgent))
#define ALERT_AGENT_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), ALERT_AGENT_TYPE))
#define ALERT_AGENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), ALERT_AGENT_TYPE, AlertAgentClass))
#define ALERT_AGENT_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), ALERT_AGENT_TYPE))
#define ALERT_AGENT_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), ALERT_AGENT_TYPE, AlertAgentClass))

typedef struct _AlertAgent AlertAgent;
typedef struct _AlertAgentClass AlertAgentClass;
typedef struct _AlertAgentPrivate AlertAgentPrivate;

struct _AlertAgent {
	GObject parent_instance;

	/*< private >*/
	AlertAgentPrivate *priv;
};

struct _AlertAgentClass {
	GObjectClass parent_class;
};

/* used by ALERT_AGENT_TYPE */
GType alert_agent_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
AlertAgent *alert_agent_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *alert_agent_get_dbus_object_path(AlertAgent *self);

void alert_agent_mute_once(AlertAgent *self, GError **error);
void alert_agent_release(AlertAgent *self, GError **error);
void alert_agent_set_ringer(AlertAgent *self, const gchar *mode, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __ALERT_AGENT_H */

