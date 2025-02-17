#ifndef DBUS_COMMON_H
#define	DBUS_COMMON_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <gio/gio.h>

extern GDBusConnection *session_conn;
extern GDBusConnection *system_conn;

void dbus_init();
gboolean dbus_session_connect(GError **error);
void dbus_session_disconnect();
gboolean dbus_system_connect(GError **error);
void dbus_system_disconnect();
void dbus_disconnect();

#ifdef	__cplusplus
}
#endif

#endif	/* DBUS_COMMON_H */

