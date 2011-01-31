#include <atm.h>
#include <linux/types.h>

#ifndef PACKETS_H
#define PACKETS_H

struct mpc_control{
  int kernel_socket;         /* socket for mpoad <--> kernel messages    */
  int MPS_socket;            /* MPS control socket for outgoing msgs     */
  int MPS_listen_socket;     /* listen socket for incoming control calls */
  int mps_ctrl_addr_set;
  unsigned char MPS_CTRL_ATM_ADDR[ATM_ESA_LEN];
  int mps_mac_addr_set;
  unsigned char MPS_MAC_ADDRESS[13];
  unsigned char OWN_ATM_ADDRESS[ATM_ESA_LEN];/* Our control ATM address   */
  struct sockaddr_atmsvc data_listen_addr;   /* Our data listen address   */
  int use_lecs;                           /* Use LECS for configuration info */
  struct sockaddr_atmsvc lecs_address;
  unsigned char LEC_ADDRESS[ATM_ESA_LEN];
  unsigned char elan_name[33];
  int INTERFACE_NUMBER;                      /* Interface number, x for lecx */
};                                



/* p_factory.c */

int send_resolution_request(
                            uint32_t rqst_id,
                            uint16_t source_ip_present, 
                            uint32_t dest_ip, 
                            uint8_t prefix_length, 
                            uint32_t service_category 
                            );

int send_cache_imposition_reply(
                                uint8_t * pRequest,
				uint32_t  tag, 
				uint8_t   code,
                                uint8_t   prefix_length,
                                uint16_t  mtu,
                                uint8_t * cli_atm_addr
                                );

int send_egress_cache_purge_request(
                                    uint16_t no_reply,
                                    uint32_t eg_MPS_ip_addr,
                                    uint8_t  prefix,
                                    uint32_t purge_ip,
                                    uint32_t cache_id
                                    );
             
int send_purge_request(
                       uint32_t src_ip,
                       uint8_t prefix_length,
                       uint32_t purge_ip,
		       int shortcut_fd
                       );

int send_purge_reply( 
                     uint8_t * pBuff
                     );

unsigned short compute_ip_csum(
			       unsigned char *addr, 
			       int count
			       );

/* p_recogn.c */


int recognize_packet( 
		     uint8_t * pBuff 
		     );

/* id_list.c */

int new_id(
	   uint32_t id,
	   uint32_t cache_id,
	   uint8_t type
	   );

uint32_t search_by_type(
			uint8_t type, 
			uint32_t cache_id
			);

void clear_expired(void);

int check_incoming(
		   uint32_t id, 
		   uint8_t type
		   );

/* tag_list.c */

uint32_t new_tag(uint32_t cache_id);

int remove_tag(uint32_t tag);

/* NHRP fixed header */

struct nhrp_fixed_h {
  uint16_t    ar_afn;     
  uint16_t    ar_pro_type;
  uint8_t     ar_pro_snap[5];  /* == 0        */ 
  uint8_t     ar_hopcnt;
  uint16_t    ar_pktsz;
  uint16_t    ar_chksum; 
  uint16_t    ar_extoff;
  uint8_t     ar_op_version;   /* 0x01 (NHRP) */
  uint8_t     ar_op_type;
  uint8_t     ar_shtl;
  uint8_t     ar_sstl;         /* == 0 no subaddress concept */
};

/* NHRP common header */

struct nhrp_common_h {
  uint8_t     src_proto_len;
  uint8_t     dst_proto_len;
  uint16_t    flags;
  uint32_t    request_ID;
  uint8_t     src_nbma_address[ATM_ESA_LEN];     
  uint32_t    src_protocol_address;     /* IP-address */
  uint32_t    dst_protocol_address;          
}; 

/* NHRP common header without source (or dest.) IP-address */

struct nhrp_common_h_short {
  uint8_t               src_proto_len;
  uint8_t               dst_proto_len;
  uint16_t              flags;
  uint32_t              request_ID;
  uint8_t               src_nbma_address[ATM_ESA_LEN];     
  uint32_t              dst_protocol_address;          
}; 

/* NHRP common header without source or dest IP-addresses */

struct nhrp_common_h_no_ip {
  uint8_t              src_proto_len;
  uint8_t              dst_proto_len;
  uint16_t             flags;
  uint32_t             request_ID;
  uint8_t              src_nbma_address[ATM_ESA_LEN];     
}; 


/* NHRP CIE */

struct nhrp_cie {
   uint8_t              code;
   uint8_t              prefix_length;
   uint16_t             unused;
   uint16_t             mtu;
   uint16_t             holding_time;
   uint8_t              cli_addr_tl;
   uint8_t              cli_saddr_tl;
   uint8_t              cli_proto_len;
   uint8_t              preference;
   uint8_t              cli_nbma_address[ATM_ESA_LEN];     
   uint32_t             cli_protocol_address;
};

/* NHRP CIE without IP-address */
struct nhrp_cie_no_ip {
   uint8_t               code;
   uint8_t               prefix_length;
   uint16_t              unused;
   uint16_t              mtu;
   uint16_t              holding_time;
   uint8_t               cli_addr_tl;
   uint8_t               cli_saddr_tl;
   uint8_t               cli_proto_len;
   uint8_t               preference;
   uint8_t               cli_nbma_address[ATM_ESA_LEN];     
};

/* NHRP CIE without NBMA addresses */

struct nhrp_cie_no_nbma {
   uint8_t      code;
   uint8_t      prefix_length;
   uint16_t     unused;
   uint16_t     mtu;
   uint16_t     holding_time;
   uint8_t      cli_addr_tl;
   uint8_t      cli_saddr_tl;
   uint8_t      cli_proto_len;
   uint8_t      preference;
   uint32_t     cli_protocol_address;
};

/* NHRP CIE without any addresses */
struct nhrp_cie_short {
   uint8_t      code;
   uint8_t      prefix_length;
   uint16_t     unused;
   uint16_t     mtu;
   uint16_t     holding_time;
   uint8_t      cli_addr_tl;
   uint8_t      cli_saddr_tl;
   uint8_t      cli_proto_len;
   uint8_t      preference;     
};


/* NHRP error indication */

struct nhrp_error_indication {
   uint8_t              src_proto_len;
   uint8_t              dst_proto_len;
   uint16_t             unused;
   uint16_t             error_code;
   uint16_t             error_offset;
   uint8_t              src_nbma_address[ATM_ESA_LEN];      
   uint32_t             src_protocol_address;     
   uint32_t             dst_protocol_address;
};

/* NHRP extension */

struct nhrp_extension {
  uint16_t   type;  /* includes C and u bits */
  uint16_t   length;
};


struct nhrp_extension_with_value {
  uint16_t   type;  
  uint16_t   length;
  uint32_t   value;
};

struct dll_header_extension {
  uint16_t type;
  uint16_t length;
  uint32_t cache_id;
  uint32_t elan_id;
  uint8_t dh_length;
  uint8_t dll_header[256];
} __attribute__ ((packed)); /* without ((packed)) sizeof() was 273 */

 /* struct is filled by parse_extensions() */

struct extension_values{
  uint32_t dll_header_present;
  struct dll_header_extension dll_ext;
  uint32_t egress_cache_tag_ext_present;
  uint32_t tag_present;
  uint32_t tag;
  uint32_t service_category_present;
  uint32_t service_category;
  uint32_t keep_alive_lifetime_present;
  uint32_t keep_alive_lifetime;
  uint32_t hop_count_present;
  uint32_t hop_count;
  uint32_t error_code_present;
  uint32_t error_code;
};

int parse_extensions(uint8_t *buff, struct extension_values *values);

  
/* Constants for fixed header */

#define AR_AFN_NSAP        3
#define AR_AFN_E164        8
#define AR_OP_VERSION_NHRP 1
#define AR_SHTL_NSAP       0x14
#define AR_PRO_TYPE_ETHER  0x0800 
#define AR_PRO_SNAP_ETHER  0

/* Constants for common header */

#define PROTO_LEN_IP   4

/* Default values */

#define MTU_DEFAULT 56325
#define MAX_PREFIX_LENGTH 0x20

/* Extension type codes */

#define NHRP_END_OF_EXTENSIONS              0x8000 /* C == 1 */
#define NHRP_RESPONDER_ADDRESS_EXTENSION    0x8003
#define NHRP_FWD_TRANSIT_NHS_REC_EXTENSION  0x8004 
#define NHRP_REV_TRANSIT_NHS_REC_EXTENSION  0x8005 
#define NHRP_AUTHENTICATION_EXTENSION       0x8007 
#define NHRP_VENDORPRIVATE_EXTENSION        0x8008 

#define MPOA_DLL_HEADER_EXTENSION           0x9000 
#define MPOA_EGRESS_CACHE_TAG_EXTENSION     0x1001 /* C == 0 */
#define MPOA_ATM_SERVICE_CATEGORY_EXTENSION 0x1002
#define MPOA_KEEPALIVE_LIFETIME_EXTENSION   0x1003
#define MPOA_HOP_COUNT_EXTENSION            0x1004
#define MPOA_ORIGINAL_ERROR_CODE_EXTENSION  0x1005

/* Type codes (ar$op.type) */

#define NHRP_RESOLUTION_REQUEST   0x01
#define NHRP_RESOLUTION_REPLY     0x02
#define NHRP_REGISTRATION_REQUEST 0x03
#define NHRP_REGISTRATION_REPLY   0x04
#define NHRP_PURGE_REQUEST        0x05
#define NHRP_PURGE_REPLY          0x06
#define NHRP_ERROR_INDICATION     0x07

#define MPOA_CACHE_IMPOSITION_REQUEST   0x80
#define MPOA_CACHE_IMPOSITION_REPLY     0x81
#define MPOA_EGRESS_CACHE_PURGE_REQUEST 0x82
#define MPOA_EGRESS_CACHE_PURGE_REPLY   0x83
#define MPOA_KEEP_ALIVE                 0x84
#define MPOA_TRIGGER                    0x85
#define MPOA_RESOLUTION_REQUEST         0x86
#define MPOA_RESOLUTION_REPLY           0x87


/* Error codes. Used in the "Error Code"-field of an  
Error Indication packet. */

#define UNRECOGNIZED_EXTENSION       0x0001
#define NHRP_LOOP_DETECTED           0x0003
#define PROTOCOL_ADDRESS_UNREACHABLE 0x0006
#define PROTOCOL_ERROR               0x0007
#define NHRP_SDU_SIZE_EXCEEDED       0x0008
#define INVALID_EXTENSION            0x0009
#define INVALID_RESOLUTION_REPL_RCVD 0x000a
#define AUTHENTICATION_FAILURE       0x000b
#define HOP_COUNT_EXCEEDED           0x000f

/* NHRP CIE codes */

#define ADMINISTRATIVELY_PROHIBITED              0x04
#define INSUFFICIENT_RESOURCES                   0x05
#define NO_INET_ADDR_TO_NBMA_ADDR_BINDING_EXISTS 0x0c
#define UNIQUE_INET_ADDR_ALREADY_REGISTERED      0x0e



/* MPOA CIE codes */

#define SUCCESS                                0x00
#define INSUFF_RES_TO_EGRESS_CACHE_ENTRTY      0x81
#define INSUFF_RES_TO_SHORTCUT                 0x82
#define INSUFF_RES_TO_EITHER_ENTRY_OR_SHORTCUT 0x83
#define UNSUPPORTED_INTERN_LAYER_PROTO         0x84
#define UNSUPPORTED_MAC_LAYER_PROTO            0x85
#define NOT_AN_MPC                             0x86
#define NOT_AN_MPS                             0x87
#define UNSPECIFIED                            0x88


/* FLAGS. The meaning of the flags differ with the type of the packet. */

#define FLAG_Q 0x8000
#define FLAG_A 0x4000
#define FLAG_D 0x2000

#define FLAG_U 0x1000
#define FLAG_S 0x0800
#define FLAG_N 0x8000

/* Lifetime of struct id's in the id_list */

#define ID_EXPIRING_TIME 600

/* ATM service categories */

#define RT_VBR  0x0001
#define NRT_VBR 0x0002
#define ABR     0x0004
#define CBR     0x0008

#endif /* PACKETS_H */








