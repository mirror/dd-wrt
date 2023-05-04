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


#ifndef _LUA_DETECTOR_APPID_SESSION_API_H_
#define  _LUA_DETECTOR_APPID_SESSION_API_H_

#include "flow.h"
#include "luaDetectorApi.h"

typedef struct {
    /**Lua_state. */
    lua_State *myLuaState;

    /**Pointer to flow created by a validator.
     */
    tAppIdData *pFlow;

    /**Reference to lua userdata. This is a key into LUA_REGISTRYINDEX */
    int userDataRef;

} DetectorFlow;

/**Allocated on Lua side.*/
typedef struct {
    /**Pointer to DetectorFlow created on C side.
     */
    DetectorFlow *pDetectorFlow;

} DetectorFlowUserData;

int DetectorFlow_register (
        lua_State *L
        );
DetectorFlowUserData *pushDetectorFlowUserData (
        lua_State *L
        );
void freeDetectorFlow(
        void *userdata
        );
#endif
