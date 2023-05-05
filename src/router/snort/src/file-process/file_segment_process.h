/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 ****************************************************************************
 *
 ****************************************************************************/
/*
 * File data reassemble and processing
 * Author(s):  Hui Cao <huica@cisco.com>
 *
 */

#ifndef _FILE_PROCESS_H_
#define _FILE_PROCESS_H_

#include <sys/types.h>
#include "sf_types.h"
#include "sfxhash.h"
#include "file_lib.h"
#include "decode.h"
#include "ipv6_port.h"
#include "util.h"

typedef struct _FileCache
{
    SFXHASH *hashTable;
    uint32_t max_files;
    uint32_t cleanup_files;
    FileCacheStatus status;
    uint64_t file_segment_memcap;
} FileCache;

typedef struct _FileSegment
{
    /*Use single list for simplicity*/
    struct _FileSegment *next;

    uint32_t   offset;
    uint32_t   size;   /* actual data size*/
    uint32_t   segment_size; /* memory allocated, including headers */
    uint8_t    data[1];  /* variable length */

} FileSegment;

typedef struct _FileKey
{
    struct in6_addr sip;
    struct in6_addr dip;
    uint64_t file_id;
} FileKey;

typedef struct _FileEntry
{
    FileContext *context;
    uint8_t *file_name;
    uint64_t offset;
    FileSegment *segments;
    uint64_t file_size;
    FileCache *file_cache;
    uint32_t file_name_size;
    bool file_resume_check;
} FileEntry;

/* Create file cache to store file segments and track file
 * memcap: total memory available for file cache, including file contexts
 * cleanup_files: maximal number of files pruned when memcap reached
 */
FileCache *file_cache_create(uint64_t memcap, uint32_t cleanup_files);

/* Free file cache */
void file_cache_free(FileCache *fileCache);

/* Get the status of file cache*/
FileCacheStatus *file_cache_status(FileCache *fileCache);

bool file_cache_shrink_to_memcap(FileCache *fileCache, uint8_t *pWork);
void file_cache_set_memcap(FileCache *fileCache, uint64_t memcap);

/* Add/update a file entry in the file cache*/
void *file_cache_update_entry (FileCache *fileCache, void* p, uint64_t file_id,
        uint8_t *file_name, uint32_t file_name_size, uint64_t file_size, bool reset, 
        bool no_update_size);

/* Process file segments with offset specified. If file segment is out of order,
 * it will be put into the file seglist. Otherwise, it will be processed.
 *
 * Return:
 *    1: continue processing/log/block this file
 *    0: ignore this file
 */
int file_segment_process( FileCache *fileCache, void* p, uint64_t file_id,
		uint64_t file_size, const uint8_t* file_data, int data_size, uint64_t offset,
		bool upload);

#endif  /* _FILE_PROCESS_H_ */

