/**************************************************************************
 * FILE PURPOSE	:  	Header file for IGMP Proxy Module.
 **************************************************************************
 * FILE NAME	:   igmp_proxy.h
 *
 * DESCRIPTION	:
 * 	Contains structures and exported function that are used by the IGMP 
 * 	proxy module.
 *
 *	(C) Copyright 2002, Texas Instruments, Inc.
 *************************************************************************/

#ifndef __IGMP_PROXY__
#define __IGMP_PROXY__

#include "listlib.h"

/* Logging Levels for the module. */
#define	IGMP_FATAL								0
#define IGMP_DEBUG								10

/* Default Configuration values as specified by the RFC 2236. */
#define DEFAULT_ROBUSTNESS						2
#define DEFAULT_QUERY_INTERVAL					125

/* This is in seconds. The value inserted into the group query is 100. i.e. 10*10. */
#define DEFAULT_QUERY_RESPONSE_INTERVAL			10

#define DEFAULT_GROUP_INTERVAL					((2 * 125) + (DEFAULT_QUERY_RESPONSE_INTERVAL * 10))
#define DEFAULT_OTHER_QUERIER_PRESENT			((2 * 125) + (DEFAULT_QUERY_RESPONSE_INTERVAL * 10) / 2)
#define DEFAULT_STARTUP_QUERY_INTERVAL			32
#define DEFAULT_STARTUP_QUERY_COUNT				DEFAULT_ROBUSTNESS

/* This is in seconds. The value inserted into the group specific query is 10 i.e. 10*1.*/
#define DEFAULT_LAST_MEMBER_QUERY_INTERVAL		1

#define DEFAULT_LAST_MEMBER_QUERY_COUNT			DEFAULT_ROBUSTNESS
#define DEFAULT_UNSOLICITED_REPORT_INTERVAL		10
#define DEFAULT_VERSION1_ROUTER_PRESENT			400

/* IGMP versions */
#define IGMP_VERSION1							1
#define IGMP_VERSION2							2

/* Mode for the IGMP interface.*/
#define UPSTREAM_INTERFACE						1
#define DOWNSTREAM_INTERFACE					2

/* Limit Definations */
#define MAX_NUMBER_GROUPS						128
#define MAX_NUMBER_QUERIES						100

enum IGMP_GROUP_STATE
{
	IGMP_ROUTER_NO_MEMBERS_PRESENT = 0,
	IGMP_ROUTER_MEMBERS_PRESENT,
	IGMP_ROUTER_VERSION_1_MEMBERS_PRESENT,
	IGMP_ROUTER_CHECKING_MEMBERSHIP
};

/**************************************************************************
 * STRUCTURE -  IGMP_ROUTER_CONFIG
 **************************************************************************
 *	This structure contains the various configuration parameters that 
 *	control the IGMP proxy module.
 **************************************************************************/
typedef struct IGMP_ROUTER_CONFIG_T
{
	char	upstream_ifaceName[IFNAMSIZ];
	int		other_querier_present_interval;
	int		query_interval;
	int		query_reponse_interval;
	int		group_membership_interval;
	int		unsolicited_report_interval;
	int		last_member_query_interval;
	int		last_member_query_count;
	int		robustness_variable;
	int		startup_query_interval;
	int		startup_query_count;
	int		v1_host_present_interval;
}IGMP_ROUTER_CONFIG;

/**************************************************************************
 * STRUCTURE -  IGMP_QUERY
 **************************************************************************
 *	This structure is the format of the IGMP query that will be sent out
 *	on a specified interface. This is valid only for version 1 and version 2
 *	queries.
 **************************************************************************/
typedef struct IGMP_QUERY_T
{
	/* Links to the list of queries. */
	LIST_NODE		links;

	/* Type of query. */
	int				type;

	/* Max response time. */
	int				max_resp_time;

	/* Group Address.	*/
	unsigned int	group_address;
}IGMP_QUERY;

/**************************************************************************
 * STRUCTURE -  IGMP_GROUP
 **************************************************************************
 *	This structure contains information about the group membership. Each
 *	group membership is linked with a corresponding network interface.
 **************************************************************************/
typedef struct IGMP_GROUP_T
{
	/* Links to other group memberships. */
	LIST_NODE				links;
	
	/* Configuration control block. */
	IGMP_ROUTER_CONFIG*		ptr_config;

	/* Multicast Group Address -- This is stored in NET format i.e as received
	 * from the network. */
	unsigned int			group_address;

	/* Multicast Group timer. */
	int						group_timer;

	/* Version 1 HOST timer. */
	int						v1_host_timer;

	/* Retransmit Timer.	*/
	int						retransmit_timer;

	/* Router state	*/
	enum IGMP_GROUP_STATE	state;

	/* List of queriers to send for the group on the attached interface. */
	IGMP_QUERY*				query_list;
}IGMP_GROUP;


/**************************************************************************
 * STRUCTURE -  IGMP_NETWORK_IFACE
 **************************************************************************
 *	This structure contains information about a network interface that is 
 *	attached to the IGMP router.
 **************************************************************************/
typedef struct IGMP_NETWORK_IFACE_T
{		
	/* Links to other network interfaces. */
	LIST_NODE				links;

	/* Upstream / Downstream mode. */
	int						mode;

	/* Socket used by the interface. */
	int						socket;

	/* Name of the interface. */
	char 					ifaceName[IFNAMSIZ];

	/* System returned index of the interface. */
	int						iface_index;

	/* IP Address of the interface.	*/
	struct in_addr			ip_address;

	/* Device Flags for the interface. */
	short int				flags;

	/* Index of the virtual interface. */
	unsigned int			vif_index;

	/* Are we the querier on this interface ? */
	int 					bIsQuerier;

	/* General Query Timer */
	int						general_query_timer;

	/* Querier Present timer. */
	int						querier_present_timer;

	/* IGMP version that is to be used on this interface. */
	int						igmp_version;

	/* List of all groups attached on this interface. */
	IGMP_GROUP*				group_database;
}IGMP_NETWORK_IFACE;

/**************************************************************************
 * STRUCTURE -  IGMP_ROUTER_MCB
 **************************************************************************
 *	This is the master control block for the IGMP router.
 **************************************************************************/
typedef struct IGMP_ROUTER_MCB_T
{
	/* List to the network interfaces connected to the router. */
	IGMP_NETWORK_IFACE*		network_if_list;
		
	/* Configuration Control block. */
	IGMP_ROUTER_CONFIG		config;

	/* Cached entry to the network interface that is confgiured as UPSTREAM. */
	IGMP_NETWORK_IFACE*		ptr_upstream_interface;

	/* Raw IGMP socket used for receiving all IGMP traffic. */
	int						igmp_router_socket;

	/* Pre-allocated free list of group memberships. This has been added so
	 * that memory is allocated only once during startup. Addition and removal
	 * of groups causes removal and insertion into this list, reducing 
	 * fragmentation. */
	IGMP_GROUP*				free_group_list;

	/* Pre-allocated free list of queries. This has been added so
	 * that memory is allocated only once during startup, reducing 
	 * fragmentation. */
	IGMP_QUERY*				free_query_list;
}IGMP_ROUTER_MCB;

#endif /* __IGMP_PROXY__ */

