/*
 * mqtt.c
 *
 * Copyright (C) 2016 Sorin Zamfir <sorin.zamfir@yahoo.com>
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

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_MQTT

#include "ndpi_api.h"
#include "ndpi_private.h"


/**
 * The type of control messages in mqtt version 3.1.1
 * see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1
 */
enum MQTT_PACKET_TYPES {
	CONNECT = 1,
	CONNACK = 2,
	PUBLISH = 3,
	PUBACK = 4,
	PUBREC = 5,
	PUBREL = 6,
	PUBCOMP = 7,
	SUBSCRIBE = 8,
	SUBACK = 9,
	UNSUBSCRIBE = 10,
	UNSUBACK = 11,
	PINGREQ = 12,
	PINGRESP = 13,
	DISCONNECT = 14
};

/**
 * Entry point when protocol is identified.
 */
static void ndpi_int_mqtt_add_connection (struct ndpi_detection_module_struct *ndpi_struct,
		struct ndpi_flow_struct *flow)
{
	ndpi_set_detected_protocol(ndpi_struct,flow,NDPI_PROTOCOL_MQTT,NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
	NDPI_LOG_INFO(ndpi_struct, "found Mqtt\n");
}

static int64_t get_var_int(const unsigned char *buf, int buf_len, u_int8_t *num_bytes)
{
	int i, multiplier = 1;
	u_int32_t value = 0;
	u_int8_t encodedByte;

	*num_bytes= 0;
	for (i = 0; i < 4; i++) {
		if (i >= buf_len)
			return -1;
		(*num_bytes)++;
		encodedByte = buf[i];
		value += ((encodedByte & 127) * multiplier);
		if ((encodedByte & 128) == 0)
			break;
		multiplier *= 128;
	}
	return value;
}

/**
 * Dissector function that searches Mqtt headers
 */
static void ndpi_search_mqtt(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
	u_int8_t pt,flags, rl_len;
	int64_t rl;

	NDPI_LOG_DBG(ndpi_struct, "search Mqtt\n");
	if (flow->packet_counter > 10) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt .. mandatory header not found!\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}

	NDPI_LOG_DBG2(ndpi_struct, "====>>>> Mqtt header: %4x%4x%4x%4x [len: %u]\n",
		      packet->payload_packet_len > 0 ? packet->payload[0] : '.',
		      packet->payload_packet_len > 1 ? packet->payload[1] : '.',
		      packet->payload_packet_len > 2 ? packet->payload[2] : '.',
		      packet->payload_packet_len > 3 ? packet->payload[3] : '.',
		      packet->payload_packet_len);
	if (packet->payload_packet_len < 2) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt .. mandatory header not found!\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}
	// we extract the remaining length
	rl = get_var_int(&packet->payload[1], packet->payload_packet_len - 1, &rl_len);
	if (rl < 0) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt .. invalid length!\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}
	NDPI_LOG_DBG(ndpi_struct, "Mqtt: msg_len %d\n", (unsigned long long)rl);
	if (packet->payload_packet_len != rl + 1 + rl_len) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt .. maximum packet size exceeded!\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}
	// we extract the packet type
	pt = (u_int8_t) ((packet->payload[0] & 0xF0) >> 4);
	NDPI_LOG_DBG2(ndpi_struct,"====>>>> Mqtt packet type: [%d]\n",pt);
	if ((pt == 0) || (pt == 15)) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt .. invalid packet type!\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}
	// we extract the flags
	flags = (u_int8_t) (packet->payload[0] & 0x0F);
	NDPI_LOG_DBG2(ndpi_struct,"====>>>> Mqtt flags type: [%d]\n",flags);
	// first stage verification
	if (((pt == CONNECT) || (pt == CONNACK) || (pt == PUBACK) || (pt == PUBREC) ||
					(pt == PUBCOMP) || (pt == SUBACK) || (pt == UNSUBACK) || (pt == PINGREQ) ||
					(pt == PINGRESP) || (pt == DISCONNECT)) && (flags > 0)) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid Packet-Flag combination flag!=0\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}
	if (((pt == PUBREL) || (pt == SUBSCRIBE) || (pt == UNSUBSCRIBE)) && (flags != 2)) {
		NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid Packet-Flag combination flag!=2\n");
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	}
	NDPI_LOG_DBG2(ndpi_struct,"====>>>> Passed first stage of identification\n");
	// second stage verification (no payload, just variable headers)
	if ((pt == CONNACK) || (pt == PUBACK) || (pt == PUBREL) ||
			(pt == PUBREC) || (pt == PUBCOMP) || (pt == UNSUBACK)) {
		if (packet->payload_packet_len != 4) { // these packets are always 4 bytes long
			NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid Packet-Length < 4 \n");
			NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
			return;
		} else {
			NDPI_LOG_INFO(ndpi_struct, "found Mqtt CONNACK/PUBACK/PUBREL/PUBREC/PUBCOMP/UNSUBACK\n");
			ndpi_int_mqtt_add_connection(ndpi_struct,flow);
			return;
		}
	}
	if ((pt == PINGREQ) || (pt == PINGRESP) || (pt == DISCONNECT)) {
		if (packet->payload_packet_len != 2) { // these packets are always 2 bytes long
			NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid Packet-Length <2 \n");
			NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
			return;
		} else {
			NDPI_LOG_INFO(ndpi_struct, "found Mqtt PING/PINGRESP/DISCONNECT\n");
			ndpi_int_mqtt_add_connection(ndpi_struct,flow);
			return;
		}
	}
	NDPI_LOG_DBG2(ndpi_struct,"====>>>> Passed second stage of identification\n");
	// third stage verification (payload)
	if (pt == CONNECT) {
		NDPI_LOG_DBG(ndpi_struct, "found Mqtt CONNECT\n");
		ndpi_int_mqtt_add_connection(ndpi_struct,flow);
		return;
	}
	if (pt == PUBLISH) {
		// payload CAN be zero bytes length (section 3.3.3 of MQTT standard)
		u_int8_t qos = (u_int8_t) (flags & 0x06) >> 1;
		u_int8_t dup = (u_int8_t) (flags & 0x08) >> 3;
		if (qos > 2) { // qos values possible are 0,1,2
			NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid PUBLISH qos\n");
			NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
			return;
		}
		if (qos == 0) {
			if (dup != 0) {
				NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid PUBLISH qos0 and dup combination\n");
				NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
				return;
			}
			if (packet->payload_packet_len < 5) { // at least topic (3Bytes + 2Bytes fixed header)
				NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid PUBLISH qos0 size\n");
				NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
				return;
			}
		}
		if ((qos == 1) || (qos == 2)) {
			if (packet->payload_packet_len < 7 ) { // at least topic + pkt identifier (3Bytes + 2Bytes + 2Bytes fixed header)
				NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid PUBLISH qos1&2\n");
				NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
				return;
			}
		}
		NDPI_LOG_INFO(ndpi_struct, "found Mqtt PUBLISH\n");
		ndpi_int_mqtt_add_connection(ndpi_struct,flow);
		return;
	}
	if (pt == SUBSCRIBE) {
		if (packet->payload_packet_len < 8) { // at least one topic+filter is required in the payload
			NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid SUBSCRIBE\n");
			NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
			return;
		} else {
			NDPI_LOG_INFO(ndpi_struct, "found Mqtt SUBSCRIBE\n");
			ndpi_int_mqtt_add_connection(ndpi_struct,flow);
			return;
		}
	}
	if (pt == SUBACK ) {
		if (packet->payload_packet_len <5 ) { // must have at least a response code
			NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid SUBACK\n");
			NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
			return;
		} else {
			NDPI_LOG_INFO(ndpi_struct, "found Mqtt SUBACK\n");
			ndpi_int_mqtt_add_connection(ndpi_struct,flow);
			return;
		}
	}
	if (pt == UNSUBSCRIBE) {
		if (packet->payload_packet_len < 7) { // at least a topic
			NDPI_LOG_DBG(ndpi_struct, "Excluding Mqtt invalid UNSUBSCRIBE\n");
			NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
			return;
		} else {
			NDPI_LOG_INFO(ndpi_struct, "found Mqtt UNSUBSCRIBE\n");
			ndpi_int_mqtt_add_connection(ndpi_struct,flow);
			return;
		}
	}
	/* We already checked every possible values of pt: we are never here */
}
/**
 * Entry point for the ndpi library
 */
void init_mqtt_dissector (struct ndpi_detection_module_struct *ndpi_struct)
{
  register_dissector("MQTT", ndpi_struct,
                     ndpi_search_mqtt,
                     NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
                      1, NDPI_PROTOCOL_MQTT);
}


