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

#ifndef _DCE2_SMB_H_
#define _DCE2_SMB_H_

#include "dce2_session.h"
#include "dce2_tcp.h"
#include "dce2_list.h"
#include "dce2_utils.h"
#include "smb.h"
#include "sf_snort_packet.h"
#include "sf_types.h"
#include "snort_debug.h"

/********************************************************************
 * Macros
 ********************************************************************/
// Used for reassembled packets
#define DCE2_MOCK_HDR_LEN__SMB_CLI \
    (sizeof(NbssHdr) + sizeof(SmbNtHdr) + sizeof(SmbWriteAndXReq))
#define DCE2_MOCK_HDR_LEN__SMB_SRV \
    (sizeof(NbssHdr) + sizeof(SmbNtHdr) + sizeof(SmbReadAndXResp))

// This is for ease of comparison so a 32 bit numeric compare can be done
// instead of a string compare.
#define DCE2_SMB_ID   0xff534d42  /* \xffSMB */
#define DCE2_SMB2_ID  0xfe534d42  /* \xfeSMB */
#define DCE2_SMB_ID_SIZE    4

// MS-FSCC Section 2.1.5 - Pathname
#define DCE2_SMB_MAX_PATH_LEN  32760
#define DCE2_SMB_MAX_COMP_LEN    255

/********************************************************************
 * Externs
 ********************************************************************/
extern SmbAndXCom smb_chain_map[SMB_MAX_NUM_COMS];
extern const char *smb_com_strings[SMB_MAX_NUM_COMS];
extern const char *smb_transaction_sub_command_strings[TRANS_SUBCOM_MAX];
extern const char *smb_transaction2_sub_command_strings[TRANS2_SUBCOM_MAX];
extern const char *smb_nt_transact_sub_command_strings[NT_TRANSACT_SUBCOM_MAX];
extern uint8_t smb_file_name[2*DCE2_SMB_MAX_PATH_LEN + UTF_16_LE_BOM_LEN + 2];
extern uint16_t smb_file_name_len;

/********************************************************************
 * Enums
 ********************************************************************/
typedef enum _DCE2_SmbSsnState
{
    DCE2_SMB_SSN_STATE__START         = 0x00,
    DCE2_SMB_SSN_STATE__NEGOTIATED    = 0x01,
    DCE2_SMB_SSN_STATE__FP_CLIENT     = 0x02,  // Fingerprinted client
    DCE2_SMB_SSN_STATE__FP_SERVER     = 0x04   // Fingerprinted server

} DCE2_SmbSsnState;

typedef enum _DCE2_SmbDataState
{
    DCE2_SMB_DATA_STATE__NETBIOS_HEADER,
    DCE2_SMB_DATA_STATE__SMB_HEADER,
    DCE2_SMB_DATA_STATE__NETBIOS_PDU

} DCE2_SmbDataState;

typedef enum _DCE2_SmbPduState
{
    DCE2_SMB_PDU_STATE__COMMAND,
    DCE2_SMB_PDU_STATE__RAW_DATA

} DCE2_SmbPduState;

typedef enum _DCE2_SmbFileDirection
{
    DCE2_SMB_FILE_DIRECTION__UNKNOWN = 0,
    DCE2_SMB_FILE_DIRECTION__UPLOAD,
    DCE2_SMB_FILE_DIRECTION__DOWNLOAD

} DCE2_SmbFileDirection;

/* This structure is to maintain that we have received a pending veridct in case of upload & we will not delete the trackers*/
typedef enum _DCE2_SmbRetransmitPending
{
   DCE2_SMB_RETRANSMIT_PENDING__UNSET = 0,
   DCE2_SMB_RETRANSMIT_PENDING__SET

} DCE2_SmbRetransmitPending;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_SmbWriteAndXRaw
{
    int remaining;  // A signed integer so it can be negative
    DCE2_Buffer *buf;

} DCE2_SmbWriteAndXRaw;

typedef struct _DCE2_SmbFileChunk
{
    uint64_t offset;
    uint32_t length;
    uint8_t *data;

} DCE2_SmbFileChunk;

typedef struct _DCE2_SmbFileTracker
{
    union
    {
        struct
        {
            int file_id;   // A signed integer so it can be set to sentinel
            uint16_t u_id;
            uint16_t tree_id;
        } id_smb1;

        struct
        {
            uint64_t file_id;
        } id_smb2;

    } file_key;

    bool is_ipc;
    bool is_smb2;
    uint16_t file_name_len;
    uint8_t *file_name;
    union
    {
        struct
        {
            // If pipe has been set to byte mode via TRANS_SET_NMPIPE_STATE
            bool byte_mode;

            // For Windows 2000
            bool used;

            // For WriteAndX requests that use raw mode flag
            // Windows only
            DCE2_SmbWriteAndXRaw *writex_raw;

            // Connection-oriented DCE/RPC tracker
            DCE2_CoTracker *co_tracker;

        } nmpipe;

        struct
        {
            uint64_t file_size;
            uint64_t file_offset;
            uint64_t bytes_processed;
            DCE2_List *file_chunks;
            uint32_t bytes_queued;
            DCE2_SmbFileDirection file_direction;
            bool sequential_only;

        } file;

    } tracker;

#define fid_v1                file_key.id_smb1.file_id
#define uid_v1                file_key.id_smb1.u_id
#define tid_v1                file_key.id_smb1.tree_id
#define fid_v2                file_key.id_smb2.file_id
#define fp_byte_mode          tracker.nmpipe.byte_mode
#define fp_used               tracker.nmpipe.used
#define fp_writex_raw         tracker.nmpipe.writex_raw
#define fp_co_tracker         tracker.nmpipe.co_tracker
#define ff_file_size          tracker.file.file_size
#define ff_file_offset        tracker.file.file_offset
#define ff_bytes_processed    tracker.file.bytes_processed
#define ff_file_direction     tracker.file.file_direction
#define ff_file_chunks        tracker.file.file_chunks
#define ff_bytes_queued       tracker.file.bytes_queued
#define ff_sequential_only    tracker.file.sequential_only

} DCE2_SmbFileTracker;

typedef enum _DCE2_SmbVersion
{
    DCE2_SMB_VERISON_NULL,
    DCE2_SMB_VERISON_1,
    DCE2_SMB_VERISON_2
} DCE2_SmbVersion;


typedef struct _Smb2Request
{
    uint64_t message_id;   /* identifies a message uniquely on connection */
    uint16_t command;
    union {
        struct {
            uint64_t offset;       /* data offset */
            uint64_t file_id;      /* file id */
        }read_req;
        struct {
            char *file_name;        /*file name*/
            uint16_t file_name_len; /*size*/
            bool durable_reconnect; /*durable reconenct? */
        }create_req;
    };
    struct _Smb2Request *next;
    struct _Smb2Request *previous;
} Smb2Request;

typedef struct _DCE2_SmbTransactionTracker
{
    int smb_type;
    uint8_t subcom;
    bool one_way;
    bool disconnect_tid;
    bool pipe_byte_mode;
    uint32_t tdcnt;
    uint32_t dsent;
    DCE2_Buffer *dbuf;
    uint32_t tpcnt;
    uint32_t psent;
    DCE2_Buffer *pbuf;
    // For Transaction2/Query File Information
    uint16_t info_level;

} DCE2_SmbTransactionTracker;

typedef struct _DCE2_SmbRequestTracker
{
    int smb_com;

    int mid;   // A signed integer so it can be set to sentinel
    uint16_t uid;
    uint16_t tid;
    uint16_t pid;

    // For WriteRaw
    bool writeraw_writethrough;
    uint32_t writeraw_remaining;
    uint16_t file_name_len;

    // For Transaction/Transaction2/NtTransact
    DCE2_SmbTransactionTracker ttracker;

    // Client can chain a write to an open.  Need to write data, but also
    // need to associate tracker with fid returned from server
    DCE2_Queue *ft_queue;

    // This is a reference to an existing file tracker
    DCE2_SmbFileTracker *ftracker;

    // Used for requests to cache data that will ultimately end up in
    // the file tracker upon response.
    uint8_t *file_name;
    uint64_t file_size;
    uint64_t file_offset;
    bool sequential_only;

    // For TreeConnect to know whether it's to IPC
    bool is_ipc;

} DCE2_SmbRequestTracker;

typedef struct _DCE2_SmbSsnData
{
    DCE2_SsnData sd;  // This member must be first

    DCE2_Policy policy;

    int dialect_index;
    int ssn_state_flags;

    DCE2_SmbDataState cli_data_state;
    DCE2_SmbDataState srv_data_state;

    DCE2_SmbPduState pdu_state;

    int uid;   // A signed integer so it can be set to sentinel
    int tid;   // A signed integer so it can be set to sentinel
    DCE2_List *uids;
    DCE2_List *tids;

    // For tracking files and named pipes
    DCE2_SmbFileTracker ftracker;
    DCE2_List *ftrackers;  // List of DCE2_SmbFileTracker

    // For tracking requests / responses
    DCE2_SmbRequestTracker rtracker;
    DCE2_Queue *rtrackers;

    // The current pid/mid node for this request/response
    DCE2_SmbRequestTracker *cur_rtracker;

    // Used for TCP segmentation to get full PDU
    DCE2_Buffer *cli_seg;
    DCE2_Buffer *srv_seg;

    // These are used for commands we don't need to process
    uint32_t cli_ignore_bytes;
    uint32_t srv_ignore_bytes;

    // The file API supports one concurrent upload/download per session.
    // This is a reference to a file tracker so shouldn't be freed.
    DCE2_SmbFileTracker *fapi_ftracker;

    Smb2Request *smb2_requests;

#ifdef ACTIVE_RESPONSE
    DCE2_SmbFileTracker *fb_ftracker;
    bool block_pdus;
#endif

    bool smbfound;
    bool smbretransmit;
    uint16_t max_outstanding_requests;
    uint16_t outstanding_requests;
    // Maximum file depth as returned from file API
    int64_t max_file_depth;

} DCE2_SmbSsnData;

typedef struct _DCE2SmbFsm
{
    char input;
    int next_state;
    int fail_state;

} DCE2_SmbFsm;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static inline DCE2_TransType DCE2_SmbAutodetect(const SFSnortPacket *);
static inline void DCE2_SmbSetFingerprintedClient(DCE2_SmbSsnData *);
static inline bool DCE2_SmbFingerprintedClient(DCE2_SmbSsnData *);
static inline void DCE2_SmbSetFingerprintedServer(DCE2_SmbSsnData *);
static inline bool DCE2_SmbFingerprintedServer(DCE2_SmbSsnData *);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_SmbInitGlobals(void);
void DCE2_SmbInitRdata(uint8_t *, int);
void DCE2_SmbSetRdata(DCE2_SmbSsnData *, uint8_t *, uint16_t);
DCE2_SmbSsnData * DCE2_SmbSsnInit(SFSnortPacket *);
void DCE2_SmbProcess(DCE2_SmbSsnData *);
void DCE2_SmbDataFree(DCE2_SmbSsnData *);
void DCE2_SmbSsnFree(void *);
#ifdef ACTIVE_RESPONSE
void DCE2_SmbInitDeletePdu(void);
#endif
void DCE2_Process_Retransmitted(SFSnortPacket *);
/*********************************************************************
 * Function: DCE2_SmbAutodetect()
 *
 * Purpose: Tries to determine if a packet is likely to be SMB.
 *
 * Arguments:
 *  const uint8_t * - pointer to packet data.
 *  uint16_t - packet data length.
 *
 * Returns:
 *  DCE2_TranType
 *
 *********************************************************************/
static inline DCE2_TransType DCE2_SmbAutodetect(const SFSnortPacket *p)
{
    if (p->payload_size > (sizeof(NbssHdr) + sizeof(SmbNtHdr)))
    {
        NbssHdr *nb_hdr = (NbssHdr *)p->payload;

        switch (NbssType(nb_hdr))
        {
            case NBSS_SESSION_TYPE__MESSAGE:
                {
                    SmbNtHdr *smb_hdr = (SmbNtHdr *)(p->payload + sizeof(NbssHdr));

                    if ((SmbId(smb_hdr) == DCE2_SMB_ID)
                            || (SmbId(smb_hdr) == DCE2_SMB2_ID))
                    {
                        return DCE2_TRANS_TYPE__SMB;
                    }
                }

                break;

            default:
                break;

        }
    }

    return DCE2_TRANS_TYPE__NONE;
}

static inline void DCE2_SmbSetFingerprintedClient(DCE2_SmbSsnData *ssd)
{
    ssd->ssn_state_flags |= DCE2_SMB_SSN_STATE__FP_CLIENT;
}

static inline bool DCE2_SmbFingerprintedClient(DCE2_SmbSsnData *ssd)
{
    return ssd->ssn_state_flags & DCE2_SMB_SSN_STATE__FP_CLIENT;
}

static inline void DCE2_SmbSetFingerprintedServer(DCE2_SmbSsnData *ssd)
{
    ssd->ssn_state_flags |= DCE2_SMB_SSN_STATE__FP_SERVER;
}

static inline bool DCE2_SmbFingerprintedServer(DCE2_SmbSsnData *ssd)
{
    return ssd->ssn_state_flags & DCE2_SMB_SSN_STATE__FP_SERVER;
}

static inline bool DCE2_SmbFileDirUnknown(DCE2_SmbFileDirection dir)
{
    return dir == DCE2_SMB_FILE_DIRECTION__UNKNOWN;
}

static inline bool DCE2_SmbFileUpload(DCE2_SmbFileDirection dir)
{
    return dir == DCE2_SMB_FILE_DIRECTION__UPLOAD;
}

static inline bool DCE2_SmbFileDownload(DCE2_SmbFileDirection dir)
{
    return dir == DCE2_SMB_FILE_DIRECTION__DOWNLOAD;
}

#endif  /* _DCE2_SMB_H_ */

