/*
 * olsr_niit.c
 *
 *  Created on: 02.02.2010
 *      Author: henning
 */

#include "defs.h"
#include "kernel_routes.h"
#include "net_os.h"
#include "olsr_niit.h"

#include <net/if.h>

#ifdef LINUX_NETLINK_ROUTING
static void handle_niit_ifchange (int if_index, struct interface *iface, enum olsr_ifchg_flag);

static bool niit4to6_active, niit6to4_active;

void olsr_init_niit(void) {
  if (olsr_cnf->ip_version == AF_INET) {
    olsr_cnf->use_niit = false;
    return;
  }

  olsr_cnf->niit4to6_if_index = if_nametoindex(DEF_NIIT4TO6_IFNAME);
  if (olsr_cnf->niit4to6_if_index <= 0) {
    OLSR_PRINTF(1, "Warning, %s device is not available, deactivating NIIT\n", DEF_NIIT4TO6_IFNAME);
    olsr_cnf->use_niit = false;
    return;
  }
  olsr_cnf->niit6to4_if_index = if_nametoindex(DEF_NIIT6TO4_IFNAME);
  if (olsr_cnf->niit6to4_if_index <= 0) {
    OLSR_PRINTF(1, "Warning, %s device is not available, deactivating NIIT\n", DEF_NIIT6TO4_IFNAME);
    olsr_cnf->use_niit = false;
    return;
  }

  niit4to6_active = olsr_if_isup(DEF_NIIT4TO6_IFNAME);
  niit6to4_active = olsr_if_isup(DEF_NIIT6TO4_IFNAME);

  olsr_add_ifchange_handler(&handle_niit_ifchange);
  olsr_add_ifchange_handler(&handle_niit_ifchange);
  return;
}

void olsr_setup_niit_routes(void) {
  struct ip_prefix_list *h;

  if (!niit4to6_active || !niit6to4_active) {
    return;
  }
  for (h = olsr_cnf->hna_entries; h != NULL; h = h->next) {
    if (ip_prefix_is_mappedv4(&h->net)) {
      olsr_os_niit_6to4_route(&h->net, true);
    }
  }
}

void olsr_cleanup_niit_routes(void) {
  struct ip_prefix_list *h;

  if (!niit6to4_active) {
    return;
  }
  for (h = olsr_cnf->hna_entries; h != NULL; h = h->next) {
    if (ip_prefix_is_mappedv4(&h->net)) {
      olsr_os_niit_6to4_route(&h->net, false);
    }
  }
}

void olsr_niit_handle_route(const struct rt_entry *rt, bool set) {
  if (olsr_cnf->ip_version == AF_INET6 && olsr_cnf->use_niit
      && niit4to6_active && niit6to4_active && is_prefix_niit_ipv6(&rt->rt_dst)) {
    struct olsr_ip_prefix dst_v4;

    prefix_mappedv4_to_v4(&dst_v4, &rt->rt_dst);
    olsr_os_niit_4to6_route(&dst_v4, set);
  }
}

static void refresh_niit4to6_routes(bool set) {
  struct rt_entry *rt;

  if (set && (!niit4to6_active || !niit6to4_active)) {
    return;
  }
  if (!set && !niit4to6_active) {
    return;
  }

  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    if (is_prefix_niit_ipv6(&rt->rt_dst)) {
      struct olsr_ip_prefix dst_v4;

      prefix_mappedv4_to_v4(&dst_v4, &rt->rt_dst);
      olsr_os_niit_4to6_route(&dst_v4, set);
    }
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt)
}

static void handle_niit_ifchange (int if_index, struct interface *iface __attribute__ ((unused)),
    enum olsr_ifchg_flag flag) {
  bool active;

  active = niit4to6_active && niit6to4_active;
  if (if_index == olsr_cnf->niit4to6_if_index) {
    niit4to6_active = flag != IFCHG_IF_REMOVE;
  }
  if (if_index == olsr_cnf->niit6to4_if_index) {
    niit6to4_active = flag != IFCHG_IF_REMOVE;
  }

  if (active != (niit4to6_active && niit6to4_active)) {
    /* niit status change */
    if (!active) {
      /* from inactive to active */
      olsr_setup_niit_routes();
      refresh_niit4to6_routes(true);
    }
    else {
      /* the other way around */
      olsr_cleanup_niit_routes();
      refresh_niit4to6_routes(false);
    }
  }

}
#endif
