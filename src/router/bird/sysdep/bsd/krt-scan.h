/*
 *	BIRD -- *BSD Kernel Route Syncer -- Scanning
 *
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_SCAN_H_
#define _BIRD_KRT_SCAN_H_

struct krt_scan_params {
};

struct krt_scan_status {
  list temp_ifs;			/* Temporary interfaces */
};

static inline int krt_scan_params_same(struct krt_scan_params *o UNUSED, struct krt_scan_params *n UNUSED) { return 1; }
static inline void krt_scan_copy_params(struct krt_scan_params *d UNUSED, struct krt_scan_params *s UNUSED) { }

#endif
