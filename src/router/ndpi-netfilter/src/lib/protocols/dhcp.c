/*
 * dhcp.c
 *
 * Copyright (C) 2016-22 - ntop.org
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

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_DHCP

#include "ndpi_api.h"
#include "ndpi_private.h"

/* freeradius/src/lib/dhcp.c */
#define DHCP_CHADDR_LEN	6
#define DHCP_SNAME_LEN	64
#define DHCP_FILE_LEN	128
#define DHCP_VEND_LEN	308
#define DHCP_OPTION_MAGIC_NUMBER 	0x63825363
#define DHCP_MAGIC_LEN             4

PACK_ON
struct dhcp_packet {
  uint8_t	msgType;
  uint8_t	htype;
  uint8_t	hlen;
  uint8_t	hops;
  uint32_t	xid;/* 4 */
  uint16_t	secs;/* 8 */
  uint16_t	flags;
  uint32_t	ciaddr;/* 12 */
  uint32_t	yiaddr;/* 16 */
  uint32_t	siaddr;/* 20 */
  uint32_t	giaddr;/* 24 */
  uint8_t	chaddr[DHCP_CHADDR_LEN]; /* 28 */
  uint8_t	pad[10]; /* 34 */
  uint8_t	sname[DHCP_SNAME_LEN]; /* 44 */
  uint8_t	file[DHCP_FILE_LEN]; /* 108 */
  uint8_t	magic[DHCP_MAGIC_LEN]; /* 236 */
  uint8_t	options[DHCP_VEND_LEN];
} PACK_OFF;


static void ndpi_int_dhcp_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					 struct ndpi_flow_struct *flow) {
  ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_DHCP,
			     NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
}

static int is_dhcp_magic(uint8_t *magic) {
  if((magic[0] == 0x63)
     && (magic[1] == 0x82)
     && (magic[2] == 0x53)
     && (magic[3] == 0x63))
    return(1);
  else
    return(0);
}

static void ndpi_search_dhcp_udp(struct ndpi_detection_module_struct *ndpi_struct,
			  struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  u_int8_t msg_type = 0;

  NDPI_LOG_DBG(ndpi_struct, "search DHCP\n");

  /* this detection also works for asymmetric dhcp traffic */

  /* check standard DHCP 0.0.0.0:68 -> 255.255.255.255:67 */
  if(packet->udp) {
    struct dhcp_packet *dhcp = (struct dhcp_packet*)packet->payload;
    
    if((packet->payload_packet_len >= 244 /* 244 is the offset of options[0] in struct dhcp_packet */)
       && (packet->udp->source == htons(67) || packet->udp->source == htons(68))
       && (packet->udp->dest == htons(67) || packet->udp->dest == htons(68))
       && is_dhcp_magic(dhcp->magic)) {	   
      u_int i = 0, foundValidMsgType = 0, opt_offset = 0;

      u_int dhcp_options_size = ndpi_min(DHCP_VEND_LEN /* maximum size of options in struct dhcp_packet */,
					 packet->payload_packet_len - 240);


      /* Parse options in two steps (since we need first the message type and
         it seems there is no specific order in the options list) */
      
      /* First iteration: search for the message type */
      while(i + 1 /* for the len */ < dhcp_options_size) {
        u_int8_t id  = dhcp->options[i];		

        if(id == 0xFF)
          break;
        else {
          /* Prevent malformed packets to cause out-of-bounds accesses */
          u_int8_t len = ndpi_min(dhcp->options[i+1] /* len as found in the packet */,
				  dhcp_options_size - (i+2) /* 1 for the type and 1 for the value */);
          if(len == 0)
            break;
	  
          if(id == 53 /* DHCP Message Type */) {
            msg_type = dhcp->options[i+2];

            if(msg_type <= 8) {
              foundValidMsgType = 1;
              break;
            }
          }
	  
          i += len + 2;
        }
      }

      if(!foundValidMsgType) {
#ifdef DHCP_DEBUG
        NDPI_LOG_DBG2(ndpi_struct, "[DHCP] Invalid message type %d. Not dhcp\n", msg_type);
#endif
        NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
        return;
      }

      /* Ok, we have a valid DHCP packet -> we can write to flow->protos.dhcp */
      NDPI_LOG_INFO(ndpi_struct, "found DHCP\n");
      ndpi_int_dhcp_add_connection(ndpi_struct, flow);

      /* Second iteration: parse the interesting options */
      while(i + 1 /* for the len */ < dhcp_options_size) {
        u_int8_t id  = dhcp->options[i];

        if(id == 0xFF)
         break;
        else {
	  int rc;	  
          /* Prevent malformed packets to cause out-of-bounds accesses */
          u_int8_t len = ndpi_min(dhcp->options[i+1] /* len as found in the packet */,
				  dhcp_options_size - (i+2) /* 1 for the type and 1 for the value */);

          if(len == 0 || opt_offset >= sizeof(flow->protos.dhcp.options))
            break;

	  rc = ndpi_snprintf((char*)&flow->protos.dhcp.options[opt_offset],
			     sizeof(flow->protos.dhcp.options) - opt_offset,
			     "%s%u", (i > 0) ? "," : "", id);
	  
	  if(rc > 0) opt_offset += rc;	

#ifdef DHCP_DEBUG
          NDPI_LOG_DBG2(ndpi_struct, "[DHCP] Id=%d [len=%d]\n", id, len);
#endif

          if(id == 55 /* Parameter Request List / Fingerprint */) {
            u_int idx, fing_offset = 0;
	    
            for(idx = 0; idx < len && fing_offset < sizeof(flow->protos.dhcp.fingerprint) - 2; idx++) {
	      rc = ndpi_snprintf((char*)&flow->protos.dhcp.fingerprint[fing_offset],
				sizeof(flow->protos.dhcp.fingerprint) - fing_offset,
				 "%s%u", (idx > 0) ? "," : "",
				 (unsigned int)dhcp->options[i+2+idx] & 0xFF);
	      
              if(rc < 0) break; else fing_offset += rc;
            }
	    
            flow->protos.dhcp.fingerprint[sizeof(flow->protos.dhcp.fingerprint) - 1] = '\0';
          } else if(id == 60 /* Class Identifier */) {
            char *name = (char*)&dhcp->options[i+2];
            int j = 0;
	    
            j = ndpi_min(len, sizeof(flow->protos.dhcp.class_ident)-1);
            strncpy((char*)flow->protos.dhcp.class_ident, name, j);
            flow->protos.dhcp.class_ident[j] = '\0';
          } else if(id == 12 /* Host Name */) {
            u_int8_t *name = &dhcp->options[i+2];

#ifdef DHCP_DEBUG
            NDPI_LOG_DBG2(ndpi_struct, "[DHCP] '%.*s'\n",name,len);
	      //	    while(j < len) { printf( "%c", name[j]); j++; }; printf("\n");
#endif
            ndpi_hostname_sni_set(flow, name, len, NDPI_HOSTNAME_NORM_ALL);
	  }

          i += len + 2;
        }
      }
    } else
      NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
  }
}


void init_dhcp_dissector(struct ndpi_detection_module_struct *ndpi_struct) {
  register_dissector("DHCP", ndpi_struct,
                     ndpi_search_dhcp_udp,
                     NDPI_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD,
                     1, NDPI_PROTOCOL_DHCP);
}
