/**
 * @file    IxQMgr.h
 *
 * @date    28-July-2005
 *
 * @brief This file contains the public API of IxQMgr component.
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

/* ------------------------------------------------------
   Doxygen group definitions
   ------------------------------------------------------ */
/**
 * @defgroup IxQMgrAPI Intel (R) IXP400 Software Queue Manager (IxQMgr) API
 *
 * @brief The public API for the IXP400 qmgr component.
 *
 * IxQMgr is a low level public interface to the Queue Manager
 *
 * @{
 */

#ifndef IXQMGR_H
#define IXQMGR_H

/*
 * User defined include files
 */

#include "IxOsal.h"

/*
 * Typedefs
 */
 
/**
 * @ingroup IxQMgrAPI
 * @enum IxQMgrDispatcherState
 *
 * @brief Queue dispatcher State.
 *
 * Indicates the Queue dispatcher state busy or free.
 *
 */
typedef enum{
   IX_QMGR_DISPATCHER_LOOP_FREE,
   IX_QMGR_DISPATCHER_LOOP_BUSY
}IxQMgrDispatcherState;
/**
 * @ingroup IxQMgrAPI
 * 
 * @enum IxQMgrDispatchGroup
 *
 * @brief QMgr queue group select identifiers.
 *
 * This enum defines the groups into which the queues are divided. Each group
 * contains 32 queues. Each qmgr interface function that works on a group of 
 * queues takes one of these identifiers as a parameter.
 *
 */
typedef enum
{
    IX_QMGR_GROUP_Q0_TO_Q31 = 0,  /**< Queues 0-31  */
    IX_QMGR_GROUP_Q32_TO_Q63,	  /**< Queues 32-63  */
    IX_QMGR_GROUP_MAX
} IxQMgrDispatchGroup;

/**
 * @ingroup IxQMgrAPI
 * 
 * @def IX_QMGR_QUELOW_GROUP 
 *
 * @brief Backward compatability for QMgr queue group select identifier,
 *        IX_QMGR_QUELOW_GROUP.
 *
 * This #define provides backward compatability for the IX_QMGR_QUELOW_GROUP,
 * which has been replaced by IX_QMGR_GROUP_Q0_TO_Q31.
 *
 * This #define has been DEPRECATED and will be removed in a future release.
 *
 */
#define IX_QMGR_QUELOW_GROUP IX_QMGR_GROUP_Q0_TO_Q31

/**
 * @ingroup IxQMgrAPI
 * 
 * @def IX_QMGR_QUEUPP_GROUP 
 *
 * @brief Backward compatability for QMgr queue group select identifier,
 *        IX_QMGR_QUEUPP_GROUP.
 *
 * This #define provides backward compatability for the IX_QMGR_QUEUPP_GROUP,
 * which has been replaced by IX_QMGR_GROUP_Q32_TO_Q63.
 *
 * This #define has been DEPRECATED and will be removed in a future release.
 *
 */
#define IX_QMGR_QUEUPP_GROUP IX_QMGR_GROUP_Q32_TO_Q63 

/**
 * @ingroup IxQMgrAPI
 *
 * @typedef IxQMgrDispatcherFuncPtr
 *
 * @brief QMgr Dispatcher Loop function pointer.
 *
 * This defines the interface for QMgr Dispather functions.
 *
 * @param group @ref IxQMgrDispatchGroup [in] - the group of the 
 *                  queue of which the dispatcher will run   
 */
typedef void (*IxQMgrDispatcherFuncPtr)(IxQMgrDispatchGroup group);

/*
 * Function Prototypes
 */

/* ------------------------------------------------------------
   Initialisation related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrInit (void)
 *
 * @brief Initialise the QMgr.
 *
 * This function must be called before any other QMgr function. It
 * sets up internal data structures.
 *
 * @return @li IX_SUCCESS, the IxQMgr successfully initialised
 * @return @li IX_FAIL, failed to initialize the Qmgr
 *
 */
PUBLIC IX_STATUS
ixQMgrInit (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrUnload (void)
 *
 * @brief Uninitialise the QMgr.
 *
 * This function will perform the tasks required to unload the QMgr component
 * cleanly. This includes unmapping kernel memory and unconfigures the access
 * layer component's queue configuration.
 * 
 * This should be called before a soft reboot or unloading of a kernel module.
 *
 * @pre It should only be called if @ref ixQMgrInit has already been called.
 *
 * @post No QMgr functions should be called until ixQMgrInit is called again.
 *
 * @return @li IX_SUCCESS, the IxQMgr successfully uninitialised
 * @return @li IX_FAIL, failed to uninitialize the Qmgr
 *
 */
PUBLIC IX_STATUS
ixQMgrUnload (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrShow (void)
 *
 * @brief Describe queue configuration and statistics for active queues.
 *
 * This function shows active queues, their configurations and statistics.
 *
 * @return @li void
 *
 */
PUBLIC void
ixQMgrShow (void);

/* ------------------------------------------------------------
   Queue dispatch related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrDispatcherLoopGet (IxQMgrDispatcherFuncPtr *qDispatcherFuncPtr)
 *
 * @brief Get QMgr DispatcherLoopRun for respective silicon device
 *
 * This function gets a function pointer to ixQMgrDispatcherLoopRunB0() for IXP42X B0
 * Silicon. However if live lock prevention is enabled a function pointer to
 * ixQMgrDispatcherLoopRunB0LLP() is given.
 *
 * @param *qDispatchFuncPtr @ref IxQMgrDispatcherFuncPtr [out]  - 
 *              the function pointer of QMgr Dispatcher
 *
 */
PUBLIC void
ixQMgrDispatcherLoopGet (IxQMgrDispatcherFuncPtr *qDispatcherFuncPtr);

#include "../qmgr/IxQMgr_sp.h"

/**
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrDispatcherLoopEnable(void)
 *
 * @brief Enables the Queue Manager Dispatcher.
 * @note API used by the ixErrHdlAcc to re-enable the 
 * queue manager dispatcher.
 * @return None
 *
 */
PUBLIC void 
ixQMgrDispatcherLoopEnable(void);

/**
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrDispatcherLoopDisable(void)
 *
 * @brief Disables the Queue Manager Dispatcher 
 * @note API used by the ixErrHdlAcc to disable the 
 * queue manager dispatcher to prevent QM Read/Write 
 * access.
 * @return None
 *
 */
PUBLIC void 
ixQMgrDispatcherLoopDisable(void);

/**
 * @ingroup IxQMgrAPI
 *
 * @fn IxQMgrDispatcherState ixQMgrDispatcherLoopStatusGet(void )
 *
 * @brief Gets the Queue dispatcher Loop status.
 * @return Returns an enum @IxQMgrDispatcherState that indicates
 *          the dispatcher state (RUN or STOP)
 *
 */
PUBLIC
IxQMgrDispatcherState ixQMgrDispatcherLoopStatusGet(void );

/**
 * @ingroup IxQMgrAPI
 *
 * @fn void ixQMgrDispatcherInterruptModeSet(BOOL mode)
 *
 * @brief Notifies the ixQMgr that the Dispatcher is in Interrupt mode.
 * @note This function records whether the Queue Dispatcher is bind 
 * to the QM Interrupt notification IRQ. 
 * 
 * @param BOOL mode [in] - TRUE - dispatcher mode is bind to Queue Manager
 *                         IRQ for queue notifications (0-63). 
 *                         FALSE - dispatcher is in poll mode.
 * @return None
 *
 */
PUBLIC
void ixQMgrDispatcherInterruptModeSet(BOOL mode);

#endif /* IXQMGR_H */

/**
 * @} defgroup IxQMgrAPI
 */
