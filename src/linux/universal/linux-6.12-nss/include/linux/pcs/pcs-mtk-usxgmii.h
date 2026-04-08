/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_PCS_MTK_USXGMII_H
#define __LINUX_PCS_MTK_USXGMII_H

#include <linux/phylink.h>

/**
 * mtk_usxgmii_select_pcs() - Get MediaTek PCS instance
 * @np:		Pointer to device node indentifying a MediaTek USXGMII PCS
 * @mode:	Ethernet PHY interface mode
 *
 * Return PCS identified by a device node and the PHY interface mode in use
 *
 * Return:	Pointer to phylink PCS instance of NULL
 */
#if IS_ENABLED(CONFIG_PCS_MTK_USXGMII)
struct phylink_pcs *mtk_usxgmii_pcs_get(struct device *dev, struct device_node *np);
void mtk_usxgmii_pcs_put(struct phylink_pcs *pcs);
#else
static inline struct phylink_pcs *mtk_usxgmii_pcs_get(struct device *dev, struct device_node *np)
{
	return NULL;
}
static inline void mtk_usxgmii_pcs_put(struct phylink_pcs *pcs) { }
#endif /* IS_ENABLED(CONFIG_PCS_MTK_USXGMII) */

#endif /* __LINUX_PCS_MTK_USXGMII_H */
