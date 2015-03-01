/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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

        if ((pLuaData->pDetectorFlow = (DetectorFlow *)calloc(1, sizeof(DetectorFlow))) == NULL)
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
    snort_ip saddr;
    snort_ip daddr;
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
        saddr.family = AF_INET6;
        saddr.bits = 128;
	    memcpy(&saddr.ip8, pattern, patternLen);
    }
    else if (patternLen == 4)
    {
        saddr.family = AF_INET;
        saddr.bits = 32;
	    memcpy(&saddr.ip32[0], pattern, patternLen);
	    saddr.ip32[1] = saddr.ip32[2] = saddr.ip32[3] = 0;
    }
    else
    {
   	    return 0;
    }
    pattern = (char *)lua_tostring(L, 3);
    patternLen = lua_strlen (L, 3);

    if (patternLen == 16)
    {
        daddr.family = AF_INET6;
        daddr.bits = 128;
	    memcpy(&daddr.ip8, pattern, patternLen);
    }
    else if (patternLen == 4)
    {
        daddr.family = AF_INET;
        daddr.bits = 32;
   	    memcpy(&daddr.ip32[0], pattern, patternLen);
	    daddr.ip32[1] = daddr.ip32[2] = daddr.ip32[3] = 0;
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

    pDetectorFlow->pFlow = AppIdEarlySessionCreate(detectorUserData->pDetector->validateParams.pkt,
                                            &saddr, sport, &daddr, dport, proto, 0);
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
    free(pDetectorFlow);

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
    uint32_t flags;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        return 0;
    }

    flags = lua_tonumber(L, 2);

    flow_mark(pLuaData->pDetectorFlow->pFlow, flags);

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
    uint32_t flags;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        _dpd.errMsg( "getFlowFlag called without detectorFlowUserData");
        return 0;
    }

    flags = lua_tonumber(L, 2);

    lua_pushnumber(L, flow_checkflag(pLuaData->pDetectorFlow->pFlow, flags));

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
    uint32_t flags;

    pLuaData = checkDetectorFlowUserData(L, 1);
    if (!pLuaData || !pLuaData->pDetectorFlow)
    {
        return 0;
    }

    flags = lua_tonumber(L, 2);

    flow_clear(pLuaData->pDetectorFlow->pFlow, flags);

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
 * @return flowKey/stack - A 4 byte flow key
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



static const luaL_reg DetectorFlow_methods[] = {
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
  sprintf(buff, "%p", toDetectorFlowUserData(L, 1));
  lua_pushfstring(L, "DetectorFlowUserData (%s)", buff);
  return 1;
}

static const luaL_reg DetectorFlow_meta[] = {
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
