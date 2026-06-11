/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "hsl_phy.h"
#include "qca808x.h"
#include "qcaphy_c45_common.h"
#include "sfp_phy.h"

#define PHY_INVALID_DATA            0xffff

LIST_HEAD(g_qca808x_phy_list);
DEFINE_MUTEX(qca808x_phy_list_mutex);

struct qca808x_phy_info* qca808x_phy_info_get(a_uint32_t phy_addr)
{
	struct qca808x_phy_info *pnode, *pdata = NULL;

	mutex_lock(&qca808x_phy_list_mutex);
	list_for_each_entry(pnode, &g_qca808x_phy_list, list) {
		if (pnode->phy_addr == phy_addr) {
			pdata = pnode;
			break;
		}
	}
	mutex_unlock(&qca808x_phy_list_mutex);

	return pdata;
}

int qca808x_phy_info_add(struct qca808x_phy_info *pinfo)
{
	if (!pinfo)
		return 0;

	mutex_lock(&qca808x_phy_list_mutex);
	list_add_tail(&pinfo->list, &g_qca808x_phy_list);
	mutex_unlock(&qca808x_phy_list_mutex);

	return 0;
}

int qca808x_phy_info_remove(struct qca808x_phy_info *pinfo)
{
	struct qca808x_phy_info *pnode;

	if (!pinfo)
		return 0;

	mutex_lock(&qca808x_phy_list_mutex);
	list_for_each_entry(pnode, &g_qca808x_phy_list, list) {
		if (pnode->phy_addr == pinfo->phy_addr) {
			list_del(&pnode->list);
			break;
		}
	}
	mutex_unlock(&qca808x_phy_list_mutex);

	return 0;
}

void qca808x_phydev_init(a_uint32_t dev_id, a_uint32_t port_id)
{
	struct qca808x_phy_info *pdata = NULL;
	struct phy_device *phydev = NULL;
	qca808x_priv *priv = NULL;
	int phy_addr = 0;

	phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
#if defined(IN_PHY_I2C_MODE)
	/* in i2c mode, need to register a fake phy device
	 * before the phy driver register */
	if (hsl_port_phy_access_type_get(dev_id, port_id) == PHY_I2C_ACCESS) {
		a_uint32_t phy_id = INVALID_PHY_ID;

		if (hsl_port_feature_get(dev_id, port_id, PHY_F_CLAUSE45))
			qcaphy_c45_get_phy_id(dev_id, phy_addr, &phy_id);
		else
			qcaphy_get_phy_id(dev_id, phy_addr, &phy_id);

		if (phy_id != QCA8081_PHY_V1_1 &&
		    phy_id != QCA8111_PHY &&
		    phy_id != INVALID_PHY_ID) {
			SSDK_ERROR("phy id 0x%x is not supported\n", phy_id);
			return;
		}

		sfp_phy_device_setup(dev_id, port_id, phy_id, priv);
	}
#endif
	hsl_port_phydev_get(dev_id, port_id, &phydev);
	if (!phydev) {
		pr_err("phydev of port % does not exist\n", port_id);
		return;
	}

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		phydev_err(phydev, "allocate phy info of port %d failed\n", port_id);
		return;
	}

	pdata->dev_id = dev_id;
	/* the phy address may be the i2c slave addr or mdio addr */
	pdata->phy_addr = phy_addr;
	pdata->phydev_addr = TO_PHY_ADDR(pdata->phy_addr);
	qca808x_phy_info_add(pdata);

	/* Update MDIO fake address for I2C mode. */
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port_id) == PHY_I2C_ACCESS)
		pdata->phydev_addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port_id);
#endif

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		phydev_err(phydev, "allocate phy private of port %d failed\n", port_id);
		return;
	}

	priv->phydev = phydev;
	priv->phy_info = pdata;
	phydev->priv = priv;
}

void qca808x_phydev_deinit(a_uint32_t dev_id, a_uint32_t port_id)
{
	struct phy_device *phydev = NULL;
	qca808x_priv *priv = NULL;

	hsl_port_phydev_get(dev_id, port_id, &phydev);
	if (phydev) {
		priv = phydev->priv;
		qca808x_phy_info_remove(priv->phy_info);
	}

#if defined(IN_PHY_I2C_MODE)
	/* in i2c mode, need to remove the fake phy device
	 * after the phy driver unregistered */
	if (hsl_port_phy_access_type_get(dev_id, port_id) == PHY_I2C_ACCESS)
		sfp_phy_device_remove(dev_id, port_id);
#endif

	if (priv && priv->phy_info) {
		kfree(priv->phy_info);
		priv->phy_info = NULL;
	}

	if (priv) {
		kfree(priv);
		priv = NULL;
	}
}
