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
#include <nts_ke_session.h>
#include <util.h>

#define NKSN_GetKeys get_keys

static int
get_keys(NKSN_Instance session, SIV_Algorithm algorithm, SIV_Algorithm exporter_algorithm,
         int next_protocol, NKE_Key *c2s, NKE_Key *s2c)
{
  c2s->length = SIV_GetKeyLength(algorithm);
  UTI_GetRandomBytes(c2s->key, c2s->length);
  s2c->length = SIV_GetKeyLength(algorithm);
  UTI_GetRandomBytes(s2c->key, s2c->length);
  return 1;
}

#include <nts_ke_server.c>

static void
prepare_request(NKSN_Instance session, int valid)
{
  uint16_t data[16];
  int index, length;

  if (valid)
    index = -1;
  else
    index = random() % 9;
  DEBUG_LOG("index=%d", index);

  NKSN_BeginMessage(session);

  memset(data, 0, sizeof (data));
  length = 2;
  assert(sizeof (data[0]) == 2);

  if (index != 0) {
    memset(data, NKE_NEXT_PROTOCOL_NTPV4 + 1, sizeof (data));
    data[0] = htons(NKE_NEXT_PROTOCOL_NTPV4);
    if (index == 1)
      length = 0;
    else if (index == 2)
      length = 3 + random() % 15 * 2;
    else
      length = 2 + random() % 16 * 2;
    TEST_CHECK(NKSN_AddRecord(session, 1, NKE_RECORD_NEXT_PROTOCOL, data, length));
  }

  if (index == 3)
    TEST_CHECK(NKSN_AddRecord(session, 1, NKE_RECORD_NEXT_PROTOCOL, data, length));

  if (index != 4) {
    data[0] = htons(random() % 2 && SIV_GetKeyLength(AEAD_AES_128_GCM_SIV) > 0 ?
                    AEAD_AES_128_GCM_SIV : AEAD_AES_SIV_CMAC_256);
    if (index == 5)
      length = 0;
    else if (index == 6)
      length = 3 + random() % 15 * 2;
    else
      length = 2 + random() % 16 * 2;
    TEST_CHECK(NKSN_AddRecord(session, 1, NKE_RECORD_AEAD_ALGORITHM, data, length));
  }

  if (index == 7)
    TEST_CHECK(NKSN_AddRecord(session, 1, NKE_RECORD_AEAD_ALGORITHM, data, length));

  if (index == 8) {
    length = random() % (sizeof (data) + 1);
    TEST_CHECK(NKSN_AddRecord(session, 1, 2000 + random() % 1000, data, length));
  }

  if (random() % 2) {
    const char server[] = "127.0.0.1";
    TEST_CHECK(NKSN_AddRecord(session, 0, NKE_RECORD_NTPV4_SERVER_NEGOTIATION,
                              server, sizeof (server) - 1));
  }

  if (random() % 2) {
    data[0] = htons(123);
    TEST_CHECK(NKSN_AddRecord(session, 0, NKE_RECORD_NTPV4_PORT_NEGOTIATION, data, length));
  }

  if (random() % 2)
    TEST_CHECK(NKSN_AddRecord(session, 0, NKE_RECORD_COMPLIANT_128GCM_EXPORT, NULL, 0));

  if (random() % 2) {
    length = random() % (sizeof (data) + 1);
    TEST_CHECK(NKSN_AddRecord(session, 0, 2000 + random() % 1000, data, length));
  }

  TEST_CHECK(NKSN_EndMessage(session));
}

static void
process_response(NKSN_Instance session, int valid)
{
  int records, errors, critical, type, length;

  for (records = errors = 0; ; records++) {
    if (!NKSN_GetRecord(session, &critical, &type, &length, NULL, 0))
      break;
    if (type == NKE_RECORD_ERROR)
      errors++;
  }

  if (valid) {
    TEST_CHECK(records >= 2);
  } else {
    TEST_CHECK(records == 1);
    TEST_CHECK(errors == 1);
  }
}

void
test_unit(void)
{
  NKSN_Instance session;
  NKE_Context context, context2;
  NKE_Cookie cookie;
  int i, j, valid, l;
  uint32_t sum, sum2;

  char conf[][100] = {
    "ntsdumpdir .",
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

  unlink("ntskeys");
  NKS_PreInitialise(0, 0, 0);
  NKS_Initialise();

  session = NKSN_CreateInstance(1, NULL, handle_message, NULL);

  for (i = 0; i < 10000; i++) {
    valid = random() % 2;
    prepare_request(session, valid);
    TEST_CHECK(process_request(session));
    process_response(session, valid);
  }


  for (i = 0; i < 10000; i++) {
    context.algorithm = AEAD_AES_SIV_CMAC_256;
    get_keys(session, context.algorithm, random() % 100, NKE_NEXT_PROTOCOL_NTPV4,
             &context.c2s, &context.s2c);
    memset(&cookie, 0, sizeof (cookie));
    TEST_CHECK(NKS_GenerateCookie(&context, &cookie));
    TEST_CHECK(NKS_DecodeCookie(&cookie, &context2));
    TEST_CHECK(context.algorithm == context2.algorithm);
    TEST_CHECK(context.c2s.length == context2.c2s.length);
    TEST_CHECK(context.s2c.length == context2.s2c.length);
    TEST_CHECK(memcmp(context.c2s.key, context2.c2s.key, context.c2s.length) == 0);
    TEST_CHECK(memcmp(context.s2c.key, context2.s2c.key, context.s2c.length) == 0);

    if (random() % 4) {
      cookie.cookie[random() % (cookie.length)]++;
    } else if (random() % 4) {
      generate_key(current_server_key);
    } else {
      l = cookie.length;
      while (l == cookie.length)
        cookie.length = random() % (sizeof (cookie.cookie) + 1);
    }
    TEST_CHECK(!NKS_DecodeCookie(&cookie, &context2));
  }

  unlink("ntskeys");
  save_keys();

  for (i = 0, sum = 0; i < MAX_SERVER_KEYS; i++) {
    sum += server_keys[i].id;
    for (j = 0; j < sizeof (server_keys[i].key); j++)
      sum += server_keys[i].key[j];
    generate_key(i);
  }

  load_keys();
  TEST_CHECK(unlink("ntskeys") == 0);

  for (i = 0, sum2 = 0; i < MAX_SERVER_KEYS; i++) {
    sum2 += server_keys[i].id;
    for (j = 0; j < sizeof (server_keys[i].key); j++)
      sum2 += server_keys[i].key[j];
  }

  TEST_CHECK(sum == sum2);

  NKSN_DestroyInstance(session);

  NKS_Finalise();
  TEST_CHECK(unlink("ntskeys") == 0);

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
