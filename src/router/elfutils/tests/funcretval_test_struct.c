/* Copyright (C) 2024 Mark J. Wielaard <mark@klomp.org>
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

typedef struct
  {
    int q;
    int r;
  } div_t;

typedef struct
  {
    long q;
    long r;
  } ldiv_t;

typedef struct
  {
    float x;
    float y;
  } point_t;

typedef struct
  {
    double x;
    double y;
  } dpoint_t;

div_t __attribute__((__noinline__))
div (int n, int d)
{
  div_t r;
  r.q = n / d;
  r.r = n % d;
  return r;
}

ldiv_t __attribute__((__noinline__))
ldiv (long n, long d)
{
  ldiv_t r;
  r.q = n / d;
  r.r = n % d;
  return r;
}

point_t __attribute__((__noinline__))
mkpt (float x, float y)
{
  point_t r;
  r.x = x;
  r.y = y;
  return r;
}

dpoint_t __attribute__((__noinline__))
dmkpt (double x, double y)
{
  dpoint_t r;
  r.x = x;
  r.y = y;
  return r;
}

int
main (void)
{
  div_t d = div (3, 2);
  ldiv_t ld = ldiv (3, 2);
  point_t p = mkpt (3.0f, 1.0f);
  dpoint_t dp = dmkpt (3.0e0, 1.0e0);

  return d.q - (int) p.y + ld.q - (int) dp.y;
}
