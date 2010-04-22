/*
 * Copyright 2007 David Gibson, IBM Corporation.
 * Based on earlier work, Copyright (C) Paul Mackerras 1997.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <stddef.h>
#include "string.h"
#include "stdio.h"
#include "ops.h"
#include "gunzip_util.h"

#define HEAD_CRC	2
#define EXTRA_FIELD	4
#define ORIG_NAME	8
#define COMMENT		0x10
#define RESERVED	0xe0


#define _LZMA_IN_CB

#include "LzmaDecode.h"


static unsigned char *inbuf;		/* input buffer */

static unsigned insize=0;		/* valid bytes in inbuf */
static unsigned inptr=0;		/* index of next byte to be processed in inbuf */
static unsigned outcnt;		/* bytes in output buffer */
static __inline__ int read_byte(unsigned char **buffer, UInt32 *bufferSize);
#include "LzmaDecode.c"
#define get_byte()  inbuf[inptr++]



static unsigned int icnt = 0;
static __inline__ int read_byte(unsigned char **buffer, UInt32 *bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
        if ( icnt++ % ( 1024 * 10 ) == 0 )
               printf(".");
	return LZMA_RESULT_OK;
}	



/**
 * gunzip_start - prepare to decompress gzip data
 * @state:     decompressor state structure to be initialized
 * @src:       buffer containing gzip compressed or uncompressed data
 * @srclen:    size in bytes of the buffer at src
 *
 * If the buffer at @src contains a gzip header, this function
 * initializes zlib to decompress the data, storing the decompression
 * state in @state.  The other functions in this file can then be used
 * to decompress data from the gzipped stream.
 *
 * If the buffer at @src does not contain a gzip header, it is assumed
 * to contain uncompressed data.  The buffer information is recorded
 * in @state and the other functions in this file will simply copy
 * data from the uncompressed data stream at @src.
 *
 * Any errors, such as bad compressed data, cause an error to be
 * printed an the platform's exit() function to be called.
 */
void gunzip_start(struct gunzip_state *state, void *src, int srclen)
{
	unsigned int i;
	unsigned int uncompressedSize;
	inptr=0;
	outcnt=0;
	insize=0;
	inbuf=(unsigned char*)src;
	// lzma args
	i = get_byte();
	state->lc = i % 9, i = i / 9;
	state->lp = i % 5, state->pb = i / 5;
        
        // skip dictionary size
        for (i = 0; i < 4; i++) 
        	get_byte();
        // get uncompressed size
	uncompressedSize = (get_byte()) +
		(get_byte() << 8) +
		(get_byte() << 16) +
		(get_byte() << 24);
        // skip high order bytes
        for (i = 0; i < 4; i++) 
        	get_byte();

}

/**
 * gunzip_partial - extract bytes from a gzip data stream
 * @state:     gzip state structure previously initialized by gunzip_start()
 * @dst:       buffer to store extracted data
 * @dstlen:    maximum number of bytes to extract
 *
 * This function extracts at most @dstlen bytes from the data stream
 * previously associated with @state by gunzip_start(), decompressing
 * if necessary.  Exactly @dstlen bytes are extracted unless the data
 * stream doesn't contain enough bytes, in which case the entire
 * remainder of the stream is decompressed.
 *
 * Returns the actual number of bytes extracted.  If any errors occur,
 * such as a corrupted compressed stream, an error is printed an the
 * platform's exit() function is called.
 */
int gunzip_partial(struct gunzip_state *state, void *dst, int dstlen)
{
int i;
	if (LzmaDecode(state->workspace, ~0, state->lc,state->lp,state->pb,(unsigned char*)dst, dstlen, &i) == LZMA_RESULT_OK)
	    {
		if ( i != dstlen )
		   fatal( "lzma data corrupted!\n");
	    return i;
	    }
	return i;
}

/**
 * gunzip_exactly - extract a fixed number of bytes from a gzip data stream
 * @state:     gzip state structure previously initialized by gunzip_start()
 * @dst:       buffer to store extracted data
 * @dstlen:    number of bytes to extract
 *
 * This function extracts exactly @dstlen bytes from the data stream
 * previously associated with @state by gunzip_start(), decompressing
 * if necessary.
 *
 * If there are less @dstlen bytes available in the data stream, or if
 * any other errors occur, such as a corrupted compressed stream, an
 * error is printed an the platform's exit() function is called.
 */
void gunzip_exactly(struct gunzip_state *state, void *dst, int dstlen)
{
	int len;

	len  = gunzip_partial(state, dst, dstlen);
	if (len < dstlen)
		fatal("\n\rgunzip_exactly: ran out of data!"
				" Wanted %d, got %d.\n\r", dstlen, len);
}

/**
 * gunzip_discard - discard bytes from a gzip data stream
 * @state:     gzip state structure previously initialized by gunzip_start()
 * @len:       number of bytes to discard
 *
 * This function extracts, then discards exactly @len bytes from the
 * data stream previously associated with @state by gunzip_start().
 * Subsequent gunzip_partial(), gunzip_exactly() or gunzip_finish()
 * calls will extract the data following the discarded bytes in the
 * data stream.
 *
 * If there are less @len bytes available in the data stream, or if
 * any other errors occur, such as a corrupted compressed stream, an
 * error is printed an the platform's exit() function is called.
 */
void gunzip_discard(struct gunzip_state *state, int len)
{
	static char discard_buf[128];

	while (len > sizeof(discard_buf)) {
		gunzip_exactly(state, discard_buf, sizeof(discard_buf));
		len -= sizeof(discard_buf);
	}

	if (len > 0)
		gunzip_exactly(state, discard_buf, len);
}

/**
 * gunzip_finish - extract all remaining bytes from a gzip data stream
 * @state:     gzip state structure previously initialized by gunzip_start()
 * @dst:       buffer to store extracted data
 * @dstlen:    maximum number of bytes to extract
 *
 * This function extracts all remaining data, or at most @dstlen
 * bytes, from the stream previously associated with @state by
 * gunzip_start().  zlib is then shut down, so it is an error to use
 * any of the functions in this file on @state until it is
 * re-initialized with another call to gunzip_start().
 *
 * If any errors occur, such as a corrupted compressed stream, an
 * error is printed an the platform's exit() function is called.
 */
int gunzip_finish(struct gunzip_state *state, void *dst, int dstlen)
{
	int len;
	len = gunzip_partial(state, dst, dstlen);
	return len;
}
