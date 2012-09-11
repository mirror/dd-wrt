/*
 *	BIRD -- *BSD Kernel Route Syncer
 *
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_SYS_H_
#define _BIRD_KRT_SYS_H_


/* Kernel interfaces */

struct kif_params {
};

struct kif_status {
};


static inline void kif_sys_init(struct kif_proto *p UNUSED) { }
static inline int kif_sys_reconfigure(struct kif_proto *p UNUSED, struct kif_config *n UNUSED, struct kif_config *o UNUSED) { return 1; }

static inline void kif_sys_preconfig(struct config *c UNUSED) { }
static inline void kif_sys_postconfig(struct kif_config *c UNUSED) { }
static inline void kif_sys_init_config(struct kif_config *c UNUSED) { }
static inline void kif_sys_copy_config(struct kif_config *d UNUSED, struct kif_config *s UNUSED) { }


/* Kernel routes */

struct krt_params {
};

struct krt_status {
};


static inline void krt_sys_init(struct krt_proto *p UNUSED) { }
static inline int krt_sys_reconfigure(struct krt_proto *p UNUSED, struct krt_config *n UNUSED, struct krt_config *o UNUSED) { return 1; }

static inline void krt_sys_preconfig(struct config *c UNUSED) { }
static inline void krt_sys_postconfig(struct krt_config *c UNUSED) { }
static inline void krt_sys_init_config(struct krt_config *c UNUSED) { }
static inline void krt_sys_copy_config(struct krt_config *d UNUSED, struct krt_config *s UNUSED) { }


#endif
