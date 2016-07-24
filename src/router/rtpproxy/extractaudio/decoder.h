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
 * $Id$
 *
 */

#ifndef _DECODER_H_
#define _DECODER_H_

#include <sys/types.h>

#include "config.h"

#ifdef ENABLE_G729
#include "g729_compat.h"
#endif
#ifdef ENABLE_G722
#include <g722_decoder.h>
#endif
#ifdef ENABLE_GSM
#include "gsm.h"
#endif

#include "rtp.h"
#include "session.h"

#define	DECODER_EOF	(-(1 << 16))
#define	DECODER_ERROR	(-(2 << 16))
#define	DECODER_SKIP	(-(3 << 16))

struct decoder_stream {
    struct session *sp;
    struct packet *pp;
    unsigned int nticks;
    unsigned int sticks;
    unsigned char lpt;
    unsigned char obuf[8 * 1024];	/* 0.5 seconds at 8 KHz 16 bits per sample */
    unsigned char *obp;
    unsigned int oblen;
#ifdef ENABLE_G729
    G729_DCTX *g729_ctx;
#endif
#ifdef ENABLE_G722
    G722_DEC_CTX *g722_ctx;
#endif
#ifdef ENABLE_GSM
    gsm ctx_gsm;
#endif
    double stime;
    double dticks;
    /* FILE *f; */
    int dflags;
};

#define D_FLAG_NONE      0x0
#define D_FLAG_NOSILENCE 0x1

void *decoder_new(struct session *, int);
int32_t decoder_get(struct decoder_stream *);
int decode_frame(struct decoder_stream *, unsigned char *, unsigned char *, unsigned int);
int generate_silence(struct decoder_stream *, unsigned char *, unsigned int);

#endif
