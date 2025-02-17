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

#ifndef __OBEX_CLIENT_H
#define __OBEX_CLIENT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_CLIENT_DBUS_SERVICE "org.bluez.obex"
#define OBEX_CLIENT_DBUS_INTERFACE "org.bluez.obex.Client1"
#define OBEX_CLIENT_DBUS_PATH "/org/bluez/obex"

/*
 * Type macros
 */
#define OBEX_CLIENT_TYPE				(obex_client_get_type())
#define OBEX_CLIENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_CLIENT_TYPE, ObexClient))
#define OBEX_CLIENT_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_CLIENT_TYPE))
#define OBEX_CLIENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_CLIENT_TYPE, ObexClientClass))
#define OBEX_CLIENT_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_CLIENT_TYPE))
#define OBEX_CLIENT_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_CLIENT_TYPE, ObexClientClass))

typedef struct _ObexClient ObexClient;
typedef struct _ObexClientClass ObexClientClass;
typedef struct _ObexClientPrivate ObexClientPrivate;

struct _ObexClient {
	GObject parent_instance;

	/*< private >*/
	ObexClientPrivate *priv;
};

struct _ObexClientClass {
	GObjectClass parent_class;
};

/* used by OBEX_CLIENT_TYPE */
GType obex_client_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexClient *obex_client_new();

/*
 * Method definitions
 */
const gchar *obex_client_create_session(ObexClient *self, const gchar *destination, const GVariant *args, GError **error);
void obex_client_remove_session(ObexClient *self, const gchar *session, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_CLIENT_H */

