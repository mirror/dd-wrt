/*******************************************************************
 *
 * OpenLLDP TLV
 *
 * Licensed under a dual GPL/Proprietary license.  
 * See LICENSE file for more info.
 * 
 * File: lldp_tlv.c
 * 
 * Authors: Terry Simons (terry.simons@gmail.com)
 *
 *******************************************************************/

#include <arpa/inet.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lldp_debug.h"
#include "tlv.h"
#include "tlv_common.h"
#include "lldp_port.h"
#include "lldp_neighbor.h"

/* There are a max of 128 TLV validators (types 0 through 127), so we'll stick them in a static array indexed by their tlv type */
uint8_t (*validate_tlv[128])(struct lldp_tlv *tlv) = {
  validate_end_of_lldpdu_tlv,        /* 0 End of LLDPU TLV        */
  validate_chassis_id_tlv,          /* 1 Chassis ID TLV          */
  validate_port_id_tlv,             /* 2 Port ID TLV             */
  validate_ttl_tlv,                 /* 3 Time To Live TLV        */
  validate_port_description_tlv,    /* 4 Port Description TLV    */
  validate_system_name_tlv,         /* 5 System Name TLV         */
  validate_system_description_tlv,  /* 6 System Description TLV  */
  validate_system_capabilities_tlv, /* 7 System Capabilities TLV */
  validate_management_address_tlv,  /* 8 Management Address TLV  */
  /* 9 - 126 are reserved and set to NULL in lldp_tlv_validator_init()                        */
  /* 127 is populated for validate_organizationally_specific_tlv in lldp_tlv_validator_init() */
};

# warning Write a test suite that will run all the possible combinations of decode
void decode_tlv_subtype(struct lldp_tlv *tlv)
{
  switch(tlv->type)
    {
    case CHASSIS_ID_TLV:
      {
	uint8_t *chassis_id = &tlv->info_string[1];

	debug_printf(DEBUG_TLV, "Chassis ID: ");

	switch(tlv->info_string[0])
	  {
	  case CHASSIS_ID_CHASSIS_COMPONENT:
	    {
	      debug_printf(DEBUG_TLV, "Chassis Component\n");
	    }break;
	  case CHASSIS_ID_INTERFACE_ALIAS:
	    {
	      debug_printf(DEBUG_TLV, "Interface Alias\n");
	    }break;
	  case CHASSIS_ID_PORT_COMPONENT:
	    {
	      debug_printf(DEBUG_TLV, "Port Component");
	    }break;
	  case CHASSIS_ID_MAC_ADDRESS:
	    {
	      debug_printf(DEBUG_TLV, "MAC Address\n");
	    }break;
	  case CHASSIS_ID_NETWORK_ADDRESS:
	    {
	      debug_printf(DEBUG_TLV, "Network Address\n");
	    }break;
	  case CHASSIS_ID_INTERFACE_NAME:
	    {
	      debug_printf(DEBUG_TLV, "Interface Name\n");
	    }break;
	  case CHASSIS_ID_LOCALLY_ASSIGNED:
	    {
	      debug_printf(DEBUG_TLV, "Locally Assigned\n");
	    }break;
	  default:
	    {
	      debug_printf(DEBUG_TLV, "Reserved (%d)\n", tlv->info_string);
	    }
	  };
	debug_hex_dump(DEBUG_TLV, chassis_id, (tlv->length - 1));
      }break;
    case PORT_ID_TLV:
      {
	uint8_t *port_id = &tlv->info_string[1];

	debug_printf(DEBUG_TLV, "Port ID: ");
	
	switch(tlv->info_string[0])
	  {
	  case PORT_ID_INTERFACE_ALIAS:
	    {
	      debug_printf(DEBUG_TLV, "Interface Alias\n");
	    }break;
	  case PORT_ID_PORT_COMPONENT:
	    {
	      debug_printf(DEBUG_TLV, "Port Component\n");
	    }break;
	  case PORT_ID_MAC_ADDRESS:
	    {
	      debug_printf(DEBUG_TLV, "MAC Address\n");
	    }break;
	  case PORT_ID_NETWORK_ADDRESS:
	    {
	      debug_printf(DEBUG_TLV, "Network Address\n");
	    }break;
	  case PORT_ID_INTERFACE_NAME:
	    {
	      debug_printf(DEBUG_TLV, "Interface Name\n");
	    }break;
	  case PORT_ID_LOCALLY_ASSIGNED:
	    {
	      debug_printf(DEBUG_TLV, "Locally Assigned\n");
	    }break;
	  default:
	    {
	      debug_printf(DEBUG_TLV, "Reserved (%d)\n", tlv->info_string);
	    }
	  };
	debug_hex_dump(DEBUG_TLV, port_id, tlv->length - 1);

      }break;
    case TIME_TO_LIVE_TLV:
      {
        uint16_t *ttl = (uint16_t *)&tlv->info_string[0];

	debug_printf(DEBUG_TLV, "Time To Live: %d seconds\n", htons(*ttl));
      }break;
    case PORT_DESCRIPTION_TLV:
      {
	char *port_description = malloc(tlv->length + 1);
	bzero(port_description, tlv->length + 1);

	memcpy(port_description, tlv->info_string, tlv->length);

	debug_printf(DEBUG_TLV, "Port Description: %s\n", port_description);

	free(port_description);
      }break;
    case SYSTEM_NAME_TLV:
      {
	debug_printf(DEBUG_TLV, "System Name: %s\n", tlv->info_string);
      }break;
    case SYSTEM_DESCRIPTION_TLV:
      {
	debug_printf(DEBUG_TLV, "System Description: %s\n", tlv->info_string);
      }break;
    case SYSTEM_CAPABILITIES_TLV:
      {
	uint16_t *system_capabilities = (uint16_t *)&tlv->info_string[0];
	uint16_t *enabled_capabilities = (uint16_t *)&tlv->info_string[2];

	decode_tlv_system_capabilities(htons(*system_capabilities), htons(*enabled_capabilities));
      }break;
    case MANAGEMENT_ADDRESS_TLV:
      {
      }break;
    case ORG_SPECIFIC_TLV:
      {
      }break;
    case END_OF_LLDPDU_TLV:
      {
      }break;
    default:
      debug_printf(DEBUG_NORMAL, "Got unrecognized type '%d'\n", tlv->type);
    };
}

void decode_tlv_system_capabilities( uint16_t system_capabilities, uint16_t enabled_capabilities)
{
  int capability = 0;

  debug_printf(DEBUG_TLV, "System Capabilities: %d, Enabled Capabilities: %d\n", system_capabilities, enabled_capabilities);

  debug_printf(DEBUG_TLV, "Capabilities:\n");

  for(capability = 1; capability <= 16; capability *= 2)
    {
      if((system_capabilities & capability))
	{
	  if(enabled_capabilities & capability)
	    {
	      debug_printf(DEBUG_TLV, "%s (enabled)\n", capability_name(capability));
	    }
	  else
	    {
	      debug_printf(DEBUG_TLV, "%s (disabled)\n", capability_name(capability));
	    }
	} 
    }
}

char *capability_name(uint16_t capability)
{
  switch(capability)
    {
    case SYSTEM_CAPABILITY_OTHER:
      return "Other";
    case SYSTEM_CAPABILITY_REPEATER:
      return "Repeater/Hub";
    case SYSTEM_CAPABILITY_BRIDGE:
      return "Bridge/Switch";
    case SYSTEM_CAPABILITY_WLAN:
      return "Wireless LAN";
    case SYSTEM_CAPABILITY_ROUTER:
      return "Router";
    case SYSTEM_CAPABILITY_TELEPHONE:
      return "Telephone";
    case SYSTEM_CAPABILITY_DOCSIS:
      return "DOCSIS/Cable Modem";
    case SYSTEM_CAPABILITY_STATION:
      return "Station";
    default:
      return "Unknown";
    };
}


char *tlv_typetoname(uint8_t tlv_type)
{
  switch(tlv_type)
    {
    case CHASSIS_ID_TLV:
      return "Chassis ID";
      break;
    case PORT_ID_TLV:
      return "Port ID";
      break;
    case TIME_TO_LIVE_TLV:
      return "Time To Live";
      break;
    case PORT_DESCRIPTION_TLV:
      return "Port Description";
      break;
    case SYSTEM_NAME_TLV:
      return "System Name";
      break;
    case SYSTEM_DESCRIPTION_TLV:
      return "System Description";
      break;
    case SYSTEM_CAPABILITIES_TLV:
      return "System Capabiltiies";
      break;
    case MANAGEMENT_ADDRESS_TLV:
      return "Management Address";     
      break;
    case ORG_SPECIFIC_TLV:
      return "Organizationally Specific";
      break;
    case END_OF_LLDPDU_TLV:
      return "End Of LLDPDU";
      break;
    default:
      return "Unknown"; 
    };
}

uint8_t tlvInitializeLLDP(struct lldp_port *lldp_port)
{
  return 0;
}

void tlvCleanupLLDP()
{
  
}


uint8_t initializeTLVFunctionValidators()
{
  int index = 0;

  /* Set all of the reserved TLVs to NULL validator functions */
  /* so they're forced to go through the generic validator    */ 
  for(index = LLDP_BEGIN_RESERVED_TLV; index < LLDP_END_RESERVED_TLV; index++)
    {
      //debug_printf(DEBUG_EXCESSIVE, "Setting TLV Validator '%d' to NULL - it's reserved\n", index);

      validate_tlv[index] = NULL;
    }

  debug_printf(DEBUG_EXCESSIVE, "Setting TLV Validator '%d' - it's the organizational specific TLV validator...\n", ORG_SPECIFIC_TLV);

  validate_tlv[ORG_SPECIFIC_TLV] = validate_organizationally_specific_tlv;

  return 0;
}

struct lldp_tlv *initialize_tlv() {
  struct lldp_tlv *tlv = malloc(sizeof(struct lldp_tlv));

  if(tlv) {
    bzero(tlv, sizeof(struct lldp_tlv));
    
    return tlv;  
  }

  return NULL;
}

void destroy_tlv(struct lldp_tlv **tlv) {
  free((*tlv)->info_string);
  free(*tlv);
}

void destroy_flattened_tlv(struct lldp_flat_tlv **tlv) {
  free((*tlv)->tlv);
  free(*tlv);
}

struct lldp_tlv *create_end_of_lldpdu_tlv(struct lldp_port *lldp_port) {

  struct lldp_tlv* tlv = initialize_tlv();

  tlv->type = END_OF_LLDPDU_TLV; // Constant defined in lldp_tlv.h
  tlv->length = 0;     // The End of LLDPDU TLV is length 0.

  tlv->info_string = NULL;

  return tlv;
}

uint8_t validate_end_of_lldpdu_tlv(struct lldp_tlv *tlv)
{
  if(tlv->length != 0)
    { 
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV type is 'End of LLDPDU' (0), but TLV length is %d when it should be 0!\n", tlv->length);

      return XEINVALIDTLV;
    }
  
  return XVALIDTLV;
}

uint8_t validate_length_max_255(struct lldp_tlv *tlv)
{
  //Length will never be below 0 because the variable used is unsigned... 
  if(tlv->length > 255)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tIt should be between 0 and 255 inclusive!\n", tlv->length);
      
      return XEINVALIDTLV;
    }
  
  return XVALIDTLV;
}

uint8_t validate_length_max_256(struct lldp_tlv *tlv)
{
  if(tlv->length < 2 || tlv->length > 256)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tIt should be between 2 and 256 inclusive!\n", tlv->length);
      
      return XEINVALIDTLV;
    }
  
  return XVALIDTLV;
}

struct lldp_tlv *create_chassis_id_tlv_invalid_length(struct lldp_port *lldp_port) {

  struct lldp_tlv* tlv = initialize_tlv();

  tlv->type = CHASSIS_ID_TLV; // Constant defined in lldp_tlv.h
  tlv->length = 7; //The size of a MAC + the size of the subtype (1 byte)

  tlv->info_string = malloc(tlv->length);  

  // CHASSIS_ID_MAC_ADDRESS is a 1-byte value - 4 in this case. Defined in lldp_tlv.h
  tlv->info_string[0] = CHASSIS_ID_MAC_ADDRESS;

  // We need to start copying at the 2nd byte, so we use [1] here...
  // This reads "memory copy to the destination at the address of tlv->info_string[1] with the source my_mac for 6 bytes" (the size of a MAC address)
  memcpy(&tlv->info_string[1], &lldp_port->source_mac[0], 6);

  return tlv;
}

struct lldp_tlv *create_chassis_id_tlv(struct lldp_port *lldp_port) {

  struct lldp_tlv* tlv = initialize_tlv();

  tlv->type = CHASSIS_ID_TLV; // Constant defined in lldp_tlv.h
  tlv->length = 7; //The size of a MAC + the size of the subtype (1 byte)

  tlv->info_string = malloc(tlv->length);  

  // CHASSIS_ID_MAC_ADDRESS is a 1-byte value - 4 in this case. Defined in lldp_tlv.h
  tlv->info_string[0] = CHASSIS_ID_MAC_ADDRESS;

  // We need to start copying at the 2nd byte, so we use [1] here...
  // This reads "memory copy to the destination at the address of tlv->info_string[1] with the source my_mac for 6 bytes" (the size of a MAC address)
  memcpy(&tlv->info_string[1], &lldp_port->source_mac[0], 6);

  return tlv;
}

uint8_t validate_chassis_id_tlv(struct lldp_tlv *tlv)
{
  // Several TLVs have this requirement.
  return validate_length_max_256(tlv);
}

struct lldp_tlv *create_port_id_tlv(struct lldp_port *lldp_port) {

  struct lldp_tlv* tlv = initialize_tlv();

  tlv->type = PORT_ID_TLV; // Constant defined in lldp_tlv.h
  tlv->length = 1 + strlen(lldp_port->if_name); //The length of the interface name + the size of the subtype (1 byte)

  tlv->info_string = malloc(tlv->length);
  bzero(tlv->info_string, tlv->length);

  // PORT_ID_INTERFACE_NAME is a 1-byte value - 5 in this case. Defined in lldp_tlv.h
  tlv->info_string[0] = PORT_ID_INTERFACE_NAME;


    // We need to start copying at the 2nd byte, so we use [1] here...
    // This reads "memory copy to the destination at the address of tlv->info_string[1] with the source lldp_port->if_name for strlen(lldp_port->if_name) bytes"
  memcpy(&tlv->info_string[1], lldp_port->if_name, strlen(lldp_port->if_name));

 return tlv;
}

uint8_t validate_port_id_tlv(struct lldp_tlv *tlv)
{
  // Several TLVs have this requirement.
  return validate_length_max_256(tlv);
}

struct lldp_tlv *create_ttl_tlv(struct lldp_port *lldp_port) {

  struct lldp_tlv* tlv = initialize_tlv();
  uint16_t ttl = htons(lldp_port->tx.txTTL);

  tlv->type = TIME_TO_LIVE_TLV; // Constant defined in lldp_tlv.h
  tlv->length = 2; // Static length defined by IEEE 802.1AB section 9.5.4

  tlv->info_string = malloc(tlv->length);

  memcpy(tlv->info_string, &ttl, tlv->length);

  return tlv;
}

uint8_t validate_ttl_tlv(struct lldp_tlv *tlv)
{
  if(tlv->length != 2)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tLength should be '2'.\n", tlv->length);

      return XEINVALIDTLV;
    }

  return XVALIDTLV;
}

struct lldp_tlv *create_port_description_tlv(struct lldp_port *lldp_port) {

struct lldp_tlv* tlv = initialize_tlv();

tlv->type = PORT_DESCRIPTION_TLV; // onstant defined in lldp_tlv.h
tlv->length = strlen(lldp_port->if_name);

tlv->info_string = malloc(tlv->length);
bzero(tlv->info_string, tlv->length);

memcpy(&tlv->info_string[0], lldp_port->if_name, strlen(lldp_port->if_name));

return tlv;

}


uint8_t validate_port_description_tlv(struct lldp_tlv *tlv)
{
  // Several TLVs have this requirement.
  return validate_length_max_255(tlv);
}

struct lldp_tlv *create_system_name_tlv(struct lldp_port *lldp_port) 
{

struct lldp_tlv* tlv = initialize_tlv();

tlv->type = SYSTEM_NAME_TLV; // Constant defined in lldp_tlv.h
tlv->length = strlen(lldp_systemname);

tlv->info_string = malloc(tlv->length);

memcpy(tlv->info_string, lldp_systemname, tlv->length); 

return tlv;
}

uint8_t validate_system_name_tlv(struct lldp_tlv *tlv)
{
  // Several TLVs have this requirement.
  return validate_length_max_255(tlv);
}

struct lldp_tlv *create_system_description_tlv(struct lldp_port *lldp_port)
{

  struct lldp_tlv* tlv = initialize_tlv();

  tlv->type = SYSTEM_DESCRIPTION_TLV; // Constant defined in lldp_tlv.h

  tlv->length = strlen(lldp_systemdesc);

  tlv->info_string = malloc(tlv->length);

  memcpy(tlv->info_string, lldp_systemdesc, tlv->length);

  return tlv;

}

uint8_t validate_system_description_tlv(struct lldp_tlv *tlv)
{
 // Several TLVs have this requirement.
  return validate_length_max_255(tlv);
}

struct lldp_tlv *create_system_capabilities_tlv(struct lldp_port *lldp_port) {
  struct lldp_tlv* tlv = initialize_tlv();

  tlv->type = SYSTEM_CAPABILITIES_TLV; // Constant defined in lldp_tlv.h

  tlv->length = 4;

  tlv->info_string = malloc(tlv->length);

  
  // Tell it we're a station for now... bit 7
  uint16_t capabilities = htons(128);

  memcpy(&tlv->info_string[0], &capabilities, sizeof(uint16_t));
  memcpy(&tlv->info_string[2], &capabilities, sizeof(uint16_t));

  return tlv;
}

uint8_t validate_system_capabilities_tlv(struct lldp_tlv *tlv)
{
  if(tlv->length != 4)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tLength should be '4'.\n", tlv->length);

      return XEINVALIDTLV;
    }

  return XVALIDTLV;
}

// NB: Initial deployment will do IPv4 only... 
// 
struct lldp_tlv *create_management_address_tlv(struct lldp_port *lldp_port) {
  struct lldp_tlv *tlv = initialize_tlv();

  tlv->type = MANAGEMENT_ADDRESS_TLV; // Constant defined in lldp_tlv.h

  #define MGMT_ADDR_STR_LEN 1
  #define MGMT_ADDR_SUBTYPE 1
  #define IPV4_LEN 4
  #define IF_NUM_SUBTYPE 1
  #define IF_NUM 4
  #define OID 1
  #define OBJ_IDENTIFIER 0

  // management address string length (1 octet)
  // management address subtype (1 octet)
  // management address (4 bytes for IPv4)
  // interface numbering subtype (1 octet)
  // interface number (4 bytes)
  // OID string length (1 byte)
  // object identifier (0 to 128 octets)
  tlv->length = MGMT_ADDR_STR_LEN + MGMT_ADDR_SUBTYPE + IPV4_LEN + IF_NUM_SUBTYPE + IF_NUM + OID + OBJ_IDENTIFIER ;

  //uint64_t tlv_offset = 0;

  tlv->info_string = malloc(tlv->length);

  // Management address string length
  // subtype of 1 byte + management address length, so 5 for IPv4
  tlv->info_string[0] = 5;

  // 1 for IPv4 as per http://www.iana.org/assignments/address-family-numbers
  tlv->info_string[1] = 1;

  // Copy in our IP
  memcpy(&tlv->info_string[2], lldp_port->source_ipaddr, 4);
  
  // Interface numbering subtype... system port number in our case.
  tlv->info_string[6] = 3;
  
  //uint32_t if_index = 0;

  // Interface number... 4 bytes long, or uint32_t
  //  memcpy(&tlv->info_string[7], &if_index, sizeof(uint32_t));

  // OID - 0 for us
  tlv->info_string[11] = 0;

  // object identifier... doesn't exist for us because it's not required, and we don't have an OID.

  return tlv;
}

uint8_t validate_management_address_tlv(struct lldp_tlv *tlv)
{
  if(tlv->length < 9 || tlv->length > 167)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tIt should be between 9 and 167 inclusive!\n", tlv->length);

      return XEINVALIDTLV;
    }

  return XVALIDTLV;
}

uint8_t validate_organizationally_specific_tlv(struct lldp_tlv *tlv)
{
  if(tlv->length < 4 || tlv->length > 511)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tIt should be between 4 and 511 inclusive!\n", tlv->length);

      return XEINVALIDTLV;
    }

  return XVALIDTLV;
}

uint8_t validate_generic_tlv(struct lldp_tlv *tlv)
{
  debug_printf(DEBUG_TLV, "Generic TLV Validation for TLV type: %d.\n", tlv->type);
  debug_printf(DEBUG_TLV, "TLV Info String Length: %d\n", tlv->length);
  debug_printf(DEBUG_TLV, "TLV Info String: ");
  debug_hex_dump(DEBUG_TLV, tlv->info_string, tlv->length);

  // Length will never fall below 0 because it's an unsigned variable
  if(tlv->length > 511)
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] TLV has invalid length '%d'.\n\tIt should be between 0 and 511 inclusive!\n", tlv->length);
      
      return XEINVALIDTLV;
    }

  return XVALIDTLV;
}

// A helper function to flatten an exploded TLV.
struct lldp_flat_tlv *flatten_tlv(struct lldp_tlv *tlv) {
 
  if(!tlv) {
    return NULL;
  }

  // Initialize to 0 here... helps memory checkers like valgrind have better accuracy.
  uint16_t type_and_length       = 0;
  struct lldp_flat_tlv *flat_tlv = NULL;

  // Munge our type and length into a single container.
  type_and_length = tlv->type;

  /*debug_printf(DEBUG_TLV, "[FLATTEN] Flatting TLV Type: %d\n", tlv->type);
  debug_hex_dump(DEBUG_TLV, tlv->info_string, tlv->length);
  debug_printf(DEBUG_TLV, "Flattening TLV: \n");
  debug_printf(DEBUG_TLV, "\tTLV Type: %d\n", tlv->type);
  debug_printf(DEBUG_TLV, "\tTLV Length: %d\n", tlv->length);*/
    
  type_and_length = type_and_length << 9;

  type_and_length |= tlv->length;
  
  //debug_printf(DEBUG_TLV, "Before Network Byte Order: ");
  //debug_hex_printf(DEBUG_TLV, (uint8_t *)&type_and_length, sizeof(type_and_length));

  // Convert it to network byte order...
  type_and_length = htons(type_and_length);

  //debug_printf(DEBUG_TLV, "After Network Byte Order: ");
  //debug_hex_printf(DEBUG_TLV, (uint8_t *)&type_and_length, sizeof(type_and_length));

  // Now cram all of the bits into our flat TLV container.
  flat_tlv = malloc(sizeof(struct lldp_flat_tlv));
  
  if(flat_tlv) {
    // We malloc for the size of the entire TLV, plus the 2 bytes for type and length.
    flat_tlv->size = tlv->length + 2;

    flat_tlv->tlv = malloc(flat_tlv->size);
    bzero(&flat_tlv->tlv[0], flat_tlv->size);

    //debug_printf(DEBUG_TLV, "Flattened TLV: ");
    //debug_hex_dump(DEBUG_TLV, flat_tlv->tlv, flat_tlv->size);

    // First, copy in the type and length
    memcpy(&flat_tlv->tlv[0], &type_and_length, sizeof(type_and_length));
    
    //debug_printf(DEBUG_TLV, "Flattened TLV: ");
    //debug_hex_dump(DEBUG_TLV, flat_tlv->tlv, flat_tlv->size);

    // Then copy in the info string, for the size of tlv->length
    memcpy(&flat_tlv->tlv[sizeof(type_and_length)], tlv->info_string, tlv->length);

    #warning Do a full dump of the TLV here, so we can visually inspect it...

    // We're done. ;)

  } else {
    debug_printf(DEBUG_NORMAL, "[ERROR] Unable to malloc buffer in %s() at line: %d!\n", __FUNCTION__, __LINE__);
  }

  //debug_printf(DEBUG_TLV, "Flattened TLV: ");
  //debug_hex_dump(DEBUG_TLV, flat_tlv->tlv, flat_tlv->size);

  return flat_tlv;
}

// A helper function to explode a flattened TLV.
struct lldp_tlv *explode_tlv(struct lldp_flat_tlv *flat_tlv) {
  
  uint16_t type_and_length = 0;
  struct lldp_tlv *tlv   = NULL;

  tlv = malloc(sizeof(struct lldp_tlv));

  if(tlv) {

    // Suck the type and length out...
    type_and_length = *(uint16_t *)&tlv[0];

    tlv->length     = type_and_length & 511;
    type_and_length = type_and_length >> 9;
    tlv->type       = type_and_length;

    tlv->info_string = malloc(tlv->length);

    if(tlv->info_string) {
      // Copy the info string into our TLV...
      memcpy(&tlv->info_string[0], &flat_tlv->tlv[sizeof(type_and_length)], tlv->length);
    } else { // tlv->info_string == NULL
      debug_printf(DEBUG_NORMAL, "[ERROR] Unable to malloc buffer in %s() at line: %d!\n", __FUNCTION__, __LINE__);
    }
  } else { // tlv == NULL
    debug_printf(DEBUG_NORMAL, "[ERROR] Unable to malloc buffer in %s() at line: %d!\n", __FUNCTION__, __LINE__);    
  }

  return tlv;
}

uint8_t tlvcpy(struct lldp_tlv *dst, struct lldp_tlv *src)
{
  if(dst != NULL && src != NULL)
    {
      dst->type = src->type;
      dst->length = src->length;
      dst->info_string = malloc(dst->length);
      
      if(dst->info_string != NULL)
	{
	  memcpy(dst->info_string, src->info_string, dst->length);
	}
      else
	{
	  debug_printf(DEBUG_NORMAL, "[ERROR] Couldn't allocate memory!!\n");

	  #warning we should probably return something like XEMALLOC here... __FUNCTION__ __LINE__
	}
    }
  else
    {
      debug_printf(DEBUG_NORMAL, "[ERROR] NULL TLV!\n");
      #warning we should return an error code here... __FUNCTION__ __LINE__
    }

  return 0;
}

struct lldp_tlv *tlvpop(uint8_t *buffer)
{
  return NULL;
}

uint8_t tlvpush(uint8_t *buffer, struct lldp_tlv *tlv)
{
  return -1;
}

uint8_t lldp_cache_tlv(struct lldp_tlv *tlv)
{
  return -1;
}

/** */
void destroy_tlv_list(struct lldp_tlv_list **tlv_list) {
  struct lldp_tlv_list *current  = *tlv_list;

  if(current == NULL) {
    debug_printf(DEBUG_NORMAL, "[WARNING] Asked to delete empty list!\n");
  }

  while(current != NULL) {
    current = current->next;
    
    free((*tlv_list)->tlv->info_string);
    free((*tlv_list)->tlv);
    free(*tlv_list);
    (*tlv_list) = current;
  }
}

void add_tlv(struct lldp_tlv *tlv, struct lldp_tlv_list **tlv_list) {
  struct lldp_tlv_list *tail = malloc(sizeof(struct lldp_tlv_list));
  struct lldp_tlv_list *tmp = *tlv_list;

  tail->tlv  = tlv;
  tail->next = NULL;

  if((*tlv_list) == NULL) {
    (*tlv_list) = tail;
  } else {
    while(tmp->next != NULL) {
      tmp = tmp->next;      
    }    

    tmp->next = tail;
  }
}
