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

#ifndef __MEDIA_PLAYER_H
#define __MEDIA_PLAYER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define MEDIA_PLAYER_DBUS_SERVICE "org.bluez"
#define MEDIA_PLAYER_DBUS_INTERFACE "org.bluez.MediaPlayer1"

/*
 * Type macros
 */
#define MEDIA_PLAYER_TYPE				(media_player_get_type())
#define MEDIA_PLAYER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), MEDIA_PLAYER_TYPE, MediaPlayer))
#define MEDIA_PLAYER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), MEDIA_PLAYER_TYPE))
#define MEDIA_PLAYER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), MEDIA_PLAYER_TYPE, MediaPlayerClass))
#define MEDIA_PLAYER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), MEDIA_PLAYER_TYPE))
#define MEDIA_PLAYER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), MEDIA_PLAYER_TYPE, MediaPlayerClass))

typedef struct _MediaPlayer MediaPlayer;
typedef struct _MediaPlayerClass MediaPlayerClass;
typedef struct _MediaPlayerPrivate MediaPlayerPrivate;

struct _MediaPlayer {
	GObject parent_instance;

	/*< private >*/
	MediaPlayerPrivate *priv;
};

struct _MediaPlayerClass {
	GObjectClass parent_class;
};

/* used by MEDIA_PLAYER_TYPE */
GType media_player_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
MediaPlayer *media_player_new(const gchar *dbus_object_path);

/*
 * Method definitions
 */
const gchar *media_player_get_dbus_object_path(MediaPlayer *self);

void media_player_fast_forward(MediaPlayer *self, GError **error);
void media_player_next(MediaPlayer *self, GError **error);
void media_player_pause(MediaPlayer *self, GError **error);
void media_player_play(MediaPlayer *self, GError **error);
void media_player_previous(MediaPlayer *self, GError **error);
void media_player_rewind(MediaPlayer *self, GError **error);
void media_player_stop(MediaPlayer *self, GError **error);

GVariant *media_player_get_properties(MediaPlayer *self, GError **error);
void media_player_set_property(MediaPlayer *self, const gchar *name, const GVariant *value, GError **error);

gboolean media_player_get_browsable(MediaPlayer *self, GError **error);
const gchar *media_player_get_device(MediaPlayer *self, GError **error);
const gchar *media_player_get_equalizer(MediaPlayer *self, GError **error);
void media_player_set_equalizer(MediaPlayer *self, const gchar *value, GError **error);
const gchar *media_player_get_name(MediaPlayer *self, GError **error);
guint32 media_player_get_position(MediaPlayer *self, GError **error);
const gchar *media_player_get_repeat(MediaPlayer *self, GError **error);
void media_player_set_repeat(MediaPlayer *self, const gchar *value, GError **error);
const gchar *media_player_get_scan(MediaPlayer *self, GError **error);
void media_player_set_scan(MediaPlayer *self, const gchar *value, GError **error);
gboolean media_player_get_searchable(MediaPlayer *self, GError **error);
const gchar *media_player_get_shuffle(MediaPlayer *self, GError **error);
void media_player_set_shuffle(MediaPlayer *self, const gchar *value, GError **error);
const gchar *media_player_get_status(MediaPlayer *self, GError **error);
const gchar *media_player_get_subtype(MediaPlayer *self, GError **error);
GVariant *media_player_get_track(MediaPlayer *self, GError **error);
// This has been renamed because 'media_player_get_type' is already used by GLib
const gchar *media_player_get_player_type(MediaPlayer *self, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __MEDIA_PLAYER_H */

