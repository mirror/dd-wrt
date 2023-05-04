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

#ifndef _SF_POLICY_DATA_H_
#define _SF_POLICY_DATA_H_

#include "generators.h"
#include "sfPolicy.h"

extern tSfPolicyId napRuntimePolicyId;
extern tSfPolicyId ipsRuntimePolicyId;
extern uint8_t iprep_current_update_counter;

static inline tSfPolicyId getNapRuntimePolicy(void)
{
    return napRuntimePolicyId;
}

static inline tSfPolicyId getIpsRuntimePolicy(void)
{
    return ipsRuntimePolicyId;
}

static inline tSfPolicyId getApplicableRuntimePolicy(uint32_t gid)
{
    if (gid == GENERATOR_INTERNAL)
        return getNapRuntimePolicy();
    else
        return getIpsRuntimePolicy();
}

static inline void setNapRuntimePolicy(tSfPolicyId id)
{
    napRuntimePolicyId = id;
}

static inline void setIpsRuntimePolicy(tSfPolicyId id)
{
    ipsRuntimePolicyId = id;
}

static inline int isNapRuntimePolicyDefault(void)
{
    return ( napRuntimePolicyId == SF_DEFAULT_POLICY_ID );
}

static inline int isIpsRuntimePolicyDefault(void)
{
    return ( ipsRuntimePolicyId == SF_DEFAULT_POLICY_ID );
}

static inline tSfPolicyId getParserPolicy(SnortConfig *sc)
{
    return sc ? sc->parserPolicyId : snort_conf->parserPolicyId;
}

static inline void setParserPolicy(SnortConfig *sc, tSfPolicyId id)
{
    if (sc)
        sc->parserPolicyId = id;
    else
        snort_conf->parserPolicyId = id;
}

static inline int isParserPolicyDefault(SnortConfig *sc)
{
    return ( ( sc ? sc->parserPolicyId : snort_conf->parserPolicyId ) == SF_DEFAULT_POLICY_ID );
}

static inline void setIPRepUpdateCount(uint8_t count)
{
   iprep_current_update_counter = count;
}

static inline uint8_t getIPRepUpdateCount(void)
{
   return iprep_current_update_counter;
}
#endif

