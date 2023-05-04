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

#ifndef _SF_POLICY_USER_DATA_H_
#define _SF_POLICY_USER_DATA_H_

#include "sf_ip.h"
#include "ipv6_port.h"
#include "sfPolicy.h"
/*SharedObjectAddStarts
#include "sf_dynamic_preprocessor.h"
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
// index for default nap & ips polices is 0
// so single function to return the default idx
static inline tSfPolicyId getDefaultPolicy(void)
{
    return SF_DEFAULT_POLICY_ID;
}
//SharedObjectDeleteEnds

tSfPolicyUserContextId sfPolicyConfigCreate( void );
void sfPolicyConfigDelete( tSfPolicyUserContextId pContext );

//Functions for setting, getting and clearing policy ids
static inline void sfPolicyUserPolicySet ( tSfPolicyUserContextId pContext, tSfPolicyId policyId )
{
    pContext->currentPolicyId = policyId;
}

static inline tSfPolicyId sfPolicyUserPolicyGet ( tSfPolicyUserContextId pContext )
{
    return pContext->currentPolicyId;
}

static inline unsigned int sfPolicyUserPolicyGetActive ( tSfPolicyUserContextId pContext )
{
    return (pContext->numActivePolicies);
}

//Functions for setting, getting and clearing user data specific to policies.
int sfPolicyUserDataSet ( tSfPolicyUserContextId pContext, tSfPolicyId policyId, void *config );

static inline void * sfPolicyUserDataGet ( tSfPolicyUserContextId pContext, tSfPolicyId policyId )
{
    if (pContext && policyId < pContext->numAllocatedPolicies)
        return pContext->userConfig[policyId];

    return NULL;
}

static inline int sfPolicyUserDataSetDefault ( tSfPolicyUserContextId pContext, void *config )
{
    return sfPolicyUserDataSet (pContext, getDefaultPolicy(), config);
}

static inline void * sfPolicyUserDataGetDefault ( tSfPolicyUserContextId pContext )
{
    return sfPolicyUserDataGet (pContext, getDefaultPolicy());
}

static inline int sfPolicyUserDataSetCurrent ( tSfPolicyUserContextId pContext, void *config )
{
    return sfPolicyUserDataSet (pContext, pContext->currentPolicyId, config);
}

static inline void * sfPolicyUserDataGetCurrent ( tSfPolicyUserContextId pContext )
{
    return sfPolicyUserDataGet (pContext, pContext->currentPolicyId);
}

void *sfPolicyUserDataClear( tSfPolicyUserContextId pContext, tSfPolicyId policyId );

int sfPolicyUserDataIterate( struct _SnortConfig *sc, tSfPolicyUserContextId pContext,
                             int ( *callback )( struct _SnortConfig *sc, 
                                                tSfPolicyUserContextId pContext, 
                                                tSfPolicyId policyId,
                                                void *config ) );

int sfPolicyUserDataFreeIterate( tSfPolicyUserContextId pContext,
                                 int ( *callback )( tSfPolicyUserContextId pContext, 
                                                    tSfPolicyId policyId, 
                                                    void *config ) );


#endif
