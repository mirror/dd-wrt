/**
 * @file IxNpeMhUnsolicitedCbMgr_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Unsolicited Callback
 * Manager module.
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
 * @defgroup IxNpeMhUnsolicitedCbMgr_p IxNpeMhUnsolicitedCbMgr_p
 *
 * @brief The private API for the Unsolicited Callback Manager module.
 * 
 * @{
 */

#ifndef IXNPEMHUNSOLICITEDCBMGR_P_H
#define IXNPEMHUNSOLICITEDCBMGR_P_H

#include "IxNpeMh.h"
#include "IxOsalTypes.h"

/*
 * #defines for function return types, etc.
 */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhUnsolicitedCbMgrInitialize (void)
 *
 * @brief This function initializes the Unsolicited Callback Manager
 * module, setting up a callback data structure for each NPE.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrInitialize (void);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrCallbackSave (
           IxNpeMhNpeId npeId,
           IxNpeMhMessageId unsolicitedMessageId,
           IxNpeMhCallback unsolicitedCallback)
 *
 * @brief This function saves a callback in the specified NPE's callback
 * table.  If a callback already exists for the specified ID then it will
 * be overwritten.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE in whose callback
 * table the callback will be saved.
 * @param IxNpeMhMessageId unsolicitedMessageId (in) - The ID of the
 * messages that this callback is for.
 * @param IxNpeMhCallback unsolicitedCallback (in) - The callback function
 * pointer to save.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrCallbackSave (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId unsolicitedMessageId,
    IxNpeMhCallback unsolicitedCallback);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrCallbackRetrieve (
           IxNpeMhNpeId npeId,
           IxNpeMhMessageId unsolicitedMessageId,
           IxNpeMhCallback *unsolicitedCallback)
 *
 * @brief This function retrieves the callback for the specified ID from
 * the specified NPE's callback table.  If no callback is registered for
 * the specified ID and NPE then a callback value of NULL will be returned.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE from whose callback
 * table the callback will be retrieved.
 * @param IxNpeMhMessageId unsolicitedMessageId (in) - The ID of the
 * messages that the callback is for.
 * @param IxNpeMhCallback unsolicitedCallback (out) - The callback function
 * pointer retrieved.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrCallbackRetrieve (
    IxNpeMhNpeId npeId,
    IxNpeMhMessageId unsolicitedMessageId,
    IxNpeMhCallback *unsolicitedCallback);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Unsolicited
 * Callback Manager module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhUnsolicitedCbMgrShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Unsolicited
 * Callback Manager module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhUnsolicitedCbMgrShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHUNSOLICITEDCBMGR_P_H */

/**
 * @} defgroup IxNpeMhUnsolicitedCbMgr_p
 */
