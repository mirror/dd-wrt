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
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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


