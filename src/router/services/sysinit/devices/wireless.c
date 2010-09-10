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

static void detect_wireless_devices(void)
{
	nvram_default_get("rate_control","minstrel");
#ifndef HAVE_NOWIFI
#ifdef HAVE_MADWIFI
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
#ifdef HAVE_ATH9K
	if (nvram_match("mimo_driver", "ath9k"))
		{
		fprintf(stderr, "load ATH9K 802.11n Driver\n");
		// some are just for future use and not (yet) there
		insmod("/lib/ath9k/compat.ko");
		insmod("/lib/ath9k/compat_firmware_class.ko");
		insmod("/lib/ath9k/cfg80211.ko");
		insmod("/lib/ath9k/mac80211.ko");
		insmod("/lib/ath9k/ath.ko");
		insmod("/lib/ath9k/ath9k_hw.ko");
		insmod("/lib/ath9k/ath9k_common.ko");
		insmod("/lib/ath9k/ath9k.ko");
		eval("iw", "wlan0", "del"); 
		/*
		int ath9kcount=getath9kdevicecount();
		int i;
		for (i = 0; i < ath9kcount; i++)
			{
			char ath9kiface[32];
			sprintf(ath9kiface, "wlan%d", i);
				eval("iw", ath9kiface, "del");
			}
		*/
		}
	else
		{
#endif
#ifdef HAVE_MADWIFI_MIMO
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
#endif
#ifdef HAVE_ATH9K
		}
#endif
#endif
#endif
}
