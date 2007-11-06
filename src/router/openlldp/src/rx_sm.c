/** @file rx_sm.c
    OpenLLDP RX Statemachine

    Licensed under a dual GPL/Proprietary license.  
    See LICENSE file for more info.
 
    Authors: Terry Simons (terry.simons@gmail.com)
*/

#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rx_sm.h"
#include "lldp_port.h"
#include "tlv/tlv.h"
#include "tlv/tlv_common.h"
#include "msap.h"
#include "lldp_debug.h"

#define FALSE 0
#define TRUE 1

/* This is an 802.1AB per-port variable, 
   so it should go in the port structure */
uint8_t badFrame;

/* Defined by the IEEE 802.1AB standard */
uint8_t rxInitializeLLDP(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB section 10.5.5.3 */
  lldp_port->rx.rcvFrame        = 0;
  
  /* As per IEEE 802.1AB section 10.1.2 */
  lldp_port->rx.tooManyNeighbors = 0;
  
  lldp_port->rx.rxInfoAge = 0;

  mibDeleteObjects(lldp_port);
  
  return 0;
}

/* Defined by the IEEE 802.1AB standard */
int rxProcessFrame(struct lldp_port *lldp_port) {
  /* 802.1AB Variables */
  uint8_t badFrame = 0;
  /* End 802.1AB Variables */

  /* Keep track of the last TLV so we can adhere to the specification */
  /* Which requires the first 3 TLVs to be in the correct order       */
  uint16_t last_tlv_type = 0;
  uint16_t num_tlvs      = 0;
  uint8_t tlv_type       = 0;
  uint16_t tlv_length    = 0;
  uint16_t tlv_offset    = 0;

  struct eth_hdr *ether_hdr;
  struct eth_hdr expect_hdr; 

  /* The TLV list for this frame */
  /* This list will be added to the MSAP cache */
  struct lldp_tlv_list *tlv_list = NULL;
 
  debug_printf(DEBUG_INT, "(%s) Processing Frame: \n", lldp_port->if_name);

  debug_hex_dump(DEBUG_INT, lldp_port->rx.frame, lldp_port->rx.recvsize);

  /* As per section 10.3.1, verify the destination and ethertype */
  expect_hdr.dst[0] = 0x01;
  expect_hdr.dst[1] = 0x80;
  expect_hdr.dst[2] = 0xc2;
  expect_hdr.dst[3] = 0x00;
  expect_hdr.dst[4] = 0x00;
  expect_hdr.dst[5] = 0x0e;
  expect_hdr.ethertype = htons(0x88cc);

  ether_hdr = (struct eth_hdr *)&lldp_port->rx.frame[0];
 
  debug_printf(DEBUG_INT, "LLPDU Dst: ");
  debug_hex_printf(DEBUG_INT, (uint8_t *)ether_hdr->dst, 6);

  debug_printf(DEBUG_EXCESSIVE, "Expect Dst: ");
  debug_hex_printf(DEBUG_EXCESSIVE, (uint8_t *)expect_hdr.dst, 6);

  /* Validate the frame's destination */
  if(
     ether_hdr->dst[0] != expect_hdr.dst[0] ||
     ether_hdr->dst[1] != expect_hdr.dst[1] ||
     ether_hdr->dst[2] != expect_hdr.dst[2] ||
     ether_hdr->dst[3] != expect_hdr.dst[3] ||
     ether_hdr->dst[4] != expect_hdr.dst[4] ||
     ether_hdr->dst[5] != expect_hdr.dst[5] ) {

    debug_printf(DEBUG_NORMAL, "[ERROR] This frame is incorrectly addressed to: ");
    debug_hex_printf(DEBUG_NORMAL, (uint8_t *)ether_hdr->dst, 6);
    debug_printf(DEBUG_NORMAL, "[ERROR] This frame should be addressed to: ");
    debug_hex_printf(DEBUG_NORMAL, (uint8_t *)expect_hdr.dst, 6);
    debug_printf(DEBUG_NORMAL, "[ERROR] statsFramesInTotal will *NOT* be incremented\n");
      
    badFrame++;
  }

  debug_printf(DEBUG_INT, "LLPDU Src: ");
  debug_hex_printf(DEBUG_INT, (uint8_t *)ether_hdr->src, 6);

  debug_printf(DEBUG_INT, "LLPDU Ethertype: %x\n", htons(ether_hdr->ethertype));

  debug_printf(DEBUG_EXCESSIVE, "Expect Ethertype: %x\n", htons(expect_hdr.ethertype));

  /* Validate the frame's ethertype */
  if(ether_hdr->ethertype != expect_hdr.ethertype) {
    debug_printf(DEBUG_NORMAL, "[ERROR] This frame has an incorrect ethertype of: '%x'.\n", htons(ether_hdr->ethertype));

    badFrame++;
  }
  
  if(!badFrame) {
    lldp_port->rx.statistics.statsFramesInTotal ++;
  }
  
  do {
    num_tlvs++;

    debug_printf(DEBUG_TLV, "Processing TLV #: %d\n", num_tlvs);

    if(tlv_offset > lldp_port->rx.recvsize) {
      debug_printf(DEBUG_NORMAL, "[ERROR] Offset is larger than received frame!");

      badFrame++;

      break;
    }
      
    uint16_t *tlv_hdr = (uint16_t *)&lldp_port->rx.frame[sizeof(struct eth_hdr) + tlv_offset];
      
    /* Grab the first 9 bits */
    tlv_length = htons(*tlv_hdr) & 0x01FF;
      
    /* Then shift to get the last 7 bits */
    tlv_type = htons(*tlv_hdr) >> 9;

    /* Validate as per 802.1AB section 10.3.2*/
    if(num_tlvs <= 3) {
      if(num_tlvs != tlv_type) {
	debug_printf(DEBUG_NORMAL, "[ERROR] TLV number %d should have tlv_type %d, but is actually %d\n", num_tlvs, num_tlvs, tlv_type);
	debug_printf(DEBUG_NORMAL, "[ERROR] statsFramesDiscardedTotal and statsFramesInErrorsTotal will be incremented as per 802.1AB 10.3.2\n");
	lldp_port->rx.statistics.statsFramesDiscardedTotal++;
	lldp_port->rx.statistics.statsFramesInErrorsTotal++;
	badFrame++;
      }
    }
 
    debug_printf(DEBUG_EXCESSIVE, "TLV type: %d (%s)  Length: %d\n", tlv_type, tlv_typetoname(tlv_type), tlv_length);
     
    /* Create a compound offset */
    uint16_t debug_offset = tlv_length + sizeof(struct eth_hdr) + tlv_offset + sizeof(*tlv_hdr);
      
    /* The TLV is telling us to index off the end of the frame... tisk tisk */
    if(debug_offset > lldp_port->rx.recvsize) {
      debug_printf(DEBUG_NORMAL, "[ERROR] Received a bad TLV:  %d bytes too long!  Frame will be skipped.\n", debug_offset - lldp_port->rx.recvsize);
	  
      badFrame++;
      
      break;
    } else {
      /* Temporary Debug to validate above... */
      debug_printf(DEBUG_EXCESSIVE, "TLV would read to: %d, Frame ends at: %d\n", debug_offset, lldp_port->rx.recvsize);
    }
    
    uint8_t *tlv_info_string = (uint8_t *)&lldp_port->rx.frame[sizeof(struct eth_hdr) + sizeof(*tlv_hdr) + tlv_offset];

    if(tlv_type == TIME_TO_LIVE_TLV) {
      if(tlv_length != 2) {
	debug_printf(DEBUG_NORMAL, "[ERROR] TTL TLV has an invalid length!  Should be '2', but is '%d'\n", tlv_length);
#warning We should actually discard this frame and print out an error...
#warning Write a unit test to stress this
      } else {
	lldp_port->rx.timers.rxTTL = htons(*(uint16_t *)&tlv_info_string[0]);
	debug_printf(DEBUG_EXCESSIVE, "rxTTL is: %d\n", lldp_port->rx.timers.rxTTL);
      }
    }
      
    struct lldp_tlv *tlv = initialize_tlv();
    
    if(!tlv) {
      debug_printf(DEBUG_NORMAL, "[ERROR] Unable to malloc buffer in %s() at line: %d!\n", __FUNCTION__, __LINE__);
      break;
    }

    tlv->type        = tlv_type;
    tlv->length      = tlv_length;
    tlv->info_string = malloc(tlv_length);
    
    if(tlv->info_string) {
      bzero(tlv->info_string, tlv_length);
      memcpy(tlv->info_string, tlv_info_string, tlv_length);
    } else {
      debug_printf(DEBUG_NORMAL, "[ERROR] Unable to malloc buffer in %s() at line: %d!\n", __FUNCTION__, __LINE__);

      destroy_tlv(&tlv);
	  
      break;
    }

    /* Validate the TLV */
    if(validate_tlv[tlv_type] != NULL) {
      debug_printf(DEBUG_EXCESSIVE, "Found a specific validator for TLV type %d.\n", tlv_type);
	  
      debug_hex_dump(DEBUG_EXCESSIVE, tlv->info_string, tlv->length);
	  
      if(validate_tlv[tlv_type](tlv) != XVALIDTLV) {
	badFrame++;
      }
    } else {
      debug_printf(DEBUG_EXCESSIVE, "Didn't find specific validator for TLV type %d.  Using validate_generic_tlv.\n", tlv_type);
      if(validate_generic_tlv(tlv) != XVALIDTLV) {
	badFrame++;
      }
    }
    
    tlv_offset += sizeof(*tlv_hdr) + tlv_length;

    last_tlv_type = tlv_type;

    decode_tlv_subtype(tlv);
    
    destroy_tlv(&tlv);
      
  }while(tlv_type != 0);

  lldp_port->rxChanges = TRUE;

  /* Report frame errors */
  if(badFrame) {
    rxBadFrameInfo(badFrame);
  }
  
  return badFrame;
}

void rxBadFrameInfo(uint8_t frameErrors) {
  debug_printf(DEBUG_NORMAL, "[WARNING] This frame had %d errors!\n", frameErrors);
}

/* Just a stub */
uint8_t mibUpdateObjects(struct lldp_port *lldp_port) {
  return 0;
}

/* Just a stub */
uint8_t mibDeleteObjects(struct lldp_port *lldp_port) {
  return 0;
}

void rxChangeToState(struct lldp_port *lldp_port, uint8_t state) {
  debug_printf(DEBUG_STATE, "[%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));

  switch(state) {
  case LLDP_WAIT_PORT_OPERATIONAL: {
    // Do nothing
  }break;    
  case RX_LLDP_INITIALIZE: {
    if(lldp_port->rx.state != LLDP_WAIT_PORT_OPERATIONAL) {
      debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    }
    
    /*
    // From the 10.5.5.3 rx state machine diagram
    rxInitializeLLDP(lldp_port);
    lldp_port->rx.rcvFrame = 0;
    */
  }break;
  case DELETE_AGED_INFO: {
    if(lldp_port->rx.state != LLDP_WAIT_PORT_OPERATIONAL) {
      debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    }

    /*
    // From the 10.5.5.3 rx state machine diagram
    lldp_port->rx.somethingChangedRemote = 0;
    mibDeleteObjects(lldp_port);
    lldp_port->rx.rxInfoAge = 0;
    lldp_port->rx.somethingChangedRemote = 1;
    */
  }break;
  case RX_WAIT_FOR_FRAME: {    
    if(!(lldp_port->rx.state == RX_LLDP_INITIALIZE ||
	 lldp_port->rx.state == DELETE_INFO ||
	 lldp_port->rx.state == UPDATE_INFO ||
	 lldp_port->rx.state == RX_FRAME)) {
      debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    }

    /*
    // From the 10.5.5.3 rx state machine diagram
    lldp_port->rx.badFrame               = 0;
    lldp_port->rx.rxInfoAge              = 0;
    lldp_port->rx.somethingChangedRemote = 0;
    */
  }break;
  case RX_FRAME: {
    if(lldp_port->rx.state != RX_WAIT_FOR_FRAME) {
      debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    }

    /*
    // From the 10.5.5.3 rx state machine diagram
    lldp_port->rx.rxChanges = 0;
    lldp_port->rcvFrame     = 0;
    rxProcessFrame(lldp_port);    
    */
  }break;
  case DELETE_INFO: {
    if(!(lldp_port->rx.state == RX_WAIT_FOR_FRAME ||
	 lldp_port->rx.state == RX_FRAME)) {
      debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    }
  }break;
  case UPDATE_INFO: {
    if(lldp_port->rx.state != RX_FRAME) {
      debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    }
  }break;
  default: {
    debug_printf(DEBUG_STATE, "[ERROR] Illegal Transition: [%s] %s -> %s\n", lldp_port->if_name, rxStateFromID(lldp_port->rx.state), rxStateFromID(state));      
    // Do nothing
  };
  };
    
    // Now update the interface state
    lldp_port->rx.state = state;
}

char *rxStateFromID(uint8_t state)
{
  switch(state)
    {
    case LLDP_WAIT_PORT_OPERATIONAL:
      return "LLDP_WAIT_PORT_OPERATIONAL";
    case DELETE_AGED_INFO:
      return "DELETE_AGED_INFO";
    case RX_LLDP_INITIALIZE:
      return "RX_LLDP_INITIALIZE";
    case RX_WAIT_FOR_FRAME:
      return "RX_WAIT_FOR_FRAME";
    case RX_FRAME:
      return "RX_FRAME";
    case DELETE_INFO:
      return "DELETE_INFO";
    case UPDATE_INFO:
      return "UPDATE_INFO";
    };

  debug_printf(DEBUG_NORMAL, "[ERROR] Unknown RX State: '%d'\n", state);
  return "Unknown";
}

uint8_t rxGlobalStatemachineRun(struct lldp_port *lldp_port)
{
  /* NB: IEEE 802.1AB Section 10.5.5.3 claims that */
  /* An unconditional transfer should occur when */
  /* "(rxInfoAge = FALSE) && (portEnabled == FALSE)" */
  /* I believe that "(rxInfoAge = FALSE)" is a typo and should be: */
  /* "(rxInfoAge == FALSE)" */
  if((lldp_port->rx.rxInfoAge == FALSE) && (lldp_port->portEnabled == FALSE))
    {
      rxChangeToState(lldp_port, LLDP_WAIT_PORT_OPERATIONAL);
    }
  
  switch(lldp_port->rx.state)
    {
    case LLDP_WAIT_PORT_OPERATIONAL:
      {
	if(lldp_port->rx.rxInfoAge == TRUE)
	  rxChangeToState(lldp_port, DELETE_AGED_INFO);
	if(lldp_port->portEnabled == TRUE) 
	  rxChangeToState(lldp_port, RX_LLDP_INITIALIZE);
      }break;
    case DELETE_AGED_INFO:
      {
	rxChangeToState(lldp_port, LLDP_WAIT_PORT_OPERATIONAL);
      }break;
    case RX_LLDP_INITIALIZE:
      {
	if((lldp_port->adminStatus == enabledRxTx) || (lldp_port->adminStatus == enabledRxOnly))
	  rxChangeToState(lldp_port, RX_WAIT_FOR_FRAME);
      }break;
    case RX_WAIT_FOR_FRAME:
      {
	if(lldp_port->rx.rxInfoAge == TRUE)
	  rxChangeToState(lldp_port, DELETE_INFO);
	if(lldp_port->rx.rcvFrame == TRUE)
	  rxChangeToState(lldp_port, RX_FRAME);
      }break;
    case DELETE_INFO:
      {
	rxChangeToState(lldp_port, RX_WAIT_FOR_FRAME);
      }break;
    case RX_FRAME:
      {
	if(lldp_port->rx.timers.rxTTL == 0)
	  rxChangeToState(lldp_port, DELETE_INFO);
	if((lldp_port->rx.timers.rxTTL != 0) && (lldp_port->rxChanges == TRUE))
	  {
	    rxChangeToState(lldp_port, UPDATE_INFO);
	  }
      }break;
    case UPDATE_INFO:
      {
	rxChangeToState(lldp_port, RX_WAIT_FOR_FRAME);
      }break;
    default:
      debug_printf(DEBUG_NORMAL, "[ERROR] The RX Global State Machine is broken!\n");
    };

  return 0;
}

void rxStatemachineRun(struct lldp_port *lldp_port)
{
  rxGlobalStatemachineRun(lldp_port);
  
  switch(lldp_port->rx.state)
    {
    case LLDP_WAIT_PORT_OPERATIONAL:
      {
	// Do nothing here... we'll transition in the global state machine check
	rx_do_lldp_wait_port_operational(lldp_port);
      }break;
    case DELETE_AGED_INFO:
      {
	rx_do_delete_aged_info(lldp_port);
      }break;
    case RX_LLDP_INITIALIZE:
      {
	rx_do_rx_lldp_initialize(lldp_port);
      }break;
    case RX_WAIT_FOR_FRAME:
      {
	rx_do_rx_wait_for_frame(lldp_port);
      }break;
    case RX_FRAME:
      {
	rx_do_rx_frame(lldp_port);
      }break;
    case DELETE_INFO: {
	rx_do_rx_delete_info(lldp_port);
      }break;
    case UPDATE_INFO: {
	rx_do_rx_update_info(lldp_port);
      }break;
    default:
      debug_printf(DEBUG_NORMAL, "[ERROR] The RX State Machine is broken!\n");      
    };

  rx_do_update_timers(lldp_port);
}

void rx_decrement_timer(uint16_t **timer) {
  if(*timer > 0) {
    *timer--;
  }
}

void rx_do_update_timers(struct lldp_port *lldp_port) {
  // Here's where we update the IEEE 802.1AB RX timers:

  //rx_decrement_timer(&lldp_port->rx.timers.rxInfoTTL);
  //rx_decrement_timer(&lldp_port->rx.timers.TooManyNeighborsTimer);
}

void rx_do_lldp_wait_port_operational(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
}


void rx_do_delete_aged_info(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
  lldp_port->rx.somethingChangedRemote = FALSE;
  mibDeleteObjects(lldp_port);
  lldp_port->rx.rxInfoAge = FALSE;
  lldp_port->rx.somethingChangedRemote = TRUE;
}

void rx_do_rx_lldp_initialize(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
  rxInitializeLLDP(lldp_port);
  lldp_port->rx.rcvFrame = FALSE;
}

void rx_do_rx_wait_for_frame(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
  lldp_port->rx.badFrame = FALSE;
  lldp_port->rx.rxInfoAge = FALSE;
  lldp_port->rx.somethingChangedRemote = FALSE;
}

void rx_do_rx_frame(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
  lldp_port->rxChanges = FALSE;
  lldp_port->rx.rcvFrame = FALSE;
  rxProcessFrame(lldp_port);

  // Clear the frame buffer out to avoid weird problems. ;)
  bzero(&lldp_port->rx.frame[0], lldp_port->mtu);
}

void rx_do_rx_delete_info(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
  mibDeleteObjects(lldp_port);
  lldp_port->rx.somethingChangedRemote = TRUE;
}

void rx_do_rx_update_info(struct lldp_port *lldp_port) {
  /* As per IEEE 802.1AB 10.5.5.3 state diagram */
  mibUpdateObjects(lldp_port);
  lldp_port->rx.somethingChangedRemote = TRUE;
}
