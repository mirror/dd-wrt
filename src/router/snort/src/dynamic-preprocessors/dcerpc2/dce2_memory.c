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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_memory.h"
#include "dce2_utils.h"
#include "dce2_config.h"
#include "dce2_event.h"
#include "memory_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_MEMCAP_OK        0
#define DCE2_MEMCAP_EXCEEDED  1

/********************************************************************
 * Global variables
 ********************************************************************/
DCE2_Memory dce2_memory;
DCE2_MemState dce2_mem_state = DCE2_MEM_STATE__OKAY;

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static void DCE2_RegMemSmb(uint32_t, DCE2_MemType);
static void DCE2_RegMemCo(uint32_t, DCE2_MemType);
static void DCE2_RegMemCl(uint32_t, DCE2_MemType);
static int DCE2_CheckMemcap(uint32_t, DCE2_MemType);

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_RegMem(uint32_t size, DCE2_MemType mtype)
{
    switch (mtype)
    {
        case DCE2_MEM_TYPE__CONFIG:
            dce2_memory.config += size;
            if (dce2_memory.config > dce2_memory.config_max)
                dce2_memory.config_max = dce2_memory.config;
            break;

        case DCE2_MEM_TYPE__ROPTION:
            dce2_memory.roptions += size;
            if (dce2_memory.roptions > dce2_memory.roptions_max)
                dce2_memory.roptions_max = dce2_memory.roptions;
            break;

        case DCE2_MEM_TYPE__RT:
            dce2_memory.rt += size;
            if (dce2_memory.rt > dce2_memory.rt_max)
                dce2_memory.rt_max = dce2_memory.rt;
            break;

        case DCE2_MEM_TYPE__INIT:
            dce2_memory.init += size;
            if (dce2_memory.init > dce2_memory.init_max)
                dce2_memory.init_max = dce2_memory.init;
            break;

        case DCE2_MEM_TYPE__SMB_SSN:
        case DCE2_MEM_TYPE__SMB_SEG:
        case DCE2_MEM_TYPE__SMB_UID:
        case DCE2_MEM_TYPE__SMB_TID:
        case DCE2_MEM_TYPE__SMB_FID:
        case DCE2_MEM_TYPE__SMB_FILE:
        case DCE2_MEM_TYPE__SMB_REQ:
            DCE2_RegMemSmb(size, mtype);
            break;

        case DCE2_MEM_TYPE__TCP_SSN:
            dce2_memory.tcp_ssn += size;
            if (dce2_memory.tcp_ssn > dce2_memory.tcp_ssn_max)
                dce2_memory.tcp_ssn_max = dce2_memory.tcp_ssn;

            dce2_memory.tcp_total += size;
            if (dce2_memory.tcp_total > dce2_memory.tcp_total_max)
                dce2_memory.tcp_total_max = dce2_memory.tcp_total;

            break;

        case DCE2_MEM_TYPE__CO_SEG:
        case DCE2_MEM_TYPE__CO_FRAG:
        case DCE2_MEM_TYPE__CO_CTX:
            DCE2_RegMemCo(size, mtype);
            break;

        case DCE2_MEM_TYPE__UDP_SSN:
            dce2_memory.udp_ssn += size;
            if (dce2_memory.udp_ssn > dce2_memory.udp_ssn_max)
                dce2_memory.udp_ssn_max = dce2_memory.udp_ssn;

            dce2_memory.udp_total += size;
            if (dce2_memory.udp_total > dce2_memory.udp_total_max)
                dce2_memory.udp_total_max = dce2_memory.udp_total;

            break;

        case DCE2_MEM_TYPE__HTTP_SSN:
            dce2_memory.http_ssn += size;
            if (dce2_memory.http_ssn > dce2_memory.http_ssn_max)
                dce2_memory.http_ssn_max = dce2_memory.http_ssn;

            dce2_memory.http_total += size;
            if (dce2_memory.http_total > dce2_memory.http_total_max)
                dce2_memory.http_total_max = dce2_memory.http_total;

            break;

        case DCE2_MEM_TYPE__CL_ACT:
        case DCE2_MEM_TYPE__CL_FRAG:
            DCE2_RegMemCl(size, mtype);
            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid memory type: %d",
                     __FILE__, __LINE__, mtype);
            break;
    }

    switch (mtype)
    {
        case DCE2_MEM_TYPE__CONFIG:
        case DCE2_MEM_TYPE__ROPTION:
        case DCE2_MEM_TYPE__RT:
        case DCE2_MEM_TYPE__INIT:
            break;
        default:
            dce2_memory.rtotal += size;
            if (dce2_memory.rtotal > dce2_memory.rtotal_max)
                dce2_memory.rtotal_max = dce2_memory.rtotal;
    }

    dce2_memory.total += size;
    if (dce2_memory.total > dce2_memory.total_max)
        dce2_memory.total_max = dce2_memory.total;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_RegMemSmb(uint32_t size, DCE2_MemType mtype)
{
    switch (mtype)
    {
        case DCE2_MEM_TYPE__SMB_SSN:
            dce2_memory.smb_ssn += size;
            if (dce2_memory.smb_ssn > dce2_memory.smb_ssn_max)
                dce2_memory.smb_ssn_max = dce2_memory.smb_ssn;
            break;
        case DCE2_MEM_TYPE__SMB_SEG:
            dce2_memory.smb_seg += size;
            if (dce2_memory.smb_seg > dce2_memory.smb_seg_max)
                dce2_memory.smb_seg_max = dce2_memory.smb_seg;
            break;
        case DCE2_MEM_TYPE__SMB_UID:
            dce2_memory.smb_uid += size;
            if (dce2_memory.smb_uid > dce2_memory.smb_uid_max)
                dce2_memory.smb_uid_max = dce2_memory.smb_uid;
            break;
        case DCE2_MEM_TYPE__SMB_TID:
            dce2_memory.smb_tid += size;
            if (dce2_memory.smb_tid > dce2_memory.smb_tid_max)
                dce2_memory.smb_tid_max = dce2_memory.smb_tid;
            break;
        case DCE2_MEM_TYPE__SMB_FID:
            dce2_memory.smb_fid += size;
            if (dce2_memory.smb_fid > dce2_memory.smb_fid_max)
                dce2_memory.smb_fid_max = dce2_memory.smb_fid;
            break;
        case DCE2_MEM_TYPE__SMB_FILE:
            dce2_memory.smb_file += size;
            if (dce2_memory.smb_file > dce2_memory.smb_file_max)
                dce2_memory.smb_file_max = dce2_memory.smb_file;
            break;
        case DCE2_MEM_TYPE__SMB_REQ:
            dce2_memory.smb_req += size;
            if (dce2_memory.smb_req > dce2_memory.smb_req_max)
                dce2_memory.smb_req_max = dce2_memory.smb_req;
            break;
        default:
            return;
    }

    dce2_memory.smb_total += size;
    if (dce2_memory.smb_total > dce2_memory.smb_total_max)
        dce2_memory.smb_total_max = dce2_memory.smb_total;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_RegMemCo(uint32_t size, DCE2_MemType mtype)
{
    switch (mtype)
    {
        case DCE2_MEM_TYPE__CO_SEG:
            dce2_memory.co_seg += size;
            if (dce2_memory.co_seg > dce2_memory.co_seg_max)
                dce2_memory.co_seg_max = dce2_memory.co_seg;
            break;
        case DCE2_MEM_TYPE__CO_FRAG:
            dce2_memory.co_frag += size;
            if (dce2_memory.co_frag > dce2_memory.co_frag_max)
                dce2_memory.co_frag_max = dce2_memory.co_frag;
            break;
        case DCE2_MEM_TYPE__CO_CTX:
            dce2_memory.co_ctx += size;
            if (dce2_memory.co_ctx > dce2_memory.co_ctx_max)
                dce2_memory.co_ctx_max = dce2_memory.co_ctx;
            break;
        default:
            return;
    }

    dce2_memory.co_total += size;
    if (dce2_memory.co_total > dce2_memory.co_total_max)
        dce2_memory.co_total_max = dce2_memory.co_total;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_RegMemCl(uint32_t size, DCE2_MemType mtype)
{
    switch (mtype)
    {
        case DCE2_MEM_TYPE__CL_ACT:
            dce2_memory.cl_act += size;
            if (dce2_memory.cl_act > dce2_memory.cl_act_max)
                dce2_memory.cl_act_max = dce2_memory.cl_act;
            break;
        case DCE2_MEM_TYPE__CL_FRAG:
            dce2_memory.cl_frag += size;
            if (dce2_memory.cl_frag > dce2_memory.cl_frag_max)
                dce2_memory.cl_frag_max = dce2_memory.cl_frag;
            break;
        default:
            return;
    }

    dce2_memory.cl_total += size;
    if (dce2_memory.cl_total > dce2_memory.cl_total_max)
        dce2_memory.cl_total_max = dce2_memory.cl_total;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_UnRegMem(uint32_t size, DCE2_MemType mtype)
{
    switch (mtype)
    {
        case DCE2_MEM_TYPE__CONFIG:
            dce2_memory.config -= size;
            break;
        case DCE2_MEM_TYPE__ROPTION:
            dce2_memory.roptions -= size;
            break;
        case DCE2_MEM_TYPE__RT:
            dce2_memory.rt -= size;
            break;
        case DCE2_MEM_TYPE__INIT:
            dce2_memory.init -= size;
            break;
        case DCE2_MEM_TYPE__SMB_SSN:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_ssn -= size;
            break;
        case DCE2_MEM_TYPE__SMB_SEG:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_seg -= size;
            break;
        case DCE2_MEM_TYPE__SMB_UID:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_uid -= size;
            break;
        case DCE2_MEM_TYPE__SMB_TID:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_tid -= size;
            break;
        case DCE2_MEM_TYPE__SMB_FID:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_fid -= size;
            break;
        case DCE2_MEM_TYPE__SMB_FILE:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_file -= size;
            break;
        case DCE2_MEM_TYPE__SMB_REQ:
            dce2_memory.smb_total -= size;
            dce2_memory.smb_req -= size;
            break;
        case DCE2_MEM_TYPE__TCP_SSN:
            dce2_memory.tcp_total -= size;
            dce2_memory.tcp_ssn -= size;
            break;
        case DCE2_MEM_TYPE__CO_SEG:
            dce2_memory.co_total -= size;
            dce2_memory.co_seg -= size;
            break;
        case DCE2_MEM_TYPE__CO_FRAG:
            dce2_memory.co_total -= size;
            dce2_memory.co_frag -= size;
            break;
        case DCE2_MEM_TYPE__CO_CTX:
            dce2_memory.co_total -= size;
            dce2_memory.co_ctx -= size;
            break;
        case DCE2_MEM_TYPE__UDP_SSN:
            dce2_memory.udp_total -= size;
            dce2_memory.udp_ssn -= size;
            break;
        case DCE2_MEM_TYPE__CL_ACT:
            dce2_memory.cl_total -= size;
            dce2_memory.cl_act -= size;
            break;
        case DCE2_MEM_TYPE__HTTP_SSN:
            dce2_memory.http_total -= size;
            dce2_memory.http_ssn -= size;
            break;
        case DCE2_MEM_TYPE__CL_FRAG:
            dce2_memory.cl_total -= size;
            dce2_memory.cl_frag -= size;
            break;
        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid memory type: %d",
                     __FILE__, __LINE__, mtype);
            break;
    }

    switch (mtype)
    {
        case DCE2_MEM_TYPE__CONFIG:
        case DCE2_MEM_TYPE__ROPTION:
        case DCE2_MEM_TYPE__RT:
        case DCE2_MEM_TYPE__INIT:
            break;
        default:
            dce2_memory.rtotal -= size;
    }

    dce2_memory.total -= size;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_CheckMemcap(uint32_t size, DCE2_MemType mtype)
{
    switch (mtype)
    {
        /*Avoid checking memcap for configurations*/
        case DCE2_MEM_TYPE__CONFIG:
        case DCE2_MEM_TYPE__ROPTION:
        case DCE2_MEM_TYPE__RT:
        case DCE2_MEM_TYPE__INIT:
            break;
        default:
            if (dce2_mem_state == DCE2_MEM_STATE__MEMCAP)
                break;
            if ((dce2_memory.rtotal + size) > DCE2_GcMemcap())
            {
                DCE2_Alert(NULL, DCE2_EVENT__MEMCAP);
                dce2_mem_state = DCE2_MEM_STATE__MEMCAP;

                return DCE2_MEMCAP_EXCEEDED;
            }

            break;
    }

    return DCE2_MEMCAP_OK;
}

uint32_t check_memory_category (DCE2_MemType mtype)
{
    switch (mtype)
    {
        case DCE2_MEM_TYPE__CONFIG:
        case DCE2_MEM_TYPE__ROPTION:
            return PP_MEM_CATEGORY_CONFIG;
        case DCE2_MEM_TYPE__RT:
            return PP_MEM_CATEGORY_MISC;
        case DCE2_MEM_TYPE__INIT:
            return PP_MEM_CATEGORY_CONFIG;
        case DCE2_MEM_TYPE__SMB_SSN:
        case DCE2_MEM_TYPE__SMB_SEG:
        case DCE2_MEM_TYPE__SMB_UID:
        case DCE2_MEM_TYPE__SMB_TID:
        case DCE2_MEM_TYPE__SMB_FID:
        case DCE2_MEM_TYPE__SMB_FILE:
        case DCE2_MEM_TYPE__SMB_REQ:
        case DCE2_MEM_TYPE__TCP_SSN:
        case DCE2_MEM_TYPE__CO_SEG:
        case DCE2_MEM_TYPE__CO_FRAG:
        case DCE2_MEM_TYPE__CO_CTX:
        case DCE2_MEM_TYPE__UDP_SSN:
        case DCE2_MEM_TYPE__CL_ACT:
        case DCE2_MEM_TYPE__CL_FRAG:
        case DCE2_MEM_TYPE__HTTP_SSN:
            return PP_MEM_CATEGORY_SESSION;
        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Invalid memory type: %d",
                     __FILE__, __LINE__, mtype);
            return PP_MEM_MAX_CATEGORY;
     }
    
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void * DCE2_Alloc(uint32_t size, DCE2_MemType mtype)
{
    void *mem;

    if (DCE2_CheckMemcap(size, mtype) != DCE2_MEMCAP_OK)
        return NULL;

    mem = _dpd.snortAlloc(1, (size_t)size, PP_DCE2, check_memory_category(mtype));
    if (mem == NULL)
    {
        DCE2_Die("%s(%d) Out of memory!", __FILE__, __LINE__);
    }

    DCE2_RegMem(size, mtype);

    return mem;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_Free(void *mem, uint32_t size, DCE2_MemType mtype)
{
    if (mem == NULL)
        return;

    DCE2_UnRegMem(size, mtype);

    _dpd.snortFree(mem, (size_t)size, PP_DCE2, check_memory_category(mtype));
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void * DCE2_ReAlloc(void *old_mem, uint32_t old_size, uint32_t new_size, DCE2_MemType mtype)
{
    void *new_mem;
    DCE2_Ret status;

    if (old_mem == NULL)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Old memory passed in was NULL.",
                 __FILE__, __LINE__);
        return NULL;
    }
    else if (new_size < old_size)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) New size is less than old size.",
                 __FILE__, __LINE__);
        return NULL;
    }
    else if (new_size == old_size)
    {
        return old_mem;
    }

    if (DCE2_CheckMemcap(new_size - old_size, mtype) == DCE2_MEMCAP_EXCEEDED)
        return NULL;

    new_mem = DCE2_Alloc(new_size, mtype);
    if (new_mem == NULL)
        return NULL;

    status = DCE2_Memcpy(new_mem, old_mem, old_size,
                         new_mem, (void *)((uint8_t *)new_mem + new_size));

    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to copy old memory into new memory.",
                 __FILE__, __LINE__);
        DCE2_Free(new_mem, new_size, mtype);
        return NULL;
    }

    DCE2_Free(old_mem, old_size, mtype);

    return new_mem;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_MemInit(void)
{
    memset(&dce2_memory, 0, sizeof(dce2_memory));
}

size_t DCE2_MemInUse()
{
    return (size_t) dce2_memory.rtotal;
}

