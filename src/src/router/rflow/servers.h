#ifndef	__SERVERS_H__
#define __SERVERS_H__

#include "headers.h"

typedef struct server_s {
	pthread_t thid;        /* Thread ID */
	char *name;            /* Thread name */
	volatile sig_atomic_t started_ok;        /* A success flag */
	void *(*server_func)(void *);	/* Server function */

	struct sockaddr_in addr;	/* Address to listen on or send to */
	int sockfd;			/* Socket file descriptor */

	char buf[4096];		/* Misc. buffer */
	size_t buf_off;		/* Buffer offset */

	struct server_s *next;	/* Next server in list */
} server;

extern server *servers_head;


int add_server(void *(*func)(void *), char *name, struct in_addr *listen_ip, int hport);
int start_servers();
void end_servers();

/* RSH server */
void *rsh_server(void *);

/* NetFlow exporter */
void *netflow_exporter(void *);

#endif	/* __SERVERS_H__ */
