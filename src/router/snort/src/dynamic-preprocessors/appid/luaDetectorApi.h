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


#ifndef _LUA_DETECTOR_API_H_
#define  _LUA_DETECTOR_API_H_

#include <sys/types.h>
#include <inttypes.h>
#include <pthread.h>
#include "client_app_base.h"
#include "service_base.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "service_api.h"
#include "client_app_api.h"
#include "appIdConfig.h"
#include "profiler.h"

typedef struct
{
    char *name;
    int proto;
    struct
    {
        char *initFunctionName;     /*client init function */
        char *cleanFunctionName;    /*client clean function */
        char *validateFunctionName; /*client validate function */
        int minMatches;             /*minimum number of matched patterns */
    } client;

    struct
    {
        char *initFunctionName;     /*server init function */
        char *cleanFunctionName;    /*server clean function */
        char *validateFunctionName; /*server validate function */
    } server;
}tDetectorPackageInfo;


typedef struct _Detector
{
    struct _Detector *next;

    /**Identifies customer created detectors using SDL. */
    unsigned isCustom:1;
    unsigned isActive:1;
    unsigned wasActive:1;

    struct
    {
        const uint8_t *data;
        uint16_t size;
        int dir;
        tAppIdData *flowp;
        SFSnortPacket *pkt;
        uint8_t macAddress[6];
    } validateParams;

    /**Pointer to flow created by a validator.
     */
    tAppIdData *pFlow;

    struct
    {
        unsigned int serviceId;

        /**present only for server detectors*/
        struct RNAServiceValidationModule serviceModule;

        /**calloced buffer to satisfy internal flow API.
        */
        struct RNAServiceElement *pServiceElement;

    } server;

    /**constructed from packageInfo read from lua detector directly. Present
     * only for client detectors.
     */
    struct
    {
        /**application fingerprint id.*/
        unsigned int appFpId;

        /**Client Application Module. */
        tRNAClientAppModule appModule;

    } client;

    char *callbackFcnName;

    lua_State *myLuaState;

    /**Reference to lua userdata. This is a key into LUA_REGISTRYINDEX */
    int detectorUserDataRef;

    /**Detector name. Lua file name is used as detector name*/
    char *name;

    /**Package information retrieved from detector lua file.
     */
    tDetectorPackageInfo packageInfo;

    unsigned detector_version;
    char *validatorBuffer;
    unsigned char digest[16];

    tAppIdConfig *pAppidActiveConfig;     ///< AppId context in which this detector should be used; used during packet processing
    tAppIdConfig *pAppidOldConfig;        ///< AppId context in which this detector should be cleaned; used at reload free and exit
    tAppIdConfig *pAppidNewConfig;        ///< AppId context in which this detector should be loaded; used at initialization and reload

#ifdef PERF_PROFILING
    /**Snort profiling stats for individual Lua detector.*/
    struct _PreprocStats *pPerfStats;
#endif

    pthread_mutex_t luaReloadMutex;

} Detector;

/**data directly accessed by Lua code should be here.
 */
typedef struct {
    /**points to detector allocated on the C side. This is needed to get detector from callback functions. This pointer must be set to
     * NULL when Detector is destroyed.
     */
    Detector *pDetector;

    /*lua accessible data elements should be defined below. */
} DetectorUserData;

int Detector_register (
        lua_State *L
        );
DetectorUserData *checkDetectorUserData (
        lua_State *L,
        int index
        );
void Detector_fini(void *detector);
void detectorRemoveAllPorts (
                             Detector *detector,
                             tAppIdConfig *pConfig
        );
Detector *createDetector(
        lua_State *L,
        const char *filename
        );
void freeDetector(
        Detector *detector
        );
int validateAnyClientApp(
        const uint8_t *data,
        uint16_t size,
        const int dir,
        tAppIdData *flowp,
        SFSnortPacket *pkt,
        Detector *userdata,
        const tAppIdConfig *pConfig
        );

enum httpPatternType
{
    HTTP_PAYLOAD    = 1,
	HTTP_USER_AGENT = 2,
    HTTP_URL        = 3
};

int Detector_addSSLCertPattern (lua_State *);
int Detector_addDNSHostPattern (lua_State *);

int Detector_addHttpPattern(lua_State *L);

void CleanHttpPatternLists(tAppIdConfig *pConfig);
void CleanClientPortPatternList(tAppIdConfig*);
void CleanServicePortPatternList(tAppIdConfig*);

int validateAnyService(ServiceValidationArgs *args);
int checkServiceElement( Detector *detector);
#endif

