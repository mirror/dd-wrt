/**
 * @file IxAtmdRxCfgInfo_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Rx Configuration
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

#ifndef IX_ATMDRXCFGINFO_P_H
#define IX_ATMDRXCFGINFO_P_H

#include "IxQMgr.h"

/**
* @brief Rx data initialisation
*/
IX_STATUS ixAtmdAccRxCfgInfoInit (void);


/**
* @brief Rx data uninitialisation
*/
IX_STATUS ixAtmdAccRxCfgInfoUninit (void);


/**
* @brief Rx config display
*/
void ixAtmdAccRxCfgInfoChannelShow (IxAtmLogicalPort port);

/**
* @brief Rx stats display
*/
void ixAtmdAccRxCfgInfoStatsShow (void);

/**
* @brief Rx stats reset
*/
void ixAtmdAccRxCfgInfoStatsReset (void);

/**
* @brief Check is a vc is already configured
*/
BOOL ixAtmdAccRxCfgRxVcExists (IxAtmLogicalPort port,
                               unsigned int vpi,
                               unsigned int vci);

/**
* @brief  Get an available rx free queue
*/
IX_STATUS ixAtmdAccRxCfgFreeQueueGet (IxAtmdAccAalType aalServiceType,
                                      unsigned int minimumQueueSize,
                                      unsigned int npeVcId);

/**
* @brief  Get an available rx channel
*/
IX_STATUS ixAtmdAccRxCfgChannelGet (IxAtmLogicalPort port,
                                    unsigned int vpi,
                                    unsigned int vci,
                                    IxAtmdAccAalType aalServiceType,
                                    unsigned int* npeVcIdPtr,
                                    IxAtmConnId* connIdPtr);

/**
* @brief  Initialise a rx channel
*/
IX_STATUS
ixAtmdAccRxCfgChannelSet (IxAtmConnId connId,
                          unsigned int npeVcId,
                          IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci,
                          IxAtmdAccAalType aalServiceType,
                          IxAtmRxQueueId rxQueueId,
                          IxAtmdAccUserId userId,
                          IxAtmdAccRxVcRxCallback rxCallback);

/**
* @brief  Unconfigure a rx channel
*/
IX_STATUS ixAtmdAccRxCfgChannelReset (unsigned int npeVcId);

/**
* @brief Get the npeVcId from the connectionId if it is a valid one
*/
IX_STATUS ixAtmdAccRxCfgNpeVcIdGet (IxAtmConnId connId,
                                    unsigned int* npeVcId);

/**
* @brief Check all resources are available for this channel
*/
IX_STATUS ixAtmdAccRxCfgFreeResourcesCheck (unsigned int npeVcId);

/**
* @brief Invalidate the connId to prevent further access to the channel
*/
IX_STATUS ixAtmdAccRxCfgConnIdInvalidate (unsigned int npeVcId);

/**
* @brief check that receive traffic is enabled
*/
BOOL ixAtmdAccRxCfgVcIsEnabled (unsigned int npeVcId);

/**
* @brief check that receive traffic is disabled
*/
BOOL ixAtmdAccRxCfgVcIsDisabled (unsigned int npeVcId);

/**
* @brief Check if the channel is disconnecting
*/
BOOL ixAtmdAccRxCfgVcIsDisconnecting (unsigned int npeVcId);

/**
* @brief Start receive traffic
*/
void ixAtmdAccRxCfgVcEnable (unsigned int npeVcId);

/**
* @brief Rollback Rx Vc Enable function
* @sa ixAtmdAccRxCfgVcEnable
*/
void ixAtmdAccRxCfgVcEnableRollback (unsigned int npeVcId);

/**
* @brief Stop receive traffic
*/
void ixAtmdAccRxCfgVcDisable (unsigned int npeVcId);

/**
* @brief Set the threshold and enable callback for rxfree queue
*/
IX_STATUS
ixAtmdAccRxCfgRxFreeCallbackSet (unsigned int npeVcId,
                                 unsigned int thresholdLevel,
                                 IxAtmdAccRxVcFreeLowCallback callback);

/**
* @brief disable the rxfree threshold events
*/
IX_STATUS ixAtmdAccRxCfgRxFreeCallbackDisable (unsigned int npeVcId);

/**
* @brief Set the threshold and enable callback for rx queue
*/
IX_STATUS
ixAtmdAccRxCfgRxCallbackSet (IxAtmRxQueueId rxAtmdQueueId,
                             IxQMgrQId rxQmgrQueueId,
                             IxAtmdAccRxDispatcher callback);


/**
* @brief Disable the callback for the rx queue
*/
IX_STATUS
ixAtmdAccRxCfgRxCallbackReset (IxAtmRxQueueId atmRxQueueId,
                               IxQMgrQId qMgrQId);


/**
* @brief Check if any channel is already set in the system
*/
BOOL ixAtmdAccRxCfgRxVcsExist (void);

/**
* @brief Release resources when a channel is down (during a disconnect)
*/
IX_STATUS ixAtmdAccRxCfgResourcesRelease (unsigned int npeVcId);

/**
* @brief send a message to NPE to update the vc lookup table and
*        enable receive
*/
IX_STATUS ixAtmdAccRxCfgNpeVcLookupTableUpdate(unsigned int npeVcId);

#endif /* IX_ATMDRXCFGINFO_P_H */


