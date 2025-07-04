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

  Header for NTS Authenticator and Encrypted Extension Fields
  extension field
  */

#ifndef GOT_NTS_NTP_AUTH_H
#define GOT_NTS_NTP_AUTH_H

#include "ntp.h"
#include "siv.h"

extern int NNA_GenerateAuthEF(NTP_Packet *packet, NTP_PacketInfo *info, SIV_Instance siv,
                              const unsigned char *nonce, int max_nonce_length,
                              const unsigned char *plaintext, int plaintext_length,
                              int min_ef_length);

extern int NNA_DecryptAuthEF(NTP_Packet *packet, NTP_PacketInfo *info, SIV_Instance siv,
                             int ef_start, unsigned char *plaintext, int buffer_length,
                             int *plaintext_length);

#endif
