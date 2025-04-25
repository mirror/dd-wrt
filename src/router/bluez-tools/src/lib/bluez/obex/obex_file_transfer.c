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

#include "obex_file_transfer.h"

#define OBEX_FILE_TRANSFER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OBEX_FILE_TRANSFER_TYPE, ObexFileTransferPrivate))

struct _ObexFileTransferPrivate {
	GDBusProxy *proxy;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(ObexFileTransfer, obex_file_transfer, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _obex_file_transfer_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _obex_file_transfer_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _obex_file_transfer_create_gdbus_proxy(ObexFileTransfer *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void obex_file_transfer_dispose(GObject *gobject)
{
	ObexFileTransfer *self = OBEX_FILE_TRANSFER(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(obex_file_transfer_parent_class)->dispose(gobject);
}

static void obex_file_transfer_finalize (GObject *gobject)
{
	ObexFileTransfer *self = OBEX_FILE_TRANSFER(gobject);
	G_OBJECT_CLASS(obex_file_transfer_parent_class)->finalize(gobject);
}

static void obex_file_transfer_class_init(ObexFileTransferClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = obex_file_transfer_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _obex_file_transfer_get_property;
	gobject_class->set_property = _obex_file_transfer_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "ObexFileTransfer D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void obex_file_transfer_init(ObexFileTransfer *self)
{
	self->priv = obex_file_transfer_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->object_path = NULL;
	g_assert(session_conn != NULL);
}

static void _obex_file_transfer_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ObexFileTransfer *self = OBEX_FILE_TRANSFER(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_value_set_string(value, obex_file_transfer_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _obex_file_transfer_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ObexFileTransfer *self = OBEX_FILE_TRANSFER(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_obex_file_transfer_create_gdbus_proxy(self, OBEX_FILE_TRANSFER_DBUS_SERVICE, self->priv->object_path, &error);
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
ObexFileTransfer *obex_file_transfer_new(const gchar *dbus_object_path)
{
	return g_object_new(OBEX_FILE_TRANSFER_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _obex_file_transfer_create_gdbus_proxy(ObexFileTransfer *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(session_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, OBEX_FILE_TRANSFER_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;
}

/* Methods */

/* Get DBus object path */
const gchar *obex_file_transfer_get_dbus_object_path(ObexFileTransfer *self)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}

/* void ChangeFolder(string folder) */
void obex_file_transfer_change_folder(ObexFileTransfer *self, const gchar *folder, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "ChangeFolder", g_variant_new ("(s)", folder), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void CopyFile(string sourcefile, string targetfile) */
void obex_file_transfer_copy_file(ObexFileTransfer *self, const gchar *sourcefile, const gchar *targetfile, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "CopyFile", g_variant_new ("(ss)", sourcefile, targetfile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void CreateFolder(string folder) */
void obex_file_transfer_create_folder(ObexFileTransfer *self, const gchar *folder, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "CreateFolder", g_variant_new ("(s)", folder), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* void Delete(string file) */
void obex_file_transfer_delete(ObexFileTransfer *self, const gchar *file, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "Delete", g_variant_new ("(s)", file), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* object, dict GetFile(string targetfile, string sourcefile) */
GVariant *obex_file_transfer_get_file(ObexFileTransfer *self, const gchar *targetfile, const gchar *sourcefile, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	GVariant *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "GetFile", g_variant_new ("(ss)", targetfile, sourcefile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret == NULL)
		return NULL;
	ret = g_variant_ref_sink(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}

/* array{dict} ListFolder() */
GVariant *obex_file_transfer_list_folder(ObexFileTransfer *self, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	GVariant *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "ListFolder", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret == NULL)
		return NULL;
	ret = g_variant_ref_sink(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}

/* void MoveFile(string sourcefile, string targetfile) */
void obex_file_transfer_move_file(ObexFileTransfer *self, const gchar *sourcefile, const gchar *targetfile, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	g_dbus_proxy_call_sync(self->priv->proxy, "MoveFile", g_variant_new ("(ss)", sourcefile, targetfile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
}

/* object, dict PutFile(string sourcefile, string targetfile) */
GVariant *obex_file_transfer_put_file(ObexFileTransfer *self, const gchar *sourcefile, const gchar *targetfile, GError **error)
{
	g_assert(OBEX_FILE_TRANSFER_IS(self));
	GVariant *ret = NULL;
	GVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, "PutFile", g_variant_new ("(ss)", sourcefile, targetfile), G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);
	if (proxy_ret == NULL)
		return NULL;
	ret = g_variant_ref_sink(proxy_ret);
	g_variant_unref(proxy_ret);
	return ret;
}
