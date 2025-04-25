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

#ifndef __OBEX_FILE_TRANSFER_H
#define __OBEX_FILE_TRANSFER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_FILE_TRANSFER_DBUS_SERVICE "org.bluez.obex"
#define OBEX_FILE_TRANSFER_DBUS_INTERFACE "org.bluez.obex.FileTransfer1"

/*
 * Type macros
 */
#define OBEX_FILE_TRANSFER_TYPE				(obex_file_transfer_get_type())
#define OBEX_FILE_TRANSFER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_FILE_TRANSFER_TYPE, ObexFileTransfer))
#define OBEX_FILE_TRANSFER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_FILE_TRANSFER_TYPE))
#define OBEX_FILE_TRANSFER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_FILE_TRANSFER_TYPE, ObexFileTransferClass))
#define OBEX_FILE_TRANSFER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_FILE_TRANSFER_TYPE))
#define OBEX_FILE_TRANSFER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_FILE_TRANSFER_TYPE, ObexFileTransferClass))

typedef struct _ObexFileTransfer ObexFileTransfer;
typedef struct _ObexFileTransferClass ObexFileTransferClass;
typedef struct _ObexFileTransferPrivate ObexFileTransferPrivate;

struct _ObexFileTransfer {
	GObject parent_instance;

	/*< private >*/
	ObexFileTransferPrivate *priv;
};

struct _ObexFileTransferClass {
	GObjectClass parent_class;
};

/* used by OBEX_FILE_TRANSFER_TYPE */
GType obex_file_transfer_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexFileTransfer *obex_file_transfer_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_file_transfer_get_dbus_object_path(ObexFileTransfer *self);

void obex_file_transfer_change_folder(ObexFileTransfer *self, const gchar *folder, GError **error);
void obex_file_transfer_copy_file(ObexFileTransfer *self, const gchar *sourcefile, const gchar *targetfile, GError **error);
void obex_file_transfer_create_folder(ObexFileTransfer *self, const gchar *folder, GError **error);
void obex_file_transfer_delete(ObexFileTransfer *self, const gchar *file, GError **error);
GVariant *obex_file_transfer_get_file(ObexFileTransfer *self, const gchar *targetfile, const gchar *sourcefile, GError **error);
GVariant *obex_file_transfer_list_folder(ObexFileTransfer *self, GError **error);
void obex_file_transfer_move_file(ObexFileTransfer *self, const gchar *sourcefile, const gchar *targetfile, GError **error);
GVariant *obex_file_transfer_put_file(ObexFileTransfer *self, const gchar *sourcefile, const gchar *targetfile, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_FILE_TRANSFER_H */

