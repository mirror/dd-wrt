/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */

#ifndef __SP_IP_PROTO_H__
#define __SP_IP_PROTO_H__

#include "rules.h"
#include "treenodes.h"
#include "sf_types.h"

void SetupIpProto(void);
uint32_t IpProtoCheckHash(void *);
int IpProtoCheckCompare(void *, void *);
int GetOtnIpProto(OptTreeNode *);
int GetIpProtos(void *, uint8_t *, int);

#endif  /* __SP_IP_PROTO_H__ */
