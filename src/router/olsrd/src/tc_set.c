/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * LSDB rewrite (c) 2007, Hannes Gredler (hannes@gredler.at)
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
#include "lq_route.h"
#include "lq_avl.h"
#include "lq_packet.h"
#include "net_olsr.h"

#include <assert.h>

/* Root of the link state database */
struct avl_tree tc_tree;
struct tc_entry *tc_myself; /* Shortcut to ourselves */

/* Sven-Ola 2007-Dec: These four constants include an assumption
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

static olsr_bool
olsr_seq_inrange_low(int beg, int end, olsr_u16_t seq)
{
  if (beg < 0) {
    if (seq >= (olsr_u16_t)beg || seq < end) {
      return OLSR_TRUE;
    }
  } else if (end >= 0x10000) {
    if (seq >= beg || seq < (olsr_u16_t)end) {
      return OLSR_TRUE;
    }
  } else if (seq >= beg && seq < end) {
    return OLSR_TRUE;
  }
  return OLSR_FALSE;
}

static olsr_bool
olsr_seq_inrange_high(int beg, int end, olsr_u16_t seq)
{
  if (beg < 0) {
    if (seq > (olsr_u16_t)beg || seq <= end) {
      return OLSR_TRUE;
    }
  } else if (end >= 0x10000) {
    if (seq > beg || seq <= (olsr_u16_t)end) {
      return OLSR_TRUE;
    }
  } else if (seq > beg && seq <= end) {
    return OLSR_TRUE;
  }
  return OLSR_FALSE;
}

/**
 * Add a new tc_entry to the tc tree
 *
 * @param (last)adr address of the entry
 * @return a pointer to the created entry
 */
static struct tc_entry *
olsr_add_tc_entry(union olsr_ip_addr *adr)
{
#if !defined(NODEBUG) && defined(DEBUG)
  struct ipaddr_str buf;
#endif
  struct tc_entry *tc;

#ifdef DEBUG
  OLSR_PRINTF(1, "TC: add entry %s\n", olsr_ip_to_string(&buf, adr));
#endif

  tc = olsr_malloc(sizeof(struct tc_entry), "add TC entry");
  if (!tc) {
    return NULL;
  }
  memset(tc, 0, sizeof(struct tc_entry));

  /* Fill entry */
  tc->addr = *adr;
  tc->vertex_node.data = tc;
  tc->vertex_node.key = &tc->addr;

  /*
   * Insert into the global tc tree.
   */
  avl_insert(&tc_tree, &tc->vertex_node, AVL_DUP_NO);

  /*
   * Initialize subtrees for edges and prefixes.
   */
  avl_init(&tc->edge_tree, avl_comp_default);
  avl_init(&tc->prefix_tree, avl_comp_prefix_default);

  /*
   * Add a rt_path for ourselves.
   */
  olsr_insert_routing_table(adr, olsr_cnf->maxplen, adr,
                            OLSR_RT_ORIGIN_INT);

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

  olsr_register_timeout_function(&olsr_time_out_tc_set, OLSR_TRUE);

  avl_init(&tc_tree, avl_comp_default);

  /*
   * Add a TC entry for ourselves.
   */
  tc_myself = olsr_add_tc_entry(&olsr_cnf->main_addr);
}

/**
 * The main ip address has changed.
 * Do the needful.
 */
void
olsr_change_myself_tc(void)
{
  struct tc_edge_entry *tc_edge;

  if (tc_myself) {
    /*
     * Check if there was a change.
     */
    if (ipequal(&tc_myself->addr, &olsr_cnf->main_addr)) {
      return;
    }

    /*
     * Flush all edges and our own tc_entry.
     */
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc_myself, tc_edge) {
      olsr_delete_tc_edge_entry(tc_edge);
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc_myself, tc_edge);
    olsr_delete_tc_entry(tc_myself);
  }

  /*
   * The old entry for ourselves is gone, generate a new one and trigger SPF.
   */
  tc_myself = olsr_add_tc_entry(&olsr_cnf->main_addr);
  changes_topology = OLSR_TRUE;
}

/**
 * Delete a TC entry.
 *
 * @param entry the TC entry to delete
 *
 */
void
olsr_delete_tc_entry(struct tc_entry *tc)
{
#if 0
  struct ipaddr_str buf;
  OLSR_PRINTF(1, "TC: del entry %s\n", olsr_ip_to_string(&buf, &tc->addr));
#endif

  /*
   * Delete the rt_path for ourselves.
   */
  olsr_delete_routing_table(&tc->addr, olsr_cnf->maxplen, &tc->addr);

  /* The edgetree and prefix tree must be empty before */
  assert(!tc->edge_tree.count && !tc->prefix_tree.count);

  avl_delete(&tc_tree, &tc->vertex_node);
  free(tc);
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

#if 0
  OLSR_PRINTF(1, "TC: lookup entry\n");
#endif

  node = avl_find(&tc_tree, adr);

  if (node) {
    return node->data;
  }

  return NULL;
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
 * format a tc_edge contents into a buffer
 */
char *
olsr_tc_edge_to_string(struct tc_edge_entry *tc_edge)
{
  static char buf[128];
  struct ipaddr_str addrbuf, dstbuf;
  struct tc_entry *tc = tc_edge->tc;

  snprintf(buf, sizeof(buf),
           "%s > %s, lq %.3f, inv-lq %.3f, etx %.3f",
           olsr_ip_to_string(&addrbuf, &tc->addr),
           olsr_ip_to_string(&dstbuf, &tc_edge->T_dest_addr),
           tc_edge->link_quality,
           tc_edge->inverse_link_quality,
           tc_edge->etx);

  return buf;
}

/**
 * Set the TC edge expiration timer.
 *
 * all timer setting shall be done using this function since
 * it does also the correct insertion and sorting in the timer tree.
 * The timer param is a relative timer expressed in milliseconds.
 */
void
olsr_set_tc_edge_timer(struct tc_edge_entry *tc_edge, unsigned int timer)
{
  tc_edge->T_time = GET_TIMESTAMP(timer);
}

/*
 * If the edge does not have a minimum acceptable link quality
 * set the etx cost to infinity such that it gets ignored during
 * SPF calculation.
 */
void
olsr_calc_tc_edge_entry_etx(struct tc_edge_entry *tc_edge)
{

  /*
   * Some sanity check before recalculating the etx.
   */
  if (olsr_cnf->lq_level < 1) {
    return;
  }

  if (tc_edge->link_quality < MIN_LINK_QUALITY &&
      tc_edge->inverse_link_quality < MIN_LINK_QUALITY) {
    tc_edge->etx = INFINITE_ETX;
  } else {
    tc_edge->etx = 1.0 / (tc_edge->link_quality * tc_edge->inverse_link_quality);
  }
}

/**
 * Add a new tc_edge_entry to the tc_edge_tree
 *
 * @param (last)adr address of the entry
 * @return a pointer to the created entry
 */
struct tc_edge_entry *
olsr_add_tc_edge_entry(struct tc_entry *tc, union olsr_ip_addr *addr,
                       olsr_u16_t ansn, unsigned int vtime,
                       float link_quality, float neigh_link_quality)
{
#if !defined(NODEBUG) && defined(DEBUG)
  struct ipaddr_str buf;
#endif
  struct tc_entry *tc_neighbor;
  struct tc_edge_entry *tc_edge, *tc_edge_inv;

  tc_edge = olsr_malloc(sizeof(struct tc_edge_entry), "add TC edge");
  if (!tc_edge) {
    return NULL;
  }
  memset(tc_edge, 0, sizeof(struct tc_edge_entry));

  /* Fill entry */
  tc_edge->T_dest_addr = *addr;
  olsr_set_tc_edge_timer(tc_edge, vtime*1000);
  tc_edge->T_seq = ansn;
  tc_edge->edge_node.data = tc_edge;
  tc_edge->edge_node.key = &tc_edge->T_dest_addr;

  if (olsr_cnf->lq_level > 0) {
    tc_edge->link_quality = neigh_link_quality;
    tc_edge->inverse_link_quality = link_quality;
  } else {

    /*
     * Set the link quality to 1.0 to mimikry a hopcount alike
     * behaviour for nodes not supporting the LQ extensions.
     */
    tc_edge->link_quality = 1.0;
    tc_edge->inverse_link_quality = 1.0;
  }

  /*
   * Insert into the edge tree.
   */
  avl_insert(&tc->edge_tree, &tc_edge->edge_node, AVL_DUP_NO);

  /*
   * Connect backpointer.
   */
  tc_edge->tc = tc;

#ifdef DEBUG
  OLSR_PRINTF(1, "TC: add edge entry %s\n", olsr_tc_edge_to_string(tc_edge));
#endif

  /*
   * Check if the neighboring router and the inverse edge is in the lsdb.
   * Create short cuts to the inverse edge for faster SPF execution.
   */
  tc_neighbor = olsr_lookup_tc_entry(&tc_edge->T_dest_addr);
  if (tc_neighbor) {
#ifdef DEBUG
    OLSR_PRINTF(1, "TC:   found neighbor tc_entry %s\n",
                olsr_ip_to_string(&buf, &tc_neighbor->addr));
#endif

    tc_edge_inv = olsr_lookup_tc_edge(tc_neighbor, &tc->addr);
    if (tc_edge_inv) {
#ifdef DEBUG
      OLSR_PRINTF(1, "TC:   found inverse edge for %s\n",
                  olsr_ip_to_string(&buf, &tc_edge_inv->T_dest_addr));
#endif

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

  return tc_edge;
}


/**
 * Delete a TC edge entry.
 *
 * @param tc the TC entry
 * @param tc_edge the TC edge entry
 */
void
olsr_delete_tc_edge_entry(struct tc_edge_entry *tc_edge)
{
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge_inv;

#ifdef DEBUG
  OLSR_PRINTF(1, "TC: del edge entry %s\n", olsr_tc_edge_to_string(tc_edge));
#endif

  tc = tc_edge->tc;
  avl_delete(&tc->edge_tree, &tc_edge->edge_node);

  /*
   * Clear the backpointer of our inverse edge.
   */
  tc_edge_inv = tc_edge->edge_inv;
  if (tc_edge_inv) {
    tc_edge_inv->edge_inv = NULL;
  }

  /*
   * Delete the tc_entry if the last edge and last prefix is gone.
   */
  if (!tc_edge->tc->edge_tree.count &&
      !tc_edge->tc->prefix_tree.count) {

    /*
     * Only remove remote tc entries.
     */
    if (tc_edge->tc != tc_myself) {
      olsr_delete_tc_entry(tc_edge->tc);
    }
  }

  free(tc_edge);
}


/**
 * Delete all destinations that have a lower ANSN.
 *
 * @param tc the entry to delete edges from
 * @param ansn the advertised neighbor set sequence number
 * @return 1 if any destinations were deleted 0 if not
 */
static int
olsr_delete_outdated_tc_edges(struct tc_entry *tc, olsr_u16_t ansn)
{
  struct tc_edge_entry *tc_edge;
  int retval = 0;

#if 0
  OLSR_PRINTF(5, "TC: deleting MPRS\n");
#endif

  OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
    if (SEQNO_GREATER_THAN(ansn, tc_edge->T_seq)) {
      /*
       * Do not delete the edge now, just mark the edge as down.
       * Downed edges will be ignored by the SPF computation.
       * It could be that a TC message spans across multiple packets,
       * which causes an edge delete followed by an edge add.
       * If the edge gets refreshed in subsequent packets then we have
       * avoided a two edge transistion.
       * If the edge really went away then after the garbage collection
       * timer has expired olsr_time_out_tc_set() will do the needful.
       */
      tc_edge->flags |= OLSR_TC_EDGE_DOWN;
      olsr_set_tc_edge_timer(tc_edge, OLSR_TC_EDGE_GC_TIME);
      retval = 1;
    }
  } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);

  return retval;
}

/*
 * Determine if a etx change was more than 10%
 * Need to know this for triggering a SPF calculation.
 */
static olsr_bool
olsr_etx_significant_change(float etx1, float etx2)
{
  float rel_lq;

  if (etx1 == 0.0 || etx2 == 0.0) {
    return OLSR_TRUE;
  }

  rel_lq = etx1 / etx2;

  if (rel_lq > 1.1 || rel_lq < 0.9) {
    return OLSR_TRUE;
  }

  return OLSR_FALSE;
}

/**
 * Update an edge registered on an entry.
 * Creates new edge-entries if not registered.
 * Bases update on a received TC message
 *
 * @param entry the TC entry to check
 * @pkt the TC edge entry in the packet
 * @return 1 if entries are added 0 if not
 */
static int
olsr_tc_update_edge(struct tc_entry *tc, unsigned int vtime_s, olsr_u16_t ansn,
                    olsr_u8_t type, const unsigned char **curr)
{
  struct tc_edge_entry *tc_edge;
  double link_quality, neigh_link_quality;
  union olsr_ip_addr neighbor;
  int edge_change;

  edge_change = 0;

  /*
   * Fetch the per-edge data
   * LQ messages also contain LQ data.
   */
  pkt_get_ipaddress(curr, &neighbor);

  if (type == LQ_TC_MESSAGE) {
    pkt_get_lq(curr, &link_quality);
    pkt_get_lq(curr, &neigh_link_quality);
    pkt_ignore_u16(curr);
  } else {
    link_quality = 1.0;
    neigh_link_quality = 1.0;
  }

  /* First check if we know this edge */
  tc_edge = olsr_lookup_tc_edge(tc, &neighbor);

  if(!tc_edge) {
      
    /*
     * Yet unknown - create it.
     * Check if the address is allowed.
     */
    if (!olsr_validate_address(&neighbor)) {
      return 0;
    }

    olsr_add_tc_edge_entry(tc, &neighbor, ansn, vtime_s,
                           link_quality, neigh_link_quality);
    edge_change = 1;

  } else {

    /*
     * We know this edge - Update entry.
     */
    olsr_set_tc_edge_timer(tc_edge, vtime_s*1000);
    tc_edge->T_seq = ansn;

    /*
     * Clear the (possibly set) down flag.
     */
    tc_edge->flags &= ~OLSR_TC_EDGE_DOWN;

    /*
     * Determine if the etx change is meaningful enough
     * in order to trigger a SPF calculation.
     */
    if (olsr_etx_significant_change(tc_edge->link_quality,
                                    neigh_link_quality)) {

      if (tc->msg_hops <= olsr_cnf->lq_dlimit)
        edge_change = 1;
    }

    /*
     * Update link quality if configured. For hop-count only mode link quality
     * remains at 1.0.
     */
    if (olsr_cnf->lq_level > 0) {
      tc_edge->link_quality = neigh_link_quality;
    }

    if (olsr_etx_significant_change(tc_edge->inverse_link_quality,
                                    link_quality)) {

      if (tc->msg_hops <= olsr_cnf->lq_dlimit)
        edge_change = 1;
    }

    /*
     * Update inverse link quality if configured. For hop-count only mode
     * inverse link quality remains at 1.0.
     */
    if (olsr_cnf->lq_level > 0) {
      tc_edge->inverse_link_quality = link_quality;
    }

    /*
     * Update the etx.
     */
    olsr_calc_tc_edge_entry_etx(tc_edge);

#if DEBUG
    if (edge_change) {          
      OLSR_PRINTF(1, "TC:   chg edge entry %s\n",
                  olsr_tc_edge_to_string(tc_edge));
    }
#endif

  }

  return edge_change;
}

/**
 * Lookup an edge hanging off a TC entry.
 *
 * @param entry the entry to check
 * @param dst_addr the destination address to check for
 * @return a pointer to the tc_edge found - or NULL
 */
struct tc_edge_entry *
olsr_lookup_tc_edge(struct tc_entry *tc, union olsr_ip_addr *edge_addr)
{
  struct avl_node *edge_node;
  
#if 0
  OLSR_PRINTF(1, "TC: lookup dst\n");
#endif

  edge_node = avl_find(&tc->edge_tree, edge_addr);

  if (edge_node) {
    return edge_node->data;
  }

  return NULL;
}

/**
 * Walk the timers and time out entries.
 *
 * @return nada
 */
void
olsr_time_out_tc_set(void)
{
  struct tc_entry *tc;

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      /*
       * Delete outdated edges.
       */
      if(TIMED_OUT(tc_edge->T_time)) {
        olsr_delete_tc_edge_entry(tc_edge);
        changes_topology = OLSR_TRUE;
      }
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc)
}

/**
 * Print the topology table to stdout
 */
void
olsr_print_tc_table(void)
{
#ifndef NODEBUG
  /* The whole function makes no sense without it. */
  struct tc_entry *tc;
  const int ipwidth = olsr_cnf->ip_version == AF_INET ? 15 : 30;

  OLSR_PRINTF(1,
              "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- TOPOLOGY\n\n"
              "%-*s %-*s %-5s  %-5s  %s\n",
              nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, (int)now.tv_usec / 10000,
              ipwidth, "Source IP addr", ipwidth, "Dest IP addr", "LQ", "ILQ", "ETX");

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      struct ipaddr_str addrbuf, dstaddrbuf;
      OLSR_PRINTF(1, "%-*s %-*s  %5.3f  %5.3f  %.2f\n",
                  ipwidth, olsr_ip_to_string(&addrbuf, &tc->addr),
                  ipwidth, olsr_ip_to_string(&dstaddrbuf, &tc_edge->T_dest_addr),
                  tc_edge->link_quality,
                  tc_edge->inverse_link_quality,
                  olsr_calc_tc_etx(tc_edge));
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);
#endif
}

float olsr_calc_tc_etx(const struct tc_edge_entry *tc_edge)
{
  return tc_edge->link_quality < MIN_LINK_QUALITY ||
         tc_edge->inverse_link_quality < MIN_LINK_QUALITY
             ? 0.0
             : 1.0 / (tc_edge->link_quality * tc_edge->inverse_link_quality);
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
void
olsr_input_tc(union olsr_message *msg, struct interface *input_if,
              union olsr_ip_addr *from_addr)
{
#ifndef NODEBUG 
  struct ipaddr_str buf;
#endif
  olsr_u16_t size, msg_seq, ansn;
  olsr_u8_t type, ttl, msg_hops;
  double vtime;
  unsigned int vtime_s;
  union olsr_ip_addr originator;
  const unsigned char *limit, *curr;
  struct tc_entry *tc;

  curr = (void *)msg;
  if (!msg) {
    return;
  }

  /* We are only interested in TC message types. */
  pkt_get_u8(&curr, &type);
  if ((type != LQ_TC_MESSAGE) && (type != TC_MESSAGE)) {
    return;
  }

  pkt_get_double(&curr, &vtime);
  vtime_s = (unsigned int)vtime;
  pkt_get_u16(&curr, &size);

  pkt_get_ipaddress(&curr, &originator);

  /* Copy header values */
  pkt_get_u8(&curr, &ttl);
  pkt_get_u8(&curr, &msg_hops);
  pkt_get_u16(&curr, &msg_seq);
  pkt_get_u16(&curr, &ansn);
  pkt_ignore_u16(&curr);

  tc = olsr_lookup_tc_entry(&originator);
  
  if (tc && 0 != tc->edge_tree.count) {
    if (olsr_seq_inrange_high(
          (int)tc->msg_seq - TC_SEQNO_WINDOW,
          tc->msg_seq,
          msg_seq) &&
        olsr_seq_inrange_high(
          (int)tc->ansn - TC_ANSN_WINDOW,
          tc->ansn, ansn)) {

      /*
       * Ignore already seen seq/ansn values (small window for mesh memory)
       */
      if ((tc->msg_seq == msg_seq) || (tc->ignored++ < 32)) {
        return;
      }

      OLSR_PRINTF(1, "Ignored to much LQTC's for %s, restarting\n",
                  olsr_ip_to_string(&buf, &originator));

    } else if (!olsr_seq_inrange_high(
                 tc->msg_seq,
                 (int)tc->msg_seq + TC_SEQNO_WINDOW * TC_SEQNO_WINDOW_MULT,
                 msg_seq) ||
               !olsr_seq_inrange_low(
                 tc->ansn,
                 (int)tc->ansn + TC_ANSN_WINDOW * TC_ANSN_WINDOW_MULT,
                 ansn)) {

      /*
       * Only accept future seq/ansn values (large window for node reconnects).
       * Restart in all other cases. Ignore a single stray message.
       */
      if (!tc->err_seq_valid) {
        tc->err_seq = msg_seq;
        tc->err_seq_valid = OLSR_TRUE;
      }
      if (tc->err_seq == msg_seq) {
        return;
      }

      OLSR_PRINTF(2, "Detected node restart for %s\n",
                  olsr_ip_to_string(&buf, &originator));
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
  tc->err_seq_valid = OLSR_FALSE;
  
  /*
   * If the sender interface (NB: not originator) of this message
   * is not in the symmetric 1-hop neighborhood of this node, the
   * message MUST be discarded.
   */
  if (check_neighbor_link(from_addr) != SYM_LINK) {
    OLSR_PRINTF(2, "Received TC from NON SYM neighbor %s\n",
                olsr_ip_to_string(&buf, from_addr));
    return;
  }

  OLSR_PRINTF(1, "Processing TC from %s, seq 0x%04x\n",
              olsr_ip_to_string(&buf, &originator), ansn);

  /*
   * Now walk the edge advertisements contained in the packet.
   * Play some efficiency games here, like checking first
   * if the edge exists in order to avoid address validation.
   */
  limit = (unsigned char *)msg + size;
  while (curr < limit) {
    if (olsr_tc_update_edge(tc, vtime_s, ansn, type, &curr)) {
      changes_topology = OLSR_TRUE;
    }
  }

  /*
   * Do the edge garbage collection at the end in order
   * to avoid malloc() churn.
   */
  if (olsr_delete_outdated_tc_edges(tc, ansn)) {
    changes_topology = OLSR_TRUE;
  }

  /*
   * Last, flood the message to our other neighbors.
   */
  olsr_forward_message(msg, &originator, msg_seq, input_if, from_addr);
  return;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
