/* sscf.h - SSCF (Q.2130) user interface */

/* Written 1995,1998 by Werner Almesberger, EPFL-LRC/ICA */


#ifndef SSCF_H
#define SSCF_H

#include "sscop.h"


typedef enum { sscf_11,sscf_22,sscf_410,sscf_34,sscf_25 } SSCF_STATE;

typedef struct {
    SSCF_STATE state;
    struct _sscf_user_ops *ops;
    void *user;
    SSCOP_DSC sscop;
} SSCF_DSC;


typedef struct _sscf_user_ops {
    void (*estab_ind)(void *user_data,void *uu_data,int uu_length);
	/* AAL-ESTABLISH.indication */
    void (*estab_conf)(void *user_data,void *uu_data,int uu_length);
	/* AAL-ESTABLISH.confirm */
    void (*rel_ind)(void *user_data,void *uu_data,int uu_length);
	/* AAL-RELEASE.indication */
    void (*rel_conf)(void *user_data); /* AAL-RELEASE.confirm */
    void (*restart)(void *user_data,void *uu_data,int uu_length,int ind);
	/* AAL-RELEASE.indication or AAL-RELEASE.confirm immediately followed
	   by AAL-ESTABLISH.indication */
    void (*data_ind)(void *user_data,void *data,int length);
	/* AAL-DATA.indication */
    void (*unitdata)(void *user_data,void *data,int length);
	/* AAL-UNITDATA.indication */
    void (*cpcs_send)(void *user_data,void *data,int length);
} SSCF_USER_OPS;


/* Attach/detach protocol */

void start_sscf(SSCF_DSC *dsc,SSCF_USER_OPS *ops,void *user_data,
  SSCOP_MODE _mode); /* gcc 2.7.2 firmly believes "mode" can shadow ... @%! */
void stop_sscf(SSCF_DSC *dsc);

/* Connection control */

void sscf_estab_req(SSCF_DSC *dsc,void *uu_data,int uu_length);
void sscf_rel_req(SSCF_DSC *dsc,void *uu_data,int uu_length);

/* Send data */

void sscf_send(SSCF_DSC *dsc,void *data,int length);
void sscf_unitdata(SSCF_DSC *dsc,void *data,int length);

#endif
