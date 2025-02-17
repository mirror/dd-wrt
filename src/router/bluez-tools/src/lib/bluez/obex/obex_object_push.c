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

#include "obex_object_push.h"

#define OBEX_OBJECT_PUSH_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OBEX_OBJECT_PUSH_TYPE, ObexObjectPushPrivate))

struct _ObexObjectPushPrivate {
	GDBusProxy *proxy;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(ObexObjectPush, obex_object_push, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _obex_object_push_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _obex_object_push_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _obex_object_push_create_gdbus_proxy(ObexObjectPush *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void obex_object_push_dispose(GObject *gobject)
{
	ObexObjectPush *self = OBEX_OBJECT_PUSH(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(obex_object_push_parent_class)->dispose(gobject);
}

static void obex_object_push_finalize (GObject *gobject)
{
	ObexObjectPush *self = OBEX_OBJECT_PUSH(gobject);
	G_OBJECT_CLASS(obex_object_push_parent_class)->finalize(gobject);
}

static void obex_object_push_class_init(ObexObjectPushClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = obex_object_push_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _obex_object_push_get_property;
	gobject_class->set_property = _obex_object_push_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "ObexObjectPush D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void obex_object_push_init(ObexObjectPush *self)
{
	self->priv = obex_object_push_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->object_path = NULL;
	g_assert(session_conn != NULL);
}

static void _obex_object_push_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ObexObjectPush *self = OBEX_OBJECT_PUSH(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, obex_object_push_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _obex_object_push_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ObexObjectPush *self = OBEX_OBJECT_PUSH(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_obex_object_push_create_gdbus_proxy(self, OBEX_OBJECT_PUSH_DBUS_SERVICE, self->priv->object_path, &error);
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
ObexObjectPush *obex_object_push_new(const gchar *dbus_object_path)
{
	return g_object_new(OBEX_OBJECT_PUSH_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _obex_object_push_create_gdbus_proxy(ObexObjectPush *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(OBEX_OBJECT_PUSH_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(session_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, OBEX_OBJECT_PUSH_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* Get DBus object path */
const gchar *obex_object_push_get_dbus_object_path(ObexObjectPush *self)
{
	g_assert(OBEX_OBJECT_PUSH_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

GVariant *obex_object_push_exchange_business_cards(ObexObjectPush *self, const gchar *clientfile, const gchar *targetfile, GError **error)
{
	g_assert(OBEX_OBJECT_PUSH_IS(self));
	GVariant *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "ExchangeBusinessCards", g_variant_new ("(ss)", clientfile, targetfile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return NULL;
	ret = g_variant_ref_sink(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}

GVariant *obex_object_push_pull_business_card(ObexObjectPush *self, const gchar *targetfile, GError **error)
{
	g_assert(OBEX_OBJECT_PUSH_IS(self));
	GVariant *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "PullBusinessCard", g_variant_new ("(s)", targetfile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return NULL;
	ret = g_variant_ref_sink(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}

GVariant *obex_object_push_send_file(ObexObjectPush *self, const gchar *sourcefile, GError **error)
{
	g_assert(OBEX_OBJECT_PUSH_IS(self));
	GVariant *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "SendFile", g_variant_new ("(s)", sourcefile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return NULL;
	ret = g_variant_ref_sink(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}