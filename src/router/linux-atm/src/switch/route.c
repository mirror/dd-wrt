/* route.c - ATM switch routing database */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>

#include "atm.h"
#include "atmd.h"
#include "route.h"


#define COMPONENT "ROUTE"


typedef struct _route {
    struct sockaddr_atmsvc addr;
    int mask;
    SIGNALING_ENTITY *sig;
    struct _route *next;
} ROUTE;


static ROUTE *routes = NULL;


void put_route(struct sockaddr_atmsvc *addr,int addr_mask,SIGNALING_ENTITY *sig)
{
    ROUTE *route;

    for (route = routes; route; route = route->next)
	if (route->mask == addr_mask &&
	  (!addr_mask || atm_equal((struct sockaddr *) addr,
	  (struct sockaddr *) &route->addr,addr_mask,
	  AXE_PRVOPT | (addr_mask == INT_MAX ? 0 : AXE_WILDCARD))))
	    diag(COMPONENT,DIAG_FATAL,"duplicate route");
    route = alloc_t(ROUTE);
    if (addr) route->addr = *addr;
    route->mask = addr_mask;
    route->sig = sig;
    route->next = routes;
    routes = route;
}


void get_routes(SIGNALING_ENTITY *sig,
  void (*callback)(struct sockaddr_atmsvc *addr,int addr_mask,void *user),
  void *user)
{
    ROUTE *route;

    for (route = routes; route; route = route->next)
	if (route->sig == sig) callback(&route->addr,route->mask,user);
}


static SIGNALING_ENTITY *dfl_find_route(struct sockaddr_atmsvc *from,
  struct sockaddr_atmsvc *to,struct atm_qos *qos)
{
    ROUTE *best,*route;
    int best_len;

    best = NULL;
    best_len = -1;
    for (route = routes; route; route = route->next)
	if (route->mask > best_len && (!route->mask ||
	  atm_equal((struct sockaddr *) to,(struct sockaddr *) &route->addr,
	  route->mask,
	  AXE_PRVOPT | (route->mask == INT_MAX ? 0 : AXE_WILDCARD)))) {
	    if (route->mask == INT_MAX) return route->sig;
	    best_len = route->mask;
	    best = route;
	}
    return best->sig;
}


static void dfl_route_sig(SIGNALING_ENTITY *sig,struct sockaddr_atmpvc *pvc,
  int up)
{
    /* do nothing */
}


SIGNALING_ENTITY *(*find_route)(struct sockaddr_atmsvc *from,
  struct sockaddr_atmsvc *to,struct atm_qos *qos) = &dfl_find_route;
void (*route_sig)(SIGNALING_ENTITY *sig,struct sockaddr_atmpvc *pvc,int up) =
  &dfl_route_sig;
