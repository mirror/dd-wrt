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
 * $Id: rebuild_packet.c,v 1.19 2005/05/28 16:01:14 kattemat Exp $
 */


#include "rebuild_packet.h"
#include "defs.h"
#include "olsr.h"
#include "mid_set.h"
#include "mantissa.h"

/**
 *Process/rebuild HNA message. Converts the OLSR
 *packet to the internal hna_message format.
 *@param hmsg the hna_message struct in wich infomation
 *is to be put.
 *@param m the entire OLSR message revieved.
 *@return negative on error
 */

void
hna_chgestruct(struct hna_message *hmsg, union olsr_message *m)
{
  struct hna_net_addr *hna_pairs, *tmp_pairs;
  int no_pairs, i;

  /*Check if everyting is ok*/
  if ((!m) || (m->v4.olsr_msgtype != HNA_MESSAGE))
    return;
  

  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IPv4 */
      struct hnapair *haddr;

      haddr = m->v4.message.hna.hna_net;

      /*
       * How many HNA pairs?
       * nextmsg contains size of
       * the addresses + 12 bytes(nextmessage, from address and the header)
       */
      no_pairs = (ntohs(m->v4.olsr_msgsize) - 12) / 8;
      
      COPY_IP(&hmsg->originator, &m->v4.originator);
      hmsg->packet_seq_number = ntohs(m->v4.seqno);
      hmsg->hop_count =  m->v4.hopcnt;

      //printf("HNA from %s\n\n", olsr_ip_to_string((union olsr_ip_addr *)&hmsg->originator));

      /* Get vtime */
      hmsg->vtime = ME_TO_DOUBLE(m->v4.olsr_vtime);

      tmp_pairs = NULL;
      hna_pairs = NULL;

      for(i = 0; i < no_pairs; i++)
	{
	  
	  hna_pairs = olsr_malloc(sizeof(struct hna_net_addr), "HNA chgestruct");
	  
	  
	  COPY_IP(&hna_pairs->net, &haddr->addr);
	  COPY_IP(&hna_pairs->netmask, &haddr->netmask);
	  hna_pairs->next = tmp_pairs;
	  
	  tmp_pairs = hna_pairs;
	  haddr++;
      
	}

    }
  else
    {
      /* IPv6 */
      struct hnapair6 *haddr6;

      haddr6 = m->v6.message.hna.hna_net;

      /*
       * How many HNA pairs?
       * nextmsg contains size of
       * the addresses + 12 bytes(nextmessage, from address and the header)
       */
      no_pairs = (ntohs(m->v6.olsr_msgsize) - 24) / 32; /* NB 32 not 8 */
      
      COPY_IP(&hmsg->originator, &m->v6.originator);
      hmsg->packet_seq_number = ntohs(m->v6.seqno);
      hmsg->hop_count =  m->v6.hopcnt;
      
      /* Get vtime */
      hmsg->vtime = ME_TO_DOUBLE(m->v6.olsr_vtime);
      
      tmp_pairs = NULL;
      hna_pairs = NULL;
      
      for(i = 0; i < no_pairs; i++)
	{
	  
	  hna_pairs = olsr_malloc(sizeof(struct hna_net_addr), "HNA chgestruct 2");	  
	  
	  COPY_IP(&hna_pairs->net, &haddr6->addr);
	  hna_pairs->netmask.v6 = olsr_netmask_to_prefix((union olsr_ip_addr *)&haddr6->netmask);

	  hna_pairs->next = tmp_pairs;
	  
	  tmp_pairs = hna_pairs;
	  haddr6++;
	  
	}

      
    }      

  /* 
     tmp_pairs = hna_pairs;
	 
     while(tmp_pairs)
     {
     printf("\t net: %s ", ip_to_string(&tmp_pairs->net));
     printf("\t mask: %s\n", ip_to_string(&tmp_pairs->netmask));
     tmp_pairs = tmp_pairs->next;
     }
     printf("\n");
  */



  hmsg->hna_net = hna_pairs;
 
}


/**
 *Process/rebuild MID message. Converts the OLSR
 *packet to the internal mid_message format.
 *@param mmsg the mid_message struct in wich infomation
 *is to be put.
 *@param m the entire OLSR message revieved.
 *@return negative on error
 */

void
mid_chgestruct(struct mid_message *mmsg, union olsr_message *m)
{
  int i;
  struct mid_alias *alias, *alias_tmp;
  int no_aliases;

  /* Checking if everything is ok */
  if ((!m) || (m->v4.olsr_msgtype != MID_MESSAGE))
    return;

  alias = NULL;

  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IPv4 */
      struct midaddr *maddr;

      maddr = m->v4.message.mid.mid_addr;
      /*
       * How many aliases?
       * nextmsg contains size of
       * the addresses + 12 bytes(nextmessage, from address and the header)
       */
      no_aliases =  ((ntohs(m->v4.olsr_msgsize) - 12) / 4);

      //printf("Aliases: %d\n", no_aliases);
      COPY_IP(&mmsg->mid_origaddr, &m->v4.originator);
      COPY_IP(&mmsg->addr, &m->v4.originator);
      /*seq number*/
      mmsg->mid_seqno = ntohs(m->v4.seqno);
      mmsg->mid_addr = NULL;

      /* Get vtime */
      mmsg->vtime = ME_TO_DOUBLE(m->v4.olsr_vtime);

      //printf("Sequencenuber of MID from %s is %d\n", ip_to_string(&mmsg->addr), mmsg->mid_seqno);


      for(i = 0; i < no_aliases; i++)
	{
	  alias = olsr_malloc(sizeof(struct mid_alias), "MID chgestruct");
	  
	  COPY_IP(&alias->alias_addr, &maddr->addr);
	  alias->next = mmsg->mid_addr;
	  mmsg->mid_addr = alias;
	  maddr++;
	}
      
      
      if(olsr_cnf->debug_level > 1)
	{
	  OLSR_PRINTF(3, "Alias list for %s: ", ip_to_string(&mmsg->mid_origaddr.v4))
	  OLSR_PRINTF(3, "%s", ip_to_string(&mmsg->addr.v4))
	  alias_tmp = mmsg->mid_addr;
	  while(alias_tmp)
	    {
	      OLSR_PRINTF(3, " - %s", ip_to_string(&alias_tmp->alias_addr.v4))
	      alias_tmp = alias_tmp->next;
	    }
	  OLSR_PRINTF(3, "\n")
	}
    }
  else
    {
      /* IPv6 */
      struct midaddr6 *maddr6;

      maddr6 = m->v6.message.mid.mid_addr;
      /*
       * How many aliases?
       * nextmsg contains size of
       * the addresses + 12 bytes(nextmessage, from address and the header)
       */
      no_aliases =  ((ntohs(m->v6.olsr_msgsize) - 12) / 16); /* NB 16 */

      //printf("Aliases: %d\n", no_aliases);
      COPY_IP(&mmsg->mid_origaddr, &m->v6.originator);
      COPY_IP(&mmsg->addr, &m->v6.originator);
      /*seq number*/
      mmsg->mid_seqno = ntohs(m->v6.seqno);
      mmsg->mid_addr = NULL;

      /* Get vtime */
      mmsg->vtime = ME_TO_DOUBLE(m->v6.olsr_vtime);

      //printf("Sequencenuber of MID from %s is %d\n", ip_to_string(&mmsg->addr), mmsg->mid_seqno);


      for(i = 0; i < no_aliases; i++)
	{
	  alias = olsr_malloc(sizeof(struct mid_alias), "MID chgestruct 2");
	  
	  //printf("Adding alias: %s\n", olsr_ip_to_string((union olsr_ip_addr *)&maddr6->addr));
	  COPY_IP(&alias->alias_addr, &maddr6->addr);
	  alias->next = mmsg->mid_addr;
	  mmsg->mid_addr = alias;
	   
	  maddr6++;
	}


      if(olsr_cnf->debug_level > 1)
	{
	  OLSR_PRINTF(3, "Alias list for %s", ip6_to_string(&mmsg->mid_origaddr.v6))
	  OLSR_PRINTF(3, "%s", ip6_to_string(&mmsg->addr.v6))

	  alias_tmp = mmsg->mid_addr;
	  while(alias_tmp)
	    {
	      OLSR_PRINTF(3, " - %s", ip6_to_string(&alias_tmp->alias_addr.v6))
	      alias_tmp = alias_tmp->next;
	    }
	  OLSR_PRINTF(3, "\n")
	}
    }

}




/**
 *Process/rebuild a message of unknown type. Converts the OLSR
 *packet to the internal unknown_message format.
 *@param umsg the unknown_message struct in wich infomation
 *is to be put.
 *@param m the entire OLSR message revieved.
 *@return negative on error
 */

void
unk_chgestruct(struct unknown_message *umsg, union olsr_message *m)
{

  /* Checking if everything is ok */
  if (!m)
    return;


  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IPv4 */
      /* address */
      COPY_IP(&umsg->originator, &m->v4.originator);
      /*seq number*/
      umsg->seqno = ntohs(m->v4.seqno);
      /* type */
      umsg->type = m->v4.olsr_msgtype;
    }
  else
    {
      /* IPv6 */
      /* address */
      COPY_IP(&umsg->originator, &m->v6.originator);
      /*seq number*/
      umsg->seqno = ntohs(m->v6.seqno);
      /* type */
      umsg->type = m->v4.olsr_msgtype;
    }
  
}



/**
 *Process/rebuild HELLO message. Converts the OLSR
 *packet to the internal hello_message format.
 *@param hmsg the hello_message struct in wich infomation
 *is to be put.
 *@param m the entire OLSR message revieved.
 *@return negative on error
 */

void
hello_chgestruct(struct hello_message *hmsg, union olsr_message *m)
{
  union olsr_ip_addr *hadr;
  struct hello_neighbor *nb;
  
  hmsg->neighbors = NULL;

  if ((!m) || (m->v4.olsr_msgtype != HELLO_MESSAGE))
    return;

  if(olsr_cnf->ip_version == AF_INET)
    {
      struct hellinfo *hinf;

      /* IPv4 */
      COPY_IP(&hmsg->source_addr, &m->v4.originator);
      hmsg->packet_seq_number = ntohs(m->v4.seqno);


      /* Get vtime */
      hmsg->vtime = ME_TO_DOUBLE(m->v4.olsr_vtime);

      /* Get htime */
      hmsg->htime = ME_TO_DOUBLE(m->v4.message.hello.htime);

      /* Willingness */
      hmsg->willingness = m->v4.message.hello.willingness;

      OLSR_PRINTF(3, "Got HELLO vtime: %f htime: %f\n", hmsg->vtime, hmsg->htime)

      for (hinf = m->v4.message.hello.hell_info; 
	   (char *)hinf < ((char *)m + (ntohs(m->v4.olsr_msgsize))); 
	   hinf = (struct hellinfo *)((char *)hinf + ntohs(hinf->size)))
	{
	  
	  for (hadr = (union olsr_ip_addr  *)&hinf->neigh_addr; 
	       (char *)hadr < (char *)hinf + ntohs(hinf->size); 
	       hadr = (union olsr_ip_addr *)&hadr->v6.s6_addr[4])
	    {
	      nb = olsr_malloc(sizeof (struct hello_neighbor), "HELLO chgestruct");

	      COPY_IP(&nb->address, hadr);

	      /* Fetch link and status */
	      nb->link = EXTRACT_LINK(hinf->link_code);
	      nb->status = EXTRACT_STATUS(hinf->link_code);

	      nb->next = hmsg->neighbors;
	      hmsg->neighbors = nb;
	    }
	}

      
    }
  else
    {
      struct hellinfo6 *hinf6;

      /* IPv6 */
      COPY_IP(&hmsg->source_addr, &m->v6.originator);
      //printf("parsing HELLO from %s\n", olsr_ip_to_string(&hmsg->source_addr));
      hmsg->packet_seq_number = ntohs(m->v6.seqno);

      /* Get vtime */
      hmsg->vtime = ME_TO_DOUBLE(m->v6.olsr_vtime);

      /* Get htime */
      hmsg->htime = ME_TO_DOUBLE(m->v6.message.hello.htime);

      /* Willingness */
      hmsg->willingness = m->v6.message.hello.willingness;

      OLSR_PRINTF(3, "Got HELLO vtime: %f htime: %f\n", hmsg->vtime, hmsg->htime)


      for (hinf6 = m->v6.message.hello.hell_info; 
	   (char *)hinf6 < ((char *)m + (ntohs(m->v6.olsr_msgsize))); 
	   hinf6 = (struct hellinfo6 *)((char *)hinf6 + ntohs(hinf6->size)))
	{

	  for (hadr = (union olsr_ip_addr *)hinf6->neigh_addr; 
	       (char *)hadr < (char *)hinf6 + ntohs(hinf6->size); 
	       hadr++)
	    {
	      nb = olsr_malloc(sizeof (struct hello_neighbor), "OLSR chgestruct 2");

	      COPY_IP(&nb->address, hadr);

	      /* Fetch link and status */
	      nb->link = EXTRACT_LINK(hinf6->link_code);
	      nb->status = EXTRACT_STATUS(hinf6->link_code);

	      nb->next = hmsg->neighbors;
	      hmsg->neighbors = nb;
	    }
	}

    }

}


/**
 *Process/rebuild TC message. Converts the OLSR
 *packet to the internal tc_message format.
 *@param tmsg the tc_message struct in wich infomation
 *is to be put.
 *@param m the entire OLSR message revieved.
 *@param from a sockaddr struct describing the 1 hop sender
 *@return negative on error
 */

void
tc_chgestruct(struct tc_message *tmsg, union olsr_message *m, union olsr_ip_addr *from_addr)
{
  struct tc_mpr_addr *mprs;
  union olsr_ip_addr *tmp_addr;

  tmsg->multipoint_relay_selector_address = NULL;

  if ((!m) || (m->v4.olsr_msgtype != TC_MESSAGE))
    return;

  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IPv4 */
      struct tcmsg *tc;
      struct neigh_info *mprsaddr, *maddr;

      tc = &m->v4.message.tc;
      mprsaddr = tc->neigh;

      if((tmp_addr = mid_lookup_main_addr(from_addr)) == 0)
	{
	  COPY_IP(&tmsg->source_addr, from_addr);
	}
      else
	{
	  COPY_IP(&tmsg->source_addr, tmp_addr);
	}


      /* Get vtime */
      tmsg->vtime = ME_TO_DOUBLE(m->v4.olsr_vtime);

      OLSR_PRINTF(3, "Got TC vtime: %f\n", tmsg->vtime)

      COPY_IP(&tmsg->originator, &m->v4.originator);
      tmsg->packet_seq_number = ntohs(m->v4.seqno);
      tmsg->hop_count =  m->v4.hopcnt;
      tmsg->ansn =  ntohs(tc->ansn);

      //printf("TC from %s seqno %d\n", olsr_ip_to_string(&tmsg->originator), tmsg->packet_seq_number);

      for (maddr = mprsaddr; (char *)maddr < ((char *)m + (ntohs(m->v4.olsr_msgsize))); maddr++)
	{
	  
	  mprs = olsr_malloc(sizeof(struct tc_mpr_addr), "TC chgestruct");

	  COPY_IP(&mprs->address, &maddr->addr);
	  mprs->next = tmsg->multipoint_relay_selector_address;
	  tmsg->multipoint_relay_selector_address = mprs;
	}
    }
  else
    {
      /* IPv6 */
      struct tcmsg6 *tc6;
      struct neigh_info6 *mprsaddr6, *maddr6;

      tc6 = &m->v6.message.tc;
      mprsaddr6 = tc6->neigh;

      if((tmp_addr = mid_lookup_main_addr(from_addr)) == 0)
	{
	  COPY_IP(&tmsg->source_addr, from_addr);
	}
      else
	{
	  COPY_IP(&tmsg->source_addr, tmp_addr);
	}

      /* Check if sender is symmetric neighbor here !! */
      
      /* Get vtime */
      tmsg->vtime = ME_TO_DOUBLE(m->v6.olsr_vtime);

      OLSR_PRINTF(3, "Got TC vtime: %f\n", tmsg->vtime)

      COPY_IP(&tmsg->originator, &m->v6.originator);
      tmsg->packet_seq_number = ntohs(m->v6.seqno);
      tmsg->hop_count =  m->v6.hopcnt;
      tmsg->ansn =  ntohs(tc6->ansn);

      for (maddr6 = mprsaddr6; (char *)maddr6 < ((char *)m + (ntohs(m->v6.olsr_msgsize))); maddr6++)
	{
	  
	  mprs = olsr_malloc(sizeof(struct tc_mpr_addr), "TC chgestruct 2");

	  COPY_IP(&mprs->address, &maddr6->addr);
	  mprs->next = tmsg->multipoint_relay_selector_address;
	  tmsg->multipoint_relay_selector_address = mprs;
	}

    }

}
