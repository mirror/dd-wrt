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

#include <math.h>

#include "rtpp_math.h"

void
PFD_init(struct PFD *pfd_p, double phi_round)
{

    pfd_p->target_clk = 0.0;
    pfd_p->phi_round = phi_round;
}

double
PFD_get_error(struct PFD *pfd_p, double dtime)
{
    double next_clk, err0r;

    if (pfd_p->phi_round > 0.0) {
        dtime = trunc(dtime * pfd_p->phi_round) / pfd_p->phi_round;
    }

    next_clk = trunc(dtime) + 1.0;
    if (pfd_p->target_clk == 0.0) {
        pfd_p->target_clk = next_clk;
        return (0.0);
    }

    err0r = pfd_p->target_clk - dtime;

    if (err0r > 0) {
        pfd_p->target_clk = next_clk + 1.0;
    } else {
        pfd_p->target_clk = next_clk;
    }

    return (err0r);
}

double
sigmoid(double x)
{

    return (x / (1 + fabs(x)));
}

double
recfilter_apply(struct recfilter *f, double x)
{

    f->lastval = f->a * x + f->b * f->lastval;
    if (f->peak_detect != 0) {
        if (f->lastval > f->maxval) {
            f->maxval = f->lastval;
        } if (f->lastval < f->minval) {
            f->minval = f->maxval;
        }
    }
    return f->lastval;
}

double
recfilter_apply_int(struct recfilter *f, int x)
{

    f->lastval = f->a * (double)(x) + f->b * f->lastval;
    if (f->peak_detect != 0) {
        if (f->lastval > f->maxval) {
            f->maxval = f->lastval;
        } if (f->lastval < f->minval) {
            f->minval = f->lastval;
        }
    }
    return f->lastval;
}

void
recfilter_init(struct recfilter *f, double fcoef, double initval, int peak_detect)
{

    f->lastval = initval;
    f->a = 1.0 - fcoef;
    f->b = fcoef;
    if (peak_detect != 0) {
        f->peak_detect = 1;
        f->maxval = initval;
        f->minval = initval;
    } else {
        f->peak_detect = 0;
        f->maxval = 0;
        f->minval = 0;
    }
}

double
freqoff_to_period(double freq_0, double foff_c, double foff_x)
{

    return (1.0 / freq_0 * (1 + foff_c * foff_x));
}
