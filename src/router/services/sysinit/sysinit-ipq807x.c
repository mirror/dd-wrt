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
#include <ctype.h>
#include <services.h>
#include <wlutils.h>

#include "devices/ethernet.c"
#include "devices/wireless.c"

#define sys_reboot()  \
	eval("sync"); \
	eval("event", "3", "1", "15")

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
void *get_deviceinfo_fap(char *var)
{
	return get_deviceinfo("/dev/mtd15", var);
}
void *get_deviceinfo_mr5500(char *var)
{
	return get_deviceinfo("/dev/mtd11", var);
}
void *get_deviceinfo_mx4200(char *var)
{
	return get_deviceinfo("/dev/mtd20", var);
}
void *get_deviceinfo_wxr(char *var)
{
	return get_deviceinfo("/dev/mtd14", var);
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

void set6g(char *file) // testing
{
	FILE *fp = fopen(file, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		unsigned char *mem = malloc(len);
		fread(mem, len, 1, fp);
		fclose(fp);
		mem[605] = 1;
		mem[606] = 1;
		mem[607] = 7;
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
#define IPQ6018 0
#define IPQ5018 0
#define QCN9000 0
#define IPQ8074 1

void patchvht160(char *file, int phynum)
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

		//		fprintf(stderr, "old boardflag = %X\n", s[1040 / 4]);
		switch (phynum) {
		case 0:
			s[1040 / 4] |= 0x800;
			s[1040 / 4] |= 0x1000;
			break;
		case 1:
			s[1280 / 4] |= 0x800;
			s[1280 / 4] |= 0x1000;
			break;
		case 2:
			s[1376 / 4] |= 0x800;
			s[1376 / 4] |= 0x1000;
			break;
		}
		//		fprintf(stderr, "new boardflag = %X\n", s[1040 / 4]);
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
		sscanf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4], \
		       &newmac[5]);                                                                                          \
		for (i = 0; i < 6; i++)                                                                                      \
			binmac[i] = newmac[i];                                                                               \
		patchmac("/tmp/caldata.bin", offset, binmac);                                                                \
		patchmac("/tmp/board.bin", offset, binmac);                                                                  \
		setmacflag("/tmp/caldata.bin");                                                                              \
		setmacflag("/tmp/board.bin");                                                                                \
	}

#define patch2(ethaddr, offset)                                                                                              \
	{                                                                                                                    \
		unsigned char binmac[6];                                                                                     \
		int i;                                                                                                       \
		unsigned int newmac[6];                                                                                      \
		sscanf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4], \
		       &newmac[5]);                                                                                          \
		for (i = 0; i < 6; i++)                                                                                      \
			binmac[i] = newmac[i];                                                                               \
		patchmac("/tmp/cal-pci-0001:01:00.0.bin", offset, binmac);                                                   \
		patchmac("/tmp/caldata2.bin", offset, binmac);                                                               \
		patchmac("/tmp/board2.bin", offset, binmac);                                                                 \
	}

static void set_memprofile(int cpus, int cores, int profile)
{
	switch (profile) {
	case 256:
		sysprintf("echo 3100000 > /proc/sys/dev/nss/n2hcfg/extra_pbuf_core0");
		sysprintf("echo 30258 > /proc/sys/dev/nss/n2hcfg/n2h_high_water_core0");
		sysprintf("echo 4096 > /proc/sys/dev/nss/n2hcfg/n2h_wifi_pool_buf");
		break;
	case 512:
		sysprintf("echo 3100000 > /proc/sys/dev/nss/n2hcfg/extra_pbuf_core0");
		sysprintf("echo 30624 > /proc/sys/dev/nss/n2hcfg/n2h_high_water_core0");
		sysprintf("echo 8192 > /proc/sys/dev/nss/n2hcfg/n2h_wifi_pool_buf");
		break;
	case 1024:
		sysprintf("echo 10000000 > /proc/sys/dev/nss/n2hcfg/extra_pbuf_core0");
		sysprintf("echo 72512 > /proc/sys/dev/nss/n2hcfg/n2h_high_water_core0");
		sysprintf("echo 36864 > /proc/sys/dev/nss/n2hcfg/n2h_wifi_pool_buf");
		break;
	}

	if (cores == 2)
		sysprintf("echo 2048 > /proc/sys/dev/nss/n2hcfg/n2h_queue_limit_core1");
	sysprintf("echo 2048 > /proc/sys/dev/nss/n2hcfg/n2h_queue_limit_core0");
	sysprintf("echo %d > /proc/sys/dev/nss/rps/hash_bitmap", (1 << cpus) - 1);
}

static void init_skb(int profile, int maple)
{
	int max_skbs = 1024;
	int max_spare_skbs = 256;
	int skb_recycler_enable = 1;

	if (maple && profile == 512) {
		max_skbs = 512;
		max_spare_skbs = 128;
		skb_recycler_enable = 0;
	}
	sysprintf("echo %d > /proc/net/skb_recycler/max_skbs", max_skbs);
	sysprintf("echo %d > /proc/net/skb_recycler/max_spare_skbs", max_spare_skbs);
	sysprintf("echo %d > /proc/net/skb_recycler/skb_recycler_enable", skb_recycler_enable);
	if (!skb_recycler_enable)
		sysprintf("echo %d > /proc/net/skb_recycler/flush", 1);
}
static int use_mesh(void)
{
	int count;
	char *next;
	char var[80];
	char *vifs;
	nvram_set("cur_nss", "11.4");
	for (count = 0; count < 3; count++) {
		char wifivifs[32];
		sprintf(wifivifs, "wlan%d_vifs", count);
		if (nvram_nmatch("mesh", "wlan%d_mode", count) && !nvram_nmatch("disabled", "wlan%d_net_mode", count))
			return 1;
		vifs = nvram_safe_get(wifivifs);
		if (vifs != NULL && *vifs) {
			foreach(var, vifs, next)
			{
				if (nvram_nmatch("mesh", "%s_mode", var) && !nvram_nmatch("disabled", "%s_net_mode", var))
					return 1;
			}
		}
	}
	nvram_set("cur_nss", "12.5");
	return 0;
}
static void load_nss_ipq60xx(int profile)
{
	init_skb(profile, 0);
	insmod("qca-ssdk-ipq60xx");
	if (profile == 256)
		eval_silence("insmod", "qca-nss-dp-ipq60xx", "mem_profile=2", use_mesh() ? "mesh=1" : "mesh=0");
	else if (profile == 512)
		eval_silence("insmod", "qca-nss-dp-ipq60xx", "mem_profile=1", use_mesh() ? "mesh=1" : "mesh=0");
	else
		eval_silence("insmod", "qca-nss-dp-ipq60xx", "mem_profile=0", use_mesh() ? "mesh=1" : "mesh=0");

	nvram_default_get("nss", "1");
	if (nvram_match("nss", "1")) {
		insmod("qca-nss-drv-ipq60xx");
		insmod("qca-nss-crypto-ipq60xx");
		insmod("qca-nss-cfi-cryptoapi-ipq60xx");
		insmod("qca-nss-netlink-ipq60xx");
		insmod("cryptodev");

		set_memprofile(4, 1, profile);

		eval_silence("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
		insmod("qca-nss-pppoe-ipq60xx");
		insmod("qca-nss-vlan-ipq60xx");
		insmod("qca-nss-qdisc-ipq60xx");
		insmod("pptp");
		insmod("qca-nss-pptp-ipq60xx");
		insmod("udp_tunnel");
		insmod("ip6_udp_tunnel");
		insmod("l2tp_core");
		insmod("qca-nss-l2tpv2-ipq60xx");
		insmod("vxlan");
		insmod("qca-nss-vxlanmgr-ipq60xx");
		insmod("tunnel6");
		insmod("ip6_tunnel");
		insmod("qca-nss-tunipip6-ipq60xx");
		insmod("qca-nss-tlsmgr-ipq60xx");
		insmod("qca-mcs");
		insmod("nss-ifb");
		insmod("qca-nss-netlink-ipq60xx");
		insmod("qca-nss-bridge-mgr-ipq60xx");
		insmod("qca-nss-wifi-meshmgr");
	}
}

static void load_nss_ipq50xx(int profile)
{
	init_skb(profile, 1);
	insmod("qca-ssdk-ipq50xx");
	if (profile == 256)
		eval_silence("insmod", "qca-nss-dp-ipq50xx", "mem_profile=2", use_mesh() ? "mesh=1" : "mesh=0");
	else if (profile == 512)
		eval_silence("insmod", "qca-nss-dp-ipq50xx", "mem_profile=1", use_mesh() ? "mesh=1" : "mesh=0");
	else
		eval_silence("insmod", "qca-nss-dp-ipq50xx", "mem_profile=0", use_mesh() ? "mesh=1" : "mesh=0");

	nvram_default_get("nss", "1");
	if (nvram_match("nss", "1")) {
		insmod("qca-nss-drv-ipq50xx");
		insmod("qca-nss-crypto-ipq50xx");
		insmod("qca-nss-cfi-cryptoapi-ipq50xx");
		insmod("qca-nss-netlink-ipq50xx");
		insmod("cryptodev");
		set_memprofile(2, 1, profile);

		eval_silence("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
		insmod("qca-nss-pppoe-ipq50xx");
		insmod("qca-nss-vlan-ipq50xx");
		insmod("qca-nss-qdisc-ipq50xx");
		insmod("pptp");
		insmod("qca-nss-pptp-ipq50xx");
		insmod("udp_tunnel");
		insmod("ip6_udp_tunnel");
		insmod("l2tp_core");
		insmod("qca-nss-l2tpv2-ipq50xx");
		insmod("vxlan");
		insmod("qca-nss-vxlanmgr-ipq50xx");
		insmod("tunnel6");
		insmod("ip6_tunnel");
		insmod("qca-nss-tunipip6-ipq50xx");
		insmod("qca-nss-tlsmgr-ipq50xx");
		insmod("qca-mcs");
		insmod("nss-ifb");
		insmod("qca-nss-bridge-mgr-ipq50xx");
		insmod("qca-nss-wifi-meshmgr");
	}
}
static void load_nss_ipq807x(int profile)
{
	init_skb(profile, 0);
	insmod("qca-ssdk-ipq807x");
	if (profile == 256)
		eval_silence("insmod", "qca-nss-dp-ipq807", "mem_profile=2", use_mesh() ? "mesh=1" : "mesh=0");
	else if (profile == 512)
		eval_silence("insmod", "qca-nss-dp-ipq807x", "mem_profile=1", use_mesh() ? "mesh=1" : "mesh=0");
	else
		eval_silence("insmod", "qca-nss-dp-ipq807x", "mem_profile=0", use_mesh() ? "mesh=1" : "mesh=0");

	nvram_default_get("nss", "1");
	if (nvram_match("nss", "1")) {
		insmod("udp_tunnel");
		insmod("ip6_udp_tunnel");
		insmod("tunnel6");
		insmod("ip6_tunnel");
		insmod("l2tp_core");
		insmod("pptp");
		insmod("vxlan");
		insmod("qca-nss-drv-ipq807x");
		insmod("qca-nss-crypto-ipq807x");
		insmod("qca-nss-cfi-cryptoapi-ipq807x");
		insmod("qca-nss-netlink-ipq807x");
		insmod("cryptodev");

		set_memprofile(4, 2, profile);

		eval_silence("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
		insmod("qca-nss-pppoe-ipq807x");
		insmod("qca-nss-vlan-ipq807x");
		insmod("qca-nss-qdisc-ipq807x");
		insmod("pptp");
		insmod("qca-nss-pptp-ipq807x");
		insmod("udp_tunnel");
		insmod("ip6_udp_tunnel");
		insmod("l2tp_core");
		insmod("qca-nss-l2tpv2-ipq807x");
		insmod("vxlan");
		insmod("qca-nss-vxlanmgr-ipq807x");
		insmod("tunnel6");
		insmod("ip6_tunnel");
		insmod("qca-nss-tunipip6-ipq807x");
		insmod("qca-nss-tlsmgr-ipq807x");
		insmod("qca-mcs");
		insmod("nss-ifb");
		insmod("qca-nss-netlink-ipq807x");
		insmod("qca-nss-bridge-mgr-ipq807x");
		insmod("qca-nss-wifi-meshmgr");
	}
}

void start_setup_affinity(void)
{
	int brand = getRouterBrand();
	if (!nvram_match("ath11k_affinity", "0")) {
		switch (brand) {
		case ROUTER_LINKSYS_MR5500:
		case ROUTER_LINKSYS_MX5500:
			/* IPQ5018 is dualcore arm64, so we need a different affinity stragedy */

			/* QCN9074 external mpci */
			set_named_smp_affinity("DP_EXT_IRQ", 0, 1);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 2);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 3);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 4);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 5);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 6);
			set_named_smp_affinity("DP_EXT_IRQ", 0, 7);
			set_named_smp_affinity("DP_EXT_IRQ", 0, 8);
			/* IPQ5018 AHB wifi */
			set_named_smp_affinity("wbm2host-tx-completions-ring1", 1, 1);
			set_named_smp_affinity("wbm2host-tx-completions-ring2", 0, 1);
			set_named_smp_affinity("reo2host-destination-ring1", 1, 1);
			set_named_smp_affinity("reo2host-destination-ring2", 1, 1);
			set_named_smp_affinity("reo2host-destination-ring3", 1, 1);
			set_named_smp_affinity("reo2host-destination-ring4", 1, 1);

			sysprintf("echo 1 > /proc/sys/dev/nss/rps/enable");
			break;
		default:
			set_named_smp_affinity("reo2host-destination-ring1", 0, 1);
			set_named_smp_affinity("reo2host-destination-ring2", 1, 1);
			set_named_smp_affinity("reo2host-destination-ring3", 2, 1);
			set_named_smp_affinity("reo2host-destination-ring4", 3, 1);

			set_named_smp_affinity("wbm2host-tx-completions-ring1", 1, 1);
			set_named_smp_affinity("wbm2host-tx-completions-ring2", 2, 1);
			set_named_smp_affinity("wbm2host-tx-completions-ring3", 3, 1);

			set_named_smp_affinity("ppdu-end-interrupts-mac1", 1, 1);
			set_named_smp_affinity("ppdu-end-interrupts-mac2", 2, 1);
			set_named_smp_affinity("ppdu-end-interrupts-mac3", 3, 1);

			set_named_smp_affinity("edma_txcmpl", 3, 1);
			set_named_smp_affinity("edma_rxfill", 3, 1);
			set_named_smp_affinity("edma_rxdesc", 3, 1);
			set_named_smp_affinity("edma_misc", 3, 1);

			set_named_smp_affinity_list("nss_queue0", "0 1 2 3", 1);
			set_named_smp_affinity("nss_queue1", 1, 1);
			set_named_smp_affinity("nss_queue2", 2, 1);
			set_named_smp_affinity("nss_queue3", 3, 1);

			set_named_smp_affinity("nss_queue0", 3, 2);

			set_named_smp_affinity_list("nss_empty_buf_sos", "1 2 3", 1);
			set_named_smp_affinity_list("nss_empty_buf_queue", "2 3", 1);
			set_named_smp_affinity("nss_empty_buf_sos", 3, 2);

			set_named_smp_affinity("ppdu-end-interrupts-mac1", 1, 1);
			set_named_smp_affinity("ppdu-end-interrupts-mac3", 2, 1);

			sysprintf("echo 1 > /proc/sys/dev/nss/rps/enable");
			break;
		}
	}
}
void start_initvlans(void)
{
	int brand = getRouterBrand();
	int i;
	switch (brand) {
	case ROUTER_LINKSYS_MR5500:
		/* setup vlan config */

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "port", "frameMaxSize", "set", "2", "0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "2", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "2", "enable");

		/*config port.5 to VLAN(1) and port.1/2/3/4 to VLAN(2) */
		sysprintf("echo 1 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "vlan", "entry", "flush");

		eval_silence("ssdk_sh", "vlan", "entry", "append", "0", "0", "6,5,4,3,2,1", "6", "1,2,3,4,5", "default", "default",
			     "default");

		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "1", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "2", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "3", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "4", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "5", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "6", "fallback");

		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "1", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "2", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "3", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "4", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "5", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "6", "0");
		eval_silence("ssdk_sh", "portVlan", "egress", "set", "1", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "1", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "1", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "2", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "2", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "2", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "3", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "3", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "3", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "4", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "4", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "4", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "5", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "5", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "5", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "6", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "qinqrole", "set", "6", "core");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "6", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "6", "disable");

		eval_silence("ssdk_sh", "portVlan", "qinqmode", "set", "stag");
		eval_silence("ssdk_sh", "portVlan", "svlanTPID", "set", "0x8100");

		eval_silence("ssdk_sh", "port", "poweron", "set", "5");
		eval_silence("ssdk_sh", "fdb", "entry", "flush", "0");

		/*drop invalid tcp*/
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x200", "0x2000", "4");
		/* drop tcp/udp checksum errors */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x204", "0x0842", "4");
		/* enable pppoe */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x214", "0x2000000", "4");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "6", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "6", "enable");

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "port", "frameMaxSize", "set", "2", "0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "2", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "2", "enable");

		/*config port.5 to VLAN(1) and port.1/2/3/4 to VLAN(2) */
		sysprintf("echo 1 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "vlan", "entry", "flush");

		eval_silence("ssdk_sh", "vlan", "entry", "append", "0", "0", "6,5,4,3,2,1", "6", "1,2,3,4,5", "default", "default",
			     "default");

		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "1", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "2", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "3", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "4", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "5", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "6", "fallback");

		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "1", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "2", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "3", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "4", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "5", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "6", "0");
		eval_silence("ssdk_sh", "portVlan", "egress", "set", "1", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "1", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "1", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "2", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "2", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "2", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "3", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "3", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "3", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "4", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "4", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "4", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "5", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "5", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "5", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "6", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "qinqrole", "set", "6", "core");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "6", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "6", "disable");

		eval_silence("ssdk_sh", "portVlan", "qinqmode", "set", "stag");
		eval_silence("ssdk_sh", "portVlan", "svlanTPID", "set", "0x8100");

		eval_silence("ssdk_sh", "port", "poweron", "set", "5");
		eval_silence("ssdk_sh", "fdb", "entry", "flush", "0");

		/*drop invalid tcp*/
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x200", "0x2000", "4");
		/* drop tcp/udp checksum errors */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x204", "0x0842", "4");
		/* enable pppoe */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x214", "0x2000000", "4");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "6", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "6", "enable");
		for (i = 1; i < 5; i++) {
			char id[32];
			sprintf(id, "%d", i);
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0", "0x9000");
		}
		for (i = 1; i < 5; i++) {
			char id[32];
			sprintf(id, "%d", i);
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1d", "0xb");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1e", "0x3c40");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1d", "0x29");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1e", "0x36dd");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x7");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x801a");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x4007");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x382a");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x7");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x3c");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x4007");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x00");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x00", "0x1200");
		}
		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "debug", "uniphy", "set", "0", "0x24", "0x54", "4");
		eval_silence("ssdk_sh", "debug", "uniphy", "set", "0", "0x21c", "0x288a", "4");
		eval_silence("ssdk_sh", "debug", "uniphy", "set", "0", "0x19c", "0xbea0", "4");
		sysprintf("echo 1 > /sys/ssdk/dev_id");

		break;
	case ROUTER_LINKSYS_MX5500:
		/* setup vlan config */

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "port", "frameMaxSize", "set", "2", "0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "2", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "2", "enable");

		/* config port.2 to VLAN(1) and port.3/4/5 to VLAN(2) */
		sysprintf("echo 1 > /sys/ssdk/dev_id");

		eval_silence("ssdk_sh", "vlan", "entry", "flush");

		eval_silence("ssdk_sh", "vlan", "entry", "append", "0", "0", "6,2,3,4,5", "6", "2,3,4,5", "default", "default",
			     "default");

		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "2", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "3", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "4", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "5", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "6", "fallback");

		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "2", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "3", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "4", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "5", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "6", "0");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "2", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "2", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "2", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "3", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "3", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "3", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "4", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "4", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "4", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "5", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "5", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "5", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "6", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "qinqrole", "set", "6", "core");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "6", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "6", "disable");

		eval_silence("ssdk_sh", "portVlan", "qinqmode", "set", "stag");
		eval_silence("ssdk_sh", "portVlan", "svlanTPID", "set", "0x8100");
		eval_silence("ssdk_sh", "port", "poweron", "set", "2");
		eval_silence("ssdk_sh", "fdb", "entry", "flush", "0");
		/*drop invalid tcp*/
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x200", "0x2000", "4");
		/* drop tcp/udp checksum errors */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x204", "0x0842", "4");
		/* enable pppoe */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x214", "0x2000000", "4");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "6", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "6", "enable");

		/* setup vlan config */

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "port", "frameMaxSize", "set", "2", "0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "2", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "2", "enable");

		/* config port.2 to VLAN(1) and port.3/4/5 to VLAN(2) */
		sysprintf("echo 1 > /sys/ssdk/dev_id");

		eval_silence("ssdk_sh", "vlan", "entry", "flush");

		eval_silence("ssdk_sh", "vlan", "entry", "append", "0", "0", "6,2,3,4,5", "6", "2,3,4,5", "default", "default",
			     "default");

		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "2", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "3", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "4", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "5", "fallback");
		eval_silence("ssdk_sh", "portVlan", "ingress", "set", "6", "fallback");

		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "2", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "3", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "4", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "5", "0");
		eval_silence("ssdk_sh", "portVlan", "defaultSVid", "set", "6", "0");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "2", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "2", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "2", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "3", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "3", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "3", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "4", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "4", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "4", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "5", "unmodified");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "5", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "5", "enable");

		eval_silence("ssdk_sh", "portVlan", "egress", "set", "6", "tagged");
		eval_silence("ssdk_sh", "portVlan", "qinqrole", "set", "6", "core");
		eval_silence("ssdk_sh", "portVlan", "vlanPropagation", "set", "6", "disable");
		eval_silence("ssdk_sh", "portVlan", "tlsMode", "set", "6", "disable");

		eval_silence("ssdk_sh", "portVlan", "qinqmode", "set", "stag");
		eval_silence("ssdk_sh", "portVlan", "svlanTPID", "set", "0x8100");
		eval_silence("ssdk_sh", "port", "poweron", "set", "2");
		eval_silence("ssdk_sh", "fdb", "entry", "flush", "0");
		/*drop invalid tcp*/
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x200", "0x2000", "4");
		/* drop tcp/udp checksum errors */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x204", "0x0842", "4");
		/* enable pppoe */
		//		eval_silence("ssdk_sh", "debug", "reg", "set", "0x214", "0x2000000", "4");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval_silence("ssdk_sh", "port", "flowctrlforcemode", "set", "6", "enable");
		eval_silence("ssdk_sh", "port", "flowctrl", "set", "6", "enable");
		for (i = 1; i < 5; i++) {
			char id[32];
			sprintf(id, "%d", i);
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0", "0x9000");
		}
		for (i = 1; i < 5; i++) {
			char id[32];
			sprintf(id, "%d", i);
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1d", "0xb");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1e", "0x3c40");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1d", "0x29");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x1e", "0x36dd");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x7");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x801a");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x4007");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x382a");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x7");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x3c");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xd", "0x4007");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0xe", "0x00");
			eval_silence("ssdk_sh", "debug", "phy", "set", id, "0x00", "0x1200");
		}
		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval_silence("ssdk_sh", "debug", "uniphy", "set", "0", "0x24", "0x54", "4");
		eval_silence("ssdk_sh", "debug", "uniphy", "set", "0", "0x21c", "0x288a", "4");
		eval_silence("ssdk_sh", "debug", "uniphy", "set", "0", "0x19c", "0xbea0", "4");
		sysprintf("echo 1 > /sys/ssdk/dev_id");

		break;
	}
}

static void setscaling(int maxfreq)
{
	char max[32];
	sprintf(max, "%d", maxfreq);
	writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "ondemand");
	if (maxfreq)
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", max);
	writeproc("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate", "1000000");
	writeproc("/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor", "10");
	writeproc("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "50");
}
static void disableportlearn(void)
{
	eval_silence("ssdk_sh", "fdb", "portLearn", "set", "0", "disable");
	eval_silence("ssdk_sh", "fdb", "portLearn", "set", "1", "disable");
	eval_silence("ssdk_sh", "fdb", "portLearn", "set", "2", "disable");
	eval_silence("ssdk_sh", "fdb", "portLearn", "set", "3", "disable");
	eval_silence("ssdk_sh", "fdb", "portLearn", "set", "4", "disable");
	eval_silence("ssdk_sh", "fdb", "portLearn", "set", "5", "disable");
	eval_silence("ssdk_sh", "stp", "portState", "set", "0", "0", "forward");
	eval_silence("ssdk_sh", "stp", "portState", "set", "0", "1", "forward");
	eval_silence("ssdk_sh", "stp", "portState", "set", "0", "2", "forward");
	eval_silence("ssdk_sh", "stp", "portState", "set", "0", "3", "forward");
	eval_silence("ssdk_sh", "stp", "portState", "set", "0", "4", "forward");
	eval_silence("ssdk_sh", "stp", "portState", "set", "0", "5", "forward");
	eval_silence("ssdk_sh", "fdb", "learnCtrl", "set", "disable");
	eval_silence("ssdk_sh", "fdb", "entry", "flush", "1");
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
	int profile = 512;
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
		maddr = get_deviceinfo_mr7350("hw_mac_addr");
		load_nss_ipq60xx(512);
		break;
	case ROUTER_FORTINET_FAP231F:
		maddr = get_deviceinfo_fap("ethaddr");
		load_nss_ipq60xx(1024);
		insmod("leds-gpio");
		break;
	case ROUTER_DYNALINK_DLWRX36:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		insmod("leds-gpio");
		break;
	case ROUTER_ASUS_AX89X:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		insmod("qca8k");
		insmod("leds-gpio");
		//		sysprintf("echo netdev > /sys/class/leds/white:wan/trigger");
		//		sysprintf("echo eth3 > /sys/class/leds/white:wan/device_name");
		//		sysprintf("echo link > /sys/class/leds/white:wan/link");
		sysprintf("echo netdev > /sys/class/leds/white:sfp/trigger");
		sysprintf("echo eth1 > /sys/class/leds/white:sfp/device_name");
		sysprintf("echo 1 > /sys/class/leds/white:sfp/link");
		sysprintf("echo netdev > /sys/class/leds/white:aqr10g/trigger");
		sysprintf("echo eth0 > /sys/class/leds/white:aqr10g/device_name");
		sysprintf("echo 1 > /sys/class/leds/white:aqr10g/link");
		nvram_default_get("eth0_label", "10gbit");
		nvram_default_get("eth1_label", "sfp");
		nvram_default_get("eth2_label", "wan");
		nvram_default_get("eth3_label", "lan1");
		nvram_default_get("eth4_label", "lan2");
		nvram_default_get("eth5_label", "lan3");
		nvram_default_get("eth6_label", "lan4");
		nvram_default_get("eth7_label", "lan5");
		nvram_default_get("eth8_label", "lan6");
		nvram_default_get("eth9_label", "lan7");
		nvram_default_get("eth10_label", "lan8");

		break;
	case ROUTER_LINKSYS_MX4200V2:
		profile = 1024;
		fwlen = 0x20000;
		maddr = get_deviceinfo_mx4200("hw_mac_addr");
		load_nss_ipq807x(1024);
		break;
	case ROUTER_LINKSYS_MX4300:
		profile = 1024;
		fwlen = 0x20000;
		maddr = get_deviceinfo_mx4200("hw_mac_addr");
		load_nss_ipq807x(1024);
		break;
	case ROUTER_LINKSYS_MX4200V1:
		fwlen = 0x20000;
		maddr = get_deviceinfo_mx4200("hw_mac_addr");
		load_nss_ipq807x(512);
		break;
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		fwlen = 0x20000;
		maddr = get_deviceinfo_mr5500("hw_mac_addr");
		load_nss_ipq50xx(512);
		break;
	default:
		fwlen = 0x20000;
		load_nss_ipq807x(512);
		break;
	}
	insmod("qca-ssdk");
	insmod("qca-nss-dp");
	int mtd = getMTD("art");
	if (mtd == -1)
		mtd = getMTD("ART");
	int uenv = getMTD("u_env");

	switch (brand) {
	case ROUTER_ASUS_AX89X:
		sprintf(mtdpath, "/dev/ubi0_1");
		break;
	default:
		sprintf(mtdpath, "/dev/mtd%d", mtd);
		break;
	}

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
		if (brand == ROUTER_LINKSYS_MR5500 || brand == ROUTER_LINKSYS_MX5500) {
			fseek(fp, 0x26800, SEEK_SET);
			out = fopen("/tmp/cal-pci-0001:01:00.0.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			out = fopen("/tmp/board2.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
		}
		if (brand == ROUTER_FORTINET_FAP231F) {
			fseek(fp, 0x33000, SEEK_SET);
			out = fopen("/tmp/ath10k_precal.bin", "wb");
			for (i = 0; i < 6; i++)
				putc(getc(fp), out);
			unsigned int newmac2[6];
			sscanf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac2[0], &newmac2[1], &newmac2[2], &newmac2[3],
			       &newmac2[4], &newmac2[5]);
			for (i = 0; i < 6; i++)
				putc(newmac2[i], out);
			for (i = 0; i < 2104; i++)
				putc(getc(fp), out);
			fclose(out);
			eval("cp", "-f", "/lib/firmware/ath10k/QCA9887/hw1.0/boarddata_0.bin", "/tmp/ath10k_board1.bin");
		}
		fclose(fp);
	} else {
		fprintf(stderr, "board data failed\n");
	}
	unsigned int newmac[6];
	char ethaddr[32];
	if (maddr) {
		fprintf(stderr, "sysinit using mac %s\n", maddr);
		sscanf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);

		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X", newmac[0] & 0xff, newmac[1] & 0xff, newmac[2] & 0xff,
			newmac[3] & 0xff, newmac[4] & 0xff, newmac[5] & 0xff);
		nvram_set("et0macaddr", ethaddr);
		nvram_set("et0macaddr_safe", ethaddr);
		set_hwaddr("eth0", ethaddr);
		set_hwaddr("eth1", ethaddr);
		set_hwaddr("eth2", ethaddr);
		set_hwaddr("eth3", ethaddr);
		set_hwaddr("eth4", ethaddr);
	}
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		removeregdomain("/tmp/caldata.bin", IPQ6018);
		removeregdomain("/tmp/board.bin", IPQ6018);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_FORTINET_FAP231F:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		removeregdomain("/tmp/caldata.bin", IPQ6018);
		removeregdomain("/tmp/board.bin", IPQ6018);
		set_envtools(getMTD("APPSBLENV"), "0x0", "0x10000", "0x10000", 1);
		break;
	case ROUTER_LINKSYS_MX5500:
	case ROUTER_LINKSYS_MR5500:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch2(ethaddr, 14);
		patch2(ethaddr, 20);
		patch2(ethaddr, 26);
		patch2(ethaddr, 32);
		patch2(ethaddr, 38);
		patch2(ethaddr, 44);
		removeregdomain("/tmp/caldata.bin", IPQ5018);
		removeregdomain("/tmp/board.bin", IPQ5018);
		removeregdomain("/tmp/caldata2.bin", QCN9000);
		removeregdomain("/tmp/board2.bin", QCN9000);
		removeregdomain("/tmp/cal-pci-0001:01:00.0.bin", QCN9000);

		/*		set6g("/tmp/caldata2.bin");
		set6g("/tmp/board2.bin");
		set6g("/tmp/cal-pci-0001:01:00.0.bin");*/
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_LINKSYS_MX4200V2:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan2_hwaddr", ethaddr);
		patch(ethaddr, 26);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		patchvht160("/tmp/caldata.bin", 0);
		patchvht160("/tmp/caldata.bin", 2);
		patchvht160("/tmp/board.bin", 0);
		patchvht160("/tmp/board.bin", 2);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_LINKSYS_MX4300:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan2_hwaddr", ethaddr);
		patch(ethaddr, 26);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		patchvht160("/tmp/caldata.bin", 0);
		patchvht160("/tmp/caldata.bin", 2);
		patchvht160("/tmp/board.bin", 0);
		patchvht160("/tmp/board.bin", 2);
		set_envtools(uenv, "0x0", "0x40000", "0x40000", 2);
		break;
	case ROUTER_LINKSYS_MX4200V1:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan2_hwaddr", ethaddr);
		patch(ethaddr, 26);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		patchvht160("/tmp/caldata.bin", 0);
		patchvht160("/tmp/caldata.bin", 2);
		patchvht160("/tmp/board.bin", 0);
		patchvht160("/tmp/board.bin", 2);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_DYNALINK_DLWRX36:
		set_envtools(getMTD("appsblenv"), "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		char *wlan0mac = get_deviceinfo_wxr("wlan0addr");
		nvram_set("wlan0_hwaddr", wlan0mac);
		patch(wlan0mac, 14);
		char *wlan1mac = get_deviceinfo_wxr("wlan1addr");
		nvram_set("wlan1_hwaddr", wlan1mac);
		patch(wlan1mac, 20);
		patch(wlan1mac, 26);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		set_envtools(getMTD("appsblenv"), "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_ASUS_AX89X:
		set_envtools(getMTD("appsblenv"), "0x0", "0x20000", "0x20000", 2);
		break;
	}

	int i;

	switch (brand) {
	case ROUTER_LINKSYS_MR5500:
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
		insmod("qca8k");
		start_initvlans();
		eval_silence("ifconfig", "eth0", "up");
		sysprintf("echo 0 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MX5500:
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
		insmod("qca8k");
		start_initvlans();
		eval_silence("ifconfig", "eth0", "up");
		sysprintf("echo 0 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MR7350:
		setscaling(1440000);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_FORTINET_FAP231F:
		sysprintf("echo netdev > /sys/class/leds/eth1G/trigger");
		sysprintf("echo eth0 > /sys/class/leds/eth1G/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth1G/link_1000");
		sysprintf("echo netdev > /sys/class/leds/eth100/trigger");
		sysprintf("echo eth0 > /sys/class/leds/eth100/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth100/link_100");
		sysprintf("echo netdev > /sys/class/leds/eth1G_lan2/trigger");
		sysprintf("echo eth1 > /sys/class/leds/eth1G_lan2/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth1G_lan2/link_1000");
		sysprintf("echo netdev > /sys/class/leds/eth100_lan2/trigger");
		sysprintf("echo eth1 > /sys/class/leds/eth100_lan2/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth100_lan2/link_100");
		setscaling(1800000);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
		setscaling(1382400);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_DYNALINK_DLWRX36:
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
		eval_silence("mount", "-t", "ubifs", "-o", "sync", "ubi0:rootfs_data", "/jffs");
		setscaling(0);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_ASUS_AX89X:
		insmod("qca83xx");
		setscaling(0);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");

		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		setscaling(0);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	}
	writestr("/sys/class/leds/wifi0/trigger", "phy0tpt");
	writestr("/sys/class/leds/wifi1/trigger", "phy1tpt");
	writestr("/sys/class/leds/wifi2/trigger", "phy2tpt");

	//	sysprintf("echo warm > /sys/kernel/reboot/mode");
	nvram_unset("sw_cpuport");
	nvram_unset("sw_wancpuport");

	detect_usbdrivers();

	return;
}
static void load_ath11k(int profile, int pci, int nss)
{
	char postfix[32] = { 0 };
	char driver_ath11k[32];
	char driver_ath11k_ahb[32];
	char driver_ath11k_pci[32];

	int od = nvram_default_geti("power_overdrive", 0);
	char overdrive[32];
	sprintf(overdrive, "poweroffset=%d", od);

	if (profile == 512)
		strcpy(postfix, "-512");
	sprintf(driver_ath11k, "ath11k%s", postfix);
	sprintf(driver_ath11k_ahb, "ath11k_ahb%s", postfix);
	sprintf(driver_ath11k_pci, "ath11k_pci%s", postfix);

	insmod("qmi_helpers");
	if (nss) {
		insmod("mac80211");
		if (nvram_match("ath11k_frame_mode", "1"))
			eval_silence("insmod", driver_ath11k, "frame_mode=1", overdrive);
		else
			eval_silence("insmod", driver_ath11k, overdrive);
	} else {
		eval_silence("insmod", "mac80211", "nss_redirect=0");
		eval_silence(
			"insmod", driver_ath11k, "nss_offload=0", "frame_mode=1",
			overdrive); // the only working nss firmware for qca5018 on mx5500/mr5500 does not work with nss offload for ath11k
		sysprintf("echo 0 > /proc/sys/dev/nss/general/redirect"); // required if nss_redirect is enabled
	}
	insmod(driver_ath11k_ahb);
	if (pci)
		insmod(driver_ath11k_pci);
}
void start_wifi_drivers(void)
{
	int notloaded = 0;
	if (use_mesh() && nvram_match("cur_nss", "12.5"))
		sys_reboot();
	if (!use_mesh() && nvram_match("cur_nss", "11.4"))
		sys_reboot();

	notloaded = insmod("compat");
	if (!notloaded) {
		dd_loginfo("sysinit", "load ATH/QCA 802.11ax Driver");
		int brand = getRouterBrand();
		int profile = 512;
		switch (brand) {
		case ROUTER_FORTINET_FAP231F:
		case ROUTER_DYNALINK_DLWRX36:
		case ROUTER_BUFFALO_WXR5950AX12:
		case ROUTER_ASUS_AX89X:
		case ROUTER_LINKSYS_MX4200V2:
		case ROUTER_LINKSYS_MX4300:
			profile = 1024;
			break;
		}
		insmod("compat_firmware_class");
		insmod("cfg80211");
		switch (brand) {
		case ROUTER_LINKSYS_MR5500:
		case ROUTER_LINKSYS_MX5500:
			load_ath11k(profile, 1, 0);
			break;
		case ROUTER_FORTINET_FAP231F:
			load_ath11k(profile, 0, !nvram_match("ath11k_nss", "0"));
			//			insmod("ath");
			//			insmod("ath10k_core");
			//			insmod("ath10k_pci");
			break;
		default:
			load_ath11k(profile, 0, !nvram_match("ath11k_nss", "0"));
			break;
		}
		wait_for_wifi();
		start_setup_affinity();
		start_initvlans();
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
	char *oclock = nvram_safe_get("overclocking");
	if (*oclock) {
		if (nvram_match("freq_fixed", "1")) {
			writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "userspace");
			writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", oclock);
		} else {
			writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "ondemand");
			writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", oclock);
		}
	}
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

void start_resetbc(void)
{
	int brand = getRouterBrand();
	if (brand == ROUTER_LINKSYS_MR7350 || brand == ROUTER_LINKSYS_MR5500 || brand == ROUTER_LINKSYS_MX5500 ||
	    brand == ROUTER_LINKSYS_MX4200V1 || brand == ROUTER_LINKSYS_MX4200V2 || brand == ROUTER_LINKSYS_MX4300) {
		if (!nvram_match("nobcreset", "1"))
			eval_silence("mtd", "resetbc", "s_env");
	}
}

void start_sysshutdown(void)
{
	start_deconfigurewifi();
	rmmod("ath11k_ahb");
}
