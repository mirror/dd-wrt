/*
 *	BIRD -- Unix Kernel Socket Route Syncer -- Dummy Include File
 *
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_IFACE_H_
#define _BIRD_KRT_IFACE_H_

/*
 *  We don't have split iface/scan/set parts. See krt-sock.h.
 */
static inline int kif_params_same(struct krt_if_params *old UNUSED, struct krt_if_params *new UNUSED) { return 1; }

#endif
