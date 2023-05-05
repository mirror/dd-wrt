/* $Id$ */

/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
**
** Author: davis mcpherson
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

#ifndef __SPP_SESSION_H__
#define __SPP_SESSION_H__

#include "decode.h"
#include "session_common.h"

/* list of function prototypes for this preprocessor */
void SetupSessionManager(void);
void SessionReload(struct _SessionCache* lws_cache, uint32_t max_sessions,
                   uint32_t aggressiveTimeout, uint32_t nominalTimeout
#ifdef REG_TEST
                   , const char* name
#endif
                   );
unsigned SessionProtocolReloadAdjust(struct _SessionCache* lws_cache, uint32_t max_sessions,
                                     unsigned maxWork, uint32_t memcap
#ifdef REG_TEST
                                     , const char* name
#endif
                                     );

#endif  /* __SPP_SESSION_H__ */
