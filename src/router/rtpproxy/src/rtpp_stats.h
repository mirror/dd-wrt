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

struct rtpp_stats_obj;

#if !defined(DEFINE_METHOD)
#error "rtpp_types.h" needs to be included
#endif

DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_dtor, void);
DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_getidxbyname, int, const char *);
DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_updatebyidx, int, int, uint64_t);
DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_updatebyname, int, const char *, uint64_t);
DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_updatebyname_d, int, const char *, double);
DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_getlvalbyname, int64_t, const char *);
DEFINE_METHOD(rtpp_stats_obj, rtpp_stats_obj_nstr, int, char *, int, const char *);

struct rtpp_stats_obj_priv;

#define RTPP_NSTATS 22

struct rtpp_stats_obj
{
    rtpp_stats_obj_dtor_t dtor;
    rtpp_stats_obj_getidxbyname_t getidxbyname;
    rtpp_stats_obj_updatebyidx_t updatebyidx;
    rtpp_stats_obj_updatebyname_t updatebyname;
    rtpp_stats_obj_updatebyname_d_t updatebyname_d;
    rtpp_stats_obj_getlvalbyname_t getlvalbyname;
    rtpp_stats_obj_nstr_t nstr;
    struct rtpp_stats_obj_priv *pvt;
};

struct rtpp_stats_obj *rtpp_stats_ctor(void);
