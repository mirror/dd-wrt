/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 *
 *		Win32 compatability by Kieron Briggs (kieron@kieron.nu)
 */
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "ctx.h"

#define LIST 1
#define SET 2
#define CHECK 3

struct client_vars {
	char *host;
	char *port;
	char *auth;
	char *user;
	char *pass;
	int action;
	char *handle;
	char *ip;
} CV;

static void
help(void) {
	fprintf(stderr, "\n");
	fprintf(stderr, "DDNS Client v3 :: http://www.ddns.nu/\n");
	fprintf(stderr, "(c) 2000 Alan Yates <alany@ay.com.au>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: ddns3 -u username -p password [-s handle ip]\n");
#ifdef WIN32
	fprintf(stderr, "Optional: [-h host] [-n port] [-l] [-a plaintext|md5|ddns]\n");
#else
	fprintf(stderr, "Optional: [-h host] [-n port] [-l] [-a plaintext|crypt|md5|ddns]\n");
#endif
	fprintf(stderr, "\n");
}

static void
parse_args(int argc, char **argv, char **env) {
	int i;

	if(argc == 1) {
		help();
		exit(1);
	}

	for(i = 1; i < argc; i++) {
		if(argv[i][0] != '-') continue;

		if(argv[i][1] == 'l') {
			CV.action = LIST;
			continue;
		}
		if(argv[i][1] == 'c') {
			CV.action = CHECK;
			continue;
		}

		if(i > argc - 2) {
			fprintf(stderr, "client_error: arguments mangled\n");
			help();
			exit(1);
		}

		if(argv[i][1] == 'h') CV.host = argv[++i];
		else if(argv[i][1] == 'n') CV.port = argv[++i];

		else if(argv[i][1] == 'a') CV.auth = argv[++i];
		else if(argv[i][1] == 'u') CV.user = argv[++i];
		else if(argv[i][1] == 'p') CV.pass = argv[++i];
		else if(argv[i][1] == 's') {
			if(i > argc - 3) {
				fprintf(stderr, "client_error: arguments mangled\n");
				help();
				exit(1);
			}
			CV.handle = argv[++i];
			CV.ip = argv[++i];
			CV.action = SET;
		}
	}
}

static void
defaults(void) {
	CV.host = "ns.ddns.nu";
	CV.port = "5000";
	CV.auth = "ddns";
	CV.action = CHECK;
}

static void
check_args(void) {
	int err = 0;

	if(!CV.user) {
		fprintf(stderr, "client_error: you must specify a username\n");
		err ++;
	}

	if(!CV.pass) {
		fprintf(stderr, "client_error: you must specify a password\n");
		err ++;
	}

	if((CV.action == SET) && !CV.handle) {
		fprintf(stderr, "client_error: you must specify a handle in set mode\n");
		err ++;
	}

	if((CV.action == SET) && !CV.ip) {
		fprintf(stderr, "client_error: you must specify a IP in set mode\n");
		err ++;
	}

	if(err > 0) {
		fprintf(stderr, "client_exit: argument parsing; %d errors\n", err);
		exit(1);
	}
}

int
main(int argc, char **argv, char **env) {
	struct ddns3_ctx *c;

	/* arguments and ugly stuff */
	memset(&CV, 0, sizeof(struct client_vars));
	defaults();
	parse_args(argc, argv, env);
	check_args();

	/* now the long and ugly stuff */

	/* connect and login to DDNS server */
	if(ddns3_ctx_new(&c, CV.host, atoi(CV.port)) < 0) {
		fprintf(stderr, "client_error: creating ddns3_ctx, exiting\n");
		exit(1);
	}
	if(ddns3_ctx_connect(c) < 0) {
		fprintf(stderr, "server_message: %s", c->buf);
		fprintf(stderr, "client_error: could not connect to %s:%d, exiting\n", CV.host, atoi(CV.port));
		exit(1);
	}
	if(ddns3_ctx_login(c, CV.auth, CV.user, CV.pass) < 0) {
		fprintf(stderr, "server_message: %s", c->buf);
		fprintf(stderr, "client_error: could not authenticate\n");
		exit(1);
	}

	if(CV.action == LIST) {
		if(ddns3_ctx_list(c) < 0) {
			fprintf(stderr, "server_message: %s", c->buf);
			fprintf(stderr, "client_error: could not get handle list\n");
		} else {
			int i;

			for(i = 0; c->list[i]; i += 2) {
				printf("%-20s %s\n", c->list[i], c->list[i+1]);
			}
		}
	} else if(CV.action == SET) {
		if(ddns3_ctx_set(c, CV.handle, CV.ip) < 0) {
			fprintf(stderr, "server_message: %s", c->buf);
			fprintf(stderr, "client_error: could not set handle %s to ip %s, closing\n", CV.handle, CV.ip);
		} else printf("server_message: %s", c->buf);
	} else if(CV.action == CHECK) {
		printf("server_message: %s", c->buf);
	}

	/* close connection nicely */
	if(ddns3_ctx_logout(c) < 0) {
		fprintf(stderr, "server_message: %s", c->buf);
		fprintf(stderr, "client_error: could not logout, giving up clean goodbye, exiting\n");
		exit(1);
	}
	if(ddns3_ctx_disconnect(c) < 0) {
		fprintf(stderr, "server_message: %s", c->buf);
		fprintf(stderr, "client_error: disconnect failed, giving up\n");
		exit(1);
	}
	if(ddns3_ctx_del(&c) < 0) {
		fprintf(stderr, "client_error: could not delete ddns3_ctx, oh well\n");
		exit(1);
	}
	
	return 0;
}
