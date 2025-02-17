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

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "dbus-common.h"
#include "helpers.h"

/* UUID Name lookup table */
typedef struct
{
    gchar *uuid;
    gchar *name;
    gchar *alt_name;
} uuid_name_lookup_table_t;

static uuid_name_lookup_table_t uuid_name_lookup_table[] = {
    {"00001000-0000-1000-8000-00805F9B34FB", "ServiceDiscoveryServer", NULL},
    {"00001001-0000-1000-8000-00805F9B34FB", "BrowseGroupDescriptor", NULL},
    {"00001002-0000-1000-8000-00805F9B34FB", "PublicBrowseGroup", NULL},
    {"00001101-0000-1000-8000-00805F9B34FB", "SerialPort", "Serial"},
    {"00001102-0000-1000-8000-00805F9B34FB", "LANAccessUsingPPP", NULL},
    {"00001103-0000-1000-8000-00805F9B34FB", "DialupNetworking", "DUN"},
    {"00001104-0000-1000-8000-00805F9B34FB", "IrMCSync", NULL},
    {"00001105-0000-1000-8000-00805F9B34FB", "OBEXObjectPush", NULL},
    {"00001106-0000-1000-8000-00805F9B34FB", "OBEXFileTransfer", NULL},
    {"00001107-0000-1000-8000-00805F9B34FB", "IrMCSyncCommand", NULL},
    {"00001108-0000-1000-8000-00805F9B34FB", "Headset", NULL},
    {"00001109-0000-1000-8000-00805F9B34FB", "CordlessTelephony", NULL},
    {"0000110A-0000-1000-8000-00805F9B34FB", "AudioSource", NULL},
    {"0000110B-0000-1000-8000-00805F9B34FB", "AudioSink", NULL},
    {"0000110C-0000-1000-8000-00805F9B34FB", "AVRemoteControlTarget", NULL},
    {"0000110D-0000-1000-8000-00805F9B34FB", "AdvancedAudioDistribution", "A2DP"},
    {"0000110E-0000-1000-8000-00805F9B34FB", "AVRemoteControl", NULL},
    {"0000110F-0000-1000-8000-00805F9B34FB", "VideoConferencing", NULL},
    {"00001110-0000-1000-8000-00805F9B34FB", "Intercom", NULL},
    {"00001111-0000-1000-8000-00805F9B34FB", "Fax", NULL},
    {"00001112-0000-1000-8000-00805F9B34FB", "HeadsetAudioGateway", NULL},
    {"00001113-0000-1000-8000-00805F9B34FB", "WAP", NULL},
    {"00001114-0000-1000-8000-00805F9B34FB", "WAPClient", NULL},
    {"00001115-0000-1000-8000-00805F9B34FB", "PANU", NULL},
    {"00001116-0000-1000-8000-00805F9B34FB", "NAP", NULL},
    {"00001117-0000-1000-8000-00805F9B34FB", "GN", NULL},
    {"00001118-0000-1000-8000-00805F9B34FB", "DirectPrinting", NULL},
    {"00001119-0000-1000-8000-00805F9B34FB", "ReferencePrinting", NULL},
    {"0000111A-0000-1000-8000-00805F9B34FB", "Imaging", NULL},
    {"0000111B-0000-1000-8000-00805F9B34FB", "ImagingResponder", NULL},
    {"0000111C-0000-1000-8000-00805F9B34FB", "ImagingAutomaticArchive", NULL},
    {"0000111D-0000-1000-8000-00805F9B34FB", "ImagingReferenceObjects", NULL},
    {"0000111E-0000-1000-8000-00805F9B34FB", "Handsfree", NULL},
    {"0000111F-0000-1000-8000-00805F9B34FB", "HandsfreeAudioGateway", NULL},
    {"00001120-0000-1000-8000-00805F9B34FB", "DirectPrintingReferenceObjects", NULL},
    {"00001121-0000-1000-8000-00805F9B34FB", "ReflectedUI", NULL},
    {"00001122-0000-1000-8000-00805F9B34FB", "BasicPringing", NULL},
    {"00001123-0000-1000-8000-00805F9B34FB", "PrintingStatus", NULL},
    {"00001124-0000-1000-8000-00805F9B34FB", "HumanInterfaceDevice", "HID"},
    {"00001125-0000-1000-8000-00805F9B34FB", "HardcopyCableReplacement", NULL},
    {"00001126-0000-1000-8000-00805F9B34FB", "HCRPrint", NULL},
    {"00001127-0000-1000-8000-00805F9B34FB", "HCRScan", NULL},
    {"00001128-0000-1000-8000-00805F9B34FB", "CommonISDNAccess", NULL},
    {"00001129-0000-1000-8000-00805F9B34FB", "VideoConferencingGW", NULL},
    {"0000112A-0000-1000-8000-00805F9B34FB", "UDIMT", NULL},
    {"0000112B-0000-1000-8000-00805F9B34FB", "UDITA", NULL},
    {"0000112C-0000-1000-8000-00805F9B34FB", "AudioVideo", NULL},
    {"00001200-0000-1000-8000-00805F9B34FB", "PnPInformation", NULL},
    {"00001201-0000-1000-8000-00805F9B34FB", "GenericNetworking", NULL},
    {"00001202-0000-1000-8000-00805F9B34FB", "GenericFileTransfer", NULL},
    {"00001203-0000-1000-8000-00805F9B34FB", "GenericAudio", NULL},
    {"00001204-0000-1000-8000-00805F9B34FB", "GenericTelephony", NULL},
    {"00001205-0000-1000-8000-00805F9B34FB", "UPnP", NULL},
    {"00001206-0000-1000-8000-00805F9B34FB", "UPnPIp", NULL},
    {"00001300-0000-1000-8000-00805F9B34FB", "ESdpUPnPIpPan", NULL},
    {"00001301-0000-1000-8000-00805F9B34FB", "ESdpUPnPIpLap", NULL},
    {"00001302-0000-1000-8000-00805F9B34FB", "EdpUPnpIpL2CAP", NULL},
    {"0000112D-0000-1000-8000-00805F9B34FB", "SIMAccess", "SAP"},

    // Custom:
    {"0000112F-0000-1000-8000-00805F9B34FB", "PhoneBookAccess", NULL},
    {"831C4071-7BC8-4A9C-A01C-15DF25A4ADBC", "ActiveSync", NULL},
};

#define UUID_NAME_LOOKUP_TABLE_SIZE \
	(sizeof(uuid_name_lookup_table)/sizeof(uuid_name_lookup_table_t))

const gchar *uuid2name(const gchar *uuid)
{
    if (uuid == NULL || strlen(uuid) == 0)
        return NULL;

    for (int i = 0; i < UUID_NAME_LOOKUP_TABLE_SIZE; i++)
    {
        if (g_ascii_strcasecmp(uuid_name_lookup_table[i].uuid, uuid) == 0)
            return uuid_name_lookup_table[i].name;
    }

    return uuid;
}

const gchar *name2uuid(const gchar *name)
{
    if (name == NULL || strlen(name) == 0)
        return NULL;

    for (int i = 0; i < UUID_NAME_LOOKUP_TABLE_SIZE; i++)
    {
        if (
                g_ascii_strcasecmp(uuid_name_lookup_table[i].name, name) == 0 ||
                (uuid_name_lookup_table[i].alt_name && g_ascii_strcasecmp(uuid_name_lookup_table[i].alt_name, name) == 0)
                )
            return uuid_name_lookup_table[i].uuid;
    }

    return name;
}

int xtoi(const gchar *str)
{
    int i = 0;
    sscanf(str, "0x%x", &i);
    return i;
}

Adapter *find_adapter(const gchar *name, GError **error)
{
    gchar *adapter_path = NULL;
    Adapter *adapter = NULL;

    Manager *manager = g_object_new(MANAGER_TYPE, NULL);

    // If name is null or empty - return default adapter
    if (name == NULL || strlen(name) == 0)
    {
        adapter_path = (gchar *) manager_default_adapter(manager, error);
        if (adapter_path)
        {
            // adapter = g_object_new(ADAPTER_TYPE, "DBusObjectPath", adapter_path, NULL);
            adapter = adapter_new(adapter_path);
        }
    }
    else
    {
        // Try to find by id
        adapter_path = (gchar *) manager_find_adapter(manager, name, error);

        // Found
        if (adapter_path)
        {
            // adapter = g_object_new(ADAPTER_TYPE, "DBusObjectPath", adapter_path, NULL);
            adapter = adapter_new(adapter_path);
        }
        else
        {
            // Try to find by name
            const GPtrArray *adapters_list = manager_get_adapters(manager);
            g_assert(adapters_list != NULL);
            for (int i = 0; i < adapters_list->len; i++)
            {
                adapter_path = g_ptr_array_index(adapters_list, i);
                // adapter = g_object_new(ADAPTER_TYPE, "DBusObjectPath", adapter_path, NULL);
                adapter = adapter_new(adapter_path);
                adapter_path = NULL;

                if (g_strcmp0(name, adapter_get_name(adapter, error)) == 0)
                {
                    if (error)
                    {
                        g_error_free(*error);
                        *error = NULL;
                    }
                    break;
                }

                g_object_unref(adapter);
                adapter = NULL;
            }
        }
    }

    g_object_unref(manager);
    if (adapter_path) g_free(adapter_path);

    return adapter;
}

Device *find_device(Adapter *adapter, const gchar *name, GError **error)
{
    g_assert(adapter != NULL && ADAPTER_IS(adapter));
    g_assert(name != NULL && strlen(name) > 0);

    Device *device = NULL;
    
    Manager *manager = g_object_new(MANAGER_TYPE, NULL);
    
    GVariant *objects = manager_get_managed_objects(manager, error);
    
    g_clear_object(&manager);
    
    if(!objects)
    {
        return NULL;
    }
    
    const gchar *object_path;
    GVariant *ifaces_and_properties;
    GVariantIter i;

    g_variant_iter_init(&i, objects);
    while (g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties) && device == NULL)
    {
        GVariant *properties = NULL;
        
        if(g_variant_lookup(ifaces_and_properties, DEVICE_DBUS_INTERFACE, "@a{sv}", &properties))
        {
            gchar *adapter_path = NULL;
            if(g_variant_lookup(properties, "Adapter", "o", &adapter_path))
            {
                if(g_strcmp0(adapter_path, adapter_get_dbus_object_path(adapter)) == 0)
                {
                    // Now check if this is the device we are looking for.
                    
                    // Try to find by MAC
                    gchar *address = NULL;
                    
                    if(g_variant_lookup(properties, "Address", "s", &address))
                    {
                        if(g_strcmp0(g_ascii_strdown(address, -1), g_ascii_strdown(name, -1)) == 0)
                        {
                            device = device_new(object_path);
                        }
                        g_free(address);
                    }
                    
                    // Try to find by name or alias
                    if(!device)
                    {
                        gchar *device_name = NULL;
                        gchar *device_alias = NULL;
                        g_variant_lookup(properties, "Name", "s", &device_name);
                        g_variant_lookup(properties, "Alias", "s", &device_alias);
                        
                        if (g_strcmp0(name, device_name) == 0 || g_strcmp0(name, device_alias) == 0) {
                            device = device_new(object_path);
                        }
                        
                        g_free(device_alias);
                        g_free(device_name);
                    }
                }
                g_free(adapter_path);
            }
            g_variant_unref(properties);
        }
        g_variant_unref(ifaces_and_properties);
    }
    g_variant_unref(objects);
    
    return device;
}

gboolean intf_supported(const gchar *dbus_service_name, const gchar *dbus_object_path, const gchar *intf_name)
{
    g_assert(dbus_service_name != NULL && strlen(dbus_service_name) > 0);
    g_assert(dbus_object_path != NULL && strlen(dbus_object_path) > 0);
    g_assert(intf_name != NULL && strlen(intf_name) > 0);

    gboolean supported = FALSE;
    GDBusConnection *conn = NULL;

    if (g_strcmp0(dbus_service_name, BLUEZ_DBUS_SERVICE_NAME) == 0)
    {
        conn = system_conn;
    }
    else if (g_strcmp0(dbus_service_name, BLUEZ_OBEX_DBUS_SERVICE_NAME) == 0)
    {
        conn = session_conn;
    }
    else
    {
        return FALSE;
    }
    g_assert(conn != NULL);

    gchar *check_intf_regex_str = g_strconcat("<interface name=\"", intf_name, "\">", NULL);

    /* Getting introspection XML */
    GError *error = NULL;
    GDBusProxy *introspection_proxy = g_dbus_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, "org.freedesktop.DBus.Introspectable", NULL, &error);
    g_assert(introspection_proxy != NULL);
    GVariant *introspection_ret = g_dbus_proxy_call_sync(introspection_proxy, "Introspect", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
    gchar *introspection_xml = NULL;
    if (!introspection_ret)
    {
#if 0
        g_critical("%s", error->message);
#else
        g_error_free(error);
        error = NULL;
        introspection_xml = g_strdup("null");
#endif
    }
    else
    {
        introspection_xml = (gchar *) g_variant_get_string(g_variant_get_child_value(introspection_ret, 0), NULL);
    }
    g_assert(error == NULL);
    
    if (g_regex_match_simple(check_intf_regex_str, introspection_xml, 0, 0))
    {
        supported = TRUE;
    }

    g_free(check_intf_regex_str);
    g_free(introspection_xml);
    if(introspection_ret)
        g_variant_unref(introspection_ret);
    g_object_unref(introspection_proxy);

    return supported;
}

gboolean is_file(const gchar *filename, GError **error)
{
    g_assert(filename != NULL && strlen(filename) > 0);

    struct stat buf;
    if (stat(filename, &buf) != 0)
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 1, "%s: %s", g_strdup(filename), strerror(errno));
        }
        return FALSE;
    }

    if (!S_ISREG(buf.st_mode))
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 2, "%s: Invalid file", g_strdup(filename));
        }
        return FALSE;
    }

    return TRUE;
}

gboolean is_dir(const gchar *dirname, GError **error)
{
    g_assert(dirname != NULL && strlen(dirname) > 0);

    struct stat buf;
    if (stat(dirname, &buf) != 0)
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 1, "%s: %s", g_strdup(dirname), strerror(errno));
        }
        return FALSE;
    }

    if (!S_ISDIR(buf.st_mode))
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 2, "%s: Invalid directory", g_strdup(dirname));
        }
        return FALSE;
    }

    return TRUE;
}

gboolean read_access(const gchar *path, GError **error)
{
    if(g_access(path, F_OK) != 0)
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 3, "%s: No such file or directory", g_strdup(path));
        }
        return FALSE;
    }
    if(g_access(path, R_OK | F_OK) != 0)
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 4, "%s: Read permission denied", g_strdup(path));
        }
        return FALSE;
    }
    
    return TRUE;
}

gboolean write_access(const gchar *path, GError **error)
{
    if(g_access(path, F_OK) != 0)
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 3, "%s: No such file or directory", g_strdup(path));
        }
        return FALSE;
    }
    if(g_access(path, W_OK | F_OK) != 0)
    {
        if (error)
        {
            *error = g_error_new(g_quark_from_string("bluez-tools"), 5, "%s: Write permission denied", g_strdup(path));
        }
        return FALSE;
    }
    
    return TRUE;
}

gchar *get_absolute_path(const gchar *path)
{
    if (g_path_is_absolute(path))
    {
        return g_strdup(path);
    }

    gchar *current_dir = g_get_current_dir();
    gchar *abs_path = g_build_filename(current_dir, path, NULL);
    g_free(current_dir);

    return abs_path;
}
