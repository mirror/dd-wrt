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
 * $Id: build_msg.c,v 1.31 2005/11/10 19:35:12 kattemat Exp $
 */


#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "build_msg.h"
#include "local_hna_set.h"
#include "mantissa.h"

#define BMSG_DBGLVL 5

#define OLSR_IPV4_HDRSIZE          12
#define OLSR_IPV6_HDRSIZE          24

#define OLSR_HELLO_IPV4_HDRSIZE    (OLSR_IPV4_HDRSIZE + 4)   
#define OLSR_HELLO_IPV6_HDRSIZE    (OLSR_IPV6_HDRSIZE + 4)
#define OLSR_TC_IPV4_HDRSIZE       (OLSR_IPV4_HDRSIZE + 4)
#define OLSR_TC_IPV6_HDRSIZE       (OLSR_IPV6_HDRSIZE + 4)
#define OLSR_MID_IPV4_HDRSIZE      OLSR_IPV4_HDRSIZE
#define OLSR_MID_IPV6_HDRSIZE      OLSR_IPV6_HDRSIZE
#define OLSR_HNA_IPV4_HDRSIZE      OLSR_IPV4_HDRSIZE
#define OLSR_HNA_IPV6_HDRSIZE      OLSR_IPV6_HDRSIZE


/* All these functions share this buffer */

static olsr_u8_t msg_buffer[MAXMESSAGESIZE - OLSR_HEADERSIZE];

/* Prototypes for internal functions */

/* IPv4 */

static olsr_bool
serialize_hello4(struct hello_message *, struct interface *);

static olsr_bool
serialize_tc4(struct tc_message *, struct interface *);

static olsr_bool
serialize_mid4(struct interface *);

static olsr_bool
serialize_hna4(struct interface *);

/* IPv6 */

static olsr_bool
serialize_hello6(struct hello_message *, struct interface *);

static olsr_bool
serialize_tc6(struct tc_message *, struct interface *);

static olsr_bool
serialize_mid6(struct interface *);

static olsr_bool
serialize_hna6(struct interface *);



/**
 * Generate HELLO packet with the contents of the parameter "message".
 * If this won't fit in one packet, chop it up into several.
 * Send the packet if the size of the data contained in the output buffer
 * reach maxmessagesize. Can generate an empty HELLO packet if the 
 * neighbor table is empty. 
 *
 *
 *@param message the hello_message struct containing the info
 *to build the hello message from.
 *@param ifp the interface to send the message on
 *
 *@return nada
 */

olsr_bool
queue_hello(struct hello_message *message, struct interface *ifp)
{
#ifdef DEBUG
  OLSR_PRINTF(BMSG_DBGLVL, "Building HELLO on %s\n-------------------\n", ifp->int_name)
#endif

  switch(olsr_cnf->ip_version)
    {
    case(AF_INET):
      return serialize_hello4(message, ifp);
    case(AF_INET6):
      return serialize_hello6(message, ifp);
    }
  return OLSR_FALSE;
}


/*
 * Generate TC packet with the contents of the parameter "message".
 * If this won't fit in one packet, chop it up into several.
 * Send the packet if the size of the data contained in the output buffer
 * reach maxmessagesize. 
 *
 *@param message the tc_message struct containing the info
 *to send
 *@param ifp the interface to send the message on
 *
 *@return nada
 */

olsr_bool
queue_tc(struct tc_message *message, struct interface *ifp)           
{
#ifdef DEBUG
  OLSR_PRINTF(BMSG_DBGLVL, "Building TC on %s\n-------------------\n", ifp->int_name)
#endif

  switch(olsr_cnf->ip_version)
    {
    case(AF_INET):
      return serialize_tc4(message, ifp);
    case(AF_INET6):
      return serialize_tc6(message, ifp);
    }
  return OLSR_FALSE;
}


/**
 *Build a MID message to the outputbuffer
 *
 *<b>NO INTERNAL BUFFER</b>
 *@param ifn use this interfaces address as main address
 *@return 1 on success
 */

olsr_bool
queue_mid(struct interface *ifp)
{
#ifdef DEBUG
  OLSR_PRINTF(BMSG_DBGLVL, "Building MID on %s\n-------------------\n", ifp->int_name)
#endif

  switch(olsr_cnf->ip_version)
    {
    case(AF_INET):
      return serialize_mid4(ifp);
    case(AF_INET6):
      return serialize_mid6(ifp);
    }
  return OLSR_FALSE;
}


/**
 *Builds a HNA message in the outputbuffer
 *<b>NB! Not internal packetformat!</b>
 *
 *@param ifp the interface to send on
 *@return nada
 */
olsr_bool
queue_hna(struct interface *ifp)
{
#ifdef DEBUG
  OLSR_PRINTF(BMSG_DBGLVL, "Building HNA on %s\n-------------------\n", ifp->int_name)
#endif

  switch(olsr_cnf->ip_version)
    {
    case(AF_INET):
      return serialize_hna4(ifp);
    case(AF_INET6):
      return serialize_hna6(ifp);
    }
  return OLSR_FALSE;
}

/*
 * Protocol specific versions
 */


static void
check_buffspace(int msgsize, int buffsize, char *type)
{
  if(msgsize > buffsize)
    {
      OLSR_PRINTF(1, "%s build, outputbuffer to small(%d/%d)!\n", type, msgsize, buffsize)
      olsr_syslog(OLSR_LOG_ERR, "%s build, outputbuffer to small(%d/%d)!\n", type, msgsize, buffsize);
      olsr_exit(__func__, EXIT_FAILURE);
    }
}


/**
 * IP version 4
 *
 *@param message the hello_message struct containing the info
 *to build the hello message from.
 *@param ifp the interface to send the message on
 *
 *@return nada
 */

static olsr_bool
serialize_hello4(struct hello_message *message, struct interface *ifp)
{
  olsr_u16_t remainsize, curr_size;
  struct hello_neighbor *nb;
  union olsr_message *m;
  struct hellomsg *h;
  struct hellinfo *hinfo;
  union olsr_ip_addr *haddr;
  int i, j;
  olsr_bool first_entry;

  if((!message) || (!ifp) || (olsr_cnf->ip_version != AF_INET))
    return OLSR_FALSE;

  remainsize = net_outbuffer_bytes_left(ifp);

  m = (union olsr_message *)msg_buffer;

  curr_size = OLSR_HELLO_IPV4_HDRSIZE;

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  /* Sanity check */
  check_buffspace(curr_size, remainsize, "HELLO");

  h = &m->v4.message.hello;
  hinfo = h->hell_info;
  haddr = (union olsr_ip_addr *)hinfo->neigh_addr;
  
  /* Fill message header */
  m->v4.ttl = message->ttl;
  m->v4.hopcnt = 0;
  m->v4.olsr_msgtype = HELLO_MESSAGE;
  /* Set source(main) addr */
  COPY_IP(&m->v4.originator, &main_addr);

  m->v4.olsr_vtime = ifp->valtimes.hello;

  /* Fill HELLO header */
  h->willingness = message->willingness; 
  h->htime = double_to_me(ifp->hello_etime);

  memset(&h->reserved, 0, sizeof(olsr_u16_t));
  

  /*
   *Loops trough all possible neighbor statuses
   *The negbor list is grouped by status
   *
   */
  /* Nighbor statuses */
  for (i = 0; i <= MAX_NEIGH; i++) 
    {
      /* Link statuses */
      for(j = 0; j <= MAX_LINK; j++)
	{

	  /* HYSTERESIS - Not adding neighbors with link type HIDE */
	  
	  if(j == HIDE_LINK)
	      continue;

	  first_entry = OLSR_TRUE;

	  /* Looping trough neighbors */
	  for (nb = message->neighbors; nb != NULL; nb = nb->next) 
	    {	  
	      if ((nb->status != i) || (nb->link != j))
		continue;

#ifdef DEBUG
	      OLSR_PRINTF(BMSG_DBGLVL, "\t%s - ", olsr_ip_to_string(&nb->address))
	      OLSR_PRINTF(BMSG_DBGLVL, "L:%d N:%d\n", j, i)
#endif		  
	      /*
	       * If there is not enough room left 
	       * for the data in the outputbuffer
	       * we must send a partial HELLO and
	       * continue building the rest of the
	       * data in a new HELLO message
	       *
	       * If this is the first neighbor in 
	       * a group, we must check for an extra
	       * 4 bytes
	       */
	      if((curr_size + ipsize + (first_entry ? 4 : 0)) > remainsize)
		{
		  /* Only send partial HELLO if it contains data */
		  if(curr_size > OLSR_HELLO_IPV4_HDRSIZE)
		    {
#ifdef DEBUG
		      OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
		      /* Complete the headers */
		      m->v4.seqno = htons(get_msg_seqno());
		      m->v4.olsr_msgsize = htons(curr_size);
		      
		      hinfo->size = htons((char *)haddr - (char *)hinfo);
		      
		      /* Send partial packet */
		      net_outbuffer_push(ifp, msg_buffer, curr_size);
		      
		      curr_size = OLSR_HELLO_IPV4_HDRSIZE;
		      
		      h = &m->v4.message.hello;
		      hinfo = h->hell_info;
		      haddr = (union olsr_ip_addr *)hinfo->neigh_addr;			  
		      /* Make sure typeheader is added */
		      first_entry = OLSR_TRUE;
		    }
		  
		  net_output(ifp);			  
		  /* Reset size and pointers */
		  remainsize = net_outbuffer_bytes_left(ifp);
		  
		  /* Sanity check */
		  check_buffspace(curr_size + ipsize + 4, remainsize, "HELLO2");
		}
	      
	      if (first_entry)
		{
		  memset(&hinfo->reserved, 0, sizeof(olsr_u8_t));
		  /* Set link and status for this group of neighbors (this is the first) */
		  hinfo->link_code = CREATE_LINK_CODE(i, j);
		  curr_size += 4; /* HELLO type section header */
		}
	      
	      COPY_IP(haddr, &nb->address);
	      
	      /* Point to next address */
	      haddr = (union olsr_ip_addr *)&haddr->v6.s6_addr[4];
	      curr_size += ipsize; /* IP address added */

	      first_entry = OLSR_FALSE;
	    }
    
	  if(!first_entry)
	    {
	      hinfo->size = htons((char *)haddr - (char *)hinfo);
	      hinfo = (struct hellinfo *)((char *)haddr);
	      haddr = (union olsr_ip_addr *)&hinfo->neigh_addr;
	    }
	} /* for j */
    } /* for i*/
     
  m->v4.seqno = htons(get_msg_seqno());
  m->v4.olsr_msgsize = htons(curr_size);
  
  net_outbuffer_push(ifp, msg_buffer, curr_size);

  /* HELLO will always be generated */
  return OLSR_TRUE;
}




/**
 * IP version 6
 *
 *@param message the hello_message struct containing the info
 *to build the hello message from.
 *@param ifp the interface to send the message on
 *
 *@return nada
 */


static olsr_bool
serialize_hello6(struct hello_message *message, struct interface *ifp)
{
  olsr_u16_t remainsize, curr_size;
  struct hello_neighbor *nb;
  union olsr_message *m;
  struct hellomsg6 *h6;
  struct hellinfo6 *hinfo6;
  union olsr_ip_addr *haddr;
  int i, j;
  olsr_bool first_entry;

  if((!message) || (!ifp) || (olsr_cnf->ip_version != AF_INET6))
    return OLSR_FALSE;

  remainsize = net_outbuffer_bytes_left(ifp);
  m = (union olsr_message *)msg_buffer;

  curr_size = OLSR_HELLO_IPV6_HDRSIZE; /* OLSR message header */

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size + ipsize + 4, remainsize, "HELLO");

  h6 = &m->v6.message.hello;
  hinfo6 = h6->hell_info;
  haddr = (union olsr_ip_addr *)hinfo6->neigh_addr;


  /* Fill message header */
  m->v6.ttl = message->ttl;
  m->v6.hopcnt = 0;
  /* Set source(main) addr */
  COPY_IP(&m->v6.originator, &main_addr);
  m->v6.olsr_msgtype = HELLO_MESSAGE;

  m->v6.olsr_vtime = ifp->valtimes.hello;
  
  /* Fill packet header */
  h6->willingness = message->willingness; 
  h6->htime = double_to_me(ifp->hello_etime);
  memset(&h6->reserved, 0, sizeof(olsr_u16_t));

  /*
   *Loops trough all possible neighbor statuses
   *The negbor list is grouped by status
   */

  for (i = 0; i <= MAX_NEIGH; i++) 
    {
      for(j = 0; j <= MAX_LINK; j++)
	{
	  first_entry = OLSR_TRUE;
	  	  
	  /*
	   *Looping trough neighbors
	   */
	  for (nb = message->neighbors; nb != NULL; nb = nb->next) 
	    {	      
	      if ((nb->status != i) || (nb->link != j))
		continue;

#ifdef DEBUG
	      OLSR_PRINTF(BMSG_DBGLVL, "\t%s - ", olsr_ip_to_string(&nb->address))
	      OLSR_PRINTF(BMSG_DBGLVL, "L:%d N:%d\n", j, i)
#endif		  


	      /*
	       * If there is not enough room left 
	       * for the data in the outputbuffer
	       * we must send a partial HELLO and
	       * continue building the rest of the
	       * data in a new HELLO message
	       *
	       * If this is the first neighbor in 
	       * a group, we must check for an extra
	       * 4 bytes
	       */
	      if((curr_size + ipsize + (first_entry ? 4 : 0)) > remainsize)
		{
		  /* Only send partial HELLO if it contains data */
		  if(curr_size > OLSR_HELLO_IPV6_HDRSIZE)
		    {
#ifdef DEBUG
		      OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
		      /* Complete the headers */
		      m->v6.seqno = htons(get_msg_seqno());
		      m->v6.olsr_msgsize = htons(curr_size);
			  
		      hinfo6->size = (char *)haddr - (char *)hinfo6;
		      hinfo6->size = htons(hinfo6->size);
			  
		      /* Send partial packet */
		      net_outbuffer_push(ifp, msg_buffer, curr_size);
		      curr_size = OLSR_HELLO_IPV6_HDRSIZE;
			  
		      h6 = &m->v6.message.hello;
		      hinfo6 = h6->hell_info;
		      haddr = (union olsr_ip_addr *)hinfo6->neigh_addr;
		      /* Make sure typeheader is added */
		      first_entry = OLSR_TRUE;
		    }
		  net_output(ifp);
		  /* Reset size and pointers */
		  remainsize = net_outbuffer_bytes_left(ifp);

		  check_buffspace(curr_size + ipsize + 4, remainsize, "HELLO2");
		      
		}

	      if(first_entry)
		{
		  memset(&hinfo6->reserved, 0, sizeof(olsr_u8_t));
		  /* Set link and status for this group of neighbors (this is the first) */
		  hinfo6->link_code = CREATE_LINK_CODE(i, j);
		  curr_size += 4; /* HELLO type section header */
		}
		  
	      COPY_IP(haddr, &nb->address);
		  
	      /* Point to next address */
	      haddr++;
	      curr_size += ipsize; /* IP address added */ 
		  
	      first_entry = OLSR_FALSE;
	    }/* looping trough neighbors */
	  
	  
	  if (!first_entry)
	    {
	      hinfo6->size = htons((char *)haddr - (char *)hinfo6);
	      hinfo6 = (struct hellinfo6 *)((char *)haddr);
	      haddr = (union olsr_ip_addr *)&hinfo6->neigh_addr;
	    }
	  
	} /* for j */
    } /* for i */

  m->v6.seqno = htons(get_msg_seqno());
  m->v6.olsr_msgsize = htons(curr_size);

  net_outbuffer_push(ifp, msg_buffer, curr_size);

  /* HELLO is always buildt */
  return OLSR_TRUE;
}



/**
 *IP version 4
 *
 *@param message the tc_message struct containing the info
 *to send
 *@param ifp the interface to send the message on
 *
 *@return nada
 */

static olsr_bool
serialize_tc4(struct tc_message *message, struct interface *ifp)           
{

  olsr_u16_t remainsize, curr_size;
  struct tc_mpr_addr *mprs;
  union olsr_message *m;
  struct tcmsg *tc;
  struct neigh_info *mprsaddr; 
  olsr_bool found = OLSR_FALSE, partial_sent = OLSR_FALSE;

  if((!message) || (!ifp) || (olsr_cnf->ip_version != AF_INET))
    return OLSR_FALSE;

  remainsize = net_outbuffer_bytes_left(ifp);

  m = (union olsr_message *)msg_buffer;

  tc = &m->v4.message.tc;


  mprsaddr = tc->neigh;
  curr_size = OLSR_TC_IPV4_HDRSIZE;

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size, remainsize, "TC");

  /* Fill header */
  m->v4.olsr_vtime = ifp->valtimes.tc;
  m->v4.olsr_msgtype = TC_MESSAGE;
  m->v4.hopcnt = message->hop_count;
  m->v4.ttl = message->ttl;
  COPY_IP(&m->v4.originator, &message->originator);

  /* Fill TC header */
  tc->ansn = htons(message->ansn);
  tc->reserved = 0;
  

  /*Looping trough MPR selectors */
  for (mprs = message->multipoint_relay_selector_address; mprs != NULL;mprs = mprs->next) 
    {
      /*If packet is to be chomped */
      if((curr_size + ipsize) > remainsize)
	{

	  /* Only add TC message if it contains data */
	  if(curr_size > OLSR_TC_IPV4_HDRSIZE)
	    {
#ifdef DEBUG
	      OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif

	      m->v4.olsr_msgsize = htons(curr_size);
	      m->v4.seqno = htons(get_msg_seqno());

	      net_outbuffer_push(ifp, msg_buffer, curr_size);
	      
	      /* Reset stuff */
	      mprsaddr = tc->neigh;
	      curr_size = OLSR_TC_IPV4_HDRSIZE;
	      found = OLSR_FALSE;
	      partial_sent = OLSR_TRUE;
	    }

	  net_output(ifp);
	  remainsize = net_outbuffer_bytes_left(ifp);
	  check_buffspace(curr_size + ipsize, remainsize, "TC2");

	}
      found = OLSR_TRUE;
#ifdef DEBUG
	  OLSR_PRINTF(BMSG_DBGLVL, "\t%s\n", 
		      olsr_ip_to_string(&mprs->address))
#endif 
      COPY_IP(&mprsaddr->addr, &mprs->address);

      curr_size += ipsize;
      mprsaddr++;
    }

  if (found)
    {
	    
      m->v4.olsr_msgsize = htons(curr_size);
      m->v4.seqno = htons(get_msg_seqno());
      
      net_outbuffer_push(ifp, msg_buffer, curr_size);

    }
  else
    {
      if((!partial_sent) && (!TIMED_OUT(send_empty_tc)))
	{
	  if(!TIMED_OUT(send_empty_tc))
	    OLSR_PRINTF(1, "TC: Sending empty package - (%d/%d/%d/%d)\n", partial_sent, (int)send_empty_tc, (int)now_times, (int)((send_empty_tc) - now_times))

	  m->v4.olsr_msgsize = htons(curr_size);
	  m->v4.seqno = htons(get_msg_seqno());

	  net_outbuffer_push(ifp, msg_buffer, curr_size);

	  found = OLSR_TRUE;
	}
    }

  return found;	
}



/**
 *IP version 6
 *
 *@param message the tc_message struct containing the info
 *to send
 *@param ifp the interface to send the message on
 *
 *@return nada
 */

static olsr_bool
serialize_tc6(struct tc_message *message, struct interface *ifp)           
{

  olsr_u16_t remainsize, curr_size;
  struct tc_mpr_addr *mprs;
  union olsr_message *m;
  struct tcmsg6 *tc6;
  struct neigh_info6 *mprsaddr6; 
  olsr_bool found = OLSR_FALSE, partial_sent = OLSR_FALSE;

  if ((!message) || (!ifp) || (olsr_cnf->ip_version != AF_INET6))
    return OLSR_FALSE;

  remainsize = net_outbuffer_bytes_left(ifp);

  m = (union olsr_message *)msg_buffer;

  tc6 = &m->v6.message.tc;

  mprsaddr6 = tc6->neigh;
  curr_size = OLSR_TC_IPV6_HDRSIZE;

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size, remainsize, "TC");

  /* Fill header */
  m->v6.olsr_vtime = ifp->valtimes.tc;
  m->v6.olsr_msgtype = TC_MESSAGE;
  m->v6.hopcnt = message->hop_count;
  m->v6.ttl = message->ttl;
  COPY_IP(&m->v6.originator, &message->originator);

  /* Fill TC header */
  tc6->ansn = htons(message->ansn);
  tc6->reserved = 0;
  

  /*Looping trough MPR selectors */
  for (mprs = message->multipoint_relay_selector_address; mprs != NULL;mprs = mprs->next) 
    {
	    
      /*If packet is to be chomped */
      if((curr_size + ipsize) > remainsize)
	{
	  /* Only add TC message if it contains data */
	  if(curr_size > OLSR_TC_IPV6_HDRSIZE)
	    {
#ifdef DEBUG
	      OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
	      m->v6.olsr_msgsize = htons(curr_size);
	      m->v6.seqno = htons(get_msg_seqno());

	      net_outbuffer_push(ifp, msg_buffer, curr_size);
	      mprsaddr6 = tc6->neigh;
	      curr_size = OLSR_TC_IPV6_HDRSIZE;
	      found = OLSR_FALSE;
	      partial_sent = OLSR_TRUE;
	    }
	  net_output(ifp);
	  remainsize = net_outbuffer_bytes_left(ifp);
	  check_buffspace(curr_size + ipsize, remainsize, "TC2");

	}
      found = OLSR_TRUE;
#ifdef DEBUG
	  OLSR_PRINTF(BMSG_DBGLVL, "\t%s\n", 
		      olsr_ip_to_string(&mprs->address))
#endif
      COPY_IP(&mprsaddr6->addr, &mprs->address);
      curr_size += ipsize;

      mprsaddr6++;
    }
	
  if (found)
    {
      m->v6.olsr_msgsize = htons(curr_size);
      m->v6.seqno = htons(get_msg_seqno());

      net_outbuffer_push(ifp, msg_buffer, curr_size);

    }
  else
    {
      if((!partial_sent) && (!TIMED_OUT(send_empty_tc)))
	{
	  OLSR_PRINTF(1, "TC: Sending empty package\n")
	    
	  m->v6.olsr_msgsize = htons(curr_size);
	  m->v6.seqno = htons(get_msg_seqno());

	  net_outbuffer_push(ifp, msg_buffer, curr_size);

	  found = OLSR_TRUE;
	}
    }

  return found;	
}




/**
 *IP version 4
 *
 *<b>NO INTERNAL BUFFER</b>
 *@param ifp use this interfaces address as main address
 *@return 1 on success
 */

static olsr_bool
serialize_mid4(struct interface *ifp)
{
  olsr_u16_t remainsize, curr_size;
  /* preserve existing data in output buffer */
  union olsr_message *m;
  struct midaddr *addrs;
  struct interface *ifs;  

  if((olsr_cnf->ip_version != AF_INET) || (!ifp) || (ifnet == NULL || ifnet->int_next == NULL))
    return OLSR_FALSE;


  remainsize = net_outbuffer_bytes_left(ifp);

  m = (union olsr_message *)msg_buffer;

  curr_size = OLSR_MID_IPV4_HDRSIZE;

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size, remainsize, "MID");

  /* Fill header */
  m->v4.hopcnt = 0;
  m->v4.ttl = MAX_TTL;
  /* Set main(first) address */
  COPY_IP(&m->v4.originator, &main_addr);
  m->v4.olsr_msgtype = MID_MESSAGE;
  m->v4.olsr_vtime = ifp->valtimes.mid;
 
  addrs = m->v4.message.mid.mid_addr;

  /* Don't add the main address... it's already there */
  for(ifs = ifnet; ifs != NULL; ifs = ifs->int_next)
    {
      if(!COMP_IP(&main_addr, &ifs->ip_addr))
	{

	  if((curr_size + ipsize) > remainsize)
	    {
	      /* Only add MID message if it contains data */
	      if(curr_size > OLSR_MID_IPV4_HDRSIZE)
		{
#ifdef DEBUG
		  OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
		  /* set size */
		  m->v4.olsr_msgsize = htons(curr_size);
		  m->v4.seqno = htons(get_msg_seqno());/* seqnumber */
		  
		  net_outbuffer_push(ifp, msg_buffer, curr_size);
		  curr_size = OLSR_MID_IPV4_HDRSIZE;
		  addrs = m->v4.message.mid.mid_addr;
		}
	      net_output(ifp);
	      remainsize = net_outbuffer_bytes_left(ifp);
	      check_buffspace(curr_size, remainsize, "MID2");
	    }
#ifdef DEBUG
	  OLSR_PRINTF(BMSG_DBGLVL, "\t%s(%s)\n", 
		      olsr_ip_to_string(&ifs->ip_addr), 
		      ifs->int_name)
#endif
	  
	  COPY_IP(&addrs->addr, &ifs->ip_addr);
	  addrs++;
	  curr_size += ipsize;
	}
    }


  m->v4.seqno = htons(get_msg_seqno());/* seqnumber */
  m->v4.olsr_msgsize = htons(curr_size);

  //printf("Sending MID (%d bytes)...\n", outputsize);
  net_outbuffer_push(ifp, msg_buffer, curr_size);


  return OLSR_TRUE;
}



/**
 *IP version 6
 *
 *<b>NO INTERNAL BUFFER</b>
 *@param ifp use this interfaces address as main address
 *@return 1 on success
 */

static olsr_bool
serialize_mid6(struct interface *ifp)
{
  olsr_u16_t remainsize, curr_size;
  /* preserve existing data in output buffer */
  union olsr_message *m;
  struct midaddr6 *addrs6;
  struct interface *ifs;

  //printf("\t\tGenerating mid on %s\n", ifn->int_name);


  if((olsr_cnf->ip_version != AF_INET6) || (!ifp) || (ifnet == NULL || ifnet->int_next == NULL))
    return OLSR_FALSE;

  remainsize = net_outbuffer_bytes_left(ifp);

  curr_size = OLSR_MID_IPV6_HDRSIZE;

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size + ipsize, remainsize, "MID");

  m = (union olsr_message *)msg_buffer;
    
  /* Build header */
  m->v6.hopcnt = 0;
  m->v6.ttl = MAX_TTL;      
  m->v6.olsr_msgtype = MID_MESSAGE;
  m->v6.olsr_vtime = ifp->valtimes.mid;
  /* Set main(first) address */
  COPY_IP(&m->v6.originator, &main_addr);
   

  addrs6 = m->v6.message.mid.mid_addr;

  /* Don't add the main address... it's already there */
  for(ifs = ifnet; ifs != NULL; ifs = ifs->int_next)
    {
      if(!COMP_IP(&main_addr, &ifs->ip_addr))
	{
	  if((curr_size + ipsize) > remainsize)
	    {
	      /* Only add MID message if it contains data */
	      if(curr_size > OLSR_MID_IPV6_HDRSIZE)
		{
#ifdef DEBUG
		  OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
		  /* set size */
		  m->v6.olsr_msgsize = htons(curr_size);
		  m->v6.seqno = htons(get_msg_seqno());/* seqnumber */
		  
		  net_outbuffer_push(ifp, msg_buffer, curr_size);
		  curr_size = OLSR_MID_IPV6_HDRSIZE;
		  addrs6 = m->v6.message.mid.mid_addr;
		}
	      net_output(ifp);
	      remainsize = net_outbuffer_bytes_left(ifp);
	      check_buffspace(curr_size + ipsize, remainsize, "MID2");
	    }
#ifdef DEBUG
		  OLSR_PRINTF(BMSG_DBGLVL, "\t%s(%s)\n", 
			      olsr_ip_to_string(&ifs->ip_addr), 
			      ifs->int_name)
#endif

	  COPY_IP(&addrs6->addr, &ifs->ip_addr);
	  addrs6++;
	  curr_size += ipsize;
	}
    }

  m->v6.olsr_msgsize = htons(curr_size);
  m->v6.seqno = htons(get_msg_seqno());/* seqnumber */

  //printf("Sending MID (%d bytes)...\n", outputsize);
  net_outbuffer_push(ifp, msg_buffer, curr_size);

  return OLSR_TRUE;
}




/**
 *IP version 4
 *
 *@param ifp the interface to send on
 *@return nada
 */
static olsr_bool
serialize_hna4(struct interface *ifp)
{
  olsr_u16_t remainsize, curr_size;
  /* preserve existing data in output buffer */
  union olsr_message *m;
  struct hnapair *pair;
  struct hna4_entry *h = olsr_cnf->hna4_entries;

  /* No hna nets */
  if((olsr_cnf->ip_version != AF_INET) || (!ifp) || h == NULL)
    return OLSR_FALSE;
    
  remainsize = net_outbuffer_bytes_left(ifp);
  
  curr_size = OLSR_HNA_IPV4_HDRSIZE;
  
  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size, remainsize, "HNA");

  m = (union olsr_message *)msg_buffer;
  
  
  /* Fill header */
  COPY_IP(&m->v4.originator, &main_addr);
  m->v4.hopcnt = 0;
  m->v4.ttl = MAX_TTL;
  m->v4.olsr_msgtype = HNA_MESSAGE;
  m->v4.olsr_vtime = ifp->valtimes.hna;
  

  pair = m->v4.message.hna.hna_net;
  
  while(h)
    {
      if((curr_size + (2 * ipsize)) > remainsize)
	{
	  /* Only add HNA message if it contains data */
	  if(curr_size > OLSR_HNA_IPV4_HDRSIZE)
	    {
#ifdef DEBUG
	      OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
	      m->v4.seqno = htons(get_msg_seqno());
	      m->v4.olsr_msgsize = htons(curr_size);
	      net_outbuffer_push(ifp, msg_buffer, curr_size);
	      curr_size = OLSR_HNA_IPV4_HDRSIZE;
	      pair = m->v4.message.hna.hna_net;
	    }
	  net_output(ifp);
	  remainsize = net_outbuffer_bytes_left(ifp);
	  check_buffspace(curr_size + (2 * ipsize), remainsize, "HNA2");
	}
#ifdef DEBUG
      OLSR_PRINTF(BMSG_DBGLVL, "\tNet: %s/%s\n", 
		  olsr_ip_to_string(&h->net),
		  olsr_ip_to_string(&h->netmask))
#endif
      COPY_IP(&pair->addr, &h->net);
      COPY_IP(&pair->netmask, &h->netmask);
      pair++;
      curr_size += (2 * ipsize);
      h = h->next;
    }

  m->v4.seqno = htons(get_msg_seqno());
  m->v4.olsr_msgsize = htons(curr_size);

  net_outbuffer_push(ifp, msg_buffer, curr_size);

  //printf("Sending HNA (%d bytes)...\n", outputsize);
  return OLSR_FALSE;
}





/**
 *IP version 6
 *
 *@param ifp the interface to send on
 *@return nada
 */
static olsr_bool
serialize_hna6(struct interface *ifp)
{
  olsr_u16_t remainsize, curr_size;
  /* preserve existing data in output buffer */
  union olsr_message *m;
  struct hnapair6 *pair6;
  union olsr_ip_addr tmp_netmask;
  struct hna6_entry *h = olsr_cnf->hna6_entries;
  
  /* No hna nets */
  if((olsr_cnf->ip_version != AF_INET6) || (!ifp) || h == NULL)
    return OLSR_FALSE;

    
  remainsize = net_outbuffer_bytes_left(ifp);

  curr_size = OLSR_HNA_IPV6_HDRSIZE;

  /* Send pending packet if not room in buffer */
  if(curr_size > remainsize)
    {
      net_output(ifp);
      remainsize = net_outbuffer_bytes_left(ifp);
    }
  check_buffspace(curr_size, remainsize, "HNA");

  m = (union olsr_message *)msg_buffer;   

  /* Fill header */
  COPY_IP(&m->v6.originator, &main_addr);
  m->v6.hopcnt = 0;
  m->v6.ttl = MAX_TTL;
  m->v6.olsr_msgtype = HNA_MESSAGE;
  m->v6.olsr_vtime = ifp->valtimes.hna;

  pair6 = m->v6.message.hna.hna_net;


  while(h)
    {
      if((curr_size + (2 * ipsize)) > remainsize)
	{
	  /* Only add HNA message if it contains data */
	  if(curr_size > OLSR_HNA_IPV6_HDRSIZE)
	    {
#ifdef DEBUG
	      OLSR_PRINTF(BMSG_DBGLVL, "Sending partial(size: %d, buff left:%d)\n", curr_size, remainsize)
#endif
	      m->v6.seqno = htons(get_msg_seqno());
	      m->v6.olsr_msgsize = htons(curr_size);
	      net_outbuffer_push(ifp, msg_buffer, curr_size);
	      curr_size = OLSR_HNA_IPV6_HDRSIZE;
	      pair6 = m->v6.message.hna.hna_net;
	    }
	  net_output(ifp);
	  remainsize = net_outbuffer_bytes_left(ifp);
	  check_buffspace(curr_size + (2 * ipsize), remainsize, "HNA2");
	}
#ifdef DEBUG
      OLSR_PRINTF(BMSG_DBGLVL, "\tNet: %s/%d\n", 
		  olsr_ip_to_string(&h->net),
		  h->prefix_len)
#endif
      COPY_IP(&pair6->addr, &h->net);
      olsr_prefix_to_netmask(&tmp_netmask, h->prefix_len);
      COPY_IP(&pair6->netmask, &tmp_netmask);
      pair6++;
      curr_size += (2 * ipsize);
      h = h->next;
    }
  
  m->v6.olsr_msgsize = htons(curr_size);
  m->v6.seqno = htons(get_msg_seqno());
  
  net_outbuffer_push(ifp, msg_buffer, curr_size);
  
  //printf("Sending HNA (%d bytes)...\n", outputsize);
  return OLSR_FALSE;

}
