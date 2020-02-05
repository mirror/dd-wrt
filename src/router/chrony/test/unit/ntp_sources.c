/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016
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

#ifdef FEAT_NTP

#include <ntp_sources.c>
#include <conf.h>
#include <ntp_io.h>

void
test_unit(void)
{
  int i, j, k, slot, found;
  uint32_t hash = 0;
  NTP_Remote_Address addrs[256], addr;
  SourceParameters params;
  char conf[] = "port 0";

  memset(&params, 0, sizeof (params));

  CNF_Initialise(0, 0);
  CNF_ParseLine(NULL, 1, conf);

  LCL_Initialise();
  SCH_Initialise();
  SRC_Initialise();
  NIO_Initialise(IPADDR_UNSPEC);
  NCR_Initialise();
  NSR_Initialise();

  for (i = 0; i < 6; i++) {
    TEST_CHECK(ARR_GetSize(records) == 1);

    DEBUG_LOG("collision mod %u", 1U << i);

    for (j = 0; j < sizeof (addrs) / sizeof (addrs[0]); j++) {
      do {
        TST_GetRandomAddress(&addrs[j].ip_addr, IPADDR_UNSPEC, -1);
      } while (UTI_IPToHash(&addrs[j].ip_addr) % (1U << i) != hash % (1U << i));

      addrs[j].port = random() % 1024;

      if (!j)
        hash = UTI_IPToHash(&addrs[j].ip_addr);

      DEBUG_LOG("adding source %s hash %"PRIu32, UTI_IPToString(&addrs[j].ip_addr),
                UTI_IPToHash(&addrs[j].ip_addr) % (1U << i));

      NSR_AddSource(&addrs[j], random() % 2 ? NTP_SERVER : NTP_PEER, &params);

      for (k = 0; k < j; k++) {
        addr = addrs[k];
        find_slot(&addr, &slot, &found);
        TEST_CHECK(found == 2);
        TEST_CHECK(!UTI_CompareIPs(&get_record(slot)->remote_addr->ip_addr,
                                   &addr.ip_addr, NULL));
        addr.port++;
        find_slot(&addr, &slot, &found);
        TEST_CHECK(found == 1);
        TEST_CHECK(!UTI_CompareIPs(&get_record(slot)->remote_addr->ip_addr,
                                   &addr.ip_addr, NULL));
      }
    }

    for (j = 0; j < sizeof (addrs) / sizeof (addrs[0]); j++) {
      DEBUG_LOG("removing source %s", UTI_IPToString(&addrs[j].ip_addr));
      NSR_RemoveSource(&addrs[j]);

      for (k = 0; k < sizeof (addrs) / sizeof (addrs[0]); k++) {
        find_slot(&addrs[k], &slot, &found);
        TEST_CHECK(found == (k <= j ? 0 : 2));
      }
    }
  }

  NSR_Finalise();
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
