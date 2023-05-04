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
#include "lengthAppCache.h"
#include "detector_dns.h"
#include "app_forecast.h"
#include "detector_pattern.h"
#include "detector_cip.h"

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

/*static const char * LuaLogLabel = "luaDetectorApi"; */
static ThrottleInfo error_throttleInfo = {0,30,0};

#ifdef PERF_PROFILING
PreprocStats luaDetectorsPerfStats;
PreprocStats luaCiscoPerfStats;
PreprocStats luaCustomPerfStats;
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
        const char *detectorName
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
    detector->name = strdup(detectorName);
    if (!detector->name)
    {
        free(pUserData->pDetector);
        return NULL;
    }

    detector->myLuaState = L;
    pthread_mutex_init(&detector->luaReloadMutex, NULL);

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

    if (detector->callbackFcnName)
    {
        free(detector->callbackFcnName);
    }

    free(detector->name);
    free(detector->validatorBuffer);

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"Detector %p: freed\n\n",detector);
#endif
    free(detector);
}

int detector_Callback(
        const uint8_t *data,
        uint16_t size,
        const int dir,
        tAppIdData *flowp,
        const SFSnortPacket *pkt,
        Detector *detector,
        const tAppIdConfig *pConfig
        )
{
    int retValue;
    lua_State *myLuaState;
    char *callbackFn;
    char *detectorName;
    PROFILE_VARS;
#ifdef PERF_PROFILING
    PreprocStats *pPerfStats1;
    PreprocStats *pPerfStats2;
#endif

    if (!data || !flowp || !pkt || !detector)
    {
        return -10;
    }

#ifdef PERF_PROFILING
    if (detector->isCustom)
        pPerfStats1 = &luaCustomPerfStats;
    else
        pPerfStats1 = &luaCiscoPerfStats;
    pPerfStats2 = detector->pPerfStats;
#endif
    PREPROC_PROFILE_START(luaDetectorsPerfStats);
    PREPROC_PROFILE_START((*pPerfStats1));
    PREPROC_PROFILE_START((*pPerfStats2));

    myLuaState = detector->myLuaState;
    detector->validateParams.data = data;
    detector->validateParams.size = size;
    detector->validateParams.dir = dir;
    detector->validateParams.flowp = flowp;
    detector->validateParams.pkt = (SFSnortPacket *)pkt;
    callbackFn = detector->callbackFcnName;
    detectorName = detector->name;

    /* Bail out if we cannot acquire the lock on the detector.
       To avoid bailing out, use recursive mutex instead. */
    if( pthread_mutex_trylock(&detector->luaReloadMutex))
    {
        detector->validateParams.pkt = NULL;
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        return -11;
    }

    if ((!callbackFn) || !(lua_checkstack(myLuaState, 1)))
    {
        _dpd.errMsgThrottled(&error_throttleInfo,
                             "Detector %s: invalid LUA %s\n", 
                             detectorName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        return -10;
    }

    lua_getglobal(myLuaState, callbackFn);

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"Detector %s: Lua Memory usage %d\n", detectorName, lua_gc(myLuaState, LUA_GCCOUNT,0));
    _dpd.debugMsg(DEBUG_LOG,"Detector %s: validating\n", detectorName);
#endif
    if (lua_pcall(myLuaState, 0, 1, 0 ))
    {
        _dpd.errMsg("Detector %s: Error validating %s\n", detectorName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        return -10;
    }

    /**detectorFlows must be destroyed after each packet is processed.*/
    sflist_static_free_all(&allocatedFlowList, freeDetectorFlow);

    /* retrieve result */
    if (!lua_isnumber(myLuaState, -1))
    {
        _dpd.errMsg("Detector %s: Validator returned non-numeric value\n", detectorName);
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        retValue = -10;
    }

    retValue = lua_tonumber(myLuaState, -1);
    lua_pop(myLuaState, 1);  /* pop returned value */
    /*lua_settop(myLuaState, 0); */

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"Detector %s: Validator returned %d\n", detectorName, retValue);
#endif

    detector->validateParams.pkt = NULL;

    pthread_mutex_unlock(&detector->luaReloadMutex);
    PREPROC_PROFILE_END((*pPerfStats2));
    PREPROC_PROFILE_END((*pPerfStats1));
    PREPROC_PROFILE_END(luaDetectorsPerfStats);

    return retValue;
}

static int Detector_registerClientCallback(lua_State *L)
{
    tAppId appId;
    const char *callback;
    Detector *detector;
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    appId = lua_tonumber(L, index++);
    callback = lua_tostring(L, index++);

    if (!detectorUserData || !callback)
    {
        lua_pushnumber(L, -1);
        return 1;   /*number of results */
    }

    detector = detectorUserData->pDetector;

    if (!(detector->callbackFcnName = strdup(callback)))
    {
        lua_pushnumber(L, -1);
        return 1;
    }
    appSetClientDetectorCallback(detector_Callback, appId, detector, detector->pAppidNewConfig);

    lua_pushnumber(L, 0);
    return 1;
}

static int Detector_registerServiceCallback(lua_State *L)
{
    tAppId appId;
    const char *callback;
    Detector *detector;
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    appId = lua_tonumber(L, index++);
    callback = lua_tostring(L, index++);

    if (!detectorUserData || !callback)
    {
        lua_pushnumber(L, -1);
        return 1;   /*number of results */
    }

    detector = detectorUserData->pDetector;

    if (!(detector->callbackFcnName = strdup(callback)))
    {
        lua_pushnumber(L, -1);
        return 1;
    }
    appSetServiceDetectorCallback(detector_Callback, appId, detector, detector->pAppidNewConfig);

    lua_pushnumber(L, 0);
    return 1;
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
        appSetLuaServiceValidator(validateAnyService, appId, APPINFO_FLAG_SERVICE_ADDITIONAL, (void *)detector);
    if (detector->packageInfo.client.initFunctionName)
        appSetLuaClientValidator(validateAnyClientApp, appId, APPINFO_FLAG_CLIENT_ADDITIONAL, (void *)detector);

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
            DEBUG_WRAP(DebugMessage(DEBUG_APPID, "%s:%s\n",detector->server.serviceModule.name, message););
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
int validateAnyService(ServiceValidationArgs *args)
{
    int retValue;
    lua_State *myLuaState = NULL;
    const char *serverName;
    struct _Detector *detector = args->userdata;
    PROFILE_VARS;
#ifdef PERF_PROFILING
    PreprocStats *pPerfStats1;
    PreprocStats *pPerfStats2;
#endif

    if (!detector)
    {
        _dpd.errMsg( "invalid LUA parameters");
        return SERVICE_ENULL;
    }

#ifdef PERF_PROFILING
    if (detector->isCustom)
        pPerfStats1 = &luaCustomPerfStats;
    else
        pPerfStats1 = &luaCiscoPerfStats;
    pPerfStats2 = detector->pPerfStats;
#endif
    PREPROC_PROFILE_START(luaDetectorsPerfStats);
    PREPROC_PROFILE_START((*pPerfStats1));
    PREPROC_PROFILE_START((*pPerfStats2));

    myLuaState = detector->myLuaState;
    detector->validateParams.data = args->data;
    detector->validateParams.size = args->size;
    detector->validateParams.dir = args->dir;
    detector->validateParams.flowp = args->flowp;
    detector->validateParams.pkt = args->pkt;
    serverName = detector->name;

    /*Note: Some frequently used header fields may be extracted and stored in detector for */
    /*better performance. */

    pthread_mutex_lock(&detector->luaReloadMutex);
    if ((!detector->packageInfo.server.validateFunctionName) || !(lua_checkstack(myLuaState, 1)))
    {
        _dpd.errMsgThrottled(&error_throttleInfo,
                             "server %s: invalid LUA %s\n",
                             serverName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END((luaDetectorsPerfStats));
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
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        return SERVICE_ENULL;
    }

    /**detectorFlows must be destroyed after each packet is processed.*/
    sflist_static_free_all(&allocatedFlowList, freeDetectorFlow);

    /* retrieve result */
    if (!lua_isnumber(myLuaState, -1))
    {
        _dpd.errMsg("server %s:  validator returned non-numeric value\n",serverName);
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        return SERVICE_ENULL;
    }

    retValue = lua_tonumber(myLuaState, -1);
    lua_pop(myLuaState, 1);  /* pop returned value */
    /*lua_settop(myLuaState, 0); */

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"server %s: Validator returned %d\n",serverName, retValue);
#endif

    detector->validateParams.pkt = NULL;
    pthread_mutex_unlock(&detector->luaReloadMutex);

    PREPROC_PROFILE_END((*pPerfStats2));
    PREPROC_PROFILE_END((*pPerfStats1));
    PREPROC_PROFILE_END(luaDetectorsPerfStats);
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

/**
 * Design Notes: In these APIs, three different AppID contexts - pAppidNewConfig, pAppidOldConfig
 * and pAppidActiveConfig are used. pAppidNewConfig is used in APIs related to the loading of the
 * detector such as service_addPorts(), client_registerPattern(), etc. A detector is loaded either
 * during reload or at initialization. Use of pAppidNewConfig will cause the data structures related
 * to the detector such as service ports, patterns, etc to be saved in the new AppID context.
 *
 * The new AppID context becomes active at the end of initialization or at reload swap.
 * FinalizeLuaModules() is called at this time, which changes all the detectors' pAppidActiveConfig
 * references to the new context. Also, pAppidOldConfig will be changed to point to the previous
 * AppID context. In the packet processing APIs such as service_addService(), client_addUser(), etc.
 * pAppidActiveConfig is used.
 *
 * In the cleanup APIs such as service_removePorts(), Detector_fini(), etc., data structures in the
 * old AppID conext need to be freed. Therefore, pAppidOldConfig is used in these APIs.
 */

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

    if (ServiceAddPort(&pp, &detector->server.serviceModule, (void*)detector, detector->pAppidNewConfig))
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

    detectorRemoveAllPorts(detector, detector->pAppidOldConfig);

    lua_pushnumber(L, 0);
    return 1;
}

/**Shared function between Lua API and RNA core.
 */
void detectorRemoveAllPorts (
                             Detector *detector,
                             tAppIdConfig *pConfig
        )
{
    ServiceRemovePorts(&validateAnyService, (void*)detector, pConfig);
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

    lua_pushnumber(L, detector->isCustom);
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
            appGetAppFromServiceId(serviceId, detector->pAppidActiveConfig), vendor, version, NULL, NULL);

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
                                       detector->validateParams.dir, detector->server.pServiceElement, APPID_SESSION_DATA_NONE, detector->pAppidActiveConfig, NULL);

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
                                     detector->validateParams.dir, detector->server.pServiceElement, NULL);

    lua_pushnumber(L, retValue);
    return 1;
}

/**Detector use this function to indicate error in service identification.
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
                                            detector->validateParams.dir, detector->server.pServiceElement,
                                            APPID_SESSION_DATA_NONE, detector->pAppidActiveConfig, NULL);

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
    sfaddr_t *ipAddr;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    ipAddr = GET_SRC_IP(detector->validateParams.pkt);

    lua_checkstack (L, 1);
    lua_pushnumber(L, sfaddr_get_ip4_value(ipAddr));
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
    sfaddr_t *ipAddr;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    if (detectorUserData == NULL)
    {
        return 0;
    }

    detector = detectorUserData->pDetector;

    ipAddr = GET_DST_IP(detector->validateParams.pkt);

    lua_checkstack (L, 1);
    lua_pushnumber(L, sfaddr_get_ip4_value(ipAddr));
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
        tAppIdData *flowp,
        SFSnortPacket *pkt,
        Detector *detector,
        const tAppIdConfig *pConfig
        )
{
    int retValue;
    lua_State *myLuaState;
    char *validateFn;
    char *clientName;
    PROFILE_VARS;
#ifdef PERF_PROFILING
    PreprocStats *pPerfStats1;
    PreprocStats *pPerfStats2;
#endif

    if (!data || !flowp || !pkt || !detector)
    {
        return CLIENT_APP_ENULL;
    }

#ifdef PERF_PROFILING
    if (detector->isCustom)
        pPerfStats1 = &luaCustomPerfStats;
    else
        pPerfStats1 = &luaCiscoPerfStats;
    pPerfStats2 = detector->pPerfStats;
#endif
    PREPROC_PROFILE_START(luaDetectorsPerfStats);
    PREPROC_PROFILE_START((*pPerfStats1));
    PREPROC_PROFILE_START((*pPerfStats2));

    myLuaState = detector->myLuaState;
    detector->validateParams.data = data;
    detector->validateParams.size = size;
    detector->validateParams.dir = dir;
    detector->validateParams.flowp = flowp;
    detector->validateParams.pkt = (SFSnortPacket *)pkt;
    validateFn = detector->packageInfo.client.validateFunctionName;
    clientName = detector->name;

    pthread_mutex_lock(&detector->luaReloadMutex);
    if ((!validateFn) || !(lua_checkstack(myLuaState, 1)))
    {
        _dpd.errMsgThrottled(&error_throttleInfo,
                            "client %s: invalid LUA %s\n",
                           clientName, lua_tostring(myLuaState, -1));
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
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
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        return SERVICE_ENULL;
    }

    /**detectorFlows must be destroyed after each packet is processed.*/
    sflist_static_free_all(&allocatedFlowList, freeDetectorFlow);

    /* retrieve result */
    if (!lua_isnumber(myLuaState, -1))
    {
        _dpd.errMsg("client %s:  validator returned non-numeric value\n",clientName);
        detector->validateParams.pkt = NULL;
        pthread_mutex_unlock(&detector->luaReloadMutex);
        PREPROC_PROFILE_END((*pPerfStats2));
        PREPROC_PROFILE_END((*pPerfStats1));
        PREPROC_PROFILE_END(luaDetectorsPerfStats);
        retValue = SERVICE_ENULL;
    }

    retValue = lua_tonumber(myLuaState, -1);
    lua_pop(myLuaState, 1);  /* pop returned value */
    /*lua_settop(myLuaState, 0); */

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG,"client %s: Validator returned %d\n",clientName, retValue);
#endif

    detector->validateParams.pkt = NULL;
    pthread_mutex_unlock(&detector->luaReloadMutex);

    PREPROC_PROFILE_END((*pPerfStats2));
    PREPROC_PROFILE_END((*pPerfStats1));
    PREPROC_PROFILE_END(luaDetectorsPerfStats);
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
    clientAppLoadForConfigCallback((void *) &(detector->client.appModule), &detector->pAppidNewConfig->clientAppConfig);
    ClientAppRegisterPattern(validateAnyClientApp, protocol, (uint8_t*)pattern, size, position, 0, (void *)detector, &detector->pAppidNewConfig->clientAppConfig);

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

static int service_addClient (lua_State *L)
{
    tAppId clientAppId, serviceId;
    const char *version;
    Detector *detector;
    DetectorUserData *detectorUserData = checkDetectorUserData(L, 1);

    clientAppId = lua_tonumber(L, 2);
    serviceId = lua_tonumber(L, 3);
    version = lua_tostring(L, 4);

    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt
            || !version)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector = detectorUserData->pDetector;

    AppIdAddClientApp(detector->validateParams.pkt, detector->validateParams.dir, detector->pAppidActiveConfig, detector->validateParams.flowp, serviceId, clientAppId, version);

    lua_pushnumber(L, 0);
    return 1;
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

    if (!detector->client.appModule.api)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector->client.appModule.api->add_app(detector->validateParams.pkt, (APPID_SESSION_DIRECTION) detector->validateParams.dir, detector->pAppidActiveConfig,
		    detector->validateParams.flowp, appGetAppFromServiceId(serviceId, detector->pAppidActiveConfig), appGetAppFromClientId(productId, detector->pAppidActiveConfig), version);

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

    if (!detector->client.appModule.api)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

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

    if (!detector->client.appModule.api)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector->client.appModule.api->add_user(detector->validateParams.flowp, userName, appGetAppFromServiceId(serviceId, detector->pAppidActiveConfig), 1);

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

    if (!detector->client.appModule.api)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector->client.appModule.api->add_payload(detector->validateParams.flowp, appGetAppFromPayloadId(payloadId, detector->pAppidActiveConfig));

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
    int          index = 1;

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
    tAppIdConfig *pConfig = detectorUserData->pDetector->pAppidNewConfig;

    pattern->seq           = seq;
    pattern->service_id    = appGetAppFromServiceId(service_id, pConfig);
    pattern->client_app    = appGetAppFromClientId(client_app, pConfig);
    pattern->payload       = appGetAppFromPayloadId(payload, pConfig);
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
             element->next = pConfig->httpPatternLists.hostPayloadPatternList;
             pConfig->httpPatternLists.hostPayloadPatternList = element;
             break;

        case HTTP_URL:
             element->next = pConfig->httpPatternLists.urlPatternList;
             pConfig->httpPatternLists.urlPatternList = element;
             break;

        case HTTP_USER_AGENT:
             element->next = pConfig->httpPatternLists.clientAgentPatternList;
             pConfig->httpPatternLists.clientAgentPatternList = element;
             break;
    }

    appInfoSetActive(pattern->service_id, true);
    appInfoSetActive(pattern->client_app, true);
    appInfoSetActive(pattern->payload, true);
    appInfoSetActive(appId, true);

    return 0;
}

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

    if (!ssl_add_cert_pattern(pattern_str, pattern_size, type, app_id, &detectorUserData->pDetector->pAppidNewConfig->serviceSslConfig))
    {
        free(pattern_str);
        _dpd.errMsg( "Failed to add an SSL pattern list member");
        return 0;
    }

    appInfoSetActive(app_id, true);
    return 0;
}

/*  On the lua side, this should look something like:
        addDNSHostPattern(<appId>, '<pattern string>' )
*/
int Detector_addDNSHostPattern (lua_State *L)
{
    uint8_t *pattern_str;
    size_t pattern_size;
    int index = 1;
    uint8_t type;
    tAppId app_id;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid DNS detector user data or context.");
        return 0;
    }

    type = lua_tointeger(L, index++);
    app_id  = (tAppId) lua_tointeger(L, index++);

    pattern_size = 0;
    const char *tmpString = lua_tolstring(L, index++, &pattern_size);
    if (!tmpString || !pattern_size)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid DNS Host pattern string");
        return 0;
    }
    pattern_str = (uint8_t *)strdup(tmpString);
    if (!pattern_str)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid DNS Host pattern string.");
        return 0;
    }

    if (!dns_add_host_pattern(pattern_str, pattern_size, type, app_id, &detectorUserData->pDetector->pAppidNewConfig->serviceDnsConfig))
    {
        free(pattern_str);
        _dpd.errMsg( "LuaDetectorApi:Failed to add an SSL pattern list member");
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

    if (!ssl_add_cname_pattern(pattern_str, pattern_size, type, app_id, &detectorUserData->pDetector->pAppidNewConfig->serviceSslConfig))
    {
        free(pattern_str);
        _dpd.errMsg( "Failed to add an SSL pattern list member");
        return 0;
    }

    appInfoSetActive(app_id, true);
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

    if (!hostPortAppCacheAdd(&ip6Addr, (uint16_t)port, (uint16_t)proto, type, app_id, detectorUserData->pDetector->pAppidNewConfig))
    {
        _dpd.errMsg("%s:Failed to backend call\n",__func__);
    }

    return 0;
}

static int Detector_addHostPortAppDynamic (lua_State *L)
{
    /*uint8_t *ipaddr_str; */
    size_t ipaddr_size;
    int index = 1;
    uint8_t type;
    tAppId app_id;
    struct in6_addr ip6Addr;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg("%s: Invalid detector user data.\n",__func__);
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

    if (!hostPortAppCacheDynamicAdd(&ip6Addr, (uint16_t)port, (uint16_t)proto, type, app_id, true))
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
        free(pattern);
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
    tAppIdConfig        *pConfig  = detectorUserData->pDetector->pAppidNewConfig;

    detector->pattern = pattern;
    detector->pattern_size = strlen((char *)pattern);
    detector->appId = appId;

    element->next = pConfig->httpPatternLists.contentTypePatternList;
    pConfig->httpPatternLists.contentTypePatternList = element;

    appInfoSetActive(appId, true);

    return 0;
}

static inline int GetDetectorUserData(lua_State *L, int index,
        DetectorUserData **detector_user_data, const char *errorString)
{
    // Verify detector user data and that we are not in packet context
    *detector_user_data = checkDetectorUserData(L, index);
    if (!*detector_user_data || (*detector_user_data)->pDetector->validateParams.pkt)
    {
        _dpd.errMsg(errorString);
        return -1;
    }
    return 0;
}

static int detector_create_chp_app(DetectorUserData *detectorUserData, tAppId appIdInstance,
                unsigned app_type_flags, int num_matches)
{
    CHPApp *new_app = (CHPApp *)calloc(1,sizeof(CHPApp));
    if (!new_app)
    {
        _dpd.errMsg( "LuaDetectorApi:Failed to allocate CHP app memory.");
        return -1;
    }
    new_app->appIdInstance = appIdInstance;
    new_app->app_type_flags = app_type_flags;
    new_app->num_matches = num_matches;

    if (sfxhash_add(detectorUserData->pDetector->pAppidNewConfig->CHP_glossary, &(new_app->appIdInstance), new_app))
    {
        _dpd.errMsg( "LuaDetectorApi:Failed to add CHP for appId %d, instance %d", CHP_APPIDINSTANCE_TO_ID(appIdInstance), CHP_APPIDINSTANCE_TO_INSTANCE(appIdInstance));
        free(new_app);
        return -1;
    }
    return 0;
}

static int Detector_CHPCreateApp (lua_State *L)
{
    DetectorUserData *detectorUserData;
    tAppId appId;
    unsigned app_type_flags;
    int num_matches;

    tAppId appIdInstance;

    int index = 1;

    if (GetDetectorUserData(L, index++, &detectorUserData,
        "LuaDetectorApi:Invalid HTTP detector user data in CHPCreateApp."))
        return 0;

    appId = lua_tointeger(L, index++);
    appIdInstance = CHP_APPID_SINGLE_INSTANCE(appId); // Last instance for the old API

    app_type_flags =    lua_tointeger(L, index++);
    num_matches =       lua_tointeger(L, index++);

    // We only want one of these for each appId.
    if (sfxhash_find(detectorUserData->pDetector->pAppidNewConfig->CHP_glossary, &appIdInstance))
    {
        _dpd.errMsg( "LuaDetectorApi:Attempt to add more than one CHP for appId %d - use CHPMultiCreateApp", appId);
        return 0;
    }

    detector_create_chp_app(detectorUserData, appIdInstance, app_type_flags, num_matches);
    return 0;
}

static inline int CHPGetKeyPatternBoolean(lua_State *L, int index)
{
    return (0 != lua_tointeger(L, index));
}

static inline int CHPGetPatternType(lua_State *L, int index, PatternType *pattern_type)
{
    *pattern_type = (PatternType) lua_tointeger(L, index);
    if(*pattern_type < AGENT_PT || *pattern_type > MAX_PATTERN_TYPE)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid CHP Action pattern type.");
        return -1;
    }
    return 0;
}

static inline int CHPGetPatternDataAndSize(lua_State *L, int index, char **pattern_data, size_t *pattern_size)
{
    const char *tmpString = NULL; // Lua owns this pointer
    *pattern_size = 0;
    *pattern_data = NULL;
    tmpString = lua_tolstring(L, index, &*pattern_size);
    if(!tmpString || !*pattern_size || !(*pattern_data = strdup(tmpString))) // non-empty pattern required
    {
        if (*pattern_size) // implies strdup() failed
            _dpd.errMsg( "LuaDetectorApi:CHP Action PATTERN string mem alloc failed.");
        else
            _dpd.errMsg( "LuaDetectorApi:Invalid CHP Action PATTERN string."); // empty string in Lua code - bad
        return -1;
    }
    return 0;
}

static inline int CHPGetActionType(lua_State *L, int index, ActionType *action_type)
{
    *action_type = (ActionType) lua_tointeger(L, index);
    if (*action_type < NO_ACTION || *action_type > MAX_ACTION_TYPE)
    {
        _dpd.errMsg( "LuaDetectorApi:Incompatible CHP Action type, might be for a later version.");
        return -1;
    }
    return 0;
}

static inline int CHPGetActionData(lua_State *L, int index, char **action_data)
{
    // An empty string is translated into a NULL pointer because the action data is optional
    const char *tmpString = NULL; // Lua owns this pointer
    size_t action_data_size = 0;
    *action_data = NULL;
    tmpString = lua_tolstring(L, index, &action_data_size);
    if (action_data_size)
    {
        if(!(*action_data = strdup(tmpString)))
        {
            _dpd.errMsg( "LuaDetectorApi:Action DATA string mem alloc failed.");
            return -1;
        }
    }
    return 0;
}

static int detector_add_chp_action(DetectorUserData *detectorUserData,
                    tAppId appIdInstance, int isKeyPattern, PatternType patternType,
                    size_t patternSize, char *patternData, ActionType actionType, char *optionalActionData)
{
    uint precedence;
    CHPListElement *tmp_chpa, *prev_chpa, *chpa;
    CHPApp *chpapp;
    tAppIdConfig *pConfig = detectorUserData->pDetector->pAppidNewConfig;

    //find the CHP App for this
    if (!(chpapp = sfxhash_find(detectorUserData->pDetector->pAppidNewConfig->CHP_glossary, &appIdInstance)))
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid attempt to add a CHP action for unknown appId %d, instance %d. - pattern:\"%s\" - action \"%s\"\n",
            CHP_APPIDINSTANCE_TO_ID(appIdInstance), CHP_APPIDINSTANCE_TO_INSTANCE(appIdInstance),
            patternData, optionalActionData ? optionalActionData : "");
        free(patternData);
        if (optionalActionData) free(optionalActionData);
        return 0;
    }

    if (isKeyPattern)
    {
        chpapp->key_pattern_count++;
        chpapp->key_pattern_length_sum += patternSize;
    }

    if (chpapp->ptype_scan_counts[patternType] == 0)
        chpapp->num_scans++;
    precedence = chpapp->ptype_scan_counts[patternType]++; // The increment is for the sake of the precedence. ptype_scan_counts means "The scan DOES count toward the total required."
    // at runtime we'll want to know how many of each type of pattern we are looking for.
    if (actionType == REWRITE_FIELD || actionType == INSERT_FIELD)
    {
        if (!appInfoEntryFlagGet(CHP_APPIDINSTANCE_TO_ID(appIdInstance), APPINFO_FLAG_SUPPORTED_SEARCH, pConfig))
        {
            _dpd.errMsg( "LuaDetectorApi: CHP action type, %d, requires previous use of action type, %d, (see appId %d, pattern=\"%s\").\n",
                actionType, GET_OFFSETS_FROM_REBUILT, CHP_APPIDINSTANCE_TO_ID(appIdInstance), patternData);
            free(patternData);
            if (optionalActionData) free(optionalActionData);
            return 0;
        }
        switch (patternType)
        {
            // permitted pattern type (modifiable HTTP/SPDY request field)
            case AGENT_PT:
            case HOST_PT:
            case REFERER_PT:
            case URI_PT:
            case COOKIE_PT:
                break;
            default:
                _dpd.errMsg( "LuaDetectorApi: CHP action type, %d, on unsupported pattern type, %d, (see appId %d, pattern=\"%s\").\n",
                    actionType, patternType, CHP_APPIDINSTANCE_TO_ID(appIdInstance), patternData);
                free(patternData);
                if (optionalActionData) free(optionalActionData);
                return 0;
        }
    }
    else if (actionType != ALTERNATE_APPID && actionType != DEFER_TO_SIMPLE_DETECT)
        chpapp->ptype_req_counts[patternType]++;

    chpa = (CHPListElement*)calloc(1,sizeof(CHPListElement));
    if (!chpa)
    {
        _dpd.errMsg( "LuaDetectorApi: Failed to allocate CHP action memory.\n");
        free(patternData);
        if (optionalActionData) free(optionalActionData);
        return 0;
    }
    chpa->chp_action.appIdInstance = appIdInstance;
    chpa->chp_action.precedence = precedence;
    chpa->chp_action.key_pattern = isKeyPattern;
    chpa->chp_action.ptype = patternType;
    chpa->chp_action.psize = patternSize;
    chpa->chp_action.pattern = patternData;
    chpa->chp_action.action = actionType;
    chpa->chp_action.action_data = optionalActionData;
    chpa->chp_action.chpapp = chpapp; // link this struct to the Glossary entry

    tmp_chpa = pConfig->httpPatternLists.chpList;
    if (!tmp_chpa) pConfig->httpPatternLists.chpList = chpa;
    else
    {
        while (tmp_chpa->next)
            tmp_chpa = tmp_chpa->next;
        tmp_chpa->next = chpa;
    }

    /* Set the safe-search bits in the appId entry */
    if (actionType == GET_OFFSETS_FROM_REBUILT)
    {
        /* This is a search engine and it is SUPPORTED for safe-search packet rewrite */
        appInfoEntryFlagSet(CHP_APPIDINSTANCE_TO_ID(appIdInstance), APPINFO_FLAG_SEARCH_ENGINE | APPINFO_FLAG_SUPPORTED_SEARCH, pConfig);
    }
    else if (actionType == SEARCH_UNSUPPORTED)
    {
        /* This is a search engine and it is UNSUPPORTED for safe-search packet rewrite */
        appInfoEntryFlagSet(CHP_APPIDINSTANCE_TO_ID(appIdInstance), APPINFO_FLAG_SEARCH_ENGINE, pConfig);
    }
    else if (actionType == DEFER_TO_SIMPLE_DETECT && strcmp(patternData,"<ignore-all-patterns>") == 0)
    {
        // Walk the list of all the patterns we have inserted, searching for this appIdInstance and free them.
        // The purpose is for the 14 and 15 to be used together to only set the APPINFO_FLAG_SEARCH_ENGINE flag
        // If the reserved pattern is not used, it is a mixed use case and should just behave normally.
        prev_chpa = NULL;
        tmp_chpa = pConfig->httpPatternLists.chpList;
        while (tmp_chpa)
        {
            if (tmp_chpa->chp_action.appIdInstance == appIdInstance)
            {
                // advance the tmp_chpa pointer by removing the item pointed to. Keep prev_chpa unchanged.

                // 1) unlink the struct, 2) free strings and then 3) free the struct.
                chpa = tmp_chpa; // preserve this pointer to be freed at the end.
                if (prev_chpa == NULL)
                {
                    // Remove from head
                    pConfig->httpPatternLists.chpList = tmp_chpa->next;
                    tmp_chpa = pConfig->httpPatternLists.chpList;
                }
                else
                {
                    // Remove from middle of list.
                    prev_chpa->next = tmp_chpa->next;
                    tmp_chpa = prev_chpa->next;
                }
                free(chpa->chp_action.pattern);
                if (chpa->chp_action.action_data) free(chpa->chp_action.action_data);
                free(chpa);
            }
            else
            {
                // advance both pointers
                prev_chpa = tmp_chpa;
                tmp_chpa = tmp_chpa->next;
            }
        }
    }
    return 0;
}
static int Detector_CHPAddAction (lua_State *L)
{
    DetectorUserData *detectorUserData;
    int key_pattern;
    PatternType ptype;
    size_t psize;
    char *pattern;
    ActionType action;
    char *action_data;

    tAppId appIdInstance;
    tAppId appId;

    int index = 1;

    if (GetDetectorUserData(L, index++, &detectorUserData,
        "LuaDetectorApi:Invalid HTTP detector user data in CHPAddAction."))
        return 0;

    // Parameter 1
    appId = lua_tointeger(L, index++);
    appIdInstance = CHP_APPID_SINGLE_INSTANCE(appId); // Last instance for the old API

    // Parameter 2
    key_pattern = CHPGetKeyPatternBoolean(L, index++);

    // Parameter 3
    if (CHPGetPatternType(L, index++, &ptype))
        return 0;

    // Parameter 4
    if (CHPGetPatternDataAndSize(L, index++, &pattern, &psize))
        return 0;

    // Parameter 5
    if (CHPGetActionType(L, index++, &action))
    {
        free(pattern);
        return 0;
    }

    // Parameter 6
    if (CHPGetActionData(L, index++, &action_data))
    {
        free(pattern);
        return 0;
    }

    return detector_add_chp_action(detectorUserData, appIdInstance, key_pattern, ptype,
                    psize, pattern, action, action_data);
}

static int Detector_CHPMultiCreateApp (lua_State *L)
{
    DetectorUserData *detectorUserData;
    tAppId appId;
    unsigned app_type_flags;
    int num_matches;

    tAppId appIdInstance;
    int instance;

    int index = 1;

    if (GetDetectorUserData(L, index++, &detectorUserData,
        "LuaDetectorApi:Invalid HTTP detector user data in CHPMultiCreateApp."))
        return 0;

    appId =             lua_tointeger(L, index++);
    app_type_flags =    lua_tointeger(L, index++);
    num_matches =       lua_tointeger(L, index++);

    for (instance=0; instance < CHP_APPID_INSTANCE_MAX; instance++ )
    {
        appIdInstance = (appId << CHP_APPID_BITS_FOR_INSTANCE) + instance;
        if (sfxhash_find(detectorUserData->pDetector->pAppidNewConfig->CHP_glossary, &appIdInstance))
            continue;
        break;
    }

    // We only want a maximum of these for each appId.
    if (instance == CHP_APPID_INSTANCE_MAX)
    {
        _dpd.errMsg( "LuaDetectorApi:Attempt to create more than %d CHP for appId %d", CHP_APPID_INSTANCE_MAX, appId);
        return 0;
    }

    if (detector_create_chp_app(detectorUserData, appIdInstance, app_type_flags, num_matches))
        return 0;

    lua_pushnumber(L, appIdInstance);
    return 1;
}


static int Detector_CHPMultiAddAction (lua_State *L)
{
    DetectorUserData *detectorUserData;
    int key_pattern;
    PatternType ptype;
    size_t psize;
    char *pattern;
    ActionType action;
    char *action_data;

    tAppId appIdInstance;

    int index = 1;

    if (GetDetectorUserData(L, index++, &detectorUserData,
        "LuaDetectorApi:Invalid HTTP detector user data in CHPMultiAddAction."))
        return 0;

    // Parameter 1
    appIdInstance = lua_tointeger(L, index++);

    // Parameter 2
    key_pattern = CHPGetKeyPatternBoolean(L, index++);

    // Parameter 3
    if (CHPGetPatternType(L, index++, &ptype))
        return 0;

    // Parameter 4
    if (CHPGetPatternDataAndSize(L, index++, &pattern, &psize))
        return 0;

    // Parameter 5
    if (CHPGetActionType(L, index++, &action))
    {
        free(pattern);
        return 0;
    }

    // Parameter 6
    if (CHPGetActionData(L, index++, &action_data))
    {
        free(pattern);
        return 0;
    }

    return detector_add_chp_action(detectorUserData, appIdInstance, key_pattern, ptype,
                    psize, pattern, action, action_data);
}

static int Detector_portOnlyService (lua_State *L)
{
    int index = 1;

    // Verify detector user data and that we are not in packet context
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid HTTP detector user data in addPortOnlyService.");
        return 0;
    }

    tAppId appId = lua_tointeger(L, index++);
    u_int16_t port = lua_tointeger(L, index++);
    u_int8_t protocol = lua_tointeger(L, index++);

    if (port == 0)
        detectorUserData->pDetector->pAppidNewConfig->ip_protocol[protocol] = appId;
    else if (protocol == 6)
        detectorUserData->pDetector->pAppidNewConfig->tcp_port_only[port] = appId;
    else if (protocol == 17)
        detectorUserData->pDetector->pAppidNewConfig->udp_port_only[port] = appId;

    return 0;
}

/* Add a length-based detector.  This is done by adding a new length sequence
 * to the cache.  Note that this does not require a validate and is only used
 * as a fallback identification.
 *
 * @param lua_State* - Lua state variable.
 * @param appId/stack        - App ID to use for this detector.
 * @param proto/stack        - Protocol (IPPROTO_TCP/DC.ipproto.tcp (6) or
 *                             IPPROTO_UDP/DC.ipproto.udp (17)).
 * @param sequence_cnt/stack - Number of elements in sequence below (max of
 *                             LENGTH_SEQUENCE_CNT_MAX).
 * @param sequence_str/stack - String that defines direction/length sequence.
 *  - Example: "I/8,R/512,I/512,R/1024,I/1024"
 *     - Direction: I(nitiator) or R(esponder).
 *     - Length   : Payload size (bytes) (> 0).
 * @return int - Number of elements on stack, which is always 1.
 * @return status/stack - 0 if successful, -1 otherwise.
 */
static int Detector_lengthAppCacheAdd(lua_State *L)
{
    int         i;
    const char *str_ptr;
    uint16_t    length;
    tLengthKey  length_sequence;
    int         index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (detectorUserData == NULL)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid detector user data!");
        lua_pushnumber(L, -1);
        return 1;
    }

    tAppId      appId        = lua_tonumber(L, index++);
    uint8_t     proto        = lua_tonumber(L, index++);
    uint8_t     sequence_cnt = lua_tonumber(L, index++);
    const char *sequence_str = lua_tostring(L, index++);

    if (((proto != IPPROTO_TCP) && (proto != IPPROTO_UDP))
        || ((sequence_cnt == 0) || (sequence_cnt > LENGTH_SEQUENCE_CNT_MAX))
        || ((sequence_str == NULL) || (strlen(sequence_str) == 0)))
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid input (%d,%u,%u,\"%s\")!",
               appId, (unsigned)proto, (unsigned)sequence_cnt, sequence_str ?: "");
        lua_pushnumber(L, -1);
        return 1;
    }

    memset(&length_sequence, 0, sizeof(length_sequence));

    length_sequence.proto        = proto;
    length_sequence.sequence_cnt = sequence_cnt;

    str_ptr = sequence_str;
    for (i = 0; i < sequence_cnt; i++)
    {
        int last_one;

        switch (*str_ptr)
        {
        case 'I':
            length_sequence.sequence[i].direction = APP_ID_FROM_INITIATOR;
            break;
        case 'R':
            length_sequence.sequence[i].direction = APP_ID_FROM_RESPONDER;
            break;
        default:
            _dpd.errMsg( "LuaDetectorApi:Invalid sequence string (\"%s\")!",
                   sequence_str);
            lua_pushnumber(L, -1);
            return 1;
        }
        str_ptr++;

        if (*str_ptr != '/')
        {
            _dpd.errMsg( "LuaDetectorApi:Invalid sequence string (\"%s\")!",
                   sequence_str);
            lua_pushnumber(L, -1);
            return 1;
        }
        str_ptr++;

        length = (uint16_t)atoi(str_ptr);
        if (length == 0)
        {
            _dpd.errMsg( "LuaDetectorApi:Invalid sequence string (\"%s\")!",
                   sequence_str);
            lua_pushnumber(L, -1);
            return 1;
        }
        length_sequence.sequence[i].length = length;

        while ((*str_ptr != ',') && (*str_ptr != 0))
        {
            str_ptr++;
        }

        last_one = (i == (sequence_cnt - 1));
        if (   (!last_one && (*str_ptr != ','))
            || (last_one && (*str_ptr != 0)))
        {
            _dpd.errMsg( "LuaDetectorApi:Invalid sequence string (\"%s\")!",
                   sequence_str);
            lua_pushnumber(L, -1);
            return 1;
        }
        str_ptr++;
    }

    if (!lengthAppCacheAdd(&length_sequence, appId, detectorUserData->pDetector->pAppidNewConfig))
    {
        _dpd.errMsg( "LuaDetectorApi:Could not add entry to cache!");
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

static int Detector_AFAddApp (lua_State *L)
{
    int index = 1;
    AFElement val;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg( "LuaDetectorApi:Invalid HTTP detector user data in AFAddApp.");
        return 0;
    }

    tAppId indicator = lua_tointeger(L, index++);
    tAppId forecast  = lua_tointeger(L, index++);
    tAppId target    = lua_tointeger(L, index++);

    if (sfxhash_find(detectorUserData->pDetector->pAppidNewConfig->AF_indicators, &indicator))
    {
        _dpd.errMsg( "LuaDetectorApi:Attempt to add more than one AFElement per appId %d", indicator);
        return 0;
    }

    val.indicator = indicator;
    val.forecast = forecast;
    val.target = target;

    if (sfxhash_add(detectorUserData->pDetector->pAppidNewConfig->AF_indicators, &indicator, &val))
    {
        _dpd.errMsg( "LuaDetectorApi:Failed to add AFElement for appId %d", indicator);
        return 0;
    }

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
    u_int32_t client_id      = lua_tointeger(L, index++);
    lua_tointeger(L, index++); // client_app_type
    u_int32_t payload_id         = lua_tointeger(L, index++);
    lua_tointeger(L, index++); // payload_type

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid HTTP detector context addAppUrl: service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t hostPatternSize = 0;
    u_int8_t* hostPattern = NULL;
    tmpString = lua_tolstring(L, index++, &hostPatternSize);
    if(!tmpString || !hostPatternSize)
    {
        _dpd.errMsg( "Invalid host pattern string:service_id %u; client_id %u; payload_id %u\n.",service_id, client_id, payload_id);
        return 0;
    }
    else if (!(hostPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate host pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
        return 0;
    }

    /* Verify that path pattern is a valid string */
    size_t pathPatternSize = 0;
    u_int8_t* pathPattern = NULL;
    tmpString = lua_tolstring(L, index++, &pathPatternSize);
    if(!tmpString || !pathPatternSize)
    {
        _dpd.errMsg( "Invalid path pattern string: service_id %u; client_id %u; payload_id %u\n.",service_id, client_id, payload_id);
        free(hostPattern);
        return 0;
    }
    else if (!(pathPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate path pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
        free(hostPattern);
        return 0;
    }

    /* Verify that scheme pattern is a valid string */
    size_t schemePatternSize;
    u_int8_t* schemePattern = NULL;
    tmpString = lua_tolstring(L, index++, &schemePatternSize);
    if(!tmpString || !schemePatternSize)
    {
        _dpd.errMsg( "Invalid scheme pattern string: service_id %u; client_id %u; payload_id %u\n.",service_id, client_id, payload_id);
        free(pathPattern);
        free(hostPattern);
        return 0;
    }
    else if (!(schemePattern = (u_int8_t*) strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate scheme pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
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

    tAppIdConfig       *pConfig = detectorUserData->pDetector->pAppidNewConfig;

    pattern->userData.service_id        = appGetAppFromServiceId(service_id, pConfig);
    pattern->userData.client_app        = appGetAppFromClientId(client_id, pConfig);
    pattern->userData.payload           = appGetAppFromPayloadId(payload_id, pConfig);
    pattern->userData.appId             = appId;
    pattern->userData.query.pattern     = queryPattern;
    pattern->userData.query.patternSize = queryPatternSize;
    pattern->patterns.host.pattern              = hostPattern;
    pattern->patterns.host.patternSize         = (int) hostPatternSize;
    pattern->patterns.path.pattern              = pathPattern;
    pattern->patterns.path.patternSize         = (int) pathPatternSize;
    pattern->patterns.scheme.pattern              = schemePattern;
    pattern->patterns.scheme.patternSize         = (int) schemePatternSize;

    DetectorAppUrlList *urlList = &pConfig->httpPatternLists.appUrlList;

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
    u_int32_t client_id      = lua_tointeger(L, index++);
    lua_tointeger(L, index++); // client_app_type 
    u_int32_t payload_id         = lua_tointeger(L, index++);
    lua_tointeger(L, index++); // payload_type

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid HTTP detector context addRTMPUrl: service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t hostPatternSize = 0;
    u_int8_t* hostPattern = NULL;
    tmpString = lua_tolstring(L, index++, &hostPatternSize);
    if(!tmpString || !hostPatternSize) 
    {
        _dpd.errMsg( "Invalid host pattern string:service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        return 0;
    }
    else if (!(hostPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate host pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
        return 0;
    }

    /* Verify that path pattern is a valid string */
    size_t pathPatternSize = 0;
    u_int8_t* pathPattern = NULL;
    tmpString = lua_tolstring(L, index++, &pathPatternSize);
    if(!tmpString || !pathPatternSize)
    {
        _dpd.errMsg( "Invalid path pattern string: service_id %u; client_id %u; payload_id %u\n.",service_id, client_id, payload_id);
        free(hostPattern);
        return 0;
    }
    else if (!(pathPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate path pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
        free(hostPattern);
        return 0;
    }

    /* Verify that scheme pattern is a valid string */
    size_t schemePatternSize;
    u_int8_t* schemePattern = NULL;
    tmpString = lua_tolstring(L, index++, &schemePatternSize);
    if(!tmpString || !schemePatternSize) 
    {
        _dpd.errMsg( "Invalid scheme pattern string: service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        free(pathPattern);
        free(hostPattern);
        return 0;
    }
    else if (!(schemePattern = (u_int8_t*) strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate scheme pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
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
    pattern->userData.client_app        = client_id;
    pattern->userData.payload           = payload_id;
    pattern->userData.appId             = appId;
    pattern->userData.query.pattern     = queryPattern;
    pattern->userData.query.patternSize = queryPatternSize;
    pattern->patterns.host.pattern              = hostPattern;
    pattern->patterns.host.patternSize         = (int) hostPatternSize;
    pattern->patterns.path.pattern              = pathPattern;
    pattern->patterns.path.patternSize         = (int) pathPatternSize;
    pattern->patterns.scheme.pattern              = schemePattern;
    pattern->patterns.scheme.patternSize         = (int) schemePatternSize;

    tAppIdConfig       *pConfig = detectorUserData->pDetector->pAppidNewConfig;
    DetectorAppUrlList *urlList = &pConfig->httpPatternLists.RTMPUrlList;

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

    sipUaPatternAdd(client_app, clientVersion, uaPattern, &detectorUserData->pDetector->pAppidNewConfig->detectorSipConfig);

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

    AppInfoTableEntry *entry = appInfoEntryCreate(tmpString, detectorUserData->pDetector->pAppidNewConfig);

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

    if (!detector->client.appModule.api)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector->client.appModule.api->add_app(detector->validateParams.pkt, (APPID_SESSION_DIRECTION) detector->validateParams.dir, detector->pAppidActiveConfig, detector->validateParams.flowp, serviceAppId, clientAppId, "");

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
            serviceId, NULL, NULL, NULL, NULL);

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

    if (!detector->client.appModule.api)
    {
        lua_pushnumber(L, -1);
        return 1;
    }

    detector->client.appModule.api->add_payload(detector->validateParams.flowp, payloadAppId);

    lua_pushnumber(L, 0);
    return 1;
}

int openAddHttpPattern(lua_State *L)
{
    int index = 1;
    tAppIdConfig *pConfig;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "Invalid HTTP detector user data addHttpPattern.");
        return 0;
    }

    pConfig = detectorUserData->pDetector->pAppidNewConfig;

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
             element->next = pConfig->httpPatternLists.hostPayloadPatternList;
             pConfig->httpPatternLists.hostPayloadPatternList = element;
             break;

        case HTTP_URL:
             element->next = pConfig->httpPatternLists.urlPatternList;
             pConfig->httpPatternLists.urlPatternList = element;
             break;

        case HTTP_USER_AGENT:
             element->next = pConfig->httpPatternLists.clientAgentPatternList;
             pConfig->httpPatternLists.clientAgentPatternList = element;
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

    tAppIdConfig *pConfig = detectorUserData->pDetector->pAppidNewConfig;
    u_int32_t service_id	= lua_tointeger(L, index++);
    u_int32_t client_id      	= lua_tointeger(L, index++);
    u_int32_t payload_id        = lua_tointeger(L, index++);

    if (detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("Invalid HTTP detector context addAppUrl: service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        return 0;
    }

    /* Verify that host pattern is a valid string */
    size_t hostPatternSize = 0;
    u_int8_t* hostPattern = NULL;
    tmpString = lua_tolstring(L, index++, &hostPatternSize);
    if(!tmpString || !hostPatternSize)
    {
        _dpd.errMsg( "Invalid host pattern string: service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        return 0;
    }
    else if (!(hostPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate host pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
        return 0;
    }

    /* Verify that path pattern is a valid string */
    size_t pathPatternSize = 0;
    u_int8_t* pathPattern = NULL;
    tmpString = lua_tolstring(L, index++, &pathPatternSize);
    if(!tmpString || !pathPatternSize)
    {
        _dpd.errMsg( "Invalid path pattern string: service_id %u; client_id %u; payload %u\n.",service_id, client_id, payload_id);
        free(hostPattern);
        return 0;
    }
    else if (!(pathPattern = (u_int8_t *)strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate path pattern: %s, service_id %u; client_id %u; payload %u\n.",tmpString, service_id, client_id, payload_id);
        free(hostPattern);
        return 0;
    }

    /* Verify that scheme pattern is a valid string */
    size_t schemePatternSize;
    u_int8_t* schemePattern = NULL;
    tmpString = lua_tolstring(L, index++, &schemePatternSize);
    if(!tmpString || !schemePatternSize) 
    {
        _dpd.errMsg( "Invalid scheme pattern string: service_id %u; client_id %u; payload_id %u\n",service_id, client_id, payload_id);
        free(pathPattern);
        free(hostPattern);
        return 0;
    }
    else if (!(schemePattern = (u_int8_t*) strdup(tmpString)))
    {
        _dpd.errMsg( "Failed to duplicate scheme pattern: %s, service_id %u; client_id %u; payload_id %u\n.",tmpString, service_id, client_id, payload_id);
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


    pattern->userData.service_id        = service_id;
    pattern->userData.client_app        = client_id;
    pattern->userData.payload           = payload_id;
    pattern->userData.appId             = APP_ID_NONE;
    pattern->userData.query.pattern     = NULL;
    pattern->userData.query.patternSize = 0;
    pattern->patterns.host.pattern              = hostPattern;
    pattern->patterns.host.patternSize         = (int) hostPatternSize;
    pattern->patterns.path.pattern              = pathPattern;
    pattern->patterns.path.patternSize         = (int) pathPatternSize;
    pattern->patterns.scheme.pattern              = schemePattern;
    pattern->patterns.scheme.patternSize         = (int) schemePatternSize;

    DetectorAppUrlList *urlList = &pConfig->httpPatternLists.appUrlList;

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

    appInfoSetActive(service_id, true);
    appInfoSetActive(client_id, true);
    appInfoSetActive(payload_id, true);

    return 0;
}

void CleanClientPortPatternList(tAppIdConfig *pConfig)
{
    tPortPatternNode *tmp;

    if ( pConfig->clientPortPattern)
    {
        while ((tmp = pConfig->clientPortPattern->luaInjectedPatterns))
        {
            pConfig->clientPortPattern->luaInjectedPatterns = tmp->next;
            free(tmp->pattern);
            free(tmp->detectorName);
            free(tmp);
        }

        free(pConfig->clientPortPattern);
    }
}

void CleanServicePortPatternList(tAppIdConfig *pConfig)
{
    tPortPatternNode *tmp;

    if ( pConfig->servicePortPattern)
    {
        while ((tmp = pConfig->servicePortPattern->luaInjectedPatterns))
        {
            pConfig->servicePortPattern->luaInjectedPatterns = tmp->next;
            free(tmp->pattern);
            free(tmp->detectorName);
            free(tmp);
        }

        free(pConfig->servicePortPattern);
    }
}

/* Add a port and pattern based detection for client application. Both port and pattern criteria
 * must be met before client application is deemed detected.
 *
 * @param lua_State* - Lua state variable.
 * @param proto/stack        - Protocol (IPPROTO_TCP/DC.ipproto.tcp (6) or
 *                             IPPROTO_UDP/DC.ipproto.udp (17)).
 * @param port/stack - port number to register.
 * @param pattern/stack - pattern to be matched.
 * @param patternLenght/stack - length of pattern
 * @param offset/stack - offset into packet payload where matching should start.
 * @param appId/stack        - App ID to use for this detector.
 * @return int - Number of elements on stack, which is always 0.
 */
static int addPortPatternClient(lua_State *L)
{
    int index = 1;
    tAppIdConfig *pConfig;
    tPortPatternNode *pPattern;
    uint8_t protocol;
    uint16_t port;
    const char*pattern;
    size_t patternSize = 0;
    unsigned position;
    tAppId appId;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "addPortPatternClient(): Invalid detector user data");
        return 0;
    }

    pConfig = detectorUserData->pDetector->pAppidNewConfig;
    protocol = lua_tonumber(L, index++);
    //port      = lua_tonumber(L, index++);
    port = 0;
    pattern = lua_tolstring(L, index++, &patternSize);
    position = lua_tonumber(L, index++);
    appId = lua_tointeger(L, index++);

    if (!pConfig->clientPortPattern)
    {
        if (!(pConfig->clientPortPattern = calloc(1, sizeof(*pConfig->clientPortPattern))))
        {
            _dpd.errMsg( "addPortPatternClient(): memory allocation failure");
            return 0;
        }
    }
    if (appId <= APP_ID_NONE || !pattern || !patternSize || (protocol != IPPROTO_TCP && protocol != IPPROTO_UDP))
    {
        _dpd.errMsg("addPortPatternClient(): Invalid input in %s\n", detectorUserData->pDetector->name);
        return 0;
    }
    if (!(pPattern  = calloc(1, sizeof(*pPattern))))
    {
        _dpd.errMsg( "addPortPatternClient(): memory allocation failure");
        return 0;
    }
    if (!(pPattern->pattern  = malloc(patternSize)))
    {
        _dpd.errMsg( "addPortPatternClient(): memory allocation failure");
        free(pPattern);
        return 0;
    }

    pPattern->appId = appId;
    pPattern->protocol = protocol;
    pPattern->port = port;
    memcpy(pPattern->pattern, pattern, patternSize);
    pPattern->length = patternSize;
    pPattern->offset = position;
    if (!(pPattern->detectorName = strdup(detectorUserData->pDetector->name)))
    {
        _dpd.errMsg( "addPortPatternClient(): memory allocation failure");
        free(pPattern->pattern);
        free(pPattern);
        return 0;
    }

    //insert ports in order.
    {
        tPortPatternNode **prev;
        tPortPatternNode **curr;

        prev = NULL;
        for (curr = &pConfig->clientPortPattern->luaInjectedPatterns;
                *curr;
                prev = curr, curr = &((*curr)->next))
        {
            if (strcmp(pPattern->detectorName, (*curr)->detectorName) || pPattern->protocol < (*curr)->protocol
                    || pPattern->port < (*curr)->port)
                break;
        }
        if (prev)
        {
            pPattern->next = (*prev)->next;
            (*prev)->next = pPattern;
        }
        else
        {
            pPattern->next = *curr;
            *curr = pPattern;
        }
    }

    appInfoSetActive(appId, true);

    return 0;
}

/* Add a port and pattern based detection for service application. Both port and pattern criteria
 * must be met before service application is deemed detected.
 *
 * @param lua_State* - Lua state variable.
 * @param proto/stack        - Protocol (IPPROTO_TCP/DC.ipproto.tcp (6) or
 *                             IPPROTO_UDP/DC.ipproto.udp (17)).
 * @param port/stack - port number to register.
 * @param pattern/stack - pattern to be matched.
 * @param patternLenght/stack - length of pattern
 * @param offset/stack - offset into packet payload where matching should start.
 * @param appId/stack        - App ID to use for this detector.
 * @return int - Number of elements on stack, which is always 0.
 */
static int addPortPatternService(lua_State *L)
{
    int index = 1;
    size_t patternSize = 0;
    tAppIdConfig *pConfig;
    tPortPatternNode *pPattern;
    uint8_t protocol;
    uint16_t port;
    const char *pattern;
    unsigned position;
    tAppId appId;

    /* Verify detector user data and that we are not in packet context */
    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData)
    {
        _dpd.errMsg( "addPortPatternService(): Invalid detector user data");
        return 0;
    }

    pConfig = detectorUserData->pDetector->pAppidNewConfig;
    protocol = lua_tonumber(L, index++);
    port      = lua_tonumber(L, index++);
    pattern = lua_tolstring(L, index++, &patternSize);
    position = lua_tonumber(L, index++);
    appId = lua_tointeger(L, index++);

    if (!pConfig->servicePortPattern)
    {
        if (!(pConfig->servicePortPattern = calloc(1, sizeof(*pConfig->servicePortPattern))))
        {
            _dpd.errMsg( "addPortPatternService(): memory allocation failure");
            return 0;
        }
    }
    if (!(pPattern = calloc(1, sizeof(*pPattern))))
    {
        _dpd.errMsg( "addPortPatternService(): memory allocation failure");
        return 0;
    }
    if (!(pPattern->pattern  = malloc(patternSize)))
    {
        _dpd.errMsg( "addPortPatternService(): memory allocation failure");
        free(pPattern);
        return 0;
    }

    pPattern->appId = appId;
    pPattern->protocol = protocol;
    pPattern->port = port;
    memcpy(pPattern->pattern, pattern, patternSize);
    pPattern->length = patternSize;
    pPattern->offset = position;
    if (!(pPattern->detectorName = strdup(detectorUserData->pDetector->name)))
    {
        _dpd.errMsg( "addPortPatternService(): memory allocation failure");
        free(pPattern->pattern);
        free(pPattern);
        return 0;
    }

    //insert ports in order.
    {
        tPortPatternNode **prev;
        tPortPatternNode **curr;

        prev = NULL;
        for (curr = &pConfig->servicePortPattern->luaInjectedPatterns;
                *curr;
                prev = curr, curr = &((*curr)->next))
        {
            if (strcmp(pPattern->detectorName, (*curr)->detectorName) || pPattern->protocol < (*curr)->protocol
                    || pPattern->port < (*curr)->port)
                break;
        }
        if (prev)
        {
            pPattern->next = (*prev)->next;
            (*prev)->next = pPattern;
        }
        else
        {
            pPattern->next = *curr;
            *curr = pPattern;
        }
    }

    appInfoSetActive(appId, true);

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

    sipServerPatternAdd(client_app, clientVersion, uaPattern, &detectorUserData->pDetector->pAppidNewConfig->detectorSipConfig);

    appInfoSetActive(client_app, true);

    return 0;
}

static inline int ConvertStringToAddress(const char * string, sfaddr_t * address)
{
    int af;
    struct in6_addr buf;

    if (strchr(string, ':'))
        af = AF_INET6;
    else if (strchr(string, '.'))
        af = AF_INET;
    else
        return 0;

    if (inet_pton(af, string, &buf))
    {
        if (sfip_set_raw(address, &buf, af) != SFIP_SUCCESS)
            return 0;
    }
    else
        return 0;

    return 1;    // success
}

/**Creates a future flow based on the current flow.  When the future flow is
 * seen, the app ID will simply be declared with the info given here.
 *
 * @param Lua_State* - Lua state variable.
 * @param detector/stack - detector object.
 * @param client_addr/stack - client address of the future flow
 * @param client_port/stack - client port of the the future flow (can use 0 for wildcard here)
 * @param server_addr/stack - server address of the future flow
 * @param server_port/stack - server port of the future flow
 * @param proto/stack - protocol type (see define IPPROTO_xxxx in /usr/include/netinet/in.h)
 * @param service_app_id/stack - service app ID to declare for future flow (can be 0 for none)
 * @param client_app_id/stack - client app ID to declare for future flow (can be 0 for none)
 * @param payload_app_id/stack - payload app ID to declare for future flow (can be 0 for none)
 * @param app_id_to_snort/stack - AppID's app ID entry to convert to Snort app ID (see note below)
 * @return int - number of elements on stack, which is 1 if successful, 0 otherwise.
 *
 * Notes: For app_id_to_snort, use the app ID that AppID knows about (it'll
 * probably be a repeat of one of the other 3 app IDs given here).  For
 * example, for "FTP Data", use 166.  Internally, this'll be converted to the
 * app ID that Snort recognizes ("ftp-data").  For this to really mean
 * anything, the app IDs entry in appMapping.data should have a Snort app ID
 * defined.
 *
 * Example: createFutureFlow("192.168.0.200", 0, "192.168.0.100", 20, 6, 166, 0, 0, 166)
 */
static int createFutureFlow (lua_State *L)
{
    sfaddr_t client_addr;
    sfaddr_t server_addr;
    uint8_t proto;
    uint16_t client_port, server_port;
    DetectorUserData *detectorUserData = NULL;
    char *pattern;
    tAppId service_app_id, client_app_id, payload_app_id, app_id_to_snort;
    int16_t snort_app_id;
    tAppIdData *fp;

    detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        return 0;
    }

    pattern = (char *)lua_tostring(L, 2);
    if (!ConvertStringToAddress(pattern, &client_addr))
        return 0;

    client_port = lua_tonumber(L, 3);

    pattern = (char *)lua_tostring(L, 4);
    if (!ConvertStringToAddress(pattern, &server_addr))
        return 0;

    server_port = lua_tonumber(L, 5);

    proto = lua_tonumber(L, 6);

    service_app_id = lua_tointeger(L, 7);
    client_app_id  = lua_tointeger(L, 8);
    payload_app_id = lua_tointeger(L, 9);

    app_id_to_snort = lua_tointeger(L, 10);
    if (app_id_to_snort > APP_ID_NONE)
    {
        AppInfoTableEntry* entry = appInfoEntryGet(app_id_to_snort, appIdActiveConfigGet());
        if (NULL == entry)
            return 0;
        snort_app_id = entry->snortId;
    }
    else
    {
        snort_app_id = 0;
    }

    fp = AppIdEarlySessionCreate(detectorUserData->pDetector->validateParams.flowp,
                                 detectorUserData->pDetector->validateParams.pkt,
                                 &client_addr, client_port, &server_addr, server_port, proto,
                                 snort_app_id,
                                 APPID_EARLY_SESSION_FLAG_FW_RULE);
    if (fp)
    {
        fp->serviceAppId = service_app_id;
        fp->clientAppId  = client_app_id;
        fp->payloadAppId = payload_app_id;
        setAppIdFlag(fp, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_NOT_A_SERVICE | APPID_SESSION_PORT_SERVICE_DONE);
        fp->rnaServiceState = RNA_STATE_FINISHED;
        fp->rnaClientState  = RNA_STATE_FINISHED;

        return 1;
    }
    else
        return 0;
}

static int isMidStreamSession(lua_State *L)
{
    DetectorUserData *detectorUserData = NULL;

    detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
    {
        lua_pushnumber(L, -1);
        return -1;
    }

    if (_dpd.sessionAPI->get_session_flags(detectorUserData->pDetector->validateParams.pkt->stream_session) & SSNFLAG_MIDSTREAM)
    {
        lua_pushnumber(L, 1);
        return 1;
    }

    lua_pushnumber(L, 0);
    return 0;
}

/* Check if traffic is going through an HTTP proxy */
static int isHttpTunnel(lua_State *L)
{
    DetectorUserData *detectorUserData = NULL;

    detectorUserData = checkDetectorUserData(L, 1);

    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
        return -1;

    httpSession *hsession = detectorUserData->pDetector->validateParams.flowp->hsession;
    if (hsession)
    {
        tunnelDest *tunDest = hsession->tunDest;
        if (tunDest)
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }

    lua_pushboolean(L, 0);
    return 0;
}

/* Get destination IP tunneled through a proxy */
static int getHttpTunneledIp(lua_State* L)
{
    DetectorUserData *detectorUserData = NULL;

    detectorUserData = checkDetectorUserData(L, 1);
    
    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
        return -1;

    httpSession *hsession = detectorUserData->pDetector->validateParams.flowp->hsession;
    if (hsession)
    {
        tunnelDest *tunDest = hsession->tunDest;
        if (!tunDest)
            lua_pushnumber(L, 0);
        else 
            lua_pushnumber(L, sfaddr_get_ip4_value(&(tunDest->ip)));
    }
    
    return 1;
}

/* Get port tunneled through a proxy */
static int getHttpTunneledPort(lua_State* L)
{
    DetectorUserData *detectorUserData = NULL;

    detectorUserData = checkDetectorUserData(L, 1);
    
    /*check inputs and whether this function is called in context of a packet */
    if (!detectorUserData || !detectorUserData->pDetector->validateParams.pkt)
        return -1;

    httpSession *hsession = detectorUserData->pDetector->validateParams.flowp->hsession;
    if (hsession)
    {
        tunnelDest *tunDest = hsession->tunDest;
        if (!tunDest)
            lua_pushnumber(L, 0);
        else
            lua_pushnumber(L, tunDest->port);
    }
    
    return 1;
}

/*Lua should inject patterns in <clientAppId, classId> format. */
static int Detector_addCipConnectionClass(lua_State *L)
{
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n", __func__);
        return -1;
    }

    uint32_t appId = lua_tointeger(L, index++);
    uint32_t classId = lua_tointeger(L, index++);

    if (CipAddConnectionClass(appId, classId) == -1)
    {
        return -1;
    }

    appInfoSetActive(appId, true);

    return 0;
}

/*Lua should inject patterns in <clientAppId, classId, serviceId> format. */
static int Detector_addCipPath(lua_State *L)
{
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n", __func__);
        return -1;
    }

    uint32_t appId = lua_tointeger(L, index++);
    uint32_t classId = lua_tointeger(L, index++);
    uint8_t serviceId = lua_tointeger(L, index++);

    if (CipAddPath(appId, classId, serviceId) == -1)
    {
        return -1;
    }

    appInfoSetActive(appId, true);

    return 0;
}

/*Lua should inject patterns in <clientAppId, classId, isClassInstance, attributeId> format. */
static int Detector_addCipSetAttribute(lua_State *L)
{
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n", __func__);
        return -1;
    }

    uint32_t appId = lua_tointeger(L, index++);
    uint32_t classId = lua_tointeger(L, index++);
    bool isClassInstance = lua_toboolean(L, index++);
    uint32_t attributeId = lua_tointeger(L, index++);

    if (CipAddSetAttribute(appId, classId, isClassInstance, attributeId) == -1)
    {
        return -1;
    }

    appInfoSetActive(appId, true);

    return 0;
}


/*Lua should inject patterns in <clientAppId, serviceId> format. */
static int Detector_addCipExtendedSymbolService(lua_State *L)
{
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n", __func__);
        return -1;
    }

    uint32_t appId = lua_tointeger(L, index++);
    uint8_t serviceId = lua_tointeger(L, index++);

    if (CipAddExtendedSymbolService(appId, serviceId) == -1)
    {
        return -1;
    }

    appInfoSetActive(appId, true);

    return 0;
}

/*Lua should inject patterns in <clientAppId, serviceId> format. */
static int Detector_addCipService(lua_State *L)
{
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n", __func__);
        return -1;
    }

    uint32_t appId = lua_tointeger(L, index++);
    uint16_t serviceId = lua_tointeger(L, index++);

    if (CipAddService(appId, serviceId) == -1)
    {
        return -1;
    }

    appInfoSetActive(appId, true);

    return 0;
}

/*Lua should inject patterns in <clientAppId, enipCommandId> format. */
static int Detector_addEnipCommand(lua_State *L)
{
    int index = 1;

    DetectorUserData *detectorUserData = checkDetectorUserData(L, index++);
    if (!detectorUserData || detectorUserData->pDetector->validateParams.pkt)
    {
        _dpd.errMsg("%s: Invalid detector user data or context.\n", __func__);
        return -1;
    }

    uint32_t appId = lua_tointeger(L, index++);
    uint16_t commandId = lua_tointeger(L, index++);

    if (CipAddEnipCommand(appId, commandId) == -1)
    {
        return -1;
    }

    appInfoSetActive(appId, true);

    return 0;
}

static const luaL_Reg Detector_methods[] = {
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
  {"addHostPortAppDynamic",    Detector_addHostPortAppDynamic},
  {"addDNSHostPattern",        Detector_addDNSHostPattern},
  {"registerClientDetectorCallback",    Detector_registerClientCallback},
  {"registerServiceDetectorCallback",   Detector_registerServiceCallback},

  /* CIP registration */
  {"addCipConnectionClass",    Detector_addCipConnectionClass},
  {"addCipPath",               Detector_addCipPath},
  {"addCipSetAttribute",       Detector_addCipSetAttribute},
  {"addCipExtendedSymbolService", Detector_addCipExtendedSymbolService},
  {"addCipService",            Detector_addCipService},
  {"addEnipCommand",           Detector_addEnipCommand},

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
  {"service_addClient",        service_addClient},

  /*client init API */
  {"client_init",              client_init},
  {"client_registerPattern",   client_registerPattern},
  {"client_getServiceId",      service_getServiceId},

  /*client service API */
  {"client_addApp",            client_addApp},
  {"client_addInfo",           client_addInfo},
  {"client_addUser",           client_addUser},
  {"client_addPayload",        client_addPayload},

  //HTTP Multi Pattern engine
  {"CHPCreateApp",             Detector_CHPCreateApp},
  {"CHPAddAction",             Detector_CHPAddAction},
  {"CHPMultiCreateApp",        Detector_CHPMultiCreateApp}, // allows multiple detectors, same appId
  {"CHPMultiAddAction",        Detector_CHPMultiAddAction},

  //App Forecasting engine
  {"AFAddApp",                 Detector_AFAddApp},

  {"portOnlyService",          Detector_portOnlyService},

  /* Length-based detectors. */
  {"AddLengthBasedDetector",   Detector_lengthAppCacheAdd},

  {"registerAppId" ,           common_registerAppId},

  {"open_createApp",           openCreateApp},
  {"open_addClientApp",        openAddClientApp},
  {"open_addServiceApp",       openAddServiceApp},
  {"open_addPayloadApp",       openAddPayloadApp},
  {"open_addHttpPattern",      openAddHttpPattern},
  {"open_addUrlPattern",       openAddUrlPattern},

  {"addPortPatternClient",     addPortPatternClient},
  {"addPortPatternService",    addPortPatternService},

  {"createFutureFlow",         createFutureFlow},
  {"isMidStreamSession",       isMidStreamSession},

  { "isHttpTunnel",             isHttpTunnel },
  { "getHttpTunneledIp",        getHttpTunneledIp },
  { "getHttpTunneledPort",      getHttpTunneledPort },

  {0, 0}
};

/**This function performs a clean exit on an api instance. It is called when RNA is performing
 * a clean exit.
 */
void Detector_fini(void *data)
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

    freeDetector(detector);

    /*lua_close will perform garbage collection after killing lua script. */
    /**Design: Lua_state does not allow me to store user variables so detectors store lua_state.
     * There is one lua_state for each lua file, which can have only one
     * detectors. So if lua detector creates a detector, registers a pattern
     * and then loses reference then lua will garbage collect but we should not free the buffer.
     *
     */
    lua_close(myLuaState);
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
  snprintf(buff, sizeof(buff), "%p", toDetectorUserData(L, 1));
  lua_pushfstring(L, "Detector (%s)", buff);
  return 1;
}

static const luaL_Reg Detector_meta[] = {
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

static void FreeCHPAppListElement (CHPListElement *element)
{
    if (element)
    {
        if (element->chp_action.pattern) free(element->chp_action.pattern);
        if (element->chp_action.action_data) free(element->chp_action.action_data);
        free (element);
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

void CleanHttpPatternLists(tAppIdConfig *pConfig)
{
    HTTPListElement *element;
    CHPListElement *chpe;
    size_t i;

    for (i = 0; i < pConfig->httpPatternLists.appUrlList.usedCount; i++)
    {
        FreeDetectorAppUrlPattern(pConfig->httpPatternLists.appUrlList.urlPattern[i]);
        pConfig->httpPatternLists.appUrlList.urlPattern[i] = NULL;
    }
    for (i = 0; i < pConfig->httpPatternLists.RTMPUrlList.usedCount; i++)
    {
        FreeDetectorAppUrlPattern(pConfig->httpPatternLists.RTMPUrlList.urlPattern[i]);
        pConfig->httpPatternLists.RTMPUrlList.urlPattern[i] = NULL;
    }
    if (pConfig->httpPatternLists.appUrlList.urlPattern)
    {
        free(pConfig->httpPatternLists.appUrlList.urlPattern);
        pConfig->httpPatternLists.appUrlList.urlPattern = NULL;
    }
    pConfig->httpPatternLists.appUrlList.allocatedCount = 0;
    if (pConfig->httpPatternLists.RTMPUrlList.urlPattern)
    {
        free(pConfig->httpPatternLists.RTMPUrlList.urlPattern);
        pConfig->httpPatternLists.RTMPUrlList.urlPattern = NULL;
    }
    pConfig->httpPatternLists.RTMPUrlList.allocatedCount = 0;
    pConfig->httpPatternLists.appUrlList.usedCount = 0;
    pConfig->httpPatternLists.RTMPUrlList.usedCount = 0;
    while ((element = pConfig->httpPatternLists.clientAgentPatternList))
    {
        pConfig->httpPatternLists.clientAgentPatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((element = pConfig->httpPatternLists.hostPayloadPatternList))
    {
        pConfig->httpPatternLists.hostPayloadPatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((element = pConfig->httpPatternLists.urlPatternList))
    {
        pConfig->httpPatternLists.urlPatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((element = pConfig->httpPatternLists.contentTypePatternList))
    {
        pConfig->httpPatternLists.contentTypePatternList = element->next;
        FreeHTTPListElement(element);
    }
    while ((chpe = pConfig->httpPatternLists.chpList))
    {
        pConfig->httpPatternLists.chpList = chpe->next;
        FreeCHPAppListElement(chpe);
    }
}

