/*
 *	BIRD -- Linux Kernel Netlink Route Syncer
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_SYS_H_
#define _BIRD_KRT_SYS_H_


/* Kernel interfaces */

struct kif_params {
};

struct kif_state {
};


static inline void kif_sys_init(struct kif_proto *p UNUSED) { }
static inline int kif_sys_reconfigure(struct kif_proto *p UNUSED, struct kif_config *n UNUSED, struct kif_config *o UNUSED) { return 1; }

static inline void kif_sys_preconfig(struct config *c UNUSED) { }
static inline void kif_sys_postconfig(struct kif_config *c UNUSED) { }
static inline void kif_sys_init_config(struct kif_config *c UNUSED) { }
static inline void kif_sys_copy_config(struct kif_config *d UNUSED, struct kif_config *s UNUSED) { }

static inline struct ifa * kif_get_primary_ip(struct iface *i) { return NULL; }


/* Kernel routes */

#define NL_NUM_TABLES 256

struct krt_params {
  int table_id;				/* Kernel table ID we sync with */
};

struct krt_state {
};


static inline void krt_sys_init(struct krt_proto *p UNUSED) { }


#endif
