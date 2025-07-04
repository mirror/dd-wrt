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

  Header file for client NTS-NTP authentication
  */

#ifndef GOT_NTS_NTP_CLIENT_H
#define GOT_NTS_NTP_CLIENT_H

#include "addressing.h"
#include "ntp.h"
#include "reports.h"

typedef struct NNC_Instance_Record *NNC_Instance;

extern NNC_Instance NNC_CreateInstance(IPSockAddr *nts_address, const char *name,
                                       uint32_t cert_set, uint16_t ntp_port);
extern void NNC_DestroyInstance(NNC_Instance inst);
extern int NNC_PrepareForAuth(NNC_Instance inst);
extern int NNC_GenerateRequestAuth(NNC_Instance inst, NTP_Packet *packet,
                                   NTP_PacketInfo *info);
extern int NNC_CheckResponseAuth(NNC_Instance inst, NTP_Packet *packet,
                                 NTP_PacketInfo *info);

extern void NNC_ChangeAddress(NNC_Instance inst, IPAddr *address);

extern void NNC_DumpData(NNC_Instance inst);

extern void NNC_GetReport(NNC_Instance inst, RPT_AuthReport *report);

#endif
