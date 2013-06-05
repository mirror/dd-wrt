/*
 * pptp.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_PPTPD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <services.h>

int  jffs = 0;

if ((nvram_match("usb_enable", "1")
	&& nvram_match("usb_storage", "1")
	&& nvram_match("usb_automnt", "1")
	&& nvram_match("usb_mntpoint", "jffs"))
	|| (nvram_match("enable_jffs2", "1")
	&& nvram_match("jffs_mounted", "1")
	&& nvram_match("sys_enable_jffs2", "1")))
		jffs = 1;

void start_pptpd(void)
{
	int ret = 0, mss = 0;
	char *lpTemp;
	FILE *fp;

	if (!nvram_invmatch("pptpd_enable", "0")) {
		stop_pptpd();
		return;
	}
#ifdef HAVE_PPTP_ACCEL
	insmod("pptp");
#endif
		
	// cprintf("stop vpn modules\n");
	// stop_vpn_modules ();

	//      copy existing peer data to /tmp
	mkdir("/jffs/etc", 0700);
	mkdir("/jffs/etc/pptpd", 0700);
	if (jffs == 1)
		system("/bin/cp /jffs/etc/pptpd/pptp_peer.db /tmp/");

	// Create directory for use by pptpd daemon and its supporting files
	mkdir("/tmp/pptpd", 0744);
	cprintf("open options file\n");
	// Create options file that will be unique to pptpd to avoid interference 
	// with pppoe and pptp
	fp = fopen("/tmp/pptpd/options.pptpd", "w");
	if (nvram_match("pptpd_radius", "1")) {
		cprintf("adding radius plugin\n");
		fprintf(fp, "plugin radius.so\nplugin radattr.so\n" 
			"radius-config-file /tmp/pptpd/radius/radiusclient.conf\n");
	}
	cprintf("check if wan_wins = zero\n");
	int nowins = 0;

	if (nvram_match("wan_wins", "0.0.0.0")) {
		nvram_set("wan_wins", "");
		nowins = 1;
	}
	if (strlen(nvram_safe_get("wan_wins")) == 0)
		nowins = 1;

	cprintf("write config\n");
	fprintf(fp, "lock\n" "name *\n" "nobsdcomp\n" "nodeflate\n" "auth\n" 
		"refuse-pap\n" "refuse-eap\n" "refuse-chap\n" "refuse-mschap\n" 
		"require-mschap-v2\n");
	if (nvram_match("pptpd_forcemppe", "1"))
		fprintf(fp, "mppe required,stateless,no40,no56\n");
	else
		fprintf(fp, "mppe stateless\n");
	fprintf(fp, "mppc\n"	//enable compression
		"debug\n" "logfd 2\n"
		"ms-ignore-domain\n"
		"chap-secrets /tmp/pptpd/chap-secrets\n"
		"ip-up-script /tmp/pptpd/ip-up\n" "ip-down-script /tmp/pptpd/ip-down\n" 
		"proxyarp\n" "ipcp-accept-local\n" "ipcp-accept-remote\n" 
		"lcp-echo-failure 15\n" "lcp-echo-interval 4\n"
//              "lcp-echo-adaptive"     //disable interval
		"mtu %s\n" "mru %s\n", nvram_safe_get("pptpd_mtu"), nvram_safe_get("pptpd_mru"));
	if (!nowins) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("wan_wins"));
	}
	if (strlen(nvram_safe_get("pptpd_wins1"))) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("pptpd_wins1"));
	}
	if (strlen(nvram_safe_get("pptpd_wins2"))) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("pptpd_wins2"));
	}

	struct dns_lists *dns_list = get_dns_list();

	if (nvram_match("dnsmasq_enable", "1")) {
		if (nvram_invmatch("lan_ipaddr", ""))
			fprintf(fp, "ms-dns %s\n", nvram_safe_get("lan_ipaddr"));
	} else if (nvram_match("local_dns", "1")) {
		if (dns_list && (nvram_invmatch("lan_ipaddr", "")
				 || strlen(dns_list->dns_server[0]) > 0 || strlen(dns_list->dns_server[1]) > 0 || strlen(dns_list->dns_server[2]) > 0)) {

			if (nvram_invmatch("lan_ipaddr", ""))
				fprintf(fp, "ms-dns %s\n", nvram_safe_get("lan_ipaddr"));
			if (strlen(dns_list->dns_server[0]) > 0)
				fprintf(fp, "ms-dns %s\n", dns_list->dns_server[0]);
			if (strlen(dns_list->dns_server[1]) > 0)
				fprintf(fp, "ms-dns %s\n", dns_list->dns_server[1]);
			if (strlen(dns_list->dns_server[2]) > 0)
				fprintf(fp, "ms-dns %s\n", dns_list->dns_server[2]);
		}
	} else {
		if (dns_list && (strlen(dns_list->dns_server[0]) > 0 || strlen(dns_list->dns_server[1]) > 0 || strlen(dns_list->dns_server[2]) > 0)) {
			if (strlen(dns_list->dns_server[0]) > 0)
				fprintf(fp, "ms-dns  %s\n", dns_list->dns_server[0]);
			if (strlen(dns_list->dns_server[1]) > 0)
				fprintf(fp, "ms-dns  %s\n", dns_list->dns_server[1]);
			if (strlen(dns_list->dns_server[2]) > 0)
				fprintf(fp, "ms-dns  %s\n", dns_list->dns_server[2]);
		}
	}
	if (dns_list)
		free(dns_list);
	if (strlen(nvram_safe_get("pptpd_dns1"))) {
		fprintf(fp, "ms-dns %s\n", nvram_safe_get("pptpd_dns1"));
	}
	if (strlen(nvram_safe_get("pptpd_dns2"))) {
		fprintf(fp, "ms-dns %s\n", nvram_safe_get("pptpd_dns2"));
	}
	//	use jffs/usb for auth scripts if available
	if (jffs == 1) {	
//		if (strlen(nvram_safe_get("openvpn_ccddef")) > 0) {	//
			fprintf(fp, "auth-up /jffs/etc/pptpd/auth-up.sh\n");
			fprintf(fp, "auth-down /jffs/etc/pptpd/auth-down.sh\n");
			if ((fp = fopen("/jffs/etc/pptpd/auth-up.sh", "r")) == NULL) {
				fclose(fp);
				fp = fopen("/jffs/etc/pptpd/auth-up.sh", "w");
					fprintf(fp, "#!/bin/sh\n");
				fclose(fp);
				chmod("/jffs/etc/pptpd/auth-up.sh", 0700);
				}
			if ((fp = fopen("/jffs/etc/pptpd/auth-down.sh", "r")) == NULL) {
				fclose(fp);		
				fp = fopen("/jffs/etc/pptpd/auth-down.sh", "w");
					fprintf(fp, "#!/bin/sh\n");
				fclose(fp);
				chmod("/jffs/etc/pptpd/auth-down.sh", 0700);
			}
		}
//	}
	else {
/*		fprintf(fp, "connect /tmp/pptpd/auth-up.sh\n");
		fprintf(fp, "disconnect /tmp/pptpd/auth-down.sh\n");
		fp = fopen("/jffs/etc/pptpd/auth-up.sh", "w");
			fprintf(fp, "#!/bin/sh\n");
		fclose(fp);
		fp = fopen("/jffs/etc/pptpd/auth-down.sh", "w");
			fprintf(fp, "#!/bin/sh\n");
		fclose(fp);
		chmod("/tmp/pptpd/auth-up.sh", 0700);
		chmod("/tmp/pptpd/auth-down.sh", 0700);	*/
		}

	// Following is all crude and need to be revisited once testing confirms
	// that it does work
	// Should be enough for testing..
	if (nvram_match("pptpd_radius", "1")) {
		if (nvram_get("pptpd_radserver") != NULL && nvram_get("pptpd_radpass") != NULL) {
			fclose(fp);

//			if (nvram_match("pptpd_radip"), "1")	//use radius for ip's // nvarm var missing :-)
//				fprintf(fp, "delegate\n");
				
			mkdir("/tmp/pptpd/radius", 0744);

			fp = fopen("/tmp/pptpd/radius/radiusclient.conf", "w");
			fprintf(fp, "auth_order radius\n"
				"login_tries 4\n"
				"login_timeout 60\n"
				"radius_timeout 10\n"
				"nologin /etc/nologin\n"
				"servers /tmp/pptpd/radius/servers\n"
				"dictionary /etc/dictionary\n"
				"seqfile /var/run/radius.seq\n"
				"mapfile /etc/port-id-map\n" "radius_retries 3\n" "authserver %s:%s\n", nvram_get("pptpd_radserver"), nvram_get("pptpd_radport") ? nvram_get("pptpd_radport") : "radius");

			if (nvram_get("pptpd_radserver") != NULL && nvram_get("pptpd_acctport") != NULL)
				fprintf(fp, "acctserver %s:%s\n", nvram_get("pptpd_radserver"), nvram_get("pptpd_acctport") ? nvram_get("pptpd_acctport") : "radacct");
			fclose(fp);

			fp = fopen("/tmp/pptpd/radius/servers", "w");
			fprintf(fp, "%s\t%s\n", nvram_get("pptpd_radserver"), nvram_get("pptpd_radpass"));
			fclose(fp);

		} else
			fclose(fp);
	} else
		fclose(fp);

	// Create pptpd.conf options file for pptpd daemon
	fp = fopen("/tmp/pptpd/pptpd.conf", "w");
	if (nvram_match("pptpd_bcrelay", "1"))
		fprintf(fp, "bcrelay %s\n", nvram_safe_get("lan_ifname"));
	fprintf(fp, "connections %s\nlocalip %s\n" "remoteip %s\n", nvram_safe_get("pptpd_conn"), nvram_safe_get("pptpd_lip"), nvram_safe_get("pptpd_rip"));
	fclose(fp);

	// Create ip-up and ip-down scripts that are unique to pptpd to avoid
	// interference with pppoe and pptp
	/*
	 * adjust for tunneling overhead (mtu - 40 byte IP - 108 byte tunnel
	 * overhead) 
	 */
	if (nvram_match("mtu_enable", "1"))
		mss = atoi(nvram_safe_get("wan_mtu")) - 40 - 108;
	else
		mss = 1500 - 40 - 108;
	char bcast[32];

	strcpy(bcast, nvram_safe_get("lan_ipaddr"));
	get_broadcast(bcast, nvram_safe_get("lan_netmask"));

	fp = fopen("/tmp/pptpd/ip-up", "w");
	fprintf(fp, "#!/bin/sh\n" "startservice set_routes\n"	// reinitialize 
		"echo $PPPD_PID $1 $5 $6 $PEERNAME >> /tmp/pptp_connected\n" "iptables -I INPUT -i $1 -j ACCEPT\n" "iptables -I FORWARD -i $1 -j ACCEPT\n"	//
		"iptables -I FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n" "iptables -t nat -I PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s\n"	// rule for wake on lan over pptp tunnel
		"%s\n", bcast, nvram_get("pptpd_ipdown_script") ? nvram_get("pptpd_ipdown_script") : "");
	//      per peer shaping                
	if (nvram_match("pptpd_radius", "1"))
		fprintf(fp, "IN=`grep -i RP-Upstream-Speed-Limit /var/run/radattr.$1 | awk '{print $2}'`\n" "OUT=`grep -i RP-Downstream-Speed-Limit /var/run/radattr.$1 | awk '{print $2}'`\n" "if [ ! -z $IN ] && [ $IN -gt 0 ]\n"	//Speed limit !0 and !empty
			"then	tc qdisc del root dev $1\n"
			"\t tc qdisc add dev $1 handle ffff: ingress\n"
			"\t tc filter add dev $1 parent ffff: protocol ip prio 50 u32 match ip src 0.0.0.0/0 police rate \"$IN\"kbit burst \"$IN\"kbit drop flowid :1\n"
			"fi\n" "if [ ! -z $OUT ] && [ $OUT -gt 0 ]\n" "then tc qdisc del dev $1 ingress\n" "\t tc qdisc add dev $1 root tbf rate \"$OUT\"kbit latency 50ms burst \"$OUT\"kbit\n" "fi\n");
	fclose(fp);
	fp = fopen("/tmp/pptpd/ip-down", "w");
	fprintf(fp, "#!/bin/sh\n" "grep -v $PPPD_PID /tmp/pptp_connected > /tmp/pptp_connected.tmp\n" "mv /tmp/pptp_connected.tmp /tmp/pptp_connected\n"
		//      calc connected time and volume per peer
		"CONTIME=$(($CONNECT_TIME+`grep $PEERNAME /tmp/pptp_peer.db | awk '{print $3}'`))\n" "SENT=$(($BYTES_SENT+`grep $PEERNAME /tmp/pptp_peer.db | awk '{print $4}'`))\n" "RCVD=$(($BYTES_RCVD+`grep $PEERNAME /tmp/pptp_peer.db | awk '{print $5}'`))\n" "grep -v $PEERNAME /tmp/ppp_peer.db > /tmp/pptp_peer.db.tmp\n" "mv /tmp/pptp_peer.db.tmp /tmp/pptp_peer.db\n" "echo \"$PEERNAME $CONTIME $SENT $RCVD\" >> /tmp/pptp_peer.db\n" "iptables -D FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n" "iptables -D INPUT -i $1 -j ACCEPT\n" "iptables -D FORWARD -i $1 -j ACCEPT\n" "iptables -t nat -D PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s\n"	// rule for wake on lan over pptp tunnel
		"%s\n", bcast, nvram_get("pptpd_ipdown_script") ? nvram_get("pptpd_ipdown_script") : "");
	if (nvram_match("pptpd_radius", "1"))
		fprintf(fp, "tc qdisc del root dev $1\n" "tc qdisc del ingress dev $1\n");
	fclose(fp);
	chmod("/tmp/pptpd/ip-up", 0744);
	chmod("/tmp/pptpd/ip-down", 0744);

	// Exctract chap-secrets from nvram and add the default account with
	// routers password
	lpTemp = nvram_safe_get("pptpd_auth");
	fp = fopen("/tmp/pptpd/chap-secrets", "w");
	// fprintf (fp, "root\t*\t%s\t*\n", nvram_safe_get ("http_passwd"));
	if (strlen(lpTemp) != 0)
		fprintf(fp, "%s\n", lpTemp);
	fclose(fp);

	chmod("/tmp/pptpd/chap-secrets", 0600);

	// Execute pptpd daemon
	ret = eval("pptpd", "-c", "/tmp/pptpd/pptpd.conf", "-o", "/tmp/pptpd/options.pptpd");

	dd_syslog(LOG_INFO, "pptpd : pptp daemon successfully started\n");
	return;
}

void stop_pptpd(void)
{

	stop_process("pptpd", "pptp server");
	stop_process("bcrelay", "pptp broadcast relay");
	unlink("/tmp/pptp_connected");
	if (jffs == 1)
		system("/bin/cp /tmp/pptp_peer.db /jffs/etc/pptpd/");
#ifdef HAVE_PPTP_ACCEL
	rmmod("pptp");
#endif
	dd_syslog(LOG_INFO, "pptpd : pptp daemon successfully stoped\n");
	return;
}
#endif
