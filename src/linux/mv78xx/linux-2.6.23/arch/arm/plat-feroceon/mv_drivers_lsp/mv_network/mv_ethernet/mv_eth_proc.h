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
#ifndef __mv_eth_proc
#define __mv_eth_proc

#define FILE_NAME	"mv_eth_tool"
#define FILE_PATH	"/proc/net/"
#define STS_FILE	"mvethtool.sts"

#define IP_RULE_STRING	    "%2x %2x %2x %4x %4x %2x:%2x:%2x:%2x:%2x:%2x %2x:%2x:%2x:%2x:%2x:%2x"
#define IP_RULE_PRINT_LIST	 command, inport, outport, dip, sip, da[0], da[1], da[2], da[3], da[4], da[5], sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]
#define IP_RULE_SCANF_LIST	&command, &inport, &outport, &dip, &sip, &da[0], &da[1], &da[2], &da[3], &da[4], &da[5], &sa[0], &sa[1], &sa[2], &sa[3], &sa[4], &sa[5]
#define IP_RULE_LIST_LEN	17

#define IP_RULE_DEL_STRING	"%2x %4x %4x"
#define IP_RULE_DEL_PRINT_LIST	command, dip, sip
#define IP_RULE_DEL_SCANF_LIST	&command, &dip, &sip
#define IP_RULE_DEL_LIST_LEN	3

#define FP_EN_DIS_STRING	"%2x"
#define FP_EN_DIS_PRINT_LIST	command

#define FP_DB_PRINT_STRING	"%2x %2x"
#define FP_DB_PRINT_PRINT_LIST	command, db_type
#define FP_DB_PRINT_SCANF_LIST	&command, &db_type
#define FP_DB_PRINT_LIST_LEN	2

#define PROC_STRING	"%2x %2x %2x %2x %2x %2x %2x:%2x:%2x:%2x:%2x:%2x %x"
#define PROC_PRINT_LIST	 command,  port,  q,  policy,  packet,  status,  mac[0],  mac[1],  mac[2],  mac[3],  mac[4],  mac[5],  weight
#define PROC_SCANF_LIST	&command, &port, &q, &policy, &packet, &status, &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5], &weight
#define LIST_LEN	13

#define PORT_CMD_STRING     "%2x %2x %x"
#define PORT_PRINTF_LIST    command, port, value
#define PORT_SCANF_LIST     &command, &port, &value
#define PORT_LIST_LEN       3

#define ETH_CMD_STRING     "%2x %x"
#define ETH_PRINTF_LIST    command, value
#define ETH_SCANF_LIST     &command, &value
#define ETH_LIST_LEN       2

typedef enum {
	COM_SRQ = 0,
	COM_SQ,
	COM_SRP,
	COM_SRQW,
	COM_STP,
	COM_STS,
	COM_HEAD,
	COM_RX_COAL,
	COM_TX_COAL,
	COM_TXDONE_Q,
 	COM_IP_RULE_SET,
	COM_IP_RULE_DEL, 
	COM_FP_DISABLE, 
	COM_FP_ENABLE, 
	COM_FP_PRINT,
	COM_FP_STATUS,
    COM_TX_EN,
    COM_SKB_REUSE,
    COM_EJP_MODE
} command_t;

typedef enum { 	/**/
	WRR = 0,
	FIXED
} policy_t;

typedef enum {
	PT_BPDU = 0,
	PT_ARP,
	PT_TCP,
	PT_UDP
} packet_t;

typedef enum {
	STS_PORT = 0,
	STS_PORT_Q,
	STS_PORT_RXP,
	STS_PORT_TXP,
	STS_PORT_REGS,
	STS_PORT_MIB,
	STS_PORT_STATIS,
    STS_PORT_MAC,
    STS_PORT_NFP_STATS,
    STS_NETDEV,
} status_t;

typedef enum {
	DB_ROUTING = 0, 
	DB_NAT,
	DB_FDB,
} db_type_t;

#endif
	
