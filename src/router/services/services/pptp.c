/*
 * pptp.c
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

static int jffs = 0;

void start_pptpd(void)
{
	char *lpTemp;
	FILE *fp;
	int i;

	if (!nvram_invmatchi("pptpd_enable", 0)) {
		return;
	}

	stop_pptpd();

	if ((nvram_matchi("usb_enable", 1) && nvram_matchi("usb_storage", 1) && nvram_matchi("usb_automnt", 1) &&
	     nvram_match("usb_mntpoint", "jffs")) ||
	    jffs_mounted())
		jffs = 1;

	insmod("gre");
	insmod("pptp");

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
	if (nvram_matchi("pptpd_radius", 1)) {
		cprintf("adding radius plugin\n");
		fprintf(fp, "plugin radius.so\nplugin radattr.so\n"
			    "radius-config-file /tmp/pptpd/radius/radiusclient.conf\n");
	}
	cprintf("check if wan_wins = zero\n");
	int nowins = 0;

	if (nvram_default_match("wan_wins", "0.0.0.0", "0.0.0.0")) {
		nowins = 1;
	}

	cprintf("write config\n");
	fprintf(fp, "lock\n"
		    "name *\n"
		    "nobsdcomp\n"
		    "nodeflate\n"
		    "auth\n"
		    "refuse-pap\n"
		    "refuse-eap\n"
		    "refuse-chap\n"
		    "refuse-mschap\n"
		    "require-mschap-v2\n");
	if (nvram_matchi("pptpd_forcemppe", 1))
		fprintf(fp, "mppe required,stateless,no40,no56\n");
	else
		fprintf(fp, "mppe stateless\n");
	fprintf(fp,
		"mppc\n" //enable compression
		"debug\n"
		"logfd 2\n" //
		"ms-ignore-domain\n" //
		"chap-secrets /tmp/pptpd/chap-secrets\n" //
		"ip-up-script /tmp/pptpd/ip-up\n" //
		"ip-down-script /tmp/pptpd/ip-down\n" //
		"proxyarp\n" //
		"ipcp-accept-local\n" //
		"ipcp-accept-remote\n" //
		"lcp-echo-failure 15\n" //
		"lcp-echo-interval 4\n" //
		"lcp-echo-adaptive\n" //
		"mtu %s\n" //
		"mru %s\n",
		nvram_safe_get("pptpd_mtu"), nvram_safe_get("pptpd_mru"));
	if (!nowins) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("wan_wins"));
	}
	if (*(nvram_safe_get("pptpd_wins1"))) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("pptpd_wins1"));
	}
	if (*(nvram_safe_get("pptpd_wins2"))) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("pptpd_wins2"));
	}

	struct dns_lists *dns_list = get_dns_list(0);

	if (nvram_matchi("dnsmasq_enable", 1)) {
		if (nvram_invmatch("lan_ipaddr", ""))
			fprintf(fp, "ms-dns %s\n", nvram_safe_get("lan_ipaddr"));
	} else {
		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				fprintf(fp, "ms-dns %s\n", dns_list->dns_server[i].ip);
		}
	}
	free_dns_list(dns_list);
	if (*(nvram_safe_get("pptpd_dns1"))) {
		fprintf(fp, "ms-dns %s\n", nvram_safe_get("pptpd_dns1"));
	}
	if (*(nvram_safe_get("pptpd_dns2"))) {
		fprintf(fp, "ms-dns %s\n", nvram_safe_get("pptpd_dns2"));
	}
	//      use jffs/usb for auth scripts if available
#if 0
	if (jffs == 1) {
		fprintf(fp, "auth-up /jffs/etc/pptpd/auth-up.sh\n");
		fprintf(fp, "auth-down /jffs/etc/pptpd/auth-down.sh\n");
#ifdef HAVE_IPV6
		if (nvram_matchi("ipv6_enable", 1)) {
			fprintf(fp, "+ipv6\n");
		}
#endif
		fclose(fp);
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
	} else
#endif
	{
#ifdef HAVE_IPV6
		if (nvram_matchi("ipv6_enable", 1)) {
			fprintf(fp, "+ipv6\n");
		}
#endif
		fclose(fp);
	}

	// Following is all crude and need to be revisited once testing confirms
	// that it does work
	// Should be enough for testing..
	if (nvram_matchi("pptpd_radius", 1)) {
		if (nvram_exists("pptpd_radserver") && nvram_exists("pptpd_radpass")) {
			mkdir("/tmp/pptpd/radius", 0744);

			fp = fopen("/tmp/pptpd/radius/radiusclient.conf", "w");
			fprintf(fp,
				"auth_order radius\n"
				"login_tries 4\n"
				"login_timeout 60\n"
				"radius_timeout 10\n"
				"nologin /etc/nologin\n"
				"servers /tmp/pptpd/radius/servers\n"
				"dictionary /etc/dictionary\n"
				"seqfile /var/run/radius.seq\n"
				"mapfile /etc/port-id-map\n"
				"radius_retries 3\n"
				"authserver %s:%s\n",
				nvram_safe_get("pptpd_radserver"),
				nvram_exists("pptpd_radport") ? nvram_safe_get("pptpd_radport") : "radius");

			if (nvram_exists("pptpd_radserver") && nvram_exists("pptpd_acctport"))
				fprintf(fp, "acctserver %s:%s\n", nvram_safe_get("pptpd_radserver"),
					nvram_exists("pptpd_acctport") ? nvram_safe_get("pptpd_acctport") : "radacct");
			fclose(fp);

			fp = fopen("/tmp/pptpd/radius/servers", "w");
			fprintf(fp, "%s\t%s\n", nvram_safe_get("pptpd_radserver"), nvram_safe_get("pptpd_radpass"));
			fclose(fp);
		}
	}
	// Create pptpd.conf options file for pptpd daemon
	fp = fopen("/tmp/pptpd/pptpd.conf", "w");
	if (nvram_matchi("pptpd_bcrelay", 1))
		fprintf(fp, "bcrelay %s\n", nvram_safe_get("lan_ifname"));
	fprintf(fp,
		"connections %s\nlocalip %s\n"
		"remoteip %s\n",
		nvram_safe_get("pptpd_conn"), nvram_safe_get("pptpd_lip"), nvram_safe_get("pptpd_rip"));
	fclose(fp);

	// Create ip-up and ip-down scripts that are unique to pptpd to avoid
	// interference with pppoe and pptp
	/*
	 * adjust for tunneling overhead (mtu - 40 byte IP - 108 byte tunnel
	 * overhead) 
	 */
	//      if (nvram_matchi("mtu_enable",1))
	//              mss = nvram_geti("wan_mtu") - 40 - 108;
	//      else
	//              mss = 1500 - 40 - 108;
	char bcast[32];

	strcpy(bcast, nvram_safe_get("lan_ipaddr"));
	get_broadcast(bcast, sizeof(bcast), nvram_safe_get("lan_netmask"));

	fp = fopen("/tmp/pptpd/ip-up", "w");
	fprintf(fp,
		"#!/bin/sh\n"
		"startservice set_routes -f\n" // reinitialize
		"echo $PPPD_PID $1 $5 $6 $PEERNAME >> /tmp/pptp_connected\n"
		"iptables -I INPUT -i $1 -j ACCEPT\n"
		"iptables -I FORWARD -i $1 -j ACCEPT\n" //
		//      "iptables -I FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n" "iptables -t nat -I PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s\n"  // rule for wake on lan over pptp tunnel
		"iptables -t nat -I PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s\n" // rule for wake on lan over pptp tunnel
		"%s\n",
		bcast, nvram_safe_get("pptpd_ipdown_script"));
	//      per peer shaping
	if (nvram_matchi("pptpd_radius", 1))
		fprintf(fp,
			"IN=`grep -i RP-Upstream-Speed-Limit /var/run/radattr.$1 | awk '{print $2}'`\n"
			"OUT=`grep -i RP-Downstream-Speed-Limit /var/run/radattr.$1 | awk '{print $2}'`\n"
			"if [ ! -z $IN ] && [ $IN -gt 0 ]\n" //Speed limit !0 and !empty
			"then	tc qdisc del root dev $1\n"
			"\t tc qdisc add dev $1 handle ffff: ingress\n"
			"\t tc filter add dev $1 parent ffff: protocol ip prio 50 u32 match ip src 0.0.0.0/0 police rate \"$IN\"kbit burst \"$IN\"kbit drop flowid :1\n"
			"fi\n"
			"if [ ! -z $OUT ] && [ $OUT -gt 0 ]\n"
			"then tc qdisc del dev $1 ingress\n"
			"\t tc qdisc add dev $1 root tbf rate \"$OUT\"kbit latency 50ms burst \"$OUT\"kbit\n"
			"fi\n");
	fclose(fp);

	fp = fopen("/tmp/pptpd/ip-down", "w");
	fprintf(fp,
		"#!/bin/sh\n"
		"sed -i \"/^$PPPD_PID /d\" /tmp/pptp_connected\n"
		"[ -e /tmp/pptp_peer.db ] || touch /tmp/pptp_peer.db\n"
		"pv() { awk -v pn=\"$1\" '$1 == pn { m=1; printf \"c=%%i; s=%%i; r=%%i; m=1\", $2, $3, $4 } END { if (!m) print \"c=0; s=0; r=0; m=0\" }' /tmp/pptp_peer.db; }\n"
		"eval $(pv $PEERNAME)\n"
		"CONTIME=$(($CONNECT_TIME+$c))\n"
		"SENT=$(($BYTES_SENT/1024+$s))\n"
		"RCVD=$(($BYTES_RCVD/1024+$r))\n"
		"[ $m -eq 1 ] && sed -i \"/^$PEERNAME /d\" /tmp/pptp_peer.db\n"
		"echo \"$PEERNAME $CONTIME $SENT $RCVD\" >> /tmp/pptp_peer.db\n"
		"iptables -D INPUT -i $1 -j ACCEPT\n"
		"iptables -D FORWARD -i $1 -j ACCEPT\n"
		"iptables -t nat -D PREROUTING -i $1 -p udp -m udp --sport 9 -j DNAT --to-destination %s\n" // rule for wake on lan over pptp tunnel
		"%s\n",
		bcast, nvram_safe_get("pptpd_ipdown_script"));
	if (nvram_matchi("pptpd_radius", 1))
		fprintf(fp, "tc qdisc del root dev $1\n"
			    "tc qdisc del ingress dev $1\n");
	fclose(fp);
	chmod("/tmp/pptpd/ip-up", 0744);
	chmod("/tmp/pptpd/ip-down", 0744);

	// Exctract chap-secrets from nvram and add the default account with
	// routers password
	lpTemp = nvram_safe_get("pptpd_auth");
	fp = fopen("/tmp/pptpd/chap-secrets", "w");
	// fprintf (fp, "root\t*\t%s\t*\n", nvram_safe_get ("http_passwd"));
	if (*lpTemp)
		fprintf(fp, "%s\n", lpTemp);
	fclose(fp);

	chmod("/tmp/pptpd/chap-secrets", 0600);

	start_pppmodules();
	// Execute pptpd daemon
	log_eval("pptpd", "-c", "/tmp/pptpd/pptpd.conf", "-o", "/tmp/pptpd/options.pptpd");

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
	dd_loginfo("pptpd", "daemon successfully stopped");
	return;
}
#endif
