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

#ifndef _SF_POLICY_H_
#define _SF_POLICY_H_

#include "sf_ip.h"
#include "ipv6_port.h"
#include "sfrt.h"
#include "snort_debug.h"

/**Number of additional policies allocated with each re-alloc operation. */
#define POLICY_ALLOCATION_CHUNK 10
#define SF_VLAN_BINDING_MAX 4096
#define SF_POLICY_ID_BINDING_MAX 4096
#define SF_NETWORK_BINDING_MAX 4096
#define SF_POLICY_UNBOUND 0xffffffff
#define SF_DEFAULT_POLICY_ID 0

/*vlan id or address range is reduced to policy id. and subsequent processing is done using policy id only. */

typedef struct
{
    /**number of vlans which are member of this group. When membership falls to 0, then this group should be deleted.
     */
    unsigned int refCount;
    char *filename;
    unsigned int isConfigProcessed:1;

} tSfPolicy;

typedef enum {
    SF_BINDING_TYPE_VLAN,
    SF_BINDING_TYPE_NETWORK,
    SF_BINDING_TYPE_POLICY_ID,
    SF_BINDING_TYPE_UNKNOWN
} tSF_BINDING_TYPE;

typedef unsigned int tSfPolicyId;

typedef struct
{
    /**group id assigned to each file name. The groupId is an abstract concept
     * to tie multiple vlans into one group. */
    tSfPolicy **ppPolicies;
    tSfPolicyId defaultPolicyId;
    /**policy id of configuration file or packet being processed. */
    tSfPolicyId numAllocatedPolicies;
    unsigned int numActivePolicies;
    /**vlan to policyId bindings. */
    tSfPolicyId vlanBindings[SF_VLAN_BINDING_MAX];
    /**policyId to policyId bindings. */
    tSfPolicyId policyIdBindings[SF_POLICY_ID_BINDING_MAX];
    /**Network to policyId bindings. */
    table_t *netBindTable;

} tSfPolicyConfig;

tSfPolicyConfig * sfPolicyInit(
    void
    );
void sfPolicyFini(
    tSfPolicyConfig *
    );
int sfPolicyAdd(
    tSfPolicyConfig *,
    char *
    );
void sfPolicyDelete(
    tSfPolicyConfig *,
    tSfPolicyId
    );
char * sfPolicyGet(
    tSfPolicyConfig *,
    tSfPolicyId
    );
int sfVlanAddBinding(
    tSfPolicyConfig *,
    int,
    char *
    );
tSfPolicyId sfVlanGetBinding(
    tSfPolicyConfig *,
    int
    );
void sfVlanDeleteBinding(
    tSfPolicyConfig *,
    int
    );
int sfPolicyIdAddBinding(
    tSfPolicyConfig *,
    int,
    char *
    );
tSfPolicyId sfPolicyIdGetBinding(
    tSfPolicyConfig *,
    int
    );
void sfPolicyIdDeleteBinding(
    tSfPolicyConfig *,
    int
    );
unsigned int sfGetApplicablePolicyId(
    tSfPolicyConfig *,
    int,
    sfaddr_t*,
    sfaddr_t*
    );
int sfNetworkAddBinding(
    tSfPolicyConfig *,
    sfcidr_t *,
    char *
    );
unsigned int sfNetworkGetBinding(
    tSfPolicyConfig *,
    sfaddr_t*
    );
void sfNetworkDeleteBinding(
    tSfPolicyConfig *,
    sfaddr_t*
    );

static inline tSfPolicyId sfGetDefaultPolicy(
    tSfPolicyConfig *config
    )
{
    if (config == NULL)
        return 0;

    return config->defaultPolicyId;
}

static inline void sfSetDefaultPolicy(
    tSfPolicyConfig *config,
    tSfPolicyId policyId
    )
{
    if ((config == NULL) || (policyId >= config->numAllocatedPolicies))
        return;

    config->defaultPolicyId = policyId;
}

static inline tSfPolicyId sfPolicyNumAllocated(
    tSfPolicyConfig *config
    )
{
    if (config == NULL)
        return 0;

    return config->numAllocatedPolicies;
}

/*dynamic array functions */
int sfDynArrayCheckBounds (
        void ** dynArray,
        unsigned int index,
        unsigned int *maxElements
        );

typedef tSfPolicyId (*GetPolicyFunc)(void);
struct _SnortConfig;
typedef tSfPolicyId (*GetParserPolicyFunc)(struct _SnortConfig *);

#endif
