/*
 *
 *  bluez-tools - a set of tools to manage bluetooth devices for linux
 *
 *  Copyright (C) 2010-2011  Alexander Orlenko <zxteam@gmail.com>
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

#include <errno.h>
#include <gio/gio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include "agent-helper.h"

gboolean agent_need_unregister;

static const gchar *_bt_agent_introspect_xml = "<node name=\"/org/blueztools\">\n\t<interface name=\"org.bluez.Agent1\">\n\t\t<method name=\"Release\">\n\t\t</method>\n\t\t<method name=\"RequestPinCode\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"pincode\" direction=\"out\" type=\"s\"/>\n\t\t</method>\n\t\t<method name=\"DisplayPinCode\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"pincode\" direction=\"in\" type=\"s\"/>\n\t\t</method>\n\t\t<method name=\"RequestPasskey\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"passkey\" direction=\"out\" type=\"u\"/>\n\t\t</method>\n\t\t<method name=\"DisplayPasskey\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"passkey\" direction=\"in\" type=\"u\"/>\n\t\t\t<arg name=\"entered\" direction=\"in\" type=\"q\"/>\n\t\t</method>\n\t\t<method name=\"RequestConfirmation\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"passkey\" direction=\"in\" type=\"u\"/>\n\t\t</method>\n\t\t<method name=\"RequestAuthorization\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t</method>\n\t\t<method name=\"AuthorizeService\">\n\t\t\t<arg name=\"device\" direction=\"in\" type=\"o\"/>\n\t\t\t<arg name=\"uuid\" direction=\"in\" type=\"s\"/>\n\t\t</method>\n\t\t<method name=\"Cancel\">\n\t\t</method>\n\t</interface>\n</node>\n";
static guint _bt_agent_registration_id = 0;
static GHashTable *_pin_hash_table = NULL;
static gboolean _interactive = TRUE;
static GMainLoop *_mainloop = NULL;

static void _bt_agent_g_destroy_notify(gpointer data);
static void _bt_agent_method_call_func(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, gpointer user_data);
static const gchar *_find_device_pin(const gchar *device_path);

static void _bt_agent_method_call_func(GDBusConnection *connection, const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name, GVariant *parameters, GDBusMethodInvocation *invocation, gpointer user_data)
{
    // g_print("%s%s\n", method_name, g_variant_print(parameters, FALSE));

    if (g_strcmp0(method_name, "AuthorizeService") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));
        const char *uuid = g_variant_get_string(g_variant_get_child_value(parameters, 1), NULL);

        if (_interactive)
          g_print("Device: %s (%s) for UUID %s\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error), uuid);

        if (error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
            g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Internal error occurred");
            return;
        }

        if (device_get_paired (device_obj, &error))
        {
            g_dbus_method_invocation_return_value(invocation, NULL);
        }
        else if (error)
        {
            g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Internal error occurred");
            g_error_free (error);
        }
        else
        {
            g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Service authorization rejected");
        }
    }
    else if (g_strcmp0(method_name, "Cancel") == 0)
    {
        if (_interactive)
            g_print("Request canceled\n");
        // Return void
        g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (g_strcmp0(method_name, "DisplayPasskey") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));
        const gchar *pin = _find_device_pin(device_get_dbus_object_path(device_obj));

        if (_interactive)
            g_print("Device: %s (%s)\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error));

        if (error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }

        g_object_unref(device_obj);

        if (_interactive)
        {
            g_print("Passkey: %u, entered: %u\n", g_variant_get_uint32(g_variant_get_child_value(parameters, 1)), g_variant_get_uint16(g_variant_get_child_value(parameters, 2)));
            g_dbus_method_invocation_return_value(invocation, NULL);
            return;
        }
        else if (pin != NULL)
        {
            /* OK, device found */
            g_dbus_method_invocation_return_value(invocation, NULL);
            return;
        }

        g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Pairing rejected");
    }
    else if (g_strcmp0(method_name, "DisplayPinCode") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));
        const gchar *pin = _find_device_pin(device_get_dbus_object_path(device_obj));
        const gchar *pincode = g_variant_get_string(g_variant_get_child_value(parameters, 1), NULL);

        if (_interactive)
            g_print("Device: %s (%s)\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error));

        if (error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }

        g_object_unref(device_obj);

        /* Try to use found PIN */
        if (pin != NULL)
        {
            if (g_strcmp0(pin, "*") == 0 || g_strcmp0(pin, pincode) == 0)
            {
                if (_interactive)
                    g_print("Pin code confirmed\n");
                g_dbus_method_invocation_return_value(invocation, NULL);
            }
            else
                g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Passkey does not match");

            return;
        }
        else if (_interactive)
        {
            g_print("Confirm pin code: %s (yes/no)? ", pincode);

            gchar yn[4] = {0,};
            errno = 0;
            if (scanf("%3s", yn) == EOF && errno)
                g_warning("%s\n", strerror(errno));
            if(g_ascii_strcasecmp(yn, "yes") == 0 || g_ascii_strcasecmp(yn, "y") == 0)
                g_dbus_method_invocation_return_value(invocation, NULL);
            else
                g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Passkey does not match");
            return;
        }

        g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Pairing rejected");
    }
    else if (g_strcmp0(method_name, "Release") == 0)
    {
        agent_need_unregister = FALSE;

        if(_mainloop)
            if (g_main_loop_is_running(_mainloop))
                g_main_loop_quit(_mainloop);

        g_print("Agent released\n");

        // Return void
        g_dbus_method_invocation_return_value(invocation, NULL);
    }
    else if (g_strcmp0(method_name, "RequestAuthorization") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));

        if (_interactive)
            g_print("Device: %s (%s)\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error));

        if(error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }

        g_object_unref(device_obj);

        if (_interactive)
        {
            g_print("Authorize this device pairing (yes/no)? ");
            gchar yn[4] = {0,};
            errno = 0;
            if (scanf("%3s", yn) == EOF && errno)
                g_warning("%s\n", strerror(errno));
            if(g_ascii_strcasecmp(yn, "yes") == 0 || g_ascii_strcasecmp(yn, "y") == 0)
                g_dbus_method_invocation_return_value(invocation, NULL);
            else
                g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Pairing rejected");
            return;
        }

        g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Pairing rejected");
    }
    else if (g_strcmp0(method_name, "RequestConfirmation") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));
        guint32 passkey = g_variant_get_uint32(g_variant_get_child_value(parameters, 1));
        const gchar *pin = _find_device_pin(device_get_dbus_object_path(device_obj));

        if (_interactive)
            g_print("Device: %s (%s)\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error));

        if(error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }

        g_object_unref(device_obj);

        /* Try to use found PIN */
        if (pin != NULL)
        {
            guint32 passkey_t;
            sscanf(pin, "%u", &passkey_t);

            if (g_strcmp0(pin, "*") == 0 || passkey_t == passkey)
            {
                if (_interactive)
                    g_print("Passkey confirmed\n");
                g_dbus_method_invocation_return_value(invocation, NULL);
            }
            else
                g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Passkey does not match");

            return;
        }
        else if (_interactive)
        {
            g_print("Confirm passkey: %u (yes/no)? ", passkey);
            gchar yn[4] = {0,};
            errno = 0;
            if (scanf("%3s", yn) == EOF && errno)
                g_warning("%s\n", strerror(errno));
            if(g_ascii_strcasecmp(yn, "yes") == 0 || g_ascii_strcasecmp(yn, "y") == 0)
                g_dbus_method_invocation_return_value(invocation, NULL);
            else
                g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Passkey does not match");
            return;
        }

        g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "Passkey does not match");
    }
    else if (g_strcmp0(method_name, "RequestPasskey") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));
        const gchar *pin = _find_device_pin(device_get_dbus_object_path(device_obj));
        guint32 ret = 0;
        gboolean invoke = FALSE;

        if (_interactive)
            g_print("Device: %s (%s)\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error));

        if(error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }

        g_object_unref(device_obj);

        /* Try to use found PIN */
        if (pin != NULL)
        {
            if (_interactive)
                g_print("Passkey found\n");
            sscanf(pin, "%u", &ret);
            invoke = TRUE;
        }
        else if (_interactive)
        {
            g_print("Enter passkey: ");
            errno = 0;
            if (scanf("%u", &ret) == EOF && errno)
                g_warning("%s\n", strerror(errno));
            invoke = TRUE;
        }

        if (invoke)
        {
            g_dbus_method_invocation_return_value(invocation, g_variant_new ("(u)", ret));
        }
        else
        {
            g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "No passkey inputted");
        }
    }
    else if (g_strcmp0(method_name, "RequestPinCode") == 0)
    {
        GError *error = NULL;
        Device *device_obj = device_new(g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL));
        const gchar *pin = _find_device_pin(device_get_dbus_object_path(device_obj));
        gchar *ret = NULL;
        gboolean invoke = FALSE;

        if (_interactive)
            g_print("Device: %s (%s)\n", device_get_alias(device_obj, &error), device_get_address(device_obj, &error));

        if(error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }

        g_object_unref(device_obj);

        /* Try to use found PIN */
        if (pin != NULL)
        {
            if (_interactive)
                g_print("Passkey found\n");
            sscanf(pin, "%ms", &ret);
            invoke = TRUE;
        }
        else if (_interactive)
        {
            g_print("Enter passkey: ");
            errno = 0;
            if (scanf("%ms", &ret) == EOF && errno)
                g_warning("%s\n", strerror(errno));
            invoke = TRUE;
        }

        if (invoke)
        {
            g_dbus_method_invocation_return_value(invocation, g_variant_new ("(s)", ret));
        }
        else
        {
            g_dbus_method_invocation_return_dbus_error(invocation, "org.bluez.Error.Rejected", "No passkey inputted");
        }

        if (ret)
            free(ret);
    }
}

static void _bt_agent_g_destroy_notify(gpointer data)
{
    g_free(data);
}

static const gchar *_find_device_pin(const gchar *device_path)
{
    if (_pin_hash_table)
    {
        GError *error = NULL;
        Device *device = device_new(device_path);
        const gchar *pin_by_addr = g_hash_table_lookup(_pin_hash_table, device_get_address(device, &error));
        const gchar *pin_by_alias = g_hash_table_lookup(_pin_hash_table, device_get_alias(device, &error));
        if(error)
        {
            g_critical("Failed to get remote device's MAC address: %s", error->message);
            g_error_free(error);
        }
        g_object_unref(device);
        const gchar *pin_all = g_hash_table_lookup(_pin_hash_table, "*");
        if (pin_by_addr)
            return pin_by_addr;
        else if (pin_by_alias)
            return pin_by_alias;
        else if (pin_all)
            return pin_all;
    }
    return NULL;
}

void register_agent_callbacks(gboolean interactive_console, GHashTable *pin_dictonary, gpointer main_loop_object, GError **error)
{
    GDBusInterfaceVTable bt_agent_table;
    memset(&bt_agent_table, 0x0, sizeof(bt_agent_table));

    if(pin_dictonary)
        _pin_hash_table = pin_dictonary;
    if(main_loop_object)
        _mainloop = (GMainLoop *) main_loop_object;

    _interactive = interactive_console;

    GDBusNodeInfo *bt_agent_node_info = g_dbus_node_info_new_for_xml(_bt_agent_introspect_xml, error);
    GDBusInterfaceInfo *bt_agent_interface_info = g_dbus_node_info_lookup_interface(bt_agent_node_info, AGENT_DBUS_INTERFACE);
    bt_agent_table.method_call = _bt_agent_method_call_func;
    _bt_agent_registration_id = g_dbus_connection_register_object(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, error), AGENT_PATH, bt_agent_interface_info, &bt_agent_table, NULL, _bt_agent_g_destroy_notify, error);
    g_dbus_node_info_unref(bt_agent_node_info);
}

void unregister_agent_callbacks(GError **error)
{
    if (_bt_agent_registration_id)
        g_dbus_connection_unregister_object(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, error), _bt_agent_registration_id);
}
