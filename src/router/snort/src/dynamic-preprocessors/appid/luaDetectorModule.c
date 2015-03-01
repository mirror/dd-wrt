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


/** @defgroup LuaDetectorCore  LuaDetectorCore
 * Functions supporting Lua detectors in core engine.
 *@{
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sfxhash.h"
#include "appIdConfig.h"
#include "luaDetectorApi.h"
#include "luaDetectorFlowApi.h"
#include "luaDetectorModule.h"

#ifdef HAVE_OPENSSL_MD5
#include <openssl/md5.h>

#define MD5CONTEXT MD5_CTX

#define MD5INIT    MD5_Init
#define MD5UPDATE  MD5_Update
#define MD5FINAL   MD5_Final
#define MD5DIGEST  MD5

#else
#include "md5.h"
#define MD5CONTEXT struct MD5Context

#define MD5INIT   MD5Init
#define MD5UPDATE MD5Update
#define MD5FINAL  MD5Final
#define MD5DIGEST MD5
#endif

#define MAXPD 1024

typedef void (tFileProcessorFn)(char *filename);

/*static const char * LuaLogLabel = "luaDetectorModule"; */

static SFXHASH *allocatedDetectorList; /*list of detectors allocated */
SF_LIST allocatedFlowList;  /*list of flows allocated. */
static uint32_t gLuaTrackerSize = 0;
static unsigned gNumDetectors = 0;
static unsigned gNumActiveDetectors;
static void luaDetectorsCleanInactive(void);

#ifdef LUA_DETECTOR_DEBUG
void luaErrorHandler(lua_State *l, lua_Debug *ar)
{

     /*fill up the debug structure with information from the lua stack */
     lua_getinfo(l, "Sln", ar);

     if (ar->event == LUA_HOOKCALL)
     {
         _dpd.debugMsg(DEBUG_LOG,"Called: %s:%d: %s (%s)", ar->short_src, ar->linedefined, (ar->name == NULL ? "[UNKNOWN]\n",: ar->name), ar->namewhat);
     }
     else if (ar->event ==LUA_HOOKRET)
     {
         _dpd.debugMsg(DEBUG_LOG,"returned: %s:%d: %s (%s)", ar->short_src, ar->linedefined, (ar->name == NULL ? "[UNKNOWN]\n",: ar->name), ar->namewhat);
     }
}
#endif

static lua_State *createLuaState()
{
    lua_State *myLuaState = NULL;

    /*Design: Whether to make all detectors share the lua_State or not?
     * Separate lua_State creates isolated environment for each detector. There
     * is no name collision or any side effects due to garbage collection. Runtime
     * memory overhead should be bearable.
     *
     * Shared lua_State saves on memory since each Lua library is loaded just once.
     */
    myLuaState = lua_open();   /* opens Lua */
    luaL_openlibs(myLuaState);

#ifdef HAVE_LIBLUAJIT
    /*linked in during compilation */
    luaopen_jit(myLuaState);

    {
        static unsigned once = 0;
        if (!once)
        {
            lua_getfield(myLuaState, LUA_REGISTRYINDEX, "_LOADED");
            lua_getfield(myLuaState, -1, "jit");  /* Get jit.* module table. */
            lua_getfield (myLuaState, -1, "version");
            if (lua_isstring(myLuaState, -1))
                _dpd.logMsg("LuaJIT: Version %s\n", lua_tostring(myLuaState, -1));
            lua_pop(myLuaState, 1);
            once = 1;
        }
    }

#endif  /*HAVE_LIBLUAJIT */

    Detector_register(myLuaState);
    lua_pop(myLuaState, 1); /*After detector register the methods are still on the stack, remove them. */

    DetectorFlow_register(myLuaState);
    lua_pop(myLuaState, 1);

#if 0
#ifdef LUA_DETECTOR_DEBUG
    lua_sethook(myLuaState,&luaErrorHandler,LUA_MASKCALL | LUA_MASKRET,0);
#endif
#endif

    /*The garbage-collector pause controls how long the collector waits before */
    /*starting a new cycle. Larger values make the collector less aggressive. */
    /*Values smaller than 100 mean the collector will not wait to start a new */
    /*cycle. A value of 200 means that the collector waits for the total memory */
    /*in use to double before starting a new cycle. */

    lua_gc(myLuaState, LUA_GCSETPAUSE,100);

    /*The step multiplier controls the relative speed of the collector relative */
    /*to memory allocation. Larger values make the collector more aggressive */
    /*but also increase the size of each incremental step. Values smaller than */
    /*100 make the collector too slow and can result in the collector never */
    /*finishing a cycle. The default, 200, means that the collector runs at */
    /*"twice" the speed of memory allocation. */

    lua_gc(myLuaState, LUA_GCSETSTEPMUL,200);

    /*set lua library paths */
    {
        char newLuaPath[PATH_MAX];
        lua_getglobal( myLuaState, "package" );
        lua_getfield( myLuaState, -1, "path" );
        const char * curLuaPath = lua_tostring(myLuaState, -1); 
        if (curLuaPath && (strlen(curLuaPath)))
        {
            snprintf(newLuaPath, PATH_MAX-1, "%s;%s/odp/libs/?.lua;%s/custom/libs/?.lua", 
                    curLuaPath,
                    appIdCommandConfig->app_id_detector_path, 
                    appIdCommandConfig->app_id_detector_path);
        }
        else
        {
            snprintf(newLuaPath, PATH_MAX-1, "%s/odp/libs/?.lua;%s/custom/libs/?.lua", 
                    appIdCommandConfig->app_id_detector_path, 
                    appIdCommandConfig->app_id_detector_path);
        }

        lua_pop( myLuaState, 1 ); 
        lua_pushstring( myLuaState, newLuaPath); 
        lua_setfield( myLuaState, -2, "path" ); 
        lua_pop( myLuaState, 1 ); 
    }

    return myLuaState;
}

/**reads packageInfo defined inside lua detector.
 */
static void getDetectorPackageInfo(
        lua_State *L,
        Detector *detector,
        int fillDefaults
        )
{
    tDetectorPackageInfo  *pkg = &detector->packageInfo;
    lua_getglobal (L, "DetectorPackageInfo");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);

        if (fillDefaults)
        {
            /*set default values first */
            pkg->name = strdup("NoName");
            pkg->server.initFunctionName = strdup("DetectorInit");
            pkg->server.cleanFunctionName = strdup("DetectorClean");
            pkg->server.validateFunctionName = strdup("DetectorValidate");
            if (!pkg->name || !pkg->server.initFunctionName || !pkg->server.cleanFunctionName || !pkg->server.validateFunctionName)
                _dpd.errMsg("failed to allocate package");

        }
        return;
    }

    /* Get all the variables */
    lua_getfield(L, -1, "name"); /* string */
    if (lua_isstring(L, -1))
    {
        pkg->name = strdup(lua_tostring(L, -1));
        if (!pkg->name)
            _dpd.errMsg("failed to allocate package name");
    }
    else if (fillDefaults)
    {
        pkg->name = strdup("NoName");
        if (!pkg->name)
            _dpd.errMsg("failed to allocate package name");
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "proto"); /* integer? */
    if (lua_isnumber(L, -1))
    {
        pkg->proto = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "client");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "init"); /* string*/
        if (lua_isstring(L, -1))
        {
            pkg->client.initFunctionName = strdup(lua_tostring(L, -1));
            if (!pkg->client.initFunctionName)
                _dpd.errMsg("failed to allocate client init function name");
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "clean"); /* string*/
        if (lua_isstring(L, -1))
        {
            pkg->client.cleanFunctionName = strdup(lua_tostring(L, -1));
            if (!pkg->client.cleanFunctionName)
                _dpd.errMsg("failed to allocate client clean function name");
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "validate"); /* string*/
        if (lua_isstring(L, -1))
        {
            pkg->client.validateFunctionName = strdup(lua_tostring(L, -1));
            if (!pkg->client.validateFunctionName)
                _dpd.errMsg("failed to allocate client validate function name");
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "minimum_matches"); /* integer*/
        if (lua_isnumber(L, -1))
        {
            pkg->client.minMatches = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);

    }
    lua_pop(L, 1);  /*pop client table */

    lua_getfield(L, -1, "server");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "init"); /* string*/
        if (lua_isstring(L, -1))
        {
            pkg->server.initFunctionName = strdup(lua_tostring(L, -1));
            if (!pkg->server.initFunctionName)
                _dpd.errMsg("failed to allocate server init function name");
        }
        else if (fillDefaults)
        {
            pkg->server.initFunctionName = strdup("DetectorInit");
            if (!pkg->server.initFunctionName)
                _dpd.errMsg("failed to allocate server init function name");
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "clean"); /* string*/
        if (lua_isstring(L, -1))
        {
            pkg->server.cleanFunctionName = strdup(lua_tostring(L, -1));
            if (!pkg->server.cleanFunctionName)
                _dpd.errMsg("failed to allocate server clean function name");
        }
        else if (fillDefaults)
        {
            pkg->server.cleanFunctionName = strdup("DetectorClean");
            if (!pkg->server.cleanFunctionName)
                _dpd.errMsg("failed to allocate server clean function name");
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "validate"); /* string*/
        if (lua_isstring(L, -1))
        {
            pkg->server.validateFunctionName = strdup(lua_tostring(L, -1));
            if (!pkg->server.validateFunctionName)
                _dpd.errMsg("failed to allocate server validate function name");
        }
        else if (fillDefaults)
        {
            pkg->server.validateFunctionName = strdup("DetectorValidate");
            if (!pkg->server.validateFunctionName)
                _dpd.errMsg("failed to allocate server validate function name");
        }
        lua_pop(L, 1);

    }
    lua_pop(L, 1);  /*pop server table */

    lua_pop(L, 1);  /*pop DetectorPackageInfo table */

}

/**Calls DetectorInit function inside lua detector.
 * Calls initialization function as defined in packageInfo, which reads either user defined name
 * or DetectorInit symbol. Pushes detectorUserData on stack as input parameter and the calls the
 * function. Notice * that on error, lua_state is not closed. This keeps faulty detectors around
 * without using it, but it keeps wrapping functions simpler.
 */
static void luaServerInit(
        lua_State *myLuaState,
        Detector *detector
        )
{
    if (!detector->packageInfo.server.initFunctionName)
    {
        _dpd.errMsg("chunk %s: DetectorInit() is not provided for server\n",detector->chunkName);
        return;
    }

    lua_getglobal(myLuaState, detector->packageInfo.server.initFunctionName);
    if (!lua_isfunction(myLuaState, -1))
    {
        _dpd.errMsg("chunk %s: does not contain DetectorInit() function\n",detector->chunkName);
        /*lua_close(myLuaState); */
        return;
    }

    /*first parameter is DetectorUserData */
    lua_rawgeti(detector->myLuaState, LUA_REGISTRYINDEX, detector->detectorUserDataRef);

    if (lua_pcall(myLuaState, 1, 1, 0) != 0)
    {
        _dpd.errMsg("error loading lua chunk %s, error %s\n",detector->chunkName, lua_tostring(myLuaState, -1));
        /*lua_close(myLuaState); */
        return;
    }
    else
    {
        if (detector->server.pServiceElement)
            detector->server.pServiceElement->ref_count = 1;
        _dpd.debugMsg(DEBUG_LOG,"Initialized %s\n",detector->chunkName);
    }
}
/**Calls init function inside lua detector.
 * Calls initialization function as defined in packageInfo. Pushes detectorUserData on stack
 * as input parameter and the calls the function. Notice * that on error, lua_state is not
 * closed. This keeps faulty detectors around without using it, but it keeps wrapping functions
 * simpler.
 */
static void luaClientInit(
        RNAClientAppModule *li)
{
    Detector *detector = (Detector *)li->userData;
    lua_State *myLuaState = detector->myLuaState;

    if (!detector->packageInfo.client.initFunctionName)
    {
        _dpd.errMsg("chunk %s: DetectorInit() is not provided for client\n",detector->chunkName);
        return;
    }
    /*printf ("readPackage beginning stack elements %d\n", lua_gettop(myLuaState)); */
    lua_getglobal(myLuaState, detector->packageInfo.client.initFunctionName);
    if (!lua_isfunction(myLuaState, -1))
    {
        _dpd.errMsg("chunk %s: does not contain DetectorInit() function\n",detector->chunkName);
        /*lua_close(myLuaState); */
        return;
    }

    /*first parameter is DetectorUserData */
    lua_rawgeti(detector->myLuaState, LUA_REGISTRYINDEX, detector->detectorUserDataRef);

    /*second parameter is a table containing configuration stuff. */
    lua_newtable(myLuaState);

    if (lua_pcall(myLuaState, 2, 1, 0) != 0)
    {
        _dpd.errMsg("Could not initialize the %s client app element: %s\n",li->name, lua_tostring(myLuaState, -1));
        /*lua_close(myLuaState); */
        return;
    }
    else
    {
        _dpd.debugMsg(DEBUG_LOG,"Initialized %s\n",detector->chunkName);
    }
}

static void luaClientFini(
        Detector *detector
        )
{
    lua_State *myLuaState = detector->myLuaState;

    if (!detector->packageInfo.client.cleanFunctionName)
    {
        return;
    }

    lua_getglobal(myLuaState, detector->packageInfo.client.cleanFunctionName);
    if (!lua_isfunction(myLuaState, -1))
    {
        _dpd.errMsg("chunk %s: does not contain DetectorFini() function\n",detector->chunkName);
        /*lua_close(myLuaState); */
        return;
    }

    /*first parameter is DetectorUserData */
    lua_rawgeti(detector->myLuaState, LUA_REGISTRYINDEX, detector->detectorUserDataRef);


    if (lua_pcall(myLuaState, 1, 1, 0) != 0)
    {
        _dpd.errMsg("Could not cleanup the %s client app element: %s\n",detector->chunkName, lua_tostring(myLuaState, -1));
        /*lua_close(myLuaState); */
    }
}

/**set tracker sizes on Lua detector sizes. Uses global module names to access functions.
 */
static inline void setLuaTrackerSize(
        lua_State *L,
        uint32_t numTrackers)
{

    /*change flow tracker size according to available memory calculation */
    lua_getglobal(L, "hostServiceTrackerModule");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "setHostServiceTrackerSize");
        if (lua_isfunction(L, -1))
        {
            lua_pushinteger (L, numTrackers);
            if (lua_pcall(L, 1, 0, 0) != 0)
            {
                _dpd.errMsg( "error setting tracker size");
            }
        }
    }
    else
    {
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG, "hostServiceTrackerModule.setHostServiceTrackerSize not found");
#endif
    }
    lua_pop(L, 1);

    /*change flow tracker size according to available memory calculation */
    lua_getglobal(L, "flowTrackerModule");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "setFlowTrackerSize");
        if (lua_isfunction(L, -1))
        {
            lua_pushinteger (L, numTrackers);
            if (lua_pcall(L, 1, 0, 0) != 0)
            {
                _dpd.errMsg( "error setting tracker size");
            }
        }
    }
    else
    {
#ifdef LUA_DETECTOR_DEBUG
        _dpd.debugMsg(DEBUG_LOG, "flowTrackerModule.setFlowTrackerSize not found");
#endif
    }
    lua_pop(L, 1);
}

static void luaCustomLoad(
        char *detectorName,
        char *validator,
        unsigned int validatorLen,
        char *chunkName,
        unsigned char * const digest
        )
{
    lua_State *myLuaState = createLuaState();
    SFXHASH_NODE *node;
    Detector *detector;
    RNAClientAppModule *cam = NULL;

    if (myLuaState == NULL)
    {
        _dpd.errMsg( "can not create new luaState");
        free(validator);
        return;
    }

    /*Load function works the same with Lua and Luac file. */
    if (luaL_loadbuffer(myLuaState, validator, validatorLen, "")  || lua_pcall(myLuaState, 0, 0, 0))
    {
        _dpd.errMsg("cannot run validator %s, error: %s\n",chunkName, lua_tostring(myLuaState, -1));
        lua_close(myLuaState);
        free(validator);
        return;
    }

    detector = createDetector(myLuaState, chunkName);
    if (!detector)
    {
        _dpd.errMsg("cannot allocate detector %s\n",chunkName);
        lua_close(myLuaState);
        free(validator);
        return;
    }

    getDetectorPackageInfo(myLuaState, detector, 0);

    detector->validatorBuffer = validator;
    /*detector->detector_version = version; */
    detector->isActive = 1;


#if 0
    if ((detector->packageInfo.proto != IPPROTO_TCP)
            && (detector->packageInfo.proto != IPPROTO_UDP))
    {
        _dpd.errMsg( "detector %s did not have a valid protocol (%u)",
                chunkName, (unsigned)detector->packageInfo.proto);
        freeDetector(detector);
        lua_close(myLuaState);
        return;
    }
#endif

    if (detector->packageInfo.server.initFunctionName)
    {
        /*add to active service list */
        detector->server.serviceModule.next = active_service_list;
        active_service_list = &detector->server.serviceModule;

        detector->server.serviceId = APP_ID_UNKNOWN;
        
        /*create a ServiceElement */
        if (checkServiceElement(detector))
        {
            detector->server.pServiceElement->validate = validateAnyService;
            detector->server.pServiceElement->userdata = detector;

            detector->server.pServiceElement->detectorType = DETECTOR_TYPE_DECODER;
        }
    }
    else
    {
        detector->client.appFpId = APP_ID_UNKNOWN;
        cam = &detector->client.appModule;
        cam->name = detector->packageInfo.name;
        cam->proto = detector->packageInfo.proto;
        cam->validate = validateAnyClientApp;
        cam->minimum_matches = detector->packageInfo.client.minMatches;
        cam->userData = detector;
        cam->api = getClientApi();
    }

    node = sfxhash_find_node(allocatedDetectorList, detectorName);
    if (node)
    {
        detector->next = node->data;
        node->data = detector;
    }
    else if (sfxhash_add(allocatedDetectorList, detectorName, detector))
    {
        _dpd.errMsg( "Failed to add to list.");
        freeDetector(detector);
        lua_close(myLuaState);
        return;
    }

    memcpy(detector->digest, digest, sizeof(detector->digest));

    _dpd.debugMsg(DEBUG_LOG,"Loaded detector %s\n",chunkName);

    gNumDetectors++;
    gNumActiveDetectors++;
}

/**Initializes Lua modules. Open lua and if available LuaJIT libraries, and registers all API modules.
 *
 * @return void
 */

void luaModuleInit(void)
{
    sflist_init(&allocatedFlowList);
    allocatedDetectorList = sfxhash_new(-1023, 0, 0, 0, 1,
                                        (SFXHASH_FREE_FCN)Detector_fini, (SFXHASH_FREE_FCN)Detector_fini, 1);
    if (!allocatedDetectorList)
    {
        _dpd.fatalMsg( "Failed to create the module hash");
        exit(-1);
    }
}

/**calculates Number of flow and host tracker entries for Lua detectors, given amount
 * of memory allocated to RNA (fraction of total system memory) and number of detectors
 * loaded in database. Calculations are based on CAICCI detector and observing memory
 * consumption per tracker.
 * @param rnaMemory - total memory RNA is allowed to use. This is calculated as a fraction of
 * total system memory.
 * @param numDetectors - number of lua detectors present in database.
 */
static inline uint32_t calculateLuaTrackerSize(u_int64_t rnaMemory, uint32_t numDetectors)
{
    u_int64_t detectorMemory = (rnaMemory/8);
    u_int64_t perTrackerMemory = (3700000*2/10000); /*derived from CAICCI run with 10,000 flow entries. Doubling for host entries. */
    unsigned numTrackers;
    if (!numDetectors)
        numDetectors = 1;
    numTrackers = (detectorMemory/perTrackerMemory)/numDetectors;
    _dpd.debugMsg(DEBUG_LOG,"calculated tracker size is %u\n",numTrackers);
    return (numTrackers > 10000)? 10000: numTrackers;
}

void loadCustomLuaModules(char *path)
{
    Detector *detector;
    int rval;
    glob_t globs;
    char pattern[PATH_MAX];
    unsigned n;
    FILE *file;
    uint8_t *validatorBuffer;
    int validatorBufferLen;
    char *detectorName;

    snprintf(pattern, sizeof(pattern), "%s/*.lua", path);

    memset(&globs, 0, sizeof(globs));
    rval = glob(pattern, 0, NULL, &globs);
    if (rval != 0 && rval != GLOB_NOMATCH)
    {
        _dpd.errMsg("Unable to read directory '%s'\n",pattern);
        return;
    }

    for (n = 0; n < globs.gl_pathc; n++)
    {
        unsigned char digest[16];
        MD5CONTEXT context;

        detectorName = strrchr(globs.gl_pathv[n], '/');
        if (!detectorName)
        {
            _dpd.errMsg("Invalid lua detector name '%s'\n",globs.gl_pathv[n]);
            continue;
        }
        if ((file = fopen(globs.gl_pathv[n], "r")) == NULL)
        {
            _dpd.errMsg("Unable to read lua detector '%s'\n",globs.gl_pathv[n]);
            continue;
        }
        detectorName++;

        /*Load lua file as a detector. */
        if (fseek(file, 0, SEEK_END))
        {
            _dpd.errMsg("Unable to seek lua detector '%s'\n",globs.gl_pathv[n]);
            continue;
        }
        validatorBufferLen = ftell(file);
        if (validatorBufferLen == -1)
        {
            _dpd.errMsg("Unable to return offset on lua detector '%s'\n",globs.gl_pathv[n]);
            continue;
        }
        if (fseek(file, 0, SEEK_SET))
        {
            _dpd.errMsg("Unable to seek lua detector '%s'\n",globs.gl_pathv[n]);
            continue;
        }

        if ((validatorBuffer = malloc(validatorBufferLen + 1)) == NULL)
        {
            _dpd.errMsg("Failed to allocate the user lua detector %s\n",globs.gl_pathv[n]);
            goto next;
        }
        fread(validatorBuffer, validatorBufferLen, 1, file);

        validatorBuffer[validatorBufferLen] = 0;

        MD5INIT(&context);
        MD5UPDATE(&context, validatorBuffer, validatorBufferLen);
        MD5FINAL(digest, &context);

        /*calculate md5sum and mark the detector active if matched. */
        detector = sfxhash_find(allocatedDetectorList, detectorName);
        if (detector)
        {

            if (!memcmp(digest, detector->digest, sizeof(digest)))
            {
                detector->isActive = 1;
                gNumActiveDetectors++;
                goto next;
            }
        }

        luaCustomLoad(detectorName, (char *)validatorBuffer, validatorBufferLen, globs.gl_pathv[n], digest);

next:
        fclose(file);
    }

    globfree(&globs);

}

void luaDetectorsLoad(void)
{
    char path[PATH_MAX];

    luaDetectorsUnload();
    
    /*loadSFLuaModules(); */

    snprintf(path, sizeof(path), "%s/odp/lua", appIdCommandConfig->app_id_detector_path);
    loadCustomLuaModules(path);
    snprintf(path, sizeof(path), "%s/custom/lua", appIdCommandConfig->app_id_detector_path);
    loadCustomLuaModules(path);
    luaDetectorsSetTrackerSize();
    luaDetectorsCleanInactive();
}

/** the inactive detectors are not deleted since some allocated values are used
 * in client and service lists.*/
static void luaDetectorsCleanInactive(void)
{
    Detector *detector;
    Detector *detector_list;
    SFXHASH_NODE *node;

    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector_list = node->data;
        for (detector = detector_list; detector; detector = detector->next)
        {
            if (!detector->isActive)
            {
                tDetectorPackageInfo  *pkg = &detector->packageInfo;

                if (detector->server.pServiceElement)
                    detector->server.pServiceElement->ref_count = 0;

                if (pkg->name)
                {
                    free(pkg->name);
                    pkg->name = NULL;

                }
                if (pkg->client.initFunctionName)
                {
                    free(pkg->client.initFunctionName);
                    pkg->client.initFunctionName = NULL;
                }
                if (pkg->client.cleanFunctionName)
                {
                    free(pkg->client.cleanFunctionName);
                    pkg->client.cleanFunctionName = NULL;
                }
                if (pkg->client.validateFunctionName)
                {
                    free(pkg->client.validateFunctionName);
                    pkg->client.validateFunctionName = NULL;
                }
                if (pkg->server.initFunctionName)
                {
                    free(pkg->server.initFunctionName);
                    pkg->server.initFunctionName = NULL;
                }
                if (pkg->server.cleanFunctionName)
                {
                    free(pkg->server.cleanFunctionName);
                    pkg->server.cleanFunctionName = NULL;
                }
                if (pkg->server.validateFunctionName)
                {
                    free(pkg->server.validateFunctionName);
                    pkg->server.validateFunctionName = NULL;
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

                if (detector->chunkName)
                {
                    free(detector->chunkName);
                    free(detector->validatorBuffer);
                    detector->chunkName = NULL;
                    detector->validatorBuffer = NULL;
                }


                if (detector->myLuaState)
                {
                    lua_close(detector->myLuaState);
                    detector->myLuaState = NULL;
                }
            }
        }
    }
}
void luaDetectorsUnload(void)
{
    Detector *detector;
    Detector *detector_list;
    SFXHASH_NODE *node;

    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector_list = node->data;
        for (detector = detector_list; detector; detector = detector->next)
        {
            if (detector->isActive && detector->packageInfo.server.initFunctionName)
                detectorRemoveAllPorts(detector);
            if (detector->isActive && detector->packageInfo.client.initFunctionName)
                luaClientFini(detector);
            detector->isActive = 0;
            if (detector->server.pServiceElement)
                detector->server.pServiceElement->ref_count = 0;
        }
    }
    gNumActiveDetectors = 0;
}

void luaDetectorsSetTrackerSize(void)
{
    Detector *detector;
    Detector *detector_list;
    SFXHASH_NODE *node;

    gLuaTrackerSize = calculateLuaTrackerSize(512*1024*1024, gNumActiveDetectors);

    _dpd.logMsg("    Setting tracker size to %u\n", gLuaTrackerSize);

    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector_list = node->data;
        for (detector = detector_list; detector; detector = detector->next)
            if (detector->isActive)
                setLuaTrackerSize(detector->myLuaState, gLuaTrackerSize);
    }
}

/**Reconfigure all Lua modules.
 * Iterates over all Lua detectors in system and reconfigures them. This
 * will however not read rna_csd_validator_map table again to check for
 * newly activated or deactivate detectors. Current design calls for restarting
 * RNA whenever detectors are activated/deactivated.
 */
void luaModuleInitAllServices(void)
{
    Detector *detector_list;
    Detector *detector;
    SFXHASH_NODE *node;

    /*reconfiguring server elements only. */
    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector_list = node->data;
        for (detector = detector_list; detector; detector = detector->next)
        {
            if (detector->isActive && detector->packageInfo.server.initFunctionName)
            {
                luaServerInit(detector->myLuaState, detector);
            }
        }
    }
}

/**Reconfigure all Lua modules.
 * Iterates over all Lua detectors in system and reconfigures them. This
 * will however not read rna_csd_validator_map table again to check for
 * newly activated or deactivate detectors. Current design calls for restarting
 * RNA whenever detectors are activated/deactivated.
 */
void luaModuleInitAllClients(void)
{
    Detector *detector_list;
    Detector *detector;
    SFXHASH_NODE *node;

    /*reconfiguring client elements. */
    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector_list = node->data;
        for (detector = detector_list; detector; detector = detector->next)
        {
            if (detector->isActive && detector->packageInfo.client.initFunctionName)
            {
                luaClientInit(&detector->client.appModule);
            }
        }
    }
}

void luaModuleCleanAllClients(void)
{
    Detector *detector;
    SFXHASH_NODE *node;

    /*reconfiguring client elements. */
    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector = node->data;
        if (detector->packageInfo.client.initFunctionName)
        {
            luaClientFini(detector);
        }

        /*dont free detector. Lua side reclaims the memory. */
    }
}

/**Finish routine for DetectorCore module. It release all Lua sessions and frees any memory.
 * @warn This function should be called once and that too when RNA is performing clean exit.
 * @return void.
  */
void luaModuleFini(void)
{

#ifdef LUA_DETECTOR_DEBUG
    _dpd.debugMsg(DEBUG_LOG, "luaModuleFini(): entered");
#endif

    /*flow can be freed during garbage collection */

    sflist_static_free_all(&allocatedFlowList, freeDetectorFlow);
    sfxhash_delete(allocatedDetectorList);
    allocatedDetectorList = NULL;
}

void RNAPndDumpLuaStats (void)
{
    Detector *detector_list;
    Detector *detector;
    SFXHASH_NODE *node;
    unsigned long long totalMem = 0;
    unsigned long long mem;


    _dpd.logMsg("Lua detector Stats");
    for (node = sfxhash_gfindfirst(allocatedDetectorList);
         node;
         node = sfxhash_gfindnext(allocatedDetectorList))
    {
        detector_list = node->data;
        for (detector = detector_list; detector; detector = detector->next)
        {
            if (detector->isActive)
            {
                mem = lua_gc(detector->myLuaState, LUA_GCCOUNT,0);
                totalMem += mem;
                _dpd.logMsg("    Detector %s: Lua Memory usage %d kb", detector->chunkName, mem);
            }
        }
    }
    _dpd.logMsg("Lua Stats total memory usage %d kb", totalMem);
}


/** @} */ /* end of LuaDetectorCore */

