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

#include "olsrd_jsoninfo.h"

#include <unistd.h>
#include <ctype.h>
#include <libgen.h>

#include "cfgparser/olsrd_conf_checksum.h"
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
#include "info/json_helpers.h"
#include "gateway_default_handler.h"
#include "egressTypes.h"
#include "nmealib/info.h"
#include "nmealib/sentence.h"

#define UUIDLEN 256
char uuid[UUIDLEN];

static struct json_session json_session;

struct timeval start_time;

static int read_uuid_from_file(const char * name, const char *file) {
  FILE *f;
  char* end;
  int r = 0;
  size_t chars;

  assert(name);
  assert(file);

  memset(uuid, 0, sizeof(uuid));

  f = fopen(file, "r");
  olsr_printf(1, "(%s) Reading UUID from '%s'\n", name, file);
  if (!f) {
    olsr_printf(1, "(%s) Could not open '%s': %s\n", name, file, strerror(errno));
    return -1;
  }
  chars = fread(uuid, 1, sizeof(uuid) - 1, f);
  if (chars > 0) {
    uuid[chars] = '\0'; /* null-terminate the string */

    /* we only use the first line of the file */
    end = strchr(uuid, '\n');
    if (end)
      *end = 0;
    r = 0;
  } else {
    olsr_printf(1, "(%s) Could not read UUID from '%s': %s\n", name, file, strerror(errno));
    r = -1;
  }

  fclose(f);
  return r;
}

void plugin_init(const char *plugin_name) {
  /* Get start time */
  gettimeofday(&start_time, NULL);

  if (!strlen(uuidfile)) {
    strscpy(uuidfile, "uuid.txt", sizeof(uuidfile));
  }
  read_uuid_from_file(plugin_name, uuidfile);
}

unsigned long long get_supported_commands_mask(void) {
  return SIW_ALL | SIW_OLSRD_CONF;
}

bool isCommand(const char *str, unsigned long long siw) {
  const char * cmd;
  switch (siw) {
    case SIW_OLSRD_CONF:
      cmd = "/olsrd.conf";
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
      cmd = "/neighbors";
      break;

    case SIW_LINKS:
      cmd = "/links";
      break;

    case SIW_ROUTES:
      cmd = "/routes";
      break;

    case SIW_HNA:
      cmd = "/hna";
      break;

    case SIW_MID:
      cmd = "/mid";
      break;

    case SIW_TOPOLOGY:
      cmd = "/topology";
      break;

    case SIW_GATEWAYS:
      cmd = "/gateways";
      break;

    case SIW_INTERFACES:
      cmd = "/interfaces";
      break;

    case SIW_2HOP:
      cmd = "/2hop";
      break;

    case SIW_SGW:
      cmd = "/sgw";
      break;

    case SIW_PUD_POSITION:
      cmd = "/pudposition";
      break;

    case SIW_VERSION:
      cmd = "/version";
      break;

    case SIW_CONFIG:
      cmd = "/config";
      break;

    case SIW_PLUGINS:
      cmd = "/plugins";
      break;

    case SIW_NEIGHBORS_FREIFUNK:
      cmd = "/neighbours";
      break;

    default:
      return false;
  }

  return !strcmp(str, cmd);
}

const char * determine_mime_type(unsigned int send_what) {
  return (send_what & SIW_OLSRD_CONF) ? "text/plain; charset=utf-8" : "application/vnd.api+json";
}

void output_start(struct autobuf *abuf) {
  char *str;

  /* global variables for tracking when to put a comma in for JSON */
  abuf_json_reset_entry_number_and_depth(&json_session, pretty);
  abuf_json_mark_output(&json_session, true, abuf);

  abuf_json_int(&json_session, abuf, "pid", getpid());
  abuf_json_int(&json_session, abuf, "systemTime", time(NULL));
  abuf_json_int(&json_session, abuf, "timeSinceStartup", now_times);

  olsrd_config_checksum_get(NULL, &str);
  abuf_json_string(&json_session, abuf, "configurationChecksum", str);

  if (*uuid) {
    abuf_json_string(&json_session, abuf, "uuid", uuid);
  }
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

static void print_msg_params(struct json_session *session, struct autobuf *abuf, struct olsr_msg_params *params, const char * name) {
  assert(session);
  assert(abuf);
  assert(params);
  assert(name);

  abuf_json_mark_object(session, true, false, abuf, name);
  abuf_json_float(session, abuf, "emissionInterval", params->emission_interval);
  abuf_json_float(session, abuf, "validityTime", params->validity_time);
  abuf_json_mark_object(session, false, false, abuf, NULL);
}

static void print_hna_array_entry(struct json_session *session, struct autobuf *abuf, union olsr_ip_addr *gw, union olsr_ip_addr *ip, uint8_t prefix_len, long long validityTime) {
  assert(session);
  assert(abuf);

  abuf_json_mark_array_entry(session, true, abuf);
  abuf_json_ip_address(session, abuf, "gateway", gw);
  abuf_json_ip_address(session, abuf, "destination", ip);
  abuf_json_int(session, abuf, "genmask", prefix_len);
  abuf_json_int(session, abuf, "validityTime", validityTime);
  abuf_json_mark_array_entry(session, false, abuf);
}

static void print_link_quality_multipliers_array_entry(struct json_session *session, struct autobuf *abuf, struct olsr_lq_mult *mult) {
  assert(session);
  assert(abuf);
  assert(mult);

  abuf_json_mark_array_entry(session, true, abuf);
  abuf_json_ip_address(session, abuf, "route", &mult->addr);
  abuf_json_float(session, abuf, "multiplier", mult->value / 65535.0);
  abuf_json_mark_array_entry(session, false, abuf);
}

static void print_ipc_net_array_entry(struct json_session *session, struct autobuf *abuf, struct ip_prefix_list *ipc_nets) {
  assert(session);
  assert(abuf);
  assert(ipc_nets);

  abuf_json_mark_array_entry(session, true, abuf);
  abuf_json_boolean(session, abuf, "host", (ipc_nets->net.prefix_len == olsr_cnf->maxplen));
  abuf_json_ip_address(session, abuf, "ipAddress", &ipc_nets->net.prefix);
  abuf_json_int(session, abuf, "genmask", ipc_nets->net.prefix_len);
  abuf_json_mark_array_entry(session, false, abuf);
}

static void print_interface_config(struct json_session *session, struct autobuf *abuf, const char * name, struct if_config_options* id) {
  assert(session);
  assert(abuf);
  assert(name);

  abuf_json_mark_object(session, true, false, abuf, name);
  if (id) {
    struct olsr_lq_mult *mult;

    abuf_json_ip_address(session, abuf, "ipv4Broadcast", &id->ipv4_multicast);
    abuf_json_ip_address(session, abuf, "ipv6Multicast", &id->ipv6_multicast);

    abuf_json_ip_address(session, abuf, "ipv4Source", &id->ipv4_src);
    abuf_json_ip_address(session, abuf, "ipv6Source", &id->ipv6_src.prefix);
    abuf_json_int(session, abuf, "ipv6SourcePrefixLength", id->ipv6_src.prefix_len);

    abuf_json_string(session, abuf, "mode", ((id->mode < IF_MODE_MESH) || (id->mode >= IF_MODE_CNT)) ? "" : OLSR_IF_MODE[id->mode]);

    abuf_json_int(session, abuf, "weightValue", id->weight.value);
    abuf_json_boolean(session, abuf, "weightFixed", id->weight.fixed);
    print_msg_params(session, abuf, &id->hello_params, "hello");
    print_msg_params(session, abuf, &id->tc_params, "tc");
    print_msg_params(session, abuf, &id->mid_params, "mid");
    print_msg_params(session, abuf, &id->hna_params, "hna");
    abuf_json_mark_object(session, true, true, abuf, "linkQualityMultipliers");
    for (mult = olsr_cnf->interface_defaults->lq_mult; mult != NULL ; mult = mult->next) {
      print_link_quality_multipliers_array_entry(session, abuf, mult);
    }
    abuf_json_mark_object(session, false, true, abuf, NULL);
    abuf_json_int(session, abuf, "linkQualityMultipliersCount", id->orig_lq_mult_cnt);
    abuf_json_boolean(session, abuf, "autoDetectChanges", id->autodetect_chg);
  }
  abuf_json_mark_object(session, false, false, abuf, NULL);
}

static void print_interface_olsr(struct json_session *session, struct autobuf *abuf, const char * name, struct interface_olsr * rifs) {
  assert(session);
  assert(abuf);
  assert(name);

  abuf_json_mark_object(session, true, false, abuf, name);
  abuf_json_boolean(session, abuf, "up", rifs != NULL);
  if (!rifs) {
    abuf_json_mark_object(session, false, false, abuf, NULL);
    return;
  }

  abuf_json_ip_address46(session, abuf, "ipv4Address", &rifs->int_addr.sin_addr, AF_INET);
  abuf_json_ip_address46(session, abuf, "ipv4Netmask", &rifs->int_netmask.sin_addr, AF_INET);
  abuf_json_ip_address46(session, abuf, "ipv4Broadcast", &rifs->int_broadaddr.sin_addr, AF_INET);
  abuf_json_string(session, abuf, "mode", ((rifs->mode < IF_MODE_MESH) || (rifs->mode >= IF_MODE_CNT)) ? "" : OLSR_IF_MODE[rifs->mode]);

  abuf_json_ip_address46(session, abuf, "ipv6Address", &rifs->int6_addr.sin6_addr, AF_INET6);
  abuf_json_ip_address46(session, abuf, "ipv6Multicast", &rifs->int6_multaddr.sin6_addr, AF_INET6);

  abuf_json_ip_address(session, abuf, "ipAddress", &rifs->ip_addr);
  abuf_json_boolean(session, abuf, "emulatedInterface", rifs->is_hcif);

  abuf_json_int(session, abuf, "olsrSocket", rifs->olsr_socket);
  abuf_json_int(session, abuf, "sendSocket", rifs->send_socket);

  abuf_json_int(session, abuf, "metric", rifs->int_metric);
  abuf_json_int(session, abuf, "mtu", rifs->int_mtu);
  abuf_json_int(session, abuf, "flags", rifs->int_flags);
  abuf_json_int(session, abuf, "index", rifs->if_index);
  abuf_json_boolean(session, abuf, "wireless", rifs->is_wireless);
  abuf_json_string(session, abuf, "name", rifs->int_name);
  abuf_json_int(session, abuf, "seqNum", rifs->olsr_seqnum);


  abuf_json_mark_object(session, true, false, abuf, "messageTimes");
  abuf_json_int(session, abuf, "hello", rifs->hello_gen_timer ? (long) (rifs->hello_gen_timer->timer_clock - now_times) : 0);
  abuf_json_int(session, abuf, "tc", rifs->tc_gen_timer ? (long) (rifs->tc_gen_timer->timer_clock - now_times) : 0);
  abuf_json_int(session, abuf, "mid", rifs->mid_gen_timer ? (long) (rifs->mid_gen_timer->timer_clock - now_times) : 0);
  abuf_json_int(session, abuf, "hna", rifs->hna_gen_timer ? (long) (rifs->hna_gen_timer->timer_clock - now_times) : 0);
  abuf_json_mark_object(session, false, false, abuf, NULL);

#ifdef __linux__




  abuf_json_boolean(session, abuf, "icmpRedirectBackup", rifs->nic_state.redirect);


  abuf_json_boolean(session, abuf, "spoofFilterBackup", rifs->nic_state.spoof);

#endif /* __linux__ */

  abuf_json_int(session, abuf, "helloEmissionInterval", rifs->hello_etime);
  abuf_json_mark_object(session, true, false, abuf, "validityTimes");
  abuf_json_int(session, abuf, "hello", me_to_reltime(rifs->valtimes.hello));
  abuf_json_int(session, abuf, "tc", me_to_reltime(rifs->valtimes.tc));
  abuf_json_int(session, abuf, "mid", me_to_reltime(rifs->valtimes.mid));
  abuf_json_int(session, abuf, "hna", me_to_reltime(rifs->valtimes.hna));
  abuf_json_mark_object(session, false, false, abuf, NULL);

  abuf_json_int(session, abuf, "forwardingTimeout", rifs->fwdtimer);


  abuf_json_int(session, abuf, "sgwZeroBwTimeout", rifs->sgw_sgw_zero_bw_timeout);


  // netbuf


  // gen_properties


  abuf_json_int(session, abuf, "ttlIndex", rifs->ttl_index);


  abuf_json_boolean(session, abuf, "immediateSendTc", rifs->immediate_send_tc);

  abuf_json_mark_object(session, false, false, abuf, NULL);
}

#ifdef __linux__
static void ipc_print_gateway_entry(struct json_session *session, struct autobuf *abuf, bool ipv6, struct gateway_entry * current_gw, struct gateway_entry * gw) {
  struct tc_entry* tc;

  assert(abuf);
  assert(gw);

  tc = olsr_lookup_tc_entry(&gw->originator);

  abuf_json_boolean(session, abuf, "selected", current_gw && (current_gw == gw));
  abuf_json_boolean(session, abuf, "selectable", isGwSelectable(gw, ipv6));
  abuf_json_ip_address(session, abuf, "originator", &gw->originator);
  abuf_json_ip_address(session, abuf, "prefix", &gw->external_prefix.prefix);
  abuf_json_int(session, abuf, "prefixLen", gw->external_prefix.prefix_len);
  abuf_json_int(session, abuf, "uplink", gw->uplink);
  abuf_json_int(session, abuf, "downlink", gw->downlink);
  abuf_json_float(session, abuf, "cost", get_gwcost_scaled(gw->path_cost));
  abuf_json_boolean(session, abuf, "IPv4", gw->ipv4);
  abuf_json_boolean(session, abuf, "IPv4-NAT", gw->ipv4nat);
  abuf_json_boolean(session, abuf, "IPv6", gw->ipv6);
  abuf_json_int(session, abuf, "expireTime", gw->expire_timer ? (gw->expire_timer->timer_clock - now_times) : 0);
  abuf_json_int(session, abuf, "cleanupTime", gw->cleanup_timer ? (gw->cleanup_timer->timer_clock - now_times) : 0);

  abuf_json_float(session, abuf, "pathcost", get_linkcost_scaled(!tc ? ROUTE_COST_BROKEN : tc->path_cost, true));
  abuf_json_int(session, abuf, "hops", !tc ? 0 : tc->hops);
}
#endif /* __linux__ */

static void ipc_print_neighbors_internal(struct json_session *session, struct autobuf *abuf, bool list_2hop) {
  struct neighbor_entry *neigh;

  assert(abuf);

  if (!list_2hop) {
    abuf_json_mark_object(session, true, true, abuf, "neighbors");
  } else {
    abuf_json_mark_object(session, true, true, abuf, "2hop");
  }

  /* Neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {
    struct neighbor_2_list_entry *list_2;
    int thop_cnt = 0;

    abuf_json_mark_array_entry(session, true, abuf);

    abuf_json_ip_address(session, abuf, "ipAddress", &neigh->neighbor_main_addr);
    abuf_json_boolean(session, abuf, "symmetric", (neigh->status == SYM));
    abuf_json_int(session, abuf, "willingness", neigh->willingness);
    abuf_json_boolean(session, abuf, "isMultiPointRelay", neigh->is_mpr);
    abuf_json_boolean(session, abuf, "wasMultiPointRelay", neigh->was_mpr);
    abuf_json_boolean(session, abuf, "multiPointRelaySelector", olsr_lookup_mprs_set(&neigh->neighbor_main_addr) != NULL);
    abuf_json_boolean(session, abuf, "skip", neigh->skip);
    abuf_json_int(session, abuf, "neighbor2nocov", neigh->neighbor_2_nocov);
    abuf_json_int(session, abuf, "linkcount", neigh->linkcount);

    if (list_2hop) {
      abuf_json_mark_object(session, true, true, abuf, "twoHopNeighbors");
    }

    thop_cnt = 0;
    for (list_2 = neigh->neighbor_2_list.next; list_2 != &neigh->neighbor_2_list; list_2 = list_2->next) {
      if (list_2hop && list_2->neighbor_2) {
        abuf_json_ip_address(session, abuf, NULL, &list_2->neighbor_2->neighbor_2_addr);
      }
      thop_cnt++;
    }

    if (list_2hop) {
      abuf_json_mark_object(session, false, true, abuf, NULL);
    }
    abuf_json_int(session, abuf, "twoHopNeighborCount", thop_cnt);

    abuf_json_mark_array_entry(session, false, abuf);
  } OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);
  abuf_json_mark_object(session, false, true, abuf, NULL);
}

void ipc_print_neighbors(struct autobuf *abuf) {
  ipc_print_neighbors_internal(&json_session, abuf, false);
}

void ipc_print_links(struct autobuf *abuf) {
  struct link_entry *my_link;

  abuf_json_mark_object(&json_session, true, true, abuf, "links");

  OLSR_FOR_ALL_LINK_ENTRIES(my_link) {
    struct lqtextbuffer lqBuffer;
    const char* lqString = get_link_entry_text(my_link, '\t', &lqBuffer);
    char * nlqString = strrchr(lqString, '\t');

    if (nlqString) {
      *nlqString = '\0';
      nlqString++;
    }

    abuf_json_mark_array_entry(&json_session, true, abuf);

    abuf_json_ip_address(&json_session, abuf, "localIP", &my_link->local_iface_addr);
    abuf_json_ip_address(&json_session, abuf, "remoteIP", &my_link->neighbor_iface_addr);
    abuf_json_string(&json_session, abuf, "olsrInterface", (my_link->inter && my_link->inter->int_name) ? my_link->inter->int_name : "");
    abuf_json_string(&json_session, abuf, "ifName", my_link->if_name ? my_link->if_name : "");
    abuf_json_int(&json_session, abuf, "validityTime", my_link->link_timer ? (long) (my_link->link_timer->timer_clock - now_times) : 0);
    abuf_json_int(&json_session, abuf, "symmetryTime", my_link->link_sym_timer ? (long) (my_link->link_sym_timer->timer_clock - now_times) : 0);
    abuf_json_int(&json_session, abuf, "asymmetryTime", my_link->ASYM_time);
    abuf_json_int(&json_session, abuf, "vtime", (long) my_link->vtime);
    // neighbor (no need to print, can be looked up via neighbours)
    abuf_json_string(&json_session, abuf, "currentLinkStatus", linkTypeToString(lookup_link_status(my_link)));
    abuf_json_string(&json_session, abuf, "previousLinkStatus", linkTypeToString(my_link->prev_status));

    abuf_json_float(&json_session, abuf, "hysteresis", my_link->L_link_quality);
    abuf_json_boolean(&json_session, abuf, "pending", my_link->L_link_pending != 0);
    abuf_json_int(&json_session, abuf, "lostLinkTime", (long) my_link->L_LOST_LINK_time);
    abuf_json_int(&json_session, abuf, "helloTime", my_link->link_hello_timer ? (long) (my_link->link_hello_timer->timer_clock - now_times) : 0);
    abuf_json_int(&json_session, abuf, "lastHelloTime", (long) my_link->last_htime);
    abuf_json_boolean(&json_session, abuf, "seqnoValid", my_link->olsr_seqno_valid);
    abuf_json_int(&json_session, abuf, "seqno", my_link->olsr_seqno);

    abuf_json_int(&json_session, abuf, "lossHelloInterval", (long) my_link->loss_helloint);
    abuf_json_int(&json_session, abuf, "lossTime", my_link->link_loss_timer ? (long) (my_link->link_loss_timer->timer_clock - now_times) : 0);

    abuf_json_int(&json_session, abuf, "lossMultiplier", (long) my_link->loss_link_multiplier);

    abuf_json_float(&json_session, abuf, "linkCost", get_linkcost_scaled(my_link->linkcost, false));

    abuf_json_float(&json_session, abuf, "linkQuality", atof(lqString));
    abuf_json_float(&json_session, abuf, "neighborLinkQuality", nlqString ? atof(nlqString) : 0.0);

    abuf_json_mark_array_entry(&json_session, false, abuf);
  } OLSR_FOR_ALL_LINK_ENTRIES_END(my_link);
  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}

void ipc_print_routes(struct autobuf *abuf) {
  struct rt_entry *rt;

  abuf_json_mark_object(&json_session, true, true, abuf, "routes");

  /* Walk the route table */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    if (rt->rt_best) {
      abuf_json_mark_array_entry(&json_session, true, abuf);
      abuf_json_ip_address(&json_session, abuf, "destination", &rt->rt_dst.prefix);
      abuf_json_int(&json_session, abuf, "genmask", rt->rt_dst.prefix_len);
      abuf_json_ip_address(&json_session, abuf, "gateway", &rt->rt_best->rtp_nexthop.gateway);
      abuf_json_int(&json_session, abuf, "metric", rt->rt_best->rtp_metric.hops);
      abuf_json_float(&json_session, abuf, "etx", get_linkcost_scaled(rt->rt_best->rtp_metric.cost, true));
      abuf_json_float(&json_session, abuf, "rtpMetricCost", get_linkcost_scaled(rt->rt_best->rtp_metric.cost, true));
      abuf_json_string(&json_session, abuf, "networkInterface", if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
      abuf_json_mark_array_entry(&json_session, false, abuf);
    }
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);

  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}

void ipc_print_topology(struct autobuf *abuf) {
  struct tc_entry *tc;

  abuf_json_mark_object(&json_session, true, true, abuf, "topology");

  /* Topology */
  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      if (tc_edge->edge_inv) {
        struct lqtextbuffer lqbuffer;
        const char* lqString = get_tc_edge_entry_text(tc_edge, '\t', &lqbuffer);
        char * nlqString = strrchr(lqString, '\t');

        if (nlqString) {
          *nlqString = '\0';
          nlqString++;
        }

        abuf_json_mark_array_entry(&json_session, true, abuf);

        // vertex_node
        abuf_json_ip_address(&json_session, abuf, "lastHopIP", &tc->addr);
        // cand_tree_node
        abuf_json_float(&json_session, abuf, "pathCost", get_linkcost_scaled(tc->path_cost, true));
        // path_list_node
        // edge_tree
        // prefix_tree
        // next_hop
        // edge_gc_timer
        abuf_json_int(&json_session, abuf, "validityTime", tc->validity_timer ? (tc->validity_timer->timer_clock - now_times) : 0);
        abuf_json_int(&json_session, abuf, "refCount", tc->refcount);
        abuf_json_int(&json_session, abuf, "msgSeq", tc->msg_seq);
        abuf_json_int(&json_session, abuf, "msgHops", tc->msg_hops);
        abuf_json_int(&json_session, abuf, "hops", tc->hops);
        abuf_json_int(&json_session, abuf, "ansn", tc->ansn);
        abuf_json_int(&json_session, abuf, "tcIgnored", tc->ignored);

        abuf_json_int(&json_session, abuf, "errSeq", tc->err_seq);
        abuf_json_boolean(&json_session, abuf, "errSeqValid", tc->err_seq_valid);

        // edge_node
        abuf_json_ip_address(&json_session, abuf, "destinationIP", &tc_edge->T_dest_addr);
        // tc
        abuf_json_float(&json_session, abuf, "tcEdgeCost", get_linkcost_scaled(tc_edge->cost, true));
        abuf_json_int(&json_session, abuf, "ansnEdge", tc_edge->ansn);
        abuf_json_float(&json_session, abuf, "linkQuality", atof(lqString));
        abuf_json_float(&json_session, abuf, "neighborLinkQuality", nlqString ? atof(nlqString) : 0.0);

        abuf_json_mark_array_entry(&json_session, false, abuf);
      }
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}

void ipc_print_hna(struct autobuf *abuf) {
  struct ip_prefix_list *hna;
  struct hna_entry *tmp_hna;

  abuf_json_mark_object(&json_session, true, true, abuf, "hna");

  /* Announced HNA entries */
  for (hna = olsr_cnf->hna_entries; hna != NULL ; hna = hna->next) {
    print_hna_array_entry( //
        &json_session, //
        abuf, //
        &olsr_cnf->main_addr, //
        &hna->net.prefix, //
        hna->net.prefix_len, //
        0);
  }

  OLSR_FOR_ALL_HNA_ENTRIES(tmp_hna) {
    struct hna_net *tmp_net;

    /* Check all networks */
    for (tmp_net = tmp_hna->networks.next; tmp_net != &tmp_hna->networks; tmp_net = tmp_net->next) {
      print_hna_array_entry( //
          &json_session, //
          abuf, //
          &tmp_hna->A_gateway_addr, //
          &tmp_net->hna_prefix.prefix, //
          tmp_net->hna_prefix.prefix_len, //
          tmp_net->hna_net_timer ? (tmp_net->hna_net_timer->timer_clock - now_times) : 0);
    }
  } OLSR_FOR_ALL_HNA_ENTRIES_END(tmp_hna);

  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}

void ipc_print_mid(struct autobuf *abuf) {
  int idx;

  abuf_json_mark_object(&json_session, true, true, abuf, "mid");

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    struct mid_entry * entry = mid_set[idx].next;

    while (entry != &mid_set[idx]) {
      abuf_json_mark_array_entry(&json_session, true, abuf);

      abuf_json_mark_object(&json_session, true, false, abuf, "main");
      abuf_json_ip_address(&json_session, abuf, "ipAddress", &entry->main_addr);
      abuf_json_int(&json_session, abuf, "validityTime", entry->mid_timer ? (entry->mid_timer->timer_clock - now_times) : 0);
      abuf_json_mark_object(&json_session, false, false, abuf, NULL); // main

      {
        struct mid_address * alias = entry->aliases;

        abuf_json_mark_object(&json_session, true, true, abuf, "aliases");
        while (alias) {
          abuf_json_mark_array_entry(&json_session, true, abuf);
          abuf_json_ip_address(&json_session, abuf, "ipAddress", &alias->alias);
          abuf_json_int(&json_session, abuf, "validityTime", alias->vtime - now_times);
          abuf_json_mark_array_entry(&json_session, false, abuf);

          alias = alias->next_alias;
        }
        abuf_json_mark_object(&json_session, false, true, abuf, NULL); // aliases
      }
      abuf_json_mark_array_entry(&json_session, false, abuf); // entry

      entry = entry->next;
    }
  }
  abuf_json_mark_object(&json_session, false, true, abuf, NULL); // mid
}

#ifdef __linux__

static void ipc_print_gateways_ipvx(struct json_session *session, struct autobuf *abuf, bool ipv6) {
  assert(abuf);

  abuf_json_mark_object(session, true, true, abuf, ipv6 ? "ipv6" : "ipv4");

  if (olsr_cnf->smart_gw_active) {
    struct gateway_entry * current_gw = olsr_get_inet_gateway(ipv6);
    struct gateway_entry * gw;
    OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
      if ((!ipv6 && !gw->ipv4) || (ipv6 && !gw->ipv6)) {
        /* gw does not advertise the requested IP version */
        continue;
      }

      abuf_json_mark_array_entry(session, true, abuf);
      ipc_print_gateway_entry(session, abuf, ipv6, current_gw, gw);
      abuf_json_mark_array_entry(session, false, abuf);
    } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)
  }

  abuf_json_mark_object(session, false, true, abuf, NULL);
}
#endif /* __linux__ */

void ipc_print_gateways(struct autobuf *abuf) {
#ifndef __linux__
  abuf_json_string(&json_session, abuf, "error", "Gateway mode is only supported in Linux");
#else /* __linux__ */
  abuf_json_mark_object(&json_session, true, false, abuf, "gateways");

  ipc_print_gateways_ipvx(&json_session, abuf, false);
  ipc_print_gateways_ipvx(&json_session, abuf, true);

  abuf_json_mark_object(&json_session, false, false, abuf, NULL);
#endif /* __linux__ */
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
static void sgw_ipvx(struct json_session *session, struct autobuf *abuf, bool ipv6) {
  struct interfaceName * sgwTunnelInterfaceNames;

  assert(abuf);

  sgwTunnelInterfaceNames = !ipv6 ? sgwTunnel4InterfaceNames : sgwTunnel6InterfaceNames;

  abuf_json_mark_object(session, true, true, abuf, ipv6 ? "ipv6" : "ipv4");

  if (olsr_cnf->smart_gw_active && sgwTunnelInterfaceNames) {
    struct gateway_entry * current_gw = olsr_get_inet_gateway(ipv6);
    int i;
    for (i = 0; i < olsr_cnf->smart_gw_use_count; i++) {
      struct interfaceName * node = &sgwTunnelInterfaceNames[i];
      struct gateway_entry * gw = node->gw;

      if (!gw) {
        continue;
      }

      abuf_json_mark_array_entry(session, true, abuf);
      ipc_print_gateway_entry(session, abuf, ipv6, current_gw, gw);
      abuf_json_ip_address(session, abuf, "destination", &gw->originator);
      abuf_json_string(session, abuf, "tunnel", node->name);
      abuf_json_int(session, abuf, "tableNr", node->tableNr);
      abuf_json_int(session, abuf, "ruleNr", node->ruleNr);
      abuf_json_int(session, abuf, "bypassRuleNr", node->bypassRuleNr);
      abuf_json_mark_array_entry(session, false, abuf);
    }
  }

  abuf_json_mark_object(session, false, true, abuf, NULL);
}

static void sgw_egress_bw(struct json_session *session, struct autobuf * abuf, const char * key, struct egress_if_bw * bw) {
  abuf_json_mark_object(session, true, false, abuf, key);

  abuf_json_boolean(session, abuf, "requireNetwork", bw->requireNetwork);
  abuf_json_boolean(session, abuf, "requireGateway", bw->requireGateway);
  abuf_json_int(session, abuf, "egressUk", bw->egressUk);
  abuf_json_int(session, abuf, "egressDk", bw->egressDk);
  abuf_json_float(session, abuf, "pathCost", get_linkcost_scaled(bw->path_cost, true));
  abuf_json_ip_address(session, abuf, "network", &bw->network.prefix);
  abuf_json_int(session, abuf, "networkLength", bw->network.prefix_len);
  abuf_json_ip_address(session, abuf, "gateway", &bw->gateway);
  abuf_json_boolean(session, abuf, "networkSet", bw->networkSet);
  abuf_json_boolean(session, abuf, "gatewaySet", bw->gatewaySet);
  abuf_json_float(session, abuf, "costs", get_gwcost_scaled(bw->costs));

  abuf_json_mark_object(session, false, false, abuf, NULL);
}

static void sgw_egress_route_info(struct json_session *session, struct autobuf * abuf, const char * key, struct sgw_route_info * ri) {
  abuf_json_mark_object(session, true, false, abuf, key);

  abuf_json_boolean(session, abuf, "active", ri->active);
  abuf_json_int(session, abuf, "family", ri->route.family);
  abuf_json_int(session, abuf, "rtTable", ri->route.rttable);
  abuf_json_int(session, abuf, "flags", ri->route.flags);
  abuf_json_int(session, abuf, "scope", ri->route.scope);
  abuf_json_int(session, abuf, "ifIndex", ri->route.if_index);
  abuf_json_int(session, abuf, "metric", ri->route.metric);
  abuf_json_int(session, abuf, "protocol", ri->route.protocol);
  abuf_json_boolean(session, abuf, "srcSet", ri->route.srcSet);
  abuf_json_boolean(session, abuf, "gwSet", ri->route.gwSet);
  abuf_json_boolean(session, abuf, "dstSet", ri->route.dstSet);
  abuf_json_boolean(session, abuf, "delSimilar", ri->route.del_similar);
  abuf_json_boolean(session, abuf, "blackhole", ri->route.blackhole);
  abuf_json_ip_address(session, abuf, "srcStore", &ri->route.srcStore);
  abuf_json_ip_address(session, abuf, "gwStore", &ri->route.gwStore);
  abuf_json_ip_address(session, abuf, "dstStore", &ri->route.dstStore.prefix);
  abuf_json_int(session, abuf, "dstStoreLength", ri->route.dstStore.prefix_len);

  abuf_json_mark_object(session, false, false, abuf, NULL);
}

static void sgw_egress(struct json_session *session, struct autobuf * abuf) {
  struct sgw_egress_if * egress_if;

  abuf_json_mark_object(session, true, true, abuf, "egress");

  egress_if = olsr_cnf->smart_gw_egress_interfaces;
  while (egress_if) {
    abuf_json_mark_array_entry(session, true, abuf);

    abuf_json_boolean(session, abuf, "selected", isEgressSelected(egress_if));
    abuf_json_string(session, abuf, "name", egress_if->name);
    abuf_json_int(session, abuf, "ifIndex", egress_if->if_index);
    abuf_json_int(session, abuf, "tableNr", egress_if->tableNr);
    abuf_json_int(session, abuf, "ruleNr", egress_if->ruleNr);
    abuf_json_int(session, abuf, "bypassRuleNr", egress_if->bypassRuleNr);
    abuf_json_boolean(session, abuf, "upPrevious", egress_if->upPrevious);
    abuf_json_boolean(session, abuf, "upCurrent", egress_if->upCurrent);
    abuf_json_boolean(session, abuf, "upChanged", egress_if->upChanged);
    sgw_egress_bw(session, abuf, "bwPrevious", &egress_if->bwPrevious);
    sgw_egress_bw(session, abuf, "bwCurrent", &egress_if->bwCurrent);
    abuf_json_boolean(session, abuf, "bwCostsChanged", egress_if->bwCostsChanged);
    abuf_json_boolean(session, abuf, "bwNetworkChanged", egress_if->bwNetworkChanged);
    abuf_json_boolean(session, abuf, "bwGatewayChanged", egress_if->bwGatewayChanged);
    abuf_json_boolean(session, abuf, "bwChanged", egress_if->bwChanged);
    sgw_egress_route_info(session, abuf, "networkRouteCurrent", &egress_if->networkRouteCurrent);
    sgw_egress_route_info(session, abuf, "egressRouteCurrent", &egress_if->egressRouteCurrent);
    abuf_json_boolean(session, abuf, "inEgressFile", egress_if->inEgressFile);

    abuf_json_mark_array_entry(session, false, abuf);
    egress_if = egress_if->next;
  }

  abuf_json_mark_object(session, false, true, abuf, NULL);
}
#endif /* __linux__ */

void ipc_print_sgw(struct autobuf *abuf) {
#ifndef __linux__
  abuf_json_string(&json_session, abuf, "error", "Gateway mode is only supported in Linux");
#else
  abuf_json_mark_object(&json_session, true, false, abuf, "sgw");

  sgw_egress(&json_session, abuf);
  sgw_ipvx(&json_session, abuf, false);
  sgw_ipvx(&json_session, abuf, true);

  abuf_json_mark_object(&json_session, false, false, abuf, NULL);
#endif /* __linux__ */
}

void ipc_print_pud_position(struct autobuf *abuf) {
  TransmitGpsInformation * txGpsInfo = olsr_cnf->pud_position;
  char * nodeId;
  NmeaSatellites * satinfo = NULL;
  size_t i = 0;

  abuf_json_mark_object(&json_session, true, false, abuf, "pudPosition");
  if (!txGpsInfo) {
    goto out;
  }

  nodeId = (char *) txGpsInfo->nodeId;

  if (!nodeId || !strlen(nodeId)) {
    abuf_json_ip_address46(&json_session, abuf, "nodeId", &olsr_cnf->main_addr, olsr_cnf->ip_version);
  } else {
    abuf_json_string(&json_session, abuf, "nodeId", (char *) txGpsInfo->nodeId);
  }

  /* utc */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_UTCDATE)) {
    abuf_json_mark_object(&json_session, true, false, abuf, "date");
    abuf_json_int(&json_session, abuf, "year", txGpsInfo->txPosition.nmeaInfo.utc.year);
    abuf_json_int(&json_session, abuf, "month", txGpsInfo->txPosition.nmeaInfo.utc.mon);
    abuf_json_int(&json_session, abuf, "day", txGpsInfo->txPosition.nmeaInfo.utc.day);
    abuf_json_mark_object(&json_session, false, false, abuf, NULL);
  }

  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_UTCTIME)) {
    abuf_json_mark_object(&json_session, true, false, abuf, "time");
    abuf_json_int(&json_session, abuf, "hour", txGpsInfo->txPosition.nmeaInfo.utc.hour);
    abuf_json_int(&json_session, abuf, "minute", txGpsInfo->txPosition.nmeaInfo.utc.min);
    abuf_json_int(&json_session, abuf, "second", txGpsInfo->txPosition.nmeaInfo.utc.sec);
    abuf_json_int(&json_session, abuf, "hsec", txGpsInfo->txPosition.nmeaInfo.utc.hsec);
    abuf_json_mark_object(&json_session, false, false, abuf, NULL);
  }

  /* present */
  abuf_json_mark_object(&json_session, true, true, abuf, "present");
  {
    uint32_t present = txGpsInfo->txPosition.nmeaInfo.present;
    i = 1;
    while (i <= NMEALIB_PRESENT_LAST) {
      const char * s = nmeaInfoFieldToString(present & i);
      if (s) {
        abuf_json_string(&json_session, abuf, NULL, s);
      }
      i <<= 1;
    }
  }
  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  abuf_json_int(&json_session, abuf, "presentValue", txGpsInfo->txPosition.nmeaInfo.present);

  /* smask */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SMASK)) {
    int smask = txGpsInfo->txPosition.nmeaInfo.smask;
    abuf_json_mark_object(&json_session, true, true, abuf, "smask");
    if (smask != NMEALIB_SENTENCE_GPNON) {
      i = 1;
      while (i <= NMEALIB_SENTENCE_LAST) {
        const char * s = nmeaSentenceToPrefix(smask & i);
        if (s) {
          abuf_json_string(&json_session, abuf, NULL, s);
        }
        i <<= 1;
      }
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
    abuf_json_int(&json_session, abuf, "smaskValue", smask);
  }

  /* sig */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SIG)) {
    abuf_json_string(&json_session, abuf, "sig", nmeaInfoSignalToString(txGpsInfo->txPosition.nmeaInfo.sig));
    abuf_json_int(&json_session, abuf, "sigValue", txGpsInfo->txPosition.nmeaInfo.sig);
  }

  /* fix */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_FIX)) {
    abuf_json_string(&json_session, abuf, "fix", nmeaInfoFixToString(txGpsInfo->txPosition.nmeaInfo.fix));
    abuf_json_int(&json_session, abuf, "fixValue", txGpsInfo->txPosition.nmeaInfo.fix);
  }

  /* PDOP */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_PDOP)) {
    abuf_json_float(&json_session, abuf, "pdop", txGpsInfo->txPosition.nmeaInfo.pdop);
  }

  /* HDOP */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_HDOP)) {
    abuf_json_float(&json_session, abuf, "hdop", txGpsInfo->txPosition.nmeaInfo.hdop);
  }

  /* VDOP */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_VDOP)) {
    abuf_json_float(&json_session, abuf, "vdop", txGpsInfo->txPosition.nmeaInfo.vdop);
  }

  /* lat */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_LAT)) {
    abuf_json_float(&json_session, abuf, "latitude", txGpsInfo->txPosition.nmeaInfo.latitude);
  }

  /* lon */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_LON)) {
    abuf_json_float(&json_session, abuf, "longitude", txGpsInfo->txPosition.nmeaInfo.longitude);
  }

  /* elv */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_ELV)) {
    abuf_json_float(&json_session, abuf, "elevation", txGpsInfo->txPosition.nmeaInfo.elevation);
  }

  /* speed */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SPEED)) {
    abuf_json_float(&json_session, abuf, "speed", txGpsInfo->txPosition.nmeaInfo.speed);
  }

  /* track */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_TRACK)) {
    abuf_json_float(&json_session, abuf, "track", txGpsInfo->txPosition.nmeaInfo.track);
  }

  /* mtrack */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_MTRACK)) {
    abuf_json_float(&json_session, abuf, "magneticTrack", txGpsInfo->txPosition.nmeaInfo.mtrack);
  }

  /* magvar */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_MAGVAR)) {
    abuf_json_float(&json_session, abuf, "magneticVariation", txGpsInfo->txPosition.nmeaInfo.magvar);
  }

  /* height */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_HEIGHT)) {
    abuf_json_float(&json_session, abuf, "separation", txGpsInfo->txPosition.nmeaInfo.height);
  }

  /* dgpsage */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_DGPSAGE)) {
    abuf_json_float(&json_session, abuf, "dgpsage", txGpsInfo->txPosition.nmeaInfo.dgpsAge);
  }

  /* dgpssid */
  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_DGPSSID)) {
    abuf_json_int(&json_session, abuf, "dgpssid", txGpsInfo->txPosition.nmeaInfo.dgpsSid);
  }

  /* sats */
  abuf_json_mark_object(&json_session, true, false, abuf, "satellites");

  satinfo = &txGpsInfo->txPosition.nmeaInfo.satellites;

  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SATINUSECOUNT)) {
    abuf_json_int(&json_session, abuf, "inUseCount", satinfo->inUseCount);
  }

  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SATINUSE)) {
    abuf_json_mark_object(&json_session, true, true, abuf, "inUse");
    for (i = 0; i < NMEALIB_MAX_SATELLITES; i++) {
      unsigned int prn = satinfo->inUse[i];
      if (prn) {
        abuf_json_int(&json_session, abuf, NULL, prn);
      }
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  }

  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    abuf_json_int(&json_session, abuf, "inViewCount", satinfo->inViewCount);
  }

  if (nmeaInfoIsPresentAll(txGpsInfo->txPosition.nmeaInfo.present, NMEALIB_PRESENT_SATINVIEW)) {
    abuf_json_mark_object(&json_session, true, true, abuf, "inView");
    for (i = 0; i < NMEALIB_MAX_SATELLITES; i++) {
      NmeaSatellite * sat = &satinfo->inView[i];
      if (!sat->prn) {
        continue;
      }

      abuf_json_mark_object(&json_session, true, false, abuf, NULL);
      abuf_json_int(&json_session, abuf, "id", sat->prn);
      abuf_json_int(&json_session, abuf, "elevation", sat->elevation);
      abuf_json_int(&json_session, abuf, "azimuth", sat->azimuth);
      abuf_json_int(&json_session, abuf, "signal", sat->snr);
      abuf_json_mark_object(&json_session, false, false, abuf, NULL);
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  }

  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  out: abuf_json_mark_object(&json_session, false, false, abuf, NULL);
}

void ipc_print_version(struct autobuf *abuf) {
  abuf_json_mark_object(&json_session, true, false, abuf, "version");

  abuf_json_string(&json_session, abuf, "version", olsrd_version);

  abuf_json_string(&json_session, abuf, "gitDescriptor", git_descriptor);
  abuf_json_string(&json_session, abuf, "gitSha", git_sha);
  abuf_json_string(&json_session, abuf, "releaseVersion", release_version);
  abuf_json_string(&json_session, abuf, "sourceHash", source_hash);

  abuf_json_mark_object(&json_session, false, false, abuf, NULL);
}

void ipc_print_olsrd_conf(struct autobuf *abuf) {
  olsrd_write_cnf_autobuf(abuf, olsr_cnf);
}

void ipc_print_interfaces(struct autobuf *abuf) {
  struct olsr_if *ifs;

  abuf_json_mark_object(&json_session, true, true, abuf, "interfaces");
  for (ifs = olsr_cnf->interfaces; ifs != NULL ; ifs = ifs->next) {
    abuf_json_mark_array_entry(&json_session, true, abuf);
    abuf_json_string(&json_session, abuf, "name", ifs->name);
    abuf_json_boolean(&json_session, abuf, "configured", ifs->configured);
    abuf_json_boolean(&json_session, abuf, "hostEmulation", ifs->host_emul);
    abuf_json_ip_address(&json_session, abuf, "hostEmulationAddress", &ifs->hemu_ip);
    print_interface_olsr(&json_session, abuf, "olsrInterface", ifs->interf);
    print_interface_config(&json_session, abuf, "InterfaceConfiguration", ifs->cnf);
    print_interface_config(&json_session, abuf, "InterfaceConfigurationDefaults", ifs->cnfi);
    abuf_json_mark_array_entry(&json_session, false, abuf);
  }
  abuf_json_mark_object(&json_session, false, true, abuf, NULL); // interfaces
}

void ipc_print_twohop(struct autobuf *abuf) {
  ipc_print_neighbors_internal(&json_session, abuf, true);
}

void ipc_print_config(struct autobuf *abuf) {
  char *str;

  abuf_json_mark_object(&json_session, true, false, abuf, "config");

  olsrd_config_checksum_get(NULL, &str);
  abuf_json_string(&json_session, abuf, "configurationChecksum", str);

  {
    size_t i = 0;
    int argc = 0;
    char **argv = NULL;
    get_argc_argv(&argc, &argv);

    abuf_json_mark_object(&json_session, true, true, abuf, "cli");
    for (i = 0; i < (size_t) argc; i++) {
      abuf_json_string(&json_session, abuf, NULL, argv[i]);
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  }

  abuf_json_string(&json_session, abuf, "configurationFile", olsr_cnf->configuration_file);
  abuf_json_int(&json_session, abuf, "olsrPort", olsr_cnf->olsrport);
  abuf_json_int(&json_session, abuf, "debugLevel", olsr_cnf->debug_level);
  abuf_json_boolean(&json_session, abuf, "noFork", olsr_cnf->no_fork);
  abuf_json_string(&json_session, abuf, "pidFile", olsr_cnf->pidfile);
  abuf_json_boolean(&json_session, abuf, "hostEmulation", olsr_cnf->host_emul);
  abuf_json_int(&json_session, abuf, "ipVersion", (olsr_cnf->ip_version == AF_INET) ? 4 : 6);
  abuf_json_boolean(&json_session, abuf, "allowNoInt", olsr_cnf->allow_no_interfaces);
  abuf_json_int(&json_session, abuf, "tosValue", olsr_cnf->tos);

  abuf_json_int(&json_session, abuf, "rtProto", olsr_cnf->rt_proto);
  abuf_json_mark_object(&json_session, true, false, abuf, "rtTable");
  abuf_json_int(&json_session, abuf, "main", olsr_cnf->rt_table);
  abuf_json_int(&json_session, abuf, "default", olsr_cnf->rt_table_default);
  abuf_json_int(&json_session, abuf, "tunnel", olsr_cnf->rt_table_tunnel);
  abuf_json_int(&json_session, abuf, "priority", olsr_cnf->rt_table_pri);
  abuf_json_int(&json_session, abuf, "tunnelPriority", olsr_cnf->rt_table_tunnel_pri);
  abuf_json_int(&json_session, abuf, "defaultOlsrPriority", olsr_cnf->rt_table_defaultolsr_pri);
  abuf_json_int(&json_session, abuf, "defaultPriority", olsr_cnf->rt_table_default_pri);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  abuf_json_mark_object(&json_session, true, false, abuf, "willingness");
  abuf_json_int(&json_session, abuf, "willingness", olsr_cnf->willingness);
  abuf_json_boolean(&json_session, abuf, "auto", olsr_cnf->willingness_auto);
  abuf_json_float(&json_session, abuf, "updateInterval", olsr_cnf->will_int);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);


  // ipc_connections: later

  abuf_json_mark_object(&json_session, true, false, abuf, "fib");
  abuf_json_string(&json_session, abuf, "metric", ((olsr_cnf->fib_metric < FIBM_FLAT) || (olsr_cnf->fib_metric >= FIBM_CNT)) ? "" : FIB_METRIC_TXT[olsr_cnf->fib_metric]);
  abuf_json_string(&json_session, abuf, "metricDefault", FIB_METRIC_TXT[olsr_cnf->fib_metric_default]);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);


  abuf_json_mark_object(&json_session, true, false, abuf, "hysteresis");
  abuf_json_boolean(&json_session, abuf, "enabled", olsr_cnf->use_hysteresis);
  abuf_json_float(&json_session, abuf, "scaling", olsr_cnf->hysteresis_param.scaling);
  abuf_json_float(&json_session, abuf, "thresholdLow", olsr_cnf->hysteresis_param.thr_low);
  abuf_json_float(&json_session, abuf, "thresholdHigh", olsr_cnf->hysteresis_param.thr_high);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  // plugins: later
  // hna_entries
  {
    struct ip_prefix_list *hna;

    abuf_json_mark_object(&json_session, true, true, abuf, "hna");

    /* Announced HNA entries */
    for (hna = olsr_cnf->hna_entries; hna; hna = hna->next) {
        print_hna_array_entry( //
            &json_session, //
            abuf, //
            &olsr_cnf->main_addr, //
            &hna->net.prefix, //
            hna->net.prefix_len, //
            0);
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  }
  // ipc_nets: later
  // interface_defaults: later
  // interfaces: later
  abuf_json_float(&json_session, abuf, "pollrate", olsr_cnf->pollrate);
  abuf_json_float(&json_session, abuf, "nicChgsPollInt", olsr_cnf->nic_chgs_pollrate);
  abuf_json_boolean(&json_session, abuf, "clearScreen", olsr_cnf->clear_screen);
  abuf_json_int(&json_session, abuf, "tcRedundancy", olsr_cnf->tc_redundancy);
  abuf_json_int(&json_session, abuf, "mprCoverage", olsr_cnf->mpr_coverage);


  abuf_json_mark_object(&json_session, true, false, abuf, "linkQuality");
  abuf_json_int(&json_session, abuf, "level", olsr_cnf->lq_level);
  abuf_json_boolean(&json_session, abuf, "fishEye", olsr_cnf->lq_fish);
  abuf_json_float(&json_session, abuf, "aging", olsr_cnf->lq_aging);
  abuf_json_string(&json_session, abuf, "algorithm", olsr_cnf->lq_algorithm);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  abuf_json_float(&json_session, abuf, "minTCVTime", olsr_cnf->min_tc_vtime);

  abuf_json_boolean(&json_session, abuf, "setIpForward", olsr_cnf->set_ip_forward);

  abuf_json_string(&json_session, abuf, "lockFile", olsr_cnf->lock_file);
  abuf_json_boolean(&json_session, abuf, "useNiit", olsr_cnf->use_niit);

  abuf_json_mark_object(&json_session, true, false, abuf, "smartGateway");
  abuf_json_boolean(&json_session, abuf, "enabled", olsr_cnf->smart_gw_active);
  abuf_json_boolean(&json_session, abuf, "alwaysRemoveServerTunnel", olsr_cnf->smart_gw_always_remove_server_tunnel);
  abuf_json_boolean(&json_session, abuf, "allowNAT", olsr_cnf->smart_gw_allow_nat);
  abuf_json_boolean(&json_session, abuf, "uplinkNAT", olsr_cnf->smart_gw_uplink_nat);
  abuf_json_int(&json_session, abuf, "useCount", olsr_cnf->smart_gw_use_count);
  abuf_json_int(&json_session, abuf, "takeDownPercentage", olsr_cnf->smart_gw_takedown_percentage);
  abuf_json_string(&json_session, abuf, "instanceId", olsr_cnf->smart_gw_instance_id);
  abuf_json_string(&json_session, abuf, "policyRoutingScript", olsr_cnf->smart_gw_policyrouting_script);

  abuf_json_mark_object(&json_session, true, false, abuf, "egress");
  // smart_gw_egress_interfaces
  {
    struct sgw_egress_if * egressif = olsr_cnf->smart_gw_egress_interfaces;

    abuf_json_mark_object(&json_session, true, true, abuf, "interfaces");
    while (egressif) {
      abuf_json_string(&json_session, abuf, NULL, egressif->name);
      egressif = egressif->next;
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  }
  abuf_json_int(&json_session, abuf, "interfacesCount", olsr_cnf->smart_gw_egress_interfaces_count);
  abuf_json_string(&json_session, abuf, "file", olsr_cnf->smart_gw_egress_file);
  abuf_json_int(&json_session, abuf, "filePeriod", olsr_cnf->smart_gw_egress_file_period);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  abuf_json_string(&json_session, abuf, "statusFile", olsr_cnf->smart_gw_status_file);
  abuf_json_int(&json_session, abuf, "tablesOffset", olsr_cnf->smart_gw_offset_tables);
  abuf_json_int(&json_session, abuf, "rulesOffset", olsr_cnf->smart_gw_offset_rules);
  abuf_json_int(&json_session, abuf, "period", olsr_cnf->smart_gw_period);
  abuf_json_int(&json_session, abuf, "stableCount", olsr_cnf->smart_gw_stablecount);
  abuf_json_int(&json_session, abuf, "threshold", olsr_cnf->smart_gw_thresh);

  abuf_json_mark_object(&json_session, true, false, abuf, "costsCalculation");
  abuf_json_int(&json_session, abuf, "exitLinkUp", olsr_cnf->smart_gw_weight_exitlink_up);
  abuf_json_int(&json_session, abuf, "exitLinkDown", olsr_cnf->smart_gw_weight_exitlink_down);
  abuf_json_int(&json_session, abuf, "etx", olsr_cnf->smart_gw_weight_etx);
  abuf_json_int(&json_session, abuf, "dividerEtx", olsr_cnf->smart_gw_divider_etx);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  abuf_json_int(&json_session, abuf, "maxCostMaxEtx", olsr_cnf->smart_gw_path_max_cost_etx_max);
  abuf_json_string(&json_session, abuf, "uplink", ((olsr_cnf->smart_gw_type < GW_UPLINK_NONE) || (olsr_cnf->smart_gw_type >= GW_UPLINK_CNT)) ? "" : GW_UPLINK_TXT[olsr_cnf->smart_gw_type]);

  abuf_json_mark_object(&json_session, true, false, abuf, "bandwidth");
  abuf_json_int(&json_session, abuf, "uplinkKbps", olsr_cnf->smart_gw_uplink);
  abuf_json_int(&json_session, abuf, "downlinkKbps", olsr_cnf->smart_gw_downlink);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  abuf_json_mark_object(&json_session, true, false, abuf, "prefix");
  abuf_json_ip_address(&json_session, abuf, "prefix", &olsr_cnf->smart_gw_prefix.prefix);
  abuf_json_int(&json_session, abuf, "length", olsr_cnf->smart_gw_prefix.prefix_len);
  abuf_json_mark_object(&json_session, false, false, abuf, NULL);

  abuf_json_mark_object(&json_session, false, false, abuf, NULL);


  abuf_json_ip_address(&json_session, abuf, "mainIp", &olsr_cnf->main_addr);
  abuf_json_ip_address(&json_session, abuf, "unicastSourceIpAddress", &olsr_cnf->unicast_src_ip);
  abuf_json_boolean(&json_session, abuf, "srcIpRoutes", olsr_cnf->use_src_ip_routes);


  abuf_json_int(&json_session, abuf, "maxPrefixLength", olsr_cnf->maxplen);
  abuf_json_int(&json_session, abuf, "ipSize", olsr_cnf->ipsize);
  abuf_json_boolean(&json_session, abuf, "delgw", olsr_cnf->del_gws);
  abuf_json_float(&json_session, abuf, "maxSendMessageJitter", olsr_cnf->max_jitter);
  abuf_json_int(&json_session, abuf, "exitValue", olsr_cnf->exit_value);
  abuf_json_float(&json_session, abuf, "maxTcValidTime", olsr_cnf->max_tc_vtime);

  abuf_json_int(&json_session, abuf, "niit4to6InterfaceIndex", olsr_cnf->niit4to6_if_index);
  abuf_json_int(&json_session, abuf, "niit6to4InterfaceIndex", olsr_cnf->niit6to4_if_index);


  abuf_json_boolean(&json_session, abuf, "hasIpv4Gateway", olsr_cnf->has_ipv4_gateway);
  abuf_json_boolean(&json_session, abuf, "hasIpv6Gateway", olsr_cnf->has_ipv6_gateway);

  abuf_json_int(&json_session, abuf, "ioctlSocket", olsr_cnf->ioctl_s);
#ifdef __linux__
  abuf_json_int(&json_session, abuf, "routeNetlinkSocket", olsr_cnf->rtnl_s);
  abuf_json_int(&json_session, abuf, "routeMonitorSocket", olsr_cnf->rt_monitor_socket);
#endif /* __linux__ */

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__
  abuf_json_int(&json_session, abuf, "routeChangeSocket", olsr_cnf->rts);
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__ */
  abuf_json_float(&json_session, abuf, "linkQualityNatThreshold", olsr_cnf->lq_nat_thresh);


  // Other settings
  abuf_json_int(&json_session, abuf, "brokenLinkCost", LINK_COST_BROKEN);
  abuf_json_int(&json_session, abuf, "brokenRouteCost", ROUTE_COST_BROKEN);


  // IpcConnect section
  abuf_json_int(&json_session, abuf, "ipcConnectMaxConnections", olsr_cnf->ipc_connections);
  {
    struct ip_prefix_list *ipc_nets;

    abuf_json_mark_object(&json_session, true, true, abuf, "ipcConnectAllowed");
    for (ipc_nets = olsr_cnf->ipc_nets; ipc_nets; ipc_nets = ipc_nets->next) {
      print_ipc_net_array_entry(&json_session, abuf, ipc_nets);
    }
    abuf_json_mark_object(&json_session, false, true, abuf, NULL);
  }


  // plugins section: use /plugins


  // InterfaceDefaults section
  print_interface_config(&json_session, abuf, "interfaceDefaults", olsr_cnf->interface_defaults);


  // Interface(s) section: use /interfaces


  // OS section
#if defined _WIN32 || defined _WIN64
  abuf_json_string(&json_session, abuf, "os", "Windows");
#elif defined __gnu_linux__
  abuf_json_string(&json_session, abuf, "os", "GNU/Linux");
#elif defined __ANDROID__
  abuf_json_string(&json_session, abuf, "os", "Android");
#elif defined __APPLE__
  abuf_json_string(&json_session, abuf, "os", "Mac OS X");
#elif defined __NetBSD__
  abuf_json_string(&json_session, abuf, "os", "NetBSD");
#elif defined __OpenBSD__
  abuf_json_string(&json_session, abuf, "os", "OpenBSD");
#elif defined __FreeBSD__ || defined __FreeBSD_kernel__
  abuf_json_string(&json_session, abuf, "os", "FreeBSD");
#else /* OS detection */
  abuf_json_string(&json_session, abuf, "os", "Undefined");
#endif /* OS detection */

  abuf_json_int(&json_session, abuf, "startTime", start_time.tv_sec);

  abuf_json_mark_object(&json_session, false, false, abuf, NULL);
}

void ipc_print_plugins(struct autobuf *abuf) {
  abuf_json_mark_object(&json_session, true, true, abuf, "plugins");
  if (olsr_cnf->plugins) {
    struct plugin_entry *plugin;

    for (plugin = olsr_cnf->plugins; plugin; plugin = plugin->next) {
      struct plugin_param *param;

      abuf_json_mark_array_entry(&json_session, true, abuf);
      abuf_json_string(&json_session, abuf, "plugin", plugin->name);

      abuf_json_mark_object(&json_session, true, false, abuf, "parameters");
      for (param = plugin->params; param; param = param->next) {
        abuf_json_string(&json_session, abuf, param->key, param->value);
      }
      abuf_json_mark_object(&json_session, false, false, abuf, NULL);

      abuf_json_mark_array_entry(&json_session, false, abuf);
    }
  }
  abuf_json_mark_object(&json_session, false, true, abuf, NULL);
}
