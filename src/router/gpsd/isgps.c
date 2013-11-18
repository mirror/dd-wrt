/* $Id: isgps.c 4065 2006-12-04 05:31:59Z esr $ */
/*****************************************************************************

This is a decoder for the unnamed protocol described in IS-GPS-200,
the Navstar GPS Interface Specification, and used as a transport layer
for both GPS satellite downlink transmissions and the RTCM104 format
for broadcasting differential-GPS corrections.

This lower layer just handles synchronizing with the incoming
bitstream and parity checking; all it does is assemble message
packets.  It needs an upper layer to analyze the packets into
bitfields and then assemble the bitfields into usable data.  

The upper layer must supply a preamble_match() hook to tell our
decoder when it has a legitimate start of packet, and a length_check()
hook to tell it when the packet has reached the length it is supposed
to have.

Here are Wolfgang's original rather cryptic notes on this code:

--------------------------------------------------------------------------
1) trim and bitflip the input.

While syncing the msb of the input gets shifted into lsb of the
assembled word.  
    word <<= 1, or in input >> 5 
    word <<= 1, or in input >> 4
    word <<= 1, or in input >> 3
    word <<= 1, or in input >> 2 
    word <<= 1, or in input >> 1 
    word <<= 1, or in input

At one point it should sync-lock.

----

Shift 6 bytes of RTCM data in as such:

---> (trim-bits-to-5-bits) ---> (end-for-end-bit-flip) ---> 

---> shift-into-30-bit-shift-register
              |||||||||||||||||||||||
	      detector-for-preamble
              |||||||||||||||||||||||
              detector-for-parity
              |||||||||||||||||||||||
--------------------------------------------------------------------------

The code was originally by Wolfgang Rupprecht.  ESR severely hacked
it, with Wolfgang's help, in order to separate message analysis from
message dumping and separate this lower layer from the upper layer 
handing RTCM decoding.  You are not expected to understand any of this.

*****************************************************************************/

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gpsd_config.h"
#include "gpsd.h"

#define MAG_SHIFT 6u
#define MAG_TAG_DATA (1 << MAG_SHIFT)
#define MAG_TAG_MASK (3 << MAG_SHIFT)

#define W_DATA_MASK	0x3fffffc0u

/*@ +charint @*/
static unsigned char   parity_array[] = {
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

static unsigned int reverse_bits[] = {
    0, 32, 16, 48, 8, 40, 24, 56, 4, 36, 20, 52, 12, 44, 28, 60,
    2, 34, 18, 50, 10, 42, 26, 58, 6, 38, 22, 54, 14, 46, 30, 62,
    1, 33, 17, 49, 9, 41, 25, 57, 5, 37, 21, 53, 13, 45, 29, 61,
    3, 35, 19, 51, 11, 43, 27, 59, 7, 39, 23, 55, 15, 47, 31, 63
};
/*@ -charint @*/

unsigned int isgps_parity(isgps30bits_t th)
{
#define P_30_MASK	0x40000000u

#define	PARITY_25	0xbb1f3480u
#define	PARITY_26	0x5d8f9a40u
#define	PARITY_27	0xaec7cd00u
#define	PARITY_28	0x5763e680u
#define	PARITY_29	0x6bb1f340u
#define	PARITY_30	0x8b7a89c0u
    isgps30bits_t        t;
    unsigned int    p;

    /*
    if (th & P_30_MASK)
	th ^= W_DATA_MASK;
    */

    /*@ +charint @*/
    t = th & PARITY_25;
    p = parity_array[t & 0xff] ^ parity_array[(t >> 8) & 0xff] ^
	parity_array[(t >> 16) & 0xff] ^ parity_array[(t >> 24) & 0xff];
    t = th & PARITY_26;
    p = (p << 1) | (parity_array[t & 0xff] ^ parity_array[(t >> 8) & 0xff] ^
		  parity_array[(t >> 16) & 0xff] ^ parity_array[(t >> 24) & 0xff]);
    t = th & PARITY_27;
    p = (p << 1) | (parity_array[t & 0xff] ^ parity_array[(t >> 8) & 0xff] ^
		  parity_array[(t >> 16) & 0xff] ^ parity_array[(t >> 24) & 0xff]);
    t = th & PARITY_28;
    p = (p << 1) | (parity_array[t & 0xff] ^ parity_array[(t >> 8) & 0xff] ^
		  parity_array[(t >> 16) & 0xff] ^ parity_array[(t >> 24) & 0xff]);
    t = th & PARITY_29;
    p = (p << 1) | (parity_array[t & 0xff] ^ parity_array[(t >> 8) & 0xff] ^
		  parity_array[(t >> 16) & 0xff] ^ parity_array[(t >> 24) & 0xff]);
    t = th & PARITY_30;
    p = (p << 1) | (parity_array[t & 0xff] ^ parity_array[(t >> 8) & 0xff] ^
		  parity_array[(t >> 16) & 0xff] ^ parity_array[(t >> 24) & 0xff]);
    /*@ -charint @*/

    gpsd_report(ISGPS_ERRLEVEL_BASE+2, "ISGPS parity %u\n", p);
    return (p);
}


#define isgps_parityok(w)	(isgps_parity(w) == ((w) & 0x3f))

#if 0
/* 
 * ESR found a doozy of a bug...
 *
 * Defining the above as a function triggers an optimizer bug in gcc 3.4.2.
 * The symptom is that parity computation is screwed up and the decoder
 * never achieves sync lock.  Something steps on the argument to 
 * isgpsparity(); the lossage appears to be related to the compiler's 
 * attempt to fold the isgps_parity() call into isgps_parityok() in some
 * tail-recursion-like manner.  This happens under -O2, but not -O1, on
 * both i386 and amd64.  Disabling all of the individual -O2 suboptions
 * does *not* fix it.
 *
 * And the fun doesn't stop there! It turns out that even with this fix, bare
 * -O2 generates bad code.  It takes "-O2 -fschedule-insns" to generate good
 * code under 3.4.[23]...which is weird because -O2 is supposed to *imply*
 * -fschedule-insns.
 *
 *  gcc 4.0 does not manifest these bugs.
 */
static bool isgps_parityok(isgps30bits_t w)
{
    return (isgpsparity(w) == (w & 0x3f));
}
#endif

void isgps_init(/*@out@*/struct gps_packet_t *session)
{
    session->isgps.curr_word = 0;
    session->isgps.curr_offset = 24;	/* first word */
    session->isgps.locked = false;
    session->isgps.bufindex = 0;
}

/*@ -usereleased -compdef @*/
enum isgpsstat_t isgps_decode(struct gps_packet_t *session, 
				     bool (*preamble_match)(isgps30bits_t *),
				     bool (*length_check)(struct gps_packet_t *),
			      size_t maxlen,
				     unsigned int c)
{
    enum isgpsstat_t res;

    /* ASCII characters 64-127, @ through DEL */
    if ((c & MAG_TAG_MASK) != MAG_TAG_DATA) {
	gpsd_report(ISGPS_ERRLEVEL_BASE+1, 
		    "ISGPS word tag not correct, skipping\n");
	return ISGPS_SKIP;
    }

    c = reverse_bits[c & 0x3f];

    /*@ -shiftnegative @*/
    if (!session->isgps.locked) {
	session->isgps.curr_offset = -5;
	session->isgps.bufindex = 0;

	while (session->isgps.curr_offset <= 0) {
	    session->isgps.curr_word <<= 1;
	    if (session->isgps.curr_offset > 0) {
		session->isgps.curr_word |= c << session->isgps.curr_offset;
	    } else {
		session->isgps.curr_word |= c >> -(session->isgps.curr_offset);
	    }
	    gpsd_report(ISGPS_ERRLEVEL_BASE+2, "ISGPS syncing at byte %d: %0x%08x\n", session->char_counter, session->isgps.curr_word);

	    if (preamble_match(&session->isgps.curr_word)) {
		if (isgps_parityok(session->isgps.curr_word)) {
		    gpsd_report(ISGPS_ERRLEVEL_BASE+1, 
				"ISGPS preamble ok, parity ok -- locked\n");
		    session->isgps.locked = true;
		    /* session->isgps.curr_offset;  XXX - testing */
		    break;
		}
		gpsd_report(ISGPS_ERRLEVEL_BASE+1, 
			    "ISGPS preamble ok, parity fail\n");
	    }
	    session->isgps.curr_offset++;
	}			/* end while */
    }
    if (session->isgps.locked) {
	res = ISGPS_SYNC;

	if (session->isgps.curr_offset > 0) {
	    session->isgps.curr_word |= c << session->isgps.curr_offset;
	} else {
	    session->isgps.curr_word |= c >> -(session->isgps.curr_offset);
	}

	if (session->isgps.curr_offset <= 0) {
	    /* weird-assed inversion */
	    if (session->isgps.curr_word & P_30_MASK)
		session->isgps.curr_word ^= W_DATA_MASK;

	    if (isgps_parityok(session->isgps.curr_word)) {
#if 0
		/*
		 * Don't clobber the buffer just because we spot
		 * another preamble pattern in the data stream. -wsr
		 */
		if (preamble_match(&session->isgps.curr_word)) {
		    gpsd_report(ISGPS_ERRLEVEL_BASE+2, 
				"ISGPS preamble spotted (index: %u)\n",
				session->isgps.bufindex);
		    session->isgps.bufindex = 0;
		}
#endif
		gpsd_report(ISGPS_ERRLEVEL_BASE+2,
			    "ISGPS processing word %u (offset %d)\n",
			    session->isgps.bufindex, session->isgps.curr_offset);
		{
		    /*
		     * Guard against a buffer overflow attack.  Just wait for
		     * the next preamble match and go on from there. 
		     */
		    if (session->isgps.bufindex >= (unsigned)maxlen){
			session->isgps.bufindex = 0;
			gpsd_report(ISGPS_ERRLEVEL_BASE+1, 
				    "ISGPS buffer overflowing -- resetting\n");
			return ISGPS_NO_SYNC;
		    }

		    session->isgps.buf[session->isgps.bufindex] = session->isgps.curr_word;

		    if ((session->isgps.bufindex == 0) &&
			!preamble_match((isgps30bits_t *)session->isgps.buf)) {
			gpsd_report(ISGPS_ERRLEVEL_BASE+1, 
				    "ISGPS word 0 not a preamble- punting\n");
			return ISGPS_NO_SYNC;
		    }
		    session->isgps.bufindex++;

		    if (length_check(session)) {
			/* jackpot, we have a complete packet*/
			session->isgps.bufindex = 0;
			res = ISGPS_MESSAGE;
		    }
		}
		session->isgps.curr_word <<= 30;	/* preserve the 2 low bits */
		session->isgps.curr_offset += 30;
		if (session->isgps.curr_offset > 0) {
		    session->isgps.curr_word |= c << session->isgps.curr_offset;
		} else {
		    session->isgps.curr_word |= c >> -(session->isgps.curr_offset);
		}
	    } else {
		gpsd_report(ISGPS_ERRLEVEL_BASE+0, 
			    "ISGPS parity failure, lost lock\n");
		session->isgps.locked = false;
	    }
	}
	session->isgps.curr_offset -= 6;
	gpsd_report(ISGPS_ERRLEVEL_BASE+2, "residual %d\n", session->isgps.curr_offset);
	return res;
    }
    /*@ +shiftnegative @*/

    /* never achieved lock */
    gpsd_report(ISGPS_ERRLEVEL_BASE+1, 
		"lock never achieved\n");
    return ISGPS_NO_SYNC;
}
/*@ +usereleased +compdef @*/

#ifdef __UNUSED__
void isgps_output_magnavox(isgps30bits_t *ip, unsigned int len, FILE *fp)
/* ship an IS-GPS-200 message to standard output in Magnavox format */
{
    isgps30bits_t w = 0;

    while (len-- > 0) {
	w <<= 30;
	w |= *ip++ & W_DATA_MASK;

	w |= isgps_parity(w);

	/* weird-assed inversion */
	if (w & P_30_MASK)
	    w ^= W_DATA_MASK;

	/*
	 * Write each 30-bit IS-GPS-200 data word as 5 Magnavox-format bytes
	 * with data in the low 6-bits of the byte.  MSB first.
	 */
	/*@ -type @*/
	(void)fputc(MAG_TAG_DATA | reverse_bits[(w >> 24) & 0x3f], fp);
	(void)fputc(MAG_TAG_DATA | reverse_bits[(w >> 18) & 0x3f], fp);
	(void)fputc(MAG_TAG_DATA | reverse_bits[(w >> 12) & 0x3f], fp);
	(void)fputc(MAG_TAG_DATA | reverse_bits[(w >> 6) & 0x3f], fp);
	(void)fputc(MAG_TAG_DATA | reverse_bits[(w) & 0x3f], fp);
	/*@ +type @*/
    }
}
#endif /* __UNUSED__ */
