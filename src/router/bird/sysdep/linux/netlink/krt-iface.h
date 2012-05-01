/*
 *	BIRD -- Unix Kernel Netlink Interface Syncer -- Dummy Include File
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_IFACE_H_
#define _BIRD_KRT_IFACE_H_

/*
 *  We don't have split iface/scan/set parts. See krt-scan.h.
 */

struct krt_if_params {
};

struct krt_if_status {
};

static inline void krt_if_construct(struct kif_config *c UNUSED) { };
static inline void krt_if_shutdown(struct kif_proto *p UNUSED) { };
static inline void krt_if_io_init(void) { };

static inline int kif_params_same(struct krt_if_params *old UNUSED, struct krt_if_params *new UNUSED) { return 1; }
static inline void kif_copy_params(struct krt_if_params *dest UNUSED, struct krt_if_params *src UNUSED) { }

#endif
