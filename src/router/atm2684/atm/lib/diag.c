/* diag.c - Diagnostic messages */

/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */


#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "atmd.h"


#define MAX_DIAG_MSG  8200
#define DUMP_LINE_WIDTH 75


typedef struct _component {
    const char *name;
    int verbosity;
    struct _component *next;
} COMPONENT;


static int sev2prio[] = { DIAG_DEBUG,LOG_DEBUG, DIAG_INFO,LOG_INFO,
  DIAG_WARN,LOG_WARNING, DIAG_ERROR,LOG_ERR, DIAG_FATAL,LOG_ALERT,
  -1,LOG_NOTICE };


static const char *app_name = NULL;
static COMPONENT *components = NULL;
static int default_verbosity = DIAG_INFO;
static FILE *log_to = NULL;
static int log_to_initialized = 0;


void set_application(const char *name)
{
    app_name = name;
}


void set_verbosity(const char *component,int level)
{
    COMPONENT *walk;

    if (!component) {
	default_verbosity = level;
	return;
    }
    for (walk = components; walk; walk = walk->next)
	if (!strcmp(walk->name,component)) break;
    if (!walk) {
	walk = alloc_t(COMPONENT);
	walk->name = component;
	walk->next = components;
	components = walk;
    }
    walk->verbosity = level;
}


int get_verbosity(const char *component)
{
    COMPONENT *walk;

    if (!component) {
        return default_verbosity;
    }
    for (walk = components; walk; walk = walk->next)
        if (!strcmp(walk->name,component)) break;
    return walk ? walk->verbosity : default_verbosity;
}


void set_logfile(const char *name)
{
    log_to_initialized = 1;
    if (log_to && log_to != stderr) {
	(void) fclose(log_to);
	log_to = stderr;
    }
    if (!name || !strcmp(name,"stderr")) {
	log_to = stderr;
	return;
    }
    if (!strcmp(name,"syslog")) {
	if (app_name) openlog(app_name,LOG_CONS,LOG_DAEMON);
	log_to = NULL;
    }
    else if (!(log_to = fopen(name,"w"))) {
	    perror(name);
	    log_to = stderr;
	}
}


FILE *get_logfile(void)
{
    if (!log_to_initialized) {
	log_to = stderr;
	log_to_initialized = 1;
    }
    return log_to;
}


void diag_fatal_debug_hook(void); /* GCC insists on a prototype */


void diag_fatal_debug_hook(void)
{
    /*
     * Set a breakpoint here to catch fatal errors before they mess up the
     * stack.
     */
}


void vdiag(const char *component,int severity,const char *fmt,va_list ap)
{
    COMPONENT *walk;
    FILE *to;
    char buffer[MAX_DIAG_MSG+1];
    int i;

    for (walk = components; walk; walk = walk->next)
	if (!strcmp(walk->name,component)) break;
    if (severity > (walk ? walk->verbosity : default_verbosity)) return;
    fflush(stdout);
    to = get_logfile();
    if (!to) {
	for (i = 0; sev2prio[i] == severity || sev2prio[i] == -1; i += 2);
	vsprintf(buffer,fmt,ap);
	syslog(sev2prio[i+1],"%s: %s",component,buffer);
    }
    else {
	if (app_name) fprintf(to,"%s:%s: ",app_name,component);
	else fprintf(to,"%s: ",component);
	vfprintf(to,fmt,ap);
	fputc('\n',to);
	fflush(to);
    }
    if (severity == DIAG_FATAL) {
	diag_fatal_debug_hook();
	fprintf(stderr,"Fatal error - Terminating\n");
	exit(1);
    }
}


void diag(const char *component,int severity,const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    vdiag(component,severity,fmt,ap);
    va_end(ap);
}


void diag_dump(const char *component,int severity,const char *title,
  const unsigned char *data,int len)
{
    char buffer[DUMP_LINE_WIDTH+1];
    char *curr;
    int data_line,left;

    if (title) diag(component,severity,"%s (%d bytes)\n",title,len);
    data_line = DUMP_LINE_WIDTH-(app_name ? strlen(app_name)+1 : 0)-
      strlen(component)-3;
    while (len) {
	left = data_line;
	curr = buffer;
	while (len && left >= 3) {
	    sprintf(curr," %02x",*data++);
	    len--;
	    curr += 3;
	    left -= 3;
	}
	diag(component,severity,"%s ",buffer);
    }
}
