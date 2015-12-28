/**
 * @file    IxErrHdlAccEthNPE_p.h
 *
 * @author Intel Corporation
 * @date    1-Jan-2006
 *    
 * @brief   This file contains the implementation of the ixErrHdlAcc 
 *          EthNPE sub-component
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/
#ifndef IxErrHdlAccEthNPE_p_H
#define IxErrHdlAccEthNPE_p_H

/*
* @note 
* Undefine the macro below to remove wait for halt if performance 
* is needed, because it may pro-long the NPE Ethernet recovery duration.
* A longer wait may caused memory leakage, if client eth transimit fail
* does not replenish the mBUF.
* Define this marco for better reliability.
*/
/*
#define IX_ERRHDLACC_WAIT_FOR_HALT
*/

#define IX_ERRHDLACC_NPEMH_ID_MAX_POLL                 (3)   /* NPEMH NpeID to be polled*/
#define IX_ERRHDLACC_NPEDL_FUNC_ID_MASK                (0x7F)  /* Functional ID Mask*/
#define IX_ERRHDLACC_NPEMHPOLL_THREAD_PRIORITY         (50) /* NPEMH  Thread polling priority*/
#define IX_ERRHDLACC_MAX_NPE_FUNC_ID                   (2)  /* Max NPE Functional ID */
#define IX_ERRHDLACC_MAX_TRY_NPESTOP                   (10) /* Max tries to stop the NPE*/
#define IX_ERRHDLACC_MAX_QMGR_RETRY_WRITE              (2)  /* Maximum tries to write to the QM*/
#define IX_ERRHDLACC_MAX_TX_SUBMIT_TOTAL_BUFFER        (32) /* Maximum TX buffer to retrieve*/
#define IX_ERRHDLACC_OUT_OF_BOUND_ADDRESS_VALIDITYMASK (0xC0000000)/* Out of bound address*/
#define IX_ERRHDLACC_MAX_WAIT_ETHACCTXRX_HALT_COUNT    (50) /* Max Ethernet TX RX Halt try counts*/

/* Macro functions definition*/
#define IX_ERRHDLACC_ETH_RELOAD_ABORT(eventStruct, errorCode) do {   \
    ixErrHdlAccEvent* event = eventStruct;                           \
    ixEthDBEventProcessorPauseModeSet(FALSE);                        \
    event->lastEvtStatus = errorCode;                                \
    event->numFail++;                                                \
    ixEthAccStartRequest();                                          \
    ixErrHdlAccNpePollThreadRun = FALSE;                             \
    } while(0)
                                                              
#define IX_ERRHDLACC_ETH_RELOAD_DONE(eventStruct) do{     \
   ixErrHdlAccEvent* event = eventStruct;                 \
   ixEthDBEventProcessorPauseModeSet(FALSE);              \
   event->lastEvtStatus = IX_ERRHDLACC_EVT_STOP_SUCCESS;  \
   event->numPass++;                                      \
   ixEthAccStartRequest();                                \
   ixErrHdlAccNpePollThreadRun = FALSE;                   \
   } while(0)  
                                
/* Eth NPE Image MBUF Address and data information data structure*/
typedef struct
{
  UINT32 funcId;
  UINT32 mBufRXRootAddr;
}IX_ERRHDLACC_ETHNPE_MBUF_INFO;

PUBLIC IX_STATUS ixErrHdlAccEthNPEHandleISR(ixErrHdlAccEvent* eventStruct);
PUBLIC void ixErrHdlAccEthNPERecoveryJob(ixErrHdlAccEvent* eventStruct);

#endif
