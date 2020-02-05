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

/* Test the system adjtime() function. Check the range of supported offset,
   support for readonly operation, and slew rate with different update
   intervals and offsets. */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static int
diff_tv(struct timeval *tv1, struct timeval *tv2)
{
  return 1000000 * (tv1->tv_sec - tv2->tv_sec) + (tv1->tv_usec - tv2->tv_usec);
}

static struct timeval
usec_to_tv(int usec)
{
  struct timeval tv;

  tv.tv_sec = usec / 1000000;
  tv.tv_usec = usec % 1000000;

  return tv;
}

static int
try_adjtime(struct timeval *new, struct timeval *old)
{
  int r;

  r = adjtime(new, old);
  if (r)
    printf("adjtime() failed : %s ", strerror(errno));
  return r;
}

static void
reset_adjtime(void)
{
  struct timeval tv;

  tv = usec_to_tv(0);
  try_adjtime(&tv, NULL);
}

static void
test_range(void)
{
  struct timeval tv;
  int i;

  printf("range:\n");

  for (i = 0; i < sizeof (time_t) * 8; i++) {
    tv.tv_usec = 0;
    tv.tv_sec = (1ULL << i) - 1;
    printf("%20lld s : ", (long long)tv.tv_sec);
    printf("%s\n", !try_adjtime(&tv, NULL) ? "ok" : "");
    tv.tv_sec = ~tv.tv_sec;
    printf("%20lld s : ", (long long)tv.tv_sec);
    printf("%s\n", !try_adjtime(&tv, NULL) ? "ok" : "");
  }
}

static void
test_readonly(void)
{
  struct timeval tv1, tv2;
  int i, r;

  printf("readonly:\n");

  for (i = 0; i <= 20; i++) {
    tv1 = usec_to_tv(1 << i);

    printf("%9d us : ", 1 << i);
    try_adjtime(&tv1, NULL);
    r = !try_adjtime(NULL, &tv2) && !diff_tv(&tv1, &tv2);
    printf("%s\n", r ? "ok" : "fail");
  }
}

static void
test_readwrite(void)
{
  struct timeval tv1, tv2, tv3;
  int i, r;

  printf("readwrite:\n");

  for (i = 0; i <= 20; i++) {
    tv1 = usec_to_tv(1 << i);
    tv3 = usec_to_tv(0);

    printf("%9d us : ", 1 << i);
    try_adjtime(&tv1, NULL);
    r = !try_adjtime(&tv3, &tv2) && !diff_tv(&tv1, &tv2);
    printf("%s\n", r ? "ok" : "fail");
  }
}

static void
xusleep(int usec)
{
  struct timeval tv;

  tv = usec_to_tv(usec);
  select(0, NULL, NULL, NULL, &tv);
}

static void
test_slew(void)
{
  struct timeval tv1, tv2, tv3;
  int i, j, k, diff, min, has_min;

  printf("slew:\n");

  for (i = 9; i <= 20; i++) {
    printf("%9d us : ", 1 << i);
    for (j = 4; j <= 20; j += 4) {
      for (min = has_min = 0, k = 4; k < 16; k += 2) {

        tv1 = usec_to_tv(1 << j);
        tv3 = usec_to_tv(0);

        xusleep(1 << i);
        reset_adjtime();

        xusleep(1 << i);
        if (try_adjtime(&tv1, NULL))
          continue;

        xusleep(1 << i);
        if (try_adjtime(&tv3, &tv2))
          continue;

        diff = diff_tv(&tv1, &tv2);
        if (!has_min || min > diff) {
          min = diff;
          has_min = 1;
        }
      }

      if (!has_min)
        continue;

      printf(" %5d (%d)", min, 1 << j);
      fflush(stdout);
    }
    printf("\n");
  }
}

int
main()
{
  test_range();
  test_readonly();
  test_readwrite();
  test_slew();

  reset_adjtime();

  return 0;
}
