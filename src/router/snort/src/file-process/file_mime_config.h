/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
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
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  9.25.2012 - Initial Source Code. Hcao
 */


#ifndef __FILE_MIME_CONFIG_H__
#define __FILE_MIME_CONFIG_H__

#include "file_api.h"
#include "file_mail_common.h"

/* Function prototypes  */
void set_mime_decode_config_defauts(DecodeConfig *decode_conf);
void set_mime_log_config_defauts(MAIL_LogConfig *log_config);
int parse_mime_decode_args(DecodeConfig *decode_conf, char *arg, const char *preproc_name, char **strtok_saveptr);
bool is_decoding_enabled(DecodeConfig *decode_conf);
bool is_mime_log_enabled(MAIL_LogConfig *log_config);
bool check_decode_config(DecodeConfig *currentConfig, DecodeConfig *defaultConfig, const char *preproc_name);
#endif

