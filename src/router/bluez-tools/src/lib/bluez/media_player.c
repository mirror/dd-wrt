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

#include "media_player.h"

#define MEDIA_PLAYER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), MEDIA_PLAYER_TYPE, MediaPlayerPrivate))

struct _MediaPlayerPrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(MediaPlayer, media_player, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _media_player_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _media_player_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _media_player_create_gdbus_proxy(MediaPlayer *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void media_player_dispose(GObject *gobject)
{
	MediaPlayer *self = MEDIA_PLAYER(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(media_player_parent_class)->dispose(gobject);
}

static void media_player_finalize (GObject *gobject)
{
	MediaPlayer *self = MEDIA_PLAYER(gobject);
	G_OBJECT_CLASS(media_player_parent_class)->finalize(gobject);
}

static void media_player_class_init(MediaPlayerClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = media_player_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _media_player_get_property;
	gobject_class->set_property = _media_player_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "MediaPlayer D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void media_player_init(MediaPlayer *self)
{
	self->priv = media_player_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _media_player_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	MediaPlayer *self = MEDIA_PLAYER(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, media_player_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _media_player_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	MediaPlayer *self = MEDIA_PLAYER(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_media_player_create_gdbus_proxy(self, MEDIA_PLAYER_DBUS_SERVICE, self->priv->object_path, &error);
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
MediaPlayer *media_player_new(const gchar *dbus_object_path)
{
	return g_object_new(MEDIA_PLAYER_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _media_player_create_gdbus_proxy(MediaPlayer *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, MEDIA_PLAYER_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *media_player_get_dbus_object_path(MediaPlayer *self)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* void FastForward() */
void media_player_fast_forward(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "FastForward", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Next() */
void media_player_next(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Next", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Pause() */
void media_player_pause(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Pause", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Play() */
void media_player_play(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Play", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Previous() */
void media_player_previous(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Previous", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Rewind() */
void media_player_rewind(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Rewind", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Stop() */
void media_player_stop(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Stop", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* Properties access methods */
GVariant *media_player_get_properties(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	return properties_get_all(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, error);
}

void media_player_set_property(MediaPlayer *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, name, value, error);
}

gboolean media_player_get_browsable(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Browsable", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *media_player_get_device(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Device", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *media_player_get_equalizer(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Equalizer", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void media_player_set_equalizer(MediaPlayer *self, const gchar *value, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Equalizer", g_variant_new_string(value), error);
}

const gchar *media_player_get_name(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Name", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

guint32 media_player_get_position(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Position", error);
	if(prop == NULL)
		return 0;
	guint32 ret = g_variant_get_uint32(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *media_player_get_repeat(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Repeat", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void media_player_set_repeat(MediaPlayer *self, const gchar *value, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Repeat", g_variant_new_string(value), error);
}

const gchar *media_player_get_scan(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Scan", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void media_player_set_scan(MediaPlayer *self, const gchar *value, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Scan", g_variant_new_string(value), error);
}

gboolean media_player_get_searchable(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Searchable", error);
	if(prop == NULL)
		return FALSE;
	gboolean ret = g_variant_get_boolean(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *media_player_get_shuffle(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Shuffle", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void media_player_set_shuffle(MediaPlayer *self, const gchar *value, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	properties_set(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Shuffle", g_variant_new_string(value), error);
}

const gchar *media_player_get_status(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Status", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

const gchar *media_player_get_subtype(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Subtype", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

GVariant *media_player_get_track(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Track", error);
	if(prop == NULL)
		return NULL;
	GVariant *ret = g_variant_ref_sink(prop);
	g_variant_unref(prop);
	return ret;
}

const gchar *media_player_get_player_type(MediaPlayer *self, GError **error)
{
	g_assert(MEDIA_PLAYER_IS(self));
	g_assert(self->priv->properties != NULL);
	GVariant *prop = properties_get(self->priv->properties, MEDIA_PLAYER_DBUS_INTERFACE, "Type", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

