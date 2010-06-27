/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2009 The ProFTPD Project team
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

/* ProFTPD module definitions.
 *
 * $Id: modules.h,v 1.53 2009/02/15 00:27:34 castaglia Exp $
 */

#ifndef PR_MODULES_H
#define PR_MODULES_H

typedef struct module_struc	module;
typedef struct modret_struc	modret_t;

struct modret_struc {
  module *mr_handler_module;		/* which module handled this? */
  int    mr_error;			/* !0 if error */
  char   *mr_numeric;			/* numeric error code */
  char   *mr_message;			/* text message */
  void	 *data;				/* add'l data -- undefined */
};

/* The following macros are for creating basic modret_t, and can
 * only be used inside of module handlers
 */

#define MODRET				static modret_t*
#define PR_HANDLED(cmd)			mod_create_ret((cmd),\
					0,NULL,NULL)
#define PR_DECLINED(cmd)		(modret_t*)NULL
#define PR_ERROR(cmd)			mod_create_ret((cmd),\
					1,NULL,NULL)
#define PR_ERROR_MSG(cmd,n,m)		mod_create_ret((cmd),\
					1,(n),(m))
#define PR_ERROR_INT(cmd,n)		mod_create_error((cmd),(n))

#define MODRET_ISDECLINED(x)		((x) == NULL)
#define MODRET_ISHANDLED(x)		((x) && !(x)->mr_error)
#define MODRET_ISERROR(x)		((x) && (x)->mr_error)
#define MODRET_HASNUM(x)		((x) && (x)->mr_numeric)
#define MODRET_HASMSG(x)		((x) && (x)->mr_message)
#define MODRET_ERROR(x)			((x) ? (x)->mr_error : 0)
#define MODRET_ERRNUM(x)		((x) ? (x)->mr_numeric : NULL)
#define MODRET_ERRMSG(x)		((x) ? (x)->mr_message : NULL)
#define MODRET_HASDATA(x)		((x) ? ((x)->data ? TRUE : FALSE) : FALSE)

typedef struct {
  char *directive;
  modret_t *(*handler)(cmd_rec*);

  module *m;				/* Reference to owning module
					 * set when module is initialized
					 */

} conftable;

/* classes for command table */

#define CL_NONE				0x0
#define CL_AUTH				(1 << 0) /* USER, PASS */
#define CL_INFO				(1 << 1) /* Informational commands (PWD, SYST, etc) */
#define CL_DIRS				(1 << 2) /* Directory commands (LIST, NLST, CWD, etc) */
#define CL_READ				(1 << 3) /* File reading commands (RETR) */
#define CL_WRITE			(1 << 4) /* Writing commands (STOR, MKD, etc) */
#define CL_MISC				(1 << 5) /* Miscellaneous (RNFR/RNTO, SITE, etc) */
#define CL_SEC				(1 << 6) /* RFC2228 Security commands */

#define CL_ALL				(CL_AUTH|CL_INFO|CL_DIRS|CL_READ| \
					CL_WRITE|CL_MISC|CL_SEC)

/* Command handler types for command table */
#define PRE_CMD				1
#define CMD				2
#define POST_CMD			3
#define POST_CMD_ERR			4
#define LOG_CMD				5
#define LOG_CMD_ERR			6
#define HOOK				7

typedef struct {

  /* See above for cmd types. */
  unsigned char cmd_type;
  char *command;

  /* Command group. */
  char *group;
  modret_t *(*handler)(cmd_rec*);

  /* Does this command require authentication? */
  unsigned char requires_auth;

  /* Can this command be issued during a transfer? (Now obsolete) */
  unsigned char interrupt_xfer;

  int class;
  module *m;
} cmdtable;

typedef struct {
  int auth_flags;			/* future use */
  char *name;
  modret_t *(*handler)(cmd_rec*);

  module *m;
} authtable;

#define PR_AUTH_FL_REQUIRED		0x00001

struct module_struc {
  module *next,*prev;

  int api_version;			/* API version _not_ module version */
  char *name;				/* Module name */

  conftable *conftable;			/* Configuration directive table */
  cmdtable *cmdtable;			/* Command table */
  authtable *authtable; 		/* Authentication handler table */

  int (*init)(void); 			/* Module initialization */
  int (*sess_init)(void);		/* Session initialization */

  char *module_version;			/* Module version */
  void *handle;				/* Module handle */

  /* Internal use; high number == higher priority. */
  int priority;
};

#define ANY_MODULE			((module*)0xffffffff)

/* Prototypes */

unsigned char command_exists(char *);
int modules_init(void);
void modules_list(int);
#define PR_MODULES_LIST_FL_SHOW_VERSION		0x00001
#define PR_MODULES_LIST_FL_SHOW_STATIC		0x00002

int modules_session_init(void);

unsigned char pr_module_exists(const char *);
module *pr_module_get(const char *);
int pr_module_load(module *m);
int pr_module_unload(module *m);

modret_t *pr_module_call(module *, modret_t *(*)(cmd_rec *), cmd_rec *);

/* Symbol table hash ("stash") support. */
typedef enum {
  PR_SYM_CONF = 1,
  PR_SYM_CMD,
  PR_SYM_AUTH,
  PR_SYM_HOOK
} pr_stash_type_t;

int init_stash(void);
int pr_stash_add_symbol(pr_stash_type_t, void *);
void *pr_stash_get_symbol(pr_stash_type_t, const char *, void *, int *);
int pr_stash_remove_symbol(pr_stash_type_t, const char *, module *);

/* This function is in main.c, but is prototyped here */
void set_auth_check(int (*ck)(cmd_rec *));

/* This callback is defined/stored in src/main.c */
extern int (*cmd_auth_chk)(cmd_rec *);

/* For use from inside module handler functions */
modret_t *mod_create_ret(cmd_rec *, unsigned char, char *, char *);
modret_t *mod_create_error(cmd_rec *, int);
modret_t *mod_create_data(cmd_rec *, void *);

/* Implemented in mod_core.c */
int core_chgrp(cmd_rec *, char *, uid_t, gid_t);
int core_chmod(cmd_rec *, char *, mode_t);

#endif /* PR_MODULES_H */
