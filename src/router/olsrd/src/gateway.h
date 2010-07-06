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

#define FORCE_DELETE_GW_ENTRY 255
#define GW_CLEANUP_INTERVAL 30000

/*
 * hack for Vienna network to remove 0.0.0.0/128.0.0.0 and 128.0.0.0/128.0.0.0 routes
 * just set MAXIMUM_GATEWAY_PREFIX_LENGTH to 1
 */
// #define MAXIMUM_GATEWAY_PREFIX_LENGTH 1

enum gateway_hna_flags {
  GW_HNA_FLAG_LINKSPEED  = 1<<0,
  GW_HNA_FLAG_IPV4       = 1<<1,
  GW_HNA_FLAG_IPV4_NAT   = 1<<2,
  GW_HNA_FLAG_IPV6       = 1<<3,
  GW_HNA_FLAG_IPV6PREFIX = 1<<4
};

/* relative to the first zero byte in the netmask (0, 1 or 12) */
enum gateway_hna_fields {
  GW_HNA_PAD         = 0,
  GW_HNA_FLAGS       = 1,
  GW_HNA_UPLINK      = 2,
  GW_HNA_DOWNLINK    = 3,
  GW_HNA_V6PREFIXLEN = 4,
  GW_HNA_V6PREFIX    = 5
};

struct gateway_entry {
  struct avl_node node;
  union olsr_ip_addr originator;
  struct olsr_ip_prefix external_prefix;
  uint32_t uplink, downlink;
  bool ipv4, ipv4nat, ipv6;

  struct timer_entry *cleanup_timer;
  uint16_t seqno;
};

AVLNODE2STRUCT(node2gateway, struct gateway_entry, node);

#define OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) \
{ \
  struct avl_node *gw_node, *next_gw_node; \
  for (gw_node = avl_walk_first(&gateway_tree); \
    gw_node; gw_node = next_gw_node) { \
    next_gw_node = avl_walk_next(gw_node); \
    gw = node2gateway(gw_node);
#define OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw) }}

extern struct avl_tree gateway_tree;

int olsr_init_gateways(void);
void olsr_cleanup_gateways(void);
void refresh_smartgw_netmask(void);
void olsr_trigger_inetgw_startup(void);
int olsr_trigger_inetgw_selection(bool ipv4, bool ipv6);
void olsr_trigger_gatewayloss_check(void);

struct gateway_entry *olsr_find_gateway_entry(union olsr_ip_addr *originator);
void olsr_update_gateway_entry(union olsr_ip_addr *originator, union olsr_ip_addr *mask, int prefixlen, uint16_t seqno);
void olsr_delete_gateway_entry(union olsr_ip_addr *originator, uint8_t prefixlen);
void olsr_print_gateway_entries(void);

bool olsr_set_inet_gateway(union olsr_ip_addr *originator, bool ipv4, bool ipv6, bool external);
struct gateway_entry *olsr_get_ipv4_inet_gateway(bool *);
struct gateway_entry *olsr_get_ipv6_inet_gateway(bool *);
bool olsr_is_smart_gateway(struct olsr_ip_prefix *prefix, union olsr_ip_addr *net);
void olsr_modifiy_inetgw_netmask(union olsr_ip_addr *mask, int prefixlen);

struct olsr_gw_handler {
  void (* handle_startup)(void);
  void (* select_gateway) (bool ipv4, bool ipv6);
  void (* handle_update_gw)(struct gateway_entry *);
  void (* handle_delete_gw)(struct gateway_entry *);
};

void olsr_set_inetgw_handler(struct olsr_gw_handler *l);
#endif /* GATEWAY_H_ */
