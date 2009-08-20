/*
 * RPC layer. It links to bus layer with transport layer(bus dependent)
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: bcm_rpc.c,v 1.11.2.56 2008/10/18 06:13:35 Exp $
 */

#include <epivers.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>

#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <rpc_osl.h>

#if (!defined(WLC_HIGH) && !defined(WLC_LOW))
#error "SPLIT"
#endif
#if defined(WLC_HIGH) && defined(WLC_LOW)
#error "SPLIT"
#endif

/* RPC may use OS APIs directly to avoid overloading osl.h
 *  HIGH_ONLY supports NDIS and LINUX so far. can be ported to other OS if needed
 */
#ifdef WLC_HIGH
#if !defined(NDIS) && !defined(linux)
#error "RPC only supports NDIS and LINUX in HIGH driver"
#endif
#endif /* WLC_HIGH */
#ifdef WLC_LOW
#error "RPC only supports HNDRTE in LOW driver"
#endif /* WLC_LOW */

#define RPC_ERROR_VAL 0x0001
#define RPC_TRACE_VAL 0x0002
#define RPC_PKTTRACE_VAL 0x0004
#define RPC_PKTLOG_VAL 0x0008

static uint32 rpc_msg_level = RPC_ERROR_VAL; /* Print error messages even for non-debug drivers */

/* osl_msg_level is a bitvector with defs in wlioctl.h */
#define	RPC_ERR(args)		do {if (rpc_msg_level & RPC_ERROR_VAL) printf args;} while (0)

#ifdef	BCMDBG
#define	RPC_TRACE(args)		do {if (rpc_msg_level & RPC_TRACE_VAL) printf args;} while (0)
#define RPC_PKTTRACE_ON()	(rpc_msg_level & RPC_PKTTRACE_VAL)
#else
#ifdef BCMDBG_ERR
#define	RPC_TRACE(args)		do {if (rpc_msg_level & RPC_TRACE_VAL) printf args;} while (0)
#define RPC_PKTTRACE_ON()	(FALSE)
#define prhex(a, b, c)		do { } while (0)  /* prhex is not defined under */
#define RPC_PKTLOG_ON()	(FALSE)
#else
#define	RPC_TRACE(args)
#define RPC_PKTTRACE_ON()	(FALSE)
#define RPC_PKTLOG_ON()	(FALSE)
#define prhex(a, b, c) 	do { } while (0)  /* prhex is not defined under */
#endif /* BCMDBG_ERR */
#endif /* BCMDBG */

#if defined(WLC_HIGH) && defined(BCMDBG)
#define RPC_PKTLOG_ON()		(rpc_msg_level & RPC_PKTLOG_VAL)
#else
#define RPC_PKTLOG_ON()		(FALSE)
#endif /* defined(WLC_HIGH) && defined(BCMDBG) */

/* OS specific files for locks */
#define RPC_INIT_WAIT_TIMEOUT_MSEC	1000
#define RPC_RETURN_WAIT_TIMEOUT_MSEC	800 /* NDIS OIDs timeout in 1 second.
					     * This timeout needs to be smaller than that
					     */

/* RPC Frame formats */
/* |--------------||-------------|
 * RPC Header      RPC Payload
 *
 * 1) RPC Header:
 * |-------|--------|----------------|
 * 31      23       15               0
 * Type     Session  Transaction ID
 * = 0 Data
 * = 1 Return
 * = 2 Mgn
 *
 * 2) payload
 * Data and Return RPC payload is RPC all dependent
 *
 * Management frame formats:
 * |--------|--------|--------|--------|
 * Byte 0       1        2        3
 * Header     Action   Version  Reason
 *
 * Version is included only for following actions:
 * -- CONNECT
 * -- RESET
 * -- DOWN
 * -- CONNECT_ACK
 * -- CONNECT_NACK
 *
 * Reason sent only by BMAC for following actions:
 * -- CONNECT_ACK
 * -- CONNECT_NACk
 */

typedef uint32 rpc_header_t;

#define RPC_HDR_LEN	sizeof(rpc_header_t)
#define RPC_ACN_LEN	sizeof(uint32)
#define RPC_VER_LEN	sizeof(EPI_VERSION_NUM)
#define RPC_RC_LEN	sizeof(uint32)

#define RPC_HDR_TYPE(_rpch) (((_rpch) >> 24) & 0xff)
#define RPC_HDR_SESSION(_rpch) (((_rpch) >> 16) & 0xff)
#define RPC_HDR_XACTION(_rpch) ((_rpch) & 0xffff) /* When the type is data or return */

#define NAME_ENTRY(x) #x

/* RPC Header defines -- attached to every RPC call */
typedef enum {
	RPC_TYPE_DATA,	/* RPC call that go straight through */
	RPC_TYPE_RTN,	/* RPC calls that are syncrhonous */
	RPC_TYPE_MGN,	/* RPC state management */
} rpc_type_t;

typedef enum {
	RPC_RC_ACK =  0,
	RPC_RC_RECONNECT,
	RPC_RC_VER_MISMATCH
} rpc_rc_t;

/* Management actions */
typedef enum {
	RPC_NULL = 0,
	RPC_CONNECT,		/* Master (high) to slave (low). Slave to copy current
				 * session id and transaction id (mostly 0)
				 */
	RPC_CONNECT_ACK,	/* Ack from LOW_RPC */
	RPC_DOWN,		/* Down the other-end. The actual action is
				 * end specific.
				 */
	RPC_CONNECT_NACK,	/* Nack from LOW_RPC. This indicates potentially that
				 * dongle could already be running
				 */
	RPC_RESET		/* Resync using other end's session id (mostly HIGH->LOW)
				 * Also, reset the oe_trans, and trans to 0
				 */
} rpc_acn_t;

/* RPC States */
typedef enum {
	UNINITED = 0,
	WAIT_INITIALIZING,
	ESTABLISHED,
	DISCONNECTED,
	ASLEEP,
	WAIT_RESUME
} rpc_state_t;

#define	HDR_STATE_MISMATCH	0x1
#define	HDR_SESSION_MISMATCH	0x2
#define	HDR_XACTION_MISMATCH	0x4

#ifdef	BCMDBG
#define RPC_PKTLOG_SIZE 100 /* Depth of the history */
#define RPC_PKTLOG_DATASIZE 16

struct rpc_pktlog {
	uint16	trans;
	int	len;
	uint32	data[RPC_PKTLOG_DATASIZE]; /* First few bytes of the payload only */
};
#endif /* BCMDBG */
#ifdef WLC_LOW
static void do_rpcdump_cmd(uint32 arg, uint argc, char *argv[]);
#else
static void bcm_rpc_fatal_dump(void *arg);
#endif

#ifdef WLC_HIGH
/* This lock is needed to handle the Receive Re-order queue that guarantees
 * in-order receive as it was observed that in NDIS at least, USB subsystem does
 * not guarantee it
 */
#ifdef NDIS
#define RPC_RO_LOCK(ri)		NdisAcquireSpinLock(&(ri)->reorder_lock)
#define RPC_RO_UNLOCK(ri)	NdisReleaseSpinLock(&(ri)->reorder_lock)
#else
#define RPC_RO_LOCK(ri)		spin_lock_irqsave(&(ri)->reorder_lock, (ri)->reorder_flags);
#define RPC_RO_UNLOCK(ri)	spin_unlock_irqrestore(&(ri)->reorder_lock, (ri)->reorder_flags);
#endif /* NDIS */
#else
#define RPC_RO_LOCK(ri)		do { } while (0)
#define RPC_RO_UNLOCK(ri)	do { } while (0)
#endif /* WLC_HIGH */

struct rpc_info {
	void *pdev;			/* Per-port driver handle for rx callback */
	struct rpc_transport_info *rpc_th;	/* transport layer handle */
	osl_t *osh;

	rpc_dispatch_cb_t dispatchcb;	/* callback when data is received */
	void *ctx;			/* Callback context */

	rpc_down_cb_t dncb;		/* callback when RPC goes down */
	void *dnctx;			/* Callback context */

	rpc_resync_cb_t resync_cb;	/* callback when host reenabled and dongle
					 * was not rebooted. Uses dnctx
					 */

	uint8 session;			/* 255 sessions enough ? */
	uint16 trans;			/* More than 255 can't be pending */
	uint16 oe_trans;		/* OtherEnd tran id, dongle->host */
	uint16 rtn_trans;		/* BMAC: callreturn Id dongle->host */
	uint16 oe_rtn_trans;		/* HIGH: received BMAC callreturn id */

	rpc_buf_t *rtn_rpcbuf;		/* RPC ID for return transaction */

	rpc_state_t  state;
	uint reset;			/* # of resets */
	uint cnt_xidooo;		/* transactionID out of order */

	uint32 version;

	bool wait_init;
	bool wait_return;

	rpc_osl_t *rpc_osh;

#ifdef WLC_HIGH
#ifdef BCMDBG
	struct rpc_pktlog *send_log;
	uint16 send_log_idx;	/* Point to the next slot to fill-in */
	uint16 send_log_num;	/* Number of entries */

	struct rpc_pktlog *recv_log;
	uint16 recv_log_idx;	/* Point to the next slot to fill-in */
	uint16 recv_log_num;	/* Number of entries */
#endif /* BCMDBG */

#ifdef NDIS
	NDIS_SPIN_LOCK reorder_lock; /* TO RAISE the IRQ */
	bool reorder_lock_alloced;
#else
	spinlock_t	reorder_lock;
	ulong reorder_flags;
#endif /* NDIS */
#endif /* WLC_HIGH */
	/* Protect against rx reordering */
	rpc_buf_t *reorder_pktq;
};

static void bcm_rpc_tx_complete(void *ctx, rpc_buf_t *buf, int status);
static void bcm_rpc_buf_recv(void *context, rpc_buf_t *);
static bool bcm_rpc_buf_recv_inorder(rpc_info_t *rpci, rpc_buf_t *rpc_buf, mbool hdr_invalid);

#ifdef WLC_HIGH
static rpc_buf_t *bcm_rpc_buf_recv_high(struct rpc_info *rpci, rpc_type_t type, rpc_acn_t acn,
	rpc_buf_t *rpc_buf);

#ifdef BCMDBG
static void bcm_rpc_pktlog_init(rpc_info_t *rpci);
static void bcm_rpc_pktlog_deinit(rpc_info_t *rpci);
static struct rpc_pktlog *bcm_rpc_prep_entry(struct rpc_info * rpci, rpc_buf_t *b,
                                             struct rpc_pktlog *cur);
static void bcm_rpc_add_send_entry(struct rpc_info * rpci, struct rpc_pktlog *cur);
static void bcm_rpc_add_recv_entry(struct rpc_info * rpci, struct rpc_pktlog *cur);
#endif /* BCMDBG */
#else
static rpc_buf_t *bcm_rpc_buf_recv_low(struct rpc_info *rpci, rpc_header_t header,
	rpc_acn_t acn, rpc_buf_t *rpc_buf);
#endif /* WLC_HIGH */

static int bcm_rpc_up(rpc_info_t *rpci);
static void bcm_rpc_down_oe(rpc_info_t *rpci);
#ifdef NDIS
static int bcm_rpc_resume_oe(struct rpc_info *rpci);
#endif

/* Header and componet retrieval functions */
static INLINE rpc_header_t
bcm_rpc_header(struct rpc_info *rpci, rpc_buf_t *rpc_buf)
{
	rpc_header_t *rpch = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);
	return ltoh32(*rpch);
}

static INLINE rpc_acn_t
bcm_rpc_mgn_acn(struct rpc_info *rpci, rpc_buf_t *rpc_buf)
{
	rpc_header_t *rpch = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);

	return (rpc_acn_t)ltoh32(*rpch);
}

static INLINE uint32
bcm_rpc_mgn_ver(struct rpc_info *rpci, rpc_buf_t *rpc_buf)
{
	rpc_header_t *rpch = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);

	return ltoh32(*rpch);
}

static INLINE rpc_rc_t
bcm_rpc_mgn_reason(struct rpc_info *rpci, rpc_buf_t *rpc_buf)
{
	rpc_header_t *rpch = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);
	return (rpc_rc_t)ltoh32(*rpch);
}


static INLINE uint
bcm_rpc_hdr_xaction_validate(struct rpc_info *rpci, rpc_header_t header, uint32 *xaction,
                             bool verbose)
{
	uint type;

	type = RPC_HDR_TYPE(header);
	*xaction = RPC_HDR_XACTION(header);

	/* High driver does not check the return transaction to be in order */
	if (type != RPC_TYPE_MGN &&
#ifdef WLC_HIGH
	    type != RPC_TYPE_RTN &&
#endif
	     *xaction != rpci->oe_trans) {
#ifdef WLC_HIGH
		if (verbose) {
			RPC_ERR(("Transaction mismatch: expected:0x%x got:0x%x type: %d\n",
				rpci->oe_trans, *xaction, type));
		}
#endif
		return HDR_XACTION_MISMATCH;
	}

	return 0;
}

static INLINE uint
bcm_rpc_hdr_session_validate(struct rpc_info *rpci, rpc_header_t header)
{
#ifdef WLC_LOW
	if (RPC_HDR_TYPE(header) == RPC_TYPE_MGN)
		return 0;
#endif

	if (rpci->session != RPC_HDR_SESSION(header))
	    return HDR_SESSION_MISMATCH;
	return 0;
}

static INLINE uint
bcm_rpc_hdr_state_validate(struct rpc_info *rpci, rpc_header_t header)
{
	uint type = RPC_HDR_TYPE(header);
	/* Everything allowed during this transition time */
	if (rpci->state == ASLEEP)
		return 0;

	/* Only managment frames allowed before ESTABLISHED state */
	if ((rpci->state != ESTABLISHED) && (type != RPC_TYPE_MGN)) {
		RPC_ERR(("bcm_rpc_header_validate: State mismatch: state:%d type:%d\n",
		           rpci->state, type));
		return HDR_STATE_MISMATCH;
	}

	return 0;
}

static INLINE mbool
bcm_rpc_hdr_validate(struct rpc_info *rpci, rpc_buf_t *rpc_buf, uint32 *xaction,
                     bool verbose)
{
	/* First the state against the type */
	mbool ret = 0;
	rpc_header_t header = bcm_rpc_header(rpci, rpc_buf);

	mboolset(ret, bcm_rpc_hdr_state_validate(rpci, header));
	mboolset(ret, bcm_rpc_hdr_xaction_validate(rpci, header, xaction, verbose));
	mboolset(ret, bcm_rpc_hdr_session_validate(rpci, header));

	return ret;
}

struct rpc_info *
BCMATTACHFN(bcm_rpc_attach)(void *pdev, osl_t *osh, struct rpc_transport_info *rpc_th)
{
	struct rpc_info *rpci;

	if ((rpci = (struct rpc_info *)MALLOC(osh, sizeof(struct rpc_info))) == NULL)
		return NULL;

	bzero(rpci, sizeof(struct rpc_info));
	rpci->osh = osh;
	rpci->pdev = pdev;
	rpci->rpc_th = rpc_th;

	/* initialize lock and queue */
	rpci->rpc_osh = rpc_osl_attach(osh);

	if (rpci->rpc_osh == NULL) {
		RPC_ERR(("bcm_rpc_attach: osl attach failed\n"));
		goto fail;
	}

	bcm_rpc_tp_register_cb(rpc_th, bcm_rpc_tx_complete, rpci,
	                       bcm_rpc_buf_recv, rpci, rpci->rpc_osh);

	rpci->version = EPI_VERSION_NUM;

	if (bcm_rpc_up(rpci)) {
		RPC_ERR(("bcm_rpc_attach: rpc_up failed\n"));
		goto fail;
	}

	return rpci;
fail:
	bcm_rpc_detach(rpci);
	return NULL;
}

void
BCMATTACHFN(bcm_rpc_detach)(struct rpc_info *rpci)
{
	if (!rpci)
		return;

#ifdef WLC_HIGH
#ifdef BCMDBG
	bcm_rpc_pktlog_deinit(rpci);
#endif /* BCMDBG */

	if (rpci->reorder_pktq) {
		rpc_buf_t * node;
		ASSERT(rpci->rpc_th);
		while ((node = rpci->reorder_pktq)) {
			rpci->reorder_pktq = bcm_rpc_buf_next_get(rpci->rpc_th,
			                                          node);
			bcm_rpc_buf_next_set(rpci->rpc_th, node, NULL);
			bcm_rpc_tp_buf_free(rpci->rpc_th, node);
		}
		ASSERT(rpci->reorder_pktq == NULL);
	}

#if defined(NDIS)
	if (rpci->reorder_lock_alloced)
		NdisFreeSpinLock(&rpcb->lock);
#endif
#endif /* WLC_HIGH */

	/* rpc is going away, cut off registered cbs from rpc_tp layer */
	bcm_rpc_tp_deregister_cb(rpci->rpc_th);

#ifdef WLC_LOW
	bcm_rpc_tp_txflowctlcb_deinit(rpci->rpc_th);
#endif

	if (rpci->rpc_osh)
		rpc_osl_detach(rpci->rpc_osh);

	MFREE(rpci->osh, rpci, sizeof(struct rpc_info));
	rpci = NULL;
}

rpc_buf_t *
bcm_rpc_buf_alloc(struct rpc_info *rpci, int datalen)
{
	rpc_buf_t *rpc_buf;
	int len = datalen + RPC_HDR_LEN;

	ASSERT(rpci->rpc_th);
	rpc_buf = bcm_rpc_tp_buf_alloc(rpci->rpc_th, len);

	if (rpc_buf == NULL)
		return NULL;

	/* Reserve space for RPC Header */
	bcm_rpc_buf_pull(rpci->rpc_th, rpc_buf, RPC_HDR_LEN);

	return rpc_buf;
}

uint
bcm_rpc_buf_header_len(struct rpc_info *rpci)
{
	return RPC_HDR_LEN;
}

void
bcm_rpc_buf_free(struct rpc_info *rpci, rpc_buf_t *rpc_buf)
{
	bcm_rpc_tp_buf_free(rpci->rpc_th, rpc_buf);
}

void
bcm_rpc_rxcb_init(struct rpc_info *rpci, void *ctx, rpc_dispatch_cb_t cb,
                  void *dnctx, rpc_down_cb_t dncb, rpc_resync_cb_t resync_cb)
{
	rpci->dispatchcb = cb;
	rpci->ctx = ctx;
	rpci->dnctx = dnctx;
	rpci->dncb = dncb;
	rpci->resync_cb = resync_cb;
}

void
bcm_rpc_rxcb_deinit(struct rpc_info *rpci)
{
	if (!rpci)
		return;

	rpci->dispatchcb = NULL;
	rpci->ctx = NULL;
	rpci->dnctx = NULL;
	rpci->dncb = NULL;
	rpci->resync_cb = NULL;
}

struct rpc_transport_info *
bcm_rpc_tp_get(struct rpc_info *rpci)
{
	return rpci->rpc_th;
}

static void
rpc_header_prep(struct rpc_info *rpci, rpc_header_t *header, uint type, uint action)
{
	uint32 v;

	v = 0;
	v |= (type << 24);

	/* Mgmt action follows the header */
	if (type == RPC_TYPE_MGN) {
		*(header + 1) = htol32(action);
#ifdef WLC_HIGH
		if (action == RPC_CONNECT || action == RPC_RESET)
			*(header + 2) = htol32(rpci->version);
#endif
	}
#ifdef WLC_LOW
	else if (type == RPC_TYPE_RTN)
		v |= (rpci->rtn_trans);
#endif
	else
		v |= (rpci->trans);

	v |= (rpci->session << 16);

	*header = htol32(v);

	RPC_TRACE(("rpc_header_prep: type:0x%x action: %d trans:0x%x\n",
	           type, action, rpci->trans));
}

#ifdef WLC_HIGH
static int
bcm_rpc_up(struct rpc_info *rpci)
{
	rpc_buf_t *rpc_buf;
	rpc_header_t *header;
	int ret;

	/* Allocate a frame, prep it, send and wait */
	rpc_buf = bcm_rpc_tp_buf_alloc(rpci->rpc_th, RPC_HDR_LEN + RPC_ACN_LEN + RPC_VER_LEN);

	if (!rpc_buf)
		return -1;

	header = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);

	rpc_header_prep(rpci, header, RPC_TYPE_MGN, RPC_CONNECT);

	RPC_OSL_LOCK(rpci->rpc_osh);
	rpci->state = WAIT_INITIALIZING;
	rpci->wait_init = TRUE;

#if defined(NDIS)
	NdisAllocateSpinLock(&rpci->reorder_lock);
	rpci->reorder_lock_alloced = TRUE;
#else
	spin_lock_init(&rpci->reorder_lock);
#endif

	RPC_OSL_UNLOCK(rpci->rpc_osh);

	if (bcm_rpc_tp_buf_send(rpci->rpc_th, rpc_buf)) {
		RPC_ERR(("%s: bcm_rpc_tp_buf_send() call failed\n", __FUNCTION__));
		return -1;
	}

	/* Wait for state to change to established. The receive thread knows what to do */
	RPC_ERR(("%s: waiting to be connected\n", __FUNCTION__));

	ret = RPC_OSL_WAIT(rpci->rpc_osh, RPC_INIT_WAIT_TIMEOUT_MSEC, NULL);

	RPC_TRACE(("%s: wait done, ret = %d\n", __FUNCTION__, ret));

	if (ret < 0) {
		rpci->wait_init = FALSE;
		return ret;
	}

	/* See if we timed out or actually initialized */
	RPC_OSL_LOCK(rpci->rpc_osh);
	if (rpci->state == ESTABLISHED)
		ret = 0;
	else
		ret = -1;
	rpci->wait_init = FALSE;
	RPC_OSL_UNLOCK(rpci->rpc_osh);

#ifdef BCMDBG
	bcm_rpc_pktlog_init(rpci);
#endif /* BCMDBG */

	return ret;
}

#ifdef NDIS
void
bcm_rpc_sleep(struct rpc_info *rpci)
{
	bcm_rpc_tp_sleep(rpci->rpc_th);
	rpci->state = ASLEEP;
	/* Ignore anything coming after this */
	rpci->session++;
}

int
bcm_rpc_shutdown(struct rpc_info *rpci)
{
	int ret = bcm_rpc_tp_shutdown(rpci->rpc_th);
	rpci->state = DISCONNECTED;
	return ret;
}

bool
bcm_rpc_resume(struct rpc_info *rpci)
{
	bcm_rpc_tp_resume(rpci->rpc_th);

	if (bcm_rpc_resume_oe(rpci) == 0) {
		rpci->trans = 0;
		rpci->oe_trans = 0;
	}
	return (rpci->state == ESTABLISHED);
}

static int
bcm_rpc_resume_oe(struct rpc_info *rpci)
{
	rpc_buf_t *rpc_buf;
	rpc_header_t *header;
	int ret;

	/* Allocate a frame, prep it, send and wait */
	rpc_buf = bcm_rpc_tp_buf_alloc(rpci->rpc_th, RPC_HDR_LEN + RPC_ACN_LEN + RPC_VER_LEN);

	if (!rpc_buf)
		return -1;

	header = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);

	rpc_header_prep(rpci, header, RPC_TYPE_MGN, RPC_RESET);

	RPC_OSL_LOCK(rpci->rpc_osh);
	rpci->state = WAIT_RESUME;
	rpci->wait_init = TRUE;
	RPC_OSL_UNLOCK(rpci->rpc_osh);

	/* Don't care for the return value */
	if (bcm_rpc_tp_buf_send(rpci->rpc_th, rpc_buf)) {
		RPC_ERR(("%s: bcm_rpc_tp_buf_send() call failed\n", __FUNCTION__));
		return -1;
	}

	/* Wait for state to change to established. The receive thread knows what to do */
	RPC_ERR(("%s: waiting to be resumed\n", __FUNCTION__));

	ret = RPC_OSL_WAIT(rpci->rpc_osh, RPC_INIT_WAIT_TIMEOUT_MSEC, NULL);

	RPC_TRACE(("%s: wait done, ret = %d\n", __FUNCTION__, ret));

	if (ret < 0) {
		rpci->wait_init = FALSE;
		return ret;
	}

	/* See if we timed out or actually initialized */
	RPC_OSL_LOCK(rpci->rpc_osh);
	if (rpci->state == ESTABLISHED)
		ret = 0;
	else
		ret = -1;
	rpci->wait_init = FALSE;
	RPC_OSL_UNLOCK(rpci->rpc_osh);

	return ret;
}
#endif /* NDIS */
#else
static int
bcm_rpc_up(struct rpc_info *rpci)
{
	rpci->state = WAIT_INITIALIZING;
	hndrte_cons_addcmd("rpcdump", do_rpcdump_cmd, (uint32)rpci);
	return 0;
}

static int
bcm_rpc_connect_resp(struct rpc_info *rpci, rpc_acn_t acn, uint32 reason)
{
	rpc_buf_t *rpc_buf;
	rpc_header_t *header;

	/* Allocate a frame, prep it, send and wait */
	rpc_buf = bcm_rpc_tp_buf_alloc(rpci->rpc_th, RPC_HDR_LEN + RPC_ACN_LEN +
	                               RPC_RC_LEN + RPC_VER_LEN);
	if (!rpc_buf) {
		RPC_ERR(("%s: bcm_rpc_tp_buf_alloc() failed\n", __FUNCTION__));
		return FALSE;
	}

	header = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);

	rpc_header_prep(rpci, header, RPC_TYPE_MGN, acn);

	*(header + 2) = ltoh32(rpci->version);
	*(header + 3) = ltoh32(reason);

	if (bcm_rpc_tp_buf_send(rpci->rpc_th, rpc_buf)) {
		RPC_ERR(("%s: bcm_rpc_tp_buf_send() call failed\n", __FUNCTION__));
		return FALSE;
	}

	return TRUE;
}
#endif /* WLC_HIGH */

void
bcm_rpc_down(struct rpc_info *rpci)
{
	RPC_ERR(("%s\n", __FUNCTION__));

	RPC_OSL_LOCK(rpci->rpc_osh);
	if (rpci->state != DISCONNECTED && rpci->state != ASLEEP) {
#ifdef WLC_HIGH
		bcm_rpc_fatal_dump(rpci);
#else
		do_rpcdump_cmd((uint32)rpci, 0, NULL);
#endif
		rpci->state = DISCONNECTED;
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		if (rpci->dncb)
			(rpci->dncb)(rpci->dnctx);
		bcm_rpc_tp_down(rpci->rpc_th);
		return;
	}
	RPC_OSL_UNLOCK(rpci->rpc_osh);
}

static void
bcm_rpc_down_oe(rpc_info_t *rpci)
{
	rpc_buf_t *rpc_buf;
	rpc_header_t *header;

	/* Allocate a frame, prep it, send and wait */
	rpc_buf = bcm_rpc_tp_buf_alloc(rpci->rpc_th, RPC_HDR_LEN + RPC_ACN_LEN);

	if (!rpc_buf)
		return;

	header = (rpc_header_t *)bcm_rpc_buf_data(rpci->rpc_th, rpc_buf);

	rpc_header_prep(rpci, header, RPC_TYPE_MGN, RPC_DOWN);

	/* Don't care for the return value */
	bcm_rpc_tp_buf_send(rpci->rpc_th, rpc_buf);
}

static void
bcm_rpc_tx_complete(void *ctx, rpc_buf_t *buf, int status)
{
	struct rpc_info *rpci = (struct rpc_info *)ctx;

	RPC_TRACE(("%s: status 0x%x\n", __FUNCTION__, status));

	ASSERT(rpci && rpci->rpc_th);

	/* RPC_BUFFER_TX: dealloc */
	if (buf)
		bcm_rpc_tp_buf_free(rpci->rpc_th, buf);

	if (status)
		bcm_rpc_down(rpci);
}

int
bcm_rpc_call(struct rpc_info *rpci, rpc_buf_t *b)
{
	rpc_header_t *header;
	int err = 0;
#if defined(WLC_HIGH) && defined(BCMDBG)
	struct rpc_pktlog cur;
#endif

	RPC_TRACE(("%s:\n", __FUNCTION__));

	RPC_OSL_LOCK(rpci->rpc_osh);
	if (rpci->state != ESTABLISHED) {
		err = -1;
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		bcm_rpc_buf_free(rpci, b);
		goto done;
	}
	RPC_OSL_UNLOCK(rpci->rpc_osh);

#if defined(WLC_HIGH) && defined(BCMDBG)
	/* Prepare the current log entry but add only if the TX was successful */
	/* This is done here before DATA pointer gets modified */
	if (RPC_PKTLOG_ON())
		bcm_rpc_prep_entry(rpci, b, &cur);
#endif

	header = (rpc_header_t *)bcm_rpc_buf_push(rpci->rpc_th, b, RPC_HDR_LEN);

	rpc_header_prep(rpci, header, RPC_TYPE_DATA, 0);

#ifdef BCMDBG
	if (RPC_PKTTRACE_ON()) {
		prhex("RPC Call ", bcm_rpc_buf_data(rpci->rpc_th, b),
		      bcm_rpc_buf_len_get(rpci->rpc_th, b));
	}
#endif

	if (bcm_rpc_tp_buf_send(rpci->rpc_th, b)) {
		RPC_ERR(("%s: bcm_rpc_tp_buf_send() call failed\n", __FUNCTION__));
		bcm_rpc_down(rpci);
		return -1;
	}

	rpci->trans++;

#if defined(WLC_HIGH) && defined(BCMDBG)
	/* Since successful add the entry */
	if (RPC_PKTLOG_ON())
		bcm_rpc_add_send_entry(rpci, &cur);
#endif /* BCMDBG */
done:
	return err;
}

#ifdef WLC_HIGH
rpc_buf_t *
bcm_rpc_call_with_return(struct rpc_info *rpci, rpc_buf_t *b)
{
	rpc_header_t *header;
	rpc_buf_t *retb = NULL;
	int ret;
#ifdef BCMDBG
	struct rpc_pktlog cur;
#endif
	bool timedout = FALSE;

	RPC_TRACE(("%s:\n", __FUNCTION__));

	RPC_OSL_LOCK(rpci->rpc_osh);
	if (rpci->state != ESTABLISHED) {
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		RPC_ERR(("%s: RPC call before ESTABLISHED state\n", __FUNCTION__));
		bcm_rpc_buf_free(rpci, b);
		return NULL;
	}
	RPC_OSL_UNLOCK(rpci->rpc_osh);

#ifdef BCMDBG
	/* Prepare the current log entry but add only if the TX was successful */
	/* This is done here before DATA pointer gets modified */
	if (RPC_PKTLOG_ON())
		bcm_rpc_prep_entry(rpci, b, &cur);
#endif

	header = (rpc_header_t *)bcm_rpc_buf_push(rpci->rpc_th, b, RPC_HDR_LEN);

	rpc_header_prep(rpci, header, RPC_TYPE_RTN, 0);

	RPC_OSL_LOCK(rpci->rpc_osh);
	rpci->trans++;
	ASSERT(rpci->rtn_rpcbuf == NULL);
	rpci->wait_return = TRUE;

	/* Prep the return packet BEFORE sending the buffer and also within spinlock
	 * within raised IRQ
	 */
	bcm_rpc_tp_recv_rtn(rpci->rpc_th);

	RPC_OSL_UNLOCK(rpci->rpc_osh);

#ifdef BCMDBG
	if (RPC_PKTTRACE_ON()) {
		prhex("RPC Call With Return Buf", bcm_rpc_buf_data(rpci->rpc_th, b),
		      bcm_rpc_buf_len_get(rpci->rpc_th, b));
	}
#endif

	if (bcm_rpc_tp_buf_send(rpci->rpc_th, b)) {
		RPC_ERR(("%s: bcm_rpc_bus_buf_send() failed\n", __FUNCTION__));

		RPC_OSL_LOCK(rpci->rpc_osh);
		rpci->wait_return = FALSE;
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		bcm_rpc_down(rpci);
		return NULL;
	}

	ret = RPC_OSL_WAIT(rpci->rpc_osh, RPC_RETURN_WAIT_TIMEOUT_MSEC, &timedout);

	RPC_OSL_LOCK(rpci->rpc_osh);
	if (ret || timedout) {
		RPC_ERR(("%s: RPC call return wait err %d timedout:%d\n",
		         __FUNCTION__, ret, timedout));
		rpci->wait_return = FALSE;
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		bcm_rpc_down(rpci);
		return NULL;
	}

	/* See if we timed out or actually initialized */
	retb = rpci->rtn_rpcbuf;
	rpci->rtn_rpcbuf = NULL;
	rpci->wait_return = FALSE; /* Could have woken up by timeout */
	RPC_OSL_UNLOCK(rpci->rpc_osh);

#ifdef BCMDBG
	/* Since successful add the entry */
	if (RPC_PKTLOG_ON())
		bcm_rpc_add_send_entry(rpci, &cur);
#endif /* BCMDBG */

	return retb;
}
#endif /* WLC_HIGH */

#ifdef WLC_LOW
int
bcm_rpc_call_return(struct rpc_info *rpci, rpc_buf_t *b)
{
	rpc_header_t *header;

	RPC_TRACE(("%s\n", __FUNCTION__));

	header = (rpc_header_t *)bcm_rpc_buf_push(rpci->rpc_th, b, RPC_HDR_LEN);

	rpc_header_prep(rpci, header, RPC_TYPE_RTN, 0);

#ifdef BCMDBG
	if (RPC_PKTTRACE_ON()) {
		prhex("RPC Call Return Buf", bcm_rpc_buf_data(rpci->rpc_th, b),
		      bcm_rpc_buf_len_get(rpci->rpc_th, b));
	}
#endif

	/* If the TX fails, it's sender's responsibilty */
	if (bcm_rpc_tp_send_callreturn(rpci->rpc_th, b)) {
		RPC_ERR(("%s: bcm_rpc_tp_buf_send() call failed\n", __FUNCTION__));
		bcm_rpc_down(rpci);
		return -1;
	}

	rpci->rtn_trans++;
	return 0;
}
#endif /* WLC_LOW */

/* This is expected to be called at DPC of the bus driver ? */
static void
bcm_rpc_buf_recv(void *context, rpc_buf_t *rpc_buf)
{
	uint xaction;
	struct rpc_info *rpci = (struct rpc_info *)context;
	mbool hdr_invalid = 0;
	ASSERT(rpci && rpci->rpc_th);

	RPC_TRACE(("%s:\n", __FUNCTION__));

	RPC_RO_LOCK(rpci);

	/* Only if the header itself checks out , and only xaction does not */
	hdr_invalid = bcm_rpc_hdr_validate(rpci, rpc_buf, &xaction, TRUE);

	if (mboolisset(hdr_invalid, HDR_XACTION_MISMATCH) &&
	    !mboolisset(hdr_invalid, ~HDR_XACTION_MISMATCH)) {
		rpc_buf_t *node = rpci->reorder_pktq;
		rpci->cnt_xidooo++;

		/* Catch roll-over or retries */
		rpci->reorder_pktq = rpc_buf;

		if (node != NULL)
			bcm_rpc_buf_next_set(rpci->rpc_th, rpc_buf, node);
		goto done;
	}

	/* Bail out if failed */
	if (!bcm_rpc_buf_recv_inorder(rpci, rpc_buf, hdr_invalid))
		goto done;

	while (rpci->reorder_pktq) {
		bool found = FALSE;
		rpc_buf_t *buf = rpci->reorder_pktq;
		rpc_buf_t *prev = rpci->reorder_pktq;
		while (buf != NULL) {
			rpc_buf_t *next = bcm_rpc_buf_next_get(rpci->rpc_th, buf);
			hdr_invalid = bcm_rpc_hdr_validate(rpci, buf, &xaction, FALSE);

			if (!mboolisset(hdr_invalid, HDR_XACTION_MISMATCH)) {
				bcm_rpc_buf_next_set(rpci->rpc_th, buf, NULL);

				/* Bail out if failed */
				if (!bcm_rpc_buf_recv_inorder(rpci, buf, hdr_invalid))
					goto done;

				if (buf == rpci->reorder_pktq)
					rpci->reorder_pktq = next;
				else
					bcm_rpc_buf_next_set(rpci->rpc_th, prev, next);
				buf = NULL;
				found = TRUE;
			} else {
				prev = buf;
				buf = next;
			}
		}

		/* bail if not found */
		if (!found)
			break;
	}

done:
	RPC_RO_UNLOCK(rpci);
}

static bool
bcm_rpc_buf_recv_inorder(rpc_info_t *rpci, rpc_buf_t *rpc_buf, mbool hdr_invalid)
{
	rpc_header_t header;
	rpc_acn_t acn = RPC_NULL;

	ASSERT(rpci && rpci->rpc_th);

	RPC_TRACE(("%s: got rpc_buf %p len %d data %p\n", __FUNCTION__,
	           rpc_buf, bcm_rpc_buf_len_get(rpci->rpc_th, rpc_buf),
	           bcm_rpc_buf_data(rpci->rpc_th, rpc_buf)));

#ifdef BCMDBG
	if (RPC_PKTTRACE_ON()) {
		prhex("RPC Rx Buf", bcm_rpc_buf_data(rpci->rpc_th, rpc_buf),
		      bcm_rpc_buf_len_get(rpci->rpc_th, rpc_buf));
	}
#endif

	header = bcm_rpc_header(rpci, rpc_buf);

	RPC_OSL_LOCK(rpci->rpc_osh);

	if (hdr_invalid) {
		RPC_ERR(("%s: bcm_rpc_hdr_validate failed on 0x%08x 0x%x\n", __FUNCTION__,
		         header, hdr_invalid));
		bcm_rpc_tp_buf_free(rpci->rpc_th, rpc_buf);
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		if (mboolisset(hdr_invalid, HDR_STATE_MISMATCH)) {
			bcm_rpc_down_oe(rpci);
			bcm_rpc_down(rpci);
		}
		return FALSE;
	}

	RPC_TRACE(("%s state:0x%x type:0x%x session:0x%x xacn:0x%x\n", __FUNCTION__, rpci->state,
		RPC_HDR_TYPE(header), RPC_HDR_SESSION(header), RPC_HDR_XACTION(header)));

	bcm_rpc_buf_pull(rpci->rpc_th, rpc_buf, RPC_HDR_LEN);

	switch (RPC_HDR_TYPE(header)) {
	case RPC_TYPE_MGN:
		acn = bcm_rpc_mgn_acn(rpci, rpc_buf);
		bcm_rpc_buf_pull(rpci->rpc_th, rpc_buf, RPC_ACN_LEN);
		RPC_TRACE(("Mgn: %x\n", acn));
		break;
	case RPC_TYPE_RTN:
#ifdef WLC_HIGH
		rpci->oe_rtn_trans = RPC_HDR_XACTION(header) + 1;
		break;
#endif
	case RPC_TYPE_DATA:
		rpci->oe_trans = RPC_HDR_XACTION(header) + 1;
		break;
	default:
		ASSERT(0);
	};

#ifdef WLC_HIGH
	rpc_buf = bcm_rpc_buf_recv_high(rpci, RPC_HDR_TYPE(header), acn, rpc_buf);
#else
	rpc_buf = bcm_rpc_buf_recv_low(rpci, header, acn, rpc_buf);
#endif
	RPC_OSL_UNLOCK(rpci->rpc_osh);

	if (rpc_buf)
		bcm_rpc_tp_buf_free(rpci->rpc_th, rpc_buf);
	return TRUE;
}

#ifdef WLC_HIGH
static void
bcm_rpc_buf_recv_mgn_high(struct rpc_info *rpci, rpc_acn_t acn, rpc_buf_t *rpc_buf)
{
	rpc_rc_t reason = RPC_RC_ACK;
	uint32 version = 0;

	RPC_ERR(("%s: Recvd:%x Version: 0x%x\nState: %x Session:%d\n", __FUNCTION__,
	         acn, rpci->version, rpci->state, rpci->session));

	if (acn == RPC_CONNECT_ACK || acn == RPC_CONNECT_NACK) {
		version = bcm_rpc_mgn_ver(rpci, rpc_buf);
		bcm_rpc_buf_pull(rpci->rpc_th, rpc_buf, RPC_VER_LEN);

		reason = bcm_rpc_mgn_reason(rpci, rpc_buf);

		RPC_ERR(("%s: Reason: %x Dongle Version: 0x%x\n", __FUNCTION__,
		         reason, version));
	}

	switch (acn) {
	case RPC_CONNECT_ACK:
		/* If the original thread has not given up,
		 * then change the state and wake it up
		 */
		if (rpci->state != UNINITED) {
			rpci->state = ESTABLISHED;
			RPC_ERR(("%s: Connected!\n", __FUNCTION__));
			if (rpci->wait_init)
				RPC_OSL_WAKE(rpci->rpc_osh);
		}
		ASSERT(reason != RPC_RC_VER_MISMATCH);
		break;

	case RPC_CONNECT_NACK:
		/* Connect failed. Just bail out by waking the thread */
		RPC_ERR(("%s: Connect failed !!!\n", __FUNCTION__));
		if (rpci->wait_init)
			RPC_OSL_WAKE(rpci->rpc_osh);
		break;

	case RPC_DOWN:
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		bcm_rpc_down(rpci);
		RPC_OSL_LOCK(rpci->rpc_osh);
		break;

	default:
		ASSERT(0);
		break;
	}
}

static rpc_buf_t *
bcm_rpc_buf_recv_high(struct rpc_info *rpci, rpc_type_t type, rpc_acn_t acn, rpc_buf_t *rpc_buf)
{
	RPC_TRACE(("%s: acn %d\n", __FUNCTION__, acn));

	switch (type) {
	case RPC_TYPE_RTN:
		if (rpci->wait_return) {
			rpci->rtn_rpcbuf = rpc_buf;
			rpc_buf = NULL;
			RPC_OSL_WAKE(rpci->rpc_osh);
		} else if (rpci->state != DISCONNECTED)
			RPC_ERR(("%s: Received return buffer but no one waiting\n", __FUNCTION__));
		break;

	case RPC_TYPE_MGN:
		bcm_rpc_buf_recv_mgn_high(rpci, acn, rpc_buf);
		break;

	case RPC_TYPE_DATA:
		ASSERT(rpci->state == ESTABLISHED);
#ifdef BCMDBG
		/* Prepare the current log entry but add only if the TX was successful */
		/* This is done here before DATA pointer gets modified */
		if (RPC_PKTLOG_ON()) {
			struct rpc_pktlog cur;
			bcm_rpc_prep_entry(rpci, rpc_buf, &cur);
			bcm_rpc_add_recv_entry(rpci, &cur);
		}
#endif /* BCMDBG */
		if (rpci->dispatchcb) {
			(rpci->dispatchcb)(rpci->ctx, rpc_buf);
			rpc_buf = NULL;
		} else {
			RPC_ERR(("%s: no rpcq callback, drop the pkt\n", __FUNCTION__));
		}
		break;

	default:
		ASSERT(0);
	}

	return (rpc_buf);
}
#else
static void
bcm_rpc_buf_recv_mgn_low(struct rpc_info *rpci, uint8 session, rpc_acn_t acn, rpc_buf_t *rpc_buf)
{
	uint32 reason = 0;
	uint32 version = 0;

	RPC_TRACE(("%s: Recvd:%x Version: 0x%x\nState: %x Session:%d\n", __FUNCTION__,
	         acn,
	         rpci->version, rpci->state, rpci->session));

	if (acn == RPC_CONNECT || acn == RPC_RESET) {
		version = bcm_rpc_mgn_ver(rpci, rpc_buf);

		RPC_ERR(("%s: Host Version: 0x%x\n", __FUNCTION__, version));

		ASSERT(rpci->state != UNINITED);

		if (version != rpci->version) {
			RPC_ERR(("RPC Establish failed due to version mismatch\n"));
			RPC_ERR(("Expected: 0x%x Got: 0x%x\n", rpci->version, version));
			RPC_ERR(("Connect failed !!!\n"));

			rpci->state = WAIT_INITIALIZING;
			bcm_rpc_connect_resp(rpci, RPC_CONNECT_NACK, RPC_RC_VER_MISMATCH);
			return;
		}

		/* When receiving CONNECT/RESET from HIGH, just
		 * resync to the HIGH's session and reset the transactions
		 */
		if ((acn == RPC_CONNECT) && (rpci->state == ESTABLISHED))
			reason = RPC_RC_RECONNECT;

		rpci->session = session;

		if (bcm_rpc_connect_resp(rpci, RPC_CONNECT_ACK, reason)) {
			/* call the resync callback if already established */
			if ((acn == RPC_CONNECT) && (rpci->state == ESTABLISHED) &&
			    (rpci->resync_cb)) {
				(rpci->resync_cb)(rpci->dnctx);
			}
			rpci->state = ESTABLISHED;
		} else {
			RPC_ERR(("%s: RPC Establish failed !!!\n", __FUNCTION__));
		}

		RPC_ERR(("Connected Session:%x!\n", rpci->session));
		rpci->oe_trans = 0;
		rpci->trans = 0;
		rpci->rtn_trans = 0;
	} else if (acn == RPC_DOWN)
		bcm_rpc_down(rpci);
}

static rpc_buf_t *
bcm_rpc_buf_recv_low(struct rpc_info *rpci, rpc_header_t header,
                     rpc_acn_t acn, rpc_buf_t *rpc_buf)
{
	switch (RPC_HDR_TYPE(header)) {
	case RPC_TYPE_MGN:
		bcm_rpc_buf_recv_mgn_low(rpci, RPC_HDR_SESSION(header), acn, rpc_buf);
		break;

	case RPC_TYPE_RTN:
	case RPC_TYPE_DATA:
		ASSERT(rpci->state == ESTABLISHED);
		if (rpci->dispatchcb) {
			(rpci->dispatchcb)(rpci->ctx, rpc_buf);
			rpc_buf = NULL;
		} else {
			RPC_ERR(("%s: no rpcq callback, drop the pkt\n", __FUNCTION__));
			ASSERT(0);
		}
		break;

	default:
		ASSERT(0);
	}

	return (rpc_buf);
}
#endif /* WLC_HIGH */

#ifdef WLC_HIGH
#ifdef BCMDBG
static void
bcm_rpc_pktlog_init(rpc_info_t *rpci)
{
	rpc_msg_level |= RPC_PKTLOG_VAL;
	if (RPC_PKTLOG_ON()) {
		if ((rpci->send_log = MALLOC(rpci->osh,
		                             sizeof(struct rpc_pktlog) * RPC_PKTLOG_SIZE)) == NULL)
			goto err;
		bzero(rpci->send_log, sizeof(struct rpc_pktlog) * RPC_PKTLOG_SIZE);
		if ((rpci->recv_log = MALLOC(rpci->osh,
		                             sizeof(struct rpc_pktlog) * RPC_PKTLOG_SIZE)) == NULL)
			goto err;
		bzero(rpci->recv_log, sizeof(struct rpc_pktlog) * RPC_PKTLOG_SIZE);
		return;
	}

err:
	bcm_rpc_pktlog_deinit(rpci);
}

static void
bcm_rpc_pktlog_deinit(rpc_info_t *rpci)
{
	if (rpci->send_log) {
		MFREE(rpci->osh, rpci->send_log, sizeof(struct rpc_pktlog) * RPC_PKTLOG_SIZE);
		rpci->send_log = NULL;
	}
	if (rpci->recv_log) {
		MFREE(rpci->osh, rpci->recv_log, sizeof(struct rpc_pktlog) * RPC_PKTLOG_SIZE);
		rpci->recv_log = NULL;
	}
	rpc_msg_level &= ~RPC_PKTLOG_VAL; /* Turn off logging on failure */
}

static struct rpc_pktlog *
bcm_rpc_prep_entry(struct rpc_info * rpci, rpc_buf_t *b, struct rpc_pktlog *cur)
{
	bzero(cur, sizeof(struct rpc_pktlog));
	cur->trans = rpci->trans;
	cur->len = bcm_rpc_buf_len_get(rpci->rpc_th, b);
	bcopy(bcm_rpc_buf_data(rpci->rpc_th, b), cur->data, RPC_PKTLOG_DATASIZE);
	return cur;
}

static void
bcm_rpc_add_send_entry(struct rpc_info * rpci, struct rpc_pktlog *cur)
{
	RPC_OSL_LOCK(rpci->rpc_osh);
	bcopy(cur, &rpci->send_log[rpci->send_log_idx], sizeof(struct rpc_pktlog));
	rpci->send_log_idx = (rpci->send_log_idx + 1) % RPC_PKTLOG_SIZE;

	if (rpci->send_log_num < RPC_PKTLOG_SIZE)
		rpci->send_log_num++;

	RPC_OSL_UNLOCK(rpci->rpc_osh);
}

static void
bcm_rpc_add_recv_entry(struct rpc_info * rpci, struct rpc_pktlog *cur)
{
	bcopy(cur, &rpci->recv_log[rpci->recv_log_idx], sizeof(struct rpc_pktlog));
	rpci->recv_log_idx = (rpci->recv_log_idx + 1) % RPC_PKTLOG_SIZE;

	if (rpci->recv_log_num < RPC_PKTLOG_SIZE)
		rpci->recv_log_num++;
}

int
bcm_rpc_pktlog_get(struct rpc_info *rpci, uint32 *buf, uint buf_size, bool send)
{
	int ret;
	int start, i, tot;

	/* Clear the whole buffer */
	bzero(buf, buf_size);
	RPC_OSL_LOCK(rpci->rpc_osh);
	if (send) {
		ret = rpci->send_log_num;
		if (ret < RPC_PKTLOG_SIZE)
			start = 0;
		else
			start = (rpci->send_log_idx + 1) % RPC_PKTLOG_SIZE;
	} else {
		ret = rpci->recv_log_num;
		if (ret < RPC_PKTLOG_SIZE)
			start = 0;
		else
			start = (rpci->recv_log_idx + 1) % RPC_PKTLOG_SIZE;
	}

	/* Return only first byte */
	if (buf_size < (uint)ret) {
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		return BCME_BUFTOOSHORT;
	}

	if (ret == 0) {
		RPC_OSL_UNLOCK(rpci->rpc_osh);
		return ret;
	}

	tot = ret;
	for (i = 0; tot > 0; tot--, i++) {
		if (send) {
			buf[i] = rpci->send_log[start].data[0];
			/* buf[i+1] = rpci->send_log[start].trans; */
			start++;
		} else
			buf[i] = rpci->recv_log[start++].data[0];
		start = (start % RPC_PKTLOG_SIZE);
	}
	RPC_OSL_UNLOCK(rpci->rpc_osh);

	return ret;
}

int
bcm_rpc_dump(rpc_info_t *rpci, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "\nbcm rpc dump:\n");
	RPC_OSL_LOCK(rpci->rpc_osh);
	bcm_bprintf(b, "Version: 0x%x State: %x\n", rpci->version, rpci->state);
	bcm_bprintf(b, "session %d trans 0x%x oe_trans 0x%x rtn_trans 0x%x oe_rtn_trans 0x%x\n",
	            rpci->session, rpci->trans, rpci->oe_trans,
	            rpci->rtn_trans, rpci->oe_rtn_trans);
	RPC_OSL_UNLOCK(rpci->rpc_osh);
	return bcm_rpc_tp_dump(rpci->rpc_th, b);
}

#endif /* BCMDBG */
#endif /* WLC_HIGH */

#ifdef WLC_LOW
static void
do_rpcdump_cmd(uint32 arg, uint argc, char *argv[])
#else
static void
bcm_rpc_fatal_dump(void *arg)
#endif
{
	rpc_info_t *rpci = (rpc_info_t *)(uintptr)arg;
	printf("Version: 0x%x State: %x\n", rpci->version, rpci->state);
	printf("session %d trans 0x%x oe_trans 0x%x rtn_trans 0x%x\n",
	       rpci->session, rpci->trans, rpci->oe_trans,
	       rpci->rtn_trans);
	printf("xactionID out of order %d\n", rpci->cnt_xidooo);

#ifdef WLC_LOW
	bcm_rpc_tp_dump(rpci->rpc_th);
#endif
}
