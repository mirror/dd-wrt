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

#include "obex_message.h"

#define OBEX_MESSAGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OBEX_MESSAGE_TYPE, ObexMessagePrivate))

struct _ObexMessagePrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(ObexMessage, obex_message, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _obex_message_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _obex_message_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _obex_message_create_gdbus_proxy(ObexMessage *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void obex_message_dispose(GObject *gobject)
{
	ObexMessage *self = OBEX_MESSAGE(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(obex_message_parent_class)->dispose(gobject);
}

static void obex_message_finalize (GObject *gobject)
{
	ObexMessage *self = OBEX_MESSAGE(gobject);
	G_OBJECT_CLASS(obex_message_parent_class)->finalize(gobject);
}

static void obex_message_class_init(ObexMessageClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = obex_message_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _obex_message_get_property;
	gobject_class->set_property = _obex_message_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "ObexMessage D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void obex_message_init(ObexMessage *self)
{
	self->priv = obex_message_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(session_conn != NULL);
}

static void _obex_message_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ObexMessage *self = OBEX_MESSAGE(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, obex_message_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _obex_message_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ObexMessage *self = OBEX_MESSAGE(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_obex_message_create_gdbus_proxy(self, OBEX_MESSAGE_DBUS_SERVICE, self->priv->object_path, &error);
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
ObexMessage *obex_message_new(const gchar *dbus_object_path)
{
	return g_object_new(OBEX_MESSAGE_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _obex_message_create_gdbus_proxy(ObexMessage *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(session_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, OBEX_MESSAGE_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "session", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *obex_message_get_dbus_object_path(ObexMessage *self)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}



/* Properties access methods */
GVariant *obex_message_get_properties(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, error);
}

void obex_message_set_property(ObexMessage *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, name, value, error);
}

void obex_message_set_deleted(ObexMessage *self, const gboolean value, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Deleted", g_variant_new_boolean(value), error);
}

const gchar *obex_message_get_folder(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Folder", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

gboolean obex_message_get_priority(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Priority", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

gboolean obex_message_get_protected(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Protected", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

gboolean obex_message_get_read(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Read", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

void obex_message_set_read(ObexMessage *self, const gboolean value, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Read", g_variant_new_boolean(value), error);
}

const gchar *obex_message_get_recipient(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Recipient", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_recipient_address(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "RecipientAddress", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_reply_to(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "ReplyTo", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_sender(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Sender", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_sender_address(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "SenderAddress", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

gboolean obex_message_get_sent(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Sent", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

guint64 obex_message_get_size(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Size", error);
	if(prop == NULL)
		return 0;
	guint64 ret = g_variant_get_uint64(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_status(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Status", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_subject(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Subject", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_timestamp(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Timestamp", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *obex_message_get_message_type(ObexMessage *self, GError **error)
{
	g_assert(OBEX_MESSAGE_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, OBEX_MESSAGE_DBUS_INTERFACE, "Type", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

