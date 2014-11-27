/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: serveur.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

#include "arpalert.h"
#include "loadconfig.h"
#include "log.h"

extern int errno;

void daemonize(void){
	int pid;
	int descriptor;
	
	pid = fork();
	if(pid < 0){
		logmsg(LOG_ERR, "[%s %i] fork[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	if(pid > 0){
		exit(0);
	}

	if( setsid() == -1 ){
		logmsg(LOG_ERR, "[%s %i] setsid[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}

	pid = fork();
	if(pid < 0){
		logmsg(LOG_ERR, "[%s %i] fork[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	if(pid > 0){
		exit(0);
	}

	// close standard file descriptors
	close(0);
	close(1);
	close(2);

	// open standard descriptors on /dev/null
	descriptor = open("/dev/null", O_RDWR);
	if(descriptor < 0){
		logmsg(LOG_ERR, "[%s %i] open[%d]: %s (/dev/null)",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	if(dup(descriptor) == -1){
		logmsg(LOG_ERR, "[%s %i] dup[%d]: %s (/dev/null)",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	if(dup(descriptor) == -1){
		logmsg(LOG_ERR, "[%s %i] dup[%d]: %s (/dev/null)",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}

	is_forked = TRUE;
}

void separe(void){
	struct passwd *pwd = NULL;
	uid_t uid = 0;
	gid_t gid = 0;
	char str[8]; // max process number = 9999999
	int fd;

	// open lock/pid file
	if ( config[CF_LOCKFILE].valeur.string != NULL &&
	     config[CF_LOCKFILE].valeur.string[0] != 0 ) {

		fd = open(config[CF_LOCKFILE].valeur.string, O_RDWR|O_CREAT, 0640);
		if(fd < 0){
			logmsg(LOG_ERR, "[%s %i] CF_LOCKFILE open(\"%s\")[%d]: %s",
			       __FILE__, __LINE__, config[CF_LOCKFILE].valeur.string, errno, strerror(errno));
			exit(1);
		}
	
		// lock file during program execution
		if(lockf(fd, F_TLOCK, 0)<0){
			logmsg(LOG_ERR,
			       "daemon instance already running (file: %s locked)",
			       config[CF_LOCKFILE].valeur.string);
			exit(1);
		}
	
		// write pid in lock file
		snprintf(str, 8, "%d\n", (int)getpid());
		write(fd, str, strlen(str));
	}

	// privilege separation
	if(config[CF_USER].valeur.string != NULL && 
	   config[CF_USER].valeur.string[0] != 0) { 

		// get uid and gid by username 
		pwd = getpwnam(config[CF_USER].valeur.string);
		if (pwd == NULL) {
			if (errno == EINTR ||
			    errno == EIO ||
			    errno == EMFILE ||
			    errno == ENFILE ||
			    errno == ENOMEM ||
			    errno == ERANGE) {
				logmsg(LOG_ERR, "[%s %i] getpwnam(%s): %s",
				       __FILE__, __LINE__, config[CF_USER].valeur.string,
				       strerror(errno));
			} else {
				logmsg(LOG_ERR, "[%s %i] getpwnam: user \"%s\" not found",
				       __FILE__, __LINE__, config[CF_USER].valeur.string);
			}
			exit(1);
		}
		uid = pwd->pw_uid;
		gid = pwd->pw_gid;

		// set default group of user
		if(setgid(gid) == -1){
			logmsg(LOG_ERR, "[%s %i] setgid[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}

		// use all groups assigned to user
		if(initgroups(config[CF_USER].valeur.string, gid) == -1){
			logmsg(LOG_ERR, "[%s %i] initgroups[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}

		// close passwd and groups
		endpwent();
		endgrent();
	}

	// chrooting
	if(config[CF_CHROOT].valeur.string != NULL &&
	   config[CF_CHROOT].valeur.string[0] != 0) {
			  
		// chrooting
		if (chroot(config[CF_CHROOT].valeur.string)){
			logmsg(LOG_ERR, "[%s %i] chroot[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}

		// change current directory
		if (chdir("/")){
			logmsg(LOG_ERR, "[%s %i] chdir[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}
	}

	// change user
	if(config[CF_USER].valeur.string != NULL &&
	   config[CF_USER].valeur.string[0] != 0) { 
		if (setuid(uid) == -1){
			logmsg(LOG_ERR, "[%s %i] setuid[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}
	}

	// create file rights
	umask(config[CF_UMASK].valeur.integer);
}

