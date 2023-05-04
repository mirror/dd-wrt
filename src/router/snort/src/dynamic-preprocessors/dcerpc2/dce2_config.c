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
 * Parses and processes configuration set in snort.conf.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_config.h"
#include "dce2_utils.h"
#include "dce2_list.h"
#include "dce2_memory.h"
#include "dce2_event.h"
#include "dce2_session.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_types.h"
#include "sfrt.h"
#include "sf_ip.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifndef WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#endif  /* WIN32 */
#include <sfPolicy.h>
#include <sfPolicyUserData.h>

/********************************************************************
 * Global variables
 ********************************************************************/
tSfPolicyUserContextId dce2_config = NULL;
DCE2_Config *dce2_eval_config = NULL;

/* Default ports */
static const uint16_t DCE2_PORTS_SMB__DEFAULT[] = {139, 445};
static const uint16_t DCE2_PORTS_TCP__DEFAULT[] = {135};
static const uint16_t DCE2_PORTS_UDP__DEFAULT[] = {135};
//static const uint16_t DCE2_PORTS_HTTP_PROXY__DEFAULT[] = {80};
static const uint16_t DCE2_PORTS_HTTP_SERVER__DEFAULT[] = {593};

static char dce2_config_error[1024];

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_GOPT__MEMCAP          "memcap"
#define DCE2_GOPT__DISABLE_DEFRAG  "disable_defrag"
#define DCE2_GOPT__MAX_FRAG_LEN    "max_frag_len"
#define DCE2_GOPT__REASSEMBLE_THRESHOLD  "reassemble_threshold"
#define DCE2_GOPT__DISABLED        "disabled"
#define DCE2_GOPT__LEGACY_MODE     "smb_legacy_mode"

#define DCE2_GOPT__EVENTS         "events"
#define DCE2_GARG__EVENTS_NONE    "none"
#define DCE2_GARG__EVENTS_MEMCAP  "memcap"
#define DCE2_GARG__EVENTS_SMB     "smb"
#define DCE2_GARG__EVENTS_CO      "co"   /* Connection-oriented DCE/RPC */
#define DCE2_GARG__EVENTS_CL      "cl"   /* Connectionless DCE/RPC */
#define DCE2_GARG__EVENTS_ALL     "all"

#define DCE2_GOPT__SMB_FINGERPRINT   "smb_fingerprint_policy"
#define DCE2_GARG__SMBFP_CLIENT      "client"
#define DCE2_GARG__SMBFP_SERVER      "server"
#define DCE2_GARG__SMBFP_BOTH        "both"
#define DCE2_GARG__SMBFP_NONE        "none"

#define DCE2_SOPT__DEFAULT  "default"
#define DCE2_SOPT__NET      "net"

#define DCE2_SOPT__POLICY               "policy"
#define DCE2_SARG__POLICY_WIN2000       "Win2000"
#define DCE2_SARG__POLICY_WINXP         "WinXP"
#define DCE2_SARG__POLICY_WINVISTA      "WinVista"
#define DCE2_SARG__POLICY_WIN2003       "Win2003"
#define DCE2_SARG__POLICY_WIN2008       "Win2008"
#define DCE2_SARG__POLICY_WIN7          "Win7"
#define DCE2_SARG__POLICY_SAMBA         "Samba"
#define DCE2_SARG__POLICY_SAMBA_3_0_37  "Samba-3.0.37"  /* Samba version 3.0.37 and previous */
#define DCE2_SARG__POLICY_SAMBA_3_0_22  "Samba-3.0.22"  /* Samba version 3.0.22 and previous */
#define DCE2_SARG__POLICY_SAMBA_3_0_20  "Samba-3.0.20"  /* Samba version 3.0.20 and previous */

#define DCE2_SOPT__DETECT              "detect"
#define DCE2_SOPT__AUTODETECT          "autodetect"
#define DCE2_SARG__DETECT_NONE         "none"
#define DCE2_SARG__DETECT_SMB          "smb"
#define DCE2_SARG__DETECT_TCP          "tcp"
#define DCE2_SARG__DETECT_UDP          "udp"
#define DCE2_SARG__DETECT_HTTP_PROXY   "rpc-over-http-proxy"
#define DCE2_SARG__DETECT_HTTP_SERVER  "rpc-over-http-server"

#define DCE2_SOPT__NO_AUTODETECT_HTTP_PROXY_PORTS  "no_autodetect_http_proxy_ports"

#define DCE2_SOPT__SMB_INVALID_SHARES  "smb_invalid_shares"

#define DCE2_SOPT__SMB_MAX_CHAIN    "smb_max_chain"
#define DCE2_SMB_MAX_CHAIN__DEFAULT    3
#define DCE2_SMB_MAX_CHAIN__MAX      255   /* uint8_t is used to store value */

#define DCE2_SOPT__SMB_FILE_INSPECTION           "smb_file_inspection"
#define DCE2_SARG__SMB_FILE_INSPECTION_ON        "on"
#define DCE2_SARG__SMB_FILE_INSPECTION_OFF       "off"
#define DCE2_SARG__SMB_FILE_INSPECTION_ONLY      "only"
#define DCE2_SARG__SMB_FILE_INSPECTION_DEPTH     "file-depth"
#define DCE2_SMB_FILE_DEPTH_DEFAULT  16384

#define DCE2_SOPT__VALID_SMB_VERSIONS        "valid_smb_versions"
#define DCE2_SARG__VALID_SMB_VERSIONS_V1     "v1"
#define DCE2_SARG__VALID_SMB_VERSIONS_V2     "v2"
#define DCE2_SARG__VALID_SMB_VERSIONS_ALL    "all"

#define DCE2_SOPT__SMB2_MAX_COMPOUND    "smb2_max_compound"
#define DCE2_SMB2_MAX_COMPOUND__DEFAULT    3
#define DCE2_SMB2_MAX_COMPOUND__MAX      255   /* uint8_t is used to store value */

/*** Don't increase max memcap number or it will overflow ***/
#define DCE2_MEMCAP__MIN      1024    /* 1 MB min */
#define DCE2_MEMCAP__MAX      ((4 * 1024 * 1024) - 1)  /* ~ 4 GB max */

#define DCE2_MAX_FRAG__MAX       65535
#define DCE2_MAX_FRAG__MIN        1514

#define DCE2_AUTO_PORTS__START  1025

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_GcState
{
    DCE2_GC_STATE__OPT_START,
    DCE2_GC_STATE__OPT,
    DCE2_GC_STATE__OPT_END,
    DCE2_GC_STATE__END

} DCE2_GcState;

typedef enum _DCE2_GcOptFlag
{
    DCE2_GC_OPT_FLAG__NULL = 0x0000,
    DCE2_GC_OPT_FLAG__MEMCAP = 0x0001,
    DCE2_GC_OPT_FLAG__DISABLE_SMB_DESEG = 0x0002,
    DCE2_GC_OPT_FLAG__DISABLE_DEFRAG = 0x0004,
    DCE2_GC_OPT_FLAG__MAX_FRAG_LEN = 0x0008,
    DCE2_GC_OPT_FLAG__EVENTS = 0x0010,
    DCE2_GC_OPT_FLAG__REASSEMBLE_THRESHOLD = 0x0020,
    DCE2_GC_OPT_FLAG__DISABLED = 0x0040,
    DCE2_GC_OPT_FLAG__SMB_FINGERPRINT = 0x0080,
	DCE2_GC_OPT_FLAG__LEGACY_MODE = 0x0100

} DCE2_GcOptFlag;

typedef enum _DCE2_ScState
{
    DCE2_SC_STATE__ROPT_START,  /* Required option */
    DCE2_SC_STATE__ROPT,
    DCE2_SC_STATE__OPT_START,
    DCE2_SC_STATE__OPT,
    DCE2_SC_STATE__OPT_END,
    DCE2_SC_STATE__END

} DCE2_ScState;

typedef enum _DCE2_ScOptFlag
{
    DCE2_SC_OPT_FLAG__NULL = 0x0000,
    DCE2_SC_OPT_FLAG__DEFAULT = 0x0001,
    DCE2_SC_OPT_FLAG__NET = 0x0002,
    DCE2_SC_OPT_FLAG__POLICY = 0x0004,
    DCE2_SC_OPT_FLAG__DETECT = 0x0008,
    DCE2_SC_OPT_FLAG__AUTODETECT = 0x0010,
    DCE2_SC_OPT_FLAG__NO_AUTODETECT_HTTP_PROXY_PORTS = 0x0020,
    DCE2_SC_OPT_FLAG__SMB_INVALID_SHARES = 0x0040,
    DCE2_SC_OPT_FLAG__SMB_MAX_CHAIN = 0x0080,
    DCE2_SC_OPT_FLAG__VALID_SMB_VERSIONS = 0x0100,
    DCE2_SC_OPT_FLAG__SMB2_MAX_COMPOUND = 0x0200,
    DCE2_SC_OPT_FLAG__SMB_FILE_INSPECTION = 0x0400

} DCE2_ScOptFlag;

typedef enum _DCE2_DetectListState
{
    DCE2_DETECT_LIST_STATE__START,
    DCE2_DETECT_LIST_STATE__TYPE_START,
    DCE2_DETECT_LIST_STATE__TYPE,
    DCE2_DETECT_LIST_STATE__TYPE_END,
    DCE2_DETECT_LIST_STATE__PORTS_START,
    DCE2_DETECT_LIST_STATE__PORTS,
    DCE2_DETECT_LIST_STATE__PORTS_END,
    DCE2_DETECT_LIST_STATE__END

} DCE2_DetectListState;

typedef enum _DCE2_SmbFileListState
{
    DCE2_SMB_FILE_LIST_STATE__START,
    DCE2_SMB_FILE_LIST_STATE__ENABLEMENT_START,
    DCE2_SMB_FILE_LIST_STATE__ENABLEMENT,
    DCE2_SMB_FILE_LIST_STATE__ENABLEMENT_END,
    DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_START,
    DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH,
    DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE_START,
    DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE,
    DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE_END,
    DCE2_SMB_FILE_LIST_STATE__END

} DCE2_SmbFileListState;

/********************************************************************
 * Structures
 ********************************************************************/
/* Just used for printing detect and autodetect configurations */
typedef struct _DCE2_PrintPortsStruct
{
    const uint8_t *port_array;
    const char *trans_str;

} DCE2_PrintPortsStruct;

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static void DCE2_GcInitConfig(DCE2_GlobalConfig *gc);
static DCE2_Ret DCE2_GcParseConfig(DCE2_GlobalConfig *, char *);
static inline DCE2_GcOptFlag DCE2_GcParseOption(char *, char *, int *);
static DCE2_Ret DCE2_GcParseMemcap(DCE2_GlobalConfig *, char **, char *);
static DCE2_Ret DCE2_GcParseMaxFrag(DCE2_GlobalConfig *, char **, char *);
static DCE2_Ret DCE2_GcParseEvents(DCE2_GlobalConfig *, char **, char *);
static inline void DCE2_GcSetEvent(DCE2_GlobalConfig *, DCE2_EventFlag);
static inline void DCE2_GcClearAllEvents(DCE2_GlobalConfig *);
static inline DCE2_EventFlag DCE2_GcParseEvent(char *, char *, int *);
static DCE2_Ret DCE2_GcParseReassembleThreshold(DCE2_GlobalConfig *, char **, char *);
static DCE2_Ret DCE2_GcParseSmbFingerprintPolicy(DCE2_GlobalConfig *, char **, char *);
static void DCE2_GcPrintConfig(const DCE2_GlobalConfig *);
static void DCE2_GcError(const char *, ...);

static DCE2_Ret DCE2_ScInitConfig(DCE2_ServerConfig *);
static DCE2_Ret DCE2_ScInitPortArray(DCE2_ServerConfig *, DCE2_DetectFlag, int);
static DCE2_Ret DCE2_ScParseConfig(DCE2_Config *, DCE2_ServerConfig *, char *, DCE2_Queue *);
static inline DCE2_ScOptFlag DCE2_ScParseOption(char *, char *, int *);
static DCE2_Ret DCE2_ScParsePolicy(DCE2_ServerConfig *, char **, char *);
static DCE2_Ret DCE2_ScParseDetect(DCE2_ServerConfig *, char **, char *, int);
static inline DCE2_DetectFlag DCE2_ScParseDetectType(char *, char *, int *);
static inline void DCE2_ScResetPortsArrays(DCE2_ServerConfig *, int);
static DCE2_Ret DCE2_ScParseSmbShares(DCE2_ServerConfig *, char **, char *);
static DCE2_Ret DCE2_ScParseSmbMaxChain(DCE2_ServerConfig *, char **, char *);
static DCE2_Ret DCE2_ScParseSmb2MaxCompound(DCE2_ServerConfig *, char **, char *);
static DCE2_Ret DCE2_ScParseValidSmbVersions(DCE2_ServerConfig *, char **, char *);
static DCE2_Ret DCE2_ScParseSmbFileInspection(DCE2_ServerConfig *, char **, char *);
static inline DCE2_ValidSmbVersionFlag DCE2_ScParseValidSmbVersion(char *, char *, int *);
static inline void DCE2_ScSetValidSmbVersion(DCE2_ServerConfig *, DCE2_ValidSmbVersionFlag);
static inline void DCE2_ScClearAllValidSmbVersionFlags(DCE2_ServerConfig *);
static DCE2_Ret DCE2_ScAddToRoutingTable(DCE2_Config *, DCE2_ServerConfig *, DCE2_Queue *);
static int DCE2_ScSmbShareCompare(const void *, const void *);
static void DCE2_ScSmbShareFree(void *);
static void DCE2_ScPrintConfig(const DCE2_ServerConfig *, DCE2_Queue *);
static void DCE2_ScPrintPorts(const DCE2_ServerConfig *, int);
static void DCE2_ScIpListDataFree(void *);
static int DCE2_ScCheckTransport(void *);
static DCE2_Ret DCE2_ScCheckPortOverlap(const DCE2_ServerConfig *);
static void DCE2_AddPortsToStreamFilter(struct _SnortConfig *, DCE2_ServerConfig *, tSfPolicyId);
static void DCE2_ScError(const char *, ...);
static void DCE2_ServerConfigCleanup(void *);
void DCE2_RegisterPortsWithSession( struct _SnortConfig *sc, DCE2_ServerConfig *policy );

/********************************************************************
 * Function: DCE2_GlobalConfigure()
 *
 * Parses the DCE/RPC global configuration and stores values in a
 * global configuration structure.
 *
 * Arguments:
 *  char *
 *      snort.conf argument line for the dcerpc2 preprocessor.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_GlobalConfigure(DCE2_Config *config, char *args)
{
    if (config == NULL)
        return;

    dce2_config_error[0] = '\0';

    config->gconfig = (DCE2_GlobalConfig *)
        DCE2_Alloc(sizeof(DCE2_GlobalConfig), DCE2_MEM_TYPE__CONFIG);

    /* Allocate memory for global config structure */
    if (config->gconfig == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for global configuration.",
                 __FILE__, __LINE__);
    }

    /* Initialize the global config structure */
    DCE2_GcInitConfig(config->gconfig);

    /* If no arguments, just use default configuration */
    if (DCE2_IsEmptyStr(args))
    {
        DCE2_GcPrintConfig(config->gconfig);
        return;
    }

    if (DCE2_GcParseConfig(config->gconfig, args) != DCE2_RET__SUCCESS)
        DCE2_Die("%s", dce2_config_error);

    DCE2_GcPrintConfig(config->gconfig);
}

/********************************************************************
 * Function: DCE2_GcInitConfig
 *
 * Initializes global configuration to defaults.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to global config structure.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_GcInitConfig(DCE2_GlobalConfig *gc)
{
    if (gc == NULL)
        return;

    /* Convert default memcap to 100MB */
    gc->memcap = DCE2_MEMCAP__DEFAULT * 1024;

    /* Enable fragmentation reassembly */
    gc->dce_defrag = DCE2_CS__ENABLED;

    /* Set default max fragment size */
    gc->max_frag_len = DCE2_SENTINEL;
}

/********************************************************************
 * Function: DCE2_GcParseConfig()
 *
 * Main parsing of global configuration.  Parses options and
 * passes off to individual option handling.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char *
 *      Pointer to the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if parsing completed without error.
 *      DCE2_RET__ERROR if an error occurred during parsing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_GcParseConfig(DCE2_GlobalConfig *gc, char *args)
{
    DCE2_GcState state = DCE2_GC_STATE__OPT_START;
    char *ptr, *end;
    char *opt_start = args;
    char last_char = 0;
    int option_mask = 0;

    ptr = args;
    end = ptr + strlen(args) + 1;    /* Include NULL byte for state */

    while (ptr < end)
    {
        char c = *ptr;

        switch (state)
        {
            case DCE2_GC_STATE__OPT_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    opt_start = ptr;  /* Save pointer to first char of option */
                    state = DCE2_GC_STATE__OPT;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_GcError("Invalid syntax: \"%s\"", ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_GC_STATE__OPT:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    DCE2_GcOptFlag opt_flag;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_GcError("Invalid option: \"%.*s\"", ptr - opt_start, opt_start);
                        return DCE2_RET__ERROR;
                    }

                    opt_flag = DCE2_GcParseOption(opt_start, ptr, &option_mask);

                    switch (opt_flag)
                    {
                        case DCE2_GC_OPT_FLAG__MEMCAP:
                            if (DCE2_GcParseMemcap(gc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_GC_OPT_FLAG__DISABLE_DEFRAG:
                            gc->dce_defrag = DCE2_CS__DISABLED;
                            break;

                        case DCE2_GC_OPT_FLAG__MAX_FRAG_LEN:
                            if (DCE2_GcParseMaxFrag(gc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_GC_OPT_FLAG__EVENTS:
                            if (DCE2_GcParseEvents(gc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_GC_OPT_FLAG__REASSEMBLE_THRESHOLD:
                            if (DCE2_GcParseReassembleThreshold(gc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_GC_OPT_FLAG__DISABLED:
                            gc->disabled = 1;
                            break;

                        case DCE2_GC_OPT_FLAG__SMB_FINGERPRINT:
                            if (DCE2_GcParseSmbFingerprintPolicy(gc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_GC_OPT_FLAG__LEGACY_MODE:
                            gc->legacy_mode = true;
                            break;

                        default:
                            return DCE2_RET__ERROR;
                    }

                    state = DCE2_GC_STATE__OPT_END;
                    continue;
                }

                break;

            case DCE2_GC_STATE__OPT_END:
                if (DCE2_IsConfigEndChar(c))
                {
                    return DCE2_RET__SUCCESS;
                }
                else if (DCE2_IsOptEndChar(c))
                {
                    state = DCE2_GC_STATE__OPT_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_GcError("Invalid syntax: \"%s\"", ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid option state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        ptr++;
    }

    return DCE2_RET__ERROR;
}

/********************************************************************
 * Function: DCE2_GcParseOption()
 *
 * Parses the option and returns an option flag.  Checks to make
 * sure option is only configured once.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character of the option name.
 *  char *
 *      Pointer to the byte after the last character of
 *      the option name.
 *  int
 *      Pointer to the current option mask.  Contains bits set
 *      for each option that has already been configured.  Mask
 *      is checked and updated for new option.
 *
 * Returns:
 *  DCE2_GcOptFlag
 *      Flag for the global option that is being configured.
 *      NULL flag if not a valid option or option has already
 *          been configured.
 *
 ********************************************************************/
static inline DCE2_GcOptFlag DCE2_GcParseOption(char *opt_start, char *opt_end, int *opt_mask)
{
    DCE2_GcOptFlag opt_flag = DCE2_GC_OPT_FLAG__NULL;
    size_t opt_len = opt_end - opt_start;

    if (opt_len == strlen(DCE2_GOPT__MEMCAP) &&
        strncasecmp(DCE2_GOPT__MEMCAP, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__MEMCAP;
    }
    else if (opt_len == strlen(DCE2_GOPT__DISABLE_DEFRAG) &&
             strncasecmp(DCE2_GOPT__DISABLE_DEFRAG, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__DISABLE_DEFRAG;
    }
    else if (opt_len == strlen(DCE2_GOPT__MAX_FRAG_LEN) &&
             strncasecmp(DCE2_GOPT__MAX_FRAG_LEN, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__MAX_FRAG_LEN;
    }
    else if (opt_len == strlen(DCE2_GOPT__EVENTS) &&
             strncasecmp(DCE2_GOPT__EVENTS, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__EVENTS;
    }
    else if (opt_len == strlen(DCE2_GOPT__REASSEMBLE_THRESHOLD) &&
             strncasecmp(DCE2_GOPT__REASSEMBLE_THRESHOLD, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__REASSEMBLE_THRESHOLD;
    }
    else if (opt_len == strlen(DCE2_GOPT__DISABLED) &&
             strncasecmp(DCE2_GOPT__DISABLED, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__DISABLED;
    }
    else if (opt_len == strlen(DCE2_GOPT__SMB_FINGERPRINT) &&
             strncasecmp(DCE2_GOPT__SMB_FINGERPRINT, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__SMB_FINGERPRINT;
    }
    else if (opt_len == strlen(DCE2_GOPT__LEGACY_MODE) &&
             strncasecmp(DCE2_GOPT__LEGACY_MODE, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_GC_OPT_FLAG__LEGACY_MODE;
    }
    else
    {
        DCE2_GcError("Invalid option: \"%.*s\"", opt_len, opt_start);
        return DCE2_GC_OPT_FLAG__NULL;
    }

    if (DCE2_CheckAndSetMask(opt_flag, opt_mask) != DCE2_RET__SUCCESS)
    {
        DCE2_GcError("Can only configure option once: \"%.*s\"", opt_len, opt_start);
        return DCE2_GC_OPT_FLAG__NULL;
    }

    return opt_flag;
}

/********************************************************************
 * Function: DCE2_GcParseMemcap()
 *
 * Parses the argument to the memcap option and adds to global
 * configuration if successfully parsed.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing memcap.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          value for the memcap.
 *      DCE2_RET__ERROR if an error occured in parsing the memcap.
 *
 ********************************************************************/
static DCE2_Ret DCE2_GcParseMemcap(DCE2_GlobalConfig *gc, char **ptr, char *end)
{
    uint32_t memcap;

    if (DCE2_ParseValue(ptr, end, &memcap, DCE2_INT_TYPE__UINT32) != DCE2_RET__SUCCESS)
    {
        DCE2_GcError("Error parsing \"%s\".  Value must be between %d and %d KB.",
                     DCE2_GOPT__MEMCAP, DCE2_MEMCAP__MIN, DCE2_MEMCAP__MAX);
        return DCE2_RET__ERROR;
    }

    if ((memcap < DCE2_MEMCAP__MIN) || (memcap > DCE2_MEMCAP__MAX))
    {
        DCE2_GcError("Invalid \"%s\".  Value must be between %d and %d KB",
                     DCE2_GOPT__MEMCAP, DCE2_MEMCAP__MIN, DCE2_MEMCAP__MAX);
        return DCE2_RET__ERROR;
    }

    /* Memcap is configured as kilobytes - convert to bytes */
    gc->memcap = memcap * 1024;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_GcParseMaxFrag()
 *
 * Parses the argument to the max frag option and adds to global
 * configuration if successfully parsed.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing max frag.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          value for the max frag option.
 *      DCE2_RET__ERROR if an error occured in parsing the max frag
 *          option.
 *
 ********************************************************************/
static DCE2_Ret DCE2_GcParseMaxFrag(DCE2_GlobalConfig *gc, char **ptr, char *end)
{
    uint16_t max_frag_len;

    if (DCE2_ParseValue(ptr, end, &max_frag_len, DCE2_INT_TYPE__UINT16) != DCE2_RET__SUCCESS)
    {
        DCE2_GcError("Error parsing \"%s\".  Value must be between %d and %d",
                     DCE2_GOPT__MAX_FRAG_LEN, DCE2_MAX_FRAG__MIN, DCE2_MAX_FRAG__MAX);
        return DCE2_RET__ERROR;
    }

    if (max_frag_len < DCE2_MAX_FRAG__MIN)
    {
        DCE2_GcError("Invalid \"%s\".  Value must be between %d and %d",
                     DCE2_GOPT__MAX_FRAG_LEN, DCE2_MAX_FRAG__MIN, DCE2_MAX_FRAG__MAX);
        return DCE2_RET__ERROR;
    }

    /* Cast since we initialize max frag len in global structure to a sentinel */
    gc->max_frag_len = (int)max_frag_len;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_GcParseEvents()
 *
 * Parses the event types for the events option and adds to
 * global configuration.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing memcap.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          events.
 *      DCE2_RET__ERROR if an error occured in parsing the events.
 *
 ********************************************************************/
static DCE2_Ret DCE2_GcParseEvents(DCE2_GlobalConfig *gc, char **ptr, char *end)
{
    DCE2_WordListState state = DCE2_WORD_LIST_STATE__START;
    char *event_start = *ptr;
    char last_char = 0;
    int one_event = 0;
    int event_mask = 0;

    DCE2_GcClearAllEvents(gc);

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_WORD_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_WORD_LIST_STATE__START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    /* Only one event */
                    event_start = *ptr;
                    one_event = 1;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__WORD_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_GcError("Invalid \"%s\" syntax: \"%s\"", DCE2_GOPT__EVENTS, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    event_start = *ptr;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_GcError("Invalid \"%s\" syntax: \"%s\"", DCE2_GOPT__EVENTS, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    DCE2_EventFlag eflag;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_GcError("Invalid \"%s\" argument: \"%.*s\"",
                                     DCE2_GOPT__EVENTS, *ptr - event_start, event_start);
                        return DCE2_RET__ERROR;
                    }


                    eflag = DCE2_GcParseEvent(event_start, *ptr, &event_mask);
                    switch (eflag)
                    {
                        case DCE2_EVENT_FLAG__NULL:
                            return DCE2_RET__ERROR;

                        case DCE2_EVENT_FLAG__NONE:
                            if (!one_event)
                            {
                                DCE2_GcError("Event type \"%s\" cannot be "
                                             "configured in a list", DCE2_GARG__EVENTS_NONE);
                                return DCE2_RET__ERROR;
                            }

                            /* Not really necessary since we're returning early,
                             * but leave it here since this would be the action */
                            DCE2_GcClearAllEvents(gc);
                            break;

                        case DCE2_EVENT_FLAG__ALL:
                            if (!one_event)
                            {
                                DCE2_GcError("Event type \"%s\" cannot be "
                                             "configured in a list", DCE2_GARG__EVENTS_ALL);
                                return DCE2_RET__ERROR;
                            }

                            DCE2_GcSetEvent(gc, eflag);
                            break;

                        default:
                            DCE2_GcSetEvent(gc, eflag);
                            break;
                    }

                    if (one_event)
                        return DCE2_RET__SUCCESS;

                    state = DCE2_WORD_LIST_STATE__WORD_END;
                    continue;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD_END:
                if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__END;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__WORD_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_GcError("Invalid \"%s\" syntax: \"%s\"", DCE2_GOPT__EVENTS, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid events state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        (*ptr)++;
    }

    if (state != DCE2_WORD_LIST_STATE__END)
    {
        DCE2_GcError("Invalid \"%s\" syntax: \"%s\"", DCE2_GOPT__EVENTS, *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_GcParseEvent()
 *
 * Parses event type and returns flag indication the type of event.
 * Checks and sets a bit in a mask to prevent multiple
 * configurations of the same event type.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character of the event type name.
 *  char *
 *      Pointer to the byte after the last character of
 *      the event type name.
 *  int
 *      Pointer to the current event type mask.  Contains bits set
 *      for each event type that has already been configured.  Mask
 *      is checked and updated for new event type.
 *
 * Returns:
 *  DCE2_EventFlag
 *      Flag indicating the type of event.
 *      DCE2_EVENT_FLAG__NULL if no event type or multiple
 *          configuration of event type.
 *
 ********************************************************************/
static inline DCE2_EventFlag DCE2_GcParseEvent(char *start, char *end, int *emask)
{
    DCE2_EventFlag eflag = DCE2_EVENT_FLAG__NULL;
    size_t event_len = end - start;

    if (event_len == strlen(DCE2_GARG__EVENTS_NONE) &&
        strncasecmp(DCE2_GARG__EVENTS_NONE, start, event_len) == 0)
    {
        eflag = DCE2_EVENT_FLAG__NONE;
    }
    else if (event_len == strlen(DCE2_GARG__EVENTS_MEMCAP) &&
             strncasecmp(DCE2_GARG__EVENTS_MEMCAP, start, event_len) == 0)
    {
        eflag = DCE2_EVENT_FLAG__MEMCAP;
    }
    else if (event_len == strlen(DCE2_GARG__EVENTS_SMB) &&
             strncasecmp(DCE2_GARG__EVENTS_SMB, start, event_len) == 0)
    {
        eflag = DCE2_EVENT_FLAG__SMB;
    }
    else if (event_len == strlen(DCE2_GARG__EVENTS_CO) &&
             strncasecmp(DCE2_GARG__EVENTS_CO, start, event_len) == 0)
    {
        eflag = DCE2_EVENT_FLAG__CO;
    }
    else if (event_len == strlen(DCE2_GARG__EVENTS_CL) &&
             strncasecmp(DCE2_GARG__EVENTS_CL, start, event_len) == 0)
    {
        eflag = DCE2_EVENT_FLAG__CL;
    }
    else if (event_len == strlen(DCE2_GARG__EVENTS_ALL) &&
             strncasecmp(DCE2_GARG__EVENTS_ALL, start, event_len) == 0)
    {
        eflag = DCE2_EVENT_FLAG__ALL;
    }
    else
    {
        DCE2_GcError("Invalid \"%s\" argument: \"%.*s\"",
                     DCE2_GOPT__EVENTS, event_len, start);
        return DCE2_EVENT_FLAG__NULL;
    }

    if (DCE2_CheckAndSetMask((int)eflag, emask) != DCE2_RET__SUCCESS)
    {
        DCE2_GcError("Event type \"%.*s\" cannot be specified more than once",
                     event_len, start);
        return DCE2_EVENT_FLAG__NULL;
    }

    return eflag;
}

/*********************************************************************
 * Function: DCE2_GcSetEvent()
 *
 * Sets the event types the user wants to see during processing in
 * the global configuration event mask.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to global config structure.
 *  DCE2_EventFlag
 *      The event type flag to set.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_GcSetEvent(DCE2_GlobalConfig *gc, DCE2_EventFlag eflag)
{
    gc->event_mask |= eflag;
}

/*********************************************************************
 * Function: DCE2_GcClearAllEvents()
 *
 * Clears all of the bits in the global configuration event mask.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to global config structure.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_GcClearAllEvents(DCE2_GlobalConfig *gc)
{
    gc->event_mask = DCE2_EVENT_FLAG__NULL;
}

/********************************************************************
 * Function: DCE2_GcParseReassembleThreshold()
 *
 * Parses the argument to the reassemble threshold option and adds
 * to global configuration if successfully parsed.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the reassemble threshold.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          value for the reassemble threshold.
 *      DCE2_RET__ERROR if an error occured in parsing the
 *          reassemble threshold.
 *
 ********************************************************************/
static DCE2_Ret DCE2_GcParseReassembleThreshold(DCE2_GlobalConfig *gc, char **ptr, char *end)
{
    uint16_t reassemble_threshold;

    if (DCE2_ParseValue(ptr, end, &reassemble_threshold, DCE2_INT_TYPE__UINT16) != DCE2_RET__SUCCESS)
    {
        DCE2_GcError("Error parsing \"%s\".  Value must be between 0 and %u inclusive",
                     DCE2_GOPT__REASSEMBLE_THRESHOLD, UINT16_MAX);
        return DCE2_RET__ERROR;
    }

    gc->reassemble_threshold = reassemble_threshold;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_GcParseSmbFingerPrintPolicy()
 *
 * Parses the smb_fingerprint_policy option
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the reassemble threshold.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse.
 *      DCE2_RET__ERROR if an error occured in parsing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_GcParseSmbFingerprintPolicy(DCE2_GlobalConfig *gc, char **ptr, char *end)
{
    DCE2_WordListState state = DCE2_WORD_LIST_STATE__START;
    char *fp_start = *ptr;
    char last_char = 0;

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_WORD_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_WORD_LIST_STATE__START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    fp_start = *ptr;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_GcError("Invalid \"%s\" syntax: \"%s\"",
                            DCE2_GOPT__SMB_FINGERPRINT, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    size_t len = *ptr - fp_start;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_GcError("Invalid \"%s\" argument: \"%*.s\"",
                                DCE2_GOPT__SMB_FINGERPRINT, *ptr - fp_start, fp_start);
                        return DCE2_RET__ERROR;
                    }

                    if (len == strlen(DCE2_GARG__SMBFP_CLIENT) &&
                            strncasecmp(DCE2_GARG__SMBFP_CLIENT, fp_start, len) == 0)
                    {
                        gc->smb_fingerprint_policy = DCE2_SMB_FINGERPRINT__CLIENT;
                    }
                    else if (len == strlen(DCE2_GARG__SMBFP_SERVER) &&
                            strncasecmp(DCE2_GARG__SMBFP_SERVER, fp_start, len) == 0)
                    {
                        gc->smb_fingerprint_policy = DCE2_SMB_FINGERPRINT__SERVER;
                    }
                    else if (len == strlen(DCE2_GARG__SMBFP_BOTH) &&
                            strncasecmp(DCE2_GARG__SMBFP_BOTH, fp_start, len) == 0)
                    {
                        gc->smb_fingerprint_policy = DCE2_SMB_FINGERPRINT__SERVER;
                        gc->smb_fingerprint_policy |= DCE2_SMB_FINGERPRINT__CLIENT;
                    }
                    else if (len == strlen(DCE2_GARG__SMBFP_NONE) &&
                            strncasecmp(DCE2_GARG__SMBFP_NONE, fp_start, len) == 0)
                    {
                        gc->smb_fingerprint_policy = DCE2_SMB_FINGERPRINT__NONE;
                    }
                    else
                    {
                        DCE2_GcError("Invalid \"%s\" argument: \"%.*s\"",
                                DCE2_GOPT__SMB_FINGERPRINT, *ptr - fp_start, fp_start);
                        return DCE2_RET__ERROR;
                    }

                    state = DCE2_WORD_LIST_STATE__END;
                    continue;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid smb fingerprint state: %d",
                    __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        (*ptr)++;
    }

    if (state != DCE2_WORD_LIST_STATE__END)
    {
        DCE2_GcError("Invalid \"%s\" syntax: \"%s\"", DCE2_GOPT__SMB_FINGERPRINT, *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScInitConfig
 *
 * Initializes a server configuration to defaults.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to server configuration structure to initialize.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if initialization fails
 *      DCE2_RET__SUCCESS if initialization succeeds
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScInitConfig(DCE2_ServerConfig *sc)
{
    if (sc == NULL)
        return DCE2_RET__ERROR;

    /* Set defaults */
    sc->policy = DCE2_POLICY__WINXP;
    sc->smb_max_chain = DCE2_SMB_MAX_CHAIN__DEFAULT;
    sc->smb2_max_compound = DCE2_SMB2_MAX_COMPOUND__DEFAULT;
    sc->valid_smb_versions_mask = DCE2_VALID_SMB_VERSION_FLAG__ALL;
    sc->autodetect_http_proxy_ports = DCE2_CS__ENABLED;
    sc->smb_file_inspection = DCE2_SMB_FILE_INSPECTION__OFF;
    sc->smb_file_depth = DCE2_SMB_FILE_DEPTH_DEFAULT;

    /* Add default detect ports */
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__SMB, 0) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__TCP, 0) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__UDP, 0) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__HTTP_PROXY, 0) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__HTTP_SERVER, 0) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;

    /* Add default autodetect ports */
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__SMB, 1) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__TCP, 1) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__UDP, 1) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__HTTP_PROXY, 1) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;
    if (DCE2_ScInitPortArray(sc, DCE2_DETECT_FLAG__HTTP_SERVER, 1) != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScInitPortArray
 *
 * Initializes a detect or autodetect port array to default
 * values.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to server configuration structure to initialize.
 *  DCE2_DetectFlag
 *      The transport for which to set defaults
 *  int
 *      Non-zero to set autodetect ports
 *      Zero to set detect ports
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if and invalid DetectFlag is passed in
 *      DCE2_RET__SUCCESS if ports arrays are successfully
 *          initialized
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScInitPortArray(DCE2_ServerConfig *sc, DCE2_DetectFlag dflag, int autodetect)
{
    if (!autodetect)
    {
        unsigned int array_len;
        unsigned int i;

        switch (dflag)
        {
            case DCE2_DETECT_FLAG__SMB:
                DCE2_ClearPorts(sc->smb_ports);
                array_len =
                    sizeof(DCE2_PORTS_SMB__DEFAULT) / sizeof(DCE2_PORTS_SMB__DEFAULT[0]);

                for (i = 0; i < array_len; i++)
                    DCE2_SetPort(sc->smb_ports, DCE2_PORTS_SMB__DEFAULT[i]);

                break;

            case DCE2_DETECT_FLAG__TCP:
                DCE2_ClearPorts(sc->tcp_ports);
                array_len =
                    sizeof(DCE2_PORTS_TCP__DEFAULT) / sizeof(DCE2_PORTS_TCP__DEFAULT[0]);

                for (i = 0; i < array_len; i++)
                    DCE2_SetPort(sc->tcp_ports, DCE2_PORTS_TCP__DEFAULT[i]);

                break;

            case DCE2_DETECT_FLAG__UDP:
                DCE2_ClearPorts(sc->udp_ports);
                array_len =
                    sizeof(DCE2_PORTS_UDP__DEFAULT) / sizeof(DCE2_PORTS_UDP__DEFAULT[0]);

                for (i = 0; i < array_len; i++)
                    DCE2_SetPort(sc->udp_ports, DCE2_PORTS_UDP__DEFAULT[i]);

                break;

            case DCE2_DETECT_FLAG__HTTP_PROXY:
                DCE2_ClearPorts(sc->http_proxy_ports);
                /* Since there currently aren't any VRT dcerpc rules using
                 * port 80, don't include it by default */
#if 0
                array_len =
                    sizeof(DCE2_PORTS_HTTP_PROXY__DEFAULT) / sizeof(DCE2_PORTS_HTTP_PROXY__DEFAULT[0]);

                for (i = 0; i < array_len; i++)
                    DCE2_SetPort(sc->http_proxy_ports, DCE2_PORTS_HTTP_PROXY__DEFAULT[i]);

#endif
                break;

            case DCE2_DETECT_FLAG__HTTP_SERVER:
                DCE2_ClearPorts(sc->http_server_ports);
                array_len =
                    sizeof(DCE2_PORTS_HTTP_SERVER__DEFAULT) / sizeof(DCE2_PORTS_HTTP_SERVER__DEFAULT[0]);

                for (i = 0; i < array_len; i++)
                    DCE2_SetPort(sc->http_server_ports, DCE2_PORTS_HTTP_SERVER__DEFAULT[i]);

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid transport type: %d",
                         __FILE__, __LINE__, dflag);
                return DCE2_RET__ERROR;
        }
    }
    else
    {
        uint8_t *port_array = NULL;
        unsigned int port;

        switch (dflag)
        {
            case DCE2_DETECT_FLAG__SMB:
                DCE2_ClearPorts(sc->auto_smb_ports);
                return DCE2_RET__SUCCESS;

            case DCE2_DETECT_FLAG__TCP:
                DCE2_ClearPorts(sc->auto_tcp_ports);
                port_array = sc->auto_tcp_ports;
                break;

            case DCE2_DETECT_FLAG__UDP:
                DCE2_ClearPorts(sc->auto_udp_ports);
                port_array = sc->auto_udp_ports;
                break;

            case DCE2_DETECT_FLAG__HTTP_PROXY:
                DCE2_ClearPorts(sc->auto_http_proxy_ports);
                return DCE2_RET__SUCCESS;

            case DCE2_DETECT_FLAG__HTTP_SERVER:
                DCE2_ClearPorts(sc->auto_http_server_ports);
                port_array = sc->auto_http_server_ports;
                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid transport type: %d",
                         __FILE__, __LINE__, dflag);
                return DCE2_RET__ERROR;
        }

        /* By default, only autodetect on ports 1025 and above,
         * and not on SMB or RPC over HTTP proxy */
        for (port = DCE2_AUTO_PORTS__START; port < DCE2_PORTS__MAX; port++)
            DCE2_SetPort(port_array, (uint16_t)port);
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CreateDefaultServerConfig()
 *
 * Creates a default server configuration for non-matching specific
 * server configurations.
 *
 * Arguments: None
 *
 * Returns: -1 on error
 *
 ********************************************************************/
int DCE2_CreateDefaultServerConfig(struct _SnortConfig *sc, DCE2_Config *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return 0;

    config->dconfig = (DCE2_ServerConfig *)
        DCE2_Alloc(sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);

    if (config->dconfig == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for default server "
                 "configuration.", __FILE__, __LINE__);
    }

    if (DCE2_ScInitConfig(config->dconfig) != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__WARN, "%s(%d) Failed to initialize default server "
                 "configuration.", __FILE__, __LINE__);
        return -1;
    }

    DCE2_AddPortsToStreamFilter(sc, config->dconfig, policy_id);
    return 0;
}

/********************************************************************
 * Function: DCE2_ServerConfigure()
 *
 * Parses the DCE/RPC server configuration and stores values in
 * server configuration.
 *
 * Arguments:
 *  char *
 *      snort.conf argument line for the dcerpc2 preprocessor.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ServerConfigure(struct _SnortConfig *snortConf, DCE2_Config *config, char *args)
{
    DCE2_ServerConfig *sc;
    DCE2_Queue *ip_queue;
    DCE2_Ret status;
    tSfPolicyId policy_id = _dpd.getParserPolicy(snortConf);

    if (config == NULL)
        return;

    dce2_config_error[0] = '\0';

    /* Must have arguments */
    if (DCE2_IsEmptyStr(args))
    {
        DCE2_Die("%s(%d) \"%s\" configuration: No arguments to server "
                 "configuration.  Must have a \"%s\" or \"%s\" argument.",
                 *_dpd.config_file, *_dpd.config_line,
                 DCE2_SNAME, DCE2_SOPT__DEFAULT, DCE2_SOPT__NET);
    }

    /* Alloc server config */
    sc = (DCE2_ServerConfig *)DCE2_Alloc(sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
    if (sc == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for server "
                 "configuration.", __FILE__, __LINE__);
    }

    if (DCE2_ScInitConfig(sc) != DCE2_RET__SUCCESS)
    {
        DCE2_ListDestroy(sc->smb_invalid_shares);
        DCE2_Free((void *)sc, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
        DCE2_Die("%s(%d) \"%s\" configuration: Failed to initialize server configuration.",
                 __FILE__, __LINE__, DCE2_SNAME);
    }

    /* The ip queue stores the IPs from a specific server configuration
     * for adding to the routing tables */
    ip_queue = DCE2_QueueNew(DCE2_ScIpListDataFree, DCE2_MEM_TYPE__CONFIG);
    if (ip_queue == NULL)
    {
        DCE2_ListDestroy(sc->smb_invalid_shares);
        DCE2_Free((void *)sc, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
        DCE2_Die("%s(%d) Failed to allocate memory for IP queue.", __FILE__, __LINE__);
    }

    status = DCE2_ScParseConfig(config, sc, args, ip_queue);
    if (status != DCE2_RET__SUCCESS)
    {
        if (sc != config->dconfig)
        {
            DCE2_ListDestroy(sc->smb_invalid_shares);
            DCE2_Free((void *)sc, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
        }

        DCE2_QueueDestroy(ip_queue);
        DCE2_Die("%s", dce2_config_error);
    }

    /* Check for overlapping detect ports */
    if (DCE2_ScCheckPortOverlap(sc) != DCE2_RET__SUCCESS)
    {
        if (sc != config->dconfig)
        {
            DCE2_ListDestroy(sc->smb_invalid_shares);
            DCE2_Free((void *)sc, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
        }

        DCE2_QueueDestroy(ip_queue);
        DCE2_Die("%s", dce2_config_error);
    }

    DCE2_AddPortsToStreamFilter(snortConf, sc, policy_id);
    DCE2_RegisterPortsWithSession(snortConf, sc);

    if ((sc != config->dconfig) &&
        (DCE2_ScAddToRoutingTable(config, sc, ip_queue) != DCE2_RET__SUCCESS))
    {
        DCE2_ListDestroy(sc->smb_invalid_shares);
        DCE2_Free((void *)sc, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
        DCE2_QueueDestroy(ip_queue);
        DCE2_Die("%s", dce2_config_error);
    }

    DCE2_ScPrintConfig(sc, ip_queue);
    DCE2_QueueDestroy(ip_queue);
}

/********************************************************************
 * Function: DCE2_ScParseConfig()
 *
 * Main parsing of a server configuration.  Parses options and
 * passes off to individual option handling.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char *
 *      Pointer to the configuration line.
 *  DCE2_Queue *
 *      Pointer to a queue for storing IPs.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if parsing completed without error.
 *      DCE2_RET__ERROR if an error occurred during parsing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseConfig(DCE2_Config *config, DCE2_ServerConfig *sc,
                                   char *args, DCE2_Queue *ip_queue)
{
    DCE2_ScState state = DCE2_SC_STATE__ROPT_START;
    char *ptr, *end;
    char *opt_start = args;
    char last_char = 0;
    int option_mask = 0;

    ptr = args;
    end = ptr + strlen(args) + 1;    /* Include NULL byte for state */

    while (ptr < end)
    {
        char c = *ptr;

        switch (state)
        {
            case DCE2_SC_STATE__ROPT_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    opt_start = ptr;
                    state = DCE2_SC_STATE__ROPT;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid syntax: \"%s\"", ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SC_STATE__ROPT:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    DCE2_ScOptFlag opt_flag;
                    DCE2_Ret status;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid option: \"%.*s\"", ptr - opt_start, opt_start);
                        return DCE2_RET__ERROR;
                    }

                    opt_flag = DCE2_ScParseOption(opt_start, ptr, &option_mask);

                    switch (opt_flag)
                    {
                        case DCE2_SC_OPT_FLAG__DEFAULT:
                            if (config->dconfig != NULL)
                            {
                                DCE2_ScError("Can only configure \"%s\" "
                                             "configuration once", DCE2_SOPT__DEFAULT);
                                return DCE2_RET__ERROR;
                            }

                            config->dconfig = sc;
                            break;

                        case DCE2_SC_OPT_FLAG__NET:
                            if (config->dconfig == NULL)
                            {
                                DCE2_ScError("Must configure \"%s\" before any "
                                             "\"%s\" configurations",
                                             DCE2_SOPT__DEFAULT, DCE2_SOPT__NET);
                                return DCE2_RET__ERROR;
                            }

                            status = DCE2_ParseIpList(&ptr, end, ip_queue);
                            if (status != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        default:
                            DCE2_ScError("Invalid first option to server configuration. "
                                         "First option must be \"%s\" or \"%s\"",
                                         DCE2_SOPT__DEFAULT, DCE2_SOPT__NET);

                            return DCE2_RET__ERROR;
                    }

                    state = DCE2_SC_STATE__OPT_END;
                    continue;
                }

                break;

            case DCE2_SC_STATE__OPT_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    opt_start = ptr;
                    state = DCE2_SC_STATE__OPT;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid syntax: \"%s\"", ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SC_STATE__OPT:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    DCE2_ScOptFlag opt_flag;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid option: \"%.*s\"", ptr - opt_start, opt_start);
                        return DCE2_RET__ERROR;
                    }

                    opt_flag = DCE2_ScParseOption(opt_start, ptr, &option_mask);

                    switch (opt_flag)
                    {
                        case DCE2_SC_OPT_FLAG__POLICY:
                            if (DCE2_ScParsePolicy(sc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_SC_OPT_FLAG__DETECT:
                            if (DCE2_ScParseDetect(sc, &ptr, end, 0) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_SC_OPT_FLAG__AUTODETECT:
                            if (DCE2_ScParseDetect(sc, &ptr, end, 1) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_SC_OPT_FLAG__NO_AUTODETECT_HTTP_PROXY_PORTS:
                            sc->autodetect_http_proxy_ports = DCE2_CS__DISABLED;
                            break;

                        case DCE2_SC_OPT_FLAG__SMB_INVALID_SHARES:
                            sc->smb_invalid_shares =
                                DCE2_ListNew(DCE2_LIST_TYPE__NORMAL, DCE2_ScSmbShareCompare,
                                             DCE2_ScSmbShareFree, DCE2_ScSmbShareFree,
                                             DCE2_LIST_FLAG__NO_DUPS | DCE2_LIST_FLAG__INS_TAIL,
                                             DCE2_MEM_TYPE__CONFIG);

                            if (sc->smb_invalid_shares == NULL)
                            {
                                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                         "%s(%d) Failed to allocate memory "
                                         "for invalid share list.", __FILE__, __LINE__);
                                return DCE2_RET__ERROR;
                            }

                            if (DCE2_ScParseSmbShares(sc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;

                            break;

                        case DCE2_SC_OPT_FLAG__SMB_MAX_CHAIN:
                            if (DCE2_ScParseSmbMaxChain(sc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_SC_OPT_FLAG__SMB2_MAX_COMPOUND:
                            if (DCE2_ScParseSmb2MaxCompound(sc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_SC_OPT_FLAG__VALID_SMB_VERSIONS:
                            if (DCE2_ScParseValidSmbVersions(sc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
                            break;

                        case DCE2_SC_OPT_FLAG__SMB_FILE_INSPECTION:
                            if (DCE2_ScParseSmbFileInspection(sc, &ptr, end) != DCE2_RET__SUCCESS)
                                return DCE2_RET__ERROR;
#ifdef ACTIVE_RESPONSE
                            if ((sc->smb_file_inspection == DCE2_SMB_FILE_INSPECTION__ONLY)
                                    || (sc->smb_file_inspection == DCE2_SMB_FILE_INSPECTION__ON))
                                _dpd.activeSetEnabled(1);
#endif
                            break;

                        case DCE2_SC_OPT_FLAG__DEFAULT:
                        case DCE2_SC_OPT_FLAG__NET:
                            DCE2_ScError("\"%s\" or \"%s\" must be the first "
                                         "option to configuration",
                                         DCE2_SOPT__DEFAULT, DCE2_SOPT__NET);
                            return DCE2_RET__ERROR;

                        default:
                            return DCE2_RET__ERROR;
                    }

                    state = DCE2_SC_STATE__OPT_END;
                    continue;
                }

                break;

            case DCE2_SC_STATE__OPT_END:
                if (DCE2_IsConfigEndChar(c))
                {
                    return DCE2_RET__SUCCESS;
                }
                else if (DCE2_IsOptEndChar(c))
                {
                    state = DCE2_SC_STATE__OPT_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid syntax: \"%s\"", ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid option state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        ptr++;
    }

    return DCE2_RET__ERROR;
}

/********************************************************************
 * Function: DCE2_ScParseOption()
 *
 * Parses the option and returns an option flag.  Checks to make
 * sure option is only configured once.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character of the option name.
 *  char *
 *      Pointer to the byte after the last character of
 *      the option name.
 *  int
 *      Pointer to the current option mask.  Contains bits set
 *      for each option that has already been configured.  Mask
 *      is checked and updated for new option.
 *
 * Returns:
 *  DCE2_ScOptFlag
 *      Flag for the server option that is being configured.
 *      NULL flag if not a valid option or option has already
 *          been configured.
 *
 ********************************************************************/
static inline DCE2_ScOptFlag DCE2_ScParseOption(char *opt_start, char *opt_end, int *opt_mask)
{
    DCE2_ScOptFlag opt_flag = DCE2_SC_OPT_FLAG__NULL;
    size_t opt_len = opt_end - opt_start;

    if (opt_len == strlen(DCE2_SOPT__DEFAULT) &&
        strncasecmp(DCE2_SOPT__DEFAULT, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__DEFAULT;
    }
    else if (opt_len == strlen(DCE2_SOPT__NET) &&
             strncasecmp(DCE2_SOPT__NET, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__NET;
    }
    else if (opt_len == strlen(DCE2_SOPT__POLICY) &&
             strncasecmp(DCE2_SOPT__POLICY, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__POLICY;
    }
    else if (opt_len == strlen(DCE2_SOPT__DETECT) &&
             strncasecmp(DCE2_SOPT__DETECT, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__DETECT;
    }
    else if (opt_len == strlen(DCE2_SOPT__AUTODETECT) &&
             strncasecmp(DCE2_SOPT__AUTODETECT, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__AUTODETECT;
    }
    else if (opt_len == strlen(DCE2_SOPT__NO_AUTODETECT_HTTP_PROXY_PORTS) &&
             strncasecmp(DCE2_SOPT__NO_AUTODETECT_HTTP_PROXY_PORTS, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__NO_AUTODETECT_HTTP_PROXY_PORTS;
    }
    else if (opt_len == strlen(DCE2_SOPT__SMB_INVALID_SHARES) &&
             strncasecmp(DCE2_SOPT__SMB_INVALID_SHARES, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__SMB_INVALID_SHARES;
    }
    else if (opt_len == strlen(DCE2_SOPT__SMB_MAX_CHAIN) &&
             strncasecmp(DCE2_SOPT__SMB_MAX_CHAIN, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__SMB_MAX_CHAIN;
    }
    else if (opt_len == strlen(DCE2_SOPT__SMB2_MAX_COMPOUND) &&
             strncasecmp(DCE2_SOPT__SMB2_MAX_COMPOUND, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__SMB2_MAX_COMPOUND;
    }
    else if (opt_len == strlen(DCE2_SOPT__SMB_FILE_INSPECTION) &&
             strncasecmp(DCE2_SOPT__SMB_FILE_INSPECTION, opt_start, opt_len) == 0)
    {
        opt_flag = DCE2_SC_OPT_FLAG__SMB_FILE_INSPECTION;
    }
    else
    {
        DCE2_ScError("Invalid option: \"%.*s\"", opt_len, opt_start);
        return DCE2_SC_OPT_FLAG__NULL;
    }

    if (DCE2_CheckAndSetMask(opt_flag, opt_mask) != DCE2_RET__SUCCESS)
    {
        DCE2_ScError("Can only configure option once: \"%.*s\"", opt_len, opt_start);
        return DCE2_SC_OPT_FLAG__NULL;
    }

    return opt_flag;
}

/********************************************************************
 * Function: DCE2_ScParsePolicy()
 *
 * Parses the argument given to the policy option.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the policy argument.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          value for the policy.
 *      DCE2_RET__ERROR if an error occured in parsing the policy.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParsePolicy(DCE2_ServerConfig *sc, char **ptr, char *end)
{
    DCE2_WordListState state = DCE2_WORD_LIST_STATE__START;
    char *policy_start = *ptr;
    char last_char = 0;

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_WORD_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_WORD_LIST_STATE__START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    policy_start = *ptr;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", DCE2_SOPT__POLICY, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    size_t policy_len = *ptr - policy_start;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                     DCE2_SOPT__POLICY, *ptr - policy_start, policy_start);
                        return DCE2_RET__ERROR;
                    }

                    if (policy_len == strlen(DCE2_SARG__POLICY_WIN2000) &&
                        strncasecmp(DCE2_SARG__POLICY_WIN2000, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__WIN2000;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_WINXP) &&
                             strncasecmp(DCE2_SARG__POLICY_WINXP, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__WINXP;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_WINVISTA) &&
                             strncasecmp(DCE2_SARG__POLICY_WINVISTA, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__WINVISTA;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_WIN2003) &&
                             strncasecmp(DCE2_SARG__POLICY_WIN2003, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__WIN2003;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_WIN2008) &&
                             strncasecmp(DCE2_SARG__POLICY_WIN2008, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__WIN2008;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_WIN7) &&
                             strncasecmp(DCE2_SARG__POLICY_WIN7, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__WIN7;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_SAMBA) &&
                             strncasecmp(DCE2_SARG__POLICY_SAMBA, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__SAMBA;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_SAMBA_3_0_37) &&
                             strncasecmp(DCE2_SARG__POLICY_SAMBA_3_0_37, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__SAMBA_3_0_37;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_SAMBA_3_0_22) &&
                             strncasecmp(DCE2_SARG__POLICY_SAMBA_3_0_22, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__SAMBA_3_0_22;
                    }
                    else if (policy_len == strlen(DCE2_SARG__POLICY_SAMBA_3_0_20) &&
                             strncasecmp(DCE2_SARG__POLICY_SAMBA_3_0_20, policy_start, policy_len) == 0)
                    {
                        sc->policy = DCE2_POLICY__SAMBA_3_0_20;
                    }
                    else
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                     DCE2_SOPT__POLICY, policy_len, policy_start);
                        return DCE2_RET__ERROR;
                    }

                    state = DCE2_WORD_LIST_STATE__END;
                    continue;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid policy state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        (*ptr)++;
    }

    if (state != DCE2_WORD_LIST_STATE__END)
    {
        DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", DCE2_SOPT__POLICY, *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScParseDetect()
 *
 * Parses the arguments to the detect or autodetect options.  These
 * options take the same arguments.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the policy argument.
 *  char *
 *      Pointer to the end of the configuration line.
 *  int
 *      Non-zero if we're configuring for the autodetect option.
 *      Zero if we're configuring for the detect option.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          arguments for the detect or autodetect options.
 *      DCE2_RET__ERROR if an error occured in parsing the detect
 *          or autodetect arguments.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseDetect(DCE2_ServerConfig *sc, char **ptr, char *end, int autodetect)
{
    DCE2_DetectListState state = DCE2_DETECT_LIST_STATE__START;
    char *type_start = *ptr;
    uint8_t *port_array = NULL;
    int dmask = DCE2_DETECT_FLAG__NULL;
    int one_type = 0;
    char last_char = 0;
    DCE2_DetectFlag dflag = DCE2_DETECT_FLAG__NULL;
    char *option_str = autodetect ? DCE2_SOPT__AUTODETECT : DCE2_SOPT__DETECT;

    DCE2_ScResetPortsArrays(sc, autodetect);

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_DETECT_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_DETECT_LIST_STATE__START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    type_start = *ptr;
                    one_type = 1;
                    state = DCE2_DETECT_LIST_STATE__TYPE;
                }
                else if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_DETECT_LIST_STATE__TYPE_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", option_str, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_DETECT_LIST_STATE__TYPE_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    type_start = *ptr;
                    state = DCE2_DETECT_LIST_STATE__TYPE;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", option_str, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_DETECT_LIST_STATE__TYPE:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                     option_str, *ptr - type_start, type_start);
                        return DCE2_RET__ERROR;
                    }

                    dflag = DCE2_ScParseDetectType(type_start, *ptr, &dmask);

                    switch (dflag)
                    {
                        case DCE2_DETECT_FLAG__NONE:
                            /* This can only be used by itself */
                            if (dmask != DCE2_DETECT_FLAG__NONE)
                            {
                                DCE2_ScError("Can only use \"%s\" type \"%s\" "
                                             "by itself", option_str, DCE2_SARG__DETECT_NONE);
                                return DCE2_RET__ERROR;
                            }

                            return DCE2_RET__SUCCESS;

                        case DCE2_DETECT_FLAG__SMB:
                            if (autodetect)
                                port_array = sc->auto_smb_ports;
                            else
                                port_array = sc->smb_ports;

                            break;

                        case DCE2_DETECT_FLAG__TCP:
                            if (autodetect)
                                port_array = sc->auto_tcp_ports;
                            else
                                port_array = sc->tcp_ports;

                            break;

                        case DCE2_DETECT_FLAG__UDP:
                            if (autodetect)
                                port_array = sc->auto_udp_ports;
                            else
                                port_array = sc->udp_ports;

                            break;

                        case DCE2_DETECT_FLAG__HTTP_PROXY:
                            if (autodetect)
                                port_array = sc->auto_http_proxy_ports;
                            else
                                port_array = sc->http_proxy_ports;

                            break;

                        case DCE2_DETECT_FLAG__HTTP_SERVER:
                            if (autodetect)
                                port_array = sc->auto_http_server_ports;
                            else
                                port_array = sc->http_server_ports;

                            break;

                        default:
                            return DCE2_RET__ERROR;
                    }

                    state = DCE2_DETECT_LIST_STATE__TYPE_END;
                    continue;
                }

                break;

            case DCE2_DETECT_LIST_STATE__TYPE_END:
                if (DCE2_IsSpaceChar(c))
                {
                    /* Guess that a port list will be next */
                    state = DCE2_DETECT_LIST_STATE__PORTS_START;
                }
                else if (one_type)
                {
                    return DCE2_ScInitPortArray(sc, dflag, autodetect);
                }
                else if (DCE2_IsListSepChar(c))
                {
                    if (DCE2_ScInitPortArray(sc, dflag, autodetect) != DCE2_RET__SUCCESS)
                        return DCE2_RET__ERROR;

                    state = DCE2_DETECT_LIST_STATE__TYPE_START;
                }
                else if (DCE2_IsListEndChar(c))
                {
                    if (DCE2_ScInitPortArray(sc, dflag, autodetect) != DCE2_RET__SUCCESS)
                        return DCE2_RET__ERROR;

                    state = DCE2_DETECT_LIST_STATE__END;
                }
                else
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", option_str, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_DETECT_LIST_STATE__PORTS_START:
                if (DCE2_IsPortChar(c) ||
                    DCE2_IsPortRangeChar(c) ||
                    DCE2_IsListStartChar(c))
                {
                    state = DCE2_DETECT_LIST_STATE__PORTS;
                    continue;
                }
                else if (one_type)
                {
                    if (!DCE2_IsSpaceChar(c))
                    {
                        return DCE2_ScInitPortArray(sc, dflag, autodetect);
                    }
                }
                else if (DCE2_IsListSepChar(c))
                {
                    if (DCE2_ScInitPortArray(sc, dflag, autodetect) != DCE2_RET__SUCCESS)
                        return DCE2_RET__ERROR;

                    state = DCE2_DETECT_LIST_STATE__TYPE_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", option_str, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_DETECT_LIST_STATE__PORTS:
                if (DCE2_ParsePortList(ptr, end, port_array) != DCE2_RET__SUCCESS)
                    return DCE2_RET__ERROR;

                state = DCE2_DETECT_LIST_STATE__PORTS_END;
                continue;

            case DCE2_DETECT_LIST_STATE__PORTS_END:
                if (one_type)
                {
                    return DCE2_RET__SUCCESS;
                }
                else if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_DETECT_LIST_STATE__END;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_DETECT_LIST_STATE__TYPE_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", option_str, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid %s state: %d",
                         __FILE__, __LINE__, option_str, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        (*ptr)++;
    }

    if (state != DCE2_DETECT_LIST_STATE__END)
    {
        DCE2_ScError("Invalid \"%s\" syntax: \"%s\"", option_str, *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScResetPortArrays()
 *
 * Clears all of the port bits in the specified port array masks
 * for the passed in server configuration.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  int
 *      Non-zero if we should clear the autodetect port bits.
 *      Zero if we should clear the detect port bits.
 *
 * Returns: None
 *
 ********************************************************************/
static inline void DCE2_ScResetPortsArrays(DCE2_ServerConfig *sc, int autodetect)
{
    if (!autodetect)
    {
        DCE2_ClearPorts(sc->smb_ports);
        DCE2_ClearPorts(sc->tcp_ports);
        DCE2_ClearPorts(sc->udp_ports);
        DCE2_ClearPorts(sc->http_proxy_ports);
        DCE2_ClearPorts(sc->http_server_ports);
    }
    else
    {
        DCE2_ClearPorts(sc->auto_smb_ports);
        DCE2_ClearPorts(sc->auto_tcp_ports);
        DCE2_ClearPorts(sc->auto_udp_ports);
        DCE2_ClearPorts(sc->auto_http_proxy_ports);
        DCE2_ClearPorts(sc->auto_http_server_ports);
    }
}

/********************************************************************
 * Function: DCE2_ScParseDetectType()
 *
 * Parses the detect type or transport argument to the detect or
 * autodetect options.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character of the detect type name.
 *  char *
 *      Pointer to the byte after the last character of
 *      the detect type name.
 *  int
 *      Pointer to the current detect type mask.  Contains bits
 *      set for each detect type that has already been configured.
 *      Mask is checked and updated for new option.
 *
 * Returns:
 *  DCE2_DetectFlag
 *      Flag for the detect type or transport.
 *      NULL flag if not a valid detect type or detect type has
 *          already been configured.
 *
 ********************************************************************/
static inline DCE2_DetectFlag DCE2_ScParseDetectType(char *start, char *end, int *dmask)
{
    DCE2_DetectFlag dflag = DCE2_DETECT_FLAG__NULL;
    size_t dtype_len = end - start;

    if (dtype_len == strlen(DCE2_SARG__DETECT_SMB)
            && strncasecmp(DCE2_SARG__DETECT_SMB, start, dtype_len) == 0)
    {
        dflag = DCE2_DETECT_FLAG__SMB;
    }
    else if (dtype_len == strlen(DCE2_SARG__DETECT_TCP)
            && strncasecmp(DCE2_SARG__DETECT_TCP, start, dtype_len) == 0)
    {
        dflag = DCE2_DETECT_FLAG__TCP;
    }
    else if (dtype_len == strlen(DCE2_SARG__DETECT_UDP)
            && strncasecmp(DCE2_SARG__DETECT_UDP, start, dtype_len) == 0)
    {
        dflag = DCE2_DETECT_FLAG__UDP;
    }
    else if (dtype_len == strlen(DCE2_SARG__DETECT_HTTP_PROXY)
            && strncasecmp(DCE2_SARG__DETECT_HTTP_PROXY, start, dtype_len) == 0)
    {
        dflag = DCE2_DETECT_FLAG__HTTP_PROXY;
    }
    else if (dtype_len == strlen(DCE2_SARG__DETECT_HTTP_SERVER)
            && strncasecmp(DCE2_SARG__DETECT_HTTP_SERVER, start, dtype_len) == 0)
    {
        dflag = DCE2_DETECT_FLAG__HTTP_SERVER;
    }
    else if (dtype_len == strlen(DCE2_SARG__DETECT_NONE)
            && strncasecmp(DCE2_SARG__DETECT_NONE, start, dtype_len) == 0)
    {
        dflag = DCE2_DETECT_FLAG__NONE;
    }
    else
    {
        DCE2_ScError("Invalid detect/autodetect type: \"%.*s\"", dtype_len, start);
        return DCE2_DETECT_FLAG__NULL;
    }

    if (DCE2_CheckAndSetMask(dflag, dmask) != DCE2_RET__SUCCESS)
    {
        DCE2_ScError("Can only configure option once: \"%.*s\"", dtype_len, start);
        return DCE2_DETECT_FLAG__NULL;
    }

    return dflag;
}

/********************************************************************
 * Function: DCE2_ScParseSmbShares()
 *
 * Parses shares given to the smb shares option.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the smb shares arguments.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          arguments to the smb shares option
 *      DCE2_RET__ERROR if an error occured in parsing the smb
 *          shares option.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseSmbShares(DCE2_ServerConfig *sc, char **ptr, char *end)
{
    DCE2_WordListState state = DCE2_WORD_LIST_STATE__START;
    char *share_start = *ptr;
    int one_share = 0;
    int quote = 0;


    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_WORD_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_WORD_LIST_STATE__START:
                if (DCE2_IsQuoteChar(c))
                {
                    quote ^= 1;
                    one_share = 1;
                    state = DCE2_WORD_LIST_STATE__QUOTE;
                }
                else if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__WORD_START;
                }
                else if (DCE2_IsGraphChar(c))
                {
                    /* Only one share */
                    share_start = *ptr;
                    one_share = 1;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                                 DCE2_SOPT__SMB_INVALID_SHARES, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__QUOTE:
                if (DCE2_IsGraphChar(c))
                {
                    share_start = *ptr;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                                 DCE2_SOPT__SMB_INVALID_SHARES, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD_START:
                if (DCE2_IsQuoteChar(c))
                {
                    quote ^= 1;
                    state = DCE2_WORD_LIST_STATE__QUOTE;
                }
                else if (DCE2_IsGraphChar(c))
                {
                    share_start = *ptr;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                                 DCE2_SOPT__SMB_INVALID_SHARES, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD:
                if (!DCE2_IsGraphChar(c))
                {
                    DCE2_SmbShare *smb_share;
                    DCE2_SmbShare *smb_share_key;
                    int share_len = *ptr - share_start;
                    int i, j;
                    DCE2_Ret status;

                    smb_share = (DCE2_SmbShare *)DCE2_Alloc(sizeof(DCE2_SmbShare), DCE2_MEM_TYPE__CONFIG);
                    smb_share_key = (DCE2_SmbShare *)DCE2_Alloc(sizeof(DCE2_SmbShare), DCE2_MEM_TYPE__CONFIG);
                    if ((smb_share == NULL) || (smb_share_key == NULL))
                    {
                        DCE2_ScSmbShareFree((void *)smb_share);
                        DCE2_ScSmbShareFree((void *)smb_share_key);

                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to allocate memory for smb share.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    smb_share->unicode_str_len = (share_len * 2) + 2;
                    smb_share->unicode_str =
                        (char *)DCE2_Alloc(smb_share->unicode_str_len, DCE2_MEM_TYPE__CONFIG);

                    smb_share->ascii_str_len = share_len + 1;
                    smb_share->ascii_str =
                        (char *)DCE2_Alloc(smb_share->ascii_str_len, DCE2_MEM_TYPE__CONFIG);

                    if ((smb_share->unicode_str == NULL) || (smb_share->ascii_str == NULL))
                    {
                        DCE2_ScSmbShareFree((void *)smb_share);
                        DCE2_ScSmbShareFree((void *)smb_share_key);

                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to allocate memory for smb share.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    for (i = 0, j = 0; i < share_len; i++, j += 2)
                    {
                        smb_share->unicode_str[j] = (char)toupper((int)share_start[i]);
                        smb_share->ascii_str[i] = (char)toupper((int)share_start[i]);
                    }

                    /* Just use ascii share as the key */
                    smb_share_key->ascii_str_len = smb_share->ascii_str_len;
                    smb_share_key->ascii_str =
                        (char *)DCE2_Alloc(smb_share_key->ascii_str_len, DCE2_MEM_TYPE__CONFIG);

                    if (smb_share_key->ascii_str == NULL)
                    {
                        DCE2_ScSmbShareFree((void *)smb_share);
                        DCE2_ScSmbShareFree((void *)smb_share_key);

                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to allocate memory for smb share.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    memcpy(smb_share_key->ascii_str, smb_share->ascii_str, smb_share_key->ascii_str_len);

                    status = DCE2_ListInsert(sc->smb_invalid_shares, (void *)smb_share_key, (void *)smb_share);
                    if (status == DCE2_RET__DUPLICATE)
                    {
                        /* Just free this share and move on */
                        DCE2_ScSmbShareFree((void *)smb_share);
                        DCE2_ScSmbShareFree((void *)smb_share_key);
                    }
                    else if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_ScSmbShareFree((void *)smb_share);
                        DCE2_ScSmbShareFree((void *)smb_share_key);

                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to insert invalid share into list.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    if (one_share)
                    {
                        if (quote && !DCE2_IsQuoteChar(c))
                        {
                            DCE2_ScError("Invalid \"%s\" syntax: Unterminated quoted string",
                                         DCE2_SOPT__SMB_INVALID_SHARES);
                            return DCE2_RET__ERROR;
                        }

                        state = DCE2_WORD_LIST_STATE__END;
                        break;
                    }

                    state = DCE2_WORD_LIST_STATE__WORD_END;
                    continue;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD_END:
                if (quote)
                {
                    if (!DCE2_IsQuoteChar(c))
                    {
                        DCE2_ScError("Invalid \"%s\" syntax: Unterminated quoted string",
                                     DCE2_SOPT__SMB_INVALID_SHARES);
                        return DCE2_RET__ERROR;
                    }

                    quote ^= 1;
                }
                else if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__END;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__WORD_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                                 DCE2_SOPT__SMB_INVALID_SHARES, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Invalid smb shares state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        (*ptr)++;
    }

    if (state != DCE2_WORD_LIST_STATE__END)
    {
        DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                     DCE2_SOPT__SMB_INVALID_SHARES, *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScParseSmbMaxChain()
 *
 * Parses the argument to the smb max chain option.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the smb max chain argument.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          argument to the smb max chain option.
 *      DCE2_RET__ERROR if an error occured in parsing the smb max
 *          chain argument.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseSmbMaxChain(DCE2_ServerConfig *sc, char **ptr, char *end)
{
    DCE2_Ret status;
    uint8_t chain_len;

    status = DCE2_ParseValue(ptr, end, &chain_len, DCE2_INT_TYPE__UINT8);
    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_ScError("Error parsing \"%s\".  Value must be between 0 and %u inclusive",
                     DCE2_SOPT__SMB_MAX_CHAIN, UINT8_MAX);
        return DCE2_RET__ERROR;
    }

    sc->smb_max_chain = chain_len;

    return DCE2_RET__SUCCESS;

}

/********************************************************************
 * Function: DCE2_ScParseSmb2MaxCompound()
 *
 * Parses the argument to the smb2 max compound option.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the smb max chain argument.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          argument to the smb2 max compound option.
 *      DCE2_RET__ERROR if an error occured in parsing the smb2 max
 *          compound argument.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseSmb2MaxCompound(DCE2_ServerConfig *sc, char **ptr, char *end)
{
    DCE2_Ret status;
    uint8_t compound_len;

    status = DCE2_ParseValue(ptr, end, &compound_len, DCE2_INT_TYPE__UINT8);
    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_ScError("Error parsing \"%s\".  Value must be between 0 and %u inclusive",
                     DCE2_SOPT__SMB2_MAX_COMPOUND, UINT8_MAX);
        return DCE2_RET__ERROR;
    }

    sc->smb2_max_compound = compound_len;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScParseValidSmbVersions()
 *
 * Parses the version types for the valid smb versions option and
 * adds to server configuration.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          valid smb versions
 *      DCE2_RET__ERROR if an error occured in parsing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseValidSmbVersions(DCE2_ServerConfig *sc, char **ptr, char *end)
{
    DCE2_WordListState state = DCE2_WORD_LIST_STATE__START;
    char *version_start = *ptr;
    char last_char = 0;
    int one_version = 0;
    int version_mask = 0;

    DCE2_ScClearAllValidSmbVersionFlags(sc);

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_WORD_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_WORD_LIST_STATE__START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    /* Only one valid smb version */
                    version_start = *ptr;
                    one_version = 1;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__WORD_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                            DCE2_SOPT__VALID_SMB_VERSIONS, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    version_start = *ptr;
                    state = DCE2_WORD_LIST_STATE__WORD;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                            DCE2_SOPT__VALID_SMB_VERSIONS, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    DCE2_ValidSmbVersionFlag vflag;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                     DCE2_SOPT__VALID_SMB_VERSIONS,
                                     *ptr - version_start, version_start);
                        return DCE2_RET__ERROR;
                    }


                    vflag = DCE2_ScParseValidSmbVersion(version_start, *ptr, &version_mask);
                    switch (vflag)
                    {
                        case DCE2_VALID_SMB_VERSION_FLAG__NULL:
                            return DCE2_RET__ERROR;

                        case DCE2_VALID_SMB_VERSION_FLAG__ALL:
                            if (!one_version)
                            {
                                DCE2_ScError("Valid smb version \"%s\" cannot be "
                                             "configured in a list", DCE2_SARG__VALID_SMB_VERSIONS_ALL);
                                return DCE2_RET__ERROR;
                            }

                            DCE2_ScSetValidSmbVersion(sc, vflag);
                            break;

                        default:
                            DCE2_ScSetValidSmbVersion(sc, vflag);
                            break;
                    }

                    if (one_version)
                        return DCE2_RET__SUCCESS;

                    state = DCE2_WORD_LIST_STATE__WORD_END;
                    continue;
                }

                break;

            case DCE2_WORD_LIST_STATE__WORD_END:
                if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__END;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_WORD_LIST_STATE__WORD_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                            DCE2_SOPT__VALID_SMB_VERSIONS, *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid valid "
                        "smb versions state: %d", __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        (*ptr)++;
    }

    if (state != DCE2_WORD_LIST_STATE__END)
    {
        DCE2_ScError("Invalid \"%s\" syntax: \"%s\"",
                DCE2_SOPT__VALID_SMB_VERSIONS, *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScParseValidSmbVersion()
 *
 * Parses smb version and returns flag indication the smb version.
 * Checks and sets a bit in a mask to prevent multiple
 * configurations of the same event type.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character of the smb version name.
 *  char *
 *      Pointer to the byte after the last character of
 *      the smb version name.
 *  int
 *      Pointer to the current valid smb versions mask.  Contains
 *      bits set for each smb version that has already been
 *      configured.  Mask is checked and updated for new version.
 *
 * Returns:
 *  DCE2_ValidSmbVersionFlag
 *      Flag indicating the smb version.
 *      DCE2_VALID_SMB_VERSION_FLAG__NULL if no version or multiple
 *          configuration of smb version.
 *
 ********************************************************************/
static inline DCE2_ValidSmbVersionFlag DCE2_ScParseValidSmbVersion(
        char *start, char *end, int *vmask)
{
    DCE2_ValidSmbVersionFlag vflag = DCE2_VALID_SMB_VERSION_FLAG__NULL;
    size_t version_len = end - start;

    if (version_len == strlen(DCE2_SARG__VALID_SMB_VERSIONS_V1) &&
             strncasecmp(DCE2_SARG__VALID_SMB_VERSIONS_V1, start, version_len) == 0)
    {
        vflag = DCE2_VALID_SMB_VERSION_FLAG__V1;
    }
    else if (version_len == strlen(DCE2_SARG__VALID_SMB_VERSIONS_V2) &&
             strncasecmp(DCE2_SARG__VALID_SMB_VERSIONS_V2, start, version_len) == 0)
    {
        vflag = DCE2_VALID_SMB_VERSION_FLAG__V2;
    }
    else if (version_len == strlen(DCE2_SARG__VALID_SMB_VERSIONS_ALL) &&
             strncasecmp(DCE2_SARG__VALID_SMB_VERSIONS_ALL, start, version_len) == 0)
    {
        vflag = DCE2_VALID_SMB_VERSION_FLAG__ALL;
    }
    else
    {
        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                     DCE2_SOPT__VALID_SMB_VERSIONS, version_len, start);
        return DCE2_VALID_SMB_VERSION_FLAG__NULL;
    }

    if (DCE2_CheckAndSetMask((int)vflag, vmask) != DCE2_RET__SUCCESS)
    {
        DCE2_ScError("Valid smb version \"%.*s\" cannot be specified more than once",
                     version_len, start);
        return DCE2_VALID_SMB_VERSION_FLAG__NULL;
    }

    return vflag;
}

/*********************************************************************
 * Function: DCE2_ScSetValidSmbVersion()
 *
 * Sets the valid smb version the user will allow during processing
 * in the server configuration valid smb versions mask.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to server config structure.
 *  DCE2_ValidSmbVersionFlag
 *      The smb version flag to set.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_ScSetValidSmbVersion(DCE2_ServerConfig *sc,
        DCE2_ValidSmbVersionFlag vflag)
{
    sc->valid_smb_versions_mask |= vflag;
}

/*********************************************************************
 * Function: DCE2_ScClearAllValidSmbVersionFlags()
 *
 * Clears all of the bits in the server configuration smb
 * valid versions mask.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to server config structure.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_ScClearAllValidSmbVersionFlags(DCE2_ServerConfig *sc)
{
    sc->valid_smb_versions_mask = DCE2_VALID_SMB_VERSION_FLAG__NULL;
}

/********************************************************************
 * Function: DCE2_ScParseSmbFileInspection()
 *
 * Parses the arguments to the smb_file_inspection option.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the policy argument.
 *  char *
 *      Pointer to the end of the configuration line.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse.
 *      DCE2_RET__ERROR if an error occured in parsing.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScParseSmbFileInspection(DCE2_ServerConfig *sc, char **ptr, char *end)
{
    DCE2_SmbFileListState state = DCE2_SMB_FILE_LIST_STATE__START;
    const char *option = DCE2_SOPT__SMB_FILE_INSPECTION;
    char *option_start = *ptr;
    char *optr;
    int no_list = 0;
    char last_char = 0;

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_SMB_FILE_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_SMB_FILE_LIST_STATE__START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    optr = *ptr;
                    no_list = 1;
                    state = DCE2_SMB_FILE_LIST_STATE__ENABLEMENT;
                }
                else if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_SMB_FILE_LIST_STATE__ENABLEMENT_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                            option, *ptr - option_start, option_start);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__ENABLEMENT_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    optr = *ptr;
                    state = DCE2_SMB_FILE_LIST_STATE__ENABLEMENT;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                            option, *ptr - option_start, option_start);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__ENABLEMENT:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    int olen = *ptr - optr;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                option, *ptr - optr, optr);
                        return DCE2_RET__ERROR;
                    }

                    if ((olen == strlen(DCE2_SARG__SMB_FILE_INSPECTION_ON))
                            && (strncasecmp(DCE2_SARG__SMB_FILE_INSPECTION_ON, optr, olen) == 0))
                    {
                        sc->smb_file_inspection = DCE2_SMB_FILE_INSPECTION__ON;
                    }
                    else if ((olen == strlen(DCE2_SARG__SMB_FILE_INSPECTION_OFF))
                            && (strncasecmp(DCE2_SARG__SMB_FILE_INSPECTION_OFF, optr, olen) == 0))
                    {
                        sc->smb_file_inspection = DCE2_SMB_FILE_INSPECTION__OFF;
                    }
                    else if ((olen == strlen(DCE2_SARG__SMB_FILE_INSPECTION_ONLY))
                            && (strncasecmp(DCE2_SARG__SMB_FILE_INSPECTION_ONLY, optr, olen) == 0))
                    {
                        sc->smb_file_inspection = DCE2_SMB_FILE_INSPECTION__ONLY;
                    }
                    else
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                option, *ptr - optr, optr);
                        return DCE2_RET__ERROR;
                    }

                    state = DCE2_SMB_FILE_LIST_STATE__ENABLEMENT_END;
                    continue;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__ENABLEMENT_END:
                if (no_list)
                {
                    return DCE2_RET__SUCCESS;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_START;
                }
                else if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_SMB_FILE_LIST_STATE__END;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                            option, *ptr - option_start, option_start);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__START))
                {
                    optr = *ptr;
                    state = DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                            option, *ptr - option_start, option_start);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    int olen = *ptr - optr;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                option, *ptr - optr, optr);
                        return DCE2_RET__ERROR;
                    }

                    if ((olen == strlen(DCE2_SARG__SMB_FILE_INSPECTION_DEPTH))
                            && (strncasecmp(DCE2_SARG__SMB_FILE_INSPECTION_DEPTH, optr, olen) == 0))
                    {
                        state = DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE_START;
                    }
                    else
                    {
                        DCE2_ScError("Invalid \"%s\" argument: \"%.*s\"",
                                option, *ptr - optr, optr);
                        return DCE2_RET__ERROR;
                    }

                    continue;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE_START:
                if (DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    optr = *ptr;
                    state = DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                            option, *ptr - option_start, option_start);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE:
                if (!DCE2_IsWordChar(c, DCE2_WORD_CHAR_POSITION__MIDDLE))
                {
                    char *start_value = optr;

                    if (!DCE2_IsWordChar(last_char, DCE2_WORD_CHAR_POSITION__END))
                    {
                        DCE2_ScError("Invalid argument to \"%s\": \"%.*s\".  Value "
                                "must be between -1 and "STDi64".\n",
                                DCE2_SARG__SMB_FILE_INSPECTION_DEPTH,
                                optr - start_value, start_value, INT64_MAX);
                        return DCE2_RET__ERROR;
                    }

                    if (DCE2_ParseValue(&optr, *ptr, &sc->smb_file_depth,
                                DCE2_INT_TYPE__INT64) != DCE2_RET__SUCCESS)
                    {
                        DCE2_ScError("Invalid argument to \"%s\": \"%.*s\".  Value "
                                "must be between -1 and "STDi64".\n",
                                DCE2_SARG__SMB_FILE_INSPECTION_DEPTH,
                                optr - start_value, start_value, INT64_MAX);
                        return DCE2_RET__ERROR;
                    }

                    if ((sc->smb_file_depth < 0) && (sc->smb_file_depth != -1))
                    {
                        DCE2_ScError("Invalid argument to \"%s\": "STDi64".  Value "
                                "must be between -1 and "STDi64".\n",
                                DCE2_SARG__SMB_FILE_INSPECTION_DEPTH,
                                sc->smb_file_depth, INT64_MAX);
                        return DCE2_RET__ERROR;
                    }

                    state = DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE_END;
                    continue;
                }

                break;

            case DCE2_SMB_FILE_LIST_STATE__FILE_DEPTH_VALUE_END:
                if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_SMB_FILE_LIST_STATE__END;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                            option, *ptr - option_start, option_start);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR, "%s(%d) Invalid %s state: %d",
                         __FILE__, __LINE__, option, state);
                return DCE2_RET__ERROR;
        }

        last_char = c;
        (*ptr)++;
    }

    if (state != DCE2_SMB_FILE_LIST_STATE__END)
    {
        DCE2_ScError("Invalid \"%s\" syntax: \"%.*s\"",
                option, *ptr - option_start, option_start);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ScAddToRoutingTable()
 *
 * Adds the server configuration to the appropriate routing table
 * (IPv4 or IPv6) based on the ip addresses and nets (as sfcidr_t)
 * from the passed in queue.  A pointer to the server configuration
 * is saved in the routing table for each ip set.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  DCE2_Queue *
 *      Queue containing the IPs and nets to add to the routing
 *      tables for this server configuration.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully add all
 *          of the IPs and nets to the routing tables for this
 *          server configuration.
 *      DCE2_RET__ERROR if an error occured in trying to add all
 *          of the IPs and nets to the routing tables for this
 *          server configuration.
 *
 ********************************************************************/
static DCE2_Ret DCE2_ScAddToRoutingTable(DCE2_Config *config,
                                         DCE2_ServerConfig *sc, DCE2_Queue *ip_queue)
{
    sfcidr_t *ip;

    if ((config == NULL) || (sc == NULL) || (ip_queue == NULL))
        return DCE2_RET__ERROR;

    for (ip = (sfcidr_t *)DCE2_QueueFirst(ip_queue);
         ip != NULL;
         ip = (sfcidr_t *)DCE2_QueueNext(ip_queue))
    {
        int rt_status;

        if (config->sconfigs == NULL)
        {
            config->sconfigs = sfrt_new(DIR_16_4x4_16x5_4x4, IPv6, 100, 20);
            if (config->sconfigs == NULL)
            {
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d): Failed to create server configuration "
                         "routing table.", __FILE__, __LINE__);
                return DCE2_RET__ERROR;
            }
        }
        else
        {
            DCE2_ServerConfig *conf;

            conf = (DCE2_ServerConfig *)sfrt_search(&ip->addr, config->sconfigs);

            if (conf != NULL)
            {
                DCE2_ScError("\"%s\": Cannot have the same net in different "
                             "server configurations", DCE2_SOPT__NET);
                return DCE2_RET__ERROR;
            }
        }

        rt_status = sfrt_insert(ip, (unsigned char)ip->bits,
                                (void *)sc, RT_FAVOR_SPECIFIC, config->sconfigs);

        if (rt_status != RT_SUCCESS)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                    "%s(%d) Failed to insert net into routing table.",
                    __FILE__, __LINE__);
            return DCE2_RET__ERROR;
        }

        /* This is a count of the number of pointers or references to this
         * server configuration in the routing tables. */
        sc->ref_count++;
    }

    return DCE2_RET__SUCCESS;
}

/*********************************************************************
 * Function: DCE2_ScIpListDataFree()
 *
 * Callback given to the queue for storing sfcidr_t structures.
 *
 * Arguments:
 *  void *
 *      The sfcidr_t structure to free.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_ScIpListDataFree(void *data)
{
    if (data == NULL)
        return;

    DCE2_Free(data, sizeof(sfcidr_t), DCE2_MEM_TYPE__CONFIG);
}

/********************************************************************
 * Function: DCE2_ScGetConfig()
 *
 * Convenience function for run-time retrieving of a server
 * configuration.  Does a lookup in the appropriate routing table
 * based on the IPs in the packet structure.
 *
 * Arguments:
 *  const SFSnortPacket *
 *      Pointer to the packet structure flowing through the system.
 *
 * Returns:
 *  DCE2_ServerConfig *
 *      A pointer to a valid server configuration if a routing
 *          table lookup succeeded.
 *      A pointer to the default server configuration if an entry
 *          in the routing table could not be found.
 *
 ********************************************************************/
const DCE2_ServerConfig * DCE2_ScGetConfig(const SFSnortPacket *p)
{
    const DCE2_ServerConfig *sc = NULL;
    sfaddr_t* ip;

    if (dce2_eval_config == NULL)
        return NULL;

    if (DCE2_SsnFromClient(p))
        ip = GET_DST_IP(((SFSnortPacket *)p));
    else
        ip = GET_SRC_IP(((SFSnortPacket *)p));

    if (dce2_eval_config->sconfigs != NULL)
        sc = sfrt_lookup(ip, dce2_eval_config->sconfigs);

    if (sc == NULL)
        return dce2_eval_config->dconfig;

    return sc;
}

/*********************************************************************
 * Function: DCE2_ScSmbShareCompare()
 *
 * Callback for the list used to hold the invalid smb shares for
 * doing a comparison.  Don't want any duplicates.
 *
 * Arguments:
 *  const void *
 *      The first share name to compare.
 *  const void *
 *      The second share name to compare.
 *
 * Returns:
 *  int
 *       0 if the shares are equal.
 *      -1 if the shares are not equal.
 *
 *********************************************************************/
static int DCE2_ScSmbShareCompare(const void *a, const void *b)
{
    DCE2_SmbShare *ashare = (DCE2_SmbShare *)a;
    DCE2_SmbShare *bshare = (DCE2_SmbShare *)b;

    if ((ashare == NULL) || (bshare == NULL))
        return -1;

    /* Just check the ascii string */
    if (ashare->ascii_str_len != bshare->ascii_str_len)
        return -1;

    if (memcmp(ashare->ascii_str, bshare->ascii_str, ashare->ascii_str_len) == 0)
        return 0;

    /* Only care about equality for dups */
    return -1;
}

/*********************************************************************
 * Function: DCE2_ScSmbShareFree()
 *
 * Callback to the list used to hold the invalid smb shares for
 * freeing the shares.
 *
 * Arguments:
 *  void *
 *      Pointer to the share structure.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_ScSmbShareFree(void *data)
{
    DCE2_SmbShare *smb_share = (DCE2_SmbShare *)data;

    if (smb_share == NULL)
        return;

    DCE2_Free((void *)smb_share->unicode_str, smb_share->unicode_str_len,
              DCE2_MEM_TYPE__CONFIG);
    DCE2_Free((void *)smb_share->ascii_str, smb_share->ascii_str_len,
              DCE2_MEM_TYPE__CONFIG);
    DCE2_Free((void *)smb_share, sizeof(DCE2_SmbShare), DCE2_MEM_TYPE__CONFIG);
}

/********************************************************************
 * Function: DCE2_GcPrintConfig()
 *
 * Prints the DCE/RPC global configuration.
 *
 * Arguments:
 *  DCE2_GlobalConfig *
 *      Pointer to the global configuration structure.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_GcPrintConfig(const DCE2_GlobalConfig *gc)
{
    char events[1000];

    if (gc == NULL)
        return;

    _dpd.logMsg("DCE/RPC 2 Preprocessor Configuration\n");
    _dpd.logMsg("  Global Configuration\n");
    if(gc->disabled)
    {
        _dpd.logMsg("    DCE/RPC 2 Preprocessor: INACTIVE\n");
    }
    _dpd.logMsg("    DCE/RPC Defragmentation: %s\n",
                gc->dce_defrag == DCE2_CS__ENABLED ? "Enabled" : "Disabled");
    if ((gc->dce_defrag == DCE2_CS__ENABLED) && (gc->max_frag_len != DCE2_SENTINEL))
        _dpd.logMsg("    Max DCE/RPC Frag Size: %u bytes\n", gc->max_frag_len);
    _dpd.logMsg("    Memcap: %u KB\n", gc->memcap / 1024);
    if (gc->reassemble_threshold != 0)
        _dpd.logMsg("    Reassemble threshold: %u bytes\n", gc->reassemble_threshold);

    snprintf(events, sizeof(events), "    Events: ");
    events[sizeof(events) - 1] = '\0';

    if (gc->event_mask == DCE2_EVENT_FLAG__NULL)
    {
        strncat(events, DCE2_GARG__EVENTS_NONE, (sizeof(events) - 1) - strlen(events));
    }
    else
    {
        if (gc->event_mask & DCE2_EVENT_FLAG__MEMCAP)
        {
            strncat(events, DCE2_GARG__EVENTS_MEMCAP, (sizeof(events) - 1) - strlen(events));
            strncat(events, " ", (sizeof(events) - 1) - strlen(events));
        }

        if (gc->event_mask & DCE2_EVENT_FLAG__SMB)
        {
            strncat(events, DCE2_GARG__EVENTS_SMB, (sizeof(events) - 1) - strlen(events));
            strncat(events, " ", (sizeof(events) - 1) - strlen(events));
        }

        if (gc->event_mask & DCE2_EVENT_FLAG__CO)
        {
            strncat(events, DCE2_GARG__EVENTS_CO, (sizeof(events) - 1) - strlen(events));
            strncat(events, " ", (sizeof(events) - 1) - strlen(events));
        }

        if (gc->event_mask & DCE2_EVENT_FLAG__CL)
        {
            strncat(events, DCE2_GARG__EVENTS_CL, (sizeof(events) - 1) - strlen(events));
            strncat(events, " ", (sizeof(events) - 1) - strlen(events));
        }
    }

    strncat(events, "\n", (sizeof(events) - 1) - strlen(events));
    _dpd.logMsg(events);

    // Just use the events buffer
    snprintf(events, sizeof(events), "    SMB Fingerprint policy: ");
    if (gc->smb_fingerprint_policy == DCE2_SMB_FINGERPRINT__NONE)
        strncat(events, "Disabled\n", (sizeof(events) - 1) - strlen(events));
    else if (gc->smb_fingerprint_policy ==
            (DCE2_SMB_FINGERPRINT__CLIENT|DCE2_SMB_FINGERPRINT__SERVER))
        strncat(events, "Client and Server\n", (sizeof(events) - 1) - strlen(events));
    else if (gc->smb_fingerprint_policy & DCE2_SMB_FINGERPRINT__CLIENT)
        strncat(events, "Client\n", (sizeof(events) - 1) - strlen(events));
    else if (gc->smb_fingerprint_policy & DCE2_SMB_FINGERPRINT__SERVER)
        strncat(events, "Server\n", (sizeof(events) - 1) - strlen(events));
    _dpd.logMsg(events);
}

/********************************************************************
 * Function: DCE2_ScPrintConfig()
 *
 * Prints a DCE/RPC server configuration.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *  DCE2_Queue *
 *      Queue that holds the nets for printing.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ScPrintConfig(const DCE2_ServerConfig *sc, DCE2_Queue *net_queue)
{
    char *policy = NULL;
    unsigned int i;

    if (sc == NULL)
        return;

    if (!DCE2_QueueIsEmpty(net_queue))
    {
        char nets[80];

        _dpd.logMsg("  Server Configuration\n");

        snprintf(nets, sizeof(nets), "    Net: ");
        nets[sizeof(nets) - 1] = '\0';

        while (!DCE2_QueueIsEmpty(net_queue))
        {
            char *ip_addr;
            uint8_t prefix;
            sfcidr_t *ip;
            char tmp_net[INET6_ADDRSTRLEN + 5];  /* Enough for IPv4 plus netmask or full IPv6 plus prefix */

            ip = (sfcidr_t *)DCE2_QueueDequeue(net_queue);
            ip_addr = sfip_to_str(&ip->addr);
            prefix = (uint8_t)ip->bits;

            DCE2_Free((void *)ip, sizeof(sfcidr_t), DCE2_MEM_TYPE__CONFIG);
            snprintf(tmp_net, sizeof(tmp_net), "%s/%u ", ip_addr, prefix);
            tmp_net[sizeof(tmp_net) - 1] = '\0';

            if ((strlen(nets) + strlen(tmp_net)) >= sizeof(nets))
            {
                _dpd.logMsg("%s\n", nets);
                snprintf(nets, sizeof(nets), "         %s", tmp_net);
                nets[sizeof(nets) - 1] = '\0';
            }
            else
            {
                strncat(nets, tmp_net, (sizeof(nets) - 1) - strlen(nets));
            }
        }

        _dpd.logMsg("%s\n", nets);
    }
    else
    {
        _dpd.logMsg("  Server Default Configuration\n");
    }

    switch (sc->policy)
    {
        case DCE2_POLICY__WIN2000:
            policy = DCE2_SARG__POLICY_WIN2000;
            break;
        case DCE2_POLICY__WINXP:
            policy = DCE2_SARG__POLICY_WINXP;
            break;
        case DCE2_POLICY__WINVISTA:
            policy = DCE2_SARG__POLICY_WINVISTA;
            break;
        case DCE2_POLICY__WIN2003:
            policy = DCE2_SARG__POLICY_WIN2003;
            break;
        case DCE2_POLICY__WIN2008:
            policy = DCE2_SARG__POLICY_WIN2008;
            break;
        case DCE2_POLICY__WIN7:
            policy = DCE2_SARG__POLICY_WIN7;
            break;
        case DCE2_POLICY__SAMBA:
            policy = DCE2_SARG__POLICY_SAMBA;
            break;
        case DCE2_POLICY__SAMBA_3_0_37:
            policy = DCE2_SARG__POLICY_SAMBA_3_0_37;
            break;
        case DCE2_POLICY__SAMBA_3_0_22:
            policy = DCE2_SARG__POLICY_SAMBA_3_0_22;
            break;
        case DCE2_POLICY__SAMBA_3_0_20:
            policy = DCE2_SARG__POLICY_SAMBA_3_0_20;
            break;
        default:
            DCE2_QueueDestroy(net_queue);
            DCE2_Die("%s(%d) Invalid policy: %d",
                     __FILE__, __LINE__, sc->policy);
    }

    _dpd.logMsg("    Policy: %s\n", policy);

    DCE2_ScPrintPorts(sc, 0);

    for (i = 0; i < DCE2_PORTS__MAX; i++)
    {
        if (DCE2_IsPortSet(sc->http_proxy_ports, (uint16_t)i))
        {
            _dpd.logMsg("    Autodetect on RPC over HTTP proxy detect ports: %s\n",
                        sc->autodetect_http_proxy_ports == DCE2_CS__ENABLED ? "Yes" : "No");
            break;
        }
    }

    DCE2_ScPrintPorts(sc, 1);

    for (i = 0; i < DCE2_PORTS__MAX; i++)
    {
        if (DCE2_IsPortSet(sc->smb_ports, (uint16_t)i) ||
            DCE2_IsPortSet(sc->auto_smb_ports, (uint16_t)i))
        {
            break;
        }
    }

    if ((i != DCE2_PORTS__MAX) && (sc->smb_invalid_shares != NULL))
    {
        char share_str[80];
        DCE2_SmbShare *share;

        snprintf(share_str, sizeof(share_str), "    Invalid SMB shares: ");
        share_str[sizeof(share_str) - 1] = '\0';

        for (share = DCE2_ListFirst(sc->smb_invalid_shares);
             share != NULL;
             share = DCE2_ListNext(sc->smb_invalid_shares))
        {
            char *tmp_share;
            unsigned int tmp_share_len;

            /* Ascii string will be NULL terminated.  Also alloc enough for space.
             * Note that if share is longer than the size of the buffer it will be
             * put into, it will be truncated */
            tmp_share_len = strlen(share->ascii_str) + 2;
            tmp_share = (char *)DCE2_Alloc(tmp_share_len, DCE2_MEM_TYPE__CONFIG);
            if (tmp_share == NULL)
            {
                DCE2_QueueDestroy(net_queue);
                DCE2_Die("%s(%d) Failed to allocate memory for printing "
                         "configuration.", __FILE__, __LINE__);
            }

            snprintf(tmp_share, tmp_share_len, "%s ", share->ascii_str);
            tmp_share[tmp_share_len - 1] = '\0';

            if ((strlen(share_str) + strlen(tmp_share)) >= sizeof(share_str))
            {
                _dpd.logMsg("%s\n", share_str);
                snprintf(share_str, sizeof(share_str), "                        %s", tmp_share);
                share_str[sizeof(share_str) - 1] = '\0';
            }
            else
            {
                strncat(share_str, tmp_share, (sizeof(share_str) - 1) - strlen(share_str));
            }

            DCE2_Free((void *)tmp_share, tmp_share_len, DCE2_MEM_TYPE__CONFIG);
        }

        _dpd.logMsg("%s\n", share_str);
    }

    if (i != DCE2_PORTS__MAX)
    {
        if (sc->smb_max_chain == 0)
            _dpd.logMsg("    Maximum SMB command chaining: Unlimitied\n");
        else if (sc->smb_max_chain == 1)
            _dpd.logMsg("    Maximum SMB command chaining: No chaining allowed\n");
        else
            _dpd.logMsg("    Maximum SMB command chaining: %u commands\n", sc->smb_max_chain);

        if (!DCE2_ScSmbFileInspection(sc))
        {
            _dpd.logMsg("    SMB file inspection: Disabled\n");
        }
        else
        {
            int64_t file_depth = DCE2_ScSmbFileDepth(sc);

            if (DCE2_ScSmbFileInspectionOnly(sc))
                _dpd.logMsg("    SMB file inspection: Only\n");
            else
                _dpd.logMsg("    SMB file inspection: Enabled\n");

            if (file_depth == -1)
                _dpd.logMsg("      File depth: Disabled\n");
            else if (file_depth == 0)
                _dpd.logMsg("      File depth: Unlimited\n");
            else
                _dpd.logMsg("      File depth: "STDi64"\n", file_depth);
        }
    }
}

/*********************************************************************
 * Function: DCE2_ScPrintPorts()
 *
 * Used for gathering the bits set in a detect or autodetect port
 * array mask and displaying in a readable form.
 *
 * Arguments:
 *  const uint8_t *
 *      The port array mask to get the set bits from.
 *  char *
 *      An array to print the readable string to.
 *  int
 *      The size of the array to print the readable string to.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_ScPrintPorts(const DCE2_ServerConfig *sc, int autodetect)
{
    unsigned int pps_idx;
    DCE2_PrintPortsStruct pps[5];

    pps[0].trans_str = "SMB";
    pps[1].trans_str = "TCP";
    pps[2].trans_str = "UDP";
    pps[3].trans_str = "RPC over HTTP server";
    pps[4].trans_str = "RPC over HTTP proxy";

    if (!autodetect)
    {
        pps[0].port_array = sc->smb_ports;
        pps[1].port_array = sc->tcp_ports;
        pps[2].port_array = sc->udp_ports;
        pps[3].port_array = sc->http_server_ports;
        pps[4].port_array = sc->http_proxy_ports;

        if (_dpd.isPafEnabled())
            _dpd.logMsg("    Detect ports (PAF)\n");
        else
            _dpd.logMsg("    Detect ports\n");
    }
    else
    {
        pps[0].port_array = sc->auto_smb_ports;
        pps[1].port_array = sc->auto_tcp_ports;
        pps[2].port_array = sc->auto_udp_ports;
        pps[3].port_array = sc->auto_http_server_ports;
        pps[4].port_array = sc->auto_http_proxy_ports;

        if (_dpd.isPafEnabled())
            _dpd.logMsg("    Autodetect ports (PAF)\n");
        else
            _dpd.logMsg("    Autodetect ports\n");
    }

    for (pps_idx = 0; pps_idx < sizeof(pps) / sizeof(DCE2_PrintPortsStruct); pps_idx++)
    {
        int port_start = 1;
        unsigned int start_port = 0, end_port = 0;
        unsigned int i;
        char ports[80];
        int got_port = 0;
        const uint8_t *port_mask_array;

        snprintf(ports, sizeof(ports), "      %s: ", pps[pps_idx].trans_str);
        ports[sizeof(ports) - 1] = '\0';
        port_mask_array = pps[pps_idx].port_array;

        for (i = 0; i < DCE2_PORTS__MAX; i++)
        {
            if (port_start)
            {
                if (DCE2_IsPortSet(port_mask_array, (uint16_t)i))
                {
                    start_port = i;
                    end_port = i;
                    port_start = 0;
                    got_port = 1;
                }
            }

            if (!port_start)
            {
                if (!DCE2_IsPortSet(port_mask_array, (uint16_t)i) || (i == (DCE2_PORTS__MAX - 1)))
                {
                    char tmp_port[15];  /* big enough to hold a full port range */

                    if (i == (DCE2_PORTS__MAX - 1) && DCE2_IsPortSet(port_mask_array, (uint16_t)i))
                        end_port = i;

                    /* Only print range if more than 2 ports */
                    if ((start_port + 1) < end_port)
                    {
                        snprintf(tmp_port, sizeof(tmp_port), "%u-%u ", start_port, end_port);
                        tmp_port[sizeof(tmp_port) - 1] = '\0';
                    }
                    else if (start_port < end_port)
                    {
                        snprintf(tmp_port, sizeof(tmp_port), "%u %u ", start_port, end_port);
                        tmp_port[sizeof(tmp_port) - 1] = '\0';
                    }
                    else
                    {
                        snprintf(tmp_port, sizeof(tmp_port), "%u ", start_port);
                        tmp_port[sizeof(tmp_port) - 1] = '\0';
                    }

                    if ((strlen(ports) + strlen(tmp_port)) >= sizeof(ports))
                    {
                        _dpd.logMsg("%s\n", ports);
                        snprintf(ports, sizeof(ports), "           %s", tmp_port);
                        ports[sizeof(ports) - 1] = '\0';
                    }
                    else
                    {
                        strncat(ports, tmp_port, (sizeof(ports) - 1) - strlen(ports));
                    }

                    port_start = 1;
                }
                else
                {
                    end_port = i;
                }
            }
        }

        if (got_port)
        {
            _dpd.logMsg("%s\n", ports);
        }
        else
        {
            strncat(ports, "None", (sizeof(ports) - 1) - strlen(ports));
            _dpd.logMsg("%s\n", ports);
        }
    }
}

/*********************************************************************
 * Function: DCE2_ScCheckTransport()
 *
 * Makes sure at least one transport for detect or autodetect.  If
 * not there is no sense in running this policy since no detection
 * will ever be done.
 *
 * Arguments:
 *  void *
 *      Pointer to a server configuration structure.
 *
 * Returns: -1 on error
 *
 *********************************************************************/
static int DCE2_ScCheckTransport(void *data)
{
    unsigned int i;
    DCE2_ServerConfig *sc = (DCE2_ServerConfig *)data;
    uint32_t *smb_ports = (uint32_t *)sc->smb_ports;
    uint32_t *tcp_ports = (uint32_t *)sc->tcp_ports;
    uint32_t *udp_ports = (uint32_t *)sc->udp_ports;
    uint32_t *http_proxy_ports = (uint32_t *)sc->http_proxy_ports;
    uint32_t *http_server_ports = (uint32_t *)sc->http_server_ports;
    uint32_t *auto_smb_ports = (uint32_t *)sc->auto_smb_ports;
    uint32_t *auto_tcp_ports = (uint32_t *)sc->auto_tcp_ports;
    uint32_t *auto_udp_ports = (uint32_t *)sc->auto_udp_ports;
    uint32_t *auto_http_proxy_ports = (uint32_t *)sc->auto_http_proxy_ports;
    uint32_t *auto_http_server_ports = (uint32_t *)sc->auto_http_server_ports;

    if (data == NULL)
        return 0;

    for (i = 0; i < DCE2_PORTS__MAX_INDEX >> 2; i++)
    {
        if (smb_ports[i] ||
            tcp_ports[i] ||
            udp_ports[i] ||
            http_proxy_ports[i] ||
            http_server_ports[i] ||
            auto_smb_ports[i] ||
            auto_tcp_ports[i] ||
            auto_udp_ports[i] ||
            auto_http_proxy_ports[i] ||
            auto_http_server_ports[i])
        {
            return 0;
        }
    }

    DCE2_Log(DCE2_LOG_TYPE__WARN, "%s: Must have at least one detect or autodetect transport "
             "enabled for a server configuration if target-based/attribute-"
             "table/adaptive-profiles is not enabled. However, if specific "
             "server configurations are configured, the default server "
             "configuration does not need to have any detect/autodetect "
             "transports configured.", DCE2_SNAME);
    return -1;
}

/*********************************************************************
 * Function: DCE2_ScCheckTransports()
 *
 * Makes sure at least one transport for detect or autodetect.  If
 * not there is no sense in running this policy since no detection
 * will ever be done.
 *
 * Arguments: None
 *
 * Returns: -1 on error
 *
 *********************************************************************/
int DCE2_ScCheckTransports(DCE2_Config *config)
{
    if (config == NULL)
        return 0;

    if (config->sconfigs == NULL)
        return DCE2_ScCheckTransport(config->dconfig);
    return sfrt_iterate2(config->sconfigs, DCE2_ScCheckTransport);
}

/*********************************************************************
 * Function: DCE2_ScCheckPortOverlap()
 *
 * Makes sure there are no overlapping detect ports configured.
 * It's okay for overlap between TCP and UDP.
 * Transports SMB, TCP, RPC over HTTP proxy and RPC over HTTP server
 * cannot define ports that overlap.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if no overlapping ports.
 *      DCE2_RET__ERROR if overlapping ports.
 *
 *********************************************************************/
static DCE2_Ret DCE2_ScCheckPortOverlap(const DCE2_ServerConfig *sc)
{
    unsigned int i;
    uint32_t *smb_ports = (uint32_t *)sc->smb_ports;
    uint32_t *tcp_ports = (uint32_t *)sc->tcp_ports;
    uint32_t *http_proxy_ports = (uint32_t *)sc->http_proxy_ports;
    uint32_t *http_server_ports = (uint32_t *)sc->http_server_ports;

    /* All port array masks should be the same size */
    for (i = 0; i < sizeof(sc->smb_ports) >> 2; i++)
    {
        /* Take 4 bytes at a time and bitwise and them.  Should
         * be 0 if there are no overlapping ports. */
        uint32_t overlap = smb_ports[i] & tcp_ports[i];
        uint32_t cached;

        if (overlap)
        {
            DCE2_ScError("Cannot have overlapping detect ports in "
                         "smb, tcp, rpc-over-http-proxy or rpc-over-http-server. "
                         "Overlapping port detected in tcp ports");
            return DCE2_RET__ERROR;
        }

        cached = smb_ports[i] | tcp_ports[i];
        overlap = http_proxy_ports[i] & cached;

        if (overlap)
        {
            DCE2_ScError("Cannot have overlapping detect ports in "
                         "smb, tcp, rpc-over-http-proxy or rpc-over-http-server. "
                         "Overlapping port detected in rpc-over-http-proxy ports");
            return DCE2_RET__ERROR;
        }

        cached |= http_proxy_ports[i];
        overlap = http_server_ports[i] & cached;

        if (overlap)
        {
            DCE2_ScError("Cannot have overlapping detect ports in "
                         "smb, tcp, rpc-over-http-proxy or rpc-over-http-server. "
                         "Overlapping port detected in rpc-over-http-server ports");
            return DCE2_RET__ERROR;
        }
    }

    return DCE2_RET__SUCCESS;
}

/*********************************************************************
 * Function: DCE2_RegisterPortsWithSession()
 *
 * Add all detect ports to session dispatch table so the DCERPC2
 * preprocessor is dispatched for all interested ports
 *
 * Arguments:
 *  _SnortConfig *
 *      Pointer to Snort Configuration structure.
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *
 * Returns: None
 *
 *********************************************************************/
void DCE2_RegisterPortsWithSession( struct _SnortConfig *sc, DCE2_ServerConfig *policy )
{
    int port;
    uint8_t ports[DCE2_PORTS__MAX_INDEX];

    // create bitmap of all ports set across all protocols
    for( port = 0; port < DCE2_PORTS__MAX_INDEX; port++ )
        ports[ port ] = policy->smb_ports[ port ] | policy->tcp_ports[ port ] |
                        policy->udp_ports[ port ] | policy->http_proxy_ports[ port ] |
                        policy->http_server_ports[ port ] | policy->auto_smb_ports[ port ] |
                        policy->auto_tcp_ports[ port ] | policy->auto_udp_ports[ port ] |
                        policy->auto_http_proxy_ports[ port ] | policy->auto_http_server_ports[ port ];

    // for every port enabled for any protocol, register for dispatch
    for (port = 0; port < DCE2_PORTS__MAX; port++)
        if (DCE2_IsPortSet(ports, (uint16_t)port))
            _dpd.sessionAPI->enable_preproc_for_port( sc,
                                                      PP_DCE2,
                                                      PROTO_BIT__TCP | PROTO_BIT__UDP,
                                                      port );
}

/*********************************************************************
 * Function: DCE2_AddPortsToStreamFilter()
 *
 * Add all detect ports to stream5 filter so stream sessions are
 * created.  Don't do autodetect ports and rely on rules to set
 * any off ports.  This is mainly necessary for SMB ports where
 * we are looking for SMB vulnerabilities in the preprocessor.
 *
 * Arguments:
 *  DCE2_ServerConfig *
 *      Pointer to a server configuration structure.
 *
 * Returns: None
 *
 *********************************************************************/
static void DCE2_AddPortsToStreamFilter(struct _SnortConfig *snortConf, DCE2_ServerConfig *sc, tSfPolicyId policy_id)
{
    unsigned int port;

    for (port = 0; port < DCE2_PORTS__MAX; port++)
    {
        if (DCE2_IsPortSet(sc->smb_ports, (uint16_t)port))
        {
            _dpd.streamAPI->set_port_filter_status
                (snortConf, IPPROTO_TCP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);
        }

        if (DCE2_IsPortSet(sc->tcp_ports, (uint16_t)port))
        {
            _dpd.streamAPI->set_port_filter_status
                (snortConf, IPPROTO_TCP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);
        }

        if (DCE2_IsPortSet(sc->udp_ports, (uint16_t)port))
        {
            _dpd.streamAPI->set_port_filter_status
                (snortConf, IPPROTO_UDP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);
        }

        if (DCE2_IsPortSet(sc->http_proxy_ports, (uint16_t)port))
        {
            _dpd.streamAPI->set_port_filter_status
                (snortConf, IPPROTO_TCP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);
        }

        if (DCE2_IsPortSet(sc->http_server_ports, (uint16_t)port))
        {
            _dpd.streamAPI->set_port_filter_status
                (snortConf, IPPROTO_TCP, (uint16_t)port, PORT_MONITOR_SESSION, policy_id, 1);
        }
    }
}

/********************************************************************
 * Function: DCE2_ParseIpList()
 *
 * Parses an IP list, creates sfcidr_t for each IP address/net and
 * adds to a queue.
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the IP list.
 *  char *
 *      Pointer to the end of the string.
 *  DCE2_Queue *
 *      Queue to store the sfcidr_t structures.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          IP list.
 *      DCE2_RET__ERROR if an error occured in parsing the IP list.
 *
 ********************************************************************/
DCE2_Ret DCE2_ParseIpList(char **ptr, char *end, DCE2_Queue *ip_queue)
{
    DCE2_IpListState state = DCE2_IP_LIST_STATE__START;
    sfcidr_t ip;

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_IP_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_IP_LIST_STATE__START:
                if (DCE2_IsIpChar(c))
                {
                    DCE2_Ret status = DCE2_ParseIp(ptr, end, &ip);
                    sfcidr_t *ip_copy;

                    if (status != DCE2_RET__SUCCESS)
                        return DCE2_RET__ERROR;

                    ip_copy = (sfcidr_t *)DCE2_Alloc(sizeof(sfcidr_t), DCE2_MEM_TYPE__CONFIG);
                    if (ip_copy == NULL)
                    {
                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to allocate memory for IP structure.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    memcpy((void *)ip_copy, (void *)&ip, sizeof(sfcidr_t));

                    status = DCE2_QueueEnqueue(ip_queue, ip_copy);
                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_Free((void *)ip_copy, sizeof(sfcidr_t), DCE2_MEM_TYPE__CONFIG);
                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to queue an IP structure.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    return DCE2_RET__SUCCESS;
                }
                else if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_IP_LIST_STATE__IP_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid IP list: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_IP_LIST_STATE__IP_START:
                if (DCE2_IsIpChar(c))
                {
                    DCE2_Ret status = DCE2_ParseIp(ptr, end, &ip);
                    sfcidr_t *ip_copy;

                    if (status != DCE2_RET__SUCCESS)
                        return DCE2_RET__ERROR;

                    ip_copy = (sfcidr_t *)DCE2_Alloc(sizeof(sfcidr_t), DCE2_MEM_TYPE__CONFIG);
                    if (ip_copy == NULL)
                    {
                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to allocate memory for IP structure.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    memcpy((void *)ip_copy, (void *)&ip, sizeof(sfcidr_t));

                    status = DCE2_QueueEnqueue(ip_queue, ip_copy);
                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_Free((void *)ip_copy, sizeof(sfcidr_t), DCE2_MEM_TYPE__CONFIG);
                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to queue an IP structure.",
                                 __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    state = DCE2_IP_LIST_STATE__IP_END;
                    continue;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid IP list: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_IP_LIST_STATE__IP_END:
                if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_IP_LIST_STATE__END;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_IP_LIST_STATE__IP_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid IP list: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Invalid IP list state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        (*ptr)++;
    }

    if (state != DCE2_IP_LIST_STATE__END)
    {
        DCE2_ScError("Invalid IP list: \"%s\"", *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ParseIP()
 *
 * Parses an IP address or net.  Gobbles up ':', '.', '/' and hex
 * digits.  Not very smart, but we'll let sfip_pton take care of it.
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the string
 *      being parsed.  This is updated to the current position
 *      after parsing the IP.
 *  char *
 *      Pointer to the end of the string.
 *  sfcidr_t *
 *      Pointer to an sfcidr_t structure that should be filled in
 *      based on the IP or net parsed.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          IP address or net.
 *      DCE2_RET__ERROR if an error occured in parsing the IP
 *          address or net.
 *
 ********************************************************************/
DCE2_Ret DCE2_ParseIp(char **ptr, char *end, sfcidr_t *ip)
{
    DCE2_IpState state = DCE2_IP_STATE__START;
    char *ip_start = NULL;
    char ip_addr[INET6_ADDRSTRLEN + 5];     /* Enough for IPv4 plus netmask or
                                               full IPv6 plus prefix */

    memset(ip_addr, 0, sizeof(ip_addr));

    while (*ptr < end)
    {
        char c = **ptr;

        switch (state)
        {
            case DCE2_IP_STATE__START:
                if (DCE2_IsIpChar(c))
                {
                    ip_start = *ptr;
                    state = DCE2_IP_STATE__IP;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid IP address: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_IP_STATE__IP:
                if (!DCE2_IsIpChar(c))
                {
                    int copy_len = *ptr - ip_start;
                    DCE2_Ret status = DCE2_Memcpy(ip_addr, ip_start, copy_len,
                                                  ip_addr, ip_addr + sizeof(ip_addr) - 1);

                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                                 "%s(%d) Failed to copy IP address.", __FILE__, __LINE__);
                        return DCE2_RET__ERROR;
                    }

                    /* No prefix - done with ip */
                    if (sfip_pton(ip_addr, ip) != SFIP_SUCCESS)
                    {
                        DCE2_ScError("Invalid IP address: \"%.*s\"", copy_len, ip_start);
                        return DCE2_RET__ERROR;
                    }

                    /* Don't allow a zero bit mask */
                    if ((sfaddr_family(&ip->addr) == AF_INET && ip->bits == 96) || ip->bits == 0)
                    {
                        DCE2_ScError("Invalid IP address with zero bit "
                                     "prefix: \"%.*s\"", copy_len, ip_start);
                        return DCE2_RET__ERROR;
                    }

                    return DCE2_RET__SUCCESS;
                }

                break;
        }

        (*ptr)++;
    }

    return DCE2_RET__ERROR;
}

/********************************************************************
 * Function: DCE2_ParsePortList()
 *
 * Parses a port list and adds bits associated with the ports
 * parsed to a bit array.
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the IP list.
 *  char *
 *      Pointer to the end of the string.
 *  uint8_t *
 *      Pointer to the port array mask to set bits for the ports
 *      parsed.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          port list.
 *      DCE2_RET__ERROR if an error occured in parsing the port list.
 *
 ********************************************************************/
DCE2_Ret DCE2_ParsePortList(char **ptr, char *end, uint8_t *port_array)
{
    char *lo_start = NULL;
    char *hi_start = NULL;
    DCE2_PortListState state = DCE2_PORT_LIST_STATE__START;
    uint16_t lo_port = 0, hi_port = 0;
    int one_port = 0;

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_PORT_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_PORT_LIST_STATE__START:
                if (DCE2_IsListStartChar(c))
                {
                    state = DCE2_PORT_LIST_STATE__PORT_START;
                }
                else if (DCE2_IsPortChar(c))
                {
                    one_port = 1;
                    lo_start = *ptr;
                    state = DCE2_PORT_LIST_STATE__PORT_LO;
                }
                else if (DCE2_IsPortRangeChar(c))
                {
                    one_port = 1;
                    lo_port = 0;
                    state = DCE2_PORT_LIST_STATE__PORT_RANGE;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid port list: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_PORT_LIST_STATE__PORT_START:
                lo_start = hi_start = NULL;

                if (DCE2_IsPortChar(c))
                {
                    lo_start = *ptr;
                    state = DCE2_PORT_LIST_STATE__PORT_LO;
                }
                else if (DCE2_IsPortRangeChar(c))
                {
                    lo_port = 0;
                    state = DCE2_PORT_LIST_STATE__PORT_RANGE;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid port list: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_PORT_LIST_STATE__PORT_LO:
                if (!DCE2_IsPortChar(c))
                {
                    DCE2_Ret status = DCE2_GetValue(lo_start, *ptr, &lo_port,
                                                    0, DCE2_INT_TYPE__UINT16, 10);

                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_ScError("Invalid port: \"%.*s\"", *ptr - lo_start, lo_start);
                        return DCE2_RET__ERROR;
                    }

                    if (DCE2_IsPortRangeChar(c))
                    {
                        state = DCE2_PORT_LIST_STATE__PORT_RANGE;
                    }
                    else
                    {
                        DCE2_SetPort(port_array, lo_port);

                        if (one_port)
                            return DCE2_RET__SUCCESS;

                        state = DCE2_PORT_LIST_STATE__PORT_END;
                        continue;
                    }
                }

                break;

            case DCE2_PORT_LIST_STATE__PORT_RANGE:
                if (DCE2_IsPortChar(c))
                {
                    hi_start = *ptr;
                    state = DCE2_PORT_LIST_STATE__PORT_HI;
                }
                else
                {
                    DCE2_SetPortRange(port_array, lo_port, UINT16_MAX);

                    if (one_port)
                        return DCE2_RET__SUCCESS;

                    state = DCE2_PORT_LIST_STATE__PORT_END;
                    continue;
                }

                break;

            case DCE2_PORT_LIST_STATE__PORT_HI:
                if (!DCE2_IsPortChar(c))
                {
                    DCE2_Ret status = DCE2_GetValue(hi_start, *ptr, &hi_port,
                                                    0, DCE2_INT_TYPE__UINT16, 10);

                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_ScError("Invalid port: \"%.*s\"", *ptr - hi_start, hi_start);
                        return DCE2_RET__ERROR;
                    }

                    DCE2_SetPortRange(port_array, lo_port, hi_port);

                    if (one_port)
                        return DCE2_RET__SUCCESS;

                    state = DCE2_PORT_LIST_STATE__PORT_END;
                    continue;
                }

                break;

            case DCE2_PORT_LIST_STATE__PORT_END:
                if (DCE2_IsListEndChar(c))
                {
                    state = DCE2_PORT_LIST_STATE__END;
                }
                else if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_PORT_LIST_STATE__PORT_START;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_ScError("Invalid port list: \"%s\"", *ptr);
                    return DCE2_RET__ERROR;
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Invalid port list state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        (*ptr)++;
    }

    if (state != DCE2_PORT_LIST_STATE__END)
    {
        DCE2_ScError("Invalid port list: \"%s\"", *ptr);
        return DCE2_RET__ERROR;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ParseValue()
 *
 * Parses what should be an integer value and stores in memory
 * passed in as an argument.  This function will parse positive
 * and negative values and decimal, octal or hexidecimal.  The
 * positive and negative modifiers can only be used with
 * decimal values.
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the IP list.
 *  char *
 *      Pointer to the end of the string.
 *  void *
 *      Pointer to the place where the value should be stored if
 *      parsed successfully.
 *  DCE2_IntType
 *      The type of integer that the value pointer points to and
 *      that should be parsed.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          integer value.
 *      DCE2_RET__ERROR if an error occured in parsing the integer
 *          value.
 *
 ********************************************************************/
DCE2_Ret DCE2_ParseValue(char **ptr, char *end, void *value, DCE2_IntType int_type)
{
    char *value_start = *ptr;
    DCE2_ValueState state = DCE2_VALUE_STATE__START;
    int negate = 0;

    while (*ptr < end)
    {
        char c = **ptr;

        switch (state)
        {
            case DCE2_VALUE_STATE__START:
                if (c == DCE2_CFG_TOK__HEX_OCT_START)
                {
                    /* Just in case it's just a 0 */
                    value_start = *ptr;
                    state = DCE2_VALUE_STATE__HEX_OR_OCT;
                }
                else if (isdigit((int)c))
                {
                    value_start = *ptr;
                    state = DCE2_VALUE_STATE__DECIMAL;
                }
                else if (c == DCE2_CFG_TOK__MINUS)
                {
                    if ((int_type == DCE2_INT_TYPE__UINT8) ||
                        (int_type == DCE2_INT_TYPE__UINT16) ||
                        (int_type == DCE2_INT_TYPE__UINT32) ||
                        (int_type == DCE2_INT_TYPE__UINT64))
                    {
                        return DCE2_RET__ERROR;
                    }

                    negate = 1;
                    state = DCE2_VALUE_STATE__MODIFIER;
                }
                else if (c == DCE2_CFG_TOK__PLUS)
                {
                    negate = 0;
                    state = DCE2_VALUE_STATE__MODIFIER;
                }
                else if (!isspace((int)c))  /* Allow for leading space */
                {
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_VALUE_STATE__MODIFIER:
                if (isdigit((int)c))
                {
                    value_start = *ptr;
                    state = DCE2_VALUE_STATE__DECIMAL;
                }
                else
                {
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_VALUE_STATE__HEX_OR_OCT:
                if (tolower((int)c) == tolower((int)DCE2_CFG_TOK__HEX_SEP))
                {
                    state = DCE2_VALUE_STATE__HEX_START;
                }
                else if (isdigit((int)c))
                {
                    value_start = *ptr;
                    state = DCE2_VALUE_STATE__OCTAL;
                }
                else
                {
                    /* It's just a zero */
                    return DCE2_GetValue(value_start, *ptr, value, negate, int_type, 10);
                }

                break;

            case DCE2_VALUE_STATE__DECIMAL:
                if (!isdigit((int)c))
                {
                    return DCE2_GetValue(value_start, *ptr, value, negate, int_type, 10);
                }

                break;

            case DCE2_VALUE_STATE__HEX_START:
                if (isxdigit((int)c))
                {
                    value_start = *ptr;
                    state = DCE2_VALUE_STATE__HEX;
                }
                else
                {
                    return DCE2_RET__ERROR;
                }

                break;

            case DCE2_VALUE_STATE__HEX:
                if (!isxdigit((int)c))
                {
                    return DCE2_GetValue(value_start, *ptr, value, negate, int_type, 16);
                }

                break;

            case DCE2_VALUE_STATE__OCTAL:
                if (!isdigit((int)c))
                {
                    return DCE2_GetValue(value_start, *ptr, value, negate, int_type, 8);
                }

                break;

            default:
                DCE2_Log(DCE2_LOG_TYPE__ERROR,
                         "%s(%d) Invalid value state: %d",
                         __FILE__, __LINE__, state);
                return DCE2_RET__ERROR;
        }

        (*ptr)++;
    }

    // In case we hit the end before getting a non-type character.
    switch (state)
    {
        case DCE2_VALUE_STATE__HEX_OR_OCT:
            return DCE2_GetValue(value_start, end, value, negate, int_type, 8);
        case DCE2_VALUE_STATE__DECIMAL:
            return DCE2_GetValue(value_start, end, value, negate, int_type, 10);
        case DCE2_VALUE_STATE__HEX:
            return DCE2_GetValue(value_start, end, value, negate, int_type, 16);
        case DCE2_VALUE_STATE__OCTAL:
            return DCE2_GetValue(value_start, end, value, negate, int_type, 8);
        default:
            break;
    }

    /* If we break out of the loop before finishing, didn't
     * get a valid value */
    return DCE2_RET__ERROR;
}

/********************************************************************
 * Function: DCE2_GetValue()
 *
 * Parses integer values up to 64 bit unsigned.  Stores the value
 * parsed in memory passed in as an argument.
 *
 * Arguments:
 *  char *
 *      Pointer to the first character in the string to parse.
 *  char *
 *      Pointer to the byte after the last character of
 *      the string to parse.
 *  void *
 *      Pointer to the memory where the parsed integer should
 *      be stored on successful parsing.
 *  int
 *      Non-zero if the parsed value should be negated.
 *      Zero if the parsed value should not be negated.
 *  DCE2_IntType
 *      The type of integer we want to parse and the integer type
 *      that the pointer that the parsed value will be put in is.
 *  uint8_t
 *      The base that the parsed value should be converted to.
 *      Only 8, 10 and 16 are supported.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if we were able to successfully parse the
 *          integer to the type specified.
 *      DCE2_RET__ERROR if an error occured in parsing.
 *
 ********************************************************************/
DCE2_Ret DCE2_GetValue(char *start, char *end, void *int_value, int negate,
                       DCE2_IntType int_type, uint8_t base)
{
    uint64_t value = 0;
    uint64_t place = 1;
    uint64_t max_value = 0;

    if ((end == NULL) || (start == NULL) || (int_value == NULL))
        return DCE2_RET__ERROR;

    if (start >= end)
        return DCE2_RET__ERROR;

    for (end = end - 1; end >= start; end--)
    {
        uint64_t add_value;
        char c = *end;

        if ((base == 16) && !isxdigit((int)c))
            return DCE2_RET__ERROR;
        else if ((base != 16) && !isdigit((int)c))
            return DCE2_RET__ERROR;

        if (isdigit((int)c))
            add_value = (uint64_t)(c - '0') * place;
        else
            add_value = (uint64_t)((toupper((int)c) - 'A') + 10) * place;

        if ((UINT64_MAX - value) < add_value)
            return DCE2_RET__ERROR;

        value += add_value;
        place *= base;
    }

    switch (int_type)
    {
        case DCE2_INT_TYPE__INT8:
            max_value = ((UINT8_MAX - 1) / 2);
            if (negate) max_value++;
            break;
        case DCE2_INT_TYPE__UINT8:
            max_value = UINT8_MAX;
            break;
        case DCE2_INT_TYPE__INT16:
            max_value = ((UINT16_MAX - 1) / 2);
            if (negate) max_value++;
            break;
        case DCE2_INT_TYPE__UINT16:
            max_value = UINT16_MAX;
            break;
        case DCE2_INT_TYPE__INT32:
            max_value = ((UINT32_MAX - 1) / 2);
            if (negate) max_value++;
            break;
        case DCE2_INT_TYPE__UINT32:
            max_value = UINT32_MAX;
            break;
        case DCE2_INT_TYPE__INT64:
            max_value = ((UINT64_MAX - 1) / 2);
            if (negate) max_value++;
            break;
        case DCE2_INT_TYPE__UINT64:
            max_value = UINT64_MAX;
            break;
    }

    if (value > max_value)
        return DCE2_RET__ERROR;

    if (negate)
        value *= -1;

    switch (int_type)
    {
        case DCE2_INT_TYPE__INT8:
            *(int8_t *)int_value = (int8_t)value;
            break;
        case DCE2_INT_TYPE__UINT8:
            *(uint8_t *)int_value = (uint8_t)value;
            break;
        case DCE2_INT_TYPE__INT16:
            *(int16_t *)int_value = (int16_t)value;
            break;
        case DCE2_INT_TYPE__UINT16:
            *(uint16_t *)int_value = (uint16_t)value;
            break;
        case DCE2_INT_TYPE__INT32:
            *(int32_t *)int_value = (int32_t)value;
            break;
        case DCE2_INT_TYPE__UINT32:
            *(uint32_t *)int_value = (uint32_t)value;
            break;
        case DCE2_INT_TYPE__INT64:
            *(int64_t *)int_value = (int64_t)value;
            break;
        case DCE2_INT_TYPE__UINT64:
            *(uint64_t *)int_value = (uint64_t)value;
            break;
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_GcError()
 *
 * Formats errors related to global configuration and puts in
 * global error buffer.
 *
 * Arguments:
 *  const char *
 *      The format string
 *  ...
 *      The arguments to format string
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_GcError(const char *format, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    buf[sizeof(buf) - 1] = '\0';

    snprintf(dce2_config_error, sizeof(dce2_config_error),
             "%s(%d): \"%s\" configuration: %s.  Please consult documentation.",
             *_dpd.config_file, *_dpd.config_line, DCE2_GNAME, buf);

    dce2_config_error[sizeof(dce2_config_error) - 1] = '\0';
}

/********************************************************************
 * Function: DCE2_ScError()
 *
 * Formats errors related to server configuration and puts in
 * global error buffer.
 *
 * Arguments:
 *  const char *
 *      The format string
 *  ...
 *      The arguments to format string
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ScError(const char *format, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    buf[sizeof(buf) - 1] = '\0';

    snprintf(dce2_config_error, sizeof(dce2_config_error),
             "%s(%d): \"%s\" configuration: %s.  Please consult documentation.",
             *_dpd.config_file, *_dpd.config_line, DCE2_SNAME, buf);

    dce2_config_error[sizeof(dce2_config_error) - 1] = '\0';
}

/********************************************************************
 * Function: DCE2_FreeConfig
 *
 * Frees a dcerpc configuration
 *
 * Arguments:
 *  DCE2_Config *
 *      The configuration to free.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_FreeConfig(DCE2_Config *config)
{
    if (config == NULL)
        return;

    if (config->gconfig != NULL)
        DCE2_Free((void *)config->gconfig, sizeof(DCE2_GlobalConfig), DCE2_MEM_TYPE__CONFIG);

    if (config->dconfig != NULL)
    {
        if (config->dconfig->smb_invalid_shares != NULL)
            DCE2_ListDestroy(config->dconfig->smb_invalid_shares);

        DCE2_Free((void *)config->dconfig, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
    }

    /* Free routing tables and server configurations */
    if (config->sconfigs != NULL)
    {
        /* UnRegister routing table memory */
        DCE2_UnRegMem(sfrt_usage(config->sconfigs), DCE2_MEM_TYPE__RT);

        sfrt_cleanup(config->sconfigs, DCE2_ServerConfigCleanup);
        sfrt_free(config->sconfigs);
    }

    DCE2_Free((void *)config, sizeof(DCE2_Config), DCE2_MEM_TYPE__CONFIG);
}

/********************************************************************
 * Function: DCE2_FreeConfigs
 *
 * Frees a dcerpc configuration
 *
 * Arguments:
 *  DCE2_Config *
 *      The configuration to free.
 *
 * Returns: None
 *
 ********************************************************************/
static int DCE2_FreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    DCE2_Config *pPolicyConfig = (DCE2_Config *)pData;

    //do any housekeeping before freeing DCE22_Config

    sfPolicyUserDataClear (config, policyId);
    DCE2_FreeConfig(pPolicyConfig);

    return 0;
}

void DCE2_FreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, DCE2_FreeConfigsPolicy);
    sfPolicyConfigDelete(config);
}


/******************************************************************
 * Function: DCE2_ServerConfigCleanup()
 *
 * Free server configurations in routing table.  Each server
 * configuration keeps a reference count of the number of pointers
 * in the routing table that are pointed to it.  The server
 * configuration is only freed when this count reaches zero.
 *
 * Arguments:
 *  void *
 *      Pointer to server configuration.
 *
 * Returns: None
 *
 ******************************************************************/
static void DCE2_ServerConfigCleanup(void *data)
{
    DCE2_ServerConfig *sc = (DCE2_ServerConfig *)data;

    if (sc != NULL)
    {
        sc->ref_count--;
        if (sc->ref_count == 0)
        {
            DCE2_ListDestroy(sc->smb_invalid_shares);
            DCE2_Free((void *)sc, sizeof(DCE2_ServerConfig), DCE2_MEM_TYPE__CONFIG);
        }
    }
}

