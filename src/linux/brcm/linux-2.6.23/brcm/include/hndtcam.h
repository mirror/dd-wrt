/*
 * HND SOCRAM TCAM software interface.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: hndtcam.h,v 1.3 2008/12/17 13:09:49 Exp $
 */
#ifndef _hndtcam_h_
#define _hndtcam_h_

/*
 * 0 - 1
 * 1 - 2 Consecutive locations are patched
 * 2 - 4 Consecutive locations are patched
 * 3 - 8 Consecutive locations are patched
 * 4 - 16 Consecutive locations are patched
 * Define default to patch 2 locations
 */

#ifdef  PATCHCOUNT
#define SRPC_PATCHCOUNT PATCHCOUNT
#else
#define PATCHCOUNT 0
#define SRPC_PATCHCOUNT PATCHCOUNT
#endif

/* N Consecutive location to patch */
#define SRPC_PATCHNLOC (1 << (SRPC_PATCHCOUNT))

/* patch values and address structure */
typedef struct patchaddrvalue {
	uint32	addr;
	uint32	value;
} patchaddrvalue_t;

extern void hnd_patch_init(void *srp);
extern void hnd_tcam_write(void *srp, uint16 index, uint32 data);
extern void hnd_tcam_read(void *srp, uint16 index, uint32 *content);
void * hnd_tcam_init(void *srp, uint no_addrs);
extern void hnd_tcam_disablepatch(void *srp);
extern void hnd_tcam_enablepatch(void *srp);
#ifdef CONFIG_XIP
extern void hnd_tcam_bootloader_load(void *srp, char *pvars);
#else
extern void hnd_tcam_load(void *srp, const  patchaddrvalue_t *patchtbl);
#endif /* CONFIG_XIP */

#endif /* _hndtcam_h_ */
