/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PHY_PATCH_RTL826X_H__
#define __PHY_PATCH_RTL826X_H__

#include <linux/phy.h>

int rtl826x_phy_patch_sds_get(struct phy_device *phydev, u32 sds_page, u32 sds_reg);
int rtl826x_phy_patch_sds_set(struct phy_device *phydev, u32 sds_page, u32 sds_reg,
			      u32 data);

int rtl8261n_phy_patch_db_init(struct phy_device *phydev);
int rtl8264b_phy_patch_db_init(struct phy_device *phydev);

#endif
