
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

#include <ddnvram.h>
#include <shutils.h>
#include <utils.h>
#include <ctype.h>
#include <services.h>
#include <wlutils.h>

#include "devices/ethernet.c"
#include "devices/wireless.c"

#define sys_reboot()                           \
	{                                      \
		eval("sync");                  \
		eval("event", "3", "1", "15"); \
	}

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

void *get_deviceinfo(char *partname, char *var)
{
	char mtd[64];
	static char res[256];
	bzero(res, sizeof(res));
	sprintf(mtd, "/dev/mtd%d", getMTD(partname));
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
			int a;
			for (a = 0; a < 17; a++)
				if (res[a] == 0xa || res[a] == 0xd)
					res[a] = 0;
			free(mem);
			return res;
		}
	}
	free(mem);
	return NULL;
}

void *get_deviceinfo_linksys(char *var)
{
	return get_deviceinfo("devinfo", var);
}

void *get_deviceinfo_fap(char *var)
{
	return get_deviceinfo("APPSBLENV", var);
}

void *get_deviceinfo_wxr(char *var)
{
	return get_deviceinfo("appsblenv", var);
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

		//              fprintf(stderr, "old boardflag = %X\n", s[68 / 4]);
		//              s[68 / 4] |= 0x800;
		//              s[68 / 4] |= 0x1000;
		//              fprintf(stderr, "new boardflag = %X\n", s[68 / 4]);

		//              fprintf(stderr, "old boardflag = %X\n", s[1040 / 4]);
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
		//              fprintf(stderr, "new boardflag = %X\n", s[1040 / 4]);
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

#define patch_qcn9000(ethaddr, offset)                                                                                       \
	{                                                                                                                    \
		unsigned char binmac[6];                                                                                     \
		int i;                                                                                                       \
		unsigned int newmac[6];                                                                                      \
		sscanf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4], \
		       &newmac[5]);                                                                                          \
		for (i = 0; i < 6; i++)                                                                                      \
			binmac[i] = newmac[i];                                                                               \
		patchmac("/tmp/cal-pci-0001:01:00.0.bin", offset, binmac);                                                   \
		patchmac("/tmp/cal-pci-0000:01:00.0.bin", offset, binmac);                                                   \
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
	const char *next;
	char var[80];
	char *vifs;
	for (count = 0; count < 3; count++) {
		char wifivifs[32];
		char base[32];
		sprintf(base, "wlan%d", count);
		sprintf(wifivifs, "wlan%d_vifs", count);
		if (nvram_nmatch("mesh", "wlan%d_mode", count) && !nvram_nmatch("disabled", "wlan%d_net_mode", count))
			return 1;
		vifs = nvram_safe_get(wifivifs);
		if (vifs != NULL && *vifs) {
			foreach(var, vifs, next) {
				if (nvram_nmatch("mesh", "%s_mode", var) && !nvram_nmatch("disabled", "%s_net_mode", var))
					return 1;
			}
		}
	}
}

static int use_nss_11_4(int setcur)
{
	int count;
	const char *next;
	char var[80];
	char *vifs;
	if (setcur)
		nvram_set("cur_nss", "11.4");
	if (nvram_match("force_old_nss", "1"))
		return 1;
	for (count = 0; count < 3; count++) {
		char wifivifs[32];
		char base[32];
		sprintf(base, "wlan%d", count);
		sprintf(wifivifs, "wlan%d_vifs", count);
		if (nvram_nmatch("mesh", "wlan%d_mode", count) && !nvram_nmatch("disabled", "wlan%d_net_mode", count))
			return 1;
		vifs = nvram_safe_get(wifivifs);
		if (vifs != NULL && *vifs) {
			foreach(var, vifs, next) {
				if (nvram_nmatch("mesh", "%s_mode", var) && !nvram_nmatch("disabled", "%s_net_mode", var))
					return 1;
			}
		}
	}
	if (setcur)
		nvram_set("cur_nss", "12.5");
	return 0;
}

int nss_disabled(int setcur)
{
	int count;
	const char *next;
	char var[80];
	char *vifs;
	if (setcur)
		nvram_set("nonss", "1");

	if (nvram_match("nss", "0")) {
		return 1;
	}
	for (count = 0; count < 3; count++) {
		char wifivifs[32];
		char base[32];
		sprintf(base, "wlan%d", count);
		sprintf(wifivifs, "wlan%d_vifs", count);
		if ((nvram_nmatch("wdssta", "wlan%d_mode", count) || nvram_nmatch("wdssta_mtik", "wlan%d_mode", count)) &&
		    !nvram_nmatch("disabled", "wlan%d_net_mode", count))
			return 1;
#if 0
		vifs = nvram_safe_get(wifivifs);
		if (vifs != NULL && *vifs) {
			foreach(var, vifs, next) {
				if ((nvram_nmatch("wdsap", "%s_mode", var) || nvram_nmatch("wdsap_mtik", "%s_mode", var) || nvram_nmatch("apup", "%s_mode", var)) &&
				    !nvram_nmatch("disabled", "%s_net_mode", var))
					return 1;
			}
		}
#endif
	}
	nvram_set("nonss", "0");
	return 0;
}

void loadnss(const char *module, const char *type)
{
	char driver[64];
	snprintf(driver, sizeof(driver), "%s-%s", module, type);
	insmod(driver);
}

static void load_nss(int profile, int maple, int cores, char *type)
{
	init_skb(profile, maple);
	loadnss("qca-ssdk", type);

	char driver[64];
	snprintf(driver, sizeof(driver), "qca-nss-dp-%s", type);

	if (profile == 256)
		eval("insmod", driver, "mem_profile=2");
	else if (profile == 512)
		eval("insmod", driver, "mem_profile=1");
	else
		eval("insmod", driver, "mem_profile=0");

	snprintf(driver, sizeof(driver), "qca-nss-drv-%s", type);

	eval("insmod", driver, use_nss_11_4(1) ? "mesh=1" : "mesh=0", nss_disabled(1) ? "disable_nss=1" : "disable_nss=0");
	if (!nss_disabled(0)) {
		loadnss("qca-nss-crypto", type);
		loadnss("qca-nss-cfi-cryptoapi", type);
		loadnss("qca-nss-netlink", type);
	}
	insmod("cryptodev");
	set_memprofile(cores, 1, profile);

	eval("insmod", "bonding", "miimon=1000", "downdelay=200", "updelay=200");
	if (!nss_disabled(0)) {
		loadnss("qca-nss-pppoe", type);
		loadnss("qca-nss-vlan", type);
		loadnss("qca-nss-qdisc", type);
	}
	insmod("pptp");
	if (!nss_disabled(0))
		loadnss("qca-nss-pptp", type);
	insmod("udp_tunnel");
	insmod("ip6_udp_tunnel");
	insmod("l2tp_core");
	if (!nss_disabled(0))
		loadnss("qca-nss-l2tpv2", type);
	insmod("vxlan");
	if (!nss_disabled(0))
		loadnss("qca-nss-vxlanmgr", type);
	insmod("tunnel6");
	insmod("ip6_tunnel");
	if (!nss_disabled(0)) {
		loadnss("qca-nss-tunipip6", type);
		loadnss("qca-nss-tlsmgr", type);
		insmod("qca-mcs");
		insmod("nss-ifb");
		loadnss("qca-nss-bridge-mgr", type);
	}
	insmod("qca-nss-wifi-meshmgr");
}

static void load_nss_ipq60xx(int profile)
{
	nvram_default_get("nss", "1");
	//	if (use_mesh())
	//		profile = 1024;
	load_nss(profile, 0, 4, "ipq60xx");
}

static void load_nss_ipq50xx(int profile)
{
	nvram_default_get("nss", "0");
	//	if (use_mesh())
	//		profile = 1024;
	load_nss(profile, 1, 2, "ipq50xx");
}

static void load_nss_ipq807x(int profile)
{
	nvram_default_get("nss", "1");
	//	if (use_mesh())
	//		profile = 1024;
	load_nss(profile, 0, 4, "ipq807x");
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
		case ROUTER_LINKSYS_MR7500:
			set_named_smp_affinity("DP_EXT_IRQ", 0, 1);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 2);
			set_named_smp_affinity("DP_EXT_IRQ", 2, 3);
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
		default:
			set_named_smp_affinity("DP_EXT_IRQ", 0, 1);
			set_named_smp_affinity("DP_EXT_IRQ", 1, 2);
			set_named_smp_affinity("DP_EXT_IRQ", 2, 3);
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
		eval("ssdk_sh", "port", "frameMaxSize", "set", "2", "0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval("ssdk_sh", "port", "flowctrlforcemode", "set", "2", "enable");
		eval("ssdk_sh", "port", "flowctrl", "set", "2", "enable");

		eval("ssdk_sh", "debug", "uniphy", "set", "0", "0x24", "0x54", "4");
		eval("ssdk_sh", "debug", "uniphy", "set", "0", "0x21c", "0x288a", "4");
		eval("ssdk_sh", "debug", "uniphy", "set", "0", "0x19c", "0xbea0", "4");
		sysprintf("echo 1 > /sys/ssdk/dev_id");

		break;
	case ROUTER_LINKSYS_MX5500:
		/* setup vlan config */

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval("ssdk_sh", "port", "frameMaxSize", "set", "2", "0x800");

		/* enable flowctrl to prevent low performance of PPTP connection with Cisco 7301. */
		eval("ssdk_sh", "port", "flowctrlforcemode", "set", "2", "enable");
		eval("ssdk_sh", "port", "flowctrl", "set", "2", "enable");

		sysprintf("echo 0 > /sys/ssdk/dev_id");
		eval("ssdk_sh", "debug", "uniphy", "set", "0", "0x24", "0x54", "4");
		eval("ssdk_sh", "debug", "uniphy", "set", "0", "0x21c", "0x288a", "4");
		eval("ssdk_sh", "debug", "uniphy", "set", "0", "0x19c", "0xbea0", "4");
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
	eval("ssdk_sh", "fdb", "portLearn", "set", "0", "disable");
	eval("ssdk_sh", "fdb", "portLearn", "set", "1", "disable");
	eval("ssdk_sh", "fdb", "portLearn", "set", "2", "disable");
	eval("ssdk_sh", "fdb", "portLearn", "set", "3", "disable");
	eval("ssdk_sh", "fdb", "portLearn", "set", "4", "disable");
	eval("ssdk_sh", "fdb", "portLearn", "set", "5", "disable");
	eval("ssdk_sh", "stp", "portState", "set", "0", "0", "forward");
	eval("ssdk_sh", "stp", "portState", "set", "0", "1", "forward");
	eval("ssdk_sh", "stp", "portState", "set", "0", "2", "forward");
	eval("ssdk_sh", "stp", "portState", "set", "0", "3", "forward");
	eval("ssdk_sh", "stp", "portState", "set", "0", "4", "forward");
	eval("ssdk_sh", "stp", "portState", "set", "0", "5", "forward");
	eval("ssdk_sh", "fdb", "learnCtrl", "set", "disable");
	eval("ssdk_sh", "fdb", "entry", "flush", "1");
}

static void wait_for_eth(const char *eth)
{
	int cnt = 0;
	while ((cnt++) < 100 && !ifexists(eth)) {
		usleep(100 * 1000);
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
	klogctl(8, NULL, nvram_geti("console_loglevel"));

	int brand = getRouterBrand();
	char *maddr = NULL;
	int fwlen = 0x10000;
	int profile = 512;
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
		fwlen = 0x20000;
		load_nss_ipq60xx(512);
		nvram_default_get("sfe", "3");
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_LINKSYS_MR7500:
		insmod("aquantia");
		fwlen = 0x20000;
		load_nss_ipq60xx(512);
		nvram_default_get("sfe", "3");
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_FORTINET_FAP231F:
		maddr = get_deviceinfo_fap("ethaddr");
		load_nss_ipq60xx(1024);
		insmod("leds-gpio");
		nvram_default_get("sfe", "3");
		break;
	case ROUTER_GLINET_AX1800:
		load_nss_ipq60xx(512);
		insmod("leds-gpio");
		nvram_default_get("sfe", "3");
		break;
	case ROUTER_DYNALINK_DLWRX36:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		nvram_default_get("sfe", "3");
		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		insmod("aquantia");
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		insmod("leds-gpio");
		nvram_default_get("sfe", "3");
		break;
	case ROUTER_ASUS_AX89X:
		insmod("aquantia");
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		insmod("qca8k");
		insmod("leds-gpio");
		//              sysprintf("echo netdev > /sys/class/leds/white:wan/trigger");
		//              sysprintf("echo eth3 > /sys/class/leds/white:wan/device_name");
		//              sysprintf("echo link > /sys/class/leds/white:wan/link");
		sysprintf("echo netdev > /sys/class/leds/white:sfp/trigger");
		sysprintf("echo 10gsfp > /sys/class/leds/white:sfp/device_name");
		sysprintf("echo 1 > /sys/class/leds/white:sfp/link");
		sysprintf("echo netdev > /sys/class/leds/white:aqr10g/trigger");
		sysprintf("echo 10gcopper > /sys/class/leds/white:aqr10g/device_name");
		sysprintf("echo 1 > /sys/class/leds/white:aqr10g/link");
		nvram_default_get("sfe", "3");

		break;
	case ROUTER_LINKSYS_MX4200V2:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		nvram_default_get("sfe", "3");
		break;
	case ROUTER_LINKSYS_MX8500:
		insmod("aquantia");
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		nvram_default_get("sfe", "3");
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_LINKSYS_MX5300:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		nvram_default_get("sfe", "3");
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_LINKSYS_MX4300:
		profile = 1024;
		fwlen = 0x20000;
		load_nss_ipq807x(1024);
		nvram_default_get("sfe", "3");
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_LINKSYS_MX4200V1:
		fwlen = 0x20000;
		load_nss_ipq807x(512);
		nvram_default_get("sfe", "3");
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_LINKSYS_MR5500:
		fwlen = 0x20000;
		load_nss_ipq50xx(512);
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	case ROUTER_LINKSYS_MX5500:
		fwlen = 0x20000;
		load_nss_ipq50xx(512);
		maddr = get_deviceinfo_linksys("hw_mac_addr");
		break;
	default:
		fwlen = 0x20000;
		nvram_default_get("sfe", "3");
		load_nss_ipq807x(512);
		break;
	}
	insmod("qca-ssdk");
	insmod("qca-nss-dp");

	switch (brand) {
	case ROUTER_LINKSYS_MR5500:
		insmod("qca8k");
		break;
	case ROUTER_LINKSYS_MX5500:
		insmod("qca8k");
		break;
	}
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
		switch (brand) {
		case ROUTER_LINKSYS_MR5500:
		case ROUTER_LINKSYS_MX5500: {
			fseek(fp, 0x26800, SEEK_SET);
			out = fopen("/tmp/cal-pci-0001:01:00.0.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			out = fopen("/tmp/board2.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			break;
		}
		case ROUTER_LINKSYS_MR7500: {
			fseek(fp, 0x26800, SEEK_SET);
			out = fopen("/tmp/cal-pci-0000:01:00.0.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			out = fopen("/tmp/board2.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			break;
		}
		case ROUTER_LINKSYS_MX8500: {
			fseek(fp, 0x26800, SEEK_SET);
			out = fopen("/tmp/cal-pci-0000:01:00.0.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			out = fopen("/tmp/board2.bin", "wb");
			for (i = 0; i < fwlen; i++)
				putc(getc(fp), out);
			fclose(out);
			break;
		}
		case ROUTER_LINKSYS_MX5300: {
			fseek(fp, 0x33000, SEEK_SET);
			out = fopen("/tmp/ath10k_board1.bin", "wb");
			for (i = 0; i < 6; i++)
				putc(getc(fp), out);
			unsigned int newmac2[6];
			sscanf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac2[0], &newmac2[1], &newmac2[2], &newmac2[3],
			       &newmac2[4], &newmac2[5]);
			for (i = 0; i < 6; i++) {
				putc(newmac2[i], out);
				getc(fp);
			}
			for (i = 0; i < 12052; i++)
				putc(getc(fp), out);
			fclose(out);
			break;
		}
		case ROUTER_GLINET_AX1800: {
			fseek(fp, 0, SEEK_SET);
			unsigned char newmac2[6];
			fread(newmac2, 6, 1, fp);
			static char tempaddr[32];
			maddr = tempaddr;
			sprintf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", newmac2[0] & 0xff, newmac2[1] & 0xff, newmac2[2] & 0xff,
				newmac2[3] & 0xff, newmac2[4] & 0xff, newmac2[5] & 0xff);
			break;
		}
		case ROUTER_FORTINET_FAP231F: {
			fseek(fp, 0x33000, SEEK_SET);
			out = fopen("/tmp/ath10k_board1.bin", "wb");
			for (i = 0; i < 6; i++)
				putc(getc(fp), out);
			unsigned int newmac2[6];
			sscanf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac2[0], &newmac2[1], &newmac2[2], &newmac2[3],
			       &newmac2[4], &newmac2[5]);
			for (i = 0; i < 6; i++) {
				putc(newmac2[i], out);
				getc(fp);
			}
			for (i = 0; i < 2104; i++)
				putc(getc(fp), out);
			fclose(out);
			eval("cp", "-f", "/lib/firmware/ath10k/QCA9887/hw1.0/boarddata_0.bin", "/tmp/ath10k_precal.bin");
			break;
		}
		}
		fclose(fp);
	} else {
		fprintf(stderr, "board data failed\n");
	}
	if (brand == ROUTER_ASUS_AX89X) {
		FILE *fp = fopen(mtdpath, "rb");
		if (fp) {
			fseek(fp, 0x1000 + 14, SEEK_SET);
			unsigned char newmac2[6];
			fread(newmac2, 6, 1, fp);
			fclose(fp);
			static char tempaddr[32];
			maddr = tempaddr;
			sprintf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", newmac2[0] & 0xff, newmac2[1] & 0xff, newmac2[2] & 0xff,
				newmac2[3] & 0xff, newmac2[4] & 0xff, newmac2[5] & 0xff);
		}
	}
	wait_for_eth("wan");
	wait_for_eth("lan1");
	unsigned int newmac[6];
	char ethaddr[32];
	if (maddr) {
		if (brand == ROUTER_ASUS_AX89X) {
			wait_for_eth("lan8");
			wait_for_eth("10gcopper");
			wait_for_eth("10gsfp");
		}
		fprintf(stderr, "sysinit using mac %s\n", maddr);
		sscanf(maddr, "%02X:%02X:%02X:%02X:%02X:%02X", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);

		sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X", newmac[0] & 0xff, newmac[1] & 0xff, newmac[2] & 0xff,
			newmac[3] & 0xff, newmac[4] & 0xff, newmac[5] & 0xff);
		if (brand == ROUTER_ASUS_AX89X) {
			nvram_set("wlan0_hwaddr", ethaddr);
			MAC_SUB(maddr);
			MAC_SUB(maddr);
			MAC_SUB(maddr);
			MAC_SUB(maddr);
			nvram_set("wlan1_hwaddr", ethaddr);
			MAC_ADD(maddr);
			MAC_ADD(maddr);
			MAC_ADD(maddr);
			MAC_ADD(maddr);
			MAC_ADD(maddr);
		}
		nvram_set("et0macaddr", ethaddr);
		nvram_set("et0macaddr_safe", ethaddr);
		set_hwaddr("eth0", ethaddr);
		set_hwaddr("wan", ethaddr);
		set_hwaddr("lan1", ethaddr);
		set_hwaddr("lan2", ethaddr);
		set_hwaddr("lan3", ethaddr);
		set_hwaddr("lan4", ethaddr);
		set_hwaddr("lan5", ethaddr);
		set_hwaddr("lan6", ethaddr);
		set_hwaddr("lan7", ethaddr);
		set_hwaddr("lan8", ethaddr);
		set_hwaddr("10gcopper", ethaddr);
		set_hwaddr("10gsfp", ethaddr);
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
	case ROUTER_GLINET_AX1800:
		MAC_ADD(ethaddr);
		MAC_ADD(ethaddr);
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_SUB(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		removeregdomain("/tmp/caldata.bin", IPQ6018);
		removeregdomain("/tmp/board.bin", IPQ6018);
		set_envtools(getMTD("appsblenv"), "0x0", "0x40000", "0x20000", 1);
		break;
	case ROUTER_LINKSYS_MR7500:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		nvram_set("wlan2_hwaddr", ethaddr);
		patch_qcn9000(ethaddr, 14);
		patch_qcn9000(ethaddr, 20);
		patch_qcn9000(ethaddr, 26);
		patch_qcn9000(ethaddr, 32);
		patch_qcn9000(ethaddr, 38);
		patch_qcn9000(ethaddr, 44);
		removeregdomain("/tmp/caldata.bin", IPQ6018);
		removeregdomain("/tmp/board.bin", IPQ6018);
		removeregdomain("/tmp/caldata2.bin", QCN9000);
		removeregdomain("/tmp/board2.bin", QCN9000);
		removeregdomain("/tmp/cal-pci-0000:01:00.0.bin", QCN9000);
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
		patch_qcn9000(ethaddr, 14);
		patch_qcn9000(ethaddr, 20);
		patch_qcn9000(ethaddr, 26);
		patch_qcn9000(ethaddr, 32);
		patch_qcn9000(ethaddr, 38);
		patch_qcn9000(ethaddr, 44);
		removeregdomain("/tmp/caldata.bin", IPQ5018);
		removeregdomain("/tmp/board.bin", IPQ5018);
		removeregdomain("/tmp/caldata2.bin", QCN9000);
		removeregdomain("/tmp/board2.bin", QCN9000);
		removeregdomain("/tmp/cal-pci-0001:01:00.0.bin", QCN9000);

		/*              set6g("/tmp/caldata2.bin");
		   set6g("/tmp/board2.bin");
		   set6g("/tmp/cal-pci-0001:01:00.0.bin"); */
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
	case ROUTER_LINKSYS_MX8500:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		nvram_set("wlan2_hwaddr", ethaddr);
		patch_qcn9000(ethaddr, 14);
		patch_qcn9000(ethaddr, 20);
		patch_qcn9000(ethaddr, 26);
		patch_qcn9000(ethaddr, 32);
		patch_qcn9000(ethaddr, 38);
		patch_qcn9000(ethaddr, 44);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		removeregdomain("/tmp/caldata2.bin", QCN9000);
		removeregdomain("/tmp/board2.bin", QCN9000);
		removeregdomain("/tmp/cal-pci-0000:01:00.0.bin", QCN9000);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
		break;
	case ROUTER_LINKSYS_MX5300:
		MAC_ADD(ethaddr);
		nvram_set("wlan0_hwaddr", ethaddr);
		patch(ethaddr, 14);
		MAC_ADD(ethaddr);
		nvram_set("wlan1_hwaddr", ethaddr);
		patch(ethaddr, 20);
		MAC_ADD(ethaddr);
		nvram_set("wlan2_hwaddr", ethaddr);
		removeregdomain("/tmp/caldata.bin", IPQ8074);
		removeregdomain("/tmp/board.bin", IPQ8074);
		set_envtools(uenv, "0x0", "0x40000", "0x20000", 2);
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
		start_initvlans();
		eval("ifconfig", "lan1", "up");
		sysprintf("echo 0 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MX5500:
		writeproc("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
		start_initvlans();
		eval("ifconfig", "wan", "up");
		sysprintf("echo 0 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MR7350:
		setscaling(1512000);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_GLINET_AX1800: // todo. check real cpu clock on device
		setscaling(1512000);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		break;
	case ROUTER_LINKSYS_MR7500:
		setscaling(1800000);
		//      disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		eval("fw_setenv", "bootcmd",
		     "aq_load_fw; if test $auto_recovery = no; then bootipq; elif test $boot_part = 1; then run bootpart1; else run bootpart2; fi");

		//reload firmware

		/*
		   // for reference only 
		   if [ "1" = $wan25G_enable ] ; then
		   echo "wan25g_enable!!!"
		   echo `ssdk_sh debug phy set 8  0x4007c400 0x9454`
		   echo `ssdk_sh debug phy set 8  0x40070020 0x0e1`
		   else
		   if [ "0" = $wan25G_enable ] ; then
		   echo "wan25G_disable!!!!"
		   echo `ssdk_sh debug phy set 8  0x4007c400 0x9c54`
		   echo `ssdk_sh debug phy set 8  0x40070020 0x01e1`
		   fi
		   fi */

		break;
	case ROUTER_FORTINET_FAP231F:
		sysprintf("echo netdev > /sys/class/leds/eth1G/trigger");
		sysprintf("echo lan1 > /sys/class/leds/eth1G/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth1G/link_1000");
		sysprintf("echo netdev > /sys/class/leds/eth100/trigger");
		sysprintf("echo lan1 > /sys/class/leds/eth100/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth100/link_100");
		sysprintf("echo netdev > /sys/class/leds/eth1G_lan2/trigger");
		sysprintf("echo wan > /sys/class/leds/eth1G_lan2/device_name");
		sysprintf("echo 1 > /sys/class/leds/eth1G_lan2/link_1000");
		sysprintf("echo netdev > /sys/class/leds/eth100_lan2/trigger");
		sysprintf("echo wan > /sys/class/leds/eth100_lan2/device_name");
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
	case ROUTER_LINKSYS_MX8500:
		setscaling(0);
		disableportlearn();
		sysprintf("echo 1 > /proc/sys/dev/nss/clock/auto_scale");
		eval("fw_setenv", "bootcmd",
		     "aq_load_fw; if test $auto_recovery = no; then bootipq; elif test $boot_part = 1; then run bootpart1; else run bootpart2; fi");

		break;
	case ROUTER_LINKSYS_MX5300:
		setscaling(0);
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
		eval("mount", "-t", "ubifs", "-o", "sync", "ubi0:rootfs_data", "/jffs");
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

	//      sysprintf("echo warm > /sys/kernel/reboot/mode");
	nvram_unset("sw_cpuport");
	nvram_unset("sw_wancpuport");
	nvram_set("old_nss", nvram_safe_get("nss"));

	if (nvram_geti("nvram_ver") < 11) {
		nvram_set("nvram_ver", "11");
		nvram_set("wan_ifname2", "wan");
		nvram_set("wan_ifname", "wan");
		nvram_set("wan_ifnames", "wan");
		nvram_set("wan_default", "wan");
		nvram_set("wan_iface", "wan");
	}

	detect_usbdrivers();

	if (nvram_match("testing", "1")) {
		char *part = getUEnv("boot_part");
		if (part) {
			if (!strcmp(part, "2")) {
				eval("fw_setenv", "boot_part", "1");
			} else {
				eval("fw_setenv", "boot_part", "2");
			}
		}
	}
	return;
}

static void load_ath11k_internal(int profile, int pci, int nss, int frame_mode, char *cert_region, int coldboot)
{
	char postfix[32] = { 0 };
	char driver_ath11k[32];
	char driver_ath11k_ahb[32];
	char driver_ath11k_pci[32];
	char driver_frame_mode[32];
	char driver_regionvariant[32];
	char driver_coldboot[32];

	if (!nvram_match("noath11k", "1")) {
		int od = nvram_default_geti("power_overdrive", 0);
		char overdrive[32];
		sprintf(overdrive, "poweroffset=%d", od);
		if (!nss) {
			profile = 1024;
			nvram_set("mem_profile", "1024");
		}
		//		if (profile == 512)
		//			strcpy(postfix, "-512");
		sprintf(driver_ath11k, "ath11k%s", postfix);
		sprintf(driver_ath11k_ahb, "ath11k_ahb%s", postfix);
		sprintf(driver_ath11k_pci, "ath11k_pci%s", postfix);
		sprintf(driver_frame_mode, "frame_mode=%d", frame_mode);
		sprintf(driver_regionvariant, "regionvariant=%s", cert_region);
		sprintf(driver_coldboot, "cold_boot_cal=%d", coldboot);
		insmod("qmi_helpers");
		if (nss) {
			insmod("mac80211");
			eval("insmod", driver_ath11k, driver_frame_mode, overdrive, driver_regionvariant, driver_coldboot);
		} else {
			eval("insmod", "mac80211", "nss_redirect=0");
			eval("insmod", driver_ath11k, "nss_offload=0", driver_frame_mode, overdrive, driver_regionvariant,
			     driver_coldboot);
			sysprintf("echo 0 > /proc/sys/dev/nss/general/redirect"); // required if nss_redirect is enabled
		}
		insmod(driver_ath11k_ahb);
		if (pci)
			insmod(driver_ath11k_pci);
	}
}

void start_wifi_drivers(void)
{
	int notloaded = 0;
	if (!nvram_match("force_old_nss", "1")) {
		if (use_nss_11_4(0) && nvram_match("cur_nss", "12.5"))
			sys_reboot();
		if (!use_nss_11_4(0) && nvram_match("cur_nss", "11.4"))
			sys_reboot();
	}
	if (nss_disabled(0) && nvram_match("nonss", "0"))
		sys_reboot();
	if (!nss_disabled(0) && nvram_match("nonss", "1"))
		sys_reboot();

	if (!nvram_match("nss", nvram_safe_get("old_nss")))
		sys_reboot();
	notloaded = insmod("compat");
	char *fm = nvram_safe_get("ath11k_frame_mode");
	int frame_mode = 2;
	int minif = 2;
	char *cert_region = "";
	if (*fm)
		frame_mode = atoi(fm);
	if (frame_mode > 2 || frame_mode < 0)
		frame_mode = 2;
	if (nss_disabled(0) && frame_mode == 2)
		frame_mode = 1;

	if (!notloaded) {
		dd_loginfo("sysinit", "load ATH/QCA 802.11ax Driver");
		int brand = getRouterBrand();
		int profile = 512;
		nvram_set("mem_profile", "512");
		switch (brand) {
		case ROUTER_FORTINET_FAP231F:
		case ROUTER_LINKSYS_MX4200V2:
		case ROUTER_LINKSYS_MX4300:
		case ROUTER_LINKSYS_MX8500:
		case ROUTER_LINKSYS_MX5300:
			minif = 3;
		case ROUTER_DYNALINK_DLWRX36:
		case ROUTER_BUFFALO_WXR5950AX12:
		case ROUTER_ASUS_AX89X:
			//		case ROUTER_GLINET_AX1800:
			profile = 1024;
			nvram_set("mem_profile", "1024");
			break;
		}
		insmod("compat_firmware_class");
		insmod("cfg80211");
		switch (brand) {
		case ROUTER_LINKSYS_MR5500:
		case ROUTER_LINKSYS_MX5500:
			if (frame_mode == 2)
				frame_mode = 1;
			load_ath11k_internal(profile, 1, 0, frame_mode, "", 1);
			break;
		case ROUTER_FORTINET_FAP231F:
			load_ath11k_internal(profile, 0, !nvram_match("ath11k_nss", "0") && !nss_disabled(0), frame_mode, "", 1);
			wait_for_wifi(2);
			load_ath10k();
			minif = 3;
			break;
		case ROUTER_LINKSYS_MR7500:
			/*                      eval("ssdk_sh", "debug", "phy", "set", "0x8", "0x401e2680", "0x1");
			   usleep(100 * 1000);
			   eval("aq-fw-download", "/lib/firmware/marvell/AQR114C.cld", "eth0", "8");
			   sleep(1);
			   eval("ssdk_sh", "port", "autoneg", "restart", "5");
			   sleep(2);
			   eval("ssdk_sh", "debug", "phy", "set", "8", "0x401ec430", "0xc0ef");
			   eval("ssdk_sh", "debug", "phy", "set", "8", "0x401ec431", "0xc0e0");
			   eval("ssdk_sh", "debug", "phy", "set", "8", "0x40070010", "0x9de1");
			   sleep(1);
			   eval("ssdk_sh", "debug", "phy", "set", "8", "0x40070000", "0x3200"); */
			//                      char *cert_region = get_deviceinfo_linksys("cert_region");
			//                      if (!cert_region)
			load_ath11k_internal(profile, 1, !nvram_match("ath11k_nss", "0") && !nss_disabled(0), frame_mode,
					     cert_region, 0);
			minif = 3;
			break;
		case ROUTER_LINKSYS_MX8500:
			//                      char *cert_region = get_deviceinfo_linksys("cert_region");
			//                      if (!cert_region)
			load_ath11k_internal(profile, 1, !nvram_match("ath11k_nss", "0") && !nss_disabled(0), frame_mode,
					     cert_region, 1);
			minif = 3;
			break;

		case ROUTER_LINKSYS_MX5300:
			//                      char *cert_region = get_deviceinfo_linksys("cert_region");
			//                      if (!cert_region)
			load_ath11k_internal(profile, 0, !nvram_match("ath11k_nss", "0") && !nss_disabled(0), frame_mode,
					     cert_region, 1);
			wait_for_wifi(2);
			load_ath10k();
			minif = 3;
			break;

		default:
			load_ath11k_internal(profile, 0, !nvram_match("ath11k_nss", "0") && !nss_disabled(0), frame_mode, "", 1);
			break;
		}
		wait_for_wifi(minif);
		start_setup_affinity();
		start_initvlans();
		writestr("/sys/class/leds/wifi0/trigger", "phy0tpt");
		writestr("/sys/class/leds/wifi1/trigger", "phy1tpt");
		writestr("/sys/class/leds/wifi2/trigger", "phy2tpt");
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

void sys_overclocking(void)
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
	return "wan";
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
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MR7500:
	case ROUTER_LINKSYS_MX8500:
	case ROUTER_LINKSYS_MX5300:
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
		if (!nvram_match("nobcreset", "1"))
			eval("mtd", "resetbc", "s_env");
		break;
	}
}

void start_sysshutdown(void)
{
	start_deconfigurewifi();
	rmmod("ath11k_ahb");
}

static void set_linksys_defaults(int triband)
{
	nvram_set("wlan0_ssid", get_deviceinfo_linksys("default_ssid"));
	nvram_set("wlan0_akm", "psk3");
	nvram_set("wlan0_psk3", "1");
	nvram_set("wlan0_ccmp", "1");
	nvram_set("wlan0_security_mode", "wpa");
	nvram_set("wlan0_sae_key", get_deviceinfo_linksys("default_passphrase"));
	nvram_set("wlan1_ssid", get_deviceinfo_linksys("default_ssid"));
	nvram_set("wlan1_akm", "psk3");
	nvram_set("wlan1_psk3", "1");
	nvram_set("wlan1_ccmp", "1");
	nvram_set("wlan1_security_mode", "wpa");
	nvram_set("wlan1_sae_key", get_deviceinfo_linksys("default_passphrase"));
	if (triband) {
		nvram_set("wlan2_ssid", get_deviceinfo_linksys("default_ssid"));
		nvram_set("wlan2_akm", "psk3");
		nvram_set("wlan2_psk3", "1");
		nvram_set("wlan2_ccmp", "1");
		nvram_set("wlan2_security_mode", "wpa");
		nvram_set("wlan2_sae_key", get_deviceinfo_linksys("default_passphrase"));
	}
}

void start_arch_defaults(void)
{
	int brand = getRouterBrand();

	switch (brand) {
	case ROUTER_LINKSYS_MR7500:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX8500:
	case ROUTER_LINKSYS_MX5300:
	case ROUTER_LINKSYS_MX4300:
	case ROUTER_LINKSYS_MX4200V1:
		set_linksys_defaults(1);
		break;
	case ROUTER_LINKSYS_MR7350:
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		set_linksys_defaults(0);
		break;
	default:
		break;
	}
}
