/*
 * vncrepeater.c
 *
 * Copyright (C) 2009 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_VNCREPEATER
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

char *vncrepeater_deps(void)
{
	return "vncr_enable";
}

char *vncrepeater_proc(void)
{
	return "repeater";
}

void stop_vncrepeater(void);
void start_vncrepeater(void)
{
	char wan_if_buffer[33];
	stop_vncrepeater();
	if (!nvram_matchi("vncr_enable", 1))
		return;
	eval("iptables", "-I", "INPUT", "-p", "tcp", "-i", safe_get_wan_face(wan_if_buffer), "--dport", "5900", "-j", "ACCEPT");
	eval("iptables", "-I", "INPUT", "-p", "tcp", "-i", safe_get_wan_face(wan_if_buffer), "--dport", "5500", "-j", "ACCEPT");

	FILE *fp = fopen("/tmp/vncrepeater.ini", "wb");
	fprintf(fp, "[general]\n");
	fprintf(fp, "viewerport=5900\n");
	fprintf(fp, "serverport=5500\n");
	fprintf(fp, "ownipaddress=0.0.0.0\n");
	fprintf(fp, "maxsessions=100\n");
	fprintf(fp, "runasuser=root\n");
	fprintf(fp, "allowedmodes=3\n");
	fprintf(fp, "logginglevel=1\n");
	fprintf(fp, "[mode1]\n");
	fprintf(fp, "allowedmode1serverport=0\n");
	fprintf(fp, "requirelistedserver=0\n");
	fprintf(fp, "srvListAllow0=0.0.0.0\n");
	fprintf(fp, "[mode2]\n");
	fprintf(fp, "requirelistedid=0\n");
	fprintf(fp, "[eventinterface]\n");
	fprintf(fp, "useeventinterface=false\n");
	fclose(fp);

	system("repeater /tmp/vncrepeater.ini&");
	dd_loginfo("vncrepeater", "daemon successfully started");

	return;
}

void stop_vncrepeater(void)
{
	char wan_if_buffer[33];
	eval("iptables", "-D", "INPUT", "-p", "tcp", "-i", safe_get_wan_face(wan_if_buffer), "--dport", "5900", "-j", "ACCEPT");
	eval("iptables", "-D", "INPUT", "-p", "tcp", "-i", safe_get_wan_face(wan_if_buffer), "--dport", "5500", "-j", "ACCEPT");
	stop_process("vncrepeater", "daemon");
	nvram_delstates(vncrepeater_deps());
}
#endif
