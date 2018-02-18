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

#include "ipcalc.h"
#include "defs.h"
#include "two_hop_neighbor_table.h"
#include "mid_set.h"
#include "mpr.h"
#include "neighbor_table.h"
#include "olsr.h"
#include "scheduler.h"
#include "link_set.h"
#include "mpr_selector_set.h"
#include "net_olsr.h"

struct neighbor_entry neighbortable[HASHSIZE];

void
olsr_init_neighbor_table(void)
{
  int i;

  for (i = 0; i < HASHSIZE; i++) {
    neighbortable[i].next = &neighbortable[i];
    neighbortable[i].prev = &neighbortable[i];
  }
}

/**
 * Unlink, delete and free a nbr2_list entry.
 */
static void
olsr_del_nbr2_list(struct neighbor_2_list_entry *nbr2_list)
{
  struct neighbor_2_entry *nbr2;

  nbr2 = nbr2_list->neighbor_2;

  if (nbr2->neighbor_2_pointer < 1) {
    DEQUEUE_ELEM(nbr2);
    free(nbr2);
  }

  /*
   * Kill running timers.
   */
  olsr_stop_timer(nbr2_list->nbr2_list_timer);
  nbr2_list->nbr2_list_timer = NULL;

  /* Dequeue */
  DEQUEUE_ELEM(nbr2_list);

  free(nbr2_list);

  /* Set flags to recalculate the MPR set and the routing table */
  changes_neighborhood = true;
  changes_topology = true;
}

/**
 * Delete a two hop neighbor from a neighbors two hop neighbor list.
 *
 * @param neighbor the neighbor to delete the two hop neighbor from.
 * @param neigh2 the IP address of the two hop neighbor to delete.
 *
 * @return positive if entry deleted
 */
int
olsr_delete_neighbor_2_pointer(struct neighbor_entry *neighbor, struct neighbor_2_entry *neigh2)
{
  struct neighbor_2_list_entry *nbr2_list;

  nbr2_list = neighbor->neighbor_2_list.next;

  while (nbr2_list != &neighbor->neighbor_2_list) {
    if (nbr2_list->neighbor_2 == neigh2) {
      olsr_del_nbr2_list(nbr2_list);
      return 1;
    }
    nbr2_list = nbr2_list->next;
  }
  return 0;
}

/**
 *Check if a two hop neighbor is reachable via a given
 *neighbor.
 *
 *@param neighbor neighbor-entry to check via
 *@param neighbor_main_address the addres of the two hop neighbor
 *to find.
 *
 *@return a pointer to the neighbor_2_list_entry struct
 *representing the two hop neighbor if found. NULL if not found.
 */
struct neighbor_2_list_entry *
olsr_lookup_my_neighbors(const struct neighbor_entry *neighbor, const union olsr_ip_addr *neighbor_main_address)
{
  struct neighbor_2_list_entry *entry;

  for (entry = neighbor->neighbor_2_list.next; entry != &neighbor->neighbor_2_list; entry = entry->next) {

    if (ipequal(&entry->neighbor_2->neighbor_2_addr, neighbor_main_address))
      return entry;

  }
  return NULL;
}

/**
 * Update a neighbours main_addr inlcuding hash
*/

void
olsr_update_neighbor_main_addr(struct neighbor_entry *entry, const union olsr_ip_addr *new_main_addr)
{
  /*remove from old pos*/
  DEQUEUE_ELEM(entry);

  /*update main addr*/
  entry->neighbor_main_addr = *new_main_addr;

  /*insert it again*/
  QUEUE_ELEM(neighbortable[olsr_ip_hashing(new_main_addr)], entry);

}

/**
 *Delete a neighbr table entry.
 *
 *Remember: Deleting a neighbor entry results
 *the deletion of its 2 hop neighbors list!!!
 *@param neighbor_addr the neighbor entry to delete
 *
 *@return always 1
 */

int
olsr_delete_neighbor_table(const union olsr_ip_addr *neighbor_addr)
{
  struct neighbor_2_list_entry *two_hop_list, *two_hop_to_delete;
  uint32_t hash;
  struct neighbor_entry *entry;

  //printf("inserting neighbor\n");

  hash = olsr_ip_hashing(neighbor_addr);

  entry = neighbortable[hash].next;

  /*
   * Find neighbor entry
   */
  while (entry != &neighbortable[hash]) {
    if (ipequal(&entry->neighbor_main_addr, neighbor_addr))
      break;

    entry = entry->next;
  }

  if (entry == &neighbortable[hash])
    return 0;

  two_hop_list = entry->neighbor_2_list.next;

  while (two_hop_list != &entry->neighbor_2_list) {
    two_hop_to_delete = two_hop_list;
    two_hop_list = two_hop_list->next;

    two_hop_to_delete->neighbor_2->neighbor_2_pointer--;
    olsr_delete_neighbor_pointer(two_hop_to_delete->neighbor_2, entry);

    olsr_del_nbr2_list(two_hop_to_delete);
  }

  /* Dequeue */
  DEQUEUE_ELEM(entry);

  free(entry);

  changes_neighborhood = true;
  signal_link_changes(true);
  return 1;

}

/**
 *Insert a neighbor entry in the neighbor table
 *
 *@param main_addr the main address of the new node
 *
 *@return 0 if neighbor already exists 1 if inserted
 */
struct neighbor_entry *
olsr_insert_neighbor_table(const union olsr_ip_addr *main_addr)
{
  uint32_t hash;
  struct neighbor_entry *new_neigh;

  hash = olsr_ip_hashing(main_addr);

  /* Check if entry exists */

  for (new_neigh = neighbortable[hash].next; new_neigh != &neighbortable[hash]; new_neigh = new_neigh->next) {
    if (ipequal(&new_neigh->neighbor_main_addr, main_addr))
      return new_neigh;
  }

  //printf("inserting neighbor\n");

  new_neigh = olsr_malloc(sizeof(struct neighbor_entry), "New neighbor entry");

  /* Set address, willingness and status */
  new_neigh->neighbor_main_addr = *main_addr;
  new_neigh->willingness = WILL_NEVER;
  new_neigh->status = NOT_SYM;

  new_neigh->neighbor_2_list.next = &new_neigh->neighbor_2_list;
  new_neigh->neighbor_2_list.prev = &new_neigh->neighbor_2_list;

  new_neigh->linkcount = 0;
  new_neigh->is_mpr = false;
  new_neigh->was_mpr = false;

  /* Queue */
  QUEUE_ELEM(neighbortable[hash], new_neigh);

  return new_neigh;
}

/**
 *Lookup a neighbor entry in the neighbortable based on an address.
 *
 *@param dst the IP address of the neighbor to look up
 *
 *@return a pointer to the neighbor struct registered on the given
 *address. NULL if not found.
 */
struct neighbor_entry *
olsr_lookup_neighbor_table(const union olsr_ip_addr *dst)
{
  /*
   *Find main address of node
   */
  union olsr_ip_addr *tmp_ip = mid_lookup_main_addr(dst);
  if (tmp_ip != NULL)
    dst = tmp_ip;
  return olsr_lookup_neighbor_table_alias(dst);
}

/**
 *Lookup a neighbor entry in the neighbortable based on an address.
 *
 *@param dst the IP address of the neighbor to look up
 *
 *@return a pointer to the neighbor struct registered on the given
 *address. NULL if not found.
 */
struct neighbor_entry *
olsr_lookup_neighbor_table_alias(const union olsr_ip_addr *dst)
{
  struct neighbor_entry *entry;
  uint32_t hash = olsr_ip_hashing(dst);

  //printf("\nLookup %s\n", olsr_ip_to_string(&buf, dst));
  for (entry = neighbortable[hash].next; entry != &neighbortable[hash]; entry = entry->next) {
    //printf("Checking %s\n", olsr_ip_to_string(&buf, &entry->neighbor_main_addr));
    if (ipequal(&entry->neighbor_main_addr, dst))
      return entry;

  }
  //printf("NOPE\n\n");

  return NULL;

}

int
update_neighbor_status(struct neighbor_entry *entry, int lnk)
{
  /*
   * Update neighbor entry
   */

  if (lnk == SYM_LINK) {
    /* N_status is set to SYM */
    if (entry->status == NOT_SYM) {
      struct neighbor_2_entry *two_hop_neighbor;

      /* Delete posible 2 hop entry on this neighbor */
      if ((two_hop_neighbor = olsr_lookup_two_hop_neighbor_table(&entry->neighbor_main_addr)) != NULL) {
        olsr_delete_two_hop_neighbor_table(two_hop_neighbor);
      }

      changes_neighborhood = true;
      changes_topology = true;
      if (olsr_cnf->tc_redundancy > 1)
        signal_link_changes(true);
    }
    entry->status = SYM;
  } else {
    if (entry->status == SYM) {
      changes_neighborhood = true;
      changes_topology = true;
      if (olsr_cnf->tc_redundancy > 1)
        signal_link_changes(true);
    }
    /* else N_status is set to NOT_SYM */
    entry->status = NOT_SYM;
    /* remove neighbor from routing list */
  }

  return entry->status;
}

/**
 * Callback for the nbr2_list timer.
 */
void
olsr_expire_nbr2_list(void *context)
{
  struct neighbor_2_list_entry *nbr2_list;
  struct neighbor_entry *nbr;
  struct neighbor_2_entry *nbr2;

  nbr2_list = (struct neighbor_2_list_entry *)context;
  nbr2_list->nbr2_list_timer = NULL;

  nbr = nbr2_list->nbr2_nbr;
  nbr2 = nbr2_list->neighbor_2;

  nbr2->neighbor_2_pointer--;
  olsr_delete_neighbor_pointer(nbr2, nbr);

  olsr_del_nbr2_list(nbr2_list);
}

/**
 *Prints the registered neighbors and two hop neighbors
 *to STDOUT.
 *
 *@return nada
 */
#ifndef NODEBUG
void
olsr_print_neighbor_table(void)
{
  /* The whole function doesn't do anything else. */
  const int iplen = olsr_cnf->ip_version == AF_INET ? (INET_ADDRSTRLEN - 1) : (INET6_ADDRSTRLEN - 1);
  int idx;

  OLSR_PRINTF(1,
              "\n--- %s ------------------------------------------------ NEIGHBORS\n\n"
              "%*s\tHyst\tLQ\tETX\tSYM   MPR   MPRS  will\n", olsr_wallclock_string(),
              iplen, "IP address");

  for (idx = 0; idx < HASHSIZE; idx++) {
    struct neighbor_entry *neigh;
    for (neigh = neighbortable[idx].next; neigh != &neighbortable[idx]; neigh = neigh->next) {
      struct link_entry *lnk = get_best_link_to_neighbor(&neigh->neighbor_main_addr);
      if (lnk) {
        struct ipaddr_str buf;
        struct lqtextbuffer lqbuffer1, lqbuffer2;

        OLSR_PRINTF(1, "%-*s\t%5.3f\t%s\t%s\t%s  %s  %s  %d\n", iplen, olsr_ip_to_string(&buf, &neigh->neighbor_main_addr),
                    (double)lnk->L_link_quality,
                    get_link_entry_text(lnk, '/', &lqbuffer1),
                    get_linkcost_text(lnk->linkcost,false, &lqbuffer2),
                    neigh->status == SYM ? "YES " : "NO  ",
                    neigh->is_mpr ? "YES " : "NO  ",
                    olsr_lookup_mprs_set(&neigh->neighbor_main_addr) == NULL ? "NO  " : "YES ",
                    neigh->willingness);
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
