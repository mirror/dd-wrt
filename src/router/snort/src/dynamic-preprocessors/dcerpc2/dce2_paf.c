/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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

#include "sf_types.h"
#include "sfPolicy.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "dce2_utils.h"
#include "dce2_session.h"
#include "dce2_smb.h"
#include "dce2_debug.h"
#include "snort_dce2.h"
#include "includes/dcerpc.h"
#include "includes/smb.h"

#define DCE2_SMB_PAF_SHIFT(x64, x8) { x64 <<= 8; x64 |= (uint64_t)x8; }

static uint8_t dce2_smbpaf_id = 0;
static uint8_t dce2_tcppaf_id = 0;

// Enumerations for PAF states
typedef enum _DCE2_PafSmbStates
{
    DCE2_PAF_SMB_STATES__0 = 0,  // NetBIOS type
    DCE2_PAF_SMB_STATES__1,      // Added bit of NetBIOS length
    DCE2_PAF_SMB_STATES__2,      // First byte of NetBIOS length
    DCE2_PAF_SMB_STATES__3,      // Second byte of NetBIOS length
    // Junk states
    DCE2_PAF_SMB_STATES__4,      // 0xff
    DCE2_PAF_SMB_STATES__5,      // 'S'
    DCE2_PAF_SMB_STATES__6,      // 'M'
    DCE2_PAF_SMB_STATES__7       // 'B'

} DCE2_PafSmbStates;

typedef enum _DCE2_PafTcpStates
{
    DCE2_PAF_TCP_STATES__0 = 0,
    DCE2_PAF_TCP_STATES__1,
    DCE2_PAF_TCP_STATES__2,
    DCE2_PAF_TCP_STATES__3,
    DCE2_PAF_TCP_STATES__4,   // Byte order
    DCE2_PAF_TCP_STATES__5,
    DCE2_PAF_TCP_STATES__6,
    DCE2_PAF_TCP_STATES__7,
    DCE2_PAF_TCP_STATES__8,   // First byte of fragment length
    DCE2_PAF_TCP_STATES__9    // Second byte of fragment length

} DCE2_PafTcpStates;


// State tracker for DCE/RPC over SMB PAF
typedef struct _DCE2_PafSmbState
{
    DCE2_PafSmbStates state;
    uint64_t nb_hdr;   // Enough for NetBIOS header and 4 bytes SMB header

} DCE2_PafSmbState;

// State tracker for DCE/RPC over TCP PAF
typedef struct _DCE2_PafTcpState
{
    DCE2_PafTcpStates state;
    DceRpcBoFlag byte_order;
    uint16_t frag_len;

} DCE2_PafTcpState;


// Local function prototypes
static inline bool DCE2_PafSmbIsValidNetbiosHdr(uint32_t, bool, SmbNtHdr *, uint32_t *);
static inline bool DCE2_PafAbort(void *, uint64_t);
static PAF_Status DCE2_SmbPaf(void *, void **, const uint8_t *, uint32_t, uint64_t *, uint32_t *, uint32_t *);
static PAF_Status DCE2_TcpPaf(void *, void **, const uint8_t *, uint32_t, uint64_t *, uint32_t *, uint32_t *);


/*********************************************************************
 * Function: DCE2_PafAbort()
 *
 * Purpose: Queries the dcerpc2 session data to see if paf abort
 *          flag is set.
 *
 * Arguments:
 *  void *   - stream session pointer
 *  uint32_t - flags passed in to callback.
 *             Should have PKT_FROM_CLIENT or PKT_FROM_SERVER set.
 *
 * Returns:
 *  bool - true if we should abort PAF, false if not.
 *
 *********************************************************************/
static inline bool DCE2_PafAbort(void *ssn, uint64_t flags)
{
    DCE2_SsnData *sd;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(ssn, PP_DCE2);
    if ((sd != NULL) && DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Aborting PAF because of session data check.\n"));
        return true;
    }

    return false;
}

/*********************************************************************
 * Function: DCE2_PafSmbIsValidNetbiosHdr()
 *
 * Purpose: Validates that the NetBIOS header is valid.  If in
 *          junk states, header type must be Session Message.
 *
 * Arguments:
 *  uint32_t - the 4 bytes of the NetBIOS header
 *  bool - whether we're in a junk data state or not
 *  SmbNtHdr * - Pointer to SMB header protocol identifier
 *  uint32_t * - output parameter - Length in the NetBIOS header
 *
 * Returns:
 *  bool - true if valid, false if not
 *
 *********************************************************************/
static inline bool DCE2_PafSmbIsValidNetbiosHdr(uint32_t nb_hdr, bool junk, SmbNtHdr *nt_hdr, uint32_t *nb_len)
{
    uint8_t type = (uint8_t)(nb_hdr >> 24);
    uint8_t bit = (uint8_t)((nb_hdr & 0x00ff0000) >> 16);
    uint32_t smb_id = nt_hdr ? SmbId(nt_hdr): 0;
    uint32_t nbs_hdr = 0;

    if (junk)
    {
        if (type != NBSS_SESSION_TYPE__MESSAGE)
            return false;
    }
    else
    {
        switch (type)
        {
            case NBSS_SESSION_TYPE__MESSAGE:
            case NBSS_SESSION_TYPE__REQUEST:
            case NBSS_SESSION_TYPE__POS_RESPONSE:
            case NBSS_SESSION_TYPE__NEG_RESPONSE:
            case NBSS_SESSION_TYPE__RETARGET_RESPONSE:
            case NBSS_SESSION_TYPE__KEEP_ALIVE:
                break;
            default:
                return false;
        }
    }

    /*The bit should be checked only for SMB1, because the length in NetBIOS header should not exceed 0x1FFFF. See [MS-SMB] 2.1 Transport
     * There is no such limit for SMB2 or SMB3 */
    if (smb_id == DCE2_SMB_ID)
    {
        if ((bit != 0x00) && (bit != 0x01))
            return false;
    }

    nbs_hdr = htonl(nb_hdr);
    if(smb_id == DCE2_SMB2_ID)
        *nb_len = NbssLen2((const NbssHdr *)&nbs_hdr);
    else
        *nb_len = NbssLen((const NbssHdr *)&nbs_hdr);

    return true;
}

/*********************************************************************
 * Function: DCE2_SmbPaf()
 *
 * Purpose: The DCE/RPC over SMB PAF callback.
 *          Inspects a byte at a time changing state and shifting
 *          bytes onto the 64bit nb_hdr member.  At state 3
 *          determines if NetBIOS header is valid and if so sets
 *          flush point.  If not valid goes to states 4-7 where
 *          there is the possibility that junk data was inserted
 *          before request/response.  Needs to validate SMB ID at
 *          this point.  At state 7 determines if NetBIOS header
 *          is valid and that the SMB ID is present.  Stays in
 *          state 7 until this is the case.
 *
 * Arguments:
 *  void * - stream5 session pointer
 *  void ** - SMB state tracking structure
 *  const uint8_t * - payload data to inspect
 *  uint32_t - length of payload data
 *  uint32_t - flags to check whether client or server
 *  uint32_t * - pointer to set flush point
 *  uint32_t * - pointer to set header flush point
 *
 * Returns:
 *  PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 *
 *********************************************************************/
PAF_Status DCE2_SmbPaf(void *ssn, void **user, const uint8_t *data,
        uint32_t len, uint64_t *flags, uint32_t *fp, uint32_t *fp_eoh)
{
    DCE2_PafSmbState *ss = *(DCE2_PafSmbState **)user;
    uint32_t n = 0;
    PAF_Status ps = PAF_SEARCH;
    uint32_t nb_len = 0;
    SmbNtHdr *nt_hdr = NULL;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_START_MSG));
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "SMB: %u bytes of data\n", len));

#ifdef DEBUG_MSGS
    DCE2_DEBUG_CODE(DCE2_DEBUG__PAF, printf("Session pointer: %p\n",
                _dpd.sessionAPI->get_application_data(ssn, PP_DCE2));)
    if (*flags & FLAG_FROM_CLIENT)
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Packet from Client\n"));
    else
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Packet from Server\n"));
#endif

    if (DCE2_PafAbort(ssn, *flags))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
        return PAF_ABORT;
    }

    if (ss == NULL)
    {
        // beware - we allocate here but s5 calls free() directly
        // so no pointers allowed
        ss = calloc(1, sizeof(DCE2_PafSmbState));

        if (ss == NULL)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
            return PAF_ABORT;
        }

        *user = ss;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Start state: %u\n", ss->state));

    while (n < len)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, " State %d : 0x%02x", ss->state, data[n]));

#ifdef DEBUG_MSGS
        if (isprint(data[n]))
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, " '%c'\n", data[n]));
        else
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "\n"));
#endif

        switch (ss->state)
        {
            case DCE2_PAF_SMB_STATES__0:
                ss->nb_hdr = (uint64_t)data[n];
                ss->state++;
                break;
            case DCE2_PAF_SMB_STATES__3:
                DCE2_SMB_PAF_SHIFT(ss->nb_hdr, data[n]);
                /*(data + n + 1) points to the SMB header protocol identifier (0xFF,'SMB' or 0xFE,'SMB'), which follows the NetBIOS header*/
                if ( len >= (DCE2_SMB_ID_SIZE + n + 1))
                {
                    nt_hdr = (SmbNtHdr *)(data + n + 1);
                }

                if (DCE2_PafSmbIsValidNetbiosHdr((uint32_t)ss->nb_hdr, false, nt_hdr, &nb_len))
                {
                    *fp = (nb_len + sizeof(NbssHdr) + n) - ss->state;
                    ss->state = DCE2_PAF_SMB_STATES__0;
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF,
                                "Setting flush point: %u\n", *fp));
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
                    return PAF_FLUSH;
                }
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Invalid NetBIOS header - "
                                         "entering junk data states.\n"));
                ss->state++;
                break;
            case DCE2_PAF_SMB_STATES__7:
                DCE2_SMB_PAF_SHIFT(ss->nb_hdr, data[n]);

                /*(data + n - sizeof(DCE2_SMB_ID) + 1) points to the smb_idf field in SmbNtHdr (0xFF,'SMB' or 0xFE,'SMB'), which follows the NetBIOS header*/
                nt_hdr = (SmbNtHdr *)(data + n - sizeof(DCE2_SMB_ID) + 1);

                /*ss->nb_hdr is the value to 4 bytes of NetBIOS header + 4 bytes of SMB header protocol identifier . Right shift by 32 bits to get the value of NetBIOS header*/
                if (!DCE2_PafSmbIsValidNetbiosHdr((uint32_t)(ss->nb_hdr >> 32), true, nt_hdr, &nb_len))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Invalid NetBIOS header - "
                                             "staying in State 7.\n"));
                    break;
                }
                if (((uint32_t)ss->nb_hdr != DCE2_SMB_ID)
                        && ((uint32_t)ss->nb_hdr != DCE2_SMB2_ID))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Invalid SMB ID - "
                                             "staying in State 7.\n"));
                    break;
                }

                *fp = (nb_len + sizeof(NbssHdr) + n) - ss->state;
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF,
                            "Setting flush point: %u\n", *fp));
                ss->state = DCE2_PAF_SMB_STATES__0;
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
                return PAF_FLUSH;
            default:
                DCE2_SMB_PAF_SHIFT(ss->nb_hdr, data[n]);
                ss->state++;
                break;
        }

        n++;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
    return ps;
}

/*********************************************************************
 * Function: DCE2_TcpPaf()
 *
 * Purpose: The DCE/RPC over TCP PAF callback.
 *          Inspects a byte at a time changing state.  At state 4
 *          gets byte order of PDU.  At states 8 and 9 gets
 *          fragment length and sets flush point if no more data.
 *          Otherwise accumulates flush points because there can
 *          be multiple PDUs in a single TCP segment (evasion case).
 *
 * Arguments:
 *  void * - stream5 session pointer
 *  void ** - TCP state tracking structure
 *  const uint8_t * - payload data to inspect
 *  uint32_t - length of payload data
 *  uint32_t - flags to check whether client or server
 *  uint32_t * - pointer to set flush point
 *  uint32_t * - pointer to set header flush point
 *
 * Returns:
 *  PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 *
 *********************************************************************/
PAF_Status DCE2_TcpPaf(void *ssn, void **user, const uint8_t *data,
        uint32_t len, uint64_t *flags, uint32_t *fp, uint32_t *fp_eoh)
{
    DCE2_PafTcpState *ds = *(DCE2_PafTcpState **)user;
    uint32_t n = 0;
    int start_state;
    PAF_Status ps = PAF_SEARCH;
    uint32_t tmp_fp = 0;
    DCE2_SsnData *sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(ssn, PP_DCE2);
    int num_requests = 0;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_START_MSG));
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "TCP: %u bytes of data\n", len));

    DCE2_DEBUG_CODE(DCE2_DEBUG__PAF, printf("Session pointer: %p\n",
                _dpd.sessionAPI->get_application_data(ssn, PP_DCE2));)

#ifdef DEBUG_MSGS
    if (*flags & FLAG_FROM_CLIENT)
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Packet from Client\n"));
    else
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Packet from Server\n"));
#endif

    if (DCE2_PafAbort(ssn, *flags))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
        return PAF_ABORT;
    }

    if (sd == NULL)
    {
        // Need packet to see if it's an autodetect port then do an autodetect
        // if autodetect port and not autodetected
        //     return PAF_ABORT

        bool autodetected = false;

#ifdef TARGET_BASED
        if (_dpd.isAdaptiveConfigured())
        {
            int16_t proto_id = _dpd.sessionAPI->get_application_protocol_id(ssn);

            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "No session data - checking adaptive "
                                     "to see if it's DCE/RPC.\n"));

            if (proto_id == dce2_proto_ids.dcerpc)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Adaptive says it's "
                            "DCE/RPC - no need to autodetect\n"));
                autodetected = true;
            }
            else if (proto_id != 0)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Adaptive says it's "
                            "not DCE/RPC - aborting\n"));
                return PAF_ABORT;
            }
        }

        if (!autodetected)
        {
#endif
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "No session data - autodetecting\n"));

            if (len >= sizeof(DceRpcCoHdr))
            {
                DceRpcCoHdr *co_hdr = (DceRpcCoHdr *)data;

                if ((DceRpcCoVersMaj(co_hdr) == DCERPC_PROTO_MAJOR_VERS__5)
                        && (DceRpcCoVersMin(co_hdr) == DCERPC_PROTO_MINOR_VERS__0)
                        && (((*flags & FLAG_FROM_CLIENT)
                                && DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND)
                            || ((*flags & FLAG_FROM_SERVER)
                                && DceRpcCoPduType(co_hdr) == DCERPC_PDU_TYPE__BIND_ACK))
                        && (DceRpcCoFragLen(co_hdr) >= sizeof(DceRpcCoHdr)))
                {
                    autodetected = true;
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Autodetected!\n"));
                }
            }
            else if ((*data == DCERPC_PROTO_MAJOR_VERS__5) && (*flags & FLAG_FROM_CLIENT))
            {
                autodetected = true;
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Autodetected!\n"));
            }
#ifdef TARGET_BASED
        }
#endif

        if (!autodetected)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Couldn't autodetect - aborting\n"));
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
            return PAF_ABORT;
        }
    }

    if (ds == NULL)
    {
        // beware - we allocate here but s5 calls free() directly
        // so no pointers allowed
        ds = calloc(1, sizeof(DCE2_PafTcpState));

        if (ds == NULL)
            return PAF_ABORT;

        *user = ds;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Start state: %u\n", ds->state));
    start_state = (uint8_t)ds->state;  // determines how many bytes already looked at

    while (n < len)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, " State %d : 0x%02x", ds->state, data[n]));

#ifdef DEBUG_MSGS
        if (isprint(data[n]))
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, " '%c'\n", data[n]));
        else
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "\n"));
#endif

        switch (ds->state)
        {
            case DCE2_PAF_TCP_STATES__4:  // Get byte order
                ds->byte_order = DceRpcByteOrder(data[n]);
                ds->state++;
#ifdef DEBUG_MSGS
                if (ds->byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Byte order: Little endian\n"));
                else
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Byte order: Big endian\n"));
#endif
                break;
            case DCE2_PAF_TCP_STATES__8:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "First byte of fragment length\n"));
                if (ds->byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
                    ds->frag_len = data[n];
                else
                    ds->frag_len = data[n] << 8;
                ds->state++;
                break;
            case DCE2_PAF_TCP_STATES__9:
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Second byte of fragment length\n"));
                if (ds->byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
                    ds->frag_len |= data[n] << 8;
                else
                    ds->frag_len |= data[n];

                /* If we get a bad frag length abort */
                if (ds->frag_len < sizeof(DceRpcCoHdr))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
                    return PAF_ABORT;
                }

                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Fragment length: %u\n", ds->frag_len));

                /* Increment n here so we can continue */
                n += ds->frag_len - (uint8_t)ds->state;
                num_requests++;
                /* Might have multiple PDUs in one segment.  If the last PDU is partial,
                 * flush just before it */
                if ((num_requests == 1) || (n <= len))
                    tmp_fp += ds->frag_len;
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Requests: %u\n", num_requests));
                ds->state = DCE2_PAF_TCP_STATES__0;
                continue;  // we incremented n already
            default:
                ds->state++;
                break;
        }

        n++;
    }

    if (tmp_fp != 0)
    {
        *fp = tmp_fp - start_state;
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "Setting flush point: %u\n", *fp));
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
        return PAF_FLUSH;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__PAF, "%s\n", DCE2_DEBUG__PAF_END_MSG));
    return ps;
}

/*********************************************************************
 * Function: DCE2_PafRegisterPort()
 * Function: DCE2_PafRegisterService()
 *
 * Purpose: Registers callbacks for interested ports and services.
 *          SMB and TCP ports are mutually exclusive so only one or
 *          the other will be registered for any given port.
 *
 * Arguments:
 *  uint16_t - port or service to register
 *  tSfPolicyId - the policy to register for
 *  DCE2_TransType - the type of DCE/RPC transport to register for.
 *
 * Returns:
 *  int - 0 for success.
 *
 *********************************************************************/
int DCE2_PafRegisterPort (struct _SnortConfig *sc, uint16_t port, tSfPolicyId pid, DCE2_TransType trans)
{
    if (!_dpd.isPafEnabled())
        return 0;

    switch (trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            dce2_smbpaf_id = _dpd.streamAPI->register_paf_port(sc, pid, port, 0, DCE2_SmbPaf, true);
            dce2_smbpaf_id = _dpd.streamAPI->register_paf_port(sc, pid, port, 1, DCE2_SmbPaf, true);
            break;
        case DCE2_TRANS_TYPE__TCP:
            dce2_tcppaf_id = _dpd.streamAPI->register_paf_port(sc, pid, port, 0, DCE2_TcpPaf, true);
            dce2_tcppaf_id = _dpd.streamAPI->register_paf_port(sc, pid, port, 1, DCE2_TcpPaf, true);
            break;
        default:
            DCE2_Die("Invalid transport type sent to paf registration function");
            break;
    }
    return 0;
}

#ifdef TARGET_BASED
int DCE2_PafRegisterService (struct _SnortConfig *sc, uint16_t app_id, tSfPolicyId pid, DCE2_TransType trans)
{
    if (!_dpd.isPafEnabled())
        return 0;

    switch (trans)
    {
        case DCE2_TRANS_TYPE__SMB:
            dce2_smbpaf_id = _dpd.streamAPI->register_paf_service(sc, pid, app_id, 0, DCE2_SmbPaf, true);
            dce2_smbpaf_id = _dpd.streamAPI->register_paf_service(sc, pid, app_id, 1, DCE2_SmbPaf, true);
            break;
        case DCE2_TRANS_TYPE__TCP:
            dce2_tcppaf_id = _dpd.streamAPI->register_paf_service(sc, pid, app_id, 0, DCE2_TcpPaf, true);
            dce2_tcppaf_id = _dpd.streamAPI->register_paf_service(sc, pid, app_id, 1, DCE2_TcpPaf, true);
            break;
        default:
            DCE2_Die("Invalid transport type sent to paf registration function");
            break;
    }
    return 0;
}
#endif

