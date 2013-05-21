/**
 * @file IxTimeSyncAccCodelet.h
 *
 * @author Intel Corporation
 *
 * @date 23 December 2004
 *
 * @brief This is the header file for Intel (R) IXP400 Software Time Sync Access Codelet
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

#ifndef IXTIMESYNCACCCODELET_H
#define IXTIMESYNCACCCODELET_H

#if defined(__ixp46X)

/**
 * @ingroup Codelets
 *
 * @defgroup IxTimeSyncAccCodelet Intel (R) IXP400 Software Time Sync Access Codelet  
 *
 * @brief Intel (R) IXP400 Software Time Sync Access Codelet 
 *
 * <PRE>
 * This codelet shows how to use some of Time Sync Access (timeSyncAcc) 
 * API functions. It demonstrates the followings: 
 *
 *  	- how to configure Time Sync channel to operate in master or slave mode   
 *	
 *	- how to set frequency scale value (fsv)
 *
 *	- how to set and get system time
 *
 *	- how to set target time
 *	
 *	- how to setup target time in interrupt mode 
 *	
 *	- how to enable and disable target time interrupt
 *
 * Basically, Time Sync Access codelet supports three configurations as
 * follow:
 *
 *	configuration 0: NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)
 *	configuration 1: NPE A - Slave,  NPE B - Master, NPE C - Slave
 *	configuration 2: NPE A - Master, NPE B - Slave,  NPE C - Slave
 * 
 * User provides his/her choice of configuration when executing Time Sync
 * Access codelet. Based on the selected user configuration, each NPE will 
 * be configured to operate in the desired operating mode - master or slave.
 * In addition, all NPEs and ethernet ports will be setup to transmit one 
 * PTP message using UDP protocol every 2 seconds. PTP Sync message will be
 * transmitted at port where NPE is configured to operate in master mode.
 * Conversely, PTP Delay_Req message will be transmitted from slave mode
 * operating port. The transmission and reception of PTP Sync or Delay_Req
 * messages can be demonstrated if the user connects master operating port 
 * to slave operating port. The following table summarizes the activities
 * between the two connecting master port and slave port:
 *
 *	master port			slave port
 *	transmit Sync message		receive Sync message
 *	receive Delay_Req message	transmit Delay_Req message
 
 *  
 * Time Sync Access codelet sets tick rate to 1000 ticks per second. 
 * In other word, the system time will tick every one millisecond.
 * For demonstration purpose, the target time is configured to hit system
 * time every one second. For example, target time is set to one second
 * ahead of current system time. When system time reaches target time, 
 * interrupt will occur. Corresponding ISR will be invoked to be processed.
 * Two main things will be performed during ISR. First, the codelet will
 * check if any NPE channel has detected PTP's Sync or Delay_Req message.
 * If yes, the captured system time at RECV/XMIT snapshot registers will
 * be read and printed along with the PTP message information. Then, the 
 * codelet will set new target time to a second later. This process will
 * be repeated until the user terminates the Time Sync Access codelet
 * execution. 
 * 
 *
 * <B> VxWorks* User Guide </B><BR>
 * (1) <B> ixTimeSyncAccCodeletMain </B><BR>
 * This function is the main function for timeSyncAcc codelet.
 * This function will perform the followings:
 *
 *	- configure all Time Sync Channels to operate in the mode 
 *	  specified in the user selected configuration. 
 *
 *	- set tick rate to get system time to start ticking
 * 
 *	- setup target time to hit every one second (in interrupt mode)
 *
 *	- setup and enable all NPEs and all ethernet components.
 *	
 *	- spawn a thread to transmit Sync message from master port 
 *	  and Delay_Req message from slave port every 2 seconds.
 *
 * <I> Usage :
 *
 *	(a) connect slave port to any master port
 *
 *	(b) run timeSyncAcc codelet:
 *
 *	    -> ixTimeSyncAccCodeletMain x
 *
 *	    where x =
 * 		0 -> NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)  
 *		1 -> NPE A - Slave,  NPE B - Master, NPE C - Slave  
 *		2 -> NPE A - Master, NPE B - Slave,  NPE C - Slave  
 *
 * </I> 
 *
 * (2) <B> ixTimeSyncAccCodeletUninit </B><BR>
 * This function will unload all initialized modules, free all resources, 
 * and nicely terminates timeSyncAcc codelet execution.
 *
 *  <I> Usage :
 * 	-> ixTimeSyncAccCodeletUninit
 *
 *  </I>
 *
 *
 * <B> Linux* User Guide </B><BR>
 * (1) <B> ixTimeSyncAccCodeletMain </B><BR>
 * This function is the main function for timeSyncAcc codelet.
 * This function will perform the followings:
 *
 *	- configure all Time Sync Channels to operate in the mode 
 *	  specified in the user selected configuration. 
 *
 *	- set tick rate to get system time to start ticking
 * 
 *	- setup target time to hit every one second (in interrupt mode)
 *
 *	- setup and enable all NPEs and all ethernet components.
 *	
 *	- spawn a thread to transmit Sync message from master port 
 *	  and Delay_Req message from slave port every 2 seconds.
 *
 * This function will be invoked and executed when the user loads the
 * timeSyncAcc codelet module using 'insmod' command.
 * 
 *  <I> Usage :
 *
 *	(a) connect slave port to any master port
 *
 *	(b) run timeSyncAcc codelet:
 *
 *	    prompt> insmod ixp400_codelets_timeSyncAcc.o config=x
 *
 *	    where x =
 * 		0 -> NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)  
 *		1 -> NPE A - Slave,  NPE B - Master, NPE C - Slave  
 *		2 -> NPE A - Master, NPE B - Slave,  NPE C - Slave  
 *
 *  </I>
 *
 * (2) <B> ixTimeSyncAccCodeletUninit </B><BR>
 * This function will unload all initialized modules, free all resources, 
 * and nicely terminates timeSyncAcc codelet execution.
 * It will be invoked when 'rmmod' command is executed.
 * 
 *  <I> Usage :
 * 	prompt> rmmod ixp400_codelets_timeSyncAcc
 *
 *  </I>
 * </PRE>
 *
 *
 * @{
 */

/*********************************************************************
 *	Include files
 *********************************************************************/

#include "IxOsal.h"
#include "IxTimeSyncAcc.h"
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxFeatureCtrl.h"
#include "IxQMgr.h"
#include "IxNpeDl.h"
#include "IxNpeMh.h"


/*********************************************************************
 *	Macro constants and function definition 
 *********************************************************************/
/**
 * @def IX_TIMESYNCACC_CODELET_ROLLOVER_VALUE
 *
 * @brief rollover value 
 */
#define IX_TIMESYNCACC_CODELET_ROLLOVER_VALUE (0xFFFFFFFF) 

/**
 * @def IX_TIMESYNCACC_CODELET_APB_CLOCK_FREQUENCY
 *
 * @brief APB clock frequency in MHz (66 MHz) 
 */
#define	IX_TIMESYNCACC_CODELET_APB_CLOCK_FREQUENCY IX_OSAL_IXP400_TIME_STAMP_RESOLUTION 

/**
 * @def IX_TIMESYNCACC_CODELET_FSV_DEFAULT
 *
 * @brief define default value for frequency scale value or tick rate.
 *	  With this default tick rate, system time would tick  
 *	  approximately every milli-second.
 */
#define	IX_TIMESYNCACC_CODELET_FSV_DEFAULT (0xFFFFFFFF / (IX_TIMESYNCACC_CODELET_APB_CLOCK_FREQUENCY / 1000))

/**
 * @def IX_TIMESYNCACC_CODELET_TARGET_TIME_HIT_INTERVAL
 *
 * @brief define default value for target time interval. With default 
 * tick rate, default interval is approximately 1 second. 
 */
#define IX_TIMESYNCACC_CODELET_TARGET_TIME_HIT_INTERVAL (1000)

/**
 * @def IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS
 *
 * @brief maximum number of time sync channels 
 */
#define IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS (3)

/**
 * @def IX_TIMESYNCACC_CODELET_MAX_CONFIGURATIONS
 *
 * @brief maximum number of supported configurations 
 */
#define IX_TIMESYNCACC_CODELET_MAX_CONFIGURATIONS (3)

/**
 * @def IX_TIMESYNCACC_CODELET_PTP_MESSAGE_LEN
 *
 * @brief PTP message length in byte 
 */
#define IX_TIMESYNCACC_CODELET_PTP_MESSAGE_LEN (132) 

/**
 * @def IX_TIMESYNCACC_CODELET_UDP_PAYLOAD_LEN
 *
 * @brief UDP payload size in byte (offset: byte 38 and 39) 
 */
#define IX_TIMESYNCACC_CODELET_UDP_PAYLOAD_LEN (IX_TIMESYNCACC_CODELET_PTP_MESSAGE_LEN) 

/**
 * @def IX_TIMESYNCACC_CODELET_UDP_HEADER_LEN
 *
 * @brief UDP header size in byte
 */
#define IX_TIMESYNCACC_CODELET_UDP_HEADER_LEN (32)

/**
 * @def IX_TIMESYNCACC_CODELET_UDP_CHECKSUM_LEN
 *
 * @brief UDP checksum length in byte
 */
#define IX_TIMESYNCACC_CODELET_UDP_CHECKSUM_LEN (2)

/**
 * @def IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN
 *
 * @brief UDP frame size in byte
 */
#define IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN \
	(IX_TIMESYNCACC_CODELET_UDP_PAYLOAD_LEN + IX_TIMESYNCACC_CODELET_UDP_HEADER_LEN + IX_TIMESYNCACC_CODELET_UDP_CHECKSUM_LEN)

/**
 * @def IX_TIMESYNCACC_CODELET_IP_DATAGRAM
 *
 * @brief frame type field of UDP header (offset: byte 12 and 13) for PTP message. 
 *	  It is set to IP_DATAGRAM.
 */
#define IX_TIMESYNCACC_CODELET_IP_DATAGRAM (0x0800)

/**
 * @def IX_TIMESYNCACC_CODELET_IP_HEADER_LEN
 *
 * @brief IP_HEADER_LEN field of UDP header (offset: byte 14) for PTP message  
 */
#define IX_TIMESYNCACC_CODELET_IP_HEADER_LEN (69)

/**
 * @def IX_TIMESYNCACC_CODELET_IP_DATAGRAM_LEN
 *
 * @brief IP_DATAGRAM_LEN field of UDP header (offset: byte 16 and 17) for PTP message  
 */
#define IX_TIMESYNCACC_CODELET_IP_DATAGRAM_LEN (152)

/**
 * @def IX_TIMESYNCACC_CODELET_TIME_TO_LIVE
 *
 * @brief TIME_TO_LIVE field of UDP header (offset: byte 22) for PTP message  
 */
#define IX_TIMESYNCACC_CODELET_TIME_TO_LIVE (1) 

/**
 * @def IX_TIMESYNCACC_CODELET_UDP_PROTOCOL
 *
 * @brief UDP PROTOCOL field of UDP header (offset: byte 23) for PTP message  
 */
#define IX_TIMESYNCACC_CODELET_UDP_PROTOCOL (17)

/**
 * @def IX_TIMESYNCACC_CODELET_PTP_EVENT_PORT
 *
 * @brief Destination port number field of UDP header (offset: byte 36 and 37) for PTP message.
 *	  Event port (319) is used to communicates Sync and Delay_Req messages.
 */
#define IX_TIMESYNCACC_CODELET_PTP_EVENT_PORT (319)

/**
 * @def IX_TIMESYNCACC_CODELET_PTP_MESSAGE_TYPE
 *
 * @brief PTP_MESSAGE_TYPE field for PTP message (offset: byte 62)   
 */
#define IX_TIMESYNCACC_CODELET_PTP_MESSAGE_TYPE (1)

/**
 * @def IX_TIMESYNCACC_CODELET_INVALID_PARAM
 *
 * @brief value for invalid parameter
 */
#define IX_TIMESYNCACC_CODELET_INVALID_PARAM (0xFFFFFFFF)

/**
 * @def IX_TIMESYNCACC_CODELET_PTP_MSG_XMIT_INTERVAL
 *
 * @brief time interval for PTP message transmission (2 seconds)  
 */
#define IX_TIMESYNCACC_CODELET_PTP_MSG_XMIT_INTERVAL (2000)

/**
 * @def IX_TIMESYNCACC_CODELET_LSB_VALUE(x)
 *
 * @brief Get LSB value of x   
 */
#define IX_TIMESYNCACC_CODELET_LSB_VALUE(x) ((x) & 0x000000FF)

/**
 * @def IX_TIMESYNCACC_CODELET_MSB_VALUE(x)
 *
 * @brief Get MSB value of x (where x is a USHORT type)  
 */
#define IX_TIMESYNCACC_CODELET_MSB_VALUE(x) ((x) >> 8)


/*********************************************************************
 *	typedef definitions
 *********************************************************************/

/**
 * @typedef IxTimeSyncAccCodeletUninitFuncPtr
 *
 * @brief Definition of void function pointer with one input parameter.
 *	  The data type of input parameter is IxNpeDlNpeId. 
 */
typedef void (*IxTimeSyncAccCodeletUninitFuncPtr) (IxNpeDlNpeId);

/**
 * @struct IxTimeSyncAccCodeletUninitFuncMap
 *
 * @brief This struct is used to store each supporting module's
 *	  unload function's pointer, function parameter, and the
 *	  state whether the module is initialized.  
 */
typedef struct
{
	IxOsalVoidFnPtr funcPtr;	/*!< function pointer */ 	
	IxNpeDlNpeId funcParameter;	/*!< function's input parameter */
	BOOL initialized;		/*!< initialization state: TRUE - initialized,
								   FALSE - not initialized */	
} IxTimeSyncAccCodeletUninitFuncMap;

/**
 * @struct IxTimeSyncAccCodeletTSChannelConfig
 *
 * @brief This struct is used to store all three Time Sync channels'
 *	  operating mode: master or slave. 
 */
typedef struct  
{
	IxTimeSyncAcc1588PTPPortMode tsChannelMode[IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS]; /*!< channel operating mode */
} IxTimeSyncAccCodeletTSChannelConfig;


/*********************************************************************
 *	enum definition 
 *********************************************************************/
/**
 * @enum IxTimeSyncAccCodeletModuleId
 *
 * @brief Module ID list. These modules are required to be initialized 
 *	  or setup for PTP message transmission from each NPE.
 */
typedef enum  
{
	IX_TIMESYNCACC_CODELET_MBUF_ALLOC, 	/*!< mBuf Memory Allocation */
	IX_TIMESYNCACC_CODELET_Q_MGR,		/*!< Q Mgr */ 
	IX_TIMESYNCACC_CODELET_DISPATCHER, 	/*!< Q Mgr Dispatcher */
	IX_TIMESYNCACC_CODELET_NPE_MH,		/*!< NPE Message Handler */
	IX_TIMESYNCACC_CODELET_NPE_DL,		/*!< NPE Downloader */
	IX_TIMESYNCACC_CODELET_NPE_A,		/*!< NPE A */
	IX_TIMESYNCACC_CODELET_NPE_B,		/*!< NPE B */
	IX_TIMESYNCACC_CODELET_NPE_C,		/*!< NPE C */
	IX_TIMESYNCACC_CODELET_ETH_ACC,		/*!< Ethernet Access Component */
	IX_TIMESYNCACC_CODELET_ETH_PORTS,	/*!< Ethernet Port */
	IX_TIMESYNCACC_CODELET_TX_PTP		/*!< PTP Message Transmission Setup */ 
} IxTimeSyncAccCodeletModuleId;


/*********************************************************************
 *	Function Prototype 
 *********************************************************************/
/**
 * @fn ixTimeSyncAccCodeletMain (UINT32 configIndex)
 *
 * @brief  ixTimeSyncAccCodeletMain is the main function for 
 *          timeSyncAcc codelet. 
 *
 * This function will perform the followings:
 *
 *	- configure all Time Sync Channels to operate in the mode 
 *	  specified in the user selected configuration. 
 *
 *	- set tick rate to get system time to start ticking
 * 
 *	- setup target time to hit every one second (in interrupt mode)
 *
 *	- setup and enable all NPEs and all ethernet components.
 *	
 *	- spawn a thread to transmit Sync message from master port 
 *	  and Delay_Req message from slave port every 2 seconds.
 *
 * @param
 * configIndex UINT32 [in] - choice of configuration 
 *   - 0 -> NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)  
 *   - 1 -> NPE A - Slave,  NPE B - Master, NPE C - Slave  
 *   - 2 -> NPE A - Master, NPE B - Slave,  NPE C - Slave  
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - start codelet successfully
 *          @li IX_FAIL    - fail
 */
PUBLIC IX_STATUS ixTimeSyncAccCodeletMain (UINT32 configIndex);


/**
 * @fn ixTimeSyncAccCodeletUninit ()
 *
 * @brief  This function will unload all initialized modules, free all 
 *	   resources, and nicely terminates timeSyncAcc codelet execution.
 *
 * @return void
 */
PUBLIC void ixTimeSyncAccCodeletUninit (void);

/** @} */

#endif /* end of #ifdef __ixp46X */

#endif /* end of IXTIMESYNCACCCODELET_H */

