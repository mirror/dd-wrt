/*
 * quantenna.c
 *
 * Copyright (C) 2014 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_QTN
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

struct txpower_ac_qtn_s {
	uint16 min;
	uint16 max;
	uint8 pwr;
};

static const struct txpower_ac_qtn_s txpower_list_qtn_rtac87u[] = {
	/* 1 ~ 25% */
	{ 1, 25, 14},
	/* 26 ~ 50% */
	{ 26, 50, 17},
	/* 51 ~ 75% */
	{ 51, 75, 20},
	/* 76 ~ 100% */
	{ 76, 100, 23},
	{ 0, 0, 0x0}
};


int get_tx_power_qtn(void)
{
	const struct txpower_ac_qtn_s *p_to_table;
	int txpower = 80;

	p_to_table = &txpower_list_qtn_rtac87u[0];
	txpower = atoi(nvram_safe_get("wl1_txpwr"));

	for (p_to_table; p_to_table->min != 0; ++p_to_table) {
		if (txpower >= p_to_table->min && txpower <= p_to_table->max) {
			return p_to_table->pwr;
		}
	}

	/* default max power */
	return 23;
}


int gen_stateless_conf(void)
{
	int ret;
	FILE *fp;

	int l_len;
	// check security
	char auth[8];
	char crypto[16];
	char beacon[] = "WPAand11i";
	char encryption[] = "TKIPandAESEncryption";
	char key[65];
	char ssid[65];
	char region[5];
	int channel = atoi(nvram_safe_get("wl1_channel"));
	int bw = atoi(nvram_safe_get("wl1_nbw"));
	uint32_t index = 0;

	sprintf(ssid, "%s", nvram_safe_get("wl1_ssid"));
	sprintf(region, "%s", nvram_safe_get("1:ccode"));

	fp = fopen("/tmp/stateless_slave_config", "w");

	if(nvram_match("wl1_mode","sta")){
		/* media bridge mode */
		fprintf(fp, "wifi0_mode=sta\n");

		strncpy(auth, nvram_safe_get("wl1_akm"), sizeof(auth));
		strncpy(crypto, nvram_safe_get("wl1_crypto"), sizeof(crypto));
		strncpy(key, nvram_safe_get("wl1_wpa_psk"), sizeof(key));

		strncpy(ssid, nvram_safe_get("wl1_ssid"), sizeof(ssid));
		fprintf(fp, "wifi0_SSID=\"%s\"\n", ssid);
		
		/* convert security from nvram to qtn */
		if(!strcmp(auth, "psk2") && !strcmp(crypto, "aes")){
			fprintf(fp, "wifi0_auth_mode=PSKAuthentication\n");
			fprintf(fp, "wifi0_beacon=11i\n");
			fprintf(fp, "wifi0_encryption=AESEncryption\n");
			fprintf(fp, "wifi0_passphrase=%s\n", key);
		}
		else if(!strcmp(auth, "pskpsk2") && !strcmp(crypto, "aes") ){
			fprintf(fp, "wifi0_auth_mode=PSKAuthentication\n");
			fprintf(fp, "wifi0_beacon=WPAand11i\n");
			fprintf(fp, "wifi0_encryption=AESEncryption\n");
			fprintf(fp, "wifi0_passphrase=%s\n", key);
		}
		else if(!strcmp(auth, "pskpsk2") && !strcmp(crypto, "tkip+aes") ){
			fprintf(fp, "wifi0_auth_mode=PSKAuthentication\n");
			fprintf(fp, "wifi0_beacon=WPAand11i\n");
			fprintf(fp, "wifi0_encryption=TKIPandAESEncryption\n");
			fprintf(fp, "wifi0_passphrase=%s\n", key);
		}
		else{
			fprintf(fp, "wifi0_auth_mode=NONE\n");
			fprintf(fp, "wifi0_beacon=Basic\n");
		}

		/* auto channel for media bridge mode */
		channel = 0;
	}else{
		/* not media bridge mode */
		fprintf(fp, "wifi0_mode=ap\n");

		strncpy(auth, nvram_safe_get("wl1_akm"), sizeof(auth));
		strncpy(crypto, nvram_safe_get("wl1_crypto"), sizeof(crypto));
		strncpy(key, nvram_safe_get("wl1_wpa_psk"), sizeof(key));

		strncpy(ssid, nvram_safe_get("wl1_ssid"), sizeof(ssid));
		fprintf(fp, "wifi0_SSID=\"%s\"\n", ssid);

		if(!strcmp(auth, "psk2") && !strcmp(crypto, "aes")){
			fprintf(fp, "wifi0_auth_mode=PSKAuthentication\n");
			fprintf(fp, "wifi0_beacon=11i\n");
			fprintf(fp, "wifi0_encryption=AESEncryption\n");
			fprintf(fp, "wifi0_passphrase=%s\n", key);
		}
		else if(!strcmp(auth, "pskpsk2") && !strcmp(crypto, "aes") ){
			fprintf(fp, "wifi0_auth_mode=PSKAuthentication\n");
			fprintf(fp, "wifi0_beacon=WPAand11i\n");
			fprintf(fp, "wifi0_encryption=AESEncryption\n");
			fprintf(fp, "wifi0_passphrase=%s\n", key);
		}
		else if(!strcmp(auth, "pskpsk2") && !strcmp(crypto, "tkip+aes") ){
			fprintf(fp, "wifi0_auth_mode=PSKAuthentication\n");
			fprintf(fp, "wifi0_beacon=WPAand11i\n");
			fprintf(fp, "wifi0_encryption=TKIPandAESEncryption\n");
			fprintf(fp, "wifi0_passphrase=%s\n", key);
		}
		else{
			fprintf(fp, "wifi0_beacon=Basic\n");
		}
	}
	fprintf(fp, "wifi0_region=%s\n", "us");
	nvram_set("wl1_country_code", nvram_safe_get("1:ccode"));
	fprintf(fp, "wifi0_vht=1\n");
	if(bw==20) fprintf(fp, "wifi0_bw=20\n");
	else if(bw==40) fprintf(fp, "wifi0_bw=40\n");
	else if(bw==80) fprintf(fp, "wifi0_bw=80\n");
	else fprintf(fp, "wifi0_bw=80\n");

	/* if media bridge mode, always auto channel */
	fprintf(fp, "wifi0_channel=%d\n", channel);
	fprintf(fp, "wifi0_pwr=%d\n", get_tx_power_qtn());
	if(nvram_match("wl1_itxbf","1") || nvram_match("wl1_txbf","1")){
		fprintf(fp, "wifi0_bf=1\n");
	}else{
		fprintf(fp, "wifi0_bf=0\n");
	}
	fprintf(fp, "wifi0_staticip=1\n");
	fprintf(fp, "slave_ipaddr=\"192.168.1.111/16\"\n");

	fclose(fp);

	return 1;
}


void start_qtn(void)
{
	gen_stateless_conf();
	sysprintf("cp /etc/qtn/* /tmp/");
	eval("ifconfig", "br0:1", "169.254.39.1", "netmask", "255.255.255.0");
	eval("ifconfig", "br0:2", "1.1.1.1", "netmask", "255.255.255.0");
	eval("tftpd"); // bootloader from qtn will load files from /tmp directory now
	return;
}

void stop_qtn(void)
{
	return;
}
#endif
