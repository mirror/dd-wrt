/*
 * backup.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <bcmtypedefs.h>
//#include <netconf.h>
#include <bcmnvram.h>
//#include <shutils.h>
//#include <utils.h>
#define NVROUTER "DD_BOARD"
#define NVROUTER_ALT "alternate_name"

static int NVRAMSPACE = NVRAM_SPACE;

static char *filter[] = {
	"ATEMODE",
	"Ate_power_on_off_ret",
	"CFEver",
	"DEF-p_wireless_eth1_11a-authmode",
	"DEF-p_wireless_eth1_11a-authmode_ex",
	"DEF-p_wireless_eth1_11a-crypto",
	"DEF-p_wireless_eth1_11a-wpapsk",
	"DEF-p_wireless_eth1_11bg-authmode",
	"DEF-p_wireless_eth1_11bg-authmode_ex",
	"DEF-p_wireless_eth1_11bg-crypto",
	"DEF-p_wireless_eth1_11bg-wpapsk",
	"DEF-p_wireless_eth2_11a-authmode",
	"DEF-p_wireless_eth2_11a-authmode_ex",
	"DEF-p_wireless_eth2_11a-crypto",
	"DEF-p_wireless_eth2_11a-wpapsk",
	"DEF-p_wireless_eth2_11bg-authmode",
	"DEF-p_wireless_eth2_11bg-authmode_ex",
	"DEF-p_wireless_eth2_11bg-crypto",
	"DEF-p_wireless_eth2_11bg-wpapsk",
	"GemtekPmonVer",
	"HW_ver",
	"PA",
	"aa2g",
	"af_hash",
	"ag0",
	"ag1",
	"antswctl2g",
	"antswitch",
	"bl_version",
	"blink_diag_led",
	"boardflags",
	"boardflags2",
	"boardflags3",
	"boardnum",
	"boardpwrctl",
	"boardrev",
	"boardtype",
	"boot_hw_model",
	"boot_hw_ver",
	"boot_ver",
	"boot_wait",
	"bootflags",
	"bootnv_ver",
	"bootpartition",
	"bw40po",
	"bwduppo",
	"bxa2g",
	"bypasscrccheck",
	"cardbus",
	"cck2gpo",
	"ccode",
	"cddpo",
	"cert_region",
	"cfe_disable_2G",
	"cfe_disable_5G",
	"cfe_ping_timeout",
	"cisco_default",
	"clkdivsf",
	"clkfreq",
	"csromrev",
	"cthwver",
	"custom_id",
	"default_passphrase",
	"default_ssid",
	"deviceType",
	"devid",
	"devinfo_version",
	"devpath0",
	"devpath1",
	"devpath2",
	"devpath3",
	"et0macaddr",
	"et0macaddr_safe",
	"et0mdcport",
	"et0phyaddr",
	"et1macaddr",
	"et1mdcport",
	"et1phyaddr",
	"et2macaddr",
	"et2mdcport",
	"et2phyaddr",
	"et_pwrsave",
	"et_txq_thresh",
	"extpagain2g",
	"firmwarename",
	"gpio0",
	"gpio1",
	"gpio10",
	"gpio11",
	"gpio12",
	"gpio13",
	"gpio14",
	"gpio15",
	"gpio16",
	"gpio17",
	"gpio18",
	"gpio19",
	"gpio2",
	"gpio20",
	"gpio21",
	"gpio22",
	"gpio3",
	"gpio4",
	"gpio5",
	"gpio6",
	"gpio7",
	"gpio8",
	"gpio9",
	"gs_sku_id",
	"hardware_version",
	"hw_mac_addr",
	"hw_model",
	"hw_rev",
	"hw_revision",
	"hw_version",
	"il0macaddr",
	"image_first_offset",
	"image_second_offset",
	"itt2ga0",
	"itt2ga1",
	"lan_ifname",
	"lan_ifnames",
	"landevs",
	"ledbh0",
	"ledbh10",
	"ledbh11",
	"ledbh2",
	"ledbh3",
	"ledbh5",
	"leddc",
	"loader_version",
	"macaddr",
	"manufacturer",
	"manufacturerURL",
	"manufacturer_date",
	"maxp2ga0",
	"maxp2ga1",
	"maxpartialboots",
	"mcs2gpo0",
	"mcs2gpo1",
	"mcs2gpo2",
	"mcs2gpo3",
	"mcs2gpo4",
	"mcs2gpo5",
	"mcs2gpo6",
	"mcs2gpo7",
	"melco_id",
	"mfg_data_version",
	"mfg_wait",
	"model",
	"modelDescription",
	"modelNumber",
	"nospare",
	"ntype",
	"nvram_factory_reset",
	"nvram_reboot",
	"nvram_space",
	"nvram_version",
	"odmpid",
	"ofdm2gpo",
	"os_ram_addr",
	"pa2gw0a0",
	"pa2gw0a1",
	"pa2gw1a0",
	"pa2gw1a1",
	"pa2gw2a0",
	"pa2gw2a1",
	"parefldovoltage",
	"pcie_force_gen1",
	"pdetrange2g",
	"phyid_num",
	"pincode",
	"pmon_date",
	"pmon_ver",
	"prodid",
	"product",
	"productid",
	"product_name",
	"region",
	"regrev",
	"regulation_domain",
	"regulation_domain_5G",
	"reset_gpio",
	"rgmii_port",
	"rssisav2g",
	"rssismc2g",
	"rssismf2g",
	"rxchain",
	"rxpo2g",
	"sdram_config",
	"sdram_init",
	"sdram_ncdl",
	"sdram_refresh",
	"secret_code",
	"serial_no",
	"serial_number",
	"sn",
	"sromrev",
	"stbcpo",
	"strap_p5_mode",
	"sw_version",
	"tc_passphrase",
	"tc_ssid",
	"territory_code",
	"test_led_gpio",
	"tftp_max_retries",
	"tftp_recv_timeout",
	"tftp_rrq_timeout",
	"tftpd_ipaddr",
	"tftpserverip",
	"tri2g",
	"triso2g",
	"tssipos2g",
	"txchain",
	"um_board_id",
	"usb_gpio_reverse",
	"uuid_key",
	"vlan0hwname",
	"vlan0ports",
	"vlan1hwname",
	"vlan1ports",
	"vlan2hwname",
	"vlan2ports",
	"wait_time",
	"wan_ifname",
	"wan_ifnames",
	"wandevs",
	"watchdog",
	"wl0_country_code",
	"wl0_country_rev",
	"wl0_wandevs",
	"wl0gpio0",
	"wl0gpio1",
	"wl0gpio2",
	"wl0gpio3",
	"wl0gpio4",
	"wl1_country_code",
	"wl1_country_rev",
	"wl_country",
	"wl_dmarxctl",
	"wl_dmatxctl",
	"wl_msglevel",
	"wl_pcie_mrrs",
	"wl_use_coregpio",
	"wltest",
	"wps_lock_forever_cnt",
	"xtalfreq",
	"cpurev",
	"color",
	"PA",
	"nocountrysel",
	"nvram_min_ver",
	"board_id",		//netgear specific
	"hwrev",
	"hwver",
	"winbond_flash",
	"scratch",
	"dl_ram_addr",
	"os_flash_addr",
	"wl0id",
	"wl1id",
	"wl2id",
	"ctf_fa_cap",
	NULL
};

static int nvram_critical_internal(char *name, int ommit)
{
	int a = 0;
	int len = strlen(name);
	int i;
	if (ommit) {
		for (i = 0; i < len; i++) {
			if (!isascii(name[i])) {
				fprintf(stderr, "ommit %s (illegal nvram name)\n", name);
				return 1;
			}
		}
	}
	while (filter[a] != NULL) {
		if (!strcmp(name, filter[a++])) {
			return 1;
		}
	}
	if (!strcmp(name, "lan_hwaddr")) {
		nvram_unset(name);
		return 1;
	}
	if (!strcmp(name, "wlan0_hwaddr")) {
		nvram_unset(name);
		return 1;
	}
	if (!strcmp(name, "wlan1_hwaddr")) {
		nvram_unset(name);
		return 1;
	}
	if (!strcmp(name, "wlan2_hwaddr")) {
		nvram_unset(name);
		return 1;
	}
	if (!strcmp(name, "wlan3_hwaddr")) {
		nvram_unset(name);
		return 1;
	}
	if (strncmp(name, "sb/", 3) && strncmp(name, "pci/", 4) && strncmp(name, "pcie/", 5) && strncmp(name, "0:", 2) && strncmp(name, "1:", 2) && strncmp(name, "2:", 2) && strncmp(name, "3:", 2)
	    && !strstr(name, "_hwaddr"))
		return 0;
	else
		return 1;
}

int nvram_critical(char *name)
{
	return nvram_critical_internal(name, 1);
}

void nvram_clear(void)
{
	NVRAMSPACE = nvram_size();
	if (NVRAMSPACE < 0) {
		fprintf(stderr, "nvram driver returns bogus space\n");
		return;
	}
	char *buf = (char *)malloc(NVRAMSPACE);

	nvram_getall(buf, NVRAMSPACE);
	nvram_open();
	char *p = buf;
	int i;

	while (*p) {
		int len = strlen(p);

		for (i = 0; i < len; i++)
			if (p[i] == '=')
				p[i] = 0;
		if (!nvram_critical_internal(p, 0))
			nvram_immed_set(p, NULL);
		p += len + 1;
	}
	nvram_close();
	free(buf);
}

static void save(FILE * fp, char *p, int not)
{
	int i;
	while (*p) {
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
		const char *val = nvram_safe_get(name);
		fputc(strlen(val) & 255, fp);
		fputc(strlen(val) >> 8, fp);
		fwrite(val, 1, strlen(val), fp);
		p += len + 1;
	}

}

#define getRouterName() nvram_exists(NVROUTER_ALT)?nvram_safe_get(NVROUTER_ALT):nvram_safe_get(NVROUTER)

int nvram_restore(char *filename, int force)
{
	char sign[7];
//      char *nvram_ver = NULL;
#ifdef HAVE_REGISTER
	if (!isregistered_real()) {
		return -1;
	}
#endif
	sign[6] = 0;
	int c = 0;
	for (c = 0; c < 2; c++) {
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
			if (c) {
				nvram_clear();
				nvram_open();
			}
			unsigned char b;

			fread((char *)&b, 1, 1, fp);
			count = b;
			fread((char *)&b, 1, 1, fp);
			count += ((unsigned int)b << 8);
			len -= 2;
			int i;

			for (i = 0; i < count && len > 0; i++) {
				unsigned short l = 0;
				unsigned char cl = 0;

				fread((char *)&cl, 1, 1, fp);
				char *name = (char *)malloc(cl + 1);

				fread(name, cl, 1, fp);
				name[cl] = 0;
				len -= (cl + 1);

				fread((char *)&b, 1, 1, fp);
				l = b;
				fread((char *)&b, 1, 1, fp);
				l += ((unsigned int)b << 8);

				char *value = (char *)malloc(l + 1);

				fread(value, l, 1, fp);
				len -= (l + 2);
				value[l] = 0;
//                              if (!strcmp(name, "nvram_ver"))
//                                      nvram_ver = value;
				char *routername = getRouterName();
#if defined(HAVE_NEWPORT) || defined(HAVE_VENTANA) || defined(HAVE_LAGUNA)
				if (!strncmp(routername, "Gateworks Newport GW61", 22) && !strncmp(value, "Gateworks Newport GW61", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Newport GW62", 22) && !strncmp(value, "Gateworks Newport GW62", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Newport GW63", 22) && !strncmp(value, "Gateworks Newport GW63", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Newport GW64", 22) && !strncmp(value, "Gateworks Newport GW64", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Newport GW65", 22) && !strncmp(value, "Gateworks Newport GW65", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Newport GW69", 22) && !strncmp(value, "Gateworks Newport GW69", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Ventana GW51", 22) && !strncmp(value, "Gateworks Ventana GW51", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Ventana GW52", 22) && !strncmp(value, "Gateworks Ventana GW52", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Ventana GW53", 22) && !strncmp(value, "Gateworks Ventana GW53", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Ventana GW54", 22) && !strncmp(value, "Gateworks Ventana GW54", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Ventana GW55", 22) && !strncmp(value, "Gateworks Ventana GW55", 22))
					goto success;
				if (!strncmp(routername, "Gateworks Laguna GW23", 21) && !strncmp(value, "Gateworks Laguna GW23", 21))
					goto success;
#endif
				if (!c && !strcmp(name, routername)) {
					fprintf(stdout, "backup is for board %s, board is %s\n", value, routername);
#ifndef HAVE_X86
					if (!nvram_match(routername, value)) {
						if (!force) {
							fprintf(stderr, "incompatible backup file!\n");
							fclose(fp);
							return -2;
						} else {
							fprintf(stderr, "WARNING: incompatible backup file!\n");
						}
					}
#endif
				}
			      success:;
				if (c && !nvram_critical(name)) {
					nvram_immed_set(name, value);
				}
				free(value);
				free(name);
			}
			if (c) {
				nvram_close();
				_nvram_commit();
			}
		} else {
			fclose(fp);
			return -2;
		}
		fclose(fp);
	}
	return 0;

}

int nvram_size(void);
int nvram_backup(char *filename)
{

	int backupcount = 0;
	char sign[7] = { "DD-WRT" };
#ifdef HAVE_REGISTER
	if (!isregistered_real()) {
		return -1;
	}
#endif
	NVRAMSPACE = nvram_size();
	if (NVRAMSPACE < 0) {
		fprintf(stderr, "nvram driver returns bogus space\n");
		return -1;
	}
	char *buf = (char *)malloc(NVRAMSPACE);
	memset(buf, 0, NVRAMSPACE);

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
	if (!fp) {
		return -1;
	}

	fwrite(sign, 6, 1, fp);
	fputc(backupcount & 255, fp);	// high byte
	fputc(backupcount >> 8, fp);	// low byte
	//first save all "wl_" prefixed parameters
	save(fp, p, 1);
	nvram_getall(buf, NVRAMSPACE);
	p = buf;
	//now save anything else (this should prevent problems with backups, since wl0 parameters are getting higher priority now which solves restore problems with wds etc.
	save(fp, p, 0);
	fclose(fp);
	free(buf);
	return 0;
}

#ifdef TEST
int main(int argc, char *argv[])
{

	nvram_backup("/tmp/fixup.bin");
	nvram_restore("/tmp/fixup.bin", 0);
}

#endif
