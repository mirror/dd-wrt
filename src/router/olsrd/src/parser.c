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
 * $Id: parser.c,v 1.29 2005/11/19 08:49:44 kattemat Exp $
 */

#include "parser.h"
#include "defs.h"
#include "process_package.h"
#include "mantissa.h"
#include "hysteresis.h"
#include "duplicate_set.h"
#include "mid_set.h"
#include "olsr.h"
#include "rebuild_packet.h"
#include "net_os.h"
#include "log.h"
#include "print_packet.h"

#ifdef WIN32
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef errno
#define errno WSAGetLastError()
#undef strerror
#define strerror(x) StrError(x)
#endif

struct parse_function_entry *parse_functions;

static char inbuf[MAXMESSAGESIZE+1];

/**
 *Initialize the parser. 
 *
 *@return nada
 */
void
olsr_init_parser()
{
  OLSR_PRINTF(3, "Initializing parser...\n")

  /* Initialize the packet functions */
  olsr_init_package_process();

}

void
olsr_parser_add_function(void (*function)(union olsr_message *, struct interface *, union olsr_ip_addr *), olsr_u32_t type, int forwarding)
{
  struct parse_function_entry *new_entry;

  OLSR_PRINTF(3, "Parser: registering event for type %d\n", type)
 

  new_entry = olsr_malloc(sizeof(struct parse_function_entry), "Register parse function");

  new_entry->function = function;
  new_entry->type = type;
  new_entry->caller_forwarding = forwarding;

  /* Queue */
  new_entry->next = parse_functions;
  parse_functions = new_entry;

  OLSR_PRINTF(3, "Register parse function: Added function for type %d\n", type)

}



int
olsr_parser_remove_function(void (*function)(union olsr_message *, struct interface *, union olsr_ip_addr *), olsr_u32_t type, int forwarding)
{
  struct parse_function_entry *entry, *prev;

  entry = parse_functions;
  prev = NULL;

  while(entry)
    {
      if((entry->function == function) &&
	 (entry->type == type) &&
	 (entry->caller_forwarding == forwarding))
	{
	  if(entry == parse_functions)
	    {
	      parse_functions = entry->next;
	    }
	  else
	    {
	      prev->next = entry->next;
	    }
	  free(entry);
	  return 1;
	}

      prev = entry;
      entry = entry->next;
    }

  return 0;
}


/**
 *Process a newly received OLSR packet. Checks the type
 *and to the neccessary convertions and call the
 *corresponding functions to handle the information.
 *@param from the sockaddr struct describing the sender
 *@param olsr the olsr struct containing the message
 *@param size the size of the message
 *@return nada
 */

void
parse_packet(struct olsr *olsr, int size, struct interface *in_if, union olsr_ip_addr *from_addr)
{
  union olsr_message *m = (union olsr_message *)olsr->olsr_msg;
  struct unknown_message unkpacket;
  int count;
  int msgsize;
  int processed;
  struct parse_function_entry *entry;

  count = size - ((char *)m - (char *)olsr);

  if (count < minsize)
    return;

  if (ntohs(olsr->olsr_packlen) != size)
    {
      OLSR_PRINTF(1, "Size error detected in received packet.\nRecieved %d, in packet %d\n", size, ntohs(olsr->olsr_packlen))
	    
      olsr_syslog(OLSR_LOG_ERR, " packet length error in  packet received from %s!",
	     olsr_ip_to_string(from_addr));
      return;
    }

  //printf("Message from %s\n\n", olsr_ip_to_string(from_addr)); 
      
  /* Display packet */
  if(disp_pack_in)
    print_olsr_serialized_packet(stdout, (union olsr_packet *)olsr, size, from_addr);

  if(olsr_cnf->ip_version == AF_INET)
    msgsize = ntohs(m->v4.olsr_msgsize);
  else
    msgsize = ntohs(m->v6.olsr_msgsize);


  /*
   * Hysteresis update - for every OLSR package
   */
  if(olsr_cnf->use_hysteresis)
    {
      if(olsr_cnf->ip_version == AF_INET)
	{
	  /* IPv4 */
	  update_hysteresis_incoming(from_addr, 
				     &in_if->ip_addr,
				     ntohs(olsr->olsr_seqno));
	}
      else
	{
	  /* IPv6 */
	  update_hysteresis_incoming(from_addr, 
				     &in_if->ip_addr, 
				     ntohs(olsr->olsr_seqno));
	}
    }

  if (olsr_cnf->lq_level > 0)
    {
      olsr_update_packet_loss(from_addr, &in_if->ip_addr,
                              ntohs(olsr->olsr_seqno));
    }
  
  for ( ; count > 0; m = (union olsr_message *)((char *)m + (msgsize)))
    {

      processed = 0;      
      if (count < minsize)
	break;
      
      if(olsr_cnf->ip_version == AF_INET)
	msgsize = ntohs(m->v4.olsr_msgsize);
      else
	msgsize = ntohs(m->v6.olsr_msgsize);
      
      count -= msgsize;

      /* Check size of message */
      if(count < 0)
	{
	  OLSR_PRINTF(1, "packet length error in  packet received from %s!",
		      olsr_ip_to_string(from_addr))

	  olsr_syslog(OLSR_LOG_ERR, " packet length error in  packet received from %s!",
		 olsr_ip_to_string(from_addr));
	  break;
	}


      /* Treat TTL hopcnt */
      if(olsr_cnf->ip_version == AF_INET)
	{
	  /* IPv4 */
	  if (m->v4.ttl <= 0 && olsr_cnf->lq_fish == 0)
	    {
	      OLSR_PRINTF(2, "Dropping packet type %d from neigh %s with TTL 0\n", 
			  m->v4.olsr_msgtype,
			  olsr_ip_to_string(from_addr))
	      continue;
	    }
	}
      else
	{
	  /* IPv6 */
	  if (m->v6.ttl <= 0 && olsr_cnf->lq_fish == 0) 
	    {
	      OLSR_PRINTF(2, "Dropping packet type %d from %s with TTL 0\n", 
			  m->v4.olsr_msgtype,
			  olsr_ip_to_string(from_addr))
	      continue;
	    }
	}

      /*RFC 3626 section 3.4:
       *  2    If the time to live of the message is less than or equal to
       *  '0' (zero), or if the message was sent by the receiving node
       *  (i.e., the Originator Address of the message is the main
       *  address of the receiving node): the message MUST silently be
       *  dropped.
       */

      /* Should be the same for IPv4 and IPv6 */
      if(COMP_IP(&m->v4.originator, &main_addr))
	{
#ifdef DEBUG
	  OLSR_PRINTF(3, "Not processing message originating from us!\n")
#endif
	  continue;
	}


      //printf("MESSAGETYPE: %d\n", m->v4.olsr_msgtype);

      entry = parse_functions;

      while(entry)
	{
	  /* Should be the same for IPv4 and IPv6 */

	  /* Promiscuous or exact match */
	  if((entry->type == PROMISCUOUS) || 
	     (entry->type == m->v4.olsr_msgtype))
	    {
	      entry->function(m, in_if, from_addr);
	      if(entry->caller_forwarding)
		processed = 1;
	    }
	  entry = entry->next;
	}


      /* UNKNOWN PACKETTYPE */
      if(processed == 0)
	{
	  unk_chgestruct(&unkpacket, m);
	  
	  OLSR_PRINTF(3, "Unknown type: %d, size %d, from %s\n",
		      m->v4.olsr_msgtype,
		      size,
		      olsr_ip_to_string(&unkpacket.originator))

	  /* Forward message */
	  if(!COMP_IP(&unkpacket.originator, &main_addr))
	    {	      
	      /* Forward */
	      olsr_forward_message(m, 
				   &unkpacket.originator, 
				   unkpacket.seqno, 
				   in_if,
				   from_addr);
	    }

	}

    } /* for olsr_msg */ 


}




/**
 *Processing OLSR data from socket. Reading data, setting 
 *wich interface recieved the message, Sends IPC(if used) 
 *and passes the packet on to parse_packet().
 *
 *@param fd the filedescriptor that data should be read from.
 *@return nada
 */
void
olsr_input(int fd)
{
  /* sockaddr_in6 is bigger than sockaddr !!!! */
  struct sockaddr_storage from;
  socklen_t fromlen;
  int cc;
  struct interface *olsr_in_if;
  union olsr_ip_addr from_addr;

  for (;;) 
    {
      fromlen = sizeof(struct sockaddr_storage);

      cc = olsr_recvfrom(fd, 
			 inbuf, 
			 sizeof (inbuf), 
			 0, 
			 (struct sockaddr *)&from, 
			 &fromlen);

      if (cc <= 0) 
	{
	  if (cc < 0 && errno != EWOULDBLOCK)
	    {
	      OLSR_PRINTF(1, "error recvfrom: %s", strerror(errno))
	      olsr_syslog(OLSR_LOG_ERR, "error recvfrom: %m");
	    }
	  break;
	}

      if(olsr_cnf->ip_version == AF_INET)
	{
	  /* IPv4 sender address */
	  COPY_IP(&from_addr, &((struct sockaddr_in *)&from)->sin_addr.s_addr);
	}
      else
	{
	  /* IPv6 sender address */
	  COPY_IP(&from_addr, &((struct sockaddr_in6 *)&from)->sin6_addr);
	}
      
      
#ifdef DEBUG
      OLSR_PRINTF(5, "Recieved a packet from %s\n", olsr_ip_to_string((union olsr_ip_addr *)&((struct sockaddr_in *)&from)->sin_addr.s_addr))
#endif
	
	if ((olsr_cnf->ip_version == AF_INET) && (fromlen != sizeof (struct sockaddr_in)))
	  break;
	else if ((olsr_cnf->ip_version == AF_INET6) && (fromlen != sizeof (struct sockaddr_in6)))
	  break;
      
      /* are we talking to ourselves? */
      if(if_ifwithaddr(&from_addr) != NULL)
	return;
      
      if((olsr_in_if = if_ifwithsock(fd)) == NULL)
	{
	  OLSR_PRINTF(1, "Could not find input interface for message from %s size %d\n",
		      olsr_ip_to_string(&from_addr),
		      cc)
	  olsr_syslog(OLSR_LOG_ERR, "Could not find input interface for message from %s size %d\n",
		 olsr_ip_to_string(&from_addr),
		 cc);
	  return ;
	}

      /*
       * &from - sender
       * &inbuf.olsr 
       * cc - bytes read
       */
      parse_packet((struct olsr *)inbuf, cc, olsr_in_if, &from_addr);
    
    }
}




/**
 *Processing OLSR data from socket. Reading data, setting 
 *wich interface recieved the message, Sends IPC(if used) 
 *and passes the packet on to parse_packet().
 *
 *@param fd the filedescriptor that data should be read from.
 *@return nada
 */
void
olsr_input_hostemu(int fd)
{
  /* sockaddr_in6 is bigger than sockaddr !!!! */
  struct sockaddr_storage from;
  socklen_t fromlen;
  int cc;
  struct interface *olsr_in_if;
  union olsr_ip_addr from_addr;
  olsr_u16_t pcklen;

  /* Host emulator receives IP address first to emulate
     direct link */

  if((cc = recv(fd, from_addr.v6.s6_addr, ipsize, 0)) != (int)ipsize)
    {
      fprintf(stderr, "Error receiving host-client IP hook(%d) %s!\n", cc, strerror(errno));
      COPY_IP(&from_addr, &((struct olsr *)inbuf)->olsr_msg->originator);
    }

  /* are we talking to ourselves? */
  if(if_ifwithaddr(&from_addr) != NULL)
    return;
      
  /* Extract size */
  if((cc = recv(fd, &pcklen, 2, MSG_PEEK)) != 2)
    {
      if(cc <= 0)
	{
	  fprintf(stderr, "Lost olsr_switch connection - exit!\n");
	  olsr_exit(__func__, EXIT_FAILURE);
	}
      fprintf(stderr, "[hust-emu] error extracting size(%d) %s!\n", cc, strerror(errno));
      return;
    }
  else
    {
      pcklen = ntohs(pcklen);
    }

  fromlen = sizeof(struct sockaddr_storage);
  
  cc = olsr_recvfrom(fd, 
		     inbuf, 
		     pcklen, 
		     0, 
		     (struct sockaddr *)&from, 
		     &fromlen);

  if (cc <= 0) 
    {
      if (cc < 0 && errno != EWOULDBLOCK)
	{
	  OLSR_PRINTF(1, "error recvfrom: %s", strerror(errno))
	    olsr_syslog(OLSR_LOG_ERR, "error recvfrom: %m");
	}
      return;
    }
  
  if(cc != pcklen)
    {
      printf("Could not read whole packet(size %d, read %d)\n", pcklen, cc);
      return;
    }

  if((olsr_in_if = if_ifwithsock(fd)) == NULL)
    {
      OLSR_PRINTF(1, "Could not find input interface for message from %s size %d\n",
		  olsr_ip_to_string(&from_addr),
		  cc)
	olsr_syslog(OLSR_LOG_ERR, "Could not find input interface for message from %s size %d\n",
		    olsr_ip_to_string(&from_addr),
		    cc);
      return;
    }
  
  /*
   * &from - sender
   * &inbuf.olsr 
   * cc - bytes read
   */
  parse_packet((struct olsr *)inbuf, cc, olsr_in_if, &from_addr);
  
}


