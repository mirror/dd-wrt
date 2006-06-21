/*
 * WPA Supplicant / dbus-based control interface
 * Copyright (c) 2006, Dan Williams <dcbw@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef CTRL_IFACE_DBUS_H
#define CTRL_IFACE_DBUS_H

#ifdef CONFIG_CTRL_IFACE_DBUS

#define WPA_SUPPLICANT_DBUS_SERVICE	"fi.epitest.hostap.WPASupplicant"
#define WPA_SUPPLICANT_DBUS_PATH	"/fi/epitest/hostap/WPASupplicant"
#define WPA_SUPPLICANT_DBUS_INTERFACE	"fi.epitest.hostap.WPASupplicant"


struct ctrl_iface_dbus_priv *
wpa_supplicant_dbus_ctrl_iface_init(struct wpa_global *global);
void wpa_supplicant_dbus_ctrl_iface_deinit(struct ctrl_iface_dbus_priv *iface);

#else /* CONFIG_CTRL_IFACE_DBUS */

static inline struct ctrl_iface_dbus_priv *
wpa_supplicant_dbus_ctrl_iface_init(struct wpa_global *global)
{
	return (struct ctrl_iface_dbus_priv *) 1;
}

static inline void
wpa_supplicant_dbus_ctrl_iface_deinit(struct ctrl_iface_dbus_priv *iface)
{
}

#endif /* CONFIG_CTRL_IFACE_DBUS */

#endif /* CTRL_IFACE_DBUS_H */
