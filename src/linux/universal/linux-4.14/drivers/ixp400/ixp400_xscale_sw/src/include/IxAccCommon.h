/**
 * @file IxAccCommon.h
 *
 * @date 17 February 2006

 * @brief This file contains the shared information across the components 
 * when more than one components co-exists together
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.4
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

/**
 *
 * @defgroup IxAccCommon Intel (R) IXP400 Software Component Co-exist
 *
 * @brief component coexist information
 */


#ifndef __IX_ACC_COMMON_H__
#define __IX_ACC_COMMON_H__

#include "IxOsal.h"
#include "IxEthNpe.h"

/**
 * @ingroup IxAccCommon
 * @def ixEthHssAccCoexistEnable
 *
 * @brief global variables used across ethAcc and hssAcc components
 *
 */

extern BOOL ixEthHssAccCoexistEnable;

/**
 * @ingroup IxAccCommon
 * @def ixEthHssComMutexInitDone
 *
 * @brief global variables used across ethAcc and hssAcc components
 *
 */


extern BOOL ixEthHssComMutexInitDone;

/**
 * @ingroup IxAccCommon
 * @def ixEthHssCoexistLock
 *
 * @brief Common mutex used by ethernet & hss components
 *
 */
extern IxOsalMutex ixEthHssCoexistLock;

/**
 * @ingroup IxAccCommon
 * @def IX_ETH_HSS_COM_MUT_UNLOCK
 *
 * @brief macros for common mutex control for ETH & HSS co-existence
 *
 */

#define IX_ETH_HSS_COM_MUT_UNLOCK() 	ixOsalMutexUnlock (&ixEthHssCoexistLock)



/**
 * @ingroup IxAccCommon
 * @def IX_ETH_HSS_COM_MUT_LOCK(result)
 *
 * @brief macros for common mutex control for ETH & HSS co-existence
 *
 */

#define IX_ETH_HSS_COM_MUT_LOCK(result) \
    do { \
         result = ixOsalMutexLock(&ixEthHssCoexistLock, IX_OSAL_WAIT_FOREVER); \
            \
         if (result != IX_SUCCESS) \
         { \
            printf("Failed to lock ixEthHssCoexistLock mutex %x\n",result); \
         } \
    } while (0);

/**
 * @ingroup IxAccCommon
 *
 * @fn ixEthNpePortMapCreate(void)
 *
 * @brief Select the default port parameters for particular Intel(R) IXP4XX Product Line, 
 *        setup lookup tables for port conversions
 * 	  Ethernet PortId => LogicalId, 
 * 	  LogicalId => Ethernet PortId,  
 *        Ethernet PortId => Physical Address 
 *
 * @param none
 *
 * @return IxEthAccStatus
 * IX_ETH_NPE_SUCCESS - port mapping lookup table successfully built
 * IX_ETH_NPE_FAIL - unknown featureCtrl device ID, failed to get default port mapping
 * @internal
 */
IxEthNpeStatus ixEthNpePortMapCreate(void);

#endif /* __IX_ACC_COMMON_H__ */



