/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 */
#define _XOPEN_SOURCE
#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include "ctx.h"
#include "auth.h"
#include "crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static char *ddns3_auth_salts = 
	"abcdefghijklmnopqrstuzwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890./";

static int
auth_plaintext(struct ddns3_ctx *c, char *user, char *passwd) {
	if(!strstr(c->hello, "plaintext")) return -1;

	sprintf(c->buf, "LOGIN %s %s", user, passwd);
	return 0;
}

#ifndef WIN32
static int
auth_crypt(struct ddns3_ctx *c, char *user, char *passwd) {
	char salt[2];
	char *hash;

	if(!strstr(c->hello, "crypt")) return -1;

	srand(getpid() * time(0));
	salt[0] = ddns3_auth_salts[rand() % strlen(ddns3_auth_salts)];
	salt[1] = ddns3_auth_salts[rand() % strlen(ddns3_auth_salts)];
	hash = ddns3_crypto_crypt(passwd, salt);
	sprintf(c->buf, "LOGIN %s %s", user, hash);
	free(hash);
	return 0;
}
#endif

static int
auth_md5(struct ddns3_ctx *c, char *user, char *passwd) {
	char *hash;

	if(!strstr(c->hello, "md5")) return -1;

	hash = ddns3_crypto_md5hash(passwd, strlen(passwd));
	sprintf(c->buf, "LOGIN %s %s", user, hash);
	free(hash);
	return 0;
}

static int
auth_ddns(struct ddns3_ctx *c, char *user, char *passwd) {
	char *hash, *p;
	char buf[4096];

	if(!strstr(c->hello, "ddns")) return -1;

	strcpy(buf, passwd);
	strcat(buf, ":");
	p = strstr(c->hello, "ddns{");
	strncat(buf, p+5, 32);
	hash = ddns3_crypto_md5hash(buf, strlen(buf));
	sprintf(c->buf, "LOGIN %s %s", user, hash);
	free(hash);
	return 0;
}

/* FIXME: there is no server support for strong auth yet */
static int
auth_strong(struct ddns3_ctx *c, char *user, char *passwd) {
	char *phash, *uhash, *challenge, *p;
	char buf[4096];
	int i;

	if(!strstr(c->hello, "ddns")) return -1;

	/* password hash */
	strcpy(buf, passwd);
	strcat(buf, ":");
	p = strstr(c->hello, "ddns{");
	strncat(buf, p+5, 32);
	phash = ddns3_crypto_md5hash(buf, strlen(buf));

	/* random challenge */
	srand(getpid() * time(0));
	for(i = 0; i < 100; i++) buf[i] = rand();
	challenge = ddns3_crypto_md5hash(buf, 100);

	/* username hash */
	strcpy(buf, user);
	strcat(buf, ":");
	strncat(buf, challenge, 32);
	uhash = ddns3_crypto_md5hash(buf, strlen(buf));

	sprintf(c->buf, "LOGIN %s %s %s", uhash, phash, challenge);

	free(uhash);
	free(phash);
	free(challenge);
	return 0;
}


static struct ddns3_auth known_auths[] = {
	{"plaintext",	auth_plaintext},
#ifndef WIN32
	{"crypt",	auth_crypt},
#endif
	{"md5",		auth_md5},
	{"ddns",	auth_ddns},
	{"strong",	auth_strong},
	{0, 0}
};

int
ddns3_auth_makechallenge(struct ddns3_ctx *c, char *auth, char *user, char *passwd) {
	int i;
	int ret = -2;
	
	for(i = 0; known_auths[i].name; i++)
		if(!strcmp(known_auths[i].name, auth)) {
			ret = known_auths[i].makechallenge(c, user, passwd);
			break;
		}
	if(ret < -1)
		strcpy(c->buf, "client_error: authentication type unknown to client\n");
	else if(ret < 0)
		strcpy(c->buf, "client_error: authentication type not supported by server\n");

	return ret;
}
