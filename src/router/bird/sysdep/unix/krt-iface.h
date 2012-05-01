/*
 *	BIRD -- Unix Kernel Interface Syncer
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_IFACE_H_
#define _BIRD_KRT_IFACE_H_

struct krt_if_params {
};

struct krt_if_status {
};

extern int if_scan_sock;

static inline int kif_params_same(struct krt_if_params *old UNUSED, struct krt_if_params *new UNUSED) { return 1; }
static inline void kif_copy_params(struct krt_if_params *dest UNUSED, struct krt_if_params *src UNUSED) { }

#endif
