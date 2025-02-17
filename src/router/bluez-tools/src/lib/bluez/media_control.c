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

#include "media_control.h"

#define MEDIA_CONTROL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), MEDIA_CONTROL_TYPE, MediaControlPrivate))

struct _MediaControlPrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(MediaControl, media_control, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _media_control_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _media_control_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _media_control_create_gdbus_proxy(MediaControl *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void media_control_dispose(GObject *gobject)
{
	MediaControl *self = MEDIA_CONTROL(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(media_control_parent_class)->dispose(gobject);
}

static void media_control_finalize (GObject *gobject)
{
	MediaControl *self = MEDIA_CONTROL(gobject);
	G_OBJECT_CLASS(media_control_parent_class)->finalize(gobject);
}

static void media_control_class_init(MediaControlClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = media_control_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _media_control_get_property;
	gobject_class->set_property = _media_control_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "MediaControl D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void media_control_init(MediaControl *self)
{
	self->priv = media_control_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _media_control_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	MediaControl *self = MEDIA_CONTROL(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, media_control_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _media_control_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	MediaControl *self = MEDIA_CONTROL(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_media_control_create_gdbus_proxy(self, MEDIA_CONTROL_DBUS_SERVICE, self->priv->object_path, &error);
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
MediaControl *media_control_new(const gchar *dbus_object_path)
{
	return g_object_new(MEDIA_CONTROL_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _media_control_create_gdbus_proxy(MediaControl *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, MEDIA_CONTROL_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *media_control_get_dbus_object_path(MediaControl *self)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* void FastForward() */
void media_control_fast_forward(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "FastForward", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Next() */
void media_control_next(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Next", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Pause() */
void media_control_pause(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Pause", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Play() */
void media_control_play(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Play", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Previous() */
void media_control_previous(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Previous", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Rewind() */
void media_control_rewind(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Rewind", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Stop() */
void media_control_stop(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Stop", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void VolumeDown() */
void media_control_volume_down(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "VolumeDown", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void VolumeUp() */
void media_control_volume_up(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "VolumeUp", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* Properties access methods */
GVariant *media_control_get_properties(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, MEDIA_CONTROL_DBUS_INTERFACE, error);
}

void media_control_set_property(MediaControl *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, MEDIA_CONTROL_DBUS_INTERFACE, name, value, error);
}

gboolean media_control_get_connected(MediaControl *self, GError **error)
{
	g_assert(MEDIA_CONTROL_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_CONTROL_DBUS_INTERFACE, "Connected", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

