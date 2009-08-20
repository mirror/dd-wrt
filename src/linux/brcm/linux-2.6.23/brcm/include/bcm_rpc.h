/*
 * RPC module header file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: bcm_rpc.h,v 13.1.4.10 2008/09/06 16:07:16 Exp $
 */

#ifndef _BCM_RPC_H_
#define _BCM_RPC_H_

#include <typedefs.h>
#include <rpc_osl.h>

typedef struct rpc_info rpc_info_t;
typedef struct rpc_buf rpc_buf_t;
struct rpc_transport_info;
typedef void (*rpc_dispatch_cb_t)(void *ctx, struct rpc_buf* buf);
typedef void (*rpc_resync_cb_t)(void *ctx);
typedef void (*rpc_down_cb_t)(void *ctx);

extern struct rpc_info *bcm_rpc_attach(void *pdev, osl_t *osh, struct rpc_transport_info *rpc_th);
extern void bcm_rpc_down(struct rpc_info *rpci);
#ifdef NDIS
extern void bcm_rpc_sleep(struct rpc_info *rpc);
extern bool bcm_rpc_resume(struct rpc_info *rpc);
extern int bcm_rpc_shutdown(struct rpc_info *rpc);
#endif
extern void bcm_rpc_detach(struct rpc_info *rpc);
extern struct rpc_buf *bcm_rpc_buf_alloc(struct rpc_info *rpc, int len);
extern void bcm_rpc_buf_free(struct rpc_info *rpc, struct rpc_buf *b);
/* get rpc transport handle */
extern struct rpc_transport_info *bcm_rpc_tp_get(struct rpc_info *rpc);


/* callback for: data_rx, down, resync */
extern void bcm_rpc_rxcb_init(struct rpc_info *rpc, void *ctx, rpc_dispatch_cb_t cb,
                              void *dnctx, rpc_down_cb_t dncb, rpc_resync_cb_t resync_cb);
extern void bcm_rpc_rxcb_deinit(struct rpc_info *rpci);

/* HOST or CLIENT rpc call, requiring no return value */
extern int bcm_rpc_call(struct rpc_info *rpc, struct rpc_buf *b);

/* HOST rpc call, demanding return.
 *   The thread may be suspended and control returns back to OS
 *   The thread will resume(waked up) on either the return signal received or timeout
 *     The implementation details depend on OS
 */
extern struct rpc_buf *bcm_rpc_call_with_return(struct rpc_info *rpc, struct rpc_buf *b);

/* CLIENT rpc call to respond to bcm_rpc_call_with_return, requiring no return value */
extern int bcm_rpc_call_return(struct rpc_info *rpc, struct rpc_buf *retb);

extern uint bcm_rpc_buf_header_len(struct rpc_info *rpci);

#if defined(WLC_HIGH) && defined(BCMDBG)
extern int bcm_rpc_pktlog_get(struct rpc_info *rpci, uint32 *buf, uint buf_size, bool send);
extern int bcm_rpc_dump(rpc_info_t *rpci, struct bcmstrbuf *b);
#endif

#endif /* _BCM_RPC_H_ */
