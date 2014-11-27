/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: arpalert.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <fcntl.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "arpalert.h"
#include "loadconfig.h"
#include "log.h"
#include "data.h"
#include "maclist.h"
#include "capture.h"
#include "serveur.h"
#include "alerte.h"
#include "sens.h"
#include "signals.h"
#include "sens_timeouts.h"
#include "loadmodule.h"
#include "func_time.h"
#include "macname.h"

extern int errno;

void die(int);
void dumpmaclist(int);

int dumptime = 0;
int nettoyage = 0;

int main(int argc, char **argv){
	fd_set read_filed_set;
	int selret, max_filed, index;
	void (* check_timeout)(void);
	void (* check_temp)(void);
	struct timeval timeout;
	struct timeval temp_timeout;
	struct timeval cur_timeout;
	struct timeval *tmout;
	void *(* get_next_interupt[6])(struct timeval *tv);
	
	// set flags as not forked
	is_forked = FALSE;
	
	// init current_time
	//current_time = time(NULL);
	gettimeofday(&current_t, NULL);
	
	// read config file
	config_load(argc, argv);

	// log system initialization
	initlog();
	
	// load module alert
	module_load();

	// pcap initialization
	cap_init();

	// daemonize arpalert
	if(config[CF_DAEMON].valeur.integer == TRUE) daemonize();

	// privilege separation and chrooting
	separe();
	
	// set up signals
	signals_init();

	// mac structurs initialization
	data_init();

	// initialize acl checks
	if(sens_init(SENS_LOAD) == -1){
		logmsg(LOG_ERR,
		       "errors in file \"%s\": not reload",
		       config[CF_AUTHFILE].valeur.string);
		exit(1);
	}

	// sens_timeouts initializations
	sens_timeout_init();
	
	// alert
	alerte_init();

	// load vendor database
	macname_init();
	if(macname_load(MACNAME_LOAD) == -1){
		exit(1);
	}

	// load maclist
	maclist_load();

	// init abuse counter
	cap_abus();

	// init sheduler system
	index = 0;

	// check timeout for program lauched
	get_next_interupt[index] = alerte_next;
	index++;

	// check capture management
	get_next_interupt[index] = cap_next;
	index++;

	// check data management
	get_next_interupt[index] = data_next;
	index++;

	// signals
	get_next_interupt[index] = signals_next;
	index++;

	// check timeout for sens_timeout functions
	if(config[CF_UNAUTH_TO_METHOD].valeur.integer == 2){
		get_next_interupt[index] = alerte_next;
		index++;
	}

	// end
	get_next_interupt[index] = NULL;

	// scheduler
	while(TRUE){

		// generate bitfield
		FD_ZERO(&read_filed_set);
		max_filed = cap_gen_bitfield(&read_filed_set);

		// generate timeouts
		cur_timeout.tv_sec = -1;
		check_temp = NULL;
		check_timeout = NULL;
		index = 0;

		while(get_next_interupt[index] != NULL){
			
			// run next interupt
		   check_temp = get_next_interupt[index](&temp_timeout);

			// if timeout is returned
			if(temp_timeout.tv_sec != -1){
				if(cur_timeout.tv_sec != -1){
					if(time_comp(&cur_timeout, &temp_timeout) == BIGEST){
						cur_timeout.tv_sec = temp_timeout.tv_sec;
						cur_timeout.tv_usec = temp_timeout.tv_usec;
						check_timeout = check_temp;
					}
				} else {
					cur_timeout.tv_sec = temp_timeout.tv_sec;
					cur_timeout.tv_usec = temp_timeout.tv_usec;
					check_timeout = check_temp;
				}
			}

			// get next interrupt
			index++;
		}

		// calculate timeout time from the next timeout date
		if(cur_timeout.tv_sec != -1){
		   time_sous(&cur_timeout, &current_t, &timeout);

			// prevent negative timeout
			if(timeout.tv_sec < 0){
				timeout.tv_usec = 0;
				timeout.tv_sec = 0;
			}
			// add 10000µs for prevent premature timeout
			timeout.tv_usec += 10000;
			tmout = &timeout;

		// if no timeout
		} else {
			tmout = NULL;
		}

		// block waiting for next system event or timeout
		selret = select(max_filed + 1, &read_filed_set,
		                NULL, NULL, tmout);

		// maj current hour
		gettimeofday(&current_t, NULL);
	
		// errors:
		#if (__NetBSD__)
		if (selret == -1 && errno != EINTR && errno != EINVAL){
		#else
		if (selret == -1 && errno != EINTR){
		#endif
			logmsg(LOG_ERR, "[%s %i] select[%d]: %s",
			       __FILE__, __LINE__, errno, strerror(errno));
			exit(1);
		}

		// timeouts
		if(selret == 0){
			if(check_timeout != NULL){
				check_timeout();
			}
		}

		// network pcap events
		if(selret > 0){
			cap_sniff(&read_filed_set);
		}
	}

	exit(1);
}

