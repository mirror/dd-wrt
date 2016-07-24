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

struct rtpp_hash_table_obj;
struct rtpp_hash_table_entry;

DEFINE_METHOD(rtpp_hash_table_obj, hash_table_append, struct rtpp_hash_table_entry *, const char *, void *);
DEFINE_METHOD(rtpp_hash_table_obj, hash_table_remove, void, const char *key, struct rtpp_hash_table_entry *sp);
DEFINE_METHOD(rtpp_hash_table_obj, hash_table_remove_nc, void, struct rtpp_hash_table_entry *sp);
DEFINE_METHOD(rtpp_hash_table_obj, hash_table_findfirst, struct rtpp_hash_table_entry *,
  const char *key, void **);
DEFINE_METHOD(rtpp_hash_table_obj, hash_table_findnext,  struct rtpp_hash_table_entry *,
  struct rtpp_hash_table_entry *, void **);
DEFINE_METHOD(rtpp_hash_table_obj, hash_table_dtor, void);

struct rtpp_hash_table_priv;

struct rtpp_hash_table_obj
{
    hash_table_append_t append;
    hash_table_remove_t remove;
    hash_table_remove_nc_t remove_nc;
    hash_table_findfirst_t findfirst;
    hash_table_findnext_t findnext;
    hash_table_dtor_t dtor;
    struct rtpp_hash_table_priv *pvt;
};

struct rtpp_hash_table_obj *rtpp_hash_table_ctor(void);
