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

// @file    shmem_config.c
// @author  Pramod Chandrashekar <pramod@sourcefire.com>

#include <string.h>
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
#include "snort_debug.h"

#include "sflinux_helpers.h"
#include "shmem_config.h"

static const char* const MODULE_NAME ="SharedMemConfig";

ShmemUserInfo *shmusr_ptr = NULL;
ShmemDataMgmtFunctions *dmfunc_ptr = NULL;

static DatasetInfo dataset_names[] =
{
    { "SFIPReputation.rt", IPREP }
};

static void ConstructSegmentNames (int dataset, int group_id, int numa_node)
{
    int i;

    snprintf(shmusr_ptr->mgmtSeg, sizeof(shmusr_ptr->mgmtSeg),
        "%s.%d.%d",SHMEM_MGMT,group_id,numa_node);

    for (i=0; i<MAX_SEGMENTS; i++)
        snprintf(shmusr_ptr->dataSeg[i], sizeof(shmusr_ptr->dataSeg[0]),
            "%s.%d.%d.%d",dataset_names[dataset].name,group_id,numa_node,i);
}

int InitShmemUser (uint32_t instance_num, int instance_type, int dataset, int group_id,
        int numa_node, const char* path, uint32_t instance_polltime, uint16_t max_instances)
{
    int rval = SF_EINVAL, num_nodes;

    if ((instance_num >= max_instances) ||
        (instance_type != READ && instance_type != WRITE) ||
        (dataset != IPREP) || !path || !instance_polltime )
        goto exit;

    if ((shmusr_ptr = calloc(1, sizeof(*shmusr_ptr))) == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Unable to allocate memory for configuration data"););
        goto exit;
    }

    shmusr_ptr->instance_num      = instance_num;
    shmusr_ptr->instance_type     = instance_type;
    shmusr_ptr->dataset           = dataset;
    shmusr_ptr->group_id          = group_id;
    shmusr_ptr->instance_polltime = instance_polltime;

    num_nodes = CheckNumaNodes();
    if (numa_node > num_nodes)
        numa_node = NUMA_0;

    shmusr_ptr->numa_node = numa_node;
    strncpy(shmusr_ptr->path,path,sizeof(shmusr_ptr->path));
    shmusr_ptr->path[sizeof(shmusr_ptr->path)-1] = '\0';
    ConstructSegmentNames(dataset,group_id,numa_node);

    return SF_SUCCESS;

exit: 
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Error in setting config"););
    return rval;
}

int InitShmemDataMgmtFunctions (
    CreateMallocZero create_malloc_zero,
    GetDataSize get_data_size,
    LoadData load_data)
{
    if ((dmfunc_ptr = (ShmemDataMgmtFunctions*)
                      malloc(sizeof(ShmemDataMgmtFunctions))) == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION,
            "Could not allocate memory for Shmem Datamanagement function list"););
        return SF_EINVAL;
    }
    dmfunc_ptr->CreatePerProcessZeroSegment = create_malloc_zero;   
    dmfunc_ptr->GetSegmentSize = get_data_size;
    dmfunc_ptr->LoadShmemData  = load_data;

    return SF_SUCCESS;
}

void FreeShmemUser()
{
    if (shmusr_ptr)
        free(shmusr_ptr);
    shmusr_ptr = NULL;
}

void FreeShmemDataMgmtFunctions()
{
    if (dmfunc_ptr)
        free(dmfunc_ptr);
    dmfunc_ptr = NULL;
}

void PrintConfig()
{
    int i;

    _dpd.logMsg("Instance number    %u:",shmusr_ptr->instance_num);
    _dpd.logMsg("Instance type      %d:",shmusr_ptr->instance_type);
    _dpd.logMsg("Instance datatype  %d:",shmusr_ptr->dataset);
    _dpd.logMsg("Instance Group ID  %d:",shmusr_ptr->group_id);
    _dpd.logMsg("Instance Numa node %d:",shmusr_ptr->numa_node);
    _dpd.logMsg("Instance Poll time %d:",shmusr_ptr->instance_polltime);
    _dpd.logMsg("Data Path is       %s:",shmusr_ptr->path);

    for (i=0; i<MAX_SEGMENTS; i++)
        _dpd.logMsg("Available data segments are %s",shmusr_ptr->dataSeg[i]);
}

