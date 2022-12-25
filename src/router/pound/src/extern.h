/*
 * Global variables needed by everybody
 */

extern char *progname;          /* program name */

extern char *user,		/* user to run as */
  *group,			/* group to run as */
  *root_jail,			/* directory to chroot to */
  *pid_name;			/* file to record pid in */


extern unsigned worker_min_count; /* min. number of worker threads */
extern unsigned worker_max_count; /* max. number of worker threads */
extern unsigned worker_idle_timeout;

extern unsigned grace;		/* grace period before shutdown */

extern int anonymise;		/* anonymise client address */
extern unsigned alive_to;	/* check interval for resurrection */
extern int daemonize;		/* run as daemon */
extern int enable_supervisor;   /* run supervisor process */
extern int log_facility;	/* log facility to use */
extern int print_log;		/* print log messages to stdout/stderr */

extern regex_t HEADER,		/* Allowed header */
  CONN_UPGRD,			/* upgrade in connection header */
  CHUNK_HEAD,			/* chunk header line */
  RESP_SKIP,			/* responses for which we skip response */
  RESP_IGN,			/* responses for which we ignore content */
  LOCATION,			/* the host we are redirected to */
  AUTHORIZATION;		/* the Authorisation header */

#ifndef  SOL_TCP
/* for systems without the definition */
extern int SOL_TCP;
#endif

extern LISTENER_HEAD listeners;	/* all available listeners */
extern SERVICE_HEAD services;	/* global services (if any) */

enum
  {
    FEATURE_DNS
  };

int feature_is_set (int f);
