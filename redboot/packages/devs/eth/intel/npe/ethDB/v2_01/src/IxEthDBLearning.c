/**
 * @file IxEthDBLearning.c
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
 * @brief hashes the mac address in a mac descriptor with a XOR function
 *
 * @param entry pointer to a mac descriptor to be hashed
 * 
 * This function only extracts the mac address and employs ixEthDBKeyXORHash()
 * to do the actual hashing.
 * Used only to add a whole entry to a hash table, as opposed to searching which
 * takes only a key and uses the key hashing directly.
 *
 * @see ixEthDBKeyXORHash()
 *
 * @return the hash value
 *
 * @internal
 */
UINT32 ixEthDBEntryXORHash(void *entry)
{
    MacDescriptor *descriptor = (MacDescriptor *) entry;

    return ixEthDBKeyXORHash(descriptor->macAddress);
}

/**
 * @brief hashes a mac address
 *
 * @param key pointer to a 6 byte structure (typically an IxEthDBMacAddr pointer)
 * to be hashed
 *
 * Given a 6 bytes MAC address, the hash used is:
 *
 * hash(MAC[0:5]) = MAC[0:1] ^ MAC[2:3] ^ MAC[4:5]
 *
 * Used by the hash table to search and remove entries based
 * solely on their keys (mac addresses).
 *
 * @return the hash value
 *
 * @internal
 */
UINT32 ixEthDBKeyXORHash(void *key)
{
    UINT32 hashValue;
    UINT8 *value = (UINT8 *) key;
    
    hashValue  = (value[5] << 8) | value[4];
    hashValue ^= (value[3] << 8) | value[2];
    hashValue ^= (value[1] << 8) | value[0];

    return hashValue;
}

/**
 * @brief mac descriptor match function
 *
 * @param reference mac address (typically an IxEthDBMacAddr pointer) structure
 * @param entry pointer to a mac descriptor whose key (mac address) is to be 
 * matched against the reference key
 *
 * Used by the hash table to retrieve entries. Hashing entries can produce
 * collisions, i.e. descriptors with different mac addresses and the same
 * hash value, where this function is used to differentiate entries.
 *
 * @retval TRUE if the entry matches the reference key (equal addresses)
 * @retval FALSE if the entry does not match the reference key
 *
 * @internal
 */
BOOL ixEthDBAddressMatch(void *reference, void *entry)
{
    return (ixEthDBAddressCompare(reference, ((MacDescriptor *) entry)->macAddress) == 0);
}

/**
 * @brief compares two mac addresses
 *
 * @param mac1 first mac address to compare
 * @param mac2 second mac address to compare
 * 
 * This comparison works in a similar way to strcmp, producing similar results.
 * Used to insert values keyed on mac addresses into binary search trees.
 *
 * @retval -1 if mac1 < mac2
 * @retval 0 if ma1 == mac2
 * @retval 1 if mac1 > mac2
 */
UINT32 ixEthDBAddressCompare(UINT8 *mac1, UINT8 *mac2)
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

