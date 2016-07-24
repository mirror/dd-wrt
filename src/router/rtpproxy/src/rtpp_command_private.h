/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
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

#ifndef _RTPP_COMMAND_PRIVATE_H_
#define _RTPP_COMMAND_PRIVATE_H_

struct rtpp_command_stat {
    uint64_t cnt;
    int cnt_idx;
};

struct rtpp_command_stats {
    struct rtpp_command_stat ncmds_rcvd;
    struct rtpp_command_stat ncmds_succd;
    struct rtpp_command_stat ncmds_errs;
    struct rtpp_command_stat ncmds_repld;

    struct rtpp_command_stat nsess_complete;
    struct rtpp_command_stat nsess_created;

    struct rtpp_command_stat nplrs_created;
    struct rtpp_command_stat nplrs_destroyed;
};

#define RTPC_MAX_ARGC   20

enum rtpp_cmd_op {DELETE, RECORD, PLAY, NOPLAY, COPY, UPDATE, LOOKUP, INFO,
  QUERY, VER_FEATURE, GET_VER, DELETE_ALL, GET_STATS};

struct common_cmd_args {
    enum rtpp_cmd_op op;
    const char *rname;
    const char *hint;
    char *call_id;
    char *from_tag;
    char *to_tag;
};

struct rtpp_command
{
    char buf[1024 * 8];
    char buf_t[256];
    char buf_r[256];
    char *argv[RTPC_MAX_ARGC];
    int argc;
    struct sockaddr_storage raddr;
    socklen_t rlen;
    char *cookie;
    int controlfd;
    double dtime;
    struct rtpp_command_stats *csp;
    int umode;
    struct common_cmd_args cca;
    int no_glock;
};

#define ECODE_CMDUNKN      0

#define ECODE_PARSE_NARGS  1
#define ECODE_PARSE_MODS   2

#define ECODE_PARSE_1      5
#define ECODE_PARSE_2      6
#define ECODE_PARSE_3      7
#define ECODE_PARSE_4      8
#define ECODE_PARSE_5      9
#define ECODE_PARSE_10    10
#define ECODE_PARSE_11    11
#define ECODE_PARSE_12    12
#define ECODE_PARSE_13    13
#define ECODE_PARSE_14    14
#define ECODE_PARSE_15    15
#define ECODE_PARSE_16    16
#define ECODE_PARSE_6     17
#define ECODE_PARSE_7     18

#define ECODE_RTOOBIG_1   25

#define ECODE_INVLARG_1   31
#define ECODE_INVLARG_2   32
#define ECODE_INVLARG_3   33
#define ECODE_INVLARG_4   34
#define ECODE_INVLARG_5   35

#define ECODE_SESUNKN     50

#define ECODE_PLRFAIL     60
#define ECODE_CPYFAIL     65
#define ECODE_STSFAIL     68

#define ECODE_LSTFAIL_1   71
#define ECODE_LSTFAIL_2   72

#define ECODE_NOMEM_1     81
#define ECODE_NOMEM_2     82
#define ECODE_NOMEM_3     83
#define ECODE_NOMEM_4     84
#define ECODE_NOMEM_5     85
#define ECODE_NOMEM_6     86
#define ECODE_NOMEM_7     87
#define ECODE_NOMEM_8     88

#define ECODE_SLOWSHTDN   99

#endif
