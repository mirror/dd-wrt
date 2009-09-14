/*
 *	BIRD -- UNIX Configuration
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "lib/unix.h"
#include <stdio.h>

CF_DECLS

CF_KEYWORDS(LOG, SYSLOG, ALL, DEBUG, TRACE, INFO, REMOTE, WARNING, ERROR, AUTH, FATAL, BUG, STDERR, SOFT)

%type <i> log_mask log_mask_list log_cat
%type <g> log_file
%type <t> cfg_name

CF_GRAMMAR

CF_ADDTO(conf, log_config)

log_config: LOG log_file log_mask ';' {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = $2;
    c->mask = $3;
    add_tail(&new_config->logfiles, &c->n);
  }
 ;

log_file:
   TEXT {
     FILE *f = tracked_fopen(new_config->pool, $1, "a");
     if (!f) cf_error("Unable to open log file `%s': %m", $1);
     $$ = f;
   }
 | SYSLOG { $$ = NULL; }
 | STDERR { $$ = stderr; }
 ;

log_mask:
   ALL { $$ = ~0; }
 | '{' log_mask_list '}' { $$ = $2; }
 ;

log_mask_list:
   log_cat { $$ = 1 << $1; }
 | log_mask_list ',' log_cat { $$ = $1 | (1 << $3); }
 ;

log_cat:
   DEBUG { $$ = L_DEBUG[0]; }
 | TRACE { $$ = L_TRACE[0]; }
 | INFO { $$ = L_INFO[0]; }
 | REMOTE { $$ = L_REMOTE[0]; }
 | WARNING { $$ = L_WARN[0]; }
 | ERROR { $$ = L_ERR[0]; }
 | AUTH { $$ = L_AUTH[0]; }
 | FATAL { $$ = L_FATAL[0]; }
 | BUG { $$ = L_BUG[0]; }
 ;

/* Unix specific commands */

CF_CLI_HELP(CONFIGURE, [soft] [\"<file>\"], [[Reload configuration]])

CF_CLI(CONFIGURE, cfg_name, [\"<file>\"], [[Reload configuration]])
{ cmd_reconfig($2, RECONFIG_HARD); } ;

CF_CLI(CONFIGURE SOFT, cfg_name, [\"<file>\"], [[Reload configuration and ignore changes in filters]])
{ cmd_reconfig($3, RECONFIG_SOFT); } ;

CF_CLI(DOWN,,, [[Shut the daemon down]])
{ cli_msg(7, "Shutdown requested"); order_shutdown(); } ;

cfg_name:
   /* empty */ { $$ = NULL; }
 | TEXT
 ;

CF_CODE

CF_END
