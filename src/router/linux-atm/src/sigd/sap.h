/* sap.h - SAP manipulations */

/* Written 1996-2000 by Werner Almesberger, EPFL-LRC/ICA */


#ifndef SAP_H
#define SAP_H

#include <linux/atm.h>
#include <linux/atmsvc.h>

#include "atmsap.h"


#define IE_PROBLEM(cause,ie) (((cause) << 8) | (ie))
#define IE_PB_CAUSE(problem) ((problem) >> 8)
#define IE_PB_IE(problem) ((problem) & 0xff)


int sap_compat(const struct sockaddr_atmsvc *old_addr,
  const struct sockaddr_atmsvc *new_addr,struct sockaddr_atmsvc *res_addr,
  const struct atm_sap *old_sap,const struct atm_sap *new_sap,
  struct atm_sap *res_sap,const struct atm_qos *old_qos,
  const struct atm_qos *new_qos,struct atm_qos *res_qos);
int sap_encode(Q_DSC *dsc,const struct sockaddr_atmsvc *addr,
  const struct atm_sap *sap,const struct atm_qos *qos,int uni,int max_rate);

/*
 * sap_encode returns zero on success, -errno on error.
 */

unsigned int sap_decode(Q_DSC *dsc,struct sockaddr_atmsvc *addr,
  struct atm_sap *sap,struct atm_qos *qos,int uni);

/*
 * sap_decode returns zero on success, the problem report (encoded with
 * IE_PROBLEM) on error.
 */

#endif
