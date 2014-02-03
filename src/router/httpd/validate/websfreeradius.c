#ifdef HAVE_FREERADIUS
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else				/* !WEBS */
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
#endif				/* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
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

extern char *(*websGetVar) (webs_t wp, char *var, char *d);

void radius_generate_certificate(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	nvram_set("radius_country", websGetVar(wp, "radius_country", ""));
	nvram_set("radius_state", websGetVar(wp, "radius_state", ""));
	nvram_set("radius_locality", websGetVar(wp, "radius_locality", ""));
	nvram_set("radius_expiration", websGetVar(wp, "radius_expiration", "365"));
	nvram_set("radius_passphrase", websGetVar(wp, "radius_passphrase", "whatever"));
	nvram_set("radius_organisation", websGetVar(wp, "radius_organisation", ""));
	nvram_set("radius_email", websGetVar(wp, "radius_email", ""));
	nvram_set("radius_common", websGetVar(wp, "radius_common", ""));

//system("rm /jffs/etc/freeradius/certs/dh");
	system("rm /jffs/etc/freeradius/certs/server.csr");
	system("rm /jffs/etc/freeradius/certs/server.key");
	system("rm /jffs/etc/freeradius/certs/ca.pem");
	system("rm /jffs/etc/freeradius/certs/ca.key");
	system("rm /jffs/etc/freeradius/certs/server.crt");
	system("rm /jffs/etc/freeradius/certs/server.p12");
	system("rm /jffs/etc/freeradius/certs/server.pem");
	system("rm /jffs/etc/freeradius/certs/ca.der");
	system("rm /jffs/etc/freeradius/certs/index.txt");
	system("rm /jffs/etc/freeradius/certs/serial");
	system("rm -rf /jffs/etc/freeradius/certs/clients");	//delete client certificates since they will become invalid
	system("startservice_f gen_radius_cert");
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
		db->users = realloc(db->users, sizeof(struct radiususer) * (db->usercount + 1));
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
		memcpy(&db->users[todel], &db->users[todel + 1], sizeof(struct radiususer) * ((db->usercount - 1) - todel));
	db->usercount--;
	if (db->usercount > 0)
		db->users = realloc(db->users, sizeof(struct radiususer) * (db->usercount));
	else {
		free(db->users);
		db->users = NULL;
	}
	writeradiusdb(db);
	freeradiusdb(db);
}

void add_radius_client(webs_t wp)
{
	nvram_set("radius_enabled", websGetVar(wp, "radius_enabled", "0"));
	struct radiusclientdb *db = loadradiusclientdb();
	if (db == NULL) {
		db = safe_malloc(sizeof(struct radiusclientdb));
		db->usercount = 0;
		db->users = safe_malloc(sizeof(struct radiusclient));
	} else {
		db->users = realloc(db->users, sizeof(struct radiusclient) * (db->usercount + 1));
	}
	db->users[db->usercount].fieldlen = sizeof(struct radiususer) - 8;
	db->users[db->usercount].clientsize = 0;
	db->users[db->usercount].client = NULL;
	db->users[db->usercount].passwd = NULL;
	db->users[db->usercount].passwordsize = 0;
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
		memcpy(&db->users[todel], &db->users[todel + 1], sizeof(struct radiusclient) * ((db->usercount - 1) - todel));
	db->usercount--;
	if (db->usercount > 0)
		db->users = realloc(db->users, sizeof(struct radiusclient) * (db->usercount));
	else {
		free(db->users);
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

		db->users = realloc(db->users, sizeof(struct radiusclient) * (db->usercount + 1));

		db->users[db->usercount].client = safe_malloc(strlen(u) + 1);
		strcpy(db->users[db->usercount].client, u);
		db->users[db->usercount].clientsize = strlen(u) + 1;
		db->users[db->usercount].passwd = safe_malloc(strlen(p) + 1);
		strcpy(db->users[db->usercount].passwd, p);
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
		sprintf(filename, "/jffs/etc/freeradius/certs/clients/%s-cert.pem", u);
		sysprintf("rm -f %s", filename);
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

		char *en = websGetVar(wp, enabled, "0");	// returns NULL if not set
		if (!en)
			break;
		db->users = realloc(db->users, sizeof(struct radiususer) * (db->usercount + 1));

		db->users[db->usercount].user = safe_malloc(strlen(u) + 1);
		strcpy(db->users[db->usercount].user, u);
		db->users[db->usercount].usersize = strlen(u) + 1;
		db->users[db->usercount].passwd = safe_malloc(strlen(p) + 1);
		strcpy(db->users[db->usercount].passwd, p);
		db->users[db->usercount].passwordsize = strlen(p) + 1;
		db->users[db->usercount].downstream = atoi(d);
		db->users[db->usercount].upstream = atoi(up);
		long expiration = atol(e);
		if (expiration) {
			long curtime = ((tm / 60) / 60) / 24;	//in days
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
	nvram_set("nowebaction", "1");
	applytake(value);
}

#endif
