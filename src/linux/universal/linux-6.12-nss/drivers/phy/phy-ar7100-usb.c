/*
 * Copyright (C) 2018 John Crispin <john@phrozen.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>
#include <linux/delay.h>
#include <linux/reset.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

struct ar7100_usb_phy {
	struct reset_control	*rst_phy;
	struct reset_control	*rst_host;
	struct reset_control	*rst_ohci_dll;
	void __iomem		*io_base;
	struct phy		*phy;
};

static int ar7100_usb_phy_power_off(struct phy *phy)
{
	struct ar7100_usb_phy *priv = phy_get_drvdata(phy);
	int err = 0;

	err |= reset_control_assert(priv->rst_host);
	err |= reset_control_assert(priv->rst_phy);
	err |= reset_control_assert(priv->rst_ohci_dll);

	return err;
}

static int ar7100_usb_phy_power_on(struct phy *phy)
{
	struct ar7100_usb_phy *priv = phy_get_drvdata(phy);
	int err = 0;

	err |= ar7100_usb_phy_power_off(phy);
	mdelay(100);
	err |= reset_control_deassert(priv->rst_ohci_dll);
	err |= reset_control_deassert(priv->rst_phy);
	err |= reset_control_deassert(priv->rst_host);
	mdelay(500);
	iowrite32(0xf0000, priv->io_base + AR71XX_USB_CTRL_REG_CONFIG);
	iowrite32(0x20c00, priv->io_base + AR71XX_USB_CTRL_REG_FLADJ);

	return err;
}

static const struct phy_ops ar7100_usb_phy_ops = {
	.power_on	= ar7100_usb_phy_power_on,
	.power_off	= ar7100_usb_phy_power_off,
	.owner		= THIS_MODULE,
};

static int ar7100_usb_phy_probe(struct platform_device *pdev)
{
	struct phy_provider *phy_provider;
	struct ar7100_usb_phy *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->io_base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->io_base))
		return PTR_ERR(priv->io_base);

	priv->rst_phy = devm_reset_control_get(&pdev->dev, "phy");
	if (IS_ERR(priv->rst_phy))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->rst_phy), "phy reset is missing");

	priv->rst_host = devm_reset_control_get(&pdev->dev, "host");
	if (IS_ERR(priv->rst_host))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->rst_host), "host reset is missing");

	priv->rst_ohci_dll = devm_reset_control_get(&pdev->dev, "usb-ohci-dll");
	if (IS_ERR(priv->rst_ohci_dll))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->rst_host), "ohci-dll reset is missing");

	priv->phy = devm_phy_create(&pdev->dev, NULL, &ar7100_usb_phy_ops);
	if (IS_ERR(priv->phy))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->phy), "failed to create PHY");

	phy_set_drvdata(priv->phy, priv);

	phy_provider = devm_of_phy_provider_register(&pdev->dev, of_phy_simple_xlate);

	return PTR_ERR_OR_ZERO(phy_provider);
}

static const struct of_device_id ar7100_usb_phy_of_match[] = {
	{ .compatible = "qca,ar7100-usb-phy" },
	{}
};
MODULE_DEVICE_TABLE(of, ar7100_usb_phy_of_match);

static struct platform_driver ar7100_usb_phy_driver = {
	.probe	= ar7100_usb_phy_probe,
	.driver = {
		.of_match_table	= ar7100_usb_phy_of_match,
		.name		= "ar7100-usb-phy",
	}
};
module_platform_driver(ar7100_usb_phy_driver);

MODULE_DESCRIPTION("ATH79 USB PHY driver");
MODULE_AUTHOR("Alban Bedel <albeu@free.fr>");
MODULE_LICENSE("GPL");
