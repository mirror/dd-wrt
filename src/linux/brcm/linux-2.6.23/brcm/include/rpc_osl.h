/*
 * RPC OSL
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: rpc_osl.h,v 13.2 2008-03-05 07:21:30 Exp $
 */

#ifndef _rpcosl_h_
#define _rpcosl_h_

#ifdef WLC_HIGH
typedef struct rpc_osl rpc_osl_t;
extern rpc_osl_t *rpc_osl_attach(osl_t *osh);
extern void rpc_osl_detach(rpc_osl_t *rpc_osh);

#define RPC_OSL_LOCK(rpc_osh) rpc_osl_lock((rpc_osh))
#define RPC_OSL_UNLOCK(rpc_osh) rpc_osl_unlock((rpc_osh))
#define RPC_OSL_WAIT(rpc_osh, to, ptimedout)	rpc_osl_wait((rpc_osh), (to), (ptimedout))
#define RPC_OSL_WAKE(rpc_osh)			rpc_osl_wake((rpc_osh))
extern void rpc_osl_lock(rpc_osl_t *rpc_osh);
extern void rpc_osl_unlock(rpc_osl_t *rpc_osh);
extern int rpc_osl_wait(rpc_osl_t *rpc_osh, uint ms, bool *ptimedout);
extern void rpc_osl_wake(rpc_osl_t *rpc_osh);

#else
typedef void rpc_osl_t;
#define rpc_osl_attach(a)	(rpc_osl_t *)0x0dadbeef
#define rpc_osl_detach(a)	do { }	while (0)

#define RPC_OSL_LOCK(a)		do { }	while (0)
#define RPC_OSL_UNLOCK(a)	do { }	while (0)
#define RPC_OSL_WAIT(a, b, c)	(TRUE)
#define RPC_OSL_WAKE(a, b)	do { }	while (0)

#endif /* WLC_HIGH */
#endif	/* _rpcosl_h_ */
