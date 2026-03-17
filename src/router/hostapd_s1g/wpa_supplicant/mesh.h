/*
 * WPA Supplicant - Basic mesh mode routines
 * Copyright (c) 2013-2014, cozybit, Inc.  All rights reserved.
 * Copyright 2023 Morse Micro
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MESH_H
#define MESH_H

int wpa_supplicant_join_mesh(struct wpa_supplicant *wpa_s,
			     struct wpa_ssid *ssid);
int wpa_supplicant_leave_mesh(struct wpa_supplicant *wpa_s,
			      bool need_deinit);
void wpa_supplicant_mesh_iface_deinit(struct wpa_supplicant *wpa_s,
				      struct hostapd_iface *ifmsh,
				      bool also_clear_hostapd);
int wpas_mesh_scan_result_text(const u8 *ies, size_t ies_len, char *buf,
			       char *end);
int wpas_mesh_add_interface(struct wpa_supplicant *wpa_s, char *ifname,
			    size_t len);
int wpas_mesh_peer_remove(struct wpa_supplicant *wpa_s, const u8 *addr);
int wpas_mesh_peer_add(struct wpa_supplicant *wpa_s, const u8 *addr,
		       int duration);

#ifdef CONFIG_MESH

void wpa_mesh_notify_peer(struct wpa_supplicant *wpa_s, const u8 *addr,
			  const u8 *ies, size_t ie_len);
void wpa_supplicant_mesh_add_scan_ie(struct wpa_supplicant *wpa_s,
				     struct wpabuf **extra_ie);
/**
 * mesh_iface_wpa_get_status - Get mesh WPA status
 * @wpa_s: Pointer to wpa supplicant
 * @buf: Buffer for status information
 * @buflen: Maximum buffer length
 * Returns: Number of bytes written to buf
 *
 * Query Mesh module for status information. This function fills in
 * a text area with current status information.
 */
int mesh_iface_wpa_get_status(struct wpa_supplicant *wpa_s, char *buf, size_t buflen);

#else /* CONFIG_MESH */

static inline void wpa_mesh_notify_peer(struct wpa_supplicant *wpa_s,
					const u8 *addr,
					const u8 *ies, size_t ie_len)
{
}

static inline void wpa_supplicant_mesh_add_scan_ie(struct wpa_supplicant *wpa_s,
						   struct wpabuf **extra_ie)
{
}

static inline int mesh_iface_wpa_get_status(struct wpa_supplicant *wpa_s,
		char *buf, size_t buflen)
{
	return 0;
}
#endif /* CONFIG_MESH */

#endif /* MESH_H */
