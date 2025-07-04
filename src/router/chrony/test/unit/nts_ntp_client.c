/*
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
 */

#include <config.h>
#include "test.h"

#ifdef FEAT_NTS

#include "socket.h"
#include "ntp.h"
#include "nts_ke_client.h"

#define NKC_CreateInstance(address, name, cert_set) Malloc(1)
#define NKC_DestroyInstance(inst) Free(inst)
#define NKC_Start(inst) (random() % 2)
#define NKC_IsActive(inst) (random() % 2)
#define NKC_GetRetryFactor(inst) (1)

static int get_nts_data(NKC_Instance inst, NKE_Context *context, NKE_Context *alt_context,
                        NKE_Cookie *cookies, int *num_cookies, int max_cookies,
                        IPSockAddr *ntp_address);
#define NKC_GetNtsData get_nts_data

#include <nts_ntp_client.c>

static int
get_nts_data(NKC_Instance inst, NKE_Context *context, NKE_Context *alt_context,
             NKE_Cookie *cookies, int *num_cookies, int max_cookies,
             IPSockAddr *ntp_address)
{
  int i;

  if (random() % 2)
    return 0;

  do {
    context->algorithm = AEAD_AES_SIV_CMAC_256 + random() %
                         (AEAD_AES_256_GCM_SIV - AEAD_AES_SIV_CMAC_256 + 10);
  } while (SIV_GetKeyLength(context->algorithm) <= 0);

  context->c2s.length = SIV_GetKeyLength(context->algorithm);
  UTI_GetRandomBytes(context->c2s.key, context->c2s.length);
  context->s2c.length = SIV_GetKeyLength(context->algorithm);
  UTI_GetRandomBytes(context->s2c.key, context->s2c.length);

  if (random() % 2) {
    *alt_context = *context;
    UTI_GetRandomBytes(alt_context->c2s.key, alt_context->c2s.length);
    UTI_GetRandomBytes(alt_context->s2c.key, alt_context->s2c.length);
  } else {
    alt_context->algorithm = AEAD_SIV_INVALID;
  }

  *num_cookies = random() % max_cookies + 1;
  for (i = 0; i < *num_cookies; i++) {
    cookies[i].length = random() % (sizeof (cookies[i].cookie) + 1);
    if (random() % 4 != 0)
      cookies[i].length = cookies[i].length / 4 * 4;
    memset(cookies[i].cookie, random(), cookies[i].length);
  }

  ntp_address->ip_addr.family = IPADDR_UNSPEC;
  ntp_address->port = 0;

  return 1;
}

static int
get_request(NNC_Instance inst)
{
  unsigned char nonce[NTS_MIN_UNPADDED_NONCE_LENGTH], uniq_id[NTS_MIN_UNIQ_ID_LENGTH];
  int nonce_length, expected_length, req_cookies;
  NTP_PacketInfo info;
  NTP_Packet packet;

  memset(&packet, 0, sizeof (packet));
  memset(&info, 0, sizeof (info));
  info.version = 4;
  info.mode = MODE_CLIENT;
  info.length = random() % (sizeof (packet) + 1);
  if (random() % 4 != 0)
    info.length = info.length / 4 * 4;

  if (inst->num_cookies > 0 && random() % 2) {
    inst->num_cookies = 0;

    TEST_CHECK(!NNC_GenerateRequestAuth(inst, &packet, &info));
  }

  while (!NNC_PrepareForAuth(inst)) {
    inst->next_nke_attempt = SCH_GetLastEventMonoTime() + random() % 10 - 7;
  }

  TEST_CHECK(inst->num_cookies > 0);
  TEST_CHECK(inst->siv);

  switch (inst->context.algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      nonce_length = 16;
      break;
    case AEAD_AES_128_GCM_SIV:
      nonce_length = 12;
      break;
    default:
      assert(0);
  }

  memcpy(nonce, inst->nonce, sizeof (nonce));
  memcpy(uniq_id, inst->uniq_id, sizeof (uniq_id));
  TEST_CHECK(NNC_PrepareForAuth(inst));
  TEST_CHECK(memcmp(nonce, inst->nonce, sizeof (nonce)) != 0);
  TEST_CHECK(memcmp(uniq_id, inst->uniq_id, sizeof (uniq_id)) != 0);

  req_cookies = MIN(NTS_MAX_COOKIES - inst->num_cookies + 1,
                    MAX_TOTAL_COOKIE_LENGTH /
                      (inst->cookies[inst->cookie_index].length + 4));
  expected_length = info.length + 4 + sizeof (inst->uniq_id) +
                    req_cookies * (4 + inst->cookies[inst->cookie_index].length) +
                    4 + 4 + nonce_length + SIV_GetTagLength(inst->siv);
  DEBUG_LOG("algo=%d length=%d cookie_length=%d expected_length=%d",
            (int)inst->context.algorithm, info.length,
            inst->cookies[inst->cookie_index].length, expected_length);

  if (info.length % 4 == 0 && info.length >= NTP_HEADER_LENGTH &&
      inst->cookies[inst->cookie_index].length % 4 == 0 &&
      inst->cookies[inst->cookie_index].length >= (NTP_MIN_EF_LENGTH - 4) &&
      expected_length <= sizeof (packet)) {
    TEST_CHECK(NNC_GenerateRequestAuth(inst, &packet, &info));
    TEST_CHECK(info.length == expected_length);
    return 1;
  } else {
    TEST_CHECK(!NNC_GenerateRequestAuth(inst, &packet, &info));
    return 0;
  }
}

static void
prepare_response(NNC_Instance inst, NTP_Packet *packet, NTP_PacketInfo *info, int valid, int nak)
{
  unsigned char cookie[508], plaintext[528], nonce[448];
  int nonce_length, ef_length, cookie_length, plaintext_length, min_auth_length;
  int i, index, auth_start;
  SIV_Instance siv;

  memset(packet, 0, sizeof (*packet));
  packet->lvm = NTP_LVM(0, 4, MODE_SERVER);
  memset(info, 0, sizeof (*info));
  info->version = 4;
  info->mode = MODE_SERVER;
  info->length = NTP_HEADER_LENGTH;

  if (valid)
    index = -1;
  else
    index = random() % (nak ? 2 : 8);

  DEBUG_LOG("index=%d nak=%d", index, nak);

  if (index != 0)
    TEST_CHECK(NEF_AddField(packet, info, NTP_EF_NTS_UNIQUE_IDENTIFIER, inst->uniq_id,
                            sizeof (inst->uniq_id)));
  if (index == 1)
    ((unsigned char *)packet)[NTP_HEADER_LENGTH + 4]++;

  if (nak) {
    packet->stratum = NTP_INVALID_STRATUM;
    packet->reference_id = htonl(NTP_KOD_NTS_NAK);
    return;
  }

  nonce_length = SIV_GetMinNonceLength(inst->siv) + random() %
                 (MIN(SIV_GetMaxNonceLength(inst->siv), sizeof (nonce)) -
                  SIV_GetMinNonceLength(inst->siv) + 1);
  assert(nonce_length >= 1 && nonce_length <= sizeof (nonce));

  do {
    cookie_length = random() % (sizeof (cookie) + 1);
  } while (cookie_length % 4 != 0 ||
           ((index != 2) == (cookie_length < NTP_MIN_EF_LENGTH - 4 ||
                             cookie_length > NKE_MAX_COOKIE_LENGTH)));

  min_auth_length = random() % (512 + 1);

  DEBUG_LOG("nonce_length=%d cookie_length=%d min_auth_length=%d",
            nonce_length, cookie_length, min_auth_length);

  UTI_GetRandomBytes(nonce, nonce_length);
  UTI_GetRandomBytes(cookie, cookie_length);

  if (cookie_length >= 12 && cookie_length <= 32 && random() % 2 == 0)
    TEST_CHECK(NEF_AddField(packet, info, NTP_EF_NTS_COOKIE, cookie, cookie_length));

  plaintext_length = 0;
  if (index != 3) {
    for (i = random() % ((sizeof (plaintext) - 16) / (cookie_length + 4)); i >= 0; i--) {
      TEST_CHECK(NEF_SetField(plaintext, sizeof (plaintext), plaintext_length,
                              NTP_EF_NTS_COOKIE, cookie,
                              i == 0 ? cookie_length : random() % (cookie_length + 1) / 4 * 4,
                              &ef_length));
      plaintext_length += ef_length;
    }
  }
  auth_start = info->length;
  if (index != 4) {
    if (index == 5) {
      assert(plaintext_length + 16 <= sizeof (plaintext));
      memset(plaintext + plaintext_length, 0, 16);
      plaintext_length += 16;
    }
    siv = SIV_CreateInstance(inst->context.algorithm);
    TEST_CHECK(siv);
    TEST_CHECK(SIV_SetKey(siv, inst->context.s2c.key, inst->context.s2c.length));
    TEST_CHECK(NNA_GenerateAuthEF(packet, info, siv,
                                  nonce, nonce_length, plaintext, plaintext_length,
                                  min_auth_length));
    SIV_DestroyInstance(siv);
  }
  if (index == 6)
    ((unsigned char *)packet)[auth_start + 8]++;
  if (index == 7)
    TEST_CHECK(NEF_AddField(packet, info, 0x7000, inst->uniq_id, sizeof (inst->uniq_id)));
}

void
test_unit(void)
{
  NNC_Instance inst;
  NTP_PacketInfo info;
  NTP_Packet packet;
  IPSockAddr addr;
  IPAddr ip_addr;
  int i, j, prev_num_cookies, valid;

  TEST_CHECK(SIV_GetKeyLength(AEAD_AES_SIV_CMAC_256) > 0);

  SCK_GetLoopbackIPAddress(AF_INET, &addr.ip_addr);
  addr.port = 0;

  inst = NNC_CreateInstance(&addr, "test", 0, 0);
  TEST_CHECK(inst);

  for (i = 0; i < 100000; i++) {
    if (!get_request(inst))
      continue;

    valid = random() % 2;

    TEST_CHECK(!inst->nak_response);
    TEST_CHECK(!inst->ok_response);

    if (random() % 2) {
      prepare_response(inst, &packet, &info, 0, 1);
      TEST_CHECK(!NNC_CheckResponseAuth(inst, &packet, &info));
      TEST_CHECK(!inst->nak_response);
      TEST_CHECK(!inst->ok_response);
      for (j = random() % 3; j > 0; j--) {
        prepare_response(inst, &packet, &info, 1, 1);
        TEST_CHECK(!NNC_CheckResponseAuth(inst, &packet, &info));
        TEST_CHECK(inst->nak_response);
        TEST_CHECK(!inst->ok_response);
      }
    }

    prev_num_cookies = inst->num_cookies;
    prepare_response(inst, &packet, &info, valid, 0);

    if (valid) {
      TEST_CHECK(NNC_CheckResponseAuth(inst, &packet, &info));
      TEST_CHECK(inst->num_cookies >= MIN(NTS_MAX_COOKIES, prev_num_cookies + 1));
      TEST_CHECK(inst->ok_response);
    }

    prev_num_cookies = inst->num_cookies;
    TEST_CHECK(!NNC_CheckResponseAuth(inst, &packet, &info));
    TEST_CHECK(inst->num_cookies == prev_num_cookies);
    TEST_CHECK(inst->ok_response == valid);

    if (random() % 10 == 0) {
      TST_GetRandomAddress(&ip_addr, IPADDR_INET4, 32);
      NNC_ChangeAddress(inst, &ip_addr);
      TEST_CHECK(UTI_CompareIPs(&inst->nts_address.ip_addr, &ip_addr, NULL) == 0);
    }
  }

  NNC_DestroyInstance(inst);
}
#else
void
test_unit(void)
{
  TEST_REQUIRE(0);
}
#endif
