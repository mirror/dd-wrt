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
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_SsnFlag
{
    DCE2_SSN_FLAG__NONE               = 0x0000,
    DCE2_SSN_FLAG__SEEN_CLIENT        = 0x0001,
    DCE2_SSN_FLAG__SEEN_SERVER        = 0x0002,
    DCE2_SSN_FLAG__AUTODETECTED       = 0x0010,
    DCE2_SSN_FLAG__PAF_ABORT          = 0x0020,
    DCE2_SSN_FLAG__NO_INSPECT         = 0x0040,
    DCE2_SSN_FLAG__SMB2               = 0x0080,
    DCE2_SSN_FLAG__ALL                = 0xffff

} DCE2_SsnFlag;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_SsnData
{
    DCE2_TransType trans;
    DCE2_Policy server_policy;
    DCE2_Policy client_policy;
    int flags;
    const DCE2_ServerConfig *sconfig;
    const SFSnortPacket *wire_pkt;
    uint64_t alert_mask;
    DCE2_Roptions ropts;
    int autodetect_dir;

    uint32_t cli_seq;
    uint32_t cli_nseq;
    uint32_t srv_seq;
    uint32_t srv_nseq;

    tSfPolicyId policy_id;
    tSfPolicyUserContextId config;

} DCE2_SsnData;

/********************************************************************
 * Extern variables
 ********************************************************************/
extern uint8_t dce2_no_inspect;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static inline int DCE2_SsnIsEstablished(const SFSnortPacket *);
static inline int DCE2_SsnIsMidstream(const SFSnortPacket *);
static inline void DCE2_SsnSetAppData(const SFSnortPacket *, void *, StreamAppDataFree);
static inline void * DCE2_SsnGetAppData(const SFSnortPacket *);
static inline int DCE2_SsnGetReassembly(const SFSnortPacket *);
static inline void DCE2_SsnSetReassembly(const SFSnortPacket *);
static inline int DCE2_SsnIsRebuilt(const SFSnortPacket *);
static inline int DCE2_SsnIsStreamInsert(const SFSnortPacket *);
static inline void DCE2_SsnFlush(SFSnortPacket *);
static inline int DCE2_SsnFromServer(const SFSnortPacket *);
static inline int DCE2_SsnFromClient(const SFSnortPacket *);
static inline bool DCE2_SsnIsPafActive(const SFSnortPacket *);
static inline void DCE2_SsnSetSeenClient(DCE2_SsnData *);
static inline int DCE2_SsnSeenClient(DCE2_SsnData *);
static inline void DCE2_SsnSetSeenServer(DCE2_SsnData *);
static inline int DCE2_SsnSeenServer(DCE2_SsnData *);
static inline void DCE2_SsnSetAutodetected(DCE2_SsnData *, const SFSnortPacket *);
static inline int DCE2_SsnAutodetected(DCE2_SsnData *);
static inline int DCE2_SsnAutodetectDir(DCE2_SsnData *);
static inline void DCE2_SsnSetNoInspect(const SFSnortPacket *);
static inline int DCE2_SsnNoInspect(DCE2_SsnData *sd);
static inline void DCE2_SsnSetPolicy(DCE2_SsnData *, DCE2_Policy);
static inline DCE2_Policy DCE2_SsnGetPolicy(DCE2_SsnData *);
static inline DCE2_Policy DCE2_SsnGetServerPolicy(DCE2_SsnData *);
static inline DCE2_Policy DCE2_SsnGetClientPolicy(DCE2_SsnData *);
static inline bool DCE2_SsnIsWindowsPolicy(DCE2_SsnData *);
static inline bool DCE2_SsnIsSambaPolicy(DCE2_SsnData *);
static inline bool DCE2_SsnIsServerWindowsPolicy(DCE2_SsnData *);
static inline bool DCE2_SsnIsServerSambaPolicy(DCE2_SsnData *);

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
static inline int DCE2_SsnIsEstablished(const SFSnortPacket *p)
{
    return _dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_ESTABLISHED;
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
static inline int DCE2_SsnIsMidstream(const SFSnortPacket *p)
{
    return _dpd.sessionAPI->get_session_flags
        (p->stream_session) & SSNFLAG_MIDSTREAM;
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
static inline void DCE2_SsnSetAppData(const SFSnortPacket *p, void *data, StreamAppDataFree sdfree)
{
    _dpd.sessionAPI->set_application_data(p->stream_session, PP_DCE2, data, sdfree);
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
static inline void * DCE2_SsnGetAppData(const SFSnortPacket *p)
{
    return _dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
}

/********************************************************************
 * Function: DCE2_SsnGetReassembly()
 *
 * Purpose: Gets reassembly direction for the session.
 *
 * Arguments:
 *  DCE2_SsnData * - session data pointer
 *
 * Returns:
 *  int - the reassembly direction
 *        SSN_DIR_NONE, SSN_DIR_FROM_CLIENT, SSN_DIR_FROM_SERVER or SSN_DIR_BOTH
 *
 ********************************************************************/
static inline int DCE2_SsnGetReassembly(const SFSnortPacket *p)
{
    return (int)_dpd.streamAPI->get_reassembly_direction(p->stream_session);
}

/********************************************************************
 * Function: DCE2_SsnSetReassembly()
 *
 * Purpose: Sets reassembly direction for the session to
 *          SSN_DIR_BOTH since the preprocessor looks at both
 *          client and server packets.
 *
 * Arguments:
 *  DCE2_SsnData * - session data pointer
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_SsnSetReassembly(const SFSnortPacket *p)
{
    _dpd.streamAPI->set_reassembly(p->stream_session, STREAM_FLPOLICY_FOOTPRINT,
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
static inline int DCE2_SsnIsRebuilt(const SFSnortPacket *p)
{
    return (PacketHasPAFPayload(p));
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
static inline int DCE2_SsnIsStreamInsert(const SFSnortPacket *p)
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
static inline void DCE2_SsnFlush(SFSnortPacket *p)
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
static inline int DCE2_SsnFromServer(const SFSnortPacket *p)
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
static inline int DCE2_SsnFromClient(const SFSnortPacket *p)
{
    return p->flags & FLAG_FROM_CLIENT;
}

/********************************************************************
 * Function: DCE2_SsnIsPafActive()
 *
 * Purpose: Checks stream api to see if PAF is active for both sides
 *          of the session.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  bool - true if paf is active
 *         false if not
 *
 ********************************************************************/
static inline bool DCE2_SsnIsPafActive(const SFSnortPacket *p)
{
    if ((p->stream_session == NULL)
            || (_dpd.streamAPI->is_paf_active(p->stream_session, true)
                && _dpd.streamAPI->is_paf_active(p->stream_session, false)))
        return true;

    return false;
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
static inline void DCE2_SsnSetSeenClient(DCE2_SsnData *sd)
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
static inline int DCE2_SsnSeenClient(DCE2_SsnData *sd)
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
static inline void DCE2_SsnSetSeenServer(DCE2_SsnData *sd)
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
static inline int DCE2_SsnSeenServer(DCE2_SsnData *sd)
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
static inline void DCE2_SsnSetAutodetected(DCE2_SsnData *sd, const SFSnortPacket *p)
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
static inline int DCE2_SsnAutodetected(DCE2_SsnData *sd)
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
static inline int DCE2_SsnAutodetectDir(DCE2_SsnData *sd)
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
static inline void DCE2_SsnClearAutodetected(DCE2_SsnData *sd)
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
static inline void DCE2_SsnSetNoInspect(const SFSnortPacket *p)
{
    _dpd.sessionAPI->set_application_data(p->stream_session, PP_DCE2,
            (void *)&dce2_no_inspect, NULL);
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
static inline int DCE2_SsnNoInspect(DCE2_SsnData *sd)
{
    return (void *)sd == (void *)&dce2_no_inspect;
}

/********************************************************************
 * Function: DCE2_SsnSetPolicy()
 *
 * Purpose:
 *  Convenience function to set policy for session, client or server.
 *  Sets policy for the current direction packet is from.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *  DCE2_Policy    - the policy to set client/server to
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_SsnSetPolicy(DCE2_SsnData *sd, DCE2_Policy policy)
{
    if (DCE2_SsnFromClient(sd->wire_pkt))
        sd->client_policy = policy;
    else
        sd->server_policy = policy;
}

/********************************************************************
 * Function: DCE2_SsnGetPolicy()
 *
 * Purpose:
 *  Convenience function to get policy for session, client or server
 *  determined by the direction the traffic is flowing to.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *  DCE2_Policy - If from the client returns server policy.
 *                If from server returns client policy.
 *
 ********************************************************************/
static inline DCE2_Policy DCE2_SsnGetPolicy(DCE2_SsnData *sd)
{
    if (DCE2_SsnFromClient(sd->wire_pkt))
        return sd->server_policy;
    else
        return sd->client_policy;
}

/********************************************************************
 * Function: DCE2_SsnGetServerPolicy()
 *
 * Purpose:
 *  Returns server policy for session
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *  DCE2_Policy - The server policy
 *
 ********************************************************************/
static inline DCE2_Policy DCE2_SsnGetServerPolicy(DCE2_SsnData *sd)
{
    return sd->server_policy;
}

/********************************************************************
 * Function: DCE2_SsnGetClientPolicy()
 *
 * Purpose:
 *  Returns client policy for session
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns: None
 *  DCE2_Policy - The client policy
 *
 ********************************************************************/
static inline DCE2_Policy DCE2_SsnGetClientPolicy(DCE2_SsnData *sd)
{
    return sd->client_policy;
}

/********************************************************************
 * Function: DCE2_SsnIsWindowsPolicy()
 *
 * Purpose:
 *  Convenience function to determine if policy traffic is going to
 *  is a Windows one.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  bool  -  true if Samba, false if not
 *
 ********************************************************************/
static inline bool DCE2_SsnIsWindowsPolicy(DCE2_SsnData *sd)
{
    DCE2_Policy policy = DCE2_SsnGetPolicy(sd);

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            return true;
        default:
            break;
    }

    return false;
}

/********************************************************************
 * Function: DCE2_SsnIsSambaPolicy()
 *
 * Purpose:
 *  Convenience function to determine if policy traffic is going to
 *  is a Samba one.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  bool  -  true if Samba, false if not
 *
 ********************************************************************/
static inline bool DCE2_SsnIsSambaPolicy(DCE2_SsnData *sd)
{
    DCE2_Policy policy = DCE2_SsnGetPolicy(sd);

    switch (policy)
    {
        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_20:
            return true;
        default:
            break;
    }

    return false;
}

/********************************************************************
 * Function: DCE2_SsnIsServerWindowsPolicy()
 *
 * Purpose:
 *  Convenience function to determine if server policy is a
 *  Windows one.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  bool  -  true if Samba, false if not
 *
 ********************************************************************/
static inline bool DCE2_SsnIsServerWindowsPolicy(DCE2_SsnData *sd)
{
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);

    switch (policy)
    {
        case DCE2_POLICY__WIN2000:
        case DCE2_POLICY__WINXP:
        case DCE2_POLICY__WINVISTA:
        case DCE2_POLICY__WIN2003:
        case DCE2_POLICY__WIN2008:
        case DCE2_POLICY__WIN7:
            return true;
        default:
            break;
    }

    return false;
}

/********************************************************************
 * Function: DCE2_SsnIsServerSambaPolicy()
 *
 * Purpose:
 *  Convenience function to determine if server policy is a
 *  Samba one.
 *
 * Arguments:
 *  DCE2_SsnData * - pointer to session data
 *
 * Returns:
 *  bool  -  true if Samba, false if not
 *
 ********************************************************************/
static inline bool DCE2_SsnIsServerSambaPolicy(DCE2_SsnData *sd)
{
    DCE2_Policy policy = DCE2_SsnGetServerPolicy(sd);

    switch (policy)
    {
        case DCE2_POLICY__SAMBA:
        case DCE2_POLICY__SAMBA_3_0_37:
        case DCE2_POLICY__SAMBA_3_0_22:
        case DCE2_POLICY__SAMBA_3_0_20:
            return true;
        default:
            break;
    }

    return false;
}

#endif  /* _DCE2_SESSION_H_ */

