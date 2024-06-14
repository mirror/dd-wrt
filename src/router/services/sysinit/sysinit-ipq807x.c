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

#define ALT_PART_NAME_LENGTH 16
struct per_part_info {
	char name[ALT_PART_NAME_LENGTH];
	uint32_t primaryboot;
};

#define NUM_ALT_PARTITION 9
typedef struct {
#define SMEM_DUAL_BOOTINFO_MAGIC_START 0xA3A2A1A0
#define SMEM_DUAL_BOOTINFO_MAGIC_START_TRYMODE 0xA3A2A1A1
#define SMEM_DUAL_BOOTINFO_MAGIC_END 0xB3B2B1B0
	uint32_t magic;
	uint32_t age;
	uint32_t numaltpart;
	struct per_part_info per_part_entry[NUM_ALT_PARTITION];
	uint32_t magic_end;
} ipq_smem_bootconfig_info_t;

void start_finishupgrade(void)
{
	char mtdpath[64];
	char mtdpath1[64];
	int mtd = getMTD("bootconfig");
	sprintf(mtdpath, "/dev/mtdblock%d", mtd);
	int mtd1 = getMTD("bootconfig1");
	sprintf(mtdpath1, "/dev/mtdblock%d", mtd1);

	ipq_smem_bootconfig_info_t *ipq_smem_bootconfig_info = NULL;

	unsigned int *smem = (unsigned int *)calloc(0x80000, 1);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		fread(smem, 0x80000, 1, fp);
		fclose(fp);
		int i;
		unsigned int *p = smem;
		for (i = 0; i < 0x80000 - sizeof(ipq_smem_bootconfig_info); i += 4) {
			if (*p == SMEM_DUAL_BOOTINFO_MAGIC_START) {
				ipq_smem_bootconfig_info = (ipq_smem_bootconfig_info_t *)p;
				break;
			}
			p++;
		}
	}
	if (ipq_smem_bootconfig_info) {
		int i;
		for (i = 0; i < ipq_smem_bootconfig_info->numaltpart; i++) {
			if (!strncmp(ipq_smem_bootconfig_info->per_part_entry[i].name, "rootfs", 6)) {
				ipq_smem_bootconfig_info->per_part_entry[i].primaryboot = 0;
			}
		}
	}
	ipq_smem_bootconfig_info->age++;
	fp = fopen(mtdpath, "wb");
	if (fp) {
		fwrite(smem, 0x80000, 1, fp);
	}
	fclose(fp);
	fp = fopen(mtdpath1, "wb");
	if (fp) {
		fwrite(smem, 0x80000, 1, fp);
	}
	fclose(fp);
	free(smem);
}

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

void *get_deviceinfo(char *mtd, char *var)
{
	static char res[256];
	bzero(res, sizeof(res));
	FILE *fp = fopen(mtd, "rb");
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
void *get_deviceinfo_mr7350(char *var)
{
	return get_deviceinfo("/dev/mtd13", var);
}
void *get_deviceinfo_mx4200(char *var)
{
	return get_deviceinfo("/dev/mtd20", var);
}

void calcchecksum(void *caldata, int offset, int size)
{
	int i;
	unsigned short *cdata = (unsigned short *)caldata;
	unsigned short *ptr_eeprom = (unsigned short *)caldata;
	unsigned short crc = 0;
	ptr_eeprom += offset / 2;
	cdata[0x5] = 0;
	for (i = 0; i < size - offset; i += 2) {
		crc ^= le16toh(*ptr_eeprom);
		ptr_eeprom++;
	}
	crc = ~crc;
	cdata[0x5] = htole16(crc);
}

void patchmac(char *file, int offset, unsigned char *binmac)
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		char *mem = malloc(len);
		fread(mem, len, 1, fp);
		fclose(fp);
		memcpy(mem + offset, binmac, 6);
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		fwrite(mem, len, 1, fp);
		fclose(fp);
		free(mem);
	}
}

// IPQ8074 regdomain offset 52, 1112, 1280, 1448
// IPQ6018 regdomain offset 52, 1104

void removeregdomain(char *file, int type)
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
		fread(mem, len, 1, fp);
		fclose(fp);
		int regdomain = s[52 / 2];
		s[52 / 2] = 0;
		if (type == 0) {
			s[1104 / 2] = 0;
		} else {
			s[1112 / 2] = 0;
			s[1280 / 2] = 0;
			s[1448 / 2] = 0;
		}
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		fwrite(mem, len, 1, fp);
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
		fread(mem, len, 1, fp);
		fclose(fp);
		s[62 / 2] = 1;
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		fwrite(mem, len, 1, fp);
		fclose(fp);
		free(mem);
	}
}

/*WHAL_OPFLAGS_11G                     = 0x00000002,
WHAL_OPFLAGS_5G_HT40            = 0x00000004,
WHAL_OPFLAGS_2G_HT40            = 0x00000008,
WHAL_OPFLAGS_5G_HT20            = 0x00000010,
WHAL_OPFLAGS_2G_HT20            = 0x00000020,
WHAL_OPFLAGS_5G_VHT20         = 0x00000040,
WHAL_OPFLAGS_2G_VHT20         = 0x00000080,
WHAL_OPFLAGS_5G_VHT40         = 0x00000100,
WHAL_OPFLAGS_2G_VHT40         = 0x00000200,
WHAL_OPFLAGS_5G_VHT80         = 0x00000400,
WHAL_OPFLAGS_5G_VHT80P80  = 0x00000800,
WHAL_OPFLAGS_5G_VHT160      = 0x00001000
*/
void patchvht160(char *file)
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		unsigned int *s;
		unsigned char *mem = malloc(len);
		s = (unsigned int *)mem;
		fread(mem, len, 1, fp);
		fclose(fp);

		//		fprintf(stderr, "old boardflag = %X\n", s[68 / 4]);
		//		s[68 / 4] |= 0x800;
		//		s[68 / 4] |= 0x1000;
		//		fprintf(stderr, "new boardflag = %X\n", s[68 / 4]);

		fprintf(stderr, "old boardflag = %X\n", s[1040 / 4]);
		s[1040 / 4] |= 0x800;
		s[1040 / 4] |= 0x1000;
		fprintf(stderr, "new boardflag = %X\n", s[1040 / 4]);
		calcchecksum(mem, 0, len);
		FILE *fp = fopen(file, "wb");
		fwrite(mem, len, 1, fp);
		fclose(fp);
		free(mem);
	}
}

#define patch(ethaddr, offset)                                                                                               \
	{                                                                                                                    \
		unsigned char binmac[6];                                                                                     \
		int i;                                                                                                       \
		unsigned int newmac[6];                                                                                      \
		sscanf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4], \
		       &newmac[5]);                                                                                          \
		for (i = 0; i < 6; i++)                                                                                      \
			binmac[i] = newmac[i];                                                                               \
		patchmac("/tmp/caldata.bin", offset, binmac);                                                                \
		patchmac("/tmp/board.bin", offset, binmac);                                                                  \
		setmacflag("/tmp/caldata.bin");                                                                              \
		setmacflag("/tmp/board.bin");                                                                                \
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
	if (brand == ROUTER_LINKSYS_MR7350) {
		maddr = get_deviceinfo_mr7350("hw_mac_addr");
		insmod("qca-ssdk-ipq60xx");
		insmod("qca-nss-dp-ipq60xx");
	} else if (brand == ROUTER_DYNALINK_DLWRX36) {
		fwlen = 0x20000;
		insmod("qca-ssdk-ipq807x");
		insmod("qca-nss-dp-ipq807x");
	} else {
		fwlen = 0x20000;
		maddr = get_deviceinfo_mx4200("hw_mac_addr");
		insmod("qca-ssdk-ipq807x");
		insmod("qca-nss-dp-ipq807x");
	}

	insmod("qca-ssdk");
	insmod("qca-nss-dp");
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
	} else {
		fprintf(stderr, "board data failed\n");
	}
	unsigned int newmac[6];
	char ethaddr[32];
	if (maddr) {
		fprintf(stderr, "sysinit using mac %s\n", maddr);
		sscanf(maddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);

		sprintf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", newmac[0] & 0xff, newmac[1] & 0xff, newmac[2] & 0xff,
			newmac[3] & 0xff, newmac[4] & 0xff, newmac[5] & 0xff);
		nvram_set("et0macaddr", ethaddr);
		nvram_set("et0macaddr_safe", ethaddr);
		set_hwaddr("eth0", ethaddr);
		set_hwaddr("eth1", ethaddr);
		set_hwaddr("eth2", ethaddr);
		set_hwaddr("eth3", ethaddr);
		set_hwaddr("eth4", ethaddr);
	}

	if (brand == ROUTER_LINKSYS_MR7350) {
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		removeregdomain("/tmp/caldata.bin", 0);
		removeregdomain("/tmp/board.bin", 0);
	}
	if (brand == ROUTER_LINKSYS_MX4200V2) {
		MAC_ADD(ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		patch(ethaddr, 26);
		removeregdomain("/tmp/caldata.bin", 1);
		removeregdomain("/tmp/board.bin", 1);
	}
	if (brand == ROUTER_LINKSYS_MX4200V1) {
		removeregdomain("/tmp/caldata.bin", 1);
		removeregdomain("/tmp/board.bin", 1);
	}
	eval("modprobe", "ath11k_ahb");
	if (brand == ROUTER_LINKSYS_MR7350 || brand == ROUTER_LINKSYS_MX4200V1 || brand == ROUTER_LINKSYS_MX4200V2) {
		if (!nvram_match("nobcreset", "1"))
			eval("mtd", "resetbc", "s_env");
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
	}
	if (brand == ROUTER_DYNALINK_DLWRX36) {
		set_envtools(getMTD("appsblenv"), "0x0", "0x40000", "0x20000", 2);
	}
	if (brand == ROUTER_LINKSYS_MR7350) {
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


	}

	if (brand == ROUTER_DYNALINK_DLWRX36 || brand == ROUTER_LINKSYS_MX4200V1 || brand == ROUTER_LINKSYS_MX4200V2) {
		writeproc("/proc/irq/64/smp_affinity", "1");
		writeproc("/proc/irq/65/smp_affinity", "2");
		writeproc("/proc/irq/66/smp_affinity", "4");
		writeproc("/proc/irq/67/smp_affinity", "8");

		writeproc("/proc/irq/47/smp_affinity", "1");
		writeproc("/proc/irq/53/smp_affinity", "2");
		writeproc("/proc/irq/56/smp_affinity", "4");

		writeproc("/proc/irq/58/smp_affinity", "2");
		writeproc("/proc/irq/60/smp_affinity", "4");
		writeproc("/proc/irq/62/smp_affinity", "8");

		writeproc("/proc/irq/32/smp_affinity", "4");
		writeproc("/proc/irq/33/smp_affinity", "4");
		writeproc("/proc/irq/34/smp_affinity", "4");
		writeproc("/proc/irq/35/smp_affinity", "4");
	}
	if (brand == ROUTER_DYNALINK_DLWRX36) {
		sysprintf("echo netdev > /sys/class/leds/90000.mdio-1:1c:green:wan/trigger");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:green:wan/link_2500");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:green:wan/tx");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:green:wan/rx");
		sysprintf("echo netdev > /sys/class/leds/90000.mdio-1:1c:yellow:wan/trigger");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:yellow:wan/tx");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:yellow:wan/rx");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:yellow:wan/link_10");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:yellow:wan/link_100");
		sysprintf("echo 1 > /sys/class/leds/90000.mdio-1:1c:yellow:wan/link_1000");
	}

	sysprintf("ssdk_sh debug module_func set servcode 0xf 0x0 0x0");
	sysprintf("ssdk_sh servcode config set 1 n 0 0xfffefc7f 0xffbdff 0 0 0 0 0 0");
	sysprintf("ssdk_sh debug module_func set servcode 0x0 0x0 0x0");
	sysprintf("ssdk_sh acl list create 56 48");
	sysprintf("ssdk_sh acl rule add 56 0 1 n 0 0 mac n n n n n y 01-80-c2-00-00-00 ff-ff-ff-ff-ff-ff n n n n n n n n n n n n n n n n n n n n n n n y n n n n n n n n n n 0 0 n n n n n n n n n n n n n y n n n n n n n n n n n n y n n n n n n n n n n n n 0");
	sysprintf("ssdk_sh acl rule add 56 1 1 n 0 0 mac n n n n n n n yes 0x8809 0xffff n n n n n n n n n n n n n n n n n n n n n y n n n n n n n n n n 0 0 n n n n n n n n n n n n n y n n n n n n n n n n n n y n n n n n n n n n n n n 0");
	sysprintf("ssdk_sh acl rule add 56 2 1 n 0 0 mac n n n n n n n yes 0x888e 0xffff n n n n n n n n n n n n n n n n n n n n n y n n n n n n n n n n 0 0 n n n n n n n n n n n n n y n n n n n n n n n n n n y n n n n n n n n n n n n 0");
	sysprintf("ssdk_sh acl list bind 56 0 2 1");
	sysprintf("ssdk_sh fdb portLearn set 0 disable");
	sysprintf("ssdk_sh fdb portLearn set 1 disable");
	sysprintf("ssdk_sh fdb portLearn set 2 disable");
	sysprintf("ssdk_sh fdb portLearn set 3 disable");
	sysprintf("ssdk_sh fdb portLearn set 4 disable");
	sysprintf("ssdk_sh fdb portLearn set 5 disable");
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
