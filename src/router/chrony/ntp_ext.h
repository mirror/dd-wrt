/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019
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

  Header file for NTP extension fields
  */

#ifndef GOT_NTP_EXT_H
#define GOT_NTP_EXT_H

#include "ntp.h"

extern int NEF_SetField(unsigned char *buffer, int buffer_length, int start,
                        int type, void *body, int body_length, int *length);
extern int NEF_AddBlankField(NTP_Packet *packet, NTP_PacketInfo *info, int type,
                             int body_length, void **body);
extern int NEF_AddField(NTP_Packet *packet, NTP_PacketInfo *info,
                        int type, void *body, int body_length);
extern int NEF_ParseSingleField(unsigned char *buffer, int buffer_length, int start,
                                int *length, int *type, void **body, int *body_length);
extern int NEF_ParseField(NTP_Packet *packet, int packet_length, int start,
                          int *length, int *type, void **body, int *body_length);

#endif
