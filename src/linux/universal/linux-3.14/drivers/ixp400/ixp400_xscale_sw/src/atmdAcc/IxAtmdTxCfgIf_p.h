/**
 * @file IxAtmdTxCfgIf_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Tx Control
 *
 * This file defines the error counters used in the Atmd Tx code
 * and the initialisation functions
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

#ifndef IXATMDTX_CFGIF_P_H
#define IXATMDTX_CFGIF_P_H

#include "IxOsal.h"

/**
* @brief TxCfgIf initialisation function
* @note - this function should be used once
*
* @return @li IX_SUCCESS on success
* @return @li IX_FAIL on error
*/
IX_STATUS
ixAtmdAccTxCfgIfInit (void);


/**
* @brief TxCfgIf uninitialisation function
*
* @return @li IX_SUCCESS on success
* @return @li IX_FAIL on error
*/
IX_STATUS
ixAtmdAccTxCfgIfUninit (void);


/**
* @brief TxCfgIf stats display function
* @note  This function uses printf
*
* @return none
*/
void
ixAtmdAccTxCfgIfStatsShow (void);

/**
* @brief TxCfgIf sttas reset function
*
* @return none
*/
void
ixAtmdAccTxCfgIfStatsReset (void);

/**
* @brief  display tx port configuration
*
* @return none
*/
void
ixAtmdAccTxCfgIfPortShow (IxAtmLogicalPort port);

/**
* @brief  display tx configuration
*
* @return none
*/
void
ixAtmdAccTxCfgIfChannelShow (IxAtmLogicalPort port);





#endif /* IXATMDTXCFGIF_P_H */


