/*
 * ethernet.c
 *
 * Copyright (C) 2015 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 * 
 * detects ethernet adapters and loads the drivers
 */
#include <malloc.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

static char pcidrivers[] = {
	"dmfe.ko de4x5.ko de2104x.ko tulip.ko xircom_cb.ko winbond-840.ko uli526x.ko hp100.ko mlx4_en.ko mlx4_core.ko starfire.ko w5300.ko w5100.ko sungem.ko cassini.ko niu.ko sunhme.ko cxgb.ko cxgb4.ko cxgb4vf.ko s2io.ko vxge.ko myri10ge.ko fealnx.ko enc28j60.ko netxen_nic.ko qlge.ko qla3xxx.ko qlcnic.ko ks8851_mll.ko ks8851.ko ksz884x.ko via-velocity.ko via-rhine.ko fmvj18x_cs.ko enic.ko tg3.ko cnic.ko bnx2x.ko jme.ko stmmac.ko sfc.ko dnet.ko bna.ko ns83820.ko natsemi.ko ipg.ko tlan.ko r6040.ko forcedeth.ko sky2.ko skge.ko epic100.ko smsc9420.ko smsc911x.ko smc91c92_cs.ko be2net.ko xirc2ps_cs.ko amd8111e.ko pcnet32.ko nmclan_cs.ko et131x.ko 8139too.ko 8139cp.ko r8169.ko sundance.ko dl2k.ko tehuti.ko ixgbe.ko e1000e.ko igbvf.ko e1000gcu.ko e1000gbe.ko e100.ko ixgb.ko igb.ko e1000.ko sc92031.ko sis190.ko sis900.ko axnet_cs.ko 8390.ko ne2k-pci.ko pcnet_cs.ko 3c59x.ko 3c589_cs.ko 3c574_cs.ko typhoon.ko ethoc.ko acenic.ko yellowfin.ko hamachi.ko samsung-sxgbe.ko atl1c.ko atl1e.ko atl1.ko atl2.ko alx.ko"
};

static char usbdrivers[] = {
	"r8152.ko dm9601.ko gl620a.ko smsc95xx.ko ax88179_178a.ko cdc_mbim.ko asix.ko rtl8150.ko pegasus.ko net1080.ko sr9700.ko kaweth.ko cx82310_eth.ko usbnet.ko lg-vl600.ko hso.ko zaurus.ko int51x1.ko mcs7830.ko catc.ko smsc75xx.ko plusb.ko kalmia.ko"
};

static int detect_driver(char *drivers, char *list)
{
	int basecount = getifcount("eth");
	static char word[256];
	char *next, *wordlist;
	int ret;
	int rcc = 0;
	wordlist = drivers;
	foreach(word, wordlist, next) {
		fprintf(stderr, "try %s\n", word);
		ret = eval("modprobe", word);
		if (!ret) {
			int newcount = getifcount("eth");
			if (newcount > basecount) {
				basecount = newcount;
				char *pcid = nvram_safe_get(list);
				char *newdriver = malloc(strlen(pcid) + strlen(word) + 2);
				if (strlen(pcid))
					sprintf(newdriver, "%s %s", pcid, word);
				else
					sprintf(newdriver, "%s", word);
				nvram_set(list, newdriver);
				rcc++;
			}
		} else {
			eval("modprobe", "-r", word);
		}
	}
	return rcc;
}

static int detect_pcidrivers(void)
{
	static char word[256];
	char *next, *wordlist;
	int rcc = 0;
	if (!nvram_match("pci_detected", "1")) {
		rcc = detect_driver(pcidrivers, "pcidrivers");
		nvram_set("pci_detected", "1");
		nvram_commit();
	} else {
		wordlist = nvram_safe_get("pcidrivers");
		foreach(word, wordlist, next) {
			eval("modprobe", "word");
		}
		if (strlen(wordlist))
			rcc = 1;
		else
			rcc = 0;
	}
	return rcc;
}

static int detect_usbdrivers(void)
{
	static char word[256];
	char *next, *wordlist;
	int rcc = 0;
	if (!nvram_match("usb_detected", "1")) {
		rcc = detect_driver(usbdrivers, "usbdrivers");
		nvram_set("usb_detected", "1");
		nvram_commit();
	} else {
		wordlist = nvram_safe_get("pcidrivers");
		foreach(word, wordlist, next) {
			eval("modprobe", "word");
		}
		if (strlen(wordlist))
			rcc = 1;
		else
			rcc = 0;

	}
	return rcc;
}

static int detect_ethernet_devices(void)
{
	int rcc = 0;
	rcc |= detect_pcidrivers();
	if (nvram_match("usb_enable", "1")) {
		rcc |= detect_usbdrivers();
	}
	return rcc;
}

void start_detectdrivers(void)
{
	nvram_set("usb_detected", "0");
	nvram_set("pci_detected", "0");
	detect_ethernet_devices();
}

#ifdef TEST

void main(int argc, char *argv[])
{
	start_detectdrivers();
}

#endif
