/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_PCS_MTK_LYNXI_H
#define __LINUX_PCS_MTK_LYNXI_H

#include <linux/phylink.h>
#include <linux/regmap.h>

#define MTK_SGMII_FLAG_PN_SWAP BIT(0)
struct phylink_pcs *mtk_pcs_lynxi_create(struct device *dev,
					 struct regmap *regmap,
					 u32 ana_rgc3, u32 flags);
void mtk_pcs_lynxi_destroy(struct phylink_pcs *pcs);

#if IS_ENABLED(CONFIG_PCS_MTK_LYNXI)
struct phylink_pcs *mtk_pcs_lynxi_get(struct device *dev, struct device_node *np);
void mtk_pcs_lynxi_put(struct phylink_pcs *pcs);
#else
static inline struct phylink_pcs *mtk_pcs_lynxi_get(struct device *dev, struct device_node *np)
{
	return NULL;
}
static inline void mtk_pcs_lynxi_put(struct phylink_pcs *pcs) { }
#endif /* IS_ENABLED(CONFIG_PCS_MTK_LYNXI) */
#endif
