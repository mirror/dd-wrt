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
 * Provides convenience functions for parsing and querying configuration.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _DCE2_CONFIG_H_
#define _DCE2_CONFIG_H_

#include "dce2_debug.h"
#include "dce2_utils.h"
#include "dce2_list.h"
#include "sf_types.h"
#include "sf_ip.h"
#include "sfrt.h"
#include "sf_snort_packet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_GNAME  "dcerpc2"
#define DCE2_SNAME  "dcerpc2_server"

#define DCE2_CFG_TOK__DASH           '-'
#define DCE2_CFG_TOK__UNDERSCORE     '_'
#define DCE2_CFG_TOK__QUOTE          '"'
#define DCE2_CFG_TOK__LIST_START     '['
#define DCE2_CFG_TOK__LIST_END       ']'
#define DCE2_CFG_TOK__OPT_SEP        ','
#define DCE2_CFG_TOK__LIST_SEP       ','
#define DCE2_CFG_TOK__PORT_RANGE     ':'
#define DCE2_CFG_TOK__OPNUM_RANGE    '-'
#define DCE2_CFG_TOK__DOT            '.'
#define DCE2_CFG_TOK__IP6_TET_SEP    ':'
#define DCE2_CFG_TOK__IP4_TET_SEP    '.'
#define DCE2_CFG_TOK__IP_PREFIX_SEP  '/'
#define DCE2_CFG_TOK__MINUS          '-'
#define DCE2_CFG_TOK__PLUS           '+'
#define DCE2_CFG_TOK__HEX_SEP        'x'
#define DCE2_CFG_TOK__HEX_OCT_START  '0'
#define DCE2_CFG_TOK__END            '\0'

#define DCE2_PORTS__MAX  (UINT16_MAX + 1)
#define DCE2_PORTS__MAX_INDEX  (DCE2_PORTS__MAX / 8)

#define DCE2_MEMCAP__DEFAULT  (100 * 1024)  /* 100 MB */

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_Policy
{
    DCE2_POLICY__NONE,
    DCE2_POLICY__WIN2000,
    DCE2_POLICY__WINXP,
    DCE2_POLICY__WINVISTA,
    DCE2_POLICY__WIN2003,
    DCE2_POLICY__WIN2008,
    DCE2_POLICY__WIN7,
    DCE2_POLICY__SAMBA,
    DCE2_POLICY__SAMBA_3_0_37,
    DCE2_POLICY__SAMBA_3_0_22,
    DCE2_POLICY__SAMBA_3_0_20,
    DCE2_POLICY__MAX

} DCE2_Policy;

typedef enum _DCE2_DetectFlag
{
    DCE2_DETECT_FLAG__NULL = 0x0000,
    DCE2_DETECT_FLAG__NONE = 0x0001,
    DCE2_DETECT_FLAG__SMB = 0x0002,
    DCE2_DETECT_FLAG__TCP = 0x0004,
    DCE2_DETECT_FLAG__UDP = 0x0008,
    DCE2_DETECT_FLAG__HTTP_PROXY = 0x0010,
    DCE2_DETECT_FLAG__HTTP_SERVER = 0x0020,
    DCE2_DETECT_FLAG__ALL = 0xffff

} DCE2_DetectFlag;

typedef enum _DCE2_EventFlag
{
    DCE2_EVENT_FLAG__NULL = 0x0000,
    DCE2_EVENT_FLAG__NONE = 0x0001,
    DCE2_EVENT_FLAG__MEMCAP = 0x0002,
    DCE2_EVENT_FLAG__SMB = 0x0004,
    DCE2_EVENT_FLAG__CO = 0x0008,
    DCE2_EVENT_FLAG__CL = 0x0010,
    DCE2_EVENT_FLAG__ALL = 0xffff

} DCE2_EventFlag;

typedef enum _DCE2_ValidSmbVersionFlag
{
    DCE2_VALID_SMB_VERSION_FLAG__NULL = 0x0000,
    DCE2_VALID_SMB_VERSION_FLAG__V1 = 0x0001,
    DCE2_VALID_SMB_VERSION_FLAG__V2 = 0x0002,
    DCE2_VALID_SMB_VERSION_FLAG__ALL = 0xffff

} DCE2_ValidSmbVersionFlag;

typedef enum _DCE2_SmbFingerprintFlag
{
    DCE2_SMB_FINGERPRINT__NONE = 0x0000,
    DCE2_SMB_FINGERPRINT__CLIENT = 0x0001,
    DCE2_SMB_FINGERPRINT__SERVER = 0x0002

} DCE2_SmbFingerprintFlag;

/* Whether an option is on or off: CS - configuration switch */
typedef enum _DCE2_CS
{
    DCE2_CS__DISABLED = 0,
    DCE2_CS__ENABLED

} DCE2_CS;

typedef enum _DCE2_SmbFileInspection
{
    DCE2_SMB_FILE_INSPECTION__OFF = 0,
    DCE2_SMB_FILE_INSPECTION__ON,
    DCE2_SMB_FILE_INSPECTION__ONLY

} DCE2_SmbFileInspection;

typedef enum _DCE2_WordCharPosition
{
    DCE2_WORD_CHAR_POSITION__START,
    DCE2_WORD_CHAR_POSITION__MIDDLE,
    DCE2_WORD_CHAR_POSITION__END

} DCE2_WordCharPosition;

typedef enum _DCE2_WordListState
{
    DCE2_WORD_LIST_STATE__START,
    DCE2_WORD_LIST_STATE__WORD_START,
    DCE2_WORD_LIST_STATE__QUOTE,
    DCE2_WORD_LIST_STATE__WORD,
    DCE2_WORD_LIST_STATE__WORD_END,
    DCE2_WORD_LIST_STATE__END

} DCE2_WordListState;

typedef enum _DCE2_ValueState
{
    DCE2_VALUE_STATE__START,
    DCE2_VALUE_STATE__MODIFIER,
    DCE2_VALUE_STATE__HEX_OR_OCT,
    DCE2_VALUE_STATE__DECIMAL,
    DCE2_VALUE_STATE__HEX_START,
    DCE2_VALUE_STATE__HEX,
    DCE2_VALUE_STATE__OCTAL

} DCE2_ValueState;

typedef enum _DCE2_PortListState
{
    DCE2_PORT_LIST_STATE__START,
    DCE2_PORT_LIST_STATE__PORT_START,
    DCE2_PORT_LIST_STATE__PORT_LO,
    DCE2_PORT_LIST_STATE__PORT_RANGE,
    DCE2_PORT_LIST_STATE__PORT_HI,
    DCE2_PORT_LIST_STATE__PORT_END,
    DCE2_PORT_LIST_STATE__END

} DCE2_PortListState;

typedef enum _DCE2_IpListState
{
    DCE2_IP_LIST_STATE__START,
    DCE2_IP_LIST_STATE__IP_START,
    DCE2_IP_LIST_STATE__IP_END,
    DCE2_IP_LIST_STATE__END

} DCE2_IpListState;

typedef enum _DCE2_IpState
{
    DCE2_IP_STATE__START,
    DCE2_IP_STATE__IP

} DCE2_IpState;

typedef enum _DCE2_IntType
{
    DCE2_INT_TYPE__INT8,
    DCE2_INT_TYPE__UINT8,
    DCE2_INT_TYPE__INT16,
    DCE2_INT_TYPE__UINT16,
    DCE2_INT_TYPE__INT32,
    DCE2_INT_TYPE__UINT32,
    DCE2_INT_TYPE__INT64,
    DCE2_INT_TYPE__UINT64

} DCE2_IntType;

/********************************************************************
 * Structures
 ********************************************************************/
/* Global configuration struct */
typedef struct _DCE2_GlobalConfig
{
    int disabled;
    uint32_t memcap;
    int event_mask;
    DCE2_CS dce_defrag;
    int max_frag_len;
    uint16_t reassemble_threshold;
    int smb_fingerprint_policy;
    bool legacy_mode;

} DCE2_GlobalConfig;

typedef struct _DCE2_SmbShare
{
    char *unicode_str;
    unsigned int unicode_str_len;
    char *ascii_str;
    unsigned int ascii_str_len;

} DCE2_SmbShare;

/* Server configuration struct */
typedef struct _DCE2_ServerConfig
{
    DCE2_Policy policy;

    uint8_t smb_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t tcp_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t udp_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t http_proxy_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t http_server_ports[DCE2_PORTS__MAX_INDEX];

    uint8_t auto_smb_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t auto_tcp_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t auto_udp_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t auto_http_proxy_ports[DCE2_PORTS__MAX_INDEX];
    uint8_t auto_http_server_ports[DCE2_PORTS__MAX_INDEX];

    uint8_t smb_max_chain;
    uint8_t smb2_max_compound;

    DCE2_CS autodetect_http_proxy_ports;

    DCE2_SmbFileInspection smb_file_inspection;
    int64_t smb_file_depth;

    DCE2_List *smb_invalid_shares;
    int valid_smb_versions_mask;

    /* Used when freeing from routing table */
    uint32_t ref_count;

} DCE2_ServerConfig;

typedef struct _DCE2_Config
{
    DCE2_GlobalConfig *gconfig;
    DCE2_ServerConfig *dconfig;
    table_t *sconfigs;
    uint32_t ref_count;

#ifdef DCE2_LOG_EXTRA_DATA
    uint32_t xtra_logging_smb_file_name_id;
#endif

} DCE2_Config;

/********************************************************************
 * Extern variables
 ********************************************************************/
extern DCE2_Config *dce2_eval_config;
extern tSfPolicyUserContextId dce2_config;
extern DCE2_Config *dce2_eval_config;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static inline uint32_t DCE2_GcMemcap(void);
static inline int DCE2_GcMaxFrag(void);
static inline uint16_t DCE2_GcMaxFragLen(void);
static inline int DCE2_GcAlertOnEvent(DCE2_EventFlag);
static inline int DCE2_GcReassembleEarly(void);
static inline uint16_t DCE2_GcReassembleThreshold(void);
static inline DCE2_CS DCE2_GcDceDefrag(void);
static inline bool DCE2_GcSmbFingerprintClient(void);
static inline bool DCE2_GcSmbFingerprintServer(void);

static inline DCE2_Policy DCE2_ScPolicy(const DCE2_ServerConfig *);
static inline int DCE2_ScIsDetectPortSet(const DCE2_ServerConfig *, const uint16_t, const DCE2_TransType);
static inline int DCE2_ScIsAutodetectPortSet(const DCE2_ServerConfig *, const uint16_t, const DCE2_TransType);
static inline DCE2_CS DCE2_ScAutodetectHttpProxyPorts(const DCE2_ServerConfig *);
static inline uint8_t DCE2_ScSmbMaxChain(const DCE2_ServerConfig *);
static inline DCE2_List * DCE2_ScSmbInvalidShares(const DCE2_ServerConfig *);
static inline bool DCE2_ScSmbFileInspection(const DCE2_ServerConfig *);
static inline bool DCE2_ScSmbFileInspectionOnly(const DCE2_ServerConfig *);
static inline int64_t DCE2_ScSmbFileDepth(const DCE2_ServerConfig *);
static inline uint8_t DCE2_ScSmb2MaxCompound(const DCE2_ServerConfig *);
static inline uint8_t DCE2_ScIsValidSmbVersion(const DCE2_ServerConfig *, DCE2_ValidSmbVersionFlag);

static inline bool DCE2_IsPortSet(const uint8_t *, const uint16_t);
static inline void DCE2_SetPort(uint8_t *, const uint16_t);
static inline void DCE2_SetPortRange(uint8_t *, uint16_t, uint16_t);
static inline void DCE2_ClearPorts(uint8_t *);

static inline int DCE2_IsWordChar(const char, const DCE2_WordCharPosition);
static inline int DCE2_IsGraphChar(const char);
static inline int DCE2_IsQuoteChar(const char);
static inline int DCE2_IsListSepChar(const char);
static inline int DCE2_IsOptEndChar(const char);
static inline int DCE2_IsSpaceChar(const char);
static inline int DCE2_IsConfigEndChar(const char);
static inline int DCE2_IsPortChar(const char);
static inline int DCE2_IsPortRangeChar(const char);
static inline int DCE2_IsListStartChar(const char);
static inline int DCE2_IsListEndChar(const char);
static inline int DCE2_IsIpChar(const char);
static inline DCE2_Ret DCE2_CheckAndSetMask(int, int *);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
void DCE2_GlobalConfigure(DCE2_Config *, char *);
void DCE2_ServerConfigure(struct _SnortConfig *, DCE2_Config *, char *);
int DCE2_CreateDefaultServerConfig(struct _SnortConfig *, DCE2_Config *, tSfPolicyId);
int DCE2_ScCheckTransports(DCE2_Config *);
const DCE2_ServerConfig * DCE2_ScGetConfig(const SFSnortPacket *);
int DCE2_ScIsPortSet(const DCE2_ServerConfig *, const uint16_t, const DCE2_TransType);
int DCE2_ScIsDetectPortSet(const DCE2_ServerConfig *, const uint16_t, const DCE2_TransType);
int DCE2_ScIsAutodetectPortSet(const DCE2_ServerConfig *, const uint16_t, const DCE2_TransType);
int DCE2_ScIsNoAutoPortSet(const DCE2_ServerConfig *, const uint16_t);
DCE2_Ret DCE2_ParseValue(char **, char *, void *, DCE2_IntType);
DCE2_Ret DCE2_GetValue(char *, char *, void *, int, DCE2_IntType, uint8_t);
DCE2_Ret DCE2_ParseIpList(char **, char *, DCE2_Queue *);
DCE2_Ret DCE2_ParseIp(char **, char *, sfcidr_t *);
DCE2_Ret DCE2_ParsePortList(char **, char *, uint8_t *);
void DCE2_FreeConfigs(tSfPolicyUserContextId config);
void DCE2_FreeConfig(DCE2_Config *);

/********************************************************************
 * Function: DCE2_GcMemcap()
 *
 * Convenience function for getting the memcap configured for
 * the preprocessor.
 *
 * Arguments: None
 *
 * Returns:
 *  uint32_t
 *      The memcap configured for the preprocessor.
 *
 ********************************************************************/
static inline uint32_t DCE2_GcMemcap(void)
{
    return dce2_eval_config->gconfig->memcap;
}

/********************************************************************
 * Function: DCE2_GcMaxFrag()
 *
 * Convenience function for checking if the maximum fragment length
 * was configured for the preprocessor.
 *
 * Arguments: None
 *
 * Returns:
 *  int
 *      1 if it was configured.
 *      0 if it was not configured.
 *
 ********************************************************************/
static inline int DCE2_GcMaxFrag(void)
{
    if (dce2_eval_config->gconfig->max_frag_len != DCE2_SENTINEL) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_GcMaxFragLen()
 *
 * Convenience function for getting the maximum fragment length
 * that is configured for the preprocessor.  If not configured,
 * just return the maximum the return value can hold.  One should
 * check if configured first.
 *
 * Arguments: None
 *
 * Returns:
 *  uint16_t
 *      The maximum fragment length configured.
 *      UINT16_MAX if not configured.
 *
 ********************************************************************/
static inline uint16_t DCE2_GcMaxFragLen(void)
{
    if (DCE2_GcMaxFrag())
        return (uint16_t)dce2_eval_config->gconfig->max_frag_len;
    return UINT16_MAX;
}

/********************************************************************
 * Function: DCE2_GcAlertOnEvent()
 *
 * Convenience function for determining if we are configured
 * to alert on a certain event type.
 *
 * Arguments:
 *  DCE2_EventFlag
 *      The event type to check to see if we are configured
 *      to alert on.
 *
 * Returns:
 *  int
 *      Non-zero if we are configured to alert on this event type.
 *      Zero if we are not configured to alert on this event type.
 *
 ********************************************************************/
static inline int DCE2_GcAlertOnEvent(DCE2_EventFlag eflag)
{
    return dce2_eval_config->gconfig->event_mask & eflag;
}

/********************************************************************
 * Function: DCE2_GcDceDefrag()
 *
 * Convenience function for determining if we are configured
 * to do DCE/RPC defragmentation.
 *
 * Arguments: None
 *
 * Returns:
 *  DCE2_CS
 *      DCE2_CS__ENABLED if we are configured to do DCE/RPC
 *          defragmentation.
 *      DCE2_CS__DISABLED if we are not configured to do DCE/RPC
 *          defragmentation.
 *
 ********************************************************************/
static inline DCE2_CS DCE2_GcDceDefrag(void)
{
    return dce2_eval_config->gconfig->dce_defrag;
}

/********************************************************************
 * Function: DCE2_GcReassembleEarly()
 *
 * Convenience function for checking if the reassemble threshold
 * was configured for the preprocessor.
 *
 * Arguments: None
 *
 * Returns:
 *  int
 *      1 if it was configured.
 *      0 if it was not configured.
 *
 ********************************************************************/
static inline int DCE2_GcReassembleEarly(void)
{
    if (dce2_eval_config->gconfig->reassemble_threshold > 0)
        return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_GcReassembleThreshold()
 *
 * Convenience function for getting the reassemble threshold
 * that is configured for the preprocessor.  If not configured,
 * just return the maximum the return value can hold.  One should
 * check if configured first.
 *
 * Arguments: None
 *
 * Returns:
 *  uint16_t
 *      The reassemble threshold configured.
 *      UINT16_MAX if not configured.
 *
 ********************************************************************/
static inline uint16_t DCE2_GcReassembleThreshold(void)
{
    if (DCE2_GcReassembleEarly())
        return dce2_eval_config->gconfig->reassemble_threshold;
    return UINT16_MAX;
}

/********************************************************************
 * Function: DCE2_GcSmbFingerprintClient()
 *
 * Convenience function for finding out if the preprocessor is
 * configured to fingerprint the client policy base off SMB
 * traffic.
 *
 * Arguments: None
 *
 * Returns:
 *  bool  - true if configure to fingerprint client, false if not
 *
 ********************************************************************/
static inline bool DCE2_GcSmbFingerprintClient(void)
{
    return dce2_eval_config->gconfig->smb_fingerprint_policy
        & DCE2_SMB_FINGERPRINT__CLIENT ? true : false;
}

/********************************************************************
 * Function: DCE2_GcSmbFingerprintServer()
 *
 * Convenience function for finding out if the preprocessor is
 * configured to fingerprint the server policy base off SMB
 * traffic.
 *
 * Arguments: None
 *
 * Returns:
 *  bool  - true if configure to fingerprint server, false if not
 *
 ********************************************************************/
static inline bool DCE2_GcSmbFingerprintServer(void)
{
    return dce2_eval_config->gconfig->smb_fingerprint_policy
        & DCE2_SMB_FINGERPRINT__SERVER ? true : false;
}

/********************************************************************
 * Function: DCE2_GcIsLegacyMode()
 *
 * Convenience function for getting whether it is in the legacy mode
 *
 * Arguments: None
 *
 * Returns:  true:  legacy mode
 *           false: normal mode
 *
 ********************************************************************/
static inline bool DCE2_GcIsLegacyMode(void)
{
	return dce2_eval_config->gconfig->legacy_mode;
}

/********************************************************************
 * Function: DCE2_ScPolicy()
 *
 * Convenience function for getting the policy the server
 * configuration is configured for.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *
 * Returns:
 *  DCE2_Policy
 *      The policy the server configuration is configured for.
 *      DCE2_POLICY__NONE if a NULL pointer is passed in.
 *
 ********************************************************************/
static inline DCE2_Policy DCE2_ScPolicy(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return DCE2_POLICY__NONE;
    return sc->policy;
}

/*********************************************************************
 * Function: DCE2_ScIsDetectPortSet()
 *
 * Determines if the server configuration is configured to detect
 * on the port and transport passed in.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *  const uint16_t
 *      The port to check.
 *  const DCE2_TransType
 *      The transport to check for the port.
 *
 * Returns:
 *  int
 *      1 if configured to detect on this port for the given
 *          transport.
 *      0 if not configured to detect on this port for the given
 *          transport, or if the server configuration passed in
 *          is NULL.
 *
 *********************************************************************/
static inline int DCE2_ScIsDetectPortSet(const DCE2_ServerConfig *sc, const uint16_t port,
                                         const DCE2_TransType ttype)
{
    const uint8_t *port_array;

    if (sc == NULL)
        return 0;

    switch (ttype)
    {
        case DCE2_TRANS_TYPE__SMB:
            port_array = sc->smb_ports;
            break;
        case DCE2_TRANS_TYPE__TCP:
            port_array = sc->tcp_ports;
            break;
        case DCE2_TRANS_TYPE__UDP:
            port_array = sc->udp_ports;
            break;
        case DCE2_TRANS_TYPE__HTTP_PROXY:
            port_array = sc->http_proxy_ports;
            break;
        case DCE2_TRANS_TYPE__HTTP_SERVER:
            port_array = sc->http_server_ports;
            break;
        default:
            return 0;
    }

    return DCE2_IsPortSet(port_array, port);
}

/*********************************************************************
 * Function: DCE2_ScIsAutodetectPortSet()
 *
 * Determines if the server configuration is configured to autodetect
 * on the port and transport passed in.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *  const uint16_t
 *      The port to check.
 *  const DCE2_TransType
 *      The transport to check for the port.
 *
 * Returns:
 *  int
 *      1 if configured to autodetect on this port for the given
 *          transport.
 *      0 if not configured to autodetect on this port for the given
 *          transport, or if the server configuration passed in
 *          is NULL.
 *
 *********************************************************************/
static inline int DCE2_ScIsAutodetectPortSet(const DCE2_ServerConfig *sc, const uint16_t port,
                                             const DCE2_TransType ttype)
{
    const uint8_t *port_array;

    if (sc == NULL)
        return 0;

    switch (ttype)
    {
        case DCE2_TRANS_TYPE__SMB:
            port_array = sc->auto_smb_ports;
            break;
        case DCE2_TRANS_TYPE__TCP:
            port_array = sc->auto_tcp_ports;
            break;
        case DCE2_TRANS_TYPE__UDP:
            port_array = sc->auto_udp_ports;
            break;
        case DCE2_TRANS_TYPE__HTTP_PROXY:
            port_array = sc->auto_http_proxy_ports;
            break;
        case DCE2_TRANS_TYPE__HTTP_SERVER:
            port_array = sc->auto_http_server_ports;
            break;
        default:
            return 0;
    }

    return DCE2_IsPortSet(port_array, port);
}

/********************************************************************
 * Function: DCE2_ScAutodetectHttpProxyPorts()
 *
 * Convenience function to determine if the server configuration
 * is configured to autodetect on all rpc over http proxy detect
 * ports.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *
 * Returns:
 *  DCE2_CS
 *      DCE2_CS__ENABLED if configured to autodetect on all rpc
 *          over http proxy ports.  This is also returned it the
 *          server configuration passed in is NULL.
 *      DCE2_CS__DISABLED if not configured to autodetect on all
 *          rpc over http proxy ports.
 *
 ********************************************************************/
static inline DCE2_CS DCE2_ScAutodetectHttpProxyPorts(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return DCE2_CS__ENABLED;
    return sc->autodetect_http_proxy_ports;
}

/********************************************************************
 * Function: DCE2_ScSmbMaxChain()
 *
 * Convenience function to get the SMB maximum amount of command
 * chaining allowed.  A value of 0 means unlimited.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *
 * Returns:
 *  uint8_t
 *      The value for the maximum amount of command chaining.
 *      0 is returned if the server configuration passed in is NULL.
 *
 ********************************************************************/
static inline uint8_t DCE2_ScSmbMaxChain(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return 0;
    return sc->smb_max_chain;
}

/********************************************************************
 * Function: DCE2_ScSmbInvalidShares()
 *
 * Returns the list of SMB invalid shares configured.  If no
 * shares were configured, this will return a NULL list.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *
 * Returns:
 *  DCE2_List *
 *      Pointer to the list containing the SMB invalid share
 *          strings.
 *      NULL if no shares were configured or the server
 *          configuration passed in is NULL.
 *
 ********************************************************************/
static inline DCE2_List * DCE2_ScSmbInvalidShares(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return NULL;
    return sc->smb_invalid_shares;
}

static inline bool DCE2_ScSmbFileInspection(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return false;
    return ((sc->smb_file_inspection == DCE2_SMB_FILE_INSPECTION__ON)
            || (sc->smb_file_inspection == DCE2_SMB_FILE_INSPECTION__ONLY));
}

static inline bool DCE2_ScSmbFileInspectionOnly(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return false;
    return sc->smb_file_inspection == DCE2_SMB_FILE_INSPECTION__ONLY;
}

static inline int64_t DCE2_ScSmbFileDepth(const DCE2_ServerConfig *sc)
{
    if (!DCE2_ScSmbFileInspection(sc)) return -1;
    return sc->smb_file_depth;
}

/********************************************************************
 * Function: DCE2_ScSmb2MaxChain()
 *
 * Convenience function to get the SMB maximum amount of command
 * compounding allowed.  A value of 0 means unlimited.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *
 * Returns:
 *  uint8_t
 *      The value for the maximum amount of command compounding.
 *      0 is returned if the server configuration passed in is NULL.
 *
 ********************************************************************/
static inline uint8_t DCE2_ScSmb2MaxCompound(const DCE2_ServerConfig *sc)
{
    if (sc == NULL) return 0;
    return sc->smb2_max_compound;
}

/********************************************************************
 * Function: DCE2_ScIsValidSmbVersion()
 *
 * Convenience function to check if an smb version flag is set.
 *
 * Arguments:
 *  const DCE2_ServerConfig *
 *      Pointer to the server configuration to check.
 *  const DCE2_ValidSmbVersionFlag
 *      The version flag to test
 *
 * Returns:
 *  int
 *      non-zero if the flag is set
 *      0 if the flag is not set
 *
 ********************************************************************/
static inline uint8_t DCE2_ScIsValidSmbVersion(
        const DCE2_ServerConfig *sc, DCE2_ValidSmbVersionFlag vflag)
{
    if (sc == NULL) return 0;
    return sc->valid_smb_versions_mask & vflag;
}

/*********************************************************************
 * Function: DCE2_IsPortSet()
 *
 * Checks to see if a port is set in one in the port array mask
 * passed in.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *  const uint16_t
 *      The port to check for in the mask.
 *
 * Returns:
 *  int
 *      Non-zero if the port is set.
 *      Zero if the port is not set.
 *
 *********************************************************************/
static inline bool DCE2_IsPortSet(const uint8_t *port_array, const uint16_t port)
{
    return isPortEnabled( port_array, port );
}

/*********************************************************************
 * Function: DCE2_SetPort()
 *
 * Sets a port in the port array mask passed in.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *  const uint16_t
 *      The port to set in the port array mask.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_SetPort(uint8_t *port_array, const uint16_t port)
{
    enablePort( port_array, port );
}

/*********************************************************************
 * Function: DCE2_SetPortRange()
 *
 * Sets ports from lo to hi in one of the transport port
 * configurations.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *  uint16_t
 *      The lo port to start setting ports in the port array mask.
 *  uint16_t
 *      The hi port to end setting ports in the port array mask.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_SetPortRange(uint8_t *port_array, uint16_t lo_port, uint16_t hi_port)
{
    unsigned int i;

    if (lo_port > hi_port)
    {
        uint16_t tmp = lo_port;
        lo_port = hi_port;
        hi_port = tmp;
    }

    for (i = lo_port; i <= hi_port; i++)
        DCE2_SetPort(port_array, (uint16_t)i);
}

/********************************************************************
 * Function: DCE2_ClearPorts()
 *
 * Clears all of the port bits set in the port array mask.
 *
 * Arguments:
 *  uint8_t *
 *      Pointer to a port array mask.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_ClearPorts(uint8_t *port_array)
{
    memset(port_array, 0, DCE2_PORTS__MAX_INDEX);
}

/********************************************************************
 * Function: DCE2_IsWordChar()
 *
 * Determines if a character is a valid word character based on
 * position in the word.  Of course, this is the preprocessor's
 * definition.  Mainly used for restricting preprocessor option
 * names and set argument names.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *  DCE2_WordCharPosition
 *      The position in the word the character is.
 *
 * Returns:
 *  int
 *      1 if a valid word character.
 *      0 if not a valid word character.
 *
 ********************************************************************/
static inline int DCE2_IsWordChar(const char c, const DCE2_WordCharPosition pos)
{
    if (pos == DCE2_WORD_CHAR_POSITION__START)
    {
        if (isalpha((int)c))
            return 1;
    }
    else if (pos == DCE2_WORD_CHAR_POSITION__MIDDLE)
    {
        if (isalpha((int)c) ||
            isdigit((int)c) ||
            (c == DCE2_CFG_TOK__DASH) ||
            (c == DCE2_CFG_TOK__UNDERSCORE) ||
            (c == DCE2_CFG_TOK__DOT))
        {
            return 1;
        }
    }
    else if (pos == DCE2_WORD_CHAR_POSITION__END)
    {
        if (isalpha((int)c) || isdigit((int)c))
            return 1;
    }

    return 0;
}

/********************************************************************
 * Function: DCE2_IsListSepChar()
 *
 * Determines if the character passed in is a character that
 * separates values in lists.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid list separator character.
 *      0 if not a valid list separator character.
 *
 ********************************************************************/
static inline int DCE2_IsListSepChar(const char c)
{
    if (c == DCE2_CFG_TOK__LIST_SEP) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsOptEndChar()
 *
 * Determines if the character passed in is a character that
 * marks the end of an option and start of a new option.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid option end character.
 *      0 if not a valid option end character.
 *
 ********************************************************************/
static inline int DCE2_IsOptEndChar(const char c)
{
    if (c == DCE2_CFG_TOK__OPT_SEP) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsSpaceChar()
 *
 * Determines if the character passed in is a character that
 * the preprocessor considers a to be a space character.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid space character.
 *      0 if not a valid space character.
 *
 ********************************************************************/
static inline int DCE2_IsSpaceChar(const char c)
{
    if (isspace((int)c)) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsConfigEndChar()
 *
 * Determines if the character passed in is a character that
 * the preprocessor considers a to be an end of configuration
 * character.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid end of configuration character.
 *      0 if not a valid end of configuration character.
 *
 ********************************************************************/
static inline int DCE2_IsConfigEndChar(const char c)
{
    if (c == DCE2_CFG_TOK__END) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsPortChar()
 *
 * Determines if the character passed in is a character that
 * the preprocessor considers a to be a valid character for a port.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid port character.
 *      0 if not a valid port character.
 *
 ********************************************************************/
static inline int DCE2_IsPortChar(const char c)
{
    if (isdigit((int)c)) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsPortRangeChar()
 *
 * Determines if the character passed in is a character that can be
 * placed before, between or after a port to specify a port range.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid port range character.
 *      0 if not a valid port range character.
 *
 ********************************************************************/
static inline int DCE2_IsPortRangeChar(const char c)
{
    if (c == DCE2_CFG_TOK__PORT_RANGE) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsOpnumChar()
 *
 * Determines if the character passed in is a character that
 * the preprocessor considers a to be a valid character for a
 * DCE/RPC opnum.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid DCE/RPC opnum character.
 *      0 if not a valid DCE/RPC opnum character.
 *
 ********************************************************************/
static inline int DCE2_IsOpnumChar(const char c)
{
    if (isdigit((int)c)) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsOpnumRangeChar()
 *
 * Determines if the character passed in is a character that is
 * used to indicate a range of DCE/RPC opnums.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid DCE/RPC opnum range character.
 *      0 if not a valid DCE/RPC opnum range character.
 *
 ********************************************************************/
static inline int DCE2_IsOpnumRangeChar(const char c)
{
    if (c == DCE2_CFG_TOK__OPNUM_RANGE) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsListStartChar()
 *
 * Determines if the character passed in is a character that is
 * used to indicate the start of a list.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid start of list character.
 *      0 if not a valid start of list character.
 *
 ********************************************************************/
static inline int DCE2_IsListStartChar(const char c)
{
    if (c == DCE2_CFG_TOK__LIST_START) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsListEndChar()
 *
 * Determines if the character passed in is a character that is
 * used to indicate the end of a list.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid end of list character.
 *      0 if not a valid end of list character.
 *
 ********************************************************************/
static inline int DCE2_IsListEndChar(const char c)
{
    if (c == DCE2_CFG_TOK__LIST_END) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsQuoteChar()
 *
 * Determines if the character passed in is a what the preprocessor
 * considers to be a quote character.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid quote character.
 *      0 if not a valid quote character.
 *
 ********************************************************************/
static inline int DCE2_IsQuoteChar(const char c)
{
    if (c == DCE2_CFG_TOK__QUOTE) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_IsIpChar()
 *
 * Determines if the character passed in is a character that can
 * be used in an IP address - IPv4 or IPv6.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid IP character.
 *      0 if not a valid IP character.
 *
 ********************************************************************/
static inline int DCE2_IsIpChar(const char c)
{
    if (isxdigit((int)c) ||
        (c == DCE2_CFG_TOK__IP6_TET_SEP) ||
        (c == DCE2_CFG_TOK__IP4_TET_SEP) ||
        (c == DCE2_CFG_TOK__IP_PREFIX_SEP))
    {
        return 1;
    }

    return 0;
}

/********************************************************************
 * Function: DCE2_IsGraphChar()
 *
 * Determines is the character passed in is a graphical character.
 * Characters excluded are what the preprocessor considers as
 * meta characters or space characters.
 *
 * Arguments:
 *  const char
 *      The character to make the determination on.
 *
 * Returns:
 *  int
 *      1 if a valid graphical character.
 *      0 if not a valid graphical character.
 *
 ********************************************************************/
static inline int DCE2_IsGraphChar(const char c)
{
    if (!DCE2_IsListStartChar(c) && !DCE2_IsListEndChar(c) &&
        !DCE2_IsQuoteChar(c) && !DCE2_IsListSepChar(c) &&
        !DCE2_IsSpaceChar(c))
        return 1;

    return 0;
}

/*********************************************************************
 * Function: DCE2_CheckAndSetMask()
 *
 * Checks to see if a flag passed in is already set in the mask
 * passed in.  If it is, error is returned.  If it is not, the
 * flag is set in the mask.
 *
 * Arguments:
 *  int
 *      The flag to check and set.
 *  int *
 *      The mask to check and set the flag against.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if the flag is already set in the mask.
 *      DCE2_RET__SUCCESS if the flag is not already set in the mask.
 *
 *********************************************************************/
static inline DCE2_Ret DCE2_CheckAndSetMask(int flag, int *mask)
{
    if (*mask & flag)
        return DCE2_RET__ERROR;

    *mask |= flag;

    return DCE2_RET__SUCCESS;
}

void DCE2_RegisterPortsWithSession( struct _SnortConfig *sc, DCE2_ServerConfig *policy );

#endif  /* _DCE2_CONFIG_H_ */

