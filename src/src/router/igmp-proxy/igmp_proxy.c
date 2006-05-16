/**************************************************************************
 * FILE PURPOSE	:  	IGMP Proxy Implementation.
 **************************************************************************
 * FILE NAME	:   igmp_proxy.c
 *
 * DESCRIPTION	:
 * 	This file contains the implementation of the IGMP proxy module.
 *
 *	CALL-INs:
 *
 *	CALL-OUTs:
 *
 *	User-Configurable Items:
 *
 *	(C) Copyright 2002, Texas Instruments, Inc.
 *************************************************************************/

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
#include "igmp_proxy.h"

/************************** STANDARD DEFINITIONS ***************************/

#define ntohl(x)			__bswap_32 (x)
#define ntohs(x)			__bswap_16 (x)
#define htonl(x)			__bswap_32 (x)
#define htons(x)			__bswap_16 (x)

#define LOCAL_MCAST(x)		(((x) & htonl(0xFFFFFF00)) == htonl(0xE0000000))

#define	MAX_BUFFER_SIZE		1600
#define MAXCTRLSIZE			(sizeof(struct cmsghdr) +sizeof(struct in_pktinfo))

/************************** GLOBAL Declarations ***************************/

/* Log Level for all debug messages. */
int proxyDebugLevel = IGMP_FATAL;

/* The global IGMP router master control block. */
IGMP_ROUTER_MCB		igmp_router;

/* Timer Pipe. */
int timerPipe[2];

/* The status flag which indicates that the state of the module. */
volatile int IGMPProxyRunning = 0;

/* Counters which keep track of total memory allocated and freed up. */
int malloc_counter = 0;
int free_counter   = 0;
int	malloced_size  = 0;

/**************************** STATIC Declarations ***************************/

/* Initialization functions */
static int igmp_create_network_list(IGMP_ROUTER_MCB *ptr_igmp_router);

/* IGMP Timer Handlers. */
static void igmp_router_timer_expired (IGMP_ROUTER_MCB *ptr_igmp_router);
static IGMP_GROUP* igmp_router_group_timer_expired (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,IGMP_GROUP*);
static IGMP_GROUP* igmp_router_retransmit_timer_expired (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,IGMP_GROUP*);
static void igmp_router_v1_host_timer_expired (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,IGMP_GROUP*);

/* IGMP Message Handlers. */
static void igmp_router_received_general_query (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,struct iphdr*);
static void igmp_router_version2_report_handler (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,struct igmphdr*);
static void igmp_router_version1_report_handler (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,struct igmphdr*);
static void igmp_router_leave_handler (IGMP_ROUTER_MCB*,IGMP_NETWORK_IFACE*,struct igmphdr*);
static void igmp_network_socket_read_data (IGMP_ROUTER_MCB*);

/* Proxy Functions */
static void igmp_proxy_add_membership (IGMP_ROUTER_MCB*,IGMP_GROUP*);
static void igmp_proxy_drop_membership(IGMP_ROUTER_MCB*,IGMP_GROUP*);

/* Cleanup Functions */
static void igmp_router_kill (void);
static void igmp_proxy_clean_interface(IGMP_ROUTER_MCB*ptr_router,IGMP_NETWORK_IFACE*ptr_net);
static void igmp_proxy_stop (IGMP_ROUTER_MCB *ptr_router);

/* Lookup Functions */
static IGMP_GROUP* igmp_router_group_lookup(IGMP_NETWORK_IFACE*	ptr_net,unsigned int address);

/* General Utility functions. */
static void print_usage (void);
static void logMsg (int level,char* fmt,...);
static unsigned short in_cksum(unsigned short *addr, int len);
static char *getGroupState (IGMP_GROUP* ptr_group);
static void* my_malloc (int size);
static void my_free (void *ptr);
static void xdump( u_char*  cp, int  length, char*  prefix );

/********************************* FUNCTIONS ******************************/

/**************************************************************************
 * FUNCTION NAME : igmp_proxy_add_membership 
 **************************************************************************
 * DESCRIPTION   :
 * 	Add membership for the group. 
 *
 * NOTES		 :
 *	There are 2 things that can be done here 
 *	1. Blindly add group membership to the upstream interface
 *	2. Add group membership to the upstream interface only if the upstream 
 *	   interface is not already a member of the group. 
 *	The control comes here when a new group report is received on an 
 *	interface. Now suppose we got a report for Group G1 on interface I1. We 
 *	will add the membership of G1 on the upstream interface. Then if we 
 *	receive a report of G1 on interface I2. Do we again add membership ? 
 *	Lets go with alternative 1 for the time being. 
 ***************************************************************************/
static void igmp_proxy_add_membership 
(
	IGMP_ROUTER_MCB*	ptr_router,
	IGMP_GROUP*			ptr_new_group
)
{
	unsigned char		network_list[MAXVIFS];
	IGMP_NETWORK_IFACE*	ptr_net;
	IGMP_GROUP*			ptr_group;
	struct ip_mreqn 	mreq;
	struct mfcctl		mfcctrl;

	/* Initialize the VIF's for the MFC entry. */
	memset ((void *)&network_list, 0xFF, sizeof(network_list));

	/* Use the setsockopt IP_ADD_MEMBERSHIP with the upstream socket so that the HOST 
	 * sends out a membership report on the upstream interface. */
	mreq.imr_ifindex		  = ptr_router->ptr_upstream_interface->iface_index; 
	mreq.imr_multiaddr.s_addr = ptr_new_group->group_address;
	mreq.imr_address.s_addr   = htonl(ptr_router->ptr_upstream_interface->ip_address.s_addr);
	if (setsockopt(ptr_router->ptr_upstream_interface->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
	    (void*)&mreq, sizeof(mreq)) < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to add membership for Group:%x on upstream Error:%d\n", 
				ptr_new_group->group_address, errno);
		return;
	}

	/* Add the MFC entry for the network also. Cycle through all the interfaces and match all 
	 * the groups that match the group being added. Then add an MFC entry. */
	ptr_net = (IGMP_NETWORK_IFACE *)list_get_head ((LIST_NODE **)&ptr_router->network_if_list);
	while (ptr_net != NULL)
	{
		/* Do the work only for the downstream interface */
		if (ptr_net->mode == UPSTREAM_INTERFACE)
		{
			/* Get the next interface. */
			ptr_net = (IGMP_NETWORK_IFACE *)list_get_next ((LIST_NODE *)ptr_net);
			continue;
		}

		/* Cycle through all the groups. */
		ptr_group = (IGMP_GROUP *)list_get_head ((LIST_NODE **)&ptr_net->group_database);
		while (ptr_group != NULL)
		{
			/* We had a HIT on network for the new group being added. */
			if (ptr_group->group_address == ptr_new_group->group_address)
			{
				logMsg (IGMP_DEBUG, "DEBUG: MFC Entry will be created for iface:%s.\n",ptr_net->ifaceName);
				network_list[ptr_net->vif_index] = 1;
				break;
			}
			/* Get the next group. */
			ptr_group = (IGMP_GROUP *)list_get_next ((LIST_NODE *)ptr_group);
		}

		/* Get the next interface. */
		ptr_net = (IGMP_NETWORK_IFACE *)list_get_next ((LIST_NODE *)ptr_net);		
	}

	/* Initialize the MFC ctrl structure.*/
	memset ((void *)&mfcctrl, 0, sizeof (mfcctrl));

	/* Fill up the MFC structure. Indicating that all multicast Traffic comming from 
	 * 'upstream_address' for Group 'group_address' through the 'upstream VIF' needs to 
	 * be forwarded to all networks which have a TTL of 1. Since the implementation in the
	 * kernel has been changed to no longer care for the SOURCE of Multicast packets we
	 * can safely pass a 0 as the ORIGIN of multicast packets. */	
	mfcctrl.mfcc_origin.s_addr 		= 0;
	mfcctrl.mfcc_mcastgrp.s_addr 	= ptr_new_group->group_address;
	mfcctrl.mfcc_parent				= ptr_router->ptr_upstream_interface->vif_index;
	memcpy((void *)&mfcctrl.mfcc_ttls[0], (void *)&network_list[0], sizeof (network_list));
	if (setsockopt (ptr_router->igmp_router_socket, IPPROTO_IP, MRT_ADD_MFC, 
					(char *)&mfcctrl, sizeof(mfcctrl)) < 0)
	{
		/* Error: MFC Entry creation failed. */
		logMsg (IGMP_FATAL, "IGMP Error: Unable to add MFC entry for group:%x Error:%d.\n",
					ptr_new_group->group_address,errno);
		return;
	}
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_proxy_drop_membership 
 **************************************************************************
 * DESCRIPTION   :
 * 	Drop membership for the group. 
 *
 * NOTES		 :
 *	The GROUP has expired or is being removed as there are no more members 
 *	present.Use the setsockopt with IP_DROP_MEMBERSHIP on the upstream socket 
 *	indicating that we are leaving the group, but do so provided that none of 
 *	the other downstream interfaces have any memberships for the specific 
 *	group.
 ***************************************************************************/
static void igmp_proxy_drop_membership 
(
	IGMP_ROUTER_MCB*	ptr_router,
	IGMP_GROUP*			ptr_igmp_group
)
{
	IGMP_NETWORK_IFACE*	ptr_net;
	IGMP_GROUP*			ptr_group;
	int					network_list[MAXVIFS];
	int					drop_membership = 1;
	struct ip_mreqn 	mreq;
	struct mfcctl		mfcctrl;

	/* Initialize the network list. */
	memset ((void *)&network_list[0], 0xFF, sizeof (network_list));

	/* Cycle through all the downstream network interfaces. */
	ptr_net = (IGMP_NETWORK_IFACE *)list_get_head ((LIST_NODE **)&ptr_router->network_if_list);
	while (ptr_net != NULL)
	{
		/* Do the work only for the downstream interface */
		if (ptr_net->mode == UPSTREAM_INTERFACE)
		{
			/* Get the next interface. */
			ptr_net = (IGMP_NETWORK_IFACE *)list_get_next ((LIST_NODE *)ptr_net);
			continue;
		}
		
		/* Cycle through all the groups for the downstream interface */
		ptr_group = (IGMP_GROUP *)list_get_head ((LIST_NODE **)&ptr_net->group_database);
		while (ptr_group != NULL)
		{
			/* Did we get a hit ? */
			if (ptr_group->group_address == ptr_igmp_group->group_address)
			{
				/* YES. Cannot delete membership on upstream interface as there is a 
				 * group on an interface which is still present. */
				logMsg (IGMP_DEBUG, "DEBUG: Group %x on Network:%s still there. Not dropping.\n",
						ptr_igmp_group->group_address, ptr_net->ifaceName);

				/* Remember the network for which the MFC entry remains valid.*/
				network_list[ptr_net->vif_index] = 1;

				/* Cannot drop membership. */
				drop_membership = 0;
			}
			/* Get the next group. */
			ptr_group = (IGMP_GROUP *)list_get_next ((LIST_NODE *)ptr_group);
		}

		/* Get the next interface. */
		ptr_net = (IGMP_NETWORK_IFACE *)list_get_next ((LIST_NODE *)ptr_net);
	}

	/* Check if we can drop membership for the specific group. */
	if (drop_membership)
	{
		/* Use setsocketopt with IP_DROP_MEMBERSHIP on the upstream socket to remove 
		 * the membership for the group from the upstream interface. */
		mreq.imr_ifindex		  = ptr_router->ptr_upstream_interface->iface_index; 
		mreq.imr_multiaddr.s_addr = ptr_igmp_group->group_address;
		mreq.imr_address.s_addr   = htonl(ptr_router->ptr_upstream_interface->ip_address.s_addr);
		if (setsockopt(ptr_router->ptr_upstream_interface->socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
	    	 			(void*)&mreq, sizeof(mreq)) < 0)
		{
			logMsg (IGMP_FATAL, "IGMP Error: Unable to drop membership for Group:%x on upstream Error:%d\n",
						ptr_igmp_group->group_address, errno);
			return;
		}
	
		/* Initialize the MFC ctrl. */		
		memset ((void *)&mfcctrl, 0, sizeof (mfcctrl));
		
		/* Fill up the MFC structure. Since the implementation in the kernel has been 
		 * changed to no longer care for the SOURCE of Multicast packets we can safely 
		 * pass a 0 as the ORIGIN of multicast packets. */
		mfcctrl.mfcc_origin.s_addr 		= 0;
		mfcctrl.mfcc_mcastgrp.s_addr 	= ptr_igmp_group->group_address;
		if (setsockopt (ptr_router->igmp_router_socket, IPPROTO_IP, MRT_DEL_MFC, 
						(char *)&mfcctrl, sizeof(mfcctrl)) < 0)
		{
			/* Error: MFC Entry creation failed. */
			logMsg (IGMP_FATAL, "IGMP Error: Unable to del MFC entry for group:%x Error:%d.\n",
						ptr_igmp_group->group_address,errno);
			return;
		}
	}
	else
	{			
		/* Update the multicast forwarding cache. First delete the existing MFC entry and then
		 * add the new one. Initialize the MFC ctrl. */
		memset ((void *)&mfcctrl, 0, sizeof (mfcctrl));

		/* Fill up the MFC structure. Since the implementation in the kernel has been 
		 * changed to no longer care for the SOURCE of Multicast packets we can safely 
		 * pass a 0 as the ORIGIN of multicast packets. */
		mfcctrl.mfcc_origin.s_addr 		= 0;
		mfcctrl.mfcc_mcastgrp.s_addr 	= ptr_igmp_group->group_address;
		if (setsockopt (ptr_router->igmp_router_socket, IPPROTO_IP, MRT_DEL_MFC, 
						(char *)&mfcctrl, sizeof(mfcctrl)) < 0)
		{
			/* Error: MFC Entry creation failed. */
			logMsg (IGMP_FATAL, "IGMP Error: Unable to del MFC entry for group:%x Error:%d.\n",
						ptr_igmp_group->group_address,errno);
			return;
		}

		/* Add the new modifed MFC entry. Fill up the MFC structure. Since the implementation 
		 * in the kernel has been changed to no longer care for the SOURCE of Multicast 
		 * packets we can safely pass a 0 as the ORIGIN of multicast packets. */		
		mfcctrl.mfcc_origin.s_addr 		= 0;
		mfcctrl.mfcc_mcastgrp.s_addr 	= ptr_igmp_group->group_address;
		mfcctrl.mfcc_parent				= ptr_router->ptr_upstream_interface->vif_index;
		memcpy((void *)&mfcctrl.mfcc_ttls[0], (void *)&network_list[0], sizeof (network_list));
		if (setsockopt (ptr_router->igmp_router_socket, IPPROTO_IP, MRT_ADD_MFC, 
						(char *)&mfcctrl, sizeof(mfcctrl)) < 0)
		{
			/* Error: MFC Entry creation failed. */
			logMsg (IGMP_FATAL, "IGMP Error: Updation of MFC entry for group:%x failed. Error:%d.\n",
						ptr_igmp_group->group_address,errno);
			return;
		}
	}
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_send_query
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is used to send a query specified by 'type' for the
 * 	'group_address' on the network 'ptr_network'. 
 *  If the group_address passed is 0 a GENERAL query is sent out.
 *
 * NOTES		 :
 * 	The function assumes that the group_address passed to it is in host 
 * 	order.
 ***************************************************************************/
static void igmp_router_send_query 
(
	IGMP_ROUTER_MCB*	ptr_router,
	IGMP_NETWORK_IFACE*	ptr_network,
	unsigned char		max_resp_time,
	unsigned int		group_address
)
{
	struct igmphdr		igmp_header;
	struct sockaddr_in	to;
	int					num_bytes;

	/* Do not send out any queries if we are not the querier. */
	if (ptr_network->bIsQuerier == 0)
		return;

	/* Initialize the IGMP header. */
	memset ((void *)&igmp_header, 0, sizeof (igmp_header));

	/* Fill in the fields in the header. */
	igmp_header.type  = IGMP_HOST_MEMBERSHIP_QUERY;
	igmp_header.code  = max_resp_time;
	igmp_header.csum  = 0;
	igmp_header.group = group_address;

	/* Fill in the checksum. */
	igmp_header.csum = in_cksum((unsigned short *)&igmp_header, 8);

	/* Initialize the to structure. */
	memset ((void *)&to, 0, sizeof (to));
	
	/* Fill in the structure. */
	to.sin_family = AF_INET;
	
	/* Is this a group specific query or general query ? */
	if (group_address == 0)
		to.sin_addr.s_addr = htonl(INADDR_ALLHOSTS_GROUP);	/* General Query. 		*/
	else
		to.sin_addr.s_addr = group_address;					/* Group Specific Query.*/

	/* Send out the query. */
	num_bytes = sendto (ptr_network->socket, (char *)&igmp_header, sizeof (igmp_header), 0,
						(struct sockaddr *)&to, sizeof (to));
	if (num_bytes < 0)
	{
		logMsg (IGMP_FATAL,"IGMP Error: Unable to send query ErrorNumber:%d.\n",errno); 
	}
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_version1_report_handler
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the handler when the router receives an IGMP version 1 report.
 ***************************************************************************/
static void igmp_router_version1_report_handler 
(
	IGMP_ROUTER_MCB *	ptr_router,
	IGMP_NETWORK_IFACE*	ptr_network,
	struct igmphdr*		ptr_header
)
{
	IGMP_GROUP*			ptr_group;
	
	/* Ignore reports received on the upstream interface. Report supression is
	 * handled by the kernel. */
	if (ptr_network->mode == UPSTREAM_INTERFACE)
		return;

	/* Process the packet only if this is non local multicast address. */
	if (LOCAL_MCAST(ptr_header->group))
		return;

	/* Check if the group already exists. */
	ptr_group = igmp_router_group_lookup (ptr_network, ptr_header->group);
	if (ptr_group == NULL)
	{
		/* No previous membership exists,so create a new one. Allocate memory 
		 * for the group membership. */
		ptr_group = (IGMP_GROUP *)list_remove((LIST_NODE **)&ptr_router->free_group_list);
		if (ptr_group == NULL)
		{
			logMsg (IGMP_FATAL, "IGMP Error: No more free group available. Dropping membership request.\n");
			return;
		}

		/* Initialize the memory block. */
		memset ((void *)ptr_group, 0, sizeof (IGMP_GROUP));

		/* Fill in the fields. */
		ptr_group->ptr_config		= &ptr_router->config;
		ptr_group->group_address	= ptr_header->group;		
		ptr_group->group_timer		= ptr_router->config.group_membership_interval;
		ptr_group->v1_host_timer	= ptr_router->config.group_membership_interval;

		/* Retransmit timers do not come into the picture. */		
		ptr_group->retransmit_timer = 0;

		/* Add the group address to the network group database. */
		list_add ((LIST_NODE **)&ptr_network->group_database, (LIST_NODE *)ptr_group);

		/* Proxy work - Add the group membership on the upstream interface. */
		igmp_proxy_add_membership (ptr_router, ptr_group);
	}
	else
	{
		/* Previous membership exists. So simply refresh the timer. */
		ptr_group->group_timer	 = ptr_router->config.group_membership_interval;
		ptr_group->v1_host_timer = ptr_router->config.group_membership_interval;
	}

	/* NOTE: If we were in the CHECKING Membership state, we would have scheduled some
	 * group specific queries, since we have got a report from one of the hosts who
	 * is still a member of the group. We do not need to send out any more group specific
	 * queries. Simply cancel all such queries. */
	if (ptr_group->state == IGMP_ROUTER_CHECKING_MEMBERSHIP)
	{
		/* Concatenate the query list for the group back to the free list. */
		list_cat ((LIST_NODE **)&ptr_router->free_query_list,(LIST_NODE **)&ptr_group->query_list);

		/* We are done with the query list. */
		ptr_group->query_list = NULL;
	}

	/* The group has members associated with it. */
	ptr_group->state = IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT;

	/* Print the Current State of the router. */
	logMsg (IGMP_DEBUG,"Network:%s Group:%x State = %s.\n",ptr_network->ifaceName,
			ptr_group->group_address, getGroupState (ptr_group));
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_version2_report_handler
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the handler when the router receives an IGMP version 2 report.
 ***************************************************************************/
static void igmp_router_version2_report_handler
(
	IGMP_ROUTER_MCB *	ptr_router,
	IGMP_NETWORK_IFACE*	ptr_network,
	struct igmphdr*		ptr_header
)
{
	IGMP_GROUP*			ptr_group;

	/* Ignore reports received on the upstream interface. Report supression is
	 * handled by the kernel. */
	if (ptr_network->mode == UPSTREAM_INTERFACE)
		return;

	/* Process the packet only if the packet is destined for a NON local multicast address. */
	if (LOCAL_MCAST(ptr_header->group))
		return;

	/* Check if the group already exists. */
	ptr_group = igmp_router_group_lookup (ptr_network, ptr_header->group);
	if (ptr_group == NULL)
	{
		/* No previous membership exists,so create a new one. Allocate memory 
		 * for the group membership. */
		ptr_group = (IGMP_GROUP *)list_remove((LIST_NODE **)&ptr_router->free_group_list);
		if (ptr_group == NULL)
		{
			logMsg (IGMP_FATAL, "IGMP Error: No more free group available. Dropping membership request.\n");
			return;
		}

		/* Initialize the memory block. */
		memset ((void *)ptr_group, 0, sizeof (IGMP_GROUP));

		/* Fill in the fields. */
		ptr_group->ptr_config    = &ptr_router->config;
		ptr_group->group_address = ptr_header->group;
		ptr_group->group_timer	 = ptr_router->config.group_membership_interval;

		/* V1 timers and retransmit timers do not come into the picture. */
		ptr_group->v1_host_timer	= 0;
		ptr_group->retransmit_timer = 0;

		/* Add the group address to the network group database. */
		list_add ((LIST_NODE **)&ptr_network->group_database, (LIST_NODE *)ptr_group);

		/* Proxy work - Add the group membership on the upstream interface. */
		igmp_proxy_add_membership (ptr_router, ptr_group);
	}
	else
	{
		/* Previous membership exists. So simply refresh the timer. */
		ptr_group->group_timer	 = ptr_router->config.group_membership_interval;
	}

	/* NOTE: If we were in the CHECKING Membership state, we would have scheduled some
	 * group specific queries, since we have got a report from one of the hosts who
	 * is still a member of the group. We do not need to send out any more group specific
	 * queries. Simply cancel all such queries. */
	if (ptr_group->state == IGMP_ROUTER_CHECKING_MEMBERSHIP)
	{
		/* Concatenate the query list for the group back to the free list. */
		list_cat ((LIST_NODE **)&ptr_router->free_query_list,(LIST_NODE **)&ptr_group->query_list);

		/* We are done with the query list. */
		ptr_group->query_list = NULL;
	}

	/* NOTE: If we were in version 1 members present state then we remain there 
	 * for backward compatibilty. In all other cases we shift to MEMBERS present
	 * state. */
	if (ptr_group->state != IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT)
		ptr_group->state = IGMP_ROUTER_MEMBERS_PRESENT;

	/* Print the Current State of the router. */
	logMsg (IGMP_DEBUG,"Network:%s Group:%x State = %s.\n",ptr_network->ifaceName,
			ptr_group->group_address, getGroupState (ptr_group));
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_leave_handler
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the handler when the router receives an IGMP leave message.
 ***************************************************************************/
static void igmp_router_leave_handler
(
	IGMP_ROUTER_MCB *	ptr_router,
	IGMP_NETWORK_IFACE*	ptr_network,
	struct igmphdr*		ptr_header
)
{
	IGMP_GROUP*	ptr_group;
	IGMP_QUERY*	ptr_query;
	int			index = 0;

	/* Ignore leaves received on the upstream interface. Report supression is
	 * handled by the kernel. */
	if (ptr_network->mode == UPSTREAM_INTERFACE)
		return;

	/* Get the group membership information. */
	ptr_group = igmp_router_group_lookup (ptr_network, ptr_header->group);
	if (ptr_group == NULL)
		return;

	/* SANITY Check -- As per RFC 2236 if we are in version 1 members present state. 
	 * Ignore all leave reports. */
	if (ptr_group->state == IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT)
	{
		logMsg (IGMP_FATAL, "Leave report group:%x network:%s but in V1 state. Ignoring.\n",
				ptr_header->group, ptr_network->ifaceName);
		return;
	}

	/* Send out Last Member query count number of group specific queries. */
	for (index = 0; index < (ptr_router->config.last_member_query_count - 1); index ++)
	{
		/* Get a query from the free pool. */
		ptr_query = (IGMP_QUERY *)list_remove ((LIST_NODE **)&ptr_router->free_query_list);
		if (ptr_query == NULL)
		{
			logMsg (IGMP_FATAL, "IGMP Error: No more free queries available.\n");
			return;
		}

		/* Initialize the query. */
		memset ((void *)ptr_query, 0, sizeof (IGMP_QUERY));

		/* Fill in the fields. */
		ptr_query->type				= IGMP_HOST_MEMBERSHIP_QUERY;
		ptr_query->max_resp_time	= ptr_router->config.last_member_query_interval * 10;
		ptr_query->group_address	= ptr_group->group_address;

		/* Place the query in the schedule list for the specific interface. */
		list_add ((LIST_NODE **)&ptr_group->query_list, (LIST_NODE *)ptr_query);
	}

	/* Once the queries have been scheduled. Change the state of the group to 
	 * membership checking state. */
	ptr_group->state = IGMP_ROUTER_CHECKING_MEMBERSHIP;

	/* Start the RETRANSMISSION Timer. */
	ptr_group->retransmit_timer		= ptr_router->config.last_member_query_interval;

	/* V1 Host timers does not come into the picture. */
	ptr_group->v1_host_timer		= 0;

	/* Set the timer on the group membership interval - As specified by the RFC-2236 Pg 13 */
	ptr_group->group_timer = ptr_router->config.last_member_query_interval * 
							 ptr_router->config.last_member_query_count;

	/* Print the Current State of the router. */
	logMsg (IGMP_DEBUG,"Network:%s Group:%x State = %s.\n",ptr_network->ifaceName,
			ptr_group->group_address, getGroupState (ptr_group));
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_received_general_query
 **************************************************************************
 * DESCRIPTION   :
 *	The function handles the reception of a general query. Reception of a 
 *	query indicates the prescence of another IGMP Router. The Querier 
 *	algorithm now comes into action as we need to decide who will be the
 *	querier.
 ***************************************************************************/
static void igmp_router_received_general_query 
(
	IGMP_ROUTER_MCB *	ptr_router,
	IGMP_NETWORK_IFACE*	ptr_network,
	struct iphdr*		ptr_header
)
{
	/* Is the source IP address less than our source IP address. */
	if (ptr_header->saddr < ptr_network->ip_address.s_addr)
	{
		/* YES. We are no longer the querier on the network. */
		logMsg (IGMP_DEBUG, "DEBUG: Received Query from %x on network %s moving to NonQuerier.\n",
				ptr_header->saddr, ptr_network->ifaceName);

		/* Set the flag. */
		ptr_network->bIsQuerier = 0;

		/* Start the other querier present timer. */
		ptr_network->querier_present_timer = ptr_router->config.other_querier_present_interval;

		/* Reset the General Query timer. */
		ptr_network->general_query_timer = 0;
	}
	else
	{
		/* NO. We remain the querier. */
		logMsg (IGMP_DEBUG, "DEBUG: Received Query from %x on network %s but remain Querier.\n",
				ptr_header->saddr, ptr_network->ifaceName);

		/* Reset the other querier present timer */
		ptr_network->querier_present_timer = 0;
	}
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_network_socket_read_data
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called when data has been received on one of the network
 * 	interfaces. The function picks up the data, does basic validation,
 * 	determines the type and passes the packet to the appropriate handlers. 
 ***************************************************************************/
static void igmp_network_socket_read_data (IGMP_ROUTER_MCB*	ptr_router)
{
	int					num;
	int					len;
	struct iphdr*		ptr_ipheader;
	char				buffer[MAX_BUFFER_SIZE];
	unsigned int		src_address;
	struct igmphdr*		ptr_igmp_header;
	IGMP_NETWORK_IFACE*	ptr_network;
	struct in_pktinfo* 	ptr_iface_info = NULL;
	struct msghdr 		msg;
	struct cmsghdr*		cmsg;
	struct iovec 		iov;
	char 				ctrl[MAXCTRLSIZE];

	/* Read the data from the socket. But before doing so read the
	 * auxillary data, which indicates the device from where the
	 * packet was received. */

	/* Initialize the message structure. */
	memset ((void *)&msg, 0, sizeof (msg));

	/* Fill up the message structure. */	
	iov.iov_base 		= (char *)&buffer[0];
	iov.iov_len 		= MAX_BUFFER_SIZE;
	msg.msg_iov 		= &iov;
	msg.msg_iovlen		= 1;
	msg.msg_control 	= ctrl;
	msg.msg_controllen 	= MAXCTRLSIZE;

	/* Read the data from the socket. */
	num = recvmsg (ptr_router->igmp_router_socket, &msg, 0);
	if (num < 0)
	{
		logMsg (IGMP_FATAL,"IGMP Error: Unable to read IGMP data. Error Code:%d.\n",errno);
		return;
	}

	/* Cycle through the data and get packet information. */
	for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg,cmsg)) 
	{
		/* Is this the packet information ? */
		if (cmsg->cmsg_type == IP_PKTINFO)
			ptr_iface_info = (struct in_pktinfo *)CMSG_DATA(cmsg);
	}

	/* Make sure we got the interface information on which the packet was
	 * received. */
	if (ptr_iface_info == NULL)
	{
		logMsg (IGMP_FATAL, "IGMP Error: No packet information. Dropping packet !\n");
		return;
	}

	/* Cycle through all the interfaces, for a hit with the specific interface
	 * index. */
	ptr_network = (IGMP_NETWORK_IFACE *)list_get_head ((LIST_NODE **)&ptr_router->network_if_list);
	while (ptr_network != NULL)
	{
		/* Did we get a hit ? */
		if (ptr_network->iface_index == ptr_iface_info->ipi_ifindex)
			break;

		/* Get the next network interface. */		
		ptr_network = (IGMP_NETWORK_IFACE *)list_get_next ((LIST_NODE *)ptr_network);
	}

	/* Sanity Check -- We should have a hit ? If not then the IGMP Proxy
	 * module is not in sync. with the system ? */
	if (ptr_network == NULL)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to find iface matching index=%d.\n",
				ptr_iface_info->ipi_ifindex);
		return;			
	}

	/* Get the pointer to the start of the IP header. */
	ptr_ipheader = (struct iphdr*)&buffer[0];

	/* Get the length of the IGMP pakcet. */
	len = num - (ptr_ipheader->ihl << 2);

	/* Get the length of the IGMP packet. */
	if (len < IGMP_MINLEN)
	{
		logMsg (IGMP_FATAL,"IGMP Error: Rxed IGMP pkt:%d bytes is less than MIN length.\n",num);
		return;
	}

	/* Extract the SRC IP Address from the IP header */
    src_address = ptr_ipheader->saddr;

	/* Get the IGMP packet type. */
 	ptr_igmp_header = (struct igmphdr *)((char *)&buffer[0] + (ptr_ipheader->ihl << 2));

	/* Process on the basis of the packet type. */
	switch (ptr_igmp_header->type)
	{
		case IGMP_HOST_MEMBERSHIP_QUERY:
		{
			logMsg (IGMP_DEBUG,"DEBUG: Received Membership Query.\n");
			igmp_router_received_general_query (ptr_router, ptr_network, ptr_ipheader);
			break;
		}
		case IGMP_HOST_MEMBERSHIP_REPORT:
		{
			logMsg (IGMP_DEBUG,"DEBUG: Received Version 1 Membership Report.\n");
			igmp_router_version1_report_handler (ptr_router, ptr_network, ptr_igmp_header);
			break;
		}
		case IGMPV2_HOST_MEMBERSHIP_REPORT:
		{
			logMsg (IGMP_DEBUG,"DEBUG: Received Version 2 Membership Report.\n");
			igmp_router_version2_report_handler (ptr_router, ptr_network, ptr_igmp_header);
			break;
		}
		case IGMP_HOST_LEAVE_MESSAGE:
		{
			logMsg (IGMP_DEBUG,"DEBUG: Received Leave Report.\n");
			igmp_router_leave_handler (ptr_router, ptr_network, ptr_igmp_header);
			break;
		}
		default:
		{
			/* Error: Invalid packet type. Silently discard !*/
			break;
		}
	}
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_group_timer_expired
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the handler when the group timer expires. The function removes 
 *	the concerned group from the group database and places it back into the 
 *	free list. Because of the manipulation of the group database and free 
 *	list the function needs to return the next group entry that needs to be 
 *	processed.
 *	
 * RETURNS		 :
 *	Pointer to the next group entry in the database.
 ***************************************************************************/
static IGMP_GROUP* igmp_router_group_timer_expired 
(
	IGMP_ROUTER_MCB *		ptr_router,
	IGMP_NETWORK_IFACE*		ptr_net,
	IGMP_GROUP*				ptr_group
)
{
	IGMP_GROUP*				ptr_next_group;

	/* Debug Message */
	logMsg (IGMP_DEBUG, "DEBUG: Group timer expired.\n");

	/* NOTE: If we were in the CHECKING Membership state, we would have scheduled some
	 * group specific queries and set a group membership timer. The timer has gone off
	 * and we did not get any reports. So remove the membership completely and all 
	 * queries connected to it. We should have no queries that are scheduled, by the time 
	 * we reach here, because the group membership timer is more than the retransmission 
	 * timer * retransmission count. The retransmission timeout will take place before group 
	 * timer expires. */
	if (ptr_group->state == IGMP_ROUTER_CHECKING_MEMBERSHIP)
	{
		/* Concatenate the query list for the group back to the free list. */
		list_cat ((LIST_NODE **)&ptr_router->free_query_list,(LIST_NODE **)&ptr_group->query_list);

		/* We are done with the query list. */
		ptr_group->query_list = NULL;
	}

	/* Get the next group. -- This is required as the current group is about to be deleted. */
	ptr_next_group = (IGMP_GROUP*)list_get_next((LIST_NODE *)ptr_group);

	/* Remove the group from the group database list. */
	if (list_remove_node ((LIST_NODE **)&ptr_net->group_database,(LIST_NODE *)ptr_group) < 0)
	{
		logMsg (IGMP_FATAL,"IGMP Error: Group Removal of %s unsuccesful.\n",ptr_group->group_address);
		return NULL;
	}

	/* Proxy work - Check if we can drop group membership on the upstream interface. 
	 * If so do it. */
	igmp_proxy_drop_membership (ptr_router, ptr_group);	

	/* Before adding the group back to the free list. Initialize the state */
	ptr_group->state = IGMP_ROUTER_NO_MEMBERS_PRESENT;

	/* Add the expired group to the free list. */
	list_add ((LIST_NODE **)&ptr_router->free_group_list, (LIST_NODE *)ptr_group);

	/* Print the Current State of the route. */
	logMsg (IGMP_DEBUG, "Network:%s Group:%x State = No members present.\n",
			ptr_net->ifaceName, ptr_group->group_address);

	/* Return the next group that is to be processed. */
	return ptr_next_group;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_v1_host_timer_expired
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the handler when the v1 host timer expires
 ***************************************************************************/
static void igmp_router_v1_host_timer_expired 
(
	IGMP_ROUTER_MCB *		ptr_router,
	IGMP_NETWORK_IFACE*		ptr_net,
	IGMP_GROUP*				ptr_group
)
{
	/* DEBUG Message. */
	logMsg (IGMP_DEBUG, "DEBUG: IGMP v1 host timer expired.\n");

	/* Sanity Check -- The host timer can expire only when we are in the Version 1 members
	 * present state. All other states means our state machine is getting corrupted. */
	if (ptr_group->state != IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT)
	{
		logMsg (IGMP_FATAL,"IGMP Error: V1 host timer expired but group:%s for Network:%s in %s.\n",
				ptr_group->group_address, ptr_net->ifaceName, getGroupState(ptr_group));
		return;
	}

	/* Move the group to the Members present state. */
	ptr_group->state = IGMP_ROUTER_MEMBERS_PRESENT;
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_retransmit_timer_expired
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the handler when the retransmit timer expires. The function 
 *	removes the concerned group from the group database and places it back 
 *	into the free list. Because of the manipulation of the group database 
 *	and free list the function needs to return the next group entry that 
 *	needs to be processed.
 *	
 * RETURNS		 :
 *	Pointer to the next group entry in the database.
 ***************************************************************************/
static IGMP_GROUP* igmp_router_retransmit_timer_expired 
(
	IGMP_ROUTER_MCB *		ptr_router,
	IGMP_NETWORK_IFACE*		ptr_net,
	IGMP_GROUP*				ptr_group	
)
{
	IGMP_QUERY*		ptr_query;
	IGMP_GROUP*		ptr_next_group;

	/* Debug Message. */
	logMsg (IGMP_DEBUG, "DEBUG: Retransmission timer expired.\n");

	/* Check if there are any more queries that need to sent out. */
	ptr_query = (IGMP_QUERY *)list_remove ((LIST_NODE **)&ptr_group->query_list);
	if (ptr_query != NULL)
	{
		/* Send out the group specific queries. */
		logMsg (IGMP_DEBUG, "DEBUG: Sending out group specific query.\n");
		igmp_router_send_query (ptr_router, ptr_net, ptr_query->max_resp_time, ptr_query->group_address);

		/* Once the query has been sent out. Move the query back to the free list. */
		list_add ((LIST_NODE **)&ptr_router->free_query_list, (LIST_NODE *)ptr_query);

		/* Start the retransmission timer. */
		ptr_group->retransmit_timer = ptr_router->config.last_member_query_interval;

		/* The group is still alive. Retransmission timer has NOT expired. */
		return ptr_group;
	}
	else
	{
		/* There are no more queries. Retransmit Timer has expired and we are still in the 
		 * CHECKING Membership state. Clean up */

		/* Get the next group. -- This is required as the current group is about to be deleted. */
		ptr_next_group = (IGMP_GROUP*)list_get_next((LIST_NODE *)ptr_group);

		/* Remove the group from the group database list. */
		if (list_remove_node ((LIST_NODE **)&ptr_net->group_database,(LIST_NODE *)ptr_group) < 0)
		{
			logMsg (IGMP_FATAL,"Error: Group Removal of %s unsuccesful.\n",ptr_group->group_address);
			return NULL;
		}

		/* Proxy work - Check if we can drop group membership on the upstream interface. 
		 * If so do it. */
		igmp_proxy_drop_membership (ptr_router, ptr_group);

		/* Before adding the group back to the free list. Initialize the state */
		ptr_group->state = IGMP_ROUTER_NO_MEMBERS_PRESENT;

		/* Add the expired group to the free list. */
		list_add ((LIST_NODE **)&ptr_router->free_group_list, (LIST_NODE *)ptr_group);

		/* Print the Current State of the route. */
		logMsg (IGMP_DEBUG,"Network:%s Group:%x State = No members present.\n",
				ptr_net->ifaceName, ptr_group->group_address);

		/* The group has expired and has been deleted. Return the next entry ! */
		return ptr_next_group;
	}
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_timer_expired
 **************************************************************************
 * DESCRIPTION   :
 * 	This is the function that is called when the timer expires. The function
 *	cycles through all the interfaces and groups aging timers and calling the
 *	appropriate handlers for the timers which have expired.
 ***************************************************************************/
static void igmp_router_timer_expired (IGMP_ROUTER_MCB *ptr_router)
{
	IGMP_NETWORK_IFACE*	ptr_net;
	IGMP_GROUP*			ptr_group;	

	/* Cycle through all the network interfaces. */
	ptr_net=(IGMP_NETWORK_IFACE *)list_get_head((LIST_NODE **)&ptr_router->network_if_list);
	while (ptr_net != NULL)
	{
		/* Do not run the router on the upstream interface. */
		if (ptr_net == ptr_router->ptr_upstream_interface)
		{
			/* Get the next network interface. */
			ptr_net = (IGMP_NETWORK_IFACE*) list_get_next((LIST_NODE *)ptr_net);
			continue;
		}

		/* Check if we are the querier on the specific interface. */
		if (ptr_net->bIsQuerier == 1)
		{
			/* YES. We are the querier on this interface. Check if it is time to send
			 * out a general query. */
			if (ptr_net->general_query_timer <= 0)
			{
				/* General Query Timer has expired. Send out a general query. */
				logMsg (IGMP_DEBUG, "DEBUG: Sending out general query on network:%s.\n",
						ptr_net->ifaceName);

				/* Send out the query.*/
				igmp_router_send_query (ptr_router, ptr_net,
								        ptr_router->config.query_reponse_interval * 10, 0);

				/* Restart the general query timer. */
				ptr_net->general_query_timer = ptr_router->config.query_interval;
			}
			else
			{
				/* Decrement the general query timer. */
				ptr_net->general_query_timer = ptr_net->general_query_timer - 1;
			}
		}
		else
		{
			/* NO. We are not the querier on this interface. Has the other querier
			 * present timer expired ? */
			if (ptr_net->querier_present_timer <= 0)
			{
				/* YES. The other querier present timer has expired. Take over
				 * querier duties. */
				logMsg (IGMP_DEBUG, "DEBUG: Network %s We are now the querier.\n", ptr_net->ifaceName);
				ptr_net->bIsQuerier = 1;
			}
			else
			{
				/* Decrement the querier present timer. */
				ptr_net->querier_present_timer = ptr_net->querier_present_timer - 1;
			}
		}

		/* Cycle through all the groups for the specific network */
		ptr_group = (IGMP_GROUP*)list_get_head((LIST_NODE**)&ptr_net->group_database);
		while (ptr_group != NULL)
		{
			/* Sanity Check -- The group should be in one of the following state. */
			if (ptr_group->state < IGMP_ROUTER_NO_MEMBERS_PRESENT || 
				ptr_group->state > IGMP_ROUTER_CHECKING_MEMBERSHIP)
			{
				logMsg (IGMP_FATAL, "IGMP Error: Invalid group state %d for group:%x network:%s.\n",
						ptr_group->state, ptr_group->group_address, ptr_net->ifaceName);
				return;
			}

			/************************** RETRANSMISSION TIMERS *****************************/

			/* Retransmit timers should fire only when we are in the CHECKING MEMBERSHIP state.*/
			if (ptr_group->state == IGMP_ROUTER_CHECKING_MEMBERSHIP)
			{
				if (ptr_group->retransmit_timer <= 0)
				{
					/* Retransmit timer has gone off. */
					ptr_group  = igmp_router_retransmit_timer_expired (ptr_router, ptr_net, ptr_group);
					continue;
				}
				else
				{
					/* Decrement the retransmit timer. */
					ptr_group->retransmit_timer = ptr_group->retransmit_timer - 1;

					/* Goto the next group. We are finished with this group. */
					ptr_group = (IGMP_GROUP*)list_get_next((LIST_NODE *)ptr_group);
					continue;
				}
			}

			/************************** VERSION 1 HOST TIMER *****************************/

			/* Version 1 host timers should fire only in the VERSION 1 members
			 * present state. */
			if (ptr_group->state == IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT)
			{
				/* Check if the host timer has expired. */
				if (ptr_group->v1_host_timer <= 0)
				{
					/* The V1 host timer has expired. */
					igmp_router_v1_host_timer_expired (ptr_router, ptr_net, ptr_group);
				}
				else
				{
					/* Decrement the version 1 host timer. */
					ptr_group->v1_host_timer = ptr_group->v1_host_timer - 1;
				}

				/* Fall through.... */
			}

			/************************** GROUP TIMERS *****************************/

			/* Group timers should fire only if members are present. It could have
			 * either version 1 or version 2 members. */

			if (ptr_group->state == IGMP_ROUTER_MEMBERS_PRESENT  ||
				ptr_group->state == IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT)
			{
				/* Check if the group timer has expired. */
				if (ptr_group->group_timer <= 0)
				{
					/* The group timer has expired. */
					ptr_group = igmp_router_group_timer_expired (ptr_router, ptr_net, ptr_group);
				}
				else
				{
					/* Decrement the group timer. */
					ptr_group->group_timer = ptr_group->group_timer - 1;

					/* Print the Current State of the router. */
					logMsg (IGMP_DEBUG, "Network:%s Group:%x State = %s. Group Timer:%d Query Timer:%d\n",
								ptr_net->ifaceName, ptr_group->group_address, 
								getGroupState (ptr_group),ptr_group->group_timer,
							   	ptr_net->general_query_timer);
					logMsg (IGMP_DEBUG, "*****************************************************\n\n");

					/* Get the next group. */
					ptr_group = (IGMP_GROUP*)list_get_next((LIST_NODE *)ptr_group);
				}
			}
		}
		/* Get the next network interface. */
		ptr_net = (IGMP_NETWORK_IFACE*) list_get_next((LIST_NODE *)ptr_net);
	}
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_tick
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called every time the IGMP Timer expires which will be
 * 	once every second.		
 ***************************************************************************/
static void igmp_router_tick(void)
{
	char	*buffer = "IGMPTimer";

	/* Write to the pipe indicating that the timer has expired. */
	if (write (timerPipe[1], buffer, strlen (buffer)) < 0)
		logMsg (IGMP_FATAL,"IGMP Error: IGMP Timer write failed.\n");
	return;
}

/**************************************************************************
 * FUNCTION NAME : main
 **************************************************************************
 * DESCRIPTION   :
 * 	Entry point for the IGMP proxy implementation. 
 ***************************************************************************/
int main (int argc, char *argv[])
{
	int					index;
	int 				socket_option;
	unsigned char 		opt;
	fd_set 				allset;
	fd_set 				rset;
	IGMP_GROUP*			ptr_group;
	IGMP_QUERY*			ptr_query;
	int 				num;
	int					maxfd = 0;
	struct itimerval	timer, otimer;
	char				timerBuffer[20];
	char*				ptr_upstream_address = NULL;
	char*				ptr_upstream_ifaceName = NULL;

	/* Validate the arguments that are passed to the module. */
	if (argc < 3 || argc > 4)
	{
		print_usage ();
		return -2;
	}

	/* Initialize the IGMP router control block. */
	memset ((void *)&igmp_router, 0, sizeof (igmp_router));

	/* Parse the options and validate them. */
	index = 1;
	while (index < argc)
	{
		/* Check if the user selected verbose mode. */
		if (strcasecmp (argv[index], "-v") == 0)
		{
			/* Set the proxy debug level on. */
			proxyDebugLevel	= IGMP_DEBUG;
		}
		else
		{		
 			/* Is this the name of the upstream interface ? */
			if (strcasecmp (argv[index], "-i") == 0)
			{
				/* Remember the upstream interface name. */
				ptr_upstream_ifaceName = argv[index + 1];

				/* Go onto the next argument. */
				index = index + 1;
			}
			else
			{
				/* Incorrect Parameter. */
				print_usage ();
				return -2;
			}
		}
		index = index + 1;
	}

	/* Make sure that the user has entered the correct addresses. */
	if (ptr_upstream_ifaceName == NULL)
	{
		print_usage ();
		return -2;
	}
	
	/* Configure the IGMP router control block with the default values. */
	igmp_router.config.other_querier_present_interval = DEFAULT_OTHER_QUERIER_PRESENT;
	igmp_router.config.query_interval				  = DEFAULT_QUERY_INTERVAL	;
	igmp_router.config.query_reponse_interval		  = DEFAULT_QUERY_RESPONSE_INTERVAL;
	igmp_router.config.group_membership_interval 	  = DEFAULT_GROUP_INTERVAL;
	igmp_router.config.unsolicited_report_interval    = DEFAULT_UNSOLICITED_REPORT_INTERVAL;
	igmp_router.config.last_member_query_interval     = DEFAULT_LAST_MEMBER_QUERY_INTERVAL;
	igmp_router.config.last_member_query_count        = DEFAULT_LAST_MEMBER_QUERY_COUNT;
	igmp_router.config.robustness_variable 			  = DEFAULT_ROBUSTNESS;
	igmp_router.config.startup_query_interval 		  = DEFAULT_STARTUP_QUERY_INTERVAL;
	igmp_router.config.startup_query_count 			  = DEFAULT_STARTUP_QUERY_COUNT;
	igmp_router.config.v1_host_present_interval		  = DEFAULT_VERSION1_ROUTER_PRESENT;

	/* Store the configured parameters in the control block. */
	strcpy (igmp_router.config.upstream_ifaceName, ptr_upstream_ifaceName);
	
	/* Create the IGMP router socket. */	
	igmp_router.igmp_router_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
	if (igmp_router.igmp_router_socket < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to open IGMP Raw socket.\n");
		return -1;
	}

	/* Set the socket options. */
	socket_option = 1;
   	if (setsockopt(igmp_router.igmp_router_socket, IPPROTO_IP, MRT_INIT, 
				   (char *)&socket_option, sizeof(int)) < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to set MRT_INIT.\n");
		return -1;
	}
	setsockopt (igmp_router.igmp_router_socket, SOL_SOCKET, SO_REUSEADDR,
				(void*)&socket_option, sizeof(socket_option));
	setsockopt (igmp_router.igmp_router_socket, IPPROTO_IP, IP_PKTINFO, 
				(void*)&socket_option, sizeof(socket_option));
	opt = 1;
	setsockopt (igmp_router.igmp_router_socket, IPPROTO_IP, IP_MULTICAST_TTL,
				(void*)&opt, sizeof(opt));
	opt = 0;
	setsockopt (igmp_router.igmp_router_socket, IPPROTO_IP, IP_MULTICAST_LOOP, 
				(void*)&opt, sizeof(opt));

	/* Initialize the pre-allocated group list. */
	for (index = 0; index < MAX_NUMBER_GROUPS; index ++)
	{
		/* Allocate memory for the IGMP group. */
		ptr_group = (IGMP_GROUP *)my_malloc (sizeof (IGMP_GROUP));
		if (ptr_group == NULL)
		{
			logMsg (IGMP_FATAL, "IGMP Error: Unable to allocate memory for IGMP groups.\n");
			return -1;
		}

		/* Initialize the memory block. */
		memset ((void *)ptr_group, 0, sizeof (IGMP_GROUP));

		/* Initially there are no members present. */
		ptr_group->state = IGMP_ROUTER_NO_MEMBERS_PRESENT;

		/* Add the group to the free list. */
		list_add ((LIST_NODE **)&igmp_router.free_group_list, (LIST_NODE *)ptr_group);
	}

	/* Initialize the pre-allocated query list. */
	for (index = 0; index < MAX_NUMBER_QUERIES; index ++)
	{
		/* Allocate memory for the IGMP group. */
		ptr_query = (IGMP_QUERY *)my_malloc (sizeof (IGMP_QUERY));
		if (ptr_query == NULL)
		{
			logMsg (IGMP_FATAL, "IGMP Error: Unable to allocate memory for IGMP queries.\n");
			return -1;
		}

		/* Initialize the memory block. */
		memset ((void *)ptr_query, 0, sizeof (IGMP_QUERY));

		/* Add the group to the free list. */
		list_add ((LIST_NODE **)&igmp_router.free_query_list, (LIST_NODE *)ptr_query);
	}

	/* Set the signal handlers to catch termination of the module. */
	signal(SIGKILL, (void *)igmp_router_kill);
	signal(SIGTERM, (void *)igmp_router_kill);
	signal(SIGHUP,  (void *)igmp_router_kill);

	/* Create the network interface database. */
	if (igmp_create_network_list(&igmp_router) < 0)
		return -3;

	/* Sanity check: Make sure we got a hit on the upstream interface. */
	if (igmp_router.ptr_upstream_interface == NULL)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to locate upstream interface. Aborting !\n");
		return -2;
	}

	/* Create the readset on which we will wait for raw IGMP data. */
	FD_ZERO(&rset);

	/* Create a pipe for sending timer messages. */
	if (pipe(timerPipe) < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to create network timer.\n");
		return -1;
	}

	/* Once the timer pipe has been created add it to the read set. */
	FD_SET(timerPipe[0], &rset);
	if (maxfd < timerPipe[0])
		maxfd = timerPipe[0];

	/* Add the created multicast router socket to the list of descriptors. */
	FD_SET(igmp_router.igmp_router_socket, &rset);
	if (maxfd < igmp_router.igmp_router_socket)
		maxfd = igmp_router.igmp_router_socket;

	/* Create a copy of the readset. */
	memcpy ((void *)&allset, (void *)&rset, sizeof (fd_set));

	/* Set the timer parameters */
	timer.it_interval.tv_sec  = 1;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec     = 1;
	timer.it_value.tv_usec    = 0;

	/* Set up the signal catcher for the alarm clock */
	signal(SIGALRM,(void *)igmp_router_tick);

	/* Initialize the timer. */
	if (setitimer(ITIMER_REAL,&timer,&otimer) < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to create the timer.\n");
		return -1;
	}

	/* The timer has been created succesfully. */
	logMsg (IGMP_DEBUG, "DEBUG: IGMP Timer Created.\n");

	/* The IGMP proxy is running. */
	IGMPProxyRunning = 1;

	/* Loop indefinately. */
	while (IGMPProxyRunning)
	{
		/* Wait for data to arrive on the sockets. */
		num = select(maxfd+1, &rset, NULL, NULL, NULL);

		/* We could be out of the select call because of :
		 * 	1. SIGALARM Generation. Fails with error code EINTR
		 * 	2. Data has arrived on the sockets. */
		if (num < 0)
		{
			/* Error: Select failed. */
			if (errno == EINTR)
			{
				/* YES -- Check if this is because of our SIG_ALARM. */
				if (FD_ISSET (timerPipe[0], &rset))
				{
					/* Timer expired. Read the pipe message. */
					if (read (timerPipe[0], &timerBuffer[0], sizeof(timerBuffer)) < 0)
					{
						logMsg (IGMP_FATAL, "IGMP Error: Read on timer pipe failed.\n");
						break;
					}
					else
					{					
						/* Handle IGMP Timer expiration. */
						igmp_router_timer_expired (&igmp_router);
						continue;
					}
				}
				else
				{
					/* NO -- This is not because of our signal. */
					logMsg (IGMP_FATAL,"IGMP Error: Exiting Application - No data on pipe.");
					break;
				}				
			}
			else
			{
				/* Failure - Select failed. */
				logMsg (IGMP_FATAL,"IGMP Error: Select failed Error Number:%d.",errno);
				break;
			}
		}
		
		/* Select was successful. Was timer expiration one of the events ? */
		if (FD_ISSET (timerPipe[0], &rset))
		{
			/* Timer expired. Read the pipe message. */
			if (read (timerPipe[0], &timerBuffer[0], sizeof(timerBuffer)) < 0)
			{
				logMsg (IGMP_FATAL, "IGMP Error: Read on timer pipe failed.\n");
				break;
			}

			/* Handle IGMP Timer expiration. */
			igmp_router_timer_expired (&igmp_router);

			/* Decrement the number of sockets already handled. */
			num = num - 1;
		}
		
		/* Make sure there is data pending on the multicast router socket. */
		if (FD_ISSET (igmp_router.igmp_router_socket, &rset))
		{
			/* Read the data from the socket. */
			igmp_network_socket_read_data (&igmp_router);
			
			/* Decrement the number of sockets already handled. */
			num = num - 1;
		}

		/* Sanity Check - There should be no pending data on any of the
		 * descriptors. */
		if (num != 0)
			logMsg (IGMP_FATAL,"IGMP Error: Num of descriptors=%d.\n",num);

		/* Copy the read set. */
		FD_ZERO(&rset);
		memcpy ((void *)&rset, (void *)&allset, sizeof (fd_set));
	}

	/* Stop the IGMP proxy. */
	igmp_proxy_stop (&igmp_router);
	return 0;
}

/**************************************************************************
 * FUNCTION NAME : igmp_create_network_list
 **************************************************************************
 * DESCRIPTION   :
 * 	Creates a network list of all interfaces that are present in the system
 * 	and attaches it to the IGMP proxy module.
 * 	
 * RETURNS		 :
 * 	 < 0		- Error.
 * 	 0			- Success.
 ***************************************************************************/
static int igmp_create_network_list(IGMP_ROUTER_MCB *ptr_igmp_router)
{
	struct ifreq* 		ptr_ifr;
	struct ifreq 		ifr;
	int					sock;
	int					index;
	struct ip_mreqn 	mreq;
	struct vifctl 		vifctl;
	IGMP_NETWORK_IFACE*	ptr_network;
	struct sockaddr_in*	ptr_sockaddress;
	struct in_addr 		ifaddr;
	struct sockaddr_in	netmask;
	struct sockaddr_in 	dstaddr;
	short int 			flags;
	struct ifconf		ifcfg;
	char*				limit;
	char*				cp;
	char				buffer[MAX_BUFFER_SIZE];
	int					upstream = 0;	/* 0 - Indicates a DOWNSTREAM and 1 an UPSTREAM interface */

	/* Create a socket just for retrieving network list. */
	sock = socket (PF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to open IGMP socket for net iface.\n");
		return -1;
	}

	/* Initialize the structure. */
	memset ((void *)&ifcfg, 0, sizeof(ifcfg));
	ifcfg.ifc_len 	= sizeof (buffer);
	ifcfg.ifc_buf 	= buffer;
	
	/* Get list of network interfaces present in the system. */
    if (ioctl(sock, SIOCGIFCONF, (void*)&ifcfg) < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to get the list of network interfaces Err:%d.\n",errno);
		return -1;
	}

	/* Get the pointer to the last interface data that can be present. */
	limit = buffer + ifcfg.ifc_len;

	/* Cycle through all the interfaces */
	for (cp = buffer; cp < limit; cp = cp + sizeof (ptr_ifr->ifr_name) + sizeof(ptr_ifr->ifr_ifru))
	{
		/* Get the pointer to the interface request block. */
		ptr_ifr = (struct ifreq *)cp;

		/* Get the pointer to the socket address. */
		ptr_sockaddress = (struct sockaddr_in *)&ptr_ifr->ifr_ifru.ifru_addr;

		/* Get the index. */
		strcpy (ifr.ifr_name, ptr_ifr->ifr_name);
        if (ioctl(sock, SIOCGIFINDEX, (void*)&ifr) < 0)
		{
			logMsg (IGMP_DEBUG, "DEBUG: Unable to get index for %s.\n",ifr.ifr_name);
			continue;
        }
		index = ifr.ifr_ifindex;

		/* Get the net mask. */
		strcpy (ifr.ifr_name, ptr_ifr->ifr_name);
        if (ioctl(sock, SIOCGIFNETMASK, (void*)&ifr) < 0)
		{
			logMsg (IGMP_DEBUG, "DEBUG: Unable to get netmask for %s.\n",ifr.ifr_name);
			continue;
        }
		memcpy((void *)&netmask, (void *)&ifr.ifr_netmask, sizeof (netmask));

		/* We save the current device flags of each of the interfaces on the
		 * system. Enable multicasting and promiscuous mode of each of the
		 * interfaces and set the flags back. On exiting the proxy module, we
		 * can restore these flags back to the orignal values. */
		strcpy (ifr.ifr_name, ptr_ifr->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, (void*)&ifr) < 0)
		{
			logMsg (IGMP_FATAL, "IGMP Error: Unable to get interface flags for %s Error:%d.\n",
					ifr.ifr_name,errno);
		}
		flags = ifr.ifr_flags;

		/* Skip loopback devices. */
		if (flags & IFF_LOOPBACK)
			continue;

		/* FIX: For IGMP proxy not detecting PPP as an upstream interface. For a PPP connection
		 * the netmask is defined as 255.255.255.255, so we cannot use it to detect the 
		 * upstream router address. */
		if (flags & IFF_POINTOPOINT)
		{
			/* Point to Point Link. */
			logMsg (IGMP_DEBUG, "DEBUG: Detected %s as Point to Point interface.\n", 
					ifr.ifr_name);

			/* Detect the address of the other end of the PPP link. */
			strcpy (ifr.ifr_name, ptr_ifr->ifr_name);
			if (ioctl(sock, SIOCGIFDSTADDR,(void *)&ifr) < 0)
			{
				logMsg (IGMP_FATAL, "IGMP Error: Unable to get peer address for %s Error:%d.\n",
							ifr.ifr_name, errno);
			}
			else
			{
				memcpy ((void *)&dstaddr, (void *)&ifr.ifr_dstaddr, sizeof(struct sockaddr));
			}
		}
		else
		{
			/* Set the device flag to enable reception of all packets but do not do so for 
			 * PPP connections */
		    strcpy (ifr.ifr_name, ptr_ifr->ifr_name);
			ifr.ifr_flags = ifr.ifr_flags | IFF_ALLMULTI | IFF_PROMISC;
			if (ioctl(sock, SIOCSIFFLAGS, (void*)&ifr) < 0)
			{
				logMsg (IGMP_FATAL, "IGMP Error: Unable to set interface flags for %s.\n",
						ifr.ifr_name);
			}
		}
		
		/* Connect the interface to the IGMP proxy module. */
		ptr_network = (IGMP_NETWORK_IFACE *)my_malloc (sizeof (IGMP_NETWORK_IFACE));
		if (ptr_network == NULL)
		{
			logMsg (IGMP_FATAL, "IGMP Error: Unable to allocate memory.\n");
			return -1;
		}
		/* Initialize the allocated memory block. */	
		memset ((void *)ptr_network, 0, sizeof (IGMP_NETWORK_IFACE));

		/* Configure the network interface block. */
		strcpy (ptr_network->ifaceName, ifr.ifr_name);

		/* Remember the system index. */
		ptr_network->iface_index = index;

		/* Store the IP Address. */
		memcpy ((void *)&ptr_network->ip_address,(void *)&ptr_sockaddress->sin_addr,
				sizeof(struct in_addr));

		/* Store the orignal device flags. */
		ptr_network->flags = flags;
				
		/* Initially we support version 2 on this interface. */
		ptr_network->igmp_version = IGMP_VERSION2;

		/* By default a router will always startup as querier. */
		ptr_network->bIsQuerier   = 1;

		/* Make sure the the first query is sent out as fast as possible. */
		ptr_network->general_query_timer = 0;

		/* Create an IGMP raw socket for each interface. */
		ptr_network->socket = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
		if (ptr_network->socket < 0)
		{
			logMsg (IGMP_FATAL, "IGMP Error: Unable to open IGMP Raw socket for interface:%s.\n",
					ptr_network->ifaceName);
			return -1;
		}

		/* Set the multicast interface on the socket. */
		memcpy ((void *)&ifaddr,(void *)&ptr_network->ip_address, sizeof (struct in_addr));
		setsockopt(ptr_network->socket, IPPROTO_IP, IP_MULTICAST_IF, 
					(void*)&ptr_network->ip_address, sizeof(ifaddr));

		/* Instead of using the IP Address and subnet mask to detect the upstream 
		 * interface, we shall use the "Interface Name" for UPSTREAM interface detection. */
		if (strcmp (ptr_igmp_router->config.upstream_ifaceName, ptr_network->ifaceName) == 0)
		{
			/* This is an UPSTREAM interface. */
			upstream = 1;
		}
		else
		{
			/* This is a DOWNSTREAM interface. */
			upstream = 0;
		}

		/* Check if this interface is DOWNSTREAM / UPSTREAM ? */
		if (upstream == 1)
		{
			/* This is the UPSTREAM interface. */
			ptr_network->mode = UPSTREAM_INTERFACE;

			/* Remember the entry. */
			ptr_igmp_router->ptr_upstream_interface = ptr_network;

			/* Print Debug Information. */
			logMsg (IGMP_DEBUG, "DEBUG: Detected %s as UPSTREAM interface.\n", 
					ptr_network->ifaceName);
		}
		else
		{
			/* This is the DOWNSTREAM interface. */
			ptr_network->mode = DOWNSTREAM_INTERFACE;

			/* For each of the downstream interfaces become a member of the
			 * ALL router club. */
			mreq.imr_ifindex		  = ptr_network->iface_index; 
			mreq.imr_multiaddr.s_addr = htonl(INADDR_ALLRTRS_GROUP);
			mreq.imr_address.s_addr   = htonl(ptr_sockaddress->sin_addr.s_addr);
			if (setsockopt(ptr_network->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
						  (void*)&mreq, sizeof(mreq)) < 0)
			{
				logMsg (IGMP_FATAL, "IGMP Error: Unable to add router membership for net:%s Error:%d\n", 
						ptr_network->ifaceName,errno);
			}

			/* Print Debug Information. */
			logMsg (IGMP_DEBUG, "DEBUG: Detected %s as DOWNSTREAM and added ALL Router membership.\n", 
					ptr_network->ifaceName);
		}
		
		/* Create a virtual interface entry for the interface. */
		vifctl.vifc_vifi			= ptr_network->iface_index;
		vifctl.vifc_flags			= 0;
		vifctl.vifc_threshold   	= 0;
		vifctl.vifc_rate_limit		= 0;
		vifctl.vifc_lcl_addr.s_addr	= ptr_sockaddress->sin_addr.s_addr;
		vifctl.vifc_rmt_addr.s_addr = INADDR_ANY;
		if (setsockopt(ptr_igmp_router->igmp_router_socket, IPPROTO_IP, MRT_ADD_VIF, 
					   (char *)&vifctl, sizeof(vifctl)) < 0)
		{
			/* Error: Unable to create the VIF entry. */
			logMsg (IGMP_FATAL, "IGMP Error: Adding VIF failed for network:%s Error:%d.\n",
					ptr_network->ifaceName,errno);
			ptr_network->vif_index = -1;
		}
		else
		{
			/* Remember the index of the created virtual interface entry. */
			ptr_network->vif_index = ptr_network->iface_index;
		}

		/* Add to the list. */
		list_add ((LIST_NODE **)&ptr_igmp_router->network_if_list, (LIST_NODE *)ptr_network);	
	}
	/* Close the socket.*/
	close (sock);
	return 0;
}

/**************************************************************************
 * FUNCTION NAME : igmp_router_kill
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is the registered signal handler that is called when the 
 * 	IGMP proxy is being killed. 
 ***************************************************************************/
static void igmp_router_kill (void)
{
	/* Indicate that we need to stop the IGMP proxy module. */
	IGMPProxyRunning = 0;
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_proxy_clean_interface 
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called to clean the network interface and restore any
 * 	changes that were made to the device by the proxy module.
 ***************************************************************************/
static void igmp_proxy_clean_interface 
(
	IGMP_ROUTER_MCB*		ptr_router,
	IGMP_NETWORK_IFACE*		ptr_net
)
{
	IGMP_GROUP*				ptr_group;
	struct ifreq 			ifr;
	struct vifctl 			vifctl;
	int						sock;

	/* Print Debug Information */
	logMsg (IGMP_DEBUG, "DEBUG: Cleaning interface:%s.\n",ptr_net->ifaceName);

	/* Create a socket just for closing. */
	sock = socket (PF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to open socket for cleaning up.\n");
		return;
	}	

	/* Cycle through all the group members for that interface */
	ptr_group = (IGMP_GROUP *)list_remove ((LIST_NODE **)&ptr_net->group_database);
	while (ptr_group != NULL)
	{
		/* Drop membership and cleanup MFC for the group. */
		igmp_proxy_drop_membership (ptr_router, ptr_group);
			
		/* Concatenate the query list for the group back to the free list. */
		list_cat ((LIST_NODE **)&ptr_router->free_query_list,(LIST_NODE **)&ptr_group->query_list);

		/* Add the group member back to the free list. */
		list_add ((LIST_NODE **)&ptr_router->free_group_list,(LIST_NODE *)ptr_group);

		/* Get the next element. */
		ptr_group = (IGMP_GROUP *)list_remove ((LIST_NODE **)&ptr_net->group_database);
	}

	/* Restore device flags. */
	ifr.ifr_flags = ptr_net->flags;
	strcpy(ifr.ifr_name, ptr_net->ifaceName);
	if (ioctl(sock, SIOCSIFFLAGS, (void*)&ifr) < 0)
	{
		logMsg (IGMP_FATAL, "IGMP Error: Unable to restore interface flags for %s.\n",
				ptr_net->ifaceName);
	}

	/* Delete the VIF Entry created for the interface. */	
	memset ((void *)&vifctl, 0, sizeof (vifctl));
	vifctl.vifc_vifi = ptr_net->vif_index;
	if (setsockopt(ptr_router->igmp_router_socket, IPPROTO_IP, MRT_DEL_VIF, 
				   (char *)&vifctl, sizeof(vifctl)) < 0)
	{
		/* Error: Unable to delete the VIF entry. */
		logMsg (IGMP_FATAL, "IGMP Error: Deleting VIF failed for network:%s Error:%d.\n",
				ptr_net->ifaceName,errno);
	}

	/* Close the sockets. */
	close (sock);	
	close (ptr_net->socket);	
	return;
}

/**************************************************************************
 * FUNCTION NAME : igmp_proxy_stop
 **************************************************************************
 * DESCRIPTION   :
 * 	Cleanup and stop the IGMP proxy
 ***************************************************************************/
static void igmp_proxy_stop (IGMP_ROUTER_MCB *ptr_router)
{
	IGMP_NETWORK_IFACE*		ptr_net;
	IGMP_GROUP*				ptr_group;
	IGMP_QUERY*				ptr_query;

	/* Print Debug information. */
	logMsg (IGMP_DEBUG,"DEBUG: Stopping IGMP Proxy.\n");
	
	/* Cycle through all the network interfaces. */
	ptr_net = (IGMP_NETWORK_IFACE *) list_remove ((LIST_NODE **)&ptr_router->network_if_list);
	while (ptr_net != NULL)
	{
		/* Clean uptsream interface right at the end. This is required because
		 * we need the socket on the upstream interface to be valid to drop
		 * memberships on the upstream. We do maintain a cached entry to the
		 * interface. Use that to clean memory. */
		if (ptr_net->mode == UPSTREAM_INTERFACE)
		{
			/* Get the next network interface */
			ptr_net = (IGMP_NETWORK_IFACE *) list_remove ((LIST_NODE **)&ptr_router->network_if_list);
			continue;
		}

		/* Clean and restore the interface. */
		igmp_proxy_clean_interface (ptr_router, ptr_net);

		/* Clean up the memory for the network interface */
		my_free (ptr_net);

		/* Get the next network interface */
		ptr_net = (IGMP_NETWORK_IFACE *) list_remove ((LIST_NODE **)&ptr_router->network_if_list);
	}

	/* Clean and restore the upstream interface. */
	igmp_proxy_clean_interface (ptr_router, ptr_router->ptr_upstream_interface);	
	my_free (ptr_router->ptr_upstream_interface);

	/* Once the network interfaces have been cleaned up. Clean the free query list. */
	ptr_query = (IGMP_QUERY *)list_remove ((LIST_NODE **)&ptr_router->free_query_list);
	while (ptr_query != NULL)
	{
		my_free (ptr_query);
		ptr_query = (IGMP_QUERY *)list_remove ((LIST_NODE **)&ptr_router->free_query_list);
	}

	/* Clean up the free group list. */
	ptr_group = (IGMP_GROUP *)list_remove ((LIST_NODE **)&ptr_router->free_group_list);
	while (ptr_group != NULL)
	{
		my_free (ptr_group);
		ptr_group = (IGMP_GROUP *)list_remove ((LIST_NODE **)&ptr_router->free_group_list);
	}

	/* We are done with the multicast routing socket.. */
    if (setsockopt(ptr_router->igmp_router_socket, IPPROTO_IP, MRT_DONE, (char *)NULL, 0) < 0)
		logMsg (IGMP_FATAL, "IGMP Error: Unable to close multicast socket. Error:%d.\n",errno);

	/* Make sure that there no more memory leaks. */
	if (malloc_counter != free_counter)
		logMsg (IGMP_FATAL, "IGMP Error: Memory leaks. Malloc:%d Free:%d.\n",malloc_counter, free_counter);
	else
		logMsg (IGMP_DEBUG, "DEBUG: No memory leaks detected.\n");

	logMsg (IGMP_DEBUG, "DEBUG: IGMP Proxy was using %d bytes.\n",malloced_size);	
	return;
}

/************************** Utility Functions ******************************/

/**************************************************************************
 * FUNCTION NAME : igmp_router_group_lookup
 **************************************************************************
 * DESCRIPTION   :
 * 	Searches the group database on an interface for a specific group member.
 ***************************************************************************/
static IGMP_GROUP* igmp_router_group_lookup 
(
	IGMP_NETWORK_IFACE*	ptr_net, 
	unsigned int		group_address
)
{
	IGMP_GROUP*	ptr_group;

	/* Get the head of the list. */
	ptr_group = (IGMP_GROUP *)list_get_head ((LIST_NODE **)&ptr_net->group_database);
	while (ptr_group != NULL)
	{
		if (ptr_group->group_address == group_address)
			return ptr_group;
		ptr_group = (IGMP_GROUP *) list_get_next ((LIST_NODE *)ptr_group);
	}
	return NULL;
}

/**************************************************************************
 * FUNCTION NAME : logMsg
 **************************************************************************
 * DESCRIPTION   :
 * 	Logging Message Utility function. Prints messages only if the specified
 * 	level is less than the logging level of the module.
 ***************************************************************************/
static void logMsg (int level,char* fmt,...)
{
	va_list arg;
	
	/* Check if the message needs to be logged ? */	
	if (level > proxyDebugLevel)
		return;

	/* YES. The Message DEBUG level is greater than the module debug level. */

#ifdef SYS_LOG
	{
		/* Use the system log to log messages */
		int	systemLevel = LOG_ERR;

		/* Convert our debug levels to Linux levels. */
		switch (level)
		{
			case IGMP_DEBUG: 
				systemLevel = LOG_DEBUG;
				break;
			case IGMP_FATAL: 
				systemLevel = LOG_FATAL;
				break;
		}
		va_start (arg, fmt);
		syslog(systemLevel, fmt, arg);
		va_end (arg);
	}
#else
	/* Print the messages on the console. */
	va_start (arg, fmt);
	vprintf (fmt, arg);
	va_end (arg);
#endif
	return;
}

/**************************************************************************
 * FUNCTION NAME : printRouterState
 **************************************************************************
 * DESCRIPTION   :
 *	Utility function which states the current state of the router.
 ***************************************************************************/
static char *getGroupState (IGMP_GROUP* ptr_group)
{
	char *state_message[] =
	{
		"No Members Present state",
		"Members Present state",
		"Version 1 Members present state",
		"Checking Membership state",
		NULL
	};

	if (ptr_group->state > IGMP_ROUTER_CHECKING_MEMBERSHIP)
		logMsg (IGMP_FATAL, "Error: Incorrect IGMP router state :%d.\n", ptr_group->state);

	return state_message[ptr_group->state];
}

/**************************************************************************
 * FUNCTION NAME : in_cksum 
 **************************************************************************
 * DESCRIPTION   :
 * 	Compute the inet checksum
 ***************************************************************************/
static unsigned short in_cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    answer = ~sum;
    return (answer);
}

/**************************************************************************
 * FUNCTION NAME : my_malloc 
 **************************************************************************
 * DESCRIPTION   :
 * 	Memory allocation, but keeps track of the number of times malloc is 
 * 	called. Used for detection of memory leaks. 
 ***************************************************************************/
static void* my_malloc (int size)
{
	malloc_counter ++;
	malloced_size = malloced_size + size;
	return ((void *)(malloc (size)));
}

/**************************************************************************
 * FUNCTION NAME : my_free 
 **************************************************************************
 * DESCRIPTION   :
 * 	Cleaning up the allocated block of memory. Keeps track of the number of
 * 	times free is called. Used for memory leak detection.
 ***************************************************************************/
static void my_free (void *ptr)
{
	free_counter ++;
	free (ptr);
}

/**************************************************************************
 * FUNCTION NAME : print_usage 
 **************************************************************************
 * DESCRIPTION   :
 * 	Print the usage parameters of IGMP Proxy.
 ***************************************************************************/
static void print_usage (void)
{
	printf ("Usage: IGMPProxy [-v] -i <interface name>.\n");
	printf ("-i <interface name>  : Name of the interface on which the router is present\n");
	printf ("-v                   : Verbose mode - Print debug messages\n");
	printf ("Example: IGMPProxy -v -i nas0\n\n");
	return;
}

/**************************************************************************
 * FUNCTION NAME : xdump 
 **************************************************************************
 * DESCRIPTION   :
 * 	Generic utilty which dumps the contents of a specific memory address.
 * 	Only for debugging purposes.
 ***************************************************************************/
#define isprint(a) ((a >=' ')&&(a <= '~')) 
static void xdump( u_char*  cp, int  length, char*  prefix )
{
    int col, count;
    u_char prntBuf[120];
    u_char*  pBuf = prntBuf;
    count = 0;
    while(count < length){
        pBuf += sprintf( pBuf, "%s", prefix );
        for(col = 0;count + col < length && col < 16; col++){
            if (col != 0 && (col % 4) == 0)
                pBuf += sprintf( pBuf, " " );
            pBuf += sprintf( pBuf, "%02X ", cp[count + col] );
        }
        while(col++ < 16){      /* pad end of buffer with blanks */
            if ((col % 4) == 0)
                sprintf( pBuf, " " );
            pBuf += sprintf( pBuf, "   " );
        }
        pBuf += sprintf( pBuf, "  " );
        for(col = 0;count + col < length && col < 16; col++){
            if (isprint((int)cp[count + col]))
                pBuf += sprintf( pBuf, "%c", cp[count + col] );
            else
                pBuf += sprintf( pBuf, "." );
                }
        sprintf( pBuf, "\n" );
        printf(prntBuf);
        count += col;
        pBuf = prntBuf;
    }
}

