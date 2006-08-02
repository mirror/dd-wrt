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
 * $Id: duplicate_set.c,v 1.12 2005/02/27 18:39:43 kattemat Exp $
 */



#include "defs.h"
#include "duplicate_set.h"
#include "scheduler.h"
#include "olsr.h"

/* The duplicate table */
static struct dup_entry dup_set[HASHSIZE];

static float dup_hold_time = DUP_HOLD_TIME;

/**
 *Initialize the duplicate table entrys
 *
 *@return nada
 */
void
olsr_init_duplicate_table()
{
  int i;

  OLSR_PRINTF(3, "Initializing duplicatetable - hashsize %d\n", HASHSIZE)

  /* Since the holdingtime is rather large for duplicate
   * entries the timeoutfunction is only ran every 2 seconds
   */
  olsr_register_scheduler_event(&olsr_time_out_duplicate_table, NULL, 2, 0, NULL);
  
  for(i = 0; i < HASHSIZE; i++)
    {
      dup_set[i].next = &dup_set[i];
      dup_set[i].prev = &dup_set[i];
    }
}


/**
 *Add an entry to the duplicate set. The set is not checked
 *for duplicate entries.
 *
 *@param originator IP address of the sender of the message
 *@param seqno seqno of the message
 *
 *@return positive on success
 */
struct dup_entry *
olsr_add_dup_entry(union olsr_ip_addr *originator, olsr_u16_t seqno)
{
  olsr_u32_t hash;
  struct dup_entry *new_dup_entry;


  /* Hash the senders address */
  hash = olsr_hashing(originator);

  new_dup_entry = olsr_malloc(sizeof(struct dup_entry), "New dup entry");

  /* Address */
  COPY_IP(&new_dup_entry->addr, originator);
  /* Seqno */
  new_dup_entry->seqno = seqno;
  /* Set timer */
  new_dup_entry->timer = GET_TIMESTAMP(dup_hold_time*1000);
  /* Interfaces */
  new_dup_entry->ifaces = NULL;
  /* Forwarded */
  new_dup_entry->forwarded = 0;

  /* Insert into set */
  QUEUE_ELEM(dup_set[hash], new_dup_entry);
  /*
  dup_set[hash].next->prev = new_dup_entry;
  new_dup_entry->next = dup_set[hash].next;
  dup_set[hash].next = new_dup_entry;
  new_dup_entry->prev = &dup_set[hash];
  */
  return new_dup_entry;
}


/**
 * Check wether or not a message should be processed
 *
 */
int
olsr_check_dup_table_proc(union olsr_ip_addr *originator, olsr_u16_t seqno)
{
  olsr_u32_t hash;
  struct dup_entry *tmp_dup_table;

  /* Hash the senders address */
  hash = olsr_hashing(originator);

  /* Check for entry */
  for(tmp_dup_table = dup_set[hash].next;
      tmp_dup_table != &dup_set[hash];
      tmp_dup_table = tmp_dup_table->next)
    {
      if(COMP_IP(&tmp_dup_table->addr, originator) &&
	 (tmp_dup_table->seqno == seqno))
	{
	  return 0;
	}
    }

  return 1;
}



/**
 * Check wether or not a message should be forwarded
 *
 */
int
olsr_check_dup_table_fwd(union olsr_ip_addr *originator, 
		     olsr_u16_t seqno,
		     union olsr_ip_addr *int_addr)
{
  olsr_u32_t hash;
  struct dup_entry *tmp_dup_table;

  /* Hash the senders address */
  hash = olsr_hashing(originator);

  /* Check for entry */
  for(tmp_dup_table = dup_set[hash].next;
      tmp_dup_table != &dup_set[hash];
      tmp_dup_table = tmp_dup_table->next)
    {
      if(COMP_IP(&tmp_dup_table->addr, originator) &&
	 (tmp_dup_table->seqno == seqno))
	{
	  struct dup_iface *tmp_dup_iface;
	  /* Check retransmitted */
	  if(tmp_dup_table->forwarded)
	    return 0;
	  /* Check for interface */
	  tmp_dup_iface = tmp_dup_table->ifaces;
	  while(tmp_dup_iface)
	    {
	      if(COMP_IP(&tmp_dup_iface->addr, int_addr))
		return 0;
	      
	      tmp_dup_iface = tmp_dup_iface->next;
	    }
	}
    }
  

  return 1;
}




/**
 *Delete and dequeue a duplicate entry
 *
 *@param entry the entry to delete
 *
 */
void
olsr_del_dup_entry(struct dup_entry *entry)
{
  struct dup_iface *tmp_iface, *del_iface;

  tmp_iface = entry->ifaces;

  /* Free interfaces */
  while(tmp_iface)
    {
      del_iface = tmp_iface;
      tmp_iface = tmp_iface->next;
      free(del_iface);
    }

  /* Dequeue */
  DEQUEUE_ELEM(entry);
  free(entry);
}



void
olsr_time_out_duplicate_table(void *foo)
{
  int i;

  for(i = 0; i < HASHSIZE; i++)
    {      
      struct dup_entry *tmp_dup_table;
      tmp_dup_table = dup_set[i].next;

      while(tmp_dup_table != &dup_set[i])
	{
	  if(TIMED_OUT(tmp_dup_table->timer))
	    {
	      struct dup_entry *entry_to_delete = tmp_dup_table;
#ifdef DEBUG
	      OLSR_PRINTF(5, "DUP TIMEOUT[%s] s: %d\n", 
		          olsr_ip_to_string(&tmp_dup_table->addr),
		          tmp_dup_table->seqno)
#endif
	      tmp_dup_table = tmp_dup_table->next;
	      olsr_del_dup_entry(entry_to_delete);
	    }
	  else
	    {
	      tmp_dup_table = tmp_dup_table->next;
	    }
	}
    }
}




int
olsr_update_dup_entry(union olsr_ip_addr *originator, 
		      olsr_u16_t seqno, 
		      union olsr_ip_addr *iface)
{
  olsr_u32_t hash;
  struct dup_entry *tmp_dup_table;
  struct dup_iface *new_iface;

  /* Hash the senders address */
  hash = olsr_hashing(originator);


  /* Check for entry */
  for(tmp_dup_table = dup_set[hash].next;
      tmp_dup_table != &dup_set[hash];
      tmp_dup_table = tmp_dup_table->next)
    {
      if(COMP_IP(&tmp_dup_table->addr, originator) &&
	 (tmp_dup_table->seqno == seqno))
	{
	  break;
	}
    }

  if(tmp_dup_table == &dup_set[hash])
    /* Did not find entry - create it */
    tmp_dup_table = olsr_add_dup_entry(originator, seqno);
  
  /* 0 for now */
  tmp_dup_table->forwarded = 0;
  
  new_iface = olsr_malloc(sizeof(struct dup_iface), "New dup iface");

  COPY_IP(&new_iface->addr, iface);
  new_iface->next = tmp_dup_table->ifaces;
  tmp_dup_table->ifaces = new_iface;
  
  /* Set timer */
  tmp_dup_table->timer = GET_TIMESTAMP(dup_hold_time*1000);
  
  return 1;
}




int
olsr_set_dup_forward(union olsr_ip_addr *originator, 
		     olsr_u16_t seqno)
{
  olsr_u32_t hash;
  struct dup_entry *tmp_dup_table;

  /* Hash the senders address */
  hash = olsr_hashing(originator);

  /* Check for entry */
  for(tmp_dup_table = dup_set[hash].next;
      tmp_dup_table != &dup_set[hash];
      tmp_dup_table = tmp_dup_table->next)
    {
      if(COMP_IP(&tmp_dup_table->addr, originator) &&
	 (tmp_dup_table->seqno == seqno))
	{
	  break;
	}
    }

  if(tmp_dup_table == &dup_set[hash])
    /* Did not find entry !! */
    return 0;
  
#ifdef DEBUG
  OLSR_PRINTF(3, "Setting DUP %s/%d forwarded\n", olsr_ip_to_string(&tmp_dup_table->addr), seqno)
#endif

  /* Set forwarded */
  tmp_dup_table->forwarded = 1;
  
  /* Set timer */
  tmp_dup_table->timer = GET_TIMESTAMP(dup_hold_time*1000);
  
  return 1;
}







void
olsr_print_duplicate_table()
{
  int i;

  printf("\nDUP TABLE:\n");

  for(i = 0; i < HASHSIZE; i++)
    {      
      struct dup_entry *tmp_dup_table = dup_set[i].next;
      
      //printf("Timeout %d %d\n", i, j);
      while(tmp_dup_table != &dup_set[i])
	{
	  printf("[%s] s: %d\n", 
		 olsr_ip_to_string(&tmp_dup_table->addr),
		 tmp_dup_table->seqno);
	  tmp_dup_table = tmp_dup_table->next;
	}
    }
printf("\n");

}
