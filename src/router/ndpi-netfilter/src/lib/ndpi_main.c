/*
 * ndpi_main.c
 *
 * Copyright (C) 2011-25 - ntop.org
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __KERNEL__
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#else
  #include <asm/byteorder.h>
  #include <linux/kernel.h>
#endif

#ifdef __APPLE__
#include <netinet/ip.h>
#endif

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_UNKNOWN

#include "ndpi_config.h"
#include "ndpi_api.h"
#include "ndpi_private.h"
#include "ahocorasick.h"
#include "libcache.h"

#ifdef __KERNEL__
  #include "ndpi_define.h"
  #include "ndpi_kernel_compat.c"
  #ifdef HAVE_HYPERSCAN
    #error HYPERSCAN
  #endif
  #include <gcrypt_light.h>
  #define HAVE_LIBGCRYPT 1
  #undef MATCH_DEBUG
#else
#include "gcrypt_light.h"
#include <time.h>

#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#include <netdb.h>
#else
#include "third_party/include/windows/dirent.h"
#endif
#endif

#ifndef TH_FIN
#define TH_FIN        0x01
#define TH_SYN        0x02
#define TH_RST        0x04
#define TH_PUSH       0x08
#define TH_ACK        0x10
#define TH_URG        0x20
#define TH_ECE        0x40
#define TH_CWR        0x80
#endif

#if defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
#include <sys/endian.h>
#endif

#include "ndpi_content_match.c.inc"
#include "ndpi_dga_match.c.inc"
#include "inc_generated/ndpi_azure_match.c.inc"
#include "inc_generated/ndpi_tor_match.c.inc"
#include "inc_generated/ndpi_tor_exit_nodes_match.c.inc"
#include "inc_generated/ndpi_whatsapp_match.c.inc"
#include "inc_generated/ndpi_amazon_aws_match.c.inc"
#include "inc_generated/ndpi_ethereum_match.c.inc"
#include "inc_generated/ndpi_zoom_match.c.inc"
#include "inc_generated/ndpi_cachefly_match.c.inc"
#include "inc_generated/ndpi_cloudflare_match.c.inc"
#include "inc_generated/ndpi_digitalocean_match.c.inc"
#include "inc_generated/ndpi_ms_office365_match.c.inc"
#include "inc_generated/ndpi_ms_onedrive_match.c.inc"
#include "inc_generated/ndpi_ms_outlook_match.c.inc"
#include "inc_generated/ndpi_msteams_match.c.inc"
#include "inc_generated/ndpi_google_match.c.inc"
#include "inc_generated/ndpi_google_cloud_match.c.inc"
#include "inc_generated/ndpi_crawlers_match.c.inc"
#include "inc_generated/ndpi_icloud_private_relay_match.c.inc"
#include "inc_generated/ndpi_mullvad_match.c.inc"
#include "inc_generated/ndpi_nordvpn_match.c.inc"
#include "inc_generated/ndpi_surfshark_match.c.inc"
#include "inc_generated/ndpi_asn_telegram.c.inc"
#include "inc_generated/ndpi_asn_apple.c.inc"
#include "inc_generated/ndpi_asn_twitter.c.inc"
#include "inc_generated/ndpi_asn_netflix.c.inc"
#include "inc_generated/ndpi_asn_webex.c.inc"
#include "inc_generated/ndpi_asn_teamviewer.c.inc"
#include "inc_generated/ndpi_asn_facebook.c.inc"
#include "inc_generated/ndpi_asn_tencent.c.inc"
#include "inc_generated/ndpi_asn_opendns.c.inc"
#include "inc_generated/ndpi_asn_dropbox.c.inc"
#include "inc_generated/ndpi_asn_blizzard.c.inc"
#include "inc_generated/ndpi_asn_canonical.c.inc"
#include "inc_generated/ndpi_asn_twitch.c.inc"
#include "inc_generated/ndpi_asn_hotspotshield.c.inc"
#include "inc_generated/ndpi_asn_github.c.inc"
#include "inc_generated/ndpi_asn_steam.c.inc"
#include "inc_generated/ndpi_asn_bloomberg.c.inc"
#include "inc_generated/ndpi_asn_edgecast.c.inc"
#include "inc_generated/ndpi_asn_goto.c.inc"
#include "inc_generated/ndpi_asn_riotgames.c.inc"
#include "inc_generated/ndpi_asn_threema.c.inc"
#include "inc_generated/ndpi_asn_alibaba.c.inc"
#include "inc_generated/ndpi_asn_avast.c.inc"
#include "inc_generated/ndpi_asn_discord.c.inc"
#include "inc_generated/ndpi_asn_line.c.inc"
#include "inc_generated/ndpi_asn_vk.c.inc"
#include "inc_generated/ndpi_asn_yandex.c.inc"
#include "inc_generated/ndpi_asn_yandex_cloud.c.inc"
#include "inc_generated/ndpi_asn_disney_plus.c.inc"
#include "inc_generated/ndpi_asn_hulu.c.inc"
#include "inc_generated/ndpi_asn_epicgames.c.inc"
#include "inc_generated/ndpi_asn_nvidia.c.inc"
#include "inc_generated/ndpi_asn_roblox.c.inc"
#include "inc_generated/ndpi_domains_ms_office365_match.c.inc"
#include "inc_generated/ndpi_domains_ms_onedrive_match.c.inc"
#include "inc_generated/ndpi_domains_ms_outlook_match.c.inc"
#include "inc_generated/ndpi_domains_ms_teams_match.c.inc"
#include "inc_generated/ndpi_domains_ms_azure_match.c.inc"

/* Third party libraries */
#include "third_party/include/ndpi_patricia.h"
#include "third_party/include/ndpi_md5.h"
#include "third_party/include/ndpi_sha256.h"
#include "protocols.c"
#include "ndpi_utils.c"
#include "ndpi_hash.c"
#include "ndpi_cache.c"
#include "ndpi_fingerprint.c"
#include "ndpi_serializer.c"
#include "ndpi_memory.c"
#include "ndpi_analyze.c"
#include "ndpi_geoip.c"

/* #define MATCH_DEBUG 1 */

NDPI_STATIC int ndpi_debug_print_level = 0;
#ifdef HAVE_NBPF
#include "nbpf.h"
#endif

/* #define MATCH_DEBUG 1 */

/* ****************************************** */

static void *(*_ndpi_flow_malloc)(size_t size);
static void (*_ndpi_flow_free)(void *ptr);

static u_int32_t _ticks_per_second = 1000;

/* ****************************************** */


static ndpi_risk_info ndpi_known_risks[] = {
  { NDPI_NO_RISK,                               NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_NO_ACCOUNTABILITY  },
  { NDPI_URL_POSSIBLE_XSS,                      NDPI_RISK_SEVERE, CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_URL_POSSIBLE_SQL_INJECTION,            NDPI_RISK_SEVERE, CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_URL_POSSIBLE_RCE_INJECTION,            NDPI_RISK_SEVERE, CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_BINARY_APPLICATION_TRANSFER,           NDPI_RISK_SEVERE, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_KNOWN_PROTOCOL_ON_NON_STANDARD_PORT,   NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_TLS_SELFSIGNED_CERTIFICATE,            NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_TLS_OBSOLETE_VERSION,                  NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_WEAK_CIPHER,                       NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_CERTIFICATE_EXPIRED,               NDPI_RISK_HIGH,   CLIENT_LOW_RISK_PERCENTAGE,  NDPI_SERVER_ACCOUNTABLE },
  { NDPI_TLS_CERTIFICATE_MISMATCH,              NDPI_RISK_HIGH,   CLIENT_FAIR_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_HTTP_SUSPICIOUS_USER_AGENT,            NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_NUMERIC_IP_HOST,                       NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_HTTP_SUSPICIOUS_URL,                   NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_HTTP_SUSPICIOUS_HEADER,                NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_NOT_CARRYING_HTTPS,                NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_SUSPICIOUS_DGA_DOMAIN,                 NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_MALFORMED_PACKET,                      NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_SSH_OBSOLETE_CLIENT_VERSION_OR_CIPHER, NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_SSH_OBSOLETE_SERVER_VERSION_OR_CIPHER, NDPI_RISK_MEDIUM, CLIENT_LOW_RISK_PERCENTAGE,  NDPI_SERVER_ACCOUNTABLE },
  { NDPI_SMB_INSECURE_VERSION,                  NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_FREE_21,                               NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_UNSAFE_PROTOCOL,                       NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_DNS_SUSPICIOUS_TRAFFIC,                NDPI_RISK_MEDIUM, CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_MISSING_SNI,                       NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_HTTP_SUSPICIOUS_CONTENT,               NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_RISKY_ASN,                             NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_RISKY_DOMAIN,                          NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_MALICIOUS_FINGERPRINT,                 NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_MALICIOUS_SHA1_CERTIFICATE,            NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_DESKTOP_OR_FILE_SHARING_SESSION,       NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_TLS_UNCOMMON_ALPN,                     NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_CERT_VALIDITY_TOO_LONG,            NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_SERVER_ACCOUNTABLE },
  { NDPI_TLS_SUSPICIOUS_EXTENSION,              NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_TLS_FATAL_ALERT,                       NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_SUSPICIOUS_ENTROPY,                    NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_CLEAR_TEXT_CREDENTIALS,                NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_DNS_LARGE_PACKET,                      NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_DNS_FRAGMENTED,                        NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_INVALID_CHARACTERS,                    NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_POSSIBLE_EXPLOIT,                      NDPI_RISK_SEVERE, CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_CERTIFICATE_ABOUT_TO_EXPIRE,       NDPI_RISK_MEDIUM, CLIENT_LOW_RISK_PERCENTAGE,  NDPI_SERVER_ACCOUNTABLE },
  { NDPI_PUNYCODE_IDN,                          NDPI_RISK_LOW,    CLIENT_LOW_RISK_PERCENTAGE,  NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_ERROR_CODE_DETECTED,                   NDPI_RISK_LOW,    CLIENT_LOW_RISK_PERCENTAGE,  NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_HTTP_CRAWLER_BOT,                      NDPI_RISK_LOW,    CLIENT_LOW_RISK_PERCENTAGE,  NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_ANONYMOUS_SUBSCRIBER,                  NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_UNIDIRECTIONAL_TRAFFIC,                NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_HTTP_OBSOLETE_SERVER,                  NDPI_RISK_MEDIUM, CLIENT_LOW_RISK_PERCENTAGE,  NDPI_SERVER_ACCOUNTABLE },
  { NDPI_PERIODIC_FLOW,                         NDPI_RISK_LOW,    CLIENT_LOW_RISK_PERCENTAGE,  NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_MINOR_ISSUES,                          NDPI_RISK_LOW,    CLIENT_LOW_RISK_PERCENTAGE,  NDPI_BOTH_ACCOUNTABLE   },
  { NDPI_TCP_ISSUES,                            NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_FREE_51,                               NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_TLS_ALPN_SNI_MISMATCH,                 NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_MALWARE_HOST_CONTACTED,                NDPI_RISK_SEVERE, CLIENT_HIGH_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_BINARY_DATA_TRANSFER,                  NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_PROBING_ATTEMPT,                       NDPI_RISK_MEDIUM, CLIENT_FAIR_RISK_PERCENTAGE, NDPI_CLIENT_ACCOUNTABLE },
  { NDPI_OBFUSCATED_TRAFFIC,                    NDPI_RISK_HIGH,   CLIENT_HIGH_RISK_PERCENTAGE, NDPI_BOTH_ACCOUNTABLE },

  /* Leave this as last member */
  { NDPI_MAX_RISK,                              NDPI_RISK_LOW,    CLIENT_FAIR_RISK_PERCENTAGE, NDPI_NO_ACCOUNTABILITY   }
};

#if !defined(NDPI_CFFI_PREPROCESSING) && defined(__linux__)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(ndpi_known_risks) / sizeof(ndpi_risk_info) == NDPI_MAX_RISK + 1,
               "Invalid risks length. Do you need to update 'ndpi_known_risks' array?");
#endif
#endif


/* ****************************************** */

/* Forward */
static int addDefaultPort(struct ndpi_detection_module_struct *ndpi_str,
			  ndpi_port_range *range, ndpi_proto_defaults_t *def,
			  u_int8_t customUserProto, default_ports_tree_node_t **root,
			  const char *_func, int _line);

static void ndpi_reset_packet_line_info(struct ndpi_packet_struct *packet);
static void ndpi_int_change_protocol(struct ndpi_flow_struct *flow,
				     u_int16_t upper_detected_protocol, u_int16_t lower_detected_protocol,
				     ndpi_confidence_t confidence);

static int ndpi_callback_init(struct ndpi_detection_module_struct *ndpi_str);
static void ndpi_enabled_callbacks_init(struct ndpi_detection_module_struct *ndpi_str,
					int count_only);

static void set_default_config(struct ndpi_detection_module_config_struct *cfg);

static void internal_giveup(struct ndpi_detection_module_struct *ndpi_str,
                            struct ndpi_flow_struct *flow,
                            ndpi_protocol *ret);

/* ****************************************** */

static ndpi_custom_dga_predict_fctn ndpi_dga_function = NULL;

/* ****************************************** */

static inline u_int8_t flow_is_proto(struct ndpi_flow_struct *flow, u_int16_t p) {
  return((flow->detected_protocol_stack[0] == p) || (flow->detected_protocol_stack[1] == p));
}

/* ****************************************** */

void *ndpi_flow_malloc(size_t size) {
  return(_ndpi_flow_malloc ? _ndpi_flow_malloc(size) : ndpi_malloc(size));
}

/* ****************************************** */

void ndpi_flow_free(void *ptr) {
  if(_ndpi_flow_free)
    _ndpi_flow_free(ptr);
  else
    ndpi_free_flow((struct ndpi_flow_struct *) ptr);
}

/* *********************************************************************************** */

u_int32_t ndpi_detection_get_sizeof_ndpi_flow_struct(void) {
  return(sizeof(struct ndpi_flow_struct));
}

/* *********************************************************************************** */

u_int32_t ndpi_detection_get_sizeof_ndpi_flow_tcp_struct(void) {
  return(sizeof(struct ndpi_flow_tcp_struct));
}

/* *********************************************************************************** */

u_int32_t ndpi_detection_get_sizeof_ndpi_flow_udp_struct(void) {
  return(sizeof(struct ndpi_flow_udp_struct));
}

/* *********************************************************************************** */

char *ndpi_get_proto_by_id(struct ndpi_detection_module_struct *ndpi_str, u_int id) {
  if(!ndpi_str)
    return NULL;
  return((id >= ndpi_str->ndpi_num_supported_protocols) ? NULL : ndpi_str->proto_defaults[id].protoName);
}

/* *********************************************************************************** */

/* NOTE: name can be HTTP or YouTube but not TLS.YouTube */
u_int16_t ndpi_get_proto_by_name(struct ndpi_detection_module_struct *ndpi_str, const char *name) {
  u_int16_t i, num;
  const char *p;

  if(!ndpi_str || !name)
    return(NDPI_PROTOCOL_UNKNOWN);

  num = ndpi_get_num_supported_protocols(ndpi_str);

  for(i = 0; i < num; i++) {
    p = ndpi_get_proto_by_id(ndpi_str, i);
#ifdef __KERNEL__
    if(!p) {
	    pr_err("%s: #%d empty!\n",__func__,i);
	    continue;
    }
#endif
    if(strcasecmp(p, name) == 0)
      return(i);
  }

  return(NDPI_PROTOCOL_UNKNOWN);
}

/* *********************************************************************************** */

/* NOTE: supported both HTTP and TLS.YouTube */
ndpi_master_app_protocol ndpi_get_protocol_by_name(struct ndpi_detection_module_struct *ndpi_str, const char *name) {
  char *dot, buf[256];
  ndpi_master_app_protocol ret = { NDPI_PROTOCOL_UNKNOWN, NDPI_PROTOCOL_UNKNOWN };

  if(!ndpi_str || !name)
    return(ret);

  snprintf(buf, sizeof(buf), "%s", name);
  dot = strchr(buf, '.');

  if(dot) {
    /* TLS.YouTube */
    dot[0] = '\0';
    ret.app_protocol = ndpi_get_proto_by_name(ndpi_str,  &dot[1]);

    if(ret.app_protocol == NDPI_PROTOCOL_UNKNOWN)
      return(ret); /* Parsing error */
  } else {
    /* TLS */
  }

  ret.master_protocol = ndpi_get_proto_by_name(ndpi_str,  buf); /* both cases */

  return(ret);
}

/* ************************************************************************************* */
/* ************************************************************************************* */

static void ndpi_add_user_proto_id_mapping(struct ndpi_detection_module_struct *ndpi_str,
					   u_int16_t ndpi_proto_id, u_int16_t user_proto_id) {
  NDPI_LOG_DBG2(ndpi_str, "[DEBUG] *** %u (>= %u)-> %u\n",
		ndpi_proto_id, NDPI_MAX_SUPPORTED_PROTOCOLS,
		user_proto_id);
  if(ndpi_proto_id >= NDPI_MAX_SUPPORTED_PROTOCOLS)
    ndpi_str->ndpi_to_user_proto_id[ndpi_proto_id-NDPI_MAX_SUPPORTED_PROTOCOLS] = user_proto_id;
}

/* ************************************************************************************* */

/* Map a custom user protocol into an internal nDPI protocol id */
u_int16_t ndpi_map_user_proto_id_to_ndpi_id(struct ndpi_detection_module_struct *ndpi_str,
					    u_int16_t user_proto_id) {

#if 0 /* Too much verbose... */
  NDPI_LOG_DBG2(ndpi_str, "[DEBUG] ***** %s(%u)\n", __FUNCTION__, user_proto_id);
#endif

  if(!ndpi_str)
    return(0);

  if(user_proto_id < NDPI_MAX_SUPPORTED_PROTOCOLS)
    return(user_proto_id);
  else {
    u_int idx, idx_max = ndpi_str->ndpi_num_supported_protocols - NDPI_MAX_SUPPORTED_PROTOCOLS;

    /* TODO: improve it and remove linear scan */
    for(idx = 0; idx < idx_max; idx++) {
      if(ndpi_str->ndpi_to_user_proto_id[idx] == 0)
	break;
      else if(ndpi_str->ndpi_to_user_proto_id[idx] == user_proto_id) {
	return(idx + NDPI_MAX_SUPPORTED_PROTOCOLS);
      }
    }
  }

  return(0);
}

/* ************************************************************************************* */

/* Map an internal nDPI protocol id to a custom user protocol */
u_int16_t ndpi_map_ndpi_id_to_user_proto_id(struct ndpi_detection_module_struct *ndpi_str,
					    u_int16_t ndpi_proto_id) {
#if 0 /* Too much verbose... */
  NDPI_LOG_DBG2(ndpi_str, "[DEBUG] ***** %s(%u)\n", __FUNCTION__, ndpi_proto_id);
#endif

  if(!ndpi_str)
    return(0);

  if(ndpi_proto_id < NDPI_MAX_SUPPORTED_PROTOCOLS)
    return(ndpi_proto_id);
  else if(ndpi_proto_id < ndpi_str->ndpi_num_supported_protocols) {
    u_int id = ndpi_proto_id - NDPI_MAX_SUPPORTED_PROTOCOLS;

    if(id < ndpi_str->ndpi_num_supported_protocols)
      return(ndpi_str->ndpi_to_user_proto_id[id]);
  }

  return(0);
}

/* ************************************************************************************* */

static ndpi_port_range *ndpi_build_default_ports_range(ndpi_port_range *ports, u_int16_t portA_low, u_int16_t portA_high,
                                                       u_int16_t portB_low, u_int16_t portB_high, u_int16_t portC_low,
                                                       u_int16_t portC_high, u_int16_t portD_low, u_int16_t portD_high,
                                                       u_int16_t portE_low, u_int16_t portE_high) {
  int i = 0;

  ports[i].port_low = portA_low, ports[i].port_high = portA_high;
  i++;
  ports[i].port_low = portB_low, ports[i].port_high = portB_high;
  i++;
  ports[i].port_low = portC_low, ports[i].port_high = portC_high;
  i++;
  ports[i].port_low = portD_low, ports[i].port_high = portD_high;
  i++;
  ports[i].port_low = portE_low, ports[i].port_high = portE_high;

  return(ports);
}


/* ************************************************************************************* */

ndpi_port_range *ndpi_build_default_ports(ndpi_port_range *ports, u_int16_t portA, u_int16_t portB, u_int16_t portC,
                                          u_int16_t portD, u_int16_t portE) {
  int i = 0;

  ports[i].port_low = portA, ports[i].port_high = portA;
  i++;
  ports[i].port_low = portB, ports[i].port_high = portB;
  i++;
  ports[i].port_low = portC, ports[i].port_high = portC;
  i++;
  ports[i].port_low = portD, ports[i].port_high = portD;
  i++;
  ports[i].port_low = portE, ports[i].port_high = portE;

  return(ports);
}

/* ********************************************************************************** */

void ndpi_set_proto_breed(struct ndpi_detection_module_struct *ndpi_str,
			  u_int16_t protoId, ndpi_protocol_breed_t breed) {
  if(!ndpi_is_valid_protoId(protoId))
    return;
  else if(ndpi_str)
    ndpi_str->proto_defaults[protoId].protoBreed = breed;
}

/* ********************************************************************************** */

void ndpi_set_proto_category(struct ndpi_detection_module_struct *ndpi_str, u_int16_t protoId,
                             ndpi_protocol_category_t protoCategory) {
  if(!ndpi_is_valid_protoId(protoId))
    return;
  else if(ndpi_str)
    ndpi_str->proto_defaults[protoId].protoCategory = protoCategory;
}

/* ********************************************************************************** */

int is_flow_addr_informative(const struct ndpi_flow_struct *flow)
{
  /* The ideas is to tell if the address itself carries some useful information or not.
     Examples:
     a flow to a Facebook address is quite likely related to some Facebook apps
     a flow to an AWS address might be potentially anything
  */

  switch(flow->guessed_protocol_id_by_ip) {
  case NDPI_PROTOCOL_UNKNOWN:
    /* This is basically the list of cloud providers supported by nDPI */
  case NDPI_PROTOCOL_TENCENT:
  case NDPI_PROTOCOL_EDGECAST:
  case NDPI_PROTOCOL_ALIBABA:
  case NDPI_PROTOCOL_YANDEX_CLOUD:
  case NDPI_PROTOCOL_AMAZON_AWS:
  case NDPI_PROTOCOL_MICROSOFT_AZURE:
  case NDPI_PROTOCOL_CACHEFLY:
  case NDPI_PROTOCOL_CLOUDFLARE:
  case NDPI_PROTOCOL_GOOGLE_CLOUD:
  case NDPI_PROTOCOL_DIGITALOCEAN:
    return 0;
    /* This is basically the list of VPNs (with **entry** addresses) supported by nDPI */
  case NDPI_PROTOCOL_NORDVPN:
  case NDPI_PROTOCOL_SURFSHARK:
  case NDPI_PROTOCOL_TOR:
    return 0;
  default:
    return 1;
  }
}

/* ********************************************************************************** */

/*
  There are some (master) protocols that are informative, meaning that it shows
  what is the subprotocol about, but also that the subprotocol isn't a real protocol.

  Example:
  - DNS is informative as if we see a DNS request for www.facebook.com, the
  returned protocol is DNS.Facebook, but Facebook isn't a real subprotocol but
  rather it indicates a query for Facebook and not Facebook traffic.
  - HTTP/SSL are NOT informative as SSL.Facebook (likely) means that this is
  SSL (HTTPS) traffic containg Facebook traffic.
*/
u_int8_t ndpi_is_subprotocol_informative(u_int16_t protoId) {
  if(!ndpi_is_valid_protoId(protoId))
    return(0);

  switch(protoId) {
    /* All dissectors that have calls to ndpi_match_host_subprotocol() */
  case NDPI_PROTOCOL_DNS:
    return(1);

  default:
    return(0);
  }
}

/* ********************************************************************************** */

void exclude_dissector(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow,
                       u_int16_t dissector_idx, const char *_file, const char *_func, int _line) {
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  /* TODO */
  if(ndpi_str->cfg.log_level >= NDPI_LOG_DEBUG && ndpi_str->ndpi_debug_printf != NULL) {
    (*(ndpi_str->ndpi_debug_printf))(ndpi_str->callback_buffer[dissector_idx].first_protocol_id,
                                     ndpi_str, NDPI_LOG_DEBUG, _file, _func, _line, "exclude %s\n",
                                     ndpi_str->callback_buffer[dissector_idx].name);
  }
#else
  (void)ndpi_str;
  (void)_file;
  (void)_func;
  (void)_line;
#endif
  NDPI_DISSECTOR_BITMASK_SET(flow->excluded_dissectors_bitmask, dissector_idx);
}

/* ********************************************************************************** */

static int is_proto_enabled(struct ndpi_detection_module_struct *ndpi_str, int protoId)
{
  /* Custom protocols are always enabled */
  if(protoId >= NDPI_MAX_SUPPORTED_PROTOCOLS)
    return 1;
  if(NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_str->detection_bitmask, protoId) == 0)
    return 0;
  return 1;
}

/* ********************************************************************************** */

void ndpi_set_proto_subprotocols(struct ndpi_detection_module_struct *ndpi_str, int protoId, ...)
{
  va_list ap;
  int current_arg = protoId;
  size_t i = 0;

  if(!is_proto_enabled(ndpi_str, protoId)) {
    NDPI_LOG_DBG(ndpi_str, "[NDPI] Skip subprotocols for %d (disabled)\n", protoId);
    return;
  }

  va_start(ap, protoId);
  while (current_arg != NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS) {
    if(!is_proto_enabled(ndpi_str, current_arg)) {
      NDPI_LOG_DBG(ndpi_str, "[NDPI] Skip subprotocol %d (disabled)\n", protoId);
    } else {
      ndpi_str->proto_defaults[protoId].subprotocol_count++;
    }
    current_arg = va_arg(ap, int);
  }
  va_end(ap);

  ndpi_str->proto_defaults[protoId].subprotocols = NULL;

  /* The last protocol is not a subprotocol. */
  ndpi_str->proto_defaults[protoId].subprotocol_count--;
  /* No subprotocol was set before NDPI_NO_MORE_SUBPROTOCOLS. */
  if(ndpi_str->proto_defaults[protoId].subprotocol_count == 0) {
    return;
  }

  ndpi_str->proto_defaults[protoId].subprotocols =
    ndpi_malloc(sizeof(protoId) * ndpi_str->proto_defaults[protoId].subprotocol_count);
  if(!ndpi_str->proto_defaults[protoId].subprotocols) {
    ndpi_str->proto_defaults[protoId].subprotocol_count = 0;
    return;
  }

  va_start(ap, protoId);
  current_arg = va_arg(ap, int);

  while (current_arg != NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS) {
    if(is_proto_enabled(ndpi_str, current_arg)) {
      ndpi_str->proto_defaults[protoId].subprotocols[i++] = current_arg;
    }
    current_arg = va_arg(ap, int);
  }

  va_end(ap);
}

/* ********************************************************************************** */

void ndpi_set_proto_defaults(struct ndpi_detection_module_struct *ndpi_str,
			     u_int8_t is_cleartext, u_int8_t is_app_protocol,
			     ndpi_protocol_breed_t breed,
			     u_int16_t protoId, char *protoName,
			     ndpi_protocol_category_t protoCategory,
			     ndpi_protocol_qoe_category_t qoeCategory,
			     ndpi_port_range *tcpDefPorts,
			     ndpi_port_range *udpDefPorts) {
  char *name;
  int j;

  if(!ndpi_str || !protoName)
    return;

  if(!ndpi_is_valid_protoId(protoId)) {
    NDPI_LOG_ERR(ndpi_str, "[NDPI] %s/protoId=%d: INTERNAL ERROR\n", protoName, protoId);
    return;
  }

  if(ndpi_str->proto_defaults[protoId].protoName != NULL) {
    NDPI_LOG_DBG2(ndpi_str, "[NDPI] %s/protoId=%d: already initialized. Ignoring it\n", protoName, protoId);
    return;
  }

  name = ndpi_strdup(protoName);
  if(!name) {
    NDPI_LOG_ERR(ndpi_str, "[NDPI] %s/protoId=%d: mem allocation error\n", protoName, protoId);
    return;
  }

  ndpi_str->proto_defaults[protoId].isClearTextProto = is_cleartext;
  /*
    is_appprotocol=1 means that this is only an application protocol layered
    on top of a network protocol. Example WhatsApp=1, TLS=0
  */
  ndpi_str->proto_defaults[protoId].isAppProtocol = is_app_protocol;
  ndpi_str->proto_defaults[protoId].protoName = name;
  ndpi_str->proto_defaults[protoId].protoCategory = protoCategory;
  ndpi_str->proto_defaults[protoId].protoId = protoId;
  ndpi_str->proto_defaults[protoId].protoBreed = breed;
  ndpi_str->proto_defaults[protoId].qoeCategory = qoeCategory;
  ndpi_str->proto_defaults[protoId].subprotocols = NULL;
  ndpi_str->proto_defaults[protoId].subprotocol_count = 0;

  if(!is_proto_enabled(ndpi_str, protoId)) {
    NDPI_LOG_DBG(ndpi_str, "[NDPI] Skip default ports for %s/protoId=%d: disabled\n", protoName, protoId);
    return;
  }

  for(j = 0; j < MAX_DEFAULT_PORTS; j++) {
    if(udpDefPorts[j].port_low != 0)
      addDefaultPort(ndpi_str, &udpDefPorts[j], &ndpi_str->proto_defaults[protoId], 0, &ndpi_str->udpRoot,
		     __FUNCTION__, __LINE__);

    if(tcpDefPorts[j].port_low != 0)
      addDefaultPort(ndpi_str, &tcpDefPorts[j], &ndpi_str->proto_defaults[protoId], 0, &ndpi_str->tcpRoot,
		     __FUNCTION__, __LINE__);

    ndpi_str->proto_defaults[protoId].tcp_default_ports[j] = tcpDefPorts[j];
    ndpi_str->proto_defaults[protoId].udp_default_ports[j] = udpDefPorts[j];
  }
}

/* ******************************************************************** */

static int default_ports_tree_node_t_cmp(const void *a, const void *b) {
  default_ports_tree_node_t *fa = (default_ports_tree_node_t *) a;
  default_ports_tree_node_t *fb = (default_ports_tree_node_t *) b;

  //printf("[NDPI] %s(%d, %d)\n", __FUNCTION__, fa->default_port, fb->default_port);

  return((fa->default_port == fb->default_port) ? 0 : ((fa->default_port < fb->default_port) ? -1 : 1));
}

/* ******************************************************************** */

static int addDefaultPort(struct ndpi_detection_module_struct *ndpi_str,
			  ndpi_port_range *range,
			  ndpi_proto_defaults_t *def,
			  u_int8_t customUserProto,
			  default_ports_tree_node_t **root,
			  const char *_func,
			  int _line) {
  (void)_func;
  (void)_line;

  u_int32_t port;

  for(port = range->port_low; port <= range->port_high; port++) {
    default_ports_tree_node_t *node =
      (default_ports_tree_node_t *) ndpi_malloc(sizeof(default_ports_tree_node_t));
    default_ports_tree_node_t *ret;

    if(!node) {
      NDPI_LOG_ERR(ndpi_str, "%s:%d not enough memory\n", _func, _line);
      break;
    }

    node->proto = def, node->default_port = port, node->customUserProto = customUserProto;
    ret = (default_ports_tree_node_t *) ndpi_tsearch(node,
						     (void *) root,
						     default_ports_tree_node_t_cmp); /* Add it to the tree */

    if(ret == NULL) {
      NDPI_LOG_DBG(ndpi_str, "[NDPI] %s:%d error searching for port %u\n", _func, _line, port);
      ndpi_free(node);
      break;
    }

    if(ret != node) {
      if(node && node->proto && node->proto->protoName &&
	 ret && ret->proto && ret->proto->protoName) {
	 if(strcmp(node->proto->protoName,ret->proto->protoName))
            NDPI_LOG_DBG(ndpi_str, "[NDPI] %s:%d found duplicate for port %u: %s overwriting %s\n",
		   _func, _line, port,
		   node->proto->protoName,
		   ret->proto->protoName);
      } else {
            NDPI_LOG_DBG(ndpi_str, "[NDPI] %s:%d found duplicate for port %u: overwriting\n",
		   _func, _line, port);
      }

      ret->proto = def;
      ndpi_free(node);
      return(-1); /* Duplicates found */
    }
  }

  return(0);
}

/* ****************************************************** */

/*
  This is a function used to see if we need to
  add a trailer $ in case the string is complete
  or is a string that can be matched in the
  middle of a domain name

  Example:
  microsoft.com    ->     microsoft.com$
  apple.           ->     apple.
*/
static u_int8_t ndpi_is_middle_string_char(char c) {
  switch(c) {
  case '.':
  case '-':
    return(1);

  default:
    return(0);
  }
}

/*******************************************************/

static const u_int8_t ndpi_domain_level_automat[4][4]= {
  /* symbol,'.','-',inc */
  { 2,1,2,0 }, // start state
  { 2,0,0,0 }, // first char is '.'; disable .. or .-
  { 2,3,2,0 }, // part of domain name
  { 2,0,0,1 }  // next level domain name; disable .. or .-
};

/*
 * domain level
 *  a. = 1
 * .a. = 1
 * a.b = 2
 */

static u_int8_t ndpi_domain_level(const char *name) {
  u_int8_t level = 1, state = 0;
  char c;
  while((c = *name++) != '\0') {
    c = c == '-' ? 2 : (c == '.' ? 1:0);
    level += ndpi_domain_level_automat[state][3];
    state  = ndpi_domain_level_automat[state][(u_int8_t)c];
    if(!state) break;
  }
  return state >= 2 ? level:0;
}

/* ****************************************************** */

int ndpi_string_to_automa(struct ndpi_detection_module_struct *ndpi_str,
				 void *ac_automa, const char *value,
                                 u_int16_t protocol_id, ndpi_protocol_category_t category,
				 ndpi_protocol_breed_t breed, u_int8_t level,
                                 u_int8_t add_ends_with) {
  AC_PATTERN_t ac_pattern;
  AC_ERROR_t rc;
  u_int len;
  u_int16_t d_proto_id;
  char *value_dup = NULL;

  if(!ndpi_is_valid_protoId(protocol_id)) {
    NDPI_LOG_ERR(ndpi_str, "[NDPI] protoId=%d: INTERNAL ERROR\n", protocol_id);
    return(-1);
  }

  if((ac_automa == NULL) || (value == NULL) || !*value)
    return(-2);

  value_dup = ndpi_strdup(value);
  if(!value_dup)
    return(-1);

  memset(&ac_pattern, 0, sizeof(ac_pattern));

  len = strlen(value);
  
  if( *value == '|' ) {
      len--;
      strncpy(value_dup,value+1, len);
      value_dup[len] = '\0';
      ac_pattern.rep.from_start = 1;
//    printf("Add %d:%.*s match from start\n",ac_pattern.length,ac_pattern.length,ac_pattern.astring);
  }
  if (len > 2 && value_dup[len-1] == '|') {
      len--;
      value_dup[len] = '\0';
      ac_pattern.rep.at_end = 1;
//    printf("Add %d:%.*s match from end\n",ac_pattern.length,ac_pattern.length,ac_pattern.astring);
  }

  ac_pattern.astring      = value_dup;
  ac_pattern.length       = len;
  ac_pattern.rep.number   = protocol_id;
  ac_pattern.rep.category = (u_int16_t) category;
  ac_pattern.rep.breed    = (u_int16_t) breed;
  ac_pattern.rep.level    = level ? level : ndpi_domain_level(value_dup);
  ac_pattern.rep.at_end  |= add_ends_with && !ndpi_is_middle_string_char(value_dup[len-1]); /* len != 0 */
  ac_pattern.rep.dot      = memchr(value,'.',len) != NULL;
  ac_pattern.rep.no_override = add_ends_with > 1;

#ifdef MATCH_DEBUG
  printf("Adding to %s %lx [%s%s][protocol_id: %u][category: %u][breed: %u][level: %u]\n",
	 ac_automa->name,(unsigned long int)ac_automa,
	 ac_pattern.astring,ac_pattern.rep.at_end? "$":"", protocol_id, category, breed,ac_pattern.rep.level);
#endif

  rc = ac_automata_add((AC_AUTOMATA_t *)ac_automa, &ac_pattern);
  d_proto_id = ac_pattern.rep.number;

  if(rc != ACERR_SUCCESS) {
        ndpi_free(value_dup);
	if(rc != ACERR_DUPLICATE_PATTERN)
		return (-2);
	{
	  const char *tproto = ndpi_get_proto_by_id(ndpi_str, protocol_id);
	  if(protocol_id == d_proto_id) {
		  if(0) NDPI_LOG_ERR(ndpi_str, "[NDPI] Duplicate '%s' proto %s\n",
				  value, tproto)
	  } else {
		  NDPI_LOG_ERR(ndpi_str, "[NDPI] Missmatch '%s' proto %s[%d] origin %s[%d]\n",
				  value, tproto,protocol_id,
				  ndpi_get_proto_by_id(ndpi_str,d_proto_id),d_proto_id);
	  	  return (-3);
	  }
        }
  }

  return(0);
}

/* ****************************************************** */

static int ndpi_add_host_url_subprotocol(struct ndpi_detection_module_struct *ndpi_str,
					 char *value, int protocol_id,
                                         ndpi_protocol_category_t category,
					 ndpi_protocol_breed_t breed, u_int8_t level) {
#ifndef NDPI_ENABLE_DEBUG_MESSAGES
  NDPI_LOG_DBG2(ndpi_str, "[NDPI] Adding [%s][%d]\n", value, protocol_id);
#endif

  return ndpi_string_to_automa(ndpi_str, (AC_AUTOMATA_t *)ndpi_str->host_automa.ac_automa,
			       value, protocol_id, category, breed, level, 1);

}

/* ******************************************************************** */

int ndpi_init_empty_app_protocol(ndpi_protocol_match const * const hostname_list,
                                 ndpi_protocol_match * const empty_app_protocol) {
  if (hostname_list[0].proto_name == NULL)
    return 1;

  memset(empty_app_protocol, 0, sizeof(*empty_app_protocol));
  empty_app_protocol->proto_name = hostname_list[0].proto_name;
  empty_app_protocol->protocol_id = hostname_list[0].protocol_id;
  empty_app_protocol->protocol_category = hostname_list[0].protocol_category;
  empty_app_protocol->protocol_breed = hostname_list[0].protocol_breed;
  empty_app_protocol->level = hostname_list[0].level;

  return 0;
}

/* ******************************************************************** */

int ndpi_init_app_protocol(struct ndpi_detection_module_struct *ndpi_str,
                           ndpi_protocol_match const * const match) {
  ndpi_port_range ports_a[MAX_DEFAULT_PORTS], ports_b[MAX_DEFAULT_PORTS];

  if(ndpi_str->proto_defaults[match->protocol_id].protoName == NULL) {
    ndpi_str->proto_defaults[match->protocol_id].protoName = ndpi_strdup(match->proto_name);

    if(!ndpi_str->proto_defaults[match->protocol_id].protoName)
      return 1;

    ndpi_str->proto_defaults[match->protocol_id].isAppProtocol = 1;
    ndpi_str->proto_defaults[match->protocol_id].protoId = match->protocol_id;
    ndpi_str->proto_defaults[match->protocol_id].protoCategory = match->protocol_category;
    ndpi_str->proto_defaults[match->protocol_id].protoBreed = match->protocol_breed;

    switch(match->protocol_category) {
    case NDPI_PROTOCOL_CATEGORY_WEB:
      ndpi_str->proto_defaults[match->protocol_id].qoeCategory = NDPI_PROTOCOL_QOE_CATEGORY_WEB_BROWSING;
      break;

    case NDPI_PROTOCOL_CATEGORY_GAME:
      ndpi_str->proto_defaults[match->protocol_id].qoeCategory = NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING;
      break;

    case NDPI_PROTOCOL_CATEGORY_VOIP:
      ndpi_str->proto_defaults[match->protocol_id].qoeCategory = NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS;
      break;

    case NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS:
      ndpi_str->proto_defaults[match->protocol_id].qoeCategory = NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS;
      break;

    case NDPI_PROTOCOL_CATEGORY_MEDIA:
    case NDPI_PROTOCOL_CATEGORY_STREAMING:
    case NDPI_PROTOCOL_CATEGORY_MUSIC:
    case NDPI_PROTOCOL_CATEGORY_VIDEO:
      ndpi_str->proto_defaults[match->protocol_id].qoeCategory = NDPI_PROTOCOL_QOE_CATEGORY_BUFFERED_STREAMING;
      break;

    default:
      ndpi_str->proto_defaults[match->protocol_id].qoeCategory = NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED;
      break;
    }

    ndpi_set_proto_defaults(ndpi_str,
			    ndpi_str->proto_defaults[match->protocol_id].isClearTextProto,
			    ndpi_str->proto_defaults[match->protocol_id].isAppProtocol,
			    ndpi_str->proto_defaults[match->protocol_id].protoBreed,
			    ndpi_str->proto_defaults[match->protocol_id].protoId,
			    ndpi_str->proto_defaults[match->protocol_id].protoName,
			    ndpi_str->proto_defaults[match->protocol_id].protoCategory,
			    ndpi_str->proto_defaults[match->protocol_id].qoeCategory,
			    ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			    ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  }

  if(!is_proto_enabled(ndpi_str, match->protocol_id)) {
    NDPI_LOG_DBG(ndpi_str, "[NDPI] Skip protocol match for %s/protoId=%d: disabled\n",
		 match->string_to_match, match->protocol_id);
    return 1;
  }

  return 0;
}

/* ******************************************************************** */

void ndpi_init_protocol_match(struct ndpi_detection_module_struct *ndpi_str,
                              ndpi_protocol_match const * const match) {
  if (ndpi_init_app_protocol(ndpi_str, match) == 0) {
    ndpi_add_host_url_subprotocol(ndpi_str, match->string_to_match,
				  match->protocol_id, match->protocol_category,
				  match->protocol_breed, match->level);
  }
}

/* ******************************************************************** */

#ifndef __KERNEL__
/* Self check function to be called only for testing purposes */
void ndpi_self_check_host_match(FILE *error_out) {
  u_int32_t i, j;

  for(i = 0; host_match[i].string_to_match != NULL; i++) {
    if(host_match[i].string_to_match[0] == '.') {
      if (error_out != NULL) {
        fprintf(error_out,
                "[NDPI] INTERNAL ERROR Invalid string detected '%s'. It can not start with '.'\n",
                host_match[i].string_to_match);
        fprintf(error_out, "\nPlease fix host_match[] in ndpi_content_match.c.inc\n");
      }
      abort();
    }
    for(j = 0; host_match[j].string_to_match != NULL; j++) {
      if((i != j) && (strcmp(host_match[i].string_to_match, host_match[j].string_to_match) == 0)) {
        if (error_out != NULL) {
          fprintf(error_out,
                  "[NDPI] INTERNAL ERROR duplicate string detected '%s' [id: %u, id %u]\n",
                  host_match[i].string_to_match, i, j);
          fprintf(error_out, "\nPlease fix host_match[] in ndpi_content_match.c.inc\n");
        }
        abort();
      }
    }
  }
}
#endif
/* ******************************************************************** */

#define XGRAMS_C 26
static int ndpi_xgrams_inited = 0;
static unsigned int bigrams_bitmap[(XGRAMS_C*XGRAMS_C+31)/32];
static unsigned int impossible_bigrams_bitmap[(XGRAMS_C*XGRAMS_C+31)/32];
static unsigned int trigrams_bitmap[(XGRAMS_C*XGRAMS_C*XGRAMS_C+31)/32];


static void ndpi_xgrams_init(struct ndpi_detection_module_struct *ndpi_str,unsigned int *dst,size_t dn, const char **src,size_t sn,unsigned int l) {
unsigned int i,j,c;
  for(i=0;i < sn && src[i]; i++) {
    for(j=0,c=0; j < l; j++) {
        unsigned char a = (unsigned char)src[i][j];
        if(a < 'a' || a > 'z') { 
#ifndef __KERNEL__
	    printf("%u: c%u %c\n",i,j,a); abort(); 
#else
	    continue;
#endif
        }
        c *= XGRAMS_C;
        c += a - 'a';
    }
    if(src[i][l]) { 
#ifndef __KERNEL__
       printf("%u: c[%d] != 0\n",i,l); abort(); 
#else
       continue;
#endif
    }
    if((c >> 3) >= dn) {
#ifndef __KERNEL__
       abort();
#else
       continue;
#endif
    }
    dst[c >> 5] |= 1u << (c & 0x1f);
  }
}

/* ******************************************************************** */

ndpi_protocol_match *host_all_match_str[7] = {
	&host_match[0],
	&teams_host_match[0],
	&outlook_host_match[0],
	&ms_onedrive_host_match[0],
	&microsoft365_host_match[0],
	&azure_host_match[0],
	NULL
};


static void init_string_based_protocols(struct ndpi_detection_module_struct *ndpi_str) {
  int i;

  for(i = 0; host_match[i].string_to_match != NULL; i++)
	ndpi_init_protocol_match(ndpi_str, &host_match[i]);

  for(i = 0; teams_host_match[i].string_to_match != NULL; i++)
    ndpi_init_protocol_match(ndpi_str, &teams_host_match[i]);
  for(i = 0; outlook_host_match[i].string_to_match != NULL; i++)
    ndpi_init_protocol_match(ndpi_str, &outlook_host_match[i]);
  for(i = 0; ms_onedrive_host_match[i].string_to_match != NULL; i++)
    ndpi_init_protocol_match(ndpi_str, &ms_onedrive_host_match[i]);
  for(i = 0; microsoft365_host_match[i].string_to_match != NULL; i++)
    ndpi_init_protocol_match(ndpi_str, &microsoft365_host_match[i]);
  for(i = 0; azure_host_match[i].string_to_match != NULL; i++)
    ndpi_init_protocol_match(ndpi_str, &azure_host_match[i]);

  /* ************************ */
  if(ndpi_str->tls_cert_subject_automa.ac_automa != NULL) {
    ac_automata_release((AC_AUTOMATA_t *) ndpi_str->tls_cert_subject_automa.ac_automa,1);
    ndpi_str->tls_cert_subject_automa.ac_automa = ac_automata_init(NULL);
    ac_automata_feature(ndpi_str->tls_cert_subject_automa.ac_automa,AC_FEATURE_LC);
    ac_automata_name(ndpi_str->tls_cert_subject_automa.ac_automa,"tls_cert",AC_FEATURE_DEBUG);
  }

  for(i = 0; tls_certificate_match[i].string_to_match != NULL; i++) {
    if(!is_proto_enabled(ndpi_str, tls_certificate_match[i].protocol_id)) {
      NDPI_LOG_DBG(ndpi_str, "[NDPI] Skip tls cert match for %s/protoId=%d: disabled\n",
		   tls_certificate_match[i].string_to_match, tls_certificate_match[i].protocol_id);
      continue;
    }
    /* Note: string_to_match is not malloc'ed here as ac_automata_release is
     * called with free_pattern = 0 */
    ndpi_add_string_value_to_automa(ndpi_str->tls_cert_subject_automa.ac_automa,
				    tls_certificate_match[i].string_to_match,
                                    tls_certificate_match[i].protocol_id);
  }
  ac_automata_finalize((AC_AUTOMATA_t *) ndpi_str->tls_cert_subject_automa.ac_automa);

  /* ************************ */

  //ndpi_enable_loaded_categories(ndpi_str);

  if(!ndpi_xgrams_inited) {
    ndpi_xgrams_inited = 1;
    ndpi_xgrams_init(ndpi_str,bigrams_bitmap,sizeof(bigrams_bitmap),
		     ndpi_en_bigrams,sizeof(ndpi_en_bigrams)/sizeof(ndpi_en_bigrams[0]), 2);

    ndpi_xgrams_init(ndpi_str,impossible_bigrams_bitmap,sizeof(impossible_bigrams_bitmap),
		     ndpi_en_impossible_bigrams,sizeof(ndpi_en_impossible_bigrams)/sizeof(ndpi_en_impossible_bigrams[0]), 2);
    ndpi_xgrams_init(ndpi_str,trigrams_bitmap,sizeof(trigrams_bitmap),
		     ndpi_en_trigrams,sizeof(ndpi_en_trigrams)/sizeof(ndpi_en_trigrams[0]), 3);
  }
}

/* ******************************************************************** */

static int ndpi_validate_protocol_initialization(struct ndpi_detection_module_struct *ndpi_str) {
  u_int i,j;

  for(i = 0; i < ndpi_str->ndpi_num_supported_protocols; i++) {
    if(ndpi_str->proto_defaults[i].protoName == NULL) {
      NDPI_LOG_ERR(ndpi_str,
		   "[NDPI] INTERNAL ERROR missing protoName initialization for [protoId=%u]: recovering\n", i);
      return 1;
    } else {
#ifndef __KERNEL__
      if((i != NDPI_PROTOCOL_UNKNOWN) &&
	 (ndpi_str->proto_defaults[i].protoCategory == NDPI_PROTOCOL_CATEGORY_UNSPECIFIED)) {
	    NDPI_LOG_ERR(ndpi_str,
		     "[NDPI] INTERNAL ERROR missing category [protoId=%u/%s] initialization: recovering\n", i,
		     ndpi_str->proto_defaults[i].protoName ? ndpi_str->proto_defaults[i].protoName : "???");
	return 1;
      }
#endif
    }
    if(!strcmp(ndpi_str->proto_defaults[i].protoName,"Free")) continue;
    for(j = 0; j < i; j++)
      if(!strcmp(ndpi_str->proto_defaults[i].protoName,ndpi_str->proto_defaults[j].protoName)) {
  	    NDPI_LOG_ERR(ndpi_str, "[NDPI] INTERNAL ERROR: Name of the protocols are the same for #%u and #%u '%s' \n",i,j,
  			    ndpi_str->proto_defaults[i].protoName);
  	return 1;
      }
  }

  return 0;
}

/* ******************************************************************** */

/* This function is used to map protocol name and default ports and it MUST
   be updated whenever a new protocol is added to NDPI.

   Do NOT add web services (NDPI_SERVICE_xxx) here.
*/
static void ndpi_init_protocol_defaults(struct ndpi_detection_module_struct *ndpi_str) {
  ndpi_port_range ports_a[MAX_DEFAULT_PORTS], ports_b[MAX_DEFAULT_PORTS];
  int i;

  for (i = 0; i < (NDPI_MAX_SUPPORTED_PROTOCOLS); i++) {
      if(ndpi_str->proto_defaults[i].protoName)
        ndpi_free(ndpi_str->proto_defaults[i].protoName);
      if(ndpi_str->proto_defaults[i].subprotocols != NULL)
        ndpi_free(ndpi_str->proto_defaults[i].subprotocols);
  }

  /* Reset all settings for supported protocols */
  memset(&ndpi_str->proto_defaults[0], 0, sizeof(ndpi_str->proto_defaults[0])*NDPI_MAX_SUPPORTED_PROTOCOLS);

  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNRATED, NDPI_PROTOCOL_UNKNOWN,
			  "Unknown", NDPI_PROTOCOL_CATEGORY_UNSPECIFIED, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_FTP_CONTROL,
			  "FTP_CONTROL", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 21, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_FTP_DATA,
			  "FTP_DATA", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 20, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_MAIL_POP,
			  "POP3", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 110, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_MAIL_POPS,
			  "POPS", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 995, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MAIL_SMTP,
			  "SMTP", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 25, 587, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_MAIL_SMTPS,
			  "SMTPS", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 465, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_MAIL_IMAP,
			  "IMAP", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 143, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_MAIL_IMAPS,
			  "IMAPS", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 993, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DNS,
			  "DNS", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 53, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 53, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_DNS,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_DNS can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IPP,
			  "IPP", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IMO,
			  "IMO", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HTTP,
			  "HTTP", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 80, 0 /* ntop */, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_HTTP,
			      NDPI_PROTOCOL_WEBSOCKET,
			      NDPI_PROTOCOL_CROSSFIRE, NDPI_PROTOCOL_SOAP,
			      NDPI_PROTOCOL_BITTORRENT,
			      NDPI_PROTOCOL_ZATTOO,
			      NDPI_PROTOCOL_IRC,
			      NDPI_PROTOCOL_IPP,
			      NDPI_PROTOCOL_MPEGDASH,
			      NDPI_PROTOCOL_RTSP,
			      NDPI_PROTOCOL_APACHE_THRIFT,
			      NDPI_PROTOCOL_JSON_RPC,
			      NDPI_PROTOCOL_HL7,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_HTTP can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MDNS,
			  "MDNS", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5353, 5354, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_MDNS,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_MDNS can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NTP,
			  "NTP", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 123, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NETBIOS,
			  "NetBIOS", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 139, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 137, 138, 139, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NFS,
			  "NFS", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 2049, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 2049, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SSDP,
			  "SSDP", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BGP,
			  "BGP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 179, 2605, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SNMP,
			  "SNMP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 161, 162, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_XDMCP,
			  "XDMCP", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 177, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 177, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_DANGEROUS, NDPI_PROTOCOL_SMBV1,
			  "SMBv1", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 445, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SYSLOG,
			  "Syslog", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 514, 601, 6514, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 514, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DHCP,
			  "DHCP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 67, 68, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_POSTGRES,
			  "PostgreSQL", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 5432, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MYSQL,
			  "MySQL", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 3306, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NATS,
			  "Nats", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_AMONG_US,
			  "AmongUs", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 22023, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_NTOP,
			  "ntop", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_VMWARE,
			  "VMware", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 903, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 902, 903, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_POTENTIALLY_DANGEROUS, NDPI_PROTOCOL_GNUTELLA,
			  "Gnutella", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_EDONKEY,
			  "eDonkey", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BITTORRENT,
			  "BitTorrent", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 51413, 53646, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 6771, 51413, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GOOGLE,
                          "Google", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MSTEAMS_CALL,
			  "TeamsCall", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TIKTOK,
			  "TikTok", NDPI_PROTOCOL_CATEGORY_SOCIAL_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TEREDO,
			  "Teredo", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_WECHAT,
			  "WeChat", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MEMCACHED,
			  "Memcached", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 11211, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 11211, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SMBV23,
			  "SMBv23", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 445, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_MINING,
			  "Mining", CUSTOM_CATEGORY_MINING, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NEST_LOG_SINK,
			  "NestLogSink", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 11095, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MODBUS,
			  "Modbus", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 502, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WHATSAPP_CALL,
			  "WhatsAppCall", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_DATASAVER,
			  "DataSaver", NDPI_PROTOCOL_CATEGORY_WEB /* dummy */, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_SIGNAL,
			  "Signal", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DOH_DOT,
			  "DoH_DoT", NDPI_PROTOCOL_CATEGORY_NETWORK /* dummy */, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 853, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 784, 853, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_REDDIT,
			  "Reddit", NDPI_PROTOCOL_CATEGORY_SOCIAL_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WIREGUARD,
			  "WireGuard", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 51820, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_XBOX,
			  "Xbox", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_PLAYSTATION,
			  "Playstation", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_QQ,
			  "QQ", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_RTSP,
			  "RTSP", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 554, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 554, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_ICECAST,
			  "IceCast", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_CPHA,
			  "CPHA", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 8116, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_ZATTOO,
			  "Zattoo", NDPI_PROTOCOL_CATEGORY_VIDEO, NDPI_PROTOCOL_QOE_CATEGORY_LIVE_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_DISCORD,
			  "Discord", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_PLURALSIGHT,
			  "Pluralsight", NDPI_PROTOCOL_CATEGORY_VIDEO, NDPI_PROTOCOL_QOE_CATEGORY_LIVE_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_OCSP,
			  "OCSP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_VXLAN,
			  "VXLAN", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 4789, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_IRC,
			  "IRC", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 194, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 194, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MERAKI_CLOUD,
			  "MerakiCloud", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_JABBER,
			  "Jabber", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_DISNEYPLUS,
			  "DisneyPlus", NDPI_PROTOCOL_CATEGORY_STREAMING, NDPI_PROTOCOL_QOE_CATEGORY_BUFFERED_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_VRRP,
			  "VRRP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_STEAM,
			  "Steam", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MELSEC,
			  "MELSEC", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_WORLDOFWARCRAFT,
			  "WorldOfWarcraft", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_POTENTIALLY_DANGEROUS, NDPI_PROTOCOL_HOTSPOT_SHIELD,
			  "HotspotShield", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_TELNET,
			  "Telnet", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 23, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_STUN,
			  "STUN", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 3478, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 3478, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_IPSEC,
			  "IPSec", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 500, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 500, 4500, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_GRE,
			  "GRE", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_ICMP,
			  "ICMP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_IGMP,
			  "IGMP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_EGP,
			  "EGP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_PGM,
			  "PGM", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_SCTP,
			  "SCTP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_OSPF,
			  "IP_OSPF", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_IP_IN_IP,
			  "IP_in_IP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RTP,
			  "RTP", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RDP,
			  "RDP", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 3389, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 3389, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_VNC,
			  "VNC", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 5900, 5901, 5800, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TUMBLR,
			  "Tumblr", NDPI_PROTOCOL_CATEGORY_SOCIAL_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ZOOM,
			  "Zoom", NDPI_PROTOCOL_CATEGORY_VIDEO, NDPI_PROTOCOL_QOE_CATEGORY_LIVE_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WHATSAPP_FILES,
			  "WhatsAppFiles", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WHATSAPP,
			  "WhatsApp", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_TLS,
			  "TLS", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 443, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_TLS,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_TLS can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_DTLS,
			  "DTLS", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_DTLS,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_DTLS can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SSH,
			  "SSH", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 22, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_USENET,
			  "Usenet", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MGCP,
			  "MGCP", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IAX,
			  "IAX", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 4569, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 4569, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_AFP,
			  "AFP", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 548, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 548, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_HULU,
			  "Hulu", NDPI_PROTOCOL_CATEGORY_STREAMING, NDPI_PROTOCOL_QOE_CATEGORY_BUFFERED_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CHECKMK,
			  "CHECKMK", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 6556, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SIP,
			  "SIP", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports_range(ports_a, 5060, 5061, 0, 0, 0, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports_range(ports_b, 5060, 5061, 0, 0, 0, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TRUPHONE,
			  "TruPhone", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_ICMPV6,
			  "ICMPV6", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DHCPV6,
			  "DHCPV6", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_ARMAGETRON,
			  "Armagetron", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_CROSSFIRE,
			  "Crossfire", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_DOFUS,
			  "Dofus", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_GUILDWARS2,
			  "GuildWars2", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 6112, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_AMAZON_ALEXA,
			  "AmazonAlexa", NDPI_PROTOCOL_CATEGORY_VIRTUAL_ASSISTANT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_KERBEROS,
			  "Kerberos", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 88, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 88, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LDAP,
			  "LDAP", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 389, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 389, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_NEXON,
			  "Nexon", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MSSQL_TDS,
			  "MsSQL-TDS", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1433, 1434, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_PPTP,
			  "PPTP", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_IP_AH,
			  "AH", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_IP_ESP,
			  "ESP", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MS_RPCH,
			  "MS-RPCH", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NETFLOW,
			  "NetFlow", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 2055, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SFLOW,
			  "sFlow", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 6343, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HTTP_CONNECT,
			  "HTTP_Connect", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 8080, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_HTTP_CONNECT,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_HTTP_CONNECT can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HTTP_PROXY,
			  "HTTP_Proxy", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 8080, 3128, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_HTTP_PROXY,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_HTTP_PROXY can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CITRIX,
			  "Citrix", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1494, 2598, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WEBEX,
			  "Webex", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RADIUS,
			  "Radius", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1812, 1813, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 1812, 1813, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TEAMVIEWER,
			  "TeamViewer", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 5938, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5938, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HCL_NOTES,
			  "HCL_Notes", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1352, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SAP,
			  "SAP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 3201, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */); /* Missing dissector: port based only */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GTP,
			  "GTP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 2152, 2123, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GTP_C,
			  "GTP_C", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GTP_U,
			  "GTP_U", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GTP_PRIME,
			  "GTP_PRIME", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HSRP,
			  "HSRP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 1985, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WSD,
			  "WSD", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 3702, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ETHERNET_IP,
			  "EthernetIP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 44818, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TELEGRAM,
			  "Telegram", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_QUIC,
			  "QUIC", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 443, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_QUIC,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS); /* NDPI_PROTOCOL_QUIC can have (content-matched) subprotocols */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DIAMETER,
			  "Diameter", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 3868, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_APPLE_PUSH,
			  "ApplePush", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DROPBOX,
			  "Dropbox", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 17500, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_SONOS,
			  "Sonos", NDPI_PROTOCOL_CATEGORY_MUSIC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_SPOTIFY,
			  "Spotify", NDPI_PROTOCOL_CATEGORY_MUSIC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LISP,
			  "LISP", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 4342, 4341, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_EAQ,
			  "EAQ", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 6000, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_KAKAOTALK_VOICE,
			  "KakaoTalk_Voice", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_MPEGTS,
			  "MPEG_TS", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MIKROTIK,
			  "Mikrotik", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  /* http://en.wikipedia.org/wiki/Link-local_Multicast_Name_Resolution */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LLMNR,
			  "LLMNR", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 5355, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5355, 0, 0, 0, 0) /* UDP */); /* Missing dissector: port based only */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TOCA_BOCA,
			  "TocaBoca", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5055, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_H323,
			  "H323", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 1719, 1720, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 1719, 1720, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_OPENVPN,
			  "OpenVPN", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1194, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 1194, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NOE,
			  "NOE", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CISCOVPN,
			  "CiscoVPN", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 10000, 8008, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 10000, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TEAMSPEAK,
			  "TeamSpeak", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_POTENTIALLY_DANGEROUS, NDPI_PROTOCOL_TOR,
			  "Tor", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SKINNY,
			  "CiscoSkinny", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 2000, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RTCP,
			  "RTCP", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RSYNC,
			  "RSYNC", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 873, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ORACLE,
			  "Oracle", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1521, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CORBA,
			  "Corba", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CANONICAL,
			  "Canonical", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WHOIS_DAS,
			  "Whois-DAS", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 43, 4343, 0, 0, 0), /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0));    /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SD_RTN,
			  "SD-RTN", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SOCKS,
			  "SOCKS", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1080, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 1080, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TFTP,
			  "TFTP", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),   /* TCP */
			  ndpi_build_default_ports(ports_b, 69, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RTMP,
			  "RTMP", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1935, 0, 0, 0, 0), /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0));   /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_PINTEREST,
			  "Pinterest", NDPI_PROTOCOL_CATEGORY_SOCIAL_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MEGACO,
			  "Megaco", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),     /* TCP */
			  ndpi_build_default_ports(ports_b, 2944, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RESP,
			  "RESP", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 6379, 0, 0, 0, 0), /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0));   /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ZMQ,
			  "ZeroMQ", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_OSPF,
			  "OSPF", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 2604, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0));    /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_BLIZZARD,
			  "Blizzard", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 1119, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 1119, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_UBNTAC2,
			  "UBNTAC2", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),      /* TCP */
			  ndpi_build_default_ports(ports_b, 10001, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_VIBER,
			  "Viber", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 7985, 5242, 5243, 4244, 0),     /* TCP */
			  ndpi_build_default_ports(ports_b, 7985, 7987, 5242, 5243, 4244)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_COAP,
			  "COAP", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),        /* TCP */
			  ndpi_build_default_ports(ports_b, 5683, 5684, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MQTT,
			  "MQTT", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1883, 8883, 0, 0, 0), /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0));      /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SOMEIP,
			  "SOMEIP", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 30491, 30501, 0, 0, 0),      /* TCP */
			  ndpi_build_default_ports(ports_b, 30491, 30501, 30490, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RX,
			  "RX", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_GIT,
			  "Git", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 9418, 0, 0, 0, 0), /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0));   /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DRDA,
			  "DRDA", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GOOGLE_MEET,
			  "GoogleMeet", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GOOGLE_CALL,
			  "GoogleCall", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BJNP,
			  "BJNP", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 8612, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SMPP,
			  "SMPP", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_OOKLA,
			  "Ookla", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_AMQP,
			  "AMQP", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DNSCRYPT,
			  "DNScrypt", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TINC,
			  "TINC", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 655, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 655, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_FIX,
			  "FIX", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_HFT,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_NINTENDO,
			  "Nintendo", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_VALVE_SDR,
			  "SteamDatagramRelay", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_AJP,
			  "AJP", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 8009, 8010, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TARGUS_GETDATA,
			  "TargusDataspeed", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 5001, 5201, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5001, 5201, 0, 0, 0) /* UDP */); /* Missing dissector: port based only */
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_AMAZON_VIDEO,
			  "AmazonVideo", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DNP3,
			  "DNP3", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 20000, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IEC60870,
			  "IEC60870", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 2404, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BLOOMBERG,
			  "Bloomberg", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CAPWAP,
			  "CAPWAP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5246, 5247, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ZABBIX,
			  "Zabbix", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 10050, 10051, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_S7COMM,
			  "S7Comm", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_MSTEAMS,
			  "Teams", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WEBSOCKET,
			  "WebSocket", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ANYDESK,
			  "AnyDesk", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SOAP,
			  "SOAP", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MONGODB,
			  "MongoDB", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 27017, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_APPLE_SIRI,
			  "AppleSiri", NDPI_PROTOCOL_CATEGORY_VIRTUAL_ASSISTANT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SNAPCHAT_CALL,
			  "SnapchatCall", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HPVIRTGRP,
			  "HP_VIRTGRP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_GENSHIN_IMPACT,
			  "GenshinImpact", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 22102, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_ACTIVISION,
			  "Activision", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_FORTICLIENT,
			  "FortiClient", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 8013, 8014, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_Z3950,
			  "Z3950", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 210, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_LIKEE,
			  "Likee", NDPI_PROTOCOL_CATEGORY_SOCIAL_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_GITLAB,
			  "GitLab", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_AVAST_SECUREDNS,
			  "AVASTSecureDNS", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CASSANDRA,
			  "Cassandra", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 7000, 9042, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_FACEBOOK_VOIP,
			  "FacebookVoip", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SIGNAL_VOIP,
			  "SignalVoip", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MICROSOFT_AZURE,
			  "Azure", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GOOGLE_CLOUD,
			  "GoogleCloud", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TENCENT,
                          "Tencent", NDPI_PROTOCOL_CATEGORY_SOCIAL_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_RAKNET,
                          "RakNet", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0), /* TCP */
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_XIAOMI,
                          "Xiaomi", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_EDGECAST,
                          "Edgecast", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CACHEFLY,
                          "Cachefly", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SOFTETHER,
                          "Softether", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_MPEGDASH,
                          "MpegDash", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  /*
    Note: removed RSH port 514 as TCP/514 is often used for syslog and RSH is as such on;y
    if both source and destination ports are 514. So we removed the default for RSH and used with syslog
  */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_RSH,
                          "RSH", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IP_PIM,
                          "IP_PIM", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_COLLECTD,
                          "collectd", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 25826, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_I3D,
                          "i3D", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_RIOTGAMES,
                          "RiotGames", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ULTRASURF,
                          "UltraSurf", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_THREEMA,
                          "Threema", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ALICLOUD,
                          "AliCloud", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_AVAST,
                          "AVAST", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TIVOCONNECT,
                          "TiVoConnect", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 2190, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 2190, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_KISMET,
                          "Kismet", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_FASTCGI,
                          "FastCGI", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_UNSAFE, NDPI_PROTOCOL_FTPS,
                          "FTPS", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NATPMP,
                          "NAT-PMP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 5351, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_SYNCTHING,
                          "Syncthing", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_CRYNET,
                          "CryNetwork", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LINE,
			  "Line", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LINE_CALL,
			  "LineCall", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MUNIN,
                          "Munin", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 4949, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ELASTICSEARCH,
                          "Elasticsearch", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TUYA_LP,
                          "TuyaLP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 6667, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TPLINK_SHP,
                          "TPLINK_SHP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 9999, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 9999, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TAILSCALE,
			  "Tailscale", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 41641, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_SOURCE_ENGINE,
                          "Source_Engine", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 27015, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_BACNET,
                          "BACnet", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 47808, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_OICQ,
                          "OICQ", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 8000, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_HOTS,
                          "Heroes_of_the_Storm", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SRTP,
			  "SRTP", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BITCOIN,
			  "BITCOIN", NDPI_PROTOCOL_CATEGORY_CRYPTO_BLOCKCHAIN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 8333, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_PROTONVPN,
			  "ProtonVPN", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_APACHE_THRIFT,
			  "Thrift", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_ROBLOX,
			  "Roblox", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_SERVICE_LOCATION,
			  "Service_Location_Protocol", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 427, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 427, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MULLVAD,
			  "Mullvad", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_OPERA_VPN,
			  "OperaVPN", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_HTTP2,
                          "HTTP2", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_HAPROXY,
			  "HAProxy", NDPI_PROTOCOL_CATEGORY_WEB, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_RMCP,
			  "RMCP", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 623, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_CAN,
			  "Controller_Area_Network", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_PROTOBUF,
			  "Protobuf", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ETHEREUM,
			  "ETHEREUM", NDPI_PROTOCOL_CATEGORY_CRYPTO_BLOCKCHAIN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 30303, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TELEGRAM_VOIP,
			  "TelegramVoip", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TESLA_SERVICES,
			  "TeslaServices", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_PTPV2,
			  "PTPv2", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 319, 320, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HART_IP,
			  "HART-IP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 5094, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RTPS,
			  "RTPS", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 7401, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_OPC_UA,
			  "OPC-UA", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 4840, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_S7COMM_PLUS,
			  "S7CommPlus", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_FINS,
			  "FINS", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 9600, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 9600, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ETHERSIO,
			  "EtherSIO", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 6060, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_UMAS,
			  "UMAS", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BECKHOFF_ADS,
			  "BeckhoffADS", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 48898, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ISO9506_1_MMS,
			  "ISO9506-1-MMS", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IEEE_C37118,
			  "IEEE-C37118", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 4712, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 4713, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ETHERSBUS,
			  "Ether-S-Bus", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 5050, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MONERO,
			  "Monero", NDPI_PROTOCOL_CATEGORY_CRYPTO_BLOCKCHAIN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DCERPC,
			  "DCERPC", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 135, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 135, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_subprotocols(ndpi_str, NDPI_PROTOCOL_DCERPC,
			      NDPI_PROTOCOL_PROFINET_IO,
			      NDPI_PROTOCOL_MATCHED_BY_CONTENT,
			      NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_PROFINET_IO,
			  "PROFINET_IO", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HISLIP,
			  "HiSLIP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 4880, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_UFTP,
			  "UFTP", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 1044, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_OPENFLOW,
			  "OpenFlow", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 6653, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_JSON_RPC,
			  "JSON-RPC", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_WEBDAV,
			  "WebDAV", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_APACHE_KAFKA,
			  "Kafka", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 9092, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NOMACHINE,
			  "NoMachine", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 4000, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 4000, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_IEC62056,
			  "IEC62056", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 4059, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 4059, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HL7,
			  "HL7", NDPI_PROTOCOL_CATEGORY_HEALTH, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 2575, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DICOM,
			  "DICOM", NDPI_PROTOCOL_CATEGORY_HEALTH, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 104, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CEPH,
			  "Ceph", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 3300, 6789, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ROUGHTIME,
			  "Roughtime", NDPI_PROTOCOL_CATEGORY_SYSTEM_OS, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 2002, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 2002, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_KCP,
			  "KCP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_MUMBLE,
			  "Mumble", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_YOJIMBO,
			  "Yojimbo", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_STOMP,
			  "STOMP", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 61613, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RADMIN,
			  "Radmin", NDPI_PROTOCOL_CATEGORY_REMOTE_ACCESS, NDPI_PROTOCOL_QOE_CATEGORY_REMOTE_ACCESS,
			  ndpi_build_default_ports(ports_a, 4899, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RAFT,
			  "Raft", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CIP,
			  "CIP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 2222, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GEARMAN,
			  "Gearman", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 4730, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TENCENTGAMES,
			  "TencentGames", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_GAIJIN,
			  "GaijinEntertainment", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 20011, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_C1222,
			  "ANSI_C1222", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1153, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 1153, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DLEP,
			  "DLEP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 854, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 854, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BFD,
			  "BFD", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 3784, 3785, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_NETEASE_GAMES,
			  "NetEaseGames", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_PATHOFEXILE,
			  "PathofExile", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_PFCP,
			  "PFCP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 8805, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_FLUTE,
			  "FLUTE", NDPI_PROTOCOL_CATEGORY_DOWNLOAD_FT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_LOLWILDRIFT,
			  "LoLWildRift", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_TESO,
			  "TES_Online", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LDP,
			  "LDP", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 646, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 646, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_KNXNET_IP,
			  "KNXnet_IP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 3671, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 3671, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_EGD,
			  "EthernetGlobalData", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_BFCP,
			  "BFCP", NDPI_PROTOCOL_CATEGORY_VIDEO, NDPI_PROTOCOL_QOE_CATEGORY_LIVE_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_IQIYI,
			  "iQIYI", NDPI_PROTOCOL_CATEGORY_STREAMING, NDPI_PROTOCOL_QOE_CATEGORY_BUFFERED_STREAMING,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_VIBER_VOIP,
			  "ViberVoip", NDPI_PROTOCOL_CATEGORY_VOIP, NDPI_PROTOCOL_QOE_CATEGORY_VOIP_CALLS,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_COD_MOBILE,
			  "CoD_Mobile", NDPI_PROTOCOL_CATEGORY_GAME, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
 			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ZUG,
			  "ZUG", NDPI_PROTOCOL_CATEGORY_CRYPTO_BLOCKCHAIN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_JRMI,
			  "JRMI", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 1099, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_RIPE_ATLAS,
			  "RipeAtlas", NDPI_PROTOCOL_CATEGORY_NETWORK, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_FUN, NDPI_PROTOCOL_HLS,
			  "HLS", NDPI_PROTOCOL_CATEGORY_MEDIA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CLICKHOUSE,
			  "ClickHouse", NDPI_PROTOCOL_CATEGORY_DATABASE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_NANO,
			  "Nano", NDPI_PROTOCOL_CATEGORY_CRYPTO_BLOCKCHAIN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 7075, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_OPENWIRE,
			  "OpenWire", NDPI_PROTOCOL_CATEGORY_RPC, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 61616, 0, 0, 0, 0),  /* TCP */
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0)); /* UDP */
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_CNP_IP,
			  "CNP-IP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_ATG,
			  "ATG", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_TRDP,
			  "TRDP", NDPI_PROTOCOL_CATEGORY_IOT_SCADA, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 17225, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 17224, 17225, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LUSTRE,
			  "Lustre", NDPI_PROTOCOL_CATEGORY_DATA_TRANSFER, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DINGTALK,
			  "DingTalk", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_PALTALK,
			  "Paltalk", NDPI_PROTOCOL_CATEGORY_CHAT, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MS_OUTLOOK,
                          "Outlook", NDPI_PROTOCOL_CATEGORY_MAIL, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_MICROSOFT,
                          "Microsoft", NDPI_PROTOCOL_CATEGORY_CLOUD, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MICROSOFT_365,
                          "Microsoft365", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_MS_ONE_DRIVE,
                          "MS_OneDrive", NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_LAGOFAST,
                          "LagoFast", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_GEARUP_BOOSTER,
                          "GearUP_Booster", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_ONLINE_GAMING,
                          ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 1 /* cleartext */, 0 /* nw proto */, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_MSDO,
                          "MSDO", NDPI_PROTOCOL_CATEGORY_SW_UPDATE, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
                          ndpi_build_default_ports(ports_a, 7680, 0, 0, 0, 0) /* TCP */,
                          ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);
  ndpi_set_proto_defaults(ndpi_str, 0 /* encrypted */, 1 /* app proto */, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_HAMACHI,
			  "Hamachi", NDPI_PROTOCOL_CATEGORY_VPN, NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			  ndpi_build_default_ports(ports_a, 12975, 32976, 0, 0, 0) /* TCP */,
			  ndpi_build_default_ports(ports_b, 17771, 0, 0, 0, 0) /* UDP */);


#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../../nDPI-custom/custom_ndpi_main.c"
#endif

  /* calling function for host and content matched protocols */
  init_string_based_protocols(ndpi_str);
  ndpi_validate_protocol_initialization(ndpi_str);
}

/* ****************************************************** */

#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../../nDPI-custom/custom_ndpi_protocols.c"
#endif

/* ****************************************************** */

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
#define MATCH_DEBUG_INFO(fmt, ...) if(txt->option & AC_FEATURE_DEBUG) printf(fmt, ##__VA_ARGS__)
#else
#define MATCH_DEBUG_INFO(fmt, ...)
#endif

/* No static because it is used by fuzzer, too */
static int ac_domain_match_handler(AC_MATCH_t *m, AC_TEXT_t *txt, AC_REP_t *match) {
  AC_PATTERN_t *pattern;
  int i,start,end = m->position;

  if(m->match_num > 1) {
    for(pattern = m->patterns,i=0; i < m->match_num && i < 32; i++,pattern++) {
      if(!(m->match_map & (1u << i)))
        continue;
      start = end - pattern->length;
      if(pattern->rep.from_start && pattern->rep.at_end &&
        start == 0 && end == txt->length) {
        *match = pattern->rep; txt->match.last = pattern;
        MATCH_DEBUG_INFO("[NDPI] Searching: Found exact match^$. Proto %d \n",pattern->rep.number);
        return 1;
      }
    }
    for(pattern = m->patterns,i=0; i < m->match_num && i < 32; i++,pattern++) {
      if(!(m->match_map & (1u << i)))
        continue;
      start = end - pattern->length;
      if(pattern->rep.at_end &&
        start == 0 && end == txt->length) {
        *match = pattern->rep; txt->match.last = pattern;
        MATCH_DEBUG_INFO("[NDPI] Searching: Found exact match$. Proto %d \n",pattern->rep.number);
        return 1;
      }
    }
  }
  for(pattern = m->patterns,i=0; i < m->match_num && i < 32; i++,pattern++) {
    /*
     * See ac_automata_exact_match()
     * The bit is set if the pattern exactly matches AND
     * the length of the pattern is longer than that of the previous one.
     * Skip shorter (less precise) templates.
     */
    if(!(m->match_map & (1u << i)))
      continue;
    start = end - pattern->length;

    MATCH_DEBUG_INFO("[NDPI] Searching: [to search: %.*s/%u][pattern: %s%.*s%s/%u l:%u] %d-%d\n",
		     txt->length, txt->astring,(unsigned int) txt->length,
		     m->patterns[0].rep.from_start ? "^":"",
		     (unsigned int) pattern->length, pattern->astring,
		     m->patterns[0].rep.at_end ? "$":"", (unsigned int) pattern->length,m->patterns[0].rep.level,
		     start,end);

    if(start == 0 && end == txt->length) {
      *match = pattern->rep; txt->match.last = pattern;
      MATCH_DEBUG_INFO("[NDPI] Searching: Found exact match. Proto %d \n",pattern->rep.number);
      return 1;
    }
    /* pattern is DOMAIN.NAME and string x.DOMAIN.NAME ? */
    if(start >= 1 && !ndpi_is_middle_string_char(pattern->astring[0])) {
      /*
	The patch below allows in case of pattern ws.amazon.com
	to avoid matching aws.amazon.com whereas a.ws.amazon.com
	has to match
      */
      if(txt->astring[start-1] == '.') {
	if(!txt->match.last || txt->match.last->rep.level < pattern->rep.level) {
	  txt->match.last = pattern; *match = pattern->rep;
	  MATCH_DEBUG_INFO("[NDPI] Searching: Found domain match (pre). Proto %d \n",pattern->rep.number);
	}
      }
      continue;
    }

    /* pattern is -DOMAIN.NAME and string x-DOMAIN.NAME ? */
    if(start >= 1 && pattern->astring[0] == '-') {
      if(txt->astring[start] == '-') {
	if(!txt->match.last || txt->match.last->rep.level < pattern->rep.level) {
	  txt->match.last = pattern; *match = pattern->rep;
	  MATCH_DEBUG_INFO("[NDPI] Searching: Found domain match (pre -). Proto %d \n",pattern->rep.number);
	}
      }
      continue;
    }

    /* pattern is DOMAIN. and string DOMAIN.SOMETHING ? or
       DOMAIN- and DOMAIN-SOMETHING */
    if(start == 0 && ndpi_is_middle_string_char(pattern->astring[pattern->length - 1])) {
      if(!txt->match.last || txt->match.last->rep.level < pattern->rep.level) {
	txt->match.last = pattern; *match = pattern->rep;
	MATCH_DEBUG_INFO("[NDPI] Searching: Found domain match (post). Proto %d \n",pattern->rep.number);
      }
      continue;
    }
  }
  return 0;
}

/* ******************************************************************** */

u_int16_t ndpi_patricia_get_maxbits(ndpi_patricia_tree_t *tree) {
  if(!tree)
    return 0;
  return(tree->maxbits);
}

/* ******************************************************************** */

void ndpi_patricia_get_stats(ndpi_patricia_tree_t *tree, struct ndpi_patricia_tree_stats *stats) {
  if(tree) {
    stats->n_search = tree->stats.n_search;
    stats->n_found = tree->stats.n_found;
  } else {
    stats->n_search = 0;
    stats->n_found = 0;
  }
}

/* ******************************************************************** */

int ndpi_get_patricia_stats(struct ndpi_detection_module_struct *ndpi_struct,
			    ptree_type ptree_type,
			    struct ndpi_patricia_tree_stats *stats) {
  if(!ndpi_struct || !stats)
    return -1;

  switch(ptree_type) {
  case NDPI_PTREE_RISK_MASK:
    if(!ndpi_struct->ip_risk_mask)
      return -1;
    ndpi_patricia_get_stats(ndpi_struct->ip_risk_mask->v4, stats);
    return 0;

  case NDPI_PTREE_RISK_MASK6:
    if(!ndpi_struct->ip_risk_mask)
      return -1;
    ndpi_patricia_get_stats(ndpi_struct->ip_risk_mask->v6, stats);
    return 0;

  case NDPI_PTREE_RISK:
    if(!ndpi_struct->ip_risk)
      return -1;
    ndpi_patricia_get_stats(ndpi_struct->ip_risk->v4, stats);
    return 0;

  case NDPI_PTREE_RISK6:
    if(!ndpi_struct->ip_risk)
      return -1;
    ndpi_patricia_get_stats(ndpi_struct->ip_risk->v6, stats);
    return 0;

  case NDPI_PTREE_PROTOCOLS:
    if(!ndpi_struct->protocols)
      return -1;
    ndpi_patricia_get_stats(ndpi_struct->protocols->v4, stats);
    return 0;

  case NDPI_PTREE_PROTOCOLS6:
    if(!ndpi_struct->protocols)
      return -1;
    ndpi_patricia_get_stats(ndpi_struct->protocols->v6, stats);
    return 0;

  default:
    return -1;
  }
}

/* ****************************************************** */

int ndpi_fill_prefix_v4(ndpi_prefix_t *p, const struct in_addr *a, int b, int mb) {
  memset(p, 0, sizeof(ndpi_prefix_t));

  if(b < 0 || b > mb)
    return(-1);

  p->add.sin.s_addr = a->s_addr, p->family = AF_INET, p->bitlen = b, p->ref_count = 0;

  return(0);
}

/* ******************************************* */

int ndpi_fill_prefix_v6(ndpi_prefix_t *prefix, const struct in6_addr *addr, int bits, int maxbits) {
  memset(prefix, 0, sizeof(ndpi_prefix_t));

  if(bits < 0 || bits > maxbits)
    return -1;

  memcpy(&prefix->add.sin6, addr, (maxbits + 7) / 8);
  prefix->family = AF_INET6, prefix->bitlen = bits, prefix->ref_count = 0;

  return 0;
}

/* ******************************************* */

int ndpi_fill_prefix_mac(ndpi_prefix_t *prefix, u_int8_t *mac, int bits, int maxbits) {
  memset(prefix, 0, sizeof(ndpi_prefix_t));

  if(bits < 0 || bits > maxbits)
    return -1;

  memcpy(prefix->add.mac, mac, 6);
  prefix->family = AF_MAC, prefix->bitlen = bits, prefix->ref_count = 0;

  return 0;
}

/* ******************************************* */

ndpi_prefix_t *ndpi_patricia_get_node_prefix(ndpi_patricia_node_t *node) {
  return(node->prefix);
}

/* ******************************************* */

u_int16_t ndpi_patricia_get_node_bits(ndpi_patricia_node_t *node) {
  return(node->bit);
}

/* ******************************************* */

void ndpi_patricia_set_node_data(ndpi_patricia_node_t *node, void *data) {
  node->data = data;
}

/* ******************************************* */

void *ndpi_patricia_get_node_data(ndpi_patricia_node_t *node) {
  return(node->data);
}

/* ******************************************* */

void ndpi_patricia_set_node_u64(ndpi_patricia_node_t *node, u_int64_t value) {
  node->value.u.uv64 = value;
}

/* ******************************************* */

u_int64_t ndpi_patricia_get_node_u64(ndpi_patricia_node_t *node) {
  return(node->value.u.uv64);
}

/* ******************************************* */

NDPI_STATIC u_int8_t ndpi_is_public_ipv4(u_int32_t a /* host byte order */) {
  if(   ((a & 0xFF000000) == 0x0A000000 /* 10.0.0.0/8 */)
	|| ((a & 0xFFF00000) == 0xAC100000 /* 172.16.0.0/12 */)
	|| ((a & 0xFFFF0000) == 0xC0A80000 /* 192.168.0.0/16 */)
	|| ((a & 0xFF000000) == 0x7F000000 /* 127.0.0.0/8 */)
	|| ((a & 0xF0000000) == 0xE0000000 /* 224.0.0.0/4 */)
	)
    return(0);
  else
    return(1);
}

/* ******************************************* */

u_int16_t ndpi_network_ptree_match(struct ndpi_detection_module_struct *ndpi_str,
                                   struct in_addr *pin /* network byte order */) {
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(!ndpi_str || !ndpi_str->protocols)
    return(NDPI_PROTOCOL_UNKNOWN);

  if(ndpi_str->ndpi_num_custom_protocols == 0) {
    /*
      In case we don't have defined any custom protocol we check the ptree
      only in case of public IP addresses as in ndpi_content_match.c.inc
      we only have public IP addresses. Instead with custom protocols, users
      might have defined private protocols hence we should not skip
      the checks below
    */

    if(ndpi_is_public_ipv4(ntohl(pin->s_addr)) == 0)
      return(NDPI_PROTOCOL_UNKNOWN); /* Non public IP */
  }

  /* Make sure all in network byte order otherwise compares wont work */
  ndpi_fill_prefix_v4(&prefix, pin, 32,
		      ((ndpi_patricia_tree_t *) ndpi_str->protocols->v4)->maxbits);
  node = ndpi_patricia_search_best(ndpi_str->protocols->v4, &prefix);

  return(node ? node->value.u.uv16[0].user_value : NDPI_PROTOCOL_UNKNOWN);
}

/* ******************************************* */

u_int16_t ndpi_network_ptree6_match(struct ndpi_detection_module_struct *ndpi_str,
				    struct in6_addr *pin) {
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(!ndpi_str || !ndpi_str->protocols)
    return(NDPI_PROTOCOL_UNKNOWN);

  /* Make sure all in network byte order otherwise compares wont work */
  ndpi_fill_prefix_v6(&prefix, pin, 128,
		      ((ndpi_patricia_tree_t *) ndpi_str->protocols->v4)->maxbits);
  node = ndpi_patricia_search_best(ndpi_str->protocols->v4, &prefix);

  return(node ? node->value.u.uv16[0].user_value : NDPI_PROTOCOL_UNKNOWN);
}

/* ******************************************* */

u_int16_t ndpi_network_port_ptree_match(struct ndpi_detection_module_struct *ndpi_str,
					struct in_addr *pin /* network byte order */,
					u_int16_t port /* network byte order */) {
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(!ndpi_str || !ndpi_str->protocols)
    return(NDPI_PROTOCOL_UNKNOWN);

  if(ndpi_str->ndpi_num_custom_protocols == 0) {
    /*
      In case we don't have defined any custom protocol we check the ptree
      only in case of public IP addresses as in ndpi_content_match.c.inc
      we only have public IP addresses. Instead with custom protocols, users
      might have defined private protocols hence we should not skip
      the checks below
    */

    if(ndpi_is_public_ipv4(ntohl(pin->s_addr)) == 0)
      return(NDPI_PROTOCOL_UNKNOWN); /* Non public IP */
  }

  /* Make sure all in network byte order otherwise compares wont work */
  ndpi_fill_prefix_v4(&prefix, pin, 32,
		      ((ndpi_patricia_tree_t *) ndpi_str->protocols->v4)->maxbits);
  node = ndpi_patricia_search_best(ndpi_str->protocols->v4, &prefix);

  if(node) {
    int i;
    struct patricia_uv16_list *item;

    for(i=0; i<UV16_MAX_USER_VALUES; i++) {
      if((node->value.u.uv16[i].additional_user_value == 0)
	 || (node->value.u.uv16[i].additional_user_value == port))
	return(node->value.u.uv16[i].user_value);
    }

    /*
      If we're here it means that we don't have
      enough room for our custom value so we need
      to check the custom_user_data pointer.
    */
    item = (struct patricia_uv16_list*)node->data;

    while(item != NULL) {
      if(item->value.additional_user_value == port)
	return(item->value.user_value);
      else
	item = item->next;
    }
  }

  return(NDPI_PROTOCOL_UNKNOWN);
}

/* ******************************************* */

u_int16_t ndpi_network_port_ptree6_match(struct ndpi_detection_module_struct *ndpi_str,
					 struct in6_addr *pin,
					 u_int16_t port /* network byte order */)
{
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(!ndpi_str || !ndpi_str->protocols)
    return(NDPI_PROTOCOL_UNKNOWN);

  /* TODO: check on "private" addresses? */

  /* Make sure all in network byte order otherwise compares wont work */
  ndpi_fill_prefix_v6(&prefix, pin, 128,
		      ((ndpi_patricia_tree_t *) ndpi_str->protocols->v6)->maxbits);
  node = ndpi_patricia_search_best(ndpi_str->protocols->v6, &prefix);

  if(node) {
    int i;
    struct patricia_uv16_list *item;

    for(i=0; i<UV16_MAX_USER_VALUES; i++) {
      if((node->value.u.uv16[i].additional_user_value == 0)
	 || (node->value.u.uv16[i].additional_user_value == port))
	return(node->value.u.uv16[i].user_value);
    }

    /*
      If we're here it means that we don't have
      enough room for our custom value so we need
      to check the custom_user_data pointer.
    */
    item = (struct patricia_uv16_list*)node->data;

    while(item != NULL) {
      if(item->value.additional_user_value == port)
	return(item->value.user_value);
      else
	item = item->next;
    }
  }

  return(NDPI_PROTOCOL_UNKNOWN);
}

/* ******************************************* */

NDPI_STATIC ndpi_risk_enum ndpi_network_risk_ptree_match(struct ndpi_detection_module_struct *ndpi_str,
					     struct in_addr *pin /* network byte order */) {
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(!ndpi_str || !ndpi_str->ip_risk)
    return(NDPI_NO_RISK);

  /* Make sure all in network byte order otherwise compares wont work */
  ndpi_fill_prefix_v4(&prefix, pin, 32,
		      ((ndpi_patricia_tree_t *) ndpi_str->ip_risk->v4)->maxbits);
  node = ndpi_patricia_search_best(ndpi_str->ip_risk->v4, &prefix);

  if(node)
    return((ndpi_risk_enum)node->value.u.uv16[0].user_value);

  return(NDPI_NO_RISK);
}

/* ******************************************* */

NDPI_STATIC ndpi_risk_enum ndpi_network_risk_ptree_match6(struct ndpi_detection_module_struct *ndpi_str,
					      struct in6_addr *pin) {
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  /* Make sure all in network byte order otherwise compares wont work */
  ndpi_fill_prefix_v6(&prefix, pin, 128,
		      ((ndpi_patricia_tree_t *) ndpi_str->ip_risk->v6)->maxbits);
  node = ndpi_patricia_search_best(ndpi_str->ip_risk->v6, &prefix);

  if(node)
    return((ndpi_risk_enum)node->value.u.uv16[0].user_value);

  return(NDPI_NO_RISK);
}

/* ******************************************* */

static ndpi_patricia_node_t* add_to_ptree(ndpi_patricia_tree_t *tree, int family, void *addr, int bits) {
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(family == AF_INET)
    ndpi_fill_prefix_v4(&prefix, (struct in_addr *) addr, bits, tree->maxbits);
  else
    ndpi_fill_prefix_v6(&prefix, (struct in6_addr *) addr, bits, tree->maxbits);

  node = ndpi_patricia_lookup(tree, &prefix);
  /* if(node) memset(&node->value, 0, sizeof(node->value)); */

  return(node);
}

/* ******************************************* */

#ifndef __KERNEL__
/*
  Load a file containing IPv4 OR IPv6 addresses in CIDR format as 'protocol_id'

  Return: the number of entries loaded or -1 in case of error
*/
int ndpi_load_ptree_file(ndpi_ptree_t *ptree,
			 const char *path,
			 u_int16_t protocol_id) {
  char buffer[1024], *line, *addr, *cidr, *saveptr;
  FILE *fd;
  int len;
  u_int num_loaded = 0;

  if( !path || !ptree)
    return(-1);

  fd = fopen(path, "r");

  if(fd == NULL) {
    /* NDPI_LOG_ERR(NULL, "Unable to open file %s [%s]\n", path, strerror(errno)); */
    return(-1);
  }

  while(1) {
    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    len = strlen(line);

    if((len <= 1) || (line[0] == '#'))
      continue;

    line[len - 1] = '\0';
    addr = strtok_r(line, "/", &saveptr);

    if(addr) {
      ndpi_patricia_node_t *node;
      bool is_ipv4 = strchr(addr, ':') ? false : true;

      cidr = strtok_r(NULL, "\n", &saveptr);

      if(is_ipv4)  {
	struct in_addr addr4;

	addr4.s_addr = inet_addr(addr);

	/* printf("+ %s/%d\n", addr, cidr ? atoi(cidr) : 32); */
	node = add_to_ptree(ptree->v4, AF_INET, &addr4, cidr ? atoi(cidr) : 32 /* bits */);
      } else {
	struct in6_addr addr6;

	if(inet_pton(AF_INET6, addr, &addr6) == 1)
	  node = add_to_ptree(ptree->v6, AF_INET6, &addr6, cidr ? atoi(cidr) : 128);
	else
	  node = NULL;
      }

      if(node != NULL) {
	u_int i, found = 0;

	for(i=0; i<UV16_MAX_USER_VALUES; i++) {
	  if(node->value.u.uv16[i].user_value == 0) {
	    node->value.u.uv16[i].user_value = protocol_id,
	      node->value.u.uv16[i].additional_user_value = 0 /* port */;
	    found = 1;
	    break;
	  }
	}

	if(found)
	  num_loaded++;
      }
    }
  }

  fclose(fd);
  return(num_loaded);
}

/* ******************************************* */

/*
  Load a file containing IPv4 addresses in CIDR format as 'protocol_id'

  Return: the number of entries loaded or -1 in case of error
*/
int ndpi_load_ipv4_ptree(struct ndpi_detection_module_struct *ndpi_str,
			 const char *path, u_int16_t protocol_id) {
  if(!ndpi_str)
    return -1;

  return(ndpi_load_ptree_file(ndpi_str->protocols, path, protocol_id));
}
#endif

/* ******************************************* */

static void ndpi_init_ptree_ipv4(ndpi_patricia_tree_t *ptree, ndpi_network host_list[]) {
  int i;

  for(i = 0; host_list[i].network != 0x0; i++) {
    struct in_addr pin;
    ndpi_patricia_node_t *node;

    pin.s_addr = htonl(host_list[i].network);
    if((node = add_to_ptree(ptree, AF_INET, &pin, host_list[i].cidr /* bits */)) != NULL) {
      /*
	Two main cases:
	1) ip -> protocol: uv16[0].user_value = protocol; uv16[0].additional_user_value = 0;
	2) ip -> risk: uv16[0].user_value = risk; uv16[0].additional_user_value = 0;
      */
      node->value.u.uv16[0].user_value = host_list[i].value, node->value.u.uv16[0].additional_user_value = 0;
    }
  }
}

/* ******************************************* */

static void ndpi_init_ptree_ipv6(struct ndpi_detection_module_struct *ndpi_str,
				 ndpi_patricia_tree_t *ptree, ndpi_network6 host_list[]) {
  int i;

  for(i = 0; host_list[i].network != NULL; i++) {
    int rc;
    struct in6_addr pin;
    ndpi_patricia_node_t *node;

    rc = inet_pton(AF_INET6, host_list[i].network, &pin);
    if (rc != 1) {
      NDPI_LOG_ERR(ndpi_str, "Invalid ipv6 address [%s]: %d\n", host_list[i].network, rc);
      continue;
    }

    if((node = add_to_ptree(ptree, AF_INET6, &pin, host_list[i].cidr /* bits */)) != NULL) {
      node->value.u.uv16[0].user_value = host_list[i].value, node->value.u.uv16[0].additional_user_value = 0;
    }
  }
}

/* ******************************************* */

static int ndpi_add_host_ip_subprotocol(struct ndpi_detection_module_struct *ndpi_str,
					char *value, u_int16_t protocol_id,
					u_int8_t is_ipv6) {
  ndpi_patricia_node_t *node;
  struct in_addr pin;
  struct in6_addr pin6;
  int bits = 32;
  char *ptr = strrchr(value, '/');
  u_int16_t port = 0; /* Format ip:8.248.73.247 */
                      /* Format ipv6:[fe80::76ac:b9ff:fe6c:c124]/64 */
  char *double_column = NULL;
  bool value_ready = false;
#ifndef __KERNEL__
  struct addrinfo hints, *result, *rp;
#endif

  if(value[0] == '[') {
    is_ipv6 = 1;
    bits = 128;
    value += 1;
  }

  if(ptr) {
    ptr[0] = '\0';
    ptr++;

    if((double_column = strrchr(ptr, ':')) != NULL) {
      double_column[0] = '\0';
      port = atoi(&double_column[1]);
    }

    if(!is_ipv6) {
      if(atoi(ptr) >= 0 && atoi(ptr) <= 32)
        bits = atoi(ptr);
    } else {
      if(atoi(ptr) >= 0 && atoi(ptr) <= 128)
        bits = atoi(ptr);

      ptr = strrchr(value, ']');
      if(ptr)
        *ptr = '\0';
    }
  } else {
    /*
      Let's check if there is the port defined

      Example: ip:8.248.73.247:443@AmazonPrime
      Example: ipv6:[fe80::76ac:b9ff:fe6c:c124]:36818@CustomProtocolF
    */
    if(!is_ipv6) {
      double_column = strrchr(value, ':');
    } else {
      ptr = strrchr(value, ']');
      if(ptr) {
        double_column = strrchr(ptr, ':');
        *ptr = '\0';
      }
    }

    if(double_column) {
      double_column[0] = '\0';
      port = atoi(&double_column[1]);
    }
  }

#ifndef __KERNEL__
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;
#endif

  if(!is_ipv6 && ndpi_str->protocols) {
    /* Check if the IP address is symbolic or numeric */
#ifndef __KERNEL__
    unsigned int d[4];
    char tail[16] = { '\0' };
    int c = sscanf(value, "%3u.%3u.%3u.%3u%15s",
		   &d[0], &d[1], &d[2], &d[3], tail);

    if ((c != 4) || tail[0]) {
      /* This might be a symbolic IPv4 address */

      if(getaddrinfo(value, NULL, &hints, &result) != 0)
	return(-1);

      for(rp = result; rp != NULL; rp = rp->ai_next) {
	if(rp->ai_family == AF_INET) {
	  struct sockaddr_in *addr = (struct sockaddr_in *)rp->ai_addr;

	  memcpy(&pin, &(addr->sin_addr), sizeof(struct in_addr));
	  value_ready = true;
          bits = 32;
	  break;
	}
      }

      freeaddrinfo(result);
    }
#endif
    if(!value_ready) {
      if(inet_pton(AF_INET, value, &pin) != 1)
	return(-1);
    }

    node = add_to_ptree(ndpi_str->protocols->v4, AF_INET, &pin, bits);
#ifndef __KERNEL__
  } else if(is_ipv6 && ndpi_str->protocols) {
    if(strchr(value, ':') == NULL) {
      /* This might be a symbolic IPv6 address */

      if(getaddrinfo(value, NULL, &hints, &result) != 0)
	return(-1);

      for(rp = result; rp != NULL; rp = rp->ai_next) {
	if(rp->ai_family == AF_INET6) {
	  struct sockaddr_in6 *addr = (struct sockaddr_in6 *)rp->ai_addr;

	  memcpy(&pin6, &(addr->sin6_addr), sizeof(struct in6_addr));
	  value_ready = true;
          bits = 128;
	  break;
	}
      }

      freeaddrinfo(result);
    }
#endif
    if(!value_ready) {
      if(inet_pton(AF_INET6, value, &pin6) != 1)
	return(-1);
    }

    node = add_to_ptree(ndpi_str->protocols->v6, AF_INET6, &pin6, bits);
  } else {
    return(-1);
  }

  if(node != NULL) {
    int i;
    struct patricia_uv16_list *item;

    for(i=0; i<UV16_MAX_USER_VALUES; i++) {
      if(node->value.u.uv16[i].user_value == 0) {
	node->value.u.uv16[i].user_value = protocol_id, node->value.u.uv16[i].additional_user_value = htons(port);
	return(0);
      }
    } /* for */

    /*
      If we're here it means that we don't have
      enough room for our custom value
    */
    item = (struct patricia_uv16_list*)ndpi_malloc(sizeof(struct patricia_uv16_list));

    if(item != NULL) {
      item->value.user_value = protocol_id,
	item->value.additional_user_value = htons(port),
	item->next = (struct patricia_uv16_list*)node->data;

      node->data = item;

      return(0);
    }

    return(-1); /* All slots are full */
  }

  return(0);
}

void set_ndpi_ticks_per_second(u_int32_t ticks_per_second) {
    _ticks_per_second = ticks_per_second;
}

void set_ndpi_flow_malloc(void* (*__ndpi_flow_malloc)(size_t size))
{
  _ndpi_flow_malloc = __ndpi_flow_malloc;
}

void set_ndpi_flow_free(void (*__ndpi_flow_free)(void *ptr)) {
  _ndpi_flow_free = __ndpi_flow_free;
}

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
void ndpi_debug_printf(u_int16_t proto, struct ndpi_detection_module_struct *ndpi_str, ndpi_log_level_t log_level,
                       const char *file_name, const char *func_name, unsigned int line_number, const char *format, ...) {
  va_list args;
#define MAX_STR_LEN 250
  char str[MAX_STR_LEN];
  if(ndpi_str != NULL && log_level > NDPI_LOG_ERROR && proto > 0 && proto < NDPI_MAX_SUPPORTED_PROTOCOLS &&
     !NDPI_ISSET(&ndpi_str->cfg.debug_bitmask, proto))
    return;
  va_start(args, format);
  ndpi_vsnprintf(str, sizeof(str) - 1, format, args);
  va_end(args);

  /* While fuzzing, we want to test log code, but we don't want to log anything! */
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
  if(ndpi_str != NULL || (file_name != NULL && func_name != NULL)) {
    printf("%s:%s:%-3d - [%u]: %s", file_name, func_name, line_number, proto, str);
  } else {
    printf("Proto: %u, %s", proto, str);
  }
#else
  (void)file_name;
  (void)func_name;
  (void)line_number;
#endif

}
#endif

/* ****************************************** */

void set_ndpi_debug_function(struct ndpi_detection_module_struct *ndpi_str, ndpi_debug_function_ptr ndpi_debug_printf) {
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  if(ndpi_str)
    ndpi_str->ndpi_debug_printf = ndpi_debug_printf;
#else
  (void)ndpi_str;
  (void)ndpi_debug_printf;
#endif
}

/* ****************************************** */

#ifndef __KERNEL__
/* Keep it in order and in sync with ndpi_protocol_category_t in ndpi_typedefs.h */
static const char *categories[NDPI_PROTOCOL_NUM_CATEGORIES] = {
  "Unspecified",
  "Media",
  "VPN",
  "Email",
  "DataTransfer",
  "Web",
  "SocialNetwork",
  "Download",
  "Game",
  "Chat",
  "VoIP",
  "Database",
  "RemoteAccess",
  "Cloud",
  "Network",
  "Collaborative",
  "RPC",
  "Streaming",
  "System",
  "SoftwareUpdate",
  "",
  "",
  "",
  "",
  "",
  "Music",
  "Video",
  "Shopping",
  "Productivity",
  "FileSharing",
  "ConnCheck",
  "IoT-Scada",
  "VirtAssistant",
  "Cybersecurity",
  "AdultContent",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "Mining", /* 99 */
  "Malware",
  "Advertisement",
  "Banned_Site",
  "Site_Unavailable",
  "Allowed_Site",
  "Antimalware",
  "Crypto_Currency",
  "Gambling",
  "Health",
  "ArtifIntelligence",
  "Finance",
  "News",
  "Sport",
  "Business",
  "Internet",
  "BlockChain/Cypto",
  "Blog/Forum",
  "Government",
  "Education",
  "CNR/Proxy",
  "Hw/Sw",
  "Dating",
  "Travel",
  "Food",
  "Bots",
  "Scanners"
};

#if !defined(NDPI_CFFI_PREPROCESSING) && defined(__linux__)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(categories) / sizeof(char *) == NDPI_PROTOCOL_NUM_CATEGORIES,
               "Invalid categories length. Do you need to update 'categories' array or 'ndpi_protocol_category_t'?");
#endif
#endif
#endif /* kernel */

/* *********************************************** */

static void free_ptree_data(void *data) {
#ifndef __KERNEL__
  struct patricia_uv16_list *item = (struct patricia_uv16_list *)data;

  while(item != NULL) {
    struct patricia_uv16_list *next = item->next;

    ndpi_free(item);
    item = next;
  }
#else
    ndpi_free(data);
#endif
}

struct ndpi_global_context *ndpi_global_init(void) {
#ifndef USE_GLOBAL_CONTEXT
  return NULL;
#else
  struct ndpi_global_context *g_ctx = ndpi_calloc(1, sizeof(struct ndpi_global_context));

  if(g_ctx == NULL)
    return(NULL);

  /* Global caches (if any) are initialized during the initialization
     of the local context(s) */

  /*  Note that we don't have yet an easy way to log from this function */

  return g_ctx;
#endif
}

/* ******************************************************************** */

void ndpi_global_deinit(struct ndpi_global_context *g_ctx) {

  /*  Note that we don't have yet an easy way to log from this function */

  if(g_ctx) {

    /* Global caches are freed here, so that we are able to get statistics even
       after the uninitialization of all the local contexts */

    if(g_ctx->ookla_global_cache)
      ndpi_lru_free_cache(g_ctx->ookla_global_cache);
    if(g_ctx->bittorrent_global_cache)
      ndpi_lru_free_cache(g_ctx->bittorrent_global_cache);
    if(g_ctx->stun_global_cache)
      ndpi_lru_free_cache(g_ctx->stun_global_cache);
    if(g_ctx->tls_cert_global_cache)
      ndpi_lru_free_cache(g_ctx->tls_cert_global_cache);
    if(g_ctx->mining_global_cache)
      ndpi_lru_free_cache(g_ctx->mining_global_cache);
    if(g_ctx->msteams_global_cache)
      ndpi_lru_free_cache(g_ctx->msteams_global_cache);
    if(g_ctx->fpc_dns_global_cache)
      ndpi_lru_free_cache(g_ctx->fpc_dns_global_cache);
    if(g_ctx->signal_global_cache)
      ndpi_lru_free_cache(g_ctx->signal_global_cache);

    ndpi_free(g_ctx);
  }
}

/* ******************************************************************** */

struct ndpi_detection_module_struct *ndpi_init_detection_module(struct ndpi_global_context *g_ctx) {
  struct ndpi_detection_module_struct *ndpi_str = ndpi_malloc(sizeof(struct ndpi_detection_module_struct));
  int i;

  if(ndpi_str == NULL) {
    /* Logging this error is a bit tricky. At this point, we can't use NDPI_LOG*
       functions yet, we don't have a custom log function and, as a library,
       we shouldn't use stdout/stderr. Since this error is quite unlikely,
       simply avoid any logs at all */
    return(NULL);
  }

#ifdef WIN32
  /* Required to use getaddrinfo on Windows */
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

  memset(ndpi_str, 0, sizeof(struct ndpi_detection_module_struct));

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  #ifndef __KERNEL__
  set_ndpi_debug_function(ndpi_str, (ndpi_debug_function_ptr) ndpi_debug_printf);
  #else
  set_ndpi_debug_function(ndpi_str, ndpi_debug_print_init);
  #endif
#endif /* NDPI_ENABLE_DEBUG_MESSAGES */
  ndpi_str->cfg.log_level =  ndpi_debug_level_init;

  if((ndpi_str->protocols = ndpi_ptree_create()) == NULL) {
    NDPI_LOG_ERR(ndpi_str, "[NDPI] Error allocating tree\n");
    ndpi_exit_detection_module(ndpi_str);
    return NULL;
  }

  ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, host_protocol_list);
  ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, host_protocol_list_6);

  ndpi_str->ip_risk_mask = ndpi_ptree_create();

  ndpi_str->g_ctx = g_ctx;
  set_default_config(&ndpi_str->cfg);

  NDPI_BITMASK_SET_ALL(ndpi_str->detection_bitmask);
  ndpi_str->user_data = NULL;

  ndpi_str->ticks_per_second = _ticks_per_second; /* ndpi_str->ticks_per_second */
  ndpi_str->tcp_max_retransmission_window_size = NDPI_DEFAULT_MAX_TCP_RETRANSMISSION_WINDOW_SIZE;

  ndpi_str->ndpi_num_supported_protocols = NDPI_MAX_SUPPORTED_PROTOCOLS;
  ndpi_str->ndpi_num_custom_protocols = 0;

  spin_lock_init(&ndpi_str->host_automa_lock);
  ndpi_str->host_automa.ac_automa = ac_automata_init(ac_domain_match_handler);
  if(!ndpi_str->host_automa.ac_automa) {
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }

  ndpi_str->host_risk_mask_automa.ac_automa = ac_automata_init(ac_domain_match_handler);
  if(!ndpi_str->host_risk_mask_automa.ac_automa) {
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }

  ndpi_str->common_alpns_automa.ac_automa = ac_automata_init(ac_domain_match_handler);
  if(!ndpi_str->common_alpns_automa.ac_automa) {
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }

  load_common_alpns(ndpi_str);

  ndpi_str->tls_cert_subject_automa.ac_automa = ac_automata_init(NULL);
  if(!ndpi_str->tls_cert_subject_automa.ac_automa) {
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }

  ndpi_str->malicious_ja4_hashmap = NULL;   /* Initialized on demand */
  ndpi_str->malicious_sha1_hashmap = NULL;  /* Initialized on demand */

  ndpi_load_tcp_fingerprints(ndpi_str);
  ndpi_str->risky_domain_automa.ac_automa = NULL; /* Initialized on demand */
  ndpi_str->trusted_issuer_dn = NULL;

#ifndef __KERNEL__
  ndpi_str->custom_categories.sc_hostnames        = ndpi_domain_classify_alloc();
  if(!ndpi_str->custom_categories.sc_hostnames) {
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }
  ndpi_str->custom_categories.sc_hostnames_shadow = ndpi_domain_classify_alloc();
  if(!ndpi_str->custom_categories.sc_hostnames_shadow) {
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }
#endif

  ndpi_str->custom_categories.ipAddresses = ndpi_patricia_new(32 /* IPv4 */);
  ndpi_str->custom_categories.ipAddresses_shadow = ndpi_patricia_new(32 /* IPv4 */);
  ndpi_str->custom_categories.ipAddresses6 = ndpi_patricia_new(128 /* IPv6 */);
  ndpi_str->custom_categories.ipAddresses6_shadow = ndpi_patricia_new(128 /* IPv6 */);

  if(ndpi_str->host_automa.ac_automa)
    ac_automata_feature(ndpi_str->host_automa.ac_automa,AC_FEATURE_LC);

  if(ndpi_str->tls_cert_subject_automa.ac_automa)
    ac_automata_feature(ndpi_str->tls_cert_subject_automa.ac_automa,AC_FEATURE_LC);

  if(ndpi_str->host_risk_mask_automa.ac_automa)
    ac_automata_feature(ndpi_str->host_risk_mask_automa.ac_automa,AC_FEATURE_LC);

  if(ndpi_str->common_alpns_automa.ac_automa)
    ac_automata_feature(ndpi_str->common_alpns_automa.ac_automa,AC_FEATURE_LC);

  /* ahocorasick debug */
  /* Needed ac_automata_enable_debug(1) for show debug */
  if(ndpi_str->host_automa.ac_automa)
    ac_automata_name(ndpi_str->host_automa.ac_automa,"host",AC_FEATURE_DEBUG);

  if(ndpi_str->tls_cert_subject_automa.ac_automa)
    ac_automata_name(ndpi_str->tls_cert_subject_automa.ac_automa,"tls_cert",AC_FEATURE_DEBUG);

  if(ndpi_str->host_risk_mask_automa.ac_automa)
    ac_automata_name(ndpi_str->host_risk_mask_automa.ac_automa,"risk_mask",AC_FEATURE_DEBUG);

  if(ndpi_str->common_alpns_automa.ac_automa)
    ac_automata_name(ndpi_str->common_alpns_automa.ac_automa,"alpns",AC_FEATURE_DEBUG);

  if((ndpi_str->custom_categories.ipAddresses == NULL) || (ndpi_str->custom_categories.ipAddresses_shadow == NULL) ||
     (ndpi_str->custom_categories.ipAddresses6 == NULL) || (ndpi_str->custom_categories.ipAddresses6_shadow == NULL)) {
    NDPI_LOG_ERR(ndpi_str, "[NDPI] Error allocating Patricia trees\n");
    ndpi_exit_detection_module(ndpi_str);
    return(NULL);
  }

  for(i = 0; i < NUM_CUSTOM_CATEGORIES; i++)
    ndpi_snprintf(ndpi_str->custom_category_labels[i], CUSTOM_CATEGORY_LABEL_LEN, "User custom category %u",
		  (unsigned int) (i + 1));

/*  if(ndpi_validate_protocol_initialization(ndpi_str)) {
	ndpi_exit_detection_module(ndpi_str);
	return NULL;
  } */
  return(ndpi_str);
}

/* *********************************************** */
static void *ac_automa_list[7];
void **ndpi_get_automata(struct ndpi_detection_module_struct *ndpi_str) {
  ac_automa_list[0] = ndpi_str->host_automa.ac_automa;
  ac_automa_list[1] = ndpi_str->tls_cert_subject_automa.ac_automa;
  ac_automa_list[2] = NULL;
  ac_automa_list[3] = NULL;
  ac_automa_list[4] = ndpi_str->risky_domain_automa.ac_automa;
  ac_automa_list[5] = (void *)1;
  return ac_automa_list;
}

/*
  This function adds some exceptions for popular domain names
  in order to avoid "false" positives and avoid polluting
  results
*/
static void ndpi_add_domain_risk_exceptions(struct ndpi_detection_module_struct *ndpi_str) {
  const char *domains[] = {
    ".local",
    ".work",
    /* DGA's are used for caching */
    "akamaihd.net",
    "dropboxusercontent.com",
    NULL /* End */
  };
  const ndpi_risk risks_to_mask[] = {
    NDPI_SUSPICIOUS_DGA_DOMAIN,
    NDPI_BINARY_APPLICATION_TRANSFER,
    NDPI_NUMERIC_IP_HOST,
    NDPI_MALICIOUS_FINGERPRINT,
    NDPI_NO_RISK /* End */
  };
  u_int i;
  ndpi_risk mask = ((ndpi_risk)-1);

  for(i=0; risks_to_mask[i] != NDPI_NO_RISK; i++)
    mask &= ~(1ULL << risks_to_mask[i]);

  for(i=0; domains[i] != NULL; i++)
    ndpi_add_host_risk_mask(ndpi_str, (char*)domains[i], mask);

  for(i=0; host_match[i].string_to_match != NULL; i++) {
    switch(host_match[i].protocol_category) {
    case NDPI_PROTOCOL_CATEGORY_CONNECTIVITY_CHECK:
    case NDPI_PROTOCOL_CATEGORY_CYBERSECURITY:
      ndpi_add_host_risk_mask(ndpi_str, (char*)host_match[i].string_to_match, mask);
      break;

    default:
      /* Nothing to do */
      break;
    }
  }
}

/* *********************************************** */

static int is_ip_list_enabled(struct ndpi_detection_module_struct *ndpi_str, int protoId)
{
  if(NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_str->cfg.ip_list_bitmask, protoId) == 0)
    return 0;
  return 1;
}

/* *********************************************** */

void ndpi_load_ip_lists(struct ndpi_detection_module_struct *ndpi_str) {

  if(!ndpi_str)
    return;

#ifndef __KERNEL__
  if(!ndpi_str->custom_categories.categories_loaded)
    ndpi_enable_loaded_categories(ndpi_str);
#endif
  if(ndpi_str->finalized) /* Already finalized */
    return;

  if(ndpi_str->cfg.libgcrypt_init) {
    if(!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P,0)) {
      const char *gcrypt_ver = gcry_check_version(NULL);
      if(!gcrypt_ver) {
        NDPI_LOG_ERR(ndpi_str, "Error initializing libgcrypt\n");
      }
      NDPI_LOG_DBG(ndpi_str, "Libgcrypt %s\n", gcrypt_ver);
      /* Tell Libgcrypt that initialization has completed. */
      gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    }
  } else {
    NDPI_LOG_DBG(ndpi_str, "Libgcrypt initialization skipped\n");
  }

  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_AMAZON_AWS)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_amazon_aws_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_amazon_aws_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_MICROSOFT_AZURE)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_microsoft_azure_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_microsoft_azure_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_CACHEFLY)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_cachefly_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_cachefly_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_CLOUDFLARE)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_cloudflare_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_cloudflare_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_DIGITALOCEAN)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_digitalocean_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_digitalocean_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_GOOGLE)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_google_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_google_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_GOOGLE_CLOUD)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_google_cloud_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_google_cloud_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_MICROSOFT_365)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_microsoft_365_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_microsoft_365_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_MS_ONE_DRIVE)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_ms_one_drive_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_ms_one_drive_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_MS_OUTLOOK)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_ms_outlook_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_ms_outlook_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_MSTEAMS)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_msteams_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_msteams_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_TOR)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_tor_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_tor_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_WHATSAPP)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_whatsapp_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_whatsapp_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_ETHEREUM)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_ethereum_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_ethereum_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_ZOOM)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_zoom_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_zoom_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_MULLVAD)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_mullvad_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_mullvad_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_NORDVPN)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_nordvpn_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_nordvpn_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_SURFSHARK)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_surfshark_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_surfshark_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_TELEGRAM)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_telegram_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_telegram_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_APPLE)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_apple_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_apple_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_TWITTER)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_twitter_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_twitter_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_NETFLIX)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_netflix_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_netflix_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_WEBEX)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_webex_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_webex_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_TEAMVIEWER)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_teamviewer_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_teamviewer_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_FACEBOOK)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_facebook_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_facebook_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_TENCENT)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_tencent_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_tencent_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_OPENDNS)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_opendns_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_opendns_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_DROPBOX)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_dropbox_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_dropbox_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_BLIZZARD)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_blizzard_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_blizzard_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_CANONICAL)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_canonical_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_canonical_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_TWITCH)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_twitch_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_twitch_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_HOTSPOT_SHIELD)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_hotspot_shield_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_hotspot_shield_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_GITHUB)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_github_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_github_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_STEAM)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_steam_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_steam_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_BLOOMBERG)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_bloomberg_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_bloomberg_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_EDGECAST)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_edgecast_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_edgecast_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_GOTO)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_goto_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_goto_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_RIOTGAMES)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_riotgames_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_riotgames_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_THREEMA)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_threema_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_threema_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_ALIBABA)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_alibaba_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_alibaba_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_AVAST)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_avast_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_avast_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_DISCORD)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_discord_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_discord_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_LINE)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_line_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_line_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_VK)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_vk_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_vk_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_YANDEX)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_yandex_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_yandex_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_YANDEX_CLOUD)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_yandex_cloud_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_yandex_cloud_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_DISNEYPLUS)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_disneyplus_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_disneyplus_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_HULU)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_hulu_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_hulu_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_EPICGAMES)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_epicgames_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_epicgames_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_NVIDIA)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_nvidia_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_nvidia_protocol_list_6);
  }
  if(is_ip_list_enabled(ndpi_str, NDPI_PROTOCOL_ROBLOX)) {
    ndpi_init_ptree_ipv4(ndpi_str->protocols->v4, ndpi_protocol_roblox_protocol_list);
    ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->protocols->v6, ndpi_protocol_roblox_protocol_list_6);
  }

  if(ndpi_str->cfg.flow_risk_lists_enabled) {
    if((ndpi_str->ip_risk = ndpi_ptree_create()) == NULL) {
      NDPI_LOG_ERR(ndpi_str, "[NDPI] Error allocating risk tree\n");
      return;
    }

    if(ndpi_str->cfg.risk_anonymous_subscriber_list_icloudprivaterelay_enabled) {
      ndpi_init_ptree_ipv4(ndpi_str->ip_risk->v4, ndpi_anonymous_subscriber_icloud_private_relay_protocol_list);
      ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->ip_risk->v6, ndpi_anonymous_subscriber_icloud_private_relay_protocol_list_6);
    }

    if(ndpi_str->cfg.risk_anonymous_subscriber_list_tor_exit_nodes_enabled) {
      ndpi_init_ptree_ipv4(ndpi_str->ip_risk->v4, ndpi_anonymous_subscriber_tor_exit_nodes_protocol_list);
      ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->ip_risk->v6, ndpi_anonymous_subscriber_tor_exit_nodes_protocol_list_6);
    }

    if(ndpi_str->cfg.risk_crawler_bot_list_enabled) {
      ndpi_init_ptree_ipv4(ndpi_str->ip_risk->v4, ndpi_http_crawler_bot_protocol_list);
      ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->ip_risk->v6, ndpi_http_crawler_bot_protocol_list_6);
      /* Hard-coded lists */
      ndpi_init_ptree_ipv4(ndpi_str->ip_risk->v4, ndpi_http_crawler_bot_hardcoded_protocol_list);
      ndpi_init_ptree_ipv6(ndpi_str, ndpi_str->ip_risk->v6, ndpi_http_crawler_bot_hardcoded_protocol_list_6);
    }
  }
}

/* *********************************************** */

int is_monitoring_enabled(struct ndpi_detection_module_struct *ndpi_str, int protoId)
{
  if(NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_str->cfg.monitoring, protoId) == 0)
    return 0;
  return 1;
}

/* *********************************************** */

int ndpi_finalize_initialization(struct ndpi_detection_module_struct *ndpi_str) {
  u_int i;

  if(!ndpi_str)
    return -1;
  if(ndpi_str->finalized) /* Already finalized */
    return 0;

  if(ndpi_str->cfg.libgcrypt_init) {
    if(!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P,0)) {
      const char *gcrypt_ver = gcry_check_version(NULL);
      if(!gcrypt_ver) {
        NDPI_LOG_ERR(ndpi_str, "Error initializing libgcrypt\n");
      }
      NDPI_LOG_DBG(ndpi_str, "Libgcrypt %s\n", gcrypt_ver);
      /* Tell Libgcrypt that initialization has completed. */
      gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    }
  } else {
    NDPI_LOG_DBG(ndpi_str, "Libgcrypt initialization skipped\n");
  }

  ndpi_load_ip_lists(ndpi_str);

  ndpi_add_domain_risk_exceptions(ndpi_str);

  if(ndpi_str->cfg.ookla_cache_num_entries > 0) {
    if(ndpi_str->cfg.ookla_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->ookla_global_cache) {
        ndpi_str->g_ctx->ookla_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.ookla_cache_num_entries,
                                                                  ndpi_str->cfg.ookla_cache_ttl, 1);
      }
      ndpi_str->ookla_cache = ndpi_str->g_ctx->ookla_global_cache;
    } else {
      ndpi_str->ookla_cache = ndpi_lru_cache_init(ndpi_str->cfg.ookla_cache_num_entries,
                                                  ndpi_str->cfg.ookla_cache_ttl, 0);
    }
    if(!ndpi_str->ookla_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.ookla_cache_num_entries);
    }
  }

  if(ndpi_str->cfg.signal_cache_num_entries > 0) {
    if(ndpi_str->cfg.signal_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->signal_global_cache) {
        ndpi_str->g_ctx->signal_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.signal_cache_num_entries,
                                                                  ndpi_str->cfg.signal_cache_ttl, 1);
      }
      ndpi_str->signal_cache = ndpi_str->g_ctx->signal_global_cache;
    } else {
      ndpi_str->signal_cache = ndpi_lru_cache_init(ndpi_str->cfg.signal_cache_num_entries,
                                                  ndpi_str->cfg.signal_cache_ttl, 0);
    }
    if(!ndpi_str->signal_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.signal_cache_num_entries);
    }
  }

  if(ndpi_str->cfg.bittorrent_cache_num_entries > 0) {
    if(ndpi_str->cfg.bittorrent_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->bittorrent_global_cache) {
        ndpi_str->g_ctx->bittorrent_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.bittorrent_cache_num_entries,
                                                                       ndpi_str->cfg.bittorrent_cache_ttl, 1);
      }
      ndpi_str->bittorrent_cache = ndpi_str->g_ctx->bittorrent_global_cache;
    } else {
      ndpi_str->bittorrent_cache = ndpi_lru_cache_init(ndpi_str->cfg.bittorrent_cache_num_entries,
                                                       ndpi_str->cfg.bittorrent_cache_ttl, 0);
    }
    if(!ndpi_str->bittorrent_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.bittorrent_cache_num_entries);
    }
  }
  if(ndpi_str->cfg.stun_cache_num_entries > 0) {
    if(ndpi_str->cfg.stun_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->stun_global_cache) {
        ndpi_str->g_ctx->stun_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.stun_cache_num_entries,
                                                                 ndpi_str->cfg.stun_cache_ttl, 1);
      }
      ndpi_str->stun_cache = ndpi_str->g_ctx->stun_global_cache;
    } else {
      ndpi_str->stun_cache = ndpi_lru_cache_init(ndpi_str->cfg.stun_cache_num_entries,
                                                 ndpi_str->cfg.stun_cache_ttl, 0);
    }
    if(!ndpi_str->stun_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.stun_cache_num_entries);
    }
  }
  if(ndpi_str->cfg.tls_cert_cache_num_entries > 0) {
    if(ndpi_str->cfg.tls_cert_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->tls_cert_global_cache) {
        ndpi_str->g_ctx->tls_cert_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.tls_cert_cache_num_entries,
                                                                     ndpi_str->cfg.tls_cert_cache_ttl, 1);
      }
      ndpi_str->tls_cert_cache = ndpi_str->g_ctx->tls_cert_global_cache;
    } else {
      ndpi_str->tls_cert_cache = ndpi_lru_cache_init(ndpi_str->cfg.tls_cert_cache_num_entries,
                                                     ndpi_str->cfg.tls_cert_cache_ttl, 0);
    }
    if(!ndpi_str->tls_cert_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.tls_cert_cache_num_entries);
    }
  }
  if(ndpi_str->cfg.mining_cache_num_entries > 0) {
    if(ndpi_str->cfg.mining_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->mining_global_cache) {
        ndpi_str->g_ctx->mining_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.mining_cache_num_entries,
                                                                   ndpi_str->cfg.mining_cache_ttl, 1);
      }
      ndpi_str->mining_cache = ndpi_str->g_ctx->mining_global_cache;
    } else {
      ndpi_str->mining_cache = ndpi_lru_cache_init(ndpi_str->cfg.mining_cache_num_entries,
                                                   ndpi_str->cfg.mining_cache_ttl, 0);
    }
    if(!ndpi_str->mining_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.mining_cache_num_entries);
    }
  }
  if(ndpi_str->cfg.msteams_cache_num_entries > 0) {
    if(ndpi_str->cfg.msteams_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->msteams_global_cache) {
        ndpi_str->g_ctx->msteams_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.msteams_cache_num_entries,
                                                                    ndpi_str->cfg.msteams_cache_ttl, 1);
      }
      ndpi_str->msteams_cache = ndpi_str->g_ctx->msteams_global_cache;
    } else {
      ndpi_str->msteams_cache = ndpi_lru_cache_init(ndpi_str->cfg.msteams_cache_num_entries,
                                                    ndpi_str->cfg.msteams_cache_ttl, 0);
    }
    if(!ndpi_str->msteams_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru cache (num_entries %u)\n",
                   ndpi_str->cfg.msteams_cache_num_entries);
    }
  }

  if(ndpi_str->cfg.fpc_dns_cache_num_entries > 0) {
    if(ndpi_str->cfg.fpc_dns_cache_scope == NDPI_LRUCACHE_SCOPE_GLOBAL) {
      if(!ndpi_str->g_ctx->fpc_dns_global_cache) {
        ndpi_str->g_ctx->fpc_dns_global_cache = ndpi_lru_cache_init(ndpi_str->cfg.fpc_dns_cache_num_entries,
                                                                    ndpi_str->cfg.fpc_dns_cache_ttl, 1);
      }
      ndpi_str->fpc_dns_cache = ndpi_str->g_ctx->fpc_dns_global_cache;
    } else {
      ndpi_str->fpc_dns_cache = ndpi_lru_cache_init(ndpi_str->cfg.fpc_dns_cache_num_entries,
                                                    ndpi_str->cfg.fpc_dns_cache_ttl, 0);
    }
    if(!ndpi_str->fpc_dns_cache) {
      NDPI_LOG_ERR(ndpi_str, "Error allocating lru fpc_dns_cache (num_entries %u)\n",
                   ndpi_str->cfg.fpc_dns_cache_num_entries);

    }
  }

  ndpi_automa * const automa[] = { &ndpi_str->host_automa,
                                   &ndpi_str->tls_cert_subject_automa,
                                   &ndpi_str->host_risk_mask_automa,
                                   &ndpi_str->risky_domain_automa,
                                   &ndpi_str->common_alpns_automa };

  for(i = 0; i < NDPI_ARRAY_LENGTH(automa); ++i) {
    ndpi_automa *a = automa[i];

    if(a && a->ac_automa)
      ac_automata_finalize((AC_AUTOMATA_t *) a->ac_automa);
  }

  if(ndpi_str->cfg.tls_app_blocks_tracking_enabled) {
    ndpi_str->num_tls_blocks_to_follow = NDPI_MAX_NUM_TLS_APPL_BLOCKS;
    ndpi_str->skip_tls_blocks_until_change_cipher = 1;
  }

  if(ndpi_str->cfg.track_payload_enabled)
    ndpi_str->max_payload_track_len = 1024; /* track up to X payload bytes */

  ndpi_str->finalized = 1;

  return 0;
}

/* *********************************************** */

/* Wrappers */
void *ndpi_init_automa(void) {
  return(ac_automata_init(NULL));
}

/* ****************************************************** */

void *ndpi_init_automa_domain(void) {
  return(ac_automata_init(ac_domain_match_handler));
}

/* ****************************************************** */

int ndpi_add_string_value_to_automa(void *_automa, char *str, u_int32_t num) {
  AC_PATTERN_t ac_pattern;
  AC_AUTOMATA_t *automa = (AC_AUTOMATA_t *) _automa;
  AC_ERROR_t rc;
  char *cstr;

  if(automa == NULL)
    return(-1);

  if(!str) return(-1);
  cstr = ndpi_strdup(str);
  if(!cstr) return(-1);

  memset(&ac_pattern, 0, sizeof(ac_pattern));
  ac_pattern.astring    = cstr;
  ac_pattern.rep.number = num;
  ac_pattern.length     = strlen(ac_pattern.astring);

  rc = ac_automata_add(automa, &ac_pattern);
  return(rc == ACERR_SUCCESS ? 0 : (rc == ACERR_DUPLICATE_PATTERN ? -2 : -1));
}

/* ****************************************************** */

int ndpi_add_string_to_automa(void *_automa, char *str) {
  return(ndpi_add_string_value_to_automa(_automa, str, 1));
}

/* ****************************************************** */

void ndpi_free_automa(void *_automa)
{
    ac_automata_release((AC_AUTOMATA_t*)_automa,1);
}
void ndpi_finalize_automa(void *_automa)
{
    ac_automata_finalize((AC_AUTOMATA_t*)_automa);
}
void *ndpi_automa_host(struct ndpi_detection_module_struct *ndpi_struct)
{
    return (AC_AUTOMATA_t*)ndpi_struct->host_automa.ac_automa;
}

/* ****************************************************** */

void ndpi_automa_get_stats(void *_automa, struct ndpi_automa_stats *stats) {
  struct ac_stats ac_stats;

  ac_automata_get_stats((AC_AUTOMATA_t *) _automa, &ac_stats);
  stats->n_search = ac_stats.n_search;
  stats->n_found = ac_stats.n_found;
}

/* ****************************************************** */

int ndpi_get_automa_stats(struct ndpi_detection_module_struct *ndpi_struct,
			  automa_type automa_type,
			  struct ndpi_automa_stats *stats)
{
  if(!ndpi_struct || !stats)
    return -1;

  switch(automa_type) {
  case NDPI_AUTOMA_HOST:
    ndpi_automa_get_stats(ndpi_struct->host_automa.ac_automa, stats);
    return 0;

  case NDPI_AUTOMA_DOMAIN:
    ndpi_automa_get_stats(ndpi_struct->risky_domain_automa.ac_automa, stats);
    return 0;

  case NDPI_AUTOMA_TLS_CERT:
    ndpi_automa_get_stats(ndpi_struct->tls_cert_subject_automa.ac_automa, stats);
    return 0;

  case NDPI_AUTOMA_RISK_MASK:
    ndpi_automa_get_stats(ndpi_struct->host_risk_mask_automa.ac_automa, stats);
    return 0;

  case NDPI_AUTOMA_COMMON_ALPNS:
    ndpi_automa_get_stats(ndpi_struct->common_alpns_automa.ac_automa, stats);
    return 0;

  default:
    return -1;
  }
}

/* ****************************************************** */

static int ndpi_match_string_common(AC_AUTOMATA_t *automa, char *string_to_match,size_t string_len,
				    u_int32_t *protocol_id, ndpi_protocol_category_t *category,
				    ndpi_protocol_breed_t *breed) {
  AC_REP_t match = { NDPI_PROTOCOL_UNKNOWN, NDPI_PROTOCOL_CATEGORY_UNSPECIFIED, NDPI_PROTOCOL_UNRATED, 0, 0, 0, 0, 0 };
  AC_TEXT_t ac_input_text;
  int rc;

  if(protocol_id) *protocol_id = NDPI_PROTOCOL_UNKNOWN;

  if((automa == NULL) || (string_to_match == NULL) || (string_to_match[0] == '\0')) {
    return(-2);
  }

  if(automa->automata_open) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    printf("[%s:%d] [NDPI] Internal error: please call ndpi_finalize_initialization()\n", __FILE__, __LINE__);
#endif
    return(-1);
  }

  if(string_len & (AC_FEATURE_EXACT|AC_FEATURE_LC|AC_FEATURE_MATCH_DEBUG)) { 
	ac_input_text.option = string_len & (AC_FEATURE_EXACT|AC_FEATURE_LC|AC_FEATURE_MATCH_DEBUG);
	string_len &= ~(AC_FEATURE_EXACT|AC_FEATURE_LC|AC_FEATURE_MATCH_DEBUG);
  } else
	ac_input_text.option = 0;
  ac_input_text.astring = string_to_match; ac_input_text.length = string_len;
  rc = ac_automata_search(automa, &ac_input_text, &match);

  if(protocol_id)
    *protocol_id = rc ? match.number : NDPI_PROTOCOL_UNKNOWN;

  if(category)
    *category = rc ? match.category : 0;

  if(breed)
    *breed = rc ? match.breed : 0;

  return rc;
}

/* ****************************************************** */

int ndpi_match_string(void *_automa, char *string_to_match) {
  uint32_t proto_id;
  int rc;

  if(!string_to_match)
    return(-2);

  rc = ndpi_match_string_common(_automa, string_to_match,
				strlen(string_to_match),
				&proto_id, NULL, NULL);
  if(rc < 0) return rc;

  return rc ? proto_id : NDPI_PROTOCOL_UNKNOWN;
}

/* ****************************************************** */

int ndpi_match_string_protocol_id(void *automa, char *string_to_match,
				  u_int match_len, u_int16_t *protocol_id,
				  ndpi_protocol_category_t *category,
				  ndpi_protocol_breed_t *breed) {
  u_int32_t proto_id;
  int rc = ndpi_match_string_common((AC_AUTOMATA_t*)automa, string_to_match,
				    match_len, &proto_id, category, breed);
  if(rc < 0) return rc;
  *protocol_id = (u_int16_t)proto_id;
  return(proto_id != NDPI_PROTOCOL_UNKNOWN ? 0 : -1);
}

/* ****************************************************** */

int ndpi_match_string_value(void *automa, char *string_to_match,
			    u_int match_len, u_int32_t *num) {
  int rc = ndpi_match_string_common((AC_AUTOMATA_t *)automa, string_to_match,
				    match_len, num, NULL, NULL);
  if(rc < 0) return rc;
  return rc ? 0 : -1;
}


/* *********************************************** */
#ifndef __KERNEL__
int ndpi_match_custom_category(struct ndpi_detection_module_struct *ndpi_str,
			       char *name, u_int name_len,
                               ndpi_protocol_category_t *category) {
  char buf[128];
  u_int16_t class_id;
  u_int max_len = sizeof(buf)-1;

  if(!ndpi_str->custom_categories.categories_loaded)
    ndpi_enable_loaded_categories(ndpi_str);

  if(name_len > max_len) name_len = max_len;
  memcpy(buf, name, name_len);
  buf[name_len] = '\0';

  if(ndpi_domain_classify_hostname(ndpi_str, ndpi_str->custom_categories.sc_hostnames,
				   &class_id, buf)) {
    *category = (ndpi_protocol_category_t)class_id;
    return(0);
  } else
    return(-1); /* Not found */
}

/* *********************************************** */

int ndpi_get_custom_category_match(struct ndpi_detection_module_struct *ndpi_str,
				   char *name_or_ip, u_int name_len,
				   ndpi_protocol_category_t *id) {
  char ipbuf[64], *ptr;
  struct in_addr pin;
  struct in6_addr pin6;
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;
  u_int cp_len = ndpi_min(sizeof(ipbuf) - 1, name_len);

  *id = 0;

  if(!ndpi_str->custom_categories.categories_loaded)
    ndpi_enable_loaded_categories(ndpi_str);

  if(cp_len > 0) {
    memcpy(ipbuf, name_or_ip, cp_len);
    ipbuf[cp_len] = '\0';
  } else {
    ipbuf[0] = '\0';
  }

  ptr = strrchr(ipbuf, '/');

  if(ptr)
    ptr[0] = '\0';

  if(inet_pton(AF_INET, ipbuf, &pin) == 1) {
    /* Search IPv4 */

    /* Make sure all in network byte order otherwise compares wont work */
    ndpi_fill_prefix_v4(&prefix, &pin, 32,
			((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses)->maxbits);
    node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses, &prefix);

    if(node) {
      *id = node->value.u.uv32.user_value;
      return(0);
    }
    return(-1);
  } else if(inet_pton(AF_INET6, ipbuf, &pin6) == 1) {
    /* Search IPv6 */
    ndpi_fill_prefix_v6(&prefix, &pin6, 128,
			((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6)->maxbits);
    node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses6, &prefix);

    if(node) {
      *id = node->value.u.uv32.user_value;
      return(0);
    }
    return(-1);
  } else {
    /* Search Host */
    return(ndpi_match_custom_category(ndpi_str, name_or_ip, name_len, id));
  }
}
#endif

/* ****************************************************** */

void ndpi_exit_detection_module(struct ndpi_detection_module_struct *ndpi_str) {
  if(ndpi_str != NULL) {
    int i;

    for (i = 0; i < (NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS); i++) {
      if(ndpi_str->proto_defaults[i].protoName)
        ndpi_free(ndpi_str->proto_defaults[i].protoName);
      if(ndpi_str->proto_defaults[i].subprotocols != NULL)
        ndpi_free(ndpi_str->proto_defaults[i].subprotocols);
    }

#ifdef HAVE_NBPF
    for(i = 0; (i < MAX_NBPF_CUSTOM_PROTO) && (ndpi_str->nbpf_custom_proto[i].tree != NULL); i++)
      nbpf_free(ndpi_str->nbpf_custom_proto[i].tree);
#endif

    /* NDPI_PROTOCOL_TINC */
    if(ndpi_str->tinc_cache)
      cache_free((cache_t)(ndpi_str->tinc_cache));
    ndpi_bittorrent_done(ndpi_str);

    if(!ndpi_str->cfg.ookla_cache_scope &&
       ndpi_str->ookla_cache)
      ndpi_lru_free_cache(ndpi_str->ookla_cache);

    if(!ndpi_str->cfg.signal_cache_scope &&
       ndpi_str->signal_cache)
      ndpi_lru_free_cache(ndpi_str->signal_cache);

    if(!ndpi_str->cfg.bittorrent_cache_scope &&
       ndpi_str->bittorrent_cache)
      ndpi_lru_free_cache(ndpi_str->bittorrent_cache);

    if(!ndpi_str->cfg.stun_cache_scope &&
       ndpi_str->stun_cache)
      ndpi_lru_free_cache(ndpi_str->stun_cache);

    if(!ndpi_str->cfg.tls_cert_cache_scope &&
       ndpi_str->tls_cert_cache)
      ndpi_lru_free_cache(ndpi_str->tls_cert_cache);

    if(!ndpi_str->cfg.mining_cache_scope &&
       ndpi_str->mining_cache)
      ndpi_lru_free_cache(ndpi_str->mining_cache);

    if(!ndpi_str->cfg.msteams_cache_scope &&
       ndpi_str->msteams_cache)
      ndpi_lru_free_cache(ndpi_str->msteams_cache);

    if(!ndpi_str->cfg.fpc_dns_cache_scope &&
       ndpi_str->fpc_dns_cache)
      ndpi_lru_free_cache(ndpi_str->fpc_dns_cache);

    if(ndpi_str->protocols)    ndpi_ptree_destroy(ndpi_str->protocols);
    if(ndpi_str->ip_risk_mask) ndpi_ptree_destroy(ndpi_str->ip_risk_mask);
    if(ndpi_str->ip_risk)      ndpi_ptree_destroy(ndpi_str->ip_risk);

    if(ndpi_str->udpRoot != NULL) ndpi_tdestroy(ndpi_str->udpRoot, ndpi_free);
    if(ndpi_str->tcpRoot != NULL) ndpi_tdestroy(ndpi_str->tcpRoot, ndpi_free);

    if(ndpi_str->host_automa.ac_automa != NULL)
      ac_automata_release((AC_AUTOMATA_t*)ndpi_str->host_automa.ac_automa, 1);

    if(ndpi_str->risky_domain_automa.ac_automa != NULL)
      ac_automata_release((AC_AUTOMATA_t *) ndpi_str->risky_domain_automa.ac_automa,
		          1 /* free patterns strings memory */);

    if(ndpi_str->tls_cert_subject_automa.ac_automa != NULL)
      ac_automata_release((AC_AUTOMATA_t *) ndpi_str->tls_cert_subject_automa.ac_automa, 1);
#ifndef __KERNEL__
    if(ndpi_str->malicious_ja4_hashmap != NULL)
      ndpi_hash_free(&ndpi_str->malicious_ja4_hashmap);

    if(ndpi_str->malicious_sha1_hashmap != NULL)
      ndpi_hash_free(&ndpi_str->malicious_sha1_hashmap);

    if(ndpi_str->tcp_fingerprint_hashmap != NULL)
      ndpi_hash_free(&ndpi_str->tcp_fingerprint_hashmap);

    ndpi_domain_classify_free(ndpi_str->custom_categories.sc_hostnames_shadow);
    ndpi_domain_classify_free(ndpi_str->custom_categories.sc_hostnames);
#endif

    if(ndpi_str->custom_categories.ipAddresses != NULL)
      ndpi_patricia_destroy((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses, free_ptree_data);

    if(ndpi_str->custom_categories.ipAddresses_shadow != NULL)
      ndpi_patricia_destroy((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses_shadow, free_ptree_data);

    if(ndpi_str->custom_categories.ipAddresses6 != NULL)
      ndpi_patricia_destroy((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6, free_ptree_data);

    if(ndpi_str->custom_categories.ipAddresses6_shadow != NULL)
      ndpi_patricia_destroy((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6_shadow, free_ptree_data);

    if(ndpi_str->host_risk_mask_automa.ac_automa != NULL)
      ac_automata_release((AC_AUTOMATA_t *) ndpi_str->host_risk_mask_automa.ac_automa,
			  1 /* free patterns strings memory */);

    if(ndpi_str->common_alpns_automa.ac_automa != NULL)
      ac_automata_release((AC_AUTOMATA_t *) ndpi_str->common_alpns_automa.ac_automa,
			  1 /* free patterns strings memory */);

    if(ndpi_str->trusted_issuer_dn) {
      ndpi_list *head = ndpi_str->trusted_issuer_dn;

      while(head != NULL) {
	ndpi_list *next;

	if(head->value) ndpi_free(head->value);
	next = head->next;
	ndpi_free(head);
	head = next;
      }
    }

#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../../nDPI-custom/ndpi_exit_detection_module.c"
#endif

#ifndef __KERNEL__
    ndpi_free_geoip(ndpi_str);
#endif

    if(ndpi_str->callback_buffer)
      ndpi_free(ndpi_str->callback_buffer);

    if(ndpi_str->callback_buffer_tcp_payload)
      ndpi_free(ndpi_str->callback_buffer_tcp_payload);

#ifndef __KERNEL__
    if(ndpi_str->public_domain_suffixes)
      ndpi_hash_free(&ndpi_str->public_domain_suffixes);
#endif

    if(ndpi_str->address_cache)
      ndpi_term_address_cache(ndpi_str->address_cache);

    ndpi_free(ndpi_str);
  }

#ifdef WIN32
  WSACleanup();
#endif
}

/* ****************************************************** */

static default_ports_tree_node_t *ndpi_get_guessed_protocol_id(struct ndpi_detection_module_struct *ndpi_str,
                                                               u_int8_t proto, u_int16_t sport, u_int16_t dport) {
  default_ports_tree_node_t node;
  /* Set use_sport to config value if direction detection is enabled */
  int use_sport = ndpi_str->cfg.direction_detect_enabled ? ndpi_str->cfg.use_client_port_in_guess : 1;

  if(sport && dport) {
    const void *ret;

    node.default_port = dport; /* Check server port first */
    ret = ndpi_tfind(&node, (proto == IPPROTO_TCP) ? (void *) &ndpi_str->tcpRoot : (void *) &ndpi_str->udpRoot,
		     default_ports_tree_node_t_cmp);

    if(ret == NULL && use_sport) {
      node.default_port = sport;
      ret = ndpi_tfind(&node, (proto == IPPROTO_TCP) ? (void *) &ndpi_str->tcpRoot : (void *) &ndpi_str->udpRoot,
		       default_ports_tree_node_t_cmp);
    }

    if(ret)
      return(*(default_ports_tree_node_t **) ret);
  }

  return(NULL);
}

/* ****************************************************** */

static u_int16_t guess_protocol_id(struct ndpi_detection_module_struct *ndpi_str,
                                    u_int8_t proto, u_int16_t sport, u_int16_t dport, u_int8_t *user_defined_proto) {
  *user_defined_proto = 0; /* Default */

  if(sport && dport) {
    default_ports_tree_node_t *found = ndpi_get_guessed_protocol_id(ndpi_str, proto, sport, dport);

    if(found != NULL) {
      u_int16_t guessed_proto = found->proto->protoId;

      *user_defined_proto = found->customUserProto;
      return(guessed_proto);
    }
  } else {
    /* No TCP/UDP */

    /* All these calls to `is_proto_enabled()` are needed to avoid classification by-port
       if the protocol is disabled */
    switch(proto) {
    case NDPI_IPSEC_PROTOCOL_ESP:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_ESP))
        return(NDPI_PROTOCOL_IP_ESP);
      break;
    case NDPI_IPSEC_PROTOCOL_AH:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_AH))
        return(NDPI_PROTOCOL_IP_AH);
      break;
    case NDPI_GRE_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_GRE))
        return(NDPI_PROTOCOL_IP_GRE);
      break;
    case NDPI_PGM_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_PGM))
        return(NDPI_PROTOCOL_IP_PGM);
      break;
    case NDPI_PIM_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_PIM))
        return(NDPI_PROTOCOL_IP_PIM);
      break;
    case NDPI_ICMP_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_ICMP))
        return(NDPI_PROTOCOL_IP_ICMP);
      break;
    case NDPI_IGMP_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_IGMP))
        return(NDPI_PROTOCOL_IP_IGMP);
      break;
    case NDPI_EGP_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_EGP))
        return(NDPI_PROTOCOL_IP_EGP);
      break;
    case NDPI_SCTP_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_SCTP))
        return(NDPI_PROTOCOL_IP_SCTP);
      break;
    case NDPI_OSPF_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_OSPF))
        return(NDPI_PROTOCOL_IP_OSPF);
      break;
    case NDPI_IPIP_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_IP_IN_IP))
        return(NDPI_PROTOCOL_IP_IP_IN_IP);
      break;
    case NDPI_ICMPV6_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_ICMPV6))
        return(NDPI_PROTOCOL_IP_ICMPV6);
      break;
    case NDPI_VRRP_PROTOCOL_TYPE:
      if(is_proto_enabled(ndpi_str, NDPI_PROTOCOL_IP_VRRP))
        return(NDPI_PROTOCOL_IP_VRRP);
      break;
    }
  }

  return(NDPI_PROTOCOL_UNKNOWN);
}

/* ******************************************************************** */

u_int ndpi_get_num_supported_protocols(struct ndpi_detection_module_struct *ndpi_str) {
  return(ndpi_str ? ndpi_str->ndpi_num_supported_protocols : 0);
}

/* ******************************************************************** */

#ifdef WIN32
char *strsep(char **sp, char *sep) {
  char *p, *s;
  if(sp == NULL || *sp == NULL || **sp == '\0')
    return(NULL);
  s = *sp;
  p = s + strcspn(s, sep);
  if(*p != '\0')
    *p++ = '\0';
  *sp = p;
  return(s);
}
#endif

/* ******************************************************************** */

int ndpi_add_ip_risk_mask(struct ndpi_detection_module_struct *ndpi_str,
			  char *ip, ndpi_risk mask) {
  char *cidr, *saveptr, *addr = strtok_r(ip, "/", &saveptr);
  int is_ipv6 = 0;
  ndpi_patricia_node_t *node = NULL;

  if(!addr || strlen(addr) == 0)
    return(-2);

  if(ip[0] == '[') {
    is_ipv6 = 1;
    addr += 1;
    addr[strlen(addr) - 1] = '\0'; /* strip ']' */
  }

  cidr = strtok_r(NULL, "\n", &saveptr);

  if(!is_ipv6 && ndpi_str->ip_risk_mask && ndpi_str->ip_risk_mask->v4) {
    struct in_addr pin;

    if(inet_pton(AF_INET, addr, &pin) != 1)
      return(-1);
    node = add_to_ptree(ndpi_str->ip_risk_mask->v4, AF_INET,
			&pin, cidr ? atoi(cidr) : 32 /* bits */);
  } else if(is_ipv6 && ndpi_str->ip_risk_mask && ndpi_str->ip_risk_mask->v6) {
    struct in6_addr pin6;

    if(inet_pton(AF_INET6, addr, &pin6) != 1)
      return(-1);
    node = add_to_ptree(ndpi_str->ip_risk_mask->v6, AF_INET6,
			&pin6, cidr ? atoi(cidr) : 128 /* bits */);
  } else {
    return(-2);
  }

  if(node) {
    node->value.u.uv64 = (u_int64_t)mask;
    return(0);
  }
  return(-1);
}

/* ******************************************************************** */

int ndpi_add_host_risk_mask(struct ndpi_detection_module_struct *ndpi_str,
			    char *host, ndpi_risk mask) {
  AC_PATTERN_t ac_pattern;
  AC_ERROR_t rc;
  u_int len;
  char *host_dup = NULL;

  if((ndpi_str == NULL) || (ndpi_str->host_risk_mask_automa.ac_automa == NULL) || (host == NULL))
    return(-2);

  /* Zap heading/trailing quotes */
  switch(host[0]) {
  case '"':
  case '\'':
      host = &host[1];
      len = strlen(host);
      if(len > 0)
	host[len-1] = '\0';

    break;
  }

  host_dup = ndpi_strdup(host);
  if(!host_dup)
    return(-1);

  memset(&ac_pattern, 0, sizeof(ac_pattern));

  len = strlen(host);

  ac_pattern.astring      = host_dup;
  ac_pattern.length       = len;
  ac_pattern.rep.number64 = (ndpi_risk)mask;
  ac_pattern.rep.level    = ndpi_domain_level(host);
  ac_pattern.rep.at_end   = 0;
  ac_pattern.rep.dot      = memchr(host,'.',len) != NULL;

  rc = ac_automata_add(ndpi_str->host_risk_mask_automa.ac_automa, &ac_pattern);

  if(rc != ACERR_SUCCESS) {
    ndpi_free(host_dup);

    if(rc != ACERR_DUPLICATE_PATTERN)
      return (-2);
  }

  return(0);
}

/* ******************************************************************** */

int ndpi_add_trusted_issuer_dn(struct ndpi_detection_module_struct *ndpi_str, char *dn) {
#ifndef __KERNEL__
  ndpi_list *head;

  if(dn == NULL)
    return(-1);
  else
    head = (ndpi_list*)ndpi_malloc(sizeof(ndpi_list));

  if(head == NULL) return(-2);

  if(dn[0] == '"') {
    char buf[128], *quote;

    ndpi_snprintf(buf, sizeof(buf), "%s", &dn[1]);

    if((quote = strchr(buf, '"')) != NULL)
      quote[0] = '\0';

    head->value = ndpi_strdup(buf);
  } else
    head->value = ndpi_strdup(dn);

  if(head->value == NULL) {
    ndpi_free(head);
    return(-3);
  }

  head->next = ndpi_str->trusted_issuer_dn;
  ndpi_str->trusted_issuer_dn = head;
#endif

  return(0);
}
/* ******************************************************************** */

int ndpi_handle_rule(struct ndpi_detection_module_struct *ndpi_str,
		            char *rule) {
  char *at, *proto, *elem;
  ndpi_proto_defaults_t *def;
  u_int subprotocol_id, i;
  int ret = 0;

  at = strrchr(rule, '@');
  if(at == NULL) {
    /* This looks like a mask rule or an invalid rule */
    char _rule[256], *rule_type, *key, *saveptr;

    ndpi_snprintf(_rule, sizeof(_rule), "%s", rule);
    rule_type = strtok_r(rule, ":",&saveptr);

    if(!rule_type) {
      NDPI_LOG_ERR(ndpi_str, "Invalid rule '%s'\n", rule);
      return(-1);
    }

    if(!strcmp(rule_type, "trusted_issuer_dn"))
      return(ndpi_add_trusted_issuer_dn(ndpi_str, strtok_r(NULL, ":",&saveptr)));

    key = strtok_r(NULL, "=", &saveptr);
    if(key) {
      char *value = strtok_r(NULL, "=", &saveptr);

      if(value) {
	ndpi_risk risk_mask = (ndpi_risk)atoll(value);

	if(!strcmp(rule_type, "ip_risk_mask") ||
	   !strcmp(rule_type, "ipv6_risk_mask")) {
	  return(ndpi_add_ip_risk_mask(ndpi_str, key, risk_mask));
	} else if(!strcmp(rule_type, "host_risk_mask")) {
	  return(ndpi_add_host_risk_mask(ndpi_str, key, risk_mask));
	}
      }
    }

    NDPI_LOG_ERR(ndpi_str, "Unknown rule '%s'\n", rule);
    return(-1);
  } else
    at[0] = 0, proto = &at[1];

  for(i = 0; proto[i] != '\0'; i++) {
    switch(proto[i]) {
    case '/':
    case '&':
    case '^':
    case ':':
    case ';':
    case '\'':
    case '"':
    case ' ':
      proto[i] = '_';
      break;
    }
  }

  subprotocol_id = ndpi_get_proto_by_name(ndpi_str, proto);

  if(subprotocol_id == NDPI_PROTOCOL_UNKNOWN)
    def = NULL;
  else
    def = &ndpi_str->proto_defaults[subprotocol_id];

  if(def == NULL) {
    ndpi_port_range ports_a[MAX_DEFAULT_PORTS], ports_b[MAX_DEFAULT_PORTS];
    char *equal = strchr(proto, '=');
    u_int16_t user_proto_id = ndpi_str->ndpi_num_supported_protocols;

    if(equal != NULL) {
      /* PROTO=VALUE */

      equal[0] = '\0';
      user_proto_id = atoi(&equal[1]);

      /* NOTE: ndpi_str->ndpi_num_supported_protocols >= NDPI_MAX_SUPPORTED_PROTOCOLS */
      ndpi_add_user_proto_id_mapping(ndpi_str, ndpi_str->ndpi_num_supported_protocols, user_proto_id);

      /* printf("***** ADDING MAPPING %s: %u -> %u\n", proto, ndpi_str->ndpi_num_supported_protocols, user_proto_id); */
    } else
      ndpi_add_user_proto_id_mapping(ndpi_str, ndpi_str->ndpi_num_supported_protocols,
				     ndpi_str->ndpi_num_supported_protocols);

    if(ndpi_str->ndpi_num_custom_protocols >= (NDPI_MAX_NUM_CUSTOM_PROTOCOLS - 1)) {
      NDPI_LOG_ERR(ndpi_str, "Too many protocols defined (%u): skipping protocol %s\n",
		   ndpi_str->ndpi_num_custom_protocols, proto);
      return(-2);
    }

    ndpi_set_proto_defaults(ndpi_str, 1 /* is_cleartext */,
			    1 /* is_app_protocol */,
			    NDPI_PROTOCOL_ACCEPTABLE, /* breed*/
			    ndpi_str->ndpi_num_supported_protocols /* protoId */,
			    proto, /* protoName */
			    NDPI_PROTOCOL_CATEGORY_UNSPECIFIED, /* TODO add protocol category support in rules */
			    NDPI_PROTOCOL_QOE_CATEGORY_UNSPECIFIED,
			    ndpi_build_default_ports(ports_a, 0, 0, 0, 0, 0) /* TCP */,
			    ndpi_build_default_ports(ports_b, 0, 0, 0, 0, 0) /* UDP */);

    def = &ndpi_str->proto_defaults[ndpi_str->ndpi_num_supported_protocols];
    subprotocol_id = ndpi_str->ndpi_num_supported_protocols;
    ndpi_str->ndpi_num_supported_protocols++, ndpi_str->ndpi_num_custom_protocols++;
  }

  while((elem = strsep(&rule, ",")) != NULL) {
    char *attr = elem, *value = NULL;
    ndpi_port_range range;
    int is_tcp = 0, is_udp = 0, is_ip = 0;
    u_int8_t is_ipv6_ip = 0;

    if(strncmp(attr, "tcp:", 4) == 0)
      is_tcp = 1, value = &attr[4];
    else if(strncmp(attr, "udp:", 4) == 0)
      is_udp = 1, value = &attr[4];
    else if(strncmp(attr, "ipv6:", 5) == 0)
      is_ip = 1, is_ipv6_ip = 1, value = &attr[5];
    else if(strncmp(attr, "ip:", 3) == 0)
      is_ip = 1, value = &attr[3];
    else if(strncmp(attr, "host:", 5) == 0) {
      /* host:"<value>",host:"<value>",.....@<subproto> */

      value = &attr[5];
      if(value[0] == '"')
	value++; /* remove leading " */

      if(value[0] != '\0') {
	u_int i, max_len = strlen(value) - 1;

	if(value[max_len] == '"')
	  value[max_len] = '\0'; /* remove trailing " */

	for(i=0; i<max_len; i++)
	  value[i] = tolower(value[i]);
      }
    } else if(strncmp(attr, "nbpf:", 5) == 0) {
#ifdef HAVE_NBPF
      char *filter = &attr[5];

      if(ndpi_str->num_nbpf_custom_proto >= MAX_NBPF_CUSTOM_PROTO) {
	NDPI_LOG_ERR(ndpi_str, "nBPF: too many protocols\n");
	return(-4); /* Too many protocols */
      }

      if(filter[0] == '"') {
	u_int len;

	filter = &filter[1];
	len = strlen(filter);

	if(len > 0)
	  filter[len-1] = '\0';
      }

      if((ndpi_str->nbpf_custom_proto[ndpi_str->num_nbpf_custom_proto].tree =
	  nbpf_parse(filter, NULL)) == NULL) {
	NDPI_LOG_ERR(ndpi_str, "nBPF invalid filter: %s\n", filter)
	  return(-5); /* Invalid filter */
      } else
	ndpi_str->nbpf_custom_proto[ndpi_str->num_nbpf_custom_proto].l7_protocol = subprotocol_id;

      ndpi_str->num_nbpf_custom_proto++;
#else
      NDPI_LOG_INFO(ndpi_str, "nDPI compiled without nBPF support: skipping rule\n");
      return(-6);
#endif
    }

    if(is_tcp || is_udp) {
      u_int p_low, p_high;
      int rc;

      if(sscanf(value, "%u-%u", &p_low, &p_high) == 2)
	range.port_low = p_low, range.port_high = p_high;
      else
	range.port_low = range.port_high = atoi(&elem[4]);

      rc = addDefaultPort(ndpi_str, &range, def, 1 /* Custom user proto */,
			  is_tcp ? &ndpi_str->tcpRoot : &ndpi_str->udpRoot, __FUNCTION__, __LINE__);

      if(rc != 0) ret = rc;
    } else if(is_ip) {
      int rc = ndpi_add_host_ip_subprotocol(ndpi_str, value, subprotocol_id, is_ipv6_ip);

      if(rc != 0)
	return(rc);
    } else {
      ndpi_add_host_url_subprotocol(ndpi_str, value, subprotocol_id, NDPI_PROTOCOL_CATEGORY_UNSPECIFIED,
				    NDPI_PROTOCOL_ACCEPTABLE, 0);
    }
  }

  return(ret);
}

/* ******************************************************************** */
#ifndef __KERNEL__
/*
 * Format:
 *
 * <proto,param,value>
 *
 * Notes:
 *  - proto might be empty
 *  - empty lines or lines starting with # are ignored
 */
int load_config_file_fd(struct ndpi_detection_module_struct *ndpi_str, FILE *fd) {
  char buffer[512], *line, *proto, *param = NULL, *value, *saveptr;
  int len, i, num_commas;
  ndpi_cfg_error rc;

  if(!ndpi_str || !fd)
    return -1;

  while(1) {
    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    len = strlen(line);

    if((len <= 1) || (line[0] == '#'))
      continue;

    line[len - 1] = '\0';

    /* First parameter might be missing */
    num_commas = 0;
    for(i = 0; i < len; i++) {
      if(line[i] == ',')
        num_commas++;
    }

    if(num_commas == 1) {
      proto = NULL;
      param = strtok_r(line, ",", &saveptr);
    } else if(num_commas == 2) {
      proto = strtok_r(line, ",", &saveptr);
      if(proto) {
        param = strtok_r(NULL, ",", &saveptr);
      }
    } else {
      NDPI_LOG_ERR(ndpi_str, "Error parsing [%s]\n", line);
      continue;
    }

    if(param) {
      value = strtok_r(NULL, ",", &saveptr);
      if(value) {
        rc = ndpi_set_config(ndpi_str, proto, param, value);
        if(rc != NDPI_CFG_OK) {
          NDPI_LOG_ERR(ndpi_str, "Error ndpi_set_config [%s/%s/%s]: %d\n",
                       proto, param, value, rc);
          return rc;
        }
        continue;
      }
    }
    NDPI_LOG_ERR(ndpi_str, "Error parsing [%s]\n", line);
    return -2;
  }
  return 0;
}

/* ******************************************************************** */

/*
 * Format:
 *
 * <host|ip>	<category_id>
 *
 * Notes:
 *  - host and category are separated by a single TAB
 *  - empty lines or lines starting with # are ignored
 */
int ndpi_load_categories_file(struct ndpi_detection_module_struct *ndpi_str,
			      const char *path, void *user_data) {
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_categories_file_fd(ndpi_str, fd, user_data);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

int load_categories_file_fd(struct ndpi_detection_module_struct *ndpi_str,
			    FILE *fd, void *user_data) {
  char buffer[512], *line, *name, *category, *saveptr;
  int len, num = 0, cat_id;

  if(!ndpi_str || !fd)
    return(-1);

  while(1) {
    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    len = strlen(line);

    if((len <= 1) || (line[0] == '#'))
      continue;

    line[len - 1] = '\0';
    name = strtok_r(line, "\t", &saveptr);

    if(name) {
      category = strtok_r(NULL, "\t", &saveptr);

      if(category) {
        const char *errstrp;
        cat_id = ndpi_strtonum(category, 1, NDPI_PROTOCOL_NUM_CATEGORIES - 1, &errstrp, 10);
        if(errstrp == NULL) {

          int rc = ndpi_load_category(ndpi_str, name,
				      (ndpi_protocol_category_t)cat_id,
				      user_data);

          if(rc >= 0)
	    num++;
        }
      }
    }
  }

  /*
    Not necessay to call ndpi_enable_loaded_categories() as
    ndpi_set_protocol_detection_bitmask2() will do that
  */
  /* ndpi_enable_loaded_categories(ndpi_str); */

  return(num);
}

/* ******************************************************************** */

/*
  Loads a file (separated by <cr>) of domain names associated with the
  specified category
*/
int ndpi_load_category_file(struct ndpi_detection_module_struct *ndpi_str,
			    char *path, ndpi_protocol_category_t category_id) {
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_category_file_fd(ndpi_str, fd, category_id);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

int load_category_file_fd(struct ndpi_detection_module_struct *ndpi_str,
			  FILE *fd, ndpi_protocol_category_t category_id) {
  char buffer[256], *line;
  u_int num_loaded = 0;
  unsigned int failed_lines = 0;
  unsigned int lines_read = 0;

  (void)lines_read;

  if(!ndpi_str || !fd || !ndpi_str->protocols)
    return(0);

  while(1) {
    int len;

    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    lines_read++;
    len = strlen(line);

    if(len <= 1 || len == sizeof(buffer) - 1) {
      NDPI_LOG_ERR(ndpi_str, "[NDPI] Failed to read file line #%u, line too short/long\n",
                   lines_read);
      failed_lines++;
      continue;
    } else if (line[0] == '#')
      continue;

    int i = 0;
    for (i = 0; i < len; ++i) {
      if (line[i] == '\r' || line[i] == '\n') {
        line[i] = '\0';
        break;
      }

      if (line[i] != '-'
	  && line[i] != '.'
	  && line[i] != ':'
	  && line[i] != '/'
	  && ndpi_isalnum(line[i]) == 0
          /* non standard checks for the sake of compatibility */
          && line[i] != '_')
        break;
    }

    if ((i != len - 2) && (i != len - 1) && (line[i] != '\0')) {
      NDPI_LOG_ERR(ndpi_str, "[NDPI] Failed to read file line #%u [%s], invalid characters [%c] found [pos: %u]\n",
		   lines_read, line, line[i], i);
      failed_lines++;
      continue;
    }

    if(ndpi_load_category(ndpi_str, line, category_id, NULL) >= 0)
      num_loaded++;
  }

  if(failed_lines)
    return(-1 * failed_lines);

  return(num_loaded);
}

/* ******************************************************************** */

int load_protocol_id_file_fd(struct ndpi_detection_module_struct *ndpi_str,
			     FILE *fd, u_int16_t protocol_id) {
  char buffer[256], *line;
  u_int num_loaded = 0;
  unsigned int failed_lines = 0;
  unsigned int lines_read = 0;

  (void)lines_read;

  if(!ndpi_str || !fd || !ndpi_str->protocols)
    return(0);

  while(1) {
    int len;

    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    lines_read++;
    len = strlen(line);

    if(len <= 1 || len == sizeof(buffer) - 1) {
      NDPI_LOG_ERR(ndpi_str, "[NDPI] Failed to read file line #%u, line too short/long\n",
                   lines_read);
      failed_lines++;
      continue;
    } else if (line[0] == '#')
      continue;

    int i = 0;

    for (i = 0; i < len; ++i) {
      if (line[i] == '\r' || line[i] == '\n') {
        line[i] = '\0';
        break;
      }
    }

    if(strchr(line, ':') != NULL) {
      if(ndpi_add_host_ip_subprotocol(ndpi_str, line, protocol_id, 1 /* IPv6 */) == 0)
	num_loaded++;
      else
	failed_lines++;
    } else if(strchr(line, '.') != NULL) {
      /* IPv4 */
      if(ndpi_add_host_ip_subprotocol(ndpi_str, line, protocol_id, 0 /* IPv4 */) == 0)
	num_loaded++;
      else
	failed_lines++;
    } else {
      /* No clue */
      failed_lines++;
      continue;
    }
  }

  if(failed_lines)
    return(-1 * failed_lines);

  return(num_loaded);
}

/* ******************************************************************** */

/*
  Loads a file (separated by <cr>) of IP addresses associated with the
  specified protocol
*/
int ndpi_load_protocol_id_file(struct ndpi_detection_module_struct *ndpi_str,
			       char *path, u_int16_t protocol_id) {
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_protocol_id_file_fd(ndpi_str, fd, protocol_id);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

/*
  Load files (whose name is <categoryid>_<label>.<extension>) stored
  in a directory and bind each domain to the specified category.

  It can be used to load all files store in the lists/ directory

  It returns the number of loaded files or -1 in case of failure
*/
int ndpi_load_categories_dir(struct ndpi_detection_module_struct *ndpi_str,
			     char *dir_path) {
  DIR *dirp;
  struct dirent *dp;
  int failed_files = 0;
  int num_loaded = 0;

  if(!ndpi_str || !dir_path)
    return(0);

  dirp = opendir(dir_path);
  if (dirp == NULL)
    return(0);

  while((dp = readdir(dirp)) != NULL) {
    char *underscore, *extn;

    if(dp->d_name[0] == '.') continue;
    extn = strrchr(dp->d_name, '.');

    if((extn == NULL) || strcmp(extn, ".list"))
      continue;

    /* Check if the format is <proto it>_<string>.<extension> */
    if((underscore = strchr(dp->d_name, '_')) != NULL) {
      int cat_id;
      const char *errstrp;

      underscore[0] = '\0';
      cat_id = ndpi_strtonum(dp->d_name, 1, NDPI_PROTOCOL_NUM_CATEGORIES - 1, &errstrp, 10);
      if(errstrp == NULL) {
	/* Valid file */
	char path[512];

	underscore[0] = '_';
	snprintf(path, sizeof(path), "%s/%s", dir_path, dp->d_name);

	if (ndpi_load_category_file(ndpi_str, path, (ndpi_protocol_category_t)cat_id) < 0) {
	  NDPI_LOG_ERR(ndpi_str, "Failed to load '%s'\n", path);
	  failed_files++;
	}else
	  num_loaded++;
      }
    }
  }

  (void)closedir(dirp);

  if(failed_files)
    return(-1 * failed_files);

  return(num_loaded);
}

/* ******************************************************************** */

/*
  Load files (whose name is <protocolid>_<label>.<extension>) stored
  in a directory and bind each domain to the specified protocol.

  It can be used to load all files store in the lists/ directory

  It returns the number of loaded files or -1 in case of failure
*/
int ndpi_load_protocols_dir(struct ndpi_detection_module_struct *ndpi_str,
			    char *dir_path) {
  DIR *dirp;
  struct dirent *dp;
  int failed_files = 0;
  int num_loaded = 0;

  if(!ndpi_str || !dir_path)
    return(0);

  dirp = opendir(dir_path);
  if (dirp == NULL)
    return(0);

  while((dp = readdir(dirp)) != NULL) {
    char *underscore, *extn;

    if(dp->d_name[0] == '.') continue;
    extn = strrchr(dp->d_name, '.');

    if((extn == NULL) || strcmp(extn, ".list"))
      continue;

    /* Check if the format is <proto it>_<string>.<extension> */
    if((underscore = strchr(dp->d_name, '_')) != NULL) {
      int proto_id;
      const char *errstrp;

      underscore[0] = '\0';
      proto_id = ndpi_strtonum(dp->d_name, 1, NDPI_LAST_IMPLEMENTED_PROTOCOL - 1, &errstrp, 10);
      if(errstrp == NULL) {
	/* Valid file */
	char path[512];

	underscore[0] = '_';
	snprintf(path, sizeof(path), "%s/%s", dir_path, dp->d_name);

	if (ndpi_load_protocol_id_file(ndpi_str, path, proto_id) < 0) {
	  NDPI_LOG_ERR(ndpi_str, "Failed to load '%s'\n", path);
	  failed_files++;
	}else
	  num_loaded++;
      }
    }
  }

  (void)closedir(dirp);

  if(failed_files)
    return(-1 * failed_files);

  return(num_loaded);
}

/* ******************************************************************** */

static int ndpi_load_risky_domain(struct ndpi_detection_module_struct *ndpi_str,
				  char* domain_name) {
  if(ndpi_str->risky_domain_automa.ac_automa == NULL) {
    ndpi_str->risky_domain_automa.ac_automa = ac_automata_init(ac_domain_match_handler);
    if(!ndpi_str->risky_domain_automa.ac_automa) return -1;
    ac_automata_feature(ndpi_str->risky_domain_automa.ac_automa,AC_FEATURE_LC);
    ac_automata_name(ndpi_str->risky_domain_automa.ac_automa, "risky", 0);
  }

  return ndpi_string_to_automa(ndpi_str, (AC_AUTOMATA_t *)ndpi_str->risky_domain_automa.ac_automa,
			       domain_name, 1, 0, 0, 0, 1); /* domain, protocol, category, breed, level , at_end */
}

/* ******************************************************************** */

/*
 * Format:
 *
 * <domain name>
 *
 * Notes:
 *  - you can add a .<domain name> to avoid mismatches
 */
int ndpi_load_risk_domain_file(struct ndpi_detection_module_struct *ndpi_str, const char *path) {
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_risk_domain_file_fd(ndpi_str, fd);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

int load_risk_domain_file_fd(struct ndpi_detection_module_struct *ndpi_str, FILE *fd) {
  char buffer[128], *line;
  int len, num = 0;

  if(!ndpi_str || !fd)
    return(-1);

  while(1) {
    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    len = strlen(line);

    if((len <= 1) || (line[0] == '#'))
      continue;

    line[len - 1] = '\0';

    if(ndpi_load_risky_domain(ndpi_str, line) >= 0)
      num++;
  }

  if(ndpi_str->risky_domain_automa.ac_automa)
    ac_automata_finalize((AC_AUTOMATA_t *)ndpi_str->risky_domain_automa.ac_automa);

  return(num);
}

/* ******************************************************************** */
/*
 * Format:
 *
 * <ja4 hash>[,<other info>]
 *
 */
int ndpi_load_malicious_ja4_file(struct ndpi_detection_module_struct *ndpi_str, const char *path) {
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_malicious_ja4_file_fd(ndpi_str, fd);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

int load_malicious_ja4_file_fd(struct ndpi_detection_module_struct *ndpi_str, FILE *fd) {
  char buffer[128], *line;
  int len, num = 0;

  if(!ndpi_str || !fd)
    return(-1);
  if(ndpi_str->malicious_ja4_hashmap == NULL && ndpi_hash_init(&ndpi_str->malicious_ja4_hashmap) != 0)
    return(-1);

  while(1) {
    char *comma;

    line = fgets(buffer, sizeof(buffer), fd);

    if(line == NULL)
      break;

    len = strlen(line);

    if((len <= 1) || (line[0] == '#'))
      continue;

    line[len - 1] = '\0';

    if((comma = strchr(line, ',')) != NULL)
      comma[0] = '\0';

    len = strlen(line);

    if(len != 36 /* size of JA4C */) {
      NDPI_LOG_ERR(ndpi_str, "Not a JA4C: [%s]\n", line);
      continue;
    }

    if(ndpi_hash_add_entry(&ndpi_str->malicious_ja4_hashmap, line, len, 0) == 0)
      num++;
  }

  return(num);
}
/* ******************************************************************** */

/*
 * Format:
 *
 * <sha1 hash>
 * <other info>,<sha1 hash>
 * <other info>,<sha1 hash>[,<other info>[...]]
 *
 */
int ndpi_load_malicious_sha1_file(struct ndpi_detection_module_struct *ndpi_str, const char *path)
{
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_malicious_sha1_file_fd(ndpi_str, fd);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

int load_malicious_sha1_file_fd(struct ndpi_detection_module_struct *ndpi_str, FILE *fd)
{
  char buffer[128];
  char *first_comma, *second_comma;
  size_t i, len;
  int num = 0;

  if(!ndpi_str || !fd)
    return(-1);
  if(ndpi_str->malicious_sha1_hashmap == NULL && ndpi_hash_init(&ndpi_str->malicious_sha1_hashmap) != 0)
    return(-1);

  while (fgets(buffer, sizeof(buffer), fd) != NULL) {
    len = strlen(buffer);

    if(len <= 1 || buffer[0] == '#')
      continue;

    first_comma = strchr(buffer, ',');
    if(first_comma != NULL) {
      first_comma++;
      second_comma = strchr(first_comma, ',');
      if(second_comma == NULL)
        second_comma = &buffer[len - 1];
    } else {
      first_comma = &buffer[0];
      second_comma = &buffer[len - 1];
    }

    second_comma[0] = '\0';
    if((second_comma - first_comma) != 40) {
      NDPI_LOG_ERR(ndpi_str, "Not a SSL certificate sha1 hash: [%s]\n", first_comma);
      continue;
    }

    for (i = 0; i < 40; ++i)
      first_comma[i] = toupper(first_comma[i]);

    if(ndpi_hash_add_entry(&ndpi_str->malicious_sha1_hashmap, first_comma,
			   second_comma - first_comma, 0) == 0)
      num++;
  }

  return num;
}

#endif // __KERNEL__

/* ******************************************************************** */

/*
  Format:
  <tcp|udp>:<port>,<tcp|udp>:<port>,.....@<proto>

  Subprotocols Format:
  host:"<value>",host:"<value>",.....@<subproto>

  IP based Subprotocols Format (<value> is IP or CIDR):
  ip:<value>,ip:<value>,.....@<subproto>

  nBPF-based Filters
  nbpf:"<nBPF filter>@<proto>

  Example:
  tcp:80,tcp:3128@HTTP
  udp:139@NETBIOS

*/
NDPI_STATIC int ndpi_load_protocols_file(struct ndpi_detection_module_struct *ndpi_str, const char* path) {
#ifdef __KERNEL__
  return -1;
#else
  int rc;
  FILE *fd;

  if(!ndpi_str || !path)
    return(-1);

  fd = fopen(path, "r");
  if(fd == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Unable to open file %s [%s]\n", path, strerror(errno));
    return -1;
  }

  rc = load_protocols_file_fd(ndpi_str, fd);

  fclose(fd);

  return rc;
}

/* ******************************************************************** */

int load_protocols_file_fd(struct ndpi_detection_module_struct *ndpi_str, FILE *fd) {
  char *buffer, *old_buffer;
  int chunk_len = 1024, buffer_len = chunk_len, old_buffer_len;
  int i;

  if(!ndpi_str || !fd)
    return -1;

  buffer = ndpi_malloc(buffer_len);
  if(buffer == NULL) {
    NDPI_LOG_ERR(ndpi_str, "Memory allocation failure\n");
    return -2;
  }

  while(1) {
    char *line = buffer;
    int line_len = buffer_len;

    while(((line = fgets(line, line_len, fd)) != NULL)
	  && strlen(line) > 0
	  && (line[strlen(line) - 1] != '\n')) {
      i = strlen(line);
      old_buffer = buffer;
      old_buffer_len = buffer_len;
      buffer_len += chunk_len;

      buffer = ndpi_realloc(old_buffer, old_buffer_len, buffer_len);
      if(buffer == NULL) {
	NDPI_LOG_ERR(ndpi_str, "Memory allocation failure\n");
	ndpi_free(old_buffer);
	return -2;
      }

      line = &buffer[i];
      line_len = chunk_len;
    } /* while */

    if(!line) /* safety check */
      break;

    i = strlen(buffer);
    if((i <= 1) || (buffer[0] == '#'))
      continue;
    else {
      buffer[i - 1] = '\0';
      i--;

      if((i > 0) && (buffer[i-1] == '\r'))
	buffer[i - 1] = '\0';

      if(buffer[0] == '\0')
	continue;
    }

    /* printf("Processing: \"%s\"\n", buffer); */

    if(ndpi_handle_rule(ndpi_str, buffer) != 0)
      NDPI_LOG_INFO(ndpi_str, "Discraded rule '%s'\n", buffer);
  }

  ndpi_free(buffer);

  return 0;
#endif
}

/* ******************************************************************** */

void register_dissector(char *dissector_name, struct ndpi_detection_module_struct *ndpi_str,
                        void (*func)(struct ndpi_detection_module_struct *,
                                     struct ndpi_flow_struct *flow),
                        const NDPI_SELECTION_BITMASK_PROTOCOL_SIZE ndpi_selection_bitmask,
                        int num_protocol_ids, ...)
{
  va_list ap;
  int i, dissector_enabled = 0, first_protocol_id = -1;
  u_int32_t idx = ndpi_str->callback_buffer_num;

  if(idx >= NDPI_MAX_NUM_DISSECTORS) {
    /*
     * You need to increase NDPI_MAX_NUM_DISSECTORS define and recompile everything!
     * Please note that custom protocols are independent from NDPI_MAX_NUM_DISSECTORS, so
     * if you hit this error is because you are already changing the code
     * (adding a new dissector)...
     */
    NDPI_LOG_ERR(ndpi_str, "[NDPI] Internal Error. Too many dissectors!!\n");
    /* Not sure what to do here...*/
    return;
  }

  va_start(ap, num_protocol_ids);
  for(i = 0; i < num_protocol_ids; i++) {
    int ndpi_protocol_id = va_arg(ap, int);
    if(!is_proto_enabled(ndpi_str, ndpi_protocol_id)) {
      NDPI_LOG_DBG(ndpi_str, "Protocol %d not enabled for dissector %s\n",
                   ndpi_protocol_id, dissector_name);
    } else {

      if(ndpi_str->proto_defaults[ndpi_protocol_id].dissector_idx != 0) {
        NDPI_LOG_ERR(ndpi_str, "Internal error: protocol %d/%s has been already registered (%d/%d)\n",
                     ndpi_protocol_id, dissector_name,
                     ndpi_str->proto_defaults[ndpi_protocol_id].dissector_idx,
                     idx);
        /* TODO */
      } else {

        if(first_protocol_id == -1)
          first_protocol_id = ndpi_protocol_id;

        ndpi_str->proto_defaults[ndpi_protocol_id].dissector_idx = idx;
      }
      dissector_enabled = 1;
    }
  }
  va_end(ap);

  if(dissector_enabled) {
    NDPI_LOG_DBG2(ndpi_str, "Dissector %s enabled. Registering %d...\n", dissector_name, idx);

    memcpy(ndpi_str->callback_buffer[idx].name, dissector_name,
           ndpi_min(sizeof(ndpi_str->callback_buffer[idx].name) - 1, strlen(dissector_name)));
    ndpi_str->callback_buffer[idx].func = func;
    ndpi_str->callback_buffer[idx].dissector_idx = idx;
    ndpi_str->callback_buffer[idx].ndpi_selection_bitmask = ndpi_selection_bitmask;
    ndpi_str->callback_buffer[idx].first_protocol_id = first_protocol_id; /* Just for logging */

    ndpi_str->callback_buffer_num++;
  } else {
    NDPI_LOG_DBG(ndpi_str, "Dissector %s disabled\n", dissector_name);
  }
  return;
}

/* ******************************************************************** */

static int ndpi_callback_init(struct ndpi_detection_module_struct *ndpi_str) {

  struct call_function_struct *all_cb = NULL;

  if(ndpi_str->callback_buffer)
    ndpi_free(ndpi_str->callback_buffer);

  ndpi_str->callback_buffer = ndpi_calloc(NDPI_MAX_SUPPORTED_PROTOCOLS+1,sizeof(struct call_function_struct));
  if(!ndpi_str->callback_buffer) return 1;

  ndpi_str->callback_buffer_num = 0;
  /* HTTP */
  init_http_dissector(ndpi_str);

  /* BLIZZARD */
  init_blizzard_dissector(ndpi_str);

  /* TLS+DTLS */
  init_tls_dissector(ndpi_str);

  /* RTP */
  init_rtp_dissector(ndpi_str);

  /* RTSP */
  init_rtsp_dissector(ndpi_str);

  /* RDP */
  init_rdp_dissector(ndpi_str);

  /* STUN */
  init_stun_dissector(ndpi_str);

  /* SIP */
  init_sip_dissector(ndpi_str);

  /* IMO */
  init_imo_dissector(ndpi_str);

  /* Teredo */
  init_teredo_dissector(ndpi_str);

  /* EDONKEY */
  init_edonkey_dissector(ndpi_str);

  /* GNUTELLA */
  init_gnutella_dissector(ndpi_str);

  /* NATS */
  init_nats_dissector(ndpi_str);

  /* SOCKS */
  init_socks_dissector(ndpi_str);

  /* IRC */
  init_irc_dissector(ndpi_str);

  /* JABBER */
  init_jabber_dissector(ndpi_str);

  /* MAIL_POP */
  init_mail_pop_dissector(ndpi_str);

  /* MAIL_IMAP */
  init_mail_imap_dissector(ndpi_str);

  /* MAIL_SMTP */
  init_mail_smtp_dissector(ndpi_str);

  /* USENET */
  init_usenet_dissector(ndpi_str);

  /* DNS */
  init_dns_dissector(ndpi_str);

  /* VMWARE */
  init_vmware_dissector(ndpi_str);

  /* NON_TCP_UDP */
  init_non_tcp_udp_dissector(ndpi_str);

  /* IAX */
  init_iax_dissector(ndpi_str);

  /* Media Gateway Control Protocol */
  init_mgcp_dissector(ndpi_str);

  /* ZATTOO */
  init_zattoo_dissector(ndpi_str);

  /* QQ */
  init_qq_dissector(ndpi_str);

  /* SSH */
  init_ssh_dissector(ndpi_str);

  /* VNC */
  init_vnc_dissector(ndpi_str);

  /* VXLAN */
  init_vxlan_dissector(ndpi_str);

  /* TEAMVIEWER */
  init_teamviewer_dissector(ndpi_str);

  /* DHCP */
  init_dhcp_dissector(ndpi_str);

  /* STEAM */
  init_steam_dissector(ndpi_str);

  /* XBOX */
  init_xbox_dissector(ndpi_str);

  /* SMB */
  init_smb_dissector(ndpi_str);

  /* MINING */
  init_mining_dissector(ndpi_str);

  /* TELNET */
  init_telnet_dissector(ndpi_str);

  /* NTP */
  init_ntp_dissector(ndpi_str);

  /* NFS */
  init_nfs_dissector(ndpi_str);

  /* SSDP */
  init_ssdp_dissector(ndpi_str);

  /* POSTGRES */
  init_postgres_dissector(ndpi_str);

  /* MYSQL */
  init_mysql_dissector(ndpi_str);

  /* BGP */
  init_bgp_dissector(ndpi_str);

  /* SNMP */
  init_snmp_dissector(ndpi_str);

  /* ICECAST */
  init_icecast_dissector(ndpi_str);

  /* KERBEROS */
  init_kerberos_dissector(ndpi_str);

  /* SYSLOG */
  init_syslog_dissector(ndpi_str);

  /* NETBIOS */
  init_netbios_dissector(ndpi_str);

  /* IPP */
  init_ipp_dissector(ndpi_str);

  /* LDAP */
  init_ldap_dissector(ndpi_str);

  /* XDMCP */
  init_xdmcp_dissector(ndpi_str);

  /* TFTP */
  init_tftp_dissector(ndpi_str);

  /* MSSQL_TDS */
  init_mssql_tds_dissector(ndpi_str);

  /* PPTP */
  init_pptp_dissector(ndpi_str);

  /* DHCPV6 */
  init_dhcpv6_dissector(ndpi_str);

  /* AFP */
  init_afp_dissector(ndpi_str);

  /* check_mk */
  init_checkmk_dissector(ndpi_str);

  /* cpha */
  init_cpha_dissector(ndpi_str);

  /* NEXON */
  init_nexon_dissector(ndpi_str);

  /* DOFUS */
  init_dofus_dissector(ndpi_str);

  /* CROSSIFIRE */
  init_crossfire_dissector(ndpi_str);

  /* Guild Wars 2 */
  init_guildwars2_dissector(ndpi_str);

  /* ARMAGETRON */
  init_armagetron_dissector(ndpi_str);

  /* DROPBOX */
  init_dropbox_dissector(ndpi_str);

  /* SONOS */
  init_sonos_dissector(ndpi_str);

  /* SPOTIFY */
  init_spotify_dissector(ndpi_str);

  /* RADIUS */
  init_radius_dissector(ndpi_str);

  /* CITRIX */
  init_citrix_dissector(ndpi_str);

  /* HCL Notes */
  init_hcl_notes_dissector(ndpi_str);

  /* GTP */
  init_gtp_dissector(ndpi_str);

  /* HSRP */
  init_hsrp_dissector(ndpi_str);

  /* DCERPC */
  init_dcerpc_dissector(ndpi_str);

  /* NETFLOW */
  init_netflow_dissector(ndpi_str);

  /* SFLOW */
  init_sflow_dissector(ndpi_str);

  /* H323 */
  init_h323_dissector(ndpi_str);

  /* OPENVPN */
  init_openvpn_dissector(ndpi_str);

  /* NOE */
  init_noe_dissector(ndpi_str);

  /* CISCOVPN */
  init_ciscovpn_dissector(ndpi_str);

  /* TEAMSPEAK */
  init_teamspeak_dissector(ndpi_str);

  /* SKINNY */
  init_skinny_dissector(ndpi_str);

  /* RSYNC */
  init_rsync_dissector(ndpi_str);

  /* WHOIS_DAS */
  init_whois_das_dissector(ndpi_str);

  /* ORACLE */
  init_oracle_dissector(ndpi_str);

  /* CORBA */
  init_corba_dissector(ndpi_str);

  /* RTMP */
  init_rtmp_dissector(ndpi_str);

  /* FTP_CONTROL */
  init_ftp_control_dissector(ndpi_str);

  /* FTP_DATA */
  init_ftp_data_dissector(ndpi_str);

  /* MEGACO */
  init_megaco_dissector(ndpi_str);

  /* RESP */
  init_resp_dissector(ndpi_str);

  /* ZMQ */
  init_zmq_dissector(ndpi_str);

  /* TELEGRAM */
  init_telegram_dissector(ndpi_str);

  /* QUIC */
  init_quic_dissector(ndpi_str);

  /* DIAMETER */
  init_diameter_dissector(ndpi_str);

  /* APPLE_PUSH */
  init_apple_push_dissector(ndpi_str);

  /* EAQ */
  init_eaq_dissector(ndpi_str);

  /* KAKAOTALK_VOICE */
  init_kakaotalk_voice_dissector(ndpi_str);

  /* MIKROTIK */
  init_mikrotik_dissector(ndpi_str);

  /* MPEGTS */
  init_mpegts_dissector(ndpi_str);

  /* UBNTAC2 */
  init_ubntac2_dissector(ndpi_str);

  /* COAP */
  init_coap_dissector(ndpi_str);

  /* MQTT */
  init_mqtt_dissector(ndpi_str);

  /* SOME/IP */
  init_someip_dissector(ndpi_str);

  /* RX */
  init_rx_dissector(ndpi_str);

  /* GIT */
  init_git_dissector(ndpi_str);

  /* DRDA */
  init_drda_dissector(ndpi_str);

  /* BJNP */
  init_bjnp_dissector(ndpi_str);

  /* SMPP */
  init_smpp_dissector(ndpi_str);

  /* TINC */
  init_tinc_dissector(ndpi_str);

  /* FIX */
  init_fix_dissector(ndpi_str);

  /* NINTENDO */
  init_nintendo_dissector(ndpi_str);

  /* MODBUS */
  init_modbus_dissector(ndpi_str);

  /* CAPWAP */
  init_capwap_dissector(ndpi_str);

  /* ZABBIX */
  init_zabbix_dissector(ndpi_str);

  /*** Put false-positive sensitive protocols at the end ***/

  /* VIBER */
  init_viber_dissector(ndpi_str);

  /* BITTORRENT */
  init_bittorrent_dissector(ndpi_str);

  /* WHATSAPP */
  init_whatsapp_dissector(ndpi_str);

  /* OOKLA */
  init_ookla_dissector(ndpi_str);

  /* AMQP */
  init_amqp_dissector(ndpi_str);

  /* Steam Datagram Relay */
  init_valve_sdr_dissector(ndpi_str);

  /* LISP */
  init_lisp_dissector(ndpi_str);

  /* AJP */
  init_ajp_dissector(ndpi_str);

  /* Memcached */
  init_memcached_dissector(ndpi_str);

  /* Nest Log Sink */
  init_nest_log_sink_dissector(ndpi_str);

  /* WireGuard VPN */
  init_wireguard_dissector(ndpi_str);

  /* Amazon_Video */
  init_amazon_video_dissector(ndpi_str);

  /* S7 comm */
  init_s7comm_dissector(ndpi_str);

  /* IEC 60870-5-104 */
  init_104_dissector(ndpi_str);

  /* DNP3 */
  init_dnp3_dissector(ndpi_str);

  /* WEBSOCKET */
  init_websocket_dissector(ndpi_str);

  /* SOAP */
  init_soap_dissector(ndpi_str);

  /* DNScrypt */
  init_dnscrypt_dissector(ndpi_str);

  /* MongoDB */
  init_mongodb_dissector(ndpi_str);

  /* AmongUS */
  init_among_us_dissector(ndpi_str);

  /* HP Virtual Machine Group Management */
  init_hpvirtgrp_dissector(ndpi_str);

  /* Genshin Impact */
  init_genshin_impact_dissector(ndpi_str);

  /* Z39.50 international standard client–server, application layer communications protocol */
  init_z3950_dissector(ndpi_str);

  /* AVAST SecureDNS */
  init_avast_securedns_dissector(ndpi_str);

  /* Cassandra */
  init_cassandra_dissector(ndpi_str);

  /* EthernetIP */
  init_ethernet_ip_dissector(ndpi_str);

  /* WSD */
  init_wsd_dissector(ndpi_str);

  /* TocaBoca */
  init_toca_boca_dissector(ndpi_str);

  /* SD-RTN Software Defined Real-time Network */
  init_sd_rtn_dissector(ndpi_str);

  /* RakNet */
  init_raknet_dissector(ndpi_str);

  /* Xiaomi */
  init_xiaomi_dissector(ndpi_str);

  /* MpegDash */
  init_mpegdash_dissector(ndpi_str);

  /* RSH */
  init_rsh_dissector(ndpi_str);

  /* IPsec */
  init_ipsec_dissector(ndpi_str);

  /* collectd */
  init_collectd_dissector(ndpi_str);

  /* i3D */
  init_i3d_dissector(ndpi_str);

  /* RiotGames */
  init_riotgames_dissector(ndpi_str);

  /* UltraSurf */
  init_ultrasurf_dissector(ndpi_str);

  /* Threema */
  init_threema_dissector(ndpi_str);

  /* AliCloud */
  init_alicloud_dissector(ndpi_str);

  /* AVAST */
  init_avast_dissector(ndpi_str);

  /* Softether */
  init_softether_dissector(ndpi_str);

  /* Activision */
  init_activision_dissector(ndpi_str);

  /* Discord */
  init_discord_dissector(ndpi_str);

  /* TiVoConnect */
  init_tivoconnect_dissector(ndpi_str);

  /* Kismet */
  init_kismet_dissector(ndpi_str);

  /* FastCGI */
  init_fastcgi_dissector(ndpi_str);

  /* NATPMP */
  init_natpmp_dissector(ndpi_str);

  /* Syncthing */
  init_syncthing_dissector(ndpi_str);

  /* CryNetwork */
  init_crynet_dissector(ndpi_str);

  /* Line voip */
  init_line_dissector(ndpi_str);

  /* Munin */
  init_munin_dissector(ndpi_str);

  /* Elasticsearch */
  init_elasticsearch_dissector(ndpi_str);

  /* TUYA LP */
  init_tuya_lp_dissector(ndpi_str);

  /* TPLINK_SHP */
  init_tplink_shp_dissector(ndpi_str);

  /* Meraki Cloud */
  init_merakicloud_dissector(ndpi_str);

  /* Tailscale */
  init_tailscale_dissector(ndpi_str);

  /* Source Engine */
  init_source_engine_dissector(ndpi_str);

  /* BACnet */
  init_bacnet_dissector(ndpi_str);

  /* OICQ */
  init_oicq_dissector(ndpi_str);

  /* Heroes of the Storm */
  init_hots_dissector(ndpi_str);

  /* EpicGames */
  init_epicgames_dissector(ndpi_str);

  /*BITCOIN*/
  init_bitcoin_dissector(ndpi_str);

  /* Apache Thrift */
  init_apache_thrift_dissector(ndpi_str);

  /* Service Location Protocol */
  init_slp_dissector(ndpi_str);

  /* HTTP/2 */
  init_http2_dissector(ndpi_str);

  /* HAProxy */
  init_haproxy_dissector(ndpi_str);

  /* RMCP */
  init_rmcp_dissector(ndpi_str);

  /* Controller Area Network */
  init_can_dissector(ndpi_str);

  /* Protobuf */
  init_protobuf_dissector(ndpi_str);

  /* ETHEREUM */
  init_ethereum_dissector(ndpi_str);

  /* Precision Time Protocol v2 */
  init_ptpv2_dissector(ndpi_str);

  /* Highway Addressable Remote Transducer over IP */
  init_hart_ip_dissector(ndpi_str);

  /* Real-time Publish-Subscribe Protocol */
  init_rtps_dissector(ndpi_str);

  /* OPC Unified Architecture */
  init_opc_ua_dissector(ndpi_str);

  /* Factory Interface Network Service */
  init_fins_dissector(ndpi_str);

  /* Ether-S-I/O */
  init_ethersio_dissector(ndpi_str);

  /* Automation Device Specification */
  init_beckhoff_ads_dissector(ndpi_str);

  /* Manufacturing Message Specification */
  init_iso9506_1_mms_dissector(ndpi_str);

  /* IEEE C37.118 Synchrophasor Protocol */
  init_ieee_c37118_dissector(ndpi_str);

  /* Ether-S-Bus */
  init_ethersbus_dissector(ndpi_str);

  /* Monero Protocol */
  init_monero_dissector(ndpi_str);

  /* PROFINET/IO */
  init_profinet_io_dissector(ndpi_str);

  /* HiSLIP */
  init_hislip_dissector(ndpi_str);

  /* UFTP */
  init_uftp_dissector(ndpi_str);

  /* OpenFlow */
  init_openflow_dissector(ndpi_str);

  /* JSON-RPC */
  init_json_rpc_dissector(ndpi_str);

  /* Apache Kafka */
  init_kafka_dissector(ndpi_str);

  /* NoMachine */
  init_nomachine_dissector(ndpi_str);

  /* IEC 62056 */
  init_iec62056_dissector(ndpi_str);

  /* HL7 */
  init_hl7_dissector(ndpi_str);

  /* DICOM */
  init_dicom_dissector(ndpi_str);

  /* Ceph */
  init_ceph_dissector(ndpi_str);

  /* Roughtime */
  init_roughtime_dissector(ndpi_str);

  /* KCP */
  init_kcp_dissector(ndpi_str);

  /* Mumble */
  init_mumble_dissector(ndpi_str);

  /* Zoom */
  init_zoom_dissector(ndpi_str);

  /* Yojimbo */
  init_yojimbo_dissector(ndpi_str);

  /* STOMP */
  init_stomp_dissector(ndpi_str);

  /* RDP */
  init_radmin_dissector(ndpi_str);

  /* Raft */
  init_raft_dissector(ndpi_str);

  /* CIP (Common Industrial Protocol) */
  init_cip_dissector(ndpi_str);

  /* Gearman */
  init_gearman_dissector(ndpi_str);

  /* Tencent Games */
  init_tencent_games_dissector(ndpi_str);

  /* Gaijin Entertainment */
  init_gaijin_dissector(ndpi_str);

  /* ANSI C12.22 / IEEE 1703 */
  init_c1222_dissector(ndpi_str);

  /* Dynamic Link Exchange Protocol */
  init_dlep_dissector(ndpi_str);

  /* Bidirectional Forwarding Detection */
  init_bfd_dissector(ndpi_str);

  /* NetEase Games */
  init_netease_games_dissector(ndpi_str);

  /* Path of Exile */
  init_pathofexile_dissector(ndpi_str);

  /* Packet Forwarding Control Protocol */
  init_pfcp_dissector(ndpi_str);

  /* File Delivery over Unidirectional Transport */
  init_flute_dissector(ndpi_str);

  /* League of Legends: Wild Rift */
  init_lolwildrift_dissector(ndpi_str);

  /* The Elder Scrolls Online */
  init_teso_dissector(ndpi_str);

  /* Label Distribution Protocol */
  init_ldp_dissector(ndpi_str);

  /* KNXnet/IP */
  init_knxnet_ip_dissector(ndpi_str);

  /* Binary Floor Control Protocol */
  init_bfcp_dissector(ndpi_str);

  /* iQIYI */
  init_iqiyi_dissector(ndpi_str);

  /* Ethernet Global Data */
  init_egd_dissector(ndpi_str);

  /* Call of Duty: Mobile */
  init_cod_mobile_dissector(ndpi_str);

  /* ZUG */
  init_zug_dissector(ndpi_str);

  /* JRMI Java Remote Method Invocation*/
  init_jrmi_dissector(ndpi_str);

  /* (Magellan) Ripe Atlas */
  init_ripe_atlas_dissector(ndpi_str);

  /* Cloudflare WARP */
  init_cloudflare_warp_dissector(ndpi_str);

  /* Nano Cryptocurrency Protocol */
  init_nano_dissector(ndpi_str);

  /* OpenWire */
  init_openwire_dissector(ndpi_str);

  /* ISO/IEC 14908-4 */
  init_cnp_ip_dissector(ndpi_str);

  /* ATG */
  init_atg_dissector(ndpi_str);

  /* Train Real Time Data Protocol */
  init_trdp_dissector(ndpi_str);

  /* Lustre */
  init_lustre_dissector(ndpi_str);

  /* DingTalk */
  init_dingtalk_dissector(ndpi_str);

  /* Paltalk */
  init_paltalk_dissector(ndpi_str);

  /* LagoFast */
  init_lagofast_dissector(ndpi_str);

  /* GearUP Booster */
  init_gearup_booster_dissector(ndpi_str);

  /* Microsoft Delivery Optimization */
  init_msdo_dissector(ndpi_str);

  /* MELSEC Communication Protocol */
  init_melsec_dissector(ndpi_str);

  /* Hamachi */
  init_hamachi_dissector(ndpi_str);

#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../../nDPI-custom/custom_ndpi_main_init.c"
#endif

  /* ----------------------------------------------------------------- */

  ndpi_str->callback_buffer_size = ndpi_str->callback_buffer_num;

  /* Resize callback_buffer */
  all_cb = ndpi_calloc(ndpi_str->callback_buffer_size+1,sizeof(struct call_function_struct));
  if(all_cb) {
    memcpy((char *)all_cb,(char *)ndpi_str->callback_buffer, (ndpi_str->callback_buffer_size+1) * sizeof(struct call_function_struct));
    ndpi_free(ndpi_str->callback_buffer);
    ndpi_str->callback_buffer = all_cb;
  }

  NDPI_LOG_DBG2(ndpi_str, "callback_buffer_size is %u\n", ndpi_str->callback_buffer_size);
  /* Calculating the size of an array for callback functions */
  ndpi_enabled_callbacks_init(ndpi_str, 1);
  all_cb = ndpi_calloc(ndpi_str->callback_buffer_size_tcp_payload +
		       ndpi_str->callback_buffer_size_tcp_no_payload +
		       ndpi_str->callback_buffer_size_udp +
		       ndpi_str->callback_buffer_size_non_tcp_udp,
		       sizeof(struct call_function_struct));
  if(!all_cb) return 1;

  if(ndpi_str->callback_buffer_tcp_payload)
	  ndpi_free(ndpi_str->callback_buffer_tcp_payload);
  ndpi_str->callback_buffer_tcp_payload = all_cb;
  all_cb += ndpi_str->callback_buffer_size_tcp_payload;
  ndpi_str->callback_buffer_tcp_no_payload = all_cb;
  all_cb += ndpi_str->callback_buffer_size_tcp_no_payload;
  ndpi_str->callback_buffer_udp = all_cb;
  all_cb += ndpi_str->callback_buffer_size_udp;
  ndpi_str->callback_buffer_non_tcp_udp = all_cb;

  ndpi_enabled_callbacks_init(ndpi_str, 0);

  NDPI_LOG_DBG(ndpi_str, "Tot num dissectors: %d (TCP: %d, TCP_NO_PAYLOAD: %d, UDP: %d, NO_TCP_UDP: %d)\n",
               ndpi_str->callback_buffer_size,
               ndpi_str->callback_buffer_size_tcp_payload,
               ndpi_str->callback_buffer_size_tcp_no_payload,
               ndpi_str->callback_buffer_size_udp,
               ndpi_str->callback_buffer_size_non_tcp_udp);

  return 0;
}

/* ******************************************************************** */

static inline int ndpi_proto_cb_tcp_payload(const struct ndpi_detection_module_struct *ndpi_str, uint32_t idx) {
  return (ndpi_str->callback_buffer[idx].ndpi_selection_bitmask &
	  (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC)) != 0;
}

/* ******************************************************************** */

static inline int ndpi_proto_cb_tcp_nopayload(const struct ndpi_detection_module_struct *ndpi_str, uint32_t idx) {
  return (ndpi_str->callback_buffer[idx].ndpi_selection_bitmask &
	  (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC)) != 0
    && (ndpi_str->callback_buffer[idx].ndpi_selection_bitmask &
	NDPI_SELECTION_BITMASK_PROTOCOL_HAS_PAYLOAD) == 0;
}

/* ******************************************************************** */

static inline int ndpi_proto_cb_udp(const struct ndpi_detection_module_struct *ndpi_str, uint32_t idx) {
  return (ndpi_str->callback_buffer[idx].ndpi_selection_bitmask &
	  (NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC)) != 0;
}

/* ******************************************************************** */

static inline int ndpi_proto_cb_other(const struct ndpi_detection_module_struct *ndpi_str, uint32_t idx) {
  return (ndpi_str->callback_buffer[idx].ndpi_selection_bitmask &
	  (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP |
	   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP)) == 0
    ||
    (ndpi_str->callback_buffer[idx].ndpi_selection_bitmask &
     NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC) != 0;
}

/* ******************************************************************** */

static void ndpi_enabled_callbacks_init(struct ndpi_detection_module_struct *ndpi_str,
					int count_only) {
  uint32_t a;

  /* now build the specific buffer for tcp, udp and non_tcp_udp */
  ndpi_str->callback_buffer_size_tcp_payload = 0;
  ndpi_str->callback_buffer_size_tcp_no_payload = 0;

  for(a = 0; a < ndpi_str->callback_buffer_size; a++) {
    if(!ndpi_proto_cb_tcp_payload(ndpi_str,a)) continue;
    if(!count_only) {
      NDPI_LOG_DBG2(ndpi_str, "callback_buffer_tcp_payload, adding buffer %u as entry %u\n", a,
		    ndpi_str->callback_buffer_size_tcp_payload);
      memcpy(&ndpi_str->callback_buffer_tcp_payload[ndpi_str->callback_buffer_size_tcp_payload],
	     &ndpi_str->callback_buffer[a], sizeof(struct call_function_struct));
    }
    ndpi_str->callback_buffer_size_tcp_payload++;
  }

  for(a = 0; a < ndpi_str->callback_buffer_size; a++) {
    if(!ndpi_proto_cb_tcp_nopayload(ndpi_str,a)) continue;
    if(!count_only) {
      NDPI_LOG_DBG2(ndpi_str,
                    "\tcallback_buffer_tcp_no_payload, additional adding buffer %u to no_payload process\n", a);
      memcpy(&ndpi_str->callback_buffer_tcp_no_payload[ndpi_str->callback_buffer_size_tcp_no_payload],
	     &ndpi_str->callback_buffer[a], sizeof(struct call_function_struct));
    }
    ndpi_str->callback_buffer_size_tcp_no_payload++;
  }

  ndpi_str->callback_buffer_size_udp = 0;

  for(a = 0; a < ndpi_str->callback_buffer_size; a++) {
    if(!ndpi_proto_cb_udp(ndpi_str,a)) continue;
    if(!count_only) {
      NDPI_LOG_DBG2(ndpi_str, "callback_buffer_size_udp: adding buffer : %u\n", a);

      memcpy(&ndpi_str->callback_buffer_udp[ndpi_str->callback_buffer_size_udp], &ndpi_str->callback_buffer[a],
	     sizeof(struct call_function_struct));
    }
    ndpi_str->callback_buffer_size_udp++;
  }

  ndpi_str->callback_buffer_size_non_tcp_udp = 0;

  for(a = 0; a < ndpi_str->callback_buffer_size; a++) {
    if(!ndpi_proto_cb_other(ndpi_str,a)) continue;
    if(!count_only) {
      NDPI_LOG_DBG2(ndpi_str, "callback_buffer_non_tcp_udp: adding buffer : %u\n", a);

      memcpy(&ndpi_str->callback_buffer_non_tcp_udp[ndpi_str->callback_buffer_size_non_tcp_udp],
	     &ndpi_str->callback_buffer[a], sizeof(struct call_function_struct));
    }
    ndpi_str->callback_buffer_size_non_tcp_udp++;
  }
}

/* ******************************************************************** */

/* handle extension headers in IPv6 packets
 * arguments:
 *  l3len: the packet length excluding the IPv6 header
 * 	l4ptr: pointer to the byte following the initial IPv6 header
 * 	l4len: the length of the IPv6 packet parsed from the IPv6 header
 * 	nxt_hdr: next header value from the IPv6 header
 * result:
 * 	l4ptr: pointer to the start of the actual layer 4 header
 * 	l4len: length of the actual layer 4 header
 * 	nxt_hdr: first byte of the layer 4 packet
 * returns 0 upon success and 1 upon failure
 */
int ndpi_handle_ipv6_extension_headers(u_int16_t l3len, const u_int8_t **l4ptr,
                                       u_int16_t *l4len, u_int8_t *nxt_hdr) {
  while(l3len > 1 && (*nxt_hdr == 0 || *nxt_hdr == 43 || *nxt_hdr == 44 || *nxt_hdr == 60 || *nxt_hdr == 135 || *nxt_hdr == 59)) {
    u_int16_t ehdr_len, frag_offset;

    // no next header
    if(*nxt_hdr == 59) {
      return(1);
    }

    // fragment extension header has fixed size of 8 bytes and the first byte is the next header type
    if(*nxt_hdr == 44) {
      if(*l4len < 8) {
	return(1);
      }

      if(l3len < 5) {
        return 1;
      }
      l3len -= 5;

      *nxt_hdr = (*l4ptr)[0];
      frag_offset = ntohs(*(u_int16_t *)((*l4ptr) + 2)) >> 3;
      // Handle ipv6 fragments as the ipv4 ones: keep the first fragment, drop the others
      if(frag_offset != 0)
	return(1);
      *l4len -= 8;
      (*l4ptr) += 8;
      continue;
    }

    // the other extension headers have one byte for the next header type
    // and one byte for the extension header length in 8 byte steps minus the first 8 bytes
    if(*l4len < 2) {
      return(1);
    }

    ehdr_len = (*l4ptr)[1];
    ehdr_len *= 8;
    ehdr_len += 8;

    if(ehdr_len > l3len) {
      return 1;
    }
    l3len -= ehdr_len;

    if(*l4len < ehdr_len) {
      return(1);
    }

    *nxt_hdr = (*l4ptr)[0];

    *l4len -= ehdr_len;
    (*l4ptr) += ehdr_len;
  }

  return(0);
}

/* ******************************************************************** */

/* Used by dns.c */
u_int8_t iph_is_valid_and_not_fragmented(const struct ndpi_iphdr *iph, const u_int16_t ipsize) {
  /*
    returned value:
    0: fragmented
    1: not fragmented
  */
  //#ifdef REQUIRE_FULL_PACKETS

  if(iph->protocol == IPPROTO_UDP) {
    if((ipsize < iph->ihl * 4)
       || (ipsize < ntohs(iph->tot_len))
       || (ntohs(iph->tot_len) < iph->ihl * 4)
       || (iph->frag_off & htons(0x1FFF)) != 0) {
      return(0);
    }
  }
  //#endif

  return(1);
}

/* ******************************************************************** */

/*
  extract the l4 payload, if available
  returned value:
  0: ok, extracted
  1: packet too small
  2,3: fragmented, ....
  else
  0: ok, extracted
  1: error or not available
*/
static u_int8_t ndpi_detection_get_l4_internal(struct ndpi_detection_module_struct *ndpi_str, const u_int8_t *l3,
                                               u_int16_t l3_len, const u_int8_t **l4_return, u_int16_t *l4_len_return,
                                               u_int8_t *l4_protocol_return, u_int32_t flags) {
  const struct ndpi_iphdr *iph = NULL;
  const struct ndpi_ipv6hdr *iph_v6 = NULL;
  u_int16_t l4len = 0;
  const u_int8_t *l4ptr = NULL;
  u_int8_t l4protocol = 0;

  if(l3 == NULL || l3_len < sizeof(struct ndpi_iphdr))
    return(1);

  iph = (const struct ndpi_iphdr *) l3;

  if((iph->version == 4 /* IPVERSION */) && (iph->ihl >= 5)) {
    NDPI_LOG_DBG2(ndpi_str, "IPv4 header\n");
  }
  else if(iph->version == 6 && l3_len >= sizeof(struct ndpi_ipv6hdr)) {
    NDPI_LOG_DBG2(ndpi_str, "ipv6 header\n");
    iph_v6 = (const struct ndpi_ipv6hdr *) l3;
    iph = NULL;
  } else {
    return(1);
  }

  if((flags & NDPI_DETECTION_ONLY_IPV6) && iph != NULL) {
    NDPI_LOG_DBG2(ndpi_str, "ipv4 header found but excluded by flag\n");
    return(1);
  } else if((flags & NDPI_DETECTION_ONLY_IPV4) && iph_v6 != NULL) {
    NDPI_LOG_DBG2(ndpi_str, "ipv6 header found but excluded by flag\n");
    return(1);
  }

  /* 0: fragmented; 1: not fragmented */
  if(iph != NULL && iph_is_valid_and_not_fragmented(iph, l3_len)) {
    u_int16_t len = ndpi_min(ntohs(iph->tot_len), l3_len);
    u_int16_t hlen = (iph->ihl * 4);

    l4ptr = (((const u_int8_t *) iph) + iph->ihl * 4);

    if(len == 0)
      len = l3_len;

    l4len = (len > hlen) ? (len - hlen) : 0;
    l4protocol = iph->protocol;
  }

  else if(iph_v6 != NULL && (l3_len - sizeof(struct ndpi_ipv6hdr)) >= ntohs(iph_v6->ip6_hdr.ip6_un1_plen)) {
    l4ptr = (((const u_int8_t *) iph_v6) + sizeof(struct ndpi_ipv6hdr));
    l4len = ntohs(iph_v6->ip6_hdr.ip6_un1_plen);
    l4protocol = iph_v6->ip6_hdr.ip6_un1_nxt;

    // we need to handle IPv6 extension headers if present
    if(ndpi_handle_ipv6_extension_headers(l3_len - sizeof(struct ndpi_ipv6hdr), &l4ptr, &l4len, &l4protocol) != 0) {
      return(1);
    }

  } else {
    return(1);
  }

  if(l4_return != NULL) {
    *l4_return = l4ptr;
  }

  if(l4_len_return != NULL) {
    *l4_len_return = l4len;
  }

  if(l4_protocol_return != NULL) {
    *l4_protocol_return = l4protocol;
  }

  return(0);
}

/* ****************************************************** */

void ndpi_free_flow_data(struct ndpi_flow_struct* flow) {
  if(flow) {
    if(flow->num_risk_infos) {
      u_int i;

      for(i=0; i<flow->num_risk_infos; i++)
	ndpi_free(flow->risk_infos[i].info);
    }

    if(flow->tcp.fingerprint)
      ndpi_free(flow->tcp.fingerprint);

    if(flow->tcp.fingerprint_raw)
      ndpi_free(flow->tcp.fingerprint_raw);

    if(flow->http.url)
      ndpi_free(flow->http.url);

    if(flow->http.content_type)
      ndpi_free(flow->http.content_type);

    if(flow->http.request_content_type)
      ndpi_free(flow->http.request_content_type);

    if(flow->http.referer)
      ndpi_free(flow->http.referer);

    if(flow->http.host)
      ndpi_free(flow->http.host);

    if(flow->http.user_agent)
      ndpi_free(flow->http.user_agent);

    if(flow->http.nat_ip)
      ndpi_free(flow->http.nat_ip);

    if(flow->http.detected_os)
      ndpi_free(flow->http.detected_os);

    if(flow->http.server)
      ndpi_free(flow->http.server);

    if(flow->http.filename)
      ndpi_free(flow->http.filename);

    if(flow->http.username)
      ndpi_free(flow->http.username);

    if(flow->http.password)
      ndpi_free(flow->http.password);

    if(flow->kerberos_buf.pktbuf)
      ndpi_free(flow->kerberos_buf.pktbuf);

    if(flow->monit)
      ndpi_free(flow->monit);

    if(flow_is_proto(flow, NDPI_PROTOCOL_QUIC) ||
       flow_is_proto(flow, NDPI_PROTOCOL_TLS) ||
       flow_is_proto(flow, NDPI_PROTOCOL_DTLS) ||
       flow_is_proto(flow, NDPI_PROTOCOL_MAIL_SMTPS) ||
       flow_is_proto(flow, NDPI_PROTOCOL_MAIL_POPS) ||
       flow_is_proto(flow, NDPI_PROTOCOL_MAIL_IMAPS) ||
       flow_is_proto(flow, NDPI_PROTOCOL_FTPS)) {
      if(flow->protos.tls_quic.server_names)
	ndpi_free(flow->protos.tls_quic.server_names);

      if(flow->protos.tls_quic.advertised_alpns)
	ndpi_free(flow->protos.tls_quic.advertised_alpns);

      if(flow->protos.tls_quic.negotiated_alpn)
	ndpi_free(flow->protos.tls_quic.negotiated_alpn);

      if(flow->protos.tls_quic.tls_supported_versions)
	ndpi_free(flow->protos.tls_quic.tls_supported_versions);

      if(flow->protos.tls_quic.issuerDN)
	ndpi_free(flow->protos.tls_quic.issuerDN);

      if(flow->protos.tls_quic.subjectDN)
	ndpi_free(flow->protos.tls_quic.subjectDN);

      if(flow->protos.tls_quic.ja4_client_raw)
	ndpi_free(flow->protos.tls_quic.ja4_client_raw);
    }

    if(flow_is_proto(flow, NDPI_PROTOCOL_SIP)) {
      if(flow->protos.sip.from)
        ndpi_free(flow->protos.sip.from);
      if(flow->protos.sip.to)
        ndpi_free(flow->protos.sip.to);
    }

    if (flow_is_proto(flow, NDPI_PROTOCOL_SSDP)) {
      if(flow->protos.ssdp.bootid)
      ndpi_free(flow->protos.ssdp.bootid);

      if(flow->protos.ssdp.usn)
        ndpi_free(flow->protos.ssdp.usn);

      if(flow->protos.ssdp.cache_controle)
        ndpi_free(flow->protos.ssdp.cache_controle);

      if(flow->protos.ssdp.location)
        ndpi_free(flow->protos.ssdp.location);

      if(flow->protos.ssdp.household_smart_speaker_audio)
        ndpi_free(flow->protos.ssdp.household_smart_speaker_audio);

      if(flow->protos.ssdp.rincon_household)
        ndpi_free(flow->protos.ssdp.rincon_household);

      if(flow->protos.ssdp.rincon_bootseq)
        ndpi_free(flow->protos.ssdp.rincon_bootseq);

      if(flow->protos.ssdp.rincon_wifimode)
        ndpi_free(flow->protos.ssdp.rincon_wifimode);

      if(flow->protos.ssdp.rincon_variant)
        ndpi_free(flow->protos.ssdp.rincon_variant);

      if(flow->protos.ssdp.sonos_securelocation)
        ndpi_free(flow->protos.ssdp.sonos_securelocation);

      if(flow->protos.ssdp.securelocation_upnp)
        ndpi_free(flow->protos.ssdp.securelocation_upnp);

      if(flow->protos.ssdp.location_smart_speaker_audio)
        ndpi_free(flow->protos.ssdp.location_smart_speaker_audio);

      if(flow->protos.ssdp.nt)
        ndpi_free(flow->protos.ssdp.nt);

      if(flow->protos.ssdp.nts)
        ndpi_free(flow->protos.ssdp.nts);

      if(flow->protos.ssdp.server)
        ndpi_free(flow->protos.ssdp.server);

      if(flow->protos.ssdp.method)
        ndpi_free(flow->protos.ssdp.method);

      if(flow->protos.ssdp.man)
        ndpi_free(flow->protos.ssdp.man);

      if(flow->protos.ssdp.mx)
        ndpi_free(flow->protos.ssdp.mx);

      if(flow->protos.ssdp.st)
        ndpi_free(flow->protos.ssdp.st);

      if(flow->protos.ssdp.user_agent)
        ndpi_free(flow->protos.ssdp.user_agent);
    }

    if(flow->tls_quic.message[0].buffer)
      ndpi_free(flow->tls_quic.message[0].buffer);
    if(flow->tls_quic.message[1].buffer)
      ndpi_free(flow->tls_quic.message[1].buffer);

    if(flow->l4_proto == IPPROTO_UDP) {
      if(flow->l4.udp.quic_reasm_buf)
        ndpi_free(flow->l4.udp.quic_reasm_buf);
      if(flow->l4.udp.quic_reasm_buf_bitmap)
        ndpi_free(flow->l4.udp.quic_reasm_buf_bitmap);
    }

    if(flow->flow_payload != NULL)
      ndpi_free(flow->flow_payload);

    if(flow->tls_quic.obfuscated_heur_state)
      ndpi_free(flow->tls_quic.obfuscated_heur_state);
  }
}

static inline uint32_t get_timestamp(uint64_t ts_l,uint32_t divisor) {
#if defined(__KERNEL__) && !defined(CONFIG_64BIT)
	do_div(ts_l,divisor);
	return (uint32_t)ts_l;
#else
	return (uint32_t)(ts_l/divisor);
#endif
}

int ndpi_set_protocol_detection_bitmask2(struct ndpi_detection_module_struct *ndpi_str,
					 const NDPI_PROTOCOL_BITMASK *dbm) {
  if(!ndpi_str)
    return -1;

  NDPI_BITMASK_SET(ndpi_str->detection_bitmask, *dbm);

  ndpi_init_protocol_defaults(ndpi_str);

//  ndpi_enabled_callbacks_init(ndpi_str, 0);

  if(ndpi_callback_init(ndpi_str)) {
    NDPI_LOG_ERR(ndpi_str, "[NDPI] Error allocating callbacks\n");
    return -1;
  }
  return 0;
}

/* ************************************************ */

static int ndpi_init_packet(struct ndpi_detection_module_struct *ndpi_str,
			    struct ndpi_flow_struct *flow,
			    const u_int64_t current_time_ms,
			    const unsigned char *packet_data,
			    unsigned short packetlen,
			    struct ndpi_flow_input_info *input_info) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  const struct ndpi_iphdr *decaps_iph = NULL;
  u_int16_t l3len;
  u_int16_t l4len, l4_packet_len;
  const u_int8_t *l4ptr;
  u_int8_t l4protocol;
  u_int8_t l4_result;

  /* need at least 20 bytes for ip header */
  if(packetlen < 20)
    return 1;

  packet->current_time_ms = current_time_ms;
  packet->current_time = get_timestamp(current_time_ms,ndpi_str->ticks_per_second);

  if(!ndpi_str->input_info)
	  ndpi_str->input_info = input_info;

  packet->iph = (const struct ndpi_iphdr *)packet_data;

  /* reset payload_packet_len, will be set if ipv4 tcp or udp */
  packet->payload = NULL;
  packet->payload_packet_len = 0;
  packet->l3_packet_len = packetlen;

  packet->tcp = NULL, packet->udp = NULL;
  packet->iphv6 = NULL;

  l3len = packet->l3_packet_len;

  ndpi_reset_packet_line_info(packet);
  packet->packet_lines_parsed_complete = 0;
  packet->http_check_content = 0;

  if(packet->iph != NULL)
    decaps_iph = packet->iph;

  if(decaps_iph && (decaps_iph->version == 4 /* IPVERSION */) && (decaps_iph->ihl >= 5)) {
    NDPI_LOG_DBG2(ndpi_str, "ipv4 header\n");
  } else if(decaps_iph && decaps_iph->version == 6 && l3len >= sizeof(struct ndpi_ipv6hdr) &&
	    (ndpi_str->ip_version_limit & NDPI_DETECTION_ONLY_IPV4) == 0) {
    NDPI_LOG_DBG2(ndpi_str, "ipv6 header\n");
    packet->iphv6 = (struct ndpi_ipv6hdr *)packet->iph;
    packet->iph = NULL;
  } else {
    packet->iph = NULL;
    return(1);
  }

  /* needed:
   *  - unfragmented packets
   *  - ip header <= packet len
   *  - ip total length >= packet len
   */

  l4ptr = NULL;
  l4len = 0;
  l4protocol = 0;

  l4_result =
    ndpi_detection_get_l4_internal(ndpi_str, (const u_int8_t *) decaps_iph, l3len, &l4ptr, &l4len, &l4protocol, 0);

  if(l4_result != 0) {
    return(1);
  }

  l4_packet_len = l4len;
  flow->l4_proto = l4protocol;

  /* TCP / UDP detection */
  if(l4protocol == IPPROTO_TCP) {
    u_int16_t tcp_header_len;

    if(l4_packet_len < sizeof(struct ndpi_tcphdr) /* min size of tcp */)
      return(1);

    /* tcp */
    packet->tcp = (struct ndpi_tcphdr *) l4ptr;
    tcp_header_len = packet->tcp->doff * 4;

    if(l4_packet_len >= tcp_header_len) {
      if(ndpi_str->cfg.tcp_fingerprint_enabled &&
         flow->tcp.fingerprint == NULL) {
	u_int8_t *t = (u_int8_t*)packet->tcp;
	u_int16_t flags = ntohs(*((u_int16_t*)&t[12])) & 0xFFF;
	u_int16_t syn_mask = TH_SYN | TH_ECE | TH_CWR;

	if((flags & syn_mask) && ((flags & TH_ACK) == 0)) {
	  char fingerprint[128], options_fp[128];
	  u_int8_t i, fp_idx = 0, options_fp_len = 0;

	  if(tcp_header_len >= sizeof(struct ndpi_tcphdr)) {
	    u_int8_t *options = (u_int8_t*)(&t[sizeof(struct ndpi_tcphdr)]);
	    u_int8_t options_len = tcp_header_len - sizeof(struct ndpi_tcphdr);
	    u_int16_t tcp_win = ntohs(packet->tcp->window);
	    u_int8_t ip_ttl;
	    u_int8_t sha_hash[NDPI_SHA256_BLOCK_SIZE];
	    u_int32_t tcp_mss = 0, tcp_wscale = 0;
	    int rc;

	    if(packet->iph)
	      ip_ttl = packet->iph->ttl;
	    else
	      ip_ttl = packet->iphv6->ip6_hdr.ip6_un1_hlim;

	    if(ip_ttl <= 32) ip_ttl = 32;
	    else if(ip_ttl <= 64)  ip_ttl = 64;
	    else if(ip_ttl <= 128) ip_ttl = 128;
	    else if(ip_ttl <= 192) ip_ttl = 192;
	    else ip_ttl = 255;

	    switch(ndpi_str->cfg.tcp_fingerprint_format) {
	    case NDPI_NATIVE_TCP_FINGERPRINT:
	      fp_idx = snprintf(fingerprint, sizeof(fingerprint), "%u_%u_%u_", flags, ip_ttl, tcp_win);
	      break;

	    case NDPI_MUONFP_TCP_FINGERPRINT:
	      fp_idx = snprintf(fingerprint, sizeof(fingerprint), "%u:", tcp_win);
	      break;
	    }

	    if(options_len == 0) {
	      const char *msg;

	      /*
		Massive Internet scanner detected. Examples:
		- https://zmap.io
		- https://github.com/robertdavidgraham/masscan
	      */

	      if(tcp_win == 1024)
		msg = "Massive scanner detected (probably masscan)";
	      else if(tcp_win == 65535)
		msg = "Massive scanner detected (probably zmap)";
	      else
		msg = "Massive scanner detected";

	      ndpi_set_risk(ndpi_str, flow, NDPI_MALICIOUS_FINGERPRINT, (char*)msg);
	    } else {
	      for(i=0; i<options_len; /* don't increase here */) {
		u_int8_t kind = options[i];

#ifdef DEBUG_TCP_OPTIONS
		printf("Option kind: %u\n", kind);
#endif

		if(ndpi_str->cfg.tcp_fingerprint_format == NDPI_NATIVE_TCP_FINGERPRINT) {
		  rc = snprintf(&options_fp[options_fp_len], sizeof(options_fp)-options_fp_len, "%02x", kind);

		  if((rc < 0) || ((int)(options_fp_len + rc) == sizeof(options_fp)))
		    break;

		  options_fp_len += rc;
		} else if(ndpi_str->cfg.tcp_fingerprint_format == NDPI_MUONFP_TCP_FINGERPRINT) {
		  if(fp_idx >= sizeof(fingerprint))
		    break;

		  rc = snprintf(&fingerprint[fp_idx], sizeof(fingerprint)-fp_idx, "%s%u", (i > 0) ? "-" : "", kind);

		  if((rc < 0) || ((int)(fp_idx + rc) == sizeof(fingerprint)))
		    break;

		  fp_idx += rc;
		}

		if(kind == 0) /* EOL */ {
		  i++;
		  continue;
		} else if(kind == 1) /* NOP */
		  i++;
		else if((i+1) < options_len) {
		  u_int8_t len = options[i+1];

#ifdef DEBUG_TCP_OPTIONS
		  printf("\tOption len: %u\n", len);
#endif

		  if(len == 0)
		    continue;
		  else if(kind == 8) {
		    switch(ndpi_str->cfg.tcp_fingerprint_format) {
		    case NDPI_NATIVE_TCP_FINGERPRINT:
		      /* Timestamp: ignore it */
		      break;

		    case NDPI_MUONFP_TCP_FINGERPRINT:
		      /* Nothing to do */
		      break;
		    }
		  } else if(len > 2) {
		    int j = i+2;
		    u_int8_t opt_len = len - 2;

		    if((kind == 2 /* Maximum segment size */) || (kind == 3 /* TCP window scale */)) {
		      u_int32_t val = 0;

		      if(opt_len == 1)
			val = options[j];
		      else if(opt_len == 2)
			val = (options[j] << 8) + options[j+1];
		      else if(opt_len == 3)
			val = (options[j] << 16) + (options[j+1] << 8) + options[j+2];
		      else if(opt_len == 4)
			val = (options[j] << 24) + (options[j+1] << 16) + (options[j+2] << 8) + options[j+3];

		      if(kind == 2)
			tcp_mss = val;
		      else if(kind == 3)
			tcp_wscale = val;
		    }

		    if(ndpi_str->cfg.tcp_fingerprint_format == NDPI_NATIVE_TCP_FINGERPRINT) {
		      while((opt_len > 0) && (j < options_len)) {
			rc = snprintf(&options_fp[options_fp_len], sizeof(options_fp)-options_fp_len, "%02x", options[j]);
			if((rc < 0) || ((int)(options_fp_len + rc) == sizeof(options_fp))) break;

			options_fp_len += rc;
			j++, opt_len--;
		      }
		    }
		  }

		  i += len;
		} else
		  break;
	      } /* for */

	      if((options_len == 4) && (tcp_mss > 0)) {
		/*
		  Not inherently malicious, but unusual for modern general-purpose OSes.
		  More suspicious if coming from a device that should support full TCP options (e.g., a Windows/Linux server).
		  Less suspicious if from an embedded device or legacy system.

		  For this reason we ignore packets originating from private IP
		  that might be originated by outdated systems.
		*/
		if(packet->iphv6 /* Modern IP stack */
		   || (packet->iph
		       && ndpi_is_public_ipv4(ntohl(packet->iph->saddr))))
		  ndpi_set_risk(ndpi_str, flow, NDPI_MALICIOUS_FINGERPRINT,
				"Unusual TCP fingerprint (scanner detected?)");
	      }
	    }

#ifdef DEBUG_TCP_OPTIONS
	    printf("Raw Options Fingerprint: %s\n", options_fp);
#endif

	    switch(ndpi_str->cfg.tcp_fingerprint_format) {
	    case NDPI_NATIVE_TCP_FINGERPRINT:
	      ndpi_sha256((const u_char*)options_fp, options_fp_len, sha_hash);

	      snprintf(&fingerprint[fp_idx], sizeof(fingerprint)-fp_idx, "%02x%02x%02x%02x%02x%02x",
		       sha_hash[0], sha_hash[1], sha_hash[2],
		       sha_hash[3], sha_hash[4], sha_hash[5]);
	      break;

	    case NDPI_MUONFP_TCP_FINGERPRINT:
	      if(fp_idx < sizeof(fingerprint)) {
		if(tcp_mss > 0)
		  rc = snprintf(&fingerprint[fp_idx], sizeof(fingerprint)-fp_idx, ":%u", tcp_mss);
		else
		  rc = snprintf(&fingerprint[fp_idx], sizeof(fingerprint)-fp_idx, ":");

		if(rc > 0) {
		  fp_idx += rc;

		  if(fp_idx < sizeof(fingerprint)) {
		    if(tcp_wscale > 0)
		      rc = snprintf(&fingerprint[fp_idx], sizeof(fingerprint)-fp_idx, ":%u", tcp_wscale);
		    else
		      rc = snprintf(&fingerprint[fp_idx], sizeof(fingerprint)-fp_idx, ":");

		    if(rc > 0)
		      fp_idx += rc;
		  }
		}
	      }
	      break;
	    }

	    flow->tcp.fingerprint = ndpi_strdup(fingerprint);

	    if(ndpi_str->cfg.tcp_fingerprint_raw_enabled)
	      flow->tcp.fingerprint_raw = ndpi_strdup(options_fp);

	    flow->tcp.os_hint = ndpi_get_os_from_tcp_fingerprint(ndpi_str, flow->tcp.fingerprint);
	  }
	}
      }

      packet->payload_packet_len = l4_packet_len - tcp_header_len;
      packet->payload = ((u_int8_t *) packet->tcp) + tcp_header_len;
    } else {
      /* tcp header not complete */
      return(1);
    }
  } else if(l4protocol == IPPROTO_UDP) {
    if(l4_packet_len < 8 /* size of udp */)
      return(1);
    packet->udp = (struct ndpi_udphdr *) l4ptr;
    packet->payload_packet_len = l4_packet_len - 8;
    packet->payload = ((u_int8_t *) packet->udp) + 8;
  } else if((l4protocol == IPPROTO_ICMP) || (l4protocol == IPPROTO_ICMPV6)) {
    if((l4protocol == IPPROTO_ICMP && l4_packet_len < sizeof(struct ndpi_icmphdr)) ||
       (l4protocol == IPPROTO_ICMPV6 && l4_packet_len < sizeof(struct ndpi_icmp6hdr)))
      return(1);
    packet->payload = ((u_int8_t *) l4ptr);
    packet->payload_packet_len = l4_packet_len;
  } else {
    packet->payload = ((u_int8_t *) l4ptr);
    packet->payload_packet_len = l4_packet_len;
  }

  return(0);
}

/* ************************************************ */

static u_int8_t ndpi_is_multi_or_broadcast(struct ndpi_flow_struct *flow) {

  if(!flow->is_ipv6) {
    /* IPv4 */
    u_int32_t daddr = ntohl(flow->s_address.v4);

    if(((daddr & 0xF0000000) == 0xE0000000 /* multicast 224.0.0.0/4 */)
       || ((daddr & 0x000000FF) == 0x000000FF /* last byte is 0xFF, not super correct, but a good approximation */)
       || ((daddr & 0x000000FF) == 0x00000000 /* last byte is 0x00, not super correct, but a good approximation */)
       || (daddr == 0xFFFFFFFF))
      return(1);
  } else {
    /* IPv6 */

    if((ntohl((*(u_int32_t *)&flow->s_address.v6)) & 0xFF000000) == 0xFF000000)
      return(1);
  }

  return(0);
}

/* ************************************************ */

static int fully_enc_heuristic(struct ndpi_detection_module_struct *ndpi_str,
                               struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  unsigned int i, len, cnt, cnt_consecutives = 0;

  if(flow->l4_proto == IPPROTO_TCP &&
     ndpi_seen_flow_beginning(flow)) {
    /* See original paper, Algorithm 1, for the reference numbers */
#ifndef __KERNEL__
  {
    struct ndpi_popcount popcount;
    int ratio;
    /* Ex1 */
    ndpi_popcount_init(&popcount);
    ndpi_popcount_count(&popcount, packet->payload, packet->payload_packet_len);

    ratio = popcount.pop_count * 100 / (popcount.tot_bytes_count ? popcount.tot_bytes_count:1);
    if( ratio < 340 || ratio >= 460)
	    return 0;
  }
#endif
    /* Ex2 */
    len = ndpi_min(6, packet->payload_packet_len);
    cnt = 0;
    for(i = 0; i < len; i++) {
      if(ndpi_isprint(packet->payload[i]))
        cnt += 1;
    }
    if(cnt == len) {
      return 0;
    }

    /* Ex3 */
    cnt = 0;
    for(i = 0; i < packet->payload_packet_len; i++) {
      if(ndpi_isprint(packet->payload[i])) {
        cnt += 1;
        cnt_consecutives += 1;
        if(cnt_consecutives >= 20) { /* Ex4 */
          return 0;
        }
      } else {
        cnt_consecutives = 0;
      }
    }
    if(cnt*100 / packet->payload_packet_len > 50) {
      return 0;
    }

    return 1;
  }
  return 0;
}

/* ************************************************ */
int current_pkt_from_client_to_server(struct ndpi_detection_module_struct *ndpi_str,
				      const struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  return packet->packet_direction == flow->client_packet_direction;
}

/* ******************************************************************** */

int current_pkt_from_server_to_client(struct ndpi_detection_module_struct *ndpi_str,
				      const struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  return packet->packet_direction != flow->client_packet_direction;
}

/* ******************************************************************** */

static int tcp_ack_padding(struct ndpi_packet_struct *packet) {
  const struct ndpi_tcphdr *tcph = packet->tcp;

  if(tcph && tcph->ack && !tcph->psh &&
     packet->payload_packet_len < 8 &&
     packet->payload_packet_len > 1 /* To avoid TCP keep-alives */) {
    int i;

    for(i = 0; i < packet->payload_packet_len; i++)
      if(packet->payload[i] != 0)
        return 0;
    return 1;
  }

  return 0;
}

/* ******************************************************************** */

static void ndpi_connection_tracking(struct ndpi_detection_module_struct *ndpi_str,
				     struct ndpi_flow_struct *flow) {
    /* const for gcc code optimization and cleaner code */
    struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
    const struct ndpi_iphdr *iph = packet->iph;
    const struct ndpi_ipv6hdr *iphv6 = packet->iphv6;
    const struct ndpi_tcphdr *tcph = packet->tcp;
    const struct ndpi_udphdr *udph = packet->udp;

#ifndef __KERNEL__
    if(ndpi_str->max_payload_track_len > 0 && packet->payload_packet_len > 0) {
      /* printf("LEN: %u [%s]\n", packet->payload_packet_len, packet->payload); */

      if(flow->flow_payload == NULL)
	flow->flow_payload = (char*)ndpi_malloc(ndpi_str->max_payload_track_len + 1);

      if(flow->flow_payload != NULL)  {
	u_int i;

	for(i=0; (i<packet->payload_packet_len)
	      && (flow->flow_payload_len < ndpi_str->max_payload_track_len); i++) {
	  flow->flow_payload[flow->flow_payload_len++] =
	    (ndpi_isprint(packet->payload[i]) || ndpi_isspace(packet->payload[i])) ? packet->payload[i] : '.';
	}
      }
    }
#endif

  if(ndpi_str->max_payload_track_len > 0 && packet->payload_packet_len > 0) {
    /* printf("LEN: %u [%s]\n", packet->payload_packet_len, packet->payload); */

    if(flow->flow_payload == NULL)
      flow->flow_payload = (char*)ndpi_malloc(ndpi_str->max_payload_track_len + 1);

    if(flow->flow_payload != NULL)  {
      u_int i;

      for(i=0; (i<packet->payload_packet_len)
	    && (flow->flow_payload_len < ndpi_str->max_payload_track_len); i++) {
	flow->flow_payload[flow->flow_payload_len++] =
	  (ndpi_isprint(packet->payload[i])
	   || ndpi_isspace(packet->payload[i])) ? packet->payload[i] : '.';
      }
    }
  }

  packet->tcp_retransmission = 0, packet->packet_direction = 0;

  if(!ndpi_str->cfg.direction_detect_enabled) {
    packet->packet_direction = flow->packet_direction;
  } else {
    if(iph != NULL && ntohl(iph->saddr) < ntohl(iph->daddr))
      packet->packet_direction = 1;

    if((iphv6 != NULL)
       && NDPI_COMPARE_IPV6_ADDRESS_STRUCTS(&iphv6->ip6_src, &iphv6->ip6_dst) != 0)
      packet->packet_direction = 1;
  }

  flow->is_ipv6 = (packet->iphv6 != NULL);

  flow->last_packet_time_ms = packet->current_time_ms;
  flow->last_packet_time = packet->current_time;

  packet->packet_lines_parsed_complete = 0;

  if(tcph != NULL) {
    u_int8_t flags = ((u_int8_t*)tcph)[13];

    if(flags == 0)
      ndpi_set_risk(ndpi_str, flow, NDPI_TCP_ISSUES, "TCP NULL scan");
    else if(flags == (TH_FIN | TH_PUSH | TH_URG))
      ndpi_set_risk(ndpi_str, flow, NDPI_TCP_ISSUES, "TCP XMAS scan");

    if(ndpi_str->cfg.direction_detect_enabled &&
       (tcph->source != tcph->dest))
      packet->packet_direction = (ntohs(tcph->source) < ntohs(tcph->dest)) ? 1 : 0;

    if(packet->packet_direction == 0 /* cli -> srv */) {
      if(flags == TH_FIN)
	ndpi_set_risk(ndpi_str, flow, NDPI_TCP_ISSUES, "TCP FIN scan");

      flow->l4.tcp.cli2srv_tcp_flags |= flags;
    } else
      flow->l4.tcp.srv2cli_tcp_flags |= flags;

    if((ndpi_str->input_info == NULL)
       || ndpi_str->input_info->seen_flow_beginning == NDPI_FLOW_BEGINNING_UNKNOWN) {
      if(tcph->syn != 0 && tcph->ack == 0 && flow->l4.tcp.seen_syn == 0
	 && flow->l4.tcp.seen_syn_ack == 0 &&
	 flow->l4.tcp.seen_ack == 0) {
	flow->l4.tcp.seen_syn = 1;
      } else {
	if(tcph->syn != 0 && tcph->ack != 0 && flow->l4.tcp.seen_syn == 1
	   && flow->l4.tcp.seen_syn_ack == 0 &&
	   flow->l4.tcp.seen_ack == 0) {
	  flow->l4.tcp.seen_syn_ack = 1;
	} else {
	  if(tcph->syn == 0 && tcph->ack == 1 && flow->l4.tcp.seen_syn == 1 && flow->l4.tcp.seen_syn_ack == 1 &&
	     flow->l4.tcp.seen_ack == 0) {
	    flow->l4.tcp.seen_ack = 1;
	  }
	}
      }
    }

    if(ndpi_str->cfg.tcp_ack_paylod_heuristic && tcp_ack_padding(packet)) {
      NDPI_LOG_DBG2(ndpi_str, "TCP ACK with zero padding. Ignoring\n");
      packet->tcp_retransmission = 1;
    } else if(flow->l4.tcp.next_tcp_seq_nr[0] == 0 || flow->l4.tcp.next_tcp_seq_nr[1] == 0 ||
	      (tcph->syn && flow->packet_counter == 0)) {
      /* initialize tcp sequence counters */
      /* the ack flag needs to be set to get valid sequence numbers from the other
       * direction. Usually it will catch the second packet syn+ack but it works
       * also for asymmetric traffic where it will use the first data packet
       *
       * if the syn flag is set add one to the sequence number,
       * otherwise use the payload length.
       *
       * If we receive multiple syn-ack (before any real data), keep the last one
       */
      if(tcph->ack != 0) {
	flow->l4.tcp.next_tcp_seq_nr[packet->packet_direction] =
	  ntohl(tcph->seq) + (tcph->syn ? 1 : packet->payload_packet_len);

	/*
	  Check to avoid discrepancies in case we analyze a flow that does not start with SYN...
	  but that is already started when nDPI being to process it. See also (***) below
	*/
	if(flow->num_processed_pkts > 1)
	  flow->l4.tcp.next_tcp_seq_nr[1 - packet->packet_direction] = ntohl(tcph->ack_seq);
      }
    } else if(packet->payload_packet_len > 0) {
      /* check tcp sequence counters */
      if(((u_int32_t)(ntohl(tcph->seq) - flow->l4.tcp.next_tcp_seq_nr[packet->packet_direction])) >
	 ndpi_str->tcp_max_retransmission_window_size) {
	if(flow->l4.tcp.last_tcp_pkt_payload_len > 0)
	  packet->tcp_retransmission = 1;

	/* CHECK IF PARTIAL RETRY IS HAPPENING */
	if((flow->l4.tcp.next_tcp_seq_nr[packet->packet_direction] - ntohl(tcph->seq) <
	    packet->payload_packet_len)) {
	  if(flow->num_processed_pkts > 1) /* See also (***) above */
	    flow->l4.tcp.next_tcp_seq_nr[packet->packet_direction] = ntohl(tcph->seq) + packet->payload_packet_len;
	}
      }
      else {
	flow->l4.tcp.next_tcp_seq_nr[packet->packet_direction] = ntohl(tcph->seq) + packet->payload_packet_len;
      }
    }

    if(tcph->rst) {
      flow->l4.tcp.next_tcp_seq_nr[0] = 0;
      flow->l4.tcp.next_tcp_seq_nr[1] = 0;
    }

    flow->l4.tcp.last_tcp_pkt_payload_len = packet->payload_packet_len;
  } else if(udph != NULL) {
    if(ndpi_str->cfg.direction_detect_enabled &&
       (udph->source != udph->dest))
      packet->packet_direction = (htons(udph->source) < htons(udph->dest)) ? 1 : 0;
  }

  if(flow->init_finished == 0) {
    u_int16_t s_port = 0, d_port = 0; /* Source/Dest ports */

    flow->init_finished = 1;

    if(tcph != NULL) {
      if(ndpi_str->input_info &&
	 ndpi_str->input_info->seen_flow_beginning == NDPI_FLOW_BEGINNING_SEEN) {
	flow->l4.tcp.seen_syn = 1;
	flow->l4.tcp.seen_syn_ack = 1;
	flow->l4.tcp.seen_ack = 1;
      }

      s_port = tcph->source, d_port = tcph->dest;
    } else if(udph != NULL) {
      s_port = udph->source;
      d_port = udph->dest;
    }

    /* Client/Server direction */

    if(ndpi_str->input_info &&
       ndpi_str->input_info->in_pkt_dir != NDPI_IN_PKT_DIR_UNKNOWN) {
      if(ndpi_str->input_info->in_pkt_dir == NDPI_IN_PKT_DIR_C_TO_S)
	flow->client_packet_direction = packet->packet_direction;
      else
	flow->client_packet_direction = !packet->packet_direction;
    } else {
      if(tcph && tcph->syn) {
	if(tcph->ack == 0) {
	  flow->client_packet_direction = packet->packet_direction;
	} else {
	  flow->client_packet_direction = !packet->packet_direction;
	}
      } else if(ntohs(s_port) > 1024 && ntohs(d_port) < 1024) {
	flow->client_packet_direction = packet->packet_direction;
      } else if(ntohs(s_port) < 1024 && ntohs(d_port) > 1024) {
	flow->client_packet_direction = !packet->packet_direction;
      } else {
	flow->client_packet_direction = packet->packet_direction;
      }
    }

    if(current_pkt_from_client_to_server(ndpi_str, flow)) {
      if(flow->is_ipv6 == 0) {
	flow->c_address.v4 = packet->iph->saddr;
	flow->s_address.v4 = packet->iph->daddr;
      } else {
	memcpy(flow->c_address.v6, &packet->iphv6->ip6_src, 16);
	memcpy(flow->s_address.v6, &packet->iphv6->ip6_dst, 16);
      }

      flow->c_port = s_port;
      flow->s_port = d_port;
    } else {
      if(flow->is_ipv6 == 0) {
	flow->c_address.v4 = packet->iph->daddr;
	flow->s_address.v4 = packet->iph->saddr;
      } else {
	memcpy(flow->c_address.v6, &packet->iphv6->ip6_dst, 16);
	memcpy(flow->s_address.v6, &packet->iphv6->ip6_src, 16);
      }

      flow->c_port = d_port;
      flow->s_port = s_port;
    }
  }

  if(flow->packet_counter < MAX_PACKET_COUNTER && packet->payload_packet_len) {
    flow->packet_counter++;
  }

  if(flow->all_packets_counter < MAX_PACKET_COUNTER)
    flow->all_packets_counter++;

  if((flow->packet_direction_counter[packet->packet_direction] < MAX_PACKET_COUNTER)
     && packet->payload_packet_len) {
    flow->packet_direction_counter[packet->packet_direction]++;
  }

  if(flow->packet_direction_complete_counter[packet->packet_direction] < MAX_PACKET_COUNTER) {
    flow->packet_direction_complete_counter[packet->packet_direction]++;
  }

  if(ndpi_str->input_info &&
     ndpi_str->input_info->in_pkt_dir == NDPI_IN_PKT_DIR_UNKNOWN) {
    if(current_pkt_from_client_to_server(ndpi_str, flow))
      ndpi_str->input_info->in_pkt_dir = NDPI_IN_PKT_DIR_C_TO_S;
    else
      ndpi_str->input_info->in_pkt_dir = NDPI_IN_PKT_DIR_S_TO_C;
  }
}

/* ************************************************ */

static u_int32_t check_ndpi_subprotocols(struct ndpi_detection_module_struct * const ndpi_str,
                                         struct ndpi_flow_struct * const flow,
                                         NDPI_SELECTION_BITMASK_PROTOCOL_SIZE const ndpi_selection_packet,
                                         u_int16_t detected_protocol)
{
  u_int32_t num_calls = 0, a;

  if(detected_protocol == NDPI_PROTOCOL_UNKNOWN)
    return num_calls;

  for (a = 0; a < ndpi_str->proto_defaults[detected_protocol].subprotocol_count; a++) {
    u_int16_t subproto_id = ndpi_str->proto_defaults[detected_protocol].subprotocols[a];

    if(subproto_id == (uint16_t)NDPI_PROTOCOL_MATCHED_BY_CONTENT ||
       subproto_id == flow->detected_protocol_stack[0] ||
       subproto_id == flow->detected_protocol_stack[1]) {
      continue;
    }

    u_int16_t subproto_index = ndpi_str->proto_defaults[subproto_id].dissector_idx;

    if((ndpi_str->callback_buffer[subproto_index].ndpi_selection_bitmask & ndpi_selection_packet) ==
       ndpi_str->callback_buffer[subproto_index].ndpi_selection_bitmask &&
       !NDPI_DISSECTOR_BITMASK_IS_SET(flow->excluded_dissectors_bitmask, subproto_index)) {
      ndpi_str->current_dissector_idx = subproto_index;
      ndpi_str->callback_buffer[subproto_index].func(ndpi_str, flow);
      num_calls++;
    }
  }

  return num_calls;
}

/* ************************************************ */

static u_int32_t check_ndpi_detection_func(struct ndpi_detection_module_struct * const ndpi_str,
					   struct ndpi_flow_struct * const flow,
					   NDPI_SELECTION_BITMASK_PROTOCOL_SIZE const ndpi_selection_packet,
					   struct call_function_struct const * const callback_buffer,
					   uint32_t callback_buffer_size) {
  void *func = NULL;
  u_int32_t num_calls = 0;
  /* First callback is associated to classification by-port,
     if we don't already have a partial classification */
  u_int16_t fast_callback_protocol_id = flow->fast_callback_protocol_id ? flow->fast_callback_protocol_id : flow->guessed_protocol_id;
  u_int16_t dissector_idx = ndpi_str->proto_defaults[fast_callback_protocol_id].dissector_idx;

  if(fast_callback_protocol_id != NDPI_PROTOCOL_UNKNOWN &&
     ndpi_str->callback_buffer[dissector_idx].func &&
     !NDPI_DISSECTOR_BITMASK_IS_SET(flow->excluded_dissectors_bitmask, dissector_idx) &&
     (ndpi_str->callback_buffer[dissector_idx].ndpi_selection_bitmask & ndpi_selection_packet) ==
     ndpi_str->callback_buffer[dissector_idx].ndpi_selection_bitmask) {

    ndpi_str->current_dissector_idx = dissector_idx;
    ndpi_str->callback_buffer[dissector_idx].func(ndpi_str, flow);
    func = ndpi_str->callback_buffer[dissector_idx].func;
    num_calls++;
  }

  if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN)
    {
      u_int32_t a;
      /* TODO: optimize as today we're doing a linear scan */

      for (a = 0; a < callback_buffer_size; a++) {
        dissector_idx = callback_buffer[a].dissector_idx;

        if((func != callback_buffer[a].func) &&
	   (callback_buffer[a].ndpi_selection_bitmask & ndpi_selection_packet) ==
	   callback_buffer[a].ndpi_selection_bitmask &&
	   !NDPI_DISSECTOR_BITMASK_IS_SET(flow->excluded_dissectors_bitmask, dissector_idx))
	  {
            ndpi_str->current_dissector_idx = dissector_idx;
	    callback_buffer[a].func(ndpi_str, flow);
	    num_calls++;

	    if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN)
	      {
		break; /* Stop after the first detected protocol. */
	      }
	  }
      }
    }

  num_calls += check_ndpi_subprotocols(ndpi_str, flow, ndpi_selection_packet,
                                       flow->detected_protocol_stack[0]);
  num_calls += check_ndpi_subprotocols(ndpi_str, flow, ndpi_selection_packet,
                                       flow->detected_protocol_stack[1]);

  return num_calls;
}

/* ************************************************ */

NDPI_STATIC u_int32_t check_ndpi_other_flow_func(struct ndpi_detection_module_struct *ndpi_str,
				     struct ndpi_flow_struct *flow,
				     NDPI_SELECTION_BITMASK_PROTOCOL_SIZE *ndpi_selection_packet)
{
  return check_ndpi_detection_func(ndpi_str, flow, *ndpi_selection_packet,
				   ndpi_str->callback_buffer_non_tcp_udp,
				   ndpi_str->callback_buffer_size_non_tcp_udp);
}

/* ************************************************ */

static u_int32_t check_ndpi_udp_flow_func(struct ndpi_detection_module_struct *ndpi_str,
					  struct ndpi_flow_struct *flow,
					  NDPI_SELECTION_BITMASK_PROTOCOL_SIZE *ndpi_selection_packet) {
  return check_ndpi_detection_func(ndpi_str, flow, *ndpi_selection_packet,
				   ndpi_str->callback_buffer_udp,
				   ndpi_str->callback_buffer_size_udp);
}

/* ************************************************ */

static u_int32_t check_ndpi_tcp_flow_func(struct ndpi_detection_module_struct *ndpi_str,
					  struct ndpi_flow_struct *flow,
					  NDPI_SELECTION_BITMASK_PROTOCOL_SIZE *ndpi_selection_packet)
{
  if (ndpi_get_packet_struct(ndpi_str)->payload_packet_len != 0) {
    return check_ndpi_detection_func(ndpi_str, flow, *ndpi_selection_packet,
				     ndpi_str->callback_buffer_tcp_payload,
				     ndpi_str->callback_buffer_size_tcp_payload);
  } else {
    /* no payload */
    return check_ndpi_detection_func(ndpi_str, flow, *ndpi_selection_packet,
				     ndpi_str->callback_buffer_tcp_no_payload,
				     ndpi_str->callback_buffer_size_tcp_no_payload);
  }
}

/* ********************************************************************************* */

static u_int32_t ndpi_check_flow_func(struct ndpi_detection_module_struct *ndpi_str,
			              struct ndpi_flow_struct *flow,
			              NDPI_SELECTION_BITMASK_PROTOCOL_SIZE *ndpi_selection_packet) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  if(!flow)
    return(0);
  else if(packet->tcp != NULL)
    return(check_ndpi_tcp_flow_func(ndpi_str, flow, ndpi_selection_packet));
  else if(packet->udp != NULL)
    return(check_ndpi_udp_flow_func(ndpi_str, flow, ndpi_selection_packet));
  else
    return(check_ndpi_other_flow_func(ndpi_str, flow, ndpi_selection_packet));
}

/* ********************************************************************************* */

u_int16_t ndpi_guess_host_protocol_id(struct ndpi_detection_module_struct *ndpi_str,
				      struct ndpi_flow_struct *flow) {
#ifndef __KERNEL__
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  u_int16_t ret = NDPI_PROTOCOL_UNKNOWN;
  int use_client = ndpi_str->cfg.use_client_ip_in_guess;

  if(packet->iph) {
    struct in_addr addr;

    /* guess host protocol; server first */
    addr.s_addr = flow->s_address.v4;
    ret = ndpi_network_port_ptree_match(ndpi_str, &addr, flow->s_port);

    if(ret == NDPI_PROTOCOL_UNKNOWN && use_client) {
      addr.s_addr = flow->c_address.v4;
      ret = ndpi_network_port_ptree_match(ndpi_str, &addr, flow->c_port);
    }
  } else {
    struct in6_addr addr;

    addr = *(struct in6_addr *)&flow->s_address.v6;
    ret = ndpi_network_port_ptree6_match(ndpi_str, &addr, flow->s_port);

    if(ret == NDPI_PROTOCOL_UNKNOWN && use_client) {
      addr = *(struct in6_addr *)&flow->c_address.v6;
      ret = ndpi_network_port_ptree6_match(ndpi_str, &addr, flow->c_port);
    }
  }

  return(ret);
#else
  return NDPI_PROTOCOL_UNKNOWN;
#endif
}

/* ********************************************************************************* */

static u_int64_t make_msteams_key(struct ndpi_flow_struct *flow, u_int8_t use_client) {
  u_int64_t key;

  if(use_client) {
    if(flow->is_ipv6)
      key = ndpi_quick_hash64((const char *)flow->c_address.v6, 16);
    else
      key = ntohl(flow->c_address.v4);
  } else {
    if(flow->is_ipv6)
      key = ndpi_quick_hash64((const char *)flow->s_address.v6, 16);
    else
      key = ntohl(flow->s_address.v4);
  }

  return key;
}

/* ********************************************************************************* */

static void ndpi_reconcile_msteams_udp(struct ndpi_detection_module_struct *ndpi_str,
				       struct ndpi_flow_struct *flow,
				       u_int16_t master) {
  /* This function can NOT access &ndpi_str->packet since it is called also from ndpi_detection_giveup(), via ndpi_reconcile_protocols() */

  if(flow->l4_proto == IPPROTO_UDP) {
    u_int16_t sport = ntohs(flow->c_port);
    u_int16_t dport = ntohs(flow->s_port);
    u_int8_t  s_match = ((sport >= 3478) && (sport <= 3481)) ? 1 : 0;
    u_int8_t  d_match = ((dport >= 3478) && (dport <= 3481)) ? 1 : 0;

    if(s_match || d_match) {
      ndpi_int_change_protocol(flow,
			       NDPI_PROTOCOL_MSTEAMS_CALL, master,
			       /* Keep the same confidence */
			       flow->confidence);


      if(ndpi_str->msteams_cache)
	ndpi_lru_add_to_cache(ndpi_str->msteams_cache,
			      make_msteams_key(flow, s_match ? 0 /* server */ : 1 /* client */),
			      0 /* dummy */,
			      ndpi_get_current_time(flow));

    }
  }
}

/* ********************************************************************************* */

static int ndpi_reconcile_msteams_call_udp_port(struct ndpi_flow_struct *flow,
						u_int16_t sport, u_int16_t dport) {

  /*
    https://extremeportal.force.com/ExtrArticleDetail?an=000101782

    Audio:   UDP 50000-50019; 3478; 3479
    Video:   UDP 50020-50039; 3480
    Sharing: UDP 50040-50059; 3481
  */

  if((dport == 3478) || (dport == 3479) || ((sport >= 50000) && (sport <= 50019)))
    flow->flow_multimedia_types |= ndpi_multimedia_audio_flow;
  else if((dport == 3480) || ((sport >= 50020) && (sport <= 50039)))
    flow->flow_multimedia_types |= ndpi_multimedia_video_flow;
  else if((dport == 3481) || ((sport >= 50040) && (sport <= 50059)))
    flow->flow_multimedia_types |= ndpi_multimedia_screen_sharing_flow;
  else {
    flow->flow_multimedia_types = ndpi_multimedia_unknown_flow;
    return(0);
  }

  return(1);
}

/* ********************************************************************************* */

static void ndpi_reconcile_msteams_call_udp(struct ndpi_flow_struct *flow) {
  if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_MSTEAMS_CALL) {
    if(flow->l4_proto == IPPROTO_UDP) {
      u_int16_t sport = ntohs(flow->c_port);
      u_int16_t dport = ntohs(flow->s_port);

      if(ndpi_reconcile_msteams_call_udp_port(flow, sport, dport) == 0)
	ndpi_reconcile_msteams_call_udp_port(flow, dport, sport);
    }
  }
}

/* ********************************************************************************* */

static void ndpi_reconcile_protocols(struct ndpi_detection_module_struct *ndpi_str,
				     struct ndpi_flow_struct *flow,
				     ndpi_protocol *ret) {
  u_int i, skip_risk = 0;

  /* This function can NOT access &ndpi_str->packet since it is called also from ndpi_detection_giveup() */

  // printf("====>> %u.%u [%u]\n", ret->master_protocol, ret->app_protocol, flow->detected_protocol_stack[0]);

  if((flow->risk != 0) && (flow->risk != flow->risk_shadow)) {
    /* Trick to avoid evaluating exceptions when nothing changed */
    ndpi_handle_risk_exceptions(ndpi_str, flow);
    flow->risk_shadow = flow->risk;
  }

  switch(ret->proto.app_protocol) {
  case NDPI_PROTOCOL_MICROSOFT_AZURE:
    ndpi_reconcile_msteams_udp(ndpi_str, flow, flow->detected_protocol_stack[1]);
    break;

    /*
      Skype for a host doing MS Teams means MS Teams
      (MS Teams uses Skype as transport protocol for voice/video)
    */
  case NDPI_PROTOCOL_MSTEAMS:
    if(flow->l4_proto == IPPROTO_TCP) {
      // printf("====>> NDPI_PROTOCOL_MSTEAMS\n");

      if(ndpi_str->msteams_cache)
	ndpi_lru_add_to_cache(ndpi_str->msteams_cache,
			      make_msteams_key(flow, 1 /* client */),
			      0 /* dummy */,
			      ndpi_get_current_time(flow));
    }
    break;

  case NDPI_PROTOCOL_STUN:
    if(flow->guessed_protocol_id_by_ip == NDPI_PROTOCOL_MICROSOFT_AZURE)
      ndpi_reconcile_msteams_udp(ndpi_str, flow, NDPI_PROTOCOL_STUN);
    break;

  case NDPI_PROTOCOL_TLS:
    /*
      When Teams is unable to communicate via UDP
      it switches to TLS.TCP. Let's try to catch it
    */
    if((flow->guessed_protocol_id_by_ip == NDPI_PROTOCOL_MICROSOFT_AZURE)
       && (ret->proto.master_protocol == NDPI_PROTOCOL_UNKNOWN)
       && ndpi_str->msteams_cache
       ) {
      u_int16_t dummy;

      if(ndpi_lru_find_cache(ndpi_str->msteams_cache,
			     make_msteams_key(flow, 1 /* client */),
			     &dummy, 0 /* Don't remove it as it can be used for other connections */,
			     ndpi_get_current_time(flow))) {
	ndpi_int_change_protocol(flow,
				 NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_TLS,
				 NDPI_CONFIDENCE_DPI_PARTIAL);
      }
    } else if(flow->guessed_protocol_id_by_ip == NDPI_PROTOCOL_TELEGRAM) {
      ndpi_int_change_protocol(flow,
			       flow->guessed_protocol_id_by_ip, flow->detected_protocol_stack[0],
			       NDPI_CONFIDENCE_DPI_PARTIAL);
    }
    break;

  case NDPI_PROTOCOL_MSTEAMS_CALL:
    ndpi_reconcile_msteams_call_udp(flow);
    break;

  case NDPI_PROTOCOL_RDP:
    ndpi_set_risk(ndpi_str, flow, NDPI_DESKTOP_OR_FILE_SHARING_SESSION, "Found RDP"); /* Remote assistance */
    break;

  case NDPI_PROTOCOL_ANYDESK:
    if(flow->l4_proto == IPPROTO_TCP) /* TCP only */
      ndpi_set_risk(ndpi_str, flow, NDPI_DESKTOP_OR_FILE_SHARING_SESSION, "Found AnyDesk"); /* Remote assistance */
    break;

    /* Generic container for microsoft subprotocols */
  case NDPI_PROTOCOL_MICROSOFT:
    switch(flow->guessed_protocol_id_by_ip) {
    case NDPI_PROTOCOL_MICROSOFT_365:
    case NDPI_PROTOCOL_MS_ONE_DRIVE:
    case NDPI_PROTOCOL_MS_OUTLOOK:
    case NDPI_PROTOCOL_MSTEAMS:
      ndpi_int_change_protocol(flow,
			       flow->guessed_protocol_id_by_ip, flow->detected_protocol_stack[1],
			       NDPI_CONFIDENCE_DPI_PARTIAL);
      break;
    }
    break;

    /* Generic container for google subprotocols */
  case NDPI_PROTOCOL_GOOGLE:
    switch(flow->guessed_protocol_id_by_ip) {
    case NDPI_PROTOCOL_GOOGLE_CLOUD:
      ndpi_int_change_protocol(flow,
			       flow->guessed_protocol_id_by_ip, flow->detected_protocol_stack[1],
			       NDPI_CONFIDENCE_DPI_PARTIAL);

      break;
    }
    break;

  case NDPI_PROTOCOL_UNKNOWN:
    break;
  } /* switch */

  ret->proto.master_protocol = flow->detected_protocol_stack[1],
    ret->proto.app_protocol = flow->detected_protocol_stack[0];

  for(i=0; i<2; i++) {
    switch(ndpi_get_proto_breed(ndpi_str, flow->detected_protocol_stack[i])) {
    case NDPI_PROTOCOL_UNSAFE:
    case NDPI_PROTOCOL_POTENTIALLY_DANGEROUS:
    case NDPI_PROTOCOL_DANGEROUS:

      if(flow->detected_protocol_stack[i] == NDPI_PROTOCOL_SMBV1) {
  	struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
	/*
	  Same as for smb.c we need to avoid sending warnings for
	  requests sent to a broadcast address that can be sent to
	  query old devices. As we see no MAC addresses in nDPI
	  it's not simple to detect this fact, so we will use some
	  heuristic here.
	*/

	if(packet->payload_packet_len > 86 /* SMB command */) {
	  if(packet->payload[86] == 0x25 /* SMB Trans */)
	    skip_risk = 1;
	}
      }

      if(!skip_risk)
	ndpi_set_risk(ndpi_str, flow, NDPI_UNSAFE_PROTOCOL, NULL);
      break;

    default:
      /* Nothing to do */
      break;
    }
  } /* for */
}

/* ********************************************************************************* */

/* #define BITTORRENT_CACHE_DEBUG */

int search_into_bittorrent_cache(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow) {

#ifdef BITTORRENT_CACHE_DEBUG
  printf("[%s:%u] search_into_bittorrent_cache(%u, %u) [bt_check_performed=%d]\n",
	 __FILE__, __LINE__, ntohs(flow->c_port), ntohs(flow->s_port),
	 flow->bt_check_performed);
#endif

  if(flow->bt_check_performed /* Do the check once */)
    return(0);

  if(ndpi_struct->bittorrent_cache) {
    u_int16_t cached_proto;
    u_int8_t found = 0;
    u_int64_t key, key1, key2;

    flow->bt_check_performed = 1;

    /* Check cached communications */
    key = make_bittorrent_peers_key(flow);
    key1 = make_bittorrent_host_key(flow, 1, 0), key2 = make_bittorrent_host_key(flow, 0, 0);

    found =
      ndpi_lru_find_cache(ndpi_struct->bittorrent_cache, key, &cached_proto, 0 /* Don't remove it as it can be used for other connections */, ndpi_get_current_time(flow))
      || ndpi_lru_find_cache(ndpi_struct->bittorrent_cache, key1, &cached_proto, 0     /* Don't remove it as it can be used for other connections */, ndpi_get_current_time(flow))
      || ndpi_lru_find_cache(ndpi_struct->bittorrent_cache, key2, &cached_proto, 0     /* Don't remove it as it can be used for other connections */, ndpi_get_current_time(flow));

#ifdef BITTORRENT_CACHE_DEBUG
    printf("[BitTorrent] *** [%s] SEARCHING ports %u / %u [0x%llx][0x%llx][0x%llx][found: %u]\n",
           flow->l4_proto == IPPROTO_UDP ? "UDP": "TCP",
           ntohs(flow->c_port), ntohs(flow->s_port),
           (long long unsigned int)key, (long long unsigned int)key1, (long long unsigned int)key2, found);
#endif

    return(found);
  }

  return(0);
}

/* ********************************************************************************* */

/*
  NOTE:

  This function is called only by ndpi_detection_giveup() as it checks
  flows that have anomalous conditions such as SYN+RST ACK+RST....
  As these conditions won't happen with nDPI protocol-detected protocols
  it is not necessary to call this function elsewhere
*/
static void ndpi_check_tcp_flags(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow) {
  // printf("[TOTAL] %u / %u [tot: %u]\n", flow->packet_direction_complete_counter[0], flow->packet_direction_complete_counter[1], flow->all_packets_counter);
  bool is_probing = false;

  if((flow->l4.tcp.cli2srv_tcp_flags & TH_SYN)
     && (flow->l4.tcp.srv2cli_tcp_flags & TH_RST)
     && (flow->packet_counter == 0 /* Ignore connections terminated by RST but that exchanged data (3WH + RST) */))
    ndpi_set_risk(ndpi_struct, flow, NDPI_TCP_ISSUES, "Connection refused (server)"), is_probing = true;
  else if((flow->l4.tcp.cli2srv_tcp_flags & TH_SYN)
	  && (flow->l4.tcp.cli2srv_tcp_flags & TH_RST)
	  && (flow->packet_counter == 0 /* Ignore connections terminated by RST but that exchanged data (3WH + RST) */))
    ndpi_set_risk(ndpi_struct, flow, NDPI_TCP_ISSUES, "Connection refused (client)"), is_probing = true;
  else if((flow->l4.tcp.srv2cli_tcp_flags & TH_RST) && (flow->packet_direction_complete_counter[1 /* server -> client */] == 1))
    ndpi_set_risk(ndpi_struct, flow, NDPI_TCP_ISSUES, "Connection refused"), is_probing = true;

  if(is_probing)
    ndpi_set_risk(ndpi_struct, flow, NDPI_PROBING_ATTEMPT, "TCP probing attempt");
}

/* ******************************************************************** */

static void ndpi_check_probing_attempt(struct ndpi_detection_module_struct *ndpi_str,
                                       struct ndpi_flow_struct *flow) {
  /* TODO: check UDP traffic too */

  if((flow->l4_proto == IPPROTO_TCP)
     && (flow->l4.tcp.cli2srv_tcp_flags & TH_PUSH)
     && (flow->l4.tcp.srv2cli_tcp_flags & TH_PUSH)) {
    if(flow->packet_direction_counter[0]
       && flow->packet_direction_counter[1]) {
      /* Both directions observed */
      /* Nothing to do */
    } else {
      /* Skipping rules where an early match might be confused with a probing attempt */
      if(flow->confidence == NDPI_CONFIDENCE_DPI) {
	switch(flow->detected_protocol_stack[0]) {
	case NDPI_PROTOCOL_SSH:
	  if(flow->protos.ssh.hassh_server[0] == '\0')
	    ndpi_set_risk(ndpi_str, flow, NDPI_PROBING_ATTEMPT, "SSH Probing");
	  break;

	case NDPI_PROTOCOL_TLS:
	case NDPI_PROTOCOL_MAIL_SMTPS:
	case NDPI_PROTOCOL_MAIL_POPS:
	case NDPI_PROTOCOL_MAIL_IMAPS:
	case NDPI_PROTOCOL_DTLS:
	  if(flow->host_server_name[0] == '\0')
	    ndpi_set_risk(ndpi_str, flow, NDPI_PROBING_ATTEMPT, "TLS Probing");
	  break;

	case NDPI_PROTOCOL_QUIC:
	  if(flow->host_server_name[0] == '\0')
	    ndpi_set_risk(ndpi_str, flow, NDPI_PROBING_ATTEMPT, "QUIC Probing");
	  break;
	}
      }
    }
  }
}

/* ********************************************************************************* */

static int is_unidir_traffic_exception(struct ndpi_flow_struct *flow) {

  switch(flow->detected_protocol_stack[0]) {
  case NDPI_PROTOCOL_NETFLOW:
  case NDPI_PROTOCOL_SFLOW:
  case NDPI_PROTOCOL_COLLECTD:
    return 1;

  case NDPI_PROTOCOL_SYSLOG:
  case NDPI_PROTOCOL_MDNS:
  case NDPI_PROTOCOL_SONOS:
  case NDPI_PROTOCOL_RTP:
    if(flow->l4_proto == IPPROTO_UDP)
      return 1;
  }
  return 0;
}

/* ********************************************************************************* */

static void internal_giveup(struct ndpi_detection_module_struct *ndpi_struct,
                            struct ndpi_flow_struct *flow,
                            ndpi_protocol *ret) {

  if(flow->already_gaveup) {
    NDPI_LOG_INFO(ndpi_struct, "Already called!\n"); /* We shoudn't be here ...*/
    return;
  }
  flow->already_gaveup = 1;

  NDPI_LOG_DBG2(ndpi_struct, "");

  /* This (internal) function is expected to be called for **every** flows,
     exactly once, as **last** code processing the flow itself */

  /* TODO: this function is similar to ndpi_detection_giveup(). We should try to unify them
     or to have two more distinct logics...
     The/A critical point is that ndpi_detection_giveup() is public and it is always used by
     any programs linking to libnDPI: we must be sure to not change the external behavior
   */

  /* ***
   * *** We can't access ndpi_str->packet from this function!!
   * ***/

  if(!ndpi_is_multi_or_broadcast(flow) &&
     !is_unidir_traffic_exception(flow)) {

    if(flow->packet_direction_complete_counter[flow->client_packet_direction] == 0)
      ndpi_set_risk(ndpi_struct, flow, NDPI_UNIDIRECTIONAL_TRAFFIC, "No client to server traffic");
    else if(flow->packet_direction_complete_counter[!flow->client_packet_direction] == 0)
      ndpi_set_risk(ndpi_struct, flow, NDPI_UNIDIRECTIONAL_TRAFFIC, "No server to client traffic");
  }

  /* TODO */
  (void)ret;
}

/* ********************************************************************************* */

ndpi_protocol ndpi_detection_giveup(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow,
				    u_int8_t *protocol_was_guessed) {
  ndpi_protocol ret = NDPI_PROTOCOL_NULL;
  u_int16_t cached_proto;

  /* *** We can't access ndpi_str->packet from this function!! *** */

  *protocol_was_guessed = 0;

  if(!ndpi_str || !flow)
    return(ret);

  if(flow->l4_proto == IPPROTO_TCP) {
    ndpi_check_tcp_flags(ndpi_str, flow);
    ndpi_check_probing_attempt(ndpi_str, flow);
  }

  /* Init defaults */
  ret.proto.master_protocol = flow->detected_protocol_stack[1],
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  ret.protocol_by_ip = flow->guessed_protocol_id_by_ip;
#ifndef __KERNEL__
  ret.category = flow->category;
#endif

  /* Ensure that we don't change our mind if detection is already complete */
  if(ret.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN) {
    /* Reason: public "ndpi_detection_giveup", already classified */
    internal_giveup(ndpi_str, flow, &ret);
    return(ret);
  }

  /* Partial classification */
  if(flow->fast_callback_protocol_id != NDPI_PROTOCOL_UNKNOWN) {
    ndpi_set_detected_protocol(ndpi_str, flow, flow->fast_callback_protocol_id, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI_PARTIAL);
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  }

  /* Check some caches */

  /* Does it looks like BitTorrent? */
  if(ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
     search_into_bittorrent_cache(ndpi_str, flow)) {
    ndpi_set_detected_protocol(ndpi_str, flow, NDPI_PROTOCOL_BITTORRENT, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI_PARTIAL_CACHE);
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  }
  /* Does it looks like some Mining protocols? */
  if(ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
     ndpi_str->mining_cache &&
     ndpi_lru_find_cache(ndpi_str->mining_cache, mining_make_lru_cache_key(flow),
			 &cached_proto, 0 /* Don't remove it as it can be used for other connections */,
			 ndpi_get_current_time(flow))) {
    ndpi_set_detected_protocol(ndpi_str, flow, cached_proto, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI_PARTIAL_CACHE);
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  }

  /* Does it looks like Ookla? */
  if(ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
     ntohs(flow->s_port) == 8080 && ookla_search_into_cache(ndpi_str, flow)) {
    ndpi_set_detected_protocol(ndpi_str, flow, NDPI_PROTOCOL_OOKLA, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI_PARTIAL_CACHE);
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  }

  /* TODO: not sure about the best "order" among fully encrypted logic, classification by-port and classification by-ip...*/
  if(ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
     flow->first_pkt_fully_encrypted == 1) {
    ndpi_set_risk(ndpi_str, flow, NDPI_OBFUSCATED_TRAFFIC, "Fully Encrypted");
  }

  /* If guess_ip_before_port is enabled, classify by-ip first */
  if((ndpi_str->cfg.guess_ip_before_port))
    {
      if((ndpi_str->cfg.guess_on_giveup & NDPI_GIVEUP_GUESS_BY_IP) &&
	 ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
	 flow->guessed_protocol_id_by_ip != NDPI_PROTOCOL_UNKNOWN) {

	ndpi_set_detected_protocol(ndpi_str, flow,
				   flow->guessed_protocol_id_by_ip,
				   ret.proto.master_protocol,
				   NDPI_CONFIDENCE_MATCH_BY_IP);
	ret.proto.app_protocol = flow->detected_protocol_stack[0];
      }
    }
  /* Classification by-port */
  if((ndpi_str->cfg.guess_on_giveup & NDPI_GIVEUP_GUESS_BY_PORT) &&
     ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
     flow->guessed_protocol_id != NDPI_PROTOCOL_UNKNOWN) {
    ndpi_set_detected_protocol(ndpi_str, flow, flow->guessed_protocol_id, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_MATCH_BY_PORT);
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  }
  /* Classification by-ip, as last effort if guess_ip_before_port is disabled*/
  if(!(ndpi_str->cfg.guess_ip_before_port) &&
     (ndpi_str->cfg.guess_on_giveup & NDPI_GIVEUP_GUESS_BY_IP) &&
     ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
     flow->guessed_protocol_id_by_ip != NDPI_PROTOCOL_UNKNOWN) {

    ndpi_set_detected_protocol(ndpi_str, flow,
			       flow->guessed_protocol_id_by_ip,
			       ret.proto.master_protocol,
			       NDPI_CONFIDENCE_MATCH_BY_IP);
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  }

  if(ret.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN) {
    *protocol_was_guessed = 1;
#ifndef __KERNEL__
    ndpi_fill_protocol_category(ndpi_str, flow, &ret);
#endif
  }

  /* Reason: public "ndpi_detection_giveup" */
  internal_giveup(ndpi_str, flow, &ret);

  return(ret);
}

/* ********************************************************************************* */

void ndpi_process_extra_packet(struct ndpi_detection_module_struct *ndpi_str,
			       struct ndpi_flow_struct *flow,
			       const unsigned char *packet_data, const unsigned short packetlen,
			       const u_int64_t current_time_ms,
			       struct ndpi_flow_input_info *input_info) {
  if(flow == NULL)
    return;

  /* need at least 20 bytes for ip header */
  if(packetlen < 20) {
    return;
  }

  /* set up the packet headers for the extra packet function to use if it wants */
  if(ndpi_init_packet(ndpi_str, flow, current_time_ms, packet_data, packetlen, input_info) != 0)
    return;

  ndpi_connection_tracking(ndpi_str, flow);

  /* call the extra packet function (which may add more data/info to flow) */
  if(flow->extra_packets_func) {
    struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);

    /* Safety check to skip non TCP/UDP packets sent to extra dissectors */
    if((packet != NULL) && ((packet->udp != NULL) || (packet->tcp != NULL))) {
      if((flow->extra_packets_func(ndpi_str, flow) == 0) ||
	 (!flow->monitoring && ++flow->num_extra_packets_checked == flow->max_extra_packets_to_check)) {
	flow->extra_packets_func = NULL; /* Done */
      }
    }
  }
}

/* ********************************************************************************* */
#ifdef __KERNEL__
  void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow,
                                 ndpi_protocol *ret) 
  {
  }
#endif

#ifndef __KERNEL__
int ndpi_load_ip_category(struct ndpi_detection_module_struct *ndpi_str,
			  const char *ip_address_and_mask,
			  ndpi_protocol_category_t category,
			  void *user_data) {
  ndpi_patricia_node_t *node = NULL;
  int bits = 32, is_ipv6 = 0;
  char *ptr;
  char ipbuf[128];

  if(ip_address_and_mask[0] == '[') {
    is_ipv6 = 1;
    bits = 128;
    ip_address_and_mask++; /* Strip '[' */
  }

  strncpy(ipbuf, ip_address_and_mask, sizeof(ipbuf) - 1);
  ipbuf[sizeof(ipbuf) - 1] = '\0';

  ptr = strrchr(ipbuf, '/');

  if(ptr) {
    *(ptr++) = '\0';
    if(atoi(ptr) >= 0 && atoi(ptr) <= 32)
      bits = atoi(ptr);
  }
  ptr = strrchr(ipbuf, ']');
  if(ptr)
    *ptr = '\0'; /* Strip ']' */

  if(!is_ipv6 && ndpi_str->custom_categories.ipAddresses_shadow) {
    struct in_addr pin;

    if(inet_pton(AF_INET, ipbuf, &pin) != 1) {
      NDPI_LOG_DBG2(ndpi_str, "Invalid ip4/ip4+netmask: %s\n", ip_address_and_mask);
      return(-1);
    }

    node = add_to_ptree(ndpi_str->custom_categories.ipAddresses_shadow, AF_INET, &pin, bits);
  } else if(is_ipv6 && ndpi_str->custom_categories.ipAddresses6_shadow) {
    struct in6_addr pin6;

    if(inet_pton(AF_INET6, ipbuf, &pin6) != 1) {
      NDPI_LOG_DBG2(ndpi_str, "Invalid ip6/ip6+netmask: %s\n", ip_address_and_mask);
      return(-1);
    }
    node = add_to_ptree(ndpi_str->custom_categories.ipAddresses6_shadow, AF_INET6, &pin6, bits);
  } else {
    return(-1);
  }

  if(node != NULL) {
    node->value.u.uv32.user_value = (u_int16_t)category, node->value.u.uv32.additional_user_value = 0;
    node->custom_user_data = user_data;
  }


  return(0);
}

/* ********************************************************************************* */

int ndpi_load_hostname_category(struct ndpi_detection_module_struct *ndpi_str,
				const char *name_to_add,
				ndpi_protocol_category_t category) {
  if(ndpi_str->custom_categories.sc_hostnames_shadow == NULL)
    return(-1);

  return(ndpi_domain_classify_add(ndpi_str, ndpi_str->custom_categories.sc_hostnames_shadow,
				  (u_int16_t)category, (char*)name_to_add) ? 0 : -1);
}

/* ********************************************************************************* */

/* Loads an IP or name category */
int ndpi_load_category(struct ndpi_detection_module_struct *ndpi_struct, const char *ip_or_name,
		       ndpi_protocol_category_t category, void *user_data) {
  int rv;

  /* Try to load as IP address first */
  rv = ndpi_load_ip_category(ndpi_struct, ip_or_name, category, user_data);

  if(rv < 0) {
    /*
      IP load failed, load as hostname

      NOTE:
      we cannot add user_data here as with Aho-Corasick this
      information would not be used
    */
    rv = ndpi_load_hostname_category(ndpi_struct, ip_or_name, category);
  }

  return(rv);
}

/* ********************************************************************************* */

int ndpi_enable_loaded_categories(struct ndpi_detection_module_struct *ndpi_str) {
  int i;
  static char *built_in = "built-in";

  if(ndpi_str->custom_categories.categories_loaded)
    return(-1); /* Already loaded */

  /* First add the nDPI known categories matches */
  for(i = 0; category_match[i].string_to_match != NULL; i++)
    ndpi_load_category(ndpi_str, category_match[i].string_to_match,
		       category_match[i].protocol_category, built_in);

  ndpi_domain_classify_free(ndpi_str->custom_categories.sc_hostnames);
  ndpi_str->custom_categories.sc_hostnames        = ndpi_str->custom_categories.sc_hostnames_shadow;
  ndpi_str->custom_categories.sc_hostnames_shadow = ndpi_domain_classify_alloc();

  if(ndpi_str->custom_categories.ipAddresses != NULL)
    ndpi_patricia_destroy((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses, free_ptree_data);

  if(ndpi_str->custom_categories.ipAddresses6 != NULL)
    ndpi_patricia_destroy((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6, free_ptree_data);

  ndpi_str->custom_categories.ipAddresses = ndpi_str->custom_categories.ipAddresses_shadow;
  ndpi_str->custom_categories.ipAddresses_shadow = ndpi_patricia_new(32 /* IPv4 */);

  ndpi_str->custom_categories.ipAddresses6 = ndpi_str->custom_categories.ipAddresses6_shadow;
  ndpi_str->custom_categories.ipAddresses6_shadow = ndpi_patricia_new(128 /* IPv6 */);

  ndpi_str->custom_categories.categories_loaded = 1;

  return(0);
}

/* ********************************************************************************* */

/* NOTE u_int32_t is represented in network byte order */
void* ndpi_find_ipv4_category_userdata(struct ndpi_detection_module_struct *ndpi_str,
				       u_int32_t saddr) {
  ndpi_patricia_node_t *node;

  if(saddr == 0 || !ndpi_str || !ndpi_str->custom_categories.ipAddresses)
    node = NULL;
  else {
    ndpi_prefix_t prefix;

    ndpi_fill_prefix_v4(&prefix, (struct in_addr *) &saddr, 32,
			((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses)->maxbits);
    node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses, &prefix);
  }

  return(node ? node->custom_user_data : NULL);
}

/* ********************************************************************************* */

void* ndpi_find_ipv6_category_userdata(struct ndpi_detection_module_struct *ndpi_str,
				       struct in6_addr *saddr) {
  ndpi_patricia_node_t *node;

  if(!saddr || !ndpi_str || !ndpi_str->custom_categories.ipAddresses6)
    node = NULL;
  else {
    ndpi_prefix_t prefix;

    ndpi_fill_prefix_v6(&prefix, saddr, 128,
			((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6)->maxbits);
    node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses6, &prefix);
  }

  return(node ? node->custom_user_data : NULL);
}


/* ********************************************************************************* */

/* NOTE u_int32_t is represented in network byte order */
int ndpi_fill_ip_protocol_category(struct ndpi_detection_module_struct *ndpi_str,
				   struct ndpi_flow_struct *flow,
				   u_int32_t saddr, u_int32_t daddr,
				   ndpi_protocol *ret) {
  bool match_client = true;

  ret->custom_category_userdata = NULL;

  if(ndpi_str->custom_categories.categories_loaded &&
     ndpi_str->custom_categories.ipAddresses) {

    ndpi_prefix_t prefix;
    ndpi_patricia_node_t *node;

    if(saddr == 0)
      node = NULL;
    else {
      /* Make sure all in network byte order otherwise compares wont work */
      ndpi_fill_prefix_v4(&prefix, (struct in_addr *) &saddr, 32,
			  ((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses)->maxbits);
      node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses, &prefix);
    }

    if(node == NULL) {
      if(daddr != 0) {
	ndpi_fill_prefix_v4(&prefix, (struct in_addr *) &daddr, 32,
			    ((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses)->maxbits);
	node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses, &prefix);
	match_client = false;
      }
    } else {
      match_client = true;
    }

    if(node) {
      ret->category = (ndpi_protocol_category_t) node->value.u.uv32.user_value;
      ret->custom_category_userdata = node->custom_user_data;

      if((ret->category == CUSTOM_CATEGORY_MALWARE) && (match_client == false)) {
	ndpi_set_risk(ndpi_str, flow, NDPI_MALWARE_HOST_CONTACTED, "Client contacted malware host");
      }

      return(1);
    }
  }

  ret->category = ndpi_get_proto_category(ndpi_str, *ret);

  return(0);
}

/* ********************************************************************************* */

int ndpi_fill_ipv6_protocol_category(struct ndpi_detection_module_struct *ndpi_str,
				     struct ndpi_flow_struct *flow,
				     struct in6_addr *saddr, struct in6_addr *daddr,
				     ndpi_protocol *ret) {
  bool match_client = true;

  ret->custom_category_userdata = NULL;

  if(ndpi_str->custom_categories.categories_loaded &&
     ndpi_str->custom_categories.ipAddresses6) {

    ndpi_prefix_t prefix;
    ndpi_patricia_node_t *node;

    ndpi_fill_prefix_v6(&prefix, saddr, 128,
                        ((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6)->maxbits);
    node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses6, &prefix);

    if(node == NULL) {
      ndpi_fill_prefix_v6(&prefix, daddr, 128,
                          ((ndpi_patricia_tree_t *) ndpi_str->custom_categories.ipAddresses6)->maxbits);
      node = ndpi_patricia_search_best(ndpi_str->custom_categories.ipAddresses6, &prefix);
      match_client = false;
    } else {
      match_client = true;
    }

    if(node) {
      ret->category = (ndpi_protocol_category_t) node->value.u.uv32.user_value;
      ret->custom_category_userdata = node->custom_user_data;

      if((ret->category == CUSTOM_CATEGORY_MALWARE) && (match_client == false)) {
	ndpi_set_risk(ndpi_str, flow, NDPI_MALWARE_HOST_CONTACTED, "Client contacted malware host");
      }

      return(1);
    }
  }

  ret->category = ndpi_get_proto_category(ndpi_str, *ret);

  return(0);
}

/* ********************************************************************************* */

void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow,
				 ndpi_protocol *ret) {
  if((ret->proto.master_protocol == NDPI_PROTOCOL_UNKNOWN)
     && (ret->proto.app_protocol == NDPI_PROTOCOL_UNKNOWN))
    return;

  if(ndpi_str->custom_categories.categories_loaded) {
    if(flow->guessed_header_category != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) {
      flow->category = ret->category = flow->guessed_header_category;
      return;
    }

    if(flow->host_server_name[0] != '\0') {
      u_int32_t id;
      int rc = ndpi_match_custom_category(ndpi_str, flow->host_server_name,
					  strlen(flow->host_server_name), &id);
      if(rc == 0) {
	flow->category = ret->category = (ndpi_protocol_category_t) id;
	return;
      }
    }
  }
  flow->category = ret->category = ndpi_get_proto_category(ndpi_str, *ret);
}
#endif

/* ********************************************************************************* */

static void ndpi_reset_packet_line_info(struct ndpi_packet_struct *packet) {
  packet->parsed_lines = 0, packet->empty_line_position_set = 0, packet->host_line.ptr = NULL,
    packet->host_line.len = 0, packet->referer_line.ptr = NULL, packet->referer_line.len = 0,
    packet->authorization_line.len = 0, packet->authorization_line.ptr = NULL,
    packet->content_line.ptr = NULL, packet->content_line.len = 0, packet->accept_line.ptr = NULL,
    packet->accept_line.len = 0, packet->user_agent_line.ptr = NULL, packet->user_agent_line.len = 0,
    packet->http_url_name.ptr = NULL, packet->http_url_name.len = 0,
    packet->content_disposition_line.ptr = NULL,
    packet->content_disposition_line.len = 0,
    packet->http_origin.len = 0, packet->http_origin.ptr = NULL,
    packet->server_line.ptr = NULL,
    packet->server_line.len = 0, packet->http_method.ptr = NULL, packet->http_method.len = 0,
    packet->http_response.ptr = NULL, packet->http_response.len = 0,
    packet->forwarded_line.ptr = NULL, packet->forwarded_line.len = 0;
    packet->upgrade_line.ptr = NULL, packet->upgrade_line.len = 0;
    packet->bootid.ptr = NULL, packet->bootid.len = 0;
    packet->usn.ptr = NULL, packet->usn.len = 0;
    packet->cache_controle.ptr = NULL, packet->cache_controle.len = 0;
    packet->location.ptr = NULL, packet->location.len = 0;
    packet->household_smart_speaker_audio.ptr = NULL, packet->household_smart_speaker_audio.len = 0;
    packet->rincon_household.ptr = NULL, packet->rincon_household.len = 0;
    packet->rincon_bootseq.ptr = NULL, packet->rincon_bootseq.len = 0;
    packet->rincon_wifimode.ptr = NULL, packet->rincon_wifimode.len = 0;
    packet->rincon_variant.ptr = NULL, packet->rincon_variant.len = 0;
    packet->sonos_securelocation.ptr = NULL, packet->sonos_securelocation.len = 0;
    packet->securelocation_upnp.ptr = NULL, packet->securelocation_upnp.len = 0;
    packet->location_smart_speaker_audio.ptr = NULL, packet->location_smart_speaker_audio.len = 0;
    packet->nt.ptr = NULL, packet->nt.len = 0;
    packet->nts.ptr = NULL, packet->nts.len = 0;
    packet->man.ptr = NULL, packet->man.len = 0;
    packet->mx.ptr = NULL, packet->mx.len = 0;
    packet->st.ptr = NULL, packet->st.len = 0;
}

/* ********************************************************************************* */

static int ndpi_is_ntop_protocol(ndpi_protocol *ret) {
  if((ret->proto.master_protocol == NDPI_PROTOCOL_HTTP) && (ret->proto.app_protocol == NDPI_PROTOCOL_NTOP))
    return(1);
  else
    return(0);
}

/* ********************************************************************************* */

static void ndpi_search_shellscript(struct ndpi_detection_module_struct *ndpi_struct,
                                    struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct const * const packet = ndpi_get_packet_struct(ndpi_struct);

  NDPI_LOG_DBG(ndpi_struct, "search Shellscript\n");

  if (packet->payload_packet_len < 3)
    return;

  if (packet->payload[0] != '#' ||
      packet->payload[1] != '!' ||
      (packet->payload[2] != '/' && packet->payload[2] != ' '))
    return;

  NDPI_LOG_INFO(ndpi_struct, "found Shellscript\n");
  ndpi_set_risk(ndpi_struct, flow, NDPI_POSSIBLE_EXPLOIT, "Shellscript found");
}

/* ********************************************************************************* */

/* ELF format specs: https://man7.org/linux/man-pages/man5/elf.5.html */
static void ndpi_search_elf(struct ndpi_detection_module_struct *ndpi_struct,
                            struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct const * const packet = ndpi_get_packet_struct(ndpi_struct);
  static const uint32_t elf_signature = 0x7f454c46; /* [DEL]ELF */
  static const uint32_t max_version = 6;

  NDPI_LOG_DBG(ndpi_struct, "search ELF file\n");

  if (packet->payload_packet_len < 24)
    return;

  if (ntohl(get_u_int32_t(packet->payload, 0)) != elf_signature)
    return;

  if (le32toh(get_u_int32_t(packet->payload, 20)) > max_version)
    return;

  NDPI_LOG_INFO(ndpi_struct, "found ELF file\n");
  ndpi_set_risk(ndpi_struct, flow, NDPI_BINARY_APPLICATION_TRANSFER, "ELF found");
}

/* ********************************************************************************* */

/* PE32/PE32+ format specs: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format */
static void ndpi_search_portable_executable(struct ndpi_detection_module_struct *ndpi_struct,
                                            struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct const * const packet = ndpi_get_packet_struct(ndpi_struct);
  static const uint16_t dos_signature = 0x4d5a; /* MZ */
  static const uint32_t pe_signature = 0x50450000; /* PE */

  NDPI_LOG_DBG(ndpi_struct, "search Portable Executable (PE) file\n");

  if (packet->payload_packet_len < 0x3C /* offset to PE header */ + 4)
    return;

  if (ntohs(get_u_int16_t(packet->payload, 0)) != dos_signature)
    return;

  uint32_t const pe_offset = le32toh(get_u_int32_t(packet->payload, 0x3C));
  if ((u_int32_t)(packet->payload_packet_len - 4) <= pe_offset ||
      be32toh(get_u_int32_t(packet->payload, pe_offset)) != pe_signature)
    return;

  NDPI_LOG_INFO(ndpi_struct, "found Portable Executable (PE) file\n");
  ndpi_set_risk(ndpi_struct, flow, NDPI_BINARY_APPLICATION_TRANSFER, "Portable Executable (PE32/PE32+) found");
}

/* ********************************************************************************* */

static int ndpi_check_protocol_port_mismatch_exceptions(default_ports_tree_node_t *expected_proto,
							ndpi_protocol *returned_proto) {
  /*
    For TLS (and other protocols) it is not simple to guess the exact protocol so before
    triggering an alert we need to make sure what we have exhausted all the possible
    options available
  */

  if(ndpi_is_ntop_protocol(returned_proto)) return(1);

  if(returned_proto->proto.master_protocol == NDPI_PROTOCOL_TLS) {
    switch(expected_proto->proto->protoId) {
    case NDPI_PROTOCOL_MAIL_IMAPS:
    case NDPI_PROTOCOL_MAIL_POPS:
    case NDPI_PROTOCOL_MAIL_SMTPS:
      return(1); /* This is a reasonable exception */
      break;
    }
  }

  return(0);
}

/* ****************************************************** */
static int ndpi_do_guess(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow, ndpi_protocol *ret) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  ret->proto.master_protocol = ret->proto.app_protocol = NDPI_PROTOCOL_UNKNOWN;
#ifndef __KERNEL__
  ret->category = 0;
#endif

  if(packet->iphv6 || packet->iph) {
    u_int8_t user_defined_proto;

    /* guess protocol */
    flow->guessed_protocol_id = (int16_t) guess_protocol_id(ndpi_str, flow->l4_proto,
							    ntohs(flow->c_port), ntohs(flow->s_port),
							    &user_defined_proto);
    flow->guessed_protocol_id_by_ip = ndpi_guess_host_protocol_id(ndpi_str, flow);
    flow->fast_callback_protocol_id = NDPI_PROTOCOL_UNKNOWN;

    ret->protocol_by_ip = flow->guessed_protocol_id_by_ip;
#ifndef __KERNEL__
    if(ndpi_str->custom_categories.categories_loaded && packet->iph) {
      if(ndpi_str->ndpi_num_custom_protocols != 0)
	ndpi_fill_ip_protocol_category(ndpi_str, flow, flow->c_address.v4, flow->s_address.v4, ret);
      else
        ndpi_fill_ipv6_protocol_category(ndpi_str, flow, (struct in6_addr *)flow->c_address.v6,
					 (struct in6_addr *)flow->s_address.v6, ret);
      flow->guessed_header_category = ret->category;
    } else
      flow->guessed_header_category = NDPI_PROTOCOL_CATEGORY_UNSPECIFIED;
#endif

    if(flow->guessed_protocol_id >= NDPI_MAX_SUPPORTED_PROTOCOLS) {
      /* This is a custom protocol and it has priority over everything else */
      ret->proto.master_protocol = NDPI_PROTOCOL_UNKNOWN,
	ret->proto.app_protocol = flow->guessed_protocol_id;
      flow->confidence = NDPI_CONFIDENCE_CUSTOM_RULE;
      ndpi_fill_protocol_category(ndpi_str, flow, ret);
      return(-1);
    }

    if(user_defined_proto && flow->guessed_protocol_id != NDPI_PROTOCOL_UNKNOWN) {
      ret->proto.master_protocol = NDPI_PROTOCOL_UNKNOWN;
      ret->proto.app_protocol = flow->guessed_protocol_id;
      flow->confidence = NDPI_CONFIDENCE_CUSTOM_RULE;
      ndpi_fill_protocol_category(ndpi_str, flow, ret);
      return(-1);
    }
  }

  if(flow->guessed_protocol_id_by_ip >= NDPI_MAX_SUPPORTED_PROTOCOLS) {
    NDPI_SELECTION_BITMASK_PROTOCOL_SIZE ndpi_selection_packet = {0};

    /* This is a custom protocol and it has priority over everything else */
    ret->proto.master_protocol = flow->guessed_protocol_id, ret->proto.app_protocol = flow->guessed_protocol_id_by_ip;

    flow->num_dissector_calls += ndpi_check_flow_func(ndpi_str, flow, &ndpi_selection_packet);
#if 0
     NDPI_LOG(flow ? flow->detected_protocol_stack[0] : NDPI_PROTOCOL_UNKNOWN, ndpi_str, NDPI_LOG_TRACE,
             "[%d/%d] dissector_calls %d\n",
	     flow->detected_protocol_stack[0], flow->detected_protocol_stack[1],flow->num_dissector_calls
	     );
#endif
    flow->confidence = NDPI_CONFIDENCE_CUSTOM_RULE;
    ndpi_fill_protocol_category(ndpi_str, flow, ret);
    return(-1);
  }

  return(0);
}

/* ********************************************************************************* */

static void fpc_update(struct ndpi_detection_module_struct *ndpi_str,
                       struct ndpi_flow_struct *flow,
		       u_int16_t fpc_master, u_int16_t fpc_app,
		       ndpi_fpc_confidence_t fpc_confidence)
{

  NDPI_LOG_DBG(ndpi_str, "FPC %d.%d/%s -> %d.%d/%s\n",
               flow->fpc.proto.master_protocol, flow->fpc.proto.app_protocol,
               ndpi_fpc_confidence_get_name(flow->fpc.confidence),
               fpc_master, fpc_app,
               ndpi_fpc_confidence_get_name(fpc_confidence));
  flow->fpc.proto.master_protocol = fpc_master;
  flow->fpc.proto.app_protocol = fpc_app;
  flow->fpc.confidence = fpc_confidence;
}

/* ********************************************************************************* */

static void fpc_check_eval(struct ndpi_detection_module_struct *ndpi_str,
                           struct ndpi_flow_struct *flow)
{
  u_int16_t fpc_dns_cached_proto;

  if(!ndpi_str->cfg.fpc_enabled)
    return;

  /* Order by most reliable logic */

  /* DPI */
  if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN) {
    fpc_update(ndpi_str, flow, flow->detected_protocol_stack[1],
               flow->detected_protocol_stack[0], NDPI_FPC_CONFIDENCE_DPI);
    return;
  }

  /* Check via fpc DNS cache */
  if(ndpi_str->fpc_dns_cache &&
     ndpi_lru_find_cache(ndpi_str->fpc_dns_cache, fpc_dns_cache_key_from_flow(flow),
                         &fpc_dns_cached_proto, 0 /* Don't remove it as it can be used for other connections */,
                         ndpi_get_current_time(flow))) {
    fpc_update(ndpi_str, flow, NDPI_PROTOCOL_UNKNOWN,
               fpc_dns_cached_proto, NDPI_FPC_CONFIDENCE_DNS);
    return;
  }

  /* Check via IP */
  if(flow->guessed_protocol_id_by_ip != NDPI_PROTOCOL_UNKNOWN) {
    fpc_update(ndpi_str, flow, NDPI_PROTOCOL_UNKNOWN,
               flow->guessed_protocol_id_by_ip, NDPI_FPC_CONFIDENCE_IP);
    return;
  }
}
/* ********************************************************************************* */

static char* ndpi_expected_ports_str(ndpi_port_range *default_ports, char *str, u_int str_len) {
  int rc;

  str[0] = '\0';

  if(default_ports[0].port_low != 0) {
    u_int8_t i, offset;

    offset = snprintf(str, str_len, "Expected on port ");

    for(i=0; (i<MAX_DEFAULT_PORTS) && (default_ports[i].port_low != 0); i++) {
      if(default_ports[i].port_low == default_ports[i].port_high)
        rc = snprintf(&str[offset], str_len-offset, "%s%u",
		      (i > 0) ? "," : "",
		      default_ports[i].port_low);
      else
        rc = snprintf(&str[offset], str_len-offset, "%s%u-%u",
                      (i > 0) ? "," : "",
                      default_ports[i].port_low,
                      default_ports[i].port_high);

      if(rc > 0)
	offset += rc;
      else
	break;
    }

    str[offset] = '\0';
  }

  return(str);
}

/* ********************************************************************************* */

static ndpi_protocol ndpi_internal_detection_process_packet(struct ndpi_detection_module_struct *ndpi_str,
							    struct ndpi_flow_struct *flow,
							    const unsigned char *packet_data,
							    const unsigned short packetlen,
							    const u_int64_t current_time_ms,
							    struct ndpi_flow_input_info *input_info) {
  struct ndpi_packet_struct *packet;
  NDPI_SELECTION_BITMASK_PROTOCOL_SIZE ndpi_selection_packet;
  u_int32_t num_calls = 0;
  ndpi_protocol ret;

  memset(&ret, 0, sizeof(ret));

  if((!flow) || (!ndpi_str) || (ndpi_str->finalized != 1))
    return(ret);

  flow->num_processed_pkts++;
  packet = ndpi_get_packet_struct(ndpi_str);

  NDPI_LOG_DBG(ndpi_str, "[%d/%d] START packet processing\n",
               flow->detected_protocol_stack[0],
	       flow->detected_protocol_stack[1]);

  ret.proto.master_protocol = flow->detected_protocol_stack[1],
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
  ret.protocol_by_ip = flow->guessed_protocol_id_by_ip;
#ifndef __KERNEL__
  ret.category = flow->category;
#endif

  if(flow->monit)
    memset(flow->monit, '\0', sizeof(*flow->monit));

  if(flow->fail_with_unknown) {
    // printf("%s(): FAIL_WITH_UNKNOWN\n", __FUNCTION__);
    return(ret);
  }

  if(ndpi_str->cfg.max_packets_to_process > 0 &&
     flow->num_processed_pkts >= ndpi_str->cfg.max_packets_to_process &&
     !flow->monitoring) {
    flow->extra_packets_func = NULL; /* To allow ndpi_extra_dissection_possible() to fail */
    flow->fail_with_unknown = 1;
    /* Let's try to update ndpi_str->input_info->in_pkt_dir even in this case.
     * It is quite uncommon, so we are not going to spend a lot of resources here... */
    if(ndpi_init_packet(ndpi_str, flow, current_time_ms, packet_data, packetlen, input_info) == 0)
      ndpi_connection_tracking(ndpi_str, flow);

    /* Reason: too many packets */
    internal_giveup(ndpi_str, flow, &ret);

    return(ret); /* Avoid spending too much time with this flow */
  }

  ndpi_str->current_ts = current_time_ms;

  /* Init default */

  if(flow->extra_packets_func) {
    ndpi_process_extra_packet(ndpi_str, flow, packet_data, packetlen, current_time_ms, input_info);
    /* Update in case of new match */
    ret.proto.master_protocol = flow->detected_protocol_stack[1];
    ret.proto.app_protocol = flow->detected_protocol_stack[0];
#ifndef __KERNEL__
      ret.category = flow->category;
#endif

    if(flow->extra_packets_func == NULL) {
      /* Reason: extra dissection ended */
      internal_giveup(ndpi_str, flow, &ret);
    }

    return(ret);
  } else if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN) {
    if(ndpi_init_packet(ndpi_str, flow, current_time_ms, packet_data, packetlen, input_info) != 0)
      return(ret);

    goto ret_protocols;
  }

  if(ndpi_init_packet(ndpi_str, flow, current_time_ms, packet_data, packetlen, input_info) != 0)
    return(ret);

  if(flow->num_processed_pkts == 1) {
    /* first packet of this flow to be analyzed */

#ifdef HAVE_NBPF
    if(ndpi_str->nbpf_custom_proto[0].tree != NULL) {
      u_int8_t i;
      nbpf_pkt_info_t t;

      memset(&t, 0, sizeof(t));

      if(packet->iphv6 != NULL) {
	t.tuple.eth_type = 0x86DD;
	t.tuple.ip_version = 6;
	memcpy(&t.tuple.ip_src.v6, &packet->iphv6->ip6_src, 16);
	memcpy(&t.tuple.ip_dst.v6, &packet->iphv6->ip6_dst, 16);
      } else {
	t.tuple.eth_type = 0x0800;
	t.tuple.ip_version = 4;
	t.tuple.ip_src.v4 = packet->iph->saddr;
	t.tuple.ip_dst.v4 = packet->iph->daddr;
      }

      t.tuple.l3_proto = flow->l4_proto;

      if(packet->tcp)
	t.tuple.l4_src_port = packet->tcp->source, t.tuple.l4_dst_port = packet->tcp->dest;
      else if(packet->udp)
	t.tuple.l4_src_port = packet->udp->source, t.tuple.l4_dst_port = packet->udp->dest;

      for(i=0; (i<MAX_NBPF_CUSTOM_PROTO) && (ndpi_str->nbpf_custom_proto[i].tree != NULL); i++) {
	if(nbpf_match(ndpi_str->nbpf_custom_proto[i].tree, &t)) {
	  /* match found */
	  ret.proto.master_protocol = ret.proto.app_protocol = ndpi_str->nbpf_custom_proto[i].l7_protocol;
	  ndpi_fill_protocol_category(ndpi_str, flow, &ret);
	  ndpi_reconcile_protocols(ndpi_str, flow, &ret);
	  flow->confidence = NDPI_CONFIDENCE_NBPF;

	  return(ret);
	}
      }
    }
#endif
  }

  ndpi_connection_tracking(ndpi_str, flow);

  /* build ndpi_selection packet bitmask */
  ndpi_selection_packet = NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC;
  if(packet->iph != NULL)
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_IP | NDPI_SELECTION_BITMASK_PROTOCOL_IPV4_OR_IPV6;

  if(packet->tcp != NULL)
    ndpi_selection_packet |=
      (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP | NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP);

  if(packet->udp != NULL)
    ndpi_selection_packet |=
      (NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP | NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP);

  if(packet->payload_packet_len != 0) {
    uint8_t *pcnt = &flow->num_processed_packets[flow->packet_direction & 1];
    if(*pcnt != 0xff) (*pcnt)++;
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_HAS_PAYLOAD;
  }

  if(packet->tcp_retransmission == 0)
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_NO_TCP_RETRANSMISSION;

  if(packet->iphv6 != NULL)
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_IPV6 | NDPI_SELECTION_BITMASK_PROTOCOL_IPV4_OR_IPV6;

  if(!flow->protocol_id_already_guessed) {
    flow->protocol_id_already_guessed = 1;

    if(ndpi_do_guess(ndpi_str, flow, &ret) == -1) {

      /* Reason: custom rules */
      internal_giveup(ndpi_str, flow, &ret);

      return(ret);
    }
  }

  num_calls = ndpi_check_flow_func(ndpi_str, flow, &ndpi_selection_packet);

 ret_protocols:
  if(flow->detected_protocol_stack[1] != NDPI_PROTOCOL_UNKNOWN) {
    ret.proto.master_protocol = flow->detected_protocol_stack[1], ret.proto.app_protocol = flow->detected_protocol_stack[0];

    if(ret.proto.app_protocol == ret.proto.master_protocol)
      ret.proto.master_protocol = NDPI_PROTOCOL_UNKNOWN;
  } else
    ret.proto.app_protocol = flow->detected_protocol_stack[0];

#ifndef __KERNEL__
  /* Don't overwrite the category if already set */
  if((flow->category == NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) && (ret.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN))
    ndpi_fill_protocol_category(ndpi_str, flow, &ret);
  else
    ret.category = flow->category;
#endif

  if((!flow->risk_checked)
     && ((ret.proto.master_protocol != NDPI_PROTOCOL_UNKNOWN) || (ret.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN))
     ) {
    default_ports_tree_node_t *found;
    ndpi_port_range *default_ports;

    if(packet->udp)
      found = ndpi_get_guessed_protocol_id(ndpi_str, IPPROTO_UDP,
					   ntohs(flow->c_port),
					   ntohs(flow->s_port)),
	default_ports = ndpi_str->proto_defaults[ret.proto.master_protocol ? ret.proto.master_protocol : ret.proto.app_protocol].udp_default_ports;
    else if(packet->tcp)
      found = ndpi_get_guessed_protocol_id(ndpi_str, IPPROTO_TCP,
					   ntohs(flow->c_port),
					   ntohs(flow->s_port)),
	default_ports = ndpi_str->proto_defaults[ret.proto.master_protocol ? ret.proto.master_protocol : ret.proto.app_protocol].tcp_default_ports;
    else
      found = NULL, default_ports = NULL;

    if(found
       && (found->proto->protoId != NDPI_PROTOCOL_UNKNOWN)
       && (found->proto->protoId != ret.proto.master_protocol)
       && (found->proto->protoId != ret.proto.app_protocol)
       ) {
      // printf("******** %u / %u\n", found->proto->protoId, ret.proto.master_protocol);

      if(!ndpi_check_protocol_port_mismatch_exceptions(found, &ret)) {
	/*
	  Before triggering the alert we need to make some extra checks
	  - the protocol found is not running on the port we have found
	  (i.e. two or more protools share the same default port)
	*/
	u_int8_t found = 0, i;

	for(i=0; (i<MAX_DEFAULT_PORTS) && (default_ports[i].port_low != 0); i++) {
	  if(default_ports[i].port_low >= ntohs(flow->s_port) &&
	     default_ports[i].port_high <= ntohs(flow->s_port)) {
	    found = 1;
	    break;
	  }
	} /* for */

	if(!found) {
	  default_ports_tree_node_t *r = ndpi_get_guessed_protocol_id(ndpi_str, packet->udp ? IPPROTO_UDP : IPPROTO_TCP,
								      ntohs(flow->c_port), ntohs(flow->s_port));

	  if((r == NULL)
	     || ((r->proto->protoId != ret.proto.app_protocol) && (r->proto->protoId != ret.proto.master_protocol))) {
	    if(default_ports && (default_ports[0].port_low != 0)) {
	      char str[64];

	      ndpi_set_risk(ndpi_str, flow, NDPI_KNOWN_PROTOCOL_ON_NON_STANDARD_PORT,
			    ndpi_expected_ports_str(default_ports, str, sizeof(str)));
	    }
	  }
	}
      }
    } else if((!ndpi_is_ntop_protocol(&ret)) && default_ports && (default_ports[0].port_low != 0)) {
      u_int8_t found = 0, i, num_loops = 0;

    check_default_ports:
      for(i=0; (i<MAX_DEFAULT_PORTS) && (default_ports[i].port_low != 0); i++) {
	if((default_ports[i].port_low >= ntohs(flow->c_port) &&
            default_ports[i].port_high <= ntohs(flow->c_port)) ||
           (default_ports[i].port_low >= ntohs(flow->s_port) &&
            default_ports[i].port_high <= ntohs(flow->s_port))) {
	  found = 1;
	  break;
	}
      } /* for */

      if((num_loops == 0) && (!found)) {
	if(packet->udp)
	  default_ports = ndpi_str->proto_defaults[ret.proto.app_protocol].udp_default_ports;
	else
	  default_ports = ndpi_str->proto_defaults[ret.proto.app_protocol].tcp_default_ports;

	num_loops = 1;
	goto check_default_ports;
      }

      if(!found) {
	default_ports_tree_node_t *r = ndpi_get_guessed_protocol_id(ndpi_str, packet->udp ? IPPROTO_UDP : IPPROTO_TCP,
								    ntohs(flow->c_port), ntohs(flow->s_port));

	if((r == NULL)
	   || ((r->proto->protoId != ret.proto.app_protocol)
	       && (r->proto->protoId != ret.proto.master_protocol))) {
	  if(ret.proto.app_protocol != NDPI_PROTOCOL_FTP_DATA) {
	    ndpi_port_range *default_ports;

	    if(packet->udp)
	      default_ports = ndpi_str->proto_defaults[ret.proto.master_protocol ? ret.proto.master_protocol : ret.proto.app_protocol].udp_default_ports;
	    else if(packet->tcp)
	      default_ports = ndpi_str->proto_defaults[ret.proto.master_protocol ? ret.proto.master_protocol : ret.proto.app_protocol].tcp_default_ports;
	    else
	      default_ports = NULL;

	    if(default_ports && (default_ports[0].port_low != 0)) {
	      char str[64];

	      ndpi_set_risk(ndpi_str, flow, NDPI_KNOWN_PROTOCOL_ON_NON_STANDARD_PORT,
			    ndpi_expected_ports_str(default_ports, str, sizeof(str)));
	    }
	  }
	}
      }
    }

    flow->risk_checked = 1;
  }

  if(!flow->tree_risk_checked) {
    ndpi_risk_enum net_risk = NDPI_NO_RISK;

    /* Right now, all the 3 supported risks are only about the *client* ip.
       Don't check the server ip, to try avoiding false positives */

    if(ndpi_str->ip_risk && ndpi_str->ip_risk->v4
       && packet->iph
       && ndpi_is_public_ipv4(ntohl(packet->iph->saddr))
       && ndpi_is_public_ipv4(ntohl(packet->iph->daddr))) {
      struct in_addr addr;

      addr.s_addr = flow->c_address.v4;
      net_risk = ndpi_network_risk_ptree_match(ndpi_str, &addr);
    } else if(ndpi_str->ip_risk && ndpi_str->ip_risk->v6 &&
              packet->iphv6) { /* TODO: some checks on "local" addresses? */
      struct in6_addr addr;

      addr = *(struct in6_addr *)&flow->c_address.v6;
      net_risk = ndpi_network_risk_ptree_match6(ndpi_str, &addr);
    }

    if(net_risk != NDPI_NO_RISK)
      ndpi_set_risk(ndpi_str, flow, net_risk, NULL);

    flow->tree_risk_checked = 1;
  }

  /* It is common to don't trigger any dissectors for pure TCP ACKs
     and for for retransmissions */
  if(num_calls == 0 &&
     (packet->tcp_retransmission == 0 && packet->payload_packet_len != 0))
    flow->fail_with_unknown = 1;
  flow->num_dissector_calls += num_calls;
#if 0
   NDPI_LOG(flow ? flow->detected_protocol_stack[0] : NDPI_PROTOCOL_UNKNOWN, ndpi_str, NDPI_LOG_TRACE,
             "[%d/%d] dissector_calls %d\n",
	     flow->detected_protocol_stack[0], flow->detected_protocol_stack[1],flow->num_dissector_calls
	     );
#endif
  /* ndpi_reconcile_protocols(ndpi_str, flow, &ret); */

  if(ndpi_str->cfg.fully_encrypted_heuristic &&
     ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN && /* Only for unknown traffic */
     flow->packet_counter == 1 && packet->payload_packet_len > 0) {
    flow->first_pkt_fully_encrypted = fully_enc_heuristic(ndpi_str, flow);
  }

  if((ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN)
     && (packet->payload_packet_len > 0)
     && (flow->packet_counter <= 5)) {
    ndpi_search_portable_executable(ndpi_str, flow);
    ndpi_search_elf(ndpi_str, flow);
    ndpi_search_shellscript(ndpi_str, flow);
  }

#ifndef __KERNEL__
  if(ndpi_str->cfg.compute_entropy &&
     flow->skip_entropy_check == 0 &&
     flow->first_pkt_fully_encrypted == 0 &&
     flow->packet_counter < 5 &&
     /* The following protocols do their own entropy calculation/classification. */
     ret.proto.app_protocol != NDPI_PROTOCOL_IP_ICMP) {

    if (ret.proto.app_protocol != NDPI_PROTOCOL_HTTP &&
        ret.proto.master_protocol != NDPI_PROTOCOL_HTTP) {
      flow->entropy = ndpi_entropy(packet->payload, packet->payload_packet_len);
    }

    ndpi_entropy2risk(ndpi_str, flow);
  }
#endif

  /* First Packet Classification */
  if(flow->all_packets_counter == 1)
    fpc_check_eval(ndpi_str, flow);

  if(ret.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN &&
     flow->extra_packets_func == NULL) {
    /* Reason: "normal" classification, without extra dissection */
    internal_giveup(ndpi_str, flow, &ret);
  }

  return(ret);
}

/* ********************************************************************************* */

ndpi_protocol ndpi_detection_process_packet(struct ndpi_detection_module_struct *ndpi_str,
					    struct ndpi_flow_struct *flow, const unsigned char *packet_data,
					    const unsigned short packetlen, const u_int64_t current_time_ms,
					    struct ndpi_flow_input_info *input_info) {
  ndpi_protocol p  = ndpi_internal_detection_process_packet(ndpi_str, flow, packet_data,
							    packetlen, current_time_ms,
							    input_info);

  p.proto.master_protocol = ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, p.proto.master_protocol);
  p.proto.app_protocol = ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, p.proto.app_protocol);
  p.protocol_by_ip = ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, p.protocol_by_ip);

  return(p);
}

/* ********************************************************************************* */

u_int32_t ndpi_bytestream_to_number(const u_int8_t *str, u_int16_t max_chars_to_read, u_int16_t *bytes_read) {
  u_int32_t val;
  val = 0;

  // cancel if eof, ' ' or line end chars are reached
  while(max_chars_to_read > 0 && *str >= '0' && *str <= '9') {
    val *= 10;
    val += *str - '0';
    str++;
    max_chars_to_read = max_chars_to_read - 1;
    *bytes_read = *bytes_read + 1;
  }

  return(val);
}

/* ********************************************************************************* */

u_int64_t ndpi_bytestream_to_number64(const u_int8_t *str, u_int16_t max_chars_to_read, u_int16_t *bytes_read) {
  u_int64_t val;
  val = 0;
  // cancel if eof, ' ' or line end chars are reached
  while(max_chars_to_read > 0 && *str >= '0' && *str <= '9') {
    val *= 10;
    val += *str - '0';
    str++;
    max_chars_to_read = max_chars_to_read - 1;
    *bytes_read = *bytes_read + 1;
  }
  return(val);
}

/* ********************************************************************************* */

u_int64_t ndpi_bytestream_dec_or_hex_to_number64(const u_int8_t *str, u_int16_t max_chars_to_read,
						 u_int16_t *bytes_read) {
  u_int64_t val;
  val = 0;
  if(max_chars_to_read <= 2 || str[0] != '0' || str[1] != 'x') {
    return(ndpi_bytestream_to_number64(str, max_chars_to_read, bytes_read));
  } else {
    /*use base 16 system */
    str += 2;
    max_chars_to_read -= 2;
    *bytes_read = *bytes_read + 2;
    while(max_chars_to_read > 0) {
      if(*str >= '0' && *str <= '9') {
	val *= 16;
	val += *str - '0';
      } else if(*str >= 'a' && *str <= 'f') {
	val *= 16;
	val += *str + 10 - 'a';
      } else if(*str >= 'A' && *str <= 'F') {
	val *= 16;
	val += *str + 10 - 'A';
      } else {
	break;
      }
      str++;
      max_chars_to_read = max_chars_to_read - 1;
      *bytes_read = *bytes_read + 1;
    }
  }
  return(val);
}

/* ********************************************************************************* */

u_int32_t ndpi_bytestream_to_ipv4(const u_int8_t *str, u_int16_t max_chars_to_read, u_int16_t *bytes_read) {
  u_int32_t val;
  u_int16_t read = 0;
  u_int16_t oldread;
  u_int32_t c;

  /* ip address must be X.X.X.X with each X between 0 and 255 */
  oldread = read;
  c = ndpi_bytestream_to_number(str, max_chars_to_read, &read);
  if(c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
    return(0);

  read++;
  val = c << 24;
  oldread = read;
  c = ndpi_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
  if(c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
    return(0);

  read++;
  val = val + (c << 16);
  oldread = read;
  c = ndpi_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
  if(c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
    return(0);

  read++;
  val = val + (c << 8);
  oldread = read;
  c = ndpi_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
  if(c > 255 || oldread == read || max_chars_to_read == read)
    return(0);

  val = val + c;

  *bytes_read = *bytes_read + read;

  return(htonl(val));
}

/* ********************************************************************************* */

struct header_line {
  char *name;
  struct ndpi_int_one_line_struct *line;
  int	idx;
};

static void parse_single_packet_line(struct ndpi_detection_module_struct *ndpi_str)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  struct ndpi_int_one_line_struct *line;
  size_t length;
  struct header_line *hs = NULL;
  struct header_line *h;
  /* Some bogus response doesn't have the space after ":". Skip leading spaces later...  */
  struct header_line headers_a[] = { { "Accept:", &packet->accept_line },
                                     { "Authorization:", &packet->authorization_line },
                                     { NULL, NULL} };
  struct header_line headers_b[] = { { "BOOTID.UPNP.ORG:", &packet->bootid},
                                     { NULL, NULL} };
  struct header_line headers_u[] = { { "User-agent:", &packet->user_agent_line },
                                     { "Upgrade:", &packet->upgrade_line },
                                     { "USN:", &packet->usn },
                                     { NULL, NULL} };
  struct header_line headers_c[] = { { "Content-Disposition:", &packet->content_disposition_line },
                                     { "Content-type:", &packet->content_line },
                                     { "CACHE-CONTROL:", &packet->cache_controle},
                                     { NULL, NULL} };
  struct header_line headers_o[] = { { "Origin:", &packet->http_origin },
                                     { NULL, NULL} };
  struct header_line headers_h[] = { { "Host:", &packet->host_line },
                                     { "HOUSEHOLD.SMARTSPEAKER.AUDIO:", &packet->household_smart_speaker_audio },
                                     { NULL, NULL} };
  struct header_line headers_x[] = { { "X-Forwarded-For:", &packet->forwarded_line },
                                     { "X-RINCON-HOUSEHOLD:", &packet->rincon_household },
                                     { "X-RINCON-BOOTSEQ:", &packet->rincon_bootseq },
                                     { "X-RINCON-WIFIMODE:", &packet->rincon_wifimode },
                                     { "X-RINCON-VARIANT:", &packet->rincon_variant },
                                     { "X-SONOS-HHSECURELOCATION:", &packet->sonos_securelocation },
                                     { NULL, NULL} };
  struct header_line headers_r[] = { { "Referer:", &packet->referer_line },
                                     { NULL, NULL} };
  struct header_line headers_s[] = { { "Server:", &packet->server_line },
                                     { "SECURELOCATION.UPNP.ORG:", &packet->securelocation_upnp },
                                     { "ST", &packet->st },
                                     { NULL, NULL} };
  struct header_line headers_l[] = { { "LOCATION:", &packet->location },
                                     { "LOCATION.SMARTSPEAKER.AUDIO:", &packet->location_smart_speaker_audio },
                                     { NULL, NULL}};
  struct header_line headers_m[] = { { "MAN:", &packet->man },
                                     { "MX:", &packet->mx },
                                     { NULL, NULL}};
  struct header_line headers_n[] = { { "NT:", &packet->nt },
                                     { "NTS:", &packet->nts },
                                     { NULL, NULL}};

  line = &packet->line[packet->parsed_lines];
  if(line->len == 0)
    return;

  /* First line of a HTTP response parsing. Expected a "HTTP/1.? ???" */
  if(packet->parsed_lines == 0 && line->len >= NDPI_STATICSTRING_LEN("HTTP/1.X 200 ") &&
     strncasecmp((const char *)line->ptr, "HTTP/1.", NDPI_STATICSTRING_LEN("HTTP/1.")) == 0 &&
     line->ptr[NDPI_STATICSTRING_LEN("HTTP/1.X ")] > '0' && /* response code between 000 and 699 */
     line->ptr[NDPI_STATICSTRING_LEN("HTTP/1.X ")] < '6') {
    packet->http_response.ptr = &line->ptr[NDPI_STATICSTRING_LEN("HTTP/1.1 ")];
    packet->http_response.len = line->len - NDPI_STATICSTRING_LEN("HTTP/1.1 ");
    return;
  }
  if(packet->parsed_lines == 0 && line->len > 0) {
    /*
      Check if the file contains a : otherwise ignore the line as this
      line i slike "GET /....
    */
    if(memchr((char *)line->ptr, ':', line->len) == NULL)
      return;
  }

  switch(line->ptr[0]) {
  case 'a':
  case 'A':
    hs = headers_a;
    break;
  case 'b':
  case 'B':
    hs = headers_b;
    break;
  case 'c':
  case 'C':
    hs = headers_c;
    break;
  case 'h':
  case 'H':
    hs = headers_h;
    break;
  case 'o':
  case 'O':
    hs = headers_o;
    break;
  case 'r':
  case 'R':
    hs = headers_r;
    break;
  case 's':
  case 'S':
    hs = headers_s;
    break;
  case 'u':
  case 'U':
    hs = headers_u;
    break;
  case 'x':
  case 'X':
    hs = headers_x;
    break;
  case 'l':
  case 'L':
    hs = headers_l;
    break;
  case 'm':
  case 'M':
      hs = headers_m;
      break;
  case 'n':
  case 'N':
    hs = headers_n;
    break;
  default:
    return;
  }

  for(h = &hs[0]; h->name; h++) {
    length = strlen(h->name);
    if(line->len > length &&
       strncasecmp((const char *)line->ptr, h->name, length) == 0) {
      h->line->ptr = &line->ptr[length];
      h->line->len = line->len - length;

      /* Stripping leading spaces */
      while(h->line->len > 0 && h->line->ptr[0] == ' ') {
        h->line->len--;
        h->line->ptr++;
      }
      if(h->line->len == 0)
        h->line->ptr = NULL;

      /* Stripping trailing spaces */
      while(h->line->len > 0 && h->line->ptr[h->line->len - 1] == ' ') {
        h->line->len--;
      }
      if(h->line->len == 0)
        h->line->ptr = NULL;

      break;
    }
  }

  if(packet->content_line.len > 0) {
    /* application/json; charset=utf-8 */
    char separator[] = {';', '\r', '\0'};
    int i;

    for(i = 0; separator[i] != '\0'; i++) {
      char *c = memchr((char *) packet->content_line.ptr, separator[i], packet->content_line.len);

      if(c != NULL)
	packet->content_line.len = c - (char *) packet->content_line.ptr;
    }
  }
}

/* ********************************************************************************* */

/* internal function for every detection to parse one packet and to increase the info buffer */
void ndpi_parse_packet_line_info(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow) {
  u_int32_t a;
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);

  if((packet->payload_packet_len < 3) || (packet->payload == NULL))
    return;

  if(packet->packet_lines_parsed_complete != 0)
    return;

  packet->packet_lines_parsed_complete = 1;
  ndpi_reset_packet_line_info(packet);

  packet->line[packet->parsed_lines].ptr = packet->payload;
  packet->line[packet->parsed_lines].len = 0;

  for(a = 0; ((a+1) < packet->payload_packet_len) && (packet->parsed_lines < NDPI_MAX_PARSE_LINES_PER_PACKET); a++) {
    if((packet->payload[a] == 0x0d) && (packet->payload[a+1] == 0x0a)) {
      /* If end of line char sequence CR+NL "\r\n", process line */

      flow->http.request_header_observed = 1;

      if(((a + 3) < packet->payload_packet_len)
	 && (packet->payload[a+2] == 0x0d)
	 && (packet->payload[a+3] == 0x0a)) {
	/* \r\n\r\n */
	int diff; /* No unsigned ! */
	u_int32_t a1 = a + 4;

	diff = packet->payload_packet_len - a1;

	if(diff > 0) {
	  diff = ndpi_min((unsigned int)diff, sizeof(flow->initial_binary_bytes));
	  memcpy(&flow->initial_binary_bytes, &packet->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}
      }

      packet->line[packet->parsed_lines].len =
	(u_int16_t)(((size_t) &packet->payload[a]) - ((size_t) packet->line[packet->parsed_lines].ptr));

      parse_single_packet_line(ndpi_str);

      if(packet->line[packet->parsed_lines].len == 0) {
	packet->empty_line_position = a;
	packet->empty_line_position_set = 1;
      }

      if(packet->parsed_lines >= (NDPI_MAX_PARSE_LINES_PER_PACKET - 1))
	return;

      packet->parsed_lines++;
      packet->line[packet->parsed_lines].ptr = &packet->payload[a + 2];
      packet->line[packet->parsed_lines].len = 0;

      a++; /* next char in the payload */
    }
  }

  if(packet->parsed_lines >= 1) {
    packet->line[packet->parsed_lines].len =
      (u_int16_t)(((size_t) &packet->payload[packet->payload_packet_len]) -
		  ((size_t) packet->line[packet->parsed_lines].ptr));

    parse_single_packet_line(ndpi_str);
    packet->parsed_lines++;
  }
}

/* ********************************************************************************* */

NDPI_STATIC void ndpi_parse_packet_line_info_any(struct ndpi_detection_module_struct *ndpi_str) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_str);
  u_int32_t a;
  u_int16_t end = packet->payload_packet_len;

  if(packet->packet_lines_parsed_complete != 0)
    return;

  packet->hdr_line = &packet->host_line;
  packet->packet_lines_parsed_complete = 1;
  packet->parsed_lines = 0;

  if(packet->payload_packet_len == 0)
    return;

  packet->line[packet->parsed_lines].ptr = packet->payload;
  packet->line[packet->parsed_lines].len = 0;

  for(a = 0; a < end; a++) {
    if(packet->payload[a] == 0x0a) {
      packet->line[packet->parsed_lines].len = (u_int16_t)(((size_t) &packet->payload[a]) - ((size_t) packet->line[packet->parsed_lines].ptr));

      if(a > 0 && packet->payload[a - 1] == 0x0d)
	packet->line[packet->parsed_lines].len--;

      if(packet->parsed_lines >= (NDPI_MAX_PARSE_LINES_PER_PACKET - 1))
	break;

      packet->parsed_lines++;
      packet->line[packet->parsed_lines].ptr = &packet->payload[a + 1];
      packet->line[packet->parsed_lines].len = 0;

      if((a + 1) >= packet->payload_packet_len)
	break;

      //a++;
    }
  }
}

/* ********************************************************************************* */

u_int8_t ndpi_detection_get_l4(const u_int8_t *l3, u_int16_t l3_len, const u_int8_t **l4_return,
			       u_int16_t *l4_len_return, u_int8_t *l4_protocol_return, u_int32_t flags) {
  return(ndpi_detection_get_l4_internal(NULL, l3, l3_len, l4_return, l4_len_return, l4_protocol_return, flags));
}

/* ********************************************************************************* */

void ndpi_set_detected_protocol_keeping_master(struct ndpi_detection_module_struct *ndpi_str,
					       struct ndpi_flow_struct *flow,
					       u_int16_t detected_protocol,
					       ndpi_confidence_t confidence) {
  u_int16_t master;

  master = flow->detected_protocol_stack[1] ? flow->detected_protocol_stack[1] : flow->detected_protocol_stack[0];

  if (master != NDPI_PROTOCOL_UNKNOWN)
    ndpi_set_detected_protocol(ndpi_str, flow, detected_protocol, master, confidence);
  else
    ndpi_set_detected_protocol(ndpi_str, flow, NDPI_PROTOCOL_UNKNOWN, detected_protocol, confidence);
}

/* ********************************************************************************* */

void ndpi_set_detected_protocol(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow,
				u_int16_t upper_detected_protocol, u_int16_t lower_detected_protocol,
				ndpi_confidence_t confidence) {
  ndpi_protocol ret;

  if(flow->monitoring) {
    NDPI_LOG_ERR(ndpi_str, "Impossible to update classification while in monitoring state! %d/%d->%d/%d\n",
                 flow->detected_protocol_stack[1], flow->detected_protocol_stack[0],
                 upper_detected_protocol, lower_detected_protocol);
    return;
  }

  ndpi_int_change_protocol(flow, upper_detected_protocol, lower_detected_protocol, confidence);
  ret.proto.master_protocol = flow->detected_protocol_stack[1], ret.proto.app_protocol = flow->detected_protocol_stack[0];
  ndpi_reconcile_protocols(ndpi_str, flow, &ret);
}

/* ********************************************************************************* */

void reset_detected_protocol(struct ndpi_flow_struct *flow) {
  flow->detected_protocol_stack[1] = NDPI_PROTOCOL_UNKNOWN;
  flow->detected_protocol_stack[0] = NDPI_PROTOCOL_UNKNOWN;
  flow->confidence = NDPI_CONFIDENCE_UNKNOWN;
}

/* ********************************************************************************* */

u_int16_t ndpi_get_flow_masterprotocol(struct ndpi_flow_struct *flow) {
  return(flow->detected_protocol_stack[1]);
}

/* ********************************************************************************* */

u_int16_t ndpi_get_flow_appprotocol(struct ndpi_flow_struct *flow) {
  return(flow->detected_protocol_stack[0]);
}

/* ********************************************************************************* */

ndpi_protocol_category_t ndpi_get_flow_category(struct ndpi_flow_struct *flow)
{
  return(flow->category);
}

/* ********************************************************************************* */

void ndpi_get_flow_ndpi_proto(struct ndpi_flow_struct *flow,
			      struct ndpi_proto * ndpi_proto)
{
  ndpi_proto->proto.master_protocol = ndpi_get_flow_masterprotocol(flow);
  ndpi_proto->proto.app_protocol = ndpi_get_flow_appprotocol(flow);
#ifndef __KERNEL__
  ndpi_proto->category = ndpi_get_flow_category(flow);
#endif
}

/* ********************************************************************************* */

static void ndpi_int_change_flow_protocol(struct ndpi_flow_struct *flow,
					  u_int16_t upper_detected_protocol, u_int16_t lower_detected_protocol,
					  ndpi_confidence_t confidence) {
  flow->detected_protocol_stack[0] = upper_detected_protocol,
    flow->detected_protocol_stack[1] = lower_detected_protocol;
  flow->confidence = confidence;
}

/* ********************************************************************************* */

/* generic function for changing the protocol
 *
 * what it does is:
 * 1.update the flow protocol stack with the new protocol
 */
static void ndpi_int_change_protocol(struct ndpi_flow_struct *flow,
				     u_int16_t upper_detected_protocol, u_int16_t lower_detected_protocol,
				     ndpi_confidence_t confidence) {
  if((upper_detected_protocol == NDPI_PROTOCOL_UNKNOWN) && (lower_detected_protocol != NDPI_PROTOCOL_UNKNOWN))
    upper_detected_protocol = lower_detected_protocol;

  if(upper_detected_protocol == lower_detected_protocol)
    lower_detected_protocol = NDPI_PROTOCOL_UNKNOWN;

  ndpi_int_change_flow_protocol(flow, upper_detected_protocol, lower_detected_protocol, confidence);
}

/* ********************************************************************************* */

void change_category(struct ndpi_flow_struct *flow,
		     ndpi_protocol_category_t protocol_category) {
  flow->category = protocol_category;
}

/* ********************************************************************************* */

u_int8_t ndpi_is_ipv6(const ndpi_ip_addr_t *ip) {
  return(ip->ipv6.u6_addr.u6_addr32[1] != 0 || ip->ipv6.u6_addr.u6_addr32[2] != 0 ||
	 ip->ipv6.u6_addr.u6_addr32[3] != 0);
}

/* ********************************************************************************* */

char *ndpi_get_ip_string(const ndpi_ip_addr_t *ip, char *buf, u_int buf_len) {
  const u_int8_t *a = (const u_int8_t *) &ip->ipv4;

  if(ndpi_is_ipv6(ip)) {
    struct in6_addr addr = *(struct in6_addr *)&ip->ipv6.u6_addr;

    if(inet_ntop(AF_INET6, &addr, buf, buf_len) == NULL)
      buf[0] = '\0';

    return(buf);
  }

  ndpi_snprintf(buf, buf_len, "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);

  return(buf);
}

/* ****************************************************** */

/* Returns -1 on failutre, otherwise fills parsed_ip and returns the IP version */
int ndpi_parse_ip_string(const char *ip_str, ndpi_ip_addr_t *parsed_ip) {
  int rv = -1;
  memset(parsed_ip, 0, sizeof(*parsed_ip));

  if(strchr(ip_str, '.')) {
    if(inet_pton(AF_INET, ip_str, &parsed_ip->ipv4) > 0)
      rv = 4;
  } else {
    if(inet_pton(AF_INET6, ip_str, &parsed_ip->ipv6) > 0)
      rv = 6;
  }

  return(rv);
}

/* ****************************************************** */

u_int16_t ntohs_ndpi_bytestream_to_number(const u_int8_t *str,
					  u_int16_t max_chars_to_read, u_int16_t *bytes_read) {
  u_int16_t val = ndpi_bytestream_to_number(str, max_chars_to_read, bytes_read);
  return(ntohs(val));
}

/* ****************************************************** */

bool ndpi_is_proto(ndpi_master_app_protocol proto, u_int16_t p) {
  return(((proto.app_protocol == p) || (proto.master_protocol == p)) ? true : false);
}

/* ****************************************************** */

bool ndpi_is_proto_unknown(ndpi_master_app_protocol proto) {
  return(((proto.app_protocol == NDPI_PROTOCOL_UNKNOWN)
	  && (proto.master_protocol == NDPI_PROTOCOL_UNKNOWN)) ? true : false);
}

/* ****************************************************** */

bool ndpi_is_proto_equals(ndpi_master_app_protocol to_check,
			  ndpi_master_app_protocol to_match, bool exact_match_only) {

  if(exact_match_only) {
    return((memcmp(&to_check, &to_match, sizeof(to_match)) == 0) ? true : false);
  } else {
    if(to_match.master_protocol != NDPI_PROTOCOL_UNKNOWN) {
      if(ndpi_is_proto(to_check, to_match.master_protocol))
	return(true);
    }

    if(to_match.app_protocol != NDPI_PROTOCOL_UNKNOWN) {
      if(ndpi_is_proto(to_check, to_match.app_protocol))
	return(true);
    }

    return(false);
  }
}

/* ****************************************************** */

u_int16_t ndpi_get_lower_proto(ndpi_protocol proto) {
  return((proto.proto.master_protocol != NDPI_PROTOCOL_UNKNOWN) ? proto.proto.master_protocol : proto.proto.app_protocol);
}

/* ****************************************************** */

u_int16_t ndpi_get_upper_proto(ndpi_protocol proto) {
  return((proto.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN) ? proto.proto.app_protocol : proto.proto.master_protocol);
}

/* ****************************************************** */

static ndpi_protocol ndpi_internal_guess_undetected_protocol(struct ndpi_detection_module_struct *ndpi_str,
							     struct ndpi_flow_struct *flow, u_int8_t proto) {
  ndpi_protocol ret = NDPI_PROTOCOL_NULL;
  u_int8_t user_defined_proto;

  if(!ndpi_str)
    return(ret);

#ifdef BITTORRENT_CACHE_DEBUG
  printf("[%s:%u] [flow: %p] proto %u\n", __FILE__, __LINE__, flow, proto);
#endif

  if(flow && ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP))) {
    if(flow->guessed_protocol_id != NDPI_PROTOCOL_UNKNOWN) {
      if(flow->guessed_protocol_id_by_ip != NDPI_PROTOCOL_UNKNOWN) {
        ret.proto.master_protocol = flow->guessed_protocol_id;
        ret.proto.app_protocol = flow->guessed_protocol_id_by_ip;
      } else {
        ret.proto.app_protocol = flow->guessed_protocol_id;
      }
    } else {
      ret.proto.app_protocol = flow->guessed_protocol_id_by_ip;
    }

    if(ret.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
       search_into_bittorrent_cache(ndpi_str, flow)) {
      /* This looks like BitTorrent */
      ret.proto.app_protocol = NDPI_PROTOCOL_BITTORRENT;
    }
  } else {
    ret.proto.app_protocol = guess_protocol_id(ndpi_str, proto, 0, 0, &user_defined_proto);
  }

#ifndef __KERNEL__
  ret.category = ndpi_get_proto_category(ndpi_str, ret);
#endif

#ifdef BITTORRENT_CACHE_DEBUG
  printf("[%s:%u] Guessed %u.%u\n", __FILE__, __LINE__, ret.proto.master_protocol, ret.proto.app_protocol);
#endif

  return(ret);
}

/* ****************************************************** */

/* Extension of ndpi_guess_undetected_protocol() with IPv4 matching for host and protocol */

ndpi_protocol ndpi_guess_undetected_protocol_v4(struct ndpi_detection_module_struct *ndpi_str,
						struct ndpi_flow_struct *flow, u_int8_t proto,
						u_int32_t shost /* host byte order */, u_int16_t sport,
						u_int32_t dhost /* host byte order */, u_int16_t dport) {
  u_int32_t rc = NDPI_PROTOCOL_UNKNOWN;
  ndpi_protocol ret = NDPI_PROTOCOL_NULL;
  u_int8_t user_defined_proto;

  if(!ndpi_str)
    return ret;

  if((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP)) {
    if(shost && dhost) {
      struct in_addr addr;
      u_int16_t rcode = NDPI_PROTOCOL_UNKNOWN;

      /* guess host protocol; server first */
      addr.s_addr = htonl(shost);
      rcode = ndpi_network_port_ptree_match(ndpi_str, &addr, htons(sport));

      if(rcode == NDPI_PROTOCOL_UNKNOWN) {
	addr.s_addr = htonl(dhost);
	rcode = ndpi_network_port_ptree_match(ndpi_str, &addr, htons(dport));
      }

      if(rcode == NDPI_PROTOCOL_UNKNOWN)
	rc = ndpi_search_tcp_or_udp_raw(ndpi_str, flow, shost, dhost);
      else
	rc = (u_int32_t)rcode;
    } else
      rc = NDPI_PROTOCOL_UNKNOWN;

    if(rc != NDPI_PROTOCOL_UNKNOWN) {
      ret.proto.app_protocol = rc,
	ret.proto.master_protocol = guess_protocol_id(ndpi_str, proto, sport, dport, &user_defined_proto);

      if(ret.proto.app_protocol == ret.proto.master_protocol)
	ret.proto.master_protocol = NDPI_PROTOCOL_UNKNOWN;
    } else {
      ret.proto.app_protocol = guess_protocol_id(ndpi_str, proto, sport, dport, &user_defined_proto),
	ret.proto.master_protocol = NDPI_PROTOCOL_UNKNOWN;
    }

    if(ret.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN) {
#ifndef __KERNEL__
      ret.category = ndpi_get_proto_category(ndpi_str, ret);
#endif
      return(ret);
    }
  }

  return(ndpi_guess_undetected_protocol(ndpi_str, flow, proto));
}

/* ****************************************************** */

ndpi_protocol ndpi_guess_undetected_protocol(struct ndpi_detection_module_struct *ndpi_str,
					     struct ndpi_flow_struct *flow, u_int8_t proto) {
  ndpi_protocol p = ndpi_internal_guess_undetected_protocol(ndpi_str, flow, proto);

  p.proto.master_protocol = ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, p.proto.master_protocol),
    p.proto.app_protocol = ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, p.proto.app_protocol);

  return(p);
}

/* ****************************************************** */

char *ndpi_protocol2id(ndpi_protocol proto, char *buf, u_int buf_len) {
  if((proto.proto.master_protocol != NDPI_PROTOCOL_UNKNOWN) && (proto.proto.master_protocol != proto.proto.app_protocol)) {
    if(proto.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN)
      ndpi_snprintf(buf, buf_len, "%u.%u", proto.proto.master_protocol, proto.proto.app_protocol);
    else
      ndpi_snprintf(buf, buf_len, "%u", proto.proto.master_protocol);
  } else
    ndpi_snprintf(buf, buf_len, "%u", proto.proto.app_protocol);

  return(buf);
}

/* ****************************************************** */

char *ndpi_protocol2name(struct ndpi_detection_module_struct *ndpi_str,
			 ndpi_protocol proto, char *buf, u_int buf_len) {
  if((proto.proto.master_protocol != NDPI_PROTOCOL_UNKNOWN) && (proto.proto.master_protocol != proto.proto.app_protocol)) {
    if(proto.proto.app_protocol != NDPI_PROTOCOL_UNKNOWN)
      ndpi_snprintf(buf, buf_len, "%s.%s", ndpi_get_proto_name(ndpi_str, proto.proto.master_protocol),
		    ndpi_get_proto_name(ndpi_str, proto.proto.app_protocol));
    else
      ndpi_snprintf(buf, buf_len, "%s", ndpi_get_proto_name(ndpi_str, proto.proto.master_protocol));
  } else
    ndpi_snprintf(buf, buf_len, "%s", ndpi_get_proto_name(ndpi_str, proto.proto.app_protocol));

  return(buf);
}


#ifndef __KERNEL__
/* ****************************************************** */

int ndpi_is_custom_category(ndpi_protocol_category_t category) {
  switch(category) {
  case NDPI_PROTOCOL_CATEGORY_CUSTOM_1:
  case NDPI_PROTOCOL_CATEGORY_CUSTOM_2:
  case NDPI_PROTOCOL_CATEGORY_CUSTOM_3:
  case NDPI_PROTOCOL_CATEGORY_CUSTOM_4:
  case NDPI_PROTOCOL_CATEGORY_CUSTOM_5:
    return(1);

  default:
    return(0);
  }
}

/* ****************************************************** */

void ndpi_category_set_name(struct ndpi_detection_module_struct *ndpi_str,
			    ndpi_protocol_category_t category,
			    char *name) {
  if(!ndpi_str || !name)
    return;

  switch(category) {
  case NDPI_PROTOCOL_CATEGORY_CUSTOM_1:
    ndpi_snprintf(ndpi_str->custom_category_labels[0], CUSTOM_CATEGORY_LABEL_LEN, "%s", name);
    break;

  case NDPI_PROTOCOL_CATEGORY_CUSTOM_2:
    ndpi_snprintf(ndpi_str->custom_category_labels[1], CUSTOM_CATEGORY_LABEL_LEN, "%s", name);
    break;

  case NDPI_PROTOCOL_CATEGORY_CUSTOM_3:
    ndpi_snprintf(ndpi_str->custom_category_labels[2], CUSTOM_CATEGORY_LABEL_LEN, "%s", name);
    break;

  case NDPI_PROTOCOL_CATEGORY_CUSTOM_4:
    ndpi_snprintf(ndpi_str->custom_category_labels[3], CUSTOM_CATEGORY_LABEL_LEN, "%s", name);
    break;

  case NDPI_PROTOCOL_CATEGORY_CUSTOM_5:
    ndpi_snprintf(ndpi_str->custom_category_labels[4], CUSTOM_CATEGORY_LABEL_LEN, "%s", name);
    break;

  default:
    break;
  }
}
#endif
/* ****************************************************** */

const char *ndpi_confidence_get_name(ndpi_confidence_t confidence)
{
  switch(confidence) {
  case NDPI_CONFIDENCE_UNKNOWN:
    return "Unknown";

  case NDPI_CONFIDENCE_MATCH_BY_PORT:
    return "Match by port";

  case NDPI_CONFIDENCE_DPI_PARTIAL:
    return "DPI (partial)";

  case NDPI_CONFIDENCE_DPI_PARTIAL_CACHE:
    return "DPI (partial cache)";

  case NDPI_CONFIDENCE_DPI_CACHE:
    return "DPI (cache)";

  case NDPI_CONFIDENCE_DPI:
    return "DPI";

  case NDPI_CONFIDENCE_NBPF:
    return "nBPF";

  case NDPI_CONFIDENCE_MATCH_BY_IP:
    return "Match by IP";

  case NDPI_CONFIDENCE_DPI_AGGRESSIVE:
    return "DPI (aggressive)";

  case NDPI_CONFIDENCE_CUSTOM_RULE:
    return "Match by custom rule";

  default:
    return NULL;
  }
}

/* ****************************************************** */

const char *ndpi_fpc_confidence_get_name(ndpi_fpc_confidence_t fpc_confidence)
{
  switch(fpc_confidence) {
  case NDPI_FPC_CONFIDENCE_UNKNOWN:
    return "Unknown";

  case NDPI_FPC_CONFIDENCE_IP:
    return "IP address";

  case NDPI_FPC_CONFIDENCE_DNS:
    return "DNS";

  case NDPI_FPC_CONFIDENCE_DPI:
    return "DPI";

  default:
    return "Invalid"; /* Out of sync with ndpi_fpc_confidence_t definition */
  }
}

/* ****************************************************** */

#ifndef __KERNEL__

const char *ndpi_category_get_name(struct ndpi_detection_module_struct *ndpi_str,
				   ndpi_protocol_category_t category) {
  if((!ndpi_str) || (category >= NDPI_PROTOCOL_NUM_CATEGORIES)) {
    static char b[24];

    if(!ndpi_str)
      ndpi_snprintf(b, sizeof(b), "NULL nDPI");
    else
      ndpi_snprintf(b, sizeof(b), "Invalid category %d", (int) category);
    return(b);
  }

  if((category >= NDPI_PROTOCOL_CATEGORY_CUSTOM_1) && (category <= NDPI_PROTOCOL_CATEGORY_CUSTOM_5)) {
    switch((int)category) {
    case NDPI_PROTOCOL_CATEGORY_CUSTOM_1:
      return(ndpi_str->custom_category_labels[0]);
    case NDPI_PROTOCOL_CATEGORY_CUSTOM_2:
      return(ndpi_str->custom_category_labels[1]);
    case NDPI_PROTOCOL_CATEGORY_CUSTOM_3:
      return(ndpi_str->custom_category_labels[2]);
    case NDPI_PROTOCOL_CATEGORY_CUSTOM_4:
      return(ndpi_str->custom_category_labels[3]);
    case NDPI_PROTOCOL_CATEGORY_CUSTOM_5:
      return(ndpi_str->custom_category_labels[4]);
    }
  }
  return(categories[category]);
}

/* ****************************************************** */

static int category_depends_on_master(int proto)
{
  switch(proto) {
  case NDPI_PROTOCOL_MAIL_POP:
  case NDPI_PROTOCOL_MAIL_SMTP:
  case NDPI_PROTOCOL_MAIL_IMAP:
  case NDPI_PROTOCOL_MAIL_POPS:
  case NDPI_PROTOCOL_MAIL_SMTPS:
  case NDPI_PROTOCOL_MAIL_IMAPS:
  case NDPI_PROTOCOL_DNS:
    return 1;
  }

  return 0;
}

/* ****************************************************** */

ndpi_protocol_category_t ndpi_get_proto_category(struct ndpi_detection_module_struct *ndpi_str,
						 ndpi_protocol proto) {

  if(proto.category != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED)
    return(proto.category);

#if 0
  proto.proto.master_protocol = ndpi_map_user_proto_id_to_ndpi_id(ndpi_str, proto.proto.master_protocol),
    proto.proto.app_protocol = ndpi_map_user_proto_id_to_ndpi_id(ndpi_str, proto.proto.app_protocol);
#endif

  /* Simple rule: sub protocol first, master after, with some exceptions (i.e. mail) */

  if(category_depends_on_master(proto.proto.master_protocol)) {
    if(ndpi_is_valid_protoId(proto.proto.master_protocol))
      return(ndpi_str->proto_defaults[proto.proto.master_protocol].protoCategory);
  } else if((proto.proto.master_protocol == NDPI_PROTOCOL_UNKNOWN) ||
	    (ndpi_str->proto_defaults[proto.proto.app_protocol].protoCategory != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED)) {
    if(ndpi_is_valid_protoId(proto.proto.app_protocol))
      return(ndpi_str->proto_defaults[proto.proto.app_protocol].protoCategory);
  } else if(ndpi_is_valid_protoId(proto.proto.master_protocol))
    return(ndpi_str->proto_defaults[proto.proto.master_protocol].protoCategory);

  return(NDPI_PROTOCOL_CATEGORY_UNSPECIFIED);
}
#endif

/* ****************************************************** */

char *ndpi_get_proto_name(struct ndpi_detection_module_struct *ndpi_str,
			  u_int16_t proto_id) {
  if(!ndpi_str) return("Unknown");

  proto_id = ndpi_map_user_proto_id_to_ndpi_id(ndpi_str, proto_id);

  if((proto_id >= ndpi_str->ndpi_num_supported_protocols)
     || (!ndpi_is_valid_protoId(proto_id))
     || (ndpi_str->proto_defaults[proto_id].protoName == NULL))
    proto_id = NDPI_PROTOCOL_UNKNOWN;

  return(ndpi_str->proto_defaults[proto_id].protoName);
}

/* ****************************************************** */

ndpi_protocol_breed_t ndpi_get_proto_breed(struct ndpi_detection_module_struct *ndpi_str,
					   u_int16_t proto_id) {

  if(!ndpi_str) return(NDPI_PROTOCOL_UNRATED);

  proto_id = ndpi_map_user_proto_id_to_ndpi_id(ndpi_str, proto_id);

  if((proto_id >= ndpi_str->ndpi_num_supported_protocols) ||
     (!ndpi_is_valid_protoId(proto_id)) ||
     (ndpi_str->proto_defaults[proto_id].protoName == NULL))
    proto_id = NDPI_PROTOCOL_UNKNOWN;

  return(ndpi_str->proto_defaults[proto_id].protoBreed);
}

/* ****************************************************** */

char *ndpi_get_proto_breed_name(ndpi_protocol_breed_t breed_id) {
  switch(breed_id) {
  case NDPI_PROTOCOL_SAFE:
    return("Safe");
  case NDPI_PROTOCOL_ACCEPTABLE:
    return("Acceptable");
  case NDPI_PROTOCOL_FUN:
    return("Fun");
  case NDPI_PROTOCOL_UNSAFE:
    return("Unsafe");
  case NDPI_PROTOCOL_POTENTIALLY_DANGEROUS:
    return("Potentially Dangerous");
  case NDPI_PROTOCOL_TRACKER_ADS:
    return("Tracker/Ads");
  case NDPI_PROTOCOL_DANGEROUS:
    return("Dangerous");
  case NDPI_PROTOCOL_UNRATED:
    return("Unrated");
  default:
    return("???");
  }
}

/* ****************************************************** */

#ifdef OBSOLETE
/* TODO: remove one day as it's a duplicate */
u_int16_t ndpi_get_protocol_id(struct ndpi_detection_module_struct *ndpi_str, char *proto) {
  return(ndpi_get_proto_by_name(ndpi_str, proto));
}
#endif

/* ****************************************************** */

int ndpi_get_category_id(struct ndpi_detection_module_struct *ndpi_str, char *cat) {
#ifndef __KERNEL__
  int i;

  if(!ndpi_str) return(-1);

  for(i = 0; i < NDPI_PROTOCOL_NUM_CATEGORIES; i++) {
    const char *name = ndpi_category_get_name(ndpi_str, i);

    if(strcasecmp(cat, name) == 0)
      return(i);
  }
#endif

  return(-1);
}

#ifndef __KERNEL__
/* ****************************************************** */


static char *default_ports_string(char *ports_str, ndpi_port_range *default_ports){

  //dont display zero ports on help screen
  if (default_ports[0].port_low == 0)
    //- for readability
    return "-";

  int j=0;
  do
    {
      char port[18];
      if(default_ports[j].port_low == default_ports[j].port_high)
        sprintf(port,"%d,",default_ports[j].port_low);
      else
        sprintf(port,"%d-%d,",default_ports[j].port_low, default_ports[j].port_high);
      strcat(ports_str,port);
      j++;
    } while (j < MAX_DEFAULT_PORTS && default_ports[j].port_low != 0);

  //remove last comma
  ports_str[strlen(ports_str)-1] = '\0';

  return ports_str;

}

/* ****************************************************** */


void ndpi_dump_protocols(struct ndpi_detection_module_struct *ndpi_str, FILE *dump_out) {
  int i;

  if(!ndpi_str || !dump_out) return;

  for(i = 0; i < (int) ndpi_str->ndpi_num_supported_protocols; i++) {

    char udp_ports[128] = "";
    char tcp_ports[128] = "";

    fprintf(dump_out, "%3d %8d %-22s %-10s %-8s %-12s %-18s %-31s %-31s\n",
	    i, ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, i),
	    ndpi_str->proto_defaults[i].protoName,
	    ndpi_get_l4_proto_name(ndpi_get_l4_proto_info(ndpi_str, i)),
	    ndpi_str->proto_defaults[i].isAppProtocol ? "" : "X",
	    ndpi_get_proto_breed_name(ndpi_str->proto_defaults[i].protoBreed),
	    ndpi_category_get_name(ndpi_str, ndpi_str->proto_defaults[i].protoCategory),
	    default_ports_string(udp_ports,ndpi_str->proto_defaults[i].udp_default_ports),
	    default_ports_string(tcp_ports,ndpi_str->proto_defaults[i].tcp_default_ports)
	    );

  }
}

/* ********************************** */

/* Helper function used to generate Options fields in OPNsense */

void ndpi_generate_options(u_int opt, FILE *options_out) {
  struct ndpi_detection_module_struct *ndpi_str;
  NDPI_PROTOCOL_BITMASK all;
  u_int i;

  if (!options_out) return;
  ndpi_str = ndpi_init_detection_module(NULL);
  if (!ndpi_str) return;

  NDPI_BITMASK_SET_ALL(all);
  ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

  switch(opt) {
  case 0: /* List known protocols */
    {
      for(i = 1 /* Skip unknown */; i < ndpi_str->ndpi_num_supported_protocols; i++) {
        fprintf(options_out, "            <Option%d value=\"%u\">%s</Option%d>\n",
		i, i, ndpi_str->proto_defaults[i].protoName, i);
      }
    }
    break;

  case 1: /* List known categories */
    {
      for(i = 1 /* Skip Unknown */; i < NDPI_PROTOCOL_NUM_CATEGORIES; i++) {
	const char *name = ndpi_category_get_name(ndpi_str, i);

	if((name != NULL) && (name[0] != '\0')) {
	  fprintf(options_out, "            <Option%d value=\"%u\">%s</Option%d>\n",
		  i, i, name, i);
	}
      }
    }
    break;

  case 2: /* List known risks */
    {
      for(i = 1 /* Skip no risk */; i < NDPI_MAX_RISK; i++) {
        ndpi_risk_enum r = (ndpi_risk_enum)i;

        fprintf(options_out, "            <Option%d value=\"%u\">%s</Option%d>\n",
                i, i, ndpi_risk2str(r), i);
      }
    }
    break;

  default:
    fprintf(options_out, "%s\n", "WARNING: option -a out of range");
    break;
  }

  ndpi_exit_detection_module(ndpi_str);
}

/* ****************************************************** */

void ndpi_dump_risks_score(FILE *risk_out) {
  u_int i;

  if(!risk_out)
    return;

  fprintf(risk_out, "%3s %-46s %-44s %-8s %s %-8s %-8s\n",
	  "Id", "Code", "Risk", "Severity", "Score", "CliScore", "SrvScore");

  for(i = 1; i < NDPI_MAX_RISK; i++) {
    ndpi_risk_enum r = (ndpi_risk_enum)i;
    ndpi_risk risk   = (uint64_t)2 << (r-1);
    ndpi_risk_info* info = ndpi_risk2severity(r);
    ndpi_risk_severity s =info->severity;
    u_int16_t client_score, server_score;
    u_int16_t score = ndpi_risk2score(risk, &client_score, &server_score);

    fprintf(risk_out, "%3d %-46s %-44s %-8s %-8u %-8u %-8u\n",
	    i, ndpi_risk2code(r), ndpi_risk2str(r),
	    ndpi_severity2str(s),
	    score,
	    client_score, server_score);
  }
}

#endif
/* ****************************************************** */

char *ndpi_strnstr(const char *haystack, const char *needle, size_t len)
{
  if (!haystack || !needle) {
    return NULL;
  }

  return (char *)ndpi_memmem(haystack, strnlen(haystack, len), needle, strlen(needle));
}

/* ****************************************************** */

/*
 * Same as ndpi_strnstr but case-insensitive.
 * Please note that this function is *NOT* equivalent to strncasecmp().
 */
const char * ndpi_strncasestr(const char *s, const char *find, size_t len) {

  if (!s || !find) {
    return NULL;
  }

  const size_t find_len = strlen(find);

  if (find_len == 0) {
    return s;
  }

  const size_t s_len = strnlen(s, len);

  /* If 'find' is longer than 's', no match is possible */
  if (find_len > s_len) {
    return NULL;
  }

  const char *const end_of_search = s + s_len - find_len + 1;

  /* Cache the lowercased first character of 'find' */
  const unsigned char fc = tolower((unsigned char) *find);

  for (; s < end_of_search; ++s) {
    if (tolower((unsigned char)*s) == fc) {
      if (strncasecmp(s + 1, find + 1, find_len - 1) == 0) {
        return s;
      }
    }
  }

  return NULL;
}

/* ****************************************************** */

int ndpi_match_prefix(const u_int8_t *payload,
		      size_t payload_len, const char *str, size_t str_len) {
  int rc = str_len <= payload_len ? memcmp(payload, str, str_len) == 0 : 0;

  return(rc);
}

/* ****************************************************** */

int ndpi_match_string_subprotocol(struct ndpi_detection_module_struct *ndpi_str, char *string_to_match,
				  u_int string_to_match_len, ndpi_protocol_match_result *ret_match) {
  ndpi_automa *automa = &ndpi_str->host_automa;
  int rc;

  if(!ndpi_str) return(NDPI_PROTOCOL_UNKNOWN);

  if((automa->ac_automa == NULL) || (string_to_match_len == 0))
    return(NDPI_PROTOCOL_UNKNOWN);

  spin_lock_bh(&ndpi_str->host_automa_lock);
  rc = ndpi_match_string_common(((AC_AUTOMATA_t *) automa->ac_automa),
	  		        string_to_match,string_to_match_len, &ret_match->protocol_id,
			        &ret_match->protocol_category, &ret_match->protocol_breed);
  spin_unlock_bh(&ndpi_str->host_automa_lock);
  return rc < 0 ? rc : (int)ret_match->protocol_id;
}

/* **************************************** */

static u_int8_t ndpi_is_more_generic_protocol(u_int16_t previous_proto, u_int16_t new_proto) {
  /* Sometimes certificates are more generic than previously identified protocols */

  if((previous_proto == NDPI_PROTOCOL_UNKNOWN) || (previous_proto == new_proto))
    return(0);

  switch(previous_proto) {
  case NDPI_PROTOCOL_WHATSAPP_CALL:
  case NDPI_PROTOCOL_WHATSAPP_FILES:
    if(new_proto == NDPI_PROTOCOL_WHATSAPP)
      return(1);
    break;
  case NDPI_PROTOCOL_FACEBOOK_VOIP:
    if(new_proto == NDPI_PROTOCOL_FACEBOOK)
      return(1);
    break;
  }

  return(0);
}

/* ****************************************************** */

static u_int16_t ndpi_automa_match_string_subprotocol(struct ndpi_detection_module_struct *ndpi_str,
						      struct ndpi_flow_struct *flow, char *string_to_match,
						      u_int string_to_match_len, u_int16_t master_protocol_id,
						      ndpi_protocol_match_result *ret_match) {
  int matching_protocol_id;

  matching_protocol_id =
    ndpi_match_string_subprotocol(ndpi_str, string_to_match, string_to_match_len, ret_match);

  if(matching_protocol_id < 0)
    return NDPI_PROTOCOL_UNKNOWN;

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  {
    char m[256];
    u_int len = ndpi_min(sizeof(m) - 1, string_to_match_len);

    strncpy(m, string_to_match, len);
    m[len] = '\0';

    NDPI_LOG_DBG2(ndpi_str, "[NDPI] ndpi_match_host_subprotocol(%s): %s\n", m,
		  ndpi_str->proto_defaults[matching_protocol_id].protoName);
  }
#endif

  if(flow &&
     (matching_protocol_id != NDPI_PROTOCOL_UNKNOWN) &&
     (!ndpi_is_more_generic_protocol(flow->detected_protocol_stack[0], matching_protocol_id))) {
    /* Move the protocol on slot 0 down one position */
    flow->detected_protocol_stack[1] = master_protocol_id,
      flow->detected_protocol_stack[0] = matching_protocol_id;
    flow->confidence = NDPI_CONFIDENCE_DPI;
#ifndef __KERNEL__
    if(!category_depends_on_master(master_protocol_id) &&
       flow->category == NDPI_PROTOCOL_CATEGORY_UNSPECIFIED)
      flow->category = ret_match->protocol_category;
#endif
    return(flow->detected_protocol_stack[0]);
  }
  if(!flow && matching_protocol_id != NDPI_PROTOCOL_UNKNOWN)
    return matching_protocol_id;

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  {
    char m[256];
    u_int len = ndpi_min(sizeof(m) - 1, string_to_match_len);

    strncpy(m, string_to_match, len);
    m[len] = '\0';

    NDPI_LOG_DBG2(ndpi_str, "[NTOP] Unable to find a match for '%s'\n", m);
  }
#endif

  ret_match->protocol_id = NDPI_PROTOCOL_UNKNOWN, ret_match->protocol_category = NDPI_PROTOCOL_CATEGORY_UNSPECIFIED,
    ret_match->protocol_breed = NDPI_PROTOCOL_UNRATED;

  return(NDPI_PROTOCOL_UNKNOWN);
}

/* ****************************************************** */

void ndpi_check_subprotocol_risk(struct ndpi_detection_module_struct *ndpi_str,
				 struct ndpi_flow_struct *flow, u_int16_t subprotocol_id) {

  if(!ndpi_str) return;

  switch(subprotocol_id) {
  case NDPI_PROTOCOL_ANYDESK:
    ndpi_set_risk(ndpi_str, flow, NDPI_DESKTOP_OR_FILE_SHARING_SESSION, "Found AnyDesk"); /* Remote assistance */
    break;
  }
}

/* ****************************************************** */

u_int16_t ndpi_match_host_subprotocol(struct ndpi_detection_module_struct *ndpi_str,
				      struct ndpi_flow_struct *flow,
				      char *string_to_match, u_int string_to_match_len,
				      ndpi_protocol_match_result *ret_match,
				      u_int16_t master_protocol_id,
				      int update_flow_classification) {
  u_int16_t rc;
  ndpi_protocol_category_t id;

  if(!ndpi_str) return(-1);

  memset(ret_match, 0, sizeof(*ret_match));

  rc = ndpi_automa_match_string_subprotocol(ndpi_str, update_flow_classification ? flow : NULL,
					    string_to_match, string_to_match_len,
					    master_protocol_id, ret_match);
  id = ret_match->protocol_category;

#ifndef __KERNEL__
  if(ndpi_get_custom_category_match(ndpi_str, string_to_match,
			            string_to_match_len, &id) != -1) {
    /* if(id != -1) */ {
      ret_match->protocol_category = id;
      flow->category = id;
      rc = master_protocol_id;
    }
  }

  if(ndpi_str->risky_domain_automa.ac_automa != NULL) {
    u_int32_t proto_id;
    u_int16_t rc1 = ndpi_match_string_common(ndpi_str->risky_domain_automa.ac_automa,
					     string_to_match, string_to_match_len,
					     &proto_id, NULL, NULL);
    if(rc1 > 0) {
      if(is_flowrisk_info_enabled(ndpi_str, NDPI_RISKY_DOMAIN)) {
        char str[64] = { '\0' };

        strncpy(str, string_to_match, ndpi_min(string_to_match_len, sizeof(str)-1));
        ndpi_set_risk(ndpi_str, flow, NDPI_RISKY_DOMAIN, str);
      } else {
        ndpi_set_risk(ndpi_str, flow, NDPI_RISKY_DOMAIN, NULL);
      }
    }
  }
#else
  {
      static const char pref_str[]="RISK_DOMAIN_";
      char risk_domain_str[sizeof(pref_str) + 64 + 1];
      u_int32_t val;
      size_t len = sizeof(pref_str)-1,len2 = ndpi_min(string_to_match_len, 64-1);

      strcpy(risk_domain_str,pref_str);
      strncpy(&risk_domain_str[len],string_to_match,len2);
      len += len2;
      risk_domain_str[len] = '\0';
      if(ndpi_match_string_value(ndpi_str->host_automa.ac_automa, risk_domain_str, len | AC_FEATURE_EXACT, &val) != -1)			 
	      ndpi_set_risk(ndpi_str, flow, NDPI_RISKY_DOMAIN, &risk_domain_str[len]);
  }
#endif

  /* Add punycode check */
  if(ndpi_check_punycode_string(string_to_match, string_to_match_len)) {
    if(is_flowrisk_info_enabled(ndpi_str, NDPI_PUNYCODE_IDN)) {
      char str[64] = { '\0' };

      strncpy(str, string_to_match, ndpi_min(string_to_match_len, sizeof(str)-1));
      ndpi_set_risk(ndpi_str, flow, NDPI_PUNYCODE_IDN, str);
    } else {
      ndpi_set_risk(ndpi_str, flow, NDPI_PUNYCODE_IDN, NULL);
    }
  }

  return(rc);
}

/* **************************************** */

int ndpi_match_hostname_protocol(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow,
				 u_int16_t master_protocol, char *name, u_int name_len) {
  ndpi_protocol_match_result ret_match;
  u_int16_t subproto, what_len;
  char *what;

  if(!ndpi_struct) return(0);

  if((name_len > 2) && (name[0] == '*') && (name[1] == '.'))
    what = &name[1], what_len = name_len - 1;
  else
    what = name, what_len = name_len;

  subproto = ndpi_match_host_subprotocol(ndpi_struct, flow, what, what_len,
					 &ret_match, master_protocol, 1);

  if(subproto != NDPI_PROTOCOL_UNKNOWN) {
    ndpi_set_detected_protocol(ndpi_struct, flow, subproto, master_protocol, NDPI_CONFIDENCE_DPI);
#ifndef __KERNEL__
    if(!category_depends_on_master(master_protocol))
      change_category(flow, ret_match.protocol_category);
#endif

    if(subproto == NDPI_PROTOCOL_OOKLA) {
      ookla_add_to_cache(ndpi_struct, flow);
    }

    return(1);
  } else
    return(0);
}

/* ****************************************************** */

static inline int ndpi_match_xgram(unsigned int *map,unsigned int l,const char *str) {
  unsigned int i,c;

  for(i=0,c=0; *str && i < l; i++) {
    unsigned char a = (unsigned char)(*str++);
    if(a < 'a' || a > 'z') return 0;
    c *= XGRAMS_C;
    c += a-'a';
  }

  return (map[c >> 5] & (1u << (c & 0x1f))) != 0;
}

int ndpi_match_bigram(const char *str) {
  return ndpi_match_xgram(bigrams_bitmap, 2, str);
}

NDPI_STATIC int ndpi_match_impossible_bigram(const char *str) {
  return ndpi_match_xgram(impossible_bigrams_bitmap, 2, str);
}

/* ****************************************************** */

NDPI_STATIC int ndpi_match_trigram(const char *str) {
  return ndpi_match_xgram(trigrams_bitmap, 3, str);
}

/* ****************************************************** */

void ndpi_free_flow(struct ndpi_flow_struct *flow) {
  if(flow) {
    ndpi_free_flow_data(flow);
#ifndef __KERNEL__
    ndpi_free(flow);
#endif
  }
}

/* ****************************************************** */

char *ndpi_revision() {
  return(NDPI_GIT_RELEASE);
}

/* ****************************************************** */

#ifndef __KERNEL__
void gettimeofday64(struct timespec64* tp, void * tzp) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  tp->tv_sec = tv.tv_sec;
  tp->tv_nsec = tv.tv_usec*1000;
}
#endif

int NDPI_BITMASK_COMPARE(NDPI_PROTOCOL_BITMASK a, NDPI_PROTOCOL_BITMASK b) {
  unsigned int i;

  for(i = 0; i < NDPI_NUM_FDS_BITS; i++) {
    if(a.fds_bits[i] & b.fds_bits[i])
      return(1);
  }

  return(0);
}

#ifdef __KERNEL__
int NDPI_BITMASK_IS_EMPTY(NDPI_PROTOCOL_BITMASK a) {
  unsigned int i;

  for(i = 0; i < NDPI_NUM_FDS_BITS; i++)
    if(a.fds_bits[i] != 0)
      return(0);

  return(1);
}
#endif

u_int16_t ndpi_get_api_version() {
  return(NDPI_API_VERSION);
}

const char *ndpi_get_gcrypt_version(void) {
  return gcry_check_version(NULL);
}

ndpi_proto_defaults_t *ndpi_get_proto_defaults(struct ndpi_detection_module_struct *ndpi_str) {
  return(ndpi_str ? ndpi_str->proto_defaults : NULL);
}

u_int ndpi_get_ndpi_num_supported_protocols(struct ndpi_detection_module_struct *ndpi_str) {
  return(ndpi_str ? ndpi_str->ndpi_num_supported_protocols : 0);
}

u_int ndpi_get_ndpi_num_custom_protocols(struct ndpi_detection_module_struct *ndpi_str) {
  return(ndpi_str ? ndpi_str->ndpi_num_custom_protocols : 0);
}

u_int ndpi_get_ndpi_detection_module_size() {
  return(sizeof(struct ndpi_detection_module_struct));
}

/* ******************************************************************** */

u_int32_t ndpi_get_current_time(struct ndpi_flow_struct *flow)
{
  if(flow)
    return flow->last_packet_time;
  return 0;
}

/* ******************************************************************** */

/*
  This function tells if it's possible to further dissect a given flow
  0 - All possible dissection has been completed
  1 - Additional dissection is possible
*/
u_int8_t ndpi_extra_dissection_possible(struct ndpi_detection_module_struct *ndpi_str,
                                        struct ndpi_flow_struct *flow) {
  NDPI_LOG_DBG2(ndpi_str, "Protos (%u.%u): %d\n",
		flow->detected_protocol_stack[0],
		flow->detected_protocol_stack[1],
		!!flow->extra_packets_func);

  if(!flow->extra_packets_func) {
    ndpi_check_probing_attempt(ndpi_str, flow);
    return(0);
  }

  return(1);
}

/* ******************************************************************** */

const char *ndpi_get_l4_proto_name(ndpi_l4_proto_info proto) {
  switch(proto) {
  case ndpi_l4_proto_unknown:
    return("");

  case ndpi_l4_proto_tcp_only:
    return("TCP");

  case ndpi_l4_proto_udp_only:
    return("UDP");

  case ndpi_l4_proto_tcp_and_udp:
    return("TCP/UDP");
  }

  return("");
}

/* ******************************************************************** */

ndpi_l4_proto_info ndpi_get_l4_proto_info(struct ndpi_detection_module_struct *ndpi_struct,
					  u_int16_t ndpi_proto_id) {
  if(ndpi_struct && ndpi_proto_id < ndpi_struct->ndpi_num_supported_protocols) {
    u_int16_t idx = ndpi_struct->proto_defaults[ndpi_proto_id].dissector_idx;
    NDPI_SELECTION_BITMASK_PROTOCOL_SIZE bm = ndpi_struct->callback_buffer[idx].ndpi_selection_bitmask;

    if(bm & NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP)
      return(ndpi_l4_proto_tcp_only);
    else if(bm & NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP)
      return(ndpi_l4_proto_udp_only);
    else if(bm & NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP)
      return(ndpi_l4_proto_tcp_and_udp);
  }

  return(ndpi_l4_proto_unknown); /* default */
}

/* ******************************************************************** */

ndpi_ptree_t *ndpi_ptree_create(void) {
  ndpi_ptree_t *tree = (ndpi_ptree_t *) ndpi_malloc(sizeof(ndpi_ptree_t));

  if(tree) {
    tree->v4 = ndpi_patricia_new(32);
    tree->v6 = ndpi_patricia_new(128);

    if((!tree->v4) || (!tree->v6)) {
      ndpi_ptree_destroy(tree);
      return(NULL);
    }
  }

  return(tree);
}

/* ******************************************************************** */

void ndpi_ptree_destroy(ndpi_ptree_t *tree) {
  if(tree) {
    if(tree->v4)
      ndpi_patricia_destroy(tree->v4, free_ptree_data);

    if(tree->v6)
      ndpi_patricia_destroy(tree->v6, free_ptree_data);

    ndpi_free(tree);
  }
}

/* ******************************************************************** */

int ndpi_ptree_insert(ndpi_ptree_t *tree, const ndpi_ip_addr_t *addr,
		      u_int8_t bits, u_int64_t user_data) {
  u_int8_t is_v6 = ndpi_is_ipv6(addr);
  ndpi_patricia_tree_t *ptree;
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;

  if(!tree)
    return(-4);

  ptree = is_v6 ? tree->v6 : tree->v4;

  if(bits > ptree->maxbits)
    return(-1);

  if(is_v6)
    ndpi_fill_prefix_v6(&prefix, (const struct in6_addr *) &addr->ipv6, bits, ptree->maxbits);
  else
    ndpi_fill_prefix_v4(&prefix, (const struct in_addr *) &addr->ipv4, bits, ptree->maxbits);

  /* Verify that the node does not already exist */
  node = ndpi_patricia_search_best(ptree, &prefix);

  if(node && (node->prefix->bitlen == bits))
    return(-2);

  node = ndpi_patricia_lookup(ptree, &prefix);

  if(node != NULL) {
    node->value.u.uv64 = user_data;

    return(0);
  }

  return(-3);
}

/* ******************************************************************** */

int ndpi_ptree_match_addr(ndpi_ptree_t *tree,
			  const ndpi_ip_addr_t *addr, u_int64_t *user_data) {
  u_int8_t is_v6 = ndpi_is_ipv6(addr);
  ndpi_patricia_tree_t *ptree;
  ndpi_prefix_t prefix;
  ndpi_patricia_node_t *node;
  int bits;

  if(!tree)
    return(-2);

  ptree = is_v6 ? tree->v6 : tree->v4;
  bits = ptree->maxbits;

  if(is_v6)
    ndpi_fill_prefix_v6(&prefix, (const struct in6_addr *) &addr->ipv6,
			bits, ptree->maxbits);
  else
    ndpi_fill_prefix_v4(&prefix, (const struct in_addr *) &addr->ipv4,
			bits, ptree->maxbits);

  node = ndpi_patricia_search_best(ptree, &prefix);

  if(node) {
    *user_data = node->value.u.uv64;

    return(0);
  }

  return(-1);
}

/* ******************************************************************** */

NDPI_STATIC void ndpi_md5(const u_char *data, size_t data_len, u_char hash[16]) {
  ndpi_MD5_CTX ctx;

  ndpi_MD5Init(&ctx);
  ndpi_MD5Update(&ctx, data, data_len);
  ndpi_MD5Final(hash, &ctx);
}

/* ******************************************************************** */

void ndpi_sha256(const u_char *data, size_t data_len, u_int8_t sha_hash[32]) {
  ndpi_SHA256_CTX sha_ctx;

  ndpi_sha256_init(&sha_ctx);
  ndpi_sha256_update(&sha_ctx, data, data_len);
  ndpi_sha256_final(&sha_ctx, sha_hash);
}

/* ******************************************************************** */

static int enough(int a, int b) {
  u_int8_t percentage = 20;

  if(b <= 1) return(0);
  if(a == 0) return(1);

  if(b > (((a+1)*percentage)/100)) return(1);

  return(0);
}

/* ******************************************************************** */

u_int8_t ends_with(struct ndpi_detection_module_struct *ndpi_struct,
                   char *str, char *ends) {
  u_int str_len = str ? strlen(str) : 0;
  u_int8_t ends_len = strlen(ends);
  u_int8_t rc;

  if(str_len < ends_len) return(0);

  rc = (strncmp(&str[str_len-ends_len], ends, ends_len) != 0) ? 0 : 1;

  NDPI_LOG_DBG2(ndpi_struct, "[DGA] %s / %s [rc: %u]\n", str, ends, rc);

  return(rc);
}

/* ******************************************************************** */

static int ndpi_is_trigram_char(char c) {
  if(ndpi_isdigit(c) || (c == '.') || (c == '-'))
    return(0);
  else
    return(1);
}

/* ******************************************************************** */

static int ndpi_is_vowel(char c) {
  switch(c) {
  case 'a':
  case 'e':
  case 'i':
  case 'o':
  case 'u':
  case 'y': // Not a real vowel...
  case 'x': // Not a real vowel...
    return(1);

  default:
    return(0);
  }
}

/* ******************************************************************** */

int ndpi_check_dga_name(struct ndpi_detection_module_struct *ndpi_str,
			struct ndpi_flow_struct *flow,
			char *name, u_int8_t is_hostname, u_int8_t check_subproto,
			u_int8_t flow_fully_classified) {

  /* Get domain name if ndpi_load_domain_suffixes(..) has been called */
#ifndef __KERNEL__
  name = (char*)ndpi_get_host_domain(ndpi_str, name);
#endif

  if(ndpi_dga_function != NULL) {
    /* A custom DGA function is defined */
    int rc = ndpi_dga_function(name, is_hostname);

    if(rc) {
      if(flow)
	ndpi_set_risk(ndpi_str, flow, NDPI_SUSPICIOUS_DGA_DOMAIN, name);
    }

    return(rc);
  } else {
    int len, rc = 0, trigram_char_skip = 0;
    u_int8_t max_num_char_repetitions = 0, last_char = 0, num_char_repetitions = 0, num_dots = 0, num_trigram_dots = 0;
    u_int8_t max_domain_element_len = 0, curr_domain_element_len = 0, first_element_is_numeric = 1;
    ndpi_protocol_match_result ret_match;

    if((!name)
       || (strchr(name, '_') != NULL)
       || (strchr(name, '-') != NULL)
       || (ends_with(ndpi_str, name, "in-addr.arpa"))
       || (ends_with(ndpi_str, name, "ip6.arpa"))
       /* Ignore TLD .local .lan and .home */
       || (ends_with(ndpi_str, name, ".local"))
       || (ends_with(ndpi_str, name, ".lan"))
       || (ends_with(ndpi_str, name, ".home"))
       )
      return(0);

    if(flow && (flow->detected_protocol_stack[1] != NDPI_PROTOCOL_UNKNOWN || flow_fully_classified))
      return(0); /* Ignore DGA check for protocols already fully detected */

    if(check_subproto &&
       ndpi_match_string_subprotocol(ndpi_str, name, strlen(name), &ret_match) > 0)
      return(0); /* Ignore DGA for known domain names */

    if(ndpi_isdigit(name[0])) {
      struct in_addr ip_addr;
      char buf2[22];
      
      ip_addr.s_addr = inet_addr(name);
      if(strcmp(inet_ntop(AF_INET,&ip_addr,buf2,sizeof(buf2)), name) == 0)
	return(0); /* Ignore numeric IPs */
    }

    if(strncmp(name, "www.", 4) == 0)
      name = &name[4];

    NDPI_LOG_DBG2(ndpi_str, "[DGA] check %s\n", name);

    len = strlen(name);

    if(len >= 5) {
      int num_found = 0, num_impossible = 0, num_bigram_checks = 0,
	num_trigram_found = 0, num_trigram_checked = 0, num_dash = 0,
	num_digits = 0, num_vowels = 0, num_trigram_vowels = 0, num_words = 0, skip_next_bigram = 0;
      char tmp[128], *word, *tok_tmp;
      u_int i, j, max_tmp_len = sizeof(tmp)-1;

      len = ndpi_snprintf(tmp, max_tmp_len, "%s", name);

      if(len < 0) {
	NDPI_LOG_DBG2(ndpi_str, "[DGA] too short");
	return(0);
      } else
	tmp[(u_int)len < max_tmp_len ? (u_int)len : max_tmp_len] = '\0';

      for(i=0, j=0; (i<(u_int)len) && (j<max_tmp_len); i++) {
	tmp[j] = tolower(name[i]);

	if(tmp[j] == '.') {
	  num_dots++;
	} else if(num_dots == 0) {
	  if(!ndpi_isdigit(tmp[j]))
	    first_element_is_numeric = 0;
	}

	if(ndpi_is_vowel(tmp[j]))
	  num_vowels++;

	if(last_char == tmp[j]) {
	  if(++num_char_repetitions > max_num_char_repetitions)
	    max_num_char_repetitions = num_char_repetitions;
	} else
	  num_char_repetitions = 1, last_char = tmp[j];

	if(ndpi_isdigit(tmp[j])) {
	  num_digits++;

	  if(((j+2)<(u_int)len) && ndpi_isdigit(tmp[j+1]) && (tmp[j+2] == '.')) {
	    /* Check if there are too many digits */
	    if(num_digits < 4)
	      return(0); /* Double digits */
	  }
	}

	switch(tmp[j]) {
	case '.':
	case '-':
	case '_':
	case '/':
	case ')':
	case '(':
	case ';':
	case ':':
	case '[':
	case ']':
	case ' ':
	  /*
	    Domain/word separator chars

	    NOTE:
	    this function is used also to detect other type of issues
	    such as invalid/suspiciuous user agent
	  */
	  if(curr_domain_element_len > max_domain_element_len)
	    max_domain_element_len = curr_domain_element_len;

	  curr_domain_element_len = 0;
	  break;

	default:
	  curr_domain_element_len++;
	  break;
	}

	j++;
      }

      if(num_dots == 0) /* Doesn't look like a domain name */
        return(0);

      if(curr_domain_element_len > max_domain_element_len)
        max_domain_element_len = curr_domain_element_len;

      NDPI_LOG_DBG2(ndpi_str, "[DGA] [max_num_char_repetitions: %u][max_domain_element_len: %u]\n",
                    max_num_char_repetitions, max_domain_element_len);

      if(
	 (is_hostname
	  && (num_dots > 5)
	  && (!first_element_is_numeric)
	  )
	 || (max_num_char_repetitions > 5 /* num or consecutive repeated chars */)
	 /*
	   In case of a name with too many consecutive chars an alert is triggered
	   This is the case for instance of the wildcard DNS query used by NetBIOS
	   (ckaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa) and that can be exploited
	   for reflection attacks
	   - https://www.akamai.com/uk/en/multimedia/documents/state-of-the-internet/ddos-reflection-netbios-name-server-rpc-portmap-sentinel-udp-threat-advisory.pdf
	   - http://ubiqx.org/cifs/NetBIOS.html
	 */
	 || ((max_domain_element_len >= 19 /* word too long. Example bbcbedxhgjmdobdprmen.com */) && ((num_char_repetitions > 1) || (num_digits > 1)))
	 ) {
	if(flow) {
	  ndpi_set_risk(ndpi_str, flow, NDPI_SUSPICIOUS_DGA_DOMAIN, name);
	}

	NDPI_LOG_DBG2(ndpi_str, "[DGA] Found!");

	return(1);
      }

      tmp[j] = '\0';
      len = j;

      u_int max_num_consecutive_digits_first_word = 0, num_word = 0;

      for(word = strtok_r(tmp, ".", &tok_tmp); ; word = strtok_r(NULL, ".", &tok_tmp)) {
	u_int num_consecutive_digits = 0, word_len;

	if(!word) break; else num_word++;

	num_words++;

	if(num_words > 2)
	  break; /* Stop after the 2nd word of the domain name */

	if((word_len = strlen(word)) < 5) continue;

	if((word_len < 10) && (ends_with(ndpi_str, word, "cdn") /* Content Delivery Network ? */))
	  continue; /* Ignore names (not too long) that end with cdn [ ssl.p.jwpcdn.com or www.awxcdn.com ] */

	NDPI_LOG_DBG2(ndpi_str, "[DGA] word(%s) [%s][len: %u]\n", word, name, (unsigned int)strlen(word));

	trigram_char_skip = 0;

	for(i = 0; word[i+1] != '\0'; i++) {
	  if(ndpi_isdigit(word[i]))
	    num_consecutive_digits++;
	  else {
	    if((num_word == 1) && (num_consecutive_digits > max_num_consecutive_digits_first_word))
	      max_num_consecutive_digits_first_word = num_consecutive_digits;

	    num_consecutive_digits = 0;
	  }

	  switch(word[i]) {
	  case '-':
	    num_dash++;
	    /*
	      Let's check for double+consecutive --
	      that are usually ok
	      r2---sn-uxaxpu5ap5-2n5e.gvt1.com
	    */
	    if(word[i+1] == '-')
	      return(0); /* Double dash */
	    continue;

	  case '_':
	  case ':':
	    continue;
	    break;

	  case '.':
	    continue;
	    break;
	  }

	  num_bigram_checks++;

	  NDPI_LOG_DBG2(ndpi_str, "[DGA] checking %c%c\n", word[i], word[i+1]);

	  if(ndpi_match_impossible_bigram(&word[i])) {
	    NDPI_LOG_DBG2(ndpi_str, "[DGA] IMPOSSIBLE %s\n", &word[i]);

	    num_impossible++;
	  } else {
	    if(!skip_next_bigram) {
	      if(ndpi_match_bigram(&word[i])) {
		num_found++, skip_next_bigram = 1;
	      }
	    } else
	      skip_next_bigram = 0;
	  }

	  if((num_trigram_dots < 2) && (word[i+2] != '\0')) {
	    NDPI_LOG_DBG2(ndpi_str, "[DGA] %s [trigram_char_skip: %u]\n", &word[i], trigram_char_skip);

	    if(ndpi_is_trigram_char(word[i]) && ndpi_is_trigram_char(word[i+1]) && ndpi_is_trigram_char(word[i+2])) {
	      if(trigram_char_skip) {
		trigram_char_skip--;
	      } else {
		num_trigram_checked++;

		if(ndpi_match_trigram(&word[i]))
		  num_trigram_found++, trigram_char_skip = 2 /* 1 char overlap */;
		else
		  NDPI_LOG_DBG2(ndpi_str, "[DGA] NO Trigram %c%c%c\n", word[i], word[i+1], word[i+2]);

		/* Count vowels */
		num_trigram_vowels += ndpi_is_vowel(word[i]) + ndpi_is_vowel(word[i+1]) + ndpi_is_vowel(word[i+2]);
	      }
	    } else {
	      if(word[i] == '.')
		num_trigram_dots++;

	      trigram_char_skip = 0;
	    }
	  }
	} /* for */

	if((num_word == 1) && (num_consecutive_digits > max_num_consecutive_digits_first_word))
	  max_num_consecutive_digits_first_word = num_consecutive_digits;
      } /* for */

      NDPI_LOG_DBG2(ndpi_str, "[DGA] max_num_consecutive_digits_first_word=%u\n", max_num_consecutive_digits_first_word);

      NDPI_LOG_DBG2(ndpi_str, "[DGA] [%s][num_found: %u][num_impossible: %u][num_digits: %u][num_bigram_checks: %u][num_vowels: %u/%u][num_trigram_vowels: %u][num_trigram_found: %u/%u][vowels: %u][rc: %u]\n",
		    name, num_found, num_impossible, num_digits, num_bigram_checks, num_vowels, len, num_trigram_vowels,
		    num_trigram_checked, num_trigram_found, num_vowels, rc);

      if((len > 16) && (num_dots < 3) && ((num_vowels*4) < (len-num_dots))) {
	if((num_trigram_checked > 2) && (num_trigram_vowels >= (num_trigram_found-1)))
	  ; /* skip me */
	else
	  rc = 1;
      }

      if(num_bigram_checks
	 /* We already checked num_dots > 0 */
	 && ((num_found == 0) || ((num_digits > 5) && (num_words <= 3) && (num_impossible > 0))
	     || enough(num_found, num_impossible)
	     || ((num_trigram_checked > 2)
		 && ((num_trigram_found < (num_trigram_checked/2))
		     || ((num_trigram_vowels < (num_trigram_found-1)) && (num_dash == 0) && (num_dots > 1) && (num_impossible > 0)))
		 )
	     )
	 )
	rc = 1;

      if((num_trigram_checked > 2) && (num_vowels == 0))
	rc = 1;

      if(num_dash > 2)
	rc = 0;

      /* Skip names whose first word item has at least 3 consecutive digits */
      if(max_num_consecutive_digits_first_word > 2)
        rc = 0;

      if(rc)
	NDPI_LOG_DBG2(ndpi_str, "[DGA] %s [num_found: %u][num_impossible: %u]\n",
		      name, num_found, num_impossible);
    }

    NDPI_LOG_DBG2(ndpi_str, "[DGA] Result: %u\n", rc);

    if(rc && flow)
      ndpi_set_risk(ndpi_str, flow, NDPI_SUSPICIOUS_DGA_DOMAIN, name);

    return(rc);
  }
}

/* ******************************************************************** */

ndpi_risk_info* ndpi_risk2severity(ndpi_risk_enum risk) {
  return(&ndpi_known_risks[risk]);
}

#ifndef __KERNEL__
struct ndpi_packet_struct *
ndpi_get_packet_struct(struct ndpi_detection_module_struct *ndpi_mod) {
	return &ndpi_mod->packet;
}
#endif
/* ******************************************************************** */

char *ndpi_hostname_sni_set(struct ndpi_flow_struct *flow,
			    const u_int8_t *value, size_t value_len,
			    int normalize) {
  char *dst;
  size_t len, i;

  len = ndpi_min(value_len, sizeof(flow->host_server_name) - 1);
  dst = flow->host_server_name;

  if(!normalize) {
    memcpy(dst,&value[value_len - len],len);
    dst[len] = '\0';
  } else {
    for(i = 0; i < len; i++) {
      char c = value[value_len - len + i];
      if(!c) break;
      if(c == ':') break; /* e.g. skip port in "239.255.255.250:1900" */
      if(normalize & NDPI_HOSTNAME_NORM_LC) c = tolower(c);
      if(normalize & NDPI_HOSTNAME_NORM_REPLACE_IC) {
        if (c == '\t') c = ' ';
        if (ndpi_isprint(c) == 0)
	  c = '?';
      }
      dst[i] = c;
    }

    dst[i] = '\0';
    if(normalize & NDPI_HOSTNAME_NORM_STRIP_EOLSP) {
      /* Removing spaces at the end of a line */
      while(i > 0 && dst[i-1] == ' ')
        dst[--i] = '\0';
    }
  }

  return dst;
}

/* ******************************************************************** */

char *ndpi_user_agent_set(struct ndpi_flow_struct *flow,
			  const u_int8_t *value, size_t value_len) {
  if(flow->http.user_agent != NULL) {
    /* Already set: ignore double set */
    return NULL;
  }
  if(value_len == 0) {
    return NULL;
  }

  flow->http.user_agent = ndpi_malloc(value_len + 1);
  if(flow->http.user_agent != NULL) {
    memcpy(flow->http.user_agent, value, value_len);
    flow->http.user_agent[value_len] = '\0';
  }

  return flow->http.user_agent;
}

/* ******************************************************************** */

int ndpi_seen_flow_beginning(const struct ndpi_flow_struct *flow)
{
  if(flow->l4_proto == IPPROTO_TCP &&
     (flow->l4.tcp.seen_syn == 0 || flow->l4.tcp.seen_syn_ack == 0 ||
      flow->l4.tcp.seen_ack == 0))
    return 0;
  return 1;
}

/* ******************************************************************** */

void ndpi_set_user_data(struct ndpi_detection_module_struct *ndpi_str, void *user_data) {
  if (ndpi_str == NULL)
    {
      return;
    }

  if (ndpi_str->user_data != NULL)
    {
      NDPI_LOG_ERR(ndpi_str, "%s", "User data is already set. Overwriting.")
	}

  ndpi_str->user_data = user_data;
}

/* ******************************************************************** */

void *ndpi_get_user_data(struct ndpi_detection_module_struct *ndpi_str)
{
  if(ndpi_str)
    return ndpi_str->user_data;
  return NULL;
}

/* ******************************************************************** */

static u_int16_t __get_proto_id(const char *proto_name_or_id)
{
  struct ndpi_detection_module_struct *module;
  NDPI_PROTOCOL_BITMASK all;
  u_int16_t proto_id;
  long val;

  /* Let try to decode the string as numerical protocol id */
  /* We can't easily use ndpi_strtonum because we want to be sure that there are no
     others characters after the number */
#ifndef __KERNEL__
  {
  char *endptr;
  errno = 0;    /* To distinguish success/failure after call */
  val = strtol(proto_name_or_id, &endptr, 10);
  if(errno == 0 && *endptr == '\0' &&
     (val >= 0 && val < NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS))
	    return val;
  }
#else
  val = atoi(proto_name_or_id);
  if(val >= 0 && val < NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS)
	return val;
#endif
  /* Try to decode the string as protocol name */
  /* Use a temporary module with all protocols enabled */
  module = ndpi_init_detection_module(NULL);
  if(!module)
    return NDPI_PROTOCOL_UNKNOWN;
  NDPI_BITMASK_SET_ALL(all);
  ndpi_set_protocol_detection_bitmask2(module, &all);
  /* Try to be fast: we need only the protocol name -> protocol id mapping! */
  ndpi_set_config(module, "any", "ip_list.load", "0");
  ndpi_set_config(module, NULL, "flow_risk_lists.load", "0");
  ndpi_finalize_initialization(module);
  proto_id = ndpi_get_proto_by_name(module, proto_name_or_id);
  ndpi_exit_detection_module(module);

  return proto_id;
}

/* ******************************************************************** */

static ndpi_cfg_error _set_param_const_flag(struct ndpi_detection_module_struct *ndpi_str,
                                                void *_variable, const char *value,
                                                const char *min_value, const char *max_value,
                                                const char *proto, const char *param)
{
  return NDPI_CFG_INVALID_PARAM;
}

static ndpi_risk_enum __get_flowrisk_id(const char *flowrisk_name_or_id)
{
#ifndef __KERNEL__
  char *endptr;
#endif
  long val;
  int i;

  /* Let try to decode the string as numerical flow risk id */
  /* We can't easily use ndpi_strtonum because we want to be sure that there are no
     others characters after the number */
#ifndef __KERNEL__
  errno = 0;    /* To distinguish success/failure after call */
  val = strtol(flowrisk_name_or_id, &endptr, 10);
  if(errno == 0 && *endptr == '\0' &&
     (val >= 0 && val < NDPI_MAX_RISK)) {
    return val;

  }
#else
  if(!kstrtol(flowrisk_name_or_id, 10, &val)) {
    if(val >= 0 && val < NDPI_MAX_RISK)
      return val;
  }
#endif

  /* Try to decode the string as flow risk name */
  for(i = 0; i < NDPI_MAX_RISK; i++) {
    if(ndpi_risk_shortnames[i] && strcmp(ndpi_risk_shortnames[i], flowrisk_name_or_id) == 0)
      return i;
  }

  return NDPI_NO_RISK;
}

/* ******************************************************************** */

static ndpi_cfg_error _set_param_enable_disable(struct ndpi_detection_module_struct *ndpi_str,
                                                void *_variable, const char *value,
                                                const char *min_value, const char *max_value,
                                                const char *proto, const char *param) {
  int *variable = (int *)_variable;

  (void)ndpi_str;
  (void)min_value;
  (void)max_value;
  (void)proto;
  (void)param;

  if(strcmp(value, "1") == 0 ||
     strcmp(value, "enable") == 0) {
    *variable = 1;
    return NDPI_CFG_OK;
  }

  if(strcmp(value, "0") == 0 || strcmp(value, "off") == 0 ||
     strcmp(value, "disable") == 0) {
    *variable = 0;
    return NDPI_CFG_OK;
  }

  return NDPI_CFG_INVALID_PARAM;
}

/* ******************************************************************** */

static ndpi_cfg_error _set_param_int(struct ndpi_detection_module_struct *ndpi_str,
                                     void *_variable, const char *value,
                                     const char *min_value, const char *max_value,
                                     const char *proto, const char *param) {
  int *variable = (int *)_variable;
  const char *errstrp;
  long val;

  (void)ndpi_str;
  (void)proto;
  (void)param;

  val = ndpi_strtonum(value, LONG_MIN, LONG_MAX, &errstrp, 0);
  if(errstrp) {
    return NDPI_CFG_INVALID_PARAM;
  }

  /* Min and max values are set in the code, so we can convert them
     to integers without too many checks...*/
  if(min_value && max_value &&
#ifndef __KERNEL__
     (val < strtol(min_value, NULL, 0) || val > strtol(max_value, NULL, 0))
#else
     (val < atoi(min_value) || val > atoi(max_value))
#endif
     )
    return NDPI_CFG_INVALID_PARAM;

  *variable = val;

  return NDPI_CFG_OK;
}

static ndpi_cfg_error _set_param_const_int(struct ndpi_detection_module_struct *ndpi_str,
                                                void *_variable, const char *value,
                                                const char *min_value, const char *max_value,
                                                const char *proto, const char *param)
{
  return NDPI_CFG_INVALID_PARAM;
}


/* ******************************************************************** */

static char *_get_param_int(void *_variable, const char *proto, char *buf, int buf_len) {
  int *variable = (int *)_variable;

  (void)proto;

  snprintf(buf, buf_len, "%d", *variable);
  buf[buf_len - 1] = '\0';
  return buf;
}

/* ******************************************************************** */

static char *_get_param_string(void *_variable, const char *proto, char *buf, int buf_len) {
  char *variable = (char *)_variable;

  (void)proto;

  snprintf(buf, buf_len, "%s", variable);
  buf[buf_len - 1] = '\0';
  return buf;
}

#ifndef __KERNEL__
/* ******************************************************************** */

static ndpi_cfg_error _set_param_filename(struct ndpi_detection_module_struct *ndpi_str,
                                          void *_variable, const char *value,
                                          const char *min_value, const char *max_value,
                                          const char *proto, const char *param) {
  char *variable = (char *)_variable;

  (void)ndpi_str;
  (void)min_value;
  (void)max_value;
  (void)proto;
  (void)param;

  if(value == NULL) { /* Valid value */
    variable[0] = '\0';
    return NDPI_CFG_OK;
  }

  if(access(value, F_OK) != 0)
    return NDPI_CFG_INVALID_PARAM;
  strncpy(variable, value, CFG_MAX_LEN);
  return NDPI_CFG_OK;
}
#endif

/* ******************************************************************** */

static ndpi_cfg_error _set_param_filename_config(struct ndpi_detection_module_struct *ndpi_str,
                                                 void *_variable, const char *value,
                                                 const char *min_value, const char *max_value,
                                                 const char *proto, const char *param) {
#ifndef __KERNEL__
  int rc;
  FILE *fd;

  rc = _set_param_filename(ndpi_str, _variable, value, min_value, max_value, proto, param);
  if(rc != 0 || value == NULL || ndpi_str == NULL)
    return rc;

  fd = fopen(value, "r");

  if(fd == NULL)
    return NDPI_CFG_INVALID_PARAM; /* It shoudn't happen because we already checked it */

  rc = load_config_file_fd(ndpi_str, fd);

  fclose(fd);

  if(rc < 0)
    return rc;
#endif

  return NDPI_CFG_OK;
}

/* ******************************************************************** */

static char *_get_param_protocol_enable_disable(void *_variable, const char *proto,
						char *buf, int buf_len)
{
  NDPI_PROTOCOL_BITMASK *bitmask = (NDPI_PROTOCOL_BITMASK *)_variable;
  u_int16_t proto_id;

  proto_id = __get_proto_id(proto);
  if(proto_id == NDPI_PROTOCOL_UNKNOWN)
    return NULL;

  snprintf(buf, buf_len, "%d", !!NDPI_ISSET(bitmask, proto_id));
  buf[buf_len - 1] = '\0';
  return buf;
}

static ndpi_cfg_error _set_param_protocol_enable_disable(struct ndpi_detection_module_struct *ndpi_str,
                                                         void *_variable, const char *value,
                                                         const char *min_value, const char *max_value,
                                                         const char *proto, const char *param)
{
  NDPI_PROTOCOL_BITMASK *bitmask = (NDPI_PROTOCOL_BITMASK *)_variable;
  u_int16_t proto_id;

  (void)ndpi_str;
  (void)min_value;
  (void)max_value;
  (void)param;

  if(strcmp(proto, "any") == 0 ||
     strcmp(proto, "all") == 0 ||
     strcmp(proto, "$PROTO_NAME_OR_ID") == 0) {
    if(strcmp(value, "1") == 0 ||
       strcmp(value, "enable") == 0) {
      NDPI_BITMASK_SET_ALL(*bitmask);
      return NDPI_CFG_OK;
    }
    if(strcmp(value, "0") == 0 ||
       strcmp(value, "disable") == 0) {
      NDPI_BITMASK_RESET(*bitmask);
      return NDPI_CFG_OK;
    }
  }

  proto_id = __get_proto_id(proto);
  if(proto_id == NDPI_PROTOCOL_UNKNOWN)
    return NDPI_CFG_INVALID_PARAM;

  if(strcmp(value, "1") == 0 ||
     strcmp(value, "enable") == 0) {
    NDPI_BITMASK_ADD(*bitmask, proto_id);
    return NDPI_CFG_OK;
  }
  if(strcmp(value, "0") == 0 ||
     strcmp(value, "disable") == 0) {
    NDPI_BITMASK_DEL(*bitmask, proto_id);
    return NDPI_CFG_OK;
  }
  return NDPI_CFG_INVALID_PARAM;
}

static char *_get_param_flowrisk_enable_disable(void *_variable, const char *proto,
                                                char *buf, int buf_len)
{
  NDPI_PROTOCOL_BITMASK *bitmask = (NDPI_PROTOCOL_BITMASK *)_variable;
  ndpi_risk_enum flowrisk_id;

  flowrisk_id = __get_flowrisk_id(proto);
  if(flowrisk_id == NDPI_NO_RISK)
    return NULL;

  snprintf(buf, buf_len, "%d", !!NDPI_ISSET(bitmask, flowrisk_id));
  buf[buf_len - 1] = '\0';
  return buf;
}

static ndpi_cfg_error _set_param_flowrisk_enable_disable(struct ndpi_detection_module_struct *ndpi_str,
                                                         void *_variable, const char *value,
                                                         const char *min_value, const char *max_value,
                                                         const char *proto, const char *_param)
{
  NDPI_PROTOCOL_BITMASK *bitmask = (NDPI_PROTOCOL_BITMASK *)_variable;
  ndpi_risk_enum flowrisk_id;
  char param[128] = {0};

  (void)ndpi_str;
  (void)min_value;
  (void)max_value;
  (void)proto;

  if(strncmp(_param, "flow_risk.", 10) != 0)
    return NDPI_CFG_INVALID_PARAM;

  _param += 10; /* Strip initial "flow_risk." */

  if(strlen(_param) > 5 &&
     strncmp(_param + (strlen(_param) - 5), ".info", 5) == 0)
    memcpy(param, _param, ndpi_min(strlen(_param) - 5, sizeof(param) - 1)); /* Strip trailing ".info" */
  else
    strncpy(param, _param, sizeof(param) - 1);

  if(strcmp(param, "any") == 0 ||
     strcmp(param, "all") == 0 ||
     strcmp(param, "$FLOWRISK_NAME_OR_ID") == 0) {
    if(strcmp(value, "1") == 0 ||
       strcmp(value, "enable") == 0) {
      NDPI_BITMASK_SET_ALL(*bitmask);
      return NDPI_CFG_OK;
    }
    if(strcmp(value, "0") == 0 ||
       strcmp(value, "disable") == 0) {
      NDPI_BITMASK_RESET(*bitmask);
      return NDPI_CFG_OK;
    }
  }

  flowrisk_id = __get_flowrisk_id(param);
  if(flowrisk_id == NDPI_NO_RISK)
    return NDPI_CFG_INVALID_PARAM;

  if(strcmp(value, "1") == 0 ||
     strcmp(value, "enable") == 0) {
    NDPI_BITMASK_ADD(*bitmask, flowrisk_id);
    return NDPI_CFG_OK;
  }
  if(strcmp(value, "0") == 0 ||
     strcmp(value, "disable") == 0) {
    NDPI_BITMASK_DEL(*bitmask, flowrisk_id);
    return NDPI_CFG_OK;
  }
  return NDPI_CFG_INVALID_PARAM;
}

static int clbk_only_with_global_ctx(struct ndpi_detection_module_struct *ndpi_str,
                                     void *_variable, const char *proto,
                                     const char *param)
{
  int *variable = (int *)_variable;

  (void)proto;
  (void)param;

  /* Integer set > 0 only if there is a global context */
  if(*variable > 0 && !ndpi_str->g_ctx) {
    *variable = 0;
    return -1;
  }
  return 0;
}


enum cfg_param_type {
  CFG_PARAM_ENABLE_DISABLE = 0,
  CFG_PARAM_INT,
  CFG_PARAM_PROTOCOL_ENABLE_DISABLE,
  CFG_PARAM_FILENAME_CONFIG, /* We call ndpi_set_config() immediately for each row in it */
  CFG_PARAM_CONST_INT,
  CFG_PARAM_CONST_FLAG,
  CFG_PARAM_FLOWRISK_ENABLE_DISABLE,
};

typedef ndpi_cfg_error (*cfg_set)(struct ndpi_detection_module_struct *ndpi_str,
                                  void *_variable, const char *value,
                                  const char *min_value, const char *max_value,
                                  const char *proto, const char *param);
typedef char *(*cfg_get)(void *_variable, const char *proto, char *buf, int buf_len);
typedef int (*cfg_calback)(struct ndpi_detection_module_struct *ndpi_str, void *_variable, const char *proto, const char *param);

static const struct cfg_op {
  enum cfg_param_type type;
  cfg_set fn_set;
  cfg_get fn_get;
} cfg_ops[] = {
  { CFG_PARAM_ENABLE_DISABLE,          _set_param_enable_disable,          _get_param_int },
  { CFG_PARAM_INT,                     _set_param_int,                     _get_param_int },
  { CFG_PARAM_PROTOCOL_ENABLE_DISABLE, _set_param_protocol_enable_disable, _get_param_protocol_enable_disable },
  { CFG_PARAM_FILENAME_CONFIG,         _set_param_filename_config,         _get_param_string },
  { CFG_PARAM_CONST_INT,               _set_param_const_int,               _get_param_int },
  { CFG_PARAM_CONST_FLAG,              _set_param_const_flag,              _get_param_int },
  { CFG_PARAM_FLOWRISK_ENABLE_DISABLE, _set_param_flowrisk_enable_disable, _get_param_flowrisk_enable_disable },
};

#define __OFF(a)	offsetof(struct ndpi_detection_module_config_struct, a)

static const struct cfg_param {
  char *proto;
  char *param;
  char *default_value;
  char *min_value;
  char *max_value;
  enum cfg_param_type type;
  int offset;
  cfg_calback fn_callback;
  int nonlocked;
} cfg_params[] = {
  /* Per-protocol parameters */

  { "http",          "metadata.request_content_type",           "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_request_content_type_enabled), NULL, 1 },
  { "http",          "metadata.referer",                        "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_referer_enabled), NULL, 1 },
  { "http",          "metadata.host",                           "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_host_enabled), NULL, 1 },
  { "http",          "metadata.username",                       "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_username_enabled), NULL, 1 },
  { "http",          "metadata.password",                       "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_password_enabled), NULL, 1 },

  { "tls",           "certificate_expiration_threshold",        "30", "0", "365", CFG_PARAM_INT, __OFF(tls_certificate_expire_in_x_days), NULL, 1 },
  { "tls",           "application_blocks_tracking",             "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_app_blocks_tracking_enabled), NULL, 0 },
  { "tls",           "dpi.heuristics",                          "0x00", "0", "0x07", CFG_PARAM_INT, __OFF(tls_heuristics), NULL , 1},
  { "tls",           "dpi.heuristics.max_packets_extra_dissection", "25", "0", "255", CFG_PARAM_INT, __OFF(tls_heuristics_max_packets), NULL, 1 },
  { "tls",           "metadata.sha1_fingerprint",               "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_sha1_fingerprint_enabled), NULL, 1 },
  { "tls",           "metadata.versions_supported",             "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_versions_supported_enabled), NULL, 1 },
  { "tls",           "metadata.alpn_negotiated",                "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_alpn_negotiated_enabled), NULL, 1 },
  { "tls",           "metadata.cipher",                         "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_cipher_enabled), NULL, 1 },
  { "tls",           "metadata.cert_server_names",              "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_cert_server_names_enabled), NULL, 1 },
  { "tls",           "metadata.cert_validity",                  "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_cert_validity_enabled), NULL, 1 },
  { "tls",           "metadata.cert_issuer",                    "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_cert_issuer_enabled), NULL, 1 },
  { "tls",           "metadata.cert_subject",                   "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_cert_subject_enabled), NULL, 1 },
  { "tls",           "metadata.browser",                        "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_broswer_enabled), NULL, 1 },
  { "tls",           "metadata.ja3s_fingerprint",               "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_ja3s_fingerprint_enabled), NULL, 1 },
  { "tls",           "metadata.ja4c_fingerprint",               "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_ja4c_fingerprint_enabled), NULL, 1 },
  { "tls",           "metadata.ja4r_fingerprint",               "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_ja4r_fingerprint_enabled), NULL, 1 },
  { "tls",           "subclassification",                       "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_subclassification_enabled), NULL, 1 },
  { "tls",           "subclassification_cert",                  "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tls_subclassification_cert_enabled), NULL, 1 },
  { "tls",           "mem_buf_size_limit",                      "16384", "0", "32768", CFG_PARAM_INT, __OFF(tls_buf_size_limit), NULL, 1 },

  { "quic",          "subclassification",                       "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(quic_subclassification_enabled), NULL, 1 },

  { "smtp",          "tls_dissection",                          "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(smtp_opportunistic_tls_enabled), NULL, 1 },

  { "imap",          "tls_dissection",                          "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(imap_opportunistic_tls_enabled), NULL, 1 },

  { "pop",           "tls_dissection",                          "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(pop_opportunistic_tls_enabled), NULL, 1 },

  { "ftp",           "tls_dissection",                          "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(ftp_opportunistic_tls_enabled), NULL },

  { "sip",           "metadata.attribute.from",                 "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(sip_attribute_from_enabled), NULL, 1 },
  { "sip",           "metadata.attribute.from_imsi",            "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(sip_attribute_from_imsi_enabled), NULL, 1 },
  { "sip",           "metadata.attribute.to",                   "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(sip_attribute_to_enabled), NULL, 1 },
  { "sip",           "metadata.attribute.to_imsi",              "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(sip_attribute_to_imsi_enabled), NULL, 1 },

  { "stun",          "tls_dissection",                          "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(stun_opportunistic_tls_enabled), NULL, 1 },
  { "stun",          "max_packets_extra_dissection",            "6", "0", "255", CFG_PARAM_INT, __OFF(stun_max_packets_extra_dissection), NULL, 1 },
  { "stun",          "metadata.attribute.mapped_address",       "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(stun_mapped_address_enabled), NULL, 1 },
  { "stun",          "metadata.attribute.response_origin",      "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(stun_response_origin_enabled), NULL, 1 },
  { "stun",          "metadata.attribute.other_address",        "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(stun_other_address_enabled), NULL, 1 },
  { "stun",          "metadata.attribute.relayed_address",      "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(stun_relayed_address_enabled), NULL, 1 },
  { "stun",          "metadata.attribute.peer_address",         "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(stun_peer_address_enabled), NULL, 1 },

  { "bittorrent",    "metadata.hash",                           "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(bittorrent_hash_enabled), NULL, 1 },

  { "ssdp",          "metadata",                                "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(ssdp_metadata_enabled), NULL, 1 },

  { "dns",           "subclassification",                       "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(dns_subclassification_enabled), NULL, 1 },
  { "dns",           "process_response",                        "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(dns_parse_response_enabled), NULL, 1 },

  { "http",          "process_response",                        "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_parse_response_enabled), NULL, 1 },
  { "http",          "subclassification",                       "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(http_subclassification_enabled), NULL, 1 },

  { "ookla",         "dpi.aggressiveness",                      "0x01", "0", "1", CFG_PARAM_INT, __OFF(ookla_aggressiveness), NULL, 1 },

  { "zoom",          "max_packets_extra_dissection",            "4", "0", "255", CFG_PARAM_INT, __OFF(zoom_max_packets_extra_dissection), NULL, 1 },

  { "zoom",          "max_packets_extra_dissection",            "4", "0", "255", CFG_PARAM_INT, __OFF(zoom_max_packets_extra_dissection), NULL, 1 },

  { "rtp",           "search_for_stun",                         "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(rtp_search_for_stun), NULL, 1 },
  { "rtp",           "max_packets_extra_dissection",            "32", "0", "255", CFG_PARAM_INT, __OFF(rtp_max_packets_extra_dissection), NULL, 1 },
  
  { "openvpn",       "dpi.heuristics",                          "0x00", "0", "0x01", CFG_PARAM_INT, __OFF(openvpn_heuristics), NULL, 1 },
  { "openvpn",       "dpi.heuristics.num_messages",             "10", "0", "255", CFG_PARAM_INT, __OFF(openvpn_heuristics_num_msgs), NULL, 1 },
  { "openvpn",       "subclassification_by_ip",                 "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(openvpn_subclassification_by_ip), NULL, 1 },

  { "wireguard",     "subclassification_by_ip",                 "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(wireguard_subclassification_by_ip), NULL, 1 },

  { "$PROTO_NAME_OR_ID", "log",                                 "disable", NULL, NULL, CFG_PARAM_PROTOCOL_ENABLE_DISABLE, __OFF(debug_bitmask), NULL, 1 },
  { "$PROTO_NAME_OR_ID", "ip_list.load",                        "1", NULL, NULL, CFG_PARAM_PROTOCOL_ENABLE_DISABLE, __OFF(ip_list_bitmask), NULL, 0 },
  { "$PROTO_NAME_OR_ID", "monitoring",                          "disable", NULL, NULL, CFG_PARAM_PROTOCOL_ENABLE_DISABLE, __OFF(monitoring), NULL, 1 },

  /* Global parameters */

  { NULL,            "packets_limit_per_flow",                  "32", "0", "255", CFG_PARAM_INT, __OFF(max_packets_to_process), NULL, 1 },
  { NULL,            "flow.direction_detection",                "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(direction_detect_enabled), NULL, 0 },
  { NULL,            "flow.track_payload",                      "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(track_payload_enabled), NULL, 0 },
  { NULL,            "flow.use_client_ip_in_guess",             "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(use_client_ip_in_guess), NULL, 1},
  { NULL,            "flow.use_client_port_in_guess",           "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(use_client_port_in_guess), NULL, 1},
  { NULL,            "tcp_ack_payload_heuristic",               "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tcp_ack_paylod_heuristic), NULL, 1 },
  { NULL,            "fully_encrypted_heuristic",               "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(fully_encrypted_heuristic), NULL, 1 },
  { NULL,            "libgcrypt.init",                          "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(libgcrypt_init), NULL, 0 },
  { NULL,            "dpi.guess_on_giveup",                     "0x3", "0", "3", CFG_PARAM_INT, __OFF(guess_on_giveup), NULL, 1 },
  { NULL,            "dpi.guess_ip_before_port",                "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(guess_ip_before_port), NULL, 1},
  { NULL,            "dpi.compute_entropy",                     "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(compute_entropy), NULL, 0 },
  { NULL,            "dpi.address_cache_size",                  "0", "0", "16777215", CFG_PARAM_INT, __OFF(address_cache_size), NULL, 0 },
  { NULL,            "fpc",                                     "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(fpc_enabled), NULL, 1 },

  { NULL,            "metadata.tcp_fingerprint",                "enable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tcp_fingerprint_enabled), NULL, 1 },
  { NULL,            "metadata.tcp_fingerprint_raw",            "disable", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(tcp_fingerprint_raw_enabled), NULL, 1 },
  { NULL,            "metadata.tcp_fingerprint_format",         "0", "0" /* min */, "1" /* max */, CFG_PARAM_INT, __OFF(tcp_fingerprint_format), NULL, 1 },

  { NULL,            "flow_risk_lists.load",                    "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(flow_risk_lists_enabled), NULL, 0 },

  { NULL,            "flow_risk.$FLOWRISK_NAME_OR_ID",          "enable", NULL, NULL, CFG_PARAM_FLOWRISK_ENABLE_DISABLE, __OFF(flowrisk_bitmask), NULL, 0 },
  { NULL,            "flow_risk.$FLOWRISK_NAME_OR_ID.info",     "enable", NULL, NULL, CFG_PARAM_FLOWRISK_ENABLE_DISABLE, __OFF(flowrisk_info_bitmask), NULL, 0 },

  { NULL,            "flow_risk.anonymous_subscriber.list.icloudprivaterelay.load", "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(risk_anonymous_subscriber_list_icloudprivaterelay_enabled), NULL, 0 },
  { NULL,            "flow_risk.anonymous_subscriber.list.tor.load",                "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(risk_anonymous_subscriber_list_tor_exit_nodes_enabled), NULL, 0 },
  { NULL,            "flow_risk.crawler_bot.list.load",                             "1", NULL, NULL, CFG_PARAM_ENABLE_DISABLE, __OFF(risk_crawler_bot_list_enabled), NULL, 0 },

  { NULL,            "filename.config",                         NULL, NULL, NULL, CFG_PARAM_FILENAME_CONFIG, __OFF(filename_config), NULL, 0 },

  { NULL,            "log.level",                               "0", "0", "3", CFG_PARAM_INT, __OFF(log_level), NULL, 1 },

  /* LRU caches */

  { NULL,            "lru.ookla.size",                          "1024", "0", "16777215", CFG_PARAM_INT, __OFF(ookla_cache_num_entries), NULL, 0 },
  { NULL,            "lru.ookla.ttl",                           "120", "0", "16777215", CFG_PARAM_INT, __OFF(ookla_cache_ttl), NULL, 0 },
  { NULL,            "lru.ookla.scope",                         "0", "0", "1", CFG_PARAM_INT, __OFF(ookla_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL,            "lru.bittorrent.size",                     "32768", "0", "16777215", CFG_PARAM_INT, __OFF(bittorrent_cache_num_entries), NULL, 0 },
  { NULL,            "lru.bittorrent.ttl",                      "300", "0", "16777215", CFG_PARAM_INT, __OFF(bittorrent_cache_ttl), NULL, 0 },
  { NULL,            "lru.bittorrent.scope",                    "0", "0", "1", CFG_PARAM_INT, __OFF(bittorrent_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL,            "lru.stun.size",                           "1024", "0", "16777215", CFG_PARAM_INT, __OFF(stun_cache_num_entries), NULL, 0 },
  { NULL,            "lru.stun.ttl",                            "300", "0", "16777215", CFG_PARAM_INT, __OFF(stun_cache_ttl), NULL, 0 },
  { NULL,            "lru.stun.scope",                          "0", "0", "1", CFG_PARAM_INT, __OFF(stun_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL,            "lru.tls_cert.size",                       "1024", "0", "16777215", CFG_PARAM_INT, __OFF(tls_cert_cache_num_entries), NULL, 0 },
  { NULL,            "lru.tls_cert.ttl",                        "300", "0", "16777215", CFG_PARAM_INT, __OFF(tls_cert_cache_ttl), NULL, 0 },
  { NULL,            "lru.tls_cert.scope",                      "0", "0", "1", CFG_PARAM_INT, __OFF(tls_cert_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL,            "lru.mining.size",                         "1024", "0", "16777215", CFG_PARAM_INT, __OFF(mining_cache_num_entries), NULL, 0 },
  { NULL,            "lru.mining.ttl",                          "300", "0", "16777215", CFG_PARAM_INT, __OFF(mining_cache_ttl), NULL, 0 },
  { NULL,            "lru.mining.scope",                        "0", "0", "1", CFG_PARAM_INT, __OFF(mining_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL,            "lru.mining.size",                         "1024", "0", "16777215", CFG_PARAM_INT, __OFF(mining_cache_num_entries), NULL , 0 },
  { NULL,            "lru.mining.ttl",                          "0", "0", "16777215", CFG_PARAM_INT, __OFF(mining_cache_ttl), NULL , 0 },
  { NULL,            "lru.mining.scope",                        "0", "0", "1", CFG_PARAM_INT, __OFF(mining_cache_scope), clbk_only_with_global_ctx , 0 },

  { NULL,            "lru.msteams.size",                        "1024", "0", "16777215", CFG_PARAM_INT, __OFF(msteams_cache_num_entries), NULL , 0 },
  { NULL,            "lru.msteams.ttl",                         "60", "0", "16777215", CFG_PARAM_INT, __OFF(msteams_cache_ttl), NULL , 0 },
  { NULL,            "lru.msteams.scope",                       "0", "0", "1", CFG_PARAM_INT, __OFF(msteams_cache_scope), clbk_only_with_global_ctx , 0 },

  { NULL,            "lru.fpc_dns.size",                        "1024", "0", "16777215", CFG_PARAM_INT, __OFF(fpc_dns_cache_num_entries), NULL, 0 },
  { NULL,            "lru.fpc_dns.ttl",                         "60", "0", "16777215", CFG_PARAM_INT, __OFF(fpc_dns_cache_ttl), NULL, 0 },
  { NULL,            "lru.fpc_dns.scope",                       "0", "0", "1", CFG_PARAM_INT, __OFF(fpc_dns_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL,            "lru.signal.size",                         "32768", "0", "16777215", CFG_PARAM_INT, __OFF(signal_cache_num_entries), NULL, 0 },
  { NULL,            "lru.signal.ttl",                          "300", "0", "16777215", CFG_PARAM_INT, __OFF(signal_cache_ttl), NULL, 0 },
  { NULL,            "lru.signal.scope",                        "0", "0", "1", CFG_PARAM_INT, __OFF(signal_cache_scope), clbk_only_with_global_ctx, 0 },

  { NULL, NULL, NULL, NULL, NULL, 0, -1, NULL },
};

#undef __OFF

static void set_default_config(struct ndpi_detection_module_config_struct *cfg)
{
  const struct cfg_param *c;

  for(c = &cfg_params[0]; c && c->param; c++) {
    cfg_ops[c->type].fn_set(NULL, (void *)((char *)cfg + c->offset),
                            c->default_value, c->min_value, c->max_value, c->proto, c->param);
  }
}

ndpi_cfg_error ndpi_set_config(struct ndpi_detection_module_struct *ndpi_str,
		               const char *proto, const char *param, const char *value)
{
  const struct cfg_param *c;
  ndpi_cfg_error rc;
  int ret;

  if(!ndpi_str || !param || !value)
    return NDPI_CFG_INVALID_CONTEXT;

  NDPI_LOG_DBG(ndpi_str, "Set [%s][%s][%s]\n", proto, param, value);

  if(proto && (strcmp(proto, "NULL") == 0))
    proto = NULL;

  for(c = &cfg_params[0]; c && c->param; c++) {
    if((((proto == NULL && c->proto == NULL) ||
	 (proto && c->proto && strcmp(proto, c->proto) == 0)) &&
        strcmp(param, c->param) == 0) ||
       (proto && c->proto &&
	strcmp(c->proto, "$PROTO_NAME_OR_ID") == 0 &&
	strcmp(param, c->param) == 0) ||
       (proto == NULL && c->proto == NULL &&
	strncmp(c->param, "flow_risk.$FLOWRISK_NAME_OR_ID", 30) == 0 &&
	strncmp(param, "flow_risk.", 10) == 0 &&
	!ndpi_str_endswith(param, ".info") &&
	!ndpi_str_endswith(param, ".load")) ||
       (proto == NULL && c->proto == NULL &&
	strncmp(c->param, "flow_risk.$FLOWRISK_NAME_OR_ID.info", 35) == 0 &&
	strncmp(param, "flow_risk.", 10) == 0 &&
	ndpi_str_endswith(param, ".info"))) {

      if(!c->nonlocked && ndpi_str->finalized)
	return NDPI_CFG_CONTEXT_ALREADY_INITIALIZED;
      if(c->type == CFG_PARAM_CONST_FLAG || 
	 c->type == CFG_PARAM_CONST_INT)
	      return NDPI_CFG_INVALID_PARAM;

      rc = cfg_ops[c->type].fn_set(ndpi_str, (void *)((char *)&ndpi_str->cfg + c->offset),
                                   value, c->min_value, c->max_value, proto, param);
      if(rc == NDPI_CFG_OK && c->fn_callback) {
        ret = c->fn_callback(ndpi_str, (void *)((char *)&ndpi_str->cfg + c->offset),
                             proto, param);
        if(ret < 0)
          rc = NDPI_CFG_CALLBACK_ERROR;
        else
          rc = ret;
      }
      return rc;
    }
  }
  return NDPI_CFG_NOT_FOUND;
}

ndpi_cfg_error ndpi_set_config_u64(struct ndpi_detection_module_struct *ndpi_str,
                                   const char *proto, const char *param, uint64_t value)
{
  char value_str[21];
  int value_len;

  value_len = ndpi_snprintf(value_str, sizeof(value_str), "%llu", (unsigned long long int)value);
  if (value_len <= 0 || value_len >= (int)sizeof(value_str))
    {
      return NDPI_CFG_INVALID_PARAM;
    }

  return ndpi_set_config(ndpi_str, proto, param, value_str);
}

char *ndpi_get_config(struct ndpi_detection_module_struct *ndpi_str,
		      const char *proto, const char *param, char *buf, int buf_len)
{
  const struct cfg_param *c;

  if(!ndpi_str || !param || !buf || buf_len <= 0)
    return NULL;

  NDPI_LOG_DBG(ndpi_str, "Get [%s][%s]\n", proto, param);

  for(c = &cfg_params[0]; c && c->param; c++) {
    if((((proto == NULL && c->proto == NULL) ||
	 (proto && c->proto && strcmp(proto, c->proto) == 0)) &&
        strcmp(param, c->param) == 0) ||
       (proto && c->proto &&
	strcmp(c->proto, "$PROTO_NAME_OR_ID") == 0 &&
	strcmp(param, c->param) == 0) ||
       (proto == NULL && c->proto == NULL &&
	strcmp(c->param, "flow_risk.$FLOWRISK_NAME_OR_ID") == 0 &&
	strcmp(param, c->param) == 0)) {

      return cfg_ops[c->type].fn_get((void *)((char *)&ndpi_str->cfg + c->offset), proto, buf, buf_len);
    }
  }
  return NULL;
}

static const char *config_header = "#Protocol (empty/NULL for global knobs), parameter, value, [default value], [min value, max_value], type\n";
char *ndpi_dump_config_str(struct ndpi_detection_module_struct *ndpi_str,
		       char *output, int *size)
{
  const struct cfg_param *c;
  int l,la,len;
  char buf[64],lbuf[256];

  if(output && *size > 0)
	*output = '\0';
  if(!ndpi_str) 
    return output;

  l = 0;
  if(output)
      l += snprintf(output, *size, "%s",config_header);
    else
      l += strlen(config_header);

  /* TODO */
  for(c = &cfg_params[0]; c && c->param; c++) {
    la = 0;
    lbuf[0] = '\0';
    switch(c->type) {
    case CFG_PARAM_ENABLE_DISABLE:
    case CFG_PARAM_INT:
    case CFG_PARAM_CONST_FLAG:
    case CFG_PARAM_CONST_INT:
      if(c->proto)
	 la = snprintf(lbuf,sizeof(lbuf)-2,"%s ",c->proto);
      la += snprintf(&lbuf[la],sizeof(lbuf)-la-2,"%s: %s [%s]",
              c->param,
	      _get_param_int((void *)((char *)&ndpi_str->cfg + c->offset), c->proto, buf, sizeof(buf)),
	      c->default_value);
      if(c->min_value && c->max_value)
        la += snprintf(&lbuf[la], sizeof(lbuf)-2-la, " [%s-%s]", c->min_value, c->max_value);

      if(c->type == CFG_PARAM_CONST_FLAG || c->type == CFG_PARAM_CONST_INT)
          la += snprintf(&lbuf[la], sizeof(lbuf)-2-la, " [const]\n");
        else
          la += snprintf(&lbuf[la], sizeof(lbuf)-2-la, " [%s]\n",
		      c->nonlocked ? "var": ( ndpi_str->finalized ? "final" : "var"));
      break;
    case CFG_PARAM_FILENAME_CONFIG:
#ifndef __KERNEL__
      if(c->proto)
	 la = snprintf(lbuf,sizeof(lbuf)-2,"%s ",c->proto);
      la += snprintf(&lbuf[la],sizeof(lbuf)-la-2, "%s: %s [%s]\n",
              c->param,
	      _get_param_string((void *)((char *)&ndpi_str->cfg + c->offset), c->proto, buf, sizeof(buf)),
	      c->default_value);
#endif
      break;
    /* TODO */
    case CFG_PARAM_PROTOCOL_ENABLE_DISABLE:
      if(c->proto)
	 la = snprintf(lbuf,sizeof(lbuf)-2,"%s ",c->proto);
      la += snprintf(&lbuf[la],sizeof(lbuf)-la-2, "%s: %s [all %s]",
              c->param,
              /* TODO */ _get_param_protocol_enable_disable((void *)((char *)&ndpi_str->cfg + c->offset), "any", buf, sizeof(buf)),
	      c->default_value);
      la += snprintf(&lbuf[la], sizeof(lbuf)-2-la, " [%s]\n",
	      ndpi_str->finalized ? "final" : "var");
      
      break;
    case CFG_PARAM_FLOWRISK_ENABLE_DISABLE:
      if(c->proto)
	 la = snprintf(lbuf,sizeof(lbuf)-2,"%s ",c->proto);
      la += snprintf(&lbuf[la],sizeof(lbuf)-la-2,  "%s: %s [all %s]",
	      c->param,
	      /* TODO */ _get_param_flowrisk_enable_disable((void *)((char *)&ndpi_str->cfg + c->offset), "any", buf, sizeof(buf)),
	      c->default_value);
      la += snprintf(&lbuf[la], sizeof(lbuf)-2-la, " [%s]\n",
	      ndpi_str->finalized ? "final" : "var");
       
      break;
    }
    if(!la) continue;
    lbuf[la] = '\0';
    if(output) {
       len = ndpi_min(la,*size - l);
       strncpy(output + l,lbuf,len);
       l += len;
    } else {
       l += la;
    }
  }
  if(!output) {
	*size = l;
  }  else {
	output[l] = '\0';
  }
  return output;
}
#ifndef __KERNEL__
char *ndpi_dump_config(struct ndpi_detection_module_struct *ndpi_str, FILE *fd)
{
  char *buf = NULL;
  size_t buf_len = 0;
  ndpi_dump_config_str(ndpi_str,NULL,(int *)&buf_len);
  buf = ndpi_malloc(buf_len + 4);
  if(buf) {
	  ndpi_dump_config_str(ndpi_str,buf,(int *)&buf_len);
	  fprintf(fd,"%s",buf);
	  ndpi_free(buf);
  }
  return NULL;
}
#endif

void* ndpi_memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len) {
  if (!haystack || !needle || haystack_len < needle_len) {
    return NULL;
  }

  if (needle_len == 0) {
    return (void *)haystack;
  }

  if (needle_len == 1) {
    return (void *)memchr(haystack, *(const u_int8_t *)needle, haystack_len);
  }

  const u_int8_t *const end_of_search = (const u_int8_t *)haystack + haystack_len - needle_len + 1;

  const u_int8_t *curr = (const u_int8_t *)haystack;

  while (1) {
    /* Find the first occurrence of the first character from the needle */
    curr = (const u_int8_t *)memchr(curr, *(const u_int8_t *)needle, end_of_search - curr);

    if (!curr) {
      return NULL;
    }

    /* Check the rest of the needle for a match */
    if (memcmp(curr, needle, needle_len) == 0) {
      return (void *)curr;
    }

    /* Shift one character to the right for the next search */
    curr++;
  }

  return NULL;
}

size_t ndpi_strlcpy(char *dst, const char* src, size_t dst_len, size_t src_len) {
  if (!dst || !src || dst_len == 0) {
    return 0;
  }

  size_t copy_len = ndpi_min(src_len, dst_len - 1);
  memmove(dst, src, copy_len);
  dst[copy_len] = '\0';

  return src_len;
}

int ndpi_memcasecmp(const void *s1, const void *s2, size_t n) {
  if (s1 == NULL && s2 == NULL) {
    return 0;
  }

  if (s1 == NULL) {
    return -1;
  }

  if (s2 == NULL) {
    return 1;
  }

  if (n == 0) {
    return 0;
  }

  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  if (n == 1) {
    return tolower(*p1) - tolower(*p2);
  }

  /* Early exit optimization - check first and last bytes */

  int first_cmp = tolower(p1[0]) - tolower(p2[0]);
  if (first_cmp != 0) {
    return first_cmp;
  }

  int last_cmp = tolower(p1[n-1]) - tolower(p2[n-1]);
  if (last_cmp != 0) {
    return last_cmp;
  }

  size_t i;
  for (i = 1; i < n-1; i++) {
    int cmp = tolower(p1[i]) - tolower(p2[i]);
    if (cmp != 0) {
      return cmp;
    }
  }

  return 0;
}
