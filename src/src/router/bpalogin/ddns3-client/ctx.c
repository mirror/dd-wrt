/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 */
#include "ctx.h"
#include "sockio.h"
#include "auth.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _DEBUG
	#define DEBUG(x) x
#else
	#define DEBUG(x)
#endif /* _DEBUG */

static int
ctxsend(struct ddns3_ctx *c, char *cmd) { 
	int ret;

	ret = sprintf(c->buf, "%s\r\n", cmd);
	ret = ddns3_sockio_write(c->sock, c->buf, ret);
	if(ret < 0) return -1;
	DEBUG( printf(">>> %s", c->buf); )
	return 0;
}

static int
recvline(struct ddns3_ctx *c) {
	int ret;

	/* FIXME: we expect atomic line reception, fairly evil stuff */
	ret = ddns3_sockio_read(c->sock, c->buf, DDNS3_BUF);
	if(ret < 0) return -1;
	c->buf[ret] = 0;
	DEBUG( printf("<<< %s", c->buf); )
	if(!strncmp(c->buf, "+OK",  3)) return T_OK;
	if(!strncmp(c->buf, "-ERR", 4)) return T_ERR;
	if(!strncmp(c->buf, "@RET", 4)) return T_RET;
	if(!strncmp(c->buf, "DDNS", 4)) return T_DDNS;
	if(!strncmp(&(c->buf[ret-3]), ".\r\n", 3)) return T_DOT;
	return -1;
}

static int
recvlist(struct ddns3_ctx *c) {
	int ret, i = 0;
	char *p1, *p2, **lp = 0;
	char handle[128], ip[16];

	if(c->list) {
		for(p1 = c->list[i]; p1; p1 = c->list[i++]) 
			free(p1);
		free(c->list);
		c->list = 0;
	}

	/* FIXME: we excpect atomic list reception, evil! */
	ret = recvline(c);
	if(ret != T_DOT) return -1;
	c->buf[strlen(c->buf) - 3] = 0;

	/* parse the list we get back, yuck */
	i = 0;
	p2 = c->buf;
	lp = (char **) realloc(lp, (i + 1) * sizeof(char *));
	while((p1 = strtok(p2, "\n"))) {
		i += 2;
		lp = (char **) realloc(lp, (i + 1) * sizeof(char *));
		sscanf(p1, "%s%s", handle, ip);
		lp[i-2] = strdup(handle);
		lp[i-1] = strdup(ip);
		lp[i] = 0;
		p2 = 0;
	}
	c->list = lp;

	return 0;
}



int
ddns3_ctx_new(struct ddns3_ctx **c, char *host, int port) {

	/* allocate context */
	*c = (struct ddns3_ctx *) calloc(1, sizeof(struct ddns3_ctx));
	if(!(*c)) {
		perror("calloc()");
		return -1;
	}

	/* set URL */
	sprintf((*c)->buf, "%s:%hd", host, (short) port);
	(*c)->url = (char *) strdup((*c)->buf);

	/* initialize sockets */
	ddns3_sockio_init();

	return 0;
}

int
ddns3_ctx_del(struct ddns3_ctx **c) {
	int i = 0;
	char *p;

	/* just in case */
	ddns3_sockio_close((*c)->sock);

	if((*c)->url) free((*c)->url);
	if((*c)->hello) free((*c)->hello);

	if((*c)->list) {
		for(p = (*c)->list[i]; p; p = (*c)->list[++i]) 
			free(p);
		free((*c)->list);
	}

	free(*c);
	*c = 0;

	/* clean up socket library */
	ddns3_sockio_cleanup();

	return 0;
}

int
ddns3_ctx_connect(struct ddns3_ctx *c) {
	int ret;
	
	/* connect to server */
	DEBUG( printf("connect(%s)... ", c->url); )
	ret = ddns3_sockio_connect(c->url);
	if(ret < 0) return -1;
	c->sock = ret;
	DEBUG( printf("OK\n"); )
	
	/* grab opening header */
	ret = recvline(c);
	if(ret != T_DDNS) return -1;
	c->hello = strdup(c->buf);

	return 0;
}

int
ddns3_ctx_disconnect(struct ddns3_ctx *c) {
	int ret;
	
	/* QUIT */
	ret = ctxsend(c, "QUIT");
	if(ret < 0) return -1;

	/* reply */
	ret = recvline(c);
	if(ret != T_OK) return -1;

	/* close socket */
	ddns3_sockio_close(c->sock);
	return 0;
}

int
ddns3_ctx_login(struct ddns3_ctx *c, char *auth, char *user, char *passwd) {
	int ret;
	char buf[4096];

	/* build challenge command */
	ret = ddns3_auth_makechallenge(c, auth, user, passwd);
	if(ret < 0) return -1;

	/* LOGIN */
	strcpy(buf, c->buf);
	ret = ctxsend(c, buf);
	if(ret < 0) return -1;

	/* reply */
	ret = recvline(c);
	if(ret != T_OK) return -1;

	return 0;
}

int
ddns3_ctx_logout(struct ddns3_ctx *c) {
	int ret;
	
	/* LOGOUT */
	ret = ctxsend(c, "LOGOUT");
	if(ret < 0) return -1;

	/* reply */
	ret = recvline(c);
	if(ret != T_OK) return -1;

	return 0;
}

int
ddns3_ctx_list(struct ddns3_ctx *c) {
	int ret;
	
	/* LIST */
	ret = ctxsend(c, "LIST");
	if(ret < 0) return -1;

	/* get returned value */
	ret = recvline(c);
	if(ret != T_RET) return -1;

	ret = recvlist(c);
	if(ret < 0) return -1;

	return 0;
}

int
ddns3_ctx_set(struct ddns3_ctx *c, char *handle, char *ip) {
	int ret;
	char buf[4096];
	
	/* SET */
	ret = sprintf(buf, "SET %s %s", handle, ip);
	ret = ctxsend(c, buf);
	if(ret < 0) return -1;

	/* reply */
	ret = recvline(c);
	if(ret != T_OK) return -1;

	return 0;
}
