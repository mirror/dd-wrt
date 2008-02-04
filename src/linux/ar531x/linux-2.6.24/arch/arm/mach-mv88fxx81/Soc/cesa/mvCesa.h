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
* mvCesa.h - Header File for Cryptographic Engines and Security Accelerator
*
* DESCRIPTION:
*       This header file contains macros typedefs and function declaration for
*       the Marvell Cryptographic Engines and Security Accelerator.
*
*******************************************************************************/

#ifndef __mvCesa_h__
#define __mvCesa_h__

#include "mvOs.h"
#include "mvLru.h"
#include "mvCesaRegs.h"

#define MV_CESA_AUTH_BLOCK_SIZE         64 /* bytes */

#define MV_CESA_MD5_DIGEST_SIZE         16 /* bytes */
#define MV_CESA_SHA1_DIGEST_SIZE        20 /* bytes */
 
#define MV_CESA_MAX_DIGEST_SIZE         MV_CESA_SHA1_DIGEST_SIZE

#define MV_CESA_DES_KEY_LENGTH          8   /* bytes = 64 bits */
#define MV_CESA_3DES_KEY_LENGTH         24  /* bytes = 192 bits */
#define MV_CESA_AES_128_KEY_LENGTH      16  /* bytes = 128 bits */  
#define MV_CESA_AES_192_KEY_LENGTH      24  /* bytes = 192 bits */ 
#define MV_CESA_AES_256_KEY_LENGTH      32  /* bytes = 256 bits */

#define MV_CESA_MAX_CRYPTO_KEY_LENGTH   MV_CESA_AES_256_KEY_LENGTH

#define MV_CESA_DES_BLOCK_SIZE          8 /* bytes = 64 bits */
#define MV_CESA_3DES_BLOCK_SIZE         8 /* bytes = 64 bits */

#define MV_CESA_AES_BLOCK_SIZE          16 /* bytes = 128 bits */

#define MV_CESA_MAX_IV_LENGTH           MV_CESA_AES_BLOCK_SIZE 

#define MV_CESA_MAX_MAC_KEY_LENGTH      64 /* bytes */


#define MV_CESA_MAX_CHAN                2

/* For systems with 4K bytes SRAM memory */
#define MV_CESA_MAX_BUF_SIZE            1600

/* For systems with 8K bytes SRAM memory */
/*#define MV_CESA_MAX_BUF_SIZE            3648*/

#define MV_CESA_MAX_CACHE_SA            9
#define MV_CESA_MAX_MBUF_FRAGS          20

typedef struct
{
	MV_U8               cryptoKey[MV_CESA_MAX_CRYPTO_KEY_LENGTH];
	MV_U8               macKey[MV_CESA_MAX_MAC_KEY_LENGTH];
	MV_CESA_OPERATION   operation;
	MV_CESA_DIRECTION   direction;
	MV_CESA_CRYPTO_ALG  cryptoAlgorithm;
	MV_CESA_CRYPTO_MODE cryptoMode;
   	MV_U8               cryptoKeyLength;
	MV_CESA_MAC_MODE    macMode;
	MV_U8               macKeyLength; 
	MV_U8               digestSize; 	

} MV_CESA_OPEN_SESSION;

typedef struct 
{
    MV_BUF_INFO *pFrags; 
	MV_U16	    numFrags;
    MV_U16      mbufSize;  

} MV_CESA_MBUF;

typedef struct 
{
    void* 	pReqPrv; /* instead of reqId */
    MV_U32 	retCode;
    MV_U16  sessionId;

} MV_CESA_RESULT;

typedef void    (*MV_CESA_CALLBACK) (MV_CESA_RESULT* pResult);


typedef struct 
{
    void*               pReqPrv;    /* instead of reqId */
    MV_CESA_MBUF*       pSrc;
    MV_CESA_MBUF*       pDst;
    MV_CESA_CALLBACK*   pFuncCB;
    MV_U16              sessionId;
    MV_U16              ivFromUser;		
    MV_U16              ivOffset;
    MV_U16              cryptoOffset;
    MV_U16              cryptoLength;
    MV_U16              digestOffset;
    MV_U16              macOffset;
    MV_U16              macLength;

} MV_CESA_COMMAND;



MV_STATUS   mvCesaInit (int numOfSession, int queueDepth, char* pSramBase);
MV_STATUS   mvCesaFinish (void);
MV_STATUS   mvCesaSessionOpen(MV_CESA_OPEN_SESSION *pSession, short* pSid);
MV_STATUS   mvCesaSessionClose(short sid);
MV_STATUS   mvCesaCryptoIvSet(int chan, MV_U8* pIV, int ivSize);

MV_STATUS   mvCesaAction (MV_CESA_COMMAND* pCmd);

MV_U32      mvCesaChanInProcessGet(void);
MV_STATUS   mvCesaReadyDispatch(MV_U32 chanMask);
MV_STATUS   mvCesaReadyGet(MV_U32 chanMap, MV_CESA_RESULT* pResult);
MV_BOOL     mvCesaIsReady(int chan);

int     	mvCesaMbufOffset(MV_CESA_MBUF* pMbuf, int offset, int* pBufOffset);
MV_STATUS   mvCesaCopyFromMbuf(MV_U8* pDst, MV_CESA_MBUF* pSrcMbuf, 
                               int offset, int size);
MV_STATUS   mvCesaCopyToMbuf(MV_U8* pSrc, MV_CESA_MBUF* pDstMbuf, 
                               int offset, int size);
MV_STATUS   mvCesaMbufCopy(MV_CESA_MBUF* pMbufDst, int dstMbufOffset, 
                           MV_CESA_MBUF* pMbufSrc, int srcMbufOffset, int size);

/********** Debug functions ********/

void        mvCesaDebugMbuf(char* str, MV_CESA_MBUF *pMbuf, int offset, int size);
void        mvCesaDebugSA(short sid, int mode);
void        mvCesaDebugStats(void);
void        mvCesaDebugStatsClear(void);
void        mvCesaDebugRegs(void);
void        mvCesaDebugStatus(void);
void        mvCesaDebugChan(int chan, int mode);
void        mvCesaDebugQueue(int mode);
void        mvCesaDebugCacheIdx(int idx);
void        mvCesaDebugSram(int mode);
void        mvCesaDebugSAD(int mode);


/********  CESA Private definitions ********/

#define CESA_IDMA_CTRL_LOW_VALUE      ICCLR_DST_BURST_LIM_128BYTE   \
                                    | ICCLR_SRC_BURST_LIM_128BYTE   \
                                    | ICCLR_INT_MODE_MASK           \
                                    | ICCLR_BLOCK_MODE              \
                                    | ICCLR_CHAN_ENABLE             \
                                    | ICCLR_DESC_MODE_16M


#define MV_CESA_MAX_IDMA_DESC   (MV_CESA_MAX_MBUF_FRAGS*2 + 5)

typedef enum
{
    MV_CESA_IDLE     = 0,
    MV_CESA_PENDING,
    MV_CESA_PROCESS,
    MV_CESA_READY,

} MV_CESA_STATE;


/* Session database */    

/* Map of Key materials of the session in SRAM. 
 * Each field must be 8 byte aligned 
 */
typedef struct  
{
    MV_U8  cryptoKey[MV_CESA_MAX_CRYPTO_KEY_LENGTH];
    MV_U8  macInnerIV[MV_CESA_MAX_DIGEST_SIZE];
    short  sid;
    MV_U8  reserved[2];
    MV_U8  macOuterIV[MV_CESA_MAX_DIGEST_SIZE];
    MV_U8  reservedOuter[4];

} MV_CESA_CACHE_SA;

typedef struct  
{
    MV_CESA_CACHE_SA    cacheSA;
    MV_U32              config;
    int                 cacheIdx;   
    MV_U8               cryptoKeyLength;
    MV_U8               cryptoIvSize;
    MV_U8               cryptoBlockSize;
    MV_U8               digestSize;
    MV_U8               macKeyLength;
    MV_U8               valid;
    MV_U8               ctrMode;
    MV_U32              count;

} MV_CESA_SA;


/* Request queue */
typedef struct
{
    MV_U8               chanId;
    MV_U8               state;
    MV_U8               fragMode;
    MV_U8               fixOffset;
    MV_U16              cpOffset;
    MV_U16              cpSize;
    MV_CESA_COMMAND*    pCmd;
    MV_CESA_COMMAND*    pOrgCmd;

} MV_CESA_REQ;  


/* Session Cache management */

/* SRAM map */
/* Total SRAM size calculation */
/*  SRAM size = 
 *              MV_CESA_MAX_BUF_SIZE*MV_CESA_MAX_CHAN +
 *              sizeof(MV_CESA_DESC)*MV_CESA_MAX_CHAN +
 *              MV_CESA_MAX_IV_LENGTH*MV_CESA_MAX_CHAN +
 *              MV_CESA_MAX_IV_LENGTH*MV_CESA_MAX_CHAN +
 *              MV_CESA_MAX_DIGEST_SIZE*MV_CESA_MAX_CHAN +
 *              sizeof(MV_CESA_CACHE_SA)*MV_CESA_MAX_CACHE_SA
 *            = 1600*2 + 32*2 + 16*2 + 16*2 + 24*2 + 80*9 = 4096 bytes
 *            = 3648*2 + 32*2 + 16*2 + 16*2 + 24*2 + 80*9 = 8192 bytes
 */
typedef struct
{
    MV_U8               buf[MV_CESA_MAX_CHAN][MV_CESA_MAX_BUF_SIZE];
    MV_CESA_DESC        desc[MV_CESA_MAX_CHAN];
    MV_U8               cryptoIV[MV_CESA_MAX_CHAN][MV_CESA_MAX_IV_LENGTH];
    MV_U8               tempCryptoIV[MV_CESA_MAX_CHAN][MV_CESA_MAX_IV_LENGTH];
    MV_U8               tempDigest[MV_CESA_MAX_CHAN][MV_CESA_MAX_DIGEST_SIZE+4];
    MV_CESA_CACHE_SA    cacheSA[MV_CESA_MAX_CACHE_SA];
} MV_CESA_SRAM_MAP;


/* Channel management */
typedef struct
{
    MV_CESA_STATE   state;
    MV_BUF_INFO     idmaDescBuf;
    MV_DMA_DESC*    pIdmaList;
    MV_ULONG        idmaPhysAddr;
    MV_U16          sramBufOffset;

} MV_CESA_CHAN;


typedef struct
{
    MV_U32  openedCount;
    MV_U32  closedCount;
    MV_U32  fragCount;
    MV_U32  largeCount;
    MV_U32  reqCount;
    MV_U32  maxReqCount;
    MV_U32  procCount[MV_CESA_MAX_CHAN];
    MV_U32  readyCount[MV_CESA_MAX_CHAN];
    MV_U32  notReadyCount[MV_CESA_MAX_CHAN];

} MV_CESA_STATS;

typedef struct
{
    short               sid;
    int                 bufOffset;
    int                 cryptoSize;
    int                 macSize;
    int                 newDigestOffset;
    MV_U8               orgDigest[MV_CESA_MAX_DIGEST_SIZE];
    
} MV_CESA_FRAGS;

/* External variables */
extern MV_BOOL          cesaLargeInProc;

extern MV_CESA_STATS    cesaStats;   
extern MV_CESA_FRAGS    cesaFrags;
extern MV_LRU_CACHE*    pCesaCacheLRU;

extern MV_CESA_SA*       pCesaSAD;
extern MV_U16            cesaMaxSA;

extern MV_CESA_REQ*      pCesaReqFirst;
extern MV_CESA_REQ*      pCesaReqLast;
extern MV_CESA_REQ*      pCesaReqEmpty;
extern MV_CESA_REQ*      pCesaReqProcess;
extern MV_CESA_REQ*      pCesaReqReady;
extern int               cesaQueueDepth;
extern int               cesaReqResources;

extern MV_CESA_SRAM_MAP*  cesaSramVirtPtr;
extern MV_U32           cesaSramPhysAddr;

extern MV_CESA_CHAN     pCesaChan[MV_CESA_MAX_CHAN];

extern MV_U32           cesaChanReadyMap;

/* Additional DEBUG functions */
void        mvCesaDebugCacheSA(MV_CESA_CACHE_SA* pCacheSA, int mode);
void        mvCesaDebugCmd(MV_CESA_COMMAND* pCmd,  int mode);
void        mvCesaDebugDescriptor(MV_CESA_DESC* pDesc);



#endif /* __mvCesa_h__ */ 
