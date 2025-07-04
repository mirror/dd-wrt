/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016, 2021
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

#include <conf.h>
#include <cmdparse.h>
#include <nameserv_async.h>
#include <ntp_core.h>
#include <ntp_io.h>
#include <sched.h>

static char *requested_name = NULL;
static DNS_NameResolveHandler resolve_handler = NULL;
static void *resolve_handler_arg = NULL;

#define DNS_Name2IPAddressAsync(name, handler, arg) \
  requested_name = (name), \
  resolve_handler = (handler), \
  resolve_handler_arg = (arg)
#define NCR_ChangeRemoteAddress(inst, remote_addr, ntp_only) \
  change_remote_address(inst, remote_addr, ntp_only)
#define NCR_ProcessRxKnown(remote_addr, local_addr, ts, msg, len) (random() % 2)
#define NIO_IsServerConnectable(addr) (random() % 2)
#define SCH_GetLastEventMonoTime() get_mono_time()

static void change_remote_address(NCR_Instance inst, NTP_Remote_Address *remote_addr,
                                  int ntp_only);
static double get_mono_time(void);

#include <ntp_sources.c>

#undef NCR_ChangeRemoteAddress

static void
resolve_random_address(DNS_Status status, int rand_bits)
{
  IPAddr ip_addrs[DNS_MAX_ADDRESSES];
  int i, n_addrs;

  TEST_CHECK(requested_name);
  requested_name = NULL;

  if (status == DNS_Success) {
    n_addrs = random() % DNS_MAX_ADDRESSES + 1;
    for (i = 0; i < n_addrs; i++)
      TST_GetRandomAddress(&ip_addrs[i], IPADDR_UNSPEC, rand_bits);
  } else {
    n_addrs = 0;
  }

  (resolve_handler)(status, n_addrs, ip_addrs, resolve_handler_arg);
}

static int
update_random_address(NTP_Remote_Address *addr, int rand_bits)
{
  NTP_Remote_Address new_addr;
  NSR_Status status;

  TST_GetRandomAddress(&new_addr.ip_addr, IPADDR_UNSPEC, rand_bits);
  new_addr.port = random() % 1024;

  status = NSR_UpdateSourceNtpAddress(addr, &new_addr);
  if (status == NSR_InvalidAF) {
    TEST_CHECK(!UTI_IsIPReal(&addr->ip_addr));
  } else {
    TEST_CHECK(status == NSR_Success || status == NSR_AlreadyInUse);
  }

  TEST_CHECK(strlen(NSR_StatusToString(status)) > 0);

  return status == NSR_Success;
}

static void
change_remote_address(NCR_Instance inst, NTP_Remote_Address *remote_addr, int ntp_only)
{
  int update = !ntp_only && random() % 4 == 0, update_pos = random() % 2, r = 0;

  TEST_CHECK(record_lock);

  if (update && update_pos == 0)
    r = update_random_address(random() % 2 ? remote_addr : NCR_GetRemoteAddress(inst), 4);

  NCR_ChangeRemoteAddress(inst, remote_addr, ntp_only);

  if (update && update_pos == 1)
    r = update_random_address(random() % 2 ? remote_addr : NCR_GetRemoteAddress(inst), 4);

  if (r)
    TEST_CHECK(UTI_IsIPReal(&saved_address_update.old_address.ip_addr));
}

static double get_mono_time(void) {
  static double t = 0.0;

  if (random() % 2)
    t += TST_GetRandomDouble(0.0, 100.0);

  return t;
}

void
test_unit(void)
{
  char source_line[] = "127.0.0.1 offline", conf[] = "port 0", name[64];
  int i, j, k, family, slot, found, pool, prev_n;
  uint32_t hash = 0, conf_id;
  NTP_Remote_Address addrs[256], addr;
  NTP_Local_Address local_addr;
  NTP_Local_Timestamp local_ts;
  struct UnresolvedSource *us;
  RPT_ActivityReport report;
  CPS_NTP_Source source;
  NSR_Status status;
  NTP_Packet msg;

  CNF_Initialise(0, 0);
  CNF_ParseLine(NULL, 1, conf);

  PRV_Initialise();
  LCL_Initialise();
  TST_RegisterDummyDrivers();
  SCH_Initialise();
  SRC_Initialise();
  NIO_Initialise();
  NCR_Initialise();
  REF_Initialise();
  NSR_Initialise();

  CPS_ParseNTPSourceAdd(source_line, &source);

  TEST_CHECK(n_sources == 0);

  for (i = 0; i < 6; i++) {
    TEST_CHECK(ARR_GetSize(records) == 1);

    DEBUG_LOG("collision mod %u", 1U << i);

    for (j = 0; j < sizeof (addrs) / sizeof (addrs[0]); j++) {
      while (1) {
        do {
          TST_GetRandomAddress(&addrs[j].ip_addr, IPADDR_UNSPEC, -1);
        } while (UTI_IPToHash(&addrs[j].ip_addr) % (1U << i) != hash % (1U << i));

        for (k = 0; k < j; k++)
          if (UTI_CompareIPs(&addrs[k].ip_addr, &addrs[j].ip_addr, NULL) == 0)
            break;
        if (k == j)
          break;
      }

      addrs[j].port = random() % 1024;

      if (!j)
        hash = UTI_IPToHash(&addrs[j].ip_addr);

      DEBUG_LOG("adding source %s hash %"PRIu32, UTI_IPToString(&addrs[j].ip_addr),
                UTI_IPToHash(&addrs[j].ip_addr) % (1U << i));

      status = NSR_AddSource(&addrs[j], random() % 2 ? NTP_SERVER : NTP_PEER,
                             &source.params, NULL);
      TEST_CHECK(status == NSR_Success);
      TEST_CHECK(n_sources == j + 1);

      TEST_CHECK(strcmp(NSR_GetName(&addrs[j].ip_addr), UTI_IPToString(&addrs[j].ip_addr)) == 0);

      for (k = 0; k <= j; k++) {
        addr = addrs[k];
        found = find_slot2(&addr, &slot);
        TEST_CHECK(found == 2);
        TEST_CHECK(!UTI_CompareIPs(&get_record(slot)->remote_addr->ip_addr,
                                   &addr.ip_addr, NULL));
        addr.port++;
        found = find_slot2(&addr, &slot);
        TEST_CHECK(found == 1);
        TEST_CHECK(!UTI_CompareIPs(&get_record(slot)->remote_addr->ip_addr,
                                   &addr.ip_addr, NULL));
      }

      status = NSR_AddSource(&addrs[j], NTP_SERVER, &source.params, &conf_id);
      TEST_CHECK(status == NSR_AlreadyInUse);
    }

    for (j = 0; j < sizeof (addrs) / sizeof (addrs[0]); j++) {
      DEBUG_LOG("removing source %s", UTI_IPToString(&addrs[j].ip_addr));
      NSR_RemoveSource(&addrs[j].ip_addr);

      for (k = 0; k < sizeof (addrs) / sizeof (addrs[0]); k++) {
        found = find_slot2(&addrs[k], &slot);
        TEST_CHECK(found == (k <= j ? 0 : 2));
      }
    }
  }

  TEST_CHECK(n_sources == 0);

  status = NSR_AddSourceByName("a b", IPADDR_UNSPEC, 0, 0, 0, &source.params, &conf_id);
  TEST_CHECK(status == NSR_InvalidName);

  local_addr.ip_addr.family = IPADDR_INET4;
  local_addr.ip_addr.addr.in4 = 0;
  local_addr.if_index = -1;
  local_addr.sock_fd = 0;
  memset(&local_ts, 0, sizeof (local_ts));

  for (i = 0; i < 500; i++) {
    for (j = 0; j < 20; j++) {
      snprintf(name, sizeof (name), "ntp%d.example.net", (int)(random() % 10));
      family = random() % 2 ? IPADDR_UNSPEC : random() % 2 ? IPADDR_INET4 : IPADDR_INET6;
      pool = random() % 2;
      prev_n = n_sources;

      DEBUG_LOG("%d/%d adding source %s pool=%d", i, j, name, pool);
      status = NSR_AddSourceByName(name, family, 0, pool,
                                   random() % 2 ? NTP_SERVER : NTP_PEER,
                                   &source.params, &conf_id);
      TEST_CHECK(status == NSR_UnresolvedName);

      TEST_CHECK(n_sources == prev_n + (pool ? source.params.max_sources * 2 : 1));
      TEST_CHECK(unresolved_sources);

      for (us = unresolved_sources; us->next; us = us->next)
        ;
      TEST_CHECK(strcmp(us->name, name) == 0);
      TEST_CHECK(us->family == family);
      if (pool) {
        TEST_CHECK(us->address.ip_addr.family == IPADDR_UNSPEC && us->pool_id >= 0);
      } else {
        TEST_CHECK(strcmp(NSR_GetName(&us->address.ip_addr), name) == 0);
        TEST_CHECK(find_slot2(&us->address, &slot) == 2);
        TEST_CHECK(get_record(slot)->family == family);
      }

      if (random() % 2) {
        if (!resolving_id || random() % 2) {
          NSR_ResolveSources();
        } else {
          SCH_RemoveTimeout(resolving_id);
          resolve_sources_timeout(NULL);
          TEST_CHECK(resolving_id == 0);
          TEST_CHECK(requested_name);
        }

        TEST_CHECK(!!unresolved_sources == (resolving_id != 0) || requested_name);
      }

      while (requested_name && random() % 2) {
        TEST_CHECK(resolving_source);
        TEST_CHECK(strcmp(requested_name, resolving_source->name) == 0);
        TEST_CHECK(!record_lock);

        switch (random() % 3) {
          case 0:
            resolve_random_address(DNS_Success, 4);
            break;
          case 1:
            resolve_random_address(DNS_TryAgain, 0);
            break;
          case 2:
            resolve_random_address(DNS_Failure, 0);
            break;
        }
      }

      while (random() % 8 > 0) {
        slot = random() % ARR_GetSize(records);
        if (!get_record(slot)->remote_addr)
          continue;

        switch (random() % 5) {
          case 0:
            msg.lvm = NTP_LVM(0, NTP_VERSION, random() % 2 ? MODE_CLIENT : MODE_SERVER);
            NSR_ProcessTx(get_record(slot)->remote_addr, &local_addr,
                          &local_ts, &msg, 0);
            break;
          case 1:
            msg.lvm = NTP_LVM(0, NTP_VERSION, random() % 2 ? MODE_CLIENT : MODE_SERVER);
            NSR_ProcessRx(get_record(slot)->remote_addr, &local_addr,
                          &local_ts, &msg, 0);
            break;
          case 2:
            NSR_HandleBadSource(&get_record(slot)->remote_addr->ip_addr);
            break;
          case 3:
            NSR_SetConnectivity(NULL, &get_record(slot)->remote_addr->ip_addr, SRC_OFFLINE);
            break;
          case 4:
            update_random_address(get_record(slot)->remote_addr, 4);
            TEST_CHECK(!UTI_IsIPReal(&saved_address_update.old_address.ip_addr));
            break;
        }

        TEST_CHECK(!record_lock);
      }

      NSR_GetActivityReport(&report);
      TEST_CHECK(report.online == 0);
      TEST_CHECK(report.offline >= 0);
      TEST_CHECK(report.burst_online == 0);
      TEST_CHECK(report.burst_offline == 0);
      TEST_CHECK(report.unresolved >= 0);

      if (random() % 4 == 0) {
        NSR_RemoveSourcesById(conf_id);
        TEST_CHECK(n_sources <= prev_n);
      } else if (random() % 8 == 0) {
        NSR_RefreshAddresses();
        TEST_CHECK(unresolved_sources);
      }
    }

    NSR_RemoveAllSources();
    TEST_CHECK(n_sources == 0);

    for (j = 0; j < ARR_GetSize(pools); j++) {
      TEST_CHECK(get_pool(j)->sources == 0);
      TEST_CHECK(get_pool(j)->unresolved_sources == 0);
      TEST_CHECK(get_pool(j)->confirmed_sources == 0);
      TEST_CHECK(get_pool(j)->max_sources == 0);
    }

    while (requested_name) {
      TEST_CHECK(resolving_source);
      resolve_random_address(random() % 2 ? DNS_Success : DNS_TryAgain, 4);
    }

    if (unresolved_sources && resolving_id == 0)
      NSR_ResolveSources();

    TEST_CHECK(!!unresolved_sources == (resolving_id != 0));

    if (resolving_id) {
      SCH_RemoveTimeout(resolving_id);
      resolve_sources_timeout(NULL);
    }

    TEST_CHECK(resolving_id == 0);
    TEST_CHECK(!requested_name);
    TEST_CHECK(!unresolved_sources);
  }

  NSR_Finalise();
  REF_Finalise();
  NCR_Finalise();
  NIO_Finalise();
  SRC_Finalise();
  SCH_Finalise();
  LCL_Finalise();
  PRV_Finalise();
  CNF_Finalise();
  HSH_Finalise();
}
