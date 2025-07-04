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

  Header file for server NTS-NTP authentication
  */

#ifndef GOT_NTS_NTP_SERVER_H
#define GOT_NTS_NTP_SERVER_H

#include "ntp.h"

extern void NNS_Initialise(void);
extern void NNS_Finalise(void);

extern int NNS_CheckRequestAuth(NTP_Packet *packet, NTP_PacketInfo *info, uint32_t *kod);
extern int NNS_GenerateResponseAuth(NTP_Packet *request, NTP_PacketInfo *req_info,
                                    NTP_Packet *response, NTP_PacketInfo *res_info,
                                    uint32_t kod);

#endif
