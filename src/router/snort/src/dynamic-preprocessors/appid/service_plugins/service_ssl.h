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


#ifndef __SERVICE_SSL_H__
#define __SERVICE_SSL_H__

#include "service_api.h"

extern struct RNAServiceValidationModule ssl_service_mod;
tAppId getSslServiceAppId(short srcPort);
bool isSslServiceAppId(tAppId appId);
void service_ssl_clean(tServiceSslConfig *);
int ssl_detector_process_patterns(tServiceSslConfig *);
int ssl_scan_hostname(const u_int8_t*, size_t, tAppId*, tAppId*,tServiceSslConfig *);
int ssl_scan_cname(const u_int8_t*, size_t, tAppId*, tAppId*,tServiceSslConfig *);
int ssl_add_cert_pattern(uint8_t *, size_t, uint8_t, tAppId,tServiceSslConfig *);
int ssl_add_cname_pattern(uint8_t *, size_t, uint8_t, tAppId,tServiceSslConfig *);
void ssl_detector_free_patterns(tServiceSslConfig *);
int setSSLSquelch(SFSnortPacket *p, int type, tAppId appId);

#endif  /* __SERVICE_SSL_H__ */

