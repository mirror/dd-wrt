/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
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

#ifndef _RTPP_COMMAND_H_
#define _RTPP_COMMAND_H_

struct proto_cap {
    const char  *pc_id;
    const char  *pc_description;
};

struct rtpp_command;
struct rtpp_command_stats;
struct cfg;
struct cfg_stable;
struct sockaddr;

extern struct proto_cap proto_caps[];

int handle_command(struct cfg *, struct rtpp_command *);
void free_command(struct rtpp_command *);
struct rtpp_command *get_command(struct cfg *, int, int *, double,
  struct rtpp_command_stats *csp, int umode);
void reply_error(struct cfg *cf, struct rtpp_command *cmd, int ecode);
void reply_port(struct cfg *cf, struct rtpp_command *cmd, int lport,
  struct sockaddr **lia);
int rtpp_create_listener(struct cfg *, struct sockaddr *, int *, int *);

void rtpc_doreply(struct cfg *, char *, int, struct rtpp_command *, int);

#endif
