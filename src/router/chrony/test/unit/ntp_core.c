/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017-2018
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
#include <sysincl.h>
#include <cmdparse.h>
#include <conf.h>
#include <keys.h>
#include <ntp_io.h>
#include <sched.h>
#include <local.h>
#include "test.h"

#ifdef FEAT_NTP

static struct timespec current_time;
static NTP_Receive_Buffer req_buffer, res_buffer;
static int req_length, res_length;

#define NIO_OpenServerSocket(addr) ((addr)->ip_addr.family != IPADDR_UNSPEC ? 100 : 0)
#define NIO_CloseServerSocket(fd) assert(fd == 100)
#define NIO_OpenClientSocket(addr) ((addr)->ip_addr.family != IPADDR_UNSPEC ? 101 : 0)
#define NIO_CloseClientSocket(fd) assert(fd == 101)
#define NIO_IsServerSocket(fd) (fd == 100)
#define NIO_SendPacket(msg, to, from, len, process_tx) (memcpy(&req_buffer, msg, len), req_length = len, 1)
#define SCH_AddTimeoutByDelay(delay, handler, arg) (1 ? 102 : (handler(arg), 1))
#define SCH_AddTimeoutInClass(delay, separation, randomness, class, handler, arg) \
  add_timeout_in_class(delay, separation, randomness, class, handler, arg)
#define SCH_RemoveTimeout(id) assert(!id || id == 102)
#define LCL_ReadRawTime(ts) (*ts = current_time)
#define LCL_ReadCookedTime(ts, err) do {double *p = err; *ts = current_time; if (p) *p = 0.0;} while (0)
#define LCL_GetSysPrecisionAsLog() (random() % 10 - 30)
#define SRC_UpdateReachability(inst, reach)
#define SRC_ResetReachability(inst)

static SCH_TimeoutID
add_timeout_in_class(double min_delay, double separation, double randomness,
                     SCH_TimeoutClass class, SCH_TimeoutHandler handler, SCH_ArbitraryArgument arg)
{
  return 102;
}

#include <ntp_core.c>

static void
advance_time(double x)
{
  UTI_AddDoubleToTimespec(&current_time, x, &current_time);
}

static uint32_t
get_random_key_id(void)
{
  uint32_t id;

  do {
    id = random() % 6 + 2;
  } while (!KEY_KeyKnown(id));

  return id;
}

static void
send_request(NCR_Instance inst)
{
  NTP_Local_Address local_addr;
  NTP_Local_Timestamp local_ts;
  uint32_t prev_tx_count;

  prev_tx_count = inst->report.total_tx_count;

  transmit_timeout(inst);
  TEST_CHECK(!inst->valid_rx);
  TEST_CHECK(prev_tx_count + 1 == inst->report.total_tx_count);

  advance_time(1e-5);

  if (random() % 2) {
    local_addr.ip_addr.family = IPADDR_UNSPEC;
    local_addr.if_index = INVALID_IF_INDEX;
    local_addr.sock_fd = 101;
    local_ts.ts = current_time;
    local_ts.err = 0.0;
    local_ts.source = NTP_TS_KERNEL;

    NCR_ProcessTxKnown(inst, &local_addr, &local_ts, &req_buffer.ntp_pkt, req_length);
  }
}

static void
process_request(NTP_Remote_Address *remote_addr)
{
  NTP_Local_Address local_addr;
  NTP_Local_Timestamp local_ts;

  local_addr.ip_addr.family = IPADDR_UNSPEC;
  local_addr.if_index = INVALID_IF_INDEX;
  local_addr.sock_fd = 100;
  local_ts.ts = current_time;
  local_ts.err = 0.0;
  local_ts.source = NTP_TS_KERNEL;

  res_length = 0;
  NCR_ProcessRxUnknown(remote_addr, &local_addr, &local_ts,
                       &req_buffer.ntp_pkt, req_length);
  res_length = req_length;
  res_buffer = req_buffer;

  advance_time(1e-5);

  if (random() % 2) {
    local_ts.ts = current_time;
    NCR_ProcessTxUnknown(remote_addr, &local_addr, &local_ts,
                         &res_buffer.ntp_pkt, res_length);
  }
}

static void
send_response(int interleaved, int authenticated, int allow_update, int valid_ts, int valid_auth)
{
  NTP_Packet *req, *res;
  int auth_len = 0;

  req = &req_buffer.ntp_pkt;
  res = &res_buffer.ntp_pkt;

  TEST_CHECK(req_length >= NTP_NORMAL_PACKET_LENGTH);

  res->lvm = NTP_LVM(LEAP_Normal, NTP_LVM_TO_VERSION(req->lvm),
                     NTP_LVM_TO_MODE(req->lvm) == MODE_CLIENT ? MODE_SERVER : MODE_ACTIVE);
  res->stratum = 1;
  res->poll = req->poll;
  res->precision = -20;
  res->root_delay = UTI_DoubleToNtp32(0.1);
  res->root_dispersion = UTI_DoubleToNtp32(0.1);
  res->reference_id = 0;
  UTI_ZeroNtp64(&res->reference_ts);
  res->originate_ts = interleaved ? req->receive_ts : req->transmit_ts;

  advance_time(TST_GetRandomDouble(1e-4, 1e-2));
  UTI_TimespecToNtp64(&current_time, &res->receive_ts, NULL);
  advance_time(TST_GetRandomDouble(-1e-4, 1e-3));
  UTI_TimespecToNtp64(&current_time, &res->transmit_ts, NULL);
  advance_time(TST_GetRandomDouble(1e-4, 1e-2));

  if (!valid_ts) {
    switch (random() % (allow_update ? 4 : 5)) {
      case 0:
        res->originate_ts.hi = random();
        break;
      case 1:
        res->originate_ts.lo = random();
        break;
      case 2:
        UTI_ZeroNtp64(&res->originate_ts);
        break;
      case 3:
        UTI_ZeroNtp64(&res->receive_ts);
        break;
      case 4:
        UTI_ZeroNtp64(&res->transmit_ts);
        break;
      default:
        assert(0);
    }
  }

  if (authenticated) {
    res->auth_keyid = req->auth_keyid ? req->auth_keyid : htonl(get_random_key_id());
    auth_len = KEY_GetAuthLength(ntohl(res->auth_keyid));
    assert(auth_len);
    if (NTP_LVM_TO_VERSION(res->lvm) == 4 && random() % 2)
      auth_len = MIN(auth_len, NTP_MAX_V4_MAC_LENGTH - 4);

    if (KEY_GenerateAuth(ntohl(res->auth_keyid), (unsigned char *)res,
                         NTP_NORMAL_PACKET_LENGTH, res->auth_data, auth_len) != auth_len)
      assert(0);
    res_length = NTP_NORMAL_PACKET_LENGTH + 4 + auth_len;
  } else {
    res_length = NTP_NORMAL_PACKET_LENGTH;
  }

  if (!valid_auth && authenticated) {
    assert(auth_len);

    switch (random() % 4) {
      case 0:
        res->auth_keyid = htonl(ntohl(res->auth_keyid) + 1);
        break;
      case 1:
        res->auth_keyid = htonl(ntohl(res->auth_keyid) ^ 1);
        if (KEY_GenerateAuth(ntohl(res->auth_keyid), (unsigned char *)res,
                             NTP_NORMAL_PACKET_LENGTH, res->auth_data, auth_len) != auth_len)
          assert(0);
        break;
      case 2:
        res->auth_data[random() % auth_len]++;
        break;
      case 3:
        res_length = NTP_NORMAL_PACKET_LENGTH + 4 * (random() % ((4 + auth_len) / 4));
        if (NTP_LVM_TO_VERSION(res->lvm) == 4 &&
            res_length == NTP_NORMAL_PACKET_LENGTH + NTP_MAX_V4_MAC_LENGTH)
          res_length -= 4;
        break;
      default:
        assert(0);
    }
  }
}

static void
process_response(NCR_Instance inst, int good, int valid, int updated_sync, int updated_init)
{
  NTP_Local_Address local_addr;
  NTP_Local_Timestamp local_ts;
  NTP_Packet *res;
  uint32_t prev_rx_count, prev_valid_count;
  struct timespec prev_rx_ts, prev_init_rx_ts;
  int prev_open_socket, ret;

  res = &res_buffer.ntp_pkt;

  local_addr.ip_addr.family = IPADDR_UNSPEC;
  local_addr.if_index = INVALID_IF_INDEX;
  local_addr.sock_fd = NTP_LVM_TO_MODE(res->lvm) != MODE_SERVER ? 100 : 101;
  local_ts.ts = current_time;
  local_ts.err = 0.0;
  local_ts.source = NTP_TS_KERNEL;

  prev_rx_count = inst->report.total_rx_count;
  prev_valid_count = inst->report.total_valid_count;
  prev_rx_ts = inst->local_rx.ts;
  prev_init_rx_ts = inst->init_local_rx.ts;
  prev_open_socket = inst->local_addr.sock_fd != INVALID_SOCK_FD;

  ret = NCR_ProcessRxKnown(inst, &local_addr, &local_ts, res, res_length);

  if (good > 0)
    TEST_CHECK(ret);
  else if (!good)
    TEST_CHECK(!ret);

  if (prev_open_socket)
    TEST_CHECK(prev_rx_count + 1 == inst->report.total_rx_count);
  else
    TEST_CHECK(prev_rx_count == inst->report.total_rx_count);

  if (valid)
    TEST_CHECK(prev_valid_count + 1 == inst->report.total_valid_count);
  else
    TEST_CHECK(prev_valid_count == inst->report.total_valid_count);

  if (updated_sync)
    TEST_CHECK(UTI_CompareTimespecs(&inst->local_rx.ts, &prev_rx_ts));
  else
    TEST_CHECK(!UTI_CompareTimespecs(&inst->local_rx.ts, &prev_rx_ts));

  if (updated_init > 0)
    TEST_CHECK(UTI_CompareTimespecs(&inst->init_local_rx.ts, &prev_init_rx_ts));
  else if (!updated_init)
    TEST_CHECK(!UTI_CompareTimespecs(&inst->init_local_rx.ts, &prev_init_rx_ts));

  if (valid) {
    TEST_CHECK(UTI_IsZeroTimespec(&inst->init_local_rx.ts));
    TEST_CHECK(UTI_IsZeroNtp64(&inst->init_remote_ntp_tx));
  }
}

static void
process_replay(NCR_Instance inst, NTP_Receive_Buffer *packet_queue,
               int queue_length, int updated_init)
{
  do {
    res_buffer = packet_queue[random() % queue_length];
  } while (!UTI_CompareNtp64(&res_buffer.ntp_pkt.transmit_ts,
                             &inst->remote_ntp_tx));
  process_response(inst, 0, 0, 0, updated_init);
  advance_time(1e-6);
}

#define PACKET_QUEUE_LENGTH 10

void
test_unit(void)
{
  char source_line[] = "127.0.0.1 maxdelaydevratio 1e6";
  char conf[][100] = {
    "allow",
    "port 0",
    "local",
    "keyfile ntp_core.keys"
  };
  int i, j, k, interleaved, authenticated, valid, updated, has_updated;
  CPS_NTP_Source source;
  NTP_Remote_Address remote_addr;
  NCR_Instance inst1, inst2;
  NTP_Receive_Buffer packet_queue[PACKET_QUEUE_LENGTH];

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);

  LCL_Initialise();
  TST_RegisterDummyDrivers();
  SCH_Initialise();
  SRC_Initialise();
  NIO_Initialise(IPADDR_UNSPEC);
  NCR_Initialise();
  REF_Initialise();

  TST_SuspendLogging();
  KEY_Initialise();
  TST_ResumeLogging();

  CNF_SetupAccessRestrictions();

  CPS_ParseNTPSourceAdd(source_line, &source);

  for (i = 0; i < 1000; i++) {
    source.params.interleaved = random() % 2;
    source.params.authkey = random() % 2 ? get_random_key_id() : INACTIVE_AUTHKEY;
    source.params.version = random() % 4 + 1;

    UTI_ZeroTimespec(&current_time);
    advance_time(TST_GetRandomDouble(1.0, 1e9));

    TST_GetRandomAddress(&remote_addr.ip_addr, IPADDR_UNSPEC, -1);
    remote_addr.port = 123;

    inst1 = NCR_GetInstance(&remote_addr, random() % 2 ? NTP_SERVER : NTP_PEER, &source.params);
    NCR_StartInstance(inst1);
    has_updated = 0;

    for (j = 0; j < 50; j++) {
      DEBUG_LOG("client/peer test iteration %d/%d", i, j);

      interleaved = random() % 2 && (inst1->mode != MODE_CLIENT ||
                                     inst1->tx_count < MAX_CLIENT_INTERLEAVED_TX);
      authenticated = random() % 2;
      valid = (!interleaved || (source.params.interleaved && has_updated)) &&
              (!source.params.authkey || authenticated);
      updated = (valid || inst1->mode == MODE_ACTIVE) &&
                (!source.params.authkey || authenticated);
      has_updated = has_updated || updated;
      if (inst1->mode == MODE_CLIENT)
        updated = 0;

      send_request(inst1);

      send_response(interleaved, authenticated, 1, 0, 1);
      DEBUG_LOG("response 1");
      process_response(inst1, 0, 0, 0, updated);

      if (source.params.authkey) {
        send_response(interleaved, authenticated, 1, 1, 0);
        DEBUG_LOG("response 2");
        process_response(inst1, 0, 0, 0, 0);
      }

      send_response(interleaved, authenticated, 1, 1, 1);
      DEBUG_LOG("response 3");
      process_response(inst1, -1, valid, valid, updated);
      DEBUG_LOG("response 4");
      process_response(inst1, 0, 0, 0, 0);

      advance_time(-1.0);

      send_response(interleaved, authenticated, 1, 1, 1);
      DEBUG_LOG("response 5");
      process_response(inst1, 0, 0, 0, updated && valid);

      advance_time(1.0);

      send_response(interleaved, authenticated, 1, 1, 1);
      DEBUG_LOG("response 6");
      process_response(inst1, 0, 0, valid && updated, updated);
    }

    NCR_DestroyInstance(inst1);

    inst1 = NCR_GetInstance(&remote_addr, random() % 2 ? NTP_SERVER : NTP_PEER, &source.params);
    NCR_StartInstance(inst1);

    for (j = 0; j < 20; j++) {
      DEBUG_LOG("server test iteration %d/%d", i, j);

      send_request(inst1);
      process_request(&remote_addr);
      process_response(inst1, 1, 1, 1, 0);
      advance_time(1 << inst1->local_poll);
    }

    NCR_DestroyInstance(inst1);

    inst1 = NCR_GetInstance(&remote_addr, NTP_PEER, &source.params);
    NCR_StartInstance(inst1);
    inst2 = NCR_GetInstance(&remote_addr, NTP_PEER, &source.params);
    NCR_StartInstance(inst2);

    res_length = req_length = 0;

    for (j = 0; j < 20; j++) {
      DEBUG_LOG("peer replay test iteration %d/%d", i, j);

      send_request(inst1);
      res_buffer = req_buffer;
      assert(!res_length || res_length == req_length);
      res_length = req_length;

      TEST_CHECK(inst1->valid_timestamps == (j > 0));

      DEBUG_LOG("response 1->2");
      process_response(inst2, j > source.params.interleaved, j > 0, j > 0, 1);

      packet_queue[(j * 2) % PACKET_QUEUE_LENGTH] = res_buffer;

      for (k = 0; k < j % 4 + 1; k++) {
        DEBUG_LOG("replay ?->1 %d", k);
        process_replay(inst1, packet_queue, MIN(j * 2 + 1, PACKET_QUEUE_LENGTH), k ? -1 : 1);
        DEBUG_LOG("replay ?->2 %d", k);
        process_replay(inst2, packet_queue, MIN(j * 2 + 1, PACKET_QUEUE_LENGTH), -1);
      }

      advance_time(1 << (source.params.minpoll - 1));

      send_request(inst2);
      res_buffer = req_buffer;
      assert(res_length == req_length);

      TEST_CHECK(inst2->valid_timestamps == (j > 0));

      DEBUG_LOG("response 2->1");
      process_response(inst1, 1, 1, 1, 1);

      packet_queue[(j * 2 + 1) % PACKET_QUEUE_LENGTH] = res_buffer;

      for (k = 0; k < j % 4 + 1; k++) {
        DEBUG_LOG("replay ?->1 %d", k);
        process_replay(inst1, packet_queue, MIN(j * 2 + 2, PACKET_QUEUE_LENGTH), k ? -1 : 1);
        DEBUG_LOG("replay ?->2 %d", k);
        process_replay(inst2, packet_queue, MIN(j * 2 + 2, PACKET_QUEUE_LENGTH), -1);
      }

      advance_time(1 << (source.params.minpoll - 1));
    }

    NCR_DestroyInstance(inst1);
    NCR_DestroyInstance(inst2);
  }

  KEY_Finalise();
  REF_Finalise();
  NCR_Finalise();
  NIO_Finalise();
  SRC_Finalise();
  SCH_Finalise();
  LCL_Finalise();
  CNF_Finalise();
  HSH_Finalise();
}

#else
void
test_unit(void)
{
  TEST_REQUIRE(0);
}
#endif
