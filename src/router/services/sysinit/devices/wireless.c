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

static void detect_wireless_devices(void)
{
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
#endif
#endif
}
