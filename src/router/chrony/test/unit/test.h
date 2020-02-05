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

#ifndef GOT_TEST_H
#define GOT_TEST_H

#include <addressing.h>

extern void test_unit(void);

#define TEST_CHECK(expr) \
  do { \
    if (!(expr)) { \
      TST_Fail(__LINE__); \
      exit(1); \
    } \
  } while (0)

#define TEST_REQUIRE(expr) \
  do { \
    if (!(expr)) { \
      TST_Skip(__LINE__); \
      exit(0); \
    } \
  } while (0)

extern void TST_Fail(int line);
extern void TST_Skip(int line);

extern void TST_SuspendLogging(void);
extern void TST_ResumeLogging(void);

extern double TST_GetRandomDouble(double min, double max);
extern void TST_GetRandomAddress(IPAddr *ip, int family, int bits);
extern void TST_SwapAddressBit(IPAddr *ip, unsigned int b);
extern void TST_RegisterDummyDrivers(void);

#endif
