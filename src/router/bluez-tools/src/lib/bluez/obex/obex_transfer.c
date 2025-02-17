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

#include "../../dbus-common.h"
#include "../../properties.h"

#include "obex_transfer.h"

#define OBEX_TRANSFER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OBEX_TRANSFER_TYPE, ObexTransferPrivate))

struct _ObexTransferPrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(ObexTransfer, obex_transfer, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _obex_transfer_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _obex_transfer_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _obex_transfer_create_gdbus_proxy(ObexTransfer *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void obex_transfer_dispose(GObject *gobject)
{
	ObexTransfer *self = OBEX_TRANSFER(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(obex_transfer_parent_class)->dispose(gobject);
}

static void obex_transfer_finalize (GObject *gobject)
{
	ObexTransfer *self = OBEX_TRANSFER(gobject);
	G_OBJECT_CLASS(obex_transfer_parent_class)->finalize(gobject);
}

static void obex_transfer_class_init(ObexTransferClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = obex_transfer_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _obex_transfer_get_property;
	gobject_class->set_property = _obex_transfer_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "ObexTransfer D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void obex_transfer_init(ObexTransfer *self)
{
	self->priv = obex_transfer_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(session_conn != NULL);
}

static void _obex_transfer_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ObexTransfer *self = OBEX_TRANSFER(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, obex_transfer_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _obex_transfer_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ObexTransfer *self = OBEX_TRANSFER(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_obex_transfer_create_gdbus_proxy(self, OBEX_TRANSFER_DBUS_SERVICE, self->priv->object_path, &error);
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
ObexTransfer *obex_transfer_new(const gchar *dbus_object_path)
{
	return g_object_new(OBEX_TRANSFER_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _obex_transfer_create_gdbus_proxy(ObexTransfer *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(session_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, OBEX_TRANSFER_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "session", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *obex_transfer_get_dbus_object_path(ObexTransfer *self)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* void Cancel() */
void obex_transfer_cancel(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Cancel", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Resume() */
void obex_transfer_resume(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Resume", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Suspend() */
void obex_transfer_suspend(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Suspend", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* Properties access methods */
GVariant *obex_transfer_get_properties(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, error);
}

void obex_transfer_set_property(ObexTransfer *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, name, value, error);
}

const gchar *obex_transfer_get_filename(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Filename", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_transfer_get_name(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Name", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_transfer_get_session(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Session", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

guint64 obex_transfer_get_size(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Size", error);
	if(prop == NULL)
		return 0;
	guint64 ret = g_variant_get_uint64(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_transfer_get_status(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Status", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

guint64 obex_transfer_get_time(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Time", error);
	if(prop == NULL)
		return 0;
	guint64 ret = g_variant_get_uint64(prop);
	g_variant_unref(prop);
	return ret;
}

guint64 obex_transfer_get_transferred(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Transferred", error);
	if(prop == NULL)
		return 0;
	guint64 ret = g_variant_get_uint64(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_transfer_get_transfer_type(ObexTransfer *self, GError **error)
{
	g_assert(OBEX_TRANSFER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_TRANSFER_DBUS_INTERFACE, "Type", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

