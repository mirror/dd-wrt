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
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  9.25.2012 - Initial Source Code. Hcao
 */

#ifndef _FILE_AGENT_H_
#define _FILE_AGENT_H_

#include "file_api.h"
#include "file_inspect_config.h"
#include "file_sha.h"

#define FILE_NAME_LEN             200


typedef struct _FileInfo
{
    char sha256[SHA256_HASH_SIZE];
    size_t file_size;
    void *file_mem;

} FileInfo;


/* Initialize file processing.
 * This should be called when this preprocessor initialized (snort starts)
 */
void file_agent_init(struct _SnortConfig *sc, void *);

/* Initialize file processing thread.
 * This should be called after initialization of daemon
 */
void file_agent_thread_init(struct _SnortConfig *sc, void *);

/* Close file processing.
 * This should be called when snort exit
 */
void file_agent_close(void);


#endif 
