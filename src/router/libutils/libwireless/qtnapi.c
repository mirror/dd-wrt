/*
 * qtnapi.c
 *
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <bcmnvram.h>
#include <wlioctl.h>
#include <wlutils.h>

#include <shutils.h>
#include <utils.h>
#include <sys/sysinfo.h>

#include <qtnapi.h>

#define dbG(a, ...)

#define dbg(fmt, args...) fprintf(stderr, fmt, ##args)
#define dbG(fmt, args...) dbg("%s(0x%04x): " fmt, __FUNCTION__, __LINE__, ##args)

#define MAX_RETRY_TIMES 30
#define MAX_TOTAL_TIME 120
#define WIFINAME "wifi0"

static int s_c_rpc_use_udp = 0;
static int qtn_qcsapi_init = 0;
static int qtn_init = 0;

char *wl_vifname_qtn(int unit, int subunit)
{
	static char tmp[128];

	if ((subunit > 0) && (subunit < 4)) {
		sprintf(tmp, "wifi%d", subunit);
		return strdup(tmp);
	} else
		return strdup("");
}

/* Serialize using fcntl() calls 
 */

int file_lock(char *tag)
{
	char fn[64];
	struct flock lock;
	int lockfd = -1;
	pid_t lockpid;

	sprintf(fn, "/tmp/%s.lock", tag);
	if ((lockfd = open(fn, O_CREAT | O_RDWR, 0666)) < 0)
		goto lock_error;

	pid_t pid = getpid();
	if (read(lockfd, &lockpid, sizeof(pid_t))) {
		// check if we already hold a lock
		if (pid == lockpid) {
			// don't close the file here as that will release all locks
			return -1;
		}
	}

	bzero(&lock, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_pid = pid;

	if (fcntl(lockfd, F_SETLKW, &lock) < 0) {
		close(lockfd);
		goto lock_error;
	}

	lseek(lockfd, 0, SEEK_SET);
	write(lockfd, &pid, sizeof(pid_t));
	return lockfd;
lock_error:
	// No proper error processing
	dd_syslog(LOG_INFO, "Error %d locking %s, proceeding anyway", errno, fn);
	return -1;
}

void file_unlock(int lockfd)
{
	if (lockfd >= 0) {
		ftruncate(lockfd, 0);
		close(lockfd);
	}
}

time_t uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

int rpc_qcsapi_init(int verbose)
{
	const char *host;
	CLIENT *clnt;
	int retry = 0;
	time_t start_time = uptime();

	/* setup RPC based on udp protocol */
	do {
		if (verbose)
			dbG("#%d attempt to create RPC connection\n", retry + 1);

		host = client_qcsapi_find_host_addr(0, NULL);
		if (!host) {
			if (verbose)
				dbG("Cannot find the host\n");
			sleep(1);
			continue;
		}

		if (!s_c_rpc_use_udp) {
			clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
		} else {
			clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
		}

		if (clnt == NULL) {
			clnt_pcreateerror(host);
			sleep(1);
			continue;
		} else {
			client_qcsapi_set_rpcclient(clnt);
			qtn_qcsapi_init = 1;
			return 0;
		}
	} while ((retry++ < MAX_RETRY_TIMES) && ((uptime() - start_time) < MAX_TOTAL_TIME));

	return -1;
}

#define nvram_get_int(name) nvram_geti(name)

int rpc_qtn_ready()
{
	int ret, qtn_ready;
	int lock;

	qtn_ready = nvram_get_int("qtn_ready");

	lock = file_lock("qtn");

	if (qtn_ready && !qtn_init) {
		ret = rpc_qcsapi_init(0);
		if (ret < 0) {
			qtn_ready = 0;
			dbG("rpc_qcsapi_init error, return: %d\n", ret);
		} else {
			ret = qcsapi_init();
			if (ret < 0) {
				qtn_ready = 0;
				dbG("Qcsapi qcsapi_init error, return: %d\n", ret);
			} else
				qtn_init = 1;
		}
	}

	file_unlock(lock);

	nvram_set("wl1_country_code", nvram_safe_get("1:ccode"));
	return qtn_ready;
}

int rpc_qcsapi_set_SSID(const char *ifname, const char *ssid)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_set_SSID(ifname, ssid);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_SSID %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("Set SSID of interface %s as: %s\n", ifname, ssid);

	return 0;
}

int rpc_qcsapi_set_SSID_broadcast(const char *ifname, const char *option)
{
	int ret;
	int OPTION = 1 - atoi(option);

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_set_option(ifname, qcsapi_SSID_broadcast, OPTION);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_option::SSID_broadcast %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("Set Broadcast SSID of interface %s as: %s\n", ifname, OPTION ? "TRUE" : "FALSE");

	return 0;
}

int enable_qtn_telnetsrv(int enable_flag)
{
	int ret;

	if (!rpc_qtn_ready()) {
		fprintf(stderr, "ATE command error\n");
		return -1;
	}
	if (enable_flag == 0) {
		ret = qcsapi_wifi_run_script("set_test_mode", "enable_telnet_srv 0");
	} else {
		ret = qcsapi_wifi_run_script("set_test_mode", "enable_telnet_srv 1");
	}
	if (ret < 0) {
		fprintf(stderr, "[ate] set telnet server error\n");
		return -1;
	}
	return 0;
}

int rpc_qcsapi_set_vht(const char *mode)
{
	int ret;
	int VHT;

	if (!rpc_qtn_ready())
		return -1;

	switch (atoi(mode)) {
	case 0:
		VHT = 1;
		break;
	default:
		VHT = 0;
		break;
	}

	ret = qcsapi_wifi_set_vht(WIFINAME, VHT);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_vht %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}
	dbG("Set wireless mode of interface %s as: %s\n", WIFINAME, VHT ? "11ac" : "11n");

	return 0;
}

int rpc_qcsapi_set_bw(const char *bw)
{
	int ret;
	int BW = 20;

	if (!rpc_qtn_ready())
		return -1;

	switch (atoi(bw)) {
	case 20:
		BW = 20;
		break;
	case 40:
		BW = 40;
		break;
	case 0:
	case 80:
		BW = 80;
		break;
	}

	ret = qcsapi_wifi_set_bw(WIFINAME, BW);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_bw %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}
	dbG("Set bw of interface %s as: %d MHz\n", WIFINAME, BW);

	return 0;
}

int rpc_qcsapi_set_channel(const char *chspec_buf)
{
	int ret;
	int channel = 0;
	char str_ch[] = "149";

	if (!rpc_qtn_ready())
		return -1;

	channel = atoi(chspec_buf);

	ret = qcsapi_wifi_set_channel(WIFINAME, channel);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_channel %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}
	dbG("Set channel of interface %s as: %d\n", WIFINAME, channel);

	snprintf(str_ch, sizeof(str_ch), "%d", channel);
	ret = qcsapi_config_update_parameter(WIFINAME, "channel", str_ch);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_config_update_parameter %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}
	dbG("update channel to wireless_conf.txt %s as: %s\n", WIFINAME, str_ch);

	return 0;
}

int rpc_qcsapi_set_beacon_type(const char *ifname, const char *auth_mode)
{
	int ret;
	char *p_new_beacon = NULL;

	if (!rpc_qtn_ready())
		return -1;

	if (!strcmp(auth_mode, "open"))
		p_new_beacon = strdup("Basic");
	else if (!strcmp(auth_mode, "psk"))
		p_new_beacon = strdup("WPA");
	else if (!strcmp(auth_mode, "psk2"))
		p_new_beacon = strdup("11i");
	else if (!strcmp(auth_mode, "pskpsk2"))
		p_new_beacon = strdup("WPAand11i");
	else
		p_new_beacon = strdup("Basic");

	ret = qcsapi_wifi_set_beacon_type(ifname, p_new_beacon);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_beacon_type %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("Set beacon type of interface %s as: %s\n", ifname, p_new_beacon);

	if (p_new_beacon)
		free(p_new_beacon);

	return 0;
}

int rpc_qcsapi_set_WPA_encryption_modes(const char *ifname, const char *crypto)
{
	int ret;
	string_32 encryption_modes;

	if (!rpc_qtn_ready())
		return -1;

	if (!strcmp(crypto, "tkip"))
		strcpy(encryption_modes, "TKIPEncryption");
	else if (!strcmp(crypto, "aes"))
		strcpy(encryption_modes, "AESEncryption");
	else if (!strcmp(crypto, "tkip+aes"))
		strcpy(encryption_modes, "TKIPandAESEncryption");
	else
		strcpy(encryption_modes, "AESEncryption");

	ret = qcsapi_wifi_set_WPA_encryption_modes(ifname, encryption_modes);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_WPA_encryption_modes %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("Set WPA encryption mode of interface %s as: %s\n", ifname, encryption_modes);

	return 0;
}

int rpc_qcsapi_set_region(const char *ifname, const char *region)
{
	int ret, qcsapi_retval;

	if (!rpc_qtn_ready())
		return -1;

	qcsapi_retval = qcsapi_regulatory_set_regulatory_region(ifname, region);

	if (qcsapi_retval == -qcsapi_region_database_not_found) {
		qcsapi_retval = qcsapi_wifi_set_regulatory_region(ifname, region);
	}
	return 0;
}

int rpc_qcsapi_set_key_passphrase(const char *ifname, const char *wpa_psk)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_set_key_passphrase(ifname, 0, wpa_psk);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_key_passphrase %s error, return: %d\n", ifname, ret);

		ret = qcsapi_wifi_set_pre_shared_key(ifname, 0, wpa_psk);
		if (ret < 0)
			dbG("Qcsapi qcsapi_wifi_set_pre_shared_key %s error, return: %d\n", ifname, ret);

		return ret;
	}
	dbG("Set WPA preshared key of interface %s as: %s\n", ifname, wpa_psk);

	return 0;
}

int rpc_qcsapi_set_dtim(const char *dtim)
{
	int ret;
	int DTIM = atoi(dtim);

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_set_dtim(WIFINAME, DTIM);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_dtim %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}
	dbG("Set dtim of interface %s as: %d\n", WIFINAME, DTIM);

	return 0;
}

int rpc_qcsapi_set_beacon_interval(const char *beacon_interval)
{
	int ret;
	int BCN = atoi(beacon_interval);

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_set_beacon_interval(WIFINAME, BCN);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_beacon_interval %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}
	dbG("Set beacon_interval of interface %s as: %d\n", WIFINAME, BCN);

	return 0;
}

int rpc_qcsapi_set_mac_address_filtering(const char *ifname, const char *mac_address_filtering)
{
	int ret;
	qcsapi_mac_address_filtering MAF;
	qcsapi_mac_address_filtering orig_mac_address_filtering;

	if (!rpc_qtn_ready())
		return -1;

	ret = rpc_qcsapi_get_mac_address_filtering(ifname, &orig_mac_address_filtering);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_mac_address_filtering %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("Original mac_address_filtering setting of interface %s: %d\n", ifname, orig_mac_address_filtering);

	if (!strcmp(mac_address_filtering, "disabled"))
		MAF = qcsapi_disable_mac_address_filtering;
	else if (!strcmp(mac_address_filtering, "deny"))
		MAF = qcsapi_accept_mac_address_unless_denied;
	else if (!strcmp(mac_address_filtering, "allow"))
		MAF = qcsapi_deny_mac_address_unless_authorized;
	else
		MAF = qcsapi_disable_mac_address_filtering;

	ret = qcsapi_wifi_set_mac_address_filtering(ifname, MAF);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_mac_address_filtering %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("Set mac_address_filtering of interface %s as: %d (%s)\n", ifname, MAF, mac_address_filtering);

	if ((MAF != orig_mac_address_filtering) && (MAF != qcsapi_disable_mac_address_filtering))
		rpc_qcsapi_set_wlmaclist(ifname);

	return 0;
}

void rpc_update_macmode(const char *mac_address_filtering)
{
	int ret;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	int i, unit = 1;

	if (!rpc_qtn_ready())
		return;

	ret = rpc_qcsapi_set_mac_address_filtering(WIFINAME, mac_address_filtering);
	if (ret < 0) {
		dbG("rpc_qcsapi_set_mac_address_filtering %s error, return: %d\n", WIFINAME, ret);
	}

	char *next;
	char var[80];
	char *vifs = nvram_safe_get("wl1_vifs");
	foreach(var, vifs, next)
	{
		ret = rpc_qcsapi_set_mac_address_filtering(wl_vifname_qtn(unit, i), mac_address_filtering);
		if (ret < 0)
			dbG("rpc_qcsapi_set_mac_address_filtering %s error, return: %d\n", wl_vifname_qtn(unit, i), ret);
	}
}

int rpc_qcsapi_authorize_mac_address(const char *ifname, const char *macaddr)
{
	int ret;
	qcsapi_mac_addr address_to_authorize;

	if (!rpc_qtn_ready())
		return -1;

	ether_atoe(macaddr, address_to_authorize);
	ret = qcsapi_wifi_authorize_mac_address(ifname, address_to_authorize);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_authorize_mac_address %s error, return: %d\n", ifname, ret);
		return ret;
	}
	//      dbG("authorize MAC addresss of interface %s: %s\n", ifname, macaddr);

	return 0;
}

int rpc_qcsapi_deny_mac_address(const char *ifname, const char *macaddr)
{
	int ret;
	qcsapi_mac_addr address_to_deny;

	if (!rpc_qtn_ready())
		return -1;

	ether_atoe(macaddr, address_to_deny);
	ret = qcsapi_wifi_deny_mac_address(ifname, address_to_deny);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_deny_mac_address %s error, return: %d\n", ifname, ret);
		return ret;
	}
	//      dbG("deny MAC addresss of interface %s: %s\n", ifname, macaddr);

	return 0;
}

int rpc_qcsapi_wds_set_psk(const char *ifname, const char *macaddr, const char *wpa_psk)
{
	int ret;
	qcsapi_mac_addr peer_address;

	if (!rpc_qtn_ready())
		return -1;

	ether_atoe(macaddr, peer_address);
	ret = qcsapi_wds_set_psk(WIFINAME, peer_address, wpa_psk);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wds_set_psk %s error, return: %d\n", ifname, ret);
		return ret;
	}
	dbG("remove WDS Peer of interface %s: %s\n", ifname, macaddr);

	return 0;
}

int rpc_qcsapi_get_SSID(const char *ifname, qcsapi_SSID *ssid)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_SSID(ifname, (char *)ssid);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_SSID %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_SSID_broadcast(const char *ifname, int *p_current_option)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_option(ifname, qcsapi_SSID_broadcast, p_current_option);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_option::SSID_broadcast %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_vht(qcsapi_unsigned_int *vht)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_vht(WIFINAME, vht);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_vht %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_bw(qcsapi_unsigned_int *p_bw)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_bw(WIFINAME, p_bw);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_bw %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_channel(qcsapi_unsigned_int *p_channel)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_channel(WIFINAME, p_channel);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_channel %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_snr(void)
{
	int ret;
	int snr;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_avg_snr(WIFINAME, &snr);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_avg_snr %s error, return: %d\n", WIFINAME, ret);
		return 0;
	}

	return snr;
}

int rpc_qcsapi_get_channel_list(string_1024 *list_of_channels)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_list_channels(WIFINAME, *list_of_channels);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_list_channels %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_beacon_type(const char *ifname, char *p_current_beacon)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_beacon_type(ifname, p_current_beacon);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_beacon_type %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_WPA_encryption_modes(const char *ifname, char *p_current_encryption_mode)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_WPA_encryption_modes(ifname, p_current_encryption_mode);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_WPA_encryption_modes %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_key_passphrase(const char *ifname, char *p_current_key_passphrase)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_key_passphrase(ifname, 0, p_current_key_passphrase);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_key_passphrase %s error, return: %d\n", ifname, ret);

		ret = qcsapi_wifi_get_pre_shared_key(ifname, 0, p_current_key_passphrase);
		if (ret < 0)
			dbG("Qcsapi qcsapi_wifi_get_pre_shared_key %s error, return: %d\n", ifname, ret);

		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_dtim(qcsapi_unsigned_int *p_dtim)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_dtim(WIFINAME, p_dtim);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_dtim %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_beacon_interval(qcsapi_unsigned_int *p_beacon_interval)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_beacon_interval(WIFINAME, p_beacon_interval);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_beacon_interval %s error, return: %d\n", WIFINAME, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_mac_address_filtering(const char *ifname, qcsapi_mac_address_filtering *p_mac_address_filtering)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_mac_address_filtering(ifname, p_mac_address_filtering);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_mac_address_filtering %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_authorized_mac_addresses(const char *ifname, char *list_mac_addresses, const unsigned int sizeof_list)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_authorized_mac_addresses(ifname, list_mac_addresses, sizeof_list);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_authorized_mac_addresses %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_get_denied_mac_addresses(const char *ifname, char *list_mac_addresses, const unsigned int sizeof_list)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_get_denied_mac_addresses(ifname, list_mac_addresses, sizeof_list);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_get_denied_mac_addresses %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_interface_get_mac_addr(const char *ifname, qcsapi_mac_addr *current_mac_addr)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_interface_get_mac_addr(ifname, (uint8_t *)current_mac_addr);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_interface_get_mac_addr %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_get_state(const char *ifname, char *wps_state, const qcsapi_unsigned_int max_len)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_get_state(ifname, wps_state, max_len);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_get_state %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wifi_disable_wps(const char *ifname, int disable_wps)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_disable_wps(ifname, disable_wps);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_disable_wps %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_get_ap_pin(const char *ifname, char *wps_pin, int force_regenerate)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_get_ap_pin(ifname, wps_pin, force_regenerate);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_get_ap_pin %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_get_configured_state(const char *ifname, char *wps_state, const qcsapi_unsigned_int max_len)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_get_configured_state(ifname, wps_state, max_len);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_get_configured_state %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_cancel(const char *ifname)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_cancel(ifname);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_cancel %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_registrar_report_button_press(const char *ifname)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_registrar_report_button_press(ifname);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_registrar_report_button_press %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_wps_registrar_report_pin(const char *ifname, const char *wps_pin)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wps_registrar_report_pin(ifname, wps_pin);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wps_registrar_report_pin %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_qcsapi_restore_default_config(int flag)
{
	int ret;

	if (!qtn_qcsapi_init) {
		ret = rpc_qcsapi_init(1);
		if (ret < 0) {
			dbG("[restore_default] rpc_qcsapi_init error, return: %d\n", ret);
			return -1;
		}
	}

	ret = qcsapi_restore_default_config(flag);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_restore_default_config error, return: %d\n", ret);
		return ret;
	}
	dbG("QTN restore default config successfully\n");

	qtn_qcsapi_init = 0;

	return 0;
}

int rpc_qcsapi_bootcfg_commit(void)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_bootcfg_commit();
	if (ret < 0) {
		dbG("Qcsapi qcsapi_bootcfg_commit error, return: %d\n", ret);
		return ret;
	}
	dbG("QTN commit bootcfg successfully\n");

	return 0;
}

void rpc_show_config(void)
{
	int ret;
	char mac_address_filtering_mode[][8] = { "Disable", "Reject", "Accept" };

	if (!rpc_qtn_ready())
		return;

	qcsapi_SSID ssid;
	ret = rpc_qcsapi_get_SSID(WIFINAME, &ssid);
	if (ret < 0)
		dbG("rpc_qcsapi_get_SSID %s error, return: %d\n", WIFINAME, ret);
	else
		dbG("current SSID of interface %s: %s\n", WIFINAME, ssid);

	int current_option;
	ret = rpc_qcsapi_get_SSID_broadcast(WIFINAME, &current_option);
	if (ret < 0)
		dbG("rpc_qcsapi_get_SSID_broadcast %s error, return: %d\n", WIFINAME, ret);
	else
		dbG("current SSID broadcast option of interface %s: %s\n", WIFINAME, current_option ? "TRUE" : "FALSE");

	qcsapi_unsigned_int vht;
	ret = rpc_qcsapi_get_vht(&vht);
	if (ret < 0)
		dbG("rpc_qcsapi_get_vht error, return: %d\n", ret);
	else
		dbG("current wireless mode: %s\n", (unsigned int)vht ? "11ac" : "11n");

	qcsapi_unsigned_int bw;
	ret = rpc_qcsapi_get_bw(&bw);
	if (ret < 0)
		dbG("rpc_qcsapi_get_bw error, return: %d\n", ret);
	else
		dbG("current channel bandwidth: %d MHz\n", bw);

	qcsapi_unsigned_int channel;
	ret = rpc_qcsapi_get_channel(&channel);
	if (ret < 0)
		dbG("rpc_qcsapi_get_channel error, return: %d\n", ret);
	else
		dbG("current channel: %d\n", channel);

	string_1024 list_of_channels;
	ret = rpc_qcsapi_get_channel_list(&list_of_channels);
	if (ret < 0)
		dbG("rpc_qcsapi_get_channel_list error, return: %d\n", ret);
	else
		dbG("current channel list: %s\n", list_of_channels);

	string_16 current_beacon_type;
	ret = rpc_qcsapi_get_beacon_type(WIFINAME, (char *)&current_beacon_type);
	if (ret < 0)
		dbG("rpc_qcsapi_get_beacon_type %s error, return: %d\n", WIFINAME, ret);
	else
		dbG("current beacon type of interface %s: %s\n", WIFINAME, current_beacon_type);

	string_32 encryption_mode;
	ret = rpc_qcsapi_get_WPA_encryption_modes(WIFINAME, (char *)&encryption_mode);
	if (ret < 0)
		dbG("rpc_qcsapi_get_WPA_encryption_modes %s error, return: %d\n", WIFINAME, ret);
	else
		dbG("current WPA encryption mode of interface %s: %s\n", WIFINAME, encryption_mode);

	string_64 key_passphrase;
	ret = rpc_qcsapi_get_key_passphrase(WIFINAME, (char *)&key_passphrase);
	if (ret < 0)
		dbG("rpc_qcsapi_get_key_passphrase %s error, return: %d\n", WIFINAME, ret);
	else
		dbG("current WPA preshared key of interface %s: %s\n", WIFINAME, key_passphrase);

	qcsapi_unsigned_int dtim;
	ret = rpc_qcsapi_get_dtim(&dtim);
	if (ret < 0)
		dbG("rpc_qcsapi_get_dtim error, return: %d\n", ret);
	else
		dbG("current DTIM interval: %d\n", dtim);

	qcsapi_unsigned_int beacon_interval;
	ret = rpc_qcsapi_get_beacon_interval(&beacon_interval);
	if (ret < 0)
		dbG("rpc_qcsapi_get_beacon_interval error, return: %d\n", ret);
	else
		dbG("current Beacon interval: %d\n", beacon_interval);

	qcsapi_mac_address_filtering mac_address_filtering;
	ret = rpc_qcsapi_get_mac_address_filtering(WIFINAME, &mac_address_filtering);
	if (ret < 0)
		dbG("rpc_qcsapi_get_mac_address_filtering %s error, return: %d\n", WIFINAME, ret);
	else
		dbG("current MAC filter mode of interface %s: %s\n", WIFINAME, mac_address_filtering_mode[mac_address_filtering]);
}

void rpc_set_radio(int unit, int subunit, int on)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	int ret;
	char interface_status = 0;
	qcsapi_mac_addr wl_macaddr;
	char macbuf[13], macaddr_str[18];
	unsigned long long macvalue;
	unsigned char *macp;

	if (subunit > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (subunit > 0) {
		ret = qcsapi_interface_get_status(wl_vifname_qtn(unit, subunit), &interface_status);
		//              if (ret < 0)
		//                      dbG("Qcsapi qcsapi_interface_get_status %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

		if (on) {
			if (interface_status) {
				dbG("vif %s has existed already\n", wl_vifname_qtn(unit, subunit));

				return;
			}

			bzero(&wl_macaddr, sizeof(wl_macaddr));
			ret = qcsapi_interface_get_mac_addr(WIFINAME, (uint8_t *)wl_macaddr);
			if (ret < 0)
				dbG("Qcsapi qcsapi_interface_get_mac_addr %s error, return: %d\n", WIFINAME, ret);

			sprintf(macbuf, "%02X%02X%02X%02X%02X%02X", wl_macaddr[0], wl_macaddr[1], wl_macaddr[2], wl_macaddr[3],
				wl_macaddr[4], wl_macaddr[5]);
			macvalue = strtoll(macbuf, (char **)NULL, 16);
			macvalue += subunit;
			macp = (unsigned char *)&macvalue;
			bzero(macaddr_str, sizeof(macaddr_str));
			sprintf(macaddr_str, "%02X:%02X:%02X:%02X:%02X:%02X", *(macp + 5), *(macp + 4), *(macp + 3), *(macp + 2),
				*(macp + 1), *(macp + 0));
			ether_atoe(macaddr_str, wl_macaddr);

			ret = qcsapi_wifi_create_bss(wl_vifname_qtn(unit, subunit), wl_macaddr);
			if (ret < 0) {
				dbG("Qcsapi qcsapi_wifi_create_bss %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

				return;
			}

			ret = rpc_qcsapi_set_SSID(wl_vifname_qtn(unit, subunit), nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_SSID %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			ret = rpc_qcsapi_set_SSID_broadcast(wl_vifname_qtn(unit, subunit),
							    nvram_safe_get(strcat_r(prefix, "closed", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_SSID_broadcast %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			ret = rpc_qcsapi_set_beacon_type(wl_vifname_qtn(unit, subunit),
							 nvram_safe_get(strcat_r(prefix, "akm", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_beacon_type %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			ret = rpc_qcsapi_set_WPA_encryption_modes(wl_vifname_qtn(unit, subunit),
								  nvram_safe_get(strcat_r(prefix, "crypto", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_WPA_encryption_modes %s error, return: %d\n", wl_vifname_qtn(unit, subunit),
				    ret);

			ret = rpc_qcsapi_set_key_passphrase(wl_vifname_qtn(unit, subunit),
							    nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_key_passphrase %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
		} else {
			ret = qcsapi_wifi_remove_bss(wl_vifname_qtn(unit, subunit));
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_remove_bss %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
		}
	} else {
		ret = qcsapi_wifi_rfenable(WIFINAME, (qcsapi_unsigned_int)on);
		if (ret < 0)
			dbG("Qcsapi qcsapi_wifi_rfenable %s, return: %d\n", WIFINAME, ret);
	}
}

int rpc_update_ap_isolate(const char *ifname, const int isolate)
{
	int ret;

	if (!rpc_qtn_ready())
		return -1;

	ret = qcsapi_wifi_set_ap_isolate(ifname, isolate);
	if (ret < 0) {
		dbG("Qcsapi qcsapi_wifi_set_ap_isolate %s error, return: %d\n", ifname, ret);
		return ret;
	}

	return 0;
}

int rpc_get_temperature(void)
{
	int statval = 0;
	int qcsapi_retval;
	int temp_rficinternal, temp_rficexternal, temp_internal;

	if (!rpc_qtn_ready())
		return 0;
	qcsapi_retval = qcsapi_get_temperature_info(&temp_rficinternal, &temp_rficexternal, &temp_internal);
	if (qcsapi_retval >= 0) {
		return temp_internal;
	}
	return 0;
}

void rpc_parse_nvram(const char *name, const char *value)
{
	if (!rpc_qtn_ready())
		return;

	if (!strcmp(name, "wl1_ssid"))
		rpc_qcsapi_set_SSID(WIFINAME, value);
	else if (!strcmp(name, "wl1_closed"))
		rpc_qcsapi_set_SSID_broadcast(WIFINAME, value);
	else if (!strcmp(name, "wl1_nmode_x"))
		rpc_qcsapi_set_vht(value);
	else if (!strcmp(name, "wl1_nbw"))
		rpc_qcsapi_set_bw(value);
	else if (!strcmp(name, "wl1_channel"))
		rpc_qcsapi_set_channel(value);
	else if (!strcmp(name, "wl1_akm"))
		rpc_qcsapi_set_beacon_type(WIFINAME, value);
	else if (!strcmp(name, "wl1_crypto"))
		rpc_qcsapi_set_WPA_encryption_modes(WIFINAME, value);
	else if (!strcmp(name, "wl1_wpa_psk"))
		rpc_qcsapi_set_key_passphrase(WIFINAME, value);
	else if (!strcmp(name, "wl1_dtim"))
		rpc_qcsapi_set_dtim(value);
	else if (!strcmp(name, "wl1_bcn"))
		rpc_qcsapi_set_beacon_interval(value);
	else if (!strcmp(name, "wl1_radio"))
		rpc_set_radio(1, 0, nvram_get_int(name));
	else if (!strcmp(name, "wl1_macmode"))
		rpc_update_macmode(value);
	else if (!strcmp(name, "wl1_maclist_x"))
		rpc_update_wlmaclist();
	else if (!strcmp(name, "wl1_mode_x"))
		rpc_update_wdslist();
	else if (!strcmp(name, "wl1_wdslist"))
		rpc_update_wdslist();
	else if (!strcmp(name, "wl1_wds_psk"))
		rpc_update_wds_psk(value);
	else if (!strcmp(name, "wl1_ap_isolate"))
		rpc_update_ap_isolate(WIFINAME, atoi(value));
	else if (!strncmp(name, "wl1.", 4))
		rpc_update_mbss(name, value);

	//      rpc_show_config();
}

int rpc_qcsapi_set_wlmaclist(const char *ifname)
{
	int ret;
	qcsapi_mac_address_filtering mac_address_filtering;
	char list_mac_addresses[1024];
	char *m = NULL;
	char *p, *pp;

	if (!rpc_qtn_ready())
		return -1;

	ret = rpc_qcsapi_get_mac_address_filtering(ifname, &mac_address_filtering);
	if (ret < 0) {
		dbG("rpc_qcsapi_get_mac_address_filtering %s error, return: %d\n", ifname, ret);
		return ret;
	} else {
		if (mac_address_filtering == qcsapi_accept_mac_address_unless_denied) {
			ret = qcsapi_wifi_clear_mac_address_filters(ifname);
			if (ret < 0) {
				dbG("Qcsapi qcsapi_wifi_clear_mac_address_filters %s, error, return: %d\n", ifname, ret);
				return ret;
			}

			pp = p = strdup(nvram_safe_get("wl1_maclist_x"));
			if (pp) {
				while ((m = strsep(&p, "<")) != NULL) {
					if (!*m)
						continue;
					ret = rpc_qcsapi_deny_mac_address(ifname, m);
					if (ret < 0)
						dbG("rpc_qcsapi_deny_mac_address %s error, return: %d\n", ifname, ret);
				}
				free(pp);
			}

			ret = rpc_qcsapi_get_denied_mac_addresses(ifname, list_mac_addresses, sizeof(list_mac_addresses));
			if (ret < 0)
				dbG("rpc_qcsapi_get_denied_mac_addresses %s error, return: %d\n", ifname, ret);
			else
				dbG("current denied MAC addresses of interface %s: %s\n", ifname, list_mac_addresses);
		} else if (mac_address_filtering == qcsapi_deny_mac_address_unless_authorized) {
			ret = qcsapi_wifi_clear_mac_address_filters(ifname);
			if (ret < 0) {
				dbG("Qcsapi qcsapi_wifi_clear_mac_address_filters %s error, return: %d\n", ifname, ret);
				return ret;
			}

			pp = p = strdup(nvram_safe_get("wl1_maclist_x"));
			if (pp) {
				while ((m = strsep(&p, "<")) != NULL) {
					if (!*m)
						continue;
					ret = rpc_qcsapi_authorize_mac_address(ifname, m);
					if (ret < 0)
						dbG("rpc_qcsapi_authorize_mac_address %s error, return: %d\n", ifname, ret);
				}
				free(pp);
			}

			ret = rpc_qcsapi_get_authorized_mac_addresses(ifname, list_mac_addresses, sizeof(list_mac_addresses));
			if (ret < 0)
				dbG("rpc_qcsapi_get_authorized_mac_addresses %s error, return: %d\n", ifname, ret);
			else
				dbG("current authorized MAC addresses of interface %s: %s\n", ifname, list_mac_addresses);
		}
	}

	return ret;
}

void rpc_update_wlmaclist(void)
{
	int ret;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	int i, unit = 1;

	if (!rpc_qtn_ready())
		return;

	ret = rpc_qcsapi_set_wlmaclist(WIFINAME);
	if (ret < 0)
		dbG("rpc_qcsapi_set_wlmaclist %s error, return: %d\n", WIFINAME, ret);

	char *next;
	char var[80];
	char *vifs = nvram_safe_get("wl1_vifs");
	foreach(var, vifs, next)
	{
		ret = rpc_qcsapi_set_wlmaclist(wl_vifname_qtn(unit, i));
		if (ret < 0)
			dbG("rpc_qcsapi_set_wlmaclist %s error, return: %d\n", wl_vifname_qtn(unit, i), ret);
	}
}

void rpc_update_wdslist()
{
	int ret, i;
	qcsapi_mac_addr peer_address;
	char *m = NULL;
	char *p, *pp;

	if (!rpc_qtn_ready())
		return;

	for (i = 0; i < 8; i++) {
		ret = qcsapi_wds_get_peer_address(WIFINAME, 0, (uint8_t *)&peer_address);
		if (ret < 0) {
			if (ret == -19)
				break; // No such device
			dbG("Qcsapi qcsapi_wds_get_peer_address %s error, return: %d\n", WIFINAME, ret);
		} else {
			//                      dbG("current WDS peer index 0 addresse: %s\n", wl_ether_etoa((struct ether_addr *) &peer_address));
			ret = qcsapi_wds_remove_peer(WIFINAME, peer_address);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wds_remove_peer %s error, return: %d\n", WIFINAME, ret);
		}
	}

	if (nvram_matchi("wl1_mode_x", 0))
		return;

	pp = p = strdup(nvram_safe_get("wl1_wdslist"));
	if (pp) {
		while ((m = strsep(&p, "<")) != NULL) {
			if (!*(m))
				continue;

			ether_atoe(m, peer_address);
			ret = qcsapi_wds_add_peer(WIFINAME, peer_address);
			if (ret < 0)
				dbG("Qcsapi qcsapi_wds_add_peer %s error, return: %d\n", WIFINAME, ret);
			else {
				ret = rpc_qcsapi_wds_set_psk(WIFINAME, m, nvram_safe_get("wl1_wds_psk"));
				if (ret < 0)
					dbG("Qcsapi rpc_qcsapi_wds_set_psk %s error, return: %d\n", WIFINAME, ret);
			}
		}
		free(pp);
	}

	for (i = 0; i < 8; i++) {
		char tmp[20];
		ret = qcsapi_wds_get_peer_address(WIFINAME, i, (uint8_t *)&peer_address);
		if (ret < 0) {
			if (ret == -19)
				break; // No such device
			dbG("Qcsapi qcsapi_wds_get_peer_address %s error, return: %d\n", WIFINAME, ret);
		} else
			dbG("current WDS peer index 0 addresse: %s\n", ether_etoa((struct ether_addr *)&peer_address, tmp));
	}
}

void rpc_update_wds_psk(const char *wds_psk)
{
	int ret, i;
	qcsapi_mac_addr peer_address;

	if (!rpc_qtn_ready())
		return;

	if (nvram_matchi("wl1_mode_x", 0))
		return;

	for (i = 0; i < 8; i++) {
		ret = qcsapi_wds_get_peer_address(WIFINAME, i, (uint8_t *)&peer_address);
		if (ret < 0) {
			if (ret == -19)
				break; // No such device
			dbG("Qcsapi qcsapi_wds_get_peer_address %s error, return: %d\n", WIFINAME, ret);
		} else {
			char tmp[20];
			//                      dbG("current WDS peer index 0 addresse: %s\n", wl_ether_etoa((struct ether_addr *) &peer_address));
			ret = rpc_qcsapi_wds_set_psk(WIFINAME, ether_etoa((struct ether_addr *)&peer_address, tmp), wds_psk);
			if (ret < 0)
				dbG("rpc_qcsapi_wds_set_psk %s error, return: %d\n", WIFINAME, ret);
		}
	}
}

#define SET_SSID 0x01
#define SET_CLOSED 0x02
#define SET_AUTH 0x04
#define SET_CRYPTO 0x08
#define SET_WPAPSK 0x10
#define SET_MACMODE 0x20
#define SET_ALL 0x3F

static void rpc_reload_mbss(int unit, int subunit, const char *name_mbss)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	unsigned char set_type = 0;
	int ret;
	char *auth_mode;

	if (!rpc_qtn_ready())
		return;

	if (subunit > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (!strcmp(name_mbss, "ssid"))
		set_type = SET_SSID;
	else if (!strcmp(name_mbss, "closed"))
		set_type = SET_CLOSED;
	else if (!strcmp(name_mbss, "akm"))
		set_type = SET_AUTH;
	else if (!strcmp(name_mbss, "crypto"))
		set_type = SET_CRYPTO;
	else if (!strcmp(name_mbss, "wpa_psk"))
		set_type = SET_WPAPSK;
	else if (!strcmp(name_mbss, "macmode"))
		set_type = SET_MACMODE;
	else if (!strcmp(name_mbss, "all"))
		set_type = SET_ALL;

	if (set_type & SET_SSID) {
		ret = rpc_qcsapi_set_SSID(wl_vifname_qtn(unit, subunit), nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
		if (ret < 0)
			dbG("rpc_qcsapi_set_SSID %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
	}

	if (set_type & SET_CLOSED) {
		ret = rpc_qcsapi_set_SSID_broadcast(wl_vifname_qtn(unit, subunit), nvram_safe_get(strcat_r(prefix, "closed", tmp)));
		if (ret < 0)
			dbG("rpc_qcsapi_set_SSID_broadcast %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
	}

	if (set_type & SET_AUTH) {
		ret = rpc_qcsapi_set_beacon_type(wl_vifname_qtn(unit, subunit), nvram_safe_get(strcat_r(prefix, "akm", tmp)));
		if (ret < 0)
			dbG("rpc_qcsapi_set_beacon_type %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
	}

	if (set_type & SET_CRYPTO) {
		auth_mode = nvram_safe_get(strcat_r(prefix, "akm", tmp));
		if (!strcmp(auth_mode, "psk") || !strcmp(auth_mode, "psk2") || !strcmp(auth_mode, "pskpsk2")) {
			ret = rpc_qcsapi_set_WPA_encryption_modes(wl_vifname_qtn(unit, subunit),
								  nvram_safe_get(strcat_r(prefix, "crypto", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_WPA_encryption_modes %s error, return: %d\n", wl_vifname_qtn(unit, subunit),
				    ret);
		}
	}

	if (set_type & SET_WPAPSK) {
		auth_mode = nvram_safe_get(strcat_r(prefix, "akm", tmp));
		if (!strcmp(auth_mode, "psk") || !strcmp(auth_mode, "psk2") || !strcmp(auth_mode, "pskpsk2")) {
			ret = rpc_qcsapi_set_key_passphrase(wl_vifname_qtn(unit, subunit),
							    nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));
			if (ret < 0)
				dbG("rpc_qcsapi_set_key_passphrase %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
		}
	}

	if (set_type & SET_MACMODE) {
		ret = rpc_qcsapi_set_mac_address_filtering(wl_vifname_qtn(unit, subunit),
							   nvram_safe_get(strcat_r(prefix, "macmode", tmp)));
		if (ret < 0) {
			dbG("rpc_qcsapi_set_mac_address_filtering %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			return;
		} else
			rpc_qcsapi_set_wlmaclist(wl_vifname_qtn(unit, subunit));
	}
}

void rpc_update_mbss(const char *name, const char *value)
{
	int ret, unit, subunit;
	char name_mbss[32];
	char interface_status = 0;
	qcsapi_mac_addr wl_macaddr;
	char macbuf[13], macaddr_str[18];
	unsigned long long macvalue;
	unsigned char *macp;

	//        if (nvram_get_int("sw_mode") == SW_MODE_REPEATER && nvram_get_int("wlc_band"))
	//                return;

	if (!rpc_qtn_ready())
		return;

	if (sscanf(name, "wl%d.%d_%s", &unit, &subunit, name_mbss) != 3)
		return;

	if ((subunit < 1) || (subunit > 3))
		return;

	ret = qcsapi_interface_get_status(wl_vifname_qtn(unit, subunit), &interface_status);
	//      if (ret < 0)
	//              dbG("qcsapi_interface_get_status %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
	fprintf(stderr, "update %s = %s\n", name, name_mbss);
	if (!strcmp(name_mbss, "bss_enabled")) {
		if (value && !strcmp(value, "1")) {
			if (interface_status) {
				dbG("vif %s has existed already\n", wl_vifname_qtn(unit, subunit));

				return;
			}

			bzero(&wl_macaddr, sizeof(wl_macaddr));
			ret = qcsapi_interface_get_mac_addr(WIFINAME, (uint8_t *)wl_macaddr);
			if (ret < 0)
				dbG("Qcsapi qcsapi_interface_get_mac_addr %s error, return: %d\n", WIFINAME, ret);

			sprintf(macbuf, "%02X%02X%02X%02X%02X%02X", wl_macaddr[0], wl_macaddr[1], wl_macaddr[2], wl_macaddr[3],
				wl_macaddr[4], wl_macaddr[5]);
			macvalue = strtoll(macbuf, (char **)NULL, 16);
			macvalue += subunit;
			macp = (unsigned char *)&macvalue;
			bzero(macaddr_str, sizeof(macaddr_str));
			sprintf(macaddr_str, "%02X:%02X:%02X:%02X:%02X:%02X", *(macp + 5), *(macp + 4), *(macp + 3), *(macp + 2),
				*(macp + 1), *(macp + 0));
			ether_atoe(macaddr_str, wl_macaddr);

			ret = qcsapi_wifi_create_bss(wl_vifname_qtn(unit, subunit), wl_macaddr);
			if (ret < 0) {
				dbG("Qcsapi qcsapi_wifi_create_bss %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

				return;
			} else
				dbG("Qcsapi qcsapi_wifi_create_bss %s successfully\n", wl_vifname_qtn(unit, subunit));

			rpc_reload_mbss(unit, subunit, "all");
		} else {
			ret = qcsapi_wifi_remove_bss(wl_vifname_qtn(unit, subunit));
			if (ret < 0)
				dbG("Qcsapi qcsapi_wifi_remove_bss %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);
			else
				dbG("Qcsapi qcsapi_wifi_remove_bss %s successfully\n", wl_vifname_qtn(unit, subunit));
		}
	} else
		rpc_reload_mbss(unit, subunit, name_mbss);
}

char *getWscStatusStr_qtn()
{
	int ret, state = -1;
	char wps_state[32], state_str[32];

	if (!rpc_qtn_ready())
		return "5 GHz radio is not ready";

	ret = rpc_qcsapi_wps_get_state(WIFINAME, wps_state, sizeof(wps_state));
	if (ret < 0)
		dbG("rpc_qcsapi_wps_get_state %s error, return: %d\n", WIFINAME, ret);
	else if (sscanf(wps_state, "%d %s", &state, state_str) != 2)
		dbG("prase wps state error!\n");

	switch (state) {
	case 0: /* WPS_INITIAL */
		return "initialization";
		break;
	case 1: /* WPS_START */
		return "Start WPS Process";
		break;
	case 2: /* WPS_SUCCESS */
		return "Success";
		break;
	case 3: /* WPS_ERROR */
		return "Fail due to WPS message exchange error!";
		break;
	case 4: /* WPS_TIMEOUT */
		return "Fail due to WPS time out!";
		break;
	case 5: /* WPS_OVERLAP */
		return "Fail due to PBC session overlap!";
		break;
	default:
		if (nvram_matchi("wps_enable", 1))
			return "Idle";
		else
			return "Not used";
		break;
	}
}

char *get_WPSConfiguredStr_qtn()
{
	int ret;
	char wps_configured_state[32];

	if (!rpc_qtn_ready())
		return "";

	wps_configured_state[0] = 0;
	ret = rpc_qcsapi_wps_get_configured_state(WIFINAME, wps_configured_state, sizeof(wps_configured_state));
	if (ret < 0)
		dbG("rpc_qcsapi_wps_get_configured_state %s error, return: %d\n", WIFINAME, ret);

	if (!strcmp(wps_configured_state, "configured"))
		return "Yes";
	else
		return "No";
}

char *getWPSAuthMode_qtn()
{
	string_16 current_beacon_type;
	int ret;

	if (!rpc_qtn_ready())
		return "";

	bzero(&current_beacon_type, sizeof(current_beacon_type));
	ret = rpc_qcsapi_get_beacon_type(WIFINAME, (char *)&current_beacon_type);
	if (ret < 0)
		dbG("rpc_qcsapi_get_beacon_type %s error, return: %d\n", WIFINAME, ret);

	if (!strcmp(current_beacon_type, "Basic"))
		return "Open System";
	else if (!strcmp(current_beacon_type, "WPA"))
		return "WPA-Personal";
	else if (!strcmp(current_beacon_type, "11i"))
		return "WPA2-Personal";
	else if (!strcmp(current_beacon_type, "WPAand11i"))
		return "WPA-Auto-Personal";
	else
		return "Open System";
}

char *getWPSEncrypType_qtn()
{
	string_32 encryption_mode;
	int ret;

	if (!rpc_qtn_ready())
		return "";

	bzero(&encryption_mode, sizeof(encryption_mode));
	ret = rpc_qcsapi_get_WPA_encryption_modes(WIFINAME, (char *)&encryption_mode);
	if (ret < 0)
		dbG("rpc_qcsapi_get_WPA_encryption_modes %s error, return: %d\n", WIFINAME, ret);

	if (!strcmp(encryption_mode, "TKIPEncryption"))
		return "TKIP";
	else if (!strcmp(encryption_mode, "AESEncryption"))
		return "AES";
	else if (!strcmp(encryption_mode, "TKIPandAESEncryption"))
		return "TKIP+AES";
	else
		return "AES";
}

int getassoclist_qtn(const char *name, unsigned char *list)
{
	qcsapi_unsigned_int count;
	qcsapi_mac_addr the_mac_addr;
	int ret, i;
	if (!rpc_qtn_ready())
		return -1;
	ret = qcsapi_wifi_get_count_associations(WIFINAME, &count);
	if (ret < 0) {
		qtn_init = 0;
		return 0;
	}
	unsigned int *cnt = (unsigned int *)list;
	unsigned char *maclist = &list[4];
	cnt[0] = 0;
	for (i = 0; i < count; i++) {
		ret = qcsapi_wifi_get_associated_device_mac_addr(WIFINAME, i, the_mac_addr);
		if (ret < 0)
			break;
		cnt[0]++;
		memcpy(maclist, the_mac_addr, 6);
		maclist += 6;
	}
	return cnt[0];
}

int getRssiIndex_qtn(const char *name, int index)
{
	unsigned int rssi;
	if (!rpc_qtn_ready())
		return -1;
	int ret = qcsapi_wifi_get_rssi_per_association(WIFINAME, index, &rssi);
	if (ret < 0) {
		qtn_init = 0;
		return 0;
	}
	return -rssi;
}

int getNoiseIndex_qtn(const char *name, int index)
{
	int noise;
	if (!rpc_qtn_ready())
		return -1;
	int ret = qcsapi_wifi_get_hw_noise_per_association(WIFINAME, index, &noise);
	if (ret < 0) {
		qtn_init = 0;
		return 0;
	}
	return noise / 10;
}

int getTXRate_qtn(const char *name, int index)
{
	int rate;
	if (!rpc_qtn_ready())
		return -1;
	qcsapi_wifi_get_tx_phy_rate_per_association(WIFINAME, index, &rate);
	return rate;
}

int getRXRate_qtn(const char *name, int index)
{
	int rate;
	if (!rpc_qtn_ready())
		return -1;
	qcsapi_wifi_get_rx_phy_rate_per_association(WIFINAME, index, &rate);
	return rate;
}
