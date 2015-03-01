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


#ifndef _HOST_PORT_APP_CACHE_
#define _HOST_PORT_APP_CACHE_

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "appId.h"
#include "sf_snort_packet.h"

void hostPortAppCacheInit(void);
void hostPortAppCacheFini(void);
tAppId hostPortAppCacheFind(const snort_ip *ip, uint16_t port, uint16_t proto);
int hostPortAppCacheAdd(const struct in6_addr *ip, uint16_t port, uint16_t proto, unsigned type, tAppId appId);
void hostPortAppCacheDump(void);

#endif
