/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/
#ifndef __mvEgigaProc
#define __mvEgigaProc

#define MAX_PORT 	2 
#define MAX_Q		7 
#define MAC_ADDR_LEN	6
#define FILE_NAME	"mv_eth_tool"
#define FILE_PATH	"/proc/net/"
#define STS_FILE	"mvethtool.sts"

#define PROC_STRING	"%2x %2x %2x %2x %2x %2x %2x %2x:%2x:%2x:%2x:%2x:%2x %x"
#define PROC_PRINT_LIST	 command,  port,  q,  direct,  policy,  packett,  status,  mac[0],  mac[1],  mac[2],  mac[3],  mac[4],  mac[5],  weight
#define PROC_SCANF_LIST	&command, &port, &q, &direct, &policy, &packett, &status, &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5], &weight
#define LIST_LEN	14


typedef enum {
	COM_SRQ = 0,
	COM_SQ,
	COM_SRP,
	COM_SRQW,
	COM_STP,
	COM_STS,
	COM_HEAD,} command_t;

typedef enum {
	RX = 0,
	TX,} direction_t;

typedef enum { 	/**/
	WRR = 0,
	FIXED,} policy_t;

typedef enum {
	PT_BPDU = 0,
	PT_ARP,
	PT_TCP,
	PT_UDP,} packett_t;

typedef enum {
	STS_PORT = 0,
	STS_PORT_Q,
	STS_PORT_RXP,
	STS_PORT_TXP,
	STS_PORT_REGS,
	STS_PORT_MIB,
	STS_PORT_STATIS,} status_t;

#endif
	
