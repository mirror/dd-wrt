/*
 * sysinit-ipq807x.c
 *
 * Copyright (C) 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include "devices/ethernet.c"
#include "devices/wireless.c"

void set_envtools(int mtd, char *offset, char *envsize, char *blocksize, int nums)
{
	char m[32];
	sprintf(m, "/dev/mtd%d", mtd);
	FILE *fp = fopen("/tmp/fw_env.config", "wb");
	if (fp) {
		if (nums)
			fprintf(fp, "%s\t%s\t%s\t%s\t%d\n", m, offset, envsize, blocksize, nums);
		else
			fprintf(fp, "%s\t%s\t%s\t%s\n", m, offset, envsize, blocksize);
		fclose(fp);
	}
}

void *get_deviceinfo_mr7350(char *var)
{
	static char res[256];
	bzero(res, sizeof(res));
	FILE *fp = fopen("/dev/mtd13", "rb");
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

void *get_deviceinfo_mx4200(char *var)
{
	static char res[256];
	bzero(res, sizeof(res));
	FILE *fp = fopen("/dev/mtd21", "rb");
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

void calcchecksum(void *caldata, int offset, int size)
{
	int i;
	unsigned short *cdata = (unsigned short *)caldata;
	unsigned short *ptr_eeprom = (unsigned short *)caldata;
	unsigned short crc = 0;
	ptr_eeprom += offset / 2;
	fprintf(stderr, "orig checksum %X\n", cdata[5]);
	cdata[0x5] = 0;
	for (i = 0; i < size - offset; i += 2) {
		crc ^= le16toh(*ptr_eeprom);
		ptr_eeprom++;
	}
	crc = ~crc;
	cdata[0x5] = htole16(crc);
	fprintf(stderr, "checksum %X\n", crc);
}

void patchmac(char *file, int offset, char *binmac)
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		char *mem = malloc(len);
		for (i = 0; i < len; i++)
			mem[i] = getc(fp);
		fclose(fp);
		memcpy(mem + offset, binmac, 6);
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		for (i = 0; i < len; i++)
			putc(mem[i], fp);
		fclose(fp);
		free(mem);
	}
}

// IPQ8074 regdomain offset 52, 1112, 1280, 1448
// IPQ6018 regdomain offset 52, 1104

void removeregdomain(char *file)
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		unsigned short *s;
		unsigned char *mem = malloc(len);
		s = (unsigned short *)mem;
		for (i = 0; i < len; i++)
			mem[i] = getc(fp);
		fclose(fp);
		int regdomain = s[52 / 2];
		s[52 / 2] = 0;
		if (s[1112 / 2] == regdomain)
			s[1112 / 2] = 0;
		if (s[1280 / 2] == regdomain)
			s[1280 / 2] = 0;
		if (s[1448 / 2] == regdomain)
			s[1448 / 2] = 0;
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		for (i = 0; i < len; i++)
			putc(mem[i], fp);
		fclose(fp);
		free(mem);
	}
}

void setmacflag(char *file)
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		unsigned short *s;
		unsigned char *mem = malloc(len);
		s = (unsigned short *)mem;
		for (i = 0; i < len; i++)
			mem[i] = getc(fp);
		fclose(fp);
		s[62 / 2] = 1;
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		for (i = 0; i < len; i++)
			putc(mem[i], fp);
		fclose(fp);
		free(mem);
	}
}

void chksum_main(int argc, char *argv[])
{
	FILE *fp = fopen("/tmp/caldata.bin", "rb");
	int offset = atoi(argv[1]);
	int size = atoi(argv[2]);
	char *mem = malloc(0x10000);
	int i;
	for (i = 0; i < 0x10000; i++)
		mem[i] = getc(fp);
	fclose(fp);
	calcchecksum(mem, offset, size);
}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	char dev[64];
	char mtdpath[64];

	if (!nvram_matchi("disable_watchdog", 1)) {
		insmod("imx2_wdt");
	}

	/*
	 * Setup console 
	 */
	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();
	char *maddr = NULL;
	int fwlen = 0x10000;
	if (brand == ROUTER_LINKSYS_MR7350)
		maddr = get_deviceinfo_mr7350("hw_mac_addr");
	else {
		fwlen = 0x20000;
		maddr = get_deviceinfo_mx4200("hw_mac_addr");
	}

	insmod("qca-ssdk");
	insmod("qca-nss-dp");
	eval("modprobe", "ath11k_ahb");
	int mtd = getMTD("art");
	if (mtd == -1)
		mtd = getMTD("ART");
	int uenv = getMTD("u_env");
	sprintf(mtdpath, "/dev/mtd%d", mtd);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		fseek(fp, 0x1000, SEEK_SET);
		int i;
		FILE *out = fopen("/tmp/caldata.bin", "wb");
		for (i = 0; i < fwlen; i++)
			putc(getc(fp), out);
		fclose(out);
		fseek(fp, 0x1000, SEEK_SET);
		out = fopen("/tmp/board.bin", "wb");
		for (i = 0; i < fwlen; i++)
			putc(getc(fp), out);
		fclose(out);
		fclose(fp);
	}
	if (!nvram_match("nobcreset", "1"))
		eval("mtd", "resetbc", "s_env");
	set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
	unsigned int newmac[6];
	unsigned char binmac[6];
	if (maddr) {
		fprintf(stderr, "sysinit using mac %s\n", maddr);
		sscanf(maddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
	}

	char ethaddr[32];
	sprintf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", newmac[0] & 0xff, newmac[1] & 0xff, newmac[2] & 0xff, newmac[3] & 0xff,
		newmac[4] & 0xff, newmac[5] & 0xff);
	nvram_set("et0macaddr", ethaddr);
	nvram_set("et0macaddr_safe", ethaddr);
	set_hwaddr("eth0", ethaddr);
	set_hwaddr("eth1", ethaddr);
	set_hwaddr("eth2", ethaddr);
	set_hwaddr("eth3", ethaddr);
	set_hwaddr("eth4", ethaddr);

	if (brand == ROUTER_LINKSYS_MR7350) {
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		sscanf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
		int i;
		for (i = 0; i < 6; i++)
			binmac[i] = newmac[i];
		patchmac("/tmp/caldata.bin", 14, binmac);
		patchmac("/tmp/board.bin", 14, binmac);
		sysprintf("echo %s > /sys/devices/platform/soc@0/c000000.wifi/ieee80211/phy0/macaddress", ethaddr);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		sscanf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
		for (i = 0; i < 6; i++)
			binmac[i] = newmac[i];
		patchmac("/tmp/caldata.bin", 20, binmac);
		patchmac("/tmp/board.bin", 20, binmac);
		sysprintf("echo %s > /sys/devices/platform/soc@0/c000000.wifi/ieee80211/phy1/macaddress", ethaddr);
		writeproc("/proc/irq/61/smp_affinity", "1");
		writeproc("/proc/irq/62/smp_affinity", "2");
		writeproc("/proc/irq/63/smp_affinity", "4");
		writeproc("/proc/irq/64/smp_affinity", "8");

		writeproc("/proc/irq/47/smp_affinity", "1");
		writeproc("/proc/irq/53/smp_affinity", "2");
		writeproc("/proc/irq/56/smp_affinity", "4");

		writeproc("/proc/irq/57/smp_affinity", "2");
		writeproc("/proc/irq/59/smp_affinity", "4");

		writeproc("/proc/irq/33/smp_affinity", "4");
		writeproc("/proc/irq/34/smp_affinity", "4");
		writeproc("/proc/irq/35/smp_affinity", "4");
		writeproc("/proc/irq/36/smp_affinity", "4");
		setmacflag("/tmp/caldata.bin");
		setmacflag("/tmp/board.bin");
	}
	if (brand == ROUTER_LINKSYS_MX4200V2) {
		MAC_ADD(ethaddr);
		sscanf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
		int i;
		for (i = 0; i < 6; i++)
			binmac[i] = newmac[i];
		patchmac("/tmp/caldata.bin", 20, binmac);
		patchmac("/tmp/board.bin", 20, binmac);

		MAC_ADD(ethaddr);
		sscanf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
		int i;
		for (i = 0; i < 6; i++)
			binmac[i] = newmac[i];
		patchmac("/tmp/caldata.bin", 14, binmac);
		patchmac("/tmp/board.bin", 14, binmac);

		MAC_ADD(ethaddr);
		sscanf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
		int i;
		for (i = 0; i < 6; i++)
			binmac[i] = newmac[i];
		patchmac("/tmp/caldata.bin", 26, binmac);
		patchmac("/tmp/board.bin", 26, binmac);
		setmacflag("/tmp/caldata.bin");
		setmacflag("/tmp/board.bin");
	}

	removeregdomain("/tmp/caldata.bin");
	removeregdomain("/tmp/board.bin");

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

char *set_wan_state(int state)
{
	return NULL;
}

void start_devinit_arch(void)
{
}
