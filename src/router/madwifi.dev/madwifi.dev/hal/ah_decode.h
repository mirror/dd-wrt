/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ah_decode.h#2 $
 */
#ifndef _ATH_AH_DECODE_H_
#define _ATH_AH_DECODE_H_
/*
 * Register tracing support.
 *
 * Setting hw.ath.hal.alq=1 enables tracing of all register reads and
 * writes to the file /tmp/ath_hal.log.  The file format is a simple
 * fixed-size array of records.  When done logging set hw.ath.hal.alq=0
 * and then decode the file with the arcode program (that is part of the
 * HAL).  If you start+stop tracing the data will be appended to an
 * existing file.
 */
struct athregrec {
	u_int32_t	op	: 8,
			reg	: 24;
	u_int32_t	val;
};

enum {
	OP_READ		= 0,		/* register read */
	OP_WRITE	= 1,		/* register write */
	OP_DEVICE	= 2,		/* device identification */
	OP_MARK		= 3,		/* application marker */
};

enum {
	AH_MARK_RESET,			/* ar*Reset entry, bChannelChange */
	AH_MARK_RESET_LINE,		/* ar*_reset.c, line %d */
	AH_MARK_RESET_DONE,		/* ar*Reset exit, error code */
	AH_MARK_CHIPRESET,		/* ar*ChipReset, channel num */
	AH_MARK_PERCAL,			/* ar*PerCalibration, channel num */
	AH_MARK_SETCHANNEL,		/* ar*SetChannel, channel num */
	AH_MARK_ANI_RESET,		/* ar*AniReset, opmode */
	AH_MARK_ANI_POLL,		/* ar*AniReset, listen time */
	AH_MARK_ANI_CONTROL,		/* ar*AniReset, cmd */
};
#endif /* _ATH_AH_DECODE_H_ */
