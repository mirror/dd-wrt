/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
   jffs2_bbc_lzss_comp.c -- Lempel-Ziv-Storer-Szymanski compression module for jffs2
   Copyright (C) 2004 Patrik Kluba
   Based on the LZSS source included in LDS (lossless datacompression sources)
   Block-compression modifications by Patrik Kluba
   $Header: /openwrt/openwrt/package/linux/kernel-patches/301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*/

/*
Original copyright follows:

**************************************************************
	LZSS.C -- A Data Compression Program
**************************************************************
    4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************

*/

/*

	2004-02-16  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Initial release
					
*/

/* lzss.c */

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

static unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
			with extra F-1 bytes to facilitate string comparison */
static unsigned long		match_position, match_length;  /* of longest match.  These are
			set by the InsertNode() procedure. */
static unsigned long		lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
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
	unsigned long  i, p;
	unsigned char  *key;
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
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)  break;
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

/* modified for block compression */
/* on return, srclen will contain the number of successfully compressed bytes
   and dstlen will contain completed compressed bytes */

static int Encode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long *srclen,
			unsigned long *dstlen)
{
	unsigned long i, len, r, c, s, last_match_length, code_buf_ptr;
	unsigned char code_buf[17], mask;
	unsigned char *ip, *op;
	unsigned long written = 0;
	unsigned long read = 0;
	unsigned char *srcend = srcbuf + *srclen;
	unsigned char *dstend = dstbuf + *dstlen;
	ip = srcbuf;
	op = dstbuf;
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
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		} else {
			code_buf[code_buf_ptr++] = match_position;
			code_buf[code_buf_ptr++] = (((match_position >> 4) & 0xf0)
			  | (match_length - (THRESHOLD + 1)));  /* Send position and
					length pair. Note match_length > THRESHOLD. */
		}
		if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
			if ((op + code_buf_ptr) > dstend) {
				*dstlen = written; /* written contains bytes of complete compressed
				                 code */
				return -1;
			};
			for (i = 0; i < code_buf_ptr; *(op++) = code_buf[i++]);  /* Send at most 8 units of */
				     /* code together */
			written += code_buf_ptr;
			*srclen = read; /* this many bytes have been successfully compressed */
			code_buf[0] = 0;  code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;
		for (i = 0; (i < last_match_length) && (ip < srcend); i++) {
			c = *(ip++);
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1) text_buf[s + N] = c;  /* If the position is
				near the end of buffer, extend the buffer to make
				string comparison easier. */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
			InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
		}
		read += i;
		while (i++ < last_match_length) {	/* After the end of text, */
			DeleteNode(s);					/* no need to read, but */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		/* buffer may not be empty. */
		}
	} while (len > 0);	/* until length of string to be processed is zero */
	if (code_buf_ptr > 1) {		/* Send remaining code. */
		if ((op + code_buf_ptr) > dstend) {
			*dstlen = written;
			return -1;
		}
		for (i = 0; i < code_buf_ptr; *(op++) = code_buf[i++]);
		written += code_buf_ptr;
		*srclen = read;
	}
	*dstlen = written;
	return 0;
}

static int Decode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long srclen,
					unsigned long dstlen)	/* Just the reverse of Encode(). */
{
	unsigned long i, r, c, j, k, flags;
	unsigned char *ip, *op;
	unsigned long written;
	unsigned long read;
	unsigned char *srcend = srcbuf + srclen;
	unsigned char *dstend = dstbuf + dstlen;
	read = written = 0;
	ip = srcbuf;
	op = dstbuf;
	for (i = 0; i < N - F; i++) text_buf[i] = ' ';
	r = N - F;  flags = 0;
	for ( ; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if (ip >= srcend) return 0;
			c = *(ip++);
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			if (ip >= srcend) return 0;
			c = *(ip++);
			if (op >= dstend) return -1;
			*(op++) = text_buf[r++] = c;  r &= (N - 1);
		} else {
			if ((ip + 2) > srcend) return 0;
			i = *(ip++);
			j = *(ip++);
			i |= ((j & 0xf0) << 4);  j = (j & 0x0f) + THRESHOLD;
			if ((op + j + 1) > dstend) return -1;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				*(op++) = text_buf[r++] = c;  r &= (N - 1);
			}
		}
	}
}

/* interface to jffs2 bbc follows */

#include "jffs2_bbc_framework.h"


#define JFFS2_BBC_LZSS_BLOCK_SIGN {0x27, 0x6f, 0x12, 0xc4}

static int
jffs2_bbc_lzss_compressor_init (void);

static void
jffs2_bbc_lzss_compressor_deinit (void);

static int
jffs2_bbc_lzss_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen);

static int
jffs2_bbc_lzss_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime);

static int
jffs2_bbc_lzss_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen);

static char *
jffs2_bbc_lzss_proc_info (void);

static int
jffs2_bbc_lzss_proc_command (char *command);

struct jffs2_bbc_compressor_type jffs2_bbc_lzss = {
	"lzss",
	0,
	JFFS2_BBC_LZSS_BLOCK_SIGN,
	jffs2_bbc_lzss_compressor_init,
	NULL,
	NULL,
	jffs2_bbc_lzss_compressor_deinit,
	jffs2_bbc_lzss_compress,
	jffs2_bbc_lzss_estimate,
	jffs2_bbc_lzss_decompress,
	jffs2_bbc_lzss_proc_info,
	jffs2_bbc_lzss_proc_command
};

static int
jffs2_bbc_lzss_compressor_init (void)
{
	return 0;
}

static void
jffs2_bbc_lzss_compressor_deinit (void)
{
}

static int
jffs2_bbc_lzss_compress (void *model, unsigned char *input,
			unsigned char *output, unsigned long *sourcelen,
			unsigned long *dstlen)
{
	int retval;
	unsigned long dst = *dstlen;
	*(output++) = jffs2_bbc_lzss.block_sign[0];
	*(output++) = jffs2_bbc_lzss.block_sign[1];
	dst -= 2;
	retval = Encode(input, output, sourcelen, &dst);
	dst += 2;
	*dstlen = dst;
	return retval;
}

static int
jffs2_bbc_lzss_estimate (void *model, unsigned char *input,
			unsigned long sourcelen, unsigned long *dstlen,
			unsigned long *readtime, unsigned long *writetime)
{
	*dstlen = sourcelen * 60 / 100;
	*readtime = JFFS2_BBC_ZLIB_READ_TIME * 12 / 10;
	*writetime = JFFS2_BBC_ZLIB_WRITE_TIME * 3;
	return 0;
}

static int
jffs2_bbc_lzss_decompress (void *model, unsigned char *input,
			  unsigned char *output, unsigned long sourcelen,
			  unsigned long dstlen)
{
	if (	( *(input++) != (unsigned char)jffs2_bbc_lzss.block_sign[0] ) ||
			( *(input++) != (unsigned char)jffs2_bbc_lzss.block_sign[1] )
	   ) {
		return -1;
	} else {
		return Decode(input, output, sourcelen - 2, dstlen);
	}
}

static char *
jffs2_bbc_lzss_proc_info (void)
{
	return "Lempel-Ziv-Storer-Szymanski compression module";
}

static int
jffs2_bbc_lzss_proc_command (char *command)
{
	return 0;
}

struct jffs2_bbc_compressor_type *
jffs2_bbc_lzss_init (int mode)
{
	if (jffs2_bbc_register_compressor (&jffs2_bbc_lzss) == 0)
	{
		return &jffs2_bbc_lzss;
	}
	else
	{
		return NULL;
	}
}

void
jffs2_bbc_lzss_deinit (void)
{
	jffs2_bbc_unregister_compressor (&jffs2_bbc_lzss);
}
