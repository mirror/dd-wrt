/*
 * someip.c
 *
 * Copyright (C) 2016 Sorin Zamfir <sorin.zamfir@yahoo.com>
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your omessage_typeion) any later version.
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

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_SOMEIP

#include "ndpi_api.h"
#include "ndpi_private.h"

enum SOMEIP_MESSAGE_TYPES {
  SOMEIP_REQUEST = 0x00,
  SOMEIP_REQUEST_NO_RETURN = 0x01,
  SOMEIP_NOTIFICATION = 0x02,
  SOMEIP_REQUEST_ACK = 0x40,
  SOMEIP_REQUEST_NO_RETURN_ACK = 0x41,
  SOMEIP_NOTIFICATION_ACK = 0x42,
  SOMEIP_RESPONSE = 0x80,
  SOMEIP_ERROR = 0x81,
  SOMEIP_RESPONSE_ACK = 0xc0,
  SOMEIP_ERROR_ACK = 0xc1
};

enum SOMEIP_RETURN_CODES {
  E_OK = 0x00,
  E_NOT_OK = 0x01,
  E_UNKNOWN_SERVICE = 0x02,
  E_UNKNOWN_METHOD = 0x03,
  E_NOT_READY = 0x04,
  E_NOT_REACHABLE = 0x05,
  E_TIMEOUT = 0x06,
  E_WRONG_PROTOCOL_VERSION = 0x07,
  E_WRONG_INTERFACE_VERSION = 0x08,
  E_MALFORMED_MESSAGE = 0x09,
  E_WRONG_MESSAGE_TYPE = 0x0a,
  E_RETURN_CODE_LEGAL_THRESHOLD = 0x40  //return codes from 0x40 (inclusive) and upwards are illegal.
};

enum SPECIAL_MESSAGE_IDS {
  MSG_MAGIC_COOKIE = 0xffff0000,
  MSG_MAGIC_COOKIE_ACK = 0xffff8000,
  MSG_SD = 0xffff8100
};

enum PROTOCOL_VERSION{
  LEGAL_PROTOCOL_VERSION = 0x01
};

enum MAGIC_COOKIE_CONSTANTS{
  MC_REQUEST_ID = 0xDEADBEEF,
  MC_LENGTH = 0x08,
  MC_INTERFACE_VERSION = 0x01
};

/**
 * Entry point when protocol is identified.
 */
static void ndpi_int_someip_add_connection (struct ndpi_detection_module_struct *ndpi_struct,
					    struct ndpi_flow_struct *flow)
{
  ndpi_set_detected_protocol(ndpi_struct,flow,NDPI_PROTOCOL_SOMEIP,NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
  NDPI_LOG_INFO(ndpi_struct, "found SOME/IP\n");
}

static u_int32_t someip_data_cover_32(const u_int8_t *data)
{
  u_int32_t value;

  memcpy(&value,data,sizeof(u_int32_t));

  return value;
}
/**
 * Dissector function that searches SOME/IP headers
 */
static void ndpi_search_someip(struct ndpi_detection_module_struct *ndpi_struct,
			       struct ndpi_flow_struct *flow)
{
  const struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  u_int32_t message_id, request_id, someip_len;
  u_int8_t protocol_version,interface_version,message_type,return_code;
  
  if (packet->payload_packet_len < 16) {
    NDPI_LOG_DBG(ndpi_struct,
	     "Excluding SOME/IP .. mandatory header not found (not enough data for all fields)\n");
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
    return;
  }
  
  //####Maybe check carrier protocols?####

  NDPI_LOG_DBG(ndpi_struct, "search SOME/IP\n");

  //we extract the Message ID and Request ID and check for special cases later
  message_id = ntohl(someip_data_cover_32(&packet->payload[0]));
  request_id = ntohl(someip_data_cover_32(&packet->payload[8]));

  NDPI_LOG_DBG2(ndpi_struct, "====>>>> SOME/IP Message ID: %08x [len: %u]\n",
	   message_id, packet->payload_packet_len);

  //####Maximum packet size in SOMEIP depends on the carrier protocol, and I'm not certain how well enforced it is, so let's leave that for round 2####

  // we extract the remaining length
  someip_len = ntohl(someip_data_cover_32(&packet->payload[4]));
  if (packet->payload_packet_len != (someip_len + 8)) {
    NDPI_LOG_DBG(ndpi_struct, "Excluding SOME/IP .. Length field invalid!\n");
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
    return;
  }

  protocol_version = (u_int8_t) (packet->payload[12]);
  NDPI_LOG_DBG2(ndpi_struct,"====>>>> SOME/IP protocol version: [%d]\n",protocol_version);
  if (protocol_version != LEGAL_PROTOCOL_VERSION){
    NDPI_LOG_DBG(ndpi_struct, "Excluding SOME/IP .. invalid protocol version!\n");
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
    return;
  }

  interface_version = (packet->payload[13]);

  message_type = (u_int8_t) (packet->payload[14]);
  message_type &= (~0x20); /* Clear TP  bit */
  NDPI_LOG_DBG2(ndpi_struct,"====>>>> SOME/IP message type: [%d]\n",message_type);

  if ((message_type != SOMEIP_REQUEST) && (message_type != SOMEIP_REQUEST_NO_RETURN) && (message_type != SOMEIP_NOTIFICATION) && (message_type != SOMEIP_REQUEST_ACK) && 
      (message_type != SOMEIP_REQUEST_NO_RETURN_ACK) && (message_type != SOMEIP_NOTIFICATION_ACK) && (message_type != SOMEIP_RESPONSE) && 
      (message_type != SOMEIP_ERROR) && (message_type != SOMEIP_RESPONSE_ACK) && (message_type != SOMEIP_ERROR_ACK)) {
    NDPI_LOG_DBG(ndpi_struct, "Excluding SOME/IP .. invalid message type!\n");
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
    return;
  }

  return_code = (u_int8_t) (packet->payload[15]);
  NDPI_LOG_DBG2(ndpi_struct,"====>>>> SOME/IP return code: [%d]\n", return_code);
  if ((return_code >= E_RETURN_CODE_LEGAL_THRESHOLD)) {
    NDPI_LOG_DBG(ndpi_struct, "Excluding SOME/IP .. invalid return code!\n");
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
    return;
  }
	
  if (message_id == MSG_MAGIC_COOKIE){
    if ((someip_len == MC_LENGTH) && (request_id == MC_REQUEST_ID) && (interface_version == MC_INTERFACE_VERSION) &&
	(message_type == SOMEIP_REQUEST_NO_RETURN) && (return_code == E_OK)){
      NDPI_LOG_DBG2(ndpi_struct, "found SOME/IP Magic Cookie 0x%x\n",message_type);
      ndpi_int_someip_add_connection(ndpi_struct, flow);
      return;
    }											
    else{
      NDPI_LOG_DBG(ndpi_struct, "Excluding SOME/IP, invalid header for Magic Cookie\n");
      NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
      return;
    }
  }
	
  if (message_id == MSG_MAGIC_COOKIE_ACK){
    if ((someip_len == MC_LENGTH) && (request_id == MC_REQUEST_ID) && (interface_version == MC_INTERFACE_VERSION) &&
	(message_type == SOMEIP_REQUEST_NO_RETURN) && (return_code == E_OK)){
      NDPI_LOG_DBG2(ndpi_struct, "found SOME/IP Magic Cookie ACK 0x%x\n",message_type);
      ndpi_int_someip_add_connection(ndpi_struct, flow);
      return;
    }											
    else{
      NDPI_LOG_DBG(ndpi_struct, "Excluding SOME/IP, invalid header for Magic Cookie ACK\n");
      NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
      return;
    }
  }

  if (message_id == MSG_SD){
    NDPI_LOG_DBG2(ndpi_struct, "SOME/IP-SD currently not supported [%d]\n", message_type);
  }

  ndpi_int_someip_add_connection(ndpi_struct, flow);
}
/**
 * Entry point for the ndpi library
 */
void init_someip_dissector (struct ndpi_detection_module_struct *ndpi_struct)
{
  register_dissector("SOME/IP", ndpi_struct,
                     ndpi_search_someip,
                     NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
                     1, NDPI_PROTOCOL_SOMEIP);
}


