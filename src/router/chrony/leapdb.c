/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2009-2018, 2020, 2022
 * Copyright (C) Patrick Oppenlander 2023, 2024
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

  =======================================================================

  This module provides leap second information. */

#include "config.h"

#include "sysincl.h"

#include "conf.h"
#include "leapdb.h"
#include "logging.h"
#include "util.h"

/* ================================================== */

/* Source of leap second data */
enum {
  SRC_NONE,
  SRC_TIMEZONE,
  SRC_LIST,
} leap_src;

/* Offset between leap-seconds.list timestamp epoch and Unix epoch.
   leap-seconds.list epoch is 1 Jan 1900, 00:00:00 */
#define LEAP_SEC_LIST_OFFSET 2208988800

/* ================================================== */

static NTP_Leap
get_tz_leap(time_t when, int *tai_offset)
{
  struct tm stm, *tm;
  time_t t;
  char *tz_env, tz_orig[128];
  NTP_Leap tz_leap = LEAP_Normal;

  tm = gmtime(&when);
  if (!tm)
    return tz_leap;

  stm = *tm;

  /* Temporarily switch to the timezone containing leap seconds */
  tz_env = getenv("TZ");
  if (tz_env) {
    if (strlen(tz_env) >= sizeof (tz_orig))
      return tz_leap;
    strcpy(tz_orig, tz_env);
  }
  setenv("TZ", CNF_GetLeapSecTimezone(), 1);
  tzset();

  /* Get the TAI-UTC offset, which started at the epoch at 10 seconds */
  t = mktime(&stm);
  if (t != -1)
    *tai_offset = t - when + 10;

  /* Set the time to 23:59:60 and see how it overflows in mktime() */
  stm.tm_sec = 60;
  stm.tm_min = 59;
  stm.tm_hour = 23;

  t = mktime(&stm);

  if (tz_env)
    setenv("TZ", tz_orig, 1);
  else
    unsetenv("TZ");
  tzset();

  if (t == -1)
    return tz_leap;

  if (stm.tm_sec == 60)
    tz_leap = LEAP_InsertSecond;
  else if (stm.tm_sec == 1)
    tz_leap = LEAP_DeleteSecond;

  return tz_leap;
}

/* ================================================== */

static NTP_Leap
get_list_leap(time_t when, int *tai_offset)
{
  FILE *f;
  char line[1024];
  NTP_Leap ret_leap = LEAP_Normal;
  int ret_tai_offset = 0, prev_lsl_tai_offset = 10;
  int64_t when1900, lsl_updated = 0, lsl_expiry = 0;
  const char *leap_sec_list = CNF_GetLeapSecList();

  if (!(f = UTI_OpenFile(NULL, leap_sec_list, NULL, 'r', 0))) {
    LOG(LOGS_ERR, "Failed to open leap seconds list %s", leap_sec_list);
    goto out;
  }

  /* Leap second happens at midnight */
  when = (when / (24 * 3600) + 1) * (24 * 3600);

  /* leap-seconds.list timestamps are relative to 1 Jan 1900, 00:00:00 */
  when1900 = (int64_t)when + LEAP_SEC_LIST_OFFSET;

  while (fgets(line, sizeof line, f) > 0) {
    int64_t lsl_when;
    int lsl_tai_offset;
    char *p;

    /* Ignore blank lines */
    for (p = line; *p && isspace(*p); ++p)
      ;
    if (!*p)
      continue;

    if (*line == '#') {
      /* Update time line starts with #$ */
      if (line[1] == '$' && sscanf(line + 2, "%"SCNd64, &lsl_updated) != 1)
        goto error;
      /* Expiration time line starts with #@ */
      if (line[1] == '@' && sscanf(line + 2, "%"SCNd64, &lsl_expiry) != 1)
        goto error;
      /* Comment or a special comment we don't care about */
      continue;
    }

    /* Leap entry */
    if (sscanf(line, "%"SCNd64" %d", &lsl_when, &lsl_tai_offset) != 2)
      goto error;

    if (when1900 == lsl_when) {
      if (lsl_tai_offset > prev_lsl_tai_offset)
        ret_leap = LEAP_InsertSecond;
      else if (lsl_tai_offset < prev_lsl_tai_offset)
        ret_leap = LEAP_DeleteSecond;
      /* When is rounded to the end of the day, so offset hasn't changed yet! */
      ret_tai_offset = prev_lsl_tai_offset;
    } else if (when1900 > lsl_when) {
      ret_tai_offset = lsl_tai_offset;
    }

    prev_lsl_tai_offset = lsl_tai_offset;
  }

  /* Make sure the file looks sensible */
  if (!feof(f) || !lsl_updated || !lsl_expiry)
    goto error;

  if (when1900 >= lsl_expiry)
    LOG(LOGS_WARN, "Leap second list %s needs update", leap_sec_list);

  goto out;

error:
  if (f)
    fclose(f);
  LOG(LOGS_ERR, "Failed to parse leap seconds list %s", leap_sec_list);
  return LEAP_Normal;

out:
  if (f)
    fclose(f);
  *tai_offset = ret_tai_offset;
  return ret_leap;
}

/* ================================================== */

static int
check_leap_source(NTP_Leap (*src)(time_t when, int *tai_offset))
{
  int tai_offset = 0;

  /* Check that the leap second source has good data for Jun 30 2012 and Dec 31 2012 */
  if (src(1341014400, &tai_offset) == LEAP_InsertSecond && tai_offset == 34 &&
      src(1356912000, &tai_offset) == LEAP_Normal && tai_offset == 35)
    return 1;

  return 0;
}

/* ================================================== */

void
LDB_Initialise(void)
{
  const char *leap_tzname, *leap_sec_list;

  leap_tzname = CNF_GetLeapSecTimezone();
  if (leap_tzname && !check_leap_source(get_tz_leap)) {
    LOG(LOGS_WARN, "Timezone %s failed leap second check, ignoring", leap_tzname);
    leap_tzname = NULL;
  }

  leap_sec_list = CNF_GetLeapSecList();
  if (leap_sec_list && !check_leap_source(get_list_leap)) {
    LOG(LOGS_WARN, "Leap second list %s failed check, ignoring", leap_sec_list);
    leap_sec_list = NULL;
  }

  if (leap_sec_list) {
    LOG(LOGS_INFO, "Using leap second list %s", leap_sec_list);
    leap_src = SRC_LIST;
  } else if (leap_tzname) {
    LOG(LOGS_INFO, "Using %s timezone to obtain leap second data", leap_tzname);
    leap_src = SRC_TIMEZONE;
  }
}

/* ================================================== */

NTP_Leap
LDB_GetLeap(time_t when, int *tai_offset)
{
  static time_t last_ldb_leap_check;
  static NTP_Leap ldb_leap;
  static int ldb_tai_offset;

  /* Do this check at most twice a day */
  when = when / (12 * 3600) * (12 * 3600);
  if (last_ldb_leap_check == when)
    goto out;

  last_ldb_leap_check = when;
  ldb_leap = LEAP_Normal;
  ldb_tai_offset = 0;

  switch (leap_src) {
  case SRC_NONE:
    break;
  case SRC_TIMEZONE:
    ldb_leap = get_tz_leap(when, &ldb_tai_offset);
    break;
  case SRC_LIST:
    ldb_leap = get_list_leap(when, &ldb_tai_offset);
    break;
  }

out:
  *tai_offset = ldb_tai_offset;
  return ldb_leap;
}

/* ================================================== */

void
LDB_Finalise(void)
{
  /* Nothing to do */
}
