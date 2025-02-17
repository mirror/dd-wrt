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

#ifndef __OBEX_OBJECT_PUSH_H
#define __OBEX_OBJECT_PUSH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_OBJECT_PUSH_DBUS_SERVICE "org.bluez.obex"
#define OBEX_OBJECT_PUSH_DBUS_INTERFACE "org.bluez.obex.ObjectPush1"

/*
 * Type macros
 */
#define OBEX_OBJECT_PUSH_TYPE				(obex_object_push_get_type())
#define OBEX_OBJECT_PUSH(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_OBJECT_PUSH_TYPE, ObexObjectPush))
#define OBEX_OBJECT_PUSH_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_OBJECT_PUSH_TYPE))
#define OBEX_OBJECT_PUSH_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_OBJECT_PUSH_TYPE, ObexObjectPushClass))
#define OBEX_OBJECT_PUSH_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_OBJECT_PUSH_TYPE))
#define OBEX_OBJECT_PUSH_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_OBJECT_PUSH_TYPE, ObexObjectPushClass))

typedef struct _ObexObjectPush ObexObjectPush;
typedef struct _ObexObjectPushClass ObexObjectPushClass;
typedef struct _ObexObjectPushPrivate ObexObjectPushPrivate;

struct _ObexObjectPush {
	GObject parent_instance;

	/*< private >*/
	ObexObjectPushPrivate *priv;
};

struct _ObexObjectPushClass {
	GObjectClass parent_class;
};

/* used by OBEX_OBJECT_PUSH_TYPE */
GType obex_object_push_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexObjectPush *obex_object_push_new(const gchar *dbus_object_path);
const gchar *obex_object_push_get_dbus_object_path(ObexObjectPush *self);

GVariant *obex_object_push_exchange_business_cards(ObexObjectPush *self, const gchar *clientfile, const gchar *targetfile, GError **error);
GVariant *obex_object_push_pull_business_card(ObexObjectPush *self, const gchar *targetfile, GError **error);
GVariant *obex_object_push_send_file(ObexObjectPush *self, const gchar *sourcefile, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_OBJECT_PUSH_H */

