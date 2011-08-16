/* $Id$ */

/*
** Copyright (C) 2005-2011 Sourcefire, Inc.
** AUTHOR: Steven Sturges
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

/* stream_ignore.h
 * 
 * Purpose: Handle hash table storage and lookups for ignoring
 *          entire data streams.
 *
 * Arguments:
 *   
 * Effect:
 *
 * Comments: Used by Stream4 & Stream5 -- don't delete too soon.
 *
 * Any comments?
 *
 */

#ifndef STREAM_IGNORE_H_
#define STREAM_IGNORE_H_

#include "ipv6_port.h"

int IgnoreChannel(snort_ip_p cliIP, uint16_t cliPort,
                  snort_ip_p srvIP, uint16_t srvPort,
                  char protocol, char direction, char flags,
                  uint32_t timeout, int16_t appId);

char CheckIgnoreChannel(Packet *, int16_t *appId);
void CleanupIgnore(void);

#endif /* STREAM_IGNORE_H_ */

