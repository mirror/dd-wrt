/* $Id$ */
/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 *
 * Program: Snort
 *
 * Purpose: Check out the README file for info on what you can do
 *          with Snort.
 *
 * Author: Martin Roesch (roesch@clark.net)
 *
 * Comments: Ideas and code stolen liberally from Mike Borella's IP Grab
 *           program. Check out his stuff at http://www.borella.net.  I
 *           also have ripped some util functions from TCPdump, plus Mike's
 *           prog is derived from it as well.  All hail TCPdump....
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <pcap.h>
#ifdef HAVE_MALLOC_TRIM
#include <malloc.h>
#endif

#ifndef WIN32
#include <netdb.h>
#else
#include <Iphlpapi.h>
#endif

#ifdef HAVE_GETOPT_LONG
//#define _GNU_SOURCE
/* A GPL copy of getopt & getopt_long src code is now in sfutil */
# undef HAVE_GETOPT_LONG
#endif
#include <getopt.h>

#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#ifndef WIN32
# include <grp.h>
# include <pwd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif  /* !WIN32 */

#if !defined(CATCH_SEGV) && !defined(WIN32)
# include <sys/resource.h>
#endif

#include "decode.h"
#include "encode.h"
#include "sfdaq.h"
#include "active.h"
#include "snort.h"
#include "rules.h"
#include "treenodes.h"
#include "plugbase.h"
#include "snort_debug.h"
#include "util.h"
#include "parser.h"
#include "tag.h"
#include "log.h"
#include "detect.h"
#include "mstring.h"
#include "fpcreate.h"
#include "fpdetect.h"
#include "sfthreshold.h"
#include "rate_filter.h"
#include "packet_time.h"
#include "detection-plugins/sp_flowbits.h"
#include "preprocessors/spp_perfmonitor.h"
#include "preprocessors/perf-base.h"
#include "preprocessors/perf.h"
#include "mempool.h"
#include "strlcpyu.h"
#include "sflsq.h"
#include "sp_replace.h"
#include "output-plugins/spo_log_tcpdump.h"
#include "event_queue.h"
#include "asn1.h"
#include "mpse.h"
#include "generators.h"
#include "ppm.h"
#include "profiler.h"
#include "dynamic-plugins/sp_dynamic.h"
#include "dynamic-plugins/sf_dynamic_define.h"
#include "dynamic-output/plugins/output.h"
#include "sfutil/strvec.h"
#include "detection_util.h"
#include "sfcontrol_funcs.h"
#include "idle_processing_funcs.h"
#include "file_service.h"
#ifdef SIDE_CHANNEL
# include "sidechannel.h"
#endif

#include "dynamic-plugins/sf_dynamic_engine.h"
#include "dynamic-plugins/sf_dynamic_detection.h"
#define PROFILE_PREPROCS_NOREDEF
#include "dynamic-plugins/sf_dynamic_preprocessor.h"
#include "dynamic-plugins/sp_preprocopt.h"
#ifdef SIDE_CHANNEL
# include "dynamic-plugins/sf_dynamic_side_channel.h"
#endif

#ifdef TARGET_BASED
# include "target-based/sftarget_reader.h"
#endif

#ifdef EXIT_CHECK
# include "cpuclock.h"
#endif
#include "sfActionQueue.h"

#ifdef INTEL_SOFT_CPM
#include "sfutil/intel-soft-cpm.h"
#endif

#include "session_api.h"

#include "stream_common.h"
#include "stream5_ha.h"

#ifdef CONTROL_SOCKET
#include "dump.h"
#endif

/* Macros *********************************************************************/
#ifndef DLT_LANE8023
/*
 * Old OPEN BSD Log format is 17.
 * Define DLT_OLDPFLOG unless DLT_LANE8023 (Suse 6.3) is already
 * defined in bpf.h.
 */
# define DLT_OLDPFLOG 17
#endif

#define ALERT_MODE_OPT__NONE       "none"
#define ALERT_MODE_OPT__PKT_CNT    "packet-count"
#define ALERT_MODE_OPT__FULL       "full"
#define ALERT_MODE_OPT__FAST       "fast"
#define ALERT_MODE_OPT__CONSOLE    "console"
#define ALERT_MODE_OPT__CMG        "cmg"
#define ALERT_MODE_OPT__JH         "jh"
#define ALERT_MODE_OPT__DJR        "djr"
#define ALERT_MODE_OPT__AJK        "ajk"
#define ALERT_MODE_OPT__UNIX_SOCK  "unsock"
#define ALERT_MODE_OPT__TEST       "test"

#define LOG_MODE_OPT__NONE    "none"
#define LOG_MODE_OPT__PCAP    "pcap"
#define LOG_MODE_OPT__ASCII   "ascii"

#ifdef MPLS
# define MPLS_PAYLOAD_OPT__IPV4      "ipv4"
# define MPLS_PAYLOAD_OPT__IPV6      "ipv6"
# define MPLS_PAYLOAD_OPT__ETHERNET  "ethernet"
#endif

#define DEFAULT_PAF_MAX  16384

/* Data types *****************************************************************/

typedef enum _GetOptArgType
{
    LONGOPT_ARG_NONE = 0,
    LONGOPT_ARG_REQUIRED,
    LONGOPT_ARG_OPTIONAL

} GetOptArgType;

/* Externs *******************************************************************/
/* Undefine the one from sf_dynamic_preprocessor.h */
#ifdef PERF_PROFILING
extern PreprocStats detectPerfStats, decodePerfStats, metaPerfStats,
       totalPerfStats, eventqPerfStats, rulePerfStats, mpsePerfStats;
extern PreprocStats ruleCheckBitPerfStats, ruleSetBitPerfStats, ruleFailedFlowbitsPerfStats;
extern PreprocStats ruleRTNEvalPerfStats, ruleOTNEvalPerfStats, ruleHeaderNoMatchPerfStats;
extern PreprocStats ruleAddEventQPerfStats, ruleNQEventQPerfStats;
extern PreprocStats preprocRuleOptionPerfStats;
#endif

/* for getopt */
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

extern ListHead *head_tmp;

/* Globals/Public *************************************************************/
PacketCount pc;  /* packet count information */
uint32_t *netmasks = NULL;   /* precalculated netmask array */
char **protocol_names = NULL;
char *snort_conf_file = NULL;   /* -c */
char *snort_conf_dir = NULL;

SnortConfig *snort_cmd_line_conf = NULL;
SnortConfig *snort_conf = NULL;

#if defined(SNORT_RELOAD) && !defined(WIN32)
SnortConfig *snort_conf_new = NULL;
SnortConfig *snort_conf_old = NULL;
#endif

int internal_log_level = INTERNAL_LOG_LEVEL__MESSAGE;

tSfActionQueueId decoderActionQ = NULL;
MemPool decoderAlertMemPool;

VarNode *cmd_line_var_list = NULL;

#ifdef TARGET_BASED
pthread_t attribute_reload_thread_id;
pid_t attribute_reload_thread_pid;
volatile int attribute_reload_thread_running = 0;
volatile int attribute_reload_thread_stop = 0;
int reload_attribute_table_flags = 0;
#endif

volatile bool snort_initializing = true;
static volatile int snort_exiting = 0;
static pid_t snort_main_thread_pid = 0;
#ifndef WIN32
static pthread_t snort_main_thread_id = 0;
#endif

#if defined(SNORT_RELOAD) && !defined(WIN32)
static volatile int snort_reload = 0;
static volatile int snort_swapped = 0;
static volatile int snort_reload_thread_created = 0;
static pthread_t snort_reload_thread_id;
static pid_t snort_reload_thread_pid;
#ifdef CONTROL_SOCKET
static volatile int reloadInProgress = 0;
#endif
#endif

const struct timespec thread_sleep = { 0, 100 };
#ifdef OPENBSD
const struct timespec packet_sleep = { 0, 1 };
#endif

#ifdef HAVE_PCAP_LEX_DESTROY
extern void pcap_lex_destroy(void);
#endif

PreprocConfigFuncNode *preproc_config_funcs = NULL;
OutputConfigFuncNode *output_config_funcs = NULL;
RuleOptConfigFuncNode *rule_opt_config_funcs = NULL;
RuleOptOverrideInitFuncNode *rule_opt_override_init_funcs = NULL;
RuleOptParseCleanupNode *rule_opt_parse_cleanup_list = NULL;
RuleOptByteOrderFuncNode *rule_opt_byte_order_funcs = NULL;

PreprocSignalFuncNode *preproc_clean_exit_funcs = NULL;
PreprocSignalFuncNode *preproc_shutdown_funcs = NULL;
PreprocSignalFuncNode *preproc_reset_funcs = NULL;
PreprocSignalFuncNode *preproc_reset_stats_funcs = NULL;
PreprocStatsFuncNode *preproc_stats_funcs = NULL;

PluginSignalFuncNode *plugin_shutdown_funcs = NULL;
PluginSignalFuncNode *plugin_clean_exit_funcs = NULL;
#ifdef SNORT_RELOAD
PostConfigFuncNode *plugin_reload_funcs = NULL;
#endif

OutputFuncNode *AlertList = NULL;   /* Alert function list */
OutputFuncNode *LogList = NULL;     /* Log function list */

PeriodicCheckFuncNode *periodic_check_funcs = NULL;
DynamicRuleNode *dynamic_rules = NULL;
grinder_t grinder;

#ifdef SIDE_CHANNEL
pthread_mutex_t snort_process_lock;
static bool snort_process_lock_held = false;
#endif

/* Locals/Private ************************************************************/
static long int pcap_loop_count = 0;
static SF_QUEUE *pcap_save_queue = NULL;

#if defined(INLINE_FAILOPEN) && !defined(WIN32)
static pthread_t inline_failopen_thread_id;
static pid_t inline_failopen_thread_pid;
static volatile int inline_failopen_thread_running = 0;
static volatile int inline_failopen_initialized = 0;
static int inline_failopen_pass_pkt_cnt = 0;
static void * SnortPostInitThread(void *);
static DAQ_Verdict IgnoreCallback (void*, const DAQ_PktHdr_t*, const uint8_t*);
#endif

static char signal_error_msg[STD_BUF];
static int exit_signal = 0;
static bool dump_stats_signal = false;
static bool rotate_stats_signal = false;
#ifdef TARGET_BASED
static bool no_attr_table_signal = false;
#endif

#ifndef SNORT_RELOAD
static volatile bool reload_signal = false;
#else
/* reload_signal is incremented in the signal handler for SIGNAL_SNORT_RELOAD
 * which is handled in the main thread.  The reload thread compares the
 * reload_signal count to reload_total which it increments after an equality
 * test between reload_signal and reload_total fails (which means we got a new
 *  SIGNAL_SNORT_RELOAD).  They need to be the same type and size to do this
 *  comparison.  See ReloadConfigThread() */
typedef uint32_t snort_reload_t;
static volatile snort_reload_t reload_signal = 0;
static snort_reload_t reload_total = 0;
#endif

static int done_processing = 0;
static int exit_logged = 0;

static SF_LIST *pcap_object_list = NULL;
static SF_QUEUE *pcap_queue = NULL;
static char* pcap_filter = NULL;

static int snort_argc = 0;
static char **snort_argv = NULL;

/* command line options for getopt */
static char *valid_options =
    "?A:bB:c:CdDeEfF:"
#ifndef WIN32
    "g:"
#endif
    "G:h:Hi:Ik:K:l:L:"
#ifndef WIN32
    "m:"
#endif
    "Mn:NOpP:q"
#ifndef WIN32
    "Q"
#endif
    "r:R:sS:"
#ifndef WIN32
    "t:"
#endif
    "T"
#ifndef WIN32
    "u:"
#endif
    "UvVw:"
#ifdef WIN32
    "W"
#endif
    "XxyZ:"
;

static struct option long_options[] =
{
   {"logid", LONGOPT_ARG_REQUIRED, NULL, 'G'},
   {"perfmon-file", LONGOPT_ARG_REQUIRED, NULL, 'Z'},
   {"snaplen", LONGOPT_ARG_REQUIRED, NULL, 'P'},
   {"version", LONGOPT_ARG_NONE, NULL, 'V'},
   {"help", LONGOPT_ARG_NONE, NULL, '?'},
   {"conf-error-out", LONGOPT_ARG_NONE, NULL,'x'},
   {"dynamic-engine-lib", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_ENGINE_FILE},
   {"dynamic-engine-lib-dir", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_ENGINE_DIRECTORY},
   {"dynamic-detection-lib", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_LIBRARY_FILE},
   {"dynamic-detection-lib-dir", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_LIBRARY_DIRECTORY},
   {"dump-dynamic-rules", LONGOPT_ARG_REQUIRED, NULL, DUMP_DYNAMIC_RULES},
   {"dynamic-preprocessor-lib", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_PREPROC_FILE},
   {"dynamic-preprocessor-lib-dir", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_PREPROC_DIRECTORY},
   {"dynamic-output-lib", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_OUTPUT_FILE},
   {"dynamic-output-lib-dir", LONGOPT_ARG_REQUIRED, NULL, DYNAMIC_OUTPUT_DIRECTORY},
   {"alert-before-pass", LONGOPT_ARG_NONE, NULL, ALERT_BEFORE_PASS},
   {"treat-drop-as-alert", LONGOPT_ARG_NONE, NULL, TREAT_DROP_AS_ALERT},
   {"treat-drop-as-ignore", LONGOPT_ARG_NONE, NULL, TREAT_DROP_AS_IGNORE},
   {"process-all-events", LONGOPT_ARG_NONE, NULL, PROCESS_ALL_EVENTS},
   {"pid-path", LONGOPT_ARG_REQUIRED, NULL, PID_PATH},
   {"create-pidfile", LONGOPT_ARG_NONE, NULL, CREATE_PID_FILE},
   {"nolock-pidfile", LONGOPT_ARG_NONE, NULL, NOLOCK_PID_FILE},
   {"no-interface-pidfile", LONGOPT_ARG_NONE, NULL, NO_IFACE_PID_FILE},

#ifdef INLINE_FAILOPEN
   {"disable-inline-init-failopen", LONGOPT_ARG_NONE, NULL, DISABLE_INLINE_FAILOPEN},
#endif

   {"nostamps", LONGOPT_ARG_NONE, NULL, NO_LOGGING_TIMESTAMPS},

#ifdef TARGET_BASED
   {"disable-attribute-reload-thread", LONGOPT_ARG_NONE, NULL, DISABLE_ATTRIBUTE_RELOAD},
#endif

   {"pcap-single", LONGOPT_ARG_REQUIRED, NULL, PCAP_SINGLE},
   {"pcap-file", LONGOPT_ARG_REQUIRED, NULL, PCAP_FILE_LIST},
   {"pcap-list", LONGOPT_ARG_REQUIRED, NULL, PCAP_LIST},

#ifndef WIN32
   {"pcap-dir", LONGOPT_ARG_REQUIRED, NULL, PCAP_DIR},
   {"pcap-filter", LONGOPT_ARG_REQUIRED, NULL, PCAP_FILTER},
   {"pcap-no-filter", LONGOPT_ARG_NONE, NULL, PCAP_NO_FILTER},
#endif

   {"pcap-loop", LONGOPT_ARG_REQUIRED, NULL, PCAP_LOOP},
   {"pcap-reload", LONGOPT_ARG_NONE, NULL, PCAP_RELOAD},
   {"pcap-reset", LONGOPT_ARG_NONE, NULL, PCAP_RESET},
   {"pcap-show", LONGOPT_ARG_NONE, NULL, PCAP_SHOW},

#ifdef EXIT_CHECK
   {"exit-check", LONGOPT_ARG_REQUIRED, NULL, ARG_EXIT_CHECK},
#endif

   {"search-method", LONGOPT_ARG_REQUIRED, NULL, DETECTION_SEARCH_METHOD},
   {"man", LONGOPT_ARG_REQUIRED, NULL, DETECTION_SEARCH_METHOD},

#ifdef MPLS
   {"enable-mpls-multicast", LONGOPT_ARG_NONE, NULL, ENABLE_MPLS_MULTICAST},
   {"enable-mpls-overlapping-ip", LONGOPT_ARG_NONE, NULL, ENABLE_OVERLAPPING_IP},
   {"max-mpls-labelchain-len", LONGOPT_ARG_REQUIRED, NULL, MAX_MPLS_LABELCHAIN_LEN},
   {"mpls-payload-type", LONGOPT_ARG_REQUIRED, NULL, MPLS_PAYLOAD_TYPE},
#endif

   {"require-rule-sid", LONGOPT_ARG_NONE, NULL, REQUIRE_RULE_SID},

   {"daq", LONGOPT_ARG_REQUIRED, NULL, ARG_DAQ_TYPE},
   {"daq-mode", LONGOPT_ARG_REQUIRED, NULL, ARG_DAQ_MODE},
   {"daq-var", LONGOPT_ARG_REQUIRED, NULL, ARG_DAQ_VAR},
   {"daq-dir", LONGOPT_ARG_REQUIRED, NULL, ARG_DAQ_DIR},
   {"daq-list", LONGOPT_ARG_OPTIONAL, NULL, ARG_DAQ_LIST},
   {"dirty-pig", LONGOPT_ARG_NONE, NULL, ARG_DIRTY_PIG},

   {"enable-inline-test", LONGOPT_ARG_NONE, NULL, ENABLE_INLINE_TEST},

   {"cs-dir", LONGOPT_ARG_REQUIRED, NULL, ARG_CS_DIR},
   {"ha-peer", LONGOPT_ARG_NONE, NULL, ARG_HA_PEER},
   {"ha-out", LONGOPT_ARG_REQUIRED, NULL, ARG_HA_OUT},
   {"ha-in", LONGOPT_ARG_REQUIRED, NULL, ARG_HA_IN},

   {"suppress-config-log", LONGOPT_ARG_NONE, NULL, SUPPRESS_CONFIG_LOG},

   {0, 0, 0, 0}
};

typedef void (*log_func_t)(Packet*);
static void LogPacket (Packet* p)
{
    pc.log_pkts++;
    CallLogPlugins(p, NULL, NULL);
}
static void IgnorePacket (Packet* p) { }
static log_func_t log_func = IgnorePacket;

/* Private function prototypes ************************************************/
static void InitNetmasks(void);
static void InitProtoNames(void);
static const char* GetPacketSource(char**);

static void CleanExit(int);
static void SnortInit(int, char **);
static void InitPidChrootAndPrivs(pid_t);
static void ParseCmdLine(int, char **);
static int ShowUsage(char *);
static void PrintVersion(void);
static void SetSnortConfDir(void);
static void InitGlobals(void);
static SnortConfig * MergeSnortConfs(SnortConfig *, SnortConfig *);
static void InitSignals(void);
#if defined(NOCOREFILE) && !defined(WIN32)
static void SetNoCores(void);
#endif
static void SnortCleanup(int);

static void ParseCmdLineDynamicLibInfo(SnortConfig *, int, char *);
static DynamicLibInfo * DupDynamicLibInfo(DynamicLibInfo *);
static void FreeDynamicLibInfo(DynamicLibInfo *);
static void FreeDynamicLibInfos(SnortConfig *);

static void FreeOutputConfigs(OutputConfig *);
#ifdef SIDE_CHANNEL
static void FreeSideChannelModuleConfigs(SideChannelModuleConfig *);
#endif
static void FreePreprocConfigs(SnortConfig *);
static void FreeRuleStateList(RuleState *);
static void FreeClassifications(ClassType *);
static void FreeReferences(ReferenceSystemNode *);
static void FreePlugins(SnortConfig *);
static void FreePreprocessors(SnortConfig *);

static void SnortUnprivilegedInit(void);
static int SetPktProcessor(void);
static void PacketLoop(void);
#if 0
static char * ConfigFileSearch(void);
#endif
static void SnortReset(void);

static void LoadDynamicPlugins(SnortConfig *);

static void SnortIdle(void);
#ifndef WIN32
static void SnortStartThreads(void);
#endif

/* Signal handler declarations ************************************************/
static void SigDumpStatsHandler(int);
static void SigExitHandler(int);
static void SigReloadHandler(int);
static void SigRotateStatsHandler(int);
#ifdef CONTROL_SOCKET
static void SigPipeHandler(int);
#endif
static void SigOopsHandler(int);

#ifdef TARGET_BASED
static void SigNoAttributeTableHandler(int);
#endif

#if defined(SNORT_RELOAD) && !defined(WIN32)
static SnortConfig * ReloadConfig(void);
static void * ReloadConfigThread(void *);
static int VerifyReload(SnortConfig *);
static int VerifyOutputs(SnortConfig *, SnortConfig *);
static int VerifyLibInfos(DynamicLibInfo *, DynamicLibInfo *);
#endif  /* SNORT_RELOAD */

int InMainThread ()
{
    return (
#ifndef WIN32
		pthread_equal(snort_main_thread_id, pthread_self())
#else
		1
#endif
		);
}

bool SnortIsInitializing( )
{
    return snort_initializing;
}

static int IsProcessingPackets(uint16_t type, const uint8_t *data, uint32_t length, void **new_config,
                               char *statusBuf, int statusBuf_len)
{
    return (!snort_initializing && !snort_exiting && !exit_signal) ? 0 : -1;
}

#if defined(SNORT_RELOAD) && !defined(WIN32) && defined(CONTROL_SOCKET)

static pthread_mutex_t reload_mutex = PTHREAD_MUTEX_INITIALIZER;

static int ControlSocketReloadConfig(uint16_t type, const uint8_t *data, uint32_t length, void **new_config,
                                     char *statusBuf, int statusBuf_len)
{
    SnortConfig *new_sc;

    pthread_mutex_lock(&reload_mutex);
    while (reloadInProgress)
    {
        sleep(1);
    }
    reloadInProgress = 1;
    pthread_mutex_unlock(&reload_mutex);

    LogMessage("\n");
    LogMessage("        --== Reloading Snort ==--\n");
    LogMessage("\n");

    new_sc = ReloadConfig();
    if (new_sc == NULL)
    {
        reloadInProgress = 0;
        return -1;
    }
    *new_config = (void *)new_sc;
    return 0;
}

static int ControlSocketReloadSwap(uint16_t type, void *new_config, void **old_config)
{
    PostConfigFuncNode *idxPlugin = NULL;

    *old_config = (void *)snort_conf;
    snort_conf = (SnortConfig *)new_config;
    SwapPreprocConfigurations(snort_conf);

#ifdef INTEL_SOFT_CPM
    if (snort_conf->fast_pattern_config->search_method == MPSE_INTEL_CPM)
        IntelPmActivate(snort_conf);
#endif

    /* Do any reload for plugin data */
    idxPlugin = plugin_reload_funcs;
    while(idxPlugin)
    {
        idxPlugin->func(snort_conf, SIGHUP, idxPlugin->arg);
        idxPlugin = idxPlugin->next;
    }
    return 0;
}

static void ControlSocketReloadFree(uint16_t type, void *old_config, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    SnortConfig *old_sc = (SnortConfig *)old_config;

    FreeSwappedPreprocConfigurations(snort_conf);

    SnortConfFree(old_sc);

#ifdef INTEL_SOFT_CPM
    if (snort_conf->fast_pattern_config->search_method != MPSE_INTEL_CPM)
        IntelPmStopInstance();
#endif

    LogMessage("\n");
    LogMessage("        --== Reload Complete ==--\n");
    LogMessage("\n");

    reloadInProgress = 0;
}
#endif

static inline void CheckForReload(void)
{
#if defined(SNORT_RELOAD) && !defined(WIN32)

    /* Check for a new configuration */
    if (snort_reload)
    {
        PostConfigFuncNode *idxPlugin = NULL;
        snort_reload = 0;

        /* There was an error reloading.  A non-reloadable configuration
         * option changed */
        if (snort_conf_new == NULL)
        {
#ifdef RELOAD_ERROR_FATAL
            CleanExit(1);
#else
            Restart();
#endif
        }

        snort_conf_old = snort_conf;
        snort_conf = snort_conf_new;
        snort_conf_new = NULL;

        ScRestoreInternalLogLevel();

        SwapPreprocConfigurations(snort_conf);

        /* Need to do this here because there is potentially outstanding
         * state data pointing to the previous configuration.  A race
         * condition is created if these are free'd in the reload thread
         * where a double free could occur. */
        FreeSwappedPreprocConfigurations(snort_conf);

#ifdef INTEL_SOFT_CPM
        if (snort_conf->fast_pattern_config->search_method == MPSE_INTEL_CPM)
            IntelPmActivate(snort_conf);
#endif

        snort_swapped = 1;

        /* Do any reload for plugin data */
        idxPlugin = plugin_reload_funcs;
        while(idxPlugin)
        {
            idxPlugin->func(snort_conf, SIGHUP, idxPlugin->arg);
            idxPlugin = idxPlugin->next;
        }

    }
#endif
}

/*  F U N C T I O N   D E F I N I T I O N S  **********************************/

static int InlineFailOpen (void)
{
#if defined(INLINE_FAILOPEN) && !defined(WIN32)
    if (ScAdapterInlineMode() &&
        !ScReadMode() && !ScDisableInlineFailopen())
    {
        /* If in inline mode, start a thread to handle the rest of snort
         * initialization, then dispatch packets until that initialization
         * is complete. */
        LogMessage("Fail Open Thread starting..\n");

        if (pthread_create(&inline_failopen_thread_id, NULL, SnortPostInitThread, NULL))
        {
            ErrorMessage("Failed to start Fail Open Thread.  "
                         "Starting normally\n");
        }
        else
        {
            while (!inline_failopen_thread_running)
                nanosleep(&thread_sleep, NULL);

            LogMessage("Fail Open Thread started tid=%p (pid=%u)\n",
                    (void*)inline_failopen_thread_id, inline_failopen_thread_pid);

# ifdef DEBUG
            {
                FILE *tmp = fopen("/var/tmp/fo_threadid", "w");
                if ( tmp )
                {
                    fprintf(tmp, "Fail Open Thread PID: %u\n", inline_failopen_thread_pid);
                    fclose(tmp);
                }
            }
# endif
            DAQ_Start();
            SetPktProcessor();
            inline_failopen_initialized = 1;

            /* Passing packets is in the main thread because some systems
             * may have to refer to packet passing thread via process id
             * (linuxthreads) */
            while (snort_initializing)
            {
                int error = DAQ_Acquire(1, IgnoreCallback, NULL);

                if (error)
                    break;
            }

            pthread_join(inline_failopen_thread_id, NULL);
            inline_failopen_thread_running = 0;

            LogMessage("Fail Open Thread terminated, passed %d packets.\n",
                       inline_failopen_pass_pkt_cnt);

            return 1;
        }
    }
#endif
    return 0;
}

/*
 *
 * Function: main(int, char *)
 *
 * Purpose:  Handle program entry and exit, call main prog sections
 *           This can handle both regular (command-line) style
 *           startup, as well as Win32 Service style startup.
 *
 * Arguments: See command line args in README file
 *
 * Returns: 0 => normal exit, 1 => exit on error
 *
 */
int main(int argc, char *argv[])
{
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
    /* Do some sanity checking, because some people seem to forget to
     * put spaces between their parameters
     */
    if ((argc > 1) &&
        ((_stricmp(argv[1], (SERVICE_CMDLINE_PARAM SERVICE_INSTALL_CMDLINE_PARAM)) == 0) ||
         (_stricmp(argv[1], (SERVICE_CMDLINE_PARAM SERVICE_UNINSTALL_CMDLINE_PARAM)) == 0) ||
         (_stricmp(argv[1], (SERVICE_CMDLINE_PARAM SERVICE_SHOW_CMDLINE_PARAM)) == 0)))
    {
        FatalError("You must have a space after the '%s' command-line parameter\n",
                   SERVICE_CMDLINE_PARAM);
    }

    /* If the first parameter is "/SERVICE", then start Snort as a Win32 service */
    if((argc > 1) && (_stricmp(argv[1],SERVICE_CMDLINE_PARAM) == 0))
    {
        return SnortServiceMain(argc, argv);
    }
#endif /* WIN32 && ENABLE_WIN32_SERVICE */

    snort_argc = argc;
    snort_argv = argv;

    return SnortMain(argc, argv);
}

/*
 *
 * Function: SnortMain(int, char *)
 *
 * Purpose:  The real place that the program handles entry and exit.  Called
 *           called by main(), or by SnortServiceMain().
 *
 * Arguments: See command line args in README file
 *
 * Returns: 0 => normal exit, 1 => exit on error
 *
 */
int SnortMain(int argc, char *argv[])
{
    char* tmp_ptr = NULL;
    const char* intf;
    int daqInit;

#ifndef WIN32
    // must be done now in case of fatal error
    // and again after daemonization
    snort_main_thread_id = pthread_self();
#endif

    SnortInit(argc, argv);

#if 0
    sleep(10);
#endif

    intf = GetPacketSource(&tmp_ptr);
    daqInit = intf || snort_conf->daq_type;

    if ( daqInit )
    {
        DAQ_Init(snort_conf);
        DAQ_New(snort_conf, intf);
    }
    if ( tmp_ptr )
        free(tmp_ptr);

    if ( ScDaemonMode() )
    {
        GoDaemon();
    }

    // this must follow daemonization
    snort_main_thread_pid = gettid();
#ifndef WIN32
    snort_main_thread_id = pthread_self();
#endif

#ifndef WIN32
    /* Change groups */
    InitGroups(ScUid(), ScGid());
#endif

#if !defined(HAVE_LINUXTHREADS) && !defined(WIN32)
    // this could be moved to linux threads location
    // and only done there
    SnortStartThreads();
#endif

#if defined(SNORT_RELOAD) && !defined(WIN32) && defined(CONTROL_SOCKET)
    if (ControlSocketRegisterHandler(CS_TYPE_RELOAD, &ControlSocketReloadConfig, &ControlSocketReloadSwap, &ControlSocketReloadFree))
    {
        LogMessage("Failed to register the reload control handler.\n");
    }
#endif

    if (ControlSocketRegisterHandler(CS_TYPE_IS_PROCESSING, &IsProcessingPackets, NULL, NULL))
    {
        LogMessage("Failed to register the is processing control handler.\n");
    }

#ifdef CONTROL_SOCKET
    if (ControlSocketRegisterHandler(CS_TYPE_DUMP_PACKETS, &PacketDumpCommand, NULL, NULL))
    {
        LogMessage("Failed to register the packet dump control handler.\n");
    }
#endif

    if ( ScTestMode() )
    {
        if ( daqInit && DAQ_UnprivilegedStart() )
            SetPktProcessor();
        SnortUnprivilegedInit();
    }
    else if ( DAQ_UnprivilegedStart() )
    {
        SnortUnprivilegedInit();
        SetPktProcessor();
        DAQ_Start();
    }
    else if ( !InlineFailOpen() )
    {
        DAQ_Start();
        SetPktProcessor();
        SnortUnprivilegedInit();
    }

    PacketLoop();

    // DAQ is shutdown in CleanExit() since we don't always return here
    CleanExit(0);

    return 0;
}

#ifndef WIN32
/* All threads need to be created after daemonizing.  If created in
 * the parent thread, when it goes away, so will all of the threads.
 * The child does not "inherit" threads created in the parent. */
static void SnortStartThreads(void)
{
    
    ControlSocketInit();

#ifdef SIDE_CHANNEL
    SideChannelStartTXThread();
#endif

# ifdef SNORT_RELOAD
    if (ScIdsMode())
    {
        LogMessage("Reload thread starting...\n");
        if (pthread_create(&snort_reload_thread_id, NULL, ReloadConfigThread, NULL) != 0)
        {
            ErrorMessage("Could not create configuration reload thread.\n");
            CleanExit(1);
        }

        while (!snort_reload_thread_created)
            nanosleep(&thread_sleep, NULL);

        LogMessage("Reload thread started, thread %p (%u)\n",
                (void*)snort_reload_thread_id, snort_reload_thread_pid);
    }
# endif

# ifdef TARGET_BASED
    SFAT_StartReloadThread();
# endif
}
#else   /* WIN32 */
//------------------------------------------------------------------------------
// interface stuff
//------------------------------------------------------------------------------

static void PrintAllInterfaces (void)
{
    char errorbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs;
    pcap_if_t *dev;
    int j = 1;
    MIB_IFTABLE *iftable = NULL;
    unsigned int len = 0;
    unsigned int ret, i;

    if (pcap_findalldevs(&alldevs, errorbuf) == -1)
        FatalError("Could not get device list: %s.", errorbuf);

    /* max of two iterations here -- first to get the
     * correct length if not big enough. Second to
     * get the data. */
    for (len = sizeof(iftable[0]); ; ) {
        if (iftable)
            free(iftable);
        iftable = SnortAlloc(len);
        ret = GetIfTable(iftable, &len, TRUE);
        if (ret == NO_ERROR)
            break;
        else if (ret != ERROR_INSUFFICIENT_BUFFER)
            FatalError("Could not get device list: %s.", errorbuf);;
    }

    printf("Index\tPhysical Address\tIP Address\tDevice Name\tDescription\n");
    printf("-----\t----------------\t----------\t-----------\t-----------\n");

    for (dev = alldevs; dev != NULL; dev = dev->next, j++)
    {
        uint8_t *mac_addr = NULL;
        for (i = 0; i<iftable->dwNumEntries; i++)
        {
            if (strncmp(dev->description, iftable->table[i].bDescr, iftable->table[i].dwDescrLen) == 0)
            {
                mac_addr = iftable->table[i].bPhysAddr;
                break;
            }
        }
        printf("%5d\t", j);
        if (mac_addr)
        {
            printf("%02X:%02X:%02X:%02X:%02X:%02X\t",
                mac_addr[0], mac_addr[1], mac_addr[2],
                mac_addr[3], mac_addr[4], mac_addr[5]);
        }
        else
        {
            printf("00:00:00:00:00:00\t");
        }
        if (dev->addresses)
        {
            struct sockaddr_in* saddr = (struct sockaddr_in*)dev->addresses->addr;
            sfip_t dev_ip;
            if ((saddr->sin_family == AF_INET) || (saddr->sin_family == AF_INET6))
            {
                sfip_set_raw(&dev_ip, &saddr->sin_addr, saddr->sin_family);
                printf("%s\t", inet_ntoa(&dev_ip));
            }
            else
            {
                printf("disabled\t");
            }
            printf("%s\t%s\n", dev->name, dev->description);
        }
        else
        {
            printf("disabled\t%s\t%s\n", dev->name, dev->description);
        }

    }
    pcap_freealldevs(alldevs);
    free(iftable);
}
#endif  /* WIN32 */

// pcap list stuff ...

static void PQ_SetFilter (const char* f)
{
    if (pcap_filter != NULL)
        free(pcap_filter);

    pcap_filter = f ? SnortStrdup(f) : NULL;
}

static void PQ_Single (const char* pcap)
{
    PcapReadObject* pro;

    if (pcap_object_list == NULL)
    {
        pcap_object_list = sflist_new();
        if (pcap_object_list == NULL)
            FatalError("Could not allocate list to store pcap\n");
    }

    pro = (PcapReadObject *)SnortAlloc(sizeof(PcapReadObject));
    pro->type = PCAP_SINGLE;
    pro->arg = SnortStrdup(pcap);
    pro->filter = NULL;

    if (sflist_add_tail(pcap_object_list, (NODE_DATA)pro) == -1)
        FatalError("Could not add pcap object to list: %s\n", pcap);
}

static void PQ_Multi (char type, const char* list)
{
    PcapReadObject* pro;

    if (pcap_object_list == NULL)
    {
        pcap_object_list = sflist_new();
        if (pcap_object_list == NULL)
            FatalError("Could not allocate list to store pcaps\n");
    }

    pro = (PcapReadObject *)SnortAlloc(sizeof(PcapReadObject));
    pro->type = type;
    pro->arg = SnortStrdup(list);
    if (pcap_filter != NULL)
        pro->filter = SnortStrdup(pcap_filter);
    else
        pro->filter = NULL;

    if (sflist_add_tail(pcap_object_list, (NODE_DATA)pro) == -1)
        FatalError("Could not add pcap object to list: %s\n", list);
}

static void PQ_SetUp (void)
{
    if (pcap_object_list != NULL)
    {
        if (sflist_count(pcap_object_list) == 0)
        {
            sflist_free_all(pcap_object_list, NULL);
            FatalError("No pcaps specified.\n");
        }

        pcap_queue = sfqueue_new();
        pcap_save_queue = sfqueue_new();
        if ((pcap_queue == NULL) || (pcap_save_queue == NULL))
            FatalError("Could not allocate pcap queues.\n");

        if (GetPcaps(pcap_object_list, pcap_queue) == -1)
            FatalError("Error getting pcaps.\n");

        if (sfqueue_count(pcap_queue) == 0)
            FatalError("No pcaps found.\n");

        /* free pcap list used to get params */
        while (sflist_count(pcap_object_list) > 0)
        {
            PcapReadObject *pro = (PcapReadObject *)sflist_remove_head(pcap_object_list);
            if (pro == NULL)
                FatalError("Failed to remove pcap item from list.\n");

            if (pro->arg != NULL)
                free(pro->arg);

            if (pro->filter != NULL)
                free(pro->filter);

            free(pro);
        }

        sflist_free_all(pcap_object_list, NULL);
        pcap_object_list = NULL;
    }
    if (pcap_filter != NULL)
    {
        free(pcap_filter);
        pcap_filter = NULL;
    }
}

static int PQ_CleanUp (void)
{
    /* clean up pcap queues */
    if (pcap_queue != NULL)
        sfqueue_free_all(pcap_queue, free);

    if (pcap_save_queue != NULL)
        sfqueue_free_all(pcap_save_queue, free);

    return 0;
}

static void PQ_Show (const char* pcap)
{
    if ( !ScPcapShow() )
        return;

    if ( !strcmp(pcap, "-") ) pcap = "stdin";

    fprintf(stdout,
        "Reading network traffic from \"%s\" with snaplen = %d\n",
        pcap, DAQ_GetSnapLen());
}

static const char* PQ_First (void)
{
    const char* pcap = (char*)sfqueue_remove(pcap_queue);

    if ( !pcap )
        return pcap;

    if ( sfqueue_add(pcap_save_queue, (NODE_DATA)pcap) == -1 )
        FatalError("Could not add pcap to saved list\n");

    return pcap;
}

// this must follow 2nd or later start and not stop because we force a
// reset when the dlt changes even if not enabled with --pcap-reset to
// avoid eventually flushing stream packets through a different grinder
// than the one they were queued with.
static void PQ_Reset ()
{
    static int dlt = -1;
    int new_dlt = DAQ_GetBaseProtocol();

    if ( ScPcapReset() || ((dlt != new_dlt) && (dlt != -1)) )
        SnortReset();

    dlt = new_dlt;

    /* open a new tcpdump file - necessary because the snaplen and
     * datalink could be different between pcaps */
    if (snort_conf->log_tcpdump)
    {
        /* this sleep is to ensure we get a new log file since it has a
         * time stamp with resolution to the second */
#ifdef WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        LogTcpdumpReset();
    }
}

static int PQ_Next (void)
{
    char reopen_pcap = 0;

    if (sfqueue_count(pcap_queue) > 0)
    {
        reopen_pcap = 1;
    }
    else if (pcap_loop_count)
    {
        if (pcap_loop_count > 0)
            pcap_loop_count--;

        if (pcap_loop_count != 0)
        {
            SF_QUEUE *tmp;

            /* switch pcap lists */
            tmp = pcap_queue;
            pcap_queue = pcap_save_queue;
            pcap_save_queue = tmp;

            reopen_pcap = 1;
        }
    }

    if (reopen_pcap)
    {
        /* reinitialize pcap */
        const char* pcap = PQ_First();

        if ( !pcap )
            FatalError("Could not get pcap from list\n");

        DAQ_Stop();
        DAQ_Delete();

        DAQ_New(snort_conf, pcap);
        DAQ_Start();

        PQ_Reset();
        PQ_Show(pcap);
        SetPktProcessor();

        SetSampleTime(perfmon_config, NULL);

#if defined(SNORT_RELOAD) && !defined(WIN32)
        if ( snort_conf->run_flags & RUN_FLAG__PCAP_RELOAD && ScIdsMode())
        {
            /* Awaiting user confirmation */
            printf("Hit return to continue.\n");
            fflush(stdout);
            while(getc(stdin) != '\n');

            SigReloadHandler(SIGNAL_SNORT_RELOAD);

            while (!snort_reload)
                sleep(1);
        }
#endif

        return 1;
    }
    return 0;
}

static char* GetFirstInterface (void)
{
    char *iface = NULL;
    char errorbuf[PCAP_ERRBUF_SIZE];
#ifdef WIN32
    pcap_if_t *alldevs;

    if ( (pcap_findalldevs(&alldevs, errorbuf) == -1) || !alldevs )
    {
        FatalError( "Failed to lookup interface: %s. "
            "Please specify one with -i switch\n", errorbuf);
    }

    /* Pick first interface */
    iface = SnortStrdup(alldevs->name);
    pcap_freealldevs(alldevs);
#else
    DEBUG_WRAP(DebugMessage(
        DEBUG_INIT, "interface is NULL, looking up interface...."););

    /* look up the device and get the handle */
    iface = pcap_lookupdev(errorbuf);

    if ( !iface )
    {
        FatalError( "Failed to lookup interface: %s. "
            "Please specify one with -i switch\n", errorbuf);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "found interface %s\n",
                            PRINT_INTERFACE(iface)););

    iface = SnortStrdup(iface);
#endif
    return iface;
}

static const char* GetPacketSource (char** sptr)
{
    const char* intf = "other";

    if ( ScReadMode() )
    {
        intf = PQ_First();
        PQ_Show(intf);
    }
    else if ( !ScVersionMode() && !ScRuleDumpMode() )
    {
        intf = snort_conf->interface;

        // don't get interface if daq is explicitly configured
        // since we can't assume that an interface is compatible
        if ( !intf && !ScTestMode() &&
            (!snort_conf->daq_type ||
            // but we make execptions for these:
            // TBD make selection based on DAQ_TYPE_XXX
            !strcasecmp(snort_conf->daq_type, "afpacket") ||
            !strcasecmp(snort_conf->daq_type, "pcap") ||
            !strcasecmp(snort_conf->daq_type, "dump")) )
        {
            intf = GetFirstInterface();
            *sptr = (char*)intf;
        }
    }
    return intf;
}

static void InitPidChrootAndPrivs(pid_t pid)
{
    /* create the PID file */
    if ( !ScReadMode() &&
        (ScDaemonMode() || *snort_conf->pidfile_suffix || ScCreatePidFile()))
    {
        CreatePidFile(DAQ_GetInterfaceSpec(), pid);
    }

#ifndef WIN32
    /* Drop the Chrooted Settings */
    if (snort_conf->chroot_dir)
        SetChroot(snort_conf->chroot_dir, &snort_conf->log_dir);
#endif

#ifndef WIN32
    /* Drop privileges if requested, when initialization is done */
    SetUidGid(ScUid(), ScGid());
#endif
}

static void LoadDynamicPlugins(SnortConfig *sc)
{
    unsigned i;

    if (sc == NULL)
        return;

    if (sc->dyn_engines != NULL)
    {
        /* Load the dynamic engines */
        for (i = 0; i < sc->dyn_engines->count; i++)
        {
            switch (sc->dyn_engines->lib_paths[i]->ptype)
            {
                case PATH_TYPE__FILE:
                    LoadDynamicEngineLib(sc->dyn_engines->lib_paths[i]->path, 0);
                    break;

                case PATH_TYPE__DIRECTORY:
                    LoadAllDynamicEngineLibs(sc->dyn_engines->lib_paths[i]->path);
                    break;
            }
        }
    }

    if (sc->dyn_rules != NULL)
    {
        /* Load the dynamic detection libs */
        for (i = 0; i < sc->dyn_rules->count; i++)
        {
            switch (sc->dyn_rules->lib_paths[i]->ptype)
            {
                case PATH_TYPE__FILE:
                    LoadDynamicDetectionLib(sc->dyn_rules->lib_paths[i]->path, 0);
                    break;

                case PATH_TYPE__DIRECTORY:
                    LoadAllDynamicDetectionLibs(sc->dyn_rules->lib_paths[i]->path);
                    break;
            }
        }
    }

    if (sc->dyn_preprocs != NULL)
    {
        /* Load the dynamic preprocessors */
        for (i = 0; i < sc->dyn_preprocs->count; i++)
        {
            switch (sc->dyn_preprocs->lib_paths[i]->ptype)
            {
                case PATH_TYPE__FILE:
                    LoadDynamicPreprocessor(sc->dyn_preprocs->lib_paths[i]->path, 0);
                    break;

                case PATH_TYPE__DIRECTORY:
                    LoadAllDynamicPreprocessors(sc->dyn_preprocs->lib_paths[i]->path);
                    break;
            }
        }
    }

# ifdef SIDE_CHANNEL
    if (sc->dyn_side_channels != NULL)
    {
        /* Load the dynamic side channels */
        for (i = 0; i < sc->dyn_side_channels->count; i++)
        {
            switch (sc->dyn_side_channels->lib_paths[i]->ptype)
            {
                case PATH_TYPE__FILE:
                    LoadDynamicSideChannelLib(sc->dyn_side_channels->lib_paths[i]->path, 0);
                    break;

                case PATH_TYPE__DIRECTORY:
                    LoadAllDynamicSideChannelLibs(sc->dyn_side_channels->lib_paths[i]->path);
                    break;
            }
        }
    }
# endif /* SIDE_CHANNEL */

    ValidateDynamicEngines();
}

static void DisplayDynamicPluginVersions(void)
{
    void *lib = NULL;
    DynamicPluginMeta *meta;

    RemoveDuplicateEngines();
    RemoveDuplicateDetectionPlugins();
    RemoveDuplicatePreprocessorPlugins();
#ifdef SIDE_CHANNEL
    RemoveDuplicateSideChannelPlugins();
#endif /* SIDE_CHANNEL */

    lib = GetNextEnginePluginVersion(NULL);
    while ( lib != NULL )
    {
        meta = GetDetectionPluginMetaData(lib);

        LogMessage("           Rules Engine: %s  Version %d.%d  <Build %d>\n",
                   meta->uniqueName, meta->major, meta->minor, meta->build);
        lib = GetNextEnginePluginVersion(lib);
    }

    lib = GetNextDetectionPluginVersion(NULL);
    while ( lib != NULL )
    {
        meta = GetEnginePluginMetaData(lib);

        LogMessage("           Rules Object: %s  Version %d.%d  <Build %d>\n",
                   meta->uniqueName, meta->major, meta->minor, meta->build);
        lib = GetNextDetectionPluginVersion(lib);
    }

    lib = GetNextPreprocessorPluginVersion(NULL);
    while ( lib != NULL )
    {
        meta = GetPreprocessorPluginMetaData(lib);

        LogMessage("           Preprocessor Object: %s  Version %d.%d  <Build %d>\n",
                   meta->uniqueName, meta->major, meta->minor, meta->build);
        lib = GetNextPreprocessorPluginVersion(lib);
    }

#ifdef SIDE_CHANNEL
    lib = GetNextSideChannelPluginVersion(NULL);
    while ( lib != NULL )
    {
        meta = GetSideChannelPluginMetaData(lib);

        LogMessage("           Side Channel Object: %s  Version %d.%d  <Build %d>\n",
                   meta->uniqueName, meta->major, meta->minor, meta->build);
        lib = GetNextSideChannelPluginVersion(lib);
    }
#endif /* SIDE_CHANNEL */

    lib = GetNextOutputModule(NULL);
    while ( lib != NULL )
    {
        LogMessage("           Output Module: %s  Version %u\n",
            GetOutputModuleName(lib), GetOutputModuleVersion(lib));
        lib = GetNextOutputModule(lib);
    }
}

/*
 * This function will print versioning information regardless of whether or
 * not the quiet flag is set.  If the quiet flag has been set and we want
 * to honor it, check it before calling this function.
 */
static void PrintVersion(void)
{
    DisplayBanner();

    //  Get and print out library versions
    DisplayDynamicPluginVersions();
}

static void PrintDaqModules (SnortConfig* sc, char* dir)
{
    if ( dir )
        ConfigDaqDir(sc, dir);

    DAQ_Load(snort_conf);
    DAQ_PrintTypes(stdout);
    DAQ_Unload();
}

#ifdef EXIT_CHECK
static uint64_t exitTime = 0;

static void ExitCheckStart (void)
{
    if ( exitTime )
    {
        return;
    }
    LogMessage("Exit Check: signaling at " STDu64 "callback\n", pc.total_from_daq);
    get_clockticks(exitTime);
#ifndef WIN32
    kill(0, SIGINT);  // send to all processes in my process group
#else
    raise(SIGINT);
#endif
}

static void ExitCheckEnd (void)
{
    uint64_t now = 0;
    double usecs = 0.0;

    if ( !exitTime )
    {
        LogMessage(
            "Exit Check: callbacks = " STDu64 "(limit not reached)\n",
            pc.total_from_daq
        );
        return;
    }
    get_clockticks(now);
    exitTime = now - exitTime;
    usecs = exitTime / get_ticks_per_usec();

    LogMessage("Exit Check: usecs = %f\n", usecs);
}
#endif

#ifdef HAVE_DAQ_ACQUIRE_WITH_META
static int MetaCallback(
    void* user, const DAQ_MetaHdr_t *metahdr, const uint8_t* data)
{
    tSfPolicyId policy_id = getDefaultPolicy();
    SnortPolicy *policy;
    PreprocMetaEvalFuncNode *idx;
    PROFILE_VARS;

#ifdef SIDE_CHANNEL
    if (ScSideChannelEnabled() && !snort_process_lock_held)
    {
        pthread_mutex_lock(&snort_process_lock);
        snort_process_lock_held = true;
    }
#endif

    /* First thing we do is process a Usr signal that we caught */
    if (SignalCheck())
    {
#ifndef SNORT_RELOAD
        /* Got SIGNAL_SNORT_RELOAD */
        Restart();
#endif
    }

    CheckForReload();

    PREPROC_PROFILE_START(metaPerfStats);

    policy = snort_conf->targeted_policies[policy_id];
    idx = policy->preproc_meta_eval_funcs;
    while (idx != NULL)
    {
        idx->func(metahdr->type, data);
        idx = idx->next;
    }

    PREPROC_PROFILE_END(metaPerfStats);

#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
    if (ScTerminateService() || ScPauseService())
    {
        return 0;  // time to go
    }
#endif

    ControlSocketDoWork(0);
#ifdef SIDE_CHANNEL
    SideChannelDrainRX(0);
#endif

    return 0;
}
#endif

void SetupMetadataCallback(void)
{
#ifdef HAVE_DAQ_ACQUIRE_WITH_META
    DAQ_Set_MetaCallback(&MetaCallback);
#endif
}

// non-local for easy access from core
static Packet s_packet;
static DAQ_PktHdr_t s_pkth;
static uint8_t s_data[65536];

static DAQ_Verdict PacketCallback(
    void* user, const DAQ_PktHdr_t* pkthdr, const uint8_t* pkt)
{
    int inject = 0;
    DAQ_Verdict verdict = DAQ_VERDICT_PASS;
    PROFILE_VARS;

    PREPROC_PROFILE_START(totalPerfStats);

#ifdef SIDE_CHANNEL
    if (ScSideChannelEnabled() && !snort_process_lock_held)
    {
        pthread_mutex_lock(&snort_process_lock);
        snort_process_lock_held = true;
    }
#endif

#ifdef EXIT_CHECK
    if (snort_conf->exit_check && (pc.total_from_daq >= snort_conf->exit_check))
        ExitCheckStart();
#endif

    /* First thing we do is process a Usr signal that we caught */
    if (SignalCheck())
    {
#ifndef SNORT_RELOAD
        /* Got SIGNAL_SNORT_RELOAD */
        PREPROC_PROFILE_END(totalPerfStats);
        Restart();
#endif
    }

    pc.total_from_daq++;

    /* Increment counter that we're evaling rules for caching results */
    rule_eval_pkt_count++;

#ifdef TARGET_BASED
    /* Load in a new attribute table if we need to... */
    AttributeTableReloadCheck();
#endif

    CheckForReload();

    /* Save off the time of each and every packet */
    packet_time_update(&pkthdr->ts);

#ifdef REG_TEST
    if ( snort_conf->pkt_skip && pc.total_from_daq <= snort_conf->pkt_skip )
    {
        PREPROC_PROFILE_END(totalPerfStats);
        return verdict;
    }
#endif

#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
    if (ScTerminateService() || ScPauseService())
    {
        PREPROC_PROFILE_END(totalPerfStats);
        return verdict;  // time to go
    }
#endif

    /* reset the thresholding subsystem checks for this packet */
    sfthreshold_reset();

    PREPROC_PROFILE_START(eventqPerfStats);
    SnortEventqReset();
    Replace_ResetQueue();
#ifdef ACTIVE_RESPONSE
    Active_ResetQueue();
#endif
    PREPROC_PROFILE_END(eventqPerfStats);

    verdict = ProcessPacket(&s_packet, pkthdr, pkt, NULL);

#ifdef ACTIVE_RESPONSE
    if ( Active_ResponseQueued() )
    {
        Active_SendResponses(&s_packet);
    }
#endif
    if ( Active_PacketWasDropped() )
    {
        if ( verdict == DAQ_VERDICT_PASS ) 
        {
#ifdef HAVE_DAQ_VERDICT_RETRY
            FileContext *file_context = get_current_file_context(s_packet.ssnptr);
            if(file_context && file_context->verdict == FILE_VERDICT_PENDING) 
            {
                //  File module is waiting for a response from the cloud.
                verdict = DAQ_VERDICT_RETRY;
            }
            else
#endif
            {
                verdict = DAQ_VERDICT_BLOCK;
            }
        }
    }
    else
    {
        Replace_ModifyPacket(&s_packet);

        if ( s_packet.packet_flags & PKT_MODIFIED )
        {
            // this packet was normalized and/or has replacements
            Encode_Update(&s_packet);
            verdict = DAQ_VERDICT_REPLACE;
        }
#ifdef NORMALIZER
        else if ( s_packet.packet_flags & PKT_RESIZED )
        {
            // we never increase, only trim, but
            // daq doesn't support resizing wire packet
            if ( !DAQ_Inject(s_packet.pkth, 0, s_packet.pkt, s_packet.pkth->pktlen) )
            {
                verdict = DAQ_VERDICT_BLOCK;
                inject = 1;
            }
        }
#endif
        else
        {
            if ((s_packet.packet_flags & PKT_IGNORE) ||
                (session_api && (session_api->get_ignore_direction(s_packet.ssnptr) == SSN_DIR_BOTH)))
            {
                if ( !Active_GetTunnelBypass() )
                {
                    verdict = DAQ_VERDICT_WHITELIST;
                }
                else
                {
                    verdict = DAQ_VERDICT_PASS;
                    pc.internal_whitelist++;
                }
            }
            else if ( s_packet.packet_flags & PKT_TRUST )
            {
                if (session_api)
                    session_api->set_ignore_direction(s_packet.ssnptr, SSN_DIR_BOTH);

                verdict = DAQ_VERDICT_WHITELIST;
            }
            else
            {
                verdict = DAQ_VERDICT_PASS;
            }
        }
    }
#ifdef ENABLE_HA
    /* This needs to be called here since the session could have been updated anywhere up to this point. :( */
    if (session_api)
        session_api->process_ha(s_packet.ssnptr);
#endif

    /* Collect some "on the wire" stats about packet size, etc */
    UpdateWireStats(&sfBase, pkthdr->caplen, Active_PacketWasDropped(), inject);
    Active_Reset();
    Encode_Reset();

    if( session_api )
        session_api->check_session_timeout(4, pkthdr->ts.tv_sec);
    ControlSocketDoWork(0);
#ifdef SIDE_CHANNEL
    SideChannelDrainRX(0);
#endif

    s_packet.pkth = NULL;  // no longer avail on segv

    PREPROC_PROFILE_END(totalPerfStats);
    return verdict;
}

static void PrintPacket(Packet *p)
{
    if (p->iph != NULL)
    {
        PrintIPPkt(stdout, GET_IPH_PROTO((p)), p);
    }
#ifndef NO_NON_ETHER_DECODER
    else if (p->ah != NULL)
    {
        PrintArpHeader(stdout, p);
    }
    else if (p->eplh != NULL)
    {
        PrintEapolPkt(stdout, p);
    }
    else if (p->wifih && ScOutputWifiMgmt())
    {
        PrintWifiPkt(stdout, p);
    }
#endif  // NO_NON_ETHER_DECODER
}

DAQ_Verdict ProcessPacket(
    Packet* p, const DAQ_PktHdr_t* pkthdr, const uint8_t* pkt, void* ft)
{
    DAQ_Verdict verdict = DAQ_VERDICT_PASS;

    // set runtime policy to default...idx is same for both nap & ips
    setNapRuntimePolicy(getDefaultPolicy());
    setIpsRuntimePolicy(getDefaultPolicy());

    /* call the packet decoder */
    (*grinder) (p, pkthdr, pkt);
    assert(p->pkth && p->pkt);

    if (ft)
    {
        p->packet_flags |= (PKT_PSEUDO | PKT_REBUILT_FRAG);
        p->pseudo_type = PSEUDO_PKT_IP;
        p->fragtracker = ft;
        Encode_SetPkt(p);
    }
    if ( !p->proto_bits )
        p->proto_bits = PROTO_BIT__OTHER;

    // required until decoders are fixed
    else if ( !p->family && (p->proto_bits & PROTO_BIT__IP) )
        p->proto_bits &= ~PROTO_BIT__IP;

    /***** Policy specific decoding should into this function *****/
    p->configPolicyId = snort_conf->targeted_policies[ getNapRuntimePolicy() ]->configPolicyId;

    /* just throw away the packet if we are configured to ignore this port */
    if ( !(p->packet_flags & PKT_IGNORE) )
    {
        /* start calling the detection processes */
        Preprocess(p);
        log_func(p);
    }

    if ( Active_SessionWasDropped() )
    {
        if ( !Active_PacketForceDropped() )
            Active_DropAction(p);
        else
            Active_ForceDropAction(p);

        if ( Active_GetTunnelBypass() )
        {
            pc.internal_blacklist++;
            return verdict;
        }

        if ( ScIpsInlineMode() || Active_PacketForceDropped() )
            verdict = DAQ_VERDICT_BLACKLIST;
        else
            verdict = DAQ_VERDICT_IGNORE;
    }

#ifdef CONTROL_SOCKET
    if (packet_dump_stop)
        PacketDumpClose();
    else if (packet_dump_file &&
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
             pkthdr->address_space_id == packet_dump_address_space_id &&
#endif
             (!packet_dump_fcode.bf_insns || sfbpf_filter(packet_dump_fcode.bf_insns, (uint8_t *)pkt,
                                                          pkthdr->caplen, pkthdr->pktlen)))
    {
        pcap_dump((uint8_t*)packet_dump_file, (const struct pcap_pkthdr*)pkthdr, pkt);
        pcap_dump_flush((pcap_dumper_t*)packet_dump_file);
    }
#endif

    return verdict;
}

Packet *NewGrinderPkt(Packet *p, DAQ_PktHdr_t* phdr, uint8_t *pkt)
{
    if( !p )
    {
        IP6Option* ip6_extensions;
        p = SnortAlloc(sizeof(*p));
        ip6_extensions = SnortAlloc(sizeof(IP6Option) * ScMaxIP6Extensions());

        if ( !p || !ip6_extensions )
            FatalError("Encode_New() => Failed to allocate packet\n");

        p->ip6_extensions = ip6_extensions;
    }

    if ( phdr && pkt )
    {
        (*grinder)(p, phdr, pkt);
    }


    return p;
}

void DeleteGrinderPkt( Packet *p)
{
    if(!p)
        return;

    if(p->ip6_extensions)
        free(p->ip6_extensions);

    free(p);
}


/*
 * Function: ShowUsage(char *)
 *
 * Purpose:  Display the program options and exit
 *
 * Arguments: argv[0] => name of the program (argv[0])
 *
 * Returns: 0 => success
 */
static int ShowUsage(char *program_name)
{
    fprintf(stdout, "USAGE: %s [-options] <filter options>\n", program_name);
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
    fprintf(stdout, "       %s %s %s [-options] <filter options>\n", program_name
                                                                   , SERVICE_CMDLINE_PARAM
                                                                   , SERVICE_INSTALL_CMDLINE_PARAM);
    fprintf(stdout, "       %s %s %s\n", program_name
                                       , SERVICE_CMDLINE_PARAM
                                       , SERVICE_UNINSTALL_CMDLINE_PARAM);
    fprintf(stdout, "       %s %s %s\n", program_name
                                       , SERVICE_CMDLINE_PARAM
                                       , SERVICE_SHOW_CMDLINE_PARAM);
#endif

#ifdef WIN32
# define FPUTS_WIN32(msg) fputs(msg,stdout)
# define FPUTS_UNIX(msg)  NULL
# define FPUTS_BOTH(msg)  fputs(msg,stdout)
#else
# define FPUTS_WIN32(msg)
# define FPUTS_UNIX(msg)  fputs(msg,stdout)
# define FPUTS_BOTH(msg)  fputs(msg,stdout)
#endif

    FPUTS_BOTH ("Options:\n");
    FPUTS_BOTH ("        -A         Set alert mode: fast, full, console, test or none "
                                  " (alert file alerts only)\n");
    FPUTS_UNIX ("                   \"unsock\" enables UNIX socket logging (experimental).\n");
    FPUTS_BOTH ("        -b         Log packets in tcpdump format (much faster!)\n");
    FPUTS_BOTH ("        -B <mask>  Obfuscated IP addresses in alerts and packet dumps using CIDR mask\n");
    FPUTS_BOTH ("        -c <rules> Use Rules File <rules>\n");
    FPUTS_BOTH ("        -C         Print out payloads with character data only (no hex)\n");
    FPUTS_BOTH ("        -d         Dump the Application Layer\n");
    FPUTS_UNIX ("        -D         Run Snort in background (daemon) mode\n");
    FPUTS_BOTH ("        -e         Display the second layer header info\n");
    FPUTS_WIN32("        -E         Log alert messages to NT Eventlog. (Win32 only)\n");
    FPUTS_BOTH ("        -f         Turn off fflush() calls after binary log writes\n");
    FPUTS_BOTH ("        -F <bpf>   Read BPF filters from file <bpf>\n");
    FPUTS_UNIX ("        -g <gname> Run snort gid as <gname> group (or gid) after initialization\n");
    FPUTS_BOTH ("        -G <0xid>  Log Identifier (to uniquely id events for multiple snorts)\n");
    FPUTS_BOTH ("        -h <hn>    Set home network = <hn>\n"
                "                   (for use with -l or -B, does NOT change $HOME_NET in IDS mode)\n");
    FPUTS_BOTH ("        -H         Make hash tables deterministic.\n");
    FPUTS_BOTH ("        -i <if>    Listen on interface <if>\n");
    FPUTS_BOTH ("        -I         Add Interface name to alert output\n");
    FPUTS_BOTH ("        -k <mode>  Checksum mode (all,noip,notcp,noudp,noicmp,none)\n");
    FPUTS_BOTH ("        -K <mode>  Logging mode (pcap[default],ascii,none)\n");
    FPUTS_BOTH ("        -l <ld>    Log to directory <ld>\n");
    FPUTS_BOTH ("        -L <file>  Log to this tcpdump file\n");
    FPUTS_UNIX ("        -M         Log messages to syslog (not alerts)\n");
    FPUTS_UNIX ("        -m <umask> Set umask = <umask>\n");
    FPUTS_BOTH ("        -n <cnt>   Exit after receiving <cnt> packets\n");
    FPUTS_BOTH ("        -N         Turn off logging (alerts still work)\n");
    FPUTS_BOTH ("        -O         Obfuscate the logged IP addresses\n");
    FPUTS_BOTH ("        -p         Disable promiscuous mode sniffing\n");
    printf     ("        -P <snap>  Set explicit snaplen of packet (default: %d)\n",
                                    DAQ_GetSnapLen());
    FPUTS_BOTH ("        -q         Quiet. Don't show banner and status report\n");
#ifndef WIN32
    FPUTS_BOTH ("        -Q         Enable inline mode operation.\n");
#endif
    FPUTS_BOTH ("        -r <tf>    Read and process tcpdump file <tf>\n");
    FPUTS_BOTH ("        -R <id>    Include 'id' in snort_intf<id>.pid file name\n");
    FPUTS_BOTH ("        -s         Log alert messages to syslog\n");
    FPUTS_BOTH ("        -S <n=v>   Set rules file variable n equal to value v\n");
    FPUTS_UNIX ("        -t <dir>   Chroots process to <dir> after initialization\n");
    FPUTS_BOTH ("        -T         Test and report on the current Snort configuration\n");
    FPUTS_UNIX ("        -u <uname> Run snort uid as <uname> user (or uid) after initialization\n");
    FPUTS_BOTH ("        -U         Use UTC for timestamps\n");
    FPUTS_BOTH ("        -v         Be verbose\n");
    FPUTS_BOTH ("        -V         Show version number\n");
    FPUTS_WIN32("        -W         Lists available interfaces. (Win32 only)\n");
#if defined(NON_ETHER_DECODER) && defined(DLT_IEEE802_11)
    FPUTS_BOTH ("        -w         Dump 802.11 management and control frames\n");
#endif
    FPUTS_BOTH ("        -X         Dump the raw packet data starting at the link layer\n");
    FPUTS_BOTH ("        -x         Exit if Snort configuration problems occur\n");
    FPUTS_BOTH ("        -y         Include year in timestamp in the alert and log files\n");
    FPUTS_BOTH ("        -Z <file>  Set the performonitor preprocessor file path and name\n");
    FPUTS_BOTH ("        -?         Show this information\n");
    FPUTS_BOTH ("<Filter Options> are standard BPF options, as seen in TCPDump\n");

    FPUTS_BOTH ("Longname options and their corresponding single char version\n");
    FPUTS_BOTH ("   --logid <0xid>                  Same as -G\n");
    FPUTS_BOTH ("   --perfmon-file <file>           Same as -Z\n");
    FPUTS_BOTH ("   --pid-path <dir>                Specify the directory for the Snort PID file\n");
    FPUTS_BOTH ("   --snaplen <snap>                Same as -P\n");
    FPUTS_BOTH ("   --help                          Same as -?\n");
    FPUTS_BOTH ("   --version                       Same as -V\n");
    FPUTS_BOTH ("   --alert-before-pass             Process alert, drop, sdrop, or reject before pass, default is pass before alert, drop,...\n");
    FPUTS_BOTH ("   --treat-drop-as-alert           Converts drop, sdrop, and reject rules into alert rules during startup\n");
    FPUTS_BOTH ("   --treat-drop-as-ignore          Use drop, sdrop, and reject rules to ignore session traffic when not inline.\n");
    FPUTS_BOTH ("   --process-all-events            Process all queued events (drop, alert,...), default stops after 1st action group\n");
    FPUTS_BOTH ("   --enable-inline-test            Enable Inline-Test Mode Operation\n");
    FPUTS_BOTH ("   --dynamic-engine-lib <file>     Load a dynamic detection engine\n");
    FPUTS_BOTH ("   --dynamic-engine-lib-dir <path> Load all dynamic engines from directory\n");
    FPUTS_BOTH ("   --dynamic-detection-lib <file>  Load a dynamic rules library\n");
    FPUTS_BOTH ("   --dynamic-detection-lib-dir <path> Load all dynamic rules libraries from directory\n");
    FPUTS_BOTH ("   --dump-dynamic-rules <path>     Creates stub rule files of all loaded rules libraries\n");
    FPUTS_BOTH ("   --dynamic-preprocessor-lib <file>  Load a dynamic preprocessor library\n");
    FPUTS_BOTH ("   --dynamic-preprocessor-lib-dir <path> Load all dynamic preprocessor libraries from directory\n");
    FPUTS_BOTH ("   --dynamic-output-lib <file>  Load a dynamic output library\n");
    FPUTS_BOTH ("   --dynamic-output-lib-dir <path> Load all dynamic output libraries from directory\n");
    FPUTS_UNIX ("   --create-pidfile                Create PID file, even when not in Daemon mode\n");
    FPUTS_UNIX ("   --nolock-pidfile                Do not try to lock Snort PID file\n");
    FPUTS_UNIX ("   --no-interface-pidfile          Do not include the interface name in Snort PID file\n");
#ifdef INLINE_FAILOPEN
    FPUTS_UNIX ("   --disable-inline-init-failopen  Do not fail open and pass packets while initializing with inline mode.\n");
#endif
#ifdef TARGET_BASED
    FPUTS_UNIX ("   --disable-attribute-reload-thread Do not create a thread to reload the attribute table\n");
#endif
    FPUTS_BOTH ("   --pcap-single <tf>              Same as -r.\n");
    FPUTS_BOTH ("   --pcap-file <file>              file that contains a list of pcaps to read - read mode is implied.\n");
    FPUTS_BOTH ("   --pcap-list \"<list>\"            a space separated list of pcaps to read - read mode is implied.\n");
    FPUTS_UNIX ("   --pcap-dir <dir>                a directory to recurse to look for pcaps - read mode is implied.\n");
    FPUTS_UNIX ("   --pcap-filter <filter>          filter to apply when getting pcaps from file or directory.\n");
    FPUTS_UNIX ("   --pcap-no-filter                reset to use no filter when getting pcaps from file or directory.\n");
    FPUTS_BOTH ("   --pcap-loop <count>             this option will read the pcaps specified on command line continuously.\n"
                "                                   for <count> times.  A value of 0 will read until Snort is terminated.\n");
    FPUTS_BOTH ("   --pcap-reset                    if reading multiple pcaps, reset snort to post-configuration state before reading next pcap.\n");
#if defined(SNORT_RELOAD) && !defined(WIN32)
    FPUTS_BOTH ("   --pcap-reload                   if reading multiple pcaps, reload snort config between pcaps.\n");
#endif
    FPUTS_BOTH ("   --pcap-show                     print a line saying what pcap is currently being read.\n");
    FPUTS_BOTH ("   --exit-check <count>            Signal termination after <count> callbacks from DAQ_Acquire(), showing the time it\n"
                "                                   takes from signaling until DAQ_Stop() is called.\n");
    FPUTS_BOTH ("   --conf-error-out                Same as -x\n");
#ifdef MPLS
    FPUTS_BOTH ("   --enable-mpls-multicast         Allow multicast MPLS\n");
    FPUTS_BOTH ("   --enable-mpls-overlapping-ip    Handle overlapping IPs within MPLS clouds\n");
    FPUTS_BOTH ("   --max-mpls-labelchain-len       Specify the max MPLS label chain\n");
    FPUTS_BOTH ("   --mpls-payload-type             Specify the protocol (ipv4, ipv6, ethernet) that is encapsulated by MPLS\n");
#endif
    FPUTS_BOTH ("   --require-rule-sid              Require that all snort rules have SID specified.\n");
    FPUTS_BOTH ("   --daq <type>                    Select packet acquisition module (default is pcap).\n");
    FPUTS_BOTH ("   --daq-mode <mode>               Select the DAQ operating mode.\n");
    FPUTS_BOTH ("   --daq-var <name=value>          Specify extra DAQ configuration variable.\n");
    FPUTS_BOTH ("   --daq-dir <dir>                 Tell snort where to find desired DAQ.\n");
    FPUTS_BOTH ("   --daq-list[=<dir>]              List packet acquisition modules available in dir.  Default is static modules only.\n");
    FPUTS_BOTH ("   --dirty-pig                     Don't flush packets and release memory on shutdown.\n");
    FPUTS_BOTH ("   --cs-dir <dir>                  Directory to use for control socket.\n");
    FPUTS_BOTH ("   --ha-peer                       Activate live high-availability state sharing with peer.\n");
    FPUTS_BOTH ("   --ha-out <file>                 Write high-availability events to this file.\n");
    FPUTS_BOTH ("   --ha-in <file>                  Read high-availability events from this file on startup (warm-start).\n");
    FPUTS_BOTH ("   --suppress-config-log           Suppress configuration information output.\n");
#undef FPUTS_WIN32
#undef FPUTS_UNIX
#undef FPUTS_BOTH
    return 0;
}

static void ParseCmdLineDynamicLibInfo(SnortConfig *sc, int type, char *path)
{
    DynamicLibInfo *dli = NULL;
    DynamicLibPath *dlp = NULL;

    if ((sc == NULL) || (path == NULL))
        FatalError("%s(%d) NULL arguments.\n", __FILE__, __LINE__);

    switch (type)
    {
        case DYNAMIC_PREPROC_FILE:
        case DYNAMIC_PREPROC_DIRECTORY:
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Dynamic preprocessor specifier\n"););
            if (sc->dyn_preprocs == NULL)
            {
                sc->dyn_preprocs = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
                sc->dyn_preprocs->type = DYNAMIC_TYPE__PREPROCESSOR;
            }
            else if (sc->dyn_preprocs->count >= MAX_DYNAMIC_LIBS)
            {
                FatalError("Maximum number of loaded Dynamic Preprocessor Libs "
                           "(%d) exceeded.\n", MAX_DYNAMIC_LIBS);
            }

            dli = sc->dyn_preprocs;
            break;

        case DYNAMIC_LIBRARY_FILE:
        case DYNAMIC_LIBRARY_DIRECTORY:
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Dynamic detection specifier\n"););
            if (sc->dyn_rules == NULL)
            {
                sc->dyn_rules = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
                sc->dyn_rules->type = DYNAMIC_TYPE__DETECTION;
            }
            else if (sc->dyn_rules->count >= MAX_DYNAMIC_LIBS)
            {
                FatalError("Maximum number of loaded Dynamic Detection Libs "
                           "(%d) exceeded.\n", MAX_DYNAMIC_LIBS);
            }

            dli = sc->dyn_rules;
            break;

        case DYNAMIC_ENGINE_FILE:
        case DYNAMIC_ENGINE_DIRECTORY:
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Dynamic engine specifier\n"););
            if (sc->dyn_engines == NULL)
            {
                sc->dyn_engines = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
                sc->dyn_engines->type = DYNAMIC_TYPE__ENGINE;
            }
            else if (sc->dyn_engines->count >= MAX_DYNAMIC_LIBS)
            {
                FatalError("Maximum number of loaded Dynamic Engine Libs "
                           "(%d) exceeded.\n", MAX_DYNAMIC_LIBS);
            }

            dli = sc->dyn_engines;
            break;
        case DYNAMIC_OUTPUT_FILE:
            output_load_module(path);
            return;
            break;
        case DYNAMIC_OUTPUT_DIRECTORY:
            output_load(path);
            return;
            break;


        default:
            FatalError("%s(%d) Invalid dynamic type: %d\n", __FILE__, __LINE__, type);
            break;
    }

    dlp = (DynamicLibPath *)SnortAlloc(sizeof(DynamicLibPath));
    switch (type)
    {
        case DYNAMIC_PREPROC_FILE:
        case DYNAMIC_LIBRARY_FILE:
        case DYNAMIC_ENGINE_FILE:
        case DYNAMIC_OUTPUT_FILE:
            dlp->ptype = PATH_TYPE__FILE;
            break;

        case DYNAMIC_PREPROC_DIRECTORY:
        case DYNAMIC_LIBRARY_DIRECTORY:
        case DYNAMIC_ENGINE_DIRECTORY:
        case DYNAMIC_OUTPUT_DIRECTORY:
            dlp->ptype = PATH_TYPE__DIRECTORY;
            break;

        default:
            FatalError("%s(%d) Invalid dynamic type: %d\n", __FILE__, __LINE__, type);
            break;
    }

    dlp->path = SnortStrdup(path);
    dli->lib_paths[dli->count] = dlp;
    dli->count++;
}

/*
 * Function: ParseCmdLine(int, char **)
 *
 * Parses command line arguments
 *
 * Arguments:
 *  int
 *      count of arguments passed to the routine
 *  char **
 *      2-D character array, contains list of command line args
 *
 * Returns: None
 *
 */

static void ParseCmdLine(int argc, char **argv)
{
    int ch;
    int option_index = -1;
    char *endptr;   /* for SnortStrtol calls */
    SnortConfig *sc;
    int output_logging = 0;
    int output_alerting = 0;
    int syslog_configured = 0;
#ifndef WIN32
    int daemon_configured = 0;
#endif

    int version_flag_parsed = 0;
    int quiet_flag_parsed = 0;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Parsing command line...\n"););

    if (snort_cmd_line_conf != NULL)
    {
        FatalError("%s(%d) Trying to parse the command line again.\n",
                   __FILE__, __LINE__);
    }

    snort_cmd_line_conf = SnortConfNew();
    snort_conf = snort_cmd_line_conf;     /* Set the global for log messages */
    sc = snort_cmd_line_conf;

    optind = 1;

    /* Look for a -D and/or -M switch so we can start logging to syslog
     * with "snort" tag right away */
    while ((ch = getopt_long(argc, argv, valid_options, long_options, &option_index)) != -1)
    {
        switch (ch)
        {
            case 'M':
                if (syslog_configured)
                    break;

                /* If daemon or logging to syslog use "snort" as identifier and
                 * start logging there now */
                openlog("snort", LOG_PID | LOG_CONS, LOG_DAEMON);

                sc->logging_flags |= LOGGING_FLAG__SYSLOG;
                syslog_configured = 1;
                break;

#ifndef WIN32
            case 'E':
                sc->run_flags |= RUN_FLAG__DAEMON_RESTART;
                /* Fall through */
            case 'D':
                if (daemon_configured)
                    break;

                /* If daemon or logging to syslog use "snort" as identifier and
                 * start logging there now */
                openlog("snort", LOG_PID | LOG_CONS, LOG_DAEMON);

                ConfigDaemon(sc, optarg);
                daemon_configured = 1;
                break;
#endif

            case 'V':
                version_flag_parsed = 1;
                break;

            case 'q':
                quiet_flag_parsed = 1;
                break;

            case '?':  /* show help and exit with 1 */
                PrintVersion();
                ShowUsage(argv[0]);
                exit(1);
                break;

            default:
                break;
        }
    }

    if (version_flag_parsed)
    {
        sc->run_mode_flags |= RUN_MODE_FLAG__VERSION;
    }
    else if (quiet_flag_parsed)
    {
        ConfigQuiet(sc, NULL);
        internal_log_level = INTERNAL_LOG_LEVEL__ERROR;
    }

    /*
    **  Set this so we know whether to return 1 on invalid input.
    **  Snort uses '?' for help and getopt uses '?' for telling us there
    **  was an invalid option, so we can't use that to tell invalid input.
    **  Instead, we check optopt and it will tell us.
    */
    optopt = 0;
    optind = 1;

    /* loop through each command line var and process it */
    while ((ch = getopt_long(argc, argv, valid_options, long_options, &option_index)) != -1)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Processing cmd line switch: %c\n", ch););

        switch (ch)
        {
            case DYNAMIC_ENGINE_FILE:       /* Load dynamic engine specified */
            case DYNAMIC_ENGINE_DIRECTORY:  /* Load dynamic engine specified */
            case DYNAMIC_PREPROC_FILE:      /* Load dynamic preprocessor lib specified */
            case DYNAMIC_PREPROC_DIRECTORY:
            case DYNAMIC_LIBRARY_FILE:      /* Load dynamic detection lib specified */
            case DYNAMIC_LIBRARY_DIRECTORY:
            case DYNAMIC_OUTPUT_FILE:      /* Load dynamic output lib specified */
            case DYNAMIC_OUTPUT_DIRECTORY:
                ParseCmdLineDynamicLibInfo(sc, ch, optarg);
                break;

            case DUMP_DYNAMIC_RULES:
                ConfigDumpDynamicRulesPath(sc, optarg);
                break;

            case ALERT_BEFORE_PASS:
                ConfigAlertBeforePass(sc, NULL);
                break;

            case PROCESS_ALL_EVENTS:
                ConfigProcessAllEvents(sc, NULL);
                break;

            case TREAT_DROP_AS_ALERT:
                ConfigTreatDropAsAlert(sc, NULL);
                break;

            case TREAT_DROP_AS_IGNORE:
                ConfigTreatDropAsIgnore(sc, NULL);
                break;

            case PID_PATH:
                ConfigPidPath(sc, optarg);
                break;

            case CREATE_PID_FILE:
                ConfigCreatePidFile(sc, NULL);
                break;

            case NOLOCK_PID_FILE:
                sc->run_flags |= RUN_FLAG__NO_LOCK_PID_FILE;
                break;

            case NO_IFACE_PID_FILE:
                sc->run_flags |= RUN_FLAG__NO_IFACE_PID_FILE;
                break;

#ifdef INLINE_FAILOPEN
            case DISABLE_INLINE_FAILOPEN:
                ConfigDisableInlineFailopen(sc, NULL);
                break;
#endif
            case NO_LOGGING_TIMESTAMPS:
                ConfigNoLoggingTimestamps(sc, NULL);
                break;

#ifdef EXIT_CHECK
            case ARG_EXIT_CHECK:
                {
                    char* endPtr;

                    sc->exit_check = SnortStrtoul(optarg, &endPtr, 0);
                    if ((errno == ERANGE) || (*endPtr != '\0'))
                        FatalError("--exit-check value must be non-negative integer\n");

                    LogMessage("Exit Check: limit = "STDu64" callbacks\n", sc->exit_check);
                }

                break;
#endif

#ifdef TARGET_BASED
            case DISABLE_ATTRIBUTE_RELOAD:
                ConfigDisableAttributeReload(sc, NULL);
                break;
#endif
            case DETECTION_SEARCH_METHOD:
                if (sc->fast_pattern_config != NULL)
                    FatalError("Can only configure search method once.\n");

                sc->fast_pattern_config = FastPatternConfigNew();

                if (fpSetDetectSearchMethod(sc->fast_pattern_config, optarg) == -1)
                    FatalError("Invalid search method: %s.\n", optarg);

                break;

            case ARG_DAQ_TYPE:
                ConfigDaqType(sc, optarg);
                break;

            case ARG_DAQ_MODE:
                ConfigDaqMode(sc, optarg);
                break;

            case ARG_DAQ_VAR:
                ConfigDaqVar(sc, optarg);
                break;

            case ARG_DAQ_DIR:
                ConfigDaqDir(sc, optarg);
                break;

            case ARG_DAQ_LIST:
                PrintDaqModules(sc, optarg);
                exit(0);
                break;

            case ARG_DIRTY_PIG:
                ConfigDirtyPig(sc, optarg);
                break;

            case 'A':  /* alert mode */
                output_alerting = 1;

                if (strcasecmp(optarg, ALERT_MODE_OPT__NONE) == 0)
                {
                    sc->no_alert = 1;
                }
                else if (strcasecmp(optarg, ALERT_MODE_OPT__PKT_CNT) == 0)
                {
                    /* print packet count at start of alert */
                    sc->output_flags |= OUTPUT_FLAG__ALERT_PKT_CNT;
                }
                else if (strcasecmp(optarg, ALERT_MODE_OPT__FULL) == 0)
                {
                    ParseOutput(sc, NULL, "alert_full");
                }
                else if (strcasecmp(optarg, "console:" ALERT_MODE_OPT__FULL) == 0)
                {
                    ParseOutput(sc, NULL, "alert_full: stdout");
                }
                else if (strcasecmp(optarg, ALERT_MODE_OPT__FAST) == 0)
                {
                    ParseOutput(sc, NULL, "alert_fast");
                }
                else if (
                    strcasecmp(optarg, "console:" ALERT_MODE_OPT__FAST) == 0 ||
                    strcasecmp(optarg, ALERT_MODE_OPT__CONSOLE) == 0 )
                {
                    ParseOutput(sc, NULL, "alert_fast: stdout");
                }
                else if ((strcasecmp(optarg, ALERT_MODE_OPT__CMG) == 0) ||
                         (strcasecmp(optarg, ALERT_MODE_OPT__JH) == 0) ||
                         (strcasecmp(optarg, ALERT_MODE_OPT__DJR) == 0))
                {
                    ParseOutput(sc, NULL, "alert_fast: stdout packet");
                    sc->no_log = 1;
                    /* turn on layer2 headers */
                    sc->output_flags |= OUTPUT_FLAG__SHOW_DATA_LINK;
                    /* turn on data dump */
                    sc->output_flags |= OUTPUT_FLAG__APP_DATA;
                }
                else if (strcasecmp(optarg, ALERT_MODE_OPT__AJK) == 0)
                {
                    ParseOutput(sc, NULL, "unified2");
                }
                else if (strcasecmp(optarg, ALERT_MODE_OPT__UNIX_SOCK) == 0)
                {
                    ParseOutput(sc, NULL, "alert_unixsock");
                }
                else if (strcasecmp(optarg, ALERT_MODE_OPT__TEST) == 0)
                {
                    ParseOutput(sc, NULL, "alert_test");
                    sc->no_log = 1;
                }
                else if (strcasecmp(optarg, "console:" ALERT_MODE_OPT__TEST) == 0)
                {
                    ParseOutput(sc, NULL, "alert_test: stdout");
                    sc->no_log = 1;
                }
                else
                {
                    FatalError("Unknown command line alert option: %s\n", optarg);
                }

                break;

            case 'b':  /* log packets in binary format for post-processing */
                ParseOutput(sc, NULL, "log_tcpdump");
                output_logging = 1;
                break;

            case 'B':  /* obfuscate with a substitution mask */
                ConfigObfuscationMask(sc, optarg);
                break;

            case 'c':  /* use configuration file x */
                sc->run_mode_flags |= RUN_MODE_FLAG__IDS;
                snort_conf_file = SnortStrdup(optarg);
                break;

            case 'C':  /* dump the application layer as text only */
                ConfigDumpCharsOnly(sc, NULL);
                break;

            case 'd':  /* dump the application layer data */
                ConfigDumpPayload(sc, NULL);
                break;

#ifndef WIN32
            case 'E':  /* Restarting from daemon mode */
            case 'D':  /* daemon mode */
                /* These are parsed at the beginning so as to start logging
                 * to syslog right away */
                break;
#endif

            case 'e':  /* show second level header info */
                ConfigDecodeDataLink(sc, NULL);
                break;
#ifdef WIN32
            case 'E':  /* log alerts to Event Log */
                ParseOutput(sc, NULL, "alert_syslog");
                sc->logging_flags &= ~LOGGING_FLAG__SYSLOG_REMOTE;
                output_alerting = 1;
                break;
#endif
            case 'f':
                sc->output_flags |= OUTPUT_FLAG__LINE_BUFFER;
                break;

            case 'F':   /* read BPF filter in from a file */
                ConfigBpfFile(sc, optarg);
                break;

#ifndef WIN32
            case 'g':   /* setgid */
                ConfigSetGid(sc, optarg);
                break;
#endif

            case 'G':  /* snort loG identifier */
                sc->event_log_id = SnortStrtoul(optarg, &endptr, 0);
                if ((errno == ERANGE) || (*endptr != '\0') ||
                    (sc->event_log_id > UINT16_MAX))
                {
                    FatalError("Snort log identifier invalid: %s.  It must "
                               "be between 0 and %u.\n", optarg, UINT16_MAX);
                }

                /* Forms upper 2 bytes.  Lower two bytes are the event id */
                sc->event_log_id <<= 16;

                break;

            case 'h':
                /* set home network to x, this will help determine what to set
                 * logging diectories to */
                ConfigReferenceNet(sc, optarg);
                break;

            case 'H':
                sc->run_flags |= RUN_FLAG__STATIC_HASH;
                break;

            case 'i':
                ConfigInterface(sc, optarg);
                break;

            case 'I':  /* add interface name to alert string */
                ConfigAlertWithInterfaceName(sc, NULL);
                break;

            case 'k':  /* set checksum mode */
                ConfigChecksumMode(sc, optarg);
                break;

            case 'K':  /* log mode */
                if (strcasecmp(optarg, LOG_MODE_OPT__NONE) == 0)
                {
                    sc->no_log = 1;
                }
                else if (strcasecmp(optarg, LOG_MODE_OPT__PCAP) == 0)
                {
                    ParseOutput(sc, NULL, "log_tcpdump");
                }
                else if (strcasecmp(optarg, LOG_MODE_OPT__ASCII) == 0)
                {
                    ParseOutput(sc, NULL, "log_ascii");
                }
                else
                {
                    FatalError("Unknown command line log option: %s\n", optarg);
                }

                output_logging = 1;
                break;

            case 'l':  /* use log dir <X> */
                ConfigLogDir(sc, optarg);
                break;

            case 'L':  /* set BinLogFile name */
                /* implies tcpdump format logging
                 * 256 is kind of arbitrary but should be more than enough */
                if (strlen(optarg) < 256)
                {
                    ParseOutput(sc, NULL, "log_tcpdump");
                    sc->pcap_log_file = SnortStrdup(optarg);
                }
                else
                {
                    FatalError("log_tcpdump file name \"%s\" has to be less "
                               "than or equal to 256 characters.\n", optarg);
                }

                output_logging = 1;
                break;

            case 'M':
                /* This is parsed at the beginning so as to start logging
                 * to syslog right away */
                break;

#ifndef WIN32
            case 'm':  /* set the umask for the output files */
                ConfigUmask(sc, optarg);
                break;
#endif

            case 'n':  /* grab x packets and exit */
                ConfigPacketCount(sc, optarg);
                break;

            case 'N':  /* no logging mode */
                ConfigNoLog(sc, NULL);
                break;

            case 'O':  /* obfuscate the logged IP addresses for privacy */
                ConfigObfuscate(sc, NULL);
                break;

            case 'p':  /* disable explicit promiscuous mode */
                ConfigNoPromiscuous(sc, NULL);
                break;

            case 'P':  /* explicitly define snaplength of packets */
                ConfigPacketSnaplen(sc, optarg);
                break;

            case 'q':  /* no stdout output mode */
                /* This is parsed at the beginning so as to start logging
                 * in quiet mode right away */
                break;

#ifndef WIN32
            case 'Q':
                LogMessage("Enabling inline operation\n");
                sc->run_flags |= RUN_FLAG__INLINE;
                break;
#endif
            case ENABLE_INLINE_TEST:
                LogMessage("Enable Inline Test Mode\n");
                sc->run_flags |= RUN_FLAG__INLINE_TEST;
                break;


            case 'r':  /* read packets from a TCPdump file instead of the net */
            case PCAP_SINGLE:
                PQ_Single(optarg);
                sc->run_flags |= RUN_FLAG__READ;
                break;

            case 'R': /* augment pid file name suffix */
                if ((strlen(optarg) >= MAX_PIDFILE_SUFFIX) || (strlen(optarg) <= 0) ||
                    (strstr(optarg, "..") != NULL) || (strstr(optarg, "/") != NULL))
                {
                        FatalError("Invalid pidfile suffix: %s.  Suffix must "
                                   "less than %u characters and not have "
                                   "\"..\" or \"/\" in the name.\n", optarg,
                                   MAX_PIDFILE_SUFFIX);
                }

                SnortStrncpy(sc->pidfile_suffix, optarg, sizeof(sc->pidfile_suffix));
                break;

            case 's':  /* log alerts to syslog */
#ifndef WIN32
                ParseOutput(sc, NULL, "alert_syslog");
#else
                sc->logging_flags |= LOGGING_FLAG__SYSLOG_REMOTE;
#endif
                output_alerting = 1;
                break;

            case 'S':  /* set a rules file variable */
                {
                    char *equal_ptr = strchr(optarg, '=');
                    VarNode *node;

                    if (equal_ptr == NULL)
                    {
                        FatalError("Format for command line variable definitions "
                                   "is:\n -S var=value\n");
                    }

                    /* Save these and parse when snort conf is parsed so
                     * they can be added to the snort conf configuration */
                    node = (VarNode *)SnortAlloc(sizeof(VarNode));
                    node->name = SnortStrndup(optarg, equal_ptr - optarg);

                    /* Make sure it's not already in the list */
                    if (cmd_line_var_list != NULL)
                    {
                        VarNode *tmp = cmd_line_var_list;

                        while (tmp != NULL)
                        {
                            if (strcasecmp(tmp->name, node->name) == 0)
                            {
                                FreeVarList(cmd_line_var_list);
                                FatalError("Duplicate variable name: %s.\n",
                                           tmp->name);
                            }

                            tmp = tmp->next;
                        }
                    }

                    node->value = SnortStrdup(equal_ptr + 1);
                    node->line = SnortStrdup(optarg);
                    node->next = cmd_line_var_list;
                    cmd_line_var_list = node;

                    /* Put line in a parser parsable form - we know the
                     * equals is already there */
                    equal_ptr = strchr(node->line, '=');
                    *equal_ptr = ' ';
                }

                break;

#ifndef WIN32
            case 't':  /* chroot to the user specified directory */
                ConfigChrootDir(sc, optarg);
                break;
#endif

            case 'T':  /* test mode, verify that the rules load properly */
                sc->run_mode_flags |= RUN_MODE_FLAG__TEST;
                break;

#ifndef WIN32
            case 'u':  /* setuid */
                ConfigSetUid(sc, optarg);
                break;
#endif

            case 'U':  /* use UTC */
                ConfigUtc(sc, NULL);
                break;

            case 'v':  /* be verbose */
                ConfigVerbose(sc, NULL);
                break;

            case 'V':  /* prog ver already gets printed out, so we just exit */
                break;

#ifdef WIN32
            case 'W':
                PrintVersion();
                PrintAllInterfaces();
                exit(0);  /* XXX Should maybe use CleanExit here? */
                break;
#endif

#if !defined(NO_NON_ETHER_DECODER) && defined(DLT_IEEE802_11)
            case 'w':  /* show 802.11 all frames info */
                sc->output_flags |= OUTPUT_FLAG__SHOW_WIFI_MGMT;
                break;
#endif
            case 'X':  /* display verbose packet bytecode dumps */
                ConfigDumpPayloadVerbose(sc, NULL);
                break;

            case 'x':
                sc->run_flags |= RUN_FLAG__CONF_ERROR_OUT;
                break;

            case 'y':  /* Add year to timestamp in alert and log files */
                ConfigShowYear(sc, NULL);
                break;

            case 'Z':  /* Set preprocessor perfmon file path/filename */
                ConfigPerfFile(sc, optarg);
                break;

            case PCAP_FILE_LIST:
            case PCAP_LIST:
#ifndef WIN32
            case PCAP_DIR:
#endif
                PQ_Multi((char)ch, optarg);
                sc->run_flags |= RUN_FLAG__READ;
                break;

            case PCAP_LOOP:
                {
                    long int loop_count = SnortStrtol(optarg, &endptr, 0);

                    if ((errno == ERANGE) || (*endptr != '\0') ||
                        (loop_count < 0) || (loop_count > 2147483647))
                    {
                        FatalError("Valid values for --pcap-loop are between 0 and 2147483647\n");
                    }

                    if (loop_count == 0)
                        pcap_loop_count = -1;
                    else
                        pcap_loop_count = loop_count;
                }

                break;

            case PCAP_RESET:
                sc->run_flags |= RUN_FLAG__PCAP_RESET;
                break;

#if defined(SNORT_RELOAD) && !defined(WIN32)
            case PCAP_RELOAD:
                sc->run_flags |= RUN_FLAG__PCAP_RELOAD;
                break;
#endif

#ifndef WIN32
            case PCAP_FILTER:
                PQ_SetFilter(optarg);
                break;

            case PCAP_NO_FILTER:
                PQ_SetFilter(NULL);
                break;
#endif

            case PCAP_SHOW:
                sc->run_flags |= RUN_FLAG__PCAP_SHOW;
                break;
#ifdef MPLS
            case ENABLE_MPLS_MULTICAST:
                ConfigEnableMplsMulticast(sc, NULL);
                break;

            case ENABLE_OVERLAPPING_IP:
                ConfigEnableMplsOverlappingIp(sc, NULL);
                break;

            case MAX_MPLS_LABELCHAIN_LEN:
                ConfigMaxMplsLabelChain(sc, optarg);
                break;

            case MPLS_PAYLOAD_TYPE:
                ConfigMplsPayloadType(sc, optarg);
                break;
#endif
            case REQUIRE_RULE_SID:
                sc->run_flags |= RUN_FLAG__REQUIRE_RULE_SID;
                break;

            case ARG_CS_DIR:
                if ( optarg != NULL )
                    sc->cs_dir = SnortStrdup(optarg);
                break;
#ifdef REG_TEST
            case ARG_HA_PEER:
                sc->ha_peer = true;
                break;

            case ARG_HA_OUT:
                sc->ha_out = SnortStrdup(optarg);
                break;

            case ARG_HA_IN:
                sc->ha_in = SnortStrdup(optarg);
                break;
#endif

            case SUPPRESS_CONFIG_LOG:
                sc->suppress_config_log = 1;
                break;

            case '?':  /* show help and exit with 1 */
                PrintVersion();
                ShowUsage(argv[0]);
                /* XXX Should do a clean exit */
                exit(1);
                break;

            default:
                FatalError("Invalid option: %c.\n", ch);
                break;
        }
    }

    sc->bpf_filter = copy_argv(&argv[optind]);

    if ((sc->run_mode_flags & RUN_MODE_FLAG__TEST) &&
        (sc->run_flags & RUN_FLAG__DAEMON))
    {
        FatalError("Cannot use test mode and daemon mode together.\n"
                   "To verify configuration, run first in test "
                   "mode and then restart in daemon mode.\n");
    }

    if ((sc->run_flags & RUN_FLAG__INLINE) &&
            (sc->run_flags & RUN_FLAG__INLINE_TEST))
    {
        FatalError("Cannot use inline adapter mode and inline test "
                "mode together. \n");
    }

    // TBD no reason why command line args only can't be checked
    // marginally useful, perhaps, but why do we go out of our way
    // to make things hard on the user?
    if ((sc->run_mode_flags & RUN_MODE_FLAG__TEST) &&
        (snort_conf_file == NULL))
    {
        FatalError("Test mode must be run with a snort configuration "
                   "file.  Use the '-c' option on the command line to "
                   "specify a configuration file.\n");
    }
    if (pcap_loop_count && !(sc->run_flags & RUN_FLAG__READ))
    {
        FatalError("--pcap-loop can only be used in combination with pcaps "
                   "on the command line.\n");
    }

#if defined(SNORT_RELOAD) && !defined(WIN32)
    if ((sc->run_flags & RUN_FLAG__PCAP_RELOAD) &&
        !(sc->run_flags & RUN_FLAG__READ))
    {
        FatalError("--pcap-reload can only be used in combination with pcaps "
                   "on the command line.\n");
    }
#endif

    /* Set the run mode based on what we've got from command line */

    /* Version overrides all */
    if (sc->run_mode_flags & RUN_MODE_FLAG__VERSION)
    {
        sc->run_mode = RUN_MODE__VERSION;
    }
    /* Next dumping so rule stubs */
    else if (sc->run_mode_flags & RUN_MODE_FLAG__RULE_DUMP)
    {
        sc->run_mode = RUN_MODE__RULE_DUMP;
    }
    /* Next if we want to test a snort conf */
    else if (sc->run_mode_flags & RUN_MODE_FLAG__TEST)
    {
        sc->run_mode = RUN_MODE__TEST;
    }
    /* Now if there is a snort conf.  If a snort conf wasn't given on the
     * command line, we'll look in a default place if the next ones
     * don't match */
    else if ((sc->run_mode_flags & RUN_MODE_FLAG__IDS) && (snort_conf_file != NULL))
    {
        sc->run_mode = RUN_MODE__IDS;
    }
    /* If logging but not alerting or log directory is set */
    else if ((output_logging && !output_alerting) || (sc->log_dir != NULL))
    {
        sc->no_alert = 1;
        sc->run_mode = RUN_MODE__PACKET_LOG;
    }
    /* If none of the above and not logging or alerting and verbose */
    else if ((!output_logging && !output_alerting) &&
             (sc->logging_flags & LOGGING_FLAG__VERBOSE))
    {
        sc->no_alert = 1;
        sc->no_log = 1;
        sc->run_mode = RUN_MODE__PACKET_DUMP;
    }

#if 1
    if (!sc->run_mode)
    {
        sc->no_alert = 1;
        sc->no_log = 1;
        sc->run_mode = RUN_MODE__PACKET_DUMP;
    }
#else
    if (!sc->run_mode)
        sc->run_mode = RUN_MODE__IDS;

    /* If mode mandates a conf and we don't have one,  check for default. */
    if (((sc->run_mode == RUN_MODE__IDS) || (sc->run_mode == RUN_MODE__TEST)) &&
        (snort_conf_file == NULL))
    {
        snort_conf_file = ConfigFileSearch();
        if (snort_conf_file == NULL)
        {
            DisplayBanner();
            ShowUsage(argv[0]);
            FatalError("\n\nUh, you need to tell me to do something...");
        }
    }
#endif

    if ((sc->run_mode == RUN_MODE__PACKET_LOG) &&
        (sc->output_configs == NULL))
    {
        ParseOutput(sc, NULL, "log_tcpdump");
    }

    switch ( snort_conf->run_mode )
    {
        case RUN_MODE__IDS:
            if (ScLogVerbose())
                log_func = PrintPacket;
            break;

        case RUN_MODE__PACKET_LOG:
            log_func = LogPacket;
            break;

        case RUN_MODE__PACKET_DUMP:
            log_func = PrintPacket;
            break;

        default:
            break;
    }
    SetSnortConfDir();
}

/*
 * Function: SetPktProcessor()
 *
 * Purpose:  Set root decoder based on datalink
 */
// TBD add GetDecoder(dlt) to decode module and hide all
// protocol decoder functions.
static int SetPktProcessor(void)
{
    const char* slink = NULL;
    const char* extra = NULL;
    int dlt = DAQ_GetBaseProtocol();

    switch ( dlt )
    {
        case DLT_EN10MB:
            slink = "Ethernet";
            grinder = DecodeEthPkt;
            break;

#ifdef DLT_LOOP
        case DLT_LOOP:
#endif
        case DLT_NULL:
            /* loopback and stuff.. you wouldn't perform intrusion detection
             * on it, but it's ok for testing. */
            slink = "LoopBack";
            extra = "Data link layer header parsing for this network type "
                    "isn't implemented yet";
            grinder = DecodeNullPkt;
            break;

        case DLT_RAW:
        case DLT_IPV4:
            slink = "Raw IP4";
            extra = "There's no second layer header available for this datalink";
            grinder = DecodeRawPkt;
            break;

        case DLT_IPV6:
            slink = "Raw IP6";
            extra = "There's no second layer header available for this datalink";
            grinder = DecodeRawPkt6;
            break;

#ifdef DLT_I4L_IP
        case DLT_I4L_IP:
            slink = "I4L-ip";
            grinder = DecodeEthPkt;
            break;
#endif

#ifndef NO_NON_ETHER_DECODER
#ifdef DLT_I4L_CISCOHDLC
        case DLT_I4L_CISCOHDLC:
            slink = "I4L-cisco-h";
            grinder = DecodeI4LCiscoIPPkt;
            break;
#endif

        case DLT_PPP:
            slink = "PPP";
            extra = "Second layer header parsing for this datalink "
                    "isn't implemented yet";
            grinder = DecodePppPkt;
            break;

#ifdef DLT_I4L_RAWIP
        case DLT_I4L_RAWIP:
            // you need the I4L modified version of libpcap to get this stuff
            // working
            slink = "I4L-rawip";
            grinder = DecodeI4LRawIPPkt;
            break;
#endif

#ifdef DLT_IEEE802_11
        case DLT_IEEE802_11:
            slink = "IEEE 802.11";
            grinder = DecodeIEEE80211Pkt;
            break;
#endif
#ifdef DLT_ENC
        case DLT_ENC:
            slink = "Encapsulated data";
            grinder = DecodeEncPkt;
            break;

#else
        case 13:
#endif /* DLT_ENC */
        case DLT_IEEE802:
            slink = "Token Ring";
            grinder = DecodeTRPkt;
            break;

        case DLT_FDDI:
            slink = "FDDI";
            grinder = DecodeFDDIPkt;
            break;

#ifdef DLT_CHDLC
        case DLT_CHDLC:
            slink = "Cisco HDLC";
            grinder = DecodeChdlcPkt;
            break;
#endif

        case DLT_SLIP:
            slink = "SLIP";
            extra = "Second layer header parsing for this datalink "
                    "isn't implemented yet\n";
            grinder = DecodeSlipPkt;
            break;

#ifdef DLT_PPP_SERIAL
        case DLT_PPP_SERIAL:         /* PPP with full HDLC header*/
            slink = "PPP Serial";
            extra = "Second layer header parsing for this datalink "
                    " isn't implemented yet";
            grinder = DecodePppSerialPkt;
            break;
#endif

#ifdef DLT_LINUX_SLL
        case DLT_LINUX_SLL:
            slink = "Linux SLL";
            grinder = DecodeLinuxSLLPkt;
            break;
#endif

#ifdef DLT_PFLOG
        case DLT_PFLOG:
            slink = "OpenBSD PF log";
            grinder = DecodePflog;
            break;
#endif

#ifdef DLT_OLDPFLOG
        case DLT_OLDPFLOG:
            slink = "Old OpenBSD PF log";
            grinder = DecodeOldPflog;
            break;
#endif
#endif  // NO_NON_ETHER_DECODER

        default:
            /* oops, don't know how to handle this one */
            FatalError("Cannot decode data link type %d\n", dlt);
            break;
    }

    if ( !ScReadMode() || ScPcapShow() )
    {
        LogMessage("Decoding %s\n", slink);
    }
    if (extra && ScOutputDataLink())
    {
        LogMessage("%s\n", extra);
        snort_conf->output_flags &= ~OUTPUT_FLAG__SHOW_DATA_LINK;
    }
#ifdef ACTIVE_RESPONSE
    Encode_Init();
#endif
    return 0;
}

/*
 *  Handle idle time checks in snort packet processing loop
 */
static void SnortIdle(void)
{
    /* Rollover of performance log */
    if (IsSetRotatePerfFileFlag())
    {
        sfRotateBaseStatsFile(perfmon_config);
        sfRotateFlowStatsFile(perfmon_config);
        ClearRotatePerfFileFlag();
    }
#ifdef OPENBSD
    else if (reload_signal != reload_total)
        nanosleep(&packet_sleep, NULL);
#endif

    if( session_api )
        session_api->check_session_timeout(16384, time(NULL));
    ControlSocketDoWork(1);
#ifdef SIDE_CHANNEL
    SideChannelDrainRX(0);
#endif
    IdleProcessingExecute();
}

void PacketLoop (void)
{
    int error;
    int pkts_to_read = (int)snort_conf->pkt_cnt;

    TimeStart();

    while ( !exit_logged )
    {
        error = DAQ_Acquire(pkts_to_read, PacketCallback, NULL);

#ifdef CONTROL_SOCKET
        if (packet_dump_stop)
            PacketDumpClose();
#endif

#ifdef SIDE_CHANNEL
        /* If we didn't manage to lock the process lock in a DAQ acquire callback, lock it now. */
        if (ScSideChannelEnabled() && !snort_process_lock_held)
        {
            pthread_mutex_lock(&snort_process_lock);
            snort_process_lock_held = true;
        }
#endif

        if ( error )
        {
            if ( !ScReadMode() || !PQ_Next() )
            {
                /* If not read-mode or no next pcap, we're done */
                break;
            }
        }
        /* Check for any pending signals when no packets are read*/
        else
        {
            // TBD SnortIdle() only checks for perf file rotation
            // and that can only be done after calling SignalCheck()
            // so either move SnortIdle() to SignalCheck() or directly
            // set the flag in the signal handler (and then clear it
            // in SnortIdle()).

            // check for signals
            if ( SignalCheck() )
            {
#ifndef SNORT_RELOAD
                // Got SIGNAL_SNORT_RELOAD
                Restart();
#endif
            }
            CheckForReload();
        }
        if ( pkts_to_read > 0 )
        {
            if ( snort_conf->pkt_cnt <= pc.total_from_daq )
                break;
            else
                pkts_to_read = (int)(snort_conf->pkt_cnt - pc.total_from_daq);
        }
        // idle time processing..quick things to check or do ...
        // TBD fix this per above ... and if it stays here, should
        // prolly change the name if acquire breaks due to a signal
        // (since in that case we aren't idle here)
        SnortIdle();

#ifdef SIDE_CHANNEL
        /* Unlock the Snort process lock once we've hit the DAQ acquire timeout. */
        if (snort_process_lock_held)
        {
            snort_process_lock_held = false;
            pthread_mutex_unlock(&snort_process_lock);
        }
#endif
    }
#ifdef CONTROL_SOCKET
    PacketDumpClose();
#endif

#ifdef SIDE_CHANNEL
    /* Error conditions can lead to exiting the packet loop prior to unlocking the process lock.  */
    if (snort_process_lock_held)
    {
        snort_process_lock_held = false;
        pthread_mutex_unlock(&snort_process_lock);
    }
#endif

    if ( !exit_logged && error )
    {
        if ( error == DAQ_READFILE_EOF )
            error = 0;
        else if ( error > 0 )
        {
            DAQ_Abort();
            exit(1);
        }
        CleanExit(error);
    }
    done_processing = 1;
}

/* Resets Snort to a post-configuration state */
static void SnortReset(void)
{
    PreprocSignalFuncNode *idxPreprocReset;
    PreprocSignalFuncNode *idxPreprocResetStats;

    /* reset preprocessors */
    idxPreprocReset = preproc_reset_funcs;
    while (idxPreprocReset != NULL)
    {
        idxPreprocReset->func(-1, idxPreprocReset->arg);
        idxPreprocReset = idxPreprocReset->next;
    }

    SnortEventqReset();
    Replace_ResetQueue();
#ifdef ACTIVE_RESPONSE
    Active_ResetQueue();
#endif

    sfthreshold_reset_active();
    RateFilter_ResetActive();
    TagCacheReset();

#ifdef PERF_PROFILING
    ShowPreprocProfiles();
    ShowRuleProfiles();
#endif

    DropStats(0);

    /* zero out packet count */
    memset(&pc, 0, sizeof(pc));

#ifdef PERF_PROFILING
    ResetRuleProfiling();
    ResetPreprocProfiling();
#endif

    /* reset preprocessor stats */
    idxPreprocResetStats = preproc_reset_stats_funcs;
    while (idxPreprocResetStats != NULL)
    {
        idxPreprocResetStats->func(-1, idxPreprocResetStats->arg);
        idxPreprocResetStats = idxPreprocResetStats->next;
    }
}


#if 0
/* locate one of the possible default config files */
/* allocates memory to hold filename */
static char *ConfigFileSearch(void)
{
    struct stat st;
    int i;
    char *conf_files[]={"/etc/snort.conf", "./snort.conf", NULL};
    char *fname = NULL;
    char *rval = NULL;

    i = 0;

    /* search the default set of config files */
    while(conf_files[i])
    {
        fname = conf_files[i];

        if(stat(fname, &st) != -1)
        {
            rval = SnortStrdup(fname);
            break;
        }
        i++;
    }

    /* search for .snortrc in the HOMEDIR */
    if(!rval)
    {
        char *home_dir = NULL;

        if((home_dir = getenv("HOME")) != NULL)
        {
            char *snortrc = "/.snortrc";
            int path_len;

            path_len = strlen(home_dir) + strlen(snortrc) + 1;

            /* create the full path */
            fname = (char *)SnortAlloc(path_len);

            SnortSnprintf(fname, path_len, "%s%s", home_dir, snortrc);

            if(stat(fname, &st) != -1)
                rval = fname;
            else
                free(fname);
        }
    }

    return rval;
}
#endif

/* Signal Handlers ************************************************************/
static void SigExitHandler(int signal)
{
    if (exit_signal != 0)
        return;

    /* Don't want to have to wait to start processing packets before
     * getting out of dodge */
    if (snort_initializing)
        _exit(0);

    exit_signal = signal;
}

static void SigDumpStatsHandler(int signal)
{
    dump_stats_signal = true;
}

static void SigRotateStatsHandler(int signal)
{
    rotate_stats_signal = true;
}

static void SigReloadHandler(int signal)
{
#if defined(SNORT_RELOAD) && !defined(WIN32)
    reload_signal++;
#else
    reload_signal = true;
#endif
}

#ifdef CONTROL_SOCKET
static void SigPipeHandler(int signal)
{
}
#endif

#ifdef TARGET_BASED
static void SigNoAttributeTableHandler(int signal)
{
   no_attr_table_signal = true;
}
#endif

static void SigOopsHandler(int signal)
{
    if ( s_packet.pkth )
    {
        s_pkth = *s_packet.pkth;

        if ( s_packet.pkt )
            memcpy(s_data, s_packet.pkt, 0xFFFF & s_packet.pkth->caplen);
    }
    SnortAddSignal(signal, SIG_DFL, 0);

    raise(signal);
}

static void PrintStatistics (void)
{
    if ( ScTestMode() || ScVersionMode() || ScRuleDumpMode() )
        return;

    fpShowEventStats(snort_conf);

#ifdef PERF_PROFILING
    {
        int saved_internal_log_level = internal_log_level;
        internal_log_level = INTERNAL_LOG_LEVEL__MESSAGE;

        ShowPreprocProfiles();
        ShowRuleProfiles();

        internal_log_level = saved_internal_log_level;
    }
#endif

    DropStats(2);
    print_thresholding(snort_conf->threshold_config, 1);
}

/****************************************************************************
 *
 * Function: CleanExit()
 *
 * Purpose:  Clean up misc file handles and such and exit
 *
 * Arguments: exit value;
 *
 * Returns: void function
 *
 ****************************************************************************/
static void CleanExit(int exit_val)
{
    SnortConfig tmp;

#ifdef TARGET_BASED
#ifdef DEBUG
#if 0
    SFLAT_dump();
#endif
#endif
#endif

    /* Have to trick LogMessage to log correctly after snort_conf
     * is freed */
    memset(&tmp, 0, sizeof(tmp));

    if (snort_conf != NULL)
    {
        tmp.internal_log_level = snort_conf->internal_log_level;
        tmp.run_mode = snort_conf->run_mode;
        tmp.run_flags |= (snort_conf->run_flags & RUN_FLAG__DAEMON);

        tmp.logging_flags |=
            (snort_conf->logging_flags & LOGGING_FLAG__SYSLOG);
    }

    SnortCleanup(exit_val);
    snort_conf = &tmp;

    if (!ScVersionMode())
    {
        LogMessage("Snort exiting\n");
    }
#ifndef WIN32
    closelog();
#endif
    if ( !done_processing )
        exit(exit_val);
}

static void SnortCleanup(int exit_val)
{
    PreprocSignalFuncNode *idxPreproc = NULL;
    PluginSignalFuncNode *idxPlugin = NULL;

    /* This function can be called more than once.  For example,
     * once from the SIGINT signal handler, and once recursively
     * as a result of calling pcap_close() below.  We only need
     * to perform the cleanup once, however.  So the static
     * variable already_exiting will act as a flag to prevent
     * double-freeing any memory.  Not guaranteed to be
     * thread-safe, but it will prevent the simple cases.
     */
    static int already_exiting = 0;
    if( already_exiting != 0 )
    {
        return;
    }
    already_exiting = 1;
    snort_exiting = 1;
    snort_initializing = false;  /* just in case we cut out early */

    Active_Suspend();  // rules that fire now can't actually block

    if ( DAQ_WasStarted() )
    {
#ifdef EXIT_CHECK
        if (snort_conf->exit_check)
            ExitCheckEnd();
#endif
        DAQ_Stop();
    }

    ControlSocketCleanUp();
#ifdef SIDE_CHANNEL
    SideChannelStopTXThread();
    SideChannelCleanUp();
#endif
    IdleProcessingCleanUp();

    if ( snort_conf->dirty_pig )
    {
        DAQ_Delete();
        DAQ_Term();
        ScRestoreInternalLogLevel();
        PrintStatistics();
        return;
    }
#if defined(SNORT_RELOAD) && !defined(WIN32)
    /* Setting snort_exiting will cause the thread to break out
     * of it's loop and exit */
    if (snort_reload_thread_created)
        pthread_join(snort_reload_thread_id, NULL);
#endif

#if defined(INLINE_FAILOPEN) && !defined(WIN32)
    if (inline_failopen_thread_running)
        pthread_kill(inline_failopen_thread_id, SIGKILL);
#endif

#if defined(TARGET_BASED) && !defined(WIN32)
    if (attribute_reload_thread_running)
    {
        /* Set the flag to stop the attribute reload thread and
         * send VTALRM signal to pull it out of the idle sleep.
         * Thread exits normally on next iteration through its
         * loop.
         *
         * If its doing other processing, that continues post
         * interrupt and thread exits normally.
         */
        attribute_reload_thread_stop = 1;
        pthread_kill(attribute_reload_thread_id, SIGVTALRM);
        while (attribute_reload_thread_running)
            nanosleep(&thread_sleep, NULL);
        pthread_join(attribute_reload_thread_id, NULL);
    }
#endif

    /* Do some post processing on any incomplete Preprocessor Data */
    idxPreproc = preproc_shutdown_funcs;
    while (idxPreproc)
    {
        idxPreproc->func(SIGQUIT, idxPreproc->arg);
        idxPreproc = idxPreproc->next;
    }

    /* Do some post processing on any incomplete Plugin Data */
    idxPlugin = plugin_shutdown_funcs;
    while(idxPlugin)
    {
        idxPlugin->func(SIGQUIT, idxPlugin->arg);
        idxPlugin = idxPlugin->next;
    }

    if (!ScTestMode() && !ScVersionMode() && !ScRuleDumpMode() )
    {
        if ( !exit_val )
            TimeStop();
    }

    /* Exit preprocessors */
    idxPreproc = preproc_clean_exit_funcs;
    while(idxPreproc)
    {
        idxPreproc->func(SIGQUIT, idxPreproc->arg);
        idxPreproc = idxPreproc->next;
    }

    /* Do some post processing on any incomplete Plugin Data */
    idxPlugin = plugin_clean_exit_funcs;
    while(idxPlugin)
    {
        idxPlugin->func(SIGQUIT, idxPlugin->arg);
        idxPlugin = idxPlugin->next;
    }

    if (decoderActionQ != NULL)
    {
        sfActionQueueDestroy (decoderActionQ);
        mempool_destroy (&decoderAlertMemPool);
        decoderActionQ = NULL;
        memset(&decoderAlertMemPool, 0, sizeof(decoderAlertMemPool));
    }

    DAQ_Delete();
    DAQ_Term();
    ScRestoreInternalLogLevel(); // Do we need this?
    PrintStatistics();

#ifdef ACTIVE_RESPONSE
    Active_Term();
    Encode_Term();
#endif


    CleanupProtoNames();

#ifdef TARGET_BASED
    SFAT_Cleanup();
#endif

    PQ_CleanUp();

    ClosePidFile();

    /* remove pid file */
    if (SnortStrnlen(snort_conf->pid_filename, sizeof(snort_conf->pid_filename)) > 0)
    {
        int ret;

        ret = unlink(snort_conf->pid_filename);

        if (ret != 0)
        {
            ErrorMessage("Could not remove pid file %s: %s\n",
                         snort_conf->pid_filename, strerror(errno));
        }
    }

#ifdef INTEL_SOFT_CPM
    //IntelPmPrintBufferStats();
#endif

    /* free allocated memory */
    if (snort_conf == snort_cmd_line_conf)
    {
        SnortConfFree(snort_cmd_line_conf);
        snort_cmd_line_conf = NULL;
        snort_conf = NULL;
    }
    else
    {
        SnortConfFree(snort_cmd_line_conf);
        snort_cmd_line_conf = NULL;
        SnortConfFree(snort_conf);
        snort_conf = NULL;
    }

#ifdef SNORT_RELOAD
    if (snort_conf_new != NULL)
    {
        /* If main thread is exiting, it won't swap in the new configuration,
         * so free it here, really just to quiet valgrind.  Note this needs to
         * be done here since some preprocessors, will potentially need access
         * to the data here since stream5 flushes out its cache and potentially
         * sends reassembled packets back through Preprocess */
        SnortConfFree(snort_conf_new);
        snort_conf_new = NULL;
    }
#endif

    EventTrace_Term();

    detection_filter_cleanup();
    sfthreshold_free();
    RateFilter_Cleanup();
    asn1_free_mem();
    FreeOutputConfigFuncs();
    FreePreprocConfigFuncs();

    FreeRuleOptConfigFuncs(rule_opt_config_funcs);
    rule_opt_config_funcs = NULL;

    FreeRuleOptOverrideInitFuncs(rule_opt_override_init_funcs);
    rule_opt_override_init_funcs = NULL;

    FreeRuleOptByteOrderFuncs(rule_opt_byte_order_funcs);
    rule_opt_byte_order_funcs = NULL;

    FreeRuleOptParseCleanupList(rule_opt_parse_cleanup_list);
    rule_opt_parse_cleanup_list = NULL;

    FreeOutputList(AlertList);
    AlertList = NULL;

    FreeOutputList(LogList);
    LogList = NULL;

    /* Global lists */
    FreePreprocStatsFuncs(preproc_stats_funcs);
    preproc_stats_funcs = NULL;

    FreePreprocSigFuncs(preproc_shutdown_funcs);
    preproc_shutdown_funcs = NULL;

    FreePreprocSigFuncs(preproc_clean_exit_funcs);
    preproc_clean_exit_funcs = NULL;

    FreePreprocSigFuncs(preproc_reset_funcs);
    preproc_reset_funcs = NULL;

    FreePreprocSigFuncs(preproc_reset_stats_funcs);
    preproc_reset_stats_funcs = NULL;

    FreePluginSigFuncs(plugin_shutdown_funcs);
    plugin_shutdown_funcs = NULL;

    FreePluginSigFuncs(plugin_clean_exit_funcs);
    plugin_clean_exit_funcs = NULL;

#ifdef SNORT_RELOAD
    FreePluginPostConfigFuncs(plugin_reload_funcs);
    plugin_reload_funcs = NULL;
#endif

    FreePeriodicFuncs(periodic_check_funcs);
    periodic_check_funcs = NULL;

    ParserCleanup();

    /* Stuff from plugbase */
    DynamicRuleListFree(dynamic_rules);
    dynamic_rules = NULL;

    CloseDynamicPreprocessorLibs();
    CloseDynamicDetectionLibs();
    CloseDynamicEngineLibs();
#ifdef SIDE_CHANNEL
    CloseDynamicSideChannelLibs();
#endif
    output_unload();

    CleanupTag();
    ClearDumpBuf();

#ifdef PERF_PROFILING
    CleanupPreprocStatsNodeList();
#endif

    if (netmasks != NULL)
    {
        free(netmasks);
        netmasks = NULL;
    }

    if (protocol_names != NULL)
    {
        int i;

        for (i = 0; i < NUM_IP_PROTOS; i++)
        {
            if (protocol_names[i] != NULL)
                free(protocol_names[i]);
        }

        free(protocol_names);
        protocol_names = NULL;
    }

#ifdef INTEL_SOFT_CPM
    IntelPmStopInstance();
#endif

    SynToMulticastDstIpDestroy();

    FreeVarList(cmd_line_var_list);

    if (snort_conf_file != NULL)
        free(snort_conf_file);

    if (snort_conf_dir != NULL)
        free(snort_conf_dir);

    if (s_packet.ip6_extensions != NULL)
	free(s_packet.ip6_extensions);

    close_fileAPI();
}

void Restart(void)
{
    int daemon_mode = ScDaemonMode();

#ifndef WIN32
    if ((!ScReadMode() && (getuid() != 0)) ||
        (snort_conf->chroot_dir != NULL))
    {
        LogMessage("Reload via Signal Reload does not work if you aren't root "
                   "or are chroot'ed.\n");
# ifdef SNORT_RELOAD
        /* We are restarting because of a configuration verification problem */
        CleanExit(1);
# else
        return;
# endif
    }
#endif  /* WIN32 */

    LogMessage("\n");
    LogMessage("***** Restarting Snort *****\n");
    LogMessage("\n");
    SnortCleanup(0);

    if (daemon_mode)
    {
        int ch;
        int option_index = -1;

        optind = 1;

        while ((ch = getopt_long(snort_argc, snort_argv, valid_options, long_options, &option_index)) != -1)
        {
            switch (ch)
            {
                case 'D':
                    {
                        int i = optind-1, j;
                        int index = strlen(snort_argv[i]) - 1;

                        /* 'D' isn't the last option in the opt string so
                         * optind hasn't moved past this option string yet */
                        if ((snort_argv[i][0] != '-')
                                || ((index > 0) && (snort_argv[i][1] == '-'))
                                || (snort_argv[i][index] != 'D'))
                        {
                            i++;
                        }

                        /* Replace -D with -E to indicate we've already daemonized */
                        for (j = 0; j < (int)strlen(snort_argv[i]); j++)
                        {
                            if (snort_argv[i][j] == 'D')
                            {
                                snort_argv[i][j] = 'E';
                                break;
                            }
                        }
                    }

                    break;

                default:
                    break;
            }
        }
    }

#ifdef PARANOID
    execv(snort_argv[0], snort_argv);
#else
    execvp(snort_argv[0], snort_argv);
#endif

    /* only get here if we failed to restart */
    LogMessage("Restarting %s failed: %s\n", snort_argv[0], strerror(errno));

#ifndef WIN32
    closelog();
#endif

    exit(-1);
}

void print_packet_count(void)
{
    LogMessage("[" STDu64 "]", pc.total_from_daq);
}

/*
 *  Check for signal activity
 */
int SignalCheck(void)
{
    switch (exit_signal)
    {
        case SIGTERM:
            if (!exit_logged)
            {
                ErrorMessage("*** Caught Term-Signal\n");
                exit_logged = 1;
                if ( DAQ_BreakLoop(DAQ_SUCCESS) )
                    return 0;
            }
            CleanExit(0);
            break;

        case SIGINT:
            if (!exit_logged)
            {
                ErrorMessage("*** Caught Int-Signal\n");
                exit_logged = 1;
                if ( DAQ_BreakLoop(DAQ_SUCCESS) )
                    return 0;
            }
            CleanExit(0);
            break;

        case SIGQUIT:
            if (!exit_logged)
            {
                ErrorMessage("*** Caught Quit-Signal\n");
                exit_logged = 1;
                if ( DAQ_BreakLoop(DAQ_SUCCESS) )
                    return 0;
            }
            CleanExit(0);
            break;

        default:
            break;
    }

    exit_signal = 0;

    if (dump_stats_signal)
    {
        ErrorMessage("*** Caught Dump Stats-Signal\n");
        DropStats(0);
    }

    dump_stats_signal = false;

    if (rotate_stats_signal)
    {
        ErrorMessage("*** Caught Signal: 'Rotate Perfmonitor Stats'\n");

        /* Make sure the preprocessor is enabled - it can only be enabled
         * in default policy */
        if (!ScIsPreprocEnabled(PP_PERFMONITOR, 0))
        {
            ErrorMessage("!!! Cannot rotate stats - Perfmonitor is not configured !!!\n");
        }
        else
        {
            SetRotatePerfFileFlag();
        }
    }

    rotate_stats_signal = false;

#ifdef TARGET_BASED
    if (no_attr_table_signal)
        ErrorMessage("!!! Cannot reload attribute table - Attribute table is not configured !!!\n");
    no_attr_table_signal = false;
#endif

#ifndef SNORT_RELOAD
    if (reload_signal )
    {
        ErrorMessage("*** Caught Reload-Signal\n");
        reload_signal = false;
        return 1;
    }
    reload_signal = false;
#endif

    return 0;
}

static void InitGlobals(void)
{
    memset(&pc, 0, sizeof(pc));

    InitNetmasks();
    InitProtoNames();
#ifdef SIDE_CHANNEL
    pthread_mutex_init(&snort_process_lock, NULL);
#endif
}

/* Alot of this initialization can be skipped if not running in IDS mode
 * but the goal is to minimize config checks at run time when running in
 * IDS mode so we keep things simple and enforce that the only difference
 * among run_modes is how we handle packets via the log_func. */
SnortConfig * SnortConfNew(void)
{
    SnortConfig *sc = (SnortConfig *)SnortAlloc(sizeof(SnortConfig));

    sc->pkt_cnt = 0;
#ifdef REG_TEST
    sc->pkt_skip = 0;
#endif
    sc->pkt_snaplen = -1;
    /*user_id and group_id should be initialized to -1 by default, because
     * chown() use this later, -1 means no change to user_id/group_id*/
    sc->user_id = -1;
    sc->group_id = -1;

    sc->checksum_flags = CHECKSUM_FLAG__ALL;
    sc->tagged_packet_limit = 256;
    sc->default_rule_state = RULE_STATE_ENABLED;
    sc->pcre_match_limit = 1500;
    sc->pcre_match_limit_recursion = 1500;
    sc->ipv6_max_frag_sessions = 10000;
    sc->ipv6_frag_timeout = 60;  /* This is the default timeout on BSD */

    memset(sc->pid_path, 0, sizeof(sc->pid_path));
    memset(sc->pid_filename, 0, sizeof(sc->pid_filename));
    memset(sc->pidfile_suffix, 0, sizeof(sc->pidfile_suffix));

#ifdef TARGET_BASED
    /* Default max size of the attribute table */
    sc->max_attribute_hosts = DEFAULT_MAX_ATTRIBUTE_HOSTS;
    sc->max_attribute_services_per_host = DEFAULT_MAX_ATTRIBUTE_SERVICES_PER_HOST;

    /* Default max number of services per rule */
    sc->max_metadata_services = DEFAULT_MAX_METADATA_SERVICES;
#endif
#if defined(FEAT_OPEN_APPID)
    sc->max_metadata_appid = DEFAULT_MAX_METADATA_APPID;
#endif /* defined(FEAT_OPEN_APPID) */

#ifdef MPLS
    sc->mpls_stack_depth = DEFAULT_LABELCHAIN_LENGTH;
#endif

    sc->targeted_policies = NULL;
    sc->num_policies_allocated = 0;

#ifndef REG_TEST
    sc->paf_max = DEFAULT_PAF_MAX;
#endif

    /* Default secure hash pattern type */
    sc->Default_Protected_Content_Hash_Type = SECHASH_NONE;

    sc->max_ip6_extensions = DEFAULT_MAX_IP6_EXTENSIONS;

    sc->internal_log_level = INTERNAL_LOG_LEVEL__MESSAGE;

    return sc;
}

void SnortConfFree(SnortConfig *sc)
{
    tSfPolicyId i;

    if (sc == NULL)
        return;

    if (sc->dynamic_rules_path != NULL)
        free(sc->dynamic_rules_path);

    if (sc->log_dir != NULL)
        free(sc->log_dir);

    if (sc->orig_log_dir != NULL)
        free(sc->orig_log_dir);

    if (sc->interface != NULL)
        free(sc->interface);

    if (sc->bpf_file != NULL)
        free(sc->bpf_file);

    if (sc->pcap_log_file != NULL)
        free(sc->pcap_log_file);

    if (sc->chroot_dir != NULL)
        free(sc->chroot_dir);

    if (sc->alert_file != NULL)
        free(sc->alert_file);

    if (sc->perf_file != NULL)
        free(sc->perf_file);

    if (sc->bpf_filter != NULL)
        free(sc->bpf_filter);

    if (sc->event_trace_file != NULL)
        free(sc->event_trace_file);

#ifdef PERF_PROFILING
    if (sc->profile_rules.filename != NULL)
        free(sc->profile_rules.filename);

    if (sc->profile_preprocs.filename != NULL)
        free(sc->profile_preprocs.filename);
#endif

    FreeDynamicLibInfos(sc);

    FreeOutputConfigs(sc->output_configs);
    FreeOutputConfigs(sc->rule_type_output_configs);
#ifdef SIDE_CHANNEL
    FreeSideChannelModuleConfigs(sc->side_channel_config.module_configs);
#endif
    FreePreprocConfigs(sc);

    if (sc->config_table != NULL)
        sfghash_delete(sc->config_table);

    if (sc->base_version != NULL)
        free(sc->base_version);

    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        SnortPolicyFree(sc->targeted_policies[i]);
    }

    FreeRuleStateList(sc->rule_state_list);
    FreeClassifications(sc->classifications);
    FreeReferences(sc->references);

    FreeRuleLists(sc);
    SoRuleOtnLookupFree(sc->so_rule_otn_map);
    OtnLookupFree(sc->otn_map);
    VarTablesFree(sc);
    PortTablesFree(sc->port_tables);
    FastPatternConfigFree(sc->fast_pattern_config);

    ThresholdConfigFree(sc->threshold_config);
    RateFilter_ConfigFree(sc->rate_filter_config);
    DetectionFilterConfigFree(sc->detection_filter_config);

    FreePlugins(sc);

    PreprocessorRuleOptionsFree(sc->preproc_rule_options);

    OtnxMatchDataFree(sc->omd);

    if (sc->pcre_ovector != NULL)
        free(sc->pcre_ovector);

    if ( sc->event_queue_config )
        EventQueueConfigFree(sc->event_queue_config);

    if ( sc->event_queue )
        SnortEventqFree(sc->event_queue);

    if (sc->ip_proto_only_lists != NULL)
    {
        unsigned int j;

        for (j = 0; j < NUM_IP_PROTOS; j++)
            sflist_free_all(sc->ip_proto_only_lists[j], NULL);

        free(sc->ip_proto_only_lists);
    }

    sfPolicyFini(sc->policy_config);

    fpDeleteFastPacketDetection(sc);

    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        SnortPolicy *p = sc->targeted_policies[i];

        if (p != NULL)
            free(p);
    }

    free(sc->targeted_policies);

    if ( sc->react_page )
        free(sc->react_page);

    if ( sc->daq_type )
        free(sc->daq_type);

    if ( sc->daq_mode )
        free(sc->daq_mode);

    if ( sc->daq_vars )
        StringVector_Delete(sc->daq_vars);

    if ( sc->daq_dirs )
        StringVector_Delete(sc->daq_dirs);

#ifdef ACTIVE_RESPONSE
    if ( sc->respond_device )
        free(sc->respond_device);

     if (sc->eth_dst )
        free(sc->eth_dst);
#endif

    if (sc->gtp_ports)
        free(sc->gtp_ports);

    if(sc->cs_dir)
        free(sc->cs_dir);

#ifdef REG_TEST

    if(sc->ha_out)
        free(sc->ha_out);

    if(sc->ha_in)
        free(sc->ha_in);
#endif

    free_file_config(sc->file_config);

#ifdef SIDE_CHANNEL
    if (sc->side_channel_config.opts)
        free(sc->side_channel_config.opts);
#endif

#ifdef INTEL_SOFT_CPM
    IntelPmRelease(sc->ipm_handles);
#endif

#ifdef SNORT_RELOAD
    FreePreprocessorReloadData(sc);
#endif

    free(sc);
#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif

}

/****************************************************************************
 *
 * Function: InitNetMasks()
 *
 * Purpose: Loads the netmask struct in network order.  Yes, I know I could
 *          just load the array when I define it, but this is what occurred
 *          to me when I wrote this at 3:00 AM.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
static void InitNetmasks(void)
{
    if (netmasks == NULL)
        netmasks = (uint32_t *)SnortAlloc(33 * sizeof(uint32_t));

    netmasks[0]  = 0x00000000;
    netmasks[1]  = 0x80000000;
    netmasks[2]  = 0xC0000000;
    netmasks[3]  = 0xE0000000;
    netmasks[4]  = 0xF0000000;
    netmasks[5]  = 0xF8000000;
    netmasks[6]  = 0xFC000000;
    netmasks[7]  = 0xFE000000;
    netmasks[8]  = 0xFF000000;
    netmasks[9]  = 0xFF800000;
    netmasks[10] = 0xFFC00000;
    netmasks[11] = 0xFFE00000;
    netmasks[12] = 0xFFF00000;
    netmasks[13] = 0xFFF80000;
    netmasks[14] = 0xFFFC0000;
    netmasks[15] = 0xFFFE0000;
    netmasks[16] = 0xFFFF0000;
    netmasks[17] = 0xFFFF8000;
    netmasks[18] = 0xFFFFC000;
    netmasks[19] = 0xFFFFE000;
    netmasks[20] = 0xFFFFF000;
    netmasks[21] = 0xFFFFF800;
    netmasks[22] = 0xFFFFFC00;
    netmasks[23] = 0xFFFFFE00;
    netmasks[24] = 0xFFFFFF00;
    netmasks[25] = 0xFFFFFF80;
    netmasks[26] = 0xFFFFFFC0;
    netmasks[27] = 0xFFFFFFE0;
    netmasks[28] = 0xFFFFFFF0;
    netmasks[29] = 0xFFFFFFF8;
    netmasks[30] = 0xFFFFFFFC;
    netmasks[31] = 0xFFFFFFFE;
    netmasks[32] = 0xFFFFFFFF;
}

/****************************************************************************
 *
 * Function: InitProtoNames()
 *
 * Purpose: Initializes the protocol names
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
static void InitProtoNames(void)
{
    int i;

    if (protocol_names == NULL)
        protocol_names = (char **)SnortAlloc(sizeof(char *) * NUM_IP_PROTOS);

    for (i = 0; i < NUM_IP_PROTOS; i++)
    {
        switch(i)
        {
#ifdef REG_TEST
#define PROTO_000 "IP"        //Appears as HOPOPT on some systems
#define PROTO_004 "IPENCAP"   //Appears as IPV4 on some systems
#define PROTO_255 "PROTO:255" //Appears as RESERVED on some systems
            case 0:
                protocol_names[i] = SnortStrdup(PROTO_000);
                break;
            case 4:
                protocol_names[i] = SnortStrdup(PROTO_004);
                break;
            case 255:
                protocol_names[i] = SnortStrdup(PROTO_255);
                break;
#endif
            default:
            {
                struct protoent *pt = getprotobynumber(i);

                if (pt != NULL)
                {
                    size_t j;

                    protocol_names[i] = SnortStrdup(pt->p_name);
                    for (j = 0; j < strlen(protocol_names[i]); j++)
                        protocol_names[i][j] = toupper(protocol_names[i][j]);
                }
                else
                {
                    char protoname[10];

                    SnortSnprintf(protoname, sizeof(protoname), "PROTO:%03d", i);
                    protocol_names[i] = SnortStrdup(protoname);
                }
            }
        }
    }
}


static void SetSnortConfDir(void)
{
    /* extract the config directory from the config filename */
    if (snort_conf_file != NULL)
    {
#ifndef WIN32
        char *path_sep = strrchr(snort_conf_file, '/');
#else
        char *path_sep = strrchr(snort_conf_file, '\\');
#endif

        /* is there a directory seperator in the filename */
        if (path_sep != NULL)
        {
            path_sep++;  /* include path separator */
            snort_conf_dir = SnortStrndup(snort_conf_file, path_sep - snort_conf_file);
        }
        else
        {
            snort_conf_dir = SnortStrdup("./");
        }

        DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Config file = %s, config dir = "
                    "%s\n", snort_conf_file, snort_conf_dir););
    }
}

static void FreePlugins(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    FreePreprocessors(sc);

    FreePluginPostConfigFuncs(sc->plugin_post_config_funcs);
    sc->plugin_post_config_funcs = NULL;
}

static void FreePreprocessors(SnortConfig *sc)
{
    tSfPolicyId i;

    if (sc == NULL)
        return;

    FreePreprocCheckConfigFuncs(sc->preproc_config_check_funcs);
    sc->preproc_config_check_funcs = NULL;

    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        SnortPolicy *p = sc->targeted_policies[i];

        if (p == NULL)
            continue;

        FreePreprocEvalFuncs(p->preproc_eval_funcs);
        p->preproc_eval_funcs = NULL;
        p->num_preprocs = 0;

        FreePreprocEvalFuncs(p->unused_preproc_eval_funcs);
        p->unused_preproc_eval_funcs = NULL;

        FreePreprocMetaEvalFuncs(p->preproc_meta_eval_funcs);
        p->preproc_meta_eval_funcs = NULL;
        p->num_meta_preprocs = 0;

        FreeDetectionEvalFuncs(p->detect_eval_funcs);
        p->detect_eval_funcs = NULL;
        p->num_detects = 0;
    }

    FreePreprocPostConfigFuncs(sc->preproc_post_config_funcs);
    sc->preproc_post_config_funcs = NULL;
}

static SnortConfig * MergeSnortConfs(SnortConfig *cmd_line, SnortConfig *config_file)
{
    unsigned int i;

    /* Move everything from the command line config over to the
     * config_file config */

    if (cmd_line == NULL)
    {
        FatalError("%s(%d) Merging snort configs: snort conf is NULL.\n",
                   __FILE__, __LINE__);
    }

    ResolveOutputPlugins(cmd_line, config_file);

    if (config_file == NULL)
    {
        if (cmd_line->log_dir == NULL)
            cmd_line->log_dir = SnortStrdup(DEFAULT_LOG_DIR);
    }
    else if ((cmd_line->log_dir == NULL) && (config_file->log_dir == NULL))
    {
        config_file->log_dir = SnortStrdup(DEFAULT_LOG_DIR);
    }
    else if (cmd_line->log_dir != NULL)
    {
        if (config_file->log_dir != NULL)
            free(config_file->log_dir);

        config_file->log_dir = SnortStrdup(cmd_line->log_dir);
    }

    if (config_file == NULL)
        return cmd_line;

    /* Used because of a potential chroot */
    config_file->orig_log_dir = SnortStrdup(config_file->log_dir);

    config_file->run_mode = cmd_line->run_mode;
    config_file->run_mode_flags |= cmd_line->run_mode_flags;

    if ((cmd_line->run_mode == RUN_MODE__TEST) &&
        (config_file->run_flags & RUN_FLAG__DAEMON))
    {
        /* Just ignore deamon setting in conf file */
        config_file->run_flags &= ~RUN_FLAG__DAEMON;
    }

    config_file->run_flags |= cmd_line->run_flags;
    config_file->output_flags |= cmd_line->output_flags;
    config_file->logging_flags |= cmd_line->logging_flags;

    config_file->internal_log_level = cmd_line->internal_log_level;
    config_file->suppress_config_log = cmd_line->suppress_config_log;

    /* Merge checksum flags.  If command line modified them, use from the
     * command line, else just use from config_file. */
    for (i = 0; i < config_file->num_policies_allocated; i++)
    {
        if (config_file->targeted_policies[i] != NULL)
        {
            if (cmd_line->checksum_flags_modified)
                config_file->targeted_policies[i]->checksum_flags = cmd_line->checksum_flags;

            if (cmd_line->checksum_drop_flags_modified)
                config_file->targeted_policies[i]->checksum_drop_flags = cmd_line->checksum_drop_flags;
        }
    }

    config_file->event_log_id = cmd_line->event_log_id;

    if (cmd_line->dynamic_rules_path != NULL)
    {
        if(strcmp(cmd_line->dynamic_rules_path, "")  != 0)
        {
            if( config_file->dynamic_rules_path != NULL )
                free(config_file->dynamic_rules_path);
            config_file->dynamic_rules_path = SnortStrdup(cmd_line->dynamic_rules_path);
        }
    }

    if (cmd_line->dyn_engines != NULL)
    {
        FreeDynamicLibInfo(config_file->dyn_engines);
        config_file->dyn_engines = DupDynamicLibInfo(cmd_line->dyn_engines);
    }

    if (cmd_line->dyn_rules != NULL)
    {
        FreeDynamicLibInfo(config_file->dyn_rules);
        config_file->dyn_rules = DupDynamicLibInfo(cmd_line->dyn_rules);
    }

    if (cmd_line->dyn_preprocs != NULL)
    {
        FreeDynamicLibInfo(config_file->dyn_preprocs);
        config_file->dyn_preprocs = DupDynamicLibInfo(cmd_line->dyn_preprocs);
    }

    if (cmd_line->pid_path[0] != '\0')
        ConfigPidPath(config_file, cmd_line->pid_path);

    config_file->exit_check = cmd_line->exit_check;

    /* Command line only configures search method */
    if (cmd_line->fast_pattern_config != NULL)
        config_file->fast_pattern_config->search_method = cmd_line->fast_pattern_config->search_method;

    if (cmd_line->obfuscation_net.family != 0)
        memcpy(&config_file->obfuscation_net, &cmd_line->obfuscation_net, sizeof(sfip_t));

    if (cmd_line->homenet.family != 0)
        memcpy(&config_file->homenet, &cmd_line->homenet, sizeof(sfip_t));

    if (cmd_line->interface != NULL)
    {
        if (config_file->interface != NULL)
            free(config_file->interface);
        config_file->interface = SnortStrdup(cmd_line->interface);
    }

    if (cmd_line->bpf_file != NULL)
    {
        if (config_file->bpf_file != NULL)
            free(config_file->bpf_file);
        config_file->bpf_file = SnortStrdup(cmd_line->bpf_file);
    }

    if (cmd_line->bpf_filter != NULL)
        config_file->bpf_filter = SnortStrdup(cmd_line->bpf_filter);

    if (cmd_line->pkt_snaplen != -1)
        config_file->pkt_snaplen = cmd_line->pkt_snaplen;

    if (cmd_line->pkt_cnt != 0)
        config_file->pkt_cnt = cmd_line->pkt_cnt;

#ifdef REG_TEST
    if (cmd_line->pkt_skip != 0)
        config_file->pkt_skip = cmd_line->pkt_skip;
#endif

    if (cmd_line->group_id != -1)
        config_file->group_id = cmd_line->group_id;

    if (cmd_line->user_id != -1)
        config_file->user_id = cmd_line->user_id;

    /* Only configurable on command line */
    if (cmd_line->pcap_log_file != NULL)
        config_file->pcap_log_file = SnortStrdup(cmd_line->pcap_log_file);

    if (cmd_line->file_mask != 0)
        config_file->file_mask = cmd_line->file_mask;

    if (cmd_line->pidfile_suffix[0] != '\0')
    {
        SnortStrncpy(config_file->pidfile_suffix, cmd_line->pidfile_suffix,
                     sizeof(config_file->pidfile_suffix));
    }

    if (cmd_line->chroot_dir != NULL)
    {
        if (config_file->chroot_dir != NULL)
            free(config_file->chroot_dir);
        config_file->chroot_dir = SnortStrdup(cmd_line->chroot_dir);
    }

    if (cmd_line->perf_file != NULL)
    {
        if (config_file->perf_file != NULL)
            free(config_file->perf_file);
        config_file->perf_file = SnortStrdup(cmd_line->perf_file);
    }

    if ( cmd_line->daq_type )
        config_file->daq_type = SnortStrdup(cmd_line->daq_type);

    if ( cmd_line->daq_mode )
        config_file->daq_mode = SnortStrdup(cmd_line->daq_mode);

    if ( cmd_line->dirty_pig )
        config_file->dirty_pig = cmd_line->dirty_pig;

    if ( cmd_line->daq_vars )
    {
        /* Command line overwrites daq_vars */
        if (config_file->daq_vars)
            StringVector_Delete(config_file->daq_vars);

        config_file->daq_vars = StringVector_New();
        StringVector_AddVector(config_file->daq_vars, cmd_line->daq_vars);
    }
    if ( cmd_line->daq_dirs )
    {
        /* Command line overwrites daq_dirs */
        if (config_file->daq_dirs)
            StringVector_Delete(config_file->daq_dirs);

        config_file->daq_dirs = StringVector_New();
        StringVector_AddVector(config_file->daq_dirs, cmd_line->daq_dirs);
    }
#ifdef MPLS
    if (cmd_line->mpls_stack_depth != DEFAULT_LABELCHAIN_LENGTH)
        config_file->mpls_stack_depth = cmd_line->mpls_stack_depth;

    /* Set MPLS payload type here if it hasn't been defined */
    if ((cmd_line->mpls_payload_type == 0) &&
        (config_file->mpls_payload_type == 0))
    {
        config_file->mpls_payload_type = DEFAULT_MPLS_PAYLOADTYPE;
    }
    else if (cmd_line->mpls_payload_type != 0)
    {
        config_file->mpls_payload_type = cmd_line->mpls_payload_type;
    }
#endif

    if (cmd_line->run_flags & RUN_FLAG__PROCESS_ALL_EVENTS)
        config_file->event_queue_config->process_all_events = 1;

    if (cmd_line->cs_dir != NULL)
    {
        if (config_file->cs_dir != NULL)
            free(config_file->cs_dir);
        config_file->cs_dir = SnortStrdup(cmd_line->cs_dir);
    }
    if (config_file->cs_dir)
    {

#ifndef WIN32
        /*
         *  If an absolute path is specified, then use that.
         *  otherwise, relative to pid path
         */
        if ((config_file->cs_dir[0] != '/') && config_file->pid_path[0])
        {

            char fullpath[PATH_MAX];

            if (config_file->pid_path[strlen(config_file->pid_path) - 1] == '/')
            {
                SnortSnprintf(fullpath, sizeof(fullpath),
                        "%s%s", config_file->pid_path, config_file->cs_dir);
            }
            else
            {
                SnortSnprintf(fullpath, sizeof(fullpath),
                        "%s/%s", config_file->pid_path, config_file->cs_dir);
            }
            free (config_file->cs_dir);
            config_file->cs_dir = SnortStrdup(fullpath);

        }
#else
        /*Not supported in WINDOWS*/
        free (config_file->cs_dir);
        config_file->cs_dir = NULL;
#endif
        ControlSocketConfigureDirectory(config_file->cs_dir);
    }

#ifdef REG_TEST
    config_file->ha_peer = cmd_line->ha_peer;

    if ( cmd_line->ha_out )
    {
        if(config_file->ha_out != NULL)
            free(config_file->ha_out);
        config_file->ha_out = strdup(cmd_line->ha_out);
    }

    if ( cmd_line->ha_in )
    {
        if(config_file->ha_in != NULL)
            free(config_file->ha_in);
        config_file->ha_in = strdup(cmd_line->ha_in);
    }
#endif

    return config_file;
}

static void FreeDynamicLibInfos(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    if (sc->dyn_engines != NULL)
    {
        FreeDynamicLibInfo(sc->dyn_engines);
        sc->dyn_engines = NULL;
    }

    if (sc->dyn_rules != NULL)
    {
        FreeDynamicLibInfo(sc->dyn_rules);
        sc->dyn_rules = NULL;
    }

    if (sc->dyn_preprocs != NULL)
    {
        FreeDynamicLibInfo(sc->dyn_preprocs);
        sc->dyn_preprocs = NULL;
    }

#ifdef SIDE_CHANNEL
    if (sc->dyn_side_channels != NULL)
    {
        FreeDynamicLibInfo(sc->dyn_side_channels);
        sc->dyn_side_channels = NULL;
    }
#endif
}

static void FreeDynamicLibInfo(DynamicLibInfo *lib_info)
{
    unsigned i;

    if (lib_info == NULL)
        return;

    for (i = 0; i < lib_info->count; i++)
    {
        free(lib_info->lib_paths[i]->path);
        free(lib_info->lib_paths[i]);
    }

    free(lib_info);
}

static DynamicLibInfo * DupDynamicLibInfo(DynamicLibInfo *src)
{
    DynamicLibInfo *dst;
    unsigned i;

    if (src == NULL)
        return NULL;

    dst = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
    dst->type = src->type;
    dst->count = src->count;

    for (i = 0; i < src->count; i++)
    {
        DynamicLibPath *dylib_path = (DynamicLibPath *)SnortAlloc(sizeof(DynamicLibPath));

        dylib_path->ptype = src->lib_paths[i]->ptype;
        dylib_path->path = SnortStrdup(src->lib_paths[i]->path);

        dst->lib_paths[i] = dylib_path;
    }

    return dst;
}

void FreeVarList(VarNode *head)
{
    while (head != NULL)
    {
        VarNode *tmp = head;

        head = head->next;

        if (tmp->name != NULL)
            free(tmp->name);

        if (tmp->value != NULL)
            free(tmp->value);

        if (tmp->line != NULL)
            free(tmp->line);

        free(tmp);
    }
}

void SnortInit(int argc, char **argv)
{
    InitSignals();

#if defined(NOCOREFILE) && !defined(WIN32)
    SetNoCores();
#else
    StoreSnortInfoStrings();
#endif

#ifdef WIN32
    if (!init_winsock()) // TBD moves to windows daq
        FatalError("Could not Initialize Winsock!\n");
#endif

    InitGlobals();

    /* chew up the command line */
    ParseCmdLine(argc, argv);

    switch (snort_conf->run_mode)
    {
        case RUN_MODE__VERSION:
            break;

        case RUN_MODE__RULE_DUMP:
            LogMessage("Running in Rule Dump mode\n");
            break;

        case RUN_MODE__IDS:
            LogMessage("Running in IDS mode\n");
            break;

        case RUN_MODE__TEST:
            LogMessage("Running in Test mode\n");
            break;

        case RUN_MODE__PACKET_LOG:
            LogMessage("Running in packet logging mode\n");
            break;

        case RUN_MODE__PACKET_DUMP:
            LogMessage("Running in packet dump mode\n");
            break;

        default:
            break;
    }

    if (ScSuppressConfigLog() || ScVersionMode())
        ScSetInternalLogLevel(INTERNAL_LOG_LEVEL__ERROR);

    LogMessage("\n");
    LogMessage("        --== Initializing Snort ==--\n");

    if (SnortStrnlen(signal_error_msg, STD_BUF)> 0)
    {
        ErrorMessage("%s", signal_error_msg);
    }

    if (!ScVersionMode())
    {
        /* Every run mode except version will potentially need output
         * If output plugins should become dynamic, this needs to move */
        RegisterOutputPlugins();
#ifdef DEBUG
        DumpOutputPlugins();
#endif
    }

    init_fileAPI();

    /* if we're using the rules system, it gets initialized here */
    if (snort_conf_file != NULL)
    {
        SnortConfig *sc;

        /* initialize all the plugin modules */
        RegisterPreprocessors();
        RegisterRuleOptions();
        InitTag();

#ifdef DEBUG
        DumpPreprocessors();
        DumpRuleOptions();
#endif

#ifdef PERF_PROFILING
        /* Register the main high level perf stats */
        RegisterPreprocessorProfile("detect", &detectPerfStats, 0, &totalPerfStats);
        RegisterPreprocessorProfile("mpse", &mpsePerfStats, 1, &detectPerfStats);
        RegisterPreprocessorProfile("rule eval", &rulePerfStats, 1, &detectPerfStats);
        RegisterPreprocessorProfile("rtn eval", &ruleRTNEvalPerfStats, 2, &rulePerfStats);
        RegisterPreprocessorProfile("rule tree eval", &ruleOTNEvalPerfStats, 2, &rulePerfStats);
        RegisterPreprocessorProfile("preproc_rule_options", &preprocRuleOptionPerfStats, 3, &ruleOTNEvalPerfStats);
        RegisterPreprocessorProfile("decode", &decodePerfStats, 0, &totalPerfStats);
        RegisterPreprocessorProfile("eventq", &eventqPerfStats, 0, &totalPerfStats);
        RegisterPreprocessorProfile("total", &totalPerfStats, 0, NULL);
        RegisterPreprocessorProfile("daq meta", &metaPerfStats, 0, NULL);
#endif

        LogMessage("Parsing Rules file \"%s\"\n", snort_conf_file);
        sc = ParseSnortConf();

        /* Merge the command line and config file confs to take care of
         * command line overriding config file.
         * Set the global snort_conf that will be used during run time */
        snort_conf = MergeSnortConfs(snort_cmd_line_conf, sc);

        InitSynToMulticastDstIp(snort_conf);

#ifdef TARGET_BASED
        /* Parse attribute table stuff here since config max_attribute_hosts
         * is apart from attribute table configuration.
         * Only attribute table in default policy is processed. Attribute table in
         * other policies indicates that attribute table in default table should
         * be used. Filenames for attribute_table should be same across all policies.
         */
        {
            tSfPolicyId defaultPolicyId = sfGetDefaultPolicy(snort_conf->policy_config);
            TargetBasedConfig *tbc = &snort_conf->targeted_policies[defaultPolicyId]->target_based_config;

            if (tbc->args != NULL)
            {
                char *saved_file_name = file_name;
                int saved_file_line = file_line;

                file_name = tbc->file_name;
                file_line = tbc->file_line;

                SFAT_ParseAttributeTable(tbc->args);

                file_name = saved_file_name;
                file_line = saved_file_line;
            }
        }
#endif

        if (snort_conf->asn1_mem != 0)
            asn1_init_mem(snort_conf->asn1_mem);
        else
            asn1_init_mem(256);

        if (snort_conf->alert_file != NULL)
        {
            char *tmp = snort_conf->alert_file;
            snort_conf->alert_file = ProcessFileOption(snort_conf, snort_conf->alert_file);
            free(tmp);
        }

#ifdef PERF_PROFILING
        /* Parse profiling here because of file option and potential
         * dependence on log directory */
        {
            char *opts = NULL;
            int in_table;

            in_table = sfghash_find2(snort_conf->config_table,
                                     CONFIG_OPT__PROFILE_PREPROCS, (void *)&opts);
            if (in_table)
                ConfigProfilePreprocs(snort_conf, opts);

            in_table = sfghash_find2(snort_conf->config_table,
                                     CONFIG_OPT__PROFILE_RULES, (void *)&opts);
            if (in_table)
                ConfigProfileRules(snort_conf, opts);
        }
#endif

        if (ScAlertBeforePass())
        {
            OrderRuleLists(snort_conf, "activation dynamic drop sdrop reject alert pass log");
        }

        LogMessage("Tagged Packet Limit: %ld\n", snort_conf->tagged_packet_limit);


        /* Handles Fatal Errors itself. */
        SnortEventqNew(snort_conf->event_queue_config, snort_conf->event_queue);
    }
    else if (ScPacketLogMode() || ScPacketDumpMode())
    {
        /* Make sure there is a log directory */
        /* This will return the cmd line conf and resolve the output
         * configuration */
        SnortConfig* sc = ParseSnortConf();
        snort_conf = MergeSnortConfs(snort_cmd_line_conf, sc);
        InitTag();
        SnortEventqNew(snort_conf->event_queue_config, snort_conf->event_queue);
    }


    /* Allocate an array for IP6 extensions for the main Packet struct */
    // Make sure this memory is freed on exit.
    s_packet.ip6_extensions = SnortAlloc(sizeof(*s_packet.ip6_extensions) * ScMaxIP6Extensions());
    
    /* Finish up the pcap list and put in the queues */
    PQ_SetUp();

    if ((snort_conf->bpf_filter == NULL) && (snort_conf->bpf_file != NULL))
    {
        LogMessage("Reading filter from bpf file: %s\n", snort_conf->bpf_file);
        snort_conf->bpf_filter = read_infile(snort_conf->bpf_file);
    }

    if (snort_conf->bpf_filter != NULL)
        LogMessage("Snort BPF option: %s\n", snort_conf->bpf_filter);

    LoadDynamicPlugins(snort_conf);

    /* Display snort version information here so that we can also show dynamic
     * plugin versions, if loaded.  */
    if (ScVersionMode())
    {
        ScRestoreInternalLogLevel();
        PrintVersion();
        CleanExit(0);
    }

    /* Validate the log directory for logging packets - probably should
     * add test mode as well, but not expected behavior */
    if ( !(ScNoLog() && ScNoAlert()) )
    {
        if (ScPacketLogMode())
            CheckLogDir();

        LogMessage("Log directory = %s\n", snort_conf->log_dir);
    }

    if (ScOutputUseUtc())
        snort_conf->thiszone = 0;
    else
        snort_conf->thiszone = gmt2local(0);  /* ripped from tcpdump */

    ConfigureOutputPlugins(snort_conf);

    /* Have to split up configuring preprocessors between internal and dynamic
     * because the dpd structure has a pointer to the stream api and stream5
     * needs to be configured first to set this */
    ConfigurePreprocessors(snort_conf, 0);

    InitDynamicEngines(snort_conf->dynamic_rules_path);

    if (ScRuleDumpMode())
    {
        if( snort_conf->dynamic_rules_path == NULL )
        {
            FatalError("%s(%d) Please specify the directory path for dumping the dynamic rules \n",
                                       __FILE__, __LINE__);
        }

        DumpDetectionLibRules();
        CleanExit(0);
    }

    /* This will load each dynamic preprocessor module specified and set
     * the _dpd structure for each */
    InitDynamicPreprocessors();

    /* Now configure the dynamic preprocessors since the dpd structure
     * should be filled in and have the correct values */
    ConfigurePreprocessors(snort_conf, 1);

    ParseRules(snort_conf);
    RuleOptParseCleanup();

    InitDynamicDetectionPlugins(snort_conf);

    EventTrace_Init();

    if (ScIdsMode() || ScTestMode())
    {
        detection_filter_print_config(snort_conf->detection_filter_config);
        RateFilter_PrintConfig(snort_conf->rate_filter_config);
        print_thresholding(snort_conf->threshold_config, 0);
        PrintRuleOrder(snort_conf->rule_lists);

        /* Check rule state lists, enable/disabled
         * and err on 'special' GID without OTN.
         */
        /*
         * Modified toi use sigInfo.shared in otn instead of the GENERATOR ID  - man
         */
        SetRuleStates(snort_conf);

        /* Verify the preprocessors are configured properly */
        if (CheckPreprocessorsConfig(snort_conf))
        {
            SnortFatalExit();
        }

        /* Remove disabled preprocessors if policies are disabled  */
        FilterConfigPreprocessors(snort_conf);

        /* Need to do this after dynamic detection stuff is initialized, too */
        FlowBitsVerify();
    }

    if (snort_conf->file_mask != 0)
        umask(snort_conf->file_mask);
    else
        umask(077);    /* set default to be sane */

    // the following was moved from unpriv init; hopefully it can live here.
    decoderActionQ = sfActionQueueInit(snort_conf->event_queue_config->max_events*2);
    if (mempool_init(&decoderAlertMemPool,
                snort_conf->event_queue_config->max_events*2, sizeof(EventNode)) != 0)
    {
        FatalError("%s(%d) Could not initialize decoder action queue memory pool.\n",
                __FILE__, __LINE__);
    }

    fpCreateFastPacketDetection(snort_conf);

#ifdef INTEL_SOFT_CPM
    if (snort_conf->fast_pattern_config->search_method == MPSE_INTEL_CPM)
        IntelPmActivate(snort_conf);
#endif

#ifdef PPM_MGR
    PPM_PRINT_CFG(&snort_conf->ppm_cfg);
#endif

#ifdef SIDE_CHANNEL
    RegisterSideChannelModules();
    InitDynamicSideChannelPlugins();
    ConfigureSideChannelModules(snort_conf);
    SideChannelConfigure(snort_conf);
    SideChannelInit();
#endif

    // If we suppressed output at the beginning of SnortInit(),
    // then restore it now.
    ScRestoreInternalLogLevel();
}

#if defined(INLINE_FAILOPEN) && !defined(WIN32)
static void * SnortPostInitThread(void *data)
{
    sigset_t mtmask;

    inline_failopen_thread_pid = gettid();
    inline_failopen_thread_running = 1;

    /* Don't handle any signals here */
    sigfillset(&mtmask);
    pthread_sigmask(SIG_BLOCK, &mtmask, NULL);

    while (!inline_failopen_initialized)
        nanosleep(&thread_sleep, NULL);

    SnortUnprivilegedInit();

    pthread_exit((void *)NULL);
}

static DAQ_Verdict IgnoreCallback (
    void *user, const DAQ_PktHdr_t* pkthdr, const uint8_t* pkt)
{
    /* Empty function -- do nothing with the packet we just read */
    inline_failopen_pass_pkt_cnt++;

#ifdef DEBUG
    {
        FILE *tmp = fopen("/var/tmp/fo_threadid", "a");
        if ( tmp )
        {
            fprintf(tmp, "Packet Count %d\n", inline_failopen_pass_pkt_cnt);
            fclose(tmp);
        }
    }
#endif
    return DAQ_VERDICT_PASS;
}
#endif /* defined(INLINE_FAILOPEN) && !defined(WIN32) */

// this function should only include initialization that must be done as a
// non-root user such as creating log files.  other initialization stuff should
// be in the main initialization function since, depending on platform and
// configuration, this may be running in a background thread while passing
// packets in a fail open mode in the main thread.  we don't want big delays
// here to cause excess latency or dropped packets in that thread which may
// be the case if all threads are pinned to a single cpu/core.
//
// clarification: once snort opens/starts the DAQ, packets are queued for snort
// and must be disposed of quickly or the queue will overflow and packets will
// be dropped so the fail open thread does the remaining initialization while
// the main thread passes packets.  prior to opening and starting the DAQ,
// packet passing is done by the driver/hardware.  the goal then is to put as
// much initialization stuff in SnortInit() as possible and to restrict this
// function to those things that depend on DAQ startup or non-root user/group.
static void SnortUnprivilegedInit(void)
{
#ifdef ACTIVE_RESPONSE
    // this depends on instantiated daq capabilities
    // so it is done here instead of SnortInit()
    Active_Init(snort_conf);
#endif

    InitPidChrootAndPrivs(snort_main_thread_pid);

#if defined(HAVE_LINUXTHREADS) && !defined(WIN32)
    // this must be done after dropping privs for linux threads
    // to ensure that child threads can communicate with parent
    SnortStartThreads();
#endif

    // perfmon, for one, opens a log file for writing here
    PostConfigPreprocessors(snort_conf);

    // log_tcpdump opens a log file for writing here; also ...
    // note that things like opening log_tcpdump will fail here if the
    // user specified -u (we dropped privileges) and the log defaults
    // to /var/log/snort.  in this case they must override log path.
    PostConfigInitPlugins(snort_conf, snort_conf->plugin_post_config_funcs);

    FileAPIPostInit();

#ifdef SIDE_CHANNEL
    SideChannelPostInit();
#endif

    LogMessage("\n");
    LogMessage("        --== Initialization Complete ==--\n");

    /* Tell 'em who wrote it, and what "it" is */
    PrintVersion();

    if (ScTestMode())
    {
        LogMessage("\n");
        LogMessage("Snort successfully validated the configuration!\n");
        CleanExit(0);
    }

    LogMessage("Commencing packet processing (pid=%u)\n", snort_main_thread_pid);

    snort_initializing = false;
}

#if defined(NOCOREFILE) && !defined(WIN32)
static void SetNoCores(void)
{
    struct rlimit rlim;

    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}
#endif

/* Add a signal handler
 *
 * If check needed, also check whether previous signal_handler is neither SIG_IGN nor SIG_DFL
 *
 * Return:
 *     0: error
 *     1: success
 */
int SnortAddSignal(int sig, sighandler_t signal_handler, int check_needed)
{
    sighandler_t pre_handler;

#ifdef HAVE_SIGACTION
    struct sigaction action;
    struct sigaction old_action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = signal_handler;
    sigaction(sig, &action, &old_action);
    pre_handler = old_action.sa_handler;
#else
    pre_handler = signal(sig, signal_handler);
#endif
    if (SIG_ERR == pre_handler)
    {
        SnortSnprintfAppend(signal_error_msg, STD_BUF,
                "Could not add handler for signal %d \n", sig);
        return 0;
    }
    else if (check_needed && (SIG_IGN != pre_handler) && (SIG_DFL!= pre_handler))
    {
        SnortSnprintfAppend(signal_error_msg, STD_BUF,
                "WARNING: Handler is already installed for signal %d.\n", sig);
    }
    return 1;
}
static void InitSignals(void)
{

#ifndef WIN32
# if defined(LINUX) || defined(FREEBSD) || defined(OPENBSD) || \
     defined(SOLARIS) || defined(BSD) || defined(MACOS)
    sigset_t set;

    sigemptyset(&set);
#  if defined(INLINE_FAILOPEN) || \
      defined(TARGET_BASED) || defined(SNORT_RELOAD)
    pthread_sigmask(SIG_SETMASK, &set, NULL);
#  else
    sigprocmask(SIG_SETMASK, &set, NULL);
#  endif /* INLINE_FAILOPEN */
# else
    sigsetmask(0);
# endif /* LINUX, BSD, SOLARIS */
#endif  /* !WIN32 */

    /* Make this prog behave nicely when signals come along.
     * Windows doesn't like all of these signals, and will
     * set errno for some.  Ignore/reset this error so it
     * doesn't interfere with later checks of errno value.  */
    signal_error_msg[0] = '\0';
    SnortAddSignal(SIGTERM, SigExitHandler, 1);
    SnortAddSignal(SIGINT, SigExitHandler, 1);
#ifndef WIN32
    SnortAddSignal(SIGQUIT, SigExitHandler, 1);
    SnortAddSignal(SIGNAL_SNORT_DUMP_STATS, SigDumpStatsHandler, 1);
    SnortAddSignal(SIGNAL_SNORT_RELOAD, SigReloadHandler, 1);
    SnortAddSignal(SIGNAL_SNORT_ROTATE_STATS, SigRotateStatsHandler, 1);
#endif

#ifdef CONTROL_SOCKET
    SnortAddSignal(SIGPIPE, SigPipeHandler, 1);
#endif

#ifdef TARGET_BASED
#ifndef WIN32
    /* Used to print warning if attribute table is not configured
     * When it is, it will set new signal handler */
    SnortAddSignal(SIGNAL_SNORT_READ_ATTR_TBL, SigNoAttributeTableHandler, 1);
#endif
#endif

    SnortAddSignal(SIGABRT, SigOopsHandler, 1);
    SnortAddSignal(SIGSEGV, SigOopsHandler, 1);
#ifndef WIN32
    SnortAddSignal(SIGBUS, SigOopsHandler, 1);
#endif

    errno = 0;
}

static void FreeOutputConfigs(OutputConfig *head)
{
    while (head != NULL)
    {
        OutputConfig *tmp = head;

        head = head->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        if (tmp->opts != NULL)
            free(tmp->opts);

        if (tmp->file_name != NULL)
            free(tmp->file_name);

        /* Don't free listhead as it's just a pointer to the user defined
         * rule's rule list node's list head */

        free(tmp);
    }
}

#ifdef SIDE_CHANNEL
static void FreeSideChannelModuleConfigs(SideChannelModuleConfig *head)
{
    while (head != NULL)
    {
        SideChannelModuleConfig *tmp = head;

        head = head->next;

        if (tmp->keyword != NULL)
            free(tmp->keyword);

        if (tmp->opts != NULL)
            free(tmp->opts);

        if (tmp->file_name != NULL)
            free(tmp->file_name);

        free(tmp);
    }
}
#endif

static void FreePreprocConfigs(SnortConfig *sc)
{
    tSfPolicyId i;

    if (sc == NULL)
        return;

    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        SnortPolicy *p = sc->targeted_policies[i];
        PreprocConfig *head;

        if (p == NULL)
            continue;

        head = p->preproc_configs;

        while (head != NULL)
        {
            PreprocConfig *tmp = head;

            head = head->next;

            if (tmp->keyword != NULL)
                free(tmp->keyword);

            if (tmp->opts != NULL)
                free(tmp->opts);

            if (tmp->file_name != NULL)
                free(tmp->file_name);

            free(tmp);
        }
    }
}

static void FreeRuleStateList(RuleState *head)
{
    while (head != NULL)
    {
        RuleState *tmp = head;

        head = head->next;

        free(tmp);
    }
}

static void FreeClassifications(ClassType *head)
{
    while (head != NULL)
    {
        ClassType *tmp = head;

        head = head->next;

        if (tmp->name != NULL)
            free(tmp->name);

        if (tmp->type != NULL)
            free(tmp->type);

        free(tmp);
    }
}

static void FreeReferences(ReferenceSystemNode *head)
{
    while (head != NULL)
    {
        ReferenceSystemNode *tmp = head;

        head = head->next;

        if (tmp->name != NULL)
            free(tmp->name);

        if (tmp->url != NULL)
            free(tmp->url);

        free(tmp);
    }
}

#if defined(SNORT_RELOAD) && !defined(WIN32)
static void updatePeriodicCheck()
{
    PeriodicCheckFuncNode *checkFunc;

    /* reset preprocessors */
    checkFunc = periodic_check_funcs;
    while (checkFunc != NULL)
    {
        if ( 0 == checkFunc->time_left )
        {
            checkFunc->func(-1, checkFunc->arg);
            checkFunc->time_left = checkFunc->period;
            //LogMessage("        --== Share Memory! ==--\n");
        }
        else
            checkFunc->time_left--;

        checkFunc = checkFunc->next;
    }

}

static void * ReloadConfigThread(void *data)
{
    sigset_t mtmask;

    /* Don't handle any signals here */
    sigfillset(&mtmask);
    pthread_sigmask(SIG_BLOCK, &mtmask, NULL);

    snort_reload_thread_pid = gettid();
    snort_reload_thread_created = 1;

    while (snort_initializing)
        nanosleep(&thread_sleep, NULL);

    while (!snort_exiting)
    {
        if (reload_signal != reload_total)
        {
            int reload_failed = 0;

#ifdef CONTROL_SOCKET
            pthread_mutex_lock(&reload_mutex);
            if (!reloadInProgress)
            {
                reloadInProgress = 1;
                pthread_mutex_unlock(&reload_mutex);
#endif
                reload_total++;

                LogMessage("\n");
                LogMessage("        --== Reloading Snort ==--\n");
                LogMessage("\n");

                if (ScSuppressConfigLog())
                    ScSetInternalLogLevel(INTERNAL_LOG_LEVEL__ERROR);

                snort_conf_new = ReloadConfig();

                // Restore Log level if we suppressed it earlier
                ScRestoreInternalLogLevel();

                if (snort_conf_new == NULL)
                    reload_failed = 1;
                snort_reload = 1;

                while (!snort_swapped && !snort_exiting)
                    nanosleep(&thread_sleep, NULL);

                snort_swapped = 0;

                SnortConfFree(snort_conf_old);
                snort_conf_old = NULL;

#ifdef INTEL_SOFT_CPM
                if (snort_conf->fast_pattern_config->search_method != MPSE_INTEL_CPM)
                    IntelPmStopInstance();
#endif

                if (snort_exiting && !reload_failed)
                {
                    /* This will load the new preprocessor configurations and
                     * free the old ones, so any preprocessor cleanup that
                     * requires a configuration will be using the new one
                     * unless it relies on old configurations that are still
                     * attached to existing sessions. */
                    SwapPreprocConfigurations(snort_conf);
                    FreeSwappedPreprocConfigurations(snort_conf);

#ifdef CONTROL_SOCKET
                    reloadInProgress = 0;
#endif
                    /* Get out of the loop and exit */
                    break;
                }

                if (!reload_failed)
                {
                    LogMessage("\n");
                    LogMessage("        --== Reload Complete ==--\n");
                    LogMessage("\n");
                }
#ifdef CONTROL_SOCKET
                reloadInProgress = 0;
            }
            else
                pthread_mutex_unlock(&reload_mutex);
#endif
        }
        /* Use the maintenance thread for periodic check*/
        updatePeriodicCheck();

        sleep(1);
    }

    pthread_exit((void *)0);
}

static SnortConfig * ReloadConfig(void)
{
    SnortConfig *sc;

#ifdef HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif

    sc = ParseSnortConf();

    sc = MergeSnortConfs(snort_cmd_line_conf, sc);

    sc->reloadPolicyFlag = 1;

#ifdef PERF_PROFILING
    /* Parse profiling here because of file option and potential
     * dependence on log directory */
    {
        char *opts = NULL;
        int in_table;

        in_table = sfghash_find2(sc->config_table,
                                 CONFIG_OPT__PROFILE_PREPROCS, (void *)&opts);
        if (in_table)
            ConfigProfilePreprocs(sc, opts);

        in_table = sfghash_find2(sc->config_table,
                                 CONFIG_OPT__PROFILE_RULES, (void *)&opts);
        if (in_table)
            ConfigProfileRules(sc, opts);
    }
#endif

    if (VerifyReload(sc) == -1)
    {
        SnortConfFree(sc);
        return NULL;
    }

    if (sc->output_flags & OUTPUT_FLAG__USE_UTC)
        sc->thiszone = 0;
    else
        sc->thiszone = gmt2local(0);

    /* Preprocessors will have a reload callback */
    ConfigurePreprocessors(sc, 1);

    FlowbitResetCounts();
    ParseRules(sc);
    RuleOptParseCleanup();

    ReloadDynamicRules(sc);

    /* Handles Fatal Errors itself. */
    SnortEventqNew(sc->event_queue_config, sc->event_queue);

    detection_filter_print_config(sc->detection_filter_config);
    RateFilter_PrintConfig(sc->rate_filter_config);
    print_thresholding(sc->threshold_config, 0);
    PrintRuleOrder(sc->rule_lists);

    SetRuleStates(sc);

    if (file_sevice_config_verify(snort_conf, sc) == -1)
    {
        SnortConfFree(sc);
        return NULL;
    }

    if (VerifyReloadedPreprocessors(sc))
    {
        SnortConfFree(sc);
        return NULL;
    }

    if (CheckPreprocessorsConfig(sc))
    {
        SnortConfFree(sc);
        return NULL;
    }

    FilterConfigPreprocessors(sc);
    PostConfigPreprocessors(sc);

    /* Need to do this after dynamic detection stuff is initialized, too */
    FlowBitsVerify();

    if ((sc->file_mask != 0) && (sc->file_mask != snort_conf->file_mask))
        umask(sc->file_mask);

    /* Transfer any user defined rule type outputs to the new rule list */
    {
        RuleListNode *cur = snort_conf->rule_lists;

        for (; cur != NULL; cur = cur->next)
        {
            RuleListNode *new = sc->rule_lists;

            for (; new != NULL; new = new->next)
            {
                if (strcasecmp(cur->name, new->name) == 0)
                {
                    OutputFuncNode *alert_list = cur->RuleList->AlertList;
                    OutputFuncNode *log_list = cur->RuleList->LogList;

                    sc->head_tmp = new->RuleList;

                    for (; alert_list != NULL; alert_list = alert_list->next)
                    {
                        AddFuncToOutputList(sc, alert_list->func,
                                            OUTPUT_TYPE__ALERT, alert_list->arg);
                    }

                    for (; log_list != NULL; log_list = log_list->next)
                    {
                        AddFuncToOutputList(sc, log_list->func,
                                            OUTPUT_TYPE__LOG, log_list->arg);
                    }

                    sc->head_tmp = NULL;
                    break;
                }
            }
        }
    }

    /* XXX XXX Can't do any output plugins */
    //PostConfigInitPlugins(sc->plugin_post_config_funcs);

    fpCreateFastPacketDetection(sc);

#ifdef PPM_MGR
    PPM_PRINT_CFG(&sc->ppm_cfg);
#endif

    return sc;
}

static int VerifyReload(SnortConfig *sc)
{
    if (sc == NULL)
        return -1;

#ifdef TARGET_BASED
    {
        SnortPolicy *p1 = sc->targeted_policies[getDefaultPolicy()];
        SnortPolicy *p2 = snort_conf->targeted_policies[getDefaultPolicy()];

        if ((p1->target_based_config.args != NULL) &&
            (p2->target_based_config.args != NULL))
        {
            if (strcasecmp(p1->target_based_config.args,
                           p2->target_based_config.args) != 0)
            {
                ErrorMessage("Snort Reload: Changing the attribute table "
                             "configuration requires a restart.\n");
                return -1;
            }
        }
        else if (p1->target_based_config.args !=
                 p2->target_based_config.args)
        {
            /* Covers one being NULL and not the other */
            ErrorMessage("Snort Reload: Changing the attribute table "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
#endif

    if ((snort_conf->alert_file != NULL) && (sc->alert_file != NULL))
    {
        if (strcasecmp(snort_conf->alert_file, sc->alert_file) != 0)
        {
            ErrorMessage("Snort Reload: Changing the alert file "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->alert_file != sc->alert_file)
    {
        ErrorMessage("Snort Reload: Changing the alert file "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->asn1_mem != sc->asn1_mem)
    {
        ErrorMessage("Snort Reload: Changing the asn1 memory configuration "
                     "requires a restart.\n");
        return -1;
    }

    if ((sc->bpf_filter == NULL) && (sc->bpf_file != NULL))
        sc->bpf_filter = read_infile(sc->bpf_file);

    if ((sc->bpf_filter != NULL) && (snort_conf->bpf_filter != NULL))
    {
        if (strcasecmp(snort_conf->bpf_filter, sc->bpf_filter) != 0)
        {
            ErrorMessage("Snort Reload: Changing the bpf filter configuration "
                         "requires a restart.\n");
            return -1;
        }
    }
    else if (sc->bpf_filter != snort_conf->bpf_filter)
    {
        ErrorMessage("Snort Reload: Changing the bpf filter configuration "
                     "requires a restart.\n");
        return -1;
    }

#ifdef ACTIVE_RESPONSE
    if ( sc->respond_attempts != snort_conf->respond_attempts ||
         sc->respond_device != snort_conf->respond_device )
    {
        ErrorMessage("Snort Reload: Changing config response "
                     "requires a restart.\n");
        return -1;
    }
#endif

    if ((snort_conf->chroot_dir != NULL) &&
        (sc->chroot_dir != NULL))
    {
        if (strcasecmp(snort_conf->chroot_dir, sc->chroot_dir) != 0)
        {
            ErrorMessage("Snort Reload: Changing the chroot directory "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->chroot_dir != sc->chroot_dir)
    {
        ErrorMessage("Snort Reload: Changing the chroot directory "
                     "configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->run_flags & RUN_FLAG__DAEMON) !=
        (sc->run_flags & RUN_FLAG__DAEMON))
    {
        ErrorMessage("Snort Reload: Changing to or from daemon mode "
                     "requires a restart.\n");
        return -1;
    }

    if ((snort_conf->interface != NULL) && (sc->interface != NULL))
    {
        if (strcasecmp(snort_conf->interface, sc->interface) != 0)
        {
            ErrorMessage("Snort Reload: Changing the interface "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->interface != sc->interface)
    {
        ErrorMessage("Snort Reload: Changing the interface "
                     "configuration requires a restart.\n");
        return -1;
    }

    /* Orig log dir because a chroot might have changed it */
    if ((snort_conf->orig_log_dir != NULL) &&
        (sc->orig_log_dir != NULL))
    {
        if (strcasecmp(snort_conf->orig_log_dir, sc->orig_log_dir) != 0)
        {
            ErrorMessage("Snort Reload: Changing the log directory "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->orig_log_dir != sc->orig_log_dir)
    {
        ErrorMessage("Snort Reload: Changing the log directory "
                     "configuration requires a restart.\n");
        return -1;
    }

#ifdef TARGET_BASED
    if (snort_conf->max_attribute_hosts != sc->max_attribute_hosts)
    {
        ErrorMessage("Snort Reload: Changing max_attribute_hosts "
                     "configuration requires a restart.\n");
        return -1;
    }
    if (snort_conf->max_attribute_services_per_host != sc->max_attribute_services_per_host)
    {
        ErrorMessage("Snort Reload: Changing max_attribute_services_per_host "
                     "configuration requires a restart.\n");
        return -1;
    }
#endif

    if (snort_conf->no_log != sc->no_log)
    {
        ErrorMessage("Snort Reload: Changing from log to no log or vice "
                     "versa requires a restart.\n");
        return -1;
    }

    if ((snort_conf->run_flags & RUN_FLAG__NO_PROMISCUOUS) !=
        (sc->run_flags & RUN_FLAG__NO_PROMISCUOUS))
    {
        ErrorMessage("Snort Reload: Changing to or from promiscuous mode "
                     "requires a restart.\n");
        return -1;
    }

#ifdef PPM_MGR
    /* XXX XXX Not really sure we need to disallow this */
    if (snort_conf->ppm_cfg.rule_log != sc->ppm_cfg.rule_log)
    {
        ErrorMessage("Snort Reload: Changing the ppm rule_log "
                     "configuration requires a restart.\n");
        return -1;
    }
#endif

#ifdef PERF_PROFILING
    if ((snort_conf->profile_rules.num != sc->profile_rules.num) ||
        (snort_conf->profile_rules.sort != sc->profile_rules.sort) ||
        (snort_conf->profile_rules.append != sc->profile_rules.append))
    {
        ErrorMessage("Snort Reload: Changing rule profiling number, sort "
                     "or append configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->profile_rules.filename != NULL) &&
        (sc->profile_rules.filename != NULL))
    {
        if (strcasecmp(snort_conf->profile_rules.filename,
                       sc->profile_rules.filename) != 0)
        {
            ErrorMessage("Snort Reload: Changing the rule profiling filename "
                         "configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->profile_rules.filename !=
             sc->profile_rules.filename)
    {
        ErrorMessage("Snort Reload: Changing the rule profiling filename "
                     "configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->profile_preprocs.num !=  sc->profile_preprocs.num) ||
        (snort_conf->profile_preprocs.sort != sc->profile_preprocs.sort) ||
        (snort_conf->profile_preprocs.append != sc->profile_preprocs.append))
    {
        ErrorMessage("Snort Reload: Changing preprocessor profiling number, "
                     "sort or append configuration requires a restart.\n");
        return -1;
    }

    if ((snort_conf->profile_preprocs.filename != NULL) &&
        (sc->profile_preprocs.filename != NULL))
    {
        if (strcasecmp(snort_conf->profile_preprocs.filename,
                       sc->profile_preprocs.filename) != 0)
        {
            ErrorMessage("Snort Reload: Changing the preprocessor profiling "
                         "filename configuration requires a restart.\n");
            return -1;
        }
    }
    else if (snort_conf->profile_preprocs.filename !=
             sc->profile_preprocs.filename)
    {
        ErrorMessage("Snort Reload: Changing the preprocessor profiling "
                     "filename configuration requires a restart.\n");
        return -1;
    }
#endif

    if (snort_conf->group_id != sc->group_id)
    {
        ErrorMessage("Snort Reload: Changing the group id "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->user_id != sc->user_id)
    {
        ErrorMessage("Snort Reload: Changing the user id "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->pkt_snaplen != sc->pkt_snaplen)
    {
        ErrorMessage("Snort Reload: Changing the packet snaplen "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->threshold_config->memcap !=
        sc->threshold_config->memcap)
    {
        ErrorMessage("Snort Reload: Changing the threshold memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->rate_filter_config->memcap !=
        sc->rate_filter_config->memcap)
    {
        ErrorMessage("Snort Reload: Changing the rate filter memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->detection_filter_config->memcap !=
        sc->detection_filter_config->memcap)
    {
        ErrorMessage("Snort Reload: Changing the detection filter memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (VerifyLibInfos(snort_conf->dyn_engines, sc->dyn_engines) == -1)
    {
        ErrorMessage("Snort Reload: Any change to the dynamic engine "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (VerifyLibInfos(snort_conf->dyn_rules, sc->dyn_rules) == -1)
    {
        ErrorMessage("Snort Reload: Any change to the dynamic detection "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (VerifyLibInfos(snort_conf->dyn_preprocs, sc->dyn_preprocs) == -1)
    {
        ErrorMessage("Snort Reload: Any change to the dynamic preprocessor "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (snort_conf->so_rule_memcap != sc->so_rule_memcap)
    {
        ErrorMessage("Snort Reload: Changing the so rule memcap "
                     "configuration requires a restart.\n");
        return -1;
    }

    if (VerifyOutputs(snort_conf, sc) == -1)
        return -1;

#ifdef SIDE_CHANNEL
    if (SideChannelVerifyConfig(sc) != 0)
    {
        ErrorMessage("Snort Reload: Changing the side channel configuration requires a restart.\n");
        return -1;
    }
#endif

    if (ScMaxIP6Extensions() != sc->max_ip6_extensions)
    {
	ErrorMessage("Snort Reload: Changing Max IP6 extensions "
		     "configuration requires a restart.\n");
	return -1;
    }

    return 0;
}

static int VerifyOutputs(SnortConfig *old_config, SnortConfig *new_config)
{
    OutputConfig *old_output_config, *new_output_config;
    int old_outputs = 0, new_outputs = 0;

    /* Get from output_configs to see if output has changed */
    for (old_output_config = old_config->output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {
        old_outputs++;
    }

    for (new_output_config = new_config->output_configs;
         new_output_config != NULL;
         new_output_config = new_output_config->next)
    {
        new_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    for (old_output_config = old_config->output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {

        for (new_output_config = new_config->output_configs;
                new_output_config != NULL;
                new_output_config = new_output_config->next)
        {
            if (strcasecmp(old_output_config->keyword, new_output_config->keyword) == 0)
            {
                if ((old_output_config->opts != NULL) &&
                        (new_output_config->opts != NULL) &&
                        (strcasecmp(old_output_config->opts, new_output_config->opts) == 0))
                {
                    new_outputs++;
                    break;
                }
                else if (old_output_config->opts == NULL &&
                        new_output_config->opts == NULL)
                {
                    new_outputs++;
                    break;
                }
            }

        }

        old_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    /* Check user defined rule type outputs */
    for (old_output_config = old_config->rule_type_output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {
        old_outputs++;
    }

    for (new_output_config = new_config->rule_type_output_configs;
         new_output_config != NULL;
         new_output_config = new_output_config->next)
    {
        new_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    /* Do user defined rule type outputs as well */
    for (old_output_config = old_config->rule_type_output_configs;
         old_output_config != NULL;
         old_output_config = old_output_config->next)
    {
        for (new_output_config = new_config->rule_type_output_configs;
             new_output_config != NULL;
             new_output_config = new_output_config->next)
        {
            if (strcasecmp(old_output_config->keyword,
                           new_output_config->keyword) == 0)
            {
                if (old_output_config->opts && new_output_config->opts)
                {
                    if (strcasecmp(old_output_config->opts,
                                   new_output_config->opts) == 0)
                    {
                        new_outputs++;
                        break;
                    }
                }
                if (!old_output_config->opts && !new_output_config->opts)
                {
                    new_outputs++;
                    break;
                }
            }
        }

        old_outputs++;
    }

    if (new_outputs != old_outputs)
    {
        ErrorMessage("Snort Reload: Any change to any output "
                     "configurations requires a restart.\n");
        return -1;
    }

    return 0;
}

static int VerifyLibInfos(DynamicLibInfo *old_info, DynamicLibInfo *new_info)
{
    if ((old_info != NULL) && (new_info != NULL))
    {
        unsigned i;

        if (old_info->type != new_info->type)
        {
            FatalError("%s(%d) Incompatible library types.\n",
                       __FILE__, __LINE__);
        }

        if (old_info->count != new_info->count)
            return -1;

        for (i = 0; i < old_info->count; i++)
        {
            unsigned j;
            DynamicLibPath *old_path = old_info->lib_paths[i];

            for (j = 0; j < new_info->count; j++)
            {
                DynamicLibPath *new_path = new_info->lib_paths[j];

                if ((strcmp(old_path->path, new_path->path) == 0) &&
                    (old_path->ptype == new_path->ptype))
                {
                    if (old_path->last_mod_time != new_path->last_mod_time)
                        return -1;

                    break;
                }
            }

            if (j == new_info->count)
                return -1;
        }
    }
    else if (old_info != new_info)
    {
        return -1;
    }

    return 0;
}
#endif  /* SNORT_RELOAD */

