/*
 * radius.c
 *
 * Copyright (C) 2009 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>

#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <cy_conf.h>
#include <bcmdevs.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
#include <broadcom.h>
#include <radiusdb.h>

static unsigned int readword(FILE *in)
{
	unsigned int value;
	value = (unsigned int)(getc(in) & 0xff) << 24;
	value |= (unsigned int)(getc(in) & 0xff) << 16;
	value |= (unsigned int)(getc(in) & 0xff) << 8;
	value |= (unsigned int)(getc(in) & 0xff);
	return value;
}

static void writeword(unsigned int value, FILE *out)
{
	putc((value >> 24) & 0xff, out);
	putc((value >> 16) & 0xff, out);
	putc((value >> 8) & 0xff, out);
	putc(value & 0xff, out);
}

struct radiusdb *loadradiusdb(void)
{
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtusers.db", "rb");
	if (fp == NULL)
		return NULL;
	struct radiusdb *db;
	if (feof(fp)) {
		fclose(fp);
		return NULL;
	}
	db = safe_malloc(sizeof(struct radiusdb));
	db->usercount = readword(fp);
	if (db->usercount)
		db->users = safe_malloc(db->usercount * sizeof(struct radiususer));
	else
		db->users = NULL;
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		int curlen = 0;
		db->users[i].fieldlen = readword(fp);
		db->users[i].usersize = readword(fp);
		curlen += 8;
		if (db->users[i].usersize) {
			db->users[i].user = safe_malloc(db->users[i].usersize);
			fread(db->users[i].user, db->users[i].usersize, 1, fp);
			curlen += db->users[i].usersize;
		} else
			db->users[i].user = NULL;

		db->users[i].passwordsize = readword(fp);
		curlen += 4;
		if (db->users[i].passwordsize) {
			db->users[i].passwd = safe_malloc(db->users[i].passwordsize);
			fread(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
			curlen += db->users[i].passwordsize;
		} else
			db->users[i].passwd = NULL;

		db->users[i].downstream = readword(fp);
		db->users[i].upstream = readword(fp);
		curlen += 8;
		if (curlen < db->users[i].fieldlen) {
			db->users[i].expiration = readword(fp);
			curlen += 4;
		} else
			db->users[i].expiration = 0;

		if (curlen < db->users[i].fieldlen) {
			db->users[i].enabled = readword(fp);
			curlen += 4;
		} else
			db->users[i].enabled = 1;

		if ((db->users[i].fieldlen - curlen) > 0) //for backward compatiblity
			fseek(fp, db->users[i].fieldlen - curlen, SEEK_CUR);
	}
	fclose(fp);
	return db;
}

void writeradiusdb(struct radiusdb *db)
{
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtusers.db", "wb");
	if (fp == NULL)
		return;
	writeword(db->usercount, fp);
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		if (db->users[i].user)
			db->users[i].usersize = strlen(db->users[i].user) + 1;
		else
			db->users[i].usersize = 0;

		if (db->users[i].passwd)
			db->users[i].passwordsize = strlen(db->users[i].passwd) + 1;
		else
			db->users[i].passwordsize = 0;

		db->users[i].fieldlen =
			sizeof(struct radiususer) + db->users[i].usersize + db->users[i].passwordsize - (sizeof(char *) * 2);

		writeword(db->users[i].fieldlen, fp);
		writeword(db->users[i].usersize, fp);
		if (db->users[i].usersize)
			fwrite(db->users[i].user, db->users[i].usersize, 1, fp);
		writeword(db->users[i].passwordsize, fp);
		if (db->users[i].passwordsize)
			fwrite(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
		writeword(db->users[i].downstream, fp);
		writeword(db->users[i].upstream, fp);
		writeword(db->users[i].expiration, fp);
		writeword(db->users[i].enabled, fp);
	}
	fclose(fp);
}

void freeradiusdb(struct radiusdb *db)
{
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		if (db->users[i].passwd && db->users[i].passwordsize)
			free(db->users[i].passwd);
		if (db->users[i].user && db->users[i].usersize)
			free(db->users[i].user);
	}
	if (db->users)
		free(db->users);
	free(db);
}

struct radiusclientdb *loadradiusclientdb(void)
{
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtclients.db", "rb");
	if (fp == NULL)
		return NULL;
	struct radiusclientdb *db;
	if (feof(fp)) {
		fclose(fp);
		return NULL;
	}
	db = malloc(sizeof(struct radiusclientdb));
	db->usercount = readword(fp);
	if (db->usercount)
		db->users = safe_malloc(db->usercount * sizeof(struct radiusclient));
	else
		db->users = NULL;
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		int curlen = 0;
		db->users[i].fieldlen = readword(fp);
		db->users[i].clientsize = readword(fp);
		curlen += 8;
		if (db->users[i].clientsize) {
			db->users[i].client = safe_malloc(db->users[i].clientsize);
			fread(db->users[i].client, db->users[i].clientsize, 1, fp);
			curlen += db->users[i].clientsize;
		} else
			db->users[i].client = NULL;

		db->users[i].passwordsize = readword(fp);
		curlen += 4;
		if (db->users[i].passwordsize) {
			db->users[i].passwd = safe_malloc(db->users[i].passwordsize);
			fread(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
			curlen += db->users[i].passwordsize;
		} else
			db->users[i].passwd = NULL;

		if ((db->users[i].fieldlen - curlen) > 0) //for backward compatiblity
			fseek(fp, db->users[i].fieldlen - curlen, SEEK_CUR);
	}
	fclose(fp);
	return db;
}

void writeradiusclientdb(struct radiusclientdb *db)
{
	FILE *fp = fopen("/jffs/etc/freeradius/dd-wrtclients.db", "wb");
	if (fp == NULL)
		return;
	writeword(db->usercount, fp);
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		if (db->users[i].client)
			db->users[i].clientsize = strlen(db->users[i].client) + 1;
		else
			db->users[i].clientsize = 0;

		if (db->users[i].passwd)
			db->users[i].passwordsize = strlen(db->users[i].passwd) + 1;
		else
			db->users[i].passwordsize = 0;

		db->users[i].fieldlen =
			sizeof(struct radiusclient) + db->users[i].clientsize + db->users[i].passwordsize - (sizeof(char *) * 2);

		writeword(db->users[i].fieldlen, fp);
		writeword(db->users[i].clientsize, fp);
		if (db->users[i].clientsize)
			fwrite(db->users[i].client, db->users[i].clientsize, 1, fp);
		writeword(db->users[i].passwordsize, fp);
		if (db->users[i].passwordsize)
			fwrite(db->users[i].passwd, db->users[i].passwordsize, 1, fp);
	}
	fclose(fp);
}

void freeradiusclientdb(struct radiusclientdb *db)
{
	unsigned int i;
	for (i = 0; i < db->usercount; i++) {
		if (db->users[i].passwd && db->users[i].passwordsize) {
			free(db->users[i].passwd);
		}
		if (db->users[i].client && db->users[i].clientsize) {
			free(db->users[i].client);
		}
	}
	if (db->users) {
		free(db->users);
	}
	free(db);
}

void gen_cert(char *name, int type, char *common, char *pass)
{
	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return;
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
	fprintf(fp, "default_days		= %s\n", nvram_default_get("radius_expiration", "365"));
	fprintf(fp, "default_crl_days	= 30\n"
		    "default_md		= sha256\n"
		    "preserve		= no\n"
		    "policy			= policy_match\n"
		    "\n"
		    "[ policy_match ]\n"
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
		    "\n"
		    "[ req ]\n"
		    "prompt			= no\n");
	if (type == TYPE_CA)
		fprintf(fp, "distinguished_name	= certificate_authority\n");
	else if (type == TYPE_CLIENT)
		fprintf(fp, "distinguished_name	= client\n");
	else
		fprintf(fp, "distinguished_name	= server\n");

	fprintf(fp,
		"default_bits		= 2048\n"
		"input_password		= %s\n"
		"output_password		= %s\n",
		nvram_default_get("radius_passphrase", "whatever"), pass);
	if (type == TYPE_CA) {
		fprintf(fp, "x509_extensions		= v3_ca\n");
		fprintf(fp, "\n"
			    "[certificate_authority]\n");
	} else if (type == TYPE_CLIENT) {
		fprintf(fp, "\n"
			    "[client]\n");
	} else {
		fprintf(fp, "\n"
			    "[server]\n");
	}

	if (!nvram_match("radius_country", ""))
		fprintf(fp, "countryName		= %s\n", nvram_safe_get("radius_country"));
	if (!nvram_match("radius_state", ""))
		fprintf(fp, "stateOrProvinceName	= %s\n", nvram_safe_get("radius_state"));
	if (!nvram_match("radius_locality", ""))
		fprintf(fp, "localityName		= %s\n", nvram_safe_get("radius_locality"));
	if (!nvram_match("radius_organisation", ""))
		fprintf(fp, "organizationName	= %s\n", nvram_safe_get("radius_organisation"));
	if (!nvram_match("radius_email", ""))
		fprintf(fp, "emailAddress		= %s\n", nvram_safe_get("radius_email"));

	fprintf(fp, "commonName		= \"%s\"\n", common);

	if (type == TYPE_CA)
		fprintf(fp, "\n[v3_ca]\n"
			    "subjectKeyIdentifier	= hash\n"
			    "authorityKeyIdentifier	= keyid:always,issuer:always\n"
			    "basicConstraints	= CA:true\n");

	fclose(fp);
}
