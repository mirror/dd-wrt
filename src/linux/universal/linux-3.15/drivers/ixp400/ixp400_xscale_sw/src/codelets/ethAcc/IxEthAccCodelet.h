 /**
 * @file IxEthAccCodelet.h
 *
 * @date 22 April 2002
 *
 * @brief This file contains the interface for the Ethernet Access Codelet.
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
#ifndef IXETHACCCODELET_H
#define IXETHACCCODELET_H

/**
 * @defgroup Codelets Intel (R) IXP400 Software Codelets
 *
 * @brief Intel (R) IXP400 Software EthAcc Codelet
 *
 * @{
 */

/**
 * @ingroup Codelets
 *
 * @defgroup IxEthAccCodelet Intel (R) IXP400 Software Ethernet Access Codelet (IxEthAccCodelet) API
 *
 * @brief Intel (R) IXP400 Software Ethernet Access Codelet API
 *
 * This codelet demonstrates both Ethernet Data and Control plane services and 
 * Ethernet Management services. 
 * <UL>
 *   <LI> A) Ethernet Data and Control plane services: 
 *   <UL>
 *        <LI> Configuring both ports as a receiver sink from an external source (such 
 *            as Smartbits).
 *        <LI> Configuring Port-1 to automatically transmit frames and receive frames on Port-2. 
 *             Frames generated and transmitted in Port-1 are loopbacked into Port-2 by using cross cable.
 *        <LI> Configuring and performing a software loopback on each of the two ethernet ports.
 *	  <LI> Configuring both ports to act as a bridge so that frames received on one port are retransmitted on the other.
 *   </UL>
 * </UL>
 *
 *   <UL>
 *   <LI> B) Ethernet Management services: 
 *      <UL>
 *	<LI> Adding and removing static/dynamic entries.
 *	<LI> Calling the maintenance interface (shall be run as a separate background task)
 *      <LI> Calling the show routine to display the MAC address filtering tables.    
 *      </UL>
 *   </UL>
 *
 * <b> Definition </b><br>
 * In the context of this codelet, the following definitions are applicable.<br>
 * Port 1 = ixe0 = Ethernet port associated with NPE-B Ethernet Coprocessor.<br> 
 * Port 2 = ixe1 = Ethernet port associated with NPE-C Ethernet Coprocessor.<br>    
 * Port 3 = ixe2 = Ethernet port associated with NPE-A Ethernet Coprocessor.<br>    
 *
 * <b> Design constraints </b><br>
 * This codelet assumes that the underlying Intel(R) IXP4XX Product Line of Network Processors
 * have two Ethernet NPEs. For silicon with single Ethernet NPE, operation will be only 
 * functional in the particular Ethernet port that corresponds to the available Ethernet NPE.
 * Particularly, bridge operation will not work as two Ethernet ports are needed in this operation. 
 *
 * <b> Assumptions </b><br>
 * This codelet illustrates the use EthAcc APIs. The operations provided may not be
 * working on the best performance as the target of this codelet is just to show the 
 * functionality of APIs. In order to get better performance, #undef
 * IX_ETHACC_CODELET_TXGENRXSINK_VERIFY to disable traffic verification. <br>
 *
 * Please note that this codelet is not optimized for production quality.
 *   
 * For performance testing, please use the operations below:
 * <UL>
 * <LI> Rx Sink Operation.
 * <LI> TxGenRxSink Operation.
 * <LI> Bridge Operation with Ethernet frames sent into either one of the Ethernet Ports.
 * </UL>
 * 
 * The operations below need special tuning to optimize them. Tuning can be done by either 
 * using a lower traffic(frames/second), reducing the value of  IX_ETHACC_CODELET_TXGEN_PCKS
 * or  #undef IX_ETHACC_CODELET_TXGENRXSINK_VERIFY.
 * <UL>  
 *  <LI> Software Loopback Operation.
 *  <LI> PHY Loopback Operation.
 *  <LI> Bridge Operation with Ethernet frames sent into both Ethernet Ports.
 * </UL>
 *
 * <b> VxWorks* User Guide </b><br>
 * ixEthAccCodeletMain() function is used as a single point of execution for 
 * EthAcc Codelet. It allows user to enter selection for different type 
 * of supported operations described below: 
 *
 * <pre>
 *  <i> Usage :
 *      >ixEthAccCodeletMain (operationType,inPort,outPort,disableStats)
 *      Where operationType:
 *           1 = To sink received frames as fast as possible for available ports.
 *           2 = To software loopback received frames to the same port for available ports.
 *           3 = To generate and transmit frames from outPort, remote loopback by using 
 *               an external cross cable to inPort, and received on inPort (TxGenRxSink).
 *           4 = To generate frames and perform PHY loopback on the same port for available ports. 
 *           5 = To transmit any frame received on inPort through outPort (Bridge).
 *           6 = To transmit any 802.1Q-tagged frame received on inPort through outPort,
 *               using QoS (QoS Bridge)
 *           7 = To transmit frames received on inPort through outPort, provided
 *               they meet the MAC address firewall criteria (Firewall Bridge)
 *           8 = To activate Ethernet MAC learning facility.
 *           9 = To transmit frames received on inPort through outPort, using
 *               802.3 <=> 802.11 frame header conversions for frames matching certain
 *               MAC addresses
 *
 *      Where inPort and outPort are Ethernet portId:
 *           0 = NPE-B
 *           1 = NPE-C
 *           2 = NPE-A
 *
 *	Where disableStats:
 *	     0 = Enable statistic polling task thread from running.
 *	     1 = disable statistic polling task thread.
 * </i>
 * </pre>
 *
 * <b> Linux* User Guide </b><br>
 * The idea of using the ixEthAccCodeletMain() as a single point of execution for
 * EthAcc codelet. The operation selected will be executed when user issue 'insmod' 
 * in command prompt.
 *
 * <pre>
 * <i>  Usage :
 *      >insmod ixp400_codelets_ethAcc.o operationType=<x> inPort=<y> outPort=<z> disableStats=<i>
 *      Where x:
 *           1 = To sink received frames as fast as possible for available ports.
 *           2 = To software loopback received frames to the same port for available ports.
 *           3 = To generate and transmit frames from outPort (z), remote loopback by using 
 *               an external cross cable to inPort (y), and received on inPort (y) (TxGenRxSink).
 *           4 = To generate frames and perform PHY loopback on the same port for available ports. 
 *           5 = To transmit any frame received on inPort (y) through outPort (z) (Bridge).
 *           6 = To transmit any 802.1Q-tagged frame received on inPort (y) through outPort (z),
 *               using QoS (QoS Bridge)
 *           7 = To transmit frames received on inPort (y) through outPort (z), provided
 *               they meet the MAC address firewall criteria (Firewall Bridge)
 *           8 = To activate Ethernet MAC learning facility.
 *           9 = To transmit frames received on inPort (y) through outPort (z), using
 *               802.3 <=> 802.11 frame header conversions for frames matching certain
 *               MAC addresses
 *
 *      Where inPort and outPort are Ethernet portId:
 *           0 = NPE-B
 *           1 = NPE-C
 *           2 = NPE-A
 *               
 *	Where i:
 *	     0 = Enable statistic polling task thread from running.
 *	     1 = Disable statistic polling task thread. 
 * </i>
 * </pre>
 *
 * <b> WinCE* User Guide </b><br>
 * The Ethernet Access Codelet uses serial console to print out menus and accept input from
 * users. Users need to choose and enter which operation to be executed from the menus.
 *
 * <pre>
 * <i>  Usage :
 *      Menu: Choose type of test you want to execute.
 *      Options:
 *           1 = To sink received frames as fast as possible for available ports.
 *           2 = To software loopback received frames to the same port for available ports.
 *           3 = To generate and transmit frames from port 1, remote loopback by using 
 *               an external cross cable to port 2, and received on port 2 (TxGenRxSink).
 *           4 = To generate frames and perform PHY loopback on the same port for available ports. 
 *           5 = To transmit any frame received on one port through the other one (Bridge).
 *           6 = To transmit any 802.1Q-tagged frame received on one port through the other
 *               one, using QoS (QoS Bridge)
 *           7 = To transmit frames received on one port through the other port, provided
 *               they meet the MAC address firewall criteria (Firewall Bridge)
 *           8 = To activate Ethernet MAC learning facility.
 *           9 = To transmit frames received on one port through the other port, using
 *               802.3 <=> 802.11 frame header conversions for frames matching certain
 *               MAC addresses
 *         100 = Exit Ethernet Access codelet.
 *        
 * </i>
 * </pre>
 *
 * <P>
 * <B>MAC Setup</B>
 * <P>
 * The default MAC setup will be:
 * <UL>
 *	<LI>Promiscuous mode enabled (for learning example)
 *	<LI>Frame Check Sequence appended for all frames generated on the Intel XScale(R) processor
 * </UL>
 *
 * <P>
 * <B>PHY Setup</B>
 * <P>
 * This codelet uses two PHYs as defined by IX_ETHACC_CODELET_MAX_PHY 
 * The default PHY setup will be: 
 * <UL>
 *	<LI>100Mbits,
 *	<LI>full duplex,
 *	<LI>auto-negotiation on.
 * </UL>
 *
 * <P>
 * <B>Jumbo frames</B>
 * <P>
 * This codelet setup enable Jumbo frame reception
 * The default setup will be: 
 * <UL>
 *	<LI>frames up to a msdu size of 9018 are supported.
 * </UL>
 *
 * <P>
 * <B>Test Equipment</B>
 * <P>
 * The test harness will consist of external test equipment capable of
 * generating Ethernet packets (e.g. SmartBits).
 * <P>
 * The test equipment must be capable of performing at least the following
 * actions to support the scenarios outlined for the Codelet:
 * <UL>
 *   <LI> Send/receive an Ethernet data stream.
 *   <LI> Send/receive frames of different length.
 *   <LI> Detect CRC errors.
 *   <LI> Append FCS.
 *   <LI> Support 100Mbit full duplex mode.
 * </UL>
 *
 * @{
 */

#include "IxOsal.h"
#include "IxEthAcc.h"

/*
 * #defines for function return types, etc.
 */

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_NPEA_MAC
 *
 * @brief Hard-encoded MAC address for NPEA.  
 */
#define IX_ETHACC_CODELET_NPEA_MAC {{0x2, 0x0, 0x6, 0x7, 0x8, 0x9}}

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_NPEB_MAC
 *
 * @brief Hard-encoded MAC address for NPEB.  
 */
#define IX_ETHACC_CODELET_NPEB_MAC {{0x2, 0x0, 0xa, 0xb, 0xc, 0xd}}

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_NPEC_MAC
 *
 * @brief Hard-encoded MAC address for NPEC.
 */
#define IX_ETHACC_CODELET_NPEC_MAC {{0x2, 0x0, 0xe, 0xf, 0xa, 0xb}}

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_RX_MBUF_POOL_SIZE
 *
 * @brief Size of receive MBuf pool.  
 */
#define IX_ETHACC_CODELET_RX_MBUF_POOL_SIZE   (512*IX_ETHACC_CODELET_MAX_PORT)

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_TX_MBUF_POOL_SIZE
 *
 * @brief Size of transmit MBuf pool.  
 */
#define IX_ETHACC_CODELET_TX_MBUF_POOL_SIZE   128

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_MAX_PORT
 *
 * @brief Number of Ethernet Ports supported for this codelet.
 */
#define IX_ETHACC_CODELET_MAX_PORT IX_ETHNPE_MAX_NUMBER_OF_PORTS

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_MBUF_POOL_SIZE
 *
 * @brief Size of MBuf pool.
 */
#define IX_ETHACC_CODELET_MBUF_POOL_SIZE \
  (((IX_ETHACC_CODELET_RX_MBUF_POOL_SIZE/IX_ETHACC_CODELET_MAX_PORT) \
   + IX_ETHACC_CODELET_TX_MBUF_POOL_SIZE) \
  * IX_ETHACC_CODELET_MAX_PORT)

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_PCK_SIZE
 *
 * @brief Size of MBuf packet (recommended size for ethAcc component)
 */
#define IX_ETHACC_CODELET_PCK_SIZE IX_ETHACC_RX_MBUF_MIN_SIZE

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_PCK_LEN
 *
 * @brief Length of MBuf payload (in bytes).
 */
#define IX_ETHACC_CODELET_PCK_LEN 1536

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_MBUF_DATA_POOL_SIZE
 *
 * @brief Size of MBuf data pool.
 */
#define IX_ETHACC_CODELET_MBUF_DATA_POOL_SIZE \
  (IX_ETHACC_CODELET_MBUF_POOL_SIZE * IX_ETHACC_CODELET_PCK_SIZE)

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_TXGEN_PCK_LEN
 *
 * @brief Size of packets for TxGenRxSink Operation.
 */
#define IX_ETHACC_CODELET_TXGEN_PCK_LEN 60

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_TXGEN_PCKS 
 *
 * @brief Number of packets to generate for the TxGenRxSink Operation.
 */
#define IX_ETHACC_CODELET_TXGEN_PCKS 128

/* 
 *  Compilation trap to ensure 
 *  IX_ETHACC_CODELET_TXGEN_PCKS <= IX_ETHACC_CODELET_TX_MBUF_POOL_SIZE
 */
#if IX_ETHACC_CODELET_TXGEN_PCKS > IX_ETHACC_CODELET_TX_MBUF_POOL_SIZE
    #error Number of TX Gen packets is greater than Tx MBuf pool
#endif

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_TXGENRXSINK_VERIFY
 *
 * @brief Verify payload for TxGenRxSink operation. 
 *        To undefine, change to #undef.
 * 
 * @Warning Defining this will affect performance
 */
#undef IX_ETHACC_CODELET_TXGENRXSINK_VERIFY 

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_RX_FCS_STRIP
 *
 * @brief Strip FCS from incoming frames.
 *        To undefine, change to #undef. 
 */
#define IX_ETHACC_CODELET_RX_FCS_STRIP 

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_RX_FCS_APPEND
 *
 * @brief Append FCS for outgoing frames.
 *        To undefine, change to #undef. 
 */
#undef IX_ETHACC_CODELET_TX_FCS_APPEND

/**
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_FRAME_SIZE
 *
 * @brief Maximum size of a frame. 

 * This maximum frame size includes different network settings :
 * @li Ethernet frames (up to 1518 bytes), 
 * @li BabyJumbo frames (up to nearly 1600 bytes) 
 * @li Jumbo frames (9K bytes). 
 * Note that different encapsulation types may
 * extend the MTU size of 9000. The NPE firmware
 * compares only the overall ethernet frame size (MSDU), with may be
 * stripped from the FCS at the time of comparison. The frame size is 
 * affected by other operations which changes the frame size, like the 
 * addition or the removal of VLAN tags.
 *
 */
#ifdef IX_ETHACC_CODELET_RX_FCS_STRIP
#define IX_ETHACC_CODELET_FRAME_SIZE (14+9000) /* eth header + 9K MTU */
#else
#define IX_ETHACC_CODELET_FRAME_SIZE (14+9000+4)/* eth header + 9K MTU + FCS */
#endif


/** 
 * @ingroup IxEthAccCodelet 
 *
 * @def IX_ETHACC_CODELET_RX_FCS_STRIP
 *
 * @brief Type of operations of the Ethernet Access Codelet. 
*/
typedef enum
{
    IX_ETHACC_CODELET_RX_SINK = 1,    /**< All frames received (from external device)
                                        *  will be sinked for available ports. */
    IX_ETHACC_CODELET_SW_LOOPBACK,    /**< All frames received are software loopbacked 
                                        *  to the same port for available ports */
    IX_ETHACC_CODELET_TXGEN_RXSINK,   /**< Frames generated and transmitted from port 1, 
                                        *  remote loopbacked to port 2 by using cross cable
					*  and received on port 2. */
    IX_ETHACC_CODELET_PHY_LOOPBACK,   /**< Frames generated and PHY loopbacked on the 
                                        *  same port for available ports. */
    IX_ETHACC_CODELET_BRIDGE,         /**< Frames received on one port will be transmitted
                                        *  through the other port. */
    IX_ETHACC_CODELET_BRIDGE_QOS,     /**< Frames received on one port will be transmitted
                                        *  through the other port, with priority enabled */
    IX_ETHACC_CODELET_BRIDGE_FIREWALL,/**< Frames received on one port will be transmitted
                                        *  through the other port if the MAC address match 
					*  the firewall criteria */
    IX_ETHACC_CODELET_ETH_LEARNING,   /**< Ethernet Learning Facility where it adds some static
                                        *  and dynamic entries. Dynamic entries are then aged and 
					*  verified that they no longer appear in the database. */
    IX_ETHACC_CODELET_BRIDGE_WIFI     /**< Ethernet 802.3 <=> 802.11 header conversion test */
} IxEthAccCodeletOperation;

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */

/*
 * Function definitions
 */

/** 
 * @ingroup IxEthAccCodelet
 *
 * @fn ixEthAccCodeletMain(IxEthAccCodeletOperation operationType,
 *                         IxEthAccPortId inPort,
 *                         IxEthAccPortId outPort,
 *			   int disableStats)
 *
 * @brief This function is used as a single point of execution for EthAcc codelet.
 *
 * @param "IxEthAccCodeletOperation [in] operationType" - The type of operation 
 *        to be executed. Refer to the descriptions above.
 *
 * @return
 * - IX_SUCCESS : If operation selected is successfully setup
 * - IX_FAIL    : If operation selected fails to be setup.
 */
PUBLIC IX_STATUS
ixEthAccCodeletMain(IxEthAccCodeletOperation operationType,
                    IxEthAccPortId inPort,
                    IxEthAccPortId outPort,
		    int disableStats);

#endif /* IXETHACCCODELET_H */

/** @} defgroup IxEthAccCodelet*/

/** @} defgroup Codelets*/

