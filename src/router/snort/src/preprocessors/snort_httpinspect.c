/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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

/**
**  @file       snort_httpinspect.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      This file wraps the HttpInspect functionality for Snort
**              and starts the HttpInspect flow.
**
**
**  The file takes a Packet structure from the Snort IDS to start the
**  HttpInspect flow.  This also uses the Stream Interface Module which
**  is also Snort-centric.  Mainly, just a wrapper to HttpInspect
**  functionality, but a key part to starting the basic flow.
**
**  The main bulk of this file is taken up with user configuration and
**  parsing.  The reason this is so large is because HttpInspect takes
**  very detailed configuration parameters for each specified server.
**  Hopefully every web server that is out there can be emulated
**  with these configuration options.
**
**  The main functions of note are:
**    - HttpInspectSnortConf::this is the configuration portion
**    - SnortHttpInspect::this is the actual inspection flow
**    - LogEvents:this is where we log the HttpInspect events
**
**  NOTES:
**
**  - 2.11.03:  Initial Development.  DJR
**  - 2.4.05:   Added tab_uri_delimiter config option.  AJM.
*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort.h"
#include "detect.h"
#include "decode.h"
#include "log.h"
#include "event.h"
#include "generators.h"
#include "snort_debug.h"
#include "plugbase.h"
#include "util.h"
#include "event_queue.h"
#include "session_common.h"
#include "session_api.h"
#include "stream_api.h"
#include "sfsnprintfappend.h"

#include "hi_return_codes.h"
#include "hi_ui_config.h"
#include "hi_ui_iis_unicode_map.h"
#include "hi_si.h"
#include "hi_mi.h"
#include "hi_norm.h"
#include "hi_client.h"
#include "snort_httpinspect.h"
#include "detection_util.h"
#include "profiler.h"
#include "hi_cmd_lookup.h"
#include "Unified2_common.h"
#include "mempool.h"
#include "file_mail_common.h"
#include "file_api.h"
#include "sf_email_attach_decode.h"
#include "file_decomp.h"
#include "hi_eo_log.h"
#include "memory_stats.h"

#ifdef DUMP_BUFFER
#include "hi_buffer_dump.h"
#endif

#ifdef PERF_PROFILING
extern PreprocStats hiDetectPerfStats;
extern int hiDetectCalled;
#endif

extern char *snort_conf_dir;

extern MemPool *hi_gzip_mempool;

extern tSfPolicyUserContextId hi_config;

extern char** xffFields;

/* Stats tracking for HTTP Inspect */
HIStats hi_stats;

DataBuffer HttpDecodeBuf;

const HiSearchToken hi_patterns[] =
{
    {"<SCRIPT",         7,  HI_JAVASCRIPT},
    {NULL,              0, 0}
};

const HiSearchToken html_patterns[] =
{
    {"JAVASCRIPT",      10, HTML_JS},
    {"ECMASCRIPT",      10, HTML_EMA},
    {"VBSCRIPT",         8, HTML_VB},
    {NULL,               0, 0}
};

void *hi_javascript_search_mpse = NULL;
void *hi_htmltype_search_mpse = NULL;
HISearch hi_js_search[HI_LAST];
HISearch hi_html_search[HTML_LAST];
HISearch *hi_current_search = NULL;
HISearchInfo hi_search_info;


#define MAX_FILENAME    1000

/*
**  GLOBAL subkeywords.
*/
/**
**  Takes an integer arugment
*/
/**
**  Specifies whether to alert on anomalous
**  HTTP servers or not.
*/
#define ANOMALOUS_SERVERS "detect_anomalous_servers"
/**
**  Alert on general proxy use
*/
#define PROXY_ALERT "proxy_alert"
/**
**  Takes an inspection type argument
**  stateful or stateless
*/
#define DEFAULT       "default"

/*
**  SERVER subkeywords.
*/
#define PORTS             "ports"
#define FLOW_DEPTH        "flow_depth"
#define SERVER_FLOW_DEPTH "server_flow_depth"
#define CLIENT_FLOW_DEPTH "client_flow_depth"
#define POST_DEPTH        "post_depth"
#define IIS_UNICODE_MAP   "iis_unicode_map"
#define CHUNK_LENGTH      "chunk_length"
#define SMALL_CHUNK_LENGTH  "small_chunk_length"
#define MAX_HDR_LENGTH    "max_header_length"
#define PIPELINE          "no_pipeline_req"
#define ASCII             "ascii"
#define DOUBLE_DECODE     "double_decode"
#define U_ENCODE          "u_encode"
#define BARE_BYTE         "bare_byte"
/* Base 36 is deprecated and essentially a noop
 * Leave this here so as to print out a warning when the option is used */
#define BASE36            "base36"
#define UTF_8             "utf_8"
#define IIS_UNICODE       "iis_unicode"
#define NON_RFC_CHAR      "non_rfc_char"
#define MULTI_SLASH       "multi_slash"
#define IIS_BACKSLASH     "iis_backslash"
#define DIRECTORY         "directory"
#define APACHE_WS         "apache_whitespace"
#define IIS_DELIMITER     "iis_delimiter"
#define PROFILE_STRING    "profile"
#define NON_STRICT        "non_strict"
#define ALLOW_PROXY       "allow_proxy_use"
#define OVERSIZE_DIR      "oversize_dir_length"
#define INSPECT_URI_ONLY  "inspect_uri_only"
#define GLOBAL_ALERT      "no_alerts"
#define WEBROOT           "webroot"
#define TAB_URI_DELIMITER "tab_uri_delimiter"
#define WHITESPACE        "whitespace_chars"
#define NORMALIZE_HEADERS "normalize_headers"
#define NORMALIZE_COOKIES "normalize_cookies"
#define NORMALIZE_UTF     "normalize_utf"
#define NORMALIZE_JS      "normalize_javascript"
#define MAX_JS_WS         "max_javascript_whitespaces"
#define MAX_HEADERS       "max_headers"
#define INSPECT_COOKIES   "enable_cookie"
#define EXTRACT_GZIP      "inspect_gzip"
#define UNLIMIT_DECOMPRESS "unlimited_decompress"
#define INSPECT_RESPONSE  "extended_response_inspection"
#define COMPRESS_DEPTH    "compress_depth"
#define DECOMPRESS_DEPTH  "decompress_depth"
#define MAX_GZIP_MEM      "max_gzip_mem"
#define EXTENDED_ASCII    "extended_ascii_uri"
#define OPT_DISABLED      "disabled"
#define ENABLE_XFF        "enable_xff"
#define XFF_HEADERS_TOK   "xff_headers"
#define HTTP_METHODS      "http_methods"
#define LOG_URI           "log_uri"
#define LOG_HOSTNAME      "log_hostname"
#define HTTP_MEMCAP       "memcap"
#define MAX_SPACES    "max_spaces"
#define INSPECT_SWF       "decompress_swf"
#define INSPECT_PDF       "decompress_pdf"
#define NORMALIZE_NULLS   "normalize_random_nulls_in_text"
#define FAST_BLOCKING     "fast_blocking"

#define DECOMPRESS_DEFLATE "deflate"
#define DECOMPRESS_LZMA    "lzma"
#define LEGACY_MODE        "legacy_mode"

#define MAX_CLIENT_DEPTH 1460
#define MAX_SERVER_DEPTH 65535

/*
**  Alert subkeywords
*/
#define BOOL_YES     "yes"
#define BOOL_NO      "no"

/*
**  PROFILE subkeywords
*/
#define APACHE        "apache"
#define IIS           "iis"
#define IIS4_0        "iis4_0"
#define IIS5_0        "iis5_0" /* 5.0 only. For 5.1 and beyond, use IIS */
#define ALL           "all"

/*
**  IP Address list delimiters
*/
#define START_IPADDR_LIST "{"
#define END_IPADDR_LIST   "}"

/*
**  Port list delimiters
*/
#define START_PORT_LIST "{"
#define END_PORT_LIST   "}"

/*
**  XFF Header list delimiters, states, etc.
*/
#define START_XFF_HEADER_LIST  "{"
#define END_XFF_HEADER_LIST    "}"
#define START_XFF_HEADER_ENTRY "["
#define END_XFF_HEADER_ENTRY   "]"

#define XFF_MIN_PREC        (1)
#define XFF_MAX_PREC        (255)

#define XFF_STATE_START     (1)
#define XFF_STATE_OPEN      (2)
#define XFF_STATE_NAME      (3)
#define XFF_STATE_PREC      (4)
#define XFF_STATE_CLOSE     (5)
#define XFF_STATE_END       (6)

/*
**  Keyword for the default server configuration
*/
#define SERVER_DEFAULT "default"

typedef enum {
    CONFIG_MAX_SPACES = 0,
    CONFIG_MAX_JS_WS
} SpaceType;

typedef enum
{
   CD_CHARSET_UNKNOWN,
   CD_CHARSET_UTF8,
   CD_CHARSET_ISO_9959_1,
   CD_CHARSET_MIME
}CD_Charset;

static char** getHttpXffPrecedence(void* ssn, uint32_t flags, int* nFields);

/*
**  NAME
**    ProcessGlobalAlert::
*/
/**
**  Process the global alert keyword.
**
**  There is no arguments to this keyword, because you can only turn
**  all the alerts off.  As of now, we aren't going to support turning
**  all the alerts on.
**
**  @param GlobalConf  pointer to the global configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the lenght of the error string buffer
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
/*
static int ProcessGlobalAlert(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                              char *ErrorString, int ErrStrLen)
{
    GlobalConf->no_alerts = 1;

    return 0;
}
*/

static int ProcessIISUnicodeMap(uint8_t **iis_unicode_map,
                                char **iis_unicode_map_filename,
                                int *iis_unicode_map_codepage,
                                char *ErrorString, int ErrStrLen,
                                char **saveptr)
{
    char *pcToken;
    int  iRet;
    char filename[MAX_FILENAME];
    char *pcEnd;
    int  iCodeMap;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to token '%s'.", IIS_UNICODE_MAP);

        return -1;
    }

    /*
    **  If an absolute path is specified, then use that.
    */
#ifndef WIN32
    if(pcToken[0] == '/')
    {
        iRet = SnortSnprintf(filename, sizeof(filename), "%s", pcToken);
    }
    else
    {
        /*
        **  Set up the file name directory
        */
        if (snort_conf_dir[strlen(snort_conf_dir) - 1] == '/')
        {
            iRet = SnortSnprintf(filename, sizeof(filename),
                                 "%s%s", snort_conf_dir, pcToken);
        }
        else
        {
            iRet = SnortSnprintf(filename, sizeof(filename),
                                 "%s/%s", snort_conf_dir, pcToken);
        }
    }
#else
    if(strlen(pcToken)>3 && pcToken[1]==':' && pcToken[2]=='\\')
    {
        iRet = SnortSnprintf(filename, sizeof(filename), "%s", pcToken);
    }
    else
    {
        /*
        **  Set up the file name directory
        */
        if (snort_conf_dir[strlen(snort_conf_dir) - 1] == '\\' ||
            snort_conf_dir[strlen(snort_conf_dir) - 1] == '/' )
        {
            iRet = SnortSnprintf(filename, sizeof(filename),
                                 "%s%s", snort_conf_dir, pcToken);
        }
        else
        {
            iRet = SnortSnprintf(filename, sizeof(filename),
                                 "%s\\%s", snort_conf_dir, pcToken);
        }
    }
#endif

    if(iRet != SNORT_SNPRINTF_SUCCESS)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                 "Filename too long for token '%s'.", IIS_UNICODE_MAP);

        return -1;
    }

    /*
    **  Set the filename
    */
    *iis_unicode_map_filename = strdup(filename);
    if(*iis_unicode_map_filename == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Could not strdup() '%s' filename.",
                      IIS_UNICODE_MAP);

        return -1;
    }

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No codemap to select from IIS Unicode Map file.");

        return -1;
    }

    /*
    **  Grab the unicode codemap to use
    */
    iCodeMap = strtol(pcToken, &pcEnd, 10);
    if(*pcEnd || iCodeMap < 0)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid IIS codemap argument.");

        return -1;
    }

    /*
    **  Set the codepage
    */
    *iis_unicode_map_codepage = iCodeMap;

    /*
    **  Assume that the pcToken we now have is the filename of the map
    **  table.
    */
    iRet = hi_ui_parse_iis_unicode_map(iis_unicode_map, filename, iCodeMap);
    if (iRet)
    {
        if(iRet == HI_INVALID_FILE)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Unable to open the IIS Unicode Map file '%s'.",
                          filename);
        }
        else if(iRet == HI_FATAL_ERR)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Did not find specified IIS Unicode codemap in "
                          "the specified IIS Unicode Map file.");
        }
        else
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "There was an error while parsing the IIS Unicode Map file.");
        }

        return -1;
    }

    return 0;
}

static int ProcessOversizeDir(HTTPINSPECT_CONF *ServerConf,
                              char *ErrorString, int ErrStrLen,
                              char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int  iDirLen;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to token '%s'.", OVERSIZE_DIR);

        return -1;
    }

    /*
    **  Grab the oversize directory length
    */
    iDirLen = strtol(pcToken, &pcEnd, 10);
    if(*pcEnd || iDirLen < 0)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to token '%s'.", OVERSIZE_DIR);

        return -1;
    }

    ServerConf->long_dir = iDirLen;

    return 0;
}

static int ProcessHttpMemcap(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken, *pcEnd;
    int memcap;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                    "No argument to '%s' token.", HTTP_MEMCAP);
        return -1;
    }

    memcap = SnortStrtolRange(pcToken, &pcEnd, 10, 0 , INT_MAX);
    if(*pcEnd)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                    "Invalid argument to '%s'.", HTTP_MEMCAP);

        return -1;
    }

    if(memcap < MIN_HTTP_MEMCAP || memcap > MAX_HTTP_MEMCAP)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.  Must be between %d and "
                "%d.", HTTP_MEMCAP, MIN_HTTP_MEMCAP, MAX_HTTP_MEMCAP);

        return -1;
    }

    GlobalConf->memcap = memcap;

    return 0;

}


static int ProcessMaxGzipMem(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
        char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken, *pcEnd;
    int max_gzip_mem;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "No argument to '%s' token.", MAX_GZIP_MEM);
        return -1;
    }

    max_gzip_mem = SnortStrtolRange(pcToken, &pcEnd, 10, 0, INT_MAX);
    if ((pcEnd == pcToken) || *pcEnd || (errno == ERANGE))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.", MAX_GZIP_MEM);
        return -1;
    }

    if(max_gzip_mem < GZIP_MEM_MIN)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.", MAX_GZIP_MEM);
        return -1;
    }
    GlobalConf->max_gzip_mem = (unsigned int)max_gzip_mem;

    return 0;

}

static int ProcessCompressDepth(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                            char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  compress_depth;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "No argument to '%s' token.", COMPRESS_DEPTH);

        return -1;
    }

    compress_depth = SnortStrtol(pcToken, &pcEnd, 10);
    if(*pcEnd)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.", COMPRESS_DEPTH);

        return -1;
    }

    if(compress_depth <= 0 || compress_depth > MAX_GZIP_DEPTH)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.  Must be between 1 and "
                "%d.", COMPRESS_DEPTH, MAX_GZIP_DEPTH);

        return -1;
    }

    GlobalConf->compr_depth = compress_depth;

    return 0;
}

static int ProcessDecompressDepth(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                            char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  decompress_depth;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "No argument to '%s' token.", DECOMPRESS_DEPTH);

        return -1;
    }

    decompress_depth = SnortStrtol(pcToken, &pcEnd, 10);
    if(*pcEnd)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.", DECOMPRESS_DEPTH);

        return -1;
    }

    if(decompress_depth <= 0 || decompress_depth > MAX_GZIP_DEPTH)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.  Must be between 1 and "
                "%d.", DECOMPRESS_DEPTH, MAX_GZIP_DEPTH);

        return -1;
    }

    GlobalConf->decompr_depth = decompress_depth;

    return 0;
}

/*
**  NAME
**      ProcessGlobalConf::
*/
/**
**  This is where we process the global configuration for HttpInspect.
**
**  We set the values of the global configuraiton here.  Any errors that
**  are encountered are specified in the error string and the type of
**  error is returned through the return code, i.e. fatal, non-fatal.
**
**  The configuration options that are dealt with here are:
**      - global_alert
**          This tells us whether to do any internal alerts or not, on
**          a global scale.
**      - max_pipeline
**          Tells HttpInspect how many pipeline requests to buffer looking
**          for a response before inspection.
**      - inspection_type
**          What type of inspection for HttpInspect to do, stateless or
**          stateful.
**
**  @param GlobalConf  pointer to the global configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
int ProcessGlobalConf(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                      char *ErrorString, int ErrStrLen, char **saveptr)
{
    int  iRet;
    char *pcToken;
    int  iTokens = 0;

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        /*
        **  Show that we at least got one token
        */
        iTokens = 1;

        if(!strcmp(IIS_UNICODE_MAP, pcToken))
        {
            iRet = ProcessIISUnicodeMap(&GlobalConf->iis_unicode_map, &GlobalConf->iis_unicode_map_filename,
                                        &GlobalConf->iis_unicode_codepage, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(ANOMALOUS_SERVERS, pcToken))
        {
            /*
            **  This is easy to configure since we just look for the token
            **  and turn on the option.
            */
            GlobalConf->anomalous_servers = 1;
        }
        else if(!strcmp(PROXY_ALERT, pcToken))
        {
            GlobalConf->proxy_alert = 1;
        }
        else if (!strcmp(MAX_GZIP_MEM, pcToken))
        {
            iRet = ProcessMaxGzipMem(GlobalConf, ErrorString, ErrStrLen, saveptr);
            if(iRet)
                return iRet;
        }
        else if (!strcmp(COMPRESS_DEPTH, pcToken))
        {
            iRet = ProcessCompressDepth(GlobalConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
                return iRet;
        }
        else if (!strcmp(DECOMPRESS_DEPTH, pcToken))
        {
            iRet = ProcessDecompressDepth(GlobalConf, ErrorString, ErrStrLen, saveptr);
            if(iRet)
                return iRet;
        }
        else if (!strcmp(OPT_DISABLED, pcToken))
        {
            GlobalConf->disabled = 1;
            return 0;
        }
        else if (!strcmp(NORMALIZE_NULLS, pcToken))
        {
            GlobalConf->normalize_nulls = TRUE;
        }
        else if (!strcmp(FAST_BLOCKING, pcToken))
        {
            GlobalConf->fast_blocking = TRUE;
        }

        else if (!strcmp(HTTP_MEMCAP, pcToken))
        {
            iRet = ProcessHttpMemcap(GlobalConf, ErrorString, ErrStrLen, saveptr);
            if(iRet)
                return iRet;
        }
        else if (!file_api->parse_mime_decode_args(&(GlobalConf->decode_conf), pcToken, "HTTP", saveptr))
        {
            continue;
        }
        else
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid keyword '%s' for '%s' configuration.",
                          pcToken, GLOBAL);

            return -1;
        }
    }

    /*
    **  If there are not any tokens to the configuration, then
    **  we let the user know and log the error.  return non-fatal
    **  error.
    */
    if(!iTokens)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No tokens to '%s' configuration.", GLOBAL);

        return -1;
    }

    /*
    **  Let's check to make sure that we get a default IIS Unicode Codemap
    */
    if(!GlobalConf->iis_unicode_map)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Global configuration must contain an IIS Unicode Map "
                      "configuration.  Use token '%s'.", IIS_UNICODE_MAP);

        return -1;
    }

    return 0;
}


/*
**  NAME
**    ProcessProfile::
*/
/** Returns error messages for failed hi_ui_config_set_profile calls.
 **
 ** Called exclusively by ProcessProfile.
 */
static inline int _ProcessProfileErr(int iRet, char* ErrorString,
                                     int ErrStrLen, char *token)
{
    if(iRet == HI_MEM_ALLOC_FAIL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Memory allocation failed while setting the '%s' "
                      "profile.", token);
        return -1;
    }
    else
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Undefined error code for set_profile_%s.", token);
        return -1;
    }
}

/*
**  NAME
**    ProcessProfile::
*/
/**
**  Process the PROFILE configuration.
**
**  This function verifies that the argument to the profile configuration
**  is valid.  We also check to make sure there is no additional
**  configuration after the PROFILE.  This is no allowed, so we
**  alert on that fact.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessProfile(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                          HTTPINSPECT_CONF *ServerConf,
                          char *ErrorString, int ErrStrLen,
                          char **saveptr)
{
    char *pcToken;
    int  iRet;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to '%s'.", PROFILE_STRING);

        return -1;
    }

    /*
    **  Load the specific type of profile
    */
    if(!strcmp(APACHE, pcToken))
    {
        iRet = hi_ui_config_set_profile_apache(ServerConf);
        if (iRet)
        {
            /*  returns -1 */
            return _ProcessProfileErr(iRet, ErrorString, ErrStrLen, pcToken);
        }

        ServerConf->profile = HI_APACHE;
    }
    else if(!strcmp(IIS, pcToken))
    {
        iRet = hi_ui_config_set_profile_iis(ServerConf, GlobalConf->iis_unicode_map);
        if (iRet)
        {
            /* returns -1 */
            return _ProcessProfileErr(iRet, ErrorString, ErrStrLen, pcToken);
        }

        ServerConf->profile = HI_IIS;
    }
    else if(!strcmp(IIS4_0, pcToken) || !strcmp(IIS5_0, pcToken))
    {
        iRet = hi_ui_config_set_profile_iis_4or5(ServerConf, GlobalConf->iis_unicode_map);
        if (iRet)
        {
            /* returns -1 */
            return _ProcessProfileErr(iRet, ErrorString, ErrStrLen, pcToken);
        }

        ServerConf->profile = (pcToken[3]=='4'?HI_IIS4:HI_IIS5);
    }
    else if(!strcmp(ALL, pcToken))
    {
        iRet = hi_ui_config_set_profile_all(ServerConf, GlobalConf->iis_unicode_map);
        if (iRet)
        {
            /* returns -1 */
            return _ProcessProfileErr(iRet, ErrorString, ErrStrLen, pcToken);
        }

        ServerConf->profile = HI_ALL;
    }
    else
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid profile argument '%s'.", pcToken);

        return -1;
    }

    return 0;


}

/*
**  NAME
**    ProcessPorts::
*/
/**
**  Process the port list for the server configuration.
**
**  This configuration is a list of valid ports and is ended by a
**  delimiter.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessPorts(HTTPINSPECT_CONF *ServerConf,
                        char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int  iPort;
    int  iEndPorts = 0;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid port list format.");

        return -1;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must start a port list with the '%s' token.",
                      START_PORT_LIST);

        return -1;
    }

    memset(ServerConf->ports, 0, MAXPORTS_STORAGE);

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndPorts = 1;
            break;
        }

        iPort = strtol(pcToken, &pcEnd, 10);

        /*
        **  Validity check for port
        */
        if(*pcEnd)
        {
            SnortSnprintf(ErrorString, ErrStrLen, "Invalid port number.");

            return -1;
        }

        if(iPort < 0 || iPort > MAXPORTS-1)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid port number.  Must be between 0 and 65535.");

            return -1;
        }

        enablePort( ServerConf->ports, iPort );

        if(ServerConf->port_count < MAXPORTS)
            ServerConf->port_count++;
    }

    if(!iEndPorts)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must end '%s' configuration with '%s'.",
                      PORTS, END_PORT_LIST);

        return -1;
    }

    return 0;
}

/*
**  NAME
**    ProcessFlowDepth::
*/
/**
**  Configure the flow depth for a server.
**
**  Check that the value for flow depth is within bounds
**  and is a valid number.
**
**  @param ServerConf  pointer to the server configuration
**  @param ServerOrClient which flowdepth is being set
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessFlowDepth(HTTPINSPECT_CONF *ServerConf, int ServerOrClient,
                            char *ErrorString, int ErrStrLen, char **saveptr, char *pToken, int maxDepth)
{
    char *pcToken;
    int  iFlowDepth;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to '%s' token.", pToken);

        return -1;
    }

    iFlowDepth = SnortStrtol(pcToken, &pcEnd, 10);
    if(*pcEnd)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.", pToken);

        return -1;
    }

    /* -1 here is okay, which means ignore ALL server side traffic */
    if(iFlowDepth < -1 || iFlowDepth > maxDepth)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.  Must be between -1 and %d.",
                      pToken, maxDepth);

        return -1;
    }

    if (ServerOrClient == HI_SI_CLIENT_MODE)
        ServerConf->client_flow_depth = iFlowDepth;
    else
        ServerConf->server_flow_depth = iFlowDepth;

    return 0;
}

/*
**  NAME
**    ProcessPostDepth::
*/
/**
**  Configure the post depth for client requests
**
**  Checks that the value for flow depth is within bounds
**  and is a valid number.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessPostDepth(HTTPINSPECT_CONF *ServerConf,
                            char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  post_depth;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "No argument to '%s' token.", POST_DEPTH);

        return -1;
    }

    post_depth = SnortStrtol(pcToken, &pcEnd, 10);
    if(*pcEnd)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.", POST_DEPTH);

        return -1;
    }

    /* 0 means 'any depth' */
    if(post_depth < -1 || post_depth > ( IP_MAXPACKET - (IP_HEADER_LEN + TCP_HEADER_LEN) ) )
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid argument to '%s'.  Must be between -1 and "
                "%d.", POST_DEPTH,( IP_MAXPACKET - (IP_HEADER_LEN + TCP_HEADER_LEN) ));

        return -1;
    }

    ServerConf->post_depth = post_depth;

    return 0;
}


/*
**  NAME
**    ProcessChunkLength::
*/
/**
**  Process and verify the chunk length for the server configuration.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessChunkLength(HTTPINSPECT_CONF *ServerConf,
                              char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  iChunkLength;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to '%s' token.", CHUNK_LENGTH);

        return -1;
    }

    iChunkLength = SnortStrtolRange(pcToken, &pcEnd, 10, 0, INT_MAX);
    if ((pcEnd == pcToken) || *pcEnd || (errno == ERANGE))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.", CHUNK_LENGTH);

        return -1;
    }

    ServerConf->chunk_length = iChunkLength;

    return 0;
}

/*
**  NAME
**    ProcessSmallChunkLength::
*/
/**
**  Process and verify the small chunk length for the server configuration.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessSmallChunkLength(HTTPINSPECT_CONF *ServerConf,
                                   char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int num_toks = 0;
    bool got_param_end = 0;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if (!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid cmd list format.");

        return -1;
    }

    if (strcmp(START_PORT_LIST, pcToken))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Must start small chunk length parameters with the '%s' token.",
                START_PORT_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if (!strcmp(END_PORT_LIST, pcToken))
        {
            got_param_end = 1;
            break;
        }

        num_toks++;
        if (num_toks > 2)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Too many arguments to '%s'.", SMALL_CHUNK_LENGTH);

            return -1;
        }

        if (num_toks == 1)
        {
            uint32_t chunk_length = (uint32_t)SnortStrtoulRange(pcToken, &pcEnd, 10, 0, UINT8_MAX);
            if ((pcEnd == pcToken) || *pcEnd || (errno == ERANGE))
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                              "Invalid argument to chunk length param of '%s'. "
                              "Must be between 0 and %u.\n",
                              SMALL_CHUNK_LENGTH, UINT8_MAX);

                return -1;
            }

            ServerConf->small_chunk_length.size = (uint8_t)chunk_length;
        }
        else
        {
            uint32_t num_chunks_threshold = (uint32_t)SnortStrtoulRange(pcToken, &pcEnd, 10, 0, UINT8_MAX);
            if ((pcEnd == pcToken) || *pcEnd || (errno == ERANGE))
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                              "Invalid argument to number of consecutive chunks "
                              "threshold of '%s'. Must be between 0 and %u.\n",
                              SMALL_CHUNK_LENGTH, UINT8_MAX);

                return -1;
            }

            ServerConf->small_chunk_length.num = (uint8_t)num_chunks_threshold;
        }
    }

    if (num_toks != 2)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Not enough arguments to '%s'.", SMALL_CHUNK_LENGTH);

        return -1;
    }

    if (!got_param_end)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must end '%s' configuration with '%s'.", SMALL_CHUNK_LENGTH, END_PORT_LIST);

        return -1;
    }

    return 0;
}

/*
**  NAME
**    ProcessMaxHeaders::
*/
/**
**  Process and verify the maximum allowed number of headers for the
**  server configuration.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessMaxHeaders(HTTPINSPECT_CONF *ServerConf,
                              char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  length;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to '%s' token.", MAX_HEADERS);

        return -1;
    }

    length = strtol(pcToken, &pcEnd, 10);
    if(*pcEnd || pcEnd == pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.", MAX_HEADERS);

        return -1;
    }

    if(length < 0)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'. Valid range is 0 to 1024.", MAX_HEADERS);

        return -1;
    }

    if(length > 1024)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.  Valid range is 0 to 1024.", MAX_HEADERS);

        return -1;
    }

    ServerConf->max_headers = length;

    return 0;
}

/*
**  NAME
**    ProcessMaxHdrLen::
*/
/**
**  Process and verify the maximum allowed header length for the
**  server configuration.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessMaxHdrLen(HTTPINSPECT_CONF *ServerConf,
                              char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  length;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to '%s' token.", MAX_HDR_LENGTH);

        return -1;
    }

    length = strtol(pcToken, &pcEnd, 10);
    if(*pcEnd || pcEnd == pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.", MAX_HDR_LENGTH);

        return -1;
    }

    if(length < 0)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'. Valid range is 0 to 65535.", MAX_HDR_LENGTH);

        return -1;
    }

    if(length > 65535)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.  Valid range is 0 to 65535.", MAX_HDR_LENGTH);

        return -1;
    }

    ServerConf->max_hdr_len = length;

    return 0;
}

/*
**  NAME
**    ProcessConfOpt::
*/
/**
**  Set the CONF_OPT on and alert fields.
**
**  We check to make sure of valid parameters and then
**  set the appropriate fields.  Not much more to it, than
**  that.
**
**  @param ConfOpt  pointer to the configuration option
**  @param Option   character pointer to the option being configured
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessConfOpt(HTTPINSPECT_CONF_OPT *ConfOpt, char *Option,
                          char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to token '%s'.", Option);

        return -1;
    }

    /*
    **  Check for the alert value
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
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to token '%s'.", Option);

        return -1;
    }

    ConfOpt->on = 1;

    return 0;
}

/*
**  NAME
**    ProcessNonRfcChar::
*/
/***
**  Configure any characters that the user wants alerted on in the
**  URI.
**
**  This function allocates the memory for CONF_OPT per character and
**  configures the alert option.
**
**  @param ConfOpt  pointer to the configuration option
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessNonRfcChar(HTTPINSPECT_CONF *ServerConf,
                             char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int  iChar;
    int  iEndChar = 0;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid '%s' list format.", NON_RFC_CHAR);

        return -1;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must start a '%s' list with the '%s' token.",
                      NON_RFC_CHAR, START_PORT_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndChar = 1;
            break;
        }

        iChar = strtol(pcToken, &pcEnd, 16);
        if(*pcEnd)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid argument to '%s'.  Must be a single character.",
                          NON_RFC_CHAR);

            return -1;
        }

        if(iChar < 0 || iChar > 255)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid character value to '%s'.  Must be a single "
                          "character no greater than 255.", NON_RFC_CHAR);

            return -1;
        }

        ServerConf->non_rfc_chars[iChar] = 1;
    }

    if(!iEndChar)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must end '%s' configuration with '%s'.",
                      NON_RFC_CHAR, END_PORT_LIST);

        return -1;
    }

    return 0;
}

/*
**  NAME
**    ProcessWhitespaceChars::
*/
/***
**  Configure any characters that the user wants to be treated as
**  whitespace characters before and after a URI.
**
**
**  @param ServerConf  pointer to the server configuration structure
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessWhitespaceChars(HTTPINSPECT_CONF *ServerConf,
                             char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int  iChar;
    int  iEndChar = 0;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid '%s' list format.", WHITESPACE);

        return -1;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must start a '%s' list with the '%s' token.",
                      WHITESPACE, START_PORT_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndChar = 1;
            break;
        }

        iChar = strtol(pcToken, &pcEnd, 16);
        if(*pcEnd)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid argument to '%s'.  Must be a single character.",
                          WHITESPACE);

            return -1;
        }

        if(iChar < 0 || iChar > 255)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid character value to '%s'.  Must be a single "
                          "character no greater than 255.", WHITESPACE);

            return -1;
        }

        ServerConf->whitespace[iChar] = HI_UI_CONFIG_WS_BEFORE_URI;
    }

    if(!iEndChar)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Must end '%s' configuration with '%s'.",
                       WHITESPACE, END_PORT_LIST);

        return -1;
    }

    return 0;
}

static bool Is_Field_Prec_Unique( uint8_t *Name_Array[], uint8_t Prec_Array[],
                                 uint8_t *Name, uint8_t Precedence,
                                 char *ErrorString, int ErrStrLen )
{
    int i;

    for( i=0; i<HTTP_MAX_XFF_FIELDS; i++ )
    {
        /* A NULL entry indicates the end of the list */
        if( Name_Array[i] == NULL )
            break;

        if( strcmp( (char *)Name, (char *)Name_Array[i] ) == 0 )
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Duplicate XFF Field name: %s.", Name);

            return( false );
        }

        if( (Precedence != 0) && (Precedence == Prec_Array[i]) )
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Duplicate XFF Precedence value: %u.", Precedence);

            return( false );
        }
    }
    return( true );
}

static int Find_Open_Field( uint8_t *Name_Array[] )
{
    int i;

    for( i=0; i<HTTP_MAX_XFF_FIELDS; i++ )
        if( Name_Array[i] == NULL )
            return( i );
    return( -1 );
}

static void Push_Down_XFF_List( uint8_t *Name_Array[], uint8_t *Length_Array, uint8_t Prec_Array[], int Start )
{
    int i;

    for( i=(HTTP_MAX_XFF_FIELDS-1); i>=Start; i-- )
    {
        Name_Array[i] = Name_Array[i-1];
        Length_Array[i] = Length_Array[i-1];
        Prec_Array[i] = Prec_Array[i-1];
    }
}

/* The caller of Add_XFF_Field is responsible for determining that at least one
   additional entry is availble in both the field name array and the precedence
   value array. */
static int Add_XFF_Field( HTTPINSPECT_CONF *ServerConf, uint8_t *Prec_Array, uint8_t *Field_Name,
                          uint8_t Precedence, char *ErrorString, int ErrStrLen )
{
    uint8_t **Fields = ServerConf->xff_headers;
    uint8_t *Lengths = ServerConf->xff_header_lengths;
    size_t Length = 0;
    int i;
    char **Builtin_Fields;
    unsigned char *cp;
    const char *Special_Chars = { "_-" };
    bool fieldAdded = false;

    if(!Field_Name )
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Field name is null" );
        return -1;
    }
    
    if( (Length = strlen( (char *)Field_Name )) > UINT8_MAX )
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Field name limited to %u characters", UINT8_MAX);
        return -1;
    }

    cp = (unsigned char*)Field_Name;
    while( *cp != '\0' )
    {
        *cp = (uint8_t)toupper(*cp);  // Fold to upper case for runtime comparisons
        if( (isalnum( *cp ) == 0) && (strchr( Special_Chars, (int)(*cp) ) == NULL) )
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid xff field name: %s ", Field_Name);
            return( -1 );
        }
        cp += 1;
    }

    Builtin_Fields = hi_client_get_field_names();
    for( i=0; Builtin_Fields[i]!=NULL; i++ )
    {
        if( strcasecmp(Builtin_Fields[i], HTTP_XFF_FIELD_X_FORWARDED_FOR) == 0 ) continue;
        if( strcasecmp(Builtin_Fields[i], HTTP_XFF_FIELD_TRUE_CLIENT_IP) == 0 ) continue;
        if( strcasecmp(Builtin_Fields[i], (char *)Field_Name) == 0 )
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Cannot use: '%s' as an xff header.", Field_Name);
            return -1;
        }
    }

    if( !Is_Field_Prec_Unique( Fields, Prec_Array, Field_Name, Precedence,
                              ErrorString, ErrStrLen ) )
        return( -1 );

    if( Precedence == 0 )
    {
        if( (i = Find_Open_Field( Fields )) >= 0 )
        {
            Fields[i] = Field_Name;
            Lengths[i] = (uint8_t)Length;
            Prec_Array[i] = 0;
            fieldAdded = true;
        }
        else
            return( -1 );
    }
    else
    {
        for( i=0; i<HTTP_MAX_XFF_FIELDS; i++ )
        {
            /* If the space is open, place the name & prec */
            if( Fields[i] == NULL )
            {
                Fields[i] = Field_Name;
                Lengths[i] = (uint8_t)Length;
                Prec_Array[i] = Precedence;
                fieldAdded = true;
                break;
            }

            /* if the new entry is higher precedence than the current list element,
               push the list down and place the new entry here. */
            if( Precedence < Prec_Array[i] )
            {
                Push_Down_XFF_List( Fields, Lengths, Prec_Array, i+1 );
                Fields[i] = Field_Name;
                Lengths[i] = (uint8_t)Length;
                Prec_Array[i] = Precedence;
                fieldAdded = true;
                break;
            }
        }
    }

    if (fieldAdded)
    {
        for (i = 0; i < HTTP_MAX_XFF_FIELDS; i++)
        {
            if (!xffFields[i])
            {
                xffFields[i] = SnortStrndup((char *)Field_Name, UINT8_MAX);
                break;
            }
            else if (!strncasecmp(xffFields[i], (char *)Field_Name, UINT8_MAX)) break;
        }
    }

    return( 0 );
}

static int ProcessXFF_HeaderList(HTTPINSPECT_CONF *ServerConf,
                      char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int Parse_State;
    bool Keep_Parsing = false;
    bool Have_XFF = false;
    bool Have_TCI = false;
    uint8_t Prec_List[HTTP_MAX_XFF_FIELDS];
    unsigned char Count = 0;
    unsigned char Max_XFF = { (HTTP_MAX_XFF_FIELDS-HTTP_XFF_BUILTIN_NAMES) };
    uint8_t *Field_Name=NULL;
    unsigned int Precedence;
    int i;
    uint8_t addXffFieldName = 0;

    /* NOTE:  This procedure assumes that the ServerConf->xff_headers array
              contains all NULL's due to the structure allocation process. */

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid XFF list format.");

        return -1;
    }

    for( i=0; i<HTTP_MAX_XFF_FIELDS; i++ ) Prec_List[i] = 0;
    Parse_State = XFF_STATE_START;
    Keep_Parsing = true;

    do
    {
        switch( Parse_State )
        {
            case( XFF_STATE_START ):
            {
                if( strcmp( START_XFF_HEADER_LIST, pcToken ) != 0 )
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                        "Must start an xff header list with the '%s' token.",
                        START_XFF_HEADER_LIST);
                    return( -1 );
                }
                Parse_State = XFF_STATE_OPEN;
                break;
            }
            case( XFF_STATE_OPEN ):
            {
                if( strcmp( START_XFF_HEADER_ENTRY, pcToken ) == 0 )
                    Parse_State = XFF_STATE_NAME;
                else if( strcmp( END_XFF_HEADER_LIST, pcToken ) == 0 )
                {
                    Parse_State = XFF_STATE_END;
                    Keep_Parsing = false;
                }
                else
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                 "xff header parsing error");
                    return( -1 );
                }
                break;
            }
            case( XFF_STATE_NAME ):
            {
                Field_Name = (uint8_t *)SnortStrdup( pcToken );
                Parse_State = XFF_STATE_PREC;
                break;
            }
            case( XFF_STATE_PREC ):
            {
                Precedence = xatou( pcToken, "Precedence" );

                if( (Precedence > XFF_MAX_PREC) || (Precedence < XFF_MIN_PREC) )
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                 "illegal precendence value: %u", Precedence);
                    if( Field_Name )
                        free(Field_Name);
                    return( -1 );
                }

                if( strcasecmp( (char *)Field_Name, HTTP_XFF_FIELD_X_FORWARDED_FOR) == 0 )
                {
                    Max_XFF += 1;
                    Have_XFF = true;
                }
                else if( strcasecmp( (char *)Field_Name, HTTP_XFF_FIELD_TRUE_CLIENT_IP) == 0 )
                {
                    Max_XFF += 1;
                    Have_TCI = true;
                }
                else
                    Count += 1;

                if( Count > Max_XFF )
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                 "too many xff field names");
                    if( Field_Name )
                        free(Field_Name);
                    return( -1 );
                }

                if( Add_XFF_Field( ServerConf, Prec_List, Field_Name, (uint8_t)Precedence,
                                   ErrorString, ErrStrLen ) != 0 )
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                 "Error adding xff header: %s", Field_Name);
                    if( Field_Name )
                        free(Field_Name);
                    return( -1 );
                }
                addXffFieldName = 1;
                Parse_State = XFF_STATE_CLOSE;
                break;
            }
            case( XFF_STATE_CLOSE ):
            {
                if( strcmp( END_XFF_HEADER_ENTRY, pcToken ) != 0 )
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                        "Must end an xff header entry with the '%s' token.",
                        END_XFF_HEADER_ENTRY);
                    if( Field_Name )
                        free(Field_Name);
                    return( -1 );
                }
                Parse_State = XFF_STATE_OPEN;
                break;
            }
            default:
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                             "xff header parsing error");
                 if( Field_Name )
                     free(Field_Name);
                return( -1 );
            }
        }
    }
    while( Keep_Parsing && ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL) );

    if( Field_Name && !addXffFieldName )
    {
        free(Field_Name);
        Field_Name = NULL;
    }

    if( Parse_State != XFF_STATE_END )
    {
        SnortSnprintf(ErrorString, ErrStrLen, "xff header parsing error");
        return( -1 );
    }

    /* NOTE:  The number of fields added here MUST be represented in HTTP_XFF_BUILTIN_NAMES value
              to assure that we reserve room for them on the list. */
    if( !Have_XFF )
    {
        Field_Name = (uint8_t *)SnortStrdup( HTTP_XFF_FIELD_X_FORWARDED_FOR );
        if( Add_XFF_Field( ServerConf, Prec_List, Field_Name, 0,
                           ErrorString, ErrStrLen ) != 0 )
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                         "problem adding builtin xff field - too many fields?");
            if( Field_Name )
                free(Field_Name);
            return( -1 );
        }
    }

    if( !Have_TCI )
    {
        Field_Name = (uint8_t *)SnortStrdup( HTTP_XFF_FIELD_TRUE_CLIENT_IP );
        if( Add_XFF_Field( ServerConf, Prec_List, Field_Name, 0,
                           ErrorString, ErrStrLen ) != 0 )
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                         "problem adding builtin xff field - too many fields?");
            if( Field_Name )
                free(Field_Name);
            return( -1 );
        }
    }


    return 0;
}

static char** getHttpXffFields(int* nFields)
{
    if (!xffFields[0])
    {
        if (nFields) *nFields = 0;
        return NULL;
    }

    if (nFields)
    {
        for ((*nFields) = 0; ((*nFields) < HTTP_MAX_XFF_FIELDS) && xffFields[*nFields];
             (*nFields)++)
            ;
    }
    return xffFields;
}

static int ProcessHttpMethodList(HTTPINSPECT_CONF *ServerConf,
                      char *ErrorString, int ErrStrLen, char **saveptr)
{
    HTTP_CMD_CONF *HTTPMethods = NULL;
    char *pcToken;
    char *cmd;
    int  iEndCmds = 0;
    int  iRet;


    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid cmd list format.");

        return -1;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Must start a http request method list with the '%s' token.",
                START_PORT_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndCmds = 1;
            break;
        }

        cmd = pcToken;

        /* Max method length cannot exceed 256, as this is the size of the key array used in KMapAdd function */
        if(strlen(pcToken) > MAX_METHOD_LEN)
        {
            snprintf(ErrorString, ErrStrLen,
                    "Length of the http request method should not exceed the max request method length of '%d'.",
                    MAX_METHOD_LEN);
            return -1;
        }

        HTTPMethods = http_cmd_lookup_find(ServerConf->cmd_lookup, cmd, strlen(cmd), &iRet);

        if (HTTPMethods == NULL)
        {
            /* Add it to the list */
            // note that struct includes 1 byte for null, so just add len
            HTTPMethods = (HTTP_CMD_CONF *)calloc(1, sizeof(HTTP_CMD_CONF)+strlen(cmd));
            if (HTTPMethods == NULL)
            {
                FatalError("%s(%d) Could not allocate memory for HTTP Method configuration.\n",
                                           __FILE__, __LINE__);
            }

            strcpy(HTTPMethods->cmd_name, cmd);

            http_cmd_lookup_add(ServerConf->cmd_lookup, cmd, strlen(cmd), HTTPMethods);
        }
    }


    if(!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen,
            "Must end '%s' configuration with '%s'.",
                HTTP_METHODS, END_PORT_LIST);

        return -1;
    }

    return 0;
}

static int ProcessDecompressionTypeList(HTTPINSPECT_CONF *ServerConf,
                      char *ConfigType, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *cmd;
    int  iEndCmds = 0;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Invalid algorithm list format.");

        return -1;
    }

    if(strcmp(START_PORT_LIST, pcToken))
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                "Must start an algotithm list with the '%s' token.",
                START_PORT_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(END_PORT_LIST, pcToken))
        {
            iEndCmds = 1;
            break;
        }

        cmd = pcToken;

        /* FIX THIS:  For now, the decompression types are limited to SWF/LZMA, SWF/Deflate, and PDF/Deflate.
                      Hardcode the comparison logic and storage in the HTTPINSPECT_CONF struc. */
        if( 0 == strcmp(ConfigType, INSPECT_SWF))
        {
            if( 0 == strcmp(cmd, DECOMPRESS_DEFLATE) )
            {
                ServerConf->file_decomp_modes |= (FILE_SWF_ZLIB_BIT | FILE_REVERT_BIT);
            }
#ifdef LZMA
            else if( 0 == strcmp(cmd, DECOMPRESS_LZMA) )
            {
                ServerConf->file_decomp_modes |= (FILE_SWF_LZMA_BIT | FILE_REVERT_BIT);
            }
#endif
            else
            {
                snprintf(ErrorString, ErrStrLen,
                         "Bad cmd element passed to ProcessDecompressionTypeList(): %s", cmd);
                return( -1 );

            }
        }
        else if( 0 == strcmp(ConfigType, INSPECT_PDF) )
        {
            if( 0 == strcmp(cmd, DECOMPRESS_DEFLATE) )
            {
                ServerConf->file_decomp_modes |= (FILE_PDF_DEFL_BIT | FILE_REVERT_BIT);
            }
            else
            {
            snprintf(ErrorString, ErrStrLen,
                     "Bad cmd passed to ProcessDecompressionTypeList(): %s", cmd);
            return( -1 );

            }
        }
        else
        {
            snprintf(ErrorString, ErrStrLen,
                     "Bad ConfigType passed to ProcessDecompressionTypeList(): %s", ConfigType);
            return( -1 );
        }
    }

    if(!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen,
            "Must end '%s' configuration with '%s'.",
                ConfigType, END_PORT_LIST);

        return -1;
    }

    return 0;
}

/*
**  NAME
**    ProcessMaxSpaces::
*/
/**
**  Process and verify the maximum allowed spaces for the
**  server configuration.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessMaxSpaces(HTTPINSPECT_CONF *ServerConf,
                              char *ErrorString, int ErrStrLen, char **saveptr,
                              char *configOption, SpaceType type)
{
    char *pcToken;
    int  num_spaces;
    char *pcEnd;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No argument to '%s' token.", configOption);

        return -1;
    }

     num_spaces = SnortStrtolRange(pcToken, &pcEnd, 10, 0 , INT_MAX);
     if(*pcEnd)
     {
         SnortSnprintf(ErrorString, ErrStrLen,
                     "Invalid argument to '%s'.", configOption);

         return -1;
     }

    if(num_spaces < 0)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'. Valid range is 0 to 65535.", configOption);

        return -1;
    }

    if(num_spaces > 65535)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "Invalid argument to '%s'.  Valid range is 0 to 65535.", configOption);

        return -1;
    }

    switch(type)
    {
        case CONFIG_MAX_SPACES:
            ServerConf->max_spaces = num_spaces;
            break;
        case CONFIG_MAX_JS_WS:
            ServerConf->max_js_ws = num_spaces;
            break;
        default:
            break;
    }

    return 0;
}


/*
**  NAME
**    ProcessServerConf::
*/
/**
**  Process the global server configuration.
**
**  Take the configuration and translate into the global server
**  configuration.  We also check for any configuration errors and
**  invalid keywords.
**
**  @param ServerConf  pointer to the server configuration
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessServerConf(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                             HTTPINSPECT_CONF *ServerConf,
                             char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  iRet;
    int  iPorts = 0;
    HTTPINSPECT_CONF_OPT *ConfOpt;

    /*
    **  Check for profile keyword first, it's the only place in the
    **  configuration that is correct.
    */
    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(pcToken == NULL)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "WARNING: No tokens to '%s' configuration.", SERVER);

        return 1;
    }

    if(!strcmp(PROFILE_STRING, pcToken))
    {
        iRet = ProcessProfile(GlobalConf, ServerConf, ErrorString, ErrStrLen, saveptr);
        if (iRet)
        {
            return iRet;
        }

        pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
        if(pcToken == NULL)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "No port list to the profile token.");

            return -1;
        }

        do
        {
            if(!strcmp(PORTS, pcToken))
            {
                iRet = ProcessPorts(ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }

                iPorts = 1;
            }
            else if(!strcmp(IIS_UNICODE_MAP, pcToken))
            {
                iRet = ProcessIISUnicodeMap(&ServerConf->iis_unicode_map,
                                            &ServerConf->iis_unicode_map_filename,
                                            &ServerConf->iis_unicode_codepage,
                                            ErrorString,ErrStrLen, saveptr);
                if (iRet)
                {
                    return -1;
                }
            }
            else if(!strcmp(ALLOW_PROXY, pcToken))
            {
                ServerConf->allow_proxy = 1;
            }
            else if(!strcmp(FLOW_DEPTH, pcToken) || !strcmp(SERVER_FLOW_DEPTH, pcToken))
            {
                iRet = ProcessFlowDepth(ServerConf, HI_SI_SERVER_MODE, ErrorString, ErrStrLen, saveptr, pcToken, MAX_SERVER_DEPTH);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if(!strcmp(CLIENT_FLOW_DEPTH, pcToken))
            {
                iRet = ProcessFlowDepth(ServerConf, HI_SI_CLIENT_MODE, ErrorString, ErrStrLen, saveptr, pcToken, MAX_CLIENT_DEPTH);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if(!strcmp(POST_DEPTH, pcToken))
            {
                iRet = ProcessPostDepth(ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if(!strcmp(GLOBAL_ALERT, pcToken))
            {
                ServerConf->no_alerts = 1;
            }
            else if(!strcmp(OVERSIZE_DIR, pcToken))
            {
                iRet = ProcessOversizeDir(ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }

            }
            else if(!strcmp(INSPECT_URI_ONLY, pcToken))
            {
                ServerConf->uri_only = 1;
            }
            else if (!strcmp(NORMALIZE_HEADERS, pcToken))
            {
                ServerConf->normalize_headers = 1;
            }
            else if (!strcmp(NORMALIZE_COOKIES, pcToken))
            {
                ServerConf->normalize_cookies = 1;
            }
            else if (!strcmp(NORMALIZE_UTF, pcToken))
            {
                ServerConf->normalize_utf = 1;
            }
            else if (!strcmp(NORMALIZE_JS, pcToken))
            {
                if(!ServerConf->inspect_response)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                        "Enable '%s' before setting '%s'",INSPECT_RESPONSE, NORMALIZE_JS);
                    return -1;
                }
                ServerConf->normalize_javascript = 1;
            }
            else if(!strcmp(MAX_JS_WS, pcToken))
            {
                if(!ServerConf->normalize_javascript)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                "Enable '%s' before setting '%s'", NORMALIZE_JS, MAX_JS_WS);
                    return -1;
                }
                iRet = ProcessMaxSpaces(ServerConf, ErrorString, ErrStrLen, saveptr, MAX_JS_WS, CONFIG_MAX_JS_WS);
                if (iRet)
                {
                    return iRet;
                }
            }

            else if (!strcmp(INSPECT_COOKIES, pcToken))
            {
                ServerConf->enable_cookie = 1;
            }
            else if (!strcmp(INSPECT_RESPONSE, pcToken))
            {
                ServerConf->inspect_response = 1;
            }
            else if (!strcmp(HTTP_METHODS, pcToken))
            {
                iRet = ProcessHttpMethodList(ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if (!strcmp(EXTRACT_GZIP, pcToken))
            {
                if(!ServerConf->inspect_response)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Enable '%s' before setting '%s'",INSPECT_RESPONSE, EXTRACT_GZIP);
                    return -1;
                }

                ServerConf->extract_gzip = 1;
            }
            else if (!strcmp(UNLIMIT_DECOMPRESS, pcToken))
            {
                if(!ServerConf->extract_gzip)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Enable '%s' before setting '%s'",EXTRACT_GZIP, UNLIMIT_DECOMPRESS);
                    return -1;
                }

                if((GlobalConf->compr_depth != MAX_GZIP_DEPTH) && (GlobalConf->decompr_depth != MAX_GZIP_DEPTH))
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "'%s' and '%s' should be set to max in the default policy to enable '%s'",
                            COMPRESS_DEPTH, DECOMPRESS_DEPTH, UNLIMIT_DECOMPRESS);
                    return -1;
                }

                GlobalConf->compr_depth = GlobalConf->decompr_depth = MAX_GZIP_DEPTH;
                ServerConf->unlimited_decompress = 1;
            }
#ifdef FILE_DECOMP_SWF
            else if (!strcmp(INSPECT_SWF, pcToken))
            {
                if(!ServerConf->inspect_response)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Enable '%s' before setting '%s'",INSPECT_RESPONSE, INSPECT_SWF);
                    return -1;
                }

                ProcessDecompressionTypeList(ServerConf,pcToken,ErrorString,ErrStrLen, saveptr);
            }
#endif
#ifdef FILE_DECOMP_PDF
            else if (!strcmp(INSPECT_PDF, pcToken))
            {
                if(!ServerConf->inspect_response)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Enable '%s' before setting '%s'",INSPECT_RESPONSE, INSPECT_PDF);
                    return -1;
                }

                ProcessDecompressionTypeList(ServerConf,pcToken,ErrorString,ErrStrLen, saveptr);
            }
#endif
            else if(!strcmp(MAX_HDR_LENGTH, pcToken))
            {
                iRet = ProcessMaxHdrLen(ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if(!strcmp(MAX_HEADERS, pcToken))
            {
                iRet = ProcessMaxHeaders(ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if(!strcmp(MAX_SPACES, pcToken))
            {
                iRet = ProcessMaxSpaces(ServerConf, ErrorString, ErrStrLen, saveptr, MAX_SPACES, CONFIG_MAX_SPACES);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if(!strcmp(ENABLE_XFF, pcToken))
            {
                ServerConf->enable_xff = 1;
            }
            else if(!strcmp(XFF_HEADERS_TOK, pcToken))
            {
                if(!ServerConf->enable_xff)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                                  "Enable '%s' before setting '%s'",ENABLE_XFF, XFF_HEADERS_TOK);
                    return -1;
                }

                if( ((iRet = ProcessXFF_HeaderList(ServerConf, ErrorString, ErrStrLen, saveptr)) != 0)  )
                {
                    return iRet;
                }
            }
            else if(!strcmp(LOG_URI, pcToken))
            {
                ServerConf->log_uri = 1;
            }
            else if(!strcmp(LOG_HOSTNAME, pcToken))
            {
                ServerConf->log_hostname = 1;
            }
            else if(!strcmp(SMALL_CHUNK_LENGTH, pcToken))
            {
                iRet = ProcessSmallChunkLength(ServerConf,ErrorString,ErrStrLen, saveptr);
                if (iRet)
                {
                    return iRet;
                }
            }
            else if (!strcmp(LEGACY_MODE, pcToken))
            {
                pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
                if (pcToken == NULL)
                    break;

                if(!strcmp(BOOL_YES, pcToken))
                {
                    ServerConf->h2_mode = false;
                }
                else if(!strcmp(BOOL_NO, pcToken))
                {
                    ServerConf->h2_mode = true;
                }
                else
                {
                   continue;
                }
            }
            else
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                              "Invalid token while configuring the profile token.  "
                              "The only allowed tokens when configuring profiles "
                              "are: '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
                              "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
#ifdef FILE_DECOMP_SWF
"'%s', "
#endif
#ifdef FILE_DECOMP_PDF
"'%s', "
#endif
                              "'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', and '%s'. ",
                              PORTS,IIS_UNICODE_MAP, ALLOW_PROXY, FLOW_DEPTH,
                              CLIENT_FLOW_DEPTH, GLOBAL_ALERT, OVERSIZE_DIR, MAX_HDR_LENGTH,
                              INSPECT_URI_ONLY, INSPECT_COOKIES, INSPECT_RESPONSE,
                              EXTRACT_GZIP,MAX_HEADERS, NORMALIZE_COOKIES, ENABLE_XFF, XFF_HEADERS_TOK,
#ifdef FILE_DECOMP_SWF
INSPECT_SWF,
#endif
#ifdef FILE_DECOMP_PDF
INSPECT_PDF,
#endif
                              NORMALIZE_HEADERS, NORMALIZE_UTF, UNLIMIT_DECOMPRESS, HTTP_METHODS,
                              LOG_URI, LOG_HOSTNAME, MAX_SPACES, NORMALIZE_JS, MAX_JS_WS, LEGACY_MODE);

                return -1;
            }

        }  while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL);

        if(!iPorts)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "No port list to the profile token.");

            return -1;
        }

        return 0;
    }

    /*
    **  If there is no profile configuration then we go into the hard-core
    **  configuration.
    */

    hi_ui_config_reset_http_methods(ServerConf);

    do
    {
        if(!strcmp(PORTS, pcToken))
        {
            iRet = ProcessPorts(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(FLOW_DEPTH, pcToken) || !strcmp(SERVER_FLOW_DEPTH, pcToken))
        {
            iRet = ProcessFlowDepth(ServerConf, HI_SI_SERVER_MODE, ErrorString, ErrStrLen, saveptr, pcToken, MAX_SERVER_DEPTH);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(CLIENT_FLOW_DEPTH, pcToken))
        {
            iRet = ProcessFlowDepth(ServerConf, HI_SI_CLIENT_MODE, ErrorString, ErrStrLen, saveptr, pcToken, MAX_CLIENT_DEPTH);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(POST_DEPTH, pcToken))
        {
            iRet = ProcessPostDepth(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(IIS_UNICODE_MAP, pcToken))
        {
            iRet = ProcessIISUnicodeMap(&ServerConf->iis_unicode_map,
                                        &ServerConf->iis_unicode_map_filename,
                                        &ServerConf->iis_unicode_codepage,
                                        ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(CHUNK_LENGTH, pcToken))
        {
            iRet = ProcessChunkLength(ServerConf,ErrorString,ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(SMALL_CHUNK_LENGTH, pcToken))
        {
            iRet = ProcessSmallChunkLength(ServerConf,ErrorString,ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(PIPELINE, pcToken))
        {
            ServerConf->no_pipeline = 1;
        }
        else if(!strcmp(NON_STRICT, pcToken))
        {
            ServerConf->non_strict = 1;
        }
        else if(!strcmp(ALLOW_PROXY, pcToken))
        {
            ServerConf->allow_proxy = 1;
        }
        else if(!strcmp(GLOBAL_ALERT, pcToken))
        {
            ServerConf->no_alerts = 1;
        }
        else if(!strcmp(TAB_URI_DELIMITER, pcToken))
        {
            ServerConf->tab_uri_delimiter = 1;
        }
        else if(!strcmp(EXTENDED_ASCII, pcToken))
        {
            ServerConf->extended_ascii_uri = 1;
        }
        else if (!strcmp(NORMALIZE_HEADERS, pcToken))
        {
            ServerConf->normalize_headers = 1;
        }
        else if (!strcmp(NORMALIZE_COOKIES, pcToken))
        {
            ServerConf->normalize_cookies = 1;
        }
        else if (!strcmp(NORMALIZE_UTF, pcToken))
        {
            ServerConf->normalize_utf = 1;
        }
        else if (!strcmp(NORMALIZE_JS, pcToken))
        {
            if(!ServerConf->inspect_response)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Enable '%s' before setting '%s'", INSPECT_RESPONSE, NORMALIZE_JS);
                return -1;
            }
            ServerConf->normalize_javascript = 1;
        }
        else if(!strcmp(MAX_JS_WS, pcToken))
        {
            if(!ServerConf->normalize_javascript)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                                    "Enable '%s' before setting '%s'", NORMALIZE_JS, MAX_JS_WS);
                return -1;
            }
            iRet = ProcessMaxSpaces(ServerConf, ErrorString, ErrStrLen, saveptr, MAX_JS_WS, CONFIG_MAX_JS_WS);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(OVERSIZE_DIR, pcToken))
        {
            iRet = ProcessOversizeDir(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }

        }
        else if(!strcmp(INSPECT_URI_ONLY, pcToken))
        {
            ServerConf->uri_only = 1;
        }

        else if(!strcmp(INSPECT_COOKIES, pcToken))
        {
            ServerConf->enable_cookie = 1;
        }
        else if(!strcmp(INSPECT_RESPONSE, pcToken))
        {
            ServerConf->inspect_response = 1;
        }
        else if (!strcmp(HTTP_METHODS, pcToken))
        {
            iRet = ProcessHttpMethodList(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(EXTRACT_GZIP, pcToken))
        {
            if(!ServerConf->inspect_response)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Enable '%s' inspection before setting '%s'",INSPECT_RESPONSE, EXTRACT_GZIP);
                return -1;
            }

            ServerConf->extract_gzip = 1;
        }
        else if(!strcmp(UNLIMIT_DECOMPRESS, pcToken))
        {
            if(!ServerConf->extract_gzip)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Enable '%s' inspection before setting '%s'",EXTRACT_GZIP, UNLIMIT_DECOMPRESS);
                return -1;
            }
            if((GlobalConf->compr_depth != MAX_GZIP_DEPTH) && (GlobalConf->decompr_depth != MAX_GZIP_DEPTH))
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                    "'%s' and '%s' should be set to max in the default policy to enable '%s'",
                    COMPRESS_DEPTH, DECOMPRESS_DEPTH, UNLIMIT_DECOMPRESS);
                return -1;
            }

            GlobalConf->compr_depth = GlobalConf->decompr_depth = MAX_GZIP_DEPTH;
            ServerConf->unlimited_decompress = 1;
        }
#ifdef FILE_DECOMP_SWF
        else if (!strcmp(INSPECT_SWF, pcToken))
        {
            if(!ServerConf->inspect_response)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Enable '%s' before setting '%s'",INSPECT_RESPONSE, INSPECT_SWF);
                return -1;
            }

            ProcessDecompressionTypeList(ServerConf,pcToken,ErrorString,ErrStrLen, saveptr);
        }
#endif
#ifdef FILE_DECOMP_PDF
        else if (!strcmp(INSPECT_PDF, pcToken))
        {
            if(!ServerConf->inspect_response)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Enable '%s' before setting '%s'",INSPECT_RESPONSE, INSPECT_PDF);
                return -1;
            }

            ProcessDecompressionTypeList(ServerConf,pcToken,ErrorString,ErrStrLen, saveptr);
        }
#endif
        /*
        **  Start the CONF_OPT configurations.
        */
        else if(!strcmp(ASCII, pcToken))
        {
            ConfOpt = &ServerConf->ascii;
            iRet = ProcessConfOpt(ConfOpt, ASCII, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(UTF_8, pcToken))
        {
            /*
            **  In order for this to work we also need to set ASCII
            */
            ServerConf->ascii.on    = 1;

            ConfOpt = &ServerConf->utf_8;
            iRet = ProcessConfOpt(ConfOpt, UTF_8, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(IIS_UNICODE, pcToken))
        {
            if(ServerConf->iis_unicode_map == NULL)
            {
                ServerConf->iis_unicode_map = GlobalConf->iis_unicode_map;
            }

            /*
            **  We need to set up:
            **    - ASCII
            **    - DOUBLE_DECODE
            **    - U_ENCODE
            **    - BARE_BYTE
            **    - IIS_UNICODE
            **
            **     Base 36 is deprecated and essentially a noop
            **    - BASE36
            */
            ServerConf->ascii.on           = 1;

            ConfOpt = &ServerConf->iis_unicode;
            iRet = ProcessConfOpt(ConfOpt, IIS_UNICODE, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(DOUBLE_DECODE, pcToken))
        {
            ServerConf->ascii.on             = 1;

            ConfOpt = &ServerConf->double_decoding;
            iRet = ProcessConfOpt(ConfOpt, DOUBLE_DECODE, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(U_ENCODE, pcToken))
        {
            /*
            **  We set the unicode map to default if it's not already
            **  set.
            */
            if(ServerConf->iis_unicode_map == NULL)
            {
                ServerConf->iis_unicode_map = GlobalConf->iis_unicode_map;
            }

            ConfOpt = &ServerConf->u_encoding;
            iRet = ProcessConfOpt(ConfOpt, U_ENCODE, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(BARE_BYTE, pcToken))
        {
            ConfOpt = &ServerConf->bare_byte;
            iRet = ProcessConfOpt(ConfOpt, BARE_BYTE, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(BASE36, pcToken))
        {
            /* Base 36 is deprecated and essentially a noop */
            ErrorMessage("WARNING: %s (%d): The \"base36\" option to the "
                    "\"http_inspect\" preprocessor configuration is "
                    "deprecated and void of functionality.\n",
                    file_name, file_line);

            /* Need to get and chuck yes/no argument to option since
             * we're not doing anything with this anymore. */
            pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
        }
        else if(!strcmp(NON_RFC_CHAR, pcToken))
        {
            iRet = ProcessNonRfcChar(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(MULTI_SLASH, pcToken))
        {
            ConfOpt = &ServerConf->multiple_slash;
            iRet = ProcessConfOpt(ConfOpt, MULTI_SLASH, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(IIS_BACKSLASH, pcToken))
        {
            ConfOpt = &ServerConf->iis_backslash;
            iRet = ProcessConfOpt(ConfOpt, IIS_BACKSLASH, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(DIRECTORY, pcToken))
        {
            ConfOpt = &ServerConf->directory;
            iRet = ProcessConfOpt(ConfOpt, DIRECTORY, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(APACHE_WS, pcToken))
        {
            ConfOpt = &ServerConf->apache_whitespace;
            iRet = ProcessConfOpt(ConfOpt, APACHE_WS, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(WHITESPACE, pcToken))
        {
            iRet = ProcessWhitespaceChars(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
         else if(!strcmp(IIS_DELIMITER, pcToken))
        {
            ConfOpt = &ServerConf->iis_delimiter;
            iRet = ProcessConfOpt(ConfOpt, IIS_DELIMITER, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(WEBROOT, pcToken))
        {
            ConfOpt = &ServerConf->webroot;
            iRet = ProcessConfOpt(ConfOpt, WEBROOT, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(MAX_HDR_LENGTH, pcToken))
        {
            iRet = ProcessMaxHdrLen(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(MAX_HEADERS, pcToken))
        {
            iRet = ProcessMaxHeaders(ServerConf, ErrorString, ErrStrLen, saveptr);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(MAX_SPACES, pcToken))
        {
            iRet = ProcessMaxSpaces(ServerConf, ErrorString, ErrStrLen, saveptr, MAX_SPACES, CONFIG_MAX_SPACES);
            if (iRet)
            {
                return iRet;
            }
        }
        else if(!strcmp(ENABLE_XFF, pcToken))
        {
            ServerConf->enable_xff = 1;
        }
        else if(!strcmp(XFF_HEADERS_TOK, pcToken))
        {
            if(!ServerConf->enable_xff)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Enable '%s' before setting '%s'",ENABLE_XFF, XFF_HEADERS_TOK);
                return -1;
            }

            if( ((iRet = ProcessXFF_HeaderList(ServerConf, ErrorString, ErrStrLen, saveptr)) != 0)  )
            {
                return iRet;
            }
        }
        else if(!strcmp(LOG_URI, pcToken))
        {
            ServerConf->log_uri = 1;
        }
        else if(!strcmp(LOG_HOSTNAME, pcToken))
        {
            ServerConf->log_hostname = 1;
        }
        else if (!strcmp(LEGACY_MODE, pcToken))
        {
            pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
            if(pcToken ==  NULL)
                break;

            if(!strcmp(BOOL_YES, pcToken))
            {
                ServerConf->h2_mode = false;
            }
            else if(!strcmp(BOOL_NO, pcToken))
            {
                ServerConf->h2_mode = true;
            }
            else
            {
               continue;
            }
        }
        else
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Invalid keyword '%s' for server configuration.",
                          pcToken);

            return -1;
        }

    } while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL);

    return 0;
}

static void PrintFileDecompOpt(HTTPINSPECT_CONF *ServerConf)
{
    LogMessage("      Decompress response files: %s %s %s\n",
               ((ServerConf->file_decomp_modes & FILE_SWF_ZLIB_BIT) != 0) ? "SWF-ZLIB" : "",
               ((ServerConf->file_decomp_modes & FILE_SWF_LZMA_BIT) != 0) ? "SWF-LZMA" : "",
               ((ServerConf->file_decomp_modes & FILE_PDF_DEFL_BIT) != 0) ? "PDF-DEFL" : "");
}

static int PrintConfOpt(HTTPINSPECT_CONF_OPT *ConfOpt, char *Option)
{
    if(!ConfOpt || !Option)
    {
        return HI_INVALID_ARG;
    }

    if(ConfOpt->on)
    {
        LogMessage("      %s: YES alert: %s\n", Option,
               ConfOpt->alert ? "YES" : "NO");
    }
    else
    {
        LogMessage("      %s: OFF\n", Option);
    }

    return 0;
}

static int PrintServerConf(HTTPINSPECT_CONF *ServerConf)
{
    char buf[STD_BUF+1];
    int iCtr;
    int iChar = 0;
    char* paf = "";
    PROFILES prof;

    if(!ServerConf)
    {
        return HI_INVALID_ARG;
    }

    prof = ServerConf->profile;
    LogMessage("      Server profile: %s\n",
        prof==HI_ALL?"All":
        prof==HI_APACHE?"Apache":
        prof==HI_IIS?"IIS":
        prof==HI_IIS4?"IIS4":"IIS5");


    memset(buf, 0, STD_BUF+1);

    if ( ScPafEnabled() && stream_api )
        paf = " (PAF)";

    SnortSnprintf(buf, STD_BUF + 1, "      Ports%s: ", paf);

    /*
    **  Print out all the applicable ports.
    */
    for(iCtr = 0; iCtr < MAXPORTS; iCtr++)
    {
        if( isPortEnabled( ServerConf->ports, iCtr ) )
        {
            sfsnprintfappend(buf, STD_BUF, "%d ", iCtr);
        }
    }

    LogMessage("%s\n", buf);

    LogMessage("      Server Flow Depth: %d\n", ServerConf->server_flow_depth);
    LogMessage("      Client Flow Depth: %d\n", ServerConf->client_flow_depth);
    LogMessage("      Max Chunk Length: %d\n", ServerConf->chunk_length);
    if (ServerConf->small_chunk_length.size > 0)
        LogMessage("      Small Chunk Length Evasion: chunk size <= %u, threshold >= %u times\n",
                   ServerConf->small_chunk_length.size, ServerConf->small_chunk_length.num);
    LogMessage("      Max Header Field Length: %d\n", ServerConf->max_hdr_len);
    LogMessage("      Max Number Header Fields: %d\n", ServerConf->max_headers);
    LogMessage("      Max Number of WhiteSpaces allowed with header folding: %d\n", ServerConf->max_spaces);
    LogMessage("      Inspect Pipeline Requests: %s\n",
               ServerConf->no_pipeline ? "NO" : "YES");
    LogMessage("      URI Discovery Strict Mode: %s\n",
               ServerConf->non_strict ? "NO" : "YES");
    LogMessage("      Allow Proxy Usage: %s\n",
               ServerConf->allow_proxy ? "YES" : "NO");
    LogMessage("      Disable Alerting: %s\n",
               ServerConf->no_alerts ? "YES":"NO");
    LogMessage("      Oversize Dir Length: %d\n",
               ServerConf->long_dir);
    LogMessage("      Only inspect URI: %s\n",
               ServerConf->uri_only ? "YES" : "NO");
    LogMessage("      Normalize HTTP Headers: %s\n",
               ServerConf->normalize_headers ? "YES" : "NO");
    LogMessage("      Inspect HTTP Cookies: %s\n",
               ServerConf->enable_cookie ? "YES" : "NO");
    LogMessage("      Inspect HTTP Responses: %s\n",
               ServerConf->inspect_response ? "YES" : "NO");
    LogMessage("      Extract Gzip from responses: %s\n",
               ServerConf->extract_gzip ? "YES" : "NO");
    PrintFileDecompOpt(ServerConf);
    LogMessage("      Unlimited decompression of gzip data from responses: %s\n",
                ServerConf->unlimited_decompress ? "YES" : "NO");
    LogMessage("      Normalize Javascripts in HTTP Responses: %s\n",
                       ServerConf->normalize_javascript ? "YES" : "NO");
    if(ServerConf->normalize_javascript)
    {
        if(ServerConf->max_js_ws)
            LogMessage("      Max Number of WhiteSpaces allowed with Javascript Obfuscation in HTTP responses: %d\n", ServerConf->max_js_ws);
    }
    LogMessage("      Normalize HTTP Cookies: %s\n",
               ServerConf->normalize_cookies ? "YES" : "NO");
    LogMessage("      Enable XFF and True Client IP: %s\n",
               ServerConf->enable_xff ? "YES"  :  "NO");
    LogMessage("      Log HTTP URI data: %s\n",
               ServerConf->log_uri ? "YES"  :  "NO");
    LogMessage("      Log HTTP Hostname data: %s\n",
               ServerConf->log_hostname ? "YES"  :  "NO");
    LogMessage("      Extended ASCII code support in URI: %s\n",
               ServerConf->extended_ascii_uri ? "YES" : "NO");


    PrintConfOpt(&ServerConf->ascii, "Ascii");
    PrintConfOpt(&ServerConf->double_decoding, "Double Decoding");
    PrintConfOpt(&ServerConf->u_encoding, "%U Encoding");
    PrintConfOpt(&ServerConf->bare_byte, "Bare Byte");
    PrintConfOpt(&ServerConf->utf_8, "UTF 8");
    PrintConfOpt(&ServerConf->iis_unicode, "IIS Unicode");
    PrintConfOpt(&ServerConf->multiple_slash, "Multiple Slash");
    PrintConfOpt(&ServerConf->iis_backslash, "IIS Backslash");
    PrintConfOpt(&ServerConf->directory, "Directory Traversal");
    PrintConfOpt(&ServerConf->webroot, "Web Root Traversal");
    PrintConfOpt(&ServerConf->apache_whitespace, "Apache WhiteSpace");
    PrintConfOpt(&ServerConf->iis_delimiter, "IIS Delimiter");

    if(ServerConf->iis_unicode_map_filename)
    {
        LogMessage("      IIS Unicode Map Filename: %s\n",
                   ServerConf->iis_unicode_map_filename);
        LogMessage("      IIS Unicode Map Codepage: %d\n",
                   ServerConf->iis_unicode_codepage);
    }
    else if(ServerConf->iis_unicode_map)
    {
        LogMessage("      IIS Unicode Map: "
                   "GLOBAL IIS UNICODE MAP CONFIG\n");
    }
    else
    {
        LogMessage("      IIS Unicode Map:  NOT CONFIGURED\n");
    }

    /*
    **  Print out the non-rfc chars
    */
    memset(buf, 0, STD_BUF+1);
    SnortSnprintf(buf, STD_BUF + 1, "      Non-RFC Compliant Characters: ");
    for(iCtr = 0; iCtr < 256; iCtr++)
    {
        if(ServerConf->non_rfc_chars[iCtr])
        {
            sfsnprintfappend(buf, STD_BUF, "0x%.2x ", (u_char)iCtr);
            iChar = 1;
        }
    }

    if(!iChar)
    {
        sfsnprintfappend(buf, STD_BUF, "NONE");
    }

    LogMessage("%s\n", buf);

    /*
    **  Print out the whitespace chars
    */
    iChar = 0;
    memset(buf, 0, STD_BUF+1);
    SnortSnprintf(buf, STD_BUF + 1, "      Whitespace Characters: ");
    for(iCtr = 0; iCtr < 256; iCtr++)
    {
        if(ServerConf->whitespace[iCtr])
        {
            sfsnprintfappend(buf, STD_BUF, "0x%.2x ", (u_char)iCtr);
            iChar = 1;
        }
    }

    if(!iChar)
    {
        sfsnprintfappend(buf, STD_BUF, "NONE");
    }

    LogMessage("%s\n", buf);

    LogMessage("      Legacy mode: %s\n",
               ServerConf->h2_mode ? "YES" : "NO");

    return 0;
}

static void registerPortsWithStream( HTTPINSPECT_CONF *policy, char *network )
{
    uint32_t port;
    uint32_t dir = 0;

    if( policy->client_flow_depth > -1 )
        dir |= SSN_DIR_FROM_CLIENT;
    if( ( policy->server_extract_size > -1 ) )
        dir |= SSN_DIR_FROM_SERVER;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            stream_api->register_reassembly_port( network, port, dir );
    }
}

static void enableHiForConfiguredPorts( struct _SnortConfig *sc, HTTPINSPECT_CONF *policy )
{
    int port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            session_api->enable_preproc_for_port( sc, PP_HTTPINSPECT, PROTO_BIT__TCP, port );
    }
}

int ProcessUniqueServerConf(struct _SnortConfig *sc, HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                            char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pIpAddressList = NULL;
    char *pIpAddressList2 = NULL;
    char *brkt = NULL;
    char firstIpAddress = 1;
    sfcidr_t Ip;
    HTTPINSPECT_CONF *ServerConf = NULL;
    int iRet;
    int retVal = -1;

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        SnortSnprintf(ErrorString, ErrStrLen,
                      "No arguments to '%s' token.", SERVER);

        retVal = -1;
        goto _return;
    }

    /*
    **  Check for the default configuration first
    */
    if (strcasecmp(SERVER_DEFAULT, pcToken) == 0)
    {
        if (GlobalConf->global_server != NULL)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                          "Cannot configure '%s' settings more than once.",
                          GLOBAL_SERVER);

            goto _return;
        }

        GlobalConf->global_server =
            (HTTPINSPECT_CONF *)SnortPreprocAlloc(1, sizeof(HTTPINSPECT_CONF),
                                     PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);

        ServerConf = GlobalConf->global_server;

        iRet = hi_ui_config_default(ServerConf);
        if (iRet)
        {
            snprintf(ErrorString, ErrStrLen,
                     "Error configuring default global configuration.");
            return -1;
        }

        iRet = ProcessServerConf(GlobalConf, ServerConf, ErrorString, ErrStrLen, saveptr);
        if (iRet)
        {
            retVal =  iRet;
            goto _return;
        }

        // register enabled ports for reassembly with Stream for the default
        registerPortsWithStream( ServerConf, NULL );
        enableHiForConfiguredPorts( sc, ServerConf );

        /*
        **  Start writing out the Default Server Config
        */
        LogMessage("    DEFAULT SERVER CONFIG:\n");
    }
    else
    {
        /*
        **  Convert string to IP address
        */
        /* get the first delimiter*/
        if(strcmp(START_IPADDR_LIST, pcToken) == 0)
        {
            /*list begin token matched*/
            if ((pIpAddressList = strtok_r(NULL, END_IPADDR_LIST, saveptr)) == NULL)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Invalid IP Address list in '%s' token.", SERVER);

                goto _return;
            }
        }
        else
        {
            /*list begin didn't match so this must be an IP address*/
            pIpAddressList = pcToken;
        }


        pIpAddressList2 = strdup(pIpAddressList);
        if (!pIpAddressList2)
        {
            SnortSnprintf(ErrorString, ErrStrLen,
                    "Could not allocate memory for server configuration.");

            goto _return;
        }



        for (pcToken = strtok_r(pIpAddressList, CONF_SEPARATORS, &brkt);
             pcToken;
             pcToken = strtok_r(NULL, CONF_SEPARATORS, &brkt))
        {

            if (sfip_pton(pcToken, &Ip) != SFIP_SUCCESS)
            {
                SnortSnprintf(ErrorString, ErrStrLen,
                        "Invalid IP to '%s' token.", SERVER);

                goto _return;
            }

            /*
             **  allocate the memory for the server configuration
             */
            if (firstIpAddress)
            {
                ServerConf = (HTTPINSPECT_CONF *)SnortPreprocAlloc(1, sizeof(HTTPINSPECT_CONF),
                                                      PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);
                if(!ServerConf)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Could not allocate memory for server configuration.");

                    goto _return;
                }

                iRet = ProcessServerConf(GlobalConf, ServerConf, ErrorString, ErrStrLen, saveptr);
                if (iRet)
                {
                    retVal = iRet;
                    goto _return;
                }
            }

            iRet = hi_ui_config_add_server(GlobalConf, &Ip, ServerConf);
            if (iRet)
            {
                /*
                 **  Check for already added servers
                 */
                if(iRet == HI_NONFATAL_ERR)
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Duplicate server configuration.");

                    goto _return;
                }
                else
                {
                    SnortSnprintf(ErrorString, ErrStrLen,
                            "Error when adding server configuration.");

                    goto _return;
                }
            }

            // register enabled ports for reassembly with Stream for the current netowrk
            registerPortsWithStream( ServerConf, pcToken );
            enableHiForConfiguredPorts( sc, ServerConf );

            if (firstIpAddress)
            {
                //process the first IP address as usual
                firstIpAddress = 0;
            }

            //create a reference
            ServerConf->referenceCount++;

        }

        if (firstIpAddress)
        {
            //no IP address was found
            SnortSnprintf(ErrorString, ErrStrLen,
                    "Invalid IP Address list in '%s' token.", SERVER);

            goto _return;
        }

        /*
        **  Print out the configuration header
        */
        LogMessage("    SERVER: %s\n", pIpAddressList2);
    }

    /*
    **  Finish printing out the server configuration
    */
    PrintServerConf(ServerConf);

    retVal = 0;

_return:
    if (pIpAddressList2)
    {
        free(pIpAddressList2);
    }
    return retVal;
}

int PrintGlobalConf(HTTPINSPECT_GLOBAL_CONF *GlobalConf)
{
    LogMessage("HttpInspect Config:\n");

    LogMessage("    GLOBAL CONFIG\n");
    if(GlobalConf->disabled)
    {
        LogMessage("      Http Inspect: INACTIVE\n");
        LogMessage("      Max Gzip Memory: %d\n",
                                GlobalConf->max_gzip_mem);
        LogMessage("      Memcap used for logging URI and Hostname: %u\n",
                                GlobalConf->memcap);
        return 0;
    }
    LogMessage("      Detect Proxy Usage:       %s\n",
               GlobalConf->proxy_alert ? "YES" : "NO");
    LogMessage("      IIS Unicode Map Filename: %s\n",
               GlobalConf->iis_unicode_map_filename);
    LogMessage("      IIS Unicode Map Codepage: %d\n",
               GlobalConf->iis_unicode_codepage);
    LogMessage("      Memcap used for logging URI and Hostname: %u\n",
               GlobalConf->memcap);
    LogMessage("      Max Gzip Memory: %d\n",
                GlobalConf->max_gzip_mem);
    LogMessage("      Max Gzip Sessions: %d\n",
               GlobalConf->max_gzip_sessions);
    LogMessage("      Gzip Compress Depth: %d\n",
               GlobalConf->compr_depth);
    LogMessage("      Gzip Decompress Depth: %d\n",
               GlobalConf->decompr_depth);
    LogMessage("      Normalize Random Nulls in Text: %s\n",
               GlobalConf->normalize_nulls ? "YES" : "NO");

    return 0;
}

/*
**  NAME
**    LogEvents::
*/
/**
**  This is the routine that logs HttpInspect alerts through Snort.
**
**  Every Session gets looked at for any logged events, and if there are
**  events to be logged then we select the one with the highest priority.
**
**  We use a generic event structure that we set for each different event
**  structure.  This way we can use the same code for event logging regardless
**  of what type of event strucure we are dealing with.
**
**  The important things to know about this function is how to work with
**  the event queue.  The number of unique events is contained in the
**  stack_count variable.  So we loop through all the unique events and
**  find which one has the highest priority.  During this loop, we also
**  re-initialize the individual event counts for the next iteration, saving
**  us time in a separate initialization phase.
**
**  After we've iterated through all the events and found the one with the
**  highest priority, we then log that event through snort.
**
**  We've mapped the HttpInspect and the Snort alert IDs together, so we
**  can access them directly instead of having a more complex mapping
**  function.  It's the only good way to do this.
**
**  @param Session          pointer to Session construct
**  @param p                pointer to the Snort packet construct
**  @param iInspectMode     inspection mode to take event queue from
**
**  @return integer
**
**  @retval 0 this function only return success
*/
static inline int LogEvents(HI_SESSION *hi_ssn, Packet *p,
        int iInspectMode, HttpSessionData *hsd)
{
    HI_GEN_EVENTS GenEvents;
    HI_EVENT      *OrigEvent;
    HI_EVENT      *HiEvent = NULL;
    uint64_t      uiMask = 0;
    int           iGenerator;
    int           iStackCnt;
    uint64_t      iEvent;
    int           iCtr;

    /*
    **  Set the session ptr, if applicable
    */
    if(iInspectMode == HI_SI_CLIENT_MODE)
    {
        GenEvents.stack =       hi_ssn->client.event_list.stack;
        GenEvents.stack_count = &(hi_ssn->client.event_list.stack_count);
        GenEvents.events =      hi_ssn->client.event_list.events;

        iGenerator = GENERATOR_SPP_HTTP_INSPECT_CLIENT;
    }
    else if(iInspectMode == HI_SI_SERVER_MODE)
    {
        GenEvents.stack =       hi_ssn->server.event_list.stack;
        GenEvents.stack_count = &(hi_ssn->server.event_list.stack_count);
        GenEvents.events =      hi_ssn->server.event_list.events;

        iGenerator = GENERATOR_SPP_HTTP_INSPECT;
    }
    else
    {
        GenEvents.stack =       hi_ssn->anom_server.event_list.stack;
        GenEvents.stack_count = &(hi_ssn->anom_server.event_list.stack_count);
        GenEvents.events =      hi_ssn->anom_server.event_list.events;

        iGenerator = GENERATOR_SPP_HTTP_INSPECT;
    }

    /*
    **  Now starts the generic event processing
    */
    iStackCnt = *(GenEvents.stack_count);

    /*
    **  IMPORTANT::
    **  We have to check the stack count of the event queue before we process
    **  an log.
    */
    if(iStackCnt == 0)
    {
        return 0;
    }

    /*
    **  Cycle through the events and select the event with the highest
    **  priority.
    */
    for(iCtr = 0; iCtr < iStackCnt; iCtr++)
    {
        iEvent = (uint64_t)GenEvents.stack[iCtr];
        OrigEvent = &(GenEvents.events[iEvent]);

        /*
        **  Set the event to start off the comparison
        */
        if(!HiEvent)
        {
            HiEvent = OrigEvent;
        }

        /*
        **  This is our "comparison function".  Log the event with the highest
        **  priority.
        */
        if(OrigEvent->event_info->priority < HiEvent->event_info->priority)
        {
            HiEvent = OrigEvent;
        }

        /*
        **  IMPORTANT:
        **    This is how we reset the events in the event queue.
        **    If you miss this step, you can be really screwed.
        */
        OrigEvent->count = 0;
    }

    /*
    **  We use the iEvent+1 because the event IDs between snort and
    **  HttpInspect are mapped off-by-one.  Don't ask why, drink Bud
    **  Dry . . . They're mapped off-by one because in the internal
    **  HttpInspect queue, events are mapped starting at 0.  For some
    **  reason, it appears that the first event can't be zero, so we
    **  use the internal value and add one for snort.
    */
    iEvent = (uint64_t)HiEvent->event_info->alert_id + 1;

    uiMask = (uint64_t)1 << (iEvent & 63);

    if (hsd != NULL)
    {
        /* We've already logged this event for this session,
         * don't log it again */
        if (hsd->event_flags & uiMask)
            return 0;
        hsd->event_flags |= uiMask;
    }

    SnortEventqAdd(iGenerator, iEvent, 1, 0, 3, HiEvent->event_info->alert_str,0);

    /*
    **  Reset the event queue stack counter, in the case of pipelined
    **  requests.
    */
    *(GenEvents.stack_count) = 0;

    return 0;
}

static inline int SetSiInput(HI_SI_INPUT *SiInput, Packet *p)
{
    IP_COPY_VALUE(SiInput->sip, GET_SRC_IP(p));
    IP_COPY_VALUE(SiInput->dip, GET_DST_IP(p));
    SiInput->sport = p->sp;
    SiInput->dport = p->dp;

    /*
    **  We now set the packet direction
    */
    if(p->ssnptr &&
            session_api->get_session_flags(p->ssnptr) & SSNFLAG_MIDSTREAM)
    {
        SiInput->pdir = HI_SI_NO_MODE;
    }
    else if(p->packet_flags & PKT_FROM_SERVER)
    {
        SiInput->pdir = HI_SI_SERVER_MODE;
    }
    else if(p->packet_flags & PKT_FROM_CLIENT)
    {
        SiInput->pdir = HI_SI_CLIENT_MODE;
    }
    else
    {
        SiInput->pdir = HI_SI_NO_MODE;
    }

    return HI_SUCCESS;

}

static inline void ApplyClientFlowDepth (Packet* p, int flow_depth)
{
    switch (flow_depth)
    {
    case -1:
        // Inspect none of the client if there is normalized/extracted
        // URI/Method/Header/Body data */
        SetDetectLimit(p, 0);
        break;

    case 0:
        // Inspect all of the client, even if there is normalized/extracted
        // URI/Method/Header/Body data */
        /* XXX: HUGE performance hit here */
        SetDetectLimit(p, p->dsize);
        break;

    default:
        // Limit inspection of the client, even if there is normalized/extracted
        // URI/Method/Header/Body data */
        /* XXX: Potential performance hit here */
        if (flow_depth < p->dsize)
        {
            SetDetectLimit(p, flow_depth);
        }
        else
        {
            SetDetectLimit(p, p->dsize);
        }
        break;
    }
}

// FIXTHIS extra data masks should only be updated as extra data changes state
// eg just once when captured; this function is called on every packet and
// repeatedly sets the flags on session
static inline void HttpLogFuncs(HTTPINSPECT_GLOBAL_CONF *GlobalConf, HttpSessionData *hsd, Packet *p, int iCallDetect )
{
    if(!hsd)
        return;

    /* for pipelined HTTP requests */
    if ( !iCallDetect )
        stream_api->clear_extra_data(p->ssnptr, p, 0);

    if ( hsd->tList_start != NULL )
    {
        if( hsd->tList_start->tID == hsd->http_resp_id || hsd->tList_end->tID == hsd->http_req_id )
        {
           if(!(p->packet_flags & PKT_STREAM_INSERT) && !(p->packet_flags & PKT_REBUILT_STREAM))
               SetExtraData(p, GlobalConf->xtra_trueip_id);
           else
               stream_api->set_extra_data(p->ssnptr, p, GlobalConf->xtra_trueip_id);
        }
    }

    if(hsd->log_flags & HTTP_LOG_URI)
    {
        stream_api->set_extra_data(p->ssnptr, p, GlobalConf->xtra_uri_id);
    }

    if(hsd->log_flags & HTTP_LOG_HOSTNAME)
    {
        stream_api->set_extra_data(p->ssnptr, p, GlobalConf->xtra_hname_id);
    }

#ifndef SOURCEFIRE
    if(hsd->log_flags & HTTP_LOG_JSNORM_DATA)
    {
        SetExtraData(p, GlobalConf->xtra_jsnorm_id);
    }
    if(hsd->log_flags & HTTP_LOG_GZIP_DATA)
    {
        SetExtraData(p, GlobalConf->xtra_gzip_id);
    }
#endif
}

static inline void setFileName(Packet *p)
{
    uint8_t *buf = NULL;
    uint32_t len = 0;
    uint32_t type = 0;
    GetHttpUriData(p->ssnptr, &buf, &len, &type);
    file_api->set_file_name (p->ssnptr, buf, len, false);
}


static inline bool rfc_2616_token(u_char c)
{
    return isalpha(c) || isdigit(c) ||
        c == '!' || c == '#' || c == '$' || c == '%' ||
        c == '&' || c == '\'' || c == '*' || c == '+' ||
        c == '-' || c == '.' || c == '^' || c == '_' ||
        c == '`' || c == '|' || c == '~';
}

static inline bool rfc5987_attr_char(u_char c)
{
    return rfc_2616_token(c) && (( c != '*')|| (c !='\'') || (c !='%'));
}


/* Extract the filename from the content-dispostion header- RFC6266 */
static inline int extract_file_name(u_char *start, int length, u_char **fname_ptr, uint8_t *charset)
{
    u_char *cur = start, *tmp = NULL;
    u_char *end = start+length;
    u_char *fname_begin = NULL;
    u_char *fname_end = NULL;
    bool char_set = false;
    enum {
        CD_STATE_START,
        CD_STATE_BEFORE_VAL,
        CD_STATE_VAL,
        CD_STATE_QUOTED_VAL,
        CD_STATE_BEFORE_EXT_VAL,
        CD_STATE_CHARSET,
        CD_STATE_LANGUAGE,
        CD_STATE_EXT_VAL,
        CD_STATE_FINAL
    };
    const char *cd_file1 = "filename";
    const char *cd_file2 = "filename*";

    if (length <= 0)
	    return -1;
    uint8_t state = CD_STATE_START;

    while(cur < end)
    {
        switch(state)
        {
            case CD_STATE_START:
                {
                    if( (tmp = (u_char *)SnortStrcasestr((const char *)cur, end-cur, cd_file2)))
                    {
                        state = CD_STATE_BEFORE_EXT_VAL;
                        cur = tmp + strlen(cd_file2)-1;
                    }
                    else if( (tmp = (u_char *)SnortStrcasestr((const char *)cur, end-cur, cd_file1)))
                    {
                        state = CD_STATE_BEFORE_VAL;
                        cur = tmp + strlen(cd_file1)-1;
                    }
                    else
                        return -1;
                }
                break;
            case CD_STATE_BEFORE_VAL:
                {
                    if(*cur == '=')
                        state = CD_STATE_VAL;
                    else if( *cur != ' ')
                        state = CD_STATE_START;
                }
                break;
            case CD_STATE_VAL:
                {
                    if( !fname_begin && *cur == '"')
                        state = CD_STATE_QUOTED_VAL;
                    else if(rfc_2616_token(*cur))
                    {
                        if(!fname_begin)
                            fname_begin = cur;
                    }
                    else if(*cur == ';' || *cur == '\r' || *cur == '\n' || *cur == ' ' || *cur == '\t')
                    {
                        if(fname_begin)
                        {
                            fname_end = cur - 1;
                            state =  CD_STATE_FINAL;
                        }
                    }
                    else
                       return -1;
                }
                break;
            case CD_STATE_QUOTED_VAL:
                {
                    if(!fname_begin)
                        fname_begin = cur;
                    if(*cur == '"' )
                    {
                        fname_end = cur;
                        state =  CD_STATE_FINAL;
                    }
                }
                break;

            case CD_STATE_BEFORE_EXT_VAL:
                {
                    if(*cur == '=')
                        state = CD_STATE_CHARSET;
                    else if( *cur != ' ')
                        state = CD_STATE_START;
                }
                break;
            case CD_STATE_CHARSET:
                {
                    if( *cur == '\'')
                    {
                        if(!char_set)
                            return -1;
                        else
                            state = CD_STATE_LANGUAGE;
                    }
                    else if(!char_set)
                    {
                        /* Ignore space before the ext-value */
                        while(cur < end && *cur == ' ' )
                            cur++;
                        if(cur < end)
                        {
                            if(!strncasecmp((const char*)cur,"UTF-8",5))
                            {
                                *charset = CD_CHARSET_UTF8;
                                cur += 5;
                            }
                            else if(!strncasecmp((const char *)cur,"ISO-8859-1",10))
                            {
                                *charset = CD_CHARSET_ISO_9959_1;
                                cur += 10;
                            }
                            else if(!strncasecmp((const char *)cur,"mime-charset",12))
                            {
                                *charset = CD_CHARSET_MIME;
                                cur+=12;
                            }
                            else
                                return -1;
                            char_set = true;
                            continue;
                        }
                    }
                    else
                        return -1;
                }
                break;
            case CD_STATE_LANGUAGE:
                {
                    if(*cur == '\'')
                        state = CD_STATE_EXT_VAL;
                }
                break;
            case CD_STATE_EXT_VAL:
                {
                    if(!rfc5987_attr_char(*cur))
                    {
                        if(*cur == '%')
                        {
                            //Percent encoded, check if the next two digits are hex
                            if(!fname_begin)
                                fname_begin = cur;
                            if( !(cur+2 < end && isxdigit(*++cur) && isxdigit(*++cur)))
                                return -1;
                        }
                        else if(*cur == ';' || *cur == '\r' || *cur == '\n' || *cur == ' ' || *cur == '\t')
                        {
                            fname_end = cur;
                            state = CD_STATE_FINAL;
                        }
                    }
                    else
                    {
                         if(!fname_begin)
                             fname_begin = cur;
                    }
                }
                break;
            case CD_STATE_FINAL:
                {
                    if(fname_begin && fname_end)
                    {
                        *fname_ptr = fname_begin;
                        return fname_end-fname_begin;
                    }
                }
            default:
                return -1;
        }
        cur++;
    }
    switch(state)
    {
        case CD_STATE_FINAL:
        case CD_STATE_VAL:
        case CD_STATE_EXT_VAL:
        {
            if(fname_begin)
            {
                *fname_ptr = fname_begin;
                 if(!fname_end)
                     fname_end = end;
                 return fname_end-fname_begin;
            }
        }
    }
    return -1;
}

static bool set_file_name_cd_header(u_char *start, u_char *end, void *ssn)
{
    uint8_t unfold_buf[DECODE_BLEN] = {0};
    uint32_t unfold_size =0;
    int num_spaces = 0;
    u_char *p = start;
    u_char *fname = NULL;
    int len = 0;
    uint8_t charset = 0;

    sf_unfold_header(p, end-p, unfold_buf, sizeof(unfold_buf), &unfold_size, 0 , &num_spaces);
    if(!unfold_size)
        return false;

    if((len = extract_file_name(unfold_buf, unfold_size, &fname, &charset)) > 0 )
    {
        //Strip the size to 255 if bigger
        file_api->set_file_name(ssn, fname, len > 255 ? 255:len, true);
        return true;
    }
    return false;
}

static inline bool is_boundary_present(const u_char *start, const u_char *end)
{
    uint8_t unfold_buf[DECODE_BLEN] = {0};
    uint32_t unfold_size =0;
    int num_spaces = 0;
    const u_char *p = start;
    const char  *BOUNDARY_STR = "boundary=";

    sf_unfold_header(p, end-p, unfold_buf, sizeof(unfold_buf), &unfold_size, 0 , &num_spaces);
    if(!unfold_size)
        return false;

    return SnortStrcasestr((const char *)unfold_buf, unfold_size, BOUNDARY_STR);
     
}

static inline int processPostFileData(HTTPINSPECT_GLOBAL_CONF *GlobalConf, Packet *p, HI_SESSION *Session, HttpSessionData *hsd)
{
    const u_char *start = Session->client.request.content_type;
    const u_char *end = (Session->client.request.post_raw + Session->client.request.post_raw_size);

    if ( !PacketHasPAFPayload(p) || (p->packet_flags & PKT_PSEUDO_FLUSH))
        return 0;

    if ( hsd && start && is_boundary_present(start, end))
    {
        /* mime parsing
         * mime boundary should be processed before this
         */
        if (!hsd->mime_ssn)
        {
            hsd->mime_ssn = (MimeState *)SnortPreprocAlloc(1, sizeof(MimeState),
                                              PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);
            if (!hsd->mime_ssn)
                return -1;
            hsd->mime_ssn->log_config = &(GlobalConf->mime_conf);
            hsd->mime_ssn->decode_conf = &(GlobalConf->decode_conf);
            hsd->mime_ssn->mime_mempool = mime_decode_mempool;
            hsd->mime_ssn->log_mempool = mime_log_mempool;
            /*Set log buffers per session*/
            if (file_api->set_log_buffers(&(hsd->mime_ssn->log_state),
                    hsd->mime_ssn->log_config, hsd->mime_ssn->log_mempool, p->ssnptr, PP_HTTPINSPECT) < 0)
            {
                return -1;
            }
        }
        else
        {
            file_api->reset_mime_paf_state(&(hsd->mime_ssn->mime_boundary));
        }

        file_api->process_mime_data(p, start, end, hsd->mime_ssn, 1, false,"HTTP", PP_HTTPINSPECT);
    }
    else
    {
        if (file_api->file_process(p,(uint8_t *)Session->client.request.post_raw,
                (uint16_t)Session->client.request.post_raw_size,
                    file_api->get_file_position(p), true, false, false))
        {
            if( Session->client.request.content_disp )
            {
                if(!set_file_name_cd_header((u_char *)Session->client.request.content_disp,(u_char *)end, p->ssnptr))
                {
                    setFileName(p);
                }
            }
            else
            {
                setFileName(p);
            }
        }
    }
    return 0;
}
static inline void processFileData(Packet *p, HttpSessionData *hsd, bool *fileProcessed)
{
    if (*fileProcessed || !PacketHasPAFPayload(p))
        return;

    if (hsd->mime_ssn)
    {
        uint8_t *end = ( uint8_t *)(p->data) + p->dsize;
        file_api->process_mime_data(p, p->data, end, hsd->mime_ssn, 1, false, "HTTP", PP_HTTPINSPECT);
        *fileProcessed = true;
    }
    else if (file_api->get_file_processed_size(p->ssnptr) >0)
    {
        file_api->file_process(p, (uint8_t *)p->data, p->dsize, file_api->get_file_position(p), true, false, false);
        *fileProcessed = true;
    }
}

static inline int get_file_current_position(Packet *p,bool decomp_more,bool is_first)
{
    int file_data_position = SNORT_FILE_POSITION_UNKNOWN;
    uint64_t processed_size = file_api->get_file_processed_size(p->ssnptr);

    if(decomp_more)
    {
        if(is_first)
        {
            if(PacketHasStartOfPDU(p))
                file_data_position = SNORT_FILE_START;
            else if(processed_size)
                file_data_position = SNORT_FILE_MIDDLE;
        }
        else
        {
            if(processed_size)
                file_data_position = SNORT_FILE_MIDDLE;
        }
    }
    else
    {
        if(is_first)
        {
            file_data_position = file_api->get_file_position(p);
        }
        else
        {
            if(p->packet_flags & PKT_PDU_TAIL)
                file_data_position = SNORT_FILE_END;
            else if(processed_size)
                file_data_position = SNORT_FILE_MIDDLE;
        }
    }
    return file_data_position;
}

char *convert_range_flag_to_str(uint16_t range_flag)
{
    switch (range_flag)
    {
        case HTTP_RESP_RANGE_NONE:
            return "Range None";
        case RANGE_WITH_RESP_FULL_CONTENT:
            return "Full Content";
        case RANGE_WITH_RESP_PARTIAL_CONTENT:
            return "Partial Content";
        case RANGE_WITH_RESP_ERROR:
            return "Error in Range Field";
        case RANGE_WITH_RESP_NON_BYTE:
            return "Non-Byte unit";
        case RANGE_WITH_UNKNOWN_CONTENT_RANGE:
            return "Unknown Range Content";
        case RANGE_WITH_RESP_UNKNOWN_CONTENT_SIZE:
            return "Unknown Range Content Length";
        default:
            return "Skip Range";
    }
}

/*
**  NAME
**    SnortHttpInspect::
*/
/**
**  This function calls the HttpInspect function that processes an HTTP
**  session.
**
**  We need to instantiate a pointer for the HI_SESSION that HttpInspect
**  fills in.  Right now stateless processing fills in this session, which
**  we then normalize, and eventually detect.  We'll have to handle
**  separately the normalization events, etc.
**
**  This function is where we can see from the highest level what the
**  HttpInspect flow looks like.
**
**  @param GlobalConf pointer to the global configuration
**  @param p          pointer to the Packet structure
**
**  @return integer
**
**  @retval  0 function successful
**  @retval <0 fatal error
**  @retval >0 non-fatal error
*/
#define HTTP_BUF_URI_FLAG           0x01
#define HTTP_BUF_HEADER_FLAG        0x02
#define HTTP_BUF_CLIENT_BODY_FLAG   0x04
#define HTTP_BUF_METHOD_FLAG        0x08
#define HTTP_BUF_COOKIE_FLAG        0x10
#define HTTP_BUF_STAT_CODE          0x20
#define HTTP_BUF_STAT_MSG           0x40
int SnortHttpInspect(HTTPINSPECT_GLOBAL_CONF *GlobalConf, Packet *p)
{
    HI_SESSION  *Session;
    HI_SI_INPUT SiInput;
    int iInspectMode = 0;
    int iRet;
    int iCallDetect = 1;
    HttpSessionData *hsd = NULL;
    bool fileProcessed = false;
    bool is_first = true;

    if (stream_api && stream_api->is_session_http2(p->ssnptr)
        && !(p->packet_flags & PKT_REBUILT_STREAM)
        && !(p->packet_flags & PKT_PDU_TAIL))
    {
        return 0;
    }

    PROFILE_VARS;

    if(!(stream_api->get_preproc_flags(p->ssnptr) & PP_HTTPINSPECT_PAF_FLUSH_POST_HDR))
        hi_stats.total++;

    /*
    **  Set up the HI_SI_INPUT pointer.  This is what the session_inspection()
    **  routines use to determine client and server traffic.  Plus, this makes
    **  the HttpInspect library very independent from snort.
    */
    SetSiInput(&SiInput, p);

    /*
    **  HTTPINSPECT PACKET FLOW::
    **
    **  Session Inspection Module::
    **    The Session Inspection Module retrieves the appropriate server
    **    configuration for sessions, and takes care of the stateless
    **    vs. stateful processing in order to do this.  Once this module
    **    does it's magic, we're ready for the primetime.
    **
    **  HTTP Inspection Module::
    **    This isn't really a module in HttpInspect, but more of a helper
    **    function that sends the data to the appropriate inspection
    **    routine (client, server, anomalous server detection).
    **
    **  HTTP Normalization Module::
    **    This is where we normalize the data from the HTTP Inspection
    **    Module.  The Normalization module handles what type of normalization
    **    to do (client, server).
    **
    **  HTTP Detection Module::
    **    This isn't being used in the first iteration of HttpInspect, but
    **    all the HTTP detection components of signatures will be.
    **
    **  HTTP Event Output Module::
    **    The Event Ouput Module handles any events that have been logged
    **    in the inspection, normalization, or detection phases.
    */

    /*
    **  Session Inspection Module::
    */
    iRet = hi_si_session_inspection(GlobalConf, &Session, &SiInput, &iInspectMode, p);
    if (iRet)
        return iRet;

    /* If no mode then we just look for anomalous servers if configured
     * to do so and get out of here */
    if (iInspectMode == HI_SI_NO_MODE)
    {
        /* Let's look for rogue HTTP servers and stuff */
        if (GlobalConf->anomalous_servers && (p->dsize > 5))
        {
            iRet = hi_server_anomaly_detection(Session, p->data, p->dsize);
            if (iRet)
                return iRet;

            /*
             **  We log events before doing detection because every non-HTTP
             **  packet is possible an anomalous server.  So we still want to
             **  go through the regular detection engine, and just log any
             **  alerts here before returning.
             **
             **  Return normally if this isn't either HTTP client or server
             **  traffic.
             */
            if (Session->anom_server.event_list.stack_count)
                LogEvents(Session, p, iInspectMode, NULL);
        }

        return 0;
    }

    hsd = GetHttpSessionData(p);

    /*
     ** HI_EO_SERVER_PROTOCOL_OTHER alert added to detect 'SSH tunneling over HTTP',
     ** In SSH over HTTP evasion, first data message will always be 'HTTP response with SSH server
     ** version/banner' (without any client request). If the HTTP server response is the
     ** first message in http_session, this alert will be generated
     */
    if(!hsd && ( SiInput.pdir == HI_SI_SERVER_MODE ) && (p->packet_flags & PKT_STREAM_ORDER_OK))
    {
        if(p->ssnptr &&
             ((session_api->get_session_flags(p->ssnptr) &(SSNFLAG_SEEN_BOTH|SSNFLAG_MIDSTREAM)) == SSNFLAG_SEEN_BOTH))
        {
            if(hi_eo_generate_event(Session, HI_EO_SERVER_PROTOCOL_OTHER))
            {
                hi_eo_server_event_log(Session, HI_EO_SERVER_PROTOCOL_OTHER, NULL, NULL);
            }
            LogEvents(Session, p, iInspectMode, hsd);
        }
    }

    if ( ScPafEnabled() &&
        (p->packet_flags & PKT_STREAM_INSERT) &&
        (!(p->packet_flags & PKT_PDU_TAIL)) )
    {
        int flow_depth;

        if ( iInspectMode == HI_SI_CLIENT_MODE )
        {
            flow_depth = Session->server_conf->client_flow_depth;
            ApplyClientFlowDepth(p, flow_depth);
        }
        else
        {
            ApplyFlowDepth(Session->server_conf, p, hsd, 0, 1, GET_PKT_SEQ(p));
        }

        p->packet_flags |= PKT_HTTP_DECODE;
        HttpLogFuncs(GlobalConf, hsd, p, iCallDetect);

        if ( p->alt_dsize == 0 )
        {
            DisableDetect( p );
            EnablePreprocessor(p, PP_SDF);
            return 0;
        }
        // see comments on call to Detect() below
        PREPROC_PROFILE_START(hiDetectPerfStats);
        Detect(p);
#ifdef PERF_PROFILING
        hiDetectCalled = 1;
#endif
        PREPROC_PROFILE_END(hiDetectPerfStats);
        return 0;
    }

    if (hsd == NULL)
    {
        hsd = SetNewHttpSessionData(p, (void *)Session);
        if (hsd == NULL)
            return 0;
    }
    else
    {
        /* Gzip data should not be logged with all the packets of the session.*/
        hsd->log_flags &= ~HTTP_LOG_GZIP_DATA;
        hsd->log_flags &= ~HTTP_LOG_JSNORM_DATA;
    }

    /*
    **  HTTP Inspection Module::
    **
    **  This is where we do the client/server inspection and find the
    **  various HTTP protocol fields.  We then normalize these fields and
    **  call the detection engine.
    **
    **  The reason for the loop is for pipelined requests.  Doing pipelined
    **  requests in this way doesn't require any memory or tracking overhead.
    **  Instead, we just process each request linearly.
    */
    uint16_t vlanId = p->vh ? VTH_VLAN( p->vh ) : 0;
    if (hsd->decomp_state)
        hsd->decomp_state->stage = HTTP_DECOMP_START;
    do
    {
        /*
        **  INIT:
        **  We set this equal to zero (again) because of the pipelining
        **  requests.  We don't want to bail before we get to setting the
        **  URI, so we make sure here that this can't happen.
        */
        SetHttpDecode(0);
        ClearHttpBuffers();

        iRet = hi_mi_mode_inspection(Session, iInspectMode, p, hsd);
        if (iRet)
        {
            if (hsd)
            {
                processFileData(p, hsd, &fileProcessed);
            }
            LogEvents(Session, p, iInspectMode, hsd);
            return iRet;
        }

        iRet = hi_normalization(Session, iInspectMode, hsd);
        if (iRet)
        {
            LogEvents(Session, p, iInspectMode, hsd);
            return iRet;
        }

        HttpLogFuncs(GlobalConf, hsd, p, iCallDetect);

        /*
        **  Let's setup the pointers for the detection engine, and
        **  then go for it.
        */
        if ( iInspectMode == HI_SI_CLIENT_MODE )
        {
            const HttpBuffer* hb;
            ClearHttpBuffers();  // FIXTHIS needed here and right above??
#ifdef DUMP_BUFFER
            clearReqBuffers();
	    clearRespBuffers();
#endif
            if ( Session->client.request.uri_norm )
            {
                SetHttpBufferEncoding(
                    HTTP_BUFFER_URI,
                    Session->client.request.uri_norm,
                    Session->client.request.uri_norm_size,
                    Session->client.request.uri_encode_type);

                SetHttpBuffer(
                    HTTP_BUFFER_RAW_URI,
                    Session->client.request.uri,
                    Session->client.request.uri_size);

                p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
		dumpBuffer(URI_DUMP, Session->client.request.uri_norm, Session->client.request.uri_norm_size);
#endif

            }
            else if ( Session->client.request.uri )
            {
                SetHttpBufferEncoding(
                    HTTP_BUFFER_URI,
                    Session->client.request.uri,
                    Session->client.request.uri_size,
                    Session->client.request.uri_encode_type);

                SetHttpBuffer(
                    HTTP_BUFFER_RAW_URI,
                    Session->client.request.uri,
                    Session->client.request.uri_size);

                p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
		dumpBuffer(RAW_URI_DUMP, Session->client.request.uri, Session->client.request.uri_size);
#endif
            }

            if ( Session->client.request.header_norm ||
                 Session->client.request.header_raw )
            {
                if ( Session->client.request.header_norm )
                {
                    SetHttpBufferEncoding(
                        HTTP_BUFFER_HEADER,
                        Session->client.request.header_norm,
                        Session->client.request.header_norm_size,
                        Session->client.request.header_encode_type);

                    SetHttpBuffer(
                        HTTP_BUFFER_RAW_HEADER,
                        Session->client.request.header_raw,
                        Session->client.request.header_raw_size);

                    p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
		    dumpBuffer(REQ_HEADER_DUMP, Session->client.request.header_norm, Session->client.request.header_norm_size);
#endif

#ifdef DEBUG
                    hi_stats.req_header_len += Session->client.request.header_norm_size;
#endif
                }
                else
                {
                    SetHttpBufferEncoding(
                        HTTP_BUFFER_HEADER,
                        Session->client.request.header_raw,
                        Session->client.request.header_raw_size,
                        Session->client.request.header_encode_type);

                    SetHttpBuffer(
                        HTTP_BUFFER_RAW_HEADER,
                        Session->client.request.header_raw,
                        Session->client.request.header_raw_size);

                    p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
	            dumpBuffer(RAW_REQ_HEADER_DUMP, Session->client.request.header_raw, Session->client.request.header_raw_size);
#endif
                }
            }

            if(Session->client.request.method & (HI_POST_METHOD | HI_GET_METHOD))
            {
                if(Session->client.request.post_raw)
                {
                    if(processPostFileData(GlobalConf, p, Session, hsd) != 0)
                        return 0;

                    if(Session->server_conf->post_depth > -1)
                    {
                        if(Session->server_conf->post_depth &&
                                ((int)Session->client.request.post_raw_size > Session->server_conf->post_depth))
                        {
                            Session->client.request.post_raw_size = Session->server_conf->post_depth;
                        }
                        SetHttpBufferEncoding(
                            HTTP_BUFFER_CLIENT_BODY,
                            Session->client.request.post_raw,
                            Session->client.request.post_raw_size,
                            Session->client.request.post_encode_type);

                        p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
			dumpBuffer(CLIENT_BODY_DUMP, Session->client.request.post_raw, Session->client.request.post_raw_size);
#endif
                    }

                }
            }
            else if (hsd)
            {
                processFileData(p, hsd, &fileProcessed);
            }

            if ( Session->client.request.method_raw )
            {
                SetHttpBuffer(
                    HTTP_BUFFER_METHOD,
                    Session->client.request.method_raw,
                    Session->client.request.method_size);

                p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
		dumpBuffer(METHOD_DUMP, Session->client.request.method_raw, Session->client.request.method_size);
#endif
            }

            if ( Session->client.request.cookie_norm ||
                 Session->client.request.cookie.cookie )
            {
                if ( Session->client.request.cookie_norm )
                {
                    SetHttpBufferEncoding(
                        HTTP_BUFFER_COOKIE,
                        Session->client.request.cookie_norm,
                        Session->client.request.cookie_norm_size,
                        Session->client.request.cookie_encode_type);

                    SetHttpBuffer(
                        HTTP_BUFFER_RAW_COOKIE,
                        Session->client.request.cookie.cookie,
                        Session->client.request.cookie.cookie_end -
                            Session->client.request.cookie.cookie);

                    p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
	            dumpBuffer(COOKIE_DUMP, Session->client.request.cookie_norm, Session->client.request.cookie_norm_size);
#endif
                }
                else
                {
                    SetHttpBufferEncoding(
                        HTTP_BUFFER_COOKIE,
                        Session->client.request.cookie.cookie,
                        Session->client.request.cookie.cookie_end -
                            Session->client.request.cookie.cookie,
                        Session->client.request.cookie_encode_type);

                    SetHttpBuffer(
                        HTTP_BUFFER_RAW_COOKIE,
                        Session->client.request.cookie.cookie,
                        Session->client.request.cookie.cookie_end -
                            Session->client.request.cookie.cookie);

                    p->packet_flags |= PKT_HTTP_DECODE;
#ifdef DUMP_BUFFER
		    dumpBuffer(RAW_COOKIE_DUMP, Session->client.request.cookie.cookie, Session->client.request.cookie.cookie_end - Session->client.request.cookie.cookie);
#endif
                }
            }
            else if ( !Session->server_conf->enable_cookie &&
                (hb = GetHttpBuffer(HTTP_BUFFER_HEADER)) )
            {
                SetHttpBufferEncoding(
                    HTTP_BUFFER_COOKIE, hb->buf, hb->length, hb->encode_type);

                hb = GetHttpBuffer(HTTP_BUFFER_RAW_HEADER);
                assert(hb);

                SetHttpBuffer(HTTP_BUFFER_RAW_COOKIE, hb->buf, hb->length);
#ifdef DUMP_BUFFER
		dumpBuffer(RAW_COOKIE_DUMP, hb->buf, hb->length);
#endif
                p->packet_flags |= PKT_HTTP_DECODE;
            }

            if ( IsLimitedDetect(p) )
            {
                ApplyClientFlowDepth(p, Session->server_conf->client_flow_depth);

                if( !GetHttpBufferMask() && (p->alt_dsize == 0)  )
                {
                    DisableDetect( p );
                    EnablePreprocessor(p, PP_SDF);
                    return 0;
                }
            }

            if (Session->client.request.range_flag != HTTP_RANGE_NONE)
            {
                if (Session->client.request.method != HI_GET_METHOD)
                {
                    if (hi_eo_generate_event(Session, HI_EO_CLIENT_RANGE_NON_GET_METHOD))
                    {
                        hi_eo_client_event_log(Session, HI_EO_CLIENT_RANGE_NON_GET_METHOD, NULL, NULL);
                    } 
                }
                else
                {
                    if (Session->client.request.range_flag == RANGE_WITH_REQ_ERROR)
                    {
                        if (hi_eo_generate_event(Session, HI_EO_CLIENT_RANGE_FIELD_ERROR))
                        {
                            hi_eo_client_event_log(Session, HI_EO_CLIENT_RANGE_FIELD_ERROR, NULL, NULL);
                        } 
                    }
                    else
                    {
                        if (vlanId == 0) 
                            hsd->resp_state.look_for_partial_content |= GET_REQ_WITH_RANGE;
                    }
                }
            }
        }
        else   /* Server mode */
        {
            const HttpBuffer* hb;

            /*
            **  We check here to see whether this was a server response
            **  header or not.  If the header size is 0 then, we know that this
            **  is not the header and don't do any detection.
            */
#if defined(FEAT_OPEN_APPID)
            if( !(Session->server_conf->inspect_response || Session->server_conf->appid_enabled) &&
#else
            if( !(Session->server_conf->inspect_response) &&
#endif /* defined(FEAT_OPEN_APPID) */
                IsLimitedDetect(p) && !p->alt_dsize )
            {
                DisableDetect( p );
                    EnablePreprocessor(p, PP_SDF);
                if(Session->server_conf->server_flow_depth == -1)
                {
                    EnablePreprocessor(p, PP_DCE2);
                }
                return 0;
            }
            ClearHttpBuffers();

             if ( Session->server.response.header_norm ||
                  Session->server.response.header_raw )
             {
                 if ( Session->server.response.header_norm )
                 {
                     SetHttpBufferEncoding(
                         HTTP_BUFFER_HEADER,
                         Session->server.response.header_norm,
                         Session->server.response.header_norm_size,
                         Session->server.response.header_encode_type);

                     SetHttpBuffer(
                         HTTP_BUFFER_RAW_HEADER,
                         Session->server.response.header_raw,
                         Session->server.response.header_raw_size);
#ifdef DUMP_BUFFER
		     dumpBuffer(RESP_HEADER_DUMP, Session->server.response.header_norm, Session->server.response.header_norm_size);
#endif
                 }
                 else
                 {
                     SetHttpBuffer(
                         HTTP_BUFFER_HEADER,
                         Session->server.response.header_raw,
                         Session->server.response.header_raw_size);

                     SetHttpBuffer(
                         HTTP_BUFFER_RAW_HEADER,
                         Session->server.response.header_raw,
                         Session->server.response.header_raw_size);
#ifdef DUMP_BUFFER
		     dumpBuffer(RAW_RESP_HEADER_DUMP, Session->server.response.header_raw, Session->server.response.header_raw_size);
#endif
                 }
             }

             if ( Session->server.response.cookie_norm ||
                  Session->server.response.cookie.cookie )
             {
                 if(Session->server.response.cookie_norm )
                 {
                     SetHttpBufferEncoding(
                         HTTP_BUFFER_COOKIE,
                         Session->server.response.cookie_norm,
                         Session->server.response.cookie_norm_size,
                         Session->server.response.cookie_encode_type);

                     SetHttpBuffer(
                         HTTP_BUFFER_RAW_COOKIE,
                         Session->server.response.cookie.cookie,
                         Session->server.response.cookie.cookie_end -
                             Session->server.response.cookie.cookie);
#ifdef DUMP_BUFFER
	             dumpBuffer(RESP_COOKIE_DUMP, Session->server.response.cookie_norm, Session->server.response.cookie_norm_size);
#endif
                 }
                 else
                 {
                     SetHttpBuffer(
                         HTTP_BUFFER_COOKIE,
                         Session->server.response.cookie.cookie,
                         Session->server.response.cookie.cookie_end -
                             Session->server.response.cookie.cookie);

                     SetHttpBuffer(
                         HTTP_BUFFER_RAW_COOKIE,
                         Session->server.response.cookie.cookie,
                         Session->server.response.cookie.cookie_end -
                             Session->server.response.cookie.cookie);
#ifdef DUMP_BUFFER
		     dumpBuffer(RAW_RESP_COOKIE_DUMP, Session->server.response.cookie.cookie, Session->server.response.cookie.cookie_end - Session->server.response.cookie.cookie);
#endif
                 }
             }
             else if ( !Session->server_conf->enable_cookie &&
                 (hb = GetHttpBuffer(HTTP_BUFFER_HEADER)) )
             {
                 SetHttpBufferEncoding(
                     HTTP_BUFFER_COOKIE, hb->buf, hb->length, hb->encode_type);

                 hb = GetHttpBuffer(HTTP_BUFFER_RAW_HEADER);
                 assert(hb);

                 SetHttpBuffer(HTTP_BUFFER_RAW_COOKIE, hb->buf, hb->length);

#ifdef DUMP_BUFFER
		 dumpBuffer(RAW_RESP_COOKIE_DUMP, hb->buf, hb->length);
#endif
             }

             if(Session->server.response.status_code)
             {
                 SetHttpBuffer(
                     HTTP_BUFFER_STAT_CODE,
                     Session->server.response.status_code,
                     Session->server.response.status_code_size);
                 
                 if (!strncmp((const char*)Session->server.response.status_code, "206", 3))
                 {
                     if ((Session->server.response.range_flag == RANGE_WITH_RESP_ERROR) && 
                         hi_eo_generate_event(Session, HI_EO_SERVER_RANGE_FIELD_ERROR))
                     {
                         hi_eo_server_event_log(Session, HI_EO_SERVER_RANGE_FIELD_ERROR, NULL, NULL);
                     }
                     if ((vlanId == 0) && !(hsd->resp_state.look_for_partial_content &= GET_REQ_WITH_RANGE) &&
                         hi_eo_generate_event(Session, HI_EO_SERVER_NON_RANGE_GET_PARTIAL_METHOD))
                     {
                         hi_eo_server_event_log(Session, HI_EO_SERVER_NON_RANGE_GET_PARTIAL_METHOD, NULL, NULL);
                     }
                     
                     if (Session->server.response.range_flag == HTTP_RESP_RANGE_NONE)
                     {
                         hsd->resp_state.look_for_partial_content |= CONTENT_NONE;
                     }
                     else if (Session->server.response.range_flag == RANGE_WITH_RESP_FULL_CONTENT)
                     {
                         hsd->resp_state.look_for_partial_content |= FULL_CONTENT;
                     }
                     else
                     {
                         hsd->resp_state.look_for_partial_content |= PARTIAL_CONTENT;
                     }

                     if ((Session->client.request.range_flag == HTTP_RANGE_WITH_FULL_CONTENT_REQ) && 
                         ((Session->server.response.range_flag == RANGE_WITH_RESP_UNKNOWN_CONTENT_SIZE) ||
                          (Session->server.response.range_flag == RANGE_WITH_UNKNOWN_CONTENT_RANGE) ||
                          (Session->server.response.range_flag == RANGE_WITH_RESP_ERROR)))
                     {
                         hsd->resp_state.look_for_partial_content |= FULL_CONTENT;
                     } 
                 }

#ifdef DUMP_BUFFER
		 dumpBuffer(STAT_CODE_DUMP, Session->server.response.status_code, Session->server.response.status_code_size);
#endif
             }

             if(Session->server.response.status_msg)
             {
                 SetHttpBuffer(
                     HTTP_BUFFER_STAT_MSG,
                     Session->server.response.status_msg,
                     Session->server.response.status_msg_size);
#ifdef DUMP_BUFFER
		 dumpBuffer(STAT_MSG_DUMP, Session->server.response.status_msg, Session->server.response.status_msg_size);
#endif
             }

             if(Session->server.response.body_raw_size > 0)
             {
                 int detect_data_size = (int)Session->server.response.body_size;

                 /*body_size is included in the data_extracted*/
                 if((Session->server_conf->server_flow_depth > 0) &&
                         (hsd->resp_state.data_extracted  < (Session->server_conf->server_flow_depth + (int)Session->server.response.body_raw_size)))
                 {
                     /*flow_depth is smaller than data_extracted, need to subtract*/
                     if(Session->server_conf->server_flow_depth < hsd->resp_state.data_extracted)
                         detect_data_size -= hsd->resp_state.data_extracted - Session->server_conf->server_flow_depth;
                 }
                 else if (Session->server_conf->server_flow_depth)
                 {
                     detect_data_size = 0;
                 }

                 /* Do we have a file decompression object? */
                 if( hsd->fd_state != 0 )
                 {
                     fd_status_t Ret_Code;

                     uint16_t Data_Len;
                     const uint8_t *Data;

                     hsd->fd_state->Next_In = (uint8_t *) (Data = Session->server.response.body_raw);
                     hsd->fd_state->Avail_In = (Data_Len = (uint16_t) detect_data_size);

                     (void)File_Decomp_SetBuf( hsd->fd_state );

                     Ret_Code = File_Decomp( hsd->fd_state );

                     if( Ret_Code == File_Decomp_DecompError )
                     {
                         Session->server.response.body = Data;
                         Session->server.response.body_raw = Data;
                         Session->server.response.body_size = Data_Len;
                         Session->server.response.body_raw_size = Data_Len;

                         if(hi_eo_generate_event(Session, hsd->fd_state->Error_Event))
                         {
                             hi_eo_server_event_log(Session, hsd->fd_state->Error_Event, NULL, NULL);
                         }
                         File_Decomp_StopFree( hsd->fd_state );
                         hsd->fd_state = NULL;
                      }
                     /* If we didn't find a Sig, then clear the File_Decomp state
                        and don't keep looking. */
                     else if( Ret_Code == File_Decomp_NoSig )
                     {
                         File_Decomp_StopFree( hsd->fd_state );
                         hsd->fd_state = NULL;
                     }
                     else
                     {
                         Session->server.response.body = hsd->fd_state->Buffer;
                         Session->server.response.body_size = (DECODE_BLEN - hsd->fd_state->Avail_Out);
                         Session->server.response.body_raw = Data;
                         Session->server.response.body_raw_size = Data_Len;
                     }

                     setFileDataPtr(Session->server.response.body, (uint16_t)Session->server.response.body_size);
#ifdef DUMP_BUFFER
	             dumpBuffer(FILE_DATA_DUMP, Session->server.response.body, (uint16_t)Session->server.response.body_size);
#endif
                 }
                 else
                 {
                     setFileDataPtr(Session->server.response.body, (uint16_t)detect_data_size);
#ifdef DUMP_BUFFER
                     dumpBuffer(FILE_DATA_DUMP, Session->server.response.body, (uint16_t)detect_data_size);
#endif
                 }

                 if (ScPafEnabled() && PacketHasPAFPayload(p) && !(p->packet_flags & PKT_PSEUDO_FLUSH))
                 {
                     bool decomp_more = (hsd->decomp_state && hsd->decomp_state->stage == HTTP_DECOMP_MID)?true:false;
                     char *pfile_type = NULL;

                     int file_data_position = get_file_current_position(p,decomp_more,is_first);

                     if (file_data_position == SNORT_FILE_POSITION_UNKNOWN && hsd->resp_state.eoh_found)
                     {
                         file_data_position = SNORT_FILE_START;
                     }
                     if (file_api->file_process(p, (uint8_t *)Session->server.response.body_raw,
                                                   (uint16_t)Session->server.response.body_raw_size,
                                                   file_data_position, false, false, false))
                     {
                         setFileName(p);
                     }
                     if (GlobalConf->normalize_nulls)
                     {
                         /* Call File API to get the file type */
                         pfile_type = file_api->file_get_filetype (p->ssnptr);
                         if (pfile_type)
                         {
                             if (SnortStrcasestr(pfile_type,strlen(pfile_type), "Unknown" ) ||
                                SnortStrcasestr(pfile_type,strlen(pfile_type), "RTF" ))
                             {

                                 Session->server.response.body_size = NormalizeRandomNulls(
                                                                  (uint8_t*) Session->server.response.body,
                                                                  Session->server.response.body_size,
                                                                  (uint8_t*) Session->server.response.body);
                             }
                         }
                     }
                 }
                 is_first = false;
#ifdef DUMP_BUFFER
                 dumpBuffer(RESP_BODY_DUMP, Session->server.response.body_raw, Session->server.response.body_raw_size);
#endif
             }

             if( IsLimitedDetect(p) &&
                 !GetHttpBufferMask() && (p->alt_dsize == 0)  )
             {
                 DisableDetect( p );
                 EnablePreprocessor(p, PP_SDF);
                 if(Session->server_conf->server_flow_depth == -1)
                 {
                            EnablePreprocessor(p, PP_DCE2);
                 }
                 return 0;
             }
        }

        /*
        **  If we get here we either had a client or server request/response.
        **  We do the detection here, because we're starting a new paradigm
        **  about protocol decoders.
        **
        **  Protocol decoders are now their own detection engine, since we are
        **  going to be moving protocol field detection from the generic
        **  detection engine into the protocol module.  This idea scales much
        **  better than having all these Packet struct field checks in the
        **  main detection engine for each protocol field.
        */
        PREPROC_PROFILE_START(hiDetectPerfStats);
        Detect(p);
#ifdef PERF_PROFILING
        hiDetectCalled = 1;
#endif
        PREPROC_PROFILE_END(hiDetectPerfStats);

        /*
        **  Handle event stuff after we do detection.
        **
        **  Here's the reason why:
        **    - since snort can only handle one logged event per packet,
        **      we only log HttpInspect events if there wasn't one in the
        **      detection engine.  I say that events generated in the
        **      "advanced generic content matching" engine is more
        **      important than generic events that I can log here.
        */
        LogEvents(Session, p, iInspectMode, hsd);

        /*
        **  We set the global detection flag here so that if request pipelines
        **  fail, we don't do any detection.
        */
        iCallDetect = 0;

#ifdef PPM_MGR
        /*
        **  Check PPM here to ensure decompression loop doesn't spin indefinitely
        */
        if( PPM_PKTS_ENABLED() )
        {
            PPM_GET_TIME();
            PPM_PACKET_TEST();

            if( PPM_PACKET_ABORT_FLAG() )
                return 0;
        }
#endif
    } while(Session->client.request.pipeline_req || (hsd->decomp_state && hsd->decomp_state->stage == HTTP_DECOMP_MID));

    if ( iCallDetect == 0 )
    {
        /* Detect called at least once from above pkt processing loop. */
        DisableAllDetect( p );

        /* dcerpc2 preprocessor may need to look at this for
         * RPC over HTTP setup */
        EnablePreprocessor(p, PP_DCE2);

        /* sensitive_data preprocessor may look for PII over HTTP */
        EnablePreprocessor(p, PP_SDF);
    }

    return 0;
}

int HttpInspectInitializeGlobalConfig(HTTPINSPECT_GLOBAL_CONF *config,
                                      char *ErrorString, int iErrStrLen)
{
    int iRet;

    if (config == NULL)
    {
        snprintf(ErrorString, iErrStrLen, "Global configuration is NULL.");
        return -1;
    }

    iRet = hi_ui_config_init_global_conf(config);
    if (iRet)
    {
        snprintf(ErrorString, iErrStrLen,
                 "Error initializing Global Configuration.");
        return -1;
    }

    iRet = hi_client_init(config);
    if (iRet)
    {
        snprintf(ErrorString, iErrStrLen,
                 "Error initializing client module.");
        return -1;
    }

    file_api->set_mime_decode_config_defauts(&(config->decode_conf));
    file_api->set_mime_log_config_defauts(&(config->mime_conf));

    RegisterGetHttpXffFields(getHttpXffFields);
    session_api->register_get_http_xff_precedence(getHttpXffPrecedence);

    return 0;
}

HttpSessionData * SetNewHttpSessionData(Packet *p, void *data)
{
    HttpSessionData *hsd;

    if (p->ssnptr == NULL)
        return NULL;

    hi_stats.session_count++;          

    hsd = (HttpSessionData *)SnortPreprocAlloc(1, sizeof(HttpSessionData), 
                                  PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
    hi_stats.mem_used += (sizeof(HttpSessionData) + sizeof(DECOMPRESS_STATE) + sizeof(HTTP_LOG_STATE));
    init_decode_utf_state(&hsd->utf_state);

    session_api->set_application_data(p->ssnptr, PP_HTTPINSPECT, hsd, FreeHttpSessionData);

    hsd->fd_state = (fd_session_p_t)NULL;
    hsd->resp_state.eoh_found = false;
    hsd->resp_state.look_for_partial_content = CONTENT_NONE;
    hsd->resp_state.chunk_len_state = CHUNK_LEN_DEFAULT;

    return hsd;
}

void FreeHttpSessionData(void *data)
{
    HttpSessionData *hsd = (HttpSessionData *)data;

    if (hsd == NULL)
        return;
    hi_stats.session_count--;    

    if (hsd->decomp_state != NULL)
    {
        inflateEnd(&(hsd->decomp_state->d_stream));
        mempool_free(hi_gzip_mempool, hsd->decomp_state->bkt);
    }

    if (hsd->log_state != NULL)
    {
        mempool_free(http_mempool, hsd->log_state->log_bucket);
        SnortPreprocFree(hsd->log_state, sizeof(HTTP_LOG_STATE), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
    }

    while(hsd->tList_start != NULL )
        deleteNode_tList(hsd);

    file_api->free_mime_session(hsd->mime_ssn);

    if( hsd->fd_state != 0 )
    {
        File_Decomp_StopFree(hsd->fd_state);   // Stop & Stop &  Free fd session object
        hsd->fd_state = NULL;                  // ...just for good measure
    }

    hi_stats.mem_used -= (sizeof(HttpSessionData) + sizeof(DECOMPRESS_STATE) + sizeof(HTTP_LOG_STATE));
    SnortPreprocFree(hsd, sizeof(HttpSessionData), PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
}

int GetHttpTrueIP(void *data, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    sfaddr_t *true_ip;

    true_ip = GetTrueIPForSession(data);
    if(!true_ip)
        return 0;

    if(sfaddr_family(true_ip) == AF_INET6)
    {
        *type = EVENT_INFO_XFF_IPV6;
        *len = sizeof(struct in6_addr); /*ipv6 address size in bytes*/
        *buf = (uint8_t*)sfaddr_get_ip6_ptr(true_ip);
    }
    else
    {
        *type = EVENT_INFO_XFF_IPV4;
        *len = sizeof(struct in_addr); /*ipv4 address size in bytes*/
        *buf = (uint8_t*)sfaddr_get_ip4_ptr(true_ip);
    }

    return 1;
}

int IsGzipData(void *data)
{
    HttpSessionData *hsd = NULL;

    if (data == NULL)
        return -1;
    hsd = (HttpSessionData *)session_api->get_application_data(data, PP_HTTPINSPECT);

    if(hsd == NULL)
        return -1;

    if((hsd->log_flags & HTTP_LOG_GZIP_DATA) && (file_data_ptr.len > 0 ))
        return 0;
    else
        return -1;
}


int GetHttpGzipData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    if(!IsGzipData(data))
    {
        *buf = (uint8_t*)file_data_ptr.data;
        *len = file_data_ptr.len;
        *type = EVENT_INFO_GZIP_DATA;
        return 1;
    }

    return 0;

}

int IsJSNormData(void *data)
{
    HttpSessionData *hsd = NULL;

    if (data == NULL)
        return -1;
    hsd = (HttpSessionData *)session_api->get_application_data(data, PP_HTTPINSPECT);

    if(hsd == NULL)
        return -1;

    if((hsd->log_flags & HTTP_LOG_JSNORM_DATA) && (file_data_ptr.len > 0 ))
        return 0;
    else
        return -1;

}

int GetHttpJSNormData(void *data,  uint8_t **buf, uint32_t *len, uint32_t *type)
{
    if(!IsJSNormData(data))
    {
        *buf = (uint8_t*) file_data_ptr.data;
        *len = file_data_ptr.len;
        *type = EVENT_INFO_JSNORM_DATA;
        return 1;
    }

    return 0;
}

int GetHttpUriData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    HttpSessionData *hsd = NULL;

    if (data == NULL)
        return 0;
    hsd = (HttpSessionData *)session_api->get_application_data(data, PP_HTTPINSPECT);

    if(hsd == NULL)
        return 0;

    if(hsd->log_state && hsd->log_state->uri_bytes > 0)
    {
        *buf = hsd->log_state->uri_extracted;
        *len = hsd->log_state->uri_bytes;
        *type = EVENT_INFO_HTTP_URI;
        return 1;
    }

    return 0;
}


int GetHttpHostnameData(void *data, uint8_t **buf, uint32_t *len, uint32_t *type)
{
    HttpSessionData *hsd = NULL;

    if (data == NULL)
        return 0;
    hsd = (HttpSessionData *)session_api->get_application_data(data, PP_HTTPINSPECT);

    if(hsd == NULL)
        return 0;

    if(hsd->log_state && hsd->log_state->hostname_bytes > 0)
    {
        *buf = hsd->log_state->hostname_extracted;
        *len = hsd->log_state->hostname_bytes;
        *type = EVENT_INFO_HTTP_HOSTNAME;
        return 1;
    }

    return 0;
}

void HI_SearchInit(void)
{
    const HiSearchToken *tmp;
    hi_javascript_search_mpse = search_api->search_instance_new();
    if (hi_javascript_search_mpse == NULL)
    {
        FatalError("%s(%d) Could not allocate memory for HTTP <script> tag search.\n",
                               __FILE__, __LINE__);
    }
    for (tmp = &hi_patterns[0]; tmp->name != NULL; tmp++)
    {
        hi_js_search[tmp->search_id].name = tmp->name;
        hi_js_search[tmp->search_id].name_len = tmp->name_len;
        search_api->search_instance_add(hi_javascript_search_mpse, tmp->name, tmp->name_len, tmp->search_id);
    }
    search_api->search_instance_prep(hi_javascript_search_mpse);

    hi_htmltype_search_mpse = search_api->search_instance_new();
    if (hi_htmltype_search_mpse == NULL)
    {
        FatalError("%s(%d) Could not allocate memory for HTTP <script> type search.\n",
                                   __FILE__, __LINE__);
    }
    for (tmp = &html_patterns[0]; tmp->name != NULL; tmp++)
    {
        hi_html_search[tmp->search_id].name = tmp->name;
        hi_html_search[tmp->search_id].name_len = tmp->name_len;
        search_api->search_instance_add(hi_htmltype_search_mpse, tmp->name, tmp->name_len, tmp->search_id);
    }
    search_api->search_instance_prep(hi_htmltype_search_mpse);
}

void HI_SearchFree(void)
{
    if (hi_javascript_search_mpse != NULL)
        search_api->search_instance_free(hi_javascript_search_mpse);

    if (hi_htmltype_search_mpse != NULL)
        search_api->search_instance_free(hi_htmltype_search_mpse);
}

int HI_SearchStrFound(void *id, void *unused, int index, void *data, void *unused2)
{
    int search_id = (int)(uintptr_t)id;

    hi_search_info.id = search_id;
    hi_search_info.index = index;
    hi_search_info.length = hi_current_search[search_id].name_len;

    /* Returning non-zero stops search, which is okay since we only look for one at a time */
    return 1;
}

bool GetHttpFastBlockingStatus()
{
    HTTPINSPECT_GLOBAL_CONF *http_conf = NULL;
    tSfPolicyId policyId = getNapRuntimePolicy();

    sfPolicyUserPolicySet(hi_config, policyId);
    http_conf =  (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(hi_config);

    return(http_conf->fast_blocking);
}

static int GetHttpInspectConf( void *ssn, uint32_t flags, HTTPINSPECT_CONF **serverConf, HTTPINSPECT_CONF **clientConf )
{
    int iRet = HI_SUCCESS;
    tSfPolicyId policyId = getNapRuntimePolicy();
    HTTPINSPECT_GLOBAL_CONF *pPolicyConfig = NULL;
    HI_SI_INPUT SiInput;
    int iInspectMode = 0;
    SessionControlBlock *scb = (SessionControlBlock *)ssn;
    sfPolicyUserPolicySet (hi_config, policyId);
    pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(hi_config);
    if( !scb )
        return iRet;

    SiInput.pdir = HI_SI_NO_MODE;

    if(  flags & PKT_FROM_CLIENT )
    {
        SiInput.pdir = HI_SI_CLIENT_MODE;

        IP_COPY_VALUE(SiInput.sip, &scb->client_ip);
        IP_COPY_VALUE(SiInput.dip, &scb->server_ip);

        SiInput.sport = ntohs(scb->client_port);
        SiInput.dport = ntohs(scb->server_port);
    }
    else if (  flags & PKT_FROM_SERVER )
    {
        SiInput.pdir = HI_SI_SERVER_MODE;

        IP_COPY_VALUE(SiInput.sip, &scb->server_ip);
        IP_COPY_VALUE(SiInput.dip, &scb->client_ip);

        SiInput.sport = ntohs(scb->server_port);
        SiInput.dport = ntohs(scb->client_port);
    }

    iRet = GetHttpConf(pPolicyConfig, serverConf, clientConf, &SiInput, &iInspectMode, ssn);
    return iRet;
}

static char** getHttpXffPrecedence(void* ssn, uint32_t flags, int* nFields)
{
    HTTPINSPECT_CONF *serverConf = NULL;
    HTTPINSPECT_CONF *clientConf = NULL;

    GetHttpInspectConf(ssn, flags, &serverConf, &clientConf);

    if (!serverConf || !serverConf->xff_headers[0])
    {
        if (nFields) *nFields = 0;
        return NULL;
    }

    if (nFields)
    {
        for ((*nFields) = 0; ((*nFields) < HTTP_MAX_XFF_FIELDS) && serverConf->xff_headers[*nFields]; (*nFields)++)
            ;
    }
    return (char**)serverConf->xff_headers;
}

int GetHttpFlowDepth(void *ssn, uint32_t flags)
{
    HTTPINSPECT_CONF *serverConf = 0;
    HTTPINSPECT_CONF *clientConf = 0;
    int flow_depth  = 0;

    GetHttpInspectConf( ssn, flags, &serverConf, &clientConf );
    if( !serverConf )
        return flow_depth;

    if( serverConf->file_policy )
    {
        flow_depth = 0;
    }
    else if(  flags & PKT_FROM_CLIENT )
    {
        //Only for POST
        flow_depth =  serverConf->post_depth;
    }
    else if( flags & PKT_FROM_SERVER )
    {
        flow_depth = serverConf->server_flow_depth;
    }
    return flow_depth;
}

uint8_t isHttpRespPartialCont(void *data)
{
    HttpSessionData *hsd = NULL;

    if (data == NULL) {
        return CONTENT_NONE;
    }

    hsd = (HttpSessionData *)session_api->get_application_data(data, PP_HTTPINSPECT);
    if (hsd == NULL) {
        return CONTENT_NONE;
    }

    return hsd->resp_state.look_for_partial_content;
}
