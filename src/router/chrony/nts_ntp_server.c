/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020, 2022
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

  Server NTS-NTP authentication
  */

#include "config.h"

#include "sysincl.h"

#include "nts_ntp_server.h"

#include "conf.h"
#include "logging.h"
#include "memory.h"
#include "ntp.h"
#include "ntp_ext.h"
#include "nts_ke_server.h"
#include "nts_ntp.h"
#include "nts_ntp_auth.h"
#include "siv.h"
#include "util.h"

#define MAX_SERVER_SIVS 2

struct NtsServer {
  SIV_Instance sivs[MAX_SERVER_SIVS];
  SIV_Algorithm siv_algorithms[MAX_SERVER_SIVS];
  unsigned char nonce[NTS_MIN_UNPADDED_NONCE_LENGTH];
  NKE_Cookie cookies[NTS_MAX_COOKIES];
  int num_cookies;
  int siv_index;
  NTP_int64 req_tx;
};

/* The server instance handling all requests */
struct NtsServer *server;

/* ================================================== */

void
NNS_Initialise(void)
{
  const char **certs, **keys;
  int i;

  /* Create an NTS-NTP server instance only if NTS-KE server is enabled */
  if (CNF_GetNtsServerCertAndKeyFiles(&certs, &keys) <= 0) {
    server = NULL;
    return;
  }

  server = Malloc(sizeof (struct NtsServer));

  server->siv_algorithms[0] = AEAD_AES_SIV_CMAC_256;
  server->siv_algorithms[1] = AEAD_AES_128_GCM_SIV;
  assert(MAX_SERVER_SIVS == 2);

  for (i = 0; i < 2; i++)
    server->sivs[i] = SIV_CreateInstance(server->siv_algorithms[i]);

  /* AES-SIV-CMAC-256 is required on servers */
  if (!server->sivs[0])
    LOG_FATAL("Missing AES-SIV-CMAC-256");
}

/* ================================================== */

void
NNS_Finalise(void)
{
  int i;

  if (!server)
    return;

  for (i = 0; i < MAX_SERVER_SIVS; i++) {
    if (server->sivs[i])
      SIV_DestroyInstance(server->sivs[i]);
  }
  Free(server);
  server = NULL;
}

/* ================================================== */

int
NNS_CheckRequestAuth(NTP_Packet *packet, NTP_PacketInfo *info, uint32_t *kod)
{
  int ef_type, ef_body_length, ef_length, has_uniq_id = 0, has_auth = 0, has_cookie = 0;
  int i, plaintext_length, parsed, requested_cookies, cookie_length = -1, auth_start = 0;
  unsigned char plaintext[NTP_MAX_EXTENSIONS_LENGTH];
  NKE_Context context;
  NKE_Cookie cookie;
  SIV_Instance siv;
  void *ef_body;

  *kod = 0;

  if (!server)
    return 0;

  server->num_cookies = 0;
  server->siv_index = -1;
  server->req_tx = packet->transmit_ts;

  if (info->ext_fields == 0 || info->mode != MODE_CLIENT)
    return 0;

  requested_cookies = 0;

  for (parsed = NTP_HEADER_LENGTH; parsed < info->length; parsed += ef_length) {
    if (!NEF_ParseField(packet, info->length, parsed,
                        &ef_length, &ef_type, &ef_body, &ef_body_length))
      /* This is not expected as the packet already passed NAU_ParsePacket() */
      return 0;

    switch (ef_type) {
      case NTP_EF_NTS_UNIQUE_IDENTIFIER:
        has_uniq_id = 1;
        break;
      case NTP_EF_NTS_COOKIE:
        if (has_cookie || ef_body_length > sizeof (cookie.cookie)) {
          DEBUG_LOG("Unexpected cookie/length");
          return 0;
        }
        cookie.length = ef_body_length;
        memcpy(cookie.cookie, ef_body, ef_body_length);
        has_cookie = 1;
        /* Fall through */
      case NTP_EF_NTS_COOKIE_PLACEHOLDER:
        requested_cookies++;

        if (cookie_length >= 0 && cookie_length != ef_body_length) {
          DEBUG_LOG("Invalid cookie/placeholder length");
          return 0;
        }
        cookie_length = ef_body_length;
        break;
      case NTP_EF_NTS_AUTH_AND_EEF:
        if (parsed + ef_length != info->length) {
          DEBUG_LOG("Auth not last EF");
          return 0;
        }

        auth_start = parsed;
        has_auth = 1;
        break;
      default:
        break;
    }
  }

  if (!has_uniq_id || !has_cookie || !has_auth) {
    DEBUG_LOG("Missing an NTS EF");
    return 0;
  }

  if (!NKS_DecodeCookie(&cookie, &context)) {
    *kod = NTP_KOD_NTS_NAK;
    return 0;
  }

  /* Find the SIV instance needed for authentication */
  for (i = 0; i < MAX_SERVER_SIVS && context.algorithm != server->siv_algorithms[i]; i++)
    ;
  if (i == MAX_SERVER_SIVS || !server->sivs[i]) {
    DEBUG_LOG("Unexpected SIV");
    return 0;
  }
  server->siv_index = i;
  siv = server->sivs[i];

  if (!SIV_SetKey(siv, context.c2s.key, context.c2s.length)) {
    DEBUG_LOG("Could not set C2S key");
    return 0;
  }

  if (!NNA_DecryptAuthEF(packet, info, siv, auth_start,
                         plaintext, sizeof (plaintext), &plaintext_length)) {
    *kod = NTP_KOD_NTS_NAK;
    return 0;
  }

  for (parsed = 0; parsed < plaintext_length; parsed += ef_length) {
    if (!NEF_ParseSingleField(plaintext, plaintext_length, parsed,
                              &ef_length, &ef_type, &ef_body, &ef_body_length)) {
      DEBUG_LOG("Could not parse encrypted EF");
      return 0;
    }

    switch (ef_type) {
      case NTP_EF_NTS_COOKIE_PLACEHOLDER:
        if (cookie_length != ef_body_length) {
          DEBUG_LOG("Invalid cookie/placeholder length");
          return 0;
        }
        requested_cookies++;
        break;
      default:
        break;
    }
  }

  if (!SIV_SetKey(siv, context.s2c.key, context.s2c.length)) {
    DEBUG_LOG("Could not set S2C key");
    return 0;
  }

  /* Prepare data for NNS_GenerateResponseAuth() to minimise the time spent
     there (when the TX timestamp is already set) */

  UTI_GetRandomBytes(server->nonce, sizeof (server->nonce));

  assert(sizeof (server->cookies) / sizeof (server->cookies[0]) == NTS_MAX_COOKIES);
  for (i = 0; i < NTS_MAX_COOKIES && i < requested_cookies; i++)
    if (!NKS_GenerateCookie(&context, &server->cookies[i]))
      return 0;

  server->num_cookies = i;

  return 1;
}

/* ================================================== */

int
NNS_GenerateResponseAuth(NTP_Packet *request, NTP_PacketInfo *req_info,
                         NTP_Packet *response, NTP_PacketInfo *res_info,
                         uint32_t kod)
{
  int i, ef_type, ef_body_length, ef_length, parsed;
  void *ef_body;
  unsigned char plaintext[NTP_MAX_EXTENSIONS_LENGTH];
  int plaintext_length;

  if (!server || req_info->mode != MODE_CLIENT || res_info->mode != MODE_SERVER)
    return 0;

  /* Make sure this is a response to the request from the last call
     of NNS_CheckRequestAuth() */
  BRIEF_ASSERT(UTI_CompareNtp64(&server->req_tx, &request->transmit_ts) == 0);

  for (parsed = NTP_HEADER_LENGTH; parsed < req_info->length; parsed += ef_length) {
    if (!NEF_ParseField(request, req_info->length, parsed,
                        &ef_length, &ef_type, &ef_body, &ef_body_length))
      /* This is not expected as the packet already passed parsing */
      return 0;

    switch (ef_type) {
      case NTP_EF_NTS_UNIQUE_IDENTIFIER:
        /* Copy the ID from the request */
        if (!NEF_AddField(response, res_info, ef_type, ef_body, ef_body_length))
          return 0;
      default:
        break;
    }
  }

  /* NTS NAK response does not have any other fields */
  if (kod == NTP_KOD_NTS_NAK)
    return 1;

  for (i = 0, plaintext_length = 0; i < server->num_cookies; i++) {
    if (!NEF_SetField(plaintext, sizeof (plaintext), plaintext_length,
                      NTP_EF_NTS_COOKIE, server->cookies[i].cookie,
                      server->cookies[i].length, &ef_length))
      return 0;

    plaintext_length += ef_length;
    assert(plaintext_length <= sizeof (plaintext));
  }

  server->num_cookies = 0;

  if (server->siv_index < 0)
    return 0;

  /* Generate an authenticator field which will make the length
     of the response equal to the length of the request */
  if (!NNA_GenerateAuthEF(response, res_info, server->sivs[server->siv_index],
                          server->nonce, sizeof (server->nonce),
                          plaintext, plaintext_length,
                          req_info->length - res_info->length))
    return 0;

  return 1;
}
