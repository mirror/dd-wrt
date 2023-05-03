/*
 * ndpi_main.h
 *
 * Copyright (C) 2011-22 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
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

#ifndef __NDPI_MAIN_H__
#define __NDPI_MAIN_H__
#ifdef HAVE_CONFIG_H
#include "ndpi_config.h"
#endif
#include "ndpi_includes.h"
#ifdef NDPI_LIB_COMPILATION
/* for macros NDPI_LOG_* in ndpi_define.h */
#include "ndpi_config.h"
#endif
#include "ndpi_define.h"
#include "ndpi_protocol_ids.h"

#include "ndpi_kernel_compat.h"

#include "ndpi_typedefs.h"
#include "ndpi_api.h"
#include "ndpi_protocols.h"

/* used by ndpi_set_proto_subprotocols */
#define NDPI_PROTOCOL_NO_MORE_SUBPROTOCOLS (-1)
#define NDPI_PROTOCOL_MATCHED_BY_CONTENT (-2)

#ifdef __cplusplus
extern "C" {
#endif

NDPI_STATIC  void *ndpi_tdelete(const void * __restrict, void ** __restrict,
		     int (*)(const void *, const void *));
NDPI_STATIC  void *ndpi_tfind(const void *, void *, int (*)(const void *, const void *));
NDPI_STATIC  void *ndpi_tsearch(const void *, void**, int (*)(const void *, const void *));
NDPI_STATIC  void ndpi_twalk(const void *, void (*)(const void *, ndpi_VISIT, int, void*), void *user_data);
NDPI_STATIC  void ndpi_tdestroy(void *vrootp, void (*freefct)(void *));

NDPI_STATIC  int NDPI_BITMASK_COMPARE(NDPI_PROTOCOL_BITMASK a, NDPI_PROTOCOL_BITMASK b);
NDPI_STATIC  int NDPI_BITMASK_IS_EMPTY(NDPI_PROTOCOL_BITMASK a);
NDPI_STATIC  void NDPI_DUMP_BITMASK(NDPI_PROTOCOL_BITMASK a);

  NDPI_STATIC u_int8_t ndpi_net_match(u_int32_t ip_to_check,
				 u_int32_t net,
				 u_int32_t num_bits);

  NDPI_STATIC u_int8_t ndpi_ips_match(u_int32_t src, u_int32_t dst,
				 u_int32_t net, u_int32_t num_bits);

NDPI_STATIC  u_int16_t ntohs_ndpi_bytestream_to_number(const u_int8_t * str,
					    u_int16_t max_chars_to_read,
					    u_int16_t * bytes_read);

NDPI_STATIC  u_int32_t ndpi_bytestream_to_number(const u_int8_t * str, u_int16_t max_chars_to_read,
				      u_int16_t * bytes_read);
NDPI_STATIC  u_int64_t ndpi_bytestream_to_number64(const u_int8_t * str, u_int16_t max_chars_to_read,
					u_int16_t * bytes_read);
NDPI_STATIC  u_int32_t ndpi_bytestream_dec_or_hex_to_number(const u_int8_t * str,
						 u_int16_t max_chars_to_read,
						 u_int16_t * bytes_read);
NDPI_STATIC  u_int64_t ndpi_bytestream_dec_or_hex_to_number64(const u_int8_t * str,
						   u_int16_t max_chars_to_read,
						   u_int16_t * bytes_read);
NDPI_STATIC  u_int32_t ndpi_bytestream_to_ipv4(const u_int8_t * str, u_int16_t max_chars_to_read,
				    u_int16_t * bytes_read);

NDPI_STATIC  void ndpi_set_detected_protocol(struct ndpi_detection_module_struct *ndpi_struct,
				  struct ndpi_flow_struct *flow,
				  u_int16_t upper_detected_protocol,
				  u_int16_t lower_detected_protocol,
				  ndpi_confidence_t confidence);

  NDPI_STATIC void ndpi_set_detected_protocol_keeping_master(struct ndpi_detection_module_struct *ndpi_str,
						 struct ndpi_flow_struct *flow,
						 u_int16_t detected_protocol,
						 ndpi_confidence_t confidence);

  NDPI_STATIC void ndpi_parse_packet_line_info(struct ndpi_detection_module_struct *ndpi_struct,
					  struct ndpi_flow_struct *flow);
  NDPI_STATIC void ndpi_parse_packet_line_info_any(struct ndpi_detection_module_struct *ndpi_struct,
					      struct ndpi_flow_struct *flow);

  NDPI_STATIC u_int16_t ndpi_check_for_email_address(struct ndpi_detection_module_struct *ndpi_struct,
						u_int16_t counter);

  NDPI_STATIC void ndpi_int_change_category(struct ndpi_detection_module_struct *ndpi_struct,
				       struct ndpi_flow_struct *flow,
				       ndpi_protocol_category_t protocol_category);

  NDPI_STATIC void ndpi_set_proto_subprotocols(struct ndpi_detection_module_struct *ndpi_mod,
				      int protoId, ...);

  NDPI_STATIC int ndpi_packet_src_ip_eql(const struct ndpi_packet_struct *packet, const ndpi_ip_addr_t * ip);
  NDPI_STATIC int ndpi_packet_dst_ip_eql(const struct ndpi_packet_struct *packet, const ndpi_ip_addr_t * ip);
  NDPI_STATIC void ndpi_packet_src_ip_get(const struct ndpi_packet_struct *packet, ndpi_ip_addr_t * ip);
  NDPI_STATIC void ndpi_packet_dst_ip_get(const struct ndpi_packet_struct *packet, ndpi_ip_addr_t * ip);

  NDPI_STATIC int ndpi_parse_ip_string(const char *ip_str, ndpi_ip_addr_t *parsed_ip);
  NDPI_STATIC char *ndpi_get_ip_string(const ndpi_ip_addr_t * ip, char *buf, u_int buf_len);
  NDPI_STATIC u_int8_t ndpi_is_ipv6(const ndpi_ip_addr_t *ip);

  NDPI_STATIC char* ndpi_get_proto_by_id(struct ndpi_detection_module_struct *ndpi_mod, u_int id);
//  NDPI_STATIC u_int16_t ndpi_get_proto_by_name(struct ndpi_detection_module_struct *ndpi_mod, const char *name);

  NDPI_STATIC u_int16_t ndpi_guess_protocol_id(struct ndpi_detection_module_struct *ndpi_struct,
					  struct ndpi_flow_struct *flow,
					  u_int8_t proto, u_int16_t sport, u_int16_t dport,
					  u_int8_t *user_defined_proto);

  NDPI_STATIC u_int8_t ndpi_is_proto(ndpi_protocol proto, u_int16_t p);

  NDPI_STATIC void ndpi_search_tcp_or_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
NDPI_STATIC  void ndpi_debug_get_last_log_function_line(struct ndpi_detection_module_struct *ndpi_struct,
					     const char **file, const char **func, u_int32_t * line);
#endif

  /** Checks when the @p payload starts with the string literal @p str.
   * When the string is larger than the payload, check fails.
   * @return non-zero if check succeeded
   */
NDPI_STATIC  int ndpi_match_prefix(const u_int8_t *payload, size_t payload_len,
			const char *str, size_t str_len);

 void gettimeofday64(struct timespec64 *, void * );
  /* version of ndpi_match_prefix with string literal */
#define ndpi_match_strprefix(payload, payload_len, str)			\
	ndpi_match_prefix((payload), (payload_len), (str), (sizeof(str)-1))

NDPI_STATIC  int ndpi_handle_ipv6_extension_headers(u_int16_t l3len,
					 const u_int8_t ** l4ptr, u_int16_t * l4len,
					 u_int8_t * nxt_hdr);
  
NDPI_STATIC  void ndpi_set_proto_defaults(struct ndpi_detection_module_struct *ndpi_str,
			       u_int8_t is_cleartext, u_int8_t is_app_protocol,
			       ndpi_protocol_breed_t breed,
			       u_int16_t protoId, char *protoName,
			       ndpi_protocol_category_t protoCategory,
			       ndpi_port_range *tcpDefPorts,
			       ndpi_port_range *udpDefPorts);    
NDPI_STATIC  void ndpi_set_risk(struct ndpi_detection_module_struct *ndpi_str,
		     struct ndpi_flow_struct *flow, ndpi_risk_enum r,
		     char *risk_message);
NDPI_STATIC  void ndpi_unset_risk(struct ndpi_detection_module_struct *ndpi_str,
		       struct ndpi_flow_struct *flow, ndpi_risk_enum r);    
NDPI_STATIC  int ndpi_isset_risk(struct ndpi_detection_module_struct *ndpi_str,
		      struct ndpi_flow_struct *flow, ndpi_risk_enum r);
NDPI_STATIC  int ndpi_is_printable_buffer(u_int8_t const * const buf, size_t len);
NDPI_STATIC  int ndpi_normalize_printable_string(char * const str, size_t len);
NDPI_STATIC  int ndpi_is_valid_hostname(char * const str, size_t len);
#define NDPI_ENTROPY_ENCRYPTED_OR_RANDOM(entropy) (entropy > 7.0f)
NDPI_STATIC  float ndpi_entropy(u_int8_t const * const buf, size_t len);
NDPI_STATIC  u_int16_t ndpi_calculate_icmp4_checksum(u_int8_t const * const buf, size_t len);
NDPI_STATIC  void load_common_alpns(struct ndpi_detection_module_struct *ndpi_str);
NDPI_STATIC  u_int8_t is_a_common_alpn(struct ndpi_detection_module_struct *ndpi_str,
			    const char *alpn_to_check, u_int alpn_to_check_len);    

NDPI_STATIC  char *ndpi_hostname_sni_set(struct ndpi_flow_struct *flow, const u_int8_t *value, size_t value_len);
NDPI_STATIC  char *ndpi_user_agent_set(struct ndpi_flow_struct *flow, const u_int8_t *value, size_t value_len);

  NDPI_STATIC int64_t ndpi_asn1_ber_decode_length(const unsigned char *payload, int payload_len, u_int16_t *value_len);
  NDPI_STATIC char* ndpi_intoav4(unsigned int addr, char* buf, u_int16_t bufLen);
  NDPI_STATIC int ndpi_current_pkt_from_client_to_server(const struct ndpi_packet_struct *packet, const struct ndpi_flow_struct *flow);
  NDPI_STATIC int ndpi_current_pkt_from_server_to_client(const struct ndpi_packet_struct *packet, const struct ndpi_flow_struct *flow);
  NDPI_STATIC int ndpi_seen_flow_beginning(const struct ndpi_flow_struct *flow);

#ifdef __cplusplus
}
#endif

#endif	/* __NDPI_MAIN_H__ */
