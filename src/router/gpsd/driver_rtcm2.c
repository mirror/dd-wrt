/*****************************************************************************

This is a decoder for RTCM-104 2.x, an obscure and complicated serial
protocol used for broadcasting pseudorange corrections from
differential-GPS reference stations.  The applicable
standard is

RTCM RECOMMENDED STANDARDS FOR DIFFERENTIAL GNSS (GLOBAL NAVIGATION
SATELLITE) SERVICE, VERSION 2.3 (RTCM PAPER 136-2001/SC104-STD)

Ordering instructions are accessible from <http://www.rtcm.org/>
under "Publications".  This describes version 2.3 of the RTCM specification.
RTCM-104 was later completely redesigned as level 3.0.

Also applicable is ITU-R M.823: "Technical characteristics of
differential transmissions for global navigation satellite systems
from maritime radio beacons in the frequency band 283.5 - 315 kHz in
region 1 and 285 - 325 kHz in regions 2 & 3."

The RTCM 2.x protocol uses as a transport layer the GPS satellite downlink
protocol described in IS-GPS-200, the Navstar GPS Interface
Specification.  This code relies on the lower-level packet-assembly
code for that protocol in isgps.c.

The lower layer's job is done when it has assembled a message of up to
33 30-bit words of clean parity-checked data.  At this point this upper layer
takes over.  struct rtcm2_msg_t is overlaid on the buffer and the bitfields
are used to extract pieces of it.  Those pieces are copied and (where
necessary) reassembled into a struct rtcm2_t.

This code and the contents of isgps.c are evolved from code by
Wolfgang Rupprecht.  Wolfgang's decoder was loosely based on one
written by John Sager in 1999.  Here are John Sager's original notes:

The RTCM decoder prints a legible representation of the input data.
The RTCM SC-104 specification is copyrighted, so I cannot
quote it - in fact, I have never read it! Most of the information
used to develop the decoder came from publication ITU-R M.823.
This is a specification of the data transmitted from LF DGPS
beacons in the 300kHz band. M.823 contains most of those parts of
RTCM SC-104 directly relevant to the air interface (there
are one or two annoying and vital omissions!). Information
about the serial interface format was gleaned from studying
the output of a beacon receiver test program made available on
Starlink's website.

This code has been checked against ASCII dumps made by a proprietary
decoder running under Windows and is known to be consistent with it
with respect to message types 1, 3, 9, 14, 16, and 31.  Decoding of
message types 4, 5, 6, 7, and 13 has not been checked. Message types
8, 10-12, 15-27, 28-30 (undefined), 31-37, 38-58 (undefined), and
60-63 are not yet supported.

This file is Copyright (c) 2010 by the GPSD project
BSD terms apply: see the file COPYING in the distribution root for details.

*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "gpsd.h"

/*
 * Structures for interpreting words in an RTCM-104 2.x message (after
 * parity checking and removing inversion).  Note, these structures
 * are overlayed on the raw data in order to decode them into
 * bitfields; this will fail horribly if your C compiler introduces
 * padding between or before bit fields, or between 8-bit-aligned
 * bitfields and character arrays despite #pragma pack(1).  The right
 * things happen under gcc 4.x on amd64, i386, ia64, all arm and mips
 * variants, m68k, and powerpc)
 *
 * (In practice, the only class of machines on which this is likely
 * to fail are word-aligned architectures without barrel shifters.
 * Very few of these are left in 2012. By test, we know of s390, s390x,
 * and sparc.)
 *
 * The RTCM 2.1 standard is less explicit than it should be about signed-integer
 * representations.  Two's complement is specified for some but not all.
 */

#define	ZCOUNT_SCALE	0.6	/* sec */
#define	PRCSMALL	0.02	/* meters */
#define	PRCLARGE	0.32	/* meters */
#define	RRSMALL		0.002	/* meters/sec */
#define	RRLARGE		0.032	/* meters/sec */

#define MAXPCSMALL     (0x7FFF * PCSMALL)  /* 16-bits signed */
#define MAXRRSMALL     (0x7F   * RRSMALL)  /*  8-bits signed */

#define XYZ_SCALE	0.01	/* meters */
#define DXYZ_SCALE	0.1	/* meters */
#define	LA_SCALE	(90.0/32767.0)	/* degrees */
#define	LO_SCALE	(180.0/32767.0)	/* degrees */
#define	FREQ_SCALE	0.1	/* kHz */
#define	FREQ_OFFSET	190.0	/* kHz */
#define CNR_OFFSET	24	/* dB */
#define TU_SCALE	5	/* minutes */

#define LATLON_SCALE	0.01	/* degrees */
#define RANGE_SCALE	4	/* kilometers */

#pragma pack(1)

/*
 * Reminder: Emacs reverse-region is useful...
 */

#ifndef WORDS_BIGENDIAN	/* little-endian, like x86 */

struct rtcm2_msg_t {
    struct rtcm2_msghw1 {			/* header word 1 */
	uint            parity:6;
	uint            refstaid:10;	/* reference station ID */
	uint            msgtype:6;		/* RTCM message type */
	uint            preamble:8;		/* fixed at 01100110 */
	uint            _pad:2;
    } w1;

    struct rtcm2_msghw2 {			/* header word 2 */
	uint            parity:6;
	uint            stathlth:3;		/* station health */
	uint            frmlen:5;
	uint            sqnum:3;
	uint            zcnt:13;
	uint            _pad:2;
    } w2;

    union {
	/* msg 1 - differential gps corrections */
	struct rtcm2_msg1 {
	    struct gps_correction_t {
		struct {			/* msg 1 word 3 */
		    uint            parity:6;
		    int             prc1:16;
		    uint            satident1:5;	/* satellite ID */
		    uint            udre1:2;
		    uint            scale1:1;
		    uint            _pad:2;
		} w3;

		struct {			/* msg 1 word 4 */
		    uint            parity:6;
		    uint            satident2:5;	/* satellite ID */
		    uint            udre2:2;
		    uint            scale2:1;
		    uint            iod1:8;
		    int             rrc1:8;
		    uint            _pad:2;
		} w4;

		struct {			/* msg 1 word 5 */
		    uint            parity:6;
		    int             rrc2:8;
		    int             prc2:16;
		    uint            _pad:2;
		} w5;

		struct {			/* msg 1 word 6 */
		    uint            parity:6;
		    int             prc3_h:8;
		    uint            satident3:5;	/* satellite ID */
		    uint            udre3:2;
		    uint            scale3:1;
		    uint            iod2:8;
		    uint            _pad:2;
		} w6;

		struct {			/* msg 1 word 7 */
		    uint            parity:6;
		    uint            iod3:8;
		    int             rrc3:8;
		    uint            prc3_l:8;		/* NOTE: uint for low byte */
		    uint            _pad:2;
		} w7;
	    } corrections[(RTCM2_WORDS_MAX - 2) / 5];
	} type1;

	/* msg 3 - reference station parameters */
	struct rtcm2_msg3 {
	    struct {
		uint        parity:6;
		uint	    x_h:24;
		uint        _pad:2;
	    } w3;
	    struct {
		uint        parity:6;
		uint	    y_h:16;
		uint	    x_l:8;
		uint        _pad:2;
	    } w4;
	    struct {
		uint        parity:6;
		uint	    z_h:8;
		uint	    y_l:16;
		uint        _pad:2;
	    } w5;

	    struct {
		uint        parity:6;
		uint	    z_l:24;
		uint        _pad:2;
	    } w6;
	} type3;

	/* msg 4 - reference station datum */
	struct rtcm2_msg4 {
	    struct {
		uint        parity:6;
		uint	    datum_alpha_char2:8;
		uint	    datum_alpha_char1:8;
		uint	    spare:4;
		uint	    dat:1;
		uint	    dgnss:3;
		uint        _pad:2;
	    } w3;
	    struct {
		uint        parity:6;
		uint	    datum_sub_div_char2:8;
		uint	    datum_sub_div_char1:8;
		uint	    datum_sub_div_char3:8;
		uint        _pad:2;
	    } w4;
	    struct {
		uint        parity:6;
		uint	    dy_h:8;
		uint	    dx:16;
		uint        _pad:2;
	    } w5;
	    struct {
		uint        parity:6;
		uint	    dz:24;
		uint	    dy_l:8;
		uint        _pad:2;
	    } w6;
	} type4;

	/* msg 5 - constellation health */
	struct rtcm2_msg5 {
	    struct b_health_t {
		uint        parity:6;
		uint	    unassigned:2;
		uint	    time_unhealthy:4;
		uint	    loss_warn:1;
		uint	    new_nav_data:1;
		uint	    health_enable:1;
		uint	    cn0:5;
		uint	    data_health:3;
		uint	    issue_of_data_link:1;
		uint	    sat_id:5;
		uint	    reserved:1;
		uint        _pad:2;
	    } health[MAXHEALTH];
	} type5;

	/* msg 6 - null message */

	/* msg 7 - beacon almanac */
	struct rtcm2_msg7 {
	    struct b_station_t {
		struct {
		    uint            parity:6;
		    int	    	    lon_h:8;
		    int	            lat:16;
		    uint            _pad:2;
		} w3;
		struct {
		    uint            parity:6;
		    uint	    freq_h:6;
		    uint	    range:10;
		    uint	    lon_l:8;
		    uint            _pad:2;
		} w4;
		struct {
		    uint            parity:6;
		    uint	    encoding:1;
		    uint	    sync_type:1;
		    uint	    mod_mode:1;
		    uint	    bit_rate:3;
		    /*
		     * ITU-R M.823-2 page 9 and RTCM-SC104 v2.1 pages
		     * 4-21 and 4-22 are in conflict over the next two
		     * field sizes.  ITU says 9+3, RTCM says 10+2.
		     * The latter correctly decodes the USCG station
		     * id's so I'll use that one here. -wsr
		     */
		    uint	    station_id:10;
		    uint	    health:2;
		    uint	    freq_l:6;
		    uint            _pad:2;
		} w5;
	    } almanac[(RTCM2_WORDS_MAX - 2)/3];
	} type7;

	/* msg 13 - Ground Transmitter Parameters (RTCM2.3 only) */
	struct rtcm2_msg13 {
	    struct {
		uint        parity:6;
		int         lat:16;
		uint        reserved:6;
		uint        rangeflag:1;
		uint        status:1;
		uint        _pad:2;
	    } w1;
	    struct {
		uint        parity:6;
		uint        range:8;
		int         lon:16;
		uint        _pad:2;
	    } w2;
	} type13;

	/* msg 14 - GPS Time of Week (RTCM2.3 only) */
	struct rtcm2_msg14 {
	    struct {
		uint        parity:6;
		uint        leapsecs:6;
		uint        hour:8;
		uint        week:10;
		uint        _pad:2;
	    } w1;
	} type14;

	/* msg 16 - text msg */
	struct rtcm2_msg16 {
	    struct {
		uint        parity:6;
		uint	    byte3:8;
		uint	    byte2:8;
		uint	    byte1:8;
		uint        _pad:2;
	    } txt[RTCM2_WORDS_MAX-2];
	} type16;

	/* msg 31 - differential GLONASS corrections */
	struct rtcm2_msg31 {
	    struct glonass_correction_t {
		struct {			/* msg 1 word 3 */
		    uint            parity:6;
		    int             prc1:16;
		    uint            satident1:5;	/* satellite ID */
		    uint            udre1:2;
		    uint            scale1:1;
		    uint            _pad:2;
		} w3;

		struct {			/* msg 1 word 4 */
		    uint            parity:6;
		    uint            satident2:5;	/* satellite ID */
		    uint            udre2:2;
		    uint            scale2:1;
		    uint            tod1:7;
		    uint            change1:1;
		    int             rrc1:8;
		    uint            _pad:2;
		} w4;

		struct {			/* msg 1 word 5 */
		    uint            parity:6;
		    int             rrc2:8;
		    int             prc2:16;
		    uint            _pad:2;
		} w5;

		struct {			/* msg 1 word 6 */
		    uint            parity:6;
		    int             prc3_h:8;
		    uint            satident3:5;	/* satellite ID */
		    uint            udre3:2;
		    uint            scale3:1;
		    uint            tod2:7;
		    uint            change2:1;
		    uint            _pad:2;
		} w6;

		struct {			/* msg 1 word 7 */
		    uint            parity:6;
		    uint            tod3:7;
		    uint            change3:1;
		    int             rrc3:8;
		    uint            prc3_l:8;	/* NOTE: uint for low byte */
		    uint            _pad:2;
		} w7;
	    } corrections[(RTCM2_WORDS_MAX - 2) / 5];
	} type31;

	/* unknown message */
	isgps30bits_t	rtcm2_msgunk[RTCM2_WORDS_MAX-2];
    } msg_type;
};

#endif /* LITTLE_ENDIAN */

#ifdef WORDS_BIGENDIAN
#ifndef S_SPLINT_S	/* splint thinks it's a duplicate definition */

struct rtcm2_msg_t {
    struct rtcm2_msghw1 {			/* header word 1 */
	uint            _pad:2;
	uint            preamble:8;		/* fixed at 01100110 */
	uint            msgtype:6;		/* RTCM message type */
	uint            refstaid:10;	/* reference station ID */
	uint            parity:6;
    } w1;

    struct rtcm2_msghw2 {			/* header word 2 */
	uint            _pad:2;
	uint            zcnt:13;
	uint            sqnum:3;
	uint            frmlen:5;
	uint            stathlth:3;		/* station health */
	uint            parity:6;
    } w2;

    union {
	/* msg 1 - differential GPS corrections */
	struct rtcm2_msg1 {
	    struct gps_correction_t {
		struct {			/* msg 1 word 3 */
		    uint            _pad:2;
		    uint            scale1:1;
		    uint            udre1:2;
		    uint            satident1:5;	/* satellite ID */
		    int             prc1:16;
		    uint            parity:6;
		} w3;

		struct {			/* msg 1 word 4 */
		    uint            _pad:2;
		    int             rrc1:8;
		    uint            iod1:8;
		    uint            scale2:1;
		    uint            udre2:2;
		    uint            satident2:5;	/* satellite ID */
		    uint            parity:6;
		} w4;

		struct {			/* msg 1 word 5 */
		    uint            _pad:2;
		    int             prc2:16;
		    int             rrc2:8;
		    uint            parity:6;
		} w5;

		struct {			/* msg 1 word 6 */
		    uint            _pad:2;
		    uint            iod2:8;
		    uint            scale3:1;
		    uint            udre3:2;
		    uint            satident3:5;	/* satellite ID */
		    int             prc3_h:8;
		    uint            parity:6;
		} w6;

		struct {			/* msg 1 word 7 */
		    uint            _pad:2;
		    uint            prc3_l:8;		/* NOTE: uint for low byte */
		    int             rrc3:8;
		    uint            iod3:8;
		    uint            parity:6;
		} w7;
	    } corrections[(RTCM2_WORDS_MAX - 2) / 5];
	} type1;

	/* msg 3 - reference station parameters */
	struct rtcm2_msg3 {
	    struct {
		uint        _pad:2;
		uint	    x_h:24;
		uint        parity:6;
	    } w3;
	    struct {
		uint        _pad:2;
		uint	    x_l:8;
		uint	    y_h:16;
		uint        parity:6;
	    } w4;
	    struct {
		uint        _pad:2;
		uint	    y_l:16;
		uint	    z_h:8;
		uint        parity:6;
	    } w5;

	    struct {
		uint        _pad:2;
		uint	    z_l:24;
		uint        parity:6;
	    } w6;
	} type3;

	/* msg 4 - reference station datum */
	struct rtcm2_msg4 {
	    struct {
		uint        _pad:2;
		uint	    dgnss:3;
		uint	    dat:1;
		uint	    spare:4;
		uint	    datum_alpha_char1:8;
		uint	    datum_alpha_char2:8;
		uint        parity:6;
	    } w3;
	    struct {
		uint        _pad:2;
		uint	    datum_sub_div_char3:8;
		uint	    datum_sub_div_char1:8;
		uint	    datum_sub_div_char2:8;
		uint        parity:6;
	    } w4;
	    struct {
		uint        _pad:2;
		uint	    dx:16;
		uint	    dy_h:8;
		uint        parity:6;
	    } w5;
	    struct {
		uint        _pad:2;
		uint	    dy_l:8;
		uint	    dz:24;
		uint        parity:6;
	    } w6;
	} type4;

	/* msg 5 - constellation health */
	struct rtcm2_msg5 {
	    struct b_health_t {
		uint        _pad:2;
		uint	    reserved:1;
		uint	    sat_id:5;
		uint	    issue_of_data_link:1;
		uint	    data_health:3;
		uint	    cn0:5;
		uint	    health_enable:1;
		uint	    new_nav_data:1;
		uint	    loss_warn:1;
		uint	    time_unhealthy:4;
		uint	    unassigned:2;
		uint        parity:6;
	    } health[MAXHEALTH];
	} type5;

	/* msg 6 - null message */

	/* msg 7 - beacon almanac */
	struct rtcm2_msg7 {
	    struct b_station_t {
		struct {
		    uint            _pad:2;
		    int	            lat:16;
		    int	    	    lon_h:8;
		    uint            parity:6;
		} w3;
		struct {
		    uint            _pad:2;
		    uint	    lon_l:8;
		    uint	    range:10;
		    uint	    freq_h:6;
		    uint            parity:6;
		} w4;
		struct {
		    uint            _pad:2;
		    uint	    freq_l:6;
		    uint	    health:2;
		    uint	    station_id:10;
			     /* see comments in LE struct above. */
		    uint	    bit_rate:3;
		    uint	    mod_mode:1;
		    uint	    sync_type:1;
		    uint	    encoding:1;
		    uint            parity:6;
		} w5;
	    } almanac[(RTCM2_WORDS_MAX - 2)/3];
	} type7;

	/* msg 13 - Ground Transmitter Parameters (RTCM2.3 only) */
	struct rtcm2_msg13 {
	    struct {
		uint        _pad:2;
		uint        status:1;
		uint        rangeflag:1;
		uint        reserved:6;
		int         lat:16;
		uint        parity:6;
	    } w1;
	    struct {
		uint        _pad:2;
		int         lon:16;
		uint        range:8;
		uint        parity:6;
	    } w2;
	} type13;

	/* msg 14 - GPS Time of Week (RTCM2.3 only) */
	struct rtcm2_msg14 {
	    struct {
		uint        _pad:2;
		uint        week:10;
		uint        hour:8;
		uint        leapsecs:6;
		uint        parity:6;
	    } w1;
	} type14;

	/* msg 16 - text msg */
	struct rtcm2_msg16 {
	    struct {
		uint        _pad:2;
		uint	    byte1:8;
		uint	    byte2:8;
		uint	    byte3:8;
		uint        parity:6;
	    } txt[RTCM2_WORDS_MAX-2];
	} type16;

	/* msg 31 - differential GLONASS corrections */
	struct rtcm2_msg31 {
	    struct glonass_correction_t {
		struct {			/* msg 1 word 3 */
		    uint            _pad:2;
		    uint            scale1:1;
		    uint            udre1:2;
		    uint            satident1:5;	/* satellite ID */
		    int             prc1:16;
		    uint            parity:6;
		} w3;

		struct {			/* msg 1 word 4 */
		    uint            _pad:2;
		    int             rrc1:8;
		    uint            change1:1;
		    uint            tod1:7;
		    uint            scale2:1;
		    uint            udre2:2;
		    uint            satident2:5;	/* satellite ID */
		    uint            parity:6;
		} w4;

		struct {			/* msg 1 word 5 */
		    uint            _pad:2;
		    int             prc2:16;
		    int             rrc2:8;
		    uint            parity:6;
		} w5;

		struct {			/* msg 1 word 6 */
		    uint            _pad:2;
		    uint            change2:1;
		    uint            tod2:7;
		    uint            scale3:1;
		    uint            udre3:2;
		    uint            satident3:5;	/* satellite ID */
		    int             prc3_h:8;
		    uint            parity:6;
		} w6;

		struct {			/* msg 1 word 7 */
		    uint            _pad:2;
		    uint            prc3_l:8;	/* NOTE: uint for low byte */
		    int             rrc3:8;
		    uint            change3:1;
		    uint            tod3:7;
		    uint            parity:6;
		} w7;
	    } corrections[(RTCM2_WORDS_MAX - 2) / 5];
	} type31;

	/* unknown message */
	isgps30bits_t	rtcm2_msgunk[RTCM2_WORDS_MAX-2];
    } msg_type;
};

#endif /* S_SPLINT_S */
#endif /* BIG ENDIAN */

#ifdef RTCM104V2_ENABLE

#define PREAMBLE_PATTERN 0x66

static unsigned int tx_speed[] = { 25, 50, 100, 110, 150, 200, 250, 300 };

#define DIMENSION(a) (unsigned)(sizeof(a)/sizeof(a[0]))

void rtcm2_unpack( /*@out@*/ struct rtcm2_t *tp, char *buf)
/* break out the raw bits into the content fields */
{
    int len;
    unsigned int n, w;
    struct rtcm2_msg_t *msg = (struct rtcm2_msg_t *)buf;

    tp->type = msg->w1.msgtype;
    tp->length = msg->w2.frmlen;
    tp->zcount = msg->w2.zcnt * ZCOUNT_SCALE;
    tp->refstaid = msg->w1.refstaid;
    tp->seqnum = msg->w2.sqnum;
    tp->stathlth = msg->w2.stathlth;

    len = (int)tp->length;
    n = 0;
    switch (tp->type) {
    case 1:
    case 9:
    {
	struct gps_correction_t *m = &msg->msg_type.type1.corrections[0];

	while (len >= 0) {
	    if (len >= 2) {
		tp->gps_ranges.sat[n].ident = m->w3.satident1;
		tp->gps_ranges.sat[n].udre = m->w3.udre1;
		tp->gps_ranges.sat[n].iod = m->w4.iod1;
		tp->gps_ranges.sat[n].prc = m->w3.prc1 *
		    (m->w3.scale1 ? PRCLARGE : PRCSMALL);
		tp->gps_ranges.sat[n].rrc = m->w4.rrc1 *
		    (m->w3.scale1 ? RRLARGE : RRSMALL);
		n++;
	    }
	    if (len >= 4) {
		tp->gps_ranges.sat[n].ident = m->w4.satident2;
		tp->gps_ranges.sat[n].udre = m->w4.udre2;
		tp->gps_ranges.sat[n].iod = m->w6.iod2;
		tp->gps_ranges.sat[n].prc = m->w5.prc2 *
		    (m->w4.scale2 ? PRCLARGE : PRCSMALL);
		tp->gps_ranges.sat[n].rrc = m->w5.rrc2 *
		    (m->w4.scale2 ? RRLARGE : RRSMALL);
		n++;
	    }
	    if (len >= 5) {
		tp->gps_ranges.sat[n].ident = m->w6.satident3;
		tp->gps_ranges.sat[n].udre = m->w6.udre3;
		tp->gps_ranges.sat[n].iod = m->w7.iod3;
		/*@ -shiftimplementation @*/
		tp->gps_ranges.sat[n].prc =
		    ((m->w6.prc3_h << 8) | (m->w7.prc3_l)) *
		    (m->w6.scale3 ? PRCLARGE : PRCSMALL);
		tp->gps_ranges.sat[n].rrc =
		    m->w7.rrc3 * (m->w6.scale3 ? RRLARGE : RRSMALL);
		/*@ +shiftimplementation @*/
		n++;
	    }
	    len -= 5;
	    m++;
	}
	tp->gps_ranges.nentries = n;
    }
	break;
    case 3:
    {
	struct rtcm2_msg3 *m = &msg->msg_type.type3;

	if ((tp->ecef.valid = len >= 4)) {
	    tp->ecef.x = ((m->w3.x_h << 8) | (m->w4.x_l)) * XYZ_SCALE;
	    tp->ecef.y = ((m->w4.y_h << 16) | (m->w5.y_l)) * XYZ_SCALE;
	    tp->ecef.z = ((m->w5.z_h << 24) | (m->w6.z_l)) * XYZ_SCALE;
	}
    }
	break;
    case 4:
	if ((tp->reference.valid = len >= 2)) {
	    struct rtcm2_msg4 *m = &msg->msg_type.type4;

	    tp->reference.system =
		(m->w3.dgnss == 0) ? NAVSYSTEM_GPS :
		((m->w3.dgnss == 1) ? NAVSYSTEM_GLONASS : NAVSYSTEM_UNKNOWN);
	    tp->reference.sense =
		(m->w3.dat != 0) ? SENSE_GLOBAL : SENSE_LOCAL;
	    if (m->w3.datum_alpha_char1) {
		tp->reference.datum[n++] = (char)(m->w3.datum_alpha_char1);
	    }
	    if (m->w3.datum_alpha_char2) {
		tp->reference.datum[n++] = (char)(m->w3.datum_alpha_char2);
	    }
	    if (m->w4.datum_sub_div_char1) {
		tp->reference.datum[n++] = (char)(m->w4.datum_sub_div_char1);
	    }
	    if (m->w4.datum_sub_div_char2) {
		tp->reference.datum[n++] = (char)(m->w4.datum_sub_div_char2);
	    }
	    if (m->w4.datum_sub_div_char3) {
		tp->reference.datum[n++] = (char)(m->w4.datum_sub_div_char3);
	    }
	    /* we used to say n++ here, but scan-build complains */
	    tp->reference.datum[n] = '\0';
	    if (len >= 4) {
		tp->reference.dx = m->w5.dx * DXYZ_SCALE;
		tp->reference.dy =
		    ((m->w5.dy_h << 8) | m->w6.dy_l) * DXYZ_SCALE;
		tp->reference.dz = m->w6.dz * DXYZ_SCALE;
	    } else
		tp->reference.sense = SENSE_INVALID;
	}
	break;
    case 5:
	for (n = 0; n < (unsigned)len; n++) {
	    struct consat_t *csp = &tp->conhealth.sat[n];
	    struct b_health_t *m = &msg->msg_type.type5.health[n];

	    csp->ident = m->sat_id;
	    csp->iodl = m->issue_of_data_link != 0;
	    csp->health = m->data_health;
	    /*@+ignoresigns@*/
	    csp->snr = (int)(m->cn0 ? (m->cn0 + CNR_OFFSET) : SNR_BAD);
	    /*@-ignoresigns@*/
	    csp->health_en = m->health_enable != 0;
	    csp->new_data = m->new_nav_data != 0;
	    csp->los_warning = m->loss_warn != 0;
	    csp->tou = m->time_unhealthy * TU_SCALE;
	}
	tp->conhealth.nentries = n;
	break;
    case 7:
	for (w = 0; w < (unsigned)len; w++) {
	    struct station_t *np = &tp->almanac.station[n];
	    struct b_station_t *mp = &msg->msg_type.type7.almanac[w];

	    np->latitude = mp->w3.lat * LA_SCALE;
	    /*@-shiftimplementation@*/
	    np->longitude = ((mp->w3.lon_h << 8) | mp->w4.lon_l) * LO_SCALE;
	    /*@+shiftimplementation@*/
	    np->range = mp->w4.range;
	    np->frequency =
		(((mp->w4.freq_h << 6) | mp->w5.freq_l) * FREQ_SCALE) +
		FREQ_OFFSET;
	    np->health = mp->w5.health;
	    np->station_id = mp->w5.station_id,
		np->bitrate = tx_speed[mp->w5.bit_rate];
	    n++;
	}
	tp->almanac.nentries = (unsigned)(len / 3);
	break;
    case 13:
	tp->xmitter.status = (bool)msg->msg_type.type13.w1.status;
	tp->xmitter.rangeflag = (bool)msg->msg_type.type13.w1.rangeflag;
	tp->xmitter.lat = msg->msg_type.type13.w1.lat * LATLON_SCALE;
	tp->xmitter.lon = msg->msg_type.type13.w2.lon * LATLON_SCALE;
	tp->xmitter.range = msg->msg_type.type13.w2.range * RANGE_SCALE;
	if (tp->xmitter.range == 0)
	    tp->xmitter.range = 1024;
	break;
    case 14:
	tp->gpstime.week = msg->msg_type.type14.w1.week;
	tp->gpstime.hour = msg->msg_type.type14.w1.hour;
	tp->gpstime.leapsecs = msg->msg_type.type14.w1.leapsecs;
	break;
    case 16:
	/*@ -boolops @*/
	for (w = 0; w < (unsigned)len; w++) {
	    if (!msg->msg_type.type16.txt[w].byte1) {
		break;
	    }
	    tp->message[n++] = (char)(msg->msg_type.type16.txt[w].byte1);
	    if (!msg->msg_type.type16.txt[w].byte2) {
		break;
	    }
	    tp->message[n++] = (char)(msg->msg_type.type16.txt[w].byte2);
	    if (!msg->msg_type.type16.txt[w].byte3) {
		break;
	    }
	    tp->message[n++] = (char)(msg->msg_type.type16.txt[w].byte3);
	}
	/*@ +boolops @*/
	tp->message[n] = '\0';
	break;

    case 31:
    {
	struct glonass_correction_t *m = &msg->msg_type.type31.corrections[0];

	while (len >= 0) {
	    if (len >= 2) {
		tp->glonass_ranges.sat[n].ident = m->w3.satident1;
		tp->glonass_ranges.sat[n].udre = m->w3.udre1;
		tp->glonass_ranges.sat[n].change = (bool)m->w4.change1;
		tp->glonass_ranges.sat[n].tod = m->w4.tod1;
		tp->glonass_ranges.sat[n].prc = m->w3.prc1 *
		    (m->w3.scale1 ? PRCLARGE : PRCSMALL);
		tp->glonass_ranges.sat[n].rrc = m->w4.rrc1 *
		    (m->w3.scale1 ? RRLARGE : RRSMALL);
		n++;
	    }
	    if (len >= 4) {
		tp->glonass_ranges.sat[n].ident = m->w4.satident2;
		tp->glonass_ranges.sat[n].udre = m->w4.udre2;
		tp->glonass_ranges.sat[n].change = (bool)m->w6.change2;
		tp->glonass_ranges.sat[n].tod = m->w6.tod2;
		tp->glonass_ranges.sat[n].prc = m->w5.prc2 *
		    (m->w4.scale2 ? PRCLARGE : PRCSMALL);
		tp->glonass_ranges.sat[n].rrc = m->w5.rrc2 *
		    (m->w4.scale2 ? RRLARGE : RRSMALL);
		n++;
	    }
	    if (len >= 5) {
		tp->glonass_ranges.sat[n].ident = m->w6.satident3;
		tp->glonass_ranges.sat[n].udre = m->w6.udre3;
		tp->glonass_ranges.sat[n].change = (bool)m->w7.change3;
		tp->glonass_ranges.sat[n].tod = m->w7.tod3;
		/*@ -shiftimplementation @*/
		tp->glonass_ranges.sat[n].prc =
		    ((m->w6.prc3_h << 8) | (m->w7.prc3_l)) *
		    (m->w6.scale3 ? PRCLARGE : PRCSMALL);
		tp->glonass_ranges.sat[n].rrc =
		    m->w7.rrc3 * (m->w6.scale3 ? RRLARGE : RRSMALL);
		/*@ +shiftimplementation @*/
		n++;
	    }
	    len -= 5;
	    m++;
	}
	tp->glonass_ranges.nentries = n;
    }
	break;

    default:
	memcpy(tp->words, msg->msg_type.rtcm2_msgunk,
	       (RTCM2_WORDS_MAX - 2) * sizeof(isgps30bits_t));
	break;
    }
}

static bool preamble_match(isgps30bits_t * w)
{
    return (((struct rtcm2_msghw1 *)w)->preamble == PREAMBLE_PATTERN);
}

static bool length_check(struct gps_packet_t *lexer)
{
    return lexer->isgps.bufindex >= 2
	&& lexer->isgps.bufindex >=
	((struct rtcm2_msg_t *)lexer->isgps.buf)->w2.frmlen + 2u;
}

enum isgpsstat_t rtcm2_decode(struct gps_packet_t *lexer, unsigned int c)
{
    return isgps_decode(lexer,
			preamble_match, length_check, RTCM2_WORDS_MAX, c);
}

#endif /* RTCM104V2_ENABLE */
