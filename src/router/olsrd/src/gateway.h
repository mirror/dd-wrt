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

#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "common/avl.h"
#include "common/list.h"
#include "defs.h"
#include "olsr.h"
#include "scheduler.h"
#include "gateway_list.h"
#include <net/if.h>
#include <stdbool.h>

/** used to signal to olsr_delete_gateway_entry to force deletion */
#define FORCE_DELETE_GW_ENTRY 255

/** the interval (in milliseconds) on which to run gateway cleanup */
#define GW_CLEANUP_INTERVAL 30000

/**
 * @return true if multi-gateway mode is enabled
 */
static INLINE bool multi_gateway_mode(void) {
  return (olsr_cnf->smart_gw_use_count > 1);
}

/*
 * hack for Vienna network:
 * set MAXIMUM_GATEWAY_PREFIX_LENGTH to 1 to remove 0.0.0.0/128.0.0.0 and
 * 128.0.0.0/128.0.0.0 routes
 */
// #define MAXIMUM_GATEWAY_PREFIX_LENGTH 1

/** gateway HNA flags */
enum gateway_hna_flags {
  GW_HNA_FLAG_LINKSPEED   = 1u << 0,
  GW_HNA_FLAG_IPV4        = 1u << 1,
  GW_HNA_FLAG_IPV4_NAT    = 1u << 2,
  GW_HNA_FLAG_IPV6        = 1u << 3,
  GW_HNA_FLAG_IPV6PREFIX  = 1u << 4
};

/** gateway HNA field byte offsets in the netmask field of the HNA */
enum gateway_hna_fields {
  GW_HNA_PAD              = 0,
  GW_HNA_FLAGS            = 1,
  GW_HNA_UPLINK           = 2,
  GW_HNA_DOWNLINK         = 3,
  GW_HNA_V6PREFIXLEN      = 4,
  GW_HNA_V6PREFIX         = 5
};

/** a gateway entry */
struct gateway_entry {
    struct avl_node node;
    union olsr_ip_addr originator;
    struct olsr_ip_prefix external_prefix;
    uint32_t uplink;
    uint32_t downlink;
    int64_t path_cost; /**< the gateway path costs */
    bool ipv4;
    bool ipv4nat;
    bool ipv6;

    struct timer_entry *expire_timer;
    struct timer_entry *cleanup_timer;
    uint16_t seqno;
};

enum sgw_multi_change_phase {
  GW_MULTI_CHANGE_PHASE_STARTUP = 0,
  GW_MULTI_CHANGE_PHASE_RUNTIME = 1,
  GW_MULTI_CHANGE_PHASE_SHUTDOWN = 2
};

#ifdef __linux__
/** structure that holds an interface name, mark and a pointer to the gateway that uses it */
struct interfaceName {
  char name[IFNAMSIZ]; /**< interface name */
  uint8_t tableNr; /**< routing table number */
  uint8_t ruleNr; /**< IP rule number */
  uint8_t bypassRuleNr; /**< bypass IP rule number */
  struct gateway_entry *gw; /**< gateway that uses this interface name */
};
#endif /* __linux__ */

/**
 * static INLINE struct gateway_entry * node2gateway (struct avl_node *ptr)
 *
 * Converts a node into a gateway entry
 */
AVLNODE2STRUCT(node2gateway, struct gateway_entry, node);

/**
 * Loop over all gateway entries and put the iterated gateway entry in gw
 */
#define OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) \
{ \
  struct avl_node *gw_node, *next_gw_node; \
  for (gw_node = avl_walk_first(&gateway_tree); \
    gw_node; gw_node = next_gw_node) { \
    next_gw_node = avl_walk_next(gw_node); \
    gw = node2gateway(gw_node); \
    if (gw) {
#define OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw) }}}

/** the gateway tree */
extern struct avl_tree gateway_tree;

/** the list IPv4 gateways */
extern struct gw_list gw_list_ipv4;

/** the list IPv6 gateways */
extern struct gw_list gw_list_ipv6;

/**
 * Function pointer table for gateway plugin hooks.
 */
struct olsr_gw_handler {
    /**
     * Called on olsrd startup to initialise the plugin.
     */
    void (*init)(void);

    /**
     * Called on olsrd shutdown to cleanup the plugin.
     */
    void (*cleanup)(void);

    /**
     * Called on olsrd startup to perform the initial gateway selection.
     */
    void (*startup)(void);

    /**
     * Called when the costs of a gateway must be determined.
     *
     * @param gw the gateway
     * @return the costs
     */
    int64_t (*getcosts)(struct gateway_entry *gw);

    /**
     * Called when a new gateway must be chosen.
     *
     * @param ipv4 true when an IPv4 gateway must be chosen
     * @param ipv6 true when an IPv6 gateway must be chosen
     */
    void (*choose)(bool ipv4, bool ipv6);

    /**
     * Called when a gateway HNA is received.
     *
     * @param gw the gateway entry
     */
    void (*update)(struct gateway_entry * gw);

    /**
     * Called when a TC or a HNA is removed.
     *
     * @param gw the gateway entry
     */
    void (* delete)(struct gateway_entry * gw);
};

/*
 * Main Interface
 */

int olsr_init_gateways(void);
int olsr_startup_gateways(void);
void olsr_shutdown_gateways(void);
void olsr_cleanup_gateways(void);
void olsr_trigger_inetgw_startup(void);
#ifndef NODEBUG
void olsr_print_gateway_entries(void);
#else
#define olsr_print_gateway_entries() do { } while(0)
#endif

/*
 * Tx Path Interface
 */

void olsr_modifiy_inetgw_netmask(union olsr_ip_addr *mask, int prefixlen, bool zero);

/*
 * Interface to adjust uplink/downlink speed
 */

void refresh_smartgw_netmask(void);

/*
 * TC/SPF/HNA Interface
 */

bool olsr_is_smart_gateway(struct olsr_ip_prefix *prefix, union olsr_ip_addr *net);
void olsr_update_gateway_entry(union olsr_ip_addr *originator, union olsr_ip_addr *mask, int prefixlen, uint16_t seqno, olsr_reltime vtime);
void olsr_delete_gateway_entry(union olsr_ip_addr *originator, uint8_t prefixlen, bool immediate);
void olsr_trigger_gatewayloss_check(void);

/*
 * Gateway Plugin Functions
 */

bool olsr_set_inet_gateway(struct gateway_entry * chosen_gw, bool ipv4, bool ipv6);
struct gateway_entry *olsr_get_inet_gateway(bool ipv6);

/*
 * Multi Smart Gateway functions
 */

void doRoutesMultiGw(bool egressChanged, bool olsrChanged, enum sgw_multi_change_phase phase);
bool isEgressSelected(struct sgw_egress_if * egress_if);

#endif /* GATEWAY_H_ */
