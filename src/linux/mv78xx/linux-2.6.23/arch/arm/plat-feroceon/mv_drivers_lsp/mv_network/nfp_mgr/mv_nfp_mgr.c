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
* mv_nfp_mgr.c - Marvell Network Fast Processing Manager
*
* DESCRIPTION:
*       
*       Supported Features:
*
*******************************************************************************/

/* includes */
#include "mvCommon.h"  /* Should be included before mvSysHwConfig */
#include "mvTypes.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <net/route.h>

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
#include <net/netfilter/nf_nat.h>
#endif

#include "mvDebug.h"
#include "mvOs.h"
#include "mvSysHwConfig.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "eth/nfp/mvNfp.h"
#include "mv_nfp_mgr.h"

/* defines */
#define FP_MAX_STR_SIZE		256
#define AGING_TIMER_PERIOD	((CONFIG_MV_ETH_NFP_AGING_TIMER)*HZ) 

#define MV_FTP_CTRL_PORT 21

/* debug control */
#define FP_MGR_DEBUG
#undef FP_MGR_DEBUG

#define FP_MGR_DBG_OFF		0x0000
#define FP_MGR_DBG_INIT		0x0001
#define FP_MGR_DBG_CLR		0x0002
#define FP_MGR_DBG_ARP		0x0004
#define FP_MGR_DBG_ROUTE	0x0008
#define FP_MGR_DBG_NAT		0x0010
#define FP_MGR_DBG_ALL		0xffff

#ifdef FP_MGR_DEBUG 
static unsigned int mv_fp_mgr_dbg = FP_MGR_DBG_ALL; 
#define FP_MGR_DBG(FLG, X) if( (mv_fp_mgr_dbg & (FLG)) == (FLG) ) printk X
#else
#define FP_MGR_DBG(FLG, X)
#endif

#define MAX_MSG_SIZE		128 - 4	
/* structure definitions */
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
#define MASQ_TARGET_NAME	"MASQUERADE"
#define SNAT_TARGET_NAME	"SNAT"
#define DNAT_TARGET_NAME	"DNAT"
#define REDIRECT_TARGET_NAME	"REDIRECT"

typedef struct _fp_iptables_nat_rule
{
	struct _fp_iptables_nat_rule *next;

	/* Relevant target names are: */
	/* SNAT, DNAT, MASQUERADE, REDIRECT */
	char target_name[XT_FUNCTION_MAXNAMELEN-1];

	/* Relevant for SNAT/DNAT: */
	__be32 sip, dip, smsk, dmsk;

	char iniface[IFNAMSIZ], outiface[IFNAMSIZ];	

	/* Protocol, 0 = ANY */
	u_int16_t proto;

} FP_IPTABLES_NAT_RULE;

typedef struct _fp_user_nat_table 
{
	FP_IPTABLES_NAT_RULE *rule_chain;
} FP_USER_NAT_TABLE;

struct nat_rule_db {
	MV_FP_NAT_RULE *rule_chain;
	u32 max_size;
};

static struct nat_rule_db   mgr_nat_rule_db;

#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

typedef struct _mv_fp_arp_rule {
	struct _mv_fp_arp_rule *next;

	int     if_index;
	u32     ip;
	u8      mac[MV_MAC_ADDR_SIZE];
	int     confirmed;

} MV_FP_ARP_RULE; 

struct rule_db {
	MV_FP_RULE  *rule_chain;
	u32         max_size;
};

struct arp_rule_db {
	MV_FP_ARP_RULE *rule_chain;
	u32 max_size;
};

/* global variables */
static struct rule_db       mgr_rule_db;
static struct arp_rule_db   mgr_arp_rule_db;
static struct timer_list    aging_timer;
static spinlock_t	        nfp_mgr_lock;

int                         fp_disable_flag;
struct map_eth_devs         *fp_eth_devs = NULL;

#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT

struct fdb_rule_db {
	MV_FP_FDB_RULE *rule_chain;
	u32 max_size;	
};
#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */


#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
/* we have two tables: previous and current */
static FP_USER_NAT_TABLE    fp_user_nat_table[2];	
/* user NAT table: 0 or 1 */
int                         curr_table, old_table;			
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

#ifdef CONFIG_MV_ETH_NFP_DUAL
int mgr_to_fp_chan, fp_to_mgr_chan;
#endif /* CONFIG_MV_ETH_NFP_DUAL */

static void fp_arp_rule_print(const MV_FP_ARP_RULE *rule);
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
static int fp_nat_db_clear_and_update(void);
#endif


static void __exit fp_exit_module(void)
{
	fp_rule_db_clear();
	fp_arp_db_clear();

	if (fp_eth_devs != NULL)
		kfree(fp_eth_devs);

#ifdef CONFIG_MV_ETH_NFP_DUAL
	mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_RULE_DB_DESTROY_OPCODE, NULL, 0);
#else
	mvFpRuleDbDestroy();
#endif /* CONFIG_MV_ETH_NFP_DUAL */

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
    fp_nat_db_clear();

#ifdef CONFIG_MV_ETH_NFP_DUAL
	mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_NAT_DB_DESTROY_OPCODE, NULL, 0);
#else
	mvFpNatDbDestroy();	
#endif /* CONFIG_MV_ETH_NFP_DUAL */
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT
	fp_fdb_db_clear();
    mvFpFdbDestroy();
#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */
}

static int is_valid_index(int if_index)
{
	if( (if_index < 0) || (if_index >= ETH_FP_IFINDEX_MAX) )
	{
		/*printk("if_index %d is OUT of RANGE\n", if_index);*/
		return 0;
	}
    if( (fp_eth_devs == NULL) || 
	    (fp_eth_devs[if_index].if_type == MV_FP_IF_INV) )
		return 0;

	return 1;
}

/* Find and return the first matching rule in the ARP Database */
static MV_FP_ARP_RULE* fp_arp_rule_find(u32 ip)
{
	MV_FP_ARP_RULE  *curr_rule;

	for (curr_rule = mgr_arp_rule_db.rule_chain; curr_rule != NULL; curr_rule = curr_rule->next) {
		if (curr_rule->ip == ip) {
			return curr_rule;
		}
	}
	return NULL;
}

static MV_FP_RULE* fp_rule_find(u32 sip, u32 dip)
{
	MV_FP_RULE      *curr_rule;

	for (curr_rule = mgr_rule_db.rule_chain; curr_rule != NULL; curr_rule = curr_rule->next) {
		if(	(curr_rule->routingInfo.srcIp == sip) && 
			(curr_rule->routingInfo.dstIp == dip) ) {
				return curr_rule;
		}
	}
	return NULL;
}

static int ip_in_same_network(u32 ip1, u32 ip2, u32 mask)
{
	if ((mask != 0) && ((ip1 & mask) == (ip2 & mask)))
		return 1;
	return 0;
}


#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT

static MV_FP_NAT_RULE* fp_nat_rule_find(u32 sip, u32 dip, u16 sport, u16 dport, u8 proto) 
{
	MV_FP_NAT_RULE  *curr_rule;

	for (curr_rule = mgr_nat_rule_db.rule_chain; curr_rule != NULL; curr_rule = curr_rule->next) {
		if (	curr_rule->srcIp == sip && 
			curr_rule->dstIp == dip && 
			curr_rule->srcPort == sport && 
			curr_rule->dstPort == dport && 
			curr_rule->proto == proto) {
				return curr_rule;
		}
	}
	return NULL;
}

/* Update NFP Rule "NAT awareness" according to the given iptables rule */
static void update_awareness(FP_IPTABLES_NAT_RULE *nat_rule, MV_FP_RULE *fp_rule, int add_del_flag)
{
	int                 in_dev_ifindex = -1, out_dev_ifindex = -1;
	struct net_device   *dev;
	int                 inport_avail = 0, outport_avail = 0;	

	if (!strcmp(nat_rule->target_name, MASQ_TARGET_NAME) || !strcmp(nat_rule->target_name, SNAT_TARGET_NAME)) {
		if ((nat_rule->outiface != NULL) && strcmp(nat_rule->outiface, "")) {
			dev = dev_get_by_name(nat_rule->outiface);
			if (dev != NULL) {
				out_dev_ifindex = dev->ifindex;
			dev_put(dev);
			}
		
			if (is_valid_index(out_dev_ifindex)) {
				outport_avail = 1;
			}
		}
	}
	else if (!strcmp(nat_rule->target_name, REDIRECT_TARGET_NAME) || !strcmp(nat_rule->target_name, DNAT_TARGET_NAME)) {
		if ((nat_rule->iniface != NULL) && strcmp(nat_rule->iniface, "")) {
			dev = dev_get_by_name(nat_rule->iniface);
			if (dev != NULL) {
				in_dev_ifindex = dev->ifindex;
			dev_put(dev);
			}
		
			if (is_valid_index(in_dev_ifindex)) {
				inport_avail = 1;
			}
		}
	}

	if (!strcmp(nat_rule->target_name, MASQ_TARGET_NAME) || !strcmp(nat_rule->target_name, SNAT_TARGET_NAME)) {
		if (	(ip_in_same_network(fp_rule->routingInfo.srcIp, nat_rule->sip, nat_rule->smsk)) || 
			(outport_avail && fp_rule->routingInfo.outIfIndex == out_dev_ifindex) ) {
			/* update SNAT awareness */
			if (add_del_flag) {
				FP_MGR_DBG(FP_MGR_DBG_NAT, ("adding snat awareness\n"));
				fp_rule->mgmtInfo.snat_aware_refcnt++;
				fp_rule->routingInfo.aware_flags |= MV_FP_SNAT_CMD_MAP;
			}
			else {
				fp_rule->mgmtInfo.snat_aware_refcnt--;
				if (fp_rule->mgmtInfo.snat_aware_refcnt == 0) {
					FP_MGR_DBG(FP_MGR_DBG_NAT, ("deleting snat awareness\n"));
					fp_rule->routingInfo.aware_flags &= ~MV_FP_SNAT_CMD_MAP;	
				}
			}
		}
		if (outport_avail && fp_rule->routingInfo.inIfIndex == out_dev_ifindex) {
			/* update DNAT awareness, because this is the reverse direction of the SNAT */
			if (add_del_flag) {
				FP_MGR_DBG(FP_MGR_DBG_NAT, ("adding dnat awareness\n"));
				fp_rule->mgmtInfo.dnat_aware_refcnt++;
				fp_rule->routingInfo.aware_flags |= MV_FP_DNAT_CMD_MAP;
			}
			else {
				fp_rule->mgmtInfo.dnat_aware_refcnt--;
				if (fp_rule->mgmtInfo.dnat_aware_refcnt == 0) {
					FP_MGR_DBG(FP_MGR_DBG_NAT, ("deleting dnat awareness\n"));
					fp_rule->routingInfo.aware_flags &= ~MV_FP_DNAT_CMD_MAP;
				}	
			}
		}
	}
	else if (!strcmp(nat_rule->target_name, REDIRECT_TARGET_NAME) || !strcmp(nat_rule->target_name, DNAT_TARGET_NAME)) {
		if (	(ip_in_same_network(fp_rule->routingInfo.dstIp, nat_rule->dip, nat_rule->dmsk)) || 
			(inport_avail && fp_rule->routingInfo.inIfIndex == in_dev_ifindex) ) {
			/* update DNAT awareness */
			if (add_del_flag) {
				FP_MGR_DBG(FP_MGR_DBG_NAT, ("adding dnat awareness\n"));
				fp_rule->mgmtInfo.dnat_aware_refcnt++;
				fp_rule->routingInfo.aware_flags |= MV_FP_DNAT_CMD_MAP;
			}
			else {
				fp_rule->mgmtInfo.dnat_aware_refcnt--;
				if (fp_rule->mgmtInfo.dnat_aware_refcnt == 0) {
					FP_MGR_DBG(FP_MGR_DBG_NAT, ("deleting dnat awareness\n"));
					fp_rule->routingInfo.aware_flags &= ~MV_FP_DNAT_CMD_MAP;
				}
			}
		}
		if (inport_avail && fp_rule->routingInfo.outIfIndex == in_dev_ifindex) {
			/* update SNAT awareness, because this is the reverse direction of the DNAT */
			if (add_del_flag) {
				FP_MGR_DBG(FP_MGR_DBG_NAT, ("adding snat awareness\n"));
				fp_rule->mgmtInfo.snat_aware_refcnt++;
				fp_rule->routingInfo.aware_flags |= MV_FP_SNAT_CMD_MAP;
			}
			else {
				fp_rule->mgmtInfo.snat_aware_refcnt--;
				if (fp_rule->mgmtInfo.snat_aware_refcnt == 0) {
					FP_MGR_DBG(FP_MGR_DBG_NAT, ("deleting snat awareness\n"));
					fp_rule->routingInfo.aware_flags &= ~MV_FP_SNAT_CMD_MAP;
				}
			}
		}
	}
}
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

/* When setting a rule, check if it needs to be aware of any currently existing user NAT rules */
static void fp_new_rule_awareness(MV_FP_RULE *rule)
{
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
	FP_IPTABLES_NAT_RULE *nat_rule;
#endif
	rule->mgmtInfo.snat_aware_refcnt = 0;
	rule->mgmtInfo.dnat_aware_refcnt = 0;
	rule->routingInfo.aware_flags = 0;
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
	nat_rule = fp_user_nat_table[1-curr_table].rule_chain;
	while (nat_rule != NULL) {
		update_awareness(nat_rule, rule, 1);
		nat_rule = nat_rule->next;
	}
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */
}

/* Clear all NFP databases and update them with the contents of */
/* the NFP Manager databases. */
static int fp_db_clear_and_update(void)
{
	int status = 0;
	unsigned long flags;
	MV_FP_RULE *curr_rule;

#ifdef CONFIG_MV_ETH_NFP_DUAL
	status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_RULE_DB_CLEAR_OPCODE, NULL, 0);
#else
	status = mvFpRuleDbClear();
#endif /* CONFIG_MV_ETH_NFP_DUAL */

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_rule_db.rule_chain;
	while (curr_rule != NULL) { 
		if (!is_zero_ether_addr(curr_rule->routingInfo.dstMac)) {
			fp_new_rule_awareness(curr_rule);
#ifdef CONFIG_MV_ETH_NFP_DUAL
			status |= mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
							MV_FP_RULE_SET_OPCODE, curr_rule, sizeof(MV_FP_RULE));
#else
			status |= mvFpRuleSet(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
		}
		curr_rule = curr_rule->next;
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	return status;
}


#ifdef CONFIG_MV_ETH_NFP_DUAL
static void aging_timer_function(unsigned long data)
{
	unsigned long       flags;
	MV_FP_ARP_RULE      *curr_arp_rule;
	MV_FP_RULE          *curr_rule;
	int                 rule_index = 0;
	ARP_RULE_AGING_MSG  arp_rule_aging_msg;
	RULE_AGING_MSG      rule_aging_msg;
	NAT_RULE_AGING_MSG nat_rule_aging_msg;
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
	MV_FP_NAT_RULE  *curr_nat_rule;
#endif
	static int          current_arp_rule_position = 0;
	static int          current_rule_position = 0;
	static int          current_nat_rule_position = 0;


	/* Collect ARP information from DB */
	spin_lock_irqsave(&nfp_mgr_lock, flags);

	/* move pointer to rule to the next group of rules we want to handle */
	for (	curr_arp_rule = mgr_arp_rule_db.rule_chain, rule_index = 0; 
		(curr_arp_rule != NULL) && (rule_index < (current_arp_rule_position*AGING_QUANTUM)); 
		curr_arp_rule = curr_arp_rule->next, rule_index++);

	rule_index = 0;
	memset(&arp_rule_aging_msg, 0, sizeof(ARP_RULE_AGING_MSG));
	while ((curr_arp_rule != NULL) && (rule_index < AGING_QUANTUM)) {
		arp_rule_aging_msg.num_tuples++;
		arp_rule_aging_msg.info[rule_index].ip = curr_arp_rule->ip;
		curr_arp_rule = curr_arp_rule->next;
		rule_index++;
	}
	current_arp_rule_position++;
	if( (curr_arp_rule == NULL) || 
        ((current_arp_rule_position*AGING_QUANTUM) >= mgr_arp_rule_db.max_size))
		current_arp_rule_position = 0; 

	mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
				MV_FP_MAX_ARP_COUNT_GET_OPCODE, &arp_rule_aging_msg, sizeof(arp_rule_aging_msg));

	/* Collect routing information from DB */	

	/* move pointer to rule to the next group of rules we want to handle */
	for (	curr_rule = mgr_rule_db.rule_chain, rule_index = 0; 
		(curr_rule != NULL) && (rule_index < (current_rule_position*AGING_QUANTUM)); 
		curr_rule = curr_rule->next, rule_index++);

	rule_index = 0;
	memset(&rule_aging_msg, 0, sizeof(RULE_AGING_MSG));

	while ((curr_rule != NULL) && (rule_index < AGING_QUANTUM)) {
		rule_aging_msg.num_tuples++;
		rule_aging_msg.info[rule_index].sip = curr_rule->routingInfo.srcIp;
		rule_aging_msg.info[rule_index].dip = curr_rule->routingInfo.dstIp;
		curr_rule = curr_rule->next;
		rule_index++;
	}
	current_rule_position++;
	if ((curr_rule == NULL) || ((current_rule_position*AGING_QUANTUM) >= mgr_rule_db.max_size))
		current_rule_position = 0; 
	mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
				MV_FP_ROUTE_COUNT_GET_OPCODE, &rule_aging_msg, sizeof(rule_aging_msg));

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
	/* Collect NAT information from DB */

	/* move pointer to rule to the next group of rules we want to handle */
	for (	curr_nat_rule = mgr_nat_rule_db.rule_chain, rule_index = 0; 
		(curr_nat_rule != NULL) && (rule_index < (current_nat_rule_position*AGING_QUANTUM)); 
		curr_nat_rule = curr_nat_rule->next, rule_index++);

	rule_index = 0;
	memset(&nat_rule_aging_msg, 0, sizeof(NAT_RULE_AGING_MSG));

	while ((curr_nat_rule != NULL) && (rule_index < AGING_QUANTUM)) {
		nat_rule_aging_msg.num_tuples++;
		nat_rule_aging_msg.info[rule_index].sip = curr_nat_rule->srcIp;
		nat_rule_aging_msg.info[rule_index].dip = curr_nat_rule->dstIp;
		nat_rule_aging_msg.info[rule_index].sport = curr_nat_rule->srcPort;
		nat_rule_aging_msg.info[rule_index].dport = curr_nat_rule->dstPort;
		nat_rule_aging_msg.info[rule_index].proto = curr_nat_rule->proto;
		curr_nat_rule = curr_nat_rule->next;
		rule_index++;
	}

	current_nat_rule_position++;
	if( (curr_nat_rule == NULL) || 
        	( (current_nat_rule_position*AGING_QUANTUM) >= mgr_nat_rule_db.max_size) )
			current_nat_rule_position = 0; 

	mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
				MV_FP_NAT_COUNT_GET_OPCODE, &nat_rule_aging_msg, sizeof(nat_rule_aging_msg));

#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

  	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	aging_timer.expires = jiffies + AGING_TIMER_PERIOD;
    	add_timer(&aging_timer);
}

#else /* not CONFIG_MV_ETH_NFP_DUAL */

static void aging_timer_function(unsigned long data)
{
	unsigned long   flags;
	MV_FP_ARP_RULE  *curr_arp_rule;
	MV_FP_RULE      *curr_rule;
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
	MV_FP_NAT_RULE  *curr_nat_rule;
#endif

    	spin_lock_irqsave(&nfp_mgr_lock, flags);

	/* Collect routing information from DB */	
	curr_rule = mgr_rule_db.rule_chain;
	while (curr_rule != NULL) 
    	{
		curr_rule->mgmtInfo.new_count = mvFpRouteCountGet(curr_rule->routingInfo.srcIp, 
						                                  curr_rule->routingInfo.dstIp);
        	if(curr_rule->mgmtInfo.new_count != curr_rule->mgmtInfo.old_count)
        	{
            		/* Lookup ARP entry to confirm */
            		curr_arp_rule = fp_arp_rule_find(curr_rule->routingInfo.srcIp);
            		if(curr_arp_rule != NULL)
            		{
                		/* ARP Entry Found - confirmed */
                		curr_arp_rule->confirmed = 1;
            		}
            		else
            		{
                		/* ARP Entry Not Found - Confirm default gateway for incoming interface */
                		curr_arp_rule = fp_arp_rule_find(fp_eth_devs[curr_rule->routingInfo.inIfIndex].def_gtw_ip);
                		if(curr_arp_rule != NULL)
                		{
                    			curr_arp_rule->confirmed = 1;
                		}       		
            		}
        	}
		curr_rule = curr_rule->next;
	}

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
	/* Collect NAT information from DB */
	curr_nat_rule = mgr_nat_rule_db.rule_chain;
	while (curr_nat_rule != NULL) 
    	{
		curr_nat_rule->new_count = mvFpNatCountGet(curr_nat_rule->srcIp, 
						                           curr_nat_rule->dstIp, 
						                           curr_nat_rule->srcPort, 
						                           curr_nat_rule->dstPort, 
						                           curr_nat_rule->proto);
		curr_nat_rule = curr_nat_rule->next;
	}
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	aging_timer.expires = jiffies + AGING_TIMER_PERIOD;
   	add_timer(&aging_timer);
}
#endif /* CONFIG_MV_ETH_NFP_DUAL */

/* Return ARP rule confirmation status */
int fp_is_arp_confirmed(u32 ip, const u8 *mac)
{
	unsigned long   flags;
	MV_FP_ARP_RULE  *curr_rule;
    	int             confirmed;

	if (fp_disable_flag == 1)
		return 0;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_arp_rule_db.rule_chain;
	while (curr_rule != NULL) {
		if( (curr_rule->ip == ip) && (memcmp(curr_rule->mac, mac, MV_MAC_ADDR_SIZE) == 0) ) 
        	{
            		confirmed = curr_rule->confirmed;
            		curr_rule->confirmed = 0;
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return confirmed;
		}
		curr_rule = curr_rule->next;
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Return routing rule confirmation status */
int fp_is_route_confirmed(u32 src_ip, u32 dst_ip)
{
	unsigned long   flags;
	MV_FP_RULE      *curr_rule;
    	int             confirmed = 0;

	if (fp_disable_flag == 1)
		return 0;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_rule_db.rule_chain;
	while (curr_rule != NULL) {
		if( (curr_rule->routingInfo.srcIp == src_ip) && 
			(curr_rule->routingInfo.dstIp == dst_ip) ) 
        	{
            		if(curr_rule->mgmtInfo.new_count != curr_rule->mgmtInfo.old_count)
            		{
                		confirmed = 1;
                		curr_rule->mgmtInfo.old_count = curr_rule->mgmtInfo.new_count;
            		}
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return confirmed;
		}
		curr_rule = curr_rule->next;
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

#ifdef CONFIG_MV_ETH_NFP_DUAL
void fp_rcv_info_msg(int from_cpu, u16 opcode, void* p_msg, u8 size)
{
	int                 i = 0;
	ARP_RULE_AGING_MSG  *arp_rule_aging_msg;
	RULE_AGING_MSG      *rule_aging_msg;
	NAT_RULE_AGING_MSG  *nat_rule_aging_msg;
	MV_FP_ARP_RULE      *curr_arp_rule;
	MV_FP_RULE          *curr_rule;
	MV_FP_NAT_RULE      *curr_nat_rule;

	switch(opcode)
	{
		case MV_FP_MAX_ARP_COUNT_REPLY_OPCODE:
			arp_rule_aging_msg = (ARP_RULE_AGING_MSG *)p_msg;
			for (i = 0; i < arp_rule_aging_msg->num_tuples; i++) {
				curr_arp_rule = fp_arp_rule_find(arp_rule_aging_msg->info[i].ip);
				if (curr_arp_rule != NULL)
					fp_update_arp_rule_aging(curr_arp_rule, arp_rule_aging_msg->info[i].count);
			}
			break;
		case MV_FP_ROUTE_COUNT_REPLY_OPCODE:
			rule_aging_msg = (RULE_AGING_MSG *)p_msg;
			for (i = 0; i < rule_aging_msg->num_tuples; i++) {
				curr_rule = fp_rule_find(rule_aging_msg->info[i].sip, 
							rule_aging_msg->info[i].dip);
				if (curr_rule != NULL) 
					fp_update_rule_aging(curr_rule, rule_aging_msg->info[i].count);
			}
			break;
		case MV_FP_NAT_COUNT_REPLY_OPCODE:
			nat_rule_aging_msg = (NAT_RULE_AGING_MSG *)p_msg;
			for (i = 0; i < nat_rule_aging_msg->num_tuples; i++) {
				curr_nat_rule = fp_nat_rule_find(nat_rule_aging_msg->info[i].sip, 
								nat_rule_aging_msg->info[i].dip, 
								nat_rule_aging_msg->info[i].sport, 
								nat_rule_aging_msg->info[i].dport, 
								nat_rule_aging_msg->info[i].proto);
				if (curr_nat_rule != NULL) 
					fp_update_nat_rule_aging(curr_nat_rule, nat_rule_aging_msg->info[i].count);
			}
			break;
		default:
			printk("fp_rcv_info_msg: unknown message type\n"); 
			break;
	}
}
#endif /* CONFIG_MV_ETH_NFP_DUAL */

/* Initialize NFP Manager */
int fp_mgr_init(void)
{
	int i = 0;
#ifdef CONFIG_MV_ETH_NFP_DUAL
	int                 status = 0;
	MV_FP_INIT_STRUCT   fpdb_init_struct;
#endif /* CONFIG_MV_ETH_NFP_DUAL */

	fp_eth_devs = kmalloc((ETH_FP_IFINDEX_MAX * sizeof(struct map_eth_devs)), GFP_ATOMIC);
	if (fp_eth_devs == NULL)
		return -ENOMEM;

	for (i = 0; i < ETH_FP_IFINDEX_MAX; i++) {
		fp_eth_devs[i].if_type = MV_FP_IF_INV;
        	fp_eth_devs[i].dev = NULL;
        	fp_eth_devs[i].def_gtw_ip = 0;
	}
	fp_disable_flag = 0;
  	spin_lock_init(&nfp_mgr_lock);

	memset(&aging_timer, 0, sizeof(struct timer_list));
	init_timer(&aging_timer);
    	aging_timer.function = aging_timer_function;
    	aging_timer.data = -1;
	aging_timer.expires = jiffies + AGING_TIMER_PERIOD;
	add_timer(&aging_timer);

    /* start NFP disabled - it can be enabled by the egigatool */
	fp_mgr_disable(); 

#ifdef CONFIG_MV_ETH_NFP_DUAL
	mgr_to_fp_chan = mvIpcChanCreate(MV_IPC_HOST_ID, MV_IPC_COPROCESSOR_ID,
                                        MV_IPC_RX_CB_MODE, MAX_MSG_SIZE, 64 /*mgr_rule_db.max_size*/);
 
	fp_to_mgr_chan = mvIpcChanCreate(MV_IPC_COPROCESSOR_ID, MV_IPC_HOST_ID, 
                                        MV_IPC_RX_CB_MODE, MAX_MSG_SIZE, 64 /*mgr_rule_db.max_size*/);

	status = mvIpcChanRcvCbSet(fp_to_mgr_chan, fp_rcv_info_msg);
	if ((status != 0) || (mgr_to_fp_chan < 0) || (fp_to_mgr_chan < 0))
        {
		printk("fp_mgr_init: Error setting up IPC channels\n");
		return status;
        }

	/* Connect Channel Dispatch function */
	mvIpcDbConnect(mvIpcChanDoorbellGet(fp_to_mgr_chan), mvIpcChanDispatchIsr, (void*)fp_to_mgr_chan);
	/* Enable Doorbell interrupt */
	MV_IPC_DOORBELL_ENABLE(MV_IPC_HOST_ID, mvIpcChanDoorbellGet(fp_to_mgr_chan));

	memset(&fpdb_init_struct, 0, sizeof(MV_FP_INIT_STRUCT));
	fpdb_init_struct.mgr_to_fp_chan = mgr_to_fp_chan;
	fpdb_init_struct.fp_to_mgr_chan = fp_to_mgr_chan;
	fpdb_init_struct.rule_db_size = mgr_rule_db.max_size;
	status = mvIpcChanMsgSend(	mvIpcTxSharedChanGet(MV_IPC_COPROCESSOR_ID), 
					MV_SERVICE_NFP_ID, MV_FP_RULE_DB_INIT_OPCODE,
					&fpdb_init_struct, sizeof(MV_FP_INIT_STRUCT));
	if (status != 0) 
	{
		printk("fp_mgr_init: Error sending NFP initialization message\n");
		return status;
	}
	return 0;
#else
	/* Note at this stage the Manager DB is already initialized so mgr_rule_db.max_size is set */
	return mvFpRuleDbInit(mgr_rule_db.max_size);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
}

/* Register a network interface that works with NFP */
int fp_mgr_if_register(	int if_index, MV_FP_IF_TYPE if_type, 
			            struct net_device* dev)
{
	/* sanity checks */
	if( (fp_eth_devs == NULL) || 
	    (if_index < 0) || (if_index >= ETH_FP_IFINDEX_MAX) )
		return -1;

	fp_eth_devs[if_index].dev = dev;
	fp_eth_devs[if_index].if_type = if_type;

	return 0;
}

int fp_mgr_if_unregister(int if_index)
{
	if (fp_eth_devs || 
		(if_index < 0) || 
		(if_index >= ETH_FP_IFINDEX_MAX))
		return -1;

	memset(&fp_eth_devs[if_index], 0, sizeof(struct map_eth_devs));
	return 0;
}

/* This function is called when user-space tool disables NFP */
/* All databases are cleared, and learning of new rules is disabled	*/
/* for all types of rules: static, dynamic, arp, routing etc.		*/
void fp_mgr_disable(void)
{
	if (fp_disable_flag == 0) {
		fp_disable_flag = 1;
		del_timer(&aging_timer);
	}
	printk("Network Fast Processing Disabled\n");
}

/* This function is called when user-space tool enables NFP */
int fp_mgr_enable(void)
{
	int status;

	if (fp_disable_flag == 1) {
		fp_disable_flag = 0;
		status = fp_db_clear_and_update();

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
        status |= fp_nat_db_clear_and_update();
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

		aging_timer.expires = jiffies + AGING_TIMER_PERIOD;
		add_timer(&aging_timer);
		printk("Network Fast Processing Enabled\n");
		return status;
	}
	printk("Network Fast Processing Enabled\n");
	return 0;
}

/* This function is called when user-space tool asks about NFP status (enabled/disabled) */
int fp_mgr_status(void)
{
	if (fp_disable_flag == 0)
		printk("Network Fast Processing is currently Enabled\n");
	else
		printk("Network Fast Processing is currently Disabled\n");

	return 0;
}

/* Initialize NFP Rule Database (Routing + ARP information table) */
int fp_rule_db_init(u32 db_size)
{
	FP_MGR_DBG(FP_MGR_DBG_INIT, ("FP_MGR: Initializing Rule Database\n"));
	mgr_rule_db.rule_chain = NULL;
	mgr_rule_db.max_size = db_size;
	/* Initialization of the NFP HAL Database is called from fp_mgr_init */
	return 0;
}
 
/* Initialize NFP Manager ARP Database */
int fp_arp_db_init(u32 db_size)
{
	FP_MGR_DBG(FP_MGR_DBG_INIT, ("FP_MGR: Initializing ARP Rule Database\n"));
	mgr_arp_rule_db.rule_chain = NULL;
	mgr_arp_rule_db.max_size = db_size;
	return 0;
}

/* Clear NFP Rule Database (Routing + ARP information table) */
int fp_rule_db_clear(void)
{
	int status = 0;
	unsigned long flags;
	MV_FP_RULE *curr_rule;
	MV_FP_RULE *tmp_rule;

	FP_MGR_DBG(FP_MGR_DBG_CLR, ("FP_MGR: Clearing Rule Database\n"));
	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_rule_db.rule_chain;
	while (curr_rule != NULL) {
		tmp_rule = curr_rule;
		curr_rule = curr_rule->next;
		kfree(tmp_rule);
	}
	mgr_rule_db.rule_chain = NULL;

#ifdef CONFIG_MV_ETH_NFP_DUAL
	status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_RULE_DB_CLEAR_OPCODE, NULL, 0);
#else
	status = mvFpRuleDbClear();
#endif /* CONFIG_MV_ETH_NFP_DUAL */

	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

/* Clear NFP Manager ARP Database */
int fp_arp_db_clear(void)
{
	unsigned long   flags;
	MV_FP_ARP_RULE  *curr_rule;
	MV_FP_ARP_RULE  *tmp_rule;

	FP_MGR_DBG(FP_MGR_DBG_CLR, ("FP_MGR: Clearing ARP Rule Database\n"));
	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_arp_rule_db.rule_chain;
	while (curr_rule != NULL) {
		tmp_rule = curr_rule;
		curr_rule = curr_rule->next;
		kfree(tmp_rule);
	}
	mgr_arp_rule_db.rule_chain = NULL;
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Print NFP Rule Database (Routing + ARP information table) */
int fp_rule_db_print(MV_FP_OP_TYPE op)
{
	int             status = 0;
	unsigned long   flags;
	MV_FP_RULE      *curr_rule;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	if (op == MV_FP_MANAGER) {
		printk("Printing NFP Manager Rule Database: \n");
		curr_rule = mgr_rule_db.rule_chain;
		while (curr_rule != NULL) { 
			mvFpRulePrint(curr_rule);
			curr_rule = curr_rule->next;
		}
	}
	else {
#ifdef CONFIG_MV_ETH_NFP_DUAL
		status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_RULE_DB_PRINT_OPCODE, NULL, 0);
#else
		status = mvFpRuleDbPrint();
#endif /* CONFIG_MV_ETH_NFP_DUAL */
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

/* Print ARP Rule */
static void fp_arp_rule_print(const MV_FP_ARP_RULE *rule)
{
	printk("ARP Rule: ");
	printk("IP = %u.%u.%u.%u, ", NIPQUAD(rule->ip));
	printk("MAC = %02X:%02X:%02X:%02X:%02X:%02X\n", rule->mac[0], rule->mac[1], rule->mac[2], 
							rule->mac[3], rule->mac[4], rule->mac[5]);
}

/* Print NFP Manager ARP Database */
int fp_arp_db_print(void)
{
	unsigned long   flags;
	MV_FP_ARP_RULE  *curr_rule;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	printk("Printing NFP Manager ARP Rule Database: \n");
	curr_rule = mgr_arp_rule_db.rule_chain;
	while (curr_rule != NULL) {
		fp_arp_rule_print(curr_rule);
		curr_rule = curr_rule->next;
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Copy 6 bytes. Warning - doesn't perform any checks on memory, just copies */
static inline void mac_addr_copy(u8 *dst, const u8 *src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
	dst[4] = src[4];
	dst[5] = src[5];
}

/* Update the Routing + ARP database with new ARP information: */
/* Go over the database and update the destination MAC address */
/* for each entry with a matching default gateway IP address.  */
static int fp_rule_db_arp_update(u32 ip, const u8 *mac)
{
	int             status = 0;
	unsigned long   flags;
	MV_FP_RULE      *curr_rule;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	for (curr_rule = mgr_rule_db.rule_chain; curr_rule != NULL; curr_rule = curr_rule->next) 
    	{
		if(	(curr_rule->routingInfo.defGtwIp == ip) && 
			(curr_rule->mgmtInfo.ruleType != MV_FP_STATIC_RULE)) 
        	{
			mac_addr_copy(curr_rule->routingInfo.dstMac, mac);
			if (!is_zero_ether_addr(mac)) 
            		{
				/* Now we have a full rule - we can update the NFP database */
				fp_new_rule_awareness(curr_rule);
#ifdef CONFIG_MV_ETH_NFP_DUAL
				status |= mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
						MV_FP_RULE_SET_OPCODE, curr_rule, sizeof(MV_FP_RULE));
#else
				status |= mvFpRuleSet(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
			}
			else
            		{
#ifdef CONFIG_MV_ETH_NFP_DUAL
				status |= mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
						MV_FP_RULE_DELETE_OPCODE, curr_rule, sizeof(MV_FP_RULE));
#else
				status |= mvFpRuleDelete(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
            		}
		}
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

/* Set ARP information in the ARP database, and update the Routing + ARP database if necessary */
/* If we have a complete rule, update the NFP database */
int fp_arp_info_set(int if_index, u32 ip, const u8 *mac)
{
	unsigned long   flags;
	MV_FP_ARP_RULE  *curr_rule;
	MV_FP_ARP_RULE  *new_rule;

    	if(!is_valid_index(if_index))
        	return 0;

	FP_MGR_DBG(FP_MGR_DBG_ARP, ("nfp_mgr Set ARP: if=%d, IP=%u.%u.%u.%u, MAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
                                if_index, NIPQUAD(ip), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	for (curr_rule = mgr_arp_rule_db.rule_chain; curr_rule != NULL; curr_rule = curr_rule->next) {
		if (curr_rule->ip == ip) {
			/* We've found a match with an existing entry. Let's update it */
			mac_addr_copy(curr_rule->mac, mac);
			curr_rule->confirmed = 1;
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return fp_rule_db_arp_update(ip, mac);
		}
	}
	/* We haven't found a matching existing entry. Let's add a new one */
	new_rule = kmalloc(sizeof(MV_FP_ARP_RULE), GFP_ATOMIC);
	if (new_rule == NULL) {
		spin_unlock_irqrestore(&nfp_mgr_lock, flags);
		return -ENOMEM;
	}
	new_rule->ip = ip;
	mac_addr_copy(new_rule->mac, mac);
	new_rule->confirmed = 1;
    	new_rule->if_index = if_index;
	new_rule->next = NULL;
	
	if (mgr_arp_rule_db.rule_chain == NULL) {
		/* There is no rule in this table yet */
		mgr_arp_rule_db.rule_chain = new_rule;
	}
	else {
		/* Let's add this rule at the tail of the list */
		curr_rule = mgr_arp_rule_db.rule_chain;
		while (curr_rule->next != NULL)
			curr_rule = curr_rule->next;
		
		curr_rule->next = new_rule;		
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return fp_rule_db_arp_update(ip, mac);
}

/* Delete ARP information from the ARP database, and update the Routing + ARP database if necessary */
/* If a rule became incomplete, update the NFP database */
int fp_arp_info_delete(u32 ip)
{
	unsigned long   flags;
	MV_FP_ARP_RULE  *curr_rule;
	MV_FP_ARP_RULE  *prev_rule;
	u8              mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	spin_lock_irqsave(&nfp_mgr_lock, flags);

	prev_rule = NULL;	
	for (	curr_rule = mgr_arp_rule_db.rule_chain; 
		curr_rule != NULL; 
		prev_rule = curr_rule, curr_rule = curr_rule->next) {
		if (curr_rule->ip == ip) {
			/* Note there should only be one matching entry for this IP address */
			if (prev_rule == NULL)
				mgr_arp_rule_db.rule_chain = curr_rule->next;	
			else
				prev_rule->next = curr_rule->next;
			kfree(curr_rule);
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);

            		FP_MGR_DBG(FP_MGR_DBG_ARP, 
                		("nfp_mgr: ARP delete: IP=%u.%u.%u.%u, MAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
                        	NIPQUAD(ip), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

			return fp_rule_db_arp_update(ip, mac);
		}
	}
	/* We haven't found a matching entry, so nothing to do */
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Set a new rule or update an existing one  */
/* When looking for an existing rule, search */
/* a match for SIP, DIP, and default gateway IP */
/* Rule type is also taken into account: */
/* a new static rule can overrode an existing rule of any type, */
/* while a new dynamic rule can only override an existing dynamic rule */
int fp_rule_set(MV_FP_RULE *rule)
{
	unsigned long   flags;
	MV_FP_RULE      *curr_rule;
	MV_FP_RULE      *new_rule;
	int             status = 0;

	/* Static rules are supplied with GbE port numbers, while */
	/* dynamic rules are supplied with Linux interface indices. */
	if (rule->mgmtInfo.ruleType == MV_FP_DYNAMIC_RULE) {
		struct net_device   *in_dev, *out_dev;

		/* We are not interested in this rule if the input interface or */
		/* the output interface is not one of our GbE interfaces.	*/
		if( !(is_valid_index(rule->routingInfo.inIfIndex)) || 
            	    !(is_valid_index(rule->routingInfo.outIfIndex)) )
			return 0;

		in_dev = dev_get_by_index(rule->routingInfo.inIfIndex);
		out_dev = dev_get_by_index(rule->routingInfo.outIfIndex);
		if ( (in_dev != NULL) && (out_dev != NULL) )
		{
			/* We are not interested in this rule if the MTU of the */
			/* output interface is different from the MTU of the input interface */
			/* We also don't support NFP rules if the MTU of the outgoing interface is too big, */
			/* becasue HW doesn't calculate checksum. */
			if (	(in_dev->mtu != out_dev->mtu) || 
				(out_dev->mtu > ETH_CSUM_MAX_BYTE_COUNT) )
			{
				dev_put(in_dev);
				dev_put(out_dev);
				return 0;
			}
		}
		if (in_dev != NULL)
			dev_put(in_dev);
		if (out_dev != NULL)
		dev_put(out_dev);
	}

	spin_lock_irqsave(&nfp_mgr_lock, flags);
    	curr_rule = fp_rule_find(rule->routingInfo.srcIp, rule->routingInfo.dstIp);
    	if(curr_rule != NULL)
    	{
        	if( (curr_rule->mgmtInfo.ruleType == MV_FP_DYNAMIC_RULE) || 
	        	(rule->mgmtInfo.ruleType == MV_FP_STATIC_RULE) ) 
        	{
	        	/* Update existing rule, but only if the current rule is a dynamic rule, */
	        	/* or the new rule is a static rule which overrides the current rule. */		
	        	mvFpRuleCopy(curr_rule, rule);

	        	if (!is_zero_ether_addr(curr_rule->routingInfo.dstMac)) {
		        	/* Now we have a full rule - we can update the NFP database */
		        	fp_new_rule_awareness(curr_rule);
#ifdef CONFIG_MV_ETH_NFP_DUAL
			    	status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
			                        MV_FP_RULE_SET_OPCODE, curr_rule, sizeof(MV_FP_RULE));
#else
			    	status = mvFpRuleSet(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
		    	}
	    	}
        	/* else, we have found a matching entry, but the current rule is static and */
	    	/* the new rule is dynamic, so static rule remains unchanged, and we can exit here. */
	    	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	    	return status;
    	}
	/* We haven't found a matching existing entry. Let's add a new one */
	new_rule = kmalloc(sizeof(MV_FP_RULE), GFP_ATOMIC);
	if (new_rule == NULL) {
		spin_unlock_irqrestore(&nfp_mgr_lock, flags);
		return -ENOMEM;
	}
	mvFpRuleCopy(new_rule, rule);	
	new_rule->next = NULL;

	if (mgr_rule_db.rule_chain == NULL) {
		/* There is no rule in this table yet */
		mgr_rule_db.rule_chain = new_rule;
	}
	else {
		/* Let's add this rule at the tail of the list */
		curr_rule = mgr_rule_db.rule_chain;		
		while (curr_rule->next != NULL)
			curr_rule = curr_rule->next;
		curr_rule->next = new_rule;		
	}
	if (!is_zero_ether_addr(new_rule->routingInfo.dstMac)) {
		/* Now we have a full rule - we can update the the NFP database */
		fp_new_rule_awareness(new_rule);
#ifdef CONFIG_MV_ETH_NFP_DUAL
		status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
				MV_FP_RULE_SET_OPCODE, new_rule, sizeof(MV_FP_RULE));
#else
		status = mvFpRuleSet(new_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

/* Delete an existing rule */
/* When looking for an existing rule, search */
/* a match for SIP, DIP */
int fp_rule_delete(u32 src_ip, u32 dst_ip, MV_FP_RULE_TYPE rule_type)
{
	unsigned long   flags;
	MV_FP_RULE      *curr_rule;
	MV_FP_RULE      *prev_rule;
	int             status = 0;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	prev_rule = NULL;	

	for(curr_rule = mgr_rule_db.rule_chain; 
		curr_rule != NULL; 
		prev_rule = curr_rule, curr_rule = curr_rule->next) {
		if( (curr_rule->routingInfo.srcIp == src_ip) && 
			(curr_rule->routingInfo.dstIp == dst_ip) && 
			(curr_rule->mgmtInfo.ruleType == rule_type) ) {
			/* Note there should only be one matching entry, if any */
			if (prev_rule == NULL) {
				mgr_rule_db.rule_chain = curr_rule->next;
			}
			else {
				prev_rule->next = curr_rule->next;
			}
#ifdef CONFIG_MV_ETH_NFP_DUAL
			status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
						MV_FP_RULE_DELETE_OPCODE, curr_rule, sizeof(MV_FP_RULE));
#else
			status = mvFpRuleDelete(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
			kfree(curr_rule);
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return status;
		}
	}
	/* We haven't found a matching entry, so nothing to do */
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Set Routing information received from the IP stack when a new Routing cache entry is created */
/* Look for matching ARP information to complete the rule */
/* If we have a complete rule, update the NFP database */
int fp_routing_info_set(u32 src_ip, u32 dst_ip, u32 def_gtw_ip, int ingress_if, int egress_if)
{
	MV_FP_RULE          new_rule;
	MV_FP_ARP_RULE      *arp_rule;
	struct net_device   *dev;
	u8                  invalid_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	FP_MGR_DBG(FP_MGR_DBG_ROUTE, ("FP_MGR: Setting Routing Information\n"));
	FP_MGR_DBG(FP_MGR_DBG_ROUTE, 
			("SIP = %u.%u.%u.%u, DIP = %u.%u.%u.%u, GTW = %u.%u.%u.%u, In IF = %d, Out IF = %d\n", 
			NIPQUAD(src_ip), NIPQUAD(dst_ip), 
			NIPQUAD(def_gtw_ip), ingress_if, egress_if));

	new_rule.mgmtInfo.actionType = MV_FP_ROUTE_CMD;
	new_rule.mgmtInfo.new_count = 0;
	new_rule.mgmtInfo.old_count = 0;
	new_rule.mgmtInfo.ruleType = MV_FP_DYNAMIC_RULE; 

	new_rule.routingInfo.srcIp = src_ip;
	new_rule.routingInfo.dstIp = dst_ip;
	new_rule.routingInfo.defGtwIp = def_gtw_ip;
	
	dev = dev_get_by_index(egress_if);
	if (dev != NULL) { 
		mac_addr_copy(new_rule.routingInfo.srcMac, dev->dev_addr);
		dev_put(dev);
	}
	else /* Should never happen */
		mac_addr_copy(new_rule.routingInfo.srcMac, invalid_mac);

    	fp_eth_devs[egress_if].def_gtw_ip = def_gtw_ip;

	/* Note: searching destination MAC according to default gateway IP */
	if ((arp_rule = fp_arp_rule_find(def_gtw_ip)) != NULL) 
		mac_addr_copy(new_rule.routingInfo.dstMac, arp_rule->mac);
	else 
		mac_addr_copy(new_rule.routingInfo.dstMac, invalid_mac);

	new_rule.routingInfo.inIfIndex = ingress_if;
	new_rule.routingInfo.outIfIndex = egress_if;
	new_rule.next = NULL;
	return fp_rule_set(&new_rule);
}

/* Delete Routing information from the Routing + ARP database, and update the NFP database */
int fp_routing_info_delete(u32 src_ip, u32 dst_ip)
{
	FP_MGR_DBG(FP_MGR_DBG_ROUTE, ("FP_MGR: Deleting Routing Information\n"));
	FP_MGR_DBG(FP_MGR_DBG_ROUTE, (	"SIP = %u.%u.%u.%u, DIP = %u.%u.%u.%u\n", 
					NIPQUAD(src_ip), NIPQUAD(dst_ip)));
	return fp_rule_delete(src_ip, dst_ip, MV_FP_DYNAMIC_RULE);
}

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT

/* Initialize NFP NAT Rule Database (SNAT + DNAT table) */
int fp_nat_db_init(u32 db_size)
{
	FP_MGR_DBG(FP_MGR_DBG_INIT, ("FP_MGR: Initializing NAT Rule Database\n"));
	mgr_nat_rule_db.rule_chain = NULL;
	mgr_nat_rule_db.max_size = db_size;

#ifdef CONFIG_MV_ETH_NFP_DUAL
	return mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_NAT_DB_INIT_OPCODE, 
				&(mgr_nat_rule_db.max_size), sizeof(mgr_nat_rule_db.max_size));
#else
	return mvFpNatDbInit(mgr_nat_rule_db.max_size);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
}

/* Clear NFP NAT Rule Database (SNAT + DNAT table) */
int fp_nat_db_clear(void)
{
	int             status = 0;
	unsigned long   flags;
	MV_FP_NAT_RULE  *curr_rule;
	MV_FP_NAT_RULE  *tmp_rule;

	FP_MGR_DBG(FP_MGR_DBG_CLR, ("FP_MGR: Clearing NAT Rule Database\n"));
	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_nat_rule_db.rule_chain;
	while (curr_rule != NULL) {
		tmp_rule = curr_rule;
		curr_rule = curr_rule->next;
		kfree(tmp_rule);
	}
	mgr_nat_rule_db.rule_chain = NULL;
#ifdef CONFIG_MV_ETH_NFP_DUAL
	status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_NAT_DB_CLEAR_OPCODE, NULL, 0);
#else
	status = mvFpNatDbClear();
#endif /* CONFIG_MV_ETH_NFP_DUAL */
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

static int fp_nat_db_clear_and_update(void)
{
    int             status = 0;
	unsigned long   flags;
    MV_FP_NAT_RULE  *curr_nat_rule = mgr_nat_rule_db.rule_chain;

	spin_lock_irqsave(&nfp_mgr_lock, flags);

#ifdef CONFIG_MV_ETH_NFP_DUAL
	status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_NAT_DB_CLEAR_OPCODE, NULL, 0);
#else
	status = mvFpNatDbClear();
#endif /* CONFIG_MV_ETH_NFP_DUAL */

	while (curr_nat_rule != NULL) { 
#ifdef CONFIG_MV_ETH_NFP_DUAL
		status |= mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
					MV_FP_NAT_RULE_SET_OPCODE, curr_nat_rule, sizeof(MV_FP_NAT_RULE));
#else
		status |= mvFpNatRuleSet(curr_nat_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
		curr_nat_rule = curr_nat_rule->next;
    }
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
    return status;
}

/* Print NFP NAT Rule Database (SNAT + DNAT table) */
int fp_nat_db_print(MV_FP_OP_TYPE op)
{
	int             status = 0;
	unsigned long   flags;
	MV_FP_NAT_RULE  *curr_rule;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	if (op == MV_FP_MANAGER) {
		printk("Printing NFP Manager NAT Rule Database: \n");
		curr_rule = mgr_nat_rule_db.rule_chain;
		while (curr_rule != NULL) {
			mvFpNatRulePrint(curr_rule);
			curr_rule = curr_rule->next;
		}
	}
	else {
#ifdef CONFIG_MV_ETH_NFP_DUAL
		status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, MV_FP_NAT_DB_PRINT_OPCODE, NULL, 0);
#else
		status = mvFpNatDbPrint();
#endif /* CONFIG_MV_ETH_NFP_DUAL */
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

static int is_relevant_nat_protocol(u8 proto)
{
	/* For now we only knw how to handle NAT rules for TCP/UDP/ICMP/Zero_Hop protocols */
	/* so we disregard everything else */
	if (	(proto == MV_IP_PROTO_TCP) || 
		(proto == MV_IP_PROTO_UDP) || 
		(proto == MV_IP_PROTO_ICMP) || 
		(proto == MV_IP_PROTO_ZERO_HOP))
		return 1;

	return 0;
}

static int is_relevant_port(u16 src_port, u16 dst_port)
{
	/* We want to pass FTP control stream packets to Linux */
	if (	(MV_16BIT_BE(src_port) != MV_FTP_CTRL_PORT) 
		&& (MV_16BIT_BE(dst_port) != MV_FTP_CTRL_PORT))
		return 1;
	return 0;
}

/* Set a new NAT rule, or update an existing one */
int fp_nat_info_set(u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port, u8 proto, 
			        u32 new_src_ip, u32 new_dst_ip, u16 new_src_port, u16 new_dst_port, 
			        int if_index, enum nf_nat_manip_type maniptype)
{
	unsigned long   flags;
	MV_FP_NAT_RULE  *curr_rule;
	MV_FP_NAT_RULE  *new_rule;
	int             status = 0;

	/* we are not interested in NAT rules that do not involve our interfaces */
	if (!is_valid_index(if_index))
		return 0;

	if (!is_relevant_nat_protocol(proto))
		return 0;

	if (!is_relevant_port(src_port, dst_port))
		return 0;

	if (	(src_ip == new_src_ip) && (dst_ip == new_dst_ip) && 
		(src_port == new_src_port) && (dst_port == new_dst_port) ) 
    	{
   		return 0;
	}

   	FP_MGR_DBG(FP_MGR_DBG_NAT, ("FP_MGR: Setting NAT Information\n"));
	FP_MGR_DBG(FP_MGR_DBG_NAT,
	        ("SIP=%u.%u.%u.%u, DIP=%u.%u.%u.%u, Proto=%u, SPort=%u, DPort=%u, if_index=%d\n",
			        NIPQUAD(src_ip), NIPQUAD(dst_ip), proto, MV_16BIT_BE(src_port), 
			        MV_16BIT_BE(dst_port), if_index));

    	FP_MGR_DBG(FP_MGR_DBG_NAT,
        	("\tNewSIP=%u.%u.%u.%u, NewDIP=%u.%u.%u.%u, NewSPort=%u, NewDPort=%u\n\n", 
            	NIPQUAD(new_src_ip), NIPQUAD(new_dst_ip), 
		MV_16BIT_BE(new_src_port), MV_16BIT_BE(new_dst_port)));

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	for (curr_rule = mgr_nat_rule_db.rule_chain; curr_rule != NULL; curr_rule = curr_rule->next) {
		if( curr_rule->srcIp == src_ip && 
			curr_rule->dstIp == dst_ip && 
			curr_rule->srcPort == src_port && 
			curr_rule->dstPort == dst_port && 
			curr_rule->proto == proto) 
        {

			/* Updating existing rule */
			curr_rule->new_count = 0;
            		curr_rule->old_count = 0;

			if (maniptype == IP_NAT_MANIP_DST) {
				curr_rule->newIp = new_dst_ip;
				curr_rule->newPort = new_dst_port;
			}
			else {
				curr_rule->newIp = new_src_ip;
				curr_rule->newPort = new_src_port;
			}
			if (maniptype == IP_NAT_MANIP_DST) {
				if ((dst_port != 0) && (dst_port != new_dst_port))
					curr_rule->flags |= MV_FP_DNAT_CMD_MAP;
				else
					curr_rule->flags |= MV_FP_DIP_CMD_MAP; 
			}
			else {
				if ((src_port != 0) && (src_port != new_src_port))
					curr_rule->flags |= MV_FP_SNAT_CMD_MAP;
				else
					curr_rule->flags |= MV_FP_SIP_CMD_MAP;
			}	
			/* Now we have a full rule - we can update the NFP database       */
#ifdef CONFIG_MV_ETH_NFP_DUAL
			status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
				            MV_FP_NAT_RULE_SET_OPCODE, curr_rule, sizeof(MV_FP_NAT_RULE));
#else
			status = mvFpNatRuleSet(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return status;
		}
	}
	/* We haven't found a matching existing entry. Let's add a new one */
	new_rule = kmalloc(sizeof(MV_FP_NAT_RULE), GFP_ATOMIC);
	if (new_rule == NULL) {
		spin_unlock_irqrestore(&nfp_mgr_lock, flags);
		return -ENOMEM;
	}
	new_rule->srcIp = src_ip;
	new_rule->dstIp = dst_ip;
	new_rule->srcPort = src_port;
	new_rule->dstPort = dst_port;
	new_rule->proto = proto;
	new_rule->old_count = 0;
    	new_rule->new_count = 0;
	if (maniptype == IP_NAT_MANIP_DST) {
		new_rule->newIp = new_dst_ip;
		new_rule->newPort = new_dst_port;
	}
	else {
		new_rule->newIp = new_src_ip;
		new_rule->newPort = new_src_port;
	}
	if (maniptype == IP_NAT_MANIP_DST) {
		if ((dst_port != 0) && (dst_port != new_dst_port))
			new_rule->flags = MV_FP_DNAT_CMD_MAP;
		else
			new_rule->flags = MV_FP_DIP_CMD_MAP;
	}
	else {
		if ((src_port != 0) && (src_port != new_src_port))
			new_rule->flags = MV_FP_SNAT_CMD_MAP;
		else
			new_rule->flags = MV_FP_SIP_CMD_MAP;
	}
	new_rule->next = NULL;

	if (mgr_nat_rule_db.rule_chain == NULL) {
		/* There is no rule in this table yet */
		mgr_nat_rule_db.rule_chain = new_rule;
	}
	else {
		/* Let's add this rule at the tail of the list */
		curr_rule = mgr_nat_rule_db.rule_chain;		
		while (curr_rule->next != NULL)
			curr_rule = curr_rule->next;
		curr_rule->next = new_rule;		
	}
#ifdef CONFIG_MV_ETH_NFP_DUAL
	status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
				MV_FP_NAT_RULE_SET_OPCODE, new_rule, sizeof(MV_FP_NAT_RULE));
#else
	status = mvFpNatRuleSet(new_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return status;
}

/* Delete a NAT rule */
int fp_nat_info_delete(u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port, u8 proto)
{
	unsigned long   flags;
	MV_FP_NAT_RULE  *curr_rule;
	MV_FP_NAT_RULE  *prev_rule;
	int             status = 0;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	prev_rule = NULL;
	for (	curr_rule = mgr_nat_rule_db.rule_chain; 
		curr_rule != NULL; 
		prev_rule = curr_rule, curr_rule = curr_rule->next) {
		if (	curr_rule->srcIp == src_ip && 
			curr_rule->dstIp == dst_ip && 
			curr_rule->srcPort == src_port && 
			curr_rule->dstPort == dst_port && 
			curr_rule->proto == proto) {

			if (prev_rule == NULL)
				mgr_nat_rule_db.rule_chain = curr_rule->next;	
			else
				prev_rule->next = curr_rule->next;
#ifdef CONFIG_MV_ETH_NFP_DUAL
			status = mvIpcChanMsgSend(mgr_to_fp_chan, MV_SERVICE_NFP_ID, 
					MV_FP_NAT_RULE_DELETE_OPCODE, curr_rule, sizeof(MV_FP_NAT_RULE));
#else
			status = mvFpNatRuleDelete(curr_rule);
#endif /* CONFIG_MV_ETH_NFP_DUAL */
			kfree(curr_rule);
			spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return status;
		}
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Return NAT rule confirmation status */
int fp_is_nat_confirmed(u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port, u8 proto)
{
	unsigned long   flags;
	MV_FP_NAT_RULE  *curr_rule;
    	int             confirmed = 0;

	if (fp_disable_flag == 1)
    	{
		return 0;
    	}
	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_nat_rule_db.rule_chain;
	while (curr_rule != NULL) {
		if( (curr_rule->srcIp == src_ip) && 
			(curr_rule->dstIp == dst_ip) && 
			(curr_rule->srcPort == src_port) && 
			(curr_rule->dstPort == dst_port) && 
			(curr_rule->proto == proto)) 
        	{
            		if(curr_rule->new_count != curr_rule->old_count)
            		{
                		curr_rule->old_count = curr_rule->new_count;
                		confirmed = 1;
            		}
		    	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
			return confirmed;
		}
		curr_rule = curr_rule->next;
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
	return 0;
}

/* Set or clear the "NAT Aware" flags according to user's newly added or deleted iptables rule */
static void fp_handle_user_nat_rule(FP_IPTABLES_NAT_RULE *nat_rule, int add_del_flag)
{
	/* This NAT rule that the user added can affect routing entries based on */
	/* input interface, output interface, SIP, DIP */
	/* For now, protocol and source/dest port are not taken into consideration */

	MV_FP_RULE      *curr_rule;
	unsigned long   flags;
	
	spin_lock_irqsave(&nfp_mgr_lock, flags);
	curr_rule = mgr_rule_db.rule_chain; 
	while (curr_rule != NULL) {
		update_awareness(nat_rule, curr_rule, add_del_flag);		
		mvFpRuleAwareSet(curr_rule);	
		curr_rule = curr_rule->next;
	}
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);
}

/* Initialize our user NAT tables, mirroring iptables NAT rules table */
void init_fp_user_nat_tables(void)
{
	fp_user_nat_table[0].rule_chain = NULL;
	fp_user_nat_table[1].rule_chain = NULL;
	curr_table = 0;
	old_table = 1 - curr_table;
}

/* Add a user NAT rule to our mirror table */
int add_fp_user_nat_rule(struct ipt_entry *e, int table)
{
	struct ipt_entry_target *t = NULL;
	FP_IPTABLES_NAT_RULE *curr_rule = NULL, *new_rule = NULL;

	t = ipt_get_target(e);
	/* Disregard targets that do not interest us */
	if (	strcmp(t->u.kernel.target->name, MASQ_TARGET_NAME) && 
		strcmp(t->u.kernel.target->name, REDIRECT_TARGET_NAME) && 
		strcmp(t->u.kernel.target->name, SNAT_TARGET_NAME) && 
		strcmp(t->u.kernel.target->name, DNAT_TARGET_NAME))
		return 0;

	new_rule = kmalloc(sizeof(FP_IPTABLES_NAT_RULE), GFP_ATOMIC);
	if (new_rule == NULL)
		return -ENOMEM;
	memset(new_rule, 0, sizeof(FP_IPTABLES_NAT_RULE));

	strcpy(new_rule->target_name, t->u.kernel.target->name);
	strcpy(new_rule->iniface, e->ip.iniface);
	strcpy(new_rule->outiface, e->ip.outiface);
	new_rule->sip = e->ip.src.s_addr; 
	new_rule->dip = e->ip.dst.s_addr;
	new_rule->smsk = e->ip.smsk.s_addr;
	new_rule->dmsk = e->ip.dmsk.s_addr;
	new_rule->proto = e->ip.proto;
	new_rule->next = NULL;

	if (fp_user_nat_table[table].rule_chain == NULL) {
		/* There is no rule in this table yet */
		fp_user_nat_table[table].rule_chain = new_rule;
	}
	else {
		/* Let's add this rule at the tail of the list */
		curr_rule = fp_user_nat_table[table].rule_chain;		
		while (curr_rule->next != NULL)
			curr_rule = curr_rule->next;
		curr_rule->next = new_rule;		
	}
	return 0;
}

/* Clear our user NAT rule table */
void clear_fp_user_nat_table(int table)
{
	FP_IPTABLES_NAT_RULE *curr_rule;
	FP_IPTABLES_NAT_RULE *tmp_rule;

	curr_rule = fp_user_nat_table[table].rule_chain;
	while (curr_rule != NULL) {
		tmp_rule = curr_rule;
		curr_rule = curr_rule->next;
		kfree(tmp_rule);
	}
	fp_user_nat_table[table].rule_chain = NULL;
}

/* Check if two user NAT rules are equal */
static int rules_are_equal(FP_IPTABLES_NAT_RULE *r1, FP_IPTABLES_NAT_RULE *r2)
{
	if (strcmp(r1->target_name, r2->target_name))
		return 0;

	if (	(r1->sip != r2->sip) || (r1->dip != r2->dip) || 
		(r1->smsk != r2->smsk) || (r2->dmsk != r2->dmsk) || 
		(r1->proto != r2->proto))
		return 0;

	if (strcmp(r1->iniface, r2->iniface) || strcmp(r1->outiface, r2->outiface))
		return 0;

	return 1;
}

/* Check if a rule exists in our user NAT table */
static int rule_exists_in_table(FP_IPTABLES_NAT_RULE *rule, int table)
{
	FP_IPTABLES_NAT_RULE *curr_rule;

	curr_rule = fp_user_nat_table[table].rule_chain;
	while (curr_rule != NULL) {
		if (rules_are_equal(rule, curr_rule))
			return 1;
		curr_rule = curr_rule->next;
	}
	return 0;
}

/* Compare the two mirror tables to discover which new rules were added, and which rules were deleted */
void compare_fp_user_nat_tables(void)
{
	FP_IPTABLES_NAT_RULE *curr_rule;
	/* Stage 1: */
	/* Compare current table to previous one to detect newly added rules */
	/* For each rule in curr_table: */
	/* If it exists also in old_table, we don't care */
	/* If not, it is new */
	curr_rule = fp_user_nat_table[curr_table].rule_chain;
	while (curr_rule != NULL) {
		if (!rule_exists_in_table(curr_rule, old_table)) {
			fp_handle_user_nat_rule(curr_rule, 1); /* pass 1 for adding a new rule */
		}
		curr_rule = curr_rule->next;
	}
	
	/* Stage 2: */
	/* Compare previous table to current one to detect newly deleted rules */
	/* For each rule in old_table: */
	/* If it exists also in curr_table, we don't care */
	/* If not, it was deleted */
	curr_rule = fp_user_nat_table[old_table].rule_chain;
	while (curr_rule != NULL) {
		if (!rule_exists_in_table(curr_rule, curr_table)) {
			fp_handle_user_nat_rule(curr_rule, 0); /* pass 0 for deleting a rule */
		}
		curr_rule = curr_rule->next;
	}
}

#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

module_exit(fp_exit_module);

#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT

/* Initialize Fast Bridge Rule Database */
int fp_fdb_db_init(u32 db_size)
{
        FP_MGR_DBG(FP_MGR_DBG_INIT, ("FP_MGR: Initializing Bridge Rule Database\n"));

		mgr_fdb_rule_db.rule_chain = NULL;
		mgr_fdb_rule_db.max_size = db_size;

		return mvFpFdbInit(db_size);
}

/* Set Bridging information in the FDB database */
int fp_fdb_info_set(u32 if_bridge, u32 if_index, const u8 *mac, int is_local)
{
	unsigned long flags;
	MV_FP_FDB_RULE rule;

	if (is_local && fp_mgr_if_register(if_bridge, 
									   MV_FP_IF_BRG, 
									   __dev_get_by_index(if_bridge))) {
		FP_MGR_DBG(FP_MGR_DBG_FDB, ("FDB_MNG: failed to register bridge=%d\n", if_bridge));
	}

	memset(&rule, 0, sizeof(MV_FP_FDB_RULE));
	rule.mgmtInfo.actionType = MV_FP_BRIDGE_CMD;
	rule.mgmtInfo.ruleType = MV_FP_DYNAMIC_RULE; 

	mac_addr_copy(rule.fdbInfo.mac, mac);
	rule.fdbInfo.ifIndex = if_index;
	rule.fdbInfo.bridge = if_bridge;
	rule.fdbInfo.flags = (unsigned short)is_local;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	mvFpFdbRuleSet(&rule);
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	return 0;
}

/* Delete bridging information in the FDB database */
int fp_fdb_info_del(u32 if_bridge, u32 if_index, const u8 *mac, int is_local)
{
	unsigned long flags;
	MV_FP_FDB_RULE rule;

	if (is_local && !fp_mgr_if_unregister(if_bridge)) 
		FP_MGR_DBG(FP_MGR_DBG_FDB, ("FDB_MNG: unregister bridge=%d\n", if_bridge));
    
	memset(&rule, 0, sizeof(MV_FP_FDB_RULE));

	mac_addr_copy(rule.fdbInfo.mac, mac);
	rule.fdbInfo.ifIndex = if_index;
	rule.fdbInfo.bridge = if_bridge;
	rule.fdbInfo.flags = (unsigned short)is_local;

	/* has expired */
	spin_lock_irqsave(&nfp_mgr_lock, flags);
	if (!is_local && (mvFpFdbRuleAge(&rule) > 0)) {
		spin_unlock_irqrestore(&nfp_mgr_lock, flags);
		return 1; 
	}

	mvFpFdbRuleDel(&rule);
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	return 0;
}

/* Clear Fast Path FDB Rule Database */
int fp_fdb_db_clear(void)
{
	unsigned long flags;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	mvFpFdbClear();
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	return 0;
}

/* Print Fast Path FDB Rule Database */
int fp_fdb_db_print(MV_FP_OP_TYPE op)
{
	unsigned long flags;

	spin_lock_irqsave(&nfp_mgr_lock, flags);
	mvFpFdbPrint();
	spin_unlock_irqrestore(&nfp_mgr_lock, flags);

	return 0;
}

#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */
