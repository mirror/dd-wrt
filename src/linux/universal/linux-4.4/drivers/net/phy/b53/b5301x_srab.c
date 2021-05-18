/*
 * B53 register access through Switch Register Access Bridge Registers
 *
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/platform_data/b53.h>

#include "b53_priv.h"

/* command and status register of the SRAB */
#define B53_SRAB_CMDSTAT		0x2c
#define  B53_SRAB_CMDSTAT_RST		BIT(2)
#define  B53_SRAB_CMDSTAT_WRITE		BIT(1)
#define  B53_SRAB_CMDSTAT_GORDYN	BIT(0)
#define  B53_SRAB_CMDSTAT_PAGE		24
#define  B53_SRAB_CMDSTAT_REG		16

/* high order word of write data to switch registe */
#define B53_SRAB_WD_H			0x30

/* low order word of write data to switch registe */
#define B53_SRAB_WD_L			0x34

/* high order word of read data from switch register */
#define B53_SRAB_RD_H			0x38

/* low order word of read data from switch register */
#define B53_SRAB_RD_L			0x3c

/* command and status register of the SRAB */
#define B53_SRAB_CTRLS			0x40
#define  B53_SRAB_CTRLS_RCAREQ		BIT(3)
#define  B53_SRAB_CTRLS_RCAGNT		BIT(4)
#define  B53_SRAB_CTRLS_SW_INIT_DONE	BIT(6)

/* the register captures interrupt pulses from the switch */
#define B53_SRAB_INTR			0x44


extern int bcm5301x_robo_rreg(void *robo, u8 page, u8 reg, void *val, int len);
extern int bcm5301x_robo_wreg(void *robo, u8 page, u8 reg, void *val, int len);

static int b53_srab_read8(struct b53_device *dev, u8 page, u8 reg, u8 *val)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_rreg(dev->robo, page, reg, val, 1);
	}
	return ret;
}

static int b53_srab_read16(struct b53_device *dev, u8 page, u8 reg, u16 *val)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_rreg(dev->robo, page, reg, val, 2);
	}
	return ret;
}

static int b53_srab_read32(struct b53_device *dev, u8 page, u8 reg, u32 *val)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_rreg(dev->robo, page, reg, val, 4);
	}
	return ret;
}

static int b53_srab_read48(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_rreg(dev->robo, page, reg, val, 6);
	}

	return ret;
}

static int b53_srab_read64(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_rreg(dev->robo, page, reg, val, 8);
	}

	return ret;
}

static int b53_srab_write8(struct b53_device *dev, u8 page, u8 reg, u8 value)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_wreg(dev->robo, page, reg, &value, 1);
	}

	return ret;
}

static int b53_srab_write16(struct b53_device *dev, u8 page, u8 reg,
			     u16 value)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_wreg(dev->robo, page, reg, &value, 2);
	}

	return ret;
}

static int b53_srab_write32(struct b53_device *dev, u8 page, u8 reg,
				    u32 value)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_wreg(dev->robo, page, reg, &value, 4);
	}

	return ret;

}

static int b53_srab_write48(struct b53_device *dev, u8 page, u8 reg,
				    u64 value)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_wreg(dev->robo, page, reg, &value, 6);
	}

	return ret;

}

static int b53_srab_write64(struct b53_device *dev, u8 page, u8 reg,
			     u64 value)
{
	int ret = -1;

	if (dev->robo) {
		ret = bcm5301x_robo_wreg(dev->robo, page, reg, &value, 8);
	}

	return ret;
}

#ifndef CONFIG_PLAT_BCM5301X
static int do_ioctl(struct ifreq *ifr, int cmd)
{
	mm_segment_t old_fs = get_fs();
	int ret;
	struct net_device *dev = dev_get_by_name(&init_net, "eth0");
	if (!dev || !dev->netdev_ops || !dev->netdev_ops->ndo_do_ioctl) {
	    printk(KERN_INFO "no netdev found on eth0\n");
	    return -1;
	}
	set_fs(KERNEL_DS);
	ret = dev->netdev_ops->ndo_do_ioctl(dev, ifr, cmd);
	set_fs(old_fs);

	return ret;
}

static u16 mdio_read(__u16 phy_id, __u8 reg)
{
	struct ifreq ifr;
	struct mii_ioctl_data *mii = if_mii(&ifr);
	int err;

	mii->phy_id = phy_id;
	mii->reg_num = reg;

	err = do_ioctl(&ifr, SIOCGMIIREG);
	if (err < 0) {
		printk(KERN_ERR "failed to read mdio reg %i with err %i.\n", reg, err);

		return 0xffff;
	}

	return mii->val_out;
}

static void mdio_write(__u16 phy_id, __u8 reg, __u16 val)
{
	struct ifreq ifr;
	struct mii_ioctl_data *mii = if_mii(&ifr);
	int err;

	mii->phy_id = phy_id;
	mii->reg_num = reg;
	mii->val_in = val;

	err = do_ioctl(&ifr, SIOCSMIIREG);
	if (err < 0) {
		printk(KERN_ERR "failed to write mdio reg: %i with err %i.\n", reg, err);
		return;
	}
}

static int b53_mdio_phy_read16(struct b53_device *dev, int addr, u8 reg,
			       u16 *value)
{
	*value = mdio_read(addr, reg);
	return 0;
}

static int b53_mdio_phy_write16(struct b53_device *dev, int addr, u8 reg,
				u16 value)
{
	mdio_write(addr, reg, value);
	return 0;
}
static struct b53_io_ops b53_mdio_ops = {
	.read8 = b53_srab_read8,
	.read16 = b53_srab_read16,
	.read32 = b53_srab_read32,
	.read48 = b53_srab_read48,
	.read64 = b53_srab_read64,
	.write8 = b53_srab_write8,
	.write16 = b53_srab_write16,
	.write32 = b53_srab_write32,
	.write48 = b53_srab_write48,
	.write64 = b53_srab_write64,
	.phy_read16 = b53_mdio_phy_read16,
	.phy_write16 = b53_mdio_phy_write16,
};
#endif
static struct b53_io_ops b53_srab_ops = {
	.read8 = b53_srab_read8,
	.read16 = b53_srab_read16,
	.read32 = b53_srab_read32,
	.read48 = b53_srab_read48,
	.read64 = b53_srab_read64,
	.write8 = b53_srab_write8,
	.write16 = b53_srab_write16,
	.write32 = b53_srab_write32,
	.write48 = b53_srab_write48,
	.write64 = b53_srab_write64,
};


static int b53_srab_probe(struct platform_device *pdev)
{
	struct b53_platform_data *pdata = pdev->dev.platform_data;
	struct b53_device *dev;

	if (!pdata)
		return -EINVAL;


#ifdef CONFIG_PLAT_BCM5301X
	dev = b53_switch_alloc(&pdev->dev, &b53_srab_ops, pdata->regs);
#else
	dev = b53_switch_alloc(&pdev->dev, &b53_mdio_ops, pdata->regs);
#endif
	if (!dev)
		return -ENOMEM;

	if (pdata)
		dev->pdata = pdata;

	platform_set_drvdata(pdev, dev);

	return b53_switch_register(dev);
}

static int b53_srab_remove(struct platform_device *pdev)
{
	struct b53_device *dev = platform_get_drvdata(pdev);

	if (dev)
		b53_switch_remove(dev);

	return 0;
}

static struct platform_driver b53_srab_driver = {
	.probe = b53_srab_probe,
	.remove = b53_srab_remove,
	.driver = {
		.name = "b5301x-srab-switch",
	},
};

module_platform_driver(b53_srab_driver);
MODULE_AUTHOR("Hauke Mehrtens <hauke@hauke-m.de>");
MODULE_DESCRIPTION("B53 Switch Register Access Bridge Registers (SRAB) access driver");
MODULE_LICENSE("Dual BSD/GPL");
