/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019-2020
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

  NTP authentication
  */

#include "config.h"

#include "sysincl.h"

#include "keys.h"
#include "logging.h"
#include "memory.h"
#include "ntp_auth.h"
#include "ntp_signd.h"
#include "nts_ntp.h"
#include "nts_ntp_client.h"
#include "nts_ntp_server.h"
#include "srcparams.h"
#include "util.h"

/* Structure to hold authentication configuration and state */

struct NAU_Instance_Record {
  NTP_AuthMode mode;            /* Authentication mode of NTP packets */
  uint32_t key_id;              /* Identifier of a symmetric key */
  NNC_Instance nts;             /* Client NTS state */
};

/* ================================================== */

static int
generate_symmetric_auth(uint32_t key_id, NTP_Packet *packet, NTP_PacketInfo *info)
{
  int auth_len, max_auth_len;

  if (info->length + NTP_MIN_MAC_LENGTH > sizeof (*packet)) {
    DEBUG_LOG("Packet too long");
    return 0;
  }

  /* Truncate long MACs in NTPv4 packets to allow deterministic parsing
     of extension fields (RFC 7822) */
  max_auth_len = (info->version == 4 ? NTP_MAX_V4_MAC_LENGTH : NTP_MAX_MAC_LENGTH) - 4;
  max_auth_len = MIN(max_auth_len, sizeof (*packet) - info->length - 4);

  auth_len = KEY_GenerateAuth(key_id, packet, info->length,
                              (unsigned char *)packet + info->length + 4, max_auth_len);
  if (auth_len < NTP_MIN_MAC_LENGTH - 4) {
    DEBUG_LOG("Could not generate auth data with key %"PRIu32, key_id);
    return 0;
  }

  *(uint32_t *)((unsigned char *)packet + info->length) = htonl(key_id);

  info->auth.mac.start = info->length;
  info->auth.mac.length = 4 + auth_len;
  info->auth.mac.key_id = key_id;
  info->length += info->auth.mac.length;

  return 1;
}

/* ================================================== */

static int
check_symmetric_auth(NTP_Packet *packet, NTP_PacketInfo *info)
{
  int trunc_len;

  if (info->auth.mac.length < NTP_MIN_MAC_LENGTH)
    return 0;

  trunc_len = info->version == 4 && info->auth.mac.length <= NTP_MAX_V4_MAC_LENGTH ?
              NTP_MAX_V4_MAC_LENGTH : NTP_MAX_MAC_LENGTH;

  if (!KEY_CheckAuth(info->auth.mac.key_id, packet, info->auth.mac.start,
                     (unsigned char *)packet + info->auth.mac.start + 4,
                     info->auth.mac.length - 4, trunc_len - 4))
    return 0;

  return 1;
}

/* ================================================== */

static NAU_Instance
create_instance(NTP_AuthMode mode)
{
  NAU_Instance instance;

  instance = MallocNew(struct NAU_Instance_Record);
  instance->mode = mode;
  instance->key_id = INACTIVE_AUTHKEY;
  instance->nts = NULL;

  assert(sizeof (instance->key_id) == 4);

  return instance;
}

/* ================================================== */

NAU_Instance
NAU_CreateNoneInstance(void)
{
  return create_instance(NTP_AUTH_NONE);
}

/* ================================================== */

NAU_Instance
NAU_CreateSymmetricInstance(uint32_t key_id)
{
  NAU_Instance instance = create_instance(NTP_AUTH_SYMMETRIC);

  instance->key_id = key_id;

  if (!KEY_KeyKnown(key_id))
    LOG(LOGS_WARN, "Key %"PRIu32" is %s", key_id, "missing");
  else if (!KEY_CheckKeyLength(key_id))
    LOG(LOGS_WARN, "Key %"PRIu32" is %s", key_id, "too short");

  return instance;
}

/* ================================================== */

NAU_Instance
NAU_CreateNtsInstance(IPSockAddr *nts_address, const char *name, uint32_t cert_set,
                      uint16_t ntp_port)
{
  NAU_Instance instance = create_instance(NTP_AUTH_NTS);

  instance->nts = NNC_CreateInstance(nts_address, name, cert_set, ntp_port);

  return instance;
}

/* ================================================== */

void
NAU_DestroyInstance(NAU_Instance instance)
{
  if (instance->mode == NTP_AUTH_NTS)
    NNC_DestroyInstance(instance->nts);
  Free(instance);
}

/* ================================================== */

int
NAU_IsAuthEnabled(NAU_Instance instance)
{
  return instance->mode != NTP_AUTH_NONE;
}

/* ================================================== */

int
NAU_GetSuggestedNtpVersion(NAU_Instance instance)
{
  /* If the MAC in NTPv4 packets would be truncated, prefer NTPv3 for
     compatibility with older chronyd servers */
  if (instance->mode == NTP_AUTH_SYMMETRIC &&
      KEY_GetAuthLength(instance->key_id) + sizeof (instance->key_id) > NTP_MAX_V4_MAC_LENGTH)
    return 3;

  return NTP_VERSION;
}

/* ================================================== */

int
NAU_PrepareRequestAuth(NAU_Instance instance)
{
  switch (instance->mode) {
    case NTP_AUTH_NTS:
      if (!NNC_PrepareForAuth(instance->nts))
        return 0;
      break;
    default:
      break;
  }

  return 1;
}

/* ================================================== */

int
NAU_GenerateRequestAuth(NAU_Instance instance, NTP_Packet *request, NTP_PacketInfo *info)
{
  switch (instance->mode) {
    case NTP_AUTH_NONE:
      break;
    case NTP_AUTH_SYMMETRIC:
      if (!generate_symmetric_auth(instance->key_id, request, info))
        return 0;
      break;
    case NTP_AUTH_NTS:
      if (!NNC_GenerateRequestAuth(instance->nts, request, info))
        return 0;
      break;
    default:
      assert(0);
  }

  info->auth.mode = instance->mode;

  return 1;
}

/* ================================================== */

int
NAU_CheckRequestAuth(NTP_Packet *request, NTP_PacketInfo *info, uint32_t *kod)
{
  *kod = 0;

  switch (info->auth.mode) {
    case NTP_AUTH_NONE:
      break;
    case NTP_AUTH_SYMMETRIC:
      if (!check_symmetric_auth(request, info))
        return 0;
      break;
    case NTP_AUTH_MSSNTP:
      /* MS-SNTP requests are not authenticated */
      break;
    case NTP_AUTH_MSSNTP_EXT:
      /* Not supported yet */
      return 0;
    case NTP_AUTH_NTS:
      if (!NNS_CheckRequestAuth(request, info, kod))
        return 0;
      break;
    default:
      return 0;
  }

  return 1;
}

/* ================================================== */

int
NAU_GenerateResponseAuth(NTP_Packet *request, NTP_PacketInfo *request_info,
                         NTP_Packet *response, NTP_PacketInfo *response_info,
                         NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                         uint32_t kod)
{
  switch (request_info->auth.mode) {
    case NTP_AUTH_NONE:
      break;
    case NTP_AUTH_SYMMETRIC:
      if (!generate_symmetric_auth(request_info->auth.mac.key_id, response, response_info))
        return 0;
      break;
    case NTP_AUTH_MSSNTP:
      /* Sign the packet asynchronously by ntp_signd */
      if (!NSD_SignAndSendPacket(request_info->auth.mac.key_id, response, response_info,
                                 remote_addr, local_addr))
        return 0;
      /* Don't send the original packet */
      return 0;
    case NTP_AUTH_NTS:
      if (!NNS_GenerateResponseAuth(request, request_info, response, response_info, kod))
        return 0;
      break;
    default:
      DEBUG_LOG("Could not authenticate response auth_mode=%d", (int)request_info->auth.mode);
      return 0;
  }

  response_info->auth.mode = request_info->auth.mode;

  return 1;
}

/* ================================================== */

int
NAU_CheckResponseAuth(NAU_Instance instance, NTP_Packet *response, NTP_PacketInfo *info)
{
  /* The authentication must match the expected mode */
  if (info->auth.mode != instance->mode)
    return 0;

  switch (info->auth.mode) {
    case NTP_AUTH_NONE:
      break;
    case NTP_AUTH_SYMMETRIC:
      /* Check if it is authenticated with the specified key */
      if (info->auth.mac.key_id != instance->key_id)
        return 0;
      /* and that the MAC is valid */
      if (!check_symmetric_auth(response, info))
        return 0;
      break;
    case NTP_AUTH_NTS:
      if (!NNC_CheckResponseAuth(instance->nts, response, info))
        return 0;
      break;
    default:
      return 0;
  }

  return 1;
}

/* ================================================== */

void
NAU_ChangeAddress(NAU_Instance instance, IPAddr *address)
{
  switch (instance->mode) {
    case NTP_AUTH_NONE:
    case NTP_AUTH_SYMMETRIC:
      break;
    case NTP_AUTH_NTS:
      NNC_ChangeAddress(instance->nts, address);
      break;
    default:
      assert(0);
  }
}

/* ================================================== */

void
NAU_DumpData(NAU_Instance instance)
{
  switch (instance->mode) {
    case NTP_AUTH_NTS:
      NNC_DumpData(instance->nts);
      break;
    default:
      break;
  }
}

/* ================================================== */

void
NAU_GetReport(NAU_Instance instance, RPT_AuthReport *report)
{
  memset(report, 0, sizeof (*report));

  report->mode = instance->mode;
  report->last_ke_ago = -1;

  switch (instance->mode) {
    case NTP_AUTH_NONE:
      break;
    case NTP_AUTH_SYMMETRIC:
      report->key_id = instance->key_id;
      KEY_GetKeyInfo(instance->key_id, &report->key_type, &report->key_length);
      break;
    case NTP_AUTH_NTS:
      NNC_GetReport(instance->nts, report);
      break;
    default:
      assert(0);
  }
}
