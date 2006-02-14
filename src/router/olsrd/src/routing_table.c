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
 * $Id: routing_table.c,v 1.23 2005/11/16 23:55:54 tlopatic Exp $
 */



#include "defs.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "mid_set.h"
#include "neighbor_table.h"
#include "olsr.h"
#include "link_set.h"


struct rt_entry routingtable[HASHSIZE];
struct rt_entry hna_routes[HASHSIZE];


/* Begin:
 * Prototypes for internal functions 
 */

static int
olsr_fill_routing_table_with_neighbors(void);

static struct destination_n *
olsr_fill_routing_table_with_two_hop_neighbors(void);

static struct rt_entry *
olsr_check_for_higher_hopcount(struct rt_entry *, struct hna_net *, olsr_u16_t);

struct rt_entry *
olsr_check_for_lower_hopcount(struct rt_entry *, struct hna_net *, olsr_u16_t);

static olsr_bool
two_hop_neighbor_reachable(struct neighbor_2_list_entry *);

/* End:
 * Prototypes for internal functions 
 */


/**
 *Initialize the routing table
 */
int
olsr_init_routing_table()
{
  int index;

  /*
   *The hna routes hash will almost always
   *be indexed to 0
   *But it is kept as a hash to be compatible
   *with the functions used on the regular
   *routing table
   */
  for(index=0;index<HASHSIZE;index++)
    {
      routingtable[index].next = &routingtable[index];
      routingtable[index].prev = &routingtable[index];
      hna_routes[index].next = &hna_routes[index];
      hna_routes[index].prev = &hna_routes[index];
    }
  return 1;
}

/**
 *Look up an entry in the routing table.
 *
 *@param dst the address of the entry
 *
 *@return a pointer to a rt_entry struct 
 *representing the route entry.
 */
struct rt_entry *
olsr_lookup_routing_table(union olsr_ip_addr *dst)
{

  struct rt_entry *rt_table;
  olsr_u32_t      hash;

  hash = olsr_hashing(dst);

  for(rt_table = routingtable[hash].next;
      rt_table != &routingtable[hash];
      rt_table = rt_table->next)
    {
      if (COMP_IP(&rt_table->rt_dst, dst))
	{
	  return(rt_table);
	}
    }
  return(NULL);
  
}

/**
 * Look up an entry in the HNA routing table.
 *
 * @param dst the address of the entry
 *
 * @return a pointer to a rt_entry struct 
 * representing the route entry.
 */

struct rt_entry *
olsr_lookup_hna_routing_table(union olsr_ip_addr *dst)
{
  struct rt_entry *walker;
  olsr_u32_t hash = olsr_hashing(dst);

  for (walker = hna_routes[hash].next; walker != &hna_routes[hash];
       walker = walker->next)
    if (COMP_IP(&walker->rt_dst, dst))
      return walker;

  return NULL;
}

/**
 *Delete all the entries in the routing table hash
 *
 *@param table the routing hash table
 *
 *@return nada
 */
void
olsr_free_routing_table(struct rt_entry *table)
{
  olsr_u8_t       index;
  
  for(index=0;index<HASHSIZE;index++)
    {
      struct rt_entry *destination = table[index].next;

      while(destination != &table[index])
	{
	  struct rt_entry *dst_to_delete = destination;
	  destination = destination->next;

	  DEQUEUE_ELEM(dst_to_delete);
	  free(dst_to_delete);
	}
    }

}


/**
 *Insert an 1 or 2 neighbor-entry into the routing table.
 *
 *@param dst the destination
 *
 *@return the new rt_entry struct
 */
struct rt_entry *
olsr_insert_routing_table(union olsr_ip_addr *dst, 
			  union olsr_ip_addr *router, 
			  struct interface *iface, 
			  int metric,
			  float etx)
{
  struct rt_entry *new_route_entry, *rt_list;
  olsr_u32_t       hash;

  hash = olsr_hashing(dst);
  rt_list = &routingtable[hash];

  new_route_entry = olsr_malloc(sizeof(struct rt_entry), "Insert routing table");

  COPY_IP(&new_route_entry->rt_dst, dst);
  COPY_IP(&new_route_entry->rt_router, router);
  new_route_entry->rt_if = iface;

  new_route_entry->rt_metric = metric;
  new_route_entry->rt_etx = etx;
  
  if(COMP_IP(dst, router))
    /* Not GW */
    new_route_entry->rt_flags = (RTF_UP|RTF_HOST);
  else
    new_route_entry->rt_flags = (RTF_UP|RTF_HOST|RTF_GATEWAY);

  if(olsr_cnf->ip_version == AF_INET)
    /* IPv4 */
    new_route_entry->rt_mask.v4 = NETMASK_HOST;
  else
    /* IPv6 */
    new_route_entry->rt_mask.v6 = 128;

  /* queue */
  rt_list->next->prev = new_route_entry;
  new_route_entry->next = rt_list->next;
  rt_list->next = new_route_entry;
  new_route_entry->prev = rt_list;
  
  return(new_route_entry);
}



/**
 *Insert all the one hop neighbors in the routing table.
 *
 *@return
 */
static int
olsr_fill_routing_table_with_neighbors()
{
  olsr_u8_t              index;

#ifdef DEBUG
  OLSR_PRINTF(7, "FILL ROUTING TABLE WITH NEIGHBORS\n")
#endif

  for(index=0;index<HASHSIZE;index++)
    {
      struct neighbor_entry *neighbor;

      for(neighbor = neighbortable[index].next;
	  neighbor != &neighbortable[index];
	  neighbor=neighbor->next)     
	{

	  if(neighbor->status == SYM)
	    {
	      static struct mid_address addrs;
	      struct mid_address *addrs2;

	      /*
	       *Insert all the neighbors addresses
	       */

	      COPY_IP(&addrs.alias, &neighbor->neighbor_main_addr);
	      addrs.next_alias = mid_lookup_aliases(&neighbor->neighbor_main_addr);
	      addrs2 = &addrs;

	      while(addrs2!=NULL)
		{
		  struct link_entry *link = get_best_link_to_neighbor(&addrs2->alias);
#ifdef DEBUG
		  OLSR_PRINTF(7, "(ROUTE)Adding neighbor %s\n", olsr_ip_to_string(&addrs.alias))
#endif
		  if(link)
		    {
		      struct interface *iface = if_ifwithaddr(&link->local_iface_addr);
		      if(iface)
			{
			  olsr_insert_routing_table(&addrs2->alias, 
						    &link->neighbor_iface_addr,
						    iface,
						    1,
						    0);
			}
		    }
	      
		  addrs2 = addrs2->next_alias;
		}
	    }
	}
    }


  return 1;
}


/**
 * Check if a two hop neighbor is reachable trough
 * a one hop neighbor with willingness != WILL_NEVER
 *
 * @return OLSR_TRUE if reachable OLSR_FALSE if not
 */
static olsr_bool
two_hop_neighbor_reachable(struct neighbor_2_list_entry *neigh_2_list)
{
  struct neighbor_list_entry *neighbors;

  for(neighbors = neigh_2_list->neighbor_2->neighbor_2_nblist.next;
      neighbors != &neigh_2_list->neighbor_2->neighbor_2_nblist;
      neighbors = neighbors->next)
    {
      if((neighbors->neighbor->status != NOT_NEIGH) &&
	 (neighbors->neighbor->willingness != WILL_NEVER))
	return OLSR_TRUE;
    }

  return OLSR_FALSE;  
}


/**
 *Insert all the two hop neighbors that is not already added
 *in the routing table.
 *
 *@return a pointer to a destination_n linked-list of the neighbors.
 */

static struct destination_n *
olsr_fill_routing_table_with_two_hop_neighbors()
{
  struct destination_n *list_destination_n=NULL;
  olsr_u8_t            index;

  //printf("FILL ROUTING TABLE WITH TWO HOP NEIGHBORS\n");

  for(index=0;index<HASHSIZE;index++)
    {
      struct neighbor_entry *neighbor;

      for(neighbor = neighbortable[index].next;
	  neighbor != &neighbortable[index];
	  neighbor=neighbor->next)     
	{
	  struct neighbor_2_list_entry *neigh_2_list; 

	  if(neighbor->status != SYM)
	    continue;
	  
	  /*
	   *Insert all the two hop neighbors
	   */
	  for(neigh_2_list = neighbor->neighbor_2_list.next;
	      neigh_2_list != &neighbor->neighbor_2_list;
	      neigh_2_list = neigh_2_list->next)
	    {
	      union olsr_ip_addr *n2_addr;
	      static struct mid_address addrs;
	      struct mid_address *addrsp;
	      
	      n2_addr = &neigh_2_list->neighbor_2->neighbor_2_addr;
	      
	      if(olsr_lookup_routing_table(n2_addr))
		{
#ifdef DEBUG
		  OLSR_PRINTF(7, "2hop: %s already added\n", olsr_ip_to_string(n2_addr))
#endif
		  continue;
		}	    

	      if(!two_hop_neighbor_reachable(neigh_2_list))
		{
		  OLSR_PRINTF(1, "Two hop neighbor %s not added - no one hop neighbors.\n",
			      olsr_ip_to_string(n2_addr))
		  continue;
		}

	      COPY_IP(&addrs.alias, n2_addr);
	      addrs.next_alias = mid_lookup_aliases(n2_addr);
	      addrsp = &addrs;

	      while(addrsp!=NULL)
		{
		  struct link_entry *link = get_best_link_to_neighbor(&neighbor->neighbor_main_addr);
#ifdef DEBUG
		  OLSR_PRINTF(7, "(ROUTE)Adding neighbor %s\n", olsr_ip_to_string(&addrsp->alias))
#endif
		  if(link)
		    {
		      struct interface *iface = if_ifwithaddr(&link->local_iface_addr);
		      if(iface)
			{
			  struct rt_entry *new_route_entry = 
			    olsr_insert_routing_table(&addrsp->alias, 
						      &link->neighbor_iface_addr,
						      iface,
						      2,
						      0);
			  
			  if(new_route_entry != NULL)
			    {
			      struct destination_n *list_destination_tmp;
			      list_destination_tmp = olsr_malloc(sizeof(struct destination_n), 
								 "Fill rt table 2 hop tmp");
			      
			      list_destination_tmp->destination = new_route_entry;
			      list_destination_tmp->next = list_destination_n;
			      list_destination_n = list_destination_tmp;
			    }
			}
		    }
		  addrsp = addrsp->next_alias; 
		}
	    }
	}
      
    }
  
  return list_destination_n;
}




/**
 *Recalculate the routing table
 *
 *@return nada
 */
void 
olsr_calculate_routing_table()
{
  struct destination_n *list_destination_n_1 = NULL;

  olsr_move_route_table(routingtable, old_routes);

  /* Add neighbors */
  olsr_fill_routing_table_with_neighbors();
  /* Add two hop enighbors - now they are the "outer rim" */
  list_destination_n_1 = olsr_fill_routing_table_with_two_hop_neighbors();

  while(list_destination_n_1!=NULL)
    {
      /* List_destination_n_1 holds the "outer rim" */
      struct destination_n *list_destination_n = list_destination_n_1;

      list_destination_n_1=NULL;

      /* Next "outer rim" */
      while(list_destination_n!=NULL)
	{
	  struct destination_n *destination_n_1=NULL;
	  struct tc_entry      *topo_entry;

	  if((topo_entry = olsr_lookup_tc_entry(&list_destination_n->destination->rt_dst)) != NULL)
	    {
	      struct topo_dst *topo_dest = topo_entry->destinations.next;

	      /* Loop trough this nodes MPR selectors */
	      while(topo_dest != &topo_entry->destinations)
		{
		  static struct mid_address tmp_addrs;
		  struct mid_address *tmp_addrsp;
		  
		  /* Do not add ourselves */
		  if(if_ifwithaddr(&topo_dest->T_dest_addr))
		    {
		      topo_dest=topo_dest->next;
		      continue;
		    }
		  
		  /* Find mid nodes */		  
		  COPY_IP(&tmp_addrs.alias, &topo_dest->T_dest_addr);
		  tmp_addrs.next_alias = mid_lookup_aliases(&topo_dest->T_dest_addr);
		  tmp_addrsp = &tmp_addrs;
		  
		  while(tmp_addrsp!=NULL)
		    {
		      if(NULL==olsr_lookup_routing_table(&tmp_addrsp->alias))
			{
			  /* PRINT OUT: Last Hop to Final Destination */
			  /* The function ip_to_string has to be seperately */
			  OLSR_PRINTF(3, "%s -> ", olsr_ip_to_string(&list_destination_n->destination->rt_dst))
			  OLSR_PRINTF(3, "%s\n", olsr_ip_to_string(&tmp_addrsp->alias))
			  
			  destination_n_1 = olsr_malloc(sizeof(struct destination_n), 
							"Calculate routing table 2");
			  
			  /* Add this entry to the "outer rim" */
			  destination_n_1->destination = 
			    olsr_insert_routing_table(&tmp_addrsp->alias, 
						      &list_destination_n->destination->rt_router, 
						      list_destination_n->destination->rt_if,
						      list_destination_n->destination->rt_metric+1,
						      0);
			  if(destination_n_1->destination != NULL)
			    {
			      destination_n_1->next=list_destination_n_1;
			      list_destination_n_1=destination_n_1;
			    }
			}
		      tmp_addrsp = tmp_addrsp->next_alias;
		    }
		  
		  /* Next MPR selector */
		  topo_dest=topo_dest->next;
		  
		} /* End loop trought MPR selectors */
	      
	    } /* End check if already added */
	  
	  /* Delete this entry - do next */ 
	  destination_n_1 = list_destination_n;
	  list_destination_n = list_destination_n->next;
	  free(destination_n_1);
	  
	}

    }


  if(olsr_cnf->debug_level > 5)
    {
      printf("************** TABLES ****************\n");
      printf("Routing table:\n");
      olsr_print_routing_table(routingtable);
      printf("Old table:\n");
      olsr_print_routing_table(old_routes);
      printf("**************************************\n");
    }

  
  /* Update routes */
  olsr_update_kernel_routes();

  olsr_free_routing_table(old_routes);
}




/**
 *Check for a entry with a higher hopcount than
 *a given value in a routing table
 *
 *@param routes the routingtable to look in
 *@param net the network entry to look for
 *@param metric the metric to check for
 *
 *@return the localted entry if found. NULL if not
 */
static struct rt_entry *
olsr_check_for_higher_hopcount(struct rt_entry *routes, struct hna_net *net, olsr_u16_t metric)
{
  int index;

  for(index=0;index<HASHSIZE;index++)
    {
      struct rt_entry *tmp_routes;
      /* All entries */
      for(tmp_routes = routes[index].next;
	  tmp_routes != &routes[index];
	  tmp_routes = tmp_routes->next)
	{
	  if(COMP_IP(&tmp_routes->rt_dst, &net->A_network_addr) &&
	     (memcmp(&tmp_routes->rt_mask, &net->A_netmask, netmask_size) == 0))
	    {
	      /* Found a entry */
	      if(tmp_routes->rt_metric > metric)
		return tmp_routes;
	      else
		return NULL;
	    }
	}
    }

  return NULL;
}



/**
 *Check for a entry with a lower or equal hopcount than
 *a given value in a routing table
 *
 *@param routes the routingtable to look in
 *@param net the network entry to look for
 *@param metric the metric to check for
 *
 *@return the localted entry if found. NULL if not
 */
struct rt_entry *
olsr_check_for_lower_hopcount(struct rt_entry *routes, struct hna_net *net, olsr_u16_t metric)
{
  int index;

  for(index=0;index<HASHSIZE;index++)
    {
      struct rt_entry *tmp_routes;
      /* All entries */
      for(tmp_routes = routes[index].next;
	  tmp_routes != &routes[index];
	  tmp_routes = tmp_routes->next)
	{
	  if(COMP_IP(&tmp_routes->rt_dst, &net->A_network_addr) &&
	     (memcmp(&tmp_routes->rt_mask, &net->A_netmask, netmask_size) == 0))
	    {
	      /* Found a entry */
	      if(tmp_routes->rt_metric <= metric)
		return tmp_routes;
	      else
		return NULL;
	    }
	}
    }



  return NULL;
}




/**
 *Calculate the HNA routes
 *
 */
void
olsr_calculate_hna_routes()
{
  olsr_u32_t index;

#ifdef DEBUG
  OLSR_PRINTF(3, "Calculating HNA routes\n")
#endif

  olsr_move_route_table(hna_routes, old_hna);

  
  for(index=0;index<HASHSIZE;index++)
    {
      struct hna_entry *tmp_hna;
      /* All entries */
      for(tmp_hna = hna_set[index].next;
	  tmp_hna != &hna_set[index];
	  tmp_hna = tmp_hna->next)
	{
	  struct hna_net *tmp_net;
	  /* All networks */
	  for(tmp_net = tmp_hna->networks.next;
	      tmp_net != &tmp_hna->networks;
	      tmp_net = tmp_net->next)
	    {
	      struct rt_entry *tmp_rt, *new_rt;
	      //printf("HNA: checking %s -> ", olsr_ip_to_string(&tmp_hna->A_gateway_addr));
	      //printf("%s", olsr_ip_to_string(&tmp_net->A_network_addr));

	      /* If no route to gateway - skip */
	      if((tmp_rt = olsr_lookup_routing_table(&tmp_hna->A_gateway_addr)) == NULL)
		{
		  continue;
		}

	      /* If there exists a better or equal entry - skip */
	      if(olsr_check_for_lower_hopcount(hna_routes, tmp_net, tmp_rt->rt_metric) != NULL)
		{
		  continue;
		}

	      /* If we find an entry with higher hopcount we just edit it */
	      if((new_rt = olsr_check_for_higher_hopcount(hna_routes, tmp_net, tmp_rt->rt_metric)) != NULL)
		{
		  /* Fill struct */
		  /* Net */
		  COPY_IP(&new_rt->rt_dst, &tmp_net->A_network_addr);
		  new_rt->rt_mask = tmp_net->A_netmask;
		  /* Gateway */
		  COPY_IP(&new_rt->rt_router, &tmp_rt->rt_router);
		  /* Metric */
		  new_rt->rt_metric = tmp_rt->rt_metric;
		  /* Flags */
		  new_rt->rt_flags = RTF_UP | RTF_GATEWAY;
		  /* Interface */
		  new_rt->rt_if = tmp_rt->rt_if;
		}
	      /* If not - create a new one */
	      else
		{
		  olsr_u32_t  hna_hash;

		  new_rt = olsr_malloc(sizeof(struct rt_entry), "New rt entry");

		  /* Fill struct */
		  /* Net */
		  COPY_IP(&new_rt->rt_dst, &tmp_net->A_network_addr);
		  new_rt->rt_mask = tmp_net->A_netmask;
		  /* Gateway */
		  COPY_IP(&new_rt->rt_router, &tmp_rt->rt_router);
		  /* Metric */
		  new_rt->rt_metric = tmp_rt->rt_metric;
		  /* Flags */
		  new_rt->rt_flags = RTF_UP | RTF_GATEWAY;

		  /* Interface */
		  new_rt->rt_if = tmp_rt->rt_if;
	      		  
		  /* Queue HASH will almost always be 0 */
		  hna_hash = olsr_hashing(&tmp_net->A_network_addr);
		  hna_routes[hna_hash].next->prev = new_rt;
		  new_rt->next = hna_routes[hna_hash].next;
		  hna_routes[hna_hash].next = new_rt;
		  new_rt->prev = &hna_routes[hna_hash];
		}
	    }
	}
    }

  /* Update kernel */
  olsr_update_kernel_hna_routes();

  if(olsr_cnf->debug_level > 2)
    {
      OLSR_PRINTF(3, "HNA table:\n")
      olsr_print_routing_table(hna_routes);
    }

  olsr_free_routing_table(old_hna);

}





/**
 *Print the routingtable to STDOUT
 *
 */

void
olsr_print_routing_table(struct rt_entry *table)
{

  olsr_u8_t index;

  printf("ROUTING TABLE\n");
  printf("DESTINATION\tNEXT HOP\tHOPCNT\tINTERFACE\n");
  for(index=0;index<HASHSIZE;index++)
    {
      struct rt_entry *destination;
      for(destination = table[index].next;
	  destination != &table[index];
	  destination = destination->next)
	{
	  printf("%s\t", olsr_ip_to_string(&destination->rt_dst));
	  printf("%s\t%d\t%s\n", 
		 olsr_ip_to_string(&destination->rt_router),
		 destination->rt_metric,
		 destination->rt_if->int_name);
	}
    }
}





