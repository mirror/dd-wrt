#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>

#include "bluez-api.h"

#include "dbus-common.h"

GDBusConnection *session_conn = NULL;
GDBusConnection *system_conn = NULL;

static gboolean dbus_initialized = FALSE;

void dbus_init()
{
    dbus_initialized = TRUE;
}

gboolean dbus_session_connect(GError **error)
{
    g_assert(dbus_initialized == TRUE);

    session_conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, error);
    if (!session_conn)
    {
        return FALSE;
    }

    return TRUE;
}

void dbus_session_disconnect()
{
    g_object_unref(session_conn);
}

gboolean dbus_system_connect(GError **error)
{
    g_assert(dbus_initialized == TRUE);

    system_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, error);
    if (!system_conn)
    {
        return FALSE;
    }

    return TRUE;
}

void dbus_system_disconnect()
{
    g_object_unref(system_conn);
}

void dbus_disconnect()
{
    if (system_conn)
        dbus_system_disconnect();
    if (session_conn)
        dbus_session_disconnect();
}
