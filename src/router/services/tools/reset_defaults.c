/*
 * beep.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

static char *filter[] = { "lan_ifnames",
	"lan_ifname",
	"wan_ifnames",
	"wan_ifname",
	"et0macaddr",
	"et0macaddr_safe",
	"et1macaddr",
	"et2macaddr",
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
	"et1mdcport",
	"et2mdcport",
	"devpath0",
	"devpath1",
	"devpath2",
	"gpio0",
	"gpio1",
	"gpio2",
	"gpio3",
	"gpio4",
	"gpio5",
	"gpio6",
	"gpio7",
	"gpio8",
	"gpio9",
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
	"gpio20",
	"gpio21",
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
	"itt2ga0" "pa2gw0a0",
	"pa2gw1a0",
	"pa2gw2a0",
	"maxp2ga1",
	"itt2ga1" "pa2gw0a1",
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
	"et1phyaddr",
	"et2phyaddr",
	"hardware_version",
	"test_led_gpio",
	"GemtekPmonVer",
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
	"DEF-p_wireless_eth1_11bg-authmode_ex",
	"DEF-p_wireless_eth1_11bg-authmode",
	"DEF-p_wireless_eth1_11bg-crypto",
	"DEF-p_wireless_eth1_11bg-wpapsk",
	"DEF-p_wireless_eth2_11a-authmode_ex",
	"DEF-p_wireless_eth2_11a-authmode",
	"DEF-p_wireless_eth2_11a-crypto",
	"DEF-p_wireless_eth2_11a-wpapsk",
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
	"default_ssid",
	"default_passphrase",
	"tc_passphrase",
	"tc_ssid",
	"image_second_offset",
	"image_first_offset",
	"serial_number",
	"tftpd_ipaddr",
	"modelNumber",
	"manufacturer",
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
	"HW_ver",
	"pcie_force_gen1",
	"csromrev",
	"ATEMODE",
	"PA",
	"nospare",
	"cert_region",
	"devinfo_version",
	"modelDescription",
	"loader_version",
	"gs_sku_id",
	"et_txq_thresh",
	"sn",
	"um_board_id",
	NULL
};

extern struct nvram_param *srouter_defaults;

static int isCritical(char *name)
{
	int a = 0;

	while (filter[a] != NULL) {
		if (!strcmp(name, filter[a++])) {
			return 1;
		}
	}
	return 0;
}

extern void load_defaults(void);
extern void free_defaults(void);

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#else
#define NVRAMSPACE NVRAM_SPACE
#endif

void start_defaults(void)
{
	fprintf(stderr, "restore nvram to defaults\n");
	char *buf = (char *)malloc(NVRAMSPACE);
	int i;
	struct nvram_param *t;

	nvram_getall(buf, NVRAMSPACE);
	char *p = buf;

	//clean old values
	while (strlen(p) != 0) {
		int len = strlen(p);

		for (i = 0; i < len; i++)
			if (p[i] == '=')
				p[i] = 0;
		char *name = p;

		if (!isCritical(name))
			nvram_unset(name);
		p += len + 1;
	}
	load_defaults();
	for (t = srouter_defaults; t->name; t++) {
		nvram_set(t->name, t->value);
	}
	free_defaults();
	free(buf);
	nvram_commit();
}
