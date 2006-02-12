/*
 * Linux device driver tunables for
 * Broadcom BCM44XX and BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 * $Id$
 */

#ifndef _et_linux_h_
#define _et_linux_h_

/* tunables */
#define	NTXD		64		/* # tx dma ring descriptors (must be ^2) */
#define	NRXD		512		/* # rx dma ring descriptors (must be ^2) */
#define	NRXBUFPOST	32		/* try to keep this # rbufs posted to the chip */
#define	BUFSZ		2048		/* packet data buffer size */
#define	RXBUFSZ		(BUFSZ - 256)	/* receive buffer size */

#if defined(ILSIM) || defined(__arch_um__)
#undef	NTXD
#define	NTXD		16
#undef	NRXD
#define	NRXD		16
#undef	NRXBUFPOST
#define	NRXBUFPOST	2
#endif

#endif	/* _et_linux_h_ */
