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

#include <clientlog.c>
#include "test.h"

void
test_unit(void)
{
  int i, j, index;
  struct timespec ts;
  IPAddr ip;
  char conf[][100] = {
    "clientloglimit 10000",
    "ratelimit interval 3 burst 4 leak 3",
    "cmdratelimit interval 3 burst 4 leak 3",
  };

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);

  CLG_Initialise();

  TEST_CHECK(ARR_GetSize(records) == 16);

  for (i = 0; i < 500; i++) {
    DEBUG_LOG("iteration %d", i);

    ts.tv_sec = (time_t)random() & 0x0fffffff;
    ts.tv_nsec = 0;

    for (j = 0; j < 1000; j++) {
      TST_GetRandomAddress(&ip, IPADDR_UNSPEC, i % 8 ? -1 : i / 8 % 9);
      DEBUG_LOG("address %s", UTI_IPToString(&ip));

      if (random() % 2) {
        index = CLG_LogNTPAccess(&ip, &ts);
        TEST_CHECK(index >= 0);
        CLG_LimitNTPResponseRate(index);
      } else {
        index = CLG_LogCommandAccess(&ip, &ts);
        TEST_CHECK(index >= 0);
        CLG_LimitCommandResponseRate(index);
      }

      UTI_AddDoubleToTimespec(&ts, (1 << random() % 14) / 100.0, &ts);
    }
  }

  DEBUG_LOG("records %u", ARR_GetSize(records));
  TEST_CHECK(ARR_GetSize(records) == 64);

  for (i = j = 0; i < 10000; i++) {
    ts.tv_sec += 1;
    index = CLG_LogNTPAccess(&ip, &ts);
    TEST_CHECK(index >= 0);
    if (!CLG_LimitNTPResponseRate(index))
      j++;
  }

  DEBUG_LOG("requests %d responses %d", i, j);
  TEST_CHECK(j * 4 < i && j * 6 > i);

  CLG_Finalise();
  CNF_Finalise();
}
