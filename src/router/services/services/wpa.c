/*
 * wpa.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_NAS
#ifndef HAVE_MADWIFI
#ifndef HAVE_RT2880
#ifndef HAVE_RT61
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <nvparse.h>
#include "snmp.h"
#include <signal.h>
#include <utils.h>
#include <syslog.h>
#include <wlutils.h>
#include <services.h>

void run_nas_notify(char *ifname)
{
	char *argv[] = { "nas4not", "lan", ifname, "up", NULL, /* role */
			 NULL, /* crypto */
			 NULL, /* auth */
			 NULL, /* passphrase */
			 NULL, /* ssid */
			 NULL };
	char *str = NULL;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_", pidfile[] = "/tmp/nas.wlXXXXXXXlan.pid";
	int unit;
	char remote[ETHER_ADDR_LEN];
	char ssid[48], pass[80], auth[16], crypto[16], role[8];
	int i;

	/*
	 * the wireless interface must be configured to run NAS 
	 */
	unit = get_wl_instance(ifname);
	if (unit == -1)
		return;
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	snprintf(pidfile, sizeof(pidfile), "/tmp/nas.wl%dlan.pid", unit);

	if (!(str = file2str(pidfile))) // no pidfile means no nas was run (required)
	{
		return;
	}
	free(str);
	sleep(3);
	/*
	 * find WDS link configuration 
	 */
	wl_ioctl(ifname, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN);
	for (i = 0; i < MAX_NVPARSE; i++) {
		char mac[ETHER_ADDR_STR_LEN];
		uint8 ea[ETHER_ADDR_LEN];

		if (get_wds_wsec(unit, i, mac, role, crypto, auth, ssid, pass) && ether_atoe(mac, ea) &&
		    !bcmp(ea, remote, ETHER_ADDR_LEN)) {
			argv[4] = role;
			argv[5] = crypto;
			argv[6] = auth;
			argv[7] = pass;
			argv[8] = ssid;
			break;
		}
	}

	/*
	 * did not find WDS link configuration, use wireless' 
	 */
	if (i == MAX_NVPARSE) {
		/*
		 * role 
		 */
		argv[4] = "auto";
		/*
		 * crypto 
		 */
		argv[5] = nvram_safe_get(strcat_r(prefix, "crypto", tmp));
		/*
		 * auth mode 
		 */
		argv[6] = nvram_safe_get(strcat_r(prefix, "akm", tmp));
		/*
		 * passphrase 
		 */
		argv[7] = nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
		/*
		 * ssid 
		 */
		argv[8] = nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	}
	int pid;

	_log_evalpid(argv, ">/dev/console", 0, &pid);
}

static void start_radius(char *prefix)
{
	// wrt-radauth $IFNAME $server $port $share $override $mackey $maxun &

	if (nvram_nmatch("1", "%s_radauth", prefix) && nvram_nmatch("ap", "%s_mode", prefix)) {
		char *server = nvram_nget("%s_radius_ipaddr", prefix);
		char *port = nvram_nget("%s_radius_port", prefix);
		char *share = nvram_nget("%s_radius_key", prefix);
		char *ifname = nvram_nget("%s_ifname", prefix);
		char type[32];

		sprintf(type, "%s_radmactype", prefix);
		char *pragma = "";

		if (nvram_default_matchi(type, 0, 0))
			pragma = "-n1 ";
		if (nvram_matchi(type, 1))
			pragma = "-n2 ";
		if (nvram_matchi(type, 2))
			pragma = "-n3 ";
		if (nvram_matchi(type, 3))
			pragma = "";
		sleep(1); // some delay is usefull
		sysprintf("wrt-radauth %s %s %s %s %s %s %s %s &", pragma, ifname, server, port, share,
			  nvram_nget("%s_radius_override", prefix), nvram_nget("%s_radmacpassword", prefix),
			  nvram_nget("%s_max_unauth_users", prefix));
	}
}

static void start_nas_single(char *type, char *prefix);

static void convert_wds(int instance)
{
	char wds_mac[254];
	char buf[254];

	if (nvram_nmatch("", "wl%d_wds", instance)) // For Router, accept
		// all WDS link
		strcpy(wds_mac, "*");
	else // For AP, assign remote WDS MAC
		strcpy(wds_mac, nvram_nget("wl%d_wds", instance));

	/*
	 * For WPA-PSK mode, we want to convert wl_wds_mac to wl0_wds0 ...
	 * wl0_wds255 
	 */
	if (nvram_nmatch("psk", "wl%d_security_mode", instance) || nvram_nmatch("psk2", "wl%d_security_mode", instance)) {
		int i = 0;
		int j;
		char mac[254];
		char *next;

		foreach(mac, wds_mac, next)
		{
			snprintf(buf, sizeof(buf), "%s,auto,%s,%s,%s,%s", mac, nvram_nget("wl%d_crypto", instance),
				 nvram_nget("wl%d_security_mode", instance), nvram_nget("wl%d_ssid", instance),
				 nvram_nget("wl%d_wpa_psk", instance));
			nvram_nset(buf, "wl%d_wds%d", instance, i);
			i++;
		}

		/*
		 * Del unused entry 
		 */
		for (j = i; j < MAX_NVPARSE; j++)
			del_wds_wsec(instance, j);
	}
}

static char *getSecMode(char *prefix)
{
	char wep[32];
	char crypto[32];

	sprintf(wep, "%s_wep", prefix);
	sprintf(crypto, "%s_crypto", prefix);
	/*
	 * BugBug - should we bail when mode is wep ? 
	 */
	if (nvram_match(wep, "wep") || nvram_match(wep, "on") || nvram_match(wep, "restricted") || nvram_match(wep, "enabled"))
		return "1";
	else if (nvram_match(crypto, "tkip"))
		return "2";
	else if (nvram_match(crypto, "aes"))
		return "4";
	else if (nvram_match(crypto, "tkip+aes"))
		return "6";
	else
		return "0";
}

static char *getAuthMode(char *prefix)
{
	char akm[32];

	sprintf(akm, "%s_akm", prefix);
	if (!*(nvram_safe_get(akm)) || nvram_match(akm, "disabled") || nvram_match(akm, "wep"))
		return NULL;
	if (nvram_match(akm, "radius"))
		return "32";
	else if (nvram_match(akm, "wpa"))
		return "2";
	else if (nvram_match(akm, "psk"))
		return "4";
	else if (nvram_match(akm, "psk2"))
		return "128";
	else if (nvram_match(akm, "psk psk2"))
		return "132";
	else if (nvram_match(akm, "wpa2"))
		return "64";
	else if (nvram_match(akm, "wpa wpa2"))
		return "66";
	else
		return "255";
}

static char *getKey(char *prefix)
{
	char akm[32];

	sprintf(akm, "%s_akm", prefix);
	if (nvhas(akm, "wpa") || nvhas(akm, "wpa2") || nvhas(akm, "radius")) {
		return nvram_nget("%s_radius_key", prefix);
	} else if (nvhas(akm, "psk") || nvhas(akm, "psk2")) {
		return nvram_nget("%s_wpa_psk", prefix);
	} else
		return "";
}

/*
 * static void start_nas_ap(char *prefix,char *type) { char sec[32];
 * sprintf(sec,"%s_security_mode",prefix); int i; for (i=0;i<strlen(sec);i++)
 * if (sec[i]=='.')sec[i]='X';
 * 
 * char *security_mode = nvram_safe_get (sec);
 * 
 * if (strstr (security_mode, "psk") || strstr (security_mode, "wpa")) { char
 * auth[32]; sprintf(auth,"%s_auth",prefix); nvram_set (auth, "0"); }
 * convert_wds ();
 * 
 * if (!type || !*type) { if (nvram_match ("wl0_mode", "ap")) type = "lan";
 * else type = "wan"; }
 * 
 * snprintf (cfgfile, sizeof (cfgfile), "/tmp/nas.%s.conf", type); snprintf
 * (pidfile, sizeof (pidfile), "/tmp/nas.%s.pid", type);
 * 
 * { char *argv[] = { "/usr/sbin/nas", cfgfile, pidfile, type, NULL }; pid_t
 * pid;
 * 
 * _eval (argv, NULL, 0, &pid); cprintf ("done\n"); } } 
 */
static void start_nas_lan(int c)
{
	char wlname[32];
	char *next;
	char var[80];
	char vifname[32];

	sprintf(wlname, "wl%d", c);
	start_radius(wlname); // quick fix, should be vif capable in future
	dd_loginfo("nas", "start nas for %s", wlname);
	start_nas_single("lan", wlname);

	sprintf(vifname, "wl%d_vifs", c);
	char *vifs = nvram_safe_get(vifname);

	foreach(var, vifs, next)
	{
		dd_loginfo("nas", "start nas for %s", var);
		start_nas_single("lan", var);
	}
}

static void start_nas_wan(int c)
{
	char wlname[32];

	sprintf(wlname, "wl%d", c);
	start_nas_single("wan", wlname);

	char *next;
	char var[80];
	char vif[16];
	char *vifs = nvram_nget("wl%d_vifs", c);

	foreach(var, vifs, next)
	{
		sprintf(vif, "%s_mode", var);
		if (nvram_match(vif, "sta") || nvram_match(vif, "wet") || nvram_match(vif, "apsta") ||
		    nvram_match(vif, "apstawet")) {
			start_nas_single("wan", var);
		} else {
			start_nas_single("lan", var);
		}
	}
}

static void stop_nas_process(void)
{
	int ret = 0;
	char name[80], *next;

	unlink("/tmp/.nas");

	led_control(LED_SEC, LED_OFF);
	led_control(LED_SEC0, LED_OFF);
	led_control(LED_SEC1, LED_OFF);

	stop_process("nas", "daemon");
	stop_process("wrt-radauth", "radauth daemon");

#ifdef HAVE_WPA_SUPPLICANT
	killall("wpa_supplicant", SIGKILL);
#endif
	int cnt = get_wl_instances();
	int c;
	char vifs_name[32];

	for (c = 0; c < cnt; c++) {
		char pidname[32];
		sprintf(pidname, "/tmp/nas.wl%dlan.pid", c);
		unlink(pidname);
		sprintf(pidname, "/tmp/nas.wl%dwan.pid", c);
		unlink(pidname);
		sprintf(vifs_name, "wl%d_vifs", c);
		char *vifs = nvram_safe_get(vifs_name);
		foreach(name, vifs, next)
		{
			sprintf(pidname, "/tmp/nas.%slan.pid", name);
			unlink(pidname);
		}
	}

	cprintf("done\n");
	return;
}

#ifdef HAVE_WPA_SUPPLICANT
extern void setupSupplicant(char *prefix);
#endif
void start_nas(void)
{
	unlink("/tmp/.nas");
	FILE *check = fopen("/tmp/.startnas", "rb");
	if (check) {
		fclose(check);
		return;
	}
	check = fopen("/tmp/.startnas", "wb");
	putc('f', check);
	fclose(check);
	sleep(1);
	char *iface;
	network_delay("nas");
	int cnt = get_wl_instances();
	int c;
	int deadcount;
	int radiostate = -1;
	stop_nas_process(); // ensure that no nas is running
#ifdef HAVE_QTN
	cnt = 1;
#endif
	for (c = 0; c < cnt; c++) {
		if (nvram_nmatch("disabled", "wl%d_net_mode", c))
			continue;

		for (deadcount = 0; deadcount < 5; deadcount++) {
			wl_ioctl(get_wl_instance_name(c), WLC_GET_RADIO, &radiostate, sizeof(int));
			if ((radiostate & WL_RADIO_SW_DISABLE) == 0) //radio turned on - ready
				break;
			sleep(1);
		}

		if ((radiostate & WL_RADIO_SW_DISABLE) != 0) { // radio turned off
			fprintf(stderr, "Radio: %d currently turned off\n", c);
			continue;
		}

		char wlname[32];

		sprintf(wlname, "wl%d", c);
		if (nvram_nmatch("sta", "wl%d_mode", c) || nvram_nmatch("wet", "wl%d_mode", c) ||
		    nvram_nmatch("apsta", "wl%d_mode", c) || nvram_nmatch("apstawet", "wl%d_mode", c)) {
			cprintf("start nas wan\n");
#ifdef HAVE_WPA_SUPPLICANT
			if (nvram_nmatch("8021X", "wl%d_akm", c) && nvram_nmatch("sta", "wl%d_mode", c))
				setupSupplicant(wlname);
			else
#endif
				start_nas_wan(c);

		} else {
			dd_loginfo("nas", "start nas lan");
			start_nas_lan(c);

			int s;

			for (s = 1; s <= MAX_WDS_DEVS; s++) {
				char *dev;

				if (nvram_nmatch("0", "wl%d_wds%d_enable", c, s))
					continue;

				dev = nvram_nget("wl%d_wds%d_if", c, s);

				run_nas_notify(dev);
			}
		}
	}
	//      iface = get_wl_instance_name(c);
	//      wlconf_up(iface);       // double tip
	//      }
	unlink("/tmp/.startnas");
	check = fopen("/tmp/.startmon", "wb");
	putc('f', check);
	fclose(check);
	return;
}

static void start_nas_single(char *type, char *prefix)
{
	FILE *fnas;
	char pidfile[64];
	char *auth_mode = "255"; /* -m N = WPA authorization mode (N = 0:
					 * none, 1: 802.1x, 2: WPA PSK, 255:
					 * disabled) */
	char *sec_mode = { 0 }; /* -w N = security mode bitmask (N = 1: WEP,
				 * 2: TKIP, 4: AES) */
	char *key = { 0 }, *iface = { 0 }, *mode = { 0 };

	if (!strcmp(prefix, "wl0")) {
		led_control(LED_SEC0, LED_OFF);
		convert_wds(0);
	}
	if (!strcmp(prefix, "wl1")) {
		led_control(LED_SEC1, LED_OFF);
		convert_wds(1);
	}
	if (!strcmp(prefix, "wl2")) {
		convert_wds(2);
	}

	snprintf(pidfile, sizeof(pidfile), "/tmp/nas.%s%s.pid", prefix, type);

	char apmode[32];

	sprintf(apmode, "%s_mode", prefix);
	if (!strcmp(type, "wan") && nvram_match(apmode, "ap")) {
		dd_loginfo("nas", "type is wan but if is ap, ignore nas");
		return;
	}
	// if (!strcmp (type, "lan"))
	// iface = "br0";
	// else

	if (0 == type || 0 == *type)
		type = "lan";
	if (!strcmp(type, "lan") && nvram_invmatch(apmode, "ap"))
		iface = "br0";
	else {
		if (!strcmp(prefix, "wl0")) {
			iface = get_wl_instance_name(0);
		} else if (!strcmp(prefix, "wl1")) {
			iface = get_wl_instance_name(1);
		} else if (!strcmp(prefix, "wl2")) {
			iface = get_wl_instance_name(2);
		} else {
			iface = prefix;
		}
	}

	sec_mode = getSecMode(prefix);
	auth_mode = getAuthMode(prefix);

	if (strcmp(sec_mode, "0")) {
		if (!strcmp(prefix, "wl0"))
			led_control(LED_SEC0, LED_ON);
		if (!strcmp(prefix, "wl1"))
			led_control(LED_SEC1, LED_ON);
	}

	if (auth_mode == NULL) {
		return; // no nas required
	}
	if (strcmp(nvram_safe_get(apmode), "sta") && strcmp(nvram_safe_get(apmode), "wet") &&
	    strcmp(nvram_safe_get(apmode), "apstawet") && strcmp(nvram_safe_get(apmode), "apsta")) {
		mode = "-A";
		dd_loginfo("nas", "NAS lan (%s interface) successfully started", prefix);
		fnas = fopen("/tmp/.nas", "a");
		fputc('L', fnas); // L as LAN
		fclose(fnas);
	} else {
		mode = "-S";
		dd_loginfo("nas", "NAS wan (%s interface) successfully started", prefix);
		fnas = fopen("/tmp/.nas", "a");
		fputc('W', fnas); // W as WAN
		fclose(fnas);
	}

	char rekey[32];
	char ssid[32];
	char radius[32];
	char port[32];
	char index[32];

	sprintf(rekey, "%s_wpa_gtk_rekey", prefix);
	sprintf(ssid, "%s_ssid", prefix);
	sprintf(radius, "%s_radius_ipaddr", prefix);
	sprintf(port, "%s_radius_port", prefix);
	sprintf(index, "%s_key", prefix);
	nvram_default_get(rekey, "3600");
	key = getKey(prefix);
	char tmp[256];

	pid_t pid;
	FILE *fp = { 0 };

	if (!strcmp(mode, "-S")) {
		if (nvram_nmatch("wet", "%s_mode", prefix) || nvram_nmatch("apstawet", "%s_mode", prefix)) {
			char *const argv[] = { "nas",
					       "-P",
					       pidfile,
					       "-H",
					       "34954",
					       "-l",
					       getBridge(iface, tmp),
					       "-i",
					       iface,
					       mode,
					       "-m",
					       auth_mode,
					       "-k",
					       key,
					       "-s",
					       nvram_safe_get(ssid),
					       "-w",
					       sec_mode,
					       "-g",
					       nvram_safe_get(rekey),
					       NULL };
			_log_evalpid(argv, NULL, 0, &pid);
		} else {
			char *const argv[] = { "nas",	"-P",
					       pidfile, "-H",
					       "34954", "-i",
					       iface,	mode,
					       "-m",	auth_mode,
					       "-k",	key,
					       "-s",	nvram_safe_get(ssid),
					       "-w",	sec_mode,
					       "-g",	nvram_safe_get(rekey),
					       NULL };
			_log_evalpid(argv, NULL, 0, &pid);
		}

	} else {
		if (!strcmp(auth_mode, "2") || !strcmp(auth_mode, "64") || !strcmp(auth_mode, "66")) {
			char *const argv[] = { "nas",
					       "-P",
					       pidfile,
					       "-H",
					       "34954",
					       "-l",
					       nvram_nmatch("0", "%s_bridged", iface) ? iface : getBridge(iface, tmp),
					       "-i",
					       iface,
					       mode,
					       "-m",
					       auth_mode,
					       "-r",
					       key,
					       "-s",
					       nvram_safe_get(ssid),
					       "-w",
					       sec_mode,
					       "-g",
					       nvram_safe_get(rekey),
					       "-h",
					       nvram_safe_get(radius),
					       "-p",
					       nvram_safe_get(port),
					       NULL };
			_log_evalpid(argv, NULL, 0, &pid);

		} else if (!strcmp(auth_mode, "32")) {
			int idx = nvram_geti(index);
			char wepkey[32];

			sprintf(wepkey, "%s_key%d", prefix, idx);

			char *const argv[] = { "nas",
					       "-P",
					       pidfile,
					       "-H",
					       "34954",
					       "-l",
					       nvram_nmatch("0", "%s_bridged", iface) ? iface : getBridge(iface, tmp),
					       "-i",
					       iface,
					       mode,
					       "-m",
					       auth_mode,
					       "-r",
					       key,
					       "-s",
					       nvram_safe_get(ssid),
					       "-w",
					       sec_mode,
					       "-I",
					       nvram_safe_get(index),
					       "-k",
					       nvram_safe_get(wepkey),
					       "-h",
					       nvram_safe_get(radius),
					       "-p",
					       nvram_safe_get(port),
					       NULL };
			_log_evalpid(argv, NULL, 0, &pid);
		} else {
			char *const argv[] = { "nas",
					       "-P",
					       pidfile,
					       "-H",
					       "34954",
					       "-l",
					       nvram_nmatch("0", "%s_bridged", iface) ? iface : getBridge(iface, tmp),
					       "-i",
					       iface,
					       mode,
					       "-m",
					       auth_mode,
					       "-k",
					       key,
					       "-s",
					       nvram_safe_get(ssid),
					       "-w",
					       sec_mode,
					       "-g",
					       nvram_safe_get(rekey),
					       NULL };
			_log_evalpid(argv, NULL, 0, &pid);
		}
	}

	fp = fopen(pidfile, "w");
	if (fp)
		fprintf(fp, "%d", pid);
	fclose(fp);

	cprintf("done\n");

	return;
}

void stop_nas(void)
{
	int ret = 0;
	char name[80], *next;
	FILE *check = fopen("/tmp/.startnas", "rb");
	if (check) {
		fclose(check);
		return;
	}
	stop_nas_process();
}

#endif
#endif
#endif
#endif
