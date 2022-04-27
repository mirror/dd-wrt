/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * Copyright (c) 2004-2005 Atheros Communications, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_ath_pci.c 3312 2008-01-30 21:01:05Z proski $
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
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/cache.h>

#include <linux/pci.h>
#include <linux/pci-aspm.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "if_media.h"
#include <net80211/ieee80211_var.h>

#include "if_athvar.h"
#include "ah_devid.h"
#include "if_ath_pci.h"
#include "ah_ext.h"
#ifdef HAVE_WPROBE
#include "wprobe-core.c"
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
/*
 * PCI initialization uses Linux 2.4.x version and
 * older kernels do not support this
 */
#error Atheros PCI version requires at least Linux kernel version 2.4.0
#endif				/* kernel < 2.4.0 */

struct ath_pci_softc {
	struct ath_softc aps_sc;
#ifdef CONFIG_PM
	u32 aps_pmstate[16];
#endif
};

/*
 * Module glue.
 */
#include "release.h"
static char *version = RELEASE_VERSION;
static char *dev_info = "ath_pci";

#include <linux/ethtool.h>

/*
 * User a static table of PCI IDs for now.  While this is the
 * "new way" to do things, we may want to switch back to having
 * the HAL check them by defining a probe method.
 */
static struct pci_device_id ath_pci_id_table[] __devinitdata = {
	{ 0x168c, 0x001b, 0x0777, 0x3005 },	//UBNT XR5 offset 10
	{ 0x168c, 0x001b, 0x7777, 0x3005 },	//UBNT XR5 offset 10
	{ 0x168c, 0x001b, 0x0777, 0x3004 },	//UBNT XR4
	{ 0x168c, 0x001b, 0x0777, 0x3007 },	//UBNT XR7 offset 10
	{ 0x168c, 0x001b, 0x0777, 0x3b02 },	//UBNT XR2.3
	{ 0x168c, 0x001b, 0x0777, 0x3c02 },	//UBNT XR2.6 offset 10
	{ 0x168c, 0x001b, 0x0777, 0x3002 },	//UBNT XR2
	{ 0x168c, 0x001b, 0x7777, 0x3002 },	//UBNT XR2
	{ 0x168c, 0x001b, 0x0777, 0x3003 },	//UBNT XR3 offset 10
	{ 0x168c, 0x001b, 0x0777, 0x3c03 },	//UBNT XR3-3.6 offset 10
	{ 0x168c, 0x001b, 0x0777, 0x3b03 },	//UBNT XR3-2.8
	{ 0x168c, 0x001b, 0x0777, 0x3009 },	//UBNT XR9 offset 10
	{ 0x168c, 0x001b, 0x0777, 0x1107 },	//UBNT UB5 
//      {0x168c, 0x0027, 0x0777, 0x4082},       //UBNT SR71A offset 10
//      {0x168c, 0x0027, 0x168c, 0x2082},       //UBNT SR71 offset 10
	{ 0x168c, 0x0013, 0x0777, 0x2009 },	//UBNT SR9 
	{ 0x168c, 0x0013, 0x7777, 0x2009 },	//UBNT SR9 offset 12 
	{ 0x168c, 0x0013, 0x7777, 0x2004 },	//UBNT SR4 offset 6
	{ 0x168c, 0x0013, 0x0777, 0x2004 },	//UBNT SR4 offset 6
	{ 0x168c, 0x0013, 0x0777, 0x1004 },	//UBNT SR4C offset 6
	{ 0x168c, 0x0013, 0x7777, 0x1004 },	//UBNT SR4C offset 6
	{ 0x168c, 0x0013, 0x168c, 0x1042 },	//UBNT SRC offset 1
	{ 0x168c, 0x0013, 0x168c, 0x2041 },	//UBNT SR2 offset 10
	{ 0x168c, 0x0013, 0x168c, 0x2042 },	//UBNT SR5 offset 7
	{ 0x168c, 0x0013, 0x168c, 0x2051 },	//
	{ 0x168c, 0x001b, 0x168c, 0x2063 },	//NMP
	{ 0x168c, 0x001b, 0x17f9, 0x000d },	//alfa
	{ 0x168c, 0x001b, 0x168c, 0x2062 },	//NMP
	{ 0x168c, 0x001b, 0x1458, 0xe901 },	//Gigabyte
	{ 0x168c, 0x001b, 0x168d, 0x1031 },	//Alfa
	{ 0x168c, 0x001b, 0x168d, 0x10a2 },	//Alfa
	{ 0x168c, 0x001b, 0xdb11, 0xf50 },	//dbii F50-pro-i
//      {0x168c, 0x001b, 0x19b6, 0x2201},       //Mikrotik R5H
//      {0x168c, 0x001b, 0x19b6, 0x2203},       //Mikrotik R5H
	{ 0x168c, 0x0207, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0007, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0xff16, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0xff96, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0012, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0013, PCI_ANY_ID, PCI_ANY_ID },
	{ 0xa727, 0x0013, PCI_ANY_ID, PCI_ANY_ID },	/* 3com */
	{ 0x10b7, 0x0013, PCI_ANY_ID, PCI_ANY_ID },	/* 3com 3CRDAG675 */
	{ 0x168c, 0x1014, PCI_ANY_ID, PCI_ANY_ID },	/* IBM minipci 5212 */
	{ 0x168c, 0x101a, PCI_ANY_ID, PCI_ANY_ID },	/* some Griffin-Lite */
	{ 0x168c, 0x0015, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0016, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0017, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0018, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0019, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x001a, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x001b, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x018a, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x001c, PCI_ANY_ID, PCI_ANY_ID },	/* PCI Express 5424 */
	{ 0x168c, 0x001d, PCI_ANY_ID, PCI_ANY_ID },	/* PCI Express ???  */
	{ 0x168c, 0x9013, PCI_ANY_ID, PCI_ANY_ID },	/* sonicwall */
//      { 0x168c, 0x0023, PCI_ANY_ID, PCI_ANY_ID },
//      { 0x168c, 0x0024, PCI_ANY_ID, PCI_ANY_ID },
//      { 0x168c, 0x0027, PCI_ANY_ID, PCI_ANY_ID},
//      { 0x168c, 0x0029, PCI_ANY_ID, PCI_ANY_ID }, /* 0x0029: Merlin (PCI) */
//      { 0x168c, 0x002a, PCI_ANY_ID, PCI_ANY_ID }, /* 0x002A: Merlin (Express)*/
	{ 0x168c, 0xff1a, PCI_ANY_ID, PCI_ANY_ID },
	{ 0 }
};

static u16 ath_devidmap[][2] = {
	{ 0x9013, 0x0013 },
	{ 0xff16, 0x0013 },
	{ 0xff96, 0x0013 },
	{ 0xff1a, 0x001a }
};

static int ath_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	unsigned long phymem;
	void __iomem *mem;
	struct ath_pci_softc *sc;
	struct net_device *dev;
	const char *athname;
	u_int8_t csz;
	int ret;
	u_int8_t powerfix = 0;
	u32 val;
	u16 vdevice;
	int i;

//      if (id->vendor == 0x168c && id->device == 0x001b
//          && id->subvendor == 0x168c && id->subdevice == 0x2063)
//              powerfix = 7;

//      if (id->vendor == 0x168c &&
//          id->device == 0x001b &&
//          id->subvendor == 0x168c && id->subdevice == 0x2062)
//              powerfix = 7;
// chris
	powerfix = 0;

	if (id->vendor == 0x168c && id->device == 0x001b && id->subvendor == 0x1458 && id->subdevice == 0xe901)
		powerfix = 2;

	pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S);

	if (pci_enable_device(pdev))
		return -EIO;

	/* XXX 32-bit addressing only */
	if (pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		printk(KERN_ERR "%s: 32-bit DMA not available\n", dev_info);
		goto bad;
	}

	/*
	 * Cache line size is used to size and align various
	 * structures used to communicate with the hardware.
	 */
	pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE, &csz);
	if (csz == 0) {
		/*
		 * Linux 2.4.18 (at least) writes the cache line size
		 * register as a 16-bit wide register which is wrong.
		 * We must have this setup properly for rx buffer
		 * DMA to work so force a reasonable value here if it
		 * comes up zero.
		 */
		csz = L1_CACHE_BYTES >> 2;
		pci_write_config_byte(pdev, PCI_CACHE_LINE_SIZE, csz);
	}
	/*
	 * The default setting of latency timer yields poor results,
	 * set it to the value used by other systems.  It may be worth
	 * tweaking this setting more.
	 */
	pci_write_config_byte(pdev, PCI_LATENCY_TIMER, 0xa8);
	pci_set_master(pdev);

	/*
	 * Disable the RETRY_TIMEOUT register (0x41) to keep
	 * PCI Tx retries from interfering with C3 CPU state.
	 *
	 * Code taken from ipw2100 driver - jg
	 */

	pci_read_config_dword(pdev, 0x40, &val);
	if ((val & 0x0000ff00) != 0)
		pci_write_config_dword(pdev, 0x40, val & 0xffff00ff);

	ret = pci_request_region(pdev, 0, "madwifi");
	if (ret) {
		dev_err(&pdev->dev, "cannot reserve PCI memory region\n");
		goto bad;
	}

	mem = pci_iomap(pdev, 0, 0);
	if (!mem) {
		dev_err(&pdev->dev, "cannot remap PCI memory region\n");
		ret = -EIO;
		goto bad1;
	}

	dev = m_alloc_netdev(sizeof(struct ath_pci_softc), "wifi%d", ether_setup);
	if (dev == NULL) {
		printk(KERN_ERR "%s: no memory for device state\n", dev_info);
		goto bad2;
	}
	sc = netdev_priv(dev);
	sc->aps_sc.sc_dev = dev;
	sc->aps_sc.sc_iobase = mem;

	/*
	 * Mark the device as detached to avoid processing
	 * interrupts until setup is complete.
	 */
	sc->aps_sc.sc_invalid = 1;

	if (dev_alloc_name(dev, dev->name) < 0) {
		printk(KERN_ERR "%s: cannot allocate name\n", dev_info);
		goto bad3;
	}

	dev->irq = pdev->irq;

	SET_MODULE_OWNER(dev);
	SET_NETDEV_DEV(dev, &pdev->dev);

	sc->aps_sc.sc_bdev = (void *)pdev;

	pci_set_drvdata(pdev, dev);
#if !defined(IRQF_DISABLED) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
	if (request_irq(dev->irq, ath_intr, SA_SHIRQ, dev->name, dev)) {
		printk(KERN_WARNING "%s: request_irq for %d failed\n", dev->name, dev->irq);
		goto bad3;
	}
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
	if (request_irq(dev->irq, ath_intr, IRQF_SHARED | IRQF_DISABLED, dev->name, dev)) {
		printk(KERN_WARNING "%s: request_irq for %d failed\n", dev->name, dev->irq);
		goto bad3;
	}
#else
	if (request_irq(dev->irq, ath_intr, IRQF_SHARED, dev->name, dev)) {
		printk(KERN_WARNING "%s: request_irq for %d failed\n", dev->name, dev->irq);
		goto bad3;
	}
#endif

	/* looking for device type from broken device id */
	vdevice = id->device;
	for (i = 0; i < ARRAY_SIZE(ath_devidmap); i++) {
		if (id->device == ath_devidmap[i][0]) {
			vdevice = ath_devidmap[i][1];
			break;
		}
	}

	/*
	 * Auto-enable soft led processing for IBM cards and for
	 * 5211 minipci cards.  Users can also manually enable/disable
	 * support with a sysctl.
	 */
	if (vdevice == AR5212_DEVID_IBM || vdevice == AR5211_DEVID) {
		sc->aps_sc.sc_softled = 1;
		sc->aps_sc.sc_ledpin = 0;
	}

	/* Enable softled on PIN1 on HP Compaq nc6xx, nc4000 & nx5000 laptops */
	if (pdev->subsystem_vendor == PCI_VENDOR_ID_COMPAQ) {
		sc->aps_sc.sc_softled = 1;
		sc->aps_sc.sc_ledpin = 1;
	}

	if (ath_attach(vdevice, dev, NULL) != 0)
		goto bad4;

	if (powerfix) {
		struct ath_hal *ah = sc->aps_sc.sc_ah;
		ah->ah_setCapability(ah, HAL_CAP_POWERFIX, 0, powerfix, NULL);
	}

	athname = ath_hal_probe(id->vendor, vdevice);
	printk(KERN_INFO "%s: %s: %s: mem=0x%lx, irq=%d\n", dev_info, dev->name, athname ? athname : "Atheros ???", phymem, dev->irq);

	/* ready to process interrupts */
	sc->aps_sc.sc_invalid = 0;

	sc->aps_sc.sc_vendor = 0;
	sc->aps_sc.sc_poweroffset = 0;
	sc->aps_sc.sc_dev_vendor = id->vendor;
	sc->aps_sc.sc_dev_device = id->device;

	if (id->subvendor == 0x0777 && id->subdevice == 0x3005) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 1;	// xr5 vendor id (internal)    
	}
	if (id->subvendor == 0x168c && id->subdevice == 0x2063) {
		sc->aps_sc.sc_poweroffset = 8;
		sc->aps_sc.sc_vendor = 1;	// NMP 8603    
	}
	if (id->subvendor == 0x7777 && id->subdevice == 0x3005) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 1;	// xr5 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3002) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 2;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x7777 && id->subdevice == 0x3002) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 2;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3007) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 7;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3003) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 13;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3004) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 14;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x2004) {
		sc->aps_sc.sc_poweroffset = 6;
		sc->aps_sc.sc_vendor = 24;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x7777 && id->subdevice == 0x1004) {
		sc->aps_sc.sc_poweroffset = 6;
		sc->aps_sc.sc_vendor = 24;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x777 && id->subdevice == 0x1004) {
		sc->aps_sc.sc_poweroffset = 6;
		sc->aps_sc.sc_vendor = 24;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x7777 && id->subdevice == 0x2004) {
		sc->aps_sc.sc_poweroffset = 6;
		sc->aps_sc.sc_vendor = 24;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3c03) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 1336;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x1107) {
		sc->aps_sc.sc_poweroffset = 3;
		sc->aps_sc.sc_vendor = 1;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3b03) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 1328;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x4082) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 71;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x168c && id->subdevice == 0x2082) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 71;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3b02) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 23;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3c02) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 26;	// xr2 vendor id (internal)    
	}
	if (id->subvendor == 0x7777 && id->subdevice == 0x2009) {
		sc->aps_sc.sc_poweroffset = 12;
		sc->aps_sc.sc_vendor = 4;	// sr9 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x2009) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 4;	// sr9 vendor id (internal)    
	}
	if (id->subvendor == 0x0777 && id->subdevice == 0x3009) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 9;	// xr9 vendor id (internal)    
	}
	if (id->subvendor == 0x7777 && id->subdevice == 0x3009) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 9;	// xr9 vendor id (internal)    
	}
	if (id->subvendor == 0x168c && id->subdevice == 0x2041) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 3;	// xr5 vendor id (internal)    
	}
	if (id->subvendor == 0x168c && id->subdevice == 0x2042) {
		sc->aps_sc.sc_poweroffset = 7;
		sc->aps_sc.sc_vendor = 5;	// xr5 vendor id (internal)    
	}
	if (id->subvendor == 0x168c && id->subdevice == 0x2051) {
		sc->aps_sc.sc_poweroffset = 10;
		sc->aps_sc.sc_vendor = 5;	// xr5 vendor id (internal)    
	}
	if (id->subvendor == 0x168c && id->subdevice == 0x1042) {
		sc->aps_sc.sc_poweroffset = 1;	// usually not correct
		sc->aps_sc.sc_vendor = 5;	// xr5 vendor id (internal)    
	}
	if (id->subvendor == 0x168d) {
		if ((id->subdevice & 0xf000) == 0x1000)
			sc->aps_sc.sc_poweroffset = (id->subdevice & 0x00f0) >> 16;
		sc->aps_sc.sc_vendor = 99;
	}
	if (id->subvendor == 0x17f9 && id->subdevice == 0x000d) {
		if ((id->subdevice & 0xf000) == 0x1000)
			sc->aps_sc.sc_poweroffset = (id->subdevice & 0x00f0) >> 16;
		sc->aps_sc.sc_vendor = 99;
	}
	if (id->subvendor == 0xdb11 && id->subdevice == 0xf50) {
		sc->aps_sc.sc_poweroffset = 8;
		sc->aps_sc.sc_vendor = 0xdb11;	// dbii F50-pro   
	}
/*	if (id->subvendor == 0x19b6 && id->subdevice == 0x2201)
	    {
	    sc->aps_sc.sc_poweroffset = 8; 
	    sc->aps_sc.sc_vendor = 99;	
	    }
	if (id->subvendor == 0x19b6 && id->subdevice == 0x2203)
	    {
	    sc->aps_sc.sc_poweroffset = 8; 
	    sc->aps_sc.sc_vendor = 99;	
	    }*/

	return 0;
bad4:
	free_irq(dev->irq, dev);
bad3:
	free_netdev(dev);
bad2:
	pci_iounmap(pdev, mem);
bad1:
	pci_release_region(pdev, 0);
bad:
	pci_disable_device(pdev);
	return (-ENODEV);
}

static void ath_pci_remove(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct ath_pci_softc *sc = netdev_priv(dev);
	u16 val;

	/*
	 * Do a config read to clear pre-existing pci error status.
	 * Merlin WAR for bug# 34991.
	 */
	pci_read_config_word(pdev, PCI_COMMAND, &val);

	ath_detach(dev);
	if (dev->irq)
		free_irq(dev->irq, dev);
	pci_iounmap(pdev, sc->aps_sc.sc_iobase);
	pci_release_region(pdev, 0);
	pci_disable_device(pdev);
	free_netdev(dev);
}

#ifdef CONFIG_PM
static int ath_pci_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct net_device *dev = pci_get_drvdata(pdev);

	ath_suspend(dev);
	PCI_SAVE_STATE(pdev, ((struct ath_pci_softc *)dev->priv)->aps_pmstate);
	pci_disable_device(pdev);
	return pci_set_power_state(pdev, PCI_D3hot);
}

static int ath_pci_resume(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	u32 val;
	int err;

	err = pci_set_power_state(pdev, PCI_D0);
	if (err)
		return err;

	/* XXX - Should this return nonzero on fail? */
	PCI_RESTORE_STATE(pdev, ((struct ath_pci_softc *)dev->priv)->aps_pmstate);

	err = pci_enable_device(pdev);
	if (err)
		return err;

	pci_set_master(pdev);
	/*
	 * Suspend/Resume resets the PCI configuration space, so we have to
	 * re-disable the RETRY_TIMEOUT register (0x41) to keep
	 * PCI Tx retries from interfering with C3 CPU state
	 *
	 * Code taken from ipw2100 driver - jg
	 */
	pci_read_config_dword(pdev, 0x40, &val);
	if ((val & 0x0000ff00) != 0)
		pci_write_config_dword(pdev, 0x40, val & 0xffff00ff);
	ath_resume(dev);

	return 0;
}
#endif				/* CONFIG_PM */

MODULE_DEVICE_TABLE(pci, ath_pci_id_table);

static struct pci_driver ath_pci_driver = {
	.name = "ath_pci",
	.id_table = ath_pci_id_table,
	.probe = ath_pci_probe,
	.remove = ath_pci_remove,
#ifdef CONFIG_PM
	.suspend = ath_pci_suspend,
	.resume = ath_pci_resume,
#endif				/* CONFIG_PM */
	/* Linux 2.4.6 has save_state and enable_wake that are not used here */
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,22)
	/* include the device name so later versions of kudzu DTRT */
	strncpy(info.bus_info, pci_name((struct pci_dev *)sc->sc_bdev), sizeof(info.bus_info) - 1);
#endif
	return copy_to_user(addr, &info, sizeof(info)) ? -EFAULT : 0;
}

MODULE_AUTHOR("Errno Consulting, Sam Leffler");
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

static int __init init_ath_pci(void)
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
	if (pci_register_driver(&ath_pci_driver) < 0) {
		printk(KERN_ERR "%s: No devices found, driver not installed.\n", dev_info);
		return (-ENODEV);
	}
	ath_sysctl_register();
	return (0);
}

module_init(init_ath_pci);

static void __exit exit_ath_pci(void)
{
	ath_sysctl_unregister();
	pci_unregister_driver(&ath_pci_driver);
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

module_exit(exit_ath_pci);

/* return bus cachesize in 4B word units */
void bus_read_cachesize(struct ath_softc *sc, u_int8_t *csz)
{
	pci_read_config_byte(sc->sc_bdev, PCI_CACHE_LINE_SIZE, csz);
	/*
	 ** This check was put in to avoid "unplesant" consequences if the bootrom
	 ** has not fully initialized all PCI devices.  Sometimes the cache line size
	 ** register is not set
	 ** 
	 ** On PCIe subsystem, the PCI_CACHE_LINE_SIZE register is reserved and
	 ** the value is zero.
	 ** This symptom is observed on Python test board + XB92.
	 */

//    if(*csz == 0){
//        *csz = DEFAULT_CACHELINE >> 2;   // Use the default size
//    }
}
