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

#ifndef __OBEX_SYNCHRONIZATION_H
#define __OBEX_SYNCHRONIZATION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define OBEX_SYNCHRONIZATION_DBUS_SERVICE "org.bluez.obex"
#define OBEX_SYNCHRONIZATION_DBUS_INTERFACE "org.bluez.obex.Synchronization1"

/*
 * Type macros
 */
#define OBEX_SYNCHRONIZATION_TYPE				(obex_synchronization_get_type())
#define OBEX_SYNCHRONIZATION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), OBEX_SYNCHRONIZATION_TYPE, ObexSynchronization))
#define OBEX_SYNCHRONIZATION_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), OBEX_SYNCHRONIZATION_TYPE))
#define OBEX_SYNCHRONIZATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), OBEX_SYNCHRONIZATION_TYPE, ObexSynchronizationClass))
#define OBEX_SYNCHRONIZATION_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), OBEX_SYNCHRONIZATION_TYPE))
#define OBEX_SYNCHRONIZATION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), OBEX_SYNCHRONIZATION_TYPE, ObexSynchronizationClass))

typedef struct _ObexSynchronization ObexSynchronization;
typedef struct _ObexSynchronizationClass ObexSynchronizationClass;
typedef struct _ObexSynchronizationPrivate ObexSynchronizationPrivate;

struct _ObexSynchronization {
	GObject parent_instance;

	/*< private >*/
	ObexSynchronizationPrivate *priv;
};

struct _ObexSynchronizationClass {
	GObjectClass parent_class;
};

/* used by OBEX_SYNCHRONIZATION_TYPE */
GType obex_synchronization_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ObexSynchronization *obex_synchronization_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *obex_synchronization_get_dbus_object_path(ObexSynchronization *self);

void obex_synchronization_set_location(ObexSynchronization *self, const gchar *location, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __OBEX_SYNCHRONIZATION_H */

