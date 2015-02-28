/*
 *	BIRD -- UNIX Kernel Route Syncer
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_H_
#define _BIRD_KRT_H_

struct config;
struct krt_config;
struct krt_proto;
struct kif_config;
struct kif_proto;

#include "lib/krt-sys.h"

/* Flags stored in net->n.flags, rest are in nest/route.h */

#define KRF_VERDICT_MASK 0x0f
#define KRF_CREATE 0			/* Not seen in kernel table */
#define KRF_SEEN 1			/* Seen in kernel table during last scan */
#define KRF_UPDATE 2			/* Need to update this entry */
#define KRF_DELETE 3			/* Should be deleted */
#define KRF_IGNORE 4			/* To be ignored */

#define EA_KRT_SOURCE	EA_CODE(EAP_KRT, 0)
#define EA_KRT_METRIC	EA_CODE(EAP_KRT, 1)
#define EA_KRT_PREFSRC	EA_CODE(EAP_KRT, 2)
#define EA_KRT_REALM	EA_CODE(EAP_KRT, 3)

/* Whenever we recognize our own routes, we allow learing of foreign routes */

#ifdef CONFIG_SELF_CONSCIOUS
#define KRT_ALLOW_LEARN
#endif

/* krt.c */

extern struct protocol proto_unix_kernel;

struct krt_config {
  struct proto_config c;
  struct krt_params sys;	/* Sysdep params */
  int persist;			/* Keep routes when we exit */
  int scan_time;		/* How often we re-scan routes */
  int learn;			/* Learn routes from other sources */
  int devroutes;		/* Allow export of device routes */
  int graceful_restart;		/* Regard graceful restart recovery */
};

struct krt_proto {
  struct proto p;
  struct krt_state sys;		/* Sysdep state */

#ifdef KRT_ALLOW_LEARN
  struct rtable krt_table;	/* Internal table of inherited routes */
#endif

#ifndef CONFIG_ALL_TABLES_AT_ONCE
  timer *scan_timer;
#endif

  node krt_node;		/* Node in krt_proto_list */
  byte ready;			/* Initial feed has been finished */
  byte initialized;		/* First scan has been finished */
};

extern pool *krt_pool;

#define KRT_CF ((struct krt_config *)p->p.cf)

#define KRT_TRACE(pr, fl, msg, args...) do {	\
  DBG("KRT: " msg "\n" , ## args);		\
  if (pr->p.debug & fl)				\
    { log(L_TRACE "%s: " msg, pr->p.name , ## args); } } while(0)

struct proto_config * kif_init_config(int class);
void kif_request_scan(void);
void krt_got_route(struct krt_proto *p, struct rte *e);
void krt_got_route_async(struct krt_proto *p, struct rte *e, int new);

/* Values for rte->u.krt_sync.src */
#define KRT_SRC_UNKNOWN	-1	/* Nobody knows */
#define KRT_SRC_BIRD	 0	/* Our route (not passed in async mode) */
#define KRT_SRC_REDIRECT 1	/* Redirect route, delete it */
#define KRT_SRC_ALIEN	 2	/* Route installed by someone else */
#define KRT_SRC_KERNEL	 3	/* Kernel routes, are ignored by krt syncer */

extern struct protocol proto_unix_iface;

struct kif_primary_item {
  node n;
  byte *pattern;
  ip_addr prefix;
  int pxlen;
};

struct kif_config {
  struct proto_config c;
  struct kif_params sys;	/* Sysdep params */
  int scan_time;		/* How often we re-scan interfaces */
  list primary;			/* Preferences for primary addresses (struct kif_primary_item) */
};

struct kif_proto {
  struct proto p;
  struct kif_state sys;		/* Sysdep state */
};

#define KIF_CF ((struct kif_config *)p->p.cf)

struct proto_config * krt_init_config(int class);


/* krt sysdep */

void krt_sys_init(struct krt_proto *);
void krt_sys_start(struct krt_proto *);
void krt_sys_shutdown(struct krt_proto *);
int krt_sys_reconfigure(struct krt_proto *p UNUSED, struct krt_config *n, struct krt_config *o);

void krt_sys_preconfig(struct config *);
void krt_sys_postconfig(struct krt_config *);
void krt_sys_init_config(struct krt_config *);
void krt_sys_copy_config(struct krt_config *, struct krt_config *);

int  krt_capable(rte *e);
void krt_do_scan(struct krt_proto *);
void krt_replace_rte(struct krt_proto *p, net *n, rte *new, rte *old, struct ea_list *eattrs);


/* kif sysdep */

void kif_sys_init(struct kif_proto *);
void kif_sys_start(struct kif_proto *);
void kif_sys_shutdown(struct kif_proto *);
int kif_sys_reconfigure(struct kif_proto *, struct kif_config *, struct kif_config *);

void kif_sys_init_config(struct kif_config *);
void kif_sys_copy_config(struct kif_config *, struct kif_config *);

void kif_do_scan(struct kif_proto *);

struct ifa *kif_get_primary_ip(struct iface *i);

#endif
