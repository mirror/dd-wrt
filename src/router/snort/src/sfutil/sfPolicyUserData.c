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
#include "string.h"
#include "sf_types.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/** @defgroup sfPolicyConfig Sourcefire policy configuration module
 *
 *  Create a user policy configuration context. A context provides facility for creating
 *  policy specific data instances. User can create as many policy instances as memory
 *  resources will allow. User can create/delete context, set/clear/get user date for a
 *  specific policy, default policy or current policy. User can also iterate over all instances
 *  user data.
 *
 *  In current design, preprocessor use this module directly to manage policy specific data
 *  instances. A future enhancement can be to extract policy management code from each processor
 *  and put it in a new policy management module. Policy management module will set a single
 *  pointer to user data before calling appropriate callback function in a preprocessor. As
 *  an example, policy module will iterate over all policies and call CleanExit functions in every
 *  preprocessor for each policy. This will make policy management module will hide policies from
 *  preprocessors and make them policy agnostic.
 *  @{
 */

/**Create a user context.
 * Allocates a new context and return it to user. All transactions within a context are independent from
 * any other transactions in a different context.
 *
 * @returns tSfPolicyUserContextId
*/
tSfPolicyUserContextId sfPolicyConfigCreate(void)
{
    tSfPolicyUserContext *pTmp = NULL;

    pTmp = calloc(1, sizeof(tSfPolicyUserContext));

    return pTmp;
}

/**Delete a user policy data context.
 * @param pContext
 */
void sfPolicyConfigDelete(
        tSfPolicyUserContextId pContext
        )
{
    if (pContext == NULL)
        return;

    if (pContext->userConfig != NULL)
        free(pContext->userConfig);

    free(pContext);
}

/**Store a pointer to user data.
 * @param pContext
 * @param policyId is 0 based.
 * @param config - pointer to user configuration.
 */
int sfPolicyUserDataSet (
        tSfPolicyUserContextId pContext,
        tSfPolicyId policyId,
        void *config
        )
{
    void **ppTmp;

    if (policyId >= pContext->numAllocatedPolicies)
    {
        //expand the array
        ppTmp = (void **)calloc(policyId+POLICY_ALLOCATION_CHUNK, sizeof(void *));
        if (!(ppTmp))
        {
            return -1;
        }

        if (pContext->numAllocatedPolicies)
        {
            memcpy(ppTmp, pContext->userConfig, sizeof(void*)*(pContext->numAllocatedPolicies));
            free(pContext->userConfig);
        }

        pContext->userConfig = ppTmp;
        pContext->numAllocatedPolicies = policyId + POLICY_ALLOCATION_CHUNK;
    }

    if (pContext->userConfig[policyId])
    {
        //dont overwrite existing configuration
        return -1;
    }

    pContext->userConfig[policyId] = config;
    pContext->numActivePolicies++;

    return 0;
}

/**user is responsible for freeing any memory.
 */
void * sfPolicyUserDataClear (
        tSfPolicyUserContextId pContext,
        tSfPolicyId policyId
        )
{
    void *pTmp = NULL;

    if (policyId < pContext->numAllocatedPolicies)
    {
        pTmp = pContext->userConfig[policyId];
        pContext->userConfig[policyId] = NULL;
        pContext->numActivePolicies--;
    }

    return pTmp;
}

int sfPolicyUserDataIterate (
        struct _SnortConfig *sc,
        tSfPolicyUserContextId pContext,
        int (*callback)(struct _SnortConfig *sc, tSfPolicyUserContextId pContext, tSfPolicyId policyId, void* config)
        )
{
    tSfPolicyId policyId;
    int ret = 0;

    //must not use numActivePolicies because the callback may delete a policy
    for (policyId = 0; policyId < pContext->numAllocatedPolicies; policyId++)
    {
        if (pContext->userConfig[policyId])
        {
            ret = callback(sc, pContext, policyId, pContext->userConfig[policyId]);
            if (ret != 0)
                break;
        }
    }

    return ret;
}

int sfPolicyUserDataFreeIterate (
        tSfPolicyUserContextId pContext,
        int (*callback)(tSfPolicyUserContextId pContext, tSfPolicyId policyId, void* config)
        )
{
    tSfPolicyId policyId;
    int ret = 0;

    //must not use numActivePolicies because the callback may delete a policy
    for (policyId = 0; policyId < pContext->numAllocatedPolicies; policyId++)
    {
        if (pContext->userConfig[policyId])
        {
            ret = callback(pContext, policyId, pContext->userConfig[policyId]);
            if (ret != 0)
                break;
        }
    }

    return ret;
}

/** @} */ //

