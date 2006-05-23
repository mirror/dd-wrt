/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
   jffs2_bbc_lzo_comp.c -- LZO1X-1 (and -999) compression module for jffs2
   Copyright (C) 2004 Patrik Kluba
   Based on the original LZO sources
   $Header: /openwrt/openwrt/package/linux/kernel-patches/301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*/

/*
   Original copyright notice follows:

   lzo1x_9x.c -- implementation of the LZO1X-999 compression algorithm
   lzo_ptr.h -- low-level pointer constructs
   lzo_swd.ch -- sliding window dictionary
   lzoconf.h -- configuration for the LZO real-time data compression library
   lzo_mchw.ch -- matching functions using a window
   minilzo.c -- mini subset of the LZO real-time data compression library
   config1x.h -- configuration for the LZO1X algorithm
   lzo1x.h -- public interface of the LZO1X compression algorithm

   These files are part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
*/

/*

	2004-02-16  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Initial release
					-removed all 16 bit code
					-all sensitive data will be on 4 byte boundary
					-removed check parts for library use
					-removed all but LZO1X-* compression
					
*/

#ifndef __KERNEL__
  #include <sys/types.h>
  #include <stddef.h>
  #include <string.h>
  #include <limits.h>
#else
  #include <linux/kernel.h>
  #include <linux/types.h>
  #include <linux/stddef.h>
  #include <linux/string.h>
  #define USHRT_MAX     65535
  /* #define UINT_MAX      4294967295U */
#endif

/* data type definitions */
#define U32 unsigned long
#define S32 signed long
#define I32 long
#define U16 unsigned short
#define S16 signed short
#define I16 short
#define U8 unsigned char
#define S8 signed char
#define I8 char

/*************************************/

/* lzo_swd.ch */

#define SWD_N				N
#define SWD_F				F
#define SWD_THRESHOLD		THRESHOLD

/* shortest unsigned int that 2 * SWD_F + SWD_N (currently 53248) fits in */
typedef unsigned short swd_uint;
/* upper limit of that data type */
#define SWD_UINT_MAX	USHRT_MAX

/* minilzo.c */

#define LZO_VERSION_DATE	"Jul 12 2002"
#define LZO_VERSION_STRING	"1.08"
#define LZO_VERSION			0x1080

/* lzo_ptr.h */

/* Integral types that have *exactly* the same number of bits as a lzo_voidp */
typedef unsigned long lzo_ptr_t;
typedef long lzo_sptr_t;


/*************************************/

/* config1x.h */

#define M1_MAX_OFFSET	0x0400
#define M2_MAX_OFFSET	0x0800
#define M3_MAX_OFFSET	0x4000
#define M4_MAX_OFFSET	0xbfff

#define MX_MAX_OFFSET	(M1_MAX_OFFSET + M2_MAX_OFFSET)

#define M1_MIN_LEN		2
#define M1_MAX_LEN		2
#define M2_MIN_LEN		3
#define M2_MAX_LEN		8
#define M3_MIN_LEN		3
#define M3_MAX_LEN		33
#define M4_MIN_LEN		3
#define M4_MAX_LEN		9

#define M1_MARKER		0
#define M2_MARKER		64
#define M3_MARKER		32
#define M4_MARKER		16

#define MIN_LOOKAHEAD		(M2_MAX_LEN + 1)

/* minilzo.c */

#define LZO_BYTE(x)       ((unsigned char) ((x) & 0xff))

#define LZO_MAX(a,b)        ((a) >= (b) ? (a) : (b))
#define LZO_MIN(a,b)        ((a) <= (b) ? (a) : (b))
#define LZO_MAX3(a,b,c)     ((a) >= (b) ? LZO_MAX(a,c) : LZO_MAX(b,c))
#define LZO_MIN3(a,b,c)     ((a) <= (b) ? LZO_MIN(a,c) : LZO_MIN(b,c))

#define lzo_sizeof(type)    ((lzo_uint) (sizeof(type)))

#define LZO_HIGH(array)     ((lzo_uint) (sizeof(array)/sizeof(*(array))))

#define LZO_SIZE(bits)      (1u << (bits))
#define LZO_MASK(bits)      (LZO_SIZE(bits) - 1)

#define LZO_LSIZE(bits)     (1ul << (bits))
#define LZO_LMASK(bits)     (LZO_LSIZE(bits) - 1)

#define LZO_USIZE(bits)     ((lzo_uint) 1 << (bits))
#define LZO_UMASK(bits)     (LZO_USIZE(bits) - 1)

#define LZO_STYPE_MAX(b)    (((1l  << (8*(b)-2)) - 1l)  + (1l  << (8*(b)-2)))
#define LZO_UTYPE_MAX(b)    (((1ul << (8*(b)-1)) - 1ul) + (1ul << (8*(b)-1)))

#define _LZO_STRINGIZE(x)           #x
#define _LZO_MEXPAND(x)             _LZO_STRINGIZE(x)

#define _LZO_CONCAT2(a,b)           a ## b
#define _LZO_CONCAT3(a,b,c)         a ## b ## c
#define _LZO_CONCAT4(a,b,c,d)       a ## b ## c ## d
#define _LZO_CONCAT5(a,b,c,d,e)     a ## b ## c ## d ## e

#define _LZO_ECONCAT2(a,b)          _LZO_CONCAT2(a,b)
#define _LZO_ECONCAT3(a,b,c)        _LZO_CONCAT3(a,b,c)
#define _LZO_ECONCAT4(a,b,c,d)      _LZO_CONCAT4(a,b,c,d)
#define _LZO_ECONCAT5(a,b,c,d,e)    _LZO_CONCAT5(a,b,c,d,e)

#define lzo_dict_t    const lzo_bytep
#define lzo_dict_p    lzo_dict_t *
#define lzo_moff_t    lzo_uint

#define MEMCPY8_DS(dest,src,len) \
    memcpy(dest,src,len); \
    dest += len; \
    src += len

#define MEMCPY_DS(dest,src,len) \
    do *dest++ = *src++; \
    while (--len > 0)

#define MEMMOVE_DS(dest,src,len) \
    do *dest++ = *src++; \
    while (--len > 0)

#define BZERO8_PTR(s,l,n)   memset((s),0,(lzo_uint)(l)*(n))

#define LZO_BASE 65521u
#define LZO_NMAX 5552

#define LZO_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define LZO_DO2(buf,i)  LZO_DO1(buf,i); LZO_DO1(buf,i+1);
#define LZO_DO4(buf,i)  LZO_DO2(buf,i); LZO_DO2(buf,i+2);
#define LZO_DO8(buf,i)  LZO_DO4(buf,i); LZO_DO4(buf,i+4);
#define LZO_DO16(buf,i) LZO_DO8(buf,i); LZO_DO8(buf,i+8);

#define IS_SIGNED(type)       (((type) (-1)) < ((type) 0))
#define IS_UNSIGNED(type)     (((type) (-1)) > ((type) 0))

#define IS_POWER_OF_2(x)        (((x) & ((x) - 1)) == 0)

#define D_BITS          14
#define D_INDEX1(d,p)       d = DM((0x21*DX3(p,5,5,6)) >> 5)
#define D_INDEX2(d,p)       d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)

#define LZO_HASH            LZO_HASH_LZO_INCREMENTAL_B

#define DL_MIN_LEN          M2_MIN_LEN

#define D_SIZE        LZO_SIZE(D_BITS)
#define D_MASK        LZO_MASK(D_BITS)

#define D_HIGH        ((D_MASK >> 1) + 1)

#define DINDEX1             D_INDEX1
#define DINDEX2             D_INDEX2

#define DX2(p,s1,s2) \
	(((((lzo_uint32)((p)[2]) << (s2)) ^ (p)[1]) << (s1)) ^ (p)[0])

#define DX3(p,s1,s2,s3) ((DX2((p)+1,s2,s3) << (s1)) ^ (p)[0])
#define DMS(v,s)        ((lzo_uint) (((v) & (D_MASK >> (s))) << (s)))
#define DM(v)           DMS(v,0)

#define DENTRY(p,in)                          (p)
#define GINDEX(m_pos,m_off,dict,dindex,in)    m_pos = dict[dindex]

#define LZO_CHECK_MPOS_DET(m_pos,m_off,in,ip,max_offset) \
	(m_pos == NULL || (m_off = (lzo_moff_t) (ip - m_pos)) > max_offset)

#define LZO_CHECK_MPOS_NON_DET(m_pos,m_off,in,ip,max_offset) \
    (BOUNDS_CHECKING_OFF_IN_EXPR( \
	(PTR_LT(m_pos,in) || \
	 (m_off = (lzo_moff_t) PTR_DIFF(ip,m_pos)) <= 0 || \
	  m_off > max_offset) ))

#define BOUNDS_CHECKING_OFF_IN_EXPR(expr)     (expr)

#define DD_BITS			0
#define DD_SIZE         LZO_SIZE(DD_BITS)
#define DD_MASK         LZO_MASK(DD_BITS)

#define DL_BITS        (D_BITS - DD_BITS)
#define DL_SIZE        LZO_SIZE(DL_BITS)
#define DL_MASK        LZO_MASK(DL_BITS)

#define UPDATE_D(dict,drun,dv,p,in)       dict[ DINDEX(dv,p) ] = DENTRY(p,in)
#define UPDATE_I(dict,drun,index,p,in)    dict[index] = DENTRY(p,in)
#define UPDATE_P(ptr,drun,p,in)           (ptr)[0] = DENTRY(p,in)

#define __COPY4(dst,src)  * (lzo_uint32p)(dst) = * (const lzo_uint32p)(src)
#define COPY4(dst,src)	__COPY4((lzo_ptr_t)(dst),(lzo_ptr_t)(src))

#define TEST_IP         (ip < ip_end)
#define TEST_OP         (op <= op_end)

#define NEED_IP(x) \
            if ((lzo_uint)(ip_end - ip) < (lzo_uint)(x))  goto input_overrun
#define NEED_OP(x) \
            if ((lzo_uint)(op_end - op) < (lzo_uint)(x))  goto output_overrun
#define TEST_LOOKBEHIND(m_pos,out)    if (m_pos < out) goto lookbehind_overrun

/* lzo1x_9x.c */

#define LZO_UINT_MAX	UINT_MAX
#define N			M4_MAX_OFFSET
#define THRESHOLD	    1
#define F		     2048

#define SWD_BEST_OFF	(LZO_MAX3( M2_MAX_LEN, M3_MAX_LEN, M4_MAX_LEN ) + 1)

/* ../include/lzoconf.h */

typedef U32 lzo_uint32;
typedef I32 lzo_int32;
typedef U32 lzo_uint;
typedef I32 lzo_int;
typedef int lzo_bool;

#define lzo_byte                U8
#define lzo_bytep               U8 *
#define lzo_charp               char *
#define lzo_voidp               void *
#define lzo_shortp              short *
#define lzo_ushortp             unsigned short *
#define lzo_uint32p             lzo_uint32 *
#define lzo_int32p              lzo_int32 *
#define lzo_uintp               lzo_uint *
#define lzo_intp                lzo_int *
#define lzo_voidpp              lzo_voidp *
#define lzo_bytepp              lzo_bytep *
#define lzo_sizeof_dict_t		sizeof(lzo_bytep)

#define LZO_E_OK                    0
#define LZO_E_ERROR                 (-1)
#define LZO_E_OUT_OF_MEMORY         (-2)	/* not used right now */
#define LZO_E_NOT_COMPRESSIBLE      (-3)	/* not used right now */
#define LZO_E_INPUT_OVERRUN         (-4)
#define LZO_E_OUTPUT_OVERRUN        (-5)
#define LZO_E_LOOKBEHIND_OVERRUN    (-6)
#define LZO_E_EOF_NOT_FOUND         (-7)
#define LZO_E_INPUT_NOT_CONSUMED    (-8)

#define LZO_PTR_ALIGN_UP(_ptr,_size) \
   ((_ptr) + (lzo_uint) __lzo_align_gap((const lzo_voidp)(_ptr),(lzo_uint)(_size)))
#define LZO_ALIGN(_ptr,_size) LZO_PTR_ALIGN_UP(_ptr,_size)

typedef int
	(*lzo_compress_t) (const lzo_byte * src, lzo_uint src_len,
			   lzo_byte * dst, lzo_uintp dst_len,
			   lzo_voidp wrkmem);

typedef int
	(*lzo_decompress_t) (const lzo_byte * src, lzo_uint src_len,
			     lzo_byte * dst, lzo_uintp dst_len,
			     lzo_voidp wrkmem);

typedef int
	(*lzo_optimize_t) (lzo_byte * src, lzo_uint src_len,
			   lzo_byte * dst, lzo_uintp dst_len,
			   lzo_voidp wrkmem);

typedef int
	(*lzo_compress_dict_t) (const lzo_byte * src, lzo_uint src_len,
				lzo_byte * dst, lzo_uintp dst_len,
				lzo_voidp wrkmem,
				const lzo_byte * dict, lzo_uint dict_len);

typedef int
	(*lzo_decompress_dict_t) (const lzo_byte * src, lzo_uint src_len,
				  lzo_byte * dst, lzo_uintp dst_len,
				  lzo_voidp wrkmem,
				  const lzo_byte * dict, lzo_uint dict_len);

typedef int
	(*lzo_compress_asm_t) (const lzo_byte * src, lzo_uint src_len,
			       lzo_byte * dst, lzo_uintp dst_len,
			       lzo_voidp wrkmem);

typedef int
	(*lzo_decompress_asm_t) (const lzo_byte * src, lzo_uint src_len,
				 lzo_byte * dst, lzo_uintp dst_len,
				 lzo_voidp wrkmem);

typedef void (*lzo_progress_callback_t) (lzo_uint, lzo_uint);

typedef union
{
	lzo_bytep p;
	lzo_uint u;
} __lzo_pu_u;
typedef union
{
	lzo_bytep p;
	lzo_uint32 u32;
} __lzo_pu32_u;
typedef union
{
	void *vp;
	lzo_bytep bp;
	lzo_uint32 u32;
	long l;
} lzo_align_t;

/* lzo1x.h */

#define LZO1X_1_MEM_COMPRESS    ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1X_999_MEM_COMPRESS  ((lzo_uint32) (14 * 16384L * sizeof(short)))

/* lzo_ptr.h */

#define PTR(a)				((lzo_ptr_t) (a))
#define PTR_LINEAR(a)		PTR(a)
#define PTR_ALIGNED_4(a)	((PTR_LINEAR(a) & 3) == 0)
#define PTR_ALIGNED_8(a)	((PTR_LINEAR(a) & 7) == 0)
#define PTR_ALIGNED2_4(a,b)	(((PTR_LINEAR(a) | PTR_LINEAR(b)) & 3) == 0)
#define PTR_ALIGNED2_8(a,b)	(((PTR_LINEAR(a) | PTR_LINEAR(b)) & 7) == 0)
#define PTR_LT(a,b)			(PTR(a) < PTR(b))
#define PTR_GE(a,b)			(PTR(a) >= PTR(b))
#define PTR_DIFF(a,b)		((lzo_ptrdiff_t) (PTR(a) - PTR(b)))
#define pd(a,b)		        ((lzo_uint) ((a)-(b)))

typedef ptrdiff_t lzo_ptrdiff_t;

typedef union
{
	char a_char;
	unsigned char a_uchar;
	short a_short;
	unsigned short a_ushort;
	int a_int;
	unsigned int a_uint;
	long a_long;
	unsigned long a_ulong;
	lzo_int a_lzo_int;
	lzo_uint a_lzo_uint;
	lzo_int32 a_lzo_int32;
	lzo_uint32 a_lzo_uint32;
	ptrdiff_t a_ptrdiff_t;
	lzo_ptrdiff_t a_lzo_ptrdiff_t;
	lzo_ptr_t a_lzo_ptr_t;
	lzo_voidp a_lzo_voidp;
	void *a_void_p;
	lzo_bytep a_lzo_bytep;
	lzo_bytepp a_lzo_bytepp;
	lzo_uintp a_lzo_uintp;
	lzo_uint *a_lzo_uint_p;
	lzo_uint32p a_lzo_uint32p;
	lzo_uint32 *a_lzo_uint32_p;
	unsigned char *a_uchar_p;
	char *a_char_p;
}
lzo_full_align_t;

/* lzo_mchw.ch */

typedef struct
{
	int init;

	lzo_uint look;

	lzo_uint m_len;
	lzo_uint m_off;

	lzo_uint last_m_len;
	lzo_uint last_m_off;

	const lzo_byte *bp;
	const lzo_byte *ip;
	const lzo_byte *in;
	const lzo_byte *in_end;
	lzo_byte *out;

	lzo_progress_callback_t cb;

	lzo_uint textsize;
	lzo_uint codesize;
	lzo_uint printcount;

	unsigned long lit_bytes;
	unsigned long match_bytes;
	unsigned long rep_bytes;
	unsigned long lazy;

	lzo_uint r1_lit;
	lzo_uint r1_m_len;

	unsigned long m1a_m, m1b_m, m2_m, m3_m, m4_m;
	unsigned long lit1_r, lit2_r, lit3_r;
}
lzo1x_999_t;

#define getbyte(c) 	((c).ip < (c).in_end ? *((c).ip)++ : (-1))

/* lzo_swd.ch */

#define SWD_UINT(x)			((swd_uint)(x))
#define SWD_HSIZE			16384
#define SWD_MAX_CHAIN		2048
#define HEAD3(b,p) \
	(((0x9f5f*(((((lzo_uint32)b[p]<<5)^b[p+1])<<5)^b[p+2]))>>5) & (SWD_HSIZE-1))
#define HEAD2(b,p)      (b[p] ^ ((unsigned)b[p+1]<<8))
#define NIL2				SWD_UINT_MAX

typedef struct
{
	lzo_uint n;
	lzo_uint f;
	lzo_uint threshold;

	lzo_uint max_chain;
	lzo_uint nice_length;
	lzo_bool use_best_off;
	lzo_uint lazy_insert;

	lzo_uint m_len;
	lzo_uint m_off;
	lzo_uint look;
	int b_char;

	lzo_uint best_off[SWD_BEST_OFF];

	lzo1x_999_t *c;
	lzo_uint m_pos;

	lzo_uint best_pos[SWD_BEST_OFF];

	const lzo_byte *dict;
	const lzo_byte *dict_end;
	lzo_uint dict_len;

	lzo_uint ip;
	lzo_uint bp;
	lzo_uint rp;
	lzo_uint b_size;

	unsigned char *b_wrap;

	lzo_uint node_count;
	lzo_uint first_rp;

	unsigned char b[SWD_N + SWD_F + SWD_F];
	swd_uint head3[SWD_HSIZE];
	swd_uint succ3[SWD_N + SWD_F];
	swd_uint best3[SWD_N + SWD_F];
	swd_uint llen3[SWD_HSIZE];

	swd_uint head2[65536L];
}
lzo1x_999_swd_t;

#define s_head3(s,key)		s->head3[key]
#define swd_pos2off(s,pos) \
	(s->bp > (pos) ? s->bp - (pos) : s->b_size - ((pos) - s->bp))

static __inline__ void
swd_getbyte (lzo1x_999_swd_t * s)
{
	int c;

	if ((c = getbyte (*(s->c))) < 0)
	{
		if (s->look > 0)
			--s->look;
	}
	else
	{
		s->b[s->ip] = LZO_BYTE (c);
		if (s->ip < s->f)
			s->b_wrap[s->ip] = LZO_BYTE (c);
	}
	if (++s->ip == s->b_size)
		s->ip = 0;
	if (++s->bp == s->b_size)
		s->bp = 0;
	if (++s->rp == s->b_size)
		s->rp = 0;
}

static void
swd_initdict (lzo1x_999_swd_t * s, const lzo_byte * dict, lzo_uint dict_len)
{
	s->dict = s->dict_end = NULL;
	s->dict_len = 0;

	if (!dict || dict_len <= 0)
		return;
	if (dict_len > s->n)
	{
		dict += dict_len - s->n;
		dict_len = s->n;
	}

	s->dict = dict;
	s->dict_len = dict_len;
	s->dict_end = dict + dict_len;
	memcpy (s->b, dict, dict_len);
	s->ip = dict_len;
}

static void
swd_insertdict (lzo1x_999_swd_t * s, lzo_uint node, lzo_uint len)
{
	lzo_uint key;

	s->node_count = s->n - len;
	s->first_rp = node;

	while (len-- > 0)
	{
		key = HEAD3 (s->b, node);
		s->succ3[node] = s_head3 (s, key);
		s->head3[key] = SWD_UINT (node);
		s->best3[node] = SWD_UINT (s->f + 1);
		s->llen3[key]++;

		key = HEAD2 (s->b, node);
		s->head2[key] = SWD_UINT (node);

		node++;
	}
}

static int
swd_init (lzo1x_999_swd_t * s, const lzo_byte * dict, lzo_uint dict_len)
{

	s->n = SWD_N;
	s->f = SWD_F;
	s->threshold = SWD_THRESHOLD;



	s->max_chain = SWD_MAX_CHAIN;
	s->nice_length = SWD_F;
	s->use_best_off = 0;
	s->lazy_insert = 0;

	s->b_size = s->n + s->f;
	if (2 * s->f >= s->n || s->b_size + s->f >= NIL2)
		return LZO_E_ERROR;
	s->b_wrap = s->b + s->b_size;
	s->node_count = s->n;

	memset (s->llen3, 0, sizeof (s->llen3[0]) * SWD_HSIZE);
	memset (s->head2, 0xff, sizeof (s->head2[0]) * 65536L);

	s->ip = 0;
	swd_initdict (s, dict, dict_len);
	s->bp = s->ip;
	s->first_rp = s->ip;

	s->look = (lzo_uint) (s->c->in_end - s->c->ip);
	if (s->look > 0)
	{
		if (s->look > s->f)
			s->look = s->f;
		memcpy (&s->b[s->ip], s->c->ip, s->look);
		s->c->ip += s->look;
		s->ip += s->look;
	}

	if (s->ip == s->b_size)
		s->ip = 0;

	if (s->look >= 2 && s->dict_len > 0)
		swd_insertdict (s, 0, s->dict_len);

	s->rp = s->first_rp;
	if (s->rp >= s->node_count)
		s->rp -= s->node_count;
	else
		s->rp += s->b_size - s->node_count;

	return LZO_E_OK;
}

static __inline__ void
swd_remove_node (lzo1x_999_swd_t * s, lzo_uint node)
{
	if (s->node_count == 0)
	{
		lzo_uint key;

		key = HEAD3 (s->b, node);

		--s->llen3[key];

		key = HEAD2 (s->b, node);

		if ((lzo_uint) s->head2[key] == node)
			s->head2[key] = NIL2;
	}
	else
		--s->node_count;
}

static void
swd_accept (lzo1x_999_swd_t * s, lzo_uint n)
{

	while (n--)
	{
		lzo_uint key;

		swd_remove_node (s, s->rp);

		key = HEAD3 (s->b, s->bp);
		s->succ3[s->bp] = s_head3 (s, key);
		s->head3[key] = SWD_UINT (s->bp);
		s->best3[s->bp] = SWD_UINT (s->f + 1);
		s->llen3[key]++;

		key = HEAD2 (s->b, s->bp);
		s->head2[key] = SWD_UINT (s->bp);;

		swd_getbyte (s);
	}
}

static void
swd_search (lzo1x_999_swd_t * s, lzo_uint node, lzo_uint cnt)
{
	const unsigned char *p1;
	const unsigned char *p2;
	const unsigned char *px;

	lzo_uint m_len = s->m_len;
	const unsigned char *b = s->b;
	const unsigned char *bp = s->b + s->bp;
	const unsigned char *bx = s->b + s->bp + s->look;
	unsigned char scan_end1;

	scan_end1 = bp[m_len - 1];
	for (; cnt-- > 0; node = s->succ3[node])
	{
		p1 = bp;
		p2 = b + node;
		px = bx;

		if (p2[m_len - 1] == scan_end1 &&
		    p2[m_len] == p1[m_len] &&
		    p2[0] == p1[0] && p2[1] == p1[1])
		{
			lzo_uint i;

			p1 += 2;
			p2 += 2;
			do
			{
			}
			while (++p1 < px && *p1 == *++p2);

			i = p1 - bp;

			if (i < SWD_BEST_OFF)
			{
				if (s->best_pos[i] == 0)
					s->best_pos[i] = node + 1;
			}

			if (i > m_len)
			{
				s->m_len = m_len = i;
				s->m_pos = node;
				if (m_len == s->look)
					return;
				if (m_len >= s->nice_length)
					return;
				if (m_len > (lzo_uint) s->best3[node])
					return;
				scan_end1 = bp[m_len - 1];
			}
		}
	}
}

static lzo_bool
swd_search2 (lzo1x_999_swd_t * s)
{
	lzo_uint key;

	key = s->head2[HEAD2 (s->b, s->bp)];
	if (key == NIL2)
		return 0;

	if (s->best_pos[2] == 0)
		s->best_pos[2] = key + 1;

	if (s->m_len < 2)
	{
		s->m_len = 2;
		s->m_pos = key;
	}
	return 1;
}

static void
swd_findbest (lzo1x_999_swd_t * s)
{
	lzo_uint key;
	lzo_uint cnt, node;
	lzo_uint len;

	key = HEAD3 (s->b, s->bp);
	node = s->succ3[s->bp] = s_head3 (s, key);
	cnt = s->llen3[key]++;

	if (cnt > s->max_chain && s->max_chain > 0)
		cnt = s->max_chain;
	s->head3[key] = SWD_UINT (s->bp);

	s->b_char = s->b[s->bp];
	len = s->m_len;
	if (s->m_len >= s->look)
	{
		if (s->look == 0)
			s->b_char = -1;
		s->m_off = 0;
		s->best3[s->bp] = SWD_UINT (s->f + 1);
	}
	else
	{

		if (swd_search2 (s))

			if (s->look >= 3)
				swd_search (s, node, cnt);
		if (s->m_len > len)
			s->m_off = swd_pos2off (s, s->m_pos);
		s->best3[s->bp] = SWD_UINT (s->m_len);

		if (s->use_best_off)
		{
			int i;
			for (i = 2; i < SWD_BEST_OFF; i++)
				if (s->best_pos[i] > 0)
					s->best_off[i] =
						swd_pos2off (s,
							     s->best_pos[i] -
							     1);
				else
					s->best_off[i] = 0;
		}

	}

	swd_remove_node (s, s->rp);

	key = HEAD2 (s->b, s->bp);
	s->head2[key] = SWD_UINT (s->bp);

}

/* lzo_mchw.ch */

static int
init_match (lzo1x_999_t * c, lzo1x_999_swd_t * s,
	    const lzo_byte * dict, lzo_uint dict_len, lzo_uint32 flags)
{
	int r;

	c->init = 1;

	s->c = c;

	c->last_m_len = c->last_m_off = 0;

	c->textsize = c->codesize = c->printcount = 0;
	c->lit_bytes = c->match_bytes = c->rep_bytes = 0;
	c->lazy = 0;

	r = swd_init (s, dict, dict_len);
	if (r != 0)
		return r;

	s->use_best_off = (flags & 1) ? 1 : 0;
	return r;
}

static int
find_match (lzo1x_999_t * c, lzo1x_999_swd_t * s,
	    lzo_uint this_len, lzo_uint skip)
{
	if (skip > 0)
	{
		swd_accept (s, this_len - skip);
		c->textsize += this_len - skip + 1;
	}
	else
	{
		c->textsize += this_len - skip;
	}

	s->m_len = 1;
	s->m_len = 1;

	if (s->use_best_off)
		memset (s->best_pos, 0, sizeof (s->best_pos));

	swd_findbest (s);
	c->m_len = s->m_len;
	c->m_off = s->m_off;

	swd_getbyte (s);

	if (s->b_char < 0)
	{
		c->look = 0;
		c->m_len = 0;
	}
	else
	{
		c->look = s->look + 1;
	}
	c->bp = c->ip - c->look;

	if (c->cb && c->textsize > c->printcount)
	{
		(*c->cb) (c->textsize, c->codesize);
		c->printcount += 1024;
	}

	return LZO_E_OK;
}

/* lzo1x_9x.c */

static lzo_byte *
code_match (lzo1x_999_t * c, lzo_byte * op, lzo_uint m_len, lzo_uint m_off)
{
	lzo_uint x_len = m_len;
	lzo_uint x_off = m_off;

	c->match_bytes += m_len;

	if (m_len == 2)
	{
		m_off -= 1;

		*op++ = LZO_BYTE (M1_MARKER | ((m_off & 3) << 2));
		*op++ = LZO_BYTE (m_off >> 2);

		c->m1a_m++;
	}

	else if (m_len <= M2_MAX_LEN && m_off <= M2_MAX_OFFSET)

	{

		m_off -= 1;
		*op++ = LZO_BYTE (((m_len - 1) << 5) | ((m_off & 7) << 2));
		*op++ = LZO_BYTE (m_off >> 3);
		c->m2_m++;
	}
	else if (m_len == M2_MIN_LEN && m_off <= MX_MAX_OFFSET
		 && c->r1_lit >= 4)
	{
		m_off -= 1 + M2_MAX_OFFSET;

		*op++ = LZO_BYTE (M1_MARKER | ((m_off & 3) << 2));
		*op++ = LZO_BYTE (m_off >> 2);

		c->m1b_m++;
	}
	else if (m_off <= M3_MAX_OFFSET)
	{
		m_off -= 1;
		if (m_len <= M3_MAX_LEN)
			*op++ = LZO_BYTE (M3_MARKER | (m_len - 2));
		else
		{
			m_len -= M3_MAX_LEN;
			*op++ = M3_MARKER | 0;
			while (m_len > 255)
			{
				m_len -= 255;
				*op++ = 0;
			}
			*op++ = LZO_BYTE (m_len);
		}

		*op++ = LZO_BYTE (m_off << 2);
		*op++ = LZO_BYTE (m_off >> 6);

		c->m3_m++;
	}
	else
	{
		lzo_uint k;

		m_off -= 0x4000;
		k = (m_off & 0x4000) >> 11;
		if (m_len <= M4_MAX_LEN)
			*op++ = LZO_BYTE (M4_MARKER | k | (m_len - 2));
		else
		{
			m_len -= M4_MAX_LEN;
			*op++ = LZO_BYTE (M4_MARKER | k | 0);
			while (m_len > 255)
			{
				m_len -= 255;
				*op++ = 0;
			}
			*op++ = LZO_BYTE (m_len);
		}

		*op++ = LZO_BYTE (m_off << 2);
		*op++ = LZO_BYTE (m_off >> 6);

		c->m4_m++;
	}

	c->last_m_len = x_len;
	c->last_m_off = x_off;
	return op;
}

static lzo_byte *
STORE_RUN (lzo1x_999_t * c, lzo_byte * op, const lzo_byte * ii, lzo_uint t)
{
	c->lit_bytes += t;

	if (op == c->out && t <= 238)
	{
		*op++ = LZO_BYTE (17 + t);
	}
	else if (t <= 3)
	{
		op[-2] |= LZO_BYTE (t);

		c->lit1_r++;
	}
	else if (t <= 18)
	{
		*op++ = LZO_BYTE (t - 3);
		c->lit2_r++;
	}
	else
	{
		lzo_uint tt = t - 18;

		*op++ = 0;
		while (tt > 255)
		{
			tt -= 255;
			*op++ = 0;
		}
		*op++ = LZO_BYTE (tt);
		c->lit3_r++;
	}
	do
		*op++ = *ii++;
	while (--t > 0);

	return op;
}

static lzo_byte *
code_run (lzo1x_999_t * c, lzo_byte * op, const lzo_byte * ii,
	  lzo_uint lit, lzo_uint m_len)
{
	if (lit > 0)
	{
		op = STORE_RUN (c, op, ii, lit);
		c->r1_m_len = m_len;
		c->r1_lit = lit;
	}
	else
	{
		c->r1_m_len = 0;
		c->r1_lit = 0;
	}

	return op;
}

static int
len_of_coded_match (lzo_uint m_len, lzo_uint m_off, lzo_uint lit)
{
	int n = 4;

	if (m_len < 2)
		return -1;
	if (m_len == 2)
		return (m_off <= M1_MAX_OFFSET && lit > 0
			&& lit < 4) ? 2 : -1;
	if (m_len <= M2_MAX_LEN && m_off <= M2_MAX_OFFSET)
		return 2;
	if (m_len == M2_MIN_LEN && m_off <= MX_MAX_OFFSET && lit >= 4)
		return 2;
	if (m_off <= M3_MAX_OFFSET)
	{
		if (m_len <= M3_MAX_LEN)
			return 3;
		m_len -= M3_MAX_LEN;
		while (m_len > 255)
		{
			m_len -= 255;
			n++;
		}
		return n;
	}
	if (m_off <= M4_MAX_OFFSET)
	{
		if (m_len <= M4_MAX_LEN)
			return 3;
		m_len -= M4_MAX_LEN;
		while (m_len > 255)
		{
			m_len -= 255;
			n++;
		}
		return n;
	}
	return -1;
}

static lzo_int
min_gain (lzo_uint ahead, lzo_uint lit1, lzo_uint lit2, int l1, int l2,
	  int l3)
{
	lzo_int lazy_match_min_gain = 0;

	lazy_match_min_gain += ahead;

	if (lit1 <= 3)
		lazy_match_min_gain += (lit2 <= 3) ? 0 : 2;
	else if (lit1 <= 18)
		lazy_match_min_gain += (lit2 <= 18) ? 0 : 1;

	lazy_match_min_gain += (l2 - l1) * 2;
	if (l3 > 0)
		lazy_match_min_gain -= (ahead - l3) * 2;

	if (lazy_match_min_gain < 0)
		lazy_match_min_gain = 0;

	return lazy_match_min_gain;
}

static void
better_match (const lzo1x_999_swd_t * swd, lzo_uint * m_len, lzo_uint * m_off)
{
	if (*m_len <= M2_MIN_LEN)
		return;

	if (*m_off <= M2_MAX_OFFSET)
		return;

	if (*m_off > M2_MAX_OFFSET &&
	    *m_len >= M2_MIN_LEN + 1 && *m_len <= M2_MAX_LEN + 1 &&
	    swd->best_off[*m_len - 1]
	    && swd->best_off[*m_len - 1] <= M2_MAX_OFFSET)
	{
		*m_len = *m_len - 1;
		*m_off = swd->best_off[*m_len];
		return;
	}

	if (*m_off > M3_MAX_OFFSET &&
	    *m_len >= M4_MAX_LEN + 1 && *m_len <= M2_MAX_LEN + 2 &&
	    swd->best_off[*m_len - 2]
	    && swd->best_off[*m_len - 2] <= M2_MAX_OFFSET)
	{
		*m_len = *m_len - 2;
		*m_off = swd->best_off[*m_len];
		return;
	}

	if (*m_off > M3_MAX_OFFSET &&
	    *m_len >= M4_MAX_LEN + 1 && *m_len <= M3_MAX_LEN + 1 &&
	    swd->best_off[*m_len - 1]
	    && swd->best_off[*m_len - 1] <= M3_MAX_OFFSET)
	{
		*m_len = *m_len - 1;
		*m_off = swd->best_off[*m_len];
	}

}

/* minilzo.c */

static lzo_bool
lzo_assert (int expr)
{
	return (expr) ? 1 : 0;
}

/* lzo1x_9x.c */

static int
lzo1x_999_compress_internal (const lzo_byte * in, lzo_uint in_len,
			     lzo_byte * out, lzo_uintp out_len,
			     lzo_voidp wrkmem,
			     const lzo_byte * dict, lzo_uint dict_len,
			     lzo_progress_callback_t cb,
			     int try_lazy,
			     lzo_uint good_length,
			     lzo_uint max_lazy,
			     lzo_uint nice_length,
			     lzo_uint max_chain, lzo_uint32 flags)
{
	lzo_byte *op;
	const lzo_byte *ii;
	lzo_uint lit;
	lzo_uint m_len, m_off;
	lzo1x_999_t cc;
	lzo1x_999_t *const c = &cc;
	lzo1x_999_swd_t *const swd = (lzo1x_999_swd_t *) wrkmem;
	int r;

	if (!lzo_assert
	    (LZO1X_999_MEM_COMPRESS >= lzo_sizeof (lzo1x_999_swd_t)))
		return LZO_E_ERROR;

	if (try_lazy < 0)
		try_lazy = 1;

	if (good_length <= 0)
		good_length = 32;

	if (max_lazy <= 0)
		max_lazy = 32;

	if (nice_length <= 0)
		nice_length = 0;

	if (max_chain <= 0)
		max_chain = SWD_MAX_CHAIN;

	c->init = 0;
	c->ip = c->in = in;
	c->in_end = in + in_len;
	c->out = out;
	c->cb = cb;
	c->m1a_m = c->m1b_m = c->m2_m = c->m3_m = c->m4_m = 0;
	c->lit1_r = c->lit2_r = c->lit3_r = 0;

	op = out;
	ii = c->ip;
	lit = 0;
	c->r1_lit = c->r1_m_len = 0;

	r = init_match (c, swd, dict, dict_len, flags);
	if (r != 0)
		return r;
	if (max_chain > 0)
		swd->max_chain = max_chain;
	if (nice_length > 0)
		swd->nice_length = nice_length;

	r = find_match (c, swd, 0, 0);
	if (r != 0)
		return r;
	while (c->look > 0)
	{
		lzo_uint ahead;
		lzo_uint max_ahead;
		int l1, l2, l3;

		c->codesize = op - out;

		m_len = c->m_len;
		m_off = c->m_off;

		if (lit == 0)
			ii = c->bp;

		if (m_len < 2 ||
		    (m_len == 2
		     && (m_off > M1_MAX_OFFSET || lit == 0 || lit >= 4))
		    || (m_len == 2 && op == out) || (op == out && lit == 0))
		{

			m_len = 0;
		}
		else if (m_len == M2_MIN_LEN)
		{

			if (m_off > MX_MAX_OFFSET && lit >= 4)
				m_len = 0;
		}

		if (m_len == 0)
		{

			lit++;
			swd->max_chain = max_chain;
			r = find_match (c, swd, 1, 0);
			continue;
		}

		if (swd->use_best_off)
			better_match (swd, &m_len, &m_off);

		ahead = 0;
		if (try_lazy <= 0 || m_len >= max_lazy)
		{

			l1 = 0;
			max_ahead = 0;
		}
		else
		{

			l1 = len_of_coded_match (m_len, m_off, lit);

			max_ahead = LZO_MIN (try_lazy, l1 - 1);

		}

		while (ahead < max_ahead && c->look > m_len)
		{
			lzo_int lazy_match_min_gain;

			if (m_len >= good_length)
				swd->max_chain = max_chain >> 2;
			else
				swd->max_chain = max_chain;
			r = find_match (c, swd, 1, 0);
			ahead++;

			if (c->m_len < m_len)
				continue;

			if (c->m_len == m_len && c->m_off >= m_off)
				continue;

			if (swd->use_best_off)
				better_match (swd, &c->m_len, &c->m_off);

			l2 = len_of_coded_match (c->m_len, c->m_off,
						 lit + ahead);
			if (l2 < 0)
				continue;

			l3 = (op == out) ? -1 : len_of_coded_match (ahead,
								    m_off,
								    lit);

			lazy_match_min_gain =
				min_gain (ahead, lit, lit + ahead, l1, l2,
					  l3);
			if (c->m_len >= m_len + lazy_match_min_gain)
			{
				c->lazy++;

				if (l3 > 0)
				{

					op = code_run (c, op, ii, lit, ahead);
					lit = 0;

					op = code_match (c, op, ahead, m_off);
				}
				else
				{
					lit += ahead;
				}
				goto lazy_match_done;
			}
		}

		op = code_run (c, op, ii, lit, m_len);
		lit = 0;

		op = code_match (c, op, m_len, m_off);
		swd->max_chain = max_chain;
		r = find_match (c, swd, m_len, 1 + ahead);

	      lazy_match_done:;
	}

	if (lit > 0)
		op = STORE_RUN (c, op, ii, lit);

	*op++ = M4_MARKER | 1;
	*op++ = 0;
	*op++ = 0;

	c->codesize = op - out;

	*out_len = op - out;

	if (c->cb)
		(*c->cb) (c->textsize, c->codesize);

	return LZO_E_OK;
}

static int
lzo1x_999_compress_level (const lzo_byte * in, lzo_uint in_len,
			  lzo_byte * out, lzo_uintp out_len,
			  lzo_voidp wrkmem,
			  const lzo_byte * dict, lzo_uint dict_len,
			  lzo_progress_callback_t cb, int compression_level)
{
	static const struct
	{
		int try_lazy;
		lzo_uint good_length;
		lzo_uint max_lazy;
		lzo_uint nice_length;
		lzo_uint max_chain;
		lzo_uint32 flags;
	} c[9] =
	{
		{
		0, 0, 0, 8, 4, 0},
		{
		0, 0, 0, 16, 8, 0},
		{
		0, 0, 0, 32, 16, 0},
		{
		1, 4, 4, 16, 16, 0},
		{
		1, 8, 16, 32, 32, 0},
		{
		1, 8, 16, 128, 128, 0},
		{
		2, 8, 32, 128, 256, 0},
		{
		2, 32, 128, F, 2048, 1},
		{
		2, F, F, F, 4096, 1}
	};

	if (compression_level < 1 || compression_level > 9)
		return LZO_E_ERROR;

	compression_level -= 1;
	return lzo1x_999_compress_internal (in, in_len, out, out_len, wrkmem,
					    dict, dict_len, cb,
					    c[compression_level].try_lazy,
					    c[compression_level].good_length,
					    c[compression_level].max_lazy,
					    0,
					    c[compression_level].max_chain,
					    c[compression_level].flags);
}

static int
lzo1x_999_compress (const lzo_byte * in, lzo_uint in_len,
		    lzo_byte * out, lzo_uintp out_len, lzo_voidp wrkmem)
{
	return lzo1x_999_compress_level (in, in_len, out, out_len, wrkmem,
					 NULL, 0, 0, 8);
}

/* minilzo.c */

static const lzo_byte __lzo_copyright[] = LZO_VERSION_STRING;

static lzo_uint
_lzo1x_1_do_compress (const lzo_byte * in, lzo_uint in_len,
		      lzo_byte * out, lzo_uintp out_len, lzo_voidp wrkmem)
{

	register const lzo_byte *ip;

	lzo_byte *op;
	const lzo_byte *const in_end = in + in_len;
	const lzo_byte *const ip_end = in + in_len - 8 - 5;
	const lzo_byte *ii;
	lzo_dict_p const dict = (lzo_dict_p) wrkmem;

	op = out;
	ip = in;
	ii = ip;

	ip += 4;
	for (;;)
	{
		register const lzo_byte *m_pos;

		lzo_uint m_off;
		lzo_uint m_len;
		lzo_uint dindex;

		DINDEX1 (dindex, ip);
		GINDEX (m_pos, m_off, dict, dindex, in);
		if (LZO_CHECK_MPOS_NON_DET
		    (m_pos, m_off, in, ip, M4_MAX_OFFSET))
			goto literal;

		if (m_off <= M2_MAX_OFFSET || m_pos[3] == ip[3])
			goto try_match;
		DINDEX2 (dindex, ip);
		GINDEX (m_pos, m_off, dict, dindex, in);

		if (LZO_CHECK_MPOS_NON_DET
		    (m_pos, m_off, in, ip, M4_MAX_OFFSET))
			goto literal;
		if (m_off <= M2_MAX_OFFSET || m_pos[3] == ip[3])
			goto try_match;
		goto literal;

	      try_match:
		if (m_pos[0] != ip[0] || m_pos[1] != ip[1])
		{
		}
		else
		{
			if (m_pos[2] == ip[2])
			{
				goto match;
			}
			else
			{
			}
		}

	      literal:
		UPDATE_I (dict, 0, dindex, ip, in);
		++ip;
		if (ip >= ip_end)
			break;
		continue;

	      match:
		UPDATE_I (dict, 0, dindex, ip, in);

		if (pd (ip, ii) > 0)
		{
			register lzo_uint t = pd (ip, ii);

			if (t <= 3)
			{
				op[-2] |= LZO_BYTE (t);
			}
			else if (t <= 18)
				*op++ = LZO_BYTE (t - 3);
			else
			{
				register lzo_uint tt = t - 18;

				*op++ = 0;
				while (tt > 255)
				{
					tt -= 255;
					*op++ = 0;
				}
				*op++ = LZO_BYTE (tt);;
			}
			do
				*op++ = *ii++;
			while (--t > 0);
		}

		ip += 3;
		if (m_pos[3] != *ip++ || m_pos[4] != *ip++
		    || m_pos[5] != *ip++ || m_pos[6] != *ip++
		    || m_pos[7] != *ip++ || m_pos[8] != *ip++)
		{
			--ip;
			m_len = ip - ii;

			if (m_off <= M2_MAX_OFFSET)
			{
				m_off -= 1;

				*op++ = LZO_BYTE (((m_len -
						    1) << 5) | ((m_off & 7) <<
								2));
				*op++ = LZO_BYTE (m_off >> 3);
			}
			else if (m_off <= M3_MAX_OFFSET)
			{
				m_off -= 1;
				*op++ = LZO_BYTE (M3_MARKER | (m_len - 2));
				goto m3_m4_offset;
			}
			else

			{
				m_off -= 0x4000;

				*op++ = LZO_BYTE (M4_MARKER |
						  ((m_off & 0x4000) >> 11) |
						  (m_len - 2));
				goto m3_m4_offset;
			}
		}
		else
		{
			{
				const lzo_byte *end = in_end;
				const lzo_byte *m = m_pos + M2_MAX_LEN + 1;
				while (ip < end && *m == *ip)
					m++, ip++;
				m_len = (ip - ii);
			}


			if (m_off <= M3_MAX_OFFSET)
			{
				m_off -= 1;
				if (m_len <= 33)
					*op++ = LZO_BYTE (M3_MARKER |
							  (m_len - 2));
				else
				{
					m_len -= 33;
					*op++ = M3_MARKER | 0;
					goto m3_m4_len;
				}
			}
			else
			{
				m_off -= 0x4000;

				if (m_len <= M4_MAX_LEN)
					*op++ = LZO_BYTE (M4_MARKER |
							  ((m_off & 0x4000) >>
							   11) | (m_len - 2));

				else
				{
					m_len -= M4_MAX_LEN;
					*op++ = LZO_BYTE (M4_MARKER |
							  ((m_off & 0x4000) >>
							   11));
				      m3_m4_len:
					while (m_len > 255)
					{
						m_len -= 255;
						*op++ = 0;
					}

					*op++ = LZO_BYTE (m_len);
				}
			}

		      m3_m4_offset:
			*op++ = LZO_BYTE ((m_off & 63) << 2);
			*op++ = LZO_BYTE (m_off >> 6);
		}
		ii = ip;
		if (ip >= ip_end)
			break;
	}

	*out_len = op - out;
	return pd (in_end, ii);
}

static int
lzo1x_1_compress (const lzo_byte * in, lzo_uint in_len,
		  lzo_byte * out, lzo_uintp out_len, lzo_voidp wrkmem)
{
	lzo_byte *op = out;
	lzo_uint t;

	if (in_len <= M2_MAX_LEN + 5)
		t = in_len;
	else
	{
		t = _lzo1x_1_do_compress (in, in_len, op, out_len, wrkmem);
		op += *out_len;
	}

	if (t > 0)
	{
		const lzo_byte *ii = in + in_len - t;

		if (op == out && t <= 238)
			*op++ = LZO_BYTE (17 + t);
		else if (t <= 3)
			op[-2] |= LZO_BYTE (t);
		else if (t <= 18)
			*op++ = LZO_BYTE (t - 3);
		else
		{
			lzo_uint tt = t - 18;

			*op++ = 0;
			while (tt > 255)
			{
				tt -= 255;
				*op++ = 0;
			}

			*op++ = LZO_BYTE (tt);
		}
		do
			*op++ = *ii++;
		while (--t > 0);
	}

	*op++ = M4_MARKER | 1;
	*op++ = 0;
	*op++ = 0;

	*out_len = op - out;
	return 0;
}

static int
lzo1x_decompress (const lzo_byte * in, lzo_uint in_len,
		  lzo_byte * out, lzo_uintp out_len, lzo_voidp wrkmem)
{
	register lzo_byte *op;
	register const lzo_byte *ip;
	register lzo_uint t;

	register const lzo_byte *m_pos;

	const lzo_byte *const ip_end = in + in_len;
	lzo_byte *const op_end = out + *out_len;

	*out_len = 0;

	op = out;
	ip = in;

	if (*ip > 17)
	{
		t = *ip++ - 17;
		if (t < 4)
			goto match_next;
		NEED_OP (t);
		NEED_IP (t + 1);
		do
			*op++ = *ip++;
		while (--t > 0);
		goto first_literal_run;
	}

	while (TEST_IP && TEST_OP)
	{
		t = *ip++;
		if (t >= 16)
			goto match;
		if (t == 0)
		{
			NEED_IP (1);
			while (*ip == 0)
			{
				t += 255;
				ip++;
				NEED_IP (1);
			}
			t += 15 + *ip++;
		}
		NEED_OP (t + 3);
		NEED_IP (t + 4);
		if (PTR_ALIGNED2_4 (op, ip))
		{
			COPY4 (op, ip);

			op += 4;
			ip += 4;
			if (--t > 0)
			{
				if (t >= 4)
				{
					do
					{
						COPY4 (op, ip);
						op += 4;
						ip += 4;
						t -= 4;
					}
					while (t >= 4);
					if (t > 0)
						do
							*op++ = *ip++;
						while (--t > 0);
				}
				else
					do
						*op++ = *ip++;
					while (--t > 0);
			}
		}
		else
		{
			*op++ = *ip++;
			*op++ = *ip++;
			*op++ = *ip++;
			do
				*op++ = *ip++;
			while (--t > 0);
		}
	      first_literal_run:

		t = *ip++;
		if (t >= 16)
			goto match;

		m_pos = op - (1 + M2_MAX_OFFSET);
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;
		TEST_LOOKBEHIND (m_pos, out);
		NEED_OP (3);
		*op++ = *m_pos++;
		*op++ = *m_pos++;
		*op++ = *m_pos;

		goto match_done;

		while (TEST_IP && TEST_OP)
		{
		      match:
			if (t >= 64)
			{
				m_pos = op - 1;
				m_pos -= (t >> 2) & 7;
				m_pos -= *ip++ << 3;
				t = (t >> 5) - 1;
				TEST_LOOKBEHIND (m_pos, out);
				NEED_OP (t + 3 - 1);
				goto copy_match;

			}
			else if (t >= 32)
			{
				t &= 31;
				if (t == 0)
				{
					NEED_IP (1);
					while (*ip == 0)
					{
						t += 255;
						ip++;
						NEED_IP (1);
					}
					t += 31 + *ip++;
				}

				m_pos = op - 1;
				m_pos -= (ip[0] >> 2) + (ip[1] << 6);

				ip += 2;
			}
			else if (t >= 16)
			{
				m_pos = op;
				m_pos -= (t & 8) << 11;

				t &= 7;
				if (t == 0)
				{
					NEED_IP (1);
					while (*ip == 0)
					{
						t += 255;
						ip++;
						NEED_IP (1);
					}
					t += 7 + *ip++;
				}

				m_pos -= (ip[0] >> 2) + (ip[1] << 6);

				ip += 2;
				if (m_pos == op)
					goto eof_found;
				m_pos -= 0x4000;
			}
			else
			{

				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;
				TEST_LOOKBEHIND (m_pos, out);
				NEED_OP (2);
				*op++ = *m_pos++;
				*op++ = *m_pos;

				goto match_done;
			}

			TEST_LOOKBEHIND (m_pos, out);
			NEED_OP (t + 3 - 1);
			if (t >= 2 * 4 - (3 - 1)
			    && PTR_ALIGNED2_4 (op, m_pos))
			{
				COPY4 (op, m_pos);
				op += 4;
				m_pos += 4;
				t -= 4 - (3 - 1);
				do
				{
					COPY4 (op, m_pos);
					op += 4;
					m_pos += 4;
					t -= 4;
				}
				while (t >= 4);
				if (t > 0)
					do
						*op++ = *m_pos++;
					while (--t > 0);
			}
			else

			{
			      copy_match:
				*op++ = *m_pos++;
				*op++ = *m_pos++;
				do
					*op++ = *m_pos++;
				while (--t > 0);
			}

		      match_done:
			t = ip[-2] & 3;

			if (t == 0)
				break;

		      match_next:
			NEED_OP (t);
			NEED_IP (t + 1);
			do
				*op++ = *ip++;
			while (--t > 0);
			t = *ip++;
		}
	}
	*out_len = op - out;
	return LZO_E_EOF_NOT_FOUND;

      eof_found:
	*out_len = op - out;
	return (ip == ip_end ? LZO_E_OK :
		(ip <
		 ip_end ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));

      input_overrun:
	*out_len = op - out;
	return LZO_E_INPUT_OVERRUN;

      output_overrun:
	*out_len = op - out;
	return LZO_E_OUTPUT_OVERRUN;

      lookbehind_overrun:
	*out_len = op - out;
	return LZO_E_LOOKBEHIND_OVERRUN;
}

/* lzo1x_oo.ch */

#define NO_LIT          LZO_UINT_MAX

static void
copy2 (lzo_byte * ip, const lzo_byte * m_pos, lzo_ptrdiff_t off)
{
	ip[0] = m_pos[0];
	if (off == 1)
		ip[1] = m_pos[0];
	else
		ip[1] = m_pos[1];
}

static void
copy3 (lzo_byte * ip, const lzo_byte * m_pos, lzo_ptrdiff_t off)
{
	ip[0] = m_pos[0];
	if (off == 1)
	{
		ip[2] = ip[1] = m_pos[0];
	}
	else if (off == 2)
	{
		ip[1] = m_pos[1];
		ip[2] = m_pos[0];
	}
	else
	{
		ip[1] = m_pos[1];
		ip[2] = m_pos[2];
	}
}

static int
lzo1x_optimize (lzo_byte * in, lzo_uint in_len,
		lzo_byte * out, lzo_uintp out_len, lzo_voidp wrkmem)
{
	register lzo_byte *op;
	register lzo_byte *ip;
	register lzo_uint t;
	register lzo_byte *m_pos;
	lzo_uint nl;
	const lzo_byte *const ip_end = in + in_len;
	const lzo_byte *const op_end = out + *out_len;
	lzo_byte *litp = NULL;
	lzo_uint lit = 0;
	lzo_uint next_lit = NO_LIT;
	long o_m1_a = 0, o_m1_b = 0, o_m2 = 0, o_m3_a = 0, o_m3_b = 0;

	*out_len = 0;

	op = out;
	ip = in;

	if (*ip > 17)
	{
		t = *ip++ - 17;
		if (t < 4)
			goto match_next;
		goto first_literal_run;
	}

	while (TEST_IP && TEST_OP)
	{
		t = *ip++;
		if (t >= 16)
			goto match;
		litp = ip - 1;
		if (t == 0)
		{
			t = 15;
			while (*ip == 0)
				t += 255, ip++;
			t += *ip++;
		}
		lit = t + 3;
	      copy_literal_run:
		*op++ = *ip++;
		*op++ = *ip++;
		*op++ = *ip++;
	      first_literal_run:
		do
			*op++ = *ip++;
		while (--t > 0);

		t = *ip++;

		if (t >= 16)
			goto match;
		m_pos = op - 1 - 0x800;
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;
		*op++ = *m_pos++;
		*op++ = *m_pos++;
		*op++ = *m_pos++;
		lit = 0;
		goto match_done;

		while (TEST_IP && TEST_OP)
		{
			if (t < 16)
			{
				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;

				if (litp == NULL)
					goto copy_m1;

				nl = ip[-2] & 3;
				if (nl == 0 && lit == 1 && ip[0] >= 16)
				{
					next_lit = nl;
					lit += 2;
					*litp = LZO_BYTE ((*litp & ~3) | lit);
					copy2 (ip - 2, m_pos, op - m_pos);
					o_m1_a++;
				}
				else if (nl == 0 && ip[0] < 16 && ip[0] != 0
					 && (lit + 2 + ip[0] < 16))
				{
					t = *ip++;
					*litp &= ~3;
					copy2 (ip - 3 + 1, m_pos, op - m_pos);
					litp += 2;
					if (lit > 0)
						memmove (litp + 1, litp, lit);
					lit += 2 + t + 3;
					*litp = LZO_BYTE (lit - 3);

					o_m1_b++;
					*op++ = *m_pos++;
					*op++ = *m_pos++;
					goto copy_literal_run;
				}
			      copy_m1:
				*op++ = *m_pos++;
				*op++ = *m_pos++;
			}
			else
			{
			      match:
				if (t >= 64)
				{
					m_pos = op - 1;
					m_pos -= (t >> 2) & 7;
					m_pos -= *ip++ << 3;
					t = (t >> 5) - 1;
					if (litp == NULL)
						goto copy_m;

					nl = ip[-2] & 3;
					if (t == 1 && lit > 3 && nl == 0 &&
					    ip[0] < 16 && ip[0] != 0
					    && (lit + 3 + ip[0] < 16))
					{
						t = *ip++;
						copy3 (ip - 1 - 2, m_pos,
						       op - m_pos);
						lit += 3 + t + 3;
						*litp = LZO_BYTE (lit - 3);
						o_m2++;
						*op++ = *m_pos++;
						*op++ = *m_pos++;
						*op++ = *m_pos++;
						goto copy_literal_run;
					}
				}
				else
				{
					if (t >= 32)
					{
						t &= 31;
						if (t == 0)
						{
							t = 31;
							while (*ip == 0)
								t += 255,
									ip++;
							t += *ip++;
						}
						m_pos = op - 1;
						m_pos -= *ip++ >> 2;
						m_pos -= *ip++ << 6;
					}
					else
					{
						m_pos = op;
						m_pos -= (t & 8) << 11;
						t &= 7;
						if (t == 0)
						{
							t = 7;
							while (*ip == 0)
								t += 255,
									ip++;
							t += *ip++;
						}
						m_pos -= *ip++ >> 2;
						m_pos -= *ip++ << 6;
						if (m_pos == op)
							goto eof_found;
						m_pos -= 0x4000;
					}
					if (litp == NULL)
						goto copy_m;

					nl = ip[-2] & 3;
					if (t == 1 && lit == 0 && nl == 0
					    && ip[0] >= 16)
					{
						next_lit = nl;
						lit += 3;
						*litp = LZO_BYTE ((*litp & ~3)
								  | lit);
						copy3 (ip - 3, m_pos,
						       op - m_pos);
						o_m3_a++;
					}
					else if (t == 1 && lit <= 3 && nl == 0
						 && ip[0] < 16 && ip[0] != 0
						 && (lit + 3 + ip[0] < 16))
					{
						t = *ip++;
						*litp &= ~3;
						copy3 (ip - 4 + 1, m_pos,
						       op - m_pos);
						litp += 2;
						if (lit > 0)
							memmove (litp + 1,
								 litp, lit);
						lit += 3 + t + 3;
						*litp = LZO_BYTE (lit - 3);

						o_m3_b++;
						*op++ = *m_pos++;
						*op++ = *m_pos++;
						*op++ = *m_pos++;
						goto copy_literal_run;
					}
				}
			      copy_m:
				*op++ = *m_pos++;
				*op++ = *m_pos++;
				do
					*op++ = *m_pos++;
				while (--t > 0);
			}

		      match_done:
			if (next_lit == NO_LIT)
			{
				t = ip[-2] & 3;
				lit = t;
				litp = ip - 2;
			}
			else
				t = next_lit;
			next_lit = NO_LIT;
			if (t == 0)
				break;
		      match_next:
			do
				*op++ = *ip++;
			while (--t > 0);
			t = *ip++;
		}
	}

	*out_len = op - out;
	return LZO_E_EOF_NOT_FOUND;

      eof_found:
	*out_len = op - out;
	return (ip == ip_end ? LZO_E_OK :
		(ip <
		 ip_end ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));
}

/* interface to jffs2 bbc follows */

#include "jffs2_bbc_framework.h"

#define BLOCKSIZE		4096
#define OUTBLOCKSIZE	(BLOCKSIZE + BLOCKSIZE / 64 + 16 + 3)

#define JFFS2_BBC_LZO_BLOCK_SIGN {0x3f, 0x47, 0x5a, 0x18}

static int
jffs2_bbc_lzo_compressor_init (void);

static void
jffs2_bbc_lzo_compressor_deinit (void);

static int
jffs2_bbc_lzo_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen);

static int
jffs2_bbc_lzo_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime);

static int
jffs2_bbc_lzo_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen);

static char *
jffs2_bbc_lzo_proc_info (void);

static int
jffs2_bbc_lzo_proc_command (char *command);

struct jffs2_bbc_compressor_type jffs2_bbc_lzo = {
	"lzo",
	0,
	JFFS2_BBC_LZO_BLOCK_SIGN,
	jffs2_bbc_lzo_compressor_init,
	NULL,
	NULL,
	jffs2_bbc_lzo_compressor_deinit,
	jffs2_bbc_lzo_compress,
	jffs2_bbc_lzo_estimate,
	jffs2_bbc_lzo_decompress,
	jffs2_bbc_lzo_proc_info,
	jffs2_bbc_lzo_proc_command
};

static int
no_lzo1x_optimize (lzo_byte * src, lzo_uint src_len,
		   lzo_byte * dst, lzo_uintp dst_len, lzo_voidp wrkmem)
{
	return 0;
}

#ifdef __KERNEL__
static lzo_compress_t lzo1x_compressor = lzo1x_1_compress;
static lzo_optimize_t lzo1x_optimizer = no_lzo1x_optimize;
static int lzo1x_compressor_type = 1;
static int lzo1x_optimize_type = 0;
static unsigned long lzo1x_compressor_memsize = LZO1X_1_MEM_COMPRESS;
#else
static lzo_compress_t lzo1x_compressor = lzo1x_999_compress;
static lzo_optimize_t lzo1x_optimizer = lzo1x_optimize;
static int lzo1x_compressor_type = 999;
static int lzo1x_optimize_type = 1;
static unsigned long lzo1x_compressor_memsize = LZO1X_999_MEM_COMPRESS;
#endif

static lzo_bytep wrkmem = NULL;	/* temporary buffer for compression, used by lzo */
static lzo_bytep cmprssmem = NULL;	/* temporary buffer for compression, used by interface */

static int
jffs2_bbc_lzo_compressor_init (void)
{
	wrkmem = (lzo_bytep) jffs2_bbc_malloc (lzo1x_compressor_memsize);
	cmprssmem = (lzo_bytep) jffs2_bbc_malloc (OUTBLOCKSIZE);
	return !(wrkmem && cmprssmem);
}

static void
jffs2_bbc_lzo_compressor_deinit (void)
{
	jffs2_bbc_free (wrkmem);
	jffs2_bbc_free (cmprssmem);
}

static int
jffs2_bbc_lzo_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen)
{
	lzo_uint csize = OUTBLOCKSIZE;
	lzo_uint isize = *sourcelen;
	int retval;
	if ((retval =
	     lzo1x_compressor (input, *sourcelen, cmprssmem, &csize,
			       wrkmem)) != LZO_E_OK)
	{
		*sourcelen = *dstlen = 0;
		return retval;
	}
	else
	{
		retval = lzo1x_optimizer (cmprssmem, csize, input, &isize,
					  NULL);
		csize += 2;
		if (csize <= *dstlen) {
			*dstlen = csize;
			*(output++) = jffs2_bbc_lzo.block_sign[0];
			*(output++) = jffs2_bbc_lzo.block_sign[1];
			memcpy (output, cmprssmem, csize - 2);
			return retval;
		} else {
			*sourcelen = *dstlen = 0;
			return -1;
		}
	}
}

static int
jffs2_bbc_lzo_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime)
{
	*dstlen = sourcelen * 55 / 100;
	*readtime = JFFS2_BBC_ZLIB_READ_TIME / 2;
	*writetime = JFFS2_BBC_ZLIB_WRITE_TIME * 8 / 10; /* LZO1X-1 is much-much faster,
	but LZO1X-999 is slow. The default mode for inside kernel compression is LZO1X-1
	This should be *0.4 really */
	return 0;
}

static int
jffs2_bbc_lzo_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen)
{
	lzo_uint outlen = dstlen;
	if (	( *(input++) != (unsigned char)jffs2_bbc_lzo.block_sign[0] ) ||
			( *(input++) != (unsigned char)jffs2_bbc_lzo.block_sign[1] )
	   ) {
		return -1;
	} else {
		return lzo1x_decompress (input, sourcelen - 2, output, &outlen, NULL);
	}
}

static char *
jffs2_bbc_lzo_proc_info (void)
{
	if (lzo1x_compressor_type == 1)
	{
		if (lzo1x_optimize_type == 1)
		{
			return "LZO1X-1 compression with optimization";
		}
		else
		{
			return "LZO1X-1 compression without optimization";
		}
	}
	else if (lzo1x_compressor_type == 999)
	{
		if (lzo1x_optimize_type == 1)
		{
			return "LZO1X-999 compression with optimization";
		}
		else
		{
			return "LZO1X-999 compression without optimization";
		}
	}
	else
	{
		return "Unknown configuration!";
	}
}

static int
jffs2_bbc_lzo_proc_command (char *command)
{
	switch (*command)
	{
	case 'o':
		/* switch optimization off */
		lzo1x_optimizer = no_lzo1x_optimize;
		lzo1x_optimize_type = 0;
		jffs2_bbc_print1 ("Compression optimization switched off.\n");
		return 0;
	case 'O':
		/* switch optimization on */
		lzo1x_optimizer = lzo1x_optimize;
		lzo1x_optimize_type = 1;
		jffs2_bbc_print1 ("Compression optimization switched on.\n");
		return 0;
	case '1':
		/* switch compression to LZO1X-1 */
		jffs2_bbc_free (wrkmem);
		lzo1x_compressor_type = 1;
		lzo1x_compressor = lzo1x_1_compress;
		lzo1x_compressor_memsize = LZO1X_1_MEM_COMPRESS;
		wrkmem = (lzo_bytep)
			jffs2_bbc_malloc (lzo1x_compressor_memsize);
		jffs2_bbc_print1 ("Compression type switched to LZO1X-1.\n");
		return 0;
	case '9':
		/* switch compression to LZO1X-999 */
		jffs2_bbc_free (wrkmem);
		lzo1x_compressor_type = 999;
		lzo1x_compressor = lzo1x_999_compress;
		lzo1x_compressor_memsize = LZO1X_999_MEM_COMPRESS;
		wrkmem = (lzo_bytep)
			jffs2_bbc_malloc (lzo1x_compressor_memsize);
		jffs2_bbc_print1
			("Compression type switched to LZO1X-999.\n");
		return 0;
	default:
		jffs2_bbc_print1 ("Unknown command!\n");
		return 0;
	}
}


struct jffs2_bbc_compressor_type *
jffs2_bbc_lzo_init (int mode)
{
	if (jffs2_bbc_register_compressor (&jffs2_bbc_lzo) == 0)
	{
		return &jffs2_bbc_lzo;
	}
	else
	{
		return NULL;
	}
}

void
jffs2_bbc_lzo_deinit (void)
{
	jffs2_bbc_unregister_compressor (&jffs2_bbc_lzo);
}
