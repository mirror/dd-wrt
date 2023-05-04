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
 *  Author(s):  Hui Cao <huica@cisco.com>
 ****************************************************************************/

#include "file_segment_process.h"
#include "parser.h"
#include "file_resume_block.h"
#include "file_config.h"
#include "memory_stats.h"

#ifdef REG_TEST
#include "file_stats.h"
#include "reg_test.h"
#include <stdio.h>
#endif

#define   UNKNOWN_FILE_SIZE           ~0

extern FileSession* get_file_session(void *ssnptr);

static inline void file_segment_free(FileCache *fileCache, FileSegment* file_segment)
{
    if (!file_segment)
        return;

    if (fileCache)
	{
		if(fileCache->status.segment_mem_in_use >= file_segment->segment_size)
			fileCache->status.segment_mem_in_use -= file_segment->segment_size;
		else
			fileCache->status.segment_mem_in_use = 0;
	}

	SnortPreprocFree(file_segment, sizeof(FileSegment), PP_FILE, PP_MEM_CATEGORY_SESSION);
}

static inline void file_segments_free (FileEntry *file_entry)
{
    FileSegment *current_segment;
    current_segment = file_entry->segments;
    while (current_segment)
    {
        FileSegment *previous_segment = current_segment;
        current_segment = current_segment->next;
        file_segment_free(file_entry->file_cache, previous_segment);
    }

    file_entry->segments = NULL;
    file_entry->offset = 0;
}

static inline void file_entry_free(FileEntry *file_entry)
{

    if (!file_entry)
        return;

    if (file_entry->file_name)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE,
                              "File name: %s released (%p)\n", file_entry->file_name, file_entry->file_name));
        SnortPreprocFree(file_entry->file_name, file_entry->file_name_size, PP_FILE, PP_MEM_CATEGORY_SESSION);
        file_entry->file_name = NULL;
        file_entry->file_name_size = 0;
    }

    if (file_entry->context)
    {
        file_entry->context->attached_file_entry = NULL;
        file_context_free(file_entry->context);
        file_entry->context = NULL;
    }

    file_entry->file_size = 0;

    file_segments_free(file_entry);

}

static int file_entry_free_func(void *option_key, void *data)
{
    FileEntry *file_entry = ( FileEntry *)data;
    file_entry_free(file_entry);
    return 0;
}

/* Prune file entries based on LRU
 */
static int  pruneFileCache(FileCache *fileCache, FileEntry *file)
{
    SFXHASH_NODE  *lru_node = NULL;
    int pruned = 0;
    int mustdie = fileCache->cleanup_files;

    while (pruned < mustdie &&
            (sfxhash_count(fileCache->hashTable) > 0))
    {
        if ((lru_node =  sfxhash_lru_node(fileCache->hashTable)) != NULL)
        {
            if (lru_node->data == file)
                break;
            if (sfxhash_free_node(fileCache->hashTable, lru_node) != SFXHASH_OK)
            {
                LogMessage("WARNING: failed to remove file entry from hash.\n");
                break;
            }
            pruned++;
        }
    }

    fileCache->status.prunes += pruned;
    return pruned;
}

FileEntry *file_cache_get(FileCache *fileCache, void* p, uint64_t file_id,
    bool can_create)
{
    SFXHASH_NODE *hnode;
    FileKey fileKey;
    Packet *pkt = (Packet *)p;
    sfaddr_t* srcIP;
    sfaddr_t* dstIP;
    SAVE_DAQ_PKT_HDR(p);

    if ((fileCache == NULL) || (fileCache->hashTable == NULL))
    {
        FILE_WARNING("Failed to get file cache info");
        return NULL;
    }

    if ((pkt->packet_flags & PKT_FROM_CLIENT))
    {
        srcIP = GET_SRC_IP(pkt);
        dstIP = GET_DST_IP(pkt);
    }
    else
    {
        srcIP = GET_DST_IP(pkt);
        dstIP = GET_SRC_IP(pkt);
    }

    sfaddr_copy_to_raw(&fileKey.dip, dstIP);
    sfaddr_copy_to_raw(&fileKey.sip, srcIP);
    fileKey.file_id = file_id;

    if (!can_create)
    {
        hnode = sfxhash_find_node(fileCache->hashTable, &fileKey);
    }
    else
    {

        hnode = sfxhash_get_node(fileCache->hashTable, &fileKey);

        if (!hnode)
        {
            /*No more file entries, free up some old ones*/
            if(pruneFileCache(fileCache, NULL) == 0)
            {
                FILE_WARNING("No free node available");
                return NULL;
            }

            /* Should have some freed nodes now */
            hnode = sfxhash_get_node(fileCache->hashTable, &fileKey);

#ifdef DEBUG_MSGS
            if (!hnode)
                LogMessage("%s(%d) Problem, no freed nodes\n", __FILE__, __LINE__);
#endif
        }
    }

    if (hnode && hnode->data)
    {
        FileEntry *file_entry = (FileEntry *)hnode->data;

        return file_entry;
    }
    else
    {
        FILE_WARNING("No free node available");
        return NULL;
    }
}

/* Initialize file cache based on memcap
 * File cache cost includes three part:
 * 1) file hash table
 * 2) file context
 * 3) file segment
 *
 * Both 1) and 2) can be limited by maximal files, 3) can be limited by memcap.
 */
static inline uint32_t get_max_files_from_memcap (uint64_t memcap)
{
    /* Per file cost*/
    uint32_t per_file_cost = sizeof(FileContext) + sizeof(FileKey) + sizeof(FileEntry);

    return (memcap/per_file_cost);
}

static inline FileSegment* file_segment_alloc (FileCache *fileCache,
        const uint8_t* file_data, int data_size, uint64_t offset, FileEntry *file)
{
    FileSegment* ss;
    unsigned int size = sizeof(*ss);

    if ( data_size > 0 )
        size += (uint64_t)data_size - 1;  /* ss contains 1st byte */
    else
        return NULL;

    /* Check against memcap here*/
    if ((fileCache->status.segment_mem_in_use + size) > fileCache->file_segment_memcap)
    {
        /* make more memory available by pruning. Return NULL if nothing is pruned*/
        if(pruneFileCache(fileCache, file) == 0)
            return NULL;
        else if ((fileCache->status.segment_mem_in_use + size) > fileCache->file_segment_memcap)
            return NULL;
    }

    fileCache->status.segment_mem_in_use += size;

    ss = (FileSegment*) SnortPreprocAlloc(1, size, PP_FILE, PP_MEM_CATEGORY_SESSION);
    ss->segment_size = size;
    ss->size = data_size;
    ss->offset = offset;
    memcpy(ss->data, file_data, data_size);

    if (fileCache->status.segment_mem_in_use_max < fileCache->status.segment_mem_in_use)
    {
        fileCache->status.segment_mem_in_use_max = fileCache->status.segment_mem_in_use;
    }

    return ss;
}

/* Update the segment list based on new data
 * Use the original data if possible
 * Input:
 *    offset: offset in file for the new segment
 *    data_size: size of new segment
 *    file: the file entry
 */
static inline int _file_segments_update( FileCache *fileCache,
        const uint8_t* file_data, uint64_t offset,
        int data_size, FileEntry *file)
{
    FileSegment *current_segment = file->segments;
    uint64_t start = offset;
    uint64_t end = offset + data_size;
    FileSegment *new_segment;
    /* left points to segment that "next" pointer needs to be updated */
    FileSegment *left = NULL;
    FileSegment *previous = NULL;
    bool find_left = false;

    /* Find left boundary, left points to segment that needs update*/
    while (current_segment)
    {
        if (current_segment->offset > start)
        {
            find_left = true;
            left = previous;
            break;
        }

        previous = current_segment;
        current_segment = current_segment->next;
    }

    if (find_left)
    {
        if (!left)
        {   
            /* Need to insert at begining */
            if (end > file->segments->offset)
            {   
                /* Overlap, trim off exrta data from end */
                data_size = file->segments->offset - offset;
            }
        }
        else
        {   
            /* Need to insert in the middle */
            if (left->offset + left->size > start)
            {   
                /* Overlap, trim begining */
                offset = left->offset +left->size;
                data_size = end - offset;
                file_data = file_data + offset - start;
            }
            if (left->next->offset < end)
            {   
                /* Overlap, trim end of data */
                data_size = left->next->offset - offset;
            }
        }
    }
    else if (previous)
    {   
        /* Need to insert at end */
        left = previous;
        if (left->offset + left->size > start)
        {   
            /* Overlap, trim begining */
            offset = left->offset + left->size;
            data_size = end - offset;
            file_data = file_data + offset - start;
        }
    }
    if (data_size > 0)
    {
        new_segment = file_segment_alloc(fileCache, file_data, data_size, offset, file);
        if (!new_segment)
            return 0;
    }
    else
    {   
        /* data_size <= 0 means a complete coverlapped segment we have already seen */
		FILE_DEBUG("Complete overlapped segment, discarding.");
        return 0;
    }
    
    if (!left)
    {   
        /* inserting at begining or list is empty */
        new_segment->next = file->segments;
        file->segments = new_segment;
    }
    else
    {   
        /* inserting at end or middle */
        new_segment->next = left->next;
        left->next = new_segment;
    }
    return 1;
}

static inline FilePosition get_file_position(uint64_t file_size, int data_size,
        uint64_t offset)
{
    if (offset == 0)
    {
        if (file_size == (uint64_t) data_size)
            return SNORT_FILE_FULL;
        else
            return SNORT_FILE_START;
    }

    if (file_size <= data_size + offset)
        return SNORT_FILE_END;

    return SNORT_FILE_MIDDLE;
}

static inline int _process_one_file_segment (void* p, FileEntry *fileEntry,
        const uint8_t* file_data, int *data_size, uint64_t file_size)
{
    int ret;

    if(fileEntry->offset < file_size && fileEntry->offset + *data_size > file_size)
        *data_size = file_size - fileEntry->offset;

    FilePosition position = get_file_position(file_size, *data_size, fileEntry->offset);
    FILE_DEBUG("Processing segment, File size: %u, data size: %d, File position: %d",file_size,*data_size,position);

    if(fileEntry->file_size != UNKNOWN_FILE_SIZE)
    {
        if (position == SNORT_FILE_END && fileEntry->context && fileEntry->context->smb_unknown_file_size)
        {
            if((fileEntry->context->file_state.sig_state == FILE_SIG_FLUSH) && fileEntry->context->sha256)
            {
                SnortPreprocFree(fileEntry->context->sha256, SHA256_HASH_SIZE, PP_FILE, PP_MEM_CATEGORY_SESSION);
                fileEntry->context->sha256 = NULL;
            } 
            fileEntry->context->smb_unknown_file_size = false;
        }
        ret = file_api->process_file(fileEntry->context, p, (uint8_t *)file_data, *data_size, position, false);
    }
    else
    {
        Packet *pkt = (Packet *)p;
        if((fileEntry->context->file_state.sig_state == FILE_SIG_FLUSH) && fileEntry->context && fileEntry->context->sha256)
        {
            SnortPreprocFree(fileEntry->context->sha256, SHA256_HASH_SIZE, PP_FILE, PP_MEM_CATEGORY_SESSION);
            fileEntry->context->sha256 = NULL;
        }
        if(pkt->packet_flags & PKT_PDU_TAIL)
        {
            fileEntry->context->file_state.sig_state = FILE_SIG_FLUSH;
            fileEntry->context->smb_unknown_file_size = true;
        }
        else
            fileEntry->context->file_state.sig_state = FILE_SIG_PROCESSING;

        ret = file_api->process_file(fileEntry->context, p, (uint8_t *)file_data, *data_size, position, false);
    }
    return ret;
}

static inline int _process_file_segments(FileCache *fileCache, void* p, FileEntry *fileEntry,
        uint64_t file_size)
{
    int ret = 1;
    /*Process the packet update the offset */
    FileSegment *current_segment = fileEntry->segments;
    while (current_segment && (fileEntry->offset == current_segment->offset))
    {
        ret = _process_one_file_segment(p, fileEntry, current_segment->data,
                (int *)&current_segment->size, file_size);

        if (!ret)
        {
            file_segments_free(fileEntry);
            break;
        }

        fileEntry->offset += current_segment->size;
        fileEntry->segments = current_segment->next;
        file_segment_free(fileCache, current_segment);
        current_segment = fileEntry->segments;
    }

    return ret;
}

/* Create file cache */
FileCache *file_cache_create(uint64_t memcap, uint32_t cleanup_files)
{
    FileCache *fileCache = NULL;
    int  max_files = 0;
    uint64_t file_segment_memcap = memcap/2;

    if( !memcap )
    {
        WarningMessage("%s(%d) File cache memory unlimited!\n",
                file_name, file_line);
    }

    /* Half for file segment, half for file context tracking*/
    max_files = get_max_files_from_memcap(memcap - file_segment_memcap);

    fileCache = SnortPreprocAlloc(1, sizeof(*fileCache), PP_FILE, PP_MEM_CATEGORY_SESSION);
    if( fileCache )
    {
        fileCache->max_files = max_files;
        /* Okay, now create the table */
        fileCache->hashTable = sfxhash_new(max_files, sizeof(FileKey), sizeof(FileEntry),
                0, 0, NULL, file_entry_free_func, 1 );

        if (!fileCache->hashTable)
        {
            FatalError( "%s(%d) Unable to create a file cache.\n", file_name, file_line);
        }

        sfxhash_set_max_nodes( fileCache->hashTable, max_files );
        fileCache->file_segment_memcap = file_segment_memcap;
        fileCache->cleanup_files = cleanup_files;
    }
    else
    {
        FatalError( "%s(%d) Unable to create a file cache.\n",
                file_name, file_line);
    }

    return fileCache;
}

/* Release file cache */
void file_cache_free( FileCache *fileCache )
{
    if (fileCache)
    {
        sfxhash_delete(fileCache->hashTable);
        SnortPreprocFree(fileCache, sizeof(FileCache), PP_FILE, PP_MEM_CATEGORY_SESSION);
    }
}

/* Add/update a file entry specified by file_id in the file cache*/
void *file_cache_update_entry (FileCache *fileCache, void* p, uint64_t file_id,
        uint8_t *file_name, uint32_t file_name_size, uint64_t file_size, bool reset, bool no_update_size)
{
    FileEntry *fileEntry;

    fileEntry = file_cache_get(fileCache, p, file_id, true);

    if (!fileEntry)
    {
        FILE_DEBUG("Failed to add file entry: file_id %d not found in cache",file_id);
        return NULL;
    }

    if(fileEntry->context && reset)
        file_entry_free(fileEntry);

    /* If no context, set resume check */
    if(!fileEntry->context)
        fileEntry->file_resume_check = true;

    if (file_name)
    {
        FILE_DEBUG("Add file: %s (%p) with file id %d", file_name,file_name,file_id);
        if (fileEntry->file_name && fileEntry->file_name != file_name)
        {
            FILE_DEBUG("File name: %s released (%p)", fileEntry->file_name,fileEntry->file_name);
            SnortPreprocFree(fileEntry->file_name, fileEntry->file_name_size, PP_FILE, PP_MEM_CATEGORY_SESSION);
        }
        fileEntry->file_name = file_name;
        fileEntry->file_name_size = file_name_size;
    }

    if (file_size)
    {
        if(no_update_size)
        {
            if(!fileEntry->file_size)
                fileEntry->file_size = file_size;
        }
        else 
        {
            fileEntry->file_size = file_size;
        }
    }

    return fileEntry;
}

static inline void update_file_session(void *ssnptr, FileCache *fileCache,
    uint64_t file_id, FileContext *context)
{
    FileSession *file_session;

    if (!file_api->set_current_file_context(ssnptr, context))
        return;

    file_session = get_file_session (ssnptr);

    if (!file_session->file_cache)
        file_session->file_cache = fileCache;

    file_session->file_id = file_id;
    FILE_DEBUG("Updated file_id: %u, file cache %p in file session %p",file_id, fileCache, file_session);
}

/*
 * Process file segment, do file segment reassemble if the file segment is
 * out of order. file_id is unique, used as a key to find the file entity.
 * Return:
 *    1: continue processing/log/block this file
 *    0: ignore this file segment
 */
int file_segment_process( FileCache *fileCache, void* p, uint64_t file_id,
        uint64_t file_size, const uint8_t* file_data, int data_size, uint64_t offset,
        bool upload)
{
    FileEntry *fileEntry;
    int ret = 0;
    Packet *pkt = (Packet *)p;
    void *ssnptr = pkt->ssnptr;
    File_Verdict verdict = FILE_VERDICT_UNKNOWN;
    uint32_t file_sig = 0;
    SAVE_DAQ_PKT_HDR(p);
    FILE_DEBUG("Processing segment: file_id: %u, file_size: %u, data_size: %d, offset: %u, direction: %d",file_id, file_size, data_size, offset, upload);

    fileEntry = file_cache_get(fileCache, p, file_id, true);

    if (fileEntry == NULL)
    {
        FILE_ERROR("Processing segment failed: no file entry in cache");
        return 0;
    }

    if (!fileEntry->context)
        fileEntry->file_resume_check = true;

    if(fileEntry->file_resume_check)
    {
        if(fileEntry->file_name_size > 0)
        {
            file_sig = file_api->str_to_hash(fileEntry->file_name, fileEntry->file_name_size);
        }
        else if(fileEntry->context && fileEntry->context->file_name_size > 0 && fileEntry->context->file_name)
        {
            file_sig = file_api->str_to_hash(fileEntry->context->file_name, fileEntry->context->file_name_size);
        }

        if(file_sig)
            verdict = file_resume_block_check(p, file_sig);

        if (verdict == FILE_VERDICT_BLOCK || verdict == FILE_VERDICT_REJECT || verdict == FILE_VERDICT_PENDING)
        {
#ifdef HAVE_DAQ_DP_ADD_DC
            DAQ_DC_Params params;
            sfaddr_t *srcIP = GET_SRC_IP(pkt);
            sfaddr_t *dstIP = GET_DST_IP(pkt);
            memset(&params, 0, sizeof(params));
            params.flags = DAQ_DC_ALLOW_MULTIPLE;
            params.timeout_ms = 15 * 60 * 1000; /* 15 minutes */
            if (pkt->packet_flags & PKT_FROM_CLIENT)
                DAQ_Add_Dynamic_Protocol_Channel(pkt, srcIP, 0, dstIP,  pkt->dp, GET_IPH_PROTO(pkt),
                        &params);
            else if (pkt->packet_flags & PKT_FROM_SERVER)
                DAQ_Add_Dynamic_Protocol_Channel(pkt, dstIP, 0, srcIP, pkt->sp, GET_IPH_PROTO(pkt),
                        &params);
#endif
            FILE_INFO("Processing segment stopped, verdict: %d\n",verdict);
            return 1;
        }
        fileEntry->file_resume_check = false;
    }

    if (fileEntry->file_size)
        file_size = fileEntry->file_size;
    else
    {
        FILE_ERROR("Processing segment failed: file size 0");
        return 0;
    }

    if (!fileEntry->file_cache)
        fileEntry->file_cache = fileCache;

    if (!fileEntry->context)
    {
        fileEntry->context = file_api->create_file_context(ssnptr);
        file_api->init_file_context(ssnptr, upload, fileEntry->context);
        fileEntry->context->file_id = (uint32_t)file_id;
        fileEntry->context->attached_file_entry = fileEntry;
        update_file_session(ssnptr, fileCache, file_id, fileEntry->context);
        file_name_set(fileEntry->context, fileEntry->file_name, fileEntry->file_name_size, true);
    }
    else if (fileEntry->context->verdict != FILE_VERDICT_UNKNOWN && !fileEntry->context->smb_unknown_file_size)
    {
        /*A new file session, but policy might be different*/
        if (((fileEntry->context->sha256))
                || !fileEntry->context->file_signature_enabled )
        {
            /* Just check file type and signature */
            return _process_one_file_segment(p, fileEntry, file_data, &data_size, file_size);
        }
    }

    if(offset < fileEntry->offset && offset + data_size > fileEntry->offset)
    {
        file_data += (fileEntry->offset - offset);
        data_size = (offset + data_size)-fileEntry->offset;
        offset = fileEntry->offset;
    }

    /* Walk through the segments that can be flushed*/
    if (fileEntry->offset == offset)
    {
        /*Process the packet update the offset */
        ret = _process_one_file_segment(p, fileEntry, file_data, &data_size, file_size);
        fileEntry->offset += data_size;
        if (!ret)
        {
            file_segments_free(fileEntry);
            return 0;
        }

        ret = _process_file_segments(fileCache, p, fileEntry, file_size);
    }
    else if ((fileEntry->offset < file_size) && (fileEntry->offset < offset))
    {
        ret = _file_segments_update(fileCache, file_data, offset, data_size, fileEntry);
    }
#ifdef REG_TEST
    if(ret && fileEntry->file_name_size)
    {
        FILE_REG_DEBUG_WRAP(printFileContext(fileEntry->context));
        fileEntry->file_name_size = 0;
    }
#endif

    return ret;
}

/* Return the status of file cache */
FileCacheStatus *file_cache_status(FileCache *fileCache)
{
    return (&(fileCache->status));
}

static bool file_cache_prune_files(FileCache *fileCache, uint8_t *pWork)
{
#ifdef REG_TEST
    if (REG_TEST_FLAG_FILE_CACHE & getRegTestFlags())
        printf("file-cache prunefiles-before %u \n", sfxhash_count(fileCache->hashTable));
#endif
    for (; *pWork > 0 && sfxhash_count(fileCache->hashTable) > fileCache->hashTable->max_nodes; (*pWork)--)
        pruneFileCache(fileCache,NULL);
#ifdef REG_TEST
    if (REG_TEST_FLAG_FILE_CACHE & getRegTestFlags())
        printf("file-cache prunefiles-after %u \n", sfxhash_count(fileCache->hashTable));
#endif
    return sfxhash_count(fileCache->hashTable) <= fileCache->hashTable->max_nodes;
}

static bool file_cache_prune_segment(FileCache *fileCache, uint8_t *pWork)
{
#ifdef REG_TEST
    if (REG_TEST_FLAG_FILE_CACHE & getRegTestFlags())
        printf("file-cache prunesegment-before %"PRIu64" \n",
            fileCache->status.segment_mem_in_use);
#endif
    for (; *pWork > 0 && fileCache->status.segment_mem_in_use > fileCache->file_segment_memcap; (*pWork)--)
        pruneFileCache(fileCache,NULL);
#ifdef REG_TEST
    if (REG_TEST_FLAG_FILE_CACHE & getRegTestFlags())
        printf("file-cache prunesegment-after %"PRIu64" \n",
            fileCache->status.segment_mem_in_use);
#endif
    return fileCache->status.segment_mem_in_use <= fileCache->file_segment_memcap;
}

bool file_cache_shrink_to_memcap(FileCache *fileCache, uint8_t *pWork)
{
    if (fileCache == NULL)
        return true;

    bool cache_shrunk = file_cache_prune_files(fileCache, pWork) &&
                        file_cache_prune_segment(fileCache, pWork);
#ifdef REG_TEST
    if (cache_shrunk && REG_TEST_FLAG_FILE_CACHE & getRegTestFlags())
       printf("file-cache done 1\n");
#endif
    return cache_shrunk;
}

void file_cache_set_memcap(FileCache *fileCache, uint64_t memcap)
{
    if (fileCache == NULL)
        return;
    sfxhash_set_max_nodes(fileCache->hashTable, get_max_files_from_memcap(memcap/2));
    fileCache->file_segment_memcap = memcap/2;
#ifdef REG_TEST
    if (REG_TEST_FLAG_FILE_CACHE & getRegTestFlags())
    {
        printf("file-cache mem-files %"PRIu32" \n", get_max_files_from_memcap(memcap/2));
        printf("file-cache mem-segment %"PRIu64" \n", memcap/2);
    }
#endif
}

