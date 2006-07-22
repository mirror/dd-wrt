
/**
 * @file IxEthAal5App_p.h
 *
 * @brief This file contains the private declaration of the IxEthAal5App
 * component's main functions, utilities, etc
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

#ifndef IXETHAAL5APP_P_H
#define IXETHAAL5APP_P_H

/* User defined header files */
#include "IxOsal.h"
#include "IxAtmTypes.h"
#include "IxAtmdAcc.h"
#include "IxAtmm.h"
#include "IxNpeDl.h"

typedef enum
{
    IX_EAA_UTOPIA_MODE = 1,
    IX_EAA_ADSL_MODE
}IxEAAModeType;

/**
*
* @def IX_ETHAAL5APP_CODELET_DEBUG_DO
*
* @brief Execute statements if NDEBUG not defined. To use the debug
* codes add IX_DEBUG=1 during the compilation
* When IX_DEBUG is not called and for optimization purpose, it is preferably 
* to add the do-while statement.
*/
#ifndef NDEBUG
#define IX_ETHAAL5APP_CODELET_DEBUG_DO(statements) statements
#else
#define IX_ETHAAL5APP_CODELET_DEBUG_DO(x) do{}while(0);
#endif

/**
 * @fn ixEthAal5AppUtilsAtmVcRegisterConnect
 *
 * @brief Register and connect an Aal Pdu receive and transmit VC for a
 *        particular port/vpi/vci.
 *
 * This function allows a user to connect to an Aal5 Pdu receive and 
 * transmit service for a particular port/vpi/vci. It registers the callback 
 * and allocates internal resources and a Connection Id to be used in further 
 * API calls related to this VCC.
 *
 * @param port (in) VC identification : logical PHY port [0..7]
 * @param vpi (in) VC identification : ATM Vpi [0..255]
 * @param vci (in) VC identification : ATM Vci [0..65535]
 * @param atmService (in) type of service
 * @param rxQueueId (in) this identifieds which of two Qs the VC
 *     should use when icoming traffic is processed
 * @param rxCallback (in) function called when mbufs are received.
 *     This parameter cannot be a null pointer.
 * @param minimumReplenishCount (in) number of free mbufs to be used with
 *     this channel. Use a high number when the expected traffic rate on
 *     this channel is high, or when the user's mbufs are small, or when
 *     the RxVcFreeLow Notification has to be invoked less often.
 * @param bufferFreeCallback (in) function to be called to return
 *     ownership of buffers to the IxEthAal5AppUtils user.
 * @param userId (in) user Id to use in callback communications
 * @param rxConnId (out) Rx Conn Id passed back from IxAtmdAcc
 * @param txConnId (out) Tx Conn Id passed back from IxAtmdAcc
 *
 * @return @li IX_SUCCESS successful call to ixEthAal5AppUtilsAtmVcRegisterConnect
 * @return @li IX_FAIL parameter error, VC already in use or port is
 * not initialised or some other error occurs during processing.
 *
 */
IX_STATUS
ixEthAal5AppUtilsAtmVcRegisterConnect (
    IxAtmLogicalPort port,
    IxAtmmVc txVc,
    IxAtmmVc rxVc,
    IxAtmdAccAalType aalType,
    IxAtmRxQueueId rxQueueId,
    IxAtmdAccRxVcRxCallback rxCallback,
    unsigned int minimumReplenishCount,
    IxAtmdAccTxVcBufferReturnCallback bufferFreeCallback,
    IxAtmdAccRxVcFreeLowCallback rxFreeLowCallback,
    IxAtmdAccUserId userId,
    IxAtmConnId *rxConnId,
    IxAtmConnId *txConnId);

/**
 * @fn ixEthAal5AppUtilsAtmVcUnregisterDisconnect 
 *
 * @brief Disconnect and unregister an Aal transmit and receive VC.
 *
 * @param rxConnId (in) conn Id of the Rx VC to disconnect and unregister
 * @param txConnId (in) conn Id of the Tx VC to disconnect and unregister
 *
 * @return @li IX_SUCCESS successful call to ixEthAal5AppUtilsAtmVcUnregisterDisconnect
 * @return @li IX_FAIL parameter error, failed to disconnect,
 * not initialised or some other error occurs during processing.
 *
 */
IX_STATUS
ixEthAal5AppUtilsAtmVcUnregisterDisconnect (
    IxAtmConnId rxConnId, 
    IxAtmConnId txConnId);

/**
 * @fn ixEthAal5AppUtilsAtmAllVcsDisconnect
 *
 * @brief Disconnect and unregister all registered VCs.
 *
 * @param none
 *
 * @return @li IX_SUCCESS successful call to ixEthAal5AppUtilsAtmAllVcsDisconnect
 * @return @li IX_FAIL parameter error, failed to disconnect,
 * not initialised or some other error occurs during processing.
 *
 */
IX_STATUS
ixEthAal5AppUtilsAtmAllVcsDisconnect (void);

/**
 * @fn ixEthAal5AppShowTaskDisable
 *
 * @brief Disable the ShowTask's loop
 *
 */
void 
ixEthAal5AppShowTaskDisable(void);

/**
 * @fn ixEAAEthDBRecordsShow
 *
 * @brief Show the Ethernet DB Records
 *
 */
void ixEAAEthDBRecordsShow(void);
#endif 
/* IXETHAAL5APP_P_H */

