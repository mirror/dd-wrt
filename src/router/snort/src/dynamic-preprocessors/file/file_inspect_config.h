/* $Id */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2013-2013 Sourcefire, Inc.
**
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

/*
 * Definitions, structs, function prototype(s) for
 *		the file preprocessor.
 * Author: Hui Cao
 */

#ifndef SPP_FILE_INSPECT_CONFIG_H
#define SPP_FILE_INSPECT_CONFIG_H

#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"
#include "file_sha.h"

#define FILE_CAPTURE_QUEUE_SIZE_DEFAULT       3000 /*files*/
#define FILE_CAPTURE_DISK_SIZE_DEFAULT        300  /*MB*/

typedef struct _FileSigInfo
{
    File_Verdict verdict;
} FileSigInfo;

/*
 * Global File preprocessor configuration.
 *
 */
typedef struct _fileInspectConfig
{
    bool file_type_enabled;
    bool file_signature_enabled;
    bool file_capture_enabled;
    uint32_t file_capture_queue_size;
    char *capture_dir;
    int ref_count;
    char *hostname;
    int portno;
    ShaHash *sig_table;
#if defined(DEBUG_MSGS) || defined (REG_TEST)
    int verdict_delay; /* used for debug, mimic delay to get verdicts */
#endif
    uint32_t capture_disk_size;  /* In megabytes*/

} FileInspectConf;


void file_config_parse(FileInspectConf*, const u_char* );

/* Return values
 *  0: equal
 *  -1: no the same
 */
int file_config_compare(FileInspectConf*  , FileInspectConf* );

/* Release resource of file configruation*/
void file_config_free(FileInspectConf*);

#endif /* SPP_FILE_INSPECT_CONFIG_H */
