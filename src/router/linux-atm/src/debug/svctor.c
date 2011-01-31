/* svctor.c - SVC Torture */

/* Written 1998,1999 by Werner Almesberger, EPFL ICA */

/*
 * This program frantically tries to concurrently set up connections to
 * itself. Once it has obtained all the connections it was looking for,
 * it exits, leaving the system with a lot of things to clean up.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#define ITF		 0 /* interface we use - should be command-line arg */
#define MAX_PAR		 4 /* maximum number of concurrent connect()s */
#define EXIT_LIM	 3 /* exit after establishing that many connections */
#define MAX_ADDR	10 /* maximum number of local addresses */
#define SAP		"bhli:oui=0x0060D7,id=0x010000ff"
#define QOS		"ubr,aal5:tx:max_sdu=100,rx:max_sdu=100"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <atm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/atmdev.h>


int main(void)
{
    static int fd[MAX_PAR]; /* to force initialization */
    struct sockaddr_atmsvc local[MAX_ADDR];
    struct atmif_sioc req;
    struct atm_sap sap;
    struct atm_qos qos;
    int listen_fd;
    fd_set rset,wset;
    int fds,completed = 0,connects = 0,accepts = 0;

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    if (text2sap(SAP,&sap,0) < 0) {
	fprintf(stderr,"text2sap\n");
	return 1;
    }
    if (text2qos(QOS,&qos,0) < 0) {
	fprintf(stderr,"text2qos\n");
	return 1;
    }
    listen_fd = socket(PF_ATMSVC,SOCK_DGRAM,0);
    if (listen_fd < 0) {
	perror("socket");
	return 1;
    }
    req.number = ITF;
    req.arg = local;
    req.length = sizeof(local);
    if (ioctl(listen_fd,ATM_GETADDR,&req) < 0) {
	perror("ioctl");
	return 1;
    }
    if (!req.length) {
	fprintf(stderr,"No local address\n");
	return 1;
    }
    if (setsockopt(listen_fd,SOL_ATM,SO_ATMSAP,&sap,sizeof(sap)) < 0) {
	perror("setsockopt SO_ATMSAP");
	return 1;
    }
    if (setsockopt(listen_fd,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (bind(listen_fd,(struct sockaddr *) local,sizeof(*local)) < 0) {
	perror("bind");
	return 1;
    }
    if (fcntl(listen_fd,F_SETFL,O_NONBLOCK) < 0) {
	perror("fnctl");
	return 1;
    }
    if (listen(listen_fd,5) < 0) {
	perror("listen");
	return 1;
    }
    FD_SET(listen_fd,&rset);
    fds = listen_fd+1;
    (void) signal(SIGCHLD,SIG_IGN);
    while (1) {
	static struct timeval no_delay;
	fd_set _rset = rset;
	fd_set _wset = wset;
	int ret,i,empty;

	no_delay.tv_sec = 0;
	no_delay.tv_usec = 100000;
	ret = select(fds,&_rset,&_wset,NULL,&no_delay);
	if (ret < 0) {
	    perror("select");
	    return 1;
	}
	if (FD_ISSET(listen_fd,&_rset)) {
	    pid_t pid;

	    pid = fork();
	    if (pid < 0) {
		perror("fork");
		return 1;
	    }
	    if (!pid) {
		if (accept(listen_fd,NULL,NULL) >= 0) exit(0);
		perror("accept");
		return 1;
	    }
	    accepts++;
	}
	empty = -1;
	for (i = 0; i < MAX_PAR; i++)
	    if (!fd[i]) empty = i;
	    else if (FD_ISSET(fd[i],&_wset)) {
		    struct sockaddr_atmsvc dummy;

		    if (connect(fd[i],(struct sockaddr *) &dummy,sizeof(dummy))
		      < 0) {
			perror("connect");
			return 1;
		    }
		    FD_CLR(fd[i],&wset);
		    fd[i] = 0;
		    empty = i;
		    if (++completed == EXIT_LIM) {
			printf("%d attempted, %d completed, %d accepts\n",
			  connects,completed,accepts);
			return 0;
		    }
		}
	if (empty != -1) {
	    fd[empty] = socket(PF_ATMSVC,SOCK_DGRAM,0);
	    if (fd[empty] < 0) {
		perror("socket");
		return 1;
	    }
	    if (fcntl(fd[empty],F_SETFL,O_NONBLOCK) < 0) {
		perror("fnctl");
		return 1;
	    }
	    if (setsockopt(fd[empty],SOL_ATM,SO_ATMSAP,&sap,sizeof(sap)) < 0) {
		perror("setsockopt SO_ATMSAP");
		return 1;
	    }
	    if (setsockopt(fd[empty],SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
		perror("setsockopt SO_ATMQOS");
		return 1;
	    }
	    if (connect(fd[empty],(struct sockaddr *) local,sizeof(*local)) < 0
	      && errno != EINPROGRESS) {
		perror("connect");
		return 1;
	    }
	    FD_SET(fd[empty],&wset);
	    if (fds <= fd[empty]) fds = fd[empty]+1;
	    connects++;
	}
    }
    return 0;
}
