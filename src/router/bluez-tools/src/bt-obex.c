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

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <gio/gio.h>
#include <glib/gstdio.h>

#include "lib/dbus-common.h"
#include "lib/helpers.h"
#include "lib/bluez-api.h"

static GHashTable *_transfers = NULL;
static GHashTable *_transfer_infos = NULL;
static GMainLoop *mainloop = NULL;
static gchar *_root_path = NULL;
static gboolean _update_progress = FALSE;

typedef struct _ObexTransferInfo ObexTransferInfo;

struct _ObexTransferInfo {
    gchar *filename;
    guint64 filesize;
    gchar *obex_root;
    gchar *status;
};

static void sigterm_handler(int sig)
{
    g_message("%s received", sig == SIGTERM ? "SIGTERM" : "SIGINT");

    if (g_main_loop_is_running(mainloop))
        g_main_loop_quit(mainloop);
}

static void _obex_server_object_manager_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    if(g_strcmp0(signal_name, "InterfacesAdded") == 0)
    {
        const gchar *interface_object_path = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
        GVariant *interfaces_and_properties = g_variant_get_child_value(parameters, 1);
        GVariant *properties = NULL;
        
        if(g_variant_lookup(interfaces_and_properties, OBEX_TRANSFER_DBUS_INTERFACE, "@a{sv}", &properties))
        {
            g_print("[OBEX Server] Transfer started\n");
            ObexTransfer *t = obex_transfer_new(interface_object_path);
            g_hash_table_insert(_transfers, g_strdup(interface_object_path), t);
            
            ObexTransferInfo *info = g_malloc0(sizeof(ObexTransferInfo));
            info->filesize = g_variant_get_uint64(g_variant_lookup_value(properties, "Size", NULL));
            info->status = g_strdup(g_variant_get_string(g_variant_lookup_value(properties, "Status", NULL), NULL));
            ObexSession *session = obex_session_new(g_variant_get_string(g_variant_lookup_value(properties, "Session", NULL), NULL));
            
            info->obex_root = g_strdup(obex_session_get_root(session, NULL));
            
            g_object_unref(session);
            
            g_hash_table_insert(_transfer_infos, g_strdup(interface_object_path), info);
        }
        
        if(g_variant_lookup(interfaces_and_properties, OBEX_SESSION_DBUS_INTERFACE, "@a{sv}", &properties))
        {
            g_print("[OBEX Server] OBEX session opened\n");
        }
        
        g_variant_unref(interfaces_and_properties);
        if(properties)
            g_variant_unref(properties);
    }
    else if(g_strcmp0(signal_name, "InterfacesRemoved") == 0)
    {
        const gchar *interface_object_path = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
        GVariant *interfaces = g_variant_get_child_value(parameters, 1);
        const gchar **inf_array = g_variant_get_strv(interfaces, NULL);
        g_variant_unref(interfaces);
        const gchar **inf = NULL;
        for(inf = inf_array; *inf != NULL; inf++)
        {
            if(g_strcmp0(*inf, OBEX_TRANSFER_DBUS_INTERFACE) == 0)
            {
                g_print("[OBEX Server] OBEX transfer closed\n");
                ObexTransfer *transfer = g_hash_table_lookup(_transfers, interface_object_path);
                g_hash_table_remove(_transfers, interface_object_path);
                g_object_unref(transfer);
                g_free(g_hash_table_lookup(_transfer_infos, interface_object_path));
                g_hash_table_remove(_transfer_infos, interface_object_path);
            }
            
            if(g_strcmp0(*inf, OBEX_SESSION_DBUS_INTERFACE) == 0)
            {
                g_print("[OBEX Server] OBEX session closed\n");
            }
        }
        g_free(inf_array);
    }
}

static void _obex_server_properties_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    const gchar *arg0 = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
    GVariant *changed_properties = g_variant_get_child_value(parameters, 1);
    
    if(g_strcmp0(arg0, OBEX_TRANSFER_DBUS_INTERFACE) == 0)
    {
        ObexTransfer *transfer = g_hash_table_lookup(_transfers, object_path);
        
        guint64 size = 0x0;
        guint64 transferred = 0x0;
        obex_transfer_get_size(transfer, NULL);
        g_variant_lookup(changed_properties, "Size", "t", &size);
        if(!size)
            size = obex_transfer_get_size(transfer, NULL);
        g_variant_lookup(changed_properties, "Transferred", "t", &transferred);
        
        if(size && transferred)
        {
            guint pp = (transferred / (gfloat) size)*100;

            if (!_update_progress)
            {
                g_print("[OBEXTransfer] Progress: %3u%%", pp);
                _update_progress = TRUE;
            }
            else
            {
                g_print("\b\b\b\b%3u%%", pp);
            }

            if (pp == 100)
            {
                g_print("\n");
                _update_progress = FALSE;
            }
        }
        
        gchar *status = NULL;
        g_variant_lookup(changed_properties, "Status", "s", &status);
        
        if(status)
        {
            if(g_strcmp0(status, "active") == 0)
            {
                // g_print("[OBEX Server] Transfer active\n");
            }
            else if(g_strcmp0(status, "complete") == 0)
            {
                g_print("[OBEX Server] Transfer succeeded\n");
                ObexTransferInfo *info = g_hash_table_lookup(_transfer_infos, object_path);
                g_rename(g_build_filename(info->obex_root, info->filename, NULL), g_build_filename(_root_path, info->filename, NULL));
            }
            else if(g_strcmp0(status, "error") == 0)
            {
                g_print("[OBEX Server] Transfer failed\n");
            }
            else if(g_strcmp0(status, "queued") == 0)
            {
                g_print("[OBEX Server] Transfer queued\n");
            }
            else if(g_strcmp0(status, "suspended") == 0)
            {
                g_print("[OBEX Server] Transfer halted\n");
            }
            g_free(status);
        }
    }
    
    g_variant_unref(changed_properties);
}

static void _obex_opp_client_object_manager_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    if(g_strcmp0(signal_name, "InterfacesAdded") == 0)
    {
        const gchar *interface_object_path = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
        GVariant *interfaces_and_properties = g_variant_get_child_value(parameters, 1);
        GVariant *properties = NULL;
        
        if(g_variant_lookup(interfaces_and_properties, OBEX_TRANSFER_DBUS_INTERFACE, "@a{sv}", &properties))
        {
            // g_print("[OBEX Client] Transfer started\n");
            ObexTransfer *t = obex_transfer_new(interface_object_path);
            g_hash_table_insert(_transfers, g_strdup(interface_object_path), t);

            ObexTransferInfo *info = g_malloc0(sizeof(ObexTransferInfo));
            info->filesize = g_variant_get_uint64(g_variant_lookup_value(properties, "Size", NULL));
            info->filename = g_strdup(g_variant_get_string(g_variant_lookup_value(properties, "Name", NULL), NULL));
            info->status = g_strdup(g_variant_get_string(g_variant_lookup_value(properties, "Status", NULL), NULL));
            ObexSession *session = obex_session_new(g_variant_get_string(g_variant_lookup_value(properties, "Session", NULL), NULL));
            
            info->obex_root = g_strdup(obex_session_get_root(session, NULL));
            
            g_object_unref(session);
            
            g_hash_table_insert(_transfer_infos, g_strdup(interface_object_path), info);
            if(g_strcmp0(info->status, "queued") == 0)
                g_print("[Transfer#%s] Waiting...\n", info->filename);
        }
        
        if(g_variant_lookup(interfaces_and_properties, OBEX_SESSION_DBUS_INTERFACE, "@a{sv}", &properties))
        {
            // g_print("[OBEX Client] OBEX session opened\n");
        }
        
        g_variant_unref(interfaces_and_properties);
        if(properties)
            g_variant_unref(properties);
    }
    else if(g_strcmp0(signal_name, "InterfacesRemoved") == 0)
    {
        const gchar *interface_object_path = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
        GVariant *interfaces = g_variant_get_child_value(parameters, 1);
        const gchar **inf_array = g_variant_get_strv(interfaces, NULL);
        g_variant_unref(interfaces);
        const gchar **inf = NULL;
        for(inf = inf_array; *inf != NULL; inf++)
        {
            if(g_strcmp0(*inf, OBEX_TRANSFER_DBUS_INTERFACE) == 0)
            {
                // g_print("[OBEX Client] OBEX transfer closed\n");
                ObexTransfer *transfer = g_hash_table_lookup(_transfers, interface_object_path);
                g_hash_table_remove(_transfers, interface_object_path);
                g_object_unref(transfer);
                g_free(g_hash_table_lookup(_transfer_infos, interface_object_path));
                g_hash_table_remove(_transfer_infos, interface_object_path);
                if (g_main_loop_is_running(mainloop))
                    g_main_loop_quit(mainloop);
            }
            
            if(g_strcmp0(*inf, OBEX_SESSION_DBUS_INTERFACE) == 0)
            {
                // g_print("[OBEX Client] OBEX session closed\n");
            }
        }
        g_free(inf_array);
    }
}

static void _obex_opp_client_properties_handler(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name, const gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    const gchar *arg0 = g_variant_get_string(g_variant_get_child_value(parameters, 0), NULL);
    GVariant *changed_properties = g_variant_get_child_value(parameters, 1);
    
    if(g_strcmp0(arg0, OBEX_TRANSFER_DBUS_INTERFACE) == 0)
    {
        ObexTransfer *transfer = g_hash_table_lookup(_transfers, object_path);
        ObexTransferInfo *info = g_hash_table_lookup(_transfer_infos, object_path);
        
        guint64 size = 0x0;
        guint64 transferred = 0x0;
        obex_transfer_get_size(transfer, NULL);
        g_variant_lookup(changed_properties, "Size", "t", &size);
        if(!size)
            size = obex_transfer_get_size(transfer, NULL);
        g_variant_lookup(changed_properties, "Transferred", "t", &transferred);
        
        if(size && transferred && g_strcmp0(info->status, "active") == 0)
        {
            guint pp = (transferred / (gfloat) size)*100;

            if (!_update_progress)
            {
                g_print("[Transfer#%s] Progress: %3u%%", obex_transfer_get_name(transfer, NULL), pp);
                _update_progress = TRUE;
            }
            else
            {
                g_print("\b\b\b\b%3u%%", pp);
            }

            if (pp == 100)
            {
                g_print("\n");
                _update_progress = FALSE;
            }
        }
        
        gchar *status = NULL;
        g_variant_lookup(changed_properties, "Status", "s", &status);
        
        if(status)
        {
            g_free(info->status);
            info->status = g_strdup(status);
            
            if(g_strcmp0(status, "active") == 0)
            {
                // g_print("[Client Server] Transfer active\n");
            }
            else if(g_strcmp0(status, "complete") == 0)
            {
                if(_update_progress)
                {
                    _update_progress = FALSE;
                    g_print("\b\b\b\b%3u%%", 100);
                    g_print("\n");
                }
                    
                g_print("[Transfer#%s] Completed\n", info->filename);
            }
            else if(g_strcmp0(status, "error") == 0)
            {
                if(_update_progress)
                {
                    _update_progress = FALSE;
                    g_print("\n");
                }

                g_print("[Transfer#%s] Failed\n", info->filename);
            }
            else if(g_strcmp0(status, "queued") == 0)
            {
                // g_print("[OBEX Client] Transfer queued\n");
            }
            else if(g_strcmp0(status, "suspended") == 0)
            {
                if(_update_progress)
                {
                    _update_progress = FALSE;
                    g_print("\n");
                }
                    
                g_print("[Transfer#%s] Suspended\n", info->filename);
            }
            g_free(status);
        }
    }
    
    g_variant_unref(changed_properties);
}

void _agent_approved_callback(ObexAgent *obex_agent, const gchar* obex_transfer_path, const gchar *name, const guint64 size, gpointer user_data)
{
    ObexTransferInfo *info = g_hash_table_lookup(_transfer_infos, obex_transfer_path);
    if(!info)
    {
        info = g_malloc0(sizeof(ObexTransferInfo));
        g_hash_table_insert(_transfer_infos, g_strdup(obex_transfer_path), info);
        ObexTransfer *transfer = g_hash_table_lookup(_transfers, obex_transfer_path);
        ObexSession *session = obex_session_new(obex_transfer_get_session(transfer, NULL));
        info->obex_root = g_strdup(obex_session_get_root(session, NULL));
    }
    info->filename = g_strdup(name);
    info->filesize = size;
}

/* Main arguments */
static gchar *adapter_arg = NULL;
static gboolean server_arg = FALSE;
static gboolean auto_accept = FALSE;
static gchar *server_path_arg = NULL;
static gboolean opp_arg = FALSE;
static gchar *opp_device_arg = NULL;
static gchar *opp_file_arg = NULL;
static gchar *ftp_arg = NULL;

static GOptionEntry entries[] = {
    {"adapter", 'a', 0, G_OPTION_ARG_STRING, &adapter_arg, "Adapter name or MAC", "<name|mac>"},
    {"server", 's', 0, G_OPTION_ARG_NONE, &server_arg, "Register self as OBEX server", NULL},
    {"auto-accept", 'y', 0, G_OPTION_ARG_NONE, &auto_accept, "Automatically accept incoming files", NULL},
    {"opp", 'p', 0, G_OPTION_ARG_NONE, &opp_arg, "Send file to remote device", NULL},
    {"ftp", 'f', 0, G_OPTION_ARG_STRING, &ftp_arg, "Start FTP session with remote device", "<name|mac>"},
    {NULL}
};

int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;

    /* Query current locale */
    setlocale(LC_CTYPE, "");

    dbus_init();

    context = g_option_context_new(" - a bluetooth OBEX client/server");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_set_summary(context, "Version "PACKAGE_VERSION);
    g_option_context_set_description(context,
                                     "Server Options:\n"
                                     "  -s, --server [<path>]\n"
                                     "  Register self at OBEX server and use given `path` as OPP save directory\n"
                                     "  If `path` does not specified - use current directory\n\n"
                                     "OPP Options:\n"
                                     "  -p, --opp <name|mac> <file>\n"
                                     "  Send `file` to remote device using Object Push Profile\n\n"
                                     "Report bugs to <"PACKAGE_BUGREPORT">."
                                     "Project home page <"PACKAGE_URL">."
                                     );

    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        g_print("%s: %s\n", g_get_prgname(), error->message);
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }
    else if (!server_arg && !opp_arg && (!ftp_arg || strlen(ftp_arg) == 0))
    {
        g_print("%s", g_option_context_get_help(context, FALSE, NULL));
        exit(EXIT_FAILURE);
    }
    else if (server_arg && argc != 1 && (argc != 2 || strlen(argv[1]) == 0))
    {
        g_print("%s: Invalid arguments for --server\n", g_get_prgname());
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }
    else if (opp_arg && (argc != 3 || strlen(argv[1]) == 0 || strlen(argv[2]) == 0))
    {
        g_print("%s: Invalid arguments for --opp\n", g_get_prgname());
        g_print("Try `%s --help` for more information.\n", g_get_prgname());
        exit(EXIT_FAILURE);
    }

    g_option_context_free(context);

    if (!dbus_system_connect(&error))
    {
        g_printerr("Couldn't connect to DBus system bus: %s\n", error->message);
        exit(EXIT_FAILURE);
    }

    if (!dbus_session_connect(&error))
    {
        g_printerr("Couldn't connect to DBus session bus: %s\n", error->message);
        exit(EXIT_FAILURE);
    }

    /* Check, that bluetooth daemon is running */
    if (!intf_supported(BLUEZ_DBUS_SERVICE_NAME, MANAGER_DBUS_PATH, MANAGER_DBUS_INTERFACE))
    {
        g_printerr("%s: bluez service is not found\n", g_get_prgname());
        g_printerr("Did you forget to run bluetoothd?\n");
        exit(EXIT_FAILURE);
    }

    /* Check, that obexd daemon is running */
    if (!intf_supported(BLUEZ_OBEX_DBUS_SERVICE_NAME, MANAGER_DBUS_PATH, MANAGER_DBUS_INTERFACE))
    {
        g_printerr("%s: obex service is not found\n", g_get_prgname());
        g_printerr("Did you forget to run obexd?\n");
        exit(EXIT_FAILURE);
    }

    if (server_arg)
    {
        if (argc == 2)
        {
            server_path_arg = argv[1];
        }

        /* Check that `path` is valid */
        gchar *root_folder = server_path_arg == NULL ? g_get_current_dir() : g_strdup(server_path_arg);
        if (!is_dir(root_folder, &error))
        {
            exit_if_error(error);
        }
        if(!write_access(root_folder, &error))
        {
            exit_if_error(error);
        }

        _transfers = g_hash_table_new(g_str_hash, g_str_equal);
        _transfer_infos = g_hash_table_new(g_str_hash, g_str_equal);

        ObexAgentManager *manager = obex_agent_manager_new();
        
        guint obex_server_object_id = g_dbus_connection_signal_subscribe(session_conn, "org.bluez.obex", "org.freedesktop.DBus.ObjectManager", NULL, NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _obex_server_object_manager_handler, NULL, NULL);
        guint obex_server_properties_id = g_dbus_connection_signal_subscribe(session_conn, "org.bluez.obex", "org.freedesktop.DBus.Properties", "PropertiesChanged", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _obex_server_properties_handler, NULL, NULL);
        
        ObexAgent *agent = obex_agent_new(root_folder, auto_accept);
        _root_path = g_strdup(root_folder);
        g_free(root_folder);
        obex_agent_set_approved_callback(agent, _agent_approved_callback, NULL);

        obex_agent_manager_register_agent(manager, OBEX_AGENT_DBUS_PATH, &error);
        exit_if_error(error);

        mainloop = g_main_loop_new(NULL, FALSE);

        /* Add SIGTERM && SIGINT handlers */
        struct sigaction sa;
        memset(&sa, 0, sizeof (sa));
        sa.sa_handler = sigterm_handler;
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGINT, &sa, NULL);

        g_main_loop_run(mainloop);

        /* Waiting for connections... */

        g_main_loop_unref(mainloop);

        /* Stop active transfers */
        GHashTableIter iter;
        gpointer key, value;
        
        g_hash_table_iter_init(&iter, _transfers);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            ObexTransfer *t = OBEX_TRANSFER(value);
            obex_transfer_cancel(t, NULL); // skip errors
            g_object_unref(t);
            g_hash_table_iter_remove(&iter);
        }
        g_hash_table_unref(_transfers);
        
        // Remove transfer information
        g_hash_table_iter_init(&iter, _transfer_infos);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            g_free(value);
            g_hash_table_iter_remove(&iter);
        }
        g_hash_table_unref(_transfers);

        g_dbus_connection_signal_unsubscribe(session_conn, obex_server_object_id);
        g_dbus_connection_signal_unsubscribe(session_conn, obex_server_properties_id);
        
        g_free(_root_path);
        
        obex_agent_manager_unregister_agent(manager, OBEX_AGENT_DBUS_PATH, &error);
        g_object_unref(agent);
        g_object_unref(manager);
    }
    else if (opp_arg)
    {
        opp_device_arg = argv[1];
        opp_file_arg = argv[2];

        /* Check that `file` is valid */
        if (!is_file(opp_file_arg, &error))
        {
            exit_if_error(error);
        }
        
        _transfers = g_hash_table_new(g_str_hash, g_str_equal);
        _transfer_infos = g_hash_table_new(g_str_hash, g_str_equal);

        gchar * files_to_send[] = {NULL, NULL};
        files_to_send[0] = g_path_is_absolute(opp_file_arg) ? g_strdup(opp_file_arg) : get_absolute_path(opp_file_arg);

        /* Get source address (address of adapter) */
        Adapter *adapter = find_adapter(adapter_arg, &error);
        exit_if_error(error);
        gchar *src_address = g_strdup(adapter_get_address(adapter, &error));
        exit_if_error(error);
        
        /* Get destination address (address of remote device) */
        gchar *dst_address = NULL;
        if (g_regex_match_simple("^\\x{2}:\\x{2}:\\x{2}:\\x{2}:\\x{2}:\\x{2}$", opp_device_arg, 0, 0))
        {
            dst_address = g_strdup(opp_device_arg);
        }
        else
        {
            Device *device = find_device(adapter, opp_device_arg, &error);
            exit_if_error(error);
            dst_address = g_strdup(device_get_address(device, &error));
            exit_if_error(error);
            g_object_unref(device);
        }

        g_object_unref(adapter);

        /* Build arguments */
        GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(b, "{sv}", "Target", g_variant_new_string("opp"));
        g_variant_builder_add(b, "{sv}", "Source", g_variant_new_string(src_address));
        GVariant *device_dict = g_variant_builder_end(b);
        g_variant_builder_unref(b);

        mainloop = g_main_loop_new(NULL, FALSE);

        ObexClient *client = obex_client_new();
        const gchar *session_path = obex_client_create_session(client, dst_address, device_dict, &error);
        exit_if_error(error);
        ObexSession *session = obex_session_new(session_path);
        ObexObjectPush *oop = obex_object_push_new(obex_session_get_dbus_object_path(session));
        
        // initialize GDBus OBEX OPP client callbacks
        guint obex_opp_object_man_id = g_dbus_connection_signal_subscribe(session_conn, "org.bluez.obex", "org.freedesktop.DBus.ObjectManager", NULL, NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _obex_opp_client_object_manager_handler, NULL, NULL);
        guint obex_opp_properties_id = g_dbus_connection_signal_subscribe(session_conn, "org.bluez.obex", "org.freedesktop.DBus.Properties", "PropertiesChanged", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, _obex_opp_client_properties_handler, NULL, NULL);
        
        /* Sending file(s) */
        obex_object_push_send_file(oop, files_to_send[0], &error);
        exit_if_error(error);

        /* Add SIGTERM && SIGINT handlers */
        struct sigaction sa;
        memset(&sa, 0, sizeof (sa));
        sa.sa_handler = sigterm_handler;
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGINT, &sa, NULL);

        g_main_loop_run(mainloop);

        /* Sending files process here ?? */

        g_main_loop_unref(mainloop);

        g_dbus_connection_signal_unsubscribe(session_conn, obex_opp_object_man_id);
        g_dbus_connection_signal_unsubscribe(session_conn, obex_opp_properties_id);
        
        /* Stop active transfers */
        GHashTableIter iter;
        gpointer key, value;
        
        g_hash_table_iter_init(&iter, _transfers);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            ObexTransfer *t = OBEX_TRANSFER(value);
            obex_transfer_cancel(t, NULL); // skip errors
            g_object_unref(t);
            g_hash_table_iter_remove(&iter);
        }
        g_hash_table_unref(_transfers);
        
        // Remove transfer information objects
        g_hash_table_iter_init(&iter, _transfer_infos);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            g_free(value);
            g_hash_table_iter_remove(&iter);
        }
        g_hash_table_unref(_transfers);
        
        g_object_unref(client);

        g_variant_unref(device_dict);

        g_free(src_address);
        g_free(dst_address);
        g_free(files_to_send[0]);
        files_to_send[0] = NULL;
    }
    else if (ftp_arg)
    {
        /* Get source address (address of adapter) */
        Adapter *adapter = find_adapter(adapter_arg, &error);
        exit_if_error(error);
        gchar *src_address = g_strdup(adapter_get_address(adapter, &error));
        exit_if_error(error);

        /* Get destination address (address of remote device) */
        Device *device = find_device(adapter, ftp_arg, &error);
        exit_if_error(error);
        gchar *dst_address = g_strdup(device == NULL ? ftp_arg : device_get_address(device, &error));
        exit_if_error(error);

        g_object_unref(device);
        g_object_unref(adapter);

        /* Build arguments */
        GVariantBuilder *b;
        GVariant *device_dict;
        b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(b, "{sv}", "Target", g_variant_new_string("ftp"));
        g_variant_builder_add(b, "{sv}", "Source", g_variant_new_string(src_address));
        device_dict = g_variant_builder_end(b);
        g_variant_builder_unref(b);

        ObexClient *client = g_object_new(OBEX_CLIENT_TYPE, NULL);
        ObexAgent *agent = g_object_new(OBEX_AGENT_TYPE, NULL);

        /* Create FTP session */
        gchar *session_path = g_strdup(obex_client_create_session(client, dst_address, device_dict, &error));
        exit_if_error(error);

        ObexFileTransfer *ftp_session = obex_file_transfer_new(session_path);
        g_free(session_path);

        g_print("FTP session opened\n");

        while (TRUE)
        {
            gchar *cmd = readline("> ");
            if (cmd == NULL)
            {
                continue;
            }
            else
            {
                add_history(cmd);
            }

            gint f_argc;
            gchar **f_argv;
            /* Parsing command line */
            if (!g_shell_parse_argv(cmd, &f_argc, &f_argv, &error))
            {
                g_print("%s\n", error->message);
                g_error_free(error);
                error = NULL;

                g_free(cmd);
                continue;
            }

            /* Execute commands */
            if (g_strcmp0(f_argv[0], "cd") == 0)
            {
                if (f_argc != 2 || strlen(f_argv[1]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    obex_file_transfer_change_folder(ftp_session, f_argv[1], &error);
                    if (error)
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                }
            }
            else if (g_strcmp0(f_argv[0], "mkdir") == 0)
            {
                if (f_argc != 2 || strlen(f_argv[1]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    obex_file_transfer_create_folder(ftp_session, f_argv[1], &error);
                    if (error)
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                }
            }
            else if (g_strcmp0(f_argv[0], "ls") == 0)
            {
                if (f_argc != 1)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    // GPtrArray *folders = obex_file_transfer_list_folder(ftp_session, &error);
                    GVariant *folder_list = obex_file_transfer_list_folder(ftp_session, &error);
                    GPtrArray *folders = g_ptr_array_new();
                    gsize arr_size = 0;
                    // Pass the pointer
                    folders->pdata = (gpointer) g_variant_get_fixed_array(folder_list, &arr_size, sizeof(gpointer));
                    folders->len = arr_size;
                    
                    if (error)
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                    else
                    {
                        for (int i = 0; i < folders->len; i++)
                        {
                            /*
                            GHashTable *el = g_ptr_array_index(folders, i);
                            g_print(
                                    "%s\t%llu\t%s\n",
                                    g_value_get_string(g_hash_table_lookup(el, "Type")),
                                    G_VALUE_HOLDS_UINT64(g_hash_table_lookup(el, "Size")) ?
                                    g_value_get_uint64(g_hash_table_lookup(el, "Size")) :
                                    0,
                                    g_value_get_string(g_hash_table_lookup(el, "Name"))
                                    );
                            */
                            
                            // DO NOT FREE THIS
                            GVariant *el = g_ptr_array_index(folders, i);
                            g_print(
                                    "%s\t%" G_GINT64_FORMAT"\t%s\n",
                                    g_variant_get_string(g_variant_lookup_value(el, "Type", G_VARIANT_TYPE("s")), NULL),
                                    g_variant_get_uint64(g_variant_lookup_value(el, "Size", G_VARIANT_TYPE("t"))),
                                    g_variant_get_string(g_variant_lookup_value(el, "Name", G_VARIANT_TYPE("s")), NULL)
                                    );
                        }
                    }
                    
                    // Do not free the contents of the array. g_variant_unref will free contents
                    if (folders)
                        g_ptr_array_free(folders, FALSE);
                    
                    if (folder_list)
                        g_variant_unref(folder_list);

                    /*obexclient_remove_session(client, obexclient_file_transfer_get_dbus_object_path(ftp_session), &error);
                    exit_if_error(error);
                    g_object_unref(ftp_session);
                    session_path = obexclient_create_session(client, device_dict, &error);
                    exit_if_error(error);
                    ftp_session = g_object_new(OBEXCLIENT_FILE_TRANSFER_TYPE, "DBusObjectPath", session_path, NULL);
                    g_free(session_path);*/

                }
            }
            else if (g_strcmp0(f_argv[0], "get") == 0)
            {
                if (f_argc != 3 || strlen(f_argv[1]) == 0 || strlen(f_argv[2]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    gchar *abs_dst_path = get_absolute_path(f_argv[2]);
                    gchar *dir = g_path_get_dirname(abs_dst_path);
                    if (!is_dir(dir, &error))
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                    else
                    {
                        obex_file_transfer_get_file(ftp_session, abs_dst_path, f_argv[1], &error);
                        if (error)
                        {
                            g_print("%s\n", error->message);
                            g_error_free(error);
                            error = NULL;
                        }
                    }
                    g_free(dir);
                    g_free(abs_dst_path);
                }
            }
            else if (g_strcmp0(f_argv[0], "put") == 0)
            {
                if (f_argc != 3 || strlen(f_argv[1]) == 0 || strlen(f_argv[2]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    gchar *abs_src_path = get_absolute_path(f_argv[1]);
                    if (!is_file(abs_src_path, &error))
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                    else
                    {
                        obex_file_transfer_put_file(ftp_session, abs_src_path, f_argv[2], &error);
                        if (error)
                        {
                            g_print("%s\n", error->message);
                            g_error_free(error);
                            error = NULL;
                        }
                    }
                    g_free(abs_src_path);
                }
            }
            else if (g_strcmp0(f_argv[0], "cp") == 0)
            {
                if (f_argc != 3 || strlen(f_argv[1]) == 0 || strlen(f_argv[2]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    obex_file_transfer_copy_file(ftp_session, f_argv[1], f_argv[2], &error);
                    if (error)
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                }
            }
            else if (g_strcmp0(f_argv[0], "mv") == 0)
            {
                if (f_argc != 3 || strlen(f_argv[1]) == 0 || strlen(f_argv[2]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    obex_file_transfer_move_file(ftp_session, f_argv[1], f_argv[2], &error);
                    if (error)
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                }
            }
            else if (g_strcmp0(f_argv[0], "rm") == 0)
            {
                if (f_argc != 2 || strlen(f_argv[1]) == 0)
                {
                    g_print("invalid arguments\n");
                }
                else
                {
                    obex_file_transfer_delete(ftp_session, f_argv[1], &error);
                    if (error)
                    {
                        g_print("%s\n", error->message);
                        g_error_free(error);
                        error = NULL;
                    }
                }
            }
            else if (g_strcmp0(f_argv[0], "help") == 0)
            {
                g_print(
                        "help\t\t\tShow this message\n"
                        "exit\t\t\tClose FTP session\n"
                        "cd <folder>\t\tChange the current folder of the remote device\n"
                        "mkdir <folder>\t\tCreate a new folder in the remote device\n"
                        "ls\t\t\tList folder contents\n"
                        "get <src> <dst>\t\tCopy the src file (from remote device) to the dst file (on local filesystem)\n"
                        "put <src> <dst>\t\tCopy the src file (from local filesystem) to the dst file (on remote device)\n"
                        "cp <src> <dst>\t\tCopy a file within the remote device from src file to dst file\n"
                        "mv <src> <dst>\t\tMove a file within the remote device from src file to dst file\n"
                        "rm <target>\t\tDeletes the specified file/folder\n"
                        );
            }
            else if (g_strcmp0(f_argv[0], "exit") == 0 || g_strcmp0(f_argv[0], "quit") == 0)
            {
                obex_client_remove_session(client, obex_file_transfer_get_dbus_object_path(ftp_session), &error);
                exit_if_error(error);

                g_strfreev(f_argv);
                g_free(cmd);
                break;
            }
            else
            {
                g_print("invalid command\n");
            }

            g_strfreev(f_argv);
            g_free(cmd);
        }

        g_object_unref(agent);
        g_object_unref(client);
        g_object_unref(ftp_session);

        g_variant_unref(device_dict);

        g_free(src_address);
        g_free(dst_address);
    }

    dbus_disconnect();

    exit(EXIT_SUCCESS);
}
