/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020
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

  Header file for the NTS Key Establishment protocol
  */

#ifndef GOT_NTS_KE_H
#define GOT_NTS_KE_H

#include "siv.h"

#define NKE_PORT                        4460

#define NKE_RECORD_CRITICAL_BIT         (1U << 15)
#define NKE_RECORD_END_OF_MESSAGE       0
#define NKE_RECORD_NEXT_PROTOCOL        1
#define NKE_RECORD_ERROR                2
#define NKE_RECORD_WARNING              3
#define NKE_RECORD_AEAD_ALGORITHM       4
#define NKE_RECORD_COOKIE               5
#define NKE_RECORD_NTPV4_SERVER_NEGOTIATION 6
#define NKE_RECORD_NTPV4_PORT_NEGOTIATION 7
#define NKE_RECORD_COMPLIANT_128GCM_EXPORT 1024

#define NKE_NEXT_PROTOCOL_NTPV4         0

#define NKE_ERROR_UNRECOGNIZED_CRITICAL_RECORD 0
#define NKE_ERROR_BAD_REQUEST           1
#define NKE_ERROR_INTERNAL_SERVER_ERROR 2

#define NKE_ALPN_NAME                   "ntske/1"
#define NKE_EXPORTER_LABEL              "EXPORTER-network-time-security"

#define NKE_MAX_MESSAGE_LENGTH          16384
#define NKE_MAX_RECORD_BODY_LENGTH      256
#define NKE_MAX_COOKIE_LENGTH           256
#define NKE_MAX_COOKIES                 8
#define NKE_MAX_KEY_LENGTH SIV_MAX_KEY_LENGTH

#define NKE_RETRY_FACTOR2_CONNECT       4
#define NKE_RETRY_FACTOR2_TLS           10
#define NKE_MAX_RETRY_INTERVAL2         19

typedef struct {
  int length;
  unsigned char key[NKE_MAX_KEY_LENGTH];
} NKE_Key;

typedef struct {
  SIV_Algorithm algorithm;
  NKE_Key c2s;
  NKE_Key s2c;
} NKE_Context;

typedef struct {
  int length;
  unsigned char cookie[NKE_MAX_COOKIE_LENGTH];
} NKE_Cookie;

#endif
