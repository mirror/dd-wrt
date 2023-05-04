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

#ifndef DCERPC_H
#define DCERPC_H

#ifdef HAVE_CONFIG_H
#include "config.h"  /* For WORDS_BIGENDIAN */
#endif

/********************************************************************
 * Enumerations
 ********************************************************************/
/* DCE/RPC byte order flag */
typedef enum _DceRpcBoFlag
{
    DCERPC_BO_FLAG__NONE,
    DCERPC_BO_FLAG__BIG_ENDIAN,
    DCERPC_BO_FLAG__LITTLE_ENDIAN

} DceRpcBoFlag;

/*
 * Common to Connectionless and Connection Oriented
 */
typedef enum _DceRpcPduType
{
    DCERPC_PDU_TYPE__REQUEST = 0,
    DCERPC_PDU_TYPE__PING,
    DCERPC_PDU_TYPE__RESPONSE,
    DCERPC_PDU_TYPE__FAULT,
    DCERPC_PDU_TYPE__WORKING,
    DCERPC_PDU_TYPE__NOCALL,
    DCERPC_PDU_TYPE__REJECT,
    DCERPC_PDU_TYPE__ACK,
    DCERPC_PDU_TYPE__CL_CANCEL,
    DCERPC_PDU_TYPE__FACK,
    DCERPC_PDU_TYPE__CANCEL_ACK,
    DCERPC_PDU_TYPE__BIND,
    DCERPC_PDU_TYPE__BIND_ACK,
    DCERPC_PDU_TYPE__BIND_NACK,
    DCERPC_PDU_TYPE__ALTER_CONTEXT,
    DCERPC_PDU_TYPE__ALTER_CONTEXT_RESP,
    DCERPC_PDU_TYPE__AUTH3,
    DCERPC_PDU_TYPE__SHUTDOWN,
    DCERPC_PDU_TYPE__CO_CANCEL,
    DCERPC_PDU_TYPE__ORPHANED,
    DCERPC_PDU_TYPE__MICROSOFT_PROPRIETARY_OUTLOOK2003_RPC_OVER_HTTP,
    DCERPC_PDU_TYPE__MAX

} DceRpcPduType;

/* Version 4 is for Connectionless
 * Version 5 is for Connection oriented */
typedef enum _DceRpcProtoMajorVers
{
    DCERPC_PROTO_MAJOR_VERS__4 = 4,
    DCERPC_PROTO_MAJOR_VERS__5 = 5

} DceRpcProtoMajorVers;

typedef enum _DceRpcProtoMinorVers
{
    DCERPC_PROTO_MINOR_VERS__0 = 0,
    DCERPC_PROTO_MINOR_VERS__1 = 1

} DceRpcProtoMinorVers;

/*
 * Connectionless
 */
typedef enum _DceRpcClFlags1
{
    DCERPC_CL_FLAGS1__RESERVED_01 = 0x01,
    DCERPC_CL_FLAGS1__LASTFRAG = 0x02,
    DCERPC_CL_FLAGS1__FRAG = 0x04,
    DCERPC_CL_FLAGS1__NOFACK = 0x08,
    DCERPC_CL_FLAGS1__MAYBE = 0x10,
    DCERPC_CL_FLAGS1__IDEMPOTENT = 0x20,
    DCERPC_CL_FLAGS1__BROADCAST = 0x40,
    DCERPC_CL_FLAGS1__RESERVED_80 = 0x80

} DceRpcClFlags1;

typedef enum _DceRpcClFlags2
{
    DCERPC_CL_FLAGS2__RESERVED_01 = 0x01,
    DCERPC_CL_FLAGS2__CANCEL_PENDING = 0x02,
    DCERPC_CL_FLAGS2__RESERVED_04 = 0x04,
    DCERPC_CL_FLAGS2__RESERVED_08 = 0x08,
    DCERPC_CL_FLAGS2__RESERVED_10 = 0x10,
    DCERPC_CL_FLAGS2__RESERVED_20 = 0x20,
    DCERPC_CL_FLAGS2__RESERVED_40 = 0x40,
    DCERPC_CL_FLAGS2__RESERVED_80 = 0x80

} DceRpcClFlags2;

typedef enum _DCERPC_AuthProto
{
    DCERPC_AUTH_PROTO__NONE = 0,
    DCERPC_AUTH_PROTO__OSF_DCERPC_PK_AUTH = 1

} DCERPC_AuthProto;

/*
 * Connection oriented
 */
typedef enum _DceRpcCoPfcFlags
{
    DCERPC_CO_PFC_FLAGS__FIRST_FRAG = 0x01,
    DCERPC_CO_PFC_FLAGS__LAST_FRAG = 0x02,
    DCERPC_CO_PFC_FLAGS__PENDING_CANCEL = 0x04,
    DCERPC_CO_PFC_FLAGS__RESERVED_1 = 0x08,
    DCERPC_CO_PFC_FLAGS__CONC_MPX = 0x10,
    DCERPC_CO_PFC_FLAGS__DID_NOT_EXECUTE = 0x20,
    DCERPC_CO_PFC_FLAGS__MAYBE = 0x40,
    DCERPC_CO_PFC_FLAGS__OBJECT_UUID = 0x80

} DceRpcCoPfcFlags;

/* Presentation context definition result */
typedef enum _DceRpcCoContDefResult
{
    DCERPC_CO_CONT_DEF_RESULT__ACCEPTANCE = 0,
    DCERPC_CO_CONT_DEF_RESULT__USER_REJECTION,
    DCERPC_CO_CONT_DEF_RESULT__PROVIDER_REJECTION

} DceRpcCoContDefResult;

/* Presentation provider rejection reason */
typedef enum _DceRpcCoProvRejReason
{
    DCERPC_CO_PROV_REJ_REASON__REASON_NOT_SPECIFIED = 0,
    DCERPC_CO_PROV_REJ_REASON__ABSTRACT_SYNTAX_NOT_SUPPORTED,
    DCERPC_CO_PROV_REJ_REASON__PROPOSED_TRANSFER_SYNTAXES_NOT_SUPPORTED,
    DCERPC_CO_PROV_REJ_REASON__LOCAL_LIMIT_EXCEEDED

} DceRpcCoProvRejReason;

typedef enum _DceRpcCoBindNakReason
{
    DCERPC_CO_BIND_NAK_REASON__REASON_NOT_SPECIFIED = 0,
    DCERPC_CO_BIND_NAK_REASON__TEMPORARY_CONGESTION,
    DCERPC_CO_BIND_NAK_REASON__LOCAL_LIMIT_EXECEEDED,
    DCERPC_CO_BIND_NAK_REASON__CALLED_PADDR_UNKNOWN,
    DCERPC_CO_BIND_NAK_REASON__PROTOCOL_VERSION_NOT_SUPPORTED,
    DCERPC_CO_BIND_NAK_REASON__DEFAULT_CONTEXT_NOT_SUPPORTED,
    DCERPC_CO_BIND_NAK_REASON__USER_DATA_NOT_READABLE,
    DCERPC_CO_BIND_NAK_REASON__NO_PSAP_AVAILABLE

} DceRpcCoBindNakReason;

typedef enum _DceRpcCoAuthLevelType
{
    DCERPC_CO_AUTH_LEVEL__NONE = 1,
    DCERPC_CO_AUTH_LEVEL__CONNECT,
    DCERPC_CO_AUTH_LEVEL__CALL,
    DCERPC_CO_AUTH_LEVEL__PKT,
    DCERPC_CO_AUTH_LEVEL__PKT_INTEGRITY,
    DCERPC_CO_AUTH_LEVEL__PKT_PRIVACY

} DceRpcCoAuthLevelType;

/********************************************************************
 * Structures
 ********************************************************************/
#ifdef WIN32
#pragma pack(push, dcerpc_hdrs, 1)
#else
#pragma pack(1)
#endif

typedef struct _Uuid
{
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_high_and_version;
    uint8_t clock_seq_and_reserved;
    uint8_t clock_seq_low;
    uint8_t node[6];

} Uuid;

/*
 * Connectionless
 */
typedef struct _DceRpcClHdr   /* Connectionless header */
{
    uint8_t rpc_vers;
    uint8_t ptype;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t drep[3];
    uint8_t serial_hi;
    Uuid object;
    Uuid if_id;
    Uuid act_id;
    uint32_t server_boot;
    uint32_t if_vers;
    uint32_t seqnum;
    uint16_t opnum;
    uint16_t ihint;
    uint16_t ahint;
    uint16_t len;
    uint16_t fragnum;
    uint8_t auth_proto;
    uint8_t serial_lo;

} DceRpcClHdr;

/* ack PDU contains no body */

/* cancel PDU */
typedef struct _DceRpcClCancel
{
    uint32_t vers;
    uint32_t cancel_id;

} DceRpcClCancel;

/* cancel_ack PDU */
typedef struct _DceRpcClCancelAck
{
    uint32_t vers;
    uint32_t cancel_id;
    int server_is_accepting;

} DceRpcClCancelAck;

/* fack PDU */
typedef struct _DceRpcClFack
{
    uint8_t vers;
    uint8_t pad1;
    uint16_t window_size;
    uint32_t max_tpdu;
    uint32_t max_frag_size;
    uint16_t serial_num;
    uint16_t selack_len;
    uint32_t selack[1];  /* variable length */

} DceRpcClFack;

/* fault PDU */
typedef struct _DceRpcClFault
{
    uint32_t status;  /* status code */

} DceRpcClFault;

/* nocall PDU (negative reply to ping) contains no body */
/* ping PDU contains no body */

/* reject PDU is the same as fack */
typedef DceRpcClFault DceRpcClReject;

/* request PDU contains stub data as body */
/* response PDU contains stub data as body */

/* working PDU (positive reply to ping) contains no body */

/*
 * Connection oriented
 */
typedef struct _DceRpcCoVersion
{
    uint8_t major;
    uint8_t minor;

} DceRpcCoVersion;

/* Connection oriented common header */
typedef struct _DceRpcCoHdr
{
    DceRpcCoVersion pversion;
    uint8_t ptype;
    uint8_t pfc_flags;
    uint8_t packed_drep[4];
    uint16_t frag_length;
    uint16_t auth_length;
    uint32_t call_id;

} DceRpcCoHdr;

/* Presentation syntax id */
typedef struct _DceRpcCoSynId
{
    Uuid if_uuid;
    uint32_t if_version;

} DceRpcCoSynId;

/* Presentation context element */
typedef struct _DceRpcCoContElem
{
    uint16_t p_cont_id;
    uint8_t n_transfer_syn;  /* number of transfer syntaxes */
    uint8_t reserved;
    DceRpcCoSynId abstract_syntax;
#if 0
    DceRpcCoSynId transfer_syntaxes[]; /* variable length */
#endif
} DceRpcCoContElem;

#if 0   /* Put this in the Bind header */
/* Presentation context list */
typedef struct _DceRpcCoContList
{
    uint8_t n_context_elem;   /* number of context elements */
    uint8_t reserved;
    uint16_t reserved2;
#if 0
    DceRpcCoContElem p_cont_elem[];  /* variable length */
#endif
} DceRpcCoContList;
#endif

/* Presentation result */
typedef struct _DceRpcCoContResult
{
    uint16_t result;
    uint16_t reason;
    DceRpcCoSynId transfer_syntax;

} DceRpcCoContResult;

typedef struct _DceRpcCoContResultList
{
    uint8_t n_results;
    uint8_t reserved;
    uint16_t reserved2;
#if 0
    DceRpcCoContResult p_results[];  /* variable length */
#endif
} DceRpcCoContResultList;

/* DCE version supported */
typedef struct _DceRpcCoVerSup
{
    uint8_t n_protocols;  /* number of protocols */
#if 0
    DceRpcCoVersion protocols[];  /* variable length */
#endif
} DceRpcCoVerSup;

/* Bind */
typedef struct _DceRpcCoBind
{
    uint16_t max_xmit_frag;
    uint16_t max_recv_frag;
    uint32_t assoc_group_id;
    uint8_t n_context_elem;   /* number of context elements */
    uint8_t reserved;
    uint16_t reserved2;
#if 0
    uint16_t p_cont_id;
    uint8_t n_tranfer_syn;  /* number of transfer syntaxes */
    uint8_t reserved;
    DceRpcCoSynId abstract_syntax;
#endif
#if 0
    DceRpcCoContList p_context_elem_list;  /* variable length */
    auth_verifier_co_t auth_verifier;
#endif
} DceRpcCoBind;

/* Bind response */
typedef struct _DceRpcCoBindAck
{
    uint16_t max_xmit_frag;
    uint16_t max_recv_frag;
    uint32_t assoc_group_id;
    uint16_t sec_addr_len;
#if 0
    char sec_addr[];  /* variable length */
    uint8_t pad2[align(4)];     /* this is really to align the above field
                                   whose last member is a variable len str.
                                   It will be 0-3 bytes long. */
    DceRpcCoContResultList p_context_elem;
    aut_verifier_co_t auth_verifier;
#endif
} DceRpcCoBindAck;

typedef DceRpcCoBind DceRpcCoAltCtx;
typedef DceRpcCoBindAck DceRpcCoAltCtxResp;

typedef struct _DceRpcCoBindNak
{
    DceRpcCoBindNakReason provider_reject_reason;
#if 0
    DceRpcCoVerSup versions;  /* variable length */
#endif
} DceRpcCoBindNak;

#if 0
typedef struct _DceRpcCoCancel
{
    auth_verifier_co_t auth_verifier;
} DceRpcCoCancel;
#endif

typedef struct _DceRpcCoFault
{
    uint32_t alloc_hint;
    uint16_t context_id;
    uint8_t cancel_count;
    uint8_t reserved;
    uint32_t status;
    uint8_t reserved2[4];
#if 0
    uint8_t stub data[]   /* 8 octet aligned if auth_verifier, which will
                             take care of the pad. */
    auth_verifier_co_t auth_verifier;
#endif
} DceRpcCoFault;

#if 0
typedef struct _DceRpcCoOrphaned
{
    auth_verifier_co_t auth_verifier;
} DceRpcCoOrphaned;
#endif

typedef struct _DceRpcCoRequest
{
    uint32_t alloc_hint;
    uint16_t context_id;
    uint16_t opnum;
#if 0
    Uuid object;           /* only if object flag is set */
    uint8_t stub data[];   /* 8 octet aligned if auth_verifier, which will
                              take care of the pad. */
    auth_verifier_co_t auth_verifier;
#endif
} DceRpcCoRequest;

typedef struct _DceRpcCoResponse
{
    uint32_t alloc_hint;
    uint16_t context_id;
    uint8_t cancel_count;
    uint8_t reserved;
#if 0
    uint8_t stub data[]   /* 8 octet aligned if auth_verifier, which will
                             take care of the pad. */
    auth_verifier_co_t auth_verifier;
#endif
} DceRpcCoResponse;

#if 0
typedef struct _DceRpcCoShutdown
{
    // nothing

} DceRpcCoShutdown;
#endif

typedef struct _DceRpcCoAuthVerifier
{
#if 0
    uint8_t auth_pad[];  /* variable length to restore 4 byte alignment */
#endif
    uint8_t auth_type;
    uint8_t auth_level;
    uint8_t auth_pad_length;
    uint8_t auth_reserved;
    uint32_t auth_context_id;
#if 0
    uint8_t auth_value[];  /* variable auth_length */
#endif

} DceRpcCoAuthVerifier;

/* Optional Data used with Reject/Disconnect header
 * These do not share the common header, but are special
 * cases (pretty much the same as the common header) */
typedef uint16_t DceRpcReasonCode;

typedef struct _DceRpcCoOptData
{
    DceRpcCoVersion pversion;
    uint8_t reserved[2];
    uint8_t packed_drep[4];
    uint32_t reject_status;
    uint8_t reserved2[4];

} DceRpcCoOptData;

typedef struct _DceRpcCoRejHdr
{
    DceRpcReasonCode reason_code;
    DceRpcCoOptData rpc_info;

} DceRpcCoRejHdr;

/* Disconnect header same as Reject header */
typedef DceRpcCoRejHdr DceRpcCoDiscHdr;

#ifdef WIN32
#pragma pack(pop, dcerpc_hdrs)
#else
#pragma pack()
#endif

/********************************************************************
 * Inline functions prototypes
 ********************************************************************/
static inline DceRpcBoFlag DceRpcByteOrder(const uint8_t);
static inline uint16_t DceRpcNtohs(const uint16_t *, const DceRpcBoFlag);
static inline uint16_t DceRpcHtons(const uint16_t *, const DceRpcBoFlag);
static inline uint32_t DceRpcNtohl(const uint32_t *, const DceRpcBoFlag);
static inline uint32_t DceRpcHtonl(const uint32_t *, const DceRpcBoFlag);

/* Connectionless */
static inline uint8_t DceRpcClRpcVers(const DceRpcClHdr *);
static inline DceRpcBoFlag DceRpcClByteOrder(const DceRpcClHdr *);
static inline uint32_t DceRpcClIfaceVers(const DceRpcClHdr *);
static inline uint16_t DceRpcClOpnum(const DceRpcClHdr *);
static inline uint32_t DceRpcClSeqNum(const DceRpcClHdr *);
static inline uint16_t DceRpcClFragNum(const DceRpcClHdr *);
static inline int DceRpcClFragFlag(const DceRpcClHdr *);
static inline int DceRpcClLastFrag(const DceRpcClHdr *);
static inline int DceRpcClFirstFrag(const DceRpcClHdr *);
static inline uint16_t DceRpcClLen(const DceRpcClHdr *);
static inline int DceRpcClFrag(const DceRpcClHdr *);

/* Connection oriented */
static inline uint8_t DceRpcCoVersMaj(const DceRpcCoHdr *);
static inline uint8_t DceRpcCoVersMin(const DceRpcCoHdr *);
static inline DceRpcPduType DceRpcCoPduType(const DceRpcCoHdr *);
static inline int DceRpcCoFirstFrag(const DceRpcCoHdr *);
static inline int DceRpcCoLastFrag(const DceRpcCoHdr *);
static inline int DceRpcCoObjectFlag(const DceRpcCoHdr *);
static inline DceRpcBoFlag DceRpcCoByteOrder(const DceRpcCoHdr *);
static inline uint16_t DceRpcCoFragLen(const DceRpcCoHdr *);
static inline uint16_t DceRpcCoAuthLen(const DceRpcCoHdr *);
static inline uint32_t DceRpcCoCallId(const DceRpcCoHdr *);
static inline uint16_t DceRpcCoCtxId(const DceRpcCoHdr *, const DceRpcCoRequest *);
static inline uint16_t DceRpcCoCtxIdResp(const DceRpcCoHdr *, const DceRpcCoResponse *);
static inline uint16_t DceRpcCoOpnum(const DceRpcCoHdr *, const DceRpcCoRequest *);
static inline uint16_t DceRpcCoBindMaxXmitFrag(const DceRpcCoHdr *, const DceRpcCoBind *);
static inline uint16_t DceRpcCoBindAckMaxRecvFrag(const DceRpcCoHdr *, const DceRpcCoBindAck *);
static inline uint8_t DceRpcCoNumCtxItems(const DceRpcCoBind *);
static inline uint16_t DceRpcCoContElemCtxId(const DceRpcCoHdr *, const DceRpcCoContElem *);
static inline uint8_t DceRpcCoContElemNumTransSyntaxes(const DceRpcCoContElem *);
static inline const Uuid * DceRpcCoContElemIface(const DceRpcCoContElem *);
static inline uint16_t DceRpcCoContElemIfaceVersMaj(const DceRpcCoHdr *, const DceRpcCoContElem *);
static inline uint16_t DceRpcCoContElemIfaceVersMin(const DceRpcCoHdr *, const DceRpcCoContElem *);
static inline uint16_t DceRpcCoSecAddrLen(const DceRpcCoHdr *, const DceRpcCoBindAck *);
static inline uint8_t DceRpcCoContNumResults(const DceRpcCoContResultList *);
static inline uint16_t DceRpcCoContRes(const DceRpcCoHdr *, const DceRpcCoContResult *);
static inline uint16_t DceRpcCoAuthPad(const DceRpcCoAuthVerifier *);
static inline uint8_t DceRpcCoAuthLevel(const DceRpcCoAuthVerifier *);

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
static inline DceRpcBoFlag DceRpcByteOrder(const uint8_t value)
{
    if ((value & 0x10) >> 4)
        return DCERPC_BO_FLAG__LITTLE_ENDIAN;

    return DCERPC_BO_FLAG__BIG_ENDIAN;
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
static inline uint16_t DceRpcNtohs(const uint16_t *ptr, const DceRpcBoFlag bo_flag)
{
    uint16_t value;

    if (ptr == NULL)
        return 0;

#ifdef WORDS_MUSTALIGN
    value = *((uint8_t *)ptr) << 8 | *((uint8_t *)ptr + 1);
#else
    value = *ptr;
#endif  /* WORDS_MUSTALIGN */

    if (bo_flag == DCERPC_BO_FLAG__NONE)
        return value;

#ifdef WORDS_BIGENDIAN
    if (bo_flag == DCERPC_BO_FLAG__BIG_ENDIAN)
#else
    if (bo_flag == DCERPC_BO_FLAG__LITTLE_ENDIAN)
#endif  /* WORDS_BIGENDIAN */
        return value;

    return ((value & 0xff00) >> 8) | ((value & 0x00ff) << 8);
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
static inline uint16_t DceRpcHtons(const uint16_t *ptr, const DceRpcBoFlag bo_flag)
{
    return DceRpcNtohs(ptr, bo_flag);
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
static inline uint32_t DceRpcNtohl(const uint32_t *ptr, const DceRpcBoFlag bo_flag)
{
    uint32_t value;

    if (ptr == NULL)
        return 0;

#ifdef WORDS_MUSTALIGN
    value = *((uint8_t *)ptr)     << 24 | *((uint8_t *)ptr + 1) << 16 |
            *((uint8_t *)ptr + 2) << 8  | *((uint8_t *)ptr + 3);
#else
    value = *ptr;
#endif  /* WORDS_MUSTALIGN */

    if (bo_flag == DCERPC_BO_FLAG__NONE)
        return value;

#ifdef WORDS_BIGENDIAN
    if (bo_flag == DCERPC_BO_FLAG__BIG_ENDIAN)
#else
    if (bo_flag == DCERPC_BO_FLAG__LITTLE_ENDIAN)
#endif  /* WORDS_BIGENDIAN */
        return value;

    return ((value & 0xff000000) >> 24) | ((value & 0x00ff0000) >> 8) |
           ((value & 0x0000ff00) << 8)  | ((value & 0x000000ff) << 24);
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
static inline uint32_t DceRpcHtonl(const uint32_t *ptr, const DceRpcBoFlag bo_flag)
{
    return DceRpcNtohl(ptr, bo_flag);
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
static inline uint8_t DceRpcClRpcVers(const DceRpcClHdr *cl)
{
    return cl->rpc_vers;
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
static inline uint8_t DceRpcClPduType(const DceRpcClHdr *cl)
{
    return cl->ptype;
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
static inline DceRpcBoFlag DceRpcClByteOrder(const DceRpcClHdr *cl)
{
    return DceRpcByteOrder(cl->drep[0]);
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
static inline const Uuid * DceRpcClIface(const DceRpcClHdr *cl)
{
    return &cl->if_id;
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
static inline uint32_t DceRpcClIfaceVers(const DceRpcClHdr *cl)
{
    return DceRpcNtohl(&cl->if_vers, DceRpcClByteOrder(cl));
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
static inline uint16_t DceRpcClOpnum(const DceRpcClHdr *cl)
{
    return DceRpcNtohs(&cl->opnum, DceRpcClByteOrder(cl));
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
static inline uint32_t DceRpcClSeqNum(const DceRpcClHdr *cl)
{
    return DceRpcNtohl(&cl->seqnum, DceRpcClByteOrder(cl));
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
static inline uint16_t DceRpcClFragNum(const DceRpcClHdr *cl)
{
    return DceRpcNtohs(&cl->fragnum, DceRpcClByteOrder(cl));
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
static inline int DceRpcClFragFlag(const DceRpcClHdr *cl)
{
    return cl->flags1 & DCERPC_CL_FLAGS1__FRAG;
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
static inline int DceRpcClLastFrag(const DceRpcClHdr *cl)
{
    return cl->flags1 & DCERPC_CL_FLAGS1__LASTFRAG;
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
static inline int DceRpcClFirstFrag(const DceRpcClHdr *cl)
{
    return (DceRpcClFragFlag(cl) && (DceRpcClFragNum(cl) == 0));
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
static inline uint16_t DceRpcClLen(const DceRpcClHdr *cl)
{
    return DceRpcNtohs(&cl->len, DceRpcClByteOrder(cl));
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
static inline int DceRpcClFrag(const DceRpcClHdr *cl)
{
    if (DceRpcClFragFlag(cl))
    {
        if (DceRpcClLastFrag(cl) && (DceRpcClFragNum(cl) == 0))
            return 0;

        return 1;
    }

    return 0;
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
static inline uint8_t DceRpcCoVersMaj(const DceRpcCoHdr *co)
{
    return co->pversion.major;
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
static inline uint8_t DceRpcCoVersMin(const DceRpcCoHdr *co)
{
    return co->pversion.minor;
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
static inline DceRpcPduType DceRpcCoPduType(const DceRpcCoHdr *co)
{
    return (DceRpcPduType)co->ptype;
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
static inline int DceRpcCoFirstFrag(const DceRpcCoHdr *co)
{
    return co->pfc_flags & DCERPC_CO_PFC_FLAGS__FIRST_FRAG;
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
static inline int DceRpcCoLastFrag(const DceRpcCoHdr *co)
{
    return co->pfc_flags & DCERPC_CO_PFC_FLAGS__LAST_FRAG;
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
static inline int DceRpcCoObjectFlag(const DceRpcCoHdr *co)
{
    return co->pfc_flags & DCERPC_CO_PFC_FLAGS__OBJECT_UUID;
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
static inline DceRpcBoFlag DceRpcCoByteOrder(const DceRpcCoHdr *co)
{
    return DceRpcByteOrder(co->packed_drep[0]);
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
static inline uint16_t DceRpcCoFragLen(const DceRpcCoHdr *co)
{
    return DceRpcNtohs(&co->frag_length, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoAuthLen(const DceRpcCoHdr *co)
{
    return DceRpcNtohs(&co->auth_length, DceRpcCoByteOrder(co));
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
static inline uint32_t DceRpcCoCallId(const DceRpcCoHdr *co)
{
    return DceRpcNtohl(&co->call_id, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoOpnum(const DceRpcCoHdr *co, const DceRpcCoRequest *cor)
{
    return DceRpcNtohs(&cor->opnum, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoCtxId(const DceRpcCoHdr *co, const DceRpcCoRequest *cor)
{
    return DceRpcNtohs(&cor->context_id, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoCtxIdResp(const DceRpcCoHdr *co, const DceRpcCoResponse *cor)
{
    return DceRpcNtohs(&cor->context_id, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoBindMaxXmitFrag(const DceRpcCoHdr *co, const DceRpcCoBind *cob)
{
    return DceRpcNtohs(&cob->max_xmit_frag, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoBindAckMaxRecvFrag(const DceRpcCoHdr *co, const DceRpcCoBindAck *coba)
{
    return DceRpcNtohs(&coba->max_recv_frag, DceRpcCoByteOrder(co));
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
static inline uint8_t DceRpcCoNumCtxItems(const DceRpcCoBind *cob)
{
    return cob->n_context_elem;
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
static inline uint16_t DceRpcCoContElemCtxId(const DceRpcCoHdr *co, const DceRpcCoContElem *coce)
{
    return DceRpcNtohs(&coce->p_cont_id, DceRpcCoByteOrder(co));
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
static inline uint8_t DceRpcCoContElemNumTransSyntaxes(const DceRpcCoContElem *coce)
{
    return coce->n_transfer_syn;
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
static inline const Uuid * DceRpcCoContElemIface(const DceRpcCoContElem *coce)
{
    return &coce->abstract_syntax.if_uuid;
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
static inline uint16_t DceRpcCoContElemIfaceVersMaj(const DceRpcCoHdr *co, const DceRpcCoContElem *coce)
{
    return (uint16_t)(DceRpcNtohl(&coce->abstract_syntax.if_version, DceRpcCoByteOrder(co)) & 0x0000ffff);
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
static inline uint16_t DceRpcCoContElemIfaceVersMin(const DceRpcCoHdr *co, const DceRpcCoContElem *coce)
{
    return (uint16_t)(DceRpcNtohl(&coce->abstract_syntax.if_version, DceRpcCoByteOrder(co)) >> 16);
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
static inline uint16_t DceRpcCoSecAddrLen(const DceRpcCoHdr *co, const DceRpcCoBindAck *coba)
{
    return DceRpcNtohs(&coba->sec_addr_len, DceRpcCoByteOrder(co));
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
static inline uint8_t DceRpcCoContNumResults(const DceRpcCoContResultList *cocrl)
{
    return cocrl->n_results;
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
static inline uint16_t DceRpcCoContRes(const DceRpcCoHdr *co, const DceRpcCoContResult *cocr)
{
    return DceRpcNtohs(&cocr->result, DceRpcCoByteOrder(co));
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
static inline uint16_t DceRpcCoAuthPad(const DceRpcCoAuthVerifier *coav)
{
    return coav->auth_pad_length;
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
static inline uint8_t DceRpcCoAuthLevel(const DceRpcCoAuthVerifier *coav)
{
    return coav->auth_level;
}

#endif  /* DCERPC_H */

