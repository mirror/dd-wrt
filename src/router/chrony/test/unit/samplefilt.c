/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2018
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

#include <local.h>
#include "test.h"

#define LCL_GetSysPrecisionAsQuantum() (1.0e-6)

#include <samplefilt.c>

void
test_unit(void)
{
  NTP_Sample sample_in, sample_out;
  SPF_Instance filter;
  int i, j, k, sum_count, min_samples, max_samples;
  double mean, combine_ratio, sum_err;

  LCL_Initialise();

  for (i = 0; i <= 100; i++) {
    max_samples = random() % 20 + 1;
    min_samples = random() % (max_samples) + 1;
    combine_ratio = TST_GetRandomDouble(0.0, 1.0);

    filter = SPF_CreateInstance(min_samples, max_samples, 2.0, combine_ratio);

    for (j = 0, sum_count = 0, sum_err = 0.0; j < 100; j++) {
      DEBUG_LOG("iteration %d/%d", i, j);

      mean = TST_GetRandomDouble(-1.0e3, 1.0e3);
      UTI_ZeroTimespec(&sample_in.time);

      for (k = 0; k < 100; k++) {
        UTI_AddDoubleToTimespec(&sample_in.time, TST_GetRandomDouble(1.0e-1, 1.0e2),
                                &sample_in.time);
        sample_in.offset = mean + TST_GetRandomDouble(-1.0, 1.0);
        sample_in.peer_dispersion = TST_GetRandomDouble(1.0e-4, 2.0e-4);
        sample_in.root_dispersion = TST_GetRandomDouble(1.0e-3, 2.0e-3);
        sample_in.peer_delay = TST_GetRandomDouble(1.0e-2, 2.0e-2);
        sample_in.root_delay = TST_GetRandomDouble(1.0e-1, 2.0e-1);
        sample_in.stratum = random() % 16;
        sample_in.leap = random() % 4;

        TEST_CHECK(SPF_AccumulateSample(filter, &sample_in));
        TEST_CHECK(!SPF_AccumulateSample(filter, &sample_in));

        TEST_CHECK(SPF_GetNumberOfSamples(filter) == MIN(k + 1, max_samples));

        SPF_GetLastSample(filter, &sample_out);
        TEST_CHECK(!memcmp(&sample_in, &sample_out, sizeof (sample_in)));

        SPF_SlewSamples(filter, &sample_in.time, 0.0, 0.0);
        SPF_AddDispersion(filter, 0.0);

        if (k + 1 < min_samples)
          TEST_CHECK(!SPF_GetFilteredSample(filter, &sample_out));

        TEST_CHECK(SPF_GetNumberOfSamples(filter) == MIN(k + 1, max_samples));
      }

      if (random() % 10) {
        TEST_CHECK(SPF_GetFilteredSample(filter, &sample_out));

        TEST_CHECK(SPF_GetAvgSampleDispersion(filter) <= 2.0);

        sum_err += sample_out.offset - mean;
        sum_count++;

        TEST_CHECK(UTI_CompareTimespecs(&sample_out.time, &sample_in.time) <= 0 &&
                   sample_out.time.tv_sec >= 0);
        TEST_CHECK(fabs(sample_out.offset - mean) <= 1.0);
        TEST_CHECK(sample_out.peer_dispersion >= 1.0e-4 &&
                   (sample_out.peer_dispersion <= 2.0e-4 || filter->max_samples > 1));
        TEST_CHECK(sample_out.root_dispersion >= 1.0e-3 &&
                   (sample_out.root_dispersion <= 2.0e-3 || filter->max_samples > 1));
        TEST_CHECK(sample_out.peer_delay >= 1.0e-2 &&
                   sample_out.peer_delay <= 2.0e-2);
        TEST_CHECK(sample_out.root_delay >= 1.0e-1 &&
                   sample_out.root_delay <= 2.0e-1);
        TEST_CHECK(sample_out.leap >= 0 && sample_out.leap <= 3);
        TEST_CHECK(sample_out.stratum >= 0 && sample_out.stratum <= 15);

        if (max_samples == 1)
          TEST_CHECK(!memcmp(&sample_in, &sample_out, sizeof (sample_in)));

      } else {
        SPF_DropSamples(filter);
      }

      TEST_CHECK(SPF_GetNumberOfSamples(filter) == 0);
    }

    TEST_CHECK(fabs(sum_err / sum_count) < 0.3);

    SPF_DestroyInstance(filter);
  }

  LCL_Finalise();
}
