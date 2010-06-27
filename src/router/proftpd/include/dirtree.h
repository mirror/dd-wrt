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

/*
 * Configuration structure, server, command and associated prototypes.
 *
 * $Id: dirtree.h,v 1.74 2009/03/24 06:23:27 castaglia Exp $
 */

#ifndef PR_DIRTREE_H
#define PR_DIRTREE_H

#include "pool.h"
#include "sets.h"
#include "table.h"

typedef struct config_struc config_rec;

struct conn_struc;

typedef struct server_struc {
  struct server_struc *next, *prev;

  pool *pool;			/* Memory pool for this server */
  xaset_t *set;			/* Set holding all servers */

  /* The label/name for this server configuration. */
  const char *ServerName;

  /* The address for this server configuration. */
  const char *ServerAddress;

  /* The fully qualified domain name for this server configuration. */
  const char *ServerFQDN;

  /* Port number to which to listen. A value of zero disables the server_rec.
   */
  unsigned int ServerPort;

  /* TCP settings: max segment size, receive/send buffer sizes.
   */
  int tcp_mss_len;

  int tcp_rcvbuf_len;
  unsigned char tcp_rcvbuf_override;

  int tcp_sndbuf_len;
  unsigned char tcp_sndbuf_override;

  /* Administrator name */
  char *ServerAdmin;

  /* Internal address of this server */
  pr_netaddr_t *addr;

  /* The listener for this server.  Note that this listener, and that
   * pointed to by ipbind->ib_listener (where ipbind->ib_server points to
   * this server_rec) are the same.  Ideally, we'd only want one pointer to
   * the listener around, and avoid the duplication.  To do this would
   * require further structural changes.
   */
  struct conn_struc *listen;

  /* Configuration details */
  xaset_t *conf;
  int config_type;

  /* Internal server ID, automatically assigned */
  unsigned int sid;

} server_rec;

typedef struct cmd_struc {
  pool *pool;
  server_rec *server;
  config_rec *config;
  pool *tmp_pool;		/* Temporary pool which only exists
				 * while the cmd's handler is running
				 */
  int argc;

  char *arg;			/* entire argument (excluding command) */
  char **argv;
  char *group;			/* Command grouping */

  int  class;			/* The command class */
  int  stash_index;		/* hack to speed up symbol hashing in modules.c */
  pr_table_t *notes;		/* Private data for passing/retaining between handlers */
} cmd_rec;

struct config_struc {
  struct config_struc *next,*prev;

  int config_type;
  unsigned int config_id;

  pool *pool;			/* memory pool for this object */
  xaset_t *set;			/* The set we are stored in */
  char *name;
  int argc;
  void **argv;

  long flags;			/* Flags */

  server_rec *server;		/* Server this config element is attached to */
  config_rec *parent;		/* Our parent configuration record */
  xaset_t *subset;		/* Sub-configuration */
};

#define CONF_ROOT		(1 << 0) /* No conf record */
#define CONF_DIR		(1 << 1) /* Per-Dir configuration */
#define CONF_ANON		(1 << 2) /* Anon. FTP configuration */
#define CONF_LIMIT		(1 << 3) /* Limits commands available */
#define CONF_VIRTUAL		(1 << 4) /* Virtual host */
#define CONF_DYNDIR		(1 << 5) /* .ftpaccess file */
#define CONF_GLOBAL		(1 << 6) /* "Global" context (applies to main server and ALL virtualhosts */
#define CONF_CLASS		(1 << 7) /* Class context */
#define CONF_NAMED		(1 << 8) /* Named virtual host */
#define CONF_USERDATA		(1 << 14) /* Runtime user data */
#define CONF_PARAM		(1 << 15) /* config/args pair */

/* config_rec flags */
#define CF_MERGEDOWN		(1 << 0) /* Merge option down */
#define CF_MERGEDOWN_MULTI	(1 << 1) /* Merge down, allowing multiple instances */
#define CF_DYNAMIC		(1 << 2) /* Dynamically added entry */
#define CF_DEFER		(1 << 3) /* Defer hashing until authentication */
#define CF_SILENT		(1 << 4) /* Do not print a config dump when merging */

/* Operation codes for dir_* funcs */
#define OP_HIDE			1	/* Op for hiding dirs/files */
#define OP_COMMAND		2	/* Command operation */

/* For the Order directive */
#define ORDER_ALLOWDENY		0
#define ORDER_DENYALLOW		1

/* The following macro determines the "highest" level available for
 * configuration directives.  If a current dir_config is available, it's
 * subset is used, otherwise anon config or main server
 */

#define CURRENT_CONF		(session.dir_config ? session.dir_config->subset \
				 : (session.anon_config ? session.anon_config->subset \
                                    : main_server->conf))
#define TOPLEVEL_CONF		(session.anon_config ? session.anon_config->subset : main_server->conf)

extern server_rec		*main_server;
extern int			tcpBackLog;
extern int			SocketBindTight;
extern char			ServerType;
extern int			ServerMaxInstances;
extern int			ServerUseReverseDNS;

/* These macros are used to help handle configuration in modules */
#define CONF_ERROR(x, s)	return PR_ERROR_MSG((x),NULL,pstrcat((x)->tmp_pool, \
				(x)->argv[0],": ",(s),NULL));

#define CHECK_ARGS(x, n)	if((x)->argc-1 < n) \
				CONF_ERROR(x,"missing arguments")

#define CHECK_VARARGS(x, n, m)	if((x)->argc - 1 < n || (x)->argc - 1 > m) \
				CONF_ERROR(x,"missing arguments")

#define CHECK_HASARGS(x, n)	((x)->argc - 1) == n

#define CHECK_CONF(x,p)		if (!check_context((x),(p))) \
				CONF_ERROR((x), \
				pstrcat((x)->tmp_pool,"directive not allowed in ", \
				get_context_name((x)), \
				" context",NULL))

#define CHECK_CMD_ARGS(x, n)	\
  if ((x)->argc != (n)) { \
    pr_response_add_err(R_501, _("Invalid number of arguments")); \
    return PR_ERROR((x)); \
  }

#define CHECK_CMD_MIN_ARGS(x, n)	\
  if ((x)->argc < (n)) { \
    pr_response_add_err(R_501, _("Invalid number of arguments")); \
    return PR_ERROR((x)); \
  }

/* Prototypes */

/* KLUDGE: disable umask() for not G_WRITE operations.  Config/
 * Directory walking code will be completely redesigned in 1.3,
 * this is only necessary for perfomance reasons in 1.1/1.2
 */
void kludge_disable_umask(void);
void kludge_enable_umask(void);

int pr_define_add(const char *, int);
unsigned char pr_define_exists(const char *);

void init_config(void);
int fixup_servers(xaset_t *list);
int parse_config_path(pool *, const char *);
config_rec *add_config_set(xaset_t **, const char *);
config_rec *add_config(server_rec *, const char *);
config_rec *add_config_param(const char *, int, ...);
config_rec *add_config_param_str(const char *, int, ...);
config_rec *add_config_param_set(xaset_t **, const char *, int, ...);
config_rec *pr_conf_add_server_config_param_str(server_rec *, const char *,
  int, ...);
config_rec *find_config_next(config_rec *, config_rec *, int,
  const char *, int);
config_rec *find_config(xaset_t *, int, const char *, int);
void find_config_set_top(config_rec *);
int remove_config(xaset_t *, const char *, int);

/* Returns the assigned ID for the provided directive name, or zero
 * if no ID mapping was found.
 */
unsigned int pr_config_get_id(const char *name);

/* Returns the buffer size to use for data transfers.
 */
int pr_config_get_xfer_bufsz(void);

/* Assigns a unique ID for the given configuration directive.  The
 * mapping of directive to ID is stored in a lookup table, so that
 * searching of the config database by directive name can be done using
 * ID comparisons rather than string comparisons.
 *
 * Returns the ID assigned for the given directive, or zero if there was an
 * error.
 */
unsigned int pr_config_set_id(const char *name);

void *get_param_ptr(xaset_t *, const char *, int);
void *get_param_ptr_next(const char *, int);
xaset_t *get_dir_ctxt(pool *, char *);

config_rec *dir_match_path(pool *, char *);
void build_dyn_config(pool *, const char *, struct stat *, unsigned char);
unsigned char dir_hide_file(const char *);
int dir_check_full(pool *, cmd_rec *, const char *, const char *, int *);
int dir_check_limits(cmd_rec *, config_rec *, const char *, int);
int dir_check(pool *, cmd_rec *, const char *, const char *, int *);
int dir_check_canon(pool *, cmd_rec *, const char *, const char *, int *);
int is_dotdir(const char *);
int is_fnmatch(const char *);
int login_check_limits(xaset_t *, int, int, int *);
char *path_subst_uservar(pool *, char **);
void resolve_anonymous_dirs(xaset_t *);
void resolve_deferred_dirs(server_rec *);
void fixup_dirs(server_rec *, int);
unsigned char check_context(cmd_rec *, int);
char *get_context_name(cmd_rec *);
int get_boolean(cmd_rec *, int);
char *get_full_cmd(cmd_rec *);

void pr_config_dump(void (*)(const char *, ...), xaset_t *, char *);

#ifdef PR_USE_DEVEL
void pr_dirs_dump(void (*)(const char *, ...), xaset_t *, char *);
#endif /* PR_USE_DEVEL */

#endif /* PR_DIRTREE_H */
