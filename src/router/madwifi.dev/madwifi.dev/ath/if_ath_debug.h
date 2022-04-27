/*
 * This software is distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_ath_radar.h 2464 2007-06-15 22:51:56Z mtaylor $
 */
#ifndef _IF_ATH_DEBUG_H_

enum {
	ATH_DEBUG_XMIT = 0x00000001,	/* basic xmit operation */
	ATH_DEBUG_XMIT_DESC = 0x00000002,	/* xmit descriptors */
	ATH_DEBUG_RECV = 0x00000004,	/* basic recv operation */
	ATH_DEBUG_RECV_DESC = 0x00000008,	/* recv descriptors */
	ATH_DEBUG_RATE = 0x00000010,	/* rate control */
	ATH_DEBUG_RESET = 0x00000020,	/* reset processing */
	ATH_DEBUG_SKB_REF = 0x00000040,	/* sbk references */
	ATH_DEBUG_BEACON = 0x00000080,	/* beacon handling */
	ATH_DEBUG_WATCHDOG = 0x00000100,	/* watchdog timeout */
	ATH_DEBUG_INTR = 0x00001000,	/* ISR */
	ATH_DEBUG_TX_PROC = 0x00002000,	/* tx ISR proc */
	ATH_DEBUG_RX_PROC = 0x00004000,	/* rx ISR proc */
	ATH_DEBUG_BEACON_PROC = 0x00008000,	/* beacon ISR proc */
	ATH_DEBUG_CALIBRATE = 0x00010000,	/* periodic calibration */
	ATH_DEBUG_KEYCACHE = 0x00020000,	/* key cache management */
	ATH_DEBUG_STATE = 0x00040000,	/* 802.11 state transitions */
	ATH_DEBUG_TSF = 0x00080000,	/* timestamp processing */
	ATH_DEBUG_LED = 0x00100000,	/* led management */
	ATH_DEBUG_FF = 0x00200000,	/* fast frames */
	ATH_DEBUG_TURBO = 0x00400000,	/* turbo/dynamic turbo */
	ATH_DEBUG_UAPSD = 0x00800000,	/* uapsd */
	ATH_DEBUG_DOTH = 0x01000000,	/* 11.h */
	ATH_DEBUG_DOTHFILT = 0x02000000,	/* 11.h radar pulse filter algorithm */
	ATH_DEBUG_DOTHFILTVBSE = 0x04000000,	/* 11.h radar pulse filter algorithm - pulse level debug */
	ATH_DEBUG_DOTHFILTNOSC = 0x08000000,	/* 11.h radar pulse filter algorithm - disable short circuit of processing after detection */
	ATH_DEBUG_DOTHPULSES = 0x10000000,	/* 11.h radar pulse events */
	ATH_DEBUG_TXBUF = 0x20000000,	/* TX buffer usage/leak debugging */
	ATH_DEBUG_SKB = 0x40000000,	/* SKB usage/leak debugging [applies to all vaps] */
	ATH_DEBUG_FATAL = 0x80000000,	/* fatal errors */
	ATH_DEBUG_ANY = 0xffffffff,
	ATH_DEBUG_GLOBAL = (ATH_DEBUG_SKB | ATH_DEBUG_SKB_REF)
};

#define	EPRINTF(_sc, _fmt, ...) \
		printk(KERN_ERR "%s: %s: " _fmt, \
			SC_DEV_NAME(_sc), __func__, ## __VA_ARGS__)

#ifdef AR_DEBUG

/* DEBUG-ONLY DEFINITIONS */
#define	DFLAG_ISSET(sc, _m) ((sc->sc_debug & _m))
#define	DPRINTF(_sc, _m, _fmt, ...) do {				\
	if (DFLAG_ISSET((_sc), (_m))) 					\
		printk(KERN_DEBUG "%s: %s: " _fmt, \
			SC_DEV_NAME(_sc), __func__, ## __VA_ARGS__); \
} while (0)
#define	KEYPRINTF(_sc, _ix, _hk, _mac) do {				\
	if (DFLAG_ISSET((_sc), ATH_DEBUG_KEYCACHE))			\
		ath_keyprint((_sc), __func__, _ix, _hk, _mac);		\
} while (0)

#define	IFF_DUMPPKTS(_sc, _m)   DFLAG_ISSET((_sc), (_m))

#define	WPRINTF(_sc, _fmt, ...) \
		printk(KERN_WARNING "%s: %s: " _fmt, \
			SC_DEV_NAME(_sc), __func__, ## __VA_ARGS__)

#define	IPRINTF(_sc, _fmt, ...) \
		printk(KERN_INFO "%s: %s: " _fmt, \
			SC_DEV_NAME(_sc), __func__, ## __VA_ARGS__)
#else
#define	DFLAG_ISSET(sc, _m)		0
#define	DPRINTF(sc, _m, _fmt, ...)
#define	KEYPRINTF(sc, k, ix, mac)
#define WPRINTF(...)
#define IPRINTF(...)
#define IFF_DUMPPKTS(...) 0

#endif

#endif				/* #ifndef _IF_ATH_DEBUG_H_ */
