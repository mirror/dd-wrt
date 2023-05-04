/*
**
**  reload_api.h
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#ifndef __RELOAD_API_H__
#define __RELOAD_API_H__

#ifdef SNORT_RELOAD

/* 
  The reload adjust function should do incremental adjustments to the running memory.
  The reload adjust function is registered using ReloadAdjustRegister from snort proper
  and _dpd.reloadAdjustRegister for plugins.
  The idle flag should be used to do more work when there is no traffic flowing.
  The user data is optional data needed to adjust the running memory.
  The function should return true if the adjustments are complete.
*/
typedef bool (*ReloadAdjustFunc)(bool idle, tSfPolicyId raPolicyId, void* userData);
/*
  The reload adjust user free function is a function that can free the option user data
  associated with the reload adjust function.
*/
typedef void (*ReloadAdjustUserFreeFunc)(void* userData);

#endif

#endif

