/*
 * Global definitions header file
 *
 * $Id: lane.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#ifndef LANE_H
#define LANE_H

/* System includes needed for types */
#include <sys/types.h>

/* Local includes needed for types */
#include "units.h"


/* Type definitions */
typedef struct {
  unsigned le_header : 16;
  unsigned char destaddr[6];
  unsigned char srcaddr[6];
  unsigned type_len : 16;
} LaneData_t;

typedef struct {
  long port;
  long vpi;
  long vci;
} LaneVcc_t;

#define LE_HEADER_MAX 0xefff

typedef struct {
  unsigned short tag;
  union {
    unsigned char mac_address[6];
    struct route_designator {
      unsigned char reserved[4];
      unsigned short designator;
    } route;
  } a_r;
} LaneDestination_t;

#define LANE_DEST_NP 0x0000
#define LANE_DEST_MAC 0x0001
#define LANE_DEST_RD 0x0002

typedef unsigned short LecId_t;
#define LECID_MAX 0xfeff

typedef struct {
  unsigned char addr[20];
} AtmAddr_t;
#define ATM_ADDR_LEN (sizeof(AtmAddr_t))

typedef struct lanedest_l {
  LaneDestination_t *addr;
  struct lanedest_l *next;
} LaneDestList_t;

typedef struct {
  LaneVcc_t *pvc;
  AtmAddr_t *address;
  LecId_t lecid;
  LaneDestList_t *destinations;
} InitPvc_t;

typedef struct {
  unsigned marker : 16;
  unsigned protocol : 8;
  unsigned version : 8;
  unsigned opcode : 16;
  unsigned status : 16;
  unsigned transaction_id : 32;
  LecId_t lecid;
  unsigned flags : 16;
  LaneDestination_t source;
  LaneDestination_t target;
  AtmAddr_t source_addr;
  unsigned lan_type : 8;
  unsigned max_frame : 8;
  unsigned reserved : 8;
  unsigned elan_name_size : 8;
  AtmAddr_t target_addr;
  char elan_name[32];
} LaneControl_t;

typedef struct {
  unsigned marker : 16;
  unsigned protocol : 8;
  unsigned version : 8;
  unsigned opcode : 16;
} LaneVccReady_t;

/* Global function prototypes */

/* Global data */
#define LE_MARKER 0xff00
#define LE_PROTOCOL 0x01
#define LE_VERSION 0x01

/* Opcodes */
#define LE_CONFIGURE_REQUEST 0x0001
#define LE_CONFIGURE_RESPONSE 0x0101
#define LE_JOIN_REQUEST 0x0002
#define LE_JOIN_RESPONSE 0x0102
#define READY_QUERY 0x0003
#define READY_IND 0x0103
#define LE_REGISTER_REQUEST 0x0004
#define LE_REGISTER_RESPONSE 0x0104
#define LE_UNREGISTER_REQUEST 0x0005
#define LE_UNREGISTER_RESPONSE 0x0105
#define LE_ARP_REQUEST 0x0006
#define LE_ARP_RESPONSE 0x0106
#define LE_FLUSH_REQUEST 0x0007
#define LE_FLUSH_RESPONSE 0x0107
#define LE_NARP_REQUEST 0x0008
#define LE_TOPOLOGY_REQUEST 0x0009

/* Status codes */
#define LE_STATUS_SUCCESS 0		/* Success */
#define LE_STATUS_BAD_VERSION 1		/* Version not supported */
#define LE_STATUS_BAD_REQ 2		/* Invalid request parameters */
#define LE_STATUS_DUPLICATE_REG 4	/* Duplicate LAN registration */
#define LE_STATUS_DUPLICATE_ADDR 5	/* Duplicate ATM address */
#define LE_STATUS_NO_RESOURCES 6	/* Insufficient resources */
#define LE_STATUS_NO_ACCESS 7		/* Access denied */
#define LE_STATUS_BAD_LECID 8		/* Invalid requestor-id */
#define LE_STATUS_BAD_DEST 9		/* Invalid LAN destination */
#define LE_STATUS_BAD_ADDR 10		/* Invalid ATM address */
#define LE_STATUS_NO_CONFIG 20		/* No configuration */
#define LE_STATUS_CONFIG_ERROR 21	/* LE_CONFIGURE error */
#define LE_STATUS_NO_INFO 22		/* Insufficient Information */
#define LE_STATUS_MAX 22	

/* Flags */
#define LE_FLAG_REMOTE 0x0001
#define LE_FLAG_PROXY 0x0080
#define LE_FLAG_TOPOLOGY_CHANGE 0x0100

/* Lan types */
#define LE_LAN_TYPE_UNSPECIFIED 0x00
#define LE_LAN_TYPE_802_3 0x01
#define LE_LAN_TYPE_802_5 0x02

/* Max frame sizes, mtus */
#define LE_MAX_FRAME_UNSPECIFIED 0x00
#define LE_MAX_FRAME_1516 0x01
#define LE_MAX_FRAME_4544 0x02
#define LE_MAX_FRAME_9234 0x03
#define LE_MAX_FRAME_18190 0x04

extern const Unit_t main_unit;

#endif

