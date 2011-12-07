/*
 * nfs.c
 * Copyright (C) 2009-2011 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ipq_protocols.h"
#ifdef IPOQUE_PROTOCOL_NFS

static void ipoque_int_nfs_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_NFS, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_nfs(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	u8 offset = 0;
	if (packet->tcp != NULL)
		offset = 4;

	if (packet->payload_packet_len < (40 + offset))
		goto exclude_nfs;

	IPQ_LOG(IPOQUE_PROTOCOL_NFS, ipoque_struct, IPQ_LOG_DEBUG, "NFS user match stage 1\n");


	if (offset != 0 && get_u32(packet->payload, 0) != htonl(0x80000000 + packet->payload_packet_len - 4))
		goto exclude_nfs;

	IPQ_LOG(IPOQUE_PROTOCOL_NFS, ipoque_struct, IPQ_LOG_DEBUG, "NFS user match stage 2\n");

	if (get_u32(packet->payload, 4 + offset) != 0)
		goto exclude_nfs;

	IPQ_LOG(IPOQUE_PROTOCOL_NFS, ipoque_struct, IPQ_LOG_DEBUG, "NFS user match stage 3\n");

	if (get_u32(packet->payload, 8 + offset) != htonl(0x02))
		goto exclude_nfs;

	IPQ_LOG(IPOQUE_PROTOCOL_NFS, ipoque_struct, IPQ_LOG_DEBUG, "NFS match stage 3\n");

	if (get_u32(packet->payload, 12 + offset) != htonl(0x000186a5)
		&& get_u32(packet->payload, 12 + offset) != htonl(0x000186a3)
		&& get_u32(packet->payload, 12 + offset) != htonl(0x000186a0))
		goto exclude_nfs;

	IPQ_LOG(IPOQUE_PROTOCOL_NFS, ipoque_struct, IPQ_LOG_DEBUG, "NFS match stage 4\n");

	if (ntohl(get_u32(packet->payload, 16 + offset)) > 4)
		goto exclude_nfs;

	IPQ_LOG(IPOQUE_PROTOCOL_NFS, ipoque_struct, IPQ_LOG_DEBUG, "NFS match\n");

	ipoque_int_nfs_add_connection(ipoque_struct);
	return;

  exclude_nfs:
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_NFS);
}

#endif
