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

#ifndef SPP_FILE_H
#define SPP_FILE_H

#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "snort_bounds.h"

typedef struct _File_Stats {

    uint64_t file_types_total;  /* Total number of file type callbacks */
    uint64_t file_signatures_total;  /* Total number of file signature callbacks */
    uint64_t files_to_disk_total;  /* Files needed to be saved on disk */
    uint64_t file_duplicates_total; /* Files already on disk */
    uint64_t files_saved;          /* Files saved on disk */
    uint64_t file_reserve_failures;
    uint64_t file_capture_min;
    uint64_t file_capture_max;
    uint64_t file_capture_memcap;
    uint64_t file_read_failures;
    uint64_t file_agent_memcap_failures;
    uint64_t file_data_to_disk; /*file data stored */
    uint64_t files_to_host_total; /*files sent */
    uint64_t file_data_to_host;  /*file data sent */
    uint64_t file_transfer_failures; /*file transfer failures */

} File_Stats;

/* Prototypes for public interface, initialzie file inspect functions */
extern void SetupFileInspect(void);

extern File_Stats file_inspect_stats;
extern tSfPolicyUserContextId file_config;

#define FILE_FATAL_ERROR  DynamicPreprocessorFatalMessage

#endif /* SPP_FILE_H */
