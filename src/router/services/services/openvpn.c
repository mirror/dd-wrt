/*
 * openvpn.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com
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

#if defined(HAVE_IPQ806X)
#define usecrypto 1
#elif defined(HAVE_IPQ6018)
#define usecrypto 1
#elif defined(HAVE_MVEBU)
#define usecrypto 1
#else
#define usecrypto nvram_matchi("use_crypto", 1)
#endif
//use in interfrace when WAP is detected
char IN_IF[8] = { 0 };

static void run_openvpn(char *prg, char *path)
{
	char *conf;
	asprintf(&conf, "/tmp/%s/openvpn.conf", path);
	if (usecrypto) {
		insmod("cryptodev");
		dd_logstart("openvpn", eval(prg, "--config", conf, "--daemon", "--engine", "devcrypto"));
	} else {
		rmmod("cryptodev");
		dd_logstart("openvpn", eval(prg, "--config", conf, "--daemon"));
	}
	free(conf);
}

int cleanup_pbr(char *tablenr)
{
	int limit = 1000;
	//char cmd[256] = { 0 };
	eval("ip", "route", "flush", "table", tablenr);
	//this is no longer necessary as the bug in OpenVPN about misdetection of default route is resolved in 2.5.2
	//eval("ip", "route", "flush", "table", "9");

	//eval("while", "ip", "rule", "delete", "from", "0/0", "to", "0/0", "table", tablenr2, ";", "do", "true", ";", "done");  //does not work revert to system()
	//sprintf(cmd, "while ip rule delete from 0/0 to 0/0 table %s; do true; done", tablenr);
	//system(cmd);
	while (limit > 0 && !eval("ip", "rule", "delete", "from", "0/0", "to", "0/0", "table", tablenr)) {
		limit--;
	}
	return 0;
}

int is_ipv4(char *ip)
{
	int num;
	int flag = 1;
	int counter = 0;
	int i;
	char *p = strtok(ip, ".");
	while (p && flag) {
		for (i = 0; p[i] != '\0'; i++) {
			if (!isdigit(p[i])) {
				return 0;
			}
		}
		num = atoi(p);
		if (num >= 0 && num <= 255 && (counter++ < 4)) {
			flag = 1;
			p = strtok(NULL, ".");
		} else {
			flag = 0;
			break;
		}
	}
	return flag && (counter == 4);
}

int split_ipv4(char *ipv4)
{
	char ip_mask[100] = { 0 };
	char ip[64] = { 0 };
	char mask[4] = { 0 };
	int mask_ok = 0;
	char *p;
	strncpy(ip_mask, ipv4, (sizeof ip_mask) - 1);
	p = strtok(ip_mask, "//");
	if (p != NULL) {
		strncpy(ip, p, (sizeof ip) - 1);
		p = strtok(NULL, "//");
		if (p != NULL)
			strncpy(mask, p, (sizeof mask) - 1);
	}
	if (mask[0] != '\0') {
		if (atoi(mask) > 0 && atoi(mask) <= 32) {
			mask_ok = 1;
		}
	} else if (!strchr(ipv4, '/')) {
		mask_ok = 1;
	}
	if (is_ipv4(ip) && mask_ok) {
		return 1;
	} else {
		return 0;
	}
}

int test_ipv4(char *iprule)
{
	char *pch;
	int flag = 1;
	char ipv4[5][100] = { 0 };
	char strtosplit[100] = { 0 };
	int i = 1;
	int j;
	strncpy(strtosplit, iprule, (sizeof strtosplit) - 1);
	pch = strtok(strtosplit, " ");
	while (pch != NULL) {
		if (isdigit(pch[0]) && strchr(pch, '.')) {
			strncpy(ipv4[i], pch, (sizeof ipv4[i]) - 1);
			if (i < 5) {
				i++;
			} else {
				break;
			}
		}
		pch = strtok(NULL, " ");
	}
	for (j = 1; j < i; j++) {
		if (!split_ipv4(ipv4[j])) {
			flag = 0; //no valid IP
		}
	}
	return flag;
}

void setroute_pbr(char *tablenr)
{
	char *curline = nvram_safe_get("openvpncl_route");
	char cmd[256] = { 0 };

	cleanup_pbr(tablenr);

	if (nvram_matchi("openvpncl_killswitch", 1) && nvram_matchi("openvpncl_spbr", 1)) {
		eval("ip", "route", "add", "prohibit", "default", "table", tablenr);
		dd_loginfo("openvpn", "PBR killswitch is active using: %s", curline);
		// need table with a number below table 10 with default gateway to let OpenVPN get default route, possible bug where OpenVPN uses default route from lowest table but not from main table resolved in OpenVPN 2.5.2
		//eval("ip", "route", "add", "default", "via", nvram_safe_get("wan_gateway"), "table", "9");
	} else {
		eval("ip", "route", "add", "default", "via", nvram_safe_get("wan_gateway"), "table", tablenr);
		dd_loginfo("openvpn", "PBR is active but NO killwitch: %s", curline);
	}
	//add other routes from main table
	sprintf(cmd,
		"ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do ip route add $route table %s >/dev/null 2>&1; done",
		tablenr);
	sysprintf(cmd);

	while (curline) {
		char *nextLine = strchr(curline, '\n');
		size_t curlinelen = nextLine ? ((unsigned)(nextLine - curline)) : strlen(curline);
		char *tempstr = (char *)malloc(curlinelen + 1);
		char iprule[128] = { 0 };
		char *piprule;
		if (tempstr) {
			memcpy(tempstr, curline, curlinelen);
			tempstr[curlinelen] = '\0'; // NUL-terminate!
			//dd_loginfo("openvpn", "tempstr= [%s]\n", tempstr);
			// check if line starts with a # skip
			if (tempstr[0] == '#') {
				//dd_loginfo("openvpn", "Skip line starting with #: %s\n", tempstr);
			} else {
				// add from if starting with digit
				if (isdigit(tempstr[0])) {
					//dd_loginfo("openvpn", "String started with digit added from for backwards compatiblitiy: %s\n", tempstr);
					strcpy(iprule, "from ");
				}
				strcat(iprule, tempstr);
				if ((piprule = strchr(iprule, '#'))) {
					piprule[0] = '\0';
				} else if ((piprule = strchr(iprule, '\r'))) {
					piprule[0] = '\0';
				}
				if (*iprule) {
					if (test_ipv4(iprule)) {
						sysprintf("ip rule add %s table %s", iprule, tablenr);
					} else {
						dd_loginfo("openvpn", "ip rule %s has NO valid IP address, not added to table %s",
							   iprule, tablenr);
					}
				}
			}
			free(tempstr);
		} else {
			dd_loginfo("openvpn", "malloc() failed in creating PBR!?");
		}
		curline = nextLine ? (nextLine + 1) : NULL;
	}
}

void create_openvpnrules(FILE *fp)
{
	fprintf(fp, "[[ ! -z \"$ifconfig_netmask\" ]] && vpn_netmask=\"/$ifconfig_netmask\"\n");
	fprintf(fp,
		"cat << EOF > /tmp/openvpncl_fw.sh\n"
		"#!/bin/sh\n"); // write firewall rules on route up to separate script to expand env. parameters
	if (nvram_matchi("openvpncl_nat", 1)) {
		fprintf(fp,
			"iptables -D POSTROUTING -t nat -o $dev -j MASQUERADE 2> /dev/null\n" //
			"iptables -I POSTROUTING -t nat -o $dev -j MASQUERADE\n");
	}
	if (nvram_match("openvpncl_mit", "1"))
		fprintf(fp, "iptables -t raw -D PREROUTING ! -i $dev -d $ifconfig_local$vpn_netmask -j DROP 2> /dev/null\n"
			    "iptables -t raw -I PREROUTING ! -i $dev -d $ifconfig_local$vpn_netmask -j DROP\n");
	if (nvram_matchi("openvpncl_blockmulticast",
			 1) //block multicast on bridged vpns
	    && nvram_match("openvpncl_tuntap", "tap") && nvram_matchi("openvpncl_bridge", 1)) {
		fprintf(fp, "insmod ebtables\n"
			    "insmod ebtable_filter\n"
			    "insmod ebtable_nat\n"
			    "insmod ebt_pkttype\n"
			    //                      "ebtables -I FORWARD -o tap1 --pkttype-type multicast -j DROP\n"
			    //                      "ebtables -I OUTPUT -o tap1 --pkttype-type multicast -j DROP\n"
			    "ebtables -t nat -D POSTROUTING -o $dev --pkttype-type multicast -j DROP\n"
			    "ebtables -t nat -I POSTROUTING -o $dev --pkttype-type multicast -j DROP\n");
	}
	if (nvram_matchi("openvpncl_fw", 0)) {
		if (nvram_match("openvpncl_tuntap", "tun")) {
			fprintf(fp, "iptables -D INPUT -i $dev -m state --state NEW -j ACCEPT 2> /dev/null\n"
				    "iptables -D FORWARD -i $dev -m state --state NEW -j ACCEPT 2> /dev/null\n"
				    "iptables -I INPUT -i $dev -m state --state NEW -j ACCEPT\n"
				    "iptables -I FORWARD -i $dev -m state --state NEW -j ACCEPT\n");
		}
	}
	fprintf(fp, "EOF\n"
		    "chmod +x /tmp/openvpncl_fw.sh\n");

	if (nvram_match("openvpncl_tuntap", "tun")) {
		//use awk print*f* to skip newline
		//fprintf(fp, "alldns=$(env | grep 'dhcp-option DNS' | awk '{print $NF}')\n"
		fprintf(fp,
			"alldns=\"$(env | grep 'dhcp-option DNS' |  awk '{ printf \"%%s \",$3 }')\"\n"); //do not forget to escape %s with %%s
		// might reverse dns servers in string: alldns=echo "alldns" | awk '{ for (i=NF; i>1; i--) printf("%s ",$i); print $1; }'
		//"nvram set openvpn_get_dns=\"$alldns\"\n"
		fprintf(fp,
			"if [[ ! -z \"$alldns\" ]]; then\n"
			"spbr=\"$(nvram get openvpncl_spbr)\"\n"
			"splitdns=\"$(nvram get openvpncl_splitdns)\"\n"
			"cat /tmp/resolv.dnsmasq > /tmp/resolv.dnsmasq_isp\n"
			"[[ $spbr != \"1\" || $splitdns = \"0\" ]] && { rm -f /tmp/resolv.dnsmasq; nvram set openvpn_get_dns=\"$alldns\"; }\n"
			"set=0\n"
			"for dns in $alldns;do\n"
			"[[ $spbr != \"1\" || $splitdns = \"0\" ]] && echo \"nameserver $dns\" >> /tmp/resolv.dnsmasq\n"
			//route only the pushed DNS servers via the tunnel the added ones must be routed manually if required
			"grep -q \"^dhcp-option DNS $dns\" /tmp/openvpncl/openvpn.conf || ip route add $dns via $route_vpn_gateway dev $dev 2> /dev/null\n");
		//"ip route add $dns via $route_vpn_gateway dev $dev 2> /dev/null\n"
		if (nvram_matchi("openvpncl_splitdns", 1) && nvram_invmatch("openvpncl_spbr", "0")) {
			fprintf(fp,
				"if [[ $set -eq 0 ]]; then\n"
				"logger -p user.info \"OpenVPN split DNS invoked\"\n"
				"[[ $spbr =  \"2\" ]] && { dns=$(nvram get wan_dns); [[ -z \"$dns\" ]] && dns=$(nvram get wan_get_dns); dns=${dns%%%% *}; }\n"
				"sed '/^[[:blank:]]*#/d;s/#.*//' \"/tmp/openvpncl/policy_ips\" | while read pbrip; do\n"
				"case $pbrip in\n"
				"[0-9]*)\n"
				"sourcepbr=\"-s\"\n"
				";;\n"
				"iif*)\n"
				"sourcepbr=\"-i\"\n"
				//"#pbrip=${pbrip#* }"
				"pbrip=\"${pbrip#iif }\"\n"
				";;\n"
				"*)\n"
				"continue\n"
				";;\n"
				"esac\n"
				"echo \"iptables -t nat -D PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns\" >> /tmp/OVPNDEL\n"
				"echo \"iptables -t nat -D PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns\" >> /tmp/OVPNDEL\n"
				"echo \"iptables -t nat -D PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns >/dev/null 2>&1\" >> /tmp/openvpncl_fw.sh\n"
				"echo \"iptables -t nat -I PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns\" >> /tmp/openvpncl_fw.sh\n"
				"echo \"iptables -t nat -D PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns >/dev/null 2>&1\" >> /tmp/openvpncl_fw.sh\n"
				"echo \"iptables -t nat -I PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns\" >> /tmp/openvpncl_fw.sh\n"
				"done\n"
				"set=1\n"
				"fi\n");
		}
		fprintf(fp, "done\n"
			    "else\n"
			    "logger -p user.warning \"OpenVPN No VPN DNS servers available\"\n"
			    "fi\n");
		//escape general killswitch if PBR via WAN  //todo move this to up.sh instead of rout-up.sh so that it runs when the OVPN is not connecting
		if (nvram_matchi("openvpncl_killswitch", 1) && nvram_matchi("openvpncl_spbr", 2)) {
			fprintf(fp, "logger -p user.info \"OpenVPN firewall PBR via WAN escape\"\n");
			fprintf(fp, "echo \"iptables -N vpn-pbr\" >> /tmp/openvpncl_fw.sh\n");
			fprintf(fp, "sed '/^[[:blank:]]*#/d;s/#.*//' \"/tmp/openvpncl/policy_ips\" | while read pbrip; do\n"
				    "case $pbrip in\n"
				    "[0-9]*)\n"
				    "sourcepbr=\"-s\"\n"
				    ";;\n"
				    "iif*)\n"
				    "sourcepbr=\"-i\"\n"
				    "pbrip=\"${pbrip#iif }\"\n"
				    ";;\n"
				    "*port*)\n"
				    "sourcepbr=\"port\"\n"
				    "pbrip=\"--$pbrip\"\n"
				    ";;\n"
				    "*)\n"
				    "continue\n"
				    ";;\n"
				    "esac\n"
				    "if [[ $sourcepbr = \"port\" ]];then\n"
				    "echo \"iptables -A vpn-pbr -p tcp $pbrip -j ACCEPT \" >> /tmp/openvpncl_fw.sh\n"
				    "echo \"iptables -A vpn-pbr -p udp $pbrip -j ACCEPT \" >> /tmp/openvpncl_fw.sh\n"
				    "else\n"
				    "echo \"iptables -A vpn-pbr $sourcepbr $pbrip -j ACCEPT \" >> /tmp/openvpncl_fw.sh\n"
				    "fi\n"
				    "done\n");
			//fprintf(fp, "[[ $($nv get wan_proto) = \"disabled\" ]] && { IN_IF=\"-i br0\"; logger -p user.info \"VPN Killswitch for WAP\"; } || IN_IF=\"\"\n");
			//IN_IF is declared global scope and made line 720
			fprintf(fp, "echo \"iptables -D FORWARD %s -o $(get_wanface) -j vpn-pbr\" >> /tmp/openvpncl_fw.sh\n",
				IN_IF);
			fprintf(fp, "echo \"iptables -I FORWARD %s -o $(get_wanface) -j vpn-pbr\" >> /tmp/openvpncl_fw.sh\n",
				IN_IF);
		}
	}
	fprintf(fp, "/tmp/openvpncl_fw.sh\n");
	//if (*(nvram_safe_get("openvpncl_route")) && strncmp((nvram_safe_get("openvpncl_route")), "#", 1) != 0 && nvram_invmatch("openvpncl_spbr", "0")) {     //policy based routing
	if (*(nvram_safe_get("openvpncl_route")) && nvram_invmatch("openvpncl_spbr", "0")) { //policy based routing
		write_nvram("/tmp/openvpncl/policy_ips", "openvpncl_route");
		//fprintf(fp, "sed '/^[[:blank:]]*#/d;s/#.*//;/^$/d' \"/tmp/openvpncl/policy_ips\" | while read IP; do [[ \"$IP\" == [0-9]* ]] && IP=\"from $IP\"; ip rule add table 10 $IP; done\n");  //rules added in setroute_pbr
		//remove possible wrong from all (bug in ip rule command where it can add garbage), courtesy of @eibgrad
		fprintf(fp, "for i in $(ip rule | grep 'from all lookup 10' | cut -d: -f1); do ip rule del prior $i; done\n");
		/*              if (nvram_match("openvpncl_tuntap", "tap"))
                        fprintf(fp, "ip route add default via $route_vpn_gateway table 10\n"); //needs investigation cause in TAP mode no gateway is received
                else */
		if (!nvram_match("openvpncl_tuntap", "tap")) {
			//fprintf(fp, "ip route add default via $route_vpn_gateway table 10\n");
			//flush table so in case of SIGHUP and having a new endpoint the endpoint route is flushed
			fprintf(fp, "ip route flush table 10\n");
			if (nvram_matchi("openvpncl_spbr",
					 1)) { //PBR via VPN openvpncl_spbr=1
				fprintf(fp, "ip route add 0.0.0.0/1 via $route_vpn_gateway table 10\n");
				fprintf(fp, "ip route add 128.0.0.0/1 via $route_vpn_gateway table 10\n");
				if (nvram_matchi("openvpncl_killswitch", 1)) {
					fprintf(fp, "ip route add prohibit default table 10 >/dev/null 2>&1\n");
				}
			} else {
				fprintf(fp, "ip route add default via %s table 10 >/dev/null 2>&1\n",
					nvram_safe_get("wan_gateway"));
			}
			//Adding local routes to alternate PBR routing table by egc
			fprintf(fp, "ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do\n"
				    "\t ip route add $route table 10 >/dev/null 2>&1\n"
				    "done\n");
		}
		fprintf(fp, "ip route flush cache\n"
			    "echo $ifconfig_remote >>/tmp/gateway.txt\n"
			    "echo $route_vpn_gateway >>/tmp/gateway.txt\n"
			    "echo $ifconfig_local >>/tmp/gateway.txt\n");
	}
	fprintf(fp,
		"exit 0\n"); //to prevent exit 2 fall through when DNS route is already made by provider
}

void create_openvpnserverrules(FILE *fp)
{
	fprintf(fp,
		"cat << EOF > /tmp/openvpnsrv_fw.sh\n"
		"#!/bin/sh\n"); // write firewall rules on route up to separate script to expand env. parameters
	if (nvram_matchi("openvpn_blockmulticast",
			 1) //block multicast on bridged vpns
	    && nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "insmod ebtables\n"
			    "insmod ebtable_filter\n"
			    "insmod ebtable_nat\n"
			    "insmod ebt_pkttype\n"
			    "ebtables -t nat -D POSTROUTING -o $dev --pkttype-type multicast -j DROP\n"
			    "ebtables -t nat -I POSTROUTING -o $dev --pkttype-type multicast -j DROP\n");
	if (nvram_matchi("openvpn_dhcpbl", 1) //block dhcp on bridged vpns
	    && nvram_match("openvpn_tuntap", "tap") && nvram_matchi("openvpn_proxy", 0))
		fprintf(fp,
			"insmod ebtables\n"
			"insmod ebt_ip\n"
			"insmod ebtable_filter\n"
			"insmod ebtable_nat\n"
			"ebtables -t nat -D PREROUTING -i $dev -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o $dev -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -I PREROUTING -i $dev -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -I POSTROUTING -o $dev -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n");
	if (nvram_default_matchi("openvpn_fw", 1, 0)) {
		fprintf(fp, "iptables -I INPUT -i $dev -m state --state NEW -j DROP\n");
		fprintf(fp, "iptables -I FORWARD -i $dev -m state --state NEW -j DROP\n");
	}
	if (nvram_match("openvpn_mit", "1"))
		fprintf(fp, "iptables -t raw -D PREROUTING ! -i $dev -d $ifconfig_local/$ifconfig_netmask -j DROP\n"
			    "iptables -t raw -I PREROUTING ! -i $dev -d $ifconfig_local/$ifconfig_netmask -j DROP\n");
	if (nvram_match("openvpn_tuntap", "tun")) {
		if (nvram_matchi("openvpn_allowcnwan", 1)) {
			fprintf(fp,
				"iptables -t nat -D POSTROUTING -s $(nvram get openvpn_net)/$(nvram get openvpn_tunmask) -o $(get_wanface) -j MASQUERADE >/dev/null 2>&1\n");
			fprintf(fp,
				"iptables -t nat -A POSTROUTING -s $(nvram get openvpn_net)/$(nvram get openvpn_tunmask) -o $(get_wanface) -j MASQUERADE\n");
#ifdef HAVE_IPV6
			if (nvram_invmatch("openvpn_v6netmask", "") && nvram_matchi("ipv6_enable", 1)) {
				fprintf(fp,
					"ip6tables -t nat -D POSTROUTING -s $(nvram get openvpn_v6netmask) -o $(get_wanface) -j MASQUERADE >/dev/null 2>&1\n");
				fprintf(fp,
					"ip6tables -t nat -A POSTROUTING -s $(nvram get openvpn_v6netmask) -o $(get_wanface) -j MASQUERADE\n");
			}
#endif
		}
		if (nvram_matchi("openvpn_allowcnlan", 1)) {
			fprintf(fp,
				"iptables -t nat -D POSTROUTING -o br+ -s $(nvram get openvpn_net)/$(nvram get openvpn_tunmask) -j MASQUERADE >/dev/null 2>&1\n");
			fprintf(fp,
				"iptables -t nat -A POSTROUTING -o br+ -s $(nvram get openvpn_net)/$(nvram get openvpn_tunmask) -j MASQUERADE\n");
#ifdef HAVE_IPV6
			if (nvram_invmatch("openvpn_v6netmask", "") && nvram_matchi("ipv6_enable", 1)) {
				fprintf(fp,
					"ip6tables -t nat -D POSTROUTING -o br+ -s $(nvram get openvpn_v6netmask) -j MASQUERADE >/dev/null 2>&1\n");
				fprintf(fp,
					"ip6tables -t nat -A POSTROUTING -o br+ -s $(nvram get openvpn_v6netmask) -j MASQUERADE\n");
			}
#endif
		}
	}
	fprintf(fp, "EOF\n"
		    "chmod +x /tmp/openvpnsrv_fw.sh\n");
	fprintf(fp, "/tmp/openvpnsrv_fw.sh\n");
}

void start_openvpnserver(void)
{
	char wan_if_buffer[33];
	int jffs = 0;
	if (nvram_invmatchi("openvpn_enable", 1))
		return;
	insmod("tun");
	eval("modprobe", "geniv");
	eval("modprobe", "seqiv");
	eval("modprobe", "ctr");
	eval("modprobe", "chacha20_generic");
	eval("modprobe", "chacha20_x86_64");
	eval("modprobe", "chacha20_ssse3-x86_64");
	eval("modprobe", "chacha20_avx512vl-x86_64");
	eval("modprobe", "chacha20_avx2-x86_64");
	eval("modprobe", "poly1305_generic");
	eval("modprobe", "poly1305_x86_64");
	eval("modprobe", "chacha20poly1305");
	eval("modprobe", "ghash_generic");
	eval("modprobe", "gcm");
	eval("modprobe", "ccm");
	eval("modprobe", "ovpn-dco-v2");
	update_timezone();
	if (jffs_mounted())
		jffs = 1;
	dd_loginfo("openvpn", "OpenVPN daemon (Server) starting/restarting...");
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
	if (nvram_matchi("openvpn_tls_btn", 2) && nvram_invmatch("openvpn_static", "")) { //static key
		fprintf(fp, "secret /tmp/openvpn/static.key\n");
	} else if (nvram_matchi("openvpn_pkcs", 1) && nvram_invmatch("openvpn_pkcs12", "")) {
		if (nvram_invmatch("openvpn_dh", "") && nvram_matchi("openvpn_dh_btn", 0))
			fprintf(fp, "dh /tmp/openvpn/dh.pem\n");
		fprintf(fp, "pkcs12 /tmp/openvpn/cert.p12\n");
	} else {
		if (nvram_invmatch("openvpn_dh", "") && nvram_matchi("openvpn_dh_btn",
								     0)) //egc use ECDH instead of PEM key
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
		fprintf(fp,
			"keepalive 10 120\n"
			"verb 3\n"
			"mute 3\n"
			"syslog\n"
			"writepid /var/run/openvpnd.pid\n"
			"management 127.0.0.1 14\n"
			"management-log-cache 100\n"
			"topology subnet\n"
			"script-security 2\n"
			"port %s\n"
			"proto %s\n",
			nvram_safe_get("openvpn_port"), nvram_safe_get("openvpn_proto"));
		//egc
		if (nvram_invmatch("openvpn_auth", ""))
			fprintf(fp, "auth %s\n", nvram_safe_get("openvpn_auth"));
		if (nvram_invmatch("openvpn_cipher", ""))
			fprintf(fp, "cipher %s\n", nvram_safe_get("openvpn_cipher"));
		char dcbuffer[128] = { 0 };
		char *dc1 = nvram_safe_get("openvpn_dc1");
		char *dc2 = nvram_safe_get("openvpn_dc2");
		char *dc3 = nvram_safe_get("openvpn_dc3");
		sprintf(dcbuffer, "%s%s%s%s%s", dc1, *dc2 ? ":" : "", dc2, *dc3 ? ":" : "", dc3);
		if (dcbuffer[0])
			fprintf(fp, "data-ciphers %s\n", dcbuffer);

		fprintf(fp, "client-connect /tmp/openvpn/clcon.sh\n");
		fprintf(fp, "client-disconnect /tmp/openvpn/cldiscon.sh\n");
		if (jffs == 1) //  use usb/jffs for ccd if available
			fprintf(fp, "client-config-dir /jffs/etc/openvpn/ccd\n");
		else
			fprintf(fp, "client-config-dir /tmp/openvpn/ccd\n");
		if (nvram_invmatch("openvpn_scramble", "off")) {
			fprintf(fp, "disable-dco\n");
			fprintf(fp,
				"scramble %s\n", //scramble XOR patch for reordering packet content to protect against DPI
				nvram_safe_get("openvpn_scramble"));
		}
		if (nvram_invmatch("openvpn_lzo", "off")) {
			if (nvram_match("openvpn_lzo", "compress lz4"))
				fprintf(fp, "compress lz4\n");
			else if (nvram_match("openvpn_lzo", "compress lz4-v2"))
				fprintf(fp, "compress lz4-v2\n");
			else if (nvram_match("openvpn_lzo", "compress"))
				fprintf(fp, "compress\n");
			else
				fprintf(fp,
					"comp-lzo %s\n", //yes/no/adaptive/disable
					nvram_safe_get("openvpn_lzo"));
		}
		if (nvram_invmatch("openvpn_auth",
				   "none")) //? the server directive already set will expand to tls-server
			fprintf(fp, "tls-server\n");
		if (nvram_matchi("openvpn_dupcn", 1))
			fprintf(fp, "duplicate-cn\n");
		if (nvram_matchi("openvpn_dupcn",
				 0) //keep peer ip persistant for x sec. works only when dupcn=off & no proxy mode
		    && nvram_matchi("openvpn_proxy", 0))
			fprintf(fp, "ifconfig-pool-persist /tmp/openvpn/ip-pool 86400\n");
		if (nvram_matchi("openvpn_cl2cl", 1))
			fprintf(fp, "client-to-client\n");
		if (nvram_matchi("openvpn_redirgate", 1)) {
#ifdef HAVE_IPV6
			if (nvram_invmatch("openvpn_v6netmask", "") && nvram_matchi("ipv6_enable", 1)) {
				fprintf(fp, "push \"redirect-gateway ipv6 def1\"\n");
			} else {
#endif
				fprintf(fp, "push \"redirect-gateway def1\"\n");
#ifdef HAVE_IPV6
			}
#endif
		} else if (nvram_matchi("openvpn_redirgate", 2)) {
			char lanaddr[32] = { 0 };
			char *mypoint;
			char srvnetwork[32] = { 0 };
			snprintf(lanaddr, sizeof(lanaddr), "%s", nvram_safe_get("lan_ipaddr"));
			//truncate and replace with .0
			mypoint = strrchr(lanaddr, '.');
			if (mypoint != NULL) {
				*mypoint = '\0';
				snprintf(srvnetwork, sizeof(srvnetwork), "%s%s", lanaddr, ".0");
				fprintf(fp, "push \"route %s %s vpn_gateway\"\n", srvnetwork, nvram_safe_get("lan_netmask"));
			}
		}
		if (nvram_invmatchi("openvpn_tlscip", 0))
			fprintf(fp, "tls-cipher %s\n", nvram_safe_get("openvpn_tlscip"));
		if (nvram_match("openvpn_proto", "udp") || nvram_match("openvpn_proto", "udp4") ||
		    nvram_match("openvpn_proto", "udp6"))
			fprintf(fp,
				"fast-io\n"); //experimental!improving CPU efficiency by 5%-10%
		else //TCP_NODELAY is generally a good latency optimization
			fprintf(fp, "tcp-nodelay\n");
		if (nvram_invmatch("openvpn_mtu", "") && nvram_invmatchi("openvpn_mtu", 0))
			fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpn_mtu"));
		if (nvram_invmatch("openvpn_fragment", "") &&
		    (nvram_match("openvpn_proto", "udp") || nvram_match("openvpn_proto", "udp4") ||
		     nvram_match("openvpn_proto", "udp6"))) {
			fprintf(fp, "fragment %s\n", nvram_safe_get("openvpn_fragment"));
			if (nvram_matchi("openvpn_mssfix", 1))
				fprintf(fp,
					"mssfix\n"); //mssfix=1450 (default), should be set on one side only. when fragment->=mss
		} else
			fprintf(fp, "mtu-disc yes\n");
		if (nvram_match("openvpn_tuntap", "tun")) {
			fprintf(fp, "server %s %s\n", nvram_safe_get("openvpn_net"), nvram_safe_get("openvpn_tunmask"));
			fprintf(fp, "dev tun2\n");
#ifdef HAVE_IPV6
			if (nvram_invmatch("openvpn_v6netmask", "") && nvram_matchi("ipv6_enable", 1)) {
				fprintf(fp, "server-ipv6 %s\n", nvram_safe_get("openvpn_v6netmask"));
			}
#endif
		} else if (nvram_match("openvpn_tuntap", "tap") && nvram_matchi("openvpn_proxy", 0)) {
			fprintf(fp, "server-bridge %s %s %s %s\n", nvram_safe_get("openvpn_gateway"),
				nvram_safe_get("openvpn_mask"), nvram_safe_get("openvpn_startip"), nvram_safe_get("openvpn_endip"));
			fprintf(fp, "dev tap2\n");
		} else if (nvram_match("openvpn_tuntap", "tap") && nvram_matchi("openvpn_proxy", 1) &&
			   nvram_matchi("openvpn_redirgate", 1))
			fprintf(fp, "server-bridge\n"
				    "dev tap2\n");
		else
			fprintf(fp, "server-bridge nogw\n"
				    "dev tap2\n");
		if (*(nvram_safe_get("openvpn_tlsauth"))) {
			if (nvram_matchi("openvpn_tls_btn", 1))
				fprintf(fp,
					"tls-crypt /tmp/openvpn/ta.key\n"); //egc: tls_btn 1 is tls-crypt, 0 is tls-auth, 2 is static key, 4 is tls-crypt-v2
			else if (nvram_matchi("openvpn_tls_btn", 4))
				fprintf(fp, "tls-crypt-v2 /tmp/openvpn/ta.key\n");
			else if (nvram_matchi("openvpn_tls_btn", 0))
				fprintf(fp, "tls-auth /tmp/openvpn/ta.key 0\n");
		}
		//egc use ECDH instead of PEM key
		if (nvram_matchi("openvpn_dh_btn", 1)) {
			fprintf(fp, "dh none\n"
				    "ecdh-curve secp384r1\n");
		}
		//egc: add route-up and down to .conf
		fprintf(fp, "route-up /tmp/openvpn/route-up.sh\n");
		fprintf(fp, "route-pre-down /tmp/openvpn/route-down.sh\n");
		if (*(nvram_safe_get("openvpn_crl")))
			fprintf(fp, "crl-verify /tmp/openvpn/ca.crl\n");
		/* for QOS */
		if (nvram_matchi("wshaper_enable", 1))
			fprintf(fp, "passtos\n");
		if (nvram_matchi("openvpn_enuserpass", 1) && nvram_invmatchi("openvpn_userpassnum", 0)) {
			fprintf(fp, "script-security 3\n");
			fprintf(fp, "username-as-common-name\n");
			fprintf(fp,
				"verify-client-cert optional\n"); // TODO make setting, this can be require, optional or none, now optional so only password and CA file are necessary
			fprintf(fp, "auth-user-pass-verify /tmp/openvpn/quickAuth.sh via-env\n");
		}
	} // else //egc loading a single public server certificate is not what we want, use inline certificates
	//write_nvram("/tmp/openvpn/cert.pem", "openvpn_client");
	fprintf(fp, "%s\n", nvram_safe_get("openvpn_config"));
	fclose(fp);

	// egc make file with username and password
	if (nvram_matchi("openvpn_enuserpass", 1) && nvram_invmatchi("openvpn_userpassnum", 0)) {
		fp = fopen("/tmp/openvpn/openvpn-auth", "wb");
		if (!fp) {
			dd_loginfo("openvpn", "ERROR: Could not open %s to store usernames and passwords",
				   "/tmp/openvpn/openvpn-auth");
		} else {
			int i;
			int userpassnum = nvram_geti("openvpn_userpassnum");
			char *nvuserpass = nvram_safe_get("openvpn_userpass");
			char *userpass = strdup(nvuserpass);
			char *originalpointer = userpass; // strsep destroys the pointer by moving it
			for (i = 0; i < userpassnum; i++) {
				char *sep = strsep(&userpass, "=");
				fprintf(fp, "%s", sep); //printf username
				sep = strsep(&userpass, " ");
				fprintf(fp, " %s\n", sep); //printf password
			}
			debug_free(originalpointer);
			fclose(fp);
		}
		// make script to execute
		fp = fopen("/tmp/openvpn/quickAuth.sh", "wb");
		if (!fp) {
			dd_loginfo("openvpn", "ERROR: Could not open username/password authentication script %s ",
				   "/tmp/openvpn/quickAuth.sh");
		} else {
			fprintf(fp, "#!/bin/sh\n");
			fprintf(fp, "pass=\"$(awk -v usr=$username '$1==usr { print $2 }' /tmp/openvpn/openvpn-auth)\"\n");
			fprintf(fp, "[[ -n $pass && $pass == $password ]] && exit 0\n");
			fprintf(fp, "exit 1\n");
			fclose(fp);
			chmod("/tmp/openvpn/quickAuth.sh", 0700);
		}
	}

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
		fprintf(fp,
			"brctl addif br0 tap2\n"
			"ifconfig tap2 0.0.0.0 up\n"); //non promisc for performance reasons
	}
	if (nvram_matchi("wshaper_enable", 1)) {
		fprintf(fp, "startservice set_routes -f\n");
		fprintf(fp, "restart firewall\n");
		fprintf(fp, "sleep 5\n");
	}
	/* egc: add interface as listen interface to DNSMasq via nvram parameter dnsmasq_addifvpn */
	if (nvram_match("openvpn_tuntap", "tun")) {
		fprintf(fp, "nvram set dnsmasq_addifvpn=$dev\n");
		fprintf(fp, "service dnsmasq restart\n");
	}
	create_openvpnserverrules(fp);
	fprintf(fp, "exit 0\n");
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
	if (nvram_matchi("openvpn_blockmulticast", 1) && nvram_match("openvpn_tuntap", "tap"))
		fprintf(fp, "ebtables -t nat -D POSTROUTING -o $dev --pkttype-type multicast -j DROP\n");
	if (nvram_matchi("openvpn_dhcpbl", 1) && nvram_match("openvpn_tuntap", "tap") && nvram_matchi("openvpn_proxy", 0))
		fprintf(fp,
			"ebtables -t nat -D PREROUTING -i $dev -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n"
			"ebtables -t nat -D POSTROUTING -o $dev -p ipv4 --ip-proto udp --ip-sport 67:68 --ip-dport 67:68 -j DROP\n");
	if (nvram_default_matchi("openvpn_fw", 1, 0)) {
		fprintf(fp, "iptables -D INPUT -i $dev -m state --state NEW -j DROP\n");
		fprintf(fp, "iptables -D FORWARD -i $dev -m state --state NEW -j DROP\n");
	}
	if (nvram_match("openvpn_mit", "1"))
		fprintf(fp, "iptables -t raw -D PREROUTING ! -i $dev -d $ifconfig_local/$ifconfig_netmask -j DROP\n");
	if (nvram_matchi("openvpn_allowcnwan", 1)) {
		fprintf(fp,
			"iptables -t nat -D POSTROUTING -s $(nvram get openvpn_net)/$(nvram get openvpn_tunmask) -o $(get_wanface) -j MASQUERADE\n");
#ifdef HAVE_IPV6
		if (nvram_invmatch("openvpn_v6netmask", "") && nvram_matchi("ipv6_enable", 1)) {
			fprintf(fp,
				"ip6tables -t nat -D POSTROUTING -s $(nvram get openvpn_v6netmask) -o $(get_wanface) -j MASQUERADE >/dev/null 2>&1\n");
		}
#endif
	}
	if (nvram_matchi("openvpn_allowcnlan", 1)) {
		fprintf(fp,
			"iptables -t nat -D POSTROUTING -o br+ -s $(nvram get openvpn_net)/$(nvram get openvpn_tunmask) -j MASQUERADE\n");
#ifdef HAVE_IPV6
		if (nvram_invmatch("openvpn_v6netmask", "") && nvram_matchi("ipv6_enable", 1)) {
			fprintf(fp,
				"ip6tables -t nat -D POSTROUTING -o br+ -s $(nvram get openvpn_v6netmask) -j MASQUERADE >/dev/null 2>&1\n");
		}
#endif
	}

	/* egc: delete interface as listen interface to DNSMasq */
	if (nvram_match("openvpn_tuntap", "tun")) {
		fprintf(fp, "nvram unset dnsmasq_addifvpn\n");
		fprintf(fp, "service dnsmasq restart\n");
	}
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
		fprintf(fp, "brctl delif br0 $dev\n"
			    "ifconfig $dev down\n");
	fprintf(fp, "exit 0\n");
	fclose(fp);
	chmod("/tmp/openvpn/route-up.sh", 0700);
	chmod("/tmp/openvpn/route-down.sh", 0700);
	eval("ln", "-s", "/usr/sbin/openvpn", "/tmp/openvpnserver");
	run_openvpn("/tmp/openvpnserver", "openvpn");
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
		//remove ebtables rules on shutdown done with route-down script
		//system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap2 --pkttype-type multicast -j DROP");
		eval("ebtables", "-t", "nat", "-D", "POSTROUTING", "-o", "tap2", "-p", "ipv4", "--ip-proto", "udp", "--ip-sport",
		     "67:68", "--ip-dport", "67:68", "-j", "DROP");
		eval("ebtables", "-t", "nat", "-D", "PREROUTING", "-i", "tap2", "-p", "ipv4", "--ip-proto", "udp", "--ip-sport",
		     "67:68", "--ip-dport", "67:68", "-j", "DROP");
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
		unlink("/tmp/openvpnsrv_fw.sh");
		unlink("/tmp/openvpn/openvpn-auth");
		unlink("/tmp/openvpn/quickAuth.sh");
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
	char wan_if_buffer[33];
	if (nvram_invmatchi("openvpncl_enable", 1))
		return;
	eval("modprobe", "geniv");
	eval("modprobe", "seqiv");
	eval("modprobe", "ghash_generic");
	eval("modprobe", "ctr");
	eval("modprobe", "chacha20_generic");
	eval("modprobe", "chacha20_x86_64");
	eval("modprobe", "chacha20_ssse3-x86_64");
	eval("modprobe", "chacha20_avx512vl-x86_64");
	eval("modprobe", "chacha20_avx2-x86_64");
	eval("modprobe", "poly1305_generic");
	eval("modprobe", "poly1305_x86_64");
	eval("modprobe", "chacha20poly1305");
	eval("modprobe", "gcm");
	eval("modprobe", "ccm");
	eval("modprobe", "ovpn-dco-v2");
	insmod("tun");
	dd_loginfo("openvpn", "OpenVPN daemon (Client) starting/restarting...");
	mkdir("/tmp/openvpncl", 0700);
	write_nvram("/tmp/openvpncl/ca.crt", "openvpncl_ca");
	write_nvram("/tmp/openvpncl/client.crt", "openvpncl_client");
	write_nvram("/tmp/openvpncl/client.key", "openvpncl_key");
	write_nvram("/tmp/openvpncl/ta.key", "openvpncl_tlsauth");
	write_nvram("/tmp/openvpncl/cert.p12", "openvpncl_pkcs12");
	write_nvram("/tmp/openvpncl/static.key", "openvpncl_static");
	chmod("/tmp/openvpncl/client.key", 0600);

	//Start PBR routing and general killswitch if appropriate
	if (*(nvram_safe_get("openvpncl_route")) && nvram_invmatch("openvpncl_spbr", "0")) {
		dd_loginfo("openvpn", "PBR via tunnel now using setroute_pbr(): %s", nvram_safe_get("openvpncl_route"));
		setroute_pbr("10");
	}
	//In interface
	if (nvram_match("wan_proto", "disabled")) {
		dd_loginfo(
			"openvpn",
			"Router is not in Gateway mode, WAP detected will try to adjust firewall rules but checking is necessary!");
		strcpy(IN_IF, "-i br0");
	}
	if (nvram_matchi("openvpncl_killswitch", 1) &&
	    nvram_invmatch("openvpncl_spbr",
			   "1")) { //if no VPN PBR and killswitch active, activate general killswitch
		dd_loginfo("openvpn", "General Killswitch for OpenVPN enabled from OpenVPN");
		//eval("restart" , "firewall");
		//eval("iptables", "-D", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//eval("iptables", "-I", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//eval("iptables", "-D", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-m", "state", "--state", "NEW", "-j", "DROP");
		//eval("iptables", "-I", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-m", "state", "--state", "NEW", "-j", "DROP");
		//todo set IN_IF to deal with WAP
		eval("iptables", "-D", "FORWARD", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		eval("iptables", "-I", "FORWARD", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//consider restarting SFE to drop existing connections e.g. eval("restart", "sfe"); or: stop_sfe(); start_sfe();
#ifdef HAVE_SFE
		if (nvram_match("sfe", "1")) {
			stop_sfe();
			start_sfe();
		}
		system("cat /proc/net/ip_conntrack_flush 2>&1");
		system("cat /proc/sys/net/netfilter/nf_conntrack_flush 2>&1");
#endif
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
	if (nvram_matchi("openvpncl_tls_btn", 2) && nvram_invmatch("openvpncl_static", "")) { //static key
		fprintf(fp, "secret /tmp/openvpncl/static.key\n");
	} else if (nvram_matchi("openvpncl_pkcs", 1) && nvram_invmatch("openvpncl_pkcs12", "")) {
		fprintf(fp, "pkcs12 /tmp/openvpncl/cert.p12\n");
	} else {
		if (nvram_invmatch("openvpncl_ca", ""))
			fprintf(fp, "ca /tmp/openvpncl/ca.crt\n");
		if (nvram_invmatch("openvpncl_client", ""))
			fprintf(fp, "cert /tmp/openvpncl/client.crt\n");
		if (nvram_invmatch("openvpncl_key", ""))
			fprintf(fp, "key /tmp/openvpncl/client.key\n");
	}
	/*
#ifdef HAVE_ERC
	fprintf(fp, "management 127.0.0.1 5001\n" "management-log-cache 100\n" "verb 3\n" "mute 3\n" "syslog\n" "writepid /var/run/openvpncl.pid\n" "resolv-retry infinite\n" "nobind\n" "script-security 2\n");
#else
	fprintf(fp, "management 127.0.0.1 16\n" "management-log-cache 100\n" "verb 3\n" "mute 3\n" "syslog\n" "writepid /var/run/openvpncl.pid\n" "resolv-retry infinite\n" "nobind\n" "script-security 2\n");
#endif
*/

#ifdef HAVE_ERC
	fprintf(fp, "management 127.0.0.1 5001\n");
#else
	fprintf(fp, "management 127.0.0.1 16\n");
#endif
	fprintf(fp, "management-log-cache 100\n"
		    "verb 3\n"
		    "mute 3\n"
		    "syslog\n"
		    "writepid /var/run/openvpncl.pid\n"
		    "resolv-retry infinite\n"
		    "script-security 2\n");
	if (!nvram_matchi("openvpncl_nobind", 0)) {
		fprintf(fp, "nobind\n");
	}
	if (!nvram_matchi(
		    "openvpncl_tls_btn",
		    2)) { //if not openvpncl_tls_btn=2 then Static key is used and the PKI boxes are hidden and client is not set
		fprintf(fp, "client\n");
	}
	fprintf(fp, "dev %s\n", ovpniface);
	fprintf(fp, "proto %s\n", nvram_safe_get("openvpncl_proto"));
	if (nvram_invmatch("openvpncl_cipher", ""))
		fprintf(fp, "cipher %s\n", nvram_safe_get("openvpncl_cipher"));
	if (nvram_invmatch("openvpncl_auth", ""))
		fprintf(fp, "auth %s\n", nvram_safe_get("openvpncl_auth"));
	//egc
	char dcbuffer[128] = { 0 };
	char *dc1 = nvram_safe_get("openvpncl_dc1");
	char *dc2 = nvram_safe_get("openvpncl_dc2");
	char *dc3 = nvram_safe_get("openvpncl_dc3");
	sprintf(dcbuffer, "%s%s%s%s%s", dc1, *dc2 ? ":" : "", dc2, *dc3 ? ":" : "", dc3);

	if (dcbuffer[0])
		fprintf(fp, "data-ciphers %s\n", dcbuffer);

	if (nvram_matchi("openvpncl_upauth", 1))
		fprintf(fp, "auth-user-pass %s\n", "/tmp/openvpncl/credentials");
	fprintf(fp, "remote %s %s\n", nvram_safe_get("openvpncl_remoteip"), nvram_safe_get("openvpncl_remoteport"));
	if (nvram_matchi("openvpncl_multirem", 1)) {
		fprintf(fp, "server-poll-timeout 20\n");
		if (nvram_matchi("openvpncl_randomsrv", 1))
			fprintf(fp, "remote-random\n");
		int i;
		char tempip[32] = { 0 };
		char tempport[32] = { 0 };
		for (i = 2; i < 6; i++) {
			sprintf(tempip, "openvpncl_remoteip%d", i);
			sprintf(tempport, "openvpncl_remoteport%d", i);
			if (nvram_invmatch(tempip, "") && nvram_invmatch(tempport, ""))
				fprintf(fp, "remote %s %s\n", nvram_safe_get(tempip), nvram_safe_get(tempport));
		}
	}
	if (nvram_invmatch("openvpncl_scramble", "off")) {
		fprintf(fp, "disable-dco\n");
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
			fprintf(fp, "comp-lzo %s\n", //yes/no/adaptive/disable
				nvram_safe_get("openvpncl_lzo"));
	}
	if (*(nvram_safe_get("openvpncl_route")) && nvram_matchi("openvpncl_spbr",
								 1)) { //policy routing: we need redirect-gw so we get gw info
		fprintf(fp, "redirect-private def1\n");
		if (nvram_invmatch("openvpncl_tuntap", "tun"))
			fprintf(fp, "ifconfig-noexec\n");
		else
			fprintf(fp, "pull-filter ignore \"redirect-gateway\"\n");
	}
	if (nvram_invmatch("openvpncl_auth", "none") && nvram_invmatchi("openvpncl_tlscip",
									0)) //not needed if we have no auth anyway
		fprintf(fp, "tls-client\n");
	if (nvram_invmatch("openvpncl_mtu", "") && nvram_invmatchi("openvpncl_mtu", 0))
		fprintf(fp, "tun-mtu %s\n", nvram_safe_get("openvpncl_mtu"));
	if (nvram_invmatch("openvpncl_fragment", "") &&
	    (nvram_match("openvpncl_proto", "udp") || nvram_match("openvpncl_proto", "udp4") ||
	     nvram_match("openvpncl_proto", "udp6"))) {
		fprintf(fp, "fragment %s\n", nvram_safe_get("openvpncl_fragment"));
		if (nvram_matchi("openvpncl_mssfix", 1))
			fprintf(fp,
				"mssfix\n"); //mssfix=1450 (default), should be set on one side only. when fragment->=mss
	} else
		fprintf(fp, "mtu-disc yes\n");
	if (nvram_matchi("openvpncl_certtype", 1))
		fprintf(fp, "remote-cert-tls server\n");
	if (nvram_match("openvpncl_proto", "udp") || nvram_match("openvpncl_proto", "udp4") ||
	    nvram_match("openvpncl_proto", "udp6"))
		fprintf(fp,
			"fast-io\n"); //experimental!improving CPU efficiency by 5%-10%
	//      if (nvram_match("openvpncl_tuntap", "tun"))
	//              fprintf(fp, "tun-ipv6\n");      //enable ipv6 support.
	if (*(nvram_safe_get("openvpncl_tlsauth"))) {
		if (nvram_matchi("openvpncl_tls_btn", 1))
			fprintf(fp,
				"tls-crypt /tmp/openvpncl/ta.key\n"); //egc: tls_btn 1 is tls-crypt, 0 is tls-auth, 2 is static key, 4 is tls-crypt-v2
		else if (nvram_matchi("openvpncl_tls_btn", 4))
			fprintf(fp, "tls-crypt-v2 /tmp/openvpncl/ta.key\n");
		else if (nvram_matchi("openvpncl_tls_btn", 0))
			fprintf(fp, "tls-auth /tmp/openvpncl/ta.key 1\n");
	}
	if (nvram_invmatchi("openvpncl_tlscip", 0))
		fprintf(fp, "tls-cipher %s\n", nvram_safe_get("openvpncl_tlscip"));
	/* for QOS */
	if (nvram_matchi("wshaper_enable", 1))
		fprintf(fp, "passtos\n");
	//egc: add route-up and down to .conf
	fprintf(fp, "route-up /tmp/openvpncl/route-up.sh\n");
	fprintf(fp, "route-pre-down /tmp/openvpncl/route-down.sh\n");
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
	if (nvram_match("openvpncl_tuntap", "tap") && nvram_matchi("openvpncl_bridge", 1) && nvram_matchi("openvpncl_nat", 0)) {
		fprintf(fp,
			"brctl addif br0 %s\n"
			"ifconfig %s 0.0.0.0 up\n",
			ovpniface,
			ovpniface); //non promisc for performance reasons
		if (nvram_match("openvpncl_tuntap", "tap") && *(nvram_safe_get("openvpncl_ip")))
			fprintf(fp, "ifconfig %s %s netmask %s up\n", ovpniface, nvram_safe_get("openvpncl_ip"),
				nvram_safe_get("openvpncl_mask"));
	}
	if (nvram_matchi("wshaper_enable", 1)) {
		fprintf(fp, "startservice set_routes -f\n");
		fprintf(fp, "restart firewall\n");
		fprintf(fp, "sleep 5\n");
	}
	create_openvpnrules(fp);
	fclose(fp);
	//route-pre-down script
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
	if (nvram_match("openvpncl_tuntap", "tap") && nvram_matchi("openvpncl_bridge", 1) && nvram_matchi("openvpncl_nat", 0))
		fprintf(fp,
			"brctl delif br0 %s\n"
			"ifconfig %s down\n",
			ovpniface, ovpniface);
	else if (nvram_match("openvpncl_tuntap", "tap") && *(nvram_safe_get("openvpncl_ip")))
		fprintf(fp, "ifconfig $dev down\n");

	if (nvram_matchi("openvpncl_nat", 1)) {
		fprintf(fp, "iptables -D POSTROUTING -t nat -o $dev -j MASQUERADE\n");
	}
	if (nvram_matchi("openvpncl_fw", 0)) {
		if (nvram_match("openvpncl_tuntap", "tun")) {
			fprintf(fp, "iptables -D INPUT -i $dev -m state --state NEW -j ACCEPT\n"
				    "iptables -D FORWARD -i $dev -m state --state NEW -j ACCEPT\n");
		}
	}
	if (*(nvram_safe_get("openvpncl_route"))) { //policy based routing
		write_nvram("/tmp/openvpncl/policy_ips", "openvpncl_route");
		fprintf(fp, "if [[ \"$(nvram get openvpncl_enable)\" == '0' ]]; then\n");
		fprintf(fp,
			"while ip rule delete from 0/0 to 0/0 table 10; do true; done\n"); //egc: added to delete ip rules
		fprintf(fp, "ip route flush table 10\n");
		fprintf(fp, "ip route flush cache\n"
			    "fi\n");
	}
	if (nvram_match("openvpncl_tuntap", "tun")) {
		fprintf(fp,
			"[ -f /tmp/resolv.dnsmasq_isp ] && cp -f /tmp/resolv.dnsmasq_isp /tmp/resolv.dnsmasq && nvram unset openvpn_get_dns\n");
	}
	if (nvram_match("openvpncl_mit", "1")) {
		fprintf(fp, "[[ ! -z \"$ifconfig_netmask\" ]] && vpn_netmask=\"/$ifconfig_netmask\"\n");
		fprintf(fp, "iptables -t raw -D PREROUTING ! -i $dev -d $ifconfig_local$vpn_netmask -j DROP\n");
	}
	if (nvram_match("openvpncl_splitdns", "1")) {
		fprintf(fp, "ovpndel=\"/tmp/OVPNDEL\"\n"
			    "if [[ -f \"$ovpndel\" ]]; then\n"
			    "(while read line; do $line >/dev/null 2>&1; done < $ovpndel)\n"
			    "rm -f $ovpndel\n"
			    "fi\n");
	}
	//code to delete chains for PBR via WAN
	if (nvram_matchi("openvpncl_killswitch", 1) && nvram_matchi("openvpncl_spbr", 2)) {
		fprintf(fp, "iptables -D FORWARD %s -o $(get_wanface) -j vpn-pbr\n", IN_IF);
		fprintf(fp, "iptables -F vpn-pbr\n"
			    "iptables -X vpn-pbr\n");
	}
	// remove ebtales rules
	if (nvram_matchi("openvpncl_blockmulticast", 1) && nvram_match("openvpncl_tuntap", "tap") &&
	    nvram_matchi("openvpncl_bridge", 1))
		fprintf(fp, "ebtables -t nat -D POSTROUTING -o $dev --pkttype-type multicast -j DROP\n");
	/*
	if (nvram_matchi("block_multicast",0) //block multicast on bridged vpns
				&& nvram_match("openvpncl_tuntap", "tap")
				&& nvram_matchi("openvpncl_bridge",1)) {
				fprintf(fp,
-						"ebtables -t nat -D POSTROUTING -o $dev --pkttype-type multicast -j DROP\n"
-						"ebtables -t nat -D PREROUTING -i $dev --pkttype-type multicast -j DROP\n"
						"if [ `ebtables -t nat -L|grep -e '-j' -c` -ne 0 ]\n"
						"then rmmod ebtable_nat\n" "\t rmmod ebtables\n", ovpniface, ovpniface);
		} 
*/
	fprintf(fp, "ip route flush cache\n"
		    "exit 0\n");
	fclose(fp);

	chmod("/tmp/openvpncl/route-up.sh", 0700);
	chmod("/tmp/openvpncl/route-down.sh", 0700);
	run_openvpn("openvpn", "openvpncl");
	//start openvpn-watchdog with control script to kill running
	if (nvram_matchi("openvpncl_wdog", 1)) {
		if (nvram_match("openvpncl_tuntap", "tap") && nvram_matchi("openvpncl_bridge", 1) && nvram_matchi("openvpncl_nat", 0)) {
			eval("/usr/bin/controlovpnwdog.sh", "1", "1"); // use br0 as interface to ping
		} else {
			eval("/usr/bin/controlovpnwdog.sh", "1", "0");
		}
	}
	return;
}

void stop_openvpn(void)
{
	char wan_if_buffer[33];
	nvram_unset("openvpn_get_dns");
	if (stop_process("openvpn", "OpenVPN daemon (Client)")) {
		//remove ebtables rules on shutdown. done with route-down script
		//system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP");
		unlink("/tmp/openvpncl/ca.crt");
		unlink("/tmp/openvpncl/client.crt");
		unlink("/tmp/openvpncl/client.key");
		unlink("/tmp/openvpncl/ta.key");
		unlink("/tmp/openvpncl/cert.p12");
		unlink("/tmp/openvpncl/static.key");
		unlink("/tmp/openvpncl/openvpn.conf");
		unlink("/tmp/openvpncl/route-up.sh");
		unlink("/tmp/openvpncl/route-down.sh");
		unlink("/tmp/openvpncl_fw.sh"); //remove created firewall rules to prevent used by Firewall if VPN is down
		unlink("/tmp/OVPNDEL");
		// remove pbr table
		cleanup_pbr("10");
		//remove kill switch
		//eval("iptables", "-D", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//eval("iptables", "-D", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-m", "state", "--state", "NEW", "-j", "DROP");
		eval("iptables", "-D", "FORWARD", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//dd_loginfo("openvpn", "General Killswitch for OpenVPN removed in 2 using wanface %s", safe_get_wan_face(wan_if_buffer));
		// to kill running watchdog
		eval("/usr/bin/controlovpnwdog.sh", "0");
	}
}

void stop_openvpn_wandone(void)
{
	char wan_if_buffer[33];

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
		//remove ebtables rules on shutdown done with route-down script
		//system("/usr/sbin/ebtables -t nat -D POSTROUTING -o tap1 --pkttype-type multicast -j DROP");
		unlink("/tmp/openvpncl/ca.crt");
		unlink("/tmp/openvpncl/client.crt");
		unlink("/tmp/openvpncl/client.key");
		unlink("/tmp/openvpncl/ta.key");
		unlink("/tmp/openvpncl/cert.p12");
		unlink("/tmp/openvpncl/static.key");
		unlink("/tmp/openvpncl/openvpn.conf");
		unlink("/tmp/openvpncl/route-up.sh");
		unlink("/tmp/openvpncl/route-down.sh");
		unlink("/tmp/openvpncl_fw.sh");
		unlink("/tmp/OVPNDEL");
		// remove pbr table
		cleanup_pbr("10");
		//remove kill switch
		//eval("iptables", "-D", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//eval("iptables", "-D", "FORWARD", "-i", "br+", "-o", safe_get_wan_face(wan_if_buffer), "-m", "state", "--state", "NEW", "-j", "DROP");
		eval("iptables", "-D", "FORWARD", "-o", safe_get_wan_face(wan_if_buffer), "-j", "DROP");
		//dd_loginfo("openvpn", "General Killswitch for OpenVPN removed in 3 using wanface %s", safe_get_wan_face(wan_if_buffer));
		// to kill running watchdog
		eval("/usr/bin/controlovpnwdog.sh", "0");
	}
}
#endif
