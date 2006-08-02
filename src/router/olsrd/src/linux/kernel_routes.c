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
 * $Id: kernel_routes.c,v 1.18 2005/02/27 18:39:43 kattemat Exp $
 */



#include "kernel_routes.h"
#include "link_set.h"
#include "olsr.h"
#include "log.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>



static struct sockaddr_in6 null_addr6; /* Address used as Originator Address IPv6 */

/**
 *Insert a route in the kernel routing table
 *
 *@param destination the route to add
 *
 *@return negative on error
 */
int
olsr_ioctl_add_route(struct rt_entry *destination)
{
  struct rtentry kernel_route;
  int tmp;
  char dst_str[16], mask_str[16], router_str[16];

  inet_ntop(AF_INET, &destination->rt_dst.v4, dst_str, 16);
  inet_ntop(AF_INET, &destination->rt_mask.v4, mask_str, 16);
  inet_ntop(AF_INET, &destination->rt_router.v4, router_str, 16);

  OLSR_PRINTF(1, "(ioctl)Adding route with metric %d to %s/%s via %s/%s.\n",
              destination->rt_metric, dst_str, mask_str, router_str,
              destination->rt_if->int_name)
  
  memset(&kernel_route, 0, sizeof(struct rtentry));

  ((struct sockaddr_in*)&kernel_route.rt_dst)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_gateway)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_genmask)->sin_family = AF_INET;

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = destination->rt_dst.v4;
  ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = destination->rt_mask.v4;

  if(destination->rt_dst.v4 != destination->rt_router.v4)
    {
      ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr.s_addr=destination->rt_router.v4;
    }

  kernel_route.rt_flags = destination->rt_flags;
  
  kernel_route.rt_metric = destination->rt_metric + 1;

  /* 
   * Thales Internet GW fix
   */

  if((del_gws) &&
     (destination->rt_dst.v4 == INADDR_ANY) &&
     (destination->rt_dst.v4 == INADDR_ANY))
    {
      delete_all_inet_gws();
      del_gws = 0;
    }

  /*
   * Set interface
   */
  if((kernel_route.rt_dev = malloc(strlen(destination->rt_if->int_name) + 1)) == 0)
    {
      fprintf(stderr, "Out of memory!\n%s\n", strerror(errno));
      olsr_exit(__func__, EXIT_FAILURE);
    }

  strcpy(kernel_route.rt_dev, destination->rt_if->int_name);

  
  //printf("Inserting route entry on device %s\n\n", kernel_route.rt_dev);
  
  /*
  printf("Adding route:\n\tdest: %s\n", olsr_ip_to_string(&destination->rt_dst));    
  printf("\trouter: %s\n", olsr_ip_to_string(&destination->rt_router));    
  printf("\tmask: %s\n", olsr_ip_to_string((union olsr_ip_addr *)&destination->rt_mask));    
  printf("\tmetric: %d\n", destination->rt_metric);    
  */

  //printf("\tiface: %s\n", kernel_route.rt_dev);    
  
  tmp = ioctl(ioctl_s,SIOCADDRT,&kernel_route);
  /*  kernel_route.rt_dev=*/

  /*
   *Send IPC route update message
   */
  
  if(olsr_cnf->open_ipc)
      {
	ipc_route_send_rtentry(&destination->rt_dst, 
			       &destination->rt_router, 
			       destination->rt_metric, 
			       1,
			       destination->rt_if->int_name); /* Send interface name */
      }
  
  
  if (ifnet && kernel_route.rt_dev)
    {
      free(kernel_route.rt_dev);
    }
  
  
  return tmp;
}




/**
 *Insert a route in the kernel routing table
 *
 *@param destination the route to add
 *
 *@return negative on error
 */
int
olsr_ioctl_add_route6(struct rt_entry *destination)
{

  struct in6_rtmsg kernel_route;
  int tmp;
  struct in6_addr zeroaddr;

  OLSR_PRINTF(2, "(ioctl)Adding route: %s(hopc %d)\n", 
	      olsr_ip_to_string(&destination->rt_dst), 
	      destination->rt_metric + 1)
  

  memset(&zeroaddr, 0, ipsize); /* Use for comparision */


  memset(&kernel_route, 0, sizeof(struct in6_rtmsg));

  COPY_IP(&kernel_route.rtmsg_dst, &destination->rt_dst);

  kernel_route.rtmsg_flags = destination->rt_flags;
  kernel_route.rtmsg_metric = destination->rt_metric;
  
  kernel_route.rtmsg_dst_len = destination->rt_mask.v6;

  if(memcmp(&destination->rt_dst, &destination->rt_router, ipsize) != 0)
    {
      COPY_IP(&kernel_route.rtmsg_gateway, &destination->rt_router);
    }
  else
    {
      COPY_IP(&kernel_route.rtmsg_gateway, &destination->rt_dst);
    }

      /*
       * set interface
       */
  kernel_route.rtmsg_ifindex = destination->rt_if->if_index;


  
  //OLSR_PRINTF(3, "Adding route to %s using gw ", olsr_ip_to_string((union olsr_ip_addr *)&kernel_route.rtmsg_dst))
  //OLSR_PRINTF(3, "%s\n", olsr_ip_to_string((union olsr_ip_addr *)&kernel_route.rtmsg_gateway))

  if((tmp = ioctl(ioctl_s, SIOCADDRT, &kernel_route)) >= 0)
    {
      if(olsr_cnf->open_ipc)
	{
	  if(memcmp(&destination->rt_router, &null_addr6, ipsize) != 0)
	    ipc_route_send_rtentry(&destination->rt_dst, 
				   &destination->rt_router, 
				   destination->rt_metric, 
				   1,
				   destination->rt_if->int_name); /* Send interface name */

	}
    }
    return(tmp);
}



/**
 *Remove a route from the kernel
 *
 *@param destination the route to remove
 *
 *@return negative on error
 */
int
olsr_ioctl_del_route(struct rt_entry *destination)
{
  struct rtentry kernel_route;
  int tmp;
  char dst_str[16], mask_str[16], router_str[16];

  inet_ntop(AF_INET, &destination->rt_dst.v4, dst_str, 16);
  inet_ntop(AF_INET, &destination->rt_mask.v4, mask_str, 16);
  inet_ntop(AF_INET, &destination->rt_router.v4, router_str, 16);

  OLSR_PRINTF(1, "(ioctl)Deleting route with metric %d to %s/%s via %s/%s.\n",
              destination->rt_metric, dst_str, mask_str, router_str,
              destination->rt_if->int_name)
  
  memset(&kernel_route,0,sizeof(struct rtentry));

  ((struct sockaddr_in*)&kernel_route.rt_dst)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_gateway)->sin_family = AF_INET;
  ((struct sockaddr_in*)&kernel_route.rt_genmask)->sin_family = AF_INET;

  ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = destination->rt_dst.v4;
  if(destination->rt_dst.v4 != destination->rt_router.v4)
    ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr.s_addr = destination->rt_router.v4;
  ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = destination->rt_mask.v4;


  kernel_route.rt_dev = NULL;

  kernel_route.rt_flags = destination->rt_flags;
  
  kernel_route.rt_metric = destination->rt_metric + 1;

  /*
  printf("Deleteing route:\n\tdest: %s\n", olsr_ip_to_string(&destination->rt_dst));    
  printf("\trouter: %s\n", olsr_ip_to_string(&destination->rt_router));    
  printf("\tmask: %s\n", olsr_ip_to_string((union olsr_ip_addr *)&destination->rt_mask));    
  printf("\tmetric: %d\n", destination->rt_metric);    
  //printf("\tiface: %s\n", kernel_route.rt_dev);    
  */

  tmp = ioctl(ioctl_s, SIOCDELRT, &kernel_route);


    /*
     *Send IPC route update message
     */

  if(olsr_cnf->open_ipc)
    ipc_route_send_rtentry(&destination->rt_dst, 
			   NULL, 
			   destination->rt_metric, 
			   0,
			   NULL); /* Send interface name */

  return tmp;
}






/**
 *Remove a route from the kernel
 *
 *@param destination the route to remove
 *
 *@return negative on error
 */
int
olsr_ioctl_del_route6(struct rt_entry *destination)
{

  struct in6_rtmsg kernel_route;
  int tmp;

  union olsr_ip_addr tmp_addr = destination->rt_dst;

  OLSR_PRINTF(2, "(ioctl)Deleting route: %s(hopc %d)\n", 
	      olsr_ip_to_string(&destination->rt_dst), 
	      destination->rt_metric)


  OLSR_PRINTF(1, "Deleting route: %s\n", olsr_ip_to_string(&tmp_addr))

  memset(&kernel_route,0,sizeof(struct in6_rtmsg));

  kernel_route.rtmsg_dst_len = destination->rt_mask.v6;

  memcpy(&kernel_route.rtmsg_dst, &destination->rt_dst, ipsize);

  memcpy(&kernel_route.rtmsg_gateway, &destination->rt_router, ipsize);

  kernel_route.rtmsg_flags = destination->rt_flags;
  kernel_route.rtmsg_metric = destination->rt_metric;


  tmp = ioctl(ioctl_s, SIOCDELRT,&kernel_route);


    /*
     *Send IPC route update message
     */

  if(olsr_cnf->open_ipc)
    ipc_route_send_rtentry(&destination->rt_dst, 
			   NULL, 
			   destination->rt_metric, 
			   0,
			   NULL); /* Send interface name */

  return tmp;
}



int
delete_all_inet_gws()
{
  struct rtentry kernel_route;
  
  int s;
  char buf[BUFSIZ], *cp, *cplim;
  struct ifconf ifc;
  struct ifreq *ifr;
  
  OLSR_PRINTF(1, "Internet gateway detected...\nTrying to delete default gateways\n")
  
  /* Get a socket */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      olsr_syslog(OLSR_LOG_ERR, "socket: %m");
      close(s);
      return -1;
    }
  
  ifc.ifc_len = sizeof (buf);
  ifc.ifc_buf = buf;
  if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) 
    {
      olsr_syslog(OLSR_LOG_ERR, "ioctl (get interface configuration)");
      close(s);
      return -1;
    }

  ifr = ifc.ifc_req;
#define size(p) (sizeof (p))
  cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
  for (cp = buf; cp < cplim;cp += sizeof (ifr->ifr_name) + size(ifr->ifr_addr)) 
    {
      ifr = (struct ifreq *)cp;
      
      
      if(strcmp(ifr->ifr_ifrn.ifrn_name, "lo") == 0)
	{
	  OLSR_PRINTF(1, "Skipping loopback...\n")
	  continue;
	}

      OLSR_PRINTF(1, "Trying 0.0.0.0/0 %s...", ifr->ifr_ifrn.ifrn_name)
      
      
      memset(&kernel_route,0,sizeof(struct rtentry));
      
      ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_addr.s_addr = 0;
      ((struct sockaddr_in *)&kernel_route.rt_dst)->sin_family=AF_INET;
      ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_addr.s_addr = 0;
      ((struct sockaddr_in *)&kernel_route.rt_genmask)->sin_family=AF_INET;

      ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_addr.s_addr = INADDR_ANY;
      ((struct sockaddr_in *)&kernel_route.rt_gateway)->sin_family=AF_INET;
      
      //memcpy(&kernel_route.rt_gateway, gw, ipsize);
      
	   
	   
      kernel_route.rt_flags = RTF_UP | RTF_GATEWAY;
	   
	   
      if((kernel_route.rt_dev = malloc(6)) == 0)
	{
	  fprintf(stderr, "Out of memory!\n%s\n", strerror(errno));
	  olsr_exit(__func__, EXIT_FAILURE);
	}
	   
      strncpy(kernel_route.rt_dev, ifr->ifr_ifrn.ifrn_name, 6);

  
      //printf("Inserting route entry on device %s\n\n", kernel_route.rt_dev);
      
      if((ioctl(s, SIOCDELRT, &kernel_route)) < 0)
	OLSR_PRINTF(1, "NO\n")
      else
	OLSR_PRINTF(1, "YES\n")


      free(kernel_route.rt_dev);
      
    }
  
  close(s);
  
  return 0;
       
}
