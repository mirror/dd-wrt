/*
 * openvpn.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <gottschall@dd-wrt.com
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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <services.h>

#ifdef HAVE_OPENVPN

void start_openvpnserver(void)
{

	if (nvram_invmatch("openvpn_enable", "1"))
		return;
	mkdir("/tmp/openvpn", 0700);
	write_nvram("/tmp/openvpn/dh.pem", "openvpn_dh");
	write_nvram("/tmp/openvpn/ca.crt", "openvpn_ca");
	write_nvram("/tmp/openvpn/cert.pem", "openvpn_crt");
	write_nvram("/tmp/openvpn/ca.crl", "openvpn_crl");
	write_nvram("/tmp/openvpn/key.pem", "openvpn_key");
	write_nvram("/tmp/openvpn/ta.key", "openvpn_tlsauth");
	/*
	   26.10.2010 Sash      
	   write openvpn server config file on current config and common settings
	   TODO GUI integration
	 */
	write_nvram("/tmp/openvpn/openvpn.conf", "openvpn_config");
	FILE *fp = fopen("/tmp/openvpn/openvpn.conf", "a+b");	//be sure to append and force non override
	if (fp == NULL)
		return;
	fprintf(fp, "dh /tmp/openvpn/dh.pem\n");
	fprintf(fp, "ca /tmp/openvpn/ca.crt\n");
	fprintf(fp, "cert /tmp/openvpn/cert.pem\n");
	fprintf(fp, "key /tmp/openvpn/key.pem\n");
//	fprintf(fp, "crl-verify /tmp/openvpn/ca.crl\n");  disable to prevent problems for now
//	fprintf(fp, "tls-auth /tmp/openvpn/ta.key 0\n");
	//be sure Chris old style ist still working
	if (nvram_match("openvpn_switch", "1")) {
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_crt");
		fprintf(fp, "client-to-client\n");
		fprintf(fp, "keepalive 10 60\n");
		fprintf(fp, "verb 4\n");
		fprintf(fp, "mute 20\n");
		fprintf(fp, "log-append /var/log/openvpn\n");
		fprintf(fp, "tls-server\n");
		fprintf(fp, "port %s\n", nvram_safe_get("openvpn_port"));
		fprintf(fp, "proto %s\n", nvram_safe_get("openvpn_proto"));
		if (nvram_match("openvpn_certtype", "1"))
			fprintf(fp, "ns-cert-type server\n");
		if (nvram_match("openvpn_lzo", "1"))
			fprintf(fp, "comp-lzo\n");
		if (nvram_match("openvpn_cl2cl", "1"))
			fprintf(fp, "client-to-client\n");
		if (nvram_match("openvpn_tuntap", "tun")) {
			fprintf(fp, "server %s %s\n",
				nvram_safe_get("openvpn_net"),
				nvram_safe_get("openvpn_mask"));
			fprintf(fp, "dev tun\n");
		}
		else {
			fprintf(fp, "server-bridge %s %s %s %s\n",
				nvram_safe_get("openvpn_gateway"),
				nvram_safe_get("openvpn_mask"),
				nvram_safe_get("openvpn_startip"),
				nvram_safe_get("openvpn_endip"));
			fprintf(fp, "dev tap\n");
		}
		fprintf(fp, "management 127.0.0.1 5001\n");
		fprintf(fp, "management-log-cache 50\n");
	} else {
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_client");

	}
	fclose(fp);

	sysprintf("iptables -I INPUT -p %s --dport %s -j ACCEPT\n",nvram_match("openvpn_proto","udp")?"udp":"tcp",nvram_safe_get("openvpn_port"));

	fp = fopen("/tmp/openvpn/route-up.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "startservice set_routes\n");;
	fprintf(fp, "iptables -I FORWARD 1 -i %s+ -j ACCEPT\n",nvram_safe_get("openvpn_tuntap"));
	fprintf(fp, "iptables -I FORWARD 2 -o %s+ -j ACCEPT\n",nvram_safe_get("openvpn_tuntap"));
	fclose(fp);

	fp = fopen("/tmp/openvpn/route-down.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "iptables -D FORWARD -i %s+ -j ACCEPT\n",nvram_safe_get("openvpn_tuntap"));
	fprintf(fp, "iptables -D FORWARD -o %s+ -j ACCEPT\n",nvram_safe_get("openvpn_tuntap"));
	fclose(fp);

	chmod("/tmp/openvpn/route-up.sh", 0700);
	chmod("/tmp/openvpn/route-down.sh", 0700);

	if (nvram_match("use_crypto", "1"))
		eval("openvpn", "--config", "/tmp/openvpn/openvpn.conf",
		     "--route-up", "/tmp/openvpn/route-up.sh", "--down",
		     "/tmp/openvpn/route-down.sh", "--daemon", "--engine",
		     "cryptodev");
	else
		eval("openvpn", "--config", "/tmp/openvpn/openvpn.conf",
		     "--route-up", "/tmp/openvpn/route-up.sh", "--down",
		     "/tmp/openvpn/route-down.sh", "--daemon");
}

void stop_openvpnserver(void)
{
	stop_process("openvpn", "OpenVPN daemon");
	return;
}

void start_openvpnserverwan(void)
{
	if (nvram_match("openvpn_onwan", "1"))
		start_openvpnserver();
	return;
}

void stop_openvpnserverwan(void)
{
	if (nvram_match("openvpn_onwan", "1"))
		stop_openvpnserver();
	return;
}

void start_openvpnserversys(void)
{
	if (nvram_match("openvpn_onwan", "0"))
		start_openvpnserver();
	return;
}

void stop_openvpnserversys(void)
{
	if (nvram_match("openvpn_onwan", "0"))
		stop_openvpnserver();
	return;
}

void start_openvpn(void)
{
	if (nvram_invmatch("openvpncl_enable", "1"))
		return;
	mkdir("/tmp/openvpncl", 0700);
	write_nvram("/tmp/openvpncl/ca.crt", "openvpncl_ca");
	write_nvram("/tmp/openvpncl/client.crt", "openvpncl_client");
	write_nvram("/tmp/openvpncl/client.key", "openvpncl_key");
	write_nvram("/tmp/openvpncl/openvpn.conf", "openvpncl_config");

	FILE *fp = fopen("/tmp/openvpncl/openvpn.conf", "a+b");
	if (fp == NULL)
		return;
	fprintf(fp, "client\n");
	fprintf(fp, "dev %s\n", nvram_safe_get("openvpncl_tuntap"));
	fprintf(fp, "proto %s\n", nvram_safe_get("openvpncl_proto"));
	fprintf(fp, "remote %s %s\n", nvram_safe_get("openvpncl_remoteip"),
		nvram_safe_get("openvpncl_remoteport"));
	fprintf(fp, "resolv-retry infinite\n");
	fprintf(fp, "nobind\n");
	// fprintf(fp,"user nobody\n");
	// fprintf(fp,"group nobody\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	if (nvram_invmatch("openvpncl_mtu", ""))
		fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpncl_mtu"));
	if (nvram_invmatch("openvpncl_extramtu", ""))
		fprintf(fp, "tun-mtu-extra %s\n",
			nvram_safe_get("openvpncl_extramtu"));
	if (nvram_invmatch("openvpncl_mssfix", ""))
		fprintf(fp, "mssfix %s\n", nvram_safe_get("openvpncl_mssfix"));
	fprintf(fp, "ca /tmp/openvpncl/ca.crt\n");
	fprintf(fp, "cert /tmp/openvpncl/client.crt\n");
	fprintf(fp, "key /tmp/openvpncl/client.key\n");
//	fprintf(fp, "tls-auth /tmp/openvpncl/client.key 1\n"); for future usage
	fprintf(fp, "management 127.0.0.1 5002\n");
	fprintf(fp, "management-log-cache 50\n");

	// Botho 22/05/2006 - start
	if (nvram_match("openvpncl_certtype", "1"))
		fprintf(fp, "ns-cert-type server\n");
	// end
	if (nvram_match("openvpncl_lzo", "1"))
		fprintf(fp, "comp-lzo\n");
	fclose(fp);

	fp = fopen("/tmp/openvpncl/route-up.sh", "wb");
	if (fp == NULL)
		return;
	if (nvram_match("openvpncl_nat", "1"))
		fprintf(fp,
			"iptables -A POSTROUTING -t nat -o tun0 -j MASQUERADE\n");
	else {
		fprintf(fp, "iptables -A FORWARD 1 -i %s+ -j ACCEPT\n",nvram_safe_get("openvpncl_tuntap"));
		fprintf(fp, "iptables -A FORWARD 2 -o %s+ -j ACCEPT\n",nvram_safe_get("openvpncl_tuntap"));
	}
	fclose(fp);

	fp = fopen("/tmp/openvpncl/route-down.sh", "wb");
	if (fp == NULL)
		return;
	if (nvram_match("openvpncl_nat", "1"))
		fprintf(fp,
			"iptables -D POSTROUTING -t nat -o tun0 -j MASQUERADE\n");
	else {
		fprintf(fp, "iptables -D FORWARD -i %s+ -j ACCEPT\n",nvram_safe_get("openvpncl_tuntap"));
		fprintf(fp, "iptables -D FORWARD -o %s+ -j ACCEPT\n",nvram_safe_get("openvpncl_tuntap"));
	}
	fclose(fp);

	chmod("/tmp/openvpncl/route-up.sh", 0700);
	chmod("/tmp/openvpncl/route-down.sh", 0700);

	if (nvram_match("use_crypto", "1"))
		eval("openvpn", "--config", "/tmp/openvpncl/openvpn.conf",
		     "--route-up", "/tmp/openvpn/route-up.sh", "--down",
		     "/tmp/openvpn/route-down.sh", "--daemon", "--engine",
		     "cryptodev");
	else
		eval("openvpn", "--config", "/tmp/openvpncl/openvpn.conf",
		     "--route-up", "/tmp/openvpncl/route-up.sh", "--down",
		     "/tmp/openvpncl/route-down.sh", "--daemon");
	return;
}

void stop_openvpn(void)
{
	stop_process("openvpn", "OpenVPN daemon");
}

void stop_openvpn_wandone(void)
{
	if (nvram_invmatch("openvpncl_enable", "1"))
		return;
	stop_process("openvpn", "OpenVPN daemon");
}

#endif
