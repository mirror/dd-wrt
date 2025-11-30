/*
 * Copyright (C) 2015 Alban Bedel <albeu@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>
#include <linux/reset.h>

struct ar7200_usb_phy {
	struct reset_control	*rst_phy;
	struct reset_control	*rst_phy_analog;
	struct reset_control	*suspend_override;
	struct phy		*phy;
};

static int ar7200_usb_phy_power_on(struct phy *phy)
{
	struct ar7200_usb_phy *priv = phy_get_drvdata(phy);
	int err = 0;

	if (priv->suspend_override)
		err = reset_control_assert(priv->suspend_override);
	if (priv->rst_phy)
		err |= reset_control_deassert(priv->rst_phy);
	if (priv->rst_phy_analog)
		err |= reset_control_deassert(priv->rst_phy_analog);

	return err;
}

static int ar7200_usb_phy_power_off(struct phy *phy)
{
	struct ar7200_usb_phy *priv = phy_get_drvdata(phy);
	int err = 0;

	if (priv->suspend_override)
		err = reset_control_deassert(priv->suspend_override);
	if (priv->rst_phy)
		err |= reset_control_assert(priv->rst_phy);
	if (priv->rst_phy_analog)
		err |= reset_control_assert(priv->rst_phy_analog);

	return err;
}

static const struct phy_ops ar7200_usb_phy_ops = {
	.power_on	= ar7200_usb_phy_power_on,
	.power_off	= ar7200_usb_phy_power_off,
	.owner		= THIS_MODULE,
};

static int ar7200_usb_phy_probe(struct platform_device *pdev)
{
	struct phy_provider *phy_provider;
	struct ar7200_usb_phy *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->rst_phy = devm_reset_control_get(&pdev->dev, "phy");
	if (IS_ERR(priv->rst_phy))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->rst_phy), "phy reset is missing");

	priv->rst_phy_analog = devm_reset_control_get_optional(
		&pdev->dev, "phy-analog");
	if (IS_ERR(priv->rst_phy_analog))
		return PTR_ERR(priv->rst_phy_analog);

	priv->suspend_override = devm_reset_control_get_optional(
		&pdev->dev, "suspend-override");
	if (IS_ERR(priv->suspend_override))
		return PTR_ERR(priv->suspend_override);

	priv->phy = devm_phy_create(&pdev->dev, NULL, &ar7200_usb_phy_ops);
	if (IS_ERR(priv->phy))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->phy), "failed to create PHY");

	phy_set_drvdata(priv->phy, priv);

	phy_provider = devm_of_phy_provider_register(&pdev->dev, of_phy_simple_xlate);

	return PTR_ERR_OR_ZERO(phy_provider);
}

static const struct of_device_id ar7200_usb_phy_of_match[] = {
	{ .compatible = "qca,ar7200-usb-phy" },
	{}
};
MODULE_DEVICE_TABLE(of, ar7200_usb_phy_of_match);

static struct platform_driver ar7200_usb_phy_driver = {
	.probe	= ar7200_usb_phy_probe,
	.driver = {
		.of_match_table	= ar7200_usb_phy_of_match,
		.name		= "ar7200-usb-phy",
	}
};
module_platform_driver(ar7200_usb_phy_driver);

MODULE_DESCRIPTION("ATH79 USB PHY driver");
MODULE_AUTHOR("Alban Bedel <albeu@free.fr>");
MODULE_LICENSE("GPL");
