/*
 * backup.c
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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <syslog.h>

#include <typedefs.h>
//#include <netconf.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#else
#define NVRAMSPACE NVRAM_SPACE
#endif


static char *filter[] = { "lan_ifnames",
	"lan_ifname",
	"wan_ifnames",
	"wan_ifname",
	"et0macaddr",
	"et1macaddr",
	"il0macaddr",
	"boardnum",
	"boardtype",
	"boardrev",
	"melco_id",
	"product_name",
	"phyid_num",
	"cardbus",
	"CFEver",
	"clkfreq",
	"xtalfreq",
	"bootflags",
	"et0mdcport",
	"devpath0",
	"devpath1",
	"gpio2",
	"gpio4",
	"gpio5",
	"gpio7",
	"gpio8",
	"gpio9",
	"gpio10",
	"gpio12",
	"gpio13",
	"gpio15",
	"gpio22",
	"landevs",
	"wandevs",
	"wl0_wandevs",
	"boardpwrctl",
	"os_ram_addr",
	"tftpd_ipaddr",
	"tftp_rrq_timeout",
	"tftp_max_retries",
	"tftp_recv_timeout",
	"blink_diag_led",
	"cfe_ping_timeout",
	"et_pwrsave",
	"clkdivsf",
	"antswctl2g",
	"devid",
	"macaddr",
	"ccode",
	"regrev",
	"ledbh0",
	"ledbh11",
	"ledbh10",
	"ledbh2",
	"ledbh3",
	"ledbh5",
	"leddc",
	"aa2g",
	"ag0",
	"ag1",
	"bxa2g",
	"rssisav2g",
	"rssismc2g",
	"rssismf2g",
	"tri2g",
	"rxpo2g",
	"txchain",
	"rxchain",
	"antswitch",
	"tssipos2g",
	"extpagain2g",
	"pdetrange2g",
	"triso2g",
	"antswctl2g",
	"cck2gpo",
	"ofdm2gpo",
	"mcs2gpo0",
	"mcs2gpo1",
	"mcs2gpo2",
	"mcs2gpo3",
	"mcs2gpo4",
	"mcs2gpo5",
	"mcs2gpo6",
	"mcs2gpo7",
	"cddpo",
	"stbcpo",
	"bw40po",
	"bwduppo",
	"maxp2ga0",
	"itt2ga0"
	"pa2gw0a0",
	"pa2gw1a0",
	"pa2gw2a0",
	"maxp2ga1",
	"itt2ga1"
	"pa2gw0a1",
	"pa2gw1a1",
	"pa2gw2a1",
	"wl_msglevel",
	"wl_pcie_mrrs",
	"wl_dmatxctl",
	"wl_dmarxctl",
	"nvram_version",
	"product",
	"custom_id",
	"hw_rev",
	"boardflags",
	"boardflags2",
	"boardflags3",
	"sromrev",
	"sdram_config",
	"sdram_init",
	"sdram_refresh",
	"sdram_ncdl",
	"boot_wait",
	"wait_time",
	"et0phyaddr",
	"hardware_version",
	"test_led_gpio",
	"GemtekPmonVer",
	"et0mdcport",
	"vlan0ports",
	"vlan1ports",
	"vlan2ports",
	"vlan0hwname",
	"vlan1hwname",
	"vlan2hwname",
	"wl_use_coregpio",
	"wl0gpio0",
	"wl0gpio1",
	"wl0gpio2",
	"wl0gpio3",
	"wl0gpio4",
	"reset_gpio",
	"af_hash",
	"DEF-p_wireless_eth1_11a-authmode_ex",
	"DEF-p_wireless_eth1_11a-authmode",
	"DEF-p_wireless_eth1_11a-crypto",
	"DEF-p_wireless_eth1_11a-wpapsk",
	"DEF-p_wireless_eth2_11bg-authmode_ex",
	"DEF-p_wireless_eth2_11bg-authmode",
	"DEF-p_wireless_eth2_11bg-crypto",
	"DEF-p_wireless_eth2_11bg-wpapsk",
	"region",
	"pincode",
	"nvram_version",
	"watchdog",
	"serial_no",
	"secret_code",
	"boot_ver",
	"mfg_wait",
	"bootnv_ver",
	"ntype",
	"cthwver",
	"parefldovoltage",
	"boot_hw_model",
	"hw_model",
	"model",
	"odmpid",
	"wl_country",
	"boot_hw_ver",
	"hw_version",
	"regulation_domain",
	"regulation_domain_5G",
	"Ate_power_on_off_ret",
	"pmon_ver",
	"sw_version",
	"hw_version",
	"hw_revision",
	"hw_mac_addr",
	"tc_passphrase",
	"tc_ssid",
	"image_second_offset",
	"image_first_offset",
	"serial_number",
	"tftpd_ipaddr",
	"modelNumber",
	"manufacturer_date",
	"wps_lock_forever_cnt",
	"mfg_data_version",
	"maxpartialboots",
	"uuid_key",
	"wl0_country_rev",
	"wl1_country_rev",
	"wl0_country_code",
	"wl1_country_code",
	"firmwarename",
	"wltest",
	"bootpartition",
	"bypasscrccheck",
	"tftpserverip",
	"prodid",
	"cfe_disable_2G",
	"cfe_disable_5G",
	"usb_gpio_reverse",
	"nvram_space",
	"pmon_date",
	"bl_version",
	NULL
};

int nvram_critical(char *name)
{
	int a = 0;
	if (name[0] == '@') {
		fprintf(stderr, "ommit %s\n", name);
		return 1;
	}
	while (filter[a] != NULL) {
		if (!strcmp(name, filter[a++])) {
			return 1;
		}
	}
	if (strncmp(name, "sb/", 3) && strncmp(name, "pci/", 4) && strncmp(name, "0:", 2) && strncmp(name, "1:", 2))
		return 1;
	return 0;
}

void nvram_clear(void)
{
	char *buf = (char *)safe_malloc(NVRAMSPACE);

	nvram_getall(buf, NVRAMSPACE);
	nvram_open();
	char *p = buf;
	int i;

	while (strlen(p) != 0) {
		int len = strlen(p);

		for (i = 0; i < len; i++)
			if (p[i] == '=')
				p[i] = 0;
		if (!nvram_critical(p))
			nvram_immed_set(p, NULL);
		p += len + 1;
	}
	nvram_close();
}

static void save(FILE * fp, char *p, int not)
{
	int i;
	while (strlen(p) != 0) {
		int len = strlen(p);
		if (len > 2 && (!!strncmp(p, "wl_", 3)) == not) {
			p += len + 1;
			continue;
		}
		for (i = 0; i < len; i++)
			if (p[i] == '=')
				p[i] = 0;
		char *name = p;
		fputc(strlen(name), fp);
		fwrite(name, 1, strlen(name), fp);
		char *val = nvram_safe_get(name);
		fputc(strlen(val) & 255, fp);
		fputc(strlen(val) >> 8, fp);
		fwrite(val, 1, strlen(val), fp);
		p += len + 1;
	}

}

int nvram_restore(char *filename)
{
	char sign[7];
	char *nvram_ver = NULL;
#ifdef HAVE_REGISTER
	if (!isregistered_real()) {
		return -1;
	}
#endif
	sign[6] = 0;
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return -1;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	int count = 0;

	fread(sign, 6, 1, fp);
	len -= 6;
	if (!strcmp(sign, "DD-WRT")) {
		nvram_clear();
		nvram_open();
		unsigned char b;

		fread((char *)&b, 1, 1, fp);
		count = b;
		fread((char *)&b, 1, 1, fp);
		count += ((unsigned int)b << 8);
		len -= 2;
		int i;

		for (i = 0; i < count && len > 0; i++) {
			unsigned short l = 0;
			unsigned char c = 0;

			fread((char *)&c, 1, 1, fp);
			char *name = (char *)safe_malloc(c + 1);

			fread(name, c, 1, fp);
			name[c] = 0;
			len -= (c + 1);

			fread((char *)&b, 1, 1, fp);
			l = b;
			fread((char *)&b, 1, 1, fp);
			l += ((unsigned int)b << 8);

			char *value = (char *)safe_malloc(l + 1);

			fread(value, l, 1, fp);
			len -= (l + 2);
			value[l] = 0;
			// cprintf("setting %s to %s\n",name,value);
			if (!strcmp(name, "nvram_ver"))
				nvram_ver = value;

			if (!nvram_critical(name)) {
				nvram_immed_set(name, value);
			}
			free(value);
			free(name);
		}
		nvram_close();
	} else {
		return -2;
	}

	if (nvram_ver == NULL) {
		nvram_set("http_passwd", zencrypt(nvram_safe_get("http_passwd")));
		nvram_set("http_username", zencrypt(nvram_safe_get("http_username")));
		if (nvram_get("newhttp_passwd") != NULL) {
			nvram_set("newhttp_passwd", zencrypt(nvram_safe_get("newhttp_passwd")));
			nvram_set("newhttp_username", zencrypt(nvram_safe_get("newhttp_username")));
		}
	}
	nvram_commit();

	return 0;

}

int nvram_backup(char *filename)
{

	int backupcount = 0;
	char sign[7] = { "DD-WRT" };
#ifdef HAVE_REGISTER
	if (!isregistered_real()) {
		return -1;
	}
#endif
	char *buf = (char *)safe_malloc(NVRAMSPACE);

	nvram_getall(buf, NVRAMSPACE);
	char *p = buf;
	int i;

	for (i = 0; i < NVRAMSPACE; i++) {
		if (i > 0 && buf[i] == 0 && buf[i - 1] == 0) {
			break;
		}
		if (buf[i] == 0)
			backupcount++;
	}
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return -1;

	fwrite(sign, 6, 1, fp);
	fputc(backupcount & 255, fp);	// high byte
	fputc(backupcount >> 8, fp);	// low byte
	//first save all "wl_" prefixed parameters
	save(fp, p, 1);
	nvram_getall(buf, NVRAMSPACE);
	p = buf;
	//now save anything else (this should prevent problems with backups, since wl0 parameters are getting higher priority now which solves restore problems with wds etc.
	save(fp, p, 0);
	free(buf);
	fclose(fp);
	return 0;
}

#ifdef TEST
int main(int argc, char *argv[])
{

	nvram_backup("/tmp/fixup.bin");
	nvram_restore("/tmp/fixup.bin");
}

#endif
