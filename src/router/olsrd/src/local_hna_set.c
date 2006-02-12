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
 * $Id: local_hna_set.c,v 1.10 2005/02/27 18:39:43 kattemat Exp $
 */

#include "defs.h"
#include "local_hna_set.h"
#include "olsr.h"

void
add_local_hna4_entry(union olsr_ip_addr *net, union olsr_ip_addr *mask)
{
  struct hna4_entry *new_entry;

  new_entry = olsr_malloc(sizeof(struct hna4_entry), "Add local HNA entry 4");
  
  new_entry->net.v4 = net->v4;
  new_entry->netmask.v4 = mask->v4;

  /* Queue */
  new_entry->next = olsr_cnf->hna4_entries;
  olsr_cnf->hna4_entries = new_entry;
}


void
add_local_hna6_entry(union olsr_ip_addr *net, olsr_u16_t prefix_len)
{
  struct hna6_entry *new_entry;

  new_entry = olsr_malloc(sizeof(struct hna6_entry), "Add local HNA entry 6");
  
  memcpy(&new_entry->net, net, sizeof(struct in6_addr));
  prefix_len = prefix_len;

  /* Queue */
  new_entry->next = olsr_cnf->hna6_entries;
  olsr_cnf->hna6_entries = new_entry;
}


int
remove_local_hna4_entry(union olsr_ip_addr *net, union olsr_ip_addr *mask)
{
  struct hna4_entry *h4 = olsr_cnf->hna4_entries, *h4prev = NULL;

  while(h4)
    {
      if((net->v4 == h4->net.v4) && 
	 (mask->v4 == h4->netmask.v4))
	{
	  /* Dequeue */
	  if(h4prev == NULL)
	    olsr_cnf->hna4_entries = h4->next;
	  else
	    h4prev->next = h4->next;

	  free(h4);
	  return 1;
	}
      h4prev = h4;
      h4 = h4->next;
    }

  return 0;
}



int
remove_local_hna6_entry(union olsr_ip_addr *net, olsr_u16_t prefix_len)
{
  struct hna6_entry *h6 = olsr_cnf->hna6_entries, *h6prev = NULL;

  while(h6)
    {
      if((memcmp(net, &h6->net, ipsize) == 0) && 
	 (prefix_len == h6->prefix_len))
	{
	  /* Dequeue */
	  if(h6prev == NULL)
	    olsr_cnf->hna6_entries = h6->next;
	  else
	    h6prev->next = h6->next;

	  free(h6);
	  return 1;
	}
      h6prev = h6;
      h6 = h6->next;
    }

  return 0;
}



int
check_inet_gw()
{
  struct hna4_entry *h4 = olsr_cnf->hna4_entries;

  if(olsr_cnf->ip_version == AF_INET)
    {
      while(h4)
	{
	  if(h4->netmask.v4 == 0 && h4->net.v4 == 0)
	    return 1;
	  h4 = h4->next;
	}
      return 0;
    }
  return 0;

}
