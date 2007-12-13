/**
 * @file IxHssAccPCM_p.h
 *
 * @author Intel Corporation
 * @date 13 Dec 2001
 *
 * @brief This file contains the private API of the HSS Packetised
 *  Connection Manager module
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


/**
 * @defgroup IxHssAccPCM_p IxHssAccPCM_p
 *
 * @brief The private API for the HssAccess Packetised Connection Manager 
 *        module
 * 
 * @{
 */

#ifndef IXHSSACCPCM_P_H
#define IXHSSACCPCM_P_H

/*
 * inline definition
 */
#ifdef IXHSSACCPCM_C
/* Non-inline functions will be defined in this translation unit. 
 * Reason is that in GNU Compiler, if the optimization flag is turned off, 
 * all extern inline functions will not be compiled.
 */
#   ifndef __wince
#       ifndef IXHSSACCPCM_INLINE
#           define IXHSSACCPCM_INLINE
#	endif
#   else
#	ifndef IXHSSACCPCM_INLINE
#       define IXHSSACCPCM_INLINE IX_OSAL_INLINE_EXTERN
#	endif
#   endif /* __wince*/
#else
#   ifndef IXHSSACCPCM_INLINE
#       ifdef _DIAB_TOOL
            /* DIAB does not allow both the funtion prototype and
             * defintion to use extern
             */
#           define IXHSSACCPCM_INLINE IX_OSAL_INLINE
#       else
#           define IXHSSACCPCM_INLINE IX_OSAL_INLINE_EXTERN
#       endif
#   endif
#endif /* IXHSSACCPCM_C */

/*
 * User include header files
 */
#include "IxHssAcc.h"
#include "IxOsal.h"
#include "IxQMgr.h"

#include "IxHssAccCommon_p.h"

#include "IxNpeMh.h"

/**
 * Variable declarations.  Externs are followed by static variables.
 */
extern IxQMgrQId ixHssAccPCMRxFreeQId[IX_HSSACC_HSS_PORT_MAX][IX_HSSACC_HDLC_PORT_MAX];
extern IxQMgrQId ixHssAccPCMTxQId[IX_HSSACC_HSS_PORT_MAX][IX_HSSACC_HDLC_PORT_MAX];

/*
 * defines
 */

/**
 * @def IX_HSSACC_PKT_CBID_HSS_OFFSET
 *
 * @brief The offset used to bit shift the hssPortId into QMgr callback Id
 */
#define IX_HSSACC_PKT_CBID_HSS_OFFSET              16

/**
 * @def IX_HSSACC_PKT_CBID_HDLC_MASK
 *
 * 
 * @brief The mask used to insert hdlcPortId into the QMgr callback Id

 */
#define IX_HSSACC_PKT_CBID_HDLC_MASK               0x0000FFFF


/**
 * @fn IX_STATUS ixHssAccPCMConnect (IxHssAccHssPort hssPortId, 
		    IxHssAccHdlcPort hdlcPortId, 
		    BOOL hdlcFraming,
		    IxHssAccHdlcMode hdlcMode,
		    BOOL hdlcBitInvert,    
		    unsigned blockSizeInWords,
		    UINT32 rawIdleBlockPattern,
		    IxHssAccPktHdlcFraming hdlcTxFraming, 
		    IxHssAccPktHdlcFraming hdlcRxFraming, 
		    unsigned frmFlagStart,
		    IxHssAccPktRxCallback rxCallback,
		    IxHssAccPktUserId rxServiceUserId,
		    IxHssAccPktRxFreeLowCallback rxFreeLowCallback, 
		    IxHssAccPktUserId rxFreeLowUserId,
		    IxHssAccPktTxDoneCallback txDoneCallback,
		    IxHssAccPktUserId txDoneUserId)
 *
 * @brief This function configures the NPE to use the packet pipes the way 
 * a client wishes. The PCM will then configure the NPE to the way the 
 * client requested.
 *
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two identical 
 * ports (0-1).
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port and 
 * it corresponds to the physical E1/T1 channel i.e. 0, 1, 2, 3 
 * @param BOOLEAN hdlcFraming (in) - This value determines whether the service 
 * will use HDLC data processing or the RAW data type i.e. no HDLC processing, 
 * which can also be for debug.
 * @param IxHssAccHdlcMode hdlcMode (in) - This structure contains 56Kbps, HDLC-mode
 * configuration parameters
 * @param BOOLEAN hdlcBitInvert (in) - This value determines whether bit inversion
 * will occur between HDLC and HSS co-processors i.e. post-HDLC processing for
 * transmit and pre-HDLC processing for receive, for the specified HDLC Termination
 * Point
 * @param unsigned blockSizeInWords (in) -  The max tx/rx block size 
 * @param UINT32 rawIdleBlockPattern (in) -  Tx idle pattern in raw mode 
 * @param IxHssAccPktHdlcFraming hdlcTxFraming (in) - This structure contains 
 * the following information required by the NPE to configure the HDLC 
 * co-processor for TX: Idle Tx type, CRC type, and bit Endianness
 * @param IxHssAccPktHdlcFraming hdlcRxFraming (in) - This structure contains 
 * the following information required by the NPE to configure the HDLC 
 * co-processor for RX: Idle Tx type, CRC type, and bit Endianness
 * @param unsigned frmFlagStart (in) - This parameter is the number of flags 
 * that is placed before a frame when it is transmitted (0, 1, 2)
 * @param IxHssAccPktRxCallback rxCallback (in) - Pointer to 
 * the clients packet receive function.
 * @param IxHssAccPktUserId rxUserId (in) - The client supplied rx value
 * to be passed back as an argument to the supplied rxCallback
 * @param IxHssAccPktRxFreeLowCallback rxFreeLowCallback (in) - Pointer to 
 * the clients Rx free buffer request function.  If NULL, assume client will 
 * trigger independently.
 * @param IxHssAccPktUserId rxFreeLowUserId (in) - The client supplied RxFreeLow value
 * to be passed back as an argument to the supplied rxFreeLowCallback
 * @param IxHssAccPktTxDoneCallback txDoneCallback (in) - Pointer to the 
 * clients Tx done callback function
 * @param IxHssAccPktUserId txDoneUserId (in) - The client supplied txDone value
 * to be passed back as an argument to the supplied txDoneCallback
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 *         - IX_RESOURCE_ERR The function did not execute successfully due
 *                          to a resource error
 * 
 */
IX_STATUS
ixHssAccPCMConnect (IxHssAccHssPort hssPortId, 
		    IxHssAccHdlcPort hdlcPortId, 
		    BOOL hdlcFraming,
		    IxHssAccHdlcMode hdlcMode,
		    BOOL hdlcBitInvert,
		    unsigned blockSizeInWords,
		    UINT32 rawIdleBlockPattern,
		    IxHssAccPktHdlcFraming hdlcTxFraming, 
		    IxHssAccPktHdlcFraming hdlcRxFraming, 
		    unsigned frmFlagStart,
		    IxHssAccPktRxCallback rxCallback,
		    IxHssAccPktUserId rxServiceUserId,
		    IxHssAccPktRxFreeLowCallback rxFreeLowCallback, 
		    IxHssAccPktUserId rxFreeLowUserId,
		    IxHssAccPktTxDoneCallback txDoneCallback,
		    IxHssAccPktUserId txDoneUserId);


/**
 * @fn IX_STATUS ixHssAccPCMPortEnable (IxHssAccHssPort hssPortId, 
                    IxHssAccHdlcPort hdlcPortId)
 * @brief This function is called to enable packet flow on a particular HDLC port, 
 * which is on a particular HSS port
 *
 * @param unsigned hssPortId (in) - This is the HSS port number of the client we 
 * want to enable
 *
 * @param unsigned hdlcPortId (in) - This is the number of the HDLC port that is 
 * to be enabled and it corresponds to the physical E1/T1 channel i.e. 0, 1, 2, 3 
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 * 
 */
IX_STATUS 
ixHssAccPCMPortEnable (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId);


/**
 * @fn IX_STATUS ixHssAccPCMPortDisable  (IxHssAccHssPort hssPortId, 
                                          IxHssAccHdlcPort hdlcPortId);
 *
 * @brief  This function is called to disable packet flow on a particular
 * HDLC port, which is on a particular HSS port
 *
 * @param IxHssAccHssPort hssPortId - This is the HSS port number of the client 
 * we want to disable is on
 * @param IxHssAccHdlcPort hdlcPortId - This is the number of the HDLC port that is 
 * to be disabled and it corresponds to the physical E1/T1 channel 
 * i.e. 0, 1, 2, 3 
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_PARAM_ERR The function did not execute successfully due to a
 *                          parameter error
 */
IX_STATUS 
ixHssAccPCMPortDisable (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId);


/**
 * @fn IX_STATUS ixHssAccPCMDisconnect (IxHssAccHssPort hssPortId, 
                                        IxHssAccHdlcPort hdlcPortId);
 *
 * @brief This function is used to disconnect a client from a particular 
 * HDLC port on a particular HSS port.
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number on which the 
 * client that is to be disconnected is on
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port 
 * that is to be disconnected and it corresponds to the physical E1/T1 
 * channel i.e. 0, 1, 2, 3 
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_DISCONNECTING The function has iniated the disconnecting
 *                             procedure but it has not completed yet.
 * 
 */
IX_STATUS 
ixHssAccPCMDisconnect (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId);


/**
 * @fn IX_STATUS BOOL ixHssAccPCMIsDisconnectComplete (IxHssAccHssPort hssPortId, 
				 IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This function is used to check if a particular HSS/HDLC port combination 
 *        is connected.
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number on which the 
 * client that is to be disconnected is on
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port 
 * that is to be disconnected and it corresponds to the physical E1/T1 
 * channel i.e. 0, 1, 2, 3 
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_DISCONNECTING The function has iniated the disconnecting
 *                             procedure but it has not completed yet.
 * 
 */
BOOL 
ixHssAccPCMIsDisconnectComplete (IxHssAccHssPort hssPortId, 
				 IxHssAccHdlcPort hdlcPortId);


/**
 * @fn void ixHssAccPCMRxCallbackRun (IxHssAccHssPort hssPortId, 
                          IxHssAccHdlcPort hdlcPortId,
			  IX_OSAL_MBUF *buffer, unsigned numHssErrs, 
			  IxHssAccPktStatus pktStatus, UINT32 packetLength)
 *
 * @brief This function is used by the PCM to run the clients Rx Callback 
 * function.
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number on which 
 * the client of whose Rx callback being run is on
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port 
 * of whose callback is to be run and it corresponds to the physical E1/T1 
 * channel i.e. 0, 1, 2, 3 
 * @param IX_OSAL_MBUF *buffer (in) - This is the mbuf of data which has been received 
 * for the client 
 * @param unsigned numHssErrs - This is the num of hss errors that has been 
 * received on this port
 * @param IxHssAccPktStatus pktStatus (in) - The status field returned to 
 * the client indicating the status of the packet.
 * 
 */
void 
ixHssAccPCMRxCallbackRun (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId,
			  IX_OSAL_MBUF *buffer, unsigned numHssErrs, 
			  IxHssAccPktStatus pktStatus, UINT32 packetLength);

/**
 * @fn void ixHssAccPCMTxDoneCallbackRun (IxHssAccHssPort hssPortId, 
                              IxHssAccHdlcPort hdlcPortId, 
			      IX_OSAL_MBUF *buffer, unsigned numHssErrs, 
			      IxHssAccPktStatus pktStatus)
 *
 * @brief This function is used by the PCM to run the clients Tx Done 
 * Callback function.
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number on which 
 * the client of whose Tx Done callback being run is on
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port of 
 * whose Tx Done callback being run is on and it corresponds to the physical
 * E1/T1 channel i.e. 0, 1, 2, 3 
 * @param IX_OSAL_MBUF *buffer (out) - This is the mbuf which the client supplied the
 * HssPktAcc layer with, when it contained data to Tx, it is now being 
 * returned to the client so it can be re-used.
 * @param unsigned numHssErrs (out) - This is the number of Hss errors that 
 * have occured, at the NPE level. 
 *
 */
void 
ixHssAccPCMTxDoneCallbackRun (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId, 
			      IX_OSAL_MBUF *buffer, unsigned numHssErrs, 
			      IxHssAccPktStatus pktStatus);


/**
 * @fn void ixHssAccPCMRxFreeLowCallbackRun (IxHssAccHssPort hssPortId, 
                                             IxHssAccHdlcPort hdlcPortId);
 *
 * @brief This function is used by the PCM to run the clients Rx Free Callback 
 * function. This function is called when there is a low number of entries
 * in the RXFreeQMQ.
 *
 * @param IxHssAccHssPort hssPortId - This is the HSS port number on which the 
 * client of whose FreeLow callback being run is on
 * @param IxHssAccHdlcPort hdlcPortId - This is the number of the HDLC port 
 * of whose FreeLow callback being run is on and it corresponds to the 
 * physical E1/T1 channel i.e. 0, 1, 2, 3 
 *
 */
void 
ixHssAccPCMRxFreeLowCallbackRun (IxHssAccHssPort hssPortId, 
				 IxHssAccHdlcPort hdlcPortId);


/**
 * @fn BOOL ixHssAccPCMCheckTxOk (IxHssAccHssPort hssPortId, 
                                  IxHssAccHdlcPort hdlcPortId);
 *
 * @brief This funtion is called to check if it is ok to transmit for a 
 * particular packetised client.  
 * 
 * @param IxHssAccHssPort hssPortId - This is the HSS port number on which this 
 * client who we want to know if transmission is ok for is running on.
 * @param IxHssAccHdlcPort hdlcPortId - This is the HDLC port number of the client 
 * who we want to know if transmission is ok for is running on 
 *
 * @return  - TRUE It is ok to perform a Tx on this port
 *          - FALSE It is not ok to perform a Tx on this port
 */
BOOL
ixHssAccPCMCheckTxOk (IxHssAccHssPort hssPortId, 
		      IxHssAccHdlcPort hdlcPortId);


/**
 * @fn BOOL ixHssAccPCMCheckReplenishOk (IxHssAccHssPort hssPortId, 
                                              IxHssAccHdlcPort hdlcPortId);
 *
 * @brief This funtion is called to check if it is ok to replenish RxFree 
 * mbufs for a packetised client
 * 
 * @param IxHssAccHssPort hssPortId - This is the HSS port number on which this 
 * client who we want to know if is ok to replenishish on.
 * @param IxHssAccHdlcPort hdlcPortId - This is the HDLC port number of the 
 * client who we want to know if  is ok to repleinsh on 
 *
 * @return     - TRUE It is ok to replenish the RxFree queue for this client
 *             - FALSE It is not ok to replenish the RxFree queue for this client
 */
BOOL
ixHssAccPCMCheckReplenishOk (IxHssAccHssPort hssPortId, 
			     IxHssAccHdlcPort hdlcPortId);


/**
 * @fn void ixHssAccPCMShow (void)
 *
 * @brief This funtion is called show to statistics of the PCM.  
 * 
 * @return void
 */
void 
ixHssAccPCMShow (void);


/**
 * @fn void ixHssAccPCMStatsInit (void);
 *
 * @brief This funtion is called to initialise the statistics of the PCM.  
 * 
 * @return void
 */
void 
ixHssAccPCMStatsInit (void);


/**
 * @fn IxQMgrQId ixHssAccPCMRxFreeQIdGet (IxHssAccHssPort hssPortId, 
                                          IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This inline funtion is called to return an RxFree Q Id for a specified client.
 * 
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number of the 
 * RxFree Q Id we want to return
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the HDLC port number of the 
 * RxFree Q Id we want to return 
 *
 * @return IxQMgrQId -  The QId of the RxFree Q for the client whose HSS and HDLC port Id 
 */
IXHSSACCPCM_INLINE IxQMgrQId
ixHssAccPCMRxFreeQIdGet (IxHssAccHssPort hssPortId,
			 IxHssAccHdlcPort hdlcPortId);

/**
 * @fn IxQMgrQId ixHssAccPCMTxQIdGet (IxHssAccHssPort hssPortId, 
   IxHssAccHdlcPort hdlcPortId)
 *
 * @brief This inline funtion is called to return an Tx Q Id for a specified client.
 * 
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number of the 
 * Tx Q Id we want to return
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the HDLC port number of the 
 * Tx Q Id we want to return 
 *
 * @return IxQMgrQId -  The QId of the Tx Q for the client whose HSS and HDLC port Id 
 */

IXHSSACCPCM_INLINE IxQMgrQId
ixHssAccPCMTxQIdGet (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId);

/**
 * @fn IX_STATUS ixHssAccPCMnoBuffersInUseCountInc(IxHssAccHssPort hssPortId,
                                IxHssAccHdlcPort hdlcPortId,
                                UINT32 count)
 *
 * @brief This function is called to increment the number of buffers in use
 * count for the client (HSS port and HDLC channel combination).
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number of the client 
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the HDLC port number of the client
 * @param UINT32 count (in) - This is the count by which the number of buffers count
 * will be incremented for the client.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 */
IX_STATUS 
ixHssAccPCMnoBuffersInUseCountInc(IxHssAccHssPort hssPortId,
            IxHssAccHdlcPort hdlcPortId,
            UINT32 count);

/**
 * @fn IX_STATUS ixHssAccPCMnoBuffersInUseCountDec(IxHssAccHssPort hssPortId, 
                                IxHssAccHdlcPort hdlcPortId, 
                                UINT32 count)
 *
 * @brief This function is called to decrement the number of buffers in use
 * count for the client (HSS port and HDLC channel combination).
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number of the client 
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the HDLC port number of the client
 * @param UINT32 count (in) - This is the count by which the number of buffers count
 * will be decremented for the client.
 * 
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 */
IX_STATUS 
ixHssAccPCMnoBuffersInUseCountDec(IxHssAccHssPort hssPortId, 
                IxHssAccHdlcPort hdlcPortId, 
                UINT32 count);

/**
 * @fn IX_STATUS ixHssAccPCMInit (void);
 *
 * @brief This function is called to initialise the PCM.
 * 
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 */

IX_STATUS 
ixHssAccPCMInit (void);

/*
 * Inline functions
 */
 
/*
 * This inline funtion is called to return an RxFree Q Id for a specified client.
 */
IXHSSACCPCM_INLINE IxQMgrQId
ixHssAccPCMRxFreeQIdGet (IxHssAccHssPort hssPortId,
	                 IxHssAccHdlcPort hdlcPortId)
{
    return ixHssAccPCMRxFreeQId[hssPortId][hdlcPortId];
}

/*
 * This inline funtion is called to return an Tx Q Id for a specified client.
 */
IXHSSACCPCM_INLINE IxQMgrQId
ixHssAccPCMTxQIdGet (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    return ixHssAccPCMTxQId[hssPortId][hdlcPortId];
}

#endif /* IXHSSACCPCM_P_H */

/**
 * @} defgroup IXHSSACCPCM_p
 */
