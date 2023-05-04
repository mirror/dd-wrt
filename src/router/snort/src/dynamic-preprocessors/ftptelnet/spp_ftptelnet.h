/*
 * spp_ftptelnet.h
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Description:
 *
 * This file defines the publicly available functions for the FTPTelnet
 * functionality for Snort.
 *
 * NOTES:
 * - 16.09.04:  Initial Development.  SAS
 *
 */
#ifndef __SPP_FTPTELNET_H__
#define __SPP_FTPTELNET_H__

typedef struct _FTPTelnet_Stats
{
   uint64_t ftp_sessions; //Current sessions
   uint64_t max_ftp_sessions; //Max cuncurrent sessions
   uint64_t telnet_sessions;//Current sessions
   uint64_t max_telnet_sessions; //Max cuncurrent sessions
   uint64_t ftp_data_sessions;
   uint64_t max_ftp_data_sessions;
   uint64_t heap_memory;
} FTPTelnet_Stats;

extern FTPTelnet_Stats ftp_telnet_stats;

void SetupFTPTelnet(void);

#endif
