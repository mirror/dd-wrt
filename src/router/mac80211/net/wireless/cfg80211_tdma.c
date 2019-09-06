/*
 * Some TDMA support code for cfg80211.
 *
 * Copyright 2011-2013	Stanislav V. Korsakov <sta@stasoft.net>
 */

#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/cfg80211.h>
#include "wext-compat.h"
#include "nl80211.h"
#include "rdev-ops.h"

int __cfg80211_join_tdma(struct cfg80211_registered_device *rdev,
			 struct net_device *dev,
			 struct cfg80211_tdma_params *params,
			 struct cfg80211_cached_keys *connkeys)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	int err;

	ASSERT_WDEV_LOCK(wdev);

	if (wdev->ssid_len)
		return -EALREADY;

	if (!params->rate)
		return -EINVAL;

	if (WARN_ON(wdev->connect_keys))
		kfree(wdev->connect_keys);
	wdev->connect_keys = connkeys;
	wdev->chandef = params->chandef;
	err = rdev->ops->join_tdma(&rdev->wiphy, dev, params);
	if (err) {
		wdev->connect_keys = NULL;
		return err;
	}

	cfg80211_upload_connect_keys(wdev);

	memcpy(wdev->ssid, params->ssid.ssid, params->ssid.ssid_len);
	wdev->ssid_len = params->ssid.ssid_len;
	if ( wdev->ssid_len < 1)
		wdev->ssid_len = 1;

	return 0;
}

static void __cfg80211_clear_tdma(struct net_device *dev, bool nowext)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev = wiphy_to_rdev(wdev->wiphy);
	int i;

	ASSERT_WDEV_LOCK(wdev);

	kfree(wdev->connect_keys);
	wdev->connect_keys = NULL;

	rdev_set_qos_map(rdev, dev, NULL);

	/*
	 * Delete all the keys ... pairwise keys can't really
	 * exist any more anyway, but default keys might.
	 */
	if (rdev->ops->del_key)
		for (i = 0; i < 6; i++)
			rdev_del_key(rdev, dev, i, false, NULL);

	if (wdev->current_bss) {
		cfg80211_unhold_bss(wdev->current_bss);
		cfg80211_put_bss(wdev->wiphy, &wdev->current_bss->pub);
	}

	wdev->current_bss = NULL;
	wdev->ssid_len = 0;
	memset(&wdev->chandef, 0, sizeof(wdev->chandef));
}

void cfg80211_clear_tdma(struct net_device *dev, bool nowext)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;

	wdev_lock(wdev);
	__cfg80211_clear_tdma(dev, nowext);
	wdev_unlock(wdev);
}

int cfg80211_join_tdma(struct cfg80211_registered_device *rdev,
		       struct net_device *dev,
		       struct cfg80211_tdma_params *params,
		       struct cfg80211_cached_keys *connkeys)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	int err;

	wdev_lock(wdev);
	err = __cfg80211_join_tdma(rdev, dev, params, connkeys);
	wdev_unlock(wdev);

	return err;
}

int __cfg80211_leave_tdma(struct cfg80211_registered_device *rdev,
			  struct net_device *dev, bool nowext)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	int err;

	ASSERT_WDEV_LOCK(wdev);

	if (!wdev->ssid_len)
		return -ENOLINK;

	err = rdev->ops->leave_tdma(&rdev->wiphy, dev);

	if (err)
		return err;

	__cfg80211_clear_tdma(dev, nowext);
	return 0;
}

int cfg80211_leave_tdma(struct cfg80211_registered_device *rdev,
			struct net_device *dev, bool nowext)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	int err;

	ASSERT_WDEV_LOCK(wdev);
	wdev_lock(wdev);
	err = __cfg80211_leave_tdma(rdev, dev, nowext);
	wdev_unlock(wdev);

	return err;
}
