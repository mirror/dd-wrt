/* route.h - ATM switch routing database */

/* Written 1998 by Werner Almesberger, EPFL ICA */


#ifndef ROUTE_H
#define ROUTE_H

#include "atm.h"
#include "sig.h"


/*
 * PUT_ROUTE is invoked during configuration time to add static routes.
 */

void put_route(struct sockaddr_atmsvc *addr,int addr_mask,
  SIGNALING_ENTITY *sig);

/*
 * GET_ROUTES can be invoked to obtain static routes of a signaling entity. A
 * non-default routing mechanism may call GET_ROUTES from ROUTE_SIG to retrieve
 * pre-configured static routes.
 */

void get_routes(SIGNALING_ENTITY *sig,
  void (*callback)(struct sockaddr_atmsvc *addr,int addr_mask,void *user),
  void *user);

/*
 * FIND_ROUTE obtains the signaling entity for calls from FROM to TO with the
 * QoS QOS. The default implementation only considers FROM for its routing
 * decisions.
 */

extern SIGNALING_ENTITY *(*find_route)(struct sockaddr_atmsvc *from,
  struct sockaddr_atmsvc *to,struct atm_qos *qos);

/*
 * ROUTE_SIG is invoked whenever a signaling entity becomes operational
 * (UP != 0) or when it is shut down (UP == 0). This can be used to initate
 * routing protocol activities.
 */

extern void (*route_sig)(SIGNALING_ENTITY *sig,struct sockaddr_atmpvc *pvc,
  int up);

#endif
