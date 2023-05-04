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
#ifndef __SP_ICMP_TYPE_CHECK_H__
#define __SP_ICMP_TYPE_CHECK_H__

#define ICMP_TYPE_TEST_EQ 1
#define ICMP_TYPE_TEST_GT 2
#define ICMP_TYPE_TEST_LT 3
#define ICMP_TYPE_TEST_RG 4

typedef struct _IcmpTypeCheckData
{
    /* the icmp type number */
    int icmp_type;
    int icmp_type2;
    uint8_t operator;
} IcmpTypeCheckData;

void SetupIcmpTypeCheck(void);
uint32_t IcmpTypeCheckHash(void *d);
int IcmpTypeCheckCompare(void *l, void *r);

#endif  /* __SP_ICMP_TYPE_CHECK_H__ */
