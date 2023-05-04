/* $Id$ */

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
/* Snort Detection Plugin Header for IP Fragment Bits plugin*/

#ifndef __SP_IP_FRAGBITS_H__
#define __SP_IP_FRAGBITS_H__

void SetupFragBits(void);
void SetupFragOffset(void);
uint32_t IpFragBitsCheckHash(void *d);
int IpFragBitsCheckCompare(void *l, void *r);
uint32_t IpFragOffsetCheckHash(void *d);
int IpFragOffsetCheckCompare(void *l, void *r);

#endif  /* __SP_IP_FRAGBITS_H__ */
