/**
 * @file    IxErrHdlAcc_p.h
 *
 * @author Intel Corporation
 * @date    1-Jan-2006
 *    
 * @brief   This file contains the implementation of the 
 *          ixErrHdlAcc main
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
#ifndef IxErrHdlAcc_p_H
#define IxErrHdlAcc_p_H

/*#define IX_ERRHDLACC_DEBUG*/

typedef  enum{
  IX_ERRHDLACC_EVT_STOP_SUCCESS, 
  IX_ERRHDLACC_EVT_WAIT,
  IX_ERRHDLACC_EVT_RUN,
  IX_ERRHDLACC_EVT_IDLE,
  IX_ERRHDLACC_EVT_UNKNWON_FAILURE = 0x10,
  IX_ERRHDLACC_EVT_NPE_DOWNLOAD_FAIL,
  IX_ERRHDLACC_EVT_NPE_ETHMAC_REUPDATE_FAIL,
  IX_ERRHDLACC_EVT_NPE_ETH_PRIORITYTABLE_UPDATE_FAIL,
  IX_ERRHDLACC_EVT_NPE_ETH_AQMQUEUEUPDATE_FAIL,
  IX_ERRHDLACC_EVT_NPE_ETH_STOPREQUEST_FAIL,
  IX_ERRHDLACC_EVT_NPE_ETH_FEATUREUPDATE_FAIL
} IX_ERRHDLACC_EVT_STATUS;

#define IX_ERRHDLACC_CHECK_NPEID_VALIDITY(npeID, error) \
do                                                      \
{                                                       \
 if(npeID > IX_ERRHDLACC_NPEC)                          \
 {                                                      \
   return error;                                        \
 }                                                      \
}while(0)
#define IX_ERRHDLACC_CHECK_NULL_PTR(ptr) \
 do{                                     \
   if (ptr == NULL)                      \
   {                                     \
     return IX_FAIL;                     \
   }                                     \
 }while(0)                
#define IX_ERRHDLACC_EVENT_TYPE_CHECK_VALIDITY(eventType) if(eventType >= IX_ERRHDLACC_MAX_TOTAL_EVENT) { return IX_FAIL;}

#define IX_ERRHDLACC_FATAL_LOG(a,b,c,d,e,f,g)   do{ ixOsalLog ( IX_OSAL_LOG_LVL_FATAL,IX_OSAL_LOG_DEV_STDOUT,a,b,c,d,e,f,g);}while(0)
#define IX_ERRHDLACC_WARNING_LOG(a,b,c,d,e,f,g) do{ ixOsalLog ( IX_OSAL_LOG_LVL_WARNING,IX_OSAL_LOG_DEV_STDOUT,a,b,c,d,e,f,g);}while(0)

#ifdef IX_ERRHDLACC_DEBUG
#define IX_ERRHDLACC_DEBUG_LOG(a,b,c,d,e,f,g)   do{ ixOsalLog ( IX_OSAL_LOG_LVL_FATAL,IX_OSAL_LOG_DEV_STDOUT,a,b,c,d,e,f,g);}while(0)
#define IX_ERRHDLACC_DEBUG_MSG(msg)             do{ ixOsalLog ( IX_OSAL_LOG_LVL_MESSAGE,IX_OSAL_LOG_DEV_STDOUT,msg,0,0,0,0,0,0);}while(0)
#else
#define IX_ERRHDLACC_DEBUG_LOG(a,b,c,d,e,f,g) do{}while(0)
#define IX_ERRHDLACC_DEBUG_MSG(msg)   do{}while(0)
#endif

extern BOOL ixErrHdlAccInitDone;

#endif
