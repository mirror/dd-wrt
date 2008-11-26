/*
 * Mark's Mythical Table-based raw HDLC implementation
 *
 * This is designed to be a very fast, but memory efficient
 * implementation of standard HDLC protocol.
 *
 * This table based HDLC technology is PATENT PENDING, but will always be
 * remain freely distributable under the terms of the GPL version 2.0. 
 *
 * For non-GPL licensing, please contact Mark Spencer at 
 * the below e-mail address.
 *
 * Copyright (C) 2001, Linux Support Services, Inc.
 *
 * Written by Mark Spencer <markster@linux-support.net>
 * 
 * Distributed under the terms of the GNU General Public License
 * Version 2.0.
 *
 */

#ifndef _FASTHDLC_H
#define _FASTHDLC_H

struct fasthdlc_state {
	int state;		/* What state we are in */
	unsigned int data;	/* Our current data queue */
	int bits;		/* Number of bits in our data queue */
	int ones;		/* Number of ones */
};

#ifdef FAST_HDLC_NEED_TABLES
#define RETURN_COMPLETE_FLAG	(0x1000)
#define RETURN_DISCARD_FLAG	(0x2000)
#define RETURN_EMPTY_FLAG	(0x4000)

/* Unlike most HDLC implementations, we define only two states,
   when we are in a valid frame, and when we are searching for
   a frame header */

#define FRAME_SEARCH	0
#define PROCESS_FRAME	1

/* 

   HDLC Search State table -- Look for a frame header.  The return value
   of this table is as follows:

  |---8---|---7---|---6---|---5---|---4---|---3---|---2---|---1---| 
  |      Z E R O E S      |  Next |         Bits Consumed         |
  |-------|-------|-------|-------|-------|-------|-------|-------|

   The indexes for this table are the state (0 or 1) and the next 8
   bits of the stream.

   Note that this table is only used for state 0 and 1.

   The user should discard the top "bits consumed" bits of data before
   the next call.  "Next state" represents the actual next state for
   decoding.

*/
static unsigned char hdlc_search[256];

/*
  HDLC Data Table

  The indexes to this table are the number of one's we've seen so far (0-5) and
  the next 10 bits of input (which is enough to guarantee us that we
  will retrieve at least one byte of data (or frame or whatever).

  The format for the return value is:

  Bits 15: Status (1=Valid Data, 0=Control Frame (see bits 7-0 for type))
  Bits 14-12: Number of ones in a row, so far
  Bits 11-8:  The number of bits consumed (0-10)
  Bits 7-0:   The return data (if appropriate)
  
  The next state is simply bit #15

*/

#define CONTROL_COMPLETE	1
#define CONTROL_ABORT		2

#define STATUS_MASK	(1 << 15)
#define STATUS_VALID	(1 << 15)
#define STATUS_CONTROL	(0 << 15)
#define STATE_MASK	(1 << 15)
#define ONES_MASK	(7 << 12)
#define	DATA_MASK	(0xff)

static unsigned short hdlc_frame[6][1024];

static unsigned int minbits[2] = { 8, 10 };

/*
   Last, but not least, we have the encoder table.  It takes
   as its indices the number of ones so far and a byte of data
   and returns an int composed of the following fields:

   Bots 31-22: Actual Data
   Bits 21-16: Unused
   Bits 15-8:  Number of ones
   Bits 3-0:   Number of bits of output (13-4) to use

   Of course we could optimize by reducing to two tables, but I don't
   really think it's worth the trouble at this point.
  */

static unsigned int hdlc_encode[6][256];

static inline char hdlc_search_precalc(unsigned char c)
{
	int x, p=0;
	/* Look for a flag.  If this isn't a flag,
	   line us up for the next possible shot at
	   a flag */

	/* If it's a flag, we go to state 1, and have
	   consumed 8 bits */
	if (c == 0x7e) 
		return 0x10 | 8;

	/* If it's an abort, we stay in the same state
	   and have consumed 8 bits */
	if (c == 0x7f)
		return 0x00 | 8;

	/* If it's all 1's, we state in the same state and
	   have consumed 8 bits */
	if (c == 0xff)
		return 0x00 | 8;

	/* If we get here, we must have at least one zero in us
	   but we're not the flag.  So, start at the end (LSB) and
	   work our way to the top (MSB) looking for a zero.  The 
	   position of that 0 is most optimistic start of a real
	   frame header */
	x=1;
	p=7;
	while(p && (c & x)) {
		x <<= 1;
		p--;
	}
	return p;
}

#ifdef DEBUG_PRECALC
static inline void hdlc_search_print(char c, char r)
{
	int x=0x80;
	while(x) {
		printf("%s", c & x ? "1" : "0");
		x >>= 1;
	}
	printf(" => State %d, Consume %d\n", (r & 0x10) >> 4, r & 0xf);
}
#endif

#define HFP(status, ones, bits, data) \
	((status) | ((ones) << 12) | ((bits) << 8) | (data))

static inline unsigned int hdlc_frame_precalc(unsigned char x, unsigned short c)
{
	/* Assume we have seen 'x' one's so far, and have read the
	   bottom 10 bytes of c (MSB first).  Now, we HAVE to have
	   a byte of data or a frame or something.  We are assumed
	   to be at the beginning of a byte of data or something */
	unsigned char ones = x;
	unsigned char data=0;
	int bits=0;
	int consumed=0;
	while(bits < 8) {
		data >>=1;
		consumed++;
		if (ones == 5) {
			/* We've seen five ones */
			if (c & 0x0200) {
				/* Another one -- Some sort of signal frame */
				if ((!(c & 0x0100)) && (bits == 6)) {
					/* This is a frame terminator (10) */
					return HFP(0, 
						   0, 8, CONTROL_COMPLETE);
				} else {
					/* Yuck!  It's something else...
					   Abort this entire frame, and
					   start looking for a good frame */
					return HFP(0, 
						   0, consumed+1, CONTROL_ABORT);
				}
			} else {
				/* It's an inserted zero, just skip it */
				ones = 0;
				data <<= 1;
			}
		} else {
			/* Add it to our bit list, LSB to
			   MSB */
			if (c & 0x0200) {
				data |= 0x80;
				ones++;
			} else 
				ones=0;
			bits++;	
		}
		c <<= 1;
	}
	/* Consume the extra 0 now rather than later. */
	if (ones == 5) {
		ones = 0;
		consumed++;
	}
	return HFP(STATUS_VALID, ones, consumed, data);
}

#ifdef DEBUG_PRECALC

static inline void hdlc_frame_print(unsigned char x, unsigned short c, unsigned int res)
{
	int z=0x0200;
	char *status[] = {
		"Control",
		"Valid",
	};
	printf("%d one's then ", x);
	while(z) {
		printf("%s", c & z ? "1" : "0");
		z >>= 1;
	}
	printf(" => Status %s, ", res & STATUS_MASK ? "1" : "0");
	printf("Consumed: %d, ", (res & 0x0f00) >> 8);
	printf("Status: %s, ", status[(res & STATUS_MASK) >> 15]);
	printf("Ones: %d, ", (res & ONES_MASK) >> 12);
	printf("Data: %02x\n", res & 0xff);
	
}

#endif

static inline unsigned int hdlc_encode_precalc(int x, unsigned char y)
{
	int bits=0;
	int ones=x;
	unsigned short data=0;
	int z;
	for (z=0;z<8;z++) {
		/* Zero-stuff if needed */
		if (ones == 5) {
			/* Stuff a zero */
			data <<= 1;
			ones=0;
			bits++;
		}
		if (y & 0x01) {
			/* There's a one */
			data <<= 1;
			data |= 0x1;
			ones++;
			bits++;
		} else {
			data <<= 1;
			ones = 0;
			bits++;
		}
		y >>= 1;
	}
	/* Special case -- Stuff the zero at the end if appropriate */
	if (ones == 5) {
		/* Stuff a zero */
		data <<= 1;
		ones=0;
		bits++;
	}
	data <<= (10-bits);
	return (data << 22) | (ones << 8) | (bits);
}

#ifdef DEBUG_PRECALC
static inline void hdlc_encode_print(int x, unsigned char y, unsigned int val)
{
	unsigned int z;
	unsigned short c;
	printf("%d ones, %02x (", x, y);
	z = 0x80;
	while(z) {
		printf("%s", y & z ? "1" : "0");
		z >>= 1;
	}
	printf(") encoded as ");
	z = 1 << 31;
	for (x=0;x<(val & 0xf);x++) {
		printf("%s", val & z ? "1" : "0");
		z >>= 1;
	}
	printf(" with %d ones now, %d bits in len\n", (val & 0xf00) >> 8, val & 0xf);

		
}
#endif

static inline void fasthdlc_precalc(void)
{
	int x;
	int y;
	/* First the easy part -- the searching */
	for (x=0;x<256;x++) {
		hdlc_search[x] = hdlc_search_precalc(x);
#ifdef DEBUG_PRECALC
		hdlc_search_print(x, hdlc_search[x]);
#endif
	}
	/* Now the hard part -- the frame tables */
	for (x=0;x<6;x++) {
		/* Given the # of preceeding ones, process the next
		   byte of input (up to 10 actual bits) */
		for (y=0;y<1024;y++) {
			hdlc_frame[x][y] = hdlc_frame_precalc(x, y);
#ifdef DEBUG_PRECALC
			hdlc_frame_print(x, y, hdlc_frame[x][y]);
#endif
		}
	}
	/* Now another not-so-hard part, the encoding table */
	for (x=0;x<6;x++) {
		for (y=0;y<256;y++) {
			hdlc_encode[x][y] = hdlc_encode_precalc(x,y);
#ifdef DEBUG_PRECALC
			hdlc_encode_print(x,y,hdlc_encode[x][y]);
#endif
		}
	}
}


static inline void fasthdlc_init(struct fasthdlc_state *h)
{
	/* Initializes all states appropriately */
	h->state = 0;
	h->bits = 0;
	h->data = 0;
	h->ones = 0;

}

static inline int fasthdlc_tx_load_nocheck(struct fasthdlc_state *h, unsigned char c)
{
	unsigned int res;
	res = hdlc_encode[h->ones][c];
	h->ones = (res & 0xf00) >> 8;
	h->data |= (res & 0xffc00000) >> h->bits;
	h->bits += (res & 0xf);
	return 0;
}

static inline int fasthdlc_tx_load(struct fasthdlc_state *h, unsigned char c)
{
	/* Gotta have at least 10 bits left */
	if (h->bits > 22) 
		return -1;
	return fasthdlc_tx_load_nocheck(h, c);
}

static inline int fasthdlc_tx_frame_nocheck(struct fasthdlc_state *h)
{
	h->ones = 0;
	h->data |= ( 0x7e000000 >> h->bits);
	h->bits += 8;
	return 0;
}

static inline int fasthdlc_tx_frame(struct fasthdlc_state *h)
{
	if (h->bits > 24)
		return -1;
	return fasthdlc_tx_frame_nocheck(h);
}

static inline int fasthdlc_tx_run_nocheck(struct fasthdlc_state *h)
{
	unsigned char b;
	b = h->data >> 24;
	h->bits -= 8;
	h->data <<= 8;
	return b;
}

static inline int fasthdlc_tx_run(struct fasthdlc_state *h)
{
	if (h->bits < 8)
		return -1;
	return fasthdlc_tx_run_nocheck(h);
}

static inline int fasthdlc_rx_load_nocheck(struct fasthdlc_state *h, unsigned char b)
{
	/* Put the new byte in the data stream */
	h->data |= b << (24-h->bits);
	h->bits += 8;
	return 0;
}

static inline int fasthdlc_rx_load(struct fasthdlc_state *h, unsigned char b)
{
	/* Make sure we have enough space */
	if (h->bits > 24)
		return -1;
	return fasthdlc_rx_load_nocheck(h, b);
}

/*
   Returns a data character if available, logical OR'd with 
   zero or more of RETURN_COMPLETE_FLAG, RETURN_DISCARD_FLAG,
   and RETURN_EMPTY_FLAG, signifying a complete frame, a
   discarded frame, or there is nothing to return.
   */

static inline int fasthdlc_rx_run(struct fasthdlc_state *h)
{
	unsigned short next;
	int retval=RETURN_EMPTY_FLAG;
	while ((h->bits >= minbits[h->state]) && (retval == RETURN_EMPTY_FLAG)) {
		/* Run until we can no longer be assured that we will
		   have enough bits to continue */
		switch(h->state) {
		case FRAME_SEARCH:
			/* Look for an HDLC frame, keying from
			   the top byte.  */
			next = hdlc_search[h->data >> 24];
			h->bits -= next & 0x0f;
			h->data <<= next & 0x0f;
			h->state = next >> 4;
			h->ones = 0;
			break;
		case PROCESS_FRAME:
			/* Process as much as the next ten bits */
			next = hdlc_frame[h->ones][h->data >> 22];
			h->bits  -= ((next & 0x0f00) >> 8);
			h->data <<= ((next & 0x0f00) >> 8);
			h->state = (next & STATE_MASK) >> 15;
			h->ones = (next & ONES_MASK) >> 12;
			switch(next & STATUS_MASK) {
			case STATUS_CONTROL:
				if (next & CONTROL_COMPLETE) {
					/* A complete, valid frame received */
					retval = (RETURN_COMPLETE_FLAG);
					/* Stay in this state */
					h->state = 1;
				} else {
				/* An abort (either out of sync of explicit) */
					retval = (RETURN_DISCARD_FLAG);
				}
				break;
			case STATUS_VALID:
				retval = (next & DATA_MASK);
			}
		}
	}
	return retval;
}
#endif /* FAST_HDLC_NEED_TABLES */
#endif
