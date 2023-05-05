/* $Id$ */

/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
**
** Author: Steven Sturges
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

#ifndef __SPP_STREAM6_H__
#define __SPP_STREAM6_H__

#include "decode.h"
#include "stream_common.h"

/* list of function prototypes for this preprocessor */
void SetupStream6(void);

#if defined(FEAT_OPEN_APPID)
void CallHttpHeaderProcessors(Packet* p, HttpParsedHeaders * const headers);
bool IsAnybodyRegisteredForHttpHeader(void);
#endif /* defined(FEAT_OPEN_APPID) */

#endif  /* __SPP_STREAM6_H__ */
