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
#include <assert.h>

#include "ipcalc.h"
#include "defs.h"
#include "two_hop_neighbor_table.h"
#include "mid_set.h"
#include "olsr.h"
#include "rebuild_packet.h"
#include "scheduler.h"
#include "neighbor_table.h"
#include "link_set.h"
#include "tc_set.h"
#include "packet.h"             /* struct mid_alias */
#include "net_olsr.h"
#include "duplicate_handler.h"

struct mid_entry mid_set[HASHSIZE];
struct mid_address reverse_mid_set[HASHSIZE];

struct mid_entry *mid_lookup_entry_bymain(const union olsr_ip_addr *adr);

/**
 * Initialize the MID set
 *
 */
int
olsr_init_mid_set(void)
{
  int idx;

  OLSR_PRINTF(5, "MID: init\n");

  for (idx = 0; idx < HASHSIZE; idx++) {
    mid_set[idx].next = &mid_set[idx];
    mid_set[idx].prev = &mid_set[idx];

    reverse_mid_set[idx].next = &reverse_mid_set[idx];
    reverse_mid_set[idx].prev = &reverse_mid_set[idx];
  }

  return 1;
}

void olsr_delete_all_mid_entries(void) {
  int hash;

  for (hash = 0; hash < HASHSIZE; hash++) {
    while (mid_set[hash].next != &mid_set[hash]) {
      olsr_delete_mid_entry(mid_set[hash].next);
    }
  }
}

void olsr_cleanup_mid(union olsr_ip_addr *orig) {
  struct mid_entry *mid;
  mid = mid_lookup_entry_bymain(orig);
  if (mid) {
    olsr_delete_mid_entry(mid);
  }
}

/**
 * Wrapper for the timer callback.
 */
static void
olsr_expire_mid_entry(void *context)
{
#ifdef DEBUG
  struct ipaddr_str buf;
#endif /* DEBUG */
  struct mid_entry *mid;

  mid = (struct mid_entry *)context;
  mid->mid_timer = NULL;

#ifdef DEBUG
  OLSR_PRINTF(1, "MID info for %s timed out.. deleting it\n", olsr_ip_to_string(&buf, &mid->main_addr));
#endif /* DEBUG */

  olsr_delete_mid_entry(mid);
}

/**
 * Set the mid set expiration timer.
 *
 * all timer setting shall be done using this function.
 * The timer param is a relative timer expressed in milliseconds.
 */
static void
olsr_set_mid_timer(struct mid_entry *mid, olsr_reltime rel_timer)
{
  int32_t willFireIn = -1;
  if (mid->mid_timer != NULL) willFireIn = olsr_getTimeDue(mid->mid_timer->timer_clock);
  
  if (willFireIn < 0 || (olsr_reltime)willFireIn < rel_timer) {
    olsr_set_timer(&mid->mid_timer, rel_timer, 0, OLSR_TIMER_ONESHOT, &olsr_expire_mid_entry, mid, 0);
  }
}

/**
 * Insert a new interface alias to the interface association set.
 * If the main interface of the association is not yet registered
 * in the set a new entry is created.
 *
 * @param m_addr the main address of the node
 * @param alias the alias address to insert
 * @param vtime the expiration time
 * @return false if mid_address is unnecessary, true otherwise
 */

static bool
insert_mid_tuple(union olsr_ip_addr *m_addr, struct mid_address *alias, olsr_reltime vtime)
{
  struct mid_entry *tmp;
  struct mid_address *tmp_adr;
  uint32_t hash, alias_hash;
  union olsr_ip_addr *registered_m_addr;

  hash = olsr_ip_hashing(m_addr);
  alias_hash = olsr_ip_hashing(&alias->alias);

  /* Check for registered entry */
  for (tmp = mid_set[hash].next; tmp != &mid_set[hash]; tmp = tmp->next) {
    if (ipequal(&tmp->main_addr, m_addr))
      break;
  }

  /* Check if alias is already registered with m_addr */
  registered_m_addr = mid_lookup_main_addr(&alias->alias);
  if (registered_m_addr != NULL && ipequal(registered_m_addr, m_addr)) {
    /* Alias is already registered with main address. Nothing to do here. */
    return false;
  }

  /*
   * Add a rt_path for the alias.
   */
  olsr_insert_routing_table(&alias->alias, olsr_cnf->maxplen, m_addr, OLSR_RT_ORIGIN_MID);

  /*If the address was registered */
  if (tmp != &mid_set[hash]) {
    tmp_adr = tmp->aliases;
    tmp->aliases = alias;
    alias->main_entry = tmp;
    QUEUE_ELEM(reverse_mid_set[alias_hash], alias);
    alias->next_alias = tmp_adr;
    olsr_set_mid_timer(tmp, vtime);
  } else {

    /*Create new node */
    tmp = olsr_malloc(sizeof(struct mid_entry), "MID new alias");

    tmp->aliases = alias;
    alias->main_entry = tmp;
    QUEUE_ELEM(reverse_mid_set[alias_hash], alias);
    tmp->main_addr = *m_addr;
    olsr_set_mid_timer(tmp, vtime);

    /* Queue */
    QUEUE_ELEM(mid_set[hash], tmp);
  }

  /*
   * Delete possible duplicate entries in 2 hop set
   * and delete duplicate neighbor entries. Redirect
   * link entries to the correct neighbor entry.
   *
   *THIS OPTIMIZATION IS NOT SPECIFIED IN RFC3626
   */

  tmp_adr = alias;

  while (tmp_adr) {
    struct neighbor_2_entry *tmp_2_neighbor;
    struct neighbor_entry *tmp_neigh, *real_neigh;

    /* Delete possible 2 hop neighbor */
    if ((tmp_2_neighbor = olsr_lookup_two_hop_neighbor_table_mid(&tmp_adr->alias)) != NULL) {
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "Deleting 2 hop node from MID: %s to ", olsr_ip_to_string(&buf, &tmp_adr->alias));
      OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(&buf, m_addr));

      olsr_delete_two_hop_neighbor_table(tmp_2_neighbor);

      changes_neighborhood = true;
    }

    /* Delete a possible neighbor entry */
    if (((tmp_neigh = olsr_lookup_neighbor_table_alias(&tmp_adr->alias)) != NULL)
        && ((real_neigh = olsr_lookup_neighbor_table_alias(m_addr)) != NULL)) {
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "[MID]Deleting bogus neighbor entry %s real ", olsr_ip_to_string(&buf, &tmp_adr->alias));
      OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(&buf, m_addr));

      replace_neighbor_link_set(tmp_neigh, real_neigh);

      /* Dequeue */
      DEQUEUE_ELEM(tmp_neigh);
      /* Delete */
      free(tmp_neigh);

      changes_neighborhood = true;
    }
    tmp_adr = tmp_adr->next_alias;
  }
  return true;
}

/**
 * Insert an alias address for a node.
 * If the main address is not registered
 * then a new entry is created.
 *
 * @param main_add the main address of the node
 * @param alias the alias address to insert
 * @param vtime the expiration time
 */
void
insert_mid_alias(union olsr_ip_addr *main_add, const union olsr_ip_addr *alias, olsr_reltime vtime)
{
  struct neighbor_entry *ne_old, *ne_new;
  struct mid_entry *me_old;
  int ne_ref_rp_count;
  struct ipaddr_str buf1, buf2;
  struct mid_address *adr;
  if (!olsr_validate_address(alias))
    return;

  OLSR_PRINTF(1, "Inserting alias %s for ", olsr_ip_to_string(&buf1, alias));
  OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(&buf1, main_add));

  adr = olsr_malloc(sizeof(struct mid_address), "Insert MID alias");

  adr->alias = *alias;
  adr->next_alias = NULL;

  /*
   * If we have an entry for this alias in neighbortable, we better adjust it's
   * main address, because otherwise a fatal inconsistency between
   * neighbortable and link_set will be created by way of this mid entry.
   */
  ne_old = olsr_lookup_neighbor_table_alias(alias);
  if (ne_old != NULL) {
    OLSR_PRINTF(2, "Remote main address change detected. Mangling neighbortable to replace %s with %s.\n",
                olsr_ip_to_string(&buf1, alias), olsr_ip_to_string(&buf2, main_add));
    olsr_delete_neighbor_table(alias);
    ne_new = olsr_insert_neighbor_table(main_add);
    /* adjust pointers to neighbortable-entry in link_set */
    ne_ref_rp_count = replace_neighbor_link_set(ne_old, ne_new);
    if (ne_ref_rp_count > 0)
      OLSR_PRINTF(2, "Performed %d neighbortable-pointer replacements (%p -> %p) in link_set.\n", ne_ref_rp_count, ne_old, ne_new);

    me_old = mid_lookup_entry_bymain(alias);
    if (me_old) {

      /*
       * we knew aliases to the previous main address;
       * better forget about them now.
       */
      OLSR_PRINTF(2,
                  "I already have an mid entry mapping addresses to this "
                  "alias address. Removing existing mid entry to preserve consistency of mid_set.\n");
      olsr_delete_mid_entry(me_old);
    }
  }

  if (!insert_mid_tuple(main_add, adr, vtime)) {
    free(adr);
  }

  /*
   *Recalculate topology
   */
  changes_neighborhood = true;
  changes_topology = true;
}

/**
 * Lookup the main address for a alias address
 *
 * @param adr the alias address to check
 * @return the main address registered on the alias
 * or NULL if not found
 */
union olsr_ip_addr *
mid_lookup_main_addr(const union olsr_ip_addr *adr)
{
  uint32_t hash;
  struct mid_address *tmp_list;

  hash = olsr_ip_hashing(adr);

  /*Traverse MID list */
  for (tmp_list = reverse_mid_set[hash].next; tmp_list != &reverse_mid_set[hash]; tmp_list = tmp_list->next) {
    if (ipequal(&tmp_list->alias, adr))
      return &tmp_list->main_entry->main_addr;
  }
  return NULL;

}

/*
 * Find mid entry to an address.
 *
 * @param adr the main address to search for
 * @return a linked list of address structs
 */
struct mid_entry *
mid_lookup_entry_bymain(const union olsr_ip_addr *adr)
{
  struct mid_entry *tmp_list;
  uint32_t hash;

  hash = olsr_ip_hashing(adr);

  /* Check all registered nodes... */
  for (tmp_list = mid_set[hash].next; tmp_list != &mid_set[hash]; tmp_list = tmp_list->next) {
    if (ipequal(&tmp_list->main_addr, adr))
      return tmp_list;
  }
  return NULL;
}

/*
 * Find all aliases for an address.
 *
 * @param adr the main address to search for
 * @return a linked list of addresses structs
 */
struct mid_address *
mid_lookup_aliases(const union olsr_ip_addr *adr)
{
  struct mid_entry *tmp = mid_lookup_entry_bymain(adr);
  return tmp ? tmp->aliases : NULL;
}

/**
 * Update the timer for an MID entry
 *
 * @param adr the main address of the entry
 * @param vtime the expiration time
 * @return 1 if the node was updated, 0 if not
 */
int
olsr_update_mid_table(const union olsr_ip_addr *adr, olsr_reltime vtime)
{
  uint32_t hash;
  struct ipaddr_str buf;
  struct mid_entry *tmp_list = mid_set;

  OLSR_PRINTF(3, "MID: update %s\n", olsr_ip_to_string(&buf, adr));
  hash = olsr_ip_hashing(adr);

  /* Check all registered nodes... */
  for (tmp_list = mid_set[hash].next; tmp_list != &mid_set[hash]; tmp_list = tmp_list->next) {
    /*find match */
    if (ipequal(&tmp_list->main_addr, adr)) {
      olsr_set_mid_timer(tmp_list, vtime);

      return 1;
    }
  }
  return 0;
}

/**
 * Remove aliases from 'entry' which are not listed in 'declared_aliases'.
 *
 * @param message the MID message
 */
static void
olsr_prune_aliases(struct mid_message *message)
{
  const union olsr_ip_addr *m_addr = &message->mid_origaddr;
  struct mid_alias * declared_aliases = message->mid_addr;
  struct mid_entry *entry;
  uint32_t hash;
  struct mid_address *registered_aliases;
  struct mid_address *previous_alias;
  struct mid_alias *save_declared_aliases = declared_aliases;

  hash = olsr_ip_hashing(m_addr);

  /* Check for registered entry */
  for (entry = mid_set[hash].next; entry != &mid_set[hash]; entry = entry->next) {
    if (ipequal(&entry->main_addr, m_addr))
      break;
  }
  if (entry == &mid_set[hash]) {
    /* MID entry not found, nothing to prune here */
    return;
  }

  registered_aliases = entry->aliases;
  previous_alias = NULL;

  while (registered_aliases != NULL) {
    struct mid_address *current_alias = registered_aliases;
    registered_aliases = registered_aliases->next_alias;

    declared_aliases = save_declared_aliases;

    /* Go through the list of declared aliases to find the matching current alias */
    while (declared_aliases != 0 && !ipequal(&current_alias->alias, &declared_aliases->alias_addr)) {
      declared_aliases = declared_aliases->next;
    }

    if (declared_aliases == NULL) {
      /*do not remove alias if vtime still valid (so we assigned something != NULL to declared_aliases)*/
      if (!olsr_isTimedOut(current_alias->vtime)) declared_aliases = save_declared_aliases;
    }
    else current_alias->vtime=olsr_getTimestamp(message->vtime);

    if (declared_aliases == NULL) {
      struct ipaddr_str buf;
      /* Current alias not found in list of declared aliases: free current alias */
      OLSR_PRINTF(1, "MID remove: (%s, ", olsr_ip_to_string(&buf, &entry->main_addr));
      OLSR_PRINTF(1, "%s)\n", olsr_ip_to_string(&buf, &current_alias->alias));

      /* Update linked list as seen by 'entry' */
      if (previous_alias != NULL) {
        previous_alias->next_alias = current_alias->next_alias;
      } else {
        entry->aliases = current_alias->next_alias;
      }

      /* Remove from hash table */
      DEQUEUE_ELEM(current_alias);

      /*
       * Delete the rt_path for the alias.
       */
      olsr_delete_routing_table(&current_alias->alias, olsr_cnf->maxplen, &entry->main_addr);

      free(current_alias);

      /*
       *Recalculate topology
       */
      changes_neighborhood = true;
      changes_topology = true;
    } else {
      previous_alias = current_alias;
    }
  }
}

/**
 * Delete a MID entry
 *
 * @param mid the entry to delete
 */
void
olsr_delete_mid_entry(struct mid_entry *mid)
{
  struct mid_address *aliases;

  /* Free aliases */
  aliases = mid->aliases;
  while (aliases) {
    struct mid_address *tmp_aliases = aliases;
    aliases = aliases->next_alias;
    DEQUEUE_ELEM(tmp_aliases);

    /*
     * Delete the rt_path for the alias.
     */
    olsr_delete_routing_table(&tmp_aliases->alias, olsr_cnf->maxplen, &mid->main_addr);

    free(tmp_aliases);
  }

  /*
   * Kill any pending timers.
   */
  if (mid->mid_timer) {
    olsr_stop_timer(mid->mid_timer);
    mid->mid_timer = NULL;
  }

  /* Dequeue */
  DEQUEUE_ELEM(mid);
  free(mid);
}

/**
 * Print all MID entries
 * For debuging purposes
 */
void
olsr_print_mid_set(void)
{
  int idx;

  OLSR_PRINTF(1, "\n--- %s ------------------------------------------------- MID\n\n", olsr_wallclock_string());

  for (idx = 0; idx < HASHSIZE; idx++) {
    struct mid_entry *tmp_list = mid_set[idx].next;
    /*Traverse MID list */
    for (tmp_list = mid_set[idx].next; tmp_list != &mid_set[idx]; tmp_list = tmp_list->next) {
      struct mid_address *tmp_addr;
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "%s: ", olsr_ip_to_string(&buf, &tmp_list->main_addr));
      for (tmp_addr = tmp_list->aliases; tmp_addr; tmp_addr = tmp_addr->next_alias) {
        OLSR_PRINTF(1, " %s ", olsr_ip_to_string(&buf, &tmp_addr->alias));
      }
      OLSR_PRINTF(1, "\n");
    }
  }
}

/**
 *Process a received(and parsed) MID message
 *For every address check if there is a topology node
 *registered with it and update its addresses.
 *
 *@param m the OLSR message received.
 *@param in_if the incoming interface
 *@param from_addr the sender address
 *@return 1 on success
 */

bool
olsr_input_mid(union olsr_message *m, struct interface_olsr *in_if __attribute__ ((unused)), union olsr_ip_addr *from_addr)
{
  struct ipaddr_str buf;
  struct mid_alias *tmp_adr;
  struct mid_message message;

  mid_chgestruct(&message, m);

  if (!olsr_validate_address(&message.mid_origaddr)) {
    olsr_free_mid_packet(&message);
    return false;
  }
#ifdef DEBUG
  OLSR_PRINTF(5, "Processing MID from %s...\n", olsr_ip_to_string(&buf, &message.mid_origaddr));
#endif /* DEBUG */
  tmp_adr = message.mid_addr;

  /*
   *      If the sender interface (NB: not originator) of this message
   *      is not in the symmetric 1-hop neighborhood of this node, the
   *      message MUST be discarded.
   */

  if (check_neighbor_link(from_addr) != SYM_LINK) {
    OLSR_PRINTF(2, "Received MID from NON SYM neighbor %s\n", olsr_ip_to_string(&buf, from_addr));
    olsr_free_mid_packet(&message);
    return false;
  }

  /* Update the timeout of the MID */
  olsr_update_mid_table(&message.mid_origaddr, message.vtime);

  for (;tmp_adr; tmp_adr = tmp_adr->next) {
#ifndef NO_DUPLICATE_DETECTION_HANDLER
    struct interface_olsr *ifs;
    bool stop = false;
    for (ifs = ifnet; ifs != NULL; ifs = ifs->int_next) {
      if (ipequal(&ifs->ip_addr, &tmp_adr->alias_addr)) {
      /* ignore your own main IP as an incoming MID */
        olsr_handle_mid_collision(&tmp_adr->alias_addr, &message.mid_origaddr);
        stop = true;
        break;
      }
    }
    if (stop) {
      continue;
    }
#endif /* NO_DUPLICATE_DETECTION_HANDLER */
    if (!mid_lookup_main_addr(&tmp_adr->alias_addr)) {
      OLSR_PRINTF(1, "MID new: (%s, ", olsr_ip_to_string(&buf, &message.mid_origaddr));
      OLSR_PRINTF(1, "%s)\n", olsr_ip_to_string(&buf, &tmp_adr->alias_addr));
      insert_mid_alias(&message.mid_origaddr, &tmp_adr->alias_addr, message.vtime);
    } else {
      olsr_insert_routing_table(&tmp_adr->alias_addr, olsr_cnf->maxplen, &message.mid_origaddr, OLSR_RT_ORIGIN_MID);
    }
  }

  olsr_prune_aliases(&message);
  olsr_free_mid_packet(&message);

  /* Forward the message */
  return true;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
