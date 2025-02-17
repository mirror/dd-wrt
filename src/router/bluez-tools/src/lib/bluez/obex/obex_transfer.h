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

#ifndef __OBEX_TRANSFER_H
#define __OBEX_TRANSFER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_TRANSFER_DBUS_SERVICE "org.bluez.obex"
#define OBEX_TRANSFER_DBUS_INTERFACE "org.bluez.obex.Transfer1"

/*
 * Type macros
 */
#define OBEX_TRANSFER_TYPE				(obex_transfer_get_type())
#define OBEX_TRANSFER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_TRANSFER_TYPE, ObexTransfer))
#define OBEX_TRANSFER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_TRANSFER_TYPE))
#define OBEX_TRANSFER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_TRANSFER_TYPE, ObexTransferClass))
#define OBEX_TRANSFER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_TRANSFER_TYPE))
#define OBEX_TRANSFER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_TRANSFER_TYPE, ObexTransferClass))

typedef struct _ObexTransfer ObexTransfer;
typedef struct _ObexTransferClass ObexTransferClass;
typedef struct _ObexTransferPrivate ObexTransferPrivate;

struct _ObexTransfer {
	GObject parent_instance;

	/*< private >*/
	ObexTransferPrivate *priv;
};

struct _ObexTransferClass {
	GObjectClass parent_class;
};

/* used by OBEX_TRANSFER_TYPE */
GType obex_transfer_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexTransfer *obex_transfer_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_transfer_get_dbus_object_path(ObexTransfer *self);

void obex_transfer_cancel(ObexTransfer *self, GError **error);
void obex_transfer_resume(ObexTransfer *self, GError **error);
void obex_transfer_suspend(ObexTransfer *self, GError **error);

GVariant *obex_transfer_get_properties(ObexTransfer *self, GError **error);
void obex_transfer_set_property(ObexTransfer *self, const gchar *name, const GVariant *value, GError **error);

const gchar *obex_transfer_get_filename(ObexTransfer *self, GError **error);
const gchar *obex_transfer_get_name(ObexTransfer *self, GError **error);
const gchar *obex_transfer_get_session(ObexTransfer *self, GError **error);
guint64 obex_transfer_get_size(ObexTransfer *self, GError **error);
const gchar *obex_transfer_get_status(ObexTransfer *self, GError **error);
guint64 obex_transfer_get_time(ObexTransfer *self, GError **error);
guint64 obex_transfer_get_transferred(ObexTransfer *self, GError **error);
// This has been renamed because 'obex_transfer_get_type' is already used by GLib
const gchar *obex_transfer_get_transfer_type(ObexTransfer *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_TRANSFER_H */

