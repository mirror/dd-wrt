/* $Id$ */
/*
 ** Copyright (C) 1998-2011 Sourcefire, Inc.
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
/**
 * @file   packet_time.c
 * @author Chris Green <cmg@sourcefire.com>
 * @date   Tue Jun 17 17:09:59 2003
 * 
 * @brief  Easily allow modules to have a gettimeofday() based on packet time
 * 
 * In many modules in snort, especially the rate detectors need to
 * work based off time values.  It's very hard to reproduce time
 * constraints via pcap readbacks so we either have to throttle snort
 * or use the packet time.  I choose the latter.
 */

#include "packet_time.h"

static time_t s_first_packet  = 0;
static time_t s_recent_packet = 0;

void packet_time_update(time_t cur)
{
    if(s_first_packet == 0)
    {
        s_first_packet = cur;
    }

    s_recent_packet = cur;
}

time_t packet_timeofday(void)
{
    return s_recent_packet;
}

time_t packet_first_time(void)
{
    return s_first_packet;
}
