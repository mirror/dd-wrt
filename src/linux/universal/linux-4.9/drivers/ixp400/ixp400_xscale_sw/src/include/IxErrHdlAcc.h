/**
 * @file    IxErrHdlAcc.h
 *
 * @brief this file contains the public API of @ref IxErrHdlAcc component
 *
 * @par
 * IXP400 SW Release Crypto version 2.4
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

/*
 * Put the user defined include files required
 */
#ifndef IxErrHdlAcc_H
#define IxErrHdlAcc_H
#include "IxOsal.h"
 /**
 * @defgroup IxErrHdlAcc Intel (R) IXP400 Soft-Error Handler Driver (ixErrHdlAcc) API
 *
 * @brief The public API  for the IXP400 Soft-Error Handler Driver (ixErrHdlAcc)
 *
 * @{
 */

 /**
 * @ingroup IxErrHdlAcc
 * @enum IxErrHdlAccErrorEventType
 * @brief Definition of the Soft-Error Event Types
 */
typedef enum{
 IX_ERRHDLACC_NPEA_ERROR,  /* 0*/
 IX_ERRHDLACC_NPEB_ERROR,  /* 1*/
 IX_ERRHDLACC_NPEC_ERROR,  /* 2*/
 IX_ERRHDLACC_AQM_ERROR,   /* 3*/
 IX_ERRHDLACC_NO_ERROR     /* 4*/
}IxErrHdlAccErrorEventType;

/**
 * @ingroup IxErrHdlAcc
 * @brief Definition of the NPE A Error Mask Bit
 */
#define IX_ERRHDLACC_NPEA_ERROR_MASK_BIT            (1<<IX_ERRHDLACC_NPEA_ERROR)

/**
 * @ingroup IxErrHdlAcc
 * @brief Definition of the NPE B Error Mask Bit
 */
#define IX_ERRHDLACC_NPEB_ERROR_MASK_BIT            (1<<IX_ERRHDLACC_NPEB_ERROR)

/**
 * @ingroup IxErrHdlAcc
 * @brief Definition of the NPE C Error Mask Bit
 */
#define IX_ERRHDLACC_NPEC_ERROR_MASK_BIT            (1<<IX_ERRHDLACC_NPEC_ERROR)

/**
 * @ingroup IxErrHdlAcc
 * @brief Definition of the AQM Error Mask Bit
 */
#define IX_ERRHDLACC_AQM_SRAM_PARITY_ERROR_MASK_BIT (1<<IX_ERRHDLACC_AQM_ERROR)


/**
 * @ingroup IxErrHdlAcc
 * @enum IxErrHdlAccNPEId
 * @brief Definition of the NPE ID
 */
typedef enum{
 IX_ERRHDLACC_NPEA, /* 0*/
 IX_ERRHDLACC_NPEB, /* 1*/
 IX_ERRHDLACC_NPEC  /* 2*/
}IxErrHdlAccNPEId;

/**
 * @ingroup IxErrHdlAcc
 * @enum IxErrHdlAccEnableEvent
 * @brief Definition of the Soft-Error Event Enable/Disable
 */
typedef enum{
  IX_ERRHDLACC_EVT_DISABLE = 0, /* Disable the Event handling*/ 
  IX_ERRHDLACC_EVT_ENABLE       /* Enalbe the Event handling*/
}IxErrHdlAccEnableEvent;

/**
 * @ingroup IxErrHdlAcc
 * @typedef IxErrHdlAccRecoveryDoneCallback
 * @brief Definition of the Recovery Done Call-back type Function pointer 
 * with IxErrHdlAccErrorEventType type as the argument.
 */
typedef void (*IxErrHdlAccRecoveryDoneCallback)(IxErrHdlAccErrorEventType);

/**
 * @ingroup IxErrHdlAcc
 * @typedef IxErrHdlAccFuncHandler
 * @brief Definition of the Event handler type function pointer
 */
typedef IX_STATUS (*IxErrHdlAccFuncHandler)(void);

 /**
 * @brief Debug Statistical Data Structure
 */
typedef struct{

 UINT32 totalNPEAEvent;      /* Total NPE A Error Events*/
 UINT32 totalNPEBEvent;      /* Total NPE B Error Events*/
 UINT32 totalNPECEvent;      /* Total NPE C Error Events*/
 UINT32 totalAQMEvent;       /* Total AQM Error Events */
 UINT32 totalNPEARecoveryPass; 
 UINT32 totalNPEBRecoveryPass; 
 UINT32 totalNPECRecoveryPass;
 UINT32 totalAQMRecoveryPass;
}IxErrHdlAccRecoveryStatistics;

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccEnableConfigGet (UINT32* configMask)
 *
 *
 * @brief Gets the configuration word enable setup.
 
 * @note
 *    This API gets the soft-error configuration setup in the module 
 *    that was set by the call to the @ref ixErrHdlAccEnableConfigSet API.
 *
 * - Reentrant    - no
 * - ISR Callable - yes
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 * @param configMask UINT32* [in] Data UINT32 pointer to store
 * the configuration Mask whereby, any bits set or clear indicates
 * an enable or disable of a particular Error event type. 
 *                                
 *
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : An uninitialized ixErrHdlAcc module or
 *             a NULL pointer provided to the 1st 
               parameter argument of the function.
 *
 * @sa ixErrHdlAccEnableConfigSet
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccEnableConfigGet(UINT32* configMask);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccEnableConfigSet(UINT32 configMask)
 *
 *
 * @brief Configures the ixErrHdlAcc module's configuration,
 *        to enable or disable a maximum total of 32 Soft Error Events
 *        types.
 * - Reentrant    - no
 * - ISR Callable - yes
 *
 * @note
 *    @n An example to enable NPE A , NPE B and NPE C Error handling.
 *    @n ixErrHdlAccEnableConfigSet(IX_ERRHDLACC_NPEA_ERROR_MASK_BIT
 *                            @n |IX_ERRHDLACC_NPEB_ERROR_MASK_BIT
 *                            @n |IX_ERRHDLACC_NPEC_ERROR_MASK_BIT);
 *
 * @pre ixErrHdlAcc must be initialized by calling the
        ixErrHdlAccInit prior to using this API.
 *   
 * @param configMask UINT32 [in] - a 32 bit Configuration word
          where a bit N set will enable the event N 
                a bit N clear will disable the event N
                N=0, 1, 2, ...31.
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : A fail if the module is not initialized.
 * 
 * @sa ixErrHdlAccEnableConfigGet
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccEnableConfigSet(UINT32 configMask);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccNPEReset(IxErrHdlAccNPEId npeID)
 *
 *
 * @brief Reset the NPE via the  Expansion bus
 * controller FUSE register. The API sets the FUSE Bit that
 * resets the NPE. It waits for the NPE to reset and then,
 * turns off the FUSE bit to re-enable the NPE.
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre npeID must be a valid NPE ID where,
    0 - NPEA, 1 - NPEB & 2 - NPEC
 *   
 * @param npeID IxErrHdlAccNPEId [in] - NPE ID for NPEA, NPEB and NPEC. 
 * See @ref IxErrHdlAccNPEId
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : Invalid NPEID, Failure to Reset the NPE due
               to time-out while waiting for the NPE to 
               reset and for the NPE FUSE Bit to turn off.
 *
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccNPEReset(IxErrHdlAccNPEId npeID);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccInit(void)
 *
 *
 * @brief Initializes the ixErrHdlAcc component
 *
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The ixErrHdlAcc module should not already been 
 *      loaded or initialized. 
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Module has initialized or already initialized.
 * - IX_FAIL : Failure to create thread, create a semaphore
 *             object & initializing of the module which is 
 *             already initialized.
 * @sa ixErrHdlAccUnload
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccInit(void);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccUnload(void)
 *
 *
 * @brief Unloads the ixErrHdlAccModule, destroying the
 *        semaphore object and killing all threads/tasks.
 *
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 *
 * @return IX_STATUS
 * - IX_SUCCESS : Module has unloaded or already unloaded.
 * - IX_FAIL : Failure to terminate thread, destroy a semaphore
 *             objects and to de-allocate any allocated memory.
 *              
 * @sa ixErrHdlAccInit
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccUnload(void);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccErrorHandlerGet (IxErrHdlAccErrorEventType event,IxErrHdlAccFuncHandler* handler)
 *
 *
 * @brief API to get the event handler API to be called
 * within the interrupt error ISR or task level to initiate 
 * a soft-error handling of the error.
 *
 * @note
 *    For an NPE Error, the event handlers can be called 
 *    to reset the NPE irespective of whether there is 
 *    an NPE parity error or an AHB error. This API can
 *    be used to restore the NPE in a need to need basis. 
 *
 * - Reentrant    - no
 * - ISR Callable - yes
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 * @param event IxErrHdlAccErrorEventType [in] An Event ID defined as:
 * @ref IxErrHdlAccErrorEventType
 *
 * @param handler IxErrHdlAccFuncHandler* [in] A pointer to a Event Error 
 *            handler defined as @ref IxErrHdlAccFuncHandler
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : An uninitialized ixErrHdlAcc module or
 *             a NULL pointer provided to the 2nd 
 *             parameter argument of the function or
 *             an invalid Event ID (1st Parameter) 
 *
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccErrorHandlerGet(IxErrHdlAccErrorEventType event, IxErrHdlAccFuncHandler* handler);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccStatusSet(UINT32 status)
 *
 *
 * @brief Sets the current Error condition status results
 *        that indicates if there was an error in handling
 *        a soft-error.
 * @warning
 * This function  must be used only in conditions whereby, 
 * the client performs it’s own error handling in addition 
 * to the ones provided by the ixErrHdlAcc component which
 * may result in recovery failure. This is particularly 
 * applicable for customized NPE firmware done by customers.
 * The API then must be called to indicate an error in the 
 * hardware component impacted.
 *
 * @note
 * Usage guide: This API can be used to set the Event Error WORD
 * condition where, any bit N set in the status WORD (32bit) shall 
 * indicate an error. Currently only bit N = 0, 1, 2 & 3 is valid.
 * @n An Example,    
 *       @n This enables NPE A and NPE C Error handling.
 *       @n ixErrHdlAccStatusSet(IX_ERRHDLACC_NPEA_ERROR_MASK_BIT
 *                            |IX_ERRHDLACC_NPEC_ERROR_MASK_BIT);
 *                 
 *
 * - Reentrant    - no
 * - ISR Callable - yes
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 * @param status UINT32 [in] UINT32 (WORD)
 *    to write the current Event Status condition WORD
 *                                   
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : An uninitialized ixErrHdlAcc module
 * @sa ixErrHdlAccStatusGet
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccStatusSet(UINT32 status);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccStatusGet(UINT32* status)
 *
 *
 * @brief Gets the current Error condition status results
 *        that indicates if there was an error in handling
 *        a soft-error.
 * @note
 * Usage guide:
 *       This API can be used to get the Event Error WORD
 *        condition where,
 *       Any bit N set in the status WORD (32bit) shall 
 *       indicate an error.
 *       Currently only bit N = 0, 1, 2 & 3 is valid.
 *       
 * @par Usage Example:    
 *       This example gets the current status and checks
 *       for NPE A Error.
 *       @n ixErrHdlAccStatusGet(&status);
 *       @n if(status&IX_ERRHDLACC_NPEA_ERROR_MASK_BIT)
 *       @n {< NPE A Error> }
 *
 * - Reentrant    - no
 * - ISR Callable - yes
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 * @param status UINT32 [in] Pointer to a UINT32 (WORD)
 *    to write the current Event Status condition WORD
 *                                   
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : An uninitialized ixErrHdlAcc module or
 *             a NULL pointer provided to the 1st 
 *             parameter.
 * @sa ixErrHdlAccStatusSet
 * <hr>
 */
 PUBLIC IX_STATUS ixErrHdlAccStatusGet(UINT32* status);
 


/**
 * @ingroup IxErrHdlAcc
 *
 * @fn void ixErrHdlAccStatisticsShow(void)
 *
 *
 * @brief Displays the Debug/Measurement statistics
 *        recorded by the module.
 *        
 *
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 *
 * @return NONE
 * @sa ixErrHdlAccStatisticsClear
 * @sa ixErrHdlAccStatisticsGet
 * <hr>
 */
PUBLIC void ixErrHdlAccStatisticsShow(void);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccStatisticsGet(IxErrHdlAccRecoveryStatistics* statistics)
 *
 *
 * @brief API to retrieve the statistics recorded by
 *        the module.
 * @note
 * See @ref IxErrHdlAccRecoveryStatistics for details
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 * @param statistics IxErrHdlAccRecoveryStatistics [in]
 *
 * @return IX_STATUS
 * - IX_SUCCESS
 * - IX_FAIL : An uninitialized ixErrHdlAcc module or
 *             a NULL pointer provided to the 1st 
 *             parameter argument of the function.
 * @sa ixErrHdlAccStatisticsClear
 * @sa ixErrHdlAccStatisticsShow
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccStatisticsGet(IxErrHdlAccRecoveryStatistics* statistics);

/**
 * @ingroup IxErrHdlAcc
 *
 * @fn void ixErrHdlAccStatisticsClear(void)
 *
 *
 * @brief Resets and Clears the Debug/Measurement statistics
 *        recorded by the module.
 *        
 *
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 *
 * @return NONE
 * @sa ixErrHdlAccStatisticsGet
 * @sa ixErrHdlAccStatisticsShow
 * <hr>
 */
PUBLIC void ixErrHdlAccStatisticsClear(void);

 /**
 * @ingroup IxErrHdlAcc
 *
 * @fn IX_STATUS ixErrHdlAccCallbackRegister(IxErrHdlAccRecoveryDoneCallback callback)
 *
 * @brief API to register a Completion done that is called after every completion of
 * an error handling.    
 *
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The ixErrHdlAcc module should have already 
 *      been intialized prior to calling this API.
 *   
 * @param callback IxErrHdlAccRecoveryDoneCallback [in] Call-back function to be called by the module 
 *        upon completion of a recovery task. See @ref IxErrHdlAccRecoveryDoneCallback
 *
 * @return IX_STATUS
 * - IX_SUCCESS: Operation success
 * - IX_FAIL : An uninitialized ixErrHdlAcc module or
 *             a NULL pointer provided to the 1st 
 *             parameter argument of the function.
 *
 * <hr>
 */
PUBLIC IX_STATUS ixErrHdlAccCallbackRegister(IxErrHdlAccRecoveryDoneCallback callback);

#endif
/**
 *@}
 */
