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
"3c574_cs"
"3c589_cs"
"3c59x"
"8139cp"
"8139too"
"8390"
"acenic"
"altera_tse"
"alx"
"amd8111e"
"atl1"
"atl1c"
"atl1e"
"atl2"
"axnet_cs"
"be2net"
"bna"
"bnx2x"
"cassini"
"cnic"
"cxgb"
"cxgb4"
"cxgb4vf"
"de2104x"
"de4x5"
"dl2k"
"dmfe"
"dnet"
"e100"
"e1000"
"e1000e"
"e1000gcu"
"e1000gbe"
"enc28j60"
"enic"
"epic100"
"et131x"
"ethoc"
"fealnx"
"fm10k"
"fmvj18x_cs"
"forcedeth"
"hamachi"
"hp100"
"i40e"
"i40evf"
"igb"
"igbvf"
"ipg"
"ixgb"
"ixgbe"
"jme"
"ks8851"
"ks8851_mll"
"ksz884x"
"mlx4_core"
"mlx4_en"
"myri10ge"
"natsemi"
"ne2k-pci"
"netxen_nic"
"niu"
"nmclan_cs"
"ns83820"
"pcnet32"
"pcnet_cs"
"qla3xxx"
"qlcnic"
"qlge"
"r6040"
"r8169"
"s2io"
"samsung-sxgbe"
"sc92031"
"sfc"
"sis190"
"sis900"
"skge"
"sky2"
"smc91c92_cs"
"smsc911x"
"smsc9420"
"starfire"
"stmmac"
"sundance"
"sungem"
"sunhme"
"tehuti"
"tg3"
"tlan"
"tulip"
"typhoon"
"uli526x"
"via-rhine"
"via-velocity"
"vxge"
"w5100"
"w5300"
"winbond-840"
"xirc2ps_cs"
"xircom_cb"
"yellowfin"
};

static char usbdrivers[] = {
	"r8152 dm9601 gl620a smsc95xx ax88179_178a asix rtl8150 pegasus net1080 sr9700 kaweth cx82310_eth usbnet"
	" lg-vl600 hso zaurus int51x1 mcs7830 catc smsc75xx plusb kalmia"
};

static int detect_driver(char *drivers, char *list)
{
	int basecount = getifcount("eth");
	static char word[256];
	char *next, *wordlist;
	int ret;
	int newcount;
	int rcc = 0;
	wordlist = drivers;
	foreach(word, wordlist, next) {
		ret = eval("modprobe", word);
		if (!ret && (newcount = getifcount("eth")) > basecount) {
			basecount = newcount;
			char *pcid = nvram_safe_get(list);
			char *newdriver = malloc(strlen(pcid) + strlen(word) + 2);
			if (strlen(pcid))
				sprintf(newdriver, "%s %s", pcid, word);
			else
				sprintf(newdriver, "%s", word);
			nvram_set(list, newdriver);
			rcc++;
		} else {
			eval("modprobe", "-r", word);
		}
	}
	return rcc;
}

static int detect_drivers(char *enabled, char *list)
{
	static char word[256];
	char *next, *wordlist;
	int rcc = 0;
	if (!nvram_match(enabled, "1")) {
		rcc = detect_driver(pcidrivers, list);
		nvram_set(enabled, "1");
		nvram_commit();
	} else {
		wordlist = nvram_safe_get(list);
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

static int detect_pcidrivers(void)
{
	return detect_drivers("pci_detected", "pcidrivers");
}

static int detect_usbdrivers(void)
{
	return detect_drivers("usb_detected", "usbdrivers");
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
