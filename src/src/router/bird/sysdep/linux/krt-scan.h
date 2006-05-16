/*
 *	BIRD -- Linux Kernel Route Syncer -- Scanning
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
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

static inline int krt_scan_params_same(struct krt_scan_params *o, struct krt_scan_params *n) { return 1; }

#endif
