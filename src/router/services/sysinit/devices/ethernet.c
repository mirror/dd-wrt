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

static char *pcidrivers[] = {
	"hv_netvsc",
	"vmxnet3",
	"virtio_net",
	"3c574_cs",
	"3c589_cs",
	"3c59x",
	"8139cp",
	"8139too",
	"8390",
	"acenic",
	"altera_tse",
	"alx",
	"amd8111e",
	"atl1",
	"atl1c",
	"atl1e",
	"atl2",
	"axnet_cs",
	"be2net",
	"bna",
	"bnx2x",
	"bnxt_en",
	"cassini",
	"cnic",
	"cpsw_ale",
	"cxgb",
	"cxgb4",
	"cxgb4vf",
	"de2104x",
	"de4x5",
	"dl2k",
	"dmfe",
	"dnet",
	"e100",
	"e1000",
	"e1000e",
	"e1000gbe",
	"e1000gcu",
	"ec_bhf",
	"enc28j60",
	"encx24j600-regmap",
	"encx24j600",
	"enic",
	"epic100",
	"et131x",
	"ethoc",
	"fealnx",
	"fm10k",
	"fmvj18x_cs",
	"forcedeth",
	"genet",
	"hamachi",
	"hp100",
	"i40e",
	"i40evf",
	"igb",
	"igbvf",
	"ipg",
	"ixgb",
	"ixgbe",
	"jme",
	"ks8842",
	"ks8851",
	"ks8851_mll",
	"ksz884x",
	"macb",
	"mlx4_core",
	"mlx4_en",
	"myri10ge",
	"natsemi",
	"nb8800",
	"ne2k-pci",
	"netxen_nic",
	"niu",
	"nmclan_cs",
	"ns83820",
	"pcnet32",
	"pcnet_cs",
	"qed",
	"qede",
	"qla3xxx",
	"qlcnic",
	"qlge",
	"r6040",
	"r8169",
	"s2io",
	"samsung-sxgbe",
	"sc92031",
	"sfc",
	"sis190",
	"sis900",
	"skge",
	"sky2",
	"smc91c92_cs",
	"smsc911x",
	"smsc9420",
	"starfire",
	"stmmac",
	"sundance",
	"sungem",
	"sunhme",
	"tehuti",
	"tg3",
	"tlan",
	"tulip",
	"typhoon",
	"uli526x",
	"via-rhine",
	"via-velocity",
	"vxge",
	"w5100",
	"w5300",
	"winbond-840",
	"xirc2ps_cs",
	"xircom_cb",
	"yellowfin",
	NULL
};

static char *usbdrivers[] = {
	"asix",
	"ax88179_178a",
	"catc",
	"ch9200",
	"cx82310_eth",
	"dm9601",
	"gl620a",
	"int51x1",
	"kalmia",
	"kaweth",
	"lan78xx",
	"lg-vl600",
	"mcs7830",
	"net1080",
	"pegasus",
	"plusb",
	"r8152",
	"rtl8150",
	"sierra_net",
	"smsc75xx",
	"smsc95xx",
	"sr9700",
	"sr9800",
	"usbnet",
	"zaurus",
	NULL
};

static int detect_driver(char **drivers, char *list)
{
	int basecount = getifcount("eth");
	int ret;
	int newcount;
	int rcc = 0;
	int cnt = 0;
	char *driver;
	while ((driver = drivers[cnt++]) != NULL) {
		ret = eval("modprobe", driver);
		if (!ret && (newcount = getifcount("eth")) > basecount) {
			basecount = newcount;
			char *pcid = nvram_safe_get(list);
			char *newdriver = malloc(strlen(pcid) + strlen(driver) + 2);
			if (strlen(pcid))
				sprintf(newdriver, "%s %s", pcid, driver);
			else
				sprintf(newdriver, "%s", driver);
			nvram_set(list, newdriver);
			free(newdriver);
			rcc |= 1;
		} else {
			eval("modprobe", "-r", driver);
		}
	}
	return rcc;
}

static int detect_drivers(char *enabled, char *list, char **driverset)
{
	static char word[256];
	char *next, *wordlist;
	int rcc = 0;
	if (!nvram_match(enabled, "1")) {
		rcc = detect_driver(driverset, list);
		nvram_set(enabled, "1");
		nvram_commit();
	} else {
		wordlist = nvram_safe_get(list);
		if (!strlen(wordlist))
			return 0;
		foreach(word, wordlist, next) {
			eval("modprobe", word);
		}
		rcc = 1;
	}
	return rcc;
}

static int detect_pcidrivers(void)
{
	return detect_drivers("pci_detected", "pcidrivers", pcidrivers);
}

static int detect_usbdrivers(void)
{
	return detect_drivers("usb_detected", "usbdrivers", usbdrivers);
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
