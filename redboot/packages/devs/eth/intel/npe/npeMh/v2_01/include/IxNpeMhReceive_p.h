/**
 * @file IxNpeMhReceive_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Receive module.
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

/**
 * @defgroup IxNpeMhReceive_p IxNpeMhReceive_p
 *
 * @brief The private API for the Receive module.
 * 
 * @{
 */

#ifndef IXNPEMHRECEIVE_P_H
#define IXNPEMHRECEIVE_P_H

#include "IxNpeMh.h"
#include "IxOsalTypes.h"

/*
 * #defines for function return types, etc.
 */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhReceiveInitialize (void)
 *
 * @brief This function registers an internal ISR to handle the NPEs'
 * "outFIFO not empty" interrupts and receive messages from the NPEs when
 * they become available.
 *
 * @return No return value.
 */

void ixNpeMhReceiveInitialize (void);

/**
 * @fn IX_STATUS ixNpeMhReceiveMessagesReceive (
           IxNpeMhNpeId npeId)
 *
 * @brief This function reads messages from a particular NPE's outFIFO
 * until the outFIFO is empty, and for each message looks first for an
 * unsolicited callback, then a solicited callback, to pass the message
 * back to the client.  If no callback can be found the message is
 * discarded and an error reported. This function will return TIMEOUT 
 * status if NPE hang / halt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to receive
 * messages from.
 *
 * @return The function returns a status indicating success, failure or timeout.
 */

IX_STATUS ixNpeMhReceiveMessagesReceive (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhReceiveShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Receive
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return status.
 */

void ixNpeMhReceiveShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhReceiveShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Receive
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return status.
 */

void ixNpeMhReceiveShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHRECEIVE_P_H */

/**
 * @} defgroup IxNpeMhReceive_p
 */
