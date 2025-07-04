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

#include <nts_ke_session.c>

#include <local.h>
#include <socket.h>
#include <sched.h>

static NKSN_Instance client, server;
static unsigned char record[NKE_MAX_MESSAGE_LENGTH];
static int record_length, critical, type_start, records;
static int request_received;
static int response_received;

static void
send_message(NKSN_Instance inst)
{
  int i;

  record_length = random() % (NKE_MAX_MESSAGE_LENGTH - 4 + 1);
  for (i = 0; i < record_length; i++)
    record[i] = random() % 256;
  critical = random() % 2;
  type_start = random() % 30000 + 1;
  assert(sizeof (struct RecordHeader) == 4);
  records = random() % ((NKE_MAX_MESSAGE_LENGTH - 4) / (4 + record_length) + 1);

  DEBUG_LOG("critical=%d type_start=%d records=%d*%d",
            critical, type_start, records, record_length);

  NKSN_BeginMessage(inst);

  TEST_CHECK(check_message_format(&inst->message, 0));
  TEST_CHECK(!check_message_format(&inst->message, 1));

  TEST_CHECK(!NKSN_AddRecord(inst, 0, 1, record, NKE_MAX_MESSAGE_LENGTH - 4 + 1));

  TEST_CHECK(check_message_format(&inst->message, 0));
  TEST_CHECK(!check_message_format(&inst->message, 1));

  for (i = 0; i < records; i++) {
    TEST_CHECK(NKSN_AddRecord(inst, critical, type_start + i, record, record_length));
    TEST_CHECK(!NKSN_AddRecord(inst, 0, 1, &record,
                               NKE_MAX_MESSAGE_LENGTH - inst->message.length - 4 + 1));

    TEST_CHECK(check_message_format(&inst->message, 0));
    TEST_CHECK(!check_message_format(&inst->message, 1));
  }

  TEST_CHECK(NKSN_EndMessage(inst));

  TEST_CHECK(check_message_format(&inst->message, 0));
  TEST_CHECK(check_message_format(&inst->message, 1));
}

static void
verify_message(NKSN_Instance inst)
{
  unsigned char buffer[NKE_MAX_MESSAGE_LENGTH];
  int i, c, t, length, buffer_length, msg_length, prev_parsed;
  NKE_Key c2s, s2c;

  for (i = 0; i < records; i++) {
    memset(buffer, 0, sizeof (buffer));
    buffer_length = random() % (record_length + 1);
    assert(buffer_length <= sizeof (buffer));

    prev_parsed = inst->message.parsed;
    msg_length = inst->message.length;

    TEST_CHECK(NKSN_GetRecord(inst, &c, &t, &length, buffer, buffer_length));
    TEST_CHECK(c == critical);
    TEST_CHECK(t == type_start + i);
    TEST_CHECK(length == record_length);
    TEST_CHECK(memcmp(record, buffer, buffer_length) == 0);
    if (buffer_length < record_length)
      TEST_CHECK(buffer[buffer_length] == 0);

    inst->message.length = inst->message.parsed - 1;
    inst->message.parsed = prev_parsed;
    TEST_CHECK(!get_record(&inst->message, NULL, NULL, NULL, buffer, buffer_length));
    TEST_CHECK(inst->message.parsed == prev_parsed);
    inst->message.length = msg_length;
    if (msg_length < 0x8000) {
      inst->message.data[prev_parsed + 2] ^= 0x80;
      TEST_CHECK(!get_record(&inst->message, NULL, NULL, NULL, buffer, buffer_length));
      TEST_CHECK(inst->message.parsed == prev_parsed);
      inst->message.data[prev_parsed + 2] ^= 0x80;
    }
    TEST_CHECK(get_record(&inst->message, NULL, NULL, NULL, buffer, buffer_length));
    TEST_CHECK(inst->message.parsed > prev_parsed);
  }

  TEST_CHECK(!NKSN_GetRecord(inst, &critical, &t, &length, buffer, sizeof (buffer)));

  for (i = 0; i < 10; i++) {
    TEST_CHECK(NKSN_GetKeys(inst, AEAD_AES_SIV_CMAC_256, random(), random(), &c2s, &s2c));
    TEST_CHECK(c2s.length == SIV_GetKeyLength(AEAD_AES_SIV_CMAC_256));
    TEST_CHECK(s2c.length == SIV_GetKeyLength(AEAD_AES_SIV_CMAC_256));

    if (SIV_GetKeyLength(AEAD_AES_128_GCM_SIV) > 0) {
      TEST_CHECK(NKSN_GetKeys(inst, AEAD_AES_128_GCM_SIV, random(), random(), &c2s, &s2c));
      TEST_CHECK(c2s.length == SIV_GetKeyLength(AEAD_AES_128_GCM_SIV));
      TEST_CHECK(s2c.length == SIV_GetKeyLength(AEAD_AES_128_GCM_SIV));
    } else {
      TEST_CHECK(!NKSN_GetKeys(inst, AEAD_AES_128_GCM_SIV, random(), random(), &c2s, &s2c));
    }
  }
}

static int
handle_request(void *arg)
{
  NKSN_Instance server = arg;

  verify_message(server);

  request_received = 1;

  send_message(server);

  return 1;
}

static int
handle_response(void *arg)
{
  NKSN_Instance client = arg;

  response_received = 1;

  verify_message(client);

  return 1;
}

static void
check_finished(void *arg)
{
  DEBUG_LOG("checking for stopped sessions");
  if (!NKSN_IsStopped(server) || !NKSN_IsStopped(client)) {
    SCH_AddTimeoutByDelay(0.001, check_finished, NULL);
    return;
  }

  SCH_QuitProgram();
}

void
test_unit(void)
{
  NKSN_Credentials client_cred, server_cred;
  const char *cert, *key;
  int sock_fds[2], i;
  uint32_t cert_id;
  NKE_Key c2s, s2c;

  LCL_Initialise();
  TST_RegisterDummyDrivers();

  cert = "nts_ke.crt";
  key = "nts_ke.key";
  cert_id = 0;

  for (i = 0; i < 50; i++) {
    SCH_Initialise();

    server = NKSN_CreateInstance(1, NULL, handle_request, NULL);
    client = NKSN_CreateInstance(0, "test", handle_response, NULL);

    server_cred = NKSN_CreateServerCertCredentials(&cert, &key, 1);
    client_cred = NKSN_CreateClientCertCredentials(&cert, &cert_id, 1, 0);

    TEST_CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fds) == 0);
    TEST_CHECK(fcntl(sock_fds[0], F_SETFL, O_NONBLOCK) == 0);
    TEST_CHECK(fcntl(sock_fds[1], F_SETFL, O_NONBLOCK) == 0);

    TEST_CHECK(NKSN_StartSession(server, sock_fds[0], "client", server_cred, 4.0));
    TEST_CHECK(NKSN_StartSession(client, sock_fds[1], "server", client_cred, 4.0));

    TEST_CHECK(!NKSN_GetKeys(server, AEAD_AES_SIV_CMAC_256, 0, 0, &c2s, &s2c));
    TEST_CHECK(!NKSN_GetKeys(client, AEAD_AES_SIV_CMAC_256, 0, 0, &c2s, &s2c));

    send_message(client);

    request_received = response_received = 0;

    check_finished(NULL);

    SCH_MainLoop();

    TEST_CHECK(NKSN_IsStopped(server));
    TEST_CHECK(NKSN_IsStopped(client));

    TEST_CHECK(!NKSN_GetKeys(server, AEAD_AES_SIV_CMAC_256, 0, 0, &c2s, &s2c));
    TEST_CHECK(!NKSN_GetKeys(client, AEAD_AES_SIV_CMAC_256, 0, 0, &c2s, &s2c));

    TEST_CHECK(request_received);
    TEST_CHECK(response_received);

    NKSN_DestroyInstance(server);
    NKSN_DestroyInstance(client);

    NKSN_DestroyCertCredentials(server_cred);
    NKSN_DestroyCertCredentials(client_cred);

    SCH_Finalise();
  }

  LCL_Finalise();
}
#else
void
test_unit(void)
{
  TEST_REQUIRE(0);
}
#endif
