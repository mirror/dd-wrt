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

#include "alert.h"

#define ALERT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), ALERT_TYPE, AlertPrivate))

struct _AlertPrivate {
	GDBusProxy *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE(Alert, alert, G_TYPE_OBJECT);

enum {
	PROP_0,
};

static void _alert_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _alert_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _alert_create_gdbus_proxy(Alert *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void alert_dispose(GObject *gobject)
{
	Alert *self = ALERT(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(alert_parent_class)->dispose(gobject);
}

static void alert_finalize (GObject *gobject)
{
	Alert *self = ALERT(gobject);
	G_OBJECT_CLASS(alert_parent_class)->finalize(gobject);
}

static void alert_class_init(AlertClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = alert_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _alert_get_property;
	gobject_class->set_property = _alert_set_property;
	if (pspec)
		g_param_spec_unref(pspec);
}

static void alert_init(Alert *self)
{
	self->priv = alert_get_instance_private (self);
	self->priv->proxy = NULL;
	g_assert(system_conn != NULL);
	GError *error = NULL;
	_alert_create_gdbus_proxy(self, ALERT_DBUS_SERVICE, ALERT_DBUS_PATH, &error);
	g_assert(error == NULL);
}

static void _alert_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	Alert *self = ALERT(object);

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _alert_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	Alert *self = ALERT(object);
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
Alert *alert_new()
{
	return g_object_new(ALERT_TYPE, NULL);
}

/* Private DBus proxy creation */
static void _alert_create_gdbus_proxy(Alert *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(ALERT_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, ALERT_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* void NewAlert(string category, uint16 count, string description) */
void alert_new_alert(Alert *self, const gchar *category, const guint16 count, const gchar *description, GError **error)
{
	g_assert(ALERT_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "NewAlert", g_variant_new ("(sqs)", category, count, description), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void RegisterAlert(string category, object agent) */
void alert_register_alert(Alert *self, const gchar *category, const gchar *agent, GError **error)
{
	g_assert(ALERT_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "RegisterAlert", g_variant_new ("(so)", category, agent), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void UnreadAlert(string category, uint16 count) */
void alert_unread_alert(Alert *self, const gchar *category, const guint16 count, GError **error)
{
	g_assert(ALERT_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "UnreadAlert", g_variant_new ("(sq)", category, count), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

