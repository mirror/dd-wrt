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

#include <smooth.c>
#include "test.h"

void
test_unit(void)
{
  int i, j;
  struct timespec ts;
  double offset, freq, wander;
  char conf[] = "smoothtime 300 0.01";

  CNF_Initialise(0, 0);
  CNF_ParseLine(NULL, 1, conf);

  LCL_Initialise();
  SMT_Initialise();
  locked = 0;

  for (i = 0; i < 500; i++) {
    UTI_ZeroTimespec(&ts);
    SMT_Reset(&ts);

    DEBUG_LOG("iteration %d", i);

    offset = (random() % 1000000 - 500000) / 1.0e6;
    freq = (random() % 1000000 - 500000) / 1.0e9;
    update_smoothing(&ts, offset, freq);

    for (j = 0; j < 10000; j++) {
      update_smoothing(&ts, 0.0, 0.0);
      UTI_AddDoubleToTimespec(&ts, 16.0, &ts);
      get_smoothing(&ts, &offset, &freq, &wander);
    }

    TEST_CHECK(fabs(offset) < 1e-12);
    TEST_CHECK(fabs(freq) < 1e-12);
    TEST_CHECK(fabs(wander) < 1e-12);
  }

  SMT_Finalise();
  LCL_Finalise();
  CNF_Finalise();
}
