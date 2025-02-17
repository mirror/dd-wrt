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

#include "network.h"

#define NETWORK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), NETWORK_TYPE, NetworkPrivate))

struct _NetworkPrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(Network, network, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _network_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _network_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _network_create_gdbus_proxy(Network *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void network_dispose(GObject *gobject)
{
	Network *self = NETWORK(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(network_parent_class)->dispose(gobject);
}

static void network_finalize (GObject *gobject)
{
	Network *self = NETWORK(gobject);
	G_OBJECT_CLASS(network_parent_class)->finalize(gobject);
}

static void network_class_init(NetworkClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = network_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _network_get_property;
	gobject_class->set_property = _network_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "Network D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void network_init(Network *self)
{
	self->priv = network_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _network_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	Network *self = NETWORK(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, network_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _network_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	Network *self = NETWORK(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_network_create_gdbus_proxy(self, NETWORK_DBUS_SERVICE, self->priv->object_path, &error);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}

	if (error != NULL)
		g_critical("%s", error->message);

	g_assert(error == NULL);
}

/* Constructor */
Network *network_new(const gchar *dbus_object_path)
{
	return g_object_new(NETWORK_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _network_create_gdbus_proxy(Network *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(NETWORK_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, NETWORK_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *network_get_dbus_object_path(Network *self)
{
	g_assert(NETWORK_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* string Connect(string uuid) */
const gchar *network_connect(Network *self, const gchar *uuid, GError **error)
{
	g_assert(NETWORK_IS(self));
	const gchar *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "Connect", g_variant_new ("(s)", uuid), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return NULL;
	proxy_ret = g_variant_get_child_value(proxy_ret, 0);
	ret = g_variant_get_string(proxy_ret, NULL);
	g_variant_unref(proxy_ret);
	return ret;
}

/* void Disconnect() */
void network_disconnect(Network *self, GError **error)
{
	g_assert(NETWORK_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Disconnect", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* Properties access methods */
GVariant *network_get_properties(Network *self, GError **error)
{
	g_assert(NETWORK_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, NETWORK_DBUS_INTERFACE, error);
}

void network_set_property(Network *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(NETWORK_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, NETWORK_DBUS_INTERFACE, name, value, error);
}

gboolean network_get_connected(Network *self, GError **error)
{
	g_assert(NETWORK_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, NETWORK_DBUS_INTERFACE, "Connected", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *network_get_interface(Network *self, GError **error)
{
	g_assert(NETWORK_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, NETWORK_DBUS_INTERFACE, "Interface", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *network_get_uuid(Network *self, GError **error)
{
	g_assert(NETWORK_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, NETWORK_DBUS_INTERFACE, "UUID", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

