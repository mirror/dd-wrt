/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014-2016
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

  Function replacements needed when optional features are disabled.

  */

#include "config.h"

#include "clientlog.h"
#include "cmac.h"
#include "cmdmon.h"
#include "keys.h"
#include "logging.h"
#include "manual.h"
#include "memory.h"
#include "nameserv.h"
#include "nameserv_async.h"
#include "ntp_core.h"
#include "ntp_io.h"
#include "ntp_sources.h"
#include "ntp_signd.h"
#include "nts_ke_client.h"
#include "nts_ke_server.h"
#include "nts_ntp_client.h"
#include "nts_ntp_server.h"
#include "privops.h"
#include "refclock.h"
#include "sched.h"
#include "util.h"

#ifndef FEAT_CMDMON

void
CAM_Initialise(void)
{
}

void
CAM_Finalise(void)
{
}

void
CAM_OpenUnixSocket(void)
{
}

int
CAM_AddAccessRestriction(IPAddr *ip_addr, int subnet_bits, int allow, int all)
{
  return 1;
}

void
MNL_Initialise(void)
{
}

void
MNL_Finalise(void)
{
}

#endif /* !FEAT_CMDMON */

#ifndef FEAT_REFCLOCK
void
RCL_Initialise(void)
{
}

void
RCL_Finalise(void)
{
}

int
RCL_AddRefclock(RefclockParameters *params)
{
  return 0;
}

void
RCL_StartRefclocks(void)
{
}

void
RCL_ReportSource(RPT_SourceReport *report, struct timespec *now)
{
  memset(report, 0, sizeof (*report));
}

int
RCL_ModifyOffset(uint32_t ref_id, double offset)
{
  return 0;
}

#endif /* !FEAT_REFCLOCK */

#ifndef FEAT_SIGND

void
NSD_Initialise(void)
{
}

void
NSD_Finalise(void)
{
}

int
NSD_SignAndSendPacket(uint32_t key_id, NTP_Packet *packet, NTP_PacketInfo *info,
                      NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr)
{
  return 0;
}

#endif /* !FEAT_SIGND */

#ifndef HAVE_CMAC

int
CMC_GetKeyLength(CMC_Algorithm algorithm)
{
  return 0;
}

CMC_Instance
CMC_CreateInstance(CMC_Algorithm algorithm, const unsigned char *key, int length)
{
  return NULL;
}

int
CMC_Hash(CMC_Instance inst, const void *in, int in_len, unsigned char *out, int out_len)
{
  return 0;
}

void
CMC_DestroyInstance(CMC_Instance inst)
{
}

#endif /* !HAVE_CMAC */

#ifndef FEAT_NTS

void
NNS_Initialise(void)
{
}

void
NNS_Finalise(void)
{
}

int
NNS_CheckRequestAuth(NTP_Packet *packet, NTP_PacketInfo *info, uint32_t *kod)
{
  *kod = 0;
  return 0;
}

int
NNS_GenerateResponseAuth(NTP_Packet *request, NTP_PacketInfo *req_info,
                         NTP_Packet *response, NTP_PacketInfo *res_info,
                         uint32_t kod)
{
  return 0;
}

NNC_Instance
NNC_CreateInstance(IPSockAddr *nts_address, const char *name, uint32_t cert_set,
                   uint16_t ntp_port)
{
  return NULL;
}

void
NNC_DestroyInstance(NNC_Instance inst)
{
}

int
NNC_PrepareForAuth(NNC_Instance inst)
{
  return 1;
}

int
NNC_GenerateRequestAuth(NNC_Instance inst, NTP_Packet *packet, NTP_PacketInfo *info)
{
  static int logged = 0;

  LOG(logged ? LOGS_DEBUG : LOGS_WARN, "Missing NTS support");
  logged = 1;
  return 0;
}

int
NNC_CheckResponseAuth(NNC_Instance inst, NTP_Packet *packet, NTP_PacketInfo *info)
{
  return 0;
}

void
NNC_ChangeAddress(NNC_Instance inst, IPAddr *address)
{
}

void
NNC_DumpData(NNC_Instance inst)
{
}

void
NNC_GetReport(NNC_Instance inst, RPT_AuthReport *report)
{
}

void
NKS_PreInitialise(uid_t uid, gid_t gid, int scfilter_level)
{
}

void
NKS_Initialise(void)
{
}

void
NKS_Finalise(void)
{
}

void
NKS_DumpKeys(void)
{
}

void
NKS_ReloadKeys(void)
{
}

#endif /* !FEAT_NTS */
