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

#include "ipcalc.h"
#include "defs.h"
#include "link_set.h"
#include "mpr_selector_set.h"
#include "mpr.h"
#include "olsr.h"
#include "neighbor_table.h"
#include "build_msg.h"
#include "net_olsr.h"
#include "lq_plugin.h"

static bool sending_tc = false;

/**
 *Free the memory allocated for a HELLO packet.
 *
 *@param message the pointer to the packet to erase
 *
 *@return nada
 */
void
olsr_free_hello_packet(struct hello_message *message)
{
  struct hello_neighbor *nb;

  if (!message)
    return;

  nb = message->neighbors;
  while (nb) {
    struct hello_neighbor *prev_nb = nb;
    nb = nb->next;
    free(prev_nb);
  }
}

/**
 *Build an internal HELLO package for this
 *node. This MUST be done for each interface.
 *
 *@param message the hello_message struct to fill with info
 *@param outif the interface to send the message on - messages
 *are created individually for each interface!
 *@return 0
 */
int
olsr_build_hello_packet(struct hello_message *message, struct interface_olsr *outif)
{
  struct hello_neighbor *message_neighbor, *tmp_neigh;
  struct link_entry *links;
  struct neighbor_entry *neighbor;

#ifdef DEBUG
  OLSR_PRINTF(3, "\tBuilding HELLO on interface \"%s\"\n", outif->int_name ? outif->int_name : "<null>");
#endif /* DEBUG */

  message->neighbors = NULL;
  message->packet_seq_number = 0;

  //message->mpr_seq_number=neighbortable.neighbor_mpr_seq;

  /* Set willingness */

  message->willingness = olsr_cnf->willingness;
#ifdef DEBUG
  OLSR_PRINTF(3, "Willingness: %d\n", olsr_cnf->willingness);
#endif /* DEBUG */

  /* Set TTL */

  message->ttl = 1;
  message->source_addr = olsr_cnf->main_addr;

#ifdef DEBUG
  OLSR_PRINTF(5, "On link:\n");
#endif /* DEBUG */

  /* Walk all links of this interface */
  OLSR_FOR_ALL_LINK_ENTRIES(links) {
#ifdef DEBUG
    struct ipaddr_str buf;
#endif /* DEBUG */
    int lnk = lookup_link_status(links);
    /* Update the status */

    /* Check if this link tuple is registered on the outgoing interface */
    if (!ipequal(&links->local_iface_addr, &outif->ip_addr)) {
      continue;
    }

    message_neighbor = olsr_malloc_hello_neighbor("Build HELLO");

    /* Find the link status */
    message_neighbor->link = lnk;

    /*
     * Calculate neighbor status
     */
    /*
     * 2.1  If the main address, corresponding to
     *      L_neighbor_iface_addr, is included in the MPR set:
     *
     *            Neighbor Type = MPR_NEIGH
     */
    if (links->neighbor->is_mpr) {
      message_neighbor->status = MPR_NEIGH;
    }
    /*
     *  2.2  Otherwise, if the main address, corresponding to
     *       L_neighbor_iface_addr, is included in the neighbor set:
     */

    /* NOTE:
     * It is garanteed to be included when come this far
     * due to the extentions made in the link sensing
     * regarding main addresses.
     */
    else {

      /*
       *   2.2.1
       *        if N_status == SYM
       *
       *             Neighbor Type = SYM_NEIGH
       */
      if (links->neighbor->status == SYM) {
        message_neighbor->status = SYM_NEIGH;
      }

      /*
       *   2.2.2
       *        Otherwise, if N_status == NOT_SYM
       *             Neighbor Type = NOT_NEIGH
       */
      else if (links->neighbor->status == NOT_SYM) {
        message_neighbor->status = NOT_NEIGH;
      }
    }

    /* Set the remote interface address */
    message_neighbor->address = links->neighbor_iface_addr;

    /* Set the main address */
    message_neighbor->main_address = links->neighbor->neighbor_main_addr;
#ifdef DEBUG
    OLSR_PRINTF(5, "Added: %s -  status %d\n", olsr_ip_to_string(&buf, &message_neighbor->address), message_neighbor->status);
#endif /* DEBUG */
    message_neighbor->next = message->neighbors;
    message->neighbors = message_neighbor;

  }
  OLSR_FOR_ALL_LINK_ENTRIES_END(links);

  /* Add the links */

#ifdef DEBUG
  OLSR_PRINTF(5, "Not on link:\n");
#endif /* DEBUG */

  /* Add the rest of the neighbors if running on multiple interfaces */

  if (ifnet != NULL && ifnet->int_next != NULL)
    OLSR_FOR_ALL_NBR_ENTRIES(neighbor) {

#ifdef DEBUG
    struct ipaddr_str buf;
#endif /* DEBUG */
    /* Check that the neighbor is not added yet */
    tmp_neigh = message->neighbors;
    //printf("Checking that the neighbor is not yet added\n");
    while (tmp_neigh) {
      if (ipequal(&tmp_neigh->main_address, &neighbor->neighbor_main_addr)) {
        //printf("Not adding duplicate neighbor %s\n", olsr_ip_to_string(&neighbor->neighbor_main_addr));
        break;
      }
      tmp_neigh = tmp_neigh->next;
    }

    if (tmp_neigh) {
      continue;
    }

    message_neighbor = olsr_malloc_hello_neighbor("Build HELLO 2");

    message_neighbor->link = UNSPEC_LINK;

    /*
     * Calculate neighbor status
     */
    /*
     * 2.1  If the main address, corresponding to
     *      L_neighbor_iface_addr, is included in the MPR set:
     *
     *            Neighbor Type = MPR_NEIGH
     */
    if (neighbor->is_mpr) {
      message_neighbor->status = MPR_NEIGH;
    }
    /*
     *  2.2  Otherwise, if the main address, corresponding to
     *       L_neighbor_iface_addr, is included in the neighbor set:
     */

    /* NOTE:
     * It is garanteed to be included when come this far
     * due to the extentions made in the link sensing
     * regarding main addresses.
     */
    else {

      /*
       *   2.2.1
       *        if N_status == SYM
       *
       *             Neighbor Type = SYM_NEIGH
       */
      if (neighbor->status == SYM) {
        message_neighbor->status = SYM_NEIGH;
      }

      /*
       *   2.2.2
       *        Otherwise, if N_status == NOT_SYM
       *             Neighbor Type = NOT_NEIGH
       */
      else if (neighbor->status == NOT_SYM) {
        message_neighbor->status = NOT_NEIGH;
      }
    }

    message_neighbor->address = neighbor->neighbor_main_addr;
    message_neighbor->main_address = neighbor->neighbor_main_addr;
#ifdef DEBUG
    OLSR_PRINTF(5, "Added: %s -  status  %d\n", olsr_ip_to_string(&buf, &message_neighbor->address), message_neighbor->status);
#endif /* DEBUG */
    message_neighbor->next = message->neighbors;
    message->neighbors = message_neighbor;

    }
  OLSR_FOR_ALL_NBR_ENTRIES_END(neighbor);

  return 0;
}

/**
 *Free the memory allocated for a TC packet.
 *
 *@param message the pointer to the packet to erase
 *
 *@return nada
 */
void
olsr_free_tc_packet(struct tc_message *message)
{
  struct tc_mpr_addr *mprs;

  if (!message)
    return;

  mprs = message->multipoint_relay_selector_address;
  while (mprs != NULL) {
    struct tc_mpr_addr *prev_mprs = mprs;
    mprs = mprs->next;
    free(prev_mprs);
  }
}

/**
 *Build an internal TC package for this
 *node.
 *
 *@param message the tc_message struct to fill with info
 *@return 0
 */
int
olsr_build_tc_packet(struct tc_message *message)
{
  struct tc_mpr_addr *message_mpr;
  struct neighbor_entry *entry;
  bool entry_added = false;

  message->multipoint_relay_selector_address = NULL;
  message->packet_seq_number = 0;

  message->hop_count = 0;
  message->ttl = MAX_TTL;
  message->ansn = get_local_ansn();

  message->originator = olsr_cnf->main_addr;
  message->source_addr = olsr_cnf->main_addr;

  /* Loop trough all neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(entry) {
    if (entry->status != SYM) {
      continue;
    }

    switch (olsr_cnf->tc_redundancy) {
    case (2):
      {
        /* 2 = Add all neighbors */
        //printf("\t%s\n", olsr_ip_to_string(&mprs->mpr_selector_addr));
        message_mpr = olsr_malloc_tc_mpr_addr("Build TC");

        message_mpr->address = entry->neighbor_main_addr;
        message_mpr->next = message->multipoint_relay_selector_address;
        message->multipoint_relay_selector_address = message_mpr;
        entry_added = true;
        break;
      }
    case (1):
      {
        /* 1 = Add all MPR selectors and selected MPRs */
        if ((entry->is_mpr) || (olsr_lookup_mprs_set(&entry->neighbor_main_addr) != NULL)) {
          //printf("\t%s\n", olsr_ip_to_string(&mprs->mpr_selector_addr));
          message_mpr = olsr_malloc_tc_mpr_addr("Build TC 2");

          message_mpr->address = entry->neighbor_main_addr;
          message_mpr->next = message->multipoint_relay_selector_address;
          message->multipoint_relay_selector_address = message_mpr;
          entry_added = true;
        }
        break;
      }
    default:
      {
        /* 0 = Add only MPR selectors(default) */
        if (olsr_lookup_mprs_set(&entry->neighbor_main_addr) != NULL) {
          //printf("\t%s\n", olsr_ip_to_string(&mprs->mpr_selector_addr));
          message_mpr = olsr_malloc_tc_mpr_addr("Build TC 3");

          message_mpr->address = entry->neighbor_main_addr;
          message_mpr->next = message->multipoint_relay_selector_address;
          message->multipoint_relay_selector_address = message_mpr;
          entry_added = true;
        }
        break;
      }

    }                           /* Switch */
  }
  OLSR_FOR_ALL_NBR_ENTRIES_END(entry);

  if (entry_added) {
    sending_tc = true;
  } else {
    if (sending_tc) {
      /* Send empty TC */
      OLSR_PRINTF(3, "No more MPR selectors - will send empty TCs\n");
      set_empty_tc_timer(GET_TIMESTAMP((olsr_cnf->max_tc_vtime * 3) * MSEC_PER_SEC));
      sending_tc = false;
    }
  }

  return 0;
}

/**
 *Free the memory allocated for a MID packet.
 *
 *@param message the pointer to the packet to erase
 *
 *@return nada
 */

void
olsr_free_mid_packet(struct mid_message *message)
{
  struct mid_alias *tmp_adr, *tmp_adr2;

  tmp_adr = message->mid_addr;

  while (tmp_adr) {
    tmp_adr2 = tmp_adr;
    tmp_adr = tmp_adr->next;
    free(tmp_adr2);
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
