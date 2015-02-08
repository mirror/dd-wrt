#ifndef EGRESSTYPES_H
#define EGRESSTYPES_H

/* Plugin includes */

/* OLSRD includes */
#include "olsr_types.h"

/* System includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

struct sgw_route_info {
  bool active;

  struct {
    unsigned char family;
    uint32_t rttable;
    unsigned int flags;
    unsigned char scope;
    int if_index;
    int metric;
    int protocol;
    bool srcSet;
    bool gwSet;
    bool dstSet;
    bool del_similar;
    bool blackhole;

    union olsr_ip_addr srcStore;
    union olsr_ip_addr gwStore;
    struct olsr_ip_prefix dstStore;
  } route;
};

struct egress_if_bw {
  /* in the egress file */
  uint32_t egressUk; /**< in Kbps, [0, MAX_SMARTGW_SPEED] */
  uint32_t egressDk; /**< in Kbps, [0, MAX_SMARTGW_SPEED] */
  uint32_t path_cost; /**<         [0, UINT32_MAX]        */
  struct olsr_ip_prefix network;
  union olsr_ip_addr gateway;

  /* derived from network, gateway */
  bool networkSet;
  bool gatewaySet;

  /* calculated from egressUk, egressDk, path_costs */
  int64_t costs;
};

struct sgw_egress_if {
  /* configured through the SmartGatewayEgressInterfaces configuration parameter */
  char *name;

  /* updated through the smartgw_tunnel_monitor function */
  int if_index;

  /* configured through the SmartGatewayTablesOffset and SmartGatewayRulesOffset configuration parameters */
  uint8_t tableNr; /**< routing table number */
  uint8_t ruleNr; /**< IP rule number */
  uint8_t bypassRuleNr; /**< bypass IP rule number */

  /* updated through smartgw_tunnel_monitor */
  bool upPrevious;
  bool upCurrent;
  bool upChanged;

  /* configured through the readEgressFile function */
  struct egress_if_bw bwPrevious;
  struct egress_if_bw bwCurrent;
  bool bwCostsChanged;
  bool bwNetworkChanged;
  bool bwGatewayChanged;
  bool bwChanged; /* covers bwCostsChanged, bwNetworkChanged and bwGatewayChanged */

  /* egress routes */
  struct sgw_route_info networkRouteCurrent;
  struct sgw_route_info egressRouteCurrent;

  /* state for the readEgressFile function */
  bool inEgressFile;

  /* next in the list */
  struct sgw_egress_if *next;
};

static inline bool egressBwCostsChanged(struct sgw_egress_if * egress_if) {
  return (egress_if->bwPrevious.costs != egress_if->bwCurrent.costs);
}

static inline bool egressBwNetworkChanged(struct sgw_egress_if * egress_if) {
  return //
    (egress_if->bwPrevious.networkSet != egress_if->bwCurrent.networkSet) || //
    (egress_if->bwCurrent.networkSet && //
      memcmp(&egress_if->bwPrevious.network, &egress_if->bwCurrent.network, sizeof(egress_if->bwCurrent.network)));
}

static inline bool egressBwGatewayChanged(struct sgw_egress_if * egress_if) {
  return //
    (egress_if->bwPrevious.gatewaySet != egress_if->bwCurrent.gatewaySet) || //
    (egress_if->bwCurrent.gatewaySet && //
      memcmp(&egress_if->bwPrevious.gateway, &egress_if->bwCurrent.gateway, sizeof(egress_if->bwCurrent.gateway)));
}

static inline bool egressBwChanged(struct sgw_egress_if * egress_if) {
  return egress_if->bwCostsChanged || egress_if->bwNetworkChanged || egress_if->bwGatewayChanged;
}

#endif /* EGRESSTYPES_H */
