/* $Id: rtcm.h 3874 2006-11-13 05:17:19Z esr $ */
/*****************************************************************************

This is a decoder for RTCM-104, an obscure and complicated serial
protocol used for broadcasting pseudorange corrections from
differential-GPS reference stations.  The applicable
standard is

RTCM RECOMMENDED STANDARDS FOR DIFFERENTIAL NAVSTAR GPS SERVICE,
RTCM PAPER 194-93/SC 104-STD

Ordering instructions are accessible from <http://www.rtcm.org/>
under "Publications".  This describes version 2.1 of the RTCM specification.

Also applicable is ITU-R M.823: "Technical characteristics of
differential transmissions for global navigation satellite systems
from maritime radio beacons in the frequency band 283.5 - 315 kHz in
region 1 and 285 - 325 kHz in regions 2 & 3."

The RTCM protocol uses as a transport layer the GPS satellite downlink
protocol described in IS-GPS-200, the Navstar GPS Interface
Specification.  This code relies on the lower-level packet-assembly
code for that protocol in isgps.c.

The lower layer's job is done when it has assembled a message of up to
33 words of clean parity-checked data.  At this point this upper layer
takes over.  struct rtcm_msg_t is overlaid on the buffer and the bitfields
are used to extract pieces of it.  Those pieces are copied and (where
necessary) reassembled into a struct rtcm_t.

This code and the contents of isgps.c are evolved from code by Wolgang
Rupprecht.  Wolfgang's decoder was loosely based on one written by
John Sager in 1999 (in particular the dump function emits a close
descendant of Sager's dump format).  Here are John Sager's original
notes:

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

*****************************************************************************/

/*
 * Structures for interpreting words in an RTCM-104 message (after
 * parity checking and removing inversion).
 *
 * The RTCM standard is less explicit than it should be about signed-integer
 * representations.  Two's compliment is specified for prc and rrc (msg1wX),
 * but not everywhere.
 */

#define	ZCOUNT_SCALE	0.6	/* sec */
#define	PCSMALL		0.02	/* meters */
#define	PCLARGE		0.32	/* meters */
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

#pragma pack(1)

#ifndef WORDS_BIGENDIAN	/* little-endian, like x86 */

struct rtcm_msg_t {
    struct rtcm_msghw1 {			/* header word 1 */
	uint            parity:6;
	uint            refstaid:10;	/* reference station ID */
	uint            msgtype:6;		/* RTCM message type */
	uint            preamble:8;		/* fixed at 01100110 */
	uint            _pad:2;
    } w1;

    struct rtcm_msghw2 {			/* header word 2 */
	uint            parity:6;
	uint            stathlth:3;		/* station health */
	uint            frmlen:5;
	uint            sqnum:3;
	uint            zcnt:13;
	uint            _pad:2;
    } w2;

    union {
	/* msg 1 - differential gps corrections */
	struct rtcm_msg1 {
	    struct b_correction_t {
		struct {			/* msg 1 word 3 */
		    uint            parity:6;
		    int             pc1:16;
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
		    uint            issuedata1:8;
		    int             rangerate1:8;
		    uint            _pad:2;
		} w4;

		struct {			/* msg 1 word 5 */
		    uint            parity:6;
		    int             rangerate2:8;
		    int             pc2:16;
		    uint            _pad:2;
		} w5;

		struct {			/* msg 1 word 6 */
		    uint            parity:6;
		    int             pc3_h:8;
		    uint            satident3:5;	/* satellite ID */
		    uint            udre3:2;
		    uint            scale3:1;
		    uint            issuedata2:8;
		    uint            _pad:2;
		} w6;

		struct {			/* msg 1 word 7 */
		    uint            parity:6;
		    uint            issuedata3:8;
		    int             rangerate3:8;
		    uint            pc3_l:8;		/* NOTE: uint for low byte */
		    uint            _pad:2;
		} w7;
	    } corrections[(RTCM_WORDS_MAX - 2) / 5];
	} type1;

	/* msg 3 - reference station parameters */
	struct rtcm_msg3 {
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
	struct rtcm_msg4 {
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
	struct rtcm_msg5 {
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
	struct rtcm_msg7 {
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
	    } almanac[(RTCM_WORDS_MAX - 2)/3];
	} type7;

	/* msg 16 - text msg */
	struct rtcm_msg16 {
	    struct {
		uint        parity:6;
		uint	    byte3:8;
		uint	    byte2:8;
		uint	    byte1:8;
		uint        _pad:2;
	    } txt[RTCM_WORDS_MAX-2];
	} type16;

	/* unknown message */
	isgps30bits_t	rtcm_msgunk[RTCM_WORDS_MAX-2];
    } msg_type;
};

#endif /* LITTLE_ENDIAN */

#ifdef WORDS_BIGENDIAN
/* This struct was generated from the above using invert-bitfields.pl */
#ifndef S_SPLINT_S	/* splint thinks it's a duplicate definition */

struct rtcm_msg_t {
    struct rtcm_msghw1 {			/* header word 1 */
	uint            _pad:2;
	uint            preamble:8;		/* fixed at 01100110 */
	uint            msgtype:6;		/* RTCM message type */
	uint            refstaid:10;	/* reference station ID */
	uint            parity:6;
    } w1;

    struct rtcm_msghw2 {			/* header word 2 */
	uint            _pad:2;
	uint            zcnt:13;
	uint            sqnum:3;
	uint            frmlen:5;
	uint            stathlth:3;		/* station health */
	uint            parity:6;
    } w2;

    union {
	/* msg 1 - differential gps corrections */
	struct rtcm_msg1 {
	    struct b_correction_t {
		struct {			/* msg 1 word 3 */
		    uint            _pad:2;
		    uint            scale1:1;
		    uint            udre1:2;
		    uint            satident1:5;	/* satellite ID */
		    int             pc1:16;
		    uint            parity:6;
		} w3;

		struct {			/* msg 1 word 4 */
		    uint            _pad:2;
		    int             rangerate1:8;
		    uint            issuedata1:8;
		    uint            scale2:1;
		    uint            udre2:2;
		    uint            satident2:5;	/* satellite ID */
		    uint            parity:6;
		} w4;

		struct {			/* msg 1 word 5 */
		    uint            _pad:2;
		    int             pc2:16;
		    int             rangerate2:8;
		    uint            parity:6;
		} w5;

		struct {			/* msg 1 word 6 */
		    uint            _pad:2;
		    uint            issuedata2:8;
		    uint            scale3:1;
		    uint            udre3:2;
		    uint            satident3:5;	/* satellite ID */
		    int             pc3_h:8;
		    uint            parity:6;
		} w6;

		struct {			/* msg 1 word 7 */
		    uint            _pad:2;
		    uint            pc3_l:8;		/* NOTE: uint for low byte */
		    int             rangerate3:8;
		    uint            issuedata3:8;
		    uint            parity:6;
		} w7;
	    } corrections[(RTCM_WORDS_MAX - 2) / 5];
	} type1;

	/* msg 3 - reference station parameters */
	struct rtcm_msg3 {
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
	struct rtcm_msg4 {
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
	struct rtcm_msg5 {
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
	struct rtcm_msg7 {
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
	    } almanac[(RTCM_WORDS_MAX - 2)/3];
	} type7;

	/* msg 16 - text msg */
	struct rtcm_msg16 {
	    struct {
		uint        _pad:2;
		uint	    byte1:8;
		uint	    byte2:8;
		uint	    byte3:8;
		uint        parity:6;
	    } txt[RTCM_WORDS_MAX-2];
	} type16;

	/* unknown message */
	isgps30bits_t	rtcm_msgunk[RTCM_WORDS_MAX-2];
    } msg_type;
};

#endif /* S_SPLINT_S */
#endif /* BIG ENDIAN */
