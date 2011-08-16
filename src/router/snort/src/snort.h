/*
** Copyright (C) 2005-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

#include "spo_plugbase.h"
#include "decode.h"
#include "perf.h"
#include "sfdaq.h"
#include "sf_types.h"
#include "sfutil/sflsq.h"
#include "profiler.h"
#include "rules.h"
#include "treenodes.h"
#include "sfutil/sf_ipvar.h"
#include "sfutil/sfghash.h"
#include "sfutil/sfrim.h"
#include "sfutil/sfportobject.h"
#include "sfutil/asn1.h"
#include "signature.h"
#include "event_queue.h"
#include "sfthreshold.h"
#include "fpcreate.h"
#include "plugbase.h"
#include "fpdetect.h"
#include "ppm.h"
#include "sfutil/sfrf.h"
#include "sfutil/sfPolicy.h"
#include "detection_filter.h"
#include "generators.h"

#if defined(HAVE_LIBPRELUDE) || defined(INLINE_FAILOPEN) || \
    defined(TARGET_BASED) || defined(SNORT_RELOAD)
# include <pthread.h>
#endif


/*  D E F I N E S  ************************************************************/
/* Mark this as a modern version of snort */
#define SNORT_20

#define MIN_SNAPLEN  68
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

#define SIGNAL_SNORT_ROTATE_STATS   28
#define SIGNAL_SNORT_CHILD_READY    29
#ifdef TARGET_BASED
# define SIGNAL_SNORT_READ_ATTR_TBL 30
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

#ifdef DYNAMIC_PLUGIN
# define MAX_DYNAMIC_ENGINES         16
# define MAX_DYNAMIC_DETECTION_LIBS  16
# define MAX_DYNAMIC_PREPROC_LIBS    16
#endif

#ifdef TARGET_BASED
# define ATTRIBUTE_TABLE_RELOAD_FLAG          0x01
# define ATTRIBUTE_TABLE_AVAILABLE_FLAG       0x02
# define ATTRIBUTE_TABLE_RELOADING_FLAG       0x04
# define ATTRIBUTE_TABLE_TAKEN_FLAG           0x08
# define ATTRIBUTE_TABLE_PARSE_FAILED_FLAG    0x10
# define DEFAULT_MAX_ATTRIBUTE_HOSTS   10000
# define DEFAULT_MAX_METADATA_SERVICES     8
# define MAX_MAX_ATTRIBUTE_HOSTS   (512 * 1024)
# define MIN_MAX_ATTRIBUTE_HOSTS    32
# define MAX_MAX_METADATA_SERVICES 256
# define MIN_MAX_METADATA_SERVICES 1
#endif

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

#ifdef DYNAMIC_PLUGIN
    DYNAMIC_LIBRARY_DIRECTORY,
    DYNAMIC_LIBRARY_FILE,
    DYNAMIC_PREPROC_DIRECTORY,
    DYNAMIC_PREPROC_FILE,
    DYNAMIC_ENGINE_FILE,
    DYNAMIC_ENGINE_DIRECTORY,
    DUMP_DYNAMIC_RULES,
#endif

    CREATE_PID_FILE,
    TREAT_DROP_AS_ALERT,
    TREAT_DROP_AS_IGNORE,
    PROCESS_ALL_EVENTS,
    ALERT_BEFORE_PASS,
    NOLOCK_PID_FILE,

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

typedef enum _DynamicType
{
    DYNAMIC_TYPE__ENGINE,
    DYNAMIC_TYPE__DETECTION,
    DYNAMIC_TYPE__PREPROCESSOR,
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

#ifdef DYNAMIC_PLUGIN
    /* --dump-dynamic-rules */
    RUN_MODE__RULE_DUMP,
#endif

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

#ifdef DYNAMIC_PLUGIN
    /* --dump-dynamic-rules */
    RUN_MODE_FLAG__RULE_DUMP    = 0x00000002,
#endif

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

   ,RUN_FLAG__TREAT_DROP_AS_IGNORE= 0x10000000      /* --treat-drop-as-ignore */

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
#ifdef SUP_IP6
    vartable_t *ip_vartable;
#endif  /* SUP_IP6 */

    /* The portobjects in these are attached to rtns and used during runtime */
    PortVarTable *portVarTable;     /* named entries, uses a hash table */
    PortTable *nonamePortVarTable;  /* un-named entries */

    PreprocEvalFuncNode *preproc_eval_funcs;
    PreprocReassemblyPktFuncNode *preproc_reassembly_pkt_funcs;

    int preproc_proto_mask;
    SFGHASH *preproc_rule_options;
    int num_preprocs;
    int policy_mode;
    uint32_t policy_flags;

    /* mask of preprocessors that have registered runtime process functions */
    int preproc_bit_mask;

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
    int checksum_drop_flags;
    int checksum_drop_flags_modified;

    //disable_decode_alerts and disable_decode_drop
    int decoder_alert_flags;
    int decoder_drop_flags;
    int decoder_alert_flags_saved;
    int decoder_drop_flags_saved;
} SnortPolicy;

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
    int64_t pkt_cnt;            /* -n */

    char *dynamic_rules_path;   /* --dump-dynamic-rules */

#ifdef DYNAMIC_PLUGIN
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
#endif

    char pid_path[STD_BUF];  /* --pid-path or config pidpath */

#ifdef EXIT_CHECK
    uint64_t exit_check;        /* --exit-check */
#endif

    /* -h and -B */
#ifdef SUP_IP6
    sfip_t homenet;
    sfip_t obfuscation_net;
#else
    uint32_t homenet;
    uint32_t netmask;
    uint32_t obfuscation_net;
    uint32_t obfuscation_mask;
#endif

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

    uint8_t flowbit_size;

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
    char *pcap_file;         /* config read_bin_file */
    char* daq_type;          /* --daq or config daq */
    char* daq_mode;          /* --daq-mode or config daq_mode */
    void* daq_vars;          /* --daq-var or config daq_var */
    void* daq_dirs;          /* --daq-dir or config daq_dir */

    int thiszone;

#ifdef WIN32
    char syslog_server[STD_BUF];
    int syslog_server_port;
# ifdef ENABLE_WIN32_SERVICE
    int terminate_service_flag;
    int pause_service_flag;
# endif
#endif

    uint8_t ignore_ports[UINT16_MAX];        /* config ignore_ports */
    long int tagged_packet_limit;            /* config tagged_packet_limit */
    long int pcre_match_limit;               /* config pcre_match_limit */
    long int pcre_match_limit_recursion;     /* config pcre_match_limit_recursion */

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
#endif

#ifdef TARGET_BASED
    uint32_t max_attribute_hosts;    /* config max_attribute_hosts */
    uint32_t max_metadata_services;  /* config max_metadata_services */
#endif

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

    FastPatternConfig *fast_pattern_config;
    EventQueueConfig *event_queue_config;

    PreprocPostConfigFuncNode *preproc_post_config_funcs;
    PreprocCheckConfigFuncNode *preproc_config_check_funcs;
#ifdef SNORT_RELOAD
    PreprocReloadVerifyFuncNode *preproc_reload_verify_funcs;
#endif

    /* XXX XXX policy specific? */
    ThresholdConfig *threshold_config;
    RateFilterConfig *rate_filter_config;
    DetectionFilterConfig *detection_filter_config;

    SF_EVENTQ *event_queue[NUM_EVENT_QUEUES];

    SF_LIST **ip_proto_only_lists;
    uint8_t ip_proto_array[NUM_IP_PROTOS];

    int num_rule_types;
    RuleListNode *rule_lists;

    ListHead Alert;         /* Alert Block Header */
    ListHead Log;           /* Log Block Header */
    ListHead Pass;          /* Pass Block Header */
    ListHead Activation;    /* Activation Block Header */
    ListHead Dynamic;       /* Dynamic Block Header */
    ListHead Drop;
    ListHead SDrop;
    ListHead Reject;

    PluginSignalFuncNode *plugin_post_config_funcs;

    OTNX_MATCH_DATA *omd;

    /* Pattern matcher queue statistics */
    unsigned int max_inq;
    uint64_t tot_inq_flush;
    uint64_t tot_inq_inserts;
    uint64_t tot_inq_uinserts;

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

    tSfPolicyConfig *policy_config;
    SnortPolicy **targeted_policies;
    unsigned int num_policies_allocated;

    char *base_version;

    uint8_t enable_teredo; /* config enable_deep_teredo_inspection */
    uint8_t vlan_agnostic; /* config vlan_agnostic */

    uint32_t so_rule_memcap;
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
    uint64_t log_pkts;
    uint64_t pass_pkts;

    uint64_t match_limit;
    uint64_t queue_limit;
    uint64_t log_limit;
    uint64_t event_limit;

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

} PacketCount;

typedef struct _PcapReadObject
{
    int type;
    char *arg;
    char *filter;

} PcapReadObject;

/* ptr to the packet processor */
typedef void (*grinder_t)(Packet *, const DAQ_PktHdr_t*, const uint8_t *);


/*  E X T E R N S  ************************************************************/
extern SnortConfig *snort_conf;

/* Specifically for logging the IPv6 fragmented ICMP BSD vulnerability */
extern Packet *BsdPseudoPacket;

extern PacketCount pc;        /* packet count information */
extern char **protocol_names;
extern grinder_t grinder;


/*  P R O T O T Y P E S  ******************************************************/
int SnortMain(int argc, char *argv[]);
int ProcessPacket(void*, const DAQ_PktHdr_t*, const uint8_t*, void*);

void SigCantHupHandler(int signal);
void print_packet_count(void);
int SignalCheck(void);
void Restart(void);
void FreeVarList(VarNode *);
SnortConfig * SnortConfNew(void);
void SnortConfFree(SnortConfig *);
void CleanupPreprocessors(SnortConfig *);
void CleanupPlugins(SnortConfig *);

static INLINE int ScTestMode(void)
{
    return snort_conf->run_mode == RUN_MODE__TEST;
}

#ifdef DYNAMIC_PLUGIN
static INLINE int ScRuleDumpMode(void)
{
    return snort_conf->run_mode == RUN_MODE__RULE_DUMP;
}
#endif

static INLINE int ScVersionMode(void)
{
    return snort_conf->run_mode == RUN_MODE__VERSION;
}

static INLINE int ScIdsMode(void)
{
    return snort_conf->run_mode == RUN_MODE__IDS;
}

static INLINE int ScPacketLogMode(void)
{
    return snort_conf->run_mode == RUN_MODE__PACKET_LOG;
}

static INLINE int ScPacketDumpMode(void)
{
    return snort_conf->run_mode == RUN_MODE__PACKET_DUMP;
}

static INLINE int ScDaemonMode(void)
{
    return snort_conf->run_flags & RUN_FLAG__DAEMON;
}

static INLINE int ScDaemonRestart(void)
{
    return snort_conf->run_flags & RUN_FLAG__DAEMON_RESTART;
}

static INLINE int ScReadMode(void)
{
    return snort_conf->run_flags & RUN_FLAG__READ;
}

static INLINE int ScLogSyslog(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__SYSLOG;
}

#ifdef WIN32
static INLINE int ScLogSyslogRemote(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__SYSLOG_REMOTE;
}
#endif

static INLINE int ScLogVerbose(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__VERBOSE;
}

static INLINE int ScLogQuiet(void)
{
    return snort_conf->logging_flags & LOGGING_FLAG__QUIET;
}

static INLINE int ScDecoderAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__DEFAULT;
}

static INLINE int ScDecoderDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__DEFAULT;
}

static INLINE int ScDecoderOversizedAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__OVERSIZED;
}

static INLINE int ScDecoderOversizedDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__OVERSIZED;
}

static INLINE int ScDecoderIpv6BadFragAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
}

static INLINE int ScDecoderIpv6BadFragDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
}

static INLINE int ScDecoderIpv6BsdIcmpFragAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG;
}

static INLINE int ScDecoderIpv6BsdIcmpFragDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG;
}

static INLINE int ScDecoderTcpOptAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

static INLINE int ScDecoderTcpOptDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

static INLINE int ScDecoderTcpOptExpAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

static INLINE int ScDecoderTcpOptExpDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

static INLINE int ScDecoderTcpOptObsAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

static INLINE int ScDecoderTcpOptObsDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

static INLINE int ScDecoderTcpOptTTcpAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

static INLINE int ScDecoderTcpOptTTcpDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

static INLINE int ScDecoderIpOptAlerts(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_alert_flags & DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

static INLINE int ScDecoderIpOptDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->decoder_drop_flags & DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

static INLINE int ScIpChecksums(void)
{
    return snort_conf->targeted_policies[getDefaultPolicy()]->checksum_flags & CHECKSUM_FLAG__IP;
}

static INLINE int ScIpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__IP;
}

static INLINE int ScUdpChecksums(void)
{
    return snort_conf->targeted_policies[getDefaultPolicy()]->checksum_flags & CHECKSUM_FLAG__UDP;
}

static INLINE int ScUdpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__UDP;
}

static INLINE int ScTcpChecksums(void)
{
    return snort_conf->targeted_policies[getDefaultPolicy()]->checksum_flags & CHECKSUM_FLAG__TCP;
}

static INLINE int ScTcpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__TCP;
}

static INLINE int ScIcmpChecksums(void)
{
    return snort_conf->targeted_policies[getDefaultPolicy()]->checksum_flags & CHECKSUM_FLAG__ICMP;
}

static INLINE int ScIcmpChecksumDrops(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->checksum_drop_flags & CHECKSUM_FLAG__ICMP;
}

static INLINE int ScIgnoreTcpPort(uint16_t port)
{
    return snort_conf->ignore_ports[port] == IPPROTO_TCP;
}

static INLINE int ScIgnoreUdpPort(uint16_t port)
{
    return snort_conf->ignore_ports[port] == IPPROTO_UDP;
}

#ifdef MPLS
static INLINE long int ScMplsStackDepth(void)
{
    return snort_conf->mpls_stack_depth;
}

static INLINE long int ScMplsPayloadType(void)
{
    return snort_conf->mpls_payload_type;
}

static INLINE int ScMplsOverlappingIp(void)
{
    return snort_conf->run_flags & RUN_FLAG__MPLS_OVERLAPPING_IP;
}

static INLINE int ScMplsMulticast(void)
{
    return snort_conf->run_flags & RUN_FLAG__MPLS_MULTICAST;
}

#endif

static INLINE uint32_t ScIpv6FragTimeout(void)
{
    return snort_conf->ipv6_frag_timeout;
}

static INLINE uint32_t ScIpv6MaxFragSessions(void)
{
    return snort_conf->ipv6_max_frag_sessions;
}

static INLINE uint8_t ScMinTTL(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->min_ttl;
}

#ifdef NORMALIZER
static INLINE uint8_t ScNewTTL(void)
{
    return snort_conf->targeted_policies[getRuntimePolicy()]->new_ttl;
}
#endif

static INLINE uint32_t ScEventLogId(void)
{
    return snort_conf->event_log_id;
}

static INLINE int ScConfErrorOut(void)
{
    return snort_conf->run_flags & RUN_FLAG__CONF_ERROR_OUT;
}

static INLINE int ScAssureEstablished(void)
{
    return snort_conf->run_flags & RUN_FLAG__ASSURE_EST;
}

/* Set if stream5 is configured */
static INLINE int ScStateful(void)
{
    return snort_conf->run_flags & RUN_FLAG__STATEFUL;
}

static INLINE long int ScPcreMatchLimit(void)
{
    return snort_conf->pcre_match_limit;
}

static INLINE long int ScPcreMatchLimitRecursion(void)
{
    return snort_conf->pcre_match_limit_recursion;
}

#ifdef PERF_PROFILING
static INLINE int ScProfilePreprocs(void)
{
    return snort_conf->profile_preprocs.num;
}

static INLINE int ScProfileRules(void)
{
    return snort_conf->profile_rules.num;
}
#endif

static INLINE int ScStaticHash(void)
{
    return snort_conf->run_flags & RUN_FLAG__STATIC_HASH;
}

#ifdef PREPROCESSOR_AND_DECODER_RULE_EVENTS
static INLINE int ScAutoGenPreprocDecoderOtns(void)
{
    return (((snort_conf->targeted_policies[getRuntimePolicy()])->policy_flags) & POLICY_FLAG__AUTO_OTN );
}
#endif

static INLINE int ScProcessAllEvents(void)
{
    return snort_conf->event_queue_config->process_all_events;
}

static INLINE int ScInlineMode(void)
{
    return (((snort_conf->targeted_policies[getRuntimePolicy()])->policy_mode) == POLICY_MODE__INLINE );
}

static INLINE int ScAdapterInlineMode(void)
{
   return snort_conf->run_flags & RUN_FLAG__INLINE;
}

static INLINE int ScInlineTestMode(void)
{
    return (((snort_conf->targeted_policies[getRuntimePolicy()])->policy_mode) == POLICY_MODE__INLINE_TEST );
}

static INLINE int ScAdapterInlineTestMode(void)
{
    return snort_conf->run_flags & RUN_FLAG__INLINE_TEST;
}

static INLINE int ScOutputIncludeYear(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__INCLUDE_YEAR;
}

static INLINE int ScOutputUseUtc(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__USE_UTC;
}

static INLINE int ScOutputDataLink(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__SHOW_DATA_LINK;
}

static INLINE int ScVerboseByteDump(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__VERBOSE_DUMP;
}

static INLINE int ScAlertPacketCount(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__ALERT_PKT_CNT;
}

static INLINE int ScObfuscate(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__OBFUSCATE;
}

static INLINE int ScOutputAppData(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__APP_DATA;
}

static INLINE int ScOutputCharData(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__CHAR_DATA;
}

static INLINE int ScAlertInterface(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__ALERT_IFACE;
}

static INLINE int ScNoOutputTimestamp(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__NO_TIMESTAMP;
}

static INLINE int ScLineBufferedLogging(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__LINE_BUFFER;
}

static INLINE int ScDefaultRuleState(void)
{
    return snort_conf->default_rule_state;
}

static INLINE int ScRequireRuleSid(void)
{
    return snort_conf->run_flags & RUN_FLAG__REQUIRE_RULE_SID;
}

#ifdef INLINE_FAILOPEN
static INLINE int ScDisableInlineFailopen(void)
{
    return snort_conf->run_flags & RUN_FLAG__DISABLE_FAILOPEN;
}
#endif

static INLINE int ScNoLockPidFile(void)
{
    return snort_conf->run_flags & RUN_FLAG__NO_LOCK_PID_FILE;
}

static INLINE long int ScTaggedPacketLimit(void)
{
    return snort_conf->tagged_packet_limit;
}

static INLINE int ScCreatePidFile(void)
{
    return snort_conf->run_flags & RUN_FLAG__CREATE_PID_FILE;
}

static INLINE int ScPcapShow(void)
{
    return snort_conf->run_flags & RUN_FLAG__PCAP_SHOW;
}

static INLINE int ScPcapReset(void)
{
    return snort_conf->run_flags & RUN_FLAG__PCAP_RESET;
}

#ifndef NO_NON_ETHER_DECODER
static INLINE int ScOutputWifiMgmt(void)
{
    return snort_conf->output_flags & OUTPUT_FLAG__SHOW_WIFI_MGMT;
}
#endif

#ifdef TARGET_BASED
static INLINE uint32_t ScMaxAttrHosts(void)
{
    return snort_conf->max_attribute_hosts;
}

static INLINE int ScDisableAttrReload(void)
{
    return snort_conf->run_flags & RUN_FLAG__DISABLE_ATTRIBUTE_RELOAD_THREAD;
}
#endif

static INLINE int ScTreatDropAsAlert(void)
{
    return snort_conf->run_flags & RUN_FLAG__TREAT_DROP_AS_ALERT;
}

static INLINE int ScTreatDropAsIgnore(void)
{
    return snort_conf->run_flags & RUN_FLAG__TREAT_DROP_AS_IGNORE;
}

static INLINE int ScAlertBeforePass(void)
{
    return snort_conf->run_flags & RUN_FLAG__ALERT_BEFORE_PASS;
}

static INLINE int ScNoPcre(void)
{
    return snort_conf->run_flags & RUN_FLAG__NO_PCRE;
}

static INLINE int ScNoLog(void)
{
    return snort_conf->no_log;
}

static INLINE int ScNoAlert(void)
{
    return snort_conf->no_alert;
}

#if defined(WIN32) && defined(ENABLE_WIN32_SERVICE)
static INLINE int ScTerminateService(void)
{
    return snort_conf->run_flags & RUN_FLAG__TERMINATE_SERVICE;
}

static INLINE int ScPauseService(void)
{
    return snort_conf->run_flags & RUN_FLAG__PAUSE_SERVICE;
}
#endif

static INLINE int ScUid(void)
{
    return snort_conf->user_id;
}

static INLINE int ScGid(void)
{
    return snort_conf->group_id;
}

static INLINE char * ScPcapLogFile(void)
{
    return snort_conf->pcap_log_file;
}

// use of macro avoids depending on generators.h
#define EventIsInternal(gid) (gid == GENERATOR_INTERNAL)
     
static INLINE void EnableInternalEvent(RateFilterConfig *config, uint32_t sid)
{   
    if (config == NULL)
        return;

    config->internal_event_mask |= (1 << sid);
}    

static INLINE int InternalEventIsEnabled(RateFilterConfig *config, uint32_t sid)
{   
    if (config == NULL)
        return 0;

    return (config->internal_event_mask & (1 << sid));
} 

static INLINE int ScIsPreprocEnabled(uint32_t preproc_id, tSfPolicyId policy_id)
{
    SnortPolicy *policy;

    if (policy_id >= snort_conf->num_policies_allocated)
        return 0;

    policy = snort_conf->targeted_policies[policy_id];
    if (policy == NULL)
        return 0;

    if (policy->preproc_bit_mask & (1 << preproc_id))
        return 1;

    return 0;
}

static INLINE int ScDeepTeredoInspection(void)
{
    return snort_conf->enable_teredo;
}

static INLINE int ScVlanAgnostic(void)
{
    return snort_conf->vlan_agnostic;
}

static INLINE uint32_t ScSoRuleMemcap(void)
{   
    return snort_conf->so_rule_memcap;
}

#endif  /* __SNORT_H__ */

