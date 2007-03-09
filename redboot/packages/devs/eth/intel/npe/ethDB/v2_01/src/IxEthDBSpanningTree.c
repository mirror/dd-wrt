/**
 * @file IxEthDBSpanningTree.c
 *
 * @brief Implementation of the STP API
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
 * @brief sets the STP blocking state of a port
 *
 * @param portID ID of the port
 * @param blocked TRUE to block the port or FALSE to unblock it
 * 
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBSpanningTreeBlockingStateSet(IxEthDBPortId portID, BOOL blocked)
{
    IxNpeMhMessage message;
    IX_STATUS result;
    
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
 
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_SPANNING_TREE_PROTOCOL);
    
    ixEthDBPortInfo[portID].stpBlocked = blocked;

    FILL_SETBLOCKINGSTATE_MSG(message, portID, blocked);
    
    IX_ETHDB_SEND_NPE_MSG(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, result);
    
    return result;
}

/**
 * @brief retrieves the STP blocking state of a port
 *
 * @param portID ID of the port
 * @param blocked address to write the blocked status into
 *
 * Note that this function is documented in the main component
 * header file, IxEthDB.h.
 *
 * @return IX_ETH_DB_SUCCESS if the operation completed successfully
 * or an appropriate error message otherwise
 */
IX_ETH_DB_PUBLIC 
IxEthDBStatus ixEthDBSpanningTreeBlockingStateGet(IxEthDBPortId portID, BOOL *blocked)
{
    IX_ETH_DB_CHECK_PORT(portID);
    
    IX_ETH_DB_CHECK_SINGLE_NPE(portID);
 
    IX_ETH_DB_CHECK_FEATURE(portID, IX_ETH_DB_SPANNING_TREE_PROTOCOL);
    
    IX_ETH_DB_CHECK_REFERENCE(blocked);
    
    *blocked = ixEthDBPortInfo[portID].stpBlocked;
    
    return IX_ETH_DB_SUCCESS;
}
