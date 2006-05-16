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
 * $Id: process_routes.c,v 1.27 2005/05/30 13:13:38 kattemat Exp $
 */


#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "kernel_routes.h"
#include <assert.h>

#ifdef WIN32
#undef strerror
#define strerror(x) StrError(x)
#endif


struct rt_entry old_routes[HASHSIZE];
struct rt_entry old_hna[HASHSIZE];


int
olsr_init_old_table()
{
  int index;

  for(index=0;index<HASHSIZE;index++)
    {
      old_routes[index].next = &old_routes[index];
      old_routes[index].prev = &old_routes[index];
      old_hna[index].next = &old_hna[index];
      old_hna[index].prev = &old_hna[index];
    }

  return 1;
}

/**
 *Checks if there exists a route to a given host
 *in a given hash table.
 *
 *@param dst the host to check for
 *@param table the table to check
 *
 *@return 1 if the host exists in the table, 0 if not
 */
int
olsr_find_up_route(struct rt_entry *dst, struct rt_entry *table)
{ 
  struct rt_entry *destination;
  olsr_u32_t      hash;
 
  hash = olsr_hashing(&dst->rt_dst);

  for(destination = table[hash].next;
      destination != &table[hash];
      destination = destination->next)
    {
      //printf("Checking %s hc: %d ", olsr_ip_to_string(&dst->rt_dst), dst->rt_metric);
      //printf("vs %s hc: %d ... ", olsr_ip_to_string(&destination->rt_dst), destination->rt_metric);      
      if (COMP_IP(&destination->rt_dst, &dst->rt_dst) &&
	  COMP_IP(&destination->rt_router, &dst->rt_router) &&
	  (destination->rt_if->if_nr == dst->rt_if->if_nr))
	{
	  if(destination->rt_metric == dst->rt_metric)
	    {
	      return 1;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }

  return 0;

}


/**
 *Create a list containing the entries in in_table
 *that does not exist in from_table
 *
 *@param from_table the table to use
 *@param in_table the routes already added
 *
 *@return a poiter to a linked list of routes to add
 */
struct destination_n *
olsr_build_update_list(struct rt_entry *from_table,struct rt_entry *in_table)
{
  struct destination_n *kernel_route_list = NULL;
  struct rt_entry      *destination;
  olsr_u8_t            index;
  
  for(index=0;index<HASHSIZE;index++)
    {
      for(destination = from_table[index].next;
	  destination != &from_table[index];
	  destination = destination->next)
	{
	  if (!olsr_find_up_route(destination, in_table))
	    {
	      struct destination_n *route_list;
	      route_list = olsr_malloc(sizeof(struct destination_n), "create route tmp list");
	      
	      route_list->destination = destination;
	      
	      route_list->next = kernel_route_list;
	      kernel_route_list = route_list;
	    }
	}   
    }
  
  return (kernel_route_list);
}





/**
 *Deletes all OLSR routes
 *
 *
 *@return 1
 */
int
olsr_delete_all_kernel_routes()
{ 
  struct destination_n *delete_kernel_list = NULL;
  struct destination_n *tmp = NULL;
  union olsr_ip_addr *tmp_addr;

  OLSR_PRINTF(1, "Deleting all routes...\n")

  delete_kernel_list = olsr_build_update_list(hna_routes, old_hna);

  tmp = delete_kernel_list;

  OLSR_PRINTF(1, "HNA list:\n")
  while(tmp)
    {
      tmp_addr = &tmp->destination->rt_dst;
      OLSR_PRINTF(1, "Dest: %s\n", olsr_ip_to_string(tmp_addr))
      tmp = tmp->next;
    }

  olsr_delete_routes_from_kernel(delete_kernel_list);

  delete_kernel_list = olsr_build_update_list(routingtable,old_routes);

  tmp = delete_kernel_list;

  OLSR_PRINTF(1, "Route list:\n")
  while(tmp)
    {
      tmp_addr = &tmp->destination->rt_dst;
      OLSR_PRINTF(1, "Dest: %s\n", olsr_ip_to_string(tmp_addr))
      tmp = tmp->next;
    }

  olsr_delete_routes_from_kernel(delete_kernel_list);

  return 1;
}


/**
 *Perform all neccessary actions for an update of the 
 *routes in the kernel.
 *
 *@return nada
 */
void
olsr_update_kernel_routes()
{
  struct destination_n *delete_kernel_list = NULL;
  struct destination_n *add_kernel_list = NULL;

  OLSR_PRINTF(3, "Updating kernel routes...\n")
  delete_kernel_list = olsr_build_update_list(old_routes, routingtable);
  add_kernel_list = olsr_build_update_list(routingtable, old_routes);

  olsr_delete_routes_from_kernel(delete_kernel_list);
  olsr_add_routes_in_kernel(add_kernel_list);
}



/**
 *Perform all neccessary actions for an update of the 
 *HNA routes in the kernel.
 *
 *@return nada
 */
void
olsr_update_kernel_hna_routes()
{
  struct destination_n *delete_kernel_list = NULL;
  //struct destination_n *delete_kernel_list2;
  struct destination_n *add_kernel_list = NULL;

  OLSR_PRINTF(3, "Updating kernel HNA routes...\n")


  delete_kernel_list = olsr_build_update_list(old_hna, hna_routes);
  add_kernel_list = olsr_build_update_list(hna_routes, old_hna);

  olsr_delete_routes_from_kernel(delete_kernel_list);
  olsr_add_routes_in_kernel(add_kernel_list);
}


/**
 *Create a copy of the routing table and
 *clear the current table
 *
 *@param original the table to move from
 *@param the table to move to
 *
 *@return nada
 */
void
olsr_move_route_table(struct rt_entry *original, struct rt_entry *new)
{
  olsr_16_t index;

  for(index=0;index<HASHSIZE;index++)
    {
      if(original[index].next == &original[index])
	{
	  new[index].next = &new[index];
	  new[index].prev = &new[index];
	}
      else
	{
	  /* Copy to old */
	  new[index].next = original[index].next;
	  new[index].next->prev = &new[index];
	  new[index].prev = original[index].prev;
	  new[index].prev->next = &new[index];

	  /* Clear original */
	  original[index].next = &original[index];
	  original[index].prev = &original[index];
	}
    }
}


/**
 *Delete a linked list of routes from the kernel.
 *
 *@param delete_kernel_list the list to delete
 *
 *@return nada
 */
void 
olsr_delete_routes_from_kernel(struct destination_n *delete_kernel_list)
{
  struct destination_n *destination_ptr;
  int metric_counter = 1;
  olsr_bool last_run = OLSR_FALSE;


  /* Find highest metric */
  for(destination_ptr = delete_kernel_list;
      destination_ptr != NULL;
      destination_ptr = destination_ptr->next)
    {
      if(destination_ptr->destination->rt_metric > metric_counter)
	metric_counter = destination_ptr->destination->rt_metric;
    }

#ifdef DEBUG
  OLSR_PRINTF(3, "%s highest metric %d\n",
	      __func__, metric_counter)
#endif
 
  while(delete_kernel_list!=NULL)
    {
      struct destination_n *previous_node = delete_kernel_list;

      assert(metric_counter);

      /* searching for all the items with metric equal to n */
      for(destination_ptr = delete_kernel_list; destination_ptr != NULL; )
	{

	  if(destination_ptr->destination->rt_metric == metric_counter)
	    {
	      /* Make sure one-hop direct neighbors are deleted last */
	      if(metric_counter == 1 &&
		 (!last_run && 
		  COMP_IP(&destination_ptr->destination->rt_dst, 
			  &destination_ptr->destination->rt_router)))
		{
		  previous_node = destination_ptr;
		  destination_ptr = destination_ptr->next;
		}
	      else
		{
		  olsr_16_t error;		  
#ifdef DEBUG
		  OLSR_PRINTF(3, "Deleting route to %s hopcount %d\n",
			      olsr_ip_to_string(&destination_ptr->destination->rt_dst),
			      destination_ptr->destination->rt_metric)
#endif
		    if(!olsr_cnf->host_emul)
		      {
			if(olsr_cnf->ip_version == AF_INET)
			  error = olsr_ioctl_del_route(destination_ptr->destination);
			else
			  error = olsr_ioctl_del_route6(destination_ptr->destination);
			
			if(error < 0)
			  {
			    OLSR_PRINTF(1, "Delete route(%s):%s\n", olsr_ip_to_string(&destination_ptr->destination->rt_dst), strerror(errno))
			      olsr_syslog(OLSR_LOG_ERR, "Delete route:%m");
			  }
		    }
		  
		  /* Getting rid of this node and hooking up the broken point */
		  if(destination_ptr == delete_kernel_list) 
		    {
		      destination_ptr = delete_kernel_list->next;
		      free(delete_kernel_list);
		      delete_kernel_list = destination_ptr;
		      previous_node = delete_kernel_list;
		    }
		  else 
		    {
		      previous_node->next = destination_ptr->next;
		      free(destination_ptr);
		      destination_ptr = previous_node->next;
		    }
		}
	    }
	  else 
	    {
	      previous_node = destination_ptr;
	      destination_ptr = destination_ptr->next;
	    }
		
	}
      if((metric_counter == 1) && !last_run)
        {
	  last_run = OLSR_TRUE;
        }
      else
        {
	  metric_counter--;
        }
    }
 
}

/**
 *Add a list of routes to the kernel. Adding
 *is done by hopcount to be sure a route
 *to the nexthop is added.
 *
 *@param add_kernel_list the linked list of routes to add
 *
 *@return nada
 */
void 
olsr_add_routes_in_kernel(struct destination_n *add_kernel_list)
{
  int metric_counter = 1;
  olsr_bool first_run = OLSR_TRUE;
  
  while(add_kernel_list != NULL)
    {
      struct destination_n *destination_kernel = NULL;
      struct destination_n *previous_node = add_kernel_list;

      assert(metric_counter < HOPCNT_MAX);

      /* searching for all the items with metric equal to n */
      for(destination_kernel = add_kernel_list; destination_kernel != NULL; )
	{
	  if((destination_kernel->destination->rt_metric == metric_counter) &&
	     (!first_run || 
              COMP_IP(&destination_kernel->destination->rt_dst,
                      &destination_kernel->destination->rt_router)))
	    {
	      olsr_16_t error;
	      /* First add all 1-hop routes that has themselves as GW */

#ifdef DEBUG
	      OLSR_PRINTF(3, "Adding route to %s hopcount %d\n",
			  olsr_ip_to_string(&destination_kernel->destination->rt_dst),
			  destination_kernel->destination->rt_metric)
#endif
			  
		if(!olsr_cnf->host_emul)
		  {
		    if(olsr_cnf->ip_version == AF_INET)
		      error=olsr_ioctl_add_route(destination_kernel->destination);
		    else
		      error=olsr_ioctl_add_route6(destination_kernel->destination);
		    
		    if(error < 0)
		      {
			OLSR_PRINTF(1, "Add route(%s): %s\n", olsr_ip_to_string(&destination_kernel->destination->rt_dst), strerror(errno))
			  olsr_syslog(OLSR_LOG_ERR, "Add route:%m");
		      }
		  }

	      
	      /* Getting rid of this node and hooking up the broken point */
	      if(destination_kernel == add_kernel_list) 
		{
		  destination_kernel = add_kernel_list->next;
		  free(add_kernel_list);
		  add_kernel_list = destination_kernel;
		  previous_node=add_kernel_list;
		}
	      else 
		{
		  previous_node->next = destination_kernel->next;
		  free(destination_kernel);
		  destination_kernel = previous_node->next;
		}
	    }
	  else 
	    {
	      previous_node = destination_kernel;
	      destination_kernel = destination_kernel->next;
	    }
		
	}
      if(first_run)
        {
	  first_run = OLSR_FALSE;
        }
      else
        {
	  metric_counter++;
        }
    }
	
}



