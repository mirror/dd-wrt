/*-
 * Copyright (c) 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Some portions Copyright (C) 2010 Sourcefire, Inc.
 *
 * This code is derived from the Stanford/CMU enet packet filter,
 * (net/enet.c) distributed as part of 4.3BSD, and code contributed
 * to Berkeley by Steven McCanne and Van Jacobson both of Lawrence 
 * Berkeley Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)bpf.h       7.1 (Berkeley) 5/7/91
 *
 * @(#) $Header: /usr/cvsroot/sfeng/ims/src/libraries/daq/daq/sfbpf/sfbpf.h,v 1.2 2010/04/20 19:49:32 maltizer Exp $ (LBL)
 */

/*
 * This is libDAQ's cut-down version of libpcap's cut-down version of bpf.h;
 * it includes only the stuff needed for the code generator and the userland
 * BPF interpreter, and the libDAQ APIs for setting filters, etc..
 *
 * Mostly things have been renamed so as to not conflict with the original
 * libpcap BPF headers.
 *
 * Datalink type definitions have been extracted and placed in sfbpf_dlt.h.
 */

#ifndef _SFBPF_H
#define _SFBPF_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BSD style release date */
#define SFBPF_RELEASE 199606

typedef	int sfbpf_int32;
typedef	u_int sfbpf_u_int32;

/*
 * Alignment macros.  SFBPF_WORDALIGN rounds up to the next 
 * even multiple of SFBPF_ALIGNMENT. 
 */
#define SFBPF_ALIGNMENT sizeof(sfbpf_int32)
#define SFBPF_WORDALIGN(x) (((x)+(SFBPF_ALIGNMENT-1))&~(SFBPF_ALIGNMENT-1))

#define SFBPF_MAXBUFSIZE 0x8000
#define SFBPF_MINBUFSIZE 32

/*
 * Structure for "pcap_compile()", "pcap_setfilter()", etc..
 */
struct sfbpf_program {
	u_int bf_len;
	struct sfbpf_insn *bf_insns;
};
 
/*
 * Struct return by BIOCVERSION.  This represents the version number of 
 * the filter language described by the instruction encodings below.
 * bpf understands a program iff kernel_major == filter_major &&
 * kernel_minor >= filter_minor, that is, if the value returned by the
 * running kernel has the same major number and a minor number equal
 * equal to or less than the filter being downloaded.  Otherwise, the
 * results are undefined, meaning an error may be returned or packets
 * may be accepted haphazardly.
 * It has nothing to do with the source code version.
 */
struct sfbpf_version {
	u_short bv_major;
	u_short bv_minor;
};
/* Current version number of filter architecture. */
#define SFBPF_MAJOR_VERSION 1
#define SFBPF_MINOR_VERSION 1

#include <sfbpf_dlt.h>

/*
 * The instruction encodings.
 */
/* instruction classes */
#define SFBPF_CLASS(code) ((code) & 0x07)
#define		SFBPF_LD		0x00
#define		SFBPF_LDX		0x01
#define		SFBPF_ST		0x02
#define		SFBPF_STX		0x03
#define		SFBPF_ALU		0x04
#define		SFBPF_JMP		0x05
#define		SFBPF_RET		0x06
#define		SFBPF_MISC	0x07

/* ld/ldx fields */
#define SFBPF_SIZE(code)	((code) & 0x18)
#define		SFBPF_W		0x00
#define		SFBPF_H		0x08
#define		SFBPF_B		0x10
#define SFBPF_MODE(code)	((code) & 0xe0)
#define		SFBPF_IMM 	0x00
#define		SFBPF_ABS		0x20
#define		SFBPF_IND		0x40
#define		SFBPF_MEM		0x60
#define		SFBPF_LEN		0x80
#define		SFBPF_MSH		0xa0

/* alu/jmp fields */
#define SFBPF_OP(code)	((code) & 0xf0)
#define		SFBPF_ADD		0x00
#define		SFBPF_SUB		0x10
#define		SFBPF_MUL		0x20
#define		SFBPF_DIV		0x30
#define		SFBPF_OR		0x40
#define		SFBPF_AND		0x50
#define		SFBPF_LSH		0x60
#define		SFBPF_RSH		0x70
#define		SFBPF_NEG		0x80
#define		SFBPF_JA		0x00
#define		SFBPF_JEQ		0x10
#define		SFBPF_JGT		0x20
#define		SFBPF_JGE		0x30
#define		SFBPF_JSET	0x40
#define SFBPF_SRC(code)	((code) & 0x08)
#define		SFBPF_K		0x00
#define		SFBPF_X		0x08

/* ret - SFBPF_K and SFBPF_X also apply */
#define SFBPF_RVAL(code)	((code) & 0x18)
#define		SFBPF_A		0x10

/* misc */
#define SFBPF_MISCOP(code) ((code) & 0xf8)
#define		SFBPF_TAX		0x00
#define		SFBPF_TXA		0x80

/*
 * The instruction data structure.
 */
struct sfbpf_insn {
	u_short	code;
	u_char 	jt;
	u_char 	jf;
	sfbpf_u_int32 k;
};

/*
 * Macros for insn array initializers.
 */
#define SFBPF_STMT(code, k) { (u_short)(code), 0, 0, k }
#define SFBPF_JUMP(code, k, jt, jf) { (u_short)(code), jt, jf, k }

//#if __STDC__ || defined(__cplusplus)
int sfbpf_compile(int snaplen_arg, int linktype_arg, struct sfbpf_program *program, const char *buf, int optimize, sfbpf_u_int32 mask);
int sfbpf_validate(const struct sfbpf_insn *f, int len);
u_int sfbpf_filter(const struct sfbpf_insn *pc, const u_char *p, u_int wirelen, u_int buflen);
void sfbpf_freecode(struct sfbpf_program *program);
void sfbpf_print(struct sfbpf_program *fp, int verbose);
/*
#else
int sfbpf_compile();
int sfbpf_validate();
u_int sfbpf_filter();
void sfbpf_freecode();
#endif
*/
/*
 * Number of scratch memory words (for SFBPF_LD|SFBPF_MEM and SFBPF_ST).
 */
#define SFBPF_MEMWORDS 16

#ifdef __cplusplus
}
#endif

#endif
