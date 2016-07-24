/*
 * Copyright (c) 2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _RTPP_MATH_H_
#define _RTPP_MATH_H_

#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif
#ifdef ABS
#undef ABS
#endif
#define MIN(x, y)       (((x) > (y)) ? (y) : (x))
#define MAX(x, y)       (((x) > (y)) ? (x) : (y))
#define ABS(x)          ((x) > 0 ? (x) : (-x))

struct recfilter {
    double a;
    double b;
    double lastval;
    double minval;
    double maxval;
    int peak_detect;
};

struct PFD {
    double target_clk;
    double phi_round;
};

/* Function prototypes */
double sigmoid(double);

double recfilter_apply(struct recfilter *, double);
double recfilter_apply_int(struct recfilter *, int);
void recfilter_init(struct recfilter *, double, double, int);

void PFD_init(struct PFD *, double);
double PFD_get_error(struct PFD *, double);

double freqoff_to_period(double freq_0, double foff_c, double foff_x);

#endif /* _RTPP_MATH_H_ */
