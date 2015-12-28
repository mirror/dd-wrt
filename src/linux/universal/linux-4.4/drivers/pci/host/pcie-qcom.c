/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "pcie-designware.h"

/* DBI registers */
#define PCIE20_CAP				0x70
#define PCIE20_CAP_LINKCTRLSTATUS		(PCIE20_CAP + 0x10)

#define PCIE20_AXI_MSTR_RESP_COMP_CTRL0		0x818
#define PCIE20_AXI_MSTR_RESP_COMP_CTRL1		0x81c

#define PCIE20_PLR_IATU_VIEWPORT		0x900
#define PCIE20_PLR_IATU_REGION_OUTBOUND		(0x0 << 31)
#define PCIE20_PLR_IATU_REGION_INDEX(x)		(x << 0)

#define PCIE20_PLR_IATU_CTRL1			0x904
#define PCIE20_PLR_IATU_TYPE_CFG0		(0x4 << 0)
#define PCIE20_PLR_IATU_TYPE_MEM		(0x0 << 0)

#define PCIE20_PLR_IATU_CTRL2			0x908
#define PCIE20_PLR_IATU_ENABLE			BIT(31)

#define PCIE20_PLR_IATU_LBAR			0x90C
#define PCIE20_PLR_IATU_UBAR			0x910
#define PCIE20_PLR_IATU_LAR			0x914
#define PCIE20_PLR_IATU_LTAR			0x918
#define PCIE20_PLR_IATU_UTAR			0x91c

#define MSM_PCIE_DEV_CFG_ADDR			0x01000000

/* PARF registers */
#define PCIE20_PARF_PCS_DEEMPH			0x34
#define PCS_DEEMPH_TX_DEEMPH_GEN1(x)		(x << 16)
#define PCS_DEEMPH_TX_DEEMPH_GEN2_3_5DB(x)	(x << 8)
#define PCS_DEEMPH_TX_DEEMPH_GEN2_6DB(x)	(x << 0)

#define PCIE20_PARF_PCS_SWING			0x38
#define PCS_SWING_TX_SWING_FULL(x)		(x << 8)
#define PCS_SWING_TX_SWING_LOW(x)		(x << 0)

#define PCIE20_PARF_PHY_CTRL			0x40
#define PHY_CTRL_PHY_TX0_TERM_OFFSET_MASK	(0x1f << 16)
#define PHY_CTRL_PHY_TX0_TERM_OFFSET(x)		(x << 16)

#define PCIE20_PARF_PHY_REFCLK			0x4C
#define REF_SSP_EN				BIT(16)
#define REF_USE_PAD				BIT(12)

#define PCIE20_PARF_CONFIG_BITS			0x50
#define PHY_RX0_EQ(x)				(x << 24)

#define PCIE20_PARF_DBI_BASE_ADDR		0x168
#define PCIE20_PARF_SLV_ADDR_SPACE_SIZE		0x16c
#define PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT	0x178

#define PCIE20_ELBI_SYS_CTRL			0x04
#define PCIE20_ELBI_SYS_STTS			0x08
#define XMLH_LINK_UP				BIT(10)

#define PERST_DELAY_MIN_US			1000
#define PERST_DELAY_MAX_US			1005

#define LINKUP_DELAY_MIN_US			5000
#define LINKUP_DELAY_MAX_US			5100
#define LINKUP_RETRIES_COUNT			20

#define PCIE_V0					0	/* apq8064 */
#define PCIE_V1					1	/* apq8084 */

struct qcom_pcie_resources_v0 {
	struct clk *iface_clk;
	struct clk *core_clk;
	struct clk *phy_clk;
	struct clk *aux_clk;
	struct clk *ref_clk;
	struct reset_control *pci_reset;
	struct reset_control *axi_reset;
	struct reset_control *ahb_reset;
	struct reset_control *por_reset;
	struct reset_control *phy_reset;
	struct reset_control *ext_reset;
	struct regulator *vdda;
	struct regulator *vdda_phy;
	struct regulator *vdda_refclk;
	uint8_t phy_tx0_term_offset;
};

struct qcom_pcie_resources_v1 {
	struct clk *iface;
	struct clk *aux;
	struct clk *master_bus;
	struct clk *slave_bus;
	struct reset_control *core;
	struct regulator *vdda;
};

union pcie_resources {
	struct qcom_pcie_resources_v0 v0;
	struct qcom_pcie_resources_v1 v1;
};

struct qcom_pcie {
	struct pcie_port pp;
	struct device *dev;
	union pcie_resources res;
	void __iomem *parf;
	void __iomem *dbi;
	void __iomem *elbi;
	struct phy *phy;
	struct gpio_desc *reset;
	unsigned int version;
};

#define to_qcom_pcie(x)		container_of(x, struct qcom_pcie, pp)

static inline void
writel_masked(void __iomem *addr, u32 clear_mask, u32 set_mask)
{
	u32 val = readl(addr);

	val &= ~clear_mask;
	val |= set_mask;
	writel(val, addr);
}

static void qcom_ep_reset_assert_deassert(struct qcom_pcie *pcie, int assert)
{
	if (IS_ERR_OR_NULL(pcie->reset))
		return;

	gpiod_set_value(pcie->reset, assert);
	usleep_range(PERST_DELAY_MIN_US, PERST_DELAY_MAX_US);
}

static void qcom_ep_reset_assert(struct qcom_pcie *pcie)
{
	qcom_ep_reset_assert_deassert(pcie, 1);
}

static void qcom_ep_reset_deassert(struct qcom_pcie *pcie)
{
	qcom_ep_reset_assert_deassert(pcie, 0);
}

static irqreturn_t qcom_pcie_msi_irq_handler(int irq, void *arg)
{
	struct pcie_port *pp = arg;

	return dw_handle_msi_irq(pp);
}

static int qcom_pcie_link_up(struct pcie_port *pp)
{
	struct qcom_pcie *pcie = to_qcom_pcie(pp);
	u32 val = readl(pcie->dbi + PCIE20_CAP_LINKCTRLSTATUS);

	return val & BIT(29) ? 1 : 0;
}

static void qcom_pcie_disable_resources_v0(struct qcom_pcie *pcie)
{
	struct qcom_pcie_resources_v0 *res = &pcie->res.v0;

	reset_control_assert(res->pci_reset);
	reset_control_assert(res->axi_reset);
	reset_control_assert(res->ahb_reset);
	reset_control_assert(res->por_reset);
	reset_control_assert(res->phy_reset);
	reset_control_assert(res->ext_reset);
	clk_disable_unprepare(res->iface_clk);
	clk_disable_unprepare(res->core_clk);
	clk_disable_unprepare(res->phy_clk);
	clk_disable_unprepare(res->aux_clk);
	clk_disable_unprepare(res->ref_clk);
	regulator_disable(res->vdda);
	regulator_disable(res->vdda_phy);
	regulator_disable(res->vdda_refclk);
}

static void qcom_pcie_disable_resources_v1(struct qcom_pcie *pcie)
{
	struct qcom_pcie_resources_v1 *res = &pcie->res.v1;

	reset_control_assert(res->core);
	clk_disable_unprepare(res->slave_bus);
	clk_disable_unprepare(res->master_bus);
	clk_disable_unprepare(res->iface);
	clk_disable_unprepare(res->aux);
	regulator_disable(res->vdda);
}

static int qcom_pcie_enable_resources_v0(struct qcom_pcie *pcie)
{
	struct qcom_pcie_resources_v0 *res = &pcie->res.v0;
	struct device *dev = pcie->dev;
	int ret;

	ret = regulator_enable(res->vdda);
	if (ret) {
		dev_err(dev, "cannot enable vdda regulator\n");
		return ret;
	}

	ret = regulator_enable(res->vdda_refclk);
	if (ret) {
		dev_err(dev, "cannot enable vdda_refclk regulator\n");
		goto err_refclk;
	}

	ret = regulator_enable(res->vdda_phy);
	if (ret) {
		dev_err(dev, "cannot enable vdda_phy regulator\n");
		goto err_vdda_phy;
	}

	ret = reset_control_deassert(res->ext_reset);
	if (ret) {
		dev_err(dev, "cannot assert ext reset\n");
		goto err_reset_ext;
	}

	ret = clk_prepare_enable(res->iface_clk);
	if (ret) {
		dev_err(dev, "cannot prepare/enable iface clock\n");
		goto err_iface;
	}

	ret = clk_prepare_enable(res->core_clk);
	if (ret) {
		dev_err(dev, "cannot prepare/enable core clock\n");
		goto err_clk_core;
	}

	ret = clk_prepare_enable(res->phy_clk);
	if (ret) {
		dev_err(dev, "cannot prepare/enable phy clock\n");
		goto err_clk_phy;
	}

	ret = clk_prepare_enable(res->aux_clk);
	if (ret) {
		dev_err(dev, "cannot prepare/enable aux clock\n");
		goto err_clk_aux;
	}

	ret = clk_prepare_enable(res->ref_clk);
	if (ret) {
		dev_err(dev, "cannot prepare/enable ref clock\n");
		goto err_clk_ref;
	}

	ret = reset_control_deassert(res->ahb_reset);
	if (ret) {
		dev_err(dev, "cannot deassert ahb reset\n");
		goto err_reset_ahb;
	}
	udelay(1);

	return 0;

err_reset_ahb:
	clk_disable_unprepare(res->ref_clk);
err_clk_ref:
	clk_disable_unprepare(res->aux_clk);
err_clk_aux:
	clk_disable_unprepare(res->phy_clk);
err_clk_phy:
	clk_disable_unprepare(res->core_clk);
err_clk_core:
	clk_disable_unprepare(res->iface_clk);
err_iface:
	reset_control_assert(res->ext_reset);
err_reset_ext:
	regulator_disable(res->vdda_phy);
err_vdda_phy:
	regulator_disable(res->vdda_refclk);
err_refclk:
	regulator_disable(res->vdda);
	return ret;
}

static int qcom_pcie_enable_resources_v1(struct qcom_pcie *pcie)
{
	struct qcom_pcie_resources_v1 *res = &pcie->res.v1;
	struct device *dev = pcie->dev;
	int ret;

	ret = reset_control_deassert(res->core);
	if (ret) {
		dev_err(dev, "cannot deassert core reset\n");
		return ret;
	}

	ret = clk_prepare_enable(res->aux);
	if (ret) {
		dev_err(dev, "cannot prepare/enable aux clock\n");
		goto err_res;
	}

	ret = clk_prepare_enable(res->iface);
	if (ret) {
		dev_err(dev, "cannot prepare/enable iface clock\n");
		goto err_aux;
	}

	ret = clk_prepare_enable(res->master_bus);
	if (ret) {
		dev_err(dev, "cannot prepare/enable master_bus clock\n");
		goto err_iface;
	}

	ret = clk_prepare_enable(res->slave_bus);
	if (ret) {
		dev_err(dev, "cannot prepare/enable slave_bus clock\n");
		goto err_master;
	}

	ret = regulator_enable(res->vdda);
	if (ret) {
		dev_err(dev, "cannot enable vdda regulator\n");
		goto err_slave;
	}

	return 0;

err_slave:
	clk_disable_unprepare(res->slave_bus);
err_master:
	clk_disable_unprepare(res->master_bus);
err_iface:
	clk_disable_unprepare(res->iface);
err_aux:
	clk_disable_unprepare(res->aux);
err_res:
	reset_control_assert(res->core);

	return ret;
}

static int qcom_pcie_get_resources_v0(struct qcom_pcie *pcie)
{
	struct qcom_pcie_resources_v0 *res = &pcie->res.v0;
	struct device *dev = pcie->dev;

	res->vdda = devm_regulator_get(dev, "vdda");
	if (IS_ERR(res->vdda))
		return PTR_ERR(res->vdda);

	res->vdda_phy = devm_regulator_get(dev, "vdda_phy");
	if (IS_ERR(res->vdda_phy))
		return PTR_ERR(res->vdda_phy);

	res->vdda_refclk = devm_regulator_get(dev, "vdda_refclk");
	if (IS_ERR(res->vdda_refclk))
		return PTR_ERR(res->vdda_refclk);

	res->iface_clk = devm_clk_get(dev, "iface");
	if (IS_ERR(res->iface_clk))
		return PTR_ERR(res->iface_clk);

	res->core_clk = devm_clk_get(dev, "core");
	if (IS_ERR(res->core_clk))
		return PTR_ERR(res->core_clk);

	res->phy_clk = devm_clk_get(dev, "phy");
	if (IS_ERR(res->phy_clk))
		return PTR_ERR(res->phy_clk);

	res->aux_clk = devm_clk_get(dev, "aux");
	if (IS_ERR(res->aux_clk))
		return PTR_ERR(res->aux_clk);

	res->ref_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(res->ref_clk))
		return PTR_ERR(res->ref_clk);

	res->pci_reset = devm_reset_control_get(dev, "pci");
	if (IS_ERR(res->pci_reset))
		return PTR_ERR(res->pci_reset);

	res->axi_reset = devm_reset_control_get(dev, "axi");
	if (IS_ERR(res->axi_reset))
		return PTR_ERR(res->axi_reset);

	res->ahb_reset = devm_reset_control_get(dev, "ahb");
	if (IS_ERR(res->ahb_reset))
		return PTR_ERR(res->ahb_reset);

	res->por_reset = devm_reset_control_get(dev, "por");
	if (IS_ERR(res->por_reset))
		return PTR_ERR(res->por_reset);

	res->phy_reset = devm_reset_control_get(dev, "phy");
	if (IS_ERR(res->phy_reset))
		return PTR_ERR(res->phy_reset);

	res->ext_reset = devm_reset_control_get(dev, "ext");
	if (IS_ERR(res->ext_reset))
		return PTR_ERR(res->ext_reset);

	if (of_property_read_u8(dev->of_node, "phy-tx0-term-offset",
				&res->phy_tx0_term_offset))
		res->phy_tx0_term_offset = 0;

	return 0;
}

static int qcom_pcie_get_resources_v1(struct qcom_pcie *pcie)
{
	struct qcom_pcie_resources_v1 *res = &pcie->res.v1;
	struct device *dev = pcie->dev;

	res->vdda = devm_regulator_get(dev, "vdda");
	if (IS_ERR(res->vdda))
		return PTR_ERR(res->vdda);

	res->iface = devm_clk_get(dev, "iface");
	if (IS_ERR(res->iface))
		return PTR_ERR(res->iface);

	res->aux = devm_clk_get(dev, "aux");
	if (IS_ERR(res->aux) && PTR_ERR(res->aux) == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	else if (IS_ERR(res->aux))
		res->aux = NULL;

	res->master_bus = devm_clk_get(dev, "master_bus");
	if (IS_ERR(res->master_bus))
		return PTR_ERR(res->master_bus);

	res->slave_bus = devm_clk_get(dev, "slave_bus");
	if (IS_ERR(res->slave_bus))
		return PTR_ERR(res->slave_bus);

	res->core = devm_reset_control_get(dev, "core");
	if (IS_ERR(res->core))
		return PTR_ERR(res->core);

	return 0;
}

static int qcom_pcie_enable_link_training(struct pcie_port *pp)
{
	struct qcom_pcie *pcie = to_qcom_pcie(pp);
	struct device *dev = pp->dev;
	int retries;
	u32 val;

	/* enable link training */
	writel_masked(pcie->elbi + PCIE20_ELBI_SYS_CTRL, 0, BIT(0));

	/* wait for up to 100ms for the link to come up */
	retries = LINKUP_RETRIES_COUNT;
	do {
		val = readl(pcie->elbi + PCIE20_ELBI_SYS_STTS);
		if (val & XMLH_LINK_UP)
			break;
		usleep_range(LINKUP_DELAY_MIN_US, LINKUP_DELAY_MAX_US);
	} while (retries--);

	if (retries < 0 || !dw_pcie_link_up(pp)) {
		dev_err(dev, "link initialization failed\n");
		return -ENXIO;
	}

	return 0;
}

static void qcom_pcie_host_init_v1(struct pcie_port *pp)
{
	struct qcom_pcie *pcie = to_qcom_pcie(pp);
	int ret;

	qcom_ep_reset_assert(pcie);

	ret = qcom_pcie_enable_resources_v1(pcie);
	if (ret)
		return;

	/* change DBI base address */
	writel(0, pcie->parf + PCIE20_PARF_DBI_BASE_ADDR);

	if (IS_ENABLED(CONFIG_PCI_MSI))
		writel_masked(pcie->parf + PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT,
			      0, BIT(31));

	ret = phy_init(pcie->phy);
	if (ret)
		goto err_res;

	ret = phy_power_on(pcie->phy);
	if (ret)
		goto err_phy;

	dw_pcie_setup_rc(pp);

	if (IS_ENABLED(CONFIG_PCI_MSI))
		dw_pcie_msi_init(pp);

	qcom_ep_reset_deassert(pcie);

	ret = qcom_pcie_enable_link_training(pp);
	if (ret)
		goto err;

	return;

err:
	qcom_ep_reset_assert(pcie);
	phy_power_off(pcie->phy);
err_phy:
	phy_exit(pcie->phy);
err_res:
	qcom_pcie_disable_resources_v1(pcie);
}

static void qcom_pcie_prog_viewport_cfg0(struct qcom_pcie *pcie, u32 busdev)
{
	struct pcie_port *pp = &pcie->pp;

	/*
	 * program and enable address translation region 0 (device config
	 * address space); region type config;
	 * axi config address range to device config address range
	 */
	writel(PCIE20_PLR_IATU_REGION_OUTBOUND |
	       PCIE20_PLR_IATU_REGION_INDEX(0),
	       pcie->dbi + PCIE20_PLR_IATU_VIEWPORT);

	writel(PCIE20_PLR_IATU_TYPE_CFG0, pcie->dbi + PCIE20_PLR_IATU_CTRL1);
	writel(PCIE20_PLR_IATU_ENABLE, pcie->dbi + PCIE20_PLR_IATU_CTRL2);
	writel(pp->cfg0_mod_base, pcie->dbi + PCIE20_PLR_IATU_LBAR);
	writel((pp->cfg0_mod_base >> 32), pcie->dbi + PCIE20_PLR_IATU_UBAR);
	writel((pp->cfg0_mod_base + pp->cfg0_size - 1),
	       pcie->dbi + PCIE20_PLR_IATU_LAR);
	writel(busdev, pcie->dbi + PCIE20_PLR_IATU_LTAR);
	writel(0, pcie->dbi + PCIE20_PLR_IATU_UTAR);
}

static void qcom_pcie_prog_viewport_mem2_outbound(struct qcom_pcie *pcie)
{
	struct pcie_port *pp = &pcie->pp;

	/*
	 * program and enable address translation region 2 (device resource
	 * address space); region type memory;
	 * axi device bar address range to device bar address range
	 */
	writel(PCIE20_PLR_IATU_REGION_OUTBOUND |
	       PCIE20_PLR_IATU_REGION_INDEX(2),
	       pcie->dbi + PCIE20_PLR_IATU_VIEWPORT);

	writel(PCIE20_PLR_IATU_TYPE_MEM, pcie->dbi + PCIE20_PLR_IATU_CTRL1);
	writel(PCIE20_PLR_IATU_ENABLE, pcie->dbi + PCIE20_PLR_IATU_CTRL2);
	writel(pp->mem_mod_base, pcie->dbi + PCIE20_PLR_IATU_LBAR);
	writel((pp->mem_mod_base >> 32), pcie->dbi + PCIE20_PLR_IATU_UBAR);
	writel(pp->mem_mod_base + pp->mem_size - 1,
	       pcie->dbi + PCIE20_PLR_IATU_LAR);
	writel(pp->mem_bus_addr, pcie->dbi + PCIE20_PLR_IATU_LTAR);
	writel(upper_32_bits(pp->mem_bus_addr),
	       pcie->dbi + PCIE20_PLR_IATU_UTAR);

	/* 256B PCIE buffer setting */
	writel(0x1, pcie->dbi + PCIE20_AXI_MSTR_RESP_COMP_CTRL0);
	writel(0x1, pcie->dbi + PCIE20_AXI_MSTR_RESP_COMP_CTRL1);
}

static void qcom_pcie_host_init_v0(struct pcie_port *pp)
{
	struct qcom_pcie *pcie = to_qcom_pcie(pp);
	struct qcom_pcie_resources_v0 *res = &pcie->res.v0;
	struct device *dev = pcie->dev;
	int ret;

	qcom_ep_reset_assert(pcie);

	reset_control_assert(res->ahb_reset);

	ret = qcom_pcie_enable_resources_v0(pcie);
	if (ret)
		return;

	writel_masked(pcie->parf + PCIE20_PARF_PHY_CTRL, BIT(0), 0);

	/* Set Tx termination offset */
	writel_masked(pcie->parf + PCIE20_PARF_PHY_CTRL,
		      PHY_CTRL_PHY_TX0_TERM_OFFSET_MASK,
		      PHY_CTRL_PHY_TX0_TERM_OFFSET(res->phy_tx0_term_offset));

	/* PARF programming */
	writel(PCS_DEEMPH_TX_DEEMPH_GEN1(0x18) |
	       PCS_DEEMPH_TX_DEEMPH_GEN2_3_5DB(0x18) |
	       PCS_DEEMPH_TX_DEEMPH_GEN2_6DB(0x22),
	       pcie->parf + PCIE20_PARF_PCS_DEEMPH);
	writel(PCS_SWING_TX_SWING_FULL(0x78) |
	       PCS_SWING_TX_SWING_LOW(0x78),
	       pcie->parf + PCIE20_PARF_PCS_SWING);
	writel(PHY_RX0_EQ(0x4), pcie->parf + PCIE20_PARF_CONFIG_BITS);

	/* Enable reference clock */
	writel_masked(pcie->parf + PCIE20_PARF_PHY_REFCLK,
		      REF_USE_PAD, REF_SSP_EN);

	/* De-assert PHY, PCIe, POR and AXI resets */
	ret = reset_control_deassert(res->phy_reset);
	if (ret) {
		dev_err(dev, "cannot deassert phy reset\n");
		return;
	}

	ret = reset_control_deassert(res->pci_reset);
	if (ret) {
		dev_err(dev, "cannot deassert pci reset\n");
		return;
	}

	ret = reset_control_deassert(res->por_reset);
	if (ret) {
		dev_err(dev, "cannot deassert por reset\n");
		return;
	}

	ret = reset_control_deassert(res->axi_reset);
	if (ret) {
		dev_err(dev, "cannot deassert axi reset\n");
		return;
	}

	/* wait 150ms for clock acquisition */
	usleep_range(10000, 15000);

	dw_pcie_setup_rc(pp);

	if (IS_ENABLED(CONFIG_PCI_MSI))
		dw_pcie_msi_init(pp);

	qcom_ep_reset_deassert(pcie);

	ret = qcom_pcie_enable_link_training(pp);
	if (ret)
		goto err;

	qcom_pcie_prog_viewport_cfg0(pcie, MSM_PCIE_DEV_CFG_ADDR);
	qcom_pcie_prog_viewport_mem2_outbound(pcie);

	return;
err:
	qcom_ep_reset_assert(pcie);
	qcom_pcie_disable_resources_v0(pcie);
}

static void qcom_pcie_host_init(struct pcie_port *pp)
{
	struct qcom_pcie *pcie = to_qcom_pcie(pp);

	if (pcie->version == PCIE_V0)
		return qcom_pcie_host_init_v0(pp);
	else
		return qcom_pcie_host_init_v1(pp);
}

static int
qcom_pcie_rd_own_conf(struct pcie_port *pp, int where, int size, u32 *val)
{
	/* the device class is not reported correctly from the register */
	if (where == PCI_CLASS_REVISION && size == 4) {
		*val = readl(pp->dbi_base + PCI_CLASS_REVISION);
		*val &= ~(0xffff << 16);
		*val |= PCI_CLASS_BRIDGE_PCI << 16;
		return PCIBIOS_SUCCESSFUL;
	}

	return dw_pcie_cfg_read(pp->dbi_base + (where & ~0x3), where,
				size, val);
}

static struct pcie_host_ops qcom_pcie_ops = {
	.link_up = qcom_pcie_link_up,
	.host_init = qcom_pcie_host_init,
	.rd_own_conf = qcom_pcie_rd_own_conf,
};

static const struct of_device_id qcom_pcie_match[] = {
	{ .compatible = "qcom,pcie-v0", .data = (void *)PCIE_V0 },
	{ .compatible = "qcom,pcie-v1", .data = (void *)PCIE_V1 },
	{ }
};

static int qcom_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct of_device_id *match;
	struct resource *res;
	struct qcom_pcie *pcie;
	struct pcie_port *pp;
	int ret;

	match = of_match_node(qcom_pcie_match, dev->of_node);
	if (!match)
		return -ENXIO;

	pcie = devm_kzalloc(dev, sizeof(*pcie), GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie->version = (unsigned int)match->data;

	pcie->reset = devm_gpiod_get_optional(dev, "perst");
	if (IS_ERR(pcie->reset) && PTR_ERR(pcie->reset) == -EPROBE_DEFER)
		return PTR_ERR(pcie->reset);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "parf");
	pcie->parf = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->parf))
		return PTR_ERR(pcie->parf);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	pcie->dbi = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->dbi))
		return PTR_ERR(pcie->dbi);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "elbi");
	pcie->elbi = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->elbi))
		return PTR_ERR(pcie->elbi);

	pcie->phy = devm_phy_optional_get(dev, "pciephy");
	if (IS_ERR(pcie->phy))
		return PTR_ERR(pcie->phy);

	pcie->dev = dev;

	if (pcie->version == PCIE_V0)
		ret = qcom_pcie_get_resources_v0(pcie);
	else
		ret = qcom_pcie_get_resources_v1(pcie);

	if (ret)
		return ret;

	pp = &pcie->pp;
	pp->dev = dev;
	pp->dbi_base = pcie->dbi;
	pp->root_bus_nr = -1;
	pp->ops = &qcom_pcie_ops;

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = platform_get_irq_byname(pdev, "msi");
		if (pp->msi_irq < 0) {
			dev_err(dev, "cannot get msi irq\n");
			return pp->msi_irq;
		}

		ret = devm_request_irq(dev, pp->msi_irq,
				       qcom_pcie_msi_irq_handler,
				       IRQF_SHARED, "qcom-pcie-msi", pp);
		if (ret) {
			dev_err(dev, "cannot request msi irq\n");
			return ret;
		}
	}

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(dev, "cannot initialize host\n");
		return ret;
	}

	platform_set_drvdata(pdev, pcie);

	return 0;
}

static int qcom_pcie_remove(struct platform_device *pdev)
{
	struct qcom_pcie *pcie = platform_get_drvdata(pdev);

	qcom_ep_reset_assert(pcie);
	phy_power_off(pcie->phy);
	phy_exit(pcie->phy);
	if (pcie->version == PCIE_V0)
		qcom_pcie_disable_resources_v0(pcie);
	else
		qcom_pcie_disable_resources_v1(pcie);

	return 0;
}

static struct platform_driver qcom_pcie_driver = {
	.probe = qcom_pcie_probe,
	.remove = qcom_pcie_remove,
	.driver = {
		.name = "qcom-pcie",
		.of_match_table = qcom_pcie_match,
	},
};

module_platform_driver(qcom_pcie_driver);

MODULE_AUTHOR("Stanimir Varbanov <svarbanov@mm-sol.com>");
MODULE_DESCRIPTION("Qualcomm PCIe root complex driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:qcom-pcie");
