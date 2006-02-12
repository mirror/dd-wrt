#include "rflow.h"
#include "servers.h"
#include "opt.h"

server *servers_head = NULL;

int
add_server(void *(*func)(void *), char *name, struct in_addr *ip, int hport) {
	server *srv;

	if(!func || !name)
		return -1;

	srv = (server *)calloc(1, sizeof(server));
	if(!srv)
		return -1;

	srv->name = strdup(name);
	if(!srv->name) {
		free(srv);
		return -1;
	}

	srv->server_func = func;

	srv->addr.sin_family = AF_INET;
	if(ip) srv->addr.sin_addr = *ip;
	srv->addr.sin_port = htons(hport);
	srv->sockfd = -1;

	srv->next = servers_head;
	servers_head = srv;

	return 0;
}

int
start_servers() {
	sigset_t set, oset;
	server *srv;
	int rc;

	if(!servers_head) {
		printf("No servers defined. How you will gather data? ;)\n");
		return 0;
	}

	/* Block all signals */
	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, &oset);

	/* Iterate through defined servers */
	for(srv = servers_head; srv; srv=srv->next) {
		if(srv->started_ok)
			continue;

		rc = pthread_create(&srv->thid, NULL, srv->server_func, (void *)srv);

		if(rc == 0) {
			/* Wait for thread to terminate or report success */
			while(
				(pthread_kill(srv->thid, 0) == 0)
				&& (srv->started_ok == 0)
			) {
#ifdef	HAVE_SCHED_YIELD
				sched_yield();
#else
				sleep(1);
#endif
			}
			if(!srv->started_ok)
				rc = -1;
		}

		/* Failed to create particular server */
		if(rc == -1) {
			srv->thid = 0;

			/* Unblock signals */
			sigprocmask(SIG_SETMASK, &oset, NULL);

			/* Terminate previously started servers */
			end_servers();
			return -1;
		}

		printf("%s thread started.\n", srv->name);

	}

	/* Unblock signals */
	sigprocmask(SIG_SETMASK, &oset, NULL);
	
	return 0;
}

void
end_servers_r(server *srv) {
	if(!srv)
		return;

	if(srv->next)
		end_servers_r(srv->next);

	/* If alive */
	if(srv->thid && srv->started_ok) {

		/* Trying to cancel */
		if(pthread_cancel(srv->thid)) {
			printf("%s thread is already gone.\n", srv->name);
		} else {
			/* Kick it */
			pthread_kill(srv->thid, SIGALRM);

			printf("Waiting for %s thread to terminate... ",
				srv->name);
			fflush(stdout);

			pthread_join(srv->thid, NULL);
			printf("DONE.\n");
		}

	}	/* if(thid) */

	free(srv);
}


void
end_servers() {
	sig_atomic_t tmps;

	/* Remember value */
	tmps = signoff_now;

	/* Enable global signoff flag */
	signoff_now = 1;

	/* Terminate working servers */
	end_servers_r(servers_head);

	servers_head = NULL;

	/* Restore previous value */
	signoff_now = tmps;
}
