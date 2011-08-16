/*
** Copyright (C) 2006-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

void InitializeProtocolReferenceTable(void);
void FreeProtoocolReferenceTable(void);
int16_t AddProtocolReference(char *protocol);
int16_t FindProtocolReference(char *protocol);

int16_t GetProtocolReference(Packet *p);

#endif /* SFTARGET_PROTOCOL_REFERENCE_TABLE_H_ */
