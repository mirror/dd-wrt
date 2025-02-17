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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#include "../dbus-common.h"
#include "../properties.h"

#include "agent_manager.h"

#define AGENT_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), AGENT_MANAGER_TYPE, AgentManagerPrivate))

struct _AgentManagerPrivate {
	GDBusProxy *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE(AgentManager, agent_manager, G_TYPE_OBJECT);

enum {
	PROP_0,
};

static void _agent_manager_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _agent_manager_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _agent_manager_create_gdbus_proxy(AgentManager *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void agent_manager_dispose(GObject *gobject)
{
	AgentManager *self = AGENT_MANAGER(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(agent_manager_parent_class)->dispose(gobject);
}

static void agent_manager_finalize (GObject *gobject)
{
	AgentManager *self = AGENT_MANAGER(gobject);
	G_OBJECT_CLASS(agent_manager_parent_class)->finalize(gobject);
}

static void agent_manager_class_init(AgentManagerClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = agent_manager_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _agent_manager_get_property;
	gobject_class->set_property = _agent_manager_set_property;
	if (pspec)
		g_param_spec_unref(pspec);
}

static void agent_manager_init(AgentManager *self)
{
	self->priv = agent_manager_get_instance_private (self);
	self->priv->proxy = NULL;
	g_assert(system_conn != NULL);
	GError *error = NULL;
	_agent_manager_create_gdbus_proxy(self, AGENT_MANAGER_DBUS_SERVICE, AGENT_MANAGER_DBUS_PATH, &error);
	g_assert(error == NULL);
}

static void _agent_manager_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	AgentManager *self = AGENT_MANAGER(object);

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _agent_manager_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	AgentManager *self = AGENT_MANAGER(object);
	GError *error = NULL;

	switch (property_id) {

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}

	if (error != NULL)
		g_critical("%s", error->message);

	g_assert(error == NULL);
}

/* Constructor */
AgentManager *agent_manager_new()
{
	return g_object_new(AGENT_MANAGER_TYPE, NULL);
}

/* Private DBus proxy creation */
static void _agent_manager_create_gdbus_proxy(AgentManager *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(AGENT_MANAGER_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, AGENT_MANAGER_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* void RegisterAgent(object agent, string capability) */
void agent_manager_register_agent(AgentManager *self, const gchar *agent, const gchar *capability, GError **error)
{
	g_assert(AGENT_MANAGER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RegisterAgent", g_variant_new ("(os)", agent, capability), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void RequestDefaultAgent(object agent) */
void agent_manager_request_default_agent(AgentManager *self, const gchar *agent, GError **error)
{
	g_assert(AGENT_MANAGER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RequestDefaultAgent", g_variant_new ("(o)", agent), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void UnregisterAgent(object agent) */
void agent_manager_unregister_agent(AgentManager *self, const gchar *agent, GError **error)
{
	g_assert(AGENT_MANAGER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "UnregisterAgent", g_variant_new ("(o)", agent), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

