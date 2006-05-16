/*
 *  Copyright 2005, Broadcom Corporation      
 *  All Rights Reserved.      
 *        
 *  THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 *  KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 *  SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 *  FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 *
 *  $Id: lanhostconfig.h,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#if !defined(_lanhostconfig_h_)
#define _lanhostconfig_h_

#define VAR_DHCPServerConfigurable		0
#define VAR_DHCPRelay		1
#define VAR_SubnetMask		2
#define VAR_IPRouters		3
#define VAR_DNSServers		4
#define VAR_DomainName		5
#define VAR_MinAddress		6
#define VAR_MaxAddress		7
#define VAR_ReservedAddresses		8

int LANHostConfig_GetVar(struct Service *psvc, int varindex);
int LANHostConfig_SetDomainName(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs);


#endif /* _lanhostconfig_h_ */
