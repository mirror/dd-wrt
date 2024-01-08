/*
 * websfreeradius.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <stdarg.h>
#include <sha1.h>

void radius_generate_certificate(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	nvram_set("radius_country", websGetVar(wp, "radius_country", ""));
	nvram_set("radius_state", websGetVar(wp, "radius_state", ""));
	nvram_set("radius_locality", websGetVar(wp, "radius_locality", ""));
	nvram_set("radius_expiration",
		  websGetVar(wp, "radius_expiration", "365"));
	nvram_set("radius_passphrase",
		  websGetVar(wp, "radius_passphrase", "whatever"));
	nvram_set("radius_organisation",
		  websGetVar(wp, "radius_organisation", ""));
	nvram_set("radius_email", websGetVar(wp, "radius_email", ""));
	nvram_set("radius_common", websGetVar(wp, "radius_common", ""));

	//system("rm /jffs/etc/freeradius/certs/dh");
	unlink("/jffs/etc/freeradius/certs/server.csr");
	unlink("/jffs/etc/freeradius/certs/server.key");
	unlink("/jffs/etc/freeradius/certs/ca.pem");
	unlink("/jffs/etc/freeradius/certs/ca.key");
	unlink("/jffs/etc/freeradius/certs/server.crt");
	unlink("/jffs/etc/freeradius/certs/server.p12");
	unlink("/jffs/etc/freeradius/certs/server.pem");
	unlink("/jffs/etc/freeradius/certs/ca.der");
	unlink("/jffs/etc/freeradius/certs/index.txt");
	unlink("/jffs/etc/freeradius/certs/serial");
	unlink("/jffs/etc/freeradius/certs/clients"); //delete client certificates since they will become invalid
	eval("startservice", "gen_radius_cert", "-f");
}

/*struct radiususer {
	unsigned int fieldlen;
	unsigned int usersize;
	unsigned char *user;
	unsigned int passwordsize;
	unsigned char *passwd;
	unsigned int downstream;
	unsigned int upstream;
//more fields can be added in future
};

struct radiusdb {
	unsigned int usercount;
	struct radiususer *users;
};
*/
#include <radiusdb.h>
void add_radius_user(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	struct radiusdb *db = loadradiusdb();
	if (db == NULL) {
		db = safe_malloc(sizeof(struct radiusdb));
		db->usercount = 0;
		db->users = safe_malloc(sizeof(struct radiususer));
	} else {
		db->users = realloc(db->users, sizeof(struct radiususer) *
						       (db->usercount + 1));
	}
	db->users[db->usercount].fieldlen = sizeof(struct radiususer) - 8;
	db->users[db->usercount].usersize = 0;
	db->users[db->usercount].user = NULL;
	db->users[db->usercount].passwd = NULL;
	db->users[db->usercount].passwordsize = 0;
	db->users[db->usercount].downstream = 0;
	db->users[db->usercount].upstream = 0;
	db->users[db->usercount].expiration = 0;
	db->users[db->usercount].enabled = 1;
	db->usercount++;
	writeradiusdb(db);
	freeradiusdb(db);
}

void del_radius_user(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	char *val = websGetVar(wp, "del_value", NULL);
	if (val == NULL)
		return;
	int todel = atoi(val);
	struct radiusdb *db = loadradiusdb();
	if (db == NULL)
		return;
	if (db->usercount == 0)
		return;
	if (db->usercount > 1)
		memcpy(&db->users[todel], &db->users[todel + 1],
		       sizeof(struct radiususer) *
			       ((db->usercount - 1) - todel));
	db->usercount--;
	if (db->usercount > 0)
		db->users = realloc(db->users, sizeof(struct radiususer) *
						       (db->usercount));
	else {
		debug_free(db->users);
		db->users = NULL;
	}
	writeradiusdb(db);
	freeradiusdb(db);
}

void add_radius_client(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	struct radiusclientdb *db = loadradiusclientdb();
	if (!db) {
		db = malloc(sizeof(struct radiusclientdb));
		db->users = malloc(sizeof(struct radiusclient));
		db->usercount = 0;
	} else {
		db->users = realloc(db->users, sizeof(struct radiusclient) *
						       (db->usercount + 1));
	}
	bzero(&db->users[db->usercount], sizeof(struct radiusclient));
	db->users[db->usercount].fieldlen =
		sizeof(struct radiusclient) - (sizeof(char *) * 2);
	db->usercount++;
	writeradiusclientdb(db);
	freeradiusclientdb(db);
}

void del_radius_client(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	char *val = websGetVar(wp, "del_value", NULL);
	if (val == NULL)
		return;
	int todel = atoi(val);
	struct radiusclientdb *db = loadradiusclientdb();
	if (db == NULL)
		return;
	if (db->usercount == 0)
		return;
	if (db->usercount > 1)
		memcpy(&db->users[todel], &db->users[todel + 1],
		       sizeof(struct radiusclient) *
			       ((db->usercount - 1) - todel));
	db->usercount--;
	if (db->usercount > 0)
		db->users = realloc(db->users, sizeof(struct radiusclient) *
						       (db->usercount));
	else {
		debug_free(db->users);
		db->users = NULL;
	}
	writeradiusclientdb(db);
	freeradiusclientdb(db);
}

static void save_radius_clients(webs_t wp)
{
	char passwd[] = { "passwordXXXXX" };
	char user[] = { "usernameXXXXX" };
	struct radiusclientdb *db = safe_malloc(sizeof(struct radiusclientdb));
	db->usercount = 0;
	db->users = NULL;
	while (1) {
		sprintf(user, "client%d", db->usercount);
		sprintf(passwd, "shared%d", db->usercount);
		char *u = websGetVar(wp, user, NULL);
		if (!u)
			break;
		char *p = websGetVar(wp, passwd, NULL);
		if (!p)
			break;

		db->users = realloc(db->users, sizeof(struct radiusclient) *
						       (db->usercount + 1));

		db->users[db->usercount].client = strdup(u);
		db->users[db->usercount].clientsize = strlen(u) + 1;
		db->users[db->usercount].passwd = strdup(p);
		db->users[db->usercount].passwordsize = strlen(p) + 1;
		db->usercount++;
	}
	writeradiusclientdb(db);
	freeradiusclientdb(db);
}

static void save_radius_users(webs_t wp)
{
	char passwd[] = { "passwordXXXXX" };
	char user[] = { "usernameXXXXX" };
	char downstream[] = { "passwordXXXXX" };
	char upstream[] = { "usernameXXXXX" };
	char expiration[] = { "expirationXXXXX" };
	char enabled[] = { "enabledXXXXX" };
	struct radiusdb *db = safe_malloc(sizeof(struct radiusdb));
	char filename[128];
	db->usercount = 0;
	db->users = NULL;
	time_t tm;
	time(&tm);
	while (1) {
		sprintf(user, "username%d", db->usercount);
		sprintf(passwd, "password%d", db->usercount);
		sprintf(downstream, "downstream%d", db->usercount);
		sprintf(upstream, "upstream%d", db->usercount);
		sprintf(expiration, "expiration%d", db->usercount);
		sprintf(enabled, "enabled%d", db->usercount);
		char *u = websGetVar(wp, user, NULL);
		if (!u)
			break;
		sprintf(filename,
			"/jffs/etc/freeradius/certs/clients/%s-cert.pem", u);
		unlink(filename);
		char *p = websGetVar(wp, passwd, NULL);
		if (!p)
			break;

		char *d = websGetVar(wp, downstream, NULL);
		if (!d)
			break;

		char *up = websGetVar(wp, upstream, NULL);
		if (!up)
			break;

		char *e = websGetVar(wp, expiration, NULL);
		if (!e)
			break;

		char *en =
			websGetVar(wp, enabled, "0"); // returns NULL if not set
		if (!en)
			break;
		db->users = realloc(db->users, sizeof(struct radiususer) *
						       (db->usercount + 1));

		db->users[db->usercount].user = strdup(u);
		db->users[db->usercount].usersize = strlen(u) + 1;
		db->users[db->usercount].passwd = strdup(p);
		db->users[db->usercount].passwordsize = strlen(p) + 1;
		db->users[db->usercount].downstream = atoi(d);
		db->users[db->usercount].upstream = atoi(up);
		long expiration = atol(e);
		if (expiration) {
			long curtime = ((tm / 60) / 60) / 24; //in days
			expiration = expiration + curtime;
		}
		db->users[db->usercount].expiration = expiration;
		db->users[db->usercount].enabled = atoi(en);
		db->usercount++;
	}
	writeradiusdb(db);
	freeradiusdb(db);
}

void save_radius_user(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	nvram_set("radius_port", websGetVar(wp, "radius_port", "1812"));
	save_radius_users(wp);
	save_radius_clients(wp);

	char *value = websGetVar(wp, "action", "");
	addAction("freeradius");
	nvram_seti("nowebaction", 1);
	applytake(value);
}

#endif
