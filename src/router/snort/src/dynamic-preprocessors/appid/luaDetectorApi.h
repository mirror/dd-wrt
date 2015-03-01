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


#ifndef _LUA_DETECTOR_API_H_
#define  _LUA_DETECTOR_API_H_

#include <sys/types.h>
#include <inttypes.h>
#include "client_app_base.h"
#include "service_base.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "service_api.h"
#include "client_app_api.h"

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
    unsigned isCustomDetector:1;
    unsigned isActive:1;

    struct
    {
        const uint8_t *data;
        uint16_t size;
        int dir;
        tAppIdData *flowp;
        const SFSnortPacket *pkt;
        uint8_t macAddress[6];
    } validateParams;

    /**Pointer to flow created by a validator.
     */
    tAppIdData *pFlow;

    struct
    {
        unsigned int serviceId;

        /**present only for server detectors*/
        RNAServiceValidationModule serviceModule;

        /**calloced buffer to satisfy internal flow API.
        */
        RNAServiceElement *pServiceElement;

    } server;

    /**constructed from packageInfo read from lua detector directly. Present
     * only for client detectors.
     */
    struct
    {
        /**application fingerprint id.*/
        unsigned int appFpId;

        /**Client Application Module. */
        RNAClientAppModule appModule;

    } client;

    lua_State *myLuaState;

    /**Reference to lua userdata. This is a key into LUA_REGISTRYINDEX */
    int detectorUserDataRef;

    /**Name assigned to Lua code (chunk)*/
    char *chunkName;

    /**Package information retrieved from detector lua file.
     */
    tDetectorPackageInfo packageInfo;

    unsigned detector_version;
    char *validatorBuffer;
    unsigned char digest[16];
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
int Detector_fini(void *key, void *detector);
void detectorRemoveAllPorts (
        Detector *detector
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
        const SFSnortPacket *pkt,
        Detector *userdata
        );

enum httpPatternType
{
    HTTP_PAYLOAD    = 1,
	HTTP_USER_AGENT = 2,
    HTTP_URL        = 3
};

int Detector_addSSLCertPattern (lua_State *);

int Detector_addHttpPattern(lua_State *L);

void CleanHttpPatternLists(int exiting);

int validateAnyService(
        const uint8_t *data,
        uint16_t size,
        const int dir,
        FLOW *flowp,
        const SFSnortPacket *pkt,
        Detector *detector
        );
int checkServiceElement( Detector *detector);
#endif

