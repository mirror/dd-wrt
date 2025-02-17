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

#include "obex_client.h"

#define OBEX_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OBEX_CLIENT_TYPE, ObexClientPrivate))

struct _ObexClientPrivate {
	GDBusProxy *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE(ObexClient, obex_client, G_TYPE_OBJECT);

enum {
	PROP_0,
};

static void _obex_client_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _obex_client_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _obex_client_create_gdbus_proxy(ObexClient *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void obex_client_dispose(GObject *gobject)
{
	ObexClient *self = OBEX_CLIENT(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(obex_client_parent_class)->dispose(gobject);
}

static void obex_client_finalize (GObject *gobject)
{
	ObexClient *self = OBEX_CLIENT(gobject);
	G_OBJECT_CLASS(obex_client_parent_class)->finalize(gobject);
}

static void obex_client_class_init(ObexClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = obex_client_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _obex_client_get_property;
	gobject_class->set_property = _obex_client_set_property;
	if (pspec)
		g_param_spec_unref(pspec);
}

static void obex_client_init(ObexClient *self)
{
	self->priv = obex_client_get_instance_private (self);
	self->priv->proxy = NULL;
	g_assert(session_conn != NULL);
	GError *error = NULL;
	_obex_client_create_gdbus_proxy(self, OBEX_CLIENT_DBUS_SERVICE, OBEX_CLIENT_DBUS_PATH, &error);
	g_assert(error == NULL);
}

static void _obex_client_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ObexClient *self = OBEX_CLIENT(object);

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _obex_client_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ObexClient *self = OBEX_CLIENT(object);
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
ObexClient *obex_client_new()
{
	return g_object_new(OBEX_CLIENT_TYPE, NULL);
}

/* Private DBus proxy creation */
static void _obex_client_create_gdbus_proxy(ObexClient *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(OBEX_CLIENT_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(session_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, OBEX_CLIENT_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* object CreateSession(string destination, dict args) */
const gchar *obex_client_create_session(ObexClient *self, const gchar *destination, const GVariant *args, GError **error)
{
	g_assert(OBEX_CLIENT_IS(self));
	const gchar *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "CreateSession", g_variant_new ("(s@a{sv})", destination, args), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret == NULL)
		return NULL;
	proxy_ret = g_variant_get_child_value(proxy_ret, 0);
	ret = g_variant_get_string(proxy_ret, NULL);
	g_variant_unref(proxy_ret);
	return ret;
}

/* void RemoveSession(object session) */
void obex_client_remove_session(ObexClient *self, const gchar *session, GError **error)
{
	g_assert(OBEX_CLIENT_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RemoveSession", g_variant_new ("(o)", session), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

