/* $Id: rawhdlc.c,v 1.7 2001/09/24 13:22:57 kai Exp $
 *
 * support routines for cards that don't support HDLC
 *
 * Author     Brent Baccala
 * Copyright  by Karsten Keil <keil@isdn4linux.de>
 *            by Brent Baccala <baccala@FreeSoft.org>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 *
 * Some passive ISDN cards, such as the Traverse NETJet and the AMD 7930,
 * don't perform HDLC encapsulation over the B channel.  Drivers for
 * such cards use support routines in this file to perform B channel HDLC.
 *
 * Bit-synchronous HDLC encapsulation is a means of encapsulating packets
 * over a continuously transmitting serial communications link.
 * It looks like this:
 *
 *      11111111101111110...........0111111011111111111
 *      iiiiiiiiiffffffffdddddddddddffffffffiiiiiiiiiii
 *
 *      i = idle     f = flag     d = data
 *
 * When idle, the channel sends a continuous string of ones (mark
 * idle; illustrated), or a continuous string of flag characters (flag
 * idle).  The beginning of a data frame is marked by a flag character
 * (01111110), then comes the actual data, followed by another flag
 * character, after which another frame may be sent immediately (a
 * single flag may serve as both the end of one frame and the start of
 * the next), or the link may return to idle.  Obviously, the flag
 * character can not appear anywhere in the data (or a false
 * end-of-frame would occur), so the transmitter performs
 * "bit-stuffing" - inserting a zero bit after every five one bits,
 * irregardless of the original bit after the five ones.  Byte
 * ordering is irrelevent at this point - the data is treated as a
 * string of bits, not bytes.  Since no more than 5 ones may now occur
 * in a row, the flag sequence, with its 6 ones, is unique.
 *
 * Upon reception, a zero bit that occur after 5 one bits is simply
 * discarded.  A series of 6 one bits is end-of-frame, and a series of
 * 7 one bits is an abort.  Once bit-stuffing has been corrected for,
 * an integer number of bytes should now be present.  The last two
 * of these bytes form the Frame Check Sequence, a CRC that is verified
 * and then discarded.  Note that bit-stuffing is performed on the FCS
 * just as if it were regular data.
 *
 *
 *
 * int make_raw_hdlc_data(u_char *src, u_int slen,
 *                        u_char *dst, u_int dsize)
 *
 *   Used for transmission.  Copies slen bytes from src to dst, performing
 *   HDLC encapsulation (flag bytes, bit-stuffing, CRC) in the process.
 *   dsize is size of destination buffer, and should be at least
 *   ((6*slen)/5)+5 bytes to ensure adequate space will be available.
 *   Function returns length (in bytes) of valid destination buffer, or
 *   0 upon destination overflow.
 *
 * void init_hdlc_state(struct hdlc_state *stateptr, int mode)
 *
 *   Initializes hdlc_state structure before first call to read_raw_hdlc_data
 *
 *   mode = 0: Sane mode
 *   mode = 1/2: 
 *             Insane mode; NETJet use a shared unsigned int memory block (
 * 	       with busmaster DMA), the bit pattern of every word is 
 *  	       <8 B1> <8 B2> <8 Mon> <2 D> <4 C/I> <MX> <MR>
 *	       according to Siemens IOM-2 interface, so we have to handle
 *             the src buffer as unsigned int and have to shift/mask the
 *             B-channel bytes.
 *             mode 1 -> B1  mode 2  -> B2 data is used
 *
 * int read_raw_hdlc_data(struct hdlc_state *saved_state,
 *                        u_char *src, u_int slen,
 *                        u_char *dst, u_int dsize)
 *
 *   Used for reception.  Scans source buffer bit-by-bit looking for
 *   valid HDLC frames, which are copied to destination buffer.  HDLC
 *   state information is stored in a structure, which allows this
 *   function to process frames spread across several blocks of raw
 *   HDLC data.  Part of the state information is bit offsets into
 *   the source and destination buffers.
 *
 *   A return value >0 indicates the length of a valid frame, now
 *   stored in the destination buffer.  In this case, the source
 *   buffer might not be completely processed, so this function should
 *   be called again with the same source buffer, possibly with a
 *   different destination buffer.
 *
 *   A return value of zero indicates that the source buffer was
 *   completely processed without finding a valid end-of-packet;
 *   however, we might be in the middle of packet reception, so
 *   the function should be called again with the next block of
 *   raw HDLC data and the same destination buffer.  It is NOT
 *   permitted to change the destination buffer in this case,
 *   since data may already have begun to be stored there.
 *
 *   A return value of -1 indicates some kind of error - destination
 *   buffer overflow, CRC check failed, frame not a multiple of 8
 *   bits.  Destination buffer probably contains invalid data, which
 *   should be discarded.  Call function again with same source buffer
 *   and a new (or same) destination buffer.
 *
 *   Suggested calling sequence:
 *
 *      init_hdlc_state(...);
 *      for (EACH_RAW_DATA_BLOCK) {
 *         while (len = read_raw_hdlc_data(...)) {
 *             if (len == -1) DISCARD_FRAME;
 *             else PROCESS_FRAME;
 *         }
 *      }
 *
 *
 * Test the code in this file as follows:
 *    gcc -DDEBUGME -o rawhdlctest rawhdlc.c
 *    ./rawhdlctest < rawdata
 *
 * The file "rawdata" can be easily generated from a HISAX B-channel
 * hex dump (CF CF CF 02 ...) using the following perl script:
 *
 * while(<>) {
 *     @hexlist = split ' ';
 *     while ($hexstr = shift(@hexlist)) {
 *         printf "%c", hex($hexstr);
 *     }
 * }
 *
 */

#ifdef DEBUGME
#include <stdio.h>
#endif

#include <linux/types.h>
#include <linux/ppp_defs.h>
#include "rawhdlc.h"

/* There's actually an identical copy of this table in the PPP code
 * (ppp_crc16_table), but I don't want this code dependent on PPP
 */

// static 
__u16 fcstab[256] =
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#define HDLC_ZERO_SEARCH 0
#define HDLC_FLAG_SEARCH 1
#define HDLC_FLAG_FOUND  2
#define HDLC_FRAME_FOUND 3
#define HDLC_NULL 4
#define HDLC_PART 5
#define HDLC_FULL 6

#define HDLC_FLAG_VALUE	0x7e


#define MAKE_RAW_BYTE for (j=0; j<8; j++) { \
			bitcnt++;\
			out_val >>= 1;\
			if (val & 1) {\
				s_one++;\
				out_val |= 0x80;\
			} else {\
				s_one = 0;\
				out_val &= 0x7f;\
			}\
			if (bitcnt==8) {\
				if (d_cnt == dsize) return 0;\
				dst[d_cnt++] = out_val;\
				bitcnt = 0;\
			}\
			if (s_one == 5) {\
				out_val >>= 1;\
				out_val &= 0x7f;\
				bitcnt++;\
				s_one = 0;\
			}\
			if (bitcnt==8) {\
				if (d_cnt == dsize) return 0;\
				dst[d_cnt++] = out_val;\
				bitcnt = 0;\
			}\
			val >>= 1;\
		}

/* Optimization suggestion: If needed, this function could be
 * dramatically sped up using a state machine.  Each state would
 * correspond to having seen N one bits, and being offset M bits into
 * the current output byte.  N ranges from 0 to 4, M from 0 to 7, so
 * we need 5*8 = 35 states.  Each state would have a table with 256
 * entries, one for each input character.  Each entry would contain
 * three output characters, an output state, an a byte increment
 * that's either 1 or 2.  All this could fit in four bytes; so we need
 * 4 bytes * 256 characters = 1 KB for each state (35 KB total).  Zero
 * the output buffer before you start.  For each character in your
 * input, you look it up in the current state's table and get three
 * bytes to be or'ed into the output at the current byte offset, and
 * an byte increment to move your pointer forward.  A simple Perl
 * script could generate the tables.  Given HDLC semantics, probably
 * would be better to set output to all 1s, then use ands instead of ors.
 * A smaller state machine could operate on nibbles instead of bytes.
 * A state machine for 32-bit architectures could use word offsets
 * instead of byte offsets, requiring 5*32 = 160 states; probably
 * best to work on nibbles in such a case.
 */


int make_raw_hdlc_data(u_char *src, u_int slen, u_char *dst, u_int dsize)
{
	register u_int i,d_cnt=0;
	register u_char j;
	register u_char val;
	register u_char s_one = 0;
	register u_char out_val = 0;
	register u_char bitcnt = 0;
	u_int fcs;
	
	
	dst[d_cnt++] = HDLC_FLAG_VALUE;
	fcs = PPP_INITFCS;
	for (i=0; i<slen; i++) {
		val = src[i];
		fcs = PPP_FCS (fcs, val);
		MAKE_RAW_BYTE;
	}
	fcs ^= 0xffff;
	val = fcs & 0xff;
	MAKE_RAW_BYTE;
	val = (fcs>>8) & 0xff;
	MAKE_RAW_BYTE;
	val = HDLC_FLAG_VALUE;
	for (j=0; j<8; j++) { 
		bitcnt++;
		out_val >>= 1;
		if (val & 1)
			out_val |= 0x80;
		else
			out_val &= 0x7f;
		if (bitcnt==8) {
			if (d_cnt == dsize) return 0;
			dst[d_cnt++] = out_val;
			bitcnt = 0;
		}
		val >>= 1;
	}
	if (bitcnt) {
		while (8>bitcnt++) {
			out_val >>= 1;
			out_val |= 0x80;
		}
		if (d_cnt == dsize) return 0;
		dst[d_cnt++] = out_val;
	}

	return d_cnt;
}

void init_hdlc_state(struct hdlc_state *stateptr, int mode)
{
	stateptr->state = HDLC_ZERO_SEARCH;
	stateptr->r_one = 0;
	stateptr->r_val = 0;
	stateptr->o_bitcnt = 0;
	stateptr->i_bitcnt = 0;
	stateptr->insane_mode = mode;
}

/* Optimization suggestion: A similar state machine could surely
 * be developed for this function as well.
 */

int read_raw_hdlc_data(struct hdlc_state *saved_state,
                       u_char *src, u_int slen, u_char *dst, u_int dsize)
{
	int retval=0;
	register u_char val;
	register u_char state = saved_state->state;
	register u_char r_one = saved_state->r_one;
	register u_char r_val = saved_state->r_val;
	register u_int o_bitcnt = saved_state->o_bitcnt;
	register u_int i_bitcnt = saved_state->i_bitcnt;
	register u_int fcs    = saved_state->fcs;
	register u_int *isrc = (u_int *) src;
        
	/* Use i_bitcnt (bit offset into source buffer) to reload "val"
	 * in case we're starting up again partway through a source buffer
	 */

	if ((i_bitcnt >> 3) < slen) {
		if (saved_state->insane_mode==1) {
			val = isrc[(i_bitcnt >> 3)] & 0xff;
		} else if (saved_state->insane_mode==2) {
			val = (isrc[i_bitcnt >> 3] >>8) & 0xff;
		} else {
			val = src[i_bitcnt >> 3];
		}
		val >>= i_bitcnt & 7;
	}

	/* One bit per loop.  Keep going until we've got something to
	 * report (retval != 0), or we exhaust the source buffer
	 */

	while ((retval == 0) && ((i_bitcnt >> 3) < slen)) {
		if ((i_bitcnt & 7) == 0) {
			if (saved_state->insane_mode==1) {
				val = isrc[(i_bitcnt >> 3)] & 0xff;
			} else if (saved_state->insane_mode==2) {
				val = (isrc[i_bitcnt >> 3] >>8) & 0xff;
			} else {
				val = src[i_bitcnt >> 3];
			}
#ifdef DEBUGME
			printf("Input byte %d: 0x%2x\n", i_bitcnt>>3, val);
#endif
			if (val == 0xff) {
				state = HDLC_ZERO_SEARCH;
				o_bitcnt = 0;
				r_one = 0;
				i_bitcnt += 8;
				continue;
			}
		}

#ifdef DEBUGME
		/* printf("Data bit=%d (%d/%d)\n", val&1, i_bitcnt>>3, i_bitcnt&7);*/
#endif

		if (state == HDLC_ZERO_SEARCH) {
			if (val & 1) {
				r_one++;
			} else {
				r_one=0;
				state= HDLC_FLAG_SEARCH;
			}
		} else if (state == HDLC_FLAG_SEARCH) { 
			if (val & 1) {
				r_one++;
				if (r_one>6) {
					state=HDLC_ZERO_SEARCH;
				}
			} else {
				if (r_one==6) {
					o_bitcnt=0;
					r_val=0;
					state=HDLC_FLAG_FOUND;
				}
				r_one=0;
			}
		} else if (state ==  HDLC_FLAG_FOUND) {
			if (val & 1) {
				r_one++;
				if (r_one>6) {
					state=HDLC_ZERO_SEARCH;
				} else {
					r_val >>= 1;
					r_val |= 0x80;
					o_bitcnt++;
				}
			} else {
				if (r_one==6) {
					o_bitcnt=0;
					r_val=0;
					r_one=0;
					i_bitcnt++;
					val >>= 1;
					continue;
				} else if (r_one!=5) {
					r_val >>= 1;
					r_val &= 0x7f;
					o_bitcnt++;
				}
				r_one=0;	
			}
			if ((state != HDLC_ZERO_SEARCH) &&
				!(o_bitcnt & 7)) {
#ifdef DEBUGME
				printf("HDLC_FRAME_FOUND at i_bitcnt:%d\n",i_bitcnt);
#endif
				state=HDLC_FRAME_FOUND;
				fcs = PPP_INITFCS;
				dst[0] = r_val;
				fcs = PPP_FCS (fcs, r_val);
			}
		} else if (state ==  HDLC_FRAME_FOUND) {
			if (val & 1) {
				r_one++;
				if (r_one>6) {
					state=HDLC_ZERO_SEARCH;
					o_bitcnt=0;
				} else {
					r_val >>= 1;
					r_val |= 0x80;
					o_bitcnt++;
				}
			} else {
				if (r_one==6) {
					r_val=0; 
					r_one=0;
					o_bitcnt++;
					if (o_bitcnt & 7) {
						/* Alignment error */
#ifdef DEBUGME
						printf("Alignment error\n");
#endif
						state=HDLC_FLAG_SEARCH;
						retval = -1;
					} else if (fcs==PPP_GOODFCS) {
						/* Valid frame */
						state=HDLC_FLAG_FOUND;
						retval = (o_bitcnt>>3)-3;
					} else {
						/* CRC error */
#ifdef DEBUGME
						printf("CRC error; fcs was 0x%x, should have been 0x%x\n", fcs, PPP_GOODFCS);
#endif
						state=HDLC_FLAG_FOUND;
						retval = -1;
					}
				} else if (r_one==5) {
					r_one=0;
					i_bitcnt++;
					val >>= 1;
					continue;
				} else {
					r_val >>= 1;
					r_val &= 0x7f;
					o_bitcnt++;
				}
				r_one=0;	
			}
			if ((state == HDLC_FRAME_FOUND) &&
				!(o_bitcnt & 7)) {
				if ((o_bitcnt>>3)>=dsize) {
					/* Buffer overflow error */
#ifdef DEBUGME
					printf("Buffer overflow error\n");
#endif
					r_val=0; 
					state=HDLC_FLAG_SEARCH;
					retval = -1;
				} else {
					dst[(o_bitcnt>>3)-1] = r_val;
					fcs = PPP_FCS (fcs, r_val);
#ifdef DEBUGME
					printf("Output byte %d: 0x%02x; FCS 0x%04x\n", (o_bitcnt>>3)-1, r_val, fcs);
#endif
				}
			}
		}
		i_bitcnt ++;
		val >>= 1;
	}

	/* We exhausted the source buffer before anything else happened
	 * (retval==0).  Reset i_bitcnt in expectation of a new source
	 * buffer.  Other, we either had an error or a valid frame, so
	 * reset o_bitcnt in expectation of a new destination buffer.
	 */

	if (retval == 0) {
		i_bitcnt = 0;
	} else {
		o_bitcnt = 0;
	}

	saved_state->state = state;
	saved_state->r_one = r_one;
	saved_state->r_val = r_val;
	saved_state->fcs = fcs;
	saved_state->o_bitcnt = o_bitcnt;
	saved_state->i_bitcnt = i_bitcnt;

	return (retval);
}



#ifdef DEBUGME

char buffer[1024];
char obuffer[1024];

main()
{
  int buflen=0;
  int len;
  struct hdlc_state hdlc_state;

  while((buffer[buflen] = getc(stdin)) != EOF && buflen<1024) buflen++;

  printf("buflen = %d\n", buflen);

  init_hdlc_state(&hdlc_state, 0);

  while (len = read_raw_hdlc_data(&hdlc_state,buffer,buflen,obuffer,1024)) {
    if (len == -1) printf("Error @ byte %d/bit %d\n",
			  hdlc_state.i_bitcnt>>3, hdlc_state.i_bitcnt & 7);
    else {
      printf("Frame received: len %d\n", len);
    }
  }

  printf("Done\n");
}

#endif
