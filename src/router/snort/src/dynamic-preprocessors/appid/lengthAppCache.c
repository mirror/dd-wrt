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

#include "lengthAppCache.h"

#include <netinet/in.h>

#include "flow.h"
#include "sfutil.h"
#include "commonAppMatcher.h"
#include "appIdConfig.h"

#define HASH_NUM_ROWS (1024)

void lengthAppCacheInit(tAppIdConfig *pConfig)
{
    if (!(pConfig->lengthCache = sfxhash_new(HASH_NUM_ROWS,
                                    sizeof(tLengthKey),
                                    sizeof(tAppId),
                                    0,
                                    0,
                                    NULL,
                                    NULL,
                                    0)))
    {
        _dpd.errMsg("lengthAppCache: Failed to allocate length cache!");
    }
}

void lengthAppCacheFini(tAppIdConfig *pConfig)
{
    if (pConfig->lengthCache)
    {
        sfxhash_delete(pConfig->lengthCache);
        pConfig->lengthCache = NULL;
    }
}

tAppId lengthAppCacheFind(const tLengthKey *key, const tAppIdConfig *pConfig)
{
    tAppId *val;

    val = (tAppId*)sfxhash_find(pConfig->lengthCache, (void *)key);
    if (val == NULL)
    {
        return APP_ID_NONE;    /* no match */
    }
    else
    {
        return *val;           /* match found */
    }
}

int lengthAppCacheAdd(const tLengthKey *key, tAppId val, tAppIdConfig *pConfig)
{
    if (sfxhash_add(pConfig->lengthCache, (void *)key, (void *)&val))
    {
        return 0;
    }
    return 1;    /* OK */
}
