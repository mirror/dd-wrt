/**
 **************************************************************************
 * @file halAe.c
 *
 * @description
 *      This file provides Implementation of Ucode AE Library
 *
 * @par 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 **************************************************************************/ 

#include "halAe_platform.h"
#include "core_io.h"
#include "IxOsal.h"

#include "hal_global.h"
#include "hal_scratch.h"
#include "hal_dram.h"
#include "hal_sram.h"
#include "hal_ae.h"
#include "hal_et_ring.h"
#include "halAeApi.h"
#include "halOS.h"
#include "halMmap.hxx"
#include "halAeDrv.hxx"

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef WORD_BIT
#define WORD_BIT ((unsigned int)(CHAR_BIT * sizeof(unsigned int)))
#endif

#define MASK (WORD_BIT-1)
#define CLR_BIT(wrd, bit) ((wrd) & ~(1 << ((bit) & MASK)))
#define SET_BIT(wrd, bit) ((wrd) | (1 << ((bit) & MASK)))

#define FOUR_CTX                     4                 /* Four context ID */
#define EIGHT_CTX                    8                 /* Eight context ID */
#define INVALID_PC                   0xffffffff        /* Invalid PC */
#define INVALID_AE                   0xffffffff        /* Invalid AE */
#define CSR_DONE_COUNT               500               /* loop count for reading CSR */        

#define INIT_CTX_ARB_VALUE           0x0               /* context arbitor initialization value */   
#define INIT_CTX_ENABLE_VALUE        0x0               /* context enable initialization value */   
#define INIT_PC_VALUE                0x0               /* program counter initialization value */   
#define INIT_WAKEUP_EVENTS_VALUE     XCWE_VOLUNTARY    /* wakeup events initialization value */   
#define INIT_SIG_EVENTS_VALUE        0x1               /* signal events initialization value */   
#define INIT_CCENABLE_VALUE          0x2000            /* context enable initialization value */   
#define WAKEUP_EVENTS_SLEEP          0x10000           /* wakeup enents sleep initialization value */    

#define RST_CSR_SSU_BIT              16                /* reset CSR SSU bit */   
#define RST_CSR_AE_LSB               0                 /* reset CSR AE bit */     
#define SET_RESET_CSR_AE(regVal, aeMask) ((regVal) |= ((aeMask) << RST_CSR_AE_LSB))  /* set reset csr AE bits */   
#define CLR_RESET_CSR_AE(regVal, aeMask) ((regVal) &= ~((aeMask) << RST_CSR_AE_LSB))  /* clear reset csr AE bits */   

#define BAD_REGADDR                  0xffff             /* bad register address */
#define MAX_AE                       0x18               /* using hardware AE addressing */
#define UBUF_SIZE                    (1024 * 8)         /* micro-store buffer size */

#define IGNORE_BKPT_MASK  (~(1<<CE_BREAKPOINT_BITPOS))              /* ignore breakpoint mask */  
#define IGNORE_ECCER_MASK (~(1<<CE_CNTL_STORE_PARITY_ERROR_BITPOS)) /* ignore ecc error mask */  
#define IGNORE_PARER_MASK (~(1<<CE_REG_PAR_ERR_BITPOS))             /* ignore parity error mask */  
#define IGNORE_W1C_MASK (IGNORE_BKPT_MASK & IGNORE_ECCER_MASK & \
                         IGNORE_PARER_MASK)                         /* ignore write-1-clear mask */         

#define SETBIT(value, bit) (setMasks[bit] | (value))                /* set  bit */  
#define CLRBIT(value, bit) (clrMasks[bit] & (value))                /* clear bit */   

#define SPIN_LOCK_MAC(mac)         SPIN_LOCK(macLock[mac])                 /* lock Gige mac resource */              
#define SPIN_UNLOCK_MAC(mac)       SPIN_UNLOCK(macLock[mac])               /* unlock Gige mac resource */              
/* lock ring resource */               
#define SPIN_LOCK_RING(ringNum, level) \
                       SPIN_LOCK_IRQSAVE(ringLock[ringNum], (level))            
/* unlock ring resource */            
#define SPIN_UNLOCK_RING(ringNum, level) \
                       SPIN_UNLOCK_IRQRESTORE(ringLock[ringNum], (level))      
  
#define INSERT_IMMED_GPRA_CONST(inst, const_val) \
    inst = (inst               & 0xFFF00C03FFull) | \
           ((((const_val) << 12) & 0x0FF00000ull) | \
            (((const_val) << 10) & 0x0003FC00ull))                         /* patch value to immed[]' GPRA field */      
#define INSERT_IMMED_GPRB_CONST(inst, const_val) \
    inst = (inst               & 0xFFF00FFF00ull) | \
           ((((const_val) << 12) & 0x0FF00000ull) | \
            (((const_val) <<  0) & 0x000000FFull))                         /* patch value to immed[]' GPRB field */

typedef struct {
    unsigned int vaddr;                    /* page virtual */
    unsigned int paddr;                    /* page physical address */
    unsigned int size;                     /* page size */
    unsigned int loaded;                   /* loaded flag */
} AddrPair_T;

typedef struct {
    unsigned int  numPages;                /* number of pages */
    AddrPair_T   *addrs;                   /* page pairs pointer */
} AePageData_T;

typedef struct PageData_S {
    AePageData_T AePageData[MAX_AE];       /* using hardware AE addressing */
} PageData_T;

typedef struct Hal_UcloCallFrame_S {
    HalAe_UcloCall_T            uclo_callback;      /* callback function pointer */
    void*                       uclo_callback_data; /* callback data */
} Hal_UcloCallFrame_T;

static HalAePageChangeCallback_T new_page_callback = NULL;
static void*                     new_page_callback_data;
static Hal_UcloCallFrame_T ucloCallbacks[MAX_AE];

static SPINLOCK_T ringLock[MAX_EAGLETAIL_RING_ENTRIES];
static unsigned int ringBaseAddr[MAX_EAGLETAIL_RING_ENTRIES];
static unsigned int ringSize[MAX_EAGLETAIL_RING_ENTRIES];
static unsigned int ringHead[MAX_EAGLETAIL_RING_ENTRIES];
static unsigned int ringPutSpace[MAX_EAGLETAIL_RING_ENTRIES];
static unsigned int ringTail[MAX_EAGLETAIL_RING_ENTRIES];
static unsigned int ringGetSpace[MAX_EAGLETAIL_RING_ENTRIES];
static RING_TYPE ringType[MAX_EAGLETAIL_RING_ENTRIES];
static SPINLOCK_T macLock[MAX_GIGE_MAC];
static int isLockInitialized = 0;
static SPINLOCK_T halLock;

static const unsigned int TmpUaddr = 0;

static Hal_SysMemInfo_T SysMemInfo;

EXTERN unsigned int PrdMajType, PrdMinType, PrdMajRev, PrdMinRev;
EXTERN Ae_T AEs[AE_NUMCLUSTR][AE_PERCLUSTR];

DECL(unsigned int AeMask,0x7);
DECL(unsigned int AePerCluster,8);
DECL(unsigned int AeBadMask,0xff00ff00);
DECL(unsigned int AeMaxNum,23);
DECL(unsigned int MaxUstore,MAX_USTORE_PER_SEG);

DECL(unsigned int MaxLmemReg,MAX_LMEM_REG);
DECL(unsigned int UpcMask,0x1fff);

#define TWO_ZEROS {0,0}
DECL(Hal_ChipSpecificState_T css,TWO_ZEROS);

static int disableCtx(unsigned char ae, unsigned int ctxMask);
static int enableCtx(unsigned char ae, unsigned int ctxMask);
static int isAeEnabled(unsigned char ae);
static int putAeCsr(unsigned char ae, unsigned int csr, unsigned int value);
static int getAeCsr(unsigned char ae, unsigned int csr, unsigned int *value);
static int getCtxWakeupEvents(unsigned char ae, unsigned char ctx, 
                              unsigned int *events);
static int putCtxWakeupEvents(unsigned char ae, unsigned int ctxMask, 
                              unsigned int events);
static int getCtxSigEvents(unsigned char ae, unsigned char ctx, 
                           unsigned int *events);
static int putCtxSigEvents(unsigned char ae, unsigned int ctxMask, 
                           unsigned int events);
static int putCtxIndrCsr(unsigned char ae, unsigned int ctxMask, 
                         unsigned int aeCsr, unsigned int csrVal);
static int getRelRdXfer(unsigned char ae, unsigned char ctx, 
                        icp_RegType_T regType,
                        unsigned short regNum, unsigned int *data);
static int putRelRdXfer(unsigned char ae, 
                        unsigned char ctx, 
                        icp_RegType_T regType,
                        unsigned short regNum, 
                        unsigned int val);
static int getRelWrXfers(unsigned char ae, 
              unsigned char ctx, 
              icp_RegType_T regType,
              unsigned short regNum, 
              unsigned int *data, 
              unsigned int count);
void halAe_clrResetStatus(void);
int waitNumSimCycles(unsigned char ae, 
                     unsigned int cycles, 
                     int chkInactive);
int execMicroInst(unsigned char ae, 
                  unsigned char ctx, 
                  uword_T *microInst,
                  unsigned int numInst, 
                  int condCodeOff,
                  unsigned int maxCycles, 
                  unsigned int *endPC);
void halAe_setUofChecksum(unsigned int uofChecksum);
unsigned int halAe_getUofChecksum(void);
int relToAbs(unsigned char ae, 
             unsigned char ctx, 
             unsigned short relRegNum,
             unsigned short *absRegNum);
int halAe_dump(unsigned char ae);

void halAe_ExecuteCycles(unsigned int cycles);
void halAe_WaitUStoreAddrReady(unsigned char ae);
int halAe_GetProdSetting(void);

int getRelDataReg(unsigned char ae, unsigned char ctx, 
                  icp_RegType_T regType,
                  unsigned short regNum, unsigned int *data);
int getRelNNReg(unsigned char ae, 
                unsigned char ctx, 
                icp_RegType_T regType,
                unsigned short regNum,
                unsigned int *data);
int putRelDataReg(unsigned char ae, unsigned char ctx, 
                  icp_RegType_T regType,
                  unsigned short regNum, 
                  unsigned int data);
int putRelWrXfer(unsigned char ae, 
                 unsigned char ctx, 
                 icp_RegType_T regType,
                 unsigned short regNum, 
                 unsigned int data);
int putRelNN(unsigned char ae, 
             unsigned char ctx, 
             unsigned short nnNum,
             unsigned int value);
int getCtxIndrCsr(unsigned char ae, 
              unsigned char ctx, 
              unsigned int aeCsr, 
              unsigned int *csrVal);
int halAe_intrSupported(void);

int halAe_PutLM_Common(unsigned char ae, 
                       unsigned short lmAddr, 
                       unsigned int value);
int halAe_GetLM_Common(unsigned char ae, 
                       unsigned short lmAddr, 
                       unsigned int *value);
int halAe_PutSharedRam_Common(unsigned char ae, 
                              unsigned int addr, 
                              unsigned int value);
int halAe_GetSharedRam_Common(unsigned char ae, 
                              unsigned int addr, 
                              unsigned int *value);
int getRelDataReg_Common(unsigned char ae, unsigned char ctx, 
                         icp_RegType_T regType,
                         unsigned short regNum, unsigned int *data);
int getRelNNReg_Common(unsigned char ae, 
                       unsigned char ctx, 
                       icp_RegType_T regType,
                       unsigned short regNum,
                       unsigned int *data);
int putRelDataReg_Common(unsigned char ae, unsigned char ctx, 
                         icp_RegType_T regType,
                         unsigned short regNum, 
                         unsigned int data);
int putRelWrXfer_Common(unsigned char ae, 
                        unsigned char ctx, 
                        icp_RegType_T regType,
                        unsigned short regNum, 
                        unsigned int data);
int putRelNN_Common(unsigned char ae, 
                    unsigned char ctx, 
                    unsigned short nnNum,
                    unsigned int value);
int halAe_GetRelDataReg_Common(unsigned char ae, 
                               unsigned char ctx, 
                               icp_RegType_T regType,
                               unsigned short regNum, 
                               unsigned int *regData);
int halAe_PutRelDataReg_Common(unsigned char ae, 
                               unsigned char ctx, 
                               icp_RegType_T regType,
                               unsigned short regNum, 
                               unsigned int regData);
int halAe_GetAbsDataReg_Common(unsigned char ae, 
                               icp_RegType_T regType,
                               unsigned short absRegNum, 
                               unsigned int *regData);
int halAe_PutAbsDataReg_Common(unsigned char ae, 
                               icp_RegType_T regType,
                               unsigned short absRegNum, 
                               unsigned int regData);
int getCtxIndrCsr_Common(unsigned char ae, 
                         unsigned char ctx, 
                         unsigned int aeCsr, 
                         unsigned int *csrVal);

static unsigned int lock_level;

/*-----------------------------------------------------------------------------
   Function:     bitParity64
   Description: Determine parity of the number of bits.
   Returns:    0 if even; 1 if odd.
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static unsigned int 
bitParity64(uint64 word)
{
   word ^= word >> 1;
   word ^= word >> 2;
   word ^= word >> 4;
   word ^= word >> 8;
   word ^= word >> 16;
   word ^= word >> 32;
   return ((unsigned int )(word & 1));
}

/*-----------------------------------------------------------------------------
   Function:     numBitsSet
   Description: Determine the number of bits that are set in a long-word.
   Returns:
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
numBitsSet(unsigned int word)
{
   int nn = 0;

   if(word)
   {
      do
      {
         nn++;
      } while ((word &= word - 1));
   }
   return (nn);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_SetLiveCtx
   Description: Set a mask of the contexts that are loaded.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_SetLiveCtx(unsigned char ae, 
                 unsigned int ctxMask)
{
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    SET_LIVECTXMASK(ae, ctxMask);
    SPIN_UNLOCK_AE(ae);
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetLiveCtx
   Description: Get a mask of the contexts that are loaded.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetLiveCtx(unsigned char ae, 
                 unsigned int *ctxMask)
{
    VERIFY_ARGPTR(ctxMask);
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    *ctxMask = GET_LIVECTXMASK(ae);
    SPIN_UNLOCK_AE(ae);
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_setUofChecksum
   Description: Set uof file checksum
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
halAe_setUofChecksum(unsigned int uofChecksum)
{
    UOFCHECKSUM = uofChecksum;
    return;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_getUofChecksum
   Description: Get uof file checksum
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
halAe_getUofChecksum(void)
{
    return (UOFCHECKSUM);
}

/*-----------------------------------------------------------------------------
   Function:    isAeActive
   Description: Determine whether specified accelEngine is enabled, or running
   Returns:     1 if active or 0 for inactive
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static unsigned int 
isAeActive(unsigned char ae)
{
    unsigned int csr;
    
    if(isAeEnabled(ae) != HALAE_DISABLED) 
    {
       return (1); 
    }   
    getAeCsr(ae, ACTIVE_CTX_STATUS, &csr);
    return ((csr & (1 << ACS_ABO_BITPOS)));;
}

/*-----------------------------------------------------------------------------
   Function:    waitNumSimCycles
   Description: Wait for specified number of cycles
   Returns:     1 if actually waited or 0 for not
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
/* returns 1 if actually waited for specified number of cycles */
int 
waitNumSimCycles(unsigned char ae, 
                 unsigned int cycles, 
                 int chkInactive)
{
    int PreCnt = 0;
    int CurCnt = 0;
    int ElapsedCycles = 0;
    unsigned int csr;

    getAeCsr(ae, PROFILE_COUNT, (unsigned int *)&CurCnt);
    PreCnt = CurCnt;
    do {
        halAe_ExecuteCycles(cycles);        

        if (chkInactive) 
        {
            getAeCsr(ae, ACTIVE_CTX_STATUS, &csr);
            if ((csr & (1 << ACS_ABO_BITPOS)) == 0) 
            {
                return 0;
            }    
        } 
        getAeCsr(ae, PROFILE_COUNT, (unsigned int *)&CurCnt); 
    
        /* With this logic, there is still a very slight risk     */
        /* that due to very slow execution or OS-related delays   */
        /* on the IA side (greater than about 0x10000 AE cycles   */
        /* elapsed), successive reads of the PROFILE_COUNT will   */
        /* fail the test to terminate the loop, but should.       */
        /* The logic to handle the slight risk could be:          */
        /* If the number of AE cycles between successive reads    */
        /* of PROFILE_COUNT is just over 65535, then the loop     */
        /* should immediately terminate.                          */
        /* Above logic could not work under whole chip simulation */
        /* since the clock frequency domains are not the same.    */
        /* And considering the low probability, above logic will  */
        /* not be added.                                          */
    
        /* Calculate the elapsed AE cycles */
        ElapsedCycles = CurCnt - PreCnt;
        if(ElapsedCycles < 0) 
        {
            ElapsedCycles += 0x10000;
        }    
    } while (((int)cycles)>ElapsedCycles);

    /* Before returning a timeout, check active context status once again   */
    /* to make sure the context did not terminate between the last read of  */
    /* ACTIVE_CTX_STATUS and the read of PROFILE_COUNT. A large OS-related  */
    /* delay between the last read of ACTIVE_CTX_STATUS and PROFILE_COUNT   */
    /* will result in a non-zero (timeout) return when the context actually */
    /* terminated on its own.                                               */
    if (chkInactive) 
    {
        getAeCsr(ae, ACTIVE_CTX_STATUS, &csr);
        if ((csr & (1 << ACS_ABO_BITPOS)) == 0) 
        {
            return 0;
        }    
    }

    return 1;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetSharedUstoreNeigh
   Description: Get the neighbor of a shareable-ustore pair.
   Returns:     The AE-pair number 
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetSharedUstoreNeigh(unsigned char ae, 
                           unsigned char *aeNeigh)
{
    HALAE_VERIFY_LIB();    
    VERIFY_AE(ae);

    if(!aeNeigh) 
    {
       return (HALAE_BADARG);
    }    
    if(ae & 0x1) 
    {
       *aeNeigh = ae-1;
    }   
    else 
    {
       *aeNeigh = ae+1;
    }   
    
    return (HALAE_SUCCESS);    
}


/*-----------------------------------------------------------------------------
   Function:    halAe_GetAeState
   Description: Determine if the specified AE is valid or whether it's 
                initialized in or out of reset.
   Returns:     HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetAeState(unsigned char ae)
{
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    return (AE(ae).state);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_VerifyAe
   Description: Validate the AE number.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_VerifyAe(unsigned char ae)
{
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_VerifyAeMask
   Description: Validate the AE number.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int halAe_VerifyAeMask(unsigned int aeMask)
{
    HALAE_VERIFY_LIB();
    if(AeBadMask & aeMask) 
    {
       return (HALAE_BADARG);
    }   
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    putAeCtxMode
   Description: Set the AE context mode (four or eight ctx).
   Returns:     HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putAeCtxMode(unsigned char ae, 
             unsigned char mode)
{
    unsigned int csr, newCsr;

    csr = IGNORE_W1C_MASK & GET_AE_CSR(ae, CTX_ENABLES);
    if(mode == FOUR_CTX) 
    {
        newCsr = SET_BIT(csr, CE_INUSE_CONTEXTS_BITPOS);
    }    
    else 
    {
        newCsr = CLR_BIT(csr, CE_INUSE_CONTEXTS_BITPOS);
    }    
    if(newCsr != csr) 
    {
       SET_AE_CSR(ae, CTX_ENABLES, newCsr);
    }    
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAeCtxMode
   Description: Set the AE context mode (four or eight ctx).
   Returns:     HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAeCtxMode(unsigned char ae, 
                   unsigned char mode)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = putAeCtxMode(ae, mode);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAeNnMode
   Description: Set the AE next-neighbor mode. The AE should be stopped before
                calling this function. 
   Returns:     HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAeNnMode(unsigned char ae, 
                  unsigned char mode)
{
    unsigned int csr, newCsr;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    SPIN_LOCK_AE(ae);
    csr = IGNORE_W1C_MASK & GET_AE_CSR(ae, CTX_ENABLES);
    if(mode) 
    {
       newCsr = SET_BIT(csr, CE_NN_MODE_BITPOS);
    }   
    else 
    {
       newCsr = CLR_BIT(csr, CE_NN_MODE_BITPOS);
    }   

    if(newCsr != csr) 
    {
        SET_AE_CSR_X(ae, CTX_ENABLES, newCsr);
    }    
    SPIN_UNLOCK_AE(ae);
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    putAeLmMode
   Description: Set the whether localMem0/localMem1 access is relative or global.
                It's unsafe to call this function while the AE is enabled.
                lmType: ICP_LMEM0, ICP_LMEM1
                mode: 0=relative, 1=global
   Returns:     HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putAeLmMode(unsigned char ae, 
            icp_RegType_T lmType, 
            unsigned char mode)
{
    unsigned int csr, newCsr;

    csr = IGNORE_W1C_MASK & GET_AE_CSR(ae, CTX_ENABLES);
    switch(lmType)
    {
    case ICP_LMEM0:
        if(mode) 
        {
            newCsr = SET_BIT(csr, CE_LMADDR_0_GLOBAL_BITPOS);
        }    
        else 
        {
            newCsr = CLR_BIT(csr, CE_LMADDR_0_GLOBAL_BITPOS);
        }    
        break;   
    case ICP_LMEM1:
        if(mode) 
        {
            newCsr = SET_BIT(csr, CE_LMADDR_1_GLOBAL_BITPOS);
        }    
        else 
        {
            newCsr = CLR_BIT(csr, CE_LMADDR_1_GLOBAL_BITPOS);
        }    
        break;
    default: return (HALAE_BADARG);
    }

    if(newCsr != csr) 
    {
        SET_AE_CSR(ae, CTX_ENABLES, newCsr);
    }    
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAeLmMode
   Description: Set the whether localMem0/localMem1 access is relative or 
                global.It's unsafe to call this function while the AE is 
                enabled.
                lmType: ICP_LMEM0, ICP_LMEM1
                mode: 0=relative, 1=global
   Returns:     HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAeLmMode(unsigned char ae, 
                  icp_RegType_T lmType, 
                  unsigned char mode)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    SPIN_LOCK_AE(ae);
    stat = putAeLmMode(ae, lmType, mode);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putAeSharedCsMode
   Description: Set the AE shared ustore mode on/off.
   Returns:     HALAE_SUCCESS
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putAeSharedCsMode(unsigned char ae, 
                  unsigned char mode)
{
    unsigned int csrVal, newCsrVal;

    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    if(mode == 1) 
    {
        newCsrVal = SET_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    }    
    else 
    {
        newCsrVal = CLR_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    }    

    if(newCsrVal != csrVal) 
    {
        SET_AE_CSR(ae, AE_MISC_CONTROL, newCsrVal);
    }
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAeSharedCsMode
   Description: Set the AE and its Neighbor's shared ustore mode on/off.
   Returns:     HALAE_SUCCESS, HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAeSharedCsMode(unsigned char ae, 
                        unsigned char mode)
{
    int stat = HALAE_SUCCESS;
    unsigned char aeNeigh;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    halAe_GetSharedUstoreNeigh(ae, &aeNeigh);

    SPIN_LOCK_AE(ae);
    SPIN_LOCK_AE(aeNeigh);
    stat = putAeSharedCsMode(ae, mode);
    stat = putAeSharedCsMode(aeNeigh, mode);
    SPIN_UNLOCK_AE(aeNeigh);
    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getAeSharedCsMode
   Description: Get the AE' shared ustore mode on/off.
   Returns:     HALAE_SUCCESS
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getAeSharedCsMode(unsigned char ae, 
                  unsigned char *mode)
{
    unsigned int csrVal;

    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    if(csrVal & (0x1 << MMC_SHARE_CS_BITPOS)) 
    {
       *mode = 1;
    }    
    else 
    {
       *mode = 0;
    }   

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetAeSharedCsMode
   Description: Get the AE's shared ustore mode on/off.
   Returns:     HALAE_SUCCESS, HALAE_UNINIT, HALAE_BADARG, HALAE_BADLIB
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetAeSharedCsMode(unsigned char ae, 
                        unsigned char *mode)
{
    int stat = HALAE_SUCCESS;
    
    VERIFY_ARGPTR(mode);
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    SPIN_LOCK_AE(ae);
    stat = getAeSharedCsMode(ae, mode);
    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getReg10bitAddr
   Description: Get the 10-bit address of the specified register number.
   Returns:     If the type ICP_LMEM0, or ICP_LMEM1 then regNum is ignored.
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
static unsigned short 
getReg10bitAddr(unsigned int type, 
                unsigned short regNum)
{
   unsigned short regAddr;
   switch(type)
   {
    case ICP_GPA_ABS:
    case ICP_GPB_ABS:
        regAddr = (regNum & 0x7f) | 0x80; 
        break;
    case ICP_GPA_REL:
    case ICP_GPB_REL:
        regAddr = regNum & 0x1f;     
        break;
    case ICP_SR_RD_REL:
    case ICP_SR_WR_REL:
    case ICP_SR_REL:
        regAddr = 0x180 | (regNum & 0x1f); 
        break;
    case ICP_SR_INDX: 
        regAddr = 0x140 | ((regNum & 0x3) << 1); 
        break;
    case ICP_DR_RD_REL:
    case ICP_DR_WR_REL:
    case ICP_DR_REL:
        regAddr = 0x1c0 | (regNum & 0x1f); 
        break;
    case ICP_DR_INDX: 
        regAddr = 0x100 | ((regNum & 0x3) << 1); 
        break;
    case ICP_NEIGH_INDX: 
        regAddr = 0x241 | ((regNum & 0x3) << 1); 
        break;
    case ICP_NEIGH_REL: 
        regAddr = 0x280 | (regNum & 0x1f); 
        break;
    case ICP_LMEM0: 
        regAddr = 0x200; 
        break;
    case ICP_LMEM1: 
        regAddr = 0x220; 
        break;
    case ICP_NO_DEST: 
        regAddr = 0x300 | (regNum & 0xff); 
        break;
    default: 
        regAddr = BAD_REGADDR; 
        break;
   }
   return (regAddr);
}

/*-----------------------------------------------------------------------------
   Function:    getAeCsr
   Description: Read a long-word value from the specified AE CSR.
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getAeCsr(unsigned char ae, 
         unsigned int csr, 
         unsigned int *value)
{
    unsigned int iterations=CSR_DONE_COUNT;

    do{
        *value = GET_AE_CSR(ae, csr);
        if(!(GET_AE_CSR(ae, LOCAL_CSR_STATUS) & LCS_STATUS)) 
        {
             return (HALAE_SUCCESS);
        }     
    }while(iterations--);
    return (HALAE_FAIL);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetAeCsr
   Description: Read a long-word value from the specified AE CSR.
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetAeCsr(unsigned char ae, 
               unsigned int csr, 
               unsigned int *value)
{
    int stat = HALAE_SUCCESS;
    
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    VERIFY_ARGPTR(value);
    SPIN_LOCK_AE(ae);
    stat = getAeCsr(ae, csr, value);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

   
/*-----------------------------------------------------------------------------
   Function:    putAeCsr
   Description: Write a long-word value to the specified AE CSR.
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putAeCsr(unsigned char ae, 
         unsigned int csr, 
         unsigned int value)
{
    unsigned int iterations=CSR_DONE_COUNT;

    do{
        SET_AE_CSR(ae, csr, value);
        if(!(GET_AE_CSR(ae, LOCAL_CSR_STATUS) & LCS_STATUS)) 
        {
             return (HALAE_SUCCESS);
        }     
    }while(iterations--);

    return (HALAE_FAIL);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAeCsr
   Description: Write a long-word value to the specified AE CSR.
                It's unsafe to call this function to write to the 
                CSR_CTX_POINTER CSR while the AE is enabled.
   Returns:     HALAE_AEACTIVE, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAeCsr(unsigned char ae, 
               unsigned int csr, 
               unsigned int value)
{
    int stat = HALAE_SUCCESS;
    
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    if((csr == CSR_CTX_POINTER) && isAeActive(ae)) 
    {
        stat = HALAE_AEACTIVE;
    }    
    else 
    {
        stat = putAeCsr(ae, csr, value);
    }    
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetPciCsr
   Description: get PCI CSR
   Returns:     HALAE_SUCCESS or error number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetPciCsr(unsigned char ae, 
                Hal_CSR_T type, 
                unsigned int offset, 
                unsigned int numBytes, 
                unsigned int *data)
{
    unsigned int gprA0, gprB0, sr0;
    unsigned int base, shift, mask;    
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        /*        
        .reg base offset
        .areg base     0
        .breg offset   0
        .reg $value
        .addr $value 0
        .sig sigRead        
        */
        0x0F0400C0000ull, /*0. immed_w0[l0000!base, 0] */
        0x0F4400C0000ull, /*1. immed_w1[l0000!base, 0] */
        0x0F040000300ull, /*2. immed_w0[l0000!offset, 0] */
        0x0F440000300ull, /*3. immed_w1[l0000!offset, 0] */
        0x03E10040300ull, /*4. cap[read, $l0000!value, l0000!base, l0000!offset, 1], sig_done[l0000!sigRead] */
        0x0D801404213ull, /*5. nop_loop#:br_!signal[l0000!sigRead, nop_loop#] */
        0x0F0000C0300ull, /*6. nop */
        0x0E000010000ull, /*7. ctx_arb[kill], any */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    if(data == NULL)
    {
    	return (HALAE_BADARG);
    }    

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    switch(type)
    {
    case HAL_CSR_PCI_HEAD_GBE0:
        base = 0x14000;
        break;
    case HAL_CSR_PCI_HEAD_GBE1:
        base = 0x14100;
        break;
    case HAL_CSR_PCI_HEAD_GBE2:
        base = 0x14200;    
        break;
    default: 
        SPIN_UNLOCK_AE(ae);
        return (HALAE_FAIL);
    }

    switch(numBytes)
    {
    case 1:
        mask = 0xFF;
        break;
    case 2:
        mask = 0xFFFF;
        break;
    case 3:
        mask = 0xFFFFFF;
        break;
    case 4:
        mask = 0xFFFFFFFF;
        break;
    default: 
        SPIN_UNLOCK_AE(ae);
        return (HALAE_BADARG);
    }

    /* fixup instruction for base address and offset */
    INSERT_IMMED_GPRA_CONST(microInst[0], (base >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[1], (base >> 16));

    INSERT_IMMED_GPRB_CONST(microInst[2], (offset >>  0));
    INSERT_IMMED_GPRB_CONST(microInst[3], (offset >> 16));

    /* get and save the value of gpr and xfer_out reg */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, &gprB0);    
    getRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, &sr0);

    /* execute mac_rd instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                             condCodeOff, 600, NULL);

    /* get the read value */
    getRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, data);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, 0, gprA0);
    putRelDataReg(ae, ctx, ICP_GPB_REL, 0, gprB0);        
    putRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, sr0);
    
    shift = offset % 4;
    *data = (*data) >> (shift * 8);               
    *data = (*data) & mask;    

    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutPciCsr
   Description: put PCI CSR
   Returns:     HALAE_SUCCESS or error number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutPciCsr(unsigned char ae, 
                Hal_CSR_T type, 
                unsigned int offset, 
                unsigned int numBytes, 
                unsigned int data)
{
    unsigned int gprA0, gprB0, gprB1, sr0;
    unsigned int base, shift, mask;
    unsigned int orig_data;
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
    /*
        .reg base offset data
        .areg base     0
        .breg offset   0
        .breg data     1
        .reg $value
        .addr $value 0
        .sig sigWrite    
    */
        0x0F0400C0000ull, /*0. immed_w0[l0000!base, 0] */
        0x0F4400C0000ull, /*1. immed_w1[l0000!base, 0] */
        0x0F040000300ull, /*2. immed_w0[l0000!offset, 0] */
        0x0F440000300ull, /*3. immed_w1[l0000!offset, 0] */
        0x0F040000700ull, /*4. immed_w0[l0000!data, 0] */
        0x0F440000700ull, /*5. immed_w1[l0000!data, 0] */
        0x0A018000400ull, /*6. alu[$l0000!value, --, b, l0000!data] */
        0x03F10040300ull, /*7. cap[write, $l0000!value, l0000!base, l0000!offset, 1], sig_done[l0000!sigWrite] */
        0x0D802004213ull, /*8. nop_loop#:	br_!signal[l0000!sigWrite, nop_loop#] */
        0x0F0000C0300ull, /*9. nop */
        0x0E000010000ull, /*10. ctx_arb[kill], any */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    stat = halAe_GetPciCsr(ae, type, offset, 4, &orig_data);
    if(stat != HALAE_SUCCESS)
    {
       return (stat); 
    }    

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    switch(type)
    {
    case HAL_CSR_PCI_HEAD_GBE0:
        base = 0x14000;
        break;
    case HAL_CSR_PCI_HEAD_GBE1:
        base = 0x14100;
        break;
    case HAL_CSR_PCI_HEAD_GBE2:
        base = 0x14200;    
        break;
    default: 
        SPIN_UNLOCK_AE(ae);        
        return (HALAE_FAIL);
    }

    switch(numBytes)
    {
    case 1:
        mask = 0xFF;
        break;
    case 2:
        mask = 0xFFFF;
        break;
    case 3:
        mask = 0xFFFFFF;
        break;
    case 4:
        mask = 0xFFFFFFFF;
        break;
    default: 
        SPIN_UNLOCK_AE(ae);
        return (HALAE_BADARG);
    }
    
    data = data & mask;
    shift = offset % 4;
    data = data << (shift * 8); 
    orig_data = orig_data & (!(mask << (shift * 8)));
    data = data | orig_data;              
        
    /* fixup instruction for base address and offset */
    INSERT_IMMED_GPRA_CONST(microInst[0], (base >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[1], (base >> 16));

    INSERT_IMMED_GPRB_CONST(microInst[2], (offset >>  0));
    INSERT_IMMED_GPRB_CONST(microInst[3], (offset >> 16));

    INSERT_IMMED_GPRB_CONST(microInst[4], (data >>  0));
    INSERT_IMMED_GPRB_CONST(microInst[5], (data >> 16));

    /* get and save the value of gpr and xfer_out reg */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, &gprB0);    
    getRelDataReg(ae, ctx, ICP_GPB_REL, 1, &gprB1);    
    getRelWrXfers(ae, ctx, ICP_SR_WR_REL, 0, &sr0, 1);

    /* execute mac_wr instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                             condCodeOff, 600, NULL);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, 0, gprA0);
    putRelDataReg(ae, ctx, ICP_GPB_REL, 0, gprB0);        
    putRelDataReg(ae, ctx, ICP_GPB_REL, 1, gprB1);    
    putRelWrXfer(ae, ctx, ICP_SR_WR_REL, 0, sr0);

    SPIN_UNLOCK_AE(ae);
    
    return (stat);    
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutGigeCsr
   Description: Write a long-word value to the Gige CSR
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutGigeCsr(unsigned char ae, 
                 unsigned int mac, 
                 unsigned int csr, 
                 unsigned int value)
{
    unsigned int gprA0, gprA1, gprB0, gprB1, sr0;
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        /*
        .reg chan addr offset data
        .areg chan 0
        .areg addr 1
        .breg offset 0
        .breg data 1
        .reg $value
        .addr $value 0
        .sig sigWrite
        */
        0x0F0400C0000ull, /*0. immed_w0[l0000!chan, 0x0] */
        0x0F4400C0000ull, /*1. immed_w1[l0000!chan, 0x0] */
        0x0F040000300ull, /*2. immed_w0[l0000!offset, 0x0] */
        0x0F440000300ull, /*3. immed_w1[l0000!offset, 0x0] */
        0x0F040000700ull, /*4. immed_w0[l0000!data, 0x0] */
        0x0F440000700ull, /*5. immed_w1[l0000!data, 0x0] */
        0x08A40180200ull, /*6. alu_shf[l0000!addr, l0000!offset, or, l0000!chan, <<28] */
        0x0A018000400ull, /*7. alu[$l0000!value, --, b, l0000!data] */
        0x03D10008301ull, /*8. mac[csr_wr, $l0000!value, l0000!addr, 0], sig_done[l0000!sigWrite] */
        0x0D802504213ull, /*9. nop_loop#:br_!signal[l0000!sigWrite, nop_loop#], defer[1] */
        0x0F0000C0300ull, /*10. nop */
        0x0F0000C0300ull, /*11. nop */
        0x0E000010000ull, /*12. ctx_arb[kill], any */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    if(mac >= MAX_GIGE_MAC) 
    {
       return (HALAE_BADARG);
    }    
    /* should give dword address and not bigger than Gige CSR memory space */
    if((csr & 0x3) || (csr > (KILO_128 - 4))) 
    {
        return (HALAE_BADARG);    
    }

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    SPIN_LOCK_MAC(mac);

    /* fixup instruction for channel and offset */
    INSERT_IMMED_GPRA_CONST(microInst[0], (mac >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[1], (mac >> 16));

    INSERT_IMMED_GPRB_CONST(microInst[2], (csr >>  0));
    INSERT_IMMED_GPRB_CONST(microInst[3], (csr >> 16));

    INSERT_IMMED_GPRB_CONST(microInst[4], (value >>  0));
    INSERT_IMMED_GPRB_CONST(microInst[5], (value >> 16));

    /* get and save the value of gpr and xfer_out reg */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelDataReg(ae, ctx, ICP_GPA_REL, 1, &gprA1);    
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, &gprB0);    
    getRelDataReg(ae, ctx, ICP_GPB_REL, 1, &gprB1);    
    getRelWrXfers(ae, ctx, ICP_SR_WR_REL, 0, &sr0, 1);

    /* execute mac_wr instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                             condCodeOff, 600, NULL);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, 0, gprA0);
    putRelDataReg(ae, ctx, ICP_GPA_REL, 1, gprA1);
    putRelDataReg(ae, ctx, ICP_GPB_REL, 0, gprB0);        
    putRelDataReg(ae, ctx, ICP_GPB_REL, 1, gprB1);    
    putRelWrXfer(ae, ctx, ICP_SR_WR_REL, 0, sr0);

    SPIN_UNLOCK_MAC(mac);
    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetGigeCsr
   Description: Get a long-word value from Gige CSR 
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetGigeCsr(unsigned char ae, 
                 unsigned int mac, 
                 unsigned int csr, 
                 unsigned int *value)
{
    unsigned int gprA0, gprA1, gprB0, sr0;
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        /*
        .reg chan addr offset
        .areg chan 0
        .areg addr 1
        .breg offset 0
        .reg $value
        .addr $value 0
        .sig sigRead
        */
        0x0F0400C0000ull, /*0.  immed_w0[l0000!chan, 0x0] */
        0x0F4400C0000ull, /*1.  immed_w1[l0000!chan, 0x0] */
        0x0F040000300ull, /*2.  immed_w0[l0000!offset, 0x0] */
        0x0F440000300ull, /*3.  immed_w1[l0000!offset, 0x0] */
        0x08A40180200ull, /*4.  alu_shf[l0000!addr, l0000!offset, or, l0000!chan, <<28] */
        0x03C10008301ull, /*5.  mac[csr_rd, $l0000!value, l0000!addr, 0], sig_done[l0000!sigRead] */
        0x0D801904213ull, /*6.  nop_loop#: br_!signal[l0000!sigRead, nop_loop#], defer[1] */
        0x0F0000C0300ull, /*7.  nop */
        0x0E000010000ull, /*8.  ctx_arb[kill], any */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    if(mac >= MAX_GIGE_MAC) 
    {
       return (HALAE_BADARG);
    }    
    /* should give dword address and not bigger than Gige CSR memory space */
    if((csr & 0x3) || (csr > (KILO_128 - 4))) 
    {
        return (HALAE_BADARG);    
    }

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    SPIN_LOCK_MAC(mac);

    /* fixup instruction for channel and offset */
    INSERT_IMMED_GPRA_CONST(microInst[0], (mac >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[1], (mac >> 16));

    INSERT_IMMED_GPRB_CONST(microInst[2], (csr >>  0));
    INSERT_IMMED_GPRB_CONST(microInst[3], (csr >> 16));

    /* get and save the value of gpr and xfer_out reg */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelDataReg(ae, ctx, ICP_GPA_REL, 1, &gprA1);    
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, &gprB0);    
    getRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, &sr0);

    /* execute mac_rd instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                             condCodeOff, 600, NULL);

    /* get the read value */
    getRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, value);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, 0, gprA0);
    putRelDataReg(ae, ctx, ICP_GPA_REL, 1, gprA1);
    putRelDataReg(ae, ctx, ICP_GPB_REL, 0, gprB0);        
    putRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, sr0);

    SPIN_UNLOCK_MAC(mac);
    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getProdId
   Description: Gets the silicon type and revision.
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getProdId(unsigned int *majType, 
          unsigned int *minType,
          unsigned int *majRev, 
          unsigned int *minRev)
{
    unsigned int prodId;

    prodId = SysMemInfo.prodId;

    *majType = (prodId & PID_MAJOR_PROD_TYPE) >> PID_MAJOR_PROD_TYPE_BITPOS;
    *minType = (prodId & PID_MINOR_PROD_TYPE) >> PID_MINOR_PROD_TYPE_BITPOS;
    *majRev = (prodId & PID_MAJOR_REV) >> PID_MAJOR_REV_BITPOS;
    *minRev = (prodId & PID_MINOR_REV) >> PID_MINOR_REV_BITPOS;
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_Reset
   Description: Reset the specified AccelEngines and conditionally clear the
                registers to their power-up state.
   Returns:        
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
halAe_Reset(unsigned int aeMask, 
            int clrReg)
{
    unsigned int aeResetCsr;
    unsigned char ae;

#ifdef _HALAE_PRINT

    PRINTF("halAe_Reset: aeMask=0x%x\n", aeMask);

#endif /* _HALAE_PRINT */

    if(!HALAE_LIBINIT) 
    {
        return;
    }    

    SPIN_LOCK(HAL_GLOBALLOCK);

    aeMask &= ~AeBadMask;

    aeResetCsr = GET_GLB_CSR(ICP_RESET);     /* reset the selected AEs */
    SET_RESET_CSR_AE(aeResetCsr, aeMask);    /* set the appropriate AE bits */

    /* Set the SSU reset bit at the same time as the AE's */
    aeResetCsr |= (1 << RST_CSR_SSU_BIT);

    /* write to the reset csr */
    SET_GLB_CSR(ICP_RESET, aeResetCsr);        

    for(ae = 0; (ae <= AeMaxNum) && (ae < sizeof(setMasks)/sizeof(unsigned int)); ae++)
    {
        if(AeBadMask & setMasks[ae]) 
        {
           continue;
        }   
        if((aeMask & setMasks[ae])) 
        {
            SET_RST(ae);                          /* set the reset indicator */
        }
    }

    SPIN_UNLOCK(HAL_GLOBALLOCK);
    return;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_ResetTimestamp
   Description: Stop the timestamp clock, and zero the timestamps of all
                AEs that are specified by aeMask, and then restart the 
                timestamp clock. 
   Returns:     HALAE_BADLIB, HALAE_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_ResetTimestamp(unsigned int aeMask)
{
    unsigned int miscCtl, zero=0;
    unsigned char ae;

    HALAE_VERIFY_LIB();

    SPIN_LOCK(HAL_GLOBALLOCK);

    /* stop the timestamp timers */
    miscCtl = GET_GLB_CSR(MISC_CONTROL);
    if(miscCtl & MC_TIMESTAMP_ENABLE) 
    {
        SET_GLB_CSR(MISC_CONTROL, miscCtl & (~MC_TIMESTAMP_ENABLE));
    }    

    for(ae = 0; (ae <= AeMaxNum) && (ae < sizeof(setMasks)/sizeof(unsigned int)); ae++)
    {
        if(AeBadMask & setMasks[ae]) 
        {
           continue;
        }   
        if((aeMask & setMasks[ae]))
        {
            putAeCsr(ae, TIMESTAMP_LOW, zero);
            putAeCsr(ae, TIMESTAMP_HIGH, zero);
        }
    }

    /* start timestamp timers */
    SET_GLB_CSR(MISC_CONTROL, miscCtl | MC_TIMESTAMP_ENABLE);
    SPIN_UNLOCK(HAL_GLOBALLOCK);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_clrResetStatus
   Description: Force reset status to HALAE_CLR_RST
   Returns:        
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
halAe_clrResetStatus(void)
{
    unsigned char ae;    
    
    for(ae = 0; (ae <= AeMaxNum) && (ae < sizeof(setMasks)/sizeof(unsigned int)); ae++)
    {
        if(AeBadMask & setMasks[ae]) 
        {
           continue;
        }   
        CLR_RST(ae);                           /* clear the reset indicator */         
    }    
    
    return;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_ClrReset
   Description: Take the specified AccelEngines out of reset and init AEs
   Returns:        
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
halAe_ClrReset(unsigned int aeMask)
{
    unsigned int aeResetCsr;
    unsigned char ae;

    if(!HALAE_LIBINIT) 
    {
        return;
    }    

    SPIN_LOCK(HAL_GLOBALLOCK);

    aeMask &= ~AeBadMask;

    aeResetCsr = GET_GLB_CSR(ICP_RESET);
    CLR_RESET_CSR_AE(aeResetCsr, aeMask);

    /* Clear the SSU reset bit at the same time as the AE's */
    aeResetCsr &= ~(1 << RST_CSR_SSU_BIT);

    /* write to the reset csr */
    SET_GLB_CSR(ICP_RESET, aeResetCsr);

    /* Set undefined power-up/reset states to reasonable default values...
       just to make sure we're starting from a known point */
    for(ae = 0; (ae <= AeMaxNum) && (ae < sizeof(setMasks)/sizeof(unsigned int)); ae++)
    {
        if(AeBadMask & setMasks[ae]) 
        {
           continue;
        }   
        if((aeMask & setMasks[ae]))
        {
            /* clear the reset indicator */            
            CLR_RST(ae);                 
            /* init the ctx_enable */
            putAeCsr(ae, CTX_ENABLES, INIT_CTX_ENABLE_VALUE);
            /* initialize the PCs */
            putCtxIndrCsr(ae, AE_ALL_CTX, CTX_STS_INDIRECT, 
                          UpcMask & INIT_PC_VALUE);
            /* init the ctx_arb */
            putAeCsr(ae, CTX_ARB_CNTL, INIT_CTX_ARB_VALUE);
            /* enable cc */  
            putAeCsr(ae, CC_ENABLE, INIT_CCENABLE_VALUE);
            putCtxWakeupEvents(ae, AE_ALL_CTX, INIT_WAKEUP_EVENTS_VALUE);
            putCtxSigEvents(ae, AE_ALL_CTX, INIT_SIG_EVENTS_VALUE);
        }
    }

    SPIN_UNLOCK(HAL_GLOBALLOCK);

    halAe_ResetTimestamp(aeMask);

    return;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_clearGPRs
   Description: Set all GPRs, transfer regs., next neigh reg., and local-memory
                to zero.
   Returns:     HALAE_SUCCESS, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
halAe_clearGPRs(unsigned int aeMask)
{
    unsigned char ae=0;
    unsigned int regData=0, absRegNum=0, ii=0;
 
    if(halAe_getMMScheme()) 
    {
       /* disable GPRs initialization if it is not normal scheme */
       return (HALAE_SUCCESS);
    }   

    for(ae=0; ae <= AeMaxNum; ae++)
    {
        if(!(aeMask & (1 << ae))) 
        {
             continue;
        }     
        for(absRegNum=0; absRegNum < MAX_GPR_REG; absRegNum++)
        {
            halAe_PutAbsDataReg(ae, ICP_GPA_ABS, (unsigned short)absRegNum, regData);
            halAe_PutAbsDataReg(ae, ICP_GPB_ABS, (unsigned short)absRegNum, regData);
            halAe_PutAbsDataReg(ae, ICP_SR_RD_ABS, (unsigned short)absRegNum, regData);
            halAe_PutAbsDataReg(ae, ICP_SR_WR_ABS, (unsigned short)absRegNum, regData);
            halAe_PutAbsDataReg(ae, ICP_DR_RD_ABS, (unsigned short)absRegNum, regData);
            halAe_PutAbsDataReg(ae, ICP_DR_WR_ABS, (unsigned short)absRegNum, regData);
            halAe_PutAbsDataReg(ae, ICP_NEIGH_ABS, (unsigned short)absRegNum, regData);
        }
        for(ii = 0; ii < MaxLmemReg; ii++)
        {
            halAe_PutLM(ae, (unsigned short)ii, regData);
        }

    }
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_initPrivateCSR
   Description: Set SIGNATURE_ENABLE[0] to 0x1 in order to enable ALU_OUT csr
   Returns:     HALAE_SUCCESS, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
halAe_initPrivateCSR(unsigned int aeMask)
{
    unsigned char ae=0;
    unsigned int csrVal=0;

    for(ae=0; ae <= AeMaxNum; ae++)
    {
        if(!(aeMask & (1 << ae))) 
        {
             continue;
        }     
        getAeCsr(ae, SIGNATURE_ENABLE, &csrVal);
        csrVal |= 0x1;
        putAeCsr(ae, SIGNATURE_ENABLE, csrVal);
    }
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:     halAe_enableIntr
   Description: 
   Returns:        
   Modifies:
-----------------------------------------------------------------------------*/
static void 
halAe_enableIntr(void)
{
    unsigned int grp, aeMask;

    if (!halAe_intrSupported())
    {
        return;
    }

    SET_CAP_CSR(CAP_ATTN_MASK_SET, ~AeBadMask);

    for(grp = 0, aeMask = ~AeBadMask; (aeMask & 0xf); grp ++)
    {
        SET_THD_CSR(grp, CAP_THD_ENABLE_SET_A_OFFSET, AE_THREAD_A_ALLBITS);
        SET_THD_CSR(grp, CAP_THD_ENABLE_SET_B_OFFSET, AE_THREAD_B_ALLBITS);
        aeMask = aeMask >> 4;
    }

    return;
}


/*-----------------------------------------------------------------------------
   Function:     halAe_disableIntr
   Description: 
   Returns:        
   Modifies:
-----------------------------------------------------------------------------*/
static void 
halAe_disableIntr(void)
{
    unsigned int grp, aeMask;

    if (!halAe_intrSupported())
    {
        return;
    }

    SET_CAP_CSR(CAP_ATTN_MASK_CLR, ~AeBadMask);

    for(grp = 0, aeMask = ~AeBadMask; (aeMask & 0xf); grp++)
    {
        SET_THD_CSR(grp, CAP_THD_ENABLE_CLR_A_OFFSET, AE_THREAD_A_ALLBITS);
        SET_THD_CSR(grp, CAP_THD_ENABLE_CLR_B_OFFSET, AE_THREAD_B_ALLBITS);
        aeMask = aeMask >> 4;
    }

    return;
}
/*-----------------------------------------------------------------------------
  Begin support for New Page callback
  -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
   Function:    halAe_Init
   Description: Initialize the AccelEngines and take them out of reset.
                AccelEngines with their corresponding bit set in aeMask will
                be initialized to the following state: eight context mode,
                program counters set to zero, next ctx to run set to zero, all
                ctx disabled, cc_enable set to 0x2000,wakeup-events set to one,
                and sig-event to zero.  All other AccelEngines will remain
                untouched - other than being taken out of reset.  If the
                function succeeds, a zero value will be returned, otherwise,
                nonzero value indicating the failure will be returned.

   Returns:     HALAE_SUCCESS, HALAE_FAIL
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_Init(unsigned int aeMask)
{
    unsigned char ae=0, clust=0;
    int status=HALAE_SUCCESS, ii=0;
    PageData_T *pageData=NULL;

#ifdef _HALAE_PRINT

    PRINTF("halAe_Init: aeMask=0x%x, HalAe_LibInit=%d, &HalAe_LibInit=0x%x\n",
        aeMask, HALAE_LIBINIT, &HALAE_LIBINIT);

#endif /* _HALAE_PRINT */

    if(!isLockInitialized) 
    {
        isLockInitialized = 1;
        SPIN_LOCK_INIT(halLock);
    }

    SPIN_LOCK(halLock);
    if(!HALAE_LIBINIT)
    {
        if((status = halAe_OSInit()) != HALAE_SUCCESS) 
        {
            SPIN_UNLOCK(halLock);
            return (status);
        }
        /* create spinlock to protect global variables */
        SPIN_LOCK_INIT(HAL_GLOBALLOCK);
        SPIN_LOCK_INIT(HAL_CALLBACKTHDLOCK);
        SPIN_LOCK_INIT(HAL_INTERRUPTLOCK);

        ISR_CALLBACK_ROOT = NULL;

        if(halAe_GetSysMemInfo(&SysMemInfo))  
        {
            SPIN_UNLOCK(halLock); 
            SPIN_LOCK_FINI(HAL_GLOBALLOCK);
            SPIN_LOCK_FINI(HAL_CALLBACKTHDLOCK);
            SPIN_LOCK_FINI(HAL_INTERRUPTLOCK);
            return (HALAE_FAIL);
        }

        /* get the product ID */
        getProdId(&PrdMajType, &PrdMinType, &PrdMajRev, &PrdMinRev);
        if (halAe_GetProdSetting())
        {
            SPIN_UNLOCK(halLock);
            SPIN_LOCK_FINI(HAL_GLOBALLOCK);
            SPIN_LOCK_FINI(HAL_CALLBACKTHDLOCK);
            SPIN_LOCK_FINI(HAL_INTERRUPTLOCK);
            return (HALAE_FAIL);
        }

        pageData = (PageData_T*)ixOsalMemAlloc(sizeof(PageData_T));
        if (pageData == NULL) 
        {  
            SPIN_UNLOCK(halLock); 
            SPIN_LOCK_FINI(HAL_GLOBALLOCK);
            SPIN_LOCK_FINI(HAL_CALLBACKTHDLOCK);
            SPIN_LOCK_FINI(HAL_INTERRUPTLOCK);
            return (HALAE_FAIL);
        }
        for (ae = 0; ae < MAX_AE; ae++) 
        {
            pageData->AePageData[ae].numPages = 0;
            pageData->AePageData[ae].addrs = NULL;
        }
        PAGEDATA = pageData;

        /* create AE objects */
        for(clust = 0; clust < AE_NUMCLUSTR; clust++)
        {
            for(ae = 0; (ae < AePerCluster) && (ae < AE_PERCLUSTR); ae++)
            {
                /* set available ustores */
                AEs[clust][ae].freeAddr = 0;    
                AEs[clust][ae].freeSize = MaxUstore;
                AEs[clust][ae].uStoreSize = MaxUstore;
                AEs[clust][ae].liveCtxMask = AE_ALL_CTX;
                AEs[clust][ae].state = HALAE_UNINIT;
                AEs[clust][ae].ustoreDramAddr = 0;
                AEs[clust][ae].reloadSize = 0;
                SPIN_LOCK_INIT(AEs[clust][ae].aeLock);
            }
        }

        for(ii = 0; ii < MAX_EAGLETAIL_RING_ENTRIES; ii++)
        {
            SPIN_LOCK_INIT(ringLock[ii]);
        }    
        for(ii = 0; ii < MAX_GIGE_MAC; ii++) 
        {
            SPIN_LOCK_INIT(macLock[ii]);
        }    

        /* Set intr CSRs on CAPBAR to enable atten and thread interrupt
	     * once hal initialization.
         * These CSRs will be reset by SHaC reset.
	     * They will not be cleared after resetting AE and SSU via
         * calling halAe_Reset.
         * Hence, no need to re-config them in halAe_ClrReset */
        halAe_enableIntr();
        HALAE_LIBINIT = 1;        
    }
    SPIN_UNLOCK(halLock);

    /* init selected AEs, and take all AEs out of reset */
    halAe_ClrReset(aeMask);

    if (PrdMinType == HWID_ICP)
    {
        halAe_clearGPRs(aeMask);
        if (halAe_intrSupported())
        {
            halAe_initPrivateCSR(aeMask);
        }
    }

    for (ii=0; ii<MAX_AE; ii++) 
    {
        ucloCallbacks[ii].uclo_callback = NULL;
    }

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_DelLib
   Description: Restore library resources
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
halAe_DelLib(void)
{
    
#ifdef _HALAE_PRINT

    PRINTF("halAe_DelLib: HalAe_LibInit=%d\n", HALAE_LIBINIT);

#endif /* _HALAE_PRINT */

    SPIN_LOCK(halLock);
    if(HALAE_LIBINIT)
    {
        unsigned char ae=0, clust=0;
        int ii=0;
        PageData_T *pageData;

        halAe_IntrDisable(0xFFFFFFFF);

        /* Remove UCLO callbacks */
        for (ii=0; ii<MAX_AE; ii++) 
        {
            ucloCallbacks[ii].uclo_callback = NULL;
        }

        /* clear ME_ATTN for all the AEs */
        halAe_disableIntr();

        for(ii = 0; ii < MAX_EAGLETAIL_RING_ENTRIES; ii++) 
        {
            SPIN_LOCK_FINI(ringLock[ii]);           
        }    
        for(ii = 0; ii < MAX_GIGE_MAC; ii++) 
        {
            SPIN_LOCK_FINI(macLock[ii]);
        }
        for(clust = 0; clust < AE_NUMCLUSTR; clust++)
        {
            for(ae = 0; (ae < AePerCluster) && (ae < AE_PERCLUSTR); ae++)
            {
                AEs[clust][ae].state = HALAE_UNINIT;
                SPIN_LOCK_FINI(AEs[clust][ae].aeLock);
            }
        }
         
        SPIN_LOCK_FINI(HAL_GLOBALLOCK);
        SPIN_LOCK_FINI(HAL_CALLBACKTHDLOCK);
        SPIN_LOCK_FINI(HAL_INTERRUPTLOCK);

        halAe_OSClose();

        pageData = PAGEDATA;
        for (ae = 0; ae < MAX_AE; ae++) 
        {
            if (pageData->AePageData[ae].addrs) 
            {
                ixOsalMemFree(pageData->AePageData[ae].addrs);
            }   
        }
        ixOsalMemFree(pageData);
        PAGEDATA = NULL;
        
        HALAE_LIBINIT = 0;
    }
    SPIN_UNLOCK(halLock);
    
    SPIN_LOCK_FINI(halLock);
    isLockInitialized = 0;
    
    return;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_SetReloadUstore
   Description: Define the reloadable ustore, reloadSize must be 2k, 6k, or 8k.
   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_MEMALLOC
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_SetReloadUstore(unsigned char ae, 
                      unsigned int reloadSize, 
                      int sharedMode,
                      unsigned int ustoreDramAddr)
{
    unsigned int csrVal, controlStoreReload;
    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

#ifdef _HALAE_PRINT
 
    PRINTF("halAe_SetReloadUstore: Ae=%d, reloadSize=%d\n", ae, reloadSize);

#endif /* _HALAE_PRINT */

    switch(reloadSize)
    {
    case 0: controlStoreReload = 0; break;
    case 2048: controlStoreReload = 1; break;
    case 4096: controlStoreReload = 2; break;
    case 8192: controlStoreReload = 3; break;
    default: return (HALAE_BADARG);
    }

    SPIN_LOCK_AE(ae);
    if(controlStoreReload)
    {
        AE(ae).ustoreDramAddr = ustoreDramAddr;
    }
    AE(ae).reloadSize = reloadSize;
    getAeCsr(ae, AE_MISC_CONTROL, &csrVal);
    /* clear bits <22:20> and two */
    csrVal &= ~((0x7 << MMC_CS_RELOAD_BITPOS) | MMC_SHARE_CS_BITPOS); 
    csrVal |= controlStoreReload << MMC_CS_RELOAD_BITPOS | \
              ((sharedMode & 0x1) << MMC_ONE_CTX_RELOAD_BITPOS);
    putAeCsr(ae, AE_MISC_CONTROL, csrVal);
    SPIN_UNLOCK_AE(ae);

    return (HALAE_SUCCESS);
}


/*-----------------------------------------------------------------------------
   Function:    halAe_SetUstoreFreeMem
   Description: Define the ustore free region
   Returns:     HALAE_SUCCESS, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_SetUstoreFreeMem(unsigned char ae, 
                       unsigned int freeAddr, 
                       unsigned int freeSize)
{
    unsigned char scsMode=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

#ifdef _HALAE_PRINT

    PRINTF("halAe_SetUstoreFreeMem: Ae=%d, freeAddr=%d, freeSize=%d\n", ae, 
           freeAddr, freeSize);

#endif /* _HALAE_PRINT */

    SPIN_LOCK_AE(ae);
    getAeSharedCsMode(ae, &scsMode);
    if(scsMode) 
    {
        if((freeAddr > MaxUstore*2) || ((freeAddr + freeSize) > MaxUstore*2))
        {
            SPIN_UNLOCK_AE(ae);
            return (HALAE_BADARG);
        }    
    }
    else
    {
        if((freeAddr > MaxUstore) || ((freeAddr + freeSize) > MaxUstore)) 
        {
            SPIN_UNLOCK_AE(ae);
            return (HALAE_BADARG);
        }    
    }
    
    AE(ae).freeAddr = freeAddr;
    AE(ae).freeSize = freeSize;
    SPIN_UNLOCK_AE(ae);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetUstoreFreeMem
   Description: Get the ustore free region
   Returns:     HALAE_SUCCESS, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetUstoreFreeMem(unsigned char ae, 
                       unsigned int *freeAddr, 
                       unsigned int *freeSize)
{
    
#ifdef _HALAE_PRINT

    PRINTF("halAe_GetUstoreFreeMem: Ae=%d\n", ae);

#endif /* _HALAE_PRINT */

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    if(!freeAddr || !freeSize) 
    {
        return (HALAE_BADARG);
    }    

    SPIN_LOCK_AE(ae);
    *freeAddr = AE(ae).freeAddr;
    *freeSize = AE(ae).freeSize;
    SPIN_UNLOCK_AE(ae);
    return (HALAE_SUCCESS);
}

                                       
/*-----------------------------------------------------------------------------
   Function:    enableCtx
   Description: Enable the specified micro-thread(s) -- those that were
                already enabled, will remain enabled.
   Returns:     HALAE_SUCCESS, HALAE_RESET, HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
enableCtx(unsigned char ae, 
          unsigned int ctxMask)
{
    unsigned int ctxEn, wrCtxEn;

#ifdef _HALAE_PRINT

    PRINTF("enableCtx: ae=%d, ctxMask=0x%x\n", ae, ctxMask);

#endif /* _HALAE_PRINT */

    if(IS_RST(ae)) 
    {
       return (HALAE_RESET);
    }   

    /* get the contexts that are enabled */
    getAeCsr(ae, CTX_ENABLES, &ctxEn);
    /* prevent clearing the W1C bits: the breakpoint bit, 
       ECC error bit, and Parity error bit */
    ctxEn &= IGNORE_W1C_MASK;            

    if(ctxEn & CE_INUSE_CONTEXTS) {
        ctxMask &= 0x55;
    }    
    else
    {
        ctxMask &= 0xFF;
    }    

    /* set selected bits high */
    wrCtxEn = (ctxMask << CE_ENABLE_BITPOS) | ctxEn; 
    putAeCsr(ae, CTX_ENABLES, wrCtxEn);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    disableCtx
   Description: Disable the specified micro-thread(s).
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_RESET, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
disableCtx(unsigned char ae, 
           unsigned int ctxMask)
{
    unsigned int ctxEn, wrCtxEn;

#ifdef _HALAE_PRINT

    PRINTF("stopAe: ae=%d, ctxMask=0x%x\n", ae, ctxMask);

#endif /* _HALAE_PRINT */

    if(IS_RST(ae)) 
    {
       return (HALAE_RESET);
    }   

    /* get the contexts that are enabled */
    getAeCsr(ae, CTX_ENABLES, &ctxEn);
    /* prevent clearing the W1C bits: the breakpoint bit, 
       ECC error bit, and Parity error bit */
    ctxEn &= IGNORE_W1C_MASK;            

    /* set the ctx_enable<enable> bits */
    wrCtxEn = (~((ctxMask & 0xff) << CE_ENABLE_BITPOS)) & ctxEn;
    putAeCsr(ae, CTX_ENABLES, wrCtxEn);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_Start
   Description: Start AE ctx from current PC
   Returns:     HALAE_SUCCESS, HALAE_BADARG,HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_Start(unsigned char ae, 
            unsigned int ctxEnMask)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
     /* disable the other ctx */
    putCtxWakeupEvents(ae, (~ctxEnMask) & 0xff, WAKEUP_EVENTS_SLEEP);     
    stat = enableCtx(ae, ctxEnMask);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_Stop
   Description: Disable the specified micro-thread(s).
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_RESET, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_Stop(unsigned char ae, 
           unsigned int ctxMask)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = disableCtx(ae, ctxMask);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putCtxIndrCsr
   Description: Write a 32-bit value to a context(s) indirect CSR.
   Returns:     HALAE_SUCCESS,HALAE_BADARG,HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putCtxIndrCsr(unsigned char ae, 
              unsigned int ctxMask, 
              unsigned int aeCsr, 
              unsigned int csrVal)
{
    unsigned int ctx,  ctxPtr;

    switch(aeCsr)
    {
    case CTX_FUTURE_COUNT_INDIRECT:
    case CTX_WAKEUP_EVENTS_INDIRECT:
    case CTX_STS_INDIRECT:
    case CTX_SIG_EVENTS_INDIRECT:
    case LM_ADDR_0_INDIRECT:
    case LM_ADDR_1_INDIRECT:
            break;
    default: return (HALAE_BADARG);
    }

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);    /* save the ctx ptr */
    for(ctx = 0; ctx < MAX_CTX; ctx++)
    {
        if(ctxMask & setMasks[ctx])
        {
            putAeCsr(ae, CSR_CTX_POINTER, ctx);
            putAeCsr(ae, aeCsr, csrVal);
        }
    }
    putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCtxIndrCsr
   Description: Write a 32-bit value to a context(s) indirect CSR.
                It's unsafe to call this function while the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCtxIndrCsr(unsigned char ae, 
                    unsigned int ctxMask, 
                    unsigned int aeCsr, 
                    unsigned int csrVal)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
       stat = HALAE_AEACTIVE;
    }    
    else
    {
       stat = putCtxIndrCsr(ae, ctxMask, aeCsr, csrVal);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getCtxIndrCsr_Common
   Description: Read a context indirect-csr.
   Returns:     HALAE_SUCCESS, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
getCtxIndrCsr_Common(unsigned char ae, 
              unsigned char ctx, 
              unsigned int aeCsr, 
              unsigned int *csrVal)
{
    unsigned int ctxPtr;
    int status = HALAE_SUCCESS;

    switch(aeCsr)
    {
    case CTX_FUTURE_COUNT_INDIRECT:
    case CTX_STS_INDIRECT:
            break;
    case CTX_WAKEUP_EVENTS_INDIRECT:
    case CTX_SIG_EVENTS_INDIRECT:
            break;
    case LM_ADDR_0_INDIRECT:
    case LM_ADDR_1_INDIRECT:
            break;
    default: return (HALAE_BADARG);
    }

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);    /* save the ctx ptr */
    if((ctxPtr & CCP_CONTEXT) != (ctx & CCP_CONTEXT))
    {
        putAeCsr(ae, CSR_CTX_POINTER, ctx);
    }
    status = getAeCsr(ae, aeCsr, csrVal);

    if((ctxPtr & CCP_CONTEXT) != (ctx & CCP_CONTEXT))
    {
        putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */
    }    

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCtxIndrCsr
   Description: Read a context indirect-csr. It's unsafe to call this function
                while the AE is enabled.

   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCtxIndrCsr(unsigned char ae, 
                    unsigned char ctx, 
                    unsigned int aeCsr, 
                    unsigned int *csrVal)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    VERIFY_ARGPTR(csrVal);
    SPIN_LOCK_AE(ae);
    if(isAeActive(ae)) 
    {
       stat = HALAE_AEACTIVE;
    }   
    else
    {
       stat = getCtxIndrCsr(ae, ctx, aeCsr, csrVal);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCtxStatus
   Description: Write the context status CSR.
                It's unsafe to call this function while the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCtxStatus(unsigned char ae, 
                   unsigned int ctxMask, 
                   unsigned int ctxStatus)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);

    if(isAeActive(ae))
    {
       stat = HALAE_AEACTIVE;
    }   
    else
    {
       stat = putCtxIndrCsr(ae, ctxMask, CTX_STS_INDIRECT, ctxStatus);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutPC
   Description: Set the indicated threads to a specified uPC.
                It's unsafe to call this function while the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutPC(unsigned char ae, 
            unsigned int ctxMask, 
            unsigned int upc)
{
    return (halAe_PutCtxStatus(ae, ctxMask, UpcMask & upc));
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCtxStatus
   Description: Read the context status CSR.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCtxStatus(unsigned char ae, 
                       unsigned char ctx, 
                       unsigned int *ctxStatus)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    VERIFY_ARGPTR(ctxStatus);
    SPIN_LOCK_AE(ae);
    if(isAeActive(ae))
    {
       stat = HALAE_AEACTIVE;
    }   
    else
    {
       stat = getCtxIndrCsr(ae, ctx, CTX_STS_INDIRECT, ctxStatus);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetPC
   Description: Get the program counter of the specified context.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetPC(unsigned char ae, 
            unsigned char ctx, 
            unsigned int *upc)
{
    unsigned int ctxStatus;
    int stat = HALAE_SUCCESS;

    if(upc == NULL)
    {
    	return (HALAE_BADARG);
    }    

    if((stat = halAe_GetCtxStatus(ae, ctx, &ctxStatus)))
    {
        *upc = INVALID_PC;
    }    
    else
    {
        *upc = (ctxStatus & UpcMask) >> ICS_CTX_PC_BITPOS;
    }
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getCtxSigEvents
   Description: Get the state of the sig-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getCtxSigEvents(unsigned char ae, 
                unsigned char ctx, 
                unsigned int *events)
{
    unsigned int ctxPtr;

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);        /* save the ctx ptr */
    if((ctxPtr & CCP_CONTEXT) != (ctx & CCP_CONTEXT))
    {
        putAeCsr(ae, CSR_CTX_POINTER, ctx);
    }
    getAeCsr(ae, CTX_SIG_EVENTS_INDIRECT, events);

    if((ctxPtr & CCP_CONTEXT) != (ctx & CCP_CONTEXT))
    {
        putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */
    }    

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCtxSigEvents
   Description: Get the state of the sig-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCtxSigEvents(unsigned char ae, 
                      unsigned char ctx, 
                      unsigned int *events)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    VERIFY_ARGPTR(events);
    SPIN_LOCK_AE(ae);
    if(isAeActive(ae))
    {
       stat = HALAE_AEACTIVE;
    }   
    else
    { 
       stat = getCtxSigEvents(ae, ctx, events);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putCtxSigEvents
   Description: Write the state of the sig-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putCtxSigEvents(unsigned char ae, 
                unsigned int ctxMask, 
                unsigned int events)
{
    unsigned int ctx, ctxPtr;

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);    /* save the ctx ptr */
    for(ctx = 0; ctx < MAX_CTX; ctx++)
    {
        /* save signaled events */
        if(ctxMask & setMasks[ctx])
        {
            putAeCsr(ae, CSR_CTX_POINTER, ctx);
            putAeCsr(ae, CTX_SIG_EVENTS_INDIRECT, events);
        }
    }
    putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCtxSigEvents
   Description: Write the state of the sig-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCtxSigEvents(unsigned char ae, 
                      unsigned int ctxMask, 
                      unsigned int events)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    if(isAeActive(ae))
    {
       stat = HALAE_AEACTIVE;
    }   
    else 
    {
       stat = putCtxSigEvents(ae, ctxMask, events);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getCtxWakeupEvents
   Description: Read the state of the wakeup-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getCtxWakeupEvents(unsigned char ae, 
                   unsigned char ctx, 
                   unsigned int *events)
{
    unsigned int ctxPtr;

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);    /* save the ctx ptr */
    if((ctxPtr & CCP_CONTEXT) != (ctx & CCP_CONTEXT)) 
    {
        putAeCsr(ae, CSR_CTX_POINTER, ctx);
    }
    getAeCsr(ae, CTX_WAKEUP_EVENTS_INDIRECT, events);

    if((ctxPtr & CCP_CONTEXT) != (ctx & CCP_CONTEXT))
    {
        putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */
    }    

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCtxWakeupEvents
   Description: Read the state of the wakeup-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCtxWakeupEvents(unsigned char ae, 
                         unsigned char ctx, 
                         unsigned int *events)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(events);
    VERIFY_AE(ae);
    VERIFY_CTX(ctx);
    SPIN_LOCK_AE(ae);
    if(isAeActive(ae))
    {
       stat = HALAE_AEACTIVE;
    }   
    else
    {
       stat = getCtxWakeupEvents(ae, ctx, events);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putCtxWakeupEvents
   Description: Write the state of the wakeup-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putCtxWakeupEvents(unsigned char ae, 
                   unsigned int ctxMask, 
                   unsigned int events)
{
    unsigned int ctx=0, ctxPtr=0;

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);    /* save the ctx ptr */
    for(ctx = 0; ctx < MAX_CTX; ctx++)
    {
        if(ctxMask & setMasks[ctx])
        {
            putAeCsr(ae, CSR_CTX_POINTER, ctx);
            putAeCsr(ae, CTX_WAKEUP_EVENTS_INDIRECT, events);
        }
    }
    putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCtxWakeupEvents
   Description: Write the state of the wakeup-event signals.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCtxWakeupEvents(unsigned char ae, 
                         unsigned int ctxMask, 
                         unsigned int events)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    if(isAeActive(ae))
    {
       stat = HALAE_AEACTIVE;
    }   
    else
    {
       stat = putCtxWakeupEvents(ae, ctxMask, events);
    }   
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    isAeEnabled
   Description: Determine if the AE ctx is enabled -- enable to run, or 
                is running.
   Returns:     HALAE_ENABLED,HALAE_DISABLED,HALAE_BADLIB,HALAE_RESET,
                HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
isAeEnabled(unsigned char ae)
{
   unsigned int csr=0;

   getAeCsr(ae, CTX_ENABLES, &csr);
   if(csr & CE_ENABLE)
   {
      return (HALAE_ENABLED);
   }   

   getAeCsr(ae, ACTIVE_CTX_STATUS, &csr);
   if((csr & (1 << ACS_ABO_BITPOS))) 
   {
      return (HALAE_AEACTIVE);
   }    

   return (HALAE_DISABLED);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_IsAeEnabled
   Description: Determine if any of the AE ctx is enabled or is running.
   Returns:     HALAE_ENABLED,HALAE_DISABLED,HALAE_BADLIB,HALAE_RESET,
                HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_IsAeEnabled(unsigned char ae)
{
   int stat = HALAE_SUCCESS;

   HALAE_VERIFY_LIB();
   SPIN_LOCK_AE(ae);
   stat = isAeEnabled(ae);
   SPIN_UNLOCK_AE(ae);
   return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    setUwordECC
   Description: Determine the ecc and return the uword with the appropriates
                bits set.
   Returns:
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uword_T 
setUwordECC(uword_T uword)
{
    uword_T chkBit0Mask=0xff800007fffULL, chkBit1Mask=0x1f801ff801fULL,
            chkBit2Mask=0xe387e0781e1ULL, chkBit3Mask=0x7cb8e388e22ULL,
            chkBit4Mask=0xaf5b2c93244ULL, chkBit5Mask=0xf56d5525488ULL,
            chkBit6Mask=0xdaf69a46910ULL;

    uword &= ~(0x7fULL << 44);    /* clear the ecc bits */
    uword |= (uword_T)bitParity64(chkBit0Mask & uword) << 44;
    uword |= (uword_T)bitParity64(chkBit1Mask & uword) << 45;
    uword |= (uword_T)bitParity64(chkBit2Mask & uword) << 46;
    uword |= (uword_T)bitParity64(chkBit3Mask & uword) << 47;
    uword |= (uword_T)bitParity64(chkBit4Mask & uword) << 48;
    uword |= (uword_T)bitParity64(chkBit5Mask & uword) << 49;
    uword |= (uword_T)bitParity64(chkBit6Mask & uword) << 50;

    return (uword);
}

/*-----------------------------------------------------------------------------
   Function:    getReloadUwords
   Description: Read a number of long-words from the reloadable ustore dram 
                buffer. The micro engine must be inactive, possible under reset, 
                before writing to micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getReloadUwords(unsigned char ae, 
                unsigned int uAddr, 
                unsigned int numWords, 
                uword_T *uWord)
{
    int status = HALAE_SUCCESS;
    unsigned int ii=0;

    if(!uWord || (uAddr + numWords) > KILO_128) 
    {
        return (HALAE_BADARG);
    }    
    if((status = isAeEnabled(ae)) != HALAE_DISABLED) 
    {
        return (status);
    }    

    for(ii = 0; ii < numWords; ii++)
    {
        unsigned int wrdHi, wrdLo;
        wrdLo = DRAM_READ_CH0(AE(ae).ustoreDramAddr + uAddr);
        wrdHi = DRAM_READ_CH0(AE(ae).ustoreDramAddr + uAddr + 4);
        uWord[ii] = (uword_T)wrdHi << 32 | wrdLo; 
        uAddr +=8;
    }
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    putReloadUwords
   Description: Write a number of long-words to the reloadable ustore dram 
                buffer.The micro engine must be inactive, possible under reset, 
                before writing to micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int
putReloadUwords(unsigned char ae, 
                unsigned int uAddr, 
                unsigned int numWords, 
                uword_T *uWord)
{
    int status = HALAE_SUCCESS;
    unsigned int ii=0;
    uword_T uwrdEcc;

    if(!uWord || (uAddr + numWords) > KILO_128) 
    {
        return (HALAE_BADARG);
    }    
    if((status = isAeEnabled(ae)) != HALAE_DISABLED)
    {
         return (status);
    }     

    for(ii = 0; ii < numWords; ii++)
    {
        uwrdEcc = setUwordECC(uWord[ii]);
        DRAM_WRITE_CH0(AE(ae).ustoreDramAddr + uAddr, \
                       (unsigned int)(uwrdEcc & 0xffffffff));
        DRAM_WRITE_CH0(AE(ae).ustoreDramAddr + uAddr + 4, \
                       (unsigned int)(uwrdEcc >> 32));
        uAddr += 8;
    }

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    getUwords
   Description: Read a number of long-words from micro-store. The accelEngine 
                must be disabled before reading from micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getUwords(unsigned char ae, 
          unsigned int uAddr, 
          unsigned int numWords, 
          uword_T *uWord)
{
    int status = HALAE_SUCCESS;
    unsigned int ii=0, uwrdLo=0, uwrdHi=0, ustoreAddr, miscControl;

    if((status = isAeEnabled(ae)) != HALAE_DISABLED) 
    {
        return (status);
    }    

    /* if reloadable, then get it all from dram-ustore */
    miscControl = GET_AE_CSR(ae, AE_MISC_CONTROL);
    if((miscControl >> 20) & 0x3)
    {
        return (getReloadUwords(ae, uAddr, numWords, uWord));
    }
    
    /* disable SHARE_CS bit to workaround silicon bug */
    SET_AE_CSR(ae, AE_MISC_CONTROL, miscControl & 0xfffffffb);

    /* get it from phy-ustore */
    if(!uWord || (uAddr + numWords) > GET_USTORE_SIZE(ae)) 
    {
        return (HALAE_BADARG);
    }    
    /* save ustore-addr csr */
    ustoreAddr = GET_AE_CSR(ae, USTORE_ADDRESS);            

    uAddr |= UA_ECS;        /* enable ecs bit */
    for(ii = 0; ii < numWords; ii++)
    {
        SET_AE_CSR(ae, USTORE_ADDRESS, uAddr);
        /* delay several cycless till USTORE_ADDRESS actually updated */
        halAe_WaitUStoreAddrReady(ae);           

        uAddr++;
        uwrdLo = GET_AE_CSR(ae, USTORE_DATA_LOWER);
        uwrdHi = GET_AE_CSR(ae, USTORE_DATA_UPPER);
        uWord[ii] = uwrdHi;
        uWord[ii] = (uWord[ii] << 32) | uwrdLo;
    }
    
    /* restore SHARE_CS bit to workaround silicon bug */
    SET_AE_CSR(ae, AE_MISC_CONTROL, miscControl);

    SET_AE_CSR(ae, USTORE_ADDRESS, ustoreAddr);
    halAe_WaitUStoreAddrReady(ae);           

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetUwords
   Description: Read a number of long-words from micro-store. The micro engine 
                must be inactive before reading from micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetUwords(unsigned char ae, 
                unsigned int uAddr, 
                unsigned int numWords, 
                uword_T *uWord)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(uWord);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = getUwords(ae, uAddr, numWords, uWord);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putUwords
   Description: Write a number of long-words to micro-store.  The micro engine 
                must be inactive, possible under reset, before writing to 
                micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putUwords(unsigned char ae, 
          unsigned int uAddr, 
          unsigned int numWords, 
          uword_T *uWord)
{
    int status = HALAE_SUCCESS;
    unsigned int ii=0, uwrdLo=0, uwrdHi=0, ustoreAddr, miscControl;
    unsigned int csReload, fixedSize=0;
    uword_T tmp = 0;
    unsigned int shCtlStoreFlag;
    unsigned char aeNeigh;

    if((status = isAeEnabled(ae)) != HALAE_DISABLED) 
    {
        return (status);
    }    
    /* determine whether it neighbour AE runs in shared control store status */
    miscControl = GET_AE_CSR(ae, AE_MISC_CONTROL);
    shCtlStoreFlag = miscControl & (0x1 << MMC_SHARE_CS_BITPOS);
    if(shCtlStoreFlag) 
    {
       halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
       if(isAeActive(aeNeigh)) 
       {
             return (HALAE_NEIGHAEACTIVE); 
       }      
    }

    /* if reloadable, then update the DRAM */
    if((csReload = (miscControl >> 20) & 0x3))
    {
        if(putReloadUwords(ae, uAddr, numWords, uWord))
        {
           return (HALAE_BADARG);
        }   
    }
    else
    {
        if(!uWord || (uAddr + numWords) > GET_USTORE_SIZE(ae))
        {
           return (HALAE_BADARG);
        }   
    }

    /* update the phy ustore -- the fixed portion */
    switch(csReload)
    {
    case 0x0: fixedSize = 8192; break; /* 0 reloadable, 8k fixed */
    case 0x1: fixedSize = 6144; break; /* 2k reloadable, 6k fixed */
    case 0x2: fixedSize = 4096; break; /* 4k reloadable, 4k fixed */
    case 0x3: fixedSize = 0; break;    /* 8k reloadable, 0 fixed */
    }

    if(csReload)
    {
        if(uAddr >= fixedSize) 
        {
            /* uaddr beyond the fixed-region */    
            return (HALAE_SUCCESS);                  
        }    
        if((uAddr + numWords) > fixedSize) 
        {
            /* write only within the fixed-region */
            numWords = fixedSize - uAddr;           
        }    
    }

    if(!numWords)
    {
        return (HALAE_SUCCESS);
    }    
    ustoreAddr = GET_AE_CSR(ae, USTORE_ADDRESS);    /* save ustore-addr csr */
    uAddr |= UA_ECS;                                /* enable ecs bit */
    SET_AE_CSR_RAW(ae, USTORE_ADDRESS, uAddr);      /* set the uaddress */
    for(ii = 0; ii < numWords; ii++)
    {
      if (PrdMinType == HWID_ICP) 
      {
        tmp = setUwordECC(uWord[ii]);
      }  
      uwrdLo = (unsigned int)(tmp & 0xffffffff);
      uwrdHi = (unsigned int)(tmp >> 32);

      SET_AE_CSR_RAW(ae, USTORE_DATA_LOWER, uwrdLo);
      /* this will auto increment the address */
      SET_AE_CSR_RAW(ae, USTORE_DATA_UPPER, uwrdHi);    
    }
    SET_AE_CSR(ae, USTORE_ADDRESS, ustoreAddr);
                        
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutUwords
   Description: Write a number of long-words to micro-store.  The micro engine 
                must be inactive, possible under reset, before writing to 
                micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutUwords(unsigned char ae, 
                unsigned int uAddr, 
                unsigned int numWords,
                uword_T *uWord)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(uWord);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = putUwords(ae, uAddr, numWords, uWord);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_putCoalesceUwords
   Description: Write a number of long-words to micro-store memory specified 
                by the uAddr word-address such that the even address goes to 
                the even numbered AE, and the odd address goes to the odd 
                numbered AE.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_FAIL, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
halAe_putCoalesceUwords(unsigned char ae, 
                        unsigned int uAddr,
                        unsigned int numWords, 
                        uword_T *uWord)
{
    int stat = HALAE_SUCCESS;
    unsigned int ii, evenCpyCnt=0, oddCpyCnt=0;
    unsigned char aeNeigh, evenAe, oddAe;
    
    uword_T          *evenUwords, *oddUwords;
    evenUwords = (uword_T *)ixOsalMemAlloc(UBUF_SIZE*8);
    if(evenUwords == NULL) 
    {
        return (HALAE_FAIL);
    }    
    oddUwords = (uword_T *)ixOsalMemAlloc(UBUF_SIZE*8);
    if(oddUwords == NULL) 
    {
        ixOsalMemFree(evenUwords);
        return (HALAE_FAIL);
    }    

    if((stat = isAeEnabled(ae)) != HALAE_DISABLED) 
    {
        ixOsalMemFree(evenUwords);
        ixOsalMemFree(oddUwords);        

        return (stat);
    }
    halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
    if((stat = isAeEnabled(aeNeigh)) != HALAE_DISABLED) 
    {
        ixOsalMemFree(evenUwords);
        ixOsalMemFree(oddUwords);        

        return (HALAE_NEIGHAEACTIVE);
    }

    /* establish the even and odd ae */
    if (ae & 0x1)
    {
        oddAe = ae;
        evenAe = aeNeigh;
    }
    else
    {
        oddAe = aeNeigh;
        evenAe = ae;
    }

    /* split into even and odd ae buffers */
    for(ii=0; ii < numWords; ii++)
    {
        if((uAddr + ii) & 1) 
        {
            oddUwords[oddCpyCnt++] = uWord[ii];
        }    
        else 
        {
            evenUwords[evenCpyCnt++] = uWord[ii];
        }    
    }

    /* copy the even ae buffer if necessary */
    if(evenCpyCnt)
    {
        if((stat = putUwords(evenAe, (uAddr+1)/2, evenCpyCnt, evenUwords))) 
        {
            ixOsalMemFree(evenUwords);
            ixOsalMemFree(oddUwords);        

            return (stat);
        }
    }

    /* copy the odd ae buffer if necessary */
    if(oddCpyCnt)
    {
        if((stat = putUwords(oddAe, uAddr/2, oddCpyCnt, oddUwords)))
        {
            ixOsalMemFree(evenUwords);
            ixOsalMemFree(oddUwords);        
   
            return (stat);
        }
    }

    ixOsalMemFree(evenUwords);
    ixOsalMemFree(oddUwords);        

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCoalesceUwords
   Description: Write a number of long-words to micro-store memory specified by 
                the uAddr word-address so that the even address go to the even
                numbered AE, and the odd address go to the odd numbered AE.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCoalesceUwords(unsigned char ae, 
                        unsigned int uAddr,
                        unsigned int numWords, 
                        uword_T *uWord)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(uWord);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = halAe_putCoalesceUwords(ae, uAddr, numWords, uWord);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_getCoalesceUwords
   Description: Read a number of long-words from micro-store memory specified 
                by the uAddr word-address such that the even address is taken 
                from even numbered AE, and the odd address is taken from the 
                odd numbered AE.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_FAIL, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
halAe_getCoalesceUwords(unsigned char ae, 
                        unsigned int uAddr,
                        unsigned int numWords, 
                        uword_T *uWord)
{
    unsigned int ii=0, evenCpyCnt, oddCpyCnt, stat, evenCnt, oddCnt;
    unsigned char aeNeigh, evenAe, oddAe;
    
    uword_T          *evenUwords, *oddUwords;
    evenUwords = (uword_T *)ixOsalMemAlloc(UBUF_SIZE*8);
    if(evenUwords == NULL) 
    {
        return (HALAE_FAIL);
    }    
    oddUwords = (uword_T *)ixOsalMemAlloc(UBUF_SIZE*8);
    if(oddUwords == NULL) 
    {
        ixOsalMemFree(evenUwords);        
        return (HALAE_FAIL);
    }    

    if((stat = isAeEnabled(ae)) != HALAE_DISABLED)  
    {
        ixOsalMemFree(evenUwords);
        ixOsalMemFree(oddUwords);        

        return (stat);
    }
    halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
    if((stat = isAeEnabled(aeNeigh)) != HALAE_DISABLED)  
    {
        ixOsalMemFree(evenUwords);
        ixOsalMemFree(oddUwords);        

        return (stat);
    }

    /* establish the even and odd ae */
    if (ae & 0x1)
    {
        oddAe = ae;
        evenAe = aeNeigh;
    }
    else
    {
        oddAe = aeNeigh;
        evenAe = ae;
    }

    /* determine how many even and odd words to read */
    evenCpyCnt = numWords/2;
    oddCpyCnt = evenCpyCnt;
    if (uAddr & 0x1)
    {
        if(numWords & 0x1) oddCpyCnt += 1;  /* read one more odd uword */
    }
    else
    {
        if(numWords & 0x1) evenCpyCnt += 1;  /* read one more even uword */
    }

    /* read the even words if necessary */
    if (evenCpyCnt)
    {
        if((stat = getUwords(evenAe, (uAddr+1)/2, evenCpyCnt, evenUwords))) 
        {
            ixOsalMemFree(evenUwords);
            ixOsalMemFree(oddUwords);        

            return (stat);
        }
    }

    /* read the odd words if necessary */
    if (oddCpyCnt)
    {
        if((stat = getUwords(oddAe, uAddr/2, oddCpyCnt, oddUwords))) 
        {
            ixOsalMemFree(evenUwords);
            ixOsalMemFree(oddUwords);        

            return (stat);
        }
    }

    /* merge into one buffer */
    oddCnt = 0;
    evenCnt = 0;
    for(ii=0; ii < numWords; ii++)
    {
        if((uAddr + ii) & 1) 
        {
            uWord[ii] = oddUwords[oddCnt++];
        }    
        else 
        {
            uWord[ii] = evenUwords[evenCnt++];
        }    
    }

    if (oddCnt != oddCpyCnt || evenCnt != evenCpyCnt) 
    {
        ixOsalMemFree(evenUwords);
        ixOsalMemFree(oddUwords);        

    return (HALAE_FAIL); /* should not happen */
    }

    ixOsalMemFree(evenUwords);
    ixOsalMemFree(oddUwords);        

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCoalesceUwords
   Description: Read a number of long-words from micro-store memory specified 
                by the uAddr word-address such that the even address is taken 
                from even numbered AE, and the odd address is taken from the 
                odd numbered AE.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCoalesceUwords(unsigned char ae, 
                        unsigned int uAddr,
                        unsigned int numWords, 
                        uword_T *uWord)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(uWord);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = halAe_getCoalesceUwords(ae, uAddr, numWords, uWord);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    getUmem
   Description: Read a number of long-words from micro-store memory specified 
                by the uAddr word-address. The accelEngine must be disabled 
                before reading from micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_FAIL, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getUmem(unsigned char ae, 
        unsigned int uAddr, 
        unsigned int numWords,
        unsigned int *data)
{
    int status = HALAE_SUCCESS;
    unsigned int ii=0, ustoreAddr, uwrdLo, uwrdHi, miscControl;

    if(PrdMinType != HWID_ICP) 
    {
       return (HALAE_FAIL);
    }   
    if(!data || (uAddr + numWords) > GET_USTORE_SIZE(ae)) 
    {
       return (HALAE_BADARG);
    }   
    if((status = isAeEnabled(ae)) != HALAE_DISABLED) 
    {
       return (status);
    }   

    miscControl = GET_AE_CSR(ae, AE_MISC_CONTROL);
    /* disable SHARE_CS bit to workaround silicon bug */
    SET_AE_CSR(ae, AE_MISC_CONTROL, miscControl & 0xfffffffb);

    ustoreAddr = GET_AE_CSR(ae, USTORE_ADDRESS);
    uAddr |= UA_ECS;        /* enable ecs bit */
    for(ii = 0; ii < numWords; ii++)
    {
        SET_AE_CSR(ae, USTORE_ADDRESS, uAddr++);
        halAe_WaitUStoreAddrReady(ae);           

        uwrdLo = GET_AE_CSR(ae, USTORE_DATA_LOWER);
        uwrdHi = GET_AE_CSR(ae, USTORE_DATA_UPPER);

        /* Data is stored in ustore data register as: 
            Uword1[9] parity for data bits [31:16], 
            Uword1[8] parity for data bits [15:0]
            Uword1[7:4] 4'b1111, Uword1[3:0] Data[31:28]
            Uword0[31:20] Data[27:16], Uword0[19:18] 2'b11, 
            Uword0[17:10] Data[15:8], Uword0[9:8] 2'b11, Uword0[7:0] Data[7:0]
        */
        data[ii] = ((uwrdHi & 0xf) << 28) | ((uwrdLo & 0xfff00000) >> 4) | \
                    ((uwrdLo & 0x3fc00) >> 2) | (uwrdLo & 0xff);
    }

    /* restore SHARE_CS bit to workaround silicon bug */
    SET_AE_CSR(ae, AE_MISC_CONTROL, miscControl);

    SET_AE_CSR(ae, USTORE_ADDRESS, ustoreAddr);
    halAe_WaitUStoreAddrReady(ae);           

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetUmem
   Description: Read a number of long-words to micro-store memory specified by 
                the uAddr word-address. The micro engine must be inactive 
                before reading from micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG,
                HALAE_RESET, HALAE_AEACTIVE, HALAE_FAIL, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetUmem(unsigned char ae, 
              unsigned int uAddr, 
              unsigned int numWords,
              unsigned int *data)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(data);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    /* For data, phys==virt for uAddr */
    stat = getUmem(ae, uAddr, numWords, data);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putUmem
   Description: Write a number of long-words to micro-store memory specified by
                the uAddr word-address. The micro engine must be inactive, 
                possible under reset, before writing to micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_FAIL, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putUmem(unsigned char ae, 
        unsigned int uAddr, 
        unsigned int numWords,
        unsigned int *data)
{
    int status = HALAE_SUCCESS;
    unsigned int ii=0, ustoreAddr, uwrdLo, uwrdHi, tmp;
    unsigned int csrVal, shCtlStoreFlag;
    unsigned char aeNeigh;

    if(PrdMinType != HWID_ICP) 
    {
       return (HALAE_FAIL);
    }   
    if(!data || (uAddr + numWords) > GET_USTORE_SIZE(ae)) 
    {
       return (HALAE_BADARG);
    }   
    if((status = isAeEnabled(ae)) != HALAE_DISABLED) 
    {
       return (status);
    }   

    /* determine whether it neighbour AE runs in shared control store status */
    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    shCtlStoreFlag = csrVal & (0x1 << MMC_SHARE_CS_BITPOS);
    if(shCtlStoreFlag) 
    {
       halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
       if(isAeActive(aeNeigh)) 
       {
           return (HALAE_NEIGHAEACTIVE); 
       }
    }

    ustoreAddr = GET_AE_CSR(ae, USTORE_ADDRESS);    /* save the uaddress */
    uAddr |= UA_ECS;                                /* enable ecs bit */
    SET_AE_CSR_NOCHK(ae, USTORE_ADDRESS, uAddr);    /* set the uaddress */
    for(ii = 0; ii < numWords; ii++)
    {
        /*
        Data is stored in ustore data register as: 
            Uword1[9] parity for data bits [31:16], 
            Uword1[8] parity for data bits [15:0]
            Uword1[7:4] 4'b1111, Uword1[3:0] Data[31:28]
            Uword0[31:20] Data[27:16], Uword0[19:18] 2'b11, 
            Uword0[17:10] Data[15:8], Uword0[9:8] 2'b11, Uword0[7:0] Data[7:0]
        */
        uwrdLo =  ((data[ii] & 0xfff0000) << 4) | (0x3 << 18) | \
                   ((data[ii] & 0xff00) << 2) | (0x3 << 8) | (data[ii] & 0xff);
        uwrdHi = (0xf << 4) | ((data[ii] & 0xf0000000) >> 28);

        /* set parity bits -- even-parity including the parity bit */
        uwrdHi |= (numBitsSet(data[ii] & 0xffff) & 0x1) << 8;
        tmp = ((data[ii] >> 16) & 0xffff);
        uwrdHi |= (numBitsSet(tmp) & 0x1) << 9;

        SET_AE_CSR_NOCHK(ae, USTORE_DATA_LOWER, uwrdLo);
        /* this will auto increment the address */
        SET_AE_CSR_NOCHK(ae, USTORE_DATA_UPPER, uwrdHi);    
    }

    SET_AE_CSR(ae, USTORE_ADDRESS, ustoreAddr); /* clear ecs */
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutUmem
   Description: Write a number of long-words to micro-store.  The micro engine 
                must be inactive, possible under reset, before writing 
                to micro-store.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_ENABLED, HALAE_BADARG, 
                HALAE_RESET, HALAE_FAIL, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutUmem(unsigned char ae, 
              unsigned int uAddr, 
              unsigned int numWords,
              unsigned int *data)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(data);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    /* For data, phys==virt for uAddr */
    stat = putUmem(ae, uAddr, numWords, data);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_ETRingInit
   Description: To init a eagletail ring
   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_FAIL
                
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_ETRingInit(unsigned int ringNum, 
                 EAGLETAIL_RING_NEAR_WATERMARK nfwm, 
                 EAGLETAIL_RING_NEAR_WATERMARK newm, 
                 EAGLETAIL_RING_SIZE size, 
                 uint64 baseAddr)
{
    unsigned int ringCfg, ringBase;
    
    HALAE_VERIFY_LIB();

    if (ringNum >= MAX_EAGLETAIL_RING_ENTRIES) 
    {
        return (HALAE_BADARG);    
    }    

    /* nfwm & newm must be less than 1/2 the ring size */ 
    if(EAGLETAIL_RING_NEAR_WATERMARK_TO_BYTES(nfwm) > \
                                    (EAGLETAIL_RING_SIZE_TO_BYTES(size) / 2)) 
    {
       return (HALAE_BADARG);
    }   
    if(EAGLETAIL_RING_NEAR_WATERMARK_TO_BYTES(newm) > \
                                    (EAGLETAIL_RING_SIZE_TO_BYTES(size) / 2)) 
    {
       return (HALAE_BADARG);
    }   

    if(baseAddr & ~(0xFFFFFFFF << ((unsigned int)size + 6))) 
    {
       return (HALAE_BADARG);
    }   
    
    SPIN_LOCK_RING(ringNum, lock_level);
    /* baseAddr should align to ring size. bits[29:0] of the RingBase register
       correspond to bits[35:6]of the Ring's Memory Address. */
    ringBase = (unsigned int)((baseAddr & 0x0000000FFFFFFFFFull) >> 6);
    ringBase = ringBase &  (0xFFFFFFFF << (unsigned int)size);
            
    /* write ring config CSR*/          
    ringCfg = (nfwm << 10) | (newm << 5) | size;
    EAGLETAIL_RING_CSR_WRITE(ringNum, ET_RING_CONFIG, ringCfg);
      
    /* write side-effect: head and tail will be cleared to zero */
    EAGLETAIL_RING_CSR_WRITE(ringNum, ET_RING_BASE, ringBase);
    
    /* shadow copy of ring size and base address */
    ringSize[ringNum] = EAGLETAIL_RING_SIZE_TO_BYTES(size);
    ringBaseAddr[ringNum] = ringBase;
    ringTail[ringNum] = 0;
    ringHead[ringNum] = 0;
    ringPutSpace[ringNum] = ringSize[ringNum]/4;
    ringGetSpace[ringNum] = 0;
    ringType[ringNum] = -1;       
            
    SPIN_UNLOCK_RING(ringNum, lock_level);
    
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
Function:    halAe_ETRingSetType
Description: To set ring type to specified eagletail ring.
Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB

Uses:
Modifies:
-----------------------------------------------------------------------------*/
HAL_DECLSPEC int 
halAe_ETRingSetType(unsigned int ringNum, 
                    RING_TYPE type)
{
    HALAE_VERIFY_LIB();
    if (ringNum >= MAX_EAGLETAIL_RING_ENTRIES) 
    {
        return (HALAE_BADARG);    
    }    

    SPIN_LOCK_RING(ringNum, lock_level);        
    ringType[ringNum] = type;       
    SPIN_UNLOCK_RING(ringNum, lock_level);
    
    return (HALAE_SUCCESS);    
}

/*-----------------------------------------------------------------------------
Function:    halAe_ETRingPut
Description: To put data to specified eagletail ring.
Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_RINGFULL

Uses:
Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_ETRingPut(unsigned int ringNum, 
                unsigned int *in_data, 
                unsigned int *count, 
                EAGLETAIL_RING_IA_ACCESS_METHOD mode,
                unsigned int *count_available)
{
    unsigned int ii=0;
    uint64 baseAddr, offset;
    unsigned int request_count;
    
    uint64 ncBase_p, cBase_p, sBase_p;    
    unsigned int tailptr, headptr;
    unsigned int size;
    int depth;
    
    HALAE_VERIFY_LIB();

    if(ringNum >= MAX_EAGLETAIL_RING_ENTRIES) 
    {
       return (HALAE_BADARG);    
    }   
    if((!in_data) || (!count)) 
    {
       return (HALAE_BADARG);    
    }   
    if(count_available != NULL)
    {
       *count_available = 0;
    }    
    if(*count == 0)
    {
       if(count_available != NULL)
       {
          *count_available = ringPutSpace[ringNum];
       }            
       return (HALAE_SUCCESS);
    }   
    request_count = *count;
    *count = 0;    

    SPIN_LOCK_RING(ringNum, lock_level);   
    
    /* use shadow tail pointer for IA-put-only ring */
    tailptr = ringTail[ringNum];
    if(ringType[ringNum] == RINGTYPE_IAPUTGET) 
    {
       tailptr = EAGLETAIL_RING_CSR_READ(ringNum, ET_RING_TAIL_OFFSET);    
    }

    /* use shadow copy of ring size */
    size = ringSize[ringNum];
    
    /* use shadow copy of put space if possible */
    if((ringPutSpace[ringNum] < request_count) || (ringType[ringNum] == RINGTYPE_IAPUTGET))
    {
        headptr = EAGLETAIL_RING_CSR_READ(ringNum, ET_RING_HEAD_OFFSET);

        if(tailptr == headptr) 
        {
           if(EAGLETAIL_RING_IS_FULL_STATUS(ringNum)) 
           { 
              SPIN_UNLOCK_RING(ringNum, lock_level);
              return (HALAE_RINGFULL);
           }
           headptr = EAGLETAIL_RING_CSR_READ(ringNum, ET_RING_HEAD_OFFSET);           
        }   
                
        depth = headptr - tailptr;
        if(depth <= 0 ) 
        {
           depth = depth + size;
        }    
        depth = depth/4;

        /* update shadow put space */
        ringPutSpace[ringNum] = depth;
        
        if(depth < (int)request_count) 
        { 
           request_count = depth;           
        }        
    }

    if(mode == ET_RING_MMIO_ACCESS)
    {
        for(ii = 0; ii < request_count; ii++) 
        { 
            EAGLETAIL_RING_PUT(ringNum, in_data[ii]);
        }    
        /* upate shadow copy of tail pointer */
        ringTail[ringNum] = (tailptr + 4*request_count) % size;
        /* update shadow copy of put space */
        ringPutSpace[ringNum] -= request_count;        
        *count = request_count;         
        if(count_available != NULL)
        {
            *count_available = ringPutSpace[ringNum];
        }            
    }
    else
    {
        /* IA shared memory access mode, only support coherent dram and OS managed coherent dram */
        /* use shadow copy of ring base address */
        baseAddr = ringBaseAddr[ringNum];
        baseAddr = (unsigned int)((baseAddr & 0x0000000FFFFFFFFFull) << 6);       

        /* None-Coherent dram physical base address */
        ncBase_p = SysMemInfo.dramDesc[0].dramBaseAddr;
        /* Coherent dram physical base address  */       	  
        cBase_p = SysMemInfo.dramDesc[1].dramBaseAddr;
        /* sram physical base address  */       	  
        sBase_p = 0xfffc0000; 

        if(!(((baseAddr+size) <= ncBase_p) || (baseAddr >= ncBase_p + SysMemInfo.dramDesc[0].aeDramSize)))
        {
            /* shared memory access non-coherent dram */
            SPIN_UNLOCK_RING(ringNum, lock_level);
            return (HALAE_FAIL);
        }
        else if(!(((baseAddr+size) <= sBase_p) || (baseAddr >= sBase_p + SysMemInfo.sramChan[0].sramSize)))
        {
            /* shared memory access sram */
            SPIN_UNLOCK_RING(ringNum, lock_level);
            return (HALAE_FAIL);
        }
        else if((baseAddr >= cBase_p) && ((baseAddr+size) <= cBase_p + SysMemInfo.dramDesc[1].aeDramSize)) 
        {
            /* shared memory access coherent dram */
            offset = baseAddr - cBase_p;
            for(ii = 0; ii < request_count; ii++) 
            {
                DRAM_WRITE_XA(1, offset + tailptr, in_data[ii]);
                tailptr += 4;                    
                if(tailptr >= size) 
                {
                    tailptr -= size;
                }    
            }                    
            /* update tail pointer */
            EAGLETAIL_RING_CSR_WRITE(ringNum, ET_RING_TAIL_OFFSET, tailptr);
            ringTail[ringNum] = tailptr;
            /* update shadow copy of put space */
            ringPutSpace[ringNum] -= request_count;
            *count = request_count;         
            if(count_available != NULL)
            {
               *count_available = ringPutSpace[ringNum];
            }                            
        }
        else
        {
            /* shared memory access OS managed coherent dram */
            if(halAe_ringPut(baseAddr, size, tailptr, in_data, request_count)) 
            {
                SPIN_UNLOCK_RING(ringNum, lock_level);
                return (HALAE_FAIL);
            }
            
            /* update tail pointer */
            tailptr = (tailptr+(request_count*4))%size;
            EAGLETAIL_RING_CSR_WRITE(ringNum, ET_RING_TAIL_OFFSET, tailptr);
            ringTail[ringNum] = tailptr;
            /* update shadow copy of put space */
            ringPutSpace[ringNum] -= request_count;          
            *count = request_count;         
            if(count_available != NULL)
            {
               *count_available = ringPutSpace[ringNum];
            }                          
        }
    }
    SPIN_UNLOCK_RING(ringNum, lock_level);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
Function:    halAe_ETRingGet
Description: To get data from specified eagletail ring.
Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_RINGEMPTY

Uses:
Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_ETRingGet(unsigned int ringNum, 
                unsigned int *out_data, 
                unsigned int *count, 
                EAGLETAIL_RING_IA_ACCESS_METHOD mode,
                unsigned int *count_available)
{
    unsigned int ii=0;
    uint64 baseAddr, offset;
    unsigned int request_count;
    
    uint64 ncBase_p, cBase_p, sBase_p;       
    unsigned int headptr, tailptr;    
    unsigned int size;    
    int depth;
    
    if(ringNum >= MAX_EAGLETAIL_RING_ENTRIES) 
    {
       return (HALAE_BADARG);    
    }   
    if((!out_data) || (!count)) 
    {
       return (HALAE_BADARG);    
    }   
    if(count_available != NULL)
    {
       *count_available = 0;
    }
    if(*count == 0) 
    {
       if(count_available != NULL)
       {
          *count_available = ringGetSpace[ringNum];
       }
       return (HALAE_SUCCESS);
    }    
    request_count = *count;
    *count = 0;    

    SPIN_LOCK_RING(ringNum, lock_level);   

    /* use shadow copy of ring size */
    size = ringSize[ringNum];

    /* use shadow copy head pointer for IA-get-only ring */
    headptr = ringHead[ringNum];
    if(ringType[ringNum] == RINGTYPE_IAPUTGET) 
    {
       headptr = EAGLETAIL_RING_CSR_READ(ringNum, ET_RING_HEAD_OFFSET);
    }
    
    /* use shadow copy of get space if possible */
    if((ringGetSpace[ringNum] < request_count) || (ringType[ringNum] == RINGTYPE_IAPUTGET))  
    {
        tailptr = EAGLETAIL_RING_CSR_READ(ringNum, ET_RING_TAIL_OFFSET);
        
        if(tailptr == headptr) 
        {
           if(EAGLETAIL_RING_IS_EMPTY_STATUS(ringNum)) 
           { 
              SPIN_UNLOCK_RING(ringNum, lock_level);
              return (HALAE_RINGEMPTY);
           }
           tailptr = EAGLETAIL_RING_CSR_READ(ringNum, ET_RING_TAIL_OFFSET);
        }   
                    
        depth = tailptr - headptr;
        if(depth <= 0 ) 
        {
           depth = depth + size;
        }
        depth = depth/4;
        
        /* update shadow copy of get space */
        ringGetSpace[ringNum] = depth;

        if(depth < (int)(request_count)) 
        { 
           request_count = depth;           
        }        
    }
        
    if(mode == ET_RING_MMIO_ACCESS)
    {    
        for(ii = 0; ii < request_count; ii++) 
        {
            out_data[ii] = EAGLETAIL_RING_GET(ringNum);
        }    
        /* upate shadow copy of head pointer */
        ringHead[ringNum] = (headptr + 4*request_count) % size;   
        /* update shadow copy of get space */
        ringGetSpace[ringNum] -= request_count;
        *count = request_count;         
        if(count_available != NULL)
        {
            *count_available = ringGetSpace[ringNum];
        }    
    }
    else
    {
        /* IA shared memory access mode, only support coherent dram and OS managed coherent dram */
        /* use shadow copy of ring base address */
        baseAddr = ringBaseAddr[ringNum];
        baseAddr = (unsigned int)((baseAddr & 0x0000000FFFFFFFFFull) << 6);       

        /* None-Coherent dram physical base address */
        ncBase_p = SysMemInfo.dramDesc[0].dramBaseAddr;
        /* Coherent dram physical base address  */       	  
        cBase_p = SysMemInfo.dramDesc[1].dramBaseAddr;
        /* sram physical base address  */       	  
        sBase_p = 0xfffc0000; 

        if(!(((baseAddr+size) <= ncBase_p) || (baseAddr >= ncBase_p + SysMemInfo.dramDesc[0].aeDramSize)))
        {
            /* shared memory access non-coherent dram */
            SPIN_UNLOCK_RING(ringNum, lock_level);
            return (HALAE_FAIL);
        }
        else if(!(((baseAddr+size) <= sBase_p) || (baseAddr >= sBase_p + SysMemInfo.sramChan[0].sramSize)))
        {
            /* shared memory access sram */
            SPIN_UNLOCK_RING(ringNum, lock_level);
            return (HALAE_FAIL);
        }
        else if((baseAddr >= cBase_p) && ((baseAddr+size) <= cBase_p + SysMemInfo.dramDesc[1].aeDramSize)) 
        {
            /* shared memory access coherent dram */
            offset = baseAddr - cBase_p; 
            for(ii = 0; ii < request_count; ii ++) 
            {
                out_data[ii] = DRAM_READ_CH1(offset + headptr);
                headptr += 4;                    
                if(headptr >= size) 
                {
                    headptr -= size;
                }
            }
            /* update head pointer */
            EAGLETAIL_RING_CSR_WRITE(ringNum, ET_RING_HEAD_OFFSET, headptr);
            /* upate shadow copy of head pointer */
            ringHead[ringNum] = headptr;
            /* update shadow copy of get space */
            ringGetSpace[ringNum] -= request_count;
            *count = request_count;      
            if(count_available != NULL)
            {
               *count_available = ringGetSpace[ringNum];
            }    
        }
        else
        {               
            if(halAe_ringGet(baseAddr, size, headptr, out_data, request_count)) 
            {   
                *count = 0;
                SPIN_UNLOCK_RING(ringNum, lock_level);
                return (HALAE_FAIL);
            }
                
            /* update head pointer */
            headptr = (headptr+(request_count*4))%size;
            EAGLETAIL_RING_CSR_WRITE(ringNum, ET_RING_HEAD_OFFSET, headptr);		
            /* upate shadow copy of head pointer */
            ringHead[ringNum] = headptr;
            /* update shadow copy of get space */
            ringGetSpace[ringNum] -= request_count;  
            *count = request_count;           
            if(count_available != NULL)
            {
               *count_available = ringGetSpace[ringNum];
            }       
        }         
    }
    SPIN_UNLOCK_RING(ringNum, lock_level);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCtxArb
   Description: Read the accelEngine context arbitrition-control register.
   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCtxArb(unsigned char ae, 
                unsigned int *ctxArbCtl)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(ctxArbCtl);
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = getAeCsr(ae, CTX_ARB_CNTL, ctxArbCtl);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCtxArb
   Description: Write the accelEngine context arbitrition-control register.
   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCtxArb(unsigned char ae, 
                unsigned int ctxArbCtl)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    SPIN_LOCK_AE(ae);
    stat = putAeCsr(ae, CTX_ARB_CNTL, ctxArbCtl);
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    execMicroInst
   Description: Execute a list of micro-instructions, in the specified context,
                and then restore the state to the previous context. The code
                to be executed must perform a ctx_arb in order for this 
                function to terminate sucessfully.
   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED,
                HALAE_RESET, HALAE_FAIL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
execMicroInst(unsigned char ae, 
              unsigned char ctx, 
              uword_T *microInst,
              unsigned int numInst, 
              int condCodeOff,
              unsigned int maxCycles, 
              unsigned int *endPC)
{
    unsigned int    savCC, wakeupEvents, savCtx, savPc, ctxArbCtl, ctxEnables;
    uword_T            savUwords[MAX_EXEC_INST];
    int                status = HALAE_SUCCESS;
    unsigned int csrVal, newCsrVal, shCtlStoreFlag;
    unsigned char aeNeigh;
    
    VERIFY_AE(ae);
    if(isAeActive(ae)) 
    {
       return (HALAE_AEACTIVE);
    }   
    if((numInst > MAX_EXEC_INST) || !microInst || (ctx >= sizeof(setMasks)/sizeof(unsigned int))) 
    {
       return (HALAE_BADARG);
    }   
    if(IS_RST(ae)) 
    {
       return (HALAE_RESET);
    }   

    /* backup shared control store bit, and force AE to 
       none-shared mode before executing ucode snippet */
    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    shCtlStoreFlag = csrVal & (0x1 << MMC_SHARE_CS_BITPOS);
    if(shCtlStoreFlag) 
    {
       halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
       if(isAeActive(aeNeigh)) 
       {
          return (HALAE_NEIGHAEACTIVE); 
       }
    }
    newCsrVal = CLR_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    SET_AE_CSR(ae, AE_MISC_CONTROL, newCsrVal);

    /* save current states: */
    if((status = getUwords(ae, TmpUaddr, numInst, savUwords)) != HALAE_SUCCESS) 
    {
        SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);        
        return (status);   /* instructions at loc 0 thru numInst */
    }    

    /* save wakeup-events */
    getCtxWakeupEvents(ae, ctx, &wakeupEvents); 
    /* save PC */
    getCtxIndrCsr(ae, ctx, CTX_STS_INDIRECT, &savPc);    
    savPc = (savPc & UpcMask) >> ICS_CTX_PC_BITPOS;

    /* save ctx enables */
    ctxEnables = IGNORE_W1C_MASK & GET_AE_CSR(ae, CTX_ENABLES);
    /* save conditional-code */
    savCC = GET_AE_CSR(ae, CC_ENABLE);
    /* save current context */
    savCtx = GET_AE_CSR(ae, ACTIVE_CTX_STATUS);
    ctxArbCtl = GET_AE_CSR(ae, CTX_ARB_CNTL);

    /* turn off ucode parity */
    SET_AE_CSR(ae, CTX_ENABLES, ctxEnables & \
              (~(1 << CE_CNTL_STORE_PARITY_ENABLE_BITPOS)));

    /* copy instructions to ustore */
    putUwords(ae, TmpUaddr, numInst, microInst);        
    /* set PC */    
    putCtxIndrCsr(ae, setMasks[ctx], CTX_STS_INDIRECT, UpcMask & TmpUaddr);
    /* change the active context */
    SET_AE_CSR(ae, ACTIVE_CTX_STATUS, ctx & ACS_ACNO);

    if(condCodeOff) 
    {
        /* disable conditional-code*/        
        SET_AE_CSR(ae, CC_ENABLE, savCC & 0xffffdfff);    
    }
    /* wakeup-event voluntary */
    putCtxWakeupEvents(ae, setMasks[ctx], XCWE_VOLUNTARY);    
    /* enable context */
    enableCtx(ae, setMasks[ctx]);

    /* wait for it to finish */
    if(waitNumSimCycles(ae, maxCycles, 1)) 
    {
        status = HALAE_FAIL;
    }

    /* see if we need to get the current PC */
    if(endPC)
    {
        unsigned int ctxStatus;
        getCtxIndrCsr(ae, ctx, CTX_STS_INDIRECT, &ctxStatus);
        *endPC = ctxStatus & UpcMask;
    }

    /* retore to previous states: */
     /* disable context */
    disableCtx(ae, setMasks[ctx]);                      
    /* instructions */ 
    putUwords(ae, TmpUaddr, numInst, savUwords);        
    /* wakeup-events */
    putCtxWakeupEvents(ae, setMasks[ctx], wakeupEvents);
    putCtxIndrCsr(ae, setMasks[ctx], CTX_STS_INDIRECT, UpcMask & savPc);

    /* only restore shared control store bit, 
       other bit might be changed by AE code snippet */
    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    if(shCtlStoreFlag) 
    {
       newCsrVal = SET_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    }   
    else 
    {
       newCsrVal = CLR_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    }   
    SET_AE_CSR(ae, AE_MISC_CONTROL, newCsrVal);
    /* ctx-enables */    
    SET_AE_CSR(ae, CTX_ENABLES, ctxEnables); 
    /* conditional-code */    
    SET_AE_CSR(ae, CC_ENABLE, savCC);
    /* change the active context */    
    SET_AE_CSR(ae, ACTIVE_CTX_STATUS, savCtx & ACS_ACNO);
    /* restore the nxt ctx to run */    
    SET_AE_CSR(ae, CTX_ARB_CNTL, ctxArbCtl);

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    getRelDataReg_Common
   Description: Read a long-word from a GPR, read-xfer, and LM index registers.
                It's unsafe to called this function while the AE is enabled.
                Type is one of: ICP_GPA_REL, ICP_GPB_REL, ICP_SR_REL, 
                                ICP_SR_RD_REL, ICP_DR_REL, ICP_DR_RD_REL, 
                                ICP_LMEM0, ICP_LMEM1.
                Ctx = 0-7/0-6 (even).

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_RESET
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
getRelDataReg_Common(unsigned char ae, 
                     unsigned char ctx, 
                     icp_RegType_T regType,
                     unsigned short regNum, 
                     unsigned int *data)
{
    unsigned int savCtx, uAddr, uwrdLo, uwrdHi;
    unsigned int ctxArbCntl, ctxEnables, ustoreAddr;
    unsigned short    regAddr;
    int status = HALAE_SUCCESS;
    uword_T            inst, savUword;
    unsigned int csrVal, newCsrVal, shCtlStoreFlag;
    unsigned char aeNeigh;
    
    if ((regType != ICP_GPA_REL) && (regType != ICP_GPB_REL) &&
        (regType != ICP_SR_REL) && (regType != ICP_SR_RD_REL) &&
        (regType != ICP_DR_REL) && (regType != ICP_DR_RD_REL) &&
        (regType != ICP_LMEM0) && (regType != ICP_LMEM1)) 
    {
        return (HALAE_BADARG);
    }
    if(IS_RST(ae)) 
    {
        return (HALAE_RESET);
    }    

    if((regAddr = getReg10bitAddr(regType, regNum)) == BAD_REGADDR) 
    {
        return (HALAE_BADARG);
    }    

    /* instruction -- alu[--, --, B, reg] */
    switch(regType)
    {
    case ICP_GPA_REL:            /* A rel source */
        inst = 0xA070000000ull | (regAddr & 0x3ff);
        break;
    default:
        inst = ((uword_T)0xA030000000ull | ((regAddr & 0x3ff) << 10));
        break;
    }

    /* backup shared control store bit, and force AE to 
    none-shared mode before executing ucode snippet */
    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    shCtlStoreFlag = csrVal & (0x1 << MMC_SHARE_CS_BITPOS);
    if(shCtlStoreFlag) 
    {
        halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
        if(isAeActive(aeNeigh)) 
        {
            return (HALAE_NEIGHAEACTIVE); 
        }
    }

    newCsrVal = CLR_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    SET_AE_CSR(ae, AE_MISC_CONTROL, newCsrVal);

    /* read current context */
    getAeCsr(ae, ACTIVE_CTX_STATUS, &savCtx);
    getAeCsr(ae, CTX_ARB_CNTL, &ctxArbCntl);

    getAeCsr(ae, CTX_ENABLES, &ctxEnables);
    /* prevent clearing the W1C bits: the breakpoint bit, 
    ECC error bit, and Parity error bit */
    ctxEnables &= IGNORE_W1C_MASK;            

    /* change the context */
    if(ctx != (savCtx & ACS_ACNO)) 
    {
        putAeCsr(ae, ACTIVE_CTX_STATUS, ctx & ACS_ACNO);
    }
    /* save a ustore location */
    if((status = getUwords(ae, TmpUaddr, 1, &savUword)) != HALAE_SUCCESS)
    {
        /* restore AE_MISC_CONTROL csr */
        SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);    
        return (status);
    }

    /* turn off ustore parity */
    putAeCsr(ae, CTX_ENABLES, ctxEnables & \
        (~(1 << CE_CNTL_STORE_PARITY_ENABLE_BITPOS)));

    /* save ustore-addr csr */
    ustoreAddr = GET_AE_CSR(ae, USTORE_ADDRESS);            

    /* write the ALU instruction to ustore, enable ecs bit */
    uAddr = TmpUaddr | UA_ECS;                            

    /* set the uaddress */
    putAeCsr(ae, USTORE_ADDRESS, uAddr);                    
    if (PrdMinType == HWID_ICP)
    {
        inst = setUwordECC(inst);
    } 

    uwrdLo = (unsigned int)(inst & 0xffffffff);
    uwrdHi = (unsigned int)(inst >> 32);


    putAeCsr(ae, USTORE_DATA_LOWER, uwrdLo);

    /* this will auto increment the address */
    putAeCsr(ae, USTORE_DATA_UPPER, uwrdHi);

    /* set the uaddress */
    putAeCsr(ae, USTORE_ADDRESS, uAddr);

    /* delay for at least 8 cycles */
    waitNumSimCycles(ae, 8, 0);

    /* read ALU output -- the instruction should have been executed
    prior to clearing the ECS in putUwords */
    getAeCsr(ae, ALU_OUT, data);                            

    /* restore ustore-addr csr */
    SET_AE_CSR(ae, USTORE_ADDRESS, ustoreAddr);     

    /* restore the ustore */
    if((status = putUwords(ae, TmpUaddr, 1, &savUword)) != HALAE_SUCCESS)
    {
        /* restore AE_MISC_CONTROL csr */
        SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);        
        return (status);
    }

    /* restore the context */
    if(ctx != (savCtx & ACS_ACNO)) 
    {
        putAeCsr(ae, ACTIVE_CTX_STATUS, savCtx & ACS_ACNO);
    }    
    putAeCsr(ae, CTX_ARB_CNTL, ctxArbCntl);
    putAeCsr(ae, CTX_ENABLES, ctxEnables);

    /* restore AE_MISC_CONTROL csr */
    SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);    
    halAe_WaitUStoreAddrReady(ae);           

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    getRelNNReg_Common
   Description: Read a long-word from a next-neigh registers.
                It's unsafe to called this function while the AE is enabled.
                Type is ICP_NEIGH_REL.
                Ctx = 0-7/0-6 (even).

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_RESET
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
getRelNNReg_Common(unsigned char ae, 
                   unsigned char ctx, 
                   icp_RegType_T regType,
                   unsigned short regNum,
                   unsigned int *data)
{
    uword_T inst[] = {
        0x0B000000000ull,        /* alu[gpr, --, b, n$reg] */        
        0x0F0000C0300ull,        /* nop */
        0x0E000010000ull         /* ctx_arb[kill] */
    };
    unsigned short    regAddr;
    int stat = HALAE_SUCCESS;
    const int num_inst = sizeof(inst)/sizeof(inst[0]), condCodeOff = 1;
    unsigned int gprB0;

    if(regType != ICP_NEIGH_REL) 
    {
       return (HALAE_BADARG);
    }   
    if(IS_RST(ae)) 
    {
       return (HALAE_RESET);
    }   

    /* backup the value of gpr reg */
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, &gprB0);

    /* get the 10-bit address of the destination register */
    if((regAddr = getReg10bitAddr(regType, regNum)) == BAD_REGADDR) 
    {
        return (HALAE_BADARG);
    }    

    /* fixup neighbor csr address */
    inst[0] = ((uword_T)inst[0] | ((regAddr & 0x3ff) << 10));

    stat = execMicroInst(ae, ctx, inst, num_inst, condCodeOff, 
                         num_inst*5, NULL);

    /* read the value from gpr */
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, data);

    /* restore the gpr reg */
    putRelDataReg(ae, ctx, ICP_GPB_REL, 0, gprB0);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    putRelDataReg_Common
   Description: Write a long-word to a GPR or LM index register. This logic 
                cannot work for neighbor or transfer registers.
                The AE must be disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_ENABLED, 
                HALAE_FAIL, HALAE_RESET
                Type is one of: ICP_GPA_REL, ICP_GPB_REL, ICP_LMEM0, ICP_LMEM1.
                Ctx = 0-7/0, 2, 4, 6.
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
putRelDataReg_Common(unsigned char ae, 
                     unsigned char ctx, 
                     icp_RegType_T regType, 
                     unsigned short regNum, 
                     unsigned int data)
{
    unsigned short srcHiAddr, srcLoAddr, destAddr, data16Hi, data16Lo;
    uword_T inst[] = 
    {
        0x0F440000000ull,        /* immed_w1[reg, val_hi16] */
        0x0F040000000ull,        /* immed_w0[reg, val_lo16] */
        0x0F0000C0300ull,        /* nop */
        0x0E000010000ull         /* ctx_arb[kill] */
    };
    const int num_inst = sizeof(inst)/sizeof(inst[0]), condCodeOff = 1;
    const int imm_w1=0, imm_w0=1;

    /* This logic only works for GPRs and LM index registers, 
       not NN or XFER registers! */
    if((regType != ICP_GPA_REL) && (regType != ICP_GPB_REL) &&
        (regType != ICP_LMEM0) && (regType != ICP_LMEM1)) 
    {
        return (HALAE_BADARG);
    }     

    if(IS_RST(ae)) 
    {
       return (HALAE_RESET);
    }   

    /* get the 10-bit address of the destination register */
    if((destAddr = getReg10bitAddr(regType, regNum)) == BAD_REGADDR) 
    {
        return (HALAE_BADARG);
    }    

    data16Lo = 0xffff & data;
    data16Hi = 0xffff & (data >> 16);
    srcHiAddr = getReg10bitAddr(ICP_NO_DEST, 
        (unsigned short)(0xff & data16Hi));
    srcLoAddr = getReg10bitAddr(ICP_NO_DEST, 
        (unsigned short)(0xff & data16Lo));

    switch(regType)
    {
    case ICP_GPA_REL:            /* A rel source */
        inst[imm_w1] = inst[imm_w1] | ((data16Hi >> 8) << 20) |
            ((srcHiAddr & 0x3ff) << 10) | (destAddr & 0x3ff);

        inst[imm_w0] = inst[imm_w0] | ((data16Lo >> 8) << 20) | 
            ((srcLoAddr & 0x3ff) << 10) | (destAddr & 0x3ff);
        break;

    default:
        inst[imm_w1] = inst[imm_w1] | ((data16Hi >> 8) << 20) |
            ((destAddr & 0x3ff) << 10) | (srcHiAddr & 0x3ff);

        inst[imm_w0] = inst[imm_w0] | ((data16Lo >> 8) << 20) | 
            ((destAddr & 0x3ff) << 10) | (srcLoAddr & 0x3ff);
        break;
    }

    return (execMicroInst(ae, ctx, inst, num_inst, condCodeOff, 
                          num_inst*5, NULL));  
}

/*-----------------------------------------------------------------------------
   Function:    getRelRdXfer
   Description: Read a long-word from read-xfer register. The AE must be 
                disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getRelRdXfer(unsigned char ae, 
             unsigned char ctx, 
             icp_RegType_T regType,
             unsigned short regNum, 
             unsigned int *data)
{
    int status = HALAE_SUCCESS;
    unsigned int ctx_enables;
    unsigned short mask;

    if((regType != ICP_SR_REL) && (regType != ICP_SR_RD_REL) &&
        (regType != ICP_DR_REL) && (regType != ICP_DR_RD_REL)) 
    {
        return (HALAE_BADARG);
    }
    /* determine the context mode */
    status = getAeCsr(ae, CTX_ENABLES, &ctx_enables);
    if(CE_INUSE_CONTEXTS & ctx_enables)
    {
        if(ctx & 0x1)
        {
           return (HALAE_BADARG);
        }
        mask = 0x1f;
    }
    else 
    {
        mask = 0x0f;
    }    
    if (regNum & ~mask) 
    {
        return HALAE_BADARG;
    }
    status = getRelDataReg(ae, ctx, regType, regNum, data);
    return (status);
}


/*-----------------------------------------------------------------------------
   Function:    putRelRdXfer
   Description: Write a long-word to read-xfer register.The AE must be disabled
                prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
putRelRdXfer(unsigned char ae, 
             unsigned char ctx, 
             icp_RegType_T regType,
             unsigned short regNum, 
             unsigned int val)
{
    int status = HALAE_SUCCESS;
    unsigned char regAddr;
    unsigned int ctx_enables;
    unsigned short mask;
    unsigned short dr_offset = 16;

    if ((regType != ICP_SR_REL) && (regType != ICP_DR_REL) &&
        (regType != ICP_SR_RD_REL) && (regType != ICP_DR_RD_REL)) 
    {
        return (HALAE_BADARG);
    }
    /* determine the context mode */
    status = getAeCsr(ae, CTX_ENABLES, &ctx_enables);
    if(CE_INUSE_CONTEXTS & ctx_enables)
    {
        if(ctx & 0x1)
        {
           return (HALAE_BADARG);
        }
        mask = 0x1f;
        dr_offset = 32;
    }
    else 
    {
        mask = 0x0f;
    }    
    if (regNum & ~mask) 
    {
        return HALAE_BADARG;
    }
    regAddr = regNum + (ctx << 5);

    switch(regType)
    {
    case ICP_SR_RD_REL:
    case ICP_SR_REL:
        SET_AE_XFER(ae, regAddr, val); break;

    case ICP_DR_RD_REL:
    case ICP_DR_REL:
        SET_AE_XFER(ae, regAddr + dr_offset, val); break;

    default: status = HALAE_BADARG; break;
    }

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    getRelWrXfers
   Description: Read a long-word from write-xfer register.  Reading and
                writing to/from transfer registers are relative from the
                accelEngine's perspective.  Therefore, when the core reads
                from the xfer register it's actually reading the write-transfer
                registers.  The AE must be disabled prior to
                calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
getRelWrXfers(unsigned char ae, 
              unsigned char ctx, 
              icp_RegType_T regType,
              unsigned short regNum, 
              unsigned int *data, 
              unsigned int count)
{
    int status = HALAE_SUCCESS;
    unsigned char regAddr;
    unsigned int ctx_enables;
    unsigned short regMask;
    unsigned int ii;
    unsigned short dr_offset = 16;

    if((regType != ICP_SR_REL) && (regType != ICP_DR_REL) &&
        (regType != ICP_SR_WR_REL) && (regType != ICP_DR_WR_REL)) 
    {
        return (HALAE_BADARG);
    }    

    /* determine the context mode */
    getAeCsr(ae, CTX_ENABLES, &ctx_enables);
    if(CE_INUSE_CONTEXTS & ctx_enables) 
    {
        /* four-ctx mode: 32 reg per ctx */
        if(ctx & 0x1) 
        {
           return (HALAE_BADARG);
        }   
        regMask = ~0x1f;
        dr_offset = 32;
    } 
    else 
    {
        /* eight-ctx mode: 16 reg per ctx */
        regMask = ~0xf;
    }
    if((regNum & regMask) || ((regNum + count-1) & regMask)) 
    {
        return HALAE_BADARG;
    }

    switch(regType)
    {
    case ICP_DR_WR_REL:
    case ICP_DR_REL : 
        regAddr = regNum + (ctx << 5);
        for (ii=0; ii<count; ii++)
            data[ii] = GET_AE_XFER(ae, regAddr+dr_offset+ii);
        break;

    case ICP_SR_WR_REL:
    case ICP_SR_REL :
        regAddr = regNum + (ctx << 5);
        for (ii=0; ii<count; ii++)
            data[ii] = GET_AE_XFER(ae, regAddr+ii);
        break;

    default: status = HALAE_BADARG; break;
    }

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    putRelWrXfer_Common
   Description: Write a long-word to a relative write-xfer reg.  This logic
                also supports neighbor registers.
                The AE must be disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
putRelWrXfer_Common(unsigned char ae, 
                    unsigned char ctx, 
                    icp_RegType_T regType,
                    unsigned short regNum, 
                    unsigned int data)
{
    unsigned int gprVal, ctx_enables;
    unsigned short srcHiAddr, srcLoAddr, gprAddr, xfrAddr, data16Hi, data16Lo;
    unsigned short regMask;
    int status = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        0x0F440000000ull,        /* immed_w1[reg, val_hi16] */
        0x0F040000000ull,        /* immed_w0[reg, val_lo16] */
        0x0A000000000ull,        /* alu[xferReg, --,b, reg] */
        0x0F0000C0300ull,        /* nop */
        0x0E000010000ull         /* ctx_arb[kill] */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned short gprNum = 0, dly=num_inst*5;

    if ((regType != ICP_SR_REL) && (regType != ICP_DR_REL) &&
        (regType != ICP_SR_WR_REL) && (regType != ICP_DR_WR_REL) &&
        (regType != ICP_NEIGH_REL)) 
    {
        return (HALAE_BADARG);
    }    

    if(IS_RST(ae)) 
    {
       return (HALAE_RESET);
    }   

    getAeCsr(ae, CTX_ENABLES, &ctx_enables);
    if(CE_INUSE_CONTEXTS & ctx_enables) 
    {
        /* four-ctx mode: 32 reg per ctx */
        if(ctx & 0x1)
        {
            return (HALAE_BADARG);
        }
        regMask = ~0x1f;
    } 
    else 
    {
        /* eight-ctx mode: 16 reg per ctx */
        regMask = ~0xf;
    }
    if(regNum & regMask) 
    {
        return HALAE_BADARG;
    }
    /* get the value of a b-bank gpr */
    getRelDataReg(ae, ctx, ICP_GPB_REL, gprNum, &gprVal);

    /* get the 10-bit address of the destination register */
    if((xfrAddr = getReg10bitAddr(regType, regNum)) == BAD_REGADDR) 
    {
        return (HALAE_BADARG);
    }    
    gprAddr = getReg10bitAddr(ICP_GPB_REL, gprNum);

    data16Lo = 0xffff & data;
    data16Hi = 0xffff & (data >> 16);
    srcHiAddr = getReg10bitAddr(ICP_NO_DEST, 
                                (unsigned short)(0xff & data16Hi));
    srcLoAddr = getReg10bitAddr(ICP_NO_DEST, 
                                (unsigned short)(0xff & data16Lo));

    /* fixup immed_wx[gpr, const] instruction for onstant and register */
    microInst[0] = microInst[0] | ((data16Hi >> 8) << 20) |
                   ((gprAddr & 0x3ff) << 10) | (srcHiAddr & 0x3ff);

    microInst[1] = microInst[1] | ((data16Lo >> 8) << 20) |
                   ((gprAddr & 0x3ff) << 10) | (srcLoAddr & 0x3ff);

    /* fixup alu[$xfer, --, b, gpr] instruction for source/dest register */
    microInst[2] = microInst[2] | \
                   ((xfrAddr & 0x3ff) << 20) | ((gprAddr & 0x3ff) << 10);

    status = execMicroInst(ae, ctx, microInst, num_inst, 
                           condCodeOff, dly, NULL);
    putRelDataReg(ae, ctx, ICP_GPB_REL, gprNum, gprVal);

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    putRelNN_Common
   Description: Write a long-word value to the AE's NN register. The AE must be
                disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
putRelNN_Common(unsigned char ae, 
                unsigned char ctx, 
                unsigned short nnNum,
                unsigned int value)
{
    unsigned int ctxEnables;
    int stat = HALAE_SUCCESS;

    if(nnNum >= MAX_NN_REG) 
    {
       return (HALAE_BADARG);
    }   

    /* make sure nn_mode is set to self */
    getAeCsr(ae, CTX_ENABLES, &ctxEnables);
    /* prevent clearing the W1C bits: the breakpoint bit, 
       ECC error bit, and Parity error bit */        
    ctxEnables &= IGNORE_W1C_MASK;
    putAeCsr(ae, CTX_ENABLES, ctxEnables | CE_NN_MODE);

    stat = putRelWrXfer(ae, ctx, ICP_NEIGH_REL, nnNum, value);
    putAeCsr(ae, CTX_ENABLES, ctxEnables);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutLM_Common
   Description: Write a long-word value to the AE's LM location specified by 
                the lmAddr word-address. It's unsafe to call this function while 
                the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutLM_Common(unsigned char ae, 
                   unsigned short lmAddr,
                   unsigned int value)
{
    unsigned int ctxEnables, lmAddr_sav, indirectLmAddr_sav;
    int stat = HALAE_SUCCESS;
    const unsigned char global=1, ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    if(lmAddr >= MaxLmemReg) 
    {
       return (HALAE_BADARG);
    }   

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_AEACTIVE);
    }    

    /* set LM mode global */
    getAeCsr(ae, CTX_ENABLES, &ctxEnables);
    /* prevent clearing the W1C bits: the breakpoint bit, 
       ECC error bit, and Parity error bit */        
    ctxEnables &= IGNORE_W1C_MASK;

    putAeLmMode(ae, ICP_LMEM0, global);

    /* save current LM addr */
    getCtxIndrCsr(ae, ctx, LM_ADDR_0_INDIRECT, &indirectLmAddr_sav);    
    getAeCsr(ae, LM_ADDR_0_ACTIVE, &lmAddr_sav);

    /* set LM addr */
    putAeCsr(ae, LM_ADDR_0_ACTIVE, lmAddr << 2);
    stat = putRelDataReg(ae, ctx, ICP_LMEM0, lmAddr, value);

    /* restore ctx_enables, and LM_ADDR */
    putAeCsr(ae, CTX_ENABLES, ctxEnables);
    putAeCsr(ae, LM_ADDR_0_ACTIVE, lmAddr_sav);
    putCtxIndrCsr(ae, (1 << ctx), LM_ADDR_0_INDIRECT, indirectLmAddr_sav);    

    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetLM_Common
   Description: Write a long-word value to the AE's LM location specified by
                the lmAddr word-address. 
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetLM_Common(unsigned char ae,
                   unsigned short lmAddr,
                   unsigned int *value)
{
    unsigned int ctxEnables, lmAddr_sav, indirectLmAddr_sav;
    int stat = HALAE_SUCCESS;
    const unsigned char global=1, ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(value);
    VERIFY_AE(ae);
    if(lmAddr >= MaxLmemReg) 
    {
       return (HALAE_BADARG);
    }   

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_AEACTIVE);
    }   

    /* set LM mode global */
    getAeCsr(ae, CTX_ENABLES, &ctxEnables);
    /* prevent clearing the W1C bits: the breakpoint bit, 
       ECC error bit, and Parity error bit */        
    ctxEnables &= IGNORE_W1C_MASK;
    putAeLmMode(ae, ICP_LMEM0, global);

    /* save current LM addr */
    getCtxIndrCsr(ae, ctx, LM_ADDR_0_INDIRECT, &indirectLmAddr_sav);    
    getAeCsr(ae, LM_ADDR_0_ACTIVE, &lmAddr_sav);

    /* set LM addr */
    putAeCsr(ae, LM_ADDR_0_ACTIVE, lmAddr << 2);

    stat = getRelDataReg(ae, ctx, ICP_LMEM0, lmAddr, value);

    /* restore ctx_enables, and LM_ADDR */
    putAeCsr(ae, CTX_ENABLES, ctxEnables);
    putAeCsr(ae, LM_ADDR_0_ACTIVE, lmAddr_sav);
    putCtxIndrCsr(ae, (1 << ctx), LM_ADDR_0_INDIRECT, indirectLmAddr_sav);    

    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutSharedRam_Common
   Description: Write a long-word value to the SSU Share Ram location specified
                by the addr. It's unsafe to call this function while the AE 
                is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutSharedRam_Common(unsigned char ae,
                          unsigned int addr, 
                          unsigned int value)
{
    unsigned int gprA0, gprA1, sr0;
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        /*
        .reg addr value
        .areg addr 0
        .areg value 1
        .reg $value
        .addr $value 0
        .sig sigWrite
        */
        0x0D800800010ull, /*0. br=ctx[0,l000_01#] */
        0x0D804400018ull, /*1. br[kill_context#] */
        0x0D803C00011ull, /*2. l000_01#: l000_end#: br!=ctx[0,l001_01#] */
        0x0F0400C0000ull, /*3. immed_w0[addr, 0x0] */
        0x0F4400C0000ull, /*4. immed_w1[addr, 0x0] */
        0x0F0400C0001ull, /*5. immed_w0[value, 0x0] */
        0x0F4400C0001ull, /*6. immed_w1[value, 0x0] */
        0x0A0580C0001ull, /*7. alu[$value, --, b, value] */
        0x08B00009200ull, /*8. alu_shf[addr, addr, or, 4, <<16] */
        0x02010008300ull, /*9. ssu_transfer[write, shared_ram,
                                            $value, addr, 0, 1], 
                                            sig_done[sigWrite] */
        0x0D802904213ull, /*10. nop_loop#: br_!signal[sigWrite, nop_loop#], 
                                                      defer[1] */
        0x0F0000C0300ull, /*11. nop */
        0x0F0000C0300ull, /*12. nop */
        0x0F0000C0300ull, /*13. nop */
        0x0F0000C0300ull, /*14. nop */
        0x0F0000C0300ull, /*15. end_of_program#: nop */
        0x0E000010000ull, /*16. ctx_arb[kill], any */
        0x0E000010000ull, /*17. kill_context#: ctx_arb[kill], any */
        0x0F0000C0300ull  /*18. nop */
    };
    
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    if(addr > MAX_SSU_SHARED_RAM - sizeof(unsigned int)) 
    {
       return (HALAE_BADARG);
    }   

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_AEACTIVE);
    }   

    /* fixup instruction for data and address */
    INSERT_IMMED_GPRA_CONST(microInst[3], (addr >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[4], (addr >> 16));
    INSERT_IMMED_GPRA_CONST(microInst[5], (value >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[6], (value >> 16));

    /* get and save the value of gpr and xfer_out reg */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelDataReg(ae, ctx, ICP_GPA_REL, 1, &gprA1);
    getRelWrXfers(ae, ctx, ICP_SR_WR_REL, 0, &sr0, 1);

    /* execute ssu_transfer instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                         condCodeOff, 600, NULL);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, 0, gprA0);
    putRelDataReg(ae, ctx, ICP_GPA_REL, 1, gprA1);
    putRelWrXfer(ae, ctx, ICP_SR_WR_REL, 0, sr0);

    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetSharedRam_Common
   Description: Read a long-word value from the SSU Share Ram location 
                specified by the addr. It's unsafe to call this function while 
                the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetSharedRam_Common(unsigned char ae, 
                          unsigned int addr, 
                          unsigned int *value)
{
    unsigned int gprA0, sr0;
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        /*
        .reg addr
        .areg addr 0
        .reg $value
        .addr $value 0
        .sig sigRead
        */
        0x0D800800010ull, /*0. br=ctx[0,l000_01#] */
        0x0D803C00018ull, /*1. br[kill_context#] */
        0x0D803400011ull, /*2. l000_01#: l000_end#: br!=ctx[0,l001_01#] */
        0x0F0400C0000ull, /*3. immed_w0[addr, 0x0] */
        0x0F4400C0000ull, /*4. immed_w1[addr, 0x0] */
        0x08B00009200ull, /*5. alu_shf[addr, addr, or, 4, <<16] */
        0x0F0000C0300ull, /*6. nop */
        0x02210008300ull, /*7. ssu_transfer[read, shared_ram, 
                                            $value, addr, 0, 1], 
                                            sig_done[sigRead] */
        0x0D802104213ull, /*8. nop_loop#: br_!signal[sigRead, 
                                          nop_loop#], defer[1] */
        0x0F0000C0300ull, /*9. nop */
        0x0F0000C0300ull, /*10. nop */
        0x0F0000C0300ull, /*11. nop */
        0x0F0000C0300ull, /*12. nop */
        0x0F0000C0300ull, /*13. l001_01#: l001_end#: end_of_program#: nop */
        0x0E000010000ull, /*14. ctx_arb[kill], any */
        0x0E000010000ull, /*15. kill_context#: ctx_arb[kill], any */
        0x0F0000C0300ull  /*16. nop */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    if(value == NULL)
    {
    	return (HALAE_BADARG);
    }    

    if(addr > MAX_SSU_SHARED_RAM - sizeof(unsigned int)) 
    {
       return (HALAE_BADARG);
    }   

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_AEACTIVE);
    }   

    /* fixup instruction for data and address */
    INSERT_IMMED_GPRA_CONST(microInst[3], (addr >>  0));
    INSERT_IMMED_GPRA_CONST(microInst[4], (addr >> 16));

    /* get and save the value of gpr and xfer_out reg */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, &sr0);

    /* execute ssu_transfer instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                         condCodeOff, 600, NULL);

    /* get the read value */
    getRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, value);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, 0, gprA0);
    putRelRdXfer(ae, ctx, ICP_SR_RD_REL, 0, sr0);

    SPIN_UNLOCK_AE(ae);

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCDramBaseAddr
   Description: Get coherent dram base address.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCDramBaseAddr(unsigned int *baseAddr)
{
    HALAE_VERIFY_LIB();
    if(!baseAddr)
    {
        return (HALAE_BADARG);
    } 

    *baseAddr = SysMemInfo.dramDesc[1].dramBaseAddr;
   
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetNCDramBaseAddr
   Description: Get none-coherent dram base address.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetNCDramBaseAddr(unsigned int *baseAddr)
{
    HALAE_VERIFY_LIB();
    if(!baseAddr)
    {
        return (HALAE_BADARG);
    }

    *baseAddr = SysMemInfo.dramDesc[0].dramBaseAddr;
    
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutCAM
   Description: Write a long-word value to the AE's CAM register. The AE must 
                be disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutCAM(unsigned char ae, 
             unsigned char entry, 
             unsigned int data,
             unsigned char state)
{
    unsigned int gprA, gprB, gpraAddr, gprbAddr, stateAddr;
    int stat = HALAE_SUCCESS;
    uword_T microInst[] = 
    {
        0xA9B0000000ull,        /* cam_write[gprB, gpra, state] */
        0xE000010000ull            /* ctx_arb[kill] */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;
    const unsigned short gprNum=0;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    if(entry >= MAX_CAM_REG) 
    {
       return (HALAE_BADARG);
    }   

    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    gpraAddr = getReg10bitAddr(ICP_GPA_REL, gprNum);
    gprbAddr = getReg10bitAddr(ICP_GPB_REL, gprNum);
    stateAddr = getReg10bitAddr(ICP_NO_DEST, (unsigned short)(0xf & state));

    /* fixup cam_write[gprB, gpra, state] instruction 
       for the source and dest registers */
    microInst[0] = microInst[0] | ((stateAddr & 0x3ff) << 20) | \
                   ((gprbAddr & 0x3ff) << 10) | ((gpraAddr & 0x3ff));

    /* get and save the value of an a-bank and b-bank gpr */
    getRelDataReg(ae, ctx, ICP_GPA_REL, gprNum, &gprA);
    getRelDataReg(ae, ctx, ICP_GPB_REL, gprNum, &gprB);
    putRelDataReg(ae, ctx, ICP_GPA_REL, gprNum, data);
    putRelDataReg(ae, ctx, ICP_GPB_REL, gprNum, (0xf & entry));

    /* execute cam_write instruction */
    stat = execMicroInst(ae, ctx, microInst, num_inst, 
                         condCodeOff, num_inst*5, NULL);

    /* restore the registers */
    putRelDataReg(ae, ctx, ICP_GPA_REL, gprNum, gprA);
    putRelDataReg(ae, ctx, ICP_GPB_REL, gprNum, gprB);

    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetCAMState
   Description: Read entire CAM state. The AE must be disabled prior to
                calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetCAMState(unsigned char ae, 
                  unsigned int *tags,
                  unsigned char *state, 
                  unsigned char *lru)
{
    unsigned int uAddr, uwrdLo, uwrdHi, tagMiss;
    unsigned int gprA0, gprA1, gprA2, gprB0, temp, ustoreAddr;
    unsigned int result0, result1;
    int status = HALAE_SUCCESS, total_status = HALAE_SUCCESS;
    int entry;
    const char ctx = 0;
    const int condCodeOff = 1;
    uword_T inst, savUword;
    unsigned int csrVal, newCsrVal, shCtlStoreFlag;    
    unsigned char aeNeigh;
    static uword_T instGetLRU[] = 
    {
        /*               ;; read LRU
                         ; bogus entry in A2
                         ; results in A0, A1
                         ; temps: B0
                         .reg a0 a1 a2 b0
                         .addr a0 0 a
                         .addr a1 1 a
                         .addr a2 2 a
                         .addr b0 0 b */
        0xF0000C0002ull, /* immed[a2, 0] */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0xA000000000ull, /* alu[a0, --, b, b0] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[0] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8BC0000200ull, /* alu_shf[a0, a0, or, b0, <<4] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[1] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8B80000200ull, /* alu_shf[a0, a0, or, b0, <<8] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[2] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8B40000200ull, /* alu_shf[a0, a0, or, b0, <<12] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[3] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8B00000200ull, /* alu_shf[a0, a0, or, b0, <<16] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[4] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8AC0000200ull, /* alu_shf[a0, a0, or, b0, <<20] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[5] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8A80000200ull, /* alu_shf[a0, a0, or, b0, <<24] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[6] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8A40000200ull, /* alu_shf[a0, a0, or, b0, <<28] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[7] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0xA000100000ull, /* alu[a1, --, b, b0] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[8] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8BC0100201ull, /* alu_shf[a1, a1, or, b0, <<4] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[9] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8B80100201ull, /* alu_shf[a1, a1, or, b0, <<8] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[10] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8B40100201ull, /* alu_shf[a1, a1, or, b0, <<12] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[11] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8B00100201ull, /* alu_shf[a1, a1, or, b0, <<16] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[12] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8AC0100201ull, /* alu_shf[a1, a1, or, b0, <<20] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[13] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8A80100201ull, /* alu_shf[a1, a1, or, b0, <<24] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[14] is MRU */
        0xBB80000002ull, /* cam_lookup[b0, a2] ; misses, B0 is LRU */
        0x943000012Full, /* alu_shf[b0, 0xf, and, b0, >>3] */
        0x8A40100201ull, /* alu_shf[a1, a1, or, b0, <<28] */
        0xB780000000ull, /* cam_read_tag[b0, b0] ; B0 is LRU tag */
        0xBBC0000000ull, /* cam_lookup[b0, b0] ; hits, LRU[15] is MRU */
        0xE000010000ull  /* ctx_arb[kill], any */
    };
    static uword_T instRestoreRegs[] = 
    {
        0xF0000C0000ull, /* immed[a0, 0x0000]    */
        0xF4400C0000ull, /* immed_w1[a0, 0x0000] */
        0xF0000C0001ull, /* immed[a1, 0x0000]    */
        0xF4400C0001ull, /* immed_w1[a1, 0x0000] */
        0xF0000C0002ull, /* immed[a2, 0x0000]    */
        0xF4400C0002ull, /* immed_w1[a2, 0x0000] */
        0xF000000300ull, /* immed[b0, 0x0000]    */
        0xF440000300ull, /* immed_w1[b0, 0x0000] */
        0xE000010000ull  /* ctx_arb[kill], any   */
    };
    const int num_instLRU = sizeof(instGetLRU)/sizeof(instGetLRU[0]);
    const int num_instRR = sizeof(instRestoreRegs)/sizeof(instRestoreRegs[0]);

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    if (!tags || !state || !lru) 
    {
        return (HALAE_BADARG);
    }    

    SPIN_LOCK_AE(ae);

    if(IS_RST(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_RESET);
    }    
    if(isAeActive(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_AEACTIVE);
    }    
    
    /* determine whether it neighbour AE runs in shared control store status */
    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    shCtlStoreFlag = csrVal & (0x1 << MMC_SHARE_CS_BITPOS);
    if(shCtlStoreFlag) 
    {
       halAe_GetSharedUstoreNeigh(ae, &aeNeigh);
       if(isAeActive(aeNeigh))
       {
          SPIN_UNLOCK_AE(ae);
          return (HALAE_NEIGHAEACTIVE); 
       }      
    }

    /* Two stage process:
       1. Read entries and tags
       2. Read LRU info
    */

    /***********************************************************************/
    /* Step 1: Read entries and tags ; this doesn't require executing code */

    /* backup shared control store bit, and force AE to 
       none-shared mode before executing ucode snippet */
    csrVal = GET_AE_CSR(ae, AE_MISC_CONTROL);
    shCtlStoreFlag = csrVal & (0x1 << MMC_SHARE_CS_BITPOS);
    newCsrVal = CLR_BIT(csrVal, MMC_SHARE_CS_BITPOS);
    SET_AE_CSR(ae, AE_MISC_CONTROL, newCsrVal);

    /* Save ustore location */
    if ((status = getUwords(ae, TmpUaddr, 1, &savUword)) != HALAE_SUCCESS) 
    {
        SPIN_UNLOCK_AE(ae);
        SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);     
        return (status);
    }

#if (MAX_CAM_REG != 16)

    #error "MAX_CAM_REG must equal to 16 for halAe_GetCAMState to work!!"

#endif

    /* save ustore-addr csr */
    ustoreAddr = GET_AE_CSR(ae, USTORE_ADDRESS);            
    /* enable ecs bit */
    uAddr = TmpUaddr | UA_ECS;
    for (entry = 0; entry < MAX_CAM_REG; entry++)
     {

        /* Create instruction to read state bits */
        inst = 0xAFB00C0000ull; /* cam_read_state[--, 0x0] */
        inst |= entry << 10;

        /* Write instruction to ustore */
        putAeCsr(ae, USTORE_ADDRESS, uAddr);
        if (PrdMinType == HWID_ICP) 
        {
             inst = setUwordECC(inst);
        }  
        uwrdLo = (unsigned int)(inst & 0xffffffff);
        uwrdHi = (unsigned int)(inst >> 32);

        putAeCsr(ae, USTORE_DATA_LOWER, uwrdLo);

        /* this will auto increment the address */
        putAeCsr(ae, USTORE_DATA_UPPER, uwrdHi);

        /* set the uaddress */
        putAeCsr(ae, USTORE_ADDRESS, uAddr);

        /* delay for at least 8 cycles */
        waitNumSimCycles(ae, 8, 0);

        /* read ALU output -- the instruction should have been executed
           prior to clearing the ECS in putUwords */
        getAeCsr(ae, ALU_OUT, &temp);
        state[entry] = temp >> 8;

        /* Create instruction to read tag bits*/
        inst = 0xA7B00C0000ull; /* cam_read_tag[--, 0x0] */
        inst |= entry << 10;

        /* Write instruction to ustore */
        putAeCsr(ae, USTORE_ADDRESS, uAddr);
        if (PrdMinType == HWID_ICP) 
        {
            inst = setUwordECC(inst);
        }
        uwrdLo = (unsigned int)(inst & 0xffffffff);
        uwrdHi = (unsigned int)(inst >> 32);

        putAeCsr(ae, USTORE_DATA_LOWER, uwrdLo);

        /* this will auto increment the address */
        putAeCsr(ae, USTORE_DATA_UPPER, uwrdHi);

        /* set the uaddress */
        putAeCsr(ae, USTORE_ADDRESS, uAddr);

        /* delay for at least 8 cycles */
        waitNumSimCycles(ae, 8, 0);

        /* read ALU output -- the instruction should have been executed
           prior to clearing the ECS in putUwords */
        getAeCsr(ae, ALU_OUT, tags+entry);

    } /* end for entry */
    /* restore ustore-addr csr */
    SET_AE_CSR(ae, USTORE_ADDRESS, ustoreAddr);

    /* restore the ustore */
    if((status = putUwords(ae, TmpUaddr, 1, &savUword)) != HALAE_SUCCESS) 
    {
        SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);   
        halAe_WaitUStoreAddrReady(ae);           
          
        SPIN_UNLOCK_AE(ae);
        return (status);
    }

    /* restore AE_MISC_CONTROL csr */
    SET_AE_CSR(ae, AE_MISC_CONTROL, csrVal);
    halAe_WaitUStoreAddrReady(ae);           
        
    /***********************************************************************/
    /* Step 2 : Read LRU ; this requires executing code */

    /* Step 2a : compute tag value that doesn't match any existing tags */
    tagMiss = 0;
    do 
    {
        for (entry = 0; entry < MAX_CAM_REG; entry++)
            if (tags[entry] == tagMiss){
                tagMiss++;
                break;
            }
        /* if (entry < 16), we have a match */
    } while (entry < MAX_CAM_REG);

    /* tagMiss < 0xFF */
    instGetLRU[0] = instGetLRU[0] | (tagMiss << 10); /* immed[a2, 0] */

    /* Read GPRs that we'll clobber */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &gprA0);
    getRelDataReg(ae, ctx, ICP_GPA_REL, 1, &gprA1);
    getRelDataReg(ae, ctx, ICP_GPA_REL, 2, &gprA2);
    getRelDataReg(ae, ctx, ICP_GPB_REL, 0, &gprB0);

    total_status = execMicroInst(ae, ctx, instGetLRU, num_instLRU,
                                 condCodeOff, num_instLRU*5, NULL);

    /* Read results */
    getRelDataReg(ae, ctx, ICP_GPA_REL, 0, &result0);
    getRelDataReg(ae, ctx, ICP_GPA_REL, 1, &result1);

    /* Restore GPRs that we clobbered */
    INSERT_IMMED_GPRA_CONST(instRestoreRegs[0], (gprA0 >>  0));
    INSERT_IMMED_GPRA_CONST(instRestoreRegs[1], (gprA0 >> 16));
    INSERT_IMMED_GPRA_CONST(instRestoreRegs[2], (gprA1 >>  0));
    INSERT_IMMED_GPRA_CONST(instRestoreRegs[3], (gprA1 >> 16));
    INSERT_IMMED_GPRA_CONST(instRestoreRegs[4], (gprA2 >>  0));
    INSERT_IMMED_GPRA_CONST(instRestoreRegs[5], (gprA2 >> 16));
    INSERT_IMMED_GPRB_CONST(instRestoreRegs[6], (gprB0 >>  0));
    INSERT_IMMED_GPRB_CONST(instRestoreRegs[7], (gprB0 >> 16));

    status = execMicroInst(ae, ctx, instRestoreRegs, num_instRR,
                           condCodeOff, num_instRR*5, NULL);
    if (status != HALAE_SUCCESS) 
    {
        total_status = status;
    }
    SPIN_UNLOCK_AE(ae);

    lru[(result0 >> 0) & 0xF] =  0;
    lru[(result0 >> 4) & 0xF] =  1;
    lru[(result0 >> 8) & 0xF] =  2;
    lru[(result0 >>12) & 0xF] =  3;
    lru[(result0 >>16) & 0xF] =  4;
    lru[(result0 >>20) & 0xF] =  5;
    lru[(result0 >>24) & 0xF] =  6;
    lru[(result0 >>28) & 0xF] =  7;
    lru[(result1 >> 0) & 0xF] =  8;
    lru[(result1 >> 4) & 0xF] =  9;
    lru[(result1 >> 8) & 0xF] = 10;
    lru[(result1 >>12) & 0xF] = 11;
    lru[(result1 >>16) & 0xF] = 12;
    lru[(result1 >>20) & 0xF] = 13;
    lru[(result1 >>24) & 0xF] = 14;
    lru[(result1 >>28) & 0xF] = 15;

    return (total_status);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_IsInpStateSet
   Description: Determine if the AE is in the specified state.  The AE must be
                disabled prior to calling this function.
   Returns:     HALAE_ISSET, HALAE_NOTSET, HALAE_BADLIB, HALAE_BADARG, 
                HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_IsInpStateSet(unsigned char ae, 
                    unsigned char inpState)
{
    int stat = HALAE_SUCCESS;
    unsigned int upc;
    uword_T microInst[] = 
    {
        0xD800800214ull,        /* br_inp_state[state, true#] */
        0xE000010000ull,        /* ctx_arb[kill] */
        0xF0000C0300ull,        /* true#: nop */
        0xE000010000ull            /* ctx_arb[kill] */
    };
    const int num_inst = sizeof(microInst)/sizeof(microInst[0]),condCodeOff=1;
    const unsigned char ctx=0;

    if(inpState >= MAX_INP_STATE) 
    {
        return (HALAE_BADARG);
    }    

    /* set the state in the instruction */
    microInst[0] = microInst[0] | (inpState << 14);

    HALAE_VERIFY_LIB();
    SPIN_LOCK_AE(ae);

    if(isAeActive(ae)) 
    {
        SPIN_UNLOCK_AE(ae);
        return (HALAE_AEACTIVE);
    }    

    /* execute br_inp_state instruction */
    if(!(stat = execMicroInst(ae, ctx, microInst, num_inst, 
                              condCodeOff, num_inst*5, &upc)))
    {
        if(upc > 2) 
        {
           stat = HALAE_ISSET;
        }   
        else
        {
           stat = HALAE_NOTSET;
        }   
    }
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    relToAbs
   Description: Convert ctx relative reg to absolute.
   Returns:     HALAE_SUCCESS, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
relToAbs(unsigned char ae, 
         unsigned char ctx, 
         unsigned short relRegNum,
         unsigned short *absRegNum)
{
    unsigned int ctxEnables;

    VERIFY_ARGPTR(absRegNum);

    halAe_GetAeCsr(ae, CTX_ENABLES, &ctxEnables);
    if(ctxEnables & CE_INUSE_CONTEXTS) 
    {
        /* 4-ctx mode */
        if(ctx & 0x1) 
        {
           return (HALAE_BADARG);
        }   
        *absRegNum = ((ctx & 0x6) << 4) | (relRegNum & 0x1f);
    } 
    else 
    {
        /* 8-ctx mode */
        *absRegNum = ((ctx & 0x7) << 4) | (relRegNum & 0x0f);
    }
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    absToRel
   Description: Convert absolute reg to ctx relative reg.
   Returns:     HALAE_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
absToRel(unsigned char ae, 
         unsigned short absRegNum, 
         unsigned short *relRegNum, 
         unsigned char *ctx)
{
    unsigned int ctxEnables;

    halAe_GetAeCsr(ae, CTX_ENABLES, &ctxEnables);
    if(ctxEnables & CE_INUSE_CONTEXTS) 
    {
        /* 4-ctx mode */
        *relRegNum = absRegNum & 0x1F;
        *ctx = (absRegNum >> 4) & 6;
    } else 
    {
        /* 8-ctx mode */
        *relRegNum = absRegNum & 0x0F;
        *ctx = (absRegNum >> 4) & 7;
    }
    return (HALAE_SUCCESS);
}


/*-----------------------------------------------------------------------------
   Function:    halAe_GetRelDataReg_Common
   Description: Read long-words from the specified register type:
                ICP_GPA_REL, ICP_GPB_REL, ICP_DR_RD_REL, ICP_SR_RD_REL,
                ICP_DR_WR_REL, ICP_SR_WR_REL, ICP_NEIGH_REL.
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetRelDataReg_Common(unsigned char ae, 
                           unsigned char ctx, 
                           icp_RegType_T regType,
                           unsigned short regNum, 
                           unsigned int *regData)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_ARGPTR(regData);
    VERIFY_AE(ae);
    VERIFY_CTX(ctx);

    SPIN_LOCK_AE(ae);
    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    switch(regType)
    {
    case ICP_GPA_REL:
    case ICP_GPB_REL:
        stat = getRelDataReg(ae, ctx, regType, regNum, regData);
        break;

    case ICP_DR_RD_REL:
    case ICP_SR_RD_REL:
        stat = getRelRdXfer(ae, ctx, regType, regNum, regData);
        break;

    case ICP_DR_WR_REL:
    case ICP_SR_WR_REL:
        stat = getRelWrXfers(ae, ctx, regType, regNum, regData, 1);
        break;

    case ICP_NEIGH_REL:
        if(regNum >= MAX_NN_REG) stat = HALAE_BADARG;
        else stat = getRelNNReg(ae, ctx, regType, regNum, regData);
        break;
    default:
        stat = HALAE_BADARG;
        break;
    }
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutRelDataReg_Common
   Description: Write long-words to the specified register type:
                ICP_GPA_REL, ICP_GPB_REL, ICP_DR_RD_REL, ICP_SR_RD_REL,
                ICP_DR_WR_REL, ICP_SR_WR_REL, ICP_NEIGH_REL
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutRelDataReg_Common(unsigned char ae, 
                           unsigned char ctx, 
                           icp_RegType_T regType,
                           unsigned short regNum, 
                           unsigned int regData)
{
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    VERIFY_CTX(ctx);

    SPIN_LOCK_AE(ae);
    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    switch(regType)
    {
    case ICP_GPA_REL:
    case ICP_GPB_REL:
        stat = putRelDataReg(ae, ctx, regType, regNum, regData);
        break;

    case ICP_DR_RD_REL:
    case ICP_SR_RD_REL:
        stat = putRelRdXfer(ae, ctx, regType, regNum, regData);
        break;

    case ICP_DR_WR_REL:
    case ICP_SR_WR_REL:
        stat = putRelWrXfer(ae, ctx, regType, regNum, regData);
        break;

    case ICP_NEIGH_REL:
        stat = putRelNN(ae, ctx, regNum, regData);
        break;

    default:
        stat = HALAE_BADARG;
        break;
    }
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetAbsDataReg_Common
   Description: Read long-words from the specified register type:
                ICP_GPA_ABS, ICP_GPB_ABS, ICP_DR_RD_ABS, ICP_SR_RD_ABS,
                ICP_DR_WR_ABS, ICP_SR_WR_ABS, ICP_NEIGH_ABS
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetAbsDataReg_Common(unsigned char ae, 
                           icp_RegType_T regType,
                           unsigned short absRegNum, 
                           unsigned int *regData)
{
    unsigned char ctx;
    unsigned short regNum;
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);

    if(regData == NULL)
    {
    	return (HALAE_BADARG);
    }    

    absToRel(ae, absRegNum, &regNum, &ctx);

    SPIN_LOCK_AE(ae);
    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    switch(regType)
    {
    case ICP_GPA_ABS:
        stat = getRelDataReg(ae, ctx, ICP_GPA_REL, regNum, regData);
        break;

    case ICP_GPB_ABS:
        stat = getRelDataReg(ae, ctx, ICP_GPB_REL, regNum, regData);
        break;

    case ICP_DR_RD_ABS:
        stat = getRelRdXfer(ae, ctx, ICP_DR_RD_REL, regNum, regData);
        break;

    case ICP_SR_RD_ABS:
        stat = getRelRdXfer(ae, ctx, ICP_SR_RD_REL, regNum, regData);
        break;

    case ICP_DR_WR_ABS:
        stat = getRelWrXfers(ae, ctx, ICP_DR_WR_REL, regNum, regData, 1);
        break;

    case ICP_SR_WR_ABS:
        stat = getRelWrXfers(ae, ctx, ICP_SR_WR_REL, regNum, regData, 1);
        break;

    case ICP_NEIGH_ABS:
        if(regNum >= MAX_NN_REG)
        {
            stat = HALAE_BADARG;
        }    
        else 
        {
            stat = getRelNNReg(ae, ctx, ICP_NEIGH_REL, regNum, regData);
        }    
        break;

    default:
            stat = HALAE_BADARG;
            break;
    }
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAbsDataReg_Common
   Description: Write long-words to the specified register type:
                ICP_GPA_ABS, ICP_GPB_ABS, ICP_DR_RD_ABS, ICP_SR_RD_ABS,
                ICP_DR_WR_ABS, ICP_SR_WR_ABS, ICP_NEIGH_ABS
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAbsDataReg_Common(unsigned char ae, 
                           icp_RegType_T regType,
                           unsigned short absRegNum, 
                           unsigned int regData)
{
    unsigned char ctx;
    unsigned short regNum;
    int stat = HALAE_SUCCESS;

    HALAE_VERIFY_LIB();
    VERIFY_AE(ae);
    
    absToRel(ae, absRegNum, &regNum, &ctx);

    SPIN_LOCK_AE(ae);
    if(isAeActive(ae)) 
    {
       SPIN_UNLOCK_AE(ae);
       return (HALAE_AEACTIVE);
    }   

    switch(regType)
    {
    case ICP_GPA_ABS:
        stat = putRelDataReg(ae, ctx, ICP_GPA_REL, regNum, regData);
        break;
    case ICP_GPB_ABS:
        stat = putRelDataReg(ae, ctx, ICP_GPB_REL, regNum, regData);
        break;

    case ICP_DR_RD_ABS:
        stat = putRelRdXfer(ae, ctx, ICP_DR_RD_REL, regNum, regData);
        break;
    case ICP_SR_RD_ABS:
        stat = putRelRdXfer(ae, ctx, ICP_SR_RD_REL, regNum, regData);
        break;

    case ICP_DR_WR_ABS:
        stat = putRelWrXfer(ae, ctx, ICP_DR_WR_REL, regNum, regData);
        break;
    case ICP_SR_WR_ABS:
        stat = putRelWrXfer(ae, ctx, ICP_SR_WR_REL, regNum, regData);
        break;

    case ICP_NEIGH_ABS:
        stat = putRelNN(ae, ctx, regNum, regData);
        break;

    default:
        stat = HALAE_BADARG;
        break;
    }
    SPIN_UNLOCK_AE(ae);
    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetDataReg
   Description: Read a generic data register of any type
   Returns:     
-----------------------------------------------------------------------------*/
int 
halAe_GetDataReg(unsigned char hwAeNum, 
                 unsigned char ctx,
                 icp_RegType_T regType,
                 unsigned short regAddr, 
                 unsigned int *regData)
{
    int status = HALAE_SUCCESS;
    unsigned int csrVal, aeCsr;

    switch(regType)
    {
    case ICP_GPA_REL:
    case ICP_GPB_REL:
    case ICP_DR_RD_REL:
    case ICP_SR_RD_REL:
    case ICP_DR_WR_REL:
    case ICP_SR_WR_REL:
    case ICP_NEIGH_REL:
        status = halAe_GetRelDataReg(hwAeNum, ctx, regType, regAddr, regData);
        break;
    case ICP_GPA_ABS:
    case ICP_GPB_ABS:
    case ICP_DR_RD_ABS:
    case ICP_SR_RD_ABS:
    case ICP_DR_WR_ABS:
    case ICP_SR_WR_ABS:
    case ICP_NEIGH_ABS:
        status = halAe_GetAbsDataReg(hwAeNum, regType, regAddr, regData);
        break;
    case ICP_SR_INDX:
    case ICP_DR_INDX:
        /* only zero is allowed as regAddr for ICP_SR_INDX/ICP_DR_INDX */
        if(regAddr != 0) 
        {
            status = HALAE_BADARG;
            break;
        }
        /* read the T_INDEX CSR and get the xferAddr */
        if((status = halAe_GetAeCsr(hwAeNum, T_INDEX, &csrVal))) 
        {
            break;
        }     

        if(regType == ICP_SR_INDX) 
        {
            regType = ICP_SR_RD_ABS;
        }    
        else
        {
            regType = ICP_DR_RD_ABS;
        }    
        regAddr = ((csrVal & TI_XFER_REG_INDEX) >> TI_XFER_REG_INDEX_BITPOS);
        status = halAe_GetAbsDataReg(hwAeNum, regType, regAddr, regData);
        break;
    case ICP_LMEM0:
    case ICP_LMEM1:
        /* read the LM CSR and "or" with the regAddr to get the lmAddr */
        if(regType == ICP_LMEM0) 
        {
           aeCsr = LM_ADDR_0_INDIRECT;
        }   
        else
        {
           aeCsr = LM_ADDR_1_INDIRECT;
        }   
        if((status = halAe_GetCtxIndrCsr(hwAeNum, ctx, aeCsr, &csrVal)))
        {
            break;
        }

        regAddr |= ((csrVal & XLA_LM_ADDR) >> XLA_LM_ADDR_BITPOS);
        status = halAe_GetLM(hwAeNum, regAddr, regData);
        break;
    default: status = HALAE_BADARG;
    }
    return status;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_MemSet
   Description: Initialize a block of memory to a constant value
   Returns:     Error status HALAE_*
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_MemSet(uint64 vaddr_dst, 
             int c, 
             int size)
{
    int status = HALAE_SUCCESS;

    memset((void*)(unsigned int)vaddr_dst, c, size);
    status = HALAE_SUCCESS;
 
    return status;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_IntrCallbackThdEx
   Description: Call back thread: waits on halAe_IntrPoll() and then calls
                a callback function. This version follows chaining
   Returns:        
   Uses:        halAe_IntrClear(), halAe_IntrPoll()
   Modifies:
-----------------------------------------------------------------------------*/
static void 
halAe_IntrCallbackThdEx(void* data)
{
    HalAeCallback_T *thread_data = (HalAeCallback_T*)data;
    int status = HALAE_SUCCESS;
    Hal_IntrMasks_T masks;
    HalAeCallbackChain_T *chain;

    ixOsalMemSet(&masks, 0, sizeof(masks));
    while (1) 
    {
        status = halAe_IntrPoll(thread_data->type_mask, &masks);
        if (status == HALAE_DISABLED) 
        {
            break;
        }    
        if (status != HALAE_SUCCESS) 
        {
            PRINTF("halAe_IntrPoll returns 0x%X\n",status);
            break;
        }
        SPIN_LOCK(HAL_CALLBACKTHDLOCK);
        for (chain = thread_data->callback_chain.next;
             chain;
             chain = chain->next) 
        {
            (*chain->callback_func)(&masks, chain->callback_data);
        }
        SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
    } /* end while 1 */

    SEM_POST(&thread_data->terminate_sem);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_SpawnIntrCallbackThdEx
   Description: Spawn callback thread for application
                Multiple callbacks on the same interrupts are chained
   Returns:        HALAE_SUCCESS, HALAE_FAIL
   Uses:
   Modifies:    no global state
-----------------------------------------------------------------------------*/
int 
halAe_SpawnIntrCallbackThdEx(unsigned int        type_mask,
                             HalAeIntrCallback_T callback_func,
                             void*               callback_data,
                             int                 thd_priority,
                             unsigned int        callback_priority,
                             void*               *handle)
{
    HalAeCallback_T *thread_data;
    HalAeCallbackChain_T *new_chain;
    int status = HALAE_SUCCESS;
    
    /* See if there is already a thread for this mask */
    SPIN_LOCK(HAL_CALLBACKTHDLOCK);
    for (thread_data = ISR_CALLBACK_ROOT;
         thread_data;
         thread_data = thread_data->next)
    {
        HalAeCallbackChain_T *prev_chain, *curr_chain;
        if (thread_data->type_mask != type_mask) 
        {
            continue;
        }    
        /* Found the right type, link us in */
        new_chain = (HalAeCallbackChain_T*) ixOsalMemAlloc(sizeof(HalAeCallbackChain_T));
        if (new_chain == NULL) 
        {
            SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
            return HALAE_FAIL;
        }
        new_chain->callback_func = callback_func;
        new_chain->callback_data = callback_data;
        new_chain->priority      = callback_priority;
        new_chain->thread_data   = thread_data;
        /* insert new_chain with the highest priority first */
        for (prev_chain = &thread_data->callback_chain,
             curr_chain = prev_chain->next;
             curr_chain;
             prev_chain = curr_chain, curr_chain = curr_chain->next) 
        {
            if (curr_chain->priority < new_chain->priority) 
            {
                break;
            }    
        } /* end for prev/curr_chain */
        /* At this point, either curr_chain is NULL or a lower priority.
           Insert new_chain after prev_chain */
        new_chain->next = prev_chain->next;
        prev_chain->next = new_chain;
        *handle = new_chain;
        SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
        return HALAE_SUCCESS;
    } /* end for thread_data */

    if(!(thread_data = (HalAeCallback_T*) ixOsalMemAlloc(sizeof(HalAeCallback_T)))) 
    {
        SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
        return HALAE_FAIL;
    }
    new_chain = (HalAeCallbackChain_T*) ixOsalMemAlloc(sizeof(HalAeCallbackChain_T));
    if (new_chain == NULL) 
    {
        ixOsalMemFree(thread_data);
        SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
        return HALAE_FAIL;
    }
    if((status = halAe_IntrEnable(type_mask)) != HALAE_SUCCESS)
    {
        ixOsalMemFree(new_chain);
        ixOsalMemFree(thread_data);
        SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
        return status;
    }

    new_chain->callback_func = callback_func;
    new_chain->callback_data = callback_data;
    new_chain->priority      = callback_priority;
    new_chain->next          = NULL;
    new_chain->thread_data   = thread_data;

    thread_data->type_mask = type_mask;
    thread_data->callback_chain.next = new_chain;
    thread_data->callback_chain.thread_data = thread_data;
    
    /* Link thread_data into global list */
    thread_data->next = ISR_CALLBACK_ROOT;
    ISR_CALLBACK_ROOT = thread_data;

    ixOsalSemaphoreInit(&thread_data->terminate_sem, 0);

    /* avoid the deadlock problem: halAe_IntrCallbackThdEx uses HAL_CALLBACKTHDLOCK 
     * before it is released in main thread */
    SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);

    /* Pass threadAttr=NULL to ixOsalThreadCreate so that the priority of 
     * the created thread will always be inherited from parent. */    
    status = ixOsalThreadCreate(&thread_data->threadID, 
                                NULL, 
                                (IxOsalVoidFnVoidPtr)halAe_IntrCallbackThdEx,
                                (void *)thread_data);  
    
    if(status == IX_SUCCESS) 
    {
        status = ixOsalThreadStart(&thread_data->threadID);
    }

    SPIN_LOCK(HAL_CALLBACKTHDLOCK);
    
    if(status != IX_SUCCESS) 
    {        
        halAe_IntrDisable(type_mask);
        ixOsalMemFree(new_chain);
        ixOsalMemFree(thread_data);
        SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
        return HALAE_FAIL;
    }
    
    *handle = new_chain;

    SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);

    return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
   Function:    terminateCallbackThdEx
   Description: Terminates callback thread if all callbacks are removed

   Returns:     status
   Uses:
-----------------------------------------------------------------------------*/
static int 
terminateCallbackThdEx(HalAeCallbackChain_T *callback_chain)
{
    int status = HALAE_FAIL;
    HalAeCallback_T *thread_data, *prev, *curr;
    HalAeCallbackChain_T *prev_chain, *curr_chain;

    thread_data = callback_chain->thread_data;

    SPIN_LOCK(HAL_CALLBACKTHDLOCK);
    /* remove ourselves from list, if list is empty call function below */
    for (prev_chain = &thread_data->callback_chain,
             curr_chain = prev_chain->next;
         curr_chain;
         prev_chain = curr_chain, curr_chain = curr_chain->next) 
    {
        if (curr_chain != callback_chain) 
        {
            continue;
        }    
        /* curr_chain == callback_chain, remove it */
        prev_chain->next = curr_chain->next;
        ixOsalMemFree(curr_chain);
        status = HALAE_SUCCESS;
        break;
    } /* end for prev_chain, curr_chain */
    /* See if they are all gone */
    if (thread_data->callback_chain.next == NULL) 
    {
        /* remove thread_data from global list */
        for (prev = NULL, curr = ISR_CALLBACK_ROOT;
             curr;
             prev = curr, curr = curr->next) 
        {
            if (curr != thread_data) 
            {
                continue;
            }    
            /* curr == thread_data, remove it */
            if (prev) 
            {
                /* In middle of list */
                prev->next = curr->next;
            } 
            else 
            {
                /* At start of list */
                ISR_CALLBACK_ROOT = curr->next;
            }
            break;
        } /* end for prev/curr */
        /* terminate thread and free thread_data */
        
        status = halAe_TerminateCallbackThd(&thread_data->callback_chain);
    }
    SPIN_UNLOCK(HAL_CALLBACKTHDLOCK);
    return status;
}

/*-----------------------------------------------------------------------------
   Function:     halAe_TerminateCallbackThd
   Description:  Terminates callback thread

   Returns:      HALAE_SUCCESS
   Uses:
-----------------------------------------------------------------------------*/
int 
halAe_TerminateCallbackThd(void* handle)
{
    int status = HALAE_SUCCESS;
    HalAeCallbackChain_T *callback_chain;
    HalAeCallback_T *thread_data;

    callback_chain = (HalAeCallbackChain_T*)handle;
    thread_data = callback_chain->thread_data;
    if (thread_data->callback_chain.next) 
    {
        return terminateCallbackThdEx(callback_chain);
    }
    status = halAe_IntrDisable(thread_data->type_mask);

    SEM_WAIT(&thread_data->terminate_sem, IX_OSAL_WAIT_FOREVER);
    ixOsalSemaphoreDestroy(&thread_data->terminate_sem);

    ixOsalMemFree(thread_data);

    return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Begin support for New Page callback
  -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
   Function:    halAe_DefineNewPageCallback
   Description: Defines new page callback
   Returns:     n/a
   Uses:
-----------------------------------------------------------------------------*/
void
halAe_DefineNewPageCallback(HalAePageChangeCallback_T callback_func,
                            void*                     user_data)
{
    new_page_callback      = callback_func;
    new_page_callback_data = user_data;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_CallNewPageCallback
   Description: Calls new page callback
   Returns:     n/a
   Uses:
-----------------------------------------------------------------------------*/
void 
halAe_CallNewPageCallback(Hal_PageChangeReason_T reason,
                          unsigned int           hwAeNum,
                          unsigned int           arg0,
                          unsigned int           arg1)
{
    PageData_T   *pageData   = PAGEDATA;
    AePageData_T *aePageData = &pageData->AePageData[hwAeNum];

    if (reason == NEW_PAGE_LOADED) 
    {
        unsigned int new_page_num = arg0;
        unsigned int old_page_num = arg1;
        /* if *_page_num is -1, because it is unsigned, it will be larger
           than numPages and so the array will not be referenced */
        if (old_page_num < aePageData->numPages) 
        {
            aePageData->addrs[old_page_num].loaded = 0;
        }
        if (new_page_num < aePageData->numPages) 
        {
            aePageData->addrs[new_page_num].loaded = 1;
        }
    } /* end if reason == NEW_PAGE_LOADED */
    
    if (new_page_callback) {
        (*new_page_callback)(reason, hwAeNum, arg0, arg1,
                             new_page_callback_data);
    }                             
}

/*-----------------------------------------------------------------------------
   Function:    halAe_DefineUcloCallback
   Description: Defines UCLO callback
   Returns:     n/a
   Uses:
-----------------------------------------------------------------------------*/
void
halAe_DefineUcloCallback(HalAe_UcloCall_T callback_func,
                         void*            user_data)
{
    int ii;

    for (ii=0; ii<MAX_AE; ii++) 
    {
        if (ucloCallbacks[ii].uclo_callback == NULL) 
        {
            ucloCallbacks[ii].uclo_callback = callback_func;
            ucloCallbacks[ii].uclo_callback_data = user_data;
            return;
        } 
    } /* end for ii */
}

/*-----------------------------------------------------------------------------
   Function:    halAe_DeleteUcloCallback
   Description: Deletes UCLO callback
   Returns:     n/a
   Uses:
-----------------------------------------------------------------------------*/
void
halAe_DeleteUcloCallback(HalAe_UcloCall_T callback_func,
                         void*            user_data)
{
    int ii;

    for (ii=0; ii<MAX_AE; ii++) 
    {
        if ((ucloCallbacks[ii].uclo_callback == callback_func) &&
            (ucloCallbacks[ii].uclo_callback_data == user_data))
        {
            ucloCallbacks[ii].uclo_callback = NULL;
            return;
        } 
    } /* end for ii */
}

/*-----------------------------------------------------------------------------
   Function:    halAe_CallUcloCallback
   Description: Calls UCLO callback
   Returns:     n/a
   Uses:
-----------------------------------------------------------------------------*/
void 
halAe_CallUcloCallback(Hal_UcloCallReason_T reason,
                       unsigned int         arg0,
                       unsigned int*        arg1)
{
    int ii;

    for (ii=0; ii<MAX_AE; ii++) 
    {
        if (ucloCallbacks[ii].uclo_callback != NULL)
        {
            (*ucloCallbacks[ii].uclo_callback)(reason,
                                            arg0,
                                            arg1,
                                            ucloCallbacks[ii].uclo_callback_data);
        } 
    } /* end for ii */
}

/*-----------------------------------------------------------------------------
   Function:     halAe_ReadMult
   Description:  Read a block of memory from an address to buffer
   Returns:      Error status HALAE_*
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_ReadMult(void* dst, 
               uint64 vaddr_src, 
               int size)
{
    int status = HALAE_SUCCESS;

    memcpy(dst, (void*)(unsigned int)vaddr_src, size);
    status = HALAE_SUCCESS;

    return status;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_WriteMult
   Description: Write a block of memory to an address
   Returns:     Error status HALAE_*
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_WriteMult(uint64 vaddr_dst, 
                void* src, 
                int size)
{
    int status = HALAE_SUCCESS;

    memcpy((void*)(unsigned int)vaddr_dst, src, size);
    status = HALAE_SUCCESS;

    return status;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_MapVirtToPhysUaddr
   Description: Convert from virtual to physical uword address
   Returns:     HALAE_SUCCESS, HALAE_BADARG
-----------------------------------------------------------------------------*/
int
halAe_MapVirtToPhysUaddr(unsigned char hwAeNum,
                         unsigned int  vaddr,
                         unsigned int *paddr,
                         unsigned int *loaded)
{
    PageData_T   *pageData   = PAGEDATA;
    AePageData_T *aePageData;
    AddrPair_T   *addrPair;
    int  ii;

    if((loaded == NULL) || (paddr == NULL))
    {
    	return (HALAE_BADARG);
    }    
    
    if ((pageData == NULL) || (pageData->AePageData[hwAeNum].numPages == 0)) 
    {
        *paddr = vaddr;
        *loaded = 1;
        return HALAE_SUCCESS;
    }
    aePageData = &pageData->AePageData[hwAeNum];

    /* Assume addrs array is in ascending vaddr order */
    /* Assume also that addrs[0].vaddr == 0 */
    for (ii = aePageData->numPages-1, addrPair = &aePageData->addrs[ii];
         ii>=0;
         ii--, addrPair--) 
    {
         if ((vaddr - addrPair->vaddr) < addrPair->size) 
         {
             break;        
         }    
    } /* end for ii, addrPair */
    
    if (ii < 0) 
    {

#ifdef _HALAE_PRINT

        PRINTF("halAe_MapVirtToPhysUaddr: vaddr (0x%x) exceeds page size: %d 0x%x 0x%x\n",
               vaddr, ii, addrPair->vaddr, addrPair->size);

#endif /* _HALAE_PRINT */

        /*put invalid value*/
        *paddr = (unsigned int) -1;
        *loaded = 0;
        return HALAE_BADADDR;
    }

    *paddr = addrPair->paddr + (vaddr - addrPair->vaddr);
    *loaded = addrPair->loaded;

    return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_MapVirtToPhysUaddrEx
   Description: Convert from virtual to physical uword address. Also returns
                page number
   Returns:     HALAE_SUCCESS, HALAE_BADARG
-----------------------------------------------------------------------------*/
int 
halAe_MapVirtToPhysUaddrEx(unsigned char hwAeNum,
                           unsigned int  vaddr,
                           unsigned int *paddr,
                           unsigned int *loaded,
                           unsigned int *page_num)
{
    PageData_T   *pageData   = PAGEDATA;
    AePageData_T *aePageData;
    AddrPair_T   *addrPair;
    int  ii;

    if((loaded == NULL) || (paddr == NULL) || (page_num == NULL))
    {
    	return (HALAE_BADARG);
    }    
    
    if ((pageData == NULL) || (pageData->AePageData[hwAeNum].numPages == 0)) 
    {
        *paddr = vaddr;
        *loaded = 1;
        *page_num = 0;
        return HALAE_SUCCESS;
    }
    
    aePageData = &pageData->AePageData[hwAeNum];

    /* Assume addrs array is in ascending vaddr order */
    /* Assume also that addrs[0].vaddr == 0 */
    for (ii = aePageData->numPages-1, addrPair = &aePageData->addrs[ii];
         ii>=0;
         ii--, addrPair--) 
    {
         if ((vaddr - addrPair->vaddr) < addrPair->size) 
         {
             break;
         }    
    } /* end for ii, addrPair */

    if (ii < 0) 
    {

#ifdef _HALAE_PRINT

        PRINTF("HalAe_MapVirtToPhysUaddrEx: vaddr (0x%x) exceeds page size: %d 0x%x 0x%x\n",
               vaddr, ii, addrPair->vaddr, addrPair->size);

#endif /* _HALAE_PRINT */

        /*put invalid value*/
        *paddr = (unsigned int) -1;
        *loaded = 0;
        *page_num = (unsigned int) -1;
        return HALAE_BADADDR;
    }

    *paddr = addrPair->paddr + (vaddr - addrPair->vaddr);
    *loaded = addrPair->loaded;
    *page_num = ii;

    return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_MapPhysToVirtUaddr
   Description: Convert from physical to virtual uword address
   Returns:     HALAE_SUCCESS, HALAE_BADADDR, HALAE_BADARG
-----------------------------------------------------------------------------*/
int 
halAe_MapPhysToVirtUaddr(unsigned char hwAeNum,
                         unsigned int  paddr,
                         unsigned int *vaddr)
{
    PageData_T   *pageData   = PAGEDATA;
    AePageData_T *aePageData;
    AddrPair_T   *addrPair;
    int           ii;

    if(vaddr == NULL)
    {
    	return (HALAE_BADARG);
    }    
    
    if ((pageData == NULL) || (pageData->AePageData[hwAeNum].numPages == 0)) 
    {
        *vaddr = paddr;
        return HALAE_SUCCESS;
    }

    aePageData = &pageData->AePageData[hwAeNum];    

    for (ii = aePageData->numPages-1, addrPair = &aePageData->addrs[ii];
         ii>=0;
         ii--, addrPair--) 
    {
        if ((addrPair->loaded) &&
            (addrPair->paddr <= paddr) &&
            (addrPair->paddr + addrPair->size > paddr))
        {
            *vaddr = addrPair->vaddr + (paddr - addrPair->paddr);
            return HALAE_SUCCESS;
        }
    } /* end for ii, addrPair */

    return HALAE_BADADDR;
}

/*-----------------------------------------------------------------------------
   Function:    HalAe_SetNumPages
   Description: Set the number of pages and allocate arrays
   Returns:     HALAE_SUCCESS, HALAE_FAIL
-----------------------------------------------------------------------------*/
int 
halAe_SetNumPages(unsigned char hwAeNum, 
                  unsigned int numPages)
{
    PageData_T   *pageData   = PAGEDATA;
    AePageData_T *aePageData = &pageData->AePageData[hwAeNum];
    AddrPair_T   *addrPair;
    unsigned int ii;

    aePageData->addrs = (AddrPair_T*) ixOsalMemAlloc(sizeof(AddrPair_T)*numPages);
    if (aePageData->addrs == NULL) 
    {
        return HALAE_FAIL;
    }    
    aePageData->numPages = numPages;
    for (ii=0, addrPair = &aePageData->addrs[0];
         ii<numPages;
         ii++, addrPair++) 
    {
        addrPair->vaddr = 0;
        addrPair->paddr = 0;
        addrPair->size = 0;
        addrPair->loaded = 0;
    }
    return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
   Function:    HalAe_SetPageData
   Description: Set one page entry
   Returns:     HALAE_SUCCESS, HALAE_BADARG
-----------------------------------------------------------------------------*/
int 
halAe_SetPageData(unsigned char hwAeNum, 
                  unsigned int page,
                  unsigned int vaddr, 
                  unsigned int paddr, 
                  unsigned int size)
{
    PageData_T   *pageData   = PAGEDATA;
    AePageData_T *aePageData = &pageData->AePageData[hwAeNum];
    AddrPair_T   *addrPair   = &aePageData->addrs[page];

    if (page >= aePageData->numPages) 
    {
        return HALAE_BADARG;
    }
    addrPair->vaddr = vaddr;
    addrPair->paddr = paddr;
    addrPair->size  = size;
    addrPair->loaded = 0;
    return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_dump
   Description: Dump AE data
   Returns:     HALAE_SUCCESS
-----------------------------------------------------------------------------*/
int 
halAe_dump(unsigned char ae)
{
    unsigned int ctxArbCtl, ctxEn,activeCtxStatus,ccEn, tStampH, tStampL, aluOut;
    unsigned int crcRem, profileCnt, ctxPtr;
    unsigned int ctxSigEv[MAX_CTX], ctxStatus[MAX_CTX], ctxWkupEv[MAX_CTX];
    unsigned int ctxWkupEvAct[MAX_CTX], ctxFutCnt[MAX_CTX], ctxFutCntAct[MAX_CTX];
    unsigned int activeCtxSig, activeLmAddr0, activeLmAddr1, ii;

    VERIFY_AE(ae);
    if(isAeActive(ae))
    {
        PRINTF("AE %d is still running...you may have corrupted the AE\n", ae);
    }

    getAeCsr(ae, CTX_ARB_CNTL, &ctxArbCtl);
    getAeCsr(ae, CTX_ENABLES, &ctxEn);
    getAeCsr(ae, ACTIVE_CTX_STATUS, &activeCtxStatus);
    getAeCsr(ae, CTX_SIG_EVENTS_ACTIVE, &activeCtxSig);
    getAeCsr(ae, CC_ENABLE, &ccEn);
    getAeCsr(ae, ALU_OUT, &aluOut);
    getAeCsr(ae, TIMESTAMP_LOW, &tStampL);
    getAeCsr(ae, TIMESTAMP_HIGH, &tStampH);
    getAeCsr(ae, CRC_REMAINDER, &crcRem);
    getAeCsr(ae, PROFILE_COUNT, &profileCnt);
    getAeCsr(ae, LM_ADDR_0_ACTIVE, &activeLmAddr0);
    getAeCsr(ae, LM_ADDR_1_ACTIVE, &activeLmAddr1);

    getAeCsr(ae, CSR_CTX_POINTER, &ctxPtr);    /* save ctx ptr */
    for(ii = 0; ii < MAX_CTX; ii++)
    {
        putAeCsr(ae, CSR_CTX_POINTER, ii & CCP_CONTEXT);
        getAeCsr(ae, CTX_SIG_EVENTS_INDIRECT, &ctxSigEv[ii]);
        getAeCsr(ae, CTX_WAKEUP_EVENTS_INDIRECT, &ctxWkupEv[ii]);
        getAeCsr(ae, CTX_STS_INDIRECT, &ctxStatus[ii]); 
        getAeCsr(ae, CTX_WAKEUP_EVENTS_ACTIVE, &ctxWkupEvAct[ii]); 
        getAeCsr(ae, CTX_FUTURE_COUNT_INDIRECT, &ctxFutCnt[ii]); 
        getAeCsr(ae, CTX_FUTURE_COUNT_ACTIVE, &ctxFutCntAct[ii]); 
    }
    putAeCsr(ae, CSR_CTX_POINTER, ctxPtr);    /* restore ctx ptr */

    PRINTF("AE=%d:\n  ctxArbCtl=0x%x, ctxEn=0x%x, actCtxStat=0x%x, actCtxSig=0x%x, actLmAddr0=0x%x\n",
            ae, ctxArbCtl, ctxEn, activeCtxStatus, activeCtxSig, activeLmAddr0);
    PRINTF("  actLmAdr1=0x%x, ccEn=0x%x, aluOut=0x%x, tStamp=0x%x 0x%x, profCnt=0x%x, crcRem=0x%x\n",
        activeLmAddr1, ccEn, aluOut, tStampH, tStampL, profileCnt, crcRem);

    for(ii = 0; ii < MAX_CTX; ii++){
        PRINTF("  Context %d:\n", ii);
        PRINTF("    ctxPC=%d, ctxSigEv=0x%x, ctxStatus=0x%x, ctxWkupEv=0x%x\n",
                ctxStatus[ii] & UpcMask, ctxSigEv[ii], ctxStatus[ii], ctxWkupEv[ii]);

        PRINTF("    ctxWkupEvAct=0x%x, ctxFutureCnt=0x%x, ctxFutureCntAct=0x%x\n",
                ctxWkupEvAct[ii], ctxFutCnt[ii], ctxFutCntAct[ii]);
    }

        
    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetErrorStr
   Description: Get the error string for provided error code
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
char *
halAe_GetErrorStr(int errCode)
{
    switch(errCode)
    {
    case HALAE_SUCCESS:         
            return ("The operation is successful.");
    case HALAE_FAIL:
            return ("The operation is failed.");        
    case HALAE_BADARG:
            return ("Bad function argument passed.");        
    case HALAE_DUPBKP:
        return ("Add duplicated breakpoint.");        
    case HALAE_NOUSTORE:
        return ("No free ustore available.");
    case HALAE_BADADDR:
        return ("Bad address argument passed.");
    case HALAE_BADLIB:
        return ("HAL library isn't initialized.");
    case HALAE_DISABLED:
        return ("Acceleration Engine is disabled.");
    case HALAE_ENABLED:
        return ("Acceleration Engine is enabled.");
    case HALAE_RESET:
        return ("Acceleration Engine is in reset.");
    case HALAE_TIMEOUT:
        return ("The operation times out.");
    case HALAE_ISSET:
        return ("Condition/evaluation is set/true.");
    case HALAE_NOTSET:
        return ("Condition/evaluation is not set/false.");
    case HALAE_AEACTIVE:
        return ("Acceleration Engine is running.");
    case HALAE_MEMALLOC:
        return ("Memory allocation error.");
    case HALAE_RINGFULL:
        return ("Eagle-tail ring is full.");
    case HALAE_RINGEMPTY:
        return ("Eagle-tail ring is empty.");
    case HALAE_NEIGHAEACTIVE:
        return ("Neighbouring ae is running.");        
    default:
        return (NULL);
    }        
}

