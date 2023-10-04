/*
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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "devices/wireless.c"

#define ALT_PART_NAME_LENGTH 16
struct per_part_info {
	char name[ALT_PART_NAME_LENGTH];
	uint32_t primaryboot;
	uint32_t upgraded;
};

#define NUM_ALT_PARTITION 3
typedef struct {
#define _SMEM_DUAL_BOOTINFO_MAGIC       0xA5A3A1A0
	/* Magic number for identification when reading from flash */
	uint32_t magic;
	/* upgradeinprogress indicates to attempting the upgrade */
	uint32_t upgradeinprogress;
	/* numaltpart indicate number of alt partitions */
	uint32_t numaltpart;

	struct per_part_info per_part_entry[NUM_ALT_PARTITION];
} ipq_smem_bootconfig_info_t;

/* version 2 */
#define SMEM_DUAL_BOOTINFO_MAGIC_START 0xA3A2A1A0
#define SMEM_DUAL_BOOTINFO_MAGIC_END 0xB3B2B1B0

typedef struct {
	uint32_t magic_start;
	uint32_t upgradeinprogress;
	uint32_t age;
	uint32_t numaltpart;
	struct per_part_info per_part_entry[NUM_ALT_PARTITION];
	uint32_t magic_end;
} ipq_smem_bootconfig_v2_info_t;

void start_finishupgrade(void)
{
	char mtdpath[64];
	int mtd = getMTD("BOOTCONFIG");
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);

	ipq_smem_bootconfig_info_t *ipq_smem_bootconfig_info = NULL;
	ipq_smem_bootconfig_v2_info_t *ipq_smem_bootconfig_v2_info = NULL;

	unsigned int *smem = (unsigned int *)calloc(0x60000, 1);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		fread(smem, 0x60000, 1, fp);
		fclose(fp);
		int i;
		unsigned int *p = smem;
		for (i = 0; i < 0x60000 - sizeof(ipq_smem_bootconfig_v2_info); i += 4) {
			if (*p == SMEM_DUAL_BOOTINFO_MAGIC_START) {
				ipq_smem_bootconfig_v2_info = p;
				break;
			}
			if (*p == _SMEM_DUAL_BOOTINFO_MAGIC) {
				ipq_smem_bootconfig_info = p;
				break;
			}
			p++;
		}

	}
	if (ipq_smem_bootconfig_v2_info) {
		fprintf(stderr, "upgrade in progress: %d\n", ipq_smem_bootconfig_v2_info->upgradeinprogress);
		int i;
		if (ipq_smem_bootconfig_v2_info->upgradeinprogress) {
			for (i = 0; i < ipq_smem_bootconfig_v2_info->numaltpart; i++) {
				if (!strncmp(ipq_smem_bootconfig_v2_info->per_part_entry[i].name, "rootfs", 6)) {
					ipq_smem_bootconfig_v2_info->per_part_entry[i].primaryboot = !ipq_smem_bootconfig_v2_info->per_part_entry[i].primaryboot;
					ipq_smem_bootconfig_v2_info->per_part_entry[i].upgraded = 0;
				}
			}
		}
		ipq_smem_bootconfig_v2_info->upgradeinprogress = 0;
	}
	if (ipq_smem_bootconfig_info) {
		fprintf(stderr, "upgrade in progress: %d\n", ipq_smem_bootconfig_info->upgradeinprogress);

		int i;
		if (ipq_smem_bootconfig_info->upgradeinprogress) {
			for (i = 0; i < ipq_smem_bootconfig_info->numaltpart; i++) {
				if (!strncmp(ipq_smem_bootconfig_info->per_part_entry[i].name, "rootfs", 6)) {
					ipq_smem_bootconfig_info->per_part_entry[i].primaryboot = !ipq_smem_bootconfig_info->per_part_entry[i].primaryboot;
					ipq_smem_bootconfig_info->per_part_entry[i].upgraded = 0;
				}
			}
		}
		ipq_smem_bootconfig_info->upgradeinprogress = 0;
	}
	fp = fopen(mtdpath, "wb");
	if (fp) {
		fwrite(smem, 0x60000, 1, fp);
	}
	fclose(fp);
	free(smem);

}

void calcchecksum(void *caldata)	// works on little endian only so far. so consider to fix it when using on big endian systems
{
	int i;
	unsigned short *cdata = (unsigned short *)caldata;
	unsigned short *ptr_eeprom = (unsigned short *)caldata;
	cdata[1] = 0;		// clear checksum for calculation
	int size = cdata[0];
	unsigned short crc = 0;
	for (i = 0; i < size; i += 2) {
		crc ^= *ptr_eeprom;
		ptr_eeprom++;
	}
	crc = ~crc;
	cdata[1] = crc;
}

static int getbootdevice(void)
{
	char mtdpath[64];
	int mtd = getMTD("BOOTCONFIG");
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
	ipq_smem_bootconfig_info_t *ipq_smem_bootconfig_info = NULL;
	ipq_smem_bootconfig_v2_info_t *ipq_smem_bootconfig_v2_info = NULL;
	int ret = -1;

	unsigned int *smem = (unsigned int *)calloc(0x60000, 1);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		fread(smem, 0x60000, 1, fp);
		fclose(fp);
		int i;
		unsigned int *p = smem;
		for (i = 0; i < 0x60000 - sizeof(ipq_smem_bootconfig_v2_info); i += 4) {
			if (*p == SMEM_DUAL_BOOTINFO_MAGIC_START) {
				ipq_smem_bootconfig_v2_info = p;
				break;
			}
			if (*p == _SMEM_DUAL_BOOTINFO_MAGIC) {
				ipq_smem_bootconfig_info = p;
				break;
			}
			p++;
		}

	}
	if (ipq_smem_bootconfig_v2_info) {
		int i;
		for (i = 0; i < ipq_smem_bootconfig_v2_info->numaltpart; i++) {
			if (!strncmp(ipq_smem_bootconfig_v2_info->per_part_entry[i].name, "rootfs", 6)) {
				if (ipq_smem_bootconfig_v2_info->per_part_entry[i].primaryboot)
					ret = 1;
				else
					ret = 0;
			}
		}
	}
	if (ipq_smem_bootconfig_info) {
		int i;
		for (i = 0; i < ipq_smem_bootconfig_info->numaltpart; i++) {
			if (!strncmp(ipq_smem_bootconfig_info->per_part_entry[i].name, "rootfs", 6)) {
				if (ipq_smem_bootconfig_info->per_part_entry[i].primaryboot)
					ret = 1;
				else
					ret = 0;
			}
		}
	}
	free(smem);
	return ret;
}

static void setbootdevice(int dev)
{
	char mtdpath[64];
	int mtd = getMTD("BOOTCONFIG");
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);

	ipq_smem_bootconfig_info_t *ipq_smem_bootconfig_info = NULL;
	ipq_smem_bootconfig_v2_info_t *ipq_smem_bootconfig_v2_info = NULL;

	unsigned int *smem = (unsigned int *)calloc(0x60000, 1);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		fread(smem, 0x60000, 1, fp);
		fclose(fp);
		int i;
		unsigned int *p = smem;
		for (i = 0; i < 0x60000 - sizeof(ipq_smem_bootconfig_v2_info); i += 4) {
			if (*p == SMEM_DUAL_BOOTINFO_MAGIC_START) {
				ipq_smem_bootconfig_v2_info = p;
				break;
			}
			if (*p == _SMEM_DUAL_BOOTINFO_MAGIC) {
				ipq_smem_bootconfig_info = p;
				break;
			}
			p++;
		}

	}
	if (ipq_smem_bootconfig_v2_info) {
		fprintf(stderr, "upgrade in progress: %d\n", ipq_smem_bootconfig_v2_info->upgradeinprogress);
		int i;
		for (i = 0; i < ipq_smem_bootconfig_v2_info->numaltpart; i++) {
			if (!strncmp(ipq_smem_bootconfig_v2_info->per_part_entry[i].name, "rootfs", 6)) {
				fprintf(stderr, "set bootdevice from %d to %d\n", ipq_smem_bootconfig_v2_info->per_part_entry[i].primaryboot, dev);
				ipq_smem_bootconfig_v2_info->per_part_entry[i].upgraded = 1;
			}
		}
		ipq_smem_bootconfig_v2_info->upgradeinprogress = 1;
	}
	if (ipq_smem_bootconfig_info) {
		fprintf(stderr, "upgrade in progress: %d\n", ipq_smem_bootconfig_info->upgradeinprogress);

		int i;
		for (i = 0; i < ipq_smem_bootconfig_info->numaltpart; i++) {
			if (!strncmp(ipq_smem_bootconfig_info->per_part_entry[i].name, "rootfs", 6)) {
				fprintf(stderr, "set bootdevice from %d to %d\n", ipq_smem_bootconfig_info->per_part_entry[i].primaryboot, dev);
				ipq_smem_bootconfig_info->per_part_entry[i].upgraded = 1;
			}
		}
		ipq_smem_bootconfig_info->upgradeinprogress = 1;
	}
	fp = fopen(mtdpath, "wb");
	if (fp) {
		fwrite(smem, 0x60000, 1, fp);
	}
	fclose(fp);
	free(smem);

}

void start_bootsecondary(void)
{
	setbootdevice(1);
}

void start_bootprimary(void)
{
	setbootdevice(0);
}

void *get_deviceinfo(char *var)
{
	static char res[256];
	bzero(res, sizeof(res));
	FILE *fp = fopen("/dev/mtdblock/12", "rb");
	if (!fp)
		return NULL;
	char newname[64];
	snprintf(newname, 64, "%s=", var);
	char *mem = safe_malloc(0x2000);
	fread(mem, 0x2000, 1, fp);
	fclose(fp);
	int s = (0x2000 - 1) - strlen(newname);
	int i;
	int l = strlen(newname);
	for (i = 0; i < s; i++) {
		if (!strncmp(mem + i, newname, l)) {
			strncpy(res, mem + i + l, 17);
			free(mem);
			return res;
		}
	}
	free(mem);
	return NULL;
}

void *get_deviceinfo_g10(char *var)
{
	static char res[256];
	bzero(res, sizeof(res));
	FILE *fp = fopen("/dev/mtdblock/9", "rb");
	if (!fp)
		return NULL;
	char newname[64];
	snprintf(newname, 64, "%s=", var);
	char *mem = safe_malloc(0x2000);
	fread(mem, 0x2000, 1, fp);
	fclose(fp);
	int s = (0x2000 - 1) - strlen(newname);
	int i;
	int l = strlen(newname);
	for (i = 0; i < s; i++) {
		if (!strncmp(mem + i, newname, l)) {
			strncpy(res, mem + i + l, 13);
			free(mem);
			return res;
		}
	}
	free(mem);
	return NULL;
}

static void setasrockcountry(void)
{
	char buf[32];
	char c[32];
	char *set = NULL;
	char *set5 = NULL;
	char rev = -1;
	char rev5 = -1;
#define defstr "HW.RegionDomain="
	FILE *fp = popen("cat /dev/mtdblock9|grep HW.RegionDomain=", "r");
	if (!fp)
		return;
	fread(buf, 1, 18, fp);
	pclose(fp);
	buf[strlen(defstr) + 2] = 0;
	bzero(c, sizeof(c));
	strncpy(c, &buf[strlen(defstr)], 2);
//      fprintf(stderr,"isostr %s\n",buf);
	fprintf(stderr, "iso %s\n", c);
	char *ctry = getCountryByIso(c);
	fprintf(stderr, "country %s\n", ctry);
	if (!ctry)
		return;
	if (!nvram_exists("nocountrysel"))
		nvram_seti("nocountrysel", 1);
	nvram_set("wlan0_regdomain", ctry);
	nvram_set("wlan1_regdomain", ctry);
}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	FILE *fp = NULL;

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	char mtdpath[64];
	int board = getRouterBrand();
	// this is for TEW827 only. i dont know how it works for other boards. offsets might be different
	int mtd = getMTD("art");
	if (mtd == -1)
		mtd = getMTD("ART");
	char *maddr = NULL;
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);

	if (board != ROUTER_NETGEAR_R7500)
		fp = fopen(mtdpath, "rb");
	if (fp != NULL) {
		int newmac[6];
		if (board == ROUTER_TRENDNET_TEW827)
			maddr = getUEnv("lan_mac");
		if (board == ROUTER_LINKSYS_EA8500)
			maddr = get_deviceinfo("hw_mac_addr");

		if (board == ROUTER_ASROCK_G10) {
			static char mg10[20];
			maddr = get_deviceinfo_g10("HW.LAN.MAC.Address");
			if (maddr) {
				sprintf(mg10, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", maddr[0] & 0xff, maddr[1] & 0xff, maddr[2] & 0xff, maddr[3] & 0xff, maddr[4] & 0xff, maddr[5] & 0xff, maddr[6] & 0xff, maddr[7] & 0xff,
					maddr[8] & 0xff, maddr[9] & 0xff, maddr[10] & 0xff, maddr[11] & 0xff);
				maddr = &mg10[0];
			}
			setasrockcountry();
		}

		if (maddr) {
			fprintf(stderr, "sysinit using mac %s\n", maddr);
			sscanf(maddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4], &newmac[5]);
		}

		fseek(fp, 0x1000, SEEK_SET);
		char *smem = malloc(0x8000);
		fread(smem, 0x8000, 1, fp);

		fclose(fp);
		if (maddr && (board == ROUTER_TRENDNET_TEW827 || board == ROUTER_LINKSYS_EA8500 || board == ROUTER_ASROCK_G10)) {	// board calibration data with real mac addresses
			int i;
			for (i = 0; i < 6; i++) {
				smem[i + 6] = newmac[i];
				smem[i + 6 + 0x4000] = newmac[i];
			}
		}
		calcchecksum(smem);
		calcchecksum(&smem[0x4000]);

		eval("rm", "-f", "/tmp/board1.bin");
		fp = fopen("/tmp/board1.bin", "wb");
		fwrite(smem, 12064, 1, fp);
		fclose(fp);
		fp = fopen("/tmp/board2.bin", "wb");
		fwrite(&smem[0x4000], 12064, 1, fp);
		fclose(fp);
		free(smem);

	}
	/* 
	 * 
	 */
	insmod("gsp");
	insmod("slhc");

	insmod("regmap-core");
	insmod("regmap-i2c");
	insmod("regmap-spi");
	insmod("leds-tlc59116");
	insmod("leds-gpio");

	insmod("tmp421");
	insmod("mii");
	insmod("stmmac");	//for debugging purposes compiled as module
	insmod("stmmac-platform");	//for debugging purposes compiled as module
	insmod("qcom-wdt");

	/*
	 * network drivers 
	 */

	//insmod("qdpc-host.ko");
	eval("mount", "-t", "ubifs", "-o", "sync", "ubi0:rootfs_data", "/jffs");

	switch (board) {
	case ROUTER_TRENDNET_TEW827:
		if (maddr) {
			set_hwaddr("eth0", getUEnv("wan_mac"));
			set_hwaddr("eth1", getUEnv("lan_mac"));
		}
		start_finishupgrade();
		if (getbootdevice())
			nvram_seti("bootpartition", 1);
		else
			nvram_seti("bootpartition", 0);
		break;
	case ROUTER_ASROCK_G10:
		if (maddr) {
			set_hwaddr("eth1", maddr);
			set_hwaddr("eth0", maddr);
		}
		start_finishupgrade();
		if (getbootdevice())
			nvram_seti("bootpartition", 1);
		else
			nvram_seti("bootpartition", 0);
		break;
	case ROUTER_LINKSYS_EA8500:
		if (maddr) {
			set_hwaddr("eth1", maddr);
			set_hwaddr("eth0", maddr);
			nvram_set("lan_hwaddr", maddr);
			nvram_commit();
		}
		if (!nvram_match("nobcreset", "1"))
			eval("mtd", "resetbc", "s_env");
		break;
	default:
		break;
	}

	detect_wireless_devices(RADIO_ALL);

	switch (board) {
	case ROUTER_HABANERO:
		eval("swconfig", "dev", "switch0", "set", "reset", "1");
		eval("swconfig", "dev", "switch0", "set", "enable_vlan", "1");
#ifdef HAVE_ANTAIRA
		eval("insmod", "i2c-gpio-custom", "bus2=2,11,10");
		eval("insmod", "rtc-pcf8523");
		writestr("/sys/class/i2c-dev/i2c-2/device/new_device", "pcf8523 0x68");
		eval("insmod", "gpio-antaira-i2c");
		writestr("/sys/class/i2c-dev/i2c-2/device/new_device", "antairagpio 0x60");
		eval("hwclock", "-s", "-u");
		eval("ledtool", "20", "0");

#endif				/*HAVE_ANTAIRA */
		eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", "0 1 2 3 4");
		eval("swconfig", "dev", "switch0", "vlan", "2", "set", "ports", "0t 5");
		eval("swconfig", "dev", "switch0", "set", "apply");
		eval("ifconfig", "eth0", "up");
		eval("ifconfig", "eth1", "up");

		break;
	case ROUTER_ASROCK_G10:
		eval("swconfig", "dev", "switch0", "set", "reset", "1");
		eval("swconfig", "dev", "switch0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "switch0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "switch0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", "6 2 3 4 5");
		eval("swconfig", "dev", "switch0", "vlan", "2", "set", "ports", "0 1");
		eval("swconfig", "dev", "switch0", "set", "apply");
		eval("ifconfig", "eth0", "up");
		eval("ifconfig", "eth1", "up");

		writeproc("/proc/irq/101/smp_affinity", "2");
		writeproc("/proc/irq/98/smp_affinity", "2");	// move second wifi interface to core 2
		writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
		writestr("/sys/class/leds/ath10k-phy1/trigger", "phy1tpt");
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "800000");
		writeproc("/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq", "800000");
		break;
	default:
		eval("swconfig", "dev", "switch0", "set", "reset", "1");
		eval("swconfig", "dev", "switch0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "switch0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "switch0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", "6 1 2 3 4");
		eval("swconfig", "dev", "switch0", "vlan", "2", "set", "ports", "0 5");
		eval("swconfig", "dev", "switch0", "set", "apply");
		eval("ifconfig", "eth0", "up");
		eval("ifconfig", "eth1", "up");

		writeproc("/proc/irq/101/smp_affinity", "2");
		writeproc("/proc/irq/98/smp_affinity", "2");	// move second wifi interface to core 2
		writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
		writestr("/sys/class/leds/ath10k-phy1/trigger", "phy1tpt");
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "800000");
		writeproc("/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq", "800000");
		break;
	}

	switch (board) {
	case ROUTER_HABANERO:
#ifdef HAVE_ANTAIRA
		eval("insmod", "i2c-gpio-custom", "bus2=2,11,10");
		eval("insmod", "rtc-pcf8523");
		writestr("/sys/class/i2c-dev/i2c-2/device/new_device", "pcf8523 0x68");
		eval("insmod", "gpio-antaira-i2c");
		writestr("/sys/class/i2c-dev/i2c-2/device/new_device", "antairagpio 0x60");
		eval("hwclock", "-s", "-u");
		eval("ledtool", "20", "0");

/*
		if (!nvram_safe_get("sw_cpuport")) {
			nvram_seti("sw_cpuport", 0);
			nvram_seti("sw_wan", 5);
			nvram_seti("sw_lan1", 4);
			nvram_seti("sw_lan2", 3);
			nvram_seti("sw_lan3", 2);
			nvram_seti("sw_lan4", 1);
			nvram_commit();
		}
*/
#endif				/*HAVE_ANTAIRA */
		break;
		/* routers with reverse port order */
	case ROUTER_NETGEAR_R7800:
	case ROUTER_NETGEAR_R7500V2:
	case ROUTER_NETGEAR_R7500:
		nvram_seti("sw_wancpuport", 0);
		nvram_seti("sw_lancpuport", 6);
		nvram_unset("sw_cpuport");
		nvram_seti("sw_wan", 5);
		nvram_seti("sw_lan1", 4);
		nvram_seti("sw_lan2", 3);
		nvram_seti("sw_lan3", 2);
		nvram_seti("sw_lan4", 1);
		break;
	case ROUTER_ASROCK_G10:
		nvram_seti("sw_wancpuport", 0);
		nvram_seti("sw_lancpuport", 6);
		nvram_unset("sw_cpuport");
		nvram_seti("sw_wan", 1);
		nvram_seti("sw_lan1", 2);
		nvram_seti("sw_lan2", 3);
		nvram_seti("sw_lan3", 4);
		nvram_seti("sw_lan4", 5);
		break;
	case ROUTER_LINKSYS_EA8500:
	default:
		nvram_unset("sw_cpuport");
		nvram_seti("sw_wancpuport", 0);
		nvram_seti("sw_lancpuport", 6);
		nvram_seti("sw_wan", 5);
		nvram_seti("sw_lan1", 1);
		nvram_seti("sw_lan2", 2);
		nvram_seti("sw_lan3", 3);
		nvram_seti("sw_lan4", 4);
		break;
	}

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");

	writeproc("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "20");
	writeproc("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate", "1000000");
	writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
	writeproc("/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor", "performance");

	eval("ifconfig", "eth1", "up");
	eval("ifconfig", "eth0", "up");

	char macaddr[32];
	if (get_hwaddr("eth1", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");
	nvram_set("wl1_ifname", "wlan1");
}

void start_resetleds(void)
{
	writestr("/sys/class/leds/ath10k-phy0/trigger", "none");
	writestr("/sys/class/leds/ath10k-phy1/trigger", "none");
	writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
	writestr("/sys/class/leds/ath10k-phy1/trigger", "phy1tpt");
}

void start_postnetwork(void)
{
	int board = getRouterBrand();
	switch (board) {
	case ROUTER_HABANERO:
		break;
	default:
		set_gpio(459-443 + 17, 0);	// reset wifi card gpio pin
		set_gpio(494-443 + 17, 0);	// reset wifi card gpio pin
		set_gpio(459-443 + 17, 1);	// reset wifi card gpio pin
		set_gpio(494-443 + 17, 1);	// reset wifi card gpio pin
		break;
	}
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
