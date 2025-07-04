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

  Header file for NTP authentication
  */

#ifndef GOT_NTP_AUTH_H
#define GOT_NTP_AUTH_H

#include "addressing.h"
#include "ntp.h"
#include "reports.h"

typedef struct NAU_Instance_Record *NAU_Instance;

/* Create an authenticator instance in a specific mode */
extern NAU_Instance NAU_CreateNoneInstance(void);
extern NAU_Instance NAU_CreateSymmetricInstance(uint32_t key_id);
extern NAU_Instance NAU_CreateNtsInstance(IPSockAddr *nts_address, const char *name,
                                          uint32_t cert_set, uint16_t ntp_port);

/* Destroy an instance */
extern void NAU_DestroyInstance(NAU_Instance instance);

/* Check if an instance is not in the None mode */
extern int NAU_IsAuthEnabled(NAU_Instance instance);

/* Get NTP version recommended for better compatibility */
extern int NAU_GetSuggestedNtpVersion(NAU_Instance instance);

/* Perform operations necessary for NAU_GenerateRequestAuth() */
extern int NAU_PrepareRequestAuth(NAU_Instance instance);

/* Extend a request with data required by the authentication mode */
extern int NAU_GenerateRequestAuth(NAU_Instance instance, NTP_Packet *request,
                                   NTP_PacketInfo *info);

/* Verify that a request is authentic.  If it is not authentic and a non-zero
   kod code is returned, a KoD response should be sent back. */
extern int NAU_CheckRequestAuth(NTP_Packet *request, NTP_PacketInfo *info, uint32_t *kod);

/* Extend a response with data required by the authentication mode.  This
   function can be called only if the previous call of NAU_CheckRequestAuth()
   was on the same request. */
extern int NAU_GenerateResponseAuth(NTP_Packet *request, NTP_PacketInfo *request_info,
                                    NTP_Packet *response, NTP_PacketInfo *response_info,
                                    NTP_Remote_Address *remote_addr,
                                    NTP_Local_Address *local_addr,
                                    uint32_t kod);

/* Verify that a response is authentic */
extern int NAU_CheckResponseAuth(NAU_Instance instance, NTP_Packet *response,
                                 NTP_PacketInfo *info);

/* Change an authentication-specific address (e.g. after replacing a source) */
extern void NAU_ChangeAddress(NAU_Instance instance, IPAddr *address);

/* Save authentication-specific data to speed up the next start */
extern void NAU_DumpData(NAU_Instance instance);

/* Provide a report about the current authentication state */
extern void NAU_GetReport(NAU_Instance instance, RPT_AuthReport *report);

#endif
