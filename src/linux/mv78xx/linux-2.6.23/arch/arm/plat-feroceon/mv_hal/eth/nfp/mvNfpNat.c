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


/* includes */
#include "mvTypes.h"
#include "mvOs.h"
#include "mvStack.h"
#include "mvDebug.h"
#include "eth/nfp/mvNfp.h"
#include "eth/mvEth.h"


struct natRuleHashBucket *natRuleDb;
MV_U32                   natRuleDbSize;

MV_U32                      natRuleUpdateCount = 0;
MV_U32                      natRuleSetCount = 0;
MV_U32                      natRuleDeleteCount = 0;

/* Initialize NFP NAT Rule Database (SNAT + DNAT table) */
MV_STATUS   mvFpNatDbInit(MV_U32 dbSize)
{
	natRuleDb = (struct natRuleHashBucket *)mvOsMalloc(sizeof(struct natRuleHashBucket)*dbSize);
	if (natRuleDb == NULL) {
		mvOsPrintf("NFP NAT Rule DB: Not Enough Memory\n");
		return MV_NO_RESOURCE;
	}
	natRuleDbSize = dbSize;
	memset(natRuleDb, 0, sizeof(struct natRuleHashBucket)*natRuleDbSize);

    natRuleSetCount = natRuleDeleteCount = natRuleUpdateCount = 0;

    mvOsPrintf("mvFpNatDb (%p): %d entries, %d bytes\n", 
                natRuleDb, natRuleDbSize, sizeof(struct natRuleHashBucket)*natRuleDbSize);

	return MV_OK;
}

/* Clear NFP NAT Rule Database (SNAT + DNAT table) */
MV_STATUS   mvFpNatDbClear(void)
{
	MV_U32 i = 0;
	MV_FP_NAT_RULE *currRule;
	MV_FP_NAT_RULE *tmpRule;

	if (natRuleDb == NULL) 
		return MV_NOT_INITIALIZED;

	for (i = 0; i < natRuleDbSize; i++) {
		currRule = natRuleDb[i].natRuleChain;
		while (currRule != NULL) {
			tmpRule = currRule;
			currRule = currRule->next;
			mvOsFree(tmpRule);
		}
        	natRuleDb[i].natRuleChain = NULL;
	}
	return MV_OK;
}

/* Free NAT Database memory */
void mvFpNatDbDestroy(void)
{
	if (natRuleDb != NULL)
		mvOsFree(natRuleDb);
}

static void mvFpNatRuleUpdate(MV_FP_NAT_RULE *dstRule, const MV_FP_NAT_RULE *srcRule)
{
    dstRule->flags   = srcRule->flags;
    dstRule->newIp   = srcRule->newIp;
    dstRule->newPort = srcRule->newPort;
    dstRule->new_count = srcRule->new_count;
    dstRule->old_count = srcRule->old_count;
}

/* Set a NAT rule: create a new rule or update an existing rule in the SNAT + DNAT table */
MV_STATUS mvFpNatRuleSet(MV_FP_NAT_RULE *pSetRule)
{
    MV_U32          hash, hash_tr;
    MV_FP_NAT_RULE  *pNatRule, *pNewRule;

    hash = mv_jhash_3words(pSetRule->dstIp, pSetRule->srcIp, 
                            (MV_U32)((pSetRule->dstPort << 16) | pSetRule->srcPort), 
                            (MV_U32)((fp_ip_jhash_iv << 8) | pSetRule->proto));
    hash_tr = hash & (natRuleDbSize - 1);
    pNatRule = natRuleDb[hash_tr].natRuleChain;

    while(pNatRule)
    {
        /* look for a matching rule */
        if( (pNatRule->dstIp == pSetRule->dstIp) && 
            (pNatRule->srcIp == pSetRule->srcIp) &&
            (pNatRule->proto == pSetRule->proto) &&
            (pNatRule->dstPort  == pSetRule->dstPort) &&
            (pNatRule->srcPort  == pSetRule->srcPort) )
        {
            /* update rule */
            mvFpNatRuleUpdate(pNatRule, pSetRule);
            natRuleUpdateCount++;

#ifdef MV_FP_DEBUG
            mvOsPrintf("UpdNAT_%03u: DIP=0x%08x, SIP=0x%08x, proto=%d, DPort=%d, SPort=%d, hash=0x%04x, flags=0x%02x\n",
                        natRuleUpdateCount, pNatRule->dstIp, pNatRule->srcIp, pNatRule->proto, 
                        MV_16BIT_BE(pNatRule->dstPort), MV_16BIT_BE(pNatRule->srcPort), hash_tr, pNatRule->flags);
#endif
            return MV_OK;
        }
        pNatRule = pNatRule->next;
    }
    /* Allocate new entry */
    pNewRule = mvOsMalloc(sizeof(MV_FP_NAT_RULE));
    if(pNewRule == NULL)
    {
        mvOsPrintf("mvFpNatRuleSet: Can't allocate new rule\n");
        return MV_FAIL;
    }

    memcpy(pNewRule, pSetRule, sizeof(*pNewRule));
    pNewRule->next = NULL;

    if(natRuleDb[hash_tr].natRuleChain == NULL)
    {
        natRuleDb[hash_tr].natRuleChain = pNewRule;
    }
    else 
    {
	    pNatRule = natRuleDb[hash_tr].natRuleChain;
        
        while (pNatRule->next != NULL)
	        pNatRule = pNatRule->next;	    

	    pNatRule->next = pNewRule;
    }
    natRuleSetCount++;

#ifdef MV_FP_DEBUG
    mvOsPrintf("SetNAT_%03u: DIP=0x%08x, SIP=0x%08x, proto=%d, DPort=%d, SPort=%d, hash=0x%04x, flags=0x%02x\n",
                natRuleSetCount, pNewRule->dstIp, pNewRule->srcIp, pNewRule->proto, 
                MV_16BIT_BE(pNewRule->dstPort), MV_16BIT_BE(pNewRule->srcPort), hash_tr, pNewRule->flags);
#endif
    return MV_OK;
}

/* Delete a specified NAT rule from the SNAT + DNAT table */
MV_STATUS mvFpNatRuleDelete(MV_FP_NAT_RULE *natRule)
{
    MV_U32      hash, hash_tr;
    MV_FP_NAT_RULE  *currRule, *prevRule;

    natRuleDeleteCount++;

    hash = mv_jhash_3words(natRule->dstIp, natRule->srcIp, 
                            (MV_U32)((natRule->dstPort << 16) | natRule->srcPort), 
                            (MV_U32)((fp_ip_jhash_iv << 8) | natRule->proto));
    hash_tr = hash & (natRuleDbSize - 1);

    prevRule = NULL;
    for (currRule = natRuleDb[hash_tr].natRuleChain; 
	 currRule != NULL; 
	 prevRule = currRule, currRule = currRule->next) 
    {
	    if (currRule->srcIp == natRule->srcIp && 
		currRule->dstIp == natRule->dstIp && 
		currRule->srcPort == natRule->srcPort && 
		currRule->dstPort == natRule->dstPort && 
		currRule->proto == natRule->proto ) 
	    {		
		    if (prevRule == NULL)
			    natRuleDb[hash_tr].natRuleChain = currRule->next;
		    else
			    prevRule->next = currRule->next;

#ifdef MV_FP_DEBUG
		    mvOsPrintf("DelNAT_%03u: DIP=0x%08x, SIP=0x%08x, proto=%d, DPort=%d, SPort=%d, hash=0x%04x\n",
              	        natRuleDeleteCount, currRule->dstIp, currRule->srcIp, currRule->proto, 
			            MV_16BIT_BE(currRule->dstPort), MV_16BIT_BE(currRule->srcPort), hash_tr);
#endif
		mvOsFree(currRule);	
		return MV_OK;
	    }
    }
    return MV_NOT_FOUND;
}

/* Check that protocol supported for FP NAT and extract srcPort and dstPort 
 *  (or their equivalents)  from the packet.
 */
MV_U8   mvFpNatPortsGet(MV_IP_HEADER* pIpHdr, MV_U16* pDstPort, MV_U16* pSrcPort)
{
    MV_U8               proto = pIpHdr->protocol;
    MV_UDP_HEADER       *pUdpHdr;
    MV_TCP_HEADER       *pTcpHdr;
    MV_ICMP_ECHO_HEADER *pIcmpHdr;

    switch(proto)
    {
        case MV_IP_PROTO_TCP:
            pTcpHdr = (MV_TCP_HEADER*)((unsigned)pIpHdr + sizeof(MV_IP_HEADER));
            *pDstPort = pTcpHdr->dest;
            *pSrcPort = pTcpHdr->source;
            break;

        case MV_IP_PROTO_UDP:
            pUdpHdr = (MV_UDP_HEADER*)((unsigned)pIpHdr + sizeof(MV_IP_HEADER));
            *pDstPort = pUdpHdr->dest;
            *pSrcPort = pUdpHdr->source;
            break;

        case MV_IP_PROTO_ICMP:
            pIcmpHdr = (MV_ICMP_ECHO_HEADER*)((unsigned)pIpHdr + sizeof(MV_IP_HEADER));
            if( (pIcmpHdr->type == MV_ICMP_ECHO) || (pIcmpHdr->type == MV_ICMP_ECHOREPLY) )
            {
                *pDstPort = (pIcmpHdr->code << 8) | (pIcmpHdr->type);
                *pSrcPort = pIcmpHdr->id;
            }
            else
            {   
                /* Do NAT for IP + protocol only (without ports) */
                *pDstPort = 0;
                *pSrcPort = 0;
            }
            break;

	case MV_IP_PROTO_ZERO_HOP:
               /* Do NAT for IP + protocol only (without ports) */
                *pDstPort = 0;
                *pSrcPort = 0;
	    break;

        /* Other protocols supporting NAT only without ports 
         * case ???????:
         * case ???????:
         *     *pDstPort = 0;
         *     *pSrcPort = 0;
         *     break
         *
         */

        default:
            /* Skip NAT processing at all */
            proto = MV_IP_PROTO_NULL;
    }
    return proto;
}

int    mvFpNatPktUpdate(MV_IP_HEADER* pIpHdr, MV_FP_NAT_RULE* pDnatRule, MV_FP_NAT_RULE* pSnatRule)
{
    MV_UDP_HEADER       *pUdpHdr;
    MV_TCP_HEADER       *pTcpHdr;
    MV_ICMP_ECHO_HEADER *pIcmpHdr;
    int                 hdr_size = 0;
    MV_U16              *pDstPort=NULL, *pSrcPort=NULL;

    switch(pIpHdr->protocol)
    {
        case MV_IP_PROTO_TCP:
            pTcpHdr = (MV_TCP_HEADER*)((unsigned)pIpHdr + sizeof(MV_IP_HEADER));
            pDstPort = &pTcpHdr->dest;
            pSrcPort = &pTcpHdr->source;
            hdr_size = sizeof(MV_TCP_HEADER);
            break;

        case MV_IP_PROTO_UDP:
            pUdpHdr = (MV_UDP_HEADER*)((unsigned)pIpHdr + sizeof(MV_IP_HEADER));
            pDstPort = &pUdpHdr->dest;
            pSrcPort = &pUdpHdr->source;
            hdr_size = sizeof(MV_UDP_HEADER);
            break;

        case MV_IP_PROTO_ICMP:
            pIcmpHdr = (MV_ICMP_ECHO_HEADER*)((unsigned)pIpHdr + sizeof(MV_IP_HEADER));
            if( (pIcmpHdr->type == MV_ICMP_ECHO) || (pIcmpHdr->type == MV_ICMP_ECHOREPLY) )
            {
                pDstPort = &pIcmpHdr->id;
                pSrcPort = &pIcmpHdr->id;
                hdr_size = sizeof(MV_ICMP_ECHO_HEADER);
            }
            else
            {
                mvOsPrintf("Wrong ICMP type: 0x%x\n", pIcmpHdr->type & 0xFF);
            }
            break;

	    case MV_IP_PROTO_ZERO_HOP:
	        /* Do nothing - only IP addresses are updated for this protocol */
	        break;

        default:
            mvOsPrintf("Unexpected IP protocol: 0x%x\n", 
                        pIpHdr->protocol);
    }
    if(pDnatRule != NULL)
    {
        if(pDnatRule->flags & MV_FP_DIP_CMD_MAP)
            pIpHdr->dstIP = pDnatRule->newIp;
        if( (pDnatRule->flags & MV_FP_DPORT_CMD_MAP) && 
            (pDstPort != NULL) )
            *pDstPort = pDnatRule->newPort;
    }

    if(pSnatRule != NULL)
    {
        if(pSnatRule->flags & MV_FP_SIP_CMD_MAP)
            pIpHdr->srcIP = pSnatRule->newIp;

        if( (pSnatRule->flags & MV_FP_SPORT_CMD_MAP) && 
            (pSrcPort != NULL) )
            *pSrcPort = pSnatRule->newPort;
    }
    return hdr_size;
}

/* Print a NFP NAT Rule */
void    mvFpNatRulePrint(const MV_FP_NAT_RULE *rule)
{
	/* Note: some of the fields in the NAT rule may contain invalid values */
    mvOsPrintf("Original packet: ");
    mvOsPrintf("SIP=");
    mvDebugPrintIpAddr(MV_32BIT_BE(rule->srcIp)), 
    mvOsPrintf(", DIP=");
    mvDebugPrintIpAddr(MV_32BIT_BE(rule->dstIp)), 
    mvOsPrintf(", SPort=%d", MV_16BIT_BE(rule->srcPort));
    mvOsPrintf(", DPort=%d", MV_16BIT_BE(rule->dstPort)); 
    mvOsPrintf("\nNAT Info: ");
    mvOsPrintf("count=%u, flags=0x%x", rule->new_count, rule->flags);
    mvOsPrintf(", newIP=");
    mvDebugPrintIpAddr(MV_32BIT_BE(rule->newIp)); 
    mvOsPrintf(", newPort=%d", MV_16BIT_BE(rule->newPort));
    mvOsPrintf("\n");
}

/* Print NFP NAT Rule Database (SNAT + DNAT table) */
MV_STATUS   mvFpNatDbPrint(void)
{
    MV_U32 count, i = 0;
    MV_FP_NAT_RULE *currRule;
	
    mvOsPrintf("\nPrinting NFP NAT Rule Database: \n");
    count = 0;
    for (i=0; i<natRuleDbSize; i++) 
    {
	currRule = natRuleDb[i].natRuleChain;

        if (currRule != NULL)
    	    mvOsPrintf("\n%03u: NAT DB hash=0x%x\n", count, i);

       	while (currRule != NULL) 
        {
	    if( (currRule->flags != MV_FP_NULL_BINDING) || (currRule->new_count > 0) )
            {
                mvOsPrintf("%03u: Rule=%p, Next=%p\n", count, currRule, currRule->next);
	        mvFpNatRulePrint(currRule);
            }
	    currRule = currRule->next;
            count++;
       	}
    }
    return MV_OK;
}

/* Get the count value for a NAT rule */
MV_U32 mvFpNatCountGet(MV_U32 srcIp, MV_U32 dstIp, MV_U16 srcPort, MV_U16 dstPort, MV_U8 proto)
{
    MV_U32      hash, hash_tr;
    MV_FP_NAT_RULE  *pNatRule;

    hash = mv_jhash_3words(dstIp, srcIp, (MV_U32)((dstPort << 16) | srcPort), 
                            (MV_U32)((fp_ip_jhash_iv << 8) | proto));
    hash_tr = hash & (natRuleDbSize - 1);
    pNatRule = natRuleDb[hash_tr].natRuleChain;

    while(pNatRule)
    {
        /* look for a matching rule */
        if( (pNatRule->dstIp == dstIp) && 
            (pNatRule->srcIp == srcIp) &&
            (pNatRule->proto == proto) &&
            (pNatRule->dstPort  == dstPort) &&
            (pNatRule->srcPort  == srcPort) )
        {
	    return pNatRule->new_count;
	}
	pNatRule = pNatRule->next;
    }
    return 0;
}

