/*
 * Copyright 2022-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <linux/inetdevice.h>
#include <linux/in.h>

#include "morse.h"
#include "debug.h"
#include "mac.h"
#include "offload.h"
#include "wiphy.h"

#define DHCP_OFFLOAD_MAX_CMD_SIZE			(256)

static struct wireless_dev *morse_get_sta_wdev(struct morse *mors)
{
	struct morse_vif *mors_vif;

	if (is_fullmac_mode()) {
		mors_vif = morse_wiphy_get_sta_vif(mors);
		if (!mors_vif)
			return NULL;
		return &mors_vif->wdev;
	}

	return ieee80211_vif_to_wdev(morse_get_sta_vif(mors));
}

static int append_ip_addr_to_string(char *str, int n, __le32 ip)
{
	u8 *ptr = (u8 *)&ip;

	return snprintf(str, n, "%d.%d.%d.%d ", ptr[0], ptr[1], ptr[2], ptr[3]);
}

int morse_offload_dhcpc_set_address(struct morse *mors, struct morse_cmd_evt_dhcp_lease_update *evt)
{
	int ret = 0;
	static const char *const envp[] = { "HOME=/", NULL };
	char cmd[DHCP_OFFLOAD_MAX_CMD_SIZE];
	struct net_device *ndev;
	char *const argv[] = { "/bin/sh", "-c", cmd, NULL };
	int idx = 0;

	struct wireless_dev *wdev = morse_get_sta_wdev(mors);

	if (!wdev || !wdev->netdev)
		return -1;

	ndev = wdev->netdev;

	ret = snprintf(cmd, sizeof(cmd), "%s %s ",
		       mors->custom_configs.dhcpc_lease_update_script, ndev->name);
	if (ret < 0 || ret >= (sizeof(cmd) - idx))
		goto exit;
	idx = ret;

	/* The lease update script assumes the ordering of these fields are consistent. Any updates
	 * to the format here MUST be reflected in the dhcpc update script.
	 */

	ret = append_ip_addr_to_string(cmd + idx, sizeof(cmd) - idx, evt->my_ip);
	if (ret < 0 || ret >= (sizeof(cmd) - idx))
		goto exit;
	idx += ret;

	ret = append_ip_addr_to_string(cmd + idx, sizeof(cmd) - idx, evt->netmask);
	if (ret < 0 || ret >= (sizeof(cmd) - idx))
		goto exit;
	idx += ret;

	ret = append_ip_addr_to_string(cmd + idx, sizeof(cmd) - idx, evt->router);
	if (ret < 0 || ret >= (sizeof(cmd) - idx))
		goto exit;
	idx += ret;

	ret = append_ip_addr_to_string(cmd + idx, sizeof(cmd) - idx, evt->dns);
	if (ret < 0 || ret >= (sizeof(cmd) - idx))
		goto exit;
	idx += ret;

	/* WAIT_EXEC as WAIT_PROC may cause a deadlock when used with ARP offload. */
	ret = call_usermodehelper(argv[0], (char **)argv, (char **)envp, UMH_WAIT_EXEC);

exit:
	if (ret) {
		if (ret >= (sizeof(cmd) - idx))
			MORSE_INFO(mors, "%s: Command truncated - %s", __func__, cmd);
		else
			MORSE_INFO(mors,
				   "%s: Calling DHCP update script failed (errno=%d) (script_path=%s)\n",
				   __func__, ret, mors->custom_configs.dhcpc_lease_update_script);
	} else {
		MORSE_DBG(mors, "%s: DHCP script called\n", __func__);
	}
	return ret;
}
