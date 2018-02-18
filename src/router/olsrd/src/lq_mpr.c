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

#include "defs.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "link_set.h"
#include "lq_mpr.h"
#include "scheduler.h"
#include "lq_plugin.h"

void
olsr_calculate_lq_mpr(void)
{
  struct neighbor_2_entry *neigh2;
  struct neighbor_list_entry *walker;
  int i, k;
  struct neighbor_entry *neigh;
  olsr_linkcost best, best_1hop;
  bool mpr_changes = false;

  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {

    /* Memorize previous MPR status. */

    neigh->was_mpr = neigh->is_mpr;

    /* Clear current MPR status. */

    neigh->is_mpr = false;

    /* In this pass we are only interested in WILL_ALWAYS neighbours */

    if (neigh->status == NOT_SYM || neigh->willingness != WILL_ALWAYS) {
      continue;
    }

    neigh->is_mpr = true;

    if (neigh->is_mpr != neigh->was_mpr) {
      mpr_changes = true;
    }

  }
  OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);

  for (i = 0; i < HASHSIZE; i++) {
    /* loop through all 2-hop neighbours */

    for (neigh2 = two_hop_neighbortable[i].next; neigh2 != &two_hop_neighbortable[i]; neigh2 = neigh2->next) {
      best_1hop = LINK_COST_BROKEN;

      /* check whether this 2-hop neighbour is also a neighbour */

      neigh = olsr_lookup_neighbor_table(&neigh2->neighbor_2_addr);

      /* if it's a neighbour and also symmetric, then examine
         the link quality */

      if (neigh != NULL && neigh->status == SYM) {
        /* if the direct link is better than the best route via
         * an MPR, then prefer the direct link and do not select
         * an MPR for this 2-hop neighbour */

        /* determine the link quality of the direct link */

        struct link_entry *lnk = get_best_link_to_neighbor(&neigh->neighbor_main_addr);

        if (!lnk)
          continue;

        best_1hop = lnk->linkcost;

        /* see wether we find a better route via an MPR */

        for (walker = neigh2->neighbor_2_nblist.next; walker != &neigh2->neighbor_2_nblist; walker = walker->next)
          if (walker->path_linkcost < best_1hop)
            break;

        /* we've reached the end of the list, so we haven't found
         * a better route via an MPR - so, skip MPR selection for
         * this 1-hop neighbor */

        if (walker == &neigh2->neighbor_2_nblist)
          continue;
      }

      /* find the connecting 1-hop neighbours with the
       * best total link qualities */

      /* mark all 1-hop neighbours as not selected */

      for (walker = neigh2->neighbor_2_nblist.next; walker != &neigh2->neighbor_2_nblist; walker = walker->next)
        walker->neighbor->skip = false;

      for (k = 0; k < olsr_cnf->mpr_coverage; k++) {
        /* look for the best 1-hop neighbour that we haven't
         * yet selected */

        neigh = NULL;
        best = LINK_COST_BROKEN;

        for (walker = neigh2->neighbor_2_nblist.next; walker != &neigh2->neighbor_2_nblist; walker = walker->next)
          if (walker->neighbor->status == SYM && !walker->neighbor->skip && walker->path_linkcost < best) {
            neigh = walker->neighbor;
            best = walker->path_linkcost;
          }

        /* Found a 1-hop neighbor that we haven't previously selected.
         * Use it as MPR only when the 2-hop path through it is better than
         * any existing 1-hop path. */
        if ((neigh != NULL) && (best < best_1hop)) {
          neigh->is_mpr = true;
          neigh->skip = true;

          if (neigh->is_mpr != neigh->was_mpr)
            mpr_changes = true;
        }

        /* no neighbour found => the requested MPR coverage cannot
         * be satisfied => stop */

        else
          break;
      }
    }
  }

  if (mpr_changes && olsr_cnf->tc_redundancy > 0)
    signal_link_changes(true);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
