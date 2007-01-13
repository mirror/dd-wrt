/**
 * @file IxEthAal5App.h
 *
 * @author Intel Corporation
 * @date 3 Apr 2002
 *
 * @brief This file contains the declaration of the IxEthAal5App
 * component's main functions, etc.
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
#ifndef IXETHAAL5APP_H
#define IXETHAAL5APP_H

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
 * @defgroup IxEthAal5App Intel (R) IXP400 Software Ethenet Aal5 (IxEthAal5App) API 
 *
 * @brief Intel (R) IXP400 Software Ethernet Aal5 Codelet component API
 * 
 * @par
 * IxEthAal5App application is also called as IXP4XX Mini Bridge application
 * which  bridges traffic between Ethernet and Utopia ports or Ethernet
 * and ADSL ports. It uses ixEthAcc,ixAtmdAcc, ixAtmm, ixAtmSch and ixQmgr
 * software components.
 *
 * <b> VxWorks User Guide </b><br>
 * ixEthAal5AppCodeletMain() function is used as a single point of execution
 * for EthAal5 Codelet, which allows the user to enter in 2 type of modes:
 * Utopia or ADSL, in order to operate together with ethernet.
 *
 * <pre>
 *  <i> Usage :
 *      ixEthAal5AppCodeletMain (modeType)
 *	  modeType:
 *                  1 = Utopia
 *                  2 = ADSL
 * </i>
 * </pre>
 *
 * <b> Linux* User Guide </b><br>
 * ixEthAal5AppCodeletMain() function is used as a single point of execution 
 * for EthAal5 Codelet, which allows the user to enter in 2 type of modes:
 * Utopia or ADSL, in order to operate together with ethernet.
 *
 * <pre>
 *  <i> Usage :
 *      # insmod ixp400_codelets_ethAal5App.o modeType=<x>
 *      Where x:
 *                  1 = Utopia
 *                  2 = ADSL
 * </i>
 * </pre>
 *
 * Note for VxWorks* and Linux* Usage:
 * In order to observe the current traffic counters, the ixEAAShow() function
 * is executed every 15seconds. This applys to both Utopia and ADSL mode.
 *
 *
 * @par
 * <B>Features</B><br>
 * This codelet currently supports 2 Ethernet ports and up to 8VCs per 8 
 * Utopia phys (MPHY mode) or up to 8VCs per Utopia phy (SPHY mode), which will
 * be initialized at the start of application. In SPHY mode, the codelet sets
 * up 1 VC per UTOPIA phy by default. The codelet is also capable to configure
 * up to 32VCs (see "How to Setup 32VCs" section)
 * 
 * Ethernet frames are transferred across ATM link (through Utopia interface) 
 * using AAL5 protocol and Ethernet frame encapsulation described by RFC 1483. 
 * MAC address learning is performed on Ethernet frames, received by Ethernet 
 * ports and ATM interface (encapsulated). Application filters packets base 
 * on destination MAC addresses - packets are forwarded to other port only if 
 * the port has ever received packet/frame with the same source MAC address. 
 * Forwarding is done only between Ethernet and Utopia port. 
 * 
 * Several simplifications were made to keep code simple: 
 *  - Application doesn't allow packet forwarding between Ethernet ports (nor 
 *    Utopia ports).
 *  - flooding (forwarding frames/packets with unknown MAC addresses) is
 *    not supported. Two IxEthAal5App will never transfer any packets between 
 *    each other, because initialy MAC data base is empty, so all packets will
 *    be filtered out. However there is function ixEAAAddMAC which can be used
 *    to add MAC address to the data base and assign it to one of available 
 *    ports. To enable simplified flooding see comments in ixEAAEthRxCallback.
 *  - This application can not be executed more than once. It doesn't 
 *    deinitialize itself. If user wishes to change configuration and run 
 *    application again the whole system (vxWorks*) must be restarted.
 *  - currently Mac Learning/Filtering database in ixEthAcc component supports
 *    only Ethernet ports. For that reason it couldn't be used in this 
 *    application for learning Mac addresses from encapsulated Ethernet frames
 *    received from Utopia. In the near future ixEthAcc component will support
 *    all possible ports (including Utopia), but by this time a very simplified
 *    approach is used in this application: 
 *       only one Mac address is stored per VC (and there is one VC per Phy). 
 *       It means, that only one Mac address is supported simultaneously per
 *       Phy. This is done to keep code as simple as possible.
 *  - This application provides two choices of connection: DSL or UTOPIA. 
 *    DSL will be established (modeType = 2) if the DSL card is attached to the 
 *    IXDP4XX board
 *  - 2 protocols from RFC 1483 are recognized: The first packet received from 
 *    ATM will decide the behaviour of the application (ether bridged or 
 *    routed)
 *
 * @par
 * <B>How to Setup 32VC</B><br>
 * In order to setup 32VCs there are several steps involved:
 *    1) Change the IX_EAA_NUM_ATM_VCS to 32
 *    2) In ixEAASetupVc function, 
 *           - Change the configuration of each VC, i.e. which VC setup what
 *             what type of QoS and its parameters
 *           - Add more VPI and VCI values to the atmVpi and atmVci variables
 *             respectively
 *
 * @par
 * <B>From RFC 1483</B><br>
 *  VC Based Multiplexing of Routed Protocols
 *
 *  PDUs of routed protocols shall be carried as such in the Payload of
 *  the AAL5 CPCS-PDU.  The format of the AAL5 CPCS-PDU Payload field
 *  thus becomes:
 *
 * @verbatim
               Payload Format for Routed PDUs
               +-------------------------------+
               |             .                 |
               |         Carried PDU           |
               |    (up to 2^16 - 1 octets)    |
               |             .                 |
               |             .                 |
               +-------------------------------+
   @endverbatim
 *
 * VC Based Multiplexing of Bridged Protocols
 *
 *  PDUs of bridged protocols shall be carried in the Payload of the AAL5
 *  CPCS-PDU.
 *
 * @verbatim
               Payload Format for Bridged Ethernet/802.3 PDUs
               +-------------------------------+
               |         PAD 0x00-00           |
               +-------------------------------+
               |    MAC destination address    |
               +-------------------------------+
               |                               |
               |   (remainder of MAC frame)    |
               |                               |
               +-------------------------------+
               | LAN FCS (VC dependent option) |
               +-------------------------------+
   @endverbatim
 *  
 * EthAal5App Codelet Data flow is illustrated in the diagram below:
 * @verbatim
   +--------------------------------------------------------------------------+
   |       IXP4XX                                                             |
   | +------------------------------------------------------------------------+
   | |       XScale                                                           |
   | |                                                                        |
   | |    +-------------------------------------------------------------------+
   | |    |       ethAal5App                                                  |
   | |    |                                                                   |
   | |    |   +--------------------------+      +------------------------+    |
   | |    |   | ixEAAEthTxDoneCallback   |      | ixEAAAtmTDonexCallback |    |
   | |    |   | -----------------        |      | -----------------      |    |
   | |  +---->| After mbuffer data was   |      | After mbuffer data was |    |
   | |  | |   | sent, ixEthAcc component |   +--|sent, ixEthAcc component|<-+ |
   | |  | |   | returns buffer back. It  |   |  | returns buffer back. It|  | |
   | |  | |   | will be given back to    |   |  | will be given back to  |  | |
   | |  | |   | ixAtmdAcc component.     |   |  | ixAtmdAcc component.   |  | |
   | |  | |   +--------------------------+   |  +------------------------+  | |
   | |  | |                           |      |                              | |
   | |  | |                           |      |                              | |
   | |  | |   +--------------------+  |      |  +--------------------+      | |
   | |  | |   | ixEAAEthRxCallback |  |      |  | ixEAAAtmRxCallback |      | |
   | |  | |   | -----------------  |  |      |  | -----------------  |      | |
   | |  | | +>| Received frame is  |  |      |  | Ethernet frame is  |      | |
   | |  | | | | encapsulated into  |  |      |  | extracted from Atm |<-+   | |
   | |  | | | | Atm packet and sent|  |   +--+  | packet and sent    |  |   | |
   | |  | |Rx | to Utopia through  |  |   |     | to Eth phy through |  |   | |
   | |  | | | |ixAtmdAcc component.|  |   |     |ixEthAcc component. |  |   | |
   | |  | | | +--------------------+  |   |     +--------------------+  |   | |
   | |  | +---------------------|-----|---|--------------------|--------|---|-+
   | |  |   |         +--Tx-----|-----|---|--------------------+        |   | |
   | |  +---------+   |    +----|-----|---+  +---------+                |   | |
   | |  |ixEthAcc |<--+    |    |     +----->|ixAtmdAcc|-----Rx---------+   | |
   | |  |         |<-------+    +----Tx----->|         |--------------------+ |
   | +------------------------------------------------------------------------+
   |     |        \                               |
   |     |         \                              |
   | +-------+     +-------+                      |
   | | NPE B |     | NPE C |                      |
   | +-------+     +-------+                +-----------+
   |   |   |          |  |                  | Utopia    |
   |   |   |          |  |                  |           |
   |  Eth Mac        Eth Mac                |           |
   +-----------------------------------------------------------
       |                |                          |
       |                |                          |
     +---------+    +---------+                    |
     |Eth Phy 1|    |Eth Phy 2|                    
     +---------+    +---------+
   @endverbatim
 *     
 * @par
 * <B>Configuration Example 1: Using Smartbit and Adtech with UTOPIA 
 * connection </B>
 *
 * @verbatim
   +-----------+          +-----------------------+          +---------+
   | Smartbits | <------> | (Eth) IXP4XX (Utopia) | <------> | Adtech  |
   +-----------+          +-----------------------+          +---------+
   @endverbatim 

 *   On smartbits set: dst. mac = MAC1  and src mac = MAC2 <br>
 *   On Adtech add AAL5 with eth. encapsulation, dst. mac = MAC2, 
 *   src. mac = MAC1 <br>
 *
 *   Dependently which Phy is used Adtech must use same VPI/VCI address as 
 *   assigned to Phy by ethAal5App. By default there are 8 pre-configured VCs
 *   with default VPI and VCI values:
 *      VPI/VCI = 10/63
 *      VPI/VCI = 10/91
 *      VPI/VCI = 10/92
 *      VPI/VCI = 10/93
 *      VPI/VCI = 10/94
 *      VPI/VCI = 10/95
 *      VPI/VCI = 10/96
 *
 * Configuration Example 2: Using Smartbits/PCs with ADSL connection<br>
 * 
 * @verbatim
  +----------+       +--------+        +-------+       +----------+
  |Smartbits1| <---> | IXP4XX |<------>|  CO   | <---> |Smartbits2|
  |          |  Eth  |  CPE   |  ADSL  |       |  Eth  |          |
  +----------+       +--------+        +-------+       +----------+
   @endverbatim 
 *
 *   The ADSL connection is enabled by the codelet when the modeType=2
 *   and it is operating on SPHY mode.
 *
 *   On smartbits1 set: dst. mac = MAC1  and src mac = MAC2 <br>
 *   On smartbits2 set: dst. mac = MAC2  and src mac = MAC1 <br>
 *
 * Configuration Example 3: Using DSLAM and Adtech with ADSL connection<br>
 *
 * @verbatim
 *                         +------------------+
 *                         |      Adtech      |
 *                         +------------------+
 *              +----------| Eth     |   OC3  |-------+
 *              |          +---------+--------+       |
 *          Eth |                                     | OC3
                |     +--------+        +-------+     | 
                +---> | IXP4XX |<------>| DSLAM | <---+
                      |        |  ADSL  |       |       
                      +--------+        +-------+       
   @endverbatim 
 *
 * The VPI and VCI value is similar to Configuration Example 1. Ensure that
 * either the DSLAM matches the EthAal5App codelet's VPI/VCI default values
 * or the EthAal5App matches the DSLAM's VPI/VCI default values.
 *
 * By default, the codelet is setup to 1VC per ATM port (SPHY). In order to 
 * setup 8VC, please change the following:
 *                     #define IX_EAA_NUM_ATM_VCS 32
 *
 * On the Adtech, setup Eth frame type II for the AX4000 Eth card and 
 * configure the destination and source MAC address accordingly. 
 * For example the frame contains:
 *   Dest MAC Address = 00:00:00:00:03:01
 *   Src MAC Address: 00:00:00:00:02:01
 * Repeat this for all of the 8VCs
 * Also specify the frame size
 *
 * @{
 */

#include "IxOsal.h"
#include "IxEthAcc.h"
#include "IxAtmTypes.h"

#include "EthAal5User.h"
#include "IxEthAal5App_p.h"

/**
 * @ingroup IxEthAal5App
 *
 * @def IX_EAA_NUM_BUFFERS_PER_ETH
 * @brief  This is the number of buffers, which can be stored in free buffer 
 * queue for each ethernet port 
 */
#ifndef IX_EAA_NUM_BUFFERS_PER_ETH
#define IX_EAA_NUM_BUFFERS_PER_ETH  2048
#endif

/**
 * @ingroup IxEthAal5App
 *
 * @def IX_EAA_NUM_ATM_PORTS
 * @brief Define number of supported atm ports by this application
 *
 * User can define them in EthAal5User.h file to overwrite definitions below
 */
#ifndef  IX_EAA_NUM_ATM_PORTS
#if IX_UTOPIAMODE == 1
#define IX_EAA_NUM_ATM_PORTS     1
#else
#define IX_EAA_NUM_ATM_PORTS     8
#endif
#endif

/**
 * @ingroup IxEthAal5App
 *
 * @def IX_EAA_NUM_ATM_VCS
 * @brief Define number of supported atm VCs by this application
 *
 * User can define them in EthAal5User.h file to overwrite definitions below
 */
#ifndef  IX_EAA_NUM_ATM_VCS
#if IX_UTOPIAMODE == 1
#define IX_EAA_NUM_ATM_VCS       1
#else
#define IX_EAA_NUM_ATM_VCS       8
#endif
#endif

/**
 * @ingroup IxEthAal5App
 *
 * @def IX_EAA_NUM_BUFFERS_PER_VC
 * @brief This is the number of buffers per atm port 
 *
 * User can define them in EthAal5User.h file to overwrite definitions below
 */
#ifndef IX_EAA_NUM_BUFFERS_PER_VC
#define IX_EAA_NUM_BUFFERS_PER_VC 16 
#endif

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC1
 */
#define IX_EAA_VC1_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC2
 */

#define IX_EAA_VC2_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC3
 */
#define IX_EAA_VC3_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC4
 */
#define IX_EAA_VC4_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC5
 */
#define IX_EAA_VC5_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC6
 */
#define IX_EAA_VC6_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC7
 */
#define IX_EAA_VC7_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VPI for VC8
 */
#define IX_EAA_VC8_VPI 10

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC1
 */
#define IX_EAA_VC1_VCI 63

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC2
 */
#define IX_EAA_VC2_VCI 90

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC3
 */
#define IX_EAA_VC3_VCI 91

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC4
 */
#define IX_EAA_VC4_VCI 92

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC5
 */
#define IX_EAA_VC5_VCI 93

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC6
 */
#define IX_EAA_VC6_VCI 94

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC7
 */
#define IX_EAA_VC7_VCI 95

/**
 * @ingroup IxEthAal5App
 * @brief Define default VCI for VC8
 */
#define IX_EAA_VC8_VCI 96

/**
 * @ingroup IxEthAal5App 
 * 
 * @brief Define default for MAC1 address for ixEAAAddMac() function 
 */ 
#define IX_EAA_MAC1 00
/**
 * @ingroup IxEthAal5App
 *
 * @brief Define default for MAC2 address for ixEAAAddMac() function
 */
#define IX_EAA_MAC2 00
/**
 * @ingroup IxEthAal5App
 *
 * @brief Define default for MAC3 address for ixEAAAddMac() function
 */
#define IX_EAA_MAC3 00
/**
 * @ingroup IxEthAal5App
 *
 * @brief Define default for MAC4 address for ixEAAAddMac() function
 */
#define IX_EAA_MAC4 00

/**
 * @ingroup IxEthAal5App
 *
 * @brief Define default for MAC5 address for ixEAAAddMac() function
 */
#define IX_EAA_MAC5 03
/**
 * @ingroup IxEthAal5App
 *
 * @brief Define default for MAC6 address for ixEAAAddMac() function
 */
#define IX_EAA_MAC6 01
/**
 * @ingroup IxEthAal5App
 *
 * @brief Define default for Port number for ixEAAAddMac() function
 */
#define IX_EAA_PORT0             0

/**
 * @ingroup IxEthAal5App
 *
 * @brief Define medium thread priority for ixEAAPollTask() function
 */
#define IX_ETHAAL5APP_THREAD_PRI_MEDIUM       160


/**
 * @ingroup IxEthAal5App
 *
 * @fn ixEthAal5AppCodeletMain(IxEAAModeType modeType)
 *
 * @brief This is the main function that executes the EthAal5App codelet
 *
 * It first calls IxEAAMain() function which initialize the MAC database, 
 * to be an in valid Mac addresses (i.e. contain 0xffs), QMGR, NpeMh, Eth phys
 * - 100Mbit, FULL DUPLEX,  NO AUTONEGOTIATION (User can change those settings 
 * accordingly to required configuration), ATM, and Utopia interface.
 *
 * If Linux* is used, use interrupt mode - much faster under Linux* than polling
 *
 * If vxWorks* is used use poll mode - much faster under vxWorks* than 
 * interrupts and start background QMgr queues poll
 *
 * After which the main has been called, this ixEthAal5AppCodeletMain() function 
* will add the MAC address using ixEAAAddMAC() function. For a single PHY 
 * utopia mode, only one port will be setup a single MAC address. However, if
 * multiple phy is used, 8 ports will be setup and each port is assigned with
 * a unique MAC addresses.<br>
 * For single utopia phy. The following is setup using ixEAAAddMAC() function
 * @verbatim
        ixEAAAddMAC(IX_EAA_PORT0,
		    IX_EAA_MAC1, 
		    IX_EAA_MAC2, 
		    IX_EAA_MAC3, 
		    IX_EAA_MAC4, 
		    IX_EAA_MAC5, 
		    IX_EAA_MAC6);     
    @endverbatim
 *
 * For multi phy utopia the port number and  MAC6 increments using a for-loop
 * @verbatim
    for (port = 0, nextMac = 0; port < IX_EAA_NUM_ATM_PORTS; port++, nextMac++)
    {
	ixEAAAddMAC(IX_EAA_PORT0 + port,
		    IX_EAA_MAC1,
		    IX_EAA_MAC2,
		    IX_EAA_MAC3,
		    IX_EAA_MAC4,
		    IX_EAA_MAC5,
		    IX_EAA_MAC6 + nextMac);
    }
    @endverbatim
 *
 *
 * Lastly, ixEthAal5AppCodeletMain() creates a thread which purposed to display 
 * the EthAal5App codelet counter every 15secs
 */
PUBLIC IX_STATUS
ixEthAal5AppCodeletMain(IxEAAModeType modeType);

#endif 
/* IXETHAAL5APP_H */

/** @} defgroup IxEthAal5App*/

/** @} defgroup Codelets*/
