/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** 
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


#ifndef __SP_APP_ID_OPTION_H__
#define __SP_APP_ID_OPTION_H__

typedef struct _AppIdInfo
{
    char *appName;
    int16_t appid_ordinal;
} AppIdInfo;

typedef struct _AppIdOptionData
{
    int16_t  matched_appid;
    unsigned num_appid;
    unsigned num_appid_allocated;
    AppIdInfo* appid_table;
} AppIdOptionData;

void SetupAppId(void);
AppIdOptionData* optionAppIdCreate();
void optionAppIdFree(AppIdOptionData *optData);
uint32_t optionAppIdHash(const void *option);
int optionAppIdCompare(const void * _left, const void * _right);

#endif  
