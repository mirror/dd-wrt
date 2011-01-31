/* sscf.c - SSCF (Q.2130) protocol */

/* Written 1995,1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "atmd.h"

#include "sscop.h"
#include "sscf.h"


#define COMPONENT "SSCF"


/* --- SSCOP configuration ------------------------------------------------- */


#define SSCF_MaxCC		      4	/* give up after 4 retries */
#define SSCF_MaxPD		     25	/* POLL after 25 SDs */
#define SSCF_Timer_CC		1000000 /* 1 sec */
#define SSCF_Timer_KEEPALIVE	2000000	/* 2 sec */
#define SSCF_Timer_NORESP	7000000 /* 7 sec */
#define SSCF_Timer_POLL		 750000 /* 750 ms */
#define SSCF_Timer_IDLE	       15000000 /* 15 sec */


static const char *state_name[] = { "1/2","2/2","4/10","3/4","2/5" };


/* --- Helper function(s) -------------------------------------------------- */


static void next_state(SSCF_DSC *dsc,SSCF_STATE state)
{
    diag(COMPONENT,DIAG_DEBUG,"entering state %s",state_name[state]);
    dsc->state = state;
}


/* --- Invocation from SSCOP ----------------------------------------------- */


static void sscf_estab_ind(void *user_data,void *uu_data,int uu_length)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_11)
	diag(COMPONENT,DIAG_FATAL,"sscf_estab_ind in state %s",
	  state_name[dsc->state]);
    next_state(dsc,sscf_410);
    sscop_estab_resp(&dsc->sscop,NULL,0,1);
    if (dsc->ops->estab_ind) dsc->ops->estab_ind(dsc->user,uu_data,uu_length);
}


static void sscf_estab_conf(void *user_data,
  void *uu_data,int uu_length)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_22)
	diag(COMPONENT,DIAG_FATAL,"sscf_estab_conf in state %s",
	  state_name[dsc->state]);
    next_state(dsc,sscf_410);
    if (dsc->ops->estab_conf)
	dsc->ops->estab_conf(dsc->user,uu_data,uu_length);
    
}


static void sscf_restart(void *user_data,void *uu_data,int uu_length,int ind)
{
    SSCF_DSC *dsc = user_data;

    if ((!ind && dsc->state != sscf_34) || (ind && (dsc->state == sscf_11 ||
      dsc->state == sscf_34)))
	diag(COMPONENT,DIAG_FATAL,"sscf_restart (ind = %d) in state %s",
	  state_name[dsc->state],ind);
    sscop_estab_resp(&dsc->sscop,NULL,0,1);
    next_state(dsc,sscf_410);
    if (dsc->ops->restart) dsc->ops->restart(dsc->user,uu_data,uu_length,ind);
}


static void sscf_rec_ind(void *user_data)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_410)
	diag(COMPONENT,DIAG_FATAL,"sscf_rec_ind in state %s",
	  state_name[dsc->state]);
    sscop_rec_resp(&dsc->sscop);
    if (dsc->ops->estab_ind) dsc->ops->estab_ind(dsc->user,NULL,0);
}


static void sscf_rel_ind(void *user_data,void *uu_data,int uu_length,int user)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state == sscf_11 || dsc->state == sscf_34)
	diag(COMPONENT,DIAG_FATAL,"sscf_rel_ind in state %s",
	  state_name[dsc->state]);
    next_state(dsc,sscf_11);
    if (dsc->ops->rel_ind)
	dsc->ops->rel_ind(dsc->user,user ? uu_data : NULL,uu_length);
}


static void sscf_rel_conf(void *user_data)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_34)
	diag(COMPONENT,DIAG_FATAL,"sscf_rel_conf in state %s",
	  state_name[dsc->state]);
    next_state(dsc,sscf_11);
    if (dsc->ops->rel_conf) dsc->ops->rel_conf(dsc->user);
}


static void sscf_data_ind(void *user_data,void *data,int length,int sn)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_410)
	diag(COMPONENT,DIAG_FATAL,"sscf_data_ind in state %s",
	  state_name[dsc->state]);
    if (dsc->ops->data_ind) dsc->ops->data_ind(dsc->user,data,length);
    
}


static void sscf_res_ind(void *user_data,void *uu_data,int uu_length)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_410)
	diag(COMPONENT,DIAG_FATAL,"sscf_res_ind in state %s",
	  state_name[dsc->state]);
    sscop_res_resp(&dsc->sscop);
    if (dsc->ops->estab_ind) dsc->ops->estab_ind(dsc->user,uu_data,uu_length);
}


static void sscf_res_conf(void *user_data)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->state != sscf_25)
	diag(COMPONENT,DIAG_FATAL,"sscf_res_conf in state %s",
	  state_name[dsc->state]);
    next_state(dsc,sscf_410);
    if (dsc->ops->estab_conf) dsc->ops->estab_conf(dsc->user,NULL,0);
}


static void sscf_unitdata_ind(void *user_data,void *data,int length)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->ops->unitdata) dsc->ops->unitdata(dsc->user,data,length);
}


static void sscf_cpcs_send(void *user_data,void *data,int length)
{
    SSCF_DSC *dsc = user_data;

    if (dsc->ops->cpcs_send)
	dsc->ops->cpcs_send(dsc->user,data,length);
}


static SSCOP_USER_OPS sscf_ops = {
   sscf_estab_ind, *
   sscf_estab_conf, *
   sscf_rel_ind, *
   sscf_rel_conf, *
   sscf_restart,
   sscf_res_ind,
   sscf_res_conf,
   sscf_rec_ind,
   sscf_data_ind,
   sscf_unitdata_ind,
   NULL, /* no retr_ind */
   NULL, /* no retr_comp */
   NULL, /* no maa_data */
   NULL, /* no maa_error */
   sscf_cpcs_send
};


/* --- Invocation from user ------------------------------------------------ */


void start_sscf(SSCF_DSC *dsc,SSCF_USER_OPS *ops,void *user_data,
  SSCOP_MODE mode)
{
    dsc->state = sscf_11;
    dsc->ops = ops;
    dsc->user = user_data;
    start_sscop(&dsc->sscop,&sscf_ops,dsc,mode);
    dsc->sscop.cf_max_cc = SSCF_MaxCC;
    dsc->sscop.cf_max_pd = SSCF_MaxPD;
    dsc->sscop.cf_timer_cc = SSCF_Timer_CC;
    dsc->sscop.cf_timer_poll = SSCF_Timer_POLL;
    dsc->sscop.cf_timer_noresp = SSCF_Timer_NORESP;
    dsc->sscop.cf_timer_keepalive = SSCF_Timer_KEEPALIVE;
    dsc->sscop.cf_timer_idle = SSCF_Timer_IDLE;
}


void stop_sscf(SSCF_DSC *dsc)
{
    stop_sscop(&dsc->sscop);
}


void sscf_estab_req(SSCF_DSC *dsc,void *uu_data,int uu_length)
{
    switch (dsc->state) {
	case sscf_11:
	case sscf_34:
	    next_state(dsc,sscf_22);
	    sscop_estab_req(&dsc->sscop,uu_data,uu_length,1);
	    return;
	case sscf_410:
	    next_state(dsc,sscf_25);
	    sscop_res_req(&dsc->sscop,uu_data,uu_length);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"sscf_estab_req invoked in state %s",
      state_name[dsc->state]);
}


void sscf_rel_req(SSCF_DSC *dsc,void *uu_data,int uu_length)
{
    switch (dsc->state) {
	case sscf_11:
	    if (dsc->ops->rel_conf) dsc->ops->rel_conf(dsc->user);
	    return;
	case sscf_22:
	case sscf_410:
	case sscf_25:
	    next_state(dsc,sscf_34);
	    sscop_rel_req(&dsc->sscop,uu_data,uu_length);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_FATAL,"sscf_rel_req invoked in state %s",
      state_name[dsc->state]);
}


void sscf_send(SSCF_DSC *dsc,void *data,int length)
{
    switch (dsc->state) {
	case sscf_11:
	    return;
	case sscf_410:
	    sscop_send(&dsc->sscop,data,length);
	    return;
	default:
	    break;
    }
    diag(COMPONENT,DIAG_WARN,"sscf_send invoked in state %s",
      state_name[dsc->state]); /* make fatal later @@@ */
}


void sscf_unitdata(SSCF_DSC *dsc,void *data,int length)
{
    sscop_unitdata(&dsc->sscop,data,length);
}
