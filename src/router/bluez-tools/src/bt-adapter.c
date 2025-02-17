/* 
 * File:   main.c
 * Author: workout
 *
 * Created on April 18, 2014, 1:14 PM
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>
#include <glib.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dbus-common.h"
#include "lib/bluez-api.h"
#include "lib/helpers.h"

static void _adapter_property_changed(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    g_assert(user_data != NULL);
    GMainLoop *mainloop = user_data;
    
    GVariant *changed_properties = g_variant_get_child_value(parameters, 1);
    GVariant *discovering_variant = g_variant_lookup_value(changed_properties, "Discovering", NULL);
    if(discovering_variant)
    {
        const gboolean discovering = g_variant_get_boolean(discovering_variant);
        if(!discovering)
        {
            g_main_loop_quit(mainloop);
        }
        g_variant_unref(discovering_variant);
    }
    g_variant_unref(changed_properties);
}

static void _manager_device_found(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    g_assert(user_data != NULL);
    const gchar *adapter_object_path = user_data;

    GVariant *arg0 = g_variant_get_child_value(parameters, 0);
    const gchar *str_object_path = g_variant_get_string(arg0, NULL);
    g_variant_unref(arg0);

    if (!g_str_has_prefix(str_object_path, adapter_object_path))
        return;

    GVariant *interfaces_and_properties = g_variant_get_child_value(parameters, 1);
    GVariant *properties = NULL;
    if (g_variant_lookup(interfaces_and_properties, DEVICE_DBUS_INTERFACE, "@a{sv}", &properties))
    {
        g_print("[%s]\n", g_variant_get_string(g_variant_lookup_value(properties, "Address", NULL), NULL));
        g_print("  Name: %s\n", g_variant_lookup_value(properties, "Name", NULL) != NULL ? g_variant_get_string(g_variant_lookup_value(properties, "Name", NULL), NULL) : NULL);
        g_print("  Alias: %s\n", g_variant_lookup_value(properties, "Alias", NULL) != NULL ? g_variant_get_string(g_variant_lookup_value(properties, "Alias", NULL), NULL) : NULL);
        g_print("  Address: %s\n", g_variant_lookup_value(properties, "Address", NULL) != NULL ? g_variant_get_string(g_variant_lookup_value(properties, "Address", NULL), NULL) : NULL);
        g_print("  Icon: %s\n", g_variant_lookup_value(properties, "Icon", NULL) != NULL ? g_variant_get_string(g_variant_lookup_value(properties, "Icon", NULL), NULL) : NULL);
        g_print("  Class: 0x%x\n", g_variant_lookup_value(properties, "Class", NULL) != NULL ? g_variant_get_uint32(g_variant_lookup_value(properties, "Class", NULL)) : 0x0);
        g_print("  LegacyPairing: %d\n", g_variant_lookup_value(properties, "LegacyPairing", NULL) != NULL ? g_variant_get_boolean(g_variant_lookup_value(properties, "LegacyPairing", NULL)) : FALSE);
        g_print("  Paired: %d\n", g_variant_lookup_value(properties, "Paired", NULL) != NULL ? g_variant_get_boolean(g_variant_lookup_value(properties, "Paired", NULL)) : FALSE);
        g_print("  RSSI: %d\n", g_variant_lookup_value(properties, "RSSI", NULL) != NULL ? g_variant_get_int16(g_variant_lookup_value(properties, "RSSI", NULL)) : 0x0);
        g_print("\n");

        g_variant_unref(properties);
    }
    g_variant_unref(interfaces_and_properties);
}

static gboolean list_arg = FALSE;
static gchar *adapter_arg = NULL;
static gboolean info_arg = FALSE;
static gboolean discover_arg = FALSE;
static gboolean set_arg = FALSE;
static gchar *set_property_arg = NULL;
static gchar *set_value_arg = NULL;

static GOptionEntry entries[] = {
    {"list", 'l', 0, G_OPTION_ARG_NONE, &list_arg, "List all available adapters", NULL},
    {"adapter", 'a', 0, G_OPTION_ARG_STRING, &adapter_arg, "Adapter Name or MAC", "<name|mac>"},
    {"info", 'i', 0, G_OPTION_ARG_NONE, &info_arg, "Show adapter info", NULL},
    {"discover", 'd', 0, G_OPTION_ARG_NONE, &discover_arg, "Discover remote devices", NULL},
    {"set", 's', 0, G_OPTION_ARG_NONE, &set_arg, "Set adapter property", NULL},
    {NULL}
};

/*
 * 
 */
int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;

    /* Query current locale */
    setlocale(LC_CTYPE, "");

    // g_type_init(); // DEPRECATED
    dbus_init();

    context = g_option_context_new("- a bluetooth adapter manager");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_set_summary(context, "Version "PACKAGE_VERSION);
    g_option_context_set_description(context,
                                     "Set Options:\n"
                                     "  --set <property> <value>\n"
                                     "  Where `property` is one of:\n"
                                     "     Alias\n"
                                     "     Discoverable\n"
                                     "     DiscoverableTimeout\n"
                                     "     Pairable\n"
                                     "     PairableTimeout\n"
                                     "     Powered\n\n"
                                     "Report bugs to <"PACKAGE_BUGREPORT">."
                                     "Project home page <"PACKAGE_URL">."
                                     );

    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        g_print("%s: %s\n", g_get_prgname(), error->message);
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }
    else if (!list_arg && !info_arg && !discover_arg && !set_arg)
    {
        g_print("%s", g_option_context_get_help(context, FALSE, NULL));
        exit(EXIT_FAILURE);
    }
    else if (set_arg && (argc != 3 || strlen(argv[1]) == 0 || strlen(argv[2]) == 0))
    {
        g_print("%s: Invalid arguments for --set\n", g_get_prgname());
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }

    g_option_context_free(context);

    if (!dbus_system_connect(&error))
    {
        g_printerr("Couldn't connect to DBus system bus: %s\n", error->message);
        exit(EXIT_FAILURE);
    }

    /* Check, that bluetooth daemon is running */
    if (!intf_supported(BLUEZ_DBUS_SERVICE_NAME, MANAGER_DBUS_PATH, MANAGER_DBUS_INTERFACE))
    {
        g_printerr("%s: bluez service is not found\n", g_get_prgname());
        g_printerr("Did you forget to run bluetoothd?\n");
        exit(EXIT_FAILURE);
    }

    Manager *manager = g_object_new(MANAGER_TYPE, NULL);

    if (list_arg)
    {
        const GPtrArray *adapters_list = manager_get_adapters(manager);
        g_assert(adapters_list != NULL);

        if (adapters_list->len == 0)
        {
            g_print("No adapters found\n");
            exit(EXIT_FAILURE);
        }

        g_print("Available adapters:\n");
        for (int i = 0; i < adapters_list->len; i++)
        {
            const gchar *adapter_path = g_ptr_array_index(adapters_list, i);
            Adapter *adapter = adapter_new(adapter_path);
            g_print("%s (%s)\n", adapter_get_name(adapter, &error), adapter_get_address(adapter, &error));
            g_object_unref(adapter);
        }
    }
    else if (info_arg)
    {
        Adapter *adapter = find_adapter(adapter_arg, &error);
        exit_if_error(error);

        gchar *adapter_intf = g_path_get_basename(adapter_get_dbus_object_path(adapter));
        g_print("[%s]\n", adapter_intf);
        g_print("  Name: %s\n", adapter_get_name(adapter, &error));
        g_print("  Address: %s\n", adapter_get_address(adapter, &error));
        g_print("  Alias: %s [rw]\n", adapter_get_alias(adapter, &error));
        g_print("  Class: 0x%x\n", adapter_get_class(adapter, &error));
        g_print("  Discoverable: %d [rw]\n", adapter_get_discoverable(adapter, &error));
        g_print("  DiscoverableTimeout: %d [rw]\n", adapter_get_discoverable_timeout(adapter, &error));
        g_print("  Discovering: %d\n", adapter_get_discovering(adapter, &error));
        g_print("  Pairable: %d [rw]\n", adapter_get_pairable(adapter, &error));
        g_print("  PairableTimeout: %d [rw]\n", adapter_get_pairable_timeout(adapter, &error));
        g_print("  Powered: %d [rw]\n", adapter_get_powered(adapter, &error));
        g_print("  UUIDs: [");
        const gchar **uuids = adapter_get_uuids(adapter, &error);
        for (int j = 0; uuids[j] != NULL; j++)
        {
            if (j > 0) g_print(", ");
            g_print("%s", uuid2name(uuids[j]));
        }
        g_print("]\n");

        g_free(adapter_intf);
        g_object_unref(adapter);
    }
    else if (discover_arg)
    {
        Adapter *adapter = find_adapter(adapter_arg, &error);
        exit_if_error(error);

        // To store pairs MAC => Name
        GHashTable *found_devices = g_hash_table_new(g_str_hash, g_str_equal);

        // Mainloop
        GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);

        guint object_sig_sub_id = g_dbus_connection_signal_subscribe(system_conn, "org.bluez", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _manager_device_found, (gpointer) adapter_get_dbus_object_path(adapter), NULL);
        exit_if_error(error);
        guint prop_sig_sub_id = g_dbus_connection_signal_subscribe(system_conn, "org.bluez", "org.freedesktop.DBus.Properties", "PropertiesChanged", adapter_get_dbus_object_path(adapter), NULL, G_DBUS_SIGNAL_FLAGS_NONE, _adapter_property_changed, mainloop, NULL);
        exit_if_error(error);
        
        g_print("Searching...\n");
        adapter_start_discovery(adapter, &error);
        exit_if_error(error);

        g_main_loop_run(mainloop);
        /* Discovering process here... */
        g_main_loop_unref(mainloop);

        g_dbus_connection_signal_unsubscribe(system_conn, object_sig_sub_id);
        exit_if_error(error);
        g_dbus_connection_signal_unsubscribe(system_conn, prop_sig_sub_id);
        exit_if_error(error);
        
        g_print("Done\n");
        g_hash_table_unref(found_devices);
        g_object_unref(adapter);
    }
    else if (set_arg)
    {
        set_property_arg = argv[1];
        set_value_arg = argv[2];

        Adapter *adapter = find_adapter(adapter_arg, &error);
        exit_if_error(error);

        GVariant *v = NULL;

        if (g_strcmp0(set_property_arg, "Alias") == 0)
        {
            v = g_variant_new_string(set_value_arg);
        }
        else if (
                g_strcmp0(set_property_arg, "Discoverable") == 0 ||
                g_strcmp0(set_property_arg, "Pairable") == 0 ||
                g_strcmp0(set_property_arg, "Powered") == 0
                )
        {
            if (g_strcmp0(set_value_arg, "0") == 0 || g_ascii_strcasecmp(set_value_arg, "FALSE") == 0 || g_ascii_strcasecmp(set_value_arg, "OFF") == 0)
            {
                v = g_variant_new_boolean(FALSE);
            }
            else if (g_strcmp0(set_value_arg, "1") == 0 || g_ascii_strcasecmp(set_value_arg, "TRUE") == 0 || g_ascii_strcasecmp(set_value_arg, "ON") == 0)
            {
                v = g_variant_new_boolean(TRUE);
            }
            else
            {
                g_print("%s: Invalid boolean value: %s\n", g_get_prgname(), set_value_arg);
                g_print("Try `%s --help` for more information.\n", g_get_prgname());
                exit(EXIT_FAILURE);
            }
        }
        else if (
                g_strcmp0(set_property_arg, "DiscoverableTimeout") == 0 ||
                g_strcmp0(set_property_arg, "PairableTimeout") == 0
                )
        {
            v = g_variant_new_uint32((guint32) atoi(set_value_arg));
        }
        else
        {
            g_print("%s: Invalid property: %s\n", g_get_prgname(), set_property_arg);
            g_print("Try `%s --help` for more information.\n", g_get_prgname());
            exit(EXIT_FAILURE);
        }

        GVariant *props = adapter_get_properties(adapter, &error);
        exit_if_error(error);
        
        if(g_ascii_strcasecmp(set_property_arg, "Alias") == 0)
            adapter_set_alias(adapter, g_variant_get_string(v, NULL), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "Discoverable") == 0)
            adapter_set_discoverable(adapter, g_variant_get_boolean(v), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "DiscoverableTimeout") == 0)
            adapter_set_discoverable_timeout(adapter, g_variant_get_uint32(v), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "Pairable") == 0)
            adapter_set_pairable(adapter, g_variant_get_boolean(v), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "PairableTimeout") == 0)
            adapter_set_pairable_timeout(adapter, g_variant_get_uint32(v), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "Powered") == 0)
            adapter_set_powered(adapter, g_variant_get_boolean(v), &error);
        
        exit_if_error(error);
        
        GVariant *old_value = g_variant_lookup_value(props, set_property_arg, NULL);
        g_assert(old_value != NULL);
        if (g_variant_type_equal(G_VARIANT_TYPE_STRING, g_variant_get_type(old_value)))
        {
            g_print("%s: %s -> %s\n", set_property_arg, g_variant_get_string(old_value, NULL), g_variant_get_string(v, NULL));
        }
        else if (g_variant_type_equal(G_VARIANT_TYPE_BOOLEAN, g_variant_get_type(old_value)))
        {
            g_print("%s: %u -> %u\n", set_property_arg, g_variant_get_boolean(old_value), g_variant_get_boolean(v));
        }
        else if (g_variant_type_equal(G_VARIANT_TYPE_UINT32, g_variant_get_type(old_value)))
        {
            g_print("%s: %u -> %u\n", set_property_arg, g_variant_get_uint32(old_value), g_variant_get_uint32(v));
        }
        g_variant_unref(props);

        // g_variant_unref(v); /* Floating references do not need to be unref */
        g_object_unref(adapter);
    }

    g_object_unref(manager);
    dbus_disconnect();

    exit(EXIT_SUCCESS);
}
