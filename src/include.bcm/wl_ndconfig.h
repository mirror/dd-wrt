/*
 * NDIS-specific Windows registry configuration override
 * defines for Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#ifndef _wl_ndconfig_h_
#define _wl_ndconfig_h_

/* bit definitions for "flags" */
#define SKIP_MINUS_1	1		/* -1 == "don't override driver default" */
#define SIMPLE_CONFIG	2		/* config val needs no special processing */

/* "param" field only used by RNDIS */
typedef struct {
	const char *str;
	uint type;
	int cmd;
	uint32 flags;
	PNDIS_CONFIGURATION_PARAMETER param;
} ndis_config_t;

#if defined(STA) || defined(UNDER_CE)
extern ndis_config_t *
wl_findconfig(
	const char *name,
	ndis_config_t *table,
	int *index
);

extern PNDIS_CONFIGURATION_PARAMETER
wl_readparam(
	ndis_config_t *ndis_config,
	const char *str,
	NDIS_HANDLE confighandle,
	void *wl,
	int *index
);

extern VOID
wl_readconfigoverrides(
	void *wl,
	wlc_info_t *wlc,
	NDIS_HANDLE confighandle,
	wl_oid_t *wl_oid,
	char *id,
	uint unit,
	uint OS
);

extern void
wl_scanoverrides(
	void *wl,
	wlc_info_t *wlc,
	NDIS_HANDLE confighandle,
	wl_oid_t *wl_oid,
	char *id,
	uint unit
);

#endif /* defined(STA) || defined(UNDER_CE) */

#endif /* _wl_ndconfig_h_ */
