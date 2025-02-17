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

#ifndef __OBEX_PHONEBOOK_ACCESS_H
#define __OBEX_PHONEBOOK_ACCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_PHONEBOOK_ACCESS_DBUS_SERVICE "org.bluez.obex"
#define OBEX_PHONEBOOK_ACCESS_DBUS_INTERFACE "org.bluez.obex.PhonebookAccess1"

/*
 * Type macros
 */
#define OBEX_PHONEBOOK_ACCESS_TYPE				(obex_phonebook_access_get_type())
#define OBEX_PHONEBOOK_ACCESS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_PHONEBOOK_ACCESS_TYPE, ObexPhonebookAccess))
#define OBEX_PHONEBOOK_ACCESS_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_PHONEBOOK_ACCESS_TYPE))
#define OBEX_PHONEBOOK_ACCESS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_PHONEBOOK_ACCESS_TYPE, ObexPhonebookAccessClass))
#define OBEX_PHONEBOOK_ACCESS_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_PHONEBOOK_ACCESS_TYPE))
#define OBEX_PHONEBOOK_ACCESS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_PHONEBOOK_ACCESS_TYPE, ObexPhonebookAccessClass))

typedef struct _ObexPhonebookAccess ObexPhonebookAccess;
typedef struct _ObexPhonebookAccessClass ObexPhonebookAccessClass;
typedef struct _ObexPhonebookAccessPrivate ObexPhonebookAccessPrivate;

struct _ObexPhonebookAccess {
	GObject parent_instance;

	/*< private >*/
	ObexPhonebookAccessPrivate *priv;
};

struct _ObexPhonebookAccessClass {
	GObjectClass parent_class;
};

/* used by OBEX_PHONEBOOK_ACCESS_TYPE */
GType obex_phonebook_access_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexPhonebookAccess *obex_phonebook_access_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_phonebook_access_get_dbus_object_path(ObexPhonebookAccess *self);

guint16 obex_phonebook_access_get_size(ObexPhonebookAccess *self, GError **error);
const gchar **obex_phonebook_access_list_filter_fields(ObexPhonebookAccess *self, GError **error);
void obex_phonebook_access_select(ObexPhonebookAccess *self, const gchar *location, const gchar *phonebook, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_PHONEBOOK_ACCESS_H */

