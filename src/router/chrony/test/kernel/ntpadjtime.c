/*
 * Copyright (C) Miroslav Lichvar  2015
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
 */

/* Check the frequency range of the system ntp_adjtime() implementation */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timex.h>

static int
try_ntpadjtime(struct timex *t)
{
  int r;
  r = ntp_adjtime(t);
  if (r < 0)
    printf("ntp_adjtime() failed : %s ", strerror(errno));
  return r;
}

static void
reset_ntpadjtime(void)
{
  struct timex t;

  t.modes = MOD_OFFSET | MOD_FREQUENCY;
  t.offset = 0;
  t.freq = 0;
  try_ntpadjtime(&t);
}

static void
test_freqrange(void)
{
  struct timex t;
  int i;

  printf("freq range:\n");

  for (i = 0; i <= 1000; i += 50) {
    t.modes = MOD_FREQUENCY;
    t.freq = i << 16;
    printf("%4d ppm => ", i);
    if (try_ntpadjtime(&t) < 0)
      continue;

    printf("%4ld ppm : ", t.freq / (1 << 16));
    printf("%s\n", t.freq == i << 16 ? "ok" : "fail");
  }
}

int
main()
{
  test_freqrange();

  reset_ntpadjtime();

  return 0;
}
