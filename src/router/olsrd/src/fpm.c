/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include <stdio.h>
#include <assert.h>
#include "fpm.h"

#if 1 // def USE_FPM

#ifndef NDEBUG

fpm
itofpm(sfpm i)
{
  assert(FPM_MIN <= i && i <= FPM_MAX);
  return itofpm_def(i);
}

fpm
ftofpm(float f)
{
  fpm r;
  assert(FPM_MIN <= (sfpm) f && (sfpm) f <= FPM_MAX);
  r = (fpm) (sfpm) ftofpm_def(f);
  return r;
}

int
fpmtoi(fpm a)
{
  int r = fpmtoi_def((sfpm) a);
  return r;
}

float
fpmtof(fpm a)
{
  float r = fpmtof_def((sfpm) a);
  return r;
}

double
fpmtod(fpm a)
{
  double r = fpmtod_def((sfpm) a);
  return r;
}

fpm
fpmadd(fpm a, fpm b)
{
  fpm r;
  assert(0 > (sfpm) a || 0 > (sfpm) b || FPM_INT_MAX - (sfpm) a >= (sfpm) b);
  assert(0 <= (sfpm) a || 0 <= (sfpm) b || (sfpm) a >= FPM_INT_MIN - (sfpm) b);
  r = (fpm) fpmadd_def((sfpm) a, (sfpm) b);
  return r;
}

fpm
fpmsub(fpm a, fpm b)
{
  fpm r;
  assert(0 > (sfpm) a || 0 <= (sfpm) b || (sfpm) a < FPM_INT_MAX + (sfpm) b);
  assert(0 <= (sfpm) a || 0 > (sfpm) b || (sfpm) a >= FPM_INT_MIN + (sfpm) b);
  r = (fpm) fpmsub_def((sfpm) a, (sfpm) b);
  return r;
}

fpm
fpmmul(fpm a, fpm b)
{
  fpm r;
  assert((0 < (sfpm) a) != (0 < (sfpm) b) || ((double)(sfpm) a) * ((double)(sfpm) b) <= (double)FPM_INT_MAX);
  assert((0 < (sfpm) a) == (0 < (sfpm) b) || ((double)(sfpm) a) * ((double)(sfpm) b) >= (double)FPM_INT_MIN);
  r = (fpm) fpmmul_def((sfpm) a, (sfpm) b);
  return r;
}

fpm
fpmdiv(fpm a, fpm b)
{
  fpm r;

  /*
   * The following two asserts are always true
   *
   * long long tmp = ((long long)(sfpm) a << FPM_BIT);
   * assert(FPM_INT_MIN <= tmp);
   * assert(tmp <= FPM_INT_MAX);
   */
  r = (fpm) fpmdiv_def((sfpm) a, (sfpm) b);
  return r;
}

fpm
fpmimul(int a, fpm b)
{
  fpm r;
  assert((0 < a) != (0 < (sfpm) b) || ((double)a * (double)(sfpm) b) <= (double)FPM_INT_MAX);
  assert((0 < a) == (0 < (sfpm) b) || ((double)a * (double)(sfpm) b) >= (double)FPM_INT_MIN);
  r = (fpm) fpmimul_def(a, (sfpm) b);
  return r;
}

fpm
fpmmuli(fpm a, int b)
{
  fpm r;
  assert((0 < (sfpm) a) != (0 < b) || ((double)(sfpm) a * (double)b) <= (double)FPM_INT_MAX);
  assert((0 < (sfpm) a) == (0 < b) || ((double)(sfpm) a * (double)b) >= (double)FPM_INT_MIN);
  r = (fpm) fpmmuli_def((sfpm) a, b);
  return r;
}

fpm
fpmidiv(fpm a, int b)
{
  fpm r;
  r = (fpm) fpmidiv_def((sfpm) a, b);
  return r;
}

#endif /* NDEBUG */

fpm
atofpm(const char *s)
{
  float r = 0.0;
  sscanf(s, "%f", &r);
  return ftofpm(r);
}

const char *
fpmtoa(fpm a)
{
  static int idx = 0;
  static char ret[4][20];

  idx = (idx + 1) % (sizeof(ret) / sizeof(ret[0]));
  snprintf(ret[idx], sizeof(ret[0]), "%ld.%03ld", (sfpm) a >> FPM_BIT, (1000 * ((sfpm) (a) & FPM_MSK) + (FPM_NUM / 2)) >> FPM_BIT);
  return ret[idx];
}

#else /* USE_FPM */

float
atofpm(const char *s)
{
  float r = 0.0;
  sscanf(s, "%f", &r);
  return r;
}

const char *
fpmtoa(float a)
{
  static int idx = 0;
  static char ret[4][20];

  idx = (idx + 1) % (sizeof(ret) / sizeof(ret[0]));
  snprintf(ret[idx], sizeof(ret[0]), "%.3f", a);
  return ret[idx];
}

#endif /* USE_FPM */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
