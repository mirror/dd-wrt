/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 */
#ifndef _CTX_H
#define _CTX_H

#define T_OK	1
#define T_ERR	2
#define T_RET	3
#define T_DDNS	4
#define T_DOT	5

#define DDNS3_BUF 4096

struct ddns3_ctx {
	int sock;
	char *url;
	char *hello;
	
	char buf[DDNS3_BUF];
	char **list;
};

int ddns3_ctx_new(struct ddns3_ctx **c, char *host, int port);
int ddns3_ctx_del(struct ddns3_ctx **c);

int ddns3_ctx_connect(struct ddns3_ctx *c);
int ddns3_ctx_disconnect(struct ddns3_ctx *c);

int ddns3_ctx_login(struct ddns3_ctx *c, char *auth, char *user, char *passwd);
int ddns3_ctx_logout(struct ddns3_ctx *c);

int ddns3_ctx_list(struct ddns3_ctx *c);
int ddns3_ctx_set(struct ddns3_ctx *c, char *handle, char *ip);

#endif /* _CTX_H */
