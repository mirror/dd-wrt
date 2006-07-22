/**
 * @file IxAtmdTxCfgInfo_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Tx Configuration
 *
 * This file contains the functions to initialize the internal structures
 * needed during the TX datapath processing.
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

#ifndef IX_ATMDTXCFGINFO_P_H
#define IX_ATMDTXCFGINFO_P_H

#include "IxQMgr.h"
#include "IxAtmdAccCtrl.h"
#include "IxAtmdPortMgmt_p.h"

/**
* @brief Tx data initialisation
*/
IX_STATUS
ixAtmdAccTxCfgInfoInit (void);

/**
* @brief Tx data full configuration and stats display
*/
void
ixAtmdAccTxCfgInfoStatsShow (void);

/** 
* @brief display the tx configuration
*/
void
ixAtmdAccTxCfgInfoPortShow (IxAtmLogicalPort port);

/** 
* @brief display the tx configuration
*/
void
ixAtmdAccTxCfgInfoChannelShow (IxAtmLogicalPort port);

/**
* @brief Tx data  reset
*/
void
ixAtmdAccTxCfgInfoStatsReset (void);

/**
* @brief Get the connection Id associated with a channel
*        identified by the port/vpi/vci parameters
* @return @li IX_SUCCESS connection found and connection ID
*         is returned
* @return @li IX_FAIL connection not found
*/
IX_STATUS
ixAtmdAccTxCfgConnIdFind (IxAtmLogicalPort port,
                                    unsigned int vpi,
                                    unsigned int vci,
                                    IxAtmConnId * connIdPtr);

/**
* @brief Check if this port/vpi/vci is already configured
* @return @li TRUE connection found
* @return @li FALSE connection not found
*/
BOOL
ixAtmdAccTxCfgVcConfigured (IxAtmLogicalPort port,
                                  unsigned int vpi,
                                  unsigned int vci);

/**
* @brief Check if there is any connection already set for this port
* @return @li TRUE a connection is found
* @return @li FALSE no connection found
*/
BOOL
ixAtmdAccTxCfgPortVcsExist (IxAtmLogicalPort port);

/**
* @brief Check if there is any connection already set in the system
* @return @li TRUE a connection is found
* @return @li FALSE no connection found
*/
BOOL
ixAtmdAccTxCfgVcsExist (void);

/**
* @brief Get a free tx channel and allocate a new connection id
* @return @li IX_SUCCESS a free channel is found and allocated
* @return @li IX_ATMDACC_OVERLOADED no free channel found
*/
IX_STATUS
ixAtmdAccTxCfgFreeChannelGet (IxAtmdAccAalType aalServiceType, IxAtmConnId * connIdPtr);

/**
* @brief initialise the channels parameters
* @return @li IX_SUCCESS initialisation is succesfull
* @return @li IX_FAIL initialisation failed (unable to allocate from
*            memory or unexpected error occured)
*/
IX_STATUS
ixAtmdAccTxCfgChannelSet (IxAtmConnId connId,
                          IxAtmSchedulerVcId schedulerVcId,
                          IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci,
                          IxAtmdAccAalType aalServiceType,
                          IxAtmdAccUserId userId,
                          IxAtmdAccTxVcBufferReturnCallback txDoneCallback);

/**
* @brief reset the channels parameters to the default values
* @return @li IX_SUCCESS reset is succesfull
* @return @li IX_FAIL reset failed (unable to deallocate from
*            memory or unexpected error occured)
*/
IX_STATUS ixAtmdAccTxCfgChannelReset (unsigned int txId);

/**
* @brief from the connection Id, get the index in the pool of channel descriptor
* @return @li IX_SUCCESS connection Id is valid
* @return @li IX_FAIL connection Id is not valid (may be obsolete)
*/
IX_STATUS
ixAtmdAccTxCfgIndexGet (IxAtmConnId connId,
                        unsigned int *txIdPtr);

/**
* @brief check if all resources are back to the sw queue
* @return @li IX_SUCCESS all resources are back to the sw queue
* @return @li IX_FAIL resources are still in the qmgr queue, or hold by NPE
*/
BOOL
ixAtmdAccTxCfgFreeResourcesCheck (unsigned int txId);

/**
* @brief Invalidate a connection ID such a way that further attempts
*        to use it will be discarded. This is use during the disconnect
*        process
* @return @li IX_SUCCESS the connection ID is checked and is now altered.
* @return @li IX_FAIL the connection ID supplied is not valid
* @note - This funtion is idempotent
*/
void
ixAtmdAccTxCfgConnIdInvalidate (unsigned int txId);

/**
* @brief Register a TxDone callback
* @return @li IX_SUCCESS The callback is registered
* @return @li IX_FAIL an unexpected arror occured during the callback
*         registration
*/
IX_STATUS
ixAtmdAccTxCfgTxDoneCallbackRegister (unsigned int thresholdLevel,
                                      IxAtmdAccTxDoneDispatcher callback);

/**
* @brief Register a TxDone callback Unregister
* @return @None
*/
void
ixAtmdAccTxCfgTxDoneCallbackUnregister (void);


/**
* @brief Register a TxLow callback for this port
* @return @li IX_SUCCESS The callback is registered
* @return @li IX_FAIL an unexpected arror occured during the callback
*         registration
*/
IX_STATUS
ixAtmdAccTxCfgTxCallbackRegister (IxAtmLogicalPort port,
                                  unsigned int thresholdLevel,
                                  IxAtmdAccPortTxLowCallback callback);

/**
* @brief Unregister a TxLow callback for this port
* @return NONE
*/
void
ixAtmdAccTxCfgTxCallbackUnregister (IxAtmLogicalPort port);


/**
* @brief Internal QMgr callback for TX Low event
*/
void
ixAtmdAccTxLowCallBack (IxQMgrQId qId,
                             IxQMgrCallbackId cbId);

/**
* @brief Internal QMgr callback for TX Done event
*/
void
ixAtmdAccTxDoneCallBack (IxQMgrQId qId,
                              IxQMgrCallbackId cbId);

/**
* @brief Dummy scheduler
* @return @li IX_SUCCESS a vcid is provided
* @return @li IX_FAIL cannot occur, this is use for compatibility
*         with the atm scheduler
*/
IX_STATUS
ixAtmdAccTxDummyVcIdGet (IxAtmLogicalPort port,
                                   unsigned int vpi,
                                   unsigned int vci,
                                   IxAtmConnId connId,
                                   IxAtmSchedulerVcId* schedulerVcIdPtr);

/**
* @brief Dummy scheduler
* @return @li IX_SUCCESS a demand update is done : this triggers
*         the immediate transmission of the PDU submitted.
* @return @li IX_FAIL an unexpected error occured
*/
IX_STATUS
ixAtmdAccTxDummyDemandUpdate (IxAtmLogicalPort port,
                                        int vcId,
                                        unsigned int numberOfCells);

/**
* @brief Dummy scheduler
* @return none
*/
void
ixAtmdAccTxDummyDemandClear (IxAtmLogicalPort port,
                             IxAtmSchedulerVcId schedulerVcId);

/**
* @brief Register a scheduler for this port
* @return @li IX_SUCCESS registration is complete
* @return @li IX_FAIL registration failed
*/
void
ixAtmdAccTxCfgSchCallbackRegister (IxAtmLogicalPort port,
                                   IxAtmdAccTxVcDemandUpdateCallback queueUpdate,
                                   IxAtmdAccTxVcDemandClearCallback vcClear,
                                   IxAtmdAccTxSchVcIdGetCallback vcIdGet);

/**
* @brief Unregister a scheduler for this port
* @return @li IX_SUCCESS unregistration is complete
* @return @li IX_FAIL unregistration failed
*/
void
ixAtmdAccTxCfgSchCallbackUnregister (IxAtmLogicalPort port);


/**
* @brief Clean the TX VC queue from the entries realted to the channel
*        being disconnected
* @return @li IX_SUCCESS cleaning is complete, or not done
* @return @li IX_FAIL an unexpected error occured during processing
*/
IX_STATUS
ixAtmdAccTxCfgPortResourcesRelease (IxAtmConnId connId);

/**
* @brief Check if a port is enabled
* @return @li TRUE enabled
* @return @li FALSE not enabled
*/
BOOL
ixAtmdAccTxPortEnabledQuery(IxAtmLogicalPort port);

/**
* @brief Check if a port is disabled
* @return @li TRUE disabled
* @return @li FALSE not disabled
*/
BOOL
ixAtmdAccTxPortDisabledQuery(IxAtmLogicalPort port);

/**
* @brief Change the state of a port
* @return @li IX_SUCCESS state change succeeded
* @return @li IX_FAIL state change failed, state was not changed
*/
IX_STATUS
ixAtmdAccTxPortStateChangeHandler(IxAtmLogicalPort port,
                                  IxAtmdAccPortState requestedState);

/**
* @brief : Handle port setup initiated by PortMgmt
* @return @li none
*/
void
ixAtmdAccTxPortSetupNotifyHandler(unsigned int numPort);

/**
*
* @brief  prototype to get a scheduler vc id
*
* @param port (in) Specifies the ATM logical port on which the VC is
*        established
* @param vpi (in) Specifies the ATM vpi on which the VC is established
* @param vci (in) Specifies the ATM vci on which the VC is established
* @param connId (in) specifies the IxAtmdAcc connection Id already
*        associated with this VC
* @param vcId (out) pointer to a vcId
*
* @return IX_SUCCESS returns a Scheduler vcId for this VC
* @return IX_FAIL cannot process scheduling for this VC.
*                 the contents of vcId is unspecified
*
*/
IX_STATUS
ixAtmdAccTxCfgSchVcIdGet (IxAtmLogicalPort port,
                               unsigned int vpi,
                               unsigned int vci,
                               IxAtmConnId connId,
                               IxAtmSchedulerVcId *schedulerVcIdPtr);

/**
* @brief  prototype to  remove all currently queued cells from a
* registered VC
*
* @param connId (in) specifies the IxAtmdAcc connection Id already
*        associated with this VC
*
* @return none
*
*/
void
ixAtmdAccTxCfgVcDemandClear (IxAtmConnId connId);

/**
* @brief  prototype to tell the scheduler about a connect failure
*
* @param connId (in) specifies the IxAtmdAcc connection Id already
*        associated with this VC
* @param port (in) specifies the port
* @param schedulerVcId (in) specifies the scheduler Vc to be used
*
* @return none
*
*/
void
ixAtmdAccTxCfgVcDemandCancel (IxAtmConnId connId,
                             IxAtmLogicalPort port,
                             IxAtmSchedulerVcId schedulerVcId);

#endif /* IX_ATMDTXCFGINFO_P_H */


