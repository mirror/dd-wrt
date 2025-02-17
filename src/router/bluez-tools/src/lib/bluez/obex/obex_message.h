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

#ifndef __OBEX_MESSAGE_H
#define __OBEX_MESSAGE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_MESSAGE_DBUS_SERVICE "org.bluez.obex"
#define OBEX_MESSAGE_DBUS_INTERFACE "org.bluez.obex.Message1"

/*
 * Type macros
 */
#define OBEX_MESSAGE_TYPE				(obex_message_get_type())
#define OBEX_MESSAGE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_MESSAGE_TYPE, ObexMessage))
#define OBEX_MESSAGE_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_MESSAGE_TYPE))
#define OBEX_MESSAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_MESSAGE_TYPE, ObexMessageClass))
#define OBEX_MESSAGE_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_MESSAGE_TYPE))
#define OBEX_MESSAGE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_MESSAGE_TYPE, ObexMessageClass))

typedef struct _ObexMessage ObexMessage;
typedef struct _ObexMessageClass ObexMessageClass;
typedef struct _ObexMessagePrivate ObexMessagePrivate;

struct _ObexMessage {
	GObject parent_instance;

	/*< private >*/
	ObexMessagePrivate *priv;
};

struct _ObexMessageClass {
	GObjectClass parent_class;
};

/* used by OBEX_MESSAGE_TYPE */
GType obex_message_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexMessage *obex_message_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_message_get_dbus_object_path(ObexMessage *self);

GVariant *obex_message_get_properties(ObexMessage *self, GError **error);
void obex_message_set_property(ObexMessage *self, const gchar *name, const GVariant *value, GError **error);

void obex_message_set_deleted(ObexMessage *self, const gboolean value, GError **error);
const gchar *obex_message_get_folder(ObexMessage *self, GError **error);
gboolean obex_message_get_priority(ObexMessage *self, GError **error);
gboolean obex_message_get_protected(ObexMessage *self, GError **error);
gboolean obex_message_get_read(ObexMessage *self, GError **error);
void obex_message_set_read(ObexMessage *self, const gboolean value, GError **error);
const gchar *obex_message_get_recipient(ObexMessage *self, GError **error);
const gchar *obex_message_get_recipient_address(ObexMessage *self, GError **error);
const gchar *obex_message_get_reply_to(ObexMessage *self, GError **error);
const gchar *obex_message_get_sender(ObexMessage *self, GError **error);
const gchar *obex_message_get_sender_address(ObexMessage *self, GError **error);
gboolean obex_message_get_sent(ObexMessage *self, GError **error);
guint64 obex_message_get_size(ObexMessage *self, GError **error);
const gchar *obex_message_get_status(ObexMessage *self, GError **error);
const gchar *obex_message_get_subject(ObexMessage *self, GError **error);
const gchar *obex_message_get_timestamp(ObexMessage *self, GError **error);
// This has been renamed because 'obex_message_get_type' is already used by GLib
const gchar *obex_message_get_message_type(ObexMessage *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_MESSAGE_H */

