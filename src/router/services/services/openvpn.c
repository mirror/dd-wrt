/*
 * openvpn.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <gottschall@dd-wrt.com
 * Copyright (C) 2010 - 2014 Sash
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
#include <sys/stat.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <syslog.h>
#include <services.h>
#include <utils.h>
#ifdef HAVE_OPENVPN
static void run_openvpn(char *prg, char *path)
{
	char *conf;
	asprintf(&conf, "/tmp/%s/openvpn.conf", path);
	char *routeup;
	asprintf(&routeup, "/tmp/%s/route-up.sh", path);
	char *routedown;
	asprintf(&routedown, "/tmp/%s/route-down.sh", path);
	if (nvram_matchi("use_crypto", 1)) {
		insmod("cryptodev");
		eval(prg, "--config", conf, "--route-up", routeup, "--route-pre-down", routedown, "--daemon", "--engine", "cryptodev");
	} else {
		rmmod("cryptodev");
		eval(prg, "--config", conf, "--route-up", routeup, "--route-pre-down", routedown, "--daemon");
	}
	free(routedown);
	free(routeup);
	free(conf);
}

void start_openvpnserver(void)
{
	int jffs = 0;
	char proto[16];
	if (nvram_invmatchi("openvpn_enable", 1))
		return;
	strcpy(proto, nvram_safe_get("openvpn_proto"));
	if (!nvram_matchi("ipv6_enable", 1)) {
		if (!strcmp(proto, "udp")) {
			strcpy(proto, "udp4");
		} else {
			strcpy(proto, "tcp4-server");
		}
	}
	insmod("tun");
	update_timezone();
	if ((freediskSpace("/jffs") > 16384)
	    || (nvram_matchi("enable_jffs2", 1)
		&& nvram_matchi("jffs_mounted", 1)
		&& nvram_matchi("sys_enable_jffs2", 1)))
		jffs = 1;
	dd_loginfo("openvpn", "OpenVPN daemon (Server) starting/restarting...\n");
	mkdir("/tmp/openvpn", 0700);
	mkdir("/tmp/openvpn/ccd", 0700);
	write_nvram("/tmp/openvpn/dh.pem", "openvpn_dh");
	write_nvram("/tmp/openvpn/ca.crt", "openvpn_ca");
	write_nvram("/tmp/openvpn/cert.pem", "openvpn_crt");
	write_nvram("/tmp/openvpn/ca.crl", "openvpn_crl");
	write_nvram("/tmp/openvpn/key.pem", "openvpn_key");
	write_nvram("/tmp/openvpn/ta.key", "openvpn_tlsauth");
	write_nvram("/tmp/openvpn/cert.p12", "openvpn_pkcs12");
	write_nvram("/tmp/openvpn/static.key", "openvpn_static");
	chmod("/tmp/openvpn/key.pem", 0600);
	//      use jffs for ccd if available
	if (jffs == 1) {
		mkdir("/jffs/etc", 0700);
		mkdir("/jffs/etc/openvpn", 0700);
		mkdir("/jffs/etc/openvpn/ccd", 0700);
		if (*(nvram_safe_get("openvpn_ccddef"))) {
			write_nvram("/jffs/etc/openvpn/ccd/DEFAULT", "openvpn_ccddef");
			chmod("/jffs/etc/openvpn/ccd/DEFAULT", 0700);
		}
	} else {
		write_nvram("/tmp/openvpn/ccd/DEFAULT", "openvpn_ccddef");
		chmod("/tmp/openvpn/ccd/DEFAULT", 0700);
	}
	// client connect scripts (!= ccd )
	FILE *fp = fopen("/tmp/openvpn/clcon.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "%s\n", nvram_safe_get("openvpn_clcon"));
	fclose(fp);
	fp = fopen("/tmp/openvpn/cldiscon.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "%s\n", nvram_safe_get("openvpn_cldiscon"));
	fclose(fp);
	chmod("/tmp/openvpn/clcon.sh", 0700);
	chmod("/tmp/openvpn/cldiscon.sh", 0700);
	fp = fopen("/tmp/openvpn/openvpn.conf", "wb");
	if (fp == NULL)
		return;
	if (nvram_invmatch("openvpn_static", ""))
		fprintf(fp, "secret /tmp/openvpn/static.key\n");
	else if (nvram_invmatch("openvpn_pkcs12", "")) {
		fprintf(fp, "dh /tmp/openvpn/dh.pem\n");
		fprintf(fp, "pkcs12 /tmp/openvpn/cert.p12\n");
	} else {
		if (nvram_invmatch("openvpn_dh", ""))
			fprintf(fp, "dh /tmp/openvpn/dh.pem\n");
		if (nvram_invmatch("openvpn_ca", ""))
			fprintf(fp, "ca /tmp/openvpn/ca.crt\n");
		if (nvram_invmatch("openvpn_crt", ""))
			fprintf(fp, "cert /tmp/openvpn/cert.pem\n");
		if (nvram_invmatch("openvpn_key", ""))
			fprintf(fp, "key /tmp/openvpn/key.pem\n");
	}
	//be sure Chris old style config is still working
	if (nvram_matchi("openvpn_switch", 1)) {
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_crt");
		fprintf(fp, "keepalive 10 120\n"
			"verb 3\n" "mute 3\n" "syslog\n"
			"writepid /var/run/openvpnd.pid\n"
			"management 127.0.0.1 14\n"
			"management-log-cache 100\n"
			"topology subnet\n"
			"script-security 2\n" "port %s\n" "proto %s\n" "cipher %s\n" "auth %s\n", nvram_safe_get("openvpn_port"), proto, nvram_safe_get("openvpn_cipher"), nvram_safe_get("openvpn_auth"));
		fprintf(fp, "client-connect /tmp/openvpn/clcon.sh\n");
		fprintf(fp, "client-disconnect /tmp/openvpn/cldiscon.sh\n");
		if (jffs == 1)	//  use usb/jffs for ccd if available
			fprintf(fp, "client-config-dir /jffs/etc/openvpn/ccd\n");
		else
			fprintf(fp, "client-config-dir /tmp/openvpn/ccd\n");
		if (nvram_invmatch("openvpn_scramble", "off"))
			fprintf(fp, "scramble %s\n",	//scramble XOR patch for reordering packet content to protect against DPI
				nvram_safe_get("openvpn_scramble"));
		if (nvram_invmatch("openvpn_lzo", "off")) {
			if (nvram_match("openvpn_lzo", "compress lz4"))
				fprintf(fp, "compress lz4\n");
			else if (nvram_match("openvpn_lzo", "compress lz4-v2"))
				fprintf(fp, "compress lz4-v2\n");
			else if (nvram_match("openvpn_lzo", "compress"))
				fprintf(fp, "compress\n");
			else
				fprintf(fp, "comp-lzo %s\n",	//yes/no/adaptive/disable
					nvram_safe_get("openvpn_lzo"));
		}
		if (nvram_invmatch("openvpn_auth", "none"))	//not needed if we have no auth anyway
			fprintf(fp, "tls-server\n");
		if (nvram_matchi("openvpn_dupcn", 1))
			fprintf(fp, "duplicate-cn\n");
		if (nvram_matchi("openvpn_dupcn", 0)	//keep peer ip persistant for x sec. works only when dupcn=off & no proxy mode
		    && nvram_matchi("openvpn_proxy", 0))
			fprintf(fp, "ifconfig-pool-persist /tmp/openvpn/ip-pool 86400\n");
		if (nvram_matchi("openvpn_cl2cl", 1))
			fprintf(fp, "client-to-client\n");
		if (nvram_matchi("openvpn_redirgate", 1))
			fprintf(fp, "push \"redirect-gateway def1\"\n");
		if (nvram_invmatchi("openvpn_tlscip", 0))
			fprintf(fp, "tls-cipher %s\n", nvram_safe_get("openvpn_tlscip"));
		if (nvram_match("openvpn_proto", "udp"))
			fprintf(fp, "fast-io\n");	//experimental!improving CPU efficiency by 5%-10%
		else		//TCP_NODELAY is generally a good latency optimization
			fprintf(fp, "tcp-nodelay\n");
		if (nvram_invmatch("openvpn_mtu", ""))
			fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpn_mtu"));
		if (nvram_invmatch("openvpn_fragment", "")
		    && nvram_match("openvpn_proto", "udp")) {
			fprintf(fp, "fragment %s\n", nvram_safe_get("openvpn_fragment"));
			if (nvram_matchi("openvpn_mssfix", 1))
				fprintf(fp, "mssfix\n");	//mssfix=1450 (default), should be set on one side only. when fragment->=mss   
		} else
			fprintf(fp, "mtu-disc yes\n");
		if (nvram_match("openvpn_tuntap", "tun")) {
			fprintf(fp, "server %s %s\n", nvram_safe_get("openvpn_net"), nvram_safe_get("openvpn_tunmask"));
			fprintf(fp, "dev tun2\n");
#ifdef HAVE_IPV6
			//fprintf(fp, "tun-ipv6\n");    //enable ipv6 support.
#endif
		} else if (nvram_match("openvpn_tuntap", "tap") && nvram_matchi("openvpn_proxy", 0)) {
			fprintf(fp, "server-bridge %s %s %s %s\n", nvram_safe_get("openvpn_gateway"), nvram_safe_get("openvpn_mask"), nvram_safe_get("openvpn_startip"), nvram_safe_get("openvpn_endip"));
			fprintf(fp, "dev tap2\n");
		} else if (nvram_match("openvpn_tuntap", "tap") && nvram_matchi("openvpn_proxy", 1) && nvram_matchi("openvpn_redirgate", 1))
			fprintf(fp, "server-bridge\n" "dev tap2\n");
		else
			fprintf(fp, "server-bridge nogw\n" "dev tap2\n");
		if (*(nvram_safe_get("openvpn_tlsauth"))) {
			if (nvram_matchi("openvpn_tls_btn", 1))
				fprintf(fp, "tls-crypt /tmp/openvpn/ta.key\n");	//egc: tls_btn 1 is tls-crypt
			else
				fprintf(fp, "tls-auth /tmp/openvpn/ta.key 0\n");
		}
		if (*(nvram_safe_get("openvpn_crl")))
			fprintf(fp, "crl-verify /tmp/openvpn/ca.crl\n");
		/* for QOS */
		if (nvram_matchi("wshaper_enable", 1))
			fprintf(fp, "passtos\n");
	} else
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_client");
	fprintf(fp, "%s\n", nvram_safe_get("openvpn_config"));
	fclose(fp);
	fp = fopen("/tmp/openvpn/route-up.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
#if defined(HAVE_TMK) || defined(HAVE_BKM) || defined(HAVE_UNFY)
	char *gpiovpn = nvram_safe_get("gpiovpn");
	if (*gpiovpn) {
		if (strncmp(gpiovpn, "-", 1))
			fprintf(fp, "gpio enable %s\n", gpiovpn);
		else {
			gpiovpn++;
			fprintf(fp, "gpio disable %s\n", gpiovpn);
		}
	}
#endif
	//bring up tap interface when choosen
	if (nvram_match("openvpn_tuntap", "tap")) {
		fprintf(fp, "brctl addif br0 tap2\n" "ifconfig tap2 0.0.0.0 up\n");	//non promisc for performance reasons
	}
	if (nvram_matchi("wshaper_enable", 1)) {
		fprintf(fp, "startservice set_routes -f\n");
		fprintf(fp, "startservice firewall -f\n");
		fprintf(fp, "startservice wshaper -f\n");
		fprintf(fp, "sleep 5\n");
	}
	if (nvram_matchi("block_multicast", 0)	//block multicast on bridged vpns
	    && nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "insmod ebtables\n" "insmod ebtable_filter\n" "insmod ebtable_nat\n" "insmod ebt_pkttype\n"
			"ebtables -t nat -D POSTROUTING -o tap2 --pkttype-type multicast -j DROP\n" "ebtables -t nat -I POSTROUTING -o tap2 --pkttype-type multicast -j DROP\n");
	if (nvram_matchi("openvpn_dhcpbl", 1)	//block dhcp on bridged vpns
	    && nvram_match("openvpn_tuntap", "tap")
	    && nvram_matchi("openvpn_proxy", 0))
		fprintf(fp, "insmod ebtables\n" "insmod ebt_ip\n" "insmod ebtable_filter\n" "insmod ebtable_nat\n"
			"ebtables -t nat -D PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -I PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -I POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n");
	if (nvram_default_matchi("openvpn_fw", 1, 0)) {
		fprintf(fp, "iptables -I INPUT -i tap2 -m state --state NEW -j DROP\n");
		fprintf(fp, "iptables -I FORWARD -i tap2 -m state --state NEW -j DROP\n");
	}
	fprintf(fp, "iptables -t raw -I PREROUTING \! -i tun2 -d %s -j DROP\n",get_ipfrominterface("tun2", ipbuf));
	/* "stopservice wshaper\n" disable wshaper, causes fw race condition
	 * "startservice wshaper\n");*/
	fclose(fp);
	fp = fopen("/tmp/openvpn/route-down.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
#if defined(HAVE_TMK) || defined(HAVE_BKM) || defined(HAVE_UNFY)
	gpiovpn = nvram_safe_get("gpiovpn");
	if (*gpiovpn) {
		if (strncmp(gpiovpn, "-", 1))
			fprintf(fp, "gpio disable %s\n", gpiovpn);
		else {
			gpiovpn++;
			fprintf(fp, "gpio enable %s\n", gpiovpn);
		}
	}
#endif
	// remove ebtales rules
	if (nvram_matchi("block_multicast", 0)
	    && nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "ebtables -t nat -D POSTROUTING -o tap2 --pkttype-type multicast -j DROP\n");
	if (nvram_matchi("openvpn_dhcpbl", 1)
	    && nvram_match("openvpn_tuntap", "tap")
	    && nvram_matchi("openvpn_proxy", 0))
		fprintf(fp,
			"ebtables -t nat -D PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n");
	if (nvram_default_matchi("openvpn_fw", 1, 0)) {
		fprintf(fp, "iptables -D INPUT -i tap2 -m state --state NEW -j DROP\n");
		fprintf(fp, "iptables -D FORWARD -i tap2 -m state --state NEW -j DROP\n");
	}
	fprintf(fp, "iptables -t raw -D PREROUTING \! -i tun2 -d %s -j DROP\n",get_ipfrominterface("tun2", ipbuf));

/*      if ((nvram_matchi("openvpn_dhcpbl",1)
                        && nvram_match("openvpn_tuntap", "tap")
                        && nvram_matchi("openvpn_proxy",0))
                || (nvram_matchi("block_multicast",0)
                        && nvram_match("openvpn_tuntap", "tap")))
                        fprintf(fp, "if [ `ebtables -t nat -L|grep -e '-j' -c` -eq 0 ]\n"
                                "then rmmod ebtable_nat\n" "\t rmmod ebt_ip\n"
                                "elseif [ `ebtables -t nat -L|grep -e '-j' -c` -eq 0 ]\n"
                                "then rmmod ebtable_filter\n" "\t rmmod ebtables\n");   */
	if (nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "brctl delif br0 tap2\n" "ifconfig tap2 down\n");
	fclose(fp);
	chmod("/tmp/openvpn/route-up.sh", 0700);
	chmod("/tmp/openvpn/route-down.sh", 0700);
	eval("ln", "-s", "/usr/sbin/openvpn", "/tmp/openvpnserver");
	run_openvpn("/tmp/openvpnserver", "openvpn");
//      eval("stopservice", "wshaper"); disable wshaper, causes fw race condition
//      eval("startservice", "wshaper");
}

void stop_openvpnserver(void)
{
#if defined(HAVE_TMK) || defined(HAVE_BKM) || defined(HAVE_UNFY)
	char *gpiovpn = nvram_safe_get("gpiovpn");
	if (*gpiovpn) {
		if (strncmp(gpiovpn, "-", 1))
			set_gpio(atoi(gpiovpn), 0);
		else {
			gpiovpn++;
			set_gpio(atoi(gpiovpn), 1);
		}
	}
#endif
	if (stop_process("openvpnserver", "OpenVPN daemon (Server)")) {
//              eval("stopservice", "wshaper");
//              eval("startservice", "wshaper");
		//remove ebtables rules on shutdown     
		system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap2 --pkttype-type multicast -j DROP");
		system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP");
		system("/usr/sbin/ebtables -t nat -D PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP");
		unlink("/tmp/openvpn/ccd/DEFAULT");
		unlink("/tmp/openvpn/dh.pem");
		unlink("/tmp/openvpn/ca.crt");
		unlink("/tmp/openvpn/cert.pem");
		unlink("/tmp/openvpn/ca.crl");
		unlink("/tmp/openvpn/key.pem");
		unlink("/tmp/openvpn/ta.key");
		unlink("/tmp/openvpn/cert.p12");
		unlink("/tmp/openvpn/static.key");
		unlink("/tmp/openvpn/openvpn.conf");
		unlink("/tmp/openvpn/route-up.sh");
		unlink("/tmp/openvpn/route-down.sh");
	}
	return;
}

void start_openvpnserverwan(void)
{
	if (nvram_matchi("openvpn_onwan", 1))
		start_openvpnserver();
	return;
}

void stop_openvpnserverwan(void)
{
	if (nvram_matchi("openvpn_onwan", 1))
		stop_openvpnserver();
	return;
}

void start_openvpnserversys(void)
{
	if (nvram_matchi("openvpn_onwan", 0))
		start_openvpnserver();
	return;
}

void stop_openvpnserversys(void)
{
	if (nvram_matchi("openvpn_onwan", 0))
		stop_openvpnserver();
	return;
}

void start_openvpn(void)
{
	if (nvram_invmatchi("openvpncl_enable", 1))
		return;
	insmod("tun");
	dd_loginfo("openvpn", "OpenVPN daemon (Client) starting/restarting...\n");
	mkdir("/tmp/openvpncl", 0700);
	write_nvram("/tmp/openvpncl/ca.crt", "openvpncl_ca");
	write_nvram("/tmp/openvpncl/client.crt", "openvpncl_client");
	write_nvram("/tmp/openvpncl/client.key", "openvpncl_key");
	write_nvram("/tmp/openvpncl/ta.key", "openvpncl_tlsauth");
	write_nvram("/tmp/openvpncl/cert.p12", "openvpncl_pkcs12");
	write_nvram("/tmp/openvpncl/static.key", "openvpncl_static");
	chmod("/tmp/openvpn/client.key", 0600);
	char proto[16];
	strcpy(proto, nvram_safe_get("openvpncl_proto"));
	if (!nvram_matchi("ipv6_enable", 1)) {
		if (!strcmp(proto, "udp")) {
			strcpy(proto, "udp4");
		} else {
			strcpy(proto, "tcp4-client");
		}
	}
	FILE *fp;
	char ovpniface[10];
#ifdef HAVE_ERC
	sprintf(ovpniface, "%s0", nvram_safe_get("openvpncl_tuntap"));
#else
	sprintf(ovpniface, "%s1", nvram_safe_get("openvpncl_tuntap"));
#endif
	if (nvram_matchi("openvpncl_upauth", 1)) {
		fp = fopen("/tmp/openvpncl/credentials", "wb");
		fprintf(fp, "%s\n", nvram_safe_get("openvpncl_user"));
		fprintf(fp, "%s\n", nvram_safe_get("openvpncl_pass"));
		fclose(fp);
	}
	fp = fopen("/tmp/openvpncl/openvpn.conf", "wb");
	if (fp == NULL)
		return;
	if (nvram_invmatch("openvpncl_static", ""))
		fprintf(fp, "secret /tmp/openvpncl/static.key\n");
	else if (nvram_invmatch("openvpncl_pkcs12", "")) {;
		fprintf(fp, "pkcs12 /tmp/openvpncl/cert.p12\n");
	} else {
		if (nvram_invmatch("openvpncl_ca", ""))
			fprintf(fp, "ca /tmp/openvpncl/ca.crt\n");
		if (nvram_invmatch("openvpncl_client", ""))
			fprintf(fp, "cert /tmp/openvpncl/client.crt\n");
		if (nvram_invmatch("openvpncl_key", ""))
			fprintf(fp, "key /tmp/openvpncl/client.key\n");
	}
#ifdef HAVE_ERC
	fprintf(fp,
		"management 127.0.0.1 5001\n"
		"management-log-cache 100\n" "verb 3\n" "mute 3\n" "syslog\n" "writepid /var/run/openvpncl.pid\n" "client\n" "resolv-retry infinite\n" "nobind\n" "persist-key\n" "persist-tun\n" "script-security 2\n");
#else
	fprintf(fp,
		"management 127.0.0.1 16\n"
		"management-log-cache 100\n" "verb 3\n" "mute 3\n" "syslog\n" "writepid /var/run/openvpncl.pid\n" "client\n" "resolv-retry infinite\n" "nobind\n" "persist-key\n" "persist-tun\n" "script-security 2\n");
#endif
	fprintf(fp, "dev %s\n", ovpniface);
	fprintf(fp, "proto %s\n", proto);
	fprintf(fp, "cipher %s\n", nvram_safe_get("openvpncl_cipher"));
	fprintf(fp, "auth %s\n", nvram_safe_get("openvpncl_auth"));
	if (nvram_matchi("openvpncl_upauth", 1))
		fprintf(fp, "auth-user-pass %s\n", "/tmp/openvpncl/credentials");
	fprintf(fp, "remote %s %s\n", nvram_safe_get("openvpncl_remoteip"), nvram_safe_get("openvpncl_remoteport"));
	if (nvram_invmatch("openvpncl_scramble", "off")) {
		if (nvram_match("openvpncl_scramble", "obfuscate"))
			fprintf(fp, "scramble %s %s\n", nvram_safe_get("openvpncl_scramble"), nvram_safe_get("openvpncl_scrmblpw"));
		else
			fprintf(fp, "scramble %s\n", nvram_safe_get("openvpncl_scramble"));
	}
	if (nvram_invmatch("openvpncl_lzo", "off")) {
		if (nvram_match("openvpncl_lzo", "compress lz4"))
			fprintf(fp, "compress lz4\n");
		else if (nvram_match("openvpncl_lzo", "compress lz4-v2"))
			fprintf(fp, "compress lz4-v2\n");
		else if (nvram_match("openvpncl_lzo", "compress"))
			fprintf(fp, "compress\n");
		else
			fprintf(fp, "comp-lzo %s\n",	//yes/no/adaptive/disable
				nvram_safe_get("openvpncl_lzo"));
	}
	if (*(nvram_safe_get("openvpncl_route"))) {	//policy routing: we need redirect-gw so we get gw info
		fprintf(fp, "redirect-private def1\n");
		if (nvram_invmatch("openvpncl_tuntap", "tun"))
			fprintf(fp, "ifconfig-noexec\n");
		else
			fprintf(fp, "pull-filter ignore \"redirect-gateway\"\n");
	}
	if (nvram_invmatch("openvpncl_auth", "none") && nvram_invmatchi("openvpncl_tlscip", 0))	//not needed if we have no auth anyway
		fprintf(fp, "tls-client\n");
	if (nvram_invmatch("openvpncl_mtu", ""))
		fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpncl_mtu"));
	if (nvram_invmatch("openvpncl_fragment", "")
	    && nvram_match("openvpncl_proto", "udp")) {
		fprintf(fp, "fragment %s\n", nvram_safe_get("openvpncl_fragment"));
		if (nvram_matchi("openvpncl_mssfix", 1))
			fprintf(fp, "mssfix\n");	//mssfix=1450 (default), should be set on one side only. when fragment->=mss   
	} else
		fprintf(fp, "mtu-disc yes\n");
	if (nvram_matchi("openvpncl_certtype", 1))
//                fprintf(fp, "ns-cert-type server\n"); //egc: ns-cert-type deprecated and replaced by remote-cert-tls
		fprintf(fp, "remote-cert-tls server\n");
	if (nvram_match("openvpncl_proto", "udp"))
		fprintf(fp, "fast-io\n");	//experimental!improving CPU efficiency by 5%-10%
//      if (nvram_match("openvpncl_tuntap", "tun"))
//              fprintf(fp, "tun-ipv6\n");      //enable ipv6 support.
	if (*(nvram_safe_get("openvpncl_tlsauth"))) {
		if (nvram_matchi("openvpncl_tls_btn", 1))
			fprintf(fp, "tls-crypt /tmp/openvpncl/ta.key\n");	//egc: tls_btn 1 is tls-crypt
		else
			fprintf(fp, "tls-auth /tmp/openvpncl/ta.key 1\n");
	}
	if (nvram_invmatchi("openvpncl_tlscip", 0))
		fprintf(fp, "tls-cipher %s\n", nvram_safe_get("openvpncl_tlscip"));
	/* for QOS */
	if (nvram_matchi("wshaper_enable", 1))
		fprintf(fp, "passtos\n");
	fprintf(fp, "%s\n", nvram_safe_get("openvpncl_config"));
	fclose(fp);
	fp = fopen("/tmp/openvpncl/route-up.sh", "wb");
	if (fp == NULL) {
		return;
	}
	fprintf(fp, "#!/bin/sh\n");
#if defined(HAVE_TMK) || defined(HAVE_BKM) || defined(HAVE_UNFY)
	char *gpiovpn = nvram_safe_get("gpiovpn");
	if (*gpiovpn) {
		if (strncmp(gpiovpn, "-", 1))
			fprintf(fp, "gpio enable %s\n", gpiovpn);
		else {
			gpiovpn++;
			fprintf(fp, "gpio disable %s\n", gpiovpn);
		}
	}
#endif
	//bridge tap interface to br0 when choosen
	if (nvram_match("openvpncl_tuntap", "tap")
	    && nvram_matchi("openvpncl_bridge", 1)
	    && nvram_matchi("openvpncl_nat", 0)) {
		fprintf(fp, "brctl addif br0 %s\n" "ifconfig %s 0.0.0.0 up\n", ovpniface, ovpniface);	//non promisc for performance reasons
		if (nvram_match("openvpncl_tuntap", "tap")
		    && *(nvram_safe_get("openvpncl_ip")))
			fprintf(fp, "ifconfig %s %s netmask %s up\n", ovpniface, nvram_safe_get("openvpncl_ip"), nvram_safe_get("openvpncl_mask"));
	}
	if (nvram_matchi("wshaper_enable", 1)) {
		fprintf(fp, "startservice set_routes -f\n");
		fprintf(fp, "startservice firewall -f\n");
		fprintf(fp, "startservice wshaper -f\n");
		fprintf(fp, "sleep 5\n");
	}
	if (nvram_matchi("openvpncl_nat", 1))
		fprintf(fp, "iptables -D POSTROUTING -t nat -o %s -j MASQUERADE\n" "iptables -I POSTROUTING -t nat -o %s -j MASQUERADE\n", ovpniface, ovpniface);
	if (nvram_matchi("openvpncl_sec", 0))
		fprintf(fp, "iptables -D INPUT -i %s -j ACCEPT\n" "iptables -I INPUT -i %s -j ACCEPT\n", ovpniface, ovpniface);
	else {
		if (nvram_match("openvpncl_tuntap", "tun"))	//only needed with tun
			fprintf(fp,
				"iptables -D INPUT -i %s -j ACCEPT\n" //
				"iptables -D FORWARD -i %s -j ACCEPT\n" //
				"iptables -D FORWARD -o %s -j ACCEPT\n" //
				"iptables -I INPUT -i %s -j ACCEPT\n" //
				"iptables -I FORWARD -i %s -j ACCEPT\n" //
				"iptables -I FORWARD -o %s -j ACCEPT\n", ovpniface, ovpniface, ovpniface, ovpniface, ovpniface, ovpniface);
	}
	char ipbuf[128];
	fprintf(fp, "iptables -t raw -I PREROUTING \! -i %s -d %s -j DROP\n",ovpniface, get_ipfrominterface(ovpniface, ipbuf));
	if (nvram_match("openvpncl_tuntap", "tun")) {
		fprintf(fp, "cat /tmp/resolv.dnsmasq > /tmp/resolv.dnsmasq_isp\n");
		fprintf(fp, "env | grep 'dhcp-option DNS' | awk '{ print \"nameserver \" $3 }' > /tmp/resolv.dnsmasq\n");
		fprintf(fp, "cat /tmp/resolv.dnsmasq_isp >> /tmp/resolv.dnsmasq\n");
	}
	if (nvram_default_matchi("openvpncl_fw", 1, 0)) {
		fprintf(fp, "iptables -I INPUT -i %s -m state --state NEW -j DROP\n", ovpniface);
		fprintf(fp, "iptables -I FORWARD -i %s -m state --state NEW -j DROP\n", ovpniface);
	}
	if (*(nvram_safe_get("openvpncl_route"))) {	//policy based routing
		write_nvram("/tmp/openvpncl/policy_ips", "openvpncl_route");
//              fprintf(fp, "ip route flush table 10\n");
//              fprintf(fp, "for IP in `cat /tmp/openvpncl/policy_ips` ; do\n" "\t ip rule add from $IP table 10\n" "done\n"); //egc: deleted and replaced by next line
		fprintf(fp, "sed '/^[[:blank:]]*#/d;s/#.*//' \"/tmp/openvpncl/policy_ips\" | while read IP; do ip rule add table 10 from $IP; done\n");
/*              if (nvram_match("openvpncl_tuntap", "tap"))
                        fprintf(fp, "ip route add default via $route_vpn_gateway table 10\n"); //needs investigation cause in TAP mode no gateway is received
                else */
		if (!nvram_match("openvpncl_tuntap", "tap")) {
			fprintf(fp, "ip route add default via $route_vpn_gateway table 10\n");
			// Consider adding local routes to alternate PBR routing table by egc
			fprintf(fp, "ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do\n" "\t ip route add $route table 10\n" "done\n");
		}
		fprintf(fp, "ip route flush cache\n" "echo $ifconfig_remote >>/tmp/gateway.txt\n" "echo $route_vpn_gateway >>/tmp/gateway.txt\n" "echo $ifconfig_local >>/tmp/gateway.txt\n");
	}
	if (nvram_matchi("block_multicast", 0)	//block multicast on bridged vpns, when wan multicast is enabled
	    && nvram_match("openvpncl_tuntap", "tap")
	    && nvram_matchi("openvpncl_bridge", 1)) {
		fprintf(fp, "insmod ebtables\n" "insmod ebtable_filter\n" "insmod ebtable_nat\n" "insmod ebt_pkttype\n"
//                      "ebtables -I FORWARD -o tap1 --pkttype-type multicast -j DROP\n"
//                      "ebtables -I OUTPUT -o tap1 --pkttype-type multicast -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o %s --pkttype-type multicast -j DROP\n" "ebtables -t nat -I POSTROUTING -o %s --pkttype-type multicast -j DROP\n", ovpniface, ovpniface);
	}
	fclose(fp);
	fp = fopen("/tmp/openvpncl/route-down.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
#if defined(HAVE_TMK) || defined(HAVE_BKM) || defined(HAVE_UNFY)
	if (*gpiovpn)
		if (strncmp(gpiovpn, "-", 1))
			fprintf(fp, "gpio enable %s\n", gpiovpn);
		else {
			gpiovpn++;
			fprintf(fp, "gpio disable %s\n", gpiovpn);
		}
#endif
	if (nvram_match("openvpncl_tuntap", "tap")
	    && nvram_matchi("openvpncl_bridge", 1)
	    && nvram_matchi("openvpncl_nat", 0))
		fprintf(fp, "brctl delif br0 %s\n" "ifconfig %s down\n", ovpniface, ovpniface);
	else if (nvram_match("openvpncl_tuntap", "tap")
		 && *(nvram_safe_get("openvpncl_ip")))
		fprintf(fp, "ifconfig %s down\n", ovpniface);
	if (nvram_matchi("openvpncl_nat", 1))
		fprintf(fp, "iptables -D INPUT -i %s -j ACCEPT\n" "iptables -D POSTROUTING -t nat -o %s -j MASQUERADE\n", ovpniface, ovpniface);
	else {
		fprintf(fp, "iptables -D INPUT -i %s -j ACCEPT\n" "iptables -D FORWARD -i %s -j ACCEPT\n" "iptables -D FORWARD -o %s -j ACCEPT\n", ovpniface, ovpniface, ovpniface);
	}
	if (nvram_default_matchi("openvpncl_fw", 1, 0)) {
		fprintf(fp, "iptables -D INPUT -i %s -m state --state NEW -j DROP\n", ovpniface);
		fprintf(fp, "iptables -D FORWARD -i %s -m state --state NEW -j DROP\n", ovpniface);
	}
	if (*(nvram_safe_get("openvpncl_route"))) {	//policy based routing
		write_nvram("/tmp/openvpncl/policy_ips", "openvpncl_route");
		fprintf(fp, "ip route flush table 10\n");
		fprintf(fp, "while ip rule delete from 0/0 to 0/0 table 10; do true; done\n");	//egc: added to delete ip rules
	}
	if (nvram_match("openvpncl_tuntap", "tun")) {
		fprintf(fp, "[ -f /tmp/resolv.dnsmasq_isp ] && mv -f /tmp/resolv.dnsmasq_isp /tmp/resolv.dnsmasq\n");
	}
	fprintf(fp, "iptables -t raw -D PREROUTING \! -i %s -d %s -j DROP\n",ovpniface, get_ipfrominterface(ovpniface, ipbuf));
/*      if (nvram_matchi("block_multicast",0) //block multicast on bridged vpns
                && nvram_match("openvpncl_tuntap", "tap")
                && nvram_matchi("openvpncl_bridge",1)) {
                fprintf(fp,
-                       "ebtables -t nat -D POSTROUTING -o %s --pkttype-type multicast -j DROP\n"
-                       "ebtables -t nat -D PREROUTING -i %s --pkttype-type multicast -j DROP\n"
                        "if [ `ebtables -t nat -L|grep -e '-j' -c` -ne 0 ]\n"
                        "then rmmod ebtable_nat\n" "\t rmmod ebtables\n", ovpniface, ovpniface);
                } */
	fclose(fp);
	chmod("/tmp/openvpncl/route-up.sh", 0700);
	chmod("/tmp/openvpncl/route-down.sh", 0700);
	run_openvpn("openvpn", "openvpncl");
	return;
}

void stop_openvpn(void)
{
	if (stop_process("openvpn", "OpenVPN daemon (Client)")) {
/*              if (nvram_matchi("wshaper_enable",1)) {disable wshaper, causes fw race condition
                        stop_wshaper();
                        start_wshaper();
                }*/
		//remove ebtables rules on shutdown
		system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP");
		unlink("/tmp/openvpncl/ca.crt");
		unlink("/tmp/openvpncl/client.crt");
		unlink("/tmp/openvpncl/client.key");
		unlink("/tmp/openvpncl/ta.key");
		unlink("/tmp/openvpncl/cert.p12");
		unlink("/tmp/openvpncl/static.key");
		unlink("/tmp/openvpncl/openvpn.conf");
		unlink("/tmp/openvpncl/route-up.sh");
		unlink("/tmp/openvpncl/route-down.sh");
	}
}

void stop_openvpn_wandone(void)
{
#if defined(HAVE_TMK) || defined(HAVE_BKM) || defined(HAVE_UNFY)
	char *gpiovpn = nvram_safe_get("gpiovpn");
	if (*gpiovpn) {
		if (strncmp(gpiovpn, "-", 1))
			set_gpio(atoi(gpiovpn), 0);
		else {
			gpiovpn++;
			set_gpio(atoi(gpiovpn), 1);
		}
	}
#endif
	if (nvram_invmatchi("openvpncl_enable", 1))
		return;
	if (stop_process("openvpn", "OpenVPN daemon (Client)")) {
/*              if (nvram_matchi("wshaper_enable",1)) {disable wshaper, causes fw race condition
                        stop_wshaper();
                        start_wshaper();
                }*/
		//remove ebtables rules on shutdown     
		system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP");
		unlink("/tmp/openvpncl/ca.crt");
		unlink("/tmp/openvpncl/client.crt");
		unlink("/tmp/openvpncl/client.key");
		unlink("/tmp/openvpncl/ta.key");
		unlink("/tmp/openvpncl/cert.p12");
		unlink("/tmp/openvpncl/static.key");
		unlink("/tmp/openvpncl/openvpn.conf");
		unlink("/tmp/openvpncl/route-up.sh");
		unlink("/tmp/openvpncl/route-down.sh");
	}
}
#endif
