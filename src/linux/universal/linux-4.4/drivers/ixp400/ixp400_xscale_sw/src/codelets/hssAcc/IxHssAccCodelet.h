/**
 * @file IxHssAccCodelet.h
 *
 * @date 26 Mar 2002
 *
 * @brief This file contains the interface for the HSS Access Codelet.
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
#ifndef IXHSSACCCODELET_H
#define IXHSSACCCODELET_H

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
 * @defgroup IxHssAccCodelet Intel (R) IXP400 Software HSS Access Codelet (IxHssAccCodelet) API
 *
 * @brief Intel (R) IXP400 Software HSS Access Codelet API. The interface for the HSS Access Codelet.
 *
 * This module contains the implementation of the HSS Access Codelet.
 * <P>
 * The following top-level operation is supported:
 * <UL>
 *   <LI>Test Packetised and Channelised Services, with the Codelet acting
 *       as data source/sink and HSS as loopback.
 *   The Codelet will transmit data and verify that data received is the
 * same as that transmitted. Codelet runs for IX_HSS_CODELET_DURATION_IN_MS ms. 
 *   <LI>There are four clients of Packetised service per HSS port - 1 client
 *	 per E1/T1 trunk. Client 0 and 2 are running in Packetised HDLC mode while 
 *	 client 1 and 3 in Packetised RAW mode.
 * </UL>
 *
 * <b> Assumptions </b><br>
 * In Channelised service, the codelet transmits traffic continuously. When
 * the codelet runs up to IX_HSS_CODELET_DURATION_IN_MS ms, Tx counter is bigger 
 * than Rx counter. This is due to the fact that traffics submitted to NPE (i.e. 
 * Tx counter has been increased) are not transmitted out by NPE when HSS service
 * is disabled. These traffics will be dropped and not loopbacked at HSS (Hence, 
 * Rx counter not increased).<br>
 *
 * In Packetised-raw mode service (client 1 and 3), Rx counter will be bigger than 
 * Tx counter because in this service, idle packets are received by Intel XScale(R)
 * processor and causes Rx counter to be bigger than Tx counter. As for packetised-HDLC
 * service, idle packets are handled in HDLC coprocessor and not passed to Intel 
 * XScale(R) processor (Hence, Rx counter not increased).     
 *
 * <b> Limitations </b><br>
 * When executing Packetised service on both HSS ports of 266MHz Intel (R) IXP42X
 * Processor simultaneously, receive traffic verification should fail on client 3 (i.e. 
 * Packetised-raw mode) of HSS port 1. The reason why this issue occured is IXP421
 * processor does not have enough CPU resources to perform intensive packet verification 
 * tasks. However, this does not imply that the same issue will hit customer 
 * applications because actual applications will not do any packet verifications 
 * in the way that the codelet does. This issue is not seen on a 533MHz Intel (R) 
 * IXP42X processor.
 *
 * <b> VxWorks* User Guide </b><br>
 * ixHssAccCodeletMain() function is used as a single point of execution for 
 * HssAcc Codelet.  
 *
 * <pre>
 *  <i> Usage :
 *      >ixHssAccCodeletMain (operationType, portMode, verifyMode)
 *      Where operationType:
 *           1 = Packetised Service Only.
 *           2 = Channelised Service Only.
 *           3 = Packetised Service and Channelised Services.
 *
 *      Where portMode:
 *           1 = HSS Port 0 Only.
 *           2 = HSS Port 1 Only.
 *           3 = HSS Port 0 and 1.
 *	     Note :IXP43X supports only HSS Port 0
 *
 *      Where verifyMode:
 *           1 = codelet verifies traffic received in hardware loopback mode.
 *           2 = codelet does not verify traffic received in hardware loopback mode.
 * 
 * </i>
 * </pre>
 *
 * <b> Linux* User Guide </b><br>
 * The idea of using the ixHssAccCodeletMain() as a single point of execution for
 * HssAcc codelet. The operation selected will be executed when user issue 'insmod' 
 * in command prompt.
 *
 * <pre>
 * <i>  Usage :
 *      >insmod ixp400_codelets_hssAcc.o operationType=(a) portMode=(b) verifyMode=(c)
 *      Where a:
 *           1 = Packetised Service Only.
 *           2 = Channelised Service Only.
 *           3 = Packetised Service and Channelised Services.
 *        
 *      Where b:
 *           1 = HSS Port 0 Only.
 *           2 = HSS Port 1 Only.
 *           3 = HSS Port 0 and 1.
 *	     Note :IXP43X supports only HSS Port 0
 *
 *      Where c:
 *           1 = codelet verifies traffic received in hardware loopback mode.
 *           2 = codelet does not verify traffic received in hardware loopback mode.
 *
 * </i>
 * </pre>
 *
 * <b> WinCE* User Guide </b><br>
 * The HSS Access Codelet uses serial console to print out menus and accept input from
 * users. Users need to choose and enter which operation to be executed from the menus.
 *
 * <pre>
 * <i>  Usage :
 *      Menu 1: Choose type of service you want to execute.
 *      Options:
 *           1 = Packetised Service Only.
 *           2 = Channelised Service Only.
 *           3 = Packetised Service and Channelised Services.
 *         100 = Exit HSS access codelet.
 *        
 *      Menu 2: Choose which port(s) you want to execute.
 *      Where b:
 *           1 = HSS Port 0 Only.
 *           2 = HSS Port 1 Only.
 *           3 = HSS Port 0 and 1.
 *         100 = Exit HSS access codelet. 
 *	     Note :IXP43X supports only HSS Port 0
 *
 *      Menu 3: Choose if codelet should or shouldn't verify traffic. 
 *      Where c:
 *           1 = codelet verifies traffic received in hardware loopback mode.
 *           2 = codelet does not verify traffic received in hardware loopback mode.
 *         100 = Exit HSS access codelet. 
 *
 * </i>
 * </pre>
 * <P>
 * <B>Buffer Management</B>
 * <P>
 * The packetised service uses mbuf buffers to store data, and chains mbufs
 * together to form large packets.  In the transmit direction, mbufs are
 * allocated from a pool on transmit, and returned to the pool on tranmsit
 * done.  For receive, mbufs are allocated from a pool when supplying
 * buffers to the free queue, and returned to the pool on receive.
 * <P>
 * The channelised service operates quite differently.  As voice data is
 * very sensitive to latency using mbufs for transferral of the data
 * between Intel XScale(R) processor and NPE is not very appropriate.  Instead,
 * circular buffers are used whereby the NPE reads data from a block of SDRAM
 * that the Intel XScale(R) processor writes to, and writes data to a block of 
 * SDRAM that Intel XScale(R) processor reads from.  On receive, the NPE writes 
 * directly into a circular buffer that Intel XScale(R) processor subsequently 
 * reads the data from. Each channel has its own circular buffer, and all 
 * these buffers are stored contiguously. On transmit, the NPE takes a 
 * circular list of pointers from Intel XScale(R) processor and transmits the data 
 * referenced by these pointers. Each list of pointers contains a pointer for
 * each channel, and the circular list of pointers contains multiple lists 
 * stored contiguously. This is to allow Intel XScale(R) processor to transmit voice 
 * samples without having to copy data, as only the pointer to the data blocks
 * needs to be written to SDRAM.  The NPE lets Intel XScale(R) processor know, in 
 * the form of Tx and Rx offsets, where in the blocks of SDRAM it is currently 
 * reading from and writing to. This enables Intel XScale(R) processor to co-ordinate 
 * its reading and writing activities to maintain the data flow. The Tx offset 
 * lets Intel XScale(R) processor know the list offset into the Tx circular pointer 
 * list that the NPE will next use to transmit. The Rx offset lets Intel 
 * XScale(R) processor know the byte offset into each channel's Rx circular buffer 
 * that the NPE will next receive data into.
 * <P>
 * <B>Caching</B>
 * <P>
 * To improve system performance, caching may be enabled for both the
 * channelised and packetised buffers.  To allow for this, buffers need to
 * be flushed before transmit, and invalidated after receive.  Flushing the
 * buffers before transmit ensures the NPE reads and transmits the correct
 * data.  Invalidating the buffers after receive ensures Intel XScale(R) processor
 * reads and processes the correct data.  In the case of the Codelet, all data
 * is flushed and invalidated as every byte is being written to on transmit
 * and every byte is verified on receive.  In a real application flushing
 * or invalidating all the data may not be necessary, only the data that
 * the application has written before transmit or will read after receive.
 * Note, regarding the packetised service, the IxHssAcc component itself
 * takes care of flushing and invalidating mbuf headers.  The application
 * needs only to concern itself with the mbuf data.
 * <P>
 * <B>Data Verification Strategy</B>
 * <P>
 * For both the packetised and channelised service a changing pattern will
 * be transmitted.  When the HSS co-processor is performing a loopback the
 * data received is expected to be the same as that transmitted.  The data
 * transmitted carries a byte pattern that begins at a known value and is
 * incremented for each byte.  An independent byte pattern is transmitted
 * for each channel of the channelised service, and also for each port of
 * the packetised service.  When data is received it is expected to match
 * the pattern that was transmitted.  For the channelised service the first
 * non-idle byte received is expected to be the beginning of the byte
 * pattern.  For the packetised service, RAW mode clients may receive idle
 * data so this is detected and ignored.  Only non-idle data is verified.
 * <P>
 * <B>56Kbps Packetised HDLC feature</B>
 * <P>
 * This feature is demonstrated in one of the packetised HDLC clients (i.e. 
 * client 2). The CAS bit is configured to be in the least signicant bit 
 * (LSB) position with bit polarity '1'. Bit inversion is also enabled on 
 * this client as well. The data verification strategy remains the same as
 * other packetised clients.
 * 
 * @{
 */

#include "IxOsal.h"
#include "IxHssAccCodelet_p.h"

/*
 * #defines for function return types, etc.
 */

/**
 * @ingroup IxHssAccCodelet                
 *
 * @def IX_HSSACC_CODELET_DURATIO_IN_MS
 *
 * @brief Duration (in ms) of codelet.
 *  
 */
#define IX_HSSACC_CODELET_DURATION_IN_MS 10000

/*
 * Prototypes for interface functions.
 */

/** 
 * @ingroup IxHssAccCodelet
 *
 * @fn ixHssAccCodeletMain(IxHssAccCodeletOperation operationType,
 *                         IxHssAccCodeletPortMode portMode,
 *                         IxHssAccCodeletVerifyMode verifyMode)
 *
 * @brief This function is used as a single point of execution for HssAcc codelet.
 *
 * @param "IxHssAccCodeletOperation [in] operationType" - The type of operation 
 *        to be executed. Refer to the descriptions above.
 * @param "IxHssAccCodeletPortMode [in] portMode" - portMode supported.
 *        Refer to the descriptions above.
 * @param " IxHssAccCodeletVerifyMode [in] verifyMode" - verify mode supported.
 *        Refer to the description above.
 *
 * @return
 * - IX_SUCCESS : If operation selected is successfully setup
 * - IX_FAIL    : If operation selected fails to be setup.
 */
PUBLIC IX_STATUS
ixHssAccCodeletMain(IxHssAccCodeletOperation operationType,
                    IxHssAccCodeletPortMode portMode,
                    IxHssAccCodeletVerifyMode verifyMode);

#endif /* IXHSSACCCODELET_H */

/** @} defgroup IxHssAccCodelet*/

/** @} defgroup Codelets*/


