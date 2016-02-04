/*
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <cymac.h>

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

	unsigned int *smem = (unsigned int *)malloc(0x60000);
	memset(smem, 0, 0x60000);
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

	unsigned int *smem = (unsigned int *)malloc(0x60000);
	memset(smem, 0, 0x60000);
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

	unsigned int *smem = (unsigned int *)malloc(0x60000);
	memset(smem, 0, 0x60000);
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
				ipq_smem_bootconfig_v2_info->per_part_entry[i].primaryboot = dev;
			}
		}
		ipq_smem_bootconfig_v2_info->upgradeinprogress = 0;
	}
	if (ipq_smem_bootconfig_info) {
		fprintf(stderr, "upgrade in progress: %d\n", ipq_smem_bootconfig_info->upgradeinprogress);

		int i;
		for (i = 0; i < ipq_smem_bootconfig_info->numaltpart; i++) {
			if (!strncmp(ipq_smem_bootconfig_info->per_part_entry[i].name, "rootfs", 6)) {
				fprintf(stderr, "set bootdevice from %d to %d\n", ipq_smem_bootconfig_info->per_part_entry[i].primaryboot, dev);
				ipq_smem_bootconfig_info->per_part_entry[i].primaryboot = dev;
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
	memset(res, 0, sizeof(res));
	FILE *fp = fopen("/dev/mtdblock/12", "rb");
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

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	FILE *fp;
	if (!nvram_match("disable_watchdog", "1"))
		eval("watchdog");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	char mtdpath[64];
	int board = getRouterBrand();
	// this is for TEW827 only. i dont know how it works for other boards. offsets might be different
	int mtd = getMTD("art");
	char *maddr = NULL;
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
	if (board != ROUTER_NETGEAR_R7500)
		fp = fopen(mtdpath, "rb");
	if (fp) {
		int newmac[6];
		if (board == ROUTER_TRENDNET_TEW827)
			maddr = getUEnv("lan_mac");
		if (board == ROUTER_LINKSYS_EA8500)
			maddr = get_deviceinfo("hw_mac_addr");

		if (maddr) {
			fprintf(stderr, "sysinit using mac %s\n", maddr);
			sscanf(maddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4], &newmac[5]);
		}

		fseek(fp, 0x1000, SEEK_SET);
		char *smem = malloc(0x8000);
		fread(smem, 0x8000, 1, fp);

		fclose(fp);
		if (maddr && (board == ROUTER_TRENDNET_TEW827 || board == ROUTER_LINKSYS_EA8500)) {	// board calibration data with real mac addresses
			int i;
			for (i = 0; i < 6; i++) {
				smem[i + 6] = newmac[i];
				smem[i + 6 + 0x4000] = newmac[i];
			}
		}
		calcchecksum(smem);
		calcchecksum(&smem[0x4000]);

		fp = fopen("/tmp/board1.bin", "wb");
		fwrite(smem, 0x4000, 1, fp);
		fclose(fp);
		fp = fopen("/tmp/board2.bin", "wb");
		fwrite(&smem[0x4000], 0x4000, 1, fp);
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
	/*
	 * network drivers 
	 */

	//insmod("qdpc-host.ko");
	eval("mount", "-t", "ubifs", "-o", "sync", "ubi0:rootfs_data", "/jffs");

	switch (board) {
	case ROUTER_TRENDNET_TEW827:
		if (maddr) {
			eval("ifconfig", "eth0", "hw", "ether", getUEnv("wan_mac"));
			eval("ifconfig", "eth1", "hw", "ether", getUEnv("lan_mac"));
		}
		start_finishupgrade();
		if (getbootdevice())
			nvram_set("bootpartition", "1");
		else
			nvram_set("bootpartition", "0");
		break;
	case ROUTER_LINKSYS_EA8500:
		if (maddr) {
			eval("ifconfig", "eth1", "hw", "ether", maddr);
			eval("ifconfig", "eth0", "hw", "ether", maddr);
			nvram_set("lan_hwaddr", maddr);
			nvram_commit();
		}
		eval("mtd", "resetbc", "s_env");
		break;
	default:
		break;
	}

	switch (board) {
	case ROUTER_LINKSYS_EA8500:
		system("swconfig dev switch0 set reset 1");
		system("swconfig dev switch0 set enable_vlan 1");
		system("swconfig dev switch0 vlan 1 set ports \"0t 1 2 3 4\"");
		system("swconfig dev switch0 vlan 2 set ports \"0t 5\"");
		system("swconfig dev switch0 set apply");
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		break;
	default:
		system("swconfig dev switch0 set reset 1");
		system("swconfig dev switch0 set enable_vlan 0");
		system("swconfig dev switch0 vlan 1 set ports \"6 1 2 3 4\"");
		system("swconfig dev switch0 vlan 2 set ports \"5 0\"");
		system("swconfig dev switch0 set apply");
		break;
	}

	eval("ifconfig", "eth1", "up");
	eval("ifconfig", "eth0", "up");

	detect_wireless_devices();

	int s;
	struct ifreq ifr;
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		char macaddr[32];

		strcpy(macaddr, ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
		close(s);
	}

	switch (board) {
	case ROUTER_LINKSYS_EA8500:
		eval("ifconfig", "eth1", "down");
		break;
	default:
		break;
	}

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");
	nvram_set("wl1_ifname", "ath1");
}

int check_cfe_nv(void)
{
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
