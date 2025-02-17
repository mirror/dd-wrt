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

#include "health_device.h"

#define HEALTH_DEVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), HEALTH_DEVICE_TYPE, HealthDevicePrivate))

struct _HealthDevicePrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(HealthDevice, health_device, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _health_device_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _health_device_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _health_device_create_gdbus_proxy(HealthDevice *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void health_device_dispose(GObject *gobject)
{
	HealthDevice *self = HEALTH_DEVICE(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(health_device_parent_class)->dispose(gobject);
}

static void health_device_finalize (GObject *gobject)
{
	HealthDevice *self = HEALTH_DEVICE(gobject);
	G_OBJECT_CLASS(health_device_parent_class)->finalize(gobject);
}

static void health_device_class_init(HealthDeviceClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = health_device_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _health_device_get_property;
	gobject_class->set_property = _health_device_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "HealthDevice D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void health_device_init(HealthDevice *self)
{
	self->priv = health_device_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _health_device_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	HealthDevice *self = HEALTH_DEVICE(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, health_device_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _health_device_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	HealthDevice *self = HEALTH_DEVICE(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_health_device_create_gdbus_proxy(self, HEALTH_DEVICE_DBUS_SERVICE, self->priv->object_path, &error);
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
HealthDevice *health_device_new(const gchar *dbus_object_path)
{
	return g_object_new(HEALTH_DEVICE_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _health_device_create_gdbus_proxy(HealthDevice *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, HEALTH_DEVICE_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *health_device_get_dbus_object_path(HealthDevice *self)
{
	g_assert(HEALTH_DEVICE_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* object CreateChannel(object application, string configuration) */
const gchar *health_device_create_channel(HealthDevice *self, const gchar *application, const gchar *configuration, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	const gchar *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "CreateChannel", g_variant_new ("(os)", application, configuration), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return NULL;
	proxy_ret = g_variant_get_child_value(proxy_ret, 0);
	ret = g_variant_get_string(proxy_ret, NULL);
	g_variant_unref(proxy_ret);
	return ret;
}

/* void DestroyChannel(object channel) */
void health_device_destroy_channel(HealthDevice *self, const gchar *channel, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "DestroyChannel", g_variant_new ("(o)", channel), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* boolean Echo() */
gboolean health_device_echo(HealthDevice *self, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	gboolean ret = FALSE;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "Echo", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return FALSE;
	proxy_ret = g_variant_get_child_value(proxy_ret, 0);
	ret = g_variant_get_boolean(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}

/* Properties access methods */
GVariant *health_device_get_properties(HealthDevice *self, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, HEALTH_DEVICE_DBUS_INTERFACE, error);
}

void health_device_set_property(HealthDevice *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, HEALTH_DEVICE_DBUS_INTERFACE, name, value, error);
}

const gchar *health_device_get_main_channel(HealthDevice *self, GError **error)
{
	g_assert(HEALTH_DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, HEALTH_DEVICE_DBUS_INTERFACE, "MainChannel", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

