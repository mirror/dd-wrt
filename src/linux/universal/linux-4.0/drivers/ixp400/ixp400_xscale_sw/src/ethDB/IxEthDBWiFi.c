/**
 * @file IxEthDBAPI.c
 *
 * @brief Implementation of the public API
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

#include "IxEthDB_p.h"

/* forward prototypes */
IX_ETH_DB_PRIVATE MacTreeNode *ixEthDBGatewaySelect(MacTreeNode *stations, UINT32 *gwCount);
IX_ETH_DB_PRIVATE IxEthDBStatus ixEthDBWiFiEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBWiFiRecData *wifiRecData);
IX_ETH_DB_PRIVATE IxEthDBStatus ixEthDBDownloadGatewayTable (IxEthDBPortId portID, MacTreeNode *gateways);
IX_ETH_DB_PRIVATE IxEthDBStatus ixEthDBDownloadBssidTable (IxEthDBPortId portID, MacTreeNode *bssids);
IX_ETH_DB_PRIVATE IxEthDBStatus ixEthDBDuplicateAddressCheck(MacTreeNode *rootNode, MacTreeNode *currentNode, UINT32 *dgwIndexAddr);

/* forward prototypes */
IX_ETH_DB_PUBLIC MacTreeNode *ixEthDBGatewaySelect(MacTreeNode *stations, UINT32 *gwCount);

/**
 * @brief sets the BSSID value for the WiFi header conversion feature
 *
 * @param portID ID of the port
 * @param bssid pointer to the 6-byte BSSID value
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiBSSIDSet(IxEthDBPortId portID, IxEthDBMacAddr *bssid)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);
 
    IX_ETH_DB_CHECK_REFERENCE(bssid);
    
    if (ixOsalMemCopy(ixEthDBPortInfo[portID].bssid, bssid, IX_IEEE803_MAC_ADDRESS_SIZE) == NULL)
    {
       return IX_ETH_DB_FAIL;
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief updates the Frame Control and Duration/ID WiFi header
 * conversion parameters in an NPE
 *
 * @param portID ID of the port
 *
 * This function will send a message to the NPE updating the 
 * frame conversion parameters for 802.3 => 802.11 header conversion.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or IX_ETH_DB_FAIL otherwise
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiFrameControlDurationIDUpdate(IxEthDBPortId portID)
{
    IxNpeMhMessage message;
    IX_STATUS result;

    FILL_SETFRAMECONTROLDURATIONID(message, portID, ixEthDBPortInfo[portID].frameControlDurationID);
    
    IX_ETHDB_SEND_NPE_MSG(IX_ETHNPE_PHYSICAL_ID_TO_NODE(portID), message, result);
    
    return result;
}

/**
 * @brief sets the Duration/ID WiFi frame header conversion parameter
 *
 * @param portID ID of the port
 * @param durationID 16-bit value containing the new Duration/ID parameter
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiDurationIDSet(IxEthDBPortId portID, UINT16 durationID)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    ixEthDBPortInfo[portID].frameControlDurationID = (ixEthDBPortInfo[portID].frameControlDurationID & 0xFFFF0000) | durationID;
    
    return ixEthDBWiFiFrameControlDurationIDUpdate(portID);
}

/**
 * @brief sets the Frame Control WiFi frame header conversion parameter
 *
 * @param portID ID of the port
 * @param durationID 16-bit value containing the new Frame Control parameter
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiFrameControlSet(IxEthDBPortId portID, UINT16 frameControl)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    ixEthDBPortInfo[portID].frameControlDurationID = (ixEthDBPortInfo[portID].frameControlDurationID & 0xFFFF) | (frameControl << 16); 
    
    return ixEthDBWiFiFrameControlDurationIDUpdate(portID);
}

/**
 * @brief removes a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to remove
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    MacDescriptor recordTemplate;
    
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_REFERENCE(macAddr);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);
    
    ixOsalMemCopy(recordTemplate.macAddress, macAddr, IX_IEEE803_MAC_ADDRESS_SIZE);
    
    recordTemplate.type   = IX_ETH_DB_WIFI_RECORD;
    recordTemplate.portID = portID;
    
    return ixEthDBRemove(&recordTemplate, NULL);
}

/**
 * @brief adds a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 * @param wifiRecData pointer to the wifi specific data (flags, gw etc.)
 * (GW address is NULL if this is a station record)
 *
 * This function adds a record of type AP_TO_AP (gateway is not NULL)
 * or AP_TO_STA (gateway is NULL) in the main database as a
 * WiFi header conversion record.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBWiFiEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBWiFiRecData *wifiRecData)
{
    MacDescriptor recordTemplate;
    IxEthDBPortMap query;
    MacTreeNode *stations = NULL;
    MacTreeNode *gateways = NULL;
    PortInfo *portInfo;
    UINT32 gatewayCount = 1;

    IX_ETH_DB_CHECK_REFERENCE(wifiRecData);

    IX_ETH_DB_CHECK_REC_TYPE(wifiRecData->recType);

    IX_ETH_DB_CHECK_ODD_PAD(wifiRecData->padLength);

    IX_ETH_DB_CHECK_PAD_LENGTH(wifiRecData->padLength);

    IX_ETH_DB_CHECK_LOG_PORT(wifiRecData->logicalPortID);

    IX_ETH_DB_CHECK_WIFI_VLAN_TAG(wifiRecData->vlanTagFlag);

    if (wifiRecData->recType != IX_ETH_DB_WIFI_TO_ETHER && wifiRecData->recType != IX_ETH_DB_WIFI_TO_LOCAL)
    {
        IX_ETH_DB_CHECK_ADDR(wifiRecData->bssid);

        if (wifiRecData->recType == IX_ETH_DB_WIFI_AP_TO_AP)
        {
            IX_ETH_DB_CHECK_ADDR(wifiRecData->gatewayMacAddr);
        }
    }

    portInfo = &ixEthDBPortInfo[portID];

    if (portInfo->wifiRecordsCount >= MAX_ELT_SIZE)
    {
        ERROR_LOG("DB: In ixEthDBWiFiEntryAdd(): Exceeded the maximum limit of wi-fi records\n");
	return IX_ETH_DB_FAIL;
    }

    if (wifiRecData->recType == IX_ETH_DB_WIFI_AP_TO_STA || wifiRecData->recType == IX_ETH_DB_WIFI_AP_TO_AP)
    {
    	/* Check that the number of Wifi records do not exceed the maximum capacity */
    	SET_DEPENDENCY_MAP(query, portID);

    	stations = ixEthDBQuery(NULL, query, IX_ETH_DB_WIFI_RECORD, MAX_ELT_SIZE);
    	gateways = ixEthDBGatewaySelect(stations, &gatewayCount);

        if (stations != NULL)
            ixEthDBFreeMacTreeNode(stations);

        if (gateways != NULL)
            ixEthDBFreeMacTreeNode(gateways);

    	if (gatewayCount >= MAX_GW_SIZE)
    	{
           ERROR_LOG("DB: In ixEthDBWiFiEntryAdd(): Exceeded maximum limit of gateway entries\n");
	   return IX_ETH_DB_FAIL;
    	}

    	if (wifiRecData->recType == IX_ETH_DB_WIFI_AP_TO_AP)
    	{
           ixOsalMemCopy(recordTemplate.recordData.wifiData.gwMacAddress, wifiRecData->gatewayMacAddr, IX_IEEE803_MAC_ADDRESS_SIZE);
    	}
    	else
    	{
           ixOsalMemSet(recordTemplate.recordData.wifiData.gwMacAddress, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
    	}

        ixOsalMemCopy(recordTemplate.recordData.wifiData.bssid, wifiRecData->bssid, IX_IEEE803_MAC_ADDRESS_SIZE);
        recordTemplate.recordData.wifiData.recType = (wifiRecData->recType | (wifiRecData->vlanTagFlag <<2));
    }
    else
    {
        ixOsalMemSet(recordTemplate.recordData.wifiData.gwMacAddress, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
        ixOsalMemSet(recordTemplate.recordData.wifiData.bssid, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
        recordTemplate.recordData.wifiData.recType = (wifiRecData->recType | (0 <<2));
    }

    ixOsalMemCopy(recordTemplate.macAddress, macAddr, IX_IEEE803_MAC_ADDRESS_SIZE);

    recordTemplate.type   = IX_ETH_DB_WIFI_RECORD;
    recordTemplate.portID = portID;

    /* This value will be overwritten for AP and STA cases later */
    recordTemplate.recordData.wifiData.recIndex = portID;

    /* Divide pad length by 2 because we have only 4 bits to store the value 16 */
    recordTemplate.recordData.wifiData.padLength     = (wifiRecData->padLength)/2;
    recordTemplate.recordData.wifiData.logicalPortID = wifiRecData->logicalPortID;

    return ixEthDBAdd(&recordTemplate, NULL);
}

/**
 * @brief sets the BSSID/GW/Flag values in the WiFi header conversion table
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 * @param wifiRecData pointer to the wifi specific data (flags, gw, bssid etc.)
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiRecordEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBWiFiRecData *wifiRecData)
{
    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);
   
    return ixEthDBWiFiEntryAdd(portID, macAddr, wifiRecData);
}

/**
 * @brief adds a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 * @param gatewayMacAddr address of the gateway
 *
 * This function adds a record of type AP_TO_AP
 * in the main database as a WiFi header conversion record.
 *
 * This is simply a wrapper over @ref ixEthDBWiFiEntryAdd().
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiAccessPointEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, IxEthDBMacAddr *gatewayMacAddr)
{
    IxEthDBWiFiRecData wifiRecData;
    PortInfo *portInfo;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);
   
    IX_ETH_DB_CHECK_REFERENCE(gatewayMacAddr);

    portInfo = &ixEthDBPortInfo[portID];

    IX_ETH_DB_CHECK_ADDR(portInfo->bssid);

    ixOsalMemCopy(wifiRecData.bssid, portInfo->bssid, IX_IEEE803_MAC_ADDRESS_SIZE);
    ixOsalMemCopy(wifiRecData.gatewayMacAddr, gatewayMacAddr, IX_IEEE803_MAC_ADDRESS_SIZE);

    wifiRecData.recType = IX_ETH_DB_WIFI_AP_TO_AP;
    wifiRecData.vlanTagFlag = IX_ETH_DB_WIFI_VLAN_NOTAG;
    wifiRecData.padLength = IX_ETH_DB_WIFI_MIN_PAD_SIZE;
    wifiRecData.logicalPortID = 0xFF;

    return ixEthDBWiFiEntryAdd(portID, macAddr, &wifiRecData);
}

/**
 * @brief adds a WiFi header conversion record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to add
 *
 * This function adds a record of type AP_TO_STA
 * in the main database as a WiFi header conversion record.
 *
 * This is simply a wrapper over @ref ixEthDBWiFiEntryAdd().
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiStationEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    IxEthDBWiFiRecData wifiRecData;
    PortInfo *portInfo;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);
   
    portInfo = &ixEthDBPortInfo[portID];

    IX_ETH_DB_CHECK_ADDR(portInfo->bssid);

    ixOsalMemCopy(wifiRecData.bssid, portInfo->bssid, IX_IEEE803_MAC_ADDRESS_SIZE);
    ixOsalMemSet(wifiRecData.gatewayMacAddr, 0, IX_IEEE803_MAC_ADDRESS_SIZE);

    wifiRecData.recType = IX_ETH_DB_WIFI_AP_TO_STA;
    wifiRecData.vlanTagFlag = IX_ETH_DB_WIFI_VLAN_NOTAG;
    wifiRecData.padLength = IX_ETH_DB_WIFI_MIN_PAD_SIZE;
    wifiRecData.logicalPortID = 0xFF;

    return ixEthDBWiFiEntryAdd(portID, macAddr, &wifiRecData);
}

/**
 * @brief selects a set of gateways and stations from a tree of
 * WiFi header conversion records
 *
 * @param stations binary tree containing pointers to WiFi header
 * conversion records
 *
 * This function browses through the input binary tree, identifies
 * records of type AP_TO_AP and AP_TO_STA, clones these records and
 * appends them to a vine (a single right-branch binary tree) which
 * is returned as result. A maximum of MAX_GW_SIZE entries containing
 * gateways and bssids will be cloned from the original tree.
 *
 * @return vine (linear binary tree) containing record
 * clones of AP_TO_AP and AP_TO_STA types, which have a gateway field
 * and bssid filed
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
MacTreeNode *ixEthDBGatewaySelect(MacTreeNode *stations, UINT32 *gwCount)
{
    MacTreeNodeStack *stack;
    MacTreeNode *gateways, *insertionPlace;
    UINT32 gwIndex = 1; /* skip the empty root */
    IxEthDBStatus matchResult = IX_ETH_DB_FAIL;
    
    if (stations == NULL)
    {
        return NULL;
    }

    stack = ixOsalCacheDmaMalloc(sizeof (MacTreeNodeStack));

    if (stack == NULL)
    {
        ERROR_LOG("DB: (WiFi) failed to allocate the node stack for gateway tree linearization, out of memory?\n");
        return NULL;
    }
    
    /* initialize root node */
    gateways = insertionPlace = NULL;
        
    /* start browsing the station tree */
    NODE_STACK_INIT(stack);
    
    /* initialize stack by pushing the tree root at offset 0 */
    NODE_STACK_PUSH(stack, stations, 0);
    
    while (NODE_STACK_NONEMPTY(stack))
    {
        MacTreeNode *node;
        UINT32 offset, dgwIndex;
       
        NODE_STACK_POP(stack, node, offset);

        /* we can store maximum 40 (40 total, 1 empty root) entries in the gateway tree */
        if (gwIndex > MAX_GW_SIZE) break;
        
        /* check if this record has a valid BSSID address */
        if (node->descriptor != NULL && ((node->descriptor->recordData.wifiData.recType & 0x03) == IX_ETH_DB_WIFI_AP_TO_AP
                                      || (node->descriptor->recordData.wifiData.recType & 0x03) == IX_ETH_DB_WIFI_AP_TO_STA))
        {
            /* found a record, create an insertion place */
            if (insertionPlace != NULL)
            {
                matchResult = ixEthDBDuplicateAddressCheck(gateways, node, &dgwIndex);

                if (matchResult != IX_ETH_DB_SUCCESS)
                {
                   /* Not a duplicate node, add a node into the tree */
                   insertionPlace->right = ixEthDBAllocMacTreeNode();
                   insertionPlace        = insertionPlace->right;
                }
                else
                {
                   /* Address already exists in the tree, update the same index in the original record */
                   node->descriptor->recordData.wifiData.recIndex = dgwIndex;
                }
            }
            else
            {
		/* Add first node into the tree */
                gateways       = ixEthDBAllocMacTreeNode();
                insertionPlace = gateways;
            }

            if (insertionPlace == NULL)
            {
                /* no nodes left, bail out with what we have */
                ixOsalCacheDmaFree(stack);
                return gateways;
            }
            
            if (matchResult != IX_ETH_DB_SUCCESS)
            {
                /* clone the original record for the gateway tree */
                insertionPlace->descriptor = ixEthDBCloneMacDescriptor(node->descriptor);

                /* insert and update the offset in the original record */
                /* NPE expects index = [0,39], we need to offset gwIndex by one
		 * to fit the NPE's indexing 
		 */  
                node->descriptor->recordData.wifiData.recIndex = gwIndex-1;
	        (*gwCount) = gwIndex;
		gwIndex = gwIndex+1;
            }
        }
        
        /* browse the tree */
        if (node->left != NULL)
        {
            NODE_STACK_PUSH(stack, node->left, LEFT_CHILD_OFFSET(offset));
        }

        if (node->right != NULL)
        {
            NODE_STACK_PUSH(stack, node->right, RIGHT_CHILD_OFFSET(offset));
        }
    }
    
    ixOsalCacheDmaFree(stack);
    return gateways;    
}

/**
 * @brief duplicate gateway and bssid entries in the wi-fi record table checking function
 *
 * @param rootNode address of the root node gateway/bssid tree (1:1 mapped)
 * @param currentNode node to be searched in the gateway/bssid tree
 * @param dgwIndexAddr of a matched entry(duplicate) to update the index field of the main table
 *
 * This function browses through the input binary gateway/bssid tree, identifies
 * duplicate record and returns SUCCESS if the match founf in the tree else
 * returns a FAIL if matching address not found in the tree.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or else IX_ETH_DB_FAIL
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBDuplicateAddressCheck(MacTreeNode *rootNode, MacTreeNode *currentNode, UINT32 *dgwIndexAddr)
{
   MacTreeNode *wifiTree = NULL;

   wifiTree = rootNode;

   while (wifiTree != NULL)
   {
      if (currentNode->descriptor->recordData.wifiData.recType == IX_ETH_DB_WIFI_AP_TO_STA)
      {
         if (ixEthDBAddressCompare(wifiTree->descriptor->recordData.wifiData.bssid,
                currentNode->descriptor->recordData.wifiData.bssid)==0)
         {
             (*dgwIndexAddr) = wifiTree->descriptor->recordData.wifiData.recIndex;
             return IX_ETH_DB_SUCCESS;
         }
      }
      else
      {
         if ((ixEthDBAddressCompare(wifiTree->descriptor->recordData.wifiData.bssid,
                                    currentNode->descriptor->recordData.wifiData.bssid)==0) &&
             (ixEthDBAddressCompare(wifiTree->descriptor->recordData.wifiData.gwMacAddress,
                                    currentNode->descriptor->recordData.wifiData.gwMacAddress)==0))
         {
             (*dgwIndexAddr) = wifiTree->descriptor->recordData.wifiData.recIndex;
             return IX_ETH_DB_SUCCESS;
         }

      }
      wifiTree = wifiTree->right;
   }

   return IX_ETH_DB_FAIL;
}

/**
 * @brief downloads the WiFi header conversion GW table to an NPE
 *
 * @param portID ID of the port
 * @param gateways MAC addresses
 *
 * This function prepares the WiFi header conversion GW tables and
 * downloads them to the specified NPE port.
 *
 * The header conversion tables consist in the main table of
 * addresses and the secondary table of gateways. AP_TO_AP records
 * from the first table contain index fields into the second table
 * for gateway selection. But AP_TO_STA contains NULL
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBDownloadGatewayTable (IxEthDBPortId portID, MacTreeNode *gateways)
{
    MacTreeNode *gateway = NULL;
    IxNpeMhMessage message;
    PortInfo *portInfo;
    IX_STATUS result;

    portInfo = &ixEthDBPortInfo[portID];

    ixOsalMemSet((void *) portInfo->updateMethod.npeGwUpdateZone, 0, FULL_GW_BYTE_SIZE);

    /* write all gateways */
    gateway = gateways;

    while (gateway != NULL)
    {
        ixEthDBNPEGatewayNodeWrite(gateway, portInfo->updateMethod.npeGwUpdateZone,
            gateway->descriptor->recordData.wifiData.recIndex, FULL_GW_BYTE_SIZE);

        gateway = gateway->right;
    }

    FILL_SETAPMACTABLE_MSG(message,
        IX_OSAL_MMU_VIRT_TO_PHYS(portInfo->updateMethod.npeGwUpdateZone));

    IX_ETHDB_SEND_NPE_MSG(IX_ETHNPE_PHYSICAL_ID_TO_NODE(portID), message, result);

    return result;
}

/**
 * @brief downloads the WiFi header conversion bssid table to an NPE
 *
 * @param portID ID of the port
 * @param bssids - AP MAC addresses
 *
 * This function prepares the WiFi header conversion bssid tables and
 * downloads them to the specified NPE port.
 *
 * The header conversion tables consist in the main table of
 * addresses and the secondary table of bssids. AP_TO_AP and AP_TO_STA
 * records from the first table contain index fields into the second table
 * for bssid selection.
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PRIVATE
IxEthDBStatus ixEthDBDownloadBssidTable (IxEthDBPortId portID, MacTreeNode *bssids)
{
    MacTreeNode *bssid = NULL;
    IxNpeMhMessage message;
    PortInfo *portInfo;
    IX_STATUS result;

    portInfo = &ixEthDBPortInfo[portID];

    ixOsalMemSet((void *) portInfo->updateMethod.npeBssidUpdateZone, 0, FULL_BSSID_BYTE_SIZE);

    /* write all gateways */
    bssid = bssids;

    while (bssid != NULL)
    { 
        ixEthDBNPEBssidNodeWrite(bssid, portInfo->updateMethod.npeBssidUpdateZone,
            bssid->descriptor->recordData.wifiData.recIndex, FULL_BSSID_BYTE_SIZE);

        bssid = bssid->right;
    }

    FILL_SETBSSIDTABLE_MSG(message,
        IX_OSAL_MMU_VIRT_TO_PHYS(portInfo->updateMethod.npeBssidUpdateZone));

    IX_ETHDB_SEND_NPE_MSG(IX_ETHNPE_PHYSICAL_ID_TO_NODE(portID), message, result);

    return result;
}

/**
 * @brief downloads the WiFi header conversion table to an NPE
 *
 * @param portID ID of the port
 *
 * This function prepares the WiFi header conversion tables and
 * downloads them to the specified NPE port.
 *
 * The header conversion tables consist in the main table of
 * addresses and the secondary table of gateways and BSSIDs. Records
 * from the first table contain index fields into the second table
 * for gateway selection.This index is same for both GW & BSSID tables.
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBWiFiConversionTableDownload(IxEthDBPortId portID)
{
    IxEthDBPortMap query;
    MacTreeNode *stations = NULL;
    MacTreeNode *gateways = NULL;
    PortInfo *portInfo;
    IX_STATUS result;
    UINT32 dummy;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_WIFI_HEADER_CONVERSION);

    portInfo = &ixEthDBPortInfo[portID];

    SET_DEPENDENCY_MAP(query, portID);

    ixEthDBUpdateLock();

    stations = ixEthDBQuery(NULL, query, IX_ETH_DB_WIFI_RECORD, MAX_ELT_SIZE);
    gateways = ixEthDBGatewaySelect(stations, &dummy);

    result = ixEthDBDownloadGatewayTable (portID, gateways);

    if (result == IX_SUCCESS)
    {
        result = ixEthDBDownloadBssidTable (portID, gateways);
    }

    /* free the gateways tree */
    if (gateways != NULL)
    {
        ixEthDBFreeMacTreeNode(gateways);
    }

    if (result == IX_SUCCESS)
    {
        /* update the main tree (the stations tree) */
        portInfo->updateMethod.searchTree = stations;

        result = ixEthDBNPEUpdateHandler(portID, IX_ETH_DB_WIFI_RECORD);
    }

    ixEthDBUpdateUnlock();

    return result;
}

