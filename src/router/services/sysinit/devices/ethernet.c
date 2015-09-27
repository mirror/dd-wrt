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
	"dmfe de4x5 de2104x tulip xircom_cb winbond-840 uli526x hp100 mlx4_en mlx4_core starfire w5300 w5100 sungem cassini niu sunhme cxgb cxgb4 cxgb4vf s2io vxge myri10ge fealnx enc28j60 netxen_nic qlge qla3xxx qlcnic ks8851_mll ks8851 ksz884x via-velocity via-rhine fmvj18x_cs enic tg3 cnic bnx2x jme stmmac sfc dnet bna ns83820 natsemi ipg tlan r6040 forcedeth sky2 skge epic100 smsc9420 smsc911x smc91c92_cs be2net xirc2ps_cs amd8111e pcnet32 nmclan_cs et131x 8139too 8139cp r8169 sundance dl2k tehuti ixgbe e1000e igbvf e1000gcu e1000gbe e100 ixgb igb e1000 sc92031 sis190 sis900 axnet_cs 8390 ne2k-pci pcnet_cs 3c59x 3c589_cs 3c574_cs typhoon ethoc acenic yellowfin hamachi samsung-sxgbe atl1c atl1e atl1 atl2 alx"
};

static char usbdrivers[] = {
	"r8152 dm9601 gl620a smsc95xx ax88179_178a cdc_mbim asix rtl8150 pegasus net1080 sr9700 kaweth cx82310_eth usbnet lg-vl600 hso zaurus int51x1 mcs7830 catc smsc75xx plusb kalmia"
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
			eval("modprobe", word);
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
			eval("modprobe", word);
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
	rcc |= detect_usbdrivers();
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
