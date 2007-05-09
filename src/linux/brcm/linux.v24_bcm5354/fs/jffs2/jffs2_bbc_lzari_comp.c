/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
   jffs2_bbc_lzari_comp.c -- Lempel-Ziv-Arithmetic coding compression module for jffs2
   Copyright (C) 2004 Patrik Kluba
   Based on the LZARI source included in LDS (lossless datacompression sources)
   Block-compression and bitstream modifications by Patrik Kluba
   $Header: /openwrt/openwrt/package/linux/kernel-patches/301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*/

/*
Original copyright follows:

**************************************************************
	LZARI.C -- A Data Compression Program
	(tab = 4 spaces)
**************************************************************
	4/7/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************

LZARI.C (c)1989 by Haruyasu Yoshizaki, Haruhiko Okumura, and Kenji Rikitake.
All rights reserved. Permission granted for non-commercial use.

*/

/*

	2004-02-18  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Removed unused variables and fixed no return value

	2004-02-16  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Initial release

*/

/* lzari.c */

#define N		 4096	/* size of ring buffer */
#define F		   60	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

static unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
			with extra F-1 bytes to facilitate string comparison */
static unsigned long		match_position, match_length,  /* of longest match.  These are
			set by the InsertNode() procedure. */
		lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
			parents -- These constitute binary search trees. */

static void InitTree(void)  /* Initialize trees */
{
	unsigned long  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;	/* root */
	for (i = 0; i < N; i++) dad[i] = NIL;	/* node */
}

static void InsertNode(unsigned long r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	unsigned long i, p, temp;
	unsigned char *key;
	signed long cmp;

	cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;  match_length = 0;
	for ( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != NIL) p = rson[p];
			else {  rson[p] = r;  dad[r] = p;  return;  }
		} else {
			if (lson[p] != NIL) p = lson[p];
			else {  lson[p] = r;  dad[r] = p;  return;  }
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
		if (i > THRESHOLD) {
			if (i > match_length) {
				match_position = (r - p) & (N - 1);
				if ((match_length = i) >= F) break;
			} else if (i == match_length) {
				if ((temp = (r - p) & (N - 1)) < match_position)
					match_position = temp;
			}
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

static void DeleteNode(unsigned long p)  /* Delete node p from tree */
{
	unsigned long  q;
	
	if (dad[p] == NIL) return;  /* not in tree */
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {  q = rson[q];  } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
			lson[q] = lson[p];  dad[lson[p]] = q;
		}
		rson[q] = rson[p];  dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;
	else                   lson[dad[p]] = q;
	dad[p] = NIL;
}

/********** Arithmetic Compression **********/

/*  If you are not familiar with arithmetic compression, you should read
		I. E. Witten, R. M. Neal, and J. G. Cleary,
			Communications of the ACM, Vol. 30, pp. 520-540 (1987),
	from which much have been borrowed.  */

#define M   15

/*	Q1 (= 2 to the M) must be sufficiently large, but not so
	large as the unsigned long 4 * Q1 * (Q1 - 1) overflows.  */

#define Q1  (1UL << M)
#define Q2  (2 * Q1)
#define Q3  (3 * Q1)
#define Q4  (4 * Q1)
#define MAX_CUM (Q1 - 1)

#define N_CHAR  (256 - THRESHOLD + F)
	/* character code = 0, 1, ..., N_CHAR - 1 */

static unsigned long char_to_sym[N_CHAR], sym_to_char[N_CHAR + 1];
static unsigned long
	sym_freq[N_CHAR + 1],  /* frequency for symbols */
	sym_cum[N_CHAR + 1],   /* cumulative freq for symbols */
	position_cum[N + 1];   /* cumulative freq for positions */

static void StartModel(void)  /* Initialize model */
{
	unsigned long ch, sym, i;
	
	sym_cum[N_CHAR] = 0;
	for (sym = N_CHAR; sym >= 1; sym--) {
		ch = sym - 1;
		char_to_sym[ch] = sym;  sym_to_char[sym] = ch;
		sym_freq[sym] = 1;
		sym_cum[sym - 1] = sym_cum[sym] + sym_freq[sym];
	}
	sym_freq[0] = 0;  /* sentinel (!= sym_freq[1]) */
	position_cum[N] = 0;
	for (i = N; i >= 1; i--)
		position_cum[i - 1] = position_cum[i] + 10000 / (i + 200);
			/* empirical distribution function (quite tentative) */
			/* Please devise a better mechanism! */
}

static void UpdateModel(unsigned long sym)
{
	unsigned long c, ch_i, ch_sym;
	unsigned long i;
	if (sym_cum[0] >= MAX_CUM) {
		c = 0;
		for (i = N_CHAR; i > 0; i--) {
			sym_cum[i] = c;
			c += (sym_freq[i] = (sym_freq[i] + 1) >> 1);
		}
		sym_cum[0] = c;
	}
	for (i = sym; sym_freq[i] == sym_freq[i - 1]; i--) ;
	if (i < sym) {
		ch_i = sym_to_char[i];    ch_sym = sym_to_char[sym];
		sym_to_char[i] = ch_sym;  sym_to_char[sym] = ch_i;
		char_to_sym[ch_i] = sym;  char_to_sym[ch_sym] = i;
	}
	sym_freq[i]++;
	while (--i > 0) sym_cum[i]++;
	sym_cum[0]++;
}

static unsigned long BinarySearchSym(unsigned long x)
	/* 1      if x >= sym_cum[1],
	   N_CHAR if sym_cum[N_CHAR] > x,
	   i such that sym_cum[i - 1] > x >= sym_cum[i] otherwise */
{
	unsigned long i, j, k;
	
	i = 1;  j = N_CHAR;
	while (i < j) {
		k = (i + j) / 2;
		if (sym_cum[k] > x) i = k + 1;  else j = k;
	}
	return i;
}

unsigned long BinarySearchPos(unsigned long x)
	/* 0 if x >= position_cum[1],
	   N - 1 if position_cum[N] > x,
	   i such that position_cum[i] > x >= position_cum[i + 1] otherwise */
{
	unsigned long i, j, k;
	
	i = 1;  j = N;
	while (i < j) {
		k = (i + j) / 2;
		if (position_cum[k] > x) i = k + 1;  else j = k;
	}
	return i - 1;
}

/* modified for block compression */
/* on return, srclen will contain the number of successfully compressed bytes
   and dstlen will contain completed compressed bytes */

static int Encode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long *srclen,
			unsigned long *dstlen)
{
	unsigned long c, i, len, r, s, last_match_length, sym, range;
	unsigned long low = 0;
	unsigned long high = Q4;
	unsigned long shifts = 0;  /* counts for magnifying low and high around Q2 */
	unsigned char *ip, *op;
	unsigned long written = 0;
	unsigned long read = 0;
	unsigned char buffer = 0;
	unsigned char mask = 128;
	unsigned char *srcend = srcbuf + *srclen;
	unsigned char *dstend = dstbuf + *dstlen;
	ip = srcbuf;
	op = dstbuf;
	StartModel();
	InitTree();  /* initialize trees */
	s = 0;  r = N - F;
	for (i = s; i < r; i++) text_buf[i] = ' ';  /* Clear the buffer with
		any character that will appear often. */
	for (len = 0; (len < F) && (ip < srcend); len++)
		text_buf[r + len] = *(ip++);  /* Read F bytes into the last F bytes of
			the buffer */
	read = len;
	for (i = 1; i <= F; i++) InsertNode(r - i);  /* Insert the F strings,
		each of which begins with one or more 'space' characters.  Note
		the order in which these strings are inserted.  This way,
		degenerate trees will be less likely to occur. */
	InsertNode(r);  /* Finally, insert the whole string just read.  The
		global variables match_length and match_position are set. */
	do {
		if (match_length > len) match_length = len;  /* match_length
			may be spuriously long near the end of text. */
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one byte. */
			sym = char_to_sym[text_buf[r]];
			range = high - low;
			high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
			low +=       (range * sym_cum[sym    ]) / sym_cum[0];
			for ( ; ; ) {
				if (high <= Q2) {
					if ((mask >>= 1) == 0) {
						if (op >= dstend) {
							*dstlen = written;
							return -1;
						}
						*(op++) = buffer;
						buffer = 0;
						mask = 128;
						written++;
						*srclen = read;
					}
					for ( ; shifts > 0; shifts--) {
						buffer |= mask;
						if ((mask >>= 1) == 0) {
							if (op >= dstend) {
								*dstlen = written;
								return -1;
							}
							*(op++) = buffer;
							buffer = 0;
							mask = 128;
							written++;
							*srclen = read;
						}
					}
				} else if (low >= Q2) {
					buffer |= mask;
					if ((mask >>= 1) == 0) {
						if (op >= dstend) {
							*dstlen = written;
							return -1;
						}
						*(op++) = buffer;
						buffer = 0;
						mask = 128;
						written++;
						*srclen = read;
					}
					for ( ; shifts > 0; shifts--) {
						if ((mask >>= 1) == 0) {
							if (op >= dstend) {
								*dstlen = written;
								return -1;
							}
							*(op++) = buffer;
							buffer = 0;
							mask = 128;
							written++;
							*srclen = read;
						}
					}
					low -= Q2;
					high -= Q2;
				} else if (low >= Q1 && high <= Q3) {
					shifts++;
					low -= Q1;
					high -= Q1;
				} else break;
				low += low;  high += high;
			}
			UpdateModel(sym);
		} else {
			sym = char_to_sym[255 - THRESHOLD + match_length];
			range = high - low;
			high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
			low +=       (range * sym_cum[sym    ]) / sym_cum[0];
			for ( ; ; ) {
				if (high <= Q2) {
					if ((mask >>= 1) == 0) {
						if (op >= dstend) {
							*dstlen = written;
							return -1;
						}
						*(op++) = buffer;
						buffer = 0;
						mask = 128;
						written++;
						*srclen = read;
					}
					for ( ; shifts > 0; shifts--) {
						buffer |= mask;
						if ((mask >>= 1) == 0) {
							if (op >= dstend) {
								*dstlen = written;
								return -1;
							}
							*(op++) = buffer;
							buffer = 0;
							mask = 128;
							written++;
							*srclen = read;
						}
					}
				} else if (low >= Q2) {
					buffer |= mask;
					if ((mask >>= 1) == 0) {
						if (op >= dstend) {
							*dstlen = written;
							return -1;
						}
						*(op++) = buffer;
						buffer = 0;
						mask = 128;
						written++;
						*srclen = read;
					}
					for ( ; shifts > 0; shifts--) {
						if ((mask >>= 1) == 0) {
							if (op >= dstend) {
								*dstlen = written;
								return -1;
							}
							*(op++) = buffer;
							buffer = 0;
							mask = 128;
							written++;
							*srclen = read;
						}
					}
					low -= Q2;
					high -= Q2;
				} else if (low >= Q1 && high <= Q3) {
					shifts++;
					low -= Q1;
					high -= Q1;
				} else break;
				low += low;  high += high;
			}
			UpdateModel(sym);
			range = high - low;
			high = low + (range * position_cum[match_position - 1]) / position_cum[0];
			low +=       (range * position_cum[match_position    ]) / position_cum[0];
			for ( ; ; ) {
				if (high <= Q2) {
					if ((mask >>= 1) == 0) {
						if (op >= dstend) {
							*dstlen = written;
							return -1;
						}
						*(op++) = buffer;
						buffer = 0;
						mask = 128;
						written++;
						*srclen = read;
					}
					for ( ; shifts > 0; shifts--) {
						buffer |= mask;
						if ((mask >>= 1) == 0) {
							if (op >= dstend) {
								*dstlen = written;
								return -1;
							}
							*(op++) = buffer;
							buffer = 0;
							mask = 128;
							written++;
							*srclen = read;
						}
					}
				} else {
					if (low >= Q2) {
						buffer |= mask;
						if ((mask >>= 1) == 0) {
							if (op >= dstend) {
								*dstlen = written;
								return -1;
							}
							*(op++) = buffer;
							buffer = 0;
							mask = 128;
							written++;
							*srclen = read;
						}
						for ( ; shifts > 0; shifts--) {
							if ((mask >>= 1) == 0) {
								if (op >= dstend) {
									*dstlen = written;
									return -1;
								}
								*(op++) = buffer;
								buffer = 0;
								mask = 128;
								written++;
								*srclen = read;
							}
						}
						low -= Q2;
						high -= Q2;
					} else {
						if ((low >= Q1) && (high <= Q3)) {
							shifts++;
							low -= Q1;
							high -= Q1;
						} else {
							break;
						}
					}
				}
				low += low;
				high += high;
			}
		}
		last_match_length = match_length;
		for (i = 0; (i < last_match_length) && (ip < srcend); i++) {
			c = *(ip++);
			DeleteNode(s);
			text_buf[s] = c;
			if (s < F - 1)
				text_buf[s + N] = c;
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}
		read += i;
		while (i++ < last_match_length) {
			DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);
		}
	} while (len > 0);
	shifts++;
	if (low < Q1) {
		if ((mask >>= 1) == 0) {
			if (op >= dstend) {
				*dstlen = written;
				return -1;
			}
			*(op++) = buffer;
			buffer = 0;
			mask = 128;
			written++;
			*srclen = read;
		}
		for ( ; shifts > 0; shifts--) {
			buffer |= mask;
			if ((mask >>= 1) == 0) {
				if (op >= dstend) {
					*dstlen = written;
					return -1;
				}
				*(op++) = buffer;
				buffer = 0;
				mask = 128;
				written++;
				*srclen = read;
			}
		}
	} else {
		buffer |= mask;
		if ((mask >>= 1) == 0) {
			if (op >= dstend) {
				*dstlen = written;
				return -1;
			}
			*(op++) = buffer;
			buffer = 0;
			mask = 128;
			written++;
			*srclen = read;
		}
		for ( ; shifts > 0; shifts--) {
			if ((mask >>= 1) == 0) {
				if (op >= dstend) {
					*dstlen = written;
					return -1;
				}
				*(op++) = buffer;
				buffer = 0;
				mask = 128;
				written++;
				*srclen = read;
			}
		}
	}
	for (i = 0; i < 7; i++) {
		if ((mask >>= 1) == 0) {
			if (op >= dstend) {
				*dstlen = written;
				return -1;
			}
			*(op++) = buffer;
			buffer = 0;
			mask = 128;
			written++;
			*srclen = read;
		}
	}
	*dstlen = written;
	return 0;
}

static int Decode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long srclen,
					unsigned long dstlen)	/* Just the reverse of Encode(). */
{
	unsigned long i, r, j, k, c, range, sym;
	unsigned char *ip, *op;
	unsigned char *srcend = srcbuf + srclen;
	unsigned char *dstend = dstbuf + dstlen;
	unsigned char buffer = 0;
	unsigned char mask = 0;
	unsigned long low = 0;
	unsigned long high = Q4;
	unsigned long value = 0;
	ip = srcbuf;
	op = dstbuf;
	for (i = 0; i < M + 2; i++) {
		value *= 2;
		if ((mask >>= 1) == 0) {
			buffer = (ip >= srcend) ? 0 : *(ip++);
			mask = 128;
		}
		value += ((buffer & mask) != 0);
	}
	StartModel();
	for (i = 0; i < N - F; i++) text_buf[i] = ' ';
	r = N - F;
	while (op < dstend) {
		range = high - low;
		sym = BinarySearchSym((unsigned long)
				(((value - low + 1) * sym_cum[0] - 1) / range));
		high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
		low +=       (range * sym_cum[sym    ]) / sym_cum[0];
		for ( ; ; ) {
			if (low >= Q2) {
				value -= Q2;  low -= Q2;  high -= Q2;
			} else if (low >= Q1 && high <= Q3) {
				value -= Q1;  low -= Q1;  high -= Q1;
			} else if (high > Q2) break;
			low += low;  high += high;
			value *= 2;
			if ((mask >>= 1) == 0) {
				buffer = (ip >= srcend) ? 0 : *(ip++);
				mask = 128;
			}
			value += ((buffer & mask) != 0);
		}
		c = sym_to_char[sym];
		UpdateModel(sym);
		if (c < 256) {
			if (op >= dstend) return -1;
			*(op++) = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			j = c - 255 + THRESHOLD;
			range = high - low;
			i = BinarySearchPos((unsigned long)
				(((value - low + 1) * position_cum[0] - 1) / range));
			high = low + (range * position_cum[i    ]) / position_cum[0];
			low +=       (range * position_cum[i + 1]) / position_cum[0];
			for ( ; ; ) {
				if (low >= Q2) {
					value -= Q2;  low -= Q2;  high -= Q2;
				} else if (low >= Q1 && high <= Q3) {
					value -= Q1;  low -= Q1;  high -= Q1;
				} else if (high > Q2) break;
				low += low;  high += high;
				value *= 2;
				if ((mask >>= 1) == 0) {
					buffer = (ip >= srcend) ? 0 : *(ip++);
					mask = 128;
				}
				value += ((buffer & mask) != 0);
			}
			i = (r - i - 1) & (N - 1);
			for (k = 0; k < j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				if (op >= dstend) return -1;
				*(op++) = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}		
		}
	}
	return 0;
}

/* interface to jffs2 bbc follows */

#include "jffs2_bbc_framework.h"

#define JFFS2_BBC_LZARI_BLOCK_SIGN {0x73, 0x9a, 0x1c, 0x4d}

static int
jffs2_bbc_lzari_compressor_init (void);

static void
jffs2_bbc_lzari_compressor_deinit (void);

static int
jffs2_bbc_lzari_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen);

static int
jffs2_bbc_lzari_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime);

static int
jffs2_bbc_lzari_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen);

static char *
jffs2_bbc_lzari_proc_info (void);

static int
jffs2_bbc_lzari_proc_command (char *command);

struct jffs2_bbc_compressor_type jffs2_bbc_lzari = {
	"lzari",
	0,
	JFFS2_BBC_LZARI_BLOCK_SIGN,
	jffs2_bbc_lzari_compressor_init,
	NULL,
	NULL,
	jffs2_bbc_lzari_compressor_deinit,
	jffs2_bbc_lzari_compress,
	jffs2_bbc_lzari_estimate,
	jffs2_bbc_lzari_decompress,
	jffs2_bbc_lzari_proc_info,
	jffs2_bbc_lzari_proc_command
};

static int
jffs2_bbc_lzari_compressor_init (void)
{
	return 0;
}

static void
jffs2_bbc_lzari_compressor_deinit (void)
{
}

static int
jffs2_bbc_lzari_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen)
{
	int retval;
	unsigned long dst = *dstlen;
	*(output++) = jffs2_bbc_lzari.block_sign[0];
	*(output++) = jffs2_bbc_lzari.block_sign[1];
	dst -= 2;
	retval = Encode(input, output, sourcelen, &dst);
	dst += 2;
	*dstlen = dst;
	return retval;
}

static int
jffs2_bbc_lzari_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime)
{
	*dstlen = sourcelen / 2;
	*readtime = JFFS2_BBC_ZLIB_READ_TIME * 15;
	*writetime = JFFS2_BBC_ZLIB_WRITE_TIME * 7;
	return 0;
}

static int
jffs2_bbc_lzari_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen)
{
	if (	( *(input++) != (unsigned char)jffs2_bbc_lzari.block_sign[0] ) ||
			( *(input++) != (unsigned char)jffs2_bbc_lzari.block_sign[1] )
	   ) {
		return -1;
	} else {
		return Decode(input, output, sourcelen - 2, dstlen);
	}
}

static char *
jffs2_bbc_lzari_proc_info (void)
{
	return "Lempel-Ziv-Arithmetic coding compression module";
}

static int
jffs2_bbc_lzari_proc_command (char *command)
{
	return 0;
}

struct jffs2_bbc_compressor_type *
jffs2_bbc_lzari_init (int mode)
{
	if (jffs2_bbc_register_compressor (&jffs2_bbc_lzari) == 0)
	{
		return &jffs2_bbc_lzari;
	}
	else
	{
		return NULL;
	}
}

void
jffs2_bbc_lzari_deinit (void)
{
	jffs2_bbc_unregister_compressor (&jffs2_bbc_lzari);
}
