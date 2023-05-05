/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sfPolicy.h"
#include "snort_debug.h"
#include "sfrt.h"

tSfPolicyId napRuntimePolicyId = 0;
tSfPolicyId ipsRuntimePolicyId = 0;

static inline int IsBound (
    tSfPolicyId id
    )
{
    return ( id != SF_POLICY_UNBOUND );
}

static inline int NotBound (
    tSfPolicyId id
    )
{
    return !IsBound(id);
}

static void netBindFree(
        void *policy,
        void *config
        );

tSfPolicyConfig * sfPolicyInit(void)
{
    int i;
    tSfPolicyConfig *new = (tSfPolicyConfig *)calloc(1, sizeof(tSfPolicyConfig));

    if (new == NULL)
        return NULL;

    //initialize vlan bindings
    for (i = 0; i < SF_VLAN_BINDING_MAX; i++)
    {
        new->vlanBindings[i] = SF_POLICY_UNBOUND;
    }

    for (i = 0; i < SF_POLICY_ID_BINDING_MAX; i++)
    {
        new->policyIdBindings[i] = SF_POLICY_UNBOUND;
    }

    //initialize net bindings
    new->netBindTable = sfrt_new(DIR_16x7_4x4, IPv6, SF_NETWORK_BINDING_MAX, 20);

    return new;
}

void sfPolicyFini(tSfPolicyConfig *config)
{
    int i;

    if (config == NULL)
        return;

    for (i = 0; i < SF_VLAN_BINDING_MAX; i++)
    {
        sfVlanDeleteBinding(config, i);
    }

    for (i = 0; i < SF_POLICY_ID_BINDING_MAX; i++)
    {
        sfPolicyIdDeleteBinding(config, i);
    }

    sfrt_cleanup2(config->netBindTable, netBindFree, config);
    sfrt_free(config->netBindTable);

    //policyConfig are deleted when all bindings to it are deleted.

    /* free default policy */

    if (config->ppPolicies != NULL)
    {
        sfPolicyDelete(config, config->defaultPolicyId);
        free(config->ppPolicies);
    }

    free(config);
}

static void netBindFree(
        void *policyId,
        void *config
        )
{
    if (policyId && config)
    {
        sfPolicyDelete((tSfPolicyConfig *)config, *((tSfPolicyId *)policyId));
        free(policyId);
    }
}

/**Tracks filename to vlan group id */
int sfPolicyAdd(tSfPolicyConfig *config, char *fileName)
{
    tSfPolicy *pObject = NULL;
    int emptyIndex = -1;
    tSfPolicyId i;
    tSfPolicy **ppTmp;

    if (config == NULL)
        return SF_POLICY_UNBOUND;

    for (i = 0; i < config->numAllocatedPolicies; i++)
    {
        if (config->ppPolicies[i])
        {
            if (!strcmp(config->ppPolicies[i]->filename, fileName))
            {
                config->ppPolicies[i]->refCount++;
                return i;
            }
        }
        else if (emptyIndex == -1)
        {
            emptyIndex = i;
        }
    }

    if (emptyIndex == -1)
    {
        //no empty slot available. Allocate more space for policies
        ppTmp = (tSfPolicy **)calloc(config->numAllocatedPolicies + POLICY_ALLOCATION_CHUNK,
                                     sizeof(tSfPolicy *));
        if (!ppTmp)
            return SF_POLICY_UNBOUND;

        if (config->numAllocatedPolicies)
        {
            memcpy(ppTmp, config->ppPolicies,
                   sizeof(tSfPolicyConfig *) * config->numAllocatedPolicies);
            free(config->ppPolicies);
        }

        config->ppPolicies = ppTmp;
        emptyIndex = config->numAllocatedPolicies;
        config->numAllocatedPolicies += POLICY_ALLOCATION_CHUNK;
    }

    //allocate and initialize
    pObject = (tSfPolicy *)calloc(1, sizeof(tSfPolicy));
    if (!pObject)
        return SF_POLICY_UNBOUND;

    pObject->refCount++;
    pObject->filename = strdup(fileName);
    if (!pObject->filename)
    {
        free(pObject);
        return SF_POLICY_UNBOUND;
    }

    config->ppPolicies[emptyIndex] = pObject;
    config->numActivePolicies++;

    //successfully added.
    return emptyIndex;
}

void sfPolicyDelete(tSfPolicyConfig *config, tSfPolicyId policyId)
{
    tSfPolicy *pObject = NULL;

    if ((config == NULL) || (config->ppPolicies == NULL) ||
        (policyId >= config->numAllocatedPolicies))
    {
        return;
    }

    pObject = config->ppPolicies[policyId];

    if (pObject)
    {
        pObject->refCount--;
        if (pObject->refCount == 0)
        {
            if (pObject->filename)
                free(pObject->filename);
            free(pObject);
            config->ppPolicies[policyId] = NULL;
            config->numActivePolicies--;
            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                                    "sfPolicyDelete: freed policyConfig policyId %d\n", policyId););
        }
    }
}

char * sfPolicyGet(tSfPolicyConfig *config, tSfPolicyId policyId)
{
    tSfPolicy *pObject = NULL;

    if ((config == NULL) || (config->ppPolicies == NULL) ||
        (policyId >= config->numAllocatedPolicies))
    {
        return NULL;
    }

    pObject = config->ppPolicies[policyId];
    if (pObject)
        return pObject->filename;

    return NULL;
}



/**Creates policyId if needed and creates a binding between vlan and policyId.
 * Tracks vlanId  to vlan group id mapping
 */
//TBD replace calloc with SnortAlloc()
int sfVlanAddBinding(tSfPolicyConfig *config, int vlanId, char *fileName)
{
    tSfPolicyId policyId;

    if (config == NULL || vlanId >= SF_VLAN_BINDING_MAX)
        return -1;

    //create a policyId
    policyId = sfPolicyAdd(config, fileName);

    if ( NotBound(policyId) )
    {
        return -1;
    }

    config->vlanBindings[vlanId] = policyId;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                            "Added  vlandId  %d, file %s, policyId: %d\n", vlanId, fileName, policyId););
    return 0;
}

tSfPolicyId sfVlanGetBinding(tSfPolicyConfig *config, int vlanId)
{
    tSfPolicyId policyId;

    if(vlanId >= SF_VLAN_BINDING_MAX){
        //invalid policyid will never be bound. return default
        return config->defaultPolicyId;
    }

    policyId = config->vlanBindings[vlanId];

    if ( NotBound(policyId) )
    {
        //return default policyId for uninitialized binding
        return config->defaultPolicyId;
    }

    return policyId;
}

void sfVlanDeleteBinding(tSfPolicyConfig *config, int vlanId)
{
    tSfPolicyId policyId;
    if(vlanId >= SF_VLAN_BINDING_MAX)
        return; //invalid, can't delete

    if ((config == NULL) || (vlanId < 0))
        return;

    policyId = config->vlanBindings[vlanId];

    if ( IsBound(policyId) )
    {
        sfPolicyDelete(config, policyId);
        config->vlanBindings[vlanId] = SF_POLICY_UNBOUND;
    }
}

int sfPolicyIdAddBinding(tSfPolicyConfig *config, int parsedPolicyId, char *fileName)
{
    tSfPolicyId policyId;

    if (config == NULL || parsedPolicyId >= SF_POLICY_ID_BINDING_MAX)
        return -1;

    //create a policyId
    policyId = sfPolicyAdd(config, fileName);

    if ( NotBound(policyId) )
    {
        return -1;
    }

    config->policyIdBindings[parsedPolicyId] = policyId;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                            "Added  parsedPolicyId  %d, file %s, policyId: %d\n", parsedPolicyId, fileName, policyId););
    return 0;
}

tSfPolicyId sfPolicyIdGetBinding(tSfPolicyConfig *config, int parsedPolicyId)
{
    tSfPolicyId policyId;

    if(parsedPolicyId >= SF_POLICY_ID_BINDING_MAX){
        //invalid policyid will never be bound. return default
        return config->defaultPolicyId;
    }
    policyId = config->policyIdBindings[parsedPolicyId];

    if ( NotBound(policyId) )
    {
        //return default policyId for uninitialized binding
        return config->defaultPolicyId;
    }

    return policyId;
}

void sfPolicyIdDeleteBinding(tSfPolicyConfig *config, int parsedPolicyId)
{
    tSfPolicyId policyId;

    if(parsedPolicyId >= SF_POLICY_ID_BINDING_MAX)
        return; //invalid, can't delete

    if ((config == NULL) || (parsedPolicyId < 0))
        return;

    policyId = config->policyIdBindings[parsedPolicyId];

    if ( IsBound(policyId) )
    {
        sfPolicyDelete(config, policyId);
        config->policyIdBindings[parsedPolicyId] = SF_POLICY_UNBOUND;
    }
}

/**Get applicable policy given <vlan, srcIp, dstIp> of a packet. Vlan can be negative
 * number if vlan header is not present.
 *
 * Search policy bound to vlan if vlan is not negative. If matched polciy is default one,
 * then search using destination IP address. If matched policy is default then search using
 * source IP address.
 *
 * @param vlanId - vlan id from a packet. Should be unbound if vlan tag is not present.
 * @param srcIP - Source IP address
 * @param dstIP - Destination IP address
 *
 * @returns policyId
 */
tSfPolicyId sfGetApplicablePolicyId(
        tSfPolicyConfig *config,
        int vlanId,
        sfaddr_t* srcIp,
        sfaddr_t* dstIp
        )
{
    tSfPolicyId dst_id;

    if (config == NULL)
        return SF_POLICY_UNBOUND;

    if (vlanId > 0)
    {
        tSfPolicyId vlan_id = sfVlanGetBinding(config, vlanId);
        if (vlan_id > 0)
            return vlan_id;
    }

    dst_id = sfNetworkGetBinding(config, dstIp);
    return (dst_id > 0 ? dst_id : sfNetworkGetBinding(config, srcIp));
}

/**Add network binding to a policy
 * @param Ip - IP address or CIDR block to policy identified by file.
 * @param fileName - name of configuration file
 *
 * @returns 0 if successful, -1 otherwise.
 */
int sfNetworkAddBinding(
        tSfPolicyConfig *config,
        sfcidr_t* Ip,
        char *fileName
        )
{
    tSfPolicyId *policyId;
    int iRet;

    if ((config == NULL) || (Ip == NULL) || (fileName == NULL))
        return -1;

    if ((policyId = calloc(sizeof(tSfPolicyId), 1)) == NULL)
    {
        return -1;
    }

    //create a policyId
    *policyId = sfPolicyAdd(config, fileName);
    if ( NotBound(*policyId) )
    {
        free(policyId);
        return -1;
    }

    iRet = sfrt_insert(Ip, (unsigned char)Ip->bits,
                       (void *)policyId, RT_FAVOR_SPECIFIC, config->netBindTable);

    //DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Added  vlandId  %d, file %s, policyId: %d\n", vlanId, fileName, policyId););
    if (iRet)
    {
        free(policyId);
        return -1;
    }

    return 0;
}

unsigned int sfNetworkGetBinding(
        tSfPolicyConfig *config,
        sfaddr_t* ip
        )
{
    tSfPolicyId *policyId = NULL;

    if ((void *)ip == NULL)
        return config->defaultPolicyId;

    policyId = (tSfPolicyId *)sfrt_lookup(ip, config->netBindTable);

    if (!policyId)
    {
        return config->defaultPolicyId;
    }

    return *policyId;
}

void sfNetworkDeleteBinding(
        tSfPolicyConfig *config,
        sfaddr_t* ip
        )
{
    tSfPolicyId *policyId;

    if ((void *)ip == NULL)
        return;

    policyId = (tSfPolicyId *)sfrt_lookup(ip, config->netBindTable);

    if (!policyId)
        return;

    //TBD - delete function is not provided in sfrt.c
    sfPolicyDelete(config, *policyId);
}

//Move to sfDynArray.c/h
/**Dynamic array bound checks. If index is greater than maxElement then realloc like operation
 * is performed.
 * @param dynArray - dynamic array
 * @param index - 0 based. Index of element that will be accessed by application either as rvalue or lvalue.
 * @maxElements - Number of elements already allocated in dynArray. 0 value means no elements are allocated
 *     and therefore dynArray[0] will cause memory allocation.
 */
int sfDynArrayCheckBounds (
        void ** dynArray,
        unsigned int index,
        unsigned int *maxElements
        )
{
    void *ppTmp = NULL;

    if (index >= *maxElements)
    {
        //expand the array
        ppTmp = calloc(index+POLICY_ALLOCATION_CHUNK, sizeof(void *));
        if (!(ppTmp))
        {
            return -1;
        }

        if (*maxElements)
        {
            memcpy(ppTmp, *dynArray, sizeof(void*)*(*maxElements));
            free(*dynArray);
        }

        *dynArray = ppTmp;
        *maxElements = index + POLICY_ALLOCATION_CHUNK;
    }

    return 0;
}
