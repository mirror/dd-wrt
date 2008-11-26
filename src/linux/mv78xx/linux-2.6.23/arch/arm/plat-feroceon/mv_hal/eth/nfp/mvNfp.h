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
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

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
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/*******************************************************************************
* mvNfp.h - Header File for Marvell NFP (Routing and NAT)
*
* DESCRIPTION:
*       This header file contains macros, typedefs and function declarations 
* 	specific to the Marvell Network Fast Processing (Routing and NAT).
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __mvNfp_h__
#define __mvNfp_h__

/* includes */
#include "mvTypes.h"
#include "mvCommon.h"
#include "mvStack.h"
#include "mv802_3.h"
#include "eth/mvEth.h"
#include "mvSysHwConfig.h"

/* defines */
#define ETH_FP_MAX_HASH_DEPTH   8
#define ETH_FP_IFINDEX_MAX     32

/* uncomment to open some debug prints on adding and updating NFP rules */
#undef MV_FP_DEBUG

#ifdef MV_FP_DEBUG
#   define MV_NFP_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#   define MV_NFP_DBG(fmt, arg...)
#endif

#ifdef MV_FP_STATISTICS
# define MV_NFP_STAT(CODE) CODE;
#else
# define MV_NFP_STAT(CODE)
#endif

/* enumerations */
typedef enum {
	MV_FP_ROUTE_CMD, 		/* perform NFP routing */
	MV_FP_DROP_CMD, 		/* drop packet */
	MV_FP_TO_STACK_CMD,		/* pass packet to linux stack */
	MV_FP_DIP_CMD,			/* replace DIP */
	MV_FP_SIP_CMD,			/* replace SIP */
	MV_FP_DPORT_CMD,		/* replace DPORT */
	MV_FP_SPORT_CMD,		/* replace SPORT */
	MV_FP_BRIDGE_CMD, 		/* perform fast path bridging */
} MV_FP_CMD_TYPE;

#define MV_FP_NULL_BINDING  0
#define MV_FP_NULL_BINDING_SET	(1 << MV_FP_ROUTE_CMD)

#define MV_FP_DIP_CMD_MAP   (1 << MV_FP_DIP_CMD)
#define MV_FP_DPORT_CMD_MAP (1 << MV_FP_DPORT_CMD)
#define MV_FP_DNAT_CMD_MAP  (MV_FP_DIP_CMD_MAP | MV_FP_DPORT_CMD_MAP)

#define MV_FP_SIP_CMD_MAP   (1 << MV_FP_SIP_CMD)
#define MV_FP_SPORT_CMD_MAP (1 << MV_FP_SPORT_CMD)
#define MV_FP_SNAT_CMD_MAP  (MV_FP_SIP_CMD_MAP | MV_FP_SPORT_CMD_MAP)

typedef enum {
	MV_FP_STATIC_RULE, 	/* a static rule created by the user */
	MV_FP_DYNAMIC_RULE	/* a dynamic rule */

} MV_FP_RULE_TYPE;

/* structure definitions (used by the NFP Manager and the NFP Database)*/
typedef struct {
	MV_FP_CMD_TYPE	actionType;
	MV_U32		old_count;
	int 		new_count;
	MV_FP_RULE_TYPE	ruleType;
	int 		dnat_aware_refcnt;
	int 		snat_aware_refcnt;

} MV_FP_RULE_MGMT_INFO;

typedef struct {
	MV_U32  dstIp;
	MV_U32  srcIp;
  	MV_U32  defGtwIp;
	MV_U8   reserved;
	MV_U8   aware_flags; 
	/* dstMac should be 2 byte aligned */
	MV_U8   dstMac[MV_MAC_ADDR_SIZE]; 
	MV_U8   srcMac[MV_MAC_ADDR_SIZE];
	MV_U8   inIfIndex;	/* Linux interface index */
	MV_U8   outIfIndex;	/* Linux interface index */
	
} MV_FP_RULE_ROUTING_INFO;

typedef struct _mv_fp_rule {
	struct _mv_fp_rule *next;

	MV_FP_RULE_MGMT_INFO    mgmtInfo;
	MV_FP_RULE_ROUTING_INFO routingInfo;
	
} MV_FP_RULE;

struct ruleHashBucket {
	MV_FP_RULE *ruleChain; /* This is an entry in the rule hash table. */

	/* Add additional fields (such as a lock) here if required */
};

#define MV_JHASH_MIX(a, b, c)        \
{                                   \
    a -= b; a -= c; a ^= (c>>13);   \
    b -= c; b -= a; b ^= (a<<8);    \
    c -= a; c -= b; c ^= (b>>13);   \
    a -= b; a -= c; a ^= (c>>12);   \
    b -= c; b -= a; b ^= (a<<16);   \
    c -= a; c -= b; c ^= (b>>5);    \
    a -= b; a -= c; a ^= (c>>3);    \
    b -= c; b -= a; b ^= (a<<10);   \
    c -= a; c -= b; c ^= (b>>15);   \
}

/* The golden ration: an arbitrary value */
#define MV_JHASH_GOLDEN_RATIO           0x9e3779b9

extern MV_U32   fp_ip_jhash_iv;

static INLINE MV_U32 mv_jhash_3words(MV_U32 a, MV_U32 b, MV_U32 c, MV_U32 initval)
{
    a += MV_JHASH_GOLDEN_RATIO;    
    b += MV_JHASH_GOLDEN_RATIO;
    c += initval;
    MV_JHASH_MIX(a, b, c);

    return c;
}
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT

typedef struct _mv_fp_nat_rule {
	struct _mv_fp_nat_rule *next;
	MV_U32  old_count;
	MV_U32  new_count;

	/* Original packet information */
	/* Fields will contain invalid values if they are irrelevant */
	MV_U32  srcIp; 
	MV_U32  dstIp; 
	MV_U16  srcPort; 
	MV_U16  dstPort; 
	MV_U8   proto;

	/* NAT information */
	MV_U8   flags;
	MV_U32  newIp; 
	MV_U16  newPort;

} MV_FP_NAT_RULE;

struct natRuleHashBucket {
	MV_FP_NAT_RULE *natRuleChain; /* This is an entry in the NAT rule hash table. */

	/* Add additional fields (such as a lock) here if required */
};

extern MV_U32  nat_hash_depth[];

extern struct natRuleHashBucket *natRuleDb;
extern MV_U32                   natRuleDbSize;
extern MV_U32                   natRuleUpdateCount, natRuleSetCount, natRuleDeleteCount;
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT

#define MV_FP_FDB_IS_LOCAL 1

typedef struct {
	MV_U16  flags;
	MV_U8   mac[6];
	MV_U32  ifIndex;
	MV_U32  bridge;
} MV_FP_RULE_FDB_INFO;

typedef struct _mv_fp_fdb_rule {
	struct _mv_fp_fdb_rule *next;

	MV_FP_RULE_MGMT_INFO    mgmtInfo;
	MV_FP_RULE_FDB_INFO 	fdbInfo;
	
} MV_FP_FDB_RULE;

struct fdbRuleHashBucket {
	MV_FP_FDB_RULE *ruleChain;
};

extern MV_U32 fdb_hash_depth[];
extern struct fdbRuleHashBucket *fdbRuleDb;
extern MV_U32 fdbRuleDbSize;
extern MV_U32 fdbMember[];

#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */

typedef struct 
{
    MV_U32  parsing, process, multicast, non_ip, vlan_tagged;
    MV_U32  ip_not_found, ip_ttl_expired, ip_found;

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
    MV_U32  nat_bad_proto;
    MV_U32  dnat_aware;
    MV_U32  snat_aware;
    MV_U32  dnat_found;
    MV_U32  snat_found;
    MV_U32  dnat_not_found;
    MV_U32  snat_not_found;
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */

#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT
	MV_U32  fdb_rx_local;
	MV_U32  fdb_rx_unknown;
	MV_U32  fdb_tx_found;
	MV_U32  fdb_bridged;
#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */

} MV_FP_STATS;

static INLINE MV_IP_HEADER*   mvFpParsing(MV_PKT_INFO *pPktInfo, MV_FP_STATS* pFpStats)
{
    MV_U8           *pData;
    MV_IP_HEADER    *pIpHdr;
    MV_U32	    tx_status;

    pData = (MV_U8*)pPktInfo->pFrags[0].bufVirtPtr + ETH_MV_HEADER_SIZE;

    MV_NFP_STAT( pFpStats->parsing++ );

    /* Check LLC/SNAP and IP header */
    if( ((pPktInfo->status & ETH_RX_NOT_LLC_SNAP_FORMAT_MASK) == 0) ||
        ((pPktInfo->status & ETH_RX_IP_HEADER_OK_MASK) == 0) || 
        (pPktInfo->fragIP) )
    {
        /* Non IP packet: go to Linux IP stack */
        MV_NFP_STAT( pFpStats->non_ip++ );
        return NULL;
    }
    tx_status =  ( ETH_TX_GENERATE_IP_CHKSUM_MASK 
                 | ETH_TX_IP_NO_FRAG 
                 | (5 << ETH_TX_IP_HEADER_LEN_OFFSET) );  

    /* Calculate start of IP header */
    if( (pPktInfo->status & ETH_RX_VLAN_TAGGED_FRAME_MASK) )
    {
        MV_NFP_STAT(pFpStats->vlan_tagged++);
        pIpHdr = (MV_IP_HEADER*)(pData + sizeof(MV_802_3_HEADER) + MV_VLAN_HLEN);    
        tx_status |= ETH_TX_VLAN_TAGGED_FRAME_MASK;
    }
    else
    {
        pIpHdr = (MV_IP_HEADER*)(pData + sizeof(MV_802_3_HEADER));
    }

    if( (pPktInfo->status & ETH_RX_L4_TYPE_MASK) == ETH_RX_L4_TCP_TYPE )
    {
	tx_status |= (ETH_TX_L4_TCP_TYPE | ETH_TX_GENERATE_L4_CHKSUM_MASK);
    }
    else if( (pPktInfo->status & ETH_RX_L4_TYPE_MASK) == ETH_RX_L4_UDP_TYPE )
    {
	tx_status |= (ETH_TX_L4_UDP_TYPE | ETH_TX_GENERATE_L4_CHKSUM_MASK);
    }

    pPktInfo->status = tx_status;

    return pIpHdr;
}

/* Initialize NFP Rule Database (Routing + ARP information table) */
MV_STATUS   mvFpRuleDbInit(MV_U32 dbSize);
/* Clear NFP Rule Database (Routing + ARP information table) */
MV_STATUS   mvFpRuleDbClear(void);
/* Free Rule Database memory */
void        mvFpRuleDbDestroy(void);
/* Print NFP Rule Database (Routing + ARP information table) */
MV_STATUS   mvFpRuleDbPrint(void);
/* Copy all the information from src_rule to new_rule */
/* Warning - doesn't perform any checks on memory, just copies */
/* count is set to zero in new_rule */
/* Note: the next pointer is not updated . */
void        mvFpRuleCopy(MV_FP_RULE *newRule, const MV_FP_RULE *srcRule);
/* Get the maximum count value for a rule with srcIp == given ip */
MV_U32      mvFpMaxArpCountGet(MV_U32 ip);
/* Get the count value for a rule that matches the given SIP, DIP */
MV_U32      mvFpRouteCountGet(MV_U32 srcIp, MV_U32 dstIp);
/* Set a Routing Rule: create a new rule or update an existing rule  */
/* in the Routing + ARP information table */
MV_STATUS   mvFpRuleSet(MV_FP_RULE *rule);
/* Delete a specified rule from the Routing + ARP information table */
MV_STATUS   mvFpRuleDelete(MV_FP_RULE *rule);
/* Print a Rule */
void        mvFpRulePrint(const MV_FP_RULE *rule);
/* Enable NFP */
void        mvFpEnable(void);
/* Give all packets to Linux IP stack */
void        mvFpDisable(void);
MV_STATUS   mvFpInit(void);
int         mvFpProcess(MV_U32 inIfIndex, MV_U8* pData, MV_IP_HEADER* pIpHdr, MV_FP_STATS* pFpStats);
void        mvFpStatsPrint(MV_FP_STATS *pFpStats);

/* NAT SUPPORT Functions */

#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT
/* Find and return the first matching rule */
static INLINE MV_FP_NAT_RULE* mvFpNatRuleFind(MV_U32 dstIp, MV_U32 srcIp, 
                                              MV_U8 proto, MV_U16 dport, MV_U16 sport)
{
    MV_U32          hash, hash_tr;
    MV_FP_NAT_RULE* pNatRule;
    int             count = 0;

    hash = mv_jhash_3words(dstIp, srcIp, (MV_U32)((dport << 16) | sport), 
                            (MV_U32)((fp_ip_jhash_iv << 8) | proto));
    hash_tr = hash & (natRuleDbSize - 1);
/*
    mvOsPrintf("mvFpNatRuleFind: DIP=0x%08x, SIP=0x%08x, proto=%d, DPort=%d, SPort=%d, hash=0x%08x (0x%x)\n",
                dstIp, srcIp, proto, dport, sport, hash, hash_tr);
*/
    pNatRule = natRuleDb[hash_tr].natRuleChain;

    while(pNatRule)
    {
        /* look for a matching rule */
        if( (pNatRule->dstIp == dstIp) && 
            (pNatRule->srcIp == srcIp) &&
            (pNatRule->proto == proto) &&
            (pNatRule->dstPort  == dport) &&
            (pNatRule->srcPort  == sport) )
        {
            MV_NFP_STAT( nat_hash_depth[count]++);
	        pNatRule->new_count++;
            return pNatRule;
        }
        pNatRule = pNatRule->next;
        count++;
    }
    return NULL;
}

/* Initialize NFP NAT Rule Database (SNAT + DNAT table) */
MV_STATUS   mvFpNatDbInit(MV_U32 dbSize);

/* Clear NFP NAT Rule Database (SNAT + DNAT table) */
MV_STATUS   mvFpNatDbClear(void);

/* Free NAT Database memory */
void        mvFpNatDbDestroy(void);

/* Set a NAT rule: create a new rule or update an existing rule in the SNAT + DNAT table */
MV_STATUS   mvFpNatRuleSet(MV_FP_NAT_RULE *natRule);

MV_STATUS   mvFpRuleAwareSet(MV_FP_RULE *pSetRule);

/* Delete a specified NAT rule from the SNAT + DNAT table */
MV_STATUS   mvFpNatRuleDelete(MV_FP_NAT_RULE *natRule);

/* Get the count value for a NAT rule */
MV_U32      mvFpNatCountGet(MV_U32 srcIp, MV_U32 dstIp, MV_U16 srcPort, MV_U16 dstPort, MV_U8 proto);

/* Print NFP NAT Rule Database (SNAT + DNAT table) */
MV_STATUS   mvFpNatDbPrint(void);

/* Print a NAT Rule */
void        mvFpNatRulePrint(const MV_FP_NAT_RULE *rule);

/* Extract dstPort and srcPort values from the packet */
MV_U8       mvFpNatPortsGet(MV_IP_HEADER* pIpHdr, MV_U16* pDstPort, MV_U16* pSrcPort);

int         mvFpNatPktUpdate(MV_IP_HEADER* pIpHdr, MV_FP_NAT_RULE* pDnatRule, MV_FP_NAT_RULE* pSnatRule);

#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */


#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT

/* Init NFP Bridge Rule Database */
MV_STATUS 	mvFpFdbInit(MV_U32 dbSize);

/* Clear NFP Bridge Rule Database */
MV_STATUS 	mvFpFdbClear(void);

/* Destroy NFP Bridge Rule Database */
void 		mvFpFdbDestroy(void);

/* Add NFP Bridge Rule */
MV_STATUS 	mvFpFdbRuleSet(MV_FP_FDB_RULE *rule);

/* Delete NFP Bridge Rule Database */
MV_STATUS 	mvFpFdbRuleDel(MV_FP_FDB_RULE *rule);

/* Aging NFP Bridge Rule Database */
MV_U32 		mvFpFdbRuleAge(MV_FP_FDB_RULE *rule);

/* Print NFP Bridge Rule Database */
MV_STATUS 	mvFpFdbPrint(void);

#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */

#endif /* __mvNfp_h__ */
