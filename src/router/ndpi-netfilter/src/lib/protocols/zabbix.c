/*
 * zabbix.c
 *
 * Copyright (C) 2019 - ntop.org
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

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_ZABBIX

#include "ndpi_api.h"
#include "ndpi_private.h"

/* *************************************************** */

static void ndpi_int_zabbix_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					   struct ndpi_flow_struct *flow) {
  ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_ZABBIX, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
}

/* *************************************************** */

static void ndpi_search_zabbix(struct ndpi_detection_module_struct *ndpi_struct,
			       struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  u_int8_t tomatch[] = { 'Z', 'B', 'X', 'D' };

  NDPI_LOG_DBG(ndpi_struct, "search Zabbix\n");

  if((packet->payload_packet_len >= 4)
     && (memcmp(packet->payload, tomatch, 4) == 0))
    ndpi_int_zabbix_add_connection(ndpi_struct, flow);
  else
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
}

/* *************************************************** */

void init_zabbix_dissector(struct ndpi_detection_module_struct *ndpi_struct) {
  register_dissector("Zabbix", ndpi_struct,
                     ndpi_search_zabbix,
                     NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
                     1, NDPI_PROTOCOL_ZABBIX);
}
