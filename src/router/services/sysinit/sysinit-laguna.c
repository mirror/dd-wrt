/*
 * sysinit-laguna.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <linux/if_ether.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <services.h>

#include "devices/ethernet.c"
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	if (!nvram_matchi("disable_watchdog", 1)) {
		insmod("cns3xxx_wdt");
	}
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	//for extension board
	struct ifreq ifr;
	int s;

	fprintf(stderr, "try modules for ethernet adapters\n");
	nvram_seti("intel_eth", 0);
	insmod("cns3xxx_eth");
	if (detect_ethernet_devices())
		nvram_seti("intel_eth", 1);

	//load mmc drivers
	insmod("mmc_core");
	eval("insmod", "sdhci",
	     "debug_quirks=1"); // workaround for mmc detection issue.
	//      insmod("sdhci");
	insmod("sdhci-pltfm");
	insmod("sdhci-cns3xxx");
	insmod("mmc_block");
	//sata drivers
	insmod("scsi_common");
	insmod("bsg");
	insmod("scsi_mod");
	insmod("scsi_wait_scan");
	insmod("scsi_sd_mod");
	insmod("libata");
	insmod("libahci");
	insmod("ahci");
	insmod("cns3xxx_ahci");
	/*
	 * network drivers 
	 */
	detect_wireless_devices(RADIO_ALL);

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");
	eval("hwclock", "-s", "-u");
	eval("i2cset", "-f", "-y", "0", "0x20", "0", "0x0");
	eval("i2cset", "-f", "-y", "0", "0x20", "11", "0x10");
	char *modelname = nvram_safe_get("DD_BOARD");
	if (!strcmp(modelname, "Gateworks Laguna GW2391"))
		eval("gsp_updater", "-f", "/etc/gsc_2391_v51.txt", "-r", "51");
	if (!strcmp(modelname, "Gateworks Laguna GW2393"))
		eval("gsp_updater", "-f", "/etc/gsc_2391_v51.txt", "-r", "51");
	if (!strcmp(modelname, "Gateworks Laguna GW2389"))
		eval("gsp_updater", "-f", "/etc/gsc_2388_v51.txt", "-r", "51");
	if (!strcmp(modelname, "Gateworks Laguna GW2388"))
		eval("gsp_updater", "-f", "/etc/gsc_2388_v51.txt", "-r", "51");
	if (!strcmp(modelname, "Gateworks Laguna GW2387"))
		eval("gsp_updater", "-f", "/etc/gsc_2387_v46.txt", "-r", "46");
	if (!strcmp(modelname, "Gateworks Laguna GW2386"))
		eval("gsp_updater", "-f", "/etc/gsc_2386_v51.txt", "-r", "51");
	if (!strcmp(modelname, "Gateworks Laguna GW2384"))
		eval("gsp_updater", "-f", "/etc/gsc_2384_v35.txt", "-r", "35");
	if (!strcmp(modelname, "Gateworks Laguna GW2383"))
		eval("gsp_updater", "-f", "/etc/gsc_2383_v50.txt", "-r", "50");
	if (!strcmp(modelname, "Gateworks Laguna GW2382"))
		eval("gsp_updater", "-f", "/etc/gsc_2382_v50.txt", "-r", "50");
	if (!strcmp(modelname, "Gateworks Laguna GW2380"))
		eval("gsp_updater", "-f", "/etc/gsc_2380_v50.txt", "-r", "50");

	//      set_smp_affinity(51, 2);
	return;
}

void start_sysshutdown(void)
{
	start_deconfigurewifi();
}

int check_cfe_nv(void)
{
	nvram_seti("portprio_support", 0);
	return 0;
}

int check_pmon_nv(void)
{
	return 0;
}

void start_overclocking(void)
{
}

char *enable_dtag_vlan(int enable)
{
	return "eth0";
}

char *set_wan_state(int state)
{
	return NULL;
}

void start_devinit_arch(void)
{
}
void load_wifi_drivers(void)
{
}