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

#include <config.h>
#include <sysincl.h>
#include <logging.h>
#include <localp.h>
#include <util.h>

#include "test.h"

void
TST_Fail(int line)
{
  printf("FAIL (on line %d)\n", line);
  exit(1);
}

void
TST_Skip(int line)
{
  printf("SKIP (on line %d)\n", line);
  exit(0);
}

int
main(int argc, char **argv)
{
  char *test_name, *s;
  int i, seed = 0;
  struct timeval tv;

  test_name = argv[0];
  s = strrchr(test_name, '.');
  if (s)
    *s = '\0';
  s = strrchr(test_name, '/');
  if (s)
    test_name = s + 1;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-d")) {
      LOG_SetDebugLevel(2);
    } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
      seed = atoi(argv[++i]);
    } else {
      fprintf(stderr, "Unknown option\n");
      exit(1);
    }
  }

  gettimeofday(&tv, NULL);
  srandom(seed ? seed : tv.tv_sec ^ (tv.tv_usec << 10));

  printf("Testing %-30s ", test_name);
  fflush(stdout);

  LOG_Initialise();

  test_unit();

  LOG_Finalise();

  printf("PASS\n");

  return 0;
}

void TST_SuspendLogging(void)
{
  LOG_OpenFileLog("/dev/null");
}

void TST_ResumeLogging(void)
{
  LOG_OpenFileLog(NULL);
}

double
TST_GetRandomDouble(double min, double max)
{
  return min + (double)random() / RAND_MAX * (max - min);
}

void
TST_GetRandomAddress(IPAddr *ip, int family, int bits)
{
  if (family != IPADDR_INET4 && family != IPADDR_INET6)
    family = random() % 2 ? IPADDR_INET4 : IPADDR_INET6;

  ip->family = family;

  if (family == IPADDR_INET4) {
    if (bits < 0)
      bits = 32;
    assert(bits <= 32);

    if (bits > 16)
      ip->addr.in4 = (uint32_t)random() % (1U << (bits - 16)) << 16 |
                     (uint32_t)random() % (1U << 16);
    else
      ip->addr.in4 = (uint32_t)random() % (1U << bits);
  } else {
    int i, b;

    if (bits < 0)
      bits = 128;
    assert(bits <= 128);

    for (i = 0, b = 120; i < 16; i++, b -= 8) {
      if (b >= bits) {
        ip->addr.in6[i] = 0;
      } else {
        ip->addr.in6[i] = random() % (1U << MIN(bits - b, 8));
      }
    }
  }
}

void
TST_SwapAddressBit(IPAddr *ip, unsigned int b)
{
  if (ip->family == IPADDR_INET4) {
    assert(b < 32);
    ip->addr.in4 ^= 1U << (31 - b);
  } else if (ip->family == IPADDR_INET6) {
    assert(b < 128);
    ip->addr.in6[b / 8] ^= 1U << (7 - b % 8);
  } else {
    assert(0);
  }
}

static double
read_frequency(void)
{
  return 0.0;
}

static double
set_frequency(double freq_ppm)
{
  return 0.0;
}

static void
accrue_offset(double offset, double corr_rate)
{
}

static int
apply_step_offset(double offset)
{
  return 0;
}

static void
offset_convert(struct timespec *raw, double *corr, double *err)
{
  *corr = 0.0;
  if (err)
    *err = 0.0;
}

void
TST_RegisterDummyDrivers(void)
{
  lcl_RegisterSystemDrivers(read_frequency, set_frequency, accrue_offset,
                            apply_step_offset, offset_convert, NULL, NULL);
}
