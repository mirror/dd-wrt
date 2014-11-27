/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: arpalert.c 421 2006-11-04 10:56:25Z thierry $
 *
 */

#include "config.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "data.h"
#include "maclist.h"
#include "alerte.h"
#include "sens.h"
#include "loadmodule.h"
#include "macname.h"

extern int errno;

void die(int);
void dumpmaclist(int);

// set flag signals
int sigchld = 0;
int sighup  = 0;
int sigstop = 0;

void setsigchld(int signal){
	sigchld++;
	#ifdef DEBUG
	logmsg(LOG_DEBUG, "[%s %d %s] SIGCHLD",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif
}
void setsighup(int signal){
	sighup++;
	#ifdef DEBUG
	logmsg(LOG_DEBUG, "[%s %d %s] SIGHUP",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif
}
void setsigstop(int signal){
	sigstop++;
	#ifdef DEBUG
	logmsg(LOG_DEBUG, "[%s %d %s] SIGSTOP",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif
}

// setup signals
void (*setsignal (int signal, void (*function)(int)))(int) {    
	struct sigaction old, new;

	memset(&new, 0, sizeof(struct sigaction));
	new.sa_handler = function;
	new.sa_flags = SA_RESTART;
	sigemptyset(&(new.sa_mask));
	if (sigaction(signal, &new, &old)){ 
		logmsg(LOG_ERR, "[%s %i] sigaction[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	return(old.sa_handler);
}

// init signals
void signals_init(void){
	(void)setsignal(SIGINT,  setsigstop);
	(void)setsignal(SIGTERM, setsigstop);
	(void)setsignal(SIGQUIT, setsigstop);
	(void)setsignal(SIGABRT, setsigstop);
	(void)setsignal(SIGCHLD, setsigchld); 
	(void)setsignal(SIGHUP,  setsighup); 
}

// signals computing
void signals_func(void){

	// SIGCHLD
	if(sigchld > 0){
		sigchld--;
		alerte_kill_pid();
	}
	// SIGHUP
	if(sighup > 0){
		sighup--;
		maclist_reload();
		sens_reload();
		macname_reload();
		logfile_reload();
	}
	// SIGQUIT
	if(sigstop > 0){
		sigstop--;
		#ifdef DEBUG
		logmsg(LOG_DEBUG, "[%s %i %s] quit arpalert",
		       __FILE__, __LINE__, __FUNCTION__);
		#endif
		data_dump();
		module_unload();
		exit(0);
	}
}

// get next signal
void *signals_next(struct timeval *tv){
	if(sigchld > 0 ||
	   sighup  > 0 ||
	   sigstop > 0){
		tv->tv_sec = 0;
		tv->tv_usec = 0;
		return signals_func;
	}

	tv->tv_sec = -1;
	return NULL;
}

