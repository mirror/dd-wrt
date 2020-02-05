/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Header file containing common NTP bits and pieces
  */

#ifndef GOT_NTP_H
#define GOT_NTP_H

#include "sysincl.h"

#include "hash.h"

typedef struct {
  uint32_t hi;
  uint32_t lo;
} NTP_int64;

typedef uint32_t NTP_int32;

/* The UDP port number used by NTP */
#define NTP_PORT 123

/* The NTP protocol version that we support */
#define NTP_VERSION 4

/* Maximum stratum number (infinity) */
#define NTP_MAX_STRATUM 16

/* The minimum valid length of an extension field */
#define NTP_MIN_EXTENSION_LENGTH 16

/* The maximum assumed length of all extension fields in received
   packets (RFC 5905 doesn't specify a limit on length or number of
   extension fields in one packet) */
#define NTP_MAX_EXTENSIONS_LENGTH 1024

/* The minimum and maximum supported length of MAC */
#define NTP_MIN_MAC_LENGTH (4 + 16)
#define NTP_MAX_MAC_LENGTH (4 + MAX_HASH_LENGTH)

/* The maximum length of MAC in NTPv4 packets which allows deterministic
   parsing of extension fields (RFC 7822) */
#define NTP_MAX_V4_MAC_LENGTH (4 + 20)

/* Type definition for leap bits */
typedef enum {
  LEAP_Normal = 0,
  LEAP_InsertSecond = 1,
  LEAP_DeleteSecond = 2,
  LEAP_Unsynchronised = 3
} NTP_Leap;

typedef enum {
  MODE_UNDEFINED = 0,
  MODE_ACTIVE = 1,
  MODE_PASSIVE = 2,
  MODE_CLIENT = 3,
  MODE_SERVER = 4,
  MODE_BROADCAST = 5
} NTP_Mode;

typedef struct {
  uint8_t lvm;
  uint8_t stratum;
  int8_t poll;
  int8_t precision;
  NTP_int32 root_delay;
  NTP_int32 root_dispersion;
  NTP_int32 reference_id;
  NTP_int64 reference_ts;
  NTP_int64 originate_ts;
  NTP_int64 receive_ts;
  NTP_int64 transmit_ts;

  /* Optional extension fields, we don't send packets with them yet */
  /* uint8_t extensions[] */

  /* Optional message authentication code (MAC) */
  NTP_int32 auth_keyid;
  uint8_t auth_data[NTP_MAX_MAC_LENGTH - 4];
} NTP_Packet;

#define NTP_NORMAL_PACKET_LENGTH (int)offsetof(NTP_Packet, auth_keyid)

/* The buffer used to hold a datagram read from the network */
typedef struct {
  NTP_Packet ntp_pkt;
  uint8_t extensions[NTP_MAX_EXTENSIONS_LENGTH];
} NTP_Receive_Buffer;

/* Macros to work with the lvm field */
#define NTP_LVM_TO_LEAP(lvm) (((lvm) >> 6) & 0x3)
#define NTP_LVM_TO_VERSION(lvm) (((lvm) >> 3) & 0x7)
#define NTP_LVM_TO_MODE(lvm) ((lvm) & 0x7)
#define NTP_LVM(leap, version, mode) \
  ((((leap) << 6) & 0xc0) | (((version) << 3) & 0x38) | ((mode) & 0x07))

/* Special NTP reference IDs */
#define NTP_REFID_UNSYNC 0x0UL
#define NTP_REFID_LOCAL 0x7F7F0101UL /* 127.127.1.1 */
#define NTP_REFID_SMOOTH 0x7F7F01FFUL /* 127.127.1.255 */

/* Structure used to save NTP measurements.  time is the local time at which
   the sample is to be considered to have been made and offset is the offset at
   the time (positive indicates that the local clock is slow relative to the
   source).  root_delay/root_dispersion include peer_delay/peer_dispersion. */
typedef struct {
  struct timespec time;
  double offset;
  double peer_delay;
  double peer_dispersion;
  double root_delay;
  double root_dispersion;
  int stratum;
  NTP_Leap leap;
} NTP_Sample;

#endif /* GOT_NTP_H */
