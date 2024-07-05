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
void *get_deviceinfo_mr5500(char *var)
{
	return get_deviceinfo("/dev/mtd11", var);
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

static void set_memprofile(int cores, int profile)
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
	sysprintf("echo 15 > /proc/sys/dev/nss/rps/hash_bitmap");
}

static void load_nss_ipq60xx(int profile)
{
	insmod("qca-ssdk-ipq60xx");
	insmod("qca-nss-dp-ipq60xx");

	nvram_default_get("nss", "1");
	if (nvram_match("nss", "1")) {
		insmod("qca-nss-drv-ipq60xx");
		insmod("qca-nss-crypto-ipq60xx");
		insmod("qca-nss-cfi-cryptoapi-ipq60xx");
		insmod("qca-nss-netlink-ipq60xx");

		set_memprofile(1, profile);

		eval("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
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
	}
}

static void load_nss_ipq50xx(int profile)
{
	insmod("qca-ssdk-ipq50xx");
	insmod("qca-nss-dp-ipq50xx");

	nvram_default_get("nss", "1");
	if (nvram_match("nss", "1")) {
		insmod("qca-nss-drv-ipq50xx");
		insmod("qca-nss-crypto-ipq50xx");
		insmod("qca-nss-cfi-cryptoapi-ipq50xx");
		insmod("qca-nss-netlink-ipq50xx");

		set_memprofile(1, profile);

		eval("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
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
	}
}
static void load_nss_ipq807x(int profile)
{
	insmod("qca-ssdk-ipq807x");
	insmod("qca-nss-dp-ipq807x");
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

		set_memprofile(2, profile);

		eval("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
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
	}
}

void start_setup_affinity(void)
{
	int brand = getRouterBrand();
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

		set_named_smp_affinity("nss_queue0", 1, 1);
		set_named_smp_affinity("nss_queue1", 2, 1);
		set_named_smp_affinity("nss_queue2", 3, 1);
		set_named_smp_affinity("nss_queue3", 0, 1);

		set_named_smp_affinity("nss_queue0", 2, 2);
		set_named_smp_affinity("nss_empty_buf_sos", 3, 1);
		set_named_smp_affinity("nss_empty_buf_queue", 3, 1);
		set_named_smp_affinity("nss_empty_buf_sos", 2, 2);

		set_named_smp_affinity("ppdu-end-interrupts-mac1", 1, 1);
		set_named_smp_affinity("ppdu-end-interrupts-mac3", 2, 1);

		sysprintf("echo 1 > /proc/sys/dev/nss/rps/enable");
		break;
	}
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
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
		maddr = get_deviceinfo_mr7350("hw_mac_addr");
		load_nss_ipq60xx(512);
		break;
	case ROUTER_DYNALINK_DLWRX36:
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		break;
	case ROUTER_LINKSYS_MX4200V2:
		fwlen = 0x20000;
		maddr = get_deviceinfo_mx4200("hw_mac_addr");
		load_nss_ipq807x(1024);
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
		if (brand == ROUTER_LINKSYS_MR5500 || brand == ROUTER_LINKSYS_MX5500) {
			fseek(fp, 0x26800, SEEK_SET);
			out = fopen("/tmp/cal-pci-0001:01:00.0.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
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
	case ROUTER_LINKSYS_MX5500:
	case ROUTER_LINKSYS_MR5500:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch2(ethaddr, 14);
		removeregdomain("/tmp/caldata.bin", IPQ5018);
		removeregdomain("/tmp/board.bin", IPQ5018);
		removeregdomain("/tmp/caldata2.bin", QCN9000);
		removeregdomain("/tmp/board2.bin", QCN9000);
		removeregdomain("/tmp/cal-pci-0001:01:00.0.bin", QCN9000);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_LINKSYS_MX4200V2:
		MAC_ADD(ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		patch(ethaddr, 26);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		patchvht160("/tmp/caldata.bin", 0);
		patchvht160("/tmp/caldata.bin", 2);
		patchvht160("/tmp/board.bin", 0);
		patchvht160("/tmp/board.bin", 2);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_LINKSYS_MX4200V1:
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
	}

	insmod("compat");
	insmod("compat_firmware_class");
	insmod("cfg80211");
	insmod("mac80211");
	insmod("qmi_helpers");
	switch (brand) {
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		eval("insmod", "ath11k", "nss_offload=0");
		insmod("ath11k_ahb");
		insmod("ath11k_pci");
		break;
	default:
		insmod("ath11k");
		insmod("ath11k_ahb");
		break;
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

	start_setup_affinity();
	switch (brand) {
	case ROUTER_LINKSYS_MR5500:
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		/* setup vlan config */

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		sysprintf("ssdk_sh port frameMaxSize set 2 0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		sysprintf("ssdk_sh port flowctrlforcemode set 2 enable");
		sysprintf("ssdk_sh port flowctrl set 2 enable");

		/*config port.5 to VLAN(1) and port.1/2/3/4 to VLAN(2) */
		sysprintf("echo 1 > /sys/ssdk/dev_id");
		sysprintf("ssdk_sh vlan entry flush");

		sysprintf("ssdk_sh vlan entry append 1 1 6,5 6 5 default default default");
		sysprintf("ssdk_sh vlan entry append 2 2 6,1,2,3,4 6 1,2,3,4 default default default");

		sysprintf("ssdk_sh portVlan ingress set 1 fallback");
		sysprintf("ssdk_sh portVlan ingress set 2 fallback");
		sysprintf("ssdk_sh portVlan ingress set 3 fallback");
		sysprintf("ssdk_sh portVlan ingress set 4 fallback");
		sysprintf("ssdk_sh portVlan ingress set 5 fallback");
		sysprintf("ssdk_sh portVlan ingress set 6 fallback");

		sysprintf("ssdk_sh portVlan defaultSVid set 1 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 2 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 3 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 4 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 5 1");
		sysprintf("ssdk_sh portVlan defaultSVid set 6 1");
		sysprintf("ssdk_sh portVlan egress set 1 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 1 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 1 enable");

		sysprintf("ssdk_sh portVlan egress set 2 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 2 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 2 enable");

		sysprintf("ssdk_sh portVlan egress set 3 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 3 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 3 enable");

		sysprintf("ssdk_sh portVlan egress set 4 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 4 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 4 enable");

		sysprintf("ssdk_sh portVlan egress set 5 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 5 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 5 enable");

		sysprintf("ssdk_sh portVlan egress set 6 tagged");
		sysprintf("ssdk_sh portVlan qinqrole set 6 core");
		sysprintf("ssdk_sh portVlan vlanPropagation set 6 replace");
		sysprintf("ssdk_sh portVlan tlsMode set 6 disable");

		sysprintf("ssdk_sh portVlan qinqmode set stag");
		sysprintf("ssdk_sh portVlan svlanTPID set 0x8100");

		sysprintf("ssdk_sh port poweron set 5");
		sysprintf("ssdk_sh fdb entry flush 0 > /dev/null 2>&1");

		/*drop invalid tcp*/
		sysprintf("ssdk_sh debug reg set 0x200 0x2000 4");
		/* drop tcp/udp checksum errors */
		sysprintf("ssdk_sh debug reg set 0x204 0x0842 4");
		/* enable pppoe */
		sysprintf("ssdk_sh debug reg set 0x214 0x2000000 4");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		sysprintf("ssdk_sh port flowctrlforcemode set 6 enable");
		sysprintf("ssdk_sh port flowctrl set 6 enable");

		eval("ifconfig", "eth0", "up");
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
		sysprintf("echo 0 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MX5500:
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		/* setup vlan config */
		sysprintf("echo 0 > /sys/ssdk/dev_id");
		sysprintf("ssdk_sh port frameMaxSize set 2 0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		sysprintf("ssdk_sh port flowctrlforcemode set 2 enable");
		sysprintf("ssdk_sh port flowctrl set 2 enable");

		/* config port.2 to VLAN(1) and port.3/4/5 to VLAN(2) */
		sysprintf("echo 1 > /sys/ssdk/dev_id");

		sysprintf("ssdk_sh vlan entry flush");

		sysprintf("ssdk_sh vlan entry append 1 1 6,2 6 2 default default default");
		sysprintf("ssdk_sh vlan entry append 2 2 6,3,4,5 6 3,4,5 default default default");

		sysprintf("ssdk_sh portVlan ingress set 2 fallback");
		sysprintf("ssdk_sh portVlan ingress set 3 fallback");
		sysprintf("ssdk_sh portVlan ingress set 4 fallback");
		sysprintf("ssdk_sh portVlan ingress set 5 fallback");
		sysprintf("ssdk_sh portVlan ingress set 6 fallback");

		sysprintf("ssdk_sh portVlan defaultSVid set 2 1");
		sysprintf("ssdk_sh portVlan defaultSVid set 3 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 4 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 5 2");
		sysprintf("ssdk_sh portVlan defaultSVid set 6 1");

		sysprintf("ssdk_sh portVlan egress set 2 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 2 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 2 enable");

		sysprintf("ssdk_sh portVlan egress set 3 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 3 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 3 enable");

		sysprintf("ssdk_sh portVlan egress set 4 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 4 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 4 enable");

		sysprintf("ssdk_sh portVlan egress set 5 unmodified");
		sysprintf("ssdk_sh portVlan vlanPropagation set 5 disable");
		sysprintf("ssdk_sh portVlan tlsMode set 5 enable");

		sysprintf("ssdk_sh portVlan egress set 6 tagged");
		sysprintf("ssdk_sh portVlan qinqrole set 6 core");
		sysprintf("ssdk_sh portVlan vlanPropagation set 6 replace");
		sysprintf("ssdk_sh portVlan tlsMode set 6 disable");

		sysprintf("ssdk_sh portVlan qinqmode set stag");
		sysprintf("ssdk_sh portVlan svlanTPID set 0x8100");
		sysprintf("ssdk_sh port poweron set 2");
		sysprintf("ssdk_sh fdb entry flush 0 > /dev/null 2>&1");
		/*drop invalid tcp*/
		sysprintf("ssdk_sh debug reg set 0x200 0x2000 4");
		/* drop tcp/udp checksum errors */
		sysprintf("ssdk_sh debug reg set 0x204 0x0842 4");
		/* enable pppoe */
		sysprintf("ssdk_sh debug reg set 0x214 0x2000000 4");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		sysprintf("ssdk_sh port flowctrlforcemode set 6 enable");
		sysprintf("ssdk_sh port flowctrl set 6 enable");

		eval("ifconfig", "eth0", "up");
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
		sysprintf("echo 0 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_DYNALINK_DLWRX36:
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "ondemand");
		writeproc("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate", "1000000");
		writeproc("/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor", "10");
		writeproc("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "50");
		sysprintf("ssdk_sh debug module_func set servcode 0xf 0x0 0x0");
		sysprintf("ssdk_sh servcode config set 1 n 0 0xfffefc7f 0xffbdff 0 0 0 0 0 0");
		sysprintf("ssdk_sh debug module_func set servcode 0x0 0x0 0x0");
		sysprintf("ssdk_sh acl list create 56 48");
		sysprintf(
			"ssdk_sh acl rule add 56 0 1 n 0 0 mac n n n n n y 01-80-c2-00-00-00 ff-ff-ff-ff-ff-ff n n n n n n n n n n n n n n n n n n n n n n n y n n n n n n n n n n 0 0 n n n n n n n n n n n n n y n n n n n n n n n n n n y n n n n n n n n n n n n 0");
		sysprintf(
			"ssdk_sh acl rule add 56 1 1 n 0 0 mac n n n n n n n yes 0x8809 0xffff n n n n n n n n n n n n n n n n n n n n n y n n n n n n n n n n 0 0 n n n n n n n n n n n n n y n n n n n n n n n n n n y n n n n n n n n n n n n 0");
		sysprintf(
			"ssdk_sh acl rule add 56 2 1 n 0 0 mac n n n n n n n yes 0x888e 0xffff n n n n n n n n n n n n n n n n n n n n n y n n n n n n n n n n 0 0 n n n n n n n n n n n n n y n n n n n n n n n n n n y n n n n n n n n n n n n 0");
		sysprintf("ssdk_sh acl list bind 56 0 2 1");
		sysprintf("ssdk_sh fdb portLearn set 0 disable");
		sysprintf("ssdk_sh fdb portLearn set 1 disable");
		sysprintf("ssdk_sh fdb portLearn set 2 disable");
		sysprintf("ssdk_sh fdb portLearn set 3 disable");
		sysprintf("ssdk_sh fdb portLearn set 4 disable");
		sysprintf("ssdk_sh fdb portLearn set 5 disable");
		sysprintf("ssdk_sh stp portState set 0 0 forward");
		sysprintf("ssdk_sh stp portState set 0 1 forward");
		sysprintf("ssdk_sh stp portState set 0 2 forward");
		sysprintf("ssdk_sh stp portState set 0 3 forward");
		sysprintf("ssdk_sh stp portState set 0 4 forward");
		sysprintf("ssdk_sh stp portState set 0 5 forward");
		sysprintf("ssdk_sh fdb learnCtrl set disable");
		sysprintf("ssdk_sh fdb entry flush 1");
		break;
	}
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

void start_resetbc(void)
{
	int brand = getRouterBrand();
	if (brand == ROUTER_LINKSYS_MR7350 || brand == ROUTER_LINKSYS_MR5500 || brand == ROUTER_LINKSYS_MX5500 ||
	    brand == ROUTER_LINKSYS_MX4200V1 || brand == ROUTER_LINKSYS_MX4200V2) {
		if (!nvram_match("nobcreset", "1"))
			eval("mtd", "resetbc", "s_env");
	}
}