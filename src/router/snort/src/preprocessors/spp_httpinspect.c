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
**  @file       preproc_setup.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      This file initializes HttpInspect as a Snort
**              preprocessor.
**
**  This file registers the HttpInspect initialization function,
**  adds the HttpInspect function into the preprocessor list, reads
**  the user configuration in the snort.conf file, and prints out
**  the configuration that is read.
**
**  In general, this file is a wrapper to HttpInspect functionality,
**  by interfacing with the Snort preprocessor functions.  The rest
**  of HttpInspect should be separate from the preprocessor hooks.
**
**  NOTES
**
**  - 2.10.03:  Initial Development.  DJR
*/

#include <assert.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "decode.h"
#include "plugbase.h"
#include "snort_debug.h"
#include "util.h"
#include "parser.h"
#include "memory_stats.h"

#include "hi_ui_config.h"
#include "hi_ui_server_lookup.h"
#include "hi_client.h"
#include "hi_norm.h"
#include "snort_httpinspect.h"
#include "hi_util_kmap.h"
#include "hi_util_xmalloc.h"
#include "hi_cmd_lookup.h"
#include "hi_paf.h"
#include "h2_paf.h"

#ifdef DUMP_BUFFER
#include "hi_buffer_dump.h"
#endif

#include "file_decomp.h"

#include "snort.h"
#include "profiler.h"
#include "mstring.h"
#include "sp_preprocopt.h"
#include "detection_util.h"

#ifdef TARGET_BASED
#include "session_api.h"
#include "stream_api.h"
#include "sftarget_protocol_reference.h"
#endif

#include "sfPolicy.h"
#include "mempool.h"
#include "file_api.h"
#include "sf_email_attach_decode.h"

#ifdef SNORT_RELOAD
#include "reload.h"
#include "file_mime_process.h"
#ifdef REG_TEST
#include "reg_test.h"
#endif
#endif

#if defined(FEAT_OPEN_APPID)
#include "spp_stream6.h"
#endif /* defined(FEAT_OPEN_APPID) */

/*
**  Defines for preprocessor initialization
*/
/**
**  snort.conf preprocessor keyword
*/
#define GLOBAL_KEYWORD   "http_inspect"
#define SERVER_KEYWORD   "http_inspect_server"

const char *PROTOCOL_NAME = "HTTP";

/**
**  The length of the error string buffer.
*/
#define ERRSTRLEN 1000

/*
**  External Global Variables
**  Variables that we need from Snort to log errors correctly and such.
*/
extern char *file_name;
extern int file_line;

/*
**  Global Variables
**  This is the only way to work with Snort preprocessors because
**  the user configuration must be kept between the Init function
**  the actual preprocessor.  There is no interaction between the
**  two except through global variable usage.
*/
tSfPolicyUserContextId hi_config = NULL;

#ifdef TARGET_BASED
/* Store the protocol id received from the stream reassembler */
int16_t hi_app_protocol_id;
int16_t h2_app_protocol_id;
#endif

#ifdef PERF_PROFILING
PreprocStats hiPerfStats;
PreprocStats hi2PerfStats;
PreprocStats hi2InitPerfStats;
PreprocStats hi2PayloadPerfStats;
PreprocStats hi2PseudoPerfStats;
PreprocStats hiDetectPerfStats;
int hiDetectCalled = 0;
#endif

static tSfPolicyId httpCurrentPolicy = 0;

MemPool *hi_gzip_mempool = NULL;
fd_config_t hi_fd_conf;
uint8_t decompression_buffer[65535];

uint8_t dechunk_buffer[65535];

MemPool *http_mempool = NULL;
MemPool *mime_decode_mempool = NULL;
MemPool *mime_log_mempool = NULL;
int hex_lookup[256];
int valid_lookup[256];

char** xffFields = NULL;
static char** oldXffFields = NULL;

/*
** Prototypes
*/
static void HttpInspectDropStats(int);
static void HttpInspect(Packet *, void *);
static void HttpInspectCleanExit(int, void *);
static void HttpInspectReset(int, void *);
static void HttpInspectResetStats(int, void *);
static void HttpInspectInit(struct _SnortConfig *, char *);
static void addServerConfPortsToStream(struct _SnortConfig *sc, void *);
static void HttpInspectFreeConfigs(tSfPolicyUserContextId);
static void HttpInspectFreeConfig(HTTPINSPECT_GLOBAL_CONF *);
static int HttpInspectCheckConfig(struct _SnortConfig *);
static void HttpInspectAddPortsOfInterest(struct _SnortConfig *, HTTPINSPECT_GLOBAL_CONF *, tSfPolicyId);
static int HttpEncodeInit(struct _SnortConfig *, char *, char *, void **);
static int HttpEncodeEval(void *, const uint8_t **, void *);
static void HttpEncodeCleanup(void *);
static void HttpInspectRegisterRuleOptions(struct _SnortConfig *);
static void HttpInspectRegisterXtraDataFuncs(HTTPINSPECT_GLOBAL_CONF *);
static inline void InitLookupTables(void);
#ifdef TARGET_BASED
static void HttpInspectAddServicesOfInterest(struct _SnortConfig *, HTTPINSPECT_GLOBAL_CONF *, tSfPolicyId);
#endif

#ifdef SNORT_RELOAD
static int HttpMempoolFreeUsedBucket(MemPool *memory_pool);
static unsigned HttpMempoolAdjust(MemPool *memory_pool, unsigned httpMaxWork);
static bool HttpGzipReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool HttpFdReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool HttpMempoolReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool HttpMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
static bool HttpLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData);
#ifdef REG_TEST
static void display_http_mempool(MemPool *mempool, const char* old_new, const char *pool_type);
static int HttpInspectUnlimitedDecompressIterate(void *data);
static int HttpInspectUnlimitedDecompress(struct _SnortConfig *sc, tSfPolicyUserContextId config, tSfPolicyId policyId, void *pData);
static void display_gzip_fd_config_changes(HTTPINSPECT_GLOBAL_CONF* configOld, HTTPINSPECT_GLOBAL_CONF* configNew,
                                           bool old_gzip, bool new_gzip, bool old_fd, bool new_fd, bool old_ud, bool new_ud);
#endif
static void update_gzip_mempool(bool old_gzip, bool new_gzip, uint32_t max_sessions);
static void update_fd_mempool(bool old_fd, bool new_fd, uint32_t max_sessions);
static void update_gzip_fd_mempools(HTTPINSPECT_GLOBAL_CONF* configNew, bool old_gzip, bool new_gzip, bool old_fd, bool new_fd);
static void update_http_mempool(uint32_t new_memcap, uint32_t old_memcap);
static void HttpInspectReloadGlobal(struct _SnortConfig *, char *, void **);
static void HttpInspectReload(struct _SnortConfig *, char *, void **);
static int HttpInspectReloadVerify(struct _SnortConfig *, void *);
static void * HttpInspectReloadSwap(struct _SnortConfig *, void *);
static void HttpInspectReloadSwapFree(void *);
#endif


/*
**  NAME
**    HttpInspect::
*/
/**
**  This function wraps the functionality in the generic HttpInspect
**  processing.  We get a Packet structure and pass this into the
**  HttpInspect module where the first stage in HttpInspect is the
**  Session Inspection stage where most of the other Snortisms are
**  taken care of.  After that, the modules should be fairly generic,
**  and that's what we're trying to do here.
**
**  @param p a Packet structure that contains Snort info about the
**  packet.
**
**  @return void
*/
static void HttpInspect(Packet *p, void *context)
{
    tSfPolicyId policy_id = getNapRuntimePolicy();
    HTTPINSPECT_GLOBAL_CONF *pPolicyConfig = NULL ;
    PROFILE_VARS;
    sfPolicyUserPolicySet (hi_config, policy_id);
    pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(hi_config);

    if ( pPolicyConfig == NULL)
        return;

    // preconditions - what we registered for
    assert(IsTCP(p) && p->dsize && p->data);

    PREPROC_PROFILE_START(hiPerfStats);

    /*
    **  Pass in the configuration and the packet.
    */
    SnortHttpInspect(pPolicyConfig, p);

    ClearHttpBuffers();

    /* XXX:
     * NOTE: this includes the HTTPInspect directly
     * calling the detection engine -
     * to get the true HTTPInspect only stats, have another
     * var inside SnortHttpInspect that tracks the time
     * spent in Detect().
     * Subtract the ticks from this if iCallDetect == 0
     */
    PREPROC_PROFILE_END(hiPerfStats);
#ifdef PERF_PROFILING
    if (hiDetectCalled)
    {
        hiPerfStats.ticks -= hiDetectPerfStats.ticks;
        /* And Reset ticks to 0 */
        hiDetectPerfStats.ticks = 0;
        hiDetectCalled = 0;
    }
#endif

    return;
}


size_t hi_get_free_mempool(MemPool *mempool)
{
    if (mempool)
        return mempool->max_memory - mempool->used_memory;
    return 0;
}

size_t hi_get_used_mempool(MemPool *mempool)
{
    if (mempool)
        return mempool->used_memory;
    return 0;
}

size_t hi_get_max_mempool(MemPool *mempool)
{   
    if (mempool)
        return mempool->max_memory;
    return 0;
}

int HttpPrintMemStats(FILE *fd, char* buffer, PreprocMemInfo *meminfo)
{   
    time_t curr_time = time(NULL);
    int len = 0;
    // Adding stats to be printed , place holder 
    if (fd)
    {
        len = fprintf(fd, ",%lu,%lu,%lu,%lu,%lu,%lu"
               ",%zu,%zu,%zu"
               ",%zu,%zu,%zu"
               ",%zu,%zu,%zu"
               ",%zu,%zu,%zu"
               ",%lu,%u,%u"
               ",%lu,%u,%u"
               ",%lu,%u,%u"
               , hi_stats.session_count
               , hi_stats.post
               , hi_stats.get
               , hi_stats.post_params
               , hi_stats.req_headers
               , hi_stats.resp_headers
               , hi_get_free_mempool(http_mempool)
               , hi_get_used_mempool(http_mempool)
               , hi_get_max_mempool(http_mempool)
               , hi_get_free_mempool(mime_decode_mempool)
               , hi_get_used_mempool(mime_decode_mempool)
               , hi_get_max_mempool(mime_decode_mempool)
               , hi_get_free_mempool(hi_gzip_mempool)
               , hi_get_used_mempool(hi_gzip_mempool)
               , hi_get_max_mempool(hi_gzip_mempool)
               , hi_get_free_mempool(mime_log_mempool)
               , hi_get_used_mempool(mime_log_mempool)
               , hi_get_max_mempool(mime_log_mempool)
               , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
               , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
               , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
               , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
               , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
               , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
               , meminfo[PP_MEM_CATEGORY_MEMPOOL].used_memory
               , meminfo[PP_MEM_CATEGORY_MEMPOOL].num_of_alloc
               , meminfo[PP_MEM_CATEGORY_MEMPOOL].num_of_free
               ); 
        return len;
    }

    if (buffer)
    {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "Memory Statistics of httpinspect on: %s\n"
             " Http Inspect Statistics\n"
             "    Current active session                      : %lu\n"
             "    No of POST methods encountered              : %lu\n"
             "    No of GET methods encountered               : %lu\n"
             "    No of successfully extract post params      : %lu\n"
             "    No of successfully extract request params   : %lu\n"
             "    No of successfully extract response params  : %lu\n"
             "\n  Http Memory Pool:\n"
             "       Free Memory:        %14zu bytes\n"
             "       Used Memory:        %14zu bytes\n"
             "       Max Memory :        %14zu bytes\n"
             "\n  Mime Decode Memory Pool:\n"
             "       Free Memory:        %14zu bytes\n"
             "       Used Memory:        %14zu bytes\n"
             "       Max Memory :        %14zu bytes\n"
             "\n  Http gzip Memory Pool:\n"
             "       Free Memory:        %14zu bytes\n"
             "       Used Memory:        %14zu bytes\n"
             "       Max Memory :        %14zu bytes\n"
             "\n  Http Mime log Memory Pool:\n"
             "       Free Memory:        %14zu bytes\n"
             "       Used Memory:        %14zu bytes\n"
             "       Max Memory :        %14zu bytes\n"
             , ctime(&curr_time)
             , hi_stats.session_count
             , hi_stats.post
             , hi_stats.get
             , hi_stats.post_params
             , hi_stats.req_headers
             , hi_stats.resp_headers
             , hi_get_free_mempool(http_mempool)
             , hi_get_used_mempool(http_mempool)
             , hi_get_max_mempool(http_mempool)
             , hi_get_free_mempool(mime_decode_mempool)
             , hi_get_used_mempool(mime_decode_mempool)
             , hi_get_max_mempool(mime_decode_mempool)
             , hi_get_free_mempool(hi_gzip_mempool)
             , hi_get_used_mempool(hi_gzip_mempool)
             , hi_get_max_mempool(hi_gzip_mempool)
             , hi_get_free_mempool(mime_log_mempool)
             , hi_get_used_mempool(mime_log_mempool)
             , hi_get_max_mempool(mime_log_mempool)
             );
    
    } else {

        LogMessage(" Memory Statistics of Http Inspect on: %s\n ",ctime(&curr_time));
        LogMessage("    Current active session          : %lu\n", hi_stats.session_count);
        LogMessage("    No of POST methods encountered  : %lu\n", hi_stats.post);
        LogMessage("    No of GET methods encountered   : %lu\n", hi_stats.get);
        LogMessage("    No of successfully extract post params      : %lu\n", hi_stats.post_params);
        LogMessage("    No of successfully extract request params   : %lu\n", hi_stats.req_headers);
        LogMessage("    No of successfully extract response params  : %lu\n", hi_stats.resp_headers);
        LogMessage(" Http Memory Pool       :\n");
        LogMessage("      Free Memory:    %14zu bytes\n", hi_get_free_mempool(http_mempool));
        LogMessage("      Used Memory:    %14zu bytes\n", hi_get_used_mempool(http_mempool));
        LogMessage("      Max Memory :    %14zu bytes\n", hi_get_max_mempool(http_mempool));
        LogMessage(" Mime Decode Memory Pool   :\n");
        LogMessage("      Free Memory:    %14zu bytes\n", hi_get_free_mempool(mime_decode_mempool));
        LogMessage("      Used Memory:    %14zu bytes\n", hi_get_used_mempool(mime_decode_mempool));
        LogMessage("      Max Memory :    %14zu bytes\n", hi_get_max_mempool(mime_decode_mempool));
        LogMessage(" Http Gzip Memory Pool     :\n");
        LogMessage("      Free Memory:    %14zu bytes\n", hi_get_free_mempool(hi_gzip_mempool));
        LogMessage("      Used Memory:    %14zu bytes\n", hi_get_used_mempool(hi_gzip_mempool));
        LogMessage("      Max Memory :    %14zu bytes\n", hi_get_max_mempool(hi_gzip_mempool));
        LogMessage(" Http Mime log Memory Pool :\n");
        LogMessage("      Free Memory:    %14zu bytes\n", hi_get_free_mempool(mime_log_mempool));
        LogMessage("      Used Memory:    %14zu bytes\n", hi_get_used_mempool(mime_log_mempool));
        LogMessage("      Max Memory :    %14zu bytes\n", hi_get_max_mempool(mime_log_mempool));
  
    }
 
       return len;
}

static void HttpInspectDropStats(int exiting)
{
    if(!hi_stats.total)
        return;

    LogMessage("HTTP Inspect - encodings (Note: stream-reassembled "
               "packets included):\n");

#ifdef WIN32
    LogMessage("    POST methods:                         %-10I64u\n", hi_stats.post);
    LogMessage("    GET methods:                          %-10I64u\n", hi_stats.get);
    LogMessage("    HTTP Request Headers extracted:       %-10I64u\n", hi_stats.req_headers);
#ifdef DEBUG
    if (hi_stats.req_headers == 0)
    LogMessage("    Avg Request Header length:            %-10s\n", "n/a");
    else
    LogMessage("    Avg Request Header length:            %-10.2f\n", (double)hi_stats.req_header_len / (double)hi_stats.req_headers);
#endif
    LogMessage("    HTTP Request cookies extracted:       %-10I64u\n", hi_stats.req_cookies);
#ifdef DEBUG
    if (hi_stats.req_cookies == 0)
    LogMessage("    Avg Request Cookie length:            %-10s\n", "n/a");
    else
    LogMessage("    Avg Request Cookie length:            %-10.2f\n", (double)hi_stats.req_cookie_len / (double)hi_stats.req_cookies);
#endif
    LogMessage("    Post parameters extracted:            %-10I64u\n", hi_stats.post_params);
    LogMessage("    HTTP Response Headers extracted:      %-10I64u\n", hi_stats.resp_headers);
#ifdef DEBUG
    if (hi_stats.resp_headers == 0)
    LogMessage("    Avg Response Header length:           %-10s\n", "n/a");
    else
    LogMessage("    Avg Response Header length:           %-10.2f\n", (double)hi_stats.resp_header_len / (double)hi_stats.resp_headers);
#endif
    LogMessage("    HTTP Response cookies extracted:      %-10I64u\n", hi_stats.resp_cookies);
#ifdef DEBUG
    if (hi_stats.resp_cookies == 0)
    LogMessage("    Avg Response Cookie length:           %-10s\n", "n/a");
    else
    LogMessage("    Avg Response Cookie length:           %-10.2f\n", (double)hi_stats.resp_cookie_len / (double)hi_stats.resp_cookies);
#endif
    LogMessage("    Unicode:                              %-10I64u\n", hi_stats.unicode);
    LogMessage("    Double unicode:                       %-10I64u\n", hi_stats.double_unicode);
    LogMessage("    Non-ASCII representable:              %-10I64u\n", hi_stats.non_ascii);
    LogMessage("    Directory traversals:                 %-10I64u\n", hi_stats.dir_trav);
    LogMessage("    Extra slashes (\"//\"):                 %-10I64u\n", hi_stats.slashes);
    LogMessage("    Self-referencing paths (\"./\"):        %-10I64u\n", hi_stats.self_ref);
    LogMessage("    HTTP Response Gzip packets extracted: %-10I64u\n", hi_stats.gzip_pkts);
    if (hi_stats.gzip_pkts == 0)
    {
    LogMessage("    Gzip Compressed Data Processed:       %-10s\n", "n/a");
    LogMessage("    Gzip Decompressed Data Processed:     %-10s\n", "n/a");
    }
    else
    {
    LogMessage("    Gzip Compressed Data Processed:       %-10.2f\n", (double)hi_stats.compr_bytes_read);
    LogMessage("    Gzip Decompressed Data Processed:     %-10.2f\n", (double)hi_stats.decompr_bytes_read);
    }
    LogMessage("    Http/2 Rebuilt Packets:               %-10I64u\n", hi_stats.h2_rebuilt_packets);
    LogMessage("    Total packets processed:              %-10I64u\n", hi_stats.total);
    LogMessage("    Non-mempool session memory:           %-10I64u\n", hi_stats.mem_used + 
                                                       (hi_paf_get_size() * hi_stats.session_count));
    LogMessage("    http_mempool used:                    %-10I64u\n",
                                                       http_mempool ? http_mempool->used_memory:0);
    LogMessage("    hi_gzip_mempool used:                    %-10I64u\n",
                                                       hi_gzip_mempool ? hi_gzip_mempool->used_memory:0);
    LogMessage("    mime_decode_mempool used:                    %-10I64u\n",
                                                       mime_decode_mempool ? mime_decode_mempool->used_memory:0);
    LogMessage("    mime_log_mempool used:                    %-10I64u\n",
                                                       mime_log_mempool ? mime_log_mempool->used_memory:0);
    LogMessage("    Current active session:               %-10I64u\n", hi_stats.session_count);
#else
    LogMessage("    POST methods:                         "FMTu64("-10")"\n", hi_stats.post);
    LogMessage("    GET methods:                          "FMTu64("-10")"\n", hi_stats.get);
    LogMessage("    HTTP Request Headers extracted:       "FMTu64("-10")"\n", hi_stats.req_headers);
#ifdef DEBUG
    if (hi_stats.req_headers == 0)
    LogMessage("    Avg Request Header length:            %-10s\n", "n/a");
    else
    LogMessage("    Avg Request Header length:            %-10.2f\n", (double)hi_stats.req_header_len / (double)hi_stats.req_headers);
#endif
    LogMessage("    HTTP Request Cookies extracted:       "FMTu64("-10")"\n", hi_stats.req_cookies);
#ifdef DEBUG
    if (hi_stats.req_cookies == 0)
    LogMessage("    Avg Request Cookie length:            %-10s\n", "n/a");
    else
    LogMessage("    Avg Request Cookie length:            %-10.2f\n", (double)hi_stats.req_cookie_len / (double)hi_stats.req_cookies);
#endif
    LogMessage("    Post parameters extracted:            "FMTu64("-10")"\n", hi_stats.post_params);
    LogMessage("    HTTP response Headers extracted:      "FMTu64("-10")"\n", hi_stats.resp_headers);
#ifdef DEBUG
    if (hi_stats.resp_headers == 0)
    LogMessage("    HTTP response Avg Header length:      %-10s\n", "n/a");
    else
    LogMessage("    Avg Response Header length:           %-10.2f\n", (double)hi_stats.resp_header_len / (double)hi_stats.resp_headers);
#endif
    LogMessage("    HTTP Response Cookies extracted:      "FMTu64("-10")"\n", hi_stats.resp_cookies);
#ifdef DEBUG
    if (hi_stats.resp_cookies == 0)
    LogMessage("    Avg Response Cookie length:           %-10s\n", "n/a");
    else
    LogMessage("    Avg Response Cookie length:           %-10.2f\n", (double)hi_stats.resp_cookie_len / (double)hi_stats.resp_cookies);
#endif
    LogMessage("    Unicode:                              "FMTu64("-10")"\n", hi_stats.unicode);
    LogMessage("    Double unicode:                       "FMTu64("-10")"\n", hi_stats.double_unicode);
    LogMessage("    Non-ASCII representable:              "FMTu64("-10")"\n", hi_stats.non_ascii);
    LogMessage("    Directory traversals:                 "FMTu64("-10")"\n", hi_stats.dir_trav);
    LogMessage("    Extra slashes (\"//\"):                 "FMTu64("-10")"\n", hi_stats.slashes);
    LogMessage("    Self-referencing paths (\"./\"):        "FMTu64("-10")"\n", hi_stats.self_ref);
    LogMessage("    HTTP Response Gzip packets extracted: "FMTu64("-10")"\n", hi_stats.gzip_pkts);
    if (hi_stats.gzip_pkts == 0)
    {
    LogMessage("    Gzip Compressed Data Processed:       %-10s\n", "n/a");
    LogMessage("    Gzip Decompressed Data Processed:     %-10s\n", "n/a");
    }
    else
    {
    LogMessage("    Gzip Compressed Data Processed:       %-10.2f\n", (double)hi_stats.compr_bytes_read);
    LogMessage("    Gzip Decompressed Data Processed:     %-10.2f\n", (double)hi_stats.decompr_bytes_read);
    }
    LogMessage("    Http/2 Rebuilt Packets:               "FMTu64("-10")"\n", hi_stats.h2_rebuilt_packets);
    LogMessage("    Total packets processed:              "FMTu64("-10")"\n", hi_stats.total);
    LogMessage("    Non-mempool session memory:           "FMTu64("-10")"\n", hi_stats.mem_used + 
                                                       (hi_paf_get_size() * hi_stats.session_count));
    LogMessage("    http_mempool used:                    "FMTu64("-10")"\n",
                                                       http_mempool ? http_mempool->used_memory:0);
    LogMessage("    hi_gzip_mempool used:                 "FMTu64("-10")"\n",
                                                       hi_gzip_mempool ? hi_gzip_mempool->used_memory:0);
    LogMessage("    mime_decode_mempool used:             "FMTu64("-10")"\n",
                                                       mime_decode_mempool ? mime_decode_mempool->used_memory:0);
    LogMessage("    mime_log_mempool used:                "FMTu64("-10")"\n",
                                                       mime_log_mempool ? mime_log_mempool->used_memory:0);
    LogMessage("    Current active session:               "FMTu64("-10")"\n", hi_stats.session_count);
#endif
}

static void HttpInspectCleanExit(int signal, void *data)
{
    (void)File_Decomp_CleanExit();

    hi_paf_term();

    HI_SearchFree();

    oldXffFields = xffFields;
    HttpInspectFreeConfigs(hi_config);

    if (mempool_destroy(hi_gzip_mempool) == 0)
    {
        SnortPreprocFree(hi_gzip_mempool, sizeof(MemPool), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_MEMPOOL);
        hi_gzip_mempool = NULL;
    }

    if (mempool_destroy(http_mempool) == 0)
    {
        SnortPreprocFree(http_mempool, sizeof(MemPool), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_MEMPOOL);
        http_mempool = NULL;
    }
    if (mempool_destroy(mime_decode_mempool) == 0)
    {
        free(mime_decode_mempool);
        mime_decode_mempool = NULL;
    }
    if (mempool_destroy(mime_log_mempool) == 0)
    {
        free(mime_log_mempool);
        mime_log_mempool = NULL;
    }
}

static void HttpInspectReset(int signal, void *data)
{
    return;
}

static void HttpInspectResetStats(int signal, void *data)
{
    memset(&hi_stats, 0, sizeof(hi_stats));
}

static void CheckGzipConfig(HTTPINSPECT_GLOBAL_CONF *pPolicyConfig,
        tSfPolicyUserContextId context)
{
    HTTPINSPECT_GLOBAL_CONF *defaultConfig =
        (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(context);

    if (pPolicyConfig == defaultConfig)
    {
        if (!pPolicyConfig->max_gzip_mem)
            pPolicyConfig->max_gzip_mem = DEFAULT_MAX_GZIP_MEM;

        if (!pPolicyConfig->compr_depth)
            pPolicyConfig->compr_depth = DEFAULT_COMP_DEPTH;

        if (!pPolicyConfig->decompr_depth)
            pPolicyConfig->decompr_depth = DEFAULT_DECOMP_DEPTH;

        /* Until we determine exact usage of extract_gzip and file_decomp options
           we will set the max_gzip_sessions to the minimal/conservative value. */
        pPolicyConfig->max_gzip_sessions = pPolicyConfig->max_gzip_mem /
                              (sizeof(DECOMPRESS_STATE) + sizeof(fd_session_t));
    }
    else if (defaultConfig == NULL)
    {
        if (pPolicyConfig->max_gzip_mem)
        {
            FatalError("http_inspect: max_gzip_mem must be "
                    "configured in the default policy.\n");
        }

        if (pPolicyConfig->compr_depth)
        {
            FatalError("http_inspect: compress_depth must be "
                    "configured in the default policy.\n");
        }

        if (pPolicyConfig->decompr_depth)
        {
            FatalError("http_inspect: decompress_depth must be "
                    "configured in the default policy.\n");
        }
    }
    else
    {
        pPolicyConfig->max_gzip_mem = defaultConfig->max_gzip_mem;
        pPolicyConfig->compr_depth = defaultConfig->compr_depth;
        pPolicyConfig->decompr_depth = defaultConfig->decompr_depth;
        pPolicyConfig->max_gzip_sessions = defaultConfig->max_gzip_sessions;
    }
}


static void CheckMemcap(HTTPINSPECT_GLOBAL_CONF *pPolicyConfig,
        tSfPolicyUserContextId context)
{
    HTTPINSPECT_GLOBAL_CONF *defaultConfig =
        (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(context);

    if (pPolicyConfig == defaultConfig)
    {
        if (!pPolicyConfig->memcap)
            pPolicyConfig->memcap = DEFAULT_HTTP_MEMCAP;

    }
    else if (defaultConfig == NULL)
    {
        if (pPolicyConfig->memcap)
        {
            FatalError("http_inspect: memcap must be "
                    "configured in the default policy.\n");
        }

    }
    else
    {
        pPolicyConfig->memcap = defaultConfig->memcap;
    }
}

#ifdef REG_TEST
static inline void PrintHTTPSize(void)
{
    LogMessage("\nHTTP Session Size: %lu\n", (long unsigned int)sizeof(HttpSessionData));
}
#endif

/*
 **  NAME
 **    HttpInspectInit::
*/
/**
**  This function initializes HttpInspect with a user configuration.
**
**  The function is called when HttpInspect is configured in
**  snort.conf.  It gets passed a string of arguments, which gets
**  parsed into configuration constructs that HttpInspect understands.
**
**  This function gets called for every HttpInspect configure line.  We
**  use this characteristic to split up the configuration, so each line
**  is a configuration construct.  We need to keep track of what part
**  of the configuration has been configured, so we don't configure one
**  part, then configure it again.
**
**  Any upfront memory is allocated here (if necessary).
**
**  @param args a string to the preprocessor arguments.
**
**  @return void
*/
static void HttpInspectInit(struct _SnortConfig *sc, char *args)
{
    char ErrorString[ERRSTRLEN];
    int  iErrStrLen = ERRSTRLEN;
    int  iRet;
    char *pcToken;
    char *saveptr;
    HTTPINSPECT_GLOBAL_CONF *pPolicyConfig = NULL;
    tSfPolicyId policy_id = getParserPolicy(sc);

    ErrorString[0] = '\0';

#ifdef REG_TEST
    PrintHTTPSize();
#endif

    if ((args == NULL) || (strlen(args) == 0))
        ParseError("No arguments to HttpInspect configuration.");

    /* Find out what is getting configured */
    pcToken = strtok_r(args, CONF_SEPARATORS, &saveptr);
    if (pcToken == NULL)
    {
        FatalError("%s(%d)strtok returned NULL when it should not.",
                   __FILE__, __LINE__);
    }

    if (!xffFields)
    {
        xffFields = SnortPreprocAlloc(1, HTTP_MAX_XFF_FIELDS * sizeof(char *), 
                         PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);
        if (xffFields == NULL)
        {
            FatalError("http_inspect: %s(%d) failed to allocate memory for XFF fields\n", 
                       __FILE__, __LINE__);
        }
    }

    if (hi_config == NULL)
    {
        hi_config = sfPolicyConfigCreate();
        memset(&hi_stats, 0, sizeof(HIStats));

        /*
         **  Remember to add any cleanup functions into the appropriate
         **  lists.
         */
        AddFuncToPreprocCleanExitList(HttpInspectCleanExit, NULL, PRIORITY_APPLICATION, PP_HTTPINSPECT);
        AddFuncToPreprocResetList(HttpInspectReset, NULL, PRIORITY_APPLICATION, PP_HTTPINSPECT);
        AddFuncToPreprocResetStatsList(HttpInspectResetStats, NULL, PRIORITY_APPLICATION, PP_HTTPINSPECT);
        AddFuncToConfigCheckList(sc, HttpInspectCheckConfig);

        RegisterPreprocStats("http_inspect", HttpInspectDropStats);

#ifdef PERF_PROFILING
        RegisterPreprocessorProfile("httpinspect", &hiPerfStats, 0, &totalPerfStats, NULL);
        RegisterPreprocessorProfile("http2inspect", &hi2PerfStats, 0, &totalPerfStats, NULL);
        RegisterPreprocessorProfile("h2_init", &hi2InitPerfStats, 1, &hi2PerfStats, NULL);
        RegisterPreprocessorProfile("h2_payload", &hi2PayloadPerfStats, 1, &hi2PerfStats, NULL);
        RegisterPreprocessorProfile("h2_pseudo", &hi2PseudoPerfStats, 1, &hi2PerfStats, NULL);
#endif

#ifdef TARGET_BASED
        /* Find and cache protocol ID for packet comparison */
        hi_app_protocol_id = AddProtocolReference("http");
        h2_app_protocol_id = AddProtocolReference("http2");
        // register with session to handle applications
        session_api->register_service_handler( PP_HTTPINSPECT, hi_app_protocol_id );
        session_api->register_service_handler( PP_HTTPINSPECT, h2_app_protocol_id);

#endif
        hi_paf_init(0);  // FIXTHIS is cap needed?
        HI_SearchInit();
    }

    /*
    **  Global Configuration Processing
    **  We only process the global configuration once, but always check for
    **  user mistakes, like configuring more than once.  That's why we
    **  still check for the global token even if it's been checked.
    **  Force the first configuration to be the global one.
    */
    sfPolicyUserPolicySet (hi_config, policy_id);
    pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(hi_config);
    if (pPolicyConfig == NULL)
    {
        if (strcasecmp(pcToken, GLOBAL) != 0)
        {
            ParseError("Must configure the http inspect global "
                       "configuration first.");
        }

        HttpInspectRegisterRuleOptions(sc);

        pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *) SnortPreprocAlloc(1, 
                                                         sizeof(HTTPINSPECT_GLOBAL_CONF), 
                                                         PP_HTTPINSPECT, 
                                                         PP_MEM_CATEGORY_CONFIG);
        if (!pPolicyConfig)
        {
             ParseError("HTTP INSPECT preprocessor: memory allocate failed.\n");
        }

        sfPolicyUserDataSetCurrent(hi_config, pPolicyConfig);

        iRet = HttpInspectInitializeGlobalConfig(pPolicyConfig,
                                                 ErrorString, iErrStrLen);
        if (iRet == 0)
        {
            iRet = ProcessGlobalConf(pPolicyConfig, ErrorString, iErrStrLen, &saveptr);

            if (iRet == 0)
            {
                CheckGzipConfig(pPolicyConfig, hi_config);
                CheckMemcap(pPolicyConfig, hi_config);
                PrintGlobalConf(pPolicyConfig);

                /* Add HttpInspect into the preprocessor list */
                if ( pPolicyConfig->disabled )
                    return;
                AddFuncToPreprocList(sc, HttpInspect, PRIORITY_APPLICATION, PP_HTTPINSPECT, PROTO_BIT__TCP);
            }
        }
    }
    else
    {
        if (strcasecmp(pcToken, SERVER) != 0)
        {
            if (strcasecmp(pcToken, GLOBAL) != 0)
                ParseError("Must configure the http inspect global configuration first.");
            else
                ParseError("Invalid http inspect token: %s.", pcToken);
        }

        iRet = ProcessUniqueServerConf(sc, pPolicyConfig, ErrorString, iErrStrLen, &saveptr);
    }



    if (iRet)
    {
        if(iRet > 0)
        {
            /*
            **  Non-fatal Error
            */
            if(*ErrorString)
            {
                ErrorMessage("%s(%d) => %s\n",
                        file_name, file_line, ErrorString);
            }
        }
        else
        {
            /*
            **  Fatal Error, log error and exit.
            */
            if(*ErrorString)
            {
                FatalError("%s(%d) => %s\n",
                        file_name, file_line, ErrorString);
            }
            else
            {
                /*
                **  Check if ErrorString is undefined.
                */
                if(iRet == -2)
                {
                    FatalError("%s(%d) => ErrorString is undefined.\n",
                            file_name, file_line);
                }
                else
                {
                    FatalError("%s(%d) => Undefined Error.\n",
                            file_name, file_line);
                }
            }
        }
    }

}

/*
**  NAME
**    SetupHttpInspect::
*/
/**
**  This function initializes HttpInspect as a Snort preprocessor.
**
**  It registers the preprocessor keyword for use in the snort.conf
**  and sets up the initialization module for the preprocessor, in
**  case it is configured.
**
**  This function must be called in InitPreprocessors() in plugbase.c
**  in order to be recognized by Snort.
**
**  @param none
**
**  @return void
*/
void SetupHttpInspect(void)
{
RegisterMemoryStatsFunction(PP_HTTPINSPECT, HttpPrintMemStats);

#ifndef SNORT_RELOAD
    RegisterPreprocessor(GLOBAL_KEYWORD, HttpInspectInit);
    RegisterPreprocessor(SERVER_KEYWORD, HttpInspectInit);
#else
    RegisterPreprocessor(GLOBAL_KEYWORD, HttpInspectInit, HttpInspectReloadGlobal,
                         HttpInspectReloadVerify, HttpInspectReloadSwap,
                         HttpInspectReloadSwapFree);
    RegisterPreprocessor(SERVER_KEYWORD, HttpInspectInit,
                         HttpInspectReload, NULL, NULL, NULL);
#endif
#ifdef DUMP_BUFFER
    RegisterBufferTracer(getHTTPDumpBuffers, HTTP_BUFFER_DUMP_FUNC);
#endif
    InitLookupTables();
    InitJSNormLookupTable();
    (void)File_Decomp_OneTimeInit();

    DEBUG_WRAP(DebugMessage(DEBUG_HTTPINSPECT, "Preprocessor: HttpInspect is "
                "setup . . .\n"););
}

static void HttpInspectRegisterRuleOptions(struct _SnortConfig *sc)
{
    RegisterPreprocessorRuleOption(sc, "http_encode", &HttpEncodeInit,
                                    &HttpEncodeEval, &HttpEncodeCleanup , NULL, NULL, NULL, NULL );
}

static void HttpInspectRegisterXtraDataFuncs(HTTPINSPECT_GLOBAL_CONF *pPolicyConfig)
{
    if (!stream_api || !pPolicyConfig)
        return;

    pPolicyConfig->xtra_trueip_id = stream_api->reg_xtra_data_cb(GetHttpTrueIP);
    pPolicyConfig->xtra_uri_id = stream_api->reg_xtra_data_cb(GetHttpUriData);
    pPolicyConfig->xtra_hname_id = stream_api->reg_xtra_data_cb(GetHttpHostnameData);
#ifndef SOURCEFIRE
    pPolicyConfig->xtra_gzip_id = stream_api->reg_xtra_data_cb(GetHttpGzipData);
    pPolicyConfig->xtra_jsnorm_id = stream_api->reg_xtra_data_cb(GetHttpJSNormData);
#endif

}

static void updateConfigFromFileProcessing (struct _SnortConfig *sc, HTTPINSPECT_GLOBAL_CONF *pPolicyConfig)
{
    HTTPINSPECT_CONF *ServerConf = pPolicyConfig->global_server;
    /*Either one is unlimited*/
    int64_t fileDepth = file_api->get_max_file_depth(sc, true);

    /*Config file policy*/
    if (fileDepth > -1)
    {
        ServerConf->inspect_response = 1;
        ServerConf->extract_gzip = 1;
        ServerConf->log_uri = 1;
        ServerConf->unlimited_decompress = 1;
        pPolicyConfig->mime_conf.log_filename = 1;
        ServerConf->file_policy = 1;
    }

    if (!fileDepth || (!ServerConf->server_flow_depth))
        ServerConf->server_extract_size = 0;
    else if (ServerConf->server_flow_depth > fileDepth)
        ServerConf->server_extract_size = ServerConf->server_flow_depth;
    else
        ServerConf->server_extract_size = fileDepth;

    if (!fileDepth || (!ServerConf->post_depth))
        ServerConf->post_extract_size = 0;
    else if (ServerConf->post_depth > fileDepth)
        ServerConf->post_extract_size = ServerConf->post_depth;
    else
        ServerConf->post_extract_size = fileDepth;

}

static int HttpInspectVerifyPolicy(struct _SnortConfig *sc, tSfPolicyUserContextId config,
        tSfPolicyId policyId, void* pData)
{
    HTTPINSPECT_GLOBAL_CONF *pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *)pData;

    HttpInspectRegisterXtraDataFuncs(pPolicyConfig);

    if ( pPolicyConfig->disabled )
        return 0;

    if (!stream_api || (stream_api->version < STREAM_API_VERSION5))
    {
        ErrorMessage("HttpInspectConfigCheck() Streaming & reassembly "
                     "must be enabled\n");
        return -1;
    }


    if (pPolicyConfig->global_server == NULL)
    {
        ErrorMessage("HttpInspectConfigCheck() default server configuration "
                     "not specified\n");
        return -1;
    }

#ifdef TARGET_BASED
    HttpInspectAddServicesOfInterest(sc, pPolicyConfig, policyId);
#endif
    updateConfigFromFileProcessing(sc, pPolicyConfig);
    HttpInspectAddPortsOfInterest(sc, pPolicyConfig, policyId);
#if defined(FEAT_OPEN_APPID)
    if (IsAnybodyRegisteredForHttpHeader())
    {
        pPolicyConfig->global_server->appid_enabled = 1;
    }
#endif /* defined(FEAT_OPEN_APPID) */
    return 0;
}


/** Add ports configured for http preprocessor to stream5 port filtering so that if
 * any_any rules are being ignored them the the packet still reaches http-inspect.
 *
 * For ports in global_server configuration, server_lookup,
 * add the port to stream5 port filter list.
 */
static void HttpInspectAddPortsOfInterest(struct _SnortConfig *sc, HTTPINSPECT_GLOBAL_CONF *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return;

    httpCurrentPolicy = policy_id;

    addServerConfPortsToStream(sc, (void *)config->global_server);
    hi_ui_server_iterate(sc, config->server_lookup, addServerConfPortsToStream);
}

/**Add server ports from http_inspect preprocessor from snort.conf file to pass through
 * port filtering.
 */
static void addServerConfPortsToStream(struct _SnortConfig *sc, void *pData)
{
    unsigned int i;

    HTTPINSPECT_CONF *pConf = (HTTPINSPECT_CONF *)pData;
    if (pConf)
    {
        for (i = 0; i < MAXPORTS; i++)
        {
            if( isPortEnabled( pConf->ports, i ) )
            {
                bool client = (pConf->client_flow_depth > -1);
                bool server = (pConf->server_extract_size > -1);
                int64_t fileDepth = file_api->get_max_file_depth(sc, true);

                //Add port the port
                stream_api->set_port_filter_status(sc, IPPROTO_TCP,
                                                   (uint16_t) i, 
                                                   PORT_MONITOR_SESSION,
                                                   httpCurrentPolicy, 
                                                   1);

                // there is a fundamental issue here in that both hi and s5
                // can configure ports per ip independently of each other.
                // as is, we enable paf for all http servers if any server
                // has a flow depth enabled (per direction).  still, if eg
                // all server_flow_depths are -1, we will only enable client.
                if (fileDepth > 0)
                {
                    hi_paf_register_port(sc, (uint16_t)i, client, server, httpCurrentPolicy, true);
#ifdef HAVE_LIBNGHTTP2
                    if (pConf->h2_mode)
                        h2_paf_register_port(sc, (uint16_t)i, client, server, httpCurrentPolicy, true);
#endif /* HAVE_LIBNGHTTP2 */
                }
                else
                {
                    hi_paf_register_port(sc, (uint16_t)i, client, server, httpCurrentPolicy, false);
#ifdef HAVE_LIBNGHTTP2
                    if (pConf->h2_mode)
                        h2_paf_register_port(sc, (uint16_t)i, client, server, httpCurrentPolicy, false);
#endif /* HAVE_LIBNGHTTP2 */
                }
            }
        }
    }
}

#ifdef TARGET_BASED
/**
 * @param service ordinal number of service.
 */
static void HttpInspectAddServicesOfInterest(struct _SnortConfig *sc, HTTPINSPECT_GLOBAL_CONF *config, tSfPolicyId policy_id)
{
    if ((config == NULL) || (!config->global_server))
        return;

    /* Add ordinal number for the service into stream5 */
    if (hi_app_protocol_id != SFTARGET_UNKNOWN_PROTOCOL)
    {
        stream_api->set_service_filter_status(sc, hi_app_protocol_id, PORT_MONITOR_SESSION, policy_id, 1);

        if (file_api->get_max_file_depth(sc, true) > 0)
        {
            hi_paf_register_service(sc, hi_app_protocol_id, true, true, policy_id, true);
#ifdef HAVE_LIBNGHTTP2
            if (config->global_server->h2_mode)
                h2_paf_register_service(sc, hi_app_protocol_id, true, true, policy_id, true);
#endif
        }
        else
        {
            hi_paf_register_service(sc, hi_app_protocol_id, true, true, policy_id, false);
#ifdef HAVE_LIBNGHTTP2
            if (config->global_server->h2_mode)
                h2_paf_register_service(sc, hi_app_protocol_id, true, true, policy_id, false);
#endif
        }
    }

/*
#ifdef HAVE_LIBNGHTTP2
    if ((config == NULL) || (!config->global_server))
        return;

    if ((h2_app_protocol_id != SFTARGET_UNKNOWN_PROTOCOL) && (config->global_server->h2_mode))
    {
        stream_api->set_service_filter_status(sc, h2_app_protocol_id, PORT_MONITOR_SESSION, policy_id, 1);

        if (file_api->get_max_file_depth() > 0)
            h2_paf_register_service(sc, h2_app_protocol_id, true, true, policy_id, true);
        else
            h2_paf_register_service(sc, h2_app_protocol_id, true, true, policy_id, false);
    }

#endif */ /* HAVE_LIBNGHTTP2 */
}
#endif

typedef struct _HttpEncodeData
{
    int http_type;
    int encode_type;
}HttpEncodeData;

static int HttpEncodeInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr)
{
    char **toks, **toks1;
    int num_toks, num_toks1;
    int i;
    char *etype;
    char *btype;
    char *findStr1, *findStr2;
    int negate_flag = 0;
    unsigned pos;
    HttpEncodeData *idx= NULL;

    idx = (HttpEncodeData *) SnortPreprocAlloc(1, sizeof(HttpEncodeData), PP_HTTPINSPECT, 
                                  PP_MEM_CATEGORY_CONFIG);
    hi_stats.mem_used += sizeof(HttpEncodeData);

    if(idx == NULL)
    {
        FatalError("%s(%d): Failed allocate data for %s option\n",
            file_name, file_line, name);
    }


    toks = mSplit(parameters, ",", 2, &num_toks, 0);

    if(num_toks != 2 )
    {
        FatalError("%s (%d): %s option takes two parameters \n",
            file_name, file_line, name);
    }

    btype = toks[0];
    if(!strcasecmp(btype, "uri"))
    {
        idx->http_type = HTTP_BUFFER_URI;
    }
    else if(!strcasecmp(btype, "header"))
    {
        idx->http_type = HTTP_BUFFER_HEADER;
    }
    /* This keyword will not be used until post normalization is turned on */
    /*else if(!strcasecmp(btype, "post"))
    {
        idx->http_type = HTTP_BUFFER_CLIENT_BODY;
    }*/
    else if(!strcasecmp(btype, "cookie"))
    {
        idx->http_type = HTTP_BUFFER_COOKIE;
    }
    /*check for a negation when OR is present. OR and negation is not supported*/
    findStr1 = strchr(toks[1], '|');
    if( findStr1 )
    {
        findStr2 = strchr(toks[1], '!' );
        if( findStr2 )
        {
            FatalError("%s (%d): \"|\" is not supported in conjunction with \"!\" for %s option \n",
                    file_name, file_line, name);
        }

        pos = findStr1 - toks[1];
        if ( pos == 0 || pos == (strlen(toks[1]) - 1) )
        {
            FatalError("%s (%d): Invalid Parameters for %s option \n",
                    file_name, file_line, name);
        }
    }

     toks1 = mSplit(toks[1], "|", 0, &num_toks1, 0);

     for(i = 0; i < num_toks1; i++)
     {
         etype = toks1[i];

         if( *etype == '!' )
         {
             negate_flag = 1;
             etype++;
             while(isspace((int)*etype)) {etype++;}
         }

         if(!strcasecmp(etype, "utf8"))
         {
             if(negate_flag)
                 idx->encode_type &= ~HTTP_ENCODE_TYPE__UTF8_UNICODE;
             else
                 idx->encode_type |= HTTP_ENCODE_TYPE__UTF8_UNICODE;
         }

         else if(!strcasecmp(etype, "double_encode"))
         {
             if(negate_flag)
                 idx->encode_type &= ~HTTP_ENCODE_TYPE__DOUBLE_ENCODE;
             else idx->encode_type |= HTTP_ENCODE_TYPE__DOUBLE_ENCODE;
         }

         else if(!strcasecmp(etype, "non_ascii"))
         {
             if(negate_flag) idx->encode_type &= ~HTTP_ENCODE_TYPE__NONASCII;
             else
                 idx->encode_type |= HTTP_ENCODE_TYPE__NONASCII;
         }

         /* Base 36 is deprecated and essentially a noop */
         else if(!strcasecmp(etype, "base36"))
         {
             ErrorMessage("WARNING: %s (%d): The \"base36\" argument to the "
                     "\"http_encode\" rule option is deprecated and void "
                     "of functionality.\n", file_name, file_line);

             /* Set encode type so we can check below to see if base36 was the
              * only argument in the encode chain */
             idx->encode_type |= HTTP_ENCODE_TYPE__BASE36;
         }

         else if(!strcasecmp(etype, "uencode"))
         {
             if(negate_flag)
                 idx->encode_type &= ~HTTP_ENCODE_TYPE__UENCODE;
             else
                 idx->encode_type |= HTTP_ENCODE_TYPE__UENCODE;
         }

         else if(!strcasecmp(etype, "bare_byte"))
         {
             if(negate_flag)
                 idx->encode_type &= ~HTTP_ENCODE_TYPE__BARE_BYTE;
             else
                 idx->encode_type |= HTTP_ENCODE_TYPE__BARE_BYTE;
         }
         else if (!strcasecmp(etype, "iis_encode"))
         {
             if(negate_flag)
                 idx->encode_type &= ~HTTP_ENCODE_TYPE__IIS_UNICODE;
             else
                 idx->encode_type |= HTTP_ENCODE_TYPE__IIS_UNICODE;
         }
         else if  (!strcasecmp(etype, "ascii"))
         {
             if(negate_flag)
                 idx->encode_type &= ~HTTP_ENCODE_TYPE__ASCII;
             else
                 idx->encode_type |= HTTP_ENCODE_TYPE__ASCII;
         }

         else
         {
             FatalError("%s(%d): Unknown modifier \"%s\" for option \"%s\"\n",
                     file_name, file_line, toks1[i], name);
         }
         negate_flag = 0;
     }

     /* Only got base36 parameter which is deprecated.  If it's the only
      * parameter in the chain make it so it always matches as if the
      * entire rule option were non-existent. */
     if (idx->encode_type == HTTP_ENCODE_TYPE__BASE36)
     {
         idx->encode_type = 0xffffffff;
     }

     *dataPtr = idx;
     mSplitFree(&toks,num_toks);
     mSplitFree(&toks1,num_toks1);

     return 0;
}


static int HttpEncodeEval(void *p, const uint8_t **cursor, void *dataPtr)
{
    Packet* pkt = p;
    HttpEncodeData* idx = (HttpEncodeData *)dataPtr;
    const HttpBuffer* hb;

    if ( !pkt || !idx )
        return DETECTION_OPTION_NO_MATCH;

    hb = GetHttpBuffer(idx->http_type);

    if ( hb && (hb->encode_type & idx->encode_type) )
        return DETECTION_OPTION_MATCH;

    return DETECTION_OPTION_NO_MATCH;
}

static void HttpEncodeCleanup(void *dataPtr)
{
    HttpEncodeData *idx = dataPtr;
    if (idx)
    {
        SnortPreprocFree(idx, sizeof(HttpEncodeData), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
        hi_stats.mem_used -= sizeof(HttpEncodeData);
    }
}

static int HttpInspectFileDecompIterate(void *data)
{
    HTTPINSPECT_CONF *server = (HTTPINSPECT_CONF *)data;

    if (server == NULL)
        return 0;

    if (server->file_decomp_modes != 0)
        return 1;

    return 0;
}

static int HttpInspectFileDecomp(struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, void *pData)
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if ((context->global_server != NULL) && (context->global_server->file_decomp_modes != 0))
        return 1;

    if (context->server_lookup != NULL)
    {
        if (sfrt_iterate2(context->server_lookup, HttpInspectFileDecompIterate) != 0)
            return 1;
    }

    return 0;
}


static int HttpInspectExtractGzipIterate(void *data)
{
    HTTPINSPECT_CONF *server = (HTTPINSPECT_CONF *)data;

    if (server == NULL)
        return 0;

    if (server->extract_gzip)
        return 1;

    return 0;
}

static int HttpInspectExtractGzip(struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, void *pData)
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if ((context->global_server != NULL) && context->global_server->extract_gzip)
        return 1;

    if (context->server_lookup != NULL)
    {
        if (sfrt_iterate2(context->server_lookup, HttpInspectExtractGzipIterate) != 0)
            return 1;
    }

    return 0;
}

static int HttpInspectExtractUriHostIterate(void *data)
{
    HTTPINSPECT_CONF *server = (HTTPINSPECT_CONF *)data;

    if (server == NULL)
        return 0;

#if defined(FEAT_OPEN_APPID)
    if (server->log_uri || server->log_hostname || server->appid_enabled)
#else
    if (server->log_uri || server->log_hostname)
#endif /* defined(FEAT_OPEN_APPID) */
        return 1;

    return 0;
}

static int HttpInspectExtractUriHost(struct _SnortConfig *sc,
                tSfPolicyUserContextId config,
                tSfPolicyId policyId, void *pData)
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

#if defined(FEAT_OPEN_APPID)
    if ((context->global_server != NULL) && (context->global_server->log_uri || context->global_server->log_hostname || context->global_server->appid_enabled))
#else
    if ((context->global_server != NULL) && (context->global_server->log_uri || context->global_server->log_hostname))
#endif /* defined(FEAT_OPEN_APPID) */
        return 1;

    if (context->server_lookup != NULL)
    {
        if (sfrt_iterate2(context->server_lookup, HttpInspectExtractUriHostIterate) != 0)
            return 1;
    }

    return 0;
}

static int HttpEnableDecoding(struct _SnortConfig *sc,
            tSfPolicyUserContextId config,
            tSfPolicyId policyId, void *pData)
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if((context->global_server != NULL) && (context->global_server->post_extract_size > -1)
            && (file_api->is_decoding_enabled(&(context->decode_conf))))
        return 1;

    return 0;
}

static int HttpEnableMimeLog(struct _SnortConfig *sc,
            tSfPolicyUserContextId config,
            tSfPolicyId policyId, void *pData)
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if((context->global_server != NULL) && (context->global_server->post_extract_size > -1)
            && (file_api->is_mime_log_enabled(&(context->mime_conf))))
        return 1;

    return 0;
}

static int ProcessGzipAndFDMemPools( struct _SnortConfig *sc,
                                      tSfPolicyUserContextId my_hi_config,
                                      HTTPINSPECT_GLOBAL_CONF *my_defaultConfig )
{
    bool have_gzip, have_fd;
    uint32_t max_sessions = 0;
    uint32_t block_size = 0;

    have_fd = (sfPolicyUserDataIterate(sc, my_hi_config, HttpInspectFileDecomp) != 0);
    have_gzip = (sfPolicyUserDataIterate(sc, my_hi_config, HttpInspectExtractGzip) != 0);

    if( have_fd || have_gzip )
    {
        if (my_defaultConfig == NULL)
        {
            WarningMessage("http_inspect: Must configure a default global "
                           "configuration if you want to enable gzip or file decomp in any "
                           "server configuration.\n");
            return( -1 );
        }

        if( have_fd )
            block_size += sizeof( fd_session_t );
        if( have_gzip )
            block_size += sizeof( DECOMPRESS_STATE );

        if( block_size > my_defaultConfig->max_gzip_mem )
            FatalError("http_inspect: Error setting the \"max_gzip_mem\" \n");

        max_sessions = my_defaultConfig->max_gzip_mem / block_size;
        my_defaultConfig->max_gzip_sessions = max_sessions;

        if( have_fd )
        {
            hi_fd_conf.Max_Memory = (max_sessions * sizeof( fd_session_t ));
            if( File_Decomp_Config(&(hi_fd_conf)) != File_Decomp_OK )
                FatalError("http_inspect: Could not allocate file decomp mempool.\n");
        }
        else
            hi_fd_conf.fd_MemPool = NULL;

        if( have_gzip )
        {
            hi_gzip_mempool = (MemPool *)SnortPreprocAlloc(1, sizeof(MemPool), 
                                              PP_HTTPINSPECT, 
                                              PP_MEM_CATEGORY_MEMPOOL);

            if( (hi_gzip_mempool == 0) ||
                (mempool_init(hi_gzip_mempool, max_sessions,
                              sizeof(DECOMPRESS_STATE)) != 0) )
                FatalError("http_inspect: Could not allocate gzip mempool.\n");
        }
        else
            hi_gzip_mempool = NULL;
    }
    return( 0 );
 }

static int CheckFilePolicyConfig(
        struct _SnortConfig *sc,
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF*)pData;

    context->decode_conf.file_depth = file_api->get_max_file_depth(sc, true);
    if (context->decode_conf.file_depth > -1)
        context->mime_conf.log_filename = 1;
    updateMaxDepth(context->decode_conf.file_depth, &context->decode_conf.max_depth);

    return 0;
}

/*
**  NAME
**    HttpInspectCheckConfig::
*/
/**
**  This function verifies the HttpInspect configuration is complete
**
**  @return none
*/
static int HttpInspectCheckConfig(struct _SnortConfig *sc)
{
    HTTPINSPECT_GLOBAL_CONF *defaultConfig;

    if (hi_config == NULL)
        return 0;

    if (sfPolicyUserDataIterate (sc, hi_config, HttpInspectVerifyPolicy))
        return -1;

    if (sfPolicyUserDataIterate (sc, hi_config, CheckFilePolicyConfig))
        return -1;

    defaultConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(hi_config);

    if( ProcessGzipAndFDMemPools( sc, hi_config, defaultConfig ) != 0 )
        return( -1 );

    if (sfPolicyUserDataIterate(sc, hi_config, HttpInspectExtractUriHost) != 0)
    {
        uint32_t max_sessions_logged;
        if (defaultConfig == NULL)
        {
            WarningMessage("http_inspect:  Must configure a default global "
                        "configuration if you want to enable logging of uri or hostname in any "
                        "server configuration.\n");
            return -1;
        }

        max_sessions_logged = defaultConfig->memcap / (MAX_URI_EXTRACTED + MAX_HOSTNAME);

        http_mempool = (MemPool *)SnortPreprocAlloc(1, sizeof(MemPool), PP_HTTPINSPECT, 
                                       PP_MEM_CATEGORY_MEMPOOL); 
        if (mempool_init(http_mempool, max_sessions_logged, (MAX_URI_EXTRACTED + MAX_HOSTNAME)) != 0)
        {
            FatalError("http_inspect:  Could not allocate HTTP mempool.\n");
        }
    }

    if (sfPolicyUserDataIterate(sc, hi_config, HttpEnableDecoding) != 0)
    {
        if (defaultConfig == NULL)
        {
            WarningMessage("http_inspect:  Must configure a default global "
                    "configuration if you want to enable decoding in any "
                    "server configuration.\n");
            return -1;
        }
        mime_decode_mempool = (MemPool *)file_api->init_mime_mempool(defaultConfig->decode_conf.max_mime_mem,
                defaultConfig->decode_conf.max_depth, mime_decode_mempool, PROTOCOL_NAME);
    }

    if (sfPolicyUserDataIterate(sc, hi_config, HttpEnableMimeLog) != 0)
    {
        if (defaultConfig == NULL)
        {
            ErrorMessage("http_inspect:  Must configure a default global "
                    "configuration if you want to enable mime log in any "
                    "server configuration.\n");
            return -1;
        }
        mime_log_mempool = (MemPool *)file_api->init_log_mempool(0,
                defaultConfig->mime_conf.memcap, mime_log_mempool, "HTTP");
    }
    return 0;
}

static int HttpInspectFreeConfigPolicy(tSfPolicyUserContextId config,tSfPolicyId policyId, void* pData )
{
    HTTPINSPECT_GLOBAL_CONF *pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *)pData;
    HttpInspectFreeConfig(pPolicyConfig);
    sfPolicyUserDataClear (config, policyId);
    return 0;
}

static void HttpInspectFreeConfigs(tSfPolicyUserContextId config)
{
    int i;

    if(oldXffFields)
    {
        for (i = 0; (i < HTTP_MAX_XFF_FIELDS) && (oldXffFields[i]); i++)
        {
            free(oldXffFields[i]);
            oldXffFields[i] = NULL;
        }

        SnortPreprocFree(oldXffFields, HTTP_MAX_XFF_FIELDS * sizeof(char *),
             PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);
        oldXffFields = NULL;
    }

    if (config == NULL)
        return;
    sfPolicyUserDataFreeIterate (config, HttpInspectFreeConfigPolicy);
    sfPolicyConfigDelete(config);

}

static void HttpInspectFreeConfig(HTTPINSPECT_GLOBAL_CONF *config)
{
    if (config == NULL)
        return;

    hi_ui_server_lookup_destroy(config->server_lookup);

    xfree(config->iis_unicode_map_filename);
    xfree(config->iis_unicode_map);

    if (config->global_server != NULL)
    {
        int i;
        for( i=0; i<HTTP_MAX_XFF_FIELDS; i++ )
            if( config->global_server->xff_headers[i] != NULL )
            {
                free(  config->global_server->xff_headers[i] ); 
                config->global_server->xff_headers[i] = NULL;
            }

        http_cmd_lookup_cleanup(&(config->global_server->cmd_lookup));
        SnortPreprocFree(config->global_server, sizeof(HTTPINSPECT_CONF), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_CONFIG);
    }

    SnortPreprocFree(config, sizeof(HTTPINSPECT_GLOBAL_CONF), 
         PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);
}

#ifdef SNORT_RELOAD
static void _HttpInspectReload(struct _SnortConfig *sc, tSfPolicyUserContextId hi_swap_config, char *args)
{
    char ErrorString[ERRSTRLEN];
    int  iErrStrLen = ERRSTRLEN;
    int  iRet;
    HTTPINSPECT_GLOBAL_CONF *pPolicyConfig = NULL;
    char *pcToken;
    char *saveptr;
    tSfPolicyId policy_id = getParserPolicy(sc);

    ErrorString[0] = '\0';

    if ((args == NULL) || (strlen(args) == 0))
        ParseError("No arguments to HttpInspect configuration.");

    /* Find out what is getting configured */
    pcToken = strtok_r(args, CONF_SEPARATORS, &saveptr);
    if (pcToken == NULL)
    {
        FatalError("%s(%d)strtok returned NULL when it should not.",
                   __FILE__, __LINE__);
    }

    if (!oldXffFields)
    {
        oldXffFields = xffFields;
        xffFields = SnortPreprocAlloc(1, HTTP_MAX_XFF_FIELDS * sizeof(char *), 
                          PP_HTTPINSPECT, PP_MEM_CATEGORY_CONFIG);
        if (xffFields == NULL)
        {
            FatalError("http_inspect: %s(%d) failed to allocate memory for XFF fields\n", 
                       __FILE__, __LINE__);
        }
    }

    /*
    **  Global Configuration Processing
    **  We only process the global configuration once, but always check for
    **  user mistakes, like configuring more than once.  That's why we
    **  still check for the global token even if it's been checked.
    **  Force the first configuration to be the global one.
    */
    sfPolicyUserPolicySet (hi_swap_config, policy_id);
    pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetCurrent(hi_swap_config);
    if (pPolicyConfig == NULL)
    {
        if (strcasecmp(pcToken, GLOBAL) != 0)
            ParseError("Must configure the http inspect global configuration first.");

        HttpInspectRegisterRuleOptions(sc);

        pPolicyConfig = (HTTPINSPECT_GLOBAL_CONF *) SnortPreprocAlloc(1, 
                                                         sizeof(HTTPINSPECT_GLOBAL_CONF), 
                                                         PP_HTTPINSPECT,
                                                         PP_MEM_CATEGORY_CONFIG);
        if (!pPolicyConfig)
        {
             ParseError("HTTP INSPECT preprocessor: memory allocate failed.\n");
        }
        sfPolicyUserDataSetCurrent(hi_swap_config, pPolicyConfig);
        iRet = HttpInspectInitializeGlobalConfig(pPolicyConfig,
                                                 ErrorString, iErrStrLen);
        if (iRet == 0)
        {
            iRet = ProcessGlobalConf(pPolicyConfig, ErrorString, iErrStrLen, &saveptr);

            if (iRet == 0)
            {
                CheckGzipConfig(pPolicyConfig, hi_swap_config);
                CheckMemcap(pPolicyConfig, hi_swap_config);
                PrintGlobalConf(pPolicyConfig);

                /* Add HttpInspect into the preprocessor list */
                if ( pPolicyConfig->disabled )
                    return;
                AddFuncToPreprocList(sc, HttpInspect, PRIORITY_APPLICATION, PP_HTTPINSPECT, PROTO_BIT__TCP);

            }
        }
    }
    else
    {
        if (strcasecmp(pcToken, SERVER) != 0)
        {
            if (strcasecmp(pcToken, GLOBAL) != 0)
                ParseError("Must configure the http inspect global configuration first.");
            else
                ParseError("Invalid http inspect token: %s.", pcToken);
        }

        iRet = ProcessUniqueServerConf(sc, pPolicyConfig, ErrorString, iErrStrLen, &saveptr);
    }

    if (iRet)
    {
        if(iRet > 0)
        {
            /*
            **  Non-fatal Error
            */
            if(*ErrorString)
            {
                ErrorMessage("%s(%d) => %s\n",
                        file_name, file_line, ErrorString);
            }
        }
        else
        {
            /*
            **  Fatal Error, log error and exit.
            */
            if(*ErrorString)
            {
                FatalError("%s(%d) => %s\n",
                        file_name, file_line, ErrorString);
            }
            else
            {
                /*
                **  Check if ErrorString is undefined.
                */
                if(iRet == -2)
                {
                    FatalError("%s(%d) => ErrorString is undefined.\n",
                            file_name, file_line);
                }
                else
                {
                    FatalError("%s(%d) => Undefined Error.\n",
                            file_name, file_line);
                }
            }
        }
    }
}

static void HttpInspectReloadGlobal(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId hi_swap_config = (tSfPolicyUserContextId)*new_config;
    if (!hi_swap_config)
    {
        hi_swap_config = sfPolicyConfigCreate();
        if (!hi_swap_config)
            FatalError("No memory to allocate http inspect swap_configuration.\n");
        *new_config = hi_swap_config;
    }
    _HttpInspectReload(sc, hi_swap_config, args);
}

static void HttpInspectReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId hi_swap_config;
    hi_swap_config = (tSfPolicyUserContextId)GetRelatedReloadData(sc, GLOBAL_KEYWORD);
    _HttpInspectReload(sc, hi_swap_config, args);
}

static int HttpMempoolFreeUsedBucket(MemPool *memory_pool)
{
    MemBucket *lru_bucket = NULL;

    lru_bucket = mempool_get_lru_bucket(memory_pool);
    if(lru_bucket)
    {
        session_api->set_application_data(lru_bucket->scbPtr, PP_HTTPINSPECT, NULL, NULL);
        return 1;
    }
    return 0;
}

static unsigned HttpMempoolAdjust(MemPool *memory_pool, unsigned httpMaxWork)
{
    int retVal;

    /* deleting MemBucket from free list in HTTP Mempool */
    httpMaxWork = mempool_prune_freelist(memory_pool, memory_pool->max_memory, httpMaxWork);

    for( ; httpMaxWork && ((memory_pool->used_memory + memory_pool->free_memory) > memory_pool->max_memory); httpMaxWork--)
    {
        /* deleting least recently used MemBucket from Used list in HTTP Mempool */
        retVal = HttpMempoolFreeUsedBucket(memory_pool);
        if(!retVal)
           break;
    }

    return httpMaxWork;
}

static bool HttpGzipReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    maxWork = HttpMempoolAdjust(hi_gzip_mempool, initialMaxWork);
    /* This check will be true, when the gzip_mempool is disabled and mempool cleaning is also completed  */
    if( hi_gzip_mempool->used_memory + hi_gzip_mempool->free_memory == 0 )
    {
        SnortPreprocFree(hi_gzip_mempool, sizeof(MemPool), PP_HTTPINSPECT,
             PP_MEM_CATEGORY_MEMPOOL);
        hi_gzip_mempool = NULL;
		return true;
    }

    return (maxWork == initialMaxWork) ? true : false;
}

static bool HttpFdReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    maxWork = HttpMempoolAdjust(hi_fd_conf.fd_MemPool, initialMaxWork);
    /* This check will be true, when the fd_mempool is disabled and mempool cleaning is also completed  */
    if( hi_fd_conf.fd_MemPool->used_memory + hi_fd_conf.fd_MemPool->free_memory == 0 )
    {
        SnortPreprocFree(hi_fd_conf.fd_MemPool, hi_fd_conf.Max_Memory, PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_MEMPOOL);
        hi_fd_conf.fd_MemPool = NULL;
		return true;
    }

    return (maxWork == initialMaxWork) ? true : false;
}

static bool HttpMempoolReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new memcap is less than old configured memcap, need to adjust HTTP Mempool.
     * In order to adjust to new max_memory of http mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = HttpMempoolAdjust(http_mempool, initialMaxWork);

    return (maxWork == initialMaxWork) ? true : false;
}

static bool HttpMimeReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new max_mime_mem is less than old configured max_mime_mem, need to adjust HTTP Mime Mempool.
     * In order to adjust to new max_memory of mime mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = HttpMempoolAdjust(mime_decode_mempool, initialMaxWork);

    return (maxWork == initialMaxWork) ? true : false;
}

static bool HttpLogReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    /* If new memcap is less than old configured memcap, need to adjust HTTP Log Mempool.
     * In order to adjust to new max_memory of log mempool, delete buckets from free list.
     * After deleting buckets from free list, still new max_memory is less than old value , delete buckets
     * (least recently used i.e head node of used list )from used list till total memory reaches to new max_memory.
     */
    maxWork = HttpMempoolAdjust(mime_log_mempool, initialMaxWork);

    return (maxWork == initialMaxWork) ? true : false;
}

static int HttpInspectReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId hi_swap_config = (tSfPolicyUserContextId)swap_config;
    HTTPINSPECT_GLOBAL_CONF *defaultConfig;
    HTTPINSPECT_GLOBAL_CONF *defaultSwapConfig;
    bool swap_gzip, swap_fd, curr_gzip, curr_fd;
    tSfPolicyId policy_id = 0;

    if (hi_swap_config == NULL)
        return 0;

    if (sfPolicyUserDataIterate (sc, hi_swap_config, HttpInspectVerifyPolicy))
        return -1;

    defaultConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(hi_config);
    defaultSwapConfig = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(hi_swap_config);

    if (!defaultConfig)
        return 0;

    curr_gzip = (hi_gzip_mempool != NULL);
    curr_fd = (hi_fd_conf.fd_MemPool != NULL);

    policy_id = getParserPolicy(sc);

    LogMessage("HTTPInspect: gzip old=%s, fd old=%s\n", curr_gzip ? "true": "false", curr_fd ? "true": "false");
    if( curr_gzip || curr_fd )
    {
        /* Look for the case where the current and swap configs have differing gzip & fd options. */
        swap_fd = (sfPolicyUserDataIterate(sc, hi_swap_config, HttpInspectFileDecomp) != 0);
        swap_gzip = (sfPolicyUserDataIterate(sc, hi_swap_config, HttpInspectExtractGzip) != 0);

        LogMessage("HTTPInspect: gzip new=%s, fd new=%s\n", swap_gzip ? "true": "false", swap_fd ? "true": "false");
        if(defaultSwapConfig)
        {
             LogMessage("HTTPInspect: old gzip memcap=%u, new gzip memcap=%u\n", defaultConfig->max_gzip_mem, defaultSwapConfig->max_gzip_mem);
             if(curr_gzip)
                 LogMessage("HTTPInspect: HTTP-GZIP-MEMPOOL used=%zu, free=%zu, max=%zu, obj_size=%zu\n",
                         hi_gzip_mempool->used_memory, hi_gzip_mempool->free_memory, hi_gzip_mempool->max_memory, hi_gzip_mempool->obj_size);
             if(curr_fd)
                 LogMessage("HTTPInspect: HTTP-FD-MEMPOOL used=%zu, free=%zu, max=%zu, obj_size=%zu\n",
                         hi_fd_conf.fd_MemPool->used_memory, hi_fd_conf.fd_MemPool->free_memory, hi_fd_conf.fd_MemPool->max_memory, hi_fd_conf.fd_MemPool->obj_size);
             if(defaultSwapConfig->max_gzip_mem < defaultConfig->max_gzip_mem)
             {
                  /* Change in max_gzip_mem value changes the number of max_sessions of hi_gzip_mempool and max_gzip_sessions.
                     This value also changes  max_memory and max_sessions of hi_fd_conf.fd_Mempool.
                     So registering here to adjust these mempools when max_gzip_mem cahnges.
                   */
                   if( curr_gzip && curr_fd && swap_gzip && swap_fd )
                   {
                        ReloadAdjustRegister(sc, "HTTP-GZIP-MEMPOOL", policy_id, &HttpGzipReloadAdjust, NULL, NULL);
                        ReloadAdjustRegister(sc, "HTTP-FD-MEMPOOL", policy_id, &HttpFdReloadAdjust, NULL, NULL);
                   }
                   if( curr_gzip && !curr_fd && swap_gzip)
                        ReloadAdjustRegister(sc, "HTTP-GZIP-MEMPOOL", policy_id, &HttpGzipReloadAdjust, NULL, NULL);
                   if( !curr_gzip && curr_fd && swap_fd)
                        ReloadAdjustRegister(sc, "HTTP-FD-MEMPOOL", policy_id, &HttpFdReloadAdjust, NULL, NULL);
             }
             if(curr_gzip && !swap_gzip)
                 ReloadAdjustRegister(sc, "HTTP-GZIP-MEMPOOL", policy_id, &HttpGzipReloadAdjust, NULL, NULL);
             if(curr_fd && !swap_fd)
                 ReloadAdjustRegister(sc, "HTTP-FD-MEMPOOL", policy_id, &HttpFdReloadAdjust, NULL, NULL);
        }
    }
    else if (defaultSwapConfig != NULL)
    {
        ProcessGzipAndFDMemPools( sc, hi_swap_config, defaultSwapConfig );
    }

    if (http_mempool != NULL)
    {
        if (defaultSwapConfig != NULL)
        {
            LogMessage("HTTPInspect: HTTP-MEMPOOL old memcap=%d, new_memcap=%d, used=%zu, free=%zu, max=%zu, obj_size=%zu\n",
                    defaultConfig->memcap, defaultSwapConfig->memcap, http_mempool->used_memory, http_mempool->free_memory, http_mempool->max_memory, http_mempool->obj_size);
             if (defaultSwapConfig->memcap < defaultConfig->memcap)
                  ReloadAdjustRegister(sc, "HTTP-MEMPOOL", policy_id, &HttpMempoolReloadAdjust, NULL, NULL);
        }
    }
    else
    {
        if (sfPolicyUserDataIterate(sc, hi_swap_config, HttpInspectExtractUriHost) != 0)
        {
            uint32_t max_sessions_logged;

            if (defaultSwapConfig == NULL)
            {
                ErrorMessage("http_inspect:  Must configure a default global "
                            "configuration if you want to enable logging of uri or hostname in any "
                            "server configuration.\n");
                return -1;
            }

            max_sessions_logged = defaultSwapConfig->memcap / (MAX_URI_EXTRACTED + MAX_HOSTNAME);

            http_mempool = (MemPool *) SnortPreprocAlloc(1, sizeof(MemPool), PP_HTTPINSPECT, 
                                            PP_MEM_CATEGORY_MEMPOOL); 

            if (mempool_init(http_mempool, max_sessions_logged,(MAX_URI_EXTRACTED + MAX_HOSTNAME)) != 0)
            {
                FatalError("http_inspect:  Could not allocate HTTP mempool.\n");
            }
        }

    }
    if (mime_decode_mempool != NULL)
    {
        if (sfPolicyUserDataIterate (sc, hi_swap_config, CheckFilePolicyConfig))
            return -1;

        /* If max_mime_mem changes, mime mempool need to be adjusted bcz mempool max_memory will be changed.
         * Registering here to adjust Mime memory Pool when max_mime_mem changes.
         */
        if(defaultSwapConfig)
        {
            LogMessage("HTTPInspect: HTTP-MIME-MEMPOOL old memcap=%d, new_memcap=%d, used=%zu, free=%zu, max=%zu, obj_size=%zu\n",
                    defaultConfig->decode_conf.max_mime_mem, defaultSwapConfig->decode_conf.max_mime_mem, mime_decode_mempool->used_memory,
                    mime_decode_mempool->free_memory, mime_decode_mempool->max_memory, mime_decode_mempool->obj_size);
             if( defaultSwapConfig->decode_conf.max_mime_mem  < defaultConfig->decode_conf.max_mime_mem )
                  ReloadAdjustRegister(sc, "HTTP-MIME-MEMPOOL", policy_id, &HttpMimeReloadAdjust, NULL, NULL);
        }

    }
    else
    {
        if (sfPolicyUserDataIterate(sc, hi_swap_config, HttpEnableDecoding) != 0)
        {
            if (defaultSwapConfig == NULL)
            {
                ErrorMessage("http_inspect:  Must configure a default global "
                        "configuration if you want to enable decoding in any "
                        "server configuration.\n");
                return -1;
            }
            mime_decode_mempool = (MemPool *)file_api->init_mime_mempool(defaultSwapConfig->decode_conf.max_mime_mem,
                    defaultSwapConfig->decode_conf.max_depth, mime_decode_mempool, PROTOCOL_NAME);
        }
    }
    if (mime_log_mempool != NULL)
    {
       if(defaultSwapConfig)
       {
            /* If memcap of HTTP mIme changes, log mempool need to be adjusted bcz mempool max_mempory will be changed.
              * Registering here to adjust Log memory Pool when memcap changes.
              */
            LogMessage("HTTPInspect: HTTP-LOG-MEMPOOL old memcap=%d, new_memcap=%d, used=%zu, free=%zu, max=%zu, obj_size=%zu\n",
                    defaultConfig->mime_conf.memcap, defaultSwapConfig->mime_conf.memcap, mime_log_mempool->used_memory, mime_log_mempool->free_memory,
                    mime_log_mempool->max_memory, mime_log_mempool->obj_size);
            if (defaultSwapConfig->mime_conf.memcap < defaultConfig->mime_conf.memcap)
                 ReloadAdjustRegister(sc, "HTTP-LOG-MEMPOOL", policy_id, &HttpLogReloadAdjust, NULL, NULL);
       }

    }
    else
    {
        if (sfPolicyUserDataIterate(sc, hi_swap_config, HttpEnableMimeLog) != 0)
        {
            if (defaultSwapConfig == NULL)
            {
                ErrorMessage("http_inspect:  Must configure a default global "
                        "configuration if you want to enable mime log in any "
                        "server configuration.\n");
                return -1;
            }
            mime_log_mempool = (MemPool *)file_api->init_log_mempool(0,
                    defaultSwapConfig->mime_conf.memcap, mime_log_mempool, PROTOCOL_NAME);
        }
    }

    return 0;
}

#ifdef REG_TEST
static void display_http_mempool(MemPool *mempool, const char* old_new, const char *pool_type)
{
    if(mempool)
    {
        printf("\n========== START# %s HTTP %s_mempool VALUES ==============================\n", old_new, pool_type);
        printf("%s_mempool object size: %s VALUE # %zu \n",pool_type, old_new, mempool->obj_size);
        printf("%s_mempool max memory : %s VALUE # %zu \n",pool_type, old_new, mempool->max_memory);
        if(mempool->obj_size)
            printf("%s_mempool total number of buckets: %s VALUE # %u \n",pool_type, old_new,(unsigned)(mempool->max_memory / mempool->obj_size));
        printf("========== END# %s HTTP %s_mempool VALUES ==============================\n", old_new, pool_type);
        fflush(stdout);
    }
}
#endif

static void update_gzip_mempool(bool old_gzip, bool new_gzip, uint32_t max_sessions)
{
     if(old_gzip && !new_gzip)
     {
         if(hi_gzip_mempool)
            hi_gzip_mempool->max_memory = 0;
     }
     else
     {  
         if(hi_gzip_mempool)
         {
              hi_gzip_mempool->max_memory = (max_sessions * sizeof( DECOMPRESS_STATE ) );
              hi_gzip_mempool->obj_size = sizeof(DECOMPRESS_STATE);
         }
     }
}
static void update_fd_mempool(bool old_fd, bool new_fd, uint32_t max_sessions)
{
     if(old_fd && !new_fd)
     {
          if(hi_fd_conf.fd_MemPool)
          {
              hi_fd_conf.Max_Memory = 0;
              hi_fd_conf.fd_MemPool->max_memory = 0;
          }
     }
     else
     {
          if(hi_fd_conf.fd_MemPool)
          {
               hi_fd_conf.Max_Memory = (max_sessions * sizeof( fd_session_t ));
               hi_fd_conf.fd_MemPool->max_memory = (max_sessions * sizeof( fd_session_t ));
               hi_fd_conf.fd_MemPool->obj_size = sizeof( fd_session_t );
          }
     }
}
static void update_gzip_fd_mempools(HTTPINSPECT_GLOBAL_CONF* configNew,
            bool old_gzip, bool new_gzip, bool old_fd, bool new_fd)
{
    uint32_t max_sessions = 0;
    uint32_t block_size = 0;

    if( old_fd || old_gzip )
    {
         if( new_fd )
              block_size += sizeof( fd_session_t );
         if( new_gzip )
              block_size += sizeof( DECOMPRESS_STATE );

         if( block_size > configNew->max_gzip_mem )
               FatalError("http_inspect: Error setting the \"max_gzip_mem\" \n");

         if(block_size)
            max_sessions = configNew->max_gzip_mem / block_size;
         configNew->max_gzip_sessions = max_sessions;

         update_fd_mempool(old_fd, new_fd, max_sessions);
         update_gzip_mempool(old_gzip, new_gzip, max_sessions);
    }

}

static void update_http_mempool(uint32_t new_memcap, uint32_t old_memcap)
{
    uint32_t max_sessions_logged = 0;
    size_t obj_size = 0;

    obj_size = (MAX_URI_EXTRACTED + MAX_HOSTNAME);

    if(obj_size)
        max_sessions_logged = new_memcap / obj_size;

#ifdef REG_TEST
    if(REG_TEST_EMAIL_FLAG_HTTP_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
    {
        printf("\nhttp memcap value is #(OLD VALUE) %u \n", old_memcap);
        display_http_mempool(http_mempool, "OLD", "http");
        printf("\nSetting memcap to new value # (NEW VALUE )%u\n",new_memcap);
    }
#endif

    http_mempool->max_memory = max_sessions_logged * obj_size;
    http_mempool->obj_size = obj_size;

#ifdef REG_TEST
    if(REG_TEST_EMAIL_FLAG_HTTP_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
        display_http_mempool(http_mempool, "NEW", "http");
#endif
}

#ifdef REG_TEST
static int HttpInspectUnlimitedDecompressIterate(void *data)
{
    HTTPINSPECT_CONF *server = (HTTPINSPECT_CONF *)data;

    if (server == NULL)
        return 0;

    if (server->unlimited_decompress)
        return 1;

    return 0;
}

static int HttpInspectUnlimitedDecompress(struct _SnortConfig *sc,
           tSfPolicyUserContextId config,
           tSfPolicyId policyId, void *pData)
{
    HTTPINSPECT_GLOBAL_CONF *context = (HTTPINSPECT_GLOBAL_CONF *)pData;

    if (pData == NULL)
        return 0;

    if(context->disabled)
        return 0;

    if ((context->global_server != NULL) && context->global_server->unlimited_decompress)
        return 1;

    if (context->server_lookup != NULL)
    {
        if (sfrt_iterate2(context->server_lookup, HttpInspectUnlimitedDecompressIterate) != 0)
            return 1;
    }
    return 0;
}
static void display_gzip_fd_config_changes(HTTPINSPECT_GLOBAL_CONF* configOld, HTTPINSPECT_GLOBAL_CONF* configNew,
                                           bool old_gzip, bool new_gzip, bool old_fd, bool new_fd, bool old_ud, bool new_ud)
{
    if(configOld->max_gzip_mem  != configNew->max_gzip_mem )
    {
         printf("\nmax_gzip_mem value is # %u",configOld->max_gzip_mem);
         printf("\nSetting max_gzip_value to new value # ( NEW VALUE ) %u\n", configNew->max_gzip_mem);
    }
    if(configOld->compr_depth != configNew->compr_depth)
    {
         printf("\nHttp Global Compression Depth is # OLD VALUE # %u",configOld->compr_depth);
         printf("\nSetting Http Global Compression depth to # NEW VALUE # %u\n", configNew->compr_depth);
    }
    if(configOld->decompr_depth != configNew->decompr_depth)
    {
         printf("\nHttp Global Decompression Depth is # OLD VALUE # %u",configOld->decompr_depth);
         printf("\nSetting Http Global Decompression depth to # NEW VALUE # %u\n", configNew->decompr_depth);
    }
    if( old_gzip != new_gzip )
    {
         printf("\nExtract GZIP is Enabled # OLD VALUE # %s",configOld->global_server->extract_gzip ? "YES" : "NO");
         printf("\n[Setting] Extract GZIP is Enabled # NEW VALUE # %s\n",configNew->global_server->extract_gzip ? "YES" : "NO");
    }
    if( old_fd != new_fd )
    {
         printf("\nFile Decompression modes # OLD VALUE # %lu",configOld->global_server->file_decomp_modes);
         printf("\nSetting File decompression modes to # NEW VALUE # %lu\n",configNew->global_server->file_decomp_modes);
    }
    if( old_ud != new_ud )
    {
         printf("\nUnlimited Decompression Enabled # OLD VALUE # %s",configOld->global_server->unlimited_decompress ? "YES" : "NO");
         printf("\n[Setting] Unlimited Decompression Enabled# NEW VALUE # %s\n",configNew->global_server->unlimited_decompress ? "YES" : "NO");
    }
}
#endif

static void * HttpInspectReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId hi_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = hi_config;
    HTTPINSPECT_GLOBAL_CONF *configNew = NULL, *configOld = NULL;
    bool old_fd, old_gzip, new_fd, new_gzip;
#ifdef REG_TEST
    bool old_ud, new_ud;
#endif

    if (hi_swap_config == NULL)
        return NULL;

    configNew = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(hi_swap_config);
    configOld = (HTTPINSPECT_GLOBAL_CONF *)sfPolicyUserDataGetDefault(old_config);

    old_fd = (sfPolicyUserDataIterate(sc, old_config, HttpInspectFileDecomp) != 0);
    old_gzip = (sfPolicyUserDataIterate(sc, old_config, HttpInspectExtractGzip) != 0);

    new_fd = (sfPolicyUserDataIterate(sc, hi_swap_config, HttpInspectFileDecomp) != 0);
    new_gzip = (sfPolicyUserDataIterate(sc, hi_swap_config, HttpInspectExtractGzip) != 0);


    if( configNew && configOld)
    {
         if(hi_gzip_mempool || hi_fd_conf.fd_MemPool)
         {
#ifdef REG_TEST
              if( (REG_TEST_EMAIL_FLAG_GZIP_MEMPOOL_ADJUST & getRegTestFlagsForEmail() ) ||
                  (REG_TEST_EMAIL_FLAG_FD_MEMPOOL_ADJUST & getRegTestFlagsForEmail()) )
              {
                    old_ud = (sfPolicyUserDataIterate(sc, old_config, HttpInspectUnlimitedDecompress) != 0);
                    new_ud = (sfPolicyUserDataIterate(sc, hi_swap_config, HttpInspectUnlimitedDecompress) != 0);
                    display_gzip_fd_config_changes(configOld, configNew, old_gzip, new_gzip, old_fd, new_fd, old_ud, new_ud);
              }
#endif
              if((configOld->max_gzip_mem  != configNew->max_gzip_mem) ||
                 (old_gzip != new_gzip) ||
                 (old_fd != new_fd) )
              {
                   update_gzip_fd_mempools(configNew, old_gzip, new_gzip, old_fd, new_fd);
              }

         }
         if(http_mempool)
         {
              if(configOld->memcap != configNew->memcap)
              {
                   update_http_mempool(configNew->memcap, configOld->memcap);
              }
         }
         if(mime_decode_mempool)
         {
              if( (configOld->decode_conf.max_mime_mem != configNew->decode_conf.max_mime_mem) ||
                  (configOld->decode_conf.max_depth != configNew->decode_conf.max_depth) )
              {
#ifdef REG_TEST
                  displayMimeMempool(mime_decode_mempool,&(configOld->decode_conf), &(configNew->decode_conf));
#endif
                  /* Update the mime_decode_mempool with new max_memmory and object size when max_mime_mem changes. */
                  update_mime_mempool(mime_decode_mempool, configNew->decode_conf.max_mime_mem, configNew->decode_conf.max_depth);
             }
         }
         if(mime_log_mempool)
         {
              if(configOld->mime_conf.memcap != configNew->mime_conf.memcap )
              {
#ifdef REG_TEST
                  displayLogMempool(mime_log_mempool, configOld->mime_conf.memcap, configNew->mime_conf.memcap);
#endif
                  /* Update the mime_log_mempool with new max_memory and objest size when memcap changes. */
                  update_log_mempool(mime_log_mempool, configNew->mime_conf.memcap, 0);
              }
          }
#ifdef REG_TEST
          displayDecodeDepth(&(configOld->decode_conf), &(configNew->decode_conf));
#endif

    }

    hi_config = hi_swap_config;

    return (void *)old_config;
}

static void HttpInspectReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    HttpInspectFreeConfigs((tSfPolicyUserContextId)data);
}
#endif

static inline void InitLookupTables(void)
{
    int iNum;
    int iCtr;

    memset(hex_lookup, INVALID_HEX_VAL, sizeof(hex_lookup));
    memset(valid_lookup, INVALID_HEX_VAL, sizeof(valid_lookup));

    iNum = 0;
    for(iCtr = 48; iCtr < 58; iCtr++)
    {
        hex_lookup[iCtr] = iNum;
        valid_lookup[iCtr] = HEX_VAL;
        iNum++;
    }

    /*
    * Set the upper case values.
    */
    iNum = 10;
    for(iCtr = 65; iCtr < 71; iCtr++)
    {
        hex_lookup[iCtr] = iNum;
        valid_lookup[iCtr] = HEX_VAL;
        iNum++;
    }

    /*
     *  Set the lower case values.
     */
    iNum = 10;
    for(iCtr = 97; iCtr < 103; iCtr++)
    {
        hex_lookup[iCtr] = iNum;
        valid_lookup[iCtr] = HEX_VAL;
        iNum++;
   }
}

