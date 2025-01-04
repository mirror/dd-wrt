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

#include "hostapd.h"
#include "common/ieee802_11_defs.h"

/** When beacons from other Access Point are received, if the SSID is matching
 * add them as APuP peers (aka WDS STA to our own AP) the same happens on the
 * peer when receiving our beacons */
void apup_process_beacon(struct hostapd_data *hapd,
              const struct ieee80211_mgmt *mgmt, size_t len,
              const struct ieee802_11_elems *elems );
