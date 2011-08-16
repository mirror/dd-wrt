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

#ifndef _SF_POLICY_USER_DATA_H_
#define _SF_POLICY_USER_DATA_H_

#include "sf_ip.h"
#include "ipv6_port.h"
#include "sfPolicy.h"
/*SharedObjectAddStarts
#include "sf_dynamic_preprocessor.h"
extern DynamicPreprocessorData _dpd;
SharedObjectAddEnds */

typedef struct
{
    /**policy id of configuration file or packet being processed.
    */
    tSfPolicyId  currentPolicyId;

    /**Number of policies currently allocated.
     */
    unsigned int numAllocatedPolicies;

    /**Number of policies active. Since we use an array of policy pointers, 
     * number of allocated policies may be more than active policies. */
    unsigned int numActivePolicies;

    /**user configuration for a policy. This is a pointer to an array of pointers 
     * to user configuration.
    */
    void **userConfig;

} tSfPolicyUserContext;

typedef tSfPolicyUserContext * tSfPolicyUserContextId;

//SharedObjectDeleteBegins
extern tSfPolicyId runtimePolicyId;
extern tSfPolicyId parserPolicyId;

static INLINE tSfPolicyId getRuntimePolicy(void) 
{
    return runtimePolicyId;
}

static INLINE void setRuntimePolicy(tSfPolicyId id) 
{
    runtimePolicyId = id;
}

static INLINE int isRuntimePolicyDefault(void) 
{
    return (runtimePolicyId == 0);
}

static INLINE tSfPolicyId getParserPolicy(void) 
{
    return parserPolicyId;
}

static INLINE void setParserPolicy(tSfPolicyId id) 
{
    parserPolicyId = id;
}

static INLINE int isParserPolicyDefault(void) 
{
    return (parserPolicyId == 0);
}

static INLINE tSfPolicyId getDefaultPolicy(void) 
{
    return 0;
}
//SharedObjectDeleteEnds

tSfPolicyUserContextId sfPolicyConfigCreate(
        void
        );

void sfPolicyConfigDelete(
        tSfPolicyUserContextId pContext
        );

//Functions for setting, getting and clearing policy ids
static INLINE void sfPolicyUserPolicySet (
        tSfPolicyUserContextId pContext, 
        tSfPolicyId policyId 
        )
{
    pContext->currentPolicyId = policyId;
}

static INLINE tSfPolicyId sfPolicyUserPolicyGet (
        tSfPolicyUserContextId pContext 
        )
{
    return pContext->currentPolicyId;
}

static INLINE unsigned int sfPolicyUserPolicyGetActive (
        tSfPolicyUserContextId pContext
        )
{
    return (pContext->numActivePolicies);
}

//Functions for setting, getting and clearing user data specific to policies.
int sfPolicyUserDataSet (
        tSfPolicyUserContextId pContext, 
        tSfPolicyId policyId, 
        void *config
        );
static INLINE void * sfPolicyUserDataGet (
        tSfPolicyUserContextId pContext, 
        tSfPolicyId policyId
        )
{
    if ((pContext != NULL) && (policyId < pContext->numAllocatedPolicies))
    {
        return pContext->userConfig[policyId];
    }

    return NULL;
}

static INLINE int sfPolicyUserDataSetDefault (
        tSfPolicyUserContextId pContext, 
        void *config
        )
{
    return sfPolicyUserDataSet (pContext, getDefaultPolicy(), config);
}

static INLINE void * sfPolicyUserDataGetDefault (
        tSfPolicyUserContextId pContext
        )
{
    return sfPolicyUserDataGet (pContext, getDefaultPolicy());
}

static INLINE int sfPolicyUserDataSetCurrent (
        tSfPolicyUserContextId pContext, 
        void *config
        )
{
    return sfPolicyUserDataSet (pContext, pContext->currentPolicyId, config);
}
static INLINE void * sfPolicyUserDataGetCurrent (
        tSfPolicyUserContextId pContext
        )
{
    return sfPolicyUserDataGet (pContext, pContext->currentPolicyId);
}

void * sfPolicyUserDataClear (
        tSfPolicyUserContextId pContext,
        tSfPolicyId policyId
        );

int sfPolicyUserDataIterate (
        tSfPolicyUserContextId pContext,
        int (*callback)(tSfPolicyUserContextId pContext, tSfPolicyId policyId, void* config)
        );


#endif
