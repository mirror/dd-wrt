/*
 *	BIRD Internet Routing Daemon -- Configuration File Handling
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_CONF_H_
#define _BIRD_CONF_H_

#include "lib/resource.h"
#include "lib/timer.h"


/* Configuration structure */

struct config {
  pool *pool;				/* Pool the configuration is stored in */
  linpool *mem;				/* Linear pool containing configuration data */
  list protos;				/* Configured protocol instances (struct proto_config) */
  list tables;				/* Configured routing tables (struct rtable_config) */
  list roa_tables;			/* Configured ROA tables (struct roa_table_config) */
  list logfiles;			/* Configured log fils (sysdep) */

  int mrtdump_file;			/* Configured MRTDump file (sysdep, fd in unix) */
  char *syslog_name;			/* Name used for syslog (NULL -> no syslog) */
  struct rtable_config *master_rtc;	/* Configuration of master routing table */
  struct iface_patt *router_id_from;	/* Configured list of router ID iface patterns */

  u32 router_id;			/* Our Router ID */
  ip_addr listen_bgp_addr;		/* Listening BGP socket should use this address */
  unsigned listen_bgp_port;		/* Listening BGP socket should use this port (0 is default) */
  u32 listen_bgp_flags;			/* Listening BGP socket should use these flags */
  unsigned proto_default_debug;		/* Default protocol debug mask */
  unsigned proto_default_mrtdump;	/* Default protocol mrtdump mask */
  struct timeformat tf_route;		/* Time format for 'show route' */
  struct timeformat tf_proto;		/* Time format for 'show protocol' */
  struct timeformat tf_log;		/* Time format for the logfile */
  struct timeformat tf_base;		/* Time format for other purposes */

  int cli_debug;			/* Tracing of CLI connections and commands */
  char *err_msg;			/* Parser error message */
  int err_lino;				/* Line containing error */
  char *err_file_name;			/* File name containing error */
  char *file_name;			/* Name of main configuration file */
  int file_fd;				/* File descriptor of main configuration file */
  struct symbol **sym_hash;		/* Lexer: symbol hash table */
  struct symbol **sym_fallback;		/* Lexer: fallback symbol hash table */
  int obstacle_count;			/* Number of items blocking freeing of this config */
  int shutdown;				/* This is a pseudo-config for daemon shutdown */
  bird_clock_t load_time;		/* When we've got this configuration */
};

/* Please don't use these variables in protocols. Use proto_config->global instead. */
extern struct config *config;		/* Currently active configuration */
extern struct config *new_config;	/* Configuration being parsed */

struct config *config_alloc(byte *name);
int config_parse(struct config *);
int cli_parse(struct config *);
void config_free(struct config *);
int config_commit(struct config *, int type, int timeout);
int config_confirm(void);
int config_undo(void);
void config_init(void);
void cf_error(char *msg, ...) NORET;
void config_add_obstacle(struct config *);
void config_del_obstacle(struct config *);
void order_shutdown(void);

#define RECONFIG_NONE	0
#define RECONFIG_HARD	1
#define RECONFIG_SOFT	2
#define RECONFIG_UNDO	3

#define CONF_DONE	0
#define CONF_PROGRESS	1
#define CONF_QUEUED	2
#define CONF_UNQUEUED	3
#define CONF_CONFIRM	4
#define CONF_SHUTDOWN	-1
#define CONF_NOTHING	-2


/* Pools */

extern linpool *cfg_mem;

#define cfg_alloc(size) lp_alloc(cfg_mem, size)
#define cfg_allocu(size) lp_allocu(cfg_mem, size)
#define cfg_allocz(size) lp_allocz(cfg_mem, size)
char *cfg_strdup(char *c);
void cfg_copy_list(list *dest, list *src, unsigned node_size);

/* Lexer */

extern int (*cf_read_hook)(byte *buf, unsigned int max, int fd);

struct symbol {
  struct symbol *next;
  struct sym_scope *scope;
  int class;
  int aux;
  void *aux2;
  void *def;
  char name[1];
};

/* Remember to update cf_symbol_class_name() */
#define SYM_VOID 0
#define SYM_PROTO 1
#define SYM_TEMPLATE 2
#define SYM_FUNCTION 3
#define SYM_FILTER 4
#define SYM_TABLE 5
#define SYM_ROA 6

#define SYM_VARIABLE 0x100	/* 0x100-0x1ff are variable types */
#define SYM_CONSTANT 0x200	/* 0x200-0x2ff are variable types */

#define SYM_TYPE(s) (((struct f_val *) (s)->def)->type)
#define SYM_VAL(s) (((struct f_val *) (s)->def)->val)

struct include_file_stack {
  void *buffer;				/* Internal lexer state */
  char *file_name;			/* File name */
  int fd;				/* File descriptor */
  int lino;				/* Current line num */
  int depth;				/* Include depth, 0 = cannot include */

  struct include_file_stack *prev;	/* Previous record in stack */
  struct include_file_stack *up;	/* Parent (who included this file) */
};

extern struct include_file_stack *ifs;


int cf_lex(void);
void cf_lex_init(int is_cli, struct config *c);
struct symbol *cf_find_symbol(byte *c);
struct symbol *cf_default_name(char *template, int *counter);
struct symbol *cf_define_symbol(struct symbol *symbol, int type, void *def);
void cf_push_scope(struct symbol *);
void cf_pop_scope(void);
struct symbol *cf_walk_symbols(struct config *cf, struct symbol *sym, int *pos);
char *cf_symbol_class_name(struct symbol *sym);

/* Parser */

int cf_parse(void);

/* Sysdep hooks */

void sysdep_preconfig(struct config *);
int sysdep_commit(struct config *, struct config *);
void sysdep_shutdown_done(void);

#endif
