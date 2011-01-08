/**
 * @file IxAtmdDescMgmt_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Npe descriptor management
 *
 * Npe descriptor allocation and release
 *
 * @note - this module is protected by its own mutex
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

#ifndef IXATMDDESC_MGMT_P_H
#define IXATMDDESC_MGMT_P_H

#include "IxAtmdNpe_p.h"
#include "IxAtmdDefines_p.h"

/**
* @brief DescMgmt Initialisation
*/
IX_STATUS ixAtmdAccDescMgmtInit (void);


/**
* @brief DescMgmt Uninitialisation
*/
IX_STATUS ixAtmdAccDescMgmtUninit (void);


/**
* @brief DescMgmt statistics display
*/
void ixAtmdAccDescMgmtStatsShow (void);

/**
* @brief DescMgmt statistics reset
*/
void ixAtmdAccDescMgmtStatsReset (void);

/**
* @brief Get a NPE descriptor from a descriptor pool
*
* @param npeDescriptorPtr (out) pointer to a npeDescriptor
*
* @return @li IX_SUCCESS a descriptor can be retrieved from the
*         pool of descriptors and is passed back through npeDescriptorPtr
* @return @li IX_FAIL a descriptor cannot be retrieved from the
*         pool of descriptors. The content of npeDescriptorPtr is unspecified
*/
IX_STATUS
ixAtmdAccDescNpeDescriptorGet (IxAtmdAccNpeDescriptor** npeDescriptorPtr);

/**
* @brief Release a NPE descriptor to a descriptor pool
*
* @param npeDescriptorPtr (in) pointer to a valid npeDescriptor
*
* @return @li IX_SUCCESS the descriptor can be stored to the
*         pool of descriptors
* @return @li IX_FAIL the descriptor cannot be stored into the
*         pool of descriptors.
*/
IX_STATUS
ixAtmdAccDescNpeDescriptorRelease (IxAtmdAccNpeDescriptor* npeDescriptorPtr);

#ifndef NDEBUG
/**
* @brief Get the memory used by NPE descriptors in IxAtmdAcc
*        for debug and tests purposes
*
* @param descriptorMemoryUsagePtr (out) memory used
*
* @return @li IX_SUCCESS always
*/
IX_STATUS
ixAtmdAccDescMgmtMemoryUsageGet (unsigned int* descriptorMemoryUsagePtr);
#endif

#endif /* IXATMDDESCMGMT_P_H */


