/*
 **********************************************************************
 * Copyright (C) Patrick Oppenlander 2024
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

#include <leapdb.c>
#include "test.h"

struct test_vector {
  time_t when;
  int tai_offset;
  NTP_Leap leap;
  int fake;
} tests[] = {
  /* leapdb.list is a cut down version of leap-seconds.list */
  {3439756800, 34, LEAP_InsertSecond, 0}, /* 1 Jan 2009 */
  {3550089600, 35, LEAP_InsertSecond, 0}, /* 1 Jul 2012 */
  {3644697600, 36, LEAP_InsertSecond, 0}, /* 1 Jul 2015 */
  {3692217600, 37, LEAP_InsertSecond, 0}, /* 1 Jan 2017 */
  {3786825600, 36, LEAP_DeleteSecond, 1}, /* 1 Jan 2020 fake in leapdb.list */
};

static void
test_leap_source(NTP_Leap (*fn)(time_t when, int *tai_offset),
                 int skip_fakes)
{
  int i, prev_tai_offset = 34;

  for (i = 0; i < sizeof tests / sizeof tests[0]; ++i) {
    struct test_vector *t = tests + i;

    NTP_Leap leap;
    int tai_offset = -1;

    /* Our unit test leapdb.list contains a fake entry removing a leap second.
     * Skip this when testing with the right/UTC timezone using mktime(). */
    if (skip_fakes && t->fake)
      continue;

    /* One second before leap second */
    leap = fn(t->when - LEAP_SEC_LIST_OFFSET - 1, &tai_offset);
    TEST_CHECK(leap == t->leap);
    TEST_CHECK(tai_offset = prev_tai_offset);

    /* Exactly on leap second */
    leap = fn(t->when - LEAP_SEC_LIST_OFFSET, &tai_offset);
    TEST_CHECK(leap == LEAP_Normal);
    TEST_CHECK(tai_offset == t->tai_offset);

    /* One second after leap second */
    leap = fn(t->when - LEAP_SEC_LIST_OFFSET + 1, &tai_offset);
    TEST_CHECK(leap == LEAP_Normal);
    TEST_CHECK(tai_offset == t->tai_offset);

    prev_tai_offset = t->tai_offset;
  }
}

void
test_unit(void)
{
  char conf[][100] = {
    "leapsectz right/UTC",
    "leapseclist leapdb.list"
  };
  int i;

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);
  LDB_Initialise();

  if (check_leap_source(get_tz_leap)) {
    DEBUG_LOG("testing get_tz_leap");
    test_leap_source(get_tz_leap, 1);
  } else {
    DEBUG_LOG("Skipping get_tz_leap test. Either the right/UTC timezone is "
	      "missing, or mktime() doesn't support leap seconds.");
  }

  DEBUG_LOG("testing get_list_leap");
  TEST_CHECK(check_leap_source(get_list_leap));
  test_leap_source(get_list_leap, 0);

  /* This exercises the twice-per-day logic */
  DEBUG_LOG("testing LDB_GetLeap");
  test_leap_source(LDB_GetLeap, 1);

  LDB_Finalise();
  CNF_Finalise();
}
