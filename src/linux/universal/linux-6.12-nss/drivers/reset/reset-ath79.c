// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * AR71xx Reset Controller Driver
 * Author: Alban Bedel
 *
 * Copyright (C) 2015 Alban Bedel <albeu@free.fr>
 */

#include <linux/io.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/of.h>

struct ath79_reset {
	struct platform_device *pdev;
	struct reset_controller_dev rcdev;
	struct notifier_block restart_nb;
	void __iomem *base;
	spinlock_t lock;
};

#define FULL_CHIP_RESET 24
#define ETH_SWITCH_RESET 8
#define ETH_SWITCH_ARESET 12

static void __ath79_reset_update_unlocked(struct ath79_reset *ath79_reset,
			unsigned long id, bool assert)
{
	u32 val;
	val = readl(ath79_reset->base);
	if (assert)
		val |= BIT(id);
	else
		val &= ~BIT(id);
	writel(val, ath79_reset->base);
	/* Flush cache */
	readl(ath79_reset->base);
}

static int ath79_reset_update(struct reset_controller_dev *rcdev,
			unsigned long id, bool assert)
{
	struct ath79_reset *ath79_reset =
		container_of(rcdev, struct ath79_reset, rcdev);
	unsigned long flags;

	spin_lock_irqsave(&ath79_reset->lock, flags);
	__ath79_reset_update_unlocked(ath79_reset, id, assert);
	spin_unlock_irqrestore(&ath79_reset->lock, flags);

	return 0;
}

static int ath79_reset_assert(struct reset_controller_dev *rcdev,
			unsigned long id)
{
	return ath79_reset_update(rcdev, id, true);
}

static int ath79_reset_deassert(struct reset_controller_dev *rcdev,
				unsigned long id)
{
	return ath79_reset_update(rcdev, id, false);
}

static int ath79_reset_status(struct reset_controller_dev *rcdev,
			unsigned long id)
{
	struct ath79_reset *ath79_reset =
		container_of(rcdev, struct ath79_reset, rcdev);
	u32 val;

	val = readl(ath79_reset->base);

	return !!(val & BIT(id));
}

static const struct reset_control_ops ath79_reset_ops = {
	.assert = ath79_reset_assert,
	.deassert = ath79_reset_deassert,
	.status = ath79_reset_status,
};

static int ath79_reset_restart_handler(struct notifier_block *nb,
				unsigned long action, void *data)
{
	struct ath79_reset *ath79_reset =
		container_of(nb, struct ath79_reset, restart_nb);
	unsigned long flags;

	spin_lock_irqsave(&ath79_reset->lock, flags);

	if (of_device_is_compatible(ath79_reset->pdev->dev.of_node, "qca,ar9340-reset")) {
		/**
		 * AR9344 has been observed to hang on reboot in rare cases.
		 *
		 * Datasheet states to reset the ETH switch systems before asserting
		 * full chip reset. See page 111 of the AR9344 datasheet.
		 */
		__ath79_reset_update_unlocked(ath79_reset, ETH_SWITCH_RESET, true);
		mdelay(1);
		__ath79_reset_update_unlocked(ath79_reset, ETH_SWITCH_ARESET, true);
		mdelay(1);
		__ath79_reset_update_unlocked(ath79_reset, FULL_CHIP_RESET, true);
		mdelay(10);
	} else {
		__ath79_reset_update_unlocked(ath79_reset, FULL_CHIP_RESET, true);
	}

	spin_unlock_irqrestore(&ath79_reset->lock, flags);

	return NOTIFY_DONE;
}

static int ath79_reset_probe(struct platform_device *pdev)
{
	struct ath79_reset *ath79_reset;
	int err;

	ath79_reset = devm_kzalloc(&pdev->dev,
				sizeof(*ath79_reset), GFP_KERNEL);
	if (!ath79_reset)
		return -ENOMEM;

	ath79_reset->pdev = pdev;

	ath79_reset->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(ath79_reset->base))
		return PTR_ERR(ath79_reset->base);

	spin_lock_init(&ath79_reset->lock);
	ath79_reset->rcdev.ops = &ath79_reset_ops;
	ath79_reset->rcdev.owner = THIS_MODULE;
	ath79_reset->rcdev.of_node = pdev->dev.of_node;
	ath79_reset->rcdev.of_reset_n_cells = 1;
	ath79_reset->rcdev.nr_resets = 32;

	err = devm_reset_controller_register(&pdev->dev, &ath79_reset->rcdev);
	if (err)
		return err;

	ath79_reset->restart_nb.notifier_call = ath79_reset_restart_handler;
	ath79_reset->restart_nb.priority = 128;

	err = register_restart_handler(&ath79_reset->restart_nb);
	if (err)
		dev_warn(&pdev->dev, "Failed to register restart handler\n");

	return 0;
}

static const struct of_device_id ath79_reset_dt_ids[] = {
	{ .compatible = "qca,ar7100-reset", },
	{ },
};

static struct platform_driver ath79_reset_driver = {
	.probe	= ath79_reset_probe,
	.driver = {
		.name			= "ath79-reset",
		.of_match_table		= ath79_reset_dt_ids,
		.suppress_bind_attrs	= true,
	},
};
builtin_platform_driver(ath79_reset_driver);
