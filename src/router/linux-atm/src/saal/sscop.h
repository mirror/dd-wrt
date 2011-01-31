/* sscop.h - SSCOP (Q.2110) user interface */
 
/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef SSCOP_H
#define SSCOP_H

#include <stdint.h>

#include "atmd.h"

#include "queue.h"

typedef enum {
    sscop_qsaal1,	/* emulate some properties of Q.SAAL1 as specified for
			   UNI 3.0 */
    sscop_q2110		/* follow Q.2110 by the letter */
} SSCOP_MODE;

typedef enum { sscop_idle,sscop_outconn,sscop_inconn,sscop_outdisc,
  sscop_outres,sscop_inres,sscop_outrec,sscop_recresp,sscop_inrec,
  sscop_ready } SSCOP_STATE;

typedef struct {
    SSCOP_STATE state;
    struct _sscop_user_ops *ops;
    void *user;
    /* --- Configurable parameters ----------------------------------------- */
    SSCOP_MODE mode;
    int cf_max_cc,cf_max_pd,cf_max_stat;
    int cf_timer_cc,cf_timer_poll,cf_timer_noresp,cf_timer_keepalive,
      cf_timer_idle;
    /* --- SSCOP information, TX part -------------------------------------- */
    int vt_s; /* send sequence number */
    int vt_ps; /* poll send sequence number */
    int vt_a; /* acknowledge sequence number */
    int vt_pa; /* poll sequence number */
    int vt_ms; /* maximum send sequence */
    int vt_pd; /* SD PDUs between POLL PDUs */
    int vt_cc; /* number of unacknowledged BSG, END, ER or RS PDUs */
    int vt_sq; /* connection number */
    /* --- SSCOP information, RX part -------------------------------------- */
    int vr_r; /* receiver sequence number */
    int vr_h; /* highest expected SD PDU */
    int vr_mr; /* maximum acceptable (receiver) */
    int vr_sq; /* connection number */
    int vr_ps; /* non-Q.2110: keeps N(PS) received in last POLL for STAT */
    /* Other variables */
    int clear_buffers;
    int credit;
    /* Timers */
    TIMER *timer_cc,*timer_poll,*timer_noresp,*timer_keepalive,*timer_idle;
    /* Queues and buffers */
    QUEUE tx_buf,tx_q,rx_buf,rt_q;
    BUFFER *last_bgn,*last_end,*last_rs,*last_er; /* for retransmission */
    /* Misc items */
    uint32_t *list; /* STAT construction list */
} SSCOP_DSC;


/*
 * Note: UU data of primitives carrying such is only available if
 *  - the "user" flag is set (if available) and
 *  - uu_data is non-NULL, and
 *  - uu_length is non-zero
 * in all other cases, uu_data must not be dereferenced.
 *
 * Note: the "ind" parameter in restart indicates whether the release primitive
 * is an AA-RELEASE.indication (ind = 1) or to an AA-RELEASE.confirm (ind = 0).
 */


typedef struct _sscop_user_ops {
    void (*estab_ind)(void *user_data,void *uu_data,int uu_length);
	/* AA-ESTABLISH.indication */
    void (*estab_conf)(void *user_data,void *uu_data,int uu_length);
	/* AA-ESTABLISH.confirm */
    void (*rel_ind)(void *user_data,void *uu_data,int uu_length,int user);
	/* AA-RELEASE.indication */
    void (*rel_conf)(void *user_data); /* AA-RELEASE.confirm */
    void (*restart)(void *user_data,void *uu_data,int uu_length,int ind);
	/* AA-RELEASE.indication or AA-RELEASE.confirm immediately followed by
	   AA-ESTABLISH.indication */
    void (*res_ind)(void *user_data,void *uu_data,int uu_length);
	/* AA-RESYNC.indication */
    void (*res_conf)(void *user_data); /* AA-RESYNC.confirm */
    void (*rec_ind)(void *user_data); /* AA-RECOVER.indication */
    void (*data_ind)(void *user_data,void *data,int length,int sn);
	/* AA-DATA.indication */
    void (*unitdata)(void *user_data,void *data,int length);
	/* AA-UNITDATA.indication */
    void (*retr_ind)(void *user_data,void *data,int length);
	/* AA-RETRIEVE.indication */
    void (*retr_comp)(void *user_data); /* AA-RETRIEVE_COMPLETE.indication */
    void (*maa_data)(void *user_data,void *data,int length);
	/* MAA-UNITDATA.indication */
    int (*maa_error)(void *user_data,char code,int count);
	/* MAA-ERROR.indication */
    void (*cpcs_send)(void *user_data,void *data,int length);
} SSCOP_USER_OPS;


/* Attach/detach protocol */

void start_sscop(SSCOP_DSC *dsc,SSCOP_USER_OPS *ops,void *user_data,
  SSCOP_MODE _mode); /* gcc 2.7.2 firmly believes "mode" can shadow ... @%! */
void stop_sscop(SSCOP_DSC *dsc);

/* Connection control */

void sscop_estab_req(SSCOP_DSC *dsc,void *uu_data,int uu_length,int buf_rel);
    /* AA-ESTABLISH.request */
void sscop_estab_resp(SSCOP_DSC *dsc,void *uu_data,int uu_length,int buf_rel);
    /* AA-ESTABLISH.response */
void sscop_rel_req(SSCOP_DSC *dsc,void *uu_data,int uu_length);
    /* AA-RELEASE.request */
void sscop_res_req(SSCOP_DSC *dsc,void *uu_data,int uu_length);
    /* AA-RESYNC.request */
void sscop_res_resp(SSCOP_DSC *dsc); /* AA-RESYNC.response */
void sscop_rec_resp(SSCOP_DSC *dsc); /* AA-RECOVER.response */

/* Incoming PDU from lower layer */

void sscop_pdu(SSCOP_DSC *dsc,void *msg,int size); /* CPCS-UNITDATA.request */

/* Send data */

void sscop_send(SSCOP_DSC *dsc,void *buffer,int size);
    /* AA-DATA.request, 20.38 */
void sscop_unitdata(SSCOP_DSC *dsc,void *buffer,int size);
    /* AA-UNIDATA.request, 20.47 */
void sscop_maa_data(SSCOP_DSC *dsc,void *buffer,int size);
    /* MAA-UNITDATA.request, 20.47 */

/* Retrieve unsent data */

#define SSCOP_RN_UNKNOWN -2 /* -1 is (theoretically) a valid rn ... */
#define SSCOP_RN_TOTAL   -3

void sscop_retrieve(SSCOP_DSC *dsc,int rn); /* AA-RETRIEVE.request */

#endif
