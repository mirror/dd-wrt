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


#ifndef __LUA_DETECTOR_MODULE_H__
#define __LUA_DETECTOR_MODULE_H__


void luaModuleInit(void);
void luaModuleFini(void);
/**
 * \brief Load all Lua modules into a detector list
 *
 * Each RNA detector file in the folder \e app_id_detector_path is parsed for
 * detector information. If it is a valid detector, a detector data structure
 * is created for it and stored in \e allocatedDetectorList.
 *
 * @param - pointer to new AppId context
 * @return None
 */
void LoadLuaModules(tAppidStaticConfig* appidSC, tAppIdConfig *pConfig);

/**
 * \brief Finalize Lua modules
 *
 * This function should be called after LoadLuaModules(). It sets up proper AppId references
 * and tracker size for all the detectors.
 *
 * @param pConfig - pointer to active AppId context
 * @return void
 */
void FinalizeLuaModules(tAppIdConfig *pConfig);

/**
 * \brief Unload Lua modules
 *
 * This function cleans up all the data structures that were created for the Lua detectors
 * in a given AppId context. It should be called after FinalizeLuaModules().
 *
 * @param Pointer to old AppId context
 * @return None
 */
void UnloadLuaModules(tAppIdConfig *pConfig);
void luaModuleInitAllServices(void);
void luaModuleCleanAllClients(void);
void luaModuleInitAllClients(void);
void RNAPndDumpLuaStats (void);

void luaDetectorsUnload(tAppIdConfig *pConfig);
void luaDetectorsSetTrackerSize(void);

extern SF_LIST allocatedFlowList;

#endif

