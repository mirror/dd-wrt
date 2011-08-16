/* $Id$ */
/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2000,2001 Andrew R. Baker <andrewb@uab.edu>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <pcap.h>

#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#ifndef WIN32
# include <netdb.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <grp.h>
# include <pwd.h>
# include <fnmatch.h>
#endif /* !WIN32 */

#include "bounds.h"
#include "rules.h"
#include "treenodes.h"
#include "parser.h"
#include "plugbase.h"
#include "plugin_enum.h"
#include "debug.h"
#include "util.h"
#include "mstring.h"
#include "detect.h"
#include "fpcreate.h"
#include "log.h"
#include "generators.h"
#include "tag.h"
#include "signature.h"
#include "strlcatu.h"
#include "strlcpyu.h"
#include "sfthreshold.h"
#include "sfutil/sfthd.h"
#include "snort.h"
#include "event_queue.h"
#include "asn1.h"
#include "sfutil/sfghash.h"
#include "sp_preprocopt.h"
#include "detection-plugins/sp_icmp_type_check.h"
#include "detection-plugins/sp_ip_proto.h"
#include "detection-plugins/sp_pattern_match.h"
#include "sf_vartable.h"
#include "ipv6_port.h"
#include "sfutil/sf_ip.h"
#include "sflsq.h"
#include "ppm.h"
#include "rate_filter.h"
#include "detection_filter.h"
#include "sfPolicy.h"
#include "sfutil/mpse.h"
#include "sfutil/sfrim.h"
#include "sfutil/sfportobject.h"
#include "sfutil/strvec.h"
#include "active.h"

#ifdef TARGET_BASED
# include "sftarget_reader.h"
#endif

#ifdef DYNAMIC_PLUGIN
# include "dynamic-plugins/sp_dynamic.h"
#endif
 
/* Macros *********************************************************************/
#define ENABLE_ALL_RULES    1
#define ENABLE_RULE         1
#define ENABLE_ONE_RULE     0
#define MAX_RULE_OPTIONS     256
#define MAX_LINE_LENGTH    32768
#define MAX_IPLIST_ENTRIES  4096 
#define DEFAULT_LARGE_RULE_GROUP 9
#define SF_IPPROTO_UNKNOWN -1
#define MAX_RULE_COUNT (65535 * 2)

/* Rule list order keywords
 * This is separate from keywords because activation was used
 * instead of activate */
#define RULE_LIST_TYPE__ACTIVATION  "activation"
#define RULE_LIST_TYPE__ALERT       "alert"
#define RULE_LIST_TYPE__DROP        "drop"
#define RULE_LIST_TYPE__DYNAMIC     "dynamic"
#define RULE_LIST_TYPE__LOG         "log"
#define RULE_LIST_TYPE__PASS        "pass"
#define RULE_LIST_TYPE__REJECT     "reject"
#define RULE_LIST_TYPE__SDROP      "sdrop"

#define RULE_PROTO_OPT__IP    "ip"
#define RULE_PROTO_OPT__TCP   "tcp"
#define RULE_PROTO_OPT__UDP   "udp"
#define RULE_PROTO_OPT__ICMP  "icmp"

#define RULE_DIR_OPT__DIRECTIONAL    "->"
#define RULE_DIR_OPT__BIDIRECTIONAL  "<>"

/* For user defined rule type */
#define RULE_TYPE_OPT__TYPE    "type"

/* Rule options
 * Only the basic ones are here.  The detection options and preprocessor
 * detection options define their own */
#define RULE_OPT__ACTIVATED_BY      "activated_by"
#define RULE_OPT__ACTIVATES         "activates"
#define RULE_OPT__CLASSTYPE         "classtype"
#define RULE_OPT__COUNT             "count"
#define RULE_OPT__DETECTION_FILTER  "detection_filter"
#define RULE_OPT__GID               "gid"
#define RULE_OPT__MSG               "msg"
#define RULE_OPT__METADATA          "metadata"
#define RULE_OPT__LOGTO             "logto"
#define RULE_OPT__PRIORITY          "priority"
#define RULE_OPT__REFERENCE         "reference"
#define RULE_OPT__REVISION          "rev"
#define RULE_OPT__SID               "sid"
#define RULE_OPT__TAG               "tag"
#define RULE_OPT__THRESHOLD         "threshold"

/* Metadata rule option keys */
#define METADATA_KEY__ENGINE         "engine"
#define METADATA_KEY__OS             "os"
#define METADATA_KEY__RULE_FLUSHING  "rule-flushing"
#define METADATA_KEY__RULE_TYPE      "rule-type"
#define METADATA_KEY__SOID           "soid"
#define METADATA_KEY__SERVICE        "service"

/* Metadata rule option values */
#define METADATA_VALUE__DECODE    "decode"
#define METADATA_VALUE__DETECT    "detect"
#define METADATA_VALUE__DISABLED  "disabled"
#define METADATA_VALUE__ENABLED   "enabled"
#define METADATA_VALUE__OFF       "off"
#define METADATA_VALUE__ON        "on"
#define METADATA_VALUE__PREPROC   "preproc"
#define METADATA_VALUE__SHARED    "shared"

/* MPLS payload types */
#ifdef MPLS
# define MPLS_PAYLOAD_OPT__IPV4      "ipv4"
# define MPLS_PAYLOAD_OPT__IPV6      "ipv6"
# define MPLS_PAYLOAD_OPT__ETHERNET  "ethernet"
#endif

/* Tag options */
#define TAG_OPT__BYTES     "bytes"
#define TAG_OPT__DST       "dst"
#define TAG_OPT__HOST      "host"
#define TAG_OPT__PACKETS   "packets"
#define TAG_OPT__SECONDS   "seconds"
#define TAG_OPT__SESSION   "session"
#define TAG_OPT__SRC       "src"

/* Dynamic library specifier option values */
#ifdef DYNAMIC_PLUGIN
# define DYNAMIC_LIB_OPT__FILE       "file"
# define DYNAMIC_LIB_OPT__DIRECTORY  "directory"
#endif

/* Threshold options */
#define THRESHOLD_OPT__COUNT    "count"
#define THRESHOLD_OPT__GID      "gen_id"
#define THRESHOLD_OPT__IP       "ip"
#define THRESHOLD_OPT__SECONDS  "seconds"
#define THRESHOLD_OPT__SID      "sig_id"
#define THRESHOLD_OPT__TRACK    "track"
#define THRESHOLD_OPT__TYPE     "type"

#define THRESHOLD_TYPE__BOTH       "both"
#define THRESHOLD_TYPE__LIMIT      "limit"
#define THRESHOLD_TYPE__THRESHOLD  "threshold"

#define THRESHOLD_TRACK__BY_DST    "by_dst"
#define THRESHOLD_TRACK__BY_SRC    "by_src"

#define RULE_STATE_OPT__DISABLED   "disabled"
#define RULE_STATE_OPT__ENABLED    "enabled"

#define DETECTION_OPT__BLEEDOVER_PORT_LIMIT                  "bleedover-port-limit"
#define DETECTION_OPT__BLEEDOVER_WARNINGS_ENABLED            "bleedover-warnings-enabled"
#define DETECTION_OPT__DEBUG                                 "debug"
#define DETECTION_OPT__DEBUG_PRINT_NOCONTENT_RULE_TESTS      "debug-print-nocontent-rule-tests"
#define DETECTION_OPT__DEBUG_PRINT_RULE_GROUP_BUILD_DETAILS  "debug-print-rule-group-build-details"
#define DETECTION_OPT__DEBUG_PRINT_RULE_GROUPS_UNCOMPILED    "debug-print-rule-groups-uncompiled"
#define DETECTION_OPT__DEBUG_PRINT_RULE_GROUPS_COMPILED      "debug-print-rule-groups-compiled"
#define DETECTION_OPT__ENABLE_SINGLE_RULE_GROUP              "enable-single-rule-group"
#define DETECTION_OPT__MAX_QUEUE_EVENTS                      "max_queue_events"
#define DETECTION_OPT__NO_STREAM_INSERTS                     "no_stream_inserts"
#define DETECTION_OPT__SEARCH_METHOD                         "search-method"
#define DETECTION_OPT__SEARCH_OPTIMIZE                       "search-optimize"
#define DETECTION_OPT__SPLIT_ANY_ANY                         "split-any-any"
#define DETECTION_OPT__MAX_PATTERN_LEN                       "max-pattern-len"
#define DETECTION_OPT__DEBUG_PRINT_FAST_PATTERN              "debug-print-fast-pattern"

#define EVENT_QUEUE_OPT__LOG                 "log"
#define EVENT_QUEUE_OPT__MAX_QUEUE           "max_queue"
#define EVENT_QUEUE_OPT__ORDER_EVENTS        "order_events"
#define EVENT_QUEUE_OPT__PROCESS_ALL_EVENTS  "process_all_events"

#define ORDER_EVENTS_OPT__CONTENT_LENGTH   "content_length"
#define ORDER_EVENTS_OPT__PRIORITY         "priority"

#define THRESHOLD_OPT__MEMCAP   "memcap"

#define CHECKSUM_MODE_OPT__ALL      "all"
#define CHECKSUM_MODE_OPT__NONE     "none"
#define CHECKSUM_MODE_OPT__IP       "ip"
#define CHECKSUM_MODE_OPT__NO_IP    "noip"
#define CHECKSUM_MODE_OPT__TCP      "tcp"
#define CHECKSUM_MODE_OPT__NO_TCP   "notcp"
#define CHECKSUM_MODE_OPT__UDP      "udp"
#define CHECKSUM_MODE_OPT__NO_UDP   "noudp"
#define CHECKSUM_MODE_OPT__ICMP     "icmp"
#define CHECKSUM_MODE_OPT__NO_ICMP  "noicmp"

#define RULE_STATE_OPT__DISABLED  "disabled"
#define RULE_STATE_OPT__ENABLED   "enabled"

#define POLICY_MODE_PASSIVE     "tap"
#define POLICY_MODE_INLINE      "inline"
#define POLICY_MODE_INLINE_TEST "inline_test"

#ifdef PERF_PROFILING
# define PROFILE_OPT__FILENAME                "filename"
# define PROFILE_OPT__PRINT                   "print"
# define PROFILE_OPT__ALL                     "all"
# define PROFILE_OPT__SORT                    "sort"
# define PROFILE_OPT__CHECKS                  "checks"
# define PROFILE_OPT__AVG_TICKS               "avg_ticks"
# define PROFILE_OPT__TOTAL_TICKS             "total_ticks"
# define PROFILE_OPT__MATCHES                 "matches"
# define PROFILE_OPT__NO_MATCHES              "nomatches"
# define PROFILE_OPT__AVG_TICKS_PER_MATCH     "avg_ticks_per_match"
# define PROFILE_OPT__AVG_TICKS_PER_NO_MATCH  "avg_ticks_per_nomatch"
# define PROFILE_OPT__APPEND                  "append"
#endif

#ifdef PPM_MGR
# define PPM_OPT__MAX_PKT_TIME         "max-pkt-time"
# define PPM_OPT__MAX_RULE_TIME        "max-rule-time"
# define PPM_OPT__SUSPEND_TIMEOUT      "suspend-timeout"
# define PPM_OPT__SUSPEND_EXP_RULES    "suspend-expensive-rules"
# define PPM_OPT__THRESHOLD            "threshold"
# define PPM_OPT__FAST_PATH_EXP_PKTS   "fastpath-expensive-packets"
# define PPM_OPT__PKT_LOG              "pkt-log"
# define PPM_OPT__RULE_LOG             "rule-log"
# define PPM_OPT__ALERT                "alert"
# define PPM_OPT__LOG                  "log"
# define PPM_OPT__DEBUG_PKTS           "debug-pkts"
#endif

#ifdef ACTIVE_RESPONSE
#define RESPONSE_OPT__ATTEMPTS  "attempts"
#define RESPONSE_OPT__DEVICE    "device"
#endif

#define ERR_PAIR_COUNT \
    "%s has incorrect argument count; should be %d pairs.", ERR_KEY
#define ERR_NOT_PAIRED \
    "%s is missing an option or argument to go with: %s.", ERR_KEY, pairs[0]
#define ERR_EXTRA_OPTION \
    "%s has extra option of type: %s.", ERR_KEY, pairs[0]
#define ERR_BAD_OPTION \
    "%s has unknown option: %s.", ERR_KEY, pairs[0]
#define ERR_BAD_VALUE \
    "%s has unknown %s: %s.", ERR_KEY, pairs[0], pairs[1]
#define ERR_BAD_ARG_COUNT \
    "%s has incorrect argument count.", ERR_KEY
#define ERR_CREATE \
    "%s could not be created.", ERR_KEY
#define ERR_CREATE_EX \
    "%s could not be created: %s.", ERR_KEY

// arbitrary conf file name used to allow initialization w/o conf
// (required for packet logging mode)
#define NULL_CONF "null"

/* Data types *****************************************************************/
typedef void (*ParseFunc)(SnortConfig *, SnortPolicy *, char *);
typedef void (*ParseConfigFunc)(SnortConfig *, char *);
typedef void (*ParseRuleOptFunc)(SnortConfig *, RuleTreeNode *, OptTreeNode *, RuleType, char *);

/* Used to determine whether or not to parse the keyword line based on
 * whether or not we're parsing rules */
typedef enum _KeywordType
{
    KEYWORD_TYPE__MAIN,
    KEYWORD_TYPE__RULE,
    KEYWORD_TYPE__ALL

} KeywordType;

typedef enum _VarType
{
    VAR_TYPE__DEFAULT,
    VAR_TYPE__PORTVAR,
    VAR_TYPE__IPVAR

} VarType;

typedef struct _KeywordFunc
{
    char *name;
    KeywordType type;
    int expand_vars;
    int default_policy_only; 
    ParseFunc parse_func;

} KeywordFunc;

typedef struct _RuleOptFunc
{
    char *name;
    int args_required;
    int only_once;
    ParseRuleOptFunc parse_func;

} RuleOptFunc;

typedef struct _ConfigFunc
{
    char *name;
    int args_required;
    int only_once;
    int default_policy_only; 
    ParseConfigFunc parse_func;

} ConfigFunc;

/* Tracking the port_list_t structure for printing and debugging at
 * this point...temporarily... */
typedef struct
{
    int rule_type;
    int proto;
    int icmp_type;
    int ip_proto;
    char *protocol;
    char *src_port;
    char *dst_port;
    unsigned int gid;
    unsigned int sid;
    int dir;
    char content;
    char uricontent;

} port_entry_t;

typedef struct
{
    int pl_max;
    int pl_cnt;
    port_entry_t pl_array[MAX_RULE_COUNT];

} port_list_t;

/* rule counts for port lists */
typedef struct 
{
    int src;
    int dst;
    int aa;  /* any-any */
    int sd;  /* src+dst ports specified */
    int nc;  /* no content */

} rule_count_t;


/* Externs ********************************************************************/
extern VarNode *cmd_line_var_list;
extern char *snort_conf_file;
extern char *snort_conf_dir;
extern RuleOptConfigFuncNode *rule_opt_config_funcs;
extern unsigned int giFlowbitSize;

/* Globals ********************************************************************/

/* Used when a user defines a new rule type (ruletype keyword)
 * It points to the new rule type's ListHead and is used for accessing the
 * rule type's AlertList and LogList.
 * The output plugins used for the rule type need to be attached to the new
 * rule type's list head's AlertList or LogList.  It's set before calling
 * the output plugin's initialization routine, because in that routine,
 * AddFuncToOutputList is called (plugbase.c) and there, the output function
 * is attached to the new rule type's appropriate list.
 * NOTE:  This variable MUST NOT be used during runtime */
ListHead *head_tmp = NULL;

/* Set to the current snort config we're parsing.  Mostly used for the
 * plugins (output and preprocessor) since the callbacks don't pass a
 * snort configuration */
SnortConfig *snort_conf_for_parsing = NULL;

char *file_name = NULL;   /* current config file being processed */
int file_line = 0;        /* current line being processed in the file */

/* Main parsing function uses this to indicate whether or not
 * rules are to be parsed. */
static int parse_rules = 0;

/* Used when parsing rule options and a threshold is used.  Need to add
 * sid and gid to threshold object and they might not have been
 * parsed yet. */
static THDX_STRUCT *thdx_tmp = NULL;

int rule_count = 0;          /* number of rules generated */
int detect_rule_count = 0;   /* number of detection rules generated */
int decode_rule_count = 0;   /* number of decoder rules generated */
int preproc_rule_count = 0;  /* number of preprocessor rules generated */
int head_count = 0;          /* number of header blocks (chain heads?) */
int otn_count = 0;           /* number of chains */
int dynamic_rule_count = 0;

static port_list_t port_list;

static rule_count_t tcpCnt;
static rule_count_t udpCnt;
static rule_count_t icmpCnt;
static rule_count_t ipCnt;

rule_index_map_t *ruleIndexMap = NULL;   /* rule index -> sid:gid map */

static tSfPolicyId currHeadNodePolicy = 0;
static OptTreeNode *currHeadNodeOtn  = NULL;


static void ParseActivate(SnortConfig *, SnortPolicy *, char *);
static void ParseAlert(SnortConfig *, SnortPolicy *, char *);
static void ParseDrop(SnortConfig *, SnortPolicy *, char *);
static void ParseDynamic(SnortConfig *, SnortPolicy *, char *);
static void ParseLog(SnortConfig *, SnortPolicy *, char *);
static void ParsePass(SnortConfig *, SnortPolicy *, char *);
static void ParseReject(SnortConfig *, SnortPolicy *, char *);
static void ParseSdrop(SnortConfig *, SnortPolicy *, char *);

#ifdef TARGET_BASED
static void ParseAttributeTable(SnortConfig *, SnortPolicy *, char *);
#endif  /* TARGET_BASED */
static void ParseConfig(SnortConfig *, SnortPolicy *, char *);
#ifdef DYNAMIC_PLUGIN
static void ParseDynamicDetectionInfo(SnortConfig *, SnortPolicy *, char *);
static void ParseDynamicEngineInfo(SnortConfig *, SnortPolicy *, char *);
static void ParseDynamicPreprocessorInfo(SnortConfig *, SnortPolicy *, char *);
#endif  /* DYNAMIC_PLUGIN */
static void ParseEventFilter(SnortConfig *, SnortPolicy *, char *);
static void ParseInclude(SnortConfig *, SnortPolicy *, char *);
static void ParseIpVar(SnortConfig *, SnortPolicy *, char *);
static void ParsePortVar(SnortConfig *, SnortPolicy *, char *);
static void ParsePreprocessor(SnortConfig *, SnortPolicy *, char *);
static void ParseRateFilter(SnortConfig *, SnortPolicy *, char *);
static void ParseRuleState(SnortConfig *, SnortPolicy *, char *);
static void ParseRuleTypeDeclaration(SnortConfig *, SnortPolicy *, char *);
static void ParseSuppress(SnortConfig *, SnortPolicy *, char *);
static void ParseThreshold(SnortConfig *, SnortPolicy *, char *);
static void ParseVar(SnortConfig *, SnortPolicy *, char *);
static void AddVarToTable(SnortConfig *, char *, char *);

static const KeywordFunc snort_conf_keywords[] =
{
    /* Rule keywords */
    { SNORT_CONF_KEYWORD__ACTIVATE, KEYWORD_TYPE__RULE, 0, 0, ParseActivate },
    { SNORT_CONF_KEYWORD__ALERT,    KEYWORD_TYPE__RULE, 0, 0, ParseAlert },
    { SNORT_CONF_KEYWORD__DROP,     KEYWORD_TYPE__RULE, 0, 0, ParseDrop },
    { SNORT_CONF_KEYWORD__BLOCK,    KEYWORD_TYPE__RULE, 0, 0, ParseDrop },
    { SNORT_CONF_KEYWORD__DYNAMIC,  KEYWORD_TYPE__RULE, 0, 0, ParseDynamic },
    { SNORT_CONF_KEYWORD__LOG,      KEYWORD_TYPE__RULE, 0, 0, ParseLog },
    { SNORT_CONF_KEYWORD__PASS,     KEYWORD_TYPE__RULE, 0, 0, ParsePass },
    { SNORT_CONF_KEYWORD__REJECT,   KEYWORD_TYPE__RULE, 0, 0, ParseReject },
    { SNORT_CONF_KEYWORD__SDROP,    KEYWORD_TYPE__RULE, 0, 0, ParseSdrop },
    { SNORT_CONF_KEYWORD__SBLOCK,   KEYWORD_TYPE__RULE, 0, 0, ParseSdrop },

    /* Non-rule keywords */
#ifdef TARGET_BASED
    /* Need to fatal error if attribute_table is not configured in default
     * policy.  Since we're just skipping configuring non-default
     * configurations with default only configuration types, set this to
     * be configured for non-default policies and fatal in function */
    { SNORT_CONF_KEYWORD__ATTRIBUTE_TABLE,   KEYWORD_TYPE__MAIN, 1, 0, ParseAttributeTable },
#endif
    { SNORT_CONF_KEYWORD__CONFIG,            KEYWORD_TYPE__MAIN, 1, 0, ParseConfig },
#ifdef DYNAMIC_PLUGIN
    { SNORT_CONF_KEYWORD__DYNAMIC_DETECTION, KEYWORD_TYPE__MAIN, 1, 1, ParseDynamicDetectionInfo },
    { SNORT_CONF_KEYWORD__DYNAMIC_ENGINE,    KEYWORD_TYPE__MAIN, 1, 1, ParseDynamicEngineInfo },
    { SNORT_CONF_KEYWORD__DYNAMIC_PREPROC,   KEYWORD_TYPE__MAIN, 1, 1, ParseDynamicPreprocessorInfo },
#endif
    { SNORT_CONF_KEYWORD__EVENT_FILTER,      KEYWORD_TYPE__MAIN, 1, 0, ParseEventFilter },
    { SNORT_CONF_KEYWORD__INCLUDE,           KEYWORD_TYPE__ALL,  1, 0, ParseInclude },
    { SNORT_CONF_KEYWORD__IPVAR,             KEYWORD_TYPE__MAIN, 0, 0, ParseIpVar },
    { SNORT_CONF_KEYWORD__OUTPUT,            KEYWORD_TYPE__MAIN, 1, 1, ParseOutput },
    { SNORT_CONF_KEYWORD__PORTVAR,           KEYWORD_TYPE__MAIN, 0, 0, ParsePortVar },
    { SNORT_CONF_KEYWORD__PREPROCESSOR,      KEYWORD_TYPE__MAIN, 1, 0, ParsePreprocessor },
    { SNORT_CONF_KEYWORD__RATE_FILTER,       KEYWORD_TYPE__MAIN, 0, 0, ParseRateFilter },
    { SNORT_CONF_KEYWORD__RULE_STATE,        KEYWORD_TYPE__MAIN, 1, 0, ParseRuleState },
    { SNORT_CONF_KEYWORD__RULE_TYPE,         KEYWORD_TYPE__ALL,  1, 0, ParseRuleTypeDeclaration },
    { SNORT_CONF_KEYWORD__SUPPRESS,          KEYWORD_TYPE__MAIN, 0, 0, ParseSuppress },
    { SNORT_CONF_KEYWORD__THRESHOLD,         KEYWORD_TYPE__MAIN, 1, 0, ParseThreshold },
    { SNORT_CONF_KEYWORD__VAR,               KEYWORD_TYPE__MAIN, 0, 0, ParseVar },

    { NULL, KEYWORD_TYPE__ALL, 0, 0, NULL }   /* Marks end of array */
};

static void ParseOtnActivatedBy(SnortConfig *, RuleTreeNode *,
                                OptTreeNode *, RuleType, char *);
static void ParseOtnActivates(SnortConfig *, RuleTreeNode *,
                              OptTreeNode *, RuleType, char *);
static void ParseOtnClassType(SnortConfig *, RuleTreeNode *,
                              OptTreeNode *, RuleType, char *);
static void ParseOtnCount(SnortConfig *, RuleTreeNode *,
                          OptTreeNode *, RuleType, char *);
static void ParseOtnDetectionFilter(SnortConfig *, RuleTreeNode *,
                                    OptTreeNode *, RuleType, char *);
static void ParseOtnGid(SnortConfig *, RuleTreeNode *,
                        OptTreeNode *, RuleType, char *);
static void ParseOtnLogTo(SnortConfig *, RuleTreeNode *,
                          OptTreeNode *, RuleType, char *);
static void ParseOtnMessage(SnortConfig *, RuleTreeNode *,
                            OptTreeNode *, RuleType, char *);
static void ParseOtnMetadata(SnortConfig *, RuleTreeNode *,
                             OptTreeNode *, RuleType, char *);
static void ParseOtnPriority(SnortConfig *, RuleTreeNode *,
                             OptTreeNode *, RuleType, char *);
static void ParseOtnReference(SnortConfig *, RuleTreeNode *,
                              OptTreeNode *, RuleType, char *);
static void ParseOtnRevision(SnortConfig *, RuleTreeNode *,
                             OptTreeNode *, RuleType, char *);
static void ParseOtnSid(SnortConfig *, RuleTreeNode *,
                        OptTreeNode *, RuleType, char *);
static void ParseOtnTag(SnortConfig *, RuleTreeNode *,
                        OptTreeNode *, RuleType, char *);
static void ParseOtnThreshold(SnortConfig *, RuleTreeNode *,
                              OptTreeNode *, RuleType, char *);

static const RuleOptFunc rule_options[] =
{
    { RULE_OPT__ACTIVATED_BY,     1, 1, ParseOtnActivatedBy },
    { RULE_OPT__ACTIVATES,        1, 1, ParseOtnActivates },
    { RULE_OPT__CLASSTYPE,        1, 1, ParseOtnClassType },
    { RULE_OPT__COUNT,            1, 1, ParseOtnCount },
    { RULE_OPT__DETECTION_FILTER, 1, 1, ParseOtnDetectionFilter },
    { RULE_OPT__GID,              1, 1, ParseOtnGid },
    { RULE_OPT__LOGTO,            1, 1, ParseOtnLogTo },
    { RULE_OPT__METADATA,         1, 0, ParseOtnMetadata },
    { RULE_OPT__MSG,              1, 1, ParseOtnMessage },
    { RULE_OPT__PRIORITY,         1, 1, ParseOtnPriority },
    { RULE_OPT__REFERENCE,        1, 0, ParseOtnReference },
    { RULE_OPT__REVISION,         1, 1, ParseOtnRevision },
    { RULE_OPT__SID,              1, 1, ParseOtnSid },
    { RULE_OPT__TAG,              1, 1, ParseOtnTag },
    { RULE_OPT__THRESHOLD,        1, 1, ParseOtnThreshold },

    { NULL, 0, 0, NULL }   /* Marks end of array */
};

#ifdef PERF_PROFILING
/* Internal prototypes used in lists below */
static void _ConfigProfilePreprocs(SnortConfig *, char *);
static void _ConfigProfileRules(SnortConfig *, char *);
#endif

static const ConfigFunc config_opts[] =
{
    { CONFIG_OPT__ALERT_FILE, 1, 1, 1, ConfigAlertFile },
    { CONFIG_OPT__ALERT_WITH_IFACE_NAME, 0, 1, 1, ConfigAlertWithInterfaceName },
#ifdef PREPROCESSOR_AND_DECODER_RULE_EVENTS
    { CONFIG_OPT__AUTOGEN_PREPROC_DECODER_RULES, 0, 1, 0, ConfigAutogenPreprocDecoderRules },
#endif
    { CONFIG_OPT__ASN1, 1, 1, 1, ConfigAsn1 },
    { CONFIG_OPT__BINDING, 1, 0, 1, ConfigBinding },
    { CONFIG_OPT__BPF_FILE, 1, 1, 1, ConfigBpfFile },
    { CONFIG_OPT__CHECKSUM_DROP, 0, 0, 0, ConfigChecksumDrop },
    { CONFIG_OPT__CHECKSUM_MODE, 0, 0, 0, ConfigChecksumMode },
    { CONFIG_OPT__CHROOT_DIR, 1, 1, 1, ConfigChrootDir },
    { CONFIG_OPT__CLASSIFICATION, 1, 0, 0, ConfigClassification },
    { CONFIG_OPT__DAEMON, 0, 1, 1, ConfigDaemon },
    { CONFIG_OPT__DECODE_DATA_LINK, 0, 1, 1, ConfigDecodeDataLink },
    { CONFIG_OPT__DEFAULT_RULE_STATE, 0, 1, 1, ConfigDefaultRuleState },
    { CONFIG_OPT__DETECTION, 1, 0, 1, ConfigDetection },  /* This is reconfigurable */
    { CONFIG_OPT__DETECTION_FILTER, 1, 1, 1, ConfigDetectionFilter },
    { CONFIG_OPT__DISABLE_DECODE_ALERTS, 0, 1, 1, ConfigDisableDecodeAlerts },
    { CONFIG_OPT__DISABLE_DECODE_DROPS, 0, 1, 1, ConfigDisableDecodeDrops },
#ifdef INLINE_FAILOPEN
    { CONFIG_OPT__DISABLE_INLINE_FAILOPEN, 0, 1, 1, ConfigDisableInlineFailopen },
#endif
    { CONFIG_OPT__DISABLE_IP_OPT_ALERTS, 0, 1, 1, ConfigDisableIpOptAlerts },
    { CONFIG_OPT__DISABLE_IP_OPT_DROPS, 0, 1, 1, ConfigDisableIpOptDrops },
    { CONFIG_OPT__DISABLE_TCP_OPT_ALERTS, 0, 1, 1, ConfigDisableTcpOptAlerts },
    { CONFIG_OPT__DISABLE_TCP_OPT_DROPS, 0, 1, 1, ConfigDisableTcpOptDrops },
    { CONFIG_OPT__DISABLE_TCP_OPT_EXP_ALERTS, 0, 1, 1, ConfigDisableTcpOptExperimentalAlerts },
    { CONFIG_OPT__DISABLE_TCP_OPT_EXP_DROPS, 0, 1, 1, ConfigDisableTcpOptExperimentalDrops },
    { CONFIG_OPT__DISABLE_TCP_OPT_OBS_ALERTS, 0, 1, 1, ConfigDisableTcpOptObsoleteAlerts },
    { CONFIG_OPT__DISABLE_TCP_OPT_OBS_DROPS, 0, 1, 1, ConfigDisableTcpOptObsoleteDrops },
    { CONFIG_OPT__DISABLE_TTCP_ALERTS, 0, 1, 1, ConfigDisableTTcpAlerts },
    { CONFIG_OPT__DISABLE_TCP_OPT_TTCP_ALERTS, 0, 1, 1, ConfigDisableTTcpAlerts },
    { CONFIG_OPT__DISABLE_TTCP_DROPS, 0, 1, 1, ConfigDisableTTcpDrops },
    { CONFIG_OPT__DUMP_CHARS_ONLY, 0, 1, 1, ConfigDumpCharsOnly },
    { CONFIG_OPT__DUMP_PAYLOAD, 0, 1, 1, ConfigDumpPayload },
    { CONFIG_OPT__DUMP_PAYLOAD_VERBOSE, 0, 1, 1, ConfigDumpPayloadVerbose },
    { CONFIG_OPT__ENABLE_DECODE_DROPS, 0, 1, 1, ConfigEnableDecodeDrops },
    { CONFIG_OPT__ENABLE_DECODE_OVERSIZED_ALERTS, 0, 1, 1, ConfigEnableDecodeOversizedAlerts },
    { CONFIG_OPT__ENABLE_DECODE_OVERSIZED_DROPS, 0, 1, 1, ConfigEnableDecodeOversizedDrops },
    { CONFIG_OPT__ENABLE_DEEP_TEREDO_INSPECTION, 0, 1, 1, ConfigEnableDeepTeredoInspection },
    { CONFIG_OPT__ENABLE_IP_OPT_DROPS, 0, 1, 1, ConfigEnableIpOptDrops },
#ifdef MPLS
    { CONFIG_OPT__ENABLE_MPLS_MULTICAST, 0, 1, 1, ConfigEnableMplsMulticast },
    { CONFIG_OPT__ENABLE_MPLS_OVERLAPPING_IP, 0, 1, 1, ConfigEnableMplsOverlappingIp },
#endif
    { CONFIG_OPT__ENABLE_TCP_OPT_DROPS, 0, 1, 1, ConfigEnableTcpOptDrops },
    { CONFIG_OPT__ENABLE_TCP_OPT_EXP_DROPS, 0, 1, 1, ConfigEnableTcpOptExperimentalDrops },
    { CONFIG_OPT__ENABLE_TCP_OPT_OBS_DROPS, 0, 1, 1, ConfigEnableTcpOptObsoleteDrops },
    { CONFIG_OPT__ENABLE_TTCP_DROPS, 0, 1, 1, ConfigEnableTTcpDrops },
    { CONFIG_OPT__ENABLE_TCP_OPT_TTCP_DROPS, 0, 1, 1, ConfigEnableTTcpDrops },
    { CONFIG_OPT__EVENT_FILTER, 1, 1, 1, ConfigEventFilter },
    { CONFIG_OPT__EVENT_QUEUE, 1, 1, 1, ConfigEventQueue },
    { CONFIG_OPT__REACT, 1, 1, 1, ConfigReact },
#ifdef ENABLE_RESPONSE3
    { CONFIG_OPT__FLEXRESP2_INTERFACE, 1, 1, 1, ConfigFlexresp2Interface },
    { CONFIG_OPT__FLEXRESP2_ATTEMPTS, 1, 1, 1, ConfigFlexresp2Attempts },
    { CONFIG_OPT__FLEXRESP2_MEMCAP, 1, 1, 1, ConfigFlexresp2Memcap },
    { CONFIG_OPT__FLEXRESP2_ROWS, 1, 1, 1, ConfigFlexresp2Rows },
#endif
#ifdef ACTIVE_RESPONSE
    { CONFIG_OPT__RESPONSE, 1, 1, 1, ConfigResponse },
#endif
    { CONFIG_OPT__FLOWBITS_SIZE, 1, 1, 1, ConfigFlowbitsSize },
    { CONFIG_OPT__IGNORE_PORTS, 1, 0, 1, ConfigIgnorePorts },
    { CONFIG_OPT__ALERT_VLAN, 0, 1, 1, ConfigIncludeVlanInAlert },
    { CONFIG_OPT__INTERFACE, 1, 1, 1, ConfigInterface },
    { CONFIG_OPT__IPV6_FRAG, 1, 1, 1, ConfigIpv6Frag },
    { CONFIG_OPT__LAYER2RESETS, 1, 1, 1, ConfigLayer2Resets },
    { CONFIG_OPT__LOG_DIR, 1, 1, 1, ConfigLogDir },
    { CONFIG_OPT__DAQ_TYPE, 1, 1, 1, ConfigDaqType },
    { CONFIG_OPT__DAQ_MODE, 1, 1, 1, ConfigDaqMode },
    { CONFIG_OPT__DAQ_VAR, 1, 0, 1, ConfigDaqVar },
    { CONFIG_OPT__DAQ_DIR, 1, 0, 1, ConfigDaqDir },
    { CONFIG_OPT__DIRTY_PIG, 0, 1, 1, ConfigDirtyPig },
#ifdef TARGET_BASED
    { CONFIG_OPT__MAX_ATTRIBUTE_HOSTS, 1, 1, 1, ConfigMaxAttributeHosts },
    { CONFIG_OPT__MAX_METADATA_SERVICES, 1, 1, 1, ConfigMaxMetadataServices },
#endif
#ifdef MPLS
    { CONFIG_OPT__MAX_MPLS_LABELCHAIN_LEN, 0, 1, 1, ConfigMaxMplsLabelChain },
    { CONFIG_OPT__MPLS_PAYLOAD_TYPE, 0, 1, 1, ConfigMplsPayloadType },
#endif
    { CONFIG_OPT__MIN_TTL, 1, 1, 0, ConfigMinTTL },
#ifdef NORMALIZER
    { CONFIG_OPT__NEW_TTL, 1, 1, 0, ConfigNewTTL },
#endif
    { CONFIG_OPT__NO_LOG, 0, 1, 1, ConfigNoLog },
    { CONFIG_OPT__NO_PCRE, 0, 1, 1, ConfigNoPcre },
    { CONFIG_OPT__NO_PROMISCUOUS, 0, 1, 1, ConfigNoPromiscuous },
    { CONFIG_OPT__OBFUSCATE, 0, 1, 1, ConfigObfuscate },
    { CONFIG_OPT__ORDER, 1, 1, 1, ConfigRuleListOrder },
    { CONFIG_OPT__PKT_COUNT, 1, 1, 1, ConfigPacketCount },
    { CONFIG_OPT__PKT_SNAPLEN, 1, 1, 1, ConfigPacketSnaplen },
    { CONFIG_OPT__PCRE_MATCH_LIMIT, 1, 1, 1, ConfigPcreMatchLimit },
    { CONFIG_OPT__PCRE_MATCH_LIMIT_RECURSION, 1, 1, 1, ConfigPcreMatchLimitRecursion },
    /* XXX We can configure this on the command line - why not in config file ??? */
#ifdef NOT_UNTIL_WE_DAEMONIZE_AFTER_READING_CONFFILE
    { CONFIG_OPT__PID_PATH, 1, 1, 1, ConfigPidPath },
#endif
    { CONFIG_OPT__POLICY, 1, 1, 0, ConfigPolicy },
    { CONFIG_OPT__POLICY_MODE, 1, 1, 0, ConfigPolicyMode },
    { CONFIG_OPT__POLICY_VERSION , 1, 0, 0, ConfigPolicyVersion },
#ifdef PPM_MGR
    { CONFIG_OPT__PPM, 1, 0, 1, ConfigPPM },
#endif
#ifdef PERF_PROFILING
    { CONFIG_OPT__PROFILE_PREPROCS, 0, 1, 1, _ConfigProfilePreprocs },
    { CONFIG_OPT__PROFILE_RULES, 0, 1, 1, _ConfigProfileRules },
#endif
    { CONFIG_OPT__QUIET, 0, 1, 1, ConfigQuiet },
    { CONFIG_OPT__RATE_FILTER, 1, 1, 1, ConfigRateFilter },
    { CONFIG_OPT__READ_BIN_FILE, 1, 1, 1, ConfigReadPcapFile },
    { CONFIG_OPT__REFERENCE, 1, 0, 1, ConfigReference },
    { CONFIG_OPT__REFERENCE_NET, 1, 1, 1, ConfigReferenceNet },
    { CONFIG_OPT__SET_GID, 1, 1, 1, ConfigSetGid },
    { CONFIG_OPT__SET_UID, 1, 1, 1, ConfigSetUid },
    { CONFIG_OPT__SHOW_YEAR, 0, 1, 1, ConfigShowYear },
    { CONFIG_OPT__SO_RULE_MEMCAP, 1, 1, 1, ConfigSoRuleMemcap },
    { CONFIG_OPT__STATEFUL, 0, 1, 1, ConfigStateful },
    { CONFIG_OPT__TAGGED_PACKET_LIMIT, 1, 1, 1, ConfigTaggedPacketLimit },
    { CONFIG_OPT__THRESHOLD, 1, 1, 1, ConfigThreshold },
    { CONFIG_OPT__UMASK, 1, 1, 1, ConfigUmask },
    { CONFIG_OPT__UTC, 0, 1, 1, ConfigUtc },
    { CONFIG_OPT__VERBOSE, 0, 1, 1, ConfigVerbose },
    { CONFIG_OPT__VLAN_AGNOSTIC, 0, 1, 1, ConfigVlanAgnostic },
#ifdef DYNAMIC_PLUGIN
    { CONFIG_OPT__DUMP_DYNAMIC_RULES_PATH, 1, 1, 1, ConfigDumpDynamicRulesPath },
#endif
    { NULL, 0, 0, 0, NULL }   /* Marks end of array */
};

/* Used to determine if a config option has already been configured
 * Gets zeroed when initially parsing a configuration file, then each 
 * index gets set to 1 as an option is configured.  Maps to config_opts */
static uint8_t config_opt_configured[sizeof(config_opts) / sizeof(ConfigFunc)];


/* Private function prototypes ************************************************/
static void InitVarTables(SnortPolicy *);
static void InitPolicyMode(SnortPolicy *);
static void InitParser(void);
#ifdef SUP_IP6
static int VarIsIpAddr(vartable_t *, char *);
#endif
static RuleType GetRuleType(char *);
static void CreateDefaultRules(SnortConfig *);
static void ParseConfigFile(SnortConfig *, SnortPolicy *, char *);
static int ValidateUserDefinedRuleType(SnortConfig *, char *);
static void IntegrityCheckRules(SnortConfig *);
#ifdef DYNAMIC_PLUGIN
static void ParseDynamicLibInfo(DynamicLibInfo *, char *);
#endif
static int GetRuleProtocol(char *);
static RuleTreeNode * ProcessHeadNode(SnortConfig *, RuleTreeNode *, ListHead *);
static int ContinuationCheck(char *);
static ListHead * CreateRuleType(SnortConfig *, char *, RuleType, int, ListHead *);
static int GetChecksumFlags(char *);
static int GetPolicyMode(char *,int );
static void LinkDynamicRules(SnortConfig *);
static void OtnInit(SnortConfig *);
static void _ParseRuleTypeDeclaration(SnortConfig *, FILE *, char *, int);
static void printRuleListOrder(RuleListNode *);
static RuleListNode * addNodeToOrderedList(RuleListNode *, RuleListNode *, int);
static VarEntry * VarDefine(SnortConfig *, char *, char *);
static char * VarSearch(SnortConfig *, char *);
static char * ExpandVars(SnortConfig *, char *);
static VarEntry * VarAlloc(void);
static int ParsePort(char *, uint16_t *, uint16_t *, char *, int *);
static uint16_t ConvPort(char *, char *);
static char * ReadLine(FILE *);
static void DeleteVars(VarEntry *);
static int ValidateIPList(IpAddrSet *, char *);
#ifndef SUP_IP6
static int CompareIPAddrSets(IpAddrSet *, IpAddrSet *);
static int CompareIPLists(IpAddrNode *, IpAddrNode *);
#endif
static int PortVarDefine(SnortConfig *, char *, char *);
static void port_entry_free(port_entry_t *);
static int port_list_add_entry(port_list_t *, port_entry_t *);
#if 0
static port_entry_t * port_list_get(port_list_t *, int);
static void port_list_print(port_list_t *);
#endif
static void port_list_free(port_list_t *);
static void finish_portlist_table(FastPatternConfig *, char *, PortTable *);
static void PortTablesFinish(rule_port_tables_t *, FastPatternConfig *);
static rule_port_tables_t * PortTablesNew(void);
static int FinishPortListRule(rule_port_tables_t *, RuleTreeNode *, OptTreeNode *,
                              int, port_entry_t *, FastPatternConfig *);
static PortObject * ParsePortListTcpUdpPort(PortVarTable *, PortTable *, char *);
static int ParsePortList(RuleTreeNode *, PortVarTable *, PortTable *, char *, int, int);
static void ParseRule(SnortConfig *, SnortPolicy *, char *, RuleType, ListHead *);
static void TransferOutputConfigs(OutputConfig *, OutputConfig **);
static OutputConfig * DupOutputConfig(OutputConfig *);
static void RemoveOutputConfigs(OutputConfig **, int);
static void XferHeader(RuleTreeNode *, RuleTreeNode *);
static OptTreeNode * ParseRuleOptions(SnortConfig *, RuleTreeNode *,
                                      char *, RuleType, int);
#ifndef SOURCEFIRE
static void DefineAllIfaceVars(SnortConfig *);
static void DefineIfaceVar(SnortConfig *, char *, uint8_t *, uint8_t *);
#endif

#ifdef DEBUG
#if 0
static void DumpList(IpAddrNode *, int);
#endif
static void DumpChain(char *, int, int);
static void DumpRuleChains(RuleListNode *);
#endif
static void SetLinks(SnortConfig *, int);
static int ProcessIP(SnortConfig *, char *, RuleTreeNode *, int, int);
#ifndef SUP_IP6
static int ProcessIpList(SnortConfig *, char *, RuleTreeNode *, int, int);
#endif
static int TestHeader(RuleTreeNode *, RuleTreeNode *);
static void FreeRuleTreeNode(RuleTreeNode *rtn);
static void DestroyRuleTreeNode(RuleTreeNode *rtn);
static void AddrToFunc(RuleTreeNode *, int);
static void PortToFunc(RuleTreeNode *, int, int, int);
static void SetupRTNFuncList(RuleTreeNode *);
static void DisallowCrossTableDuplicateVars(SnortConfig *, char *, VarType);
static int mergeDuplicateOtn(SnortConfig *, OptTreeNode *, OptTreeNode *, RuleTreeNode *);
#if 0
#ifdef DEBUG
static void PrintRtnPorts(RuleTreeNode *);
#endif
#endif

static void FreeRuleTreeNodes(SnortConfig *);
static void FreeOutputLists(ListHead *list);

static int ParseNetworkBindingLine(tSfPolicyConfig *, int, char **, char *);
static int ParseVlanBindingLine(tSfPolicyConfig *, int, char **, char *);

static OptTreeNode * firstHeadNode(SnortConfig *, int, RuleType, tSfPolicyId *);
static OptTreeNode * nextHeadNode(SnortConfig *, int, RuleType, tSfPolicyId *);
static RuleTreeNode * findHeadNode(SnortConfig *, RuleTreeNode *, tSfPolicyId);

// only keep drop rules 
// if we are inline (and can actually drop),
// or we are going to just alert instead of drop,
// or we are going to ignore session data instead of drop.
// the alert case is tested for separately with ScTreatDropAsAlert().
static INLINE int ScKeepDropRules (void)
{
    return ( ScInlineMode() || ScAdapterInlineMode() || ScTreatDropAsIgnore() );
}

static INLINE int ScLoadAsDropRules (void)
{
    return ( ScInlineTestMode() || ScAdapterInlineTestMode() );
}

/****************************************************************************
 * Function: ParseSnortConf()
 *
 * Read the rules file a line at a time and send each rule to the rule parser
 * This is the first pass of the configuration file.  It parses everything
 * except the rules.
 *
 * Arguments: None
 *
 * Returns:
 *  SnortConfig *
 *      An initialized and configured snort configuration struct.
 *      This struct should be passed on the second pass of the
 *      configuration file to parse the rules.
 *
 ***************************************************************************/
SnortConfig * ParseSnortConf(void)
{
    SnortConfig *sc = SnortConfNew();
    VarNode *tmp = cmd_line_var_list;
    tSfPolicyId policy_id;

    file_line = 0;
    file_name = snort_conf_file ? snort_conf_file : NULL_CONF;

    /* Need to set this for plugin configurations since they're using
     * lists of callbacks */
    snort_conf_for_parsing = sc;

    InitParser();

    /* Setup the default rule action anchor points
     * Need to do this now in case we get a user defined rule type */
    CreateDefaultRules(sc);

    sc->port_tables = PortTablesNew();

    mpseInitSummary();
    OtnInit(sc);

    /* Used to store "config" configurations */
    sc->config_table = sfghash_new(20, 0, 0, free);
    if (sc->config_table == NULL)
    {
        FatalError("%s(%d) No memory to create config table.\n",
                   __FILE__, __LINE__);
    }

    sc->fast_pattern_config = FastPatternConfigNew();
    sc->event_queue_config = EventQueueConfigNew();
    sc->threshold_config = ThresholdConfigNew();
    sc->rate_filter_config = RateFilter_ConfigNew();
    sc->detection_filter_config = DetectionFilterConfigNew();
    sc->ip_proto_only_lists = (SF_LIST **)SnortAlloc(NUM_IP_PROTOS * sizeof(SF_LIST *));

    /* We're not going to parse rules on the first pass */
    parse_rules = 0;

    sc->policy_config = sfPolicyInit();
    if (sc->policy_config == NULL)
    {
        FatalError("%s(%d) No memory to create policy configuration.\n",
                   __FILE__, __LINE__);
    }

    /* Add the default policy */
    policy_id = sfPolicyAdd(sc->policy_config, file_name);
    sfSetDefaultPolicy(sc->policy_config, policy_id);
    sfDynArrayCheckBounds((void **)&sc->targeted_policies, policy_id, &sc->num_policies_allocated);
    sc->targeted_policies[policy_id] = SnortPolicyNew();
    InitVarTables(sc->targeted_policies[policy_id]);
    InitPolicyMode(sc->targeted_policies[policy_id]);
    setParserPolicy(policy_id);

#ifndef SOURCEFIRE
    /* If snort is not run with root privileges, no interfaces will be defined,
     * so user beware if an iface_ADDRESS variable is used in snort.conf and
     * snort is not run as root (even if just in read mode) */
    DefineAllIfaceVars(sc);
#endif

    /* Add command line defined variables - duplicates will already
     * have been resolved */
    while (tmp != NULL)
    {
        AddVarToTable(sc, tmp->name, tmp->value);
        tmp = tmp->next;
    }

#ifdef DYNAMIC_PLUGIN
    /* Initialize storage space for preprocessor defined rule options */
    sc->targeted_policies[policy_id]->preproc_rule_options = PreprocessorRuleOptionsNew();
    if (sc->targeted_policies[policy_id]->preproc_rule_options == NULL)
    {
        FatalError("%s(%d) Could not allocate storage for preprocessor rule "
                   "options.\n", __FILE__, __LINE__);
    }
#endif

    if ( strcmp(file_name, NULL_CONF) )
        ParseConfigFile(sc, sc->targeted_policies[policy_id], file_name);

    /* We've picked up any targeted policy configs at this point so
     * it's probably okay to parse them here */
    for (policy_id = 0;
         policy_id < sfPolicyNumAllocated(sc->policy_config);
         policy_id++)
    {
        char *fname = sfPolicyGet(sc->policy_config, policy_id);

        /* Already configured default policy */
        if (policy_id == sfGetDefaultPolicy(sc->policy_config))
            continue;

        if (fname != NULL)
        {
            sfDynArrayCheckBounds(
                (void **)&sc->targeted_policies, policy_id, &sc->num_policies_allocated);
            sc->targeted_policies[policy_id] = SnortPolicyNew();

            InitVarTables(sc->targeted_policies[policy_id]);
            InitPolicyMode(sc->targeted_policies[policy_id]); 
            setParserPolicy(policy_id);
            
            /* Need to reset this for each targeted policy */
            memset(config_opt_configured, 0, sizeof(config_opt_configured));

            /* Add command line defined variables - duplicates will already
             * have been resolved */
            tmp = cmd_line_var_list;
            while (tmp != NULL)
            {
                AddVarToTable(sc, tmp->name, tmp->value);
                tmp = tmp->next;
            }

#ifdef DYNAMIC_PLUGIN
            /* Initialize storage space for preprocessor defined rule options */
            sc->targeted_policies[policy_id]->preproc_rule_options = PreprocessorRuleOptionsNew();
            if (sc->targeted_policies[policy_id]->preproc_rule_options == NULL)
            {
                FatalError("%s(%d) Could not allocate storage for preprocessor rule "
                           "options.\n", __FILE__, __LINE__);
            }
#endif

            /* Parse as include file so if the file is specified relative to
             * the snort conf directory we'll pick it up */
            ParseInclude(sc, sc->targeted_policies[policy_id], fname);
        }
    }

    /* This can be initialized now since we've picked up any user
     * defined rules */
    sc->omd = OtnXMatchDataNew(sc->num_rule_types);

    /* Make sure this gets set back to NULL when we're done parsing */
    snort_conf_for_parsing = NULL;

    /* Reset these.  The only issue in not reseting would be if we were
     * parsing a command line again, but do it anyway */
    file_name = NULL;
    file_line = 0;

    return sc;
}

static int ParseVlanBindingLine( 
        tSfPolicyConfig *config,
        int num_toks, 
        char **toks,
        char *fileName
        )
{
    int i;
    int vlanId1, vlanId2;


    for (i = 0; i < num_toks; i++)
    {
        int  num_tok2;
        char **toks2;
        int vlanId;
        unsigned pos;
        char *findStr1, *findStr2;
        char *endp;
        findStr1 = strchr(toks[i], '-');
        if ( findStr1 )
        {
            pos = findStr1 - toks[i];
            if ( pos == 0 || pos == (strlen(toks[i]) - 1) )
            {
                return -1;
            }
            findStr2 = strchr(findStr1+1, '-');
            if ( findStr2 )
            {
                return -1;
            }
            toks2 = mSplit(toks[i], "-", 2, &num_tok2, 0);
            if (num_tok2 == 2)
            {
                vlanId1 = strtol(toks2[0], &endp, 10);
                if( *endp )
                {
                    mSplitFree(&toks2, num_tok2);
                    return -1;
                }
                vlanId2 = strtol(toks2[1], &endp, 10);
                if ( *endp )
                {
                    mSplitFree(&toks2, num_tok2);
                    return -1;
                }
                if ( (vlanId1 < 0)
                    || (vlanId2 >= SF_VLAN_BINDING_MAX)
                    || (vlanId1 > vlanId2) )
                {
                    mSplitFree(&toks2, num_tok2);
                    return -1;
                    //FatalError("Invalid range: %d:%d\n", vlanId1, vlanId2);
                }
                for (vlanId = vlanId1; vlanId <= vlanId2; vlanId++)
                {
                    if ( sfVlanAddBinding(config, vlanId, fileName) != 0)
                    {
                        mSplitFree(&toks2, num_tok2);
                        return -1;
                        //FatalError("Unable to add policy: vlan %d, file %s\n", vlanId, fileName);
                    }
                }
            }
            else
            {
                mSplitFree(&toks2, num_tok2);
                return -1;
            }
            mSplitFree(&toks2, num_tok2);
        }

        else if ( toks[i] )
        {
            vlanId = strtol(toks[i], &endp, 10);
            if( *endp )
                return -1;
            if ( (vlanId >= SF_VLAN_BINDING_MAX) ||  sfVlanAddBinding(config, vlanId, fileName) != 0)
            {
                return -1;
                //FatalError("Unable to add policy: vlan %d, file %s\n", vlanId, fileName);
            }
        }
        else
        {
            return -1;
            //FatalError("formating error in binding file: %s\n", aLine);
        }
    }

    return 0;
}

static int ParseNetworkBindingLine( 
        tSfPolicyConfig *config,
        int num_toks, 
        char **toks,
        char *fileName
        )
{
    int i;

    for (i = 0; i < num_toks; i++)
    {
        SFIP_RET status;
        sfip_t *sfip;

        if( (sfip = sfip_alloc(toks[i], &status)) == NULL )
        {
            return -1;
        }
        if (sfNetworkAddBinding(config, sfip, fileName) != 0)
        {
            sfip_free(sfip);
            return -1;
            // FatalError("Unable to add policy: vlan %d, file %s\n", vlanId, fileName);
        }

        sfip_free(sfip);
    }

    return 0;
}

#ifdef DEBUG
static void DumpRuleChains(RuleListNode *rule_lists)
{
    RuleListNode *rule = rule_lists;

    if (rule_lists == NULL)
        return;

    while (rule != NULL)
    {
        DumpChain(rule->name, rule->mode, ETHERNET_TYPE_IP);
        DumpChain(rule->name, rule->mode, IPPROTO_TCP);
        DumpChain(rule->name, rule->mode, IPPROTO_UDP);
        DumpChain(rule->name, rule->mode, IPPROTO_ICMP);

        rule = rule->next;
    }
}

/****************************************************************************
 *
 * Function: DumpChain(RuleTreeNode *, char *, char *)
 *
 * Purpose: Iterate over RTNs calling DumpList on each
 *
 * Arguments: rtn_idx => the RTN index pointer
 *                       rulename => the name of the rule the list belongs to
 *            listname => the name of the list being printed out
 *
 * Returns: void function
 *
 ***************************************************************************/
static void DumpChain(char *name, int mode, int proto)
{
#ifdef SUP_IP6
    // XXX Not yet implemented - Rule chain dumping
#else
    /* XXX XXX FIX THIS */
#endif  /* SUP_IP6 */
}

/****************************************************************************
 *
 * Function: DumpList(IpAddrNode*)
 *
 * Purpose: print out the chain lists by header block node group
 *
 * Arguments: node => the head node
 *           
 * Returns: void function
 *
 ***************************************************************************/
#if 0  // avoid warning: ‘DumpList’ defined but not used
static void DumpList(IpAddrNode *idx, int negated)
{
    DEBUG_WRAP(int i=0;);
    if(!idx)
        return;

    while(idx != NULL)
    {
#ifdef SUP_IP6
       DEBUG_WRAP(DebugMessage(DEBUG_RULES,
                        "[%d]    %s",
                        i++, sfip_ntoa(idx->ip)););
#else
       DEBUG_WRAP(DebugMessage(DEBUG_RULES,
                        "[%d]    0x%.8lX / 0x%.8lX",
                        i++, (u_long) idx->ip_addr,
                        (u_long) idx->netmask););
#endif

       if(negated)
       {
           DEBUG_WRAP(DebugMessage(DEBUG_RULES, 
                       "    (EXCEPTION_FLAG Active)\n"););
       }
       else
       {
           DEBUG_WRAP(DebugMessage(DEBUG_RULES, "\n"););
       }

       idx = idx->next;
    }    
}
#endif  /* 0 */
#endif  /* DEBUG */

/*
 * Finish adding the rule to the port tables
 *
 * 1) find the table this rule should belong to (src/dst/any-any tcp,udp,icmp,ip or nocontent)
 * 2) find an index for the sid:gid pair 
 * 3) add all no content rules to a single no content port object, the ports are irrelevant so 
 *    make it a any-any port object.
 * 4) if it's an any-any rule with content, add to an any-any port object
 * 5) find if we have a port object with these ports defined, if so get it, otherwise create it.
 *    a)do this for src and dst port 
 *    b)add the rule index/id to the portobject(s)
 *    c)if the rule is bidir add the rule and port-object to both src and dst tables
 * 
 */
static int FinishPortListRule(rule_port_tables_t *port_tables, RuleTreeNode *rtn, OptTreeNode *otn,
                              int proto, port_entry_t *pe, FastPatternConfig *fp)
{
    int large_port_group = 0;
    int src_cnt = 0;
    int dst_cnt = 0;
    int rim_index;
    PortTable *dstTable;
    PortTable *srcTable;
    PortObject *aaObject;
    PortObject *ncObject;
    rule_count_t *prc;

    /* Select the Target PortTable for this rule, based on protocol, src/dst
     * dir, and if there is rule content */
    if (proto == IPPROTO_TCP)
    {
        dstTable = port_tables->tcp_dst;
        srcTable = port_tables->tcp_src;
        aaObject = port_tables->tcp_anyany;
        ncObject = port_tables->tcp_nocontent;
        prc = &tcpCnt;
    }
    else if (proto == IPPROTO_UDP)
    {
        dstTable = port_tables->udp_dst;
        srcTable = port_tables->udp_src;
        aaObject = port_tables->udp_anyany;
        ncObject = port_tables->udp_nocontent;
        prc = &udpCnt;
    }
    else if (proto == IPPROTO_ICMP)
    {
        dstTable = port_tables->icmp_dst;
        srcTable = port_tables->icmp_src;
        aaObject = port_tables->icmp_anyany;
        ncObject = port_tables->icmp_nocontent;
        prc = &icmpCnt;
    }
    else if (proto == ETHERNET_TYPE_IP)
    {
        dstTable = port_tables->ip_dst;
        srcTable = port_tables->ip_src;
        aaObject = port_tables->ip_anyany;
        ncObject = port_tables->ip_nocontent;
        prc = &ipCnt;
    }
    else
    {
        return -1;
    }
    
    /* Count rules with both src and dst specific ports */
    if (!(rtn->flags & ANY_DST_PORT) && !(rtn->flags & ANY_SRC_PORT)) 
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
                   "***\n***Info:  src & dst ports are both specific"
                   " >> gid=%u sid=%u src=%s dst=%s\n***\n",
                   otn->sigInfo.generator, otn->sigInfo.id, 
                   pe->src_port, pe->dst_port););

        prc->sd++;
    }

    /* Create/find an index to store this rules sid and gid at,
     * and use as reference in Port Objects */
    rim_index = otn->ruleIndex;
 
    /* Add up the nocontent rules */
    if (!pe->content && !pe->uricontent)
        prc->nc++;

    /* If not an any-any rule test for port bleedover, if we are using a
     * single rule group, don't bother */
    if (!fpDetectGetSingleRuleGroup(fp) &&
        (rtn->flags & (ANY_DST_PORT|ANY_SRC_PORT)) != (ANY_DST_PORT|ANY_SRC_PORT)) 
    {
        if (!(rtn->flags & ANY_SRC_PORT)) 
        {
            src_cnt = PortObjectPortCount(rtn->src_portobject);
            if (src_cnt >= fpDetectGetBleedOverPortLimit(fp))
                large_port_group = 1;
        } 

        if (!(rtn->flags & ANY_DST_PORT)) 
        {
            dst_cnt = PortObjectPortCount(rtn->dst_portobject);
            if (dst_cnt >= fpDetectGetBleedOverPortLimit(fp))
                large_port_group = 1;
        }

        if (large_port_group && fpDetectGetBleedOverWarnings(fp))
        {

            LogMessage("***Bleedover Port Limit(%d) Exceeded for rule %u:%u "
                       "(%d)ports: ", fpDetectGetBleedOverPortLimit(fp),
                       otn->sigInfo.generator, otn->sigInfo.id,
                       (src_cnt > dst_cnt) ? src_cnt : dst_cnt);

            /* If logging to syslog, this will be all multiline */
            fflush(stdout); fflush(stderr);
            PortObjectPrintPortsRaw(rtn->src_portobject);
            LogMessage(" -> ");
            PortObjectPrintPortsRaw(rtn->dst_portobject);
            LogMessage(" adding to any-any group\n"); 
            fflush(stdout);fflush(stderr);
        }
    }
    
    /* If an any-any rule add rule index to any-any port object
     * both content and no-content type rules go here if they are 
     * any-any port rules...
     * If we have an any-any rule or a large port group or 
     * were using a single rule group we make it an any-any rule. */
    if (((rtn->flags & (ANY_DST_PORT|ANY_SRC_PORT)) == (ANY_DST_PORT|ANY_SRC_PORT)) ||
        large_port_group || fpDetectGetSingleRuleGroup(fp))
    {
        if (proto == ETHERNET_TYPE_IP)
        {
            /* Add the IP rules to the higher level app protocol groups, if they apply 
             * to those protocols.  All IP rules should have any-any port descriptors
             * and fall into this test.  IP rules that are not tcp/udp/icmp go only into the 
             * IP table */
            DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
                                    "Finishing IP any-any rule %u:%u\n",
                                    otn->sigInfo.generator,otn->sigInfo.id););

            switch (GetOtnIpProto(otn))
            {
                case IPPROTO_TCP:
                    PortObjectAddRule(port_tables->tcp_anyany, rim_index);
                    tcpCnt.aa++;
                    break;

                case IPPROTO_UDP:
                    PortObjectAddRule(port_tables->udp_anyany, rim_index);
                    udpCnt.aa++;
                    break;

                case IPPROTO_ICMP:
                    PortObjectAddRule(port_tables->icmp_anyany, rim_index);
                    icmpCnt.aa++;
                    break;

                case -1:  /* Add to all ip proto anyany port tables */
                    PortObjectAddRule(port_tables->tcp_anyany, rim_index);
                    tcpCnt.aa++;

                    PortObjectAddRule(port_tables->udp_anyany, rim_index);
                    udpCnt.aa++;

                    PortObjectAddRule(port_tables->icmp_anyany, rim_index);
                    icmpCnt.aa++;

                    break;

                default:
                    break;
            }

            /* Add to the IP ANY ANY */
            PortObjectAddRule(aaObject, rim_index);
            prc->aa++;
        }
        else
        {
            /* For other protocols-tcp/udp/icmp add to the any any group */
            PortObjectAddRule(aaObject, rim_index);
            prc->aa++;
        }

        return 0; /* done */
    }

    /* add rule index to dst table if we have a specific dst port or port list */
    if (!(rtn->flags & ANY_DST_PORT)) 
    {
        PortObject *pox;

        prc->dst++;

        DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
                                "Finishing rule: dst port rule\n"););

        /* find the proper port object */
        pox = PortTableFindInputPortObjectPorts(dstTable, rtn->dst_portobject);
        if (pox == NULL)
        {
            /* Create a permanent port object */
            pox = PortObjectDupPorts(rtn->dst_portobject); 
            if (pox == NULL)
            {
                FatalError("%s(%d) Could not dup a port object - out of memory!\n",
                           __FILE__, __LINE__);
            }

            /* Add the port object to the table, and add the rule to the port object */
            PortTableAddObject(dstTable, pox);
        }

        PortObjectAddRule(pox, rim_index);

        /* if bidir, add this rule and port group to the src table */
        if (rtn->flags & BIDIRECTIONAL) 
        {
            pox = PortTableFindInputPortObjectPorts(srcTable, rtn->dst_portobject);
            if (pox == NULL)
            {
                pox = PortObjectDupPorts(rtn->dst_portobject); 
                if (pox == NULL)
                {
                    FatalError("%s(%d) Could not dup a bidir-port object - out of memory!\n",
                               __FILE__, __LINE__);
                }

                PortTableAddObject(srcTable, pox);
            }

            PortObjectAddRule(pox, rim_index);
        }
    }

    /* add rule index to src table if we have a specific src port or port list */
    if (!(rtn->flags & ANY_SRC_PORT)) 
    {
        PortObject *pox;

        prc->src++;

        pox = PortTableFindInputPortObjectPorts(srcTable, rtn->src_portobject);
        if (pox == NULL)
        {
            pox = PortObjectDupPorts(rtn->src_portobject); 
            if (pox == NULL)
            {
                FatalError("%s(%d) Could not dup a port object - out of memory!\n",
                           __FILE__, __LINE__);
            }

            PortTableAddObject(srcTable, pox);
        }

        PortObjectAddRule(pox, rim_index);

        /* if bidir, add this rule and port group to the dst table */
        if (rtn->flags & BIDIRECTIONAL) 
        {
            pox = PortTableFindInputPortObjectPorts(dstTable, rtn->src_portobject);
            if (pox == NULL)
            {
                pox = PortObjectDupPorts(rtn->src_portobject); 
                if (pox == NULL)
                {
                    FatalError("%s(%d) Could not dup a bidir-port object - out "
                               "of memory!\n", __FILE__, __LINE__);
                }

                PortTableAddObject(dstTable, pox);
            }

            PortObjectAddRule(pox, rim_index);
        }
    }

    return 0;
}
/*
*  Parse a port string as a port var, and create or find a port object for it, 
*  and add it to the port var table. These are used by the rtn's
*  as src and dst port lists for final rtn/otn processing.
*
*  These should not be confused with the port objects used to merge ports and rules
*  to build PORT_GROUP objects. Those are generated after the otn processing.
*  
*/
static PortObject * ParsePortListTcpUdpPort(PortVarTable *pvt,
                                            PortTable *noname, char *port_str)
{
    PortObject * portobject;
    //PortObject * pox;
    char       * errstr=0;
    POParser     poparser;

    if ((pvt == NULL) || (noname == NULL) || (port_str == NULL))
        return NULL;
    
    /* 1st - check if we have an any port */
    if( strcasecmp(port_str,"any")== 0 ) 
    {
        portobject = PortVarTableFind(pvt, "any");
        if (portobject == NULL)
            ParseError("PortVarTable missing an 'any' variable.");

        return portobject;
    }

    /* 2nd - check if we have a PortVar */
    else if( port_str[0]=='$' ) 
    { 
      /*||isalpha(port_str[0])*/ /*TODO: interferes with protocol names for ports*/
      char * name = port_str;

      name++; /* advance past '$' */
      
      DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,"PortVarTableFind: finding '%s'\n", port_str););

      /* look it up  in the port var table */
      portobject = PortVarTableFind(pvt, name);
      if (portobject == NULL)
          ParseError("***PortVar Lookup failed on '%s'.", port_str);

      DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,"PortVarTableFind: '%s' found!\n", port_str););
    }
    
    /* 3rd -  and finally process a raw port list */
    else  
    {   
       /* port list = [p,p,p:p,p,...] or p or p:p , no embedded spaces due to tokenizer */ 
       PortObject * pox;
       
       DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
                 "parser.c->PortObjectParseString: parsing '%s'\n",port_str););
      
       portobject = PortObjectParseString(pvt, &poparser, 0, port_str, 0);

       DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
                 "parser.c->PortObjectParseString: '%s' done.\n",port_str););
       
       if( !portobject )
       {
          errstr = PortObjectParseError( &poparser );
          ParseError("***Rule--PortVar Parse error: (pos=%d,error=%s)\n>>%s\n>>%*s\n",
                     poparser.pos,errstr,port_str,poparser.pos,"^");
       }

       /* check if we already have this port object in the un-named port var table  ... */
       pox = PortTableFindInputPortObjectPorts(noname, portobject); 
       if( pox )
       {
         DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
                    "parser.c: already have '%s' as a PortObject - "
                    "calling PortObjectFree(portbject) line=%d\n",port_str,__LINE__ ););
         PortObjectFree( portobject );
         portobject = pox;
       }
       else
       {
           DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS, 
                "parser.c: adding '%s' as a PortObject line=%d\n",port_str,__LINE__ ););
           /* Add to the un-named port var table */
           if (PortTableAddObject(noname, portobject))
           {
               FatalError("%s(%d) Unable to add raw port object to unnamed "
                          "port var table, out of memory!\n", __FILE__, __LINE__);
           }
       }
    }

    return portobject;
}
#ifdef XXXXX
/*
* Extract the Icmp Type field to determine the PortGroup.  
*/
PortObject * GetPortListIcmpPortObject( OptTreeNode * otn, PortTable *  rulesPortTable, PortObject * anyAnyPortObject )
{
   PortObject        * portobject=0;
   int                 type;
   IcmpTypeCheckData * IcmpType;
       
   IcmpType = (IcmpTypeCheckData *)otn->ds_list[PLUGIN_ICMP_TYPE];
   
   if( IcmpType && (IcmpType->operator == ICMP_TYPE_TEST_EQ) )
   {
       type = IcmpType->icmp_type;
   } 
   else
   {
       return anyAnyPortObject;
   }

   /* TODO: optimize */
   return anyAnyPortObject;
}
/*
 * Extract the IP Protocol field to determine the PortGroup.  
*/
PortObject * GetPortListIPPortObject( OptTreeNode * otn,PortTable *  rulesPortTable, PortObject * anyAnyPortObject )
{
   if (GetOtnIpProto(otn) == -1)
       return anyAnyPortObject;
       
   /* TODO: optimize */
   return anyAnyPortObject;
}

#if 0
Not currently used
/*
* Extract the Icmp Type field to determine the PortGroup.  
*/
static 
int GetOtnIcmpType(OptTreeNode * otn )
{
   int                 type;
   IcmpTypeCheckData * IcmpType;
       
   IcmpType = (IcmpTypeCheckData *)otn->ds_list[PLUGIN_ICMP_TYPE];
   
   if( IcmpType && (IcmpType->operator == ICMP_TYPE_TEST_EQ) )
   {
       type = IcmpType->icmp_type;
   } 
   else
   {
       return -1;
   }

   return -1;
}
#endif

#endif /*  XXXX - PORTLISTS */
/*
 *   Process the rule, add it to the appropriate PortObject
 *   and add the PortObject to the rtn.
 *
 *   TCP/UDP rules use ports/portlists, icmp uses the icmp type field and ip uses the protocol
 *   field as a dst port for the purposes of looking up a rule group as packets are being
 *   processed.
 * 
 *   TCP/UDP- use src/dst ports
 *   ICMP   - use icmp type as dst port,src=-1
 *   IP     - use protocol as dst port,src=-1
 *
 *   rtn - proto_node
 *   port_str - port list string or port var name
 *   proto - protocol
 *   dst_flag - dst or src port flag, true = dst, false = src
 *
 */
static int ParsePortList(RuleTreeNode *rtn, PortVarTable *pvt, PortTable *noname,
                         char *port_str, int proto, int dst_flag)
{
    PortObject *portobject = NULL;  /* src or dst */

    /* Get the protocol specific port object */
    if( proto == IPPROTO_TCP || proto == IPPROTO_UDP )
    {
        portobject = ParsePortListTcpUdpPort(pvt, noname, port_str); 
    }
    else /* ICMP, IP  - no real ports just Type and Protocol */
    {
        portobject = PortVarTableFind(pvt, "any");
        if (portobject == NULL)
        {
            FatalError("%s(%d) PortVarTable missing an 'any' variable\n",
                       __FILE__, __LINE__);
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,"Rule-PortVar Parsed: %s \n",port_str););
    
    /* !ports - port lists can be mixed 80:90,!82, 
    * so the old NOT flag is depracated for port lists 
    */

    /* set up any any flags */
    if( PortObjectHasAny(portobject) )
    {
         if( dst_flag )
             rtn->flags |= ANY_DST_PORT;
         else
             rtn->flags |= ANY_SRC_PORT;
    }

    /* check for a pure not rule - fatal if we find one */ 
    if( PortObjectIsPureNot( portobject ) )
    {
        ParseError("Pure NOT ports are not allowed!");
        /*
           if( dst_flag )
           rtn->flags |= EXCEPT_DST_PORT;
           else
           rtn->flags |= EXCEPT_SRC_PORT;
           */
    }

    /* 
    * set to the port object for this rules src/dst port,
    * these are used during rtn/otn port verification of the rule.
    */

    if (dst_flag)
         rtn->dst_portobject = portobject;
    else
         rtn->src_portobject = portobject;

    return 0;
}

/****************************************************************************
 *
 * Function: CheckForIPListConflicts
 *
 * Purpose:  Checks For IP List Conflicts in a RuleTreeNode.  Such as
 *           negations that are overlapping and more general are not allowed.
 *
 *             For example, the following is not allowed: 
 *
 *                  [1.1.0.0/16,!1.0.0.0/8]
 *
 *             The following is allowed though (not overlapping):
 *
 *                  [1.1.0.0/16,!2.0.0.0/8]
 *
 * Arguments: addrset -- IpAddrSet pointer.
 *
 * Returns: -1 if IP is empty, 1 if a conflict exists and 0 otherwise.
 *
 ***************************************************************************/
int CheckForIPListConflicts(IpAddrSet *addrset)
{
#ifdef SUP_IP6
    /* Conflict checking takes place inside the SFIP library */
    return 0;
#else
    IpAddrNode *idx = NULL, *neg_idx = NULL;
    
    if( !addrset ) return( -1 );
  
    if(!addrset->iplist || !addrset->neg_iplist)
        return 0;
    
    for(idx = addrset->iplist; idx; idx = idx->next) 
    {
        for(neg_idx = addrset->neg_iplist; neg_idx; neg_idx = neg_idx->next)
        {
            /* A smaller netmask means "less specific" */
            if(neg_idx->netmask <= idx->netmask &&
                /* Verify they overlap */
                ((neg_idx->ip_addr & neg_idx->netmask) == 
                 (idx->ip_addr & neg_idx->netmask)))
            {
                return 1;
            }
        }
    }
    
    return 0;
#endif
}

/****************************************************************************
 *
 * Function: AddRuleFuncToList(int (*func)(), RuleTreeNode *)
 *
 * Purpose:  Adds RuleTreeNode associated detection functions to the
 *          current rule's function list
 *
 * Arguments: *func => function pointer to the detection function
 *            rtn   => pointer to the current rule
 *
 * Returns: void function
 *
 ***************************************************************************/
void AddRuleFuncToList(int (*rfunc) (Packet *, struct _RuleTreeNode *, struct _RuleFpList *, int), RuleTreeNode * rtn)
{
    RuleFpList *idx;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Adding new rule to list\n"););

    idx = rtn->rule_func;
    if(idx == NULL)
    {
        rtn->rule_func = (RuleFpList *)SnortAlloc(sizeof(RuleFpList));

        rtn->rule_func->RuleHeadFunc = rfunc;
    }
    else
    {
        while(idx->next != NULL)
            idx = idx->next;

        idx->next = (RuleFpList *)SnortAlloc(sizeof(RuleFpList));
        idx = idx->next;
        idx->RuleHeadFunc = rfunc;
    }
}


/****************************************************************************
 *
 * Function: SetupRTNFuncList(RuleTreeNode *)
 *
 * Purpose: Configures the function list for the rule header detection
 *          functions (addrs and ports)
 *
 * Arguments: rtn => the pointer to the current rules list entry to attach to
 *
 * Returns: void function
 *
 ***************************************************************************/
static void SetupRTNFuncList(RuleTreeNode * rtn)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Initializing RTN function list!\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Functions: "););

    if(rtn->flags & BIDIRECTIONAL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckBidirectional->\n"););
        AddRuleFuncToList(CheckBidirectional, rtn);
    }
    else
    {
        /* Attach the proper port checking function to the function list */
        /*
         * the in-line "if's" check to see if the "any" or "not" flags have
         * been set so the PortToFunc call can determine which port testing
         * function to attach to the list
         */
        PortToFunc(rtn, (rtn->flags & ANY_DST_PORT ? 1 : 0),
                   (rtn->flags & EXCEPT_DST_PORT ? 1 : 0), DST);

        /* as above */
        PortToFunc(rtn, (rtn->flags & ANY_SRC_PORT ? 1 : 0),
                   (rtn->flags & EXCEPT_SRC_PORT ? 1 : 0), SRC);

        /* link in the proper IP address detection function */
        AddrToFunc(rtn, SRC);

        /* last verse, same as the first (but for dest IP) ;) */
        AddrToFunc(rtn, DST);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"RuleListEnd\n"););

    /* tack the end (success) function to the list */
    AddRuleFuncToList(RuleListEnd, rtn);
}

/****************************************************************************
 *
 * Function: AddrToFunc(RuleTreeNode *, u_long, u_long, int, int)
 *
 * Purpose: Links the proper IP address testing function to the current RTN
 *          based on the address, netmask, and addr flags
 *
 * Arguments: rtn => the pointer to the current rules list entry to attach to
 *            ip =>  IP address of the current rule
 *            mask => netmask of the current rule
 *            exception_flag => indicates that a "!" has been set for this
 *                              address
 *            mode => indicates whether this is a rule for the source
 *                    or destination IP for the rule
 *
 * Returns: void function
 *
 ***************************************************************************/
static void AddrToFunc(RuleTreeNode * rtn, int mode)
{
    /*
     * if IP and mask are both 0, this is a "any" IP and we don't need to
     * check it
     */
    switch(mode)
    {
        case SRC:
            if((rtn->flags & ANY_SRC_IP) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckSrcIP -> "););
                AddRuleFuncToList(CheckSrcIP, rtn);
            }

            break;

        case DST:
            if((rtn->flags & ANY_DST_IP) == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckDstIP -> "););
                AddRuleFuncToList(CheckDstIP, rtn);
            }

            break;
    }
}

/****************************************************************************
 *
 * Function: PortToFunc(RuleTreeNode *, int, int, int)
 *
 * Purpose: Links in the port analysis function for the current rule
 *
 * Arguments: rtn => the pointer to the current rules list entry to attach to
 *            any_flag =>  accept any port if set
 *            except_flag => indicates negation (logical NOT) of the test
 *            mode => indicates whether this is a rule for the source
 *                    or destination port for the rule
 *
 * Returns: void function
 *
 ***************************************************************************/
static void PortToFunc(RuleTreeNode * rtn, int any_flag, int except_flag, int mode)
{
    /*
     * if the any flag is set we don't need to perform any test to match on
     * this port
     */
    if(any_flag)
        return;

    /* if the except_flag is up, test with the "NotEq" funcs */
    if(except_flag)
    {
        switch(mode)
        {
            case SRC:
                DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckSrcPortNotEq -> "););
                AddRuleFuncToList(CheckSrcPortNotEq, rtn);
                break;


            case DST:
                DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckDstPortNotEq -> "););
                AddRuleFuncToList(CheckDstPortNotEq, rtn);
                break;
        }

        return;
    }
    /* default to setting the straight test function */
    switch(mode)
    {
        case SRC:
            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckSrcPortEqual -> "););
            AddRuleFuncToList(CheckSrcPortEqual, rtn);
            break;

        case DST:
            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"CheckDstPortEqual -> "););
            AddRuleFuncToList(CheckDstPortEqual, rtn);
            break;
    }

    return;
}

/****************************************************************************
 * Function: ParsePreprocessor()
 *
 * Saves the preprocessor configuration for loading later after dynamic
 * preprocessor keywords and configuration functions have been registered.
 * Configuration is also used later for configuration reload to check if
 * configuration has changed.
 *
 * Arguments:
 *  SnortConfig *
 *      Snort configuration to attach preprocessor configuration to.
 *  char *
 *      The preprocessor arguments.
 *
 * Returns: void function
 *
 ***************************************************************************/
static void ParsePreprocessor(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    char *keyword;
    char *opts = NULL;
    PreprocConfig *config;

    if ((sc == NULL) || (p == NULL) || (args == NULL))
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Preprocessor\n"););

    /* break out the arguments from the keywords */
    toks = mSplit(args, ":", 2, &num_toks, '\\');
    keyword = toks[0];

    if (num_toks > 1)
        opts = toks[1];

    /* Save the configuration and load later */
    config = (PreprocConfig *)SnortAlloc(sizeof(PreprocConfig));

    if (p->preproc_configs == NULL)
    {
        p->preproc_configs = config;
    }
    else
    {
        PreprocConfig *tmp = p->preproc_configs;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = config;
    }

    config->keyword = SnortStrdup(keyword);
    config->file_name = SnortStrdup(file_name);
    config->file_line = file_line;

    if (opts != NULL)
        config->opts = SnortStrdup(opts);
    else
        config->opts = NULL;

    mSplitFree(&toks, num_toks);
}

void ConfigurePreprocessors(SnortConfig *sc, int configure_dynamic)
{
    char *stored_file_name = file_name;
    int stored_file_line = file_line;
    tSfPolicyId i;

    if (sc == NULL)
        return;

    snort_conf_for_parsing = sc;

    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        PreprocConfig *config;

        setParserPolicy(i);

        if (sc->targeted_policies[i] == NULL)
            continue;

        setParserPolicy(i);

        config = sc->targeted_policies[i]->preproc_configs;

        for (; config != NULL; config = config->next)
        {
            PreprocConfigFuncNode *node;

            if (config->configured)
                continue;

            file_name = config->file_name;
            file_line = config->file_line;

            node = GetPreprocConfig(config->keyword);
            if ((node == NULL) && configure_dynamic)
                ParseError("Unknown preprocessor: \"%s\".", config->keyword);

            if (node != NULL)
            {
#ifdef SNORT_RELOAD
                if (node->initialized)
                {
                    if (node->reload_func != NULL)
                        node->reload_func(config->opts);
                }
                else
#endif
                {
                    if (node->config_func != NULL)
                        node->config_func(config->opts);
                }

                config->configured = 1;
            }
        }
    }

#ifdef SNORT_RELOAD
    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        PreprocConfig *config;

        setParserPolicy(i);

        if (sc->targeted_policies[i] == NULL)
            continue;

        setParserPolicy(i);

        config = sc->targeted_policies[i]->preproc_configs;

        /* Set all configured preprocessors to intialized */
        for (; config != NULL; config = config->next)
        {
            if (config->configured)
            {
                PreprocConfigFuncNode *node = GetPreprocConfig(config->keyword);

                if (node != NULL)
                    node->initialized = 1;
            }
        }
    }
#endif

    /* Reset these since we're done with configuring dynamic preprocessors */
    file_name = stored_file_name;
    file_line = stored_file_line;

    snort_conf_for_parsing = NULL;
}

/* Parses standalone rate_filter configuration.
 *
 * Parses rate_filter configuration in the following format and populates internal 
 * structures: 
 * @code
 * rate_filter gid <gen-id>, sid <sig-id>, 
 *     track <by_src|by_dst|by_rule>, 
 *     count <c> , seconds <s>, 
 *     new_action <alert|drop|pass|drop|reject|sdrop>, 
 *     timeout <t> [, apply_to <cidr>];
 * @endcode
 * And then adds it into pContext.
 *
 * @param rule - string containing rate_filter configuration from snort.conf file.
 *
 * @returns void
*/
static void ParseRateFilter(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    int count_flag = 0;
    int new_action_flag = 0;
    int timeout_flag = 0;
    int seconds_flag = 0;
    int tracking_flag = 0;
    int genid_flag = 0;
    int sigid_flag = 0;
    int apply_flag = 0;
    int i;
    const char* ERR_KEY = "rate_filter";

    tSFRFConfigNode thdx;

    memset( &thdx, 0, sizeof(thdx) );

    /* Potential IP list might be present so we can't split on commas
     * Change commas to semi-colons */
    args = FixSeparators(args, ';', "rate_filter");
    toks = mSplit(args, ";", 0, &num_toks, 0);  /* get rule option pairs */

    for (i = 0; i < num_toks; i++)
    {
        char **pairs;
        int num_pairs;

        pairs = mSplit(toks[i], " \t", 2, &num_pairs, 0);  /* get rule option pairs */
        
        if (num_pairs != 2)
        {
            ParseError(ERR_NOT_PAIRED);
        }

        if (!strcasecmp(pairs[0], "gen_id"))
        {
            if ( genid_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.gid = xatou(pairs[1], "rate_filter: gen_id");
        }
        else if (!strcasecmp(pairs[0], "sig_id"))
        {
            if ( sigid_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.sid = xatou(pairs[1], "rate_filter: sig_id");
        }
        else if (!strcasecmp(pairs[0], "track"))
        {
            if ( tracking_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (!strcasecmp(pairs[1], "by_src"))
            {
                thdx.tracking = SFRF_TRACK_BY_SRC;
            }
            else if (!strcasecmp(pairs[1], "by_dst"))
            {
                thdx.tracking = SFRF_TRACK_BY_DST;
            }
            else if (!strcasecmp(pairs[1], "by_rule"))
            {
                thdx.tracking = SFRF_TRACK_BY_RULE;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else if (!strcasecmp(pairs[0], "count"))
        {
            if ( count_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.count = xatoup(pairs[1], "rate_filter: count");
        }
        else if (!strcasecmp(pairs[0], "seconds"))
        {
            if ( seconds_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.seconds = xatou(pairs[1], "rate_filter: seconds");
        }
        else if (!strcasecmp(pairs[0], "new_action"))
        {
            if ( new_action_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (!strcasecmp(pairs[1], "alert"))
            {
                thdx.newAction = RULE_TYPE__ALERT;
            }
            else if (!strcasecmp(pairs[1], "drop"))
            {
                thdx.newAction = RULE_TYPE__DROP;
            }
            else if (!strcasecmp(pairs[1], "pass"))
            {
                thdx.newAction = RULE_TYPE__PASS;
            }
            else if (!strcasecmp(pairs[1], "log"))
            {
                thdx.newAction = RULE_TYPE__LOG;
            }
            else if (!strcasecmp(pairs[1], "reject"))
            {
                thdx.newAction = RULE_TYPE__REJECT;
            }
            else if (!strcasecmp(pairs[1], "sdrop"))
            {
                thdx.newAction = RULE_TYPE__SDROP;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else if (!strcasecmp(pairs[0], "timeout"))
        {
            if ( timeout_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.timeout = xatou(pairs[1],"rate_filter: timeout");
        }
        else if (!strcasecmp(pairs[0], "apply_to"))
        {
            char *ip_list = pairs[1];

            if ( apply_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.applyTo = IpAddrSetParse(ip_list);
        }
        else
        {
            ParseError(ERR_BAD_OPTION);
        }

        mSplitFree(&pairs, num_pairs);
    }

    if ( (genid_flag != 1) || (sigid_flag != 1) || (tracking_flag != 1)
      || (count_flag != 1) || (seconds_flag != 1) || (new_action_flag != 1)
      || (timeout_flag != 1) || (apply_flag > 1) )
    {
        ParseError(ERR_BAD_ARG_COUNT);
    }
    if ( !thdx.seconds
        && (thdx.gid != GENERATOR_INTERNAL
         || thdx.sid != INTERNAL_EVENT_SESSION_ADD) )
    {
        ParseError("rate_filter: seconds must be > 0");
    }

    if (RateFilter_Create(sc->rate_filter_config,  &thdx))
    {
        ParseError(ERR_CREATE);
    }

    mSplitFree(&toks, num_toks);
}

static void ParseRuleTypeOutput(SnortConfig *sc, char *args, ListHead *list)
{
    char **toks;
    int num_toks;
    char *opts = NULL;
    OutputConfig *config;

    toks = mSplit(args, ":", 2, &num_toks, '\\');

    if (num_toks > 1)
        opts = toks[1];

    config = (OutputConfig *)SnortAlloc(sizeof(OutputConfig));

    if (sc->rule_type_output_configs == NULL)
    {
        sc->rule_type_output_configs = config;
    }
    else
    {
        OutputConfig *tmp = sc->rule_type_output_configs;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = config;
    }

    config->keyword = SnortStrdup(toks[0]);
    if (opts != NULL)
        config->opts = SnortStrdup(opts);
    config->rule_list = list;

    if (file_name != NULL)
    {
        config->file_name = SnortStrdup(file_name);
        config->file_line = file_line;
    }

    mSplitFree(&toks, num_toks);
}

void ParseOutput(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    char *opts = NULL;
    OutputConfig *config;

    toks = mSplit(args, ":", 2, &num_toks, '\\');

    if (num_toks > 1)
        opts = toks[1];

    config = (OutputConfig *)SnortAlloc(sizeof(OutputConfig));

    if (sc->output_configs == NULL)
    {
        sc->output_configs = config;
    }
    else
    {
        OutputConfig *tmp = sc->output_configs;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = config;
    }

    config->keyword = SnortStrdup(toks[0]);
    if (opts != NULL)
        config->opts = SnortStrdup(opts);

    /* This could come from parsing the command line */
    if (file_name != NULL)
    {
        config->file_name = SnortStrdup(file_name);
        config->file_line = file_line;
    }

    mSplitFree(&toks, num_toks);
}

static void TransferOutputConfigs(OutputConfig *from_list, OutputConfig **to_list)
{
    if ((from_list == NULL) || (to_list == NULL))
        return;

    for (; from_list != NULL; from_list = from_list->next)
    {
        if (*to_list == NULL)
        {
            *to_list = DupOutputConfig(from_list);
        }
        else
        {
            OutputConfig *tmp = DupOutputConfig(from_list);

            if (tmp != NULL)
            {
                tmp->next = *to_list;
                *to_list = tmp;
            }
        }
    }
}

static OutputConfig * DupOutputConfig(OutputConfig *dupme)
{
    OutputConfig *medup;

    if (dupme == NULL)
        return NULL;

    medup = (OutputConfig *)SnortAlloc(sizeof(OutputConfig));

    if (dupme->keyword != NULL)
        medup->keyword = SnortStrdup(dupme->keyword);

    if (dupme->opts != NULL)
        medup->opts = SnortStrdup(dupme->opts);

    if (dupme->file_name != NULL)
        medup->file_name = SnortStrdup(dupme->file_name);

    medup->file_line = dupme->file_line;
    medup->rule_list = dupme->rule_list;

    return medup;
}

static void RemoveOutputConfigs(OutputConfig **head, int remove_flags)
{
    OutputConfig *config;
    OutputConfig *last = NULL;

    if (head == NULL)
        return;

    config = *head;

    while (config != NULL)
    {
        int type_flags = GetOutputTypeFlags(config->keyword);

        if (type_flags & remove_flags)
        {
            OutputConfig *tmp = config;

            config = config->next;

            if (last == NULL)
                *head = config;
            else
                last->next = config;

            free(tmp->keyword);

            if (tmp->opts != NULL)
                free(tmp->opts);

            if (tmp->file_name != NULL)
                free(tmp->file_name);

            free(tmp);
        }
        else
        {
            last = config;
            config = config->next;
        }
    }
}

void ResolveOutputPlugins(SnortConfig *cmd_line, SnortConfig *config_file)
{
    int cmd_line_type_flags = 0;

    if (cmd_line == NULL)
        return;

    if (cmd_line->no_log)
    {
        /* Free any log output plugins in both lists */
        RemoveOutputConfigs(&cmd_line->output_configs, OUTPUT_TYPE__LOG);

        if (config_file != NULL)
        {
            RemoveOutputConfigs(&config_file->output_configs, OUTPUT_TYPE__LOG);
            RemoveOutputConfigs(&config_file->rule_type_output_configs, OUTPUT_TYPE__LOG);
        }
    }
    else if ((config_file != NULL) && config_file->no_log)
    {
        /* Free any log output plugins in config list */
        RemoveOutputConfigs(&config_file->output_configs, OUTPUT_TYPE__LOG);
        RemoveOutputConfigs(&config_file->rule_type_output_configs, OUTPUT_TYPE__LOG);
    }

    if (cmd_line->no_alert)
    {
        /* Free any alert output plugins in both lists */
        RemoveOutputConfigs(&cmd_line->output_configs, OUTPUT_TYPE__ALERT);

        if (config_file != NULL)
        {
            RemoveOutputConfigs(&config_file->output_configs, OUTPUT_TYPE__ALERT);
            RemoveOutputConfigs(&config_file->rule_type_output_configs, OUTPUT_TYPE__ALERT);
        }
    }
    else if ((config_file != NULL) && config_file->no_alert)
    {
        /* Free any alert output plugins in config list */
        RemoveOutputConfigs(&config_file->output_configs, OUTPUT_TYPE__ALERT);
        RemoveOutputConfigs(&config_file->rule_type_output_configs, OUTPUT_TYPE__ALERT);
    }

    /* Command line overrides configuration file output */
    if (cmd_line->output_configs != NULL)
    {
        OutputConfig *config = cmd_line->output_configs;

        for (; config != NULL; config = config->next)
        {
            int type_flags = GetOutputTypeFlags(config->keyword);

            cmd_line_type_flags |= type_flags;

            if (config_file != NULL)
            {
                RemoveOutputConfigs(&config_file->output_configs, type_flags);
                RemoveOutputConfigs(&config_file->rule_type_output_configs, type_flags);
            }
        }

        /* Put what's in the command line output into the config file output */
        if (config_file != NULL)
            TransferOutputConfigs(cmd_line->output_configs, &config_file->output_configs);
    }

    if (config_file != NULL)
    {
        if (cmd_line->no_log)
            config_file->no_log = cmd_line->no_log;

        if (cmd_line->no_alert)
            config_file->no_alert = cmd_line->no_alert;
    }

    /* Don't try to configure defaults if running in test mode */
    if (!ScTestMode())
    {
        if (config_file == NULL)
        {
            if (!cmd_line->no_log && !(cmd_line_type_flags & OUTPUT_TYPE__LOG))
                ParseOutput(cmd_line, NULL, "log_tcpdump");

            if (!cmd_line->no_alert && !(cmd_line_type_flags & OUTPUT_TYPE__ALERT))
                ParseOutput(cmd_line, NULL, "alert_full");
        }
        else
        {
            int config_file_type_flags = 0;
            OutputConfig *config = config_file->output_configs;

            for (; config != NULL; config = config->next)
                config_file_type_flags |= GetOutputTypeFlags(config->keyword);

            if (!config_file->no_log && !(config_file_type_flags & OUTPUT_TYPE__LOG))
                ParseOutput(config_file, NULL, "log_tcpdump");

            if (!config_file->no_alert && !(config_file_type_flags & OUTPUT_TYPE__ALERT))
                ParseOutput(config_file, NULL, "alert_full");
        }
    }
}

void ConfigureOutputPlugins(SnortConfig *sc)
{
    OutputConfig *config;
    char *stored_file_name = file_name;
    int stored_file_line = file_line;

    snort_conf_for_parsing = sc;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Output Plugin\n"););

    for (config = sc->output_configs; config != NULL; config = config->next)
    {
        OutputConfigFunc oc_func;

        file_name = config->file_name;
        file_line = config->file_line;

        oc_func = GetOutputConfigFunc(config->keyword);
        if (oc_func == NULL)
            ParseError("Unknown output plugin: \"%s\"", config->keyword);

        oc_func(config->opts);
    }

    /* Configure output plugins for user defined rule types */
    for (config = sc->rule_type_output_configs; config != NULL; config = config->next)
    {
        OutputConfigFunc oc_func;

        file_name = config->file_name;
        file_line = config->file_line;

        oc_func = GetOutputConfigFunc(config->keyword);
        if (oc_func == NULL)
            ParseError("Unknown output plugin \"%s\"", config->keyword);

        /* Each user defined rule type has it's own rule list and output plugin is
         * attached to it's Alert and/or Log lists */
        head_tmp = config->rule_list;
        oc_func(config->opts);
        head_tmp = NULL;
    }

    /* Reset these since we're done with configuring dynamic preprocessors */
    file_name = stored_file_name;
    file_line = stored_file_line;

    snort_conf_for_parsing = NULL;
}

static void FreeRuleTreeNode(RuleTreeNode *rtn)
{
    RuleFpList *idx, *tmp;
    if (!rtn)
        return;

    if (rtn->sip)
    {
#ifdef SUP_IP6
        sfvar_free(rtn->sip);
#else
        IpAddrSetDestroy(rtn->sip);
        free(rtn->sip);
        rtn->sip = NULL;
#endif
    }

    if (rtn->dip)
    {
#ifdef SUP_IP6
        sfvar_free(rtn->dip);
#else
        IpAddrSetDestroy(rtn->dip);
        free(rtn->dip);
        rtn->dip = NULL;
#endif
    }

    idx = rtn->rule_func;
    while (idx)
    {
        tmp = idx;
        idx = idx->next;
        free(tmp);
    }
}

static void DestroyRuleTreeNode(RuleTreeNode *rtn)
{
    if (!rtn)
        return;

    rtn->otnRefCount--;
    if (rtn->otnRefCount != 0)
        return;

    FreeRuleTreeNode(rtn);

    free(rtn);
}

/****************************************************************************
 *
 * Function: mergeDuplicateOtn()
 *
 * Purpose:  Conditionally removes duplicate SID/GIDs. Keeps duplicate with 
 *           higher revision.  If revision is the same, keeps newest rule.
 *
 * Arguments: otn_dup => The existing duplicate
 *            rtn => the RTN chain to check
 *            char => String describing the rule
 *            rule_type => enumerated rule type (alert, pass, log)
 *
 * Returns: 0 if original rule stays, 1 if new rule stays
 *
 ***************************************************************************/
static int mergeDuplicateOtn(SnortConfig *sc, OptTreeNode *otn_dup,
                             OptTreeNode *otn_new, RuleTreeNode *rtn_new)
{
    RuleTreeNode *rtn_dup = NULL;
    RuleTreeNode *rtnTmp2 = NULL;
    unsigned i;

    if (otn_dup->proto != otn_new->proto)
    {
        ParseError("GID %d SID %d in rule duplicates previous rule, with "
                   "different protocol.",
                   otn_new->sigInfo.generator, otn_new->sigInfo.id);
    }

    rtn_dup = getParserRtnFromOtn(otn_dup);

    if((rtn_dup != NULL) && (rtn_dup->type != rtn_new->type)) 
    {
        ParseError("GID %d SID %d in rule duplicates previous rule, with "
                   "different type.",
                   otn_new->sigInfo.generator, otn_new->sigInfo.id);
    }

    if((otn_new->sigInfo.shared < otn_dup->sigInfo.shared) 
        || ((otn_new->sigInfo.shared == otn_dup->sigInfo.shared) 
            && (otn_new->sigInfo.rev < otn_dup->sigInfo.rev)))
    {
        //existing OTN is newer version. Keep existing and discard the new one.
        //OTN is for new policy group, salvage RTN
        deleteRtnFromOtn(otn_new, getParserPolicy());

        ParseMessage("GID %d SID %d duplicates previous rule. Using %s.", 
                     otn_new->sigInfo.generator, otn_new->sigInfo.id,
                     otn_dup->sigInfo.shared ? "SO rule.":"higher revision");

        /* delete the data for each rule option in this OTN */
        OtnDeleteData(otn_new);

        /* Now free the OTN itself -- this function is also used
         * by the hash-table calls out of OtnRemove, so it cannot
         * be modified to delete data for rule options */
        OtnFree(otn_new);

        //Add rtn to existing otn for the first rule instance in a policy,
        //otherwise ignore it
        if (rtn_dup == NULL)
        {
            addRtnToOtn(otn_dup, getParserPolicy(), rtn_new);
        }
        else
        {
            DestroyRuleTreeNode(rtn_new);
        }

        return 0;
    }
    
    //delete existing rule instance and keep the new one

    for (i = 0; i < otn_dup->proto_node_num; i++)
    {
        rtnTmp2 = deleteRtnFromOtn(otn_dup, i);

        if ((rtnTmp2 && (i != getParserPolicy())))
        {
            addRtnToOtn(otn_new, i, rtnTmp2);
        }
    }

    if (rtn_dup)
    {
        if (ScConfErrorOut())
        {
            ParseError("GID %d SID %d in rule duplicates previous rule.",
                    otn_new->sigInfo.generator, otn_new->sigInfo.id);
        }
        else 
        {
            ParseWarning("GID %d SID %d in rule duplicates previous "
                    "rule. Ignoring old rule.\n",
                    otn_new->sigInfo.generator, otn_new->sigInfo.id);
        }

        switch (otn_new->sigInfo.rule_type)
        {
            case SI_RULE_TYPE_DETECT:
                detect_rule_count--;
                break;
            case SI_RULE_TYPE_DECODE:
                decode_rule_count--;
                break;
            case SI_RULE_TYPE_PREPROC:
                preproc_rule_count--;
                break;
            default:
                break;
        }
    }

    otn_count--;

    OtnRemove(sc->otn_map, sc->so_rule_otn_map, otn_dup);
    DestroyRuleTreeNode(rtn_dup);

    return 1;
}


/****************************************************************************
 *
 * Function: ParseRuleOptions(char *, int)
 *
 * Purpose:  Process an individual rule's options and add it to the
 *           appropriate rule chain
 *
 * Arguments: rule => rule string
 *            rule_type => enumerated rule type (alert, pass, log)
 *            *conflicts => Identifies whether there was a conflict due to duplicate
 *                rule and whether existing otn was newer or not.
 *                0 - no conflict
 *                1 - existing otn is newer.
 *                -1 - existing otn is older.
 *
 * Returns:
 *  OptTreeNode *
 *      The new OptTreeNode on success or NULL on error.
 *
 ***************************************************************************/
OptTreeNode * ParseRuleOptions(SnortConfig *sc, RuleTreeNode *rtn,
                               char *rule_opts, RuleType rule_type, int protocol)
{
    OptTreeNode *otn;
    RuleOptOtnHandler otn_handler = NULL;
    int num_detection_opts = 0;
    char *dopt_keyword = NULL;
    OptFpList *fpl = NULL;
    int got_sid = 0;

    otn = (OptTreeNode *)SnortAlloc(sizeof(OptTreeNode));

    otn->chain_node_number = otn_count;
    otn->proto = protocol;
    otn->event_data.sig_generator = GENERATOR_SNORT_ENGINE;
    otn->sigInfo.generator        = GENERATOR_SNORT_ENGINE;
    otn->sigInfo.rule_type        = SI_RULE_TYPE_DETECT; /* standard rule */
    otn->sigInfo.rule_flushing    = SI_RULE_FLUSHING_ON; /* usually just standard rules cause a flush*/

    /* Set the default rule state */
    otn->rule_state = ScDefaultRuleState();

    if (rule_opts == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "No rule options.\n"););

        if (ScRequireRuleSid())
            ParseError("Each rule must contain a Rule-sid.");

        addRtnToOtn(otn, getParserPolicy(), rtn);

        otn->ruleIndex = RuleIndexMapAdd(ruleIndexMap, 
                                         otn->sigInfo.generator, 
                                         otn->sigInfo.id);
    }
    else
    {
        char **toks;
        int num_toks;
        char configured[sizeof(rule_options) / sizeof(RuleOptFunc)];
        int i;
        OptTreeNode *otn_dup;

        if ((rule_opts[0] != '(') || (rule_opts[strlen(rule_opts) - 1] != ')'))
        {
            ParseError("Rule options must be enclosed in '(' and ')'.");
        }

        /* Move past '(' and zero out ')' */
        rule_opts++;
        rule_opts[strlen(rule_opts) - 1] = '\0';

        /* Used to determine if a rule option has already been configured
         * in the rule.  Some can only be configured once */
        memset(configured, 0, sizeof(configured));

        toks = mSplit(rule_opts, ";", 0, &num_toks, '\\');

        for (i = 0; i < num_toks; i++)
        {
            char **opts;
            int num_opts;
            char *option_args = NULL;
            int j;

            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"   option: %s\n", toks[i]););

            /* break out the option name from its data */
            opts = mSplit(toks[i], ":", 2, &num_opts, '\\');

            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"   option name: %s\n", opts[0]););

            if (num_opts == 2)
            {
                option_args = opts[1];
                DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"   option args: %s\n", option_args););
            }

            for (j = 0; rule_options[j].name != NULL; j++)
            {
                if (strcasecmp(opts[0], rule_options[j].name) == 0)
                {
                    if (configured[j] && rule_options[j].only_once)
                    {
                        ParseError("Only one '%s' rule option per rule.",
                                   opts[0]);
                    }

                    if ((option_args == NULL) && rule_options[j].args_required)
                    {
                        ParseError("No argument passed to keyword \"%s\".  "
                                   "Make sure you didn't forget a ':' or the "
                                   "argument to this keyword.\n", opts[0]);
                    }

                    rule_options[j].parse_func(sc, rtn, otn, rule_type, option_args);
                    configured[j] = 1;
                    break;
                }
            }

            /* Because we actually allow an sid of 0 */
            if ((rule_options[j].name != NULL) &&
                (strcasecmp(rule_options[j].name, RULE_OPT__SID) == 0))
            {
                got_sid = 1;
            }

            /* It's possibly a detection option plugin */
            if (rule_options[j].name == NULL)
            {
                RuleOptConfigFuncNode *dopt = rule_opt_config_funcs;

                for (; dopt != NULL; dopt = dopt->next)
                {
                    if (strcasecmp(opts[0], dopt->keyword) == 0)
                    {
                        dopt->func(option_args, otn, protocol);

                        /* If this option contains an OTN handler, save it for
                           use after the rule is done parsing. */
                        if (dopt->otn_handler != NULL)
                            otn_handler = dopt->otn_handler;

                        /* This is done so if we have a preprocessor/decoder
                         * rule, we can tell the user that detection options
                         * are not supported with those types of rules, and
                         * what the detection option is */
                        if ((dopt_keyword == NULL) &&
                            (dopt->type == OPT_TYPE_DETECTION))
                        {
                            dopt_keyword = SnortStrdup(opts[0]);
                        }

                        break;
                    }
                }

                if (dopt == NULL)
                {
#ifdef DYNAMIC_PLUGIN
                    /* Maybe it's a preprocessor rule option */
                    PreprocOptionInit initFunc = NULL;
                    PreprocOptionEval evalFunc = NULL;
                    PreprocOptionFastPatternFunc fpFunc = NULL;
                    PreprocOptionOtnHandler preprocOtnHandler = NULL;
                    PreprocOptionCleanup cleanupFunc = NULL;
                    void *opt_data = NULL;

                    int ret = GetPreprocessorRuleOptionFuncs
                        (opts[0], &initFunc, &evalFunc,
                         &preprocOtnHandler, &fpFunc, &cleanupFunc);

                    if (ret && (initFunc != NULL))
                    {
                        initFunc(opts[0], option_args, &opt_data);
                        AddPreprocessorRuleOption(opts[0], otn, opt_data, evalFunc);
                        if (preprocOtnHandler != NULL)
                            otn_handler = (RuleOptOtnHandler)preprocOtnHandler;

                        DEBUG_WRAP(DebugMessage(DEBUG_INIT, "%s->", opts[0]););
                    }
                    else
#endif
                    {
                        /* Unrecognized rule option */
                        ParseError("Unknown rule option: '%s'.", opts[0]);
                    }
                }

                if (dopt_keyword == NULL)
                    dopt_keyword = SnortStrdup(opts[0]);

                num_detection_opts++;
            }

            mSplitFree(&opts, num_opts);
        }

        if ((dopt_keyword != NULL) &&
            (otn->sigInfo.rule_type != SI_RULE_TYPE_DETECT))
        {
            /* Preprocessor and decoder rules can not have
             * detection options */
            ParseError("Preprocessor and decoder rules do not support "
                       "detection options: %s.", dopt_keyword);
        }

        if (dopt_keyword != NULL)
            free(dopt_keyword);

        if (!got_sid && !ScTestMode())
            ParseError("Each rule must contain a rule sid.");

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"OptListEnd\n"););

        addRtnToOtn(otn, getParserPolicy(), rtn);

        /* Check for duplicate SID */
        otn_dup = OtnLookup(sc->otn_map, otn->sigInfo.generator, otn->sigInfo.id);
        if (otn_dup != NULL)
        {
            otn->ruleIndex = otn_dup->ruleIndex;

            if (mergeDuplicateOtn(sc, otn_dup, otn, rtn) == 0)
            {
                /* We are keeping the old/dup OTN and trashing the new one
                 * we just created - it's free'd in the remove dup function */
                mSplitFree(&toks, num_toks);
                return NULL;
            }
        }
        else
        {
            otn->ruleIndex = RuleIndexMapAdd(ruleIndexMap, 
                                             otn->sigInfo.generator, 
                                             otn->sigInfo.id);
        }

        mSplitFree(&toks, num_toks);
    }

    otn->num_detection_opts += num_detection_opts;
    otn_count++;

    if (otn->sigInfo.rule_type == SI_RULE_TYPE_DETECT)
        detect_rule_count++;
    else if (otn->sigInfo.rule_type == SI_RULE_TYPE_DECODE)
    {
        //Set the bit if the decoder rule is enabled in the policies
        UpdateDecodeRulesArray(otn->sigInfo.id, ENABLE_RULE, ENABLE_ONE_RULE);
        decode_rule_count++;
    }
    else if (otn->sigInfo.rule_type == SI_RULE_TYPE_PREPROC)
        preproc_rule_count++;

    fpl = AddOptFuncToList(OptListEnd, otn);
    fpl->type = RULE_OPTION_TYPE_LEAF_NODE;

    if (otn_handler != NULL)
    {
        otn_handler(otn);
    }

    FinalizeContentUniqueness(otn);

    if ((thdx_tmp != NULL) && (otn->detection_filter != NULL))
    {
        ParseError("The \"detection_filter\" rule option and the \"threshold\" "
                   "rule option cannot be used in the same rule.\n");
    }

    if (thdx_tmp != NULL)
    {
        int rstat;

        thdx_tmp->sig_id = otn->sigInfo.id;
        thdx_tmp->gen_id = otn->sigInfo.generator;
        rstat = sfthreshold_create(sc->threshold_config, thdx_tmp);

        if (rstat)
        {
            if (rstat == THD_TOO_MANY_THDOBJ)
            {
                ParseError("threshold (in rule): could not create threshold - "
                           "only one per sig_id=%u.", thdx_tmp->sig_id);
            }
            else
            {
                ParseError("threshold (in rule): could not add threshold "
                           "for sig_id=%u!\n", thdx_tmp->sig_id);
            }
        }

        thdx_tmp = NULL;
    }

    /* setup gid,sid->otn mapping */
    SoRuleOtnLookupAdd(sc->so_rule_otn_map, otn);  
    OtnLookupAdd(sc->otn_map, otn);  

    return otn;
}

/****************************************************************************
 *
 * Function: GetRuleProtocol(char *)
 *
 * Purpose: Figure out which protocol the current rule is talking about
 *
 * Arguments: proto_str => the protocol string
 *
 * Returns: The integer value of the protocol
 *
 ***************************************************************************/
static int GetRuleProtocol(char *proto_str)
{
    if (strcasecmp(proto_str, RULE_PROTO_OPT__TCP) == 0)
    {
        return IPPROTO_TCP;
    }
    else if (strcasecmp(proto_str, RULE_PROTO_OPT__UDP) == 0)
    {
        return IPPROTO_UDP;
    }
    else if (strcasecmp(proto_str, RULE_PROTO_OPT__ICMP) == 0)
    {
        return IPPROTO_ICMP;
    }
    else if (strcasecmp(proto_str, RULE_PROTO_OPT__IP) == 0)
    {
        return ETHERNET_TYPE_IP;
    }
    else
    {
        /* If we've gotten here, we have a protocol string we didn't recognize
         * and should exit */
        ParseError("Bad protocol: %s.", proto_str);
    }

    return -1;
}


static int ProcessIP(SnortConfig *sc, char *addr, RuleTreeNode *rtn, int mode, int neg_list)
{
#ifndef SUP_IP6
    VarEntry *var_table = sc->targeted_policies[getParserPolicy()]->var_table;
    VarEntry *ip_var = NULL;
#else
    vartable_t *ip_vartable = sc->targeted_policies[getParserPolicy()]->ip_vartable;
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Got address string: %s\n", 
                addr););

#ifdef SUP_IP6 
    /* If a rule has a variable in it, we want to copy that variable's 
     * contents to the IP variable (IP list) stored with the rtn.
     * This code tries to look up the variable, and if found, will copy it 
     * to the rtn->{sip,dip} */
    if(mode == SRC) 
    {   
        int ret;

        if (rtn->sip == NULL)
        {
            sfip_var_t *tmp = sfvt_lookup_var(ip_vartable, addr);
            if (tmp != NULL)
            {
                rtn->sip = sfvar_create_alias(tmp, tmp->name);
                if (rtn->sip == NULL)
                    ret = SFIP_FAILURE;
                else
                    ret = SFIP_SUCCESS;
            }
            else
            {
                rtn->sip = (sfip_var_t *)SnortAlloc(sizeof(sfip_var_t));
                ret = sfvt_add_to_var(ip_vartable, rtn->sip, addr);
            }
        }
        else
        {
            ret = sfvt_add_to_var(ip_vartable, rtn->sip, addr);
        }

        /* The function sfvt_add_to_var adds 'addr' to the variable 'rtn->sip' */
        if (ret != SFIP_SUCCESS) 
        {
            if(ret == SFIP_LOOKUP_FAILURE)
            {
                ParseError("Undefined variable in the string: %s.", addr);
            }
            else if(ret == SFIP_CONFLICT)
            {
                ParseError("Negated IP ranges that are more general than "
                           "non-negated ranges are not allowed. Consider "
                           "inverting the logic: %s.", addr);
            }
            else if(ret == SFIP_NOT_ANY) 
            {
                ParseError("!any is not allowed: %s.", addr);
            }
            else
            {
                ParseError("Unable to process the IP address: %s.", addr);
            }
        }

        if(rtn->sip->head && rtn->sip->head->flags & SFIP_ANY) 
        {
            rtn->flags |= ANY_SRC_IP;
        }
    } 
    /* mode == DST */
    else 
    {
        int ret;

        if (rtn->dip == NULL)
        {
            sfip_var_t *tmp = sfvt_lookup_var(ip_vartable, addr);
            if (tmp != NULL)
            {
                rtn->dip = sfvar_create_alias(tmp, tmp->name);
                if (rtn->dip == NULL)
                    ret = SFIP_FAILURE;
                else
                    ret = SFIP_SUCCESS;
            }
            else
            {
                rtn->dip = (sfip_var_t *)SnortAlloc(sizeof(sfip_var_t));
                ret = sfvt_add_to_var(ip_vartable, rtn->dip, addr);
            }
        }
        else
        {
            ret = sfvt_add_to_var(ip_vartable, rtn->dip, addr);
        }

        if (ret != SFIP_SUCCESS) 
        {
            if(ret == SFIP_LOOKUP_FAILURE)
            {
                ParseError("Undefined variable in the string: %s.", addr);
            }
            else if(ret == SFIP_CONFLICT)
            {
                ParseError("Negated IP ranges that are more general than "
                           "non-negated ranges are not allowed. Consider "
                           "inverting the logic: %s.", addr);
            }
            else if(ret == SFIP_NOT_ANY) 
            {
                ParseError("!any is not allowed: %s.", addr);
            }
            else
            {
                ParseError("Unable to process the IP address: %s.", addr);
            }
        }

        if(rtn->dip->head && rtn->dip->head->flags & SFIP_ANY) 
        {
            rtn->flags |= ANY_DST_IP;
        }
    }
#else

    if ((var_table != NULL) && (*addr == '$'))
    {
        VarEntry *tmp = var_table;

        do
        {
            /* addr+1 to move past $ */
            if (strcmp(tmp->name, addr+1) == 0)
            {
                if (tmp->addrset != NULL)
                {
                    if (mode == SRC)
                        rtn->sip = IpAddrSetCopy(tmp->addrset);
                    else
                        rtn->dip = IpAddrSetCopy(tmp->addrset);

                    return 0;
                }

                ip_var = tmp;
                break;
            }

            tmp = tmp->next;

        } while (tmp != var_table);
    }

    ProcessIpList(sc, addr, rtn, mode, neg_list);

    if (ip_var != NULL)
    {
        if (mode == SRC)
        {
            rtn->sip->id = ip_var->id;
            ip_var->addrset = IpAddrSetCopy(rtn->sip);
        }
        else
        {
            rtn->dip->id = ip_var->id;
            ip_var->addrset = IpAddrSetCopy(rtn->dip);
        }
    }

#endif

    /* Make sure the IP lists provided by the user are valid */
    if (mode == SRC)
        ValidateIPList(rtn->sip, addr);
    else
        ValidateIPList(rtn->dip, addr);

    return 0;
}

#ifndef SUP_IP6
static int ProcessIpList(SnortConfig *sc, char *addr, RuleTreeNode *rtn, int mode, int neg_list)
{
    char *tok, *end, *tmp;
    int neg_ip;

    while (*addr)
    {
        /* Skip whitespace and leading commas */
        for(; *addr && (isspace((int)*addr) || *addr == ','); addr++) ;

        /* Handle multiple negations (such as if someone negates variable that
         * contains a negated IP */
        neg_ip = 0;
        for(; *addr == '!'; addr++) 
            neg_ip = !neg_ip;

        /* Find end of this token */
        for(end = addr+1; 
                *end && !isspace((int)*end) && *end != ']' && *end != ',';
                end++) ;

        tok = SnortStrndup(addr, end - addr);

        if (!tok)
            ParseError("Unterminated IP List '%s'.", addr);

        if(*addr == '[') 
        {
            int brack_count = 0;
            char *list_tok;

            /* Find corresponding ending bracket */
            for(end = addr; *end; end++) 
            {
                if(*end == '[') 
                    brack_count++;
                else if(*end == ']')
                    brack_count--;

                if(!brack_count)
                    break;
            }

            if(!*end) 
                ParseError("Unterminated IP List '%s'.", addr);

            addr++;

            list_tok = SnortStrndup(addr, end - addr);

            if(!list_tok)
                ParseError("Failed to allocate space for parsing '%s'.", addr);

            ProcessIpList(sc, list_tok, rtn, mode, neg_list ^ neg_ip);
            free(list_tok);
        }
        else if(*addr == '$') 
        {
            if((tmp = VarGet(tok + 1)) == NULL)
                ParseError("Undefined variable %s.", addr);

            ProcessIpList(sc, tmp, rtn, mode, neg_list ^ neg_ip); 
        }
        else if(*addr == ']')
        {
            if(!(*(addr+1))) 
            {
                /* Succesfully reached the end of this list */
                free(tok);
                return 0;
            }

            ParseError("Mismatched bracket in '%s'.", addr);
        }
        else 
        {
            /* Skip leading commas */
            for(; *addr && (*addr == ',' || isspace((int)*addr)); addr++) ;

            if(mode == SRC) 
            {
                if(!rtn->sip)
                    rtn->sip = (IpAddrSet*)SnortAlloc(sizeof(IpAddrSet));

                ParseIP(tok, rtn->sip, neg_list ^ neg_ip);

                if(rtn->sip->iplist && 
                        !rtn->sip->iplist->ip_addr && !rtn->sip->iplist->netmask) 
                    rtn->flags |= ANY_SRC_IP;

            }
            else
            {
                if(!rtn->dip)
                    rtn->dip = (IpAddrSet*)SnortAlloc(sizeof(IpAddrSet));

                ParseIP(tok, rtn->dip, neg_list ^ neg_ip);

                if(rtn->dip->iplist &&
                        !rtn->dip->iplist->ip_addr && !rtn->dip->iplist->netmask) 
                    rtn->flags |= ANY_DST_IP;

                /* Note: the neg_iplist is not checked for '!any' here since
                 * ParseIP should have already FatalError'ed on it. */
            }
        }

        free(tok);

        if(*end)
            addr = end + 1;   
        else break;
    }

    return 0;
}
#endif

/****************************************************************************
 *
 * Function: ParsePort(char *, u_short *)
 *
 * Purpose:  Convert the port string over to an integer value
 *
 * Arguments: prule_port => port rule string
 *            port => converted integer value of the port
 *
 * Returns: 0 for a normal port number, 1 for an "any" port
 *
 ***************************************************************************/
int ParsePort(char *prule_port, uint16_t *hi_port, uint16_t *lo_port, char *proto, int *not_flag)
{
    char **toks;        /* token dbl buffer */
    int num_toks;       /* number of tokens found by mSplit() */
    char *rule_port;    /* port string */

    *not_flag = 0;

    /* check for variable */
    if(!strncmp(prule_port, "$", 1))
    {
        if((rule_port = VarGet(prule_port + 1)) == NULL)
        {
            ParseError("Undefined variable %s.", prule_port);
        }
    }
    else
        rule_port = prule_port;

    if(rule_port[0] == '(')
    {
        /* user forgot to put a port number in for this rule */
        ParseError("Bad port number: \"%s\".", rule_port);
    }


    /* check for wildcards */
    if(!strcasecmp(rule_port, "any"))
    {
        *hi_port = 0;
        *lo_port = 0;
        return 1;
    } 

    if(rule_port[0] == '!')
    {
        if(!strcasecmp(&rule_port[1], "any"))
        {
            ParseWarning("Negating \"any\" is invalid. Rule "
                         "will be ignored.");
            return -1;
        } 

        *not_flag = 1;
        rule_port++;
    }

    if(rule_port[0] == ':')
    {
        *lo_port = 0;
    }

    toks = mSplit(rule_port, ":", 2, &num_toks, 0);

    switch(num_toks)
    {
        case 1:
            *hi_port = (u_short)ConvPort(toks[0], proto);

            if(rule_port[0] == ':')
            {
                *lo_port = 0;
            }
            else
            {
                *lo_port = *hi_port;

                if(strchr(rule_port, ':') != NULL)
                {
                    *hi_port = MAXPORTS-1;
                }
            }

            break;

        case 2:
            *lo_port = (u_short)ConvPort(toks[0], proto);

            if(toks[1][0] == 0)
                *hi_port = MAXPORTS-1;
            else
                *hi_port = (u_short)ConvPort(toks[1], proto);

            break;

        default:
            ParseError("Port conversion failed on \"%s\".", rule_port);
    }

    mSplitFree(&toks, num_toks);

    return 0;
}

/****************************************************************************
 *
 * Function: ConvPort(char *, char *)
 *
 * Purpose:  Convert the port string over to an integer value
 *
 * Arguments: port => port string
 *            proto => converted integer value of the port
 *
 * Returns:  the port number
 *
 ***************************************************************************/
uint16_t ConvPort(char *port, char *proto)
{
    int conv;           /* storage for the converted number */
    char *digit;      /* used to check for a number */
    struct servent *service_info;

    /*
     * convert a "word port" (http, ftp, imap, whatever) to its corresponding
     * numeric port value
     */
    if(isalpha((int) port[0]) != 0)
    {
        service_info = getservbyname(port, proto);

        if(service_info != NULL)
        {
            conv = ntohs(service_info->s_port);
            return conv;
        }
        else
        {
            ParseError("getservbyname() failed on \"%s\".", port);
        }
    }
    digit = port;
    while (*digit) {

        if(!isdigit((int) *digit))
        {
            ParseError("Invalid port: %s.", port);
        }
        digit++;
    }
    /* convert the value */
    conv = atoi(port);

    /* make sure it's in bounds */
    if ((conv < 0) || (conv > MAXPORTS-1))
    {
        ParseError("Bad port number: %s.", port);
    }

    return (uint16_t)conv;
}

/****************************************************************************
 *
 * Function: XferHeader(RuleTreeNode *, RuleTreeNode *)
 *
 * Purpose: Transfer the rule block header data from point A to point B
 *
 * Arguments: rule => the place to xfer from
 *            rtn => the place to xfer to
 *
 * Returns: void function
 *
 ***************************************************************************/
static void XferHeader(RuleTreeNode *test_node, RuleTreeNode *rtn)
{
    rtn->flags = test_node->flags;
    rtn->type = test_node->type;
    rtn->sip = test_node->sip;
    rtn->dip = test_node->dip;

    rtn->proto = test_node->proto;

    rtn->src_portobject = test_node->src_portobject;
    rtn->dst_portobject = test_node->dst_portobject;
}

/****************************************************************************
 *
 * Function: CompareIPNodes(RuleTreeNode *, RuleTreeNode *).  Support function
 *           for CompareIPLists.
 *
 * Purpose: Checks if the node's contents equal.
 *
 * Returns: 1 if they match, 0 if they don't
 *
 ***************************************************************************/
int CompareIPNodes(IpAddrNode *one, IpAddrNode *two) 
{
#ifdef SUP_IP6
     if( (sfip_compare(one->ip, two->ip) != SFIP_EQUAL) ||
         (sfip_bits(one->ip) != sfip_bits(two->ip)) ||
         (sfvar_flags(one) != sfvar_flags(two)) )
         return 0;
#else

     if( (one->ip_addr != two->ip_addr) ||
         (one->netmask != two->netmask) ||
         (one->addr_flags != two->addr_flags) )
         return 0;
#endif
    return 1;
}

#ifndef SUP_IP6
static int CompareIPAddrSets(IpAddrSet *one, IpAddrSet *two)
{
    if ((one->id != 0) && (one->id == two->id))
        return 1;

    if (!CompareIPLists(one->iplist, two->iplist)) 
        return 0;

    if (!CompareIPLists(one->neg_iplist, two->neg_iplist))
        return 0;

    return 1;
}

/****************************************************************************
 *
 * Function: CompareIPLists(RuleTreeNode *, RuleTreeNode *).  Support function
 *           for TestHeader.
 *
 * Purpose: Checks if all nodes in each list are present in the other
 *
 * Returns: 1 if they match, 0 if they don't
 *
 ***************************************************************************/
static int CompareIPLists(IpAddrNode *one, IpAddrNode *two) 
{
    IpAddrNode *idx1, *idx2;
    int i, match;
    int total1 = 0;
    int total2 = 0;
    char *usage;

    if ((one == NULL) && (two == NULL))
        return 1;

    /* Walk first list.  For each node, check if there is an equal
     * counterpart in the second list.  This method breaks down of there are 
     * duplicated nodes.  For instance, if one = {a, b} and two = {a, a}.
     * Therefore, need additional data structure[s] ('usage') to check off 
     * which nodes have been accounted for already. 
     *
     * Also, the lists are unordered, so comparing node-for-node won't work */

    for(idx1 = one; idx1; idx1 = idx1->next) 
        total1++;
    for(idx2 = two; idx2; idx2 = idx2->next) 
        total2++;

    if(total1 != total2) 
        return 0;

    usage = (char *)SnortAlloc(total1);

    for(idx1 = one; idx1; idx1 = idx1->next, i++)
    {
        match = 0;

        for(idx2 = two, i = 0; idx2; idx2 = idx2->next, i++)
        {
            if(CompareIPNodes(idx1, idx2) && !usage[i])
            {
                match = 1;
                usage[i] = 1;
                break;
            }
        }

        if(!match) {
            free(usage);
            return 0;
        }
    }

    free(usage);
    return 1;
}
#endif

/****************************************************************************
 *
 * Function: TestHeader(RuleTreeNode *, RuleTreeNode *)
 *
 * Purpose: Check to see if the two header blocks are identical
 *
 * Arguments: rule => uh
 *            rtn  => uuuuhhhhh....
 *
 * Returns: 1 if they match, 0 if they don't
 *
 ***************************************************************************/
static int TestHeader(RuleTreeNode * rule, RuleTreeNode * rtn)
{
    if ((rule == NULL) || (rtn == NULL))
        return 0;

    if (rule->type != rtn->type)
        return 0;

    if (rule->proto != rtn->proto)
        return 0;

    /* For custom rule type declarations */
    if (rule->listhead != rtn->listhead)
        return 0;

    if (rule->flags != rtn->flags)
        return 0;

    if ((rule->sip != NULL) && (rtn->sip != NULL) &&
#ifdef SUP_IP6    
            (sfvar_compare(rule->sip, rtn->sip) != SFIP_EQUAL))
#else
            (!CompareIPAddrSets(rule->sip, rtn->sip)))
#endif
    {
        return 0;
    }

    if ((rule->dip != NULL) && (rtn->dip != NULL) &&
#ifdef SUP_IP6    
            (sfvar_compare(rule->dip, rtn->dip) != SFIP_EQUAL))
#else
            (!CompareIPAddrSets(rule->dip, rtn->dip)))
#endif
    {
        return 0;
    }

    /* compare the port group pointers - this prevents confusing src/dst port objects 
     * with the same port set, and it's quicker. It does assume that we only have 
     * one port object and pointer for each unique port set...this is handled by the
     * parsing and initial port object storage and lookup.  This must be consistent during
     * the rule parsing phase. - man */ 
    if ((rule->src_portobject != rtn->src_portobject)
            || (rule->dst_portobject != rtn->dst_portobject))
    {
        return 0;
    }

    return 1;
}

/*
 * PortVarDefine
 *
 *  name - portlist name, i.e. http, smtp, ...
 *  s    - port number, port range, or a list of numbers/ranges in brackets
 * 
 *  examples:
 *  portvar http [80,8080,8138,8700:8800,!8711]
 *  portvar http $http_basic
 */
static int PortVarDefine(SnortConfig *sc, char *name, char *s)
{
    PortObject *po;
    POParser pop;
    char *errstr="unknown";
    int   rstat;
    PortVarTable *portVarTable = sc->targeted_policies[getParserPolicy()]->portVarTable;

    DisallowCrossTableDuplicateVars(sc, name, VAR_TYPE__PORTVAR); 

    if( SnortStrcasestr(s,"any") ) /* this allows 'any' or '[any]' */
    {
        if(strstr(s,"!"))
        {
            ParseError("Illegal use of negation and 'any': %s.", s);
        }

        po = PortObjectNew();
        if( !po )
        {
            FatalError("%s(%d) PortVarTable missing an 'any' variable.\n",
                       __FILE__, __LINE__);
        }
        PortObjectSetName( po, name );
        PortObjectAddPortAny( po );
    }
    else
    {
        /* Parse the Port List info into a PortObject  */
        po = PortObjectParseString(portVarTable, &pop, name, s, 0);
        if(!po)
        {
            errstr = PortObjectParseError( &pop );
            ParseError("*** PortVar Parse error: (pos=%d,error=%s)\n>>%s\n>>%*s.",
                       pop.pos,errstr,s,pop.pos,"^");
        }
    }

    /* Add The PortObject to the PortList Table */
    rstat = PortVarTableAdd(portVarTable, po);
    if( rstat < 0 )
    {
        ParseError("***PortVarTableAdd failed with '%s', exiting.", po->name);
    }
    else if( rstat > 0 )
    {
        ParseMessage("PortVar '%s', already defined.", po->name);
    }

    /* Print the PortList - PortObjects */
    LogMessage("PortVar '%s' defined : ",po->name);
    PortObjectPrintPortsRaw(po);
    LogMessage("\n");

    return 0;
}

/****************************************************************************
 *
 * Function: VarAlloc()
 *
 * Purpose: allocates memory for a variable
 *
 * Arguments: none
 *
 * Returns: pointer to new VarEntry
 *
 ***************************************************************************/
VarEntry *VarAlloc()
{
    VarEntry *new;

    new = (VarEntry *)SnortAlloc(sizeof(VarEntry));

    return(new);
}

#ifdef SUP_IP6
/****************************************************************************
 *
 * Function: VarIsIpAddr(char *, char *)
 *
 * Purpose: Checks if a var is an IP address. Necessary since moving forward
 *          we want all IP addresses handled by the IP variable table.
 *          If a list is given, this checks each value.
 *
 * Arguments: value => the string to check
 *
 * Returns: 1 if IP address, 0 otherwise
 *
 ***************************************************************************/
static int VarIsIpAddr(vartable_t *ip_vartable, char *value)
{
    char *tmp;
   
    /* empty list, consider this an IP address */
    if ((*value == '[') && (*(value+1) == ']'))
        return 1;

    while(*value == '!' || *value == '[') value++;

    /* Check for dotted-quad */
    if( isdigit((int)*value) &&
         ((tmp = strchr(value, (int)'.')) != NULL) && 
         ((tmp = strchr(tmp+1, (int)'.')) != NULL) &&
         (strchr(tmp+1, (int)'.') != NULL))
        return 1; 

    /* IPv4 with a mask, and fewer than 4 fields */
    else if( isdigit((int)*value) &&
         (strchr(value+1, (int)':') == NULL) &&
         ((tmp = strchr(value+1, (int)'/')) != NULL) &&
         isdigit((int)(*(tmp+1))) )
        return 1;

    /* IPv6 */
    else if((tmp = strchr(value, (int)':')) != NULL) 
    {
        char *tmp2;

        if((tmp2 = strchr(tmp+1, (int)':')) == NULL) 
            return 0;

        for(tmp++; tmp < tmp2; tmp++)
            if(!isxdigit((int)*tmp)) 
                return 0;

        return 1;
    }

    /* Any */
    else if(!strncmp(value, "any", 3))
        return 1;

    /* Check if it's a variable containing an IP */
    else if(sfvt_lookup_var(ip_vartable, value+1) || sfvt_lookup_var(ip_vartable, value))
        return 1;

    return 0;
}

/****************************************************************************
 * 
 * Function: CheckBrackets(char *)
 *
 * Purpose: Check that the brackets match up in a string that
 *          represents a list.
 *
 * Arguments: value => the string to check
 *
 * Returns: 1 if the brackets match correctly, 0 otherwise
 *
 ***************************************************************************/
static int CheckBrackets(char *value)
{
    int num_brackets = 0;

    while (*value == '!')
        value++;

    if ((value[0] != '[') || value[strlen(value)-1] != ']')
    {
        /* List does not begin or end with a bracket. */
        return 0;
    }

    while ((*value != '\0') && (num_brackets >= 0))
    {
        if (*value == '[')
            num_brackets++;
        else if (*value == ']')
            num_brackets--;
        value++;
    }
    if (num_brackets != 0)
    {
        /* Mismatched brackets */
        return 0;
    }

    return 1;
}

/****************************************************************************
 *
 * Function: VarIsIpList(vartable_t *, char*)
 *
 * Purpose: Checks if a var is a list of IP addresses.
 *
 * Arguments: value => the string to check
 *
 * Returns: 1 if each item is an IP address, 0 otherwise
 *
 ***************************************************************************/
static int VarIsIpList(vartable_t *ip_vartable, char *value)
{
    char *copy, *item;
    int item_is_ip = 1;

    copy = SnortStrdup((const char*)value);

    /* Ensure that the brackets are correct. */
    if (strchr((const char*)copy, ','))
    {
        /* This is a list! */
        if (CheckBrackets(copy) == 0)
        {
            free(copy);
            return 0;
        }
    }

    /* There's no need to worry about the list structure here.
     * We just strip out the IP delimiters and process each one. */
    item = strtok(copy, "[],!");
    while ((item != NULL) && item_is_ip)
    {
        item_is_ip = VarIsIpAddr(ip_vartable, item);
        item = strtok(NULL, "[],!");
    }

    free(copy);
    return item_is_ip;
}
#endif

/****************************************************************************
 *
 * Function: DisallowCrossTableDuplicateVars(char *, int) 
 *
 * Purpose: FatalErrors if the a variable name is redefined across variable 
 *          types.  Enforcing this mutual exclusion prevents the
 *          catatrophe where the variable lookup fall-through (see VarSearch)
 *          finds an unintended variable from the wrong table.  Note:  VarSearch
 *          is only necessary for ExpandVars. 
 *
 * Arguments: name => The name of the variable
 *            var_type => The type of the variable that is about to be defined.
 *                        The corresponding variable table will not be searched.
 *
 * Returns: void function
 *
 ***************************************************************************/
static void DisallowCrossTableDuplicateVars(SnortConfig *sc, char *name, VarType var_type) 
{
    VarEntry *var_table = sc->targeted_policies[getParserPolicy()]->var_table;
    PortVarTable *portVarTable = sc->targeted_policies[getParserPolicy()]->portVarTable;
    VarEntry *p = var_table;
#ifdef SUP_IP6
    vartable_t *ip_vartable = sc->targeted_policies[getParserPolicy()]->ip_vartable;
#endif

    /* If this is a faked Portvar, treat as a portvar */
    if ((var_type == VAR_TYPE__DEFAULT) &&
        (strstr(name, "_PORT") || strstr(name, "PORT_")))
    {
        var_type = VAR_TYPE__PORTVAR;
    }

    switch (var_type) 
    {
        case VAR_TYPE__DEFAULT:
            if (PortVarTableFind(portVarTable, name)
#ifdef SUP_IP6
                    || sfvt_lookup_var(ip_vartable, name)
#endif
               ) 
            {
                ParseError("Can not redefine variable name %s to be of type "
                           "'var'. Use a different name.", name);
            }
            break;

        case VAR_TYPE__PORTVAR:
            if (var_table != NULL)
            {
                do
                {
                    if(strcasecmp(p->name, name) == 0)
                    {
                        ParseError("Can not redefine variable name %s to be of "
                                   "type 'portvar'. Use a different name.", name);
                    }
                    p = p->next;
                } while(p != var_table);
            }

#ifdef SUP_IP6
            if(sfvt_lookup_var(ip_vartable, name))
            {
                ParseError("Can not redefine variable name %s to be of type "
                           "'portvar'. Use a different name.", name);
            }
#endif /* SUP_IP6 */

            break;

#ifdef SUP_IP6
        case VAR_TYPE__IPVAR:
            if (var_table != NULL)
            {
                do
                {
                    if(strcasecmp(p->name, name) == 0)
                    {
                        ParseError("Can not redefine variable name %s to be of "
                                   "type 'ipvar'. Use a different name.", name);
                    }

                    p = p->next;
                } while(p != var_table);
            }

            if(PortVarTableFind(portVarTable, name)) 
            {
                ParseError("Can not redefine variable name %s to be of type "
                           "'ipvar'. Use a different name.", name);
            }
#endif /* SUP_IP6 */

        default:
            /* Invalid function usage */
            break;
    }
}

/****************************************************************************
 *
 * Function: VarDefine(char *, char *)
 *
 * Purpose: define the contents of a variable
 *
 * Arguments: name => the name of the variable
 *            value => the contents of the variable
 *
 * Returns: void function
 *
 ***************************************************************************/
VarEntry * VarDefine(SnortConfig *sc, char *name, char *value)
{
    VarEntry *var_table = sc->targeted_policies[getParserPolicy()]->var_table;
#ifdef SUP_IP6
    vartable_t *ip_vartable = sc->targeted_policies[getParserPolicy()]->ip_vartable;
#endif
    VarEntry *p;
    uint32_t var_id = 0;

    if(value == NULL)
    {
        ParseError("Bad value in variable definition!  Make sure you don't "
                   "have a \"$\" in the var name.");
    }

#ifdef SUP_IP6
    if(VarIsIpList(ip_vartable, value)) 
    {
        SFIP_RET ret;

        if (ip_vartable == NULL)
            return NULL;

        /* Verify a variable by this name is not already used as either a 
         * portvar or regular var.  Enforcing this mutual exclusion prevents the
         * catatrophe where the variable lookup fall-through (see VarSearch)
         * finds an unintended variable from the wrong table.  Note:  VarSearch
         * is only necessary for ExpandVars. */
        DisallowCrossTableDuplicateVars(sc, name, VAR_TYPE__IPVAR); 

        if((ret = sfvt_define(ip_vartable, name, value)) != SFIP_SUCCESS)
        {
            switch(ret) {
                case SFIP_ARG_ERR:
                    ParseError("The following is not allowed: %s.", value);
                    break;

                case SFIP_DUPLICATE:
                    ParseMessage("Var '%s' redefined.", name);
                    break;

                case SFIP_CONFLICT:
                    ParseError("Negated IP ranges that are more general than "
                               "non-negated ranges are not allowed. Consider "
                               "inverting the logic in %s.", name);
                    break;

                case SFIP_NOT_ANY:
                    ParseError("!any is not allowed in %s.", name);
                    break;

                default:
                    ParseError("Failed to parse the IP address: %s.", value);
            }
        }
        return NULL;
    }
    /* Check if this is a variable that stores an IP */
    else if(*value == '$')
    {
        sfip_var_t *var;
        if((var = sfvt_lookup_var(ip_vartable, value)) != NULL) 
        {
            sfvt_define(ip_vartable, name, value);
            return NULL;
        }
    }

#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
               "VarDefine: name=%s value=%s\n",name,value););

    /* Check to see if this variable is just being aliased */
    if (var_table != NULL)
    {
        VarEntry *tmp = var_table;

        do
        {
            /* value+1 to move past $ */
            if (strcmp(tmp->name, value+1) == 0)
            {
                var_id = tmp->id;
                break;
            }

            tmp = tmp->next;

        } while (tmp != var_table);
    }

    value = ExpandVars(sc, value); 
    if(!value)
    {
        ParseError("Could not expand var('%s').", name);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,
               "VarDefine: name=%s value=%s (expanded)\n",name,value););

    DisallowCrossTableDuplicateVars(sc, name, VAR_TYPE__DEFAULT); 

    if (var_table == NULL)
    {
        p = VarAlloc();
        p->name  = SnortStrdup(name);
        p->value = SnortStrdup(value);
        
        p->prev = p;
        p->next = p;

        sc->targeted_policies[getParserPolicy()]->var_table = p;
        
        if (!var_id)
            p->id = sc->targeted_policies[getParserPolicy()]->var_id++;
        else
            p->id = var_id;

        return p;
    }

    /* See if an existing variable is being redefined */
    p = var_table;

    do
    {
        if (strcasecmp(p->name, name) == 0)
        {
            if (p->value != NULL)
                free(p->value);

            p->value = SnortStrdup(value);
            ParseWarning("Var '%s' redefined\n", p->name);
            return p;
        }

        p = p->next;

    } while (p != var_table);   /* List is circular */

    p = VarAlloc();
    p->name  = SnortStrdup(name);
    p->value = SnortStrdup(value);
    p->prev = var_table;
    p->next = var_table->next;
    p->next->prev = p;
    var_table->next = p;

    if (!var_id)
        p->id = sc->targeted_policies[getParserPolicy()]->var_id++;
    else
        p->id = var_id;

#ifdef XXXXXXX
    vlen = strlen(value);
    LogMessage("Var '%s' defined, value len = %d chars", p->name, vlen  );
 
    if( vlen < 64 )
    {
      LogMessage(", value = %s\n", value );
    }
    else
    {
      LogMessage("\n");
      n = 128;
      s = value;
      while(vlen)
      {
         if( n > vlen ) n = vlen;
         LogMessage("   %.*s\n", n, s );
         s    += n;
         vlen -= n;
      }
    }
#endif

    return p;
}

static void DeleteVars(VarEntry *var_table)
{
    VarEntry *q, *p = var_table;

    while (p)
    {
        q = p->next;
        if (p->name)
            free(p->name);
        if (p->value)
            free(p->value);
        if (p->addrset)
        {
            IpAddrSetDestroy(p->addrset);
            free(p->addrset);
        }
        free(p);
        p = q;
        if (p == var_table)
            break;  /* Grumble, it's a friggin circular list */
    }
}

/****************************************************************************
 *
 * Function: VarGet(char *)
 *
 * Purpose: get the contents of a variable
 *
 * Arguments: name => the name of the variable
 *
 * Returns: char * to contents of variable or FatalErrors on an
 *          undefined variable name
 *
 ***************************************************************************/
char *VarGet(char *name)
{
    SnortConfig *sc = snort_conf_for_parsing;
    SnortPolicy *policy;
    VarEntry *var_table;
#ifdef SUP_IP6
    vartable_t *ip_vartable;
    sfip_var_t *var;
#else
    VarEntry *p = NULL;
    char *ret = NULL;
#endif

    if (sc == NULL)
        return NULL;

    policy = sc->targeted_policies[getParserPolicy()];
    if (policy == NULL)
        return NULL;

    var_table = sc->targeted_policies[getParserPolicy()]->var_table;

#ifdef SUP_IP6
// XXX-IPv6 This function should never be used if IP6 support is enabled!
// Infact it won't presently even work for IP variables since the raw ASCII 
// value is never stored, and is never meant to be used.
    ip_vartable = sc->targeted_policies[getParserPolicy()]->ip_vartable;

    if((var = sfvt_lookup_var(ip_vartable, name)) == NULL) {
        /* Do the old style lookup since it wasn't found in 
         * the variable table */
        if(var_table != NULL)
        {
            VarEntry *p = var_table;
            do
            {
                if(strcasecmp(p->name, name) == 0)
                    return p->value;
                p = p->next;
            } while(p != var_table);
        }

        ParseError("Undefined variable name: %s.", name);
    }

    return name;

#else

    if (var_table != NULL)
    {
        p = var_table;

        do
        {
            if (strcasecmp(p->name, name) == 0)
            {
                ret = p->value;
                break;
            }

            p = p->next;

        } while (p != var_table);
    }

    if (ret == NULL)
        ParseError("Undefined variable name: %s.", name);

    return ret;
#endif
}

/****************************************************************************
 *
 * Function: ExpandVars()
 *
 * Purpose: expand all variables in a string
 *
 * Arguments:
 *  SnortConfig *
 *      The snort config that has the vartables.
 *  char *
 *      The name of the variable.
 *
 * Returns:
 *  char *
 *      The expanded string.  Note that the string is returned in a 
 *      static variable and most likely needs to be string dup'ed.
 *
 ***************************************************************************/
static char * ExpandVars(SnortConfig *sc, char *string)
{
    static char estring[ PARSERULE_SIZE ];

    char rawvarname[128], varname[128], varaux[128], varbuffer[128];
    char varmodifier, *varcontents;
    int varname_completed, c, i, j, iv, jv, l_string, name_only;
    int quote_toggle = 0;

    if(!string || !*string || !strchr(string, '$'))
        return(string);

    bzero((char *) estring, PARSERULE_SIZE);

    i = j = 0;
    l_string = strlen(string);
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "ExpandVars, Before: %s\n", string););

    while(i < l_string && j < (int)sizeof(estring) - 1)
    {
        c = string[i++];
        
        if(c == '"')
        {
            /* added checks to make sure that we are inside a quoted string
             */
            quote_toggle ^= 1;
        }

        if(c == '$' && !quote_toggle)
        {
            bzero((char *) rawvarname, sizeof(rawvarname));
            varname_completed = 0;
            name_only = 1;
            iv = i;
            jv = 0;

            if(string[i] == '(')
            {
                name_only = 0;
                iv = i + 1;
            }

            while(!varname_completed
                  && iv < l_string
                  && jv < (int)sizeof(rawvarname) - 1)
            {
                c = string[iv++];

                if((name_only && !(isalnum(c) || c == '_'))
                   || (!name_only && c == ')'))
                {
                    varname_completed = 1;

                    if(name_only)
                        iv--;
                }
                else
                {
                    rawvarname[jv++] = (char)c;
                }
            }

            if(varname_completed || iv == l_string)
            {
                char *p;

                i = iv;

                varcontents = NULL;

                bzero((char *) varname, sizeof(varname));
                bzero((char *) varaux, sizeof(varaux));
                varmodifier = ' ';

                p = strchr(rawvarname, ':');
                if (p)
                {
                    SnortStrncpy(varname, rawvarname, p - rawvarname);

                    if(strlen(p) >= 2)
                    {
                        varmodifier = *(p + 1);
                        SnortStrncpy(varaux, p + 2, sizeof(varaux));
                    }
                }
                else
                    SnortStrncpy(varname, rawvarname, sizeof(varname));

                bzero((char *) varbuffer, sizeof(varbuffer));

                varcontents = VarSearch(sc, varname);

                switch(varmodifier)
                {
                    case '-':
                        if(!varcontents || !strlen(varcontents))
                            varcontents = varaux;
                        break;

                    case '?':
                        if(!varcontents || !strlen(varcontents))
                        {
                            ErrorMessage("%s(%d): ", file_name, file_line);

                            if(strlen(varaux))
                                ParseError("%s", varaux);
                            else
                                ParseError("Undefined variable \"%s\".", varname);
                        }
                        break;
                }

                /* If variable not defined now, we're toast */
                if(!varcontents || !strlen(varcontents))
                    ParseError("Undefined variable name: %s.", varname);

                if(varcontents)
                {
                    int l_varcontents = strlen(varcontents);

                    iv = 0;

                    while(iv < l_varcontents && j < (int)sizeof(estring) - 1)
                        estring[j++] = varcontents[iv++];
                }
            }
            else
            {
                estring[j++] = '$';
            }
        }
        else
        {
            estring[j++] = (char)c;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "ExpandVars, After: %s\n", estring););

    return estring;
}

/******************************************************************
 *
 * Function: LinkDynamicRules()
 *
 * Purpose: Move through the activation and dynamic lists and link
 *          the activation rules to the rules that they activate.
 *
 * Arguments:
 *  SnortConfig *
 *
 * Returns: void function
 *
 ******************************************************************/
static void LinkDynamicRules(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    SetLinks(sc, IPPROTO_TCP);
    SetLinks(sc, IPPROTO_UDP);
    SetLinks(sc, IPPROTO_ICMP);
}

/******************************************************************
 *
 * Function: SetLinks()
 *
 * Purpose: Move through the activation and dynamic lists and link
 *          the activation rules to the rules that they activate.
 *
 * Arguments: activator => the activation rules
 *            activatee => the rules being activated
 *
 * Returns: void function
 *
 ******************************************************************/
static void SetLinks(SnortConfig *sc, int proto)
{
    OptTreeNode *otn;

    RuleTreeNode *rtn;
    SFGHASH *actHash = NULL;
    tSfPolicyId policyId = 0;

    //create a temporary hash with key as policyId, activation number
    //and data elements as otn/rtn for activated_by.
    struct _activateKey {
        int policyId;
        int activates;
    } activateKey;

    struct _activateData {
        RuleTreeNode *rtn;
        OptTreeNode  *otn;
    } *activateData;

    actHash = sfghash_new(10000,sizeof(struct _activateKey),0,free);

    /* walk thru the RTN list */
    for (otn = firstHeadNode(sc, proto, RULE_TYPE__ACTIVATE, &policyId);
         otn;
         otn = nextHeadNode(sc, proto, RULE_TYPE__ACTIVATE, &policyId))
    {
        if (otn->activates)
        {
            //add to hash
            if ((activateData = calloc(sizeof(struct _activateData), 1)) != NULL)
            {
                activateKey.policyId = policyId;
                activateKey.activates = otn->activates;
                activateData->rtn = otn->proto_nodes[getParserPolicy()];
                activateData->otn = otn;

                sfghash_add(actHash, &activateKey, activateData);
            }

        }
    }

    for (otn = firstHeadNode(sc, proto, RULE_TYPE__DYNAMIC, &policyId);
         otn;
         otn = nextHeadNode(sc, proto, RULE_TYPE__DYNAMIC, &policyId))
    {
        ActivateListNode *act_list;

        rtn = otn->proto_nodes[getParserPolicy()];

        //for (act_list = rtn->activate_list[policyId];
        for (act_list = rtn->activate_list;
             act_list;
             act_list = act_list->next)
        {
            activateKey.policyId = policyId;
            activateKey.activates = act_list->activated_by;

            activateData = sfghash_find(actHash, &activateKey);
            if(activateData)
            {
                activateData->otn->RTN_activation_ptr = otn->proto_nodes[getParserPolicy()];
                //activateData->RTN_activation_ptr = otn->proto_node[policyId];
                activateData->otn->OTN_activation_ptr = otn; 
                sfghash_remove(actHash, &activateKey);
            }
            else
            {
                //activate rule is present but the activated rule is missing
            }
        }
    }

    sfghash_delete(actHash);
    actHash = NULL;
}

char * ProcessFileOption(SnortConfig *sc, const char *filespec)
{
    char *filename = NULL;
    char buffer[STD_BUF];

    if (sc == NULL)
        sc = snort_conf;

    if(filespec == NULL)
    {
        FatalError("no arguement in this file option, remove extra ':' at the end of the alert option\n");
    }

    /* look for ".." in the string and complain and exit if it is found */
    if(strstr(filespec, "..") != NULL)
    {
        FatalError("file definition contains \"..\".  Do not do that!\n");
    }

    if(filespec[0] == '/')
    {
        /* absolute filespecs are saved as is */
        filename = SnortStrdup(filespec);
    }
    else
    {
        /* relative filespec is considered relative to the log directory */
        /* or /var/log if the log directory has not been set */
        /* Make sure this function isn't called before log dir is set */
        if ((sc != NULL) && (sc->log_dir != NULL))
        {
            strlcpy(buffer, snort_conf->log_dir, STD_BUF);
        }
        else
        {
            strlcpy(buffer, "/var/log/snort", STD_BUF);
        }

        strlcat(buffer, "/", STD_BUF - strlen(buffer));
        strlcat(buffer, filespec, STD_BUF - strlen(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        filename = SnortStrdup(buffer);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"ProcessFileOption: %s\n", filename););

    return filename;
}


#ifdef TARGET_BASED
static void ParseAttributeTable(SnortConfig *sc, SnortPolicy *p, char *args)
{
    tSfPolicyId currentPolicyId = getParserPolicy();
    tSfPolicyId defaultPolicyId = sfGetDefaultPolicy(sc->policy_config);
    TargetBasedConfig *defTbc = &sc->targeted_policies[defaultPolicyId]->target_based_config;

    /* Save for configuring after configuration is parsed in case
     * config max_attribute_hosts is configured after this */
    if ((currentPolicyId != defaultPolicyId)
            && ((defTbc->args == NULL) || (strcmp(args, defTbc->args) != 0)))
    {
        //arguments should be same as in default policy. Ignoring the arguments
        ParseError("Attribute table must be configured in default policy if "
                   "it is to be used in other policies and attribute table "
                   "filename must be the same across policies.");
    }

    p->target_based_config.args = SnortStrdup(args);

    if (file_name != NULL)
    {
        p->target_based_config.file_name = SnortStrdup(file_name);
        p->target_based_config.file_line = file_line;
    }
}
#endif

static void ParseConfig(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    char *opts = NULL;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Rule file config\n"););

    toks = mSplit(args, ":", 2, &num_toks, 0);

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Opt: %s\n", toks[0]););

    if (num_toks > 1)
    {
        /* Dup the opts because we're putting into hash table */
        opts = SnortStrdup(toks[1]);
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Args: %s\n", opts););
    }

    switch (sfghash_add(sc->config_table, toks[0], opts))
    {
        case SFGHASH_NOMEM:
            FatalError("%s(%d) No memory to add entry to config table.\n",
                       __FILE__, __LINE__);
            break;

        case SFGHASH_INTABLE:
            /* Only reference and classifications are likely dup candidates
             * right now and we're not too worried about keeping track of
             * all of them */
            if (opts != NULL)
            {
                free(opts);
                opts = toks[1];
            }

            break;

        default:
            break;
    }

    for (i = 0; config_opts[i].name != NULL; i++)
    {
        if (strcasecmp(toks[0], config_opts[i].name) == 0)
        {
	        if ((getParserPolicy() != getDefaultPolicy()) &&
                config_opts[i].default_policy_only)
	        {
	        	/* Config option configurable on by the default policy*/
                /**Dont raise parse error, ignore any config that is not allowed in non-default
                 * policy.
                 */
                DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Config option \"%s\" "
                    "configurable only by default policy. Ignoring it\n", toks[0]));
                break;
	        }

            if (config_opts[i].only_once && config_opt_configured[i])
            {
                /* Configured already and set to only configure once
                 * This array is reset for each policy read in so this is
                 * on a per policy basis */
                ParseError("Config option \"%s\" can only be "
                           "configured once.", toks[0]);
            }

            if (config_opts[i].args_required && (opts == NULL))
            {
                /* Need arguments and there are none */
                 ParseError("Config option \"%s\" requires arguments.", toks[0]);
            }

            config_opts[i].parse_func(sc, opts);
            config_opt_configured[i] = 1;
            break;
        }
    }

    if (config_opts[i].name == NULL)
    {
        /* Didn't find a matching config option */
        ParseError("Unknown config directive: %s.", toks[0]);
    }

    mSplitFree(&toks, num_toks);
}

/****************************************************************************
 *
 * Purpose: Check that special rules have an OTN.
 *          TODO: Free up memory associated with disabled rules.
 *
 * Arguments: list => Pointer for a list of rules
 *
 * Returns: void function
 *
 * Notes: man - modified to used .shared flag in otn sigInfo instead of specialGID
 *        sas - removed specialGID
 * 
 *****************************************************************************/
int CheckRuleStates(SnortConfig *sc)
{
    RuleTreeNode *rtn;
    OptTreeNode *otn;
    SFGHASH_NODE *hashNode;
    int oneErr = 0;
    tSfPolicyId policyId = 0;

    if (sc == NULL)
        return 0;

    for (hashNode = sfghash_findfirst(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        for (policyId = 0; 
             policyId < otn->proto_node_num; 
             policyId++)
        {
            rtn = otn->proto_nodes[policyId];

            if (!rtn)
            {
                continue;
            }

            if ((rtn->proto == IPPROTO_TCP) || (rtn->proto == IPPROTO_UDP) ||
                (rtn->proto == IPPROTO_ICMP) || (rtn->proto == ETHERNET_TYPE_IP)) 
            { 
                //do operation
                if ( otn->sigInfo.shared )
                {
                    if (otn->ds_list[PLUGIN_DYNAMIC] == NULL)
                    {
                        // Have a dynamic rule but no dynamic plugin
                        if (otn->sigInfo.id != otn->sigInfo.otnKey.sid)
                        {
                            // If its a different SID, but same soid metadata as something
                            // else, try to find it
                            OptTreeNode *otn_original;
                            otn_original = SoRuleOtnLookup(sc->so_rule_otn_map,
                                otn->sigInfo.otnKey.gid, otn->sigInfo.otnKey.sid);
                            if (otn_original)
                            {
                                OptFpList *opt_func = otn->opt_func;
                                while (opt_func != NULL)
                                {
                                    /* Delete the option functions that came from the
                                     * parsing -- this rule will be identical to its
                                     * "cloned" brother. */
                                    OptFpList *tmp = opt_func;
                                    opt_func = opt_func->next;
                                    free(tmp);
                                }
                                if (otn_original->sigInfo.shared)
                                {
                                    /* Its still a shared object -- has its own detection function.  */
                                    otn->ds_list[PLUGIN_DYNAMIC] = otn_original->ds_list[PLUGIN_DYNAMIC];
                                }
                                else
                                {
                                    /* It was back-converted from a shared object */
                                    int i;
                                    for (i=PLUGIN_CLIENTSERVER; i<PLUGIN_MAX; i++)
                                    {
                                        otn->ds_list[i] = otn_original->ds_list[i];
                                    }
                                    otn->sigInfo.shared = 0; /* no longer shared */
                                }
                                otn->opt_func = otn_original->opt_func;
                                otn->sigInfo.dup_opt_func = 1;
                            }
                        }
                    }

                    if (otn->sigInfo.shared && (otn->ds_list[PLUGIN_DYNAMIC] == NULL))
                    {
                        /* If still shared... */
                        ParseWarning("Encoded Rule Plugin SID: %d, GID: %d not "
                                   "registered properly.  Disabling this rule.\n",
                                   otn->sigInfo.id, otn->sigInfo.generator);
                        oneErr = 1;
                        otn->rule_state = RULE_STATE_DISABLED;
                    }
                }
            }
        }
    }

    return oneErr;
}

/****************************************************************************
 *
 * Purpose: Adjust the information for a given rule
 *          relative to the Rule State list
 *
 * Arguments: None
 *
 * Returns: void function
 *
 * Notes:  specialGID is depracated, uses sigInfo.shared flag
 * 
 *****************************************************************************/
void SetRuleStates(SnortConfig *sc)
{
    RuleState *rule_state;
    int oneErr = 0, err;

    if (sc == NULL)
        return;

    /* First, cycle through the rule state list and update the
     * rule state for each one we find. */
    for (rule_state = sc->rule_state_list; rule_state != NULL; rule_state = rule_state->next)
    {
        /* Lookup the OTN by ruleState->sid, ruleState->gid */
        OptTreeNode *otn = OtnLookup(sc->otn_map, rule_state->gid, rule_state->sid);

        if (otn == NULL)
        {
            FatalError("Rule state specified for invalid SID: %d GID: %d\n",
                       rule_state->sid, rule_state->gid);
        }

        otn->rule_state = rule_state->state;
    }

    /* Check TCP/UDP/ICMP/IP in one iteration for all rulelists and for all policies*/
    err = CheckRuleStates(sc);
    if (err)
        oneErr = 1;

#ifdef DYNAMIC_PLUGIN
#if 0
    if (oneErr)
    {
        FatalError("Misconfigured or unregistered encoded rule plugins\n");
    }
#endif
#endif
}

/****************************************************************************
 *
 * Purpose: Parses a rule state line.
 *          Format is sid, gid, state, action.
 *          state should be "enabled" or "disabled"
 *          action should be "alert", "drop", "sdrop", "log", etc.
 *
 * Arguments: args => string containing a single rule state entry
 *
 * Returns: void function
 *
 *****************************************************************************/
static void ParseRuleState(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    RuleState *state;
    char *endptr;

    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"RuleState\n"););

    toks = mSplit(args, ", ", 0, &num_toks, 0);

    if (num_toks != 4)
        ParseError("Config rule_state: Empty state info.");

    state = (RuleState *)SnortAlloc(sizeof(RuleState));

    state->sid = SnortStrtoul(toks[0], &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid sid for rule state: %s.  Sid must be between 0 and "
                   "%u inclusive.", args, UINT32_MAX);
    }

    state->gid = SnortStrtoul(toks[1], &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid gid for rule state: %s.  Gid must be between 0 and "
                   "%u inclusive.", args, UINT32_MAX);
    }

    if (strcasecmp(toks[2], RULE_STATE_OPT__DISABLED) == 0)
    {
        state->state = RULE_STATE_DISABLED;
    }
    else if (strcasecmp(toks[2], RULE_STATE_OPT__ENABLED) == 0)
    {
        state->state = RULE_STATE_ENABLED;
    }
    else
    {
        ParseError("Rule_state: Invalid state - must be either "
                   "'enabled' or 'disabled'.");
    }

    state->action = GetRuleType(toks[3]);
    if (state->action == RULE_TYPE__NONE)
    {
        ParseError("Rule_state: Invalid action - must be a valid "
                   "rule type.");
    }

    mSplitFree(&toks, num_toks);

    if (sc->rule_state_list == NULL)
    {
        sc->rule_state_list = state;
    }
    else
    {
        state->next = sc->rule_state_list;
        sc->rule_state_list = state;
    }
}

#ifdef DYNAMIC_PLUGIN
static void ParseDynamicLibInfo(DynamicLibInfo *dylib_info, char *args)
{
    char getcwd_path[PATH_MAX];
    char **toks = NULL;
    int num_toks = 0;
    char *path = NULL;
    PathType ptype = PATH_TYPE__FILE;
    DynamicLibPath *dylib_path;
    struct stat buf;

    if (dylib_info == NULL)
        return;

    if (dylib_info->count >= MAX_DYNAMIC_LIBS)
    {
        ParseError("Maximum number of loaded libriaries of this dynamic "
                   "library type exceeded: %d.", MAX_DYNAMIC_LIBS);
    }

    if (args == NULL)
    {
        if (getcwd(getcwd_path, sizeof(getcwd_path)) == NULL)
        {
            ParseError("Dynamic library path too long.  If you really "
                       "think your path needs to be as long as it is, please "
                       "submit a bug to bugs@snort.org.");
        }

        path = getcwd_path;
        ptype = PATH_TYPE__DIRECTORY;
    }
    else
    {
        toks = mSplit(args, " \t", 0, &num_toks, 0);

        if (num_toks == 1)
        {
            path = toks[0];
            ptype = PATH_TYPE__FILE;
        }
        else if (num_toks == 2)
        {
            if (strcasecmp(toks[0], DYNAMIC_LIB_OPT__FILE) == 0)
            {
                ptype = PATH_TYPE__FILE;
            }
            else if (strcasecmp(toks[0], DYNAMIC_LIB_OPT__DIRECTORY) == 0)
            {
                ptype = PATH_TYPE__DIRECTORY;
            }
            else
            {
                ParseError("Invalid specifier for Dynamic library specifier.  "
                           "Should be file|directory pathname.");
            }

            path = toks[1];
        }
        else
        {
            ParseError("Missing/incorrect dynamic engine lib specifier.");
        }
    }

    dylib_path = (DynamicLibPath *)SnortAlloc(sizeof(DynamicLibPath));
    dylib_path->ptype = ptype;
    dylib_path->path = SnortStrdup(path);

    dylib_info->lib_paths[dylib_info->count] = dylib_path;
    dylib_info->count++;

    if (toks != NULL)
        mSplitFree(&toks, num_toks);

    if (stat(dylib_path->path, &buf) == -1)
    {
        FatalError("%s(%d) Could not stat dynamic module "
                   "path \"%s\": %s.\n", __FILE__, __LINE__,
                   dylib_path->path, strerror(errno));
    }

    dylib_path->last_mod_time = buf.st_mtime;
}

/****************************************************************************
 *
 * Purpose: Parses a dynamic engine line
 *          Format is full path of dynamic engine
 *
 * Arguments: args => string containing a single dynamic engine
 *
 * Returns: void function
 *
 *****************************************************************************/
static void ParseDynamicEngineInfo(SnortConfig *sc, SnortPolicy *p, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"DynamicEngine\n"););

    if (sc->dyn_engines == NULL)
    {
        sc->dyn_engines = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
        sc->dyn_engines->type = DYNAMIC_TYPE__ENGINE;
    }

    ParseDynamicLibInfo(sc->dyn_engines, args);
}

/****************************************************************************
 *
 * Purpose: Parses a dynamic detection lib line
 *          Format is full path of dynamic engine
 *
 * Arguments: args => string containing a single dynamic engine
 *
 * Returns: void function
 *
 *****************************************************************************/
static void ParseDynamicDetectionInfo(SnortConfig *sc, SnortPolicy *p, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"DynamicDetection\n"););

    if (sc->dyn_rules == NULL)
    {
        sc->dyn_rules = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
        sc->dyn_rules->type = DYNAMIC_TYPE__DETECTION;
    }

    ParseDynamicLibInfo(sc->dyn_rules, args);
}

/****************************************************************************
 *
 * Purpose: Parses a dynamic preprocessor lib line
 *          Format is full path of dynamic engine
 *
 * Arguments: args => string containing a single dynamic engine
 *
 * Returns: void function
 *
 *****************************************************************************/
static void ParseDynamicPreprocessorInfo(SnortConfig *sc, SnortPolicy *p, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"DynamicPreprocessor\n"););

    if (sc->dyn_preprocs == NULL)
    {
        sc->dyn_preprocs = (DynamicLibInfo *)SnortAlloc(sizeof(DynamicLibInfo));
        sc->dyn_preprocs->type = DYNAMIC_TYPE__PREPROCESSOR;
    }

    ParseDynamicLibInfo(sc->dyn_preprocs, args);
}
#endif

/* verify that we are not reusing some other keyword */
static int ValidateUserDefinedRuleType(SnortConfig *sc, char *keyword)
{
    RuleListNode *node;

    if ((sc == NULL) || (sc->rule_lists == NULL))
        return 0;

    node = sc->rule_lists;

    /* This keyword cannot match any of our predefined rule types */
    if (GetRuleType(keyword) != RULE_TYPE__NONE)
        return 0;

    /* Walk through the rule list to make sure the user didn't already
     * define this one */
    while (node != NULL)
    {
        if (strcasecmp(node->name, keyword) == 0)
            return 0;

        node = node->next;
    }

    return 1;
}

/* This function does nothing.  It is just a place holder in the snort conf
 * keyword array so the keyword can be matched.  Its's a special configuration
 * case in that multiple non-escaped lines need to be read and the current
 * file pointer needs to be passed in. */
static void ParseRuleTypeDeclaration(SnortConfig *sc, SnortPolicy *p, char *arg)
{
    return;
}

static void _ParseRuleTypeDeclaration(SnortConfig *sc, FILE *fp, char *arg, int prules)
{
    char **toks;
    int num_toks;
    char *input;
    char *rule_type_name;
    RuleType type;
    int rval = 1;
    ListHead *listhead = NULL;
    int got_output = 0;

    if ((sc == NULL) || (fp == NULL) || (arg == NULL))
        return;

    /* Already parsed this or ignoring for any non-default policy, but need to move past 
     * the rule declaration because it doesn't have continuation characters 
     */
    if (prules  /* parsing rules */
            || (getParserPolicy() != getDefaultPolicy()))
    {
        while (1)
        {
            input = ReadLine(fp);
            if (input == NULL)
                ParseError("Rule type declaration syntax error: %s.", arg);

            toks = mSplit(input, " \t", 2, &num_toks, 0);

            /* Just continue for blank line */
            if (toks == NULL)
                continue;

            /* Got end of rule type */
            if ((num_toks == 1) && (strcmp(toks[0], "}") == 0))
            {
                free(input);
                mSplitFree(&toks, num_toks);
                break;
            }

            free(input);
            mSplitFree(&toks, num_toks);
        }

        return;
    }


    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Rule type declaration\n"););

    toks = mSplit(arg, " \t", 2, &num_toks, 0);

    /* Need rule type name for creating new node in rule list */
    rule_type_name = SnortStrdup(ExpandVars(sc, toks[0]));

    /* Verify keyword is unique */
    if (!ValidateUserDefinedRuleType(sc, rule_type_name))
    {
        ParseError("Duplicate rule type declaration found: %s.", rule_type_name);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Declaring new rule type: %s\n",
                            rule_type_name););

    if (num_toks == 2)
    {
        /* User put '{' on the same line, which is okay */
        if ((toks[1] != NULL) && (strcmp(toks[1], "{") != 0))
        {
            ParseError("Rule type declaration syntax error: %s.", arg);
        }
    }
    else
    {
        /* Get next line.  It should only be '{' */
        input = ReadLine(fp);
        if ((input == NULL) || (strcmp(input, "{") != 0))
        {
            ParseError("Rule type declaration syntax error: %s.", arg);
        }

        free(input);
    }

    mSplitFree(&toks, num_toks);

    input = ReadLine(fp);
    if (input == NULL)
        ParseError("Rule type declaration syntax error: %s.", arg);

    toks = mSplit(input, " \t", 2, &num_toks, 0);
    if ((num_toks != 2) ||
        (strcasecmp(toks[0], RULE_TYPE_OPT__TYPE) != 0))
    {
        ParseError("Rule type declaration syntax error: %s.", arg);
    }

    type = GetRuleType(toks[1]);
    if (type == RULE_TYPE__NONE)
        ParseError("Invalid type for rule type declaration: %s.", toks[1]);

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"\ttype(%i): %s\n", type, toks[1]););

    if (type == RULE_TYPE__PASS)
        rval = 0;

    listhead = CreateRuleType(sc, rule_type_name, type, rval, NULL);

    free(rule_type_name);
    free(input);
    mSplitFree(&toks, num_toks);

    /* Get output plugin declarations
     * This will break if '}' is found on the line and fatal error if not and
     * the line isn't an output configuration */
    while (1)
    {
        input = ReadLine(fp);
        if (input == NULL)
            ParseError("Rule type declaration syntax error: %s.", arg);

        toks = mSplit(input, " \t", 2, &num_toks, 0);

        /* Just continue for blank line */
        if (toks == NULL)
            continue;

        /* Got end of rule type */
        if ((num_toks == 1) && (strcmp(toks[0], "}") == 0))
        {
            free(input);
            mSplitFree(&toks, num_toks);
            break;
        }

        if ((num_toks != 2) ||
            (strcasecmp(toks[0], SNORT_CONF_KEYWORD__OUTPUT) != 0))
        {
            ParseError("Rule type declaration syntax error: %s.  This line "
                       "should contain an output declaration.", toks[0]);
        }

        ParseRuleTypeOutput(sc, toks[1], listhead);

        free(input);
        mSplitFree(&toks, num_toks);

        got_output = 1;
    }

    if (!got_output)
        ParseError("Rule type declaration requires an output configuration.");

    sc->num_rule_types++;
}

/* adapted from ParseRuleFile in rules.c
 * Returns NULL if the end of file is reached or some other strange file
 * reading error should occur
 * Will read lines until it finds one that isn't empty or commented
 * Returned string, if not NULL, needs to be freed */
char * ReadLine(FILE * file)
{
    char *buf = (char *)SnortAlloc(MAX_LINE_LENGTH + 1);

    /* Read a line from file and return it. Return NULL for lines that
     * are comment characters or empty */
    while ((fgets(buf, MAX_LINE_LENGTH, file)) != NULL)
    {
        int i;
        char *index;
        char *ret_line;

        file_line++;
        index = buf;

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Got line %s (%d): %s\n",
                                file_name, file_line, buf););

        /* advance through any whitespace at the beginning of the line */
        while (isspace((int)*index))
            index++;

        /* If it's an empty line or starts with a comment character */
        if ((strlen(index) == 0) || (*index == '#') || (*index == ';'))
            continue;

        /* Trim off any whitespace at the end of the line */
        for (i = strlen(index); i > 0; i--)
        {
            if (!isspace((int)index[i - 1]))
                break;
        }

        index[i] = '\0';

        /* return a copy of the line */
        ret_line = SnortStrdup(index);
        free(buf);
        return ret_line;
    }

    free(buf);
    return NULL;
}

/*
 * Same as VarGet - but this does not Fatal out if a var is not found
 */
static char * VarSearch(SnortConfig *sc, char *name)
{
    VarEntry *var_table = sc->targeted_policies[getParserPolicy()]->var_table;
    PortVarTable *portVarTable = sc->targeted_policies[getParserPolicy()]->portVarTable;
#ifdef SUP_IP6
    vartable_t *ip_vartable = sc->targeted_policies[getParserPolicy()]->ip_vartable;
    sfip_var_t *ipvar;

    if ((ipvar = sfvt_lookup_var(ip_vartable, name)) != NULL) 
        return ExpandVars(sc, ipvar->value); 
#endif

    /* XXX Return a string value */
    if (PortVarTableFind(portVarTable, name)) 
        return name;

    if (var_table != NULL)
    {
        VarEntry *p = var_table;
        do
        {
            if(strcasecmp(p->name, name) == 0)
                return p->value;
            p = p->next;

        } while(p != var_table);
    }

    return NULL;
}

/*****************************************************************
 * Function: GetPcaps()
 *
 * This function takes a list of pcap types and arguments from
 * the command line, parses them depending on type and puts them
 * in a user supplied queue. The pcap object list will contain
 * PcapReadObject structures.  The returned queue contains
 * strings representing paths to pcaps.
 *
 * returns -1 on error and 0 on success
 *
 ****************************************************************/
int GetPcaps(SF_LIST *pol, SF_QUEUE *pcap_queue)
{
    PcapReadObject *pro = NULL;
    int type = 0;
    char *arg = NULL;
    char *filter = NULL;
    int ret = 0;

    if ((pol == NULL) || (pcap_queue == NULL))
        return -1;

    for (pro = (PcapReadObject *)sflist_first(pol);
         pro != NULL;
         pro = (PcapReadObject *)sflist_next(pol))
    {
        type = pro->type;
        arg = pro->arg;
        filter = pro->filter;

        switch (type)
        {
            case PCAP_SINGLE:
                {
                    char *pcap = NULL;
                    struct stat stat_buf;

                    /* Don't check file if reading from stdin */
                    if (strcmp(arg, "-") != 0)
                    {
                        /* do a quick check to make sure file exists */
                        if (stat(arg, &stat_buf) == -1)
                        {
                            ErrorMessage("Error getting stat on pcap file: %s: %s\n",
                                         arg, strerror(errno));
                            return -1;
                        }
                        else if (!(stat_buf.st_mode & (S_IFREG|S_IFIFO)))
                        {
                            ErrorMessage("Specified pcap is not a regular file: %s\n", arg);
                            return -1;
                        }
                    }

                    pcap = SnortStrdup(arg);
                    ret = sfqueue_add(pcap_queue, (NODE_DATA)pcap);
                    if (ret == -1)
                    {
                        ErrorMessage("Could not add pcap to pcap list\n");
                        free(pcap);
                        return -1;
                    }
                }

                break;

            case PCAP_FILE_LIST:
                /* arg should be a file with a list of pcaps in it */
                {
                    FILE *pcap_file = NULL;
                    char *pcap = NULL;
                    char path_buf[4096];   /* max chars we'll accept for a path */

                    pcap_file = fopen(arg, "r");
                    if (pcap_file == NULL)
                    {
                        ErrorMessage("Could not open pcap list file: %s: %s\n",
                                     arg, strerror(errno));
                        return -1;
                    }

                    while (fgets(path_buf, sizeof(path_buf), pcap_file) != NULL)
                    {
                        char *path_buf_ptr, *path_buf_end;
                        struct stat stat_buf;

                        path_buf[sizeof(path_buf) - 1] = '\0';
                        path_buf_ptr = &path_buf[0];
                        path_buf_end = path_buf_ptr + strlen(path_buf_ptr);

                        /* move past spaces if any */
                        while (isspace((int)*path_buf_ptr))
                            path_buf_ptr++;

                        /* if nothing but spaces on line, continue */
                        if (*path_buf_ptr == '\0')
                            continue;

                        /* get rid of trailing spaces */
                        while ((path_buf_end > path_buf_ptr) &&
                               (isspace((int)*(path_buf_end - 1))))
                            path_buf_end--;

                        *path_buf_end = '\0';

                        /* do a quick check to make sure file exists */
                        if (stat(path_buf_ptr, &stat_buf) == -1)
                        {
                            ErrorMessage("Error getting stat on pcap file: %s: %s\n",
                                         path_buf_ptr, strerror(errno));
                            fclose(pcap_file);
                            return -1;
                        }
#ifndef WIN32
                        else if (stat_buf.st_mode & S_IFDIR)
                        {
                            ret = GetFilesUnderDir(path_buf_ptr, pcap_queue, filter);
                            if (ret == -1)
                            {
                                ErrorMessage("Error getting pcaps under dir: %s\n", path_buf_ptr);
                                fclose(pcap_file);
                                return -1;
                            }
                        }
#endif
                        else if (stat_buf.st_mode & S_IFREG)
                        {
#ifndef WIN32
                            if ((filter == NULL) || (fnmatch(filter, path_buf_ptr, 0) == 0))
                            {
#endif
                                pcap = SnortStrdup(path_buf_ptr);
                                ret = sfqueue_add(pcap_queue, (NODE_DATA)pcap);
                                if (ret == -1)
                                {
                                    ErrorMessage("Could not insert pcap into list: %s\n", pcap);
                                    free(pcap);
                                    fclose(pcap_file);
                                    return -1;
                                }
#ifndef WIN32
                            }
#endif
                        }
                        else
                        {
#ifdef WIN32
                            ErrorMessage("Specified entry in \'%s\' is not a regular file: %s\n",
                                         arg, path_buf_ptr);
#else
                            ErrorMessage("Specified entry in \'%s\' is not a regular file or directory: %s\n",
                                         arg, path_buf_ptr);
#endif
                            fclose(pcap_file);
                            return -1;
                        }
                    }

                    fclose(pcap_file);
                }

                break;

            case PCAP_LIST:
                /* arg should be a space separated list of pcaps */
                {
                    char *tmp = NULL;
                    char *pcap = NULL;
                    struct stat stat_buf;

                    tmp = strtok_r(arg, " ", &arg);
                    if (tmp == NULL)
                    {
                        ErrorMessage("No pcaps specified in pcap list\n");
                        return -1;
                    }

                    do
                    {
                        /* do a quick check to make sure file exists */
                        if (stat(tmp, &stat_buf) == -1)
                        {
                            ErrorMessage("Error getting stat on file: %s: %s\n",
                                       tmp, strerror(errno));
                            return -1;
                        }
                        else if (!(stat_buf.st_mode & (S_IFREG|S_IFIFO)))
                        {
                            ErrorMessage("Specified pcap is not a regular file: %s\n", tmp);
                            return -1;
                        }

                        pcap = SnortStrdup(tmp);
                        ret = sfqueue_add(pcap_queue, (NODE_DATA)pcap);
                        if (ret == -1)
                        {
                            ErrorMessage("Could not insert pcap into list: %s\n", pcap);
                            free(pcap);
                            return -1;
                        }

                    } while ((tmp = strtok_r(NULL, " ", &arg)) != NULL);
                }

                break;

#ifndef WIN32
            case PCAP_DIR:
                /* arg should be a directory name */
                ret = GetFilesUnderDir(arg, pcap_queue, filter);
                if (ret == -1)
                {
                    ErrorMessage("Error getting pcaps under dir: %s\n", arg);
                    return -1;
                }

                break;
#endif

            default:
                FatalError("Bad read multiple pcaps type\n");
                break;
        }
    }

    return 0;
}

int ValidateIPList(IpAddrSet *addrset, char *token)
{
    int check_flag = 0;
#ifdef SUP_IP6
    if(!addrset || !(addrset->head||addrset->neg_head))
    {
        check_flag = -1;
    } else {
        /* more conflict checking takes place inside the SFIP library */
        return 0;
    }
#else
    check_flag = CheckForIPListConflicts(addrset);
#endif
    
    switch( check_flag )
    {
        case -1:
            ParseError("Empty IP used either as source IP or as destination IP "
                       "in a rule. IP list: %s.", token);
            break;
            
        case 1: 
            ParseError("Negated IP ranges that are equal to or are more "
                       "general than non-negated ranges are not allowed.  "
                       "Consider inverting the logic: %s.", token);
            break;

        default:
            break;
    }

    return 0;
}

void ParserCleanup(void)
{
    port_list_free(&port_list);

    if (ruleIndexMap != NULL)
    {
        RuleIndexMapFree(&ruleIndexMap);
        ruleIndexMap = NULL;
    }
}

static void InitVarTables(SnortPolicy *p)
{
    if (p == NULL)
        return;

    if (p->var_table != NULL)
        DeleteVars(p->var_table);
    p->var_id = 1;

#ifdef SUP_IP6
    if (p->ip_vartable != NULL)
        sfvt_free_table(p->ip_vartable);
    p->ip_vartable = sfvt_alloc_table();
#endif

    if (p->portVarTable != NULL)
        PortVarTableFree(p->portVarTable);
    p->portVarTable = PortVarTableCreate();

    if (p->nonamePortVarTable != NULL)
        PortTableFree(p->nonamePortVarTable);
    p->nonamePortVarTable = PortTableNew();

    if ((p->portVarTable == NULL) || (p->nonamePortVarTable == NULL))
    {
        FatalError("%s(%d) Failed to create port variable tables.\n",
                   __FILE__, __LINE__);
    }
}

static void InitPolicyMode(SnortPolicy *p)
{
    if (!ScAdapterInlineMode() && !ScAdapterInlineTestMode())
    {
        p->policy_mode = POLICY_MODE__PASSIVE;
    }
    else if (ScAdapterInlineTestMode())
    {
        p->policy_mode =  POLICY_MODE__INLINE_TEST;
    }
    else
    {
        p->policy_mode = POLICY_MODE__INLINE;
    }
}

static void InitParser(void)
{
    rule_count = 0;
    detect_rule_count = 0;
    decode_rule_count = 0;
    preproc_rule_count = 0;
    head_count = 0;
    otn_count = 0;
    dynamic_rule_count = 0;

    memset(&tcpCnt, 0, sizeof(tcpCnt));
    memset(&udpCnt, 0, sizeof(udpCnt));
    memset(&ipCnt, 0, sizeof(ipCnt));
    memset(&icmpCnt, 0, sizeof(icmpCnt));

    port_list_free(&port_list);
    memset(&port_list, 0, sizeof(port_list));
    port_list.pl_max = MAX_RULE_COUNT;

    if (ruleIndexMap != NULL)
        RuleIndexMapFree(&ruleIndexMap);
    ruleIndexMap = RuleIndexMapCreate(MAX_RULE_COUNT);
    if (ruleIndexMap == NULL)
    {
        FatalError("%s(%d) Failed to create rule index map.\n",
                   __FILE__, __LINE__);
    }

    /* This is for determining if a config option has already been
     * configured.  Most can only be configured once */
    memset(config_opt_configured, 0, sizeof(config_opt_configured));
}

void ParseRules(SnortConfig *sc)
{
    tSfPolicyId policy_id;

    if ((sc == NULL) || (snort_conf_file == NULL))
        return;

    file_line = 0;
    file_name = snort_conf_file;

    /* Need to set this for plugin configurations since they're using
     * lists of callbacks */
    snort_conf_for_parsing = sc;

    LogMessage("\n");
    LogMessage("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    LogMessage("Initializing rule chains...\n");

    /* Global set for parsing a configuration file to tell it whether
     * we're parsing rules or not - see ParseConfigFile */
    parse_rules = 1;

    /* Set to default policy */
    policy_id = sfGetDefaultPolicy(sc->policy_config);
    setParserPolicy(policy_id);

    ParseConfigFile(sc, sc->targeted_policies[policy_id], snort_conf_file);

    /* Parse rules in targeted policies */
    for (policy_id = 0;
         policy_id < sfPolicyNumAllocated(sc->policy_config);
         policy_id++)
    {
        char *fname = sfPolicyGet(sc->policy_config, policy_id);

        if (policy_id == sfGetDefaultPolicy(sc->policy_config))
            continue;

        if (fname != NULL)
        {
            setParserPolicy(policy_id);
            ParseInclude(sc, sc->targeted_policies[policy_id], fname);
        }
    }

    LogMessage("%d Snort rules read\n", rule_count);
    LogMessage("    %d detection rules\n", detect_rule_count);
    LogMessage("    %d decoder rules\n", decode_rule_count);
    LogMessage("    %d preprocessor rules\n", preproc_rule_count);
    LogMessage("%d Option Chains linked into %d Chain Headers\n", otn_count, head_count);
    LogMessage("%d Dynamic rules\n", dynamic_rule_count);
    LogMessage("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    LogMessage("\n");


    /* plug all the dynamic rules together */
    if (dynamic_rule_count != 0)
        LinkDynamicRules(sc);

#ifdef DEBUG
    DumpRuleChains(sc->rule_lists);
#endif

    IntegrityCheckRules(sc);
    /*FindMaxSegSize();*/

    /* Compile/Finish and Print the PortList Tables */
    PortTablesFinish(sc->port_tables, sc->fast_pattern_config);

    LogMessage("+-------------------[Rule Port Counts]---------------------------------------\n");
    LogMessage("|%8s%8s%8s%8s%8s\n", " ", "tcp", "udp", "icmp", "ip");
    LogMessage("|%8s%8u%8u%8u%8u\n", "src", tcpCnt.src, udpCnt.src, icmpCnt.src, ipCnt.src);
    LogMessage("|%8s%8u%8u%8u%8u\n", "dst", tcpCnt.dst, udpCnt.dst, icmpCnt.dst, ipCnt.dst);
    LogMessage("|%8s%8u%8u%8u%8u\n", "any", tcpCnt.aa, udpCnt.aa, icmpCnt.aa, ipCnt.aa);
    LogMessage("|%8s%8u%8u%8u%8u\n", "nc", tcpCnt.nc, udpCnt.nc, icmpCnt.nc, ipCnt.nc);
    LogMessage("|%8s%8u%8u%8u%8u\n", "s+d", tcpCnt.sd, udpCnt.sd, icmpCnt.sd, ipCnt.sd);
    LogMessage("+----------------------------------------------------------------------------\n");

    ///print_rule_index_map( ruleIndexMap );
    ///port_list_print( &port_list );

    /* Make sure this gets set back to NULL when we're done parsing */
    snort_conf_for_parsing = NULL;

    /* Reset these.  The only issue in not reseting would be if we were
     * parsing a command line again, but do it anyway */
    file_name = NULL;
    file_line = 0;
}

static void ParseInclude(SnortConfig *sc, SnortPolicy *p, char *arg)
{
    struct stat file_stat;  /* for include path testing */
    /* Save place in previous file */
    char *stored_file_name = file_name;
    int stored_file_line = file_line;

    /* Including top level snort conf file */
    if (strcmp(arg, snort_conf_file) == 0)
    {
        ParseError("Cannot include \"%s\" in an include directive.",
                   snort_conf_file);
    }
    
    /* XXX Maybe not allow an include in an included file to avoid
     * potential recursion issues */

    file_line = 0;
    file_name = SnortStrdup(arg);

    /* Stat the file.  If that fails, stat it relative to the directory
     * that the top level snort configuration file was in */ 
    if (stat(file_name, &file_stat) == -1)
    {
        int path_len = strlen(snort_conf_dir) + strlen(arg) + 1;

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"ParseConfigFile: stat "
                                "on %s failed - going to config_dir\n", file_name););

        free(file_name);

        file_name = (char *)SnortAlloc(path_len);
        snprintf(file_name, path_len, "%s%s", snort_conf_dir, arg);

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"ParseConfigFile: Opening "
                                "and parsing %s\n", file_name););
    }

    ParseConfigFile(sc, p, file_name);

    free(file_name);

    file_name = stored_file_name;
    file_line = stored_file_line;
}

static void ParseConfigFile(SnortConfig *sc, SnortPolicy *p, char *fname)
{
    /* Used for line continuation */
    int continuation = 0;
    char *saved_line = NULL;
    char *new_line = NULL;
    char *buf = (char *)SnortAlloc(MAX_LINE_LENGTH + 1);
    FILE *fp = fopen(fname, "r");

    /* open the rules file */
    if (fp == NULL)
    {
        FatalError("Unable to open rules file \"%s\": %s.\n",
                   fname, strerror(errno));
    }

    /* loop thru each file line and send it to the rule parser */
    while ((fgets(buf, MAX_LINE_LENGTH, fp)) != NULL)
    {
        /* buffer indexing pointer */
        char *index = buf;

        /* Increment the line counter so the error messages know which
         * line to bitch about */
        file_line++;

        /* fgets always appends a null, so doing a strlen should be safe */
        if ((strlen(buf) + 1) == MAX_LINE_LENGTH)
        {
            ParseError("Line greater than or equal to %u characters which is "
                       "more than the parser is willing to handle.  Try "
                       "splitting it up on multiple lines if possible.",
                       MAX_LINE_LENGTH);
        }

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Got line %s (%d): %s\n",
                                fname, file_line, buf););

        /* advance through any whitespace at the beginning of the line */
        while (isspace((int)*index))
            index++;

        /* If it's an empty line or starts with a comment character */
        if ((strlen(index) == 0) || (*index == '#') || (*index == ';'))
            continue;
          
        if (continuation)
        {
            int new_line_len = strlen(saved_line) + strlen(index) + 1;

            if (new_line_len >= PARSERULE_SIZE)
            {
                ParseError("Rule greater than or equal to %u characters which "
                           "is more than the parser is willing to handle.  "
                           "Submit a bug to bugs@snort.org if you legitimately "
                           "feel like your rule or keyword configuration needs "
                           "more than this amount of space.", PARSERULE_SIZE);
            }

            new_line = (char *)SnortAlloc(new_line_len);
            snprintf(new_line, new_line_len, "%s%s", saved_line, index);

            free(saved_line);
            saved_line = NULL;
            index = new_line;

            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"concat rule: %s\n", 
                                    new_line););
        }

        /* check for a '\' continuation character at the end of the line
         * if it's there we need to get the next line in the file */
        if (ContinuationCheck(index) == 0) 
        {
            char **toks;
            int num_toks;
            char *keyword;
            char *args;
            int i;

            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                                    "[*] Processing keyword: %s\n", index););

            /* Get the keyword and args */
            toks = mSplit(index, " \t", 2, &num_toks, 0);
            if (num_toks != 2)
                ParseError("Invalid configuration line: %s", index);

            keyword = SnortStrdup(ExpandVars(sc, toks[0]));
            args = toks[1];

            for (i = 0; snort_conf_keywords[i].name != NULL; i++)
            {
                if (strcasecmp(keyword, snort_conf_keywords[i].name) == 0)
                {
                    if ((getParserPolicy() != getDefaultPolicy()) &&
                        snort_conf_keywords[i].default_policy_only)
                    {
                        /* Keyword only configurable in the default policy*/
                        DEBUG_WRAP(DebugMessage(DEBUG_INIT, 
                            "Config option \"%s\" configurable only by default policy. Ignoring it", toks[0]));
                        break;
                    }

                    if (((snort_conf_keywords[i].type == KEYWORD_TYPE__RULE) &&
                         !parse_rules) ||
                        ((snort_conf_keywords[i].type == KEYWORD_TYPE__MAIN) &&
                         parse_rules))
                    {
                        break;
                    }

                    if (snort_conf_keywords[i].expand_vars)
                        args = SnortStrdup(ExpandVars(sc, toks[1]));

                    /* Special parsing case is ruletype.
                     * Need to send the file pointer so it can parse what's
                     * between '{' and '}' which can span multiple lines
                     * without a line continuation character */
                    if (strcasecmp(keyword, SNORT_CONF_KEYWORD__RULE_TYPE) == 0)
                        _ParseRuleTypeDeclaration(sc, fp, args, parse_rules);
                    else
                        snort_conf_keywords[i].parse_func(sc, p, args);

                    break;
                }
            }

            /* Didn't find any pre-defined snort_conf_keywords.  Look for a user defined
             * rule type */

            if ((snort_conf_keywords[i].name == NULL) && parse_rules)
            {
                RuleListNode *node;

                DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Unknown rule type, "
                                        "might be declared\n"););

                for (node = sc->rule_lists; node != NULL; node = node->next)
                {
                    if (strcasecmp(node->name, keyword) == 0)
                        break;
                }

                if (node == NULL)
                    ParseError("Unknown rule type: %s.", toks[0]);

                if ( node->mode == RULE_TYPE__DROP )
                {
                    if ( ScTreatDropAsAlert() )
                        ParseRule(sc, p, args, RULE_TYPE__ALERT, node->RuleList);

                    else if ( ScKeepDropRules() ||  ScLoadAsDropRules() )
                        ParseRule(sc, p, args, node->mode, node->RuleList);
                }
                else if ( node->mode == RULE_TYPE__SDROP )
                {
                    if ( ScKeepDropRules() && !ScTreatDropAsAlert() )
                        ParseRule(sc, p, args, node->mode, node->RuleList);

                    else if ( ScLoadAsDropRules() )
                        ParseRule(sc, p, args, RULE_TYPE__DROP, node->RuleList);
                }
                else
                {
                    ParseRule(sc, p, args, node->mode, node->RuleList);
                }
            }

            if (args != toks[1])
                free(args);

            free(keyword);
            mSplitFree(&toks, num_toks);

            if(new_line != NULL)
            {
                free(new_line);
                new_line = NULL;
                continuation = 0;
            }
        }
        else
        {
            /* save the current line */
            saved_line = SnortStrdup(index);

            /* current line was a continuation itself... */
            if (new_line != NULL)
            {
                free(new_line);
                new_line = NULL;
            }

            /* set the flag to let us know the next line is 
             * a continuation line */ 
            continuation = 1;
        }   
    }

    fclose(fp);
    free(buf);
}

static int ContinuationCheck(char *rule)
{
    char *idx;  /* indexing var for moving around on the string */

    idx = rule + strlen(rule) - 1;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"initial idx set to \'%c\'\n", 
                *idx););

    while(isspace((int)*idx))
    {
        idx--;
    }

    if(*idx == '\\')
    {
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Got continuation char, "
                    "clearing char and returning 1\n"););

        /* clear the '\' so there isn't a problem on the appended string */
        *idx = '\x0';
        return 1;
    }

    return 0;
}





void ConfigAlertBeforePass(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__ALERT_BEFORE_PASS;
}

void ConfigAlertFile(SnortConfig *sc, char *args)
{
    if ((args == NULL) || (sc == NULL) || (sc->alert_file != NULL))
        return;

    sc->alert_file = SnortStrdup(args);

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"alertfile set to: %s\n", 
                            sc->alert_file););
}

void ConfigAlertWithInterfaceName(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->output_flags |= OUTPUT_FLAG__ALERT_IFACE;
}

void ConfigAsn1(SnortConfig *sc, char *args)
{
    long int num_nodes;
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    num_nodes = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') || (num_nodes <= 0))
    {
        ParseError("Invalid argument to 'asn1' configuration.  "
                   "Must be a positive integer.");
    }

    sc->asn1_mem = num_nodes;
}

#ifdef PREPROCESSOR_AND_DECODER_RULE_EVENTS
void ConfigAutogenPreprocDecoderRules(SnortConfig *sc, char *args)
{
    SnortPolicy* policy;

    if (sc == NULL)
        return;

    /* config autogenerate_preprocessor_decoder_rules */
    UpdateDecodeRulesArray( 0, ENABLE_RULE, ENABLE_ALL_RULES);
    policy = sc->targeted_policies[getParserPolicy()];
    policy->policy_flags |= POLICY_FLAG__AUTO_OTN;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Autogenerating Preprocessor and Decoder OTNs\n"););
}
#endif

void ConfigBinding(SnortConfig *sc, char *args)
{
    int num_toks;
    int num_toks1;
    char **toks;
    char **toks1;
    char *fileName;
    int bindingType;
    toks1 = mSplit(args, " \t", 3, &num_toks1, 0);
    if(num_toks1 < 3)
    {
        mSplitFree(&toks1, num_toks1);
        ParseError("Need at least two arguments to 'config binding'");
        return;
    }
    if (!strcmp("vlan", toks1[1]))
    {
        bindingType = SF_BINDING_TYPE_VLAN;
    }
    else if (!strcmp("net", toks1[1]))
    {
        bindingType = SF_BINDING_TYPE_NETWORK;
    }
    else
    {
        mSplitFree(&toks1, num_toks1);
        ParseError("Invalid binding type in 'config binding'");
        return;
    }
    fileName = toks1[0];
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Policy File: %s\n", fileName););

#define MAX_BOUND_ADDRS_PER_LINE 512
    toks = mSplit(toks1[2], ",", MAX_BOUND_ADDRS_PER_LINE + 1, &num_toks, 0);

    if (num_toks < 1)
    {
        mSplitFree(&toks1, num_toks1);
        mSplitFree(&toks, num_toks);
        ParseError(" Invalid arguments to 'config binding'");
        return;
    }

    if (num_toks >= 512)
    {
        mSplitFree(&toks1, num_toks1);
        mSplitFree(&toks, num_toks);
        ParseError(" Too many network addresses specified in 'config binding'. "
                   " Maximum is %d.\n", MAX_BOUND_ADDRS_PER_LINE);
        return;
    }

    if (bindingType == SF_BINDING_TYPE_VLAN)
    {
        if (ParseVlanBindingLine(sc->policy_config, num_toks, &toks[0], fileName))
        {
            FatalError("formating error in binding file: %s\n", fileName);
        }
    }
    else 
    {
        if (ParseNetworkBindingLine(sc->policy_config, num_toks, &toks[0], fileName))
        {
            FatalError("formating error in binding file: %s\n", fileName);
        }
    }
    mSplitFree(&toks1, num_toks1);
    mSplitFree(&toks, num_toks);
}

void ConfigBpfFile(SnortConfig *sc, char *args)
{
    if ((args == NULL) || (sc == NULL) || (sc->bpf_file != NULL))
        return;

    sc->bpf_file = SnortStrdup(args);
}

void ConfigChecksumDrop(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    if (sc->targeted_policies == NULL)
    {
        //This is the case for command line argument
        sc->checksum_drop_flags = GetChecksumFlags(args);
        sc->checksum_drop_flags_modified = 1;
    }
    else
    {
        SnortPolicy *pPolicy = sc->targeted_policies[getParserPolicy()];

        pPolicy->checksum_drop_flags = GetChecksumFlags(args);
        pPolicy->checksum_drop_flags_modified = 1;
    }
}

void ConfigChecksumMode(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    if (sc->targeted_policies == NULL)
    {
        //This is the case for command line argument
        sc->checksum_flags = GetChecksumFlags(args);
        sc->checksum_flags_modified = 1;
    }
    else
    {
        SnortPolicy *pPolicy = sc->targeted_policies[getParserPolicy()];

        pPolicy->checksum_flags = GetChecksumFlags(args);
        pPolicy->checksum_flags_modified = 1;
    }

}

static int GetChecksumFlags(char *args)
{
    char **toks;
    int num_toks;
    int i;
    int negative_flags = 0;
    int positive_flags = 0;
    int got_positive_flag = 0;
    int got_negative_flag = 0;
    int ret_flags = 0;

    if (args == NULL)
        return CHECKSUM_FLAG__ALL;

    toks = mSplit(args, " \t", 10, &num_toks, 0);
    for (i = 0; i < num_toks; i++)
    {
        if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__ALL) == 0)
        {
            positive_flags = CHECKSUM_FLAG__ALL;
            negative_flags = 0;
            got_positive_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__NONE) == 0)
        {
            positive_flags = 0;
            negative_flags = CHECKSUM_FLAG__ALL;
            got_negative_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__IP) == 0) 
        {
            positive_flags |= CHECKSUM_FLAG__IP;
            negative_flags &= ~CHECKSUM_FLAG__IP;
            got_positive_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__NO_IP) == 0) 
        {
            positive_flags &= ~CHECKSUM_FLAG__IP;
            negative_flags |= CHECKSUM_FLAG__IP;
            got_negative_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__TCP) == 0)
        {
            positive_flags |= CHECKSUM_FLAG__TCP;
            negative_flags &= ~CHECKSUM_FLAG__TCP;
            got_positive_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__NO_TCP) == 0)
        {
            positive_flags &= ~CHECKSUM_FLAG__TCP;
            negative_flags |= CHECKSUM_FLAG__TCP;
            got_negative_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__UDP) == 0)
        {
            positive_flags |= CHECKSUM_FLAG__UDP;
            negative_flags &= ~CHECKSUM_FLAG__UDP;
            got_positive_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__NO_UDP) == 0)
        {
            positive_flags &= ~CHECKSUM_FLAG__UDP;
            negative_flags |= CHECKSUM_FLAG__UDP;
            got_negative_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__ICMP) == 0)
        {
            positive_flags |= CHECKSUM_FLAG__ICMP;
            negative_flags &= ~CHECKSUM_FLAG__ICMP;
            got_positive_flag = 1;
        }
        else if (strcasecmp(toks[i], CHECKSUM_MODE_OPT__NO_ICMP) == 0)
        {
            positive_flags &= ~CHECKSUM_FLAG__ICMP;
            negative_flags |= CHECKSUM_FLAG__ICMP;
            got_negative_flag = 1;
        }
        else
        {
            ParseError("Unknown command line checksum option: %s.", toks[i]);
        }
    }

    /* Invert the negative flags with all checksums */
    negative_flags ^= CHECKSUM_FLAG__ALL;
    negative_flags &= CHECKSUM_FLAG__ALL;

    if (got_positive_flag && got_negative_flag)
    {
        /* If we got both positive and negative flags just take the
         * combination of the two */
        ret_flags = positive_flags & negative_flags;
    }
    else if (got_positive_flag)
    {
        /* If we got a positive flag assume the user wants checksums
         * to be cleared */
        ret_flags = positive_flags;
    }
    else  /* got a negative flag */
    {
        /* If we got a negative flag assume the user thinks all
         * checksums are on */
        ret_flags = negative_flags;
    }

    mSplitFree(&toks, num_toks);
    return ret_flags;
}

void ConfigChrootDir(SnortConfig *sc, char *args)
{
#ifdef WIN32
    ParseError("Setting the chroot directory is not supported in "
               "the WIN32 port of snort!");
#else
    if ((args == NULL) || (sc == NULL) || (sc->chroot_dir != NULL))
        return;

    sc->chroot_dir = SnortStrdup(args);
#endif
}

void ConfigClassification(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    char *endptr;
    ClassType *new_node, *current;
    int max_id = 0;

    if ((args == NULL) || (sc == NULL))
        return;

    toks = mSplit(args, ",", 0, &num_toks, '\\');
    if (num_toks != 3)
        ParseError("Invalid classification config: %s.", args);

    /* create the new node */
    new_node = (ClassType *)SnortAlloc(sizeof(ClassType));

    new_node->type = SnortStrdup(toks[0]);
    new_node->name = SnortStrdup(toks[1]);

    new_node->priority = SnortStrtol(toks[2], &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') || (new_node->priority <= 0))
    {
        ParseError("Invalid argument for classification priority "
                   "configuration: %s.  Must be a positive integer.", toks[2]);
    }

    current = sc->classifications;
    while (current != NULL)
    {
        /* dup check */
        if (strcasecmp(current->type, new_node->type) == 0)
        {
            if (getParserPolicy() == getDefaultPolicy())
            {
                ParseWarning("Duplicate classification \"%s\""
                        "found, ignoring this line\n", file_name, file_line, 
                        new_node->type);
            }

            break;
        }

        if (current->id > max_id)
            max_id = current->id;

        current = current->next;
    }

    /* Got a dup */
    if (current != NULL)
    {
        free(new_node->name);
        free(new_node->type);
        free(new_node);
        mSplitFree(&toks, num_toks);
        return;
    }

    /* insert node */
    new_node->id = max_id + 1;
    new_node->next = sc->classifications;
    sc->classifications = new_node;

    mSplitFree(&toks, num_toks);
}

void ConfigCreatePidFile(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__CREATE_PID_FILE;
}

void ConfigDaemon(SnortConfig *sc, char *args)
{
#ifdef WIN32
    ParseError("Setting the Daemon mode is not supported in the "
               "WIN32 port of snort!  Use 'snort /SERVICE ...' instead.");
#else
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Daemon mode flag set\n"););
    sc->run_flags |= RUN_FLAG__DAEMON;
    sc->logging_flags |= LOGGING_FLAG__QUIET;
#endif
}

void ConfigDecodeDataLink(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Decode DLL set\n"););
    sc->output_flags |= OUTPUT_FLAG__SHOW_DATA_LINK;
}

void ConfigDefaultRuleState(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    LogMessage("Found rule_state config directive\n");

    if (args == NULL)
    {
        sc->default_rule_state = RULE_STATE_ENABLED;
    }
    else if (strcasecmp(args, RULE_STATE_OPT__DISABLED) == 0)
    {
        sc->default_rule_state = RULE_STATE_DISABLED;
    }
    else
    {
        /* Any other word and just call it enabled */
        sc->default_rule_state = RULE_STATE_ENABLED;
    }
}

void ConfigDetection(SnortConfig *sc, char *args)
{
    int i;
    char **toks;
    int num_toks;
    FastPatternConfig *fp;
    int old_stream_inserts = -1;
    int old_max_queue_events = -1;

    if ((sc == NULL) || (args == NULL))
        return;

    fp = sc->fast_pattern_config;

    if (fp->configured)
    {
        ParseWarning("Reconfiguring detection options.");

        /* Save max queue events and stream inserts in case they are
         * not configured again - these will carry over into the new
         * configuration */
        old_max_queue_events = fp->max_queue_events;
        old_stream_inserts = fp->inspect_stream_insert;

        fpSetDefaults(fp);
    }

    toks = mSplit(args, ", ", 20, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        if (strcasecmp(toks[i], DETECTION_OPT__SEARCH_OPTIMIZE) == 0)
        {
            fpSetDetectSearchOpt(fp, 1);
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__ENABLE_SINGLE_RULE_GROUP) == 0)
        {
            fpDetectSetSingleRuleGroup(fp);
            LogMessage("Using Single-Rule-Group Detection\n");
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__DEBUG_PRINT_NOCONTENT_RULE_TESTS) == 0)
        {
            fpDetectSetDebugPrintNcRules(fp);
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__DEBUG_PRINT_RULE_GROUP_BUILD_DETAILS) == 0)
        {
            fpDetectSetDebugPrintRuleGroupBuildDetails(fp);
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__DEBUG_PRINT_RULE_GROUPS_UNCOMPILED) == 0)
        {
            fpDetectSetDebugPrintRuleGroupsUnCompiled(fp);
        } 
        else if (strcasecmp(toks[i], DETECTION_OPT__DEBUG_PRINT_RULE_GROUPS_COMPILED) == 0)
        {
            fpDetectSetDebugPrintRuleGroupsCompiled(fp);
        } 
        else if (strcasecmp(toks[i], DETECTION_OPT__DEBUG) == 0)
        {
            fpSetDebugMode(fp);
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__NO_STREAM_INSERTS) == 0)
        {
            fpSetStreamInsert(fp);
            old_stream_inserts = -1;  /* Don't reset to old value */
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__BLEEDOVER_WARNINGS_ENABLED) == 0)
        {
            fpDetectSetBleedOverWarnings(fp);
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__SEARCH_METHOD) == 0)
        {
            i++;
            if (i < num_toks) 
            {
                if (fpSetDetectSearchMethod(fp, toks[i]) == -1)
                {
                    ParseError("Invalid argument to 'search-method': %s.", toks[i]);
                }
            }
            else
            {
                ParseError("Need an argument to 'search-method'.");
            }
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__BLEEDOVER_PORT_LIMIT) == 0)
        {
            i++;
            if (i < num_toks)
            {
                char *endptr;
                int n = SnortStrtol(toks[i], &endptr, 0);

                if ((errno == ERANGE) || (*endptr != '\0') || (n <= 0))
                {
                    ParseError("Invalid argument for bleedover limit: %s.  "
                               "Need a non-negative integer.", toks[i]);
                }

                fpDetectSetBleedOverPortLimit(fp, n);
                LogMessage("Bleedover Port Limit : %d\n",n);
            }
            else
            {
                ParseError("Missing port-count argument to 'bleedover_port_limit'.");
            }
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__MAX_QUEUE_EVENTS) == 0)
        {
            i++;
            if (i < num_toks)
            {
                char *endptr;
                int n = SnortStrtol(toks[i], &endptr, 0);

                if ((errno == ERANGE) || (*endptr != '\0') || (n <= 0))
                {
                    ParseError("Invalid argument for max_queue_events: %s.  "
                               "Need a non-negative integer.", toks[i]);
                }

                fpSetMaxQueueEvents(fp, n);
                old_max_queue_events = -1;  /* Don't reset to old value */
            }
            else
            {
                ParseError("Missing argument to 'max_queue_events'.");
            }
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__SPLIT_ANY_ANY) == 0)
        {
            fpDetectSetSplitAnyAny(fp, 1);
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__MAX_PATTERN_LEN) == 0)
        {
            i++;
            if (i < num_toks)
            {
                char *endptr;
                int n = SnortStrtol(toks[i], &endptr, 0);

                if ((errno == ERANGE) || (*endptr != '\0') || (n < 0))
                {
                    ParseError("Invalid argument for max-pattern-len: %s.  "
                               "Need a non-negative integer.", toks[i]);
                }

                fpSetMaxPatternLen(fp, n);
            }
            else
            {
                ParseError("Missing argument to 'max-pattern-len'.");
            }
        }
        else if (strcasecmp(toks[i], DETECTION_OPT__DEBUG_PRINT_FAST_PATTERN) == 0)
        {
            fpDetectSetDebugPrintFastPatterns(fp, 1);
        }
        else
        {
            ParseError("'%s' is an invalid option to the 'config detection' "
                       "configuration.", toks[i]);
        }
    }

    mSplitFree(&toks, num_toks);

    if (old_max_queue_events != -1)
        fp->max_queue_events = old_max_queue_events;
    if (old_stream_inserts != -1)
        fp->inspect_stream_insert = old_stream_inserts;

    fp->configured = 1;
}

void ConfigDetectionFilter(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;

    if ((sc == NULL) || (args == NULL))
        return;

    if (!sc->detection_filter_config->enabled)
        return;

    toks = mSplit(args, " \t", 2, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseError("Detection filter memcap requires a positive "
                   "integer argument.");
    }

    if (strcasecmp(toks[0], THRESHOLD_OPT__MEMCAP) == 0)
    {
        char *endptr;

        sc->detection_filter_config->memcap = SnortStrtol(toks[1], &endptr, 0);
        if ((errno == ERANGE) || (*endptr != '\0') ||
            (sc->detection_filter_config->memcap < 0))
        {
            ParseError("Invalid detection filter memcap: %s.  Must be a "
                       "positive integer.", toks[1]);
        }
    }
    else
    {
        ParseError("Unknown argument to threshold configuration: %s.", toks[0]);
    }

    mSplitFree(&toks, num_toks);

}

void ConfigDisableDecodeAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the decoder alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__DEFAULT;
}

void ConfigDisableDecodeDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* OBSOLETE -- default is disabled */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of decoder alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__DEFAULT;
}

#ifdef INLINE_FAILOPEN
void ConfigDisableInlineFailopen(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Inline Init Failopen disabled\n"););
    sc->run_flags |= RUN_FLAG__DISABLE_FAILOPEN;
}
#endif

void ConfigDisableIpOptAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the alert of all the ipopt alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

void ConfigDisableIpOptDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* OBSOLETE -- default is disabled */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of all the ipopt alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

void ConfigDisableTcpOptAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the all the other tcpopt alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

void ConfigDisableTcpOptDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* OBSOLETE -- default is disabled */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of all other tcpopt alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

void ConfigDisableTcpOptExperimentalAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the tcpopt experimental alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

void ConfigDisableTcpOptExperimentalDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* OBSOLETE -- default is disabled */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of tcpopt exprimental alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

void ConfigDisableTcpOptObsoleteAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the tcpopt obsolete alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

void ConfigDisableTcpOptObsoleteDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* OBSOLETE -- default is disabled */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of tcpopt obsolete alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

void ConfigDisableTTcpAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the ttcp alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

void ConfigDisableTTcpDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* OBSOLETE -- default is disabled */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of ttcp alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

void ConfigDumpCharsOnly(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* dump the application layer as text only */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Character payload dump set\n"););
    sc->output_flags |= OUTPUT_FLAG__CHAR_DATA;
}

void ConfigDumpPayload(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* dump the application layer */
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Payload dump set\n"););
    sc->output_flags |= OUTPUT_FLAG__APP_DATA;
}

void ConfigDumpPayloadVerbose(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Verbose packet bytecode dumps enabled\n"););
    sc->output_flags |= OUTPUT_FLAG__VERBOSE_DUMP;
}

void ConfigEnableDecodeDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Enabling the drop of decoder alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags |= DECODE_EVENT_FLAG__DEFAULT;
}

void ConfigEnableDecodeOversizedAlerts(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Enabling the decoder oversized packet alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_alert_flags |= DECODE_EVENT_FLAG__OVERSIZED;
}

void ConfigEnableDecodeOversizedDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Enabling the drop of decoder oversized packets\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags |= DECODE_EVENT_FLAG__OVERSIZED;
}

void ConfigEnableDeepTeredoInspection(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Enabling deep Teredo inspection\n"););
    sc->enable_teredo = 1; /* TODO: add this to some existing flag bitfield? */
}

void ConfigEnableIpOptDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of all the ipopt alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags |= DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
}

#ifdef MPLS
void ConfigEnableMplsMulticast(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__MPLS_MULTICAST;
}

void ConfigEnableMplsOverlappingIp(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__MPLS_OVERLAPPING_IP;
}
#endif

void ConfigEnableTcpOptDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of all other tcpopt alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags |= DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
}

void ConfigEnableTcpOptExperimentalDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "enabling the drop of tcpopt exprimental alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags |= DECODE_EVENT_FLAG__TCP_EXP_OPT;
}

void ConfigEnableTcpOptObsoleteDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of tcpopt obsolete alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags |= DECODE_EVENT_FLAG__TCP_OBS_OPT;
}

void ConfigEnableTTcpDrops(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "disabling the drop of ttcp alerts\n"););
    sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__TCP_TTCP_OPT;
}

void ConfigEventFilter(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;

    if ((sc == NULL) || (args == NULL))
        return;

    if (!sc->threshold_config->enabled)
        return;

    toks = mSplit(args, " \t", 2, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseError("Threshold memcap requires a positive integer argument.");
    }

    if (strcasecmp(toks[0], THRESHOLD_OPT__MEMCAP) == 0)
    {
        char *endptr;

        sc->threshold_config->memcap = SnortStrtol(toks[1], &endptr, 0);
        if ((errno == ERANGE) || (*endptr != '\0') ||
            (sc->threshold_config->memcap < 0))
        {
            ParseError("Invalid threshold memcap: %s.  Must be a "
                       "positive integer.", toks[1]);
        }
    }
    else
    {
        ParseError("Unknown argument to threshold configuration: %s.", toks[0]);
    }

    mSplitFree(&toks, num_toks);
}

void ConfigEventQueue(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    int i;
    EventQueueConfig *eq;

    if ((sc == NULL) || (args == NULL))
        return;

    eq = sc->event_queue_config;

    toks = mSplit(args, ", ", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        if (strcasecmp(toks[i], EVENT_QUEUE_OPT__MAX_QUEUE) == 0)
        {
            i++;
            if (i < num_toks)
            {
                char *endptr;

                eq->max_events = SnortStrtol(toks[i], &endptr, 0);
                if ((errno == ERANGE) || (*endptr != '\0') ||
                    (eq->max_events <= 0))
                {
                    ParseError("Invalid argument for max_queue: %s.  Must "
                               "be a positive integer.", toks[i]);
                }
            }
            else
            {
                ParseError("No argument to 'max_queue'.  Argument must "
                           "be a positive integer.");
            }
        }
        else if (strcasecmp(toks[i], EVENT_QUEUE_OPT__LOG) == 0)
        {
            i++;
            if (i < num_toks)
            {
                char *endptr;

                eq->log_events = SnortStrtol(toks[i], &endptr, 0);
                if ((errno == ERANGE) || (*endptr != '\0') ||
                    (eq->log_events <= 0))
                {
                    ParseError("Invalid argument for 'log': %s.  Must be "
                               "a positive integer.", toks[i]);
                }
            }
            else
            {
                ParseError("No argument to 'log'.  Argument must be a "
                           "positive integer.");
            }
        }
        else if (strcasecmp(toks[i], EVENT_QUEUE_OPT__ORDER_EVENTS) == 0)
        {
            i++;
            if(i < num_toks)
            {
                if (strcasecmp(toks[i], ORDER_EVENTS_OPT__PRIORITY) == 0)
                {
                    eq->order = SNORT_EVENTQ_PRIORITY;
                }
                else if (strcasecmp(toks[i], ORDER_EVENTS_OPT__CONTENT_LENGTH) == 0)
                {
                    eq->order = SNORT_EVENTQ_CONTENT_LEN;
                }
                else
                {
                    ParseError("Invalid argument to 'order_events': %s.  "
                               "Arguments may be either 'priority' or "
                               "content_length.", toks[i]);
                }
            }
            else
            {
                ParseError("No argument to 'order_events'.  Arguments may be "
                           "either 'priority' or content_length.");
            }
        }
        else if (strcasecmp(toks[i], EVENT_QUEUE_OPT__PROCESS_ALL_EVENTS) == 0)
        {
            eq->process_all_events = 1;
        }
        else
        {
            ParseError("Invalid argument to 'event_queue'.  To configure "
                       "event_queue, the options 'max_queue', 'log', and "
                       "'order_events' must be configured.");
        }
    }

    if (eq->max_events < eq->log_events )
        eq->max_events = eq->log_events;

    mSplitFree(&toks, num_toks);
}

void ConfigReact (SnortConfig *sc, char *args)
{
    if ((sc == NULL) || (args == NULL))
        return;

    sc->react_page = SnortStrdup(args);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,
        "react: page is %s\n", sc->react_page););
}

#ifdef ENABLE_RESPONSE3
void ConfigFlexresp2Interface(SnortConfig *sc, char *args)
{
    ParseWarning("flexresp2_interface is no longer supported.\n");
}

void ConfigFlexresp2Attempts(SnortConfig *sc, char *args)
{
    ParseWarning("flexresp2_attempts is no longer supported; "
        "you must use config response: attempts <#> instead.\n");
}

void ConfigFlexresp2Memcap(SnortConfig *sc, char *args)
{
    ParseWarning("flexresp2_memcap is no longer supported.\n");
}

void ConfigFlexresp2Rows(SnortConfig *sc, char *args)
{
    ParseWarning("flexresp2_rows is no longer supported.\n");
}
#endif

#ifdef ACTIVE_RESPONSE
// TBD:  once code can be checked in, move all config funcs
// from parser.[ch] to [parser-]config.[ch] *or* at least move
// Config* declarations from parser.h to parser.c or parser-config.h.
void ConfigResponse (SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    int i;

    if ((sc == NULL) || (args == NULL))
        return;

    toks = mSplit(args, ", ", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        if ( !strcasecmp(toks[i], RESPONSE_OPT__ATTEMPTS) )
        {
            if ( ++i < num_toks )
            {
                char *endptr;
                long int value = strtol(toks[i], &endptr, 0);

                if ((errno == ERANGE) || (*endptr != '\0') ||
                    (value < 1) || (value > 20))
                {
                    ParseError("Invalid argument for attempts: %s.  "
                        "Argument must be between 1 and 20 inclusive.", toks[i]);
                }
                sc->respond_attempts = (uint8_t)value;
            }
            else
            {
                ParseError("No argument to 'attempts'.  "
                    "Argument must be between 1 and 20 inclusive.");
            }
        }
        else if ( !strcasecmp(toks[i], RESPONSE_OPT__DEVICE) )
        {
            if ( ++i < num_toks )
            {
                sc->respond_device = SnortStrdup(toks[i]);
            }
            else
            {
                ParseError("No argument to 'device'.  Use 'ip' for network "
                    "layer responses or 'eth0' etc. for link layer responses.");
            }
        }
        else
        {
            ParseError("Invalid config response option '%s'", toks[i]);
        }
    }
    mSplitFree(&toks, num_toks);
}
#endif

void ConfigFlowbitsSize(SnortConfig *sc, char *args)
{
    char *endptr;
    long int size;

    if ((sc == NULL) || (args == NULL))
        return;

    size = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') ||
        (size < 0) || (size > 2096))
    {
        ParseError("Invalid argument to 'flowbits_size': %s.  Must be a "
                   "positive integer and less than 2096.", args);
    }

    giFlowbitSize = (uint8_t)(size >> 3);
    sc->flowbit_size = (uint8_t)(size >> 3);
}

/****************************************************************************
 *
 * Purpose: Parses a protocol plus a list of ports.
 *          The protocol should be "udp" or "tcp".
 *          The ports list should be a list of numbers or pairs of numbers.
 *          Each element of the list is separated by a space character.
 *          Each pair of numbers is separated by a colon character.
 *          So the string passed in is e.g. "tcp 443 578 6667:6681 13456"
 *          The numbers do not have to be in numerical order.
 *
 * Arguments:
 *  SnortConfig *
 *      The snort config to store ignored ports.
 *  char *
 *      string containing protocol plus list of ports
 *
 * Returns: void function
 *
 *****************************************************************************/
void ConfigIgnorePorts(SnortConfig *sc, char *args)
{
    char ** toks;
    int     num_toks = 0;
    int     i, p;
    uint16_t hi_port = 0, lo_port = 0;
    int     protocol;
    int     not_flag;

    if ((sc == NULL) || (args == NULL))
        return;

    LogMessage("Found ignore_ports config directive (%s)\n", args);

    toks = mSplit(args, " \t", 0, &num_toks, 0);

    if ( !num_toks )
        ParseError("config ignore_ports: Empty port list.");

    protocol = GetRuleProtocol(toks[0]);

    if ( !(protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) )
    {
        ParseError("Invalid protocol: %s.", toks[0]);
    }

    for ( i = 1; i < num_toks; i++ )
    {  
        /*  Re-use function from rules processing  */
        ParsePort(toks[i], &hi_port, &lo_port, toks[0], &not_flag);      
           
        for ( p = lo_port; p <= hi_port; p++ )
            sc->ignore_ports[p] = (uint8_t)protocol;  /* protocol will be 6 (TCP) or 17 (UDP) */
    }
    
    mSplitFree(&toks, num_toks);
}

void ConfigIncludeVlanInAlert(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->output_flags |= OUTPUT_FLAG__ALERT_VLAN;
}

void ConfigInterface(SnortConfig *sc, char *args)
{
    if ((args == NULL) || (sc == NULL) || (sc->interface != NULL))
        return;

#ifdef WIN32
    /* first, try to handle the "-i1" case, where an interface
     * is specified by number.  If this fails, then fall-through
     * to the case outside the ifdef/endif, where an interface
     * can be specified by its fully qualified name, like as is
     * shown by running 'snort -W', ie.
     * "\Device\Packet_{12345678-90AB-CDEF-1234567890AB}"
     */
    {
        char errorbuf[PCAP_ERRBUF_SIZE];
        int adaplen = atoi(args);

        if (adaplen > 0)
        {
            pcap_if_t *alldevs;
            pcap_if_t *dev;
            int i = 1;

            if (pcap_findalldevs(&alldevs, errorbuf) == -1)
                ParseError("Could not get device list: %s.", errorbuf);

            for (dev = alldevs; dev != NULL; dev = dev->next)
            {
                if (i == adaplen)
                    break;
                i++;
            }

            if (dev == NULL)
                ParseError("Invalid device number: %d.", adaplen);

            sc->interface = SnortStrdup(dev->name);
            pcap_freealldevs(alldevs);
            return;
        }
    }
#endif  /* WIN32 */

    /* this code handles the case in which the user specifies
     * the entire name of the interface and it is compiled
     * regardless of which OS you have */
    sc->interface = SnortStrdup(args);
}

void ConfigIpv6Frag(SnortConfig *sc, char *args)
{
    int num_opts;
    char **opt_toks;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"IPv6 Rule Option\n"););

    if ((sc == NULL) || (args == NULL))
        return;

    opt_toks = mSplit(args, ",", 0, &num_opts, 0);

    for (i = 0; i < num_opts; i++)
    {
        char **arg_toks;
        int num_args;

        arg_toks = mSplit(opt_toks[i], " \t", 2, &num_args, 0);

        if(!arg_toks[1]) 
        {
             ParseError("ipv6_frag option '%s' requires an argument.",
                        arg_toks[0]);
        }

        if(!strcasecmp(arg_toks[0], "bsd_icmp_frag_alert"))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, 
                      "disabling the BSD ICMP fragmentation alert\n"););
            if(!strcasecmp(arg_toks[1], "off"))
                sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG;
        }
        else if(!strcasecmp(arg_toks[0], "bad_ipv6_frag_alert"))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, 
                      "disabling the IPv6 bad fragmentation packet alerts\n"););
            if(!strcasecmp(arg_toks[1], "off"))
                sc->targeted_policies[getParserPolicy()]->decoder_alert_flags &= ~DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
        
        }
        else if (!strcasecmp(arg_toks[0], "frag_timeout"))
        {
            long val;
            char *endp;

            if(!args)
            {
                 ParseError("Setting the ipv6_frag_timeout requires an "
                            "integer argument.");
            }

            val = strtol(arg_toks[1], &endp, 0);
            if(val <= 0 || val > 3600)
            {
                ParseError("ipv6_frag_timeout: Invalid argument '%s'.  "
                           "Must be greater that 0 and less than 3600 "
                           "secnods.", arg_toks[1]);
            }

            if(args == endp || *endp)
            {
                ParseError("ipv6_frag_timeout: Invalid argument '%s'.", 
                           arg_toks[1]);
            }

            sc->ipv6_frag_timeout = val;
        }
        else if (!strcasecmp(arg_toks[0], "max_frag_sessions"))
        {
            long val;
            char *endp;

            if(!args)
            {
                 ParseError("Setting the ipv6_max_frag_sessions requires an "
                            "integer argument.");
            }

            val = strtol(arg_toks[1], &endp, 0);
            if (val <= 0) 
            {
                ParseError("ipv6_max_frag_sessions: Invalid number of sessions "
                           "'%s'. Must be greater than 0.", arg_toks[1]);
            }

            if(args == endp || *endp)
            {
                ParseError("ipv6_max_frag_sessions: Invalid number of "
                           "sessions '%s'.", arg_toks[1]);
            }

            sc->ipv6_max_frag_sessions = val;
        }
        else if (!strcasecmp(arg_toks[0], "drop_bad_ipv6_frag"))
        {
            if(!strcasecmp(arg_toks[1], "off"))
            {
                DEBUG_WRAP(DebugMessage(DEBUG_INIT, 
                      "disabling the BSD ICMP fragmentation alert\n"););
                sc->targeted_policies[getParserPolicy()]->decoder_drop_flags &= ~DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
            }
        }
        else 
        {
             ParseError("Invalid option to ipv6_frag '%s %s'.",
                        arg_toks[0], arg_toks[1]);
        }

        mSplitFree(&arg_toks, num_args);
    }

    mSplitFree(&opt_toks, num_opts);
}

void ConfigLayer2Resets(SnortConfig *sc, char *args)
{
    ParseWarning("layer2resets is deprecated.\n");
}

void ConfigLogDir(SnortConfig *sc, char *args)
{
    if ((args == NULL) || (sc == NULL) || (sc->log_dir != NULL))
        return;

    sc->log_dir = SnortStrdup(args);
}

void ConfigDaqType(SnortConfig *sc, char *args)
{
    if ( !args || !sc )
        return;

    if ( sc->daq_type )
        ParseError("Setting DAQ to %s but %s already selected.\n",
            args, sc->daq_type);

    // will be validated later after paths are established
    sc->daq_type = SnortStrdup(args);
}

void ConfigDaqMode(SnortConfig *sc, char *args)
{
    if ( !args || !sc || sc->daq_mode )
        return;

    // will be validated later when daq is instantiated
    sc->daq_mode = SnortStrdup(args);
}

void ConfigDaqVar(SnortConfig *sc, char *args)
{
    if ( !args || !sc )
        return;

    if ( !sc->daq_vars )
    {
        sc->daq_vars = StringVector_New();

        if ( !sc->daq_vars )
            ParseError("can't allocate memory for daq_var '%s'.", args);
    }
    if ( !StringVector_Add(sc->daq_vars, args) )
        ParseError("can't allocate memory for daq_var '%s'.", args);
}

void ConfigDaqDir(SnortConfig *sc, char *args)
{
    if ( !args || !sc )
        return;

    if ( !sc->daq_dirs )
    {
        sc->daq_dirs = StringVector_New();

        if ( !sc->daq_dirs )
            ParseError("can't allocate memory for daq_dir '%s'.", args);
    }
    if ( !StringVector_Add(sc->daq_dirs, args) )
        ParseError("can't allocate memory for daq_dir '%s'.", args);
}

void ConfigDirtyPig(SnortConfig *sc, char *args)
{
    if ( sc )
        sc->dirty_pig = 1;
}

#ifdef TARGET_BASED
void ConfigMaxAttributeHosts(SnortConfig *sc, char *args)
{
    uint32_t val = 0;
    char *endp;

    if ((sc == NULL) || (args == NULL))
        return;

    val = strtoul(args, &endp, 10);
    if (args == endp || *endp || (val == 0))
    {
        ParseError("max_attribute_hosts: Invalid number of hosts '%s'.  Must "
                   "be unsigned positive integer value.", args);
    }
    if ((val > MAX_MAX_ATTRIBUTE_HOSTS) || (val < MIN_MAX_ATTRIBUTE_HOSTS))
    {
        ParseError("max_atttribute_hosts: Invalid number of hosts %s'.  "
                   "Must be between %d and %d.", args,
                   MIN_MAX_ATTRIBUTE_HOSTS, MAX_MAX_ATTRIBUTE_HOSTS);
    }

    sc->max_attribute_hosts = val;
}

void ConfigMaxMetadataServices(SnortConfig *sc, char *args)
{
    uint32_t val = 0;
    char *endp;

    if ((sc == NULL) || (args == NULL))
        return;

    val = strtoul(args, &endp, 10);
    if (args == endp || *endp || (val == 0))
    {
        ParseError("max_metadata_services: Invalid number of hosts '%s'.  Must "
                   "be unsigned positive integer value.", args);
    }
    if ((val > MAX_MAX_METADATA_SERVICES) || (val < MIN_MAX_METADATA_SERVICES))
    {
        ParseError("max_metadata_services: Invalid number of hosts '%s'.  "
                   "Must be between %d and %d.", args,
                    MIN_MAX_METADATA_SERVICES, MAX_MAX_METADATA_SERVICES);
    }

    sc->max_metadata_services = val;
}
#endif

#ifdef MPLS
void ConfigMaxMplsLabelChain(SnortConfig *sc, char *args)
{
    char *endp;
    long val = 0;

    if (sc == NULL)
        return;

    if (args != NULL)
    {
        val = strtol(args, &endp, 0);
        if ((args == endp) || *endp || (val < -1))
            val = DEFAULT_LABELCHAIN_LENGTH;
    } 
    else 
    {
        val = DEFAULT_LABELCHAIN_LENGTH;
    }

    sc->mpls_stack_depth = val;
}

void ConfigMplsPayloadType(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    if (args != NULL)
    {
        if (strcasecmp(args, MPLS_PAYLOAD_OPT__IPV4) == 0)
        {
            sc->mpls_payload_type = MPLS_PAYLOADTYPE_IPV4;
        } 
        else if (strcasecmp(args, MPLS_PAYLOAD_OPT__IPV6) == 0)
        {
            sc->mpls_payload_type = MPLS_PAYLOADTYPE_IPV6;
        } 
        else if (strcasecmp(args, MPLS_PAYLOAD_OPT__ETHERNET) == 0)
        {
            sc->mpls_payload_type = MPLS_PAYLOADTYPE_ETHERNET;
        } 
        else 
        {
            ParseError("Non supported mpls payload type: %s.", args);
        }
    } 
    else 
    {
        sc->mpls_payload_type = DEFAULT_MPLS_PAYLOADTYPE;
    }
}
#endif

void ConfigMinTTL(SnortConfig *sc, char *args)
{
    long int value;
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    value = strtol(args, &endptr, 0);

    if ((errno == ERANGE) || (*endptr != '\0') ||
        (value < 1) || (value > UINT8_MAX))
    {
        ParseError("Invalid argument to 'min_ttl' configuration: %s.  "
                   "Must be a positive integer.", args);
    }

    {
        SnortPolicy* pPolicy = sc->targeted_policies[getParserPolicy()];
        pPolicy->min_ttl = (uint8_t)value;
    }
}

#ifdef NORMALIZER
void ConfigNewTTL(SnortConfig *sc, char *args)
{
    long int value;
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    value = strtol(args, &endptr, 0);

    if ((errno == ERANGE) || (*endptr != '\0') ||
        (value < 1) || (value > UINT8_MAX))
    {
        ParseError("Invalid argument to 'new_ttl' configuration: %s.  "
                   "Must be a non-negative integer.", args);
    }

    {
        SnortPolicy* pPolicy = sc->targeted_policies[getParserPolicy()];
        pPolicy->new_ttl = (uint8_t)value;
    }
}
#endif

void ConfigNoLog(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->no_log = 1;
}

void ConfigNoLoggingTimestamps(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->output_flags |= OUTPUT_FLAG__NO_TIMESTAMP;
}

void ConfigNoPcre(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__NO_PCRE;
}

void ConfigNoPromiscuous(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__NO_PROMISCUOUS;
}

void ConfigObfuscate(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->output_flags |= OUTPUT_FLAG__OBFUSCATE;
}

void ConfigObfuscationMask(SnortConfig *sc, char *args)
{
#ifndef SUP_IP6
    struct in_addr net;       /* place to stick the local network data */
    char **toks;              /* dbl ptr to store mSplit return data in */
    int num_toks;             /* number of tokens mSplit returns */
    int nmask;                /* temporary netmask storage */
# ifdef DEBUG
    struct in_addr sin;
# endif
#endif

    if ((sc == NULL) || (args == NULL))
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Got obfus data: %s\n", args););

    sc->output_flags |= OUTPUT_FLAG__OBFUSCATE;

#ifdef SUP_IP6
    sfip_pton(args, &sc->obfuscation_net);
#else
    /* break out the CIDR notation from the IP address */
    toks = mSplit(args, "/", 2, &num_toks, 0);

    if(num_toks > 1)
    {
        /* convert the CIDR notation into a real live netmask */
        nmask = atoi(toks[1]);

        if((nmask > 0) && (nmask < 33))
        {
            sc->obfuscation_mask = htonl(netmasks[nmask]);
        }
        else
        {
            ParseError("Bad CIDR block (%s) in obfuscation mask %s. "
                       "1 to 32 please!", toks[1], args);
        }
    }
    else
    {
        ParseError("No netmask specified for obsucation mask!");
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "obfuscation netmask = %#8lX\n", 
                            sc->obfuscation_mask););

    /* convert the IP addr into its 32-bit value */
    if((net.s_addr = inet_addr(toks[0])) == INADDR_NONE)
        ParseError("Obfuscation mask (%s) didn't translate.", toks[0]);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Obfuscation Net = %s (%X)\n", 
                            inet_ntoa(net), net.s_addr););

    /* set the final homenet address up */
    sc->obfuscation_net = net.s_addr & sc->obfuscation_mask;

#ifdef DEBUG
    sin.s_addr = sc->obfuscation_net;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Obfuscation Net = %s (%X)\n", 
                            inet_ntoa(sin), sin.s_addr););
#endif

    sc->obfuscation_mask = ~sc->obfuscation_mask;

    mSplitFree(&toks, num_toks);
#endif
}

void ConfigRuleListOrder(SnortConfig *sc, char *args)
{
    OrderRuleLists(sc, args);
}

void ConfigPcreMatchLimit(SnortConfig *sc, char *args)
{
    char *endp;
    long val = 0;

    if ((sc == NULL) || (args == NULL))
        return;

    val = strtol(args, &endp, 0);
    if ((args == endp) || *endp || (val < -1))
    {
        ParseError("pcre_match_limit: Invalid value '%s'.", args);
    }

    sc->pcre_match_limit = val;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "pcre_match_limit: %d\n",
                            sc->pcre_match_limit););
}

void ConfigPcreMatchLimitRecursion(SnortConfig *sc, char *args)
{
    char *endp;
    long val = 0;

    if ((sc == NULL) || (args == NULL))
        return;

    val = strtol(args, &endp, 0);
    if ((args == endp) || *endp || (val < -1))
    {
        ParseError("pcre_match_limit_recursion: Invalid value '%s'.", args);
    }

    sc->pcre_match_limit_recursion = val;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "pcre_match_limit_recursion: %d\n",
                            sc->pcre_match_limit_recursion););
}

void ConfigPerfFile(SnortConfig *sc, char *args)
{
    if ((sc == NULL) || (args == NULL))
        return;

    sc->perf_file = SnortStrdup(args);
}

void ConfigPacketCount(SnortConfig *sc, char *args)
{
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    sc->pkt_cnt = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid packet count: %s.  Packet count must be between "
                   "0 and %u inclusive.", args, UINT32_MAX);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Exiting after %d "
                            "packets\n", sc->pkt_cnt););

}

void ConfigPacketSnaplen(SnortConfig *sc, char *args)
{
    char *endptr;
    uint32_t snaplen;

    if ((sc == NULL) || (args == NULL))
        return;

    snaplen = SnortStrtoul(args, &endptr, 0);

    if ((errno == ERANGE) || (*endptr != '\0') ||
        ((snaplen != 0) && (snaplen < MIN_SNAPLEN)) ||
        (snaplen > MAX_SNAPLEN) )
    {
        ParseError("Invalid snaplen: %s.  Snaplen must be between "
                   "%u and %u inclusive or 0 for default = %u.",
                   args, MIN_SNAPLEN, MAX_SNAPLEN, DAQ_GetSnapLen());
    }

    sc->pkt_snaplen = snaplen;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,
        "Snap length of packets set to: %d\n", sc->pkt_snaplen););
}

void ConfigPidPath(SnortConfig *sc, char *args)
{
    if ((args == NULL) || (sc == NULL))
        return;

    LogMessage("Found pid path directive (%s)\n", args);

    sc->run_flags |= RUN_FLAG__CREATE_PID_FILE;
    if (SnortStrncpy(sc->pid_path, args, sizeof(sc->pid_path)) != SNORT_STRNCPY_SUCCESS)
        ParseError("Pid path too long.");
    
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Pid Path directory = %s\n", 
                            sc->pid_path););
}

void ConfigPolicy(SnortConfig *sc, char *args)
{
    int num_toks;
    char **toks;

    if ((sc == NULL) || (sc->targeted_policies == NULL))
        return;

    toks = mSplit(args, " \t,", 20, &num_toks, 0);

    if (num_toks != 1)
    {
        mSplitFree(&toks, num_toks);
        return;
    }

    sc->targeted_policies[getParserPolicy()]->configPolicyId = (unsigned short)(atoi(toks[0]));

    mSplitFree(&toks, num_toks);
}

void ConfigPolicyMode(SnortConfig *sc, char *args)
{
    if ((sc == NULL) || (sc->targeted_policies == NULL))
        return;

    sc->targeted_policies[getParserPolicy()]->policy_mode = GetPolicyMode(args, sc->run_flags);
}

static int GetPolicyMode(char *args, int run_flags)
{
    char **toks;
    int num_toks;
    int i;
    int mode = 0;

    if (args == NULL)
    {
        if ( run_flags & RUN_FLAG__INLINE )
            return POLICY_MODE__INLINE;
        else if ( run_flags & RUN_FLAG__INLINE_TEST )
            return POLICY_MODE__INLINE_TEST;
        else
            return POLICY_MODE__PASSIVE;
    }

    toks = mSplit(args, " \t", 10, &num_toks, 0);
    for (i = 0; i < num_toks; i++)
    {
        if (strcasecmp(toks[i], POLICY_MODE_PASSIVE) == 0)
        {
            if ( ScAdapterInlineTestMode() )
                mode = POLICY_MODE__INLINE_TEST;
            else
                mode = POLICY_MODE__PASSIVE;
        }
        else if (strcasecmp(toks[i], POLICY_MODE_INLINE) == 0)
        {
            /* If --enable-inline-test is specified it overwrites
             * policy_mode: inline */
            if( ScAdapterInlineTestMode() )
                mode = POLICY_MODE__INLINE_TEST;
            else if (!ScAdapterInlineMode())
            {
               ParseWarning("Adapter is in Passive Mode. Hence switching " 
                       "policy mode to tap.");
               mode = POLICY_MODE__PASSIVE;

            }
            else
                mode = POLICY_MODE__INLINE;
        }
        else if (strcasecmp(toks[i], POLICY_MODE_INLINE_TEST) == 0)
        {
            mode = POLICY_MODE__INLINE_TEST;
        }
        else
        {
            ParseError("Unknown command line policy mode option: %s.", toks[i]);
        }
    }

    mSplitFree(&toks, num_toks);
    return mode;
}

void ConfigPolicyVersion(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;

    if ((sc == NULL) || (sc->targeted_policies == NULL) ||
        (snort_conf_file == NULL) || (args == NULL))
    {
        return;
    }

    toks = mSplit(args, " \t", 0, &num_toks, 0);

    if (num_toks == 1)
    {
        if (getParserPolicy() != getDefaultPolicy())
        {
            ParseError("Targeted policies must have base policy version and "
                       "targeted policy version.");
        }

        if (sc->base_version == NULL)
        {
            sc->base_version = SnortStrdup(toks[0]);
        }
        else
        {
            if (strcmp(sc->base_version, toks[0]) != 0)
            {
                ParseError("Base policy version mismatch: Current base "
                           "version: %s, %s: %s",
                           sc->base_version, file_name, toks[0]);
            }
        }
    }
    else if (num_toks == 2)
    {
        SnortPolicy *p = sc->targeted_policies[getParserPolicy()];

        if (getParserPolicy() == getDefaultPolicy())
            ParseError("Base policy must only have one policy version.");

        if (sc->base_version == NULL)
        {
            ParseError("Config option \"%s\" must be configured in \"%s\"",
                       CONFIG_OPT__POLICY_VERSION, snort_conf_file);
        }

        if (strcmp(sc->base_version, toks[0]) != 0)
        {
            ParseError("Base policy version mismatch: Current base "
                       "version: %s, %s: %s",
                       sc->base_version, file_name, toks[0]);
        }

        if (p->policy_version == NULL)
        {
            p->policy_version = SnortStrdup(toks[1]);
        }
        else
        {
            if (strcmp(p->policy_version, toks[1]) != 0)
            {
                ParseError("Targeted policy version mismatch: Current targeted "
                           "policy version: %s, %s: %s",
                           p->policy_version, file_name, toks[1]);
            }
        }
    }
    else
    {
        ParseError("Invalid number of arguments to \"%s\": %s.",
                   CONFIG_OPT__POLICY_VERSION, args);
    }

    mSplitFree(&toks, num_toks);
}

#ifdef PPM_MGR
/*
 * config ppm: feature, feature, feature,..
 * 
 * config ppm: max-pkt-time usecs,
 *             disable-pkt-inspection,
 *             max-rule-time usecs, 
 *             disable-rule-inspection, threshold 5,
 *             max-suspend-time secs,
 *             rule-events alert|syslog|console,
 *             pkt-events  alert|syslog|console,
 *             debug,
 *             debug-pkts
 */
void ConfigPPM(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    int i;
    char *endptr;
    unsigned long int val;
    char **opts;
    int num_opts;
    int pktOpts = 0, ruleOpts = 0;

    if (sc == NULL)
        return;
  
    toks = mSplit(args, ",", 0, &num_toks, 0);

    if (!sc->ppm_cfg.enabled)
        ppm_init(&sc->ppm_cfg);

    /* defaults are set by ppm_init() */
    for(i = 0; i < num_toks; i++)
    {
        opts = mSplit(toks[i], " \t", 0, &num_opts, 0);
        
        if (strcasecmp(opts[0], PPM_OPT__MAX_PKT_TIME) == 0)
        {
            if (num_opts != 2)
            {
                ParseError("config ppm: missing argument for '%s'.", opts[0]);
            }

            val = SnortStrtoul(opts[1], &endptr, 0);
            if ((opts[1][0] == '-') || (errno == ERANGE) || (*endptr != '\0'))
            {
                ParseError("config ppm: Invalid %s '%s'.", opts[0], opts[1]);
            }

            ppm_set_max_pkt_time(&sc->ppm_cfg, val);
        }
        else if (strcasecmp(opts[0], PPM_OPT__MAX_RULE_TIME) == 0)
        {
            if (num_opts != 2)
            {
                ParseError("config ppm: missing argument for '%s'.", opts[0]);
            }

            val = SnortStrtoul(opts[1], &endptr, 0);
            if ((opts[1][0] == '-') || (errno == ERANGE) || (*endptr != '\0'))
            {
                ParseError("config ppm: Invalid %s '%s'.", opts[0], opts[1]);
            }
           
            ppm_set_max_rule_time(&sc->ppm_cfg, val);
        }
        else if (strcasecmp(opts[0], PPM_OPT__SUSPEND_TIMEOUT) == 0)
        {
            if (num_opts != 2)
            {
                ParseError("config ppm: missing argument for '%s'.", opts[0]);
            }

            val = SnortStrtoul(opts[1], &endptr, 0);
            if ((opts[1][0] == '-') || (errno == ERANGE) || (*endptr != '\0'))
            {
                ParseError("config ppm: Invalid %s '%s'.", opts[0], opts[1]);
            }
           
            ppm_set_max_suspend_time(&sc->ppm_cfg, val);
            ruleOpts++;
        }
        else if (strcasecmp(opts[0], PPM_OPT__SUSPEND_EXP_RULES) == 0)
        {
            if (num_opts != 1)
            {
                ParseError("config ppm: too many arguments for '%s'.", opts[0]);
            }

            ppm_set_rule_action(&sc->ppm_cfg, PPM_ACTION_SUSPEND);
            ruleOpts++;
        }
        else if (strcasecmp(opts[0], PPM_OPT__THRESHOLD) == 0)
        {
            if (num_opts != 2)
            {
                ParseError("config ppm: missing argument for '%s'.", opts[0]);
            }

            val = SnortStrtoul(opts[1], &endptr, 0);
            if ((opts[1][0] == '-') || (errno == ERANGE) || (*endptr != '\0'))
            {
                ParseError("config ppm: Invalid %s '%s'.", opts[0], opts[1]);
            }

            ppm_set_rule_threshold(&sc->ppm_cfg, val);
            ruleOpts++;
        }
        else if (strcasecmp(opts[0], PPM_OPT__FAST_PATH_EXP_PKTS) == 0)
        {
            if (num_opts != 1)
            {
                ParseError("config ppm: too many arguments for '%s'.", opts[0]);
            }

            ppm_set_pkt_action(&sc->ppm_cfg, PPM_ACTION_SUSPEND);
            pktOpts++;
        }
        else if (strcasecmp(opts[0], PPM_OPT__PKT_LOG) == 0)
        {
            if (num_opts != 1)
            {
                ParseError("config ppm: too many arguments for '%s'.", opts[0]);
            }

            ppm_set_pkt_log(&sc->ppm_cfg, PPM_LOG_MESSAGE);
            pktOpts++;
        }       
        else if (strcasecmp(opts[0], PPM_OPT__RULE_LOG) == 0)
        {
            int k;
          
            if (num_opts == 1)
            {
                ParseError("config ppm: insufficient %s opts.", opts[0]);
            }
         
            for (k = 1; k < num_opts; k++)
            {
                if (strcasecmp(opts[k], PPM_OPT__ALERT) == 0)
                {
                    ppm_set_rule_log(&sc->ppm_cfg, PPM_LOG_ALERT);
                }
                else if (strcasecmp(opts[k], PPM_OPT__LOG) == 0)
                {
                    ppm_set_rule_log(&sc->ppm_cfg, PPM_LOG_MESSAGE);
                }
                else
                {
                    ParseError("config ppm: Invalid %s arg '%s'.", opts[0], opts[k]);
                }
            }

            ruleOpts++;
        }       
        else if (strcasecmp(opts[0], PPM_OPT__DEBUG_PKTS) == 0)
        {
            if (num_opts != 1)
            {
                ParseError("config ppm: too many arguments for '%s'.", opts[0]);
            }

            ppm_set_debug_pkts(&sc->ppm_cfg, 1);
            pktOpts++;
        }
#if 0
        else if(!strcasecmp(opts[0], "debug-rules"))
        {
            if( 1 != num_opts )
                ParseError("config ppm: too many arguments for '%s'.", opts[0]);
            ppm_set_debug_rules(1);
        }
#endif
        else
        {
            ParseError("'%s' is an invalid option to the 'config ppm:' "
                       "configuration.", opts[0]);
        }
    
        mSplitFree(&opts, num_opts);
    }

    mSplitFree(&toks, num_toks);
}
#endif

void ConfigProcessAllEvents(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__PROCESS_ALL_EVENTS;
}

#ifdef PERF_PROFILING
/* Profiling configurations are done later after log directory has
 * absolutely been set */
static void _ConfigProfilePreprocs(SnortConfig *sc, char *args)
{
    return;
}

void ConfigProfilePreprocs(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    int i;

    if (sc == NULL)
        return;

    LogMessage("Found profile_preprocs config directive (%s)\n",
               args == NULL ? "<no args>" : args);

    /* Initialize the defaults */
    sc->profile_preprocs.num = -1;
    sc->profile_preprocs.sort = PROFILE_SORT_AVG_TICKS;

    toks = mSplit(args, ",", 0, &num_toks, 0);

    if (num_toks > 3)
    {
        ParseError("profile_preprocs speciified with invalid options (%s)", args);
    }

    for (i = 0; i < num_toks; i++)
    {
        char **opts;
        int num_opts;
        int opt_filename = 0;
        char *endptr;

        opts = mSplit(toks[i], " \t", 0, &num_opts, 0);
        if (num_opts > 0)
        {
            opt_filename = strcasecmp(opts[0], PROFILE_OPT__FILENAME) == 0;
        }

        if ((!opt_filename && (num_opts != 2)) ||
            (opt_filename && ((num_opts > 3) || (num_opts < 2))))
        {
            ParseError("profile_preprocs has an invalid option (%s)", toks[i]);
        }

        if (strcasecmp(opts[0], PROFILE_OPT__PRINT) == 0)
        {
            if (strcasecmp(opts[1], PROFILE_OPT__ALL) == 0)
            {
                sc->profile_preprocs.num = -1;
            }
            else
            {
                sc->profile_preprocs.num = SnortStrtol(opts[1], &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0'))
                {
                    ParseError("Invalid argument to profile_preprocs 'print' "
                               "configuration: %s", opts[1]);
                }
            }
        }
        else if (strcasecmp(opts[0], PROFILE_OPT__SORT) == 0)
        {
            if (strcasecmp(opts[1], PROFILE_OPT__CHECKS) == 0)
            {
                sc->profile_preprocs.sort = PROFILE_SORT_CHECKS;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__AVG_TICKS) == 0)
            {
                sc->profile_preprocs.sort = PROFILE_SORT_AVG_TICKS;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__TOTAL_TICKS) == 0)
            {
                sc->profile_preprocs.sort = PROFILE_SORT_TOTAL_TICKS;
            }
            else
            {
                ParseError("profile_preprocs has an invalid sort option (%s)", toks[i]);
            }
        }
        else if (strcasecmp(opts[0], PROFILE_OPT__FILENAME) == 0)
        {
            sc->profile_preprocs.filename = ProcessFileOption(sc, opts[1]);
            if (opts[2] && (strcasecmp(opts[2], PROFILE_OPT__APPEND) == 0))
            {
                sc->profile_preprocs.append = 1;   
            }
            else
            {
                sc->profile_preprocs.append = 0;
            }
        }
        else
        {
            ParseError("profile_preprocs has an invalid option (%s)", toks[i]);
        }

        mSplitFree(&opts, num_opts);
    }

    mSplitFree(&toks, num_toks);
}

static void _ConfigProfileRules(SnortConfig *sc, char *args)
{
    return;
}

void ConfigProfileRules(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    int i;

    if (sc == NULL)
        return;

    LogMessage("Found profile_rules config directive (%s)\n",
               args == NULL ? "<no args>" : args);

    /* Initialize the defaults */
    sc->profile_rules.num = -1;
    sc->profile_rules.sort = PROFILE_SORT_AVG_TICKS;

    toks = mSplit(args, ",", 0, &num_toks, 0);

    if (num_toks > 3)
    {
        ParseError("profile_rules speciified with invalid options (%s)", args);
    }

    for (i = 0; i < num_toks;i++)
    {
        char **opts;
        int num_opts;
        int opt_filename = 0;
        char *endptr;

        opts = mSplit(toks[i], " \t", 0, &num_opts, 0);
        if (num_opts > 0)
        {
            opt_filename = strcasecmp(opts[0], PROFILE_OPT__FILENAME) == 0;
        }

        if ((!opt_filename && (num_opts != 2)) ||
            (opt_filename && ((num_opts > 3) || (num_opts < 2))))
        {
            ParseError("profile_rules has an invalid option (%s)", toks[i]);
        }

        if (strcasecmp(opts[0], PROFILE_OPT__PRINT) == 0)
        {
            if (strcasecmp(opts[1], PROFILE_OPT__ALL) == 0)
            {
                sc->profile_rules.num = -1;
            }
            else
            {
                sc->profile_rules.num = SnortStrtol(opts[1], &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0'))
                {
                    ParseError("Invalid argument to profile_rules 'print' "
                               "configuration: %s", opts[1]);
                }
            }
        }
        else if (strcasecmp(opts[0], PROFILE_OPT__SORT) == 0)
        {
            if (strcasecmp(opts[1], PROFILE_OPT__CHECKS) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_CHECKS;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__MATCHES) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_MATCHES;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__NO_MATCHES) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_NOMATCHES;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__AVG_TICKS) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_AVG_TICKS;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__AVG_TICKS_PER_MATCH) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_AVG_TICKS_PER_MATCH;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__AVG_TICKS_PER_NO_MATCH) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_AVG_TICKS_PER_NOMATCH;
            }
            else if (strcasecmp(opts[1], PROFILE_OPT__TOTAL_TICKS) == 0)
            {
                sc->profile_rules.sort = PROFILE_SORT_TOTAL_TICKS;
            }
            else
            {
                ParseError("profile_rules has an invalid sort option (%s)", toks[i]);
            }
        }
        else if (strcasecmp(opts[0], PROFILE_OPT__FILENAME) == 0)
        {
            sc->profile_rules.filename = ProcessFileOption(sc, opts[1]);
            if (opts[2] && (strcasecmp(opts[2], PROFILE_OPT__APPEND) == 0))
            {
                sc->profile_rules.append = 1;   
            }
            else
            {
                sc->profile_rules.append = 0;
            }
        }
        else
        {
            ParseError("profile_rules has an invalid option (%s)", toks[i]);
        }

        mSplitFree(&opts, num_opts);
    }

    mSplitFree(&toks, num_toks);
}
#endif

void ConfigQuiet(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->logging_flags |= LOGGING_FLAG__QUIET;
}

/*
 * Process the 'config rate_filter: memcap <#bytes>'
 */
// TBD refactor - was cloned from sfthreshold.c
void ConfigRateFilter(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;

    if ((sc == NULL) || (args == NULL))
        return;
           
    toks = mSplit(args, " \t", 2, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseError("Rate filter memcap requires a positive integer argument.");
    }

    if (strcasecmp(toks[0], "memcap") == 0)
    {
        char *endptr;

        sc->rate_filter_config->memcap = SnortStrtol(toks[1], &endptr, 0);
        if ((errno == ERANGE) || (*endptr != '\0') ||
            (sc->rate_filter_config->memcap < 0))
        {
            ParseError("Invalid rate filter memcap: %s.  Must be a "
                       "positive integer.", toks[1]);
        }
    }
    else
    {
        ParseError("Unknown argument to rate filter configuration: %s.", toks[0]);
    }

    mSplitFree(&toks, num_toks);
}


/* Add anything here to what was specified on the command line */
void ConfigReadPcapFile(SnortConfig *sc, char *args)
{
    if ((sc == NULL) || (args == NULL))
        return;

    /* Look for interface incompatibility and reading config */
    snort_conf->run_flags |= RUN_FLAG__READ;
    snort_conf->pcap_file = SnortStrdup(args);
}

void ConfigReference(SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    char *url = NULL;

    if ((sc == NULL) || (args == NULL))
        return;

    /* 2 tokens: name <url> */
    toks = mSplit(args, " \t", 0, &num_toks, 0);
    if (num_toks > 2)
    {
        ParseError("Reference config requires at most two arguments: "
                   "\"name [<url>]\".");
    }

    if (num_toks == 2)
        url = toks[1];

    ReferenceSystemAdd(&sc->references, toks[0], url);

    mSplitFree(&toks, num_toks);
}

/*
 * Function: ConfigReferenceNet
 *
 * Purpose: Translate the command line character string into its equivalent
 *          32-bit network byte ordered value (with netmask)
 *
 * Arguments: args => The address/CIDR block
 *
 * Returns: void function
 */
void ConfigReferenceNet(SnortConfig *sc, char *args)
{
#ifndef SUP_IP6
    struct in_addr net;    /* place to stick the local network data */
    char **toks;           /* dbl ptr to store mSplit return data in */
    int num_toks;          /* number of tokens mSplit returns */
    int nmask;             /* temporary netmask storage */
# ifdef DEBUG
    struct in_addr sin;
# endif
#endif

    if ((sc == NULL) || (args == NULL))
        return;

#ifdef SUP_IP6
    sfip_pton(args, &sc->homenet);
#else

    /* break out the CIDR notation from the IP address */
    toks = mSplit(args, "/", 2, &num_toks, 0);

    if(num_toks > 1)
    {
        /* convert the CIDR notation into a real live netmask */
        nmask = atoi(toks[1]);

        if((nmask > 0) && (nmask < 33))
        {
            sc->netmask = htonl(netmasks[nmask]);
        }
        else
        {
            ParseError("Bad CIDR block (%s) in obfuscation mask %s. "
                       "1 to 32 please!", toks[1], args);
        }
    }
    else
    {
        ParseError("No netmask specified for home network!");
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "homenet netmask = %#8lX\n",
                            sc->netmask););

    /* convert the IP addr into its 32-bit value */
    if((net.s_addr = inet_addr(toks[0])) == INADDR_NONE)
        ParseError("Homenet (%s) didn't translate", toks[0]);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Net = %s (%X)\n",
                            inet_ntoa(net), net.s_addr););

    /* set the final homenet address up */
    sc->homenet = net.s_addr & sc->netmask;

# ifdef DEBUG
    sin.s_addr = sc->homenet;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Homenet = %s (%X)\n",
                            inet_ntoa(sin), sin.s_addr););
# endif

    mSplitFree(&toks, num_toks);
#endif
}

void ConfigSetGid(SnortConfig *sc, char *args)
{
#ifdef WIN32
    ParseError("Setting the group id is not supported in the "
               "WIN32 port of snort!");
#else
    size_t i;
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    for (i = 0; i < strlen(args); i++)
    {
        /* If we get something other than a digit, assume it's
         * a group name */
        if (!isdigit((int)args[i]))
        {
            struct group *gr = getgrnam(args);

            if (gr == NULL)
                ParseError("Group \"%s\" unknown.", args);

            sc->group_id = gr->gr_gid;
            break;
        }
    }

    /* It's all digits.  Assume it's a group id */
    if (i == strlen(args))
    {
        sc->group_id = SnortStrtol(args, &endptr, 10);
        if ((errno == ERANGE) || (*endptr != '\0') ||
            (sc->group_id < 0))
        {
            ParseError("Group id \"%s\" out of range.", args);
        }
    }
#endif
}

void ConfigSetUid(SnortConfig *sc, char *args)
{
#ifdef WIN32
    ParseError("Setting the user id is not supported in the "
               "WIN32 port of snort!");
#else
    size_t i;
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    for (i = 0; i < strlen(args); i++)
    {
        /* If we get something other than a digit, assume it's
         * a user name */
        if (!isdigit((int)args[i]))
        {
            struct passwd *pw = getpwnam(args);

            if (pw == NULL)
                ParseError("User \"%s\" unknown.", args);

            sc->user_id = (int)pw->pw_uid;

            /* Why would someone want to run as another user
             * but still as root group? */
            if (sc->group_id == -1)
                sc->group_id = (int)pw->pw_gid;

            break;
        }
    }

    /* It's all digits.  Assume it's a user id */
    if (i == strlen(args))
    {
        sc->user_id = SnortStrtol(args, &endptr, 10);
        if ((errno == ERANGE) || (*endptr != '\0'))
            ParseError("User id \"%s\" out of range.", args);

        /* Set group id to user's default group if not
         * already set */
        if (sc->group_id == -1)
        {
            struct passwd *pw = getpwuid((uid_t)sc->user_id);

            if (pw == NULL)
                ParseError("User \"%s\" unknown.", args);

            sc->group_id = (int)pw->pw_gid;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "UserID: %d GroupID: %d.\n",
                            sc->user_id, sc->group_id););
#endif  /* !WIN32 */
}

void ConfigShowYear(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->output_flags |= OUTPUT_FLAG__INCLUDE_YEAR;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Enabled year in timestamp\n"););
}

void ConfigSoRuleMemcap(SnortConfig *sc, char *args)
{
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    sc->so_rule_memcap = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid so rule memcap: %s.  Memcap must be between "
                   "0 and %u inclusive.", args, UINT32_MAX);
    }
}

void ConfigStateful(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    /* If stream5 is configured, the STATEFUL flag is set.  This is
     * somewhat misnamed and is used to assure a session is established */
    sc->run_flags |= RUN_FLAG__ASSURE_EST;
}

void ConfigTaggedPacketLimit(SnortConfig *sc, char *args)
{
    char *endptr;

    if ((sc == NULL) || (args == NULL))
        return;

    sc->tagged_packet_limit = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') ||
        (sc->tagged_packet_limit < 0))
    {
        ParseError("Invalid tagged_packet_limit: %s.  Must be a "
                   "positive integer.", args);
    }
}

/*
   Process the 'config threshold: memcap #bytes, option2-name option2-value, ...'

   config threshold: memcap #bytes
*/
void ConfigThreshold(SnortConfig *sc, char *args)
{
    static int warned = 0;

    if (!warned)
    {
        ParseWarning("config threshold is deprecated;"
                   " use config event_filter instead.\n");

        warned = 1;
    }

    ConfigEventFilter(sc, args);
}

void ConfigTreatDropAsAlert(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__TREAT_DROP_AS_ALERT;
}

void ConfigTreatDropAsIgnore(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->run_flags |= RUN_FLAG__TREAT_DROP_AS_IGNORE;
}

void ConfigUmask(SnortConfig *sc, char *args)
{
#ifdef WIN32
    ParseError("Setting the umask is not supported in the "
               "WIN32 port of snort!");
#else
    char *endptr;
    long mask;

    if ((sc == NULL) || (args == NULL))
        return;

    mask = SnortStrtol(args, &endptr, 0);

    if ((errno == ERANGE) || (*endptr != '\0') ||
        (mask < 0) || (mask & ~FILEACCESSBITS))
    {
        ParseError("Bad umask: %s", args);
    }
    sc->file_mask = (mode_t)mask;
#endif
}

void ConfigUtc(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->output_flags |= OUTPUT_FLAG__USE_UTC;
}

void ConfigVerbose(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    sc->logging_flags |= LOGGING_FLAG__VERBOSE;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Verbose Flag active\n"););
}

void ConfigVlanAgnostic(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "VLAN Agnostic active\n"););
    sc->vlan_agnostic = 1; /* TODO: add this to some existing flag bitfield? */
}

#ifdef DYNAMIC_PLUGIN
void ConfigDumpDynamicRulesPath(SnortConfig *sc, char *args)
{
    if (sc == NULL)
        return;
    sc->run_mode_flags |= RUN_MODE_FLAG__RULE_DUMP;
    if ( args != NULL )
        sc->dynamic_rules_path = SnortStrdup(args);
}
#endif

/****************************************************************************
 *
 * Function: ParseRule()
 *
 * Purpose:  Process an individual rule and add it to the rule list
 *
 * Arguments: rule => rule string
 *
 * Returns: void function
 *
 ***************************************************************************/
static void ParseRule(SnortConfig *sc, SnortPolicy *p, char *args,
                      RuleType rule_type, ListHead *list)
{
    char **toks = NULL;
    int num_toks = 0;
    int protocol = 0;
    RuleTreeNode test_rtn;
    RuleTreeNode *rtn;
    OptTreeNode *otn;
    char *roptions = NULL;
    port_entry_t pe;
    PortVarTable *portVarTable = p->portVarTable;
    PortTable *nonamePortVarTable = p->nonamePortVarTable;

    if ((sc == NULL) || (args == NULL))
      return;

    memset(&test_rtn, 0, sizeof(RuleTreeNode));
  
    memset(&pe, 0, sizeof(pe));

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"[*] Rule start\n"););

    /* We have a preproc or decoder rule - we assume a header of 'tcp any any -> any any ' */
    if (*args == '(')
    {
        test_rtn.flags |= ANY_DST_PORT;
        test_rtn.flags |= ANY_SRC_PORT;
        test_rtn.flags |= ANY_DST_IP;
        test_rtn.flags |= ANY_SRC_IP;
        test_rtn.flags |= BIDIRECTIONAL;
        test_rtn.type = rule_type;
        protocol = IPPROTO_TCP;

        roptions = args;

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Preprocessor Rule detected\n"););
    }
    else
    {
        /* proto ip port dir ip port r*/
        toks = mSplit(args, " \t", 7, &num_toks, '\\');

        /* A rule might not have rule options */
        if (num_toks < 6)
        {
            ParseError("Bad rule in rules file: %s", args);
        }

        if (num_toks == 7)
            roptions = toks[6];

        test_rtn.type = rule_type;

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Non-Preprocessor Rule detected\n"););

        /* Set the rule protocol - fatal errors if protocol not found */
        protocol = GetRuleProtocol(toks[0]);
        test_rtn.proto = protocol;

        switch (protocol)
        {
            case IPPROTO_TCP:
                sc->ip_proto_array[IPPROTO_TCP] = 1;
                break;
            case IPPROTO_UDP:
                sc->ip_proto_array[IPPROTO_UDP] = 1;
                break;
            case IPPROTO_ICMP:
                sc->ip_proto_array[IPPROTO_ICMP] = 1;
                sc->ip_proto_array[IPPROTO_ICMPV6] = 1;

                if ( rule_type == RULE_TYPE__REJECT )
                    test_rtn.type = rule_type = RULE_TYPE__ALERT;
                break;
            case ETHERNET_TYPE_IP:
                /* This will be set via ip_protos */
                break;
            default:
                ParseError("Bad protocol: %s", toks[0]);
                break;
        }


        /* Process the IP address and CIDR netmask - changed version 1.2.1
         * "any" IP's are now set to addr 0, netmask 0, and the normal rules are
         * applied instead of checking the flag if we see a "!<ip number>" we
         * need to set a flag so that we can properly deal with it when we are
         * processing packets. */
        ProcessIP(sc, toks[1], &test_rtn, SRC, 0);

        /* Check to make sure that the user entered port numbers.
         * Sometimes they forget/don't know that ICMP rules need them */
        if ((strcasecmp(toks[2], RULE_DIR_OPT__DIRECTIONAL) == 0) ||
            (strcasecmp(toks[2], RULE_DIR_OPT__BIDIRECTIONAL) == 0))
        {
            ParseError("Port value missing in rule!");
        }

        DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,"Src-Port: %s\n",toks[2]););

        if (ParsePortList(&test_rtn, portVarTable, nonamePortVarTable,
                          toks[2], protocol, 0 /* =src port */ ))
        {
            ParseError("Bad source port: '%s'", toks[2]);
        }

        /* changed version 1.8.4
         * Die when someone has tried to define a rule character other
         * than -> or <> */
        if ((strcmp(toks[3], RULE_DIR_OPT__DIRECTIONAL) != 0) &&
            (strcmp(toks[3], RULE_DIR_OPT__BIDIRECTIONAL) != 0))
        {
            ParseError("Illegal direction specifier: %s", toks[3]);
        }

        /* New in version 1.3: support for bidirectional rules
         * This checks the rule "direction" token and sets the bidirectional
         * flag if the token = '<>' */
        if (strcmp(toks[3], RULE_DIR_OPT__BIDIRECTIONAL) == 0)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Bidirectional rule!\n"););
            test_rtn.flags |= BIDIRECTIONAL;
        }

        /* changed version 1.2.1
         * "any" IP's are now set to addr 0, netmask 0, and the normal rules are
         * applied instead of checking the flag
         * If we see a "!<ip number>" we need to set a flag so that we can
         * properly deal with it when we are processing packets */
        ProcessIP(sc, toks[4], &test_rtn, DST, 0);

        DEBUG_WRAP(DebugMessage(DEBUG_PORTLISTS,"Dst-Port: %s\n", toks[5]););

        if (ParsePortList(&test_rtn, portVarTable, nonamePortVarTable,
                          toks[5], protocol, 1 /* =dst port */ ))
        {
            ParseError("Bad destination port: '%s'", toks[5]);
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"test_rtn.flags = 0x%X\n", test_rtn.flags););
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Processing Head Node....\n"););

    test_rtn.listhead = list;

    rtn = ProcessHeadNode(sc, &test_rtn, list);
    /* The IPs in the test node get free'd in ProcessHeadNode if there is
     * already a matching RTN.  The portobjects will get free'd when the
     * port var table is free'd */

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Parsing Rule Options...\n"););

    otn = ParseRuleOptions(sc, rtn, roptions, rule_type, protocol);
    if (otn == NULL)
    {
        /* This otn is a dup and we're choosing to keep the old one */
        mSplitFree(&toks, num_toks);
        return;
    }

    rule_count++;
    
    /* Get rule option info */
    pe.gid = otn->sigInfo.generator;
    pe.sid = otn->sigInfo.id;

    /* Have to have at least 6 toks */
    if (num_toks != 0)
    {
        pe.protocol = SnortStrdup(toks[0]);
        pe.src_port = SnortStrdup(toks[2]);
        pe.dst_port = SnortStrdup(toks[5]);
    }

    /* See what kind of content is going in the fast pattern matcher */
#ifdef DYNAMIC_PLUGIN
    if (otn->ds_list[PLUGIN_DYNAMIC] != NULL)
    {
        DynamicData *dd = (DynamicData *)otn->ds_list[PLUGIN_DYNAMIC];
        if (dd->contentFlags & CONTENT_HTTP)
            pe.uricontent = 1;
        else if (dd->contentFlags & CONTENT_NORMAL)
            pe.content = 1;
    }
    else
#endif
    {
        /* Since http_cookie content is not used in fast pattern matcher,
         * need to iterate the entire list */
        if (otn->ds_list[PLUGIN_PATTERN_MATCH_URI] != NULL)
        {
            PatternMatchData *pmd = otn->ds_list[PLUGIN_PATTERN_MATCH_URI];
            for (; pmd != NULL; pmd = pmd->next)
            {
                if((pmd->uri_buffer) && IsHttpBufFpEligible(pmd->uri_buffer))
                {
                    pe.uricontent = 1;
                    break;
                }
            }
        }

        if (!pe.uricontent && ((otn->ds_list[PLUGIN_PATTERN_MATCH] != NULL)
                    || (otn->ds_list[PLUGIN_PATTERN_MATCH_OR] != NULL)))
        {
            pe.content = 1;
        }
    }

    if (rtn->flags & BIDIRECTIONAL)
         pe.dir = 1;

    pe.proto = protocol;
    pe.rule_type = rule_type;

    port_list_add_entry(&port_list, &pe);

    /* 
     * The src/dst port parsing must be done before the Head Nodes are processed, since they must
     * compare the ports/port_objects to find the right rtn list to add the otn rule to.
     * 
     * After otn processing we can finalize port object processing for this rule
     */
    if (FinishPortListRule(sc->port_tables, rtn, otn, protocol, &pe, sc->fast_pattern_config))
        ParseError("Failed to finish a port list rule.");

    mSplitFree(&toks, num_toks);
}

/****************************************************************************
 *
 * Function: ProcessHeadNode(RuleTreeNode *, ListHead *, int)
 *
 * Purpose:  Process the header block info and add to the block list if
 *           necessary
 *
 * Arguments: test_node => data generated by the rules parsers
 *            list => List Block Header refernece
 *            protocol => ip protocol
 *
 * Returns: void function
 *
 ***************************************************************************/
static RuleTreeNode * ProcessHeadNode(SnortConfig *sc, RuleTreeNode *test_node,
                                      ListHead *list)
{
    RuleTreeNode *rtn = findHeadNode(sc, test_node, getParserPolicy());

    /* if it doesn't match any of the existing nodes, make a new node and
     * stick it at the end of the list */
    if (rtn == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Building New Chain head node\n"););

        rtn = (RuleTreeNode *)SnortAlloc(sizeof(RuleTreeNode));

        rtn->otnRefCount++;

        /* copy the prototype header info into the new header block */
        XferHeader(test_node, rtn);

        head_count++;
        rtn->head_node_number = head_count;

        /* initialize the function list for the new RTN */
        SetupRTNFuncList(rtn);

        /* add link to parent listhead */
        rtn->listhead = list;

        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,
                "New Chain head flags = 0x%X\n", rtn->flags););
    }
    else
    {
        rtn->otnRefCount++;
        FreeRuleTreeNode(test_node);
    }

    return rtn;
}

#if 0
#ifdef DEBUG
static void PrintRtnPorts(RuleTreeNode *rtn_list)
{
    int i = 0;
    char buf[STD_BUF];

    SnortSnprintf(buf, STD_BUF, "%s", "    ");

    while (rtn_list != NULL)
    {
        if (rtn_list->flags & EXCEPT_DST_PORT)
        {
            SnortSnprintfAppend(buf, STD_BUF, "!");
        }

        SnortSnprintfAppend(buf, STD_BUF, "%d ", rtn_list->ldp);

        rtn_list = rtn_list->right;

        if (i == 15)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "%s\n", buf););

            i = 0;

            SnortSnprintf(buf, STD_BUF, "%s", "     ");
        }

        i++;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "%s\n", buf););
}
#endif
#endif

static void ParseActivate(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Activation rule\n"););
    ParseRule(sc, p, args, RULE_TYPE__ACTIVATE, &sc->Activation);
}

static void ParseAlert(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Alert\n"););
    ParseRule(sc, p, args, RULE_TYPE__ALERT, &sc->Alert);
}

static void ParseDrop(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Drop\n"););

    /* Parse as an alert if we're treating drops as alerts */
    if (ScTreatDropAsAlert())
        ParseRule(sc, p, args, RULE_TYPE__ALERT, &sc->Alert);

    else if ( ScKeepDropRules() || ScLoadAsDropRules() )
        ParseRule(sc, p, args, RULE_TYPE__DROP, &sc->Drop);
}

static void ParseDynamic(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Dynamic rule\n"););
    ParseRule(sc, p, args, RULE_TYPE__DYNAMIC, &sc->Dynamic);
}

static void ParseLog(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Log\n"););
    ParseRule(sc, p, args, RULE_TYPE__LOG, &sc->Log);
}

static void ParsePass(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Pass\n"););
    ParseRule(sc, p, args, RULE_TYPE__PASS, &sc->Pass);
}

static void ParseReject(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Reject\n"););
    ParseRule(sc, p, args, RULE_TYPE__REJECT, &sc->Reject);
#ifdef ACTIVE_RESPONSE
    Active_SetEnabled(1);
#endif
}

static void ParseSdrop(SnortConfig *sc, SnortPolicy *p, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "SDrop\n"););

    if ( ScKeepDropRules() && !ScTreatDropAsAlert() )
        ParseRule(sc, p, args, RULE_TYPE__SDROP, &sc->SDrop);
}

static void ParsePortVar(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "PortVar\n"););

    toks = mSplit(args, " \t", 0, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseError("Missing argument to %s", toks[0]);
    }

    /* Check command line variables to see if this has already
     * been defined */
    if (cmd_line_var_list != NULL)
    {
        VarNode *tmp = cmd_line_var_list;

        while (tmp != NULL)
        {
            /* Already defined this via command line */
            if (strcasecmp(toks[0], tmp->name) == 0)
            {
                mSplitFree(&toks, num_toks);
                return;
            }

            tmp = tmp->next;
        }
    }

    PortVarDefine(sc, toks[0], toks[1]);

    mSplitFree(&toks, num_toks);
}

#ifdef SUP_IP6
static void ParseIpVar(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    int ret;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "IpVar\n"););

    toks = mSplit(args, " \t", 0, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseError("Missing argument to %s", toks[0]);
    }

    /* Check command line variables to see if this has already
     * been defined */
    if (cmd_line_var_list != NULL)
    {
        VarNode *tmp = cmd_line_var_list;

        while (tmp != NULL)
        {
            /* Already defined this via command line */
            if (strcasecmp(toks[0], tmp->name) == 0)
            {
                mSplitFree(&toks, num_toks);
                return;
            }

            tmp = tmp->next;
        }
    }

    DisallowCrossTableDuplicateVars(sc, toks[0], VAR_TYPE__IPVAR);

    if((ret = sfvt_define(p->ip_vartable, toks[0], toks[1])) != SFIP_SUCCESS)
    {
        switch(ret) {
            case SFIP_ARG_ERR:
                ParseError("The following is not allowed: %s.", toks[1]);
                break;

            case SFIP_DUPLICATE:
                ParseMessage("Var '%s' redefined.", toks[0]);
                break;

            case SFIP_CONFLICT:
                ParseError("Negated IP ranges that are more general than "
                        "non-negated ranges are not allowed. Consider "
                        "inverting the logic in %s.", toks[0]);
                break;

            case SFIP_NOT_ANY:
                ParseError("!any is not allowed in %s.", toks[0]);
                break;

            default:
                ParseError("Failed to parse the IP address: %s.", toks[1]);
        }
    }

    mSplitFree(&toks, num_toks);
}
#else
static void ParseIpVar(SnortConfig *sc, SnortPolicy *p, char *args)
{
    ParseError("Unknown rule type: %s.", "ipvar");
}
#endif

static void ParseVar(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Variable\n"););

    toks = mSplit(args, " \t", 0, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseError("Missing argument to %s", toks[0]);
    }

    /* Check command line variables to see if this has already
     * been defined */
    if (cmd_line_var_list != NULL)
    {
        VarNode *tmp = cmd_line_var_list;

        while (tmp != NULL)
        {
           // Already defined this via command line 
            if (strcasecmp(toks[0], tmp->name) == 0)
            {
                mSplitFree(&toks, num_toks);
                return;
            }

            tmp = tmp->next;
        }
    } 

    AddVarToTable(sc, toks[0], toks[1]);
    mSplitFree(&toks, num_toks);
}

static void AddVarToTable(SnortConfig *sc, char *name, char *value)
{
    //TODO: snort.cfg and rules should use PortVar instead ...this allows compatability for now.
    if (strstr(name, "_PORT") || strstr(name, "PORT_"))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"PortVar\n"););
        PortVarDefine(sc, name, value);
    }
    else
    {
        VarDefine(sc, name, value);
    }
}

static void ParseThreshFilter(
    SnortConfig *sc, SnortPolicy *p, char *args, const char* ERR_KEY
) {
    char **toks;
    int num_toks;
    int count_flag = 0;
    int seconds_flag = 0;
    int type_flag = 0;
    int tracking_flag = 0;
    int genid_flag = 0;
    int sigid_flag = 0;
    int i;
    THDX_STRUCT thdx;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Event Filter\n"););

    memset(&thdx, 0, sizeof(THDX_STRUCT));

    toks = mSplit(args, ",", 0, &num_toks, 0);  /* get rule option pairs */

    for (i = 0; i < num_toks; i++)
    {
        char **pairs;
        int num_pairs;

        pairs = mSplit(toks[i], " \t", 0, &num_pairs, 0);  /* get rule option pairs */

        if (num_pairs != 2)
        {
            ParseError(ERR_NOT_PAIRED);
        }

        if (strcasecmp(pairs[0], THRESHOLD_OPT__COUNT) == 0)
        {
            if ( count_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.count = xatol(pairs[1],"event_filter: count");

            if ((thdx.count < 1) && (thdx.count != THD_NO_THRESHOLD))
            {
                ParseError("event_filter: count must be > 0 or %d\n",
                           THD_NO_THRESHOLD);
            }
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__SECONDS) == 0)
        {
            if ( seconds_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.seconds = xatoup(pairs[1],"event_filter: seconds");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__TYPE) == 0)
        {
            if ( type_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (strcasecmp(pairs[1], THRESHOLD_TYPE__LIMIT) == 0)
            {
                thdx.type = THD_TYPE_LIMIT;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TYPE__THRESHOLD) == 0)
            {
                thdx.type = THD_TYPE_THRESHOLD;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TYPE__BOTH) == 0)
            {
                thdx.type = THD_TYPE_BOTH;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__TRACK) == 0)
        {
            if ( tracking_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_SRC) == 0)
            {
                thdx.tracking = THD_TRK_SRC;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_DST) == 0)
            {
                thdx.tracking = THD_TRK_DST;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__GID) == 0)
        {
            if ( genid_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.gen_id = xatou(pairs[1], "event_filter: gen_id");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__SID) == 0)
        {
            if ( sigid_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.sig_id = xatou(pairs[1], "event_filter: sig_id");
        }
        else
        {
            ParseError(ERR_BAD_OPTION);
        }

        mSplitFree(&pairs, num_pairs);
    }

    if ((count_flag + tracking_flag + type_flag + seconds_flag + genid_flag
        + sigid_flag) != 6)
    {
        ParseError(ERR_BAD_ARG_COUNT);
    }

    if (sfthreshold_create(sc->threshold_config, &thdx))
    {
        if (thdx.sig_id == 0)
        {
            ParseError(ERR_CREATE_EX, "only one per gen_id != 0");
        }
        else if (thdx.gen_id == 0)
        {
            ParseError(ERR_CREATE_EX, "gen_id = 0 requires sig_id = 0");
        }
        else
        {
            ParseError(ERR_CREATE_EX, "gen_id, sig_id must be unique");
        }
    }

    mSplitFree(&toks, num_toks);
}

/*
   threshold gen_id #, sig_id #, type limit|threshold|both, \
       track by_src|by_dst, count #, seconds #
*/
static void ParseThreshold(SnortConfig *sc, SnortPolicy *p, char *args)
{
    static int warned = 0;

    if (!warned)
    {
        ParseWarning("threshold (standalone) is deprecated; "
            "use event_filter instead.\n");
        warned = 1;
    }

    ParseThreshFilter(sc, p, args, "standalone threshold");
}

static void ParseEventFilter(SnortConfig *sc, SnortPolicy *p, char *args)
{
    ParseThreshFilter(sc, p, args, "event_filter");
}

/*
   suppress gen_id #, sig_id #, track by_src|by_dst, ip cidr'
*/
static void ParseSuppress(SnortConfig *sc, SnortPolicy *p, char *args)
{
    char **toks;
    int num_toks;
    int tracking_flag = 0;
    int genid_flag = 0;
    int sigid_flag = 0;
    int ip_flag = 0;
    int i;
    THDX_STRUCT thdx;
    const char* ERR_KEY = "suppress";

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Suppress\n"););

    memset(&thdx, 0, sizeof(THDX_STRUCT));

    thdx.type = THD_TYPE_SUPPRESS;
    thdx.priority = THD_PRIORITY_SUPPRESS;
    thdx.tracking = THD_TRK_NONE;

    /* Potential IP list might be present so we can't split on commas
     * Change commas to semi-colons */
    args = FixSeparators(args, ';', "suppress");
    toks = mSplit(args, ";", 0, &num_toks, 0);  /* get rule option pairs */

    for (i = 0; i < num_toks; i++)
    {
        char **pairs;
        int num_pairs;

        pairs = mSplit(toks[i], " \t", 2, &num_pairs, 0);  /* get rule option pairs */

        if (num_pairs != 2)
        {
            ParseError(ERR_NOT_PAIRED);
        }

        if (strcasecmp(pairs[0], THRESHOLD_OPT__TRACK) == 0)
        {
            if ( tracking_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_SRC) == 0)
            {
                thdx.tracking = THD_TRK_SRC;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_DST) == 0)
            {
                thdx.tracking = THD_TRK_DST;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__GID) == 0)
        {
            if ( genid_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.gen_id = xatou(pairs[1], "suppress: gen_id");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__SID) == 0)
        {
            if ( sigid_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.sig_id = xatou(pairs[1], "suppress: sig_id");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__IP) == 0)
        {
            if ( ip_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.ip_address = IpAddrSetParse(pairs[1]);
        }
        else
        {
            ParseError(ERR_BAD_OPTION);
        }

        mSplitFree(&pairs, num_pairs);
    }

    if (((genid_flag + sigid_flag) != 2) ||
         (((tracking_flag + ip_flag) != 0) && ((tracking_flag + ip_flag) != 2)))
    {
        ParseError(ERR_BAD_ARG_COUNT);
    }

    if (sfthreshold_create(sc->threshold_config, &thdx))
    {
        ParseError(ERR_CREATE);
    }

    mSplitFree(&toks, num_toks);
}

/****************************************************************************
 * Function: ParseOtnActivatedBy()
 *
 * Purpose: Set an activation link record
 *
 * Arguments:
 *  OptTreeNode *
 *      The otn for this rule option
 *  RuleType
 *      The rule type of the rule using this option
 *  char *
 *      The arguments to this rule option
 *
 * Returns: None
 *
 ****************************************************************************/
static void ParseOtnActivatedBy(SnortConfig *sc, RuleTreeNode *rtn,
                                OptTreeNode *otn, RuleType rtype, char *args)
{
    ActivateListNode *node;
    long int act_num;
    char *endptr;

    if (args == NULL)
        ParseError("Activates by rule option requires an argument.");

    if (rtype != RULE_TYPE__DYNAMIC)
    {
        ParseError("Can only use 'activated_by' rule option with "
                   "'dynamic' rules.");
    }

    node = (ActivateListNode *)SnortAlloc(sizeof(ActivateListNode));

    if (rtn->activate_list == NULL)
    {
        rtn->activate_list = node;
    }
    else
    {
        ActivateListNode *tmp = rtn->activate_list;

        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = node;
    }

    act_num = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') ||
        (act_num < 0) || (act_num > INT32_MAX))
    {
        ParseError("Invalid argument to 'activates' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    node->activated_by = (int)act_num;
    otn->activated_by = (int)act_num;
}

/****************************************************************************
 * Function: ParseOtnActivates()
 *
 * Purpose: Set an activation link record
 *
 * Arguments:
 *  OptTreeNode *
 *      The otn for this rule option
 *  RuleType
 *      The rule type of the rule using this option
 *  char *
 *      The arguments to this rule option
 *
 * Returns: None
 *
 ****************************************************************************/
static void ParseOtnActivates(SnortConfig *sc, RuleTreeNode *rtn,
                              OptTreeNode *otn, RuleType rtype, char *args)
{
    long int act_num;
    char *endptr;

    if (args == NULL)
        ParseError("Activates rule option requires an argument.");

    if (rtype != RULE_TYPE__ACTIVATE)
    {
        ParseError("Can only use 'activates' rule option with "
                   "'activate' rules.");
    }

    act_num = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') ||
        (act_num < 0) || (act_num > INT32_MAX))
    {
        ParseError("Invalid argument to 'activates' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    otn->activates = (int)act_num;
}

static void ParseOtnClassType(SnortConfig *sc, RuleTreeNode *rtn,
                              OptTreeNode *otn, RuleType rtype, char *args)
{
    ClassType *class_type;

    if (args == NULL)
    {
        ParseWarning("ClassType without an argument!");
        return;
    }

    class_type = ClassTypeLookupByType(sc, args);
    if (class_type == NULL)
        ParseError("Unknown ClassType: %s", args);

    otn->sigInfo.classType = class_type;

    /* Add the class_id to class_id so we can reference it for all rules,
     * whether they have a class_id or not.  */
    otn->sigInfo.class_id = class_type->id;

    if (otn->sigInfo.priority == 0)
        otn->sigInfo.priority = class_type->priority;

    /* XXX deprecated */
    otn->event_data.classification = class_type->id;
    if (otn->event_data.priority == 0)
        otn->event_data.priority = class_type->priority;
}

/****************************************************************************
 * Function: ParseOtnCount()
 *
 * Purpose: set the number of packets to leave dynamic rule enabled for.
 *
 * Arguments:
 *  OptTreeNode *
 *      The otn for this rule option
 *  RuleType
 *      The rule type of the rule using this option
 *  char *
 *      The arguments to this rule option
 *
 * Returns: None
 *
 ***************************************************************************/
static void ParseOtnCount(SnortConfig *sc, RuleTreeNode *rtn,
                          OptTreeNode *otn, RuleType rtype, char *args)
{
    long int count;
    char *endptr;

    if (args == NULL)
        ParseError("Count rule option requires an argument.");

    if (rtype != RULE_TYPE__DYNAMIC)
    {
        ParseError("Can only use 'count' rule option with "
                   "'dynamic' rules.");
    }

    count = SnortStrtol(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') || (count < 0))
    {
        ParseError("Invalid argument to 'count' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    otn->activation_counter = (int)count;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Set activation counter to %d\n",
                            otn->activation_counter););
}

static void ParseOtnDetectionFilter(SnortConfig *sc, RuleTreeNode *rtn,
                                    OptTreeNode *otn, RuleType rtype, char *args)
{
    int count_flag = 0;
    int seconds_flag = 0;
    int tracking_flag = 0;
    char **toks;
    int num_toks;
    int i;
    static THDX_STRUCT thdx;
    const char* ERR_KEY = "detection_filter";

    memset(&thdx, 0, sizeof(THDX_STRUCT));

    toks = mSplit(args, ",", 0, &num_toks, 0);

    /* Parameter Check - enough args ?*/
    if (num_toks != 3)
    {
        ParseError(ERR_PAIR_COUNT, 3);
    }

    for (i = 0; i < num_toks; i++)
    {
        char **pairs;
        int num_pairs;

        pairs = mSplit(toks[i], " \t", 0, &num_pairs, 0);
        if (num_pairs != 2)
        {
            ParseError(ERR_NOT_PAIRED);
        }

        if (strcasecmp(pairs[0], THRESHOLD_OPT__COUNT) == 0)
        {
            if ( count_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.count = xatoup(pairs[1],"detection_filter: count");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__SECONDS) == 0)
        {
            if ( seconds_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.seconds = xatoup(pairs[1],"detection_filter: seconds");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__TRACK) == 0)
        {
            if ( tracking_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_SRC) == 0)
            {
                thdx.tracking = THD_TRK_SRC;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_DST) == 0)
            {
                thdx.tracking = THD_TRK_DST;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else
        {
            ParseError(ERR_BAD_OPTION);
        }

        mSplitFree(&pairs, num_pairs);
    }

    if ((count_flag + tracking_flag + seconds_flag) != 3)
    {
        ParseError(ERR_BAD_ARG_COUNT);
    }

    mSplitFree(&toks, num_toks);

    thdx.type = THD_TYPE_DETECT;

    otn->detection_filter =
        detection_filter_create(sc->detection_filter_config, &thdx);
}

static void ParseOtnGid(SnortConfig *sc, RuleTreeNode *rtn,
                        OptTreeNode *otn, RuleType rtype, char *args)
{
    unsigned long int gid;
    char *endptr;

    if (args == NULL)
        ParseError("Gid rule option requires an argument.");

    gid = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid argument to 'gid' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    otn->sigInfo.generator = (uint32_t)gid;
    otn->event_data.sig_generator = (uint32_t)gid;
}

/****************************************************************************
 * Function: ParseOtnLogTo()
 *
 * Purpose: stuff the special log filename onto the proper rule option
 *
 * Arguments:
 *  OptTreeNode *
 *      The otn for this rule option
 *  RuleType
 *      The rule type of the rule using this option
 *  char *
 *      The arguments to this rule option
 *
 * Returns: None
 *
 ***************************************************************************/
static void ParseOtnLogTo(SnortConfig *sc, RuleTreeNode *rtn,
                          OptTreeNode *otn, RuleType rtype, char *args)
{
    char *sptr;
    char *eptr;

    if (args == NULL)
        ParseError("'logto' requires a file name as an argument.");

    /* grab everything between the starting " and the end one */
    sptr = strchr(args, '"');
    eptr = strrchr(args, '"');

    if ((sptr != NULL) && (eptr != NULL))
    {
        /* increment past the first quote */
        sptr++;

        /* zero out the second one */
        *eptr = 0;
    }
    else
    {
        sptr = args;
    }

    /* alloc up a nice shiny clean buffer */
    otn->logto = SnortStrdup(sptr);
}

/****************************************************************************
 * Function: ParseOtnMessage()
 *
 * Purpose: Stuff the alert message onto the rule
 *
 * Arguments:
 *  OptTreeNode *
 *      The otn for this rule option
 *  RuleType
 *      The rule type of the rule using this option
 *  char *
 *      The arguments to this rule option
 *
 * Returns: None
 *
 ***************************************************************************/
static void ParseOtnMessage(SnortConfig *sc, RuleTreeNode *rtn,
                            OptTreeNode *otn, RuleType rtype, char *args)
{
    size_t i;
    int escaped = 0;
    char msg_buf[2048];  /* Arbitrary length, but should be enough */

    if (args == NULL)
        ParseError("Message rule option requires an argument.");

    if (*args == '"')
    {
        /* Have to have at least quote, char, quote */
        if (strlen(args) < 3)
            ParseError("Empty argument passed to rule option 'msg'.");

        if (args[strlen(args) - 1] != '"')
        {
            ParseError("Unmatch quote in rule option 'msg'.");
        }

        /* Move past first quote and NULL terminate last quote */
        args++;
        args[strlen(args) - 1] = '\0';

        /* If last quote is escaped, fatal error.
         * Make sure the backslash is not escaped */
        if ((args[strlen(args) - 1] == '\\') &&
            (strlen(args) > 1) && (args[strlen(args) - 2] != '\\'))
        {
            ParseError("Unmatch quote in rule option 'msg'.");
        }
    }

    /* Only valid escaped chars are ';', '"' and '\' */
    /* Would be ok except emerging threats rules are escaping other chars */
    for (i = 0; (i < sizeof(msg_buf)) && (*args != '\0');)
    {
        if (escaped)
        {
#if 0
            if ((*args != ';') && (*args != '"') && (*args != '\\'))
            {
                ParseError("Invalid escaped character in 'msg' rule "
                           "option: '%c'.  Valid characters to escape are "
                           "';', '\"' and '\\'.\n", *args);
            }
#endif

            msg_buf[i++] = *args;
            escaped = 0;
        }
        else if (*args == '\\')
        {
            escaped = 1;
        }
        else
        {
            msg_buf[i++] = *args;
        }

        args++;
    }

    if (escaped)
    {
        ParseError("Message in 'msg' rule option has invalid escape character\n");
    }

    if (i == sizeof(msg_buf))
    {
        ParseError("Message in 'msg' rule option too long.  Please limit "
                   "to %d characters.", sizeof(msg_buf));
    }

    msg_buf[i] = '\0';

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "Message: %s\n", msg_buf););

    otn->sigInfo.message = SnortStrdup(msg_buf);
}

/*
 * metadata may be key/value pairs or just keys
 * 
 * metadata: key [=] value, key [=] value, key [=] value, key, key, ... ;
 *
 * This option may be used one or more times, with one or more key/value pairs.
 *
 * updated 8/28/06 - man 
 *
 * keys:
 * 
 * engine
 * rule-flushing
 * rule-type
 * soid
 * service 
 * os
 */
static void ParseOtnMetadata(SnortConfig *sc, RuleTreeNode *rtn,
                             OptTreeNode *otn, RuleType rtype, char *args)
{
    char **metadata_toks;
    int num_metadata_toks;
    int i;

    if (args == NULL)
        ParseError("Metadata rule option requires an argument.");

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES, "metadata: %s\n", args););
    
    metadata_toks = mSplit(args, ",", 100, &num_metadata_toks, 0);
   
    for (i = 0; i < num_metadata_toks; i++)
    {
        char **key_value_toks;
        int num_key_value_toks;
        char *key = NULL;
        char *value = NULL;

        /* Split on space or equals */
        key_value_toks = mSplit(metadata_toks[i], "= ", 2, &num_key_value_toks, 0);
        key = key_value_toks[0];
        if (num_key_value_toks == 2)
            value = key_value_toks[1];

        DEBUG_WRAP(
                   DebugMessage(DEBUG_CONFIGRULES, "metadata: key=%s", key);
                   if (value != NULL)
                       DebugMessage(DEBUG_CONFIGRULES, " value=%s", value);
                   DebugMessage(DEBUG_CONFIGRULES, "\n");
                  );
       
        /* process key/value pairs */
        if (strcasecmp(key, METADATA_KEY__ENGINE) == 0)
        {
            if (value == NULL)
                ParseError("Metadata key '%s' requires a value", key);

            if (strcasecmp(value, METADATA_VALUE__SHARED) == 0)
            {
                otn->sigInfo.shared = 1;
            }
            else
            {
                ParseError("Metadata key '%s', passed an invalid value '%s'.",
                           key, value);
            }
        }
        /* this should follow 'rule-type' since it changes rule_flusing defaults set by rule-type */
        else if (strcasecmp(key, METADATA_KEY__RULE_FLUSHING) == 0)
        {
            if (value == NULL)
                ParseError("Metadata key '%s' requires a value.", key);

            if ((strcasecmp(value, METADATA_VALUE__ENABLED) == 0) ||
                (strcasecmp(value, METADATA_VALUE__ON) == 0))
            {
                otn->sigInfo.rule_flushing = SI_RULE_FLUSHING_ON;
            }
            else if ((strcasecmp(value, METADATA_VALUE__DISABLED) == 0) ||
                     (strcasecmp(value, METADATA_VALUE__OFF) == 0))
            {
                otn->sigInfo.rule_flushing = SI_RULE_FLUSHING_OFF;
            }
            else
            {
                ParseError("Metadata key '%s', passed an invalid value '%s'.",
                           key, value);
            }
        }
        else if (strcasecmp(key, METADATA_KEY__RULE_TYPE) == 0)
        {
            if (value == NULL)
                ParseError("Metadata key '%s' requires a value.", key);

            if (strcasecmp(value, METADATA_VALUE__PREPROC) == 0)
            {
                otn->sigInfo.rule_type = SI_RULE_TYPE_PREPROC;
                otn->sigInfo.rule_flushing = SI_RULE_FLUSHING_OFF;
            }
            else if (strcasecmp(value, METADATA_VALUE__DECODE) == 0)
            {
                otn->sigInfo.rule_type = SI_RULE_TYPE_DECODE;
                otn->sigInfo.rule_flushing = SI_RULE_FLUSHING_OFF;
            }
            else if (strcasecmp(value, METADATA_VALUE__DETECT) == 0)
            {
                otn->sigInfo.rule_type = SI_RULE_TYPE_DETECT;
                otn->sigInfo.rule_flushing = SI_RULE_FLUSHING_ON;
            }
            else
            {
                ParseError("Metadata key '%s', passed an invalid value '%s'.",
                           key, value);
            }
        }
        else if (strcasecmp(key, METADATA_KEY__SOID) == 0)
        {
            char **toks;
            int num_toks;
            char *endptr;
            long int long_val;

            if (value == NULL)
                ParseError("Metadata key '%s' requires a value.", key);

            /* value is a '|' separated pair of gid|sid representing
             * the GID/SID of the original rule.  This is used when
             * the rule is duplicated rule by a user with different
             * IP/port info.
             */
            toks = mSplit(value, "|", 2, &num_toks, 0);
            if (num_toks != 2)
            {
                ParseError("Metadata Key '%s' Invalid Value. Must be a pipe "
                           "(|) separated pair.", key);
            }

            long_val = SnortStrtoul(toks[0], &endptr, 10);
            if ((errno == ERANGE) || (*endptr != '\0') || (long_val > UINT32_MAX))
                ParseError("Bogus gid %s", toks[0]);

            otn->sigInfo.otnKey.gid = (uint32_t)long_val;
            
            long_val = SnortStrtoul(toks[1], &endptr, 10);
            if ((errno == ERANGE) || (*endptr != '\0') || (long_val > UINT32_MAX))
                ParseError("Bogus sid %s", toks[1]);

            otn->sigInfo.otnKey.sid = (uint32_t)long_val;

            mSplitFree(&toks, num_toks);
        }
#ifdef TARGET_BASED
        /* track all of the rules for each service */
        else if (strcasecmp(key, METADATA_KEY__SERVICE) == 0 )
        {
            // metadata: service http, ... ;
            if (value == NULL)
                ParseError("Metadata key '%s' requires a value.", key);

            if (otn->sigInfo.num_services >= sc->max_metadata_services)
            {
                FatalError("%s(%d)=> Too many service's specified for rule.\n",
                    file_name, file_line);
            }
            else
            {
                char *svc_name;
                int svc_count = otn->sigInfo.num_services;

                if (otn->sigInfo.services == NULL)
                {
                    otn->sigInfo.services = SnortAlloc(sizeof(ServiceInfo) * sc->max_metadata_services);
                }

                svc_name = otn->sigInfo.services[svc_count].service = SnortStrdup(value);
                otn->sigInfo.services[svc_count].service_ordinal = FindProtocolReference(svc_name);
                if (otn->sigInfo.services[svc_count].service_ordinal == SFTARGET_UNKNOWN_PROTOCOL)
                {
                    otn->sigInfo.services[svc_count].service_ordinal = AddProtocolReference(svc_name);
                }

                otn->sigInfo.num_services++;
            }
        }
        /* track all of the rules for each os */
        else if (strcasecmp(key, METADATA_KEY__OS) == 0 )
        {
            // metadata: os = Linux:w
            // 
            if (value == NULL)
                ParseError("Metadata key '%s' requires a value.", key);

            otn->sigInfo.os = SnortStrdup(value);
        }
#endif
        else
        {
            /* XXX Why not fatal error? */
            //ParseMessage("Ignoring Metadata : %s = %s", key, value);
        }

        mSplitFree(&key_value_toks, num_key_value_toks);
    }

    mSplitFree(&metadata_toks, num_metadata_toks);
}

static void ParseOtnPriority(SnortConfig *sc, RuleTreeNode *rtn,
                             OptTreeNode *otn, RuleType rtype, char *args)
{
    unsigned long int priority;
    char *endptr;

    if (args == NULL)
        ParseError("Priority rule option requires an argument.");

    priority = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid argument to 'gid' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    otn->sigInfo.priority = (uint32_t)priority;
    /* deprecated */
    otn->event_data.priority = (uint32_t)priority;
}

static void ParseOtnReference(SnortConfig *sc, RuleTreeNode *rtn,
                              OptTreeNode *otn, RuleType rtype, char *args)
{
    char **toks;
    int num_toks;

    if (args == NULL)
        ParseError("Reference rule option requires an argument.");

    /* 2 tokens: system, id */
    toks = mSplit(args, ",", 2, &num_toks, 0);
    if (num_toks != 2)
    {
        ParseWarning("Ignoring invalid Reference spec '%s'.", args);
        return;
    }

    AddReference(sc, &otn->sigInfo.refs, toks[0], toks[1]);

    mSplitFree(&toks, num_toks);
}

static void ParseOtnRevision(SnortConfig *sc, RuleTreeNode *rtn,
                             OptTreeNode *otn, RuleType rtype, char *args)
{
    unsigned long int rev;
    char *endptr;

    if (args == NULL)
        ParseError("Revision rule option requires an argument.");

    rev = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid argument to 'rev' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    otn->sigInfo.rev = (uint32_t)rev;
    /* deprecated */
    otn->event_data.sig_rev = (uint32_t)rev;
}

static void ParseOtnSid(SnortConfig *sc, RuleTreeNode *rtn,
                        OptTreeNode *otn, RuleType rtype, char *args)
{
    unsigned long int sid;
    char *endptr;

    if (args == NULL)
        ParseError("Revision rule option requires an argument.");

    sid = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0'))
    {
        ParseError("Invalid argument to 'sid' rule option: %s.  "
                   "Must be a positive integer.", args);
    }

    otn->sigInfo.id = (uint32_t)sid;
    /* deprecated */
    otn->event_data.sig_id = (uint32_t)sid;
}

static void ParseOtnTag(SnortConfig *sc, RuleTreeNode *rtn,
                        OptTreeNode *otn, RuleType rtype, char *args)
{
    int type = 0;
    int count = 0;
    int metric = 0;
    int packets = 0;
    int seconds = 0;
    int bytes = 0;
    int direction = 0;
    int i;
    char **toks;
    int num_toks;
    uint8_t got_count = 0;

    if (otn->tag != NULL)
        ParseError("Can only use 'tag' rule option once per rule.");

    DEBUG_WRAP(DebugMessage(DEBUG_RULES, "Parsing tag args: %s\n", args););
    toks = mSplit(args, " ,", 0, &num_toks, 0);

    if (num_toks < 3)
        ParseError("Invalid tag arguments: %s", args);

    if (strcasecmp(toks[0], TAG_OPT__SESSION) == 0)
        type = TAG_SESSION;
    else if (strcasecmp(toks[0], TAG_OPT__HOST) == 0)
        type = TAG_HOST;
    else
        ParseError("Invalid tag type: %s", toks[0]);

    for (i = 1; i < num_toks; i++)
    {
        if (!got_count)
        {
            if (isdigit((int)toks[i][0]))
            {
                long int val;
                char *endptr;

                val = SnortStrtol(toks[i], &endptr, 0);
                if ((errno == ERANGE) || (*endptr != '\0') ||
                        (val < 0) || (val > INT32_MAX))
                {
                    ParseError("Invalid argument to 'tag' rule option.  "
                            "Numbers must be between 0 and %d.", INT32_MAX);
                }

                count = (int)val;
                got_count = 1;
            }
            else
            {
                /* Check for src/dst */
                break;
            }
        }
        else
        {
            if (strcasecmp(toks[i], TAG_OPT__SECONDS) == 0)
            {
                if (metric & TAG_METRIC_SECONDS)
                    ParseError("Can only configure seconds metric to tag rule option once");
                if (!count)
                    ParseError("Tag seconds metric must have a positive count");
                metric |= TAG_METRIC_SECONDS;
                seconds = count;
            }
            else if (strcasecmp(toks[i], TAG_OPT__PACKETS) == 0)
            {
                if (metric & (TAG_METRIC_PACKETS|TAG_METRIC_UNLIMITED))
                    ParseError("Can only configure packets metric to tag rule option once");
                if (count)
                    metric |= TAG_METRIC_PACKETS;
                else
                    metric |= TAG_METRIC_UNLIMITED;
                packets = count;
            }
            else if (strcasecmp(toks[i], TAG_OPT__BYTES) == 0)
            {
                if (metric & TAG_METRIC_BYTES)
                    ParseError("Can only configure bytes metric to tag rule option once");
                if (!count)
                    ParseError("Tag bytes metric must have a positive count");
                metric |= TAG_METRIC_BYTES;
                bytes = count;
            }
            else
            {
                ParseError("Invalid tag metric: %s", toks[i]);
            }

            got_count = 0;
        }
    }

    if (!metric || got_count)
        ParseError("Invalid tag rule option: %s", args);

    if ((metric & TAG_METRIC_UNLIMITED) &&
        !(metric & (TAG_METRIC_BYTES|TAG_METRIC_SECONDS)))
    {
        ParseError("Invalid Tag options. 'packets' parameter '0' but "
                   "neither seconds or bytes specified: %s", args);
    }

    if (i < num_toks)
    {
        if (type != TAG_HOST)
            ParseError("Only tag host type can configure direction");

        if (strcasecmp(toks[i], TAG_OPT__SRC) == 0)
            direction = TAG_HOST_SRC;
        else if (strcasecmp(toks[i], TAG_OPT__DST) == 0)
            direction = TAG_HOST_DST;
        else
            ParseError("Invalid 'tag' option: %s.", toks[i]);

        i++;
    }
    else if (type == TAG_HOST)
    {
        ParseError("Tag host type must specify direction");
    }

    /* Shouldn't be any more tokens */
    if (i != num_toks)
        ParseError("Invalid 'tag' option: %s.", args);

    mSplitFree(&toks, num_toks);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Set type: %d  metric: %x count: %d\n", type, 
                            metric, count););

    otn->tag = (TagData *)SnortAlloc(sizeof(TagData));

    otn->tag->tag_type = type;
    otn->tag->tag_metric = metric;
    otn->tag->tag_seconds = seconds;
    otn->tag->tag_bytes = bytes;
    otn->tag->tag_packets = packets;
    otn->tag->tag_direction = direction;
}

/*
Parse Threshold Rule option parameters for each RULE

'threshold: type limit|threshold|both, track by_src|by_dst, count #, seconds #;'
*/
static void ParseOtnThreshold(SnortConfig *sc, RuleTreeNode *rtn,
                              OptTreeNode *otn, RuleType rtype, char *args)
{
    int count_flag = 0;
    int seconds_flag = 0;
    int type_flag = 0;
    int tracking_flag = 0;
    char **toks;
    int num_toks;
    static THDX_STRUCT thdx;
    int i;
    static int warned = 0;
    const char* ERR_KEY = "rule threshold";

    if (args == NULL)
        ParseError("Threshold rule option requires an argument.");

    if (!warned)
    {
        ParseWarning("threshold (in rule) is deprecated; "
            "use detection_filter instead.\n");

        warned = 1;
    }

    memset(&thdx, 0, sizeof(THDX_STRUCT));

    /* Make this lower than standalone threshold command defaults ??? */
    thdx.priority = -1;

    toks = mSplit(args, ",", 0, &num_toks, 0);

    /* Parameter Check - enough args ?*/
    if (num_toks != 4)
    {
        ParseError(ERR_PAIR_COUNT, 4);
    }

    for (i = 0; i < num_toks; i++)
    {
        char **pairs;
        int num_pairs;

        pairs = mSplit(toks[i], " \t", 0, &num_pairs, 0);
        if (num_pairs != 2)
        {
            ParseError(ERR_NOT_PAIRED);
        }

        if (strcasecmp(pairs[0], THRESHOLD_OPT__COUNT) == 0)
        {
            if ( count_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.count = xatoup(pairs[1],"threshold: count");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__SECONDS) == 0)
        {
            if ( seconds_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            thdx.seconds = xatoup(pairs[1],"threshold: seconds");
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__TYPE) == 0)
        {
            if ( type_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (strcasecmp(pairs[1], THRESHOLD_TYPE__LIMIT) == 0)
            {
                thdx.type = THD_TYPE_LIMIT;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TYPE__THRESHOLD) == 0)
            {
                thdx.type = THD_TYPE_THRESHOLD;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TYPE__BOTH) == 0)
            {
                thdx.type = THD_TYPE_BOTH;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else if (strcasecmp(pairs[0], THRESHOLD_OPT__TRACK) == 0)
        {
            if ( tracking_flag++ )
            {
                ParseError(ERR_EXTRA_OPTION);
            }

            if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_SRC) == 0)
            {
                thdx.tracking = THD_TRK_SRC;
            }
            else if (strcasecmp(pairs[1], THRESHOLD_TRACK__BY_DST) == 0)
            {
                thdx.tracking = THD_TRK_DST;
            }
            else
            {
                ParseError(ERR_BAD_VALUE);
            }
        }
        else
        {
            ParseError(ERR_BAD_OPTION);
        }

        mSplitFree(&pairs, num_pairs);
    }

    if ((count_flag + tracking_flag + type_flag + seconds_flag) != 4)
    {
        ParseError(ERR_BAD_ARG_COUNT);
    }

    mSplitFree(&toks, num_toks);

    /* Save this since we still need to add gid and sid */
    thdx_tmp = &thdx;
}

static void CreateDefaultRules(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    CreateRuleType(sc, RULE_LIST_TYPE__ACTIVATION, RULE_TYPE__ACTIVATE, 1, &sc->Activation);
    CreateRuleType(sc, RULE_LIST_TYPE__DYNAMIC, RULE_TYPE__DYNAMIC, 1, &sc->Dynamic);
    CreateRuleType(sc, RULE_LIST_TYPE__PASS, RULE_TYPE__PASS, 0, &sc->Pass); /* changed on Jan 06 */
    CreateRuleType(sc, RULE_LIST_TYPE__DROP, RULE_TYPE__DROP, 1, &sc->Drop);
    CreateRuleType(sc, RULE_LIST_TYPE__SDROP, RULE_TYPE__SDROP, 0, &sc->SDrop);
    CreateRuleType(sc, RULE_LIST_TYPE__REJECT, RULE_TYPE__REJECT, 1, &sc->Reject);
    CreateRuleType(sc, RULE_LIST_TYPE__ALERT, RULE_TYPE__ALERT, 1, &sc->Alert);
    CreateRuleType(sc, RULE_LIST_TYPE__LOG, RULE_TYPE__LOG, 1, &sc->Log);
}

static RuleType GetRuleType(char *arg)
{
    if (arg == NULL)
        return RULE_TYPE__NONE; 

    if (strcasecmp(arg, SNORT_CONF_KEYWORD__ACTIVATE) == 0)
        return RULE_TYPE__ACTIVATE;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__ALERT) == 0)
        return RULE_TYPE__ALERT;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__DROP) == 0)
        return RULE_TYPE__DROP;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__BLOCK) == 0)
        return RULE_TYPE__DROP;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__DYNAMIC) == 0)
        return RULE_TYPE__DYNAMIC;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__LOG) == 0)
        return RULE_TYPE__LOG;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__PASS) == 0)
        return RULE_TYPE__PASS;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__REJECT) == 0)
        return RULE_TYPE__REJECT;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__SDROP) == 0)
        return RULE_TYPE__SDROP;
    else if (strcasecmp(arg, SNORT_CONF_KEYWORD__SBLOCK) == 0)
        return RULE_TYPE__SDROP;

    return RULE_TYPE__NONE;
}

static void FreeRuleTreeNodes(SnortConfig *sc)
{
    RuleTreeNode *rtn;
    OptTreeNode *otn;
    tSfPolicyId policyId;
    SFGHASH_NODE *hashNode;

    if (sc->otn_map == NULL)
        return;

    for (hashNode = sfghash_findfirst(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;

        /* Autogenerated OTNs along with their respective pseudo RTN
         * will get cleaned up when the OTN is free'd */
        if (otn->generated)
            continue;

        for (policyId = 0;
             policyId < otn->proto_node_num;
             policyId++)
        {
            rtn = getRtnFromOtn(otn, policyId);
            DestroyRuleTreeNode(rtn);

            otn->proto_nodes[policyId] = NULL;
        }
    }
}

static void FreeOutputLists(ListHead *list)
{
    if (list->AlertList != NULL)
        FreeOutputList(list->AlertList);

    if (list->LogList != NULL)
        FreeOutputList(list->LogList);
}

void FreeRuleLists(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    FreeRuleTreeNodes(sc);

    FreeOutputLists(&sc->Drop);
    FreeOutputLists(&sc->SDrop);
    FreeOutputLists(&sc->Reject);
    FreeOutputLists(&sc->Alert);
    FreeOutputLists(&sc->Log);
    FreeOutputLists(&sc->Pass);
    FreeOutputLists(&sc->Activation);
    FreeOutputLists(&sc->Dynamic);

    /* Iterate through the user-defined types */
    if (sc->rule_lists != NULL)
    {
        RuleListNode *node = sc->rule_lists;

        while (node != NULL)
        {
            RuleListNode *tmp = node;

            node = node->next;

            if ((tmp->RuleList != &sc->Drop) &&
                (tmp->RuleList != &sc->SDrop) &&
                (tmp->RuleList != &sc->Reject) &&
                (tmp->RuleList != &sc->Alert) &&
                (tmp->RuleList != &sc->Log) &&
                (tmp->RuleList != &sc->Pass) &&
                (tmp->RuleList != &sc->Activation) &&
                (tmp->RuleList != &sc->Dynamic))
            {
                FreeOutputLists(tmp->RuleList);
                free(tmp->RuleList);
            }

            if (tmp->name)
                free(tmp->name);

            free(tmp);
        }

        sc->rule_lists = NULL;
    }
}

static void port_entry_free(port_entry_t *pentry)
{
    if (pentry->src_port != NULL)
    {
        free(pentry->src_port);
        pentry->src_port = NULL;
    }

    if (pentry->dst_port != NULL)
    {
        free(pentry->dst_port);
        pentry->dst_port = NULL;
    }

    if (pentry->protocol != NULL)
    {
        free(pentry->protocol);
        pentry->protocol = NULL;
    }
}

static int port_list_add_entry( port_list_t * plist, port_entry_t * pentry)
{
    if( !plist )
    {
        port_entry_free(pentry);
        return -1;
    }

    if( plist->pl_cnt >= plist->pl_max )
    {
        port_entry_free(pentry);
        return -1;
    }

    SafeMemcpy( &plist->pl_array[plist->pl_cnt], pentry, sizeof(port_entry_t),
                &plist->pl_array[plist->pl_cnt], 
                (char*)(&plist->pl_array[plist->pl_cnt]) + sizeof(port_entry_t));
    plist->pl_cnt++;

    return 0;   
}

#if 0
static port_entry_t * port_list_get( port_list_t * plist, int index)
{
    if( index < plist->pl_max )
    {
        return &plist->pl_array[index];
    }
    return NULL;
}

static void port_list_print( port_list_t * plist)
{
    int i;
    for(i=0;i<plist->pl_cnt;i++)
    {
        LogMessage("rule %d { ", i);
        LogMessage(" gid %u sid %u",plist->pl_array[i].gid,plist->pl_array[i].sid );
        LogMessage(" protocol %s", plist->pl_array[i].protocol);
        LogMessage(" dir %d",plist->pl_array[i].dir);
        LogMessage(" src_port %s dst_port %s ",
                plist->pl_array[i].src_port,
                plist->pl_array[i].dst_port );
        LogMessage(" content %d", 
                plist->pl_array[i].content);
        LogMessage(" uricontent %d", 
                plist->pl_array[i].uricontent);
        LogMessage(" }\n");
    }
}
#endif

static void port_list_free( port_list_t * plist)
{
    int i;
    for(i=0;i<plist->pl_cnt;i++)
    {
        port_entry_free(&plist->pl_array[i]);
    }
    plist->pl_cnt = 0;
}

/* Finish processing/setup Port Tables */
static void finish_portlist_table(FastPatternConfig *fp, char *s, PortTable *pt)
{
    PortTableSortUniqRules(pt);

    if( fpDetectGetDebugPrintRuleGroupsUnCompiled(fp) )
    {
        LogMessage("***\n***Port-Table : %s Ports/Rules-UnCompiled\n",s);
        PortTablePrintInputEx( pt, rule_index_map_print_index );
    }

    PortTableCompile( pt );

    if( fpDetectGetDebugPrintRuleGroupsCompiled(fp) )
    {
        LogMessage("***\n***Port-Table : %s Ports/Rules-Compiled\n",s);
        PortTablePrintCompiledEx( pt, rule_index_map_print_index ); 
        LogMessage("*** End of Compiled Group\n");
    }   
}

void rule_index_map_print_index( int index, char *buf, int bufsize )
{
    if( index < ruleIndexMap->num_rules )
    {
        SnortSnprintfAppend(buf, bufsize, "%u:%u ",
                            ruleIndexMap->map[index].gid,
                            ruleIndexMap->map[index].sid);
    }
}

static rule_port_tables_t * PortTablesNew(void)
{
    rule_port_tables_t *rpt =
        (rule_port_tables_t *)SnortAlloc(sizeof(rule_port_tables_t));

    /* No content rule objects */ 
    rpt->tcp_nocontent = PortObjectNew();
    if (rpt->tcp_nocontent == NULL)
        FatalError("ParseRulesFile nocontent PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->tcp_nocontent);

    rpt->udp_nocontent = PortObjectNew();
    if (rpt->udp_nocontent == NULL)
        FatalError("ParseRulesFile nocontent PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->udp_nocontent);

    rpt->icmp_nocontent = PortObjectNew();
    if (rpt->icmp_nocontent == NULL)
        FatalError("ParseRulesFile nocontent PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->icmp_nocontent);

    rpt->ip_nocontent = PortObjectNew();
    if (rpt->ip_nocontent == NULL)
        FatalError("ParseRulesFile nocontent PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->ip_nocontent);

    /* Create the Any-Any Port Objects for each protocol */
    rpt->tcp_anyany = PortObjectNew();
    if (rpt->tcp_anyany == NULL)
        FatalError("ParseRulesFile tcp any-any PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->tcp_anyany);

    rpt->udp_anyany = PortObjectNew();
    if (rpt->udp_anyany == NULL)
        FatalError("ParseRulesFile udp any-any PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->udp_anyany);

    rpt->icmp_anyany = PortObjectNew();
    if (rpt->icmp_anyany == NULL)
        FatalError("ParseRulesFile icmp any-any PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->icmp_anyany);

    rpt->ip_anyany = PortObjectNew();
    if (rpt->ip_anyany == NULL)
        FatalError("ParseRulesFile ip PortObjectNew() failed\n");
    PortObjectAddPortAny(rpt->ip_anyany);

    /* Create the tcp Rules PortTables */
    rpt->tcp_src = PortTableNew();
    if (rpt->tcp_src == NULL)
        FatalError("ParseRulesFile tcp-src PortTableNew() failed\n");

    rpt->tcp_dst = PortTableNew();
    if (rpt->tcp_dst == NULL)
        FatalError("ParseRulesFile tcp-dst PortTableNew() failed\n");

    /* Create the udp Rules PortTables */
    rpt->udp_src = PortTableNew();
    if (rpt->udp_src == NULL)
        FatalError("ParseRulesFile udp-src PortTableNew() failed\n");

    rpt->udp_dst = PortTableNew();
    if (rpt->udp_dst == NULL)
        FatalError("ParseRulesFile udp-dst PortTableNew() failed\n");

    /* Create the icmp Rules PortTables */
    rpt->icmp_src = PortTableNew();
    if (rpt->icmp_src == NULL)
        FatalError("ParseRulesFile icmp-src PortTableNew() failed\n");

    rpt->icmp_dst = PortTableNew();
    if (rpt->icmp_dst == NULL)
        FatalError("ParseRulesFile icmp-dst PortTableNew() failed\n");

    /* Create the ip Rules PortTables */
    rpt->ip_src = PortTableNew();
    if (rpt->ip_src == NULL)
        FatalError("ParseRulesFile ip-src PortTableNew() failed\n");

    rpt->ip_dst = PortTableNew();
    if (rpt->ip_dst == NULL)
        FatalError("ParseRulesFile ip-dst PortTableNew() failed\n");

    /*
     * someday these could be read from snort.conf, something like...
     * 'config portlist: large-rule-count <val>'
     */
    rpt->tcp_src->pt_lrc = DEFAULT_LARGE_RULE_GROUP; 
    rpt->tcp_dst->pt_lrc = DEFAULT_LARGE_RULE_GROUP; 
    rpt->udp_src->pt_lrc = DEFAULT_LARGE_RULE_GROUP; 
    rpt->udp_dst->pt_lrc = DEFAULT_LARGE_RULE_GROUP; 
    rpt->icmp_src->pt_lrc= DEFAULT_LARGE_RULE_GROUP; 
    rpt->icmp_dst->pt_lrc= DEFAULT_LARGE_RULE_GROUP; 
    rpt->ip_src->pt_lrc  = DEFAULT_LARGE_RULE_GROUP; 
    rpt->ip_dst->pt_lrc  = DEFAULT_LARGE_RULE_GROUP; 

    return rpt;
}

static void PortTablesFinish(rule_port_tables_t *port_tables, FastPatternConfig *fp)
{
    /* TCP-SRC */
    if (fpDetectGetDebugPrintRuleGroupsCompiled(fp))
    {
        LogMessage("*** TCP-Any-Any Port List\n");
        PortObjectPrintEx(port_tables->tcp_anyany,
                          rule_index_map_print_index);
    }

    finish_portlist_table(fp, "tcp src", port_tables->tcp_src);
    finish_portlist_table(fp, "tcp dst", port_tables->tcp_dst);

    /* UDP-SRC */   
    if (fpDetectGetDebugPrintRuleGroupsCompiled(fp))
    {
        LogMessage("*** UDP-Any-Any Port List\n");
        PortObjectPrintEx(port_tables->udp_anyany,
                          rule_index_map_print_index);
    }

    finish_portlist_table(fp, "udp src", port_tables->udp_src);
    finish_portlist_table(fp, "udp dst", port_tables->udp_dst);

    /* ICMP-SRC */   
    if (fpDetectGetDebugPrintRuleGroupsCompiled(fp))
    {
        LogMessage("*** ICMP-Any-Any Port List\n");
        PortObjectPrintEx(port_tables->icmp_anyany,
                          rule_index_map_print_index);
    }

    finish_portlist_table(fp, "icmp src", port_tables->icmp_src);
    finish_portlist_table(fp, "icmp dst", port_tables->icmp_dst);

    /* IP-SRC */   
    if (fpDetectGetDebugPrintRuleGroupsCompiled(fp))
    {
        LogMessage("IP-Any-Any Port List\n");
        PortObjectPrintEx(port_tables->ip_anyany,
                          rule_index_map_print_index);
    }

    finish_portlist_table(fp, "ip src", port_tables->ip_src);
    finish_portlist_table(fp, "ip dst", port_tables->ip_dst);

    RuleListSortUniq(port_tables->tcp_anyany->rule_list);
    RuleListSortUniq(port_tables->udp_anyany->rule_list);
    RuleListSortUniq(port_tables->icmp_anyany->rule_list);
    RuleListSortUniq(port_tables->ip_anyany->rule_list);
    RuleListSortUniq(port_tables->tcp_nocontent->rule_list); 
    RuleListSortUniq(port_tables->udp_nocontent->rule_list); 
    RuleListSortUniq(port_tables->icmp_nocontent->rule_list); 
    RuleListSortUniq(port_tables->ip_nocontent->rule_list); 
}

void VarTablesFree(SnortConfig *sc)
{
    tSfPolicyId i;

    if (sc == NULL)
        return;

    for (i = 0; i < sc->num_policies_allocated; i++)
    {
        SnortPolicy *p = sc->targeted_policies[i];

        if (p == NULL)
            continue;

        if (p->var_table != NULL)
        {
            DeleteVars(p->var_table);
            p->var_table = NULL;
        }

#ifdef SUP_IP6
        if (p->ip_vartable != NULL)
        {
            sfvt_free_table(p->ip_vartable);
            p->ip_vartable = NULL;
        }
#endif

        if (p->portVarTable != NULL)
        {
            PortVarTableFree(p->portVarTable);
            p->portVarTable = NULL;
        }

        if (p->nonamePortVarTable != NULL)
        {
            PortTableFree(p->nonamePortVarTable);
            p->nonamePortVarTable = NULL;
        }
    }
}

void PortTablesFree(rule_port_tables_t *port_tables)
{
    if (port_tables == NULL)
        return;

    if (port_tables->tcp_src)
        PortTableFree(port_tables->tcp_src);
    if (port_tables->tcp_dst)
        PortTableFree(port_tables->tcp_dst);
    if (port_tables->udp_src)
        PortTableFree(port_tables->udp_src);
    if (port_tables->udp_dst)
        PortTableFree(port_tables->udp_dst);
    if (port_tables->icmp_src)
        PortTableFree(port_tables->icmp_src);
    if (port_tables->icmp_dst)
        PortTableFree(port_tables->icmp_dst);
    if (port_tables->ip_src)
        PortTableFree(port_tables->ip_src);
    if (port_tables->ip_dst)
        PortTableFree(port_tables->ip_dst);

    if (port_tables->tcp_anyany)
        PortObjectFree(port_tables->tcp_anyany);
    if (port_tables->udp_anyany)
        PortObjectFree(port_tables->udp_anyany);
    if (port_tables->icmp_anyany)
        PortObjectFree(port_tables->icmp_anyany);
    if (port_tables->ip_anyany)
        PortObjectFree(port_tables->ip_anyany);

    if (port_tables->tcp_nocontent)
        PortObjectFree(port_tables->tcp_nocontent);
    if (port_tables->udp_nocontent)
        PortObjectFree(port_tables->udp_nocontent);
    if (port_tables->icmp_nocontent)
        PortObjectFree(port_tables->icmp_nocontent);
    if (port_tables->ip_nocontent)
        PortObjectFree(port_tables->ip_nocontent);

    free(port_tables);
}

/****************************************************************************
 *
 * Function: CreateRuleType
 *
 * Purpose: Creates a new type of rule and adds it to the end of the rule list
 *
 * Arguments: name = name of this rule type
 *                       mode = the mode for this rule type
 *                   rval = return value for this rule type (for detect events)
 *                       head = list head to use (or NULL to create a new one)
 *
 * Returns: the ListHead for the rule type
 *
 ***************************************************************************/
static ListHead * CreateRuleType(SnortConfig *sc, char *name,
                                 RuleType mode, int rval, ListHead *head)
{
    RuleListNode *node;
    int evalIndex = 0;

    if (sc == NULL)
        return NULL;

    node = (RuleListNode *)SnortAlloc(sizeof(RuleListNode));

    /* If this is the first rule list node, then we need to
     * create a new list. */
    if (sc->rule_lists == NULL)
    {
        sc->rule_lists = node;
    }
    else
    {
        RuleListNode *tmp = sc->rule_lists;
        RuleListNode *last;

        do
        {
            /* We do not allow multiple rules types with the same name. */
            if (strcasecmp(tmp->name, name) == 0)
            {
                free(node);
                return NULL;
            }

            evalIndex++;
            last = tmp;
            tmp = tmp->next;

        } while (tmp != NULL);

        last->next = node;
    }

    /* User defined rule type so we need to create a list head for it */
    if (head == NULL)
    {
        node->RuleList = (ListHead *)SnortAlloc(sizeof(ListHead));
    }
    else
    {
        /* Our default rules already have list heads */
        node->RuleList = head;
    }

    node->RuleList->ruleListNode = node;
    node->mode = mode;
    node->rval = rval;
    node->name = SnortStrdup(name);
    node->evalIndex = evalIndex;

    sc->num_rule_types++;
    
    return node->RuleList;
}

static void OtnInit(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    /* Don't initialize this more than once */
    if ((sc->so_rule_otn_map != NULL) || (sc->otn_map != NULL))
        return;

    /* Init sid-gid -> otn map */
    sc->so_rule_otn_map = SoRuleOtnLookupNew();
    if (sc->so_rule_otn_map == NULL)
         FatalError("ParseRulesFile so_otn_map sfghash_new failed.\n");

    /* Init sid-gid -> otn map */
    sc->otn_map = OtnLookupNew();
    if (sc->otn_map == NULL)
        FatalError("ParseRulesFile otn_map sfghash_new failed.\n");
}

#ifndef SOURCEFIRE
#define IFACE_VARS_MAX 128
typedef struct iface_var
{
    char name[128];
    uint32_t net;
    uint32_t netmask;
} iface_var_t;

/****************************************************************************
 *
 * Function  : DefineAllIfaceVars()
 * Purpose   : Find all up interfaces and define iface_ADDRESS vars for them
 * Arguments : none
 * Returns   : void function
 *
 ****************************************************************************/
static void DefineAllIfaceVars(SnortConfig *sc)
{
    /* Cache retrieved devs so if user is running with dropped privs and
     * does a reload, we can use previous values */
    static int num_vars = 0;
    /* Should be more than enough to cover the number of
     * interfaces on a machine */
    static iface_var_t iface_vars[IFACE_VARS_MAX];

    if (num_vars > 0)
    {
        int i;

        for (i = 0; i < num_vars; i++)
        {
            DefineIfaceVar(sc, iface_vars[i].name,
                    (uint8_t *)&iface_vars[i].net, 
                    (uint8_t *)&iface_vars[i].netmask);
        }
    }
    else
    {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_if_t *alldevs;
        pcap_if_t *dev;
        bpf_u_int32 net, netmask;
#ifdef WIN32
        int i = 1;
#endif

        if (pcap_findalldevs(&alldevs, errbuf) == -1)
            return;

        for (dev = alldevs; dev != NULL; dev = dev->next)
        {
            if (pcap_lookupnet(dev->name, &net, &netmask, errbuf) == 0)
            {
                /* We've hit the maximum variables we can cache */
                if (num_vars >= IFACE_VARS_MAX)
                    break;
#ifdef WIN32
                /* For windows, define var as the index that it will be */
                SnortSnprintf(iface_vars[num_vars].name,
                        sizeof(iface_vars[num_vars].name), "%d", i);
#else
                SnortSnprintf(iface_vars[num_vars].name,
                        sizeof(iface_vars[num_vars].name), "%s", dev->name);
#endif
                DefineIfaceVar(sc, iface_vars[num_vars].name,
                        (uint8_t *)&net, 
                        (uint8_t *)&netmask);

                iface_vars[num_vars].net = net;
                iface_vars[num_vars].netmask = netmask;
                num_vars++;
            }

#ifdef WIN32
            i++;
#endif
        }

        pcap_freealldevs(alldevs);
    }
}

/****************************************************************************
 *
 * Function  : DefineIfaceVar()
 * Purpose   : Assign network address and network mask to IFACE_ADDR_VARNAME
 *             variable.
 * Arguments : interface name (string) netaddress and netmask (4 octets each)
 * Returns   : void function
 *
 ****************************************************************************/
static void DefineIfaceVar(SnortConfig *sc, char *iname, uint8_t *network, uint8_t *netmask)
{
    char valbuf[32];
    char varbuf[BUFSIZ];

    if ((network == NULL) || (*network == 0))
        return;

    SnortSnprintf(varbuf, BUFSIZ, "%s_ADDRESS", iname);

    SnortSnprintf(valbuf, 32, "%d.%d.%d.%d/%d.%d.%d.%d",
            network[0] & 0xff, network[1] & 0xff, network[2] & 0xff, 
            network[3] & 0xff, netmask[0] & 0xff, netmask[1] & 0xff, 
            netmask[2] & 0xff, netmask[3] & 0xff);

    VarDefine(sc, varbuf, valbuf);
}
#endif

void PrintRuleOrder(RuleListNode *rule_lists)
{
    printRuleListOrder(rule_lists);
}

/****************************************************************************
 *
 * Function: OrderRuleLists
 *
 * Purpose: Orders the rule lists into the specefied order.
 *
 * Returns: void function
 *
 ***************************************************************************/
void OrderRuleLists(SnortConfig *sc, char *order)
{
    int i;
    int evalIndex = 0;
    RuleListNode *ordered_list = NULL;
    RuleListNode *prev;
    RuleListNode *node;
    char **toks;
    int num_toks;

    toks = mSplit(order, " \t", 0, &num_toks, 0);

    for( i = 0; i < num_toks; i++ )
    {
        prev = NULL;
        node = sc->rule_lists;

        while (node != NULL)
        {
            if (strcmp(toks[i], node->name) == 0)
            {
                if (prev == NULL)
                    sc->rule_lists = node->next;
                else
                    prev->next = node->next;

                /* Add node to ordered list */
                ordered_list = addNodeToOrderedList(ordered_list, node, evalIndex++);

                break;
            }
            else
            {
                prev = node;
                node = node->next;
            }
        }

        if( node == NULL )
        {
            ParseError("Ruletype \"%s\" does not exist or "
                       "has already been ordered.", toks[i]);
        }
    }

    mSplitFree(&toks, num_toks);

    /* anything left in the rule lists needs to be moved to the ordered lists */
    while (sc->rule_lists != NULL)
    {
        node = sc->rule_lists;
        sc->rule_lists = node->next;
        /* Add node to ordered list */
        ordered_list = addNodeToOrderedList(ordered_list, node, evalIndex++);
    }

    /* set the rulelists to the ordered list */
    sc->rule_lists = ordered_list;
}

static RuleListNode *addNodeToOrderedList(RuleListNode *ordered_list, 
                                          RuleListNode *node, int evalIndex)
{
    RuleListNode *prev;

    prev = ordered_list;
    
    /* set the eval order for this rule set */
    node->evalIndex = evalIndex;
    
    if(!prev)
    {
        ordered_list = node;
    }
    else
    {
        while(prev->next)
            prev = prev->next;
        prev->next = node;
    }

    node->next = NULL;

    return ordered_list;
}

static void printRuleListOrder(RuleListNode * node)
{
    char buf[STD_BUF];
    RuleListNode *first_node = node;

    SnortSnprintf(buf, STD_BUF, "Rule application order: ");

    while( node != NULL )
    {
        SnortSnprintfAppend(buf, STD_BUF, "%s%s",
                            node == first_node ? "" : "->", node->name);

        node = node->next;
    }

    LogMessage("%s\n", buf);
}

NORETURN void ParseError(const char *format, ...)
{
    char buf[STD_BUF+1];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, STD_BUF, format, ap);
    va_end(ap);

    buf[STD_BUF] = '\0';

    if (file_name != NULL)
        FatalError("%s(%d) %s\n", file_name, file_line, buf);
    else
        FatalError("%s\n", buf);
}

void ParseWarning(const char *format, ...)
{
    char buf[STD_BUF+1];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, STD_BUF, format, ap);
    va_end(ap);

    buf[STD_BUF] = '\0';

    if (file_name != NULL)
        LogMessage("WARNING %s(%d) %s\n", file_name, file_line, buf);
    else
        LogMessage("%s\n", buf);
}

void ParseMessage(const char *format, ...)
{
    char buf[STD_BUF+1];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, STD_BUF, format, ap);
    va_end(ap);

    buf[STD_BUF] = '\0';

    if (file_name != NULL)
        LogMessage("%s(%d) %s\n", file_name, file_line, buf);
    else
        LogMessage("%s\n", buf);
}

// RuleTreeNode *getRtnFromOtn() made in line

/**Delete rtn from OTN.
 *
 * @param otn pointer to structure OptTreeNode.
 * @param policyId policy id
 *
 * @return pointer to deleted RTN, NULL otherwise.
 */
RuleTreeNode * deleteRtnFromOtn(
        OptTreeNode *otn, 
        tSfPolicyId policyId
        )
{
    RuleTreeNode *rtn = NULL;

    if (otn->proto_nodes 
            && (otn->proto_node_num >= (policyId+1)))
    {
        rtn = getRtnFromOtn(otn, policyId);
        otn->proto_nodes[policyId] = NULL;

        return rtn;
    }

    return NULL;
}


/**Add RTN to OTN for a particular OTN.
 * @param otn pointer to structure OptTreeNode.
 * @param policyId policy id
 * @param rtn pointer to RuleTreeNode structure
 *
 * @return 0 if successful,
 *         -ve otherwise
 */
int addRtnToOtn(
        OptTreeNode *otn, 
        tSfPolicyId policyId, 
        RuleTreeNode *rtn
        )
{
    if (otn->proto_node_num <= policyId)
    {
        //realloc the list, initialize missing elements to 0 and add policyId
        RuleTreeNode **tmpNodeArray;
        unsigned int numNodes = (policyId + 1);

        tmpNodeArray = SnortAlloc(sizeof(RuleTreeNode *) * numNodes);
        if (!tmpNodeArray)
        {
            return -1;
        } 

        //copy original contents, the remaining elements are already zeroed out by snortAlloc
        if (otn->proto_nodes)
        {
            memcpy(tmpNodeArray, otn->proto_nodes, sizeof(RuleTreeNode *) * otn->proto_node_num);
            free(otn->proto_nodes);
        }

        otn->proto_node_num = numNodes;
        otn->proto_nodes = tmpNodeArray;
    }
    
    //add policyId
    if (otn->proto_nodes[policyId])
    {
        DestroyRuleTreeNode(rtn);
    }

    otn->proto_nodes[policyId] = rtn; 

    return 0; //success
}

// the presence of ip lists exceeds mSplit's one-level parsing
// so we transform rule string into something that mSplit can
// handle by changing ',' to c outside ip lists.
// we also strip the leading keyword.
char* FixSeparators (char* rule, char c, const char* err)
{
    int list = 0;
    char* p = strchr(rule, c); 

    if ( p && err )
    {   
        FatalError("%s(%d) => %s: '%c' not allowed in argument\n",
            file_name, file_line, err, c); 
    }   
    while ( isspace((int)*rule) ) rule++;

    p = rule;

    while ( *p ) { 
        if ( *p == '[' ) list++;
        else if ( *p == ']' ) list--;
        else if ( *p == ',' && !list ) *p = c;
        p++;
    }   
    return rule;
}

void GetNameValue (char* arg, char** nam, char** val, const char* err)
{
    while ( isspace((int)*arg) ) arg++;
    *nam = arg;

    while ( *arg && !isspace((int)*arg) ) arg++;
    if ( *arg ) *arg++ = '\0';
    *val = arg;

    if ( err && !**val )
    {   
        FatalError("%s(%d) => %s: name value pair expected: %s\n",
            file_name, file_line, err, *nam);
    }   
}

static void IntegrityCheckRules(SnortConfig *sc)
{
    OptFpList *ofl_idx = NULL;
    int opt_func_count;
    SFGHASH_NODE *hashNode = NULL;
    OptTreeNode *otn  = NULL;
    tSfPolicyId policyId = 0;
    RuleTreeNode *rtn = NULL;

    for (hashNode = sfghash_findfirst(sc->otn_map);
            hashNode;
            hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;

        for (policyId = 0; 
                policyId < otn->proto_node_num; 
                policyId++)
        {
            rtn = getRtnFromOtn(otn, policyId);

            if (!rtn)
            {
                continue;
            }

            if ((rtn->proto == IPPROTO_TCP) || (rtn->proto == IPPROTO_UDP)
                    || (rtn->proto == IPPROTO_ICMP) || (rtn->proto == ETHERNET_TYPE_IP)) 
            { 
                //do operation
                ofl_idx = otn->opt_func;
                opt_func_count = 0;

                while(ofl_idx != NULL)
                {
                    opt_func_count++;
                    //DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "%p->",ofl_idx->OptTestFunc););
                    ofl_idx = ofl_idx->next;
                }

                if(opt_func_count == 0)
                {
                    FatalError("Zero Length OTN List\n");
                }
                //DEBUG_WRAP(DebugMessage(DEBUG_DETECT,"\n"););

            }
        }
    }

    //DEBUG_WRAP(DebugMessage(DEBUG_DETECT, "OK\n"););
}

/**returns matched header node.
 */
static OptTreeNode * firstHeadNode(SnortConfig *sc, int proto,
                                   RuleType type, tSfPolicyId *policyId)
{
    RuleTreeNode *rtn;
    SFGHASH_NODE *hashNode;

    /* This is a global var */
    currHeadNodePolicy = 0;

    for (hashNode = sfghash_findfirst(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        currHeadNodeOtn = (OptTreeNode *)hashNode->data;
        for (currHeadNodePolicy = 0; 
             currHeadNodePolicy < currHeadNodeOtn->proto_node_num; 
             currHeadNodePolicy++)
        {
            rtn = getRtnFromOtn(currHeadNodeOtn, currHeadNodePolicy);

            if (rtn && (rtn->type == type) 
                    && (rtn->proto == proto))
            { 
                *policyId = currHeadNodePolicy;
                return currHeadNodeOtn;
            }
        }
    }

    return NULL;
}

static OptTreeNode * nextHeadNode(SnortConfig *sc, int proto,
                                  RuleType type, tSfPolicyId *policyId)
{
    SFGHASH_NODE *hashNode;
    RuleTreeNode *rtn;

    if (currHeadNodeOtn)
    {
        for (; 
             currHeadNodePolicy < currHeadNodeOtn->proto_node_num; 
             currHeadNodePolicy++)
        {
            rtn = getRtnFromOtn(currHeadNodeOtn, currHeadNodePolicy);

            if (rtn && (rtn->type == type) 
                    && (rtn->proto == proto))
            { 
                *policyId = currHeadNodePolicy;
                return currHeadNodeOtn;
            }
        }
    }

    for (hashNode = sfghash_findnext(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        currHeadNodeOtn = (OptTreeNode *)hashNode->data;

        for (currHeadNodePolicy = 0; 
             currHeadNodePolicy < currHeadNodeOtn->proto_node_num; 
             currHeadNodePolicy++)
        {
            rtn = getRtnFromOtn(currHeadNodeOtn, currHeadNodePolicy);

            if (rtn && (rtn->type == type) 
                && (rtn->proto == proto))
            { 
                *policyId = currHeadNodePolicy;
                return currHeadNodeOtn;
            }
        }
    }

    return NULL;
}

/**returns matched header node.                                                                                 
*/
static RuleTreeNode * findHeadNode(SnortConfig *sc, RuleTreeNode *testNode,
                                   tSfPolicyId policyId)
{
    RuleTreeNode *rtn;
    OptTreeNode *otn;
    SFGHASH_NODE *hashNode;

    for (hashNode = sfghash_findfirst(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        rtn = getRtnFromOtn(otn, policyId);

        //(protocol, rule_type) match
        if (TestHeader(rtn, testNode))
            return rtn;
    }

    return NULL;
}

void configOptsPrint()
{
    int i;

    printf("Global policy only options\n");
    for (i = 0; config_opts[i].name != NULL; i++)
    {
        if(config_opts[i].default_policy_only )
        {
            printf("\t%s,\t%s\n", config_opts[i].name, config_opts[i].only_once ? "Once":"");
        }
    }

    printf("\nPolicy Specific options\n");
    for (i = 0; config_opts[i].name != NULL; i++)
    {
        if(config_opts[i].default_policy_only == 0)
        {
            printf("\t%s,\t%s\n", config_opts[i].name, config_opts[i].only_once ? "Once":"");
        }
    }
}

SnortPolicy * SnortPolicyNew(void)
{
    SnortPolicy *pPolicy = (SnortPolicy *)SnortAlloc(sizeof(SnortPolicy));

    if (pPolicy)
    {
        // minimum possible (allows all but errors to pass by default)
        pPolicy->min_ttl = 1;

#ifdef NORMALIZER
        pPolicy->new_ttl = 5; 
#endif

        /* Turn on all decoder alerts by default except for oversized alert.
         * Useful for bug reports ... */
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__DEFAULT;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__TCP_EXP_OPT;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__TCP_OBS_OPT;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__TCP_TTCP_OPT;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__TCP_OPT_ANOMALY;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__IP_OPT_ANOMALY;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__IPV6_BAD_FRAG;
        pPolicy->decoder_alert_flags |= DECODE_EVENT_FLAG__IPV6_BSD_ICMP_FRAG;

        pPolicy->decoder_drop_flags |= DECODE_EVENT_FLAG__IPV6_BAD_FRAG;

        pPolicy->checksum_flags = CHECKSUM_FLAG__ALL;
    }

    return pPolicy;
}

void SnortPolicyFree(SnortPolicy *pPolicy)
{
    if (pPolicy == NULL)
        return;

    if (pPolicy->policy_version != NULL)
        free(pPolicy->policy_version);

#ifdef TARGET_BASED
    if (pPolicy->target_based_config.args != NULL)
    {
        free(pPolicy->target_based_config.args);
        if (pPolicy->target_based_config.file_name != NULL)
            free(pPolicy->target_based_config.file_name);
    }
#endif
}
