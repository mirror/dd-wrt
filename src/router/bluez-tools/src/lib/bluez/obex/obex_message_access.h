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

#ifndef __OBEX_MESSAGE_ACCESS_H
#define __OBEX_MESSAGE_ACCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_MESSAGE_ACCESS_DBUS_SERVICE "org.bluez.obex"
#define OBEX_MESSAGE_ACCESS_DBUS_INTERFACE "org.bluez.obex.MessageAccess1"

/*
 * Type macros
 */
#define OBEX_MESSAGE_ACCESS_TYPE				(obex_message_access_get_type())
#define OBEX_MESSAGE_ACCESS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_MESSAGE_ACCESS_TYPE, ObexMessageAccess))
#define OBEX_MESSAGE_ACCESS_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_MESSAGE_ACCESS_TYPE))
#define OBEX_MESSAGE_ACCESS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_MESSAGE_ACCESS_TYPE, ObexMessageAccessClass))
#define OBEX_MESSAGE_ACCESS_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_MESSAGE_ACCESS_TYPE))
#define OBEX_MESSAGE_ACCESS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_MESSAGE_ACCESS_TYPE, ObexMessageAccessClass))

typedef struct _ObexMessageAccess ObexMessageAccess;
typedef struct _ObexMessageAccessClass ObexMessageAccessClass;
typedef struct _ObexMessageAccessPrivate ObexMessageAccessPrivate;

struct _ObexMessageAccess {
	GObject parent_instance;

	/*< private >*/
	ObexMessageAccessPrivate *priv;
};

struct _ObexMessageAccessClass {
	GObjectClass parent_class;
};

/* used by OBEX_MESSAGE_ACCESS_TYPE */
GType obex_message_access_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexMessageAccess *obex_message_access_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_message_access_get_dbus_object_path(ObexMessageAccess *self);

const gchar **obex_message_access_list_filter_fields(ObexMessageAccess *self, GError **error);
GVariant *obex_message_access_list_folders(ObexMessageAccess *self, const GVariant *filter, GError **error);
void obex_message_access_set_folder(ObexMessageAccess *self, const gchar *name, GError **error);
void obex_message_access_update_inbox(ObexMessageAccess *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_MESSAGE_ACCESS_H */

