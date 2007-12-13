/**
 * @file IxEthAccCodeletSwBridgeWiFi.c
 *
 * @date 25 August 2004
 *
 * @brief This file contains the implementation of the Ethernet Access Codelet.
 *
 * This file implements a simple bridge between two Ethernet ports with a 
 * 802.3 <=> 802.11 frame header conversion set up at NPE level.
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

/*
 * Put the system defined include files required.
 */

/*
 * Put the user defined include files required.
 */

#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxOsal.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/* prototypes */
void ixEthAccCodeletWiFiBridgeRxCB(UINT32 cbTag, IX_OSAL_MBUF* mBufPtr, UINT32 reserved);
IX_STATUS ixEthAccCodeletSwWiFiBridgeStart(IxEthAccPortId wifiPortId, IxEthAccPortId bridgedPortId);
UINT32 ixEthAccCodeletWiFiAddressCompare(UINT8 *mac1, UINT8 *mac2);

PRIVATE IxEthDBMacAddr ixEthAccCodeletWiFiStaMacAddress   = { {0x00, 0x00, 0x11, 0x22, 0x33, 0x44} };
PRIVATE IxEthDBMacAddr ixEthAccCodeletWiFiApMacAddress    = { {0x00, 0x00, 0xA1, 0xA2, 0xA3, 0xA4} };
PRIVATE IxEthDBMacAddr ixEthAccCodeletWiFiGwMacAddress    = { {0x00, 0x00, 0xF9, 0xF8, 0xF7, 0xF6} };
PRIVATE IxEthDBMacAddr ixEthAccCodeletWiFiBSSIDMacAddress = { {0x00, 0xFF, 0xFE, 0xFD, 0xFC, 0xFB} };

PRIVATE UINT16 ixEthAccCodeletWiFiDurationID   = 0x1234;
PRIVATE UINT16 ixEthAccCodeletWiFiFrameControl = 0xABCD;

PRIVATE IxEthDBMacAddr ixEthAccCodeletWiFiRecStaAddress   = { {0x00, 0x00, 0x55, 0x66, 0x77, 0x88} };
PRIVATE IxEthDBMacAddr ixEthAccCodeletWiFiRecApAddress    = { {0x00, 0x00, 0xB1, 0xB2, 0xB3, 0xB4} };

IxEthDBWiFiRecData  ixEthAccCodeletWiFiRecStaData;
IxEthDBWiFiRecData  ixEthAccCodeletWiFiRecApData;

extern const char *mac2string(const unsigned char *mac);

/*
 * Function definition: ixEthAccCodeletSwBridgeWiFiStart()
 *
 * Configure WiFi header conversion and Start bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeWiFiStart(IxEthAccPortId wifiPortId, 
					       IxEthAccPortId bridgedPortId)
{
    IxEthDBFeature featureSet = 0;
    UINT32 firstPortCbTag = wifiPortId | (bridgedPortId << 16);
    UINT32 secondPortCbTag = bridgedPortId | (wifiPortId << 16);

    ixEthAccCodeletWiFiRecStaData.recType = IX_ETH_DB_WIFI_AP_TO_STA;
    ixEthAccCodeletWiFiRecStaData.vlanTagFlag = IX_ETH_DB_WIFI_VLAN_NOTAG;
    ixEthAccCodeletWiFiRecStaData.padLength = IX_ETH_DB_WIFI_MIN_PAD_SIZE; 
    ixEthAccCodeletWiFiRecStaData.logicalPortID = 0x0E; 
    ixOsalMemSet(ixEthAccCodeletWiFiRecStaData.gatewayMacAddr, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
    ixOsalMemCopy(ixEthAccCodeletWiFiRecStaData.bssid, &ixEthAccCodeletWiFiBSSIDMacAddress, IX_IEEE803_MAC_ADDRESS_SIZE);

    ixEthAccCodeletWiFiRecApData.recType = IX_ETH_DB_WIFI_AP_TO_AP;
    ixEthAccCodeletWiFiRecApData.vlanTagFlag = IX_ETH_DB_WIFI_VLAN_NOTAG; 
    ixEthAccCodeletWiFiRecApData.padLength = IX_ETH_DB_WIFI_MIN_PAD_SIZE; 
    ixEthAccCodeletWiFiRecApData.logicalPortID = 0x0E; 
    ixOsalMemCopy(ixEthAccCodeletWiFiRecApData.gatewayMacAddr, &ixEthAccCodeletWiFiGwMacAddress, IX_IEEE803_MAC_ADDRESS_SIZE);
    ixOsalMemCopy(ixEthAccCodeletWiFiRecApData.bssid, &ixEthAccCodeletWiFiBSSIDMacAddress, IX_IEEE803_MAC_ADDRESS_SIZE);

    if (wifiPortId == bridgedPortId)
    {
	printf("SwBridgeWiFi: Cannot configure a Bridge Operation between port %u and port %u (ports must be different)\n",
	       wifiPortId, 
               bridgedPortId);
	return (IX_FAIL);
    }

    /* Perform basic port configuration (actual callbacks will be registered later, 
     * in ixEthAccCodeletSwWiFiBridgeStart) */

    /* Configure and register the traffic callbacks for both ports */
    if ( ixEthAccCodeletPortConfigure(wifiPortId, 
				      ixEthAccCodeletBridgeRxCB, 
				      (IxEthAccPortMultiBufferRxCallback) NULL,
				      ixEthAccCodeletBridgeTxCB,
                                      firstPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to configure Port %u\n", wifiPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletPortConfigure(bridgedPortId, 
				      ixEthAccCodeletWiFiBridgeRxCB, 
				      (IxEthAccPortMultiBufferRxCallback) NULL,
				      ixEthAccCodeletBridgeTxCB,
                                      secondPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to configure Port %u\n", bridgedPortId);
	return (IX_FAIL);
    }

    /* Enable the Port in EthDB in order to configure and download the
     * WiFi Database 
     */ 
    if (ixEthDBPortEnable(wifiPortId) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeWiFi: Cannot enable port %u\n", wifiPortId);
        return (IX_FAIL);
    }

    if (ixEthDBPortEnable(bridgedPortId) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeWiFi: Cannot enable port %u\n", bridgedPortId);
        return (IX_FAIL);
    }

    /* check feature */
    if (IX_ETH_DB_SUCCESS != ixEthDBFeatureCapabilityGet(wifiPortId, &featureSet))
    {
        printf("SwBridgeWiFi: Could not get the feature capabilities of the selected WiFi port (%d)\n", wifiPortId);
        return IX_FAIL;
    }

    if ((featureSet & IX_ETH_DB_WIFI_HEADER_CONVERSION) == 0)
    {
        printf("SwBridgeWiFi: Selected WiFi port (%d) does not have an NPE image capable of 802.11 header conversion\n", wifiPortId);
        return IX_FAIL;
    }

    featureSet = 0;

    if (IX_ETH_DB_SUCCESS != ixEthDBFeatureCapabilityGet(bridgedPortId, &featureSet))
    {
        printf("SwBridgeWiFi: Could not get the feature capabilities of the selected bridge port (%d)\n", bridgedPortId);
        return IX_FAIL;
    }

    if ((featureSet & IX_ETH_DB_WIFI_HEADER_CONVERSION) != 0)
    {
        printf("SwBridgeWiFi: Selected bridge port (%d) should not have an NPE image capable of 802.11 header conversion\n", bridgedPortId);
        return IX_FAIL;
    }

    /* enable feature */
    if (IX_ETH_DB_SUCCESS != ixEthDBFeatureEnable(wifiPortId, IX_ETH_DB_WIFI_HEADER_CONVERSION, TRUE))
    {
        printf("SwBridgeWiFi: Failed to enable the WiFi header conversion feature, exiting\n");
        return IX_FAIL;
    }

    /* set the conversion table */
    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiBSSIDSet(wifiPortId, &ixEthAccCodeletWiFiBSSIDMacAddress))
    {
        printf("SwBridgeWiFi: Failed to set the BSSID value, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiStationEntryAdd(wifiPortId, &ixEthAccCodeletWiFiStaMacAddress))
    {
        printf("SwBridgeWiFi: Failed to add Station address, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiAccessPointEntryAdd(wifiPortId, 
        &ixEthAccCodeletWiFiApMacAddress, 
        &ixEthAccCodeletWiFiGwMacAddress))
    {
        printf("SwBridgeWiFi: Failed to add AccessPoint address, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiRecordEntryAdd(wifiPortId, 
        &ixEthAccCodeletWiFiRecStaAddress, 
        &ixEthAccCodeletWiFiRecStaData))
    {
        printf("SwBridgeWiFi: Failed to add Station address, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiRecordEntryAdd(wifiPortId, 
        &ixEthAccCodeletWiFiRecApAddress, 
        &ixEthAccCodeletWiFiRecApData))
    {
        printf("SwBridgeWiFi: Failed to add AccessPoint address, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiConversionTableDownload(wifiPortId))
    {
        printf("SwBridgeWiFi: Failed to download WiFi frame header conversion table, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiFrameControlSet(wifiPortId, ixEthAccCodeletWiFiFrameControl))
    {
        printf("SwBridgeWiFi: Failed to set the FrameControl value, exiting\n");
        return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBWiFiDurationIDSet(wifiPortId, ixEthAccCodeletWiFiDurationID))
    {
        printf("SwBridgeWiFi: Failed to set the Duration/ID value, exiting\n");
        return IX_FAIL;
    }

    /* display user instructions */
    printf("\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("| Sending packets to port %d using [%s] as the destination MAC  |\n", wifiPortId, mac2string(ixEthAccCodeletWiFiRecStaAddress.macAddress));
    printf("| will convert their headers to 802.11 STA->AP frames. Capture the frames on  |\n");
    printf("| port %d with your test equipment to view the conversion result.              |\n", bridgedPortId);
    printf("|                                                                             |\n");
    printf("| Sending packets to port %d using [%s] as the destination MAC  |\n", wifiPortId, mac2string(ixEthAccCodeletWiFiRecApAddress.macAddress));
    printf("| will convert their headers to 802.11 AP->AP frames. The conversion uses     |\n");
    printf("| [%s] as gateway MAC address. Capture the frames on port %d    |\n", mac2string(ixEthAccCodeletWiFiGwMacAddress.macAddress), bridgedPortId);
    printf("| with your test equipment to view the conversion result.                     |\n");
    printf("|                                                                             |\n");
    printf("| Sending 802.11 frames generated as above to port %d, when bridged to port %d, |\n", bridgedPortId, wifiPortId);
    printf("| will convert them to 802.3 frames. Observe the output on port %d.            |\n", wifiPortId);
    printf("|                                                                             |\n");
    printf("| This test is using : [%s] as BSSID                           |\n", mac2string(ixEthAccCodeletWiFiBSSIDMacAddress.macAddress));
    printf("|                      0x%4x as FrameControl                                 |\n", ixEthAccCodeletWiFiFrameControl);
    printf("|                      0x%4x as Duration/ID                                  |\n", ixEthAccCodeletWiFiDurationID);
    printf("-------------------------------------------------------------------------------\n");
    
    
    return ixEthAccCodeletSwWiFiBridgeStart(wifiPortId, bridgedPortId);
}

/*
 * Function definition: ixEthAccCodeletSwBridgeStart()
 *
 * Start bridge datapath
 */
IX_STATUS ixEthAccCodeletSwWiFiBridgeStart(IxEthAccPortId wifiPortId, 
				       IxEthAccPortId bridgedPortId)
{
    /* Enable the traffic over both ports */
    if ( ixEthAccPortEnable(wifiPortId) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to enable Port %u\n", wifiPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccPortEnable(bridgedPortId) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to enable Port %u\n", bridgedPortId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

UINT32 ixEthAccCodeletWiFiAddressCompare(UINT8 *mac1, UINT8 *mac2)
{
    UINT32 local_index;

    for (local_index = 0 ; local_index < IX_IEEE803_MAC_ADDRESS_SIZE ; local_index++)
    {
        if (mac1[local_index] > mac2[local_index])
        {
            return 1;
        }
        else if (mac1[local_index] < mac2[local_index])
        {
            return -1;
        }
    }

    return 0;
}

/**
 * Function definition: ixEthAccCodeletWiFiBridgeRxCB()
 *
 * Receive callback for Bridge Operation. Received frames are retransmitted
 * on the port loaded with the WiFi header conversion image, and 802.11
 * frames are configured (by setting the link_prot flag) to be converted
 * into 802.3 format.
 */

void ixEthAccCodeletWiFiBridgeRxCB(UINT32 cbTag, 
			       IX_OSAL_MBUF* mBufPtr, 
			       UINT32 reserved)
{
    /* Transmit the buffer on the other port */
    UINT16 frameControl;
    UINT16 durationID;
    UINT16 logicalPortID;
    UINT8 *firstAddr;
    UINT16 linkProt = 0;

    IxEthAccPortId wifiPortId    = cbTag >> 16;
    IxEthAccPortId bridgedPortId = cbTag & 0xFFFF;

    ixEthAccCodeletStats[bridgedPortId].rxCount++;

    /* 802.11 frames generated by the codelet can be detected by
     * inspecting the Frame Control, Duration/ID and first 802.11 address */
    IX_OSAL_CACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(mBufPtr), IX_OSAL_MBUF_MLEN(mBufPtr));

    frameControl  = IX_OSAL_SWAP_BE_SHARED_SHORT(* ((UINT16 *) IX_OSAL_MBUF_MDATA(mBufPtr)));
    durationID    = IX_OSAL_SWAP_BE_SHARED_SHORT(*(((UINT16 *) IX_OSAL_MBUF_MDATA(mBufPtr)) + 1));
    firstAddr     = ((UINT8 *) IX_OSAL_MBUF_MDATA(mBufPtr)) + 4;
    logicalPortID = (UINT16) IX_ETHACC_NE_DESTPORTID(mBufPtr);

    if ((frameControl == ixEthAccCodeletWiFiFrameControl) && (durationID == ixEthAccCodeletWiFiDurationID))
    {
        if (ixEthAccCodeletWiFiAddressCompare(firstAddr, ixEthAccCodeletWiFiGwMacAddress.macAddress) == 0)
        {
            /* AP to AP frame, set link_prot to 11b << 4*/
            linkProt = IX_ETHACC_RX_APTYPE;
        }
        else if (ixEthAccCodeletWiFiAddressCompare(firstAddr, ixEthAccCodeletWiFiRecStaAddress.macAddress) == 0)
        {
            /* AP to STA frame, set link_prot to 10b << 4 */
            linkProt = IX_ETHACC_RX_STATYPE;
        }

        IX_ETHACC_NE_FLAGS(mBufPtr) &= ~IX_ETHACC_NE_LINKMASK;
        IX_ETHACC_NE_FLAGS(mBufPtr) |= linkProt;
    }

    if(ixEthAccPortTxFrameSubmit(wifiPortId, mBufPtr, IX_ETH_ACC_TX_DEFAULT_PRIORITY) != IX_ETH_ACC_SUCCESS)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
	    "Tx Buffer submission failed on port 1\n", 0, 0, 0, 0, 0, 0);
    }
}

/*
 * Function definition: ixEthAccCodeletSwBridgeWiFiStop()
 *
 * Unconfigure WiFi and Stop bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeWiFiStop(IxEthAccPortId firstPortId, 
                                          IxEthAccPortId secondPortId)
{
    /* Stop the data bridge */
    if (ixEthAccCodeletSwBridgeStop(firstPortId, secondPortId)
        != IX_SUCCESS)
    {
        printf("SwBridgeWiFi: Failed to unconfigure the bridge ports\n");
        return (IX_FAIL);
    }

    /* Enable the EthDB ports to clear the firewall configuration */
    if ((ixEthDBPortEnable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("Cannot enable port %u\n", firstPortId);
        return (IX_FAIL);
    }

    /* clears the WiFi address table */
    if (IX_ETH_DB_SUCCESS != ixEthDBDatabaseClear(firstPortId, IX_ETH_DB_WIFI_RECORD))
    {
	printf("Failed to clear the WiFi database\n");
	return IX_FAIL;
    }

    if (IX_ETH_DB_SUCCESS != ixEthDBFeatureEnable(firstPortId, IX_ETH_DB_WIFI_HEADER_CONVERSION, FALSE))
    {
        printf("Failed to disable the WiFi header conversion feature\n");
        return IX_FAIL;
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

    return IX_SUCCESS;
} 
