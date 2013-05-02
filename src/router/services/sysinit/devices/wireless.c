/*
 * wireless.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
 * detects atheros wireless adapters and loads the drivers
 */

// extern int getath9kdevicecount(void);
extern void delete_ath9k_devices(char *physical_iface);

static void setWirelessLed(int phynum, int ledpin)
{
#ifdef HAVE_MADWIFI_MIMO
	int triggergpio = ledpin;
#endif
#ifdef HAVE_ATH9K
	char trigger[32];
	char sysname[32];
	sprintf(trigger, "phy%dtpt", phynum);
	if (ledpin < 32) {
		sprintf(sysname, "generic_%d", ledpin);
	} else {
#ifdef HAVE_MADWIFI_MIMO
		if (ledpin < 48)
			triggergpio = ledpin - 32;
		else
			triggergpio = ledpin - 48;
#endif
		sprintf(sysname, "wireless_generic_%d", ledpin - 32);
	}
	sysprintf("echo %s > /sys/devices/platform/leds-gpio/leds/%s/trigger", trigger, sysname);
#else
#ifdef HAVE_MADWIFI_MIMO
	if (ledpin >= 32) {
		if (ledpin < 48)
			triggergpio = ledpin - 32;
		else
			triggergpio = ledpin - 48;
	}
#endif
#endif
#ifdef HAVE_MADWIFI_MIMO
	sysprintf("echo %d >/proc/sys/dev/wifi%d/ledpin", triggergpio, phynum);
	sysprintf("echo 1 >/proc/sys/dev/wifi%d/softled", phynum);
#endif
}

#define setWirelessLedGeneric(a,b) setWirelessLed(a,b)
#define setWirelessLedPhy0(b) setWirelessLed(0,b+32)
#define setWirelessLedPhy1(b) setWirelessLed(1,b+48)

static char *has_device(char *dev)
{
	char path[64];
	static char devstr[32];
	sprintf(path, "/sys/bus/pci/devices/0000:00:%s/device", dev);
	FILE *fp = fopen(path, "rb");
	if (fp) {
		fscanf(fp, "%s", devstr);
		fclose(fp);
		return devstr;
	}

	return "";
}

#define AR5416_DEVID_PCI	0x0023
#define AR5416_DEVID_PCIE	0x0024
#define AR9160_DEVID_PCI	0x0027
#define AR9280_DEVID_PCI	0x0029
#define AR9280_DEVID_PCIE	0x002a
#define AR9285_DEVID_PCIE	0x002b
#define AR2427_DEVID_PCIE	0x002c
#define AR9287_DEVID_PCI	0x002d
#define AR9287_DEVID_PCIE	0x002e
#define AR9300_DEVID_PCIE	0x0030
#define AR9300_DEVID_AR9340	0x0031
#define AR9300_DEVID_AR9485_PCIE 0x0032
#define AR9300_DEVID_AR9580	0x0033
#define AR9300_DEVID_AR9462	0x0034
#define AR9300_DEVID_AR9330	0x0035

#define AR5416_AR9100_DEVID	0x000b

static void detect_wireless_devices(void)
{
	int loadath9k = 1;
	int loadlegacy = 1;
#ifdef HAVE_RT61
	if (!strcmp(has_device("0e.0"), "0x3592"))
		nvram_set("rtchip", "3062");
	else
		nvram_set("rtchip", "2860");
#endif
#ifdef HAVE_XSCALE
	loadath9k = 0;
	loadlegacy = 0;
	char path[32];
	int i = 0;
	char *bus[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f" };
	for (i = 0; i < 16; i++) {
		sprintf(path, "0%s.0", bus[i]);
		if (!strcmp(has_device(path), "0x0023"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0024"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0027"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0029"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x002a"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x002b"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x002c"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x002d"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x002e"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0030"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0031"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0032"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0034"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x0035"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x000b"))
			loadath9k = 1;
		if (!strcmp(has_device(path), "0x001b"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0013"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0207"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0007"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0xff16"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0012"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x1014"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x101a"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0015"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0016"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0017"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0018"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x0019"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x001a"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x001b"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x018a"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x001c"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x001d"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0x9013"))
			loadlegacy = 1;
		if (!strcmp(has_device(path), "0xff1a"))
			loadlegacy = 1;
	}
#endif
#ifndef HAVE_NOWIFI
	nvram_default_get("rate_control", "minstrel");
#ifdef HAVE_MADWIFI
	if (loadlegacy) {
		fprintf(stderr, "load ATH 802.11 a/b/g Driver\n");
		insmod("ath_hal");
		if (nvram_get("rate_control") != NULL) {
			char rate[64];

			sprintf(rate, "ratectl=%s", nvram_safe_get("rate_control"));
			eval("insmod", "ath_pci", rate);
			eval("insmod", "ath_ahb", rate);
		} else {
			insmod("ath_pci");
			insmod("ath_ahb");
		}
	}
#ifdef HAVE_ATH9K
#ifndef HAVE_MADWIFI_MIMO
	if (1)
#else
	if (nvram_match("mimo_driver", "ath9k"))
#endif
	{
		if (loadath9k) {
			fprintf(stderr, "load ATH9K 802.11n Driver\n");
			// some are just for future use and not (yet) there
			insmod("/lib/ath9k/compat.ko");
			insmod("/lib/ath9k/compat_firmware_class.ko");
			insmod("/lib/ath9k/cfg80211.ko");
			insmod("/lib/ath9k/mac80211.ko");
			insmod("/lib/ath9k/ath.ko");
			insmod("/lib/ath9k/ath9k_hw.ko");
			insmod("/lib/ath9k/ath9k_common.ko");
#ifdef HAVE_WZRG450
			system("/sbin/insmod /lib/ath9k/ath9k.ko blink=1");
#else
			insmod("/lib/ath9k/ath9k.ko");
#endif
			delete_ath9k_devices(NULL);
		}
	} else {
#endif
#ifdef HAVE_MADWIFI_MIMO
		if (loadath9k) {
			fprintf(stderr, "load ATH 802.11n Driver\n");
			insmod("/lib/80211n/ath_mimo_hal.ko");
			if (nvram_get("rate_control") != NULL) {
				char rate[64];

				sprintf(rate, "ratectl=%s", nvram_safe_get("rate_control"));
				insmod("/lib/80211n/ath_mimo_pci.ko");
				insmod("/lib/80211n/ath_mimo_ahb.ko");
			} else {
				insmod("/lib/80211n/ath_mimo_pci.ko");
				insmod("/lib/80211n/ath_mimo_ahb.ko");
			}
		}
#endif
#ifdef HAVE_ATH9K
	}
#endif
#endif
#endif
}
