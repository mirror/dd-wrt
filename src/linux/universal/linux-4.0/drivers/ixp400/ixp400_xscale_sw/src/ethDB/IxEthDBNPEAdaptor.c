/**
 * @file IxEthDBDBNPEAdaptor.c
 *
 * @brief Routines that read and write learning/search trees in NPE-specific format
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
#include "IxEthDBLog_p.h"

/* forward prototype declarations */
IX_ETH_DB_PUBLIC void ixEthDBELTShow(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBShowNpeMsgHistory(void);
IX_ETH_DB_PRIVATE UINT32 ixEthDBNullSerialize(MacTreeNode *node, 
					      void *address, UINT32 offset, UINT32 maxSize);
IX_ETH_DB_PRIVATE UINT32 ixEthDBNPELearningNodeWrite(MacTreeNode *node, void *baseAddress, 
						     UINT32 entryOffset, UINT32 maxByteSize);
IX_ETH_DB_PRIVATE UINT32 ixEthDBNPEWiFiNodeWrite(MacTreeNode *node, void *baseAddress, 
						 UINT32 entryOffset, UINT32 maxByteSize);
IX_ETH_DB_PRIVATE UINT32 ixEthDBNPEFirewallMaskedNodeWrite(MacTreeNode *node, void *baseAddress, 
							   UINT32 entryOffset, UINT32 maxByteSize);
IX_ETH_DB_PRIVATE UINT32 ixEthDBNPEFirewallNodeWrite(MacTreeNode *node, void *baseAddress, 
						     UINT32 entryOffset, UINT32 maxByteSize);

/* data */
UINT8* ixEthDBNPEUpdateArea[IX_ETH_DB_NUMBER_OF_PORTS];
UINT32 dumpEltSize;

/* private data */
IX_ETH_DB_PRIVATE IxEthDBNoteWriteFn ixEthDBNPENodeWrite[IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1];

#define IX_ETH_DB_MAX_DELTA_ZONES (6) /* at most 6 EP Delta zones, according to NPE FS */
IX_ETH_DB_PRIVATE UINT32 ixEthDBEPDeltaOffset[IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1][IX_ETH_DB_MAX_DELTA_ZONES]; 
IX_ETH_DB_PRIVATE UINT32 ixEthDBEPDelta[IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1][IX_ETH_DB_MAX_DELTA_ZONES];

/**
 * @brief allocates non-cached or contiguous NPE tree update areas for all the ports
 *
 * This function is called only once at initialization time from
 * @ref ixEthDBInit().
 *
 * @warning do not call manually
 *
 * @see ixEthDBInit()
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPEUpdateAreasInit(void)
{
    UINT32 portIndex;
    PortUpdateMethod *update;

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        update = &ixEthDBPortInfo[portIndex].updateMethod;

        if (ixEthDBPortDefinitions[portIndex].type == IX_ETH_NPE)
        {
            update->npeUpdateZone   = IX_OSAL_CACHE_DMA_MALLOC(FULL_ELT_BYTE_SIZE);

            if (update->npeUpdateZone == NULL)
            {
                ERROR_LOG("Fatal error: IX_OSAL_CACHE_DMA_MALLOC() returned NULL, no NPE update zones available\n");
            }
            else
            {
                ixOsalMemSet(update->npeUpdateZone, 0, FULL_ELT_BYTE_SIZE);
            }

            update->npeGwUpdateZone = IX_OSAL_CACHE_DMA_MALLOC(FULL_GW_BYTE_SIZE);

            if (update->npeGwUpdateZone == NULL)
            {
                ERROR_LOG("Fatal error: IX_OSAL_CACHE_DMA_MALLOC() returned NULL, no gateway update zones available\n");
            }
            else
            {
                ixOsalMemSet(update->npeGwUpdateZone, 0, FULL_GW_BYTE_SIZE);
	    }

	    update->npeBssidUpdateZone = IX_OSAL_CACHE_DMA_MALLOC(FULL_BSSID_BYTE_SIZE);

	    if (update->npeBssidUpdateZone == NULL)
            {
                ERROR_LOG("Fatal error: IX_OSAL_CACHE_DMA_MALLOC() returned NULL, no bssid update zones available\n");
            }
            else
            {
	  	ixOsalMemSet(update->npeBssidUpdateZone, 0, FULL_BSSID_BYTE_SIZE);
	    }

            update->vlanUpdateZone  = IX_OSAL_CACHE_DMA_MALLOC(FULL_VLAN_BYTE_SIZE);

            if (update->vlanUpdateZone == NULL)
            {
                ERROR_LOG("Fatal error: IX_OSAL_CACHE_DMA_MALLOC() returned NULL, no vlan update zones available\n");
            }
            else
            {
                ixOsalMemSet(update->vlanUpdateZone, 0, FULL_VLAN_BYTE_SIZE);
            }
        }
        else
        {
            /* unused */
            update->npeUpdateZone   = NULL;
            update->npeGwUpdateZone = NULL;
	    update->npeBssidUpdateZone = NULL;
            update->vlanUpdateZone  = NULL;
        }
    }
}

/**
 * @brief deallocates the NPE update areas for all the ports
 *
 * This function is called at component de-initialization time
 * by @ref ixEthDBUnload().
 *
 * @warning do not call manually
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPEUpdateAreasUnload(void)
{
    UINT32 portIndex;
    PortUpdateMethod *update;

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
	 update = &ixEthDBPortInfo[portIndex].updateMethod;

        if (ixEthDBPortDefinitions[portIndex].type == IX_ETH_NPE)
        {
	    if (update->npeUpdateZone != NULL)
	    {
               IX_OSAL_CACHE_DMA_FREE(update->npeUpdateZone);
	    }
	    if (update->npeGwUpdateZone != NULL)
	    {
               IX_OSAL_CACHE_DMA_FREE(update->npeGwUpdateZone);
	    }
	    if (update->npeBssidUpdateZone != NULL)
	    {
	       IX_OSAL_CACHE_DMA_FREE(update->npeBssidUpdateZone);
	    }
	    if (update->vlanUpdateZone != NULL)
	    {
               IX_OSAL_CACHE_DMA_FREE(update->vlanUpdateZone);
	    }
        }
    }
}

/**
 * @brief general-purpose NPE callback function
 *
 * @param npeID NPE ID
 * @param msg NPE message
 *
 * This function will unblock the caller by unlocking
 * the npeAckLock mutex defined for each NPE port
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNpeMsgAck(IxNpeMhNpeId npeID, IxNpeMhMessage msg)
{
    IxEthDBPortId portID = IX_ETHNPE_NODE_AND_PORT_TO_PHYSICAL_ID(npeID,0);
    PortInfo *portInfo;

    if (portID >= IX_ETH_DB_NUMBER_OF_PORTS)
    {
        /* invalid port */
        return;
    }

    if (ixEthDBPortDefinitions[portID].type != IX_ETH_NPE)
    {
        /* not an NPE */
        return;
    }

    portInfo = &ixEthDBPortInfo[portID];
    
    ixOsalMutexUnlock(&portInfo->npeAckLock);

    if (ixEthHssAccCoexistEnable)
    {
        /* Clear the common mutex locked during sending NPE messages */
        IX_ETH_HSS_COM_MUT_UNLOCK();
    }
}

/**
 * @brief synchronizes the database with tree
 *
 * @param portID port ID of the NPE whose tree is to be scanned
 * @param eltBaseAddress memory base address of the NPE serialized tree
 * @param eltSize size in bytes of the NPE serialized tree
 *
 * Scans the NPE learning tree and resets the age of active database records.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPESyncScan(IxEthDBPortId portID, void *eltBaseAddress, UINT32 eltSize)
{
    UINT32 eltEntryOffset;
    UINT32 entryPortID;

    /* invalidate cache */
    IX_OSAL_CACHE_INVALIDATE(eltBaseAddress, eltSize);

    for (eltEntryOffset = ELT_ROOT_OFFSET ; eltEntryOffset < eltSize ; eltEntryOffset += ELT_ENTRY_SIZE)
    {
        /* (eltBaseAddress + eltEntryOffset) points to a valid NPE tree node
         *
         * the format of the node is MAC[6 bytes]:PortID[1 byte]:Reserved[6 bits]:Active[1 bit]:Valid[1 bit]
         * therefore we can just use the pointer for database searches as only the first 6 bytes are checked
         */
        void *eltNodeAddress       = (void *) ((UINT32) eltBaseAddress + eltEntryOffset);

        /* debug */
        IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) checking node at offset %d...\n", eltEntryOffset / ELT_ENTRY_SIZE);

        if (IX_EDB_NPE_NODE_VALID(eltNodeAddress) != TRUE)
        {
            IX_ETH_DB_NPE_VERBOSE_TRACE("\t... node is empty\n");
        }
        else if (eltEntryOffset == ELT_ROOT_OFFSET)
        {
            IX_ETH_DB_NPE_VERBOSE_TRACE("\t... node is root\n");
        }

        if (IX_EDB_NPE_NODE_VALID(eltNodeAddress))
        {
            entryPortID = IX_ETHNPE_LOGICAL_ID_TO_PHYSICAL_ID(IX_EDB_NPE_NODE_PORT_ID(eltNodeAddress));

            /* check only active entries belonging to this port */
            if (ixEthDBPortInfo[portID].agingEnabled && IX_EDB_NPE_NODE_ACTIVE(eltNodeAddress) && (portID == entryPortID))
            {
                /* search record */
                HashNode *node = ixEthDBSearch((IxEthDBMacAddr *) eltNodeAddress, IX_ETH_DB_ALL_FILTERING_RECORDS);

                /* safety check, maybe user deleted record right before sync? */
                if (node != NULL)
                {
                    /* found record */
                    MacDescriptor *descriptor = (MacDescriptor *) node->data;

                    IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) synced entry [%s] already in the database, updating fields\n", mac2string(eltNodeAddress));

                    /* reset age - set to -1 so that maintenance will restore it to 0 (or more) when incrementing */
                    if (!descriptor->recordData.filteringData.staticEntry)
                    {
                        if (descriptor->type == IX_ETH_DB_FILTERING_RECORD)
                        {
                            descriptor->recordData.filteringData.age = AGE_RESET;
                        }
                        else if (descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD)
                        {
                            descriptor->recordData.filteringVlanData.age = AGE_RESET;
                        }
                    }

                    /* end transaction */
                    ixEthDBReleaseHashNode(node);
                }
            }
            else
            {
                IX_ETH_DB_NPE_VERBOSE_TRACE("\t... found portID %d, we check only port %d\n", entryPortID, portID);
            }
        }
    }
}

/**
 * @brief writes a search tree in NPE format
 *
 * @param type type of records to be written into the NPE update zone
 * @param totalSize maximum size of the linearized tree
 * @param baseAddress memory base address where to write the NPE tree into
 * @param tree search tree to write in NPE format
 * @param blocks number of written 64-byte blocks
 * @param startIndex optimal binary search start index
 *
 * Serializes the given tree in NPE linear format
 *
 * @return none
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPETreeWrite(IxEthDBRecordType type, UINT32 totalSize, void *baseAddress, MacTreeNode *tree, UINT32 *epDelta, UINT32 *blocks)
{
    MacTreeNodeStack *stack;
    UINT32 currentSize = 0;
    UINT32 offset = 0;
    UINT32 maxOffset = 0;

    stack = ixOsalCacheDmaMalloc(sizeof (MacTreeNodeStack));
    
    if (stack == NULL)
    {
        ERROR_LOG("DB: (NPEAdaptor) failed to allocate the node stack for learning tree linearization, out of memory?\n");
        return;
    }

    /* zero out empty root */
    currentSize = ixEthDBNPENodeWrite[type](NULL, baseAddress, 0, totalSize);

    NODE_STACK_INIT(stack);

    if (tree != NULL)
    {
        /* push tree root at offset 1 */
        NODE_STACK_PUSH(stack, tree, 1);
    }

    while (NODE_STACK_NONEMPTY(stack))
    {
        MacTreeNode *node;

        NODE_STACK_POP(stack, node, offset);

        /* Add node to NPE table at position indicated by offset */
        /* In the case where a node is NULL, the NodeWrite function will zero out 
         * the memory based on the offset. 
         * In the case where the given offset goes past totalSize, no entry is written
         */
        currentSize = ixEthDBNPENodeWrite[type](node, baseAddress, offset, totalSize);
        if ((node != NULL) && (currentSize <= totalSize))
        {
            IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) writing MAC [%s] at offset %d\n", mac2string(node->descriptor->macAddress), offset);

            if(maxOffset<offset)
                maxOffset = offset;

            NODE_STACK_PUSH(stack, node->left, LEFT_CHILD_OFFSET(offset));
            NODE_STACK_PUSH(stack, node->right, RIGHT_CHILD_OFFSET(offset));
        }
    }

    /* this extra write is done to determine if any more empty entries in the table need
     * to be cleared.
     */
    currentSize = ixEthDBNPENodeWrite[type](NULL, baseAddress, maxOffset+1, totalSize);
    if (currentSize < totalSize)
    {
        /* zero out rest of the tree */
        IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Clearing rest of tree\n");
        /* zero out the rest of the table */
        ixOsalMemSet((void *) (((UINT32) baseAddress) + currentSize), 0, (totalSize - currentSize));
    } 
    else
      {
	currentSize = totalSize ;
      }

    
    /* flush cache */
    IX_OSAL_CACHE_FLUSH(baseAddress, totalSize);

    IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Ethernet learning/filtering tree XScale wrote at address 0x%08X (max %d bytes):\n\n",
        (UINT32) baseAddress, totalSize);

    /* compute number of 64-byte blocks */
    if (blocks != NULL)
    {
        *blocks = currentSize ? 1 + ((currentSize - 1) / 64) : 0;

        IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Wrote %d 64-byte blocks\n", *blocks);
    }
    
    /* compute epDelta - start index for binary search */
    if (epDelta != NULL)
    {
        UINT32 deltaIndex = 0;

        *epDelta = 0;
        
        for (; deltaIndex < IX_ETH_DB_MAX_DELTA_ZONES ; deltaIndex ++)
        {
            if (ixEthDBEPDeltaOffset[type][deltaIndex] >= maxOffset)
            {
                *epDelta = ixEthDBEPDelta[type][deltaIndex];
                break;
            }
        }

        IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Computed epDelta %d (based on maxOffset %d)\n", *epDelta, maxOffset);
    }

    ixOsalCacheDmaFree(stack);
}

/**
 * @brief implements a dummy node serialization function
 *
 * @param address address of where the node is to be serialized (unused)
 * @param node tree node to be serialized (unused)
 *
 * This function is registered for safety reasons and should
 * never be called. It will display an error message if this
 * function is called.
 *
 * @return none
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
UINT32 ixEthDBNullSerialize(MacTreeNode *node, void *address, UINT32 offset, UINT32 maxSize)
{
    IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Warning, the NullSerialize function was called, wrong record type?\n");
    return 0;
}

/**
 * @brief writes a filtering entry in NPE linear format
 *
 * @param node: node to be written
 * @param baseAddress: base memory address to write node to
 * @param entryOffset: number of entries to offset
 * @param maxByteSize: maximum size of memory area to write to
 *
 * @return the byte offset of the next entry
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
UINT32 ixEthDBNPELearningNodeWrite(MacTreeNode *node, void *baseAddress, 
                                 UINT32 entryOffset, UINT32 maxByteSize)
{
    void *address = (void*) ((UINT32)baseAddress + (entryOffset * ELT_ENTRY_SIZE));
    UINT32 byteSize = (entryOffset+1) * ELT_ENTRY_SIZE;

    /* if the size is greater than max, do nothing and return the max */
    if(byteSize > maxByteSize)
        return byteSize;

    /* If node is NULL, zero out this record in memory */
    if(NULL == node)
    {
        ixOsalMemSet(address, 0, ELT_ENTRY_SIZE);
        return byteSize;
    }

    /* copy mac address */
    ixOsalMemCopy(address, node->descriptor->macAddress, IX_IEEE803_MAC_ADDRESS_SIZE);

    /* copy port ID */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_ELT_PORT_ID_OFFSET) = IX_ETHNPE_PHYSICAL_ID_TO_LOGICAL_ID(node->descriptor->portID);

    /* copy flags (valid and not active, as the NPE sets it to active) and clear reserved section (bits 2-7) */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_ELT_FLAGS_OFFSET) = (UINT8) IX_EDB_FLAGS_INACTIVE_VALID;

    IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) writing ELT node 0x%08x:0x%08x\n", * (UINT32 *) address, * (((UINT32 *) (address)) + 1));
    return byteSize;
}

/**
 * @brief writes a WiFi header conversion record in
 * NPE linear format
 *
 * @param node: node to be written
 * @param baseAddress: base memory address to write node to
 * @param entryOffset: number of entries to offset
 * @param maxByteSize: maximum size of memory area to write to
 *
 * @return the byte offset of the next entry
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
UINT32 ixEthDBNPEWiFiNodeWrite(MacTreeNode *node, void *baseAddress, 
                                 UINT32 entryOffset, UINT32 maxByteSize)
{
    void *address = (void*) ((UINT32)baseAddress + (entryOffset * ELT_ENTRY_SIZE));
    UINT32 byteSize = (entryOffset+1) * ELT_ENTRY_SIZE;

    /* if the size is greater than max, do nothing and return the max */
    if(byteSize > maxByteSize)
        return byteSize;

    /* If node is NULL, zero out this record in memory */
    if(NULL == node)
    {
        ixOsalMemSet(address, 0, ELT_ENTRY_SIZE);
        return byteSize;
    }

    /* copy mac address */
    ixOsalMemCopy(address, node->descriptor->macAddress, IX_IEEE803_MAC_ADDRESS_SIZE);

    /* copy index */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_WIFI_INDEX_OFFSET) = node->descriptor->recordData.wifiData.recIndex;

    /* copy flags (type and valid) */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_WIFI_FLAGS_OFFSET) = node->descriptor->recordData.wifiData.padLength << 4 
								| node->descriptor->recordData.wifiData.recType << 1 
								| IX_EDB_FLAGS_VALID;
    return byteSize;
}

/**
 * @brief writes a WiFi gateway header conversion record in
 * NPE linear format
 *
 * @param node: node to be written
 * @param baseAddress: base memory address to write node to
 * @param entryOffset: number of entries to offset
 * @param maxByteSize: maximum size of memory area to write to
 *
 * @return the byte offset of the next entry
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
UINT32 ixEthDBNPEGatewayNodeWrite(MacTreeNode *node, void *baseAddress, 
                                 UINT32 entryOffset, UINT32 maxByteSize)
{
    void *address = (void*) ((UINT32)baseAddress + (entryOffset * ELT_ENTRY_SIZE));
    UINT32 byteSize = (entryOffset+1) * ELT_ENTRY_SIZE;

    /* if the size is greater than max, do nothing and return the max */
    if(byteSize > maxByteSize)
        return byteSize;

    /* If node is NULL, zero out this record in memory */
    if(NULL == node)
    {
        ixOsalMemSet(address, 0, ELT_ENTRY_SIZE);
        return byteSize;
    }

    /* copy mac address */
    ixOsalMemCopy(address, node->descriptor->recordData.wifiData.gwMacAddress, IX_IEEE803_MAC_ADDRESS_SIZE);

    /* set reserved field, two bytes */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_GW_RESERVED_OFFSET)     = 0;
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_GW_RESERVED_OFFSET + 1) = 0;
    return byteSize;
}

/**
 * @brief writes a WiFi bssid header conversion record in
 * NPE linear format
 *
 * @param node: node to be written
 * @param baseAddress: base memory address to write node to
 * @param entryOffset: number of entries to offset
 * @param maxByteSize: maximum size of memory area to write to
 *
 * @return the byte offset of the next entry
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
UINT32 ixEthDBNPEBssidNodeWrite(MacTreeNode *node, void *baseAddress, 
                                 UINT32 entryOffset, UINT32 maxByteSize)
{
    void *address = (void*) ((UINT32)baseAddress + (entryOffset * ELT_ENTRY_SIZE));
    UINT32 byteSize = (entryOffset+1) * ELT_ENTRY_SIZE;

    /* if the size is greater than max, do nothing and return the max */
    if(byteSize > maxByteSize)
        return byteSize;

    /* If node is NULL, zero out this record in memory */
    if(NULL == node)
    {
        ixOsalMemSet(address, 0, ELT_ENTRY_SIZE);
        return byteSize;
    }

    /* copy mac address */
    ixOsalMemCopy(address, node->descriptor->recordData.wifiData.bssid, IX_IEEE803_MAC_ADDRESS_SIZE);

    /* set destination port and reserved fields */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_BSSID_DEST_PORT_OFFSET) = node->descriptor->recordData.wifiData.logicalPortID;
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_BSSID_RESERVED_OFFSET) = 0;
    return byteSize;
}

/**
 * @brief writes a masked firewall record in
 * NPE linear format
 *
 * @param node: node to be written
 * @param baseAddress: base memory address to write node to
 * @param entryOffset: number of entries to offset
 * @param maxByteSize: maximum size of memory area to write to
 *
 * @return the byte offset of the next entry
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
UINT32 ixEthDBNPEFirewallMaskedNodeWrite(MacTreeNode *node, void *baseAddress, 
                                 UINT32 entryOffset, UINT32 maxByteSize)
{
    void *address = (void*) ((UINT32)baseAddress + (entryOffset * FW_M_ENTRY_SIZE));
    UINT32 byteSize = (entryOffset+1) * FW_M_ENTRY_SIZE;

    /* if the size is greater than max, do nothing and return the max */
    if(byteSize > maxByteSize)
        return byteSize;

    /* If node is NULL, zero out this record in memory */
    if(NULL == node)
    {
        ixOsalMemSet(address, 0, FW_M_ENTRY_SIZE);
        return byteSize;
    }

    /* set reserved fields */
    NPE_NODE_BYTE(address, 0) = 0;
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_FW_RESERVED_OFFSET) = 0;
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_FW_RESERVED_OFFSET + 1) = 0;

    /* set flags */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_FW_FLAGS_OFFSET) = IX_EDB_FLAGS_VALID;

    /* copy mac address */
    ixOsalMemCopy((void *) ((UINT32) address + IX_EDB_NPE_NODE_FW_ADDR_OFFSET), node->descriptor->macAddress, IX_IEEE803_MAC_ADDRESS_SIZE);
    /* copy address mask */
    ixOsalMemCopy((void *) ((UINT32) address + IX_EDB_NPE_NODE_FW_MASK_OFFSET), node->descriptor->recordData.firewallData.addressMask, IX_IEEE803_MAC_ADDRESS_SIZE);
    return byteSize;
}

/**
 * @brief writes a firewall record in
 * NPE linear format
 *
 * @param node: node to be written
 * @param baseAddress: base memory address to write node to
 * @param entryOffset: number of entries to offset
 * @param maxByteSize: maximum size of memory area to write to
 *
 * @return the byte offset of the next entry
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
UINT32 ixEthDBNPEFirewallNodeWrite(MacTreeNode *node, void *baseAddress, 
                                 UINT32 entryOffset, UINT32 maxByteSize)
{
    void *address = (void*) ((UINT32)baseAddress + (entryOffset * FW_ENTRY_SIZE));
    UINT32 byteSize = (entryOffset+1) * FW_ENTRY_SIZE;

    /* if the size is greater than max, do nothing and return the max */
    if(byteSize > maxByteSize)
        return byteSize;

    /* If node is NULL, zero out this record in memory */
    if(NULL == node)
    {
        ixOsalMemSet(address, 0, FW_ENTRY_SIZE);
        return byteSize;
    }

    /* set reserved field */
    NPE_NODE_BYTE(address, 0) = 0;

    /* set flags */
    NPE_NODE_BYTE(address, IX_EDB_NPE_NODE_FW_FLAGS_OFFSET) = IX_EDB_FLAGS_VALID;

    /* copy mac address */
    ixOsalMemCopy((void *) ((UINT32) address + IX_EDB_NPE_NODE_FW_ADDR_OFFSET), node->descriptor->macAddress, IX_IEEE803_MAC_ADDRESS_SIZE);
    return byteSize;
}

/**
 * @brief registers the NPE serialization methods
 *
 * This functions registers NPE serialization methods
 * for writing the following types of records in NPE
 * readable linear format:
 * - filtering records
 * - WiFi header conversion records
 * - WiFi gateway header conversion records
 * - firewall records
 *
 * Note that this function should be called by the
 * component initialization function.
 *
 * @return number of registered record types
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
UINT32 ixEthDBRecordSerializeMethodsRegister()
{
    int i;
    
    /* safety - register a blank method for everybody first */
    for ( i = 0 ; i < IX_ETH_DB_MAX_RECORD_TYPE_INDEX + 1 ; i++)
    {
        ixEthDBNPENodeWrite[i] = ixEthDBNullSerialize;
    }
    
    /* register real methods */
    ixEthDBNPENodeWrite[IX_ETH_DB_FILTERING_RECORD]      = ixEthDBNPELearningNodeWrite;
    ixEthDBNPENodeWrite[IX_ETH_DB_FILTERING_VLAN_RECORD] = ixEthDBNPELearningNodeWrite;
    ixEthDBNPENodeWrite[IX_ETH_DB_WIFI_RECORD]           = ixEthDBNPEWiFiNodeWrite;
    ixEthDBNPENodeWrite[IX_ETH_DB_FIREWALL_RECORD]       = ixEthDBNPEFirewallNodeWrite;
    ixEthDBNPENodeWrite[IX_ETH_DB_MASKED_FIREWALL_RECORD]= ixEthDBNPEFirewallMaskedNodeWrite;
    ixEthDBNPENodeWrite[IX_ETH_DB_GATEWAY_RECORD]        = ixEthDBNPEGatewayNodeWrite;
    
    /* EP Delta arrays */
    ixOsalMemSet(ixEthDBEPDeltaOffset, 0, sizeof (ixEthDBEPDeltaOffset));
    ixOsalMemSet(ixEthDBEPDelta, 0, sizeof (ixEthDBEPDelta));
    
    /* filtering records */
    ixEthDBEPDeltaOffset[IX_ETH_DB_FILTERING_RECORD][0] = 1;
    ixEthDBEPDelta[IX_ETH_DB_FILTERING_RECORD][0]       = 0;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_FILTERING_RECORD][1] = 3;
    ixEthDBEPDelta[IX_ETH_DB_FILTERING_RECORD][1]       = 7;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_FILTERING_RECORD][2] = 511;
    ixEthDBEPDelta[IX_ETH_DB_FILTERING_RECORD][2]       = 14;
    
    /* wifi records */
    ixEthDBEPDeltaOffset[IX_ETH_DB_WIFI_RECORD][0] = 1;
    ixEthDBEPDelta[IX_ETH_DB_WIFI_RECORD][0]       = 0;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_WIFI_RECORD][1] = 3;
    ixEthDBEPDelta[IX_ETH_DB_WIFI_RECORD][1]       = 7;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_WIFI_RECORD][2] = 511;
    ixEthDBEPDelta[IX_ETH_DB_WIFI_RECORD][2]       = 14;

    /* firewall records */
    ixEthDBEPDeltaOffset[IX_ETH_DB_FIREWALL_RECORD][0] = 0;
    ixEthDBEPDelta[IX_ETH_DB_FIREWALL_RECORD][0]       = 0;

    ixEthDBEPDeltaOffset[IX_ETH_DB_FIREWALL_RECORD][1] = 1;
    ixEthDBEPDelta[IX_ETH_DB_FIREWALL_RECORD][1]       = 5;

    ixEthDBEPDeltaOffset[IX_ETH_DB_FIREWALL_RECORD][2] = 3;
    ixEthDBEPDelta[IX_ETH_DB_FIREWALL_RECORD][2]       = 13;

    ixEthDBEPDeltaOffset[IX_ETH_DB_FIREWALL_RECORD][3] = 7;
    ixEthDBEPDelta[IX_ETH_DB_FIREWALL_RECORD][3]       = 21;

    ixEthDBEPDeltaOffset[IX_ETH_DB_FIREWALL_RECORD][4] = 15;
    ixEthDBEPDelta[IX_ETH_DB_FIREWALL_RECORD][4]       = 29;

    ixEthDBEPDeltaOffset[IX_ETH_DB_FIREWALL_RECORD][5] = 31;
    ixEthDBEPDelta[IX_ETH_DB_FIREWALL_RECORD][5]       = 37;

    /* masked firewall records */
    ixEthDBEPDeltaOffset[IX_ETH_DB_MASKED_FIREWALL_RECORD][0] = 0;
    ixEthDBEPDelta[IX_ETH_DB_MASKED_FIREWALL_RECORD][0]       = 0;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_MASKED_FIREWALL_RECORD][1] = 1;
    ixEthDBEPDelta[IX_ETH_DB_MASKED_FIREWALL_RECORD][1]       = 9;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_MASKED_FIREWALL_RECORD][2] = 3;
    ixEthDBEPDelta[IX_ETH_DB_MASKED_FIREWALL_RECORD][2]       = 23;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_MASKED_FIREWALL_RECORD][3] = 7;
    ixEthDBEPDelta[IX_ETH_DB_MASKED_FIREWALL_RECORD][3]       = 37;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_MASKED_FIREWALL_RECORD][4] = 15;
    ixEthDBEPDelta[IX_ETH_DB_MASKED_FIREWALL_RECORD][4]       = 51;
    
    ixEthDBEPDeltaOffset[IX_ETH_DB_MASKED_FIREWALL_RECORD][5] = 31;
    ixEthDBEPDelta[IX_ETH_DB_MASKED_FIREWALL_RECORD][5]       = 65;
    
    return 6; /* 6 methods registered */
}

#ifndef IX_NDEBUG

IX_ETH_DB_PUBLIC UINT32 npeMsgHistory[IX_ETH_DB_NPE_MSG_HISTORY_DEPTH][2];
IX_ETH_DB_PUBLIC UINT32 npeMsgHistoryLen = 0;

/**
 * When compiled in DEBUG mode, this function can be used to display
 * the history of messages sent to the NPEs (up to 100).
 */
IX_ETH_DB_PUBLIC
void ixEthDBShowNpeMsgHistory()
{
    UINT32 i = 0;
    UINT32 base, len;

    if (npeMsgHistoryLen <= IX_ETH_DB_NPE_MSG_HISTORY_DEPTH)
    {
        base = 0;
        len  = npeMsgHistoryLen;
    }
    else
    {
        base = npeMsgHistoryLen % IX_ETH_DB_NPE_MSG_HISTORY_DEPTH;
        len  = IX_ETH_DB_NPE_MSG_HISTORY_DEPTH;
    }

    printf("NPE message history [last %d messages, from least to most recent]:\n", len);

    for (; i < len ; i++)
    {
        UINT32 pos = (base + i) % IX_ETH_DB_NPE_MSG_HISTORY_DEPTH;
        printf("msg[%d]: 0x%08x:0x%08x\n", i, npeMsgHistory[pos][0], npeMsgHistory[pos][1]);
    }
}

IX_ETH_DB_PUBLIC
void ixEthDBELTShow(IxEthDBPortId portID)
{
    IxNpeMhMessage message;
    IX_STATUS result;
    
    /* send EDB_GetMACAddressDatabase message */
    FILL_GETMACADDRESSDATABASE(message, 
        0 /* reserved */, 
        IX_OSAL_MMU_VIRT_TO_PHYS(ixEthDBPortInfo[portID].updateMethod.npeUpdateZone));

    IX_ETHDB_SEND_NPE_MSG(IX_ETHNPE_PHYSICAL_ID_TO_NODE(portID), message, result);

    if (result == IX_SUCCESS)
    {
        /* analyze NPE copy */
        UINT32 eltEntryOffset;
        UINT32 entryPortID;

        UINT32 eltBaseAddress = (UINT32) ixEthDBPortInfo[portID].updateMethod.npeUpdateZone;
        UINT32 eltSize        = FULL_ELT_BYTE_SIZE;

        /* invalidate cache */
        IX_OSAL_CACHE_INVALIDATE((void *) eltBaseAddress, eltSize);

        printf("Listing records in main learning tree for port %d\n", portID);

        for (eltEntryOffset = ELT_ROOT_OFFSET ; eltEntryOffset < eltSize ; eltEntryOffset += ELT_ENTRY_SIZE)
        {
            /* (eltBaseAddress + eltEntryOffset) points to a valid NPE tree node
            *
            * the format of the node is MAC[6 bytes]:PortID[1 byte]:Reserved[6 bits]:Active[1 bit]:Valid[1 bit]
            * therefore we can just use the pointer for database searches as only the first 6 bytes are checked
            */
            void *eltNodeAddress = (void *) ((UINT32) eltBaseAddress + eltEntryOffset);

            if (IX_EDB_NPE_NODE_VALID(eltNodeAddress))
            {
                HashNode *node;

                entryPortID = IX_ETHNPE_LOGICAL_ID_TO_PHYSICAL_ID(IX_EDB_NPE_NODE_PORT_ID(eltNodeAddress));

                /* search record */
                node = ixEthDBSearch((IxEthDBMacAddr *) eltNodeAddress, IX_ETH_DB_ALL_RECORD_TYPES);

                printf("%s - port %d - %s ", mac2string((unsigned char *) eltNodeAddress), entryPortID, 
                    IX_EDB_NPE_NODE_ACTIVE(eltNodeAddress) ? "active" : "inactive");

                /* safety check, maybe user deleted record right before sync? */
                if (node != NULL)
                {
                    /* found record */
                    MacDescriptor *descriptor = (MacDescriptor *) node->data;

                    printf("- %s ",
                        descriptor->type == IX_ETH_DB_FILTERING_RECORD ? "filtering" :
                        descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD ? "vlan" :
                        descriptor->type == IX_ETH_DB_WIFI_RECORD ? "wifi" : "other (check main DB)");

                    if (descriptor->type == IX_ETH_DB_FILTERING_RECORD) printf("- age %d - %s ", 
                        descriptor->recordData.filteringData.age,
                        descriptor->recordData.filteringData.staticEntry ? "static" : "dynamic");

                    if (descriptor->type == IX_ETH_DB_FILTERING_VLAN_RECORD) printf("- age %d - %s - tci %d ",
                        descriptor->recordData.filteringVlanData.age,
                        descriptor->recordData.filteringVlanData.staticEntry ? "static" : "dynamic",
                        descriptor->recordData.filteringVlanData.ieee802_1qTag);

                    /* end transaction */
                    ixEthDBReleaseHashNode(node);
                }
                else
                {
                    printf("- not synced");
                }

                printf("\n");
            }
        }
    }
    else
    {
        ixOsalLog(IX_OSAL_LOG_LVL_FATAL, IX_OSAL_LOG_DEV_STDOUT, 
            "EthDB: (ShowELT) Could not complete action (communication failure)\n",
            portID, 0, 0, 0, 0, 0);
    }
}

#endif
