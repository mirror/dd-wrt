/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: mid_set.c,v 1.17 2007/02/10 19:27:32 bernd67 Exp $
 */

#include "defs.h"
#include "two_hop_neighbor_table.h"
#include "mid_set.h"
#include "olsr.h"
#include "scheduler.h"
#include "neighbor_table.h"
#include "link_set.h"
#include "packet.h" /* struct mid_alias */


struct mid_entry mid_set[HASHSIZE];
struct mid_address reverse_mid_set[HASHSIZE];

struct mid_entry *mid_lookup_entry_bymain(union olsr_ip_addr *adr);

/**
 * Initialize the MID set
 *
 */

int
olsr_init_mid_set()
{
  int index;

  OLSR_PRINTF(5, "MID: init\n")

  /* Since the holdingtime is assumed to be rather large for 
   * MID entries, the timeoutfunction is only ran once every second
   */
  olsr_register_scheduler_event(&olsr_time_out_mid_set, NULL, 1, 0, NULL);

  for(index=0;index<HASHSIZE;index++)
    {
      mid_set[index].next = &mid_set[index];
      mid_set[index].prev = &mid_set[index];

      reverse_mid_set[index].next = &reverse_mid_set[index];
      reverse_mid_set[index].prev = &reverse_mid_set[index];
    }

  return 1;
}


/**
 *Insert a new interface alias to the interface association set.
 *If the main interface of the association is not yet registered
 *in the set a new entry is created.
 *
 *@param m_addr the main address of the node
 *@param alias the alias address to insert
 *
 *@return nada
 */

void 
insert_mid_tuple(union olsr_ip_addr *m_addr, struct mid_address *alias, float vtime)
{
  struct mid_entry *tmp;
  struct mid_address *tmp_adr;
  olsr_u32_t hash, alias_hash;
  union olsr_ip_addr *registered_m_addr;

  hash = olsr_hashing(m_addr);
  alias_hash = olsr_hashing(&alias->alias);

  /* Check for registered entry */
  for(tmp = mid_set[hash].next;
      tmp != &mid_set[hash];
      tmp = tmp->next)
    {
      if(COMP_IP(&tmp->main_addr, m_addr))
	break;
     }

  /* Check if alias is already registered with m_addr */
  registered_m_addr = mid_lookup_main_addr(&alias->alias);
  if (registered_m_addr != NULL && COMP_IP(registered_m_addr, m_addr))
    {
      /* Alias is already registered with main address. Nothing to do here. */
      return;
    }

  /*If the address was registered*/ 
  if(tmp != &mid_set[hash])
    {
      tmp_adr = tmp->aliases;
      tmp->aliases = alias;
      alias->main_entry = tmp;
      QUEUE_ELEM(reverse_mid_set[alias_hash], alias);
      alias->next_alias = tmp_adr;
      tmp->ass_timer = GET_TIMESTAMP(vtime*1000);
    }
      /*Create new node*/
  else
    {
      tmp = olsr_malloc(sizeof(struct mid_entry), "MID new alias");

      tmp->aliases = alias;
      alias->main_entry = tmp;
      QUEUE_ELEM(reverse_mid_set[alias_hash], alias);
      COPY_IP(&tmp->main_addr, m_addr);
      tmp->ass_timer = GET_TIMESTAMP(vtime*1000);
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

  while(tmp_adr)
    {
      struct neighbor_2_entry *tmp_2_neighbor;
      struct neighbor_entry *tmp_neigh, *real_neigh;

      /* Delete possible 2 hop neighbor */
      if((tmp_2_neighbor = olsr_lookup_two_hop_neighbor_table_mid(&tmp_adr->alias)) != NULL)
	{
	  OLSR_PRINTF(1, "Deleting 2 hop node from MID: %s to ", olsr_ip_to_string(&tmp_adr->alias))
	  OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(m_addr))

	  olsr_delete_two_hop_neighbor_table(tmp_2_neighbor);

	  changes_neighborhood = OLSR_TRUE;
	}

      /* Delete a possible neighbor entry */
      if(((tmp_neigh = olsr_lookup_neighbor_table_alias(&tmp_adr->alias)) != NULL)
	 && ((real_neigh = olsr_lookup_neighbor_table_alias(m_addr)) != NULL))

	{
	  OLSR_PRINTF(1, "[MID]Deleting bogus neighbor entry %s real ", olsr_ip_to_string(&tmp_adr->alias))
	  OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(m_addr))

	  replace_neighbor_link_set(tmp_neigh, real_neigh);

	  /* Dequeue */
	  DEQUEUE_ELEM(tmp_neigh);
	  /* Delete */
	  free(tmp_neigh);

	  changes_neighborhood = OLSR_TRUE;
	}
      
      tmp_adr = tmp_adr->next_alias;
    }


}


/**
 *Insert an alias address for a node.
 *If the main address is not registered
 *then a new entry is created.
 *
 *@param main_add the main address of the node
 *@param alias the alias address to insert
 *@param seq the sequence number to register a new node with
 *
 *@return nada
 */
void
insert_mid_alias(union olsr_ip_addr *main_add, union olsr_ip_addr *alias, float vtime)
{
  struct mid_address *adr;
  struct neighbor_entry *ne_old, *ne_new;
  struct mid_entry *me_old;
  int ne_ref_rp_count;

  adr = olsr_malloc(sizeof(struct mid_address), "Insert MID alias");
  
  OLSR_PRINTF(1, "Inserting alias %s for ", olsr_ip_to_string(alias))
  OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(main_add))

  COPY_IP(&adr->alias, alias);
  adr->next_alias = NULL;
  
  // If we have an entry for this alias in neighbortable, we better adjust it's
  // main address, because otherwise a fatal inconsistency between
  // neighbortable and link_set will be created by way of this mid entry.
  ne_old = olsr_lookup_neighbor_table_alias(alias);
  if (ne_old != NULL) {
     OLSR_PRINTF(2, "Remote main address change detected. Mangling neighbortable to replace %s with %s.\n", olsr_ip_to_string(alias), olsr_ip_to_string(main_add));
     olsr_delete_neighbor_table(alias);
     ne_new = olsr_insert_neighbor_table(main_add);
     // adjust pointers to neighbortable-entry in link_set
     ne_ref_rp_count = replace_neighbor_link_set(ne_old, ne_new);
     if (ne_ref_rp_count > 0)
        OLSR_PRINTF(2, "Performed %d neighbortable-pointer replacements (%p -> %p) in link_set.\n", ne_ref_rp_count, ne_old, ne_new);
     
     me_old = mid_lookup_entry_bymain(alias);
     if (me_old) {
        // we knew aliases to the previous main address; better forget about
        // them now.
        OLSR_PRINTF(2, "I already have an mid entry mapping addresses to this alias address. Removing existing mid entry to preserve consistency of mid_set.\n");
        mid_delete_node(me_old);
     }
  }
  
  insert_mid_tuple(main_add, adr, vtime);
  
  /*
   *Recalculate topology
   */
  changes_neighborhood = OLSR_TRUE;
  changes_topology = OLSR_TRUE;
  
  //print_mid_list();
}




/**
 *Lookup the main address for a alias address
 *
 *@param adr the alias address to check
 *
 *@return the main address registered on the alias
 *or NULL if not found
 */
union olsr_ip_addr *
mid_lookup_main_addr(union olsr_ip_addr *adr)
{
  olsr_u32_t hash;
  struct mid_address *tmp_list;

  hash = olsr_hashing(adr);
  /*Traverse MID list*/
  for(tmp_list = reverse_mid_set[hash].next; 
      tmp_list != &reverse_mid_set[hash];
      tmp_list = tmp_list->next)
	{
	  if(COMP_IP(&tmp_list->alias, adr))
	    return &tmp_list->main_entry->main_addr;
	}
  return NULL;

}


/* Find mid entry to an address.
 * @param adr the main address to search for
 *
 * @return a linked list of address structs
 */
struct mid_entry *
mid_lookup_entry_bymain(union olsr_ip_addr *adr)
{
  struct mid_entry *tmp_list;
  olsr_u32_t hash;

  //OLSR_PRINTF(1, "MID: lookup entry...")

  hash = olsr_hashing(adr);

  /* Check all registered nodes...*/
  for(tmp_list = mid_set[hash].next;
      tmp_list != &mid_set[hash];
      tmp_list = tmp_list->next)
    {
      if(COMP_IP(&tmp_list->main_addr, adr))
	return tmp_list;
    }


  return NULL;
}


/*
 *Find all aliases for an address.
 *
 *@param adr the main address to search for
 *
 *@return a linked list of addresses structs
 */
inline struct mid_address *
mid_lookup_aliases(union olsr_ip_addr *adr)
{
  struct mid_entry *tmp = mid_lookup_entry_bymain(adr);
  return tmp ? tmp->aliases : NULL;
}


/**
 *Update the timer for an entry
 *
 *@param adr the main address of the entry
 *
 *@return 1 if the node was updated, 0 if not
 */
int
olsr_update_mid_table(union olsr_ip_addr *adr, float vtime)
{
  struct mid_entry *tmp_list = mid_set;
  olsr_u32_t hash;

  OLSR_PRINTF(3, "MID: update %s\n", olsr_ip_to_string(adr))
  hash = olsr_hashing(adr);

  /* Check all registered nodes...*/
  for(tmp_list = mid_set[hash].next;
      tmp_list != &mid_set[hash];
      tmp_list = tmp_list->next)
    {
      /*find match*/
      if(COMP_IP(&tmp_list->main_addr, adr))
	{
	  // printf("MID: Updating timer for node %s\n", olsr_ip_to_string(&tmp_list->main_addr));
	  tmp_list->ass_timer = GET_TIMESTAMP(vtime*1000);

	  return 1;
	}
    }
  return 0;
}


/**
 *Remove aliases from 'entry' which are not listed in 'declared_aliases'.
 *
 *@param entry the MID entry
 *@param declared_aliases the list of declared aliases for the MID entry
 *
 *@return nada
 */
void
olsr_prune_aliases(union olsr_ip_addr *m_addr, struct mid_alias *declared_aliases)
{
  struct mid_entry *entry;
  olsr_u32_t hash;
  struct mid_address *registered_aliases;
  struct mid_address *previous_alias;
  struct mid_alias *save_declared_aliases = declared_aliases;

  hash = olsr_hashing(m_addr);

  /* Check for registered entry */
  for(entry = mid_set[hash].next;
      entry != &mid_set[hash];
      entry = entry->next)
    {
      if(COMP_IP(&entry->main_addr, m_addr))
	break;
    }
  if(entry == &mid_set[hash])
    {
      /* MID entry not found, nothing to prune here */
      return;
    }

  registered_aliases = entry->aliases;
  previous_alias = NULL;

  while(registered_aliases != 0)
    {
      struct mid_address *current_alias = registered_aliases;
      registered_aliases = registered_aliases->next_alias;

      declared_aliases = save_declared_aliases;

      /* Go through the list of declared aliases to find the matching current alias */
      while(declared_aliases != 0 &&
            ! COMP_IP(&current_alias->alias, &declared_aliases->alias_addr))
        {
          declared_aliases = declared_aliases->next;
        }

      if (declared_aliases == 0)
        {
          /* Current alias not found in list of declared aliases: free current alias */
          OLSR_PRINTF(1, "MID remove: (%s, ", olsr_ip_to_string(&entry->main_addr))
          OLSR_PRINTF(1, "%s)\n", olsr_ip_to_string(&current_alias->alias))

          /* Update linked list as seen by 'entry' */
          if (previous_alias != NULL)
            {
              previous_alias->next_alias = current_alias->next_alias;
            }
          else
            {
              entry->aliases = current_alias->next_alias;
            }

          /* Remove from hash table */
          DEQUEUE_ELEM(current_alias);
 
          free(current_alias);

          /*
           *Recalculate topology
           */
          changes_neighborhood = OLSR_TRUE;
          changes_topology = OLSR_TRUE;
        }
      else
        {
          previous_alias = current_alias;
        }
    }
}



/**
 *Find timed out entries and delete them
 *
 *@return nada
 */
void
olsr_time_out_mid_set(void *foo)
{
  int index;


  for(index=0;index<HASHSIZE;index++)
    {
      struct mid_entry *tmp_list = mid_set[index].next;
      /*Traverse MID list*/
      while(tmp_list != &mid_set[index])
	{
	  /*Check if the entry is timed out*/
	  if(TIMED_OUT(tmp_list->ass_timer))
	    {
	      struct mid_entry *entry_to_delete = tmp_list;
	      tmp_list = tmp_list->next;
#ifdef DEBUG
	      OLSR_PRINTF(1, "MID info for %s timed out.. deleting it\n", 
			  olsr_ip_to_string(&entry_to_delete->main_addr))
#endif
	      /*Delete it*/
	      mid_delete_node(entry_to_delete);
	    }
	  else
	      tmp_list = tmp_list->next;
	}
    }

  return;
}


/*
 *Delete an entry
 *
 *@param entry the entry to delete
 *
 *@return 1
 */
int
mid_delete_node(struct mid_entry *entry)
{
  struct mid_address *aliases;

  /* Free aliases */
  aliases = entry->aliases;
  while(aliases)
    {
      struct mid_address *tmp_aliases = aliases;
      aliases = aliases->next_alias;
      DEQUEUE_ELEM(tmp_aliases);
      free(tmp_aliases);
    }
  /* Dequeue */
  DEQUEUE_ELEM(entry);
  free(entry);
  
  return 0;
}


/**
 *Print all multiple interface info
 *For debuging purposes
 */
void
olsr_print_mid_set()
{

  int index;

  OLSR_PRINTF(1, "mid set: %02d:%02d:%02d.%06lu\n",nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, now.tv_usec)

  for(index=0;index<HASHSIZE;index++)
    {
      struct mid_entry *tmp_list = mid_set[index].next;
      /*Traverse MID list*/
      for(tmp_list = mid_set[index].next;
	  tmp_list != &mid_set[index];
	  tmp_list = tmp_list->next)
	{
	  struct mid_address *tmp_addr = tmp_list->aliases;

	  OLSR_PRINTF(1, "%s: ", olsr_ip_to_string(&tmp_list->main_addr))
	  while(tmp_addr)
	    {
	      OLSR_PRINTF(1, " %s ", olsr_ip_to_string(&tmp_addr->alias))
	      tmp_addr = tmp_addr->next_alias;
	    }
	  OLSR_PRINTF(1, "\n")	  
	}
    }

}





