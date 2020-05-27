/******************************************************************************
 *
 * Name:        fwfunctions.c
 * Project:     Yukon/Yukon2, PCI Gigabit Ethernet Adapter
 * Purpose:     All functions regarding firmware handling
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	    (C)Copyright 1998-2002 SysKonnect GmbH.
 *	    (C)Copyright 2002-2011 Marvell.
 *
 *	    Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *      Server Adapters.
 *
 *	    Address all question to: support@marvell.com
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 *****************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skversion.h"

#ifdef MV_INCLUDE_SDK_SUPPORT
extern SK_U32 SendFwCommand (SK_AC *pAC, SK_U32 DatagramType, char *pFwDataBuff, SK_U32 FrameLength);
unsigned int inet_addr(char *str);

/******************************************************************************
 *
 *	SetFwIpAddr() - set the Firmware IP address
 *
 * Description:
 *	Set the Firmware IP address.
 *
 * Returns:
 *	nothing
 */
int SetFwIpAddr(
	SK_AC	*pAC,			/* Adapter context */
	SK_U32	InetAddress,	/* IPv4 address */
	SK_U32	InetNetmask,	/* IPv4 netmask */
	SK_U32	InetGateway)	/* IPv4 gateway */
{

	SK_U16					DatagramLength;
	CmdSetIpv4Config		Ipv4ConfigDatagram;
	char					*pMsgBuff = NULL;   /* Message buffer */
	printk("Set IP: 0x%x\n", InetAddress);
	printk("Set NM: 0x%x\n", InetNetmask);

	/* Check tha data */
	if (InetAddress == 0) {
		printk("%s: SetFwIpAddr  No IPv4 address given!\n", DRV_NAME);
		return (-1);
	}
	else if (InetNetmask == 0) {
		printk("%s: SetFwIpAddr  No IPv4 netmask given!\n", DRV_NAME);
		return (-1);
	}
	else if (InetGateway == 0) {
		printk("%s: SetFwIpAddr  No IPv4 gateway given!\n", DRV_NAME);
		return (-1);
	}

	/* Allocate internal message buffer */
	DatagramLength = sizeof(Ipv4ConfigDatagram);
	pMsgBuff = kmalloc(DatagramLength, GFP_KERNEL);

	if (pMsgBuff == NULL) {
		printk("%s: SetFwIpAddr  Memory allocation failed!\n", DRV_NAME);
		return (-1);
	}
	memset(pMsgBuff, 0, DatagramLength);

	/* ENDIANESS */
	Ipv4ConfigDatagram.Hdr.Cmd = FW_CMD_SET_IPV4_CONFIG;
	Ipv4ConfigDatagram.Hdr.Len = FW_CMD_SET_IPV4_CONFIG_LENGTH;
	Ipv4ConfigDatagram.Ip = InetAddress;
	Ipv4ConfigDatagram.Mask = InetNetmask;
	Ipv4ConfigDatagram.Gateway = InetGateway;

	memcpy(pMsgBuff, &Ipv4ConfigDatagram, DatagramLength);


	/* SEND */
	SendFwCommand(pAC, SKGE_DGRAM_MSG2FW, pMsgBuff, FW_CMD_SET_IPV4_CONFIG_LENGTH);

	/* Free internal message buffer */
	if (pMsgBuff != NULL) {
		kfree(pMsgBuff);
	}
	return 0;
}


/* Helper functions */
unsigned int inet_addr(char *str)
{
	int a,b,c,d;
	char arr[4];
	sscanf(str,"%d.%d.%d.%d",&a,&b,&c,&d);
	arr[3] = a; arr[2] = b; arr[1] = c; arr[0] = d;
	return *(unsigned int*)arr;
}
#endif

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
