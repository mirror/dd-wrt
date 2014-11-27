/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: alerte.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "arpalert.h"
#include "alerte.h"
#include "log.h"
#include "loadconfig.h"
#include "serveur.h"
#include "func_time.h"

// alert levels
const char *alert[] = {
	 "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19"
};

char cmd_exec[2048];

extern int errno;

struct t_pid {
	int pid;
	struct timeval time;
	struct t_pid *next;
	struct t_pid *prev;
};

// used for allocate pid structur memory
struct t_pid *pid_alloc;

// unused base
struct t_pid unused_pid;

// used base
struct t_pid used_pid;

// pid list initialization 
void alerte_init(void){
	int counter;
	struct t_pid *assign;

	// init used pid chain
	used_pid.next = &used_pid;
	used_pid.prev = &used_pid;

	// if the script is not specified, quit function
	if(config[CF_ACTION].valeur.string == NULL ||
	   config[CF_ACTION].valeur.string[0] == 0){
		return;
	}

	// memory allocation for pid
	pid_alloc = (struct t_pid *) malloc(sizeof(struct t_pid) *
	            config[CF_MAXTH].valeur.integer);
	if(pid_alloc == NULL){
		logmsg(LOG_ERR, "[%s %i] malloc[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}

	// chain all pid in unused base
	assign = &unused_pid;
	counter = 0;
	while(counter < config[CF_MAXTH].valeur.integer){
		assign->next = &pid_alloc[counter];
		assign = assign->next;
		counter++;
	}
	assign->next = NULL;
}

// add a pid to list 
void addpid(int pid){
	struct t_pid *assign;

	// check if have a free process memory
	if(unused_pid.next == NULL){
		logmsg(LOG_ERR, "[%s %d] Process limit exceeded",
		       __FILE__, __LINE__);
		exit(1);
	}

	// set values
	assign = unused_pid.next;
	assign->pid = pid;
	assign->time.tv_sec = current_t.tv_sec +
	                      config[CF_TIMEOUT].valeur.integer;
	assign->time.tv_usec = current_t.tv_usec;

	// delete from the unused list
	unused_pid.next = unused_pid.next->next;
	
	// add at the end of chain
	assign->next = &used_pid;
	assign->prev = used_pid.prev;
	used_pid.prev->next = assign;
	used_pid.prev = assign;

	#ifdef DEBUG
	logmsg(LOG_DEBUG,
	       "[%s %i %s] Add pid %i",
	       __FILE__, __LINE__, __FUNCTION__,
	       assign->pid);
	#endif
}

// delete pid
void delpid(int pid){
	struct t_pid *assign;

	// if no current recorded pid
	if(used_pid.next == &used_pid) {
		return;
	}

	// find pid in pid chain
	assign = &used_pid;
	while(assign->pid != pid) {
		if(assign->next == &used_pid) {
			return;
		}
		assign = assign->next;
	}

	// delete pid from used list
	assign->next->prev = assign->prev;
	assign->prev->next = assign->next;

	// add pid to unused list
	assign->next = unused_pid.next;
	unused_pid.next = assign;
}

void alerte_kill_pid(void){
	int pid;

	#ifdef DEBUG
	logmsg(LOG_DEBUG,
	       "[%s %i %s] starting",
	       __FILE__, __LINE__, __FUNCTION__); 
	#endif

	while(TRUE){
		pid = waitpid(0, NULL, WNOHANG);
		
		// exit if no more child ended
		if(pid == 0 || ( pid == -1 && errno == ECHILD ) ){
			break;
		}
		
		// check error
		if(pid == -1 && errno != ECHILD){
			logmsg(LOG_ERR, "[%s %d] waitpid[%d]: %s",
				__FILE__, __LINE__, errno, strerror(errno));
			break;
		}
	
		#ifdef DEBUG
		logmsg(LOG_DEBUG,
		       "[%s %i %s] pid [%i] ended",
		       __FILE__, __LINE__, __FUNCTION__, pid); 
		#endif
		delpid(pid);
	}
}

// check validity of pids
void alerte_check(void){
	int return_code;
	int status;
	struct t_pid *check;
	struct t_pid *temp_check;

	#ifdef DEBUG
	logmsg(LOG_DEBUG,
	       "[%s %d %s] start cleanning processes",
	       __FILE__, __LINE__, __FUNCTION__);
	#endif

	// if no current recorded pid
	if(used_pid.next == &used_pid){
		#ifdef DEBUG
		logmsg(LOG_DEBUG, "[%s %i %s] no pid in pid list",
		       __FILE__, __LINE__, __FUNCTION__);
		#endif
		return;
	}

	// check all process
	check = used_pid.next;
	while(check != &used_pid){

		// record next occurance (the actual pointer maybe deleted)
		temp_check = check->next;
	
		// look if process's running
		return_code = waitpid(check->pid, &status, WNOHANG);

		// if time exceed
		if(return_code == 0 &&
		   time_comp(&current_t, &(check->time)) == BIGEST){
			logmsg(LOG_ERR, "kill pid %i: running time exceeded",
			       check->pid);

			// kill it
			if(kill(check->pid, 9) < 0){
				logmsg(LOG_ERR, "[%s %i] kill[%d]: %s",
				       __FILE__, __LINE__, errno, strerror(errno));
			}
		} 

		// if the process is stopped
		// else if(return_code == -1){
		else {
			#ifdef DEBUG
			logmsg(LOG_DEBUG, "[%s %i %s] pid %i is ended, removing "
			       "from check list",
			       __FILE__, __LINE__, __FUNCTION__,
			       check->pid);
			#endif

			// delete pid from list
			delpid(check->pid);
		}
		
		check = temp_check;
	}
}

// send an alert
void alerte_script(char *mac, char *ip, int alert_level, char *parm_supp,
                   char *interface, char *vendor){
	int return_pid;
	int return_code;
		  
	// if the script is not specified, quit function
	if(config[CF_ACTION].valeur.string == NULL ||
	   config[CF_ACTION].valeur.string[0] == 0){
		return;
	}

	if(unused_pid.next == NULL){
		logmsg(LOG_ERR, "Exceed maximun process currently running");
		return;
	}

	#ifdef DEBUG	
	logmsg(LOG_DEBUG, "[%s %i %s] Launch alert script [%s]", 			
          __FILE__, __LINE__, __FUNCTION__,
	       config[CF_ACTION].valeur.string);
	#endif

	return_pid = fork();
	if(return_pid == -1){
		logmsg(LOG_ERR, "[%s %i] fork[%d]: %s",
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	if(return_pid > 0){
		addpid(return_pid);
		return;
	}

	if(config[CF_ALERT_VENDOR].valeur.integer == TRUE){
		return_code = execlp(config[CF_ACTION].valeur.string,
		                     config[CF_ACTION].valeur.string,
		                     mac, ip, parm_supp, interface,
		                     alert[alert_level], vendor,
		                     (char*)0);
	}

	else {
		return_code = execlp(config[CF_ACTION].valeur.string,
		                     config[CF_ACTION].valeur.string,
		                     mac, ip, parm_supp, interface,
		                     alert[alert_level], (char*)0);
	}
	if(return_code < 0){
		logmsg(LOG_ERR, "[%s %i] execlp[%d]: %s"
		       __FILE__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	exit(0);
}

// return the next active timeout
void *alerte_next(struct timeval *tv){
	struct t_pid *check;

	// if no pid running return NULL
	if(used_pid.next == &used_pid){
		tv->tv_sec = -1;
		return NULL;
	}

	check = used_pid.next;
	tv->tv_sec = check->time.tv_sec;
	tv->tv_usec = check->time.tv_usec;
	return alerte_check;
}

