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
 * $Id: tc_set.c,v 1.32 2007/10/17 07:30:34 bernd67 Exp $
 */

#include "tc_set.h"
#include "olsr.h"
#include "scheduler.h"
#include "lq_route.h"
#include "lq_avl.h"
#include "assert.h"

/* Root of the link state database */
struct avl_tree tc_tree;
struct tc_entry *tc_myself; /* Shortcut to ourselves */

/**
 * Initialize the topology set
 *
 */
int
olsr_init_tc(void)
{
  OLSR_PRINTF(5, "TC: init topo\n");

  olsr_register_timeout_function(&olsr_time_out_tc_set, OLSR_TRUE);

  avl_init(&tc_tree, avl_comp_default);

  /*
   * Add a TC entry for ourselves.
   */
  tc_myself = olsr_add_tc_entry(&olsr_cnf->main_addr);
  return 1;
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
    if (COMP_IP(&tc_myself->addr, &olsr_cnf->main_addr)) {
      return;
    }

    /*
     * Flush all edges. This causes our own tc_entry to vanish.
     */
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc_myself, tc_edge) {
      olsr_delete_tc_edge_entry(tc_edge);
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc_myself, tc_edge);
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
static void
olsr_delete_tc_entry(struct tc_entry *tc)
{

#if 0
  OLSR_PRINTF(1, "TC: del entry %s\n", olsr_ip_to_string(&tc->addr));
#endif

  /* The edgetree must be empty before */
  assert(!tc->edge_tree.count);

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

/**
 * Grab the next topology entry.
 *
 * @param adr the address to look for
 * @return the entry found or NULL
 */
struct tc_entry *
olsr_getnext_tc_entry(struct tc_entry *tc)
{
  struct avl_node *node;

  node = avl_walk_next(&tc->vertex_node);

  if (node) {
    return node->data;
  }

  return NULL;
}

/**
 * Add a new tc_entry to the tc tree
 *
 * @param (last)adr address of the entry
 * @return a pointer to the created entry
 */
struct tc_entry *
olsr_add_tc_entry(union olsr_ip_addr *adr)
{
  struct tc_entry *tc;

#if 0
  OLSR_PRINTF(1, "TC: add entry %s\n", olsr_ip_to_string(adr));
#endif

  tc = olsr_malloc(sizeof(struct tc_entry), "add TC entry");
  if (!tc) {
    return NULL;
  }
  memset(tc, 0, sizeof(struct tc_entry));

  /* Fill entry */
  COPY_IP(&tc->addr, adr);
  tc->vertex_node.data = tc;
  tc->vertex_node.key = &tc->addr;

  /*
   * Insert into the global tc tree.
   */
  avl_insert(&tc_tree, &tc->vertex_node, AVL_DUP_NO);

  /*
   * Initialize subtree for further edges to come.
   */
  avl_init(&tc->edge_tree, avl_comp_default);

  return tc;
}

/**
 * format a tc_edge contents into a buffer
 */
char *
olsr_tc_edge_to_string(struct tc_edge_entry *tc_edge)
{
  struct tc_entry *tc;
  static char buff[128];

  tc = tc_edge->tc;

  snprintf(buff, sizeof(buff),
           "%s > %s, lq %.3f, inv-lq %.3f, etx %.3f",
           olsr_ip_to_string(&tc->addr),
           olsr_ip_to_string(&tc_edge->T_dest_addr),
           tc_edge->link_quality,
           tc_edge->inverse_link_quality,
           tc_edge->etx);

  return buff;
}

/**
 * Set the TC edge expiration timer.
 *
 * all timer setting shall be done using this function since
 * it does also the correct insertion and sorting in the timer tree.
 * The timer param is a relative timer expressed in milliseconds.
 */
static void
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
  if (tc_edge->link_quality >= MIN_LINK_QUALITY &&
      tc_edge->inverse_link_quality >= MIN_LINK_QUALITY) {
        
    tc_edge->etx = 1.0 / (tc_edge->link_quality * tc_edge->inverse_link_quality);
  } else {
    tc_edge->etx = INFINITE_ETX;
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
  struct tc_entry *tc_neighbor;
  struct tc_edge_entry *tc_edge, *tc_edge_inv;

  tc_edge = olsr_malloc(sizeof(struct tc_edge_entry), "add TC edge");
  if (!tc_edge) {
    return NULL;
  }
  memset(tc_edge, 0, sizeof(struct tc_edge_entry));

  /* Fill entry */
  COPY_IP(&tc_edge->T_dest_addr, addr);
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

#if 0
  OLSR_PRINTF(1, "TC: add edge entry %s\n", olsr_tc_edge_to_string(tc_edge));
#endif

  /*
   * Check if the neighboring router and the inverse edge is in the lsdb.
   * Create short cuts to the inverse edge for faster SPF execution.
   */
  tc_neighbor = olsr_lookup_tc_entry(&tc_edge->T_dest_addr);
  if (tc_neighbor) {

#if 0
    OLSR_PRINTF(1, "TC:   found neighbor tc_entry %s\n",
                olsr_ip_to_string(&tc_neighbor->addr));
#endif

    tc_edge_inv = olsr_lookup_tc_edge(tc_neighbor, &tc->addr);
    if (tc_edge_inv) {

#if 0
      OLSR_PRINTF(1, "TC:   found inverse edge for %s\n",
                  olsr_ip_to_string(&tc_edge_inv->T_dest_addr));
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

#if 0
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
   * Delete the tc_entry if the last edge is gone.
   */
  if (!tc_edge->tc->edge_tree.count) {

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
 * Delete all destinations that have a
 * lower ANSN than the one in the message
 *
 * @param tc the entry to delete edges from
 * @param msg the message to fetch the ANSN from
 * @return 1 if any destinations were deleted 0 if not
 */
int
olsr_tc_delete_mprs(struct tc_entry *tc, struct tc_message *msg)
{
  struct tc_edge_entry *tc_edge;
  int retval;

#if 0
  OLSR_PRINTF(5, "TC: deleting MPRS\n");
#endif

  retval = 0;

  OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {

    if (SEQNO_GREATER_THAN(msg->ansn, tc_edge->T_seq)) {

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
 * Update the destinations registered on an entry.
 * Creates new dest-entries if not registered.
 * Bases update on a receivied TC message
 *
 * @param entry the TC entry to check
 * @msg the TC message to update by
 * @return 1 if entries are added 0 if not
 */
int
olsr_tc_update_mprs(struct tc_entry *tc, struct tc_message *msg)
{
  struct tc_mpr_addr *mprs;
  struct tc_edge_entry *tc_edge;
  int edge_change;

#if 0
  OLSR_PRINTF(1, "TC: update MPRS\n");
#endif

  edge_change = 0;

  mprs = msg->multipoint_relay_selector_address;
  
  /* Add all the MPRs */

  while (mprs) {

    /* First check if we know this edge */
    tc_edge = olsr_lookup_tc_edge(tc, &mprs->address);

    if(!tc_edge) {
      
      /*
       * Yet unknown - create it.
       */
      olsr_add_tc_edge_entry(tc, &mprs->address, msg->ansn,
                             (unsigned int )msg->vtime,
                             mprs->link_quality, mprs->neigh_link_quality);
      edge_change = 1;

    } else {

      /*
       * We know this edge - Update entry.
       */
      olsr_set_tc_edge_timer(tc_edge, msg->vtime*1000);
      tc_edge->T_seq = msg->ansn;

      /*
       * Clear the (possibly set) down flag.
       */
      tc_edge->flags &= ~OLSR_TC_EDGE_DOWN;

      /*
       * Determine if the etx change is meaningful enough
       * in order to trigger a SPF calculation.
       */
      if (olsr_etx_significant_change(tc_edge->link_quality,
                                      mprs->neigh_link_quality)) {

        if (msg->hop_count <= olsr_cnf->lq_dlimit)
          edge_change = 1;
      }
      tc_edge->link_quality = mprs->neigh_link_quality;

      if (olsr_etx_significant_change(tc_edge->inverse_link_quality,
                                      mprs->link_quality)) {

        if (msg->hop_count <= olsr_cnf->lq_dlimit)
          edge_change = 1;
      }
      tc_edge->inverse_link_quality = mprs->link_quality;

      /*
       * Update the etx.
       */
      olsr_calc_tc_edge_entry_etx(tc_edge);

#if 0
      if (edge_change) {          
        OLSR_PRINTF(1, "TC:   chg edge entry %s\n",
                    olsr_tc_edge_to_string(tc_edge));
      }
#endif

    }
    mprs = mprs->next;
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
  struct tc_edge_entry *tc_edge;

  OLSR_FOR_ALL_TC_ENTRIES(tc)
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {

    /*
     * Delete outdated edges.
     */
    if(TIMED_OUT(tc_edge->T_time)) {
      olsr_delete_tc_edge_entry(tc_edge);
      changes_topology = OLSR_TRUE;
    }
  } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  OLSR_FOR_ALL_TC_ENTRIES_END(tc)
}

/**
 * Print the topology table to stdout
 */
int
olsr_print_tc_table(void)
{
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge;
  char *fstr;
  float etx;
  
  OLSR_PRINTF(1, "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- TOPOLOGY\n\n",
              nowtm->tm_hour,
              nowtm->tm_min,
              nowtm->tm_sec,
              (int)now.tv_usec / 10000);

  if (olsr_cnf->ip_version == AF_INET)
    {
      OLSR_PRINTF(1, "Source IP addr   Dest IP addr     LQ     ILQ    ETX\n");
      fstr = "%-15s  %-15s  %5.3f  %5.3f  %.2f\n";
    }
  else
    {
      OLSR_PRINTF(1, "Source IP addr                Dest IP addr                    LQ     ILQ    ETX\n");
      fstr = "%-30s%-30s  %5.3f  %5.3f  %.2f\n";
    }

  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {

      if (tc_edge->link_quality < MIN_LINK_QUALITY ||
          tc_edge->inverse_link_quality < MIN_LINK_QUALITY) {
        etx = 0.0;
      } else {
        etx = 1.0 / (tc_edge->link_quality * tc_edge->inverse_link_quality);
      }

      OLSR_PRINTF(1, fstr, olsr_ip_to_string(&tc->addr),
                  olsr_ip_to_string(&tc_edge->T_dest_addr),
                  tc_edge->link_quality, tc_edge->inverse_link_quality, etx);

    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
