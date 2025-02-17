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

#include "health_manager.h"

#define HEALTH_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), HEALTH_MANAGER_TYPE, HealthManagerPrivate))

struct _HealthManagerPrivate {
	GDBusProxy *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE(HealthManager, health_manager, G_TYPE_OBJECT);

enum {
	PROP_0,
};

static void _health_manager_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _health_manager_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _health_manager_create_gdbus_proxy(HealthManager *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void health_manager_dispose(GObject *gobject)
{
	HealthManager *self = HEALTH_MANAGER(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(health_manager_parent_class)->dispose(gobject);
}

static void health_manager_finalize (GObject *gobject)
{
	HealthManager *self = HEALTH_MANAGER(gobject);
	G_OBJECT_CLASS(health_manager_parent_class)->finalize(gobject);
}

static void health_manager_class_init(HealthManagerClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = health_manager_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _health_manager_get_property;
	gobject_class->set_property = _health_manager_set_property;
	if (pspec)
		g_param_spec_unref(pspec);
}

static void health_manager_init(HealthManager *self)
{
	self->priv = health_manager_get_instance_private (self);
	self->priv->proxy = NULL;
	g_assert(system_conn != NULL);
	GError *error = NULL;
	_health_manager_create_gdbus_proxy(self, HEALTH_MANAGER_DBUS_SERVICE, HEALTH_MANAGER_DBUS_PATH, &error);
	g_assert(error == NULL);
}

static void _health_manager_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	HealthManager *self = HEALTH_MANAGER(object);

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _health_manager_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	HealthManager *self = HEALTH_MANAGER(object);
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
HealthManager *health_manager_new()
{
	return g_object_new(HEALTH_MANAGER_TYPE, NULL);
}

/* Private DBus proxy creation */
static void _health_manager_create_gdbus_proxy(HealthManager *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(HEALTH_MANAGER_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, HEALTH_MANAGER_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* object CreateApplication(dict config) */
const gchar *health_manager_create_application(HealthManager *self, const GVariant *config, GError **error)
{
	g_assert(HEALTH_MANAGER_IS(self));
	const gchar *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "CreateApplication", g_variant_new ("(@a{sv})", config), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret != NULL)
		return NULL;
	proxy_ret = g_variant_get_child_value(proxy_ret, 0);
	ret = g_variant_get_string(proxy_ret, NULL);
	g_variant_unref(proxy_ret);
	return ret;
}

/* void DestroyApplication(object application) */
void health_manager_destroy_application(HealthManager *self, const gchar *application, GError **error)
{
	g_assert(HEALTH_MANAGER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "DestroyApplication", g_variant_new ("(o)", application), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

