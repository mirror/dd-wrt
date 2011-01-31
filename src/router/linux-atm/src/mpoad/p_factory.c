#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/atmmpc.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <atm.h>
#include "packets.h"
#include "get_vars.h"
#include "io.h"

#if 0
#define dprintf printf
#else
#define dprintf(format,args...)
#endif

extern struct mpc_control mpc_control; /* From main.c */

/* slightly modified version from RFC 1071 */
unsigned short compute_ip_csum(unsigned char *addr, int count)
{
           /* Compute Internet Checksum for "count" bytes
            *         beginning at location "addr".
            */
       uint32_t sum = 0;

        while( count > 1 )  {
           /*  This is the inner loop */
               sum += * ((uint16_t *) addr);
               addr += 2;
               count -= 2;
       }

           /*  Add left-over byte, if any */
       if( count > 0 )
               sum += * (unsigned char *) addr;

           /*  Fold 32-bit sum to 16 bits */
       while (sum>>16)
           sum = (sum & 0xffff) + (sum >> 16);

       return (~sum);
}
       

/* 
 * Prefills the fixed header. 
 * Only question is wheter the NBMA subaddress
 * can really be omitted or not.
 */
static void prefill_fixed_h(struct nhrp_fixed_h *header){
  header->ar_afn = htons(AR_AFN_NSAP);
  header->ar_pro_type = htons(AR_PRO_TYPE_ETHER);
  header->ar_hopcnt = get_ttl(); /* get_vars.c */
  header->ar_op_version = AR_OP_VERSION_NHRP;
  header->ar_shtl = AR_SHTL_NSAP; 
  header->ar_sstl = 0;
}

/* 
 * Prefills the common header.
 */
static void prefill_common_h(struct nhrp_common_h *header){
  header->src_proto_len = PROTO_LEN_IP;
  header->dst_proto_len = PROTO_LEN_IP;
  get_own_atm_addr(header->src_nbma_address);
  header->src_protocol_address = htonl(get_own_ip_addr(mpc_control.INTERFACE_NUMBER));  
}

/* 
 * Prefills the common header without the source IP_address 
 */
static void prefill_common_h_short(struct nhrp_common_h_short *header){
  header->src_proto_len = 0;
  header->dst_proto_len = PROTO_LEN_IP;
  get_own_atm_addr(header->src_nbma_address);
}

/*
 *  Generates a new request id.
 */
static uint32_t Request_ID(void){
  static uint32_t id = 0;
  id = (id + 1) %  UINT_MAX;
  return id;
}


/* 
 *  Fills the checksum and length fields in the fixed header.
 */
static void finish(int length, uint8_t *buff, 
		  struct nhrp_fixed_h *fixed){
  int append = length;
  fixed->ar_pktsz = htons(length);
  if(append % 2 == 1){ 
    append++;
  }
  fixed->ar_chksum = (compute_ip_csum(buff,append));
  return;
}

/* 
 *  NHRP purge request.
 */
int send_purge_request( 
		       uint32_t src_ip, 
		       uint8_t prefix_length,
		       uint32_t purge_ip,
		       int shortcut_fd
		       ){
  int pos = 0;
  struct nhrp_fixed_h *fixed;
  struct nhrp_common_h_short *common_short;
  struct nhrp_common_h_no_ip *common_no_ip;
  struct nhrp_cie_short *cie_short;
  uint8_t buffer[MAX_PACKET_LENGTH];
  uint8_t *buff = buffer;
  memset(buff,0,MAX_PACKET_LENGTH);
  fixed = (struct nhrp_fixed_h *)buff;
  prefill_fixed_h(fixed);
  fixed->ar_op_type = NHRP_PURGE_REQUEST;
  pos += sizeof(struct nhrp_fixed_h);
  if(src_ip){
    common_short = (struct nhrp_common_h_short *)(buff + pos);
    common_short->src_proto_len = PROTO_LEN_IP;
    common_short->dst_proto_len = 0;
    get_own_atm_addr(common_short->src_nbma_address);
    common_short->flags = htons(common_short->flags | FLAG_N);
    common_short->dst_protocol_address = src_ip;
    pos += sizeof(struct nhrp_common_h_short);
  }
  else{
    common_no_ip = (struct nhrp_common_h_no_ip *)(buff + pos);
    get_own_atm_addr(common_no_ip->src_nbma_address);
    common_no_ip->flags = htons(common_no_ip->flags | FLAG_N);
    pos += sizeof(struct nhrp_common_h_no_ip);
  }
  cie_short = (struct nhrp_cie_short *)(buff + pos);
  cie_short->prefix_length = prefix_length;
  cie_short->cli_proto_len = PROTO_LEN_IP;
  pos += sizeof(struct nhrp_cie_short); 
  *((uint32_t *)(buff + pos)) = htonl(purge_ip);
  pos += sizeof(purge_ip);  
  finish(pos, buff, fixed);
  return send_to_dataplane(buff, pos, shortcut_fd);
}

/*
 * NHRP purge reply. Sent in response to NHRP purge request if N-flag
 * is not set. Only the packet type is changed and new checksum calculated. 
 */
int send_purge_reply(uint8_t *buff)
{
  int pos;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h *)buff;
  fixed->ar_op_type = NHRP_PURGE_REPLY;
  pos = ntohs(fixed->ar_pktsz);
  finish(pos, buff, fixed);
  return send_to_mps(buff, pos);
}

/*
 * MPOA resolution request. 
 */
int send_resolution_request(uint32_t rqst_id,
			    uint16_t source_ip_present, 
			    uint32_t dest_ip,
			    uint8_t prefix_length, 
			    uint32_t service_category){
  int pos = 0;
  struct nhrp_fixed_h *fixed;
  struct nhrp_common_h *common;
  struct nhrp_common_h_short *common_s;
  struct nhrp_extension *extension;
  struct nhrp_cie_short *cie;
  struct nhrp_extension_with_value *extension_with_value;
  uint8_t buff[MAX_PACKET_LENGTH];
  memset(buff,0,MAX_PACKET_LENGTH);
 
  dprintf("mpcd: p_factory.c: sending a resolution request %x ",dest_ip);
  
  fixed = (struct nhrp_fixed_h *)buff;
  prefill_fixed_h(fixed);
  fixed->ar_op_type = MPOA_RESOLUTION_REQUEST;
  pos += sizeof(struct nhrp_fixed_h);
  if( source_ip_present ){
    common = (struct nhrp_common_h *)(buff + sizeof(struct nhrp_fixed_h));
    prefill_common_h(common);
    pos += sizeof(struct  nhrp_common_h);
    common->dst_protocol_address = dest_ip;
    if(!rqst_id){
      common->request_ID = htonl(Request_ID());
      new_id(ntohl(common->request_ID),dest_ip,MPOA_RESOLUTION_REQUEST);
    }
    else
      common->request_ID = htonl(rqst_id);
    dprintf("mpcd: p_factory.c: with request_id %d\n",ntohl(common->request_ID));
  }
  else{
    common_s = (struct nhrp_common_h_short *)(buff + sizeof(struct nhrp_fixed_h));
    prefill_common_h_short(common_s);
    pos += sizeof(struct nhrp_common_h_short);
    common_s->dst_protocol_address = dest_ip;
    if(!rqst_id){
      common_s->request_ID = htonl(Request_ID());
      new_id(ntohl(common_s->request_ID),dest_ip,MPOA_RESOLUTION_REQUEST);
    }
    else
      common_s->request_ID = htonl(rqst_id);
    dprintf("mpcd: p_factory.c: with request_id %d\n",ntohl(common_s->request_ID));
  }
  
  cie = (struct nhrp_cie_short *)(buff + pos);
  cie->prefix_length = MAX_PREFIX_LENGTH;
  pos += sizeof(struct nhrp_cie_short);
  
  fixed->ar_extoff = htons(pos);
  extension  = (struct nhrp_extension *)(buff + pos);  
  extension->type = htons(MPOA_EGRESS_CACHE_TAG_EXTENSION);
  extension->length = 0;
  pos += sizeof(struct nhrp_extension);
  if(service_category){
    extension_with_value = (struct nhrp_extension_with_value *)(buff + pos);
    extension_with_value->type = htons(MPOA_ATM_SERVICE_CATEGORY_EXTENSION);
    extension_with_value->length = htons(sizeof(service_category));
    extension_with_value->value = htonl(service_category);
    pos += sizeof(struct nhrp_extension_with_value);
  } 
  extension = (struct nhrp_extension *)(buff + pos);
  extension->type = htons(NHRP_END_OF_EXTENSIONS);
  extension->length = 0;
  pos += sizeof(struct nhrp_extension);

  finish(pos,buff,fixed);
  return send_to_mps(buff,pos);
}

/* 
 *  Sent as a reply to cache impositon request.
 */
int send_cache_imposition_reply(
				uint8_t *request,
				uint32_t tag,
				uint8_t  code,
				uint8_t  prefix_length,
				uint16_t mtu,
				uint8_t  cli_atm_addr[ATM_ESA_LEN]
				){
  int pos = 0;
  struct nhrp_fixed_h *fixed;
  struct nhrp_fixed_h *fixed_request;
  struct nhrp_extension *extension;
  struct nhrp_extension_with_value *extension_with_value;
  struct dll_header_extension *dll_ext;
  struct extension_values values;
  struct nhrp_common_h *common;
  struct nhrp_cie_no_ip *cie;
  struct nhrp_cie_short *cie_short;
  uint8_t buffer[MAX_PACKET_LENGTH];
  uint8_t *reply = buffer; 
  
  memset(reply,0,MAX_PACKET_LENGTH);
  memset(&values,0,sizeof(values));
  fixed = (struct nhrp_fixed_h *)reply;
  prefill_fixed_h(fixed);
  fixed->ar_op_type = MPOA_CACHE_IMPOSITION_REPLY;
  pos += sizeof(struct nhrp_fixed_h);
  common = (struct nhrp_common_h *) memcpy(reply+pos,
					   request+pos,
					   sizeof(struct nhrp_common_h)); 
  pos += sizeof(struct nhrp_common_h);  
  if( !code ){
    cie = (struct nhrp_cie_no_ip *)(reply + pos);
    cie->code = code;
    if(prefix_length)
      cie->prefix_length = prefix_length;
    else
      cie->prefix_length = MAX_PREFIX_LENGTH;
    cie->mtu = MTU_DEFAULT;
    cie->cli_addr_tl = ATM_ESA_LEN;
    cie->cli_saddr_tl = 0;
    cie->cli_proto_len = 0;
    memcpy(cie->cli_nbma_address,mpc_control.data_listen_addr.sas_addr.prv,ATM_ESA_LEN);
    pos += sizeof(struct nhrp_cie_no_ip);
  }
  else{
    cie_short = (struct nhrp_cie_short *)(reply + pos);
    cie_short->code = code;
    cie_short->prefix_length = prefix_length;
    cie_short->mtu = mtu;
    pos += sizeof(struct nhrp_cie_short);
  }
  fixed_request = (struct nhrp_fixed_h *)request; 
  if(fixed_request->ar_extoff)
    parse_extensions(request+ntohs(fixed_request->ar_extoff),&values);
  fixed->ar_extoff = htons(pos);
  extension_with_value = (struct nhrp_extension_with_value *)(reply + pos);
  extension_with_value->type = htons(MPOA_EGRESS_CACHE_TAG_EXTENSION);
  extension_with_value->length = htons(sizeof(tag));
  extension_with_value->value = tag;
  pos += sizeof(struct nhrp_extension_with_value);
  if(values.service_category_present){
    extension_with_value = (struct nhrp_extension_with_value *)(reply + pos);
    extension_with_value->type = htons(MPOA_ATM_SERVICE_CATEGORY_EXTENSION);
    extension_with_value->length = htons(sizeof(values.service_category));
    extension_with_value->value = htonl(CBR); /* FIXME */
    pos += sizeof(struct nhrp_extension_with_value);
  }
  if(values.dll_header_present){
    dll_ext = (struct dll_header_extension *)(reply + pos);
    dll_ext->type = htons(MPOA_DLL_HEADER_EXTENSION);
    dll_ext->length = values.dll_ext.length;
    dll_ext->cache_id = values.dll_ext.cache_id;
    dll_ext->elan_id = values.dll_ext.elan_id;
    dll_ext->dh_length = values.dll_ext.dh_length;
    memcpy(dll_ext->dll_header,values.dll_ext.dll_header,values.dll_ext.dh_length);
    pos += 13 + 14;
  }
  extension = (struct nhrp_extension *)(reply + pos);
  extension->type = htons(NHRP_END_OF_EXTENSIONS);
  extension->length = 0;
  pos += sizeof(struct  nhrp_extension);
  finish(pos, reply, fixed);
  return send_to_mps(reply,pos);
}


int send_egress_cache_purge_request(
				    uint16_t no_reply,
				    uint32_t eg_MPS_ip_addr,
				    uint8_t  prefix,
				    uint32_t purge_ip,
				    uint32_t cache_id
				    )
{
  int pos = 0;
  struct nhrp_fixed_h *fixed;
  struct dll_header_extension *extension_dll;
  struct nhrp_extension *extension;
  struct nhrp_common_h *common;
  struct nhrp_cie *cie;
  uint8_t buffer[MAX_PACKET_LENGTH];
  uint8_t *buff = buffer;
  
  memset(buff,0,MAX_PACKET_LENGTH);
  fixed = (struct nhrp_fixed_h *)buff;
  prefill_fixed_h(fixed);
  fixed->ar_op_type = MPOA_EGRESS_CACHE_PURGE_REQUEST;
  pos += sizeof(struct nhrp_fixed_h);
  common = (struct nhrp_common_h *)(buff + pos);
  if(no_reply)
    common->flags = htons(common->flags | FLAG_N);
  common->request_ID = htonl(Request_ID());
  new_id(ntohl(common->request_ID),cache_id,MPOA_EGRESS_CACHE_PURGE_REQUEST); 
  prefill_common_h(common);
  common->dst_protocol_address = ntohl(eg_MPS_ip_addr);
#ifdef MPOA_1_1
  memcpy(common->src_nbma_address, mpc_control.OWN_ATM_ADDRESS, ATM_ESA_LEN);
#endif
  pos += sizeof(struct nhrp_common_h);
  cie = (struct nhrp_cie *)(buff + pos);
  cie->prefix_length = prefix;
  cie->cli_addr_tl = ATM_ESA_LEN;
  cie->cli_proto_len = PROTO_LEN_IP;
  cie->cli_protocol_address = htonl(purge_ip);
  get_own_atm_addr(cie->cli_nbma_address);
  pos += sizeof(struct nhrp_cie);
  fixed->ar_extoff = htons(pos);
  extension_dll = (struct dll_header_extension *)(buff + pos);
  extension_dll->type = htons(MPOA_DLL_HEADER_EXTENSION);
  extension_dll->length = htons(9); /* FIXME: needs at least checking */
  extension_dll->cache_id = cache_id;
  dprintf("mpcd: p_factory.c: egress cache_purge_request, cache_id = %u",cache_id);
  pos += 13;
  extension = (struct nhrp_extension *)(buff + pos);
  extension->type = htons(NHRP_END_OF_EXTENSIONS);
  pos += sizeof(struct nhrp_extension);
  finish(pos, buff, fixed);
  return send_to_mps(buff,pos);
}

int send_egress_cache_purge_reply(uint8_t *buff){
  int pos;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h *)buff;
  fixed->ar_op_type = MPOA_EGRESS_CACHE_PURGE_REPLY;
  pos = ntohs(fixed->ar_pktsz);
  finish(pos, buff, fixed);
  return send_to_mps(buff,pos);
}


