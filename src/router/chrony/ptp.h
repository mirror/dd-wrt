/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2021
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

  This is the header file for the Precision Time Protocol (PTP).

  */
#ifndef GOT_PTP_H
#define GOT_PTP_H

#include "sysincl.h"

#include "ntp.h"

#define PTP_VERSION_2 2
#define PTP_VERSION_2_1 (2 | 1 << 4)
#define PTP_TYPE_SYNC 0
#define PTP_TYPE_DELAY_REQ 1
#define PTP_FLAG_UNICAST (1 << (2 + 8))
#define PTP_TLV_NTP 0x2023

typedef struct {
  uint8_t type;
  uint8_t version;
  uint16_t length;
  uint8_t domain;
  uint8_t min_sdoid;
  uint16_t flags;
  uint8_t correction[8];
  uint8_t msg_specific[4];
  uint8_t port_id[10];
  uint16_t sequence_id;
  uint8_t control;
  int8_t interval;
} PTP_Header;

typedef struct {
  uint16_t type;
  uint16_t length;
} PTP_TlvHeader;

typedef struct {
  PTP_Header header;
  uint8_t origin_ts[10];
  PTP_TlvHeader tlv_header;
  NTP_Packet ntp_msg;
} PTP_NtpMessage;

#define PTP_NTP_PREFIX_LENGTH (int)offsetof(PTP_NtpMessage, ntp_msg)

#endif
