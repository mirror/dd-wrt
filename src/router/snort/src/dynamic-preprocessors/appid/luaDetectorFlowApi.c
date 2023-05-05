/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


/** @defgroup LuaDetectorFlowApi  LuaDetectorFlowApi
 * This module supports API towards Lua detectors for performing specific operations on a flow object.
 * The flow object on Lua side is a userData.
 *@{
 */

#define DETECTORFLOW "DetectorFlow"
#include "luaDetectorApi.h"
#include "luaDetectorFlowApi.h"
#include "luaDetectorModule.h"
#include "common_util.h"

/*static const char * LuaLogLabel = "luaDetectorFlowApi"; */

/* Lua flag bit/index to C flag value (0 for invalid). */
static const uint64_t FLAGS_TABLE_LUA_TO_C[32] = {
    0,                                /*  0 */
    0,                                /*  1 */
    0,                                /*  2 */
    0,                                /*  3 */
    0,                                /*  4 */
    0,                                /*  5 */
    0,                                /*  6 */
    0,                                /*  7 */
    0,                                /*  8 */
    0,                                /*  9 */
    0,                                /* 10 */
    0,                                /* 11 */
    0,                                /* 12 */
    0,                                /* 13 */
    0,                                /* 14 */
    0,                                /* 15 */
    0,                                /* 16 */
    0,                                /* 17 */
    0,                                /* 18 */
    0,                                /* 19 */
    APPID_SESSION_SSL_SESSION,        /* 20: sslSession */
    APPID_SESSION_HTTP_SESSION,       /* 21: httpSession */
    APPID_SESSION_UDP_REVERSED,       /* 22: udpReversed */
    APPID_SESSION_INCOMPATIBLE,       /* 23: incompatible */
    APPID_SESSION_IGNORE_HOST,        /* 24: ignoreHost */
    0,                                /* 25: ignoreTcpSeq -- OBSOLETE */
    APPID_SESSION_CLIENT_DETECTED,    /* 26: clientAppDetected */
    0,                                /* 27: gotBanner    -- OBSOLETE */
    APPID_SESSION_NOT_A_SERVICE,      /* 28: notAService */
    0,                                /* 29: logUnknown   -- OBSOLETE */
    APPID_SESSION_CONTINUE,           /* 30: continue */
    APPID_SESSION_SERVICE_DETECTED    /* 31: serviceDetected */
};

/* C flag bit/index to Lua flag value (0 for invalid). */
static const uint64_t FLAGS_TABLE_C_TO_LUA[32] = {
        0,             /*  0 */
        0,             /*  1 */
        0,             /*  2 */
        0,             /*  3 */
        0,             /*  4 */
        0,             /*  5 */
        0,             /*  6 */
        0,             /*  7 */
        0,             /*  8 */
        0,             /*  9 */
        0,             /* 10 */
        0,             /* 11 */
        0x00400000,    /* 12: APPID_SESSION_UDP_REVERSED */
        0x00200000,    /* 13: APPID_SESSION_HTTP_SESSION */
        0x80000000,    /* 14: APPID_SESSION_SERVICE_DETECTED */
        0x04000000,    /* 15: APPID_SESSION_CLIENT_DETECTED */
        0x10000000,    /* 16: APPID_SESSION_NOT_A_SERVICE */
        0,             /* 17 */
        0,             /* 18 */
        0x40000000,    /* 19: APPID_SESSION_CONTINUE */
        0x01000000,    /* 20: APPID_SESSION_IGNORE_HOST */
        0x00800000,    /* 21: APPID_SESSION_INCOMPATIBLE */
        0,             /* 22 */
        0,             /* 23 */
        0,             /* 24 */
        0,             /* 25 */
        0,             /* 26 */
        0,             /* 27 */
        0,             /* 28 */
        0,             /* 29 */
        0,             /* 30 */
        0              /* 31 */
};

/* Convert flag bits used by the Lua code into what the C code uses. */
static inline uint64_t ConvertFlagsLuaToC(uint64_t in)
{
    uint64_t out = 0;
    unsigned i;
    uint64_t msk;

    msk = 1;
    for (i = 0; i < 32; i++)
    {
        if (in & msk)
            out |= FLAGS_TABLE_LUA_TO_C[i];
        msk <<= 1;
    }

    return out;
}

/* Convert flag bits used by the C code into what the Lua code uses. */
static inline uint64_t ConvertFlagsCToLua(uint64_t in)
{
    uint64_t out = 0;
    unsigned i;
    uint64_t msk;

    msk = 1;
    for (i = 0; i < 32; i++)
    {
        if (in & msk)
            out |= FLAGS_TABLE_C_TO_LUA[i];
        msk <<= 1;
    }

    return out;
}

static DetectorFlowUserData *toDetectorFlowUserData (lua_State *L, int index)
{
  DetectorFlowUserData *pLuaData = (DetectorFlowUserData *)lua_touserdata(L, index);

  if (pLuaData == NULL)
  {
      luaL_typerror(L, index, DETECTORFLOW);
  }
  return pLuaData;
}

static DetectorFlowUserData *checkDetectorFlowUserData (lua_State *L, int index)
{
  DetectorFlowUserData *pLuaData;

  luaL_checktype(L, index, LUA_TUSERDATA);
  pLuaData = (DetectorFlowUserData *)luaL_checkudata(L, index, DETECTORFLOW);
  if (pLuaData == NULL)
  {
      luaL_typerror(L, index, DETECTORFLOW);
  }

  return pLuaData;
}

/**Internal function to create user data for a flow.
 *
 * @param Lua_State* - Lua state variable.
 * @return pointer to allocated DetectorFlowUserData object.
 */
DetectorFlowUserData *pushDetectorFlowUserData (lua_State *L)
{
    DetectorFlowUserData *pLuaData = (DetectorFlowUserData *)lua_newuserdata(L, sizeof(DetectorFlowUserData));
    DetectorFlow *pDetectorFlow;

    if (pLuaData)
    {
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"DetectotFlowUserData %p: allocated\n\n",pLuaData);
#endif
        memset(pLuaData, 0, sizeof(*pLuaData));

        if ((pLuaData->pDetectorFlow = (DetectorFlow *)_dpd.snortAlloc(1,
            sizeof(*pDetectorFlow), PP_APP_ID, PP_MEM_CATEGORY_SESSION)) == NULL)
        {
            lua_pop(L, -1);

            return NULL;
        }

        luaL_getmetatable(L, DETECTORFLOW);
        lua_setmetatable(L, -2);

        pDetectorFlow = pLuaData->pDetectorFlow;
        pDetectorFlow->myLuaState =  L;

        lua_pushvalue( L, -1 );     /*create a copy of userData */
        pDetectorFlow->userDataRef = luaL_ref( L, LUA_REGISTRYINDEX );

#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"DetectorFlow %p: allocated\n\n",pDetectorFlow);
#endif
        sflist_add_tail(&allocatedFlowList, pDetectorFlow);
    }
    return pLuaData;
}


/**Creates a user data for a flow.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param srcAddress/stack - source address of the flow
 * @param srcPort/stack - source port of the the flow
 * @param dstAddress/stack - destination address of the flow.
 * @param dstPort/stack - detector port of the flow.
 * @param proto/stack - protocol type. See defined IPPROTO_xxxx in /usr/include/netinet/in.h
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return DetectorFlowUserData/stack - A userdata representing DetectorFlowUserData.
 */
static int DetectorFlow_new (lua_State *L)
{
    DetectorFlowUserData *pLuaData = NULL;
    sfaddr_t saddr;
    sfaddr_t daddr;
    uint8_t proto;
    uint16_t sport, dport;
    DetectorUserData *detectorUserData = NULL;
    DetectorFlow *pDetectorFlow;
    char *pattern;
    size_t patternLen;

    detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        return 0;   /*number of results */
    }

    pattern = (char *)lua_tostring(L, 2);
    patternLen = lua_strlen (L, 2);

    if (patternLen == 16)
    {
        if (sfip_set_raw(&saddr, pattern, AF_INET6) != SFIP_SUCCESS)
            return 0;
    }
    else if (patternLen == 4)
    {
        if (sfip_set_raw(&saddr, pattern, AF_INET) != SFIP_SUCCESS)
            return 0;
    }
    else
    {
   	    return 0;
    }
    pattern = (char *)lua_tostring(L, 3);
    patternLen = lua_strlen (L, 3);

    if (patternLen == 16)
    {
        if (sfip_set_raw(&daddr, pattern, AF_INET6) != SFIP_SUCCESS)
            return 0;
    }
    else if (patternLen == 4)
    {
        if (sfip_set_raw(&daddr, pattern, AF_INET) != SFIP_SUCCESS)
            return 0;
    }
    else
    {
	    return 0;
    }

    sport = lua_tonumber(L, 4);
    dport = lua_tonumber(L, 5);
    proto = lua_tonumber(L, 6);

    pLuaData = pushDetectorFlowUserData(L);
    if (!pLuaData)
    {
        _dpd.errMsg( "Failed to allocate memory.");
        return 0;
    }

    pDetectorFlow = pLuaData->pDetectorFlow;

    pDetectorFlow->pFlow = AppIdEarlySessionCreate((tAppIdData*) pDetectorFlow, detectorUserData->pDetector->validateParams.pkt,
                                            &saddr, sport, &daddr, dport, proto, 0, 0);
    if (!pDetectorFlow->pFlow)
    {
        /*calloced buffer will be freed later after the current packet is processed. */
        lua_pop(L, 1);
        return 0;
    }

    return 1;
}

/**free DetectorFlow and its corresponding user data.
 */
void freeDetectorFlow(
        void *userdata
        )
{
    DetectorFlow *pDetectorFlow = (DetectorFlow *)userdata;

    /*The detectorUserData itself is a userdata and therefore be freed by Lua side. */
    if (pDetectorFlow->userDataRef != LUA_REFNIL)
    {
        DetectorFlowUserData *pLuaData;

        lua_rawgeti(pDetectorFlow->myLuaState, LUA_REGISTRYINDEX, pDetectorFlow->userDataRef);
        pLuaData = checkDetectorFlowUserData(pDetectorFlow->myLuaState, -1);
        if (pLuaData)
        {
            pLuaData->pDetectorFlow = NULL;
        }

        lua_pop(pDetectorFlow->myLuaState, 1);    /*remove result of rawgeti */

        luaL_unref (pDetectorFlow->myLuaState, LUA_REGISTRYINDEX, pDetectorFlow->userDataRef);
        pDetectorFlow->userDataRef = LUA_REFNIL;
    }

    /*free detectorFlow */
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"DetectorFlow %p: freeing\n\n",pDetectorFlow);
#endif
    _dpd.snortFree(pDetectorFlow, sizeof(*pDetectorFlow), PP_APP_ID, PP_MEM_CATEGORY_SESSION);

}

/**Sets a flow flag.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @param flags/stack - flags to be set.
 * @return int - Number of elements on stack, which is 0
 */
static int DetectorFlow_setFlowFlag(
        lua_State *L
        )
{
    DetectorFlowUserData *pLuaData;
    uint64_t flags;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        return 0;
    }

    flags = lua_tonumber(L, 2);
    flags = ConvertFlagsLuaToC(flags);

    setAppIdFlag(pLuaData->pDetectorFlow->pFlow, flags);

    return 0;
}

/**Gets a flow flag value.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @param flags/stack - flags to get.
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return flagValue/stack - value of a given flag.
 */
static int DetectorFlow_getFlowFlag(
        lua_State *L
        )
{
    DetectorFlowUserData *pLuaData;
    uint64_t flags;
    uint64_t ret;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        _dpd.errMsg( "getFlowFlag called without detectorFlowUserData");
        return 0;
    }

    flags = lua_tonumber(L, 2);
    flags = ConvertFlagsLuaToC(flags);

    ret = getAppIdFlag(pLuaData->pDetectorFlow->pFlow, flags);
    ret = ConvertFlagsCToLua(ret);
    lua_pushnumber(L, ret);

    return 1;
}

/**Clear a flow flag.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @param flags/stack - flags to be cleared.
 * @return int - Number of elements on stack, which is 0.
 */
static int DetectorFlow_clearFlowFlag(
        lua_State *L
        )
{
    DetectorFlowUserData *pLuaData;
    uint64_t flags;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        return 0;
    }

    flags = lua_tonumber(L, 2);
    flags = ConvertFlagsLuaToC(flags);

    clearAppIdFlag(pLuaData->pDetectorFlow->pFlow, flags);

    return 0;
}

/**Set service id on a flow.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @param serviceId/stack - service Id to be set on a flow.
 * @return int - Number of elements on stack, which is 0.
 */
static int DetectorFlow_setFlowServiceId(
        lua_State *L
        )
{
    return 0;
}

/**Set client application id on a flow.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @param applId/stack - client application Id to be set on a flow.
 * @return int - Number of elements on stack, which is 0.
 */
static int DetectorFlow_setFlowClntAppId(
        lua_State *L
        )
{
    return 0;
}

/**Set client application type id on a flow.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @param applTypeId/stack - client application type id to be set on a flow.
 * @return int - Number of elements on stack, which is 0.
 */
static int DetectorFlow_setFlowClntAppType(
        lua_State *L
        )
{
    return 0;
}
/**Design: For simplicity reason I am passing flowkey (20 bytes) to lua detectors.
 * The key is used to index into local lua table and get any flow specific data that a detector needs.
 * This approach avoids embedding lua detector data into core engine flow data structure.
 *
 * For optimization, I could have created an integer index on C side. This can be taken up in future.
 */

/**Get flow key from a DetectorFlowUserData object.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorflow/stack - DetectorFlowUserData object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return flowKey/stack - A 20 byte flow key
 */
static int DetectorFlow_getFlowKey(
        lua_State *L
        )
{
    DetectorFlowUserData *pLuaData;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        return 0;
    }

    lua_pushlstring(L, (char *)&pLuaData->pDetectorFlow->pFlow->flowId, sizeof(pLuaData->pDetectorFlow->pFlow->flowId));

    return 1;
}



static const luaL_Reg DetectorFlow_methods[] = {
  /* Obsolete API names.  No longer use these!  They are here for backward
   * compatibility and will eventually be removed. */
  /*  - "new" is now "createFlow" (below) */
  {"new",                      DetectorFlow_new},

  {"createFlow",               DetectorFlow_new},
  {"setFlowFlag",              DetectorFlow_setFlowFlag},
  {"getFlowFlag",              DetectorFlow_getFlowFlag},
  {"clearFlowFlag",            DetectorFlow_clearFlowFlag},
  {"setFlowServiceId",         DetectorFlow_setFlowServiceId},
  {"setFlowClntAppId",         DetectorFlow_setFlowClntAppId},
  {"setFlowClntAppType",       DetectorFlow_setFlowClntAppType},
  {"getFlowKey",               DetectorFlow_getFlowKey},
  {0, 0}
};

/**
 * lua_close will ensure that all detectors and flows get _gc called.
 */
static int DetectorFlow_gc (
        lua_State *L
        )
{
    DetectorFlowUserData * pLuaData =  toDetectorFlowUserData(L, 1);

    if (pLuaData)
    {
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"DetectorFlowUserData %p: freeing\n\n",pLuaData);
#endif
        /*Lua frees DetectorFlowUserData buffer. */
    }

    return 0;
}

static int DetectorFlow_tostring (
        lua_State *L
        )
{
  char buff[32];
  snprintf(buff, sizeof(buff), "%p", toDetectorFlowUserData(L, 1));
  lua_pushfstring(L, "DetectorFlowUserData (%s)", buff);
  return 1;
}

static const luaL_Reg DetectorFlow_meta[] = {
  {"__gc",       DetectorFlow_gc},
  {"__tostring", DetectorFlow_tostring},
  {0, 0}
};

/**Registers C functions as an API, enabling Lua detector to call these functions. This function
 * should be called once before loading any lua detectors. This function itself is not part of API
 * and therefore can not be called by a Lua detection.
 *
 * @param Lua_State* - Lua state variable.
 * @param detectorFlow/stack - DetectorFlowUserData object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return methodArray/stack - array of newly created methods
 */
int DetectorFlow_register (
        lua_State *L
        )
{

  /* populates a new table with Detector_methods (method_table), add the table to the globals and stack*/
  luaL_openlib(L, DETECTORFLOW, DetectorFlow_methods, 0);

  /* create metatable for Foo, add it to the Lua registry, metatable on stack */
  luaL_newmetatable(L, DETECTORFLOW);

  /* populates table on stack with Detector_meta methods, puts the metatable on stack*/
  luaL_openlib(L, NULL, DetectorFlow_meta, 0);

  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -3);               /* dup methods table*/
  lua_settable(L, -3);                  /* metatable.__index = methods */

  lua_pushliteral(L, "__metatable");
  lua_pushvalue(L, -3);               /* dup methods table*/
  lua_settable(L, -3);                  /* hide metatable:
                                           metatable.__metatable = methods */
  lua_pop(L, 1);                      /* drop metatable */
  return 1;                           /* return methods on the stack */


}

/** @} */ /* end of LuaDetectorFlowApi */
