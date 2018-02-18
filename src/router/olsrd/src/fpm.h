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

#ifndef _FPM_H
#define _FPM_H

#if 1 // def USE_FPM

/*
 * The standard math should function with only 32 bits
 */
typedef long fpm;
typedef signed long sfpm;
typedef unsigned long ufpm;
#define FPM_BIT 10

#define FPM_NUM (1 << FPM_BIT)
#define FPM_MSK (FPM_NUM - 1)
#define FPM_MAX ((sfpm)(~(ufpm)0 >> 1) >> FPM_BIT)
#define FPM_MIN ((sfpm)-1 - FPM_MAX)
#define FPM_INT_MAX ((sfpm)(~(ufpm)0 >> 1))
#define FPM_INT_MIN ((sfpm)-1 - FPM_INT_MAX)

#define itofpm_def(a) (fpm)((sfpm)((a) << FPM_BIT))
#define ftofpm_def(a) (fpm)((sfpm)((a) * FPM_NUM))
#define fpmtoi_def(a) (int)((sfpm)(a) >> FPM_BIT)
#define fpmtof_def(a) ((float)(sfpm)(a) / FPM_NUM)
#define fpmtod_def(a) ((double)(sfpm)(a) / FPM_NUM)

#define fpmadd_def(a, b) (fpm)((sfpm)(a) + (sfpm)(b))
#define fpmsub_def(a, b) (fpm)((sfpm)(a) - (sfpm)(b))
#define fpmmul_def(a, b) (fpm)(((sfpm)(a) * (sfpm)(b)) >> FPM_BIT)
#define fpmdiv_def(a, b) (fpm)(((sfpm)(a) << FPM_BIT) / (sfpm)(b))

/*
 * Special: first or second factor is an integer
 */
#define fpmimul_def(a, b) (fpm)((int)(a) * (sfpm)(b))
#define fpmmuli_def(a, b) (fpm)((sfpm)(a) * (int)(b))

/*
 * Special: divisor is an integer
 */
#define fpmidiv_def(a, b) (fpm)((sfpm)(a) / (int)(b))

#ifdef NDEBUG

#define itofpm itofpm_def
#define ftofpm ftofpm_def
#define fpmtoi fpmtoi_def
#define fpmtof fpmtof_def
#define fpmtod fpmtod_def

#define fpmadd fpmadd_def
#define fpmsub fpmsub_def
#define fpmmul fpmmul_def
#define fpmdiv fpmdiv_def
#define fpmimul fpmimul_def
#define fpmmuli fpmmuli_def
#define fpmidiv fpmidiv_def

#else /* NDEBUG */

fpm itofpm(sfpm i);
fpm ftofpm(float f);
int fpmtoi(fpm a);
float fpmtof(fpm a);
double fpmtod(fpm a);

fpm fpmadd(fpm a, fpm b);
fpm fpmsub(fpm a, fpm b);
fpm fpmmul(fpm a, fpm b);
fpm fpmdiv(fpm a, fpm b);
fpm fpmimul(int a, fpm b);
fpm fpmmuli(fpm a, int b);
fpm fpmidiv(fpm a, int b);

#endif /* NDEBUG */

#define INFINITE_ETX itofpm(FPM_MAX)
#define MIN_LINK_QUALITY ftofpm(0.01)
#define ZERO_ETX itofpm(0)
#define CEIL_LQDIFF ftofpm(1.1)
#define FLOOR_LQDIFF ftofpm(0.9)

fpm atofpm(const char *);
const char *fpmtoa(fpm);

#else /* USE_FPM */

#define INFINITE_ETX ((float)(1 << 30))
#define ZERO_ETX 0.0
#define MIN_LINK_QUALITY 0.01
#define CEIL_LQDIFF 1.1
#define FLOOR_LQDIFF 0.9

float atofpm(const char *);
const char *fpmtoa(float);

#endif /* USE_FPM */

#endif /* _FPM_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
