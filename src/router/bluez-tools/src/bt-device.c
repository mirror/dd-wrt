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

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixinputstream.h>

#include "lib/dbus-common.h"
#include "lib/helpers.h"
#include "lib/agent-helper.h"
#include "lib/sdp.h"
#include "lib/bluez-api.h"

enum
{
    REC,
    ATTR,
    SEQ,
    ELEM,

    ATTR_ID,
    UUID_ID,

    LAST_E
};

static int xml_t[LAST_E] = {0, 0, 0, 0, -1, -1};

/* Main arguments */
static gchar *adapter_arg = NULL;
static gboolean list_arg = FALSE;
static gchar *connect_arg = NULL;
static gchar *disconnect_arg = NULL;
static gchar *remove_arg = NULL;
static gchar *info_arg = NULL;
static gboolean services_arg = FALSE;
static gchar *services_device_arg = NULL;
static gchar *services_pattern_arg = NULL;
static gboolean set_arg = FALSE;
static gchar *set_device_arg = NULL;
static gchar *set_property_arg = NULL;
static gchar *set_value_arg = NULL;
static gboolean verbose_arg = FALSE;

static gboolean is_verbose_attr(int attr_id)
{
    if (
            attr_id == SDP_ATTR_ID_SERVICE_CLASS_ID_LIST ||
            attr_id == SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST ||
            attr_id == SDP_ATTR_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST ||
            attr_id == SDP_ATTR_ID_DOCUMENTATION_URL ||
            attr_id == SDP_ATTR_ID_SERVICE_NAME ||
            attr_id == SDP_ATTR_ID_SERVICE_DESCRIPTION ||
            attr_id == SDP_ATTR_ID_PROVIDER_NAME ||
            attr_id == SDP_ATTR_ID_SECURITY_DESCRIPTION
            )
        return FALSE;

    return TRUE;
}

static const gchar *xml_get_attr_value(const gchar *attr_name, const gchar **attribute_names, const gchar **attribute_values)
{
    for (int i = 0; attribute_names[i] != NULL; i++)
    {
        if (g_strcmp0(attribute_names[i], attr_name) == 0)
        {
            return attribute_values[i];
        }
    }

    return NULL;
}

static void xml_start_element(GMarkupParseContext *context,
                              const gchar *element_name,
                              const gchar **attribute_names,
                              const gchar **attribute_values,
                              gpointer user_data,
                              GError **error)
{
    const gchar *id_t = xml_get_attr_value("id", attribute_names, attribute_values);
    const gchar *value_t = xml_get_attr_value("value", attribute_names, attribute_values);

    if (g_strcmp0(element_name, "record") == 0)
    {
        xml_t[REC]++;
    }
    else if (g_strcmp0(element_name, "attribute") == 0 && id_t)
    {
        int attr_id = xtoi(id_t);
        const gchar *attr_name = sdp_get_attr_id_name(attr_id);

        xml_t[ATTR]++;
        xml_t[ATTR_ID] = attr_id;

        if (!verbose_arg && is_verbose_attr(xml_t[ATTR_ID])) return;

        if (attr_name == NULL)
        {
            g_print("AttrID-%s: ", id_t);
        }
        else
        {
            g_print("%s: ", attr_name);
        }
    }
    else if (g_strcmp0(element_name, "sequence") == 0)
    {
        xml_t[SEQ]++;
    }
    else if (g_pattern_match_simple("*int*", element_name) && value_t)
    {
        xml_t[ELEM]++;

        if (!verbose_arg && is_verbose_attr(xml_t[ATTR_ID])) return;

        if (xml_t[ELEM] == 1 && xml_t[SEQ] > 1)
        {
            g_print("\n");
            for (int i = 0; i < xml_t[SEQ]; i++) g_print("  ");
        }
        else if (xml_t[ELEM] > 1)
        {
            g_print(", ");
        }

        if (xml_t[UUID_ID] == SDP_UUID_RFCOMM)
        {
            g_print("Channel: %d", xtoi(value_t));
        }
        else
        {
            g_print("0x%x", xtoi(value_t));
        }
    }
    else if (g_strcmp0(element_name, "uuid") == 0 && value_t)
    {
        int uuid_id = -1;
        const gchar *uuid_name;

        if (value_t[0] == '0' && value_t[1] == 'x')
        {
            uuid_id = xtoi(value_t);
            uuid_name = sdp_get_uuid_name(uuid_id);
        }
        else
        {
            uuid_name = uuid2name(value_t);
        }

        xml_t[ELEM]++;
        xml_t[UUID_ID] = uuid_id;

        if (!verbose_arg && is_verbose_attr(xml_t[ATTR_ID])) return;

        if (xml_t[ELEM] == 1 && xml_t[SEQ] > 1)
        {
            g_print("\n");
            for (int i = 0; i < xml_t[SEQ]; i++) g_print("  ");
        }
        else if (xml_t[ELEM] > 1)
        {
            g_print(", ");
        }

        if (uuid_name == NULL)
        {
            g_print("\"UUID-%s\"", value_t);
        }
        else
        {
            g_print("\"%s\"", uuid_name);
        }
    }
    else if (g_strcmp0(element_name, "text") == 0 && value_t)
    {
        xml_t[ELEM]++;

        if (!verbose_arg && is_verbose_attr(xml_t[ATTR_ID])) return;

        if (xml_t[ELEM] == 1 && xml_t[SEQ] > 1)
        {
            g_print("\n");
            for (int i = 0; i < xml_t[SEQ]; i++) g_print("  ");
        }
        else if (xml_t[ELEM] > 1)
        {
            g_print(", ");
        }

        g_print("\"%s\"", value_t);
    }
    else if (g_strcmp0(element_name, "boolean") == 0 && value_t)
    {
        xml_t[ELEM]++;

        if (!verbose_arg && is_verbose_attr(xml_t[ATTR_ID])) return;

        if (xml_t[ELEM] == 1 && xml_t[SEQ] > 1)
        {
            g_print("\n");
            for (int i = 0; i < xml_t[SEQ]; i++) g_print("  ");
        }
        else if (xml_t[ELEM] > 1)
        {
            g_print(", ");
        }

        g_print("%s", value_t);
    }
    else
    {
        if (error)
            *error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT, "Invalid XML element: %s", element_name);
    }
}

static void xml_end_element(GMarkupParseContext *context,
                            const gchar *element_name,
                            gpointer user_data,
                            GError **error)
{
    if (g_strcmp0(element_name, "record") == 0)
    {
        xml_t[ATTR] = 0;
        xml_t[SEQ] = 0;
        xml_t[ELEM] = 0;

        xml_t[ATTR_ID] = -1;
        xml_t[UUID_ID] = -1;
    }
    else if (g_strcmp0(element_name, "attribute") == 0)
    {
        xml_t[SEQ] = 0;
        xml_t[ELEM] = 0;

        int old_attr_id = xml_t[ATTR_ID];
        xml_t[ATTR_ID] = -1;
        xml_t[UUID_ID] = -1;

        if (!verbose_arg && is_verbose_attr(old_attr_id)) return;

        g_print("\n");
    }
    else if (g_strcmp0(element_name, "sequence") == 0)
    {
        xml_t[SEQ]--;
        xml_t[ELEM] = 0;

        xml_t[UUID_ID] = -1;
    }
}

static void create_paired_device_done(gpointer data)
{
    g_assert(data != NULL);
    GMainLoop *mainloop = data;
    g_main_loop_quit(mainloop);
}

static void _bt_device_pair_callback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    g_assert(user_data != NULL);
    GHashTable *dict = (GHashTable *) user_data;
    Device *device = (Device *) g_hash_table_lookup(dict, "device");
    GError *error = NULL;
    device_pair_finish(device, res, &error);
    exit_if_error(error);
    GMainLoop *mainloop = (GMainLoop *) g_hash_table_lookup(dict, "mainloop");
    g_main_loop_quit(mainloop);
}



static GHashTable *_bt_device_sdp_browse(const gchar *device_path, const gchar *pattern)
{
    int pipefd[2];
    pipe(pipefd);

    // Fork the process into a child process to make an exec call
    int pid = fork();

    // Child (to call sdptool)
    if (pid == 0)
    {
        // close reading end in the child
        close(pipefd[0]);    

        // send stdout to the pipe
        dup2(pipefd[1], 1);
        // send stderr to the pipe
        dup2(pipefd[1], 2);

        // this descriptor is no longer needed
        close(pipefd[1]);

        if(!g_file_test ("/bin/sdptool", G_FILE_TEST_EXISTS))
        {
            write(2, "sdptool not found\n", sizeof("sdptool not found\n"));
            exit(EXIT_FAILURE);
        }
        
        if(pattern == NULL || strlen(pattern) == 0)
            execl("/bin/sdptool", "/bin/sdptool", "browse", "--xml", device_path, (char *) 0);
        else
            execl("/bin/sdptool", "/bin/sdptool", "browse", "--xml", "--uuid", pattern, device_path, (char *) 0);
        
    }
    if(pid == -1)
    {
        perror("forking process failed");
        exit(EXIT_FAILURE);
    }

    close(pipefd[1]);  // close the write end of the pipe in the parent
    GInputStream *exec_output = (GInputStream *) g_unix_input_stream_new(pipefd[0], TRUE);
    GDataInputStream *exec_data_input_stream = g_data_input_stream_new(exec_output);
    GPtrArray *array = g_ptr_array_new();
    guint record_counter = 0;
    guint n = 0;
    GError *error = NULL;
    GString *string_buffer = g_string_new("");
    while(TRUE)
    {
        n++;
        gchar *line = g_data_input_stream_read_line_utf8(exec_data_input_stream, NULL, NULL, &error);
        exit_if_error(error);
        // If there is no content, then break out of the loop
        if(!line)
            break;
        
        if(n == 1)
        {
            if(g_regex_match_simple("Failed", line, 0, 0) || g_regex_match_simple("Error", line, 0, 0))
            {
                g_print("%s\n", line);
                exit(EXIT_FAILURE);
            }
            else if(g_regex_match_simple("not found", line, 0, 0))
            {
                g_print("Failed to start SDP discovery. Please make sure you have bluez-utils installed on your system.\n");
                exit(EXIT_FAILURE);
            }
            
            continue;
        }
        
        if(g_regex_match_simple("<\\?xml(.*)\\?>", line, 0, 0))
        {
            if(record_counter != 0)
            {
                g_ptr_array_add(array, g_string_free(string_buffer, FALSE));
                string_buffer = g_string_new("");
            }
            record_counter++;
        }
        
        g_string_append(string_buffer, line);
    }
    
    g_ptr_array_add(array, g_string_free(string_buffer, FALSE));
    
    GHashTable *sdp_hash_table = g_hash_table_new(g_int_hash, g_int_equal);
    
    int i = 0;
    for(i = 0; i < array->len; i++)
    {
        GRegex *record_id_regex = g_regex_new("<attribute id=\"0x0000\">(\\s*<uint32 value=\"(.*)\" />\\s)</attribute>", G_REGEX_CASELESS | G_REGEX_MULTILINE | G_REGEX_NEWLINE_ANYCRLF, 0, &error);
        exit_if_error(error);
        GMatchInfo *match_info;
        g_regex_match (record_id_regex, (gchar *) g_ptr_array_index(array, i), 0, &match_info);
        if(g_match_info_matches(match_info))
        {
            gchar *word = g_match_info_fetch(match_info, 2);
            guint32 *key = g_new(guint32, 1);
            *key = 0;
            sscanf(word, "0x%x", key);
            g_hash_table_insert(sdp_hash_table, key, (gchar *) g_ptr_array_index(array, i));
            g_free(word);
        }
        g_match_info_free(match_info);
        g_regex_unref(record_id_regex);
    }
    
    g_input_stream_close(exec_output, NULL, &error);
    exit_if_error(error);
    g_object_unref(exec_data_input_stream);
    g_object_unref(exec_output);
    
    g_ptr_array_unref(array);
    
    return sdp_hash_table;
}

static GOptionEntry entries[] = {
    {"adapter", 'a', 0, G_OPTION_ARG_STRING, &adapter_arg, "Adapter Name or MAC", "<name|mac>"},
    {"list", 'l', 0, G_OPTION_ARG_NONE, &list_arg, "List added devices", NULL},
    {"connect", 'c', 0, G_OPTION_ARG_STRING, &connect_arg, "Connect to the remote device", "<mac>"},
    {"disconnect", 'd', 0, G_OPTION_ARG_STRING, &disconnect_arg, "Disconnect the remote device", "<name|mac>"},
    {"remove", 'r', 0, G_OPTION_ARG_STRING, &remove_arg, "Remove device", "<name|mac>"},
    {"info", 'i', 0, G_OPTION_ARG_STRING, &info_arg, "Get info about device", "<name|mac>"},
    {"services", 's', 0, G_OPTION_ARG_NONE, &services_arg, "Discover device services", NULL},
    {"set", 0, 0, G_OPTION_ARG_NONE, &set_arg, "Set device property", NULL},
    {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose_arg, "Verbosely display remote service records", NULL},
    {NULL}
};

int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;

    /* Query current locale */
    setlocale(LC_CTYPE, "");

    /* Deprecated */
    // g_type_init();
    dbus_init();

    context = g_option_context_new("- a bluetooth device manager");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_set_summary(context, "Version "PACKAGE_VERSION);
    g_option_context_set_description(context,
                                     "Services Options:\n"
                                     "  -s, --services <name|mac> [<pattern>]\n"
                                     "  Where `pattern` is an optional specific UUID to search\n\n"
                                     "Set Options:\n"
                                     "  --set <name|mac> <property> <value>\n"
                                     "  Where\n"
                                     "    `name|mac` is a device Name or MAC\n"
                                     "    `property` is one of:\n"
                                     "       Alias\n"
                                     "       Trusted\n"
                                     "       Blocked\n\n"
                                     "Report bugs to <"PACKAGE_BUGREPORT">."
                                     "Project home page <"PACKAGE_URL">."
                                     );

    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        g_print("%s: %s\n", g_get_prgname(), error->message);
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }
    else if (!list_arg && (!connect_arg || strlen(connect_arg) == 0) && (!disconnect_arg || strlen(disconnect_arg) == 0) && (!remove_arg || strlen(remove_arg) == 0) && (!info_arg || strlen(info_arg) == 0) && !services_arg && !set_arg)
    {
        g_print("%s", g_option_context_get_help(context, FALSE, NULL));
        exit(EXIT_FAILURE);
    }
    else if (services_arg && (argc != 2 || strlen(argv[1]) == 0) && (argc != 3 || strlen(argv[1]) == 0))
    {
        g_print("%s: Invalid arguments for --services\n", g_get_prgname());
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }
    else if (set_arg && (argc != 4 || strlen(argv[1]) == 0 || strlen(argv[2]) == 0 || strlen(argv[3]) == 0))
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

    Manager *manager = manager_new();
    Adapter *adapter = find_adapter(adapter_arg, &error);
    exit_if_error(error);

    if (list_arg)
    {
        const gchar **devices_list = manager_get_devices(manager, adapter_get_dbus_object_path(adapter));

        if (devices_list == NULL)
        {
            g_print("No devices found\n");
            exit(EXIT_FAILURE);
        }

        g_print("Added devices:\n");
        const gchar **devices = NULL;
        for (devices = devices_list; *devices != NULL; devices++)
        {
            const gchar *device_path = *devices;
            Device *device = device_new(device_path);
            g_print("%s (%s)\n", device_get_alias(device, &error), device_get_address(device, &error));
            exit_if_error(error);
            g_object_unref(device);
        }
    }
    else if (connect_arg)
    {
        g_print("Connecting to: %s\n", connect_arg);
        GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);

        AgentManager *agent_manager = agent_manager_new();
        
        agent_need_unregister = TRUE;
        
        register_agent_callbacks(TRUE, NULL, mainloop, &error);
        exit_if_error(error);
        
        agent_manager_register_agent(agent_manager, AGENT_PATH, "DisplayYesNo", &error);
        exit_if_error(error);
        
        Device *device = find_device(adapter, connect_arg, &error);
        exit_if_error(error);
        if(!device)
        {
            g_printerr("Error: Device not found.\n");
            exit(EXIT_FAILURE);
        }
        
        GHashTable *user_data_hash = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(user_data_hash, "device", device);
        g_hash_table_insert(user_data_hash, "mainloop", mainloop);
        
        device_pair_async(device, (GAsyncReadyCallback) _bt_device_pair_callback, (gpointer) user_data_hash);
        g_main_loop_run(mainloop);
        
        g_print("Done\n");
        g_hash_table_unref(user_data_hash);
        g_main_loop_unref(mainloop);
        g_object_unref(device);
        unregister_agent_callbacks(&error);
    }
    else if (disconnect_arg)
    {
        Device *device = find_device(adapter, disconnect_arg, &error);
        exit_if_error(error);
        if(!device)
        {
            g_printerr("Error: Device not found.\n");
            exit(EXIT_FAILURE);
        }

        g_print("Disconnecting: %s\n", disconnect_arg);
        device_disconnect(device, &error);
        exit_if_error(error);

        g_print("Done\n");
        g_object_unref(device);
    }
    else if (remove_arg)
    {
        Device *device = find_device(adapter, remove_arg, &error);
        exit_if_error(error);
        if(!device)
        {
            g_printerr("Error: Device not found.\n");
            exit(EXIT_FAILURE);
        }
        
        adapter_remove_device(adapter, device_get_dbus_object_path(device), &error);
        exit_if_error(error);

        g_print("Done\n");
        g_object_unref(device);
    }
    else if (info_arg)
    {
        Device *device = find_device(adapter, info_arg, &error);
        exit_if_error(error);
        if(!device)
        {
            g_printerr("Error: Device not found.\n");
            exit(EXIT_FAILURE);
        }

        g_print("[%s]\n", device_get_address(device, NULL));
        g_print("  Name: %s\n", device_get_name(device, NULL));
        g_print("  Alias: %s [rw]\n", device_get_alias(device, NULL));
        g_print("  Address: %s\n", device_get_address(device, NULL));
        g_print("  Icon: %s\n", device_get_icon(device, NULL));
        g_print("  Class: 0x%x\n", device_get_class(device, NULL));
        g_print("  Paired: %d\n", device_get_paired(device, NULL));
        g_print("  Trusted: %d [rw]\n", device_get_trusted(device, NULL));
        g_print("  Blocked: %d [rw]\n", device_get_blocked(device, NULL));
        g_print("  Connected: %d\n", device_get_connected(device, NULL));
        g_print("  UUIDs: [");
        const gchar **uuids = device_get_uuids(device, NULL);
        for (int j = 0; uuids[j] != NULL; j++)
        {
            if (j > 0) g_print(", ");
            g_print("%s", uuid2name(uuids[j]));
        }
        g_print("]\n");

        g_object_unref(device);
    }
    else if (services_arg)
    {
        services_device_arg = argv[1];
        if (argc == 3)
        {
            services_pattern_arg = argv[2];
        }

        Device *device = find_device(adapter, services_device_arg, &error);
        exit_if_error(error);
        if(!device)
        {
            g_printerr("Error: Device not found.\n");
            exit(EXIT_FAILURE);
        }
        
        const gchar *device_address = device_get_address(device, &error);
        exit_if_error(error);

        g_print("Discovering services...\n");
        // Because BlueZ 5 removed the API call for discover services, we will use sdptool as an alternative
        // GHashTable *device_services = device_discover_services(device, name2uuid(services_pattern_arg), &error);
        GHashTable *device_services = _bt_device_sdp_browse(device_address, name2uuid(services_pattern_arg));
        exit_if_error(error);

        
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, device_services);
        int n = 0;
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            n++;
            if (n == 1) g_print("\n");
            // g_print("[RECORD:%d]\n", (gint) key);
            g_print("[RECORD:0x%x]\n", *(guint32 *) key);
            GMarkupParser xml_parser = {xml_start_element, xml_end_element, NULL, NULL, NULL};
            GMarkupParseContext *xml_parse_context = g_markup_parse_context_new(&xml_parser, 0, NULL, NULL);
            g_markup_parse_context_parse(xml_parse_context, value, strlen(value), &error);
            exit_if_error(error);
            g_markup_parse_context_free(xml_parse_context);
            g_print("\n");
        }

        g_print("Done\n");
        g_hash_table_unref(device_services);
        g_object_unref(device);
    }
    else if (set_arg)
    {
        set_device_arg = argv[1];
        set_property_arg = argv[2];
        set_value_arg = argv[3];

        Device *device = find_device(adapter, set_device_arg, &error);
        exit_if_error(error);
        if(!device)
        {
            g_printerr("Error: Device not found.\n");
            exit(EXIT_FAILURE);
        }

        GVariant *v = NULL;

        if (g_strcmp0(set_property_arg, "Alias") == 0)
        {
            v = g_variant_new_string(set_value_arg);
        }
        else if (
                g_strcmp0(set_property_arg, "Trusted") == 0 ||
                g_strcmp0(set_property_arg, "Blocked") == 0
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
        else
        {
            g_print("%s: Invalid property: %s\n", g_get_prgname(), set_property_arg);
            g_print("Try `%s --help` for more information.\n", g_get_prgname());
            exit(EXIT_FAILURE);
        }

        GVariant *props = device_get_properties(device, &error);
        exit_if_error(error);
        
        if(g_ascii_strcasecmp(set_property_arg, "Alias") == 0)
            device_set_alias(device, g_variant_get_string(v, NULL), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "Blocked") == 0)
            device_set_blocked(device, g_variant_get_boolean(v), &error);
        else if(g_ascii_strcasecmp(set_property_arg, "Trusted") == 0)
            device_set_trusted(device, g_variant_get_boolean(v), &error);
        
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
        g_variant_unref(props);

        // g_variant_unref(v); /* Floating references do not need to be unref */
        g_object_unref(device);
    }

    g_object_unref(adapter);
    dbus_disconnect();

    exit(EXIT_SUCCESS);
}
