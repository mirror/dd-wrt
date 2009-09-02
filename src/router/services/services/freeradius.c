/*
 * freeradius.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <services.h>
#include <radiusdb.h>

#define TYPE_SERVER 0x01
#define TYPE_CA 0x2

/* user database definition */
struct userentry {
	char username[32];
	char password[64];
	unsigned int downstream;
	unsigned int upstream;
	unsigned int enabled;
};

static void gen_cert(char *name, int type)
{
	FILE *fp = fopen(name, "wb");
	fprintf(fp, "[ ca ]\n"
		"default_ca		= CA_default\n"
		"\n"
		"[ CA_default ]\n"
		"dir			= ./\n"
		"certs			= $dir\n"
		"crl_dir			= $dir/crl\n"
		"database		= $dir/index.txt\n"
		"new_certs_dir		= $dir\n");
	if (type == TYPE_CA)
		fprintf(fp, "certificate		= $dir/ca.pem\n");
	else
		fprintf(fp, "certificate		= $dir/server.pem\n");

	fprintf(fp, "serial			= $dir/serial\n"
		"crl			= $dir/crl.pem\n");
	if (type == TYPE_CA)
		fprintf(fp, "private_key		= $dir/ca.key\n");
	else
		fprintf(fp, "private_key		= $dir/server.key\n");

	fprintf(fp, "RANDFILE		= $dir/.rand\n"
		"name_opt		= ca_default\n"
		"cert_opt		= ca_default\n");
	fprintf(fp, "default_days		= %s\n",
		nvram_default_get("radius_days", "365"));
	fprintf(fp,
		"default_crl_days	= 30\n" "default_md		= md5\n"
		"preserve		= no\n"
		"policy			= policy_match\n"
		"\n" "[ policy_match ]\n"
		"countryName		= match\n"
		"stateOrProvinceName	= match\n"
		"organizationName	= match\n"
		"organizationalUnitName	= optional\n"
		"commonName		= supplied\n"
		"emailAddress		= optional\n"
		"\n"
		"[ policy_anything ]\n"
		"countryName		= optional\n"
		"stateOrProvinceName	= optional\n"
		"localityName		= optional\n"
		"organizationName	= optional\n"
		"organizationalUnitName	= optional\n"
		"commonName		= supplied\n"
		"emailAddress		= optional\n"
		"\n" "[ req ]\n" "prompt			= no\n");
	if (type == TYPE_CA)
		fprintf(fp, "distinguished_name	= certificate_authority\n");
	else
		fprintf(fp, "distinguished_name	= server\n");

	fprintf(fp, "default_bits		= 2048\n"
		"input_password		= whatever\n"
		"output_password		= whatever\n");
	if (type == TYPE_CA) {
		fprintf(fp, "x509_extensions		= v3_ca\n");
		fprintf(fp, "\n" "[certificate_authority]\n");
	} else {
		fprintf(fp, "\n" "[server]\n");
	}

	if (!nvram_match("radius_country", ""))
		fprintf(fp, "countryName		= %s\n",
			nvram_get("radius_country"));
	if (!nvram_match("radius_state", ""))
		fprintf(fp, "stateOrProvinceName	= %s\n",
			nvram_get("radius_state"));
	if (!nvram_match("radius_locality", ""))
		fprintf(fp, "localityName		= %s\n",
			nvram_get("radius_locality"));
	if (!nvram_match("radius_organisation", ""))
		fprintf(fp, "organizationName	= %s\n",
			nvram_get("radius_organisation"));
	if (!nvram_match("radius_email", ""))
		fprintf(fp, "emailAddress		= %s\n",
			nvram_get("radius_email"));

	if (type == TYPE_CA)
		fprintf(fp, "commonName		= \"%s\"\n",
			nvram_default_get("radius_common",
					  "DD-WRT FreeRadius Certificate"));
	else
		fprintf(fp, "commonName		= \"%s\"\n",
			nvram_default_get("radius_common",
					  "DD-WRT FreeRadius Certificate"));

	if (type == TYPE_CA)
		fprintf(fp, "\n[v3_ca]\n"
			"subjectKeyIdentifier	= hash\n"
			"authorityKeyIdentifier	= keyid:always,issuer:always\n"
			"basicConstraints	= CA:true\n");

	fclose(fp);

}

void start_gen_radius_cert(void)
{
	FILE *fp = fopen("/jffs/etc/freeradius/radiusd.conf", "rb");
	if (NULL == fp) {
		//prepare files
		system("mkdir -p /jffs/etc/freeradius");
		system("cp -r /etc/freeradius /jffs/etc");
	} else
		fclose(fp);

	gen_cert("/jffs/etc/freeradius/certs/server.cnf", TYPE_SERVER);
	gen_cert("/jffs/etc/freeradius/certs/ca.cnf", TYPE_CA);
	//this takes a long time (depending from the cpu speed)
	system("cd /jffs/etc/freeradius/certs && ./bootstrap");
}

void start_freeradius(void)
{
	int ret = 0;
	pid_t pid;

	char *radiusd_argv[] =
	    { "radiusd", "-d", "/jffs/etc/freeradius", NULL };
	FILE *fp = NULL;

	stop_freeradius();
	nvram_default_get("radius_country", "DE");
	nvram_default_get("radius_state", "Saxon");
	nvram_default_get("radius_locality", "");
	nvram_default_get("radius_organisation", "DD-WRT");
	nvram_default_get("radius_email", "info@dd-wrt.com");
	nvram_default_get("radius_common", "DD-WRT FreeRadius Certificate");

	nvram_default_get("radius_port", "1812");
	nvram_default_get("radius_enabled", "0");
	if (!nvram_match("radius_enabled", "1"))
		return;
	if (!nvram_match("jffs_mounted", "1"))
		return;		//jffs is a requirement for radius and must be mounted at this point here

	fp = fopen("/jffs/etc/freeradius/radiusd.conf", "rb");
	if (NULL == fp) {
		//prepare files
		system("mkdir -p /jffs/etc/freeradius");
		system("cp -r /etc/freeradius /jffs/etc");
	} else
		fclose(fp);

	sysprintf
	    ("sed \"s/port = 0/port = %s/g\" /etc/freeradius/radiusd.conf > /jffs/etc/freeradius/radiusd.conf",
	     nvram_safe_get("radius_port"));

	fp = fopen("/jffs/etc/freeradius/certs/server.pem", "rb");
	if (NULL == fp) {
		//prepare certificates
		start_gen_radius_cert();
	} else
		fclose(fp);
	/* now generate users */
	struct radiusdb *db = loadradiusdb();
	int i;
	fp = fopen("/jffs/etc/freeradius/users", "wb");
	fprintf(fp, "DEFAULT FreeRADIUS-Proxied-To == 127.0.0.1\n"
		"\tSession-Timeout := 3600,\n"
		"\tUser-Name := \"%%{User-Name}\",\n"
		"\tAcct-Interim-Interval := 300,\n" 
		"\tFall-Through = Yes\n\n");

	for (i = 0; i < db->usercount; i++) {
		fprintf(fp, "%s        Cleartext-Password := \"%s\"\n",
			db->users[i].user, db->users[i].passwd);
		if (db->users[i].downstream)
			fprintf(fp, "\tWISPr-Bandwidth-Max-Down := %d",
				db->users[i].downstream * 1024);
		if (db->users[i].upstream) {
			if (db->users[i].downstream)
				fprintf(fp, ",\n");
			fprintf(fp, "\tWISPr-Bandwidth-Max-Up := %d\n",
				db->users[i].upstream * 1024);
		}
		fprintf(fp,"\n");
	}
	freeradiusdb(db);
	ret = _evalpid(radiusd_argv, NULL, 0, &pid);

	dd_syslog(LOG_INFO,
		  "radiusd : FreeRadius daemon successfully started\n");

	return;
}

void stop_freeradius(void)
{

	cprintf("done\n");
	if (pidof("radiusd") > 0) {
		dd_syslog(LOG_INFO,
			  "radiusd : FreeRadius daemon successfully stopped\n");
		killall("radiusd", SIGKILL);
	}
	return;
}
#endif
