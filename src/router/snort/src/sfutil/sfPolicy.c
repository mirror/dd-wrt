/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sfPolicy.h"
#include "debug.h"
#include "sfrt.h"


static INLINE int IsBound (
    tSfPolicyId id
    )
{
    return ( id != SF_VLAN_UNBOUND );
}

static INLINE int NotBound (
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
        new->vlanBindings[i] = SF_VLAN_UNBOUND;
    }

    //initialize net bindings
#ifdef SUP_IP6
    new->netBindTable = sfrt_new(DIR_16x7_4x4, IPv6, SF_NETWORK_BINDING_MAX, 20);
#else
    new->netBindTable = sfrt_new(DIR_16_4x4, IPv4, SF_NETWORK_BINDING_MAX, 20);
#endif

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
        return SF_VLAN_UNBOUND;

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
            return SF_VLAN_UNBOUND;

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
        return SF_VLAN_UNBOUND;

    pObject->refCount++;
    pObject->filename = strdup(fileName);
    if (!pObject->filename)
    {
        free(pObject);
        return SF_VLAN_UNBOUND;
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

    if (config == NULL)
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
    tSfPolicyId policyId = config->vlanBindings[vlanId];

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

    if ((config == NULL) || (vlanId < 0))
        return;

    policyId = config->vlanBindings[vlanId];

    if ( IsBound(policyId) )
    {
        sfPolicyDelete(config, policyId);
        config->vlanBindings[vlanId] = SF_VLAN_UNBOUND;
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
        snort_ip_p     srcIp,
        snort_ip_p     dstIp
        )
{
    tSfPolicyId dst_id;

    if (config == NULL)
        return SF_VLAN_UNBOUND;

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
        sfip_t* Ip, 
        char *fileName
        )
{
    tSfPolicyId *policyId;
    int iRet;
#ifdef SUP_IP6
    sfip_t tmp_ip;
#else
    uint32_t addr;
#endif

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

#ifdef SUP_IP6
    /* For IPv4, need to pass the address in host order */
    if (Ip->family == AF_INET)
    {
        if (sfip_set_ip(&tmp_ip, Ip) != SFIP_SUCCESS)
            return -1;

        tmp_ip.ip32[0] = ntohl(tmp_ip.ip32[0]);

        /* Just set ip to tmp_ip since we don't need to modify ip */
        Ip = &tmp_ip;
    }

    iRet = sfrt_insert((void *)Ip, (unsigned char)Ip->bits,
                       (void *)policyId, RT_FAVOR_SPECIFIC, config->netBindTable);
#else
    addr = ntohl(Ip->ip32[0]);
    iRet = sfrt_insert((void *)&addr, (unsigned char)Ip->bits,
                       (void *)policyId, RT_FAVOR_SPECIFIC, config->netBindTable);
#endif

    //DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Added  vlandId  %d, file %s, policyId: %d\n", vlanId, fileName, policyId););
    if (iRet)
    {
        return -1;
    }

    return 0; 
}

unsigned int sfNetworkGetBinding(
        tSfPolicyConfig *config,
        snort_ip_p ip
        )
{
    tSfPolicyId *policyId = NULL;
#ifdef SUP_IP6
    sfip_t tmp_ip;

    if ((void *)ip == NULL)
        return 0;
#else
    if (ip == 0)
        return 0;

#endif

#ifdef SUP_IP6
    if (ip->family == AF_INET)
    {
        if (sfip_set_ip(&tmp_ip, ip) != SFIP_SUCCESS)
        {
            /* Just return default configuration */
            return 0;
        }

        tmp_ip.ip32[0] = ntohl(tmp_ip.ip32[0]);

        /* Just set ip to tmp_ip since we don't need to modify ip */
        ip = &tmp_ip;
    }

    policyId = (tSfPolicyId *)sfrt_lookup((void *)ip, config->netBindTable);
#else
    ip = ntohl(ip);
    policyId = (tSfPolicyId *)sfrt_lookup((void *)&ip, config->netBindTable);
#endif

    if (!policyId)
    {
        return 0;
    }

    return *policyId;
}

void sfNetworkDeleteBinding(
        tSfPolicyConfig *config,
        snort_ip_p ip
        )
{
    tSfPolicyId *policyId;
#ifdef SUP_IP6
    sfip_t tmp_ip;

    if ((void *)ip == NULL)
        return;
#else
    if (ip == 0)
        return;

#endif

#ifdef SUP_IP6
    if (ip->family == AF_INET)
    {
        if (sfip_set_ip(&tmp_ip, ip) != SFIP_SUCCESS)
            return;

        tmp_ip.ip32[0] = ntohl(tmp_ip.ip32[0]);

        /* Just set ip to tmp_ip since we don't need to modify ip */
        ip = &tmp_ip;
    }

    policyId = (tSfPolicyId *)sfrt_lookup((void *)ip, config->netBindTable);
#else
    ip = ntohl(ip);
    policyId = (tSfPolicyId *)sfrt_lookup((void *)&ip, config->netBindTable);
#endif

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
