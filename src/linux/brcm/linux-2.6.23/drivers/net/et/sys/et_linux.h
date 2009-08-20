/*
 * Linux device driver tunables for
 * Broadcom BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * $Id: et_linux.h,v 1.14.228.1 2008/09/25 02:24:02 Exp $
 */

#ifndef _et_linux_h_
#define _et_linux_h_

/* tunables */
#define	NTXD		128		/* # tx dma ring descriptors (must be ^2) */
#define	NRXD		512		/* # rx dma ring descriptors (must be ^2) */
#define	NRXBUFPOST	128		/* try to keep this # rbufs posted to the chip */
#define	BUFSZ		2048		/* packet data buffer size */
#define	RXBUFSZ		(BUFSZ - 256)	/* receive buffer size */

#ifndef RXBND
#define RXBND		32		/* max # rx frames to process in dpc */
#endif

#if defined(ILSIM) || defined(__arch_um__)
#undef	NTXD
#define	NTXD		16
#undef	NRXD
#define	NRXD		16
#undef	NRXBUFPOST
#define	NRXBUFPOST	2
#endif

#endif	/* _et_linux_h_ */
