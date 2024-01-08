/*
 * AnchorFree.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <netdb.h>
#include <services.h>
#include "md5.h"

#ifndef HAVE_MADWIFI
#define IFPREFIX "wl"
#else
#define IFPREFIX "wlan"
#endif

void doHash(md5_ctx_t *MD, char *filename)
{
	unsigned char buf[1];
	FILE *in = fopen(filename, "rb");

	if (in == NULL) {
		return;
	}
	while (1) {
		int c = getc(in);

		if (c == EOF)
			break;
		buf[0] = c;
		md5_hash(buf, 1, MD);
	}
	fclose(in);
}

void deviceID(char *output)
{
	unsigned char key[16];
	md5_ctx_t MD;

	md5_begin(&MD);
	// fprintf (stderr, "generate hash\n");
	doHash(&MD, "/dev/mtdblock/0");
	doHash(&MD, "/dev/mtdblock0");
	doHash(&MD,
	       "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0051/eeprom");
	doHash(&MD, "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom");
	doHash(&MD, "/dev/discs/disc0/part4");
	dd_md5_end((unsigned char *)key, &MD);
	int i;

	for (i = 0; i < 16; i++) {
		unsigned int k = key[i];

		sprintf(output, "%s%X", output, k & 0xff);
	}
	// fprintf (stderr, "final hash = %s\n", output);
}
unsigned char ctable[16] = { "0123456789ABCDEF" };

void toURL(char *t, char *output)
{
	unsigned char *text = (unsigned char *)t;
	char *p = output;
	int i;
	int n = 0;

	for (i = 0; i < strlen(t); i++) {
		p[n++] = '%';
		p[n++] = ctable[text[i] / 16];
		p[n++] = ctable[text[i] % 16];
	}
	p[n++] = 0;
}

void start_anchorfree(void)
{
	int need_commit = 0;

	nvram_seti("af_dnathost", 0);
	nvram_seti("af_dnatport", 0);
	if (nvram_matchi("af_serviceid", 0))
		nvram_seti("af_registered", 0);
	nvram_seti("af_serviceid", 0);

	if (nvram_matchi("af_enable", 1) && !nvram_matchi("af_registered", 1)) {
		nvram_seti("af_registered", 1);
		dd_loginfo("anchorfree", "starting redirection\n");
		char devid[256];

		if (!nvram_exists("af_hash")) {
			bzero(devid, 256);
			deviceID(devid);
			nvram_set("af_hash", devid);
			nvram_async_commit();
		} else {
			strcpy(devid, nvram_safe_get("af_hash"));
		}
		char email[256];

		toURL(nvram_safe_get("af_email"), email);
		char ssid[128];

		if (nvram_matchi("af_ssid", 1)) {
			toURL(nvram_safe_get("af_ssid_name"), ssid);
		} else {
#ifndef HAVE_MADWIFI
			toURL(nvram_safe_get("wl0_ssid"), ssid);
#else
			toURL(nvram_safe_get("wlan0_ssid"), ssid);
#endif
		}

		if (nvram_matchi("af_ssid", 1) &&
		    !nvram_matchi("af_ssid_created", 1)) {
			nvram_seti("af_ssid_created", 1);
#ifndef HAVE_MADWIFI
			nvram_set("wl0_vifs", "wl0.1");
			nvram_set("wl_vifs", "wl0.1");
			nvram_set("wl0.1_ssid", nvram_safe_get("af_ssid_name"));
			nvram_seti("wl0.1_bridged", 0);
			nvram_set("wl0.1_ipaddr", "172.45.0.1");
			nvram_set("wl0.1_netmask", "255.255.255.0");
			nvram_seti("mdhcpd_count", 1);
			nvram_set("mdhcpd", "wl0.1>On>100>50>1440");
#else
			nvram_set("wlan0_vifs", "wlan0.1");
			nvram_set("wlan0.1_ssid",
				  nvram_safe_get("af_ssid_name"));
			nvram_seti("wlan0.1_bridged", 0);
			nvram_set("wlan0.1_ipaddr", "172.45.0.1");
			nvram_set("wlan0.1_netmask", "255.255.255.0");
			nvram_seti("mdhcpd_count", 1);
			nvram_set("mdhcpd", "wlan0.1>On>100>50>1440");
#endif
			need_commit = 1;
			stop_lan();
			start_lan();
			stop_dnsmasq();
			start_dnsmasq();
#ifdef HAVE_SMARTDNS
			stop_smartdns();
			start_smartdns();
#endif
			stop_unbound();
			start_unbound();
			start_firewall();
			stop_firewall();
			start_firewall();
			if (getSTA()) {
				stop_wan();
				start_wan_boot();
				return;
			}
		} else if (nvram_matchi("af_ssid", 0) &&
			   nvram_matchi("af_ssid_created", 1)) {
			nvram_seti("af_ssid_created", 0);
#ifndef HAVE_MADWIFI
			nvram_set("wl0_vifs", "");
			nvram_set("wl_vifs", "");
			nvram_seti("mdhcpd_count", 0);
			nvram_set("mdhcpd", "");
#else
			nvram_set("wlan0_vifs", "");
			nvram_seti("mdhcpd_count", 0);
			nvram_set("mdhcpd", "");
#endif
			need_commit = 1;
			stop_lan();
			start_lan();
			stop_dnsmasq();
			start_dnsmasq();
			stop_firewall();
			start_firewall();
			if (getSTA()) {
				stop_wan();
				start_wan_boot();
				return;
			}
		} else if (nvram_matchi("af_ssid_created", 1)) {
#ifndef HAVE_MADWIFI
			if (!nvram_match("af_ssid_name",
					 nvram_safe_get("wl0.1_ssid"))) {
				nvram_set("wl0.1_ssid",
					  nvram_safe_get("af_ssid_name"));
#else
			if (!nvram_match("af_ssid_name",
					 nvram_safe_get("wlan0.1_ssid"))) {
				nvram_set("wlan0.1_ssid",
					  nvram_safe_get("af_ssid_name"));

#endif
				need_commit = 1;
				stop_lan();
				start_lan();
				stop_dnsmasq();
				start_dnsmasq();
				stop_firewall();
				start_firewall();
				if (getSTA()) {
					stop_wan();
					start_wan_boot();
					return;
				}
			}
		}

		char addr[256];

		toURL(nvram_safe_get("af_address"), addr);
		char addr2[256];

		toURL(nvram_safe_get("af_address_2"), addr2);
		char city[256];

		toURL(nvram_safe_get("af_city"), city);
		char zip[64];

		toURL(nvram_safe_get("af_zip"), zip);
		char state[64];

		toURL(nvram_safe_get("af_state"), state);
		char country[64];

		strcpy(country, getIsoName(nvram_safe_get("af_country")));
		char cat[64];

		toURL(nvram_safe_get("af_category"), cat);
		unlink("/tmp/.anchorfree");
		char callbuffer[512];

		sprintf(callbuffer,
			"wget -q -O /tmp/.anchorfree \"http://afhrp.anchorfree.com/register.php?"
			"pid=0001&"
			"uid=%s&"
			"email=%s&"
			"ssid=%s&"
			"addr=%s&"
			"addr2=%s&"
			"city=%s&"
			"zip=%s&"
			"state=%s&"
			"country=%s&"
			"cat=%s&"
			"publish=%s\"",
			devid, email, ssid, addr, addr2, city, zip, state,
			country, cat, nvram_safe_get("af_publish"));
		system(callbuffer);
		FILE *response = fopen("/tmp/.anchorfree", "rb");

		if (response == NULL) {
			fprintf(stderr,
				"error while registration (cannot reach registration site)!\n");
			nvram_set("af_servicestatus",
				  "cannot reach registration site!");
			nvram_seti("af_registered", 0);
			nvram_async_commit();
			return;
		}
		char status[32];
		int r = fscanf(response, "status: %s\n", status);

		if (r != 1) {
			fprintf(stderr, "registration failed (bad status)\n");
			nvram_set("af_servicestatus",
				  "registration failed (bad status)");
			fclose(response);
			nvram_seti("af_registered", 0);
			nvram_async_commit();
			return;
		}
		nvram_set("af_servicestatus", status);
		fprintf(stderr, "status: %s\n", status);
		if (strcmp(status, "OK")) {
			fprintf(stderr, "registration failed\n");
			fclose(response);
			nvram_seti("af_registered", 0);
			nvram_async_commit();
			return;
		}
		bzero(status, 32);
		fscanf(response, "sid: %s\n", status);
		if (r != 1) {
			fprintf(stderr, "registration failed (bad sid)\n");
			nvram_set("af_servicestatus",
				  "registration failed (bad sid)");
			fclose(response);
			nvram_seti("af_registered", 0);
			nvram_async_commit();
			return;
		}
		fprintf(stderr, "configuring service id %s\n", status);
		nvram_set("af_serviceid", status);
		bzero(status, 32);
		fscanf(response, "host: %s\n", status);
		nvram_set("af_dnathost", status);
		bzero(status, 32);
		fscanf(response, "port: %s\n", status);
		nvram_set("af_dnatport", status);
		fclose(response);
		need_commit = 1;
	} else if (nvram_matchi("af_ssid_created", 1)) {
		nvram_seti("af_ssid_created", 0);
#ifndef HAVE_MADWIFI
		nvram_set("wl0_vifs", "");
		nvram_set("wl_vifs", "");
		nvram_seti("mdhcpd_count", 0);
		nvram_set("mdhcpd", "");
#else
		nvram_set("wlan0_vifs", "");
		nvram_seti("mdhcpd_count", 0);
		nvram_set("mdhcpd", "");
#endif
		need_commit = 1;
		stop_lan();
		start_lan();
		stop_dnsmasq();
		start_dnsmasq();
	}

	if (need_commit)
		nvram_async_commit();
	return;
}

void start_anchorfreednat(void)
{
	char dest[32];
	char source[32];

	if (nvram_matchi("af_enable", 1) && !nvram_matchi("af_dnathost", 0)) {
		char host[128];

		getIPFromName(nvram_safe_get("af_dnathost"), host,
			      sizeof(host));
		if (!strcmp(host, "0.0.0.0")) {
			fprintf(stderr, "cannot resolve %s\n",
				nvram_safe_get("af_dnathost"));
			return;
		}
		sprintf(dest, "%s:%s", host, nvram_safe_get("af_dnatport"));
		// sprintf (source, "%d/%d", nvram_safe_get ("lan_ipaddr"),
		// getmask (nvram_safe_get ("lan_netmask")));
		sprintf(source, "0.0.0.0/0");
		if (nvram_matchi("af_ssid", 1))
			sprintf(source, "%s/%d",
				nvram_safe_get(IFPREFIX "0.1_ipaddr"),
				getmask(nvram_safe_get(IFPREFIX
						       "0.1_netmask")));

		if (nvram_matchi("af_ssid", 1)) {
			eval("iptables", "-t", "nat", "-D", "PREROUTING", "-s",
			     source, "-p", "tcp", "-d",
			     nvram_safe_get("lan_ipaddr"), "-j", "DROP");
		} else {
			eval("iptables", "-t", "nat", "-D", "PREROUTING", "-s",
			     source, "-p", "tcp", "-d",
			     nvram_safe_get("lan_ipaddr"), "-j", "DNAT", "--to",
			     nvram_safe_get("lan_ipaddr"));
		}

		eval("iptables", "-t", "nat", "-D", "PREROUTING", "-s", source,
		     "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to", dest);
		if (nvram_matchi("af_ssid", 1)) {
			eval("iptables", "-t", "nat", "-A", "PREROUTING", "-s",
			     source, "-p", "tcp", "-d",
			     nvram_safe_get("lan_ipaddr"), "-j", "DROP");
		} else {
			eval("iptables", "-t", "nat", "-A", "PREROUTING", "-s",
			     source, "-p", "tcp", "-d",
			     nvram_safe_get("lan_ipaddr"), "-j", "DNAT", "--to",
			     nvram_safe_get("lan_ipaddr"));
		}

		eval("iptables", "-t", "nat", "-A", "PREROUTING", "-s", source,
		     "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to", dest);
	}
}

void stop_anchorfree_unregister(void)
{
	unlink("/tmp/.anchorfree");
	char callbuffer[512];

	sprintf(callbuffer,
		"wget -q -O- \"http://afhrp.anchorfree.com/unregister.php?"
		"uid=%s&"
		"sid=%s\"",
		nvram_safe_get("af_hash"), nvram_safe_get("af_serviceid"));
	system(callbuffer);
}

void stop_anchorfree(void)
{
	if (!nvram_matchi("af_serviceid", 0) &&
	    nvram_matchi("af_registered", 1)) {
		nvram_seti("af_registered", 0);
		char dest[32];
		char source[32];

		dd_loginfo("anchorfree", "stopping redirection\n");
		char host[128];

		getIPFromName(nvram_safe_get("af_dnathost"), host,
			      sizeof(host));
		sprintf(dest, "%s:%s", host, nvram_safe_get("af_dnatport"));
		// sprintf (source, "%s/%d", nvram_safe_get ("lan_ipaddr"),
		// getmask (nvram_safe_get ("lan_netmask")));
		sprintf(source, "0.0.0.0/0");
		if (nvram_matchi("af_ssid", 1))
			sprintf(source, "%s/%d",
				nvram_safe_get(IFPREFIX "0.1_ipaddr"),
				getmask(nvram_safe_get(IFPREFIX
						       "0.1_netmask")));

		eval("iptables", "-t", "nat", "-D", "PREROUTING", "-s", source,
		     "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to", dest);

		if (nvram_matchi("af_ssid", 1)) {
			eval("iptables", "-t", "nat", "-D", "PREROUTING", "-s",
			     source, "-p", "tcp", "-d",
			     nvram_safe_get("lan_ipaddr"), "-j", "DROP");
		} else {
			eval("iptables", "-t", "nat", "-D", "PREROUTING", "-s",
			     source, "-p", "tcp", "-d",
			     nvram_safe_get("lan_ipaddr"), "-j", "DNAT", "--to",
			     nvram_safe_get("lan_ipaddr"));
		}

		unlink("/tmp/.anchorfree");
		if (nvram_matchi("af_enable", 0))
			stop_anchorfree_unregister();
		nvram_async_commit();
	}
	return;
}
