/*
 *	BIRD -- Unix Kernel Netlink Route Syncer -- Dummy Include File
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_SET_H_
#define _BIRD_KRT_SET_H_

/*
 *  We don't have split iface/scan/set parts. See krt-scan.h.
 */

struct krt_set_params {
};

struct krt_set_status {
};

static inline void krt_set_construct(struct krt_config *c UNUSED) { };
static inline void krt_set_start(struct krt_proto *p UNUSED, int first UNUSED) { };
static inline void krt_set_shutdown(struct krt_proto *p UNUSED, int last UNUSED) { };
static inline int krt_set_params_same(struct krt_set_params *o UNUSED, struct krt_set_params *n UNUSED) { return 1; }
static inline void krt_set_copy_params(struct krt_set_params *d UNUSED, struct krt_set_params *s UNUSED) { }

#endif
