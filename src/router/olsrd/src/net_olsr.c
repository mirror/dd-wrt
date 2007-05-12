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
 * $Id: net_olsr.c,v 1.22 2007/04/25 22:08:09 bernd67 Exp $
 */

#include "net_olsr.h"
#include "log.h"
#include "olsr.h"
#include "net_os.h"
#include "print_packet.h"
#include "link_set.h"
#include <stdlib.h>
#include <assert.h>

extern olsr_bool lq_tc_pending;

static olsr_bool disp_pack_out = OLSR_FALSE;


#ifdef WIN32
#define perror(x) WinSockPError(x)
void
WinSockPError(char *);
#endif


struct deny_address_entry
{
  union olsr_ip_addr        addr;
  struct deny_address_entry *next;
};


/* Packet transform functions */

struct ptf
{
  packet_transform_function function;
  struct ptf *next;
};

static struct ptf *ptf_list;

static struct deny_address_entry *deny_entries;

static const char * const deny_ipv4_defaults[] =
  {
    "0.0.0.0",
    "127.0.0.1",
    NULL
  };

static const char * const deny_ipv6_defaults[] =
  {
    "0::0",
    "0::1",
    NULL
  };

void
net_set_disp_pack_out(olsr_bool val)
{
  disp_pack_out = val;
}

void
init_net(void)
{
  /* Block invalid addresses */
  if(olsr_cnf->ip_version == AF_INET)
    {
      union olsr_ip_addr addr;
      int i;
      /* IPv4 */
      for(i = 0; deny_ipv4_defaults[i] != NULL; i++)
	{
	  if(inet_pton(olsr_cnf->ip_version, deny_ipv4_defaults[i], &addr) <= 0)
	    {
	      fprintf(stderr, "Error converting fixed IP %s for deny rule!!\n",
		      deny_ipv4_defaults[i]);
	      continue;
	    }
	  olsr_add_invalid_address(&addr);
	}
    }
  else 
    {
      union olsr_ip_addr addr;
      int i;
      /* IPv6 */
      for(i = 0; deny_ipv6_defaults[i] != NULL; i++)
	{
	  if(inet_pton(olsr_cnf->ip_version, deny_ipv6_defaults[i], &addr) <= 0)
	    {
	      fprintf(stderr, "Error converting fixed IP %s for deny rule!!\n",
		      deny_ipv6_defaults[i]);
	      continue;
	    }
	  olsr_add_invalid_address(&addr);
	}

    }
}

/**
 * Create a outputbuffer for the given interface. This
 * function will allocate the needed storage according 
 * to the MTU of the interface.
 *
 * @param ifp the interface to create a buffer for
 *
 * @return 0 on success, negative if a buffer already existed
 *  for the given interface
 */ 
int
net_add_buffer(struct interface *ifp)
{
  /* Can the interfaces MTU actually change? If not, we can elimiate
   * the "bufsize" field in "struct olsr_netbuf".
   */
  if (ifp->netbuf.bufsize != ifp->int_mtu && ifp->netbuf.buff != NULL) {
    free(ifp->netbuf.buff);
    ifp->netbuf.buff = NULL;
  }
  
  if (ifp->netbuf.buff == NULL) {
    ifp->netbuf.buff = olsr_malloc(ifp->int_mtu, "add_netbuff");
  }

  /* Fill struct */
  ifp->netbuf.bufsize = ifp->int_mtu;
  ifp->netbuf.maxsize = ifp->int_mtu - OLSR_HEADERSIZE;

  ifp->netbuf.pending = 0;
  ifp->netbuf.reserved = 0;

  return 0;
}

/**
 * Remove a outputbuffer. Frees the allocated memory.
 *
 * @param ifp the interface corresponding to the buffer
 * to remove
 *
 * @return 0 on success, negative if no buffer is found
 */
int
net_remove_buffer(struct interface *ifp)
{
  /* Flush pending data */
  if(ifp->netbuf.pending)
    net_output(ifp);

  free(ifp->netbuf.buff);
  ifp->netbuf.buff = NULL;

  return 0;
}


/**
 * Reserve space in a outputbuffer. This should only be needed
 * in very special cases. This will decrease the reported size
 * of the buffer so that there is always <i>size</i> bytes
 * of data available in the buffer. To add data in the reserved
 * area one must use the net_outbuffer_push_reserved function.
 *
 * @param ifp the interface corresponding to the buffer
 * to reserve space on
 * @param size the number of bytes to reserve
 *
 * @return 0 on success, negative if there was not enough
 *  bytes to reserve
 */
int
net_reserve_bufspace(struct interface *ifp, int size)
{
  if(size > ifp->netbuf.maxsize)
    return -1;
  
  ifp->netbuf.reserved = size;
  ifp->netbuf.maxsize -= size;
  
  return 0;
}

/**
 * Returns the number of bytes pending in the buffer. That
 * is the number of bytes added but not sent.
 *
 * @param ifp the interface corresponding to the buffer
 *
 * @return the number of bytes currently pending
 */
olsr_u16_t
net_output_pending(struct interface *ifp)
{
  return ifp->netbuf.pending;
}



/**
 * Add data to a buffer.
 *
 * @param ifp the interface corresponding to the buffer
 * @param data a pointer to the data to add
 * @param size the number of byte to copy from data
 *
 * @return -1 if no buffer was found, 0 if there was not 
 *  enough room in buffer or the number of bytes added on 
 *  success
 */
int
net_outbuffer_push(struct interface *ifp, const void *data, const olsr_u16_t size)
{
  if((ifp->netbuf.pending + size) > ifp->netbuf.maxsize)
    return 0;

  memcpy(&ifp->netbuf.buff[ifp->netbuf.pending + OLSR_HEADERSIZE], data, size);
  ifp->netbuf.pending += size;

  return size;
}


/**
 * Add data to the reserved part of a buffer
 *
 * @param ifp the interface corresponding to the buffer
 * @param data a pointer to the data to add
 * @param size the number of byte to copy from data
 *
 * @return -1 if no buffer was found, 0 if there was not 
 *  enough room in buffer or the number of bytes added on 
 *  success
 */
int
net_outbuffer_push_reserved(struct interface *ifp, const void *data, const olsr_u16_t size)
{
  if((ifp->netbuf.pending + size) > (ifp->netbuf.maxsize + ifp->netbuf.reserved))
    return 0;

  memcpy(&ifp->netbuf.buff[ifp->netbuf.pending + OLSR_HEADERSIZE], data, size);
  ifp->netbuf.pending += size;

  return size;
}

/**
 * Report the number of bytes currently available in the buffer
 * (not including possible reserved bytes)
 *
 * @param ifp the interface corresponding to the buffer
 *
 * @return the number of bytes available in the buffer or
 */
int
net_outbuffer_bytes_left(const struct interface *ifp)
{
  return ifp->netbuf.maxsize - ifp->netbuf.pending;
}


/**
 * Add a packet transform function. Theese are functions
 * called just prior to sending data in a buffer.
 *
 * @param f the function pointer
 *
 * @returns 1
 */
int
add_ptf(packet_transform_function f)
{

  struct ptf *new_ptf;

  new_ptf = olsr_malloc(sizeof(struct ptf), "Add PTF");

  new_ptf->next = ptf_list;
  new_ptf->function = f;

  ptf_list = new_ptf;

  return 1;
}

/**
 * Remove a packet transform function
 *
 * @param f the function pointer
 *
 * @returns 1 if a functionpointer was removed
 *  0 if not
 */
int
del_ptf(packet_transform_function f)
{
  struct ptf *tmp_ptf, *prev;

  tmp_ptf = ptf_list;
  prev = NULL;

  while(tmp_ptf)
    {
      if(tmp_ptf->function == f)
	{
	  /* Remove entry */
	  if(prev == NULL)
	      ptf_list = tmp_ptf->next;
	  else
	      prev->next = tmp_ptf->next;
          free(tmp_ptf);
	  return 1;
	}
      prev = tmp_ptf;
      tmp_ptf = tmp_ptf->next;
    }

  return 0;
}

/**
 *Sends a packet on a given interface.
 *
 *@param ifp the interface to send on.
 *
 *@return negative on error
 */
int
net_output(struct interface *ifp)
{
  struct sockaddr_in *sin;  
  struct sockaddr_in dst;
  struct sockaddr_in6 *sin6;  
  struct sockaddr_in6 dst6;
  struct ptf *tmp_ptf_list;
  union olsr_packet *outmsg;
  int retval;

  sin = NULL;
  sin6 = NULL;

  if(!ifp->netbuf.pending)
    return 0;

  ifp->netbuf.pending += OLSR_HEADERSIZE;

  retval = ifp->netbuf.pending;

  outmsg = (union olsr_packet *)ifp->netbuf.buff;
  /* Add the Packet seqno */
  outmsg->v4.olsr_seqno = htons(ifp->olsr_seqnum++);
  /* Set the packetlength */
  outmsg->v4.olsr_packlen = htons(ifp->netbuf.pending);

  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IP version 4 */
      sin = (struct sockaddr_in *)&ifp->int_broadaddr;

      /* Copy sin */
      dst = *sin;
      sin = &dst;

      if (sin->sin_port == 0)
	sin->sin_port = htons(OLSRPORT);
    }
  else
    {
      /* IP version 6 */
      sin6 = (struct sockaddr_in6 *)&ifp->int6_multaddr;
      /* Copy sin */
      dst6 = *sin6;
      sin6 = &dst6;
    }

  /*
   *Call possible packet transform functions registered by plugins  
   */
  for (tmp_ptf_list = ptf_list; tmp_ptf_list != NULL; tmp_ptf_list = tmp_ptf_list->next)
    {
      tmp_ptf_list->function(ifp->netbuf.buff, &ifp->netbuf.pending);
    }

  /*
   *if the -dispout option was given
   *we print the content of the packets
   */
  if(disp_pack_out)
    print_olsr_serialized_packet(stdout, (union olsr_packet *)ifp->netbuf.buff, 
				 ifp->netbuf.pending, &ifp->ip_addr); 
  
  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IP version 4 */
      if(olsr_sendto(ifp->olsr_socket, 
                     ifp->netbuf.buff, 
		     ifp->netbuf.pending, 
		     MSG_DONTROUTE, 
		     (struct sockaddr *)sin, 
		     sizeof (*sin))
	 < 0)
	{
	  perror("sendto(v4)");
	  olsr_syslog(OLSR_LOG_ERR, "OLSR: sendto IPv4 %m");
	  retval = -1;
	}
    }
  else
    {
      /* IP version 6 */
      if(olsr_sendto(ifp->olsr_socket, 
		     ifp->netbuf.buff,
		     ifp->netbuf.pending, 
		     MSG_DONTROUTE, 
		     (struct sockaddr *)sin6, 
		     sizeof (*sin6))
	 < 0)
	{
	  perror("sendto(v6)");
	  olsr_syslog(OLSR_LOG_ERR, "OLSR: sendto IPv6 %m");
	  fprintf(stderr, "Socket: %d interface: %d\n", ifp->olsr_socket, ifp->if_nr);
	  fprintf(stderr, "To: %s (size: %d)\n", ip6_to_string(&sin6->sin6_addr), (int)sizeof(*sin6));
	  fprintf(stderr, "Outputsize: %d\n", ifp->netbuf.pending);
	  retval = -1;
	}
    }
  
  ifp->netbuf.pending = 0;

  // if we've just transmitted a TC message, let Dijkstra use the current
  // link qualities for the links to our neighbours

  olsr_update_dijkstra_link_qualities();
  lq_tc_pending = OLSR_FALSE;

  return retval;
}


/**
 * Create a IPv6 netmask based on a prefix length
 *
 * @param allocated address to build the netmask in
 * @param prefix the prefix length
 *
 * @returns 1 on success 0 on failure
 */
int
olsr_prefix_to_netmask(union olsr_ip_addr *adr, olsr_u16_t prefix)
{
  int p, i;

  if(adr == NULL)
    return 0;

  p = prefix;
  i = 0;

  memset(adr, 0, olsr_cnf->ipsize);

  for(;p > 0; p -= 8)
    {
      adr->v6.s6_addr[i] = (p < 8) ? 0xff ^ (0xff >> p) : 0xff;
      i++;
    }

#ifdef DEBUG
  OLSR_PRINTF(3, "Prefix %d = Netmask: %s\n", prefix, olsr_ip_to_string(adr));
#endif

  return 1;
}



/**
 * Calculate prefix length based on a netmask
 *
 * @param adr the address to use to calculate the prefix length
 *
 * @return the prefix length
 */
olsr_u16_t
olsr_netmask_to_prefix(union olsr_ip_addr *adr)
{
  olsr_u16_t prefix;
  int i, tmp;

  prefix = 0;

  for(i = 0; i < 16; i++)
    {
      if(adr->v6.s6_addr[i] == 0xff)
	{
	  prefix += 8;
	}
      else
	{
	  for(tmp = adr->v6.s6_addr[i];
	      tmp > 0;
	      tmp = (tmp << 1) & 0xff)
	    prefix++;
	}
    }

#ifdef DEBUG
  OLSR_PRINTF(3, "Netmask: %s = Prefix %d\n", olsr_ip_to_string(adr), prefix);
#endif

  return prefix;
}



/**
 *Converts a sockaddr struct to a string representing
 *the IP address from the sockaddr struct
 *
 *<b>NON REENTRANT!!!!</b>
 *
 *@param address_to_convert the sockaddr struct to "convert"
 *@return a char pointer to the string containing the IP
 */
char *
sockaddr_to_string(struct sockaddr *address_to_convert)
{
  struct sockaddr_in           *address;
  
  address=(struct sockaddr_in *)address_to_convert; 
  return(inet_ntoa(address->sin_addr));
  
}


/**
 *Converts the 32bit olsr_u32_t datatype to
 *a char array.
 *
 *<b>NON REENTRANT!!!!</b>
 *
 *@param address the olsr_u32_t to "convert"
 *@return a char pointer to the string containing the IP
 */

const char *
ip_to_string(const olsr_u32_t *address)
{
  struct in_addr in;
  in.s_addr=*address;
  return(inet_ntoa(in));
  
}

/**
 *Converts the 32bit olsr_u32_t datatype to
 *a char array.
 *
 *<b>NON REENTRANT</b>
 *
 *@param addr6 the address to "convert"
 *@return a char pointer to the string containing the IP
 */

const char *
ip6_to_string(const struct in6_addr *addr6)
{
  static char ipv6_buf[INET6_ADDRSTRLEN]; /* for address coversion */
  return inet_ntop(AF_INET6, addr6, ipv6_buf, sizeof(ipv6_buf));
}


const char *
olsr_ip_to_string(const union olsr_ip_addr *addr)
{
  static int index = 0;
  static char buff[4][INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN];
  const char *ret;
  
  if(olsr_cnf->ip_version == AF_INET)
    {
#if 0
      struct in_addr in;
      in.s_addr = addr->v4;
      ret = inet_ntop(AF_INET, &in, buff[index], sizeof(buff[index]));
#else
      ret = inet_ntop(AF_INET, &addr->v6, buff[index], sizeof(buff[index]));
#endif
    }
  else
    {
      /* IPv6 */
      ret = inet_ntop(AF_INET6, &addr->v6, buff[index], sizeof(buff[index]));
    }
  index = (index + 1) & 3;

  return ret;
}


void
olsr_add_invalid_address(union olsr_ip_addr *adr)
{
  struct deny_address_entry *new_entry;

  new_entry = olsr_malloc(sizeof(struct deny_address_entry), "Add deny address");

  new_entry->next = deny_entries;
  COPY_IP(&new_entry->addr, adr);

  deny_entries = new_entry;

  OLSR_PRINTF(1, "Added %s to IP deny set\n", olsr_ip_to_string(&new_entry->addr));
  return;
}

/**
 *Converts the 32bit olsr_u32_t datatype to
 *a char array.
 *
 *<b>NON REENTRANT</b>
 *
 *@param addr6 the address to "convert"
 *@return a char pointer to the string containing the IP
 */
olsr_bool
olsr_validate_address(union olsr_ip_addr *adr)
{
  struct deny_address_entry *deny_entry = deny_entries;

  while(deny_entry)
    {
      if(COMP_IP(adr, &deny_entry->addr))
	{
	  OLSR_PRINTF(1, "Validation of address %s failed!\n",
		      olsr_ip_to_string(adr));
	  return OLSR_FALSE;
	}

      deny_entry = deny_entry->next;
    }

  return OLSR_TRUE;
}
