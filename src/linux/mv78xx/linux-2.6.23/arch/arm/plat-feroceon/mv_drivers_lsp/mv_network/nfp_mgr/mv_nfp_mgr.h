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

/*******************************************************************************
* mv_nfp_mgr.h - Header File for Marvell NFP Manager
*
* DESCRIPTION:
*       This header file contains macros, typedefs and function declarations 
* 	specific to the Marvell Network Fast Processing Manager.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __mv_nfp_mgr_h__
#define __mv_nfp_mgr_h__

/* includes */
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/netfilter/nf_nat.h>
#endif
#include "eth/nfp/mvNfp.h"

/* defines */

/* enumerations */

typedef enum {
	MV_FP_MANAGER = 0, 
	MV_FP_DATABASE = 1

} MV_FP_OP_TYPE;

/* NFP interface type, used for registration */
typedef enum {
	MV_FP_IF_INV,   /* Invalid interface */
	MV_FP_IF_INT,   /* use to register a Marvell GbE interface */
	MV_FP_IF_BRG,   /* use to register a virtual interface such as bridge */
	MV_FP_IF_EXT    /* use to register an external interface such as WLAN */

} MV_FP_IF_TYPE;

/* data types */
struct map_eth_devs {
	MV_FP_IF_TYPE       if_type;
	struct net_device*  dev;
    u32                 def_gtw_ip;
};

extern struct map_eth_devs *fp_eth_devs;
extern int  fp_disable_flag;

static INLINE int fp_is_enabled(void)
{
	return (!fp_disable_flag);
}

static INLINE MV_FP_IF_TYPE fp_mgr_get_if_type(int if_index)
{
    if(if_index >= ETH_FP_IFINDEX_MAX)
    {
        mvOsPrintf("if_index %d is OUT of RANGE\n", if_index);
        return MV_FP_IF_INV;
    }
    return fp_eth_devs[if_index].if_type;
}

static INLINE struct net_device* fp_mgr_get_net_dev(int if_index)
{
    if(if_index >= ETH_FP_IFINDEX_MAX)
    {
        mvOsPrintf("if_index %d is OUT of RANGE\n", if_index);
        return NULL;
    }
    return fp_eth_devs[if_index].dev;
}

/* function headers: */

/* Initialize NFP Manager */
int fp_mgr_init(void);

/* Register a network interface that works with NFP	  */
/* Parameters: 						  */
/* if_index: Linux network interface index 		  */
/* if_type: interface type (gateway, external)		  */
/* dev: pointer to the Linux net_device			  */
/* Returns port number 					  */
int fp_mgr_if_register(	int if_index, MV_FP_IF_TYPE if_type, 
			            struct net_device* dev);
int fp_mgr_if_unregister(int if_index);

/* This function is called when user-space tool disables the NFP.	*/
/* All databases are cleared, and learning of new rules is disabled	*/
/* for all types of rules: static, dynamic, arp, routing etc.		*/
void fp_mgr_disable(void);

/* This function is called when user-space tool enables the NFP.	*/
int fp_mgr_enable(void);

/* This function is called when user-space tool asks about NFP status (enabled/disabled) */
int fp_mgr_status(void);

/* Initialize NFP Rule Database (Routing + ARP information table) 	*/
int fp_rule_db_init(u32 db_size);

/* Initialize NFP NAT Rule Database (SNAT + DNAT table) 		*/
int fp_nat_db_init(u32 db_size);

/* Initialize NFP Manager ARP Database 					*/
int fp_arp_db_init(u32 db_size);

/* Clear NFP Rule Database (Routing + ARP information table) 		*/
int fp_rule_db_clear(void);

/* Clear NFP NAT Rule Database (SNAT + DNAT table) 			*/
int fp_nat_db_clear(void);

/* Clear NFP Manager ARP Database */
int fp_arp_db_clear(void);

/* Print NFPRule Database (Routing + ARP information table) */
int fp_rule_db_print(MV_FP_OP_TYPE);

/* Print NFP NAT Rule Database (SNAT + DNAT table) */
int fp_nat_db_print(MV_FP_OP_TYPE);

/* Print NFP Manager ARP Database */
int fp_arp_db_print(void);

/* Set a new rule or update an existing one  */
/* When looking for an existing rule, search */
/* a match for SIP, DIP, and default gateway IP */
/* Rule type is also taken into account: */
/* a new static rule can overrode an existing rule of any type, */
/* while a new dynamic rule can only override an existing dynamic rule */
int fp_rule_set(MV_FP_RULE *rule);

/* Delete an existing rule */
/* When looking for an existing rule, search */
/* a match for SIP, DIP */
int fp_rule_delete(u32 src_ip, u32 dst_ip, MV_FP_RULE_TYPE rule_type);

/* Set Routing information received from the IP stack when a new Routing cache entry is created */
/* Look for matching ARP information to complete the rule */
/* If we have a complete rule, update the NFP database */
int fp_routing_info_set(u32 src_ip, u32 dst_ip, u32 def_gtw_ip, int ingress_if, int egress_if);

/* Delete Routing information from the Routing + ARP database, and update the NFP database */
int fp_routing_info_delete(u32 src_ip, u32 dst_ip);

/* Set ARP information in the ARP database, and update the Routing + ARP database if necessary */
/* If we have a complete rule, update the NFP database */
int fp_arp_info_set(int if_index, u32 ip, const u8 *mac);

/* Delete ARP information from the ARP database, and update the Routing + ARP database if necessary */
/* If a rule became incomplete, update the NFP database */
int fp_arp_info_delete(u32 ip);

/* Return ARP rule confirmation status */
int fp_is_arp_confirmed(u32 ip, const u8 *mac);

/* Return routing rule confirmation status */
int fp_is_route_confirmed(u32 src_ip, u32 dst_ip);
int fp_is_enabled(void);

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
/* Set a new NAT rule, or update an existing one */
int fp_nat_info_set(	u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port, u8 proto,  
			u32 new_src_ip, u32 new_dst_ip, u16 new_src_port, u16 new_dst_port, 
			int if_index, enum nf_nat_manip_type maniptype);

int fp_nat_info_delete(u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port, u8 proto);

/* Return NAT rule confirmation status */
int fp_is_nat_confirmed(u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port, u8 proto);
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT
int fp_fdb_db_init(u32 db_size);
int fp_fdb_info_set(u32 ifvlan, u32 ifport, const u8 *mac, int is_local);
int fp_fdb_info_del(u32 ifvlan, u32 ifport, const u8 *mac, int is_local);
int fp_fdb_db_print(MV_FP_OP_TYPE op);
int fp_fdb_db_clear(void);
#endif	/* CONFIG_MV_ETH_NFP_FDB_SUPPORT */
#endif /* __mv_nfp_mgr_h__ */


