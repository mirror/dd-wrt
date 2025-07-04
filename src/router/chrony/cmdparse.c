/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2013-2014, 2016, 2021
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

#define SSCANF_IN_RANGE(s, f, x, n, min, max) \
  (sscanf((s), (f), (x), (n)) == 1 && *(x) >= (min) && *(x) <= (max))

/* ================================================== */

CPS_Status
CPS_ParseNTPSourceAdd(char *line, CPS_NTP_Source *src)
{
  char *hostname, *cmd;
  uint32_t ef_type;
  int n, sel_option;
  
  src->family = IPADDR_UNSPEC;
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
  src->params.nts = 0;
  src->params.nts_port = SRC_DEFAULT_NTSPORT;
  src->params.copy = 0;
  src->params.ext_fields = 0;
  src->params.authkey = INACTIVE_AUTHKEY;
  src->params.cert_set = SRC_DEFAULT_CERTSET;
  src->params.max_delay = SRC_DEFAULT_MAXDELAY;
  src->params.max_delay_ratio = SRC_DEFAULT_MAXDELAYRATIO;
  src->params.max_delay_dev_ratio = SRC_DEFAULT_MAXDELAYDEVRATIO;
  src->params.max_delay_quant = 0.0;
  src->params.min_delay = 0.0;
  src->params.asymmetry = SRC_DEFAULT_ASYMMETRY;
  src->params.offset = 0.0;

  hostname = line;
  line = CPS_SplitWord(line);

  if (!*hostname)
    return CPS_MissingArgument;

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
    } else if (!strcasecmp(cmd, "copy")) {
      src->params.copy = 1;
    } else if (!strcasecmp(cmd, "iburst")) {
      src->params.iburst = 1;
    } else if (!strcasecmp(cmd, "offline")) {
      src->params.connectivity = SRC_OFFLINE;
    } else if (!strcasecmp(cmd, "certset")) {
      if (sscanf(line, "%"SCNu32"%n", &src->params.cert_set, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "key")) {
      if (sscanf(line, "%"SCNu32"%n", &src->params.authkey, &n) != 1 ||
          src->params.authkey == INACTIVE_AUTHKEY)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "asymmetry")) {
      if (sscanf(line, "%lf%n", &src->params.asymmetry, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "extfield")) {
      if (sscanf(line, "%"SCNx32"%n", &ef_type, &n) != 1)
        return CPS_InvalidValue;
      switch (ef_type) {
        case NTP_EF_EXP_MONO_ROOT:
          src->params.ext_fields |= NTP_EF_FLAG_EXP_MONO_ROOT;
          break;
        case NTP_EF_EXP_NET_CORRECTION:
          src->params.ext_fields |= NTP_EF_FLAG_EXP_NET_CORRECTION;
          break;
        default:
          return CPS_InvalidValue;
      }
    } else if (!strcasecmp(cmd, "filter")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.filter_length, &n, 0, INT_MAX))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "ipv4")) {
      src->family = IPADDR_INET4;
    } else if (!strcasecmp(cmd, "ipv6")) {
      src->family = IPADDR_INET6;
    } else if (!strcasecmp(cmd, "maxdelay")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "maxdelayratio")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay_ratio, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "maxdelaydevratio")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay_dev_ratio, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "maxdelayquant")) {
      if (sscanf(line, "%lf%n", &src->params.max_delay_quant, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "maxpoll")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.maxpoll, &n, -32, 32))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "maxsamples")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.max_samples, &n, 0, INT_MAX))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "maxsources")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.max_sources, &n, 1, INT_MAX))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "mindelay")) {
      if (sscanf(line, "%lf%n", &src->params.min_delay, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "minpoll")) {
      if (sscanf(line, "%d%n", &src->params.minpoll, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "minsamples")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.min_samples, &n, 0, INT_MAX))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "minstratum")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.min_stratum, &n, 0, NTP_MAX_STRATUM))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "nts")) {
      src->params.nts = 1;
    } else if (!strcasecmp(cmd, "ntsport")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.nts_port, &n, 0, 65535))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "offset")) {
      if (sscanf(line, "%lf%n", &src->params.offset, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "port")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->port, &n, 0, 65535))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "polltarget")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.poll_target, &n, 1, INT_MAX))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "presend")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.presend_minpoll, &n, -32, 32))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "version")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &src->params.version, &n, 1, NTP_VERSION))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "xleave")) {
      src->params.interleaved = 1;
    } else if ((sel_option = CPS_GetSelectOption(cmd)) != 0) {
      src->params.sel_options |= sel_option;
    } else {
      return CPS_InvalidOption;
    }
  }

  return CPS_Success;
}

/* ================================================== */

int
CPS_GetSelectOption(char *option)
{
  if (!strcasecmp(option, "noselect")) {
    return SRC_SELECT_NOSELECT;
  } else if (!strcasecmp(option, "prefer")) {
    return SRC_SELECT_PREFER;
  } else if (!strcasecmp(option, "require")) {
    return SRC_SELECT_REQUIRE;
  } else if (!strcasecmp(option, "trust")) {
    return SRC_SELECT_TRUST;
  }
  return 0;
}

/* ================================================== */

int
CPS_ParseAllowDeny(char *line, int *all, IPAddr *ip, int *subnet_bits)
{
  char *p, *net, *slash;
  uint32_t a, b, c;
  int bits, len, n;

  p = CPS_SplitWord(line);

  if (strcmp(line, "all") == 0) {
    *all = 1;
    net = p;
    p = CPS_SplitWord(p);
  } else {
    *all = 0;
    net = line;
  }

  /* Make sure there are no other arguments */
  if (*p)
    return 0;

  /* No specified address or network means all IPv4 and IPv6 addresses */
  if (!*net) {
    ip->family = IPADDR_UNSPEC;
    *subnet_bits = 0;
    return 1;
  }

  slash = strchr(net, '/');
  if (slash) {
    if (sscanf(slash + 1, "%d%n", &bits, &len) != 1 || slash[len + 1] || bits < 0)
      return 0;
    *slash = '\0';
  } else {
    bits = -1;
  }

  if (UTI_StringToIP(net, ip)) {
    if (bits >= 0)
      *subnet_bits = bits;
    else
      *subnet_bits = ip->family == IPADDR_INET6 ? 128 : 32;
    return 1;
  }

  /* Check for a shortened IPv4 network notation using only 1, 2, or 3 decimal
     numbers.  This is different than the numbers-and-dots notation accepted
     by inet_aton()! */

  a = b = c = 0;
  n = sscanf(net, "%"PRIu32"%n.%"PRIu32"%n.%"PRIu32"%n", &a, &len, &b, &len, &c, &len);

  if (n > 0 && !net[len]) {
    if (a > 255 || b > 255 || c > 255)
      return 0;

    ip->family = IPADDR_INET4;
    ip->addr.in4 = (a << 24) | (b << 16) | (c << 8);

    if (bits >= 0)
      *subnet_bits = bits;
    else
      *subnet_bits = n * 8;

    return 1;
  }

  /* The last possibility is a hostname */
  if (bits < 0 && DNS_Name2IPAddress(net, ip, 1) == DNS_Success) {
    *subnet_bits = ip->family == IPADDR_INET6 ? 128 : 32;
    return 1;
  }

  return 0;
}

/* ================================================== */

CPS_Status
CPS_ParseLocal(char *line, int *stratum, int *orphan, double *distance, double *activate,
               double *wait_synced, double *wait_unsynced)
{
  int n;
  char *cmd;

  *stratum = 10;
  *distance = 1.0;
  *activate = 0.0;
  *orphan = 0;
  *wait_synced = 0;
  *wait_unsynced = -1.0;

  while (*line) {
    cmd = line;
    line = CPS_SplitWord(line);

    if (!strcasecmp(cmd, "stratum")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", stratum, &n, 1, NTP_MAX_STRATUM - 1))
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "orphan")) {
      *orphan = 1;
      n = 0;
    } else if (!strcasecmp(cmd, "distance")) {
      if (sscanf(line, "%lf%n", distance, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "activate")) {
      if (sscanf(line, "%lf%n", activate, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "waitsynced")) {
      if (sscanf(line, "%lf%n", wait_synced, &n) != 1)
        return CPS_InvalidValue;
    } else if (!strcasecmp(cmd, "waitunsynced")) {
      if (sscanf(line, "%lf%n", wait_unsynced, &n) != 1)
        return CPS_InvalidValue;
    } else {
      return CPS_InvalidOption;
    }

    line += n;
  }

  if (*wait_unsynced < 0.0)
    *wait_unsynced = *orphan ? 300 : 0.0;

  return CPS_Success;
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
CPS_ParseKey(char *line, uint32_t *id, const char **type, char **key)
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
    *type = s2;
    *key = s3;
  } else {
    *type = "MD5";
    *key = s2;
  }

  return 1;
}

/* ================================================== */

int
CPS_ParseRefid(char *line, uint32_t *ref_id)
{
  int i;

  for (i = *ref_id = 0; line[i] && !isspace((unsigned char)line[i]); i++) {
    if (i >= 4)
      return 0;
    *ref_id |= (uint32_t)line[i] << (24 - i * 8);
  }

  return i;
}
