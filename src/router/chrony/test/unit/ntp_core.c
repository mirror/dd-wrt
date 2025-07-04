/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017-2018, 2023
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
#include <ntp_ext.h>
#include <ntp_io.h>
#include <sched.h>
#include <local.h>
#include "test.h"

static struct timespec current_time;
static NTP_Packet req_buffer, res_buffer;
static int req_length, res_length;

#define NIO_IsHwTsEnabled() 1
#define NIO_OpenServerSocket(addr) ((addr)->ip_addr.family != IPADDR_UNSPEC ? 100 : 0)
#define NIO_CloseServerSocket(fd) assert(fd == 100)
#define NIO_OpenClientSocket(addr) ((addr)->ip_addr.family != IPADDR_UNSPEC ? 101 : 0)
#define NIO_CloseClientSocket(fd) assert(fd == 101)
#define NIO_IsServerSocket(fd) (fd == 100)
#define NIO_IsServerSocketOpen() 1
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
    id = random() % 8 + 2;
  } while (!KEY_KeyKnown(id));

  return id;
}

static void
send_request(NCR_Instance inst, int late_hwts)
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
    zero_local_timestamp(&local_ts);
    local_ts.ts = current_time;
    local_ts.source = NTP_TS_KERNEL;

    NCR_ProcessTxKnown(inst, &local_addr, &local_ts, &req_buffer, req_length);
  }

  if (late_hwts) {
    inst->report.total_good_count++;
  } else {
    inst->report.total_good_count = 0;
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
  zero_local_timestamp(&local_ts);
  local_ts.ts = current_time;
  local_ts.source = NTP_TS_KERNEL;

  res_length = 0;
  NCR_ProcessRxUnknown(remote_addr, &local_addr, &local_ts,
                       &req_buffer, req_length);
  res_length = req_length;
  res_buffer = req_buffer;

  advance_time(1e-5);

  if (random() % 2) {
    local_ts.ts = current_time;
    NCR_ProcessTxUnknown(remote_addr, &local_addr, &local_ts,
                         &res_buffer, res_length);
  }
}

static void
send_response(int interleaved, int authenticated, int allow_update, int valid_ts, int valid_auth)
{
  NTP_Packet *req, *res;
  uint32_t key_id = 0;
  int i, auth_len = 0, ef_len, efs;

  req = &req_buffer;
  res = &res_buffer;

  TEST_CHECK(req_length >= NTP_HEADER_LENGTH);

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

  res_length = NTP_HEADER_LENGTH;

  if (NTP_LVM_TO_VERSION(res->lvm) == 4 && random() % 2) {
    unsigned char buf[128];

    memset(buf, 0, sizeof (buf));
    efs = random() % 5;

    for (i = 0; i < efs; i++) {
      ef_len = (i + 1 == efs ? NTP_MAX_V4_MAC_LENGTH + 4 : NTP_MIN_EF_LENGTH) +
               4 * (random() % 10);
      TEST_CHECK(NEF_SetField((unsigned char *)res, sizeof (*res), res_length, 0,
                              buf, ef_len - 4, &ef_len));
      res_length += ef_len;
    }
  }

  if (authenticated) {
    key_id = ntohl(*(uint32_t *)req->extensions);
    if (key_id == 0)
      key_id = get_random_key_id();
    auth_len = KEY_GetAuthLength(key_id);
    assert(auth_len);
    if (NTP_LVM_TO_VERSION(res->lvm) == 4)
      auth_len = MIN(auth_len, NTP_MAX_V4_MAC_LENGTH - 4);

    if (KEY_GenerateAuth(key_id, res, res_length,
                         (unsigned char *)res + res_length + 4, auth_len) != auth_len)
      assert(0);
    res_length += 4 + auth_len;
  }

  if (!valid_auth && authenticated) {
    assert(auth_len);

    switch (random() % 5) {
      case 0:
        key_id++;
        break;
      case 1:
        key_id ^= 1;
        if (KEY_GenerateAuth(key_id, res, res_length - auth_len - 4,
                             (unsigned char *)res + res_length - auth_len, auth_len) != auth_len)
          assert(0);
        break;
      case 2:
        ((unsigned char *)res)[res_length - auth_len + random() % auth_len]++;
        break;
      case 3:
        res_length -= 4 + auth_len;
        auth_len = 4 * (random() % (auth_len / 4));
        res_length += 4 + auth_len;
        break;
      case 4:
        if (NTP_LVM_TO_VERSION(res->lvm) == 4 && random() % 2 &&
            KEY_GetAuthLength(key_id) > NTP_MAX_V4_MAC_LENGTH - 4) {
          res_length -= 4 + auth_len;
          auth_len += 4 + 4 * (random() %
                               ((KEY_GetAuthLength(key_id) - NTP_MAX_V4_MAC_LENGTH - 4) / 4));
          if (KEY_GenerateAuth(key_id, res, res_length,
                               (unsigned char *)res + res_length + 4, auth_len) != auth_len)
              assert(0);
          res_length += 4 + auth_len;
        } else {
          memset((unsigned char *)res + res_length, 0, 4);
          auth_len += 4;
          res_length += 4;
        }
        break;
      default:
        assert(0);
    }
  }

  assert(res_length <= sizeof (*res));
  assert(res_length >= NTP_HEADER_LENGTH + auth_len);

  if (authenticated)
    *(uint32_t *)((unsigned char *)res + res_length - auth_len - 4) = htonl(key_id);
}

static void
proc_response(NCR_Instance inst, int good, int valid, int updated_sync,
              int updated_init, int save)
{
  NTP_Local_Address local_addr;
  NTP_Local_Timestamp local_ts;
  NTP_Packet *res;
  uint32_t prev_rx_count, prev_valid_count;
  struct timespec prev_rx_ts, prev_init_rx_ts;
  int ret;

  res = &res_buffer;

  local_addr.ip_addr.family = IPADDR_UNSPEC;
  local_addr.if_index = INVALID_IF_INDEX;
  local_addr.sock_fd = NTP_LVM_TO_MODE(res->lvm) != MODE_SERVER ? 100 : 101;
  zero_local_timestamp(&local_ts);
  local_ts.ts = current_time;
  local_ts.source = NTP_TS_KERNEL;

  prev_rx_count = inst->report.total_rx_count;
  prev_valid_count = inst->report.total_valid_count;
  prev_rx_ts = inst->local_rx.ts;
  prev_init_rx_ts = inst->init_local_rx.ts;

  ret = NCR_ProcessRxKnown(inst, &local_addr, &local_ts, res, res_length);

  if (save) {
    TEST_CHECK(ret);
    TEST_CHECK(inst->saved_response);
    TEST_CHECK(inst->saved_response->timeout_id != 0);
    TEST_CHECK(has_saved_response(inst));
    if (random() % 2)
      saved_response_timeout(inst);
    else
      transmit_timeout(inst);
    TEST_CHECK(inst->saved_response->timeout_id == 0);
    TEST_CHECK(!has_saved_response(inst));
  }

  if (good > 0)
    TEST_CHECK(ret);
  else if (!good)
    TEST_CHECK(!ret);

  TEST_CHECK(prev_rx_count + 1 == inst->report.total_rx_count);

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
process_replay(NCR_Instance inst, NTP_Packet *packet_queue,
               int queue_length, int updated_init)
{
  do {
    res_buffer = packet_queue[random() % queue_length];
  } while (!UTI_CompareNtp64(&res_buffer.transmit_ts, &inst->remote_ntp_tx));
  proc_response(inst, 0, 0, 0, updated_init, 0);
  advance_time(1e-6);
}

static void
add_dummy_auth(NTP_AuthMode auth_mode, uint32_t key_id, NTP_Packet *packet, NTP_PacketInfo *info)
{
  unsigned char buf[64];
  int len, fill;

  info->auth.mode = auth_mode;

  switch (auth_mode) {
    case NTP_AUTH_NONE:
      break;
    case NTP_AUTH_SYMMETRIC:
    case NTP_AUTH_MSSNTP:
    case NTP_AUTH_MSSNTP_EXT:
      switch (auth_mode) {
        case NTP_AUTH_SYMMETRIC:
          len = 16 + random() % 2 * 4;
          fill = 1 + random() % 255;
          break;
        case NTP_AUTH_MSSNTP:
          len = 16;
          fill = 0;
          break;
        case NTP_AUTH_MSSNTP_EXT:
          len = 68;
          fill = 0;
          break;
        default:
          assert(0);
      }

      assert(info->length + 4 + len <= sizeof (*packet));

      *(uint32_t *)((unsigned char *)packet + info->length) = htonl(key_id);
      info->auth.mac.key_id = key_id;
      info->length += 4;

      memset((unsigned char *)packet + info->length, fill, len);
      info->length += len;
      break;
    case NTP_AUTH_NTS:
      memset(buf, 0, sizeof (buf));
      TEST_CHECK(NEF_AddField(packet, info, NTP_EF_NTS_AUTH_AND_EEF, buf, sizeof (buf)));
      break;
    default:
      assert(0);
  }
}

#define PACKET_QUEUE_LENGTH 10

void
test_unit(void)
{
  char source_line[] = "127.0.0.1 maxdelaydevratio 1e6 noselect";
  char conf[][100] = {
    "allow",
    "port 0",
    "local",
    "keyfile ntp_core.keys"
  };
  int i, j, k, interleaved, authenticated, valid, updated, has_updated, late_hwts;
  CPS_NTP_Source source;
  NTP_Remote_Address remote_addr;
  NCR_Instance inst1, inst2;
  NTP_Packet packet_queue[PACKET_QUEUE_LENGTH], packet;
  NTP_PacketInfo info;

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);

  LCL_Initialise();
  TST_RegisterDummyDrivers();
  SCH_Initialise();
  SRC_Initialise();
  NIO_Initialise();
  NCR_Initialise();
  REF_Initialise();
  KEY_Initialise();
  CLG_Initialise();

  CNF_SetupAccessRestrictions();

  CPS_ParseNTPSourceAdd(source_line, &source);

  for (i = 0; i < 1000; i++) {
    source.params.interleaved = random() % 2;
    source.params.authkey = random() % 2 ? get_random_key_id() : INACTIVE_AUTHKEY;
    source.params.version = random() % 4 + 1;

    UTI_ZeroTimespec(&current_time);
#if HAVE_LONG_TIME_T
    advance_time(NTP_ERA_SPLIT);
#endif
    advance_time(TST_GetRandomDouble(1.0, 1e9));

    TST_GetRandomAddress(&remote_addr.ip_addr, IPADDR_UNSPEC, -1);
    remote_addr.port = 123;

    inst1 = NCR_CreateInstance(&remote_addr, random() % 2 ? NTP_SERVER : NTP_PEER,
                               &source.params, NULL);
    NCR_StartInstance(inst1);
    has_updated = 0;

    for (j = 0; j < 50; j++) {
      DEBUG_LOG("client/peer test iteration %d/%d", i, j);

      late_hwts = random() % 2;
      authenticated = random() % 2;
      interleaved = random() % 2 && (inst1->mode != MODE_CLIENT ||
                                     inst1->tx_count < MAX_CLIENT_INTERLEAVED_TX);
      authenticated = random() % 2;
      valid = (!interleaved || (source.params.interleaved && has_updated)) &&
              ((source.params.authkey == INACTIVE_AUTHKEY) == !authenticated);
      updated = (valid || inst1->mode == MODE_ACTIVE) &&
                ((source.params.authkey == INACTIVE_AUTHKEY) == !authenticated);
      has_updated = has_updated || updated;
      if (inst1->mode == MODE_CLIENT)
        updated = 0;

      DEBUG_LOG("authkey=%d version=%d interleaved=%d authenticated=%d valid=%d updated=%d has_updated=%d",
                (int)source.params.authkey, source.params.version,
                interleaved, authenticated, valid, updated, has_updated);

      send_request(inst1, late_hwts);

      send_response(interleaved, authenticated, 1, 0, 1);
      DEBUG_LOG("response 1");
      proc_response(inst1, 0, 0, 0, updated, 0);

      if (source.params.authkey) {
        send_response(interleaved, authenticated, 1, 1, 0);
        DEBUG_LOG("response 2");
        proc_response(inst1, 0, 0, 0, 0, 0);
      }

      send_response(interleaved, authenticated, 1, 1, 1);
      DEBUG_LOG("response 3");
      proc_response(inst1, -1, valid, valid, updated, valid && late_hwts);
      DEBUG_LOG("response 4");
      proc_response(inst1, 0, 0, 0, 0, 0);

      advance_time(-1.0);

      send_response(interleaved, authenticated, 1, 1, 1);
      DEBUG_LOG("response 5");
      proc_response(inst1, 0, 0, 0, updated && valid, 0);

      advance_time(1.0);

      send_response(interleaved, authenticated, 1, 1, 1);
      DEBUG_LOG("response 6");
      proc_response(inst1, 0, 0, valid && updated, updated, 0);
    }

    NCR_DestroyInstance(inst1);

    inst1 = NCR_CreateInstance(&remote_addr, random() % 2 ? NTP_SERVER : NTP_PEER,
                               &source.params, NULL);
    NCR_StartInstance(inst1);

    for (j = 0; j < 20; j++) {
      DEBUG_LOG("server test iteration %d/%d", i, j);

      send_request(inst1, 0);
      process_request(&remote_addr);
      proc_response(inst1,
                    !source.params.interleaved || source.params.version != 4 ||
                      inst1->mode == MODE_ACTIVE || j != 2,
                    1, 1, 0, 0);
      advance_time(1 << inst1->local_poll);
    }

    NCR_DestroyInstance(inst1);

    inst1 = NCR_CreateInstance(&remote_addr, NTP_PEER, &source.params, NULL);
    NCR_StartInstance(inst1);
    inst2 = NCR_CreateInstance(&remote_addr, NTP_PEER, &source.params, NULL);
    NCR_StartInstance(inst2);

    res_length = req_length = 0;

    for (j = 0; j < 20; j++) {
      DEBUG_LOG("peer replay test iteration %d/%d", i, j);

      late_hwts = random() % 2;

      send_request(inst1, late_hwts);
      res_buffer = req_buffer;
      assert(!res_length || res_length == req_length);
      res_length = req_length;

      TEST_CHECK(inst1->valid_timestamps == (j > 0));

      DEBUG_LOG("response 1->2");
      proc_response(inst2, j > source.params.interleaved, j > 0, j > 0, 1, 0);

      packet_queue[(j * 2) % PACKET_QUEUE_LENGTH] = res_buffer;

      for (k = 0; k < j % 4 + 1; k++) {
        DEBUG_LOG("replay ?->1 %d", k);
        process_replay(inst1, packet_queue, MIN(j * 2 + 1, PACKET_QUEUE_LENGTH), k ? -1 : 1);
        DEBUG_LOG("replay ?->2 %d", k);
        process_replay(inst2, packet_queue, MIN(j * 2 + 1, PACKET_QUEUE_LENGTH), -1);
      }

      advance_time(1 << (source.params.minpoll - 1));

      send_request(inst2, 0);
      res_buffer = req_buffer;
      assert(res_length == req_length);

      TEST_CHECK(inst2->valid_timestamps == (j > 0));

      DEBUG_LOG("response 2->1");
      proc_response(inst1, 1, 1, 1, 1, late_hwts);

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

  memset(&packet, 0, sizeof (packet));
  packet.lvm = NTP_LVM(LEAP_Normal, NTP_VERSION, MODE_CLIENT);

  TEST_CHECK(parse_packet(&packet, NTP_HEADER_LENGTH, &info));
  TEST_CHECK(info.auth.mode == NTP_AUTH_NONE);

  TEST_CHECK(parse_packet(&packet, NTP_HEADER_LENGTH, &info));
  add_dummy_auth(NTP_AUTH_SYMMETRIC, 100, &packet, &info);
  memset(&info.auth, 0, sizeof (info.auth));
  TEST_CHECK(parse_packet(&packet, info.length, &info));
  TEST_CHECK(info.auth.mode == NTP_AUTH_SYMMETRIC);
  TEST_CHECK(info.auth.mac.start == NTP_HEADER_LENGTH);
  TEST_CHECK(info.auth.mac.length == info.length - NTP_HEADER_LENGTH);
  TEST_CHECK(info.auth.mac.key_id == 100);

  TEST_CHECK(parse_packet(&packet, NTP_HEADER_LENGTH, &info));
  add_dummy_auth(NTP_AUTH_NTS, 0, &packet, &info);
  memset(&info.auth, 0, sizeof (info.auth));
  TEST_CHECK(parse_packet(&packet, info.length, &info));
  TEST_CHECK(info.auth.mode == NTP_AUTH_NTS);

  packet.lvm = NTP_LVM(LEAP_Normal, 3, MODE_CLIENT);

  TEST_CHECK(parse_packet(&packet, NTP_HEADER_LENGTH, &info));
  add_dummy_auth(NTP_AUTH_MSSNTP, 200, &packet, &info);
  memset(&info.auth, 0, sizeof (info.auth));
  TEST_CHECK(parse_packet(&packet, info.length, &info));
  TEST_CHECK(info.auth.mode == NTP_AUTH_MSSNTP);
  TEST_CHECK(info.auth.mac.start == NTP_HEADER_LENGTH);
  TEST_CHECK(info.auth.mac.length == 20);
  TEST_CHECK(info.auth.mac.key_id == 200);

  TEST_CHECK(parse_packet(&packet, NTP_HEADER_LENGTH, &info));
  add_dummy_auth(NTP_AUTH_MSSNTP_EXT, 300, &packet, &info);
  memset(&info.auth, 0, sizeof (info.auth));
  TEST_CHECK(parse_packet(&packet, info.length, &info));
  TEST_CHECK(info.auth.mode == NTP_AUTH_MSSNTP_EXT);
  TEST_CHECK(info.auth.mac.start == NTP_HEADER_LENGTH);
  TEST_CHECK(info.auth.mac.length == 72);
  TEST_CHECK(info.auth.mac.key_id == 300);

  CLG_Finalise();
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
