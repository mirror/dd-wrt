/*
 * sysinit-x86.c
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
 * 
 * System Initialisation for Standard PC and compatible Routers
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

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "devices/ethernet.c"
#include "devices/wireless.c"

#define sys_reboot() eval("sync"); eval("/bin/umount","-a","-r"); eval("event","3","1","15")

void start_sysinit(void)
{
	time_t tm = 0;

	if (!insmod("it8712f_wdt")) {
	} else if (!insmod("scx200_wdt")) {
	} else if (!insmod("w83877f_wdt")) {
	} else if (!insmod("it87_wdt")) {
	} else if (!insmod("mei_wdt")) {
	} else if (!insmod("sp5100_tco")) {
	} else
		insmod("softdog");

#ifdef HAVE_ERC
	if (isregistered_real() && nvram_matchi("ree_resetme", 1)) {
		fprintf(stderr, "Restoring REE default nvram\n");
		eval("nvram", "restore", "/etc/defaults/x86ree.backup");
		eval("reboot");
		eval("event", "5", "1", "15");
	}
#endif

	cprintf("sysinit() setup console\n");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

/*	modprobe("nouveau");
	modprobe("i915");
	modprobe("amdgpu");
	modprobe("radeon");
*/	
	/*
	 * eval("insmod","md5"); eval("insmod","aes"); eval("insmod","blowfish");
	 * eval("insmod","deflate"); eval("insmod","des");
	 * eval("insmod","michael_mic"); eval("insmod","cast5");
	 * eval("insmod","crypto_null"); 
	 */
	/* load sensors */
	insmod("input-polldev");
	insmod("hwmon-vid");

	modprobe("abituguru");
	modprobe("abituguru3");
	modprobe("ad7314");
	modprobe("ad7414");
	modprobe("ad7418");
	modprobe("adc128d818");
	modprobe("adcxx");
	modprobe("adm1025");
	modprobe("adm1026");
	modprobe("adm1029");
	modprobe("adm1031");
	modprobe("adm1177");
	modprobe("adm9240");
	modprobe("ads7828");
	modprobe("ads7871");
	modprobe("adt7310");
	modprobe("adt7410");
	modprobe("adt7411");
	modprobe("adt7462");
	modprobe("adt7470");
	modprobe("adt7475");
	modprobe("adt7x10");
	modprobe("aht10");
	modprobe("amc6821");
	modprobe("applesmc");
	modprobe("aquacomputer_d5next");
	modprobe("as370-hwmon");
	modprobe("asb100");
	modprobe("asc7621");
	modprobe("asus-ec-sensors");
	modprobe("atxp1");
	modprobe("axi-fan-control");
	modprobe("coretemp");
	modprobe("corsair-cpro");
	modprobe("corsair-psu");
	modprobe("da9052-hwmon");
	modprobe("da9055-hwmon");
	modprobe("dme1737");
	modprobe("drivetemp");
	modprobe("ds1621");
	modprobe("ds620");
	modprobe("emc1403");
	modprobe("emc2103");
	modprobe("emc2305");
	modprobe("emc6w201");
	modprobe("f71805f");
	modprobe("f71882fg");
	modprobe("f75375s");
	modprobe("fam15h_power");
	modprobe("fschmd");
	modprobe("ftsteutates");
	modprobe("g760a");
	modprobe("g762");
	modprobe("gl518sm");
	modprobe("gl520sm");
	modprobe("hih6130");
	modprobe("hwmon-vid");
	modprobe("i5500_temp");
	modprobe("i5k_amb");
	modprobe("ina209");
	modprobe("ina238");
	modprobe("ina2xx");
	modprobe("ina3221");
	modprobe("it87");
	modprobe("jc42");
	modprobe("k10temp");
	modprobe("k8temp");
	modprobe("lineage-pem");
	modprobe("lm63");
	modprobe("lm70");
	modprobe("lm73");
	modprobe("lm75");
	modprobe("lm77");
	modprobe("lm78");
	modprobe("lm80");
	modprobe("lm83");
	modprobe("lm85");
	modprobe("lm87");
	modprobe("lm90");
	modprobe("lm92");
	modprobe("lm93");
	modprobe("lm95234");
	modprobe("lm95241");
	modprobe("lm95245");
	modprobe("ltc2945");
	modprobe("ltc2947-core");
	modprobe("ltc2947-i2c");
	modprobe("ltc2947-spi");
	modprobe("ltc2990");
	modprobe("ltc2992");
	modprobe("ltc4151");
	modprobe("ltc4215");
	modprobe("ltc4222");
	modprobe("ltc4245");
	modprobe("ltc4260");
	modprobe("ltc4261");
	modprobe("max1111");
	modprobe("max127");
	modprobe("max16065");
	modprobe("max1619");
	modprobe("max1668");
	modprobe("max197");
	modprobe("max31722");
	modprobe("max31730");
	modprobe("max31760");
	modprobe("max31790");
	modprobe("max6620");
	modprobe("max6621");
	modprobe("max6639");
	modprobe("max6650");
	modprobe("max6697");
	modprobe("mcp3021");
	modprobe("mr75203");
	modprobe("nct6683");
	modprobe("nct6775-core");
	modprobe("nct6775-i2c");
	modprobe("nct6775");
	modprobe("nct7802");
	modprobe("nct7904");
	modprobe("npcm750-pwm-fan");
	modprobe("nzxt-kraken2");
	modprobe("nzxt-smart2");
	modprobe("pc87360");
	modprobe("pc87427");
	modprobe("pcf8591");
	modprobe("powr1220");
	modprobe("sbrmi");
	modprobe("sbtsi_temp");
	modprobe("sch5627");
	modprobe("sch5636");
	modprobe("sch56xx-common");
	modprobe("sht15");
	modprobe("sht21");
	modprobe("sht3x");
	modprobe("sht4x");
	modprobe("shtc1");
	modprobe("sis5595");
	modprobe("smm665");
	modprobe("smsc47b397");
	modprobe("smsc47m1");
	modprobe("smsc47m192");
	modprobe("stts751");
	modprobe("tc654");
	modprobe("tc74");
	modprobe("thmc50");
	modprobe("tmp102");
	modprobe("tmp103");
	modprobe("tmp108");
	modprobe("tmp401");
	modprobe("tmp421");
	modprobe("tmp464");
	modprobe("tmp513");
	modprobe("tps23861");
	modprobe("via-cputemp");
	modprobe("via686a");
	modprobe("vt1211");
	modprobe("vt8231");
	modprobe("w83627ehf");
	modprobe("w83627hf");
	modprobe("w83773g");
	modprobe("w83781d");
	modprobe("w83791d");
	modprobe("w83792d");
	modprobe("w83793");
	modprobe("w83795");
	modprobe("w83l785ts");
	modprobe("w83l786ng");
	modprobe("xgene-hwmon");

	detect_ethernet_devices();
	eval("ifconfig", "eth0", "0.0.0.0", "up");
	eval("ifconfig", "eth1", "0.0.0.0", "up");
	eval("ifconfig", "eth2", "0.0.0.0", "up");
	eval("ifconfig", "eth3", "0.0.0.0", "up");

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
	FILE *fp = fopen("/sys/bus/pci/devices/0000:04:00.0/device", "rb");	//pcengines apu fuckup check
	if (fp) {
		char checkbuf[32];
		fscanf(fp, "%s", &checkbuf[0]);
		fclose(fp);
		if (!strcmp(&checkbuf[0], "0xabcd")) {
			sys_reboot();
		}

	}

	nvram_default_get("use_ath5k", "1");
	detect_wireless_devices(RADIO_ALL);

	mknod("/dev/rtc", S_IFCHR | 0644, makedev(253, 0));
#ifdef HAVE_CPUTEMP
	// insmod("nsc_gpio");
	// insmod("scx200_gpio");
	// insmod("scx200_i2c");
	// insmod("scx200_acb");
	// insmod("lm77");
#endif

	nvram_set("wl0_ifname", "wlan0");
	mknod("/dev/crypto", S_IFCHR | 0644, makedev(10, 70));
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	eval("hwclock", "-s", "-u");
	cprintf("done\n");
	return;
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

#include "tools/recover.c"
char *set_wan_state(int state)
{
	return NULL;
}

void start_devinit_arch(void)
{
}
