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

#ifndef __OBEX_SESSION_H
#define __OBEX_SESSION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_SESSION_DBUS_SERVICE "org.bluez.obex"
#define OBEX_SESSION_DBUS_INTERFACE "org.bluez.obex.Session1"

/*
 * Type macros
 */
#define OBEX_SESSION_TYPE				(obex_session_get_type())
#define OBEX_SESSION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_SESSION_TYPE, ObexSession))
#define OBEX_SESSION_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_SESSION_TYPE))
#define OBEX_SESSION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_SESSION_TYPE, ObexSessionClass))
#define OBEX_SESSION_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_SESSION_TYPE))
#define OBEX_SESSION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_SESSION_TYPE, ObexSessionClass))

typedef struct _ObexSession ObexSession;
typedef struct _ObexSessionClass ObexSessionClass;
typedef struct _ObexSessionPrivate ObexSessionPrivate;

struct _ObexSession {
	GObject parent_instance;

	/*< private >*/
	ObexSessionPrivate *priv;
};

struct _ObexSessionClass {
	GObjectClass parent_class;
};

/* used by OBEX_SESSION_TYPE */
GType obex_session_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexSession *obex_session_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_session_get_dbus_object_path(ObexSession *self);

const gchar *obex_session_get_capabilities(ObexSession *self, GError **error);

GVariant *obex_session_get_properties(ObexSession *self, GError **error);
void obex_session_set_property(ObexSession *self, const gchar *name, const GVariant *value, GError **error);

guint8 obex_session_get_channel(ObexSession *self, GError **error);
const gchar *obex_session_get_destination(ObexSession *self, GError **error);
const gchar *obex_session_get_root(ObexSession *self, GError **error);
const gchar *obex_session_get_source(ObexSession *self, GError **error);
const gchar *obex_session_get_target(ObexSession *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_SESSION_H */

