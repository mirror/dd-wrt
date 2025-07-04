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

/* Invalid stratum number */
#define NTP_INVALID_STRATUM 0

/* The minimum and maximum supported length of MAC */
#define NTP_MIN_MAC_LENGTH (4 + 16)
#define NTP_MAX_MAC_LENGTH (4 + MAX_HASH_LENGTH)

/* The minimum valid length of an extension field */
#define NTP_MIN_EF_LENGTH 16

/* The maximum assumed length of all extension fields in an NTP packet,
   including a MAC (RFC 5905 doesn't specify a limit on length or number of
   extension fields in one packet) */
#define NTP_MAX_EXTENSIONS_LENGTH (1024 + NTP_MAX_MAC_LENGTH)

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

  uint8_t extensions[NTP_MAX_EXTENSIONS_LENGTH];
} NTP_Packet;

#define NTP_HEADER_LENGTH (int)offsetof(NTP_Packet, extensions)

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

/* Non-authentication extension fields and corresponding internal flags */

#define NTP_EF_EXP_MONO_ROOT            0xF323
#define NTP_EF_EXP_NET_CORRECTION       0xF324

#define NTP_EF_FLAG_EXP_MONO_ROOT       0x1
#define NTP_EF_FLAG_EXP_NET_CORRECTION  0x2

/* Pre-NTPv5 experimental extension field */
typedef struct {
  uint32_t magic;
  NTP_int32 root_delay;
  NTP_int32 root_dispersion;
  NTP_int64 mono_receive_ts;
  uint32_t mono_epoch;
} NTP_EFExpMonoRoot;

#define NTP_EF_EXP_MONO_ROOT_MAGIC      0xF5BEDD9AU

/* Experimental extension field to provide PTP corrections */
typedef struct {
  uint32_t magic;
  NTP_int64 correction;
  uint32_t reserved[3];
} NTP_EFExpNetCorrection;

#define NTP_EF_EXP_NET_CORRECTION_MAGIC 0x07AC2CEBU

/* Authentication extension fields */

#define NTP_EF_NTS_UNIQUE_IDENTIFIER    0x0104
#define NTP_EF_NTS_COOKIE               0x0204
#define NTP_EF_NTS_COOKIE_PLACEHOLDER   0x0304
#define NTP_EF_NTS_AUTH_AND_EEF         0x0404

/* Enumeration for authentication modes of NTP packets */
typedef enum {
  NTP_AUTH_NONE = 0,            /* No authentication */
  NTP_AUTH_SYMMETRIC,           /* NTP MAC or CMAC using a symmetric key
                                   (RFC 1305, RFC 5905, RFC 8573) */
  NTP_AUTH_MSSNTP,              /* MS-SNTP authenticator field */
  NTP_AUTH_MSSNTP_EXT,          /* MS-SNTP extended authenticator field */
  NTP_AUTH_NTS,                 /* Network Time Security (RFC 8915) */
} NTP_AuthMode;

/* Structure describing an NTP packet */
typedef struct {
  int length;
  int version;
  NTP_Mode mode;

  int ext_fields;
  int ext_field_flags;

  struct {
    NTP_AuthMode mode;
    struct {
      int start;
      int length;
      uint32_t key_id;
    } mac;
  } auth;
} NTP_PacketInfo;

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
} NTP_Sample;

/* Possible sources of timestamps */
typedef enum {
  NTP_TS_DAEMON = 0,
  NTP_TS_KERNEL,
  NTP_TS_HARDWARE
} NTP_Timestamp_Source;

#endif /* GOT_NTP_H */
