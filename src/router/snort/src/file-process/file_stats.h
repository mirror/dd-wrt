/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2013-2013 Sourcefire, Inc.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  5.25.13 - Initial Source Code. Hcao
 */

#ifndef __FILE_STATS_H__
#define __FILE_STATS_H__

#ifdef TARGET_BASED
#include "sftarget_protocol_reference.h"
#include "sftarget_reader.h"
#endif
#include "file_config.h"
#include "file_api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct _File_Stats {

    uint64_t files_total;
    uint64_t files_processed[FILE_ID_MAX + 1][2];
    uint64_t signatures_processed[FILE_ID_MAX + 1][2];
    uint64_t verdicts_type[FILE_VERDICT_MAX];
    uint64_t verdicts_signature[FILE_VERDICT_MAX];
#ifdef TARGET_BASED
    uint64_t files_by_proto[MAX_PROTOCOL_ORDINAL + 1];
    uint64_t signatures_by_proto[MAX_PROTOCOL_ORDINAL + 1];
#endif
    uint64_t data_processed[FILE_ID_MAX + 1][2];
    uint64_t file_data_total;
    uint64_t files_sig_depth;

} FileStats;

extern FileStats file_stats;


#ifdef REG_TEST
#define FILE_REG_DEBUG_WRAP(code) code
#else
#ifdef DEBUG_MSGS
#define FILE_REG_DEBUG_WRAP(code) if (DEBUG_FILE & GetDebugLevel()){code}
#else
#define FILE_REG_DEBUG_WRAP(code)
#endif
#endif

#define FILE_DEBUG_MSGS(msg) DEBUG_WRAP(DebugMessage(DEBUG_FILE, msg);)

#if defined(DEBUG_MSGS) || defined (REG_TEST)
void printFileContext(FileContext* context);
void DumpHexFile(FILE *fp, const uint8_t *data, unsigned int len);
#endif

/*
 * Print out file statistics
 */
void print_file_stats(int exiting);
#endif

