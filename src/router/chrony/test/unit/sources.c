/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016, 2018
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

#include <sources.c>
#include "test.h"

void
test_unit(void)
{
  SRC_Instance srcs[16];
  RPT_SourceReport report;
  NTP_Sample sample;
  IPAddr addr;
  int i, j, k, l, samples, sel_options;

  CNF_Initialise(0, 0);
  LCL_Initialise();
  TST_RegisterDummyDrivers();
  SCH_Initialise();
  SRC_Initialise();
  REF_Initialise();

  REF_SetMode(REF_ModeIgnore);

  for (i = 0; i < 1000; i++) {
    DEBUG_LOG("iteration %d", i);

    for (j = 0; j < sizeof (srcs) / sizeof (srcs[0]); j++) {
      TEST_CHECK(n_sources == j);

      TST_GetRandomAddress(&addr, IPADDR_UNSPEC, -1);

      sel_options = i & random() & (SRC_SELECT_NOSELECT | SRC_SELECT_PREFER |
                                    SRC_SELECT_TRUST | SRC_SELECT_REQUIRE);

      DEBUG_LOG("added source %d options %d", j, sel_options);
      srcs[j] = SRC_CreateNewInstance(UTI_IPToRefid(&addr), SRC_NTP, sel_options, &addr,
                                      SRC_DEFAULT_MINSAMPLES, SRC_DEFAULT_MAXSAMPLES,
                                      0.0, 1.0);
      SRC_UpdateReachability(srcs[j], 1);

      samples = (i + j) % 5 + 3;

      sample.offset = TST_GetRandomDouble(-1.0, 1.0);

      for (k = 0; k < samples; k++) {
        SCH_GetLastEventTime(&sample.time, NULL, NULL);
        UTI_AddDoubleToTimespec(&sample.time, TST_GetRandomDouble(k - samples, k - samples + 1),
                                &sample.time);

        sample.offset += TST_GetRandomDouble(-1.0e-2, 1.0e-2);
        sample.peer_delay = TST_GetRandomDouble(1.0e-6, 1.0e-1);
        sample.peer_dispersion = TST_GetRandomDouble(1.0e-6, 1.0e-1);
        sample.root_delay = sample.peer_delay;
        sample.root_dispersion = sample.peer_dispersion;
        sample.stratum = 1;
        sample.leap = LEAP_Normal;

        DEBUG_LOG("source %d sample %d offset %f delay %f disp %f", j, k,
                  sample.offset, sample.peer_delay, sample.peer_dispersion);

        SRC_AccumulateSample(srcs[j], &sample);
      }

      for (k = 0; k <= j; k++) {
        int passed = 0, trusted = 0, trusted_passed = 0, required = 0, required_passed = 0;
        double trusted_lo = DBL_MAX, trusted_hi = DBL_MIN;
        double passed_lo = DBL_MAX, passed_hi = DBL_MIN;

        SRC_SelectSource(srcs[k]);
        DEBUG_LOG("source %d status %u", k, sources[k]->status);

        for (l = 0; l <= j; l++) {
          TEST_CHECK(sources[l]->status > SRC_OK && sources[l]->status <= SRC_SELECTED);
          if (sources[l]->sel_options & SRC_SELECT_NOSELECT) {
            TEST_CHECK(sources[l]->status == SRC_UNSELECTABLE);
          } else if (sources[l]->status != SRC_BAD_DISTANCE) {
            if (sources[l]->status >= SRC_NONPREFERRED) {
              passed++;
              if (passed_lo > sources[l]->sel_info.lo_limit)
                passed_lo = sources[l]->sel_info.lo_limit;
              if (passed_hi < sources[l]->sel_info.hi_limit)
                passed_hi = sources[l]->sel_info.hi_limit;
            }
            if (sources[l]->sel_options & SRC_SELECT_TRUST) {
              trusted++;
              if (trusted_lo > sources[l]->sel_info.lo_limit)
                trusted_lo = sources[l]->sel_info.lo_limit;
              if (trusted_hi < sources[l]->sel_info.hi_limit)
                trusted_hi = sources[l]->sel_info.hi_limit;
              if (sources[l]->status >= SRC_NONPREFERRED)
                trusted_passed++;
            }
            if (sources[l]->sel_options & SRC_SELECT_REQUIRE) {
              required++;
              if (sources[l]->status >= SRC_NONPREFERRED)
                required_passed++;
            }
            if (sources[l]->sel_options & SRC_SELECT_PREFER)
              TEST_CHECK(sources[l]->status != SRC_NONPREFERRED);
          }
        }

        DEBUG_LOG("sources %d passed %d trusted %d/%d required %d/%d", j, passed,
                  trusted_passed, trusted, required_passed, required);

        TEST_CHECK(!trusted || !passed || (passed_lo >= trusted_lo && passed_hi <= trusted_hi));
        TEST_CHECK(!passed || trusted != 1 || (trusted == 1 && trusted_passed == 1));
        TEST_CHECK(!passed || !required || required_passed > 0);
      }
    }

    for (j = 0; j < sizeof (srcs) / sizeof (srcs[0]); j++) {
      SRC_ReportSource(j, &report, &sample.time);
      SRC_DestroyInstance(srcs[j]);
    }
  }

  REF_Finalise();
  SRC_Finalise();
  SCH_Finalise();
  LCL_Finalise();
  CNF_Finalise();
  HSH_Finalise();
}
