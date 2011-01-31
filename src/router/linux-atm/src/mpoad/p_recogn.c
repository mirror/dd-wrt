#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <atm.h>
#include <linux/types.h>
#include <linux/atmmpc.h>
#include <netinet/in.h>
#include <limits.h> /* For UINT_MAX */
#include "io.h"
#include "k_interf.h"
#include "packets.h"

extern struct mpc_control mpc_control;  /* from main.c */
extern int keep_alive_sm_running;       /* from io.c   */
static unsigned char service_category_to_traff_class(uint32_t service_category);
#if 0
#define dprintf printf
#else
#define dprintf(format,args...)
#endif

int parse_extensions(uint8_t *buff,struct extension_values * values ){
  int pos = 0;
  struct dll_header_extension *dll_ext;
  struct nhrp_extension_with_value *ext_with_value;
  struct nhrp_extension *extension = (struct nhrp_extension *)buff;

  memset(values, 0, sizeof(struct extension_values));
  while(ntohs(extension->type) != NHRP_END_OF_EXTENSIONS &&
	ntohs(extension->type) != 0){
    switch(ntohs(extension->type)){
    case MPOA_DLL_HEADER_EXTENSION:
      dprintf("mpcd: p_recogn.c: dll_header_extension.\n");
      dll_ext = (struct dll_header_extension *)(buff + pos);
      values->dll_header_present = 1;
      values->dll_ext.length = dll_ext->length;
      values->dll_ext.cache_id = dll_ext->cache_id;
      dprintf("mpcd: p_recogn.c: parse_extentsions() cache_id %d\n", dll_ext->cache_id);
      values->dll_ext.elan_id = dll_ext->elan_id;
      values->dll_ext.dh_length = dll_ext->dh_length;
      memcpy(values->dll_ext.dll_header,dll_ext->dll_header
	     ,dll_ext->dh_length);
      pos += sizeof(struct dll_header_extension) -
             sizeof(dll_ext->dll_header) + dll_ext->dh_length;
      dprintf("mpcd: p_recogn.c: parse_extensions() pos %d values->dll_ext.cache_id %d\n", pos, values->dll_ext.cache_id);
      break;
    case MPOA_EGRESS_CACHE_TAG_EXTENSION:
      dprintf("mpcd: p_recogn.c: mpoa_egress_cache_tag_extension.\n");
      values->egress_cache_tag_ext_present = 1;
      extension = (struct nhrp_extension *)(buff + pos);
      if(ntohs(extension->length)){
	values->tag_present = 1;
	pos += sizeof(struct nhrp_extension);
	values->tag = *((uint32_t*)(buff + pos));
        pos += sizeof(uint32_t);
      }
      else
	pos += sizeof(struct nhrp_extension);
      break;
    case MPOA_ATM_SERVICE_CATEGORY_EXTENSION:
      printf("mpcd: p_recogn.c: mpoa_atm_service_category_extension.");
      ext_with_value = (struct nhrp_extension_with_value *)(buff + pos);
      values->service_category_present = 1;
      values->service_category = ntohl(ext_with_value->value);
      pos += sizeof(struct nhrp_extension_with_value);
      printf("service_category = %u\n",values->service_category);
      break;
    case MPOA_KEEPALIVE_LIFETIME_EXTENSION:
      dprintf("mpcd: p_recogn.c: mpoa_keepalive_lifetime_extension.\n");
      ext_with_value = (struct nhrp_extension_with_value *)(buff + pos);
      values->keep_alive_lifetime_present = 1;
      values->keep_alive_lifetime = ntohl(ext_with_value->value);
      pos += sizeof(struct nhrp_extension_with_value);
      break;
    case MPOA_HOP_COUNT_EXTENSION:
      dprintf("mpcd: p_recogn.c: mpoa_hop_count_extension.\n");
      ext_with_value = (struct nhrp_extension_with_value *)(buff + pos);
      values->hop_count_present = 1;
      values->hop_count = ntohl(ext_with_value->value);
      pos += sizeof(struct nhrp_extension_with_value);
      break;
    case MPOA_ORIGINAL_ERROR_CODE_EXTENSION:
      dprintf("mpcd: p_recogn.c: mpoa_original_error_code_extension.\n");
      ext_with_value = (struct nhrp_extension_with_value *)(buff + pos);
      values->error_code_present = 1;
      values->error_code = ntohl(ext_with_value->value);
      pos += sizeof(struct nhrp_extension_with_value);
      break;
    default:
      printf("mpcd: p_recogn.c: Unrecognized extension: %x\n",ntohs(extension->type));
      pos += sizeof(struct nhrp_extension_with_value);
      break;
    }
    extension = (struct nhrp_extension *)(buff + pos); 
  }
  if(pos)
    pos += sizeof(struct nhrp_extension);  /* End_of_extensions extension */
  return pos;
}  

static uint32_t calculate_ip_mask(uint8_t prefix){
  int i;
  uint32_t mask = 0;
  if(prefix >= 32)
    return UINT_MAX;
  for( i = 0; i < prefix - 1; i++){
    mask = mask | 1;
    mask = mask << 1;
  }
  mask = mask | 1;
  mask = mask << (32 - prefix);
  return mask;
}

static void print_cie_code(uint8_t code){
  printf("mpcd: p_recogn.c: resolution_reply_rcvd: ");
  switch(code){
  case ADMINISTRATIVELY_PROHIBITED:
    printf("administratively prohibited. \n");
    break;
  case INSUFFICIENT_RESOURCES:
    printf("insufficient resources. \n");
    break;
  case NO_INET_ADDR_TO_NBMA_ADDR_BINDING_EXISTS:
    printf("no internet layer address to nbma address binding exists.\n");
    break;
  case UNIQUE_INET_ADDR_ALREADY_REGISTERED:
    printf("unique internet layer address already registered.\n");
    break;
  case INSUFF_RES_TO_EGRESS_CACHE_ENTRTY:
    printf("insufficient resources to accept egress cache entry.\n");
    break;
  case INSUFF_RES_TO_SHORTCUT:
    printf("insufficient resources to accpet shortcut.\n");
    break;
  case INSUFF_RES_TO_EITHER_ENTRY_OR_SHORTCUT:
    printf("insufficient resources to accept either shortcut or egress cache entry.\n");
    break;
  case UNSUPPORTED_INTERN_LAYER_PROTO:
    printf("unsupported internetwork layer protocol.\n");
    break;
  case UNSUPPORTED_MAC_LAYER_PROTO:
    printf("unsupported mac layer protocol.\n");
    break;
  case NOT_AN_MPC:
    printf("not an mpc.\n");
    break;
  case NOT_AN_MPS:
    printf("not an mps.\n");
    break;
  case UNSPECIFIED:
    printf("unspecified.\n");
    break;
  default:
    printf("unrecognized cie code %d.\n",code);
    break;
  }
  return;
}

static int nhrp_purge_request(uint8_t *buff){
  int pos = 0;
  int cie_limit = 0;
  uint32_t ip_mask;
  uint8_t  eg_MPC_data_ATM_addr[ATM_ESA_LEN];
  uint32_t eg_MPS_ip_addr;
  uint32_t purge_ip;
  struct k_message msg;
  struct extension_values values;
  struct nhrp_common_h_no_ip *common_no_ip;
  struct nhrp_cie_no_nbma *cie_no_nbma;
  struct nhrp_cie *cie;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h *)buff;
  memset(&msg,0,sizeof(struct k_message));
  if(ntohs(fixed->ar_extoff)){
    cie_limit = ntohs(fixed->ar_extoff);
  }
  else{
    cie_limit = ntohs(fixed->ar_pktsz);
  }
  pos += sizeof(struct nhrp_fixed_h);
  common_no_ip = (struct nhrp_common_h_no_ip *)(buff + pos);
  memcpy(eg_MPC_data_ATM_addr, common_no_ip->src_nbma_address
	 ,ATM_ESA_LEN);
  pos += sizeof(struct nhrp_common_h_no_ip);
  if(common_no_ip->src_proto_len == PROTO_LEN_IP){
    eg_MPS_ip_addr = ntohl((uint32_t)*(buff + pos));
    pos += sizeof(eg_MPS_ip_addr);
  }
  if(common_no_ip->dst_proto_len == PROTO_LEN_IP)
    pos += sizeof(uint32_t);
  
  while(pos < cie_limit){
    cie = (struct nhrp_cie *)(buff + pos);
    if(cie->cli_addr_tl){
      purge_ip = cie->cli_protocol_address;
      ip_mask = calculate_ip_mask(cie->prefix_length);
      pos += sizeof(struct nhrp_cie);
    }
    else{
      cie_no_nbma = (struct nhrp_cie_no_nbma *)(buff + pos);
      purge_ip = cie_no_nbma->cli_protocol_address;
      ip_mask = calculate_ip_mask(cie_no_nbma->prefix_length);
      pos += sizeof(struct nhrp_cie_no_nbma);
    }
    msg.type = INGRESS_PURGE_RCVD;
    msg.ip_mask = ip_mask;
    msg.content.in_info.in_dst_ip = purge_ip;
    memcpy(msg.MPS_ctrl, mpc_control.MPS_CTRL_ATM_ADDR, ATM_ESA_LEN);
    send_to_kernel(&msg);
  }

  /* we really do not do anything with the extensions, just parse them */
  if(ntohs(fixed->ar_extoff)){
    pos += parse_extensions(buff + ntohs(fixed->ar_extoff), &values);
  }
  if(!(ntohs(common_no_ip->flags) & FLAG_N ))
    return send_purge_reply(buff);  
  return 1;
}

static int mpoa_cache_imposition_request( uint8_t * buff ){
  int pos = 0; 
  struct k_message msg;
  struct nhrp_fixed_h * fixed = (struct nhrp_fixed_h *)buff;
  struct nhrp_common_h * common;
  struct nhrp_cie_short * cie_short;
  struct extension_values values;
  pos += sizeof( struct nhrp_fixed_h );
  memset(&values, 0, sizeof(struct extension_values));
  memset(&msg,0,sizeof(struct k_message));
  common = (struct nhrp_common_h *)(buff + pos);
  memcpy(msg.content.eg_info.in_MPC_data_ATM_addr, 
	 common->src_nbma_address ,ATM_ESA_LEN);
  pos += sizeof(struct nhrp_common_h);
  cie_short = (struct nhrp_cie_short *)(buff + pos);
  msg.content.eg_info.holding_time = ntohs(cie_short->holding_time);
  msg.ip_mask = calculate_ip_mask(cie_short->prefix_length);
  if(ntohs(fixed->ar_extoff)){
    pos += parse_extensions(buff + ntohs(fixed->ar_extoff), &values);
  }
  if(ntohs(cie_short->holding_time)) {
      if (values.dll_header_present == 0) {
          printf("mpcd: p_recogn.c: warning: ");
          printf("holding time non-zero but MPOA DLL Header Extension missing\n");
          return 0;
      }
      keep_alive_sm_running = 1;
  }
  msg.content.eg_info.cache_id = values.dll_ext.cache_id;
  msg.content.eg_info.DH_length = values.dll_ext.dh_length;
  memcpy(msg.content.eg_info.DLL_header, values.dll_ext.dll_header, msg.content.eg_info.DH_length);
  if(common->src_proto_len)
    msg.content.eg_info.mps_ip = common->src_protocol_address;
  if(common->dst_proto_len)
    msg.content.eg_info.eg_dst_ip = common->dst_protocol_address;
  msg.content.eg_info.tag = new_tag(msg.content.eg_info.cache_id);
  memcpy(msg.MPS_ctrl, mpc_control.MPS_CTRL_ATM_ADDR, ATM_ESA_LEN);
  msg.type = CACHE_IMPOS_RCVD;
  send_to_kernel(&msg);
  return send_cache_imposition_reply( buff, msg.content.eg_info.tag,
				      0x00, 0, MTU_DEFAULT, 
				      common->src_nbma_address );
}

static int mpoa_egress_cache_purge_reply( uint8_t * buff ){
  int pos = 0;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h *)buff;
  struct nhrp_common_h *common;
  struct nhrp_cie *cie;
  struct extension_values values;
  struct k_message msg;
  memset(&values, 0, sizeof(struct extension_values));
  memset(&msg,0,sizeof(struct k_message));
  pos += sizeof(struct nhrp_fixed_h);
  common = (struct nhrp_common_h*)(buff + pos);
  if(!check_incoming(ntohl(common->request_ID), MPOA_EGRESS_CACHE_PURGE_REQUEST))
    return -1;
  pos += sizeof(struct nhrp_common_h);
  cie = (struct nhrp_cie *)(buff + pos);
  msg.ip_mask = calculate_ip_mask(cie->prefix_length);
  if(fixed->ar_extoff)
    parse_extensions(buff + ntohs(fixed->ar_extoff),&values);
  else {
    printf("mpcd: p_recogn.c: warning: ");
    printf("no extensions in MPOA Egress Cache Purge Reply\n");
    return -1;
  }
  if (values.dll_header_present == 0) {
    printf("mpcd: p_recogn.c: warning: ");
    printf("DLL Header Extension missing in MPOA Egress Cache Purge Reply\n");
    return -1;
  }
  msg.content.eg_info.cache_id = values.dll_ext.cache_id;
  msg.type = EGRESS_PURGE_RCVD;
  memcpy(msg.MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN);
  return send_to_kernel(&msg);
}

static int mpoa_keep_alive(uint8_t *buff){
  int pos = 0;
  uint32_t sequence_nmbr;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h *)buff;
  struct extension_values values;
  struct nhrp_common_h_no_ip *common;
  memset(&values, 0, sizeof(struct extension_values));
  pos += sizeof(struct nhrp_fixed_h);
  common = (struct nhrp_common_h_no_ip *)(buff + pos); 
  sequence_nmbr = ntohl(common->request_ID);
  if(memcmp(common->src_nbma_address,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN)){
    printf("mpcd: p_recogn.c: new MPS! \n" );
    return -1;
  }
  if(ntohs(fixed->ar_extoff)){
    parse_extensions(buff + ntohs(fixed->ar_extoff),&values);
  }
  /* if extensions were missing sequence_nmbr == 0  => MPS Death */
  keep_alive_sm(values.keep_alive_lifetime, sequence_nmbr);
  return 1;
}

static int mpoa_trigger( uint8_t * buff ){
  int pos = 0;
  struct k_message msg;
  struct nhrp_common_h *common;
  struct extension_values values;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h*)buff;
  memset(&values, 0, sizeof(struct extension_values));
  pos += sizeof(struct nhrp_fixed_h);
  common = (struct nhrp_common_h *)(buff + pos);
  pos += sizeof(struct nhrp_common_h);
  memset(&msg,0,sizeof(struct k_message));
  if(ntohs(fixed->ar_extoff)) {
      printf("mpcd: p_recogn.c: mpoa_trigger: ar$extoff in Fixed Header != 0\n" );
      parse_extensions(buff+ntohs(fixed->ar_extoff),&values);
  }
  if(!common->dst_proto_len){
    printf("mpcd: p_recogn.c: mpoa_trigger: no destination ip to trigger! \n");
    return -1;
  }
  if(common->src_proto_len)
    msg.content.in_info.in_dst_ip = common->dst_protocol_address;
  /* 
   * If src_proto_len == 0 dst_protocol_address is found in "place"
   * of dst_ptocol_address.
   */
  else
    msg.content.in_info.in_dst_ip = common->src_protocol_address;
  memcpy(msg.MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN);
  msg.type = MPOA_TRIGGER_RCVD;
  send_to_kernel(&msg);
  return 1;
}



static int mpoa_resolution_reply( uint8_t * buff ){
  int pos = 0;
  struct k_message msg;
  struct extension_values values;
  struct nhrp_common_h *common;
  struct nhrp_cie *cie;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h*)buff;
  memset(&values, 0, sizeof(struct extension_values));
  memset(&msg,0,sizeof(struct k_message));
  pos += sizeof(struct nhrp_fixed_h);
  common = (struct nhrp_common_h*)(buff + pos);
  if(!check_incoming(ntohl(common->request_ID),MPOA_RESOLUTION_REQUEST))
    return -1;
  pos += sizeof(struct nhrp_common_h);
  cie = (struct nhrp_cie*)(buff + pos);
  if(cie->code){
    print_cie_code(cie->code);
    return -1;
  }
  msg.content.in_info.holding_time = ntohs(cie->holding_time);
  if(fixed->ar_extoff)
    pos += parse_extensions(buff + ntohs(fixed->ar_extoff), &values);
  if (values.egress_cache_tag_ext_present == 0) {
          printf("mpcd: p_recogn.c: warning: ");
          printf("received MPOA Resolution Reply ");
          printf("with no Egress Cache Tag Extension\n");
  }
  if(values.tag_present && values.tag == 0) {
          printf("mpcd: p_recogn.c: warning: ");
          printf("received MPOA Resolution Reply ");
          printf("with Egress Cache Tag Extension where tag == 0\n");
          values.tag_present = 0;
  }
  if(values.tag_present){
    msg.content.in_info.tag = values.tag;
  }
  msg.type = MPOA_RES_REPLY_RCVD;
  msg.content.in_info.in_dst_ip  = common->dst_protocol_address;
  memcpy(msg.content.in_info.eg_MPC_ATM_addr,cie->cli_nbma_address,ATM_ESA_LEN);
  memcpy(msg.MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR, ATM_ESA_LEN);
  if(values.service_category_present)
    msg.qos.txtp.traffic_class = service_category_to_traff_class(values.service_category);
  else
    msg.qos.txtp.traffic_class = ATM_UBR;
  send_to_kernel(&msg);
  keep_alive_sm_running = 1;
  return MPOA_RESOLUTION_REPLY;
}


static int checksum_check(uint8_t *buff){
  uint16_t checksum;
  int length;
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h*)buff;
  checksum = fixed->ar_chksum;
  fixed->ar_chksum = 0;
  length = ntohs(fixed->ar_pktsz);
  if(checksum != compute_ip_csum(buff,length))
    return 0;
  return 1;
}

static unsigned char service_category_to_traff_class(uint32_t service_category){
  unsigned char traffic_class = 0;
  if(service_category & CBR)
    traffic_class |= ATM_CBR;
  else
    traffic_class |= ATM_UBR;
  return traffic_class;
}

int recognize_packet(uint8_t * buff){
  struct nhrp_fixed_h *fixed = (struct nhrp_fixed_h *)buff;
  uint8_t  type;
  if(!checksum_check(buff)){
    printf("mpcd: p_recogn.c: checksum error!\n");
    return -1;
  }
  if (fixed->ar_extoff > (fixed->ar_pktsz - sizeof(struct nhrp_extension))) {
          printf("mpcd: p_recogn.c: extension offset beyond packet limits!\n");
          return -1;
  }
  type = fixed->ar_op_type;
  dprintf("mpcd: p_recogn.c: ");
  switch(type){
  case NHRP_PURGE_REQUEST :
    dprintf("purge request received.\n");
    return nhrp_purge_request(buff);
  case MPOA_CACHE_IMPOSITION_REQUEST :
    dprintf("cache imposition request recieved.\n");
    return mpoa_cache_imposition_request(buff);
  case MPOA_EGRESS_CACHE_PURGE_REPLY :
    dprintf("mpoa egress cache purge reply recieved.\n");
    return mpoa_egress_cache_purge_reply(buff);
  case MPOA_KEEP_ALIVE : 
    dprintf("keep alive recieved. \n");
    return mpoa_keep_alive(buff);
  case MPOA_TRIGGER :
    printf("mpoa trigger recieved. \n");
    return mpoa_trigger(buff);
  case MPOA_RESOLUTION_REPLY:
    dprintf("mpoa resolution reply recieved. \n");
    return mpoa_resolution_reply(buff); 
  default:
    printf("p_recogn.c: unrecognized packet: %d\n",type);
    return -1;
  }
}
