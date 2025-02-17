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
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "dbus-common.h"
#include "helpers.h"
#include "properties.h"

#include "obex_agent.h"
#include "bluez/obex/obex_transfer.h"

#define OBEX_AGENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), OBEX_AGENT_TYPE, ObexAgentPrivate))

struct _ObexAgentPrivate {
	gchar *root_folder;
	gboolean auto_accept;
        gchar *current_name;
        guint registration_id;
        void (*agent_released_callback)(ObexAgent *, gpointer);
        gpointer user_data;
        void (*agent_approved_callback)(ObexAgent *, const gchar *, const gchar *, const guint64, gpointer);
        gpointer approved_user_data;
};

G_DEFINE_TYPE_WITH_PRIVATE(ObexAgent, obex_agent, G_TYPE_OBJECT);

enum {
	PROP_0,
        PROP_ROOT_FOLDER, /* readwrite, construct only */
        PROP_AUTO_ACCPET, /* readwrite, construct only */
};

static const gchar *_obex_agent_introspect_xml = "<node name=\"/org/blueztools/obex\">\n\t<interface name=\"org.bluez.obex.Agent1\">\n\t\t<method name=\"Release\">\n\t\t</method>\n\t\t<method name=\"AuthorizePush\">\n\t\t\t<arg name=\"transfer\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"filepath\" direction=\"out\" type=\"s\"/>\n\t\t</method>\n\t\t<method name=\"Cancel\">\n\t\t</method>\n\t</interface>\n</node>\n";

/* Client API */
static gboolean _update_progress = FALSE;

static void _obex_agent_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _obex_agent_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _obex_agent_method_call_func(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, gpointer user_data);
static void _obex_agent_g_destroy_notify(gpointer data);

static void obex_agent_dispose(GObject *gobject)
{
	ObexAgent *self = OBEX_AGENT(gobject);

        if(self->priv->registration_id)
            g_dbus_connection_unregister_object(session_conn, self->priv->registration_id);
	/* Root folder free */
	g_free(self->priv->root_folder);
        /* callback free */
        if(self->priv->agent_released_callback)
            self->priv->agent_released_callback = NULL;
        /* user data free */
        if(self->priv->user_data)
            self->priv->user_data = NULL;
        /* callback free */
        if(self->priv->agent_approved_callback)
            self->priv->agent_approved_callback = NULL;
        /* user data free */
        if(self->priv->approved_user_data)
            self->priv->approved_user_data = NULL;
	/* Chain up to the parent class */
	G_OBJECT_CLASS(obex_agent_parent_class)->dispose(gobject);
}

static void obex_agent_finalize (GObject *gobject)
{
	ObexAgent *self = OBEX_AGENT(gobject);
	G_OBJECT_CLASS(obex_agent_parent_class)->finalize(gobject);
}

static void obex_agent_class_init(ObexAgentClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = obex_agent_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _obex_agent_get_property;
	gobject_class->set_property = _obex_agent_set_property;
	
	/* string RootFolder [readwrite, construct only] */
	pspec = g_param_spec_string("RootFolder", "root_folder", "Root folder location", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_ROOT_FOLDER, pspec);

	if (pspec) {
		g_param_spec_unref(pspec);
		pspec = NULL;
	}

	/* boolean AutoAccept [readwrite, construct only] */
	pspec = g_param_spec_boolean("AutoAccept", "auto_accept", "Automatically accept incoming files", FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_AUTO_ACCPET, pspec);

	if (pspec)
		g_param_spec_unref(pspec);
}

static void obex_agent_init(ObexAgent *self)
{
	self->priv = obex_agent_get_instance_private(self);
        g_assert(session_conn != NULL);
        self->priv->registration_id = 0;
        self->priv->root_folder = NULL;
        self->priv->agent_released_callback = NULL;
        self->priv->user_data = NULL;
        self->priv->agent_approved_callback = NULL;
        self->priv->approved_user_data = NULL;
		self->priv->auto_accept = FALSE;
        
        GError *error = NULL;
        GDBusInterfaceVTable obex_agent_table;
        memset(&obex_agent_table, 0x0, sizeof(obex_agent_table));
    
        GDBusNodeInfo *obex_agent_node_info = g_dbus_node_info_new_for_xml(_obex_agent_introspect_xml, &error);
        g_assert(error == NULL);
        GDBusInterfaceInfo *obex_agent_interface_info = g_dbus_node_info_lookup_interface(obex_agent_node_info, OBEX_AGENT_DBUS_INTERFACE);
        obex_agent_table.method_call = _obex_agent_method_call_func;
	self->priv->registration_id = g_dbus_connection_register_object(session_conn, OBEX_AGENT_DBUS_PATH, obex_agent_interface_info, &obex_agent_table, self, _obex_agent_g_destroy_notify, &error);
        g_assert(error == NULL);
        g_assert(self->priv->registration_id != 0);
        g_dbus_node_info_unref(obex_agent_node_info);
}

static void _obex_agent_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ObexAgent *self = OBEX_AGENT(object);

	switch (property_id) {
	case PROP_ROOT_FOLDER:
		g_value_set_string(value, self->priv->root_folder);
		break;

	case PROP_AUTO_ACCPET:
		g_value_set_boolean(value, self->priv->auto_accept);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _obex_agent_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ObexAgent *self = OBEX_AGENT(object);

	switch (property_id) {
        case PROP_ROOT_FOLDER:
		self->priv->root_folder = g_value_dup_string(value);
		break;

        case PROP_AUTO_ACCPET:
		self->priv->auto_accept = g_value_get_boolean(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/* Constructor */
ObexAgent *obex_agent_new(const gchar *root_folder, const gboolean auto_accept)
{
	return g_object_new(OBEX_AGENT_TYPE, "RootFolder", root_folder, "AutoAccept", auto_accept, NULL);
}
/* Methods */
static void _obex_agent_method_call_func(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, gpointer user_data)
{
    g_assert(user_data != NULL);
    ObexAgent *self = user_data;
    
    if (g_strcmp0(method_name, "AuthorizePush") == 0)
    {
        const gchar *transfer = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
        if (intf_supported(OBEX_TRANSFER_DBUS_SERVICE, transfer, OBEX_TRANSFER_DBUS_INTERFACE))
        {
            ObexTransfer *transfer_t = obex_transfer_new(transfer);
            g_print("[Transfer Request]\n");
            g_print("  Name: %s\n", obex_transfer_get_name(transfer_t, NULL));
            g_print("  Size: %" G_GINT64_FORMAT " bytes\n", obex_transfer_get_size(transfer_t, NULL));
            // Filename seems to be always NULL
            // g_print("  Filename: %s\n", obex_transfer_get_filename(transfer_t, NULL));
            const gchar *filename = obex_transfer_get_name(transfer_t, NULL);
            const guint64 size = obex_transfer_get_size(transfer_t, NULL);
            g_object_unref(transfer_t);

            gchar yn[4] = {0,};
			if (TRUE == self->priv->auto_accept)
				yn[0] = 'y';
			else {
				g_print("Accept (yes/no)? ");
				errno = 0;
				if (scanf("%3s", yn) == EOF && errno)
				{
					g_warning("%s\n", strerror(errno));
				}
			}
            if (g_strcmp0(yn, "y") == 0 || g_strcmp0(yn, "yes") == 0)
            {
                // IMPORTANT NOTE!
                // OBEX CANNOT WRITE FILES OUTSIDE OF /home/<user>/.cache/obexd
                // GVariant *ret = g_variant_new("(s)", g_build_filename(self->priv->root_folder, filename, NULL));
                // This will store the file in /home/<user>/.cache/obexd
                GVariant *ret = g_variant_new("(s)", filename);
                // g_print("invocation return value: %s\n", g_variant_print(ret, TRUE));
                
                // Call the callback to handle the filename and size
                if(self->priv->agent_approved_callback)
                    (*self->priv->agent_approved_callback)(self, transfer, filename, size, self->priv->approved_user_data);
                
                // Return string
                g_dbus_method_invocation_return_value(invocation, ret);
                return;
            }
            else
            {
                // Return error
                g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.obex.Error.Rejected", "File transfer rejected");
                return;
            }
        }
        else
        {
            g_print("Error: Unknown transfer request\n");
            // Return error
            g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.obex.Error.Rejected", "File transfer rejected");
            return;
        }
    }
    else if (g_strcmp0(method_name, "Cancel") == 0)
    {
        g_print("Request cancelled\n");
        // Return void
        g_dbus_method_invocation_return_value(invocation, NULL);
        return;
    }
    else if (g_strcmp0(method_name, "Release") == 0)
    {
        if (_update_progress)
        {
            g_print("\n");
            _update_progress = FALSE;
        }

        g_print("OBEXAgent released\n");

        (*self->priv->agent_released_callback)(self, self->priv->user_data);
        
        // Return void
        g_dbus_method_invocation_return_value(invocation, NULL);
        return;
    }
}

static void _obex_agent_g_destroy_notify(gpointer data)
{
    g_free(data);
}

void obex_agent_set_release_callback(ObexAgent *self, ObexAgentReleasedCallback callback_function, gpointer user_data)
{
    g_assert(OBEX_AGENT_IS(self));
    self->priv->agent_released_callback = callback_function;
    self->priv->user_data = user_data;
}

void obex_agent_clear_release_callback(ObexAgent *self)
{
    g_assert(OBEX_AGENT_IS(self));
    self->priv->agent_released_callback = NULL;
    self->priv->user_data = NULL;
}

void obex_agent_set_approved_callback(ObexAgent *self, ObexAgentApprovedCallback callback_function, gpointer user_data)
{
    g_assert(OBEX_AGENT_IS(self));
    self->priv->agent_approved_callback = callback_function;
    self->priv->approved_user_data = user_data;
}

void obex_agent_clear_approved_callback(ObexAgent *self)
{
    g_assert(OBEX_AGENT_IS(self));
    self->priv->agent_approved_callback = NULL;
    self->priv->approved_user_data = NULL;
}