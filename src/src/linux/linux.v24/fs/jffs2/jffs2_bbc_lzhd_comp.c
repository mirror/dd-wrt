/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
   jffs2_bbc_lzhd_comp.c -- Lempel-Ziv-(dynamic) Huffman compression module for jffs2
   Copyright (C) 2004 Patrik Kluba
   Based on the LZHUF source included in LDS (lossless datacompression sources)
   Block-compression and bitstream modifications by Patrik Kluba
   $Header: /openwrt/openwrt/package/linux/kernel-patches/301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*/

/*
Original copyright follows:

**************************************************************
	lzhuf.c
	written by Haruyasu Yoshizaki 11/20/1988
	some minor changes 4/6/1989
	comments translated by Haruhiko Okumura 4/7/1989
**************************************************************

LZHUF.C (c)1989 by Haruyasu Yoshizaki, Haruhiko Okumura, and Kenji Rikitake.
All rights reserved. Permission granted for non-commercial use.

*/

/*

	2004-02-18  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Replaced name lzh-d with lzhd
				Fixed no return value

	2004-02-16  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Initial release

*/

/* required because of memmove */
#ifndef __KERNEL__
  #include <string.h>
#else
  #include <linux/string.h>
#endif

/* lzhuf.c */

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

static void InitTree(void)  /* initialize trees */
{
	unsigned long  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
	for (i = 0; i < N; i++) dad[i] = NIL;
}

static void InsertNode(unsigned long r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	unsigned long i, p, c;
	signed long cmp;
	unsigned char  *key;
	
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
				match_position = ((r - p) & (N - 1)) - 1;
				if ((match_length = i) >= F)  break;
			}
			if (i == match_length) {
				if ((c = ((r - p) & (N - 1)) - 1) < match_position) {
					match_position = c;
				}
			}
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

static void DeleteNode(unsigned long p)  /* deletes node p from tree */
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
	if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
	dad[p] = NIL;
}

/* Huffman coding */

#define N_CHAR  	(256 - THRESHOLD + F)
				/* kinds of characters (character code = 0..N_CHAR-1) */
#define T 		(N_CHAR * 2 - 1)	/* size of table */
#define R 		(T - 1)			/* position of root */
#define MAX_FREQ	0x8000		/* updates tree when the */
					/* root frequency comes to this value. */

typedef unsigned long uchar; // much-much faster

/* table for encoding and decoding the upper 6 bits of position */

/* for encoding */
static uchar p_len[64] = {
	0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

static uchar p_code[64] = {
	0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
	0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
	0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
	0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
	0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

/* for decoding */
static uchar d_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
	0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
	0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
	0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
	0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
	0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static uchar d_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

static unsigned long freq[T + 1];	/* frequency table */

static unsigned long prnt[T + N_CHAR];	/* pointers to parent nodes, except for the */
			/* elements [T..T + N_CHAR - 1] which are used to get */
			/* the positions of leaves corresponding to the codes. */

static unsigned long son[T];		/* pointers to child nodes (son[], son[] + 1) */

/* initialization of tree */

static void StartHuff(void)
{
	unsigned long i, j;

	for (i = 0; i < N_CHAR; i++) {
		freq[i] = 1;
		son[i] = i + T;
		prnt[i + T] = i;
	}
	i = 0; j = N_CHAR;
	while (j <= R) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[T] = 0xffff;
	prnt[R] = 0;
}

/* reconstruction of tree */

static void reconst(void)
{
	unsigned long f, l, i, j, k;

	/* collect leaf nodes in the first half of the table */
	/* and replace the freq by (freq + 1) / 2. */
	j = 0;
	for (i = 0; i < T; i++) {
		if (son[i] >= T) {
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}
	/* begin constructing tree by connecting sons */
	for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];
		for (k = j - 1; f < freq[k]; k--);
		k++;
		l = (j - k) * 2;
		memmove(&freq[k + 1], &freq[k], l*sizeof(unsigned long));
		freq[k] = f;
		memmove(&son[k + 1], &son[k], l*sizeof(unsigned long));
		son[k] = i;
	}
	/* connect prnt */
	for (i = 0; i < T; i++) {
		if ((k = son[i]) >= T) {
			prnt[k] = i;
		} else {
			prnt[k] = prnt[k + 1] = i;
		}
	}
}

/* increment frequency of given code by one, and update tree */

static void update(unsigned long c)
{
	unsigned long i, j, k, l;

	if (freq[R] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + T];
	do {
		k = ++freq[c];

		/* if the order is disturbed, exchange nodes */
		if (k > freq[l = c + 1]) {
			while (k > freq[++l]);
			l--;
			freq[c] = freq[l];
			freq[l] = k;

			i = son[c];
			prnt[i] = l;
			if (i < T) prnt[i + 1] = l;

			j = son[l];
			son[l] = i;

			prnt[j] = c;
			if (j < T) prnt[j + 1] = c;
			son[c] = j;

			c = l;
		}
	} while (c = prnt[c]);	/* repeat up to root */
}

/* modified for block compression */
/* on return, srclen will contain the number of successfully compressed bytes
   and dstlen will contain completed compressed bytes */

static int Encode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long *srclen,
			unsigned long *dstlen)
{
	unsigned long c, i, j, k, len, r, s, last_match_length, code_buf_ptr;
	unsigned char code_buf[17], mask;
	unsigned char *ip, *op;
	unsigned long written = 0;
	unsigned long read = 0;
	unsigned short putbuf = 0;
	uchar putlen = 0;
	unsigned char *srcend = srcbuf + *srclen;
	unsigned char *dstend = dstbuf + *dstlen;
	ip = srcbuf;
	op = dstbuf;
	StartHuff();
	InitTree();  /* initialize trees */
	code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
		code_buf[0] works as eight flags, "1" representing that the unit
		is an unencoded letter (1 byte), "0" a position-and-length pair
		(2 bytes).  Thus, eight units require at most 16 bytes of code. */
	code_buf_ptr = mask = 1;
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
			c = text_buf[r];
			i = 0; j = 0; k = prnt[c + T];
			do {
				i >>= 1;
				/* if node's address is odd-numbered, choose bigger brother node */
				if (k & 1) i |= 0x8000;
				j++;
			} while ((k = prnt[k]) != R);				
			putbuf |= i >> putlen;
			if ((putlen += j) >= 8) {
				if (op >= dstend) {
					*dstlen = written;
					return -1;
				}
				*(op++) = putbuf >> 8;
				if ((putlen -= 8) >= 8) {
					if (op >= dstend) {
						*dstlen = written;
						return -1;
					}
					*(op++) = putbuf;
					written += 2;
					putlen -= 8;
					putbuf = i << (j - putlen); /**warm**/
				} else {
					putbuf <<= 8;
					written++;
				}
				*srclen = read;
			}
			update(c);
		} else {
			c = 255 - THRESHOLD + match_length;
			i = 0; j = 0; k = prnt[c + T];
			do {
				i >>= 1;
				/* if node's address is odd-numbered, choose bigger brother node */
				if (k & 1) i |= 0x8000;
				j++;
			} while ((k = prnt[k]) != R);				
			putbuf |= i >> putlen;
			if ((putlen += j) >= 8) {
				if (op >= dstend) {
					*dstlen = written;
					return -1;
				}
				*(op++) = putbuf >> 8;
				if ((putlen -= 8) >= 8) {
					if (op >= dstend) {
						*dstlen = written;
						return -1;
					}
					*(op++) = putbuf;
					written += 2;
					putlen -= 8;
					putbuf = i << (j - putlen); /**warm**/
				} else {
					putbuf <<= 8;
					written++;
				}
				*srclen = read;
			}
			update(c);
			j = p_len[match_position >> 6];
			i = p_code[match_position >> 6] << 8;			
			putbuf |= i >> putlen;
			if ((putlen += j) >= 8) {
				if (op >= dstend) {
					*dstlen = written;
					return -1;
				}
				*(op++) = putbuf >> 8;
				if ((putlen -= 8) >= 8) {
					if (op >= dstend) {
						*dstlen = written;
						return -1;
					}
					*(op++) = putbuf;
					written += 2;
					putlen -= 8;
					putbuf = i << (j - putlen); /**hot**/
				} else {
					putbuf <<= 8;
					written++;
				}
				*srclen = read;
			}
			j = 6;
			i = (match_position & 0x3f) << 10;
			putbuf |= i >> putlen;
			if ((putlen += j) >= 8) {
				if (op >= dstend) {
					*dstlen = written;
					return -1;
				}
				*(op++) = putbuf >> 8;
				if ((putlen -= 8) >= 8) {
					if (op >= dstend) {
						*dstlen = written;
						return -1;
					}
					*(op++) = putbuf;
					written += 2;
					putlen -= 8;
					putbuf = i << (j - putlen); /**hot**/
				} else {
					putbuf <<= 8;
					written++;
				}
				*srclen = read;
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
	if (putlen) {
		if (op >= dstend) {
			*dstlen = written;
			return -1;
		}
		*(op++) = putbuf >> 8;
		written++;
		*srclen = read;
	}
	*dstlen = written;
	return 0;
}

static int Decode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long srclen,
					unsigned long dstlen)	/* Just the reverse of Encode(). */
{
	unsigned long i, r, j, k, c;
	unsigned char *ip, *op;
	unsigned char *srcend = srcbuf + srclen;
	unsigned char *dstend = dstbuf + dstlen;
	unsigned short getbuf = 0;
	uchar getlen = 0;
	ip = srcbuf;
	op = dstbuf;
	StartHuff();
	for (i = 0; i < N - F; i++) text_buf[i] = ' ';
	r = N - F;
	while (op < dstend) {
		c = son[R];
		/* travel from root to leaf, */
		/* choosing the smaller child node (son[]) if the read bit is 0, */
		/* the bigger (son[]+1} if 1 */
		while (c < T) {
			while (getlen <= 8) {
				unsigned short t;
				t = (ip >= srcend) ? 0 : *(ip++);
				getbuf |= t << (8 - getlen);
				getlen += 8;
			}
			c += ((signed short)getbuf < 0);
			getbuf <<= 1;
			getlen--;
			c = son[c];
		}
		c -= T;
		update(c);
		if (c < 256) {
			if (op >= dstend) return -1;
			*(op++) = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			j = c - 255 + THRESHOLD;
			while (getlen <= 8) {
				unsigned short t;
				t = (ip >= srcend) ? 0 : *(ip++);
				getbuf |= t << (8 - getlen);
				getlen += 8;
			}
			i = getbuf >> 8;
			getbuf <<= 8;
			getlen -= 8;
			c = d_code[i] << 6;
			k = d_len[i];
			/* read lower 6 bits verbatim */
			k -= 2;
			while (k--) {
				while (getlen <= 8) {
					unsigned short t;
					t = (ip >= srcend) ? 0 : *(ip++);
					getbuf |= t << (8 - getlen);
					getlen += 8;
				}
				i = (i << 1) + ((signed short)getbuf < 0);
				getbuf <<= 1;
				getlen--;
			}
			i = c | (i & 0x3F);
			i = r - i - 1;
			i &= (N - 1);
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


#define JFFS2_BBC_LZHD_BLOCK_SIGN {0x3a, 0x98, 0xf7, 0xda}

static int
jffs2_bbc_lzhd_compressor_init (void);

static void
jffs2_bbc_lzhd_compressor_deinit (void);

static int
jffs2_bbc_lzhd_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen);

static int
jffs2_bbc_lzhd_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime);

static int
jffs2_bbc_lzhd_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen);

static char *
jffs2_bbc_lzhd_proc_info (void);

static int
jffs2_bbc_lzhd_proc_command (char *command);

struct jffs2_bbc_compressor_type jffs2_bbc_lzhd = {
	"lzhd",
	0,
	JFFS2_BBC_LZHD_BLOCK_SIGN,
	jffs2_bbc_lzhd_compressor_init,
	NULL,
	NULL,
	jffs2_bbc_lzhd_compressor_deinit,
	jffs2_bbc_lzhd_compress,
	jffs2_bbc_lzhd_estimate,
	jffs2_bbc_lzhd_decompress,
	jffs2_bbc_lzhd_proc_info,
	jffs2_bbc_lzhd_proc_command
};

static int
jffs2_bbc_lzhd_compressor_init (void)
{
	return 0;
}

static void
jffs2_bbc_lzhd_compressor_deinit (void)
{
}

static int
jffs2_bbc_lzhd_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen)
{
	int retval;
	unsigned long dst = *dstlen;
	*(output++) = jffs2_bbc_lzhd.block_sign[0];
	*(output++) = jffs2_bbc_lzhd.block_sign[1];
	dst -= 2;
	retval = Encode(input, output, sourcelen, &dst);
	dst += 2;
	*dstlen = dst;
	return retval;
}

static int
jffs2_bbc_lzhd_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime)
{
	*dstlen = sourcelen * 55 / 100;
	*readtime = JFFS2_BBC_ZLIB_READ_TIME * 8;
	*writetime = JFFS2_BBC_ZLIB_WRITE_TIME * 65 / 10;
	return 0;
}

static int
jffs2_bbc_lzhd_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen)
{
	if (	( *(input++) != (unsigned char)jffs2_bbc_lzhd.block_sign[0] ) ||
			( *(input++) != (unsigned char)jffs2_bbc_lzhd.block_sign[1] )
	   ) {
		return -1;
	} else {
		return Decode(input, output, sourcelen - 2, dstlen);
	}
}

static char *
jffs2_bbc_lzhd_proc_info (void)
{
	return "Lempel-Ziv-(dynamic) Huffman compression module";
}

static int
jffs2_bbc_lzhd_proc_command (char *command)
{
	return 0;
}

struct jffs2_bbc_compressor_type *
jffs2_bbc_lzhd_init (int mode)
{
	if (jffs2_bbc_register_compressor (&jffs2_bbc_lzhd) == 0)
	{
		return &jffs2_bbc_lzhd;
	}
	else
	{
		return NULL;
	}
}

void
jffs2_bbc_lzhd_deinit (void)
{
	jffs2_bbc_unregister_compressor (&jffs2_bbc_lzhd);
}
