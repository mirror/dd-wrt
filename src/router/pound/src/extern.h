/*
 * Global variable definitions for pound.
 *
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2025 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */

extern char const *progname;          /* program name */
extern char *syslog_tag;        /* syslog tag */
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
extern int print_log;           /* print log messages to stdout/stderr during
				   startup */
extern int enable_backend_stats;

extern GENPAT HEADER,	/* Allowed header */
  CONN_UPGRD,			/* upgrade in connection header */
  LOCATION;			/* the host we are redirected to */

#define DEFAULT_FORWARDED_HEADER "X-Forwarded-For"
extern char *forwarded_header;  /* "forwarded" header name */
extern ACL *trusted_ips;        /* Trusted IP addresses */

#ifndef  SOL_TCP
/* for systems without the definition */
extern int SOL_TCP;
#endif

extern LISTENER_HEAD listeners;	/* all available listeners */
extern SERVICE_HEAD services;	/* global services (if any) */

extern pthread_mutexattr_t mutex_attr_recursive;
extern pthread_attr_t thread_attr_detached;

enum
  {
    FEATURE_DNS,
    FEATURE_INCLUDE_DIR,
    FEATURE_WARN_DEPRECATED
  };

int feature_is_set (int f);

extern unsigned watcher_ttl;

static inline int
parser_loop (CFGPARSER_TABLE *ptab,
	     void *call_data, void *section_data,
	     struct locus_range *retrange)
{
  return cfgparser_loop (ptab, call_data, section_data,
			 feature_is_set (FEATURE_WARN_DEPRECATED)
			   ? DEPREC_WARN : DEPREC_OK,
			 retrange);
}
