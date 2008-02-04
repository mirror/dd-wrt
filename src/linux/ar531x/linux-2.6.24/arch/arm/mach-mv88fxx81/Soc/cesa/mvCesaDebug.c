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

#include "mvOs.h"
#include "mvDebug.h"

#include "mvIdma.h"
#include "mvMD5.h"
#include "mvSHA1.h"

#include "mvCesa.h"
#include "mvCesaRegs.h"
#include "mvAes.h"
#include "mvLru.h"

extern MV_U8*   mvCesaSramAddrGet(void);
extern MV_ULONG mvCesaSramVirtToPhys(void* pDev, MV_U8* pSramVirt);

const char*   mvCesaDebugOperStr(MV_CESA_OPERATION oper)
{
    switch(oper)
    {
        case MV_CESA_MAC_ONLY:
            return "MacOnly";

        case MV_CESA_CRYPTO_ONLY:
            return "CryptoOnly";

        case MV_CESA_MAC_THEN_CRYPTO:
            return "MacCrypto";

        case MV_CESA_CRYPTO_THEN_MAC:
            return "CryptoMac";

        default:
            break;
    }
    return "Null";
}

const char* mvCesaDebugCryptoAlgStr(MV_CESA_CRYPTO_ALG cryptoAlg)
{
    switch(cryptoAlg)
    {
        case MV_CESA_CRYPTO_DES:
            return "DES";

        case MV_CESA_CRYPTO_3DES:
            return "3DES";

        case MV_CESA_CRYPTO_AES:
            return "AES";

        default:
            break;
    }
    return "Null";
}

const char* mvCesaDebugMacModeStr(MV_CESA_MAC_MODE macMode)
{
    switch(macMode)
    {
        case MV_CESA_MAC_MD5:
            return "MD5";

        case MV_CESA_MAC_SHA1:
            return "SHA1";

        case MV_CESA_MAC_HMAC_MD5:
            return "HMAC-MD5";

        case MV_CESA_MAC_HMAC_SHA1:
            return "HMAC_SHA1";

        default:
            break;
    }
    return "Null";
}

void    mvCesaDebugCmd(MV_CESA_COMMAND* pCmd,  int mode)
{
    mvOsPrintf("pCmd=%p, pReqPrv=%p, pSrc=%p, pDst=%p, pCB=%p, sid=%d\n", 
                pCmd, pCmd->pReqPrv, pCmd->pSrc, pCmd->pDst, 
                pCmd->pFuncCB, pCmd->sessionId);
    mvOsPrintf("isUser=%d, ivOffs=%d, crOffs=%d, crLen=%d, digest=%d, macOffs=%d, macLen=%d\n",
                pCmd->ivFromUser, pCmd->ivOffset, pCmd->cryptoOffset, pCmd->cryptoLength,
                pCmd->digestOffset, pCmd->macOffset, pCmd->macLength);
}

/* no need to use in tool */
void     mvCesaDebugMbuf(char* str, MV_CESA_MBUF *pMbuf, int offset, int size) 
{ 
    int frag, len, fragOffset;

    if(str != NULL)
        mvOsPrintf("%s: pMbuf=%p, numFrags=%d, mbufSize=%d\n", 
                    str, pMbuf, pMbuf->numFrags, pMbuf->mbufSize); 

    frag = mvCesaMbufOffset(pMbuf, offset, &fragOffset);
    if(frag == MV_INVALID)
    {
        mvOsPrintf("CESA Mbuf Error: offset (%d) out of range\n", offset);
        return;
    }

    for(; frag<pMbuf->numFrags; frag++)
    {
        mvOsPrintf("#%2d. bufVirt=%p, bufSize=%d\n", 
                    frag, pMbuf->pFrags[frag].bufVirtPtr, 
                    pMbuf->pFrags[frag].bufSize);
        if(size > 0)
        {
            len = MV_MIN(pMbuf->pFrags[frag].bufSize, size);
            mvDebugMemDump(pMbuf->pFrags[frag].bufVirtPtr+fragOffset, len, 1);
            size -= len;
            fragOffset = 0;
        }
    }
}

void    mvCesaDebugRegs(void)
{
    int chan;

    mvOsPrintf("\t CESA Registers:\n");

    mvOsPrintf("MV_CESA_CMD_REG                     : 0x%X = 0x%08x\n", 
                MV_CESA_CMD_REG, 
                MV_REG_READ( MV_CESA_CMD_REG ) );    

    for(chan=0; chan<MV_CESA_MAX_CHAN; chan++)
    {
        mvOsPrintf("MV_CESA_CHAN_DESC_OFFSET_REG(%d)     : 0x%X = 0x%08x\n", 
                chan, MV_CESA_CHAN_DESC_OFFSET_REG(chan), 
                MV_REG_READ( MV_CESA_CHAN_DESC_OFFSET_REG(chan) ) );    
    }

    mvOsPrintf("MV_CESA_CFG_REG                     : 0x%X = 0x%08x\n", 
                MV_CESA_CFG_REG, 
                MV_REG_READ( MV_CESA_CFG_REG ) );    

    mvOsPrintf("MV_CESA_STATUS_REG                  : 0x%X = 0x%08x\n", 
                MV_CESA_STATUS_REG, 
                MV_REG_READ( MV_CESA_STATUS_REG ) );    

    mvOsPrintf("MV_CESA_ISR_CAUSE_REG               : 0x%X = 0x%08x\n", 
                MV_CESA_ISR_CAUSE_REG, 
                MV_REG_READ( MV_CESA_ISR_CAUSE_REG ) );    

    mvOsPrintf("MV_CESA_ISR_MASK_REG                : 0x%X = 0x%08x\n", 
                MV_CESA_ISR_MASK_REG, 
                MV_REG_READ( MV_CESA_ISR_MASK_REG ) );
}

void    mvCesaDebugStatus(void)
{
    int     chan;

    mvOsPrintf("\n\t CESA Status\n\n");

    mvOsPrintf("sramVirt=%p, sramPhys=0x%x, maxCacheSA=%d, pCacheLRU=%p\n",
                cesaSramVirtPtr, (MV_U32)mvCesaSramVirtToPhys(NULL, (MV_U8*)cesaSramVirtPtr), 
                MV_CESA_MAX_CACHE_SA, pCesaCacheLRU);

    mvOsPrintf("pReqQ=%p, qDepth=%d, reqSize=%d bytes, qRes=%d, readyMap=0x%x\n", 
                pCesaReqFirst, cesaQueueDepth, sizeof(MV_CESA_REQ), 
                cesaReqResources, cesaChanReadyMap);

    mvOsPrintf("pSAD=%p, maxSA=%d, sizeSA=%d bytes\n",
                pCesaSAD, cesaMaxSA, sizeof(MV_CESA_SA));

    mvOsPrintf("\n");
    for(chan=0; chan<MV_CESA_MAX_CHAN; chan++)
    {
        mvOsPrintf("Chan #%d: pChan=%p, state=%d, idmaList=%p, sramBufOffset=%d\n", 
                    chan, &pCesaChan[chan], pCesaChan[chan].state, 
                    pCesaChan[chan].pIdmaList, pCesaChan[chan].sramBufOffset);
        mvOsPrintf("          pSramBuf=%p, bufSize=%d, pSramDesc=%p, descSize=%d\n", 
                   cesaSramVirtPtr->buf[chan], MV_CESA_MAX_BUF_SIZE, 
                   &cesaSramVirtPtr->desc[chan], sizeof(MV_CESA_DESC));
    }
    mvCesaDebugRegs();
    mvCesaDebugStats();
}

void    mvCesaDebugDescriptor(MV_CESA_DESC* pDesc)
{
    mvOsPrintf("config=0x%08x, crSrcOffs=0x%04x, crDstOffs=0x%04x\n", 
            pDesc->config, pDesc->cryptoSrcOffset, pDesc->cryptoDstOffset);

    mvOsPrintf("crLen=0x%04x, crKeyOffs=0x%04x, ivOffs=0x%04x, ivBufOffs=0x%04x\n", 
            pDesc->cryptoDataLen, pDesc->cryptoKeyOffset, 
            pDesc->cryptoIvOffset, pDesc->cryptoIvBufOffset);

    mvOsPrintf("macSrc=0x%04x, digest=0x%04x, macLen=0x%04x, inIv=0x%04x, outIv=0x%04x\n", 
            pDesc->macSrcOffset, pDesc->macDigestOffset, pDesc->macDataLen,
            pDesc->macInnerIvOffset, pDesc->macOuterIvOffset);
}

void    mvCesaDebugChan(int chan, int mode)
{
    MV_CESA_DESC    *pDesc = &cesaSramVirtPtr->desc[chan];

    mvOsPrintf("Chan #%d: pChan=%p, state=%d, idmaList=%p, sramBufOffset=%d\n", 
                chan, &pCesaChan[chan], pCesaChan[chan].state, 
                pCesaChan[chan].pIdmaList, pCesaChan[chan].sramBufOffset);
    mvOsPrintf("          pSramBuf=%p, bufSize=%d, pSramDesc=%p, descSize=%d\n", 
                   cesaSramVirtPtr->buf[chan], MV_CESA_MAX_BUF_SIZE, 
                   &cesaSramVirtPtr->desc[chan], sizeof(MV_CESA_DESC));
    if(mode != 0)
    {
        mvCesaDebugDescriptor(pDesc);
    }
    mvIdmaRegs(chan);
}

void    mvCesaDebugQueue(int mode)
{
    mvOsPrintf("\n\t CESA Request Queue:\n\n");

    mvOsPrintf("pFirstReq=%p, pLastReq=%p, qDepth=%d, reqSize=%d bytes\n", 
                pCesaReqFirst, pCesaReqLast, cesaQueueDepth, sizeof(MV_CESA_REQ));

    mvOsPrintf("pEmpty=%p, pProcess=%p, pReady=%p, qResources=%d\n",
                pCesaReqEmpty, pCesaReqProcess, pCesaReqReady, 
                cesaReqResources);

    if(mode != 0)
    {
        int             count = 0;
        MV_CESA_REQ*    pReq = pCesaReqFirst;

        for(count=0; count<cesaQueueDepth; count++)
        {
            /* Print out requsts */
            mvOsPrintf("%02d. pReq=%p, state=%d, fragMode=0x%x, chanId=%d, pCmd=%p\n",
                count, pReq, pReq->state, pReq->fragMode, pReq->chanId, pReq->pCmd);
            if(mode > 1)
            {
                /* Print out Command */
                mvCesaDebugCmd(pReq->pCmd, mode);
            }
            pReq++;
        }
    }
}


void    mvCesaDebugCacheSA(MV_CESA_CACHE_SA* pCacheSA, int mode)
{
    if(pCacheSA == NULL)
    {
        mvOsPrintf("cesaCacheSA: Unexpected pCacheSA=%p\n", pCacheSA);
        return;
    }
    mvOsPrintf("pCacheSA=%p, sizeCacheSA=%d bytes, sid=%d\n", 
                pCacheSA, sizeof(MV_CESA_CACHE_SA), pCacheSA->sid);

    if(mode != 0) 
    {
        mvOsPrintf("cryptoKey=%p, maxCryptoKey=%d bytes\n",
                    pCacheSA->cryptoKey, MV_CESA_MAX_CRYPTO_KEY_LENGTH);
        mvDebugMemDump(pCacheSA->cryptoKey, MV_CESA_MAX_CRYPTO_KEY_LENGTH, 1);

        mvOsPrintf("macInnerIV=%p, maxInnerIV=%d bytes\n",
                    pCacheSA->macInnerIV, MV_CESA_MAX_DIGEST_SIZE);
        mvDebugMemDump(pCacheSA->macInnerIV, MV_CESA_MAX_DIGEST_SIZE, 1);

        mvOsPrintf("macOuterIV=%p, maxOuterIV=%d bytes\n",
                    pCacheSA->macOuterIV, MV_CESA_MAX_DIGEST_SIZE);
        mvDebugMemDump(pCacheSA->macOuterIV, MV_CESA_MAX_DIGEST_SIZE, 1);
    }
}

void    mvCesaDebugSA(short sid, int mode)
{
    MV_CESA_OPERATION   oper;
    MV_CESA_DIRECTION   dir;
    MV_CESA_CRYPTO_ALG  cryptoAlg;
    MV_CESA_CRYPTO_MODE cryptoMode;
    MV_CESA_MAC_MODE    macMode;
    MV_CESA_SA*         pSA = &pCesaSAD[sid];

    if( (pSA->valid) || (pSA->count != 0) )
    {
        mvOsPrintf("\n\nCESA SA Entry #%d (%p) - %s (count=%d)\n", 
                    sid, pSA, 
                    pSA->valid ? "Valid" : "Invalid", pSA->count);

        oper = (pSA->config & MV_CESA_OPERATION_MASK) >> MV_CESA_OPERATION_OFFSET;
        dir  = (pSA->config & MV_CESA_DIRECTION_MASK) >> MV_CESA_DIRECTION_BIT;
        mvOsPrintf("%s - %s ", mvCesaDebugOperStr(oper), 
                    (dir == MV_CESA_DIR_ENCODE) ? "Encode" : "Decode");
        if(oper != MV_CESA_MAC_ONLY)
        {
            cryptoAlg = (pSA->config & MV_CESA_CRYPTO_ALG_MASK) >> MV_CESA_CRYPTO_ALG_OFFSET;
            cryptoMode = (pSA->config & MV_CESA_CRYPTO_MODE_MASK) >> MV_CESA_CRYPTO_MODE_BIT;
            mvOsPrintf("- %s - %s ", mvCesaDebugCryptoAlgStr(cryptoAlg),
                        (cryptoMode == MV_CESA_CRYPTO_ECB) ? "ECB" : "CBC");
        }
        if(oper != MV_CESA_CRYPTO_ONLY)
        {
            macMode = (pSA->config & MV_CESA_MAC_MODE_MASK) >> MV_CESA_MAC_MODE_OFFSET;
            mvOsPrintf("- %s ", mvCesaDebugMacModeStr(macMode));            
        }
        mvOsPrintf("\n");

        if(mode != 0)
        {
            mvOsPrintf("config=0x%08x, cryptoKeySize=%d, digestSize=%d, cacheIdx=%d\n",
                        pCesaSAD[sid].config, pCesaSAD[sid].cryptoKeyLength, 
                        pCesaSAD[sid].digestSize, pCesaSAD[sid].cacheIdx);

            mvCesaDebugCacheSA(&pCesaSAD[sid].cacheSA, mode);
        }
    }
}

/**/
void    mvCesaDebugCacheIdx(int idx)
{
    mvOsPrintf("\n\t CESA Sram Cache SA entry #%d\n\n", idx);

    mvCesaDebugCacheSA(&cesaSramVirtPtr->cacheSA[idx], 1);
}

/**/
void    mvCesaDebugSram(int mode)
{
    int     chan, idx;

    mvOsPrintf("\n\t SRAM contents: base=%p, size=%d, pVirt=%p, physAddr=0x%x\n\n",
            mvCesaSramAddrGet(), sizeof(MV_CESA_SRAM_MAP), cesaSramVirtPtr,
            (MV_U32)mvCesaSramVirtToPhys(NULL, (MV_U8*)cesaSramVirtPtr) );

    for(chan=0; chan<MV_CESA_MAX_CHAN; chan++)
    {
        mvOsPrintf("\n\t Sram buffer #%d: size=%d, pVirt=%p, offs=0x%x, physAddr=0x%x\n",
                    chan, MV_CESA_MAX_BUF_SIZE, cesaSramVirtPtr->buf[chan],
                    pCesaChan[chan].sramBufOffset, 
                    (MV_U32)mvCesaSramVirtToPhys(NULL, cesaSramVirtPtr->buf[chan]));
        if(mode != 0)
            mvDebugMemDump(cesaSramVirtPtr->buf[chan], 64, 1);
    }
    mvOsPrintf("\n");
    for(chan=0; chan<MV_CESA_MAX_CHAN; chan++)
    {
        mvOsPrintf("\n\t Sram descriptor #%d: size=%d, pVirt=%p, physAddr=0x%x\n",
                    chan, sizeof(MV_CESA_DESC), &cesaSramVirtPtr->desc[chan],
                    (MV_U32)mvCesaSramVirtToPhys(NULL, (MV_U8*)&cesaSramVirtPtr->desc[chan]));
        if(mode != 0)
        {
            mvOsPrintf("\n");
            mvCesaDebugDescriptor(&cesaSramVirtPtr->desc[chan]);
        }
    }
    for(chan=0; chan<MV_CESA_MAX_CHAN; chan++)
    {
        mvOsPrintf("\n\t Sram IV #%d: size=%d, pVirt=%p, physAddr=0x%x\n",
                    chan, MV_CESA_MAX_IV_LENGTH, &cesaSramVirtPtr->cryptoIV[chan],
                    (MV_U32)mvCesaSramVirtToPhys(NULL, (MV_U8*)&cesaSramVirtPtr->desc[chan]));
        if(mode != 0)
        {
            mvOsPrintf("\n");
            mvDebugMemDump(cesaSramVirtPtr->cryptoIV[chan], MV_CESA_MAX_IV_LENGTH, 1);
        }
    }
    mvOsPrintf("\n");
    for(idx=0; idx<MV_CESA_MAX_CACHE_SA; idx++)
    {
        mvCesaDebugCacheSA(&cesaSramVirtPtr->cacheSA[idx], 0);
    }
}

void    mvCesaDebugSAD(int mode)
{
    int sid;

    mvOsPrintf("\n\t Cesa SAD status: pSAD=%p, maxSA=%d\n",
                pCesaSAD, cesaMaxSA);

    for(sid=0; sid<cesaMaxSA; sid++)
    {
        mvCesaDebugSA(sid, mode);
    }
}

void    mvCesaDebugStats(void)
{
    int chan;

    mvOsPrintf("\n\t Cesa Statistics\n");

    mvOsPrintf("Opened=%d, Closed=%d\n", 
                cesaStats.openedCount, cesaStats.closedCount);
    mvOsPrintf("Req=%d, maxReq=%d, frags=%d, large=%d\n", 
                cesaStats.reqCount, cesaStats.maxReqCount, 
                cesaStats.fragCount, cesaStats.largeCount);

    mvOsPrintf("\n");
    for(chan=0; chan<MV_CESA_MAX_CHAN; chan++)
    {
        mvOsPrintf("chan #%d: proc=%d, ready=%d, notReady=%d\n", 
                    chan, cesaStats.procCount[chan], 
                    cesaStats.readyCount[chan], cesaStats.notReadyCount[chan]);
    }
}

void    mvCesaDebugStatsClear(void)
{
    memset(&cesaStats, 0, sizeof(cesaStats));
}
