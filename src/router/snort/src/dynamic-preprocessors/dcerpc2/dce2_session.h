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

#ifndef _DCE2_SESSION_H_
#define _DCE2_SESSION_H_

#include "dce2_utils.h"
#include "dce2_config.h"
#include "dce2_memory.h"
#include "dce2_roptions.h"
#include "dcerpc.h"
#include "sf_snort_packet.h"
#include "stream_api.h"
#include "sf_dynamic_preprocessor.h"

/********************************************************************
 * Extern variables
 ********************************************************************/
extern DynamicPreprocessorData _dpd;

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_SsnFlag
{
    DCE2_SSN_FLAG__NONE               = 0x0000,
    DCE2_SSN_FLAG__SEEN_CLIENT        = 0x0001,
    DCE2_SSN_FLAG__SEEN_SERVER        = 0x0002,
    DCE2_SSN_FLAG__MISSED_PKTS        = 0x0004,
    DCE2_SSN_FLAG__AUTODETECTED       = 0x0008,
    DCE2_SSN_FLAG__NO_INSPECT         = 0x0010,
    DCE2_SSN_FLAG__ALL                = 0xffff

} DCE2_SsnFlag;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_SsnData
{
    DCE2_TransType trans;
    int flags;
    const DCE2_ServerConfig *sconfig;
    const SFSnortPacket *wire_pkt;
    int alert_mask;
    DCE2_Roptions ropts;
    int autodetect_dir;

    uint32_t cli_seq;
    uint32_t cli_nseq;
    uint32_t cli_missed_bytes;
    uint16_t cli_overlap_bytes;
    uint32_t srv_seq;
    uint32_t srv_nseq;
    uint32_t srv_missed_bytes;
    uint16_t srv_overlap_bytes;

    tSfPolicyId policy_id;
    tSfPolicyUserContextId config;

} DCE2_SsnData;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static INLINE int DCE2_SsnIsEstablished(const SFSnortPacket *);
static INLINE int DCE2_SsnIsMidstream(const SFSnortPacket *);
static INLINE void DCE2_SsnSetAppData(const SFSnortPacket *, void *, StreamAppDataFree);
static INLINE void * DCE2_SsnGetAppData(const SFSnortPacket *);
static INLINE int DCE2_SsnGetReassembly(const SFSnortPacket *);
static INLINE void DCE2_SsnSetReassembly(const SFSnortPacket *);
static INLINE int DCE2_SsnIsRebuilt(const SFSnortPacket *);
static INLINE int DCE2_SsnIsStreamInsert(const SFSnortPacket *);
static INLINE void DCE2_SsnFlush(SFSnortPacket *);
static INLINE int DCE2_SsnFromServer(const SFSnortPacket *);
static INLINE int DCE2_SsnFromClient(const SFSnortPacket *);
static INLINE int DCE2_SsnClientMissedInReassembled(const SFSnortPacket *);
static INLINE int DCE2_SsnServerMissedInReassembled(const SFSnortPacket *);
static INLINE void DCE2_SsnSetMissedPkts(DCE2_SsnData *);
static INLINE int DCE2_SsnMissedPkts(DCE2_SsnData *);
static INLINE void DCE2_SsnClearMissedPkts(DCE2_SsnData *);
static INLINE void DCE2_SsnSetSeenClient(DCE2_SsnData *);
static INLINE int DCE2_SsnSeenClient(DCE2_SsnData *);
static INLINE void DCE2_SsnSetSeenServer(DCE2_SsnData *);
static INLINE int DCE2_SsnSeenServer(DCE2_SsnData *);
static INLINE void DCE2_SsnSetAutodetected(DCE2_SsnData *, const SFSnortPacket *);
static INLINE int DCE2_SsnAutodetected(DCE2_SsnData *);
static INLINE int DCE2_SsnAutodetectDir(DCE2_SsnData *);
static INLINE void DCE2_SsnSetNoInspect(DCE2_SsnData *);
static INLINE int DCE2_SsnNoInspect(DCE2_SsnData *sd);

static INLINE uint16_t DCE2_SsnGetOverlap(DCE2_SsnData *);
static INLINE uint32_t DCE2_SsnGetMissedBytes(DCE2_SsnData *sd);

/********************************************************************
 * Function: DCE2_SsnIsEstablished()
 *
 * Purpose: Returns whether or not the session is established
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - non-zero if the session is established.
 *        zero if the session is not established.
 *
 ********************************************************************/
static INLINE int DCE2_SsnIsEstablished(const SFSnortPacket *p)
{
    return _dpd.streamAPI->get_session_flags
        (p->stream_session_ptr) & SSNFLAG_ESTABLISHED;
}

/********************************************************************
 * Function: DCE2_SsnIsEstablished()
 *
 * Purpose: Returns whether or not the session was picked
 *          up midstream.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - non-zero if the session was picked up midstream.
 *        zero if the session was not picked up midstream.
 *
 ********************************************************************/
static INLINE int DCE2_SsnIsMidstream(const SFSnortPacket *p)
{
    return _dpd.streamAPI->get_session_flags
        (p->stream_session_ptr) & SSNFLAG_MIDSTREAM;
}

/********************************************************************
 * Function: DCE2_SsnSetAppData()
 *
 * Purpose: Sets application data associated with session.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *  void * - pointer to data to store on session.
 *  StreamAppDataFree - free function for freeing data stored
 *                      on session
 *
 *  Note: Both data and free function can be NULL and have the
 *        effect of removing the session data.
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnSetAppData(const SFSnortPacket *p, void *data, StreamAppDataFree sdfree)
{
    _dpd.streamAPI->set_application_data(p->stream_session_ptr, PP_DCE2, data, sdfree);
}

/********************************************************************
 * Function: DCE2_SsnGetAppData()
 *
 * Purpose: Gets application data stored with session.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  void * - the data stored on the session.
 *
 ********************************************************************/
static INLINE void * DCE2_SsnGetAppData(const SFSnortPacket *p)
{
    return _dpd.streamAPI->get_application_data(p->stream_session_ptr, PP_DCE2);
}

/********************************************************************
 * Function: DCE2_SsnGetReassembly()
 *
 * Purpose: Gets reassembly direction for the session.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - the reassembly direction
 *        SSN_DIR_NONE, SSN_DIR_CLIENT, SSN_DIR_SERVER or SSN_DIR_BOTH
 *
 ********************************************************************/
static INLINE int DCE2_SsnGetReassembly(const SFSnortPacket *p)
{
    return (int)_dpd.streamAPI->get_reassembly_direction(p->stream_session_ptr);
}

/********************************************************************
 * Function: DCE2_SsnSetReassembly()
 *
 * Purpose: Sets reassembly direction for the session to
 *          SSN_DIR_BOTH since the preprocessor looks at both
 *          client and server packets.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnSetReassembly(const SFSnortPacket *p)
{
    _dpd.streamAPI->set_reassembly(p->stream_session_ptr, STREAM_FLPOLICY_FOOTPRINT,
                                   SSN_DIR_BOTH, STREAM_FLPOLICY_SET_ABSOLUTE);
}

/********************************************************************
 * Function: DCE2_SsnIsRebuilt()
 *
 * Purpose: Returns whether or not the packet is a stream
 *          reassembled packet.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - non-zero if the packet is stream reassembled.
 *        zero if the packet is not stream reassembled.
 *
 ********************************************************************/
static INLINE int DCE2_SsnIsRebuilt(const SFSnortPacket *p)
{
    return p->flags & FLAG_REBUILT_STREAM;
}

/********************************************************************
 * Function: DCE2_SsnIsStreamInsert()
 *
 * Purpose: Returns whether or not the packet is a stream
 *          inserted packet.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - non-zero if the packet is stream inserted.
 *        zero if the packet is not stream inserted.
 *
 ********************************************************************/
static INLINE int DCE2_SsnIsStreamInsert(const SFSnortPacket *p)
{
    return p->flags & FLAG_STREAM_INSERT;
}

/********************************************************************
 * Function: DCE2_SsnFlush()
 *
 * Purpose: Flushes the stream inserted packets on the opposite
 *          side of the conversation.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnFlush(SFSnortPacket *p)
{
    _dpd.streamAPI->response_flush_stream(p);
}

/********************************************************************
 * Function: DCE2_SsnFromServer()
 *
 * Purpose: Returns whether or not this packet is from
 *          the server. 
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - non-zero if the packet is from the server.
 *        zero if the packet is not from the server.
 *
 ********************************************************************/
static INLINE int DCE2_SsnFromServer(const SFSnortPacket *p)
{
    return p->flags & FLAG_FROM_SERVER;
}

/********************************************************************
 * Function: DCE2_SsnFromClient()
 *
 * Purpose: Returns whether or not this packet is from
 *          the client. 
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - non-zero if the packet is from the client.
 *        zero if the packet is not from the client.
 *
 ********************************************************************/
static INLINE int DCE2_SsnFromClient(const SFSnortPacket *p)
{
    return p->flags & FLAG_FROM_CLIENT;
}

/********************************************************************
 * Function: DCE2_SsnClientMissedInReassembled()
 *
 * Purpose: Returns if and how we missed packets from the client
 *          on the session, as determined by stream reassembly.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - missing packets before, after, before and after or none
 *        SSN_MISSING_BEFORE, SSN_MISSING_AFTER,
 *        SSN_MISSING_BOTH or SSN_MISSING_NONE
 *
 ********************************************************************/
static INLINE int DCE2_SsnClientMissedInReassembled(const SFSnortPacket *p)
{
    return _dpd.streamAPI->missing_in_reassembled(p->stream_session_ptr, SSN_DIR_CLIENT);
}

/********************************************************************
 * Function: DCE2_SsnServerMissedInReassembled()
 *
 * Purpose: Returns if and how we missed packets from the server
 *          on the session, as determined by stream reassembly.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int - missing packets before, after, before and after or none
 *        SSN_MISSING_BEFORE, SSN_MISSING_AFTER,
 *        SSN_MISSING_BOTH or SSN_MISSING_NONE
 *
 ********************************************************************/
static INLINE int DCE2_SsnServerMissedInReassembled(const SFSnortPacket *p)
{
    return _dpd.streamAPI->missing_in_reassembled(p->stream_session_ptr, SSN_DIR_SERVER);
}

/********************************************************************
 * Function: DCE2_SsnSetMissedPkts()
 *
 * Purpose: Sets flag that we have missed packets on this session
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnSetMissedPkts(DCE2_SsnData *sd)
{
    sd->flags |= DCE2_SSN_FLAG__MISSED_PKTS;
}

/********************************************************************
 * Function: DCE2_SsnMissedPkts()
 *
 * Purpose: Returns whether or not we've missed packets
 *          on this session.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  int - non-zero if packets were missed
 *        zero if no packets were missed
 *
 ********************************************************************/
static INLINE int DCE2_SsnMissedPkts(DCE2_SsnData *sd)
{
    return sd->flags & DCE2_SSN_FLAG__MISSED_PKTS;
}

/********************************************************************
 * Function: DCE2_SsnClearMissedPkts()
 *
 * Purpose: Clears the flag that indicates that we've missed
 *          packets on the session.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnClearMissedPkts(DCE2_SsnData *sd)
{
    sd->flags &= ~DCE2_SSN_FLAG__MISSED_PKTS;
}

/********************************************************************
 * Function: DCE2_SsnSetSeenClient()
 *
 * Purpose: Sets a flag that indicates that we have seen the
 *          client side of the conversation.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnSetSeenClient(DCE2_SsnData *sd)
{
    sd->flags |= DCE2_SSN_FLAG__SEEN_CLIENT;
}

/********************************************************************
 * Function: DCE2_SsnSeenClient()
 *
 * Purpose: Returns whether or not we've seen the client side
 *          of the conversation on this session.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  int - non-zero if we've seen the client
 *        zero if we haven't seen the client
 *
 ********************************************************************/
static INLINE int DCE2_SsnSeenClient(DCE2_SsnData *sd)
{
    return sd->flags & DCE2_SSN_FLAG__SEEN_CLIENT;
}

/********************************************************************
 * Function: DCE2_SsnSetSeenServer()
 *
 * Purpose: Sets a flag that indicates that we have seen the
 *          server side of the conversation.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnSetSeenServer(DCE2_SsnData *sd)
{
    sd->flags |= DCE2_SSN_FLAG__SEEN_SERVER;
}

/********************************************************************
 * Function: DCE2_SsnSeenServer()
 *
 * Purpose: Returns whether or not we've seen the server side
 *          of the conversation on this session.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  int - non-zero if we've seen the server
 *        zero if we haven't seen the server
 *
 ********************************************************************/
static INLINE int DCE2_SsnSeenServer(DCE2_SsnData *sd)
{
    return sd->flags & DCE2_SSN_FLAG__SEEN_SERVER;
}

/********************************************************************
 * Function: DCE2_SsnSetAutodetected()
 *
 * Purpose: Sets flag that indicates that this session
 *          was autodetected.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnSetAutodetected(DCE2_SsnData *sd, const SFSnortPacket *p)
{
    sd->flags |= DCE2_SSN_FLAG__AUTODETECTED;
    sd->autodetect_dir = p->flags & (FLAG_FROM_CLIENT | FLAG_FROM_SERVER);
}

/********************************************************************
 * Function: DCE2_SsnAutodetected()
 *
 * Purpose: Returns whether or not this session was autodetected.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  int - non-zero if session was autodetected
 *        zero if session was not autodetected
 *
 ********************************************************************/
static INLINE int DCE2_SsnAutodetected(DCE2_SsnData *sd)
{
    return sd->flags & DCE2_SSN_FLAG__AUTODETECTED;
}

/********************************************************************
 * Function: DCE2_SsnAutodetectDir()
 *
 * Purpose: Returns what direction the session was autodetected on.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  int - FLAG_FROM_CLIENT if autodetected on client packet
 *        FLAG_FROM_SERVER if autodetected on server packet
 *        zero if session was not autodetected
 *
 ********************************************************************/
static INLINE int DCE2_SsnAutodetectDir(DCE2_SsnData *sd)
{
    return sd->autodetect_dir;
}

/********************************************************************
 * Function: DCE2_SsnClearAutodetected()
 *
 * Clears the autodetect flag.  To be used after it has been
 * determined this is definitely DCE/RPC traffic.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_SsnClearAutodetected(DCE2_SsnData *sd)
{
    sd->flags &= ~DCE2_SSN_FLAG__AUTODETECTED;
    sd->autodetect_dir = 0;
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
static INLINE void DCE2_SsnSetNoInspect(DCE2_SsnData *sd)
{
    sd->flags |= DCE2_SSN_FLAG__NO_INSPECT;
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
static INLINE int DCE2_SsnNoInspect(DCE2_SsnData *sd)
{
    return sd->flags & DCE2_SSN_FLAG__NO_INSPECT;
}

/********************************************************************
 * Function: DCE2_SsnGetOverlap()
 *
 * Purpose: Returns the number of overlapped bytes, i.e. bytes
 *          that the preprocessor has already inspected.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  uint16_t - the number of overlapped bytes
 *
 ********************************************************************/
static INLINE uint16_t DCE2_SsnGetOverlap(DCE2_SsnData *sd)
{
    if ((sd->cli_overlap_bytes != 0) && DCE2_SsnFromClient(sd->wire_pkt))
    {
        return sd->cli_overlap_bytes;
    }
    else if ((sd->srv_overlap_bytes != 0) && DCE2_SsnFromServer(sd->wire_pkt))
    {
        return sd->srv_overlap_bytes;
    }

    return 0;
}

/********************************************************************
 * Function: DCE2_SsnGetMissedBytes()
 *
 * Purpose: Returns the number of missed bytes.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  uint16_t - the number of overlapped bytes
 *
 ********************************************************************/
static INLINE uint32_t DCE2_SsnGetMissedBytes(DCE2_SsnData *sd)
{
    if ((sd->cli_missed_bytes != 0) && DCE2_SsnFromClient(sd->wire_pkt))
    {
        return sd->cli_missed_bytes;
    }
    else if ((sd->srv_missed_bytes != 0) && DCE2_SsnFromServer(sd->wire_pkt))
    {
        return sd->srv_missed_bytes;
    }

    return 0;
}

#endif  /* _DCE2_SESSION_H_ */

