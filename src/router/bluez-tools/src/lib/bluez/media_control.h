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

#ifndef __MEDIA_CONTROL_H
#define __MEDIA_CONTROL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define MEDIA_CONTROL_DBUS_SERVICE "org.bluez"
#define MEDIA_CONTROL_DBUS_INTERFACE "org.bluez.MediaControl1"

/*
 * Type macros
 */
#define MEDIA_CONTROL_TYPE				(media_control_get_type())
#define MEDIA_CONTROL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), MEDIA_CONTROL_TYPE, MediaControl))
#define MEDIA_CONTROL_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), MEDIA_CONTROL_TYPE))
#define MEDIA_CONTROL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), MEDIA_CONTROL_TYPE, MediaControlClass))
#define MEDIA_CONTROL_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), MEDIA_CONTROL_TYPE))
#define MEDIA_CONTROL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), MEDIA_CONTROL_TYPE, MediaControlClass))

typedef struct _MediaControl MediaControl;
typedef struct _MediaControlClass MediaControlClass;
typedef struct _MediaControlPrivate MediaControlPrivate;

struct _MediaControl {
	GObject parent_instance;

	/*< private >*/
	MediaControlPrivate *priv;
};

struct _MediaControlClass {
	GObjectClass parent_class;
};

/* used by MEDIA_CONTROL_TYPE */
GType media_control_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
MediaControl *media_control_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *media_control_get_dbus_object_path(MediaControl *self);

void media_control_fast_forward(MediaControl *self, GError **error);
void media_control_next(MediaControl *self, GError **error);
void media_control_pause(MediaControl *self, GError **error);
void media_control_play(MediaControl *self, GError **error);
void media_control_previous(MediaControl *self, GError **error);
void media_control_rewind(MediaControl *self, GError **error);
void media_control_stop(MediaControl *self, GError **error);
void media_control_volume_down(MediaControl *self, GError **error);
void media_control_volume_up(MediaControl *self, GError **error);

GVariant *media_control_get_properties(MediaControl *self, GError **error);
void media_control_set_property(MediaControl *self, const gchar *name, const GVariant *value, GError **error);

gboolean media_control_get_connected(MediaControl *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __MEDIA_CONTROL_H */

