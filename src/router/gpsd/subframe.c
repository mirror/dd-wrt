/* $Id: subframe.c 3770 2006-11-02 05:07:11Z esr $ */
/* subframe.c -- interpret satellite subframe data. */
#include <sys/types.h>
#include "gpsd_config.h"
#include "gpsd.h"

/*@ -usedef @*/
void gpsd_interpret_subframe(struct gps_device_t *session,unsigned int words[])
/* extract leap-second from RTCM-104 subframe data */
{
    /*
     * Heavy black magic begins here!
     *
     * A description of how to decode these bits is at
     * <http://home-2.worldonline.nl/~samsvl/nav2eu.htm>
     *
     * We're after subframe 4 page 18 word 9, the leap year correction.
     * We assume that the chip is presenting clean data that has been
     * parity-checked.
     *
     * To date this code has been tested only on SiRFs.  It's in the
     * core because other chipsets reporting only GPS time but with 
     * the capability to read subframe data may want it.
     */
    int i;
    unsigned int pageid, subframe, leap;
    gpsd_report(LOG_IO, 
		"50B (raw): %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n", 
		words[0], words[1], words[2], words[3], words[4], 
		words[5], words[6], words[7], words[8], words[9]);
    /*
     * Mask off the high 2 bits and shift out the 6 parity bits.
     * Once we've filtered, we can ignore the TEL and HOW words.
     * We don't need to check parity here, the SiRF chipset does
     * that and throws a subframe error if the parity is wrong.
     */
    for (i = 0; i < 10; i++)
	words[i] = (words[i]  & 0x3fffffff) >> 6;
    /*
     * "First, throw away everything that doesn't start with 8b or
     * 74. more correctly the first byte should be 10001011. If
     * it's 01110100, then you have a subframe with inverted
     * polarity and each byte needs to be xored against 0xff to
     * remove the inversion."
     */
    words[0] &= 0xff0000;
    if (words[0] != 0x8b0000 && words[0] != 0x740000)
	return;
    if (words[0] == 0x740000)
	for (i = 1; i < 10; i++)
	    words[i] ^= 0xffffff;
    /*
     * The subframe ID is in the Hand Over Word (page 80) 
     */
    subframe = ((words[1] >> 2) & 0x07);
    /* we're not interested in anything but subframe 4 */
    if (subframe != 4)
	return;
    /*
     * Pages 66-76a,80 of ICD-GPS-200 are the subframe structures.
     * Subframe 4 page 18 is on page 74.
     * See page 105 for the mapping between magic SVIDs and pages.
     */
    pageid = (words[2] & 0x3F0000) >> 16;
    gpsd_report(LOG_PROG, "Subframe 4 SVID is %d\n", pageid);
    if (pageid == 56) {	/* magic SVID for page 18 */
	/* once we've filtered, we can ignore the TEL and HOW words */
	gpsd_report(LOG_PROG, "50B: SF=%d %06x %06x %06x %06x %06x %06x %06x %06x\n", 
		    subframe,
		    words[2], words[3], words[4], words[5], 
		    words[6], words[7], words[8], words[9]);
	leap = (words[8] & 0xff0000) >> 16;
	/*
	 * On SiRFs, there appears to be some bizarre bug that
	 * randomly causes this field to come out two's-complemented.
	 * This could very well be a general problem; work around it.
	 * At the current expected rate of issuing leap-seconds this
	 * kluge won't bite until about 2070, by which time the
	 * vendors had better have fixed their damn firmware...
	 *
	 * Carl: ...I am unsure, and suggest you
	 * experiment.  The D30 bit is in bit 30 of the 32-bit
	 * word (next to MSB), and should signal an inverted
	 * value when it is one coming over the air.  But if
	 * the bit is set and the word decodes right without
	 * inversion, then we properly caught it.  Cases where
	 * you see subframe 6 rather than 1 means we should
	 * have done the inversion but we did not.  Some other
	 * things you can watch for: in any subframe, the
	 * second word (HOW word) should have last 2 parity
	 * bits 00 -- there are bits within the rest of the
	 * word that are set as required to ensure that.  The
	 * same goes for word 10.  That means that both words
	 * 1 and 3 (the words that immediately follow words 10
	 * and 2, respectively) should always be uninverted.
	 * In these cases, the D29 and D30 from the previous
	 * words, found in the two MSBs of the word, should
	 * show 00 -- if they don't then you may find an
	 * unintended inversion due to noise on the data link.
	 */
	if (leap > 128)
	    leap ^= 0xff;
	gpsd_report(LOG_INF, "leap-seconds is %d\n", leap);
	session->context->leap_seconds = (int)leap;
	session->context->valid |= LEAP_SECOND_VALID;
    }
}
/*@ +usedef @*/
