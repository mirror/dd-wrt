/**
 * @file IxEthAccCodeletPortSetup.c
 *
 * @date 22 April 2002
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements the Ethernet port setup of the IxEthAcc 
 * component
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

/*
 * Put the system defined include files required.
 */
#include "IxOsal.h"

/*
 * Put the user defined include files required.
 */
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

#ifdef __vxworks
#ifdef IX_ETHACC_CODELET_USE_NVRAM_MAC 
#include "config.h"
#endif
#endif

/**
 * Function definition: ixEthAccCodeletPortInit()
 *
 * Initialise the Port by
 * - Set the Port mode to promiscuous
 * - Set Tx Scheduling to be the default one
 * - Enable/Disable FCS support on Rx/Tx depending on the use flag
 *   definition
 * - Set the max frame size to handle to 9k bytes
 */

IX_STATUS ixEthAccCodeletPortInit(IxEthAccPortId portId)
{
    if (ixEthAccPortInit(portId) 
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error initialising Ethernet port %u!\n",
	       (UINT32)portId);	
	return (IX_FAIL);
    }

    /* Set ports to promiscuous mode  */
    if(ixEthAccPortPromiscuousModeSet(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error setting promiscuous mode for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    /* Set default scheduling discipline: no QoS on TX path */
    if (ixEthAccTxSchedulingDisciplineSet(portId, FIFO_NO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error setting scheduling discipline for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

#ifdef IX_ETHACC_CODELET_RX_FCS_STRIP
    if(ixEthAccPortRxFrameAppendFCSDisable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error disabling FCS for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
#else
    if(ixEthAccPortRxFrameAppendFCSEnable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error enabling FCS for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
#endif

#ifdef IX_ETHACC_CODELET_TX_FCS_APPEND
    if(ixEthAccPortTxFrameAppendFCSEnable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error enabling FCS for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    } 
#else
    if(ixEthAccPortTxFrameAppendFCSDisable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Error disabling FCS for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
#endif

    if ( ixEthDBFilteringPortMaximumFrameSizeSet(portId,
						 IX_ETHACC_CODELET_FRAME_SIZE) 
	 != IX_SUCCESS )
    {
	printf("PortSetup: Error setting the frame size for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    return IX_SUCCESS;
}

/**
 * Function definition: ixEthAccCodeletPortConfigure()
 *
 * Configure a Port by
 * - Clearing the Stats
 * - Registering the Rx and TxDone Callbacks
 *     There are 2 possible (exclusive) rx callbacks to be registered
 *     One handles single buffers (one buffer per call) and the other 
 *     handles an array of buffers in each call. Only one of 
 *     them need to be set as parameter, the other one should be NULL.
 * - Setting the port MAC address
 * - Performing the initial Replenish of the Rx Free Queue
 */

IX_STATUS 
ixEthAccCodeletPortConfigure(IxEthAccPortId portId,
	     IxEthAccPortRxCallback portRxCB,
	     IxEthAccPortMultiBufferRxCallback portMultiBufferRxCB,
	     IxEthAccPortTxDoneCallback portTxDoneCB,
	     UINT32 callbackTag)
{
    IxEthAccMacAddr npeMacAddr1 = IX_ETHACC_CODELET_NPEB_MAC;
    IxEthAccMacAddr npeMacAddr2 = IX_ETHACC_CODELET_NPEC_MAC;
    IxEthAccMacAddr npeMacAddr3 = IX_ETHACC_CODELET_NPEA_MAC;
    IxEthAccMacAddr npeMacAddr;
			     
    /* Clear stats */
    ixOsalMemSet(&ixEthAccCodeletStats[portId], 
	   0, 
	   sizeof(ixEthAccCodeletStats[portId]));

    /* check the PHY is up, ignore the result. This is called to 
     * display the current PHY status to the console
     */
    (void)ixEthAccCodeletLinkUpCheck(portId);
    
    /* register the datapath Rx callbacks */
    if (NULL != portRxCB)
    {
	if(ixEthAccPortRxCallbackRegister(portId,
					  portRxCB,
					  callbackTag) 
	   != IX_ETH_ACC_SUCCESS)
	{
	    printf("PortSetup: Failed to register Rx callback for port %u\n",
		   (UINT32)portId);
	    return (IX_FAIL);
	}
    }
    else if (NULL != portMultiBufferRxCB)
    {
	if(ixEthAccPortMultiBufferRxCallbackRegister(portId,
						     portMultiBufferRxCB,
						     callbackTag) 
	   != IX_ETH_ACC_SUCCESS)
	{
	    printf("PortSetup: Failed to register multiBuffer Rx callback for port %u\n",
		   (UINT32)portId);
	    return (IX_FAIL);
	}
    }
    else
    {
	printf("PortSetup: Failed to register any Rx callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    /* register the datapath TxDone callbacks */
    if(ixEthAccPortTxDoneCallbackRegister(portId,
					  portTxDoneCB,
					  callbackTag) != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to register Tx done callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

#if defined (__vxworks) && defined (IX_ETHACC_CODELET_USE_NVRAM_MAC)
    printf("\nReading MAC address information from non-volatile storage...\n");

    if (portId == IX_ETH_PORT_1)
    {
	if(sysNvRamGet((UINT8 *)&npeMacAddr.macAddress, 
		       IX_IEEE803_MAC_ADDRESS_SIZE, 
		       NV_MAC_ADRS_NPE1) == ERROR)
	{
	    printf("PortSetup: Unable to read MAC address from non-volatile storage!\n");
	    return (IX_FAIL);
	}
    }
    else if (portId == IX_ETH_PORT_2)
    {
	if(sysNvRamGet((UINT8 *)&npeMacAddr.macAddress, 
		       IX_IEEE803_MAC_ADDRESS_SIZE, 
		       NV_MAC_ADRS_NPE2) == ERROR)
	{
	    printf("PortSetup: Unable to read MAC address from non-volatile storage!\n");
	    return (IX_FAIL);
	}
    }
    else
    {
	printf("PortSetup: Unsupported port!\n");
	return (IX_FAIL);
    }
#else
    if (portId == IX_ETH_PORT_1)
    {
	ixOsalMemCopy(npeMacAddr.macAddress, 
	       npeMacAddr1.macAddress, 
	       IX_IEEE803_MAC_ADDRESS_SIZE);
    }
    else if (portId == IX_ETH_PORT_2)
    {
	ixOsalMemCopy(npeMacAddr.macAddress, 
	       npeMacAddr2.macAddress, 
	       IX_IEEE803_MAC_ADDRESS_SIZE);
    }
    else if (portId == IX_ETH_PORT_3)
    {
	ixOsalMemCopy(npeMacAddr.macAddress, 
	       npeMacAddr3.macAddress, 
	       IX_IEEE803_MAC_ADDRESS_SIZE);
    }
    else
    {
	printf("PortSetup: Unsupported port!\n");
	return (IX_FAIL);
    }
#endif
    
    printf ("Configure MAC address...\n");
    if(ixEthAccPortUnicastMacAddressSet(portId, &npeMacAddr)
	   != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to set the Unicast MAC Address of port %u\n",
	       portId);
	return (IX_FAIL);
    }
    
    printf("Port %u MAC address is:\t\n", (UINT32)portId);
    ixEthAccPortUnicastAddressShow(portId);
    
    printf ("Provision the first RX buffers...\n");
    if(ixEthAccCodeletReplenishBuffers(portId, 
				       IX_ETHACC_CODELET_RX_MBUF_POOL_SIZE/IX_ETHACC_CODELET_MAX_PORT) 
       != IX_SUCCESS)
    {
	printf("PortSetup: Error replenishing port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    return IX_SUCCESS;
}

/*
 * Function definition: ixEthAccCodeletPortUnconfigure()
 *
 * Unconfigure the Port by registering txDone and 
 * Rx callbacks to drop the traffic
 */

IX_STATUS 
ixEthAccCodeletPortUnconfigure(IxEthAccPortId portId)
{
    if (ixEthAccPortRxCallbackRegister(portId,
				       ixEthAccCodeletMemPoolFreeRxCB,
				       (UINT32) portId)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to register Rx callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
    if (ixEthAccPortTxDoneCallbackRegister(portId,
					   ixEthAccCodeletMemPoolFreeTxCB,
					   portId)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to register Tx done callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
 
    return IX_SUCCESS;
}

/*
 * Function definition: ixEthAccCodeletPortMultiBufferUnconfigure()
 *
 * Unconfigure the Port by registering txDone and multibuffer
 * Rx callbacks to drop the traffic
 */

IX_STATUS 
ixEthAccCodeletPortMultiBufferUnconfigure(IxEthAccPortId portId)
{
    if (ixEthAccPortMultiBufferRxCallbackRegister(portId,
				       ixEthAccCodeletMultiBufferMemPoolFreeRxCB,
				       portId)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to register Rx callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
    if (ixEthAccPortTxDoneCallbackRegister(portId,
					   ixEthAccCodeletMemPoolFreeTxCB,
					   portId)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to register Tx done callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }
 
    return IX_SUCCESS;
}
