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
 * $Id: ifnet.c,v 1.46 2007/03/14 14:01:14 bernd67 Exp $
 */


#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
#define ifr_netmask ifr_addr
#endif

#include "interfaces.h"
#include "ifnet.h"
#include "defs.h"
#include "olsr.h"
#include "net_os.h"
#include "net_olsr.h"
#include "socket_parser.h"
#include "parser.h"
#include "scheduler.h"
#include "generate_msg.h"
#include "mantissa.h"
#include "lq_packet.h"
#include "log.h"
#include "link_set.h"
#include <signal.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

int bufspace = 127*1024;	/* max. input buffer size to request */


int
set_flag(char *ifname, short flag)
{
  struct ifreq ifr;

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

  /* Get flags */
  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFFLAGS, &ifr) < 0) 
    {
      fprintf(stderr,"ioctl (get interface flags)");
      return -1;
    }

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  
  //printf("Setting flags for if \"%s\"\n", ifr.ifr_name);

  if(!(ifr.ifr_flags & (IFF_UP | IFF_RUNNING)))
    {
      /* Add UP */
      ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
      /* Set flags + UP */
      if(ioctl(olsr_cnf->ioctl_s, SIOCSIFFLAGS, &ifr) < 0)
	{
	  fprintf(stderr, "ERROR(%s): %s\n", ifr.ifr_name, strerror(errno));
	  return -1;
	}
    }
  return 1;

}




void
check_interface_updates(void *foo)
{
  struct olsr_if *tmp_if;

#ifdef DEBUG
  OLSR_PRINTF(3, "Checking for updates in the interface set\n")
#endif

  for(tmp_if = olsr_cnf->interfaces; tmp_if != NULL; tmp_if = tmp_if->next)
    {
      if(tmp_if->host_emul)
	continue;
      
      if(olsr_cnf->host_emul) /* XXX: TEMPORARY! */
	continue;

      if(!tmp_if->cnf->autodetect_chg) 
        {
#ifdef DEBUG
          /* Don't check this interface */
          OLSR_PRINTF(3, "Not checking interface %s\n", tmp_if->name)
#endif
          continue;
        }

      if(tmp_if->configured)
        {
          chk_if_changed(tmp_if);
        }
      else
        {
          chk_if_up(tmp_if, 3);
        }
    }

  return;
}

/**
 * Checks if an initialized interface is changed
 * that is if it has been set down or the address
 * has been changed.
 *
 *@param iface the olsr_if struct describing the interface
 */
int
chk_if_changed(struct olsr_if *iface)
{
  struct interface *ifp, *tmp_ifp;
  struct ifreq ifr;
  struct sockaddr_in6 tmp_saddr6;
  int if_changes;
  if_changes = 0;

#ifdef DEBUG
  OLSR_PRINTF(3, "Checking if %s is set down or changed\n", iface->name)
#endif

  if(iface->host_emul)
    return -1;

  ifp = iface->interf;

  if(ifp == NULL)
    {
      /* Should not happen */
      iface->configured = 0;
      return 0;
    }

  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, iface->name, IFNAMSIZ);


  /* Get flags (and check if interface exists) */
  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFFLAGS, &ifr) < 0) 
    {
      OLSR_PRINTF(3, "No such interface: %s\n", iface->name)
      goto remove_interface;
    }

  ifp->int_flags = ifr.ifr_flags;

  /*
   * First check if the interface is set DOWN
   */

  if ((ifp->int_flags & IFF_UP) == 0)
    {
      OLSR_PRINTF(1, "\tInterface %s not up - removing it...\n", iface->name)
      goto remove_interface;
    }

  /*
   * We do all the interface type checks over.
   * This is because the interface might be a PCMCIA card. Therefore
   * It might not be the same physical interface as we configured earlier.
   */

  /* Check broadcast */
  if ((olsr_cnf->ip_version == AF_INET) && 
      !iface->cnf->ipv4_broadcast.v4 && /* Skip if fixed bcast */ 
      (!(ifp->int_flags & IFF_BROADCAST))) 
    {
      OLSR_PRINTF(3, "\tNo broadcast - removing\n")
      goto remove_interface;
    }

  if (ifp->int_flags & IFF_LOOPBACK)
    {
      OLSR_PRINTF(3, "\tThis is a loopback interface - removing it...\n")
      goto remove_interface;
    }

  ifp->is_hcif = OLSR_FALSE;

  /* trying to detect if interface is wireless. */
  ifp->is_wireless = check_wireless_interface(ifr.ifr_name);

  /* Set interface metric */
  if(iface->cnf->weight.fixed)
    ifp->int_metric = iface->cnf->weight.value;
  else
    ifp->int_metric = calculate_if_metric(ifr.ifr_name);

  /* Get MTU */
  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFMTU, &ifr) < 0)
    ifp->int_mtu = 0;
  else
    {
      ifr.ifr_mtu -= (olsr_cnf->ip_version == AF_INET6) ? UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

      if(ifp->int_mtu != ifr.ifr_mtu)
	{
	  ifp->int_mtu = ifr.ifr_mtu;
	  /* Create new outputbuffer */
	  net_remove_buffer(ifp); /* Remove old */
	  net_add_buffer(ifp);
	}
    }


  /* Get interface index */
  ifp->if_index = if_nametoindex(ifr.ifr_name);

  /*
   * Now check if the IP has changed
   */
  
  /* IP version 6 */
  if(olsr_cnf->ip_version == AF_INET6)
    {
      /* Get interface address */
      
      if(get_ipv6_address(ifr.ifr_name, &tmp_saddr6, iface->cnf->ipv6_addrtype) <= 0)
	{
	  if(iface->cnf->ipv6_addrtype == IPV6_ADDR_SITELOCAL)
	    OLSR_PRINTF(3, "\tCould not find site-local IPv6 address for %s\n", ifr.ifr_name)
	  else
	    OLSR_PRINTF(3, "\tCould not find global IPv6 address for %s\n", ifr.ifr_name)
	  
	  
	  goto remove_interface;
	}
      
#ifdef DEBUG
      OLSR_PRINTF(3, "\tAddress: %s\n", ip6_to_string(&tmp_saddr6.sin6_addr))
#endif

      if(memcmp(&tmp_saddr6.sin6_addr, &ifp->int6_addr.sin6_addr, olsr_cnf->ipsize) != 0)
	{
	  OLSR_PRINTF(1, "New IP address for %s:\n", ifr.ifr_name)
	  OLSR_PRINTF(1, "\tOld: %s\n", ip6_to_string(&ifp->int6_addr.sin6_addr))
	  OLSR_PRINTF(1, "\tNew: %s\n", ip6_to_string(&tmp_saddr6.sin6_addr))

	  /* Check main addr */
	  if(memcmp(&olsr_cnf->main_addr, &tmp_saddr6.sin6_addr, olsr_cnf->ipsize) == 0)
	    {
	      /* Update main addr */
	      memcpy(&olsr_cnf->main_addr, &tmp_saddr6.sin6_addr, olsr_cnf->ipsize);
	    }

	  /* Update address */
	  memcpy(&ifp->int6_addr.sin6_addr, &tmp_saddr6.sin6_addr, olsr_cnf->ipsize);
	  memcpy(&ifp->ip_addr, &tmp_saddr6.sin6_addr, olsr_cnf->ipsize);

	  run_ifchg_cbs(ifp, IFCHG_IF_UPDATE);

	  return 1;	  	  
	}
      return 0;

    }
  else
  /* IP version 4 */
    {
      /* Check interface address (IPv4)*/
      if(ioctl(olsr_cnf->ioctl_s, SIOCGIFADDR, &ifr) < 0) 
	{
	  OLSR_PRINTF(1, "\tCould not get address of interface - removing it\n")
	  goto remove_interface;
	}

#ifdef DEBUG
      OLSR_PRINTF(3, "\tAddress:%s\n", sockaddr_to_string(&ifr.ifr_addr))
#endif

      if(memcmp(&((struct sockaddr_in *)&ifp->int_addr)->sin_addr.s_addr,
		&((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, 
		olsr_cnf->ipsize) != 0)
	{
	  /* New address */
	  OLSR_PRINTF(1, "IPv4 address changed for %s\n", ifr.ifr_name)
	  OLSR_PRINTF(1, "\tOld:%s\n", sockaddr_to_string(&ifp->int_addr))
	  OLSR_PRINTF(1, "\tNew:%s\n", sockaddr_to_string(&ifr.ifr_addr))

	  ifp->int_addr = ifr.ifr_addr;

	  if(memcmp(&olsr_cnf->main_addr, 
		    &ifp->ip_addr,
		    olsr_cnf->ipsize) == 0)
	    {
	      OLSR_PRINTF(1, "New main address: %s\n", sockaddr_to_string(&ifr.ifr_addr))
	      olsr_syslog(OLSR_LOG_INFO, "New main address: %s\n", sockaddr_to_string(&ifr.ifr_addr));
	      memcpy(&olsr_cnf->main_addr, 
		     &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, 
		     olsr_cnf->ipsize);
	    }

	  memcpy(&ifp->ip_addr, 
		 &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, 
		 olsr_cnf->ipsize);

	  if_changes = 1;
	}

      /* Check netmask */
      if (ioctl(olsr_cnf->ioctl_s, SIOCGIFNETMASK, &ifr) < 0) 
	{
	  olsr_syslog(OLSR_LOG_ERR, "%s: ioctl (get broadaddr)", ifr.ifr_name);
	  goto remove_interface;
	}

#ifdef DEBUG
      OLSR_PRINTF(3, "\tNetmask:%s\n", sockaddr_to_string(&ifr.ifr_netmask))
#endif

      if(memcmp(&((struct sockaddr_in *)&ifp->int_netmask)->sin_addr.s_addr,
		&((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr, 
		olsr_cnf->ipsize) != 0)
	{
	  /* New address */
	  OLSR_PRINTF(1, "IPv4 netmask changed for %s\n", ifr.ifr_name)
	  OLSR_PRINTF(1, "\tOld:%s\n", sockaddr_to_string(&ifp->int_netmask))
	  OLSR_PRINTF(1, "\tNew:%s\n", sockaddr_to_string(&ifr.ifr_netmask))

	  ifp->int_netmask = ifr.ifr_netmask;

	  if_changes = 1;
	}
      
      if(!iface->cnf->ipv4_broadcast.v4)
	{
	  /* Check broadcast address */      
	  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFBRDADDR, &ifr) < 0) 
	    {
	      olsr_syslog(OLSR_LOG_ERR, "%s: ioctl (get broadaddr)", ifr.ifr_name);
	      goto remove_interface;
	    }
	  
#ifdef DEBUG
	  OLSR_PRINTF(3, "\tBroadcast address:%s\n", sockaddr_to_string(&ifr.ifr_broadaddr))
#endif
	  
	  if(memcmp(&((struct sockaddr_in *)&ifp->int_broadaddr)->sin_addr.s_addr,
		    &((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr.s_addr, 
		    olsr_cnf->ipsize) != 0)
	    {
	      
	      /* New address */
	      OLSR_PRINTF(1, "IPv4 broadcast changed for %s\n", ifr.ifr_name)
	      OLSR_PRINTF(1, "\tOld:%s\n", sockaddr_to_string(&ifp->int_broadaddr))
	      OLSR_PRINTF(1, "\tNew:%s\n", sockaddr_to_string(&ifr.ifr_broadaddr))
	      
	      ifp->int_broadaddr = ifr.ifr_broadaddr;
	      if_changes = 1;
	    }            
	}
    }

  if(if_changes)
    run_ifchg_cbs(ifp, IFCHG_IF_UPDATE);

  return if_changes;


 remove_interface:
  OLSR_PRINTF(1, "Removing interface %s\n", iface->name)
  olsr_syslog(OLSR_LOG_INFO, "Removing interface %s\n", iface->name);

  del_if_link_entries(&ifp->ip_addr);

  /*
   *Call possible ifchange functions registered by plugins  
   */
  run_ifchg_cbs(ifp, IFCHG_IF_REMOVE);
  
  /* Dequeue */
  if(ifp == ifnet)
    {
      ifnet = ifp->int_next;
    }
  else
    {
      tmp_ifp = ifnet;
      while(tmp_ifp->int_next != ifp)
	{
	  tmp_ifp = tmp_ifp->int_next;
	}
      tmp_ifp->int_next = ifp->int_next;
    }


  /* Remove output buffer */
  net_remove_buffer(ifp);

  /* Check main addr */
  if(COMP_IP(&olsr_cnf->main_addr, &ifp->ip_addr))
    {
      if(ifnet == NULL)
	{
	  /* No more interfaces */
	  memset(&olsr_cnf->main_addr, 0, olsr_cnf->ipsize);
	  OLSR_PRINTF(1, "No more interfaces...\n")
	}
      else
	{
	  COPY_IP(&olsr_cnf->main_addr, &ifnet->ip_addr);
	  OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr))
	  olsr_syslog(OLSR_LOG_INFO, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr));
	}
    }


  /*
   * Deregister scheduled functions 
   */

  if (olsr_cnf->lq_level == 0)
    {
      olsr_remove_scheduler_event(&generate_hello, 
                                  ifp, 
                                  iface->cnf->hello_params.emission_interval, 
                                  0, 
                                  NULL);
      olsr_remove_scheduler_event(&generate_tc, 
                                  ifp, 
                                  iface->cnf->tc_params.emission_interval,
                                  0, 
                                  NULL);
    }

  else
    {
      olsr_remove_scheduler_event(&olsr_output_lq_hello, 
                                  ifp, 
                                  iface->cnf->hello_params.emission_interval, 
                                  0, 
                                  NULL);
      olsr_remove_scheduler_event(&olsr_output_lq_tc, 
                                  ifp, 
                                  iface->cnf->tc_params.emission_interval,
                                  0, 
                                  NULL);
    }

  olsr_remove_scheduler_event(&generate_mid, 
			      ifp, 
			      iface->cnf->mid_params.emission_interval,
			      0, 
			      NULL);
  olsr_remove_scheduler_event(&generate_hna, 
			      ifp, 
			      iface->cnf->hna_params.emission_interval,
			      0, 
			      NULL);



  iface->configured = 0;
  iface->interf = NULL;
  /* Close olsr socket */
  close(ifp->olsr_socket);
  remove_olsr_socket(ifp->olsr_socket, &olsr_input);

  /* Free memory */
  free(ifp->int_name);
  free(ifp);

  if((ifnet == NULL) && (!olsr_cnf->allow_no_interfaces))
    {
      OLSR_PRINTF(1, "No more active interfaces - exiting.\n")
      olsr_syslog(OLSR_LOG_INFO, "No more active interfaces - exiting.\n");
      olsr_cnf->exit_value = EXIT_FAILURE;
      kill(getpid(), SIGINT);
    }

  return 0;

}

/**
 * Initializes the special interface used in
 * host-client emulation
 */
int
add_hemu_if(struct olsr_if *iface)
{
  struct interface *ifp;
  union olsr_ip_addr null_addr;
  olsr_u32_t addr[4];

  if(!iface->host_emul)
    return -1;

  ifp = olsr_malloc(sizeof (struct interface), "Interface update 2");

  memset(ifp, 0, sizeof (struct interface));

  iface->configured = OLSR_TRUE;
  iface->interf = ifp;

  ifp->is_hcif = OLSR_TRUE;
  ifp->int_name = olsr_malloc(strlen("hcif01") + 1, "Interface update 3");
  ifp->int_metric = 0;

  strcpy(ifp->int_name, "hcif01");

  OLSR_PRINTF(1, "Adding %s(host emulation):\n", ifp->int_name)

  OLSR_PRINTF(1, "       Address:%s\n", olsr_ip_to_string(&iface->hemu_ip));

  OLSR_PRINTF(1, "       Index:%d\n", iface->index);

  OLSR_PRINTF(1, "       NB! This is a emulated interface\n       that does not exist in the kernel!\n");

  ifp->int_next = ifnet;
  ifnet = ifp;

  memset(&null_addr, 0, olsr_cnf->ipsize);
  if(COMP_IP(&null_addr, &olsr_cnf->main_addr))
    {
      COPY_IP(&olsr_cnf->main_addr, &iface->hemu_ip);
      OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr))
	olsr_syslog(OLSR_LOG_INFO, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr));
    }

  /* setting the interfaces number*/
  ifp->if_nr = iface->index;

  ifp->int_mtu = OLSR_DEFAULT_MTU;

  ifp->int_mtu -= (olsr_cnf->ip_version == AF_INET6) ? UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

  /* Set up buffer */
  net_add_buffer(ifp);


  if(olsr_cnf->ip_version == AF_INET)
    {
      struct sockaddr_in sin;

      memset(&sin, 0, sizeof(sin));

      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      sin.sin_port = htons(10150);
 
     /* IP version 4 */
      ifp->ip_addr.v4 = iface->hemu_ip.v4;

      memcpy(&((struct sockaddr_in *)&ifp->int_addr)->sin_addr, &iface->hemu_ip, olsr_cnf->ipsize);
      
      /*
       *We create one socket for each interface and bind
       *the socket to it. This to ensure that we can control
       *on what interface the message is transmitted
       */
      
      ifp->olsr_socket = gethemusocket(&sin);
      
      if (ifp->olsr_socket < 0)
	{
	  fprintf(stderr, "Could not initialize socket... exiting!\n\n");
	  olsr_syslog(OLSR_LOG_ERR, "Could not initialize socket... exiting!\n\n");
	  olsr_cnf->exit_value = EXIT_FAILURE;
	  kill(getpid(), SIGINT);
	}

    }
  else
    {
      /* IP version 6 */
      memcpy(&ifp->ip_addr, &iface->hemu_ip, olsr_cnf->ipsize);

#if 0      
      /*
       *We create one socket for each interface and bind
       *the socket to it. This to ensure that we can control
       *on what interface the message is transmitted
       */
      
      ifp->olsr_socket = gethcsocket6(&addrsock6, bufspace, ifp->int_name);
      
      join_mcast(ifp, ifp->olsr_socket);
      
      if (ifp->olsr_socket < 0)
	{
	  fprintf(stderr, "Could not initialize socket... exiting!\n\n");
	  olsr_syslog(OLSR_LOG_ERR, "Could not initialize socket... exiting!\n\n");
	  olsr_cnf->exit_value = EXIT_FAILURE;
	  kill(getpid(), SIGINT);
	}
      
#endif
    }

  /* Send IP as first 4/16 bytes on socket */
  memcpy(addr, iface->hemu_ip.v6.s6_addr, olsr_cnf->ipsize);
  addr[0] = htonl(addr[0]);
  addr[1] = htonl(addr[1]);
  addr[2] = htonl(addr[2]);
  addr[3] = htonl(addr[3]);

  if(send(ifp->olsr_socket, addr , olsr_cnf->ipsize, 0) != (int)olsr_cnf->ipsize)
    {
      fprintf(stderr, "Error sending IP!");
    }  
  
  /* Register socket */
  add_olsr_socket(ifp->olsr_socket, &olsr_input_hostemu);


  if (olsr_cnf->lq_level == 0)
    {
      olsr_register_scheduler_event(&generate_hello, 
                                    ifp, 
                                    iface->cnf->hello_params.emission_interval, 
                                    0, 
                                    NULL);
      olsr_register_scheduler_event(&generate_tc, 
                                    ifp, 
                                    iface->cnf->tc_params.emission_interval,
                                    0, 
                                    NULL);
    }

  else
    {
      olsr_register_scheduler_event(&olsr_output_lq_hello, 
                                    ifp, 
                                    iface->cnf->hello_params.emission_interval, 
                                    0, 
                                    NULL);
      olsr_register_scheduler_event(&olsr_output_lq_tc, 
                                    ifp, 
                                    iface->cnf->tc_params.emission_interval,
                                    0, 
                                    NULL);
    }

  olsr_register_scheduler_event(&generate_mid, 
				ifp, 
				iface->cnf->mid_params.emission_interval,
				0, 
				NULL);
  olsr_register_scheduler_event(&generate_hna, 
				ifp, 
				iface->cnf->hna_params.emission_interval,
				0, 
				NULL);

  /* Recalculate max jitter */

  if((olsr_cnf->max_jitter == 0) || 
     ((iface->cnf->hello_params.emission_interval / 4) < olsr_cnf->max_jitter))
    olsr_cnf->max_jitter = iface->cnf->hello_params.emission_interval / 4;

  /* Recalculate max topology hold time */
  if(olsr_cnf->max_tc_vtime < iface->cnf->tc_params.emission_interval)
    olsr_cnf->max_tc_vtime = iface->cnf->tc_params.emission_interval;

  ifp->hello_etime = iface->cnf->hello_params.emission_interval;
  ifp->valtimes.hello = double_to_me(iface->cnf->hello_params.validity_time);
  ifp->valtimes.tc = double_to_me(iface->cnf->tc_params.validity_time);
  ifp->valtimes.mid = double_to_me(iface->cnf->mid_params.validity_time);
  ifp->valtimes.hna = double_to_me(iface->cnf->hna_params.validity_time);

  return 1;
}

static char basename[32];
char* if_basename(char* name);
char* if_basename(char* name)
{
	char *p = strchr(name, ':');
	if (0 == p || p - name >= (int)(sizeof(basename) / sizeof(basename[0]) - 1)) return name;
	memcpy(basename, name, p - name);
	basename[p - name] = 0;
	return basename;
}

/**
 * Initializes a interface described by iface,
 * if it is set up and is of the correct type.
 *
 *@param iface the olsr_if struct describing the interface
 *@param so the socket to use for ioctls
 *
 */
int
chk_if_up(struct olsr_if *iface, int debuglvl)
{
  struct interface ifs, *ifp;
  struct ifreq ifr;
  union olsr_ip_addr null_addr;
#ifdef linux
  int precedence = IPTOS_PREC(olsr_cnf->tos);
  int tos_bits = IPTOS_TOS(olsr_cnf->tos);
#endif

  if(iface->host_emul)
    return -1;

  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, iface->name, IFNAMSIZ);

  OLSR_PRINTF(debuglvl, "Checking %s:\n", ifr.ifr_name)

  /* Get flags (and check if interface exists) */
  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFFLAGS, &ifr) < 0) 
    {
      OLSR_PRINTF(debuglvl, "\tNo such interface!\n")
      return 0;
    }

  memset(&ifs.netbuf, 0, sizeof(struct olsr_netbuf));
  ifs.int_flags = ifr.ifr_flags;      


  if ((ifs.int_flags & IFF_UP) == 0)
    {
      OLSR_PRINTF(debuglvl, "\tInterface not up - skipping it...\n")
      return 0;
    }

  /* Check broadcast */
  if ((olsr_cnf->ip_version == AF_INET) &&
      !iface->cnf->ipv4_broadcast.v4 && /* Skip if fixed bcast */ 
      (!(ifs.int_flags & IFF_BROADCAST))) 
    {
      OLSR_PRINTF(debuglvl, "\tNo broadcast - skipping\n")
      return 0;
    }


  if (ifs.int_flags & IFF_LOOPBACK)
    {
      OLSR_PRINTF(debuglvl, "\tThis is a loopback interface - skipping it...\n")
      return 0;
    }

  ifs.is_hcif = OLSR_FALSE;

  /* trying to detect if interface is wireless. */
  ifs.is_wireless = check_wireless_interface(ifr.ifr_name);

  if(ifs.is_wireless)
    OLSR_PRINTF(debuglvl, "\tWireless interface detected\n")
  else
    OLSR_PRINTF(debuglvl, "\tNot a wireless interface\n")

  
  /* IP version 6 */
  if(olsr_cnf->ip_version == AF_INET6)
    {
      /* Get interface address */
      
      if(get_ipv6_address(ifr.ifr_name, &ifs.int6_addr, iface->cnf->ipv6_addrtype) <= 0)
	{
	  if(iface->cnf->ipv6_addrtype == IPV6_ADDR_SITELOCAL)
	    OLSR_PRINTF(debuglvl, "\tCould not find site-local IPv6 address for %s\n", ifr.ifr_name)
	  else
	    OLSR_PRINTF(debuglvl, "\tCould not find global IPv6 address for %s\n", ifr.ifr_name)
	  
	  return 0;
	}
      
      OLSR_PRINTF(debuglvl, "\tAddress: %s\n", ip6_to_string(&ifs.int6_addr.sin6_addr))
      
      /* Multicast */
      ifs.int6_multaddr.sin6_addr = (iface->cnf->ipv6_addrtype == IPV6_ADDR_SITELOCAL) ? 
	iface->cnf->ipv6_multi_site.v6 :
	iface->cnf->ipv6_multi_glbl.v6;
      /* Set address family */
      ifs.int6_multaddr.sin6_family = AF_INET6;
      /* Set port */
      ifs.int6_multaddr.sin6_port = htons(OLSRPORT);
      
#ifdef __MacOSX__
      ifs.int6_multaddr.sin6_scope_id = 0;
#endif

      OLSR_PRINTF(debuglvl, "\tMulticast: %s\n", ip6_to_string(&ifs.int6_multaddr.sin6_addr))
      
    }
  /* IP version 4 */
  else
    {
      /* Get interface address (IPv4)*/
      if(ioctl(olsr_cnf->ioctl_s, SIOCGIFADDR, &ifr) < 0) 
	{
	  OLSR_PRINTF(debuglvl, "\tCould not get address of interface - skipping it\n")
	  return 0;
	}
      
      ifs.int_addr = ifr.ifr_addr;
      
      /* Find netmask */
      
      if (ioctl(olsr_cnf->ioctl_s, SIOCGIFNETMASK, &ifr) < 0) 
	{
	  olsr_syslog(OLSR_LOG_ERR, "%s: ioctl (get netmask)", ifr.ifr_name);
	  return 0;
	}
      
      ifs.int_netmask = ifr.ifr_netmask;
      
      /* Find broadcast address */
      if(iface->cnf->ipv4_broadcast.v4)
	{
	  /* Specified broadcast */
	  memset(&ifs.int_broadaddr, 0, sizeof(struct sockaddr));
	  memcpy(&((struct sockaddr_in *)&ifs.int_broadaddr)->sin_addr.s_addr, 
		 &iface->cnf->ipv4_broadcast.v4, 
		 sizeof(olsr_u32_t));
	}
      else
	{
	  /* Autodetect */
	  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFBRDADDR, &ifr) < 0) 
	    {
	      olsr_syslog(OLSR_LOG_ERR, "%s: ioctl (get broadaddr)", ifr.ifr_name);
	      return 0;
	    }
	  
	  ifs.int_broadaddr = ifr.ifr_broadaddr;
	}
      
      /* Deactivate IP spoof filter */
      deactivate_spoof(if_basename(ifr.ifr_name), &ifs, olsr_cnf->ip_version);
      
      /* Disable ICMP redirects */
      disable_redirects(if_basename(ifr.ifr_name), &ifs, olsr_cnf->ip_version);
      
    }
  
  
  /* Get interface index */
  
  ifs.if_index = if_nametoindex(ifr.ifr_name);
  
  /* Set interface metric */
  if(iface->cnf->weight.fixed)
    ifs.int_metric = iface->cnf->weight.value;
  else
    ifs.int_metric = calculate_if_metric(ifr.ifr_name);
  OLSR_PRINTF(1, "\tMetric: %d\n", ifs.int_metric)

  /* setting the interfaces number*/
  ifs.if_nr = iface->index;


  /* Get MTU */
  if (ioctl(olsr_cnf->ioctl_s, SIOCGIFMTU, &ifr) < 0)
    ifs.int_mtu = OLSR_DEFAULT_MTU;
  else
    ifs.int_mtu = ifr.ifr_mtu;

  ifs.int_mtu -= (olsr_cnf->ip_version == AF_INET6) ? UDP_IPV6_HDRSIZE : UDP_IPV4_HDRSIZE;

  ifs.ttl_index = 0;

  /* Set up buffer */
  net_add_buffer(&ifs);
	       
  OLSR_PRINTF(1, "\tMTU - IPhdr: %d\n", ifs.int_mtu)

  olsr_syslog(OLSR_LOG_INFO, "Adding interface %s\n", iface->name);
  OLSR_PRINTF(1, "\tIndex %d\n", ifs.if_nr)

  if(olsr_cnf->ip_version == AF_INET)
    {
      OLSR_PRINTF(1, "\tAddress:%s\n", sockaddr_to_string(&ifs.int_addr))
      OLSR_PRINTF(1, "\tNetmask:%s\n", sockaddr_to_string(&ifs.int_netmask))
      OLSR_PRINTF(1, "\tBroadcast address:%s\n", sockaddr_to_string(&ifs.int_broadaddr))
    }
  else
    {
      OLSR_PRINTF(1, "\tAddress: %s\n", ip6_to_string(&ifs.int6_addr.sin6_addr))
      OLSR_PRINTF(1, "\tMulticast: %s\n", ip6_to_string(&ifs.int6_multaddr.sin6_addr))
    }
  
  ifp = olsr_malloc(sizeof (struct interface), "Interface update 2");

  iface->configured = 1;
  iface->interf = ifp;
  
  memcpy(ifp, &ifs, sizeof(struct interface));
  
  ifp->gen_properties = NULL;
  ifp->int_name = olsr_malloc(strlen(ifr.ifr_name) + 1, "Interface update 3");
      
  strcpy(ifp->int_name, if_basename(ifr.ifr_name));
  /* Segfaults if using strncpy(IFNAMSIZ) why oh why?? */
  ifp->int_next = ifnet;
  ifnet = ifp;

  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IP version 4 */
      ifp->ip_addr.v4 = ((struct sockaddr_in *)&ifp->int_addr)->sin_addr.s_addr;
      /*
       *We create one socket for each interface and bind
       *the socket to it. This to ensure that we can control
       *on what interface the message is transmitted
       */
      
      ifp->olsr_socket = getsocket((struct sockaddr *)&addrsock, bufspace, ifp->int_name);
      
      if (ifp->olsr_socket < 0)
	{
	  fprintf(stderr, "Could not initialize socket... exiting!\n\n");
	  olsr_syslog(OLSR_LOG_ERR, "Could not initialize socket... exiting!\n\n");
	  olsr_cnf->exit_value = EXIT_FAILURE;
	  kill(getpid(), SIGINT);
	}

    }
  else
    {
      /* IP version 6 */
      memcpy(&ifp->ip_addr, &ifp->int6_addr.sin6_addr, olsr_cnf->ipsize);

      
      /*
       *We create one socket for each interface and bind
       *the socket to it. This to ensure that we can control
       *on what interface the message is transmitted
       */
      
      ifp->olsr_socket = getsocket6(&addrsock6, bufspace, ifp->int_name);
      
      join_mcast(ifp, ifp->olsr_socket);
      
      if (ifp->olsr_socket < 0)
	{
	  fprintf(stderr, "Could not initialize socket... exiting!\n\n");
	  olsr_syslog(OLSR_LOG_ERR, "Could not initialize socket... exiting!\n\n");
	  olsr_cnf->exit_value = EXIT_FAILURE;
	  kill(getpid(), SIGINT);
	}
      
    }

  set_buffer_timer(ifp);

  /* Register socket */
  add_olsr_socket(ifp->olsr_socket, &olsr_input);
  
#ifdef linux 
  /* Set TOS */
  
  if (setsockopt(ifp->olsr_socket, SOL_SOCKET, SO_PRIORITY, (char*)&precedence, sizeof(precedence)) < 0)
    {
      perror("setsockopt(SO_PRIORITY)");
      olsr_syslog(OLSR_LOG_ERR, "OLSRD: setsockopt(SO_PRIORITY) error %m");
    }
  if (setsockopt(ifp->olsr_socket, SOL_IP, IP_TOS, (char*)&tos_bits, sizeof(tos_bits)) < 0)    
    {
      perror("setsockopt(IP_TOS)");
      olsr_syslog(OLSR_LOG_ERR, "setsockopt(IP_TOS) error %m");
    }
#endif
  
  /*
   *Initialize sequencenumber as a random 16bit value
   */
  ifp->olsr_seqnum = random() & 0xFFFF;

  /*
   * Set main address if this is the only interface
   */
  memset(&null_addr, 0, olsr_cnf->ipsize);
  if(COMP_IP(&null_addr, &olsr_cnf->main_addr))
    {
      COPY_IP(&olsr_cnf->main_addr, &ifp->ip_addr);
      OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr))
      olsr_syslog(OLSR_LOG_INFO, "New main address: %s\n", olsr_ip_to_string(&olsr_cnf->main_addr));
    }
  
  /*
   * Register scheduled functions 
   */

  if (olsr_cnf->lq_level == 0)
    {
      olsr_register_scheduler_event(&generate_hello, 
                                    ifp, 
                                    iface->cnf->hello_params.emission_interval, 
                                    0, 
                                    NULL);
      olsr_register_scheduler_event(&generate_tc, 
                                    ifp, 
                                    iface->cnf->tc_params.emission_interval,
                                    0, 
                                    NULL);
    }

  else
    {
      olsr_register_scheduler_event(&olsr_output_lq_hello, 
                                    ifp, 
                                    iface->cnf->hello_params.emission_interval, 
                                    0, 
                                    NULL);
      olsr_register_scheduler_event(&olsr_output_lq_tc, 
                                    ifp, 
                                    iface->cnf->tc_params.emission_interval,
                                    0, 
                                    NULL);
    }

  olsr_register_scheduler_event(&generate_mid, 
				ifp, 
				iface->cnf->mid_params.emission_interval,
				0, 
				NULL);
  olsr_register_scheduler_event(&generate_hna, 
				ifp, 
				iface->cnf->hna_params.emission_interval,
				0, 
				NULL);

  /* Recalculate max jitter */

  if((olsr_cnf->max_jitter == 0) || 
     ((iface->cnf->hello_params.emission_interval / 4) < olsr_cnf->max_jitter))
    olsr_cnf->max_jitter = iface->cnf->hello_params.emission_interval / 4;

  /* Recalculate max topology hold time */
  if(olsr_cnf->max_tc_vtime < iface->cnf->tc_params.emission_interval)
    olsr_cnf->max_tc_vtime = iface->cnf->tc_params.emission_interval;

  ifp->hello_etime = iface->cnf->hello_params.emission_interval;
  ifp->valtimes.hello = double_to_me(iface->cnf->hello_params.validity_time);
  ifp->valtimes.tc = double_to_me(iface->cnf->tc_params.validity_time);
  ifp->valtimes.mid = double_to_me(iface->cnf->mid_params.validity_time);
  ifp->valtimes.hna = double_to_me(iface->cnf->hna_params.validity_time);


  /*
   *Call possible ifchange functions registered by plugins  
   */
  run_ifchg_cbs(ifp, IFCHG_IF_ADD);

  return 1;
}


