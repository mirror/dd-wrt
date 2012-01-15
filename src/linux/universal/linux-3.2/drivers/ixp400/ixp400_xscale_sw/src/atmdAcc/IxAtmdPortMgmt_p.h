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


