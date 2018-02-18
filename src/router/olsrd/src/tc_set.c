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

#include "tc_set.h"
#include "ipcalc.h"
#include "mid_set.h"
#include "link_set.h"
#include "olsr.h"
#include "scheduler.h"
#include "olsr_spf.h"
#include "common/avl.h"
#include "lq_packet.h"
#include "net_olsr.h"
#include "lq_plugin.h"
#include "olsr_cookie.h"
#include "duplicate_set.h"
#include "gateway.h"

#include <assert.h>

/* Root of the link state database */
struct avl_tree tc_tree;
struct tc_entry *tc_myself;            /* Shortcut to ourselves */

/* Some cookies for stats keeping */
struct olsr_cookie_info *tc_edge_gc_timer_cookie = NULL;
struct olsr_cookie_info *tc_validity_timer_cookie = NULL;
struct olsr_cookie_info *tc_edge_mem_cookie = NULL;
struct olsr_cookie_info *tc_mem_cookie = NULL;

/*
 * Sven-Ola 2007-Dec: These four constants include an assumption
 * on how long a typical olsrd mesh memorizes (TC) messages in the
 * RAM of all nodes and how many neighbour changes between TC msgs.
 * In Berlin, we encounter hop values up to 70 which means that
 * messages may live up to ~15 minutes cycling between nodes and
 * obviously breaking out of the timeout_dup() jail. It may be more
 * correct to dynamically adapt those constants, e.g. by using the
 * max hop number (denotes size-of-mesh) in some form or maybe
 * a factor indicating how many (old) versions of olsrd are on.
 */

/* Value window for ansn, identifies old messages to be ignored */
#define TC_ANSN_WINDOW 256

/* Value window for seqno, identifies old messages to be ignored */
#define TC_SEQNO_WINDOW 1024

/* Enlarges the value window for upcoming ansn/seqno to be accepted */
#define TC_ANSN_WINDOW_MULT 4

/* Enlarges the value window for upcoming ansn/seqno to be accepted */
#define TC_SEQNO_WINDOW_MULT 8

static bool
olsr_seq_inrange_low(int beg, int end, uint16_t seq)
{
  if (beg < 0) {
    if (seq >= (uint16_t) beg || seq < end) {
      return true;
    }
  } else if (end >= 0x10000) {
    if (seq >= beg || seq < (uint16_t) end) {
      return true;
    }
  } else if (seq >= beg && seq < end) {
    return true;
  }
  return false;
}

static bool
olsr_seq_inrange_high(int beg, int end, uint16_t seq)
{
  if (beg < 0) {
    if (seq > (uint16_t) beg || seq <= end) {
      return true;
    }
  } else if (end >= 0x10000) {
    if (seq > beg || seq <= (uint16_t) end) {
      return true;
    }
  } else if (seq > beg && seq <= end) {
    return true;
  }
  return false;
}

/**
 * Add a new tc_entry to the tc tree
 *
 * @param adr (last)adr address of the entry
 * @return a pointer to the created entry
 */
static struct tc_entry *
olsr_add_tc_entry(union olsr_ip_addr *adr)
{
#ifdef DEBUG
  struct ipaddr_str buf;
#endif /* DEBUG */
  struct tc_entry *tc;

  /*
   * Safety net against loss of the last main IP address.
   */
  if (ipequal(&olsr_cnf->main_addr, &all_zero)) {
    return NULL;
  }
#ifdef DEBUG
  OLSR_PRINTF(1, "TC: add entry %s\n", olsr_ip_to_string(&buf, adr));
#endif /* DEBUG */

  tc = olsr_cookie_malloc(tc_mem_cookie);
  if (!tc) {
    return NULL;
  }

  /* Fill entry */
  tc->addr = *adr;
  tc->vertex_node.key = &tc->addr;
  tc->path_cost = ROUTE_COST_BROKEN;

  /*
   * Insert into the global tc tree.
   */
  avl_insert(&tc_tree, &tc->vertex_node, AVL_DUP_NO);
  olsr_lock_tc_entry(tc);

  /*
   * Initialize subtrees for edges and prefixes.
   */
  avl_init(&tc->edge_tree, avl_comp_default);
  avl_init(&tc->prefix_tree, avl_comp_prefix_default);

  /*
   * Add a rt_path for ourselves.
   */
  olsr_insert_routing_table(adr, olsr_cnf->maxplen, adr, OLSR_RT_ORIGIN_INT);

  return tc;
}

/**
 * Initialize the topology set
 *
 */
void
olsr_init_tc(void)
{
  OLSR_PRINTF(5, "TC: init topo\n");

  avl_init(&tc_tree, avl_comp_default);

  /*
   * Get some cookies for getting stats to ease troubleshooting.
   */
  tc_edge_gc_timer_cookie = olsr_alloc_cookie("TC edge GC", OLSR_COOKIE_TYPE_TIMER);
  tc_validity_timer_cookie = olsr_alloc_cookie("TC validity", OLSR_COOKIE_TYPE_TIMER);

  tc_edge_mem_cookie = olsr_alloc_cookie("tc_edge_entry", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(tc_edge_mem_cookie, sizeof(struct tc_edge_entry) + active_lq_handler->tc_lq_size);

  tc_mem_cookie = olsr_alloc_cookie("tc_entry", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(tc_mem_cookie, sizeof(struct tc_entry));

  /*
   * Add a TC entry for ourselves.
   */
  tc_myself = olsr_add_tc_entry(&olsr_cnf->main_addr);
}

void olsr_delete_all_tc_entries(void) {
  struct tc_entry *tc;

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    olsr_delete_tc_entry(tc);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc)
}

/**
 * The main ip address has changed.
 * Do the needful.
 */
void
olsr_change_myself_tc(void)
{
  if (tc_myself) {

    /*
     * Check if there was a change.
     */
    if (ipequal(&tc_myself->addr, &olsr_cnf->main_addr)) {
      return;
    }

    /*
     * Flush our own tc_entry.
     */
    olsr_delete_tc_entry(tc_myself);
  }

  /*
   * The old entry for ourselves is gone, generate a new one and trigger SPF.
   */
  tc_myself = olsr_add_tc_entry(&olsr_cnf->main_addr);
  changes_topology = true;
}

/*
 * Increment the reference counter.
 */
void
olsr_lock_tc_entry(struct tc_entry *tc)
{
  tc->refcount++;
}

/*
 * Unlock and free a tc_entry once all references are gone.
 */
void
olsr_unlock_tc_entry(struct tc_entry *tc)
{
  if (--tc->refcount) {
    return;
  }

  /*
   * All references are gone.
   */
  olsr_cookie_free(tc_mem_cookie, tc);
}

/**
 * Delete a TC entry.
 *
 * @param tc the TC entry to delete
 */
void
olsr_delete_tc_entry(struct tc_entry *tc)
{
  struct tc_edge_entry *tc_edge;
  struct rt_path *rtp;

  /* delete gateway if available */
#ifdef __linux__
  olsr_delete_gateway_entry(&tc->addr, FORCE_DELETE_GW_ENTRY, false);
#endif /* __linux__ */
  /*
   * Delete the rt_path for ourselves.
   */
  olsr_delete_routing_table(&tc->addr, olsr_cnf->maxplen, &tc->addr);

  /* The edgetree and prefix tree must be empty before */
  OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
    olsr_delete_tc_edge_entry(tc_edge);
  } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);

  OLSR_FOR_ALL_PREFIX_ENTRIES(tc, rtp) {
    olsr_delete_rt_path(rtp);
  } OLSR_FOR_ALL_PREFIX_ENTRIES_END(tc, rtp);

  /* Stop running timers */
  olsr_stop_timer(tc->edge_gc_timer);
  tc->edge_gc_timer = NULL;
  olsr_stop_timer(tc->validity_timer);
  tc->validity_timer = NULL;

  avl_delete(&tc_tree, &tc->vertex_node);
  olsr_unlock_tc_entry(tc);
}

/**
 * Look up a entry from the TC tree based on address
 *
 * @param adr the address to look for
 * @return the entry found or NULL
 */
struct tc_entry *
olsr_lookup_tc_entry(union olsr_ip_addr *adr)
{
  struct avl_node *node;

  node = avl_find(&tc_tree, adr);

  return (node ? vertex_tree2tc(node) : NULL);
}

/*
 * Lookup a tc entry. Creates one if it does not exist yet.
 */
struct tc_entry *
olsr_locate_tc_entry(union olsr_ip_addr *adr)
{
  struct tc_entry *tc;

  if (!(tc = olsr_lookup_tc_entry(adr))) {
    return olsr_add_tc_entry(adr);
  }
  return tc;
}

/**
 * Format tc_edge contents into a buffer.
 */
char *
olsr_tc_edge_to_string(struct tc_edge_entry *tc_edge)
{
  static char buf[128];
  struct ipaddr_str addrbuf, dstbuf;
  struct tc_entry *tc = tc_edge->tc;
  struct lqtextbuffer lqbuffer1, lqbuffer2;

  snprintf(buf, sizeof(buf), "%s > %s, cost (%6s) %s", olsr_ip_to_string(&addrbuf, &tc->addr),
           olsr_ip_to_string(&dstbuf, &tc_edge->T_dest_addr), get_tc_edge_entry_text(tc_edge, '/', &lqbuffer1),
           get_linkcost_text(tc_edge->cost, false, &lqbuffer2));

  return buf;
}

/**
 * Wrapper for the timer callback.
 * A TC entry has not been refreshed in time.
 * Remove it from the link-state database.
 */
static void
olsr_expire_tc_entry(void *context)
{
  struct tc_entry *tc;
  struct ipaddr_str buf;

  tc = (struct tc_entry *)context;

  OLSR_PRINTF(3, "TC: expire node entry %s\n", olsr_ip_to_string(&buf, &tc->addr));

  tc->validity_timer = NULL;

  olsr_delete_tc_entry(tc);
  changes_topology = true;
}

/**
 * Wrapper for the timer callback.
 * Does the garbage collection of older ansn entries after no edge addition to
 * the TC entry has happened for OLSR_TC_EDGE_GC_TIME.
 */
static void
olsr_expire_tc_edge_gc(void *context)
{
  struct tc_entry *tc;
  struct ipaddr_str buf;

  tc = (struct tc_entry *)context;

  OLSR_PRINTF(3, "TC: expire edge entry %s\n", olsr_ip_to_string(&buf, &tc->addr));

  tc->edge_gc_timer = NULL;

  if (olsr_delete_outdated_tc_edges(tc)) {
    changes_topology = true;
  }
}

/*
 * If the edge does not have a minimum acceptable link quality
 * set the etx cost to infinity such that it gets ignored during
 * SPF calculation.
 *
 * @return 1 if the change of the etx value was relevant
 */
bool
olsr_calc_tc_edge_entry_etx(struct tc_edge_entry *tc_edge)
{
  /*
   * Some sanity check before recalculating the etx.
   */
  if (olsr_cnf->lq_level < 1) {
    return false;
  }

  tc_edge->cost = olsr_calc_tc_cost(tc_edge);
  return true;
}

/**
 * Add a new tc_edge_entry to the tc_edge_tree
 *
 * @param tc the tc entry
 * @param addr (last)adr address of the entry
 * @param ansn ansn of this edge
 * @return a pointer to the created entry
 */
struct tc_edge_entry *
olsr_add_tc_edge_entry(struct tc_entry *tc, union olsr_ip_addr *addr, uint16_t ansn)
{
#ifdef DEBUG
  struct ipaddr_str buf;
#endif /* DEBUG */
  struct tc_entry *tc_neighbor;
  struct tc_edge_entry *tc_edge, *tc_edge_inv;

  tc_edge = olsr_cookie_malloc(tc_edge_mem_cookie);
  if (!tc_edge) {
    return NULL;
  }

  /* Fill entry */
  tc_edge->T_dest_addr = *addr;
  tc_edge->ansn = ansn;
  tc_edge->edge_node.key = &tc_edge->T_dest_addr;

  /*
   * Insert into the edge tree.
   */
  avl_insert(&tc->edge_tree, &tc_edge->edge_node, AVL_DUP_NO);
  olsr_lock_tc_entry(tc);

  /*
   * Connect backpointer.
   */
  tc_edge->tc = tc;

  /*
   * Check if the neighboring router and the inverse edge is in the lsdb.
   * Create short cuts to the inverse edge for faster SPF execution.
   */
  tc_neighbor = olsr_lookup_tc_entry(&tc_edge->T_dest_addr);
  if (tc_neighbor) {
#ifdef DEBUG
    OLSR_PRINTF(1, "TC:   found neighbor tc_entry %s\n", olsr_ip_to_string(&buf, &tc_neighbor->addr));
#endif /* DEBUG */

    tc_edge_inv = olsr_lookup_tc_edge(tc_neighbor, &tc->addr);
    if (tc_edge_inv) {
#ifdef DEBUG
      OLSR_PRINTF(1, "TC:   found inverse edge for %s\n", olsr_ip_to_string(&buf, &tc_edge_inv->T_dest_addr));
#endif /* DEBUG */

      /*
       * Connect the edges mutually.
       */
      tc_edge_inv->edge_inv = tc_edge;
      tc_edge->edge_inv = tc_edge_inv;

    }
  }

  /*
   * Update the etx.
   */
  olsr_calc_tc_edge_entry_etx(tc_edge);

#ifdef DEBUG
  OLSR_PRINTF(1, "TC: add edge entry %s\n", olsr_tc_edge_to_string(tc_edge));
#endif /* DEBUG */

  return tc_edge;
}

/**
 * Delete a TC edge entry.
 *
 * @param tc_edge the TC edge entry
 */
void
olsr_delete_tc_edge_entry(struct tc_edge_entry *tc_edge)
{
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge_inv;

#ifdef DEBUG
  OLSR_PRINTF(1, "TC: del edge entry %s\n", olsr_tc_edge_to_string(tc_edge));
#endif /* DEBUG */

  tc = tc_edge->tc;
  avl_delete(&tc->edge_tree, &tc_edge->edge_node);
  olsr_unlock_tc_entry(tc);

  /*
   * Clear the backpointer of our inverse edge.
   */
  tc_edge_inv = tc_edge->edge_inv;
  if (tc_edge_inv) {
    tc_edge_inv->edge_inv = NULL;
  }

  olsr_cookie_free(tc_edge_mem_cookie, tc_edge);
}

/**
 * Delete all destinations that have a lower ANSN.
 *
 * @param tc the entry to delete edges from
 * @return TRUE if any destinations were deleted, FALSE if not
 */
bool
olsr_delete_outdated_tc_edges(struct tc_entry *tc)
{
  struct tc_edge_entry *tc_edge;
  bool retval = false;

  OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
    if (SEQNO_GREATER_THAN(tc->ansn, tc_edge->ansn)) {
      olsr_delete_tc_edge_entry(tc_edge);
      retval = true;
    }
  }
  OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);

  return retval;
}

/**
 * Delete all destinations that are inside the borders but
 * not updated in the last tc.
 *
 * @param tc the entry to delete edges from
 * @param ansn the advertised neighbor set sequence number
 * @param lower_border the lower border
 * @param upper_border the upper border
 * @return 1 if any destinations were deleted 0 if not
 */
static int
olsr_delete_revoked_tc_edges(struct tc_entry *tc, uint16_t ansn, union olsr_ip_addr *lower_border, union olsr_ip_addr *upper_border)
{
  struct tc_edge_entry *tc_edge;
  int retval = 0;

  bool passedLowerBorder = false;

  OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
    if (!passedLowerBorder) {
      if (avl_comp_default(lower_border, &tc_edge->T_dest_addr) <= 0) {
        passedLowerBorder = true;
      } else {
        continue;
      }
    }

    if (passedLowerBorder) {
      if (avl_comp_default(upper_border, &tc_edge->T_dest_addr) <= 0) {
        break;
      }
    }

    if (SEQNO_GREATER_THAN(ansn, tc_edge->ansn)) {
      olsr_delete_tc_edge_entry(tc_edge);
      retval = 1;
    }
  }
  OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);

  if (retval)
    changes_topology = true;
  return retval;
}

/**
 * Update an edge registered on an entry.
 * Creates new edge-entries if not registered.
 * Bases update on a received TC message
 *
 * @param tc the TC entry to check
 * @param ansn the ansn of the edge
 * @param curr pointer to the packet
 * @param neighbor the neighbor of the edge
 * @return 1 if entries are added 0 if not
 */
static int
olsr_tc_update_edge(struct tc_entry *tc, uint16_t ansn, const unsigned char **curr, union olsr_ip_addr *neighbor)
{
  struct tc_edge_entry *tc_edge;
  int edge_change;

  edge_change = 0;

  /*
   * Fetch the per-edge data
   */
  pkt_get_ipaddress(curr, neighbor);

  /* First check if we know this edge */
  tc_edge = olsr_lookup_tc_edge(tc, neighbor);

  if (!tc_edge) {

    /*
     * Yet unknown - create it.
     * Check if the address is allowed.
     */
    if (!olsr_validate_address(neighbor)) {
      return 0;
    }

    tc_edge = olsr_add_tc_edge_entry(tc, neighbor, ansn);

    olsr_deserialize_tc_lq_pair(curr, tc_edge);
    edge_change = 1;

  } else {

    /*
     * We know this edge - Update entry.
     */
    tc_edge->ansn = ansn;

    /*
     * Update link quality if configured.
     */
    if (olsr_cnf->lq_level > 0) {
      olsr_deserialize_tc_lq_pair(curr, tc_edge);
    }

    /*
     * Update the etx.
     */
    if (olsr_calc_tc_edge_entry_etx(tc_edge)) {
      edge_change = 1;
    }
#if defined DEBUG && DEBUG
    if (edge_change) {
      OLSR_PRINTF(1, "TC:   chg edge entry %s\n", olsr_tc_edge_to_string(tc_edge));
    }
#endif /* defined DEBUG && DEBUG */

  }

  return edge_change;
}

/**
 * Lookup an edge hanging off a TC entry.
 *
 * @param tc the entry to check
 * @param edge_addr the destination address to check for
 * @return a pointer to the tc_edge found - or NULL
 */
struct tc_edge_entry *
olsr_lookup_tc_edge(struct tc_entry *tc, union olsr_ip_addr *edge_addr)
{
  struct avl_node *edge_node;

  edge_node = avl_find(&tc->edge_tree, edge_addr);

  return (edge_node ? edge_tree2tc_edge(edge_node) : NULL);
}

/**
 * Print the topology table to stdout
 */
#ifndef NODEBUG
void
olsr_print_tc_table(void)
{
  /* The whole function makes no sense without it. */
  struct tc_entry *tc;
  const int ipwidth = olsr_cnf->ip_version == AF_INET ? (INET_ADDRSTRLEN - 1) : (INET6_ADDRSTRLEN - 1);

  OLSR_PRINTF(1, "\n--- %s ------------------------------------------------- TOPOLOGY\n\n" "%-*s %-*s %-14s  %s\n",
              olsr_wallclock_string(), ipwidth, "Source IP addr", ipwidth, "Dest IP addr", "      LQ      ", "ETX");

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      struct ipaddr_str addrbuf, dstaddrbuf;
      struct lqtextbuffer lqbuffer1, lqbuffer2;

      OLSR_PRINTF(1, "%-*s %-*s %-14s %s\n", ipwidth, olsr_ip_to_string(&addrbuf, &tc->addr), ipwidth,
                  olsr_ip_to_string(&dstaddrbuf, &tc_edge->T_dest_addr), get_tc_edge_entry_text(tc_edge, '/', &lqbuffer1),
                  get_linkcost_text(tc_edge->cost, false, &lqbuffer2));

    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);
}
#endif /* NODEBUG */

/*
 * calculate the border IPs of a tc edge set according to the border flags
 *
 * @param lower border flag
 * @param pointer to lower border ip
 * @param upper border flag
 * @param pointer to upper border ip
 * @result 1 if lower/upper border ip have been set
 */
static int
olsr_calculate_tc_border(uint8_t lower_border, union olsr_ip_addr *lower_border_ip, uint8_t upper_border,
                         union olsr_ip_addr *upper_border_ip)
{
  if (lower_border == 0 && upper_border == 0) {
    return 0;
  }
  if (lower_border == 0xff) {
    memset(lower_border_ip, 0, sizeof(*lower_border_ip));
  } else {
    int i;

    lower_border--;
    for (i = 0; i < lower_border / 8; i++) {
      lower_border_ip->v6.s6_addr[olsr_cnf->ipsize - i - 1] = 0;
    }
    lower_border_ip->v6.s6_addr[olsr_cnf->ipsize - lower_border / 8 - 1] &= (0xffu << (lower_border & 7));
    lower_border_ip->v6.s6_addr[olsr_cnf->ipsize - lower_border / 8 - 1] |= (1u << (lower_border & 7));
  }

  if (upper_border == 0xff) {
    memset(upper_border_ip, 0xff, sizeof(*upper_border_ip));
  } else {
    int i;

    upper_border--;

    for (i = 0; i < upper_border / 8; i++) {
      upper_border_ip->v6.s6_addr[olsr_cnf->ipsize - i - 1] = 0;
    }
    upper_border_ip->v6.s6_addr[olsr_cnf->ipsize - upper_border / 8 - 1] &= (0xffu << (upper_border & 7));
    upper_border_ip->v6.s6_addr[olsr_cnf->ipsize - upper_border / 8 - 1] |= (1u << (upper_border & 7));
  }
  return 1;
}

/*
 * Process an incoming TC or TC_LQ message.
 *
 * If the message is interesting enough, update our edges for it,
 * trigger SPF and finally flood it to all our 2way neighbors.
 *
 * The order for extracting data off the message does matter,
 * as every call to pkt_get increases the packet offset and
 * hence the spot we are looking at.
 */
bool
olsr_input_tc(union olsr_message * msg, struct interface_olsr * input_if __attribute__ ((unused)), union olsr_ip_addr * from_addr)
{
  struct ipaddr_str buf;
  uint16_t size, msg_seq, ansn;
  uint8_t type, ttl, msg_hops, lower_border, upper_border;
  olsr_reltime vtime;
  union olsr_ip_addr originator;
  const unsigned char *limit, *curr;
  struct tc_entry *tc;
  bool emptyTC;

  union olsr_ip_addr lower_border_ip, upper_border_ip;
  int borderSet = 0;

  curr = (void *)msg;
  if (!msg) {
    return false;
  }

  /* We are only interested in TC message types. */
  pkt_get_u8(&curr, &type);
  if ((type != LQ_TC_MESSAGE) && (type != TC_MESSAGE)) {
    return false;
  }

  /*
   * If the sender interface (NB: not originator) of this message
   * is not in the symmetric 1-hop neighborhood of this node, the
   * message MUST be discarded.
   */
  if (check_neighbor_link(from_addr) != SYM_LINK) {
    OLSR_PRINTF(2, "Received TC from NON SYM neighbor %s\n", olsr_ip_to_string(&buf, from_addr));
    return false;
  }

  pkt_get_reltime(&curr, &vtime);
  pkt_get_u16(&curr, &size);

  pkt_get_ipaddress(&curr, &originator);

  /* Copy header values */
  pkt_get_u8(&curr, &ttl);
  pkt_get_u8(&curr, &msg_hops);
  pkt_get_u16(&curr, &msg_seq);
  pkt_get_u16(&curr, &ansn);

  /* Get borders */
  pkt_get_u8(&curr, &lower_border);
  pkt_get_u8(&curr, &upper_border);

  tc = olsr_lookup_tc_entry(&originator);

  if (vtime < (olsr_reltime)(olsr_cnf->min_tc_vtime*1000)) {
	  vtime = (olsr_reltime)(olsr_cnf->min_tc_vtime*1000);
  }

  if (tc && 0 != tc->edge_tree.count) {
    if (olsr_seq_inrange_high((int)tc->msg_seq - TC_SEQNO_WINDOW, tc->msg_seq, msg_seq)
        && olsr_seq_inrange_high((int)tc->ansn - TC_ANSN_WINDOW, tc->ansn, ansn)) {

      /*
       * Ignore already seen seq/ansn values (small window for mesh memory)
       */
      if ((tc->msg_seq == msg_seq) || (tc->ignored++ < 32)) {
        return false;
      }

      OLSR_PRINTF(1, "Ignored to much LQTC's for %s, restarting\n", olsr_ip_to_string(&buf, &originator));

    } else if (!olsr_seq_inrange_high(tc->msg_seq, (int)tc->msg_seq + TC_SEQNO_WINDOW * TC_SEQNO_WINDOW_MULT, msg_seq)
               || !olsr_seq_inrange_low(tc->ansn, (int)tc->ansn + TC_ANSN_WINDOW * TC_ANSN_WINDOW_MULT, ansn)) {

      /*
       * Only accept future seq/ansn values (large window for node reconnects).
       * Restart in all other cases. Ignore a single stray message.
       */
      if (!tc->err_seq_valid) {
        tc->err_seq = msg_seq;
        tc->err_seq_valid = true;
      }
      if (tc->err_seq == msg_seq) {
        return false;
      }

      OLSR_PRINTF(2, "Detected node restart for %s\n", olsr_ip_to_string(&buf, &originator));
    }
  }

  /*
   * Generate a new tc_entry in the lsdb and store the sequence number.
   */
  if (!tc) {
    tc = olsr_add_tc_entry(&originator);
  }

  /*
   * Update the tc entry.
   */
  tc->msg_hops = msg_hops;
  tc->msg_seq = msg_seq;
  tc->ansn = ansn;
  tc->ignored = 0;
  tc->err_seq_valid = false;

  OLSR_PRINTF(1, "Processing TC from %s, seq 0x%04x\n", olsr_ip_to_string(&buf, &originator), tc->msg_seq);

  /*
   * Now walk the edge advertisements contained in the packet.
   */

  limit = (unsigned char *)msg + size;
  borderSet = 0;
  emptyTC = curr >= limit;
  while (curr < limit) {
    if (olsr_tc_update_edge(tc, ansn, &curr, &upper_border_ip)) {
      changes_topology = true;
    }

    if (!borderSet) {
      borderSet = 1;
      memcpy(&lower_border_ip, &upper_border_ip, sizeof(lower_border_ip));
    }
  }

  /*
   * Calculate real border IPs.
   */
  if (borderSet) {
    borderSet = olsr_calculate_tc_border(lower_border, &lower_border_ip, upper_border, &upper_border_ip);
  }

  /*
   * Set or change the expiration timer accordingly.
   */
  olsr_set_timer(&tc->validity_timer, vtime, OLSR_TC_VTIME_JITTER, OLSR_TIMER_ONESHOT, &olsr_expire_tc_entry, tc,
                 tc_validity_timer_cookie);

  if (emptyTC && lower_border == 0xff && upper_border == 0xff) {
    /* handle empty TC with border flags 0xff */
    memset(&lower_border_ip, 0x00, sizeof(lower_border_ip));
    memset(&upper_border_ip, 0xff, sizeof(upper_border_ip));
    borderSet = 1;
  }

  if (borderSet) {

    /*
     * Delete all old tc edges within borders.
     */
    olsr_delete_revoked_tc_edges(tc, ansn, &lower_border_ip, &upper_border_ip);
  } else {

    /*
     * Kick the the edge garbage collection timer. In the meantime hopefully
     * all edges belonging to a multipart neighbor set will arrive.
     */
    olsr_set_timer(&tc->edge_gc_timer, OLSR_TC_EDGE_GC_TIME, OLSR_TC_EDGE_GC_JITTER, OLSR_TIMER_ONESHOT, &olsr_expire_tc_edge_gc,
                   tc, tc_edge_gc_timer_cookie);
  }

  if (emptyTC && borderSet) {
    /* cleanup MIDs and HNAs if all edges have been erased by
     * an empty TC, then alert the duplicate set and kill the
     * tc entry */
    olsr_cleanup_mid(&originator);
    olsr_cleanup_hna(&originator);
    olsr_cleanup_duplicates(&originator);

    olsr_delete_tc_entry(tc);
  }
  /* Forward the message */
  return true;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
