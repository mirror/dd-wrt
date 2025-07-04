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
#include <sysincl.h>
#include <conf.h>
#include <keys.h>
#include <local.h>
#include <ntp_ext.h>
#include <ntp_signd.h>
#include <nts_ntp_server.h>
#include <socket.h>
#include "test.h"

#include <ntp_auth.c>

static void
prepare_packet(NTP_AuthMode auth_mode, NTP_Packet *packet, NTP_PacketInfo *info, int req)
{
  unsigned char buf[64];
  int i, version;
  NTP_Mode mode;

  switch (auth_mode) {
    case NTP_AUTH_MSSNTP:
    case NTP_AUTH_MSSNTP_EXT:
      version = 3;
      mode = random() % 2 ? (req ? MODE_CLIENT : MODE_SERVER) :
                            (req ? MODE_ACTIVE : MODE_PASSIVE);
      break;
    case NTP_AUTH_NTS:
      version = 4;
      mode = req ? MODE_CLIENT : MODE_SERVER;
      break;
    default:
      version = 3 + random() % 2;
      mode = random() % 2 ? (req ? MODE_CLIENT : MODE_SERVER) :
                            (req ? MODE_ACTIVE : MODE_PASSIVE);
      break;
  }

  memset(packet, 0, sizeof (*packet));
  memset(info, 0, sizeof (*info));
  packet->lvm = NTP_LVM(LEAP_Normal, version, mode);
  info->length = NTP_HEADER_LENGTH;
  info->version = version;
  info->mode = mode;

  if (version == 4) {
    memset(buf, 0, sizeof (buf));
    for (i = random() % 5; i > 0; i--)
      TEST_CHECK(NEF_AddField(packet, info, 0, buf, sizeof (buf)));
  }
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

void
test_unit(void)
{
  int i, j, can_auth_req, can_auth_res;
  NTP_PacketInfo req_info, res_info;
  NTP_Packet req, res;
  NAU_Instance inst;
  RPT_AuthReport report;
  NTP_AuthMode mode;
  IPSockAddr nts_addr;
  uint32_t key_id, kod;
  char conf[][100] = {
    "keyfile ntp_core.keys"
  };

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);

  LCL_Initialise();
  KEY_Initialise();
  NSD_Initialise();
  NNS_Initialise();

  SCK_GetAnyLocalIPAddress(IPADDR_INET4, &nts_addr.ip_addr);
  nts_addr.port = 0;

  for (i = 0; i < 1000; i++) {
    key_id = INACTIVE_AUTHKEY;

    switch (i % 5) {
      case 0:
        inst = NAU_CreateNoneInstance();
        TEST_CHECK(!NAU_IsAuthEnabled(inst));
        TEST_CHECK(NAU_GetSuggestedNtpVersion(inst) == 4);
        mode = NTP_AUTH_NONE;
        can_auth_req = 1;
        can_auth_res = 1;
        break;
      case 1:
        key_id = random() % 7 + 2;
        inst = NAU_CreateSymmetricInstance(key_id);
        TEST_CHECK(NAU_IsAuthEnabled(inst));
        TEST_CHECK(NAU_GetSuggestedNtpVersion(inst) ==
                   (KEY_KeyKnown(inst->key_id) && KEY_GetAuthLength(inst->key_id) > 20 ? 3 : 4));
        mode = NTP_AUTH_SYMMETRIC;
        can_auth_req = KEY_KeyKnown(key_id);
        can_auth_res = can_auth_req;
        break;
      case 2:
        inst = NAU_CreateNtsInstance(&nts_addr, "test", 0, 0);
        TEST_CHECK(NAU_IsAuthEnabled(inst));
        TEST_CHECK(NAU_GetSuggestedNtpVersion(inst) == 4);
        mode = NTP_AUTH_NTS;
        can_auth_req = 0;
        can_auth_res = 0;
        break;
      case 3:
        key_id = 1 + random() % 100;
        inst = NULL;
        mode = NTP_AUTH_MSSNTP;
        can_auth_req = 1;
        can_auth_res = 0;
        break;
      case 4:
        key_id = 1 + random() % 100;
        inst = NULL;
        mode = NTP_AUTH_MSSNTP_EXT;
        can_auth_req = 0;
        can_auth_res = 0;
        break;
      default:
        assert(0);
    }

    DEBUG_LOG("iteration %d auth=%d key_id=%d", i, (int)mode, (int)key_id);

    prepare_packet(mode, &req, &req_info, 1);

    if (inst) {
      TEST_CHECK(inst->mode == mode);
      TEST_CHECK(inst->key_id == key_id);

      NAU_DumpData(inst);
      NAU_GetReport(inst, &report);
      if (random() % 2)
        NAU_ChangeAddress(inst, &nts_addr.ip_addr);

      if (inst->mode == NTP_AUTH_NTS) {
        for (j = random() % 5; j > 0; j--)
#ifdef FEAT_NTS
          TEST_CHECK(!NAU_PrepareRequestAuth(inst));
#else
          TEST_CHECK(NAU_PrepareRequestAuth(inst));
#endif
        TEST_CHECK(!NAU_GenerateRequestAuth(inst, &req, &req_info));
      } else if (can_auth_req) {
        TEST_CHECK(NAU_PrepareRequestAuth(inst));
        TEST_CHECK(NAU_GenerateRequestAuth(inst, &req, &req_info));
      } else {
        TEST_CHECK(NAU_PrepareRequestAuth(inst));
        TEST_CHECK(!NAU_GenerateRequestAuth(inst, &req, &req_info));
      }
    }

    if (!inst || !can_auth_req)
      add_dummy_auth(mode, key_id, &req, &req_info);

    assert(req_info.auth.mode == mode);
    assert(req_info.auth.mac.key_id == key_id);

    kod = 1;
    TEST_CHECK(NAU_CheckRequestAuth(&req, &req_info, &kod) == can_auth_req);
    TEST_CHECK(kod == 0);

    if (inst) {
      for (j = NTP_AUTH_NONE; j <= NTP_AUTH_NTS; j++) {
        if (j == mode && j == NTP_AUTH_NONE)
          continue;

        prepare_packet(j, &res, &res_info, 0);
        add_dummy_auth(j, key_id ? key_id : 1, &res, &res_info);

        TEST_CHECK(res_info.auth.mode == j);
        TEST_CHECK(!NAU_CheckResponseAuth(inst, &res, &res_info));
      }
    }

    prepare_packet(mode, &res, &res_info, 0);
    TEST_CHECK(NAU_GenerateResponseAuth(&req, &req_info, &res, &res_info, NULL, NULL, kod) ==
               can_auth_res);
    if (!can_auth_res)
      add_dummy_auth(mode, key_id, &res, &res_info);

    assert(res_info.auth.mode == mode);
    assert(res_info.auth.mac.key_id == key_id);

    if (inst) {
      if (mode == NTP_AUTH_SYMMETRIC) {
        res_info.auth.mac.key_id ^= 1;
        TEST_CHECK(!NAU_CheckResponseAuth(inst, &res, &res_info));
        res_info.auth.mac.key_id ^= 1;
      }

      TEST_CHECK(NAU_CheckResponseAuth(inst, &res, &res_info) == can_auth_res);

      NAU_GetReport(inst, &report);
      NAU_DestroyInstance(inst);
    }
  }

  NNS_Finalise();
  NSD_Finalise();
  KEY_Finalise();
  LCL_Finalise();
  CNF_Finalise();
  HSH_Finalise();
}
