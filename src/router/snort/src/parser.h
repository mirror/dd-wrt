/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2000-2001 Andrew R. Baker <andrewb@uab.edu>
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
#ifndef __PARSER_H__
#define __PARSER_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sfPolicy.h>

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "sflsq.h"
#include "snort.h"
#include "util.h"


/* Macros *********************************************************************/
/* Rule keywords */
#define SNORT_CONF_KEYWORD__ACTIVATE   "activate"
#define SNORT_CONF_KEYWORD__ALERT      "alert"
#define SNORT_CONF_KEYWORD__DROP       "drop"
#define SNORT_CONF_KEYWORD__BLOCK      "block"
#define SNORT_CONF_KEYWORD__DYNAMIC    "dynamic"
#define SNORT_CONF_KEYWORD__LOG        "log"
#define SNORT_CONF_KEYWORD__PASS       "pass"
#define SNORT_CONF_KEYWORD__REJECT    "reject"
#define SNORT_CONF_KEYWORD__SDROP     "sdrop"
#define SNORT_CONF_KEYWORD__SBLOCK    "sblock"

/* Include keyword */
#define SNORT_CONF_KEYWORD__INCLUDE  "include"

/* Rest of the keywords */
#define SNORT_CONF_KEYWORD__ATTRIBUTE_TABLE      "attribute_table"
#define SNORT_CONF_KEYWORD__CONFIG               "config"
#define SNORT_CONF_KEYWORD__DYNAMIC_DETECTION    "dynamicdetection"
#define SNORT_CONF_KEYWORD__DYNAMIC_ENGINE       "dynamicengine"
#define SNORT_CONF_KEYWORD__DYNAMIC_PREPROC      "dynamicpreprocessor"
#define SNORT_CONF_KEYWORD__DYNAMIC_OUTPUT       "dynamicoutput"
#ifdef SIDE_CHANNEL
# define SNORT_CONF_KEYWORD__DYNAMIC_SIDE_CHAN  "dynamicsidechannel"
#endif
#define SNORT_CONF_KEYWORD__EVENT_FILTER         "event_filter"
# define SNORT_CONF_KEYWORD__IPVAR               "ipvar"
#define SNORT_CONF_KEYWORD__OUTPUT               "output"
#define SNORT_CONF_KEYWORD__PORTVAR              "portvar"
#define SNORT_CONF_KEYWORD__PREPROCESSOR         "preprocessor"
#define SNORT_CONF_KEYWORD__RATE_FILTER          "rate_filter"
#define SNORT_CONF_KEYWORD__RULE_STATE           "rule_state"
#define SNORT_CONF_KEYWORD__RULE_TYPE            "ruletype"
#ifdef SIDE_CHANNEL
# define SNORT_CONF_KEYWORD__SIDE_CHANNEL         "sidechannel"
#endif
#define SNORT_CONF_KEYWORD__SUPPRESS             "suppress"
#define SNORT_CONF_KEYWORD__THRESHOLD            "threshold"
#define SNORT_CONF_KEYWORD__VAR                  "var"
#define SNORT_CONF_KEYWORD__VERSION              "version"
#define SNORT_CONF_KEYWORD__FILE                 "file"

/* Config options */
#define CONFIG_OPT__ALERT_FILE                      "alertfile"
#define CONFIG_OPT__ALERT_WITH_IFACE_NAME           "alert_with_interface_name"
#define CONFIG_OPT__AUTOGEN_PREPROC_DECODER_RULES   "autogenerate_preprocessor_decoder_rules"
#define CONFIG_OPT__ASN1                            "asn1"
#define CONFIG_OPT__BINDING                         "binding"
#define CONFIG_OPT__BPF_FILE                        "bpf_file"
#define CONFIG_OPT__CHECKSUM_DROP                   "checksum_drop"
#define CONFIG_OPT__CHECKSUM_MODE                   "checksum_mode"
#define CONFIG_OPT__CHROOT_DIR                      "chroot"
#define CONFIG_OPT__CLASSIFICATION                  "classification"
#define CONFIG_OPT__DAEMON                          "daemon"
#define CONFIG_OPT__DECODE_DATA_LINK                "decode_data_link"
#define CONFIG_OPT__DECODE_ESP                      "decode_esp"
#define CONFIG_OPT__DEFAULT_RULE_STATE              "default_rule_state"
#define CONFIG_OPT__DETECTION                       "detection"
#define CONFIG_OPT__DETECTION_FILTER                "detection_filter"
#define CONFIG_OPT__PROTECTED_CONTENT               "protected_content"
#ifdef INLINE_FAILOPEN
# define CONFIG_OPT__DISABLE_INLINE_FAILOPEN         "disable_inline_init_failopen"
#endif
#define CONFIG_OPT__DISABLE_DECODE_ALERTS           "disable_decode_alerts"
#define CONFIG_OPT__DISABLE_DECODE_DROPS            "disable_decode_drops"
#define CONFIG_OPT__DISABLE_IP_OPT_ALERTS           "disable_ipopt_alerts"
#define CONFIG_OPT__DISABLE_IP_OPT_DROPS            "disable_ipopt_drops"
#define CONFIG_OPT__DISABLE_TCP_OPT_ALERTS          "disable_tcpopt_alerts"
#define CONFIG_OPT__DISABLE_TCP_OPT_DROPS           "disable_tcpopt_drops"
#define CONFIG_OPT__DISABLE_TCP_OPT_EXP_ALERTS      "disable_tcpopt_experimental_alerts"
#define CONFIG_OPT__DISABLE_TCP_OPT_EXP_DROPS       "disable_tcpopt_experimental_drops"
#define CONFIG_OPT__DISABLE_TCP_OPT_OBS_ALERTS      "disable_tcpopt_obsolete_alerts"
#define CONFIG_OPT__DISABLE_TCP_OPT_OBS_DROPS       "disable_tcpopt_obsolete_drops"
#define CONFIG_OPT__DISABLE_TTCP_ALERTS             "disable_ttcp_alerts"
#define CONFIG_OPT__DISABLE_TCP_OPT_TTCP_ALERTS     "disable_tcpopt_ttcp_alerts"
#define CONFIG_OPT__DISABLE_TTCP_DROPS              "disable_ttcp_drops"
#define CONFIG_OPT__DUMP_CHARS_ONLY                 "dump_chars_only"
#define CONFIG_OPT__DUMP_PAYLOAD                    "dump_payload"
#define CONFIG_OPT__DUMP_PAYLOAD_VERBOSE            "dump_payload_verbose"
#define CONFIG_OPT__ENABLE_DECODE_DROPS             "enable_decode_drops"
#define CONFIG_OPT__ENABLE_DECODE_OVERSIZED_ALERTS  "enable_decode_oversized_alerts"
#define CONFIG_OPT__ENABLE_DECODE_OVERSIZED_DROPS   "enable_decode_oversized_drops"
#define CONFIG_OPT__ENABLE_DEEP_TEREDO_INSPECTION   "enable_deep_teredo_inspection"
#define CONFIG_OPT__ENABLE_GTP_DECODING             "enable_gtp"
#define CONFIG_OPT__ENABLE_IP_OPT_DROPS             "enable_ipopt_drops"
#ifdef MPLS
# define CONFIG_OPT__ENABLE_MPLS_MULTICAST          "enable_mpls_multicast"
# define CONFIG_OPT__ENABLE_MPLS_OVERLAPPING_IP     "enable_mpls_overlapping_ip"
#endif  /* MPLS */
#define CONFIG_OPT__ENABLE_TCP_OPT_DROPS            "enable_tcpopt_drops"
#define CONFIG_OPT__ENABLE_TCP_OPT_EXP_DROPS        "enable_tcpopt_experimental_drops"
#define CONFIG_OPT__ENABLE_TCP_OPT_OBS_DROPS        "enable_tcpopt_obsolete_drops"
#define CONFIG_OPT__ENABLE_TTCP_DROPS               "enable_ttcp_drops"
#define CONFIG_OPT__ENABLE_TCP_OPT_TTCP_DROPS       "enable_tcpopt_ttcp_drops"
#define CONFIG_OPT__EVENT_FILTER                    "event_filter"
#define CONFIG_OPT__EVENT_QUEUE                     "event_queue"
#define CONFIG_OPT__EVENT_TRACE                     "event_trace"
# define CONFIG_OPT__REACT                          "react"
#ifdef ENABLE_RESPONSE3
# define CONFIG_OPT__FLEXRESP2_INTERFACE            "flexresp2_interface"
# define CONFIG_OPT__FLEXRESP2_ATTEMPTS             "flexresp2_attempts"
# define CONFIG_OPT__FLEXRESP2_MEMCAP               "flexresp2_memcap"
# define CONFIG_OPT__FLEXRESP2_ROWS                 "flexresp2_rows"
#endif // ENABLE_RESPONSE3
#ifdef ACTIVE_RESPONSE
# define CONFIG_OPT__RESPONSE                       "response"
#endif
#define CONFIG_OPT__FLOWBITS_SIZE                   "flowbits_size"
#define CONFIG_OPT__IGNORE_PORTS                    "ignore_ports"
#define CONFIG_OPT__ALERT_VLAN                      "include_vlan_in_alerts"
#define CONFIG_OPT__INTERFACE                       "interface"
#define CONFIG_OPT__IPV6_FRAG                       "ipv6_frag"
#define CONFIG_OPT__LAYER2RESETS                    "layer2resets"
#define CONFIG_OPT__LOG_DIR                         "logdir"
#define CONFIG_OPT__DAQ_TYPE                        "daq"
#define CONFIG_OPT__DAQ_MODE                        "daq_mode"
#define CONFIG_OPT__DAQ_VAR                         "daq_var"
#define CONFIG_OPT__DAQ_DIR                         "daq_dir"
#define CONFIG_OPT__DIRTY_PIG                       "dirty_pig"
#ifdef TARGET_BASED
# define CONFIG_OPT__MAX_ATTRIBUTE_HOSTS            "max_attribute_hosts"
# define CONFIG_OPT__MAX_ATTRIBUTE_SERVICES_PER_HOST "max_attribute_services_per_host"
# define CONFIG_OPT__MAX_METADATA_SERVICES          "max_metadata_services"
#define CONFIG_OPT__DISABLE_ATTRIBUTE_RELOAD        "disable-attribute-reload-thread"
#endif  /* TARGET_BASED */
#ifdef MPLS
# define CONFIG_OPT__MAX_MPLS_LABELCHAIN_LEN        "max_mpls_labelchain_len"
# define CONFIG_OPT__MPLS_PAYLOAD_TYPE              "mpls_payload_type"
#endif  /* MPLS */
#define CONFIG_OPT__MIN_TTL                         "min_ttl"
#ifdef NORMALIZER
#define CONFIG_OPT__NEW_TTL                         "new_ttl"
#endif
#define CONFIG_OPT__NO_LOG                          "nolog"
#define CONFIG_OPT__NO_PCRE                         "nopcre"
#define CONFIG_OPT__NO_PROMISCUOUS                  "no_promisc"
#define CONFIG_OPT__OBFUSCATE                       "obfuscate"
#define CONFIG_OPT__ORDER                           "order"
#define CONFIG_OPT__PAF_MAX                         "paf_max"
#define CONFIG_OPT__PCRE_MATCH_LIMIT                "pcre_match_limit"
#define CONFIG_OPT__PCRE_MATCH_LIMIT_RECURSION      "pcre_match_limit_recursion"
#define CONFIG_OPT__PKT_COUNT                       "pkt_count"
#define CONFIG_OPT__PKT_SNAPLEN                     "snaplen"
#define CONFIG_OPT__PID_PATH                        "pidpath"
#define CONFIG_OPT__POLICY                          "policy_id"
#define CONFIG_OPT__IPS_POLICY_MODE                 "policy_mode"
#define CONFIG_OPT__NAP_POLICY_MODE                 "na_policy_mode"
#define CONFIG_OPT__POLICY_VERSION                  "policy_version"
#ifdef PPM_MGR
# define CONFIG_OPT__PPM                            "ppm"
#endif
#ifdef PERF_PROFILING
# define CONFIG_OPT__PROFILE_PREPROCS               "profile_preprocs"
# define CONFIG_OPT__PROFILE_RULES                  "profile_rules"
#endif  /* PERF_PROFILING */
#define CONFIG_OPT__QUIET                           "quiet"
#define CONFIG_OPT__RATE_FILTER                     "rate_filter"
#define CONFIG_OPT__REFERENCE                       "reference"
#define CONFIG_OPT__REFERENCE_NET                   "reference_net"
#define CONFIG_OPT__SET_GID                         "set_gid"
#define CONFIG_OPT__SET_UID                         "set_uid"
#define CONFIG_OPT__SHOW_YEAR                       "show_year"
#define CONFIG_OPT__SO_RULE_MEMCAP                  "so_rule_memcap"
#define CONFIG_OPT__STATEFUL                        "stateful"
#define CONFIG_OPT__TAGGED_PACKET_LIMIT             "tagged_packet_limit"
#define CONFIG_OPT__THRESHOLD                       "threshold"
#define CONFIG_OPT__UMASK                           "umask"
#define CONFIG_OPT__UTC                             "utc"
#define CONFIG_OPT__VERBOSE                         "verbose"
#define CONFIG_OPT__VLAN_AGNOSTIC                   "vlan_agnostic"
#define CONFIG_OPT__ADDRESSSPACE_AGNOSTIC           "addressspace_agnostic"
#define CONFIG_OPT__LOG_IPV6_EXTRA                  "log_ipv6_extra_data"
#define CONFIG_OPT__DUMP_DYNAMIC_RULES_PATH         "dump-dynamic-rules-path"
#define CONFIG_OPT__CONTROL_SOCKET_DIR              "cs_dir"
#define CONFIG_OPT__FILE                            "file"
#define CONFIG_OPT__TUNNEL_BYPASS                   "tunnel_verdicts"
#ifdef SIDE_CHANNEL
# define CONFIG_OPT__SIDE_CHANNEL                   "sidechannel"
#endif
#define CONFIG_OPT__MAX_IP6_EXTENSIONS              "max_ip6_extensions"
#define CONFIG_OPT__DISABLE_REPLACE                 "disable_replace"
#ifdef DUMP_BUFFER
#define CONFIG_OPT__BUFFER_DUMP                     "buffer_dump"
#define CONFIG_OPT__BUFFER_DUMP_ALERT               "buffer_dump_alert"
#endif
/* exported values */
extern char *file_name;
extern int file_line;


/* rule setup funcs */
SnortConfig * ParseSnortConf(void);
void ParseRules(SnortConfig *);
IpsPortFilter** ParseIpsPortList (SnortConfig*, IpProto);

void ParseOutput(SnortConfig *, SnortPolicy *, char *);
void OrderRuleLists(SnortConfig *, char *);
void PrintRuleOrder(RuleListNode *);

char * VarGet(SnortConfig *, char *);
char * ProcessFileOption(SnortConfig *, const char *);
void SetRuleStates(SnortConfig *);
int GetPcaps(SF_LIST *, SF_QUEUE *);

void ParserCleanup(void);
void FreeRuleLists(SnortConfig *);
void VarTablesFree(SnortConfig *);
void PortTablesFree(rule_port_tables_t *);
int CompareIPNodes(IpAddrNode *, IpAddrNode *);

void ResolveOutputPlugins(SnortConfig *, SnortConfig *);
void ConfigureOutputPlugins(SnortConfig *);
void ConfigurePreprocessors(SnortConfig *, int);
void ConfigureSideChannelModules(SnortConfig *);

NORETURN void ParseError(const char *, ...);
void ParseWarning(const char *, ...);
void ParseMessage(const char *, ...);

void ConfigAlertBeforePass(SnortConfig *, char *);
void ConfigAlertFile(SnortConfig *, char *);
void ConfigAlertWithInterfaceName(SnortConfig *, char *);
void ConfigAsn1(SnortConfig *, char *);
void ConfigAutogenPreprocDecoderRules(SnortConfig *, char *);
void ConfigBinding(SnortConfig *, char *);
void ConfigBpfFile(SnortConfig *, char *);
void ConfigChecksumDrop(SnortConfig *, char *);
void ConfigChecksumMode(SnortConfig *, char *);
void ConfigChrootDir(SnortConfig *, char *);
void ConfigClassification(SnortConfig *, char *);
void ConfigCreatePidFile(SnortConfig *, char *);
void ConfigDaemon(SnortConfig *, char *);
void ConfigDecodeDataLink(SnortConfig *, char *);
void ConfigDefaultRuleState(SnortConfig *, char *);
void ConfigDetection(SnortConfig *, char *);
void ConfigDetectionFilter(SnortConfig *, char *);
void ConfigDisableDecodeAlerts(SnortConfig *, char *);
void ConfigDisableDecodeDrops(SnortConfig *, char *);
#ifdef INLINE_FAILOPEN
void ConfigDisableInlineFailopen(SnortConfig *, char *);
#endif
void ConfigDisableIpOptAlerts(SnortConfig *, char *);
void ConfigDisableIpOptDrops(SnortConfig *, char *);
void ConfigDisableTcpOptAlerts(SnortConfig *, char *);
void ConfigDisableTcpOptDrops(SnortConfig *, char *);
void ConfigDisableTcpOptExperimentalAlerts(SnortConfig *, char *);
void ConfigDisableTcpOptExperimentalDrops(SnortConfig *, char *);
void ConfigDisableTcpOptObsoleteAlerts(SnortConfig *, char *);
void ConfigDisableTcpOptObsoleteDrops(SnortConfig *, char *);
void ConfigDisableTTcpAlerts(SnortConfig *, char *);
void ConfigDisableTTcpDrops(SnortConfig *, char *);
void ConfigDumpCharsOnly(SnortConfig *, char *);
void ConfigDumpPayload(SnortConfig *, char *);
void ConfigDumpPayloadVerbose(SnortConfig *, char *);
void ConfigEnableDecodeDrops(SnortConfig *, char *);
void ConfigEnableDecodeOversizedAlerts(SnortConfig *, char *);
void ConfigEnableDecodeOversizedDrops(SnortConfig *, char *);
void ConfigEnableDeepTeredoInspection(SnortConfig *sc, char *args);
void ConfigEnableGTPDecoding(SnortConfig *sc, char *args);
void ConfigEnableEspDecoding(SnortConfig *sc, char *args);
void ConfigEnableIpOptDrops(SnortConfig *, char *);
#ifdef MPLS
void ConfigEnableMplsMulticast(SnortConfig *, char *);
void ConfigEnableMplsOverlappingIp(SnortConfig *, char *);
#endif
void ConfigEnableTcpOptDrops(SnortConfig *, char *);
void ConfigEnableTcpOptExperimentalDrops(SnortConfig *, char *);
void ConfigEnableTcpOptObsoleteDrops(SnortConfig *, char *);
void ConfigEnableTTcpDrops(SnortConfig *, char *);
void ConfigEventFilter(SnortConfig *, char *);
void ConfigEventQueue(SnortConfig *, char *);
void ConfigEventTrace(SnortConfig *, char *);
#ifdef ENABLE_RESPONSE3
void ConfigFlexresp2Interface(SnortConfig *, char *);
void ConfigFlexresp2Attempts(SnortConfig *, char *);
void ConfigFlexresp2Memcap(SnortConfig *, char *);
void ConfigFlexresp2Rows(SnortConfig *, char *);
#endif
#ifdef ACTIVE_RESPONSE
void ConfigResponse(SnortConfig*, char*);
#endif
void ConfigReact(SnortConfig*, char*);
void ConfigFlowbitsSize(SnortConfig *, char *);
void ConfigIgnorePorts(SnortConfig *, char *);
void ConfigIncludeVlanInAlert(SnortConfig *, char *);
void ConfigInterface(SnortConfig *, char *);
void ConfigIpv6Frag(SnortConfig *, char *);
void ConfigLayer2Resets(SnortConfig *, char *);
void ConfigLogDir(SnortConfig *, char *);
void ConfigDaqType(SnortConfig *, char *);
void ConfigDaqMode(SnortConfig *, char *);
void ConfigDaqVar(SnortConfig *, char *);
void ConfigDaqDir(SnortConfig *, char *);
void ConfigDirtyPig(SnortConfig *, char *);
#ifdef TARGET_BASED
void ConfigMaxAttributeHosts(SnortConfig *, char *);
void ConfigMaxAttributeServicesPerHost(SnortConfig *, char *);
void ConfigMaxMetadataServices(SnortConfig *, char *);
void ConfigDisableAttributeReload(SnortConfig *, char *);
#endif
#ifdef MPLS
void ConfigMaxMplsLabelChain(SnortConfig *, char *);
void ConfigMplsPayloadType(SnortConfig *, char *);
#endif
void ConfigMinTTL(SnortConfig *, char *);
#ifdef NORMALIZER
void ConfigNewTTL(SnortConfig *, char *);
#endif
void ConfigNoLog(SnortConfig *, char *);
void ConfigNoLoggingTimestamps(SnortConfig *, char *);
void ConfigNoPcre(SnortConfig *, char *);
void ConfigNoPromiscuous(SnortConfig *, char *);
void ConfigObfuscate(SnortConfig *, char *);
void ConfigObfuscationMask(SnortConfig *, char *);
void ConfigPafMax(SnortConfig *, char *);
void ConfigRateFilter(SnortConfig *, char *);
void ConfigRuleListOrder(SnortConfig *, char *);
void ConfigPacketCount(SnortConfig *, char *);
void ConfigPacketSnaplen(SnortConfig *, char *);
void ConfigPcreMatchLimit(SnortConfig *, char *);
void ConfigPcreMatchLimitRecursion(SnortConfig *, char *);
void ConfigPerfFile(SnortConfig *sc, char *);
void ConfigDumpPeriodicMemStatsFile(SnortConfig *, char *);
void ConfigPidPath(SnortConfig *, char *);
void ConfigPolicy(SnortConfig *, char *);
void ConfigIpsPolicyMode(SnortConfig *, char *);
void ConfigNapPolicyMode(SnortConfig *, char *);
void ConfigPolicyVersion(SnortConfig *, char *);
void ConfigProtectedContent(SnortConfig *, char *);
#ifdef PPM_MGR
void ConfigPPM(SnortConfig *, char *);
#endif
void ConfigProcessAllEvents(SnortConfig *, char *);
#ifdef PERF_PROFILING
void ConfigProfilePreprocs(SnortConfig *, char *);
void ConfigProfileRules(SnortConfig *, char *);
#endif
void ConfigQuiet(SnortConfig *, char *);
void ConfigReadPcapFile(SnortConfig *, char *);
void ConfigReference(SnortConfig *, char *);
void ConfigReferenceNet(SnortConfig *, char *);
void ConfigSetGid(SnortConfig *, char *);
void ConfigSetUid(SnortConfig *, char *);
void ConfigShowYear(SnortConfig *, char *);
void ConfigSoRuleMemcap(SnortConfig *, char *);
void ConfigStateful(SnortConfig *, char *);
void ConfigTaggedPacketLimit(SnortConfig *, char *);
void ConfigThreshold(SnortConfig *, char *);
#ifdef TIMESTATS
void ConfigTimestatsInterval(SnortConfig *, char *);
#endif
void ConfigTreatDropAsAlert(SnortConfig *, char *);
void ConfigTreatDropAsIgnore(SnortConfig *, char *);
void ConfigUmask(SnortConfig *, char *);
void ConfigUtc(SnortConfig *, char *);
void ConfigVerbose(SnortConfig *, char *);
void ConfigVlanAgnostic(SnortConfig *, char *);
void ConfigAddressSpaceAgnostic(SnortConfig *, char *);
void ConfigLogIPv6Extra(SnortConfig *, char *);
void ConfigDumpDynamicRulesPath(SnortConfig *, char *);
void ConfigControlSocketDirectory(SnortConfig *, char *);
void ConfigFile(SnortConfig *, char *);
void ConfigTunnelVerdicts(SnortConfig*, char*);
void ConfigMaxIP6Extensions(SnortConfig *, char*);
void ConfigDisableReplace(SnortConfig *, char*);
#ifdef DUMP_BUFFER
void ConfigBufferDump(SnortConfig *, char *);
#endif

int addRtnToOtn(
        SnortConfig *sc,
        OptTreeNode *otn,
        tSfPolicyId policyId,
        RuleTreeNode *rtn
        );

RuleTreeNode* deleteRtnFromOtn(
        SnortConfig *sc,
        OptTreeNode *otn,
        tSfPolicyId policyId
        );

// use this so mSplit doesn't split IP lists (try c = ';')
char* FixSeparators (char* rule, char c, const char* err);

// use this as an alternative to mSplit when you just want name, value
void GetNameValue (char* arg, char** nam, char** val, const char* err);

/*Get RTN for a given OTN and policyId.
 *
 * @param otn pointer to structure OptTreeNode.
 * @param policyId policy id
 *
 * @return pointer to deleted RTN, NULL otherwise.
 */
static inline RuleTreeNode *getRtnFromOtn(OptTreeNode *otn, tSfPolicyId policyId)
{
    if (otn && otn->proto_nodes && (otn->proto_node_num > (unsigned)policyId))
    {
        return otn->proto_nodes[policyId];
    }

    return NULL;
}

/**Get rtn from otn for the current policy.
 */
static inline RuleTreeNode *getParserRtnFromOtn(SnortConfig *sc, OptTreeNode *otn)
{
    return getRtnFromOtn(otn, getParserPolicy(sc));
}

static inline RuleTreeNode *getRuntimeRtnFromOtn(OptTreeNode *otn)
{
    return getRtnFromOtn(otn, getIpsRuntimePolicy());
}

SnortPolicy * SnortPolicyNew(void);
void SnortPolicyFree(SnortPolicy *pPolicy);

#endif /* __PARSER_H__ */

