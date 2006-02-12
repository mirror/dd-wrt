/*
 * NDIS Error mappings 
 *
 * Copyright(c) 2001 Broadcom Corp.
 */

#ifndef _ndiserrmap_h_
#define _ndiserrmap_h_

extern int ndisstatus2bcmerror(NDIS_STATUS status);
extern NDIS_STATUS bcmerror2ndisstatus(int bcmerror);

#endif	/* _ndiserrmap_h_ */
