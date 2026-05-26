/* SPDX-License-Identifier: GPL-2.0 */

#ifndef REALTEK_H
#define REALTEK_H

#include <linux/phy.h>

#include "phy_patch.h"
#include "phy_patch_rtl826x.h"

struct rtl826x_priv {
	struct rtlgen_phy_patch_db *patch;
	bool enable_pma_low_power:1;
};

int rtl822x_hwmon_init(struct phy_device *phydev);

#endif /* REALTEK_H */
