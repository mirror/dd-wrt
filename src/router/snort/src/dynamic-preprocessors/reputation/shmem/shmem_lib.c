/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 ****************************************************************************/

// @file    shmem_lib.c
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "shmem_mgmt.h"
#include "shmem_lib.h"

static const char* const MODULE_NAME = "ShmemLib";

static int ShmemOpen(const char *shmemName, uint32_t size, int mode)
{
    int fd, flags;
    mode_t prev_mask;

    if (mode == WRITE)
        flags = (O_CREAT | O_RDWR);
    else if (mode == READ)
        flags = O_RDWR;
    else
    {    
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Invalid mode specified\n"););
        return -1;
    }

    prev_mask = umask(0);

    if ( (fd = shm_open(shmemName, flags,
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) )) == -1 )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Unable to open shared memory\n"););
        umask(prev_mask);
        return -1; 
    }

    umask(prev_mask);

    if (ftruncate(fd, size) == -1)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Unable to open shared memory\n"););
        return -1;
    }
    _dpd.logMsg("    Reputation Preprocessor: Size of shared memory segment %s is %u\n", shmemName, size);

    return fd;
}

static void *ShmemMMap (int fd, uint32_t size)
{
    void *shmem_ptr;

    if ((shmem_ptr =  mmap(0, size,(PROT_READ | PROT_WRITE),MAP_SHARED,fd,0))
        == MAP_FAILED )
        return NULL;

    return shmem_ptr;
}

int ShmemExists(const char *shmemName, off_t *size)
{
    struct stat sb;
    int fd;

    if ((fd = shm_open(shmemName,(O_RDWR),(S_IRUSR))) < 0 )
        return 0;

    if (size)
    {
        if (fstat(fd, &sb))
            return 0;
        *size = sb.st_size;
    }

    close(fd);
    return SF_EEXIST;
}

void ShmemUnlink(const char *shmemName)
{
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "Unlinking segment %\n",shmemName););
    shm_unlink(shmemName);
}

void ShmemDestroy(const char *shmemName)
{
    if (!ShmemExists(shmemName, NULL))
        return;
    ShmemUnlink(shmemName);
    unlink(shmemName);
    _dpd.logMsg("    Reputation Preprocessor: %s is freed\n", shmemName);
}

void* ShmemMap(const char* segment_name, uint32_t size, int mode)
{
    int fd = 0;
    void *shmem_ptr = NULL;

    if ((mode == WRITE) && ShmemExists(segment_name, NULL))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Cannot create shared memory segment %s, already exists\n",
            segment_name););
        mode = READ;
    }
    if ((fd = ShmemOpen(segment_name,size,mode)) == -1)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Failed to open shm %s\n",segment_name););
        return NULL;
    }

    if ((shmem_ptr = ShmemMMap(fd,size)) == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Failed to mmmap %s\n",segment_name););
    } 
    close(fd);

    return shmem_ptr;
}

