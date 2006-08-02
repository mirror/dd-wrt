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
 * $Id: tc_set.c,v 1.23 2005/11/17 04:25:44 tlopatic Exp $
 */


#include "tc_set.h"
#include "olsr.h"
#include "scheduler.h"
#include "lq_route.h"


struct tc_entry tc_table[HASHSIZE];


/**
 * Initialize the topology set
 *
 */

int
olsr_init_tc()
{
  int index;
 
  changes = OLSR_FALSE;

  OLSR_PRINTF(5, "TC: init topo\n")

  olsr_register_timeout_function(&olsr_time_out_tc_set);

  for(index=0;index<HASHSIZE;index++)
    {
      tc_table[index].next = &tc_table[index];
      tc_table[index].prev = &tc_table[index];
    }

  return 1;
}


/**
 *Delete a TC entry if it has no associated
 *destinations
 *
 *@param entry the TC entry to check and possibly delete
 *
 *@return 1 if entry deleted 0 if not
 */

int
olsr_tc_delete_entry_if_empty(struct tc_entry *entry)
{

  //OLSR_PRINTF(1, "TC: del entry if empty\n")

  if(entry->destinations.next == &entry->destinations)
    {
      /* dequeue */
      DEQUEUE_ELEM(entry);
      //entry->prev->next = entry->next;
      //entry->next->prev = entry->prev;
      OLSR_PRINTF(1, "TC-SET: Deleting empty entry %s ->\n", olsr_ip_to_string(&entry->T_last_addr))
      free(entry);
      return 1;
    }
  return 0;
}



/**
 * Look up a entry from the TC tabe based
 * on address
 *
 *@param adr the address to look for
 *
 *@return the entry found or NULL
 */

struct tc_entry *
olsr_lookup_tc_entry(union olsr_ip_addr *adr)
{
  struct tc_entry *entries;
  olsr_u32_t hash;

  //OLSR_PRINTF(1, "TC: lookup entry\n")

  hash = olsr_hashing(adr);

  for(entries = tc_table[hash].next; 
      entries != &tc_table[hash]; 
      entries = entries->next)
    {
      //printf("TC lookup checking: %s\n", olsr_ip_to_string(&entries->T_last_addr));
      if(COMP_IP(adr, &entries->T_last_addr))
	return entries;
    }

  return NULL;
}


/**
 *Add a new tc_entry to the tc set
 *
 *@param (last)adr address of the entry
 *
 *@return a pointer to the created entry
 */

struct tc_entry *
olsr_add_tc_entry(union olsr_ip_addr *adr)
{
  struct tc_entry *new_entry;
  olsr_u32_t hash;

  OLSR_PRINTF(1, "TC: adding entry %s\n", olsr_ip_to_string(adr))

  hash = olsr_hashing(adr);

  new_entry = olsr_malloc(sizeof(struct tc_entry), "New TC entry");

  /* Fill entry */
  COPY_IP(&new_entry->T_last_addr, adr);
  new_entry->destinations.next = &new_entry->destinations;
  new_entry->destinations.prev = &new_entry->destinations;

  /* Queue entry */
  QUEUE_ELEM(tc_table[hash], new_entry);
  /*
  new_entry->next = tc_table[hash].next;
  new_entry->prev = tc_table[hash].next->prev;
  tc_table[hash].next->prev = new_entry;
  tc_table[hash].next = new_entry;
  */

  return new_entry;
}


/**
 *Delete all destinations that have a
 *lower ANSN than the one in the message
 *
 *@param entry the entry to delete destenations from
 *@param msg the message to fetch the ANSN from
 *
 *@return 1 if any destinations were deleted 0 if not
 */

int
olsr_tc_delete_mprs(struct tc_entry *entry, struct tc_message *msg)
{
  struct topo_dst *tmp_dsts, *dst_to_del;
  int retval;

  //OLSR_PRINTF(5, "TC: deleting MPRS\n")

  tmp_dsts = entry->destinations.next;
  retval = 0;

  while(tmp_dsts != &entry->destinations)
    {
      if(SEQNO_GREATER_THAN(msg->ansn, tmp_dsts->T_seq))
	{
	  /* Delete entry */
	  dst_to_del = tmp_dsts;
	  tmp_dsts = tmp_dsts->next;

	  /* dequeue */
	  DEQUEUE_ELEM(dst_to_del);

	  free(dst_to_del);
	  retval = 1;
	}
      else
	tmp_dsts = tmp_dsts->next;

    }

  return retval;
}


/**
 *Update the destinations registered on an entry.
 *Creates new dest-entries if not registered.
 *Bases update on a receivied TC message
 *
 *@param entry the TC entry to check
 *@msg the TC message to update by
 *
 *@return 1 if entries are added 0 if not
 */

int
olsr_tc_update_mprs(struct tc_entry *entry, struct tc_message *msg)
{
  struct tc_mpr_addr *mprs;
  struct topo_dst *new_topo_dst, *existing_dst;
  int retval;

  //OLSR_PRINTF(1, "TC: update MPRS\n")

  retval = 0;


  mprs = msg->multipoint_relay_selector_address;
  
  /* Add all the MPRs */

  while(mprs != NULL)
    {
      existing_dst = olsr_tc_lookup_dst(entry, &mprs->address);

      if(existing_dst == NULL)
	{
	  /* New entry */
	  new_topo_dst = olsr_malloc(sizeof(struct topo_dst), "add TC destination");

	  memset(new_topo_dst, 0, sizeof(struct topo_dst));

	  COPY_IP(&new_topo_dst->T_dest_addr, &mprs->address);
	  new_topo_dst->T_time = GET_TIMESTAMP(msg->vtime*1000);
	  new_topo_dst->T_seq = msg->ansn;

          if (olsr_cnf->lq_level > 0)
            {
              new_topo_dst->link_quality = mprs->neigh_link_quality;
              new_topo_dst->inverse_link_quality = mprs->link_quality;

              new_topo_dst->saved_link_quality = new_topo_dst->link_quality;
              new_topo_dst->saved_inverse_link_quality =
                new_topo_dst->inverse_link_quality;
            }

	  /* Add to queue */
	  new_topo_dst->prev = &entry->destinations;
	  new_topo_dst->next = entry->destinations.next;
	  entry->destinations.next->prev = new_topo_dst;
	  entry->destinations.next = new_topo_dst;

	  retval = 1;
	}
      else
	{
	  /* Update entry */
	  existing_dst->T_time = GET_TIMESTAMP(msg->vtime*1000);
	  existing_dst->T_seq = msg->ansn;

          if (olsr_cnf->lq_level > 0)
            {
              double saved_lq, rel_lq;

              saved_lq = existing_dst->saved_link_quality;

              if (saved_lq == 0.0)
                saved_lq = -1.0;

              existing_dst->link_quality = mprs->neigh_link_quality;

              rel_lq = existing_dst->link_quality / saved_lq;

              if (rel_lq > 1.1 || rel_lq < 0.9)
                {
                  existing_dst->saved_link_quality =
                    existing_dst->link_quality;

                  if (msg->hop_count <= olsr_cnf->lq_dlimit)
                    retval = 1;

                  else
                    OLSR_PRINTF(3, "Skipping Dijkstra (4)\n")
                }

              saved_lq = existing_dst->saved_inverse_link_quality;

              if (saved_lq == 0.0)
                saved_lq = -1.0;

              existing_dst->inverse_link_quality = mprs->link_quality;

              rel_lq = existing_dst->inverse_link_quality / saved_lq;

              if (rel_lq > 1.1 || rel_lq < 0.9)
                {
                  existing_dst->saved_inverse_link_quality =
                    existing_dst->inverse_link_quality;

                  if (msg->hop_count <= olsr_cnf->lq_dlimit)
                    retval = 1;

                  else
                    OLSR_PRINTF(3, "Skipping Dijkstra (5)\n")
                }
            }
	}

      mprs = mprs->next;
    }

  return retval;
}



/**
 *Lookup a destination in a TC entry
 *
 *@param entry the entry to check
 *@param dst_addr the destination address to check for
 *
 *@return a pointer to the topo_dst found - or NULL
 */
struct topo_dst *
olsr_tc_lookup_dst(struct tc_entry *entry, union olsr_ip_addr *dst_addr)
{
  struct topo_dst *dsts;
  
  //OLSR_PRINTF(1, "TC: lookup dst\n")

  for(dsts = entry->destinations.next; 
      dsts != &entry->destinations; 
      dsts = dsts->next)
    {
      if(COMP_IP(dst_addr, &dsts->T_dest_addr))
	return dsts;
    }
  return NULL;
}






/**
 * Time out entries
 *
 *@return nada
 */

void
olsr_time_out_tc_set()
{
  int index, deleted;
  struct tc_entry *entry, *entry2;
  struct topo_dst *dst_entry, *dst_to_delete;


  for(index=0;index<HASHSIZE;index++)
    {
      /* For all TC entries */
      entry = tc_table[index].next;
      while(entry != &tc_table[index])
	{
	  //printf("INDEX: %d\n", index);
	  /* For all destination entries of that TC entry */
	  deleted = 0;
	  dst_entry = entry->destinations.next;
	  while(dst_entry != &entry->destinations)
	    {
	      /* If timed out - delete */
	      if(TIMED_OUT(dst_entry->T_time))
		{
		  deleted = 1;
		  /* Dequeue */
		  DEQUEUE_ELEM(dst_entry);
		  dst_to_delete = dst_entry;
		  dst_entry = dst_entry->next;

		  /* Delete */
		  free(dst_to_delete);

		  changes_topology = OLSR_TRUE;

		}
	      else
		dst_entry = dst_entry->next;
	    }
	  /* Delete entry if no destinations */
	  entry2 = entry;
	  entry = entry->next;
	  if(deleted)
	    olsr_tc_delete_entry_if_empty(entry2);
	}
    }

  return;
}


/**
 *Print the topology table to stdout
 */
int
olsr_print_tc_table()
{
  int i;
  struct tc_entry *entry;
  struct topo_dst *dst_entry;
  char *fstr;
  float etx;
  
  OLSR_PRINTF(1, "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- TOPOLOGY\n\n",
              nowtm->tm_hour,
              nowtm->tm_min,
              nowtm->tm_sec,
              (int)now.tv_usec / 10000)

  if (olsr_cnf->ip_version == AF_INET)
  {
    OLSR_PRINTF(1, "Source IP addr   Dest IP addr     LQ     ILQ    ETX\n")
    fstr = "%-15s  %-15s  %5.3f  %5.3f  %.2f\n";
  }

  else
  {
    OLSR_PRINTF(1, "Source IP addr                Dest IP addr                    LQ     ILQ    ETX\n")
    fstr = "%-30s%-30s  %5.3f  %5.3f  %.2f\n";
  }

  for (i = 0; i < HASHSIZE; i++)
  {
    entry = tc_table[i].next;

    while (entry != &tc_table[i])
    {
      dst_entry = entry->destinations.next;

      while(dst_entry != &entry->destinations)
      {
        if (dst_entry->link_quality < MIN_LINK_QUALITY ||
            dst_entry->inverse_link_quality < MIN_LINK_QUALITY)
          etx = 0.0;

        else
          etx = 1.0 / (dst_entry->link_quality *
                       dst_entry->inverse_link_quality);

        OLSR_PRINTF(1, fstr, olsr_ip_to_string(&entry->T_last_addr),
                    olsr_ip_to_string(&dst_entry->T_dest_addr),
                    dst_entry->link_quality, dst_entry->inverse_link_quality,
                    etx)

        dst_entry = dst_entry->next;
      }

      entry = entry->next;
    }
  }

  return 1;
}
