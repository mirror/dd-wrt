/* sscop.c - SSCOP (Q.2110) protocol */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <netinet/in.h> /* for htonl */

#include "atmd.h"

#include "sscop.h"
#include "queue.h"
#include "pdu.h"


/*
 * This is a quite exact translation of the ITU-T SSCOP SDL diagrams. They
 * are a pleasant example of how protocols _should_ be specified. It took me
 * less than a week to implement this.
 *
 * Calls back to the SSCOP user are always done last in a sequence of actions
 * to avoid reentrancy problems if the user invokes SSCOP from the callback
 * function. (Exception: data indications are delivered when needed. So you
 * shouldn't kill the protocol stack in the middle of this. Releasing the
 * call and such is fine, though.)
 *
 * Sequences of an AA-RELASE.indication/confirm immediately followed by a
 * AA-ESTABLISH.indication have been replaced by a new primitive called
 * "restart". This way, the SSCOP user is able to distinguish conditions where
 * SSCOP is in idle state after a callback from conditions where SSCOP wants to
 * continue (and where the user many not want to stop SSCOP).
 *
 * The entity receiving management information (e.g. maa_error) must not issue
 * any SSCOP primitives from that callback routine. Instead, actions must be
 * queued until SSCOP finishes processing of the current event.
 *
 * Comments of the type xx.yy are section or figure numbers in Q.2110
 *
 * KNOWN BUGS: The queue and buffer management stinks, especially the "buffer
 *	       cloning". All this ought to be rewritten to use skbuffs. Since
 *	       this will happen anyway if SSCOP ever gets integrated into the
 *	       kernel, we can safely ignore the problem for now.
 *
 *	       The lower layer is always assumed to be ready for sending. If
 *	       this is not the case (shouldn't happen), the PDU should be
 *	       discarded (or the process might be blocked for a few cycles).
 */


#define COMPONENT "SSCOP"


/* Configurable SSCOP parameters */

#define SSCOP_CF_MR	 30	/* buffer size */
#define SSCOP_CF_MaxCC	 10	/* max timeouts */
#define SSCOP_CF_MaxPD	100	/* SD/POLL ratio */
#define SSCOP_CF_MaxSTAT 67	/* max elements in STAT PDU, 7.7 */

#undef POLL_AFTER_RETRANSMISSION /* 20.38, #define this if needed */


/* Timers */

/*
 * Assumptions: RTT = 1 sec, remote needs about 1 sec to wake up. SSCF changes
 * all this anyway.
 */

#define TIMER_CC	 2000000 /* 2 sec (== RTT+eps) */
#define TIMER_POLL	 1000000 /* 1 sec (== RTT) */
#define TIMER_NORESP	 7000000 /* 7 sec (keepalive+RTT+eps) */
#define TIMER_KEEPALIVE  5000000 /* 5 sec (> poll && > RTT) */
#define TIMER_IDLE	30000000 /* 30 sec (>> keepalive) */


/* Some helper macros */

#define START_TIMER(t) ({ STOP_TIMER(dsc->timer_##t); \
  dsc->timer_##t = start_timer(dsc->cf_timer_##t, sscop_##t##_exp,dsc); })
#define STOP_TIMER(t) ({ if (t) stop_timer(t); t = NULL; })
#define MOD24(x) ((x) & ((1 << 24)-1))
#define INC24(v) (v = MOD24(v+1))
#define INC8(v) (v = (v+1) & 255)
#define NORMALIZE(x,b) (((x)-(b)) & 0xffffff)
#define NORM_RX(v) NORMALIZE((v),dsc->vr_r)
#define NORM_TX(v) NORMALIZE((v),dsc->vt_a)
#define NORM_TXP(v) NORMALIZE((v),dsc->vt_pa)
#define NEGATIVE(v) ((v) & 0x800000)


static const char *state_name[] = { "Idle","OutConnPend","InConnPend",
  "OutDiscPend","OutResyPend","InResyPend","OutRecPend","RecRespPend",
  "InRecPend","DataTransReady" };


static void do_diag(int severity,const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    vdiag(COMPONENT,severity,fmt,ap);
    va_end(ap);
}


static void maa_error(void *arg,char code,int count)
{
    /*
     * arg is void * because it's also used in pdu.c, which doesn't know about
     * SSCOP_DSC
     */
    SSCOP_DSC *dsc = arg;

    static const char *const msgs[] = {
	"A:SD PDU","B:BGN PDU","C:BGAK PDU","D:BGREJ PDU","E:END PDU",
	"F:ENDAK PDU","G:POLL PDU","H:STAT PDU","I:USTAT PDU","J:RS",
	"K:RSAK PDU","L:ER","M:ERAK","O:VT(CC)>=MaxCC",
	"P:Timer_NO_RESPONSE expiry","Q:SD or POLL, N(S) error",
	"R:STAT N(PS) error","S:STAT N(R) or list elements error",
	"T:USTAT N(R) or list elements error","U:PDU length violation",
	"V:SD PDUs must be retransmitted","W:Lack of credit",
	"X:Credit obtained","\0:Unknown error code" };
    const char *const *walk;

    if (dsc->ops->maa_error)
	if (!dsc->ops->maa_error(dsc->user,code,count)) return;
    for (walk = msgs; **walk; walk++)
	if (**walk == code) break;
    if (code != 'V')
	diag(COMPONENT,DIAG_WARN,"layer management - error %c \"%s\"",code,
	  (*walk)+2);
    else diag(COMPONENT,DIAG_WARN,"layer management - error %c,%d \"%s\"",code,
      count,(*walk)+2);
}


static int sdu_length(BUFFER *buf)
{
    return buf->length-4-SSCOP_PAD((char *) buf->data+buf->length-4);
}


static BUFFER *build_pdu(SSCOP_DSC *dsc,unsigned char type,void *data,
  int num)
{
    BUFFER *buf;
    uint32_t *trailer;
    int pad;

    buf = buffer_create(num+12,dsc->vt_s); /* space for trailers */
    if (data && num) {
	memset((char *) buf->data+(num & ~3),0,4); /* clear padding area */
	memcpy((unsigned char *) buf->data,data,num);
	pad = (4-(num & 3)) & 3;
	trailer = (uint32_t *) ((char *) buf->data+num+pad);
    }
    else {
	pad = 0;
	trailer = (uint32_t *) buf->data;
    }
    diag(COMPONENT,DIAG_DEBUG,"generating %s PDU",pdu_name[type]);
    switch (type) {
	case SSCOP_BGN:
	case SSCOP_RS:
	case SSCOP_ER:
	    *trailer++ = dsc->mode == sscop_qsaal1 ? SSCOP_TRAIL(0,0,0) :
	      SSCOP_TRAIL(0,0,dsc->vt_sq);
	    *trailer++ = SSCOP_TRAIL(type,pad,dsc->vr_mr);
	    break;
	case SSCOP_BGAK:
	case SSCOP_RSAK:
	case SSCOP_ERAK:
	    *trailer++ = SSCOP_TRAIL(0,0,0);
	    *trailer++ = SSCOP_TRAIL(type,pad,dsc->vr_mr);
	    break;
	case SSCOP_END: /* S bit is indicated by "num" */
	    *trailer++ = SSCOP_TRAIL(0,0,0);
	    *trailer++ = SSCOP_TRAIL(type,pad,0) | (num ? htonl(SSCOP_S_BIT) :
	      0);
	    break;
	case SSCOP_BGREJ:
	case SSCOP_ENDAK:
	    *trailer++ = SSCOP_TRAIL(0,0,0);
	    *trailer++ = SSCOP_TRAIL(type,pad,0);
	    break;
	case SSCOP_POLL:
	    *trailer++ = SSCOP_TRAIL(0,0,dsc->vt_ps);
	    /* fall through */
	case SSCOP_SD:
	    *trailer++ = SSCOP_TRAIL(type,pad,dsc->vt_s);
	    break;
	case SSCOP_STAT:
	    *trailer++ = SSCOP_TRAIL(0,0,dsc->vr_ps);
	    /* fall through */
	case SSCOP_USTAT:
	    *trailer++ = SSCOP_TRAIL(0,0,dsc->vr_mr);
	    *trailer++ = SSCOP_TRAIL(type,0,dsc->vr_r);
	    break;
	case SSCOP_UD:
	case SSCOP_MD:
	    *trailer++ = SSCOP_TRAIL(type,pad,0);
	    break;
	default:
	    diag(COMPONENT,DIAG_FATAL,
	      "requested construction of unknown PDU type %d",type);
    }
    buf->length = (char *) trailer-(char *) buf->data;
    return buf;
}


static void emit_pdu(SSCOP_DSC *dsc,BUFFER *buf)
{
    BUFFER **last;

    {
	PDU_VARS;

	if (DECOMPOSE_PDU(dsc,buf->data,buf->length))
	    diag(COMPONENT,DIAG_FATAL,"composed garbage");
	PRINT_PDU("SEND",buf->data);
    }
    if (dsc->ops->cpcs_send)
	dsc->ops->cpcs_send(dsc->user,buf->data,buf->length);
    switch (SSCOP_TYPE((char *) buf->data+buf->length-4)) {
	case SSCOP_BGN:
	    last = &dsc->last_bgn;
	    break;
	case SSCOP_END:
	    last = &dsc->last_end;
	    break;
	case SSCOP_RS:
	    last = &dsc->last_rs;
	    break;
	case SSCOP_ER:
	    last = &dsc->last_er;
	    break;
	default:
	    buffer_discard(buf);
	    return;
    }
    if (*last && *last != buf) {
	buffer_discard(*last);
	*last = NULL;
    }
    *last = buf;
}


static void resend_pdu(SSCOP_DSC *dsc,BUFFER *buf)
{
    if (!buf) diag(COMPONENT,DIAG_FATAL,"resend buffer is NULL");
    emit_pdu(dsc,buf);
}


static void send_pdu(SSCOP_DSC *dsc,unsigned char type,void *data,int size)
{
    BUFFER *buf;

    buf = build_pdu(dsc,type,data,size);
    emit_pdu(dsc,buf);
}


static void next_state(SSCOP_DSC *dsc,SSCOP_STATE state)
{
    diag(COMPONENT,DIAG_DEBUG,"entering state %s (%d)",state_name[state],
      (int) state);
    dsc->state = state;
}


static void bad_pdu(SSCOP_DSC *dsc,unsigned char type)
{
		/*           111111 */
		/* 0123456789012345 */
    maa_error(dsc,"?BCxFJKDALGHIyzM"[type],0);
}


static void send_ustat(SSCOP_DSC *dsc,int first,int last)
{
    uint32_t range[2];

    range[0] = htonl(first);
    range[1] = htonl(last);
    send_pdu(dsc,SSCOP_USTAT,range,8);
}


/* --- Utility functions --------------------------------------------------- */


static void clear_receive_buffer(SSCOP_DSC *dsc)
{
    queue_clear(&dsc->rx_buf);
}


static void clear_transmission_buffer(SSCOP_DSC *dsc)
{
    queue_clear(&dsc->tx_buf);
}


static void clear_transmission_queue(SSCOP_DSC *dsc)
{
    queue_clear(&dsc->tx_q);
}


static void clear_retransmission_queue(SSCOP_DSC *dsc)
{
    queue_clear(&dsc->rt_q);
}


/* --- SSCOP subroutines --------------------------------------------------- */


static void sscop_poll_exp(void *user);
static void sscop_keepalive_exp(void *user);
static void sscop_noresp_exp(void *user);


static void release_buffers(SSCOP_DSC *dsc) /* 20.49 */
{
    clear_transmission_queue(dsc);
    clear_transmission_buffer(dsc);
    clear_retransmission_queue(dsc);
    clear_receive_buffer(dsc);
}


static void clear_transmitter(SSCOP_DSC *dsc) /* 20.49 */
{
    if (!dsc->clear_buffers) {
	clear_transmission_queue(dsc);
	clear_transmission_buffer(dsc);
    }
}


static void prepare_recovery(SSCOP_DSC *dsc) /* 20.49 */
{
    if (dsc->clear_buffers) {
	clear_transmission_queue(dsc);
	clear_transmission_buffer(dsc);
    }
    clear_retransmission_queue(dsc);
}


static void prepare_retrieval(SSCOP_DSC *dsc) /* 20.49 */
{
    prepare_recovery(dsc);
    clear_receive_buffer(dsc);
}


static void deliver_data(SSCOP_DSC *dsc) /* 20.49 */
{
    BUFFER *buf;

    if (!dsc->clear_buffers) {
	while ((buf = queue_get(&dsc->rx_buf))) {
	    if (dsc->ops->data_ind)
		dsc->ops->data_ind(dsc->user,buf->data,buf->length,buf->key);
	    buffer_discard(buf);
	}
    }
    clear_receive_buffer(dsc);
}


static void initialize_state_variables(SSCOP_DSC *dsc) /* 20.49 */
{
    dsc->vt_s = dsc->vt_ps = dsc->vt_a = 0;
    dsc->vt_pa = 1;
    dsc->vt_pd = 0;
    dsc->credit = 1;
    dsc->vr_r = dsc->vr_h = 0;
}


static int detect_retransmission(SSCOP_DSC *dsc,int sq) /* 20.50 */
{
    if (dsc->mode == sscop_qsaal1) return 0;
    if (dsc->vr_sq == sq) return 1;
    dsc->vr_sq = sq;
    return 0;
}


static void set_poll_timer(SSCOP_DSC *dsc) /* 20.50 */
{
    if (queue_peek(&dsc->tx_q) || dsc->vt_s != dsc->vt_a) START_TIMER(poll);
    else START_TIMER(keepalive);
}


static void reset_data_transfer_timers(SSCOP_DSC *dsc) /* 20.51 */
{
    STOP_TIMER(dsc->timer_poll);
    STOP_TIMER(dsc->timer_keepalive);
    STOP_TIMER(dsc->timer_noresp);
    STOP_TIMER(dsc->timer_idle);
}


static void set_data_transfer_timers(SSCOP_DSC *dsc) /* 20.51 */
{
    START_TIMER(poll);
    START_TIMER(noresp);
}


static void initialize_vr_mr(SSCOP_DSC *dsc)
{
    dsc->vr_mr = SSCOP_CF_MR;
}


static void update_vr_mr(SSCOP_DSC *dsc)
{
    dsc->vr_mr = MOD24(dsc->vr_r+SSCOP_CF_MR);
}


/* --- Timer handlers ------------------------------------------------------ */


static void sscop_cc_exp(void *user)
{
    SSCOP_DSC *dsc = user;

    diag(COMPONENT,DIAG_DEBUG,"Timer CC has expired");
    dsc->timer_cc = NULL;
    switch (dsc->state) {
	case sscop_outconn: /* 20.9 */
	    if (dsc->vt_cc < dsc->cf_max_cc) {
		dsc->vt_cc++;
		resend_pdu(dsc,dsc->last_bgn);
		START_TIMER(cc);
		return;
	    }
	    maa_error(dsc,'O',0);
	    send_pdu(dsc,SSCOP_END,NULL,1);
	    next_state(dsc,sscop_idle);
	    if (dsc->ops->rel_ind) dsc->ops->rel_ind(dsc->user,NULL,0,0);
	    return;
	case sscop_outdisc: /* 20.15 */
	    if (dsc->vt_cc < dsc->cf_max_cc) {
		dsc->vt_cc++;
		resend_pdu(dsc,dsc->last_end);
		START_TIMER(cc);
		return;
	    }
	    maa_error(dsc,'O',0);
	    next_state(dsc,sscop_idle);
	    if (dsc->ops->rel_conf) dsc->ops->rel_conf(dsc->user);
	    return;
	case sscop_outres: /* 20.18 */
	    if (dsc->vt_cc < dsc->cf_max_cc) {
		dsc->vt_cc++;
		resend_pdu(dsc,dsc->last_rs);
		START_TIMER(cc);
		return;
	    }
	    maa_error(dsc,'O',0);
	    send_pdu(dsc,SSCOP_END,NULL,1);
	    next_state(dsc,sscop_idle);
	    if (dsc->ops->rel_ind) dsc->ops->rel_ind(dsc->user,NULL,0,0);
	    return;
	case sscop_outrec: /* 20.24 */
	    if (dsc->vt_cc < dsc->cf_max_cc) {
		dsc->vt_cc++;
		resend_pdu(dsc,dsc->last_er);
		START_TIMER(cc);
		return;
	    }
	    maa_error(dsc,'O',0);
	    send_pdu(dsc,SSCOP_END,NULL,1);
	    clear_receive_buffer(dsc);
	    next_state(dsc,sscop_idle);
	    if (dsc->ops->rel_ind) dsc->ops->rel_ind(dsc->user,NULL,0,0);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"Timer CC expired in state %s",
      state_name[dsc->state]);
}


static void sscop_common_exp(void *user)
{
    SSCOP_DSC *dsc = user;

    if (dsc->state != sscop_ready) {
	diag(COMPONENT,DIAG_FATAL,"sscop_common_exp invoked in state %s",
	  state_name[dsc->state]);
	return;
    }
    INC24(dsc->vt_ps);
    send_pdu(dsc,SSCOP_POLL,NULL,0);
    dsc->vt_pd = 0;
    set_poll_timer(dsc);
}


static void sscop_poll_exp(void *user)
{
    SSCOP_DSC *dsc = user;

    diag(COMPONENT,DIAG_DEBUG,"Timer POLL has expired");
    dsc->timer_poll = NULL;
    sscop_common_exp(user);
}


static void sscop_keepalive_exp(void *user)
{
    SSCOP_DSC *dsc = user;

    diag(COMPONENT,DIAG_DEBUG,"Timer KEEPALIVE has expired");
    dsc->timer_keepalive = NULL;
    sscop_common_exp(user);
}


static void sscop_idle_exp(void *user)
{
    SSCOP_DSC *dsc = user;

    diag(COMPONENT,DIAG_DEBUG,"Timer IDLE has expired");
    dsc->timer_idle = NULL;
    START_TIMER(noresp);
    sscop_common_exp(user);
}


static void sscop_noresp_exp(void *user)
{
    SSCOP_DSC *dsc = user;

    diag(COMPONENT,DIAG_DEBUG,"Timer NORESP has expired");
    dsc->timer_noresp = NULL;
    if (dsc->state != sscop_ready) {
	diag(COMPONENT,DIAG_FATAL,"Timer NORESP expired in state %s",
	  state_name[dsc->state]);
	return;
    }
    reset_data_transfer_timers(dsc);
    maa_error(dsc,'P',0);
    send_pdu(dsc,SSCOP_END,NULL,1);
    prepare_retrieval(dsc);
    next_state(dsc,sscop_idle);
    if (dsc->ops->rel_ind) dsc->ops->rel_ind(dsc->user,NULL,0,0);
}


/* --- Transmit engine ----------------------------------------------------- */


static void try_to_send(SSCOP_DSC *dsc)
{
    BUFFER *buf,*buf2;

    if (dsc->state != sscop_ready) return;
    while (queue_peek(&dsc->rt_q) || NORM_TX(dsc->vt_s) <
      NORM_TX(dsc->vt_ms) || dsc->timer_idle) {
	buf = queue_get(&dsc->rt_q);
	if (buf) {
	    if (!(buf2 = queue_lookup(&dsc->tx_buf,buf->key)))
		diag(COMPONENT,DIAG_FATAL,"didn't find PDU %d in TX buffer",
		  buf->key);
	    emit_pdu(dsc,buf);
	    buf2->extra = dsc->vt_ps;
#ifdef POLL_AFTER_RETRANSMISSION
	    if (queue_peek(dsc->rt_q))
		/* fall through to goto */
#endif
	    goto B; /* sigh ... */
	}
	else {
	    if (!queue_peek(&dsc->tx_q)) return;
	    if (NORM_TX(dsc->vt_s) < NORM_TX(dsc->vt_ms)) {
		buf = queue_get(&dsc->tx_q);
		buf2 = build_pdu(dsc,SSCOP_SD,buf->data,buf->length);
		buffer_discard(buf);
		buf2->key = dsc->vt_s;
		buf2->extra = dsc->vt_ps;
		emit_pdu(dsc,buffer_clone(buf2));
		queue_put(&dsc->tx_buf,buf2);
		INC24(dsc->vt_s);
	B:
		/* B */
		dsc->vt_pd++;
		if (dsc->timer_poll) {
		    if (dsc->vt_pd < dsc->cf_max_pd) continue;
		}
		else {
		    if (!dsc->timer_idle) STOP_TIMER(dsc->timer_keepalive);
		    else {
			STOP_TIMER(dsc->timer_idle);
			START_TIMER(noresp);
		    }
		    if (dsc->vt_pd < dsc->cf_max_pd) {
			START_TIMER(poll);
			continue;
		    }
		}
	    }
	    else {
		STOP_TIMER(dsc->timer_idle);
		START_TIMER(noresp);
	    }
	}
	/* A */
	INC24(dsc->vt_ps);
	send_pdu(dsc,SSCOP_POLL,NULL,0);
	dsc->vt_pd = 0;
	START_TIMER(poll);
    }
}


/* --- Incoming non-CC PDUs in Data Transfer Ready state ------------------- */


static void start_error_recov(SSCOP_DSC *dsc,char code)
{
    reset_data_transfer_timers(dsc);
    maa_error(dsc,code,0);
    /* D */
    dsc->vt_cc = 1;
    INC8(dsc->vt_sq);
    initialize_vr_mr(dsc);
    send_pdu(dsc,SSCOP_ER,NULL,0);
    prepare_recovery(dsc);
    START_TIMER(cc);
    next_state(dsc,sscop_outrec);
}


static void data_sd(SSCOP_DSC *dsc,int s,void *msg,int length) /* 20.38-39 */
{
    BUFFER *buf;

    if (NORM_RX(s) >= NORM_RX(dsc->vr_mr)) {
	if (NORM_RX(dsc->vr_h) < NORM_RX(dsc->vr_mr)) {
	    send_ustat(dsc,dsc->vr_h,dsc->vr_mr);
	    dsc->vr_h = dsc->vr_mr;
	}
	return;
    }
    if (s == dsc->vr_r) {
	if (dsc->ops->data_ind) dsc->ops->data_ind(dsc->user,msg,length,s);
	if (s == dsc->vr_h) {
	    dsc->vr_r = dsc->vr_h = MOD24(s+1);
	    update_vr_mr(dsc);
	    return;
	}
	while (1) {
	    INC24(dsc->vr_r);
	    update_vr_mr(dsc);
	    buf = queue_lookup(&dsc->rx_buf,dsc->vr_r);
	    if (!buf) break;
	    queue_remove(&dsc->rx_buf,buf);
	    if (dsc->ops->data_ind)
		dsc->ops->data_ind(dsc->user,buf->data,buf->length,buf->key);
	    buffer_discard(buf);
	}
	return;
    }
    buf = buffer_create(length,s);
    memcpy(buf->data,msg,length);
    if (s == dsc->vr_h) {
	queue_put(&dsc->rx_buf,buf);
	INC24(dsc->vr_h);
	return;
    }
    if (NORM_RX(dsc->vr_h) < NORM_RX(s)) {
	queue_put(&dsc->rx_buf,buf);
	send_ustat(dsc,dsc->vr_h,s);
	dsc->vr_h = MOD24(s+1);
	return;
    }
    if (!queue_lookup(&dsc->rx_buf,s)) queue_put(&dsc->rx_buf,buf);
    else {
	buffer_discard(buf);
	start_error_recov(dsc,'Q');
    }
}


/*
 * Some of the NORM_RXs in data_poll are certainly unnecessary. Maybe even
 * all of them could be removed ...
 */


static void data_poll(SSCOP_DSC *dsc,int s) /* 20.41-42 */
{
    int curr,i;

    if (!dsc->list) dsc->list = alloc(dsc->cf_max_stat*sizeof(uint32_t));
    if (NORM_RX(dsc->vr_h) > NORM_RX(s)) {
	start_error_recov(dsc,'Q');
	return;
    }
    dsc->vr_h = NORM_RX(dsc->vr_mr) < NORM_RX(s) ? dsc->vr_mr : s;
    /* K */
    curr = 0;
    i = dsc->vr_r;
    if (i != dsc->vr_h) {
	while (NORM_RX(i) < NORM_RX(dsc->vr_h)) {
	    if (queue_lookup(&dsc->rx_buf,i)) {
		INC24(i);
		continue;
	    }
	    dsc->list[curr++] = htonl(i);
	    if (curr >= dsc->cf_max_stat) {
		send_pdu(dsc,SSCOP_STAT,dsc->list,curr*4);
		curr = 0;
		dsc->list[curr++] = htonl(i);
	    }
	    do INC24(i);
	    while (i != dsc->vr_h && !queue_lookup(&dsc->rx_buf,i));
	    if (i == dsc->vr_h) break; /* append is done right b4 send_pdu */
	    dsc->list[curr++] = htonl(i);
	    INC24(i); /* short-cut, since i < VR(H) && SD.N(S) == i in RB */
	}
	dsc->list[curr++] = htonl(i);
    }
    send_pdu(dsc,SSCOP_STAT,dsc->list,curr*4);
}


static void data_ustat(SSCOP_DSC *dsc,int mr,int r,int e1,int e2) /* 20.43 */
{
    BUFFER *buf,*buf2;
    int seq1,seq2,i;

    if (NORM_TX(dsc->vt_a) <= NORM_TX(r) && NORM_TX(r) < NORM_TX(dsc->vt_s)) {
	for (i = dsc->vt_a; i != r; INC24(i)) {
	    buf = queue_lookup(&dsc->tx_buf,i);
	    if (buf) {
		queue_remove(&dsc->tx_buf,buf);
		buffer_discard(buf);
	    }
	}
	dsc->vt_a = r;
	dsc->vt_ms = mr;
	seq1 = e1;
	seq2 = e2;
	if (NORM_TX(dsc->vt_a) <= NORM_TX(seq1) && NORM_TX(seq1) <
	  NORM_TX(seq2) && NORM_TX(seq2) < NORM_TX(dsc->vt_s)) {
	    /* G */
	    while ((buf = queue_lookup(&dsc->tx_buf,seq1))) {
		buf2 = buffer_clone(buf);
		queue_put(&dsc->rt_q,buf2);
		INC24(seq1);
		if (seq1 == seq2) {
		    maa_error(dsc,'V',e2-e1);
		    return;
		}
	    }
	}
    }
    /* F */
    start_error_recov(dsc,'T');
}


static void data_stat(SSCOP_DSC *dsc,int ps,int mr,int r,void *list,
  int length) /* 20.44-46 */
{
    BUFFER *buf,*buf2;
    char *curr;
    int i,count,seq1,seq2;

    if (NEGATIVE(NORM_TXP(ps)) || NORM_TXP(ps) > NORM_TXP(dsc->vt_ps)) {
	start_error_recov(dsc,'R');
	return;
    }
    if (NORM_TX(dsc->vt_a) > NORM_TX(r) || NORM_TX(r) >
      NORM_TX(dsc->vt_s)) {
	/* H */
	start_error_recov(dsc,'S');
	return;
    }
    for (i = dsc->vt_a; i != r; INC24(i)) {
	buf = queue_lookup(&dsc->tx_buf,i);
	if (buf) {
	    queue_remove(&dsc->tx_buf,buf);
	    buffer_discard(buf);
	}
    }
    dsc->vt_a = r;
    dsc->vt_pa = ps;
    dsc->vt_ms = mr;
    i = length;
    curr = list;
    count = 0;
    if (i > 1) {
	seq1 = SSCOP_N(curr);
	curr += 4;
	i--;
	if (NORM_TX(seq1) >= NORM_TX(dsc->vt_s)) {
	    /* H */
	    start_error_recov(dsc,'S');
	    return;
	}
	/* I */
	do {
	    seq2 = SSCOP_N(curr);
	    curr += 4;
	    i--;
	    if (NORM_TX(seq1) >= NORM_TX(seq2) ||
	      NORM_TX(seq2) > NORM_TX(dsc->vt_s)) {
		/* H */
		start_error_recov(dsc,'S');
		return;
	    }
	    do {
		if (!(buf = queue_lookup(&dsc->tx_buf,seq1))) {
		    /* H */
		    start_error_recov(dsc,'S');
		    return;
		}
		if (NEGATIVE(NORM_TXP(buf->extra)) &&
		  NORM_TXP(ps) <= NORM_TXP(dsc->vt_ps) &&
		  !queue_lookup(&dsc->rt_q,seq1)) {
		    buf2 = buffer_clone(buf);
		    queue_put(&dsc->rt_q,buf2);
		    count++;
		}
		INC24(seq1);
	    }
	    while (seq1 != seq2);
	    /* J */
	    if (!i) break;
	    seq2 = SSCOP_N(curr);
	    curr += 4;
	    i--;
	    if (NORM_TX(seq1) >= NORM_TX(seq2) ||
	      NORM_TX(seq2) > NORM_TX(dsc->vt_s)) {
		/* H */
		start_error_recov(dsc,'S');
		return;
	    }
	    do {
		if (dsc->clear_buffers) {
		    buf = queue_lookup(&dsc->tx_buf,seq1);
		    if (buf) {
			queue_remove(&dsc->tx_buf,buf);
			buffer_discard(buf);
		    }
		}
		INC24(seq1);
	    }
	    while (seq1 != seq2);
	}
	while (i);
	maa_error(dsc,'V',count);
    }
    /* L */
    if (dsc->credit != (NORM_TX(dsc->vt_s) < NORM_TX(dsc->vt_ms)))
	maa_error(dsc,(dsc->credit = !dsc->credit) ? 'X' : 'W',0);
    if (dsc->timer_poll) START_TIMER(noresp);
    else if (!dsc->timer_idle) {
	    STOP_TIMER(dsc->timer_keepalive);
	    STOP_TIMER(dsc->timer_noresp);
	    START_TIMER(idle);
        }
}


/* --- Incoming PDUs ------------------------------------------------------- */


/*
 * Returns 0 if the descriptor might conceivably be gone when returning,
 * 1 otherwise.
 */
 
static int handle_sscop_pdu(SSCOP_DSC *dsc,void *msg,int size)
{
    PDU_VARS;

    if (DECOMPOSE_PDU(dsc,msg,size)) return 1;
    PRINT_PDU("RECV",msg);
    switch (type) {
	case SSCOP_UD:
	    if (dsc->ops->unitdata) dsc->ops->unitdata(dsc->user,msg,length);
	    return 1;
	case SSCOP_MD:
	    if (dsc->ops->maa_data) dsc->ops->maa_data(dsc->user,msg,length);
	    return 1;
	default:
	    break;
    }
    switch (dsc->state) {
	case sscop_idle:
	    switch (type) {
		case SSCOP_BGREJ:
		    bad_pdu(dsc,type);
		    return 1;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			send_pdu(dsc,SSCOP_BGREJ,NULL,0); /* no SSCOP-UU */
			return 1;
		    }
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->estab_ind)
			dsc->ops->estab_ind(dsc->user,msg,length);
		    return 1;
		case SSCOP_ENDAK:
		    return 1;
		case SSCOP_END:
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    return 1;
		case SSCOP_ER:
		case SSCOP_POLL:
		case SSCOP_SD:
		case SSCOP_BGAK:
		case SSCOP_ERAK:
		case SSCOP_STAT:
		case SSCOP_USTAT:
		case SSCOP_RS:
		case SSCOP_RSAK:
		    bad_pdu(dsc,type);
		    send_pdu(dsc,SSCOP_END,NULL,1);
		    return 1;
		default:
		    break;
	    }
	    break;
	case sscop_outconn: /* 20.8-10 */
	    switch (type) {
		case SSCOP_ENDAK:
		case SSCOP_SD:
		case SSCOP_ERAK:
		case SSCOP_END:
		case SSCOP_STAT:
		case SSCOP_USTAT:
		case SSCOP_POLL:
		case SSCOP_ER:
		case SSCOP_RSAK:
		case SSCOP_RS:
		    return 1; /* ignore PDU */
		case SSCOP_BGAK:
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    initialize_state_variables(dsc);
		    set_data_transfer_timers(dsc);
		    next_state(dsc,sscop_ready);
		    if (dsc->ops->estab_conf)
			dsc->ops->estab_conf(dsc->user,msg,length);
		    return 1;
		case SSCOP_BGREJ:
		    STOP_TIMER(dsc->timer_cc);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,1);
		    return 0;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) return 1;
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    initialize_vr_mr(dsc);
		    send_pdu(dsc,SSCOP_BGAK,NULL,0);
		    initialize_state_variables(dsc);
		    set_data_transfer_timers(dsc);
		    next_state(dsc,sscop_ready);
		    if (dsc->ops->estab_conf)
			dsc->ops->estab_conf(dsc->user,msg,length);
		    return 1;
		default:
		    break;
	    }
	    break;
	case sscop_inconn: /* 20.11-13 */
	    switch (type) {
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) return 1;
		    dsc->vt_ms = mr;
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		case SSCOP_ER:
		case SSCOP_BGAK:
		case SSCOP_ERAK:
		case SSCOP_RSAK:
		case SSCOP_RS:
		    bad_pdu(dsc,type);
		    return 1;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    bad_pdu(dsc,type);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_SD:
		case SSCOP_USTAT:
		case SSCOP_STAT:
		case SSCOP_POLL:
		    bad_pdu(dsc,type);
		    send_pdu(dsc,SSCOP_END,NULL,1);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_END:
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		default:
		    break;
	    }
	    break;
	case sscop_outdisc: /* 20.14-16 */
	    switch (type) {
		case SSCOP_SD:
		case SSCOP_BGAK:
		case SSCOP_POLL:
		case SSCOP_STAT:
		case SSCOP_USTAT:
		case SSCOP_ERAK:
		case SSCOP_RS:
		case SSCOP_RSAK:
		case SSCOP_ER:
		    return 1;
		case SSCOP_END:
		    STOP_TIMER(dsc->timer_cc);
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_conf) dsc->ops->rel_conf(dsc->user);
		    return 0;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    STOP_TIMER(dsc->timer_cc);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_conf) dsc->ops->rel_conf(dsc->user);
		    return 0;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			send_pdu(dsc,SSCOP_BGAK,NULL,0);
			resend_pdu(dsc,dsc->last_end);
			return 1;
		    }
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,0);
		    return 1;
	    }
	case sscop_outres: /* 20.17-19 */
	    switch (type) {
		case SSCOP_ER:
		case SSCOP_POLL:
		case SSCOP_STAT:
		case SSCOP_USTAT:
		case SSCOP_BGAK:
		case SSCOP_ERAK:
		case SSCOP_SD:
		    return 1; /* ignore */
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			send_pdu(dsc,SSCOP_BGAK,NULL,0);
			resend_pdu(dsc,dsc->last_rs);
			return 1;
		    }
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    STOP_TIMER(dsc->timer_cc);
		    bad_pdu(dsc,type);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_END:
		    STOP_TIMER(dsc->timer_cc);
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		case SSCOP_RS:
		    if (detect_retransmission(dsc,sq)) return 1;
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    initialize_vr_mr(dsc);
		    send_pdu(dsc,SSCOP_RSAK,NULL,0);
		    initialize_state_variables(dsc);
		    set_data_transfer_timers(dsc);
		    next_state(dsc,sscop_ready);
		    if (dsc->ops->res_conf) dsc->ops->res_conf(dsc->user);
		    return 1;
		case SSCOP_RSAK:
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    initialize_state_variables(dsc);
		    set_data_transfer_timers(dsc);
		    next_state(dsc,sscop_ready);
		    if (dsc->ops->res_conf) dsc->ops->res_conf(dsc->user);
		    return 1;
		default:
		    break;
	    }
	    break;
	case sscop_inres: /* 20.20-22 */
	    switch (type) {
		case SSCOP_SD:
		case SSCOP_POLL:
		case SSCOP_STAT:
		case SSCOP_USTAT:
		    send_pdu(dsc,SSCOP_END,NULL,1);
		    /* fall through */
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    bad_pdu(dsc,type);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_END:
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		case SSCOP_ER:
		case SSCOP_BGAK:
		case SSCOP_ERAK:
		case SSCOP_RSAK:
		    bad_pdu(dsc,type);
		    return 1;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		case SSCOP_RS:
		    if (!detect_retransmission(dsc,sq)) bad_pdu(dsc,type);
		    return 1;
		default:
		    break;
	    }
	    break;
	case sscop_outrec: /* 20.23-26 */
	    switch (type) {
		case SSCOP_BGAK:
		case SSCOP_RSAK:
		    bad_pdu(dsc,type);
		    return 1;
		case SSCOP_ERAK:
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_recresp);
		    deliver_data(dsc);
		    if (dsc->ops->rec_ind) dsc->ops->rec_ind(dsc->user);
		    return 1;
		case SSCOP_END:
		    STOP_TIMER(dsc->timer_cc);
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    clear_receive_buffer(dsc);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    bad_pdu(dsc,type);
		    STOP_TIMER(dsc->timer_cc);
		    clear_receive_buffer(dsc);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_STAT:
		case SSCOP_USTAT:
		case SSCOP_POLL:
		case SSCOP_SD:
		    return 1;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    clear_receive_buffer(dsc);
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		case SSCOP_ER:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    initialize_vr_mr(dsc);
		    send_pdu(dsc,SSCOP_ERAK,NULL,0);
		    deliver_data(dsc);
		    next_state(dsc,sscop_recresp);
		    if (dsc->ops->rec_ind) dsc->ops->rec_ind(dsc->user);
		    return 1;
		case SSCOP_RS:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    STOP_TIMER(dsc->timer_cc);
		    dsc->vt_ms = mr;
		    clear_receive_buffer(dsc);
		    next_state(dsc,sscop_inres);
		    if (dsc->ops->res_ind)
			dsc->ops->res_ind(dsc->user,msg,length);
		    return 0;
		default:
		    break;
	    }
	case sscop_recresp: /* 20.27-29 */
	    switch (type) {
		case SSCOP_BGAK:
		case SSCOP_RSAK:
		    bad_pdu(dsc,type);
		    return 1;
		case SSCOP_ERAK:
		case SSCOP_SD:
		case SSCOP_POLL:
		    return 1;
		case SSCOP_END:
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    bad_pdu(dsc,type);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_RS:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inres);
		    if (dsc->ops->res_ind)
			dsc->ops->res_ind(dsc->user,msg,length);
		    return 1;
		case SSCOP_ER:
		    if (!detect_retransmission(dsc,sq)) bad_pdu(dsc,type);
		    else send_pdu(dsc,SSCOP_ERAK,NULL,0);
		    return 1;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		case SSCOP_STAT:
		case SSCOP_USTAT:
		    bad_pdu(dsc,type);
		    send_pdu(dsc,SSCOP_END,NULL,1);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		default:
		    break;
	    }
	case sscop_inrec: /* 20.30-33 */
	    switch (type) {
		case SSCOP_END:
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    bad_pdu(dsc,type);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_USTAT:
		case SSCOP_STAT:
		case SSCOP_POLL:
		case SSCOP_SD:
		    bad_pdu(dsc,type);
		    send_pdu(dsc,SSCOP_END,NULL,1);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_RSAK:
		case SSCOP_BGAK:
		case SSCOP_ERAK:
		    bad_pdu(dsc,type);
		    return 1;
		case SSCOP_RS:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inres);
		    if (dsc->ops->res_ind)
			dsc->ops->res_ind(dsc->user,msg,length);
		    return 1;
		case SSCOP_ER:
		    if (!detect_retransmission(dsc,sq)) bad_pdu(dsc,type);
		    return 1;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			bad_pdu(dsc,type);
			return 1;
		    }
		    dsc->vt_ms = mr;
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		default:
		    break;
	    }
	case sscop_ready: /* 20.34-46 */
	    switch (type) {
		case SSCOP_BGAK:
		case SSCOP_ERAK:
		case SSCOP_RSAK:
		    return 1;
		case SSCOP_ER:
		    if (detect_retransmission(dsc,sq)) {
			START_TIMER(noresp);
			send_pdu(dsc,SSCOP_ERAK,NULL,0);
			return 1;
		    }
		    reset_data_transfer_timers(dsc);
		    dsc->vt_ms = mr;
		    prepare_recovery(dsc);
		    next_state(dsc,sscop_inrec);
		    deliver_data(dsc);
		    if (dsc->ops->rec_ind) dsc->ops->rec_ind(dsc->user);
		    return 1;
		case SSCOP_BGN:
		    if (detect_retransmission(dsc,sq)) {
			START_TIMER(noresp);
			send_pdu(dsc,SSCOP_BGAK,NULL,0);
			return 1;
		    }
		    reset_data_transfer_timers(dsc);
		    dsc->vt_ms = mr;
		    prepare_retrieval(dsc);
		    next_state(dsc,sscop_inconn);
		    if (dsc->ops->restart)
			dsc->ops->restart(dsc->user,msg,length,1);
		    return 1;
		case SSCOP_ENDAK:
		case SSCOP_BGREJ:
		    reset_data_transfer_timers(dsc);
		    bad_pdu(dsc,type);
		    prepare_retrieval(dsc);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,NULL,0,0);
		    return 0;
		case SSCOP_RS:
		    if (detect_retransmission(dsc,sq)) {
			START_TIMER(noresp);
			send_pdu(dsc,SSCOP_RSAK,NULL,0);
			return 1;
		    }
		    reset_data_transfer_timers(dsc);
		    dsc->vt_ms = mr;
		    prepare_retrieval(dsc);
		    next_state(dsc,sscop_inres);
		    if (dsc->ops->res_ind)
			dsc->ops->res_ind(dsc->user,msg,length);
		    return 1;
		case SSCOP_END:
		    reset_data_transfer_timers(dsc);
		    send_pdu(dsc,SSCOP_ENDAK,NULL,0);
		    prepare_retrieval(dsc);
		    next_state(dsc,sscop_idle);
		    if (dsc->ops->rel_ind)
			dsc->ops->rel_ind(dsc->user,msg,length,!s);
		    return 0;
		case SSCOP_SD:
		    data_sd(dsc,s,msg,length);
		    return 1;
		case SSCOP_POLL:
		    dsc->vr_ps = ps; /* store */
		    data_poll(dsc,s);
		    return 1;
		case SSCOP_USTAT:
		    data_ustat(dsc,mr,r,SSCOP_N(msg),SSCOP_N((char *) msg+4));
		    return 1;
		case SSCOP_STAT:
		    data_stat(dsc,ps,mr,r,msg,length/4);
		    return 1;
		default:
		    break;
	    }
    }
    return 1;
}


void sscop_pdu(SSCOP_DSC *dsc,void *msg,int size)
{
    if (handle_sscop_pdu(dsc,msg,size))
	if (dsc->state == sscop_ready) try_to_send(dsc);
}


/* --- From SSCOP user ----------------------------------------------------- */


void sscop_retrieve(SSCOP_DSC *dsc,int rn)
{
    BUFFER *buf;
    int i;

    if (dsc->state != sscop_idle && dsc->state != sscop_inconn &&
      dsc->state != sscop_outdisc && dsc->state != sscop_inres &&
      dsc->state != sscop_recresp && dsc->state != sscop_inrec)
	diag(COMPONENT,DIAG_FATAL,"sscop_retrieve invoked in state %s",
	  state_name[dsc->state]);
    if (rn != SSCOP_RN_UNKNOWN)
	for (i = rn == SSCOP_RN_TOTAL ? dsc->vt_a : MOD24(rn+1);
	  NORM_TX(dsc->vt_a) <= NORM_TX(i) && NORM_TX(i) < NORM_TX(dsc->vt_s);
	  INC24(i)) {
	    buf = queue_lookup(&dsc->tx_buf,i);
	    if (buf) {
		queue_remove(&dsc->tx_buf,buf);
		if (dsc->ops->retr_ind)
		    dsc->ops->retr_ind(dsc->user,buf->data,sdu_length(buf));
		buffer_discard(buf);
	    }
	}
    while ((buf = queue_get(&dsc->tx_q))) {
	if (dsc->ops->retr_ind)
	    dsc->ops->retr_ind(dsc->user,buf->data,sdu_length(buf));
	buffer_discard(buf);
    }
    if (dsc->ops->retr_comp) dsc->ops->retr_comp(dsc->user);
}


void sscop_send(SSCOP_DSC *dsc,void *buffer,int size) /* 20.38 */
{
    BUFFER *buf;

    if (dsc->state != sscop_ready && (dsc->state != sscop_outrec ||
      dsc->clear_buffers)) return; /* 20.23 */
    buf = buffer_create(size,0);
    memcpy(buf->data,buffer,size);
    queue_put(&dsc->tx_q,buf);
    try_to_send(dsc);
}


void sscop_estab_req(SSCOP_DSC *dsc,void *uu_data,int uu_length,int buf_rel)
{
    switch (dsc->state) {
	case sscop_outdisc: /* 20.14 */
	    STOP_TIMER(dsc->timer_cc);
	    /* fall through */
	case sscop_idle: /* 20.5 */
	    clear_transmitter(dsc);
	    dsc->clear_buffers = buf_rel;
	    dsc->vt_cc = 1;
	    INC8(dsc->vt_sq);
	    initialize_vr_mr(dsc);
	    send_pdu(dsc,SSCOP_BGN,uu_data,uu_length);
	    START_TIMER(cc);
	    next_state(dsc,sscop_outconn);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"sscop_estab_req invoked in state %s",
      state_name[dsc->state]);
}


void sscop_estab_resp(SSCOP_DSC *dsc,void *uu_data,int uu_length,int buf_rel)
    /* 20.11 */
{
    if (dsc->state != sscop_inconn)
	diag(COMPONENT,DIAG_FATAL,"sscop_estab_resp invoked in state %s",
	  state_name[dsc->state]);
    clear_transmitter(dsc);
    dsc->clear_buffers = buf_rel;
    initialize_vr_mr(dsc);
    send_pdu(dsc,SSCOP_BGAK,uu_data,uu_length);
    initialize_state_variables(dsc);
    set_data_transfer_timers(dsc);
    next_state(dsc,sscop_ready);
    try_to_send(dsc); /* probably not ... */
}


void sscop_rel_req(SSCOP_DSC *dsc,void *uu_data,int uu_length)
{
    switch (dsc->state) {
	case sscop_outrec: /* 20.24 */
	    clear_receive_buffer(dsc);
	    /* fall through */
	case sscop_outconn: /* 20.9 */
	case sscop_outres: /* 20.18 */
	    STOP_TIMER(dsc->timer_cc);
	    /* fall through */
	case sscop_inres: /* 20.20 */
	case sscop_recresp: /* 20.28 */
	case sscop_inrec: /* 20.30 */
	    dsc->vt_cc = 1;
	    send_pdu(dsc,SSCOP_END,uu_data,uu_length);
	    START_TIMER(cc);
	    next_state(dsc,sscop_outdisc);
	    return;
	case sscop_inconn: /* 20.11 */
	    send_pdu(dsc,SSCOP_BGREJ,uu_data,uu_length);
	    next_state(dsc,sscop_idle);
	    return;
	case sscop_ready: /* 20.34 */
	    reset_data_transfer_timers(dsc);
	    dsc->vt_cc = 1;
	    send_pdu(dsc,SSCOP_END,uu_data,uu_length);
	    prepare_retrieval(dsc);
	    START_TIMER(cc);
	    next_state(dsc,sscop_outdisc);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"sscop_rel_req invoked in state %s",
      state_name[dsc->state]);
}


void sscop_res_req(SSCOP_DSC *dsc,void *uu_data,int uu_length)
{
    switch (dsc->state) {
	case sscop_outrec: /* 20.25 */
	    STOP_TIMER(dsc->timer_cc);
	    clear_receive_buffer(dsc);
	    /* fall through */
	case sscop_recresp: /* 20.29 */
	    dsc->vt_cc = 1;
	    INC8(dsc->vt_sq);
	    initialize_vr_mr(dsc);
	    send_pdu(dsc,SSCOP_RS,uu_data,uu_length);
	    clear_transmitter(dsc);
	    START_TIMER(cc);
	    next_state(dsc,sscop_outres);
	    return;
	case sscop_inrec: /* 20.30 */
	    clear_transmitter(dsc);
	    dsc->vt_cc = 1;
	    INC8(dsc->vt_sq);
	    initialize_vr_mr(dsc);
	    send_pdu(dsc,SSCOP_RS,uu_data,uu_length);
	    START_TIMER(cc);
	    next_state(dsc,sscop_outres);
	    return;
	case sscop_ready: /* 20.34 */
	    reset_data_transfer_timers(dsc);
	    dsc->vt_cc = 1;
	    INC8(dsc->vt_sq);
	    initialize_vr_mr(dsc);
	    send_pdu(dsc,SSCOP_RS,uu_data,uu_length);
	    release_buffers(dsc);
	    START_TIMER(cc);
	    next_state(dsc,sscop_outres);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"sscop_res_req invoked in state %s",
      state_name[dsc->state]);
}


void sscop_res_resp(SSCOP_DSC *dsc) /* 20.20 */
{
    if (dsc->state != sscop_inres)
	diag(COMPONENT,DIAG_FATAL,"sscop_res_resp invoked in state %s",
	  state_name[dsc->state]);
    initialize_vr_mr(dsc);
    send_pdu(dsc,SSCOP_RSAK,NULL,0);
    clear_transmitter(dsc);
    initialize_state_variables(dsc);
    set_data_transfer_timers(dsc);
    next_state(dsc,sscop_ready);
    try_to_send(dsc); /* probably not ... */
}


void sscop_rec_resp(SSCOP_DSC *dsc)
{
    switch (dsc->state) {
	case sscop_inrec: /* 20.30 */
	    initialize_vr_mr(dsc);
	    send_pdu(dsc,SSCOP_ERAK,NULL,0);
	    /* fall through */
	case sscop_recresp: /* 20.28 */
	    if (!dsc->clear_buffers) clear_transmission_buffer(dsc);
	    initialize_state_variables(dsc);
	    set_data_transfer_timers(dsc);
	    next_state(dsc,sscop_ready);
	    try_to_send(dsc);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"sscop_rec_resp invoked in state %s",
      state_name[dsc->state]);
}


void sscop_unitdata(SSCOP_DSC *dsc,void *buffer,int size) /* 20.47-48 */
{
    send_pdu(dsc,SSCOP_UD,buffer,size);
}


void sscop_maa_data(SSCOP_DSC *dsc,void *buffer,int size) /* 20.47-48 */
{
    send_pdu(dsc,SSCOP_MD,buffer,size);
}


void start_sscop(SSCOP_DSC *dsc,SSCOP_USER_OPS *ops,void *user_data,
  SSCOP_MODE mode)
{
    static int initialize = 1;

    if (initialize) {
	pdu_maa = maa_error;
	pdu_diag = do_diag;
	initialize = 0;
    }
    dsc->ops = ops;
    dsc->user = user_data;
    dsc->mode = mode;
    dsc->vt_sq = dsc->vr_sq = 0; /* 20.5 */
    dsc->clear_buffers = 1;
    dsc->state = sscop_idle;
    dsc->timer_cc = dsc->timer_poll = dsc->timer_noresp =
      dsc->timer_keepalive = dsc->timer_idle = NULL;
    queue_init(&dsc->rx_buf);
    queue_init(&dsc->tx_q);
    queue_init(&dsc->tx_buf);
    queue_init(&dsc->rt_q);
    dsc->last_bgn = dsc->last_end = dsc->last_rs = dsc->last_er = NULL;
    dsc->cf_max_cc = SSCOP_CF_MaxCC;
    dsc->cf_max_pd = SSCOP_CF_MaxPD;
    dsc->cf_max_stat = SSCOP_CF_MaxSTAT;
    dsc->cf_timer_cc = TIMER_CC;
    dsc->cf_timer_poll = TIMER_POLL;
    dsc->cf_timer_noresp = TIMER_NORESP;
    dsc->cf_timer_keepalive = TIMER_KEEPALIVE;
    dsc->cf_timer_idle = TIMER_IDLE;
    dsc->list = NULL;
}


void stop_sscop(SSCOP_DSC *dsc)
{
    if (dsc->state != sscop_idle)
	diag(COMPONENT,DIAG_WARN,"stopping dsc in state %s",
	  state_name[dsc->state]);
    dsc->state = sscop_idle; /* avoid send attempts */
    STOP_TIMER(dsc->timer_cc);
    STOP_TIMER(dsc->timer_poll);
    STOP_TIMER(dsc->timer_noresp);
    STOP_TIMER(dsc->timer_keepalive);
    STOP_TIMER(dsc->timer_idle);
    queue_clear(&dsc->rx_buf);
    queue_clear(&dsc->tx_q);
    queue_clear(&dsc->tx_buf);
    queue_clear(&dsc->rt_q);
    if (dsc->last_bgn) buffer_discard(dsc->last_bgn);
    if (dsc->last_end) buffer_discard(dsc->last_end);
    if (dsc->last_rs) buffer_discard(dsc->last_rs);
    if (dsc->last_er) buffer_discard(dsc->last_er);
    if (dsc->list) free(dsc->list);
}
