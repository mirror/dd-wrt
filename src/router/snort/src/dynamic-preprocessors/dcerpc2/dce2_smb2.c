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
 * SMB2 file processing
 * Author(s):  Hui Cao <huica@cisco.com>
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dce2_smb2.h"
#include "file_api.h"
#include "dce2_session.h"
#include "sf_snort_packet.h"
#include "dce2_stats.h"
#include "dce2_config.h"
#include "snort_dce2.h"
#include "dce2_event.h"
#include "dce2_list.h"

#ifdef DUMP_BUFFER
#include "dcerpc2_buffer_dump.h"
#endif


#define   UNKNOWN_FILE_SIZE                  ~0

static void *fileCache = NULL;

static void DCE2_Smb2Inspect(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        const uint8_t *end);

/********************************************************************
 * Function: DCE2_Smb2GetFileName()
 *
 * Purpose:
 *  Parses data passed in and returns a BOM prepended byte stream.
 *
 * Arguments:
 *  const uint8_t *  - pointer to data
 *  uint32_t         - data length
 *  uint16_t *       - Returns the length of the output buffer including the NULL terminated bytes
 *
 * Returns:
 *  uint8_t *        - NULL terminated byte stream (UTF-16LE with BOM)
 *
 ********************************************************************/
static uint8_t * DCE2_Smb2GetFileName(const uint8_t *data,
        uint32_t data_len, uint16_t *file_name_len)
{
    uint8_t *fname = NULL;
    int i;

    if (data_len < 2)
        return NULL;
    if(data_len > 2*DCE2_SMB_MAX_PATH_LEN)
        return NULL;

    for (i = 0; i < data_len; i += 2)
    {
        uint16_t uchar =  SmbNtohs((uint16_t *)(data + i));
        if (uchar == 0)
            break;
    }
    fname = (uint8_t *)DCE2_Alloc(i + UTF_16_LE_BOM_LEN + 2, DCE2_MEM_TYPE__SMB_SSN);
    if (fname == NULL)
        return NULL;

    memcpy(fname, UTF_16_LE_BOM, UTF_16_LE_BOM_LEN);//Prepend with BOM
    memcpy(fname + UTF_16_LE_BOM_LEN, data, i);
    *file_name_len = i + UTF_16_LE_BOM_LEN + 2;

    return fname;
}

static inline uint32_t Smb2Tid(const Smb2Hdr *hdr)
{
    return SmbNtohl(&(((Smb2SyncHdr *)hdr)->tree_id));
}

static int DCE2_Smb2TidCompare(const void *a, const void *b)
{
    uint32_t x = (uint32_t)(uintptr_t)a;
    uint32_t y = (uint32_t)(uintptr_t)b;

    if (x == y)
        return 0;

    /* Only care about equality for finding */
    return -1;
}

static inline void DCE2_Smb2InsertTid(DCE2_SmbSsnData *ssd, const uint32_t tid,
        const uint8_t share_type)
{
    bool is_ipc = (share_type != SMB2_SHARE_TYPE_DISK);

    if (!is_ipc && (!DCE2_ScSmbFileInspection(ssd->sd.sconfig)
            || ((ssd->max_file_depth == -1) && DCE2_ScSmbFileDepth(ssd->sd.sconfig) == -1)))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Not inserting TID (%u) "
                "because it's not IPC and not inspecting normal file "
                "data.", tid));
        return;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Inserting Tid: %u\n", tid));

    if (ssd->tids == NULL)
    {
        ssd->tids = DCE2_ListNew(DCE2_LIST_TYPE__SPLAYED, DCE2_Smb2TidCompare,
                NULL, NULL, DCE2_LIST_FLAG__NO_DUPS, DCE2_MEM_TYPE__SMB_TID);

        if (ssd->tids == NULL)
        {
            return;
        }
    }

    DCE2_ListInsert(ssd->tids, (void *)(uintptr_t)tid, (void *)(uintptr_t)share_type);
}

static inline uint8_t DCE2_Smb2ShareType(DCE2_SmbSsnData *ssd, const uint32_t tid)
{
    uint8_t share_type = (uint8_t)(uintptr_t)DCE2_ListFind(ssd->tids,
            (void *)(uintptr_t)tid);
    return share_type;
}

static DCE2_Ret DCE2_Smb2FindTid(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr)
{
    /* Still process async commands*/
    if (SmbNtohl(&(smb_hdr->flags)) & SMB2_FLAGS_ASYNC_COMMAND)
        return DCE2_RET__SUCCESS;

    return DCE2_ListFindKey(ssd->tids, (void *)(uintptr_t)Smb2Tid(smb_hdr));
}

static inline void DCE2_Smb2RemoveTid(DCE2_SmbSsnData *ssd, const uint32_t tid)
{
    DCE2_ListRemove(ssd->tids, (void *)(uintptr_t)tid);
}

static inline Smb2Request *DCE2_Smb2StoreRequest(DCE2_SmbSsnData *ssd,
        uint64_t message_id, uint16_t command)
{
    Smb2Request *request = ssd->smb2_requests;
    ssd->max_outstanding_requests = 128; /* windows client max */

    while (request)
    {
        if (request->message_id == message_id)
            return request;
        request = request->next;
    }

    request = (Smb2Request *) DCE2_Alloc(sizeof(*request), DCE2_MEM_TYPE__SMB_SSN);

    if (!request)
        return NULL;

    ssd->outstanding_requests++;

    if (ssd->outstanding_requests >= ssd->max_outstanding_requests)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_MAX_REQS_EXCEEDED,
                ssd->max_outstanding_requests);
        DCE2_Free(request, sizeof(*request), DCE2_MEM_TYPE__SMB_SSN );
        ssd->outstanding_requests--;
        return NULL;
    }

    request->message_id = message_id;
    request->command = command;

    request->next = ssd->smb2_requests;
    request->previous = NULL;
    if (ssd->smb2_requests)
        ssd->smb2_requests->previous = request;
    ssd->smb2_requests = request;
    return request;
}

static inline void DCE2_Smb2StoreReadRequest(DCE2_SmbSsnData *ssd,
        uint64_t message_id, uint16_t command, uint64_t offset, uint64_t file_id)
{
    Smb2Request *request = NULL;
    if((request = DCE2_Smb2StoreRequest(ssd, message_id, command)) != NULL)
    {
        request->read_req.offset = offset;
        request->read_req.file_id = file_id;
    }
}

static inline void DCE2_Smb2StoreCreateRequest(DCE2_SmbSsnData *ssd,
        uint64_t message_id, uint16_t command, char *file_name, uint16_t file_name_len, bool durable_reconnect)
{
    Smb2Request *request = NULL;
    if((request = DCE2_Smb2StoreRequest(ssd, message_id, command)) != NULL)
    {
        request->create_req.file_name = file_name;
        request->create_req.file_name_len = file_name_len;
        request->create_req.durable_reconnect =  durable_reconnect;
    }
}

static inline Smb2Request* DCE2_Smb2GetRequest(DCE2_SmbSsnData *ssd,
        uint64_t message_id)
{
    Smb2Request *request = ssd->smb2_requests;
    while (request)
    {
        if (request->message_id == message_id)
            return request;
        request = request->next;
    }

    return NULL;
}

static inline void DCE2_Smb2RemoveRequest(DCE2_SmbSsnData *ssd,
        Smb2Request* request)
{
    if (request->previous)
    {
        request->previous->next = request->next;
    }

    if (request->next)
    {
        request->next->previous = request->previous;
    }

    if (request == ssd->smb2_requests)
    {
        ssd->smb2_requests =  request->next;
    }

    ssd->outstanding_requests--;
    DCE2_Free(request, sizeof (*request), DCE2_MEM_TYPE__SMB_SSN);

    return;
}

static inline void DCE2_Smb2FreeFileName(DCE2_SmbFileTracker *ftracker)
{
    if (ftracker->file_name)
    {
        DCE2_Free(ftracker->file_name, ftracker->file_name_len, DCE2_MEM_TYPE__SMB_SSN);
        ftracker->file_name = NULL;
    }
    ftracker->file_name_len = 0;
}

static inline void DCE2_Smb2ResetFileName(DCE2_SmbFileTracker *ftracker)
{
    DCE2_UnRegMem(ftracker->file_name_len, DCE2_MEM_TYPE__SMB_SSN);
    ftracker->file_name = NULL;
    ftracker->file_name_len = 0;
}

static inline void DCE2_Smb2ProcessFileData(DCE2_SmbSsnData *ssd, const uint8_t *file_data,
        uint32_t data_size, bool upload)
{
    int64_t file_detection_depth = DCE2_ScSmbFileDepth(ssd->sd.sconfig);
    int64_t detection_size = 0;

    if (file_detection_depth == 0)
        detection_size = data_size;
    else if ( ssd->ftracker.tracker.file.file_offset < (uint64_t)file_detection_depth)
    {
        if ( file_detection_depth - ssd->ftracker.tracker.file.file_offset < data_size )
            detection_size = file_detection_depth - ssd->ftracker.tracker.file.file_offset;
        else
            detection_size = data_size;
    }

    if (detection_size)
    {
        _dpd.setFileDataPtr((uint8_t *)file_data,
                (detection_size > UINT16_MAX) ? UINT16_MAX : (uint16_t)detection_size);

        DCE2_FileDetect(&ssd->sd);
    }

    /*Do not inspect if offset exceeds max_file_depth. If max_file_depth is negative then do not call file_segment_process*/
    if ((ssd->max_file_depth >= 0) &&
            ((ssd->max_file_depth == 0) || (ssd->ftracker.tracker.file.file_offset <  ssd->max_file_depth)))
    {
        _dpd.fileAPI->file_segment_process(fileCache, (void *)ssd->sd.wire_pkt,
            ssd->ftracker.fid_v2, ssd->ftracker.tracker.file.file_size,
            file_data, data_size, ssd->ftracker.tracker.file.file_offset,
            upload);
    }
}

/********************************************************************
 *
 * Process tree connect command
 * Share type is defined here
 *
 ********************************************************************/
static void DCE2_Smb2TreeConnect(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    /* Using structure size to decide whether it is response or request*/
    uint16_t structure_size;
    Smb2TreeConnectResponseHdr *smb_tree_connect_hdr = (Smb2TreeConnectResponseHdr *)smb_data;

    if ((const uint8_t *)smb_tree_connect_hdr + SMB2_TREE_CONNECT_RESPONSE_STRUC_SIZE > end)
        return;

    structure_size = SmbNtohs(&(smb_tree_connect_hdr->structure_size));

    if (structure_size == SMB2_TREE_CONNECT_RESPONSE_STRUC_SIZE)
    {
        DCE2_Smb2InsertTid(ssd, Smb2Tid(smb_hdr), smb_tree_connect_hdr->share_type);
    }
    return;
}

/********************************************************************
 *
 * Process tree connect command
 * Share type is defined here
 *
 ********************************************************************/
static void DCE2_Smb2TreeDisconnect(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    /* Using structure size to decide whether it is response or request*/
    uint16_t structure_size;
    Smb2TreeDisConnectHdr *smb_tree_disconnect_hdr = (Smb2TreeDisConnectHdr *)smb_data;

    if ((const uint8_t *)smb_tree_disconnect_hdr + SMB2_TREE_DISCONNECT_STRUC_SIZE > end)
        return;

    structure_size = SmbNtohs(&(smb_tree_disconnect_hdr->structure_size));

    if (structure_size == SMB2_TREE_DISCONNECT_STRUC_SIZE)
    {
        DCE2_Smb2RemoveTid(ssd, Smb2Tid(smb_hdr));
    }
    return;
}

bool IsSmb2DurableReconnect(Smb2ACreateRequestHdr *smb_create_hdr, const uint8_t *end)
{
    const uint8_t *data = (uint8_t *) smb_create_hdr + SmbNtohl(&smb_create_hdr->create_contexts_offset) - SMB2_HEADER_LENGTH;
    uint32_t remaining = SmbNtohl(&smb_create_hdr->create_contexts_length);

    while (remaining > sizeof(Smb2CreateContextHdr) && data < end) {
        Smb2CreateContextHdr *context = (Smb2CreateContextHdr *)data;
        uint32_t next = SmbNtohl(&context->next);
        uint16_t name_offset = SmbNtohs(&context->name_offset) ;
        uint16_t name_length = SmbNtohs(&context->name_length) ;
        uint16_t data_offset = SmbNtohs(&context->data_offset);
        uint32_t data_length =  SmbNtohl(&context->data_length);

        /* Check for general error condition */
        if ((next & 0x7) != 0 ||
            next > remaining ||
            name_offset != 16 ||
            name_length < 4 ||
            name_offset + name_length > remaining ||
            (data_offset & 0x7) != 0 ||
            (data_offset && (data_offset < name_offset + name_length)) ||
            (data_offset > remaining) ||
            (data_offset + data_length > remaining)) {
            return false;
        }

        if((strncmp((char *)context+name_offset, SMB2_CREATE_DURABLE_RECONNECT_V2, name_length) == 0) ||
        (strncmp((char *)context+name_offset, SMB2_CREATE_DURABLE_RECONNECT, name_length) == 0))
        {
            return true;
        }

        if(!next)
            break;

        data += next;
        remaining -= next;
    }
    return false;
}

/********************************************************************
 *
 * Process create request, first command for a file processing
 * Update file name
 *
 ********************************************************************/
static void DCE2_Smb2CreateRequest(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        Smb2ACreateRequestHdr *smb_create_hdr,const uint8_t *end)
{
    uint16_t name_offset = SmbNtohs(&(smb_create_hdr->name_offset));
    uint8_t *fname = NULL;
    uint16_t fname_len = 0;
    uint64_t message_id;
    bool durable_reconnect = false;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing create request command!\n"));
    DCE2_Smb2InitFileTracker(&ssd->ftracker, false, 0);
    message_id = SmbNtohq((const uint64_t *)(&(smb_hdr->message_id)));

    if (name_offset > SMB2_HEADER_LENGTH)
    {
        uint16_t size;
        uint8_t *file_data =  (uint8_t *) smb_create_hdr + smb_create_hdr->name_offset - SMB2_HEADER_LENGTH ;
        if (file_data >= end)
            return;
        size = SmbNtohs(&(smb_create_hdr->name_length));
        if (!size || (file_data + size > end))
            return;
        fname = DCE2_Smb2GetFileName(file_data, size, &fname_len);
        durable_reconnect = IsSmb2DurableReconnect(smb_create_hdr, end);
    }
    DCE2_Smb2StoreCreateRequest(ssd, message_id, SMB2_COM_CREATE, (char *)fname, fname_len, durable_reconnect);
}

/********************************************************************
 *
 * Process create response, need to update file id
 * For downloading, file size is decided here
 *
 ********************************************************************/
static void DCE2_Smb2CreateResponse(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        Smb2ACreateResponseHdr *smb_create_hdr, const uint8_t *end)
{
    uint64_t fileId_persistent;
    uint64_t file_size = UNKNOWN_FILE_SIZE;
    uint64_t message_id;
    Smb2Request *request = NULL;
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing create response command!\n"));
    message_id = SmbNtohq((const uint64_t *)(&(smb_hdr->message_id)));
    request = DCE2_Smb2GetRequest(ssd, message_id);
    if (!request || request->command != SMB2_COM_CREATE)
    {
        return;
    }
    fileId_persistent = SmbNtohq((const uint64_t *)(&(smb_create_hdr->fileId_persistent)));
    ssd->ftracker.fid_v2 = fileId_persistent;
    if (smb_create_hdr->end_of_file)
    {
        file_size = SmbNtohq((const uint64_t *)(&(smb_create_hdr->end_of_file)));
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Get file size %u!\n", file_size ));
        ssd->ftracker.tracker.file.file_size = file_size;
    }

    if (request->create_req.file_name && request->create_req.file_name_len)
    {
        _dpd.fileAPI->file_cache_update_entry(fileCache, (void *)ssd->sd.wire_pkt, ssd->ftracker.fid_v2,
                (uint8_t *) request->create_req.file_name, request->create_req.file_name_len, file_size, !request->create_req.durable_reconnect, false);
         DCE2_UnRegMem(request->create_req.file_name_len, DCE2_MEM_TYPE__SMB_SSN);
    }

    DCE2_Smb2RemoveRequest(ssd, request);
}

/********************************************************************
 *
 * Process create command
 *
 ********************************************************************/
static void DCE2_Smb2Create(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    uint16_t structure_size;
    Smb2ACreateRequestHdr *smb_create_hdr = (Smb2ACreateRequestHdr *)smb_data;

    structure_size = SmbNtohs(&(smb_create_hdr->structure_size));

    /* Using structure size to decide whether it is response or request */
    if (structure_size == SMB2_CREATE_REQUEST_STRUC_SIZE)
    {
        if ((const uint8_t *)smb_create_hdr + SMB2_CREATE_REQUEST_STRUC_SIZE - 1 > end)
            return;
        DCE2_Smb2CreateRequest(ssd, smb_hdr, (Smb2ACreateRequestHdr *)smb_create_hdr, end);
    }
    else if (structure_size == SMB2_CREATE_RESPONSE_STRUC_SIZE)
    {
        if ((const uint8_t *)smb_create_hdr + SMB2_CREATE_RESPONSE_STRUC_SIZE -1 > end)
            return;
        DCE2_Smb2CreateResponse(ssd, smb_hdr, (Smb2ACreateResponseHdr *)smb_create_hdr, end);
    }
    else if (structure_size == SMB2_ERROR_RESPONSE_STRUC_SIZE)
    {
        uint64_t message_id;
        Smb2Request *request;

        if ((const uint8_t *)smb_create_hdr + SMB2_ERROR_RESPONSE_STRUC_SIZE - 1 > end)
            return;

        if((SmbNtohl(&(smb_hdr->flags)) & SMB2_FLAGS_ASYNC_COMMAND) &&
                (SmbNtohl(&(smb_hdr->status)) == SMB2_STATUS_PENDING ))
        return;

        /*Response error, clean up request state*/
        message_id = SmbNtohq((const uint64_t *)(&(smb_hdr->message_id)));
        request = DCE2_Smb2GetRequest(ssd, message_id);
        if (request && request->create_req.file_name)
        {
            DCE2_Free(request->create_req.file_name, request->create_req.file_name_len, DCE2_MEM_TYPE__SMB_SSN);
        }
        if(request)
            DCE2_Smb2RemoveRequest(ssd, request);
    }
    else
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Wrong format for smb create command!\n"));
    return;
}

/********************************************************************
 *
 * Process close command
 * For some upload, file_size is decided here.
 *
 ********************************************************************/
static void DCE2_Smb2CloseCmd(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    /* Using structure size to decide whether it is response or request*/
    uint16_t structure_size;
    Smb2CloseRequestHdr *smb_close_hdr = (Smb2CloseRequestHdr *)smb_data;
    uint64_t fileId_persistent;

    if ((const uint8_t *)smb_close_hdr + SMB2_CLOSE_REQUEST_STRUC_SIZE > end)
        return;

    fileId_persistent = SmbNtohq((const uint64_t *)(&(smb_close_hdr->fileId_persistent)));

    structure_size = SmbNtohs(&(smb_close_hdr->structure_size));

    if ((structure_size == SMB2_CLOSE_REQUEST_STRUC_SIZE) &&
            !ssd->ftracker.tracker.file.file_size
            && ssd->ftracker.tracker.file.file_offset)
    {
        bool upload = DCE2_SsnFromClient(ssd->sd.wire_pkt) ? true : false;
        ssd->ftracker.tracker.file.file_size = ssd->ftracker.tracker.file.file_offset;
        _dpd.fileAPI->file_cache_update_entry(fileCache, (void *)ssd->sd.wire_pkt,
                fileId_persistent, NULL,  0, ssd->ftracker.tracker.file.file_size, false, false);
        DCE2_Smb2ProcessFileData(ssd, NULL, 0, upload);
    }
}

/********************************************************************
 *
 * Process set info command
 * For upload, file_size is decided here.
 *
 ********************************************************************/
static void DCE2_Smb2SetInfo(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    /* Using structure size to decide whether it is response or request*/
    uint16_t structure_size;
    Smb2ASetInfoRequestHdr *smb_set_info_hdr = (Smb2ASetInfoRequestHdr *)smb_data;
    uint64_t fileId_persistent;

    if ((const uint8_t *)smb_set_info_hdr + SMB2_SET_INFO_REQUEST_STRUC_SIZE > end)
        return;

    fileId_persistent = SmbNtohq((const uint64_t *)(&(smb_set_info_hdr->fileId_persistent)));

    structure_size = SmbNtohs(&(smb_set_info_hdr->structure_size));

    if (structure_size == SMB2_SET_INFO_REQUEST_STRUC_SIZE)
    {
        uint8_t *file_data =  (uint8_t *) smb_set_info_hdr + SMB2_SET_INFO_REQUEST_STRUC_SIZE - 1;
        if (smb_set_info_hdr->file_info_class == SMB2_FILE_ENDOFFILE_INFO)
        {
            uint64_t file_size;
            file_size = SmbNtohq((const uint64_t *)file_data);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Get file size %u!\n", file_size ));
            ssd->ftracker.tracker.file.file_size = file_size;
            _dpd.fileAPI->file_cache_update_entry(fileCache, (void *)ssd->sd.wire_pkt,
                    fileId_persistent, NULL,  0, file_size, false, false);
        }
    }
    return;
}

/********************************************************************
 *
 * Process read request
 *
 ********************************************************************/
static void DCE2_Smb2ReadRequest(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        Smb2ReadRequestHdr *smb_read_hdr, const uint8_t *end)
{
    uint64_t message_id, offset;
    uint64_t fileId_persistent;
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing read request command!\n"));
    message_id = SmbNtohq((const uint64_t *)(&(smb_hdr->message_id)));
    offset = SmbNtohq((const uint64_t *)(&(smb_read_hdr->offset)));
    fileId_persistent = SmbNtohq((const uint64_t *)(&(smb_read_hdr->fileId_persistent)));
    DCE2_Smb2StoreReadRequest(ssd, message_id, SMB2_COM_READ, offset, fileId_persistent);
    if (fileId_persistent && (ssd->ftracker.fid_v2 != fileId_persistent))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Persistent file ID changed read request command!\n"));
        ssd->ftracker.fid_v2 = fileId_persistent;
    }
    if (ssd->ftracker.tracker.file.file_size && (offset > ssd->ftracker.tracker.file.file_size))
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_INVALID_FILE_OFFSET);
    }
}

/********************************************************************
 *
 * Process read response
 *
 ********************************************************************/
static void DCE2_Smb2ReadResponse(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        Smb2ReadResponseHdr *smb_read_hdr, const uint8_t *end)
{
    uint8_t *file_data =  (uint8_t *) smb_read_hdr + SMB2_READ_RESPONSE_STRUC_SIZE - 1;
    int data_size = end - file_data;
    uint32_t total_data_length;
    uint64_t message_id;
    uint16_t data_offset;
    Smb2Request *request;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing read response command!\n"));

    message_id = SmbNtohq((const uint64_t *)(&(smb_hdr->message_id)));
    request = DCE2_Smb2GetRequest(ssd, message_id);
    if (!request || request->command != SMB2_COM_READ)
    {
        return;
    }
    data_offset = SmbNtohs((const uint16_t *)(&(smb_read_hdr->data_offset)));
    if (data_offset + (const uint8_t *)smb_hdr > end)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_OFF, data_offset + smb_hdr, smb_hdr, end);
    }

    ssd->ftracker.tracker.file.file_offset = request->read_req.offset;
    ssd->ftracker.fid_v2 = request->read_req.file_id;
    ssd->ftracker.tracker.file.file_direction = DCE2_SMB_FILE_DIRECTION__DOWNLOAD;

    DCE2_Smb2RemoveRequest(ssd, request);

    DCE2_Smb2ProcessFileData(ssd, file_data, data_size, false);
    ssd->ftracker.tracker.file.file_offset += data_size;
    total_data_length = SmbNtohl((const uint32_t *)&(smb_read_hdr->length));
    if (total_data_length > (uint32_t)data_size)
        ssd->pdu_state = DCE2_SMB_PDU_STATE__RAW_DATA;
}

/********************************************************************
 *
 * Process read command
 *
 ********************************************************************/
static void DCE2_Smb2Read(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    uint16_t structure_size;
    Smb2ReadRequestHdr *smb_read_hdr = (Smb2ReadRequestHdr *)smb_data;
    structure_size = SmbNtohs(&(smb_read_hdr->structure_size));

    /* Using structure size to decide whether it is response or request*/
    if (structure_size == SMB2_READ_REQUEST_STRUC_SIZE)
    {
        if ((const uint8_t *)smb_read_hdr + SMB2_READ_REQUEST_STRUC_SIZE - 1 > end)
            return;
        DCE2_Smb2ReadRequest(ssd, smb_hdr, (Smb2ReadRequestHdr *)smb_read_hdr, end);
    }
    else if (structure_size == SMB2_READ_RESPONSE_STRUC_SIZE)
    {
        if ((const uint8_t *)smb_read_hdr + SMB2_READ_RESPONSE_STRUC_SIZE - 1 > end)
            return;
        DCE2_Smb2ReadResponse(ssd, smb_hdr, (Smb2ReadResponseHdr *)smb_read_hdr, end);
    }
    else
    {
        uint64_t message_id;
        Smb2Request *request;
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Wrong format for smb read command!\n"));
        message_id = SmbNtohq((const uint64_t *)(&(smb_hdr->message_id)));
        request = DCE2_Smb2GetRequest(ssd, message_id);
        if (!request)
        {
            return;
        }
        DCE2_Smb2RemoveRequest(ssd, request);
    }
    return;
}

/********************************************************************
 *
 * Process write request
 *
 ********************************************************************/
static void DCE2_Smb2WriteRequest(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        Smb2WriteRequestHdr *smb_write_hdr, const uint8_t *end)
{
    uint8_t *file_data =  (uint8_t *) smb_write_hdr + SMB2_WRITE_REQUEST_STRUC_SIZE - 1;
    int data_size = end - file_data;
    uint64_t fileId_persistent, offset;
    uint16_t data_offset;
    uint32_t total_data_length;
    uint64_t file_size = UNKNOWN_FILE_SIZE;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing write request command!\n"));

    fileId_persistent = SmbNtohq((const uint64_t *)(&(smb_write_hdr->fileId_persistent)));
    if (fileId_persistent && (ssd->ftracker.fid_v2 != fileId_persistent))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Persistent file ID changed read request command!\n"));
        ssd->ftracker.fid_v2 = fileId_persistent;
    }
    data_offset = SmbNtohs((const uint16_t *)(&(smb_write_hdr->data_offset)));
    if (data_offset + (const uint8_t *)smb_hdr > end)
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_OFF, data_offset + smb_hdr, smb_hdr, end);
    }

    offset = SmbNtohq((const uint64_t *)(&(smb_write_hdr->offset)));
    if (ssd->ftracker.tracker.file.file_size && (offset > ssd->ftracker.tracker.file.file_size))
    {
        DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_INVALID_FILE_OFFSET);
    }
    ssd->ftracker.tracker.file.file_direction = DCE2_SMB_FILE_DIRECTION__UPLOAD;
    ssd->ftracker.tracker.file.file_offset = offset;
    if(offset == 0)
    {
        /* Try to update the file size with a max file size 
           if file size is not already updated.This is done to handle 
           some SMB clients which will not send SetInfo before uploading the file.
           This updates if size is not updated already in file_cache */
        _dpd.fileAPI->file_cache_update_entry(fileCache, (void *)ssd->sd.wire_pkt,
                fileId_persistent, NULL, 0, file_size, false, true);
    }

    DCE2_Smb2ProcessFileData(ssd, file_data, data_size, true);
    ssd->ftracker.tracker.file.file_offset += data_size;
    total_data_length = SmbNtohl((const uint32_t *)&(smb_write_hdr->length));
    if (total_data_length > (uint32_t)data_size)
        ssd->pdu_state = DCE2_SMB_PDU_STATE__RAW_DATA;
}

/********************************************************************
 *
 * Process write response
 *
 ********************************************************************/
static void DCE2_Smb2WriteResponse(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        Smb2WriteResponseHdr *smb_write_hdr, const uint8_t *end)
{
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Processing write response command!\n"));
}

/********************************************************************
 *
 * Process write command
 *
 ********************************************************************/
static void DCE2_Smb2Write(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr,
        uint8_t *smb_data, const uint8_t *end)
{
    uint16_t structure_size;
    Smb2WriteRequestHdr *smb_write_hdr = (Smb2WriteRequestHdr *)smb_data;
    structure_size = SmbNtohs(&(smb_write_hdr->structure_size));

    /* Using structure size to decide whether it is response or request*/
    if (structure_size == SMB2_WRITE_REQUEST_STRUC_SIZE)
    {
        if ((const uint8_t *)smb_write_hdr + SMB2_WRITE_REQUEST_STRUC_SIZE - 1 > end)
            return;
        DCE2_Smb2WriteRequest(ssd, smb_hdr, (Smb2WriteRequestHdr *)smb_write_hdr, end);
    }
    else if (structure_size == SMB2_WRITE_RESPONSE_STRUC_SIZE)
    {
        if ((const uint8_t *)smb_write_hdr + SMB2_WRITE_RESPONSE_STRUC_SIZE - 1 > end)
            return;
        DCE2_Smb2WriteResponse(ssd,  smb_hdr, (Smb2WriteResponseHdr *)smb_write_hdr, end);
    }
    else
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Wrong format for smb write command!\n"));
    return;
}

/********************************************************************
 *
 * Purpose:
 *  Process SMB2 commands.
 *
 * Arguments:
 *  DCE2_SmbSsnData * - the session data structure.
 *  const Smb2Hdr *  - pointer to the SMB2 header.
 *  const uint8_t *  - pointer to end of payload.
 * Returns: None
 *
 ********************************************************************/
static void DCE2_Smb2Inspect(DCE2_SmbSsnData *ssd, const Smb2Hdr *smb_hdr, const uint8_t *end)
{
    uint8_t *smb_data = (uint8_t *)smb_hdr + SMB2_HEADER_LENGTH;
    uint16_t command = SmbNtohs(&(smb_hdr->command));
    DCE2_Ret smb2_create_ret = DCE2_RET__SUCCESS;
    switch (command)
    {
    case SMB2_COM_CREATE:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Create command.\n"));
        dce2_stats.smb2_create++;
        /*If Tid is found, call DCE2_Smb2Create only if its a DISK_SHARE or its a ASYNC COMMAND*/
        smb2_create_ret = DCE2_Smb2FindTid(ssd, smb_hdr);
        if ((smb2_create_ret == DCE2_RET__SUCCESS) &&
                ((SmbNtohl(&(smb_hdr->flags)) & SMB2_FLAGS_ASYNC_COMMAND) || (DCE2_Smb2ShareType(ssd, Smb2Tid(smb_hdr)) == SMB2_SHARE_TYPE_DISK)))
        {
            DCE2_Smb2Create(ssd, smb_hdr, smb_data, end);
        }
        /*If Tid is not found, add the Tid as DISK_SHARE and call DCE2_Smb2Create*/
        else if (smb2_create_ret != DCE2_RET__SUCCESS)
        {
            DCE2_Smb2InsertTid(ssd, Smb2Tid(smb_hdr), SMB2_SHARE_TYPE_DISK);
            DCE2_Smb2Create(ssd, smb_hdr, smb_data, end);
        }
        break;
    case SMB2_COM_READ:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Read command.\n"));
        dce2_stats.smb2_read++;
        if (DCE2_Smb2FindTid(ssd, smb_hdr) != DCE2_RET__SUCCESS)
            return;
        DCE2_Smb2Read(ssd, smb_hdr, smb_data, end);
        break;
    case SMB2_COM_WRITE:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Write command.\n"));
        dce2_stats.smb2_write++;
        if (DCE2_Smb2FindTid(ssd, smb_hdr) != DCE2_RET__SUCCESS)
            return;
        DCE2_Smb2Write(ssd, smb_hdr, smb_data, end);
        break;
    case SMB2_COM_SET_INFO:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Set info command.\n"));
        dce2_stats.smb2_set_info++;
        if (DCE2_Smb2FindTid(ssd, smb_hdr) != DCE2_RET__SUCCESS)
            return;
        DCE2_Smb2SetInfo(ssd, smb_hdr, smb_data, end);
        break;
    case SMB2_COM_CLOSE:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Close command.\n"));
        dce2_stats.smb2_close++;
        if (DCE2_Smb2FindTid(ssd, smb_hdr) != DCE2_RET__SUCCESS)
            return;
        DCE2_Smb2CloseCmd(ssd, smb_hdr, smb_data, end);
        break;
    case SMB2_COM_TREE_CONNECT:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tree connect command.\n"));
        dce2_stats.smb2_tree_connect++;
        DCE2_Smb2TreeConnect(ssd, smb_hdr, smb_data, end);
        break;
    case SMB2_COM_TREE_DISCONNECT:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "Tree disconnect command.\n"));
        dce2_stats.smb2_tree_disconnect++;
        DCE2_Smb2TreeDisconnect(ssd, smb_hdr, smb_data, end);
        break;
    default:
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__SMB, "command ignored!\n"));
        break;
    }
    return;
}

/********************************************************************
 * Function: DCE2_SmbProcess()
 *
 * Purpose:
 *  This is the main entry point for SMB processing.
 *
 * Arguments:
 *  DCE2_SmbSsnData * - the session data structure.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_Smb2Process(DCE2_SmbSsnData *ssd)
{
    const SFSnortPacket *p = ssd->sd.wire_pkt;
    const uint8_t *data_ptr = p->payload;
    uint16_t data_len = p->payload_size;
    Smb2Hdr *smb_hdr;
    const uint8_t *end = data_ptr +  data_len;
    uint32_t next_command_offset = 0;

#ifdef DUMP_BUFFER
    dumpBuffer(DCERPC_SMB2_DUMP,data_ptr,data_len);
#endif
    if (ssd && ssd->pdu_state != DCE2_SMB_PDU_STATE__RAW_DATA)
    {
        /*Check header length*/
        if (data_len < sizeof(NbssHdr) + SMB2_HEADER_LENGTH)
            return;

        if (!ssd->ftracker.is_smb2)
        {
            DCE2_Smb2InitFileTracker(&(ssd->ftracker), 0, 0);
        }
    }

    /* Process the header */
    if (PacketHasStartOfPDU(p))
    {

        smb_hdr = (Smb2Hdr *)(data_ptr + sizeof(NbssHdr));
        /*
         * SMB protocol allows multiple smb commands
         * to be clubbed in a single packet.
         * So loop through to parse all the smb
         * commands.
         */
        do
        {
            DCE2_Smb2Inspect(ssd, (Smb2Hdr *)smb_hdr, end);
            /*
             * In case of message compounding, find the offset
             * of the next smb command.
             */
            next_command_offset = SmbNtohl(&(smb_hdr->next_command));

            if (next_command_offset + (uint8_t *)smb_hdr > end)
            {
                DCE2_Alert(&ssd->sd, DCE2_EVENT__SMB_BAD_NEXT_COMMAND_OFFSET);
                return;
            }

            if (next_command_offset)
            {
                smb_hdr = (Smb2Hdr *)((uint8_t *)smb_hdr +
                                                next_command_offset);
            }
        } while (next_command_offset && smb_hdr);
    }
    else if (ssd->pdu_state == DCE2_SMB_PDU_STATE__RAW_DATA)
    {
        /*continue processing raw data*/
        bool upload = DCE2_SsnFromClient(ssd->sd.wire_pkt) ? true : false;
        DCE2_Smb2ProcessFileData(ssd, data_ptr, data_len, upload);
        ssd->ftracker.tracker.file.file_offset += data_len;
    }
}

/* Initialize smb2 file tracker */
DCE2_Ret DCE2_Smb2InitFileTracker( DCE2_SmbFileTracker *ftracker,
        const bool is_ipc, const uint64_t fid)
{
    if (ftracker == NULL)
        return DCE2_RET__ERROR;

    DCE2_Smb2FreeFileName(ftracker);
    ftracker->fid_v2 = fid;
    ftracker->is_ipc = is_ipc;
    ftracker->is_smb2 = true;

    ftracker->ff_file_size = 0;
    ftracker->ff_file_offset = 0;
    ftracker->ff_file_direction = DCE2_SMB_FILE_DIRECTION__UNKNOWN;

    return DCE2_RET__SUCCESS;
}

void DCE2_Smb2UpdateStats(void)
{
    if (fileCache)
    {
        FileCacheStatus *status = _dpd.fileAPI->file_cache_status(fileCache);
        dce2_stats.smb2_prunes = status->prunes;
        dce2_stats.smb2_memory_in_use = status->segment_mem_in_use;
        dce2_stats.smb2_memory_in_use_max = status->segment_mem_in_use_max;
    }
}

/* Check whether the packet is smb2 */
DCE2_SmbVersion DCE2_Smb2Version(const SFSnortPacket *p)
{
    /* SMB2 requires PAF enabled*/
    if (!_dpd.isPafEnabled() || !DCE2_SsnIsPafActive(p))
    {
        return DCE2_SMB_VERISON_NULL;
    }

    /* Only check reassembled SMB2 packet*/
    if (IsTCP(p) && DCE2_SsnIsRebuilt(p) &&
            (p->payload_size > sizeof(NbssHdr) + sizeof(DCE2_SMB_ID)))
    {
        Smb2Hdr *smb_hdr = (Smb2Hdr *)(p->payload + sizeof(NbssHdr));
        uint32_t smb_version_id = SmbId((SmbNtHdr *)smb_hdr);
        if (smb_version_id == DCE2_SMB_ID)
            return DCE2_SMB_VERISON_1;
        else if (smb_version_id == DCE2_SMB2_ID)
            return DCE2_SMB_VERISON_2;
    }

    return DCE2_SMB_VERISON_NULL;
}

void DCE2_Smb2CleanRequests(Smb2Request *requests)
{
    Smb2Request *request = requests;
    while (request)
    {
        Smb2Request *next;
        next = request->next;
        DCE2_Free(request, sizeof (*request), DCE2_MEM_TYPE__SMB_SSN);
        request = next;
    }
}

void DCE2_Smb2Init(uint64_t memcap)
{
    if (!fileCache)
    {
        uint64_t mem_for_smb2 = memcap >> 1;
        fileCache = _dpd.fileAPI->file_cache_create (mem_for_smb2, 5);
        DCE2_RegMem(mem_for_smb2, DCE2_MEM_TYPE__SMB_SSN);
    }
}

void DCE2_Smb2Close(void)
{
    if (fileCache)
    {
        DCE2_Smb2UpdateStats();
        _dpd.fileAPI->file_cache_free (fileCache);
        fileCache = NULL;
    }
}

bool DCE2_IsFileCache(void *ptr)
{
    return fileCache == ptr;
}

#ifdef SNORT_RELOAD

/*********************************************************************
 * Function: DCE2_Smb2AdjustFileCache
 *
 * Purpose: Adapts file cache to new config in bursts of effort
 *
 * Arguments:   work            - size of work burst
*               cache_enabled   - cache is enabled in new config
 *
 * Returns: true when file cache meets memcaps, else false
 *
 *********************************************************************/
bool DCE2_Smb2AdjustFileCache(uint8_t work, bool cache_enabled)
{
    //TODO (nice to have) Adjust number of rows in hash table based on new size

    /* Delete files until new max files cap is met
     * Delete files until new segment memcap is met
     * Delete file cache if no longer used
     * return true when done, else false
    */
    bool file_cache_adapted = _dpd.fileAPI->file_cache_shrink_to_memcap(fileCache, &work);
    if (file_cache_adapted && !cache_enabled)
       DCE2_Smb2Close();

    return file_cache_adapted;
}

void DCE2_SetSmbMemcap(uint64_t smbMemcap)
{
    _dpd.fileAPI->file_cache_set_memcap(fileCache, smbMemcap);
}


#endif
