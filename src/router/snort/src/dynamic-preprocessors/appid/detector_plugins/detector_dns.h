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

#ifndef __DETECTOR_DNS_H__
#define __DETECTOR_DNS_H__

#include <netinet/in.h>

#include "service_api.h"
#include "serviceConfig.h"

extern struct RNAServiceValidationModule dns_service_mod;
extern struct RNAClientAppModule dns_udp_client_mod;
extern struct RNAClientAppModule dns_tcp_client_mod;

int dns_host_scan_hostname(const u_int8_t*, size_t, tAppId*, tAppId*, const tServiceDnsConfig*);
void service_dns_host_clean(tServiceDnsConfig *pConfig);
int dns_host_detector_process_patterns(tServiceDnsConfig *pConfig);
int dns_add_host_pattern(uint8_t *, size_t , uint8_t , tAppId , tServiceDnsConfig *);
void dns_detector_free_patterns(tServiceDnsConfig *pConfig);
char *dns_parse_host(const uint8_t *host, uint8_t host_len);

#endif  /* __DETECTOR_DNS_H__ */
