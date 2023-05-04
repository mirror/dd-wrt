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

// @file    shmem_mgmt.c
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#include "shmem_lib.h"
#include "shmem_mgmt.h"

#include <sys/mman.h>

ShmemMgmtData* mgmt_ptr    = NULL;
void*          zeroseg_ptr = NULL;
unsigned int   usec        = SLEEP_TIME;

static const char* const MODULE_NAME = "SharedMemMgmt";

static void SetShmemMgmtVariables(int value, uint32_t instance_num)
{
    int i;
    void* temp_zerosegptr;

    if (shmusr_ptr->instance_num == instance_num)
        temp_zerosegptr = zeroseg_ptr;
    else
        temp_zerosegptr = mgmt_ptr->instance[instance_num].shmemZeroPtr;

    if (value == GO_INACTIVE)
    {    
        mgmt_ptr->instance[instance_num].goInactive            = 1;
        value = mgmt_ptr->instance[instance_num].active;
    }
    else
    {
       mgmt_ptr->instance[instance_num].goInactive             = 0;
    }

    mgmt_ptr->instance[instance_num].active                    = value;
    mgmt_ptr->instance[instance_num].version                   = 0;
    mgmt_ptr->instance[instance_num].activeSegment             = NO_DATASEG;
    mgmt_ptr->instance[instance_num].prevSegment               = NO_DATASEG;
    mgmt_ptr->instance[instance_num].updateTime                = time(NULL);

    mgmt_ptr->instance[instance_num].shmemCurrPtr              = temp_zerosegptr;
    mgmt_ptr->instance[instance_num].shmemZeroPtr              = temp_zerosegptr;
    for (i=0; i<MAX_SEGMENTS; i++)
        mgmt_ptr->instance[instance_num].shmemSegActiveFlag[i] = 0;
    for (i=0; i<MAX_SEGMENTS; i++)
        mgmt_ptr->instance[instance_num].shmemSegmentPtr[i]    = temp_zerosegptr;
}

static void UnsetGoInactive()
{
    int i;

    for (i = 0; i < mgmt_ptr->maxInstances; i++)
        mgmt_ptr->instance[i].goInactive = 0;
}

static int MapShmemMgmt(unsigned int max_instances)
{
    size_t nBytes = sizeof(ShmemMgmtData) + sizeof(shmemInstance) * max_instances;
    off_t shmSize;
    int mgmtExists, i;

    mgmtExists = ShmemExists(shmusr_ptr->mgmtSeg, &shmSize);
    if (mgmtExists)
    {
        if (shmSize == nBytes)
        {
            mgmt_ptr = (ShmemMgmtData *) ShmemMap(shmusr_ptr->mgmtSeg, nBytes, shmusr_ptr->instance_type);
            if (!mgmt_ptr)
                return SF_EINVAL;
            /* Simple sanity check: If the segment is the expected size, it *should* have the same instance count. */
            if (mgmt_ptr->maxInstances == max_instances)
            {
                _dpd.logMsg("Mapped shared management region of size %zu as a %s.\n", nBytes,
                        (shmusr_ptr->instance_type == READ) ? "reader" : "writer");
                return SF_SUCCESS;
            }
            else if (shmusr_ptr->instance_type == READ)
            {
                _dpd.errMsg("%s: Expected instance count does not match that of the management segment (%u vs %u).  "
                        "Please remove the segment and try again.\n",
                        MODULE_NAME, max_instances, mgmt_ptr->maxInstances);
                munmap(mgmt_ptr, nBytes);
                mgmt_ptr = NULL;
                return SF_EINVAL;
            }
            else /* Writer */
            {
                _dpd.errMsg("%s: Expected instance count does not match that of the management segment (%u vs %u).  "
                        "Unlinking and recreating it.\n",
                        MODULE_NAME, max_instances, mgmt_ptr->maxInstances);
                munmap(mgmt_ptr, nBytes);
                mgmt_ptr = NULL;
                ShmemUnlink(shmusr_ptr->mgmtSeg);
                mgmtExists = 0;
            }
        }
        else if (shmusr_ptr->instance_type == READ)
        {
            _dpd.errMsg("%s: Management segment shared memory size does not match the expected size (%u vs %u).  "
                    "Refusing to use it.\n",
                    MODULE_NAME, shmSize, nBytes);
            return SF_EINVAL;
        }
        else /* Writer */
        {
            _dpd.logMsg("%s: Management segment shared memory size does not match the expected size (%u vs %u).  "
                    "Unlinking and recreating it.\n",
                    MODULE_NAME, shmSize, nBytes);
            ShmemUnlink(shmusr_ptr->mgmtSeg);
            mgmtExists = 0;
        }
    }

    /* This can become false in the wrong size as a writer scenario. */
    if (!mgmtExists)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "No Shmem mgmt segment present\n"););
        /* Readers must give up if there is no existing management segment. */
        if (shmusr_ptr->instance_type == READ)
            return SF_EINVAL;

        /* On the other hand, writers create a new management segment and initialize its values. */
        mgmt_ptr = (ShmemMgmtData *) ShmemMap(shmusr_ptr->mgmtSeg, nBytes, shmusr_ptr->instance_type);
        if (!mgmt_ptr)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Failed to create shmem mgmt segment\n"););
            return SF_EINVAL;
        }

        mgmt_ptr->activeSegment = NO_DATASEG;
        mgmt_ptr->maxInstances = max_instances;
        for (i = 0; i < MAX_SEGMENTS; i++)
        {
            mgmt_ptr->segment[i].version =  0;
            mgmt_ptr->segment[i].active  =  0;
            mgmt_ptr->segment[i].size    =  0;
        }
        UnsetGoInactive();
    }

    return SF_SUCCESS;
}

static void DoHeartbeat()
{
    uint32_t instance_num = shmusr_ptr->instance_num;
    if (mgmt_ptr)
    {
        mgmt_ptr->instance[instance_num].updateTime = time(NULL);
    }
    return;
}

static void ForceShutdown()
{
    int currActiveSegment;
    _dpd.logMsg("    Reputation Preprocessor: Shared memory is disabled. \n");
    if (!mgmt_ptr)
        return;
    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemCurrPtr =
        mgmt_ptr->instance[shmusr_ptr->instance_num].shmemZeroPtr;

    if ((currActiveSegment =
        mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment) >= 0)
    {
        mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment = NO_DATASEG;
        mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[currActiveSegment] = 0;
    }
    return;
}

//client side calls for shared memory
int CheckForSharedMemSegment(unsigned int max_instances)
{
    void *shmem_ptr = NULL;
    int currActive  = NO_DATASEG, newSegment = NO_DATASEG;
    uint32_t size   = 0;

    if (!mgmt_ptr)
    {
        if (MapShmemMgmt(max_instances))
            return newSegment;

        SetShmemMgmtVariables(ACTIVE,shmusr_ptr->instance_num);
    }

    if (mgmt_ptr->instance[shmusr_ptr->instance_num].goInactive)
        goto exit;

    if ((currActive = mgmt_ptr->activeSegment) >= 0)
    {
        if ( mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment != currActive &&
             mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[currActive] != TBMAP )
        {
            //new segment available and not mapped already
            mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[currActive] = TBMAP;

            if ((size = mgmt_ptr->segment[currActive].size) != 0)
            {
                if ((shmem_ptr = ShmemMap(shmusr_ptr->dataSeg[currActive],size,READ)) != NULL)
                {
                    //Store Data segment pointer for instance
                    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[currActive] = shmem_ptr;
                    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                        "Shmem ptr for segment %d is %p\n",currActive,shmem_ptr););
                    newSegment = currActive;
                }
                else
                {
                    currActive = NO_DATASEG;
                }
            }
            else
            {
                mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[currActive] = 0;
            }
        }
    }
    else if (mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment >= 0)
    {
        ForceShutdown();
        goto exit;
    }

    DoHeartbeat();

exit:
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "new segment being returned is %d\n", newSegment););
    return newSegment;
}

int InitShmemReader (
    uint32_t instance_num, int dataset, int group_id,
    int numa_node, const char* path, void*** data_ptr,
    uint32_t instance_polltime, unsigned int max_instances)
{
    int segment_number = NO_ZEROSEG;
    if (InitShmemUser(instance_num,READ,dataset,group_id,numa_node,path,instance_polltime,max_instances))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Could not initialize config data \n"););
        return segment_number;
    }
    if (dmfunc_ptr->CreatePerProcessZeroSegment(data_ptr))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Could not initialize zero segment\n"););
        return segment_number;
    }

    zeroseg_ptr = **data_ptr;

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "Address of zero segment is %p\n",zeroseg_ptr););

    if ((segment_number = CheckForSharedMemSegment(max_instances)) >=0)
    {
        SwitchToActiveSegment(segment_number,data_ptr);
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Switched to segment %d\n",segment_number););
    }
    return segment_number;
}

static int FindFirstUnusedShmemSegment()
{
    int i;
    for (i=0; i<MAX_SEGMENTS; i++)
    {
        if (mgmt_ptr->segment[i].active != 1)
            return i;
    }
    return NO_DATASEG;
}

static int FindActiveSharedMemDataSegmentVersion()
{
    if (mgmt_ptr->activeSegment < 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Active segment does not exist\n"););
        return 0;
    }
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "Active segment is %d and current version is %u\n",
        mgmt_ptr->activeSegment,mgmt_ptr->segment[mgmt_ptr->activeSegment].version););

    return mgmt_ptr->segment[mgmt_ptr->activeSegment].version;
}

static int MapShmemDataSegmentForWriter(uint32_t size, uint32_t disk_version, int *mode)
{
    int      available_segment = NO_DATASEG;
    uint32_t active_version    = 0;
    void*    shmem_ptr         = NULL;
    *mode                      = WRITE;

    if ((active_version =  FindActiveSharedMemDataSegmentVersion()) == disk_version )
    {
        if ((available_segment = mgmt_ptr->activeSegment) >= 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Attaching to segment %d\n", available_segment););
            *mode = READ;
            size = mgmt_ptr->segment[available_segment].size;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "No active segment to attach to\n"););
            goto exit;
        }
    }

    if (*mode == WRITE)
    {
        if ((available_segment = FindFirstUnusedShmemSegment()) < 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "No more segments available, all are in use\n"););
            goto exit;
        }
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Shared memory segment %d will be initialized\n",available_segment););
    }

    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[available_segment] = TBMAP;

    if ((shmem_ptr = ShmemMap(shmusr_ptr->dataSeg[available_segment],size,*mode)) != NULL)
    {
        //store data segment pointer for instance
        mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[available_segment] = shmem_ptr;
    }
    else
    {
        mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[available_segment] = 0;
        available_segment = SHMEM_ERR;
    }

exit:
    return available_segment;
}

static void ShutdownSegment(int32_t segment_num)
{
    mgmt_ptr->segment[segment_num].active  = 0;
    mgmt_ptr->segment[segment_num].version = 0;

    if (mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[segment_num] !=
                    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemZeroPtr) {
        munmap(mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[segment_num],
                mgmt_ptr->segment[segment_num].size);
    }

    ShmemDestroy(shmusr_ptr->dataSeg[segment_num]);

    mgmt_ptr->segment[segment_num].size = 0;
    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[segment_num] =
       mgmt_ptr->instance[shmusr_ptr->instance_num].shmemZeroPtr;
}

// writer side
static int InitSharedMemDataSegmentForWriter(uint32_t size, uint32_t disk_version)
{
    int segment_num = NO_DATASEG, mode = -1;
    int rval;

    if ((segment_num = MapShmemDataSegmentForWriter(size,disk_version,&mode)) < 0)
        goto exit;

    if (mode == WRITE)
    {
        if ((rval = dmfunc_ptr->LoadShmemData((void *)(
            mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[segment_num]),
            filelist_ptr, filelist_count)) != SF_SUCCESS)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Loading file into shared memory failed\n"););
            ShutdownSegment(segment_num);
            segment_num = SHMEM_ERR;
            goto exit;
        }
        mgmt_ptr->segment[segment_num].size = size;

        if (mgmt_ptr->activeSegment != segment_num)
            mgmt_ptr->activeSegment = segment_num;

        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "Active segment is %d\n",mgmt_ptr->activeSegment););

        mgmt_ptr->segment[segment_num].active = 1;
        mgmt_ptr->segment[segment_num].version = disk_version;
        ManageUnusedSegments();
    }
exit:
    return segment_num;
}


int LoadSharedMemDataSegmentForWriter(int startup)
{
    int segment_num = NO_DATASEG;
    uint32_t size;
    uint32_t disk_version, shmem_version;

    if ( !mgmt_ptr )
        return NO_DATASEG;

    shmem_version = FindActiveSharedMemDataSegmentVersion();

    //if version file is not present(open source user), increment version and reload.
    if (GetLatestShmemDataSetVersionOnDisk(&disk_version) == SF_SUCCESS)
    {
        if (disk_version > 0)
        {
            if ((shmem_version == disk_version) && !startup)
                goto exit;
        }
        else
        {
           goto force_shutdown;
        }
    }
    else
    {
        disk_version = shmem_version + 1;
        if (disk_version == 0) disk_version++;
    }

    if ( GetSortedListOfShmemDataFiles( ) != SF_SUCCESS )
        return SHMEM_ERR; 

#ifdef DEBUG_MSGS
    PrintDataFiles();
#endif

    if ((size = dmfunc_ptr->GetSegmentSize(filelist_ptr, filelist_count)) != ZEROSEG)
    {
        segment_num = InitSharedMemDataSegmentForWriter(size,disk_version);
        goto exit;
    }

force_shutdown:
    //got back zero which means its time to shutdown shared memory
    mgmt_ptr->activeSegment = NO_DATASEG;
    ForceShutdown();

exit:
   return segment_num;
}

int InitShmemWriter(
    uint32_t instance_num, int dataset, int group_id,
    int numa_node, const char* path, void*** data_ptr,
    uint32_t instance_polltime, unsigned int max_instances)
{
    int segment_number = NO_ZEROSEG;

    if (InitShmemUser(instance_num,WRITE,dataset,group_id,numa_node,path,instance_polltime,max_instances))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Could not initialize shmem writer config\n"););
        goto exit;
    }

    if (dmfunc_ptr->CreatePerProcessZeroSegment(data_ptr))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Could not initialize zero segment\n"););
        goto cleanup_exit;
    }

    zeroseg_ptr = **data_ptr;

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Address of zero segment is %p\n",zeroseg_ptr););

    if (MapShmemMgmt(max_instances))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Could not initialize shared memory management segment\n"););
        FreeShmemDataFileList();
        goto cleanup_exit;
    }

    ManageUnusedSegments();
    SetShmemMgmtVariables(ACTIVE,shmusr_ptr->instance_num);

    //valid segments are 0 through N
    if ((segment_number = LoadSharedMemDataSegmentForWriter(STARTUP)) >= 0)
        SwitchToActiveSegment(segment_number,data_ptr); //pointer switch

    goto exit;

cleanup_exit:
    FreeShmemUser();

exit:
    return segment_number;
}

//switch to active DB
void SwitchToActiveSegment(int segment_num, void*** data_ptr)
{
    if ((segment_num < 0)|| (!mgmt_ptr))
        return;

    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemCurrPtr =
        mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[segment_num];

    *data_ptr = (void *)(&mgmt_ptr->instance[shmusr_ptr->instance_num].shmemCurrPtr);

    mgmt_ptr->instance[shmusr_ptr->instance_num].prevSegment =
        mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment;

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "Prev segment has been set to %d\n",
        mgmt_ptr->instance[shmusr_ptr->instance_num].prevSegment););

    mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment = segment_num;
    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[segment_num] = 1;
}

void UnmapInactiveSegments()
{
    int i, segment_num;

    if (!mgmt_ptr)
        return;

    for (i=0; i<MAX_SEGMENTS; i++)
    {
        if (i != mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment)
        {
            if (shmusr_ptr->instance_type != WRITE)
            {
                if ((segment_num = mgmt_ptr->instance[shmusr_ptr->instance_num].prevSegment) != NO_DATASEG)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                        "Unmapping segment %d which has address %p and size %u\n",
                        segment_num,mgmt_ptr->instance[shmusr_ptr->instance_num].
                            shmemSegmentPtr[segment_num],mgmt_ptr->segment[segment_num].size););

                    munmap(mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[segment_num],
                        mgmt_ptr->segment[segment_num].size);
                    ShmemUnlink(shmusr_ptr->dataSeg[segment_num]);
                    mgmt_ptr->instance[shmusr_ptr->instance_num].prevSegment = NO_DATASEG;
                    mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegmentPtr[i] =
                        mgmt_ptr->instance[shmusr_ptr->instance_num].shmemZeroPtr;
                }
            }
            mgmt_ptr->instance[shmusr_ptr->instance_num].shmemSegActiveFlag[i] = 0;
        }
    }
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "Active segment for instance %u is %d\n",
        shmusr_ptr->instance_num,mgmt_ptr->instance[shmusr_ptr->instance_num].activeSegment););
    return;
}

static void ExpireTimedoutInstances()
{
    int i;
    int64_t max_timeout;
    time_t current_time = time(NULL);

    /*timeout will be at least 60 seconds*/
    max_timeout =  UNUSED_TIMEOUT * (int64_t) shmusr_ptr->instance_polltime + 60;
    if (max_timeout > UINT32_MAX)
        max_timeout = UINT32_MAX;

    for(i = 0; i < mgmt_ptr->maxInstances; i++)
    {
        if (!mgmt_ptr->instance[i].active)
            continue;

        if ((int64_t)current_time >  mgmt_ptr->instance[i].updateTime + max_timeout)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                        "Instance %d has expired, last update %jd and current time is %jd\n",
                        i,(intmax_t)mgmt_ptr->instance[i].updateTime,(intmax_t)current_time););
            SetShmemMgmtVariables(GO_INACTIVE, i);
        }
    }
    return;
}

//WRITER only
void ManageUnusedSegments()
{
    int s, i, in_use;

    if (!mgmt_ptr)
        return;

    DoHeartbeat();    //writer heartbeat

    if (UNUSED_TIMEOUT != -1)
        ExpireTimedoutInstances();

    for (s = 0; s < MAX_SEGMENTS; s++)
    {
        if (!mgmt_ptr->segment[s].active || (mgmt_ptr->activeSegment == s))
            continue;

        in_use = 0;
        for (i = 0; i < mgmt_ptr->maxInstances; i++)
        {
            if (mgmt_ptr->instance[i].active &&
                    !mgmt_ptr->instance[i].goInactive &&
                    mgmt_ptr->instance[i].shmemSegActiveFlag[s])
            {
                DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                            "Instance %u is still using segment %d\n", i, s););
                in_use++;
            }
        }
        if (!in_use)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Shutting down segment %d\n", s););
            ShutdownSegment(s);
        }
    }
    if (UNUSED_TIMEOUT != -1)
        UnsetGoInactive();
}

int ShutdownSharedMemory()
{
    if (mgmt_ptr)
        SetShmemMgmtVariables(INACTIVE,shmusr_ptr->instance_num);

    FreeShmemUser();
    FreeShmemDataMgmtFunctions();
    FreeShmemDataFileList();

    return SF_SUCCESS;
}

void ShmemMgmtInfo(char *buf, int bufLen)
{
    uint32_t i;
    int writed;
    int len = bufLen -1;
    char *index = buf;

    if (!mgmt_ptr)
        return;

    for (i = 0; i < mgmt_ptr->maxInstances; i++)
    {
        shmemInstance *shmem_info = (shmemInstance *) &(mgmt_ptr->instance[i]);
        if (shmem_info->shmemCurrPtr)
        {
            writed = snprintf(index, len, 
                "instance:%u active:%d goInactive:%d updateTime:%jd\n",
                i,(int)shmem_info->active,
                (int)shmem_info->goInactive,
                (intmax_t)shmem_info->updateTime);

            if (writed >= len || writed < 0)
                return;

            index += writed;
            len -= writed;

            writed = snprintf(index, len, 
                "instance:%u activeSegment:%d prevSegment:%d currentPtr:%p zeroPtr:%p\n",
                i,(int)shmem_info->activeSegment,
                (int)shmem_info->prevSegment,
                (void *)shmem_info->shmemCurrPtr,
                (void *)shmem_info->shmemZeroPtr);

            if (writed >= len || writed < 0)
                return;

            index += writed;
            len -= writed;
        }
    }
    for (i=0; i<MAX_SEGMENTS; i++)
    {
        shmemSegment *shmem_seg = &(mgmt_ptr->segment[i]);
        writed = snprintf(index, len, 
            "segment:%u active:%d version:%u\n",
            i,(int)shmem_seg->active,(uint32_t)shmem_seg->version);

        if (writed >= len || writed < 0)
            return;

        index += writed;
        len -= writed;

    }

    writed = snprintf(index, len, 
        "active segment:%d\n\n",mgmt_ptr->activeSegment);
    /* returning either way */
}

void PrintShmemMgmtInfo()
{
    uint32_t i;

    if (!mgmt_ptr)
        return;

    for (i = 0; i < mgmt_ptr->maxInstances; i++)
    {
        shmemInstance *shmem_info = (shmemInstance *) &(mgmt_ptr->instance[i]);
        if (shmem_info->shmemCurrPtr)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "instance:%u active:%d goInactive:%d updateTime:%jd\n",
                i,(int)shmem_info->active,
                (int)shmem_info->goInactive,
                (intmax_t)shmem_info->updateTime););

            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
                "instance:%u activeSegment:%d prevSegment:%d currentPtr:%p zeroPtr:%p\n",
                i,(int)shmem_info->activeSegment,
                (int)shmem_info->prevSegment,
                (void *)shmem_info->shmemCurrPtr,
                (void *)shmem_info->shmemZeroPtr););
        }
    }
    for (i=0; i<MAX_SEGMENTS; i++)
    {
        DEBUG_WRAP(shmemSegment *shmem_seg = &(mgmt_ptr->segment[i]););
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "segment:%u active:%d version:%u\n",
            i,(int)shmem_seg->active,(uint32_t)shmem_seg->version););
    }

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
        "active segment:%d\n\n",mgmt_ptr->activeSegment););
}

