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
 * $Id: mpr_selector_set.c,v 1.14 2005/12/16 08:21:59 kattemat Exp $
 */


#include "defs.h"
#include "mpr_selector_set.h"
#include "olsr.h"
#include "scheduler.h"

static olsr_u16_t ansn;

/* MPR selector list */
static struct mpr_selector mprs_list;

/**
 *Initialize MPR selector set
 */

int
olsr_init_mprs_set()
{
  OLSR_PRINTF(5, "MPRS: Init\n")
  /* Initial values */
  ansn = 0;

  olsr_register_timeout_function(&olsr_time_out_mprs_set);
  
  mprs_list.next = &mprs_list;
  mprs_list.prev = &mprs_list;
  

  return 1;
}


olsr_u16_t 
get_local_ansn()
{
  return ansn;
}

void
increase_local_ansn()
{
  ansn++;
}

/**
 * Check if we(this node) is selected as a MPR by any
 * neighbors. If the list is empty we are not MPR.
 */
olsr_bool
olsr_is_mpr()
{
    return ((mprs_list.next == &mprs_list) ? OLSR_FALSE : OLSR_TRUE);
}

/**
 *Add a MPR selector to the MPR selector set
 *
 *@param add address of the MPR selector
 *@param vtime validity time for the new entry
 *
 *@return a pointer to the new entry
 */
struct mpr_selector *
olsr_add_mpr_selector(union olsr_ip_addr *addr, float vtime)
{
  struct mpr_selector *new_entry;

  OLSR_PRINTF(1, "MPRS: adding %s\n", olsr_ip_to_string(addr))

  new_entry = olsr_malloc(sizeof(struct mpr_selector), "Add MPR selector");

  /* Fill struct */
  COPY_IP(&new_entry->MS_main_addr, addr);
  new_entry->MS_time = GET_TIMESTAMP(vtime*1000);

  /* Queue */
  QUEUE_ELEM(mprs_list, new_entry);
  /*
  new_entry->prev = &mprs_list;
  new_entry->next = mprs_list.next;
  mprs_list.next->prev = new_entry;
  mprs_list.next = new_entry;
  */

  return new_entry;
}



/**
 *Lookup an entry in the MPR selector table
 *based on address
 *
 *@param addr the addres to check for
 *
 *@return a pointer to the entry or NULL
 */
struct mpr_selector *
olsr_lookup_mprs_set(union olsr_ip_addr *addr)
{
  struct mpr_selector *mprs;

  if(addr == NULL)
    return NULL;
  //OLSR_PRINTF(1, "MPRS: Lookup....")

  mprs = mprs_list.next;

  while(mprs != &mprs_list)
    {

      if(COMP_IP(&mprs->MS_main_addr, addr))
	{
	  //OLSR_PRINTF(1, "MATCH\n")
	  return mprs;
	}
      mprs = mprs->next;
    }
  
  //OLSR_PRINTF(1, "NO MACH\n")
  return NULL;
}


/**
 *Update a MPR selector entry or create an new
 *one if it does not exist
 *
 *@param addr the address of the MPR selector
 *@param vtime tha validity time of the entry
 *
 *@return 1 if a new entry was added 0 if not
 */
int
olsr_update_mprs_set(union olsr_ip_addr *addr, float vtime)
{
  struct mpr_selector *mprs;
  int retval;

  OLSR_PRINTF(5, "MPRS: Update %s\n", olsr_ip_to_string(addr))

  retval = 0;

  if(NULL == (mprs = olsr_lookup_mprs_set(addr)))
    {
      olsr_add_mpr_selector(addr, vtime);
      retval = 1;
      changes = OLSR_TRUE;
    }
  else
    {
      mprs->MS_time = GET_TIMESTAMP(vtime*1000);
    }
  return retval;
}





/**
 *Time out MPR selector entries
 *
 *@return nada
 */
void
olsr_time_out_mprs_set()
{
  struct mpr_selector *mprs;

  mprs = mprs_list.next;

  while(mprs != &mprs_list)
    {

      if(TIMED_OUT(mprs->MS_time))
	{
	  /* Dequeue */
	  struct mpr_selector *mprs_to_delete = mprs;
	  mprs = mprs->next;

	  OLSR_PRINTF(1, "MPRS: Timing out %s\n", olsr_ip_to_string(&mprs_to_delete->MS_main_addr))

	  DEQUEUE_ELEM(mprs_to_delete);

	  /* Delete entry */
	  free(mprs_to_delete);
	  changes = OLSR_TRUE;
	}
      else
	mprs = mprs->next;
    }

}



/**
 *Print the current MPR selector set to STDOUT
 */
void
olsr_print_mprs_set()
{
  struct mpr_selector *mprs;


  mprs = mprs_list.next;
  OLSR_PRINTF(1, "MPR SELECTORS: ")

  while(mprs != &mprs_list)
    {
      OLSR_PRINTF(1, "%s ", olsr_ip_to_string(&mprs->MS_main_addr))
      mprs = mprs->next;
    }
  OLSR_PRINTF(1, "\n")
}
