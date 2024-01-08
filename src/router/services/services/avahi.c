/*
 * avahi.c
 *
 * Copyright (C) 2022 EGC
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

#ifdef HAVE_MDNS
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

void start_mdns(void)
{
	char path[64];
	char conffile[64];
	if (!nvram_matchi("mdns_enable", 1)) {
		stop_mdns();
		return;
	}
	//char avahi-conffile[64]="/tmp/avahi/avahi-daemon.conf";
	mkdir("/tmp/avahi", 0744);
	mkdir("/tmp/avahi/services", 0744);
	mkdir("/tmp/var/run/avahi-daemon", 0744);

	FILE *fp;
	fp = fopen("/tmp/mdns.conf", "wb");
	fprintf(fp,
		"[server]\n" //
		"host-name=%s\n" //
		"domain-name=%s\n" //
		"use-ipv4=yes\n" //
		"use-ipv6=%s\n" //
		"check-response-ttl=no\n" //
		"use-iff-running=no\n" //
		"#deny-interfaces=\n",
		nvram_safe_get("router_name"), nvram_safe_get("mdns_domain"), nvram_matchi("ipv6_enable", 1) ? "yes" : "no");
	char ifname[32];
	char *next;
	char *wordlist = nvram_safe_get("mdns_interfaces");
	int idx = 0;
	foreach(ifname, wordlist, next)
	{
		if (!idx)
			fprintf(fp, "allow-interfaces=");
		else
			fprintf(fp, ",");
		fprintf(fp, "%s", ifname);
		idx++;
	}
	if (idx)
		fprintf(fp, "\n");

	fprintf(fp,
		"\n"
		"[wide-area]\n" //
		"enable-wide-area=yes\n" //
		"\n" //
		"[publish]\n" //
		"publish-addresses=yes\n" //
		"publish-hinfo=yes\n" //
		"publish-workstation=yes\n" //
		"publish-domain=yes\n" //
		"#publish-dns-servers=%s\n" //
		"#publish-resolv-conf-dns-servers=yes\n" //
		"\n"
		"[reflector]\n" //
		"enable-reflector=%s\n" //
		"#reflect-ipv=no\n" //
		"\n"
		"[rlimits]\n" //
		"#rlimit-as=\n" //
		"rlimit-core=0\n" //
		"rlimit-data=4194304\n" //
		"rlimit-fsize=0\n" //
		"rlimit-nofile=30\n" //
		"rlimit-stack=4194304\n" //
		"rlimit-nproc=3\n",
		nvram_safe_get("lan_ipaddr"), nvram_matchi("mdns_reflector", 1) ? "yes" : "no");
	fclose(fp);

#ifdef HAVE_SMBD //might need SAMBA
	// add smb service to avahi, better place in samba3.c maybe enumerated for all partitions
	if (nvram_matchi("samba3_enable", 1)) {
		fp = fopen("/tmp/avahi/services/smb.service", "wb");
		fprintf(fp,
			"<?xml version=\"1.0\" standalone=\'no\'?>\n" //
			"<!DOCTYPE service-group SYSTEM \"avahi-service.dtd\">\n" //
			"<service-group>\n" //
			"\t<name replace-wildcards=\"yes\">SAMBA on %%h</name>\n" //
			"\t<service>\n" //
			"\t\t<type>_smb._tcp</type>\n" //
			"\t\t<port>445</port>\n" //
			"\t</service>\n" //
			"</service-group>\n");
		fclose(fp);
	}
#endif

#ifdef HAVE_MDNS_UTILS
	mkdir("/tmp/var/run/dbus", 0744);
	fp = fopen("/tmp/avahi-dbus.conf", "wb");
	fprintf(fp,
		"<!DOCTYPE busconfig PUBLIC\n" //
		"\"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN\"\n" //
		"\"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n" //
		"<busconfig>\n" //
		"\n" //
		"<!-- Only root or user nobody can own the Avahi service this needs some work -->\n" //
		"<policy user=\"nobody\">\n" //
		"<allow own=\"*\"/>\n" //
		"</policy>\n" //
		"<policy user=\"root\">\n" //
		"<allow own=\"*\"/>\n" //
		"</policy>\n" //
		"\n" //
		"<!-- Allow anyone to invoke methods on Avahi server, except SetHostName -->\n" //
		"<policy context=\"default\">\n" //
		"<allow send_destination=\"*\"/>\n" //
		"<allow receive_sender=\"*\"/>\n" //
		"\n" //
		"<deny send_destination=\"org.freedesktop.Avahi\"\n" //
		"send_interface=\"org.freedesktop.Avahi.Server\" send_member=\"SetHostName\"/>\n" //
		"</policy>\n" //
		"\n" //
		"<listen>unix:path=/tmp/var/run/dbus/system_bus_socket</listen>\n" //
		"\n" //
		"<!-- Allow everything, including access to SetHostName to users of the group \"nobody\" -->\n" //
		"<policy group=\"nobody\">\n" //
		"<allow send_destination=\"*\"/>\n" //
		"<allow receive_sender=\"*\"/>\n" //
		"</policy>\n" //
		"<policy user=\"root\">\n" //
		"<allow send_destination=\"*\"/>\n" //
		"<allow receive_sender=\"*\"/>\n" //
		"</policy>\n" //
		"</busconfig>\n");
	fclose(fp);

	if (pidof("dbus-daemon") > 0) {
		dd_loginfo("dbus-daemon", "dbus-daemon already running\n");
	} else {
		snprintf(conffile, sizeof(conffile), "--config-file=%s",
			 getdefaultconfig("mdns", path, sizeof(path), "avahi-dbus.conf"));
		log_eval("dbus-launch", conffile);
	}
#endif
	if (reload_process("avahi-daemon")) {
		snprintf(conffile, sizeof(conffile), getdefaultconfig("mdns", path, sizeof(path), "mdns.conf"));
		log_eval("avahi-daemon", "-D", "-f", conffile, "--no-drop-root");
	}
	return;
}

void restart_mdns(void)
{
	start_mdns();
}

void stop_mdns(void)
{
	//eval("/usr/sbin/avahi-daemon", "--kill");
	stop_process("avahi-daemon", "avahi-daemon");
	unlink("/tmp/avahi/services/smb.service");
#ifdef HAVE_MDNS_UTILS
	stop_process("dbus-daemon", "dbus-daemon");
#endif
	return;
}
#endif
