/**
 * @file IxEthDBFirewall.c
 *
 * @brief Implementation of the firewall API
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */


#include "IxEthDB_p.h"

/**
 * @brief updates the NPE firewall operating mode and 
 * firewall address table
 *
 * @param portID ID of the port
 * @param epDelta initial entry point for binary searches (NPE optimization)
 * @param address address of the firewall MAC address table
 *
 * This function will send a message to the NPE configuring the
 * firewall mode (white list or black list), invalid source 
 * address filtering and downloading a new MAC address database 
 * to be used for firewall matching.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed 
 * successfully or IX_ETH_DB_FAIL otherwise
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFirewallUpdate(IxEthDBPortId portID, void *address, UINT32 epDelta)
{
    IxNpeMhMessage message;
    IX_STATUS result;
    
    UINT32 mode        = 0;    
    PortInfo *portInfo = &ixEthDBPortInfo[portID];

    mode = (portInfo->srcAddressFilterEnabled != FALSE) << 1 | (portInfo->firewallMode == IX_ETH_DB_FIREWALL_WHITE_LIST);

    FILL_SETFIREWALLMODE_MSG(message, 
        IX_ETH_DB_PORT_ID_TO_NPE_LOGICAL_ID(portID), 
        epDelta, 
        mode, 
        IX_OSAL_MMU_VIRT_TO_PHYS(address));

    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);
    
    return result;
}

/**
 * @brief configures the firewall white list/black list
 * access mode
 *
 * @param portID ID of the port
 * @param mode firewall filtering mode (IX_ETH_DB_FIREWALL_WHITE_LIST
 * or IX_ETH_DB_FIREWALL_BLACK_LIST)
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallModeSet(IxEthDBPortId portID, IxEthDBFirewallMode mode)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
     
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FIREWALL);
    
    if (mode != IX_ETH_DB_FIREWALL_WHITE_LIST
        && mode != IX_ETH_DB_FIREWALL_BLACK_LIST)
    {
        return IX_ETH_DB_INVALID_ARG;
    }    
    
    ixEthDBPortInfo[portID].firewallMode = mode;
    
    return ixEthDBFirewallTableDownload(portID);
}

/**
 * @brief enables or disables the invalid source MAC address filter
 *
 * @param portID ID of the port
 * @param enable TRUE to enable invalid source MAC address filtering
 * or FALSE to disable it
 *
 * The invalid source MAC address filter will discard, when enabled,
 * frames whose source MAC address is a multicast or the broadcast MAC
 * address.
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed 
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallInvalidAddressFilterEnable(IxEthDBPortId portID, BOOL enable)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
    
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FIREWALL);

    ixEthDBPortInfo[portID].srcAddressFilterEnabled = enable;
    
    return ixEthDBFirewallTableDownload(portID);
}

/**
 * @brief adds a firewall record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the new record
 *
 * This function will add a new firewall record
 * on the specified port, using the specified 
 * MAC address. If the record already exists this
 * function will silently return IX_ETH_DB_SUCCESS,
 * although no duplicate records are added.
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    MacDescriptor recordTemplate;

    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FIREWALL);
    
    memcpy(recordTemplate.macAddress, macAddr, sizeof (IxEthDBMacAddr));
    
    recordTemplate.type   = IX_ETH_DB_FIREWALL_RECORD;
    recordTemplate.portID = portID;
    
    return ixEthDBAdd(&recordTemplate, NULL);
}

/**
 * @brief removes a firewall record
 *
 * @param portID ID of the port
 * @param macAddr MAC address of the record to remove
 *
 * This function will attempt to remove a firewall
 * record from the given port, using the specified
 * MAC address.
 *
 * Note that this function is documented in the main
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully of an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallEntryRemove(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    MacDescriptor recordTemplate;
    
    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_REFERENCE(macAddr);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FIREWALL);
    
    memcpy(recordTemplate.macAddress, macAddr, sizeof (IxEthDBMacAddr));
    
    recordTemplate.type   = IX_ETH_DB_FIREWALL_RECORD;
    recordTemplate.portID = portID;
    
    return ixEthDBRemove(&recordTemplate, NULL);
}

/**
 * @brief downloads the firewall address table to an NPE
 *
 * @param portID ID of the port
 *
 * This function will download the firewall address table to
 * an NPE port.
 *
 * Note that this function is documented in the main 
 * component header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed
 * successfully or IX_ETH_DB_FAIL otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBFirewallTableDownload(IxEthDBPortId portID)
{
    IxEthDBPortMap query;
    IxEthDBStatus result;
    
    IX_ETH_DB_CHECK_PORT(portID);

    IX_ETH_DB_CHECK_SINGLE_NPE(portID);

    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_FIREWALL);
    
    SET_DEPENDENCY_MAP(query, portID);

    ixEthDBUpdateLock();
    
    ixEthDBPortInfo[portID].updateMethod.searchTree = ixEthDBQuery(NULL, query, IX_ETH_DB_FIREWALL_RECORD, MAX_FW_SIZE);
    
    result = ixEthDBNPEUpdateHandler(portID, IX_ETH_DB_FIREWALL_RECORD);

    ixEthDBUpdateUnlock();

    return result;
}
