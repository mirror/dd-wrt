/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2008 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Notify the user when a given file was last changed
 *
 * Configuration:
 *   DisplayReadme <file-or-pattern>
 *
 * "DisplayReadme Readme" will tell the user when "Readme" on the current 
 * working directory was last changed. When cwd is changed (cd, cdup, ...)
 * it'll seach for Readme agin in it and also display it's last changing dates.
 * if found.
 */

#include "conf.h"

/* Flood: Nov 1, 1998
 *
 * The original logic was slightly off, using find_config in recursive mode
 * starting from the server root, which caused all DisplayReadme entries
 * to be evaluated, even if not actually part of the "closest" context
 * (i.e. picking up ALL Anonymous blocks)
 */

static void add_readme_response(pool *p, const char *file) {
  int days;
  time_t clock;
  
  struct stat buf;
  struct tm *tp;
  
  char *tptr;
  char ctime_str[28] = {'\0'};
  
  if (pr_fsio_stat(file, &buf) == 0) {
    (void) time(&clock);

    tp = pr_gmtime(p, &clock);
    days = (int)(365.25 * tp->tm_year) + tp->tm_yday;

    tp = pr_gmtime(p, &buf.st_mtime);
    days -= (int)(365.25 * tp->tm_year) + tp->tm_yday;

    memset(ctime_str, '\0', sizeof(ctime_str));
    snprintf(ctime_str, sizeof(ctime_str), "%.26s", ctime(&buf.st_mtime));
    
    if ((tptr = strchr(ctime_str, '\n')) != NULL) {
      *tptr = '\0';
    }
    
    pr_response_add(R_DUP, _("Please read the file %s"), file);
    pr_response_add(R_DUP, _("   it was last modified on %.26s - %i %s ago"),
		 ctime_str, days, days == 1 ? _("day") : _("days"));
  }
}

static void add_pattern_response(pool *p, const char *pattern) {
  glob_t g;
  int    a;
  char **path;
  
  if (!(a = pr_fs_glob(pattern, 0, NULL, &g))) {
    path = g.gl_pathv;
    while (path && *path) {
      add_readme_response(p, *path);
      path++;
    }
  } else if (a == GLOB_NOSPACE)
    pr_response_add(R_226, _("Out of memory during globbing of %s"), pattern);

  else if (a == GLOB_ABORTED)
    pr_response_add(R_226, _("Read error during globbing of %s"), pattern);

  else if (a != GLOB_NOMATCH)
    pr_response_add(R_226, _("Unknown error during globbing of %s"), pattern);
  
  pr_fs_globfree(&g);
}

MODRET show_readme(cmd_rec *cmd) {
  config_rec *c;
  char *file;
  
  c = find_config(CURRENT_CONF, CONF_PARAM, "DisplayReadme", FALSE);
  while (c) {
    file = c->argv[0];
    
    pr_log_debug(DEBUG5, "Checking for display pattern %s", file);
    add_pattern_response(cmd->tmp_pool, file);
    
    c = find_config_next(c, c->next, CONF_PARAM, "DisplayReadme",FALSE);
  }
  
  /* Originally this returned HANDLED, which was incorrect, and
   * could cause other POST_CMD handlers to not run
   */
  return PR_DECLINED(cmd);
}

MODRET readme_add_entry(cmd_rec *cmd) {
  config_rec *c;
  
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_ANON|CONF_GLOBAL);
  
  if (cmd->argc != 2) {
    CONF_ERROR(cmd, "syntax: DisplayReadme <filename-or-pattern>");
  }
  
  c = add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;
  
  pr_log_debug(DEBUG5, "Added pattern %s to readme list", cmd->argv[1]);
  return PR_HANDLED(cmd);
}

static conftable readme_config[] = {
  { "DisplayReadme",		readme_add_entry,	},
  { NULL }
};

static cmdtable readme_commands[] = {
	{ POST_CMD,	C_CWD,	G_NONE,	show_readme,	FALSE,	FALSE },
	{ POST_CMD,	C_CDUP,	G_NONE,	show_readme,	FALSE,	FALSE },
	{ POST_CMD,	C_XCWD,	G_NONE,	show_readme,	FALSE,	FALSE },
	{ POST_CMD,	C_XCUP,	G_NONE,	show_readme,	FALSE,	FALSE },

	{ POST_CMD,	C_PASS,	G_NONE, show_readme,	FALSE,	FALSE },

	{ 0,		NULL }
};

module readme_module = {
	NULL, NULL,		/* Always NULL */
	0x20,			/* API Version 2.0 */
	"readme",
	readme_config,		/* configuration table */
	readme_commands,	/* command table is for local use only */
	NULL,			/* No authentication handlers */
	NULL,			/* Initialization function */
	NULL			/* Post-fork "child mode" init */
};

