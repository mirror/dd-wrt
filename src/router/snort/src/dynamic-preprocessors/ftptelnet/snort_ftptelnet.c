/*
 * snort_ftptelnet.c
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
 * Kevin Liu <kliu@sourcefire.com>
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
 * Description:
 *
 * This file wraps the FTPTelnet functionality for Snort
 * and starts the Normalization & Protocol checks.
 *
 * The file takes a Packet structure from the Snort IDS to start the
 * FTP/Telnet Normalization & Protocol checks.  It also uses the Stream
 * Interface Module which is also Snort-centric.  Mainly, just a wrapper
 * to FTP/Telnet functionality, but a key part to starting the basic flow.
 *
 * The main bulk of this file is taken up with user configuration and
 * parsing.  The reason this is so large is because FTPTelnet takes
 * very detailed configuration parameters for each specified FTP client,
 * to provide detailed control over an internal network and robust control
 * of the external network.
 *
 * The main functions of note are:
 *   - FTPTelnetSnortConf()    the configuration portion
 *   - SnortFTPTelnet()        the actual normalization & inspection
 *   - LogEvents()             where we log the FTPTelnet events
 *
 * NOTES:
 * - 16.09.04:  Initial Development.  SAS
 *
 */

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "sf_ip.h"

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#endif

#define BUF_SIZE 1024

#include "sf_types.h"
#include "snort_debug.h"
#include "ftpp_return_codes.h"
#include "ftpp_ui_config.h"
#include "ftpp_ui_client_lookup.h"
#include "ftpp_ui_server_lookup.h"
#include "ftp_cmd_lookup.h"
#include "ftp_bounce_lookup.h"
#include "ftpp_si.h"
#include "ftpp_eo_log.h"
#include "pp_telnet.h"
#include "pp_ftp.h"
#include "snort_ftptelnet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "stream_api.h"
#include "profiler.h"
#include "sf_snort_plugin_api.h"
#include "Unified2_common.h"
#include "ssl_include.h"
#include <daq_common.h>
#include "memory_stats.h"

#ifdef DUMP_BUFFER
#include "ftptelnet_buffer_dump.h"
#endif

#ifdef PERF_PROFILING
extern PreprocStats ftpPerfStats;
extern PreprocStats telnetPerfStats;
PreprocStats ftppDetectPerfStats;
int ftppDetectCalled = 0;
#endif

#ifdef TARGET_BASED
unsigned s_ftpdata_eof_cb_id = 0;
unsigned s_ftpdata_flush_cb_id = 0;
#endif

extern tSfPolicyUserContextId ftp_telnet_config;

/*
 * GLOBAL subkeyword values
 */
#define ENCRYPTED_TRAFFIC "encrypted_traffic"
#define CHECK_ENCRYPTED   "check_encrypted"
#define INSPECT_TYPE      "inspection_type"
#define INSPECT_TYPE_STATELESS "stateless"
#define INSPECT_TYPE_STATEFUL  "stateful"
/*
 * Protocol subkeywords.
 */
#define PORTS             "ports"

/*
 * Telnet subkeywords.
 */
#define AYT_THRESHOLD     "ayt_attack_thresh"
#define NORMALIZE         "normalize"
#define DETECT_ANOMALIES  "detect_anomalies"

/*
 * FTP SERVER subkeywords.
 */
#define FTP_CMDS          "ftp_cmds"
#define PRINT_CMDS        "print_cmds"
#define MAX_PARAM_LEN     "def_max_param_len"
#define ALT_PARAM_LEN     "alt_max_param_len"
#define CMD_VALIDITY      "cmd_validity"
#define STRING_FORMAT     "chk_str_fmt"
#define TELNET_CMDS       "telnet_cmds"
#define IGNORE_TELNET_CMDS "ignore_telnet_erase_cmds"
#define DATA_CHAN_CMD     "data_chan_cmds"
#define DATA_XFER_CMD     "data_xfer_cmds"
#define DATA_REST_CMD     "data_rest_cmds"
#define FILE_PUT_CMD      "file_put_cmds"
#define FILE_GET_CMD      "file_get_cmds"
#define DATA_CHAN         "data_chan"
#define LOGIN_CMD         "login_cmds"
#define ENCR_CMD          "encr_cmds"
#define DIR_CMD           "dir_cmds"
#define IGNORE_DATA_CHAN  "ignore_data_chan"

/*
 * FTP CLIENT subkeywords
 */
#define BOUNCE            "bounce"
#define ALLOW_BOUNCE      "bounce_to"
#define MAX_RESP_LEN      "max_resp_len"

/*
 * Data type keywords
 */
#define START_CMD_FORMAT    "<"
#define END_CMD_FORMAT      ">"
#define F_INT               "int"
#define F_NUMBER            "number"
#define F_CHAR              "char"
#define F_DATE              "date"
#define F_LITERAL           "'"
#define F_STRING            "string"
#define F_STRING_FMT        "formated_string"
#define F_HOST_PORT         "host_port"
#define F_LONG_HOST_PORT    "long_host_port"
#define F_EXTD_HOST_PORT    "extd_host_port"

/*
 * Optional parameter delimiters
 */
#define START_OPT_FMT       "["
#define END_OPT_FMT         "]"
#define START_CHOICE_FMT    "{"
#define END_CHOICE_FMT      "}"
#define OR_FMT              "|"


/*
 * The cmd_validity keyword can be used with the format keyword to
 * restrict data types.  The interpretation is specific to the data
 * type.  'format' is only supported with date & char data types.
 *
 * A few examples:
 *
 * 1. Will perform validity checking of an FTP Mode command to
 * check for one of the characters A, S, B, or C.
 *
 * cmd_validity MODE char ASBC
 *
 *
 * 2. Will perform validity checking of an FTP MDTM command to
 * check for an optional date argument following the format
 * specified.  The date would uses the YYYYMMDDHHmmss+TZ format.
 *
 * cmd_validity MDTM [ date nnnnnnnnnnnnnn[.n[n[n]]] ] string
 *
 *
 * 3. Will perform validity checking of an FTP ALLO command to
 * check for an integer, then optionally, the letter R and another
 * integer.
 *
 * cmd_validity ALLO int [ char R int ]
 */

/*
 * The def_max_param_len & alt_max_param_len keywords can be used to
 * restrict parameter length for one or more commands.  The space
 * separated list of commands is enclosed in {}s.
 *
 * A few examples:
 *
 * 1. Restricts all command parameters to 100 characters
 *
 * def_max_param_len 100
 *
 * 2. Overrides CWD pathname to 256 characters
 *
 * alt_max_param_len 256 { CWD }
 *
 * 3. Overrides PWD & SYST to no parameters
 *
 * alt_max_param_len 0 { PWD SYST }
 *
 */

/*
 * Alert subkeywords
 */
#define BOOL_YES     "yes"
#define BOOL_NO      "no"

/*
 **  IP Address list delimiters
 */
#define START_IPADDR_LIST "{"
#define END_IPADDR_LIST   "}"

/*
 * Port list delimiters
 */
#define START_PORT_LIST "{"
#define END_PORT_LIST   "}"

/*
 * Keyword for the default client/server configuration
 */
#define DEFAULT "default"

/*
 * The default FTP server configuration for FTP command validation.
 * Most of this comes from RFC 959, with additional commands being
 * drawn from other RFCs/Internet Drafts that are in use.
 *
 * Any of the below can be overridden in snort.conf.
 *
 * This is here to eliminate most of it from snort.conf to
 * avoid an ugly configuration file.  The default_max_param_len
 * is somewhat arbitrary, but is taken from the majority of
 * the snort FTP rules that limit parameter size to 100
 * characters, as of 18 Sep 2004.
 *
 * The data_chan_cmds, data_xfer_cmds are used to track open
 * data channel connections.
 *
 * The login_cmds and dir_cmds are used to track state of username
 * and current directory.
 *
 * The file_put_cmds and file_get_cmds are used to track file transfers
 * over open data channel connections.
 */
/* DEFAULT_FTP_CONF_* deliberately break the default conf into
 * chunks with lengths < 509 to keep ISO C89 compilers happy
 */
static const char* DEFAULT_FTP_CONF[] = {
    "hardcoded_config "
        "def_max_param_len 100 "

        "ftp_cmds { USER PASS ACCT CWD CDUP SMNT QUIT REIN TYPE STRU"
        " MODE RETR STOR STOU APPE ALLO REST RNFR RNTO ABOR"
        " DELE RMD MKD PWD LIST NLST SITE SYST STAT HELP NOOP } "
        "ftp_cmds { AUTH ADAT PROT PBSZ CONF ENC } "
        "ftp_cmds { PORT PASV LPRT LPSV EPRT EPSV } "
        "ftp_cmds { FEAT OPTS } "
        "ftp_cmds { MDTM REST SIZE MLST MLSD } "

        "alt_max_param_len 0 { CDUP QUIT REIN PASV STOU ABOR PWD SYST NOOP } ",

    "cmd_validity MODE < char SBC > "
        "cmd_validity STRU < char FRPO [ string ] > "
        "cmd_validity ALLO < int [ char R int ] > "
        "cmd_validity TYPE < { char AE [ char NTC ] | char I | char L [ number ] } > "
        "cmd_validity PORT < host_port > "
        "cmd_validity LPRT < long_host_port > "
        "cmd_validity EPRT < extd_host_port > "
        "cmd_validity EPSV < [ { '1' | '2' | 'ALL' } ] > ",

    "data_chan_cmds { PORT PASV LPRT LPSV EPRT EPSV } "
        "data_xfer_cmds { RETR STOR STOU APPE LIST NLST } "
        "data_rest_cmds { REST } "
        "file_put_cmds { STOR STOU } "
        "file_get_cmds { RETR } "
        "login_cmds { USER PASS } "
        "dir_cmds { CWD 250 CDUP 250 PWD 257 } "
        "encr_cmds { AUTH } "
};

#define CONF_CHUNKS (sizeof(DEFAULT_FTP_CONF)/sizeof(DEFAULT_FTP_CONF[0]))

static uint8_t ftp_paf_id = 0;

static char* DefaultConf (size_t* pn) {
    unsigned i;
    size_t sz = 1, ns = 0;
    char* str = NULL;

    for ( i = 0; i < CONF_CHUNKS; i++ )
        sz += strlen(DEFAULT_FTP_CONF[i]);

    str = _dpd.snortAlloc(1, sz, PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);

    if ( !str )
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                *(_dpd.config_file), *(_dpd.config_line));

    for ( i = 0; i < CONF_CHUNKS; i++ )
        ns += snprintf(str+ns, sz-ns, "%s", DEFAULT_FTP_CONF[i]);

    *pn = sz;
    return str;
}

static int printedFTPHeader = 0;
static int _checkServerConfig(struct _SnortConfig *, void *pData);

char *maxToken = NULL;
static tSfPolicyId ftp_current_policy = 0;

static void _addPortsToStream(struct _SnortConfig *, char *, tSfPolicyId, int);
static int _addFtpServerConfPortsToStream(struct _SnortConfig *, void *);

static void _FTPTelnetAddPortsOfInterest(struct _SnortConfig *, FTPTELNET_GLOBAL_CONF *, tSfPolicyId);
#ifdef TARGET_BASED
static void _FTPTelnetAddService(struct _SnortConfig *, int16_t, tSfPolicyId);
#endif
void FTP_Set_flow_id( void *app_data, uint32_t fid );
void FTPData_Set_flow_id( void *app_data, uint32_t fid );

char* mystrtok (char* s, const char* delim)
{
    static char* last = NULL;
    if ( s || last )
        last = strtok(s, delim);
    return last;
}

char *NextToken(char *delimiters)
{
    char *retTok = mystrtok(NULL, delimiters);
    if (retTok > maxToken)
        return NULL;

    return retTok;
}

/*
 * Function: ProcessConfOpt(FTPTELNET_CONF_OPT *ConfOpt,
 *                          char *Option,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: Set the CONF_OPT on and alert fields.
 *
 *          We check to make sure of valid parameters and then set
 *          the appropriate fields.
 *
 * Arguments: ConfOpt       => pointer to the configuration option
 *            Option        => character pointer to the option being configured
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessConfOpt(FTPTELNET_CONF_OPT *ConfOpt, char *Option,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;

    pcToken = NextToken(CONF_SEPARATORS);
    if(pcToken == NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "No argument to token '%s'.", Option);

        return FTPP_FATAL_ERR;
    }

    /*
     * Check for the alert value
     */
    if(!strcmp(BOOL_YES, pcToken))
    {
        ConfOpt->alert = 1;
    }
    else if(!strcmp(BOOL_NO, pcToken))
    {
        ConfOpt->alert = 0;
    }
    else
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid argument to token '%s'.", Option);

        return FTPP_FATAL_ERR;
    }

    ConfOpt->on = 1;

    return FTPP_SUCCESS;
}

/*
 * Function: PrintConfOpt(FTPTELNET_CONF_OPT *ConfOpt,
 *                          char *Option)
 *
 * Purpose: Prints the CONF_OPT and alert fields.
 *
 * Arguments: ConfOpt       => pointer to the configuration option
 *            Option        => character pointer to the option being configured
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int PrintConfOpt(FTPTELNET_CONF_OPT *ConfOpt, char *Option)
{
    if(!ConfOpt || !Option)
    {
        return FTPP_INVALID_ARG;
    }

    if(ConfOpt->on)
    {
        _dpd.logMsg("      %s: YES alert: %s\n", Option,
                ConfOpt->alert ? "YES" : "NO");
    }
    else
    {
        _dpd.logMsg("      %s: OFF\n", Option);
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessInspectType(FTPTELNET_CONF_OPT *ConfOpt,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the type of inspection.
 *          This sets the type of inspection for FTPTelnet to do.
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessInspectType(FTPTELNET_GLOBAL_CONF *GlobalConf,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;

    pcToken = NextToken(CONF_SEPARATORS);
    if(pcToken == NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "No argument to token '%s'.", INSPECT_TYPE);

        return FTPP_FATAL_ERR;
    }

    if(!strcmp(INSPECT_TYPE_STATEFUL, pcToken))
    {
        GlobalConf->inspection_type = FTPP_UI_CONFIG_STATEFUL;
    }
    else if(!strcmp(INSPECT_TYPE_STATELESS, pcToken))
    {
        GlobalConf->inspection_type = FTPP_UI_CONFIG_STATELESS;
    }
    else
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid argument to token '%s'.  Must be either "
                "'%s' or '%s'.", INSPECT_TYPE, INSPECT_TYPE_STATEFUL,
                INSPECT_TYPE_STATELESS);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessFTPGlobalConf(FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: This is where we process the global configuration for FTPTelnet.
 *
 *          We set the values of the global configuraiton here.  Any errors
 *          that are encountered are specified in the error string and the
 *          type of error is returned through the return code, i.e. fatal,
 *          non-fatal.
 *
 *          The configuration options that are dealt with here are:
 *          - inspection_type
 *              Indicate whether to operate in stateful stateless mode
 *          - encrypted_traffic
 *              Detect and alert on encrypted sessions
 *          - check_after_encrypted
 *              Instructs the preprocessor to continue checking a data stream
 *              after it is encrypted, looking for an eventual
 *              non-ecrypted data.
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int ProcessFTPGlobalConf(FTPTELNET_GLOBAL_CONF *GlobalConf,
        char *ErrorString, int ErrStrLen)
{
    FTPTELNET_CONF_OPT *ConfOpt;
    int  iRet = 0;
    char *pcToken;
    int  iTokens = 0;

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        /*
         * Show that we at least got one token
         */
        iTokens = 1;

        /*
         * Search for configuration keywords
         */
        if (!strcmp(pcToken, CHECK_ENCRYPTED))
        {
            GlobalConf->check_encrypted_data = 1;
        }
        else if (!strcmp(pcToken, ENCRYPTED_TRAFFIC))
        {
            ConfOpt = &GlobalConf->encrypted;
            iRet = ProcessConfOpt(ConfOpt, ENCRYPTED_TRAFFIC, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(INSPECT_TYPE, pcToken))
        {
            iRet = ProcessInspectType(GlobalConf, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid keyword '%s' for '%s' configuration.",
                    pcToken, GLOBAL);

            return FTPP_FATAL_ERR;
        }
    }

    /*
     * If there are not any tokens to the configuration, then
     * we let the user know and log the error.  return non-fatal
     * error.
     */
    if(!iTokens)
    {
        snprintf(ErrorString, ErrStrLen,
                "No tokens to '%s' configuration.", GLOBAL);

        return FTPP_NONFATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessPorts(PROTO_CONF *protocol,
 *                        char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the port list for the server configuration.
 *          This configuration is a list of valid ports and is ended
 *          by a delimiter.
 *
 * Arguments: protocol      => pointer to the ports configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessPorts(PROTO_CONF *protocol,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    char *pcEnd;
    int  iPort;
    int  iEndPorts = 0;

    pcToken = NextToken(CONF_SEPARATORS);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid port list format.");

        return FTPP_FATAL_ERR;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a port list with the '%s' token.",
                START_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    /* Unset the defaults */
    for (iPort = 0;iPort<MAXPORTS;iPort++)
        protocol->ports[iPort] = 0;

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndPorts = 1;
            break;
        }

        iPort = strtol(pcToken, &pcEnd, 10);

        /*
         * Validity check for port
         */
        if(*pcEnd)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid port number.");

            return FTPP_FATAL_ERR;
        }

        if(iPort < 0 || iPort > MAXPORTS-1)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid port number.  Must be between 0 and "
                    "65535.");

            return FTPP_FATAL_ERR;
        }

        protocol->ports[iPort] = 1;

        if(protocol->port_count < MAXPORTS)
            protocol->port_count++;
    }

    if(!iEndPorts)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end '%s' configuration with '%s'.",
                PORTS, END_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessTelnetAYTThreshold(TELNET_PROTO_CONF *TelnetConf,
 *                        char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the 'are you there' threshold configuration
 *          This sets the maximum number of telnet ayt commands that
 *          we will tolerate, before alerting.
 *
 * Arguments: TelnetConf    => pointer to the telnet configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessTelnetAYTThreshold(TELNET_PROTO_CONF *TelnetConf,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    char *pcEnd = NULL;

    pcToken = NextToken(CONF_SEPARATORS);
    if(pcToken == NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "No argument to token '%s'.", AYT_THRESHOLD);

        return FTPP_FATAL_ERR;
    }

    TelnetConf->ayt_threshold = strtol(pcToken, &pcEnd, 10);

    /*
     * Let's check to see if the entire string was valid.
     * If there is an address here, then there was an
     * invalid character in the string.
     */
    if(*pcEnd)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid argument to token '%s'.  Must be a positive "
                "number.", AYT_THRESHOLD);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: PrintTelnetConf(TELNET_PROTO_CONF *TelnetConf,
 *                          char *Option)
 *
 * Purpose: Prints the telnet configuration
 *
 * Arguments: TelnetConf    => pointer to the telnet configuration
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int PrintTelnetConf(TELNET_PROTO_CONF *TelnetConf)
{
    char buf[BUF_SIZE+1];
    int iCtr;

    if(!TelnetConf)
    {
        return FTPP_INVALID_ARG;
    }

    _dpd.logMsg("    TELNET CONFIG:\n");
    memset(buf, 0, BUF_SIZE+1);
    snprintf(buf, BUF_SIZE, "      Ports: ");

    /*
     * Print out all the applicable ports.
     */
    for(iCtr = 0; iCtr < MAXPORTS; iCtr++)
    {
        if(TelnetConf->proto_ports.ports[iCtr])
        {
            _dpd.printfappend(buf, BUF_SIZE, "%d ", iCtr);
        }
    }

    _dpd.logMsg("%s\n", buf);

    _dpd.logMsg("      Are You There Threshold: %d\n",
            TelnetConf->ayt_threshold);
    _dpd.logMsg("      Normalize: %s\n", TelnetConf->normalize ? "YES" : "NO");
    _dpd.logMsg("      Detect Anomalies: %s\n",
            TelnetConf->detect_anomalies ? "YES" : "NO");

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessTelnetConf(FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: This is where we process the telnet configuration for FTPTelnet.
 *
 *          We set the values of the telnet configuraiton here.  Any errors
 *          that are encountered are specified in the error string and the
 *          type of error is returned through the return code, i.e. fatal,
 *          non-fatal.
 *
 *          The configuration options that are dealt with here are:
 *          - ports { x }           Ports on which to do telnet checks
 *          - normalize             Turns on normalization
 *          - ayt_attack_thresh x   Detect consecutive are you there commands
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int ProcessTelnetConf(FTPTELNET_GLOBAL_CONF *GlobalConf,
        char *ErrorString, int ErrStrLen)
{
    int  iRet;
    char *pcToken;
    int  iTokens = 0;

    if (GlobalConf->telnet_config != NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "Telnet can only be configured once.\n");

        return FTPP_FATAL_ERR;
    }

    GlobalConf->telnet_config =
        (TELNET_PROTO_CONF *)_dpd.snortAlloc(1, sizeof(TELNET_PROTO_CONF),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    if (GlobalConf->telnet_config == NULL)
    {
        DynamicPreprocessorFatalMessage("Out of memory trying to create "
                "telnet configuration.\n");
    }

    /*
     * Reset the global telnet configuration
     */
    if(ftpp_ui_config_reset_telnet_proto(GlobalConf->telnet_config))
    {
        snprintf(ErrorString, ErrStrLen,
                "Cannot reset the FTPTelnet global telnet configuration.");

        return FTPP_FATAL_ERR;
    }

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        /*
         * Show that we at least got one token
         */
        iTokens = 1;

        /*
         * Search for configuration keywords
         */
        if(!strcmp(PORTS, pcToken))
        {
            PROTO_CONF *ports = (PROTO_CONF*)GlobalConf->telnet_config;
            iRet = ProcessPorts(ports, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(AYT_THRESHOLD, pcToken))
        {
            iRet = ProcessTelnetAYTThreshold(GlobalConf->telnet_config,
                    ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(NORMALIZE, pcToken))
        {
            GlobalConf->telnet_config->normalize = 1;
        }
        else if(!strcmp(DETECT_ANOMALIES, pcToken))
        {
            GlobalConf->telnet_config->detect_anomalies = 1;
        }
        /*
         * Start the CONF_OPT configurations.
         */
        else
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid keyword '%s' for '%s' configuration.",
                    pcToken, GLOBAL);

            return FTPP_FATAL_ERR;
        }
    }

    /*
     * If there are not any tokens to the configuration, then
     * we let the user know and log the error.  return non-fatal
     * error.
     */
    if(!iTokens)
    {
        snprintf(ErrorString, ErrStrLen,
                "No tokens to '%s' configuration.", TELNET);
        return FTPP_NONFATAL_ERR;
    }

    /* Let's print out the telnet config */
    PrintTelnetConf(GlobalConf->telnet_config);

    return FTPP_SUCCESS;
}

#if 0
/**obsoleted during changes for bug_31418
*/
/*
 * Function: GetIPAddr(char *addrString, unsigned uint32_t *ipAddr,
 *                     char *ErrorString, int ErrStrLen)
 *
 * Purpose: This is where we convert an IP address to a numeric
 *
 *          Any errors that are encountered are specified in the error
 *          string and the type of error is returned through the return
 *          code, i.e. fatal, non-fatal.
 *
 * Arguments: addrString    => pointer to the address string
 *            ipAddr        => pointer to converted address
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int GetIPAddr(char *addrString, sfaddr_t *ipAddr,
        char *ErrorString, int ErrStrLen)
{
    if(sfip_pton(addrString, ipAddr) != SFIP_SUCCESS)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid FTP client IP address '%s'.", addrString);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}
#endif
/*
 * Function: ProcessFTPCmdList(FTP_SERVER_PROTO_CONF *ServerConf,
 *                             char *confOption,
 *                             char *ErrorString, int ErrStrLen,
 *                             int require_cmds, int require_length)
 *
 * Purpose: Process the FTP cmd lists for the client configuration.
 *          This configuration is a parameter length for the list of
 *          FTP commands and is ended by a delimiter.
 *
 * Arguments: ServerConf    => pointer to the FTP server configuration
 *            confOption    => pointer to the name of the option
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *            require_cmds  => flag to require a command list
 *            require_length => flag to require a length specifier
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessFTPCmdList(FTP_SERVER_PROTO_CONF *ServerConf,
        char *confOption,
        char *ErrorString, int ErrStrLen,
        int require_cmds, int require_length)
{
    FTP_CMD_CONF *FTPCmd = NULL;
    char *pcToken;
    char *pcEnd = NULL;
    char *cmd;
    int  iLength = 0;
    int  iEndCmds = 0;
    int  iRet;

    if (require_length)
    {
        pcToken = NextToken(CONF_SEPARATORS);
        if(!pcToken)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid cmd list format.");

            return FTPP_FATAL_ERR;
        }

        iLength = strtol(pcToken, &pcEnd, 10);

        /*
         * Let's check to see if the entire string was valid.
         * If there is an address here, then there was an
         * invalid character in the string.
         */
        if((*pcEnd) || (iLength < 0))
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid argument to token '%s'.  "
                    "Length must be a positive number",
                    confOption);

            return FTPP_FATAL_ERR;
        }
    }

    if (require_cmds)
    {
        pcToken = NextToken(CONF_SEPARATORS);
        if(!pcToken)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid cmd list format.");

            return FTPP_FATAL_ERR;
        }

        if(strcmp(START_PORT_LIST, pcToken))
        {
            snprintf(ErrorString, ErrStrLen,
                    "Must start a cmd list with the '%s' token.",
                    START_PORT_LIST);

            return FTPP_FATAL_ERR;
        }

        while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
        {
            if(!strcmp(END_PORT_LIST, pcToken))
            {
                iEndCmds = 1;
                break;
            }

            cmd = pcToken;

            FTPCmd = ftp_cmd_lookup_find(ServerConf->cmd_lookup, cmd,
                    strlen(cmd), &iRet);

            if (FTPCmd == NULL)
            {
                /* Add it to the list */
                // note that struct includes 1 byte for null, so just add len
                FTPCmd = (FTP_CMD_CONF *)_dpd.snortAlloc(1,
                                          sizeof(FTP_CMD_CONF)+strlen(cmd),
                                          PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
                if (FTPCmd == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                            *(_dpd.config_file), *(_dpd.config_line));
                }

                strcpy(FTPCmd->cmd_name, cmd);

                ftp_cmd_lookup_add(ServerConf->cmd_lookup, cmd,
                        strlen(cmd), FTPCmd);
                FTPCmd->max_param_len = ServerConf->def_max_param_len;
            }

            if (require_length)
            {
                FTPCmd->max_param_len = iLength;
                FTPCmd->max_param_len_overridden = 1;
            }
        }

        if(!iEndCmds)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Must end '%s' configuration with '%s'.",
                    FTP_CMDS, END_PORT_LIST);

            return FTPP_FATAL_ERR;
        }
    }

    if (!strcmp(confOption, MAX_PARAM_LEN))
    {
        ServerConf->def_max_param_len = iLength;
        /* Reset the max length to the default for all existing commands  */
        FTPCmd = ftp_cmd_lookup_first(ServerConf->cmd_lookup, &iRet);
        while (FTPCmd)
        {
            if (!FTPCmd->max_param_len_overridden)
            {
                FTPCmd->max_param_len = ServerConf->def_max_param_len;
            }
            FTPCmd = ftp_cmd_lookup_next(ServerConf->cmd_lookup, &iRet);
        }
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ResetStringFormat (FTP_PARAM_FMT *Fmt)
 *
 * Purpose: Recursively sets nodes that allow strings to nodes that check
 *          for a string format attack within the FTP parameter validation tree
 *
 * Arguments: Fmt       => pointer to the FTP Parameter configuration
 *
 * Returns: None
 *
 */
void ResetStringFormat (FTP_PARAM_FMT *Fmt)
{
    int i;
    if (!Fmt)
        return;

    if (Fmt->type == e_unrestricted)
        Fmt->type = e_strformat;

    ResetStringFormat(Fmt->optional_fmt);
    for (i=0;i<Fmt->numChoices;i++)
    {
        ResetStringFormat(Fmt->choices[i]);
    }
    ResetStringFormat(Fmt->next_param_fmt);
}

/*
 * Function: ProcessFTPDataChanCmdsList(FTP_SERVER_PROTO_CONF *ServerConf,
 *                             char *confOption,
 *                             char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the FTP cmd lists for the client configuration.
 *          This configuration is an indicator of data channels, data transfer,
 *          string format, encryption, or login commands.
 *
 * Arguments: ServerConf    => pointer to the FTP server configuration
 *            confOption    => pointer to the name of the option
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessFTPDataChanCmdsList(FTP_SERVER_PROTO_CONF *ServerConf,
        char *confOption,
        char *ErrorString, int ErrStrLen)
{
    FTP_CMD_CONF *FTPCmd = NULL;
    char *pcToken;
    char *cmd;
    int  iEndCmds = 0;
    int  iRet;

    pcToken = NextToken(CONF_SEPARATORS);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid %s list format.", confOption);

        return FTPP_FATAL_ERR;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a %s list with the '%s' token.",
                confOption, START_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndCmds = 1;
            break;
        }

        cmd = pcToken;

        FTPCmd = ftp_cmd_lookup_find(ServerConf->cmd_lookup, cmd,
                strlen(cmd), &iRet);

        if (FTPCmd == NULL)
        {
            /* Add it to the list */
            // note that struct includes 1 byte for null, so just add len
            FTPCmd = (FTP_CMD_CONF *)_dpd.snortAlloc(1,
                                         sizeof(FTP_CMD_CONF) + strlen(cmd),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            if (FTPCmd == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }

            strcpy(FTPCmd->cmd_name, cmd);

            FTPCmd->max_param_len = ServerConf->def_max_param_len;

            ftp_cmd_lookup_add(ServerConf->cmd_lookup, cmd,
                    strlen(cmd), FTPCmd);
        }

        if (!strcmp(confOption, DATA_CHAN_CMD))
            FTPCmd->data_chan_cmd = 1;
        else if (!strcmp(confOption, DATA_XFER_CMD))
            FTPCmd->data_xfer_cmd = 1;
        else if (!strcmp(confOption, DATA_REST_CMD))
            FTPCmd->data_rest_cmd = 1;
        else if (!strcmp(confOption, FILE_PUT_CMD))
        {
            FTPCmd->data_xfer_cmd = 1;
            FTPCmd->file_put_cmd = 1;
        }
        else if (!strcmp(confOption, FILE_GET_CMD))
        {
            FTPCmd->data_xfer_cmd = 1;
            FTPCmd->file_get_cmd = 1;
        }
        else if (!strcmp(confOption, STRING_FORMAT))
        {
            FTP_PARAM_FMT *Fmt = FTPCmd->param_format;
            if (Fmt)
            {
                ResetStringFormat(Fmt);
            }
            else
            {
                Fmt = (FTP_PARAM_FMT *)_dpd.snortAlloc(1, sizeof(FTP_PARAM_FMT),
                                          PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
                if (Fmt == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                            *(_dpd.config_file), *(_dpd.config_line));
                }

                Fmt->type = e_head;
                FTPCmd->param_format = Fmt;

                Fmt = (FTP_PARAM_FMT *)_dpd.snortAlloc(1, sizeof(FTP_PARAM_FMT),
                                          PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
                if (Fmt == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                            *(_dpd.config_file), *(_dpd.config_line));
                }

                Fmt->type = e_strformat;
                FTPCmd->param_format->next_param_fmt = Fmt;
                Fmt->prev_param_fmt = FTPCmd->param_format;
            }
            FTPCmd->check_validity = 1;
        }
        else if (!strcmp(confOption, ENCR_CMD))
            FTPCmd->encr_cmd = 1;
        else if (!strcmp(confOption, LOGIN_CMD))
            FTPCmd->login_cmd = 1;
    }

    if(!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end '%s' configuration with '%s'.",
                confOption, END_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessFTPDirCmdsList(FTP_SERVER_PROTO_CONF *ServerConf,
 *                             char *confOption,
 *                             char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the FTP cmd lists for the client configuration.
 *          This configuration is an indicator of commands used to
 *          retrieve or update the current directory.
 *
 * Arguments: ServerConf    => pointer to the FTP server configuration
 *            confOption    => pointer to the name of the option
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessFTPDirCmdsList(FTP_SERVER_PROTO_CONF *ServerConf,
        char *confOption,
        char *ErrorString, int ErrStrLen)
{
    FTP_CMD_CONF *FTPCmd = NULL;
    char *pcToken;
    char *pcEnd = NULL;
    char *cmd;
    int  iCode;
    int  iEndCmds = 0;
    int  iRet;

    pcToken = NextToken(CONF_SEPARATORS);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid %s list format.", confOption);

        return FTPP_FATAL_ERR;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a %s list with the '%s' token.",
                confOption, START_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndCmds = 1;
            break;
        }

        cmd = pcToken;

        FTPCmd = ftp_cmd_lookup_find(ServerConf->cmd_lookup, cmd,
                strlen(cmd), &iRet);

        if (FTPCmd == NULL)
        {
            /* Add it to the list  */
            // note that struct includes 1 byte for null, so just add len
            FTPCmd = (FTP_CMD_CONF *)_dpd.snortAlloc(1,
                                         sizeof(FTP_CMD_CONF) + strlen(cmd),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            if (FTPCmd == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }

            strcpy(FTPCmd->cmd_name, cmd);

            FTPCmd->max_param_len = ServerConf->def_max_param_len;

            ftp_cmd_lookup_add(ServerConf->cmd_lookup, cmd,
                    strlen(cmd), FTPCmd);
        }

        pcToken = NextToken(CONF_SEPARATORS);

        if (!pcToken)
        {
            snprintf(ErrorString, ErrStrLen,
                    "FTP Dir Cmds must have associated response code: '%s'.",
                    cmd);

            return FTPP_FATAL_ERR;
        }

        iCode = strtol(pcToken, &pcEnd, 10);

        /*
         * Let's check to see if the entire string was valid.
         * If there is an address here, then there was an
         * invalid character in the string.
         */
        if((*pcEnd) || (iCode < 0))
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid argument to token '%s'.  "
                    "Code must be a positive number",
                    confOption);

            return FTPP_FATAL_ERR;
        }

        FTPCmd->dir_response = iCode;
    }

    if(!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end '%s' configuration with '%s'.",
                confOption, END_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

static int ProcessFTPIgnoreDataChan(FTP_SERVER_PROTO_CONF *ServerConf,
        char *confOption,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;

    pcToken = NextToken(CONF_SEPARATORS);
    if (pcToken == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "No argument provided to option '%s'. "
                "Argument must be 'yes' or 'no'.",
                confOption);
        return FTPP_FATAL_ERR;
    }
    if (!strcasecmp("yes", pcToken))
    {
        ServerConf->data_chan = 1;
    }
    else if (!strcasecmp("no", pcToken))
    {
        if (ServerConf->data_chan == 1)
        {
            snprintf(ErrorString, ErrStrLen, "Both 'data_chan' and "
                    "'ignore_data_chan' configured with conflicting options.");
            return FTPP_FATAL_ERR;
        }
        ServerConf->data_chan = 0;
    }
    else
    {
        snprintf(ErrorString, ErrStrLen, "Invalid argument to token '%s'. "
                "Argument must be 'yes' or 'no'.", confOption);
        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: SetOptionalsNext(FTP_PARAM_FMT *ThisFmt,
 *                            FTP_PARAM_FMT *NextFmt,
 *                            FTP_PARAM_FMT **choices,
 *                            int numChoices)
 *
 * Purpose: Recursively updates the next value for nodes in the FTP
 *          Parameter validation tree.
 *
 * Arguments: ThisFmt       => pointer to an FTP parameter validation node
 *            NextFmt       => pointer to an FTP parameter validation node
 *            choices       => pointer to a list of FTP parameter
 *                             validation nodes
 *            numChoices    => the number of nodes in the list
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static void SetOptionalsNext(FTP_PARAM_FMT *ThisFmt, FTP_PARAM_FMT *NextFmt,
        FTP_PARAM_FMT **choices, int numChoices)
{
    if (!ThisFmt)
        return;

    if (ThisFmt->optional)
    {
        if (ThisFmt->next_param_fmt == NULL)
        {
            ThisFmt->next_param_fmt = NextFmt;
            if (numChoices)
            {
                ThisFmt->numChoices = numChoices;
                ThisFmt->choices = (FTP_PARAM_FMT **)_dpd.snortAlloc(numChoices,
                                                        sizeof(FTP_PARAM_FMT *),
                                                        PP_FTPTELNET,
                                                        PP_MEM_CATEGORY_CONFIG);
                if (ThisFmt->choices == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                            *(_dpd.config_file), *(_dpd.config_line));
                }

                memcpy(ThisFmt->choices, choices, sizeof(FTP_PARAM_FMT *) * numChoices);
            }
        }
        else
        {
            SetOptionalsNext(ThisFmt->next_param_fmt, NextFmt,
                    choices, numChoices);
        }
    }
    else
    {
        int i;
        SetOptionalsNext(ThisFmt->optional_fmt, ThisFmt->next_param_fmt,
                ThisFmt->choices, ThisFmt->numChoices);
        for (i=0;i<ThisFmt->numChoices;i++)
        {
            SetOptionalsNext(ThisFmt->choices[i], ThisFmt,
                    choices, numChoices);
        }
        SetOptionalsNext(ThisFmt->next_param_fmt, ThisFmt,
                choices, numChoices);
    }
}

/*
 * Function: ProcessDateFormat(FTP_DATE_FMT *dateFmt,
 *                             FTP_DATE_FMT *LastNonOptFmt,
 *                             char **format)
 *
 * Purpose: Sets the value for nodes in the FTP Date validation tree.
 *
 * Arguments: dateFmt       => pointer to an FTP date validation node
 *            LastNonOptFmt => pointer to previous FTP date validation node
 *            format        => pointer to next part of date validation string
 *                             Updated on function exit.
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessDateFormat(FTP_DATE_FMT *dateFmt,
        FTP_DATE_FMT *LastNonOptFmt,
        char **format)
{
    char *curr_format;
    int iRet = FTPP_SUCCESS;
    int curr_len = 0;
    char *curr_ch;
    char *start_ch;
    FTP_DATE_FMT *CurrFmt = dateFmt;

    if (!dateFmt)
        return FTPP_INVALID_ARG;

    if (!format || !*format)
        return FTPP_INVALID_ARG;

    start_ch = curr_ch = *format;

    while (*curr_ch != '\0')
    {
        switch (*curr_ch)
        {
            case 'n':
            case 'C':
            case '+':
            case '-':
            case '.':
                curr_len++;
                curr_ch++;
                break;
            case '[':
                curr_ch++;
                if (curr_len > 0)
                {
                    FTP_DATE_FMT *OptFmt;
                    OptFmt = (FTP_DATE_FMT *)_dpd.snortAlloc(1,
                                                   sizeof(FTP_DATE_FMT),
                                                   PP_FTPTELNET,
                                                   PP_MEM_CATEGORY_CONFIG);
                    if (OptFmt == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    curr_format = (char *)_dpd.snortAlloc(curr_len + 1,
                                                        sizeof(char),
                                                        PP_FTPTELNET,
                                                        PP_MEM_CATEGORY_CONFIG);
                    if (curr_format == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    strncpy(curr_format, start_ch, curr_len);
                    CurrFmt->format_string = curr_format;
                    CurrFmt->optional = OptFmt;
                    OptFmt->prev = CurrFmt;
                    iRet = ProcessDateFormat(OptFmt, CurrFmt, &curr_ch);
                    if (iRet != FTPP_SUCCESS)
                    {
                        _dpd.snortFree(OptFmt, sizeof(FTP_DATE_FMT),
                                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
                        _dpd.snortFree(curr_format,
                                       (curr_len + 1) * sizeof(char),
                                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
                        return iRet;
                    }
                    curr_len = 0;
                }
                start_ch = curr_ch;
                break;
            case ']':
                curr_ch++;
                if (curr_len > 0)
                {
                    curr_format = (char *)_dpd.snortAlloc(curr_len + 1,
                                                          sizeof(char),
                                                          PP_FTPTELNET,
                                                          PP_MEM_CATEGORY_CONFIG);
                    if (curr_format == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    strncpy(curr_format, start_ch, curr_len);
                    CurrFmt->format_string = curr_format;
                    curr_len = 0;
                }
                *format = curr_ch;
                return FTPP_SUCCESS;
                break;
            case '{':
                curr_ch++;
                {
                    FTP_DATE_FMT *NewFmt;
                    NewFmt = (FTP_DATE_FMT *)_dpd.snortAlloc(1,
                                                           sizeof(FTP_DATE_FMT),
                                                           PP_FTPTELNET,
                                                           PP_MEM_CATEGORY_CONFIG);
                    if (NewFmt == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    if (curr_len > 0)
                    {
                        curr_format = (char *)_dpd.snortAlloc(curr_len + 1,
                                                              sizeof(char),
                                                              PP_FTPTELNET,
                                                              PP_MEM_CATEGORY_CONFIG);
                        if (curr_format == NULL)
                        {
                            DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                    *(_dpd.config_file), *(_dpd.config_line));
                        }

                        strncpy(curr_format, start_ch, curr_len);
                        CurrFmt->format_string = curr_format;
                        curr_len = 0;
                    }
                    else
                    {
                        CurrFmt->empty = 1;
                    }
                    NewFmt->prev = LastNonOptFmt;
                    CurrFmt->next_a = NewFmt;
                    iRet = ProcessDateFormat(NewFmt, CurrFmt, &curr_ch);
                    if (iRet != FTPP_SUCCESS)
                    {
                        return iRet;
                    }
                    NewFmt = (FTP_DATE_FMT *)_dpd.snortAlloc(1,
                                                  sizeof(FTP_DATE_FMT),
                                                  PP_FTPTELNET,
                                                  PP_MEM_CATEGORY_CONFIG);
                    if (NewFmt == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    NewFmt->prev = LastNonOptFmt;
                    CurrFmt->next_b = NewFmt;
                    iRet = ProcessDateFormat(NewFmt, CurrFmt, &curr_ch);
                    if (iRet != FTPP_SUCCESS)
                    {
                        return iRet;
                    }

                    NewFmt = (FTP_DATE_FMT *)_dpd.snortAlloc(1,
                                                   sizeof(FTP_DATE_FMT),
                                                   PP_FTPTELNET,
                                                   PP_MEM_CATEGORY_CONFIG);
                    if (NewFmt == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    NewFmt->prev = CurrFmt;
                    CurrFmt->next = NewFmt;
                    iRet = ProcessDateFormat(NewFmt, CurrFmt, &curr_ch);
                    if (iRet != FTPP_SUCCESS)
                    {
                        return iRet;
                    }
                }
                break;
            case '}':
                curr_ch++;
                if (curr_len > 0)
                {
                    curr_format = (char *)_dpd.snortAlloc(curr_len + 1,
                                               sizeof(char), PP_FTPTELNET,
                                               PP_MEM_CATEGORY_CONFIG);
                    if (curr_format == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    strncpy(curr_format, start_ch, curr_len);
                    CurrFmt->format_string = curr_format;
                    curr_len = 0;
                    *format = curr_ch;
                    return FTPP_SUCCESS;
                }
                else
                {
                    CurrFmt->empty = 1;
                    *format = curr_ch;
                    return FTPP_SUCCESS;
                }
                break;
            case '|':
                curr_ch++;
                if (curr_len > 0)
                {
                    curr_format = (char *)_dpd.snortAlloc(curr_len + 1,
                                                    sizeof(char), PP_FTPTELNET,
                                                    PP_MEM_CATEGORY_CONFIG);
                    if (curr_format == NULL)
                    {
                        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                                *(_dpd.config_file), *(_dpd.config_line));
                    }

                    strncpy(curr_format, start_ch, curr_len);
                    CurrFmt->format_string = curr_format;
                    curr_len = 0;
                    *format = curr_ch;
                    return FTPP_SUCCESS;
                }
                else
                {
                    CurrFmt->empty = 1;
                    *format = curr_ch;
                    return FTPP_SUCCESS;
                }
                break;
            default:
                /* Uh, shouldn't get this.  */
                return FTPP_INVALID_ARG;
                break;
        }
    }

    if (curr_len > 0)
    {
        curr_format = (char *)_dpd.snortAlloc(curr_len + 1, sizeof(char),
                                      PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (curr_format == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        strncpy(curr_format, start_ch, curr_len);
        CurrFmt->format_string = curr_format;
        start_ch = curr_ch;
        curr_len = 0;
    }

    /* Should've closed all options & ORs  */
    *format = curr_ch;
    return FTPP_SUCCESS;
}

/*
 * Function: DoNextFormat(FTP_PARAM_FMT *ThisFmt, int allocated,
 *                 char *ErrorString, int ErrStrLen)
 *
 * Purpose: Processes the next FTP parameter validation node.
 *
 * Arguments: ThisFmt       => pointer to an FTP parameter validation node
 *            allocated     => indicator whether the next node is allocated
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int DoNextFormat(FTP_PARAM_FMT *ThisFmt, int allocated,
        char *ErrorString, int ErrStrLen)
{
    FTP_PARAM_FMT *NextFmt;
    int iRet = FTPP_SUCCESS;
    char *fmt = NextToken(CONF_SEPARATORS);

    if (!fmt)
        return FTPP_INVALID_ARG;

    if(!strcmp(END_CMD_FORMAT, fmt))
    {
        return FTPP_SUCCESS;
    }

    if (!strcmp(fmt, OR_FMT))
    {
        return FTPP_OR_FOUND;
    }

    if (!strcmp(fmt, END_OPT_FMT))
    {
        return FTPP_OPT_END_FOUND;
    }

    if (!strcmp(fmt, END_CHOICE_FMT))
    {
        return FTPP_CHOICE_END_FOUND;
    }

    if (!strcmp(fmt, START_OPT_FMT))
    {
        NextFmt = (FTP_PARAM_FMT *)_dpd.snortAlloc(1, sizeof(FTP_PARAM_FMT),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (NextFmt == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        ThisFmt->optional_fmt = NextFmt;
        NextFmt->optional = 1;
        NextFmt->prev_param_fmt = ThisFmt;
        if (ThisFmt->optional)
            NextFmt->prev_optional = 1;
        iRet = DoNextFormat(NextFmt, 1, ErrorString, ErrStrLen);
        if (iRet != FTPP_OPT_END_FOUND)
        {
            return FTPP_INVALID_ARG;
        }

        return DoNextFormat(ThisFmt, 0, ErrorString, ErrStrLen);
    }

    if (!strcmp(fmt, START_CHOICE_FMT))
    {
        int numChoices = 1;
        do
        {
            FTP_PARAM_FMT **tmpChoices =
                    (FTP_PARAM_FMT **)_dpd.snortAlloc(numChoices,
                                                      sizeof(FTP_PARAM_FMT *),
                                                      PP_FTPTELNET,
                                                      PP_MEM_CATEGORY_CONFIG);
            if (tmpChoices == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }

            if (ThisFmt->numChoices)
            {
                /* explicit check that we have enough room for copy */
                if (numChoices <= ThisFmt->numChoices)
                    DynamicPreprocessorFatalMessage("%s(%d) => Can't do memcpy - index out of range \n",
                            *(_dpd.config_file), *(_dpd.config_line));

                memcpy(tmpChoices, ThisFmt->choices,
                        sizeof(FTP_PARAM_FMT*) * ThisFmt->numChoices);
            }
            NextFmt = (FTP_PARAM_FMT *)_dpd.snortAlloc(1, sizeof(FTP_PARAM_FMT),
                                          PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            if (NextFmt == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }

            tmpChoices[numChoices-1] = NextFmt;
            if (ThisFmt->choices)
                _dpd.snortFree(ThisFmt->choices,
                               (ThisFmt->numChoices * sizeof(FTP_PARAM_FMT *)),
                               PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            ThisFmt->numChoices = numChoices;
            ThisFmt->choices = tmpChoices;
            NextFmt->prev_param_fmt = ThisFmt;
            iRet = DoNextFormat(NextFmt, 1, ErrorString, ErrStrLen);
            numChoices++;
        }
        while (iRet == FTPP_OR_FOUND);

        if (iRet != FTPP_CHOICE_END_FOUND)
        {
            return FTPP_INVALID_ARG;
        }

        return DoNextFormat(ThisFmt, 0, ErrorString, ErrStrLen);
    }

    if (!allocated)
    {
        NextFmt = (FTP_PARAM_FMT *)_dpd.snortAlloc(1, sizeof(FTP_PARAM_FMT),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (NextFmt == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        NextFmt->prev_param_fmt = ThisFmt;
        ThisFmt->next_param_fmt = NextFmt;
        if (ThisFmt->optional)
            NextFmt->prev_optional = 1;
    }
    else
    {
        NextFmt = ThisFmt;
    }

    /* If its not an end cmd, OR, START/END Opt...
     * it must be a parameter specification.
     */
    /* Setup the type & format specs  */
    if (!strcmp(fmt, F_INT))
    {
        NextFmt->type = e_int;
    }
    else if (!strcmp(fmt, F_NUMBER))
    {
        NextFmt->type = e_number;
    }
    else if (!strcmp(fmt, F_CHAR))
    {
        char *chars_allowed = NextToken(CONF_SEPARATORS);
        if(!chars_allowed)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Illegal format '' for token '%s'.",
                     CMD_VALIDITY);
            return FTPP_INVALID_ARG;
        }

        NextFmt->type = e_char;
        NextFmt->format.chars_allowed = 0;
        while (*chars_allowed != 0)
        {
            int bitNum = (*chars_allowed & 0x1f);
            NextFmt->format.chars_allowed |= (1 << (bitNum-1));
            chars_allowed++;
        }
    }
    else if (!strcmp(fmt, F_DATE))
    {
        FTP_DATE_FMT *DateFmt;
        char *format = NextToken(CONF_SEPARATORS);
        NextFmt->type = e_date;
        DateFmt = (FTP_DATE_FMT *)_dpd.snortAlloc(1, sizeof(FTP_DATE_FMT),
                                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (DateFmt == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        NextFmt->format.date_fmt = DateFmt;
        iRet = ProcessDateFormat(DateFmt, NULL, &format);
        if (iRet)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Illegal format %s for token '%s'.",
                    format, CMD_VALIDITY);

            return FTPP_INVALID_ARG;
        }
    }
    else if ( *fmt == *F_LITERAL )
    {
        char* end = strchr(++fmt, *F_LITERAL);
        int len = end ? end - fmt : 0;

        if ( len < 1 )
        {
            snprintf(
                    ErrorString, ErrStrLen,
                    "Illegal format '' for token '%s'.", CMD_VALIDITY
                    );
            return FTPP_INVALID_ARG;
        }
        NextFmt->type = e_literal;
        NextFmt->format.literal = (char *)_dpd.snortAlloc(1, len + 1,
                                        PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if ( !NextFmt->format.literal )
        {
            DynamicPreprocessorFatalMessage(
                    "%s(%d) => Failed to allocate memory\n",
                    *(_dpd.config_file), *(_dpd.config_line)
                    );
        }
        strncpy(NextFmt->format.literal, fmt, len);
        NextFmt->format.literal[len] = '\0';
    }
    else if (!strcmp(fmt, F_STRING))
    {
        NextFmt->type = e_unrestricted;
    }
    else if (!strcmp(fmt, F_HOST_PORT))
    {
        NextFmt->type = e_host_port;
    }
    else if (!strcmp(fmt, F_LONG_HOST_PORT))
    {
        NextFmt->type = e_long_host_port;
    }
    else if (!strcmp(fmt, F_EXTD_HOST_PORT))
    {
        NextFmt->type = e_extd_host_port;
    }
    else
    {
        snprintf(ErrorString, ErrStrLen,
                "Illegal format type %s for token '%s'.",
                fmt, CMD_VALIDITY);

        return FTPP_INVALID_ARG;
    }

    return DoNextFormat(NextFmt, 0, ErrorString, ErrStrLen);
}

/*
 * Function: ProcessFTPCmdValidity(FTP_SERVER_PROTO_CONF *ServerConf,
 *                              char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the ftp cmd validity configuration.
 *          This sets the FTP command parameter validation tree.
 *
 * Arguments: ServerConf    => pointer to the FTP server configuration
 *            confOption    => pointer to the name of the option
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessFTPCmdValidity(FTP_SERVER_PROTO_CONF *ServerConf,
        char *ErrorString, int ErrStrLen)
{
    FTP_CMD_CONF *FTPCmd = NULL;
    FTP_PARAM_FMT *HeadFmt = NULL;
    char *cmd;
    char *fmt;
    int iRet;

    fmt = NextToken(CONF_SEPARATORS);
    if(fmt == NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "No argument to token '%s'.", CMD_VALIDITY);

        return FTPP_FATAL_ERR;
    }

    cmd = fmt;

    fmt = NextToken(CONF_SEPARATORS);
    if(!fmt)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid cmd validity format.");

        return FTPP_FATAL_ERR;
    }

    if(strcmp(START_CMD_FORMAT, fmt))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a cmd validity with the '%s' token.",
                START_CMD_FORMAT);

        return FTPP_FATAL_ERR;
    }

    HeadFmt = (FTP_PARAM_FMT *)_dpd.snortAlloc(1, sizeof(FTP_PARAM_FMT),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    if (HeadFmt == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                *(_dpd.config_file), *(_dpd.config_line));
    }

    HeadFmt->type = e_head;

    iRet = DoNextFormat(HeadFmt, 0, ErrorString, ErrStrLen);

    /* Need to check to be sure we got a complete command  */
    if (iRet)
    {
        return FTPP_FATAL_ERR;
    }

    SetOptionalsNext(HeadFmt, NULL, NULL, 0);

    FTPCmd = ftp_cmd_lookup_find(ServerConf->cmd_lookup, cmd,
            strlen(cmd), &iRet);
    if (FTPCmd == NULL)
    {
        /* Add it to the list  */
        // note that struct includes 1 byte for null, so just add len
        FTPCmd = (FTP_CMD_CONF *)_dpd.snortAlloc(1,
                                         sizeof(FTP_CMD_CONF)+strlen(cmd),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (FTPCmd == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        strcpy(FTPCmd->cmd_name, cmd);

        FTPCmd->max_param_len = ServerConf->def_max_param_len;
        ftp_cmd_lookup_add(ServerConf->cmd_lookup, cmd, strlen(cmd), FTPCmd);
    }

    FTPCmd->check_validity = 1;
    if (FTPCmd->param_format)
    {
        ftpp_ui_config_reset_ftp_cmd_format(FTPCmd->param_format);
        FTPCmd->param_format = NULL;
    }
    FTPCmd->param_format = HeadFmt;

    return FTPP_SUCCESS;
}

/*
 * Function: PrintFormatDate(FTP_DATE_FMT *DateFmt)
 *
 * Purpose: Recursively prints the FTP date validation tree
 *
 * Arguments: DateFmt       => pointer to the date format node
 *
 * Returns: None
 *
 */
static void PrintFormatDate(char *buf, FTP_DATE_FMT *DateFmt)
{
    FTP_DATE_FMT *OptChild;

    if (!DateFmt->empty)
        _dpd.printfappend(buf, BUF_SIZE, "%s", DateFmt->format_string);

    if (DateFmt->optional)
    {
        OptChild = DateFmt->optional;
        _dpd.printfappend(buf, BUF_SIZE, "[");
        PrintFormatDate(buf, OptChild);
        _dpd.printfappend(buf, BUF_SIZE, "]");
    }

    if (DateFmt->next_a)
    {
        if (DateFmt->next_b)
            _dpd.printfappend(buf, BUF_SIZE, "{");
        OptChild = DateFmt->next_a;
        PrintFormatDate(buf, OptChild);
        if (DateFmt->next_b)
        {
            _dpd.printfappend(buf, BUF_SIZE, "|");
            OptChild = DateFmt->next_b;
            PrintFormatDate(buf, OptChild);
            _dpd.printfappend(buf, BUF_SIZE, "}");
        }
    }

    if (DateFmt->next)
        PrintFormatDate(buf, DateFmt->next);
}

/*
 * Function: PrintCmdFmt(FTP_PARAM_FMT *CmdFmt)
 *
 * Purpose: Recursively prints the FTP command parameter validation tree
 *
 * Arguments: CmdFmt       => pointer to the parameter validation node
 *
 * Returns: None
 *
 */
static void PrintCmdFmt(char *buf, FTP_PARAM_FMT *CmdFmt)
{
    FTP_PARAM_FMT *OptChild;

    switch(CmdFmt->type)
    {
        case e_int:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_INT);
            break;
        case e_number:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_NUMBER);
            break;
        case e_char:
            _dpd.printfappend(buf, BUF_SIZE, " %s 0x%x", F_CHAR,
                    CmdFmt->format.chars_allowed);
            break;
        case e_date:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_DATE);
            PrintFormatDate(buf, CmdFmt->format.date_fmt);
            break;
        case e_literal:
            _dpd.printfappend(buf, BUF_SIZE, " %s 0x%x", F_LITERAL,
                    CmdFmt->format.literal);
            break;
        case e_unrestricted:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_STRING);
            break;
        case e_strformat:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_STRING_FMT);
            break;
        case e_host_port:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_HOST_PORT);
            break;
        case e_long_host_port:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_LONG_HOST_PORT);
            break;
        case e_extd_host_port:
            _dpd.printfappend(buf, BUF_SIZE, " %s", F_EXTD_HOST_PORT);
            break;
        case e_head:
            break;
        default:
            break;
    }

    if (CmdFmt->optional_fmt)
    {
        OptChild = CmdFmt->optional_fmt;
        _dpd.printfappend(buf, BUF_SIZE, "[");
        PrintCmdFmt(buf, OptChild);
        _dpd.printfappend(buf, BUF_SIZE, "]");
    }

    if (CmdFmt->numChoices)
    {
        int i;
        _dpd.printfappend(buf, BUF_SIZE, "{");
        for (i=0;i<CmdFmt->numChoices;i++)
        {
            if (i)
                _dpd.printfappend(buf, BUF_SIZE, "|");
            OptChild = CmdFmt->choices[i];
            PrintCmdFmt(buf, OptChild);
        }
        _dpd.printfappend(buf, BUF_SIZE, "}");
    }

    if (CmdFmt->next_param_fmt && CmdFmt->next_param_fmt->prev_optional)
        PrintCmdFmt(buf, CmdFmt->next_param_fmt);

}

/*
 * Function: ProcessFTPMaxRespLen(FTP_CLIENT_PROTO_CONF *ClientConf,
 *                                char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the max response length configuration
 *          This sets the max length of an FTP response that we
 *          will tolerate, before alerting.
 *
 * Arguments: ClientConf    => pointer to the FTP client configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessFTPMaxRespLen(FTP_CLIENT_PROTO_CONF *ClientConf,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    char *pcEnd = NULL;
    long int max_resp_len;

    pcToken = NextToken(CONF_SEPARATORS);
    if(pcToken == NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "No argument to token '%s'.", MAX_RESP_LEN);

        return FTPP_FATAL_ERR;
    }

    max_resp_len = _dpd.SnortStrtol(pcToken, &pcEnd, 10);

    /*
     * Let's check to see if the entire string was valid.
     * If there is an address here, then there was an
     * invalid character in the string.
     */
    if ((*pcEnd) || (max_resp_len < 0) || (errno == ERANGE))
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid argument to token '%s'.  Must be a positive "
                "number.", MAX_RESP_LEN);

        return FTPP_FATAL_ERR;
    }

    ClientConf->max_resp_len = (unsigned int)max_resp_len;

    return FTPP_SUCCESS;
}

/*
 * Function:  ParseBounceTo(char *token, FTP_BOUNCE_TO*)
 *
 * Purpose: Extract the IP address, masking bits (CIDR format), and
 *          port information from an FTP Bounce To configuration.
 *
 * Arguments: token         => string pointer to the FTP bounce configuration
 *                             required format:  IP/CIDR,port[,portHi]\0
 *            FTP_BOUNCE_TO => populated with parsed data
 *
 * Returns:   int           => an error code integer (0 = success,
 *                             >0 = non-fatal error, <0 = fatal error)
 *
 */
int ParseBounceTo(char* token, FTP_BOUNCE_TO* bounce)
{
    char **toks;
    int num_toks;
    long int port_lo;
    char *endptr = NULL;
    sfcidr_t tmp_ip;

    toks = _dpd.tokenSplit(token, ",", 3, &num_toks, 0);
    if (num_toks < 2)
        return FTPP_INVALID_ARG;

    if (sfip_pton(toks[0], &tmp_ip) != SFIP_SUCCESS)
    {
        _dpd.tokenFree(&toks, num_toks);
        return FTPP_INVALID_ARG;
    }

    memcpy(&bounce->ip, &tmp_ip, sizeof(sfcidr_t));

    port_lo = _dpd.SnortStrtol(toks[1], &endptr, 10);
    if ((errno == ERANGE) || (*endptr != '\0') ||
            (port_lo < 0) || (port_lo >= MAXPORTS))
    {
        _dpd.tokenFree(&toks, num_toks);
        return FTPP_INVALID_ARG;
    }

    bounce->portlo = (unsigned short)port_lo;

    if (num_toks == 3)
    {
        long int port_hi = _dpd.SnortStrtol(toks[2], &endptr, 10);

        if ((errno == ERANGE) || (*endptr != '\0') ||
                (port_hi < 0) || (port_hi >= MAXPORTS))
        {
            _dpd.tokenFree(&toks, num_toks);
            return FTPP_INVALID_ARG;
        }

        if (bounce->portlo != (unsigned short)port_hi)
        {
            bounce->porthi = (unsigned short)port_hi;
            if (bounce->porthi < bounce->portlo)
            {
                unsigned short tmp = bounce->porthi;
                bounce->porthi = bounce->portlo;
                bounce->portlo = tmp;
            }
        }
    }

    _dpd.tokenFree(&toks, num_toks);
    return FTPP_SUCCESS;
}

/*
 * Function: ProcessFTPAlowBounce(FTP_CLIENT_PROTO_CONF *ClientConf,
 *                                char *ErrorString, int ErrStrLen)
 *
 * Purpose: Process the FTP allow bounce configuration.
 *          This creates an allow bounce node and adds it to the list for the
 *          client configuration.
 *
 * Arguments: ClientConf    => pointer to the FTP client configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int ProcessFTPAllowBounce(FTP_CLIENT_PROTO_CONF *ClientConf,
        char *ErrorString, int ErrStrLen)
{
    char *pcToken;
    int iOneAddr = 0;
    int iEndList = 0;
    int iRet;

    pcToken = NextToken(CONF_SEPARATORS);
    if(pcToken == NULL)
    {
        snprintf(ErrorString, ErrStrLen,
                "No argument to token '%s'.", ALLOW_BOUNCE);

        return FTPP_FATAL_ERR;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a %s list with the '%s' token.",
                ALLOW_BOUNCE, START_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        FTP_BOUNCE_TO *newBounce;

        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndList = 1;
            break;
        }

        /* TODO: Maybe want to redo this with high-speed searcher for ip/port.
         * Would be great if we could handle both full addresses and
         * subnets quickly -- using CIDR format.  Need something that would
         * return most specific match -- ie a specific host is more specific
         * than subnet.
         */
        newBounce = (FTP_BOUNCE_TO *)_dpd.snortAlloc(1, sizeof(FTP_BOUNCE_TO),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (newBounce == NULL)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Failed to allocate memory for Bounce");
            return FTPP_FATAL_ERR;
        }

        iRet = ParseBounceTo(pcToken, newBounce);
        if (iRet)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid argument to token '%s': %s", ALLOW_BOUNCE, pcToken);
            _dpd.snortFree(newBounce, sizeof(FTP_BOUNCE_TO),
                           PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            return FTPP_FATAL_ERR;
        }

        iRet = ftp_bounce_lookup_add(
                ClientConf->bounce_lookup, &newBounce->ip, newBounce
                );
        if (iRet)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Failed to add configuration for Bounce object '%s'.", ALLOW_BOUNCE);
            _dpd.snortFree(newBounce, sizeof(FTP_BOUNCE_TO),
                           PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            return FTPP_FATAL_ERR;
        }

        iOneAddr = 1;
    }

    if(!iEndList)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end '%s' configuration with '%s'.",
                ALLOW_BOUNCE, END_PORT_LIST);

        return FTPP_FATAL_ERR;
    }

    if(!iOneAddr)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must include at least one address in '%s' configuration.",
                ALLOW_BOUNCE);

        return FTPP_FATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: PrintFTPClientConf(char * client,
 *                              FTP_CLIENT_PROTO_CONF *ClientConf)
 *
 * Purpose: Prints the FTP client configuration
 *
 * Arguments: client        => string pointer to the client IP
 *            ClientConf    => pointer to the client configuration
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int PrintFTPClientConf(char * client, FTP_CLIENT_PROTO_CONF *ClientConf)
{
    FTP_BOUNCE_TO *FTPBounce;
    int iErr;

    if(!ClientConf)
    {
        return FTPP_INVALID_ARG;
    }

    if (!printedFTPHeader)
    {
        _dpd.logMsg("    FTP CONFIG:\n");
        printedFTPHeader = 1;
    }

    _dpd.logMsg("      FTP Client: %s\n", client);

    PrintConfOpt(&ClientConf->bounce, "  Check for Bounce Attacks");
    PrintConfOpt(&ClientConf->telnet_cmds, "  Check for Telnet Cmds");
    PrintConfOpt(&ClientConf->ignore_telnet_erase_cmds, "  Ignore Telnet Cmd Operations");
    _dpd.logMsg("        Max Response Length: %d\n", ClientConf->max_resp_len);

    FTPBounce = ftp_bounce_lookup_first(ClientConf->bounce_lookup, &iErr);
    if (FTPBounce)
    {
        _dpd.logMsg("        Allow FTP bounces to:\n");

        while (FTPBounce)
        {
            char *addr_str;
            char bits_str[5];
            int bits;
            bits_str[0] = '\0';

            addr_str = sfip_to_str(&FTPBounce->ip.addr);
            bits = (int)FTPBounce->ip.bits;
            if (bits != 128)
            {
                snprintf(bits_str, sizeof(bits_str), "/%d",
                         (sfaddr_family(&FTPBounce->ip.addr) == AF_INET) ? ((bits >= 96) ? (bits - 96) : -1) : bits);
            }
            if (FTPBounce->porthi)
            {
                _dpd.logMsg("          Address: %s%s, Ports: %d-%d\n",
                        addr_str, bits_str[0] ? bits_str : "",
                        FTPBounce->portlo, FTPBounce->porthi);
            }
            else
            {
                _dpd.logMsg("          Address: %s%s, Port: %d\n",
                        addr_str, bits_str[0] ? bits_str : "",
                        FTPBounce->portlo);
            }

            FTPBounce = ftp_bounce_lookup_next(ClientConf->bounce_lookup, &iErr);
        }
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessFTPClientOptions(FTP_CLIENT_PROTO_CONF *ClientConf,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: This is where we process the specific ftp client configuration
 *          for FTPTelnet.
 *
 *          We set the values of the ftp client configuraiton here.  Any errors
 *          that are encountered are specified in the error string and the type
 *          of error is returned through the return code, i.e. fatal, non-fatal.
 *
 * Arguments: ClientConf    => pointer to the client configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int ProcessFTPClientOptions(FTP_CLIENT_PROTO_CONF *ClientConf,
        char *ErrorString, int ErrStrLen)
{
    FTPTELNET_CONF_OPT *ConfOpt;
    int  iRet;
    char *pcToken;
    int  iTokens = 0;

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        /*
         * Show that we at least got one token
         */
        iTokens = 1;

        /*
         * Search for configuration keywords
         */
        if(!strcmp(MAX_RESP_LEN, pcToken))
        {
            iRet = ProcessFTPMaxRespLen(ClientConf, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(ALLOW_BOUNCE, pcToken))
        {
            iRet = ProcessFTPAllowBounce(ClientConf, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        /*
         * Start the CONF_OPT configurations.
         */
        else if(!strcmp(BOUNCE, pcToken))
        {
            ConfOpt = &ClientConf->bounce;
            iRet = ProcessConfOpt(ConfOpt, BOUNCE, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(TELNET_CMDS, pcToken))
        {
            ConfOpt = &ClientConf->telnet_cmds;
            iRet = ProcessConfOpt(ConfOpt, TELNET_CMDS, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(IGNORE_TELNET_CMDS, pcToken))
        {
            ConfOpt = &ClientConf->ignore_telnet_erase_cmds;
            iRet = ProcessConfOpt(ConfOpt, IGNORE_TELNET_CMDS, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid keyword '%s' for '%s' configuration.",
                    pcToken, GLOBAL);

            return FTPP_FATAL_ERR;
        }
    }

    /*
     * If there are not any tokens to the configuration, then
     * we let the user know and log the error.  return non-fatal
     * error.
     */
    if(!iTokens)
    {
        snprintf(ErrorString, ErrStrLen,
                "No tokens to '%s %s' configuration.", FTP, CLIENT);

        return FTPP_NONFATAL_ERR;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessFTPClientConf(FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: This is where we process the ftp client configuration for FTPTelnet.
 *
 *          We set the values of the ftp client configuraiton here.  Any errors
 *          that are encountered are specified in the error string and the type
 *          of error is returned through the return code, i.e. fatal, non-fatal.
 *
 *          The configuration options that are dealt with here are:
 *          ports { x }        Ports on which to do FTP checks
 *          telnet_cmds yes|no Detect telnet cmds on FTP command channel
 *          ignore_telnet_erase_cmds yes|no  Do not process telnet EAC and EAL
 *                             commands during normalization of FTP command
 *                             channel.
 *          max_resp_len x     Max response length
 *          bounce yes|no      Detect FTP bounce attacks
 *          bounce_to IP port|port-range Allow FTP bounces to specified IP/ports
 *          data_chan          Ignore data channel OR coordinate with cmd chan
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int ProcessFTPClientConf(struct _SnortConfig *sc, FTPTELNET_GLOBAL_CONF *GlobalConf,
        char *ErrorString, int ErrStrLen)
{
    int  iRet;
    int  retVal = 0;
    char *client;
    char client_list[STD_BUF];
    sfcidr_t ipAddr;
    char *pIpAddressList = NULL;
    char *pIpAddressList2 = NULL;
    char *brkt = NULL;
    char firstIpAddress = 1;
    FTP_CLIENT_PROTO_CONF *new_client_conf = NULL;

    //char *ConfigParseResumePtr = NULL;  // Use this if a default client conf is added
    char ip_list = 0;
    FTP_CLIENT_PROTO_CONF *ftp_conf = NULL;

    /*
     * If not default, create one for this IP
     */
    client = NextToken(CONF_SEPARATORS);

    if ( !client )
    {
        DynamicPreprocessorFatalMessage(
                "%s(%d) Missing ftp_telnet ftp client address.\n",
                *(_dpd.config_file), *(_dpd.config_line));
    }
    else if(strcmp(DEFAULT, client))
    {
        /*
         **  Convert string to IP address
         */
        /// get the first delimiter
        if(strcmp(START_IPADDR_LIST, client) == 0)
        {
            //list begin token matched
            ip_list = 1;
            if ((pIpAddressList = mystrtok(NULL, END_IPADDR_LIST)) == NULL)
            {
                snprintf(ErrorString, ErrStrLen,
                        "Invalid IP Address list in '%s' token.", CLIENT);

                retVal = FTPP_INVALID_ARG;
                goto _return;
            }
        }
        else
        {
            //list begin didn't match so this must be an IP address
            pIpAddressList = client;
        }

        //ConfigParseResumePtr = pIpAddressList+strlen(pIpAddressList);

        pIpAddressList2 = strdup(pIpAddressList);
        if (!pIpAddressList2)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Could not allocate memory for server configuration.");

            retVal = FTPP_INVALID_ARG;
            goto _return;
        }



        for (client = strtok_r(pIpAddressList2, CONF_SEPARATORS, &brkt);
                client;
                client = strtok_r(NULL, CONF_SEPARATORS, &brkt))
        {

            if (sfip_pton(client, &ipAddr) != SFIP_SUCCESS)
            {
                snprintf(ErrorString, ErrStrLen,
                        "Invalid IP to '%s' token.", CLIENT);

                retVal = FTPP_INVALID_ARG;
                goto _return;
            }

            /*
             **  allocate the memory for the client configuration
             */
            if (firstIpAddress)
            {
                // Write this IP into the buffer for printing
                snprintf(client_list, STD_BUF, "%s", client);

                new_client_conf = (FTP_CLIENT_PROTO_CONF *)_dpd.snortAlloc(1,
                                                  sizeof(FTP_CLIENT_PROTO_CONF),
                                                  PP_FTPTELNET,
                                                  PP_MEM_CATEGORY_CONFIG);
                if (new_client_conf == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                            *(_dpd.config_file), *(_dpd.config_line));
                }

                ftpp_ui_config_reset_ftp_client(new_client_conf, 1);

                //process the first IP address as usual
                firstIpAddress = 0;

                ftp_conf = new_client_conf;
            }
            else
            {
                // Write this IP into the buffer for printing
                snprintf(client_list + strlen(client_list), STD_BUF - strlen(client_list) , ", %s", client);

                new_client_conf = ftp_conf;
            }


            ftpp_ui_config_add_ftp_client(GlobalConf, &ipAddr, new_client_conf);

            //create a reference
            new_client_conf->referenceCount++;
        }

        if (firstIpAddress)
        {
            //no IP address was found
            snprintf(ErrorString, ErrStrLen,
                    "Invalid IP Address list in '%s' token.", CLIENT);

            retVal = FTPP_INVALID_ARG;
            goto _return;
        }
    }
    else
    {
        /**default configuration */

        if (GlobalConf->default_ftp_client != NULL)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Cannot configure '%s' settings more than once.", CLIENT);

            retVal = FTPP_INVALID_ARG;
            goto _return;
        }

        GlobalConf->default_ftp_client =
            (FTP_CLIENT_PROTO_CONF *)_dpd.snortAlloc(1,
                                     sizeof(FTP_CLIENT_PROTO_CONF),
                                     PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (GlobalConf->default_ftp_client == NULL)
        {
            DynamicPreprocessorFatalMessage("Out of memory trying to create "
                    "default ftp client configuration.\n");
        }

        ftpp_ui_config_reset_ftp_client(GlobalConf->default_ftp_client, 0);
        ftp_conf = GlobalConf->default_ftp_client;

        //ConfigParseResumePtr = client+strlen(client);
    }

    iRet = ProcessFTPClientOptions(ftp_conf, ErrorString, ErrStrLen);
    if (iRet < 0)
    {
        retVal = FTPP_INVALID_ARG;
        goto _return;
    }

    /*
     * Let's print out the FTP config
     */
    if (ip_list)
    {
        client = &client_list[0];
    }
    else if (pIpAddressList2)
    {
        client = pIpAddressList2;
    }
    PrintFTPClientConf(client, ftp_conf);

_return:
    if (pIpAddressList2)
    {
        free(pIpAddressList2);
    }
    return retVal;
}

/*
 * Function: PrintFTPServerConf(char * server,
 *                              FTP_SERVER_PROTO_CONF *ServerConf)
 *
 * Purpose: Prints the FTP server configuration
 *
 * Arguments: server        => string pointer to the server IP
 *            ServerConf    => pointer to the server configuration
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static int PrintFTPServerConf(char * server, FTP_SERVER_PROTO_CONF *ServerConf)
{
    const char* spaf = "";
    char buf[BUF_SIZE+1];
    int iCtr;
    int iRet;
    FTP_CMD_CONF *FTPCmd;

    if(!ServerConf)
    {
        return FTPP_INVALID_ARG;
    }

    if (!printedFTPHeader)
    {
        _dpd.logMsg("    FTP CONFIG:\n");
        printedFTPHeader = 1;
    }

    if ( _dpd.isPafEnabled() )
        spaf = " (PAF)";

    _dpd.logMsg("      FTP Server: %s\n", server);

    memset(buf, 0, BUF_SIZE+1);
    snprintf(buf, BUF_SIZE, "        Ports%s: ", spaf);

    /*
     * Print out all the applicable ports.
     */
    for(iCtr = 0; iCtr < MAXPORTS; iCtr++)
    {
        if(ServerConf->proto_ports.ports[iCtr])
        {
            _dpd.printfappend(buf, BUF_SIZE, "%d ", iCtr);
        }
    }

    _dpd.logMsg("%s\n", buf);

    PrintConfOpt(&ServerConf->telnet_cmds, "  Check for Telnet Cmds");
    PrintConfOpt(&ServerConf->ignore_telnet_erase_cmds, "  Ignore Telnet Cmd Operations");
    _dpd.logMsg("        Ignore open data channels: %s\n",
            ServerConf->data_chan ? "YES" : "NO");

    if (ServerConf->print_commands)
    {
        _dpd.logMsg("        FTP Commands:\n");

        FTPCmd = ftp_cmd_lookup_first(ServerConf->cmd_lookup, &iRet);
        while (FTPCmd != NULL)
        {
            memset(buf, 0, BUF_SIZE+1);
            snprintf(buf, BUF_SIZE, "          %s { %d ",
                    FTPCmd->cmd_name, FTPCmd->max_param_len);
#ifdef PRINT_DEFAULT_CONFIGS
            if (FTPCmd->data_chan_cmd)
                snprintf(buf, BUF_SIZE, "%s data_chan ");
            if (FTPCmd->data_xfer_cmd)
                snprintf(buf, BUF_SIZE, "%s data_xfer ");
            if (FTPCmd->encr_cmd)
                snprintf(buf, BUF_SIZE, "%s encr ");
#endif

            if (FTPCmd->check_validity)
            {
                FTP_PARAM_FMT *CmdFmt = FTPCmd->param_format;
                while (CmdFmt != NULL)
                {
                    PrintCmdFmt(buf, CmdFmt);

                    CmdFmt = CmdFmt->next_param_fmt;
                }
            }
            _dpd.logMsg("%s}\n", buf);
            FTPCmd = ftp_cmd_lookup_next(ServerConf->cmd_lookup, &iRet);
        }
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ProcessFTPServerOptions(FTP_SERVER_PROTO_CONF *ServerConf,
 *                          char *ErrorString, int ErrStrLen)
 *
 * Purpose: This is where we process the specific ftp server configuration
 *          for FTPTelnet.
 *
 *          We set the values of the ftp server configuraiton here.  Any errors
 *          that are encountered are specified in the error string and the type
 *          of error is returned through the return code, i.e. fatal, non-fatal.
 *
 * Arguments: ServerConf    => pointer to the server configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int ProcessFTPServerOptions(FTP_SERVER_PROTO_CONF *ServerConf,
        char *ErrorString, int ErrStrLen)
{
    FTPTELNET_CONF_OPT *ConfOpt;
    int  iRet = 0;
    char *pcToken;
    int  iTokens = 0;
    int  data_chan_configured = 0;

    while ((pcToken = NextToken(CONF_SEPARATORS)) != NULL)
    {
        /*
         * Show that we at least got one token
         */
        iTokens = 1;

        /*
         * Search for configuration keywords
         */
        if(!strcmp(PORTS, pcToken))
        {
            PROTO_CONF *ports = (PROTO_CONF*)&ServerConf->proto_ports;
            iRet = ProcessPorts(ports, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(FTP_CMDS, pcToken))
        {
            iRet = ProcessFTPCmdList(ServerConf, FTP_CMDS, ErrorString, ErrStrLen, 1, 0);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(MAX_PARAM_LEN, pcToken))
        {
            iRet = ProcessFTPCmdList(ServerConf, MAX_PARAM_LEN, ErrorString, ErrStrLen, 0, 1);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(ALT_PARAM_LEN, pcToken))
        {
            iRet = ProcessFTPCmdList(ServerConf, ALT_PARAM_LEN, ErrorString, ErrStrLen, 1, 1);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(CMD_VALIDITY, pcToken))
        {
            iRet = ProcessFTPCmdValidity(ServerConf, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(STRING_FORMAT, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(DATA_CHAN_CMD, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(DATA_XFER_CMD, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(DATA_REST_CMD, pcToken))
        {
           iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
           if (iRet)
           {
               return iRet;
           }
        }
        else if (!strcmp(FILE_PUT_CMD, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(FILE_GET_CMD, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(ENCR_CMD, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(LOGIN_CMD, pcToken))
        {
            iRet = ProcessFTPDataChanCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(DIR_CMD, pcToken))
        {
            iRet = ProcessFTPDirCmdsList(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if (!strcmp(DATA_CHAN, pcToken))
        {
            if (data_chan_configured && ServerConf->data_chan == 0)
            {
                snprintf(ErrorString, ErrStrLen, "Both 'data_chan' and "
                        "'ignore_data_chan' configured with conflicting options.");
                return FTPP_FATAL_ERR;
            }
            else
            {
                ServerConf->data_chan = 1;
                data_chan_configured = 1;
            }
        }
        else if (!strcmp(PRINT_CMDS, pcToken))
        {
            ServerConf->print_commands = 1;
        }
        else if (!strcmp(IGNORE_DATA_CHAN, pcToken))
        {
            iRet = ProcessFTPIgnoreDataChan(ServerConf, pcToken, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
            data_chan_configured = 1;
        }

        /*
         * Start the CONF_OPT configurations.
         */
        else if(!strcmp(TELNET_CMDS, pcToken))
        {
            ConfOpt = &ServerConf->telnet_cmds;
            iRet = ProcessConfOpt(ConfOpt, TELNET_CMDS, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(IGNORE_TELNET_CMDS, pcToken))
        {
            ConfOpt = &ServerConf->ignore_telnet_erase_cmds;
            iRet = ProcessConfOpt(ConfOpt, IGNORE_TELNET_CMDS, ErrorString, ErrStrLen);
            if (iRet)
            {
                return iRet;
            }
        }
        else
        {
            snprintf(ErrorString, ErrStrLen,
                    "Invalid keyword '%s' for '%s' configuration.",
                    pcToken, GLOBAL);

            return FTPP_FATAL_ERR;
        }
    }

    /*
     * If there are not any tokens to the configuration, then
     * we let the user know and log the error.  return non-fatal
     * error.
     */
    if(!iTokens)
    {
        snprintf(ErrorString, ErrStrLen,
                "No tokens to '%s %s' configuration.", FTP, SERVER);

        return FTPP_NONFATAL_ERR;
    }

    return FTPP_SUCCESS;
}

int parseFtpServerConfigStr( FTP_SERVER_PROTO_CONF *ftp_conf, char *ConfigParseResumePtr,
                             char ip_list, char *ErrorString, int ErrStrLen )
{
    int iRet = 0;
    char *saveMaxToken = maxToken;
    size_t default_conf_len;
    char *default_conf_str = DefaultConf(&default_conf_len);

    /* First, process the default configuration -- namely, the
     * list of FTP commands, and the parameter validation checks  */
    maxToken = default_conf_str + default_conf_len;
    mystrtok(default_conf_str, CONF_SEPARATORS);

    iRet = ProcessFTPServerOptions(ftp_conf, ErrorString, ErrStrLen);

    _dpd.snortFree(default_conf_str, default_conf_len,
                   PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    maxToken = saveMaxToken;

    if (iRet < 0)
        return iRet;

    /* Okay, now we need to reset the mystrtok pointers so we can process
     * the specific server configuration.  Quick hack/trick here: reset
     * the end of the client string to a conf separator, then call mystrtok.
     * That will reset mystrtok's internal pointer to the next token after
     * the client name, which is what we're expecting it to be.
     */
    if (ConfigParseResumePtr < maxToken)
    {
        /* only if there is data after the server/client name */
        if (ip_list)
            *ConfigParseResumePtr-- = END_IPADDR_LIST[0];
        else
            *ConfigParseResumePtr-- = CONF_SEPARATORS[0];

        mystrtok(ConfigParseResumePtr, CONF_SEPARATORS);
        iRet = ProcessFTPServerOptions(ftp_conf, ErrorString, ErrStrLen);
        if (iRet < 0)
            return iRet;
    }

    return iRet;
}

void enableFtpTelnetPortStreamServices( struct _SnortConfig *sc, PROTO_CONF *pc, char *network, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( pc->ports[ port ] )
        {
            _dpd.streamAPI->register_reassembly_port( network, port, direction );
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_FTPTELNET, PROTO_BIT__TCP, port );
        }
    }
}

/*
 * Function: ProcessFTPServerConf::
 *
 * Purpose: This is where we process the ftp server configuration for FTPTelnet.
 *
 *          We set the values of the ftp server configuraiton here.  Any
 *          errors that are encountered are specified in the error string and
 *          the type of error is returned through the return code, i.e. fatal,
 *          non-fatal.
 *
 *          The configuration options that are dealt with here are:
 *          ports { x }             Ports on which to do FTP checks
 *          ftp_cmds { CMD1 CMD2 ... }  Valid FTP commands
 *          def_max_param_len x     Default max param length
 *          alt_max_param_len x { CMD1 ... }  Override default max param len
 *                                  for CMD
 *          chk_str_fmt { CMD1 ...}  Detect string format attacks for CMD
 *          cmd_validity CMD < fmt > Check the parameter validity for CMD
 *          fmt is as follows:
 *              int                 Param is an int
 *              char _chars         Param is one of _chars
 *              date _datefmt       Param follows format specified where
 *                                   # = Number, C=Char, []=optional, |=OR,
 *                                   +-.=literal
 *              []                  Optional parameters
 *              string              Param is string (unrestricted)
 *          data_chan               Ignore data channel
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *            ErrorString   => error string buffer
 *            ErrStrLen     => the length of the error string buffer
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int ProcessFTPServerConf( struct _SnortConfig *sc, FTPTELNET_GLOBAL_CONF *GlobalConf,
                          char *ErrorString, int ErrStrLen )
{
    int  iRet = 0;
    int  retVal = 0;
    char *server;
    char server_list[STD_BUF];
    sfcidr_t ipAddr;
    char *pIpAddressList = NULL;
    char *pIpAddressList2 = NULL;
    char *brkt = NULL;
    char firstIpAddress = 1;
    FTP_SERVER_PROTO_CONF *new_server_conf = NULL;
    char *ConfigParseResumePtr = NULL;
    char ip_list = 0;
    FTP_SERVER_PROTO_CONF *ftp_conf = NULL;

    /*
     * If not default, create one for this IP
     */
    server = NextToken(CONF_SEPARATORS);

    if ( !server )
    {
        DynamicPreprocessorFatalMessage(
                "%s(%d) Missing ftp_telnet ftp server address.\n",
                *(_dpd.config_file), *(_dpd.config_line));
    }
    else if(strcmp(DEFAULT, server))
    {
        if(strcmp(START_IPADDR_LIST, server) == 0)
        {
            //list begin token matched
            ip_list = 1;
            if ((pIpAddressList = mystrtok(NULL, END_IPADDR_LIST)) == NULL)
            {
                snprintf(ErrorString, ErrStrLen,
                        "Invalid IP Address list in '%s' token.", SERVER);

                retVal = FTPP_INVALID_ARG;
                goto _return;
            }
        }
        else
        {
            //list begin didn't match so this must be an IP address
            pIpAddressList = server;
        }

        ConfigParseResumePtr = pIpAddressList+strlen(pIpAddressList);

        pIpAddressList2 = strdup(pIpAddressList);
        if (!pIpAddressList2)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Could not allocate memory for server configuration.");

            retVal = FTPP_INVALID_ARG;
            goto _return;
        }

        for (server = strtok_r(pIpAddressList2, CONF_SEPARATORS, &brkt);
                server;
                server = strtok_r(NULL, CONF_SEPARATORS, &brkt))
        {
            if (sfip_pton(server, &ipAddr) != SFIP_SUCCESS)
            {
                snprintf(ErrorString, ErrStrLen,
                        "Invalid IP to '%s' token.", SERVER);

                retVal = FTPP_INVALID_ARG;
                goto _return;
            }

            if (firstIpAddress)
            {
                /* Write this IP into the buffer for printing */
                snprintf(server_list, STD_BUF, "%s", server);

                new_server_conf = (FTP_SERVER_PROTO_CONF *)_dpd.snortAlloc(1,
                                                 sizeof(FTP_SERVER_PROTO_CONF),
                                         PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
                if (new_server_conf == NULL)
                {
                    DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory\n",
                            *(_dpd.config_file), *(_dpd.config_line));
                }

                ftpp_ui_config_reset_ftp_server(new_server_conf, 1);

                new_server_conf->serverAddr = strdup(server);
                if (new_server_conf->serverAddr == NULL)
                {
                    DynamicPreprocessorFatalMessage("ProcessFTPServerConf(): Out of memory allocing serverAddr.\n");
                }

                ftp_conf = new_server_conf;
                iRet = parseFtpServerConfigStr( ftp_conf, ConfigParseResumePtr, ip_list, ErrorString, ErrStrLen );
                if (iRet)
                {
                    retVal = iRet;
                    goto _return;
                }

                //process the first IP address as usual
                firstIpAddress = 0;
            }
            else
            {
                /* Write this IP into the buffer for printing */
                snprintf(server_list + strlen(server_list), STD_BUF - strlen(server_list) , ", %s", server);

                new_server_conf = ftp_conf;
            }

            ftpp_ui_config_add_ftp_server(GlobalConf, &ipAddr, new_server_conf);
            enableFtpTelnetPortStreamServices( sc, &ftp_conf->proto_ports, server,
                                               SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );

            //create a reference
            new_server_conf->referenceCount++;
        }

        if (firstIpAddress)
        {
            //no IP address was found
            snprintf(ErrorString, ErrStrLen,
                    "Invalid IP Address list in '%s' token.", CLIENT);

            retVal = FTPP_INVALID_ARG;
            goto _return;
        }
    }
    else
    {
        if (GlobalConf->default_ftp_server != NULL)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Cannot configure '%s' settings more than once.", SERVER);

            retVal = FTPP_INVALID_ARG;
            goto _return;
        }

        GlobalConf->default_ftp_server =
            (FTP_SERVER_PROTO_CONF *)_dpd.snortAlloc(1,
                                       sizeof(FTP_SERVER_PROTO_CONF),
                                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
        if (GlobalConf->default_ftp_server == NULL)
        {
            DynamicPreprocessorFatalMessage("Out of memory trying to create "
                    "default ftp server configuration.\n");
        }

        ftpp_ui_config_reset_ftp_server(GlobalConf->default_ftp_server, 0);
        ftp_conf = GlobalConf->default_ftp_server;
        ConfigParseResumePtr = server+strlen(server);
        GlobalConf->default_ftp_server->serverAddr = strdup("default");
        if (GlobalConf->default_ftp_server->serverAddr == NULL)
        {
            _dpd.snortFree(GlobalConf->default_ftp_server,
                           sizeof(FTP_SERVER_PROTO_CONF),
                           PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
            DynamicPreprocessorFatalMessage("Out of memory trying to create "
                    "default ftp server configuration.\n");
        }
        iRet = parseFtpServerConfigStr( ftp_conf, ConfigParseResumePtr, ip_list, ErrorString, ErrStrLen );
        if (iRet)
        {
            retVal = iRet;
            goto _return;
        }

        enableFtpTelnetPortStreamServices( sc, &ftp_conf->proto_ports, NULL,
                                           SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    }

    /*
     * Let's print out the FTP config
     */
    if (ip_list)
    {
        server = &server_list[0];
    }
    else if (pIpAddressList2)
    {
        server = pIpAddressList2;
    }
    PrintFTPServerConf(server, ftp_conf);

_return:
    if (pIpAddressList2)
    {
        free(pIpAddressList2);
    }
    return retVal;
}

/*
 * Function: PrintFTPGlobalConf(FTPTELNET_GLOBAL_CONF *GlobalConf)
 *
 * Purpose: Prints the FTPTelnet preprocessor global configuration
 *
 * Arguments: GlobalConf    => pointer to the global configuration
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int PrintFTPGlobalConf(FTPTELNET_GLOBAL_CONF *GlobalConf)
{
    _dpd.logMsg("FTPTelnet Config:\n");

    _dpd.logMsg("    GLOBAL CONFIG\n");
    _dpd.logMsg("      Inspection Type: %s\n",
            GlobalConf->inspection_type == FTPP_UI_CONFIG_STATELESS ?
            "stateless" : "stateful");
    PrintConfOpt(&GlobalConf->encrypted, "Check for Encrypted Traffic");
    _dpd.logMsg("      Continue to check encrypted data: %s\n",
            GlobalConf->check_encrypted_data ? "YES" : "NO");

    return FTPP_SUCCESS;
}

void FTPTelnetCleanupFTPCMDConf(void *ftpCmd)
{
    FTP_CMD_CONF *FTPCmd = (FTP_CMD_CONF *)ftpCmd;
    /* Free the FTP_PARAM_FMT stuff... */
    ftpp_ui_config_reset_ftp_cmd(FTPCmd);

    _dpd.snortFree(FTPCmd, sizeof(FTP_CMD_CONF)+strlen(FTPCmd->cmd_name),
                          PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
}

void FTPTelnetCleanupFTPServerConf(void *serverConf)
{
    FTP_SERVER_PROTO_CONF *ServerConf = (FTP_SERVER_PROTO_CONF*)serverConf;
    if (ServerConf == NULL)
        return;

    free(ServerConf->serverAddr);
    ServerConf->serverAddr = NULL;

    /* Iterate through each cmd_lookup for this server */
    ftp_cmd_lookup_cleanup(&ServerConf->cmd_lookup);
}

void FTPTelnetCleanupFTPBounceTo(void *ftpBounce)
{
    FTP_BOUNCE_TO *FTPBounce = (FTP_BOUNCE_TO *)ftpBounce;
    _dpd.snortFree(FTPBounce, sizeof(FTP_BOUNCE_TO),
                   PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
}

void FTPTelnetCleanupFTPClientConf(void *clientConf)
{
    FTP_CLIENT_PROTO_CONF *ClientConf = (FTP_CLIENT_PROTO_CONF*)clientConf;
    if (ClientConf == NULL)
        return;

    /* Iterate through each bounce_lookup for this client */
    ftp_bounce_lookup_cleanup(&ClientConf->bounce_lookup);
}

static int FTPTelnetFreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)pData;

    //do any housekeeping before freeing FTPTELNET_GLOBAL_CONF
    sfPolicyUserDataClear (config, policyId);
    FTPTelnetFreeConfig(pPolicyConfig);

    return 0;
}

void FTPTelnetFreeConfigs(tSfPolicyUserContextId GlobalConf)
{
    if (GlobalConf == NULL)
        return;

    sfPolicyUserDataFreeIterate(GlobalConf, FTPTelnetFreeConfigsPolicy);

    sfPolicyConfigDelete(GlobalConf);
}

void FTPTelnetFreeConfig(FTPTELNET_GLOBAL_CONF *GlobalConf)
{
    if (GlobalConf == NULL)
        return;

    if (GlobalConf->default_ftp_client != NULL)
    {
        FTPTelnetCleanupFTPClientConf((void *)GlobalConf->default_ftp_client);
        _dpd.snortFree(GlobalConf->default_ftp_client,
                       sizeof(FTP_CLIENT_PROTO_CONF),
                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    }

    if (GlobalConf->default_ftp_server != NULL)
    {
        FTPTelnetCleanupFTPServerConf((void *)GlobalConf->default_ftp_server);
        _dpd.snortFree(GlobalConf->default_ftp_server,
                       sizeof(FTP_SERVER_PROTO_CONF),
                       PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);
    }

    if (GlobalConf->telnet_config != NULL)
        _dpd.snortFree(GlobalConf->telnet_config, sizeof(TELNET_PROTO_CONF),
                          PP_FTPTELNET, PP_MEM_CATEGORY_CONFIG);

    ftpp_ui_client_lookup_cleanup(&GlobalConf->client_lookup);
    ftpp_ui_server_lookup_cleanup(&GlobalConf->server_lookup);

    _dpd.snortFree(GlobalConf, sizeof(FTPTELNET_GLOBAL_CONF), PP_FTPTELNET,
                                                       PP_MEM_CATEGORY_CONFIG);
}

/*
 * Function: FTPTelnetCheckFTPCmdOptions(FTP_SERVER_PROTO_CONF *serverConf)
 *
 * Purpose: This checks that the FTP configuration provided has
 *          options for CMDs that make sense:
 *          -- check if max_len == 0 & there is a cmd_validity
 *
 * Arguments: serverConf    => pointer to Server Configuration
 *
 * Returns: 0               => no errors
 *          1               => errors
 *
 */
int FTPTelnetCheckFTPCmdOptions(FTP_SERVER_PROTO_CONF *serverConf)
{
    FTP_CMD_CONF *cmdConf;
    int iRet =0;
    int config_error = 0;

    cmdConf = ftp_cmd_lookup_first(serverConf->cmd_lookup, &iRet);
    while (cmdConf && (iRet == FTPP_SUCCESS))
    {
        size_t len = strlen(cmdConf->cmd_name);
        if ( len > serverConf->max_cmd_len ) serverConf->max_cmd_len = len;

        if (cmdConf->check_validity && (cmdConf->max_param_len == 0))
        {
            _dpd.errMsg("FTPConfigCheck() configuration for server '%s', "
                    "command '%s' has max length of 0 and parameters to validate\n",
                    serverConf->serverAddr, cmdConf->cmd_name);
            config_error = 1;
        }
        cmdConf = ftp_cmd_lookup_next(serverConf->cmd_lookup, &iRet);
    }

    return config_error;
}

/*
 * Function: FTPTelnetCheckFTPServerConfigs(void)
 *
 * Purpose: This checks that the FTP server configurations are reasonable
 *
 * Arguments: None
 *
 * Returns: -1 on error
 *
 */
int FTPTelnetCheckFTPServerConfigs(struct _SnortConfig *sc, FTPTELNET_GLOBAL_CONF *config)
{
    FTP_SERVER_PROTO_CONF *serverConf;
    int iRet = 0;
    int rval;

    if (config == NULL)
        return 0;

    if ((rval = ftpp_ui_server_iterate(sc, config->server_lookup, _checkServerConfig, &iRet)))
        return rval;

    serverConf = config->default_ftp_server;
    if (FTPTelnetCheckFTPCmdOptions(serverConf))
    {
        _dpd.errMsg("FTPConfigCheck(): invalid configuration for FTP commands\n");
        return -1;
    }
    return 0;
}

static int _checkServerConfig(struct _SnortConfig *sc, void *pData)
{
    FTP_SERVER_PROTO_CONF *serverConf = (FTP_SERVER_PROTO_CONF *)pData;

    if (FTPTelnetCheckFTPCmdOptions(serverConf))
    {
        _dpd.errMsg("FTPConfigCheck(): invalid configuration for FTP commands\n");
        return -1;
    }
    return 0;
}

/*
 * Function: FTPConfigCheck(void)
 *
 * Purpose: This checks that the FTP configuration provided includes
 *          the default configurations for Server & Client.
 *
 * Arguments: None
 *
 * Returns: None
 *
 */
int FTPTelnetCheckConfigs(struct _SnortConfig *sc, void* pData, tSfPolicyId policyId)
{
    int rval;
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)pData;

    if ( pPolicyConfig == NULL )
        return 0;

    if ((pPolicyConfig->default_ftp_server == NULL) ||
            (pPolicyConfig->default_ftp_client == NULL))
    {
        _dpd.errMsg("FTP/Telnet configuration requires "
                "default client and default server configurations.\n");
        return -1;
    }
    if ( pPolicyConfig->telnet_config == NULL )
    {
        ProcessTelnetConf(pPolicyConfig,"",0);
    }

    if ((pPolicyConfig->telnet_config->ayt_threshold > 0) &&
            !pPolicyConfig->telnet_config->normalize)
    {
        _dpd.errMsg("WARNING: Telnet Configuration Check: using an "
                "AreYouThere threshold requires telnet normalization to be "
                "turned on.\n");
    }
    if ((pPolicyConfig->encrypted.alert != 0) &&
            !pPolicyConfig->telnet_config->normalize)
    {
        _dpd.errMsg("WARNING: Telnet Configuration Check: checking for "
                "encrypted traffic requires telnet normalization to be turned "
                "on.\n");
    }
    /* So we don't have to check it every time we use it */
    if ((!_dpd.streamAPI) || (_dpd.streamAPI->version < STREAM_API_VERSION5))
    {
        _dpd.errMsg("FTPConfigCheck() Streaming & reassembly must be "
                "enabled\n");
        return -1;
    }

    _dpd.setParserPolicy(sc, policyId);

    /* Add FTPTelnet into the preprocessor list */
#ifdef TARGET_BASED
    if ( _dpd.fileAPI->get_max_file_depth(sc, true) >= 0 )
    {
        _dpd.addPreproc(sc, FTPDataTelnetChecks, PRIORITY_APPLICATION, PP_FTPTELNET, PROTO_BIT__TCP);
        s_ftpdata_eof_cb_id = _dpd.streamAPI->register_event_handler(SnortFTPData_EOF);
        s_ftpdata_flush_cb_id = _dpd.streamAPI->register_ftp_flush_cb(SnortFTPData_Flush);
    }
    else
#endif
    {
        _dpd.addPreproc(sc, FTPTelnetChecks, PRIORITY_APPLICATION, PP_FTPTELNET, PROTO_BIT__TCP);
    }

    if ((rval = FTPTelnetCheckFTPServerConfigs(sc, pPolicyConfig)))
        return rval;

    _FTPTelnetAddPortsOfInterest(sc, pPolicyConfig, policyId);
#ifdef TARGET_BASED
    _FTPTelnetAddService(sc, ftp_app_id, policyId);
#endif

    return 0;

}

static int FTPConfigCheckPolicy(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    return FTPTelnetCheckConfigs(sc, pData, policyId);
}

int FTPConfigCheck(struct _SnortConfig *sc)
{
    int rval;

    if (ftp_telnet_config == NULL)
        return 0;

    if ((rval = sfPolicyUserDataIterate (sc, ftp_telnet_config, FTPConfigCheckPolicy)))
        return rval;

    return 0;
}

/*
 * Function: LogFTPPEvents(FTPP_GEN_EVENTS *GenEvents,
 *                         int iGenerator)
 *
 * Purpose: This is the routine that logs FTP/Telnet Preprocessor (FTPP)
 *          alerts through Snort.
 *
 *          Every Session gets looked at for any logged events, and if
 *          there are events to be logged then we select the one with the
 *          highest priority.
 *
 *          We use a generic event structure that we set for each different
 *          event structure.  This way we can use the same code for event
 *          logging regardless of what type of event strucure we are dealing
 *          with.
 *
 *          The important things to know about this function is how to work
 *          with the event queue.  The number of unique events is contained
 *          in the stack_count variable.  So we loop through all the unique
 *          events and find which one has the highest priority.  During this
 *          loop, we also re-initialize the individual event counts for the
 *          next iteration, saving us time in a separate initialization phase.
 *
 *          After we've iterated through all the events and found the one
 *          with the highest priority, we then log that event through snort.
 *
 *          We've mapped the FTPTelnet and the Snort alert IDs together, so
 *          we can access them directly instead of having a more complex
 *          mapping function.
 *
 * Arguments: GenEvents     => pointer a list of events
 *            iGenerator    => Generator ID (Telnet or FTP)
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static inline int LogFTPPEvents(FTPP_GEN_EVENTS *GenEvents,
        int iGenerator)
{
    FTPP_EVENT      *OrigEvent;
    FTPP_EVENT      *HiEvent = NULL;
    int           iStackCnt;
    int           iEvent;
    int           iCtr;

    /*
     * Now starts the generic event processing
     */
    iStackCnt = GenEvents->stack_count;

    /*
     * IMPORTANT::
     * We have to check the stack count of the event queue before we process
     * an log.
     */
    if(iStackCnt == 0)
    {
        return FTPP_SUCCESS;
    }

    /*
     * Cycle through the events and select the event with the highest
     * priority.
     */
    for(iCtr = 0; iCtr < iStackCnt; iCtr++)
    {
        iEvent = GenEvents->stack[iCtr];
        OrigEvent = &(GenEvents->events[iEvent]);

        /*
         * Set the event to start off the comparison
         */
        if(!HiEvent)
        {
            HiEvent = OrigEvent;
        }

        /*
         * This is our "comparison function".  Log the event with the highest
         * priority.
         */
        if(OrigEvent->event_info->priority < HiEvent->event_info->priority)
        {
            HiEvent = OrigEvent;
        }

        /*
         * IMPORTANT:
         *   This is how we reset the events in the event queue.
         *   If you miss this step, you can be really screwed.
         */
        OrigEvent->count = 0;
    }

    if (!HiEvent)
        return FTPP_SUCCESS;

    /*
     * We use the iEvent+1 because the event IDs between snort and
     * FTPTelnet are mapped off-by-one.  They're mapped off-by one
     * because in the internal FTPTelnet queue, events are mapped
     * starting at 0.  For some reason, it appears that the first
     * event can't be zero, so we use the internal value and add
     * one for snort.
     */
    iEvent = HiEvent->event_info->alert_id + 1;

    /* GenID, SID, Rev, Classification, Pri, Msg, RuleInfo  */
    _dpd.alertAdd(iGenerator,
            HiEvent->event_info->alert_sid, 1, /* Revision 1  */
            HiEvent->event_info->classification,
            HiEvent->event_info->priority,
            HiEvent->event_info->alert_str, NULL); /* No Rule info  */

    /*
     * Reset the event queue stack counter, in the case of pipelined
     * requests.
     */
    GenEvents->stack_count = 0;

    return FTPP_SUCCESS;
}

/*
 * Function: LogFTPEvents(FTP_SESSION *FtpSession)
 *
 * Purpose: This is the routine that logs FTP alerts through Snort.
 *          It maps the event into a generic event and calls
 *          LOGFTPPEvents().
 *
 * Arguments: FtpSession    => pointer the session structure
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static inline int LogFTPEvents(FTP_SESSION *FtpSession)
{
    FTPP_GEN_EVENTS GenEvents;
    int             iGenerator;
    int             iRet;

    GenEvents.stack =       FtpSession->event_list.stack;
    GenEvents.stack_count = FtpSession->event_list.stack_count;
    GenEvents.events =      FtpSession->event_list.events;
    iGenerator = GENERATOR_SPP_FTPP_FTP;

    iRet = LogFTPPEvents(&GenEvents, iGenerator);

    /* Reset the count... */
    FtpSession->event_list.stack_count = 0;

    return iRet;
}

/*
 * Function: LogTelnetEvents(TELNET_SESSION *TelnetSession)
 *
 * Purpose: This is the routine that logs Telnet alerts through Snort.
 *          It maps the event into a generic event and calls
 *          LOGFTPPEvents().
 *
 * Arguments: TelnetSession    => pointer the session structure
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static inline int LogTelnetEvents(TELNET_SESSION *TelnetSession)
{
    FTPP_GEN_EVENTS GenEvents;
    int             iGenerator;
    int             iRet;
    GenEvents.stack =       TelnetSession->event_list.stack;
    GenEvents.stack_count = TelnetSession->event_list.stack_count;
    GenEvents.events =      TelnetSession->event_list.events;
    iGenerator = GENERATOR_SPP_FTPP_TELNET;

    iRet = LogFTPPEvents(&GenEvents, iGenerator);

    /* Reset the count... */
    TelnetSession->event_list.stack_count = 0;

    return iRet;
}

/*
 * Function: SetSiInput(FTPP_SI_INPUT *SiInput, Packet *p)
 *
 * Purpose: This is the routine sets the source and destination IP
 *          address and port pairs so as to determine the direction
 *          of the FTP or telnet connection.
 *
 * Arguments: SiInput       => pointer the session input structure
 *            p             => pointer to the packet structure
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
static inline int SetSiInput(FTPP_SI_INPUT *SiInput, SFSnortPacket *p)
{
    IP_COPY_VALUE(SiInput->sip, GET_SRC_IP(p));
    IP_COPY_VALUE(SiInput->dip, GET_DST_IP(p));
    SiInput->sport = p->src_port;
    SiInput->dport = p->dst_port;

    /*
     * We now set the packet direction
     */
    if(p->stream_session &&
            _dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
    {
        SiInput->pdir = FTPP_SI_NO_MODE;
    }
    else if(p->flags & FLAG_FROM_SERVER)
    {
        SiInput->pdir = FTPP_SI_SERVER_MODE;
    }
    else if(p->flags & FLAG_FROM_CLIENT)
    {
        SiInput->pdir = FTPP_SI_CLIENT_MODE;
    }
    else
    {
        SiInput->pdir = FTPP_SI_NO_MODE;
    }

    return FTPP_SUCCESS;

}

/*
 * Function: do_detection(Packet *p)
 *
 * Purpose: This is the routine that directly performs the rules checking
 *          for each of the FTP & telnet preprocessing modules.
 *
 * Arguments: p             => pointer to the packet structure
 *
 * Returns: None
 *
 */
void do_detection(SFSnortPacket *p)
{
    //extern int     do_detect;
    //extern OptTreeNode *otn_tmp;
    PROFILE_VARS;

    /*
     * If we get here we either had a client or server request/response.
     * We do the detection here, because we're starting a new paradigm
     * about protocol decoders.
     *
     * Protocol decoders are now their own detection engine, since we are
     * going to be moving protocol field detection from the generic
     * detection engine into the protocol module.  This idea scales much
     * better than having all these Packet struct field checks in the
     * main detection engine for each protocol field.
     */
    PREPROC_PROFILE_START(ftppDetectPerfStats);
    _dpd.detect(p);

    _dpd.disableAllDetect(p);
    PREPROC_PROFILE_END(ftppDetectPerfStats);
#ifdef PERF_PROFILING
    ftppDetectCalled = 1;
#endif
    //otn_tmp = NULL;

    /*
     * We set the global detection flag here so that if request pipelines
     * fail, we don't do any detection.
     */
    //do_detect = 0;
}

/*
 * Function: SnortTelnet(FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                       Packet *p,
 *                       int iInspectMode)
 *
 * Purpose: This is the routine that handles the protocol layer checks
 *          for telnet.
 *
 * Arguments: GlobalConf    => pointer the global configuration
 *            p             => pointer to the packet structure
 *            iInspectMode  => indicator whether this is a client or server
 *                             packet.
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int SnortTelnet(FTPTELNET_GLOBAL_CONF *GlobalConf, TELNET_SESSION *TelnetSession,
        SFSnortPacket *p, int iInspectMode)
{
    int iRet;
    PROFILE_VARS;

#ifdef DUMP_BUFFER
    dumpBuffer(TELNET_DUMP,(const char *)p->payload,p->payload_size);
#endif

    if (!TelnetSession)
    {
        if (GlobalConf->inspection_type == FTPP_UI_CONFIG_STATEFUL)
        {
            return FTPP_NONFATAL_ERR;
        }
        else
        {
            return FTPP_INVALID_SESSION;
        }
    }

    if (TelnetSession->encr_state && !GlobalConf->check_encrypted_data)
    {
        return FTPP_SUCCESS;
    }

    PREPROC_PROFILE_START(telnetPerfStats);

    if (!GlobalConf->telnet_config->normalize)
    {
        do_detection(p);
    }
    else
    {
        iRet = normalize_telnet(GlobalConf, TelnetSession, p,
                iInspectMode, FTPP_APPLY_TNC_ERASE_CMDS);
        if ((iRet == FTPP_SUCCESS) || (iRet == FTPP_NORMALIZED))
        {
            do_detection(p);
        }
        LogTelnetEvents(TelnetSession);
    }
    PREPROC_PROFILE_END(telnetPerfStats);
#ifdef PERF_PROFILING
    if (ftppDetectCalled)
    {
        telnetPerfStats.ticks -= ftppDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        ftppDetectPerfStats.ticks = 0;
        ftppDetectCalled = 0;
    }
#endif

    return FTPP_SUCCESS;
}

/*
 * Function: SnortFTP(FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                       Packet *p,
 *                       int iInspectMode)
 *
 * Purpose: This is the routine that handles the protocol layer checks
 *          for FTP.
 *
 * Arguments: GlobalConf    => pointer the global configuration
 *            p             => pointer to the packet structure
 *            iInspectMode  => indicator whether this is a client or server
 *                             packet.
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int SnortFTP(FTPTELNET_GLOBAL_CONF *GlobalConf, FTP_SESSION *FTPSession,
        SFSnortPacket *p, int iInspectMode)
{
    int iRet;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();
    PROFILE_VARS;

    if (!FTPSession ||
            FTPSession->server_conf == NULL ||
            FTPSession->client_conf == NULL)
    {
        return FTPP_INVALID_SESSION;
    }

    if(((FTPSession->encr_state == AUTH_TLS_ENCRYPTED) ||
                (FTPSession->encr_state == AUTH_SSL_ENCRYPTED) ||
                (FTPSession->encr_state == AUTH_UNKNOWN_ENCRYPTED)) )
    {
        if ((iInspectMode == FTPP_SI_CLIENT_MODE) && FTPSession->encr_state_chello )
        {
            if (IsTlsClientHello(p->payload, p->payload + p->payload_size))
            {
                FTPSession->encr_state_chello = false;
                if(ssl_cb)
                    ssl_cb->session_initialize(p, FTPSession, FTP_Set_flow_id);
            }

        }

        if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
            FTPSession->encr_state = 0;
        else if (!GlobalConf->check_encrypted_data)
            return FTPP_SUCCESS;
    }

    PREPROC_PROFILE_START(ftpPerfStats);

    if (iInspectMode == FTPP_SI_SERVER_MODE)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET,
                    "Server packet: %.*s\n", p->payload_size, p->payload));

        // FIXTHIS breaks target-based non-standard ports
        //if ( !_dpd.isPafEnabled() )
        /* Force flush of client side of stream  */
        _dpd.streamAPI->response_flush_stream(p);
    }
    else
    {
        if ( !_dpd.readyForProcess(p) )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET,
                        "Client packet will be reassembled\n"));
            PREPROC_PROFILE_END(ftpPerfStats);
            return FTPP_SUCCESS;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET,
                        "Client packet: rebuilt %s: %.*s\n",
                        (p->flags & FLAG_REBUILT_STREAM) ? "yes" : "no",
                        p->payload_size, p->payload));
        }
    }

    iRet = initialize_ftp(FTPSession, p, iInspectMode);
    if (iRet)
    {
        LogFTPEvents(FTPSession);

        PREPROC_PROFILE_END(ftpPerfStats);
        return iRet;
    }

    iRet = check_ftp(FTPSession, p, iInspectMode);
    if (iRet == FTPP_SUCCESS)
    {
        /* Ideally, Detect(), called from do_detection, will look at
         * the cmd & param buffers, or the rsp & msg buffers.  Current
         * architecture does not support this...
         * So, we call do_detection() here.  Otherwise, we'd call it
         * from inside check_ftp -- each time we process a pipelined
         * FTP command.
         */
        do_detection(p);
    }

    LogFTPEvents(FTPSession);

    PREPROC_PROFILE_END(ftpPerfStats);
#ifdef PERF_PROFILING
    if (ftppDetectCalled)
    {
        ftpPerfStats.ticks -= ftppDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        ftppDetectPerfStats.ticks = 0;
        ftppDetectCalled = 0;
    }
#endif

    return iRet;
}

/*
 * Funtcion: SnortFTPTelnet
 *
 * Purpose: This function calls the FTPTelnet function that handles
 *          the protocol layer checks for an FTP or Telnet session,
 *          after determining which, if either, protocol applies.
 *
 * Arguments: GlobalConf    => pointer the global configuration
 *            p             => pointer to the packet structure
 *
 * Returns: int     => an error code integer (0 = success,
 *                     >0 = non-fatal error, <0 = fatal error)
 *
 */
int SnortFTPTelnet(SFSnortPacket *p)
{
    FTPP_SI_INPUT SiInput;
    int iInspectMode = FTPP_SI_NO_MODE;
    FTP_TELNET_SESSION *ft_ssn = NULL;
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    FTPTELNET_GLOBAL_CONF *GlobalConf = NULL;

#ifdef DUMP_BUFFER
    dumpBufferInit();
#endif

    sfPolicyUserPolicySet (ftp_telnet_config, policy_id);
    GlobalConf = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(ftp_telnet_config);

    /*
     * Set up the FTPP_SI_INPUT pointer.  This is what the session_inspection()
     * routines use to determine client and server traffic.  Plus, this makes
     * the FTPTelnet library very independent from snort.
     */
    SetSiInput(&SiInput, p);

    if (p->stream_session)
    {
        ft_ssn = (FTP_TELNET_SESSION *)
            _dpd.sessionAPI->get_application_data(p->stream_session, PP_FTPTELNET);

        if (ft_ssn != NULL)
        {
            SiInput.pproto = ft_ssn->proto;

            if (ft_ssn->proto == FTPP_SI_PROTO_TELNET)
            {
                TELNET_SESSION *telnet_ssn = (TELNET_SESSION *)ft_ssn;

                GlobalConf = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGet(telnet_ssn->global_conf, telnet_ssn->policy_id);

                if (SiInput.pdir != FTPP_SI_NO_MODE)
                {
                    iInspectMode = SiInput.pdir;
                }
                else
                {
                    if ((telnet_ssn->telnet_conf != NULL) &&
                            (telnet_ssn->telnet_conf->proto_ports.ports[SiInput.sport]))
                    {
                        iInspectMode = FTPP_SI_SERVER_MODE;
                    }
                    else if ((telnet_ssn->telnet_conf != NULL) &&
                            (telnet_ssn->telnet_conf->proto_ports.ports[SiInput.dport]))
                    {
                        iInspectMode = FTPP_SI_CLIENT_MODE;
                    }
                }
            }
            else if (ft_ssn->proto == FTPP_SI_PROTO_FTP)
            {
                FTP_SESSION *ftp_ssn = (FTP_SESSION *)ft_ssn;

                GlobalConf = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGet(ftp_ssn->global_conf, ftp_ssn->policy_id);

                if (SiInput.pdir != FTPP_SI_NO_MODE)
                {
                    iInspectMode = SiInput.pdir;
                }
                else
                {
                    if ((ftp_ssn->server_conf != NULL) &&
                            ftp_ssn->server_conf->proto_ports.ports[SiInput.sport])
                    {
                        iInspectMode = FTPP_SI_SERVER_MODE;
                    }
                    else if ((ftp_ssn->server_conf != NULL) &&
                            ftp_ssn->server_conf->proto_ports.ports[SiInput.dport])
                    {
                        iInspectMode = FTPP_SI_CLIENT_MODE;
                    }
                    else
                    {
                        iInspectMode = FTPGetPacketDir(p);
                    }
                }
            }
            else
            {
                /* XXX - Not FTP or Telnet */
                _dpd.sessionAPI->set_application_data(p->stream_session, PP_FTPTELNET, NULL, NULL);
                return 0;
            }
        }
    }

    if (GlobalConf == NULL)
        return 0;

    /*
     * FTPTelnet PACKET FLOW::
     *
     * Determine Proto Module::
     *   The Session Inspection Module retrieves the appropriate
     *   configuration for sessions, and takes care of the stateless
     *   vs. stateful processing in order to do this.  Once this module
     *   does it's magic, we're ready for the primetime.  This means
     *   determining whether this is an FTP or a Telnet session.
     *
     * Proto Specific Module::
     *   This is where we normalize the data.  The Protocol specific module
     *   handles what type of normalization to do (telnet, ftp) and does
     *   protocol related checks.
     *
     */
    if (ft_ssn == NULL)
    {
        int iRet = ftpp_si_determine_proto(p, GlobalConf, &ft_ssn, &SiInput, &iInspectMode);
        if (iRet)
            return iRet;
    }

    if (ft_ssn != NULL)
    {
        switch (SiInput.pproto)
        {
            case FTPP_SI_PROTO_TELNET:
                return SnortTelnet(GlobalConf, (TELNET_SESSION *)ft_ssn, p, iInspectMode);
                break;
            case FTPP_SI_PROTO_FTP:
                return SnortFTP(GlobalConf, (FTP_SESSION *)ft_ssn, p, iInspectMode);
                break;
        }
    }

    /* Uh, shouldn't get here  */
    return FTPP_INVALID_PROTO;
}

#ifdef TARGET_BASED
static void FTPDataProcess(SFSnortPacket *p, FTP_DATA_SESSION *data_ssn,
        uint8_t *file_data, uint16_t data_length)
{
    int status;

    _dpd.setFileDataPtr((uint8_t *)p->payload, (uint16_t)p->payload_size);

    if(data_ssn->flags & FTPDATA_FLG_FLUSH)
    {
        _dpd.fileAPI->set_file_partial(p,data_ssn->position,data_ssn->direction,true);
    }
    if(data_ssn->flags & FTPDATA_FLG_STOP)
    {
        _dpd.fileAPI->set_file_partial(p,data_ssn->position,data_ssn->direction,false);
    }
    status = _dpd.fileAPI->file_process(p, (uint8_t *)file_data,
            data_length, data_ssn->position, data_ssn->direction, false, (data_ssn->flags & FTPDATA_FLG_FLUSH) ? true : false);

    FTP_SESSION *ft_ssn = (FTP_SESSION *) _dpd.sessionAPI->get_application_data_from_key(data_ssn->ftp_key, PP_FTPTELNET);
    if(ft_ssn && (ft_ssn->flags & FTP_FLG_MALWARE_ENABLED) && _dpd.active_PacketWasDropped())
    {
        _dpd.fileAPI->file_resume_block_add_file(p, data_ssn->path_hash, 0, 0, 0, NULL, ft_ssn->control_clientPort,
                ft_ssn->control_serverPort, true , data_ssn->direction);
    }

    /* Filename needs to be set AFTER the first call to file_process( ) */
    if (data_ssn->filename && !(data_ssn->flags & FTPDATA_FLG_FILENAME_SET))
    {
        _dpd.fileAPI->set_file_name(p->stream_session,
                (uint8_t *)data_ssn->filename, data_ssn->file_xfer_info, false);
        data_ssn->flags |= FTPDATA_FLG_FILENAME_SET;
    }

    /* Ignore the rest of this transfer if file processing is complete
     * and preprocessor was configured to ignore ftp-data sessions. */
    if (!status && data_ssn->data_chan)
    {
        _dpd.sessionAPI->set_ignore_direction(
                p->stream_session, SSN_DIR_BOTH);
    }
}

void SnortFTPData_EOF(SFSnortPacket *p)
{
    FTP_SESSION *ftp_ssn;
    FTP_DATA_SESSION *data_ssn = (FTP_DATA_SESSION *)
                _dpd.sessionAPI->get_application_data(p->stream_session, PP_FTPTELNET);

    if (!PROTO_IS_FTP_DATA(data_ssn) || !FTPDataDirection(p, data_ssn))
        return;

    ftp_ssn = (FTP_SESSION *) _dpd.sessionAPI->get_application_data_from_key(data_ssn->ftp_key, PP_FTPTELNET);
    initFilePosition(&data_ssn->position, _dpd.fileAPI->get_file_processed_size(p->stream_session));
    finalFilePosition(&data_ssn->position);

    _dpd.streamAPI->request_flush_stream(p);
    if (ftp_ssn && (data_ssn->flags & FTPDATA_FLG_REST) && (ftp_ssn->rest_cmd_offset > 0))
    {
        File_Verdict verdict;
        verdict = _dpd.fileAPI->file_resume_block_check(p, data_ssn->path_hash);
        data_ssn->flags &= ~FTPDATA_FLG_REST;
        ftp_ssn->rest_cmd_offset = 0;
        if((verdict == FILE_VERDICT_BLOCK) || (verdict == FILE_VERDICT_REJECT))
        {
            _dpd.fileAPI->file_resume_block_add_file(p, data_ssn->path_hash, 0, 0, 0, NULL, ftp_ssn->control_clientPort,
                    ftp_ssn->control_serverPort, true, data_ssn->direction);
        }
        return;
    }

    if (!(data_ssn->flags & FTPDATA_FLG_STOP))
    {
        data_ssn->flags |= FTPDATA_FLG_STOP;
        FTPDataProcess(p, (FTP_DATA_SESSION *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_FTPTELNET),
                (uint8_t *)p->payload,
                (uint16_t)p->payload_size);
    }
}

void SnortFTPData_Flush(SFSnortPacket *p)
{
    FTP_DATA_SESSION *data_ssn = (FTP_DATA_SESSION *)
                _dpd.sessionAPI->get_application_data(p->stream_session, PP_FTPTELNET);

    if (!PROTO_IS_FTP_DATA(data_ssn) || !FTPDataDirection(p, data_ssn))
        return;

    initFilePosition(&data_ssn->position, _dpd.fileAPI->get_file_processed_size(p->stream_session));
    data_ssn->flags |= FTPDATA_FLG_FLUSH;

    _dpd.streamAPI->request_flush_stream(p);

    data_ssn->flags &= ~FTPDATA_FLG_FLUSH;
    return;
}

int SnortFTPData(SFSnortPacket *p)
{
    FTP_SESSION *ftp_ssn;
    FTP_DATA_SESSION *data_ssn;

    if (!p->stream_session)
        return -1;

    data_ssn = (FTP_DATA_SESSION *)
        _dpd.sessionAPI->get_application_data(p->stream_session, PP_FTPTELNET);

    if (!PROTO_IS_FTP_DATA(data_ssn))
        return -2;

    if (data_ssn->flags & FTPDATA_FLG_STOP)
        return 0;

    /* FTP-Data Session is in limbo, we need to lookup the control session
     * to figure out what to do. */
     ftp_ssn = (FTP_SESSION *) _dpd.sessionAPI->get_application_data_from_key(data_ssn->ftp_key, PP_FTPTELNET);
     if(!ftp_ssn)
         return -3;

#ifdef DAQ_PKT_FLAG_SSL_DETECTED
    //  Set up the flow context when an SSL client hello is detected and
    //  ignore packets until the flow is decrypted.
    if(p->pkt_header->flags & DAQ_PKT_FLAG_SSL_DETECTED)
    {
        ssl_callback_interface_t *ssl_cb;
        ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();
        if(ssl_cb)
        {
            ftp_ssn->data_chan_state |= DATA_CHAN_CLIENT_HELLO_SEEN;
            ssl_cb->session_initialize(p, data_ssn, FTPData_Set_flow_id);
        }
        return 0;  //  Ignore SSL client hello.
    }
    else if(ftp_ssn->data_chan_state & DATA_CHAN_CLIENT_HELLO_SEEN)
    {
        if( _dpd.streamAPI->is_session_decrypted(p->stream_session))
        {
            //  Done handling SSL handshake.
            ftp_ssn->data_chan_state &= ~DATA_CHAN_CLIENT_HELLO_SEEN;
        }
        else
        {
            return 0;   //  Ignore packet.
        }
    }
#endif
    if ((data_ssn->flags & FTPDATA_FLG_REST) && (ftp_ssn->rest_cmd_offset > 0))
    {
        File_Verdict verdict;
        verdict = _dpd.fileAPI->file_resume_block_check(p, data_ssn->path_hash);
        data_ssn->flags &= ~FTPDATA_FLG_REST;
        ftp_ssn->rest_cmd_offset = 0;
        if((verdict == FILE_VERDICT_BLOCK) || (verdict == FILE_VERDICT_REJECT))
        {
            data_ssn->flags |= FTPDATA_FLG_STOP;
            _dpd.fileAPI->file_resume_block_add_file(p, data_ssn->path_hash, 0, 0, 0, NULL, ftp_ssn->control_clientPort,
                                        ftp_ssn->control_serverPort, true, data_ssn->direction);
        }
        return 0 ;
    }
    //  bail if we have not rebuilt the stream yet.
    if (!_dpd.readyForProcess(p))
        return 0;

    if (data_ssn->file_xfer_info == FTPP_FILE_UNKNOWN)
    {
        if (!PROTO_IS_FTP(ftp_ssn))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET,
                        "FTP-DATA Invalid FTP_SESSION retrieved durring lookup\n"););

            if (data_ssn->data_chan)
                _dpd.sessionAPI->set_ignore_direction(p->stream_session, SSN_DIR_BOTH);

            return -2;
        }

        switch (ftp_ssn->file_xfer_info)
        {
            case FTPP_FILE_UNKNOWN:
                /* Keep waiting */
                break;

            case FTPP_FILE_IGNORE:
                /* This wasn't a file transfer; ignore it */
                if (data_ssn->data_chan)
                    _dpd.sessionAPI->set_ignore_direction(p->stream_session, SSN_DIR_BOTH);
                return 0;

            default:
                /* A file transfer was detected. */
                data_ssn->direction = ftp_ssn->data_xfer_dir;
                data_ssn->file_xfer_info = ftp_ssn->file_xfer_info;
                ftp_ssn->file_xfer_info  = 0;
                data_ssn->filename  = ftp_ssn->filename;
                ftp_ssn->filename   = NULL;
                break;
        }
    }

    if (!FTPDataDirection(p, data_ssn))
        return 0;

    if (isFileEnd(data_ssn->position))
    {
        data_ssn->flags |= FTPDATA_FLG_STOP;
    }
    else
    {
        initFilePosition(&data_ssn->position,
                _dpd.fileAPI->get_file_processed_size(p->stream_session));
        if (p->tcp_header && (p->tcp_header->flags & TCPHEADER_FIN))
            finalFilePosition(&data_ssn->position);
    }

    FTPDataProcess(p, data_ssn, (uint8_t *)p->payload, (uint16_t)p->payload_size);
    return 0;
}
#endif /* TARGET_BASED */

int FTPPBounceInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr)
{
    char **toks;
    int num_toks;

    toks = _dpd.tokenSplit(parameters, ",", 12, &num_toks, 0);

    if(num_toks > 0)
    {
        DynamicPreprocessorFatalMessage("ERROR: Bad arguments to '%s' option: '%s'\n",
                name, parameters);
    }

    _dpd.tokenFree(&toks, num_toks);

    *dataPtr = NULL;

    return 1;
}


/****************************************************************************
 *
 * Function: FTPPBounce(void *pkt, uint8_t **cursor, void **dataPtr)
 *
 * Purpose: Use this function to perform the particular detection routine
 *          that this rule keyword is supposed to encompass.
 *
 * Arguments: p => pointer to the decoded packet
 *            cursor => pointer to the current location in the buffer
 *            dataPtr => pointer to rule specific data (not used for this option)
 *
 * Returns: If the detection test fails, this function *must* return a zero!
 *          On success, it returns 1;
 *
 ****************************************************************************/
int FTPPBounceEval(void *pkt, const uint8_t **cursor, void *dataPtr)
{
    uint32_t ip = 0;
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    int octet=0;
    const char *start_ptr, *end_ptr;
    const char *this_param = *(const char **)cursor;

    int dsize;

    if ( !p->ip4_header )
        return 0;

    if(_dpd.Is_DetectFlag(SF_FLAG_ALT_DETECT))
    {
        dsize = _dpd.altDetect->len;
        start_ptr = (const char*) _dpd.altDetect->data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Detect buffer!\n"););
    }
    else if(_dpd.Is_DetectFlag(SF_FLAG_ALT_DECODE))
    {
        dsize = _dpd.altBuffer->len;
        start_ptr = (const char *) _dpd.altBuffer->data;
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Decode buffer!\n"););

    }
    else
    {
        start_ptr = (const char *)p->payload;
        dsize = p->payload_size;
    }

    DEBUG_WRAP(
            DebugMessage(DEBUG_PATTERN_MATCH,"[*] ftpbounce firing...\n");
            DebugMessage(DEBUG_PATTERN_MATCH,"payload starts at %p\n", start_ptr);
            );  /* END DEBUG_WRAP */

    /* save off whatever our ending pointer is */
    end_ptr = start_ptr + dsize;

    while ((this_param < end_ptr) && isspace((int)*this_param)) this_param++;

    do
    {
        int value = 0;

        do
        {
            if (!isdigit((int)*this_param))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                            "[*] ftpbounce non digit char failed..\n"););
                return RULE_NOMATCH;
            }

            value = value * 10 + (*this_param - '0');
            this_param++;

        } while ((this_param < end_ptr) &&
                (*this_param != ',') &&
                (!(isspace((int)*this_param))));

        if (value > 0xFF)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                        "[*] ftpbounce value > 256 ..\n"););
            return RULE_NOMATCH;
        }

        if (octet  < 4)
        {
            ip = (ip << 8) + value;
        }

        if ((this_param < end_ptr) && !isspace((int)*this_param))
            this_param++;

        octet++;

    } while ((this_param < end_ptr) && !isspace((int)*this_param) && (octet < 4));

    if (octet < 4)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "[*] ftpbounce insufficient data ..\n"););
        return RULE_NOMATCH;
    }

    if (ip != ntohl(p->ip4_header->source.s_addr))
    {
        /* Bounce attempt -- IPs not equal */
        return RULE_MATCH;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "PORT command not being used in bounce\n"););
        return RULE_NOMATCH;
    }

    /* Never reached */
    return RULE_NOMATCH;
}

/* Add ports configured for ftptelnet preprocessor to stream5 port filtering so that
 * if any_any rules are being ignored then the packet still reaches ftptelnet.
 *
 * For ports in global_server configuration, server_lookup and server_lookupIpv6,
 * add the port to stream5 port filter list.
 */
static void _FTPTelnetAddPortsOfInterest(struct _SnortConfig *sc, FTPTELNET_GLOBAL_CONF *config, tSfPolicyId policy_id)
{
    int i;

    if (config == NULL)
        return;

    /* For the server callback */
    ftp_current_policy = policy_id;

    _addPortsToStream(sc, config->telnet_config->proto_ports.ports, policy_id, 0);
    _addPortsToStream(sc, config->default_ftp_server->proto_ports.ports, policy_id, 1);
    ftpp_ui_server_iterate(sc, config->server_lookup,
            _addFtpServerConfPortsToStream, &i);
}

static int _addFtpServerConfPortsToStream(struct _SnortConfig *sc, void *pData)
{
    FTP_SERVER_PROTO_CONF *pConf = (FTP_SERVER_PROTO_CONF *)pData;
    _addPortsToStream(sc, pConf->proto_ports.ports, ftp_current_policy, 1);
    return 0;
}

// flush at last line feed in payload
// preproc will deal with any pipelined commands
static PAF_Status ftp_paf (
        void* ssn, void** pv, const uint8_t* data, uint32_t len,
        uint64_t *flags, uint32_t* fp, uint32_t* fp_eoh)
{
#ifdef HAVE_MEMRCHR
    uint8_t* lf =  memrchr(data, '\n', len);
#else
    uint32_t n = len;
    uint8_t* lf = NULL, * tmp = (uint8_t*) data;

    while ( (tmp = memchr(tmp, '\n', n)) )
    {
        lf = tmp++;
        n = len - (tmp - data);
    }
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s[%d] '%*.*s'\n", __FUNCTION__, len, len, len, data));

    if ( !lf )
        return PAF_SEARCH;

    *fp = lf - data + 1;
    return PAF_FLUSH;
}

#ifdef TARGET_BASED
static void _FTPTelnetAddService (struct _SnortConfig *sc, int16_t app, tSfPolicyId policy)
{
    if ( _dpd.isPafEnabled() )
    {
       ftp_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, true, ftp_paf, true);
       ftp_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, false, ftp_paf, true);
    }
}
#endif

static void _addPortsToStream(struct _SnortConfig *sc, char *ports, tSfPolicyId policy_id, int ftp)
{
    unsigned int i;

    for (i = 0; i < MAXPORTS; i++)
    {
        if (ports[i])
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_TCP, (uint16_t)i,
                    PORT_MONITOR_SESSION, policy_id, 1);

            if ( ftp && _dpd.isPafEnabled() )
            {
                ftp_paf_id = _dpd.streamAPI->register_paf_port(sc, policy_id, (uint16_t)i, true, ftp_paf, false);
                ftp_paf_id = _dpd.streamAPI->register_paf_port(sc, policy_id, (uint16_t)i, false, ftp_paf, false);
            }
        }
    }
}

int FtpTelnetInitGlobalConfig(FTPTELNET_GLOBAL_CONF *config,
        char *ErrorString, int iErrStrLen)
{
    int iRet;

    if (config == NULL)
    {
        snprintf(ErrorString, iErrStrLen, "Global configuration is NULL.");
        return FTPP_FATAL_ERR;
    }

    iRet = ftpp_ui_config_init_global_conf(config);
    if (iRet)
    {
        snprintf(ErrorString, iErrStrLen,
                "Error initializing Global Configuration.");

        return FTPP_FATAL_ERR;
    }

    return 0;
}


void FTP_Set_flow_id( void *app_data, uint32_t fid )
{
    FTP_SESSION *ssn = (FTP_SESSION *)app_data;
    if( ssn )
        ssn->flow_id = fid;
}

void FTPData_Set_flow_id( void *app_data, uint32_t fid )
{
#ifdef TARGET_BASED
    FTP_DATA_SESSION *ssn = (FTP_DATA_SESSION *)app_data;
    if( ssn )
        ssn->flow_id = fid;
#endif
}
