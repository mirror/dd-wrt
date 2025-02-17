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

#ifndef __AGENT_MANAGER_H
#define __AGENT_MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define AGENT_MANAGER_DBUS_SERVICE "org.bluez"
#define AGENT_MANAGER_DBUS_INTERFACE "org.bluez.AgentManager1"
#define AGENT_MANAGER_DBUS_PATH "/org/bluez"

/*
 * Type macros
 */
#define AGENT_MANAGER_TYPE				(agent_manager_get_type())
#define AGENT_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), AGENT_MANAGER_TYPE, AgentManager))
#define AGENT_MANAGER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), AGENT_MANAGER_TYPE))
#define AGENT_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), AGENT_MANAGER_TYPE, AgentManagerClass))
#define AGENT_MANAGER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), AGENT_MANAGER_TYPE))
#define AGENT_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), AGENT_MANAGER_TYPE, AgentManagerClass))

typedef struct _AgentManager AgentManager;
typedef struct _AgentManagerClass AgentManagerClass;
typedef struct _AgentManagerPrivate AgentManagerPrivate;

struct _AgentManager {
	GObject parent_instance;

	/*< private >*/
	AgentManagerPrivate *priv;
};

struct _AgentManagerClass {
	GObjectClass parent_class;
};

/* used by AGENT_MANAGER_TYPE */
GType agent_manager_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
AgentManager *agent_manager_new();

/*
 * Method definitions
 */
void agent_manager_register_agent(AgentManager *self, const gchar *agent, const gchar *capability, GError **error);
void agent_manager_request_default_agent(AgentManager *self, const gchar *agent, GError **error);
void agent_manager_unregister_agent(AgentManager *self, const gchar *agent, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __AGENT_MANAGER_H */

