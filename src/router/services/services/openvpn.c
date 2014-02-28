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
#include <bcmutils.h>
#include <shutils.h>
#include <syslog.h>
#include <services.h>
#include <utils.h>

#ifdef HAVE_OPENVPN

void start_openvpnserver(void)
{
	int jffs = 0;

	if (nvram_invmatch("openvpn_enable", "1"))
		return;
		
	if ((nvram_match("usb_enable", "1")
		&& nvram_match("usb_storage", "1")
		&& nvram_match("usb_automnt", "1")
		&& nvram_match("usb_mntpoint", "jffs"))
	    || (nvram_match("enable_jffs2", "1")
		&& nvram_match("jffs_mounted", "1")
		&& nvram_match("sys_enable_jffs2", "1")))
			jffs = 1;
		
	dd_syslog(LOG_INFO, "openvpn : OpenVPN daemon (Server) starting/restarting...\n");
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

	//	use jffs for ccd if available
	if (jffs == 1) {
		mkdir("/jffs/etc", 0700);
		mkdir("/jffs/etc/openvpn", 0700);
		mkdir("/jffs/etc/openvpn/ccd", 0700);
		if (strlen(nvram_safe_get("openvpn_ccddef")) > 0) {
			write_nvram("/jffs/etc/openvpn/ccd/DEFAULT", "openvpn_ccddef");
			chmod("/jffs/etc/openvpn/ccd/DEFAULT", 0700);
			}
//                      if (strlen(nvram_safe_get("openvpn_clcon")) > 0) 
//                              write_nvram("/jffs/etc/openvpn/clcon.sh", "openvpn_clcon");
//                      if (strlen(nvram_safe_get("openvpn_cldiscon")) > 0) 
//                              write_nvram("/jffs/etc/openvpn/cldiscon.sh", "openvpn_cldiscon");
	} else {
		write_nvram("/tmp/openvpn/ccd/DEFAULT", "openvpn_ccddef");
//              write_nvram("/tmp/openvpn/clcon.sh", "openvpn_clcon");
//              write_nvram("/tmp/openvpn/cldiscon.sh", "openvpn_cldiscon");
		chmod("/tmp/openvpn/ccd/DEFAULT", 0700);
	}

/*	fp = fopen("/jffs/etc/openvpn/clcon.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
	fclose(fp)
	
	fp = fopen("/jffs/etc/openvpn/cldiscon.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
	fclose(fp)
	
	chmod("/jffs/etc/openvpn/clcon.sh", 0700);
	chmod("/jffs/etc/openvpn/cldiscon.sh", 0700); */

	FILE *fp = fopen("/tmp/openvpn/openvpn.conf", "wb");
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
	if (nvram_match("openvpn_switch", "1")) {
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_crt");
		fprintf(fp, "keepalive 10 120\n"
			"verb 3\n" "mute 3\n" "syslog\n"
			"writepid /var/run/openvpnd.pid\n"
			"management 127.0.0.1 14\n"
			"management-log-cache 100\n"
			"topology subnet\n"
			"script-security 2\n"
			"port %s\n" "proto %s\n" "cipher %s\n" "auth %s\n", nvram_safe_get("openvpn_port"), nvram_safe_get("openvpn_proto"), nvram_safe_get("openvpn_cipher"), nvram_safe_get("openvpn_auth"));
		if (jffs == 1) {	//  use usb/jffs for ccd if available
			fprintf(fp, "client-config-dir /jffs/etc/openvpn/ccd\n");
//                              fprintf(fp, "client-connect /jffs/etc/openvpn/clcon.sh\n");
//                              fprintf(fp, "client-disconnect /jffs/etc/openvpn/cldiscon.sh\n");
		} else {
			fprintf(fp, "client-config-dir /tmp/openvpn/ccd\n");
//                      fprintf(fp, "client-connect /tmp/openvpn/clcon.sh\n");
//                      fprintf(fp, "client-disconnect /tmp/openvpn/cldiscon.sh\n");
		}
		if (nvram_invmatch("openvpn_lzo", "off"))
			fprintf(fp, "comp-lzo %s\n",	//yes/no/adaptive/disable 
				nvram_safe_get("openvpn_lzo"));
		if (nvram_invmatch("openvpn_auth", "none"))	//not needed if we have no auth anyway
			fprintf(fp, "tls-server\n");
		if (nvram_match("openvpn_dupcn", "1"))
			fprintf(fp, "duplicate-cn\n");
		if (nvram_match("openvpn_dupcn", "0")	//keep peer ip persistant for x sec. works only when dupcn=off & no proxy mode
		    && nvram_match("openvpn_proxy", "0"))
			fprintf(fp, "ifconfig-pool-persist /tmp/openvpn/ip-pool 86400\n");
		if (nvram_match("openvpn_cl2cl", "1"))
			fprintf(fp, "client-to-client\n");
		if (nvram_match("openvpn_redirgate", "1"))
			fprintf(fp, "push \"redirect-gateway def1\"\n");
		if (nvram_invmatch("openvpn_tlscip", "0"))
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
			if (nvram_match("openvpn_mssfix", "1"))
				fprintf(fp, "mssfix\n");	//mssfix=1450 (default), should be set on one side only. when fragment->=mss    
		} else
			fprintf(fp, "mtu-disc yes\n");
		if (nvram_match("openvpn_tuntap", "tun")) {
			fprintf(fp, "server %s %s\n", nvram_safe_get("openvpn_net"), nvram_safe_get("openvpn_tunmask"));
			fprintf(fp, "dev tun2\n");
			fprintf(fp, "tun-ipv6\n");	//enable ipv6 support.
		} else if (nvram_match("openvpn_tuntap", "tap") && nvram_match("openvpn_proxy", "0")) {
			fprintf(fp, "server-bridge %s %s %s %s\n", nvram_safe_get("openvpn_gateway"), nvram_safe_get("openvpn_mask"), nvram_safe_get("openvpn_startip"), nvram_safe_get("openvpn_endip"));
			fprintf(fp, "dev tap2\n");
		} else if (nvram_match("openvpn_tuntap", "tap") && nvram_match("openvpn_proxy", "1") && nvram_match("openvpn_redirgate", "1"))
			fprintf(fp, "server-bridge\n" "dev tap2\n");
		else
			fprintf(fp, "server-bridge nogw\n" "dev tap2\n");
		if (strlen(nvram_safe_get("openvpn_tlsauth")) > 0)
			fprintf(fp, "tls-auth /tmp/openvpn/ta.key 0\n");
		if (strlen(nvram_safe_get("openvpn_crl")) > 0)
			fprintf(fp, "crl-verify /tmp/openvpn/ca.crl\n");
		/* for QOS */
		if (nvram_invmatch("wshaper_enable", "0"))
			fprintf(fp, "passtos\n");
	} else
		write_nvram("/tmp/openvpn/cert.pem", "openvpn_client");

	fprintf(fp, "%s\n", nvram_safe_get("openvpn_config"));
	fclose(fp);

	fp = fopen("/tmp/openvpn/route-up.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	char *gpiovpn = nvram_get("gpiovpn");
	if (gpiovpn != NULL) {
		fprintf(fp, "gpio enable %s\n", gpiovpn);
	}
#endif
	//bring up tap interface when choosen
	if (nvram_match("openvpn_tuntap", "tap")) {
		fprintf(fp, "brctl addif br0 tap2\n"
			"ifconfig tap2 0.0.0.0 up\n"); //non promisc for performance reasons
	}
	if (nvram_match("block_multicast", "0")	//block multicast on bridged vpns
	    && nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "insmod ebtables\n" "insmod ebtable_filter\n" "insmod ebtable_nat\n" "insmod ebt_pkttype\n"
/*			"ebtables -D FORWARD -o tap2 --pkttype-type multicast -j DROP\n"
			"ebtables -D OUTPUT -o tap2 --pkttype-type multicast -j DROP\n"
			"ebtables -A FORWARD -o tap2 --pkttype-type multicast -j DROP\n"
			"ebtables -A OUTPUT -o tap2 --pkttype-type multicast -j DROP\n");	*/
			"ebtables -t nat -D POSTROUTING -o tap2 --pkttype-type multicast -j DROP\n" 
			"ebtables -t nat -I POSTROUTING -o tap2 --pkttype-type multicast -j DROP\n");

	if (nvram_match("openvpn_dhcpbl", "1")	//block dhcp on bridged vpns
	    && nvram_match("openvpn_tuntap", "tap")
	    && nvram_match("openvpn_proxy", "0"))
		fprintf(fp, "insmod ebtables\n" "insmod ebt_ip\n" "insmod ebtable_filter\n" "insmod ebtable_nat\n"
/*			"ebtables -D INPUT -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -D FORWARD -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -D FORWARD -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -I INPUT -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -I FORWARD -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -I FORWARD -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"	*/
			"ebtables -t nat -D PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -I PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -I POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n");
	fprintf(fp, "startservice set_routes\n" 
			"stopservice wshaper\n"
			"startservice wshaper\n");
	fclose(fp);

	fp = fopen("/tmp/openvpn/route-down.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	if (gpiovpn != NULL)
		fprintf(fp, "gpio disable %s\n", gpiovpn);
#endif
	// remove ebtales rules
	if (nvram_match("block_multicast", "0")
	    && nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "ebtables -t nat -D POSTROUTING -o tap2 --pkttype-type multicast -j DROP\n");
	if (nvram_match("openvpn_dhcpbl", "1")
	    && nvram_match("openvpn_tuntap", "tap")
	    && nvram_match("openvpn_proxy", "0"))
		fprintf(fp,
			"ebtables -t nat -D PREROUTING -i tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o tap2 -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n");
/*	if ((nvram_match("openvpn_dhcpbl", "1")
			&& nvram_match("openvpn_tuntap", "tap")
			&& nvram_match("openvpn_proxy", "0"))
		|| (nvram_match("block_multicast", "0")
			&& nvram_match("openvpn_tuntap", "tap")))
			fprintf(fp, "if [ `ebtables -t nat -L|grep -e '-j' -c` -eq 0 ]\n"
				"then rmmod ebtable_nat\n" "\t rmmod ebt_ip\n" 
				"elseif [ `ebtables -t nat -L|grep -e '-j' -c` -eq 0 ]\n"
				"then rmmod ebtable_filter\n" "\t rmmod ebtables\n");	*/

	if (nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "brctl delif br0 tap2\n" "ifconfig tap2 down\n");
	fclose(fp);

	chmod("/tmp/openvpn/route-up.sh", 0700);
	chmod("/tmp/openvpn/route-down.sh", 0700);
	eval("ln", "-s", "/usr/sbin/openvpn", "/tmp/openvpnserver");

	if (nvram_match("use_crypto", "1"))
		eval("/tmp/openvpnserver", "--config", "/tmp/openvpn/openvpn.conf", "--route-up", "/tmp/openvpn/route-up.sh", "--down-pre", "/tmp/openvpn/route-down.sh", "--daemon", "--engine", "cryptodev");
	else
		eval("/tmp/openvpnserver", "--config", "/tmp/openvpn/openvpn.conf", "--route-up", "/tmp/openvpn/route-up.sh", "--down-pre", "/tmp/openvpn/route-down.sh", "--daemon");

	eval("stopservice", "wshaper");
	eval("startservice", "wshaper");
}

void stop_openvpnserver(void)
{
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	char *gpiovpn = nvram_get("gpiovpn");
	if (gpiovpn != NULL) {
		set_gpio(atoi(gpiovpn), 0);
	}
#endif
	if (stop_process("openvpnserver", "OpenVPN daemon (Server)")) {
		eval("stopservice", "wshaper");
		eval("startservice", "wshaper");
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
	dd_syslog(LOG_INFO, "openvpn : OpenVPN daemon (Client) starting/restarting...\n");
	mkdir("/tmp/openvpncl", 0700);
	write_nvram("/tmp/openvpncl/ca.crt", "openvpncl_ca");
	write_nvram("/tmp/openvpncl/client.crt", "openvpncl_client");
	write_nvram("/tmp/openvpncl/client.key", "openvpncl_key");
	write_nvram("/tmp/openvpncl/ta.key", "openvpncl_tlsauth");
	write_nvram("/tmp/openvpncl/cert.p12", "openvpncl_pkcs12");
	write_nvram("/tmp/openvpncl/static.key", "openvpncl_static");
	chmod("/tmp/openvpn/client.key", 0600);

	FILE *fp = fopen("/tmp/openvpncl/openvpn.conf", "wb");
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
	fprintf(fp,
		"management 127.0.0.1 16\n"
		"management-log-cache 100\n" "verb 3\n" "mute 3\n" "syslog\n" 
		"writepid /var/run/openvpncl.pid\n" "client\n" 
		"resolv-retry infinite\n" "nobind\n" "persist-key\n" 
		"persist-tun\n" "script-security 2\n");
	fprintf(fp, "dev %s1\n", nvram_safe_get("openvpncl_tuntap"));
	fprintf(fp, "proto %s\n", nvram_safe_get("openvpncl_proto"));
	fprintf(fp, "cipher %s\n", nvram_safe_get("openvpncl_cipher"));
	fprintf(fp, "auth %s\n", nvram_safe_get("openvpncl_auth"));
	fprintf(fp, "remote %s %s\n", nvram_safe_get("openvpncl_remoteip"), nvram_safe_get("openvpncl_remoteport"));
	if (nvram_invmatch("openvpncl_lzo", "off"))
		fprintf(fp, "comp-lzo %s\n",	//yes/no/adaptive/disable 
			nvram_safe_get("openvpncl_lzo"));
	if (strlen(nvram_safe_get("openvpncl_route")) > 0) {	//policy routing: we need redirect-gw so we get gw info
		fprintf(fp, "redirect-private def1\n");
		if (nvram_invmatch("openvpncl_tuntap", "tun"))
			fprintf(fp, "ifconfig-noexec\n");
		else
			fprintf(fp, "route-noexec\n");
	}
	if (nvram_invmatch("openvpncl_auth", "none"))	//not needed if we have no auth anyway
		fprintf(fp, "tls-client\n");
	if (nvram_invmatch("openvpncl_mtu", ""))
		fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpncl_mtu"));
	if (nvram_invmatch("openvpncl_fragment", "")
	    && nvram_match("openvpncl_proto", "udp")) {
		fprintf(fp, "fragment %s\n", nvram_safe_get("openvpncl_fragment"));
		if (nvram_match("openvpncl_mssfix", "1"))
			fprintf(fp, "mssfix\n");	//mssfix=1450 (default), should be set on one side only. when fragment->=mss    
	} else
		fprintf(fp, "mtu-disc yes\n");
	if (nvram_match("openvpncl_certtype", "1"))
		fprintf(fp, "ns-cert-type server\n");
	if (nvram_match("openvpncl_proto", "udp"))
		fprintf(fp, "fast-io\n");	//experimental!improving CPU efficiency by 5%-10%
	if (nvram_match("openvpncl_tuntap", "tun"))
		fprintf(fp, "tun-ipv6\n");	//enable ipv6 support.
	if (strlen(nvram_safe_get("openvpncl_tlsauth")) > 0)
		fprintf(fp, "tls-auth /tmp/openvpncl/ta.key 1\n");
	if (nvram_invmatch("openvpncl_tlscip", "0"))
		fprintf(fp, "tls-cipher %s\n", nvram_safe_get("openvpncl_tlscip"));
	/* for QOS */
	if (nvram_invmatch("wshaper_enable", "0"))
		fprintf(fp, "passtos\n");

	fprintf(fp, "%s\n", nvram_safe_get("openvpncl_config"));
	fclose(fp);
	fp = fopen("/tmp/openvpncl/route-up.sh", "wb");
	if (fp == NULL) {
		return;
	}
	fprintf(fp, "#!/bin/sh\n");
	//bridge tap interface to br0 when choosen
	if (nvram_match("openvpncl_tuntap", "tap")
	    && nvram_match("openvpncl_bridge", "1")
	    && nvram_match("openvpncl_nat", "0")) {
		fprintf(fp, "brctl addif br0 tap1\n"
			"ifconfig tap1 0.0.0.0 up\n"); //non promisc for performance reasons
	} else {
		 if (nvram_match("openvpncl_tuntap", "tap") 
			&& strlen(nvram_safe_get("openvpncl_ip")) > 0) 
			fprintf(fp, "ifconfig tap1 %s netmask %s up\n", nvram_safe_get("openvpncl_ip"), nvram_safe_get("openvpncl_mask"));
	}
	if (nvram_match("openvpncl_nat", "1"))
		fprintf(fp, "iptables -I POSTROUTING -t nat -o %s1 -j MASQUERADE\n", nvram_safe_get("openvpncl_tuntap"));
	if (nvram_match("openvpncl_sec", "0"))
		fprintf(fp, "iptables -I INPUT -i %s1 -j ACCEPT\n", nvram_safe_get("openvpncl_tuntap"));
	else {
		if (nvram_match("openvpncl_tuntap", "tun"))	//only needed with tun
			fprintf(fp,
				"iptables -I INPUT -i %s1 -j ACCEPT\n"
				"iptables -I FORWARD -i %s1 -j ACCEPT\n"
				"iptables -I FORWARD -o %s1 -j ACCEPT\n", nvram_safe_get("openvpncl_tuntap"), nvram_safe_get("openvpncl_tuntap"), nvram_safe_get("openvpncl_tuntap"));
	}
	if (strlen(nvram_safe_get("openvpncl_route")) > 0) {	//policy based routing
		write_nvram("/tmp/openvpncl/policy_ips", "openvpncl_route");
//              fprintf(fp, "ip route flush table 10\n");
		fprintf(fp, "for IP in `cat /tmp/openvpncl/policy_ips` ; do\n" "\t ip rule add from $IP table 10\n" "done\n");
/*		if (nvram_match("openvpncl_tuntap", "tap"))
			fprintf(fp, "ip route add default via $route_vpn_gateway table 10\n"); //needs investigation cause in TAP mode no gateway is received
		else */
			fprintf(fp, "ip route add default via $route_vpn_gateway table 10\n");
		fprintf(fp, "ip route flush cache\n" "echo $ifconfig_remote >>/tmp/gateway.txt\n" "echo $route_vpn_gateway >>/tmp/gateway.txt\n" "echo $ifconfig_local >>/tmp/gateway.txt\n");
	}
	if (nvram_match("block_multicast", "0")	//block multicast on bridged vpns
	    && nvram_match("openvpncl_tuntap", "tap")
	    && nvram_match("openvpncl_bridge", "1")) {
		fprintf(fp, "insmod ebtables\n" "insmod ebtable_filter\n" "insmod ebtable_nat\n" "insmod ebt_pkttype\n"
//                      "ebtables -I FORWARD -o tap1 --pkttype-type multicast -j DROP\n"
//                      "ebtables -I OUTPUT -o tap1 --pkttype-type multicast -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP\n" "ebtables -t nat -I POSTROUTING -o tap1 --pkttype-type multicast -j DROP\n");
	}
	if (nvram_match("wshaper_enable", "1"))		
		fprintf(fp, "stopservice wshaper\n"
			"startservice wshaper\n");
	fclose(fp);

	fp = fopen("/tmp/openvpncl/route-down.sh", "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "#!/bin/sh\n");
	if (nvram_match("openvpncl_tuntap", "tap")
	    && nvram_match("openvpncl_bridge", "1")
	    && nvram_match("openvpncl_nat", "0"))
		fprintf(fp, "brctl delif br0 tap1\n" "ifconfig tap1 down\n");
	else if (nvram_match("openvpncl_tuntap", "tap")
		 && strlen(nvram_safe_get("openvpncl_ip")) > 0)
		fprintf(fp, "ifconfig tap1 down\n");
	if (nvram_match("openvpncl_nat", "1"))
		fprintf(fp, "iptables -D INPUT -i %s1 -j ACCEPT\n" "iptables -D POSTROUTING -t nat -o %s1 -j MASQUERADE\n", nvram_safe_get("openvpncl_tuntap"), nvram_safe_get("openvpncl_tuntap"));
	else {
		fprintf(fp,
			"iptables -D INPUT -i %s1 -j ACCEPT\n"
			"iptables -D FORWARD -i %s1 -j ACCEPT\n" "iptables -D FORWARD -o %s1 -j ACCEPT\n", nvram_safe_get("openvpncl_tuntap"), nvram_safe_get("openvpncl_tuntap"), nvram_safe_get("openvpncl_tuntap"));
	}
	if (strlen(nvram_safe_get("openvpncl_route")) > 0) {	//policy based routing
		write_nvram("/tmp/openvpncl/policy_ips", "openvpncl_route");
		fprintf(fp, "ip route flush table 10\n");
	}
/*	if (nvram_match("block_multicast", "0") //block multicast on bridged vpns
		&& nvram_match("openvpncl_tuntap", "tap")
		&& nvram_match("openvpncl_bridge", "1")) {
		fprintf(fp, "ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP\n"
			"if [ `ebtables -t nat -L|grep -e '-j' -c` -ne 0 ]\n"
			"then rmmod ebtable_nat\n" "\t rmmod ebtables\n");
		} */
	fclose(fp);

	chmod("/tmp/openvpncl/route-up.sh", 0700);
	chmod("/tmp/openvpncl/route-down.sh", 0700);

	if (nvram_match("use_crypto", "1"))
		eval("openvpn", "--config", "/tmp/openvpncl/openvpn.conf", "--route-up", "/tmp/openvpncl/route-up.sh", "--down-pre", "/tmp/openvpncl/route-down.sh", "--daemon", "--engine", "cryptodev");
	else
		eval("openvpn", "--config", "/tmp/openvpncl/openvpn.conf", "--route-up", "/tmp/openvpncl/route-up.sh", "--down-pre", "/tmp/openvpncl/route-down.sh", "--daemon");

	if (nvram_match("wshaper_enable", "1")) {
		eval("stopservice", "wshaper");
		eval("startservice", "wshaper");
	}

	return;
}

void stop_openvpn(void)
{
	if (stop_process("openvpn", "OpenVPN daemon (Client)")) {
		if (nvram_match("wshaper_enable", "1")) {
			stop_wshaper();
			start_wshaper();
		}
		//remove ebtables rules on shutdown  
		system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP");
		unlink("/tmp/openvpncl/ca.crt");
		unlink("/tmp/openvpncl/client.crt");
		unlink("/tmp/openvpncl/client.key");
		unlink("/tmp/openvpncl/ta.key");
		unlink("/tmp/openvpncl/cert.p12");
		unlink("/tmp/openvpncl/static.key");
		unlink("/tmp/openvpncl/openvpncl.conf");
		unlink("/tmp/openvpncl/route-up.sh");
		unlink("/tmp/openvpncl/route-down.sh");
	}
}

void stop_openvpn_wandone(void)
{
	if (nvram_invmatch("openvpncl_enable", "1"))
		return;

	if (stop_process("openvpn", "OpenVPN daemon (Client)")) {
		if (nvram_match("wshaper_enable", "1")) {
			stop_wshaper();
			start_wshaper();
		}
		//remove ebtables rules on shutdown     
		system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP");
		unlink("/tmp/openvpncl/ca.crt");
		unlink("/tmp/openvpncl/client.crt");
		unlink("/tmp/openvpncl/client.key");
		unlink("/tmp/openvpncl/ta.key");
		unlink("/tmp/openvpncl/cert.p12");
		unlink("/tmp/openvpncl/static.key");
		unlink("/tmp/openvpncl/openvpncl.conf");
		unlink("/tmp/openvpncl/route-up.sh");
		unlink("/tmp/openvpncl/route-down.sh");
	}
}

#endif
