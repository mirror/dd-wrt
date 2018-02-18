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

#include "olsrd_netjson.h"

#include <unistd.h>

#include "olsrd_netjson_helpers.h"
#include "olsrd_plugin.h"
#include "info/info_types.h"
#include "info/http_headers.h"
#include "info/json_helpers.h"
#include "common/avl.h"
#include "olsr.h"
#include "builddata.h"
#include "routing_table.h"
#include "mid_set.h"
#include "link_set.h"

#define NETJSON_PROTOCOL "olsrv1"

static struct json_session json_session;

unsigned long long get_supported_commands_mask(void) {
  return SIW_NETJSON & ~(SIW_NETJSON_DEVICE_CONFIGURATION | SIW_NETJSON_DEVICE_MONITORING);
}

bool isCommand(const char *str, unsigned long long siw) {
  const char * cmd = NULL;
  switch (siw) {
    case SIW_NETJSON_NETWORK_ROUTES:
      cmd = "/NetworkRoutes";
      break;

    case SIW_NETJSON_NETWORK_GRAPH:
      cmd = "/NetworkGraph";
      break;

    case SIW_NETJSON_NETWORK_COLLECTION:
      cmd = "/NetworkCollection";
      break;

    default:
      return false;
  }

  return !strcmp(str, cmd);
}

const char * determine_mime_type(unsigned int send_what __attribute__((unused))) {
  return "application/vnd.api+json";
}

void output_start(struct autobuf *abuf) {
  /* global variables for tracking when to put a comma in for JSON */
  abuf_json_reset_entry_number_and_depth(&json_session, pretty);
  abuf_json_mark_output(&json_session, true, abuf);
}

void output_end(struct autobuf *abuf) {
  abuf_json_mark_output(&json_session, false, abuf);
  abuf_puts(abuf, "\n");
  abuf_json_reset_entry_number_and_depth(&json_session, pretty);
}

void output_error(struct autobuf *abuf, unsigned int status, const char * req __attribute__((unused)), bool http_headers) {
  if (http_headers || (status == INFO_HTTP_OK)) {
    return;
  }

  /* !http_headers && !INFO_HTTP_OK */

  output_start(abuf);

  if (status == INFO_HTTP_NOCONTENT) {
    /* do nothing */
  } else {
    abuf_json_string(&json_session, abuf, "error", httpStatusToReply(status));
  }

  output_end(abuf);
}

void ipc_print_network_routes(struct autobuf *abuf) {
  struct rt_entry *rt;

  /* mandatory */
  abuf_json_string(&json_session, abuf, "type", "NetworkRoutes");
  abuf_json_string(&json_session, abuf, "protocol", NETJSON_PROTOCOL);
  abuf_json_string(&json_session, abuf, "version", release_version);
  abuf_json_string(&json_session, abuf, "metric", olsr_cnf->lq_algorithm);

  /* optional */
  abuf_json_string(&json_session, abuf, "revision", olsrd_version);
  // topology_id
  abuf_json_ip_address(&json_session, abuf, "router_id", &olsr_cnf->main_addr);

  /* Walk the route table */
  abuf_json_mark_object(&json_session, true, true, abuf, "routes");
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    if (rt->rt_best) {
      struct lqtextbuffer lqbuf;

      abuf_json_mark_array_entry(&json_session, true, abuf);
      /* mandatory */
      abuf_json_prefix(&json_session, abuf, "destination", &rt->rt_dst);
      abuf_json_ip_address(&json_session, abuf, "next", &rt->rt_best->rtp_nexthop.gateway);
      abuf_json_string(&json_session, abuf, "device", if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
      abuf_json_float(&json_session, abuf, "cost", get_linkcost_scaled(rt->rt_best->rtp_metric.cost, true));

      /* optional */
      abuf_json_string(&json_session, abuf, "cost_text", get_linkcost_text(rt->rt_best->rtp_metric.cost, true, &lqbuf));
      // source

      abuf_json_mark_array_entry(&json_session, false, abuf);
    }
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);
  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}

void ipc_print_network_graph(struct autobuf *abuf) {
  struct avl_tree nodes;
  struct mid_entry mid_self;
  struct node_entry * node_self;
  struct tc_entry * tc;
  struct link_entry * link_entry;
  struct neighbor_entry * neighbor;
  int idx;

  avl_init(&nodes, (olsr_cnf->ip_version == AF_INET) ? avl_comp_ipv4 : avl_comp_ipv6);

  /* mandatory */
  abuf_json_string(&json_session, abuf, "type", "NetworkGraph");
  abuf_json_string(&json_session, abuf, "protocol", NETJSON_PROTOCOL);
  abuf_json_string(&json_session, abuf, "version", release_version);
  abuf_json_string(&json_session, abuf, "metric", olsr_cnf->lq_algorithm);

  /* optional */
  abuf_json_string(&json_session, abuf, "revision", olsrd_version);
  // topology_id
  abuf_json_ip_address(&json_session, abuf, "router_id", &olsr_cnf->main_addr);
  // label

  /*
   * Collect all nodes
   */

  /* MID Self */
  node_self = netjson_constructMidSelf(&mid_self);
  netjson_midIntoNodesTree(&nodes, &mid_self);

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    struct mid_entry * entry = mid_set[idx].next;
    while (entry != &mid_set[idx]) {
      netjson_midIntoNodesTree(&nodes, entry);
      entry = entry->next;
    }
  }

  /* TC */
  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    netjson_tcIntoNodesTree(&nodes, tc);
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
     netjson_tcEdgeIntoNodesTree(&nodes, tc_edge);
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  /* LINKS */
  OLSR_FOR_ALL_LINK_ENTRIES(link_entry) {
    netjson_linkIntoNodesTree(&nodes, link_entry, &link_entry->local_iface_addr);
    netjson_linkIntoNodesTree(&nodes, link_entry, &link_entry->neighbor_iface_addr);
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link_entry);

  /* NEIGHBORS */
  OLSR_FOR_ALL_NBR_ENTRIES(neighbor) {
    netjson_neighborIntoNodesTree(&nodes, neighbor);
  } OLSR_FOR_ALL_NBR_ENTRIES_END(neighbor);

  /*
   * Output Nodes
   */

  abuf_json_mark_object(&json_session, true, true, abuf, "nodes");
  while (nodes.count > 0) {
    struct avl_node *node = avl_walk_first(&nodes);
    struct node_entry *node_entry = avlnode2node(node);

    if (!node_entry->isAlias) {
      abuf_json_mark_array_entry(&json_session, true, abuf);

      /* mandatory */
      abuf_json_ip_address(&json_session, abuf, "id", node->key);

      /* optional */
      // label
      if (node_entry->mid) {
        struct mid_address * alias = node_entry->mid->aliases;
        if (alias) {
          abuf_json_mark_object(&json_session, true, true, abuf, "local_addresses");
          while (alias) {
            abuf_json_ip_address(&json_session, abuf, NULL, &alias->alias);
            alias = alias->next_alias;
          }
          abuf_json_mark_object(&json_session, false, true, abuf, NULL);
        }
      } else if (node_entry->tc) {
        /* no local_addresses */
      } else if (node_entry->tc_edge) {
        /* no local_addresses */
      } else if (node_entry->link) {
        /* no local_addresses */
      } else if (node_entry->neighbor) {
        /* no local_addresses */
      }
      // properties

      abuf_json_mark_array_entry(&json_session, false, abuf);
    }

    if (node_entry->mid && (node_entry == node_self)) {
      netjson_cleanup_mid_self(node_self);
    }

    avl_delete(&nodes, node);
    free(node);
  }
  abuf_json_mark_object(&json_session, false, true, abuf, NULL);

  /*
   * Links
   */

  abuf_json_mark_object(&json_session, true, true, abuf, "links");

  OLSR_FOR_ALL_LINK_ENTRIES(link_entry) {
    struct lqtextbuffer lqbuf;
    int link_status = lookup_link_status(link_entry);

    if ((link_status != ASYM_LINK) && (link_status != SYM_LINK)) {
      continue;
    }

    /*
     * link from node to neighbor
     */
    if (link_status == SYM_LINK) {
      abuf_json_mark_array_entry(&json_session, true, abuf);

      /* mandatory */
      abuf_json_ip_address(&json_session, abuf, "source", &link_entry->local_iface_addr);
      abuf_json_ip_address(&json_session, abuf, "target", &link_entry->neighbor_iface_addr);
      abuf_json_float(&json_session, abuf, "cost", get_linkcost_scaled(link_entry->linkcost, false));

      /* optional */
      abuf_json_string(&json_session, abuf, "cost_text", get_linkcost_text(link_entry->linkcost, false, &lqbuf));
      // properties

      abuf_json_mark_array_entry(&json_session, false, abuf);
    }

    /*
     * link from neighbor to node
     */
    abuf_json_mark_array_entry(&json_session, true, abuf);

    /* mandatory */
    abuf_json_ip_address(&json_session, abuf, "source", &link_entry->neighbor_iface_addr);
    abuf_json_ip_address(&json_session, abuf, "target", &link_entry->local_iface_addr);
    abuf_json_float(&json_session, abuf, "cost", get_linkcost_scaled(link_entry->linkcost, false));

    /* optional */
    abuf_json_string(&json_session, abuf, "cost_text", get_linkcost_text(link_entry->linkcost, false, &lqbuf));
    // properties

    abuf_json_mark_array_entry(&json_session, false, abuf);
  } OLSR_FOR_ALL_LINK_ENTRIES_END(my_link);

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      struct lqtextbuffer lqbuf;

      if (ipequal(&olsr_cnf->main_addr, &tc->addr)) {
        continue;
      }

      abuf_json_mark_array_entry(&json_session, true, abuf);

      /* mandatory */
      abuf_json_ip_address(&json_session, abuf, "source", &tc->addr);
      abuf_json_ip_address(&json_session, abuf, "target", &tc_edge->T_dest_addr);
      abuf_json_float(&json_session, abuf, "cost", get_linkcost_scaled(tc_edge->cost, false));

      /* optional */
      abuf_json_string(&json_session, abuf, "cost_text", get_linkcost_text(tc_edge->cost, false, &lqbuf));
      // properties

      abuf_json_mark_array_entry(&json_session, false, abuf);
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}

void ipc_print_network_collection(struct autobuf *abuf) {
  /* mandatory */
  abuf_json_string(&json_session, abuf, "type", "NetworkCollection");

  abuf_json_mark_object(&json_session, true, true, abuf, "collection");

  abuf_json_mark_array_entry(&json_session, true, abuf);
  ipc_print_network_routes(abuf);
  abuf_json_mark_array_entry(&json_session, false, abuf);

  abuf_json_mark_array_entry(&json_session, true, abuf);
  ipc_print_network_graph(abuf);
  abuf_json_mark_array_entry(&json_session, false, abuf);

  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}
