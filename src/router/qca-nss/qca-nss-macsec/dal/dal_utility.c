/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/phy.h>
#include "nss_macsec_types.h"
#include "nss_macsec.h"
#include "dal_utility.h"


#define PHY_REG_MAX_LEN  16



static g_error_t phy_mmd_reg_write(struct phy_device *phydev, enum access_layer_t phy_layer,
	u32 reg_addr, u16 val16)
{
	int ret  = 0;
	int devad = 0;

	if (phy_layer == PHY_MMD3)
		devad = MDIO_MMD_PCS;
	else if (phy_layer == PHY_MMD7)
		devad = MDIO_MMD_AN;
	else
		return ERROR_NOT_SUPPORT;

	ret = phy_write_mmd(phydev, devad, reg_addr, val16);
	if (unlikely(ret))
		return ERROR;

	return OK;
}

static g_error_t phy_mmd_reg_read(struct phy_device *phydev, enum access_layer_t phy_layer,
	u32 reg_addr, u16 *p_val16)
{
	int val  = 0;
	int devad = 0;

	if (phy_layer == PHY_MMD3)
		devad = MDIO_MMD_PCS;
	else if (phy_layer == PHY_MMD7)
		devad = MDIO_MMD_AN;
	else
		return ERROR_NOT_SUPPORT;

	val = phy_read_mmd(phydev, devad, reg_addr);
	if (unlikely(val < 0))
		return ERROR;

	*p_val16 = val;
	return OK;
}

g_error_t reg_access_write(struct macsec_port *port, enum access_layer_t reg_layer,
	u32 reg_addr, u16 val16)
{
	g_error_t ret  = OK;

	if (port->type == MACSEC_IN_PHY)
		ret = phy_mmd_reg_write(port->phydev, reg_layer, reg_addr, val16);
	else
		return ERROR_NOT_SUPPORT;

	return ret;
}

g_error_t reg_access_read(struct macsec_port *port, enum access_layer_t reg_layer,
	u32 reg_addr, u16 *p_val16)
{
	g_error_t ret  = OK;

	if (port->type == MACSEC_IN_PHY)
		ret = phy_mmd_reg_read(port->phydev, reg_layer, reg_addr, p_val16);
	else
		return ERROR_NOT_SUPPORT;

	return ret;
}

g_error_t reg_acces_field_write(struct macsec_port *port, enum access_layer_t reg_layer,
	u32 reg_addr, u8 offset, u8 length, u16 val16)
{
	g_error_t ret  = OK;
	u16  data = 0;
	u16  data16 = 0;

	if (length == 0
		|| offset >= PHY_REG_MAX_LEN
		|| length > (PHY_REG_MAX_LEN - offset))
		return ERROR_PARAM;

	ret = reg_access_read(port, reg_layer, reg_addr, &data);
	if (unlikely(ret != OK))
		return ret;

	val16 &= ((0x1UL << length) - 1);
	data16 = (data & ~(((0x1UL << length) - 1) << offset)) |
		(val16 << offset);

	if ((data & 0xffff) == data16)
		return ret;

	ret = reg_access_write(port, reg_layer, reg_addr, data16);
	return ret;
}

g_error_t reg_acces_field_read(struct macsec_port *port, enum access_layer_t reg_layer,
	u32 reg_addr, u8 offset, u8 length, u16 *p_val16)
{
	g_error_t ret = OK;
	u16 data = 0;

	if (length == 0 || offset >= PHY_REG_MAX_LEN
		|| length > (PHY_REG_MAX_LEN - offset)
		|| !p_val16)
		return ERROR_PARAM;

	ret = reg_access_read(port, reg_layer, reg_addr, &data);
	if (unlikely(ret != OK))
		return ret;

	if ((offset == 0) && (length == 16))
		*p_val16 = data & 0xffff;
	else
		*p_val16 = (data >> offset) & ((0x1UL << length) - 1);

	return ret;
}

g_error_t reg_access_mask_write(struct macsec_port *port, enum access_layer_t reg_layer,
	u32 reg_addr, u16 mask, u16 val16)
{
	g_error_t ret = OK;
	u16 data = 0;
	u16 data16 = val16;

	if (mask != 0xffff) {
		ret = reg_access_read(port, reg_layer, reg_addr, &data);
		if (unlikely(ret != OK))
			return ret;

		data &= ~mask;
		data16 = (data & 0xffff) | (val16 & mask);
	}

	ret = reg_access_write(port, reg_layer, reg_addr, data16);

	return ret;
}


