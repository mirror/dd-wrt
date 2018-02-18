/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef EGRESSTYPES_H
#define EGRESSTYPES_H

/* Plugin includes */

/* OLSRD includes */
#include "olsr_types.h"
#include "defs.h"

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
  bool requireNetwork;
  bool requireGateway;
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

static INLINE bool egressBwCostsChanged(struct sgw_egress_if * egress_if) {
  return (egress_if->bwPrevious.costs != egress_if->bwCurrent.costs);
}

static INLINE bool egressBwNetworkChanged(struct sgw_egress_if * egress_if) {
  return //
    (egress_if->bwPrevious.requireNetwork != egress_if->bwCurrent.requireNetwork) || //
    (egress_if->bwPrevious.networkSet != egress_if->bwCurrent.networkSet) || //
    (egress_if->bwCurrent.networkSet && //
      memcmp(&egress_if->bwPrevious.network, &egress_if->bwCurrent.network, sizeof(egress_if->bwCurrent.network)));
}

static INLINE bool egressBwGatewayChanged(struct sgw_egress_if * egress_if) {
  return //
    (egress_if->bwPrevious.requireGateway != egress_if->bwCurrent.requireGateway) || //
    (egress_if->bwPrevious.gatewaySet != egress_if->bwCurrent.gatewaySet) || //
    (egress_if->bwCurrent.gatewaySet && //
      memcmp(&egress_if->bwPrevious.gateway, &egress_if->bwCurrent.gateway, sizeof(egress_if->bwCurrent.gateway)));
}

static INLINE bool egressBwChanged(struct sgw_egress_if * egress_if) {
  return egress_if->bwCostsChanged || egress_if->bwNetworkChanged || egress_if->bwGatewayChanged;
}

#endif /* EGRESSTYPES_H */
