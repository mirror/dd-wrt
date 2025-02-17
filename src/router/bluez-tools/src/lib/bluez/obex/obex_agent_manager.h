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

#ifndef __OBEX_AGENT_MANAGER_H
#define __OBEX_AGENT_MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_AGENT_MANAGER_DBUS_SERVICE "org.bluez.obex"
#define OBEX_AGENT_MANAGER_DBUS_INTERFACE "org.bluez.obex.AgentManager1"
#define OBEX_AGENT_MANAGER_DBUS_PATH "/org/bluez/obex"

/*
 * Type macros
 */
#define OBEX_AGENT_MANAGER_TYPE				(obex_agent_manager_get_type())
#define OBEX_AGENT_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_AGENT_MANAGER_TYPE, ObexAgentManager))
#define OBEX_AGENT_MANAGER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_AGENT_MANAGER_TYPE))
#define OBEX_AGENT_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_AGENT_MANAGER_TYPE, ObexAgentManagerClass))
#define OBEX_AGENT_MANAGER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_AGENT_MANAGER_TYPE))
#define OBEX_AGENT_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_AGENT_MANAGER_TYPE, ObexAgentManagerClass))

typedef struct _ObexAgentManager ObexAgentManager;
typedef struct _ObexAgentManagerClass ObexAgentManagerClass;
typedef struct _ObexAgentManagerPrivate ObexAgentManagerPrivate;

struct _ObexAgentManager {
	GObject parent_instance;

	/*< private >*/
	ObexAgentManagerPrivate *priv;
};

struct _ObexAgentManagerClass {
	GObjectClass parent_class;
};

/* used by OBEX_AGENT_MANAGER_TYPE */
GType obex_agent_manager_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexAgentManager *obex_agent_manager_new();

/*
 * Method definitions
 */
void obex_agent_manager_register_agent(ObexAgentManager *self, const gchar *agent, GError **error);
void obex_agent_manager_unregister_agent(ObexAgentManager *self, const gchar *agent, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_AGENT_MANAGER_H */

