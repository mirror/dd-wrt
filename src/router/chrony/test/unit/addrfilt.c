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

#include <addrfilt.c>
#include <logging.h>
#include <util.h>
#include "test.h"

void
test_unit(void)
{
  int i, j, sub, maxsub;
  IPAddr ip;
  ADF_AuthTable table;

  table = ADF_CreateTable();

  for (i = 0; i < 100; i++) {
    for (j = 0; j < 1000; j++) {
      if (j % 2) {
        maxsub = 32;
        TST_GetRandomAddress(&ip, IPADDR_INET4, -1);
      } else {
        maxsub = 128;
        TST_GetRandomAddress(&ip, IPADDR_INET6, -1);
      }

      DEBUG_LOG("address %s", UTI_IPToString(&ip));

      sub = random() % (maxsub + 1);

      TEST_CHECK(!ADF_IsAllowed(table, &ip));
      ADF_Allow(table, &ip, sub);
      TEST_CHECK(ADF_IsAllowed(table, &ip));

      if (sub < maxsub) {
        TST_SwapAddressBit(&ip, sub);
        TEST_CHECK(ADF_IsAllowed(table, &ip));
      }

      if (sub > 0) {
        TST_SwapAddressBit(&ip, sub - 1);
        TEST_CHECK(!ADF_IsAllowed(table, &ip));
        if (sub % 4 != 1) {
          ADF_Deny(table, &ip, sub - 1);
          TST_SwapAddressBit(&ip, sub - 1);
          TEST_CHECK(!ADF_IsAllowed(table, &ip));
        }
      }

      if (sub > 4) {
        ADF_AllowAll(table, &ip, sub - 4);
        TEST_CHECK(ADF_IsAllowed(table, &ip));
      }

      ADF_DenyAll(table, &ip, 0);
    }

    ip.family = IPADDR_INET4;
    ADF_DenyAll(table, &ip, 0);
    ip.family = IPADDR_INET6;
    ADF_DenyAll(table, &ip, 0);
  }

  ADF_DestroyTable(table);
}
