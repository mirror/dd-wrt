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
 * sftarget_protocol_reference.h
 */

#ifndef SFTARGET_PROTOCOL_REFERENCE_TABLE_H_
#define SFTARGET_PROTOCOL_REFERENCE_TABLE_H_

#include "decode.h"
#include "util.h"

#define MAX_PROTOCOL_ORDINAL 8192

typedef struct _SFTargetProtocolReference
{
    char name[STD_BUF];
    int16_t ordinal;
} SFTargetProtocolReference;

extern int16_t protocolReferenceTCP;
extern int16_t protocolReferenceUDP;
extern int16_t protocolReferenceICMP;

void InitializeProtocolReferenceTable(void);
void FreeProtoocolReferenceTable(void);
int16_t AddProtocolReference(const char *protocol);
int16_t FindProtocolReference(const char *protocol);
#if defined(FEAT_OPEN_APPID)
const char * FindProtocolName(int16_t protocol_id);
#endif /* defined(FEAT_OPEN_APPID) */

int16_t GetProtocolReference(Packet *p);

#endif /* SFTARGET_PROTOCOL_REFERENCE_TABLE_H_ */
