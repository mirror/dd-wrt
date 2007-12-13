/**
 * @file IxEthAccCodeletSwBridgeFirewall.c
 *
 * @date 22 April 2004
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements a simple bridge between two Ethernet ports 
 * with a Firewall set at the NPE level.
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

/*
 * Number of MAC Addresses to add to the Firewall Database
 */
#define IX_ETHACC_CODELET_FIREWALL_ADDRESS_COUNT (17)

/*
 * Base Firewall MAC address to add to the Firewall Database. A total of
 * IX_ETHACC_CODELET_FIREWALL_ADDRESS_COUNT addresses incrementing from
 * this one will be added to the database
 */
PRIVATE IxEthDBMacAddr 
ixEthAccCodeletFirewallMacAddress = {{0,0,11,22,33,0}};

PRIVATE IxEthDBMacAddr 
ixEthAccCodeletFirewallMacMask = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/*
 * Function definition: ixEthAccCodeletSwBridgeFirewallStart()
 *
 * Configure the Firewall and start the bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeFirewallStart(IxEthAccPortId firstPortId, 
					       IxEthAccPortId secondPortId)
{
    UINT32 firstPortCbTag = firstPortId | (secondPortId << 16);
    UINT32 secondPortCbTag = secondPortId | (firstPortId << 16);
    UINT32 addressCount;
    UINT32 i;
    IxEthDBMacAddr macAddr;
    IxEthDBFeature featureSet1 = 0;
    IxEthDBFeature featureSet2 = 0;

    if (firstPortId == secondPortId)
    {
	printf("SwBridgeFirewall: Cannot configure a Bridge Firewall Operation between port %u and port %u (ports must be different)\n",
	       firstPortId, 
               secondPortId);
	return (IX_FAIL);
    }

   /* Configure the 2 ports, register the tx & rx callbacks */
    if ( ixEthAccCodeletPortConfigure(firstPortId, 
				      ixEthAccCodeletBridgeRxCB, 
				      (IxEthAccPortMultiBufferRxCallback) NULL,
				      ixEthAccCodeletBridgeTxCB,
				      firstPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to configure Port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletPortConfigure(secondPortId, 
				      ixEthAccCodeletBridgeRxCB, 
				      NULL,
				      ixEthAccCodeletBridgeTxCB,
				      secondPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to configure Port %u\n",
	       secondPortId);
	return (IX_FAIL);
    }

    /* Enable the Firewall Feature in EthDB for each port but first 
     * check that the Firmware downloaded to the NPE can support it
     */
    ixEthDBFeatureCapabilityGet((IxEthDBPortId)firstPortId, &featureSet1);
    
    if ((featureSet1 & IX_ETH_DB_FIREWALL) == 0)
    {
	printf("SwBridgeFirewall: Port %u NPE image not firewall capable\n",
	       firstPortId);
	return (IX_FAIL);
    }
    if ( ixEthDBFeatureEnable((IxEthDBPortId)firstPortId, 
			      IX_ETH_DB_FIREWALL,
			      TRUE) != IX_ETH_DB_SUCCESS )
    {
	printf("SwBridgeFirewall: Failed to enable the firewall on port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    /* Repeat for the second port */
    ixEthDBFeatureCapabilityGet((IxEthDBPortId)secondPortId, &featureSet2);
    
    if ((featureSet2 & IX_ETH_DB_FIREWALL) == 0)
    {
	printf("SwBridgeFirewall: Port %u NPE image not firewall capable\n",
	       secondPortId);
	return (IX_FAIL);
    }
    if ( ixEthDBFeatureEnable((IxEthDBPortId)secondPortId, 
			      IX_ETH_DB_FIREWALL,
			      TRUE) != IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to enable the firewall on port %u\n", 
	       secondPortId);
	return (IX_FAIL);
    }

    /* Enable the Port in EthDB in order to configure and download the
     * Firewall Database 
     */ 
    if ((ixEthDBPortEnable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeFirewall: Cannot enable port %u\n", firstPortId);
        return (IX_FAIL);
    }
    if ((ixEthDBPortEnable(secondPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeFirewall: Cannot enable port %u\n", secondPortId);
        return (IX_FAIL);
    }
    
    /* generate a list of incremented MAC addresses to add to the 
     * firewall database
     */
    /* Non mask-based firewall */ 
    for (addressCount = 0; 
	 addressCount < IX_ETHACC_CODELET_FIREWALL_ADDRESS_COUNT;
	 addressCount++)
      {
        /* Add firewall entry for first port */
	if ((featureSet1 & IX_ETH_DB_ADDRESS_MASKING) == 0)
	  {
	    ixOsalMemCopy( macAddr.macAddress,
		    ixEthAccCodeletFirewallMacAddress.macAddress,
		    IX_IEEE803_MAC_ADDRESS_SIZE);
	    macAddr.macAddress[IX_IEEE803_MAC_ADDRESS_SIZE-1] = (UINT8)addressCount;	

	    if (ixEthDBFirewallEntryAdd(firstPortId, &macAddr)
		!= IX_ETH_DB_SUCCESS)
	      {
		printf("SwBridgeFirewall: Failed to add an Entry for Port %u\n", 
		       firstPortId);
		return (IX_FAIL);
	      }
	  }
	else
	  {
	    ixEthAccCodeletFirewallMacAddress.macAddress[IX_IEEE803_MAC_ADDRESS_SIZE-1]
	      = (UINT8)addressCount;
	    for (i = 0; i < IX_IEEE803_MAC_ADDRESS_SIZE; i++)
	      {
		macAddr.macAddress[i] = ixEthAccCodeletFirewallMacAddress.macAddress[i] & 
		  ixEthAccCodeletFirewallMacMask.macAddress[i];
	      }            
	    
	    if (ixEthDBFirewallMaskedEntryAdd(firstPortId, &macAddr, &ixEthAccCodeletFirewallMacMask)
		!= IX_ETH_DB_SUCCESS)
	      {
		printf("SwBridgeFirewall: Failed to add an masked Entry for Port %u\n", 
		       firstPortId);
		return (IX_FAIL);
	      }
	  }

        /* Add firewall entry for second port */
	if ((featureSet2 & IX_ETH_DB_ADDRESS_MASKING) == 0)
	  {
	    ixOsalMemCopy( macAddr.macAddress,
		    ixEthAccCodeletFirewallMacAddress.macAddress,
		    IX_IEEE803_MAC_ADDRESS_SIZE);
	    macAddr.macAddress[IX_IEEE803_MAC_ADDRESS_SIZE-1] = (UINT8)addressCount;	

	    if (ixEthDBFirewallEntryAdd(secondPortId, &macAddr)
		!= IX_ETH_DB_SUCCESS)
	      {
		printf("SwBridgeFirewall: Failed to add an Entry for Port %u\n", 
		       secondPortId);
		return (IX_FAIL);
	      }
	  }
	else
	  {
	    ixEthAccCodeletFirewallMacAddress.macAddress[IX_IEEE803_MAC_ADDRESS_SIZE-1]
	      = (UINT8)addressCount;
	    for (i = 0; i < IX_IEEE803_MAC_ADDRESS_SIZE; i++)
	      {
		macAddr.macAddress[i] = ixEthAccCodeletFirewallMacAddress.macAddress[i] & 
		  ixEthAccCodeletFirewallMacMask.macAddress[i];
	      }            
	    
	    if (ixEthDBFirewallMaskedEntryAdd(secondPortId, &macAddr, &ixEthAccCodeletFirewallMacMask)
		!= IX_ETH_DB_SUCCESS)
	      {
		printf("SwBridgeFirewall: Failed to add an masked Entry for Port %u\n", 
		       firstPortId);
		return (IX_FAIL);
	      }
	  }

	if ((featureSet1 & IX_ETH_DB_ADDRESS_MASKING) == 0)
	  {
            printf("Mac Address = "); 	    
	    for (i = 0; i < IX_IEEE803_MAC_ADDRESS_SIZE; i++)
	      {
		printf("%2.2x:", (UINT32)macAddr.macAddress[i]);
	      }	    
	    printf(" added to the white list\n");
	  }
	else
	  {
            printf("Mac Address = ");
	    for (i = 0; i < IX_IEEE803_MAC_ADDRESS_SIZE; i++)
	      {
		printf("%2.2x:", (UINT32)macAddr.macAddress[i]);
	      }
            printf("  Mac Mask = ");
	    for (i = 0; i < IX_IEEE803_MAC_ADDRESS_SIZE; i++)
	      {
		printf("%2.2x:", (UINT32)ixEthAccCodeletFirewallMacMask.macAddress[i]);
	      }	    
	    printf(" added to the white list\n");
	  } 	
      }
    
    printf("Download the firewall table to the NPEs ...\n");
    if (ixEthDBFirewallTableDownload(firstPortId)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to download the Firewall DB for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }
    if (ixEthDBFirewallTableDownload(secondPortId)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to download the Firewall DB for port %u\n", 
	       secondPortId);
	return (IX_FAIL);
    }

    /* Setup the firewall mode */    
    if (ixEthDBFirewallModeSet(firstPortId, 
			       IX_ETH_DB_FIREWALL_WHITE_LIST)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to set the Firewall mode for port %u\n", firstPortId);
	return (IX_FAIL);
    }
    if (ixEthDBFirewallModeSet(secondPortId, 
			       IX_ETH_DB_FIREWALL_WHITE_LIST)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to set the Firewall mode for port %u\n", secondPortId);
	return (IX_FAIL);
    }

    /* Allow traffic to pass by enabling both ports, that is if the incoming
     * destination MAC address is in the Firewall database 
     */
    if ( ixEthAccPortEnable(firstPortId) != IX_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to enable Port %u\n", firstPortId);
	return (IX_FAIL);
    }
    if ( ixEthAccPortEnable(secondPortId) != IX_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to enable Port %u\n", secondPortId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletSwBridgeFirewallStop()
 *
 * Unconfigure Firewall and Stop bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeFirewallStop(IxEthAccPortId firstPortId, 
					      IxEthAccPortId secondPortId)
{
    /* Stop the data bridge */
    if (ixEthAccCodeletSwBridgeStop(firstPortId, secondPortId) 
	!= IX_SUCCESS)
    {
	printf("SwBridgeFirewall: Failed to unconfigure the bridge ports\n");
	return (IX_FAIL);
    }

    /* Enable the EthDB ports to clear the firewall configuration */
    if ((ixEthDBPortEnable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeFirewall: Cannot enable port %u\n", firstPortId);
        return (IX_FAIL);
    }
    if ((ixEthDBPortEnable(secondPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeFirewall: Cannot enable port %u\n", secondPortId);
        return (IX_FAIL);
    }    

    printf("Clear the firewall databases on both NPEs\n");   
    if (ixEthDBDatabaseClear(firstPortId, 
			     IX_ETH_DB_FIREWALL_RECORD)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Fail to clear the Firewall database for port %u\n", 
	       firstPortId);
	return (IX_FAIL);
    }
    if (ixEthDBDatabaseClear(secondPortId, 
			     IX_ETH_DB_FIREWALL_RECORD)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeFirewall: Fail to clear the Firewall database for port %u\n", 
	       secondPortId);
	return (IX_FAIL);
    }

    /* Disable the Firewall Feature to return to the initial processing
     * capabilities
     */
    if ( ixEthDBFeatureEnable((IxEthDBPortId) firstPortId, 
			      IX_ETH_DB_FIREWALL, 
			      FALSE) != IX_ETH_DB_SUCCESS )
    {
	printf("SwBridgeFirewall: Failed to disable the Firewall Feature for port %u\n", 
	       firstPortId);
	return (IX_FAIL);
    }
    if ( ixEthDBFeatureEnable((IxEthDBPortId) secondPortId, 
			      IX_ETH_DB_FIREWALL, 
			      FALSE) != IX_ETH_DB_SUCCESS )
    {
	printf("SwBridgeFirewall: Failed to disable the Firewall Feature for port %u\n",
	       secondPortId);
	return (IX_FAIL);
    }

    /* disable the ethDB port */
    if ((ixEthDBPortDisable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeFirewall: Cannot disable port %u\n", firstPortId);
        return (IX_FAIL);
    }
    if ((ixEthDBPortDisable(secondPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeFirewall: Cannot disable port %u\n", secondPortId);
        return (IX_FAIL);
    }    

    return (IX_SUCCESS);
}
