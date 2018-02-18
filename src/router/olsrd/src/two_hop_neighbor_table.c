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

#include "two_hop_neighbor_table.h"
#include "ipcalc.h"
#include "defs.h"
#include "mid_set.h"
#include "neighbor_table.h"
#include "net_olsr.h"
#include "scheduler.h"

struct neighbor_2_entry two_hop_neighbortable[HASHSIZE];

/**
 *Initialize 2 hop neighbor table
 */
void
olsr_init_two_hop_table(void)
{
  int idx;
  for (idx = 0; idx < HASHSIZE; idx++) {
    two_hop_neighbortable[idx].next = &two_hop_neighbortable[idx];
    two_hop_neighbortable[idx].prev = &two_hop_neighbortable[idx];
  }
}

/**
 *Remove a one hop neighbor from a two hop neighbors
 *one hop list.
 *
 *@param two_hop_entry the two hop neighbor to remove the
 *one hop neighbor from
 *@param neigh address the address of the one hop neighbor to remove
 *
 *@return nada
 */

void
olsr_delete_neighbor_pointer(struct neighbor_2_entry *two_hop_entry, struct neighbor_entry *neigh)
{
  struct neighbor_list_entry *entry = two_hop_entry->neighbor_2_nblist.next;
  while (entry != &two_hop_entry->neighbor_2_nblist) {
    if (entry->neighbor == neigh) {
      struct neighbor_list_entry *entry_to_delete = entry;
      entry = entry->next;

      /* dequeue */
      DEQUEUE_ELEM(entry_to_delete);

      free(entry_to_delete);
    } else {
      entry = entry->next;
    }
  }
}

/**
 *Delete an entry from the two hop neighbor table.
 *
 *@param two_hop_neighbor the two hop neighbor to delete.
 *
 *@return nada
 */
void
olsr_delete_two_hop_neighbor_table(struct neighbor_2_entry *two_hop_neighbor)
{
  struct neighbor_list_entry *one_hop_list;

  one_hop_list = two_hop_neighbor->neighbor_2_nblist.next;

  /* Delete one hop links */
  while (one_hop_list != &two_hop_neighbor->neighbor_2_nblist) {
    struct neighbor_entry *one_hop_entry = one_hop_list->neighbor;
    struct neighbor_list_entry *entry_to_delete = one_hop_list;

    olsr_delete_neighbor_2_pointer(one_hop_entry, two_hop_neighbor);
    one_hop_list = one_hop_list->next;
    /* no need to dequeue */
    free(entry_to_delete);
  }

  /* dequeue */
  DEQUEUE_ELEM(two_hop_neighbor);
  free(two_hop_neighbor);
}

/**
 *Insert a new entry to the two hop neighbor table.
 *
 *@param two_hop_neighbor the entry to insert
 *
 *@return nada
 */
void
olsr_insert_two_hop_neighbor_table(struct neighbor_2_entry *two_hop_neighbor)
{
  uint32_t hash = olsr_ip_hashing(&two_hop_neighbor->neighbor_2_addr);

  /* Queue */
  QUEUE_ELEM(two_hop_neighbortable[hash], two_hop_neighbor);
}

/**
 *Look up an entry in the two hop neighbor table.
 *
 *@param dest the IP address of the entry to find
 *
 *@return a pointer to a neighbor_2_entry struct
 *representing the two hop neighbor
 */
struct neighbor_2_entry *
olsr_lookup_two_hop_neighbor_table(const union olsr_ip_addr *dest)
{

  struct neighbor_2_entry *neighbor_2;
  uint32_t hash = olsr_ip_hashing(dest);

  /* printf("LOOKING FOR %s\n", olsr_ip_to_string(&buf, dest)); */
  for (neighbor_2 = two_hop_neighbortable[hash].next; neighbor_2 != &two_hop_neighbortable[hash]; neighbor_2 = neighbor_2->next) {
    struct mid_address *adr;

    /* printf("Checking %s\n", olsr_ip_to_string(&buf, dest)); */
    if (ipequal(&neighbor_2->neighbor_2_addr, dest))
      return neighbor_2;

    adr = mid_lookup_aliases(&neighbor_2->neighbor_2_addr);

    while (adr) {
      if (ipequal(&adr->alias, dest))
        return neighbor_2;
      adr = adr->next_alias;
    }
  }

  return NULL;
}

/**
 *Look up an entry in the two hop neighbor table.
 *NO CHECK FOR MAIN ADDRESS OR ALIASES!
 *
 *@param dest the IP address of the entry to find
 *
 *@return a pointer to a neighbor_2_entry struct
 *representing the two hop neighbor
 */
struct neighbor_2_entry *
olsr_lookup_two_hop_neighbor_table_mid(const union olsr_ip_addr *dest)
{
  struct neighbor_2_entry *neighbor_2;
  uint32_t hash;

  /* printf("LOOKING FOR %s\n", olsr_ip_to_string(&buf, dest)); */
  hash = olsr_ip_hashing(dest);

  for (neighbor_2 = two_hop_neighbortable[hash].next; neighbor_2 != &two_hop_neighbortable[hash]; neighbor_2 = neighbor_2->next) {
    if (ipequal(&neighbor_2->neighbor_2_addr, dest))
      return neighbor_2;
  }

  return NULL;
}

/**
 *Print the two hop neighbor table to STDOUT.
 *
 *@return nada
 */
#ifndef NODEBUG
void
olsr_print_two_hop_neighbor_table(void)
{
  /* The whole function makes no sense without it. */
  int i;
  const int ipwidth = olsr_cnf->ip_version == AF_INET ? (INET_ADDRSTRLEN - 1) : (INET6_ADDRSTRLEN - 1);

  OLSR_PRINTF(1, "\n--- %s ----------------------- TWO-HOP NEIGHBORS\n\n" "IP addr (2-hop)  IP addr (1-hop)  Total cost\n",
              olsr_wallclock_string());

  for (i = 0; i < HASHSIZE; i++) {
    struct neighbor_2_entry *neigh2;
    for (neigh2 = two_hop_neighbortable[i].next; neigh2 != &two_hop_neighbortable[i]; neigh2 = neigh2->next) {
      struct neighbor_list_entry *entry;
      bool first = true;

      for (entry = neigh2->neighbor_2_nblist.next; entry != &neigh2->neighbor_2_nblist; entry = entry->next) {
        struct ipaddr_str buf;
        struct lqtextbuffer lqbuffer;
        if (first) {
          OLSR_PRINTF(1, "%-*s  ", ipwidth, olsr_ip_to_string(&buf, &neigh2->neighbor_2_addr));
          first = false;
        } else {
          OLSR_PRINTF(1, "                 ");
        }
        OLSR_PRINTF(1, "%-*s  %s\n", ipwidth, olsr_ip_to_string(&buf, &entry->neighbor->neighbor_main_addr),
                    get_linkcost_text(entry->path_linkcost, false, &lqbuffer));
      }
    }
  }
}
#endif /* NODEBUG */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
