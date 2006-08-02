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
 * $Id: process_package.c,v 1.36 2005/11/29 18:38:40 kattemat Exp $
 */


#include "defs.h"
#include "process_package.h"
#include "lq_packet.h"
#include "hysteresis.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "mpr_selector_set.h"
#include "mid_set.h"
#include "olsr.h"
#include "parser.h"
#include "duplicate_set.h"
#include "rebuild_packet.h"


/**
 *Initializing the parser functions we are using
 */
void
olsr_init_package_process()
{
  if (olsr_cnf->lq_level == 0)
    {
      olsr_parser_add_function(&olsr_process_received_hello, HELLO_MESSAGE, 1);
      olsr_parser_add_function(&olsr_process_received_tc, TC_MESSAGE, 1);
    }

  else
    {
      olsr_parser_add_function(&olsr_input_lq_hello, LQ_HELLO_MESSAGE, 1);
      olsr_parser_add_function(&olsr_input_lq_tc, LQ_TC_MESSAGE, 1);
    }

  olsr_parser_add_function(&olsr_process_received_mid, MID_MESSAGE, 1);
  olsr_parser_add_function(&olsr_process_received_hna, HNA_MESSAGE, 1);
}

void
olsr_hello_tap(struct hello_message *message, struct interface *in_if,
               union olsr_ip_addr *from_addr)
{
  struct link_entry         *link;
  struct neighbor_entry     *neighbor;
  struct hello_neighbor *walker;
  double saved_lq;
  double rel_lq;

  /*
   * Update link status
   */
  link = update_link_entry(&in_if->ip_addr, from_addr, message, in_if);

  if (olsr_cnf->lq_level > 0)
    {
      // just in case our neighbor has changed its HELLO interval

      olsr_update_packet_loss_hello_int(link, message->htime);

      // find the input interface in the list of neighbor interfaces

      for (walker = message->neighbors; walker != NULL; walker = walker->next)
        if (COMP_IP(&walker->address, &in_if->ip_addr))
          break;

      // the current reference link quality

      saved_lq = link->saved_neigh_link_quality;

      if (saved_lq == 0.0)
        saved_lq = -1.0;

      // memorize our neighbour's idea of the link quality, so that we
      // know the link quality in both directions

      if (walker != NULL)
        link->neigh_link_quality = walker->link_quality;

      else
        link->neigh_link_quality = 0.0;

      // if the link quality has changed by more than 10 percent,
      // print the new link quality table

      rel_lq = link->neigh_link_quality / saved_lq;

      if (rel_lq > 1.1 || rel_lq < 0.9)
        {
          link->saved_neigh_link_quality = link->neigh_link_quality;

          if (olsr_cnf->lq_dlimit > 0)
          {
            changes_neighborhood = OLSR_TRUE;
            changes_topology = OLSR_TRUE;
          }

          else
            OLSR_PRINTF(3, "Skipping Dijkstra (2)\n")

          // create a new ANSN

          // XXX - we should check whether we actually
          // announce this neighbour

          changes = OLSR_TRUE;
        }
    }
  
  neighbor = link->neighbor;

  /*
   * Hysteresis
   */
  if(olsr_cnf->use_hysteresis)
    {
      /* Update HELLO timeout */
      //printf("MESSAGE HTIME: %f\n", message->htime);
      olsr_update_hysteresis_hello(link, message->htime);
    }

  /* Check if we are chosen as MPR */
  if(olsr_lookup_mpr_status(message, in_if))
    /* source_addr is always the main addr of a node! */
    olsr_update_mprs_set(&message->source_addr, (float)message->vtime);



  /* Check willingness */
  if(neighbor->willingness != message->willingness)
    {
      OLSR_PRINTF(1, "Willingness for %s changed from %d to %d - UPDATING\n", 
		  olsr_ip_to_string(&neighbor->neighbor_main_addr),
		  neighbor->willingness,
		  message->willingness)
      /*
       *If willingness changed - recalculate
       */
      neighbor->willingness = message->willingness;
      changes_neighborhood = OLSR_TRUE;
      changes_topology = OLSR_TRUE;
    }


  /* Don't register neighbors of neighbors that announces WILL_NEVER */
  if(neighbor->willingness != WILL_NEVER)
    olsr_process_message_neighbors(neighbor, message);

  /* Process changes immedeatly in case of MPR updates */
  olsr_process_changes();

  olsr_free_hello_packet(message);

  return;
}

/**
 *Processes a received HELLO message. 
 *
 *@param m the incoming OLSR message
 *@return 0 on sucess
 */

void
olsr_process_received_hello(union olsr_message *m, struct interface *in_if, union olsr_ip_addr *from_addr)
{
  struct hello_message      message;

  hello_chgestruct(&message, m);

  if(!olsr_validate_address(&message.source_addr))
    {
      olsr_free_hello_packet(&message);
      return;
    }

  olsr_hello_tap(&message, in_if, from_addr);
}

void
olsr_tc_tap(struct tc_message *message, struct interface *in_if,
            union olsr_ip_addr *from_addr, union olsr_message *m)
{
  struct tc_mpr_addr              *mpr;
  struct tc_entry                 *tc_last;

  if(!olsr_check_dup_table_proc(&message->originator, 
                                message->packet_seq_number))
    {
      goto forward;
    }

  OLSR_PRINTF(3, "Processing TC from %s\n",
              olsr_ip_to_string(&message->originator))

  /*
   *      If the sender interface (NB: not originator) of this message
   *      is not in the symmetric 1-hop neighborhood of this node, the
   *      message MUST be discarded.
   */

  if(check_neighbor_link(from_addr) != SYM_LINK)
    {
      OLSR_PRINTF(2, "Received TC from NON SYM neighbor %s\n",
                  olsr_ip_to_string(from_addr))
      olsr_free_tc_packet(message);
      return;
    }

  if(olsr_cnf->debug_level > 2)
    {
      mpr = message->multipoint_relay_selector_address;
      OLSR_PRINTF(3, "mpr_selector_list:[")

      while(mpr!=NULL)
        {
          OLSR_PRINTF(3, "%s:", olsr_ip_to_string(&mpr->address))
          mpr=mpr->next;
        }

      OLSR_PRINTF(3, "]\n")
    }

  tc_last = olsr_lookup_tc_entry(&message->originator);
   
  if(tc_last != NULL)
    {
      /* Update entry */

      /* Delete destinations with lower ANSN */
      if(olsr_tc_delete_mprs(tc_last, message))
        changes_topology = OLSR_TRUE; 

      /* Update destinations */
      if(olsr_tc_update_mprs(tc_last, message))
        changes_topology = OLSR_TRUE;

      /* Delete possible empty TC entry */
      if(changes_topology)
        olsr_tc_delete_entry_if_empty(tc_last);
    }

  else
    {
      /*if message is empty then skip it */
      if(message->multipoint_relay_selector_address != NULL)
        {
          /* New entry */
          tc_last = olsr_add_tc_entry(&message->originator);      
	  
          /* Update destinations */
          olsr_tc_update_mprs(tc_last, message);
	  
          changes_topology = OLSR_TRUE;
        }
      else
        {
          OLSR_PRINTF(3, "Dropping empty TC from %s\n",
                      olsr_ip_to_string(&message->originator))
        }
    }

  /* Process changes */
  //olsr_process_changes();

 forward:

  olsr_forward_message(m, 
                       &message->originator, 
                       message->packet_seq_number, 
                       in_if,
                       from_addr);

  olsr_free_tc_packet(message);

  return;
}

/**
 *Process a received TopologyControl message
 *
 *
 *@param m the incoming OLSR message
 *@return 0 on success
 */
void
olsr_process_received_tc(union olsr_message *m, struct interface *in_if, union olsr_ip_addr *from_addr)
{ 
  struct tc_message               message;

  tc_chgestruct(&message, m, from_addr);

  if(!olsr_validate_address(&message.source_addr))
    {
      olsr_free_tc_packet(&message);
      return;
    }

  olsr_tc_tap(&message, in_if, from_addr, m);
}






/**
 *Process a received(and parsed) MID message
 *For every address check if there is a topology node
 *registered with it and update its addresses.
 *
 *@param m the OLSR message received.
 *@return 1 on success
 */

void
olsr_process_received_mid(union olsr_message *m, struct interface *in_if, union olsr_ip_addr *from_addr)
{
  struct mid_alias *tmp_adr;
  struct mid_message message;

  mid_chgestruct(&message, m);

  if(!olsr_validate_address(&message.mid_origaddr))
    {
      olsr_free_mid_packet(&message);
      return;
    }

  if(!olsr_check_dup_table_proc(&message.mid_origaddr, 
				message.mid_seqno))
    {
      goto forward;
    }

#ifdef DEBUG
  OLSR_PRINTF(5, "Processing MID from %s...\n", olsr_ip_to_string(&message.mid_origaddr))
#endif
  tmp_adr = message.mid_addr;

  /*
   *      If the sender interface (NB: not originator) of this message
   *      is not in the symmetric 1-hop neighborhood of this node, the
   *      message MUST be discarded.
   */

  if(check_neighbor_link(from_addr) != SYM_LINK)
    {
      OLSR_PRINTF(2, "Received MID from NON SYM neighbor %s\n", olsr_ip_to_string(from_addr))
      olsr_free_mid_packet(&message);
      return;
    }

  /* Update the timeout of the MID */
  olsr_update_mid_table(&message.mid_origaddr, (float)message.vtime);

  while(tmp_adr)
    {
      if(!mid_lookup_main_addr(&tmp_adr->alias_addr))
	{
	  OLSR_PRINTF(1, "MID new: (%s, ", olsr_ip_to_string(&message.mid_origaddr))
	  OLSR_PRINTF(1, "%s)\n", olsr_ip_to_string(&tmp_adr->alias_addr))
	  insert_mid_alias(&message.mid_origaddr, &tmp_adr->alias_addr, (float)message.vtime);
	}


      tmp_adr = tmp_adr->next;
    } 
  
 forward:  
  olsr_forward_message(m, 
		       &message.mid_origaddr, 
		       message.mid_seqno, 
		       in_if,
		       from_addr);
  olsr_free_mid_packet(&message);

  return;
}





/**
 *Process incoming HNA message.
 *Forwards the message if that is to be done.
 *
 *@param m the incoming OLSR message
 *the OLSR message.
 *@return 1 on success
 */

void
olsr_process_received_hna(union olsr_message *m, struct interface *in_if, union olsr_ip_addr *from_addr)
{
  struct hna_net_addr  *hna_tmp;
  struct  hna_message message;

#ifdef DEBUG
  OLSR_PRINTF(5, "Processing HNA\n")
#endif

  hna_chgestruct(&message, m);

  if(!olsr_validate_address(&message.originator))
    {
      olsr_free_hna_packet(&message);
      return;
    }

  if(!olsr_check_dup_table_proc(&message.originator, 
				message.packet_seq_number))
    {
      goto forward;
    }

  hna_tmp = message.hna_net;

  /*
   *      If the sender interface (NB: not originator) of this message
   *      is not in the symmetric 1-hop neighborhood of this node, the
   *      message MUST be discarded.
   */
  if(check_neighbor_link(from_addr) != SYM_LINK)
    {
      OLSR_PRINTF(2, "Received HNA from NON SYM neighbor %s\n", olsr_ip_to_string(from_addr))
      olsr_free_hna_packet(&message);
      return;
    }

  while(hna_tmp)
    {
      olsr_update_hna_entry(&message.originator, &hna_tmp->net, &hna_tmp->netmask, (float)message.vtime); 
      
      hna_tmp = hna_tmp->next;
    }

 forward:
  olsr_forward_message(m, 
		       &message.originator, 
		       message.packet_seq_number, 
		       in_if,
		       from_addr);
  olsr_free_hna_packet(&message);

  return;
}







/**
 *Processes an list of neighbors from an incoming HELLO message.
 *@param neighbor the neighbor who sendt the message.
 *@param message the HELLO message
 *@return nada
 */
void
olsr_process_message_neighbors(struct neighbor_entry *neighbor,
                               struct hello_message *message)
{
  struct hello_neighbor        *message_neighbors;

  for(message_neighbors = message->neighbors;
      message_neighbors != NULL;
      message_neighbors = message_neighbors->next)
    {
      union olsr_ip_addr      *neigh_addr;
      struct neighbor_2_entry *two_hop_neighbor;

      /*
       *check all interfaces
       *so that we don't add ourselves to the
       *2 hop list
       *IMPORTANT!
       */
      if(if_ifwithaddr(&message_neighbors->address) != NULL)
        continue;

      /* Get the main address */
      neigh_addr = mid_lookup_main_addr(&message_neighbors->address);

      if (neigh_addr != NULL)
        COPY_IP(&message_neighbors->address, neigh_addr);
      
      if(((message_neighbors->status == SYM_NEIGH) ||
          (message_neighbors->status == MPR_NEIGH)))
        {
	  struct neighbor_2_list_entry *two_hop_neighbor_yet =
            olsr_lookup_my_neighbors(neighbor, &message_neighbors->address);
#ifdef DEBUG
          OLSR_PRINTF(7, "\tProcessing %s\n", olsr_ip_to_string(&message_neighbors->address))
#endif
          if (two_hop_neighbor_yet != NULL)
            {
              /* Updating the holding time for this neighbor */
              two_hop_neighbor_yet->neighbor_2_timer = GET_TIMESTAMP(message->vtime*1000);
              two_hop_neighbor = two_hop_neighbor_yet->neighbor_2;
            }
          else
            {
              two_hop_neighbor =
                olsr_lookup_two_hop_neighbor_table(&message_neighbors->address);
              if (two_hop_neighbor == NULL)
                {
#ifdef DEBUG
                  OLSR_PRINTF(5, 
			      "Adding 2 hop neighbor %s\n\n", 
			      olsr_ip_to_string(&message_neighbors->address))
#endif
                  changes_neighborhood = OLSR_TRUE;
                  changes_topology = OLSR_TRUE;

                  two_hop_neighbor =
                    olsr_malloc(sizeof(struct neighbor_2_entry),
                                "Process HELLO");
		  
                  two_hop_neighbor->neighbor_2_nblist.next =
                    &two_hop_neighbor->neighbor_2_nblist;

                  two_hop_neighbor->neighbor_2_nblist.prev =
                    &two_hop_neighbor->neighbor_2_nblist;

                  two_hop_neighbor->neighbor_2_pointer = 0;
		  
                  COPY_IP(&two_hop_neighbor->neighbor_2_addr,
                          &message_neighbors->address);

                  olsr_insert_two_hop_neighbor_table(two_hop_neighbor);

                  olsr_linking_this_2_entries(neighbor, two_hop_neighbor,
                                              (float)message->vtime);
                }
              else
                {
                  /*
                    linking to this two_hop_neighbor entry
                  */	
                  changes_neighborhood = OLSR_TRUE;
                  changes_topology = OLSR_TRUE;
		  
                  olsr_linking_this_2_entries(neighbor, two_hop_neighbor,
                                              (float)message->vtime); 
                }
            }

          if (olsr_cnf->lq_level > 0)
            {
	      struct neighbor_list_entry *walker;
	      struct link_entry *link;

              link = get_best_link_to_neighbor(&neighbor->neighbor_main_addr);

	      if(!link)
		continue;

              // loop through the one-hop neighbors that see this
              // two hop neighbour

              for (walker = two_hop_neighbor->neighbor_2_nblist.next;
                   walker != &two_hop_neighbor->neighbor_2_nblist;
                   walker = walker->next)
                {
                  // have we found the one-hop neighbor that sent the
                  // HELLO message that we're current processing?

                  if (walker->neighbor == neighbor)
                    {
                      double saved_lq, rel_lq;

                      // saved previous total link quality

                      saved_lq = walker->saved_path_link_quality;

                      if (saved_lq == 0.0)
                        saved_lq = -1.0;

                      // path link quality = link quality between us
                      // and our one-hop neighbor x link quality between
                      // our one-hop neighbor and the two-hop neighbor

                      // let's compare this to ETX:

                      // 1 / LQ1 + 1 / LQ2 < 1 / LQ3 + 1 / LQ4 <=>
                      // LQ1 * LQ2 > LQ3 * LQ4

                      // so comparing path link quality values with ">" is
                      // equivalent to comparing ETX values with "<"

                      // the link quality between the 1-hop neighbour and the
                      // 2-hop neighbour

                      walker->second_hop_link_quality =
                        message_neighbors->link_quality *
                        message_neighbors->neigh_link_quality;

                      // the total quality for the route
                      // "us --- 1-hop --- 2-hop"

                      walker->path_link_quality =
                        walker->second_hop_link_quality *
                        link->loss_link_quality * link->neigh_link_quality;

                      // if the link quality has changed by more than 10
                      // percent, signal

                      rel_lq = walker->path_link_quality / saved_lq;

                      if (rel_lq > 1.1 || rel_lq < 0.9)
                        {
                          walker->saved_path_link_quality =
                            walker->path_link_quality;

                          if (olsr_cnf->lq_dlimit > 0)
                          {
                            changes_neighborhood = OLSR_TRUE;
                            changes_topology = OLSR_TRUE;
                          }

                          else
                            OLSR_PRINTF(3, "Skipping Dijkstra (3)\n")
                        }
                    }
                }
            }
        }
    }
}








/**
 *Links a one-hop neighbor with a 2-hop neighbor.
 *
 *@param neighbor the 1-hop neighbor
 *@param two_hop_neighbor the 2-hop neighbor
 *@return nada
 */
void
olsr_linking_this_2_entries(struct neighbor_entry *neighbor,struct neighbor_2_entry *two_hop_neighbor, float vtime)
{
  struct neighbor_list_entry    *list_of_1_neighbors;
  struct neighbor_2_list_entry  *list_of_2_neighbors;

  list_of_1_neighbors = olsr_malloc(sizeof(struct neighbor_list_entry), "Link entries 1");

  list_of_2_neighbors = olsr_malloc(sizeof(struct neighbor_2_list_entry), "Link entries 2");

  list_of_1_neighbors->neighbor = neighbor;

  list_of_1_neighbors->path_link_quality = 0.0;
  list_of_1_neighbors->saved_path_link_quality = 0.0;
  list_of_1_neighbors->second_hop_link_quality = 0.0;

  /* Queue */
  two_hop_neighbor->neighbor_2_nblist.next->prev = list_of_1_neighbors;
  list_of_1_neighbors->next = two_hop_neighbor->neighbor_2_nblist.next;
  two_hop_neighbor->neighbor_2_nblist.next = list_of_1_neighbors;
  list_of_1_neighbors->prev = &two_hop_neighbor->neighbor_2_nblist;


  list_of_2_neighbors->neighbor_2 = two_hop_neighbor;
  
  list_of_2_neighbors->neighbor_2_timer = GET_TIMESTAMP(vtime*1000);

  /* Queue */
  neighbor->neighbor_2_list.next->prev = list_of_2_neighbors;
  list_of_2_neighbors->next = neighbor->neighbor_2_list.next;
  neighbor->neighbor_2_list.next = list_of_2_neighbors;
  list_of_2_neighbors->prev = &neighbor->neighbor_2_list;
  
  /*increment the pointer counter*/
  two_hop_neighbor->neighbor_2_pointer++;
}






/**
 *Check if a hello message states this node as a MPR.
 *
 *@param message the message to check
 *@param n_link the buffer to put the link status in
 *@param n_status the buffer to put the status in
 *
 *@return 1 if we are selected as MPR 0 if not
 */
int
olsr_lookup_mpr_status(struct hello_message *message, struct interface *in_if)
{
  
  struct hello_neighbor  *neighbors;

  neighbors=message->neighbors;
  
  while(neighbors!=NULL)
    {  
      //printf("(linkstatus)Checking %s ",olsr_ip_to_string(&neighbors->address));
      //printf("against %s\n",olsr_ip_to_string(&main_addr));


    if(olsr_cnf->ip_version == AF_INET)
      {	
	/* IPv4 */  
	if(COMP_IP(&neighbors->address, &in_if->ip_addr))
	  {
	    //printf("ok");
	    if((neighbors->link == SYM_LINK) && (neighbors->status == MPR_NEIGH))
	      return 1;
	    
	    return 0;
	  }
      }
    else
      {	
	/* IPv6 */  
	if(COMP_IP(&neighbors->address, &in_if->int6_addr.sin6_addr))
	  {
	    //printf("ok");
	    if((neighbors->link == SYM_LINK) && (neighbors->status == MPR_NEIGH))
	      return 1;
	    
	    return 0;
	  }
      }
 
      neighbors = neighbors->next; 
    }

  /* Not found */
  return 0;
}
