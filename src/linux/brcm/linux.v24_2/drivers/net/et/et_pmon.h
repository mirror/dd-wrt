/*
 * PMON device driver tunables for
 * Broadcom BCM44XX/BCM47XX 10/100Mbps Ethernet Device Driver

 * Copyright 2004, Broadcom Corporation   
 * All Rights Reserved.                   
 *                                        
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;      
 * the contents of this file may not be disclosed to third parties, copied   
 * or duplicated in any form, in whole or in part, without the prior         
 * written permission of Broadcom Corporation.                               
 * $Id$
 */

#ifndef _et_pmon_h_
#define _et_pmon_h_

#define NTXD	        16
#define NRXD	        16
#define NRXBUFPOST	8   

#define NBUFS		(NRXBUFPOST + 8)

/* common tunables */
#define BUFSZ		2048		/* packet data buffer size */
#define	RXBUFSZ		2040		/* rx packet buffer size (leave 8 bytes for malloc) */
#define	MAXMULTILIST	32		/* max # multicast addresses */

#endif /* _et_pmon_h_ */
