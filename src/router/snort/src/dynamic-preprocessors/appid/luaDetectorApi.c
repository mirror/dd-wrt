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


/** @defgroup LuaDetectorBaseApi LuaDetectorBaseApi
 * This module supports basic API towards Lua detectors.
 *@{
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#include "client_app_base.h"
#include "service_base.h"
#include "luaDetectorApi.h"
#include "luaDetectorModule.h"
#include "luaDetectorApi.h"
#include "luaDetectorFlowApi.h"
#include <pcre.h>
#include "httpCommon.h"
#include "sf_multi_mpse.h"
#include "fw_appid.h"
#include "http_url_patterns.h"
#include "service_ssl.h"
#include "hostPortAppCache.h"
#include "appInfoTable.h"
#include "ip_funcs.h"

#define DETECTOR "Detector"
#define OVECCOUNT 30    /* should be a multiple of 3 */
#define URL_LIST_STEP_SIZE  5000

typedef enum {
    LUA_LOG_CRITICAL = 0,
    LUA_LOG_ERR = 1,
    LUA_LOG_WARN = 2,
    LUA_LOG_NOTICE = 3,
    LUA_LOG_INFO = 4,
    LUA_LOG_DEBUG = 5,
} LUA_LOG_LEVELS;

HttpPatternLists httpPatternLists;

/*static const char * LuaLogLabel = "luaDetectorApi"; */

#ifdef PERF_PROFILING
PreprocStats luaClientPerfStats;
PreprocStats luaServerPerfStats;
#endif

static void FreeDetectorAppUrlPattern(DetectorAppUrlPattern *pattern);

static DetectorUserData *toDetectorUserData (lua_State *L, int index)
{
  DetectorUserData *bar = (DetectorUserData *)lua_touserdata(L, index);
  if (bar == NULL) luaL_typerror(L, index, DETECTOR);
  return bar;
}

DetectorUserData *checkDetectorUserData (
        lua_State *L,
        int index
        )
{
  DetectorUserData *bar;

  luaL_checktype(L, index, LUA_TUSERDATA);

  bar = (DetectorUserData *)luaL_checkudata(L, index, DETECTOR);
  if (bar == NULL)
  {
      luaL_typerror(L, index, DETECTOR);
  }

  return bar;
}

static DetectorUserData *pushDetectorUserData(lua_State *L)
{
  DetectorUserData *bar = (DetectorUserData *)lua_newuserdata(L, sizeof(DetectorUserData));

  if (bar)
  {
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"DetectorUserData %p: allocated\n\n",bar);
#endif
      memset(bar, 0, sizeof(*bar));

      if ((bar->pDetector = (Detector *)calloc(1, sizeof(Detector))) == NULL)
      {
          lua_pop(L, -1);

          return NULL;
      }

      luaL_getmetatable(L, DETECTOR);
      lua_setmetatable(L, -2);

#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"Detector %p: allocated\n\n",bar->pDetector);
#endif
  }
  return bar;
}

Detector *createDetector(
        lua_State *L,
        const char *chunkName
        )
{
    DetectorUserData *pUserData;
    Detector  *detector;

    pUserData = pushDetectorUserData(L);

    if (!pUserData || !pUserData->pDetector)
    {
        _dpd.errMsg( "Failed to allocate memory.");
        return NULL;
    }

    detector = pUserData->pDetector;

    lua_pushvalue( L, -1 );     /*create a copy of userData */
    detector->detectorUserDataRef = luaL_ref( L, LUA_REGISTRYINDEX );
    detector->chunkName = strdup(chunkName);
    if (!detector->chunkName)
    {
        free(pUserData->pDetector);
        return NULL;
    }

    detector->myLuaState = L;

    return detector;
}


/**must be called only when RNA is exitting.
 */
void freeDetector(Detector *detector)
{
    tDetectorPackageInfo  *pkg = &detector->packageInfo;

    if (!detector)
        return;

    if (detector->server.pServiceElement)
    {
        free(detector->server.pServiceElement);
    }

    if (detector->server.serviceModule.name)
    {
        free((void *)(detector->server.serviceModule.name));
    }

    if (pkg->name)
    {
        free(pkg->name);
    }
    if (pkg->client.initFunctionName)
    {
        free(pkg->client.initFunctionName);
    }
    if (pkg->client.cleanFunctionName)
    {
        free(pkg->client.cleanFunctionName);
    }
    if (pkg->client.validateFunctionName)
    {
        free(pkg->client.validateFunctionName);
    }
    if (pkg->server.initFunctionName)
    {
        free(pkg->server.initFunctionName);
    }
    if (pkg->server.cleanFunctionName)
    {
        free(pkg->server.cleanFunctionName);
    }
    if (pkg->server.validateFunctionName)
    {
        free(pkg->server.validateFunctionName);
    }

    /*The detectorUserData itself is a userdata and therefore be freed by Lua side. */
    if (detector->detectorUserDataRef != LUA_REFNIL)
    {
        DetectorUserData *pUserData;

        lua_rawgeti(detector->myLuaState, LUA_REGISTRYINDEX, detector->detectorUserDataRef);
        pUserData = checkDetectorUserData(detector->myLuaState, -1);
        if (pUserData)
        {
            pUserData->pDetector = NULL;
        }

        luaL_unref (detector->myLuaState, LUA_REGISTRYINDEX, detector->detectorUserDataRef);
        detector->detectorUserDataRef = LUA_REFNIL;
    }

    free(detector->chunkName);
    free(detector->validatorBuffer);

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"Detector %p: freed\n\n",detector);
#endif
    free(detector);
}

/*converts Luastring to C compatible string. */
static int storeLuaString(const char *LuaString, char **CString)
{
    char *string = *CString;

    if (!LuaString)
    {
        return 0;
    }

    *CString = strdup(LuaString);
    if (*CString == NULL)
    {
        *CString = string;
        return -1;
    }
    if (string)
    {
        free(string);
    }

    return 0;
}

/*check service element, Allocate if necessary */
int checkServiceElement(
        Detector *detector
        )
{
    if (!detector->server.pServiceElement)
    {
        detector->server.pServiceElement = calloc(1, sizeof(*detector->server.pServiceElement));
        if (!detector->server.pServiceElement)
        {
            return 0;
        }
        detector->server.pServiceElement->name = detector->server.serviceModule.name;
    }

    return 1;
}

/**Creates a new detector instance. Creates a new detector instance and leaves the instance
 * on stack. This is the first call by a lua detector to create and instance. Later calls
 * provide the detector instance.
 *
 * @param Lua_State* - Lua state variable.
 * @param serviceName/stack - name of service
 * @param pValidator/stack - service validator function name
 * @param pFini/stack - service clean exit function name
 * @return int - Number of elements on stack, which should be 1 if success 0 otherwise.
 * @return detector - a detector instance on stack if successful
 */

static int service_init(lua_State *L)
{
    const char *pValidator;
    const char *pServiceName;
    const char *pFini;
    Detector *detector;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);
    pServiceName = lua_tostring(L, 2);
    pValidator = lua_tostring(L, 3);
    pFini = lua_tostring(L, 4);

    if ((!detectorUserData) || (!pServiceName) || (!pValidator) || (!pFini))
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    lua_getglobal(L, pValidator);
    lua_getglobal(L, pFini);
    if (!(lua_isfunction(L, -1)) || !(lua_isfunction(L, -2)))
    {
        _dpd.errMsg("%s: attempted setting validator/fini to non-function\n",detector->server.serviceModule.name);
        lua_pop(L, 2);
        return 0;
    }

    lua_pop(L, 2);

    /*old value is preserved so no error checks needed. */
    if (!detector->server.serviceModule.name)
        storeLuaString(pServiceName, (char **)&detector->server.serviceModule.name);
    storeLuaString(pValidator, &detector->packageInfo.server.validateFunctionName);
    storeLuaString(pFini, &detector->packageInfo.server.cleanFunctionName);

    /*create a ServiceElement */
    if (checkServiceElement(detector))
    {
        detector->server.pServiceElement->validate = validateAnyService;
        detector->server.pServiceElement->userdata = detector;

        detector->server.pServiceElement->detectorType = DETECTOR_TYPE_DECODER;
    }

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"allocated detector = %p\n",detector);
#endif
    return 1;
}

/**Register a pattern for fast pattern matching. Lua detector calls this function to register a pattern
 * for fast pattern matching. This is similar to registerPattern in traditional C detector.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param protocol/stack - protocol type. Values can be {tcp=6, udp=17 }
 * @param pattern/stack - pattern string.
 * @param size/stack - number of bytes in pattern
 * @param position/stack -  position offset where to start matching pattern.
 * @return int - Number of elements on stack, which is always 1.
 * @return status/stack - 0 if successful, -1 otherwise.
 */
static int service_registerPattern(
        lua_State *L
        )
{
    int protocol;
    size_t size;
    const char *pattern;
    unsigned int position;
    Detector *detector;
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    protocol = lua_tonumber(L, index++);
    pattern = lua_tostring(L, index++);
    size = lua_tonumber(L, index++);
    position = lua_tonumber(L, index++);

    if ((pattern == NULL) || (detectorUserData == NULL))
    {
        lua_pushnumber(L, -1);
        return 1;   /*number of results */
    }

    detector = detectorUserData->pDetector;

    /*Note: we can not give callback into lua directly so we have to */
    /*give a local callback function, which will do demuxing and */
    /*then call lua callback function. */

    /*mpse library does not hold reference to pattern therefore we dont need to allocate it. */

    ServiceRegisterPatternDetector(validateAnyService, protocol, (uint8_t *)pattern,
                                   size, position, detector, detector->server.serviceModule.name);

    lua_pushnumber(L, 0);
    return 1;   /*number of results */
}

static int common_registerAppId(
        lua_State *L
        )
{
    unsigned int appId;
    Detector *detector;
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    appId = lua_tonumber(L, index++);

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, -1);
        return 1;   /*number of results */
    }

    detector = detectorUserData->pDetector;

    if (detector->packageInfo.server.initFunctionName)
        appSetServiceValidator(validateAnyService, appId, APPINFO_FLAG_SERVICE_ADDITIONAL, (void *)detector);
    if (detector->packageInfo.client.initFunctionName)
        appSetClientValidator(validateAnyClientApp, appId, APPINFO_FLAG_CLIENT_ADDITIONAL, (void *)detector);

    appInfoSetActive(appId, true);

    lua_pushnumber(L, 0);
    return 1;   /*number of results */
}

static int Detector_htons(
        lua_State *L
        )
{
    unsigned short aShort;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);
    aShort = lua_tonumber(L, 2);

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, htons(aShort));
    return 1;
}

static int Detector_htonl(
        lua_State *L
        )
{
    unsigned int anInt;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);
    anInt = lua_tonumber(L, 2);

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, htonl(anInt));
    return 1;
}

/**Logs messages from detectors into wherever /etc/syslog.conf directs them.
 * examples are:
 *     detector:log(DC.logLevel.warning, 'a warning')
 *
 *@param level - level of message. See DetectorCommon for enumeration.
 *@param message - message to be logged.
 */
static int Detector_logMessage(
        lua_State *L
        )
{
    unsigned int level;
    const char *message;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    level = lua_tonumber(L, 2);
    message = lua_tostring(L, 3);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    switch (level)
    {
        case LUA_LOG_CRITICAL:
            _dpd.fatalMsg("%s:%s\n",detector->server.serviceModule.name, message);
            break;
        case LUA_LOG_ERR:
            _dpd.errMsg("%s:%s\n",detector->server.serviceModule.name, message);
            break;
        case LUA_LOG_WARN:
            _dpd.errMsg("%s:%s\n",detector->server.serviceModule.name, message);
            break;
        case LUA_LOG_NOTICE:
            _dpd.logMsg("%s:%s\n",detector->server.serviceModule.name, message);
            break;
        case LUA_LOG_INFO:
            _dpd.logMsg("%s:%s\n",detector->server.serviceModule.name, message);
            break;
        case LUA_LOG_DEBUG:
#ifdef LUA_DETECTOR_DEBUG
            _dpd.debugMsg(DEBUG_LOG,"%s:%s\n",detector->server.serviceModule.name, message);
#endif
            break;
        default:
            break;
    }

    return 0;
}

/** Analyze application payload
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param major/stack - major number of application
 * @param minor/stack - minor number of application
 * @param flags/stack - any flags
 * @return int - Number of elements on stack, which is always 0.
 */
static int service_analyzePayload(
        lua_State *L
        )
{
    unsigned int payloadId;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    payloadId = lua_tonumber(L, 2);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->validateParams.flowp->payloadAppId = payloadId;

    lua_pushnumber(L, 0);
    return 1;
}

/**Design notes: Due to following two design limitations:
 * a. lua validate functions, known only at runtime, can not be wrapped inside unique
 *    C functions at runtime and
 * b. core engine can not call lua functions directly.
 *
 * There must be a common validate function in C that in turn calls relevent Lua functions.
 * Right now there is only one detector so there is a one-to-one mapping, but the framework
 * will have to support multiple detectors in production environment. Core engine API will be
 * changed to take an additional void* that will be used to call only a unique detector.
 */
/*Common validate function that wraps lua based validate functions. */
int validateAnyService(
        const uint8_t *data,
        uint16_t size,
        const int dir,
        FLOW *flowp,
        const SFSnortPacket *pkt,
        Detector *detector
        )

{
    int retValue;
    lua_State *myLuaState = NULL;
    const char *serverName;
    PROFILE_VARS;
    PREPROC_PROFILE_START(luaServerPerfStats);

    if (!data || !flowp || !pkt || !detector)
    {
        _dpd.errMsg( "invalid LUA parameters");
        PREPROC_PROFILE_END(luaServerPerfStats);
        return SERVICE_ENULL;
    }

    myLuaState = detector->myLuaState;
    detector->validateParams.data = data;
    detector->validateParams.size = size;
    detector->validateParams.dir = dir;
    detector->validateParams.flowp = flowp;
    detector->validateParams.pkt = pkt;
    serverName = detector->chunkName;

    /*Note: Some frequently used header fields may be extracted and stored in detector for */
    /*better performance. */

    if ((!detector->packageInfo.server.validateFunctionName) || !(lua_checkstack(myLuaState, 1)))
    {
        _dpd.errMsg("server %s: invalid LUA %s\n",serverName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END(luaServerPerfStats);
        return SERVICE_ENULL;
    }

    lua_getglobal(myLuaState, detector->packageInfo.server.validateFunctionName);

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"server %s: Lua Memory usage %d\n",serverName, lua_gc(myLuaState, LUA_GCCOUNT,0));
    _dpd.debugMsg(DEBUG_LOG,"server %s: validating\n",serverName);
#endif
    if (lua_pcall(myLuaState, 0, 1, 0 ))
    {
        /*Runtime Lua errors are suppressed in production code since detectors are written for efficiency */
        /*and with defensive minimum checks. Errors are dealt as exceptions that dont impact processing */
        /*by other detectors or future packets by the same detector. */
        _dpd.errMsg("server %s: error validating %s\n",serverName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END(luaServerPerfStats);
        return SERVICE_ENULL;
    }

    /**detectorFlows must be destroyed after each packet is processed.*/
    sflist_static_free_all(&allocatedFlowList, freeDetectorFlow);

    /* retrieve result */
    if (!lua_isnumber(myLuaState, -1))
    {
        _dpd.errMsg("server %s:  validator returned non-numeric value\n",serverName);
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END(luaServerPerfStats);
        return SERVICE_ENULL;
    }

    retValue = lua_tonumber(myLuaState, -1);
    lua_pop(myLuaState, 1);  /* pop returned value */
    /*lua_settop(myLuaState, 0); */

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"server %s: Validator returned %d\n",serverName, retValue);
#endif

    detector->validateParams.pkt = NULL;

    PREPROC_PROFILE_END(luaServerPerfStats);
    return retValue;
}


/**design: dont store serviceId in detector structure since a single detector
 * can get serviceId for multiple protocols. For example SIP which gets Id for RTP and
 * SIP services.
 */

/**Get service id from database, given service name. Lua detectors call this function at init time
 * get get a service Id (an integer) from database.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param serviceName/stack - Name of service
 * @return int - Number of elements on stack, which is always 1.
 * @return serviceId/stack - serviceId if successful, -1 otherwise.
 */
static int service_getServiceId(
        lua_State *L
        )
{
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, detectorUserData->pDetector->server.serviceId);
    return 1;
}

/**Add port for a given service. Lua detectors call this function to register ports on which a
 * given service is expected to run.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param protocol/stack - protocol type. Values can be {tcp=6, udp=17 }
 * @param port/stack - port number to register.
 * @return int - Number of elements on stack, which is always 1.
 * @return status/stack - 0 if successful, -1 otherwise.
 */
static int service_addPorts(
        lua_State *L
        )
{
    RNAServiceValidationPort pp;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    pp.proto = lua_tonumber(L, 2);
    pp.port = lua_tonumber(L, 3);
    pp.reversed_validation = lua_tonumber(L, 5);
    pp.validate = &validateAnyService;

    if (!detectorUserData || ((pp.proto != IPPROTO_UDP) && (pp.proto != IPPROTO_TCP))
            || !pp.port)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    if (ServiceAddPort(&pp, &detector->server.serviceModule, (void*)detector))
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector->server.pServiceElement->ref_count++;

    lua_pushnumber(L, 0);
    return 1;
}

/**Remove all ports for a given service. Lua detectors call this function to remove ports for this service
 * when exiting. This function is not used currently by any detectors.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1.
 * @return status/stack - 0 if successful, -1 otherwise.
 */
static int service_removePorts(
        lua_State *L
        )
{
    /*RNAServiceValidationFCN validate */
    /*from initAPI */
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detectorRemoveAllPorts(detector);

    lua_pushnumber(L, 0);
    return 1;
}

/**Shared function between Lua API and RNA core.
 */
void detectorRemoveAllPorts (
        Detector *detector
        )
{
    ServiceRemovePorts(&validateAnyService, (void*)detector);
}

/**Set service name. Lua detectors call this function to set service name. It is preferred to set service name
 * when a detector is created. Afterwards there is rarely a need to change service name.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param serviceName/stack - Name of service
 * @return int - Number of elements on stack, which is always 1.
 * @return status/stack - 0 if successful, -1 otherwise.
 */
static int service_setServiceName(
        lua_State *L
        )
{
#if 0
    Detector *detector;
    char *serviceName;
    int retValue = -1;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    serviceName = (char *)lua_tostring(L, 2);

    if (!detectorUserData || !serviceName)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    retValue = storeLuaString(serviceName, (char **)&detector->server.serviceModule.name);

    lua_pushnumber(L, retValue);
    return 1;
#else
    lua_pushnumber(L, 0);
    return 1;
#endif
}

/**Get service name. Lua detectors call this function to get service name. There is
 * rarely a need to change service name.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1.
 * @return serviceName/stack - service name if successful, nil otherwise.
 */
static int service_getServiceName(
        lua_State *L
        )
{
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    lua_pushstring(L, detector->server.serviceModule.name);
    return 1;
}

/**Is this a customer defined detector. Lua detectors can call this function to verify if the detector
 * was created by Sourcefire or not.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1.
 * @return integer/stack - -1 if failed, 0 if sourcefire created, 1 otherwise.
 */
static int service_isCustomDetector(
        lua_State *L
        )
{
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    lua_pushnumber(L, detector->isCustomDetector);
    return 1;
}

/**Set service validator Lua function name. Lua detectors use this function to set a lua function name
 * as service validator function. It is preferred to set validatorname when a detector is created.
 * Afterwards there is rarely a need to change service name.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param validatorName/stack - Name of service validator
 * @return int - Number of elements on stack, which is always 0.
 */
static int service_setValidator(
        lua_State *L
        )
{
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);
    const char *pValidator;

    if (detectorUserData == NULL)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    pValidator = lua_tostring(L, 2);
    lua_getglobal(L, pValidator);
    if (!lua_isfunction(L, -1))
    {
        _dpd.errMsg("%s: attempted setting validator to non-function\n",detector->server.serviceModule.name);
        lua_pop(L, 1);
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pop(L, 1);

    if (storeLuaString(pValidator, &detector->packageInfo.server.validateFunctionName) == -1)
    {
        _dpd.errMsg( "memory allocation failure");
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

/** Add data (validator function name) to a flow. Detector use this function when confirming a flow
 * belongs to this service.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param sourcePort/stack - Source port number.
 * @return int - Number of elements on stack, which is always 0.
 */
static int service_addDataId(
        lua_State *L
        )
{
    Detector *detector;
    uint16_t sport;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    sport = lua_tonumber(L, 2);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !checkServiceElement(detectorUserData->pDetector)
            || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    AppIdFlowdataAddId(detector->validateParams.flowp, sport, detector->server.pServiceElement);

    lua_pushnumber(L, 0);
    return 1;
}

/** Add service id to a flow. Positive identification by a detector.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param serviceId/stack - id of service postively identified on this flow.
 * @param vendorName/stack - name of vendor of service. This is optional.
 * @param version/stack - version of service. This is optional.
 * @return int - Number of elements on stack, which is always 1.
 * @return int/stack - values from enum SERVICE_RETCODE
 */
static int service_addService(
        lua_State *L
        )
{
    char *vendor, *version;
    unsigned int serviceId, retValue = SERVICE_ENULL;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    serviceId = lua_tonumber(L, 2);
    vendor = (char *)luaL_optstring(L, 3, NULL);
    version = (char *)luaL_optstring(L, 4, NULL);

    /*check inputs (vendor and version may be null) and whether this function is */
    /*called in context of a packet */
    if (!detectorUserData || !checkServiceElement(detectorUserData->pDetector)
           || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, SERVICE_ENULL);
        return 1;
    }

    detector = detectorUserData->pDetector;

    /*Phase2 - discuss RNAServiceSubtype will be maintained on lua side therefore the last parameter on the following call is NULL. */
    /*Subtype is not displayed on DC at present. */
    retValue = AppIdServiceAddService(detector->validateParams.flowp, detector->validateParams.pkt,
            detector->validateParams.dir, detector->server.pServiceElement,
            appGetAppFromServiceId(serviceId), vendor, version, NULL);

    lua_pushnumber(L, retValue);
    return 1;
}

/**Function confirms the flow is not running this service.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1.
 * @return int/stack - values from enum SERVICE_RETCODE
 */
static int service_failService(
        lua_State *L
        )
{
    unsigned int retValue = SERVICE_ENULL;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs (vendor and version may be null) and whether this function is */
    /*called in context of a packet */
    if (!detectorUserData || !checkServiceElement(detectorUserData->pDetector)
           || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, SERVICE_ENULL);
        return 1;
    }

    detector = detectorUserData->pDetector;

    retValue = AppIdServiceFailService(detector->validateParams.flowp, detector->validateParams.pkt,
                                       detector->validateParams.dir, detector->server.pServiceElement);

    lua_pushnumber(L, retValue);
    return 1;

}

/**Detector use this function to indicate the flow may belong to this flow.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1.
 * @return int/stack - values from enum SERVICE_RETCODE
 */
static int service_inProcessService(
        lua_State *L
        )
{
    unsigned int retValue = -1;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs (vendor and version may be null) and whether this function is */
    /*called in context of a packet */
    if (!detectorUserData || !checkServiceElement(detectorUserData->pDetector)
           || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, SERVICE_ENULL);
        return 1;
    }

    detector = detectorUserData->pDetector;

    retValue = AppIdServiceInProcess(detector->validateParams.flowp, detector->validateParams.pkt,
                                     detector->validateParams.dir, detector->server.pServiceElement);

    lua_pushnumber(L, retValue);
    return 1;
}

/**Detector use this function to indicate error in service indentification.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1.
 * @return int/stack - values from enum SERVICE_RETCODE
 */
static int service_inCompatibleData(
        lua_State *L
        )
{
    unsigned int retValue = SERVICE_ENULL;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs (vendor and version may be null) and whether this function is */
    /*called in context of a packet */
    if (!detectorUserData || !checkServiceElement(detectorUserData->pDetector)
           || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, SERVICE_ENULL);
        return 1;
    }

    detector = detectorUserData->pDetector;

    retValue = AppIdServiceIncompatibleData(detector->validateParams.flowp, detector->validateParams.pkt,
                                            detector->validateParams.dir, detector->server.pServiceElement);

    lua_pushnumber(L, retValue);
    return 1;
}

/** Get size of current packet. It should be noted that due to restrictions on sharing pointers
 * between C and Lua, packet data is maintained on C side. Lua side can get specific fields, run
 * memcmp and pattern matching on packet data.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1 if successful, 0 otherwise.
 * @return packetSize/stack - size of packet on stack, if successful.
 */
static int Detector_getPacketSize(
        lua_State *L
        )
{
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    lua_pushnumber(L, detector->validateParams.size);
    return 1;
}

/**Get packet direction. A flow/session maintains initiater and responder sides. A packet direction
 * is determined wrt to the original initiater.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is always 1 if successful, 0 otherwise.
 * @return packetDir/stack - direction of packet on stack, if successful.
 */
static int Detector_getPacketDir(
        lua_State *L
        )
{
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        /*can not return 0 in case of error, since 0 can be a valid value. */
        return 0;
    }

    detector = detectorUserData->pDetector;

    lua_pushnumber(L, detector->validateParams.dir);
    return 1;
}

/**Perform a pcre match with grouping. A simple regular expression match with no grouping
 * can also be performed.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of group matches.  May be 0 or more.
 * @return matchedStrings/stack - matched strings are pushed on stack starting with group 0.
 *     There may be 0 or more strings.
 */
static int Detector_getPcreGroups(
        lua_State *L
        )
{
    Detector *detector;
    char *pattern;
    unsigned int offset;
    pcre *re;
    int ovector[OVECCOUNT];
    const char *error;
    int erroffset;
    int rc, i;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    pattern = (char *)lua_tostring(L, 2);
    offset = lua_tonumber(L, 3);     /*offset can be zero, no check necessary. */

    if ((pattern == NULL) || (detectorUserData == NULL))
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    {
        /*compile the regular expression pattern, and handle errors */
        re = pcre_compile(
                pattern,          /*the pattern */
                PCRE_DOTALL,      /*default options - dot matches everything including newline */
                &error,           /*for error message */
                &erroffset,       /*for error offset */
                NULL);            /*use default character tables */

        if (re == NULL)
        {
            _dpd.errMsg("PCRE compilation failed at offset %d: %s\n",erroffset, error);
            return 0;
        }


        /*pattern match against the subject string. */
        rc = pcre_exec(
                re,                                     /*compiled pattern */
                NULL,                                   /*no extra data */
                (char *)detector->validateParams.data,  /*subject string */
                detector->validateParams.size,          /*length of the subject */
                offset,                                 /*offset 0 */
                0,                                      /*default options */
                ovector,                                /*output vector for substring information */
                OVECCOUNT);                             /*number of elements in the output vector */


        if (rc < 0)
        {
            /*Matching failed: clubbing PCRE_ERROR_NOMATCH with other errors. */
            pcre_free(re);
            return 0;
        }

        /*Match succeded */

        /*printf("\nMatch succeeded at offset %d", ovector[0]); */
        pcre_free(re);


        if (rc == 0)
        {
            /*overflow of matches */
            rc = OVECCOUNT/3;
            /*printf("ovector only has room for %d captured substrings", rc - 1); */
            _dpd.errMsg("ovector only has room for %d captured substrings\n",rc - 1);
        }
    }



    lua_checkstack (L, rc);
    for (i = 0; i < rc; i++)
    {
        /*printf("%2d: %.*s\n", i, , substring_start); */
        lua_pushlstring(L, (char *)detector->validateParams.data + ovector[2*i], ovector[2*i+1] - ovector[2*i]);
    }

    return rc;
}

/**Performs a simple memory comparison.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param pattern/stack - pattern to be matched.
 * @param patternLenght/stack - length of pattern
 * @param offset/stack - offset into packet payload where matching should start.
 *
 * @return int - Number of group matches.  May be 1 if successful, and 0 if error is encountered.
 * @return memCmpResult/stack - returns -1,0,1 based on memcmp result.
 */
static int Detector_memcmp(
        lua_State *L
        )
{
    Detector *detector;
    char *pattern;
    unsigned int patternLen;
    unsigned int offset;
    int rc;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    pattern = (char *)lua_tostring(L, 2);
    patternLen = lua_tonumber(L, 3);
    offset = lua_tonumber(L, 4);     /*offset can be zero, no check necessary. */

    if ((detectorUserData == NULL) || (pattern == NULL))
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    rc = memcmp((char *)detector->validateParams.data + offset, pattern, patternLen);

    lua_checkstack (L, 1);
    lua_pushnumber(L, rc);
    return 1;
}

/**Get Packet Protocol Type
 *
 * @param Lua_State* - Lua state variable.
 * @return int - Number of elements on stack, which is protocol type if successful, 0 otherwise.
 * @return protocol type TCP or UDP
 */
static int Detector_getProtocolType (
        lua_State *L
        )
{
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt || !IPH_IS_VALID(detectorUserData->pDetector->validateParams.pkt))
    {
        lua_checkstack (L, 1);
        lua_pushnumber(L, 0);
        return 1;
    }

    detector = detectorUserData->pDetector;

    lua_checkstack (L, 1);
    lua_pushnumber(L, GET_IPH_PROTO(detector->validateParams.pkt));
    return 1;
}

/**Get source IP address from IP header.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return IPv4/stack - Source IPv4 addresss.
 */
static int Detector_getPktSrcIPAddr(
        lua_State *L
        )
{
    Detector *detector;
    snort_ip *ipAddr;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    ipAddr = GET_SRC_IP(detector->validateParams.pkt);

    lua_checkstack (L, 1);
    lua_pushnumber(L, ipAddr->ip32[0]);
    return 1;
}

/**Get source port number from IP header.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return portNumber/stack - source port number.
 */
static int Detector_getPktSrcPort(
        lua_State *L
        )
{
    Detector *detector;
    unsigned int port;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    port = detector->validateParams.pkt->src_port;

    lua_checkstack (L, 1);
    lua_pushnumber(L, port);
    return 1;
}

/**Get destination port number from IP header.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return portNumber/stack - destination Port number.
 */
static int Detector_getPktDstPort(
        lua_State *L
        )
{
    Detector *detector;
    unsigned int port;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    port = detector->validateParams.pkt->dst_port;

    lua_checkstack (L, 1);
    lua_pushnumber(L, port);
    return 1;
}

/**Get destination IP address from IP header.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return IPv4/stack - destination IPv4 addresss.
 */
static int Detector_getPktDstIPAddr(
        lua_State *L
        )
{
    Detector *detector;
    snort_ip *ipAddr;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    ipAddr = GET_DST_IP(detector->validateParams.pkt);

    lua_checkstack (L, 1);
    lua_pushnumber(L, ipAddr->ip32[0]);
    return 1;
}


/**Get packet count. This is used mostly for printing packet sequence
 * number when RNA is being tested with a pcap file.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return packetCount/stack - Total packet processed by RNA.
 */
static int Detector_getPktCount(
        lua_State *L
        )
{
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    lua_checkstack (L, 1);
    lua_pushnumber(L, app_id_processed_packet_count);
    return 1;
}

int validateAnyClientApp(
        const uint8_t *data,
        uint16_t size,
        const int dir,
        FLOW *flowp,
        const SFSnortPacket *pkt,
        Detector *detector
        )

{
    int retValue;
    lua_State *myLuaState;
    char *validateFn;
    char *clientName;
    PROFILE_VARS;
    PREPROC_PROFILE_START(luaClientPerfStats);

    if (!data || !flowp || !pkt || !detector)
    {
        PREPROC_PROFILE_END(luaClientPerfStats);
        return CLIENT_APP_ENULL;
    }

    myLuaState = detector->myLuaState;
    detector->validateParams.data = data;
    detector->validateParams.size = size;
    detector->validateParams.dir = dir;
    detector->validateParams.flowp = flowp;
    detector->validateParams.pkt = (SFSnortPacket *)pkt;
    validateFn = detector->packageInfo.client.validateFunctionName;
    clientName = detector->chunkName;

    if ((!validateFn) || !(lua_checkstack(myLuaState, 1)))
    {
        _dpd.errMsg("client %s: invalid LUA %s\n",clientName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END(luaClientPerfStats);
        return CLIENT_APP_ENULL;
    }

    lua_getglobal(myLuaState, validateFn);

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"client %s: Lua Memory usage %d\n",clientName, lua_gc(myLuaState, LUA_GCCOUNT,0));
    _dpd.debugMsg(DEBUG_LOG,"client %s: validating\n",clientName);
#endif
    if (lua_pcall(myLuaState, 0, 1, 0 ))
    {
        _dpd.errMsg("client %s: error validating %s\n",clientName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END(luaClientPerfStats);
        return SERVICE_ENULL;
    }

    /**detectorFlows must be destroyed after each packet is processed.*/
    sflist_static_free_all(&allocatedFlowList, freeDetectorFlow);

    /* retrieve result */
    if (!lua_isnumber(myLuaState, -1))
    {
        _dpd.errMsg("client %s:  validator returned non-numeric value\n",clientName);
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END(luaClientPerfStats);
        retValue = SERVICE_ENULL;
    }

    retValue = lua_tonumber(myLuaState, -1);
    lua_pop(myLuaState, 1);  /* pop returned value */
    /*lua_settop(myLuaState, 0); */

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"client %s: Validator returned %d\n",clientName, retValue);
#endif

    detector->validateParams.pkt = NULL;

    PREPROC_PROFILE_END(luaClientPerfStats);
    return retValue;
}

static int client_registerPattern(lua_State *L)
{
    int protocol;
    size_t size;
    const char *pattern;
    unsigned int position;
    Detector *detector;
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    protocol = lua_tonumber(L, index++);
    pattern = lua_tostring(L, index++);
    size = lua_tonumber(L, index++);
    position = lua_tonumber(L, index++);

    if ((pattern == NULL) || (detectorUserData == NULL))
    {
        lua_pushnumber(L, -1);
        return 1;   /*number of results */
    }

    detector = detectorUserData->pDetector;

    /*Note: we can not give callback into lua directly so we have to */
    /*give a local callback function, which will do demuxing and */
    /*then call lua callback function. */

    /*mpse library does not hold reference to pattern therefore we dont need to allocate it. */

    detector->client.appModule.userData = detector;
    clientAppLoadCallback((void *) &(detector->client.appModule));
    ClientAppRegisterPattern(validateAnyClientApp, protocol, (uint8_t*)pattern, size, position, 0, (void *)detector);

    lua_pushnumber(L, 0);
    return 1;   /*number of results */
}

/**Creates a new detector instance. Creates a new detector instance and leaves the instance
 * on stack. This is the first call by a lua detector to create and instance. Later calls
 * provide the detector instance.
 *
 * @param Lua_State* - Lua state variable.
 * @param serviceName/stack - name of service
 * @param pValidator/stack - service validator function name
 * @param pFini/stack - service clean exit function name
 * @return int - Number of elements on stack, which should be 1 if success 0 otherwise.
 * @return detector - a detector instance on stack if successful
 */

static int client_init (lua_State *L)
{
    /*nothing to do */
    return 0;
}

static int client_addApp(
        lua_State *L
        )
{
    unsigned int serviceId, productId;
    const char *version;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    serviceId = lua_tonumber(L, 2);
    productId = lua_tonumber(L, 4);
    version = lua_tostring(L, 5);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt
            || !version)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->client.appModule.api->add_app(detector->validateParams.flowp,
             appGetAppFromServiceId(serviceId), appGetAppFromClientId(productId), version);

    lua_pushnumber(L, 0);
    return 1;
}

static int client_addInfo(
        lua_State *L
        )
{
    const char *info;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    info = lua_tostring(L, 2);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt
            || !info)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->client.appModule.api->add_info(detector->validateParams.flowp, info);

    lua_pushnumber(L, 0);
    return 1;
}

static int client_addUser(
        lua_State *L
        )
{
    unsigned int serviceId;
    const char *userName;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    userName = lua_tostring(L, 2);
    serviceId = lua_tonumber(L, 3);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt || !userName)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->client.appModule.api->add_user(detector->validateParams.flowp, userName, appGetAppFromServiceId(serviceId), 1);

    lua_pushnumber(L, 0);
    return 1;
}

static int client_addPayload(
        lua_State *L
        )
{
    unsigned int payloadId;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    payloadId = lua_tonumber(L, 2);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->client.appModule.api->add_payload(detector->validateParams.flowp, appGetAppFromServiceId(payloadId));

    lua_pushnumber(L, 0);
    return 1;
}

/**Get flow object from a detector object. The flow object is then used with flowApi.
 * A new copy of flow object is provided with every call. This can be optimized by maintaining
 * a single copy.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return packetCount/stack - Total packet processed by RNA.
 * @todo maintain a single copy and return the same copy with every call to Detector_getFlow().
 */
static int Detector_getFlow(
        lua_State *L
        )
{
    DetectorFlowUserData *detectorFlowUserData;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    detectorFlowUserData = pushDetectorFlowUserData(L);
    if (!detectorFlowUserData || !detectorFlowUserData->pDetectorFlow)
    {
        _dpd.errMsg( "Failed to allocate memory.");
        return 0;
    }

    detectorFlowUserData->pDetectorFlow->pFlow = detector->validateParams.flowp;

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG, "service %s, Detector_getFlow(): allocated DetectorFlow = %p",
            detector->server.serviceModule.name, detectorFlowUserData);
#endif
    return 1;
}

int Detector_addHttpPattern(lua_State *L)
{
    int index = 1;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "Invalid HTTP detector user data addHttpPattern.");
        return 0;
    }

    /* Verify valid pattern type */
    enum httpPatternType pType = (enum httpPatternType) lua_tointeger(L, index++);
    if(pType < HTTP_PAYLOAD || pType > HTTP_URL)
    {
        _dpd.errMsg( "Invalid HTTP pattern type.");
        return 0;
    }

    /* Verify valid DHSequence */
    DHPSequence seq  = (DHPSequence) lua_tointeger(L, index++);
    if(seq < SINGLE || seq > USER_AGENT_HEADER)
    {
        _dpd.errMsg( "Invalid HTTP DHP Sequence.");
        return 0;
    }

    uint32_t service_id      = lua_tointeger(L, index++);
    uint32_t client_app      = lua_tointeger(L, index++);
    /*uint32_t client_app_type =*/ lua_tointeger(L, index++);
    uint32_t payload         = lua_tointeger(L, index++);
    /*uint32_t payload_type    =*/ lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid detector context addHttpPattern: service_id %u; client_app %u; payload %u\n",service_id, client_app, payload);
        return 0;
    }

    /* Verify that pattern is a valid string */
    size_t pattern_size = 0;
    uint8_t* pattern_str = (uint8_t*) strdup(lua_tolstring(L, index++, &pattern_size));
    if(pattern_str == NULL || pattern_size == 0)
    {
        _dpd.errMsg( "Invalid HTTP pattern string.");
        free(pattern_str);
        return 0;
    }

    uint32_t appId = lua_tointeger(L, index++);

    HTTPListElement *element = calloc(1, sizeof(*element));
    if (element == NULL)
    {
        _dpd.errMsg( "Failed to allocate HTTP list element memory.");
        free(pattern_str);
        return 0;
    }

    DetectorHTTPPattern *pattern = &element->detectorHTTPPattern;
    pattern->seq           = seq;
    pattern->service_id    = appGetAppFromServiceId(service_id);
    pattern->client_app    = appGetAppFromClientId(client_app);
    pattern->payload       = appGetAppFromPayloadId(payload);
    pattern->pattern       = pattern_str;
    pattern->pattern_size  = (int) pattern_size;
    pattern->appId         = appId;

    /* for apps that should not show up in 4.10 and ealier, we cannot include an entry in
       the legacy client app or payload tables. We will use the appId instead. This is only for
       user-agents that ID clients. if you want a user-agent to ID a payload, include it in the
       payload database. If you want a host pattern ID, use the other API.  */

    if (!service_id && !client_app && !payload && pType == 2)
    {
        pattern->client_app = appId;
    }

    switch(pType)
    {
        case HTTP_PAYLOAD:
             element->next = httpPatternLists.hostPayloadPatternList;
             httpPatternLists.hostPayloadPatternList = element;
             break;

        case HTTP_URL:
             element->next = httpPatternLists.urlPatternList;
             httpPatternLists.urlPatternList = element;
             break;

        case HTTP_USER_AGENT:
             element->next = httpPatternLists.clientAgentPatternList;
             httpPatternLists.clientAgentPatternList = element;
             break;
    }

    appInfoSetActive(pattern->service_id, true);
    appInfoSetActive(pattern->client_app, true);
    appInfoSetActive(pattern->payload, true);
    appInfoSetActive(appId, true);

    return 0;
}

//#define SSL_PATTERN_NOT_SUPPORTED

/*  On the lua side, this should look something like:
        addSSLCertPattern(<appId>, '<pattern string>' )
*/
int Detector_addSSLCertPattern (lua_State *L)
{
    uint8_t *pattern_str;
    size_t pattern_size;
    int index = 1;
    uint8_t type;
    tAppId app_id;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "Invalid SSL detector user data or context.");
        return 0;
    }

    type = lua_tointeger(L, index++);
    app_id  = (tAppId) lua_tointeger(L, index++);

#ifdef SSL_PATTERN_NOT_SUPPORTED
    /*SSL patterns not supported */
    /*_dpd.errMsg("SSL patterns not supported: %d\n",app_id); */
    return 0;
#endif

    pattern_size = 0;
    const char *tmpString = lua_tolstring(L, index++, &pattern_size);
    if (!tmpString || !pattern_size)
    {
        _dpd.errMsg( "Invalid SSL Host pattern string");
        return 0;
    }
    pattern_str = (uint8_t *)strdup(tmpString);
    if (!pattern_str)
    {
        _dpd.errMsg( "Invalid SSL Host pattern string.");
        return 0;
    }

    if (!ssl_add_cert_pattern(pattern_str, pattern_size, type, app_id))
    {
        free(pattern_str);
        _dpd.errMsg( "Failed to add an SSL pattern list member");
    }

    return 0;
}

static int Detector_addSSLCnamePattern (lua_State *L)
{
    uint8_t *pattern_str;
    size_t pattern_size;
    int index = 1;
    uint8_t type;
    tAppId app_id;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "Invalid SSL detector user data or context.");
        return 0;
    }

    type = lua_tointeger(L, index++);
    app_id  = (tAppId) lua_tointeger(L, index++);

#ifdef SSL_PATTERN_NOT_SUPPORTED
    /*SSL patterns not supported */
    _dpd.errMsg("SSL patterns not supported: %d\n",app_id);
    return 0;
#endif

    pattern_size = 0;
    const char *tmpString = lua_tolstring(L, index++, &pattern_size);
    if (!tmpString || !pattern_size)
    {
        _dpd.errMsg( "Invalid SSL Host pattern string");
        return 0;
    }
    pattern_str = (uint8_t *)strdup(tmpString);
    if (!pattern_str)
    {
        _dpd.errMsg( "Invalid SSL Host pattern string.");
        return 0;
    }

    if (!ssl_add_cname_pattern(pattern_str, pattern_size, type, app_id))
    {
        free(pattern_str);
        _dpd.errMsg( "Failed to add an SSL pattern list member");
    }

    return 0;
}

static int Detector_addHostPortApp (lua_State *L)
{
    /*uint8_t *ipaddr_str; */
    size_t ipaddr_size;
    int index = 1;
    uint8_t type;
    tAppId app_id;
    struct in6_addr ip6Addr;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n",__func__);
        return 0;
    }

    type = lua_tointeger(L, index++);
    app_id  = (tAppId) lua_tointeger(L, index++);

    ipaddr_size = 0;
    const char *tmpString = lua_tolstring(L, index++, &ipaddr_size);
    if (!tmpString || !ipaddr_size)
    {
        _dpd.errMsg("%s:Invalid ipaddr string\n",__func__);
        return 0;
    }
    if (!strchr(tmpString, ':'))
    {
        if (inet_pton(AF_INET, tmpString, &ip6Addr.s6_addr32[3]) <= 0)
        {
            _dpd.errMsg("%s: Invalid IP address: %s\n",__func__, tmpString);
            return 0;
        }
        ip6Addr.s6_addr32[0] = ip6Addr.s6_addr32[1] =  0;
        ip6Addr.s6_addr32[2] = ntohl(0x0000ffff);
    }
    else
    {
        if (inet_pton(AF_INET6, tmpString, &ip6Addr) <= 0)
        {
            _dpd.errMsg("%s: Invalid IP address: %s\n",__func__, tmpString);
            return 0;
        }
    }
    unsigned port  = lua_tointeger(L, index++);
    unsigned proto  = lua_tointeger(L, index++);

    if (!hostPortAppCacheAdd(&ip6Addr, (uint16_t)port, (uint16_t)proto, type, app_id))
    {
        _dpd.errMsg("%s:Failed to backend call\n",__func__);
    }

    return 0;
}

static int Detector_addContentTypePattern(lua_State *L)
{
    uint8_t *pattern;
    tAppId appId;
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "Invalid HTTP detector user data addContentTypePattern.");
        return 0;
    }

    size_t stringSize = 0;
    const char *tmpString = lua_tolstring(L, index++, &stringSize);
    if (!tmpString || !stringSize)
    {
        _dpd.errMsg( "Invalid HTTP Header string");
        return 0;
    }
    pattern = (uint8_t *)strdup(tmpString);
    if (!pattern)
    {
        _dpd.errMsg( "Failed to allocate Content Type pattern string.");
        return 0;
    }

    appId = lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid detector context addSipUserAgent: appId %d\n",appId);
        return 0;
    }

    HTTPListElement *element = calloc(1, sizeof(*element));
    if (!element)
    {
        _dpd.errMsg( "Failed to allocate HTTP list element memory.");
        free(pattern);
        return 0;
    }

    DetectorHTTPPattern *detector = &element->detectorHTTPPattern;
    detector->pattern = pattern;
    detector->pattern_size = strlen((char *)pattern);
    detector->appId = appId;

    element->next = httpPatternLists.contentTypePatternList;
    httpPatternLists.contentTypePatternList = element;

    appInfoSetActive(appId, true);

    return 0;
}

static int Detector_portOnlyService (lua_State *L)
{
    int index = 1;

    // Verify detector user data and that we are not in packet context
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid detector user data in portOnlyService.");
        return 0;
    }

    tAppId appId = lua_tointeger(L, index++);
    u_int16_t port = lua_tointeger(L, index++);
    u_int8_t protocol = lua_tointeger(L, index++);

    if (port == 0)
        appIdConfig.ip_protocol[protocol] = appId;
    else if (protocol == 6)
        appIdConfig.tcp_port_only[port] = appId;
    else if (protocol == 17)
        appIdConfig.udp_port_only[port] = appId;

    return 0;
}

static int Detector_addAppUrl(lua_State *L)
{
    int index = 1;
    DetectorAppUrlPattern **tmp;
    const char *tmpString;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "Invalid HTTP detector user data in addAppUrl.");
        return 0;
    }

    u_int32_t service_id      = lua_tointeger(L, index++);
    u_int32_t client_app      = lua_tointeger(L, index++);
    /*u_int32_t client_app_type =*/ lua_tointeger(L, index++);
    u_int32_t payload         = lua_tointeger(L, index++);
    /*u_int32_t payload_type    =*/ lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid HTTP detector context addAppUrl: service_id %u; client_app %u; payload %u\n",service_id, client_app, payload);
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t hostPatternSize = 0;
    u_int8_t* hostPattern = NULL;
    tmpString = lua_tolstring(L, index++, &hostPatternSize);
    if(!tmpString || !hostPatternSize || !(hostPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid host pattern string.");
        return 0;
    }

    /* Verify that path pattern is a valid string */
    size_t pathPatternSize = 0;
    u_int8_t* pathPattern = NULL;
    tmpString = lua_tolstring(L, index++, &pathPatternSize);
    if(!tmpString || !pathPatternSize || !(pathPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid path pattern string.");
        free(hostPattern);
        return 0;
    }

    /* Verify that scheme pattern is a valid string */
    size_t schemePatternSize;
    u_int8_t* schemePattern = NULL;
    tmpString = lua_tolstring(L, index++, &schemePatternSize);
    if(!tmpString || !schemePatternSize || !(schemePattern = (u_int8_t*) strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid scheme pattern string.");
        free(pathPattern);
        free(hostPattern);
        return 0;
    }

    /* Verify that query pattern is a valid string */
    size_t queryPatternSize;
    u_int8_t* queryPattern = NULL;
    tmpString = lua_tolstring(L, index++, &queryPatternSize);
    if(tmpString  && queryPatternSize)
    {
        if (!(queryPattern = (u_int8_t*) strdup(tmpString)))
        {
            _dpd.errMsg( "Invalid query pattern string.");
            free(hostPattern);
            free(pathPattern);
            free(schemePattern);
            return 0;
        }
    }

    u_int32_t appId           = lua_tointeger(L, index++);

    /* Allocate memory for data structures */
    DetectorAppUrlPattern *pattern = malloc(sizeof(DetectorAppUrlPattern));
    if (!pattern)
    {
        _dpd.errMsg( "Failed to allocate HTTP pattern memory.");
        free(hostPattern);
        free(pathPattern);
        free(schemePattern);
        if (queryPattern) free(queryPattern);
        return 0;
    }


    pattern->userData.service_id        = appGetAppFromServiceId(service_id);
    pattern->userData.client_app        = appGetAppFromClientId(client_app);
    pattern->userData.payload           = appGetAppFromPayloadId(payload);
    pattern->userData.appId             = appId;
    pattern->userData.query.pattern     = queryPattern;
    pattern->userData.query.patternSize = queryPatternSize;
    pattern->patterns.host.pattern              = hostPattern;
    pattern->patterns.host.patternSize         = (int) hostPatternSize;
    pattern->patterns.path.pattern              = pathPattern;
    pattern->patterns.path.patternSize         = (int) pathPatternSize;
    pattern->patterns.scheme.pattern              = schemePattern;
    pattern->patterns.scheme.patternSize         = (int) schemePatternSize;

    DetectorAppUrlList *urlList = &httpPatternLists.appUrlList;

    /**first time usedCount and allocatedCount are both 0, urlPattern will be NULL.
     * This case is same as malloc. In case of error, realloc will return NULL, and
     * original urlPattern buffer is left untouched.
     */
    if (urlList->usedCount == urlList->allocatedCount)
    {
        tmp = realloc(urlList->urlPattern, (urlList->allocatedCount+URL_LIST_STEP_SIZE)*sizeof(*tmp));
        if (!tmp)
        {
            FreeDetectorAppUrlPattern(pattern);
            return 0;
        }
        urlList->urlPattern = tmp;
        urlList->allocatedCount += URL_LIST_STEP_SIZE;
    }

    urlList->urlPattern[urlList->usedCount++] = pattern;

    appInfoSetActive(pattern->userData.service_id, true);
    appInfoSetActive(pattern->userData.client_app, true);
    appInfoSetActive(pattern->userData.payload, true);
    appInfoSetActive(appId, true);

    return 0;
}

static int Detector_addRTMPUrl(lua_State *L)
{
    int index = 1;
    DetectorAppUrlPattern **tmp;
    const char *tmpString;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "Invalid HTTP detector user data in addRTMPUrl.");
        return 0;
    }

    u_int32_t service_id      = lua_tointeger(L, index++);
    u_int32_t client_app      = lua_tointeger(L, index++);
    /*u_int32_t client_app_type =*/ lua_tointeger(L, index++);
    u_int32_t payload         = lua_tointeger(L, index++);
    /*u_int32_t payload_type    =*/ lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid HTTP detector context addRTMPUrl: service_id %u; client_app %u; payload %u\n",service_id, client_app, payload);
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t hostPatternSize = 0;
    u_int8_t* hostPattern = NULL;
    tmpString = lua_tolstring(L, index++, &hostPatternSize);
    if(!tmpString || !hostPatternSize || !(hostPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid host pattern string.");
        return 0;
    }

    /* Verify that path pattern is a valid string */
    size_t pathPatternSize = 0;
    u_int8_t* pathPattern = NULL;
    tmpString = lua_tolstring(L, index++, &pathPatternSize);
    if(!tmpString || !pathPatternSize || !(pathPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid path pattern string.");
        free(hostPattern);
        return 0;
    }

    /* Verify that scheme pattern is a valid string */
    size_t schemePatternSize;
    u_int8_t* schemePattern = NULL;
    tmpString = lua_tolstring(L, index++, &schemePatternSize);
    if(!tmpString || !schemePatternSize || !(schemePattern = (u_int8_t*) strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid scheme pattern string.");
        free(pathPattern);
        free(hostPattern);
        return 0;
    }

    /* Verify that query pattern is a valid string */
    size_t queryPatternSize;
    u_int8_t* queryPattern = NULL;
    tmpString = lua_tolstring(L, index++, &queryPatternSize);
    if(tmpString  && queryPatternSize)
    {
        if (!(queryPattern = (u_int8_t*) strdup(tmpString)))
        {
            _dpd.errMsg( "Invalid query pattern string.");
            free(hostPattern);
            free(pathPattern);
            free(schemePattern);
            return 0;
        }
    }

    u_int32_t appId           = lua_tointeger(L, index++);

    /* Allocate memory for data structures */
    DetectorAppUrlPattern *pattern = malloc(sizeof(DetectorAppUrlPattern));
    if (!pattern)
    {
        _dpd.errMsg( "Failed to allocate HTTP pattern memory.");
        free(hostPattern);
        free(pathPattern);
        free(schemePattern);
        if (queryPattern) free(queryPattern);
        return 0;
    }

    /* we want to put these patterns in just like for regular Urls, but we do NOT need legacy IDs for them.
     * so just use the appID for service, client, or payload ID */
    pattern->userData.service_id        = service_id;
    pattern->userData.client_app        = client_app;
    pattern->userData.payload           = payload;
    pattern->userData.appId             = appId;
    pattern->userData.query.pattern     = queryPattern;
    pattern->userData.query.patternSize = queryPatternSize;
    pattern->patterns.host.pattern              = hostPattern;
    pattern->patterns.host.patternSize         = (int) hostPatternSize;
    pattern->patterns.path.pattern              = pathPattern;
    pattern->patterns.path.patternSize         = (int) pathPatternSize;
    pattern->patterns.scheme.pattern              = schemePattern;
    pattern->patterns.scheme.patternSize         = (int) schemePatternSize;

    DetectorAppUrlList *urlList = &httpPatternLists.RTMPUrlList;

    /**first time usedCount and allocatedCount are both 0, urlPattern will be NULL.
     * This case is same as malloc. In case of error, realloc will return NULL, and
     * original urlPattern buffer is left untouched.
     */
    if (urlList->usedCount == urlList->allocatedCount)
    {
        tmp = realloc(urlList->urlPattern, (urlList->allocatedCount+URL_LIST_STEP_SIZE)*sizeof(*tmp));
        if (!tmp)
        {
            FreeDetectorAppUrlPattern(pattern);
            return 0;
        }
        urlList->urlPattern = tmp;
        urlList->allocatedCount += URL_LIST_STEP_SIZE;
    }

    urlList->urlPattern[urlList->usedCount++] = pattern;

    appInfoSetActive(pattern->userData.service_id, true);
    appInfoSetActive(pattern->userData.client_app, true);
    appInfoSetActive(pattern->userData.payload, true);
    appInfoSetActive(appId, true);

    return 0;
}

/*Lua should inject patterns in <clientAppId, clientVersion, multi-Pattern> format. */
static int Detector_addSipUserAgent(lua_State *L)
{
    int index = 1;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "Invalid HTTP detector user data addSipUserAgent.");
        return 0;
    }

    u_int32_t client_app      = lua_tointeger(L, index++);
    const char *clientVersion       = lua_tostring(L, index++);
    if(!clientVersion )
    {
        _dpd.errMsg( "Invalid sip client version string.");
        return 0;
    }

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid detector context addSipUserAgent: client_app %u\n",client_app);
        return 0;
    }

    /* Verify that ua pattern is a valid string */
    const char* uaPattern = lua_tostring(L, index++);
    if(!uaPattern)
    {
        _dpd.errMsg( "Invalid sip ua pattern string.");
        return 0;
    }

    sipUaPatternAdd(client_app, clientVersion, uaPattern);

    appInfoSetActive(client_app, true);

    return 0;
}

static int openCreateApp(lua_State *L)
{
    int index = 1;
    const char *tmpString;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "Invalid HTTP detector user data in addAppUrl.");
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t appNameLen = 0;
    tmpString = lua_tolstring(L, index++, &appNameLen);
    if(!tmpString || !appNameLen)
    {
        _dpd.errMsg( "Invalid appName string.");
        lua_pushnumber(L, APP_ID_NONE);
        return 1;   /*number of results */
    }

    AppInfoTableEntry *entry = createAppInfoEntry(tmpString);

    if (entry)
    {
        lua_pushnumber(L, entry->appId);
        return 1;   /*number of results */
    }

    lua_pushnumber(L, APP_ID_NONE);
    return 1;   /*number of results */
}

static int openAddClientApp(
        lua_State *L
        )
{
    unsigned int serviceAppId, clientAppId;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    serviceAppId = lua_tonumber(L, 2);
    clientAppId = lua_tonumber(L, 3);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->client.appModule.api->add_app(detector->validateParams.flowp, serviceAppId, clientAppId, "");

    lua_pushnumber(L, 0);
    return 1;
}

/** Add service id to a flow. Positive identification by a detector.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @param serviceId/stack - id of service postively identified on this flow.
 * @param vendorName/stack - name of vendor of service. This is optional.
 * @param version/stack - version of service. This is optional.
 * @return int - Number of elements on stack, which is always 1.
 * @return int/stack - values from enum SERVICE_RETCODE
 */
static int openAddServiceApp(
        lua_State *L
        )
{
    unsigned int serviceId, retValue = SERVICE_ENULL;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    serviceId = lua_tonumber(L, 2);

    if (!detectorUserData || !checkServiceElement(detectorUserData->pDetector)
           || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, SERVICE_ENULL);
        return 1;
    }

    detector = detectorUserData->pDetector;

    /*Phase2 - discuss RNAServiceSubtype will be maintained on lua side therefore the last parameter on the following call is NULL. */
    /*Subtype is not displayed on DC at present. */
    retValue = AppIdServiceAddService(detector->validateParams.flowp, detector->validateParams.pkt,
            detector->validateParams.dir, detector->server.pServiceElement,
            serviceId, NULL, NULL, NULL);

    lua_pushnumber(L, retValue);
    return 1;
}

static int openAddPayloadApp(
        lua_State *L
        )
{
    unsigned int payloadAppId;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    payloadAppId = lua_tonumber(L, 2);

    /*check inputs and whether this function is called in context of a */
    /*packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    detector->client.appModule.api->add_payload(detector->validateParams.flowp, payloadAppId);

    lua_pushnumber(L, 0);
    return 1;
}

int openAddHttpPattern(lua_State *L)
{
    int index = 1;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "Invalid HTTP detector user data addHttpPattern.");
        return 0;
    }

    /* Verify valid pattern type */
    enum httpPatternType pType = (enum httpPatternType) lua_tointeger(L, index++);
    if(pType < HTTP_PAYLOAD || pType > HTTP_URL)
    {
        _dpd.errMsg( "Invalid HTTP pattern type.");
        return 0;
    }

    /* Verify valid DHSequence */
    DHPSequence seq  = (DHPSequence) lua_tointeger(L, index++);
    if(seq < SINGLE || seq > USER_AGENT_HEADER)
    {
        _dpd.errMsg( "Invalid HTTP DHP Sequence.");
        return 0;
    }

    uint32_t serviceAppId  = lua_tointeger(L, index++);
    uint32_t clientAppId   = lua_tointeger(L, index++);
    uint32_t payloadAppId  = lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid detector context addHttpPattern: serviceAppId %u; clientAppId %u; payloadAppId %u\n",serviceAppId, clientAppId, payloadAppId);
        return 0;
    }

    /* Verify that pattern is a valid string */
    size_t pattern_size = 0;
    uint8_t* pattern_str = (uint8_t*) strdup(lua_tolstring(L, index++, &pattern_size));
    if(pattern_str == NULL || pattern_size == 0)
    {
        _dpd.errMsg( "Invalid HTTP pattern string.");
        free(pattern_str);
        return 0;
    }

    HTTPListElement *element = calloc(1, sizeof(*element));
    if (element == NULL)
    {
        _dpd.errMsg( "Failed to allocate HTTP list element memory.");
        free(pattern_str);
        return 0;
    }

    DetectorHTTPPattern *pattern = &element->detectorHTTPPattern;
    pattern->seq           = seq;
    pattern->service_id    = serviceAppId;
    pattern->client_app    = clientAppId;
    pattern->payload       = payloadAppId;
    pattern->pattern       = pattern_str;
    pattern->pattern_size  = (int) pattern_size;
    pattern->appId         = APP_ID_NONE;

    switch(pType)
    {
        case HTTP_PAYLOAD:
             element->next = httpPatternLists.hostPayloadPatternList;
             httpPatternLists.hostPayloadPatternList = element;
             break;

        case HTTP_URL:
             element->next = httpPatternLists.urlPatternList;
             httpPatternLists.urlPatternList = element;
             break;

        case HTTP_USER_AGENT:
             element->next = httpPatternLists.clientAgentPatternList;
             httpPatternLists.clientAgentPatternList = element;
             break;
    }

    appInfoSetActive(serviceAppId, true);
    appInfoSetActive(clientAppId, true);
    appInfoSetActive(payloadAppId, true);

    return 0;
}

static int openAddUrlPattern(lua_State *L)
{
    int index = 1;
    DetectorAppUrlPattern **tmp;
    const char *tmpString;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "Invalid HTTP detector user data in addAppUrl.");
        return 0;
    }

    u_int32_t serviceAppId      = lua_tointeger(L, index++);
    u_int32_t clientAppId      = lua_tointeger(L, index++);
    u_int32_t payloadAppId         = lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid HTTP detector context addAppUrl: serviceAppId %u; clientAppId %u; payloadAppId %u\n",serviceAppId, clientAppId, payloadAppId);
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t hostPatternSize = 0;
    u_int8_t* hostPattern = NULL;
    tmpString = lua_tolstring(L, index++, &hostPatternSize);
    if(!tmpString || !hostPatternSize || !(hostPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid host pattern string.");
        return 0;
    }

    /* Verify that path pattern is a valid string */
    size_t pathPatternSize = 0;
    u_int8_t* pathPattern = NULL;
    tmpString = lua_tolstring(L, index++, &pathPatternSize);
    if(!tmpString || !pathPatternSize || !(pathPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid path pattern string.");
        free(hostPattern);
        return 0;
    }

    /* Verify that scheme pattern is a valid string */
    size_t schemePatternSize;
    u_int8_t* schemePattern = NULL;
    tmpString = lua_tolstring(L, index++, &schemePatternSize);
    if(!tmpString || !schemePatternSize || !(schemePattern = (u_int8_t*) strdup(tmpString)))
    {
        _dpd.errMsg( "Invalid scheme pattern string.");
        free(pathPattern);
        free(hostPattern);
        return 0;
    }

    /* Allocate memory for data structures */
    DetectorAppUrlPattern *pattern = malloc(sizeof(DetectorAppUrlPattern));
    if (!pattern)
    {
        _dpd.errMsg( "Failed to allocate HTTP pattern memory.");
        free(hostPattern);
        free(pathPattern);
        free(schemePattern);
        return 0;
    }


    pattern->userData.service_id        = serviceAppId;
    pattern->userData.client_app        = clientAppId;
    pattern->userData.payload           = payloadAppId;
    pattern->userData.appId             = APP_ID_NONE;
    pattern->userData.query.pattern     = NULL;
    pattern->userData.query.patternSize = 0;
    pattern->patterns.host.pattern              = hostPattern;
    pattern->patterns.host.patternSize         = (int) hostPatternSize;
    pattern->patterns.path.pattern              = pathPattern;
    pattern->patterns.path.patternSize         = (int) pathPatternSize;
    pattern->patterns.scheme.pattern              = schemePattern;
    pattern->patterns.scheme.patternSize         = (int) schemePatternSize;

    DetectorAppUrlList *urlList = &httpPatternLists.appUrlList;

    /**first time usedCount and allocatedCount are both 0, urlPattern will be NULL.
     * This case is same as malloc. In case of error, realloc will return NULL, and
     * original urlPattern buffer is left untouched.
     */
    if (urlList->usedCount == urlList->allocatedCount)
    {
        tmp = realloc(urlList->urlPattern, (urlList->allocatedCount+URL_LIST_STEP_SIZE)*sizeof(*tmp));
        if (!tmp)
        {
            FreeDetectorAppUrlPattern(pattern);
            return 0;
        }
        urlList->urlPattern = tmp;
        urlList->allocatedCount += URL_LIST_STEP_SIZE;
    }

    urlList->urlPattern[urlList->usedCount++] = pattern;

    appInfoSetActive(serviceAppId, true);
    appInfoSetActive(clientAppId, true);
    appInfoSetActive(payloadAppId, true);

    return 0;
}

/*Lua should inject patterns in <clientAppId, clientVersion, multi-Pattern> format. */
static int Detector_addSipServer(lua_State *L)
{
    int index = 1;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "Invalid HTTP detector user data addSipServer.");
        return 0;
    }

    u_int32_t client_app      = lua_tointeger(L, index++);
    const char *clientVersion       = lua_tostring(L, index++);
    if(!clientVersion )
    {
        _dpd.errMsg( "Invalid sip client version string.");
        return 0;
    }

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid detector context addSipServer: client_app %u\n",client_app);
        return 0;
    }

    /* Verify that ua pattern is a valid string */
    const char* uaPattern = lua_tostring(L, index++);
    if(!uaPattern)
    {
        _dpd.errMsg( "Invalid sip ua pattern string.");
        return 0;
    }

    sipServerPatternAdd(client_app, clientVersion, uaPattern);

    appInfoSetActive(client_app, true);

    return 0;
}

static const luaL_reg Detector_methods[] = {
  /* Obsolete API names.  No longer use these!  They are here for backward
   * compatibility and will eventually be removed. */
  /*  - "memcmp" is now "matchSimplePattern" (below) */
  {"memcmp",                   Detector_memcmp},
  /*  - "getProtocolType" is now "getL4Protocol" (below) */
  {"getProtocolType",          Detector_getProtocolType},
  /*  - "inCompatibleData" is now "markIncompleteData" (below) */
  {"inCompatibleData",         service_inCompatibleData},
  /*  - "addDataId" is now "addAppIdDataToFlow" (below) */
  {"addDataId",                service_addDataId},
  /*  - "service_inCompatibleData" is now "service_markIncompleteData" (below) */
  {"service_inCompatibleData", service_inCompatibleData},
  /*  - "service_addDataId" is now "service_addAppIdDataToFlow" (below) */
  {"service_addDataId",        service_addDataId},

  {"getPacketSize",            Detector_getPacketSize},
  {"getPacketDir",             Detector_getPacketDir},
  {"matchSimplePattern",       Detector_memcmp},
  {"getPcreGroups",            Detector_getPcreGroups},
  {"getL4Protocol",            Detector_getProtocolType},
  {"getPktSrcAddr",            Detector_getPktSrcIPAddr},
  {"getPktDstAddr",            Detector_getPktDstIPAddr},
  {"getPktSrcPort",            Detector_getPktSrcPort},
  {"getPktDstPort",            Detector_getPktDstPort},
  {"getPktCount",              Detector_getPktCount},
  {"getFlow",                  Detector_getFlow},
  {"htons",                    Detector_htons},
  {"htonl",                    Detector_htonl},
  {"log",                      Detector_logMessage},
  {"addHttpPattern",           Detector_addHttpPattern},
  {"addAppUrl",                Detector_addAppUrl},
  {"addRTMPUrl",               Detector_addRTMPUrl},
  {"addContentTypePattern",    Detector_addContentTypePattern},
  {"addSSLCertPattern",        Detector_addSSLCertPattern},
  {"addSipUserAgent",          Detector_addSipUserAgent},
  {"addSipServer",             Detector_addSipServer},
  {"addSSLCnamePattern",       Detector_addSSLCnamePattern},
  {"addHostPortApp",           Detector_addHostPortApp},

  /*Obsolete - new detectors should not use this API */
  {"init",                     service_init},
  {"registerPattern",          service_registerPattern},
  {"getServiceID",             service_getServiceId},
  {"addPort",                  service_addPorts},
  {"removePort",               service_removePorts},
  {"setServiceName",           service_setServiceName},
  {"getServiceName",           service_getServiceName},
  {"isCustomDetector",         service_isCustomDetector},
  {"setValidator",             service_setValidator},
  {"addService",               service_addService},
  {"failService",              service_failService},
  {"inProcessService",         service_inProcessService},
  {"markIncompleteData",       service_inCompatibleData},
  {"analyzePayload",           service_analyzePayload},
  {"addAppIdDataToFlow",       service_addDataId},

  /*service API */
  {"service_init",             service_init},
  {"service_registerPattern",  service_registerPattern},
  {"service_getServiceId",     service_getServiceId},
  {"service_addPort",          service_addPorts},
  {"service_removePort",       service_removePorts},
  {"service_setServiceName",   service_setServiceName},
  {"service_getServiceName",   service_getServiceName},
  {"service_isCustomDetector", service_isCustomDetector},
  {"service_setValidator",     service_setValidator},
  {"service_addService",       service_addService},
  {"service_failService",      service_failService},
  {"service_inProcessService", service_inProcessService},
  {"service_markIncompleteData", service_inCompatibleData},
  {"service_analyzePayload",   service_analyzePayload},
  {"service_addAppIdDataToFlow", service_addDataId},

  /*client init API */
  {"client_init",              client_init},
  {"client_registerPattern",   client_registerPattern},
  {"client_getServiceId",      service_getServiceId},

  /*client service API */
  {"client_addApp",            client_addApp},
  {"client_addInfo",           client_addInfo},
  {"client_addUser",           client_addUser},
  {"client_addPayload",        client_addPayload},

  {"portOnlyService",          Detector_portOnlyService},

  {"registerAppId" ,           common_registerAppId},

  {"open_createApp",           openCreateApp},
  {"open_addClientApp",        openAddClientApp},
  {"open_addServiceApp",       openAddServiceApp},
  {"open_addPayloadApp",       openAddPayloadApp},
  {"open_addHttpPattern",      openAddHttpPattern},
  {"open_addUrlPattern",       openAddUrlPattern},

  {0, 0}
};

/**This function performs a clean exit on an api instance. It is called when RNA is performing
 * a clean exit.
 */
int Detector_fini(void *key, void *data)
{
    lua_State *myLuaState;
    Detector *detector = (Detector*) data;

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"Finishing detector %s\n",detector->server.serviceModule.name);
#endif

    myLuaState = detector->myLuaState;

    if (detector->packageInfo.server.cleanFunctionName && lua_checkstack(myLuaState, 1))
    {
        lua_getglobal(myLuaState, detector->packageInfo.server.cleanFunctionName);

        if (lua_pcall(myLuaState, 0, 0, 0 ))
        {
             /*See comment at first lua_pcall() */
#ifdef LUA_DETECTOR_DEBUG
            _dpd.errMsg( "%s: error running %s in lua: %s", detector->server.serviceModule.name,
                    detector->packageInfo.server.cleanFunctionName, lua_tostring(myLuaState, -1));
#endif
        }
    }
    else if (detector->packageInfo.client.cleanFunctionName && lua_checkstack(myLuaState, 1))
    {
        lua_getglobal(myLuaState, detector->packageInfo.client.cleanFunctionName);

        if (lua_pcall(myLuaState, 0, 0, 0 ))
        {
             /*See comment at first lua_pcall() */
#ifdef LUA_DETECTOR_DEBUG
            _dpd.errMsg( "%s: error running %s in lua: %s", detector->server.serviceModule.name,
                    detector->packageInfo.client.cleanFunctionName, lua_tostring(myLuaState, -1));
#endif
        }
    }
    else
    {
        _dpd.errMsg("%s: DetectorFini not provided\n",detector->server.serviceModule.name);
    }

    detectorRemoveAllPorts(detector);

    freeDetector(detector);

    /*lua_close will perform garbage collection after killing lua script. */
    /**Design: Lua_state does not allow me to store user variables so detectors store lua_state.
     * There is one lua_state for each lua file, which can have only one
     * detectors. So if lua detector creates a detector, registers a pattern
     * and then loses reference then lua will garbage collect but we should not free the buffer.
     *
     */
    lua_close(myLuaState);

    return 0;
}


/**Garbage collector hook function. Called when Lua side garbage collects detector api instance. Current design is to allocate
 * one of each luaState, detector and detectorUserData buffers, and hold these buffers till RNA exits. SigHups processing
 * reuses the buffers and calls DetectorInit to reinitialize. RNA ensures that DetectorUserData is not garbage collected, by
 * creating a reference in LUA_REGISTRY table. The reference is released only on RNA exit.
 *
 * If in future, one needs to free any of these buffers then one should consider references to detector buffer in  RNAServiceElement
 * stored in flows and hostServices  data structures. Other detectors at this time create one static instance for the lifetime of RNA,
 * and therefore we have adopted the same principle for Lua Detecotors.
 */
static int Detector_gc (
        lua_State *L
        )
{
    DetectorUserData *detectorUserData =  toDetectorUserData(L, -1);

    if (detectorUserData)
    {
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG,"DetectorUserData %p: freeing\n\n",detectorUserData);
#endif

    }

    return 0;
}

/*convert detector to string for printing */
static int Detector_tostring (
        lua_State *L
        )
{
  char buff[32];
  sprintf(buff, "%p", toDetectorUserData(L, 1));
  lua_pushfstring(L, "Detector (%s)", buff);
  return 1;
}

static const luaL_reg Detector_meta[] = {
  {"__gc",       Detector_gc},
  {"__tostring", Detector_tostring},
  {0, 0}
};


/**Registers C functions as an API, enabling Lua detector to call these functions. This function
 * should be called once before loading any lua detectors. This function itself is not part of API
 * and therefore can not be called by a Lua detection.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object
 * @return int - Number of elements on stack, which is 1 if successful, 0 otherwise.
 * @return methodArray/stack - array of newly created methods
 */
int Detector_register (
        lua_State *L
        )
{

  /* populates a new table with Detector_methods (method_table), add the table to the globals and stack*/
  luaL_openlib(L, DETECTOR, Detector_methods, 0);

  /* create metatable for Foo, add it to the Lua registry, metatable on stack */
  luaL_newmetatable(L, DETECTOR);

  /* populates table on stack with Detector_meta methods, puts the metatable on stack*/
  luaL_openlib(L, NULL, Detector_meta, 0);

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

/** @} */ /* end of LuaDetectorBaseApi */

static void FreeHTTPListElement(HTTPListElement *element)
{
    if (element)
    {
        if (element->detectorHTTPPattern.pattern)
            free(element->detectorHTTPPattern.pattern);
        free(element);
    }
}

static void FreeDetectorAppUrlPattern(DetectorAppUrlPattern *pattern)
{
    if (pattern)
    {
        if (pattern->userData.query.pattern)
            free(*(void **)&pattern->userData.query.pattern);
        if (pattern->patterns.host.pattern)
            free(*(void **)&pattern->patterns.host.pattern);
        if (pattern->patterns.path.pattern)
            free(*(void **)&pattern->patterns.path.pattern);
        if (pattern->patterns.scheme.pattern)
            free(*(void **)&pattern->patterns.scheme.pattern);
        free(pattern);
    }
}

void CleanHttpPatternLists(int exiting)
{
    HTTPListElement *element;
    size_t i;

    for (i = 0; i < httpPatternLists.appUrlList.usedCount; i++)
    {
        FreeDetectorAppUrlPattern(httpPatternLists.appUrlList.urlPattern[i]);
        httpPatternLists.appUrlList.urlPattern[i] = NULL;
    }
    if (exiting)
    {
        if (httpPatternLists.appUrlList.urlPattern)
        {
            free(httpPatternLists.appUrlList.urlPattern);
            httpPatternLists.appUrlList.urlPattern = NULL;
        }
        httpPatternLists.appUrlList.allocatedCount = 0;
        if (httpPatternLists.RTMPUrlList.urlPattern)
        {
            free(httpPatternLists.RTMPUrlList.urlPattern);
            httpPatternLists.RTMPUrlList.urlPattern = NULL;
        }
        httpPatternLists.RTMPUrlList.allocatedCount = 0;
    }
    httpPatternLists.appUrlList.usedCount = 0;
    httpPatternLists.RTMPUrlList.usedCount = 0;
    while ((element = httpPatternLists.clientAgentPatternList))
    {
        httpPatternLists.clientAgentPatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((element = httpPatternLists.hostPayloadPatternList))
    {
        httpPatternLists.hostPayloadPatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((element = httpPatternLists.urlPatternList))
    {
        httpPatternLists.urlPatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((element = httpPatternLists.contentTypePatternList))
    {
        httpPatternLists.contentTypePatternList = element->next;
        FreeHTTPListElement(element);
    }
}

