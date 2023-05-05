/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2006-2013 Sourcefire, Inc.
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

/*
 * Author: Steven Sturges
 * sftarget_hostentry.h
 */

#ifndef _SFTARGET_HOSTENTRY_H_
#define _SFTARGET_HOSTENTRY_H_

#include "sftarget_reader.h"

#define SFTARGET_MATCH 1
#define SFTARGET_NOMATCH 0

/* API for HostAttributeEntry 'class' */

int hasService(HostAttributeEntry *hostEntry,
               int ipprotocol,
               int protocol,
               int application);
int hasClient(HostAttributeEntry *hostEntry,
               int ipprotocol,
               int protocol,
               int application);
int hasProtocol(HostAttributeEntry *hostEntry,
               int ipprotocol,
               int protocol,
               int application);

int getProtocol(HostAttributeEntry *hostEntry,
               int ipprotocol,
               uint16_t port);

int getApplicationProtocolId(HostAttributeEntry *host_entry,
               int ipprotocol,
               uint16_t port,
               char direction);

#define SFAT_UNKNOWN_STREAM_POLICY 0
uint16_t getStreamPolicy(HostAttributeEntry *host_entry);
char isStreamPolicySet(HostAttributeEntry *host_entry);
#define SFAT_UNKNOWN_FRAG_POLICY 0
uint16_t getFragPolicy(HostAttributeEntry *host_entry);
char isFragPolicySet(HostAttributeEntry *host_entry);

#endif /* _SFTARGET_HOSTENTRY_H_ */
