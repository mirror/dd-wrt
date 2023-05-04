/*
**
**  sfcontrol.c
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2002-2013 Sourcefire, Inc.
**  Author(s):  Ron Dempster <rdempster@sourcefire.com>
**
**  NOTES
**  5.16.11 - Initial Source Code. Dempster
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

#ifndef __SF_CONTROL_FUNCS_H__
#define __SF_CONTROL_FUNCS_H__

#include "sfcontrol.h"

void ControlSocketConfigureDirectory(const char *optarg);
void ControlSocketInit(void);
void ControlSocketCleanUp(void);
int ControlSocketRegisterHandler(uint16_t type, OOBPreControlFunc oobpre, IBControlFunc ib,
                                 OOBPostControlFunc oobpost);

#ifdef CONTROL_SOCKET
void ControlSocketDoWork(int idle);
#else
#define ControlSocketDoWork(idle)   do {} while(0)
#endif

#endif

