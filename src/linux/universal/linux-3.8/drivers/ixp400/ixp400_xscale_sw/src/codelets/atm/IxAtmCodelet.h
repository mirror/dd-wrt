/**
 * @file    IxAtmCodelet.h
 *
 * @brief   IxAtmCodelet scenarios header file.
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
#ifndef IX_ATMCODELET_H
#define IX_ATMCODELET_H

/**
 * @defgroup Codelets Intel (R) IXP400 Software Codelets
 *
 * @brief Intel (R) IXP400 Software Codelets
 *
 * @{
 */

/**
 * @ingroup Codelets
 *
 * @defgroup IxAtmCodelet Intel (R) IXP400 ATM Codelet (IxAtmCodelet) API
 *
 * This codelet demonstrates an example implementation of a working Atm
 * driver that makes use of the AtmdAcc component, as well as demonstrating how
 * the lower layer IxAtmdAcc component can be used for configuration and 
 * control. 
 * This codelet also demonstrates an example implementation of OAM F4 
 * Segment, F4 End-To-End (ETE), F5 Segment and F5 ETE loopback that makes 
 * use of the AtmdAcc component, as well as demonstrating how the lower layer 
 * IxAtmdAcc component can be used for configuration and control.
 *
 * <b> Disclaimer Note: </b>
 * For Linux* Platform
 * - When 'insmod' the ATM codelet object, it will begin to send AAL packets 
 *   and display the transmit and receive statistics every 15 second
 * - Unable to 'rmmod'. User will not be able to type anything in the command
 *   line due to: a) the return carriage indicating that the task is sending
 *   AAL packets, and b) the failure to kill the thread which is used to 
 *   transmit AAL packets
 *
 * <b> VxWorks* User Guide </b><br>
 * ixAtmCodeletMain() function is used as a single point of execution for 
 * Atm Codelet, which allows the user to enter selections for different type 
 * of modes and AAL type. The function also allows the user to execute OAM 
 * ping either in UTOPIA or Software Loopback mode. In all modes, the transmit
 *  and receive statistics will be displayed every 15secs.
 *
 * <pre>
 *  <i> Usage :
 *      ixAtmCodeletMain (modeType, aalType)
 *	  modeType:
 *                  0 = Utopia Loopback Mode
 *                  1 = Software Loopback Mode
 *                  2 = Remote Loopback Mode
 *                  3 = F4 & F5 cells OAM Ping in UTOPIA Loopback mode
 *                  4 = F4 & F5 cells OAM Ping in Software Loopback mode
 *	  aalType:
 *                  1 = AAL5
 *                  2 = AAL0_48
 *                  3 = AAL0_52
 * </i>
 * </pre>
 *
 * <b> Linux* User Guide </b><br>
 * The idea of using the ixAtmCodeletMain() as a single point of execution for
 * ATM codelet is similar in VxWorks* User Guide. It also allows user to
 * execute OAM ping. Similarly, all modes will display the transmit and 
 * receive statistics every 15secs. This function will be executed when user 
 * issue'insmod' in command prompt
 *
 * <pre>
 * <i>  Usage :
 *      # insmod ixp400_codelets_atm.o modeType=<x> aalType=<y>
 *      Where x:
 *                  0 = Utopia Loopback Mode
 *                  1 = Software Loopback Mode
 *                  2 = Remote Loopback Mode
 *                  3 = F4 & F5 cells OAM Ping in UTOPIA Loopback mode
 *                  4 = F4 & F5 cells OAM Ping in Software Loopback mode
 *	Where y:
 *                  1 = AAL5
 *                  2 = AAL0_48
 *                  3 = AAL0_52
 * </i>
 * </pre>
 *
 * Note for VxWorks* and UNIX* Usage:
 * @li IX_ATM_CODELET_SWLOOPBACK_PORT_RATE and IX_ATM_CODELET_REMOTELOOPBACK_PORT_RATE
 *    defined in this header file allows the user to change the port rate
 *    (in cells/sec) accordingly. The port rate works when using ADSL 
 *    connection. The default port rate for both loopback is set to 
 *    1962cells/sec (~832kbps) <br>
 * @li IX_ATMCODELET_START_VPI and IX_ATMCODELET_START_VCI defined in this
 *    header file can be modified by the user. By default, VPI and VCI are set
 *    to 1 and 32 respectively <br>
 * @li IX_ATMCODELET_NUM_AAL5_CHANNELS, IX_ATMCODELET_NUM_AAL0_48_CHANNELS, and
 *    IX_ATMCODELET_NUM_AAL0_52_CHANNELS can be changed by the user. By 
 *    default, they are set to 32 channels
 * @li OAM Ping in UTOPIA Loopback mode will perform the following sequence in 
 *    forever loop: i) Send AAL packets, ii) Display the transmit and receive 
 *    statistics, and iii) Perform OAM Ping F4 and F5 (ETE and Segment) and 
 *    display OAM statistics <br>
 * @li OAM Ping in Software Loopback will perform the following sequence in 
 *    forever loop: i) Display the transmit and receive statistics, and
 *    ii) OAM Ping F4 and F5 (ETE and Segement) and display OAM Statistics
 *
 *
 * <b>ATM Features</b>
 * @li An interface is provided to setup Aal5 or Aal0 (48 or 52 bytes) 
 *     Transmit and Recieve VCs on one port. Only UBR VCs can be setup. When 
 *     a channel is setup using this interface both a Transmit and a Recieve 
 *     VC is created.
 * 
 * @li An interface is provided to remove all registered Aal5/Aal0 VCs.
 *
 * @li Both remote and local loopback of Aal5 or Aal0 is provided by this codelet.
 *     Local loopback refers to loopback provided by the UTOPIA interface. In 
 *     local loopback packets generated by the IXP400 are looped back to the 
 *     Atm driver by the UTOPIA hardware.
 *     Software loopback refers to the software looping all packets received 
 *     on the wire back out onto the wire.
 *     Remote loopback refers to where the far end is expected to performa a 
 *     software loopback, i.e. any packets sent by the codelet are expected to 
 *     be looped back by the far end into the codelet.
 *      
 * @li Both interrupt and polled mode of operation is provided by this codelet.
 *
 * @li An interface is provided to send Aal5 SDUs specifying the sdu size and 
 *     the number of Aal5 sdus to send.
 *
 * @li An interface is provided to send Aal0 packets specifying the packet size
 *      and the number of Aal0 packets to send.
 *
 * @li Both the Transmit port rate and the Recieve port rate can be modified. 
 *     The Tx port rate is used by IxAtmSch in performing the shaping functions
 *     The Rx port rate is not used by any component.
 *
 * @li An interface is provided to allow the querying of the ATM ports and 
 *     registered VCs.
 *
 * <b>IXP400 ATM Components used by this codelet</b>
 * @li IxAtmdAcc. This component is the low level interface by which AAL 
 *     packets get transmitted to, and received from the UTOPIA bus.
 *
 * <b>IXP400 ATM Codelet components used by this codelet</b>
 * @li IxAtmSch. This component demonstrates an ATM Traffic Shaper 
 *     implementation. IxAtmdAcc gets scheduling information from this 
 *     component.
 * 
 * @li IxAtmm. This component provides ATM port and VC management facilities. 
 *     This component also manages the configuration of the UTOPIA 
 *     co-processor
 *
 * <b>IxAtmCodelet modes of operation</b><br>
 * This codelet can be initialised to operate in one of three configurations.
 * 
 * @li IX_ATMCODELET_UTOPIA_LOOPBACK. In this mode the UTOPIA interface will 
 *     loopback all traffic transmitted.<br>
 *     <br>
 *     <b>Buffer management</b>
 *     In this mode a simple buffering mechanism is used; mbufs are allocated 
 *     from the VxWorks* pool as needed for RxFree replenishing/Tx and are 
 *     returned to the VxWorks* pool for TxDone/Rx <br>
 *     <br>
 *     <b>Interrupt/Task based processing </b>
 *     In this mode of operation the IxQMgrDispatcher is hooked to interupts. 
 *     This means that TxDone/RxLo will be perfomed from a task level and 
 *     IxAtmdAcc Tx processing/RxHi and RxFree will be precessed from an 
 *     interrupt level. <br>
 *     <br>
 *     <b>Sending Aal5 PDUs</b>
 *     In this mode of operation an interface is available to send Aal5 PDUs 
 *     of specified size in bytes. <br>
 *     <br>
 *     <b>Sending Aal0 Packets</b>
 *     In this mode of operation an interface is available to send Aal0 
 *     packets of specified size in cells. <br>
 *
 * @li IX_ATMCODELET_REMOTE_LOOPBACK.
 *     In this mode packets are sent and are expected to be looped back on the
 *      remote end.
 *
 * @li IX_ATMCODELET_SOFTWARE_LOOPBACK. In this mode of operation all traffic 
 *     sent are looped back to receive in the IXP400 device.<br>
 *     <br>
 *     <b>Buffer management</b>
 *     In this mode a more complex buffering mechanism is used; mbufs are 
 *     allocated from the VxWorks* pool and stored in a software queue. These
 *     mbufs are fetched from this software queue as needed for RxFree 
 *     replenishing/Tx and are returned to the software queue for 
 *     TxDone/Rx.<br>
 *     <br>
 *     <b>Interrupt/Task based processing</b>
 *     In this mode of operation the IxQMgrDispatcher is called form a task 
 *     every 1 msec. This means that TxDone/RxLo/IxAtmdAcc Tx processing/RxHi 
 *     and RxFree will be processed from a task level.<br>
 *     <br>
 *     <b>Sending PDUs/Packets</b>
 *     This mode of operation does not provide an interface for sending Aal5 
 *     PDUs/Aal0 Packets. <br>
 * 
 * <b>IxAtmCodelet sub-components</b>
 * <pre>
 *                           +--------------------+
 *                           |     AtmCodelet     |
 *                           +--------------------+
 *                               |          |
 *                         ,-----'          `-----,
 *                         |                      |
 *                         V                      V
 *                    +------------+        +------------+
 *                    |  RxTx      |        | SwLoopback |
 *                    +------------+        +------------+
 *
 * </pre>
 *
 * <b>AtmCodelet</b>
 * This implements the API functions.
 *
 * <b>RxTx</b>
 * This sub-component implements the IX_ATMCODELET_UTOPIA_LOOPBACK and 
 * IX_ATMCODELET_REMOTE_LOOPBACK modes of operation.
 *
 * <b>SwLoopback</b>
 * This sub-component implements the IX_ATMCODELET_SOFTWARE_LOOPBACK mode 
 * operation.
 *
 * <b>BufMan</b>
 * This sub-component implements the intefaces used internally for getting and
 * returning VxWorks* mbufs.
 *
 * <b>UTOPIA Recieve and Transmit PHY addresses</b>
 * In this codelet UTOPIA Phy Addresses are assigned numbers starting at
 * UTOPIA_PHY_ADDR and are incremented for each port used. These addresses 
 * _WILL_ need to be changed depending on the hardware setup.
 * 
 * <b>OAM Features</b>
 * @li An interface is provided to configure the access layer to enable OAM 
 *     traffic.
 * 
 * @li OAM Loopback responses are sent automatically on receipt of OAM F4 and 
 *     F5 parent Segment and ETE loopback cells, i.e. externally initiated OAM 
 *     Loopbacks.
 *
 * @li Interfaces are provided to initiate an OAM F4/F5 Segment or ETE 
 *     Loopback, see ITU-610. One loopback can be initiated at a time, this is
 *     to keep the codelet simple, this is independant of sending responses to 
 *     parent loopback cells received.
 *
 * @li An interface is provided to query OAM traffic statistics.
 *
 * <b>OAM uses the following IXP400 ATM Components </b>
 * @li IxAtmdAcc. This component is the low level interface by which OAM cells 
 *     get transmitted to, and received from the UTOPIA bus.
 *
 * @li IxAtmm. This component provides ATM port and VC management facilities. 
 *     This component also contains the configuration of the UTOPIA 
 *     co-processor.
 *
 * @li AtmUtils. This codelet provides VC setup facilities using IxAtmdAcc. 
 *
 * <B>IMPORTANT!!! </B><br>
 * If validation board is used (instead of using IXDP400 development board), 
 * then uncomment VALIDATION_PLATFORM_USED flag in component.mk
 *
 * @{
 */

/*
 * User defined include files.
 */
#include "IxOsal.h"
#include "IxAtmCodelet_p.h"

/*
 * Defines and macros
 */

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATM_CODELET_SWLOOPBACK_PORT_RATE
 *
 * @brief Port rate for Software Loopback
 *
 * By default the port rate for software loopback is set at 
 * PCR = 1962 cells/sec (832kbps)
 *
 */
#define IX_ATM_CODELET_SWLOOPBACK_PORT_RATE 1962

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATM_CODELET_REMOTELOOPBACK_PORT_RATE
 *
 * @brief Port rate for Remote Looopback
 *
 * By default the port rate for remote loopback is set at 
 * PCR = 1962 cells/sec (832kbps)
 *
 */
#define IX_ATM_CODELET_REMOTELOOPBACK_PORT_RATE 1962

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATMCODELET_START_VPI
 *
 * @brief The first VPI value.
 *
 * By default the VPI is set to 1
 */
#define IX_ATMCODELET_START_VPI               (1)     

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATMCODELET_START_VCI
 *
 * @brief The first VCI value.
 *
 * By default the VCI is set to 2
 */
#define IX_ATMCODELET_START_VCI               (32)

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATMCODELET_NUM_AAL5_CHANNELS
 *
 * @brief 32 Channels for AAL5
 */
#define IX_ATMCODELET_NUM_AAL5_CHANNELS         32

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATMCODELET_NUM_AAL0_48_CHANNELS
 *
 * @brief 32 Channels for AAL0 48-bytes
 */
#define IX_ATMCODELET_NUM_AAL0_48_CHANNELS      32 

/**
 * @ingroup IxAtmCodelet
 *
 * @def IX_ATMCODELET_NUM_AAL0_52_CHANNELS
 *
 * @brief 32 Channels for AAL0 52-bytes
 */
#define IX_ATMCODELET_NUM_AAL0_52_CHANNELS      32

/**
 * @ingroup IxAtmCodelet
 * 
 * @def   IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY
 *
 * @brief Recommended priority of queue manager dispatch loop 
 *
 */
#define IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY    240 

/**
 * @ingroup IxAtmCodelet
 * 
 * @def   IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE
 *
 * @brief Recommended stack size for queue manager dispatcher thread 
 *
 */
#define IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE 10240

/**
 * @ingroup IxAtmCodelet
 *
 * @def   IX_ATMCODELET_UTOPIA_LB_THREAD_PRIORITY
 *
 * @brief Recommended priority of UTOPIA loopback thread (for 
 * AAL0 and AAL5 type cells) 
 *
 */
#define IX_ATMCODELET_UTOPIA_LB_THREAD_PRIORITY 10

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */

/*
 * Function definitions
 */

/** 
 * @ingroup IxAtmCodelet
 *
 * @fn ixAtmCodeletMain(IxAtmCodeletMode modeType, IxAtmCodeletAalType aalType)
 *
 * @brief This function is used as a single point of execution for ATM codelet
 *
 * @param "IxAtmCodeletMode modeType" The type of mode use, either Utopia, 
 *        Software or Remote loopback mode. It also consists of OAM Ping in 
 *        Utopia or Software loopback mode.
 * @param "IxAtmCodeletAalType aalType" The type of AAL: AAL5 or AAL0 with 48-
 *        or 52-bytes cell
 *
 * @return
 * IX_SUCCESS Mode and AAL type successfully setup
 * IX_FAIL Invalid AAL or mode type, or error in setting up the modes
 */
PUBLIC IX_STATUS
ixAtmCodeletMain(IxAtmCodeletMode modeType, IxAtmCodeletAalType aalType);

#endif /*IX_ATMCODELET_H*/

/** @} defgroup IxAtmCodelet*/

/** @} defgroup Codelets*/
