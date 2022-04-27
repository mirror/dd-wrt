/*-
 * Copyright (c) 2004 Atheros Communications, Inc.
 * All rights reserved
 *
 * $Id: if_ath_ahb.c 3286 2008-01-28 20:14:20Z mentor $
 */
#include "opt_ah.h"

#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/cache.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "if_media.h"
#include <net80211/ieee80211_var.h>

#include "if_athvar.h"
#include "ah_devid.h"
#include "if_ath_ahb.h"
#include "ah_soc.h"
#ifdef HAVE_WPROBE
#include "wprobe-core.c"
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
#include <ar231x_platform.h>
#endif

struct ath_ahb_softc {
	struct ath_softc aps_sc;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
	struct ar531x_config aps_config;
#endif
};

static struct ath_ahb_softc *sclist[2] = { NULL, NULL };

static u_int8_t num_activesc = 0;

/*
 * Module glue.
 */
#include "release.h"
static char *version = RELEASE_VERSION;
static char *dev_info = "ath_ahb";

#include <linux/ethtool.h>

/* set bus cachesize in 4B word units */
void bus_read_cachesize(struct ath_softc *sc, u_int8_t *csz)
{
	/* XXX: get the appropriate value! PCI case reads from config space,
	 *   and I think this is the data cache line-size. 
	 */
	*csz = L1_CACHE_BYTES / sizeof(u_int32_t);
}

static int ahb_enable_wmac(u_int16_t devid, u_int16_t wlanNum)
{
	u_int32_t reset;
	u_int32_t enable;

	if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) || ((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ)) {
		u_int32_t reg;
		u_int32_t *en = (u_int32_t *)AR5315_AHB_ARB_CTL;

		KASSERT(wlanNum == 0, ("invalid wlan # %d", wlanNum));

		/* Enable Arbitration for WLAN */
		*en |= AR5315_ARB_WLAN;

		/* Enable global swapping so this looks like a normal BE system */
		reg = REG_READ(AR5315_ENDIAN_CTL);
		reg |= AR5315_CONFIG_WLAN;
		REG_WRITE(AR5315_ENDIAN_CTL, reg);

		/* wake up the MAC */
		/* NOTE: for the following write to succeed the
		 * RST_AHB_ARB_CTL should be set to 0. This driver
		 * assumes that the register has been set to 0 by boot loader
		 */
		reg = REG_READ(AR5315_PCI_MAC_SCR);
		reg = (reg & ~AR5315_PCI_MAC_SCR_SLMODE_M) | (AR5315_PCI_MAC_SCR_SLM_FWAKE << AR5315_PCI_MAC_SCR_SLMODE_S);
		REG_WRITE(AR5315_PCI_MAC_SCR, reg);

		/* wait for the MAC to wakeup */
		while (REG_READ(AR5315_PCI_MAC_PCICFG) & AR5315_PCI_MAC_PCICFG_SPWR_DN) ;
	} else {
		switch (wlanNum) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
		case 0:
#else
		case AR531X_WLAN0_NUM:
#endif
			reset = (AR531X_RESET_WLAN0 | AR531X_RESET_WARM_WLAN0_MAC | AR531X_RESET_WARM_WLAN0_BB);
			enable = AR531X_ENABLE_WLAN0;
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
		case 1:
#else
		case AR531X_WLAN1_NUM:
#endif
			reset = (AR531X_RESET_WLAN1 | AR531X_RESET_WARM_WLAN1_MAC | AR531X_RESET_WARM_WLAN1_BB);
			enable = AR531X_ENABLE_WLAN1;
			break;
		default:
			return -ENODEV;
		}
		/* reset the MAC or suffer lots of AHB PROC errors */
		REG_WRITE(AR531X_RESETCTL, REG_READ(AR531X_RESETCTL) | reset);
		mdelay(15);

		/* take it out of reset */
		REG_WRITE(AR531X_RESETCTL, REG_READ(AR531X_RESETCTL) & ~reset);
		udelay(25);

		/* enable it */
		REG_WRITE(AR531X_ENABLE, REG_READ(AR531X_ENABLE) | enable);
	}
	return 0;
}

static int ahb_disable_wmac(u_int16_t devid, u_int16_t wlanNum)
{
	u_int32_t enable;
	if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) || ((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ)) {
		u_int32_t *en = (u_int32_t *)AR5315_AHB_ARB_CTL;

		KASSERT(wlanNum == 0, ("invalid wlan # %d", wlanNum));

		/* Enable Arbitration for WLAN */
		*en &= ~AR5315_ARB_WLAN;
	} else {
		switch (wlanNum) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
		case 0:
#else
		case AR531X_WLAN0_NUM:
#endif
			enable = AR531X_ENABLE_WLAN0;
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
		case 1:
#else
		case AR531X_WLAN1_NUM:
#endif
			enable = AR531X_ENABLE_WLAN1;
			break;
		default:
			return -ENODEV;
		}
		REG_WRITE(AR531X_ENABLE, REG_READ(AR531X_ENABLE) & ~enable);
	}
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)

static int ahb_wmac_probe(struct platform_device *pdev)
{
	struct ar231x_board_config *bcfg = pdev->dev.platform_data;
	struct ath_ahb_softc *sc;
	struct net_device *dev;
	struct resource *res;
	const char *athname;
	int err;

	ahb_enable_wmac(bcfg->devid, pdev->id);
	dev = m_alloc_netdev(sizeof(struct ath_ahb_softc), "wifi%d", ether_setup);
	if (!dev)
		return -ENOMEM;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
	sc = netdev_priv(dev);
#else
	sc = dev->priv;
#endif
	sc->aps_sc.sc_dev = dev;

	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq <= 0) {
		printk("%s: Cannot find IRQ resource\n", dev->name);
		goto error;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk("%s: Cannot find MMIO resource\n", dev->name);
		goto error;
	}

	dev->mem_start = KSEG1ADDR(res->start);
	dev->mem_end = KSEG1ADDR(res->end);
	sc->aps_sc.sc_iobase = (void __iomem *)dev->mem_start;
	sc->aps_sc.sc_bdev = NULL;

	/* bus information for the HAL */
	sc->aps_config.board = (const struct ar531x_boarddata *)bcfg->config;
	sc->aps_config.radio = bcfg->radio;
	sc->aps_config.unit = pdev->id;
	sc->aps_config.tag = NULL;

	err = ath_attach(bcfg->devid, dev, &sc->aps_config);
	if (err != 0) {
		printk("%s: ath_attach failed: %d\n", dev->name, err);
		goto error;
	}

	athname = ath_hal_probe(ATHEROS_VENDOR_ID, bcfg->devid);
	printk(KERN_INFO "%s: %s: %s: mem=0x%lx, irq=%d\n", dev_info, dev->name, athname ? athname : "Atheros ???", dev->mem_start, dev->irq);

#if !defined(IRQF_DISABLED) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
	if (request_irq(dev->irq, ath_intr, SA_SHIRQ, dev->name, dev)) {
		printk(KERN_WARNING "%s: %s: request_irq failed\n", dev_info, dev->name);
		goto error;
	}
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
	if (request_irq(dev->irq, ath_intr, IRQF_SHARED | IRQF_DISABLED, dev->name, dev)) {
		printk(KERN_WARNING "%s: %s: request_irq failed\n", dev_info, dev->name);
		goto error;
	}
#else
	if (request_irq(dev->irq, ath_intr, IRQF_SHARED, dev->name, dev)) {
		printk(KERN_WARNING "%s: %s: request_irq failed\n", dev_info, dev->name);
		goto error;
	}
#endif

#ifdef LED1_PIN
//#if LED1_PIN == 7
	sc->aps_sc.sc_softled = 0;	/* SoftLED over GPIO */
	sc->aps_sc.sc_ledpin = 0;
//#else
//      sc->aps_sc.sc_softled = 1; /* SoftLED over GPIO */
//      sc->aps_sc.sc_ledpin = config->board->sysLedGpio;
//#endif
#else
	sc->aps_sc.sc_softled = 1;	/* SoftLED over GPIO */
	sc->aps_sc.sc_ledpin = bcfg->config->sysLedGpio;
#endif

	sc->aps_sc.sc_invalid = 0;

#ifdef LED1_PIN
#ifdef AH_SUPPORT_5112
	sc->aps_sc.sc_vendor = 1;	// ns5 5 dbm    
	sc->aps_sc.sc_poweroffset = 5;	// ns5 5 dbm    
#elif !defined(LED4_PIN)
	sc->aps_sc.sc_vendor = 26;	// eoc2610 offset 7 dbm    
	sc->aps_sc.sc_poweroffset = 10;	// eoc2610 offset 7 dbm    
#else
	sc->aps_sc.sc_vendor = 25;	// ns2 10 dbm    
	sc->aps_sc.sc_poweroffset = 10;	// ns2 10 dbm    
#endif
#endif
	sc->aps_sc.sc_invalid = 0;
	platform_set_drvdata(pdev, dev);
	return 0;

error_dev:
	free_irq(dev->irq, dev);
error:
	free_netdev(dev);

	return -ENODEV;
}

static int ahb_wmac_remove(struct platform_device *pdev)
{
	struct ar231x_board_config *bcfg = pdev->dev.platform_data;
	struct net_device *dev;

	dev = platform_get_drvdata(pdev);
	ath_detach(dev);

	if (dev->irq)
		free_irq(dev->irq, dev);

	ahb_disable_wmac(bcfg->devid, pdev->id);
	free_netdev(dev);

	return 0;
}
#else

static int exit_ath_wmac(u_int16_t wlanNum, struct ar531x_config *config)
{
	struct ath_ahb_softc *sc = sclist[wlanNum];
	struct net_device *dev;
	u_int16_t devid;

	if (sc == NULL)
		return -ENODEV;	/* XXX: correct return value? */

	dev = sc->aps_sc.sc_dev;
	ath_detach(dev);
	if (dev->irq)
		free_irq(dev->irq, dev);
	devid = sc->aps_sc.devid;
	config->tag = (void *)((unsigned long)devid);

	ahb_disable_wmac(devid, wlanNum);
	free_netdev(dev);
	sclist[wlanNum] = NULL;
	return 0;
}

static int init_ath_wmac(u_int16_t devid, u_int16_t wlanNum, struct ar531x_config *config)
{
	const char *athname;
	struct net_device *dev;
	struct ath_ahb_softc *sc;

	if (((wlanNum != 0) && (wlanNum != 1)) || (sclist[wlanNum] != NULL))
		goto bad;

	ahb_enable_wmac(devid, wlanNum);

	dev = m_alloc_netdev(sizeof(struct ath_ahb_softc), "wifi%d", ether_setup);
	if (dev == NULL) {
		printk(KERN_ERR "%s: no memory for device state\n", dev_info);
		goto bad2;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
	sc = netdev_priv(dev);
#else
	sc = dev->priv;
#endif
	sc->aps_sc.sc_dev = dev;

	/*
	 * Mark the device as detached to avoid processing
	 * interrupts until setup is complete.
	 */
	sc->aps_sc.sc_invalid = 1;
	SET_MODULE_OWNER(dev);
	sclist[wlanNum] = sc;

	if (dev_alloc_name(dev, dev->name) < 0) {
		printk(KERN_ERR "%s: cannot allocate name\n", dev_info);
		goto bad3;
	}

	switch (wlanNum) {
	case AR531X_WLAN0_NUM:
		if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) || ((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ)) {
			printk(KERN_INFO, "found 5315 %d\n", devid);
			dev->irq = AR5315_IRQ_WLAN0_INTRS;
			dev->mem_start = AR5315_WLAN0;
		} else {
			printk(KERN_INFO, "found 531X %d\n", devid);
			dev->irq = AR531X_IRQ_WLAN0_INTRS;
			dev->mem_start = AR531X_WLAN0;
		}
		break;
	case AR531X_WLAN1_NUM:
		dev->irq = AR531X_IRQ_WLAN1_INTRS;
		dev->mem_start = KSEG1ADDR(AR531X_WLAN1);
		break;
	default:
		goto bad3;
	}
	dev->mem_end = dev->mem_start + AR531X_WLANX_LEN;
	sc->aps_sc.sc_iobase = (void __iomem *)dev->mem_start;
	sc->aps_sc.sc_bdev = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	if (request_irq(dev->irq, ath_intr, IRQF_SHARED, dev->name, dev)) {
#else
	if (request_irq(dev->irq, ath_intr, IRQF_SHARED | IRQF_DISABLED, dev->name, dev)) {
#endif
		printk(KERN_WARNING "%s: %s: request_irq failed\n", dev_info, dev->name);
		goto bad3;
	}

	if (ath_attach(devid, dev, config) != 0)
		goto bad4;
	athname = ath_hal_probe(ATHEROS_VENDOR_ID, devid);
	printk(KERN_INFO "%s: %s: %s: mem=0x%lx, irq=%d\n", dev_info, dev->name, athname ? athname : "Atheros ???", dev->mem_start, dev->irq);
	/* Ready to process interrupts */

#ifdef LED1_PIN
//#if LED1_PIN == 7
	sc->aps_sc.sc_softled = 0;	/* SoftLED over GPIO */
	sc->aps_sc.sc_ledpin = 0;
//#else
//      sc->aps_sc.sc_softled = 1; /* SoftLED over GPIO */
//      sc->aps_sc.sc_ledpin = config->board->sysLedGpio;
//#endif
#else
	sc->aps_sc.sc_softled = 1;	/* SoftLED over GPIO */
	sc->aps_sc.sc_ledpin = config->board->sysLedGpio;
#endif

	sc->aps_sc.sc_invalid = 0;

#ifdef LED1_PIN
#ifdef AH_SUPPORT_5112
	sc->aps_sc.sc_vendor = 1;	// ns5 5 dbm    
	sc->aps_sc.sc_poweroffset = 5;	// ns5 5 dbm    
#elif !defined(LED4_PIN)
	sc->aps_sc.sc_vendor = 26;	// eoc2610 offset 7 dbm    
	sc->aps_sc.sc_poweroffset = 10;	// eoc2610 offset 7 dbm    
#else
	sc->aps_sc.sc_vendor = 25;	// ns2 10 dbm    
	sc->aps_sc.sc_poweroffset = 10;	// ns2 10 dbm    
#endif
#endif

	return 0;

bad4:
	free_irq(dev->irq, dev);
bad3:
	free_netdev(dev);
	sclist[wlanNum] = NULL;
bad2:
	ahb_disable_wmac(devid, wlanNum);
bad:
	return -ENODEV;
}

static int ahb_wmac_probe(struct platform_device *pdev)
{
	u_int16_t devid;
	struct ar531x_config *config;

	config = (struct ar531x_config *)pdev->dev.platform_data;
	devid = (long)config->tag;
	config->tag = NULL;
	return init_ath_wmac(devid, pdev->id, config);
}

static int ahb_wmac_remove(struct platform_device *pdev)
{
	exit_ath_wmac(pdev->id, (struct ar531x_config *)pdev->dev.platform_data);

	return 0;
}
#endif

static struct platform_driver ahb_wmac_driver = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
	.driver.name = "ar231x-wmac",
#else
	.driver.name = "ar531x-wmac",
#endif
	.probe = ahb_wmac_probe,
	.remove = ahb_wmac_remove
};

int ath_ioctl_ethtool(struct ath_softc *sc, int cmd, void __user * addr)
{
	struct ethtool_drvinfo info;

	if (cmd != ETHTOOL_GDRVINFO)
		return -EOPNOTSUPP;
	memset(&info, 0, sizeof(info));
	info.cmd = cmd;
	strncpy(info.driver, dev_info, sizeof(info.driver) - 1);
	strncpy(info.version, version, sizeof(info.version) - 1);
	return copy_to_user(addr, &info, sizeof(info)) ? -EFAULT : 0;
}

MODULE_AUTHOR("Atheros Communications, Inc.");
MODULE_DESCRIPTION("Support for Atheros 802.11 wireless LAN cards.");
#ifdef MODULE_VERSION
MODULE_VERSION(RELEASE_VERSION);
#endif
MODULE_SUPPORTED_DEVICE("Atheros WLAN cards");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

#ifdef SINGLE_MODULE
void ath_rate_sample_init(void);
void ath_rate_minstrel_init(void);
void ath_rate_sample_exit(void);
void ath_rate_minstrel_exit(void);
void exit_ieee80211_cn_idle(void);
void init_ieee80211_cn_idle(void);
void init_ieee80211_cn_policy(void);
void exit_ieee80211_cn_policy(void);
void init_ieee80211_cn_polling(void);
void exit_ieee80211_cn_polling(void);
#endif

static int __init init_ath_ahb(void)
{
	printk(KERN_INFO "%s: %s\n", dev_info, version);
#ifdef HAVE_WPROBE
	wprobe_init();
#endif
#ifdef SINGLE_MODULE
	net80211_init_module();
	ath_rate_sample_init();
	ath_rate_minstrel_init();
#ifdef HAVE_POLLING
	init_ieee80211_cn_idle();
	init_ieee80211_cn_policy();
	init_ieee80211_cn_polling();
#endif
#endif
	platform_driver_register(&ahb_wmac_driver);
	ath_sysctl_register();

	return 0;
}

module_init(init_ath_ahb);

static void __exit exit_ath_ahb(void)
{
	ath_sysctl_unregister();
	platform_driver_unregister(&ahb_wmac_driver);
#ifdef SINGLE_MODULE
#ifdef HAVE_POLLING
	exit_ieee80211_cn_polling();
	exit_ieee80211_cn_policy();
	exit_ieee80211_cn_idle();
#endif
	ath_rate_minstrel_exit();
	ath_rate_sample_exit();
	net80211_exit_module();
#endif
#ifdef HAVE_WPROBE
	wprobe_exit();
#endif
	printk(KERN_INFO "%s: driver unloaded\n", dev_info);
}

module_exit(exit_ath_ahb);
