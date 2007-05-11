#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <linux/ip.h>
#include <linux/igmp.h>
#include <linux/mroute.h>
#include <net/if.h>
#include <byteswap.h>
#include "igmprt.h"
 int log_level = LOG_INFO;	//LOG_INFO=6
//int log_level = LOG_DEBUG;            //LOG_DEBUG=7
int pipefds[2];
volatile int igmprt_running = 0;
igmp_router_t router;

#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
static int bFirstStart = 1;

#endif	/*  */
  void
igmp_add_membership (igmp_router_t * igmprt, igmp_group_t * newgrp) 
{
  unsigned char network_list[MAXVIFS];
  igmp_interface_t * ifp;
  igmp_group_t * grp;
  struct ip_mreqn mreq;
  struct mfcctl mfcctrl;
  memset ((void *) &network_list, 0xFF, sizeof (network_list));
  mreq.imr_ifindex = igmprt->upstream_interface->if_index;
  mreq.imr_multiaddr.s_addr = newgrp->group_addr;
  mreq.imr_address.s_addr =
    htonl (igmprt->upstream_interface->ip_addr.s_addr);
  if (setsockopt
       (igmprt->upstream_interface->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
	(void *) &mreq, sizeof (mreq)) < 0)
    {
      log (LOG_INFO, "setsockopt error: IP_ADD_MEMBERSHIP\n");
      return;
    }
  ifp =
    (igmp_interface_t *) list_get_head ((set_t **) & igmprt->network_if_list);
  while (ifp != NULL)
    {
      if (ifp->mode == UPSTREAM_INTERFACE)
	{
	  ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);	//next interface
	  continue;
	}
      grp =
	(igmp_group_t *) list_get_head ((set_t **) & ifp->group_database);
      while (grp != NULL)
	{
	  if (grp->group_addr == newgrp->group_addr)
	    {			//group exist
	      network_list[ifp->vif_index] = 1;
	      break;
	    }
	  grp = (igmp_group_t *) list_get_next ((set_t *) grp);	//next group
	}
      ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);	//next interface
    }
  memset ((void *) &mfcctrl, 0, sizeof (mfcctrl));
  mfcctrl.mfcc_origin.s_addr = 0;
  mfcctrl.mfcc_mcastgrp.s_addr = newgrp->group_addr;
  mfcctrl.mfcc_parent = igmprt->upstream_interface->vif_index;
  memcpy ((void *) &mfcctrl.mfcc_ttls[0], (void *) &network_list[0],
	   sizeof (network_list));
  if (setsockopt
       (igmprt->igmp_socket, IPPROTO_IP, MRT_ADD_MFC, (char *) &mfcctrl,
	sizeof (mfcctrl)) < 0)
    {
      log (LOG_INFO, "setsockopt error: MRT_ADD_MFC\n");
      return;
    }
  return;
}


/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
//void igmp_drop_membership(igmp_router_t *igmprt, igmp_group_t *igmpgrp)
  void
igmp_drop_membership (igmp_router_t * igmprt, igmp_group_t * igmpgrp,
		      igmp_interface_t * ptr_add_net, int both_flag) 
{
  igmp_interface_t * ifp;
  igmp_group_t * grp;
  int network_list[MAXVIFS];
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    // add the no_drop_num_0  and the no_drop_num_1
  int no_drop_num_0 = 1, no_drop_num_1 = 1;
  int drop_membership = 1;
  struct ip_mreqn mreq;
  struct mfcctl mfcctrl;
  memset ((void *) &network_list[0], 0xFF, sizeof (network_list));
  ifp =
    (igmp_interface_t *) list_get_head ((set_t **) & igmprt->network_if_list);
  while (ifp != NULL)
    {
      if (ifp->mode == UPSTREAM_INTERFACE)
	{
	  ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);	//next interface
	  continue;
	}
      grp =
	(igmp_group_t *) list_get_head ((set_t **) & ifp->group_database);
      while (grp != NULL)
	{
	  if (grp->group_addr == igmpgrp->group_addr)
	    {			//group exist
	      network_list[ifp->vif_index] = 1;
	      drop_membership = 0;
	      
		/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
		/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
		if (strcmp
		    (ifp->name,
		     igmprt->ptr_bothstream_interface[0]->name) == 0)
		
		{
		  no_drop_num_0 = 0;
		}
	      
	      else
	      if (strcmp
		     (ifp->name,
			igmprt->ptr_bothstream_interface[1]->name) == 0)
		
		{
		  no_drop_num_1 = 0;
		}
	      
		/* ******* end by zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	    }
	  grp = (igmp_group_t *) list_get_next ((set_t *) grp);	//next group
	}
      ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);	//next interface
    }
  if (drop_membership)
    {
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
	if (both_flag)
	
	{
	  if (no_drop_num_0 && no_drop_num_1)
	    
	    {
	      if (ptr_add_net == igmprt->ptr_bothstream_interface[0])
		
		{
		  mreq.imr_ifindex =
		    igmprt->ptr_bothstream_interface[1]->if_index;
		  mreq.imr_multiaddr.s_addr = igmpgrp->group_addr;
		  mreq.imr_address.s_addr =
		    htonl (igmprt->ptr_bothstream_interface[1]->ip_addr.
			   s_addr);
		  if (setsockopt
		       (igmprt->ptr_bothstream_interface[1]->socket,
			IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &mreq,
			sizeof (mreq)) < 0)
		    
		    {
		      return;
		    }
		}
	      
	      else
		
		{
		  mreq.imr_ifindex =
		    igmprt->ptr_bothstream_interface[0]->if_index;
		  mreq.imr_multiaddr.s_addr = igmpgrp->group_addr;
		  mreq.imr_address.s_addr =
		    htonl (igmprt->ptr_bothstream_interface[0]->ip_addr.
			   s_addr);
		  if (setsockopt
		       (igmprt->ptr_bothstream_interface[0]->socket,
			IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &mreq,
			sizeof (mreq)) < 0)
		    
		    {
		      return;
		    }
		}
	    }
	  if (no_drop_num_0)
	    
	    {
	      mreq.imr_ifindex =
		igmprt->ptr_bothstream_interface[0]->if_index;
	      mreq.imr_multiaddr.s_addr = igmpgrp->group_addr;
	      mreq.imr_address.s_addr =
		htonl (igmprt->ptr_bothstream_interface[0]->ip_addr.s_addr);
	      if (setsockopt
		   (igmprt->ptr_bothstream_interface[0]->socket, IPPROTO_IP,
		    IP_DROP_MEMBERSHIP, (void *) &mreq, sizeof (mreq)) < 0)
		
		{
		  return;
		}
	    }
	  
	  else
	  if (no_drop_num_1)
	    
	    {
	      mreq.imr_ifindex =
		igmprt->ptr_bothstream_interface[1]->if_index;
	      mreq.imr_multiaddr.s_addr = igmpgrp->group_addr;
	      mreq.imr_address.s_addr =
		htonl (igmprt->ptr_bothstream_interface[1]->ip_addr.s_addr);
	      if (setsockopt
		   (igmprt->ptr_bothstream_interface[1]->socket, IPPROTO_IP,
		    IP_DROP_MEMBERSHIP, (void *) &mreq, sizeof (mreq)) < 0)
		
		{
		  return;
		}
	    }
	}			//end of if(both_flag)
      else
	
	{
	  mreq.imr_ifindex = igmprt->upstream_interface->if_index;
	  mreq.imr_multiaddr.s_addr = igmpgrp->group_addr;
	  mreq.imr_address.s_addr =
	    htonl (igmprt->upstream_interface->ip_addr.s_addr);
	  if (setsockopt
	       (igmprt->upstream_interface->socket, IPPROTO_IP,
		IP_DROP_MEMBERSHIP, (void *) &mreq, sizeof (mreq)) < 0)
	    {
	      log (LOG_INFO, "setsockopt error: IP_DROP_MEMBERSHIP\n");
	      return;
	    }
	}
      
	/* ******* end by zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	memset ((void *) &mfcctrl, 0, sizeof (mfcctrl));
      mfcctrl.mfcc_origin.s_addr = 0;
      mfcctrl.mfcc_mcastgrp.s_addr = igmpgrp->group_addr;
      if (setsockopt
	   (igmprt->igmp_socket, IPPROTO_IP, MRT_DEL_MFC, (char *) &mfcctrl,
	    sizeof (mfcctrl)) < 0)
	{
	  log (LOG_INFO, "setsockopt error: MRT_DEL_MFC\n");
	  return;
	}
    }
  
  else
    {
      memset ((void *) &mfcctrl, 0, sizeof (mfcctrl));
      mfcctrl.mfcc_origin.s_addr = 0;
      mfcctrl.mfcc_mcastgrp.s_addr = igmpgrp->group_addr;
      if (setsockopt
	   (igmprt->igmp_socket, IPPROTO_IP, MRT_DEL_MFC, (char *) &mfcctrl,
	    sizeof (mfcctrl)) < 0)
	{
	  log (LOG_INFO, "setsockopt error: MRT_DEL_MFC\n");
	  return;
	}
      mfcctrl.mfcc_origin.s_addr = 0;
      mfcctrl.mfcc_mcastgrp.s_addr = igmpgrp->group_addr;
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
#if 0
	mfcctrl.mfcc_parent = igmprt->upstream_interface->vif_index;
      memcpy ((void *) &mfcctrl.mfcc_ttls[0], (void *) &network_list[0],
	       sizeof (network_list));
      if (setsockopt
	   (igmprt->igmp_socket, IPPROTO_IP, MRT_ADD_MFC, (char *) &mfcctrl,
	    sizeof (mfcctrl)) < 0)
	{
	  log (LOG_INFO, "setsockopt error: MRT_ADD_MFC\n");
	  return;
	}
      
#else	/*  */
	if (both_flag)
	
	{
	  if (ptr_add_net == igmprt->ptr_bothstream_interface[0])
	    
	    {
	      mfcctrl.mfcc_parent =
		igmprt->ptr_bothstream_interface[1]->vif_index;
	    }
	  
	  else
	  if (ptr_add_net == igmprt->ptr_bothstream_interface[1])
	    
	    {
	      mfcctrl.mfcc_parent =
		igmprt->ptr_bothstream_interface[0]->vif_index;
	    }
	}
      
      else
	
	{
	  mfcctrl.mfcc_parent = igmprt->upstream_interface->vif_index;
	}
      memcpy ((void *) &mfcctrl.mfcc_ttls[0], (void *) &network_list[0],
	       sizeof (network_list));
      if (setsockopt
	   (igmprt->igmp_socket, IPPROTO_IP, MRT_ADD_MFC, (char *) &mfcctrl,
	    sizeof (mfcctrl)) < 0)
	
	{
	  
	    /* Error: MFC Entry creation failed. */ 
	    return;
	}
      
#endif	/*  */
	/* ******* end by zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    }
  return;
}


/*
 * send a query to a specific group or all host group
 */ 
  void
igmprt_send_query (igmp_router_t * igmprt, igmp_interface_t * ifp,
		   unsigned char max_resp_time, unsigned int gaddr) 
{
  struct igmphdr igmphdr;
  struct sockaddr_in to;
  int num_bytes;
  if (ifp->is_querier == 0)
    return;
  memset ((void *) &igmphdr, 0, sizeof (igmphdr));
  igmphdr.type = IGMP_HOST_MEMBERSHIP_QUERY;
  igmphdr.code = max_resp_time;
  igmphdr.csum = 0;
  igmphdr.group = gaddr;
  igmphdr.csum = in_cksum ((unsigned short *) &igmphdr, 8);
  memset ((void *) &to, 0, sizeof (to));
  to.sin_family = AF_INET;
  if (gaddr == 0)
    to.sin_addr.s_addr = htonl (INADDR_ALLHOSTS_GROUP);	//general query
  else
    to.sin_addr.s_addr = gaddr;	//group specific query
  num_bytes =
    sendto (ifp->socket, (char *) &igmphdr, sizeof (igmphdr), 0,
	    (struct sockaddr *) &to, sizeof (to));
  if (num_bytes < 0)
    log (LOG_INFO, "sendto error\n");
  return;
}


/*
 * Handle whatever has to be done when receiving v1 report
 */ 
  void
igmprt_received_v1_report (igmp_router_t * igmprt, igmp_interface_t * ifp,
			   struct igmphdr *igmp) 
{
  igmp_group_t * grp;
  if (ifp->mode == UPSTREAM_INTERFACE)
    return;
  if (LOCAL_MCAST (igmp->group))
    return;
  
#ifdef IGMP_BLOCK_SOME_SPEC_ADDRS
    if (is_spec_addr (igmp->group))
    return;
  
#endif	/*  */
    grp = igmprt_group_lookup (ifp, igmp->group);
  if (grp == NULL)
    {
      grp =
	(igmp_group_t *) list_remove ((set_t **) & igmprt->free_group_list);
      if (grp == NULL)		// no more free group
	return;
      memset ((void *) grp, 0, sizeof (igmp_group_t));
      grp->config = &igmprt->config;
      grp->group_addr = igmp->group;
      grp->group_timer = igmprt->config.igmp_gmi;
      grp->v1_host_timer = igmprt->config.igmp_gmi;
      grp->rtx_timer = 0;
      list_add ((set_t **) & ifp->group_database, (set_t *) grp);
      igmp_add_membership (igmprt, grp);
    }
  
  else
    {
      grp->group_timer = igmprt->config.igmp_gmi;
      grp->v1_host_timer = igmprt->config.igmp_gmi;
    }
  if (grp->state == IGMP_CHECKING_MEMBERSHIP)
    {
      list_cat ((set_t **) & igmprt->free_query_list,
		 (set_t **) & grp->query_list);
      grp->query_list = NULL;
    }
  grp->state = IGMP_V1_MEMBERS_PRESENT;
  return;
}


/*
 * Handle whatever has to be done when receiving v2 report
 */ 
  void
igmprt_received_v2_report (igmp_router_t * igmprt, igmp_interface_t * ifp,
			   struct igmphdr *igmp) 
{
  igmp_group_t * grp;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    igmp_interface_t * ptr_net;
  if ((ifp->mode == BOTHSTREAM_INTERFACE
	 || ifp->mode ==
	 UPSTREAM_INTERFACE)  &&((igmp->group) & htonl (0xf0000000)) ==
	htonl (0xe0000000)  &&((igmp->group & htonl (0xf0ffff00)) !=
				htonl (0xe0ffff00)))
    
    {
      if (ifp == igmprt->ptr_bothstream_interface[0])
	
	{
	  ifp->mode = DOWNSTREAM_INTERFACE;
	  igmprt->ptr_bothstream_interface[1]->mode = UPSTREAM_INTERFACE;
	  ptr_net =
	    (igmp_interface_t *) list_get_head ((set_t **) & igmprt->
						network_if_list);
	  while (ptr_net != NULL)
	    
	    {
	      
		/* Do not run the router on the bothstream interface. */ 
		if (ptr_net == igmprt->ptr_bothstream_interface[0])
		
		{
		  ptr_net->mode = DOWNSTREAM_INTERFACE;
		}
	      
	      else
	      if (ptr_net == igmprt->ptr_bothstream_interface[1])
		
		{
		  ptr_net->mode = UPSTREAM_INTERFACE;
		}
	      ptr_net =
		(igmp_interface_t *) list_get_next ((set_t *) ptr_net);
	      continue;
	    }
	  igmprt->upstream_interface = igmprt->ptr_bothstream_interface[1];
	}
      
      else
      if (ifp == igmprt->ptr_bothstream_interface[1])
	
	{
	  ifp->mode = DOWNSTREAM_INTERFACE;
	  igmprt->ptr_bothstream_interface[0]->mode = UPSTREAM_INTERFACE;
	  igmprt->upstream_interface = igmprt->ptr_bothstream_interface[0];
	  ptr_net =
	    (igmp_interface_t *) list_get_head ((set_t **) & igmprt->
						network_if_list);
	  while (ptr_net != NULL)
	    
	    {
	      
		/* Do not run the router on the bothstream interface. */ 
		if (ptr_net == igmprt->ptr_bothstream_interface[0])
		
		{
		  ptr_net->mode = UPSTREAM_INTERFACE;
		}
	      
	      else
	      if (ptr_net == igmprt->ptr_bothstream_interface[1])
		
		{
		  ptr_net->mode = DOWNSTREAM_INTERFACE;
		}
	      ptr_net =
		(igmp_interface_t *) list_get_next ((set_t *) ptr_net);
	      continue;
	    }
	}
      
      else
	
	{
	  printf ("the error network\n");
	  return;
	}
    }
  
    /* ******* end by zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    if (ifp->mode == UPSTREAM_INTERFACE)
    return;
  if (LOCAL_MCAST (igmp->group))
    return;
  
#ifdef IGMP_BLOCK_SOME_SPEC_ADDRS
    if (is_spec_addr (igmp->group))
    return;
  
#endif	/*  */
    grp = igmprt_group_lookup (ifp, igmp->group);
  if (grp == NULL)
    {
      grp =
	(igmp_group_t *) list_remove ((set_t **) & igmprt->free_group_list);
      if (grp == NULL)		// no more free group
	return;
      memset ((void *) grp, 0, sizeof (igmp_group_t));
      grp->config = &igmprt->config;
      grp->group_addr = igmp->group;
      grp->group_timer = igmprt->config.igmp_gmi;
      grp->v1_host_timer = 0;
      grp->rtx_timer = 0;
      list_add ((set_t **) & ifp->group_database, (set_t *) grp);
      igmp_add_membership (igmprt, grp);
    }
  
  else
    {
      grp->group_timer = igmprt->config.igmp_gmi;
    }
  if (grp->state == IGMP_CHECKING_MEMBERSHIP)
    {
      list_cat ((set_t **) & igmprt->free_query_list,
		 (set_t **) & grp->query_list);
      grp->query_list = NULL;
    }
  if (grp->state != IGMP_V1_MEMBERS_PRESENT)
    grp->state = IGMP_MEMBERS_PRESENT;
  return;
}


/*
 * Handle whatever has to be done when receiving leave message
 */ 
  void
igmprt_received_leave (igmp_router_t * igmprt, igmp_interface_t * ifp,
		       struct igmphdr *igmp) 
{
  igmp_group_t * grp;
  igmpq_t * q;
  int index = 0;
  if (ifp->mode == UPSTREAM_INTERFACE)
    return;
  if ((grp = igmprt_group_lookup (ifp, igmp->group)) == NULL)
    return;
  if (grp->state == IGMP_V1_MEMBERS_PRESENT)
    return;
  for (index = 0; index < (igmprt->config.igmp_lmqc - 1); index++)
    {
      q = (igmpq_t *) list_remove ((set_t **) & igmprt->free_query_list);
      if (q == NULL)		// no more free queries
	return;
      memset ((void *) q, 0, sizeof (igmpq_t));
      q->type = IGMP_HOST_MEMBERSHIP_QUERY;
      q->max_resp_time = igmprt->config.igmp_lmqi * 10;
      q->group_addr = grp->group_addr;
      
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
	q->bLeave = 1;
      
#endif	/*  */
	list_add ((set_t **) & grp->query_list, (set_t *) q);
    }
  grp->state = IGMP_CHECKING_MEMBERSHIP;
  grp->rtx_timer = igmprt->config.igmp_lmqi;
  grp->v1_host_timer = 0;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    //grp->group_timer = igmprt->config.igmp_lmqi * igmprt->config.igmp_lmqc;
    grp->group_timer =
    igmprt->config.igmp_lmqi * igmprt->config.igmp_lmqc - 1;
  return;
}


/*
 * Handle whatever has to be done when receiving general query
 */ 
  void
igmprt_received_general_query (igmp_router_t * igmprt, igmp_interface_t * ifp,
			       struct iphdr *ip) 
{
  if (ip->saddr < ifp->ip_addr.s_addr)
    {				//non-querier
      ifp->is_querier = 0;
      ifp->querier_present_timer = igmprt->config.igmp_oqp;
      ifp->general_query_timer = 0;
    }
  
  else
    {				//querier
      ifp->querier_present_timer = 0;
    }
  return;
}


/*
 * Read igmp packet and process it
 */ 
  void
igmp_read_data (igmp_router_t * igmprt) 
{
  int num, len;
  struct iphdr *ip;
  struct igmphdr *igmp;
  char buffer[MAX_BUFFER_SIZE], ctrl[MAXCTRLSIZE];
  igmp_interface_t * ifp;
  struct in_pktinfo *info = NULL;
  struct cmsghdr *cmsg;
  struct msghdr msg;
  struct iovec iov;
  memset ((void *) &msg, 0, sizeof (msg));
  iov.iov_base = (char *) &buffer[0];
  iov.iov_len = MAX_BUFFER_SIZE;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = ctrl;
  msg.msg_controllen = MAXCTRLSIZE;
  num = recvmsg (igmprt->igmp_socket, &msg, 0);
  if (num < 0)
    {
      log (LOG_INFO, "recvmsg error\n");
      return;
    }
  for (cmsg = CMSG_FIRSTHDR (&msg); cmsg != NULL;
	 cmsg = CMSG_NXTHDR (&msg, cmsg))
    {
      if (cmsg->cmsg_type == IP_PKTINFO)
	info = (struct in_pktinfo *) CMSG_DATA (cmsg);
  } if (info == NULL)	//no packet infomation
    return;
  ifp =
    (igmp_interface_t *) list_get_head ((set_t **) & igmprt->network_if_list);
  while (ifp != NULL)
    {
      if (ifp->if_index == info->ipi_ifindex)
	break;
      ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);
    }
  if (ifp == NULL)		//not found
    return;
  ip = (struct iphdr *) &buffer[0];
  len = num - (ip->ihl << 2);
  if (len < IGMP_MINLEN)
    {
      log (LOG_INFO, "igmp packet length error\n");
      return;
    }
  igmp = (struct igmphdr *) ((char *) &buffer[0] + (ip->ihl << 2));
  switch (igmp->type)
    
    {
    case IGMP_HOST_MEMBERSHIP_QUERY:
      igmprt_received_general_query (igmprt, ifp, ip);
      break;
    case IGMP_HOST_MEMBERSHIP_REPORT:
      igmprt_received_v1_report (igmprt, ifp, igmp);
      break;
    case IGMPV2_HOST_MEMBERSHIP_REPORT:
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
	//igmprt_received_v2_report(igmprt, ifp, igmp);
	if (ip->saddr != ifp->ip_addr.s_addr)
	
	{
	  igmprt_received_v2_report (igmprt, ifp, igmp);
	}
      break;
    case IGMP_HOST_LEAVE_MESSAGE:
      igmprt_received_leave (igmprt, ifp, igmp);
      break;
    default:			// unknown type
      break;
    }
  return;
}


/*
 * Handle whatever has to be done when group timer expires
 */ 
/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
//igmp_group_t* igmp_group_timer_expire(igmp_router_t *igmprt, igmp_interface_t *ifp, igmp_group_t* grp)
  igmp_group_t * igmp_group_timer_expire (igmp_router_t * igmprt,
					  igmp_interface_t * ifp,
					  igmp_group_t * grp, int both_flag) 
{
  igmp_group_t * nextgrp;
  if (grp->state == IGMP_CHECKING_MEMBERSHIP)
    {
      list_cat ((set_t **) & igmprt->free_query_list,
		 (set_t **) & grp->query_list);
      grp->query_list = NULL;
    }
  nextgrp = (igmp_group_t *) list_get_next ((set_t *) grp);
  if (list_remove_node ((set_t **) & ifp->group_database, (set_t *) grp) < 0)
    return NULL;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    //igmp_drop_membership(igmprt, grp);    
    igmp_drop_membership (igmprt, grp, ifp, both_flag);
  grp->state = IGMP_NO_MEMBERS_PRESENT;
  list_add ((set_t **) & igmprt->free_group_list, (set_t *) grp);
  return nextgrp;
}


/*
 * Handle whatever has to be done when v1 host timer expires
 */ 
  void
igmp_v1_host_timer_expire (igmp_router_t * igmprt, igmp_interface_t * ifp,
			   igmp_group_t * grp) 
{
  if (grp->state != IGMP_V1_MEMBERS_PRESENT)
    return;
  grp->state = IGMP_MEMBERS_PRESENT;
  return;
}


/*
 * Handle whatever has to be done when retransmit timer expires
 */ 
/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
//igmp_group_t* igmp_rtx_timer_expire(igmp_router_t *igmprt, igmp_interface_t *ifp, igmp_group_t *grp)
  igmp_group_t * igmp_rtx_timer_expire (igmp_router_t * igmprt,
					igmp_interface_t * ifp,
					igmp_group_t * grp, int both_flag) 
{
  igmpq_t * q;
  igmp_group_t * nextgrp;
  q = (igmpq_t *) list_remove ((set_t **) & grp->query_list);
  if (q != NULL)
    {
      igmprt_send_query (igmprt, ifp, q->max_resp_time, q->group_addr);
      
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
	if (q->bLeave)
	
	{
	  q->bLeave = 0;
	  list_add ((set_t **) & grp->query_list, (set_t *) q);
	}
      
      else
	
	{
	  
#endif	/*  */
	    list_add ((set_t **) & igmprt->free_query_list, (set_t *) q);
	  
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
	}
      
#endif	/*  */
	grp->rtx_timer = igmprt->config.igmp_lmqi;
      return grp;
    }
  
  else
    {
      nextgrp = (igmp_group_t *) list_get_next ((set_t *) grp);
      if (list_remove_node ((set_t **) & ifp->group_database, (set_t *) grp)
	   < 0)
	return NULL;
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
	//igmp_drop_membership(igmprt, grp);
	igmp_drop_membership (igmprt, grp, ifp, both_flag);
      grp->state = IGMP_NO_MEMBERS_PRESENT;
      list_add ((set_t **) & igmprt->free_group_list, (set_t *) grp);
      return nextgrp;
    }
}


/*
 * Handle whatever has to be done when timer expires
 */ 
  void
igmp_timer_expire (igmp_router_t * igmprt) 
{
  igmp_interface_t * ifp;
  igmp_group_t * grp;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
  int both_flag = 0;
  ifp =
    (igmp_interface_t *) list_get_head ((set_t **) & igmprt->network_if_list);
  while (ifp != NULL)
    {
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug******* */ 
	/* Do not run the router on the upstream interface. */ 
	if (ifp->mode == BOTHSTREAM_INTERFACE)
	
	{
	  both_flag = 1;
	}
      
	//if (ifp == igmprt->upstream_interface) {
	if (ifp->mode == UPSTREAM_INTERFACE)
	{
	  ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);
	  continue;
	}
      if (ifp->is_querier == 1)
	{
	  if (ifp->general_query_timer <= 0)
	    {
	      igmprt_send_query (igmprt, ifp, igmprt->config.igmp_qri * 10,
				  0);
	      
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
		if (bFirstStart)
		
		{
		  ifp->general_query_timer = IGMP_DEF_FIRST_QI;
		  bFirstStart = 0;
		}
	      
	      else
		
#endif	/*  */
		  ifp->general_query_timer = igmprt->config.igmp_qi;
	    }
	  
	  else
	    {
	      ifp->general_query_timer = ifp->general_query_timer - 1;
	    }
	}
      
      else
	{
	  if (ifp->querier_present_timer <= 0)
	    ifp->is_querier = 1;
	  
	  else
	    ifp->querier_present_timer = ifp->querier_present_timer - 1;
	}
      grp =
	(igmp_group_t *) list_get_head ((set_t **) & ifp->group_database);
      while (grp != NULL)
	{
	  if (grp->state < IGMP_NO_MEMBERS_PRESENT
	       || grp->state > IGMP_CHECKING_MEMBERSHIP)
	    return;
	  if (grp->state == IGMP_CHECKING_MEMBERSHIP)
	    {
	      if (grp->rtx_timer <= 0)
		{
		  
		    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
		    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
		    //grp  = igmp_rtx_timer_expire(igmprt, ifp, grp);
		    grp = igmp_rtx_timer_expire (igmprt, ifp, grp, both_flag);
		  continue;
		}
	      
	      else
		{
		  grp->rtx_timer = grp->rtx_timer - 1;
		  grp = (igmp_group_t *) list_get_next ((set_t *) grp);
		  continue;
		}
	    }
	  if (grp->state == IGMP_V1_MEMBERS_PRESENT)
	    {
	      if (grp->v1_host_timer <= 0)
		igmp_v1_host_timer_expire (igmprt, ifp, grp);
	      
	      else
		grp->v1_host_timer = grp->v1_host_timer - 1;
	    }
	  if (grp->state == IGMP_MEMBERS_PRESENT
		|| grp->state == IGMP_V1_MEMBERS_PRESENT)
	    {
	      if (grp->group_timer <= 0)
		{
		  
		    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
		    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
		    //grp = igmp_group_timer_expire(igmprt, ifp, grp);
		    grp =
		    igmp_group_timer_expire (igmprt, ifp, grp, both_flag);
		}
	      
	      else
		{
		  grp->group_timer = grp->group_timer - 1;
		  grp = (igmp_group_t *) list_get_next ((set_t *) grp);
		}
	    }
	}
      ifp = (igmp_interface_t *) list_get_next ((set_t *) ifp);
    }
  return;
}
void
igmprt_tick (void) 
{
  char *buffer = "IGMPTimer";
  if (write (pipefds[1], buffer, strlen (buffer)) < 0)
    log (LOG_INFO, "write error\n");
  return;
}
int
main (int argc, char *argv[]) 
{
  int index, c, i;
  int do_fork = 0;
  fd_set allset, rset;
  igmp_group_t * grp;
  igmpq_t * q;
  int num, maxfd = 0;
  struct itimerval timer, otimer;
  char timerBuffer[20];
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    //char                  *upsname = NULL;
  char *ptr_bothstream_ifaceName[2] = { NULL, NULL };
  
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
    bFirstStart = 1;
  
#endif	/*  */
    if (argc < 3)
    show_usage ();
  for (;;)
    {
      c = getopt (argc, argv, "vfi:");
      if (c == EOF)
	break;
      switch (c)
	{
	case 'v':
	  log_level = LOG_DEBUG;
	  break;
	case 'f':
	  do_fork = 1;
	  break;
	case 'i':
	  if (!strcasecmp (optarg, "-v") || !strcasecmp (optarg, "-f"))
	    show_usage ();
	  
	  else
	    
	      /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	      /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
	      //upsname = optarg;
	      ptr_bothstream_ifaceName[0] = optarg;
	  ptr_bothstream_ifaceName[1] = "br0";
	  break;
	default:
	  show_usage ();
	}
    }
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
#if 0
    if (upsname == NULL)
    show_usage ();
  
#else	/*  */
    if (ptr_bothstream_ifaceName[0] == NULL
	&& ptr_bothstream_ifaceName[1] == NULL)
    
    {
      printf ("the error place 2\n");
      show_usage ();
    }
  
#endif	/*  */
    
    /* Daemonize */ 
    if (do_fork)
    {
      pid_t f = fork ();
      if (f < 0)
	{
	  perror ("fork");
	  exit (EXIT_FAILURE);
	}
      else if (f != 0)
	{
	  
	    /* Parent */ 
	    exit (EXIT_SUCCESS);
	}
    }
  memset ((void *) &router, 0, sizeof (router));
  router.config.igmp_oqp = IGMP_OQPI;
  router.config.igmp_qi = IGMP_DEF_QI;
  router.config.igmp_qri = IGMP_DEF_QRI;
  router.config.igmp_gmi = IGMP_GMI;
  router.config.igmp_lmqi = IGMP_DEF_LMQI;
  router.config.igmp_lmqc = IGMP_DEF_LMQC;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    //strcpy(router.config.upstream_ifname, upsname);
    strcpy (router.config.upstream_ifname, ptr_bothstream_ifaceName[0]);
  strcpy (router.config.bothstream_ifaceName[0],
	   ptr_bothstream_ifaceName[0]);
  strcpy (router.config.bothstream_ifaceName[1],
	   ptr_bothstream_ifaceName[1]);
  router.igmp_socket = socket (AF_INET, SOCK_RAW, IPPROTO_IGMP);
  if (router.igmp_socket < 0)
    {
      log (LOG_INFO, "can't create socket\n");
      return -1;
    }
  
    /* Set the socket options. */ 
    i = 1;
  if (setsockopt
       (router.igmp_socket, IPPROTO_IP, MRT_INIT, (char *) &i,
	sizeof (int)) < 0)
    {
      log (LOG_INFO, "setsockopt error: MRT_INIT\n");
      return -1;
    }
  i = 1;
  setsockopt (router.igmp_socket, SOL_SOCKET, SO_REUSEADDR, (void *) &i,
	       sizeof (i));
  i = 1;
  setsockopt (router.igmp_socket, IPPROTO_IP, IP_PKTINFO, (void *) &i,
	       sizeof (i));
  i = 1;
  setsockopt (router.igmp_socket, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &i,
	       sizeof (i));
  i = 0;
  setsockopt (router.igmp_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (void *) &i,
	       sizeof (i));
  for (index = 0; index < MAX_NUMBER_GROUPS; index++)
    {
      grp = (igmp_group_t *) malloc (sizeof (igmp_group_t));
      if (grp == NULL)
	{
	  log (LOG_INFO, "can't malloc\n");
	  return -1;
	}
      memset ((void *) grp, 0, sizeof (igmp_group_t));
      grp->state = IGMP_NO_MEMBERS_PRESENT;
      list_add ((set_t **) & router.free_group_list, (set_t *) grp);
    }
  for (index = 0; index < MAX_NUMBER_QUERIES; index++)
    {
      q = (igmpq_t *) malloc (sizeof (igmpq_t));
      if (q == NULL)
	{
	  log (LOG_INFO, "can't malloc\n");
	  return -1;
	}
      memset ((void *) q, 0, sizeof (igmpq_t));
      list_add ((set_t **) & router.free_query_list, (set_t *) q);
    }
  signal (SIGKILL, (void *) igmprt_kill);
  signal (SIGTERM, (void *) igmprt_kill);
  signal (SIGHUP, (void *) igmprt_kill);
  if (igmprt_init (&router) < 0)
    return -3;
  
/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
#if 0
    if (router.upstream_interface == NULL)
    {
      log (LOG_INFO, "can't locate upstream interface\n");
      return -2;
    }
  
#endif	/*  */
    FD_ZERO (&rset);
  if (pipe (pipefds) < 0)
    {
      log (LOG_INFO, "can't create timer\n");
      return -1;
    }
  FD_SET (pipefds[0], &rset);
  if (maxfd < pipefds[0])
    maxfd = pipefds[0];
  FD_SET (router.igmp_socket, &rset);
  if (maxfd < router.igmp_socket)
    maxfd = router.igmp_socket;
  memcpy ((void *) &allset, (void *) &rset, sizeof (fd_set));
  
	/**** 2007-01-26 modify by zg to fix cdrouter v3.4 item 302(cdrouter_mcast_100) bug ****/ 
	/**** Verify the maximum number of multicast groups received on the LAN ****/ 
#if 1
    timer.it_interval.tv_sec = 1;
  timer.it_interval.tv_usec = 0;
  timer.it_value.tv_sec = 1;
  timer.it_value.tv_usec = 0;
  
#else	/*  */
    timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 10000;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 10000;
  
#endif	/*  */
    signal (SIGALRM, (void *) igmprt_tick);
  if (setitimer (ITIMER_REAL, &timer, &otimer) < 0)
    {
      log (LOG_INFO, "set timer error\n");
      return -1;
    }
  igmprt_running = 1;
  while (igmprt_running)
    
    {
      num = select (maxfd + 1, &rset, NULL, NULL, NULL);
      if (num < 0)
	{
	  if (errno == EINTR)
	    {
	      if (FD_ISSET (pipefds[0], &rset))
		{
		  if (read
		       (pipefds[0], &timerBuffer[0],
			sizeof (timerBuffer)) < 0)
		    {
		      log (LOG_INFO, "read error\n");
		      break;
		    }
		  
		  else
		    {
		      igmp_timer_expire (&router);
		      continue;
		    }
		}
	      
	      else
		{
		  log (LOG_INFO, "no data on pipe\n");
		  break;
		}
	    }
	  
	  else
	    {
	      log (LOG_INFO, "select failed\n");
	      break;
	    }
	}
      if (FD_ISSET (pipefds[0], &rset))
	{
	  if (read (pipefds[0], &timerBuffer[0], sizeof (timerBuffer)) < 0)
	    {
	      log (LOG_INFO, "read error\n");
	      break;
	    }
	  igmp_timer_expire (&router);
	  num = num - 1;
	}
      if (FD_ISSET (router.igmp_socket, &rset))
	{
	  igmp_read_data (&router);
	  num = num - 1;
	}
      
	//if (num != 0)
	//      log(LOG_DEBUG,"num=%d\n",num);
	FD_ZERO (&rset);
      memcpy ((void *) &rset, (void *) &allset, sizeof (fd_set));
    }
  igmprt_stop (&router);
  return 0;
}


/*
 * Initialize igmp router
 */ 
  int
igmprt_init (igmp_router_t * igmprt) 
{
  struct ifreq *ifr, ifreq;
  int sock, index, upstream = 0;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
  int bothstream[2] = { 0 };
  struct ip_mreqn mreq;
  struct vifctl vifctl;
  igmp_interface_t * ifp;
  struct sockaddr_in *sin, netmask, dstaddr;
  struct in_addr ifaddr;
  short int flags;
  struct ifconf ifcfg;
  char *cp, *limit, buffer[MAX_BUFFER_SIZE];
  sock = socket (PF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      log (LOG_INFO, "can't create socket\n");
      return -1;
    }
  memset ((void *) &ifcfg, 0, sizeof (ifcfg));
  ifcfg.ifc_len = sizeof (buffer);
  ifcfg.ifc_buf = buffer;
  if (ioctl (sock, SIOCGIFCONF, (void *) &ifcfg) < 0)
    {
      log (LOG_INFO, "ioctl error: SIOCGIFCONF\n");
      return -1;
    }
  limit = buffer + ifcfg.ifc_len;
  for (cp = buffer; cp < limit;
	cp = cp + sizeof (ifr->ifr_name) + sizeof (ifr->ifr_ifru))
    {
      ifr = (struct ifreq *) cp;
      sin = (struct sockaddr_in *) &ifr->ifr_ifru.ifru_addr;
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
	if (strcmp (ifr->ifr_name, igmprt->config.bothstream_ifaceName[0]) !=
	    0  &&strcmp (ifr->ifr_name,
			  igmprt->config.bothstream_ifaceName[1]) != 0)
	
	{
//	  printf ("the ifr_name is %s no math\n", ifr->ifr_name);
	  continue;
	}
      
	/* ******* end by zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* Get the index. */ 
	strcpy (ifreq.ifr_name, ifr->ifr_name);
      if (ioctl (sock, SIOCGIFINDEX, (void *) &ifreq) < 0)	//can't get index
	continue;
      index = ifreq.ifr_ifindex;
      
	/* Get the net mask. */ 
	strcpy (ifreq.ifr_name, ifr->ifr_name);
      if (ioctl (sock, SIOCGIFNETMASK, (void *) &ifreq) < 0)	//can't get netmask
	continue;
      memcpy ((void *) &netmask, (void *) &ifreq.ifr_netmask,
	       sizeof (netmask));
      strcpy (ifreq.ifr_name, ifr->ifr_name);
      if (ioctl (sock, SIOCGIFFLAGS, (void *) &ifreq) < 0)
	log (LOG_INFO, "ioctl error: SIOCGIFFLAGS\n");
      flags = ifreq.ifr_flags;
      if (flags & IFF_LOOPBACK)
	continue;
      if (flags & IFF_POINTOPOINT)
	{
	  strcpy (ifreq.ifr_name, ifr->ifr_name);
	  if (ioctl (sock, SIOCGIFDSTADDR, (void *) &ifreq) < 0)
	    log (LOG_INFO, "ioctl error: SIOCGIFDSTADDR\n");
	  
	  else
	    memcpy ((void *) &dstaddr, (void *) &ifreq.ifr_dstaddr,
		     sizeof (struct sockaddr));
	}
      
      else
	{
	  strcpy (ifreq.ifr_name, ifr->ifr_name);
	  ifreq.ifr_flags = ifreq.ifr_flags | IFF_ALLMULTI | IFF_PROMISC;
	  if (ioctl (sock, SIOCSIFFLAGS, (void *) &ifreq) < 0)
	    log (LOG_INFO, "ioctl error: SIOCSIFFLAGS\n");
	} ifp = (igmp_interface_t *) malloc (sizeof (igmp_interface_t));
      if (ifp == NULL)
	{
	  log (LOG_INFO, "can't malloc\n");
	  return -1;
	}
      memset ((void *) ifp, 0, sizeof (igmp_interface_t));
      strcpy (ifp->name, ifreq.ifr_name);
      ifp->if_index = index;
      memcpy ((void *) &ifp->ip_addr, (void *) &sin->sin_addr,
	       sizeof (struct in_addr));
      ifp->flags = flags;
      ifp->is_querier = 1;
      ifp->general_query_timer = 0;
      ifp->socket = socket (AF_INET, SOCK_RAW, IPPROTO_IGMP);
      if (ifp->socket < 0)
	{
	  log (LOG_INFO, "can't create socket\n");
	  return -1;
	}
      memcpy ((void *) &ifaddr, (void *) &ifp->ip_addr,
		sizeof (struct in_addr));
      setsockopt (ifp->socket, IPPROTO_IP, IP_MULTICAST_IF,
		   (void *) &ifp->ip_addr, sizeof (ifaddr));
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug******* */ 
#if 0
	if (strcmp (igmprt->config.upstream_ifname, ifp->name) == 0)
	upstream = 1;		// upstream interface
      else
	upstream = 0;		// downstream interface
      if (upstream == 1)
	{
	  ifp->mode = UPSTREAM_INTERFACE;
	  igmprt->upstream_interface = ifp;
	}
      
      else
	{
	  ifp->mode = DOWNSTREAM_INTERFACE;
	  mreq.imr_ifindex = ifp->if_index;
	  mreq.imr_multiaddr.s_addr = htonl (INADDR_ALLRTRS_GROUP);
	  mreq.imr_address.s_addr = htonl (sin->sin_addr.s_addr);
	  if (setsockopt
	       (ifp->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &mreq,
		sizeof (mreq)) < 0)
	    log (LOG_INFO, "setsockopt error: IP_ADD_MEMBERSHIP\n");
	}
      
#else	/*  */
	if (bothstream[0])
	
	{
	  bothstream[0] = 0;
	}
      if (bothstream[1])
	
	{
	  bothstream[1] = 0;
	}
      if (strcmp (igmprt->config.bothstream_ifaceName[0], ifp->name) == 0)
	
	{
	  bothstream[0] = 1;
//	  printf ("set the bothstream0 sussess\n");
	}
      
      else
      if (strcmp (igmprt->config.bothstream_ifaceName[1], ifp->name) == 0)
	
	{
	  bothstream[1] = 1;
//	  printf ("set the bothstream1 sussess\n");
	}
      if (bothstream[0])
	
	{
	  ifp->mode = BOTHSTREAM_INTERFACE;
	  igmprt->ptr_bothstream_interface[0] = ifp;
	  
	    /* For each of the downstream interfaces become a member of
	       the ALL router club. */ 
	    mreq.imr_ifindex = ifp->if_index;
	  mreq.imr_multiaddr.s_addr = htonl (INADDR_ALLRTRS_GROUP);
	  mreq.imr_address.s_addr = htonl (sin->sin_addr.s_addr);
	  if (setsockopt (ifp->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
			   (void *) &mreq, sizeof (mreq)) < 0)
	    
	    {
	      log (LOG_INFO, "setsockopt error: IP_ADD_MEMBERSHIP\n");
	    }
	}
      
      else if (bothstream[1] == 1)
	
	{
	  ifp->mode = BOTHSTREAM_INTERFACE;
	  igmprt->ptr_bothstream_interface[1] = ifp;
	  
	    /* For each of the downstream interfaces become a member of
	       the ALL router club. */ 
	    mreq.imr_ifindex = ifp->if_index;
	  mreq.imr_multiaddr.s_addr = htonl (INADDR_ALLRTRS_GROUP);
	  mreq.imr_address.s_addr = htonl (sin->sin_addr.s_addr);
	  if (setsockopt (ifp->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
			   (void *) &mreq, sizeof (mreq)) < 0)
	    
	    {
	      log (LOG_INFO, "setsockopt error: IP_ADD_MEMBERSHIP\n");
	    }
	}
      
	//default use the wan interface as the UPSTREAM_INTERFACE
	if (strcmp (igmprt->config.upstream_ifname, ifp->name) == 0)
	{
	  ifp->mode = UPSTREAM_INTERFACE;
	  igmprt->upstream_interface = ifp;
	}
      
#endif	/*  */
	/* ******* end by zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	vifctl.vifc_vifi = ifp->if_index;
      vifctl.vifc_flags = 0;
      vifctl.vifc_threshold = 0;
      vifctl.vifc_rate_limit = 0;
      vifctl.vifc_lcl_addr.s_addr = sin->sin_addr.s_addr;
      vifctl.vifc_rmt_addr.s_addr = INADDR_ANY;
      if (setsockopt
	   (igmprt->igmp_socket, IPPROTO_IP, MRT_ADD_VIF, (char *) &vifctl,
	    sizeof (vifctl)) < 0)
	{
	  log (LOG_INFO, "setsockopt error: MRT_ADD_VIF\n");
	  ifp->vif_index = -1;
	}
      
      else
	{
	  ifp->vif_index = ifp->if_index;
	}
      list_add ((set_t **) & igmprt->network_if_list, (set_t *) ifp);
    }
  close (sock);
  return 0;
}
void
igmprt_kill (void) 
{
  igmprt_running = 0;
  return;
}


/*
 * Cleanup an interface 
 */ 
  void
igmp_interface_cleanup (igmp_router_t * igmprt, igmp_interface_t * ifp) 
{
  igmp_group_t * grp;
  struct ifreq ifr;
  struct vifctl vifctl;
  int sock;
  sock = socket (PF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      log (LOG_INFO, "can't create socket\n");
      return;
    }
  grp = (igmp_group_t *) list_remove ((set_t **) & ifp->group_database);
  while (grp != NULL)
    {
      
	/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
	/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug******* */ 
	//igmp_drop_membership(igmprt, grp);
	igmp_drop_membership (igmprt, grp, ifp, 0);
      list_cat ((set_t **) & igmprt->free_query_list,
		  (set_t **) & grp->query_list);
      list_add ((set_t **) & igmprt->free_group_list, (set_t *) grp);
      grp = (igmp_group_t *) list_remove ((set_t **) & ifp->group_database);
    }
  ifr.ifr_flags = ifp->flags;
  strcpy (ifr.ifr_name, ifp->name);
  if (ioctl (sock, SIOCSIFFLAGS, (void *) &ifr) < 0)
    log (LOG_INFO, "ioctl error: SIOCSIFFLAGS\n");
  memset ((void *) &vifctl, 0, sizeof (vifctl));
  vifctl.vifc_vifi = ifp->vif_index;
  if (setsockopt
       (igmprt->igmp_socket, IPPROTO_IP, MRT_DEL_VIF, (char *) &vifctl,
	sizeof (vifctl)) < 0)
    log (LOG_INFO, "setsockopt error: MRT_DEL_VIF\n");
  close (sock);
  close (ifp->socket);
  return;
}


/*
 * Cleanup the router 
 */ 
  void
igmprt_stop (igmp_router_t * igmprt) 
{
  igmp_interface_t * ifp;
  igmp_group_t * grp;
  igmpq_t * q;
  ifp =
    (igmp_interface_t *) list_remove ((set_t **) & igmprt->network_if_list);
  while (ifp != NULL)
    {
      if (ifp->mode == UPSTREAM_INTERFACE)
	{
	  ifp =
	    (igmp_interface_t *) list_remove ((set_t **) & igmprt->
					      network_if_list);
	  continue;
	}
      igmp_interface_cleanup (igmprt, ifp);
      free (ifp);
      ifp =
	(igmp_interface_t *) list_remove ((set_t **) & igmprt->
					  network_if_list);
    }
  igmp_interface_cleanup (igmprt, igmprt->upstream_interface);
  free (igmprt->upstream_interface);
  q = (igmpq_t *) list_remove ((set_t **) & igmprt->free_query_list);
  while (q != NULL)
    {
      free (q);
      q = (igmpq_t *) list_remove ((set_t **) & igmprt->free_query_list);
    }
  grp = (igmp_group_t *) list_remove ((set_t **) & igmprt->free_group_list);
  while (grp != NULL)
    {
      free (grp);
      grp =
	(igmp_group_t *) list_remove ((set_t **) & igmprt->free_group_list);
    }
  if (setsockopt
	(igmprt->igmp_socket, IPPROTO_IP, MRT_DONE, (char *) NULL, 0) < 0)
    log (LOG_INFO, "setsockopt error: MRT_DONE\n");
  return;
}


/*
 * Lookup for a group entry
 */ 
  igmp_group_t * igmprt_group_lookup (igmp_interface_t * ifp,
				      unsigned int gaddr) 
{
  igmp_group_t * grp;
  grp = (igmp_group_t *) list_get_head ((set_t **) & ifp->group_database);
  while (grp != NULL)
    {
      if (grp->group_addr == gaddr)
	return grp;
      grp = (igmp_group_t *) list_get_next ((set_t *) grp);
    }
  return NULL;
}
void
usage (char *prog) 
{
  
#if 0
    printf ("Usage: %s [-v] [-f] -i upstream_interface\n", prog);
  exit (1);
  
#else	/*  */
    printf ("Usage: %s [-v] [-f] -i upstream_interface\n", prog);
  printf ("Options:\n");
  printf ("-v                   -- Print debug messages\n");
  printf ("-f                   -- Do not fork\n");
  printf ("-i upsteam_interface -- Upstream interface name\n\n");
  exit (1);
  
#endif	/*  */
} void
log (int level, char *fmt, ...) 
{
  va_list args;
  if (level > log_level)
    return;
  va_start (args, fmt);
  vprintf (fmt, args);
  va_end (args);
  return;
}


#ifdef IGMP_BLOCK_SOME_SPEC_ADDRS
//239.255.255.250
static unsigned int spec_addrs[] = { 0xEFFFFFFA };
int
is_spec_addr (unsigned int group) 
{
  int result = 0;
  int i = 0;
  for (i = 0; i < (sizeof (spec_addrs) / sizeof (int)); i++)
    
    {
      if (group == htonl (spec_addrs[i]))
	
	{
	  result = 1;
	  break;
	}
    }
  return result;
}


#endif	/*  */
