/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/heartbeat.h - Heartbeat Definitions
***********************************************************
 $Author: jeroen $
 $Id: heartbeat.h,v 1.6 2006-12-21 14:08:50 jeroen Exp $
 $Date: 2006-12-21 14:08:50 $
**********************************************************/

#ifndef HEARTBEAT_H
#define HEARTBEAT_H "H5K7:W3NDY5UU5N1K1N1C0l3"

#include "common.h"
#include "tic.h"

/* 
 * SixXS Heartbeat Protocol
 * port - uses UDP over IPv4
 */
#define HEARTBEAT_PORT	"3740"

SOCKET heartbeat_socket(
	uint32_t *address_changed,
	int bStaticTunnel,
	const char *sIPv4Interface,
	char **sIPv4Local,
	const char *sIPv4POP,
	const char *sIPv4LocalResolve);

int heartbeat_send(SOCKET sockfd, char *sIPv4Local, char *sIPv6Local, char *sPassword, bool bBehindNAT);

void heartbeat_beat(struct TIC_Tunnel *hTunnel);
char *heartbeat_getlocalIP(struct TIC_Tunnel *hTunnel);

#endif /* HEARTBEAT_H */

