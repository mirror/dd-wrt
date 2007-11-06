/** @file tlv.h
 *
 * OpenLLDP TLV Header
 *
 * Licensed under a dual GPL/Proprietary license.  
 * See LICENSE file for more info.
 * 
 * File: lldp_tlv.h
 * 
 * Authors: Terry Simons (terry.simons@gmail.com)
 *
 *******************************************************************/

#ifndef LLDP_TLV_H
#define LLDP_TLV_H

#include "lldp_port.h"

/* TLV Types from section 9.4.1 of IEEE 802.1AB */
#define END_OF_LLDPDU_TLV       0    /* MANDATORY */
#define CHASSIS_ID_TLV          1    /* MANDATORY */
#define PORT_ID_TLV             2    /* MANDATORY */
#define TIME_TO_LIVE_TLV        3    /* MANDATORY */
#define PORT_DESCRIPTION_TLV    4    /* OPTIONAL  */
#define SYSTEM_NAME_TLV         5    /* OPTIONAL  */
#define SYSTEM_DESCRIPTION_TLV  6    /* OPTIONAL  */
#define SYSTEM_CAPABILITIES_TLV 7    /* OPTIONAL  */
#define MANAGEMENT_ADDRESS_TLV  8    /* OPTIONAL  */
/* 9 - 126 are reserved */
#define ORG_SPECIFIC_TLV        127  /* OPTIONAL */
/* TLV Subtypes from section 9 of IEEE 802.1AB */

/* Chassis ID TLV Subtypes */
/* 0 is reserved */
#define CHASSIS_ID_CHASSIS_COMPONENT 1
#define CHASSIS_ID_INTERFACE_ALIAS   2
#define CHASSIS_ID_PORT_COMPONENT    3
#define CHASSIS_ID_MAC_ADDRESS       4
#define CHASSIS_ID_NETWORK_ADDRESS   5
#define CHASSIS_ID_INTERFACE_NAME    6
#define CHASSIS_ID_LOCALLY_ASSIGNED  7 
/* 8-255 are reserved */
/* End Chassis ID TLV Subtypes */

/* Port ID TLV Subtypes */
/* 0 is reserved */
#define PORT_ID_INTERFACE_ALIAS  1
#define PORT_ID_PORT_COMPONENT   2
#define PORT_ID_MAC_ADDRESS      3
#define PORT_ID_NETWORK_ADDRESS  4
#define PORT_ID_INTERFACE_NAME   5
#define PORT_ID_AGENT_CIRCUIT_ID 6
#define PORT_ID_LOCALLY_ASSIGNED 7
/* 8-255 are reserved */ 
/* End Port ID TLV Subtypes */

/* System Capabilities TLV Subtypes */
#define SYSTEM_CAPABILITY_OTHER     1
#define SYSTEM_CAPABILITY_REPEATER  2
#define SYSTEM_CAPABILITY_BRIDGE    4
#define SYSTEM_CAPABILITY_WLAN      8
#define SYSTEM_CAPABILITY_ROUTER    16
#define SYSTEM_CAPABILITY_TELEPHONE 32
#define SYSTEM_CAPABILITY_DOCSIS    64
#define SYSTEM_CAPABILITY_STATION   128
/* 8 - 15 reserved */
/* End System Capabilities TLV Subtypes */

/* End TLV Subtypes from section 9 of IEEE 802.1AB */

struct lldp_tlv_validation_errors {
  uint64_t errors;
  uint8_t chassis_id_tlv_count;
  uint8_t port_id_tlv_count;
  uint8_t ttl_tlv_count;
  uint8_t port_description_tlv_count;
  uint8_t system_name_tlv_count;
  uint8_t system_description_tlv_count;
  uint8_t system_capabilities_tlv_count;
  uint8_t management_address_tlv_count;
};

struct lldp_test_case {
  // Pointer to a tlv template
  struct lldp_tlv_template *test_case;

  // Pointer to the next test case
  struct lldp_test_cases *next;
};

struct lldp_tlv *create_end_of_lldpdu_tlv(struct lldp_port *lldp_port);
uint8_t validate_end_of_lldpdu_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_chassis_id_tlv(struct lldp_port *lldp_port);
uint8_t validate_chassis_id_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_port_id_tlv(struct lldp_port *lldp_port);
uint8_t validate_port_id_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_ttl_tlv(struct lldp_port *lldp_port);
uint8_t validate_ttl_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_port_description_tlv(struct lldp_port *lldp_port);
uint8_t validate_port_description_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_system_name_tlv(struct lldp_port *lldp_port);
uint8_t validate_system_name_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_system_description_tlv(struct lldp_port *lldp_port);
uint8_t validate_system_description_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_system_capabilities_tlv(struct lldp_port *lldp_port);
uint8_t validate_system_capabilities_tlv(struct lldp_tlv *tlv);

struct lldp_tlv *create_management_address_tlv(struct lldp_port *lldp_port);
uint8_t validate_management_address_tlv(struct lldp_tlv *tlv);

// Should probably allow this create function to specify the OUI in question?
struct lldp_tlv *create_organizationally_specific_tlv(struct lldp_port *lldp_port, uint8_t *oui);
uint8_t validate_organizationally_specific_tlv(struct lldp_tlv *tlv);

extern uint8_t (*validate_tlv[])(struct lldp_tlv *tlv);

#define LLDP_BEGIN_RESERVED_TLV 9
#define LLDP_END_RESERVED_TLV 126

#define XVALIDTLV     0
#define XEINVALIDTLV -1


void decode_tlv_subtype(struct lldp_tlv *tlv);
void decode_oui_tlv(struct lldp_tlv *tlv);
void decode_tlv_system_capabilities( uint16_t system_capabilities, uint16_t enabled_capabilities);
char *capability_name(uint16_t capability);
char *tlv_typetoname(uint8_t tlv_type);
uint8_t tlvcpy(struct lldp_tlv *dst, struct lldp_tlv *src);
struct lldp_tlv *tlvpop(uint8_t *buffer);
uint8_t tlvpush(uint8_t *buffer, struct lldp_tlv *tlv);

uint8_t lldp_cache_tlv(struct lldp_tlv *tlv);

uint8_t tlvInitializeLLDP();
void tlvCleanupLLDP();

uint8_t initializeTLVFunctionValidators();


struct lldp_tlv *initialize_tlv();

#endif /* LLDP_TLV_H */
