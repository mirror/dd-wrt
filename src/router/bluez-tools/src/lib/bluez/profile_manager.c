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

#include "profile_manager.h"

#define PROFILE_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PROFILE_MANAGER_TYPE, ProfileManagerPrivate))

struct _ProfileManagerPrivate {
	GDBusProxy *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE(ProfileManager, profile_manager, G_TYPE_OBJECT);

enum {
	PROP_0,
};

static void _profile_manager_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _profile_manager_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _profile_manager_create_gdbus_proxy(ProfileManager *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void profile_manager_dispose(GObject *gobject)
{
	ProfileManager *self = PROFILE_MANAGER(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(profile_manager_parent_class)->dispose(gobject);
}

static void profile_manager_finalize (GObject *gobject)
{
	ProfileManager *self = PROFILE_MANAGER(gobject);
	G_OBJECT_CLASS(profile_manager_parent_class)->finalize(gobject);
}

static void profile_manager_class_init(ProfileManagerClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = profile_manager_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _profile_manager_get_property;
	gobject_class->set_property = _profile_manager_set_property;
	if (pspec)
		g_param_spec_unref(pspec);
}

static void profile_manager_init(ProfileManager *self)
{
	self->priv = profile_manager_get_instance_private (self);
	self->priv->proxy = NULL;
	g_assert(system_conn != NULL);
	GError *error = NULL;
	_profile_manager_create_gdbus_proxy(self, PROFILE_MANAGER_DBUS_SERVICE, PROFILE_MANAGER_DBUS_PATH, &error);
	g_assert(error == NULL);
}

static void _profile_manager_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ProfileManager *self = PROFILE_MANAGER(object);

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _profile_manager_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ProfileManager *self = PROFILE_MANAGER(object);
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
ProfileManager *profile_manager_new()
{
	return g_object_new(PROFILE_MANAGER_TYPE, NULL);
}

/* Private DBus proxy creation */
static void _profile_manager_create_gdbus_proxy(ProfileManager *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(PROFILE_MANAGER_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, PROFILE_MANAGER_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* void RegisterProfile(object profile, string uuid, dict options) */
void profile_manager_register_profile(ProfileManager *self, const gchar *profile, const gchar *uuid, const GVariant *options, GError **error)
{
	g_assert(PROFILE_MANAGER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RegisterProfile", g_variant_new ("(os@a{sv})", profile, uuid, options), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void UnregisterProfile(object profile) */
void profile_manager_unregister_profile(ProfileManager *self, const gchar *profile, GError **error)
{
	g_assert(PROFILE_MANAGER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "UnregisterProfile", g_variant_new ("(o)", profile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

