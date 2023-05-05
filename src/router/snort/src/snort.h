/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
** Copyright (C) 1998-2005 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */

#ifndef __SNORT_H__
#define __SNORT_H__

/*  I N C L U D E S  **********************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <stdio.h>

#include "sf_types.h"
#include "spo_plugbase.h"
#include "decode.h"
#include "perf.h"
#include "sfdaq.h"
#include "sf_types.h"
#include "sfutil/sflsq.h"
#include "sfutil/sfActionQueue.h"
#include "profiler.h"
#include "rules.h"
#include "treenodes.h"
#include "sfutil/sf_ipvar.h"
#include "sfutil/sfghash.h"
#include "sfutil/sfrim.h"
#include "sfutil/sfportobject.h"
#include "sfutil/asn1.h"
#include "sfutil/sf_sechash.h"
#include "signature.h"
#include "event_queue.h"
#include "sfthreshold.h"
#include "fpcreate.h"
#include "plugbase.h"
#include "fpdetect.h"
#include "ppm.h"
#include "sfutil/sfrf.h"
#include "sfutil/sfPolicy.h"
#include "pkt_tracer.h"
#include "detection_filter.h"
#include "generators.h"
#include "preprocids.h"
#include <signal.h>
#include "sf_dynamic_meta.h"
#if defined(INLINE_FAILOPEN) || \
    defined(TARGET_BASED) || defined(SNORT_RELOAD)
# include <pthread.h>
#endif


/*  D E F I N E S  ************************************************************/
/* Mark this as a modern version of snort */
#define SNORT_20
/*
 * The original Ethernet IEEE 802.3 standard defined the minimum Ethernet 
 * frame size as 64 bytes. The snaplen is L2 MRU for snort and hence following
 * standard, the MIN_SNAPLEN should be 64.
 */
#define MIN_SNAPLEN  64
#define MAX_SNAPLEN  UINT16_MAX

#define MAX_IFS   1

#define TIMEBUF_SIZE    26
#define MAX_PIDFILE_SUFFIX 11 /* uniqueness extension to PID file, see '-R' */
#define ASSURE_ALL    0  /* all TCP alerts fire regardless of stream state */
#define ASSURE_EST    1  /* only established TCP sessions fire alerts */

/* This macro helps to simplify the differences between Win32 and
   non-Win32 code when printing out the name of the interface */
#ifndef WIN32
# define PRINT_INTERFACE(i)  (i ? i : "NULL")
#else
# define PRINT_INTERFACE(i)  print_interface(i)
#endif

#define RF_ANY_SIP    0x01
#define RF_ANY_DIP    0x02
#define RF_ANY_SP     0x04
#define RF_ANY_DP     0x10
#define RF_ANY_FLAGS  0x20

#ifndef WIN32
# define DEFAULT_LOG_DIR            "/var/log/snort"
# define DEFAULT_DAEMON_ALERT_FILE  "alert"
#else
# define DEFAULT_LOG_DIR            "log"
# define DEFAULT_DAEMON_ALERT_FILE  "log/alert.ids"
#endif  /* WIN32 */

/* you can redefine the user ID which is allowed to
 * initialize interfaces using pcap and read from them
 */
#ifndef SNIFFUSER
# define SNIFFUSER 0
#endif

#ifdef ACCESSPERMS
# define FILEACCESSBITS ACCESSPERMS
#else
# ifdef S_IAMB
#  define FILEACCESSBITS S_IAMB
# else
#  define FILEACCESSBITS 0x1FF
# endif
#endif

#define DO_IP_CHECKSUMS     0x00000001
#define DO_TCP_CHECKSUMS    0x00000002
#define DO_UDP_CHECKSUMS    0x00000004
#define DO_ICMP_CHECKSUMS   0x00000008

#define LOG_UNIFIED         0x00000001
#define LOG_TCPDUMP         0x00000002
#define LOG_UNIFIED2         0x0000004

#ifndef SIGNAL_SNORT_RELOAD
#define SIGNAL_SNORT_RELOAD         SIGHUP
#endif
#ifndef SIGNAL_SNORT_DUMP_STATS
#define SIGNAL_SNORT_DUMP_STATS     SIGUSR1
#endif
#ifndef SIGNAL_SNORT_ROTATE_STATS
#define SIGNAL_SNORT_ROTATE_STATS   SIGUSR2
#endif

// this one should not be changed by user
#define SIGNAL_SNORT_CHILD_READY    SIGCHLD

#ifdef TARGET_BASED
#ifndef SIGNAL_SNORT_READ_ATTR_TBL
# define SIGNAL_SNORT_READ_ATTR_TBL SIGURG
#endif
#endif

#define MODE_PACKET_DUMP    1
#define MODE_PACKET_LOG     2
#define MODE_IDS            3
#define MODE_TEST           4
#define MODE_RULE_DUMP      5
#define MODE_VERSION        6

#define LOG_ASCII   1
#define LOG_PCAP    2
#define LOG_NONE    3

#define ALERT_FULL     1
#define ALERT_FAST     2
#define ALERT_NONE     3
#define ALERT_UNSOCK   4
#define ALERT_STDOUT   5
#define ALERT_CMG      6
#define ALERT_SYSLOG   8
#define ALERT_TEST     9
#define ALERT_UNIFIED  10

#ifdef MPLS
# define MPLS_PAYLOADTYPE_IPV4         1
# define MPLS_PAYLOADTYPE_ETHERNET     2
# define MPLS_PAYLOADTYPE_IPV6         3
# define MPLS_PAYLOADTYPE_ERROR       -1
# define DEFAULT_MPLS_PAYLOADTYPE      MPLS_PAYLOADTYPE_IPV4
# define DEFAULT_LABELCHAIN_LENGTH    -1
#endif

/* This feature allows us to change the state of a rule,
 * independent of it appearing in a rules file.
 */
#define RULE_STATE_DISABLED 0
#define RULE_STATE_ENABLED 1

#define MAX_DYNAMIC_ENGINES         16
#define MAX_DYNAMIC_DETECTION_LIBS  16
#define MAX_DYNAMIC_PREPROC_LIBS    16

#ifdef TARGET_BASED
# define ATTRIBUTE_TABLE_RELOAD_FLAG          0x01
# define ATTRIBUTE_TABLE_AVAILABLE_FLAG       0x02
# define ATTRIBUTE_TABLE_RELOADING_FLAG       0x04
# define ATTRIBUTE_TABLE_TAKEN_FLAG           0x08
# define ATTRIBUTE_TABLE_PARSE_FAILED_FLAG    0x10
# define DEFAULT_MAX_ATTRIBUTE_HOSTS   10000
# define DEFAULT_MAX_ATTRIBUTE_SERVICES_PER_HOST 100
# define DEFAULT_MAX_METADATA_SERVICES     8
# define MAX_MAX_ATTRIBUTE_HOSTS   (512 * 1024)
# define MIN_MAX_ATTRIBUTE_HOSTS    32
# define MAX_MAX_ATTRIBUTE_SERVICES_PER_HOST   65535
# define MIN_MAX_ATTRIBUTE_SERVICES_PER_HOST       1
# define MAX_MAX_METADATA_SERVICES 256
# define MIN_MAX_METADATA_SERVICES 1
#if defined(FEAT_OPEN_APPID)
# define MAX_MAX_METADATA_APPID 256
# define MIN_MAX_METADATA_APPID 1
# define DEFAULT_MAX_METADATA_APPID     8
#endif /* defined(FEAT_OPEN_APPID) */
#endif

# define DEFAULT_MAX_IP6_EXTENSIONS     8

struct _SnortConfig;
typedef int (*InitDetectionLibFunc)(struct _SnortConfig *);

/*  D A T A  S T R U C T U R E S  *********************************************/
typedef struct _VarEntry
{
    char *name;
    char *value;
    unsigned char flags;
    IpAddrSet *addrset;
    uint32_t id;
    struct _VarEntry *prev;
    struct _VarEntry *next;

} VarEntry;

/* GetoptLong Option numbers ********************/
typedef enum _GetOptLongIds
{
    PID_PATH = 1,

    DYNAMIC_LIBRARY_DIRECTORY,
    DYNAMIC_LIBRARY_FILE,
    DYNAMIC_PREPROC_DIRECTORY,
    DYNAMIC_PREPROC_FILE,
    DYNAMIC_ENGINE_FILE,
    DYNAMIC_ENGINE_DIRECTORY,
    DUMP_DYNAMIC_RULES,
    DYNAMIC_OUTPUT_DIRECTORY,
    DYNAMIC_OUTPUT_FILE,

    CREATE_PID_FILE,
    TREAT_DROP_AS_ALERT,
    TREAT_DROP_AS_IGNORE,
    PROCESS_ALL_EVENTS,
    ALERT_BEFORE_PASS,
    NOLOCK_PID_FILE,
    NO_IFACE_PID_FILE,

#ifdef INLINE_FAILOPEN
    DISABLE_INLINE_FAILOPEN,
#endif

    NO_LOGGING_TIMESTAMPS,
    PCAP_LOOP,
    PCAP_SINGLE,
    PCAP_FILE_LIST,
    PCAP_LIST,
    PCAP_DIR,
    PCAP_FILTER,
    PCAP_NO_FILTER,
    PCAP_RELOAD,
    PCAP_RESET,
    PCAP_SHOW,

#define EXIT_CHECK  // allow for rollback for now
#ifdef EXIT_CHECK
    ARG_EXIT_CHECK,
#endif

#ifdef TARGET_BASED
    DISABLE_ATTRIBUTE_RELOAD,
#endif

    DETECTION_SEARCH_METHOD,
    CONF_ERROR_OUT,

#ifdef MPLS
    ENABLE_MPLS_MULTICAST,
    ENABLE_OVERLAPPING_IP,
    MAX_MPLS_LABELCHAIN_LEN,
    MPLS_PAYLOAD_TYPE,
#endif

    REQUIRE_RULE_SID,

    ARG_DAQ_TYPE,
    ARG_DAQ_MODE,
    ARG_DAQ_VAR,
    ARG_DAQ_DIR,
    ARG_DAQ_LIST,
    ARG_DIRTY_PIG,

    ENABLE_INLINE_TEST,

    ARG_CS_DIR,
    ARG_HA_PEER,
    ARG_HA_OUT,
    ARG_HA_IN,
    ARG_HA_PDTS_IN,

    SUPPRESS_CONFIG_LOG,

#ifdef DUMP_BUFFER
    BUFFER_DUMP,
    BUFFER_DUMP_ALERT,
#endif

    GET_OPT_LONG_IDS_MAX

} GetOptLongIds;

typedef struct _PreprocConfig
{
    char *keyword;
    char *opts;
    char *file_name;
    int file_line;
    /* We have to configure internal and dynamic preprocessors separately,
     * mainly because of the stream_api which is set in stream5 and needs to
     * be set before calling the dynamic preprocessor initialization
     * functions which set _dpd and call the setup function.  streamAPI is set
     * in the _dpd so stream5 needs to be configured first */
    int configured;
    struct _PreprocConfig *next;

} PreprocConfig;

typedef struct _OutputConfig
{
    char *keyword;
    char *opts;
    char *file_name;
    int file_line;
    ListHead *rule_list;
    struct _OutputConfig *next;

} OutputConfig;

#ifdef SIDE_CHANNEL
typedef struct _SideChannelModuleConfig
{
    char *keyword;
    char *opts;
    char *file_name;
    int file_line;
    struct _SideChannelModuleConfig *next;
} SideChannelModuleConfig;

typedef struct _SideChannelConfig
{
    bool enabled;
    char *opts;
    SideChannelModuleConfig *module_configs;
} SideChannelConfig;
#endif

typedef enum _DynamicType
{
    DYNAMIC_TYPE__ENGINE,
    DYNAMIC_TYPE__DETECTION,
    DYNAMIC_TYPE__PREPROCESSOR,
    DYNAMIC_TYPE__SIDE_CHANNEL,
    DYNAMIC_TYPE__MAX

} DynamicType;

typedef enum _PathType
{
    PATH_TYPE__FILE,
    PATH_TYPE__DIRECTORY

} PathType;

typedef struct _DynamicLibPath
{
    PathType ptype;
    char *path;
    time_t last_mod_time;

} DynamicLibPath;

#define MAX_DYNAMIC_LIBS 16

typedef struct _DynamicLibInfo
{
    DynamicType type;
    unsigned int count;
    DynamicLibPath *lib_paths[MAX_DYNAMIC_LIBS];

} DynamicLibInfo;


typedef enum _RunMode
{
    /* -V */
    RUN_MODE__VERSION = 1,

    /* --dump-dynamic-rules */
    RUN_MODE__RULE_DUMP,

    /* neither of the above and snort.conf presence (-c or implicit) */
    RUN_MODE__IDS,

    /* snort.conf presence and -T */
    RUN_MODE__TEST,

    /* neither -V or --dump-dynamic-rules and no snort.conf, but logging
     * enabled on command line - NONE type logging seems to count here */
    RUN_MODE__PACKET_LOG,

    RUN_MODE__PACKET_DUMP

} RunMode;


typedef enum _RunModeFlag
{
    /* -V */
    RUN_MODE_FLAG__VERSION      = 0x00000001,

    /* --dump-dynamic-rules */
    RUN_MODE_FLAG__RULE_DUMP    = 0x00000002,

    /* neither of the above and snort.conf presence (-c or implicit) */
    RUN_MODE_FLAG__IDS          = 0x00000004,

    /* snort.conf presence and -T */
    RUN_MODE_FLAG__TEST         = 0x00000008,

    /* neither -V or --dump-dynamic-rules and no snort.conf, but logging
     * enabled on command line - NONE type logging seems to count here */
    RUN_MODE_FLAG__PACKET_LOG   = 0x00000010,

    RUN_MODE_FLAG__PACKET_DUMP  = 0x00000020

} RunModeFlag;

typedef enum _RunFlag
{
    RUN_FLAG__READ                = 0x00000001,     /* -r --pcap-dir, etc. */
    RUN_FLAG__DAEMON              = 0x00000002,     /* -D */
    RUN_FLAG__DAEMON_RESTART      = 0x00000004,     /* --restart */
    RUN_FLAG__NO_PROMISCUOUS      = 0x00000008,     /* -p */
    RUN_FLAG__INLINE              = 0x00000010,     /* -Q */
    RUN_FLAG__STATIC_HASH         = 0x00000020,     /* -H */
    RUN_FLAG__CREATE_PID_FILE     = 0x00000040,     /* --pid-path and --create-pidfile */
    RUN_FLAG__NO_LOCK_PID_FILE    = 0x00000080,     /* --nolock-pidfile */
    RUN_FLAG__TREAT_DROP_AS_ALERT = 0x00000100,     /* --treat-drop-as-alert */
    RUN_FLAG__ALERT_BEFORE_PASS   = 0x00000200,     /* --alert-before-pass */
    RUN_FLAG__CONF_ERROR_OUT      = 0x00000400,     /* -x and --conf-error-out */
#ifdef MPLS
    RUN_FLAG__MPLS_MULTICAST      = 0x00000800,     /* --enable_mpls_multicast */
    RUN_FLAG__MPLS_OVERLAPPING_IP = 0x00001000,     /* --enable_mpls_overlapping_ip */
#endif

    /* --process-all-events
     * this is transferred to the snort event queue var */
    RUN_FLAG__PROCESS_ALL_EVENTS  = 0x00002000,

#ifdef TARGET_BASED
    /* --disable-attribute-reload-thread */
    RUN_FLAG__DISABLE_ATTRIBUTE_RELOAD_THREAD
                                  = 0x00004000,
#endif
    RUN_FLAG__STATEFUL            = 0x00008000,     /* set if stream5 configured */
    RUN_FLAG__INLINE_TEST         = 0x00010000,     /* --enable-inline-test*/
    // UNUSED                     = 0x00020000,

#ifdef INLINE_FAILOPEN
    RUN_FLAG__DISABLE_FAILOPEN    = 0x00040000,     /* --disable-inline-init-failopen */
#endif

#ifdef MIMICK_IPV6
    RUN_FLAG__MIMICK_IP6          = 0x00100000,     /* -6 */
#endif

    RUN_FLAG__PCAP_RESET          = 0x00200000,
    RUN_FLAG__PCAP_SHOW           = 0x00400000,
    RUN_FLAG__REQUIRE_RULE_SID    = 0x00800000,
    RUN_FLAG__NO_PCRE             = 0x01000000,
    RUN_FLAG__ASSURE_EST          = 0x02000000      /* config stateful */
#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
   ,RUN_FLAG__TERMINATE_SERVICE   = 0x04000000,
    RUN_FLAG__PAUSE_SERVICE       = 0x08000000
#endif

   ,RUN_FLAG__TREAT_DROP_AS_IGNORE= 0x10000000,     /* --treat-drop-as-ignore */
#if defined(SNORT_RELOAD) && !defined(WIN32)
    RUN_FLAG__PCAP_RELOAD         = 0x20000000,     /* --pcap-reload */
#endif
    RUN_FLAG__NO_IFACE_PID_FILE   = 0x40000000      /* --no-interface-pidfile */

} RunFlag;

typedef enum _OutputFlag
{
    OUTPUT_FLAG__LINE_BUFFER       = 0x00000001,      /* -f */
    OUTPUT_FLAG__VERBOSE_DUMP      = 0x00000002,      /* -X */
    OUTPUT_FLAG__CHAR_DATA         = 0x00000004,      /* -C */
    OUTPUT_FLAG__APP_DATA          = 0x00000008,      /* -d */
    OUTPUT_FLAG__SHOW_DATA_LINK    = 0x00000010,      /* -e */
#ifndef NO_NON_ETHER_DECODER
    OUTPUT_FLAG__SHOW_WIFI_MGMT    = 0x00000020,      /* -w */
#endif
    OUTPUT_FLAG__USE_UTC           = 0x00000040,      /* -U */
    OUTPUT_FLAG__INCLUDE_YEAR      = 0x00000080,      /* -y */

    /* Note using this alters the packet - can't be used inline */
    OUTPUT_FLAG__OBFUSCATE         = 0x00000100,      /* -B */

    OUTPUT_FLAG__ALERT_IFACE       = 0x00000200,      /* -I */
    OUTPUT_FLAG__NO_TIMESTAMP      = 0x00000400,      /* --nostamps */
    OUTPUT_FLAG__ALERT_PKT_CNT     = 0x00000800,      /* -A packet-count */
    /* XXX XXX pv.outputVidInAlerts */
    OUTPUT_FLAG__ALERT_VLAN        = 0x00001000       /* config include_vlan_in_alerts */

} OutputFlag;

typedef enum _LoggingFlag
{
    LOGGING_FLAG__VERBOSE         = 0x00000001,      /* -v */
    LOGGING_FLAG__QUIET           = 0x00000002,      /* -q */
    LOGGING_FLAG__SYSLOG          = 0x00000004       /* -M */
#ifdef WIN32
   ,LOGGING_FLAG__SYSLOG_REMOTE   = 0x00000008       /* -s and -E */
#endif

} LoggingFlag;

typedef enum _InternalLogLevel
{
    INTERNAL_LOG_LEVEL__SUPPRESS_ALL,
    INTERNAL_LOG_LEVEL__ERROR,
    INTERNAL_LOG_LEVEL__WARNING,
    INTERNAL_LOG_LEVEL__MESSAGE
} InternalLogLevel;

/* -k
 * config checksum_mode
 * config checksum_drop_mode */
typedef enum _ChecksumFlag
{
    CHECKSUM_FLAG__IP   = 0x00000001,
    CHECKSUM_FLAG__TCP  = 0x00000002,
    CHECKSUM_FLAG__UDP  = 0x00000004,
    CHECKSUM_FLAG__ICMP = 0x00000008,
    CHECKSUM_FLAG__ALL  = 0x7fffffff

} ChecksumFlag;

typedef enum
{
    /* config autogenerate_preprocessor_decoder_rules */
    POLICY_FLAG__AUTO_OTN = 0x00000001
} PolicyFlag;

typedef enum _PolicyModeFlag
{
    POLICY_MODE__PASSIVE,
    POLICY_MODE__INLINE,
    POLICY_MODE__INLINE_TEST
} PolicyMode;

typedef enum _DecodeEventFlag
{
    DECODE_EVENT_FLAG__DEFAULT              = 0x00000001,
    DECODE_EVENT_FLAG__OVERSIZED            = 0x00000002,
    DECODE_EVENT_FLAG__TCP_EXP_OPT          = 0x00000004,
    DECODE_EVENT_FLAG__TCP_OBS_OPT          = 0x00000008,
    DECODE_EVENT_FLAG__TCP_TTCP_OPT         = 0x00000010,
    DECODE_EVENT_FLAG__TCP_OPT_ANOMALY      = 0x00000020,
    DECODE_EVENT_FLAG__IP_OPT_ANOMALY       = 0x00000040,
    DECODE_EVENT_FLAG__IPV6_BAD_FRAG        = 0x00000080,
    DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG   = 0x00000100

} DecodeEventFlag;

typedef enum {
    TUNNEL_GTP    = 0x01,
    TUNNEL_TEREDO = 0x02,
    TUNNEL_6IN4   = 0x04,
    TUNNEL_4IN6   = 0x08,
    TUNNEL_4IN4   = 0x10,
    TUNNEL_6IN6   = 0x20,
    TUNNEL_GRE    = 0x40,
    TUNNEL_MPLS   = 0x80
} TunnelFlags;

typedef struct _VarNode
{
    char *name;
    char *value;
    char *line;
    struct _VarNode *next;

} VarNode;

#ifdef TARGET_BASED
typedef struct _TargetBasedConfig
{
    char *args;
    char *file_name;
    int file_line;

} TargetBasedConfig;
#endif

typedef struct _SnortPolicy
{
#ifdef TARGET_BASED
    TargetBasedConfig target_based_config;
#endif
    PreprocConfig *preproc_configs;

    VarEntry *var_table;
    uint32_t var_id;
    vartable_t *ip_vartable;

    /* The portobjects in these are attached to rtns and used during runtime */
    PortVarTable *portVarTable;     /* named entries, uses a hash table */
    PortTable *nonamePortVarTable;  /* un-named entries */

    PreprocEnableMask pp_enabled[MAX_PORTS];
    PreprocEvalFuncNode *preproc_eval_funcs;
    PreprocEvalFuncNode *unused_preproc_eval_funcs;
    PreprocMetaEvalFuncNode *preproc_meta_eval_funcs;

    int preproc_proto_mask;
    int num_preprocs;
    int num_meta_preprocs;
    int ips_policy_mode;
    int nap_policy_mode;
    uint32_t policy_flags;

    /* mask of preprocessors that have registered runtime process functions */
    PreprocEnableMask preproc_bit_mask;
    PreprocEnableMask preproc_meta_bit_mask;

    int num_detects;
    //int detect_bit_mask;
    int detect_proto_mask;
    DetectionEvalFuncNode *detect_eval_funcs;

    /** Identifier assigned by user to correlate unified2 events to actual
     * policy. User or DC should assign each policy a unique number. Snort
     * will not verify uniqueness.
     */
    unsigned short configPolicyId;

    char *policy_version;

    uint8_t min_ttl;            /* config min_ttl */
#ifdef NORMALIZER
    uint8_t new_ttl;            /* config new_ttl */
#endif

    //checksum_mode and checksum_drop are now policy specific
    int checksum_flags;         /* -k */
    int checksum_flags_modified;
    int checksum_flags_saved;
    int checksum_drop_flags;
    int checksum_drop_flags_modified;

    //disable_decode_alerts and disable_decode_drop
    int decoder_alert_flags;
    int decoder_drop_flags;
    int decoder_alert_flags_saved;
    int decoder_drop_flags_saved;
    bool ssl_policy_enabled;
} SnortPolicy;

typedef struct _DynamicDetectionPlugin
{
    void *handle;
    DynamicPluginMeta metaData;
    InitDetectionLibFunc initFunc;
    struct _DynamicDetectionPlugin *next;
    struct _DynamicDetectionPlugin *prev;
} DynamicDetectionPlugin;

#ifdef INTEL_SOFT_CPM
struct _IntelPmHandles;
#endif
struct _MandatoryEarlySessionCreator;
#ifdef SNORT_RELOAD
struct _ReloadAdjustEntry;
#endif
struct _fileConfig;
struct _DynamicRuleNode;

typedef struct _IpsPortFilter
{
    tSfPolicyId parserPolicyId;
    uint16_t port_filter[MAX_PORTS + 1];
} IpsPortFilter;

typedef struct _SnortConfig
{
    RunMode run_mode;
    int run_mode_flags;
    int run_flags;
    int output_flags;
    int logging_flags;
    int log_tcpdump;
    int no_log;
    int no_alert;
    int dirty_pig;

    //used for processing command line arguments, checksum configuration
    //in conf files is maintained at policy level
    int checksum_flags;         /* -k */
    int checksum_flags_modified;
    int checksum_drop_flags;
    int checksum_drop_flags_modified;

    uint32_t event_log_id;      /* -G */
    int pkt_snaplen;
    uint64_t pkt_cnt;           /* -n */
#ifdef REG_TEST
    uint64_t pkt_skip;
#endif

    char *dynamic_rules_path;   /* --dump-dynamic-rules */

    /* --dynamic-engine-lib
     * --dynamic-engine-lib-dir
     * --dynamic-detection-lib
     * --dynamic-detection-lib-dir
     * --dynamic-preprocessor-lib
     * --dynamic-preprocessor-lib-dir
     *
     * See below for struct type
     */
    DynamicLibInfo *dyn_engines;
    DynamicLibInfo *dyn_rules;
    DynamicLibInfo *dyn_preprocs;
#ifdef SIDE_CHANNEL
    DynamicLibInfo *dyn_side_channels;
#endif

    char pid_path[STD_BUF];  /* --pid-path or config pidpath */

#ifdef EXIT_CHECK
    uint64_t exit_check;        /* --exit-check */
#endif

    /* -h and -B */
    sfcidr_t homenet;
    sfcidr_t obfuscation_net;

    /* config disable_decode_alerts
     * config enable_decode_oversized_alerts
     * config enable_decode_oversized_drops
     * config enable_decode_drops
     * config disable_decode_drops
     * config disable_tcpopt_experimental_alerts
     * config enable_tcpopt_experimental_drops
     * config disable_tcpopt_experimental_drops
     * config disable_tcpopt_obsolete_alerts
     * config enable_tcpopt_obsolete_drops
     * config disable_tcpopt_obsolete_drops
     * config disable_ttcp_alerts, config disable_tcpopt_ttcp_alerts
     * config enable_ttcp_drops, config enable_tcpopt_ttcp_drops
     * config disable_ttcp_drops
     * config disable_tcpopt_alerts
     * config enable_tcpopt_drops
     * config disable_tcpopt_drops
     * config disable_ipopt_alerts
     * config enable_ipopt_drops
     * config disable_ipopt_drops
     * config ipv6_frag:
     *   bsd_icmp_frag_alert
     *   bad_ipv6_frag_alert
     *   frag_timeout  -  not in DecoderFlags
     *   max_frag_sessions  -  not in DecoderFlags
     *   drop_bad_ipv6_frag
     */
    uint32_t ipv6_frag_timeout;
    uint32_t ipv6_max_frag_sessions;

    uint16_t flowbit_size;

    char pid_filename[STD_BUF];  /* used with pid_path */
    char pidfile_suffix[MAX_PIDFILE_SUFFIX + 1];  /* -R */
    char *log_dir;           /* -l or config log_dir */
    char *orig_log_dir;      /* set in case of chroot */
    char *interface;         /* -i or config interface */
    char *bpf_file;          /* -F or config bpf_file */
    char *pcap_log_file;     /* -L */
    char *chroot_dir;        /* -t or config chroot */
    char *alert_file;
    char *perf_file;         /* -Z */
    char *bpf_filter;        /* last command line arguments */
    char* daq_type;          /* --daq or config daq */
    char* daq_mode;          /* --daq-mode or config daq_mode */
    void* daq_vars;          /* --daq-var or config daq_var */
    void* daq_dirs;          /* --daq-dir or config daq_dir */

    char* event_trace_file;
    uint16_t event_trace_max;

    int thiszone;

#ifdef WIN32
    char syslog_server[STD_BUF];
    int syslog_server_port;
# ifdef ENABLE_WIN32_SERVICE
    int terminate_service_flag;
    int pause_service_flag;
# endif
#endif

    uint8_t ignore_ports[UINT16_MAX + 1];    /* config ignore_ports */
    long int tagged_packet_limit;            /* config tagged_packet_limit */
    long int pcre_match_limit;               /* config pcre_match_limit */
    long int pcre_match_limit_recursion;     /* config pcre_match_limit_recursion */
    int *pcre_ovector;
    int pcre_ovector_size;

#ifdef PERF_PROFILING
    ProfileConfig profile_rules;     /* config profile_rules */
    ProfileConfig profile_preprocs;  /* config profile_preprocs */
#endif

    int user_id;
    int group_id;

    mode_t file_mask;

#ifdef MPLS
    uint8_t mpls_payload_type;  /* --mpls_payload_type */
    long int mpls_stack_depth;  /* --max_mpls_labelchain_len */
#endif

    int default_rule_state;     /* config default_rule_state */

    char* react_page;        /* config react */

#ifdef ACTIVE_RESPONSE
    uint8_t respond_attempts;    /* config respond */
    char* respond_device;
    uint8_t *eth_dst;        /* config destination MAC address */
#endif

#ifdef TARGET_BASED
    uint32_t max_attribute_hosts;    /* config max_attribute_hosts */
    uint32_t max_attribute_services_per_host;    /* config max_attribute_services_per_host */
    uint32_t max_metadata_services;  /* config max_metadata_services */
#endif
#if defined(FEAT_OPEN_APPID)

    uint32_t max_metadata_appid;
#endif /* defined(FEAT_OPEN_APPID) */

    OutputConfig *output_configs;
    OutputConfig *rule_type_output_configs;
    SFGHASH *config_table;   /* table of config keywords and arguments */
    int asn1_mem;

    int active_dynamic_nodes;

    RuleState *rule_state_list;
    ClassType *classifications;
    ReferenceSystemNode *references;
    SFGHASH *so_rule_otn_map;
    SFGHASH *otn_map;
    SFGHASH *preproc_rule_options;

    FastPatternConfig *fast_pattern_config;
    EventQueueConfig *event_queue_config;

    PreprocPostConfigFuncNode *preproc_post_config_funcs;
    PreprocCheckConfigFuncNode *preproc_config_check_funcs;

    /* XXX XXX policy specific? */
    ThresholdConfig *threshold_config;
    RateFilterConfig *rate_filter_config;
    DetectionFilterConfig *detection_filter_config;

    SF_EVENTQ *event_queue[NUM_EVENT_QUEUES];

    SF_LIST **ip_proto_only_lists;
    uint8_t ip_proto_array[NUM_IP_PROTOS];

    int num_rule_types;
    RuleListNode *rule_lists;
    int evalOrder[RULE_TYPE__MAX + 1];

    ListHead Alert;         /* Alert Block Header */
    ListHead Log;           /* Log Block Header */
    ListHead Pass;          /* Pass Block Header */
    ListHead Activation;    /* Activation Block Header */
    ListHead Dynamic;       /* Dynamic Block Header */
    ListHead Drop;
    ListHead SDrop;
    ListHead Reject;

    PostConfigFuncNode *plugin_post_config_funcs;

    OTNX_MATCH_DATA *omd;

    /* Pattern matcher queue statistics */
    unsigned int max_inq;
    uint64_t tot_inq_flush;
    uint64_t tot_inq_inserts;
    uint64_t tot_inq_uinserts;

    /* Protected Content secure hash type default */
    Secure_Hash_Type Default_Protected_Content_Hash_Type;

    /* master port list table */
    rule_port_tables_t *port_tables;

#ifdef PPM_MGR
    ppm_cfg_t ppm_cfg;
#endif

    /* The port-rule-maps map the src-dst ports to rules for
     * udp and tcp, for Ip we map the dst port as the protocol,
     * and for Icmp we map the dst port to the Icmp type. This
     * allows us to use the decode packet information to in O(1)
     * select a group of rules to apply to the packet.  These
     * rules may have uricontent, content, or they may be no content
     * rules, or any combination. We process the uricontent 1st,
     * then the content, and then the no content rules for udp/tcp
     * and icmp, than we process the ip rules. */
    PORT_RULE_MAP *prmIpRTNX;
    PORT_RULE_MAP *prmTcpRTNX;
    PORT_RULE_MAP *prmUdpRTNX;
    PORT_RULE_MAP *prmIcmpRTNX;

#ifdef TARGET_BASED
    srmm_table_t *srmmTable;   /* srvc rule map master table */
    srmm_table_t *spgmmTable;  /* srvc port_group map master table */
    sopg_table_t *sopgTable;   /* service-oridnal to port_group table */
#endif

    SFXHASH *detection_option_hash_table;
    SFXHASH *detection_option_tree_hash_table;
    SFXHASH *rtn_hash_table;

    tSfPolicyConfig *policy_config;
    SnortPolicy **targeted_policies;
    IpsPortFilter **udp_ips_port_filter_list;
    unsigned int num_policies_allocated;

    char *base_version;

    uint8_t enable_teredo; /* config enable_deep_teredo_inspection */
    uint8_t enable_gtp; /* config enable_gtp */
    char *gtp_ports;
    uint8_t enable_esp;
    uint8_t vlan_agnostic; /* config vlan_agnostic */
    uint8_t addressspace_agnostic; /* config addressspace_agnostic */
    uint8_t log_ipv6_extra; /* config log_ipv6_extra_data */
    uint8_t tunnel_mask;

    uint32_t so_rule_memcap;
    uint32_t paf_max;          /* config paf_max */
    char *cs_dir;
    bool ha_peer;
    char *ha_out;
    char *ha_in;
    char *ha_pdts_in;
    char *output_dir;
    struct _fileConfig *file_config;
    int disable_all_policies;
    PreprocEnableMask reenabled_preprocessor_bits; /* flags for preprocessors to check, if all policies are disabled */
#ifdef SIDE_CHANNEL
    SideChannelConfig side_channel_config;
#endif
#ifdef SNORT_RELOAD
    int reloadPolicyFlag;
    PreprocessorSwapData *preprocSwapData;
    void *streamReloadConfig;
#endif
    tSfPolicyId parserPolicyId;
#ifdef INTEL_SOFT_CPM
    struct _IntelPmHandles *ipm_handles;
#endif

/* Used when a user defines a new rule type (ruletype keyword)
 * It points to the new rule type's ListHead and is used for accessing the
 * rule type's AlertList and LogList.
 * The output plugins used for the rule type need to be attached to the new
 * rule type's list head's AlertList or LogList.  It's set before calling
 * the output plugin's initialization routine, because in that routine,
 * AddFuncToOutputList is called (plugbase.c) and there, the output function
 * is attached to the new rule type's appropriate list.
 * NOTE:  This variable MUST NOT be used during runtime */
    ListHead *head_tmp;

    uint8_t max_ip6_extensions;

    int internal_log_level;
    int suppress_config_log;
    uint8_t disable_replace_opt;

    struct _MandatoryEarlySessionCreator* mandatoryESCreators;
    bool normalizer_set;

#ifdef DUMP_BUFFER
    char *buffer_dump_file;
#endif

#ifdef SNORT_RELOAD
    struct _ReloadAdjustEntry* raSessionEntry;
    struct _ReloadAdjustEntry* volatile raEntry;
    struct _ReloadAdjustEntry* raCurrentEntry;
    time_t raLastLog;
#endif
    DynamicDetectionPlugin *loadedDetectionPlugins;
    struct _DynamicRuleNode *dynamic_rules;
    char *memdump_file;
} SnortConfig;

/* struct to collect packet statistics */
typedef struct _PacketCount
{
    uint64_t total_from_daq;
    uint64_t total_processed;

    uint64_t s5tcp1;
    uint64_t s5tcp2;
    uint64_t ipv6opts;
    uint64_t eth;
    uint64_t ethdisc;
    uint64_t ipv6disc;
    uint64_t ip6ext;
    uint64_t other;
    uint64_t tcp;
    uint64_t udp;
    uint64_t icmp;
    uint64_t arp;
#ifndef NO_NON_ETHER_DECODER
    uint64_t eapol;
#endif
    uint64_t vlan;
    uint64_t nested_vlan;
    uint64_t ipv6;
    uint64_t ipv6_up;
    uint64_t ipv6_upfail;
    uint64_t frag6;
    uint64_t icmp6;
    uint64_t tdisc;
    uint64_t udisc;
    uint64_t tcp6;
    uint64_t udp6;
    uint64_t teredo;
    uint64_t ipdisc;
    uint64_t icmpdisc;
    uint64_t embdip;
    uint64_t ip;
    uint64_t ipx;
    uint64_t ethloopback;

    uint64_t invalid_checksums;
    uint64_t bad_ttl;

#ifdef GRE
    uint64_t ip4ip4;
    uint64_t ip4ip6;
    uint64_t ip6ip4;
    uint64_t ip6ip6;

    uint64_t gre;
    uint64_t gre_ip;
    uint64_t gre_eth;
    uint64_t gre_arp;
    uint64_t gre_ipv6;
    uint64_t gre_ipv6ext;
    uint64_t gre_ipx;
    uint64_t gre_loopback;
    uint64_t gre_vlan;
    uint64_t gre_ppp;
#endif

    uint64_t discards;
    uint64_t alert_pkts;
    uint64_t total_alert_pkts;
    uint64_t log_pkts;
    uint64_t pass_pkts;

    uint64_t match_limit;
    uint64_t queue_limit;
    uint64_t log_limit;
    uint64_t event_limit;
    uint64_t alert_limit;

    uint64_t frags;           /* number of frags that have come in */
    uint64_t frag_trackers;   /* number of tracking structures generated */
    uint64_t rebuilt_frags;   /* number of packets rebuilt */
    uint64_t frag_incomp;     /* number of frags cleared due to memory issues */
    uint64_t frag_timeout;    /* number of frags cleared due to timeout */
    uint64_t rebuild_element; /* frags that were element of rebuilt pkt */
    uint64_t frag_mem_faults; /* number of times the memory cap was hit */

    uint64_t tcp_stream_pkts; /* number of packets tcp reassembly touches */
    uint64_t rebuilt_tcp;     /* number of phoney tcp packets generated */
    uint64_t tcp_streams;     /* number of tcp streams created */
    uint64_t rebuilt_segs;    /* number of tcp segments used in rebuilt pkts */
    uint64_t queued_segs;     /* number of tcp segments stored for rebuilt pkts */
    uint64_t str_mem_faults;  /* number of times the stream memory cap was hit */

#ifdef TARGET_BASED
    uint64_t attribute_table_reloads; /* number of times attribute table was reloaded. */
#endif

#ifndef NO_NON_ETHER_DECODER
#ifdef DLT_IEEE802_11
  /* wireless statistics */
    uint64_t wifi_mgmt;
    uint64_t wifi_data;
    uint64_t wifi_control;
    uint64_t assoc_req;
    uint64_t assoc_resp;
    uint64_t reassoc_req;
    uint64_t reassoc_resp;
    uint64_t probe_req;
    uint64_t probe_resp;
    uint64_t beacon;
    uint64_t atim;
    uint64_t dissassoc;
    uint64_t auth;
    uint64_t deauth;
    uint64_t ps_poll;
    uint64_t rts;
    uint64_t cts;
    uint64_t ack;
    uint64_t cf_end;
    uint64_t cf_end_cf_ack;
    uint64_t data;
    uint64_t data_cf_ack;
    uint64_t data_cf_poll;
    uint64_t data_cf_ack_cf_poll;
    uint64_t cf_ack;
    uint64_t cf_poll;
    uint64_t cf_ack_cf_poll;
#endif
#endif  // NO_NON_ETHER_DECODER

#ifdef MPLS
    uint64_t mpls;
#endif

    uint64_t internal_blacklist;
    uint64_t internal_whitelist;

    uint64_t syn_rate_limit_events;
    uint64_t syn_rate_limit_drops;

} PacketCount;

typedef struct _PcapReadObject
{
    int type;
    char *arg;
    char *filter;

} PcapReadObject;

#if defined(DAQ_CAPA_CST_TIMEOUT)
bool Daq_Capa_Timeout;
#endif

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
bool Daq_Capa_Vrf;
#endif

/* ptr to the packet processor */
typedef void (*grinder_t)(Packet *, const DAQ_PktHdr_t*, const uint8_t *);


/*  E X T E R N S  ************************************************************/
extern const struct timespec thread_sleep;
extern volatile bool snort_initializing;
extern volatile int snort_exiting;
#ifdef SNORT_RELOAD
typedef uint32_t snort_reload_t;
extern volatile snort_reload_t reload_signal;
extern volatile int detection_lib_changed;
extern snort_reload_t reload_total;
#endif
#if defined(SNORT_RELOAD) && !defined(WIN32)
extern volatile int snort_reload_thread_created;
extern pid_t snort_reload_thread_pid;
#endif
extern SnortConfig *snort_conf;
extern SnortConfig *snort_cmd_line_conf;
extern int internal_log_level;

#include "sfutil/sfPolicyData.h"

/* Specifically for logging the IPv6 fragmented ICMP BSD vulnerability */
extern Packet *BsdPseudoPacket;

extern PacketCount pc;        /* packet count information */
extern char **protocol_names;
extern grinder_t grinder;

#ifdef SIDE_CHANNEL
extern pthread_mutex_t snort_process_lock;
#endif
extern pthread_mutex_t dynamic_rules_lock;

extern OutputFuncNode *AlertList;
extern OutputFuncNode *LogList;
extern tSfActionQueueId decoderActionQ;

#if defined(SNORT_RELOAD) && !defined(WIN32)
extern volatile int snort_reload;
#endif

#ifdef SNORT_RELOAD
extern PostConfigFuncNode *plugin_reload_funcs;
#endif
extern PeriodicCheckFuncNode *periodic_check_funcs;

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
void print_pktverdict (Packet *, uint64_t );
void print_flow(Packet *, char *, uint32_t, uint64_t, uint64_t );
#endif

/*  P R O T O T Y P E S  ******************************************************/
int SnortMain(int argc, char *argv[]);
DAQ_Verdict ProcessPacket(Packet*, const DAQ_PktHdr_t*, const uint8_t*, void*);
Packet *NewGrinderPkt(Packet *p, DAQ_PktHdr_t* phdr, uint8_t *pkt);
void DeleteGrinderPkt(Packet *);
void SetupMetadataCallback(void);
int InMainThread(void);
bool SnortIsInitializing(void);
void SigCantHupHandler(int signal);
void print_packet_count(void);
int SignalCheck(void);
void Restart(void);
void FreeVarList(VarNode *);
SnortConfig * SnortConfNew(void);
void SnortConfFree(SnortConfig *);
void CleanupPreprocessors(SnortConfig *);
void CleanupPlugins(SnortConfig *);
void CleanExit(int);
SnortConfig * MergeSnortConfs(SnortConfig *, SnortConfig *);
void SnortShutdownThreads(int);

typedef void (*sighandler_t)(int);
int SnortAddSignal(int sig, sighandler_t handler, int);

#ifdef TARGET_BASED
void SigNoAttributeTableHandler(int);
#endif

/*
 * If any of the following API are modified or new ones are
 * introduced, we have to make sure if they are called in
 * reload path. If yes, they have to use new snort config.
 */

static inline int ScTestMode(void)
{
    return snort_conf->run_mode == RUN_MODE__TEST;
}

static inline int ScRuleDumpMode(void)
{
    return snort_conf->run_mode == RUN_MODE__RULE_DUMP;
}

static inline int ScVersionMode(void)
{
    return snort_conf->run_mode == RUN_MODE__VERSION;
}

static inline int ScIdsMode(void)
{
    return snort_conf->run_mode == RUN_MODE__IDS;
}

static inline int ScPacketLogMode(void)
{
    return snort_conf->run_mode == RUN_MODE__PACKET_LOG;
}

static inline int ScPacketDumpMode(void)
{
    return snort_conf->run_mode == RUN_MODE__PACKET_DUMP;
}

static inline int ScDaemonMode(void)
{
    return snort_conf->run_flags & RUN_FLAG__DAEMON;
}

static inline int ScDaemonRestart(void)
{
    return snort_conf->run_flags & RUN_FLAG__DAEMON_RESTART;
}

static inline int ScReadMode(void)
{
    return snort_conf->run_flags & RUN_FLAG__READ;
}

static inline int ScLogSyslog(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__SYSLOG;
}

#ifdef WIN32
static inline int ScLogSyslogRemote(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__SYSLOG_REMOTE;
}
#endif

static inline int ScLogVerbose(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__VERBOSE;
}

static inline int ScLogQuiet(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__QUIET;
}

static inline int ScCheckInternalLogLevel(int level)
{
    return internal_log_level >= level;
}

static inline void ScSetInternalLogLevel(int level)
{
    if (!ScLogQuiet())
        internal_log_level = level;
}

static inline void ScRestoreInternalLogLevel(void)
{
    internal_log_level = snort_conf->internal_log_level;
}

static inline int ScDecoderAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__DEFAULT;
}

static inline int ScDecoderDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__DEFAULT;
}

static inline int ScDecoderOversizedAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__OVERSIZED;
}

static inline int ScDecoderOversizedDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__OVERSIZED;
}

static inline int ScDecoderIpv6BadFragAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
}

static inline int ScDecoderIpv6BadFragDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
}

static inline int ScDecoderIpv6BsdIcmpFragAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG;
}

static inline int ScDecoderIpv6BsdIcmpFragDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG;
}

static inline int ScDecoderTcpOptAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

static inline int ScDecoderTcpOptDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

static inline int ScDecoderTcpOptExpAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

static inline int ScDecoderTcpOptExpDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

static inline int ScDecoderTcpOptObsAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

static inline int ScDecoderTcpOptObsDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

static inline int ScDecoderTcpOptTTcpAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

static inline int ScDecoderTcpOptTTcpDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

static inline int ScDecoderIpOptAlerts(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

static inline int ScDecoderIpOptDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

static inline int ScIpChecksums(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_flags & CHECKSUM_FLAG__IP;
}

static inline int ScIpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__IP;
}

static inline int ScUdpChecksums(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_flags & CHECKSUM_FLAG__UDP;
}

static inline int ScUdpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__UDP;
}

static inline int ScTcpChecksums(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_flags & CHECKSUM_FLAG__TCP;
}

static inline int ScTcpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__TCP;
}

static inline int ScIcmpChecksums(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_flags & CHECKSUM_FLAG__ICMP;
}

static inline int ScIcmpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__ICMP;
}

static inline int ScIgnoreTcpPort(uint16_t port)
{
    return snort_conf->ignore_ports[port] & PROTO_BIT__TCP;
}

static inline int ScIgnoreUdpPort(uint16_t port)
{
    return snort_conf->ignore_ports[port] & PROTO_BIT__UDP;
}

#ifdef MPLS
static inline long int ScMplsStackDepth(void)
{
    return snort_conf->mpls_stack_depth;
}

#ifdef MPLS_RFC4023_SUPPORT
static inline long int ScMplsPayloadCheck(uint8_t ih1, long int iRet)
{
    // IPv4
    if (((ih1 & 0xF0) >> 4) == 4)
        return MPLS_PAYLOADTYPE_IPV4;
    // IPv6
    else if (((ih1 & 0xF0) >> 4) == 6)
        return MPLS_PAYLOADTYPE_IPV6;
    else
        return iRet;
}
#endif

static inline long int ScMplsPayloadType(void)
{
    return snort_conf->mpls_payload_type;
}

static inline int ScMplsOverlappingIp(void)
{
    return snort_conf->run_flags & RUN_FLAG__MPLS_OVERLAPPING_IP;
}

static inline int ScMplsMulticast(void)
{
    return snort_conf->run_flags & RUN_FLAG__MPLS_MULTICAST;
}

#endif

static inline uint32_t ScIpv6FragTimeout(void)
{
    return snort_conf->ipv6_frag_timeout;
}

static inline uint32_t ScIpv6MaxFragSessions(void)
{
    return snort_conf->ipv6_max_frag_sessions;
}

static inline uint8_t ScMinTTL(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->min_ttl;
}

#ifdef NORMALIZER
static inline uint8_t ScNewTTL(void)
{
    return snort_conf->targeted_policies[getNapRuntimePolicy()]->new_ttl;
}
#endif

static inline uint32_t ScPafMax (void)
{
    return snort_conf->paf_max;
}

static inline bool ScPafEnabled (void)
{
    return ( ScPafMax() > 0 );
}

static inline uint32_t ScEventLogId(void)
{
    return snort_conf->event_log_id;
}

static inline int ScConfErrorOut(void)
{
    return snort_conf->run_flags & RUN_FLAG__CONF_ERROR_OUT;
}

static inline int ScAssureEstablished(void)
{
    return snort_conf->run_flags & RUN_FLAG__ASSURE_EST;
}

/* Set if stream5 is configured */
static inline int ScStateful(void)
{
    return snort_conf->run_flags & RUN_FLAG__STATEFUL;
}

static inline long int ScPcreMatchLimit(void)
{
    return snort_conf->pcre_match_limit;
}

static inline long int ScPcreMatchLimitRecursion(void)
{
    return snort_conf->pcre_match_limit_recursion;
}

#ifdef PERF_PROFILING
static inline int ScProfilePreprocs(void)
{
    return snort_conf->profile_preprocs.num;
}

static inline int ScProfileRules(void)
{
    return snort_conf->profile_rules.num;
}
#endif

static inline int ScStaticHash(void)
{
    return snort_conf->run_flags & RUN_FLAG__STATIC_HASH;
}

static inline int ScAutoGenPreprocDecoderOtns(void)
{
    return (((snort_conf->targeted_policies[getNapRuntimePolicy()])->policy_flags) & POLICY_FLAG__AUTO_OTN );
}

static inline int ScProcessAllEvents(void)
{
    return snort_conf->event_queue_config->process_all_events;
}

static inline int ScNapPassiveMode(void)
{
    return (((snort_conf->targeted_policies[getNapRuntimePolicy()])->nap_policy_mode) == POLICY_MODE__PASSIVE );
}

static inline int ScIpsPassiveMode(void)
{
    return (((snort_conf->targeted_policies[getIpsRuntimePolicy()])->ips_policy_mode) == POLICY_MODE__PASSIVE );
}

static inline int ScAdapterPassiveMode(void)
{
    return !(snort_conf->run_flags & (RUN_FLAG__INLINE | RUN_FLAG__INLINE_TEST));
}

static inline int ScNapInlineMode(void)
{
    return (((snort_conf->targeted_policies[getNapRuntimePolicy()])->nap_policy_mode) == POLICY_MODE__INLINE );
}

static inline int ScIpsInlineMode(void)
{
    return (((snort_conf->targeted_policies[getIpsRuntimePolicy()])->ips_policy_mode) == POLICY_MODE__INLINE );
}

static inline int ScAdapterInlineMode(void)
{
   return snort_conf->run_flags & RUN_FLAG__INLINE;
}

static inline int ScNapInlineTestMode(void)
{
    return (((snort_conf->targeted_policies[getNapRuntimePolicy()])->nap_policy_mode) == POLICY_MODE__INLINE_TEST );
}

static inline int ScIpsInlineTestMode(void)
{
    return (((snort_conf->targeted_policies[getIpsRuntimePolicy()])->ips_policy_mode) == POLICY_MODE__INLINE_TEST );
}

static inline int ScAdapterInlineTestMode(void)
{
    return snort_conf->run_flags & RUN_FLAG__INLINE_TEST;
}

static inline int ScOutputIncludeYear(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__INCLUDE_YEAR;
}

static inline int ScOutputUseUtc(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__USE_UTC;
}

static inline int ScOutputDataLink(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__SHOW_DATA_LINK;
}

static inline int ScVerboseByteDump(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__VERBOSE_DUMP;
}

static inline int ScAlertPacketCount(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__ALERT_PKT_CNT;
}

static inline int ScObfuscate(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__OBFUSCATE;
}

static inline int ScOutputAppData(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__APP_DATA;
}

static inline int ScOutputCharData(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__CHAR_DATA;
}

static inline int ScAlertInterface(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__ALERT_IFACE;
}

static inline int ScNoOutputTimestamp(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__NO_TIMESTAMP;
}

static inline int ScLineBufferedLogging(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__LINE_BUFFER;
}

static inline int ScDefaultRuleState(void)
{
    return snort_conf->default_rule_state;
}

static inline int ScRequireRuleSid(void)
{
    return snort_conf->run_flags & RUN_FLAG__REQUIRE_RULE_SID;
}

#ifdef INLINE_FAILOPEN
static inline int ScDisableInlineFailopen(void)
{
    return snort_conf->run_flags & RUN_FLAG__DISABLE_FAILOPEN;
}
#endif

static inline int ScNoLockPidFile(void)
{
    return snort_conf->run_flags & RUN_FLAG__NO_LOCK_PID_FILE;
}

static inline long int ScTaggedPacketLimit(void)
{
    return snort_conf->tagged_packet_limit;
}

static inline int ScCreatePidFile(void)
{
    return snort_conf->run_flags & RUN_FLAG__CREATE_PID_FILE;
}

static inline int ScNoInterfacePidFile(void)
{
    return snort_conf->run_flags & RUN_FLAG__NO_IFACE_PID_FILE;
}

static inline int ScPcapShow(void)
{
    return snort_conf->run_flags & RUN_FLAG__PCAP_SHOW;
}

static inline int ScPcapReset(void)
{
    return snort_conf->run_flags & RUN_FLAG__PCAP_RESET;
}

#ifndef NO_NON_ETHER_DECODER
static inline int ScOutputWifiMgmt(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__SHOW_WIFI_MGMT;
}
#endif

#ifdef TARGET_BASED
static inline uint32_t ScMaxAttrHosts(SnortConfig *sc)
{
    return sc->max_attribute_hosts;
}

static inline uint32_t ScMaxAttrServicesPerHost(void)
{
    return snort_conf->max_attribute_services_per_host;
}

static inline int ScDisableAttrReload(SnortConfig *sc)
{
    return sc->run_flags & RUN_FLAG__DISABLE_ATTRIBUTE_RELOAD_THREAD;
}
#endif

static inline int ScTreatDropAsAlert(void)
{
    return snort_conf->run_flags & RUN_FLAG__TREAT_DROP_AS_ALERT;
}

static inline int ScTreatDropAsIgnore(void)
{
    return snort_conf->run_flags & RUN_FLAG__TREAT_DROP_AS_IGNORE;
}

static inline int ScAlertBeforePass(void)
{
    return snort_conf->run_flags & RUN_FLAG__ALERT_BEFORE_PASS;
}

static inline int ScNoPcre(void)
{
    return snort_conf->run_flags & RUN_FLAG__NO_PCRE;
}

static inline int ScGetEvalIndex(RuleType type)
{
    return snort_conf->evalOrder[type];
}

static inline int ScNoLog(void)
{
    return snort_conf->no_log;
}

static inline int ScNoAlert(void)
{
    return snort_conf->no_alert;
}

#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
static inline int ScTerminateService(void)
{
    return snort_conf->run_flags & RUN_FLAG__TERMINATE_SERVICE;
}

static inline int ScPauseService(void)
{
    return snort_conf->run_flags & RUN_FLAG__PAUSE_SERVICE;
}
#endif

static inline int ScUid(void)
{
    return snort_conf->user_id;
}

static inline int ScGid(void)
{
    return snort_conf->group_id;
}

static inline char * ScPcapLogFile(void)
{
    return snort_conf->pcap_log_file;
}

#ifdef SIDE_CHANNEL
static inline int ScSideChannelEnabled(void)
{
    return snort_conf->side_channel_config.enabled;
}
#endif

// use of macro avoids depending on generators.h
#define EventIsInternal(gid) (gid == GENERATOR_INTERNAL)

static inline void EnableInternalEvent(RateFilterConfig *config, uint32_t sid)
{
    if (config == NULL)
        return;

    config->internal_event_mask |= (1 << sid);
}

static inline int InternalEventIsEnabled(RateFilterConfig *config, uint32_t sid)
{
    if (config == NULL)
        return 0;

    return (config->internal_event_mask & (1 << sid));
}

static inline int ScIsPreprocEnabled(uint32_t preproc_id, tSfPolicyId policy_id)
{
    SnortPolicy *policy;

    if (policy_id >= snort_conf->num_policies_allocated)
        return 0;

    policy = snort_conf->targeted_policies[policy_id];
    if (policy == NULL)
        return 0;

    if (policy->preproc_bit_mask & (UINT64_C(1) << preproc_id))
        return 1;

    return 0;
}

static inline int ScDeepTeredoInspection(void)
{
    return snort_conf->enable_teredo;
}

static inline int ScGTPDecoding(void)
{
    return snort_conf->enable_gtp;
}

static inline int ScIsGTPPort(uint16_t port)
{
    return snort_conf->gtp_ports[port];
}

static inline int ScESPDecoding(void)
{
    return snort_conf->enable_esp;
}

static inline int ScVlanAgnostic(void)
{
    return snort_conf->vlan_agnostic;
}

static inline int ScAddressSpaceAgnostic(void)
{
    return snort_conf->addressspace_agnostic;
}

static inline int ScLogIPv6Extra(void)
{
    return snort_conf->log_ipv6_extra;
}

static inline uint32_t ScSoRuleMemcap(void)
{
    return snort_conf->so_rule_memcap;
}

static inline bool ScTunnelBypassEnabled (uint8_t proto)
{
    return !(snort_conf->tunnel_mask & proto);
}

static inline uint8_t ScMaxIP6Extensions(void)
{
    return snort_conf->max_ip6_extensions;
}

static inline int ScSuppressConfigLog(void)
{
    return snort_conf->suppress_config_log;
}

static inline int ScDisableReplaceOpt(void)
{
    return snort_conf->disable_replace_opt;
}

static inline int ScIpsInlineModeNewConf (SnortConfig * sc)
{
    return (((sc->targeted_policies[getParserPolicy(sc)])->ips_policy_mode) == POLICY_MODE__INLINE );
}

static inline int ScAdapterInlineModeNewConf (SnortConfig * sc)
{
    return sc->run_flags & RUN_FLAG__INLINE;
}

static inline int ScTreatDropAsIgnoreNewConf (SnortConfig * sc)
{
    return sc->run_flags & RUN_FLAG__TREAT_DROP_AS_IGNORE;
}

static inline int ScIpsInlineTestModeNewConf (SnortConfig * sc)
{
    return (((sc->targeted_policies[getParserPolicy(sc)])->ips_policy_mode) == POLICY_MODE__INLINE_TEST );
}

static inline int ScAdapterInlineTestModeNewConf (SnortConfig * sc)
{
    return sc->run_flags & RUN_FLAG__INLINE_TEST;
}

static inline int ScTestModeNewConf (SnortConfig * sc)
{
    return sc->run_mode == RUN_MODE__TEST;
}

static inline int ScConfErrorOutNewConf (SnortConfig * sc)
{
    return sc->run_flags & RUN_FLAG__CONF_ERROR_OUT;
}

static inline int ScDefaultRuleStateNewConf (SnortConfig * sc)
{
    return sc->default_rule_state;
}

static inline int ScRequireRuleSidNewConf (SnortConfig * sc)
{
    return sc->run_flags & RUN_FLAG__REQUIRE_RULE_SID;
}

static inline int ScTreatDropAsAlertNewConf (SnortConfig * sc)
{
    return sc->run_flags & RUN_FLAG__TREAT_DROP_AS_ALERT;
}

static inline int ScSuppressConfigLogNewConf (SnortConfig* sc)
{
    return sc->suppress_config_log;
}

static inline int ScLogQuietNewConf (SnortConfig* sc)
{
    return sc->logging_flags & LOGGING_FLAG__QUIET;
}

static inline void ScSetInternalLogLevelNewConf (SnortConfig* sc, int level)
{
    if (!ScLogQuietNewConf(sc))
        internal_log_level = level;
}

static inline void ScRestoreInternalLogLevelNewConf (SnortConfig* sc)
{
    internal_log_level = sc->internal_log_level;
}

static inline long int ScPcreMatchLimitNewConf (SnortConfig *sc)
{
    return sc->pcre_match_limit;
}

static inline long int ScPcreMatchLimitRecursionNewConf (SnortConfig *sc)
{
    return sc->pcre_match_limit_recursion;
}

static inline int ScNoOutputTimestampNewConf (SnortConfig *sc)
{
    return sc->output_flags & OUTPUT_FLAG__NO_TIMESTAMP;
}

static inline int ScNapPassiveModeNewConf (SnortConfig* sc)
{
    return (((sc->targeted_policies[getParserPolicy(sc)])->nap_policy_mode) == POLICY_MODE__PASSIVE );
}

static inline int ScNapInlineTestModeNewConf (SnortConfig* sc)
{
   return (((sc->targeted_policies[getParserPolicy(sc)])->nap_policy_mode) == POLICY_MODE__INLINE_TEST );
}

static inline uint32_t ScPafMaxNewConf (SnortConfig *sc)
{
    return sc->paf_max;
}

static inline bool ScPafEnabledNewConf (SnortConfig *sc)
{
    return ( ScPafMaxNewConf(sc) > 0 );
}
#if defined(DAQ_CAPA_CST_TIMEOUT)
static inline uint64_t GetTimeout( Packet *p, uint64_t *timeout)
{
  DAQ_QueryFlow_t query;
  int rval;
  query.type = DAQ_QUERYFLOW_TYPE_TIMEOUT_VAL;
  query.length = sizeof(uint64_t);
  query.value = timeout;
  rval = DAQ_QueryFlow( p->pkth, &query);

    if(rval != DAQ_SUCCESS)
       *timeout = 1;

  return 1;
}
#endif /* query flow */
#endif  /* __SNORT_H__ */

