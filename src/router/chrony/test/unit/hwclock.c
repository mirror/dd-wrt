/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016-2018, 2022
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

#if defined(FEAT_PHC) || defined(HAVE_LINUX_TIMESTAMPING)

#include <hwclock.c>

#define MAX_READINGS 20

void
test_unit(void)
{
  struct timespec start_hw_ts, start_local_ts, hw_ts, local_ts, ts;
  struct timespec readings[MAX_READINGS][3];
  HCL_Instance clock;
  double freq, jitter, interval, dj, err, sum;
  int i, j, k, l, new_sample, n_readings, count;

  LCL_Initialise();
  TST_RegisterDummyDrivers();

  for (i = 1; i <= 8; i++) {
    clock = HCL_CreateInstance(random() % (1 << i), 1 << i, 1.0, 1e-9);

    for (j = 0, count = 0, sum = 0.0; j < 100; j++) {
      UTI_ZeroTimespec(&start_hw_ts);
      UTI_ZeroTimespec(&start_local_ts);
      UTI_AddDoubleToTimespec(&start_hw_ts, TST_GetRandomDouble(0.0, 1e9), &start_hw_ts);
      UTI_AddDoubleToTimespec(&start_local_ts, TST_GetRandomDouble(0.0, 1e9), &start_local_ts);

      DEBUG_LOG("iteration %d", j);

      freq = TST_GetRandomDouble(0.9, 1.1);
      jitter = TST_GetRandomDouble(10.0e-9, 1000.0e-9);
      interval = TST_GetRandomDouble(0.1, 10.0);

      clock->n_samples = 0;
      clock->valid_coefs = 0;
      QNT_Reset(clock->delay_quants);

      new_sample = 0;

      for (k = 0; k < 100; k++) {
        UTI_AddDoubleToTimespec(&start_hw_ts, k * interval * freq, &hw_ts);
        UTI_AddDoubleToTimespec(&start_local_ts, k * interval, &local_ts);
        if (HCL_CookTime(clock, &hw_ts, &ts, NULL) && new_sample) {
          dj = fabs(UTI_DiffTimespecsToDouble(&ts, &local_ts) / jitter);
          DEBUG_LOG("delta/jitter %f", dj);
          if (clock->n_samples >= clock->max_samples / 2)
            sum += dj, count++;
          TEST_CHECK(clock->n_samples < 4 || dj <= 4.0);
          TEST_CHECK(clock->n_samples < 8 || dj <= 3.0);
        }

        UTI_AddDoubleToTimespec(&start_hw_ts, k * interval * freq + TST_GetRandomDouble(-jitter, jitter), &hw_ts);

        if (HCL_NeedsNewSample(clock, &local_ts)) {
          n_readings = random() % MAX_READINGS + 1;
          for (l = 0; l < n_readings; l++) {
            UTI_AddDoubleToTimespec(&local_ts, -TST_GetRandomDouble(0.0, jitter / 10.0), &readings[l][0]);
            readings[l][1] = hw_ts;
            UTI_AddDoubleToTimespec(&local_ts, TST_GetRandomDouble(0.0, jitter / 10.0), &readings[l][2]);
          }

          UTI_ZeroTimespec(&hw_ts);
          UTI_ZeroTimespec(&local_ts);
          if (HCL_ProcessReadings(clock, n_readings, readings, &hw_ts, &local_ts, &err)) {
            HCL_AccumulateSample(clock, &hw_ts, &local_ts, 2.0 * jitter);
            new_sample = 1;
          } else {
            new_sample = 0;
          }
        }

        TEST_CHECK(clock->valid_coefs == (clock->n_samples >= 2));

        if (!clock->valid_coefs)
          continue;

        TEST_CHECK(fabs(clock->offset) <= 2.0 * jitter);
      }
    }

    TEST_CHECK(sum / count < 2.4 / sqrt(clock->max_samples));

    HCL_DestroyInstance(clock);
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
