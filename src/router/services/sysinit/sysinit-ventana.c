/*
 * sysinit-laguna.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <cymac.h>
#include "devices/ethernet.c"
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	if (!nvram_match("disable_watchdog", "1")) {
		insmod("imx2_wdt");
		eval("watchdog");
	}
	/*
	 * Setup console 
	 */
	eval("mount", "-o", "remount,rw", "/");
	eval("mkdir", "-p", "/usr/local/nvram");

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	//for extension board
	struct ifreq ifr;
	int s;

	fprintf(stderr, "try modules for ethernet adapters\n");
	nvram_set("intel_eth", "0");
	insmod("sky2");
	if (detect_ethernet_devices())
		nvram_set("intel_eth", "1");

	insmod("regmap-spi");
	insmod("regmap-i2c");
	insmod("snd-compress");
	insmod("snd-pcm-dmaengine");
	insmod("snd-soc-core");
	insmod("snd-soc-sgtl5000");
	insmod("imx-pcm-dma");
	insmod("snd-soc-fsl-ssi");
	insmod("snd-soc-imx-audmux");
	insmod("snd-soc-imx-pcm");
	insmod("snd-soc-fsl-utils");
	insmod("snd-soc-imx-sgtl5000");

	//load mmc drivers
	insmod("mmc_core");
	eval("insmod", "sdhci", "debug_quirks=1");	// workaround for mmc detection issue. 
//      insmod("sdhci");
	insmod("sdhci-pltfm");
	insmod("sdhci-cns3xxx");
	insmod("mmc_block");
	//sata drivers
	insmod("scsi_mod");
	insmod("scsi_wait_scan");
	insmod("scsi_sd_mod");
	insmod("sd_mod");
	insmod("libata");
	insmod("libahci");
	insmod("ahci");*/
	/*
	 * network drivers 
	 */
	detect_wireless_devices();

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr_safe", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}
	eval("ifconfig", "eth0", "promisc");
	eval("ifconfig", "eth1", "promisc");
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");
	eval("hwclock", "-s");
	eval("i2cset", "-f", "-y", "0", "0x20", "0", "0x0");
	eval("i2cset", "-f", "-y", "0", "0x20", "11", "0x10");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2391")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2391"))
		sysprintf("gsp_updater -f /etc/gsc_2391_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2389")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2389"))
		sysprintf("gsp_updater -f /etc/gsc_2388_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2388")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2388"))
		sysprintf("gsp_updater -f /etc/gsc_2388_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2387")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2387"))
		sysprintf("gsp_updater -f /etc/gsc_2387_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2386")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2386"))
		sysprintf("gsp_updater -f /etc/gsc_2386_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2384")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2384"))
		sysprintf("gsp_updater -f /etc/gsc_2384_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2383")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2383"))
		sysprintf("gsp_updater -f /etc/gsc_2383_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2382")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2382"))
		sysprintf("gsp_updater -f /etc/gsc_2382_v35.txt");
	if (!strcmp(nvram_safe_get("DD_BOARD"), "Gateworks Laguna GW2380")
	    || !strcmp(nvram_safe_get("DD_BOARD2"), "Gateworks Laguna GW2380"))
		sysprintf("gsp_updater -f /etc/gsc_2380_v35.txt");

	return;
}

int check_cfe_nv(void)
{
	nvram_set("portprio_support", "0");
	return 0;
}

int check_pmon_nv(void)
{
	return 0;
}

void start_overclocking(void)
{
}

void enable_dtag_vlan(int enable)
{

}
