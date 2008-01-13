/*
 * Misc utility routines for WL and Apps
 * This header file housing the define and function prototype use by
 * both the wl driver, tools & Apps.
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef	_bcmwifi_h_
#define	_bcmwifi_h_


/* A chanspec holds the channel number, band, bandwidth and control sideband */
typedef uint16 chanspec_t;

/* channel defines */
#define CH_UPPER_SB			0x01
#define CH_LOWER_SB			0x02
#define CH_EWA_VALID			0x04
#define CH_20MHZ_APART			4
#define CH_10MHZ_APART			2
#define CH_MAX_2G_CHANNEL		14	/* Max channel in 2G band */
#define WLC_MAX_2G_CHANNEL		CH_MAX_2G_CHANNEL /* legacy define */
#define	MAXCHANNEL		224	/* max # supported channels. The max channel no is 216,
					 * this is that + 1 rounded up to a multiple of NBBY (8).
					 * DO NOT MAKE it > 255: channels are uint8's all over
					 */

#define WL_CHANSPEC_CHAN_MASK		0x00ff
#define WL_CHANSPEC_CHAN_SHIFT		0

#define WL_CHANSPEC_CTL_SB_MASK		0x0300
#define WL_CHANSPEC_CTL_SB_SHIFT	     8
#define WL_CHANSPEC_CTL_SB_LOWER	0x0100
#define WL_CHANSPEC_CTL_SB_UPPER	0x0200
#define WL_CHANSPEC_CTL_SB_NONE		0x0300

#define WL_CHANSPEC_BW_MASK		0x0C00
#define WL_CHANSPEC_BW_SHIFT		    10
#define WL_CHANSPEC_BW_10		0x0400
#define WL_CHANSPEC_BW_20		0x0800
#define WL_CHANSPEC_BW_40		0x0C00

#define WL_CHANSPEC_BAND_MASK		0xf000
#define WL_CHANSPEC_BAND_SHIFT		12
#define WL_CHANSPEC_BAND_5G		0x1000
#define WL_CHANSPEC_BAND_2G		0x2000
#define INVCHANSPEC			255

/* channel defines */
#define LOWER_20_SB(channel)	((channel > CH_10MHZ_APART) ? (channel - CH_10MHZ_APART) : 0)
#define UPPER_20_SB(channel)	((channel < (MAXCHANNEL - CH_10MHZ_APART)) ? \
				(channel + CH_10MHZ_APART) : 0)
#define CHSPEC_WLCBANDUNIT(chspec)	(CHSPEC_IS5G(chspec) ? BAND_5G_INDEX : BAND_2G_INDEX)
#define CH20MHZ_CHSPEC(channel)	(chanspec_t)((chanspec_t)(channel) | WL_CHANSPEC_BW_20 | \
				WL_CHANSPEC_CTL_SB_NONE | (((channel) <= CH_MAX_2G_CHANNEL) ? \
				WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G))
#define NEXT_20MHZ_CHAN(channel)	((channel < (MAXCHANNEL - CH_20MHZ_APART)) ? \
					(channel + CH_20MHZ_APART) : 0)
#define CH40MHZ_CHSPEC(channel, ctlsb)	(chanspec_t) \
					((channel) | (ctlsb) | WL_CHANSPEC_BW_40 | \
					((channel) <= CH_MAX_2G_CHANNEL ? WL_CHANSPEC_BAND_2G : \
					WL_CHANSPEC_BAND_5G))
#define CHSPEC_CHANNEL(chspec)	((uint8)(chspec & WL_CHANSPEC_CHAN_MASK))
#define CHSPEC_CTL_SB(chspec)	(chspec & WL_CHANSPEC_CTL_SB_MASK)
#define CHSPEC_BW(chspec)	(chspec & WL_CHANSPEC_BW_MASK)
#define CHSPEC_BAND(chspec)	(chspec & WL_CHANSPEC_BAND_MASK)

#define CHSPEC_IS10(chspec)	((chspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_10)
#define CHSPEC_IS20(chspec)	((chspec & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_20)

#ifndef CHSPEC_IS40
#define CHSPEC_IS40(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40)
#endif

#define CHSPEC_IS5G(chspec)	((chspec & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_5G)
#define CHSPEC_IS2G(chspec)	((chspec & WL_CHANSPEC_BAND_MASK) == WL_CHANSPEC_BAND_2G)
#define CHSPEC_SB_NONE(chspec)	((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_NONE)
#define CHSPEC_SB_UPPER(chspec)	((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_UPPER)
#define CHSPEC_SB_LOWER(chspec)	((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LOWER)

/* defined rate in 500kbps */
#define WLC_MAXRATE	108	/* in 500kbps units */
#define WLC_RATE_1M	2	/* in 500kbps units */
#define WLC_RATE_2M	4	/* in 500kbps units */
#define WLC_RATE_5M5	11	/* in 500kbps units */
#define WLC_RATE_11M	22	/* in 500kbps units */
#define WLC_RATE_6M	12	/* in 500kbps units */
#define WLC_RATE_9M	18	/* in 500kbps units */
#define WLC_RATE_12M	24	/* in 500kbps units */
#define WLC_RATE_18M	36	/* in 500kbps units */
#define WLC_RATE_24M	48	/* in 500kbps units */
#define WLC_RATE_36M	72	/* in 500kbps units */
#define WLC_RATE_48M	96	/* in 500kbps units */
#define WLC_RATE_54M	108	/* in 500kbps units */

/*
 * Convert chanspec to ascii string
 * @param	chspec		chanspec format
 * @param	buf		ascii string of chanspec
 * @return	pointer to buf
 */
extern char * wf_chspec_ntoa(chanspec_t chspec, char *buf);

/*
 * Convert ascii string to chanspec
 * @param	a		pointer to input string
 * @return	>= 0 if successful or 0 otherwise
 */
extern chanspec_t wf_chspec_aton(char *a);

#ifdef CONFIG_NET_RADIO
/* wl functions used by the ndis wl. */
extern uint freq2channel(uint freq);
extern uint channel2freq(uint channel);
#endif


#endif	/* _bcmwifi_h_ */
