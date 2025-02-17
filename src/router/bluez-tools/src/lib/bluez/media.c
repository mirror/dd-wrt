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

#include "media.h"

#define MEDIA_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), MEDIA_TYPE, MediaPrivate))

struct _MediaPrivate {
	GDBusProxy *proxy;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(Media, media, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _media_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _media_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _media_create_gdbus_proxy(Media *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void media_dispose(GObject *gobject)
{
	Media *self = MEDIA(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(media_parent_class)->dispose(gobject);
}

static void media_finalize (GObject *gobject)
{
	Media *self = MEDIA(gobject);
	G_OBJECT_CLASS(media_parent_class)->finalize(gobject);
}

static void media_class_init(MediaClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = media_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _media_get_property;
	gobject_class->set_property = _media_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "Media D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void media_init(Media *self)
{
	self->priv = media_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _media_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	Media *self = MEDIA(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, media_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _media_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	Media *self = MEDIA(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_media_create_gdbus_proxy(self, MEDIA_DBUS_SERVICE, self->priv->object_path, &error);
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
Media *media_new(const gchar *dbus_object_path)
{
	return g_object_new(MEDIA_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _media_create_gdbus_proxy(Media *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(MEDIA_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, MEDIA_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* Get DBus object path */
const gchar *media_get_dbus_object_path(Media *self)
{
	g_assert(MEDIA_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* void RegisterEndpoint(object endpoint, dict properties) */
void media_register_endpoint(Media *self, const gchar *endpoint, const GVariant *properties, GError **error)
{
	g_assert(MEDIA_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RegisterEndpoint", g_variant_new ("(o@a{sv})", endpoint, properties), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void RegisterPlayer(object player, dict properties) */
void media_register_player(Media *self, const gchar *player, const GVariant *properties, GError **error)
{
	g_assert(MEDIA_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RegisterPlayer", g_variant_new ("(o@a{sv})", player, properties), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void UnregisterEndpoint(object endpoint) */
void media_unregister_endpoint(Media *self, const gchar *endpoint, GError **error)
{
	g_assert(MEDIA_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "UnregisterEndpoint", g_variant_new ("(o)", endpoint), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void UnregisterPlayer(object player) */
void media_unregister_player(Media *self, const gchar *player, GError **error)
{
	g_assert(MEDIA_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "UnregisterPlayer", g_variant_new ("(o)", player), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

