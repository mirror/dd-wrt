/*
 ** Copyright (C) 2014-2017 Cisco and/or its affiliates. All rights reserved.
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

/* The hash table of expected files */
static SFXHASH *fileHash = NULL;
extern FileServiceConfig cur_config;
extern FileContext* get_main_file_context(void *ssnptr);

static FileState sig_file_state = { FILE_CAPTURE_SUCCESS, FILE_SIG_DONE };

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
}

void file_resume_block_cleanup(void)
{
    if (fileHash)
    {
        sfxhash_delete(fileHash);
        fileHash = NULL;
    }
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

/** *
 * @param sip - source IP address
 * @param dip - destination IP address
 * @param sport - server sport number
 * @param file_sig - file signature
 * @param expiry - session expiry in seconds.
 */
int file_resume_block_add_file(void *pkt, uint32_t file_sig, uint32_t timeout,
        File_Verdict verdict, uint32_t file_type_id, uint8_t *signature, uint16_t cli_port, uint16_t srv_port, bool create_pinhole)
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

    srcIP = GET_SRC_IP(p);
    dstIP = GET_DST_IP(p);
    sfaddr_copy_to_raw(&hashKey.dip, dstIP);
    sfaddr_copy_to_raw(&hashKey.sip, srcIP);
    hashKey.file_sig = file_sig;
    if ( timeout == 0  && verdict == 0 && file_type_id == 0)
    {
        FileConfig *file_config;
        FileContext *context;
        file_config =  (FileConfig *)(snort_conf->file_config);
        context = get_main_file_context(p->ssnptr);
        if (!context)
            return -1;
        timeout = (uint32_t)file_config->file_block_timeout;
        verdict = context->verdict;
        file_type_id = context->file_type_id;
        signature = context->sha256;
#ifdef HAVE_DAQ_DP_ADD_DC
        use_other_port = true;
#endif
    }
#ifdef HAVE_DAQ_DP_ADD_DC
    if (create_pinhole)
    {
        DAQ_DC_Params params;

        memset(&params, 0, sizeof(params));
        params.flags = DAQ_DC_ALLOW_MULTIPLE | DAQ_DC_PERSIST;
        params.timeout_ms = 5 * 60 * 1000; /* 5 minutes */
        if (p->packet_flags & PKT_FROM_CLIENT)
            DAQ_Add_Dynamic_Protocol_Channel(p, srcIP, 0, dstIP, use_other_port ? srv_port : p->dp, GET_IPH_PROTO(p),
                &params);
        else if (p->packet_flags & PKT_FROM_SERVER)
             DAQ_Add_Dynamic_Protocol_Channel(p, dstIP, 0, srcIP, use_other_port ? cli_port : p->sp, GET_IPH_PROTO(p),
                   &params);
    }
#endif

    hash_node = sfxhash_find_node(fileHash, &hashKey);
    if (hash_node)
    {
        if (!(node = hash_node->data))
            sfxhash_free_node(fileHash, hash_node);
    }
    else
        node = NULL;
    if (node)
    {
        node->expires = now + timeout;
        updateFileNode(node, verdict, file_type_id, signature);
    }
    else
    {

        DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Adding file node\n"););

        updateFileNode(&new_node, verdict, file_type_id, signature);

        /*
         * use the time that we keep files around
         * since this info would effectively be invalid
         * after that anyway because the file that
         * caused this will be gone.
         */
        new_node.expires = now + timeout;

        /* Add it to the table */
        if (sfxhash_add(fileHash, &hashKey, &new_node) != SFXHASH_OK)
        {
            /* Uh, shouldn't get here...
             * There is already a node or couldn't alloc space
             * for key.  This means bigger problems, but fail
             * gracefully.
             */
            DEBUG_WRAP(DebugMessage(DEBUG_FILE,
                    "Failed to add file node to hash table\n"););
            return -1;
        }
    }
    return 0;
}

static inline File_Verdict checkVerdict(Packet *p, FileNode *node, SFXHASH_NODE *hash_node)
{
    File_Verdict verdict = FILE_VERDICT_UNKNOWN;

    /*Query the file policy in case verdict has been changed*/
    /*Check file type first*/
    if (cur_config.file_type_cb)
    {
        verdict = cur_config.file_type_cb(p, p->ssnptr, node->file_type_id, 0,
                DEFAULT_FILE_ID);
    }

    if ((verdict == FILE_VERDICT_UNKNOWN) ||
            (verdict == FILE_VERDICT_STOP_CAPTURE))
    {
        if (cur_config.file_signature_cb)
        {
            verdict = cur_config.file_signature_cb(p, p->ssnptr, node->sha256, 0,
                    &sig_file_state, 0, DEFAULT_FILE_ID);
        }
    }

    if ((verdict == FILE_VERDICT_UNKNOWN) ||
            (verdict == FILE_VERDICT_STOP_CAPTURE))
    {
        verdict = node->verdict;
    }

    if (verdict == FILE_VERDICT_LOG)
    {
        sfxhash_free_node(fileHash, hash_node);
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

    /* No hash table, or its empty?  Get out of dodge.  */
    if (!fileHash || !sfxhash_count(fileHash))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE, "No expected sessions\n"););
        return verdict;
    }

    srcIP = GET_SRC_IP(p);
    dstIP = GET_DST_IP(p);
    sfaddr_copy_to_raw(&hashKey.dip, dstIP);
    sfaddr_copy_to_raw(&hashKey.sip, srcIP);
    hashKey.file_sig = file_sig;

    hash_node = sfxhash_find_node(fileHash, &hashKey);

    if (hash_node)
    {
        if (!(node = hash_node->data))
            sfxhash_free_node(fileHash, hash_node);
    }
    else
        return verdict;

    if (node)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Found resumed file\n"););
        if (node->expires && p->pkth->ts.tv_sec > node->expires)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "File expired\n"););
            sfxhash_free_node(fileHash, hash_node);
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
    }
    return verdict;
}
