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

#include "olsrd_txtinfo.h"

#include <unistd.h>

#include "ipcalc.h"
#include "builddata.h"
#include "neighbor_table.h"
#include "mpr_selector_set.h"
#include "mid_set.h"
#include "routing_table.h"
#include "lq_plugin.h"
#include "gateway.h"
#include "gateway_costs.h"
#include "olsrd_plugin.h"
#include "info/info_types.h"
#include "info/http_headers.h"
#include "gateway_default_handler.h"

unsigned long long get_supported_commands_mask(void) {
  return (SIW_ALL | SIW_OLSRD_CONF) & ~(SIW_CONFIG | SIW_PLUGINS);
}

bool isCommand(const char *str, unsigned long long siw) {
  const char * cmd;
  switch (siw) {
    case SIW_OLSRD_CONF:
      cmd = "/con";
      break;

    case SIW_ALL:
      cmd = "/all";
      break;

    case SIW_RUNTIME_ALL:
      cmd = "/runtime";
      break;

    case SIW_STARTUP_ALL:
      cmd = "/startup";
      break;

    case SIW_NEIGHBORS:
      cmd = "/nei";
      break;

    case SIW_LINKS:
      cmd = "/lin";
      break;

    case SIW_ROUTES:
      cmd = "/rou";
      break;

    case SIW_HNA:
      cmd = "/hna";
      break;

    case SIW_MID:
      cmd = "/mid";
      break;

    case SIW_TOPOLOGY:
      cmd = "/top";
      break;

    case SIW_GATEWAYS:
      cmd = "/gat";
      break;

    case SIW_INTERFACES:
      cmd = "/int";
      break;

    case SIW_2HOP:
      cmd = "/2ho";
      break;

    case SIW_SGW:
      cmd = "/sgw";
      break;

    case SIW_VERSION:
      cmd = "/ver";
      break;

    case SIW_NEIGHBORS_FREIFUNK:
      cmd = "/neighbours";
      break;

    default:
      return false;
  }

  return !strcmp(str, cmd);
}

void output_error(struct autobuf *abuf, unsigned int status, const char * req __attribute__((unused)), bool http_headers) {
  if (http_headers || (status == INFO_HTTP_OK)) {
    return;
  }

  /* !http_headers && !INFO_HTTP_OK */

  if (status == INFO_HTTP_NOCONTENT) {
    /* wget can't handle output of zero length */
    abuf_puts(abuf, "\n");
  } else {
    abuf_appendf(abuf, "error: %s\n", httpStatusToReply(status));
  }
}

static void ipc_print_neighbors_internal(struct autobuf *abuf, bool list_2hop) {
  struct ipaddr_str neighAddrBuf;
  struct neighbor_entry *neigh;
  struct neighbor_2_list_entry *list_2;
  int thop_cnt;

  const char * field;
  if (list_2hop) {
    field = "(2-hop address)+";
  } else {
    field = "2-hop count";
  }

  abuf_puts(abuf, "Table: Neighbors\n");
  abuf_appendf(abuf, "IP address\tSYM\tMPR\tMPRS\tWill.\t%s\n", field);

  /* Neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {
    abuf_appendf(abuf, "%s\t%s\t%s\t%s\t%d",
      olsr_ip_to_string(&neighAddrBuf, &neigh->neighbor_main_addr),
      (neigh->status == SYM) ? "YES" : "NO",
      neigh->is_mpr ? "YES" : "NO",
      olsr_lookup_mprs_set(&neigh->neighbor_main_addr) ? "YES" : "NO",
      neigh->willingness);
    thop_cnt = 0;

    for (list_2 = neigh->neighbor_2_list.next; list_2 != &neigh->neighbor_2_list; list_2 = list_2->next) {
      if (list_2hop) {
        if (list_2->neighbor_2) {
          abuf_appendf(abuf, "\t%s", olsr_ip_to_string(&neighAddrBuf, &list_2->neighbor_2->neighbor_2_addr));
        }
      } else {
        thop_cnt++;
      }
    }

    if (!list_2hop) {
      abuf_appendf(abuf, "\t%d", thop_cnt);
    }
    abuf_puts(abuf, "\n");
  } OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);
  abuf_puts(abuf, "\n");
}

void ipc_print_neighbors(struct autobuf *abuf) {
  ipc_print_neighbors_internal(abuf, false);
}

void ipc_print_links(struct autobuf *abuf) {
  struct link_entry *my_link;

  const char * field;
  if (vtime) {
    field = "VTime";
  } else {
    field = "Hyst.";
  }

  abuf_puts(abuf, "Table: Links\n");
  abuf_appendf(abuf, "Local IP\tRemote IP\t%s\tLQ\tNLQ\tCost\n", field);

  /* Link set */
  OLSR_FOR_ALL_LINK_ENTRIES(my_link) {
    struct ipaddr_str localAddr;
    struct ipaddr_str remoteAddr;
    struct lqtextbuffer lqbuffer;
    struct lqtextbuffer costbuffer;
    unsigned int diffI = 0;
    unsigned int diffF = 0;

    if (vtime) {
      unsigned int diff = my_link->link_timer ? (unsigned int) (my_link->link_timer->timer_clock - now_times) : 0;
      diffI = diff / 1000;
      diffF = diff % 1000;
    }

    abuf_appendf(abuf, "%s\t%s\t%u.%03u\t%s\t%s\n",
      olsr_ip_to_string(&localAddr, &my_link->local_iface_addr),
      olsr_ip_to_string(&remoteAddr, &my_link->neighbor_iface_addr),
      diffI,
      diffF,
      get_link_entry_text(my_link, '\t', &lqbuffer),
      get_linkcost_text(my_link->linkcost, false, &costbuffer));
  } OLSR_FOR_ALL_LINK_ENTRIES_END(my_link);
  abuf_puts(abuf, "\n");
}

void ipc_print_routes(struct autobuf *abuf) {
  struct rt_entry *rt;

  abuf_puts(abuf, "Table: Routes\n");
  abuf_puts(abuf, "Destination\tGateway IP\tMetric\tETX\tInterface\n");

  /* Walk the route table */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    struct ipaddr_str dstAddr;
    struct ipaddr_str nexthopAddr;
    struct lqtextbuffer costbuffer;

    if (rt->rt_best) {
      abuf_appendf(abuf, "%s/%d\t%s\t%d\t%s\t%s\t\n",
        olsr_ip_to_string(&dstAddr, &rt->rt_dst.prefix),
        rt->rt_dst.prefix_len,
        olsr_ip_to_string(&nexthopAddr, &rt->rt_best->rtp_nexthop.gateway),
        rt->rt_best->rtp_metric.hops,
        get_linkcost_text(rt->rt_best->rtp_metric.cost, true, &costbuffer),
        if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
    }
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);
  abuf_puts(abuf, "\n");
}

void ipc_print_topology(struct autobuf *abuf) {
  struct tc_entry *tc;

  const char * field;
  if (vtime) {
    field = "\tVTime";
  } else {
    field = "";
  }

  abuf_puts(abuf, "Table: Topology\n");
  abuf_appendf(abuf, "Dest. IP\tLast hop IP\tLQ\tNLQ\tCost%s\n", field);

  /* Topology */
  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      if (tc_edge->edge_inv) {
        struct ipaddr_str dstAddr;
        struct ipaddr_str lastHopAddr;
        struct lqtextbuffer lqbuffer;
        struct lqtextbuffer costbuffer;

        abuf_appendf(abuf, "%s\t%s\t%s\t%s",
          olsr_ip_to_string(&dstAddr, &tc_edge->T_dest_addr),
          olsr_ip_to_string(&lastHopAddr, &tc->addr),
          get_tc_edge_entry_text(tc_edge, '\t', &lqbuffer),
          get_linkcost_text(tc_edge->cost, false, &costbuffer));

        if (vtime) {
          unsigned int diff = (unsigned int) (tc->validity_timer ? (tc->validity_timer->timer_clock - now_times) : 0);
          abuf_appendf(abuf, "\t%u.%03u", diff / 1000, diff % 1000);
        }

        abuf_puts(abuf, "\n");
      }
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);
  abuf_puts(abuf, "\n");
}

void ipc_print_hna(struct autobuf *abuf) {
  struct ip_prefix_list *hna;
  struct hna_entry *tmp_hna;
  struct ipaddr_str prefixbuf;
  struct ipaddr_str gwaddrbuf;

  const char * field;
  if (vtime) {
    field = "\tVTime";
  } else {
    field = "";
  }

  abuf_puts(abuf, "Table: HNA\n");
  abuf_appendf(abuf, "Destination\tGateway%s\n", field);

  /* Announced HNA entries */
  for (hna = olsr_cnf->hna_entries; hna != NULL ; hna = hna->next) {
    abuf_appendf(abuf, "%s/%d\t%s",
      olsr_ip_to_string(&prefixbuf, &hna->net.prefix),
      hna->net.prefix_len,
      olsr_ip_to_string(&gwaddrbuf, &olsr_cnf->main_addr));

      if (vtime) {
        abuf_appendf(abuf, "\t%u.%03u", 0, 0);
      }
      abuf_puts(abuf, "\n");
  }

  /* HNA entries */
  OLSR_FOR_ALL_HNA_ENTRIES(tmp_hna) {
    struct hna_net *tmp_net;

    /* Check all networks */
    for (tmp_net = tmp_hna->networks.next; tmp_net != &tmp_hna->networks; tmp_net = tmp_net->next) {
      abuf_appendf(abuf, "%s/%d\t%s",
        olsr_ip_to_string(&prefixbuf, &tmp_net->hna_prefix.prefix),
        tmp_net->hna_prefix.prefix_len,
        olsr_ip_to_string(&gwaddrbuf, &tmp_hna->A_gateway_addr));

      if (vtime) {
        unsigned int diff = tmp_net->hna_net_timer ? (unsigned int) (tmp_net->hna_net_timer->timer_clock - now_times) : 0;
        abuf_appendf(abuf, "\t%u.%03u", diff / 1000, diff % 1000);
      }
      abuf_puts(abuf, "\n");
    }
  } OLSR_FOR_ALL_HNA_ENTRIES_END(tmp_hna);
  abuf_puts(abuf, "\n");
}

void ipc_print_mid(struct autobuf *abuf) {
  int idx;

  const char * field;
  if (vtime) {
    field = ":VTime";
  } else {
    field = "";
  }

  abuf_puts(abuf, "Table: MID\n");
  abuf_appendf(abuf, "IP address\t(Alias%s)+\n", field);

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    struct mid_entry *entry = mid_set[idx].next;

    while (entry && (entry != &mid_set[idx])) {
      struct mid_address *alias = entry->aliases;
      struct ipaddr_str ipAddr;

      abuf_puts(abuf, olsr_ip_to_string(&ipAddr, &entry->main_addr));
      abuf_puts(abuf, "\t");

      while (alias) {
        struct ipaddr_str buf2;

        abuf_appendf(abuf, "\t%s", olsr_ip_to_string(&buf2, &alias->alias));

        if (vtime) {
          unsigned int diff = (unsigned int) (alias->vtime - now_times);
          abuf_appendf(abuf, ":%u.%03u", diff / 1000, diff % 1000);
        }

        alias = alias->next_alias;
      }
      entry = entry->next;
      abuf_puts(abuf, "\n");
    }
  }
  abuf_puts(abuf, "\n");
}

void ipc_print_gateways(struct autobuf *abuf) {
#ifndef __linux__
  abuf_puts(abuf, "error: Gateway mode is only supported on Linux\n");
#else /* __linux__ */
  static const char *fmth = "%-6s %-45s %-15s %-6s %-9s %-9s %-7s %-4s %s\n";
  static const char *fmtv = "%c%c%-4s %-45s %-15s %-6u %-9u %-9u %-7s %-4s %s\n";

  static const char IPV4[] = "ipv4";
  static const char IPV4_NAT[] = "ipv4(n)";
  static const char IPV6[] = "ipv6";
  static const char NONE[] = "-";

  struct gateway_entry *gw;
  struct gateway_entry *current_gw_4 = olsr_get_inet_gateway(false);
  struct gateway_entry *current_gw_6 = olsr_get_inet_gateway(true);

  abuf_puts(abuf, "Table: Gateways\n");
  abuf_appendf(abuf, fmth, "Status", "Gateway IP", "ETX", "Hopcnt", "Uplink", "Downlnk", "IPv4", "IPv6", "Prefix");

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    char v4, v6;
    const char *v4type, *v6type;
    struct ipaddr_str originatorbuf;
    struct lqtextbuffer lqbuf;
    struct tc_entry *tc = olsr_lookup_tc_entry(&gw->originator);

    if (current_gw_4 && (gw == current_gw_4)) {
      v4 = 's';
    } else if (isGwSelectable(gw, false)) {
      v4 = 'u';
    } else {
      v4 = '-';
    }

    if (current_gw_6 && (gw == current_gw_6)) {
      v6 = 's';
    } else if (isGwSelectable(gw, true)) {
      v6 = 'u';
    } else {
      v6 = '-';
    }

    if (gw->ipv4) {
      v4type = gw->ipv4nat ? IPV4_NAT : IPV4;
    } else {
      v4type = NONE;
    }
    if (gw->ipv6) {
      v6type = IPV6;
    } else {
      v6type = NONE;
    }

    abuf_appendf(abuf, fmtv, //
      v4, //
      v6, //
      "", //
      olsr_ip_to_string(&originatorbuf, &gw->originator), //
      get_linkcost_text(!tc ? ROUTE_COST_BROKEN : tc->path_cost, true, &lqbuf), //
      !tc ? 0 : tc->hops, //
      gw->uplink, //
      gw->downlink, //
      v4type, //
      v6type, //
      !gw->external_prefix.prefix_len ? NONE : olsr_ip_prefix_to_string(&gw->external_prefix));
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END (gw)
#endif /* __linux__ */
  abuf_puts(abuf, "\n");
}

#ifdef __linux__

/** interface names for smart gateway tunnel interfaces, IPv4 */
extern struct interfaceName * sgwTunnel4InterfaceNames;

/** interface names for smart gateway tunnel interfaces, IPv6 */
extern struct interfaceName * sgwTunnel6InterfaceNames;

/**
 * Construct the sgw table for a given ip version
 *
 * @param abuf the string buffer
 * @param ipv6 true for IPv6, false for IPv4
 * @param fmtv the format for printing
 */
static void sgw_ipvx(struct autobuf *abuf, bool ipv6, const char * fmth, const char * fmtv) {
  struct interfaceName * sgwTunnelInterfaceNames = !ipv6 ? sgwTunnel4InterfaceNames : sgwTunnel6InterfaceNames;
  struct gwtextbuffer gwbuf;

  abuf_appendf(abuf, "Table: Smart Gateway IPv%d\n", ipv6 ? 6 : 4);
  abuf_appendf(abuf, fmth, " ", "Originator", "Prefix", "Uplink", "Downlink", "PathCost", "IPv4", "IPv4-NAT", "IPv6", "Tunnel-Name", "Destination", "Cost");

  if (olsr_cnf->smart_gw_active && sgwTunnelInterfaceNames) {
    struct gateway_entry * current_gw = olsr_get_inet_gateway(ipv6);

    int i;
    for (i = 0; i < olsr_cnf->smart_gw_use_count; i++) {
      struct interfaceName * node = &sgwTunnelInterfaceNames[i];
      struct gateway_entry * gw = node->gw;

      if (gw) {
        struct lqtextbuffer lqbuf;
        struct tc_entry* tc = olsr_lookup_tc_entry(&gw->originator);

        struct ipaddr_str originator;
        struct ipaddr_str prefix;
        char prefixAndMask[INET6_ADDRSTRLEN * 2];

        olsr_ip_to_string(&originator, &gw->originator);
        olsr_ip_to_string(&prefix, &gw->external_prefix.prefix);

        if (!ipv6) {
          union olsr_ip_addr netmask = { { 0 } };
          struct ipaddr_str prefixMask;
          prefix_to_netmask((uint8_t *) &netmask, sizeof(netmask.v4), gw->external_prefix.prefix_len);
          olsr_ip_to_string(&prefixMask, &netmask);
          snprintf(prefixAndMask, sizeof(prefixAndMask), "%s/%s", prefix.buf, prefixMask.buf);
        } else {
          snprintf(prefixAndMask, sizeof(prefixAndMask), "%s/%d", prefix.buf, gw->external_prefix.prefix_len);
        }

        abuf_appendf(abuf, fmtv, //
          (current_gw && (current_gw == gw)) ? "*" : " ", // selected
          originator.buf, // Originator
          prefixAndMask, // 4: IP/Mask, 6: IP/Length
          gw->uplink, // Uplink
          gw->downlink, // Downlink
          get_linkcost_text(!tc ? ROUTE_COST_BROKEN : tc->path_cost, true, &lqbuf), // PathCost
          gw->ipv4 ? "Y" : "N", // IPv4
          gw->ipv4nat ? "Y" : "N", // IPv4-NAT
          gw->ipv6 ? "Y" : "N", // IPv6
          node->name, // Tunnel-Name
          originator.buf, // Destination
          get_gwcost_text(gw->path_cost, &gwbuf) // Cost
          );
      }
    }
  }
  abuf_puts(abuf, "\n");
}
#endif /* __linux__ */

void ipc_print_sgw(struct autobuf *abuf) {
#ifndef __linux__
  abuf_puts(abuf, "error: Gateway mode is only supported on Linux\n");
#else

  static const char * fmth4 = "%s%-15s %-31s %-9s %-9s %-10s %-4s %-8s %-4s %-15s %-15s %s\n";
  static const char * fmtv4 = "%s%-15s %-31s %-9u %-9u %-10s %-4s %-8s %-4s %-15s %-15s %s\n";
#if 0
  static const char * fmth6 = "%s%-45s %-49s %-9s %-9s %-10s %-4s %-8s %-4s %-15s %-45s %s\n";
  static const char * fmtv6 = "%s%-45s %-49s %-9u %-9u %-10s %-4s %-8s %-4s %-15s %-45s %s\n";
#endif

  sgw_ipvx(abuf, false, fmth4, fmtv4);
#if 0
  sgw_ipvx(abuf, true, fmth6, fmtv6);
#endif
#endif /* __linux__ */
}

void ipc_print_version(struct autobuf *abuf) {
  abuf_appendf(abuf, "Version: %s (built on %s on %s)\n", olsrd_version, build_date, build_host);
  abuf_puts(abuf, "\n");
}

void ipc_print_olsrd_conf(struct autobuf *abuf) {
  olsrd_write_cnf_autobuf(abuf, olsr_cnf);
}

void ipc_print_interfaces(struct autobuf *abuf) {
  const struct olsr_if *ifs;

  abuf_puts(abuf, "Table: Interfaces\n");
  abuf_puts(abuf, "Name\tState\tMTU\tWLAN\tSrc-Adress\tMask\tDst-Adress\n");

  for (ifs = olsr_cnf->interfaces; ifs != NULL ; ifs = ifs->next) {
    const struct interface_olsr * const rifs = ifs->interf;

    abuf_appendf(abuf, "%s", ifs->name);

    if (!rifs) {
      abuf_puts(abuf, "\tDOWN\n");
      continue;
    }
    abuf_appendf(abuf, "\tUP\t%d\t%s",
        rifs->int_mtu,
        rifs->is_wireless ? "Yes" : "No");

    {
      struct ipaddr_str addrbuf;
      struct ipaddr_str maskbuf;
      struct ipaddr_str bcastbuf;

      if (olsr_cnf->ip_version == AF_INET) {
        abuf_appendf(abuf, "\t%s\t%s\t%s\n",
            ip4_to_string(&addrbuf, rifs->int_addr.sin_addr),
            ip4_to_string(&maskbuf, rifs->int_netmask.sin_addr),
            ip4_to_string(&bcastbuf, rifs->int_broadaddr.sin_addr));
      } else {
        abuf_appendf(abuf, "\t%s\t\t%s\n",
            ip6_to_string(&addrbuf, &rifs->int6_addr.sin6_addr),
            ip6_to_string(&bcastbuf, &rifs->int6_multaddr.sin6_addr));
      }
    }
  }
  abuf_puts(abuf, "\n");
}

void ipc_print_twohop(struct autobuf *abuf) {
  ipc_print_neighbors_internal(abuf, true);
}
