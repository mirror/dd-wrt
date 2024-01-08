/*
 * pppoeserver.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *					  all bugs added by 2011-2012 Sash	
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
#ifdef HAVE_PPPOESERVER
#include <stdio.h>
#include <signal.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <malloc.h>
#include <sys/stat.h>
#include <syslog.h>
#include <services.h>

static char *getifip(void)
{
	if (nvram_match("pppoeserver_interface", "br0"))
		return nvram_safe_get("lan_ipaddr");
	else
		return nvram_nget("%s_ipaddr",
				  nvram_safe_get("pppoeserver_interface"));
}

static void add_pppoe_natrule(void)
{
}

static void del_pppoe_natrule(void)
{
}

void addpppoeconnected_main(int argc, char *argv[])
{
	static int lock = 0;

	while (lock) {
		usleep(100);
	}

	lock = 1;
	sysprintf("echo \"%s\t%s\t%s\t%s\" >> /tmp/pppoe_connected", argv[1],
		  argv[2], argv[3], argv[4]);
	lock = 0;
}

void addpppoetime_main(int argc, char *argv[])
{
	static int lock = 0;

	while (lock) {
		usleep(100);
	}

	lock = 1;
	sysprintf("grep -v %s /tmp/pppoe_peer.db > /tmp/pppoe_peer.db.tmp",
		  argv[1]);
	eval("mv", "/tmp/pppoe_peer.db.tmp", "/tmp/pppoe_peer.db");
	sysprintf("echo \"%s\t\t%s\t\t%s\t\t%s\" >> /tmp/pppoe_peer.db",
		  argv[2], argv[3], argv[4], argv[1]);
	lock = 0;
}

void delpppoeconnected_main(int argc, char *argv[])
{
	static int lock = 0;

	while (lock) {
		usleep(100);
	}

	lock = 1;
	sysprintf("grep -v %s /tmp/pppoe_connected > /tmp/pppoe_connected.tmp",
		  argv[1]);
	eval("mv", "/tmp/pppoe_connected.tmp", "/tmp/pppoe_connected", "-f");
	//      just an uptime test
	sysprintf("grep -v %s /tmp/pppoe_uptime > /tmp/pppoe_uptime.tmp",
		  argv[2]);
	eval("mv", "/tmp/pppoe_uptime.tmp", "/tmp/pppoe_uptime", "-f");
	lock = 0;
}

static void makeipup(void)
{
	FILE *fp = fopen("/tmp/pppoeserver/ip-up.sh", "w");

	fprintf(fp, "#!/bin/sh\n");
	if (nvram_match("filter", "on")) // only needed if firewall is enabled
		fprintf(fp, "iptables -I INPUT -i $1 -j ACCEPT\n");
	//      if (nvram_match("pppoeserver_clip", "local")) //for radius ip's...to be worked on
	if (nvram_match("wan_proto",
			"pppoe") //only when there is an ppp0 interface
	    || nvram_match("wan_proto", "pptp") ||
	    nvram_match("wan_proto", "pppoe_dual"))
		fprintf(fp, "iptables -I FORWARD -i $1 -j ACCEPT\n"
			    "iptables -I FORWARD -o $1 -j ACCEPT\n");
	fprintf(fp, "addpppoeconnected $PPPD_PID $1 $5 $PEERNAME\n"
		//"echo \"$PPPD_PID\t$1\t$5\t`date +%%s`\t0\t$PEERNAME\" >> /tmp/pppoe_connected\n"
		//      just an uptime test
		//"echo \"`date +%%s`\t$PEERNAME\" >> /tmp/pppoe_uptime\n"      //
		//->use something like $(( ($(date +%s) - $(date -d "$dates" +%s)) / (60*60*24*31) )) for computing uptime in the gui
	);
	//      per peer shaping
	if (nvram_matchi("pppoeradius_enabled", 1)) {
		fprintf(fp,
			"IN=`grep -i RP-Upstream-Speed-Limit /var/run/radattr.$1 | awk '{print $2}'`\n"
			"OUT=`grep -i RP-Downstream-Speed-Limit /var/run/radattr.$1 | awk '{print $2}'`\n"
			"if [ ! -z $IN ] && [ $IN -gt 0 ]\n"
			"then tc qdisc del dev $1 ingress\n"
			"\t tc qdisc add dev $1 handle ffff: ingress\n"
			"\t tc filter add dev $1 parent ffff: protocol ip prio 50 u32 match ip src 0.0.0.0/0 police rate \"$IN\"kbit burst \"$IN\"kbit drop flowid :1\n"
			"fi\n"
			"if [ ! -z $OUT ] && [ $OUT -gt 0 ]\n" //only if Speed limit !0 and !empty
			"then	tc qdisc del root dev $1\n"
			"\t tc qdisc add dev $1 root tbf rate \"$OUT\"kbit latency 50ms burst \"$OUT\"kbit\n"
			"fi\n");
	}
	//tc qdisc add dev $1 root red min 150KB max 450KB limit 600KB burst 200 avpkt 1000 probability 0.02 bandwidth 100Mbit
	//eg: tc qdisc add dev $1 root red min 150KB max 450KB limit 600KB burst 200 avpkt 1000 probability 0.02 bandwidth 10Mbit
	//burst = (min+min+max)/(3*avpkt); limit = minimum: max+burst or x*max, max = 2*min
	fclose(fp);
	fp = fopen("/tmp/pppoeserver/ip-down.sh", "w");
	fprintf(fp,
		"#!/bin/sh\n"
		"delpppoeconnected $PPPD_PID $PEERNAME\n"
		//      calc connected time and volume per peer
		"CONTIME=`grep $PEERNAME /tmp/pppoe_peer.db | awk '{print $1}'`\n"
		"SENT=`grep $PEERNAME /tmp/pppoe_peer.db | awk '{print $2}'`\n"
		"RCVD=`grep $PEERNAME /tmp/pppoe_peer.db | awk '{print $3}'`\n"
		"CONTIME=$(($CONTIME+$CONNECT_TIME))\n"
		"SENT=$(($SENT+$BYTES_SENT))\n"
		"RCVD=$(($RCVD+$BYTES_RCVD))\n"
		"addpppoetime $PEERNAME $CONTIME $SENT $RCVD\n");
	if (nvram_match("wan_proto", "pppoe") ||
	    nvram_match("wan_proto", "pptp") ||
	    nvram_match("wan_proto", "pppoe_dual"))
		fprintf(fp, "iptables -D FORWARD -i $1 -j ACCEPT\n"
			    "iptables -D FORWARD -o $1 -j ACCEPT\n");
	if (nvram_match("filter", "on")) // only needed if firewall is enabled
		fprintf(fp, "iptables -D INPUT -i $1 -j ACCEPT\n");
	if (nvram_matchi("pppoeradius_enabled", 1))
		fprintf(fp, "tc qdisc del root dev $1\n"
			    "tc qdisc del dev $1 ingress\n");
	fclose(fp);

	chmod("/tmp/pppoeserver/ip-up.sh", 0700);
	chmod("/tmp/pppoeserver/ip-down.sh", 0700);

	//      copy existing peer data to /tmp
	if ((nvram_matchi("usb_enable", 1) && nvram_matchi("usb_storage", 1) &&
	     nvram_matchi("usb_automnt", 1) &&
	     nvram_match("usb_mntpoint", "jffs")) ||
	    jffs_mounted())
		mkdir("/jffs/etc", 0700);
	mkdir("/jffs/etc/pppoeserver", 0700);
	eval("/bin/cp", "/jffs/etc/pppoeserver/pppoe_peer.db", "/tmp/");
}

static void do_pppoeconfig(FILE *fp)
{
	int nowins = 0;
	int i;

	if (nvram_default_match("wan_wins", "0.0.0.0", "0.0.0.0")) {
		nowins = 1;
	}
	// fprintf (fp, "crtscts\n");
	if (nvram_default_matchi("pppoeserver_bsdcomp", 0, 0))
		fprintf(fp, "nobsdcomp\n");
	else
		fprintf(fp, "bsdcomp 12\n");
	if (nvram_default_matchi("pppoeserver_deflate", 0, 0))
		fprintf(fp, "nodeflate\n");
	else
		fprintf(fp, "deflate 12\n");
	if (nvram_default_matchi("pppoeserver_lzs", 0, 0))
		fprintf(fp, "nolzs\n");
	else
		fprintf(fp, "lzs\n");
	if (nvram_default_matchi("pppoeserver_mppc", 0, 0))
		fprintf(fp, "nomppc\n");
	else
		fprintf(fp, "mppc\n");
	if (nvram_default_matchi("pppoeserver_encryption", 1, 0))
		fprintf(fp, "require-mppe-128\n");
	else
		fprintf(fp, "nomppe\n");
	fprintf(fp,
		"auth\n" //
		//              "endpoint <epdisc>\n" needed 4 ml
		//              "multilink\n"
		"refuse-eap\n" // be sure using best auth methode
		"refuse-pap\n" //
		"refuse-chap\n" //erlauben???
		"refuse-mschap\n" //
		"require-mschap-v2\n" //
		"nopcomp\n" // no protocol field compression
		"default-mru\n" //
		"mtu %s\n"
		"mru %s\n"
		"default-asyncmap\n"
		"noipdefault\n"
		"defaultroute\n"
		"netmask 255.255.255.255\n" //
		"ip-up-script /tmp/pppoeserver/ip-up.sh\n" //
		"ip-down-script /tmp/pppoeserver/ip-down.sh\n" //
		"lcp-echo-adaptive\n" //
		"lcp-echo-interval %s\n" //
		"lcp-echo-failure %s\n" //
		"lcp-echo-adaptive\n" //
		"idle %s\n",
		nvram_safe_get("pppoeserver_mtu"),
		nvram_safe_get("pppoeserver_mru"),
		nvram_safe_get("pppoeserver_lcpechoint"),
		nvram_safe_get("pppoeserver_lcpechofail"),
		nvram_safe_get("pppoeserver_idle"));
	if (nvram_match("pppoeserver_interface", "br0"))
		fprintf(fp, "proxyarp\n"
			    "ktune\n");
	else
		fprintf(fp, "noktune\n");
	if (!nowins) {
		fprintf(fp, "ms-wins %s\n", nvram_safe_get("wan_wins"));
	}
	struct dns_lists *dns_list = get_dns_list(0);

	if (dns_list) {
		for (i = 0; i < dns_list->num_servers; i++)
			fprintf(fp, "ms-dns %s\n", dns_list->dns_server[i].ip);
		free_dns_list(dns_list);
	}
}

void start_pppoeserver(void)
{
	/*	//	calculate uptime for the GUI
	//proc/net/dev differenz = speed. interface aus datei.
	fp = fopen("/tmp/pppoeserver/calc-uptime.sh", "w");
	fprintf(fp, "#!/bin/sh\n"
		"pppoe_connected=/tmp/pppoe_connected\n"
	//for i in `cat pppoe_connected |awk '{print $4}'` ; do sed '%i,s/`awk '{print $4}'`/neu/g' pppoe_connected > pppoe_connected.tmp ; done
		"for i in `cat pppoe_connected |awk '{print $4}'` ; do\n"
		"\tCONTIME=`grep $PEERNAME /tmp/pppoe_peer.db | awk '{print $1}'`\n"
		"SENT=`grep $PEERNAME /tmp/pppoe_peer.db | awk '{print $2}'`\n"
		"done\n"
		)			
	fclose(fp);
	chmod("/tmp/pppoeserver/calc-uptime.sh", 0744);*/

	FILE *fp;
	if (nvram_default_matchi("pppoeserver_enabled", 1, 0)) {
		add_pppoe_natrule();
		if (nvram_default_matchi("pppoeradius_enabled", 0, 0)) {
			mkdir("/tmp/pppoeserver", 0777);
			fp = fopen("/tmp/pppoeserver/pppoe-server-options",
				   "wb");
			do_pppoeconfig(fp);
			fprintf(fp,
				"chap-secrets /tmp/pppoeserver/chap-secrets\n");
			fclose(fp);

			// parse chaps from nvram to file
			char word[256];
			char *next, *wordlist;

			wordlist = nvram_safe_get("pppoeserver_chaps");

			fp = fopen("/tmp/pppoeserver/chap-secrets", "wb");

			foreach(word, wordlist, next)
			{
				GETENTRYBYIDX(user, word, 0);
				GETENTRYBYIDX(pass, word, 1);
				GETENTRYBYIDX(ip, word, 2);
				GETENTRYBYIDX(enable, word, 3);
				if (!user || !pass || !ip || !enable)
					continue;

				if (!strcmp(ip, "0.0.0.0"))
					ip = "*";
				if (!strcmp(enable, "on"))
					fprintf(fp, "%s * %s %s\n", user, pass,
						ip);
			}
			fclose(fp);
			makeipup();
			// end parsing
		} else {
			mkdir("/tmp/pppoeserver", 0777);
			fp = fopen("/tmp/pppoeserver/pppoe-server-options",
				   "wb");
			do_pppoeconfig(fp);
			fprintf(fp,
				"login\n" //
				"plugin radius.so\n" //
				"plugin radattr.so\n" //
				"radius-config-file /tmp/pppoeserver/radius/radiusclient.conf\n");
			fclose(fp);
			mkdir("/tmp/pppoeserver/radius", 0777);
			fp = fopen("/tmp/pppoeserver/radius/radiusclient.conf",
				   "wb");
			fprintf(fp,
				"auth_order\tradius\n" //
				"login_tries\t4\n" //
				"login_timeout\t60\n" //
				"nologin\t/etc/nologin\n" //
				"issue\t/etc/issue\n" //
				"servers\t/tmp/pppoeserver/radius/servers\n" //
				"dictionary\t/etc/dictionary\n" //
				"login_radius\t/usr/local/sbin/login.radius\n" //
				"seqfile\t/var/run/radius.seq\n" //
				"mapfile\t/etc/port-id-map\n"
				"default_realm\n" //
				"radius_timeout\t10\n" //
				"radius_retries\t3\n" //
				"login_local\t/bin/login\n"); //
			if (nvram_match("pppoeserver_authserverip_backup",
					"0.0.0.0") ||
			    *(nvram_safe_get(
				    "pppoeserver_authserverip_backup")) == 0) {
				fprintf(fp,
					"authserver %s:%s\n" //
					"acctserver %s:%s\n", //
					nvram_safe_get(
						"pppoeserver_authserverip"),
					nvram_safe_get(
						"pppoeserver_authserverport"),
					nvram_safe_get(
						"pppoeserver_authserverip"),
					nvram_safe_get(
						"pppoeserver_acctserverport"));
			} else {
				fprintf(fp,
					"authserver %s:%s, %s:%s\n" //
					"acctserver %s:%s, %s:%s\n", //
					nvram_safe_get(
						"pppoeserver_authserverip"),
					nvram_safe_get(
						"pppoeserver_authserverport"),
					nvram_safe_get(
						"pppoeserver_authserverip_backup"),
					nvram_safe_get(
						"pppoeserver_authserverport_backup"),
					nvram_safe_get(
						"pppoeserver_authserverip"),
					nvram_safe_get(
						"pppoeserver_acctserverport"),
					nvram_safe_get(
						"pppoeserver_authserverip_backup"),
					nvram_safe_get(
						"pppoeserver_acctserverport_backup"));
			}
			fclose(fp);
			fp = fopen("/tmp/pppoeserver/radius/servers", "wb");
			fprintf(fp, "%s %s\n",
				nvram_safe_get("pppoeserver_authserverip"),
				nvram_safe_get(
					"pppoeserver_sharedkey")); // todo,
			if (nvram_invmatch("pppoeserver_authserverip_backup",
					   "0.0.0.0") ||
			    *(nvram_safe_get(
				    "pppoeserver_authserverip_backup")) != 0)
				fprintf(fp, "%s %s\n",
					nvram_safe_get(
						"pppoeserver_authserverip_backup"),
					nvram_safe_get(
						"pppoeserver_sharedkey_backup"));
			fclose(fp);
			makeipup();
		}

		//              // clamp when fw clamping is off
		//              if (nvram_match("wan_proto", "disabled"))
		//                      system("/usr/sbin/iptables -I FORWARD 1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");

		if (nvram_invmatch("wan_proto",
				   "pppoe") //if there is no ppp0, reduce rules
		    && nvram_invmatch("wan_proto", "pptp") &&
		    nvram_invmatch("wan_proto", "pppoe_dual")) {
			eval("/usr/sbin/iptables", "-I", "FORWARD", "-i",
			     "ppp+", "-j", "ACCEPT");
			eval("/usr/sbin/iptables", "-I", "FORWARD", "-o",
			     "ppp+", "-j", "ACCEPT");
		}

		start_pppmodules();
		log_eval("pppoe-server", "-k", "-I",
			 nvram_safe_get("pppoeserver_interface"), "-L",
			 getifip(), "-i", "-x",
			 nvram_safe_get("pppoeserver_sessionlimit"), "-N",
			 nvram_safe_get("pppoeserver_clcount"), "-R",
			 nvram_safe_get("pppoeserver_pool"), "-X",
			 "/var/run/pppoeserver.pid");
	}
}

void stop_pppoeserver(void)
{
	if (stop_process("pppoe-server", "daemon")) {
		del_pppoe_natrule();
		unlink("/tmp/pppoe_connected");
		unlink("/tmp/pppoeserver/radius/radiusclient.conf");
		unlink("/tmp/pppoeserver/radius/servers");
		unlink("/tmp/pppoeserver/ip-up.sh");
		unlink("/tmp/pppoeserver/ip-down.sh");
		unlink("/tmp/pppoeserver/pppoe-server-options");
		//      unlink("/tmp/pppoeserver/calc-uptime.sh");

		//      backup peer data to jffs/usb if available
		if ((nvram_matchi("usb_enable", 1) &&
		     nvram_matchi("usb_storage", 1) &&
		     nvram_matchi("usb_automnt", 1) &&
		     nvram_match("usb_mntpoint", "jffs")) ||
		    jffs_mounted()) {
			mkdir("/jffs/etc", 0700);
			mkdir("/jffs/etc/pppoeserver", 0700);
			eval("/bin/cp", "/tmp/pppoe_peer.db",
			     "/jffs/etc/pppoeserver");
		}
		//              if (nvram_match("wan_proto", "disabled"))
		//                      system("/usr/sbin/iptables -D FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");

		//system("/usr/sbin/ebtables -D INPUT -i `nvram get pppoeserver_interface` -p 0x8863 -j DROP");
		if (nvram_invmatch("wan_proto", "pppoe") &&
		    nvram_invmatch("wan_proto", "pptp") &&
		    nvram_invmatch("wan_proto", "pppoe_dual")) {
			eval("/usr/sbin/iptables", "-D", "FORWARD", "-i",
			     "ppp+", "-j", "ACCEPT");
			eval("/usr/sbin/iptables", "-D", "FORWARD", "-o",
			     "ppp+", "-j", "ACCEPT");
		}
	}
}

#endif
