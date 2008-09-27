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
 */


#if 0 /* DEPRECATED - KEPT FOR REFERENCE */

/* Ugly fix to make this compile on wireless extentions < 16 */
#define _LINUX_ETHTOOL_H

#include "../link_layer.h"
#include "../olsr_protocol.h"
#include "../scheduler.h"
#include "../interfaces.h"
#include <linux/wireless.h>
#include <linux/icmp.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <stdlib.h>

#include "olsr_protocol.h"

void
init_link_layer_notification(void);

void
poll_link_layer(void *);

int
add_spy_node(union olsr_ip_addr *, char *);

#define	MAXIPLEN	60
#define	MAXICMPLEN	76

float poll_int = 200; /* msec */

int
iw_get_range_info(char *, struct iw_range *);

int
clear_spy_list(char *);

int
convert_ip_to_mac(union olsr_ip_addr *, struct sockaddr *, char *);

void
send_ping(union olsr_ip_addr *);


void
init_link_layer_notification()
{
  struct interface *ifd;

  OLSR_PRINTF(1, "Initializing link-layer notification...\n");


  for (ifd = ifnet; ifd ; ifd = ifd->int_next) 
    {
      if(ifd->is_wireless)
	clear_spy_list(ifd->int_name);
    }

  olsr_start_timer(poll_int, 0, OLSR_TIMER_PERIODIC, &poll_link_layer, NULL, 0);

  return;
}

int
clear_spy_list(char *ifname)
{
  struct iwreq	wrq;

  /* Time to do send addresses to the driver */
  wrq.u.data.pointer = NULL;//(caddr_t) hw_address;
  wrq.u.data.length = 0;
  wrq.u.data.flags = 0;

  /* Set device name */
  strscpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name));

  if(ioctl(olsr_cnf->ioctl_s, SIOCSIWSPY, &wrq) < 0)
    {
      OLSR_PRINTF(1, "Could not clear spylist %s\n", strerror(errno));
      return -1;
    }

  return 1;
}



int
add_spy_node(union olsr_ip_addr *addr, char *interface)
{
  struct sockaddr       new_node;
  struct iwreq		wrq;
  int			nbr;		/* Number of valid addresses */
  struct sockaddr	hw_address[IW_MAX_SPY];
  char	buffer[(sizeof(struct iw_quality) +
		sizeof(struct sockaddr)) * IW_MAX_SPY];
  
  OLSR_PRINTF(1, "Adding spynode!\n\n");
  
  /* get all addresses already in the driver */

  wrq.u.data.pointer = (caddr_t) buffer;
  wrq.u.data.length = IW_MAX_SPY;
  wrq.u.data.flags = 0;

  strscpy(wrq.ifr_name, interface, sizeof(wrq.ifr_name));

  if(ioctl(olsr_cnf->ioctl_s, SIOCGIWSPY, &wrq) < 0)
    {
      OLSR_PRINTF(1, "Could not get old spylist %s\n", strerror(errno));
      return 0;
    }

  /* Copy old addresses */
  nbr = wrq.u.data.length;
  memcpy(hw_address, buffer, nbr * sizeof(struct sockaddr));

  OLSR_PRINTF(1, "Old addresses: %d\n\n", nbr);

  /* Check upper limit */
  if(nbr >= IW_MAX_SPY)
    return 0;

  /* Add new address if MAC exists in ARP cache */
  if(convert_ip_to_mac(addr, &new_node, interface) > 0)
    {
      memcpy(&hw_address[nbr], &new_node, sizeof(struct sockaddr));
      nbr++;
    }
  else
    return 0;
  
  /* Add all addresses */
  wrq.u.data.pointer = (caddr_t) hw_address;
  wrq.u.data.length = nbr;
  wrq.u.data.flags = 0;
  
  /* Set device name */
  strscpy(wrq.ifr_name, interface, sizeof(wrq.ifr_name));
  
  if(ioctl(olsr_cnf->ioctl_s, SIOCSIWSPY, &wrq) < 0)
    {
      OLSR_PRINTF(1, "Could not clear spylist %s\n", strerror(errno));
      return 0;
    }


  return 1;
}


int
convert_ip_to_mac(union olsr_ip_addr *ip, struct sockaddr *mac, char *interface)
{
  struct arpreq	arp_query;
  struct sockaddr_in tmp_sockaddr;


  memset(&arp_query, 0, sizeof(struct arpreq));

  OLSR_PRINTF(1, "\nARP conversion for %s interface %s\n", 
	      olsr_ip_to_string(ip),
	      interface);

  tmp_sockaddr.sin_family = AF_INET;
  tmp_sockaddr.sin_port = 0;

  memcpy(&tmp_sockaddr.sin_addr, ip, olsr_cnf->ipsize);

  /* Translate IP addresses to MAC addresses */
  memcpy(&arp_query.arp_pa, &tmp_sockaddr, sizeof(struct sockaddr_in));
  arp_query.arp_ha.sa_family = 0;
  arp_query.arp_flags = 0;

  strscpy(arp_query.arp_dev, interface, sizeof(arp_query.arp_dev));
  
  if((ioctl(olsr_cnf->ioctl_s, SIOCGARP, &arp_query) < 0) ||
     !(arp_query.arp_flags & ATF_COM)) /* ATF_COM - hw addr valid */
    {
      OLSR_PRINTF(1, "Arp failed: (%s) - trying lookup\n", strerror(errno));

      /* No address - create a thread that sends a PING */
      send_ping(ip);
  
      return -1;
    }

  OLSR_PRINTF(1, "Arp success!\n");

  memcpy(mac, &arp_query.arp_ha, sizeof(struct sockaddr));

  return 1;
}



/**
 *A thread that sends a ICMP echo "ping" packet
 *to a given destination to force the ARP cache
 *to be updated... kind of a kludge....
 *
 *@param _ip the IP address to ping
 */
/* ONLY IPv4 FOR NOW!!! */

void
send_ping(union olsr_ip_addr *ip)
{
  int ping_s;
  struct sockaddr dst;
  struct sockaddr_in *dst_in;
  char *packet;
  struct icmphdr *icp;

  dst_in = (struct sockaddr_in *) &dst;

  dst_in->sin_family = AF_INET;
  memcpy(&dst_in->sin_addr, ip, olsr_cnf->ipsize);

  OLSR_PRINTF(1, "pinging %s\n\n", olsr_ip_to_string(ip));

  if ((ping_s = socket(AF_INET, SOCK_RAW, PF_INET)) < 0) 
    {
      OLSR_PRINTF(1, "Could not create RAW socket for ping!\n%s\n", strerror(errno));
      return;
    }

  /* Create packet */
  packet = malloc(MAXIPLEN + MAXICMPLEN);
  
  
  icp = (struct icmphdr *)packet;
  icp->type = ICMP_ECHO;
  icp->code = 0;
  icp->checksum = 0;
  icp->un.echo.sequence = 1;
  icp->un.echo.id = getpid() & 0xFFFF;

  if((sendto(ping_s, packet, MAXIPLEN + MAXICMPLEN + 8, 0, &dst, sizeof(struct sockaddr))) !=
     MAXIPLEN + MAXICMPLEN + 8)
    {
      OLSR_PRINTF(1, "Error PING: %s\n", strerror(errno));
    }

  /* Nevermind the pong ;-) */

  OLSR_PRINTF(1, "Ping complete...\n");
  close(ping_s);

  free(packet);

  return;
}

void
poll_link_layer(void *foo)
{
  struct iwreq		wrq;
  char		        buffer[(sizeof(struct iw_quality) +
			       sizeof(struct sockaddr)) * IW_MAX_SPY];
  struct sockaddr       *hwa;
  struct iw_quality     *qual;
  int		        n;
  struct iw_range	range;
  int		        i, j;
  int                   has_range = 0;
  struct interface      *iflist;

  //OLSR_PRINTF(1, "Polling link-layer notification...\n");

  for(iflist = ifnet; iflist != NULL; iflist = iflist->int_next)
    {
      if(!iflist->is_wireless)
	continue;

      /* Collect stats */
      wrq.u.data.pointer = (caddr_t) buffer;
      wrq.u.data.length = IW_MAX_SPY;
      wrq.u.data.flags = 0;
      
      /* Set device name */
      strscpy(wrq.ifr_name, iflist->int_name, sizeof(wrq.ifr_name));
      
      /* Do the request */
      if(ioctl(olsr_cnf->ioctl_s, SIOCGIWSPY, &wrq) < 0)
	{
          OLSR_PRINTF(1, "%-8.16s  Interface doesn't support wireless statistic collection\n\n", iflist->int_name);
	  return;
	}
      
      /* Get range info if we can */
      if(iw_get_range_info(iflist->int_name, &(range)) >= 0)
	has_range = 1;
      
      /* Number of addresses */
      n = wrq.u.data.length;
      
      /* The two lists */
      hwa = (struct sockaddr *) buffer;
      qual = (struct iw_quality *) (buffer + (sizeof(struct sockaddr) * n));
      
      for(i = 0; i < n; i++)
	{
	  if(!(qual->updated & 0x7))
	    continue;
	  
	  /* Print stats for each address */
	  OLSR_PRINTF(1, "MAC");
	  for(j = 0; j < 6; j++)
	    {
                    OLSR_PRINTF(1, ":%02x", (hwa[i].sa_data[j] % 0xffffff00));
	    }
	  if(!has_range)
	    OLSR_PRINTF(1, " : Quality:%d  Signal level:%d dBm  Noise level:%d dBm",
			qual[i].qual,
			qual[i].level - 0x100, 
			qual[i].noise - 0x100);
	  else
	    OLSR_PRINTF(1, " : Quality:%d/%d  Signal level:%d dBm  Noise level:%d dBm",
			qual[i].qual,
			range.max_qual.qual,
			qual[i].level - 0x100, 
			qual[i].noise - 0x100);
	  
	  OLSR_PRINTF(1, "\n");
	  
	}
    }

  //OLSR_PRINTF(1, "\n");
  return;
}





/*
 * Get the range information out of the driver
 */
int
iw_get_range_info(char            *ifname,
		  struct iw_range *range)
{
  struct iwreq		wrq;
  char			buffer[sizeof(struct iw_range) * 2];	/* Large enough */
  union iw_range_raw    *range_raw;

  /* Cleanup */
  bzero(buffer, sizeof(buffer));

  wrq.u.data.pointer = (caddr_t) buffer;
  wrq.u.data.length = sizeof(buffer);
  wrq.u.data.flags = 0;

  /* Set device name */
  strscpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name));

  if(ioctl(olsr_cnf->ioctl_s, SIOCGIWRANGE, &wrq) < 0)
    {
      OLSR_PRINTF(1, "NO RANGE\n");
      return -1;
    }

  /* Point to the buffer */
  range_raw = (union iw_range_raw *) buffer;

  memcpy((char *) range, buffer, sizeof(struct iw_range));

  return 1;
}


#endif
