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

#include <local.h>
#include <sched.h>

#include <nts_ntp_server.c>

static void
prepare_request(NTP_Packet *packet, NTP_PacketInfo *info, int valid, int nak)
{
  unsigned char uniq_id[NTS_MIN_UNIQ_ID_LENGTH], nonce[NTS_MIN_UNPADDED_NONCE_LENGTH];
  SIV_Instance siv;
  NKE_Context context;
  NKE_Cookie cookie;
  int i, index, cookie_start, auth_start;

  context.algorithm = random() % 2 && SIV_GetKeyLength(AEAD_AES_128_GCM_SIV) > 0 ?
                      AEAD_AES_128_GCM_SIV : AEAD_AES_SIV_CMAC_256;
  context.c2s.length = SIV_GetKeyLength(context.algorithm);
  assert(context.c2s.length <= sizeof (context.c2s.key));
  UTI_GetRandomBytes(&context.c2s.key, context.c2s.length);
  context.s2c.length = SIV_GetKeyLength(context.algorithm);
  assert(context.s2c.length <= sizeof (context.s2c.key));
  UTI_GetRandomBytes(&context.s2c.key, context.s2c.length);

  TEST_CHECK(NKS_GenerateCookie(&context, &cookie));

  UTI_GetRandomBytes(uniq_id, sizeof (uniq_id));
  UTI_GetRandomBytes(nonce, sizeof (nonce));

  memset(packet, 0, sizeof (*packet));
  packet->lvm = NTP_LVM(0, 4, MODE_CLIENT);
  memset(info, 0, sizeof (*info));
  info->version = 4;
  info->mode = MODE_CLIENT;
  info->length = NTP_HEADER_LENGTH;

  if (valid)
    index = -1;
  else
    index = random() % 3;

  DEBUG_LOG("valid=%d nak=%d index=%d", valid, nak, index);

  if (index != 0)
    TEST_CHECK(NEF_AddField(packet, info, NTP_EF_NTS_UNIQUE_IDENTIFIER,
                            uniq_id, sizeof (uniq_id)));

  cookie_start = info->length;

  if (index != 1)
    TEST_CHECK(NEF_AddField(packet, info, NTP_EF_NTS_COOKIE,
                            cookie.cookie, cookie.length));

  for (i = random() % 4; i > 0; i--)
    TEST_CHECK(NEF_AddField(packet, info, NTP_EF_NTS_COOKIE_PLACEHOLDER,
                            cookie.cookie, cookie.length));

  auth_start = info->length;

  if (index != 2) {
    siv = SIV_CreateInstance(context.algorithm);
    TEST_CHECK(siv);
    TEST_CHECK(SIV_SetKey(siv, context.c2s.key, context.c2s.length));
    TEST_CHECK(NNA_GenerateAuthEF(packet, info, siv, nonce, sizeof (nonce),
                                  (const unsigned char *)"", 0, 0));
    SIV_DestroyInstance(siv);
  }

  if (nak)
    ((unsigned char *)packet)[(index == 2 ? cookie_start :
                               (index == 1 ? auth_start :
                                (random() % 2 ? cookie_start : auth_start))) +
                              4 + random() % 16]++;
}

static void
init_response(NTP_Packet *packet, NTP_PacketInfo *info)
{
  memset(packet, 0, sizeof (*packet));
  packet->lvm = NTP_LVM(0, 4, MODE_SERVER);
  memset(info, 0, sizeof (*info));
  info->version = 4;
  info->mode = MODE_SERVER;
  info->length = NTP_HEADER_LENGTH;
}

void
test_unit(void)
{
  NTP_PacketInfo req_info, res_info;
  NTP_Packet request, response;
  int i, valid, nak;
  uint32_t kod;

  char conf[][100] = {
    "ntsport 0",
    "ntsprocesses 0",
    "ntsserverkey nts_ke.key",
    "ntsservercert nts_ke.crt",
  };

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);

  LCL_Initialise();
  TST_RegisterDummyDrivers();
  SCH_Initialise();
  NKS_PreInitialise(0, 0, 0);
  NKS_Initialise();
  NNS_Initialise();

  for (i = 0; i < 50000; i++) {
    valid = random() % 2;
    nak = random() % 2;
    prepare_request(&request, &req_info, valid, nak);

    TEST_CHECK(NNS_CheckRequestAuth(&request, &req_info, &kod) == (valid && !nak));

    if (valid && !nak) {
      TEST_CHECK(kod == 0);
      TEST_CHECK(server->num_cookies > 0);

      init_response(&response, &res_info);
      TEST_CHECK(NNS_GenerateResponseAuth(&request, &req_info, &response, &res_info, kod));

      TEST_CHECK(res_info.ext_fields == 2);
      TEST_CHECK(server->num_cookies == 0);
    } else if (valid && nak) {
      TEST_CHECK(kod == NTP_KOD_NTS_NAK);
      TEST_CHECK(server->num_cookies == 0);

      init_response(&response, &res_info);
      TEST_CHECK(NNS_GenerateResponseAuth(&request, &req_info, &response, &res_info, kod));

      TEST_CHECK(res_info.ext_fields == 1);
      TEST_CHECK(server->num_cookies == 0);
    } else {
      TEST_CHECK(kod == 0);
      TEST_CHECK(server->num_cookies == 0);
    }
  }

  NNS_Finalise();
  NKS_Finalise();
  SCH_Finalise();
  LCL_Finalise();
  CNF_Finalise();
}
#else
void
test_unit(void)
{
  TEST_REQUIRE(0);
}
#endif
