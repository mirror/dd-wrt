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

#ifndef __MEDIA_H
#define __MEDIA_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define MEDIA_DBUS_SERVICE "org.bluez"
#define MEDIA_DBUS_INTERFACE "org.bluez.Media1"

/*
 * Type macros
 */
#define MEDIA_TYPE				(media_get_type())
#define MEDIA(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), MEDIA_TYPE, Media))
#define MEDIA_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), MEDIA_TYPE))
#define MEDIA_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), MEDIA_TYPE, MediaClass))
#define MEDIA_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), MEDIA_TYPE))
#define MEDIA_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), MEDIA_TYPE, MediaClass))

typedef struct _Media Media;
typedef struct _MediaClass MediaClass;
typedef struct _MediaPrivate MediaPrivate;

struct _Media {
	GObject parent_instance;

	/*< private >*/
	MediaPrivate *priv;
};

struct _MediaClass {
	GObjectClass parent_class;
};

/* used by MEDIA_TYPE */
GType media_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
Media *media_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *media_get_dbus_object_path(Media *self);

void media_register_endpoint(Media *self, const gchar *endpoint, const GVariant *properties, GError **error);
void media_register_player(Media *self, const gchar *player, const GVariant *properties, GError **error);
void media_unregister_endpoint(Media *self, const gchar *endpoint, GError **error);
void media_unregister_player(Media *self, const gchar *player, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __MEDIA_H */

