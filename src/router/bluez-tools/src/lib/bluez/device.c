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

#include "device.h"

#define DEVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), DEVICE_TYPE, DevicePrivate))

struct _DevicePrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(Device, device, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _device_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _device_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _device_create_gdbus_proxy(Device *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void device_dispose(GObject *gobject)
{
	Device *self = DEVICE(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(device_parent_class)->dispose(gobject);
}

static void device_finalize (GObject *gobject)
{
	Device *self = DEVICE(gobject);
	G_OBJECT_CLASS(device_parent_class)->finalize(gobject);
}

static void device_class_init(DeviceClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = device_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _device_get_property;
	gobject_class->set_property = _device_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "Device D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void device_init(Device *self)
{
	self->priv = device_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _device_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	Device *self = DEVICE(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, device_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _device_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	Device *self = DEVICE(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_device_create_gdbus_proxy(self, DEVICE_DBUS_SERVICE, self->priv->object_path, &error);
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
Device *device_new(const gchar *dbus_object_path)
{
	return g_object_new(DEVICE_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _device_create_gdbus_proxy(Device *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(DEVICE_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, DEVICE_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *device_get_dbus_object_path(Device *self)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* void CancelPairing() */
void device_cancel_pairing(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "CancelPairing", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Connect() */
void device_connect(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Connect", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void ConnectProfile(string uuid) */
void device_connect_profile(Device *self, const gchar *uuid, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "ConnectProfile", g_variant_new ("(s)", uuid), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Disconnect() */
void device_disconnect(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Disconnect", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void DisconnectProfile(string uuid) */
void device_disconnect_profile(Device *self, const gchar *uuid, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "DisconnectProfile", g_variant_new ("(s)", uuid), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Pair() */
void device_pair(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Pair", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* asynchronous call to void Pair() */
void device_pair_async(Device *self, GAsyncReadyCallback callback, gpointer user_data)
{
	g_assert(DEVICE_IS(self));
    g_dbus_proxy_call (self->priv->proxy,
                       "Pair",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       callback,
                       user_data);
}

/* finish asynchronous call to void Pair() */
void device_pair_finish(Device *self, GAsyncResult *res, GError **error)
{
	g_assert(DEVICE_IS(self));
        g_dbus_proxy_call_finish (self->priv->proxy,
                   res,
                   error);
        
}

/* Properties access methods */
GVariant *device_get_properties(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, DEVICE_DBUS_INTERFACE, error);
}

void device_set_property(Device *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, DEVICE_DBUS_INTERFACE, name, value, error);
}

const gchar *device_get_adapter(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Adapter", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *device_get_address(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Address", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *device_get_alias(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Alias", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void device_set_alias(Device *self, const gchar *value, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, DEVICE_DBUS_INTERFACE, "Alias", g_variant_new_string(value), error);
}

guint16 device_get_appearance(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Appearance", error);
	if(prop == NULL)
		return 0;
	guint16 ret = g_variant_get_uint16(prop);
	g_variant_unref(prop);
	return ret;
}

gboolean device_get_blocked(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Blocked", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

void device_set_blocked(Device *self, const gboolean value, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, DEVICE_DBUS_INTERFACE, "Blocked", g_variant_new_boolean(value), error);
}

guint32 device_get_class(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Class", error);
	if(prop == NULL)
		return 0;
	guint32 ret = g_variant_get_uint32(prop);
	g_variant_unref(prop);
	return ret;
}

gboolean device_get_connected(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Connected", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *device_get_icon(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Icon", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

gboolean device_get_legacy_pairing(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "LegacyPairing", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *device_get_modalias(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Modalias", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *device_get_name(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Name", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

gboolean device_get_paired(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Paired", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

gint16 device_get_rssi(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "RSSI", error);
	if(prop == NULL)
		return 0;
	gint16 ret = g_variant_get_int16(prop);
	g_variant_unref(prop);
	return ret;
}

gboolean device_get_trusted(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "Trusted", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

void device_set_trusted(Device *self, const gboolean value, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, DEVICE_DBUS_INTERFACE, "Trusted", g_variant_new_boolean(value), error);
}

const gchar **device_get_uuids(Device *self, GError **error)
{
	g_assert(DEVICE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, DEVICE_DBUS_INTERFACE, "UUIDs", error);
	if(prop == NULL)
		return NULL;
	const gchar **ret = g_variant_get_strv(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

