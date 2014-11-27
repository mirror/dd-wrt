/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: log.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef USE_SYSLOG
#include <syslog.h>
#endif

#include "arpalert.h"
#include "log.h"
#include "loadconfig.h"

extern int errno;

FILE *lf = NULL;
const char *mois[12] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

int syslog_initialized = FALSE;
int file_initialized = FALSE;

// used for compose log
#define LOG_MAX_SIZE 512
char log_msg[LOG_MAX_SIZE];

void initlog(void){
	#ifdef USE_SYSLOG
	if(config[CF_USESYSLOG].valeur.integer == TRUE){
		openlog("arpalert", LOG_CONS, LOG_DAEMON);
		syslog_initialized = TRUE;
	}
	#endif
	if(config[CF_LOGFILE].valeur.string != NULL &&
	   config[CF_LOGFILE].valeur.string[0] != 0){
		lf = fopen(config[CF_LOGFILE].valeur.string, "a");
		if(lf == NULL){
			logmsg(LOG_ERR, "[%s %d] fopen[%d] (%s): %s",
			       __FILE__, __LINE__,
			       errno, config[CF_LOGFILE].valeur.string,
			       strerror(errno));
			exit(1);
		}
	}
	file_initialized = TRUE;
}

void logfile_reload(void) {
	int ret_code;

	// check if logfile is used
	if(config[CF_LOGFILE].valeur.string == NULL ||
	   config[CF_LOGFILE].valeur.string[0] == '\0') {
		return;
	}

	// if opened, close
	if (lf != NULL) {
		ret_code = fclose(lf);
		if (ret_code != 0) {
			logmsg(LOG_ERR, "[%s %d] fclose(%s): %s",
			       __FILE__, __LINE__,
			       config[CF_LOGFILE].valeur.string,
			       strerror(errno));
		}
	}

	// open logs
	lf = fopen(config[CF_LOGFILE].valeur.string, "a");
	if(lf == NULL){
		logmsg(LOG_ERR, "[%s %d] fopen[%d] (%s): %s",
		       __FILE__, __LINE__,
		       errno, config[CF_LOGFILE].valeur.string,
		       strerror(errno));
	}
}

void logmsg(int priority, const char *fmt, ...){
	va_list ap;
	struct tm *tm;

	// check if I do log this priority
	if(priority > config[CF_LOGLEVEL].valeur.integer){
		return;
	}

	//get current time
	tm = localtime((time_t *)(&current_t.tv_sec));

	va_start(ap, fmt);
	vsnprintf(log_msg, LOG_MAX_SIZE, fmt, ap);
	va_end(ap);

	#ifdef USE_SYSLOG
	if(config[CF_USESYSLOG].valeur.integer == TRUE &&
	   syslog_initialized == TRUE){
		syslog(priority, log_msg); 
	}
	#endif

	if(config[CF_LOGFILE].valeur.string != NULL &&
	   config[CF_LOGFILE].valeur.string[0] != 0 &&
	   file_initialized == TRUE){
		fprintf(lf, "%s % 2d %02d:%02d:%02d arpalert: %s\n",
		        mois[tm->tm_mon],
		        tm->tm_mday,
		        tm->tm_hour,
		        tm->tm_min,
		        tm->tm_sec, 
		        //for year: tm->tm_year+1900,
		        log_msg);
		fflush(lf);
	}

	if(is_forked == FALSE){
		printf("%s % 2d %02d:%02d:%02d arpalert: %s\n", 
		        mois[tm->tm_mon],
		        tm->tm_mday,
		        tm->tm_hour,
		        tm->tm_min,
		        tm->tm_sec, 
		        log_msg);
	}
}

