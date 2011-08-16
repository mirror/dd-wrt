/*
 * ftp_cmd_lookup.h
 *
 * Copyright (C) 2004-2011 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
 * Kevin liu <kliu@sourcefire.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Description:
 *
 * This file contains function definitions for FTP command lookups.
 *
 * NOTES:
 * - 16.09.04:  Initial Development.  SAS
 *
 */
#ifndef __FTP_CMD_LOOKUP_H__
#define __FTP_CMD_LOOKUP_H__

#include "ftpp_include.h"
#include "ftpp_ui_config.h"

int ftp_cmd_lookup_init(CMD_LOOKUP **CmdLookup);
int ftp_cmd_lookup_cleanup(CMD_LOOKUP **CmdLookup);
int ftp_cmd_lookup_add(CMD_LOOKUP *CmdLookup, char cmd[], int len, FTP_CMD_CONF *FTPCmd);

FTP_CMD_CONF *ftp_cmd_lookup_find(CMD_LOOKUP *CmdLookup, const char cmd[], int len, int *iError);
FTP_CMD_CONF *ftp_cmd_lookup_first(CMD_LOOKUP *CmdLookup, int *iError);
FTP_CMD_CONF *ftp_cmd_lookup_next(CMD_LOOKUP *CmdLookup, int *iError);

#endif
