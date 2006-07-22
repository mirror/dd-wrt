/**
 * @file IxAtmdPortMgmt_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Utopia Port Management
 *
 * This part of atmd is responsible for UTOPIA port configuration
 * and management.
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

#ifndef IXATMDPORTMGMT_P_H
#define IXATMDPORTMGMT_P_H

#include "IxAtmTypes.h"

/*
 * Types
 */

/**
 * @enum IxAtmdAccPortState
 *
 * @brief Port state :
 *
 * IxAtmdAccPortState is used to query the state of a port (the real state
 * of a port may be up,down or down in progress). These macros are used
 * to wrap the internal state to the user-visible state
 *
 */
typedef enum
{
    IX_ATMD_PORT_DISABLED = 0,
    IX_ATMD_PORT_ENABLED
} IxAtmdAccPortState;

/**
 * @brief Port state observer:
 *
 */
typedef void (*IxAtmdAccPortSetupNotifyHandler)(
    unsigned int numPort);

/**
 * @brief Port state observer:
 *
 */
typedef IX_STATUS (*IxAtmdAccPortStateChangeHandler)(
    IxAtmLogicalPort port,
    IxAtmdAccPortState state);

/**
 * @brief Port state observer:
 *
 */
typedef BOOL (*IxAtmdAccPortStateQuery)(
    IxAtmLogicalPort port);

/*
 * Prototypes
 */

/**
 * @brief PortMgmt Initialisation
 */
IX_STATUS
ixAtmdAccPortMgmtInit (void);


/**
 * @brief PortMgmt Uninitialisation
 */
IX_STATUS
ixAtmdAccPortMgmtUninit (void);


/**
 * @brief PortMgmt Display state and stats
 */
void
ixAtmdAccPortMgmtStatsShow (void);

/**
 * @brief PortMgmt Display state and stats
 */
void
ixAtmdAccPortMgmtStatsReset (void);

/**
 * @brief PortMgmt Utopia initialisations
 */
IX_STATUS
ixAtmdAccUtopiaControlInit (void);

/**
 * @brief PortMgmt Utopia information display
 */
void
ixAtmdAccUtopiaControlStatsShow (void);

/**
 * @brief PortMgmt Utopia information reset
 */
void
ixAtmdAccUtopiaControlStatsReset (void);

/**
 * @brief Check the port exists and is configured
 */
BOOL
ixAtmdAccPortConfigured (IxAtmLogicalPort port);

/**
 * @brief Register for notifcation of port state changes
 */
void
ixAtmdAccPortStateHandlersRegister(
    IxAtmdAccPortSetupNotifyHandler setupNotifyHandler,
    IxAtmdAccPortStateChangeHandler stateChangeHandler,
    IxAtmdAccPortStateQuery isEnabledQuery,
    IxAtmdAccPortStateQuery isDisabledQuery);


/**
 * @brief UnRegister notifcation of port state changes
 */
void
ixAtmdAccPortStateHandlersUnregister(void);


/**
 * @brief Check if the port is enabled?
 */
BOOL
ixAtmdAccPortIsEnabled(IxAtmLogicalPort port);

/**
 * @brief Set the number of Tx VC queues
 */
IX_STATUS
ixAtmdAccPortMgmtNumTxVcQueuesSet (unsigned int numTxVcQueues);

/**
 * @brief Get the number of Tx VC queues
 */
IX_STATUS
ixAtmdAccPortMgmtNumTxVcQueuesGet (unsigned int *numTxVcQueues);


#endif /* IXATMDPORTMGNT_P_H */


