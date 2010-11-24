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
	chmod("/tmp/openvpn/key.pem", 0600);
	/*
	   26.10.2010 Sash      
	   write openvpn server config file on current config and common settings
	   TODO GUI integration
	 */
	FILE *fp = fopen("/tmp/openvpn/openvpn.conf", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "dh /tmp/openvpn/dh.pem\n");
	fprintf(fp, "ca /tmp/openvpn/ca.crt\n");
	fprintf(fp, "cert /tmp/openvpn/cert.pem\n");
	fprintf(fp, "key /tmp/openvpn/key.pem\n");
	//be sure Chris old style ist still working
	if (nvram_match("openvpn_switch", "1")) {
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_crt");
		fprintf(fp, "keepalive 10 120\n");
		fprintf(fp, "verb 4\n");
		fprintf(fp, "mute 5\n");
		fprintf(fp, "log-append /var/log/openvpn\n");
		fprintf(fp, "tls-server\n");
		fprintf(fp, "port %s\n", nvram_safe_get("openvpn_port"));
		fprintf(fp, "proto %s\n", nvram_safe_get("openvpn_proto"));
		fprintf(fp, "cipher %s\n", nvram_safe_get("openvpn_cipher"));
		fprintf(fp, "auth %s\n", nvram_safe_get("openvpn_auth"));
		fprintf(fp, "management 127.0.0.1 5002\n");
		fprintf(fp, "management-log-cache 50\n");
		fprintf(fp, "mtu-disc yes\n");
		fprintf(fp, "topology subnet\n");
		if (nvram_match("openvpn_dupcn", "1"))
			fprintf(fp, "duplicate-cn\n");
		else		//store client ip.keep them persistant for x sec.works only when dupcn=off
			fprintf(fp,
				"ifconfig-pool-persist /tmp/openvpn/ip-pool 86400\n");
		if (nvram_match("openvpn_certtype", "1"))
			fprintf(fp, "ns-cert-type server\n");
		if (nvram_match("openvpn_lzo", "1"))
			fprintf(fp, "comp-lzo yes\n");	//yes/no/adaptive
		if (nvram_match("openvpn_cl2cl", "1"))
			fprintf(fp, "client-to-client\n");
		if (nvram_match("openvpn_redirgate", "1"))
			fprintf(fp, "push \"redirect-gateway def1\"\n");
		if (nvram_match("openvpn_proto", "udp"))
			fprintf(fp, "fast-io\n");	//experimental!improving CPU efficiency by 5%-10%
		else		//TCP_NODELAY is generally a good latency optimization
			fprintf(fp, "tcp-nodelay\n");
		if (nvram_invmatch("openvpn_mtu", ""))
			fprintf(fp, "tun-mtu %s\n",
				nvram_safe_get("openvpn_mtu"));
		if (nvram_invmatch("openvpn_mssfix", "")
		    && nvram_match("openvpn_proto", "udp")) {
			fprintf(fp, "mssfix %s\n", nvram_safe_get("openvpn_mssfix"));	//fragment=mssfix
			fprintf(fp, "fragment %s\n",
				nvram_safe_get("openvpn_mssfix"));
		}
		if (nvram_match("openvpn_tuntap", "tun")) {
			fprintf(fp, "server %s %s\n",
				nvram_safe_get("openvpn_net"),
				nvram_safe_get("openvpn_mask"));
			fprintf(fp, "dev tun0\n");
//                      fprintf(fp, "tun-ipv6\n"); //enable ipv6 support. not supported on server in version 2.1.3
		} else {
			fprintf(fp, "server-bridge %s %s %s %s\n",
				nvram_safe_get("openvpn_gateway"),
				nvram_safe_get("openvpn_mask"),
				nvram_safe_get("openvpn_startip"),
				nvram_safe_get("openvpn_endip"));
			fprintf(fp, "dev tap0\n");
		}
		if (strlen(nvram_safe_get("openvpn_tlsauth")) > 0)
			fprintf(fp, "tls-auth /tmp/openvpn/ta.key 0\n");
		if (strlen(nvram_safe_get("openvpn_crl")) > 0)
			fprintf(fp, "crl-verify /tmp/openvpn/ca.crl\n");
	} else {
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_client");

	}
	fprintf(fp, "%s\n", nvram_safe_get("openvpn_config"));
	fclose(fp);

	//accept incoming connections on startup
	sysprintf("iptables -I INPUT -p %s --dport %s -j ACCEPT\n",
		  nvram_match("openvpn_proto", "udp") ? "udp" : "tcp",
		  nvram_safe_get("openvpn_port"));

	fp = fopen("/tmp/openvpn/route-up.sh", "wb");
	if (fp == NULL)
		return;
	//bring up tap interface when choosen
	if (nvram_match("openvpn_tuntap", "tap")) {
		fprintf(fp, "brctl addif br0 tap0\n");
		fprintf(fp, "ifconfig tap0 0.0.0.0 promisc up\n");
	}
	fprintf(fp, "startservice set_routes\n");
	fprintf(fp, "iptables -I INPUT -i %s0 -j ACCEPT\n",
		nvram_safe_get("openvpn_tuntap"));
	fprintf(fp, "iptables -I FORWARD -i %s0 -j ACCEPT\n",
		nvram_safe_get("openvpn_tuntap"));
	fprintf(fp, "iptables -I FORWARD -o %s0 -j ACCEPT\n",
		nvram_safe_get("openvpn_tuntap"));
	fclose(fp);

	fp = fopen("/tmp/openvpn/route-down.sh", "wb");
	if (fp == NULL)
		return;
	if (nvram_match("openvpn_tuntap", "tap")) {
		fprintf(fp, "brctl delif br0 tap0\n");
		fprintf(fp, "ifconfig tap0 down\n");
	}
	fprintf(fp, "iptables -D INPUT -i %s0 -j ACCEPT\n",
		nvram_safe_get("openvpn_tuntap"));
	fprintf(fp, "iptables -D INPUT -p %s --dport %s -j ACCEPT\n",
		nvram_match("openvpn_proto", "udp") ? "udp" : "tcp",
		nvram_safe_get("openvpn_port"));
	fprintf(fp, "iptables -D FORWARD -i %s0 -j ACCEPT\n",
		nvram_safe_get("openvpn_tuntap"));
	fprintf(fp, "iptables -D FORWARD -o %s0 -j ACCEPT\n",
		nvram_safe_get("openvpn_tuntap"));
	fclose(fp);

	chmod("/tmp/openvpn/route-up.sh", 0700);
	chmod("/tmp/openvpn/route-down.sh", 0700);
	eval("ln", "-s", "/usr/sbin/openvpn", "/tmp/openvpnserver");

	if (nvram_match("use_crypto", "1"))
		eval("/tmp/openvpnserver", "--config",
		     "/tmp/openvpn/openvpn.conf", "--up",
		     "/tmp/openvpn/route-up.sh", "--down",
		     "/tmp/openvpn/route-down.sh", "--daemon", "--engine",
		     "cryptodev");
	else
		eval("/tmp/openvpnserver", "--config",
		     "/tmp/openvpn/openvpn.conf", "--up",
		     "/tmp/openvpn/route-up.sh", "--down",
		     "/tmp/openvpn/route-down.sh", "--daemon");
}

void stop_openvpnserver(void)
{
	stop_process("openvpnserver", "OpenVPN daemon (Server)");
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
	write_nvram("/tmp/openvpncl/ta.key", "openvpncl_tlsauth");
	chmod("/tmp/openvpn/client.key", 0600);

	FILE *fp = fopen("/tmp/openvpncl/openvpn.conf", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "ca /tmp/openvpncl/ca.crt\n");
	fprintf(fp, "cert /tmp/openvpncl/client.crt\n");
	fprintf(fp, "key /tmp/openvpncl/client.key\n");
	fprintf(fp, "management 127.0.0.1 5001\n");
	fprintf(fp, "management-log-cache 50\n");
	fprintf(fp, "verb 4\n");
	fprintf(fp, "mute 5\n");
	fprintf(fp, "log-append /var/log/openvpncl\n");
	fprintf(fp, "client\n");
	fprintf(fp, "tls-client\n");
	fprintf(fp, "dev %s1\n", nvram_safe_get("openvpncl_tuntap"));
	fprintf(fp, "proto %s\n", nvram_safe_get("openvpncl_proto"));
	fprintf(fp, "cipher %s\n", nvram_safe_get("openvpncl_cipher"));
	fprintf(fp, "auth %s\n", nvram_safe_get("openvpncl_auth"));
	fprintf(fp, "resolv-retry infinite\n");
	fprintf(fp, "nobind\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	fprintf(fp, "mtu-disc yes\n");
	fprintf(fp, "remote %s %s\n", nvram_safe_get("openvpncl_remoteip"),
		nvram_safe_get("openvpncl_remoteport"));
	if (nvram_invmatch("openvpncl_mtu", ""))
		fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpncl_mtu"));
	if (nvram_invmatch("openvpncl_mssfix", "")
	    && nvram_match("openvpn_proto", "udp")) {
		fprintf(fp, "mssfix %s\n", nvram_safe_get("openvpncl_mssfix"));	//fragment=mssfix
		fprintf(fp, "fragment %s\n",
			nvram_safe_get("openvpncl_mssfix"));
	}
	// Botho 22/05/2006 - start
	if (nvram_match("openvpncl_certtype", "1"))
		fprintf(fp, "ns-cert-type server\n");
	// end
	if (nvram_match("openvpncl_lzo", "1"))
		fprintf(fp, "comp-lzo yes\n");
	if (nvram_match("openvpncl_proto", "udp"))
		fprintf(fp, "fast-io\n");	//experimental!improving CPU efficiency by 5%-10%
	if (nvram_match("openvpncl_tuntap", "tap"))
		fprintf(fp, "tun-ipv6\n");	//enable ipv6 support. not supported on server in version 2.1.3
	if (strlen(nvram_safe_get("openvpncl_tlsauth")) > 0)
		fprintf(fp, "tls-auth /tmp/openvpncl/ta.key 1\n");
	fprintf(fp, "%s\n", nvram_safe_get("openvpncl_config"));
	fclose(fp);

	fp = fopen("/tmp/openvpncl/route-up.sh", "wb");
	if (fp == NULL)
		return;
	//bridge tap interface to br0 when choosen
	if (nvram_match("openvpncl_bridge", "1")
	    && nvram_match("openvpncl_tuntap", "tap")) {
		fprintf(fp, "brctl addif br0 tap1\n");
	}
	if (nvram_match("openvpncl_nat", "1"))
		fprintf(fp,
			"iptables -I POSTROUTING -t nat -o %s1 -j MASQUERADE\n",
			nvram_safe_get("openvpncl_tuntap"));
	else {
		fprintf(fp, "iptables -I INPUT -i %s1 -j ACCEPT\n",
			nvram_safe_get("openvpncl_tuntap"));
		fprintf(fp, "iptables -I FORWARD -i %s1 -j ACCEPT\n",
			nvram_safe_get("openvpncl_tuntap"));
		fprintf(fp, "iptables -I FORWARD -o %s1 -j ACCEPT\n",
			nvram_safe_get("openvpncl_tuntap"));
	}
	fclose(fp);

	fp = fopen("/tmp/openvpncl/route-down.sh", "wb");
	if (fp == NULL)
		return;
	if (nvram_match("openvpncl_nat", "1"))
		fprintf(fp,
			"iptables -D POSTROUTING -t nat -o %s1 -j MASQUERADE\n",
			nvram_safe_get("openvpncl_tuntap"));
	else {
		fprintf(fp, "iptables -D INPUT -i %s1 -j ACCEPT\n",
			nvram_safe_get("openvpncl_tuntap"));
		fprintf(fp, "iptables -D FORWARD -i %s1 -j ACCEPT\n",
			nvram_safe_get("openvpncl_tuntap"));
		fprintf(fp, "iptables -D FORWARD -o %s1 -j ACCEPT\n",
			nvram_safe_get("openvpncl_tuntap"));
	}
	if (nvram_match("openvpncl_bridge", "1")
	    && nvram_match("openvpncl_tuntap", "tap")) {
		fprintf(fp, "brctl delif br0 tap1\n");
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
	stop_process("openvpn", "OpenVPN daemon (Client)");
}

void stop_openvpn_wandone(void)
{
	if (nvram_invmatch("openvpncl_enable", "1"))
		return;
	stop_process("openvpn", "OpenVPN daemon (Client)");
}

#endif
