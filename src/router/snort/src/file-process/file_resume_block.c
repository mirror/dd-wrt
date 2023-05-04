/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include "file_resume_block.h"
#include "sf_types.h"
#include "file_api.h"
#include "snort_bounds.h"
#include "ipv6_port.h"
#include "sfxhash.h"
#include "util.h"
#include "decode.h"
#include "active.h"
#include "sf_sechash.h"
#include "file_config.h"
#include "file_service.h"
#include "file_ss.h"
#include "sidechannel.h"
#include "file_lib.h"


/* The hash table of expected files */
static SFXHASH *fileHash = NULL;
extern FileServiceConfig cur_config;
extern FileContext* get_main_file_context(void *ssnptr);

static FileState sig_file_state = { FILE_CAPTURE_SUCCESS, FILE_SIG_DONE };
/* this file_cache_mutex is used to synchronize multiple add's to SFXHASH */
static pthread_mutex_t file_cache_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct _FileHashKey
{
    struct in6_addr sip;
    struct in6_addr dip;
    uint32_t file_sig;
} FileHashKey;

typedef struct _FileNode
{
    time_t expires;
    File_Verdict verdict;
    uint32_t   file_type_id;
    uint8_t sha256[SHA256_HASH_SIZE];
} FileNode;

#define MAX_FILES_TRACKED 16384

void file_resume_block_init(void)
{
    /* number of entries * overhead per entry */
    unsigned long maxmem = sfxhash_calc_maxmem(MAX_FILES_TRACKED,
			sizeof(FileHashKey) + sizeof(FileNode));

    fileHash = sfxhash_new(MAX_FILES_TRACKED, sizeof(FileHashKey), sizeof(FileNode),
			maxmem, 1, NULL, NULL, 1);

    if (!fileHash)
        FatalError("Failed to create the expected channel hash table.\n");
#ifdef SIDE_CHANNEL
    FileSSConfigInit();
#endif
}

void file_resume_block_cleanup(void)
{
    if (fileHash)
    {
        pthread_mutex_lock(&file_cache_mutex);
        sfxhash_delete(fileHash);
        fileHash = NULL;
        pthread_mutex_unlock(&file_cache_mutex);
    }
#if defined (SIDE_CHANNEL) && defined (REG_TEST)
    FileCleanSS();
#endif
}

static inline void updateFileNode(FileNode *node, File_Verdict verdict,
        uint32_t file_type_id, uint8_t *signature)
{
    node->verdict = verdict;
    node->file_type_id = file_type_id;
    if (signature)
    {
        memcpy(node->sha256, signature, SHA256_HASH_SIZE);
    }
}
#ifdef SIDE_CHANNEL
static int ProduceSSFileCache(FileHashKey *hk, FileNode *hv)
{
    uint32_t offset = 0;

    void *msg_handle = NULL;
    void *hdr_ptr = NULL;
    void *data_ptr = NULL;
    if (!hk || !hv)
    {
        return -1;
    }

    if (CreateFileSSUpdate(&msg_handle, &hdr_ptr, &data_ptr, SC_MSG_TYPE_FILE_SS_HOST_CACHE, sizeof(*hk) + sizeof(*hv)) != 0)
    {
        FILE_ERROR("Side channel: Failed to create side channel update");
        return -1;
    }

    if (data_ptr)
    {
        memcpy(data_ptr, hk, sizeof(*hk));
        offset += sizeof(*hk);

        memcpy((uint8_t *)data_ptr + offset, hv, sizeof(*hv));
        offset += sizeof(*hv);

        SendFileSSUpdate(msg_handle, hdr_ptr, data_ptr, SC_MSG_TYPE_FILE_SS_HOST_CACHE, offset);
    }
#ifdef REG_TEST
    LogMessage("produce verdict =%d file id =%d \n",hv->verdict,hv->file_type_id);
    file_sha256_print(hv->sha256);
#endif
    FILE_DEBUG("Side channel: Produce verdict: %d, file id: %d",hv->verdict,hv->file_type_id);
    return 0;
}

int ConsumeSSFileCache(const uint8_t *buf, uint32_t len)
{
    FileHashKey *hk;
    FileNode *hv;
    SFXHASH_NODE *hash_node;
    FileNode *node;

    if( !buf )
    {
        LogMessage("Side channel: No buffer\n");
        return -1;
    }

    if( len < sizeof(*hk) + sizeof(*hv) )
    {
        LogMessage("Side channel: length too small\n");
        return -1;
    }
    hk = (FileHashKey *)buf;
    hv = (FileNode *)(buf + sizeof(*hk));

    pthread_mutex_lock(&file_cache_mutex);
    hash_node = sfxhash_find_node(fileHash, hk);
    if (hash_node)
    {
      if (!(node = hash_node->data))
      {
        sfxhash_free_node(fileHash, hash_node);
      }
    }
    else
      node = NULL;

    if (node)
    {
      node->expires = hv->expires ;/* 20 minuts timeout*/
      updateFileNode(node, hv->verdict, hv->file_type_id, hv->sha256);
    }
    else if (sfxhash_add(fileHash, hk, hv) != SFXHASH_OK)
    {
      /* Uh, shouldn't get here...
       * There is already a node or couldn't alloc space
       * for key.  This means bigger problems, but fail
       * gracefully.
       */
      LogMessage("Failed to add file node to hash table\n");
      pthread_mutex_unlock(&file_cache_mutex);
      return -1;
    }
    pthread_mutex_unlock(&file_cache_mutex);
#ifdef REG_TEST
    LogMessage("consume verdict =%d file id =%d \n",hv->verdict,hv->file_type_id);
    file_sha256_print(hv->sha256);
#endif /* REG_TEST */

    return 0;
}
#endif

/*Message will be logged within 600 seconds*/
static ThrottleInfo error_throttleInfo = {0,600,0};

/** *
 * @param sip - source IP address
 * @param dip - destination IP address
 * @param sport - server sport number
 * @param file_sig - file signature
 * @param expiry - session expiry in seconds.
 */
int file_resume_block_add_file(void *pkt, uint32_t file_sig, uint32_t timeout,
        File_Verdict verdict, uint32_t file_type_id, uint8_t *signature, uint16_t cli_port, uint16_t srv_port, bool create_pinhole, bool direction)
{
    FileHashKey hashKey;
    SFXHASH_NODE *hash_node = NULL;
    FileNode *node;
    FileNode new_node;
#ifdef HAVE_DAQ_DP_ADD_DC
    bool use_other_port = false;
#endif
    sfaddr_t* srcIP;
    sfaddr_t* dstIP;
    Packet *p = (Packet*)pkt;
    time_t now = p->pkth->ts.tv_sec;
    SAVE_DAQ_PKT_HDR(p);

    srcIP = GET_SRC_IP(p);
    dstIP = GET_DST_IP(p);
    sfaddr_copy_to_raw(&hashKey.dip, dstIP);
    sfaddr_copy_to_raw(&hashKey.sip, srcIP);
    hashKey.file_sig = file_sig;

    pthread_mutex_lock(&file_cache_mutex);
    hash_node = sfxhash_find_node(fileHash, &hashKey);
    if (hash_node)
    {
        if (!(node = hash_node->data))
        {
            sfxhash_free_node(fileHash, hash_node);
        }
    }
    else
        node = NULL;
    pthread_mutex_unlock(&file_cache_mutex);

    if ( timeout == 0  && verdict == 0 && file_type_id == 0)
    {
        FileConfig *file_config;
        FileContext *context;
        file_config =  (FileConfig *)(snort_conf->file_config);
        context = get_main_file_context(p->ssnptr);
        if (!context && !node)
        {
            FILE_ERROR("Resume block: context and node not found");
            return -1;
        }
        timeout = (uint32_t)file_config->file_block_timeout;
        if(context)
        {
        verdict = context->verdict;
        file_type_id = context->file_type_id;
        signature = context->sha256;
        }
        else
        {
            verdict = node->verdict;
            file_type_id = node->file_type_id;
            signature = node->sha256;
        }
#ifdef HAVE_DAQ_DP_ADD_DC
        use_other_port = true;
#endif
    }
#ifdef HAVE_DAQ_DP_ADD_DC
    if (create_pinhole)
    {
        DAQ_DC_Params params;

        memset(&params, 0, sizeof(params));
        params.flags = DAQ_DC_ALLOW_MULTIPLE;
        params.timeout_ms = 5 * 60 * 1000; /* 5 minutes */
        if (p->packet_flags & PKT_FROM_CLIENT)
        {
            if(use_other_port)
            {
                if(direction)
                    DAQ_Add_Dynamic_Protocol_Channel(p, srcIP, 0, dstIP, srv_port, GET_IPH_PROTO(p), &params);
                else
                    DAQ_Add_Dynamic_Protocol_Channel(p, dstIP, 0, srcIP, srv_port, GET_IPH_PROTO(p), &params);
            }
            else
                DAQ_Add_Dynamic_Protocol_Channel(p, srcIP, 0, dstIP, p->dp, GET_IPH_PROTO(p), &params);
            FILE_DEBUG("Pinhole created from client packet, direction: %d, time: %d",direction, now);
        }
        else if (p->packet_flags & PKT_FROM_SERVER)
        {
            if(use_other_port)
            {
                if(direction)
                    DAQ_Add_Dynamic_Protocol_Channel(p, srcIP, 0, dstIP, srv_port, GET_IPH_PROTO(p), &params);
                else
                    DAQ_Add_Dynamic_Protocol_Channel(p, dstIP, 0, srcIP, srv_port, GET_IPH_PROTO(p), &params);
            }
            else
                DAQ_Add_Dynamic_Protocol_Channel(p, dstIP, 0, srcIP, p->sp, GET_IPH_PROTO(p), &params);
            FILE_DEBUG("Pinhole created from server packet, direction: %d, time: %d",direction, now);
        }
    }
#endif

    if (node)
    {
        FILE_DEBUG("Resume block: Updating file node");
        node->expires = now + (timeout * 4);/* 20 minuts timeout*/
        updateFileNode(node, verdict, file_type_id, signature);
#ifdef SIDE_CHANNEL
        if ((ProduceSSFileCache(&hashKey, node) < 0) && ScSideChannelEnabled())
        {
            ErrorMessageThrottled(&error_throttleInfo, "Failed to add Side channel message\n");
        }
#endif
    }
    else
    {

        FILE_DEBUG("Resume block: Adding file node");

        updateFileNode(&new_node, verdict, file_type_id, signature);

        /*
         * use the time that we keep files around
         * since this info would effectively be invalid
         * after that anyway because the file that
         * caused this will be gone.
         */
        new_node.expires = now + (timeout * 4);/* 20 minuts timeout*/

        /* Add it to the table */
#ifdef SIDE_CHANNEL
        if ((ProduceSSFileCache(&hashKey, &new_node) < 0) && ScSideChannelEnabled())
        {
            ErrorMessageThrottled(&error_throttleInfo, "Failed to add Side channel message\n");
        }
#endif
        pthread_mutex_lock(&file_cache_mutex);
        if (sfxhash_add(fileHash, &hashKey, &new_node) != SFXHASH_OK)
        {
            /* Uh, shouldn't get here...
             * There is already a node or couldn't alloc space
             * for key.  This means bigger problems, but fail
             * gracefully.
             */
            FILE_ERROR("Resume block: Failed to add file node to hash table");
            pthread_mutex_unlock(&file_cache_mutex);
            return -1;
        }
        pthread_mutex_unlock(&file_cache_mutex);
    }
    if (signature)
    {
        FILE_DEBUG("Resume block: Added file node with verdict: %d, file signature: %d, hash:"
                "%02X%02X %02X%02X %02X%02X %02X%02X" 
                "%02X%02X %02X%02X %02X%02X %02X%02X "
                "%02X%02X %02X%02X %02X%02X %02X%02X "
                "%02X%02X %02X%02X %02X%02X %02X%02X",
                verdict, file_sig,
                signature[0], signature[1], signature[2], signature[3],
                signature[4], signature[5], signature[6], signature[7],
                signature[8], signature[9], signature[10], signature[11],
                signature[12], signature[13], signature[14], signature[15],
                signature[16], signature[17], signature[18], signature[19],
                signature[20], signature[21], signature[22], signature[23],
                signature[24], signature[25], signature[26], signature[27],
                signature[28], signature[29], signature[30], signature[31]);
    }
    else
    {
        FILE_DEBUG("Resume block: Added file node with verdict: %d, file signature: %d",
                verdict, file_sig);
    }
    return 0;
}

static inline File_Verdict checkVerdict(Packet *p, FileNode *node, SFXHASH_NODE *hash_node)
{
    File_Verdict verdict = FILE_VERDICT_UNKNOWN;
    FileContext *context = NULL;
    bool partialFile = false;

    /*Query the file policy in case verdict has been changed*/
    /*Check file type first*/
    if (cur_config.file_type_cb)
    {
        verdict = cur_config.file_type_cb(p, p->ssnptr, node->file_type_id, 0,
                DEFAULT_FILE_ID);
        FILE_DEBUG("Checking verdict, node verdict: %d, file type verdict: %d",node->verdict, verdict);
    }

    if ((verdict == FILE_VERDICT_UNKNOWN) ||
            (verdict == FILE_VERDICT_STOP_CAPTURE))
    {
        if (cur_config.file_signature_cb)
        {
            context = get_main_file_context(p->ssnptr);
            if(NULL != context)
            {
                partialFile = context->partial_file;
            }
            verdict = cur_config.file_signature_cb(p, p->ssnptr, node->sha256, 0,
                    &sig_file_state, 0, DEFAULT_FILE_ID, partialFile );
            FILE_DEBUG("Checking verdict, node verdict: %d, file signature verdict: %d",node->verdict, verdict);
        }
    }

    if ((verdict == FILE_VERDICT_UNKNOWN) ||
            (verdict == FILE_VERDICT_STOP_CAPTURE))
    {
        verdict = node->verdict;
    }

    if (verdict == FILE_VERDICT_LOG)
    {
        pthread_mutex_lock(&file_cache_mutex);
        sfxhash_free_node(fileHash, hash_node);
        pthread_mutex_unlock(&file_cache_mutex);
        if (cur_config.log_file_action)
        {
            cur_config.log_file_action(p->ssnptr, FILE_RESUME_LOG);
        }
    }
    else if (verdict == FILE_VERDICT_BLOCK)
    {
        Active_ForceDropPacket();
        Active_DropSession(p);
        if (cur_config.log_file_action)
        {
            cur_config.log_file_action(p->ssnptr, FILE_RESUME_BLOCK);
        }
        node->verdict = verdict;
    }
    else if (verdict == FILE_VERDICT_REJECT)
    {
        Active_ForceDropPacket();
        Active_DropSession(p);
#ifdef ACTIVE_RESPONSE
        Active_QueueReject();
#endif
        if (cur_config.log_file_action)
        {
            cur_config.log_file_action(p->ssnptr, FILE_RESUME_BLOCK);
        }
        node->verdict = verdict;
    }
    else if (verdict == FILE_VERDICT_PENDING)
    {
        /*Take the cached verdict*/
        Active_ForceDropPacket();
        Active_DropSession(p);
#ifdef ACTIVE_RESPONSE
        if (FILE_VERDICT_REJECT == node->verdict)
            Active_QueueReject();
#endif
        if (cur_config.log_file_action)
        {
            cur_config.log_file_action(p->ssnptr, FILE_RESUME_BLOCK);
        }
        verdict = node->verdict;
    }
    FILE_DEBUG("Verdict checked for file node with hash: " 
            "%02X%02X %02X%02X %02X%02X %02X%02X" 
            "%02X%02X %02X%02X %02X%02X %02X%02X "
            "%02X%02X %02X%02X %02X%02X %02X%02X "
            "%02X%02X %02X%02X %02X%02X %02X%02X",
            node->sha256[0], node->sha256[1], node->sha256[2], node->sha256[3],
            node->sha256[4], node->sha256[5], node->sha256[6], node->sha256[7],
            node->sha256[8], node->sha256[9], node->sha256[10], node->sha256[11],
            node->sha256[12], node->sha256[13], node->sha256[14], node->sha256[15],
            node->sha256[16], node->sha256[17], node->sha256[18], node->sha256[19],
            node->sha256[20], node->sha256[21], node->sha256[22], node->sha256[23],
            node->sha256[24], node->sha256[25], node->sha256[26], node->sha256[27],
            node->sha256[28], node->sha256[29], node->sha256[30], node->sha256[31]);

    return verdict;
}

File_Verdict file_resume_block_check(void *pkt, uint32_t file_sig)
{
    File_Verdict verdict = FILE_VERDICT_UNKNOWN;
    sfaddr_t* srcIP;
    sfaddr_t* dstIP;
    SFXHASH_NODE *hash_node;
    FileHashKey hashKey;
    FileNode *node;
    Packet *p = (Packet*)pkt;
    SAVE_DAQ_PKT_HDR(pkt);

    /* No hash table, or its empty?  Get out of dodge.  */
    if (!fileHash || !sfxhash_count(fileHash))
    {
        FILE_DEBUG("No expected sessions");
        return verdict;
    }

    srcIP = GET_SRC_IP(p);
    dstIP = GET_DST_IP(p);
    sfaddr_copy_to_raw(&hashKey.dip, dstIP);
    sfaddr_copy_to_raw(&hashKey.sip, srcIP);
    hashKey.file_sig = file_sig;

    pthread_mutex_lock(&file_cache_mutex);
    hash_node = sfxhash_find_node(fileHash, &hashKey);
  
    if (hash_node)
    {
        if (!(node = hash_node->data))
        {
            sfxhash_free_node(fileHash, hash_node);
        }
    }
    else
    {
        pthread_mutex_unlock(&file_cache_mutex);
        FILE_DEBUG("File not found");
        return verdict;
    }
    pthread_mutex_unlock(&file_cache_mutex);

    if (node)
    {
        FILE_DEBUG("Found resumed file");
        if (node->expires && p->pkth->ts.tv_sec > node->expires)
        {
            FILE_DEBUG("File expired, verdict: %d",verdict);
            pthread_mutex_lock(&file_cache_mutex);
            sfxhash_free_node(fileHash, hash_node);
            pthread_mutex_unlock(&file_cache_mutex);
            return verdict;
        }
        /*Query the file policy in case verdict has been changed*/
        verdict = checkVerdict(p, node, hash_node);
    }
    if (verdict == FILE_VERDICT_BLOCK || verdict == FILE_VERDICT_REJECT || verdict == FILE_VERDICT_PENDING)
    {
        if (pkt_trace_enabled)
            addPktTraceData(VERDICT_REASON_FILE, snprintf(trace_line, MAX_TRACE_LINE,
                "File Process: file not resumed, %s\n", getPktTraceActMsg()));
        else addPktTraceData(VERDICT_REASON_FILE, 0);
        FILE_INFO("File not resumed, verdict: %d",verdict);
    }
    else
        FILE_INFO("File resumed, verdict: %d",verdict);
    return verdict;
}
