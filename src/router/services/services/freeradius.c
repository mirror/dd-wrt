/*
 * freeradius.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#ifdef HAVE_FREERADIUS

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <services.h>
#include <radiusdb.h>
#include <time.h>

static void prep(void)
{
	if (!f_exists("/jffs/etc/freeradius/radiusd.conf")) {
		//prepare files
		mkdir("/jffs/etc", 0700);
		mkdir("/jffs/etc/freeradius", 0700);
		system("cp -R /etc/freeradius /jffs/etc");
	}
}

void stop_gen_radius_cert(void)
{
}

void start_gen_radius_cert(void)
{
	if (nvram_matchi("cert_running", 1) && pidof("openssl") > 0)
		return; //already running
	prep();
	gen_cert("/jffs/etc/freeradius/certs/server.cnf", TYPE_SERVER,
		 nvram_safe_get("radius_common"),
		 nvram_safe_get("radius_passphrase"));
	gen_cert("/jffs/etc/freeradius/certs/ca.cnf", TYPE_CA,
		 nvram_safe_get("radius_common"),
		 nvram_safe_get("radius_passphrase"));
	nvram_seti("cert_running", 1);
	//this takes a long time (depending from the cpu speed)
	system("cd /jffs/etc/freeradius/certs && ./bootstrap");
	sysprintf(
		"sed \"s/private_key_password = whatever/private_key_password = %s/g\" /etc/freeradius/mods-available/eap > /jffs/etc/freeradius/mods-available/eap",
		nvram_safe_get("radius_passphrase"));
	nvram_seti("cert_running", 0);
}

void start_freeradius(void)
{
	int ret = 0;

	char *radiusd_argv[] = { "radiusd", "-d", "/jffs/etc/freeradius",
				 NULL };
	FILE *fp = NULL;
	nvram_default_get("radius_country", "DE");
	nvram_default_get("radius_state", "Saxony");
	nvram_default_get("radius_locality", "none");
	nvram_default_get("radius_expiration", "365");
	nvram_default_get("radius_passphrase", "changeme");
	nvram_default_get("radius_organisation", "DD-WRT");
	nvram_default_get("radius_email", "info@dd-wrt.com");
	nvram_default_get("radius_common", "DD-WRT FreeRadius Certificate");

	nvram_default_get("radius_port", "1812");
	nvram_default_get("radius_enabled", "0");
	if (!nvram_matchi("radius_enabled", 1)) {
		stop_freeradius();
		return;
	}

	if (!jffs_mounted() && (freediskSpace("/jffs") < 8 * 1024 * 1024))
		return; //jffs is a requirement for radius and must be mounted at this point here

	prep();
	sysprintf(
		"sed \"s/port = 0/port = %s/g\" /etc/freeradius/sites-available/default > /jffs/etc/freeradius/sites-available/default",
		nvram_safe_get("radius_port"));
	sysprintf(
		"sed \"s/private_key_password = whatever/private_key_password = %s/g\" /etc/freeradius/mods-available/eap > /jffs/etc/freeradius/mods-available/eap",
		nvram_safe_get("radius_passphrase"));

	if (!f_exists("/jffs/etc/freeradius/certs/server.pem")) {
		//prepare certificates
		start_gen_radius_cert();
	}

	int i;

	/* generate clients */
	{
		struct radiusclientdb *db = loadradiusclientdb();
		if (db) {
			fp = fopen("/jffs/etc/freeradius/clients.conf", "wb");
			if (!fp)
				return;
			system("touch /jffs/etc/freeradius/clients.manual");
			fprintf(fp, "$INCLUDE clients.manual\n");

			for (i = 0; i < db->usercount; i++) {
				if (!db->users[i].clientsize)
					continue;
				if (!db->users[i].client ||
				    !*db->users[i].client)
					continue;
				fprintf(fp,
					"client %s {\n"
					"\tipaddr = %s\n"
					"\tsecret = %s\n"
					"\tshortname = DD-WRT-RADIUS\n}\n",
					db->users[i].client,
					db->users[i].client,
					db->users[i].passwd);
			}

			fclose(fp);
			freeradiusclientdb(db);
		}
	}

	/* now generate users */
	{
		struct radiusdb *db = loadradiusdb();
		if (db) {
			fp = fopen(
				"/jffs/etc/freeradius/mods-config/files/authorize",
				"wb");
			if (!fp)
				return;
			system("touch /jffs/etc/freeradius/users.manual");
			system("touch /jffs/etc/freeradius/mods-config/files/users.manual");
			fprintf(fp, "$INCLUDE users.manual\n");
			fprintf(fp,
				"DEFAULT FreeRADIUS-Proxied-To == 127.0.0.1\n"
				"\tSession-Timeout := 3600,\n"
				"\tUser-Name := \"%%{User-Name}\",\n"
				"\tAcct-Interim-Interval := 300,\n"
				"\tFall-Through = Yes\n\n");
			time_t tm;
			struct tm tm_time;
			for (i = 0; i < db->usercount; i++) {
				if (!db->users[i].usersize)
					continue;
				if (!db->users[i].user || !*db->users[i].user)
					continue;
				if (!db->users[i].enabled)
					continue;
				fprintf(fp,
					"%s        Cleartext-Password := \"%s\"",
					db->users[i].user, db->users[i].passwd);
				if (db->users[i].expiration) {
					tm = db->users[i].expiration * 24 * 60 *
					     60;
					memcpy(&tm_time, localtime(&tm),
					       sizeof(tm_time));
					char datebuf[128];
					strftime(datebuf, sizeof(datebuf),
						 "%d %b %Y", &tm_time);
					fprintf(fp, ", Expiration == \"%s\"\n",
						datebuf);
				} else
					fprintf(fp, "\n");
				if (db->users[i].downstream) {
					fprintf(fp,
						"\tWISPr-Bandwidth-Max-Down := %d,\n",
						db->users[i].downstream * 1024);
					fprintf(fp,
						"\tRP-Downstream-Speed-Limit := %d",
						db->users[i].downstream);
				}
				if (db->users[i].upstream) {
					if (db->users[i].downstream)
						fprintf(fp, ",\n");
					fprintf(fp,
						"\tWISPr-Bandwidth-Max-Up := %d,\n",
						db->users[i].upstream * 1024);
					fprintf(fp,
						"\tRP-Upstream-Speed-Limit := %d",
						db->users[i].upstream);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
			freeradiusdb(db);
		}
	}
	if (reload_process("radiusd")) {
		_log_evalpid(radiusd_argv, NULL, 0, NULL);
	}

	return;
}

void restart_freeradius(void)
{
	start_freeradius();
}

void stop_freeradius(void)
{
	cprintf("done\n");
	stop_process("radiusd", "daemon");
	return;
}
#endif
