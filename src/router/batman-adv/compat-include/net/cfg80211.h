/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_
#define _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_

#include <linux/version.h>
#include_next <net/cfg80211.h>


#if LINUX_VERSION_IS_LESS(4, 18, 0) && IS_ENABLED(CONFIG_CFG80211)

/* cfg80211 fix: https://patchwork.kernel.org/patch/10449857/ */
static inline int batadv_cfg80211_get_station(struct net_device *dev,
					      const u8 *mac_addr,
					      struct station_info *sinfo)
{
	memset(sinfo, 0, sizeof(*sinfo));
	return cfg80211_get_station(dev, mac_addr, sinfo);
}

#define cfg80211_get_station(dev, mac_addr, sinfo) \
	batadv_cfg80211_get_station(dev, mac_addr, sinfo)

#endif /* LINUX_VERSION_IS_LESS(4, 18, 0) && IS_ENABLED(CONFIG_CFG80211) */


#if LINUX_VERSION_IS_LESS(4, 18, 0)

#define cfg80211_sinfo_release_content(sinfo)

#endif /* LINUX_VERSION_IS_LESS(4, 18, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_NET_CFG80211_H_ */
