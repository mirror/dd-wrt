/*
 * gateway.h
 *
 *  Created on: 05.01.2010
 *      Author: henning
 */

#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "common/avl.h"
#include "common/list.h"
#include "defs.h"
#include "olsr.h"
#include "scheduler.h"
#include "gateway_list.h"

/** used to signal to olsr_delete_gateway_entry to force deletion */
#define FORCE_DELETE_GW_ENTRY 255

/** the interval (in milliseconds) on which to run gateway cleanup */
#define GW_CLEANUP_INTERVAL 30000

/*
 * hack for Vienna network:
 * set MAXIMUM_GATEWAY_PREFIX_LENGTH to 1 to remove 0.0.0.0/128.0.0.0 and
 * 128.0.0.0/128.0.0.0 routes
 */
// #define MAXIMUM_GATEWAY_PREFIX_LENGTH 1

/** gateway HNA flags */
enum gateway_hna_flags {
  GW_HNA_FLAG_LINKSPEED   = 1 << 0,
  GW_HNA_FLAG_IPV4        = 1 << 1,
  GW_HNA_FLAG_IPV4_NAT    = 1 << 2,
  GW_HNA_FLAG_IPV6        = 1 << 3,
  GW_HNA_FLAG_IPV6PREFIX  = 1 << 4
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

    struct timer_entry *cleanup_timer;
    uint16_t seqno;
};

/**
 * static inline struct gateway_entry * node2gateway (struct avl_node *ptr)
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
    gw = node2gateway(gw_node);
#define OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw) }}

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

void olsr_modifiy_inetgw_netmask(union olsr_ip_addr *mask, int prefixlen);

/*
 * Interface to adjust uplink/downlink speed
 */

void refresh_smartgw_netmask(void);

/*
 * TC/SPF/HNA Interface
 */

bool olsr_is_smart_gateway(struct olsr_ip_prefix *prefix, union olsr_ip_addr *net);
void olsr_update_gateway_entry(union olsr_ip_addr *originator, union olsr_ip_addr *mask, int prefixlen, uint16_t seqno);
void olsr_delete_gateway_entry(union olsr_ip_addr *originator, uint8_t prefixlen, bool immediate);
void olsr_trigger_gatewayloss_check(void);

/*
 * Gateway Plugin Functions
 */

bool olsr_set_inet_gateway(struct gateway_entry * chosen_gw, bool ipv4, bool ipv6);
struct gateway_entry *olsr_get_inet_gateway(bool ipv6);

#endif /* GATEWAY_H_ */
