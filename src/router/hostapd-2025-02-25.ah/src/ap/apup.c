/*
 * hostapd / APuP Access Point Micro Peering
 *
 * Copyright (C) 2023-2024  Gioacchino Mazzurco <gio@polymathes.cc>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

/* Be extremely careful altering include order, move just one in the wrong place
 * and you will start getting a bunch of error of undefined bool, size_t etc. */

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/os.h"

#include "apup.h"

#include "drivers/driver.h"
#include "wpa_auth.h"
#include "ap_mlme.h"
#include "ieee802_11.h"
#include "ap_drv_ops.h"
#include "sta_info.h"

#ifdef UBUS_SUPPORT
#	include "ubus.h"
#endif

#ifdef UCODE_SUPPORT
#	include "ucode.h"
#endif

void apup_process_beacon(struct hostapd_data *hapd,
              const struct ieee80211_mgmt *mgmt, size_t len,
              const struct ieee802_11_elems *elems )
{
	if (!os_memcmp(hapd->own_addr, mgmt->bssid, ETH_ALEN))
	{
		wpa_printf(MSG_WARNING,
		           "apup_process_beacon(...) own beacon elems.ssid %.*s",
		           (int) elems->ssid_len, elems->ssid);
		return;
	}

	if (elems->ssid_len != hapd->conf->ssid.ssid_len ||
	        os_memcmp(elems->ssid, hapd->conf->ssid.ssid, elems->ssid_len))
		return;

	struct sta_info* sta_ret = ap_get_sta(hapd, mgmt->bssid);
	if (sta_ret)
		return;

	sta_ret = ap_sta_add(hapd, mgmt->bssid);

	/* TODO: this has been added just to making compiler happy after breaking
	 * changes introduced in 11a607d121df512e010148bedcb4263a03329dc7 to support
	 * IEEE80211BE Multi Link Operation. Look at that commit with more time and
	 * understand what could be a proper implementation in this context too
	 */
	const u8 *mld_link_addr = NULL;
	bool mld_link_sta = false;

	/* First add the station without more information */
	int aRet = hostapd_sta_add(
	            hapd, mgmt->bssid, sta_ret->aid, 0,
	            NULL, 0, 0, NULL, NULL, NULL, 0, NULL, 0, NULL,
	            sta_ret->flags, 0, 0, 0,
	            0, // 0 add, 1 set
	            mld_link_addr, mld_link_sta, 0);

	sta_ret->flags |= WLAN_STA_AUTH;
	wpa_auth_sm_event(sta_ret->wpa_sm, WPA_AUTH);

	/* TODO: Investigate if supporting WPA or other encryption method is
	 * possible */
	sta_ret->auth_alg = WLAN_AUTH_OPEN;
	mlme_authenticate_indication(hapd, sta_ret);

	sta_ret->capability = le_to_host16(mgmt->u.beacon.capab_info);

	if (sta_ret->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		sta_ret->flags |= WLAN_STA_SHORT_PREAMBLE;
	else
		sta_ret->flags &= ~WLAN_STA_SHORT_PREAMBLE;

	hostapd_copy_supp_rates(hapd, sta_ret, elems);

	/* Whithout this flag copy_sta_[v]ht_capab will disable [V]HT
	 * capabilities even if available */
	if (elems->ht_capabilities || elems->vht_capabilities)
		sta_ret->flags |= WLAN_STA_WMM;

	copy_sta_ht_capab(hapd, sta_ret, elems->ht_capabilities);
#ifdef CONFIG_IEEE80211AC
	copy_sta_vht_capab(hapd, sta_ret, elems->vht_capabilities);
	copy_sta_vht_oper(hapd, sta_ret, elems->vht_operation);
	copy_sta_vendor_vht(hapd, sta_ret, elems->vendor_vht, elems->vendor_vht_len);
#endif // def CONFIG_IEEE80211AC
#ifdef CONFIG_IEEE80211AX
	copy_sta_he_capab(hapd, sta_ret, IEEE80211_MODE_AP,
	                  elems->he_capabilities, elems->he_capabilities_len);
	copy_sta_he_6ghz_capab(hapd, sta_ret,  elems->he_6ghz_band_cap);
#endif // def CONFIG_IEEE80211AX
#ifdef CONFIG_IEEE80211BE
	copy_sta_eht_capab(hapd, sta_ret,
	                   IEEE80211_MODE_AP, // TODO: Make sure is the right value
	                   elems->he_capabilities, elems->he_capabilities_len,
	                   elems->eht_capabilities, elems->eht_capabilities_len);
#endif //def CONFIG_IEEE80211BE

	update_ht_state(hapd, sta_ret);

	if (hostapd_get_aid(hapd, sta_ret) < 0)
	{
		wpa_printf(MSG_INFO, "apup_process_beacon(...) No room for more AIDs");
		return;
	}

	sta_ret->flags |= WLAN_STA_ASSOC_REQ_OK;

	/* Make sure that the previously registered inactivity timer will not
	 * remove the STA immediately. */
	sta_ret->timeout_next = STA_NULLFUNC;

	sta_ret->flags |= WLAN_STA_AUTH | WLAN_STA_ASSOC;

	/* Then set the paramethers */
	int sRet = hostapd_sta_add(
	            hapd, mgmt->bssid, sta_ret->aid,
	            sta_ret->capability,
	            sta_ret->supported_rates, sta_ret->supported_rates_len,
	            0, // u16 listen_interval TODO ?
	            sta_ret->ht_capabilities,
	            sta_ret->vht_capabilities,
	            sta_ret->he_capab, sta_ret->he_capab_len,
	            sta_ret->eht_capab, sta_ret->eht_capab_len,
	            sta_ret->he_6ghz_capab,
	            sta_ret->flags,
	            0, // u8 qosinfo
	            sta_ret->vht_opmode,
	            0, // int supp_p2p_ps
	            1, // 0 add, 1 set
	            mld_link_addr, mld_link_sta, 0);

	ap_sta_set_authorized(hapd, sta_ret, 1);
	hostapd_set_sta_flags(hapd, sta_ret);

	char mIfname[IFNAMSIZ + 1];
	os_memset(mIfname, 0, IFNAMSIZ + 1);

	// last param 1 means add 0 means remove
	int mRet = hostapd_set_wds_sta(
	            hapd, mIfname, mgmt->bssid, sta_ret->aid, 1);

	wpa_printf(MSG_INFO,
	           "apup_process_beacon(...) Added APuP peer at %s with flags: %d,"
	           " capabilities %d",
	           mIfname, sta_ret->flags, sta_ret->capability);

#ifdef UBUS_SUPPORT
	hostapd_ubus_notify_apup_newpeer(hapd, mgmt->bssid, mIfname);
#endif

#ifdef UCODE_SUPPORT
	hostapd_ucode_apup_newpeer(hapd, mIfname);
#endif
}
