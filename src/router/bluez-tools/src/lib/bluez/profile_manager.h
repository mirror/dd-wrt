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

#ifndef __PROFILE_MANAGER_H
#define __PROFILE_MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

#define PROFILE_MANAGER_DBUS_SERVICE "org.bluez"
#define PROFILE_MANAGER_DBUS_INTERFACE "org.bluez.ProfileManager1"
#define PROFILE_MANAGER_DBUS_PATH "/org/bluez"

/*
 * Type macros
 */
#define PROFILE_MANAGER_TYPE				(profile_manager_get_type())
#define PROFILE_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), PROFILE_MANAGER_TYPE, ProfileManager))
#define PROFILE_MANAGER_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), PROFILE_MANAGER_TYPE))
#define PROFILE_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), PROFILE_MANAGER_TYPE, ProfileManagerClass))
#define PROFILE_MANAGER_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), PROFILE_MANAGER_TYPE))
#define PROFILE_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), PROFILE_MANAGER_TYPE, ProfileManagerClass))

typedef struct _ProfileManager ProfileManager;
typedef struct _ProfileManagerClass ProfileManagerClass;
typedef struct _ProfileManagerPrivate ProfileManagerPrivate;

struct _ProfileManager {
	GObject parent_instance;

	/*< private >*/
	ProfileManagerPrivate *priv;
};

struct _ProfileManagerClass {
	GObjectClass parent_class;
};

/* used by PROFILE_MANAGER_TYPE */
GType profile_manager_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
ProfileManager *profile_manager_new();

/*
 * Method definitions
 */
void profile_manager_register_profile(ProfileManager *self, const gchar *profile, const gchar *uuid, const GVariant *options, GError **error);
void profile_manager_unregister_profile(ProfileManager *self, const gchar *profile, GError **error);

#ifdef	__cplusplus
}
#endif

#endif /* __PROFILE_MANAGER_H */

