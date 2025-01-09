/*
 * ovpncl_config.c
 *
 * Copyright (C) 2005 - 2021 Sebastian Gottschall <s.gottschall@dd-wrt.com>, modified 2022 by egc
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <utils.h>
#include <broadcom.h>
#include <dd_defs.h>
#include <revision.h>


//debug
#include <syslog.h>
#include <shutils.h>

static int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
static char *wfgets(char *buf, int len, webs_t fp, int *eof);
size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
int wfflush(webs_t fp);
static int do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);

static int make_ovpncl_config(char *ovnpcl_fname)
{
	//char *vpnproto = nvram_safe_get("openvpn_proto");
	//char vpnproto[14] = { 0 };
	//dd_loginfo("openvpncl_config", "vpnproto %s\n", vpnproto);
	char dataciphers[72] = { 0 };

	FILE *f1 = fopen(ovnpcl_fname, "w+");
	if (!f1) {
		dd_loginfo("openvpncl_config", "Could not open %s", ovnpcl_fname);
		return -1;
	} else {
		fprintf(f1, "#This is beta build 0.92, use it with care\n");
		fprintf(f1, "#OpenVPN client config generated, check if settings are correct see: %s, made by %s\n",
			"https://forum.dd-wrt.com/phpBB2/viewtopic.php?t=327398", "egc");
		fprintf(f1,
			"client\n#windows-driver wintun     # For Windows 10 and OpenVPN 2.5 and higher\nverb 3\nnobind\npersist-key\npersist-tun\nfloat\nremote-cert-tls server\nauth-nocache\n");
		fprintf(f1, "tun-mtu 1400    # lowered default can be commented to let OpenVPN decide\n");
		fprintf(f1, "#Replace remote address with actual WAN or DDNS address\nremote %s %s\n", nvram_safe_get("wan_ipaddr"),
			nvram_safe_get("openvpn_port"));
		if (nvram_match("openvpn_tuntap", "tap")) {
			fprintf(f1, "dev tap\n");
		} else {
			fprintf(f1, "dev tun\n");
		}
		//fprintf(f1, "proto %s\n", (!strcmp(vpnproto, "udp4")) ? "udp4" : "tcp4");
		if (nvram_match("openvpn_proto", "tcp4-server") || nvram_match("openvpn_proto", "tcp-server")) {
			fprintf(f1, "proto %s\n", "tcp4");
		} else if (nvram_match("openvpn_proto", "tcp6-server")) {
			fprintf(f1, "proto %s\n", "tcp6");
		} else {
			fprintf(f1, "proto %s\n", nvram_safe_get("openvpn_proto"));
		}

		if (nvram_invmatch("openvpn_auth", "")) {
			fprintf(f1, "auth %s\n", nvram_safe_get("openvpn_auth"));
		}

		if (nvram_invmatch("openvpn_dc1", "")) {
			sprintf(dataciphers, "%s", nvram_safe_get("openvpn_dc1"));
		}
		if (dataciphers[0] != '\0' && nvram_invmatch("openvpn_dc2", "")) {
			sprintf(dataciphers, "%s:%s", dataciphers, nvram_safe_get("openvpn_dc2"));
		} else if (nvram_invmatch("openvpn_dc2", "")) {
			sprintf(dataciphers, "%s", nvram_safe_get("openvpn_dc2"));
		}
		if (dataciphers[0] != '\0' && nvram_invmatch("openvpn_dc3", "")) {
			sprintf(dataciphers, "%s:%s", dataciphers, nvram_safe_get("openvpn_dc3"));
		} else if (nvram_invmatch("openvpn_dc3", "")) {
			sprintf(dataciphers, "%s", nvram_safe_get("openvpn_dc3"));
		}
		fprintf(f1, "data-ciphers %s\n", dataciphers);
		fprintf(f1,
			"#Block IPv6, newer clients could default to IPv6\npull-filter ignore \"route-ipv6\"\npull-filter ignore \"ifconfig-ipv6\"\nblock-ipv6\n");
		switch (nvram_geti("openvpn_tls_btn")) {
		case 0:
			fprintf(f1, "key-direction 1\n<tls-auth>\n%s\n</tls-auth>\n", nvram_safe_get("openvpn_tlsauth"));
			break;
		case 1:
			fprintf(f1, "<tls-crypt>\n%s\n</tls-crypt>\n", nvram_safe_get("openvpn_tlsauth"));
			break;
		case 2:
			fprintf(f1, "<secret>\n%s\n</secret>\n", nvram_safe_get("openvpn_static"));
			break;
		case 4:
			fprintf(f1, "<tls-crypt-v2>\n%s\n</tls-crypt-v2>\n", "PLACE-CLIENT-TLS-CRYPT-v2-KEY-HERE");
			break;
		}
		if (nvram_invmatch("openvpn_ca", "")) {
			fprintf(f1, "<ca>\n%s\n</ca>\n", nvram_safe_get("openvpn_ca"));
		}
		if (nvram_invmatch("openvpn_key", "")) {
			fprintf(f1, "<cert>\n%s\n</cert>\n<key>\n%s\n</key>\n", "PLACE-PUBLIC-CLIENT-CERTIFICATE-HERE",
				"PLACE-PRIVATE-CLIENT-KEY-HERE");
		}
		if (nvram_matchi("openvpn_enuserpass", 1)) {
			//fprintf(f1, "#Make file credentials.txt in the same directory and place username and password on separate lines\nauth-user-pass credentials.txt\nauth-retry interact\n");
			fprintf(f1,
				"#For OpenVPN 2.6 and higher, inline username and password\n<auth-user-pass>\nPLACE-USERNAME-HERE\nPLACE-PASSWORD-HERE\n</auth-user-pass>\nauth-retry interact\n");
		}
		//dd_loginfo("openvpncl_config", "Success file open: %s", ovnpcl_fname);
		fclose(f1);
	}
	return 0;
}

static int download_ovpncl_config(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	char fname[128];
	snprintf(fname, sizeof(fname), "%s", path);
	char dname[128];
	snprintf(dname, sizeof(dname), "%s", path);
	char *p = strstr(dname, "..");
	if (p)
		return -1;
	p = strstr(dname, "/");
	if (p)
		return -1;

	p = strstr(dname, ".");
	if (p)
		*p = '_';
	char location[128];
	snprintf(location, sizeof(location), "/tmp/openvpn/%s", dname);

	dd_loginfo("openvpncl_config", "location: %s", location);
	dd_loginfo("openvpncl_config", "fname: %s", fname);

	if (!make_ovpncl_config(location)) {
		return do_file_attach(handler, location, wp, fname);
	} else {
		dd_loginfo("openvpncl_config", "could not open: %s", location);
	}
	return 0;
}
