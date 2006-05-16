/*
 *  Copyright 2005, Broadcom Corporation      
 *  All Rights Reserved.      
 *        
 *  THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 *  KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 *  SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 *  FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 *
 *  $Id: wanppp.h,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#if !defined(_wanppp_h_)
#define _wanppp_h_


#define VAR_ConnectionType		0
#define VAR_PossibleConnectionTypes		1
#define VAR_ConnectionStatus		2
#define VAR_Uptime		3
#define VAR_UpstreamMaxBitRate		4
#define VAR_DownstreamMaxBitRate		5
#define VAR_LastConnectionError		6
#define VAR_IdleDisconnectTime		7
#define VAR_RSIPAvailable		8
#define VAR_NATEnabled		9
#define VAR_UserName		10
#define VAR_Password		11
#define VAR_ExternalIPAddress		12
#define VAR_PortMappingNumberOfEntries		13
#define VAR_PortMappingEnabled		14
#define VAR_PortMappingLeaseDuration		15
#define VAR_RemoteHost		16
#define VAR_ExternalPort		17
#define VAR_InternalPort		18
#define VAR_PortMappingProtocol		19
#define VAR_InternalClient		20
#define VAR_PortMappingDescription		21

int WANPPPConnection_GetVar(struct Service *psvc, int varindex);

#endif /* _wanppp_h_ */
