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
 * $Id: hna_set.c,v 1.17 2005/05/29 12:47:45 br1 Exp $
 */

#include "defs.h"
#include "olsr.h"
#include "scheduler.h"


struct hna_entry hna_set[HASHSIZE];
size_t netmask_size;


/**
 *Initialize the HNA set
 */
int
olsr_init_hna_set()
{

  int index;

  if(olsr_cnf->ip_version == AF_INET)
    {
      netmask_size = sizeof(olsr_u32_t);
    }
  else
    {
      netmask_size = sizeof(olsr_u16_t);
    }

  /* Since the holdingtime is assumed to be rather large for 
   * HNA entries, the timeoutfunction is only ran once every second
   */
  olsr_register_scheduler_event(&olsr_time_out_hna_set, NULL, 1, 0, NULL);

  for(index=0;index<HASHSIZE;index++)
    {
      hna_set[index].next = &hna_set[index];
      hna_set[index].prev = &hna_set[index];
    }

  return 1;
}




/**
 *Lookup a network entry in a networkentry list
 *
 *@param nets the network list to look in
 *@param net the network to look for
 *@param mask the netmask to look for
 *
 *@return the localted entry or NULL of not found
 */
struct hna_net *
olsr_lookup_hna_net(struct hna_net *nets, union olsr_ip_addr *net, union hna_netmask *mask)
{
  struct hna_net *tmp_net;


  /* Loop trough entrys */
  for(tmp_net = nets->next;
      tmp_net != nets;
      tmp_net = tmp_net->next)
    { 
      if(COMP_IP(&tmp_net->A_network_addr, net) &&
	 (memcmp(&tmp_net->A_netmask, mask, netmask_size) == 0))
	return tmp_net;
    }
  
  /* Not found */
  return NULL;
}




/**
 *Lookup a gateway entry
 *
 *@param gw the address of the gateway
 *
 *@return the located entry or NULL if not found
 */
struct hna_entry *
olsr_lookup_hna_gw(union olsr_ip_addr *gw)
{
  struct hna_entry *tmp_hna;
  olsr_u32_t hash;

  //OLSR_PRINTF(5, "TC: lookup entry\n")

  hash = olsr_hashing(gw);
  
  /* Check for registered entry */
  for(tmp_hna = hna_set[hash].next;
      tmp_hna != &hna_set[hash];
      tmp_hna = tmp_hna->next)
    {
      if(COMP_IP(&tmp_hna->A_gateway_addr, gw))
	return tmp_hna;
    }
  
  /* Not found */
  return NULL;
}



/**
 *Add a gatewayentry to the HNA set
 *
 *@param addr the address of the gateway
 *
 *@return the created entry
 */
struct hna_entry *
olsr_add_hna_entry(union olsr_ip_addr *addr)
{
  struct hna_entry *new_entry;
  olsr_u32_t hash;

  new_entry = olsr_malloc(sizeof(struct hna_entry), "New HNA entry");

  /* Fill struct */
  COPY_IP(&new_entry->A_gateway_addr, addr);

  /* Link nets */
  new_entry->networks.next = &new_entry->networks;
  new_entry->networks.prev = &new_entry->networks;

  /* queue */
  hash = olsr_hashing(addr);
  
  hna_set[hash].next->prev = new_entry;
  new_entry->next = hna_set[hash].next;
  hna_set[hash].next = new_entry;
  new_entry->prev = &hna_set[hash];

  return new_entry;

}



/**
 *Adds a ntework entry to a HNA gateway
 *
 *@param hna_gw the gateway entry to add the
 *network to
 *@param net the networkaddress to add
 *@param mask the netmask
 *
 *@return the newly created entry
 */
struct hna_net *
olsr_add_hna_net(struct hna_entry *hna_gw, union olsr_ip_addr *net, union hna_netmask *mask)
{
  struct hna_net *new_net;


  /* Add the net */
  new_net = olsr_malloc(sizeof(struct hna_net), "Add HNA net");
  
  /* Fill struct */
  COPY_IP(&new_net->A_network_addr, net);
  memcpy(&new_net->A_netmask, mask, netmask_size);

  /* Queue */
  hna_gw->networks.next->prev = new_net;
  new_net->next = hna_gw->networks.next;
  hna_gw->networks.next = new_net;
  new_net->prev = &hna_gw->networks;

  return new_net;
}




/**
 * Update a HNA entry. If it does not exist it
 * is created.
 * This is the only function that should be called 
 * from outside concerning creation of HNA entries.
 *
 *@param gw address of the gateway
 *@param net address of the network
 *@param mask the netmask
 *@param vtime the validitytime of the entry
 *
 *@return nada
 */
void
olsr_update_hna_entry(union olsr_ip_addr *gw, union olsr_ip_addr *net, union hna_netmask *mask, float vtime)
{
  struct hna_entry *gw_entry;
  struct hna_net *net_entry;

  if((gw_entry = olsr_lookup_hna_gw(gw)) == NULL)
    /* Need to add the entry */
    gw_entry = olsr_add_hna_entry(gw);
  
  if((net_entry = olsr_lookup_hna_net(&gw_entry->networks, net, mask)) == NULL)
    {
      /* Need to add the net */
      net_entry = olsr_add_hna_net(gw_entry, net, mask);
      changes_hna = OLSR_TRUE;
    }

  /* Update holdingtime */
  net_entry->A_time = GET_TIMESTAMP(vtime*1000);

}






/**
 *Function that times out all entrys in the hna set and
 *deletes the timed out ones.
 *
 *@return nada
 */
void
olsr_time_out_hna_set(void *foo)
{
  int index;

  for(index=0;index<HASHSIZE;index++)
    {
      struct hna_entry *tmp_hna = hna_set[index].next;
      /* Check all entrys */
      while(tmp_hna != &hna_set[index])
	{
	  /* Check all networks */
	  struct hna_net *tmp_net = tmp_hna->networks.next;

	  while(tmp_net != &tmp_hna->networks)
	    {
	      if(TIMED_OUT(tmp_net->A_time))
		{
		  struct hna_net *net_to_delete = tmp_net;
		  tmp_net = tmp_net->next;
		  DEQUEUE_ELEM(net_to_delete);
		  free(net_to_delete);
		  changes_hna = OLSR_TRUE;
		}
	      else
		tmp_net = tmp_net->next;
	    }

	  /* Delete gw entry if empty */
	  if(tmp_hna->networks.next == &tmp_hna->networks)
	    {
	      struct hna_entry *hna_to_delete = tmp_hna;
	      tmp_hna = tmp_hna->next;

	      /* Dequeue */
	      DEQUEUE_ELEM(hna_to_delete);
	      /* Delete */
	      free(hna_to_delete);
	    }
	  else
	    tmp_hna = tmp_hna->next;
	}
    }

}



/**
 *Function that times out all entrys in the hna set and
 *deletes the timed out ones.
 *
 *@return nada
 */
void
olsr_print_hna_set()
{
  int index;

  OLSR_PRINTF(1, "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- HNA SET\n\n",
              nowtm->tm_hour,
              nowtm->tm_min,
              nowtm->tm_sec,
	      (int)now.tv_usec/10000)
  
  if(olsr_cnf->ip_version == AF_INET)
    OLSR_PRINTF(1, "IP net          netmask         GW IP\n")
  else
    OLSR_PRINTF(1, "IP net/prefixlen               GW IP\n")

  for(index=0;index<HASHSIZE;index++)
    {
      struct hna_entry *tmp_hna = hna_set[index].next;
      /* Check all entrys */
      while(tmp_hna != &hna_set[index])
	{
	  /* Check all networks */
	  struct hna_net *tmp_net = tmp_hna->networks.next;

	  while(tmp_net != &tmp_hna->networks)
	    {
	      if(olsr_cnf->ip_version == AF_INET)
		{
		  OLSR_PRINTF(1, "%-15s ", olsr_ip_to_string(&tmp_net->A_network_addr))
		  OLSR_PRINTF(1, "%-15s ", olsr_ip_to_string((union olsr_ip_addr *)&tmp_net->A_netmask.v4))
		  OLSR_PRINTF(1, "%-15s\n", olsr_ip_to_string(&tmp_hna->A_gateway_addr))
		}
	      else
		{
		  OLSR_PRINTF(1, "%-27s/%d", olsr_ip_to_string(&tmp_net->A_network_addr), tmp_net->A_netmask.v6)
		  OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(&tmp_hna->A_gateway_addr))
		}

	      tmp_net = tmp_net->next;
	    }
	  tmp_hna = tmp_hna->next;
	}
    }

}


