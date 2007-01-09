/*-
 * Copyright (c) 2004 Atheros Communications, Inc.
 * All rights reserved
 *
 * $Id: if_ath_ahb.c 1590 2006-05-22 04:39:55Z mrenzmann $
 */
#include "opt_ah.h"

#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/cache.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
#include <linux/platform_device.h>
#endif

#include <asm/io.h>
#include <asm/uaccess.h>

#include "if_media.h"
#include <net80211/ieee80211_var.h>

#include "if_athvar.h"
#include "ah_devid.h"
#include "if_ath_ahb.h"
#include "ah_soc.h"

struct ath_ahb_softc {
	struct ath_softc	aps_sc;
#ifdef CONFIG_PM
	u32 aps_pmstate[16];
#endif
};

static struct ath_ahb_softc *sclist[2] = {NULL, NULL};
static u_int8_t num_activesc = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static struct ar531x_boarddata *ar5312_boardConfig = NULL;
static char *radioConfig = NULL;

static int
ar5312_get_board_config(void)
{
	int dataFound;
	char *bd_config;
	
	/*
	 * Find start of Board Configuration data, using heuristics:
	 * Search back from the (aliased) end of flash by 0x1000 bytes
	 * at a time until we find the string "5311", which marks the
	 * start of Board Configuration.  Give up if we've searched
	 * more than 500KB.
	 */
	dataFound = 0;
	for (bd_config = (char *)0xbffff000;
	     bd_config > (char *)0xbff80000;
	     bd_config -= 0x1000) {
		if ( *(int *)bd_config == AR531X_BD_MAGIC) {
			dataFound = 1;
			break;
		}
	}
	
	if (!dataFound) {
		printk("Could not find Board Configuration Data\n");
		bd_config = NULL;
	}
	ar5312_boardConfig = (struct ar531x_boarddata *) bd_config;
	return dataFound;
}

static int
ar5312_get_radio_config(void)
{
	int dataFound;
	char *radio_config;
	
	/* 
	 * Now find the start of Radio Configuration data, using heuristics:
	 * Search forward from Board Configuration data by 0x1000 bytes
	 * at a time until we find non-0xffffffff.
	 */
	dataFound = 0;
	for (radio_config = ((char *) ar5312_boardConfig) + 0x1000;
	     radio_config < (char *)0xbffff000;
	     radio_config += 0x1000) {
		if (*(int *)radio_config != 0xffffffff) {
			dataFound = 1;
			break;
		}
	}

	if (!dataFound) { /* AR2316 relocates radio config to new location */
	    dataFound = 0;
	    for (radio_config = ((char *) ar5312_boardConfig) + 0xf8;
			 radio_config < (char *)0xbffff0f8;
			 radio_config += 0x1000) {
			if (*(int *)radio_config != 0xffffffff) {
				dataFound = 1;
				break;
			}
	    }
	}

	if (!dataFound) {
		printk("Could not find Radio Configuration data\n");
		radio_config = NULL;
	}
	radioConfig = radio_config;
	return dataFound;
}

static int
ar5312SetupFlash(void)
{
	if (ar5312_get_board_config())
		if (ar5312_get_radio_config())
			return 1;
	return 0;
}	

/*
 * Read 16 bits of data from offset into *data
 */
static void
ar5312BspEepromRead(u_int32_t off, u_int32_t nbytes, u_int8_t *data)
{
	int i;
	char *eepromAddr = radioConfig;
	
	for (i = 0; i < nbytes; i++, off++)
		data[i] = eepromAddr[off];
}

#endif

/* set bus cachesize in 4B word units */
void
bus_read_cachesize(struct ath_softc *sc, u_int8_t *csz)
{
	/* XXX: get the appropriate value! PCI case reads from config space,
	 *   and I think this is the data cache line-size. 
	 */
	*csz = L1_CACHE_BYTES / sizeof(u_int32_t);
}

/* NOTE: returns uncached (kseg1) address. */
void *
bus_alloc_consistent(void *hwdev, size_t size, dma_addr_t *dma_handle)
{
	void *ret;
	int gfp = GFP_ATOMIC;
     
	ret = (void *) __get_free_pages(gfp, get_order(size));
     
	if (ret != NULL) {
		memset(ret, 0, size);
		*dma_handle = __pa(ret);
		dma_cache_wback_inv((unsigned long) ret, size);
		ret = UNCAC_ADDR(ret);
	}
     
	return ret;
}

void
bus_free_consistent(void *hwdev, size_t size, void *vaddr, dma_addr_t dma_handle)
{
	unsigned long addr = (unsigned long) vaddr;
    
	addr = CAC_ADDR(addr);
	free_pages(addr, get_order(size));
}

int
ahb_enable_wmac(u_int16_t devid, u_int16_t wlanNum)
{
	u_int32_t reset;
	u_int32_t enable;
	
	if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) ||
		((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ)) {
		u_int32_t reg;
		u_int32_t *en = (u_int32_t *) AR5315_AHB_ARB_CTL;
		
		KASSERT(wlanNum == 0, ("invalid wlan # %d", wlanNum)); 

		/* Enable Arbitration for WLAN */
		*en  |= AR5315_ARB_WLAN;

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
		reg = (reg & ~AR5315_PCI_MAC_SCR_SLMODE_M) | 
		    (AR5315_PCI_MAC_SCR_SLM_FWAKE << AR5315_PCI_MAC_SCR_SLMODE_S);
		REG_WRITE(AR5315_PCI_MAC_SCR, reg);

		/* wait for the MAC to wakeup */
		while (REG_READ(AR5315_PCI_MAC_PCICFG) & AR5315_PCI_MAC_PCICFG_SPWR_DN);
	} else {
		switch (wlanNum) {
		case AR531X_WLAN0_NUM:
			reset = (AR531X_RESET_WLAN0 |
				AR531X_RESET_WARM_WLAN0_MAC |
				AR531X_RESET_WARM_WLAN0_BB);
			enable = AR531X_ENABLE_WLAN0;
			break;
		case AR531X_WLAN1_NUM:
			reset = (AR531X_RESET_WLAN1 |
				AR531X_RESET_WARM_WLAN1_MAC |
				AR531X_RESET_WARM_WLAN1_BB);
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

int
ahb_disable_wmac(u_int16_t devid, u_int16_t wlanNum)
{
	u_int32_t enable;
	if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) ||
		((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ)) {
		u_int32_t *en = (u_int32_t *) AR5315_AHB_ARB_CTL;

		KASSERT(wlanNum == 0, ("invalid wlan # %d", wlanNum) ); 

		/* Enable Arbitration for WLAN */
		*en &= ~AR5315_ARB_WLAN;
	} else { 
		switch (wlanNum) {
		case AR531X_WLAN0_NUM:
			enable = AR531X_ENABLE_WLAN0;
			break;
		case AR531X_WLAN1_NUM:
			enable = AR531X_ENABLE_WLAN1;
			break;
		default:
			return -ENODEV;
		}
		REG_WRITE(AR531X_ENABLE, REG_READ(AR531X_ENABLE) & ~enable);
	}
	return 0;
}


int
exit_ath_wmac(u_int16_t wlanNum)
{
	struct ath_ahb_softc *sc = sclist[wlanNum];
	struct net_device *dev;
	const char *sysType;
	u_int16_t devid;        

	if (sc == NULL)
		return -ENODEV; /* XXX: correct return value? */
        
	dev = sc->aps_sc.sc_dev;
	ath_detach(dev);
	if (dev->irq)
		free_irq(dev->irq, dev);
	sysType = get_system_type();
	if (!strcmp(sysType, "Atheros AR5315"))
		devid = (u_int16_t) (sysRegRead(AR5315_SREV) &
			(AR5315_REV_MAJ_M | AR5315_REV_MIN_M));
	else
		devid = (u_int16_t) ((sysRegRead(AR531X_REV) >> 8) & 
			(AR531X_REV_MAJ | AR531X_REV_MIN));
  
	ahb_disable_wmac(devid, wlanNum);
	free_netdev(dev);
	sclist[wlanNum] = NULL;
	return 0;
}

int
init_ath_wmac(u_int16_t devid, u_int16_t wlanNum, struct ar531x_config *config)
{
	const char *athname;
	struct net_device *dev;
	struct ath_ahb_softc *sc;
        
	if (((wlanNum != 0) && (wlanNum != 1)) ||
		(sclist[wlanNum] != NULL))
		goto bad;
        
	ahb_enable_wmac(devid, wlanNum);

	dev = alloc_netdev(sizeof(struct ath_ahb_softc), "wifi%d", ether_setup);
	if (dev == NULL) {
		printk(KERN_ERR "ath_dev_probe: no memory for device state\n");
		goto bad2;
	}
	sc = dev->priv;
	sc->aps_sc.sc_dev = dev;

	/*
	 * Mark the device as detached to avoid processing
	 * interrupts until setup is complete.
	 */
	sc->aps_sc.sc_invalid = 1;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,41)
	dev->owner = THIS_MODULE;
#else
	SET_MODULE_OWNER(dev);
#endif
	sclist[wlanNum] = sc;

	switch (wlanNum) {
	case AR531X_WLAN0_NUM:
		if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) ||
			((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ)) {
			dev->irq = AR5315_IRQ_WLAN0_INTRS;
			dev->mem_start = AR5315_WLAN0;
		} else {
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
	sc->aps_sc.sc_bdev = NULL;

	if (request_irq(dev->irq, ath_intr, SA_SHIRQ, dev->name, dev)) {
		printk(KERN_WARNING "%s: request_irq failed\n", dev->name);
		goto bad3;
	}
	
	if (ath_attach(devid, dev, config) != 0)
		goto bad4;
	athname = ath_hal_probe(ATHEROS_VENDOR_ID, devid);
	printk(KERN_INFO "%s: %s: mem=0x%lx, irq=%d\n",
		dev->name, athname ? athname : "Atheros ???", dev->mem_start, dev->irq);
	num_activesc++;
	/* Ready to process interrupts */

	sc->aps_sc.sc_invalid = 0;
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
static int ahb_wmac_probe(struct platform_device *pdev)
{
	u32 devid;
	struct ar531x_config *config;

	config = (struct ar531x_config *) pdev->dev.platform_data;
	devid = (u32) config->tag;
	config->tag = NULL;
	
	return init_ath_wmac((u_int16_t) devid, pdev->id, config);
}


static int ahb_wmac_remove(struct platform_device *pdev)
{
	exit_ath_wmac(pdev->id);

	return 0;
}

struct platform_driver ahb_wmac_driver = {
	.driver.name = "ar531x-wmac",
	.probe = ahb_wmac_probe,
	.remove = ahb_wmac_remove
};

#else

int
init_ahb(void)
{
	int ret;
	u_int16_t devid, radioMask;
	const char *sysType;
	struct ar531x_config config;
	
	sysType = get_system_type();
	
	/* Probe to find out the silicon revision and enable the
	   correct number of macs */
	if (!ar5312SetupFlash())
		return -ENODEV;

	config.board = ar5312_boardConfig;
	config.radio = radioConfig;
	config.unit = wlanNum;
	config.tag = NULL;

	if (!strcmp(sysType,"Atheros AR5315")) {
		devid = (u_int16_t) (sysRegRead(AR5315_SREV) &
			(AR5315_REV_MAJ_M | AR5315_REV_MIN_M));
		if (((devid & AR5315_REV_MAJ_M) == AR5315_REV_MAJ) ||
			((devid & AR5315_REV_MAJ_M) == AR5317_REV_MAJ))
			return init_ath_wmac(devid, 0);
	}

	devid = (u_int16_t) ((sysRegRead(AR531X_REV) >>8) &
		(AR531X_REV_MAJ | AR531X_REV_MIN));
	switch (devid) {
	case AR5212_AR5312_REV2:
	case AR5212_AR5312_REV7:
		/* Need to determine if we have a 5312 or a 2312 since they
		 * have the same Silicon Rev ID */
		ar5312BspEepromRead(2 * AR531X_RADIO_MASK_OFF, 2,
			(char *) &radioMask);
		if ((radioMask & AR531X_RADIO0_MASK) != 0)
			if ((ret = init_ath_wmac(devid, 0)) !=0 )
				return ret;
		/* XXX: Fall through?! */
	case AR5212_AR2313_REV8:
		if ((ret = init_ath_wmac(devid, 1)) != 0)
			return ret;
		break;
	default:
		return -ENODEV;
	}
	return 0;
}

#endif

/*
 * Module glue.
 */
#include "version.h"
#include "release.h"
static char *version = ATH_PCI_VERSION " (" RELEASE_VERSION ")";
static char *dev_info = "ath_ahb";

#include <linux/ethtool.h>

int
ath_ioctl_ethtool(struct ath_softc *sc, int cmd, void __user *addr)
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

static int __init
init_ath_ahb(void)
{
	printk(KERN_INFO "%s: %s\n", dev_info, version);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
	platform_driver_register(&ahb_wmac_driver);
#else
	if (init_ahb() != 0) {
		printk("ath_ahb: No devices found, driver not installed.\n");
		return (-ENODEV);
	}
#endif

#ifdef CONFIG_SYSCTL
	ath_sysctl_register();
#endif

	return 0;
}
module_init(init_ath_ahb);

static void __exit
exit_ath_ahb(void)
{
#ifdef CONFIG_SYSCTL
	ath_sysctl_unregister();
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
	platform_driver_register(&ahb_wmac_driver);
#else
	exit_ath_wmac(AR531X_WLAN0_NUM);
	exit_ath_wmac(AR531X_WLAN1_NUM);
#endif

	printk(KERN_INFO "%s: driver unloaded\n", dev_info);
}
module_exit(exit_ath_ahb);
