/*
 * sysinit-danube.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <sys/ioctl.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#define SIOCGMIIREG 0x8948 /* Read MII PHY register.  */
#define SIOCSMIIREG 0x8949 /* Write MII PHY register.  */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include "devices/wireless.c"

/*
	option unit		0
	option encaps	llc
	option vpi		1
	option vci		32
	option payload	bridged # some ISPs need this set to 'routed'

	local cfg="$1"

	local atmdev
	config_get atmdev "$cfg" atmdev 0

	local unit
	config_get unit "$cfg" unit 0

	local vpi
	config_get vpi "$cfg" vpi 8

	local vci
	config_get vci "$cfg" vci 35

	local encaps
	config_get encaps "$cfg" encaps

	case "$encaps" in
		1|vc) encaps=1;;
		*) encaps=0;;
	esac

	local payload
	config_get payload "$cfg" payload

	case "$payload" in
		0|routed) payload=0;;
		*) payload=1;;
	esac

	local qos
	config_get qos "$cfg" qos

	local circuit="$atmdev.$vpi.$vci"
	local pid="/var/run/br2684ctl-$circuit.pid"

	start-stop-daemon -S -b -x /usr/sbin/br2684ctl -m -p "$pid" -- \
		-c "$unit" -e "$encaps" -p "$payload" \
		-a "$circuit" ${qos:+-q "$qos"}

*/

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	cprintf("sysinit() setup console\n");
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();
	char *annex;

	/*
	 * Modules 
	 */

	/*
	 * network drivers 
	 */
	// insmod("ag7100_mod");
	// sleep(1);
	//load dsl drivers
	insmod("lantiq_etop");
	insmod("lantiq_mei");
	insmod("lantiq_atm");
	insmod("drv_dsl_cpe_api");
#ifdef HAVE_TELCOM
	nvram_default_get("annex", "a");
	nvram_default_get("vpi", "0");
	nvram_default_get("vci", "35");
#else
	nvram_default_get("vpi", "1");
	nvram_default_get("vci", "32");
#endif
	nvram_set("dsl_iface_status", "DOWN");
	nvram_set("dsl_snr_down", "");
	nvram_set("dsl_snr_up", "");
	nvram_set("dsl_datarate_ds", "");
	nvram_set("dsl_datarate_us", "");
	nvram_set("dsl_xtu_status", "");
	nvram_set("dsl_tcl_status", "");

#ifdef HAVE_WMBR_G300NH
	if (!strcmp(getUEnv("region"), "DE"))
		annex = nvram_default_get("annex", "b");
	else
#endif
		annex = nvram_default_get("annex", "a");
	char *annexfw = "/usr/lib/firmware/annex_a.bin";

	if (!strncmp(annex, "b", 1) || !strncmp(annex, "j", 1))
		annexfw = "/usr/lib/firmware/annex_b.bin";
	char *initcode = "";

	if (!strcmp(annex, "b"))
		initcode = "10_00_10_00_00_04_00_00";
	if (!strcmp(annex, "bdmt"))
		initcode = "10_00_00_00_00_00_00_00";
	if (!strcmp(annex, "badsl2"))
		initcode = "00_00_10_00_00_00_00_00";
	if (!strcmp(annex, "badsl2+"))
		initcode = "00_00_00_00_00_04_00_00";
	if (!strcmp(annex, "c"))
		initcode = "42_04_00_00_00_00_00_00";
	if (!strcmp(annex, "j"))
		initcode = "00_00_00_40_00_00_01_00";
	if (!strcmp(annex, "jadsl2"))
		initcode = "00_00_00_40_00_00_00_00";
	if (!strcmp(annex, "jadsl2+"))
		initcode = "00_00_00_00_00_00_01_00";
	if (!strcmp(annex, "a"))
		initcode = "04_01_04_00_00_01_00_00";
	if (!strcmp(annex, "at1"))
		initcode = "01_00_00_00_00_00_00_00";
	if (!strcmp(annex, "alite"))
		initcode = "00_01_00_00_00_00_00_00";
	if (!strcmp(annex, "admt"))
		initcode = "04_00_00_00_00_00_00_00";
	if (!strcmp(annex, "aadsl2"))
		initcode = "00_00_04_00_00_00_00_00";
	if (!strcmp(annex, "aadsl2+"))
		initcode = "00_00_00_00_00_01_00_00";
	if (!strcmp(annex, "l"))
		initcode = "00_00_00_00_04_00_00_00";
	if (!strcmp(annex, "m"))
		initcode = "00_00_00_00_40_00_04_00";
	if (!strcmp(annex, "madsl2"))
		initcode = "00_00_00_00_40_00_00_00";
	if (!strcmp(annex, "madsl2+"))
		initcode = "00_00_00_00_00_00_04_00";

	sysprintf("/usr/sbin/dsl_cpe_control -i%s -f %s -n /usr/sbin/dsl_notification.sh &", initcode, annexfw);

	eval("ifconfig", "eth0", "up");
	detect_wireless_devices(RADIO_ALL);

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
#ifdef HAVE_WMBR_G300NH
	FILE *fp = fopen("/dev/mtdblock/6", "rb");
	if (fp) {
		char mactmp[6];
		int copy[6];
		int i;
		char mac1[32];
		fseek(fp, 0x1fd0024 + 12, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		set_hwaddr("wifi0", mac1);
		fclose(fp);
	}
#ifdef HAVE_ATH9K
	writestr("/sys/devices/platform/leds-gpio/leds/soc:green:wlan/trigger", "phy0tpt");
#else
	writeprocsys("dev/wifi0/ledpin", "15");
	writeprocsys("dev/wifi0/softled", "1");

#endif
	set_gpio(1, 0);
	set_gpio(18, 0);

	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4 5");
	eval("swconfig", "dev", "eth0", "set", "apply");
#endif
#ifdef HAVE_SX763
	char mac[18];
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	set_hwaddr("wifi0", mac);
	writeprocsys("dev/wifi0/ledpin", "219");
	writeprocsys("dev/wifi0/softled", "1");
#endif

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

	return;
	cprintf("done\n");
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