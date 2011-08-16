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

#ifndef _SMB_H_
#define _SMB_H_

#ifdef HAVE_CONFIG_H
#include "config.h"  /* For WORDS_BIGENDIAN */
#endif

#include "debug.h"   /* For INLINE */
#include "sf_types.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define NBSS_SESSION_TYPE__MESSAGE            0x00
#define NBSS_SESSION_TYPE__REQUEST            0x81
#define NBSS_SESSION_TYPE__POS_RESPONSE       0x82
#define NBSS_SESSION_TYPE__NEG_RESPONSE       0x83
#define NBSS_SESSION_TYPE__RETARGET_RESPONSE  0x84
#define NBSS_SESSION_TYPE__KEEP_ALIVE         0x85

/* Case insensitive */
#define SMB_CORE_PROTOCOL  "PC NETWORK PROGRAM 1.0"
#define SMB_LANMAN10       "LANMAN1.0"
#define SMB_LANMAN20       "LM1.2X002"
#define SMB_LANMAN20_DOS   "DOS LM1.2X002"  /* For machines running MS-DOS */
#define SMB_LANMAN21       "LANMAN2.1"
#define SMB_LANMAN21_DOS   "DOS LANMAN2.1"  /* For machines running MS-DOS */
#define SMB_NT_LANMAN10    "NT LANMAN 1.0"
/* Not sure about these dialects, i.e which protocol they conjure.
 * They are mentioned in the LM2.0 document as being previously
 * mentioned elsewhere.
 *
 * "PCLAN1.0"
 * "MICROSOFT NETWORKS 1.03"
 * "MICROSOFT NETWORKS 3.0"
 */

#define SMB_FLG__TYPE  0x80
#define SMB_TYPE__REQUEST   0
#define SMB_TYPE__RESPONSE  1

#define SMB_FLG2__UNICODE      0x8000
#define SMB_FLG2__NT_CODES     0x4000

#define SMB_NT_STATUS_SEVERITY__SUCCESS        0
#define SMB_NT_STATUS_SEVERITY__INFORMATIONAL  1
#define SMB_NT_STATUS_SEVERITY__WARNING        2
#define SMB_NT_STATUS_SEVERITY__ERROR          3

#define SMB_NT_STATUS__SUCCESS  0x00000000

#define SMB_ERROR_CLASS__SUCCESS  0x00
#define SMB_ERROR_CLASS__ERRDOS   0x01
#define SMB_ERROR_CLASS__ERRSRV   0x02
#define SMB_ERROR_CLASS__ERRHRD   0x03
#define SMB_ERROR_CLASS__ERRXOS   0x04
#define SMB_ERROR_CLASS__ERRMX1   0xe1
#define SMB_ERROR_CLASS__ERRMX2   0xe2
#define SMB_ERROR_CLASS__ERRMX3   0xe3
#define SMB_ERROR_CLASS__ERRCMD   0xff

#define SMB_ERRDOS__MORE_DATA  234

/* SMB formats (smb_fmt) Dialect, Pathname and ASCII are all
 * NULL terminated ASCII strings unless Unicode is specified
 * in the NT LM 1.0 SMB header in which case they are NULL
 * terminated unicode strings
 */ 
#define SMB_FMT__DATA_BLOCK  1
#define SMB_FMT__ASCII       4

/* smb_com (command) codes */
#define SMB_COM_OPEN              0x02   /* open file */
#define SMB_COM_CLOSE             0x04   /* close file */
#define SMB_COM_RENAME            0x07   /* rename file */
#define SMB_COM_READ              0x0a   /* read from file*/
#define SMB_COM_WRITE             0x0b   /* write to file */
#define SMB_COM_READ_BLOCK_RAW    0x1a   /* read block raw */
#define SMB_COM_WRITE_BLOCK_RAW   0x1d   /* write block raw */
#define SMB_COM_WRITE_COMPLETE    0x20   /* write complete response */
#define SMB_COM_TRANS             0x25   /* transaction - name, bytes in/out */
#define SMB_COM_TRANS_SEC         0x26   /* transaction (secondary request/response) */
#define SMB_COM_WRITE_AND_CLOSE   0x2c   /* Write and Close */
#define SMB_COM_OPEN_ANDX         0x2d   /* open and X */
#define SMB_COM_READ_ANDX         0x2e   /* read and X */
#define SMB_COM_WRITE_ANDX        0x2f   /* write and X */
#define SMB_COM_NT_CREATE_ANDX    0xa2   /* nt create and X */
#define SMB_COM_TREE_CON          0x70   /* tree connect */
#define SMB_COM_TREE_DIS          0x71   /* tree disconnect */
#define SMB_COM_NEGPROT           0x72   /* negotiate protocol */
#define SMB_COM_SESS_SETUP_ANDX   0x73   /* Session Set Up & X (including User Logon) */
#define SMB_COM_LOGOFF_ANDX       0x74   /* User logoff and X */
#define SMB_COM_TREE_CON_ANDX     0x75   /* tree connect and X */
#define SMB_COM_NO_ANDX_COMMAND   0xff   /* no next and X command */

/* Size of word count field + Word count * 2 bytes + Size of byte count field */
#define SMB_COM_SIZE(wct)  (sizeof(uint8_t) + ((wct) * sizeof(uint16_t)) + sizeof(uint16_t))

#define SMB_TRANS_FUNC__SET_NM_P_HAND_STATE  0x01
#define SMB_TRANS_FUNC__RAW_READ_NM_PIPE     0x11
#define SMB_TRANS_FUNC__Q_NM_P_HAND_STATE    0x21
#define SMB_TRANS_FUNC__Q_NM_PIPE_INFO       0x22
#define SMB_TRANS_FUNC__PEEK_NM_PIPE         0x23
#define SMB_TRANS_FUNC__TRANSACT_NM_PIPE     0x26
#define SMB_TRANS_FUNC__RAW_WRITE_NM_PIPE    0x31
#define SMB_TRANS_FUNC__WAIT_NM_PIPE         0x53
#define SMB_TRANS_FUNC__CALL_NM_PIPE         0x54

/********************************************************************
 * Structures
 ********************************************************************/
/* Pack the structs since we'll be laying them on top of packet data */
#ifdef WIN32
#pragma pack(push,smb_hdrs,1)
#else
#pragma pack(1)
#endif

/* Treat flags as the upper byte to length */
typedef struct _NbssHdr
{
    uint8_t  type;
    uint8_t  flags;
    uint16_t length;

} NbssHdr;

typedef struct _SmbCoreHdr
{
    uint8_t  smb_idf[4];  /* contains 0xFF, 'SMB' */
    uint8_t  smb_com;     /* command code */
    uint8_t  smb_rcls;    /* error code class */
    uint8_t  smb_reh;     /* reserved (contains AH if DOS INT-24 ERR) */
    uint16_t smb_err;     /* error code */
    uint8_t  smb_reb;     /* reserved */
    uint16_t smb_res[7];  /* reserved */
    uint16_t smb_tid;     /* tree id # */
    uint16_t smb_pid;     /* caller's process id # */
    uint16_t smb_uid;     /* user id # */
    uint16_t smb_mid;     /* multiplex id # */
#if 0
    uint8_t  smb_wct;     /* count of parameter words */
    uint16_t smb_vwv[];   /* variable # words of params */
    uint16_t smb_bcc;     /* # bytes of data following */
    uint8_t  smb_data[];  /* data bytes */
#endif
} SmbCoreHdr;

typedef struct _SmbLm10Hdr
{
    uint8_t  smb_idf[4];    /* contains 0xFF, 'SMB' */
    uint8_t  smb_com;       /* command code */
    uint8_t  smb_rcls;      /* error class */
    uint8_t  smb_reh;       /* reserved for future */
    uint16_t smb_err;       /* error code */
    uint8_t  smb_flg;       /* flags */
    uint16_t smb_res[7];    /* reserved for future */
    uint16_t smb_tid;       /* authenticated resource identifier */
    uint16_t smb_pid;       /* caller's process id */
    uint16_t smb_uid;       /* unauthenticated user id */
    uint16_t smb_mid;       /* multiplex id */
#if 0
    uint8_t  smb_wct;       /* count of 16-bit words that follow */
    uint16_t smb_vwv[];     /* variable number of 16-bit words */
    uint16_t smb_bcc;       /* count of bytes that follow */
    uint8_t  smb_buf[];     /* variable number of bytes */
#endif
} SmbLm10Hdr;

typedef struct _SmbLm20Hdr
{
    uint8_t  smb_idf[4];    /* contains 0xFF,’SMB’ */
    uint8_t  smb_com;       /* command code */
    uint8_t  smb_rcls;      /* error class */
    uint8_t  smb_reh;       /* reserved for future */
    uint16_t smb_err;       /* error code */
    uint8_t  smb_flg;       /* flags */
    uint16_t smb_flg2;      /* flags */
    uint16_t smb_res[6];    /* reserved for future */
    uint16_t smb_tid;       /* authenticated resource identifier */
    uint16_t smb_pid;       /* caller’s process id */
    uint16_t smb_uid;       /* authenticated user id */
    uint16_t smb_mid;       /* multiplex id */
#if 0
    uint8_t  smb_wct;       /* count of 16-bit words that follow */
    uint16_t smb_vwv[];     /* variable number of 16-bit words */
    uint16_t smb_bcc;       /* count of bytes that follow */
    uint8_t  smb_buf[];     /* variable number of bytes */
#endif
} SmbLm20Hdr;

typedef struct _SmbNtHdr
{
    uint8_t  smb_idf[4];            /* contains 0xFF, 'SMB' */
    uint8_t  smb_com;               /* command code */
    union {
        struct {
            uint8_t  smb_rcls;      /* dos error class */
            uint8_t  smb_reh;       /* reserved for future */
            uint16_t smb_err;       /* dos error code */
        } smb_doserr;
        uint32_t smb_nt_status;     /* nt status */
    } smb_status;
    uint8_t  smb_flg;               /* flags */
    uint16_t smb_flg2;              /* flags */
    uint16_t smb_res[6];            /* reserved for future */
    uint16_t smb_tid;               /* tree id */
    uint16_t smb_pid;               /* caller's process id */
    uint16_t smb_uid;               /* authenticated user id */
    uint16_t smb_mid;               /* multiplex id */
#if 0
    uint8_t  smb_wct;       /* count of 16-bit words that follow */
    uint16_t smb_vwv[];     /* variable number of 16-bit words */
    uint16_t smb_bcc;       /* count of bytes that follow */
    uint8_t  smb_buf[];     /* variable number of bytes */
#endif
} SmbNtHdr;

/* Common fields to all commands */
typedef struct _SmbCommon
{
    uint8_t smb_wct;

} SmbCommon;

/* Common fields to all AndX commands */
typedef struct _SmbAndXCommon
{
    uint8_t  smb_wct;
    uint8_t  smb_com2;     /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;     /* reserved (must be zero) */
    uint16_t smb_off2;     /* offset (from SMB hdr start) to next cmd (@smb_wct) */

} SmbAndXCommon;

/* For server empty respones indicating client error */
typedef struct _SmbEmptyCom
{
    uint8_t  smb_wct;    /* value = 0 */
    uint16_t smb_bcc;    /* value = 0 */

} SmbEmptyCom;

/********************************************************************
 * Negotiate Protocol :: smb_com = SMB_COM_NEGPROT
 *
 ********************************************************************/
typedef struct _SmbCore_NegotiateProtocolReq   /* smb_wct = 0 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_bcc;     /* min = 2 */

#if 0
    uint8_t  smb_fmt;     /* Dialect -- 02 */
    uint8_t  smb_buf[];   /* dialect0 */
            .
            .
            .
    uint8_t  smb_fmt;     /* Dialect -- 02 */
    uint8_t  smb_bufn[];  /* dialectn*/
#endif
} SmbCore_NegotiateProtocolReq;

/* This is the Core Protocol response */
typedef struct _SmbCore_NegotiateProtocolResp    /* smb_wct = 1 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_idx;     /* index */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_NegotiateProtocolResp;

/* This is the Lanman 1.0 response */
typedef struct _SmbLm10_NegotiateProtocolResp   /* smb_wct = 13 */
{
    uint8_t  smb_wct;       /* count of 16-bit words that follow */
    uint16_t smb_index;     /* index identifying dialect selected */
    uint16_t smb_secmode;   /* security mode:
                               bit 0, 1 = User level, 0 = Share level
                               bit 1, 1 = encrypt passwords, 0 = do not encrypt passwords */
    uint16_t smb_maxxmt;    /* max transmit buffer size server supports, 1K min */
    uint16_t smb_maxmux;    /* max pending multiplexed requests server supports */
    uint16_t smb_maxvcs;    /* max VCs per server/consumer session supported */
    uint16_t smb_blkmode;   /* block read/write mode support:
                               bit 0, Read Block Raw supported (65535 bytes max)
                               bit 1, Write Block Raw supported (65535 bytes max) */
    uint32_t smb_sesskey;   /* Session Key (unique token identifying session) */
    uint16_t smb_srv_time;  /* server's current time (hhhhh mmmmmm xxxxx) */
    uint16_t smb_srv_tzone; /* server's current data (yyyyyyy mmmm ddddd) */
    uint32_t smb_rsvd;      /* reserved */
    uint16_t smb_bcc;       /* value = (size of smb_cryptkey) */
#if 0
    uint8_t  smb_cryptkey[];  /* Key used for password encryption */
#endif
} SmbLm10_NegotiateProtocolResp;

/* This is the Lanman 2.1 response */
typedef struct _SmbLm21_NegotiateProtocolResp     /* smb_wct = 13 */
{
    uint8_t  smb_wct;         /* count of 16-bit words that follow */
    uint16_t smb_index;       /* index identifying dialect selected */
    uint16_t smb_secmode;     /* security mode:
                                 bit 0, 1 = User level, 0 = Share level
                                 bit 1, 1 = encrypt passwords, 0 = do not encrypt passwords */
    uint16_t smb_maxxmt;      /* max transmit buffer size server supports, 1K min */
    uint16_t smb_maxmux;      /* max pending multiplexed requests server supports */
    uint16_t smb_maxvcs;      /* max VCs per server/consumer session supported */
    uint16_t smb_blkmode;     /* block read/write mode support:
                                 bit 0, Read Block Raw supported (65535 bytes max)
                                 bit 1, Write Block Raw supported (65535 bytes max) */
    uint32_t smb_sesskey;     /* Session Key (unique token identifying session) */
    uint16_t smb_srv_time;    /* server's current time (hhhhh mmmmmm xxxxx) */
    uint16_t smb_srv_tzone;   /* server's current data (yyyyyyy mmmm ddddd) */
    uint16_t smb_cryptkeylen; /* length of smb_cryptkey */
    uint16_t smb_rsvd;        /* reserved */
    uint16_t smb_bcc;       /* value = (size of smb_cryptkey) */
#if 0
    uint8_t  smb_cryptkey[];  /* Key used for password encryption */
    uint8_t  smb_domain[]     /* Null terminated server domain */
#endif
} SmbLm21_NegotiateProtocolResp;

/* This is the NT Lanman 1.0 response */
typedef struct _SmbNt10_NegotiateProtocolResp     /* smb_wct = 17 */
{
    uint8_t  smb_wct;         /* count of 16-bit words that follow */
    uint16_t smb_index;       /* index identifying dialect selected */
    uint8_t  smb_secmode;     /* security mode:
                                 bit 0, 1 = User level, 0 = Share level
                                 bit 1, 1 = encrypt passwords, 0 = do not encrypt passwords */
    uint16_t smb_maxmux;      /* max pending multiplexed requests server supports */
    uint16_t smb_maxvcs;      /* max VCs per server/consumer session supported */
    uint32_t smb_maxbuf;      /* maximum buffer size supported */
    uint32_t smb_maxraw;      /* maximum raw buffer size supported */
    uint32_t smb_sesskey;     /* Session Key (unique token identifying session) */
    uint32_t smb_cap;         /* capabilities */
    struct {
        uint32_t low_time;
        int32_t  high_time;
    } smb_srv_time;           /* server time */
    uint16_t smb_srv_tzone;   /* server's current data (yyyyyyy mmmm ddddd) */
    uint8_t  smb_rsvd;        /* reserved */
    uint16_t smb_bcc;         /* value = (size of smb_cryptkey) */
#if 0
    uint8_t  smb_cryptkey[];  /* Key used for password encryption */
#endif
} SmbNt10_NegotiateProtocolResp;

/********************************************************************
 * Session Setup AndX :: smb_com = SMB_COM_SESS_SETUP_ANDX
 *
 ********************************************************************/
typedef struct _SmbLm10_SessionSetupAndXReq   /* smb_wct = 10 */
{
    uint8_t  smb_wct;      /* count of 16-bit words that follow */
    uint8_t  smb_com2;     /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;     /* reserved (must be zero) */
    uint16_t smb_off2;     /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bufsize;  /* the consumers max buffer size */
    uint16_t smb_mpxmax;   /* actual maximum multiplexed pending requests */
    uint16_t smb_vc_num;   /* 0 = first (only), non zero - additional VC number */
    uint32_t smb_sesskey;  /* Session Key (valid only if smb_vc_num != 0) */
    uint16_t smb_apasslen; /* size of account password (smb_apasswd) */
    uint32_t smb_rsvd;     /* reserved */
    uint16_t smb_bcc;      /* minimum value = 0 */
#if 0
    uint8_t  smb_apasswd[*];  /* account password (* = smb_apasslen value) */
    uint8_t  smb_aname[];     /* account name string */
#endif
} SmbLm10_SessionSetupAndXReq;

typedef struct _SmbLm10_SessionSetupAndXResp   /* smb_wct = 3 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint8_t  smb_com2;    /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;    /* reserved (pad to word) */
    uint16_t smb_off2;    /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_action;  /* request mode:
                             bit0 = Logged in successfully - BUT as GUEST */
    uint16_t smb_bcc;     /* value = 0 */

} SmbLm10_SessionSetupAndXResp;

/* Extended request as defined in LM 2.0 document */
typedef struct _SmbLm20_SessionSetupAndXReq   /* smb_wct = 10 */
{
    uint8_t  smb_wct;         /* count of 16-bit words that follow */
    uint8_t  smb_com2;        /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;        /* reserved (must be zero) */
    uint16_t smb_off2;        /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bufsize;     /* the consumers max buffer size */
    uint16_t smb_mpxmax;      /* actual maximum multiplexed pending requests */
    uint16_t smb_vc_num;      /* 0 = first (only), non zero - additional VC number */
    uint32_t smb_sesskey;     /* Session Key (valid only if smb_vc_num != 0) */
    uint16_t smb_apasslen;    /* size of account password (smb_apasswd) */
    uint16_t smb_encryptlen;  /* size of encryption key (smb_encrypt) */
    uint16_t smb_encryptoff;  /* offet (from SMB hdr start) to smb_encrypt */
    uint16_t smb_bcc;         /* minimum value = 0 */
#if 0
    uint8_t  smb_apasswd[*];  /* account password (* = smb_apasslen value) */
    uint8_t  smb_aname[];     /* account name string */
    uint8_t  smb_encrypt[*];  /* encryption key. (* = smb_encryptlen value) */
#endif
} SmbLm20_SessionSetupAndXReq;

/* Extended response as defined in LM 2.0 document */
typedef struct _SmbLm20_SessionSetupAndXResp   /* smb_wct = 3 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint8_t  smb_com2;    /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;    /* reserved (pad to word) */
    uint16_t smb_off2;    /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_action;  /* request mode:
                             bit0 = Logged in successfully - BUT as GUEST */
    uint16_t smb_bcc;     /* min value = 0 */
#if 0
    smb_encresp[];        /* server response to request encryption key */
#endif
} SmbLm20_SessionSetupAndXResp;

/* Extended request as defined in LM 2.1 document */
typedef struct _SmbLm21_SessionSetupAndXReq   /* smb_wct = 10 */
{
    uint8_t  smb_wct;         /* count of 16-bit words that follow */
    uint8_t  smb_com2;        /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;        /* reserved (must be zero) */
    uint16_t smb_off2;        /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bufsize;     /* the consumers max buffer size */
    uint16_t smb_mpxmax;      /* actual maximum multiplexed pending requests */
    uint16_t smb_vc_num;      /* 0 = first (only), non zero - additional VC number */
    uint32_t smb_sesskey;     /* Session Key (valid only if smb_vc_num != 0) */
    uint16_t smb_apasslen;    /* size of account password (smb_apasswd) */
    uint32_t smb_rsvd;        /* reserved */
    uint16_t smb_bcc;         /* minimum value = 0 */
#if 0
    uint8_t  smb_apasswd[*];  /* account password (* = smb_apasslen value) */
    uint8_t  smb_aname[];     /* account name string */
    uint8_t  smb_domain[];    /* name of domain that client was authenticated on */
    uint8_t  smb_mativeos[];  /* native operation system of client */
    uint8_t  smb_nativelm[];  /* native LAN Manager type */
#endif
} SmbLm21_SessionSetupAndXReq;

/* Extended response as defined in LM 2.1 document */
typedef struct _SmbLm21_SessionSetupAndXResp   /* smb_wct = 3 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_action;     /* request mode:
                                bit0 = Logged in successfully - BUT as GUEST */
    uint16_t smb_bcc;        /* min value = 0 */
#if 0
    uint8_t smb_nativeos[];  /* server's native operating system */
    uint8_t smb_nativelm[];  /* server's native LM type */
#endif
} SmbLm21_SessionSetupAndXResp;

/* Extended request as defined in NT LM 1.0 document */
typedef struct _SmbNt10_SessionSetupAndXReq   /* smb_wct = 13 */
{
    uint8_t  smb_wct;         /* count of 16-bit words that follow */
    uint8_t  smb_com2;        /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;        /* reserved (must be zero) */
    uint16_t smb_off2;        /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bufsize;     /* the consumers max buffer size */
    uint16_t smb_mpxmax;      /* actual maximum multiplexed pending requests */
    uint16_t smb_vc_num;      /* 0 = first (only), non zero - additional VC number */
    uint32_t smb_sesskey;     /* Session Key (valid only if smb_vc_num != 0) */
    uint16_t smb_ci_passlen;  /* case insensitive password length */
    uint16_t smb_cs_passlen;  /* case sensitive password length */
    uint32_t smb_rsvd;        /* reserved */
    uint32_t smb_cap;         /* capabilities */
    uint16_t smb_bcc;         /* minimum value = 0 */
#if 0
    uint8_t  smb_ci_passwd[*];  /* case insensitive password (* = smb_ci_passlen) */
    uint8_t  smb_cs_passwd[*];  /* case sensitive password (* = smb_cs_passlen) */
    uint8_t  smb_aname[];       /* ascii or unicode account name string */
    uint8_t  smb_domain[];      /* ascii or unicode name of domain that client was authenticated on */
    uint8_t  smb_nativeos[];    /* ascii or unicode native operation system of client */
    uint8_t  smb_nativelm[];    /* ascii or unicode native LAN Manager type */
#endif
} SmbNt10_SessionSetupAndXReq;

/* Extended request for security blob */
typedef struct _SmbNt10_SessionSetupAndXReq12   /* smb_wct = 12 */
{
    uint8_t  smb_wct;         /* count of 16-bit words that follow */
    uint8_t  smb_com2;        /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;        /* reserved (must be zero) */
    uint16_t smb_off2;        /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bufsize;     /* the consumers max buffer size */
    uint16_t smb_mpxmax;      /* actual maximum multiplexed pending requests */
    uint16_t smb_vc_num;      /* 0 = first (only), non zero - additional VC number */
    uint32_t smb_sesskey;     /* Session Key (valid only if smb_vc_num != 0) */
    uint16_t smb_blob_len;    /* length of security blob */
    uint32_t smb_rsvd;        /* reserved */
    uint32_t smb_cap;         /* capabilities */
    uint16_t smb_bcc;         /* minimum value = 0 */
#if 0
    uint8_t  smb_blob[];        /* security blob */
    uint8_t  smb_nativeos[];    /* ascii or unicode native operation system of client */
    uint8_t  smb_nativelm[];    /* ascii or unicode native LAN Manager type */
#endif
} SmbNt10_SessionSetupAndXReq12;

/* Extended response as defined in NT LM 1.0 document */
typedef struct _SmbNt10_SessionSetupAndXResp   /* smb_wct = 3 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_action;     /* request mode:
                                bit0 = Logged in successfully - BUT as GUEST */
    uint16_t smb_bcc;        /* min value = 0 */
#if 0
    uint8_t  smb_nativeos[];  /* ascii or unicode server's native operating system */
    uint8_t  smb_nativelm[];  /* ascii or unicode server's native LM type */
    uint8_t  smb_domain[];    /* ascii or unicode logon domain of the user */
#endif
} SmbNt10_SessionSetupAndXResp;

/* Extended response for security blob */
typedef struct _SmbNt10_SessionSetupAndXResp4   /* smb_wct = 4 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_action;     /* request mode:
                                bit0 = Logged in successfully - BUT as GUEST */
    uint16_t smb_blob_len;   /* length of security blob */
    uint16_t smb_bcc;        /* min value = 0 */
#if 0
    uint8_t  smb_blob[];        /* security blob */
    uint8_t  smb_nativeos[];  /* ascii or unicode server's native operating system */
    uint8_t  smb_nativelm[];  /* ascii or unicode server's native LM type */
    uint8_t  smb_domain[];    /* ascii or unicode logon domain of the user */
#endif
} SmbNt10_SessionSetupAndXResp4;


/********************************************************************
 * Logoff AndX :: smb_com = SMB_COM_LOGOFF_ANDX
 *
 * Valid smb_com2:
 *  Session Setup AndX
 *
 ********************************************************************/
typedef struct _SmbLm20_LogoffAndXReq    /* smb_wct = 2 */
{
    uint8_t  smb_wct;    /* count of 16-bit words that follow */
    uint8_t  smb_com2;   /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;   /* reserved (must be zero) */
    uint16_t smb_off2;   /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bcc;    /* value = 0 */

} SmbLm20_LogoffAndXReq;

typedef struct _SmbLm20_LogoffAndXResp    /* smb_wct = 2 */
{
    uint8_t  smb_wct;    /* count of 16-bit words that follow */
    uint8_t  smb_com2;   /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;   /* reserved (pad to word) */
    uint16_t smb_off2;   /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bcc;    /* value = 0 */

} SmbLm20_LogoffAndXResp;


/*********************************************************************
 * Tree Connect :: smb_com = SMB_COM_TREE_CON
 *
 *********************************************************************/
typedef struct _SmbCore_TreeConnectReq  /* smb_wct = 0 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_bcc;     /* min = 4 */

#if 0
    uint8_t  smb_fmt;     /* ASCII -- 04 */
    uint8_t  smb_buf[];   /* path/username */
    uint8_t  smb_fmt2;    /* ASCII -- 04 */
    uint8_t  smb_buf2[];  /* password */
    uint8_t  smb_fmt3;    /* ASCII -- 04 */
    uint8_t  smb_buf3[];  /* dev name (<device> or LPT1) */
#endif
} SmbCore_TreeConnectReq;

typedef struct _SmbCore_TreeConnectResp  /* smb_wct = 2 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_xmit;    /* max xmit size */
    uint16_t smb_tid;     /* tree id */
    uint16_t smb_bcc;

} SmbCore_TreeConnectResp;


/*********************************************************************
 * Tree Connect AndX :: smb_com = SMB_COM_TREE_CON_ANDX
 *
 *********************************************************************/
typedef struct _SmbLm10_TreeConnectAndXReq   /* smb_wct = 4 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;       /* reserved (must be zero) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_flags;      /* additional information:
                                bit 0 - if set, disconnect TID in current smb_tid */
    uint16_t smb_spasslen;   /* length of smb_spasswd */
    uint16_t smb_bcc;        /* minimum value = 3 */
#if 0
    uint8_t  smb_spasswd[*]; /* net-name password (* = smb_spasslen value) */
    uint8_t  smb_path[];     /* server name and net-name */
    uint8_t  smb_dev[];      /* service name string */
#endif
} SmbLm10_TreeConnectAndXReq;

typedef struct _SmbLm10_TreeConnectAndXResp    /* smb_wct = 2 */
{
    uint8_t  smb_wct;       /* count of 16-bit words that follow */
    uint8_t  smb_com2;      /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;      /* reserved (pad to word) */
    uint16_t smb_off2;      /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_bcc;       /* min value = 3 */
#if 0
    uint8_t  smb_service[]; /* service type connected to (string) */
#endif
} SmbLm10_TreeConnectAndXResp;

typedef struct _SmbLm21_TreeConnectAndXResp    /* smb_wct = 3 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_optsupp;    /* bit mask indicating advanced OS features available
                                bit0 = 1, exclusive search bits supported */
    uint16_t smb_bcc;        /* min value = 3 */
#if 0
    uint8_t  smb_nativefs[]; /* native file system for this connection */ 
#endif
} SmbLm21_TreeConnectAndXResp;


/********************************************************************
 * Tree Disconnect :: smb_com = SMB_COM_TREE_DIS 
 *
 ********************************************************************/
typedef struct _SmbCore_TreeDisconnectReq   /* smb_wct = 0 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_TreeDisconnectReq;

typedef struct _SmbCore_TreeDisconnectResp   /* smb_wct = 0 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_TreeDisconnectResp;


/********************************************************************
 * Open File :: smb_com = SMB_COM_OPEN
 *
 ********************************************************************/
typedef struct _SmbCore_OpenReq   /* smb_wct = 2 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_mode;    /* r/w/share */
    uint16_t smb_attr;    /* attribute */
    uint16_t smb_bcc;     /* min = 2 */

#if 0
    uint8_t  smb_fmt;     /* ASCII -- 04 */
    uint8_t  smb_buf[];   /* file pathname */
#endif
} SmbCore_OpenReq;

typedef struct _SmbCore_OpenResp   /* smb_wct = 7 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_fid;     /* file handle */
    uint16_t smb_attr;    /* attribute */
    uint16_t smb_tlow;    /* time1 low */
    uint16_t smb_thigh;   /* time1 high */
    uint16_t smb_fslow;   /* file size low */
    uint16_t smb_fshigh;  /* file size high */
    uint16_t smb_access;  /* access allowed */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_OpenResp;


/********************************************************************
 * Open AndX :: smb_com = SMB_COM_OPEN_ANDX
 *
 ********************************************************************/
typedef struct _SmbLm10_OpenAndXReq   /* smb_wct = 15 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;       /* reserved (must be zero) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_flags;      /* additional information:
                                bit 0 - if set, return additional information
                                bit 1 - if set, set single user total file lock (if only access)
                                bit 2 - if set, the server should notify the consumer on any
                                        action which can modify the file (delete, setattrib,
                                        rename, etc.). if not set, the server need only notify
                                        the consumer on another open request. This bit only has
                                        meaning if bit 1 is set. */
    uint16_t smb_mode;       /* file open mode */
    uint16_t smb_sattr;      /* search attributes */
    uint16_t smb_attr;       /* file attributes (for create) */
    uint32_t smb_time;       /* create time */
    uint16_t smb_ofun;       /* open function */
    uint32_t smb_size;       /* bytes to reserve on "create" or "truncate" */
    uint32_t smb_timeout;    /* max milliseconds to wait for resource to open */
    uint32_t smb_rsvd;       /* reserved (must be zero) */
    uint16_t smb_bcc;        /* minimum value = 1 */
#if 0
    uint8_t  smb_pathname[]; /* file pathname */
#endif
} SmbLm10_OpenAndXReq;

typedef struct _SmbLm10_OpenAndXResp   /* smb_wct = 15 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_fid;        /* file handle */
    uint16_t smb_attribute;  /* attributes of file or device */
    uint32_t smb_time;       /* last modification time */
    uint32_t smb_size;       /* current file size */
    uint16_t smb_access;     /* access permissions actually allowed */
    uint16_t smb_type;       /* file type */
    uint16_t smb_state;      /* state of IPC device (e.g. pipe) */
    uint16_t smb_action;     /* action taken */
    uint32_t smb_fileid;     /* server unique file id */
    uint16_t smb_rsvd;       /* reserved */
    uint16_t smb_bcc;        /* value = 0 */

} SmbLm10_OpenAndXResp;


/********************************************************************
 * NT Create AndX :: smb_com = SMB_COM_NT_CREATE_ANDX
 *
 ********************************************************************/
typedef struct _SmbNt10_NtCreateAndXReq   /* smb_wct = 24 */
{
    uint8_t  smb_wct;           /* count of 16-bit words that follow */
    uint8_t  smb_com2;          /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;          /* reserved (pad to word) */
    uint16_t smb_off2;          /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint8_t  smb_res;           /* reserved */
    uint16_t smb_name_len;      /* length of name of file */
    uint32_t smb_flags;         /* flags */
    uint32_t smb_root_fid;      /* fid for previously opened directory */
    uint32_t smb_access;        /* specifies the type of file access */
    uint64_t   smb_alloc_size;    /* initial allocation size of the file */
    uint32_t smb_file_attrs;    /* specifies the file attributes for the file */
    uint32_t smb_share_access;  /* the type of share access */
    uint32_t smb_create_disp;   /* actions to take if file does or does not exist */
    uint32_t smb_create_opts;   /* options used when creating or opening file */
    uint32_t smb_impersonation_level;  /* security impersonation level */
    uint8_t  smb_security_flags;  /* security flags */
    uint16_t smb_bcc;           /* byte count */
#if 0
    uint8_t * name[];    /* name of file to open */
#endif
} SmbNt10_NtCreateAndXReq;

/* Specification says word count is 34, but servers (Windows and
 * Samba) respond with word count of 42.  Wireshark decodes as word
 * count 34, but there is extra data at end of packet. */
typedef struct _SmbNt10_NtCreateAndXResp    /* smb_wct = 34 */
{
    uint8_t  smb_wct;
    uint8_t  smb_com2;
    uint8_t  smb_res2;
    uint16_t smb_off2;
    uint8_t  smb_oplock_level;
    uint16_t smb_fid;
    uint32_t smb_create_action;
    uint64_t   smb_creation_time;
    uint64_t   smb_last_access_time;
    uint64_t   smb_last_write_time;
    uint64_t   smb_change_time;
    uint32_t smb_file_attrs;
    uint64_t   smb_alloc_size;
    uint64_t   smb_eof;
    uint16_t smb_file_type;
    uint16_t smb_device_state;
    uint8_t  smb_directory;
    uint16_t smb_bcc;

} SmbNt10_NtCreateAndXResp;

/* XXX Trans2 Open? */
/* XXX NT Transact Create? */

/********************************************************************
 * Close File :: smb_com = SMB_COM_CLOSE
 *
 ********************************************************************/
typedef struct _SmbCore_CloseReq   /* smb_wct = 3 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_fid;     /* file handle */
    uint16_t smb_tlow;    /* time low */
    uint16_t smb_thigh;   /* time high */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_CloseReq;

typedef struct _SmbCore_CloseResp   /* smb_wct = 0 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_CloseResp;


/********************************************************************
 * Write :: smb_com = SMB_COM_WRITE
 *
 ********************************************************************/
typedef struct _SmbCore_WriteReq   /* smb_wct = 5 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_fid;     /* file handle */
    uint16_t smb_cnt;     /* count of bytes */
    uint16_t smb_olow;    /* offset low */
    uint16_t smb_ohigh;   /* offset high */
    uint16_t smb_left;    /* count left */
    uint16_t smb_bcc;     /* length of data + 3 */

#if 0
    uint16_t smb_fmt;     /* Data Block -- 01 */
    uint16_t smb_dlen;    /* length of data */
    uint8_t smb_buf[];    /* data */
#endif
} SmbCore_WriteReq;

typedef struct _SmbCore_WriteResp   /* smb_wct = 1 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_cnt;     /* count */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_WriteResp;


/********************************************************************
 * Transaction :: smb_com = SMB_COM_TRANS
 *
 ********************************************************************/
typedef struct _SmbLm10_TransactionReq   /* smb_wct = 14 + value of smb_suwcnt */
{
    uint8_t  smb_wct;      /* count of 16-bit words that follow */
    uint16_t smb_tpscnt;   /* total number of parameter bytes being sent */
    uint16_t smb_tdscnt;   /* total number of data bytes being sent */
    uint16_t smb_mprcnt;   /* max number of parameter bytes to return */
    uint16_t smb_mdrcnt;   /* max number of data bytes to return */
    uint8_t  smb_msrcnt;   /* max number of setup words to return */
    uint8_t  smb_rsvd;     /* reserved (pad above to word) */
    uint16_t smb_flags;    /* additional information:
                              bit 0 - if set, also disconnect TID in smb_tid
                              bit 1 - if set, transaction is one way (no final response) */
    uint32_t smb_timeout;  /* number of milliseconds to wait for completion */
    uint16_t smb_rsvd1;    /* reserved */
    uint16_t smb_pscnt;    /* number of parameter bytes being sent this buffer */
    uint16_t smb_psoff;    /* offset (from start of SMB hdr) to parameter bytes */
    uint16_t smb_dscnt;    /* number of data bytes being sent this buffer */
    uint16_t smb_dsoff;    /* offset (from start of SMB hdr) to data bytes */
    uint8_t  smb_suwcnt;   /* set up word count */
    uint8_t  smb_rsvd2;    /* reserved (pad above to word) */
#if 0
    uint16_t smb_setup[*]; /* variable number of set up words (* = smb_suwcnt) */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
    uint8_t  smb_name[];   /* name of transaction */
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_param[*]; /* param bytes (* = value of smb_pscnt) */
    uint8_t  smb_pad1[];   /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_dscnt) */
#endif
} SmbLm10_TransactionReq;

typedef struct _SmbLm10_TransactionInterimResp    /* smb_wct = 0 */
{
    uint8_t   smb_wct;      /* count of 16-bit words that follow */
    uint16_t  smb_bcc;      /* must be 0 */

} SmbLm10_TransactionInterimResp;

typedef struct _SmbLm10_TransactionSecondaryReq   /* smb_wct = 8 */
{
    uint8_t  smb_wct;      /* count of 16-bit words that follow */
    uint16_t smb_tpscnt;   /* total number of parameter bytes being sent */
    uint16_t smb_tdscnt;   /* total number of data bytes being sent */
    uint16_t smb_pscnt;    /* number of parameter bytes being sent this buffer */
    uint16_t smb_psoff;    /* offset (from start of SMB hdr) to parameter bytes */
    uint16_t smb_psdisp;   /* byte displacement for these parameter bytes */
    uint16_t smb_dscnt;    /* number of data bytes being sent this buffer */
    uint16_t smb_dsoff;    /* offset (from start of SMB hdr) to data bytes */
    uint16_t smb_dsdisp;   /* byte displacement for these data bytes */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_param[*]; /* param bytes (* = value of smb_pscnt) */
    uint8_t  smb_pad1[];   /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_dscnt) */
#endif
} SmbLm10_TransactionSecondaryReq;

typedef struct _SmbLm10_TransactionResp   /* smb_wct = 10 + value of smb_suwcnt */
{
    uint8_t  smb_wct;      /* count of 16-bit words that follow */
    uint16_t smb_tprcnt;   /* total number of parameter bytes being returned */
    uint16_t smb_tdrcnt;   /* total number of data bytes being returned */
    uint16_t smb_rsvd;     /* reserved */
    uint16_t smb_prcnt;    /* number of parameter bytes being returned this buf */
    uint16_t smb_proff;    /* offset (from start of SMB hdr) to parameter bytes */
    uint16_t smb_prdisp;   /* byte displacement for these parameter bytes */
    uint16_t smb_drcnt;    /* number of data bytes being returned this buffer */
    uint16_t smb_droff;    /* offset (from start of SMB hdr) to data bytes */
    uint16_t smb_drdisp;   /* byte displacement for these data bytes */
    uint8_t  smb_suwcnt;   /* set up return word count */
    uint8_t  smb_rsvd1;    /* reserved (pad above to word) */
#if 0
    uint16_t smb_setup[*]; /* variable # of set up return words (* = smb_suwcnt) */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_param[*]; /* param bytes (* = value of smb_prcnt) */
    uint8_t  smb_pad1[];   /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_drcnt) */
#endif
} SmbLm10_TransactionResp;

typedef struct _SmbLm10_TransactNamedPipeReq    /* smb_wct = 16 */
{
    uint8_t  smb_wct;      /* count of 16-bit words that follow */
    uint16_t smb_tpscnt;   /* total number of parameter bytes being sent */
    uint16_t smb_tdscnt;   /* size of data to be written to pipe (if any) */
    uint16_t smb_mprcnt;   /* max number of parameter bytes to return */
    uint16_t smb_mdrcnt;   /* size of data to be read from pipe (if any) */
    uint8_t  smb_msrcnt;   /* value = 0 max number of setup words to return */
    uint8_t  smb_rsvd;     /* reserved (pad above to word) */
    uint16_t smb_flags;    /* additional information:
                              bit 0 - if set, also disconnect TID in smb_tid
                              bit 1 - not set, response is required */
    uint32_t smb_timeout;  /* (user defined) number of milliseconds to wait */
    uint16_t smb_rsvd1;    /* reserved */
    uint16_t smb_pscnt;    /* number of parameter bytes being sent this buffer */
    uint16_t smb_psoff;    /* offset (from start of SMB hdr) to parameter bytes */
    uint16_t smb_dscnt;    /* number of data bytes being sent this buffer */
    uint16_t smb_dsoff;    /* offset (from start of SMB hdr) to data bytes */
    uint8_t  smb_suwcnt;   /* value = 2 */
    uint8_t  smb_rsvd2;    /* reserved (pad above to word) */
    uint16_t smb_setup1;   /* function (defined below)
                              0x54 - CallNmPipe - open/write/read/close pipe
                              0x53 - WaitNmPipe - wait for pipe to be nonbusy
                              0x23 - PeekNmPipe - read but don’t remove data
                              0x21 - QNmPHandState - query pipe handle modes
                              0x01 - SetNmPHandState - set pipe handle modes
                              0x22 - QNmPipeInfo - query pipe attributes
                              0x26 - TransactNmPipe - write/read operation on pipe
                              0x11 - RawReadNmPipe - read pipe in "raw" (non message mode)
                              0x31 - RawWriteNmPipe - write pipe "raw" (non message mode) */
    uint16_t smb_setup2;   /* FID (handle) of pipe (if needed), or priority */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_name[];   /* "\PIPE\<name>0" */
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_param[*]; /* param bytes (* = value of smb_pscnt) */
    uint8_t  smb_pad1[];   /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_dscnt) */
#endif
} SmbLm10_TransactNamedPipeReq;

typedef struct _SmbLm10_TransactNamedPipeResp   /* smb_wct = 10 */
{
    uint8_t  smb_wct;      /* count of 16-bit words that follow */
    uint16_t smb_tprcnt;   /* total number of parameter bytes being returned */
    uint16_t smb_tdrcnt;   /* total number of data bytes being returned */
    uint16_t smb_rsvd;     /* reserved */
    uint16_t smb_prcnt;    /* number of parameter bytes being returned this buf */
    uint16_t smb_proff;    /* offset (from start of SMB hdr) to parameter bytes */
    uint16_t smb_prdisp;   /* byte displacement for these parameter bytes */
    uint16_t smb_drcnt;    /* number of data bytes being returned this buffer */
    uint16_t smb_droff;    /* offset (from start of SMB hdr) to data bytes */
    uint16_t smb_drdisp;   /* byte displacement for these data bytes */
    uint8_t  smb_suwcnt;   /* set up return word count */
    uint8_t  smb_rsvd1;    /* reserved (pad above to word) */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_param[*]; /* param bytes (* = value of smb_prcnt) */
    uint8_t  smb_pad1[];   /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_drcnt) */
#endif
} SmbLm10_TransactNamedPipeResp;


/********************************************************************
 * Write and Close :: smb_com = SMB_COM_WRITE_AND_CLOSE
 *
 ********************************************************************/
typedef struct _SmbLm10_WriteAndCloseReq6   /* smb_wct = 6 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_fid;     /* file handle (close after write) */
    uint16_t smb_count;   /* number of bytes to write */
    uint32_t smb_offset;  /* offset in file to begin write */
    uint32_t smb_mtime;   /* modification time */
    uint16_t smb_bcc;     /* 1 (for pad) + value of smb_count */
#if 0
    uint8_t  smb_pad;     /* force data to dword boundary */
    uint8_t  smb_data[];  /* data */
#endif
} SmbLm10_WriteAndCloseReq6;

typedef struct _SmbLm10_WriteAndCloseReq12   /* smb_wct = 12 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_fid;     /* file handle (close after write) */
    uint16_t smb_count;   /* number of bytes to write */
    uint32_t smb_offset;  /* offset in file to begin write */
    uint32_t smb_mtime;   /* modification time */
    uint32_t smb_rsvd1;   /* Optional */
    uint32_t smb_rsvd2;   /* Optional */
    uint32_t smb_rsvd3;   /* Optional */
    uint16_t smb_bcc;     /* 1 (for pad) + value of smb_count */
#if 0
    uint8_t  smb_pad;     /* force data to dword boundary */
    uint8_t  smb_data[];  /* data */
#endif
} SmbLm10_WriteAndCloseReq12;

typedef struct _SmbLm10_WriteAndCloseResp   /* smb_wct = 1 */
{
    uint8_t  smb_wct;    /* count of 16-bit words that follow */
    uint8_t  smb_count;  /* number of bytes written */
    uint16_t smb_bcc;    /* must be 0 */

} SmbLm10_WriteAndCloseResp;

/********************************************************************
 * Write Block Raw :: smb_com = SMB_COM_WRITE_BLOCK_RAW
 *
 ********************************************************************/
typedef struct _SmbLm10_WriteBlockRawReq
{
    uint8_t  smb_wct;      /* value = 12 */
    uint16_t smb_fid;      /* file handle */
    uint16_t smb_tcount;   /* total bytes (including this buf, 65,535 max ) */
    uint16_t smb_rsvd;     /* reserved */
    uint32_t smb_offset;   /* offset in file to begin write */
    uint32_t smb_timeout;  /* number of milliseconds to wait for completion */
    uint16_t smb_wmode;    /* write mode:
                              bit0 - complete write to disk and send final result response
                              bit1 - return smb_remaining (pipes/devices only) */
    uint32_t smb_rsvd2;    /* reserved */
    uint16_t smb_dsize;    /* number of data bytes this buffer (min value = 0) */
    uint16_t smb_doff;     /* offset (from start of SMB hdr) to data bytes */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_dsize) */
#endif
} SmbLm10_WriteBlockRawReq;

typedef struct _SmbLm10_WriteBlockRawFirstResp
{
    uint8_t  smb_wct;        /* value = 1 */
    uint16_t smb_remaining;  /* bytes remaining to be read (pipes/devices only) */
    uint16_t smb_bcc;        /* value = 0 */

} SmbLm10_WriteBlockRawFirstResp;

/* If write through or error :: smb_com = SMB_COM_WRITE_COMPLETE */
typedef struct _SmbLm10_WriteBlockRawFinalResp
{
    uint8_t  smb_wct;    /* value = 1 */
    uint16_t smb_count;  /* total number of bytes written */
    uint16_t smb_bcc;    /* value = 0 */

} SmbLm10_WriteBlockRawFinalResp;

typedef struct _SmbNt10_WriteBlockRawReq
{
    uint8_t  smb_wct;      /* value = 14 */
    uint16_t smb_fid;      /* file handle */
    uint16_t smb_tcount;   /* total bytes (including this buf, 65,535 max ) */
    uint16_t smb_rsvd;     /* reserved */
    uint32_t smb_offset;   /* offset in file to begin write */
    uint32_t smb_timeout;  /* number of milliseconds to wait for completion */
    uint16_t smb_wmode;    /* write mode:
                              bit0 - complete write to disk and send final result response
                              bit1 - return smb_remaining (pipes/devices only) */
    uint32_t smb_rsvd2;    /* reserved */
    uint16_t smb_dsize;    /* number of data bytes this buffer (min value = 0) */
    uint16_t smb_doff;     /* offset (from start of SMB hdr) to data bytes */
    uint32_t smb_off_high; /* high offset in file to begin write */
    uint16_t smb_bcc;      /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];    /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];  /* data bytes (* = value of smb_dsize) */
#endif
} SmbNt10_WriteBlockRawReq;


/********************************************************************
 * Write AndX :: smb_com = SMB_COM_WRITE_ANDX
 *
 ********************************************************************/
typedef struct _SmbLm10_WriteAndXReq   /* smb_wct = 12 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;       /* reserved (must be zero) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_fid;        /* file handle */
    uint32_t smb_offset;     /* offset in file to begin write */
    uint32_t smb_timeout;    /* number of milliseconds to wait for completion */
    uint16_t smb_wmode;      /* write mode:
                                bit0 - complete write before return (write through)
                                bit1 - return smb_remaining (pipes/devices only)
                                bit2 - use WriteRawNamedPipe (pipes only)
                                bit3 - this is the start of a message (pipes only) */
    uint16_t smb_countleft;  /* bytes remaining to write to satisfy user’s request */
    uint16_t smb_rsvd;       /* reserved */
    uint16_t smb_dsize;      /* number of data bytes in buffer (min value = 0) */
    uint16_t smb_doff;       /* offset (from start of SMB hdr) to data bytes */
    uint16_t smb_bcc;        /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];      /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];    /* data bytes (* = value of smb_dsize) */
#endif
} SmbLm10_WriteAndXReq;

typedef struct _SmbLm10_WriteAndXResp   /* smb_wct = 6 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_count;      /* number of bytes written */
    uint16_t smb_remaining;  /* bytes remaining to be read (pipes/devices only) */
    uint32_t smb_rsvd;       /* reserved */
    uint16_t smb_bcc;        /* value = 0 */

} SmbLm10_WriteAndXResp;

typedef struct _SmbNt10_WriteAndXReq   /* smb_wct = 14 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;       /* reserved (must be zero) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_fid;        /* file handle */
    uint32_t smb_off_low;    /* low offset in file to begin write */
    uint32_t smb_timeout;    /* number of milliseconds to wait for completion */
    uint16_t smb_wmode;      /* write mode:
                                bit0 - complete write before return (write through)
                                bit1 - return smb_remaining (pipes/devices only)
                                bit2 - use WriteRawNamedPipe (pipes only)
                                bit3 - this is the start of a message (pipes only) */
    uint16_t smb_countleft;  /* bytes remaining to write to satisfy user’s request */
    uint16_t smb_rsvd;       /* reserved */
    uint16_t smb_dsize;      /* number of data bytes in buffer (min value = 0) */
    uint16_t smb_doff;       /* offset (from start of SMB hdr) to data bytes */
    uint32_t smb_off_high;   /* high offset in file to begin write */
    uint16_t smb_bcc;        /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];      /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];    /* data bytes (* = value of smb_dsize) */
#endif
} SmbNt10_WriteAndXReq;


/********************************************************************
 * Read :: smb_com = SMB_COM_READ
 *
 ********************************************************************/
typedef struct _SmbCore_ReadReq   /* smb_wct = 5 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_fid;     /* file handle */
    uint16_t smb_cnt;     /* count of bytes */
    uint16_t smb_olow;    /* offset low */
    uint16_t smb_ohigh;   /* offset high */
    uint16_t smb_left;    /* count left */
    uint16_t smb_bcc;     /* must be 0 */

} SmbCore_ReadReq;

typedef struct _SmbCore_ReadResp   /* smb_wct = 5 */
{
    uint8_t  smb_wct;     /* count of 16-bit words that follow */
    uint16_t smb_cnt;     /* count */
    uint16_t smb_res[4];  /* reserved (MBZ) */
    uint16_t smb_bcc;     /* length of data + 3 */

#if 0
    uint8_t  smb_fmt;     /* Data Block -- 01 */
    uint16_t smb_dlen;    /* length of data */
    uint8_t  smb_buf[];   /* data */
#endif
} SmbCore_ReadResp;

/********************************************************************
 * Read Block Raw :: smb_com = SMB_COM_READ_BLOCK_RAW
 *
 ********************************************************************/
typedef struct _SmbLm10_ReadBlockRawReq   /* smb_wct = 8 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint16_t smb_fid;        /* file handle */
    uint32_t smb_offset;     /* offset in file to begin read */
    uint16_t smb_maxcnt;     /* max number of bytes to return (max 65,535) */
    uint16_t smb_mincnt;     /* min number of bytes to return (normally 0) */
    uint32_t smb_timeout;    /* number of milliseconds to wait for completion */
    uint16_t smb_rsvd;       /* reserved */
    uint16_t smb_bcc;        /* value = 0 */

} SmbLm10_ReadBlockRawReq;

typedef struct _SmbNt10_ReadBlockRawReq   /* smb_wct = 10 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint16_t smb_fid;        /* file handle */
    uint32_t smb_offset;     /* offset in file to begin read */
    uint16_t smb_maxcnt;     /* max number of bytes to return (max 65,535) */
    uint16_t smb_mincnt;     /* min number of bytes to return (normally 0) */
    uint32_t smb_timeout;    /* number of milliseconds to wait for completion */
    uint16_t smb_rsvd;       /* reserved */
    uint32_t smb_off_high;   /* high offset in file to begin write */
    uint16_t smb_bcc;        /* value = 0 */

} SmbNt10_ReadBlockRawReq;

/* Read block raw response is raw data wrapped in NetBIOS header */

/********************************************************************
 * Read AndX :: smb_com = SMB_COM_READ_ANDX
 *
 ********************************************************************/
typedef struct _SmbLm10_ReadAndXReq   /* smb_wct = 10 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;       /* reserved (must be zero) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_fid;        /* file handle */
    uint32_t smb_offset;     /* offset in file to begin read */
    uint16_t smb_maxcnt;     /* max number of bytes to return */
    uint16_t smb_mincnt;     /* min number of bytes to return */
    uint32_t smb_timeout;    /* number of milliseconds to wait for completion */
    uint16_t smb_countleft;  /* bytes remaining to satisfy user’s request */
    uint16_t smb_bcc;        /* value = 0 */

} SmbLm10_ReadAndXReq;

typedef struct _SmbLm10_ReadAndXResp    /* smb_wct = 12 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_res2;       /* reserved (pad to word) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_remaining;  /* bytes remaining to be read (pipes/devices only) */
    uint32_t smb_rsvd;       /* reserved */
    uint16_t smb_dsize;      /* number of data bytes (minimum value = 0) */
    uint16_t smb_doff;       /* offset (from start of SMB hdr) to data bytes */
    uint16_t smb_rsvd1;      /* reserved (These last 5 words are reserved in */
    uint32_t smb_rsvd2;      /* reserved order to make the ReadandX response */
    uint32_t smb_rsvd3;      /* reserved the same size as the WriteandX request) */
    uint16_t smb_bcc;        /* total bytes (including pad bytes) following */
#if 0
    uint8_t  smb_pad[];      /* (optional) to pad to word or dword boundary */
    uint8_t  smb_data[*];    /* data bytes (* = value of smb_dsize) */
#endif
} SmbLm10_ReadAndXResp;

typedef struct _SmbNt10_ReadAndXReq   /* smb_wct = 12 */
{
    uint8_t  smb_wct;        /* count of 16-bit words that follow */
    uint8_t  smb_com2;       /* secondary (X) command, 0xFF = none */
    uint8_t  smb_reh2;       /* reserved (must be zero) */
    uint16_t smb_off2;       /* offset (from SMB hdr start) to next cmd (@smb_wct) */
    uint16_t smb_fid;        /* file handle */
    uint32_t smb_off_low;    /* low offset in file to begin read */
    uint16_t smb_maxcnt;     /* max number of bytes to return */
    uint16_t smb_mincnt;     /* min number of bytes to return */
    uint32_t smb_timeout;    /* number of milliseconds to wait for completion */
    uint16_t smb_countleft;  /* bytes remaining to satisfy user’s request */
    uint32_t smb_off_high;   /* high offset in file to begin read */
    uint16_t smb_bcc;        /* value = 0 */

} SmbNt10_ReadAndXReq;

/********************************************************************
 * Rename File :: smb_com = SMB_COM_RENAME
 *
 ********************************************************************/
typedef struct _SmbCore_RenameReq  /* smb_wct = 1 */
{
    uint8_t  smb_wct;
    uint16_t smb_attrs;
    uint16_t smb_bcc;
#if 0
    uint8_t  smb_fmt;       /* ASCII -- 04 */
    uint8_t  smb_buf[];     /* old filename */
    uint8_t  smb_fmt2;      /* ASCII -- 04 */
    uint8_t  smb_buf2[];    /* new filename */
#endif
} SmbCore_RenameReq;

typedef struct _SmbCore_RenameResp  /* smb_wct = 0 */
{
    uint8_t  smb_wct;
    uint16_t smb_bcc;

} SmbCore_RenameResp;

#ifdef WIN32
#pragma pack(pop,smb_hdrs)
#else
#pragma pack()
#endif

/********************************************************************
 * Inline functions prototypes
 ********************************************************************/
static INLINE uint32_t NbssLen(const NbssHdr *);
static INLINE uint8_t NbssType(const NbssHdr *);
static INLINE uint16_t SmbNtohs(const uint16_t *);
static INLINE uint32_t SmbNtohl(const uint32_t *);
static INLINE uint16_t SmbHtons(const uint16_t *);
static INLINE uint32_t SmbHtonl(const uint32_t *);

static INLINE uint32_t SmbId(const SmbNtHdr *);
static INLINE uint32_t SmbNtStatus(const SmbNtHdr *);
static INLINE int SmbError(const SmbNtHdr *);
static INLINE int SmbType(const SmbNtHdr *);
static INLINE uint8_t SmbCom(const SmbNtHdr *);
static INLINE int SmbUnicode(const SmbNtHdr *);
static INLINE uint16_t SmbUid(const SmbNtHdr *);
static INLINE uint16_t SmbTid(const SmbNtHdr *);
static INLINE uint16_t SmbPid(const SmbNtHdr *);
static INLINE uint16_t SmbMid(const SmbNtHdr *);

static INLINE uint8_t SmbWct(const SmbCommon *);
static INLINE uint16_t SmbBcc(const uint8_t *, uint16_t);
static INLINE uint8_t SmbAndXCom2(const SmbAndXCommon *);
static INLINE uint16_t SmbAndXOff2(const SmbAndXCommon *);
static INLINE uint8_t SmbEmptyComWct(const SmbEmptyCom *);
static INLINE uint16_t SmbEmptyComBcc(const SmbEmptyCom *);

static INLINE uint16_t SmbGet16(const uint8_t *);
static INLINE uint32_t SmbGet32(const uint8_t *);

static INLINE uint16_t SmbLm10_TreeConAndXReqPassLen(const SmbLm10_TreeConnectAndXReq *);

static INLINE uint16_t SmbCore_OpenRespFid(const SmbCore_OpenResp *);
static INLINE uint16_t SmbLm10_OpenAndXRespFid(const SmbLm10_OpenAndXResp *);

static INLINE uint16_t SmbNt10_NtCreateAndXRespFid(const SmbNt10_NtCreateAndXResp *);

static INLINE uint16_t SmbCore_CloseReqFid(const SmbCore_CloseReq *);

static INLINE uint16_t SmbCore_WriteReqFid(const SmbCore_WriteReq *);

static INLINE uint16_t SmbLm10_WriteAndCloseReqFid(const SmbLm10_WriteAndCloseReq6 *);
static INLINE uint16_t SmbLm10_WriteAndCloseReqCount(const SmbLm10_WriteAndCloseReq6 *);

static INLINE uint16_t SmbLm10_WriteAndXReqFid(const SmbLm10_WriteAndXReq *);
static INLINE uint16_t SmbLm10_WriteAndXReqDoff(const SmbLm10_WriteAndXReq *);
static INLINE uint16_t SmbLm10_WriteAndXReqDsize(const SmbLm10_WriteAndXReq *);

static INLINE uint16_t SmbLm10_TransactNamedPipeReqFunc(const SmbLm10_TransactNamedPipeReq *);
static INLINE uint16_t SmbLm10_TransactNamedPipeReqFid(const SmbLm10_TransactNamedPipeReq *);
static INLINE uint16_t SmbLm10_TransactNamedPipeReqDoff(const SmbLm10_TransactNamedPipeReq *);
static INLINE uint16_t SmbLm10_TransactNamedPipeReqTotalDcnt(const SmbLm10_TransactNamedPipeReq *);
static INLINE uint16_t SmbLm10_TransactNamedPipeReqDcnt(const SmbLm10_TransactNamedPipeReq *);

static INLINE uint16_t SmbLm10_TransactSecReqDoff(const SmbLm10_TransactionSecondaryReq *);
static INLINE uint16_t SmbLm10_TransactSecReqTotalDcnt(const SmbLm10_TransactionSecondaryReq *);
static INLINE uint16_t SmbLm10_TransactSecReqDcnt(const SmbLm10_TransactionSecondaryReq *);
static INLINE uint16_t SmbLm10_TransactSecReqTotalDdisp(const SmbLm10_TransactionSecondaryReq *);

static INLINE uint16_t SmbLm10_TransactNamedPipeRespDoff(const SmbLm10_TransactNamedPipeResp *);
static INLINE uint16_t SmbLm10_TransactNamedPipeRespTotalDcnt(const SmbLm10_TransactNamedPipeResp *);
static INLINE uint16_t SmbLm10_TransactNamedPipeRespDcnt(const SmbLm10_TransactNamedPipeResp *);
static INLINE uint16_t SmbLm10_TransactNamedPipeRespTotalDdisp(const SmbLm10_TransactNamedPipeResp *);

static INLINE uint16_t SmbLm10_TransRespParamCnt(const SmbLm10_TransactionResp *);

static INLINE uint16_t SmbCore_ReadReqFid(const SmbCore_ReadReq *);

static INLINE uint16_t SmbLm10_ReadAndXReqFid(const SmbLm10_ReadAndXReq *);
static INLINE uint16_t SmbLm10_ReadAndXRespDoff(const SmbLm10_ReadAndXResp *);
static INLINE uint16_t SmbLm10_ReadAndXRespDsize(const SmbLm10_ReadAndXResp *);

static INLINE uint16_t SmbLm10_WriteBlockRawReqTotCount(const SmbLm10_WriteBlockRawReq *);
static INLINE uint16_t SmbLm10_WriteBlockRawReqFid(const SmbLm10_WriteBlockRawReq *);
static INLINE uint16_t SmbLm10_WriteBlockRawReqDoff(const SmbLm10_WriteBlockRawReq *);
static INLINE uint16_t SmbLm10_WriteBlockRawReqDsize(const SmbLm10_WriteBlockRawReq *);

static INLINE uint16_t SmbLm10_ReadBlockRawReqFid(const SmbLm10_ReadBlockRawReq *);

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
static INLINE uint32_t NbssLen(const NbssHdr *nb)
{
    /* Treat first bit of flags as the upper byte to length */
    return ((nb->flags & 0x01) << 16) | ntohs(nb->length);
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
static INLINE uint8_t NbssType(const NbssHdr *nb)
{
    return nb->type;
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
static INLINE uint16_t SmbNtohs(const uint16_t *ptr)
{
    uint16_t value;

    if (ptr == NULL)
        return 0;

#ifdef WORDS_MUSTALIGN
    value = *((uint8_t *)ptr) << 8 | *((uint8_t *)ptr + 1);
#else
    value = *ptr;
#endif  /* WORDS_MUSTALIGN */

#ifdef WORDS_BIGENDIAN
    return ((value & 0xff00) >> 8) | ((value & 0x00ff) << 8);
#else
    return value;
#endif  /* WORDS_BIGENDIAN */
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
static INLINE uint32_t SmbNtohl(const uint32_t *ptr)
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

#ifdef WORDS_BIGENDIAN
    return ((value & 0xff000000) >> 24) | ((value & 0x00ff0000) >> 8)  |
           ((value & 0x0000ff00) << 8)  | ((value & 0x000000ff) << 24);
#else
    return value;
#endif  /* WORDS_BIGENDIAN */
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
static INLINE uint16_t SmbHtons(const uint16_t *ptr)
{
    return SmbNtohs(ptr);
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
static INLINE uint32_t SmbHtonl(const uint32_t *ptr)
{
    return SmbNtohl(ptr);
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
static INLINE uint32_t SmbId(const SmbNtHdr *hdr)
{
#ifdef WORDS_MUSTALIGN
    uint8_t *idf = (uint8_t *)hdr->smb_idf;
    return *idf << 24 | *(idf + 1) << 16 | *(idf + 2) << 8 | *(idf + 3);
#else
    return ntohl(*((uint32_t *)hdr->smb_idf));
#endif  /* WORDS_MUSTALIGN */
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
static INLINE uint32_t SmbNtStatus(const SmbNtHdr *hdr)
{
    return SmbNtohl(&hdr->smb_status.smb_nt_status);
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
static INLINE int SmbError(const SmbNtHdr *hdr)
{
    if (SmbNtohs(&hdr->smb_flg2) & SMB_FLG2__NT_CODES)
    {
        /* Nt status codes are being used.  First 2 bits indicate
         * severity. */
        switch (SmbNtStatus(hdr) >> 30)
        {
            case SMB_NT_STATUS_SEVERITY__SUCCESS:
            case SMB_NT_STATUS_SEVERITY__INFORMATIONAL:
            case SMB_NT_STATUS_SEVERITY__WARNING:
                return 0;
            case SMB_NT_STATUS_SEVERITY__ERROR:
                return 1;
            default:
                return 0;
        }
    }
    else
    {
        uint8_t dos_error_class = hdr->smb_status.smb_doserr.smb_rcls;
        if (dos_error_class != SMB_ERROR_CLASS__SUCCESS)
        {
            if ((dos_error_class == SMB_ERROR_CLASS__ERRDOS) &&
                (SmbNtohs(&hdr->smb_status.smb_doserr.smb_err) == SMB_ERRDOS__MORE_DATA))
                return 0;

            return 1;
        }
        else
        {
            return 0;
        }
    }
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
static INLINE int SmbType(const SmbNtHdr *hdr)
{
    if (hdr->smb_flg & SMB_FLG__TYPE)
        return SMB_TYPE__RESPONSE;

    return SMB_TYPE__REQUEST;
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
static INLINE uint8_t SmbCom(const SmbNtHdr *hdr)
{
    return hdr->smb_com;
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
static INLINE int SmbUnicode(const SmbNtHdr *hdr)
{
    return SmbNtohs(&hdr->smb_flg2) & SMB_FLG2__UNICODE;
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
static INLINE uint16_t SmbUid(const SmbNtHdr *hdr)
{
    return SmbNtohs(&hdr->smb_uid);
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
static INLINE uint16_t SmbTid(const SmbNtHdr *hdr)
{
    return SmbNtohs(&hdr->smb_tid);
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
static INLINE uint16_t SmbPid(const SmbNtHdr *hdr)
{
    return SmbNtohs(&hdr->smb_pid);
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
static INLINE uint16_t SmbMid(const SmbNtHdr *hdr)
{
    return SmbNtohs(&hdr->smb_mid);
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
static INLINE uint8_t SmbWct(const SmbCommon *hdr)
{
    return hdr->smb_wct;
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
static INLINE uint16_t SmbBcc(const uint8_t *ptr, uint16_t com_size)
{
    /* com_size must be at least the size of the command encasing */
    if (com_size < sizeof(SmbEmptyCom))
        return 0;

    return SmbNtohs((uint16_t *)(ptr + com_size - sizeof(uint16_t)));
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
static INLINE uint8_t SmbAndXCom2(const SmbAndXCommon *andx)
{
    return andx->smb_com2;
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
static INLINE uint16_t SmbAndXOff2(const SmbAndXCommon *andx)
{
    return SmbNtohs(&andx->smb_off2);
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
static INLINE uint8_t SmbEmptyComWct(const SmbEmptyCom *ec)
{
    return ec->smb_wct;
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
static INLINE uint16_t SmbEmptyComBcc(const SmbEmptyCom *ec)
{
    return SmbNtohs(&ec->smb_bcc);
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
static INLINE uint16_t SmbGet16(const uint8_t *ptr)
{
    return SmbNtohs((uint16_t *)ptr);
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
static INLINE uint32_t SmbGet32(const uint8_t *ptr)
{
    return SmbNtohl((uint32_t *)ptr);
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
static INLINE uint16_t SmbLm10_TreeConAndXReqPassLen(const SmbLm10_TreeConnectAndXReq *tcx)
{
    return SmbNtohs(&tcx->smb_spasslen);
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
static INLINE uint16_t SmbCore_OpenRespFid(const SmbCore_OpenResp *open)
{
    return SmbNtohs(&open->smb_fid);
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
static INLINE uint16_t SmbLm10_OpenAndXRespFid(const SmbLm10_OpenAndXResp *openx)
{
    return SmbNtohs(&openx->smb_fid);
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
static INLINE uint16_t SmbNt10_NtCreateAndXRespFid(const SmbNt10_NtCreateAndXResp *ntx)
{
    return SmbNtohs(&ntx->smb_fid);
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
static INLINE uint16_t SmbCore_CloseReqFid(const SmbCore_CloseReq *close)
{
    return SmbNtohs(&close->smb_fid);
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
static INLINE uint16_t SmbCore_WriteReqFid(const SmbCore_WriteReq *write)
{
    return SmbNtohs(&write->smb_fid);
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
static INLINE uint16_t SmbLm10_WriteAndCloseReqFid(const SmbLm10_WriteAndCloseReq6 *wc)
{
    return SmbNtohs(&wc->smb_fid);
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
static INLINE uint16_t SmbLm10_WriteAndCloseReqCount(const SmbLm10_WriteAndCloseReq6 *wc)
{
    return SmbNtohs(&wc->smb_count);
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
static INLINE uint16_t SmbLm10_WriteAndXReqFid(const SmbLm10_WriteAndXReq *writex)
{
    return SmbNtohs(&writex->smb_fid);
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
static INLINE uint16_t SmbLm10_WriteAndXReqDoff(const SmbLm10_WriteAndXReq *writex)
{
    return SmbNtohs(&writex->smb_doff);
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
static INLINE uint16_t SmbLm10_WriteAndXReqRemaining(const SmbLm10_WriteAndXReq *writex)
{
    return SmbNtohs(&writex->smb_countleft);
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
static INLINE uint32_t SmbLm10_WriteAndXReqOffset(const SmbLm10_WriteAndXReq *writex)
{
    return SmbNtohl(&writex->smb_offset);
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
static INLINE uint16_t SmbLm10_WriteAndXReqDsize(const SmbLm10_WriteAndXReq *writex)
{
    return SmbNtohs(&writex->smb_dsize);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeReqFunc(const SmbLm10_TransactNamedPipeReq *trans)
{
    return SmbNtohs(&trans->smb_setup1);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeReqFid(const SmbLm10_TransactNamedPipeReq *trans)
{
    return SmbNtohs(&trans->smb_setup2);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeReqDoff(const SmbLm10_TransactNamedPipeReq *trans)
{
    return SmbNtohs(&trans->smb_dsoff);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeReqTotalDcnt(const SmbLm10_TransactNamedPipeReq *trans)
{
    return SmbNtohs(&trans->smb_tdscnt);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeReqDcnt(const SmbLm10_TransactNamedPipeReq *trans)
{
    return SmbNtohs(&trans->smb_dscnt);
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
static INLINE uint16_t SmbLm10_TransactSecReqDoff(const SmbLm10_TransactionSecondaryReq *trans)
{
    return SmbNtohs(&trans->smb_dsoff);
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
static INLINE uint16_t SmbLm10_TransactSecReqTotalDcnt(const SmbLm10_TransactionSecondaryReq *trans)
{
    return SmbNtohs(&trans->smb_tdscnt);
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
static INLINE uint16_t SmbLm10_TransactSecReqDcnt(const SmbLm10_TransactionSecondaryReq *trans)
{
    return SmbNtohs(&trans->smb_dscnt);
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
static INLINE uint16_t SmbLm10_TransactSecReqTotalDdisp(const SmbLm10_TransactionSecondaryReq *trans)
{
    return SmbNtohs(&trans->smb_dsdisp);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeRespDoff(const SmbLm10_TransactNamedPipeResp *trans)
{
    return SmbNtohs(&trans->smb_droff);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeRespTotalDcnt(const SmbLm10_TransactNamedPipeResp *trans)
{
    return SmbNtohs(&trans->smb_tdrcnt);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeRespDcnt(const SmbLm10_TransactNamedPipeResp *trans)
{
    return SmbNtohs(&trans->smb_drcnt);
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
static INLINE uint16_t SmbLm10_TransactNamedPipeRespTotalDdisp(const SmbLm10_TransactNamedPipeResp *trans)
{
    return SmbNtohs(&trans->smb_drdisp);
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
static INLINE uint16_t SmbLm10_TransRespParamCnt(const SmbLm10_TransactionResp *trans)
{
    return SmbNtohs(&trans->smb_prcnt);
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
static INLINE uint16_t SmbCore_ReadReqFid(const SmbCore_ReadReq *read)
{
    return SmbNtohs(&read->smb_fid);
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
static INLINE uint16_t SmbLm10_ReadAndXReqFid(const SmbLm10_ReadAndXReq *readx)
{
    return SmbNtohs(&readx->smb_fid);
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
static INLINE uint16_t SmbLm10_ReadAndXRespDoff(const SmbLm10_ReadAndXResp *readx)
{
    return SmbNtohs(&readx->smb_doff);
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
static INLINE uint16_t SmbLm10_ReadAndXRespDsize(const SmbLm10_ReadAndXResp *readx)
{
    return SmbNtohs(&readx->smb_dsize);
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
static INLINE uint16_t SmbLm10_WriteBlockRawReqTotCount(const SmbLm10_WriteBlockRawReq *wbr)
{
    return SmbNtohs(&wbr->smb_tcount);
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
static INLINE uint16_t SmbLm10_WriteBlockRawReqFid(const SmbLm10_WriteBlockRawReq *wbr)
{
    return SmbNtohs(&wbr->smb_fid);
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
static INLINE uint16_t SmbLm10_WriteBlockRawReqDoff(const SmbLm10_WriteBlockRawReq *wbr)
{
    return SmbNtohs(&wbr->smb_doff);
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
static INLINE uint16_t SmbLm10_WriteBlockRawReqDsize(const SmbLm10_WriteBlockRawReq *wbr)
{
    return SmbNtohs(&wbr->smb_dsize);
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
static INLINE uint16_t SmbLm10_ReadBlockRawReqFid(const SmbLm10_ReadBlockRawReq *rbr)
{
    return SmbNtohs(&rbr->smb_fid);
}

#endif /* _SMB_H_ */

