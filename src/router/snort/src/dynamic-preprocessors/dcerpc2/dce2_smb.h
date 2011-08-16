/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include "debug.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_MOCK_HDR_LEN__SMB_CLI  (sizeof(NbssHdr) + sizeof(SmbNtHdr) + sizeof(SmbLm10_WriteAndXReq))
#define DCE2_MOCK_HDR_LEN__SMB_SRV  (sizeof(NbssHdr) + sizeof(SmbNtHdr) + sizeof(SmbLm10_ReadAndXResp))

#define DCE2_SMB_ID   0xff534d42  /* \xffSMB */
#define DCE2_SMB2_ID  0xfe534d42  /* \xfeSMB */

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_SmbFidNode
{
    int uid;
    int tid;
    int fid;

} DCE2_SmbFidNode;

typedef struct _DCE2_SmbFidTrackerNode
{
    int used;   /* For Win2000 */
    DCE2_SmbFidNode fid_node;
    DCE2_CoTracker co_tracker;

} DCE2_SmbFidTrackerNode;

typedef struct _DCE2_SmbPMNode
{
    DCE2_Policy policy;
    int pid;
    int mid;
    uint16_t total_dcnt;
    DCE2_SmbFidNode fid_node;
    DCE2_Buffer *buf;

} DCE2_SmbPMNode;

typedef struct _DCE2_SmbUTNode
{
    int uid;
    int tid;
    DCE2_SmbFidTrackerNode ft_node;
    DCE2_List *fts;

} DCE2_SmbUTNode;

typedef struct _DCE2_SmbPipeTree
{
    DCE2_SmbUTNode ut_node;
    DCE2_List *uts;

} DCE2_SmbPipeTree;

typedef struct _DCE2_SmbSessionTracker
{
    DCE2_SmbPipeTree ptree;

    /* Uids created on session */
    int uid;
    DCE2_List *uids;

    /* IPC tids created on session */
    int tid;
    DCE2_List *tids;

    /* Co trackers for fids created on session using 
     * only IPC tids - specific for Samba and Win2000 */
    DCE2_SmbFidTrackerNode ft_node;
    DCE2_List *fts;

} DCE2_SmbSessionTracker;

/* For Block Raw requests */
typedef struct _DCE2_SmbBlockRaw
{
    int smb_com;
    uint16_t total_count;
    DCE2_SmbFidNode fid_node;

} DCE2_SmbBlockRaw;

typedef struct _DCE2_SmbSeg
{
    DCE2_Buffer *buf;
    uint32_t nb_len;

} DCE2_SmbSeg;

typedef struct _DCE2_SmbSsnData
{
    DCE2_SsnData sd;

    /* For SMB session tracking */
    DCE2_SmbSessionTracker sst;

    /* Client can send multiple tree connects before server responses.
     * Since for a Tree Connect we rely on the client to determine if
     * the tree will be IPC$ upon acceptance by server, we need to
     * queue them up */ 
    DCE2_CQueue *tc_queue;

    DCE2_SmbBlockRaw br;

    /* Client can send multiple writes and reads before server responses
     * so we need to queue them up to associate response with request
     * since server response does not contain the fid */
    DCE2_SmbFidNode read_fid_node;
    DCE2_CQueue *read_fid_queue;

    DCE2_SmbPMNode pm_node;
    DCE2_List *pms;

    /* Need this for potential chaining weirdness, such as
     * OpenAndX -> SessionSetupAndX or OpenAndX -> TreeConnectAndX */
    int req_uid;
    int req_tid;

    /* Client can chain a write to an open.  Need to write data, but also
     * need to associate tracker with fid returned from server */
    DCE2_Queue *ft_queue;

    DCE2_SmbSeg cli_seg;
    DCE2_SmbSeg srv_seg;

    uint32_t cli_ignore_bytes;
    uint32_t srv_ignore_bytes;

    /* Current values for reassembly */
    uint16_t uid;
    uint16_t tid;
    uint16_t fid;

    int chained_tc;     /* Set if client and chained TreeConnect */
    int last_open_fid;  /* The last inserted fid from an OpenAndX or NtCreateAndX */

    /* Boolean for whether or not packets have been currently been missed */
    char missed_pkts;

} DCE2_SmbSsnData;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static INLINE DCE2_TransType DCE2_SmbAutodetect(const SFSnortPacket *);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_SmbInitRdata(uint8_t *, int);
void DCE2_SmbSetRdata(DCE2_SmbSsnData *, uint8_t *, uint16_t);
DCE2_SmbSsnData * DCE2_SmbSsnInit(void);
void DCE2_SmbProcess(DCE2_SmbSsnData *);
void DCE2_SmbDataFree(DCE2_SmbSsnData *);
void DCE2_SmbSsnFree(void *);

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
static INLINE DCE2_TransType DCE2_SmbAutodetect(const SFSnortPacket *p)
{
    if (p->payload_size >= sizeof(NbssHdr))
    {
        NbssHdr *nb_hdr = (NbssHdr *)p->payload;

        switch (NbssType(nb_hdr))
        {
            case NBSS_SESSION_TYPE__MESSAGE:
                {
                    SmbNtHdr *smb_hdr = (SmbNtHdr *)(p->payload + sizeof(NbssHdr));

                    if (p->payload_size > (sizeof(NbssHdr) + sizeof(SmbNtHdr)))
                    {
                        if (SmbId(smb_hdr) == DCE2_SMB_ID)
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

#endif  /* _DCE2_SMB_H_ */

