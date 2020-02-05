/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2013-2014, 2016
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
  
  Module for parsing various forms of directive and command lines that
  are common to the configuration file and to the command client.

  */

#include "config.h"

#include "sysincl.h"

#include "cmdparse.h"
#include "memory.h"
#include "nameserv.h"
#include "ntp.h"
#include "util.h"

/* ================================================== */

int
CPS_ParseNTPSourceAdd(char *line, CPS_NTP_Source *src)
{
  char *hostname, *cmd;
  int n;
  
  src->port = SRC_DEFAULT_PORT;
  src->params.minpoll = SRC_DEFAULT_MINPOLL;
  src->params.maxpoll = SRC_DEFAULT_MAXPOLL;
  src->params.connectivity = SRC_ONLINE;
  src->params.auto_offline = 0;
  src->params.presend_minpoll = SRC_DEFAULT_PRESEND_MINPOLL;
  src->params.burst = 0;
  src->params.iburst = 0;
  src->params.min_stratum = SRC_DEFAULT_MINSTRATUM;
  src->params.poll_target = SRC_DEFAULT_POLLTARGET;
  src->params.version = 0;
  src->params.max_sources = SRC_DEFAULT_MAXSOURCES;
  src->params.min_samples = SRC_DEFAULT_MINSAMPLES;
  src->params.max_samples = SRC_DEFAULT_MAXSAMPLES;
  src->params.filter_length = 0;
  src->params.interleaved = 0;
  src->params.sel_options = 0;
  src->params.authkey = INACTIVE_AUTHKEY;
  src->params.max_delay = SRC_DEFAULT_MAXDELAY;
  src->params.max_delay_ratio = SRC_DEFAULT_MAXDELAYRATIO;
  src->params.max_delay_dev_ratio = SRC_DEFAULT_MAXDELAYDEVRATIO;
  src->params.min_delay = 0.0;
  src->params.asymmetry = SRC_DEFAULT_ASYMMETRY;
  src->params.offset = 0.0;

  hostname = line;
  line = CPS_SplitWord(line);

  if (!*hostname)
    return 0;

  src->name = hostname;

  /* Parse options */
  for (; *line; line += n) {
    cmd = line;
    line = CPS_SplitWord(line);
    n = 0;

    if (!strcasecmp(cmd, "auto_offline")) {
      src->params.auto_offline = 1;
    } else if (!strcasecmp(cmd, "burst")) {
      src->params.burst = 1;
    } else if (!strcasecmp(cmd, "iburst")) {
      src->params.iburst = 1;
    } else if (!strcasecmp(cmd, "offline")) {
      src->params.connectivity = SRC_OFFLINE;
    } else if (!strcasecmp(cmd, "noselect")) {
      src->params.sel_options |= SRC_SELECT_NOSELECT;
    } else if (!strcasecmp(cmd, "prefer")) {
      src->params.sel_options |= SRC_SELECT_PREFER;
    } else if (!strcasecmp(cmd, "require")) {
      src->params.sel_options |= SRC_SELECT_REQUIRE;
    } else if (!strcasecmp(cmd, "trust")) {
      src->params.sel_options |= SRC_SELECT_TRUST;
    } else if (!strcasecmp(cmd, "key")) {
      if (sscanf(line, "%"SCNu32"%n", &src->params.authkey, &n) != 1 ||
          src->params.authkey == INACTIVE_AUTHKEY)
        return 0;
    } else if (!strcasecmp(cmd, "asymmetry")) {
      if (sscanf(line, "%lf%n", &src->params.asymmetry, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "filter")) {
      if (sscanf(line, "%d%n", &src->params.filter_length, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "maxdelay")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "maxdelayratio")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay_ratio, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "maxdelaydevratio")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay_dev_ratio, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "maxpoll")) {
      if (sscanf(line, "%d%n", &src->params.maxpoll, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "maxsamples")) {
      if (sscanf(line, "%d%n", &src->params.max_samples, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "maxsources")) {
      if (sscanf(line, "%d%n", &src->params.max_sources, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "mindelay")) {
      if (sscanf(line, "%lf%n", &src->params.min_delay, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "minpoll")) {
      if (sscanf(line, "%d%n", &src->params.minpoll, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "minsamples")) {
      if (sscanf(line, "%d%n", &src->params.min_samples, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "minstratum")) {
      if (sscanf(line, "%d%n", &src->params.min_stratum, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "offset")) {
      if (sscanf(line, "%lf%n", &src->params.offset, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "port")) {
      if (sscanf(line, "%hu%n", &src->port, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "polltarget")) {
      if (sscanf(line, "%d%n", &src->params.poll_target, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "presend")) {
      if (sscanf(line, "%d%n", &src->params.presend_minpoll, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "version")) {
      if (sscanf(line, "%d%n", &src->params.version, &n) != 1)
        return 0;
    } else if (!strcasecmp(cmd, "xleave")) {
      src->params.interleaved = 1;
    } else {
      return 0;
    }
  }

  return 1;
}

/* ================================================== */

int
CPS_ParseLocal(char *line, int *stratum, int *orphan, double *distance)
{
  int n;
  char *cmd;

  *stratum = 10;
  *distance = 1.0;
  *orphan = 0;

  while (*line) {
    cmd = line;
    line = CPS_SplitWord(line);

    if (!strcasecmp(cmd, "stratum")) {
      if (sscanf(line, "%d%n", stratum, &n) != 1 ||
          *stratum >= NTP_MAX_STRATUM || *stratum <= 0)
        return 0;
    } else if (!strcasecmp(cmd, "orphan")) {
      *orphan = 1;
      n = 0;
    } else if (!strcasecmp(cmd, "distance")) {
      if (sscanf(line, "%lf%n", distance, &n) != 1)
        return 0;
    } else {
      return 0;
    }

    line += n;
  }

  return 1;
}

/* ================================================== */

void
CPS_NormalizeLine(char *line)
{
  char *p, *q;
  int space = 1, first = 1;

  /* Remove white-space at beginning and replace white-spaces with space char */
  for (p = q = line; *p; p++) {
    if (isspace((unsigned char)*p)) {
      if (!space)
        *q++ = ' ';
      space = 1;
      continue;
    }

    /* Discard comment lines */
    if (first && strchr("!;#%", *p))
      break;

    *q++ = *p;
    space = first = 0;
  }

  /* Strip trailing space */
  if (q > line && q[-1] == ' ')
    q--;

  *q = '\0';
}

/* ================================================== */

char *
CPS_SplitWord(char *line)
{
  char *p = line, *q = line;

  /* Skip white-space before the word */
  while (*q && isspace((unsigned char)*q))
    q++;

  /* Move the word to the beginning */
  while (*q && !isspace((unsigned char)*q))
    *p++ = *q++;

  /* Find the next word */
  while (*q && isspace((unsigned char)*q))
    q++;

  *p = '\0';

  /* Return pointer to the next word or NUL */
  return q;
}

/* ================================================== */

int
CPS_ParseKey(char *line, uint32_t *id, const char **hash, char **key)
{
  char *s1, *s2, *s3, *s4;

  s1 = line;
  s2 = CPS_SplitWord(s1);
  s3 = CPS_SplitWord(s2);
  s4 = CPS_SplitWord(s3);

  /* Require two or three words */
  if (!*s2 || *s4)
    return 0;

  if (sscanf(s1, "%"SCNu32, id) != 1)
    return 0;

  if (*s3) {
    *hash = s2;
    *key = s3;
  } else {
    *hash = "MD5";
    *key = s2;
  }

  return 1;
}
