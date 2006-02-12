/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wanipc.h,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#ifndef _wanipc_h_
#define _wanipc_h_

/*
  Definitions and declarations to support the WAN IPConnection service.
*/

#define VAR_ConnectionType		0
#define VAR_PossibleConnectionTypes		1
#define VAR_ConnectionStatus		2
#define VAR_Uptime		3
#define VAR_LastConnectionError		4
#define VAR_RSIPAvailable		5
#define VAR_NATEnabled		6
#define VAR_ExternalIPAddress		7
#define VAR_PortMappingNumberOfEntries		8
#define VAR_PortMappingEnabled		9
#define VAR_PortMappingLeaseDuration		10
#define VAR_RemoteHost		11
#define VAR_ExternalPort		12
#define VAR_InternalPort		13
#define VAR_PortMappingProtocol		14
#define VAR_InternalClient		15
#define VAR_PortMappingDescription		16


typedef enum { 
    IP_UNCONFIGURED, 
    IP_CONNECTING, 
    IP_AUTHENTICATING,
    IP_CONNECTED, 
    IP_PENDINGDISCONNECT, 
    IP_DISCONNECTING,
    IP_DISCONNECTED	  
} ip_conn_t;


#define CONNECTION_TIMEOUT   20

struct WANIPConnectionData {
    ip_conn_t	connection_status;
    int		connection_timeout;  /* timeout in seconds */
    time_t    connected_time;
    struct in_addr external_ipaddr;
    int igd_generation;
    int nportmappings;
    timer_t eventhandle;
};

typedef struct WANIPConnectionData WANIPConnectionData, *PWANIPConnectionData;

#endif // _wanipc_h_
