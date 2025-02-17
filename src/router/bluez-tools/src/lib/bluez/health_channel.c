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

#include "health_channel.h"

#define HEALTH_CHANNEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), HEALTH_CHANNEL_TYPE, HealthChannelPrivate))

struct _HealthChannelPrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(HealthChannel, health_channel, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _health_channel_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _health_channel_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _health_channel_create_gdbus_proxy(HealthChannel *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void health_channel_dispose(GObject *gobject)
{
	HealthChannel *self = HEALTH_CHANNEL(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(health_channel_parent_class)->dispose(gobject);
}

static void health_channel_finalize (GObject *gobject)
{
	HealthChannel *self = HEALTH_CHANNEL(gobject);
	G_OBJECT_CLASS(health_channel_parent_class)->finalize(gobject);
}

static void health_channel_class_init(HealthChannelClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = health_channel_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _health_channel_get_property;
	gobject_class->set_property = _health_channel_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "HealthChannel D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void health_channel_init(HealthChannel *self)
{
	self->priv = health_channel_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _health_channel_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	HealthChannel *self = HEALTH_CHANNEL(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, health_channel_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _health_channel_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	HealthChannel *self = HEALTH_CHANNEL(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_health_channel_create_gdbus_proxy(self, HEALTH_CHANNEL_DBUS_SERVICE, self->priv->object_path, &error);
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
HealthChannel *health_channel_new(const gchar *dbus_object_path)
{
	return g_object_new(HEALTH_CHANNEL_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _health_channel_create_gdbus_proxy(HealthChannel *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, HEALTH_CHANNEL_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *health_channel_get_dbus_object_path(HealthChannel *self)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* fd Acquire() */
guint32 health_channel_acquire(HealthChannel *self, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	guint32 ret = 0;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "Acquire", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return 0;
	proxy_ret = g_variant_get_child_value(proxy_ret, 0);
	ret = g_variant_get_uint32(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}

/* void Release() */
void health_channel_release(HealthChannel *self, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Release", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* Properties access methods */
GVariant *health_channel_get_properties(HealthChannel *self, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, HEALTH_CHANNEL_DBUS_INTERFACE, error);
}

void health_channel_set_property(HealthChannel *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, HEALTH_CHANNEL_DBUS_INTERFACE, name, value, error);
}

const gchar *health_channel_get_application(HealthChannel *self, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, HEALTH_CHANNEL_DBUS_INTERFACE, "Application", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *health_channel_get_device(HealthChannel *self, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, HEALTH_CHANNEL_DBUS_INTERFACE, "Device", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *health_channel_get_channel_type(HealthChannel *self, GError **error)
{
	g_assert(HEALTH_CHANNEL_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, HEALTH_CHANNEL_DBUS_INTERFACE, "Type", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

