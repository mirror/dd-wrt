/*	From: if_pppvar.h,v 1.2 1995/06/12 11:36:51 paulus Exp */
/*
 * if_pppvar.h - private structures and declarations for PPP.
 *
 * Copyright (c) 1989-2002 Paul Mackerras. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name(s) of the authors of this software must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Paul Mackerras
 *     <paulus@samba.org>".
 *
 * THE AUTHORS OF THIS SOFTWARE DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Copyright (c) 1984-2000 Carnegie Mellon University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any legal
 *    details, please contact
 *      Office of Technology Transfer
 *      Carnegie Mellon University
 *      5000 Forbes Avenue
 *      Pittsburgh, PA  15213-3890
 *      (412) 268-4387, fax: (412) 268-7395
 *      tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 *  ==FILEVERSION 990911==
 *
 *  NOTE TO MAINTAINERS:
 *   If you modify this file at all, please set the above date.
 *   if_pppvar.h is shipped with a PPP distribution as well as with the kernel;
 *   if everyone increases the FILEVERSION number above, then scripts
 *   can do the right thing when deciding whether to install a new if_pppvar.h
 *   file.  Don't change the format of that line otherwise, so the
 *   installation script can recognize it.
 */

/*
 * Supported network protocols.  These values are used for
 * indexing sc_npmode.
 */

#define NP_IP	0		/* Internet Protocol */
#define NP_IPX	1		/* IPX protocol */
#define NP_AT	2		/* Appletalk protocol */
#define NP_IPV6	3		/* Internet Protocol */
#define NUM_NP	4		/* Number of NPs. */

#define OBUFSIZE	256	/* # chars of output buffering */

/*
 * Structure describing each ppp unit.
 */

struct ppp {
	int		magic;		/* magic value for structure	*/
	struct ppp	*next;		/* unit with next index		*/
	unsigned long	inuse;		/* are we allocated?		*/
	int		line;		/* network interface unit #	*/
	__u32		flags;		/* miscellaneous control flags	*/
	int		mtu;		/* maximum xmit frame size	*/
	int		mru;		/* maximum receive frame size	*/
	struct slcompress *slcomp;	/* for TCP header compression	*/
	struct sk_buff_head xmt_q;	/* frames to send from pppd	*/
	struct sk_buff_head rcv_q;	/* frames for pppd to read	*/
	unsigned long	xmit_busy;	/* bit 0 set when xmitter busy  */

	/* Information specific to using ppp on async serial lines. */
	struct tty_struct *tty;		/* ptr to TTY structure	*/
	struct tty_struct *backup_tty;	/* TTY to use if tty gets closed */
	__u8		escape;		/* 0x20 if prev char was PPP_ESC */
	__u8		toss;		/* toss this frame		*/
	volatile __u8	tty_pushing;	/* internal state flag		*/
	volatile __u8	woke_up;	/* internal state flag		*/
	__u32		xmit_async_map[8]; /* 1 bit means that given control 
					   character is quoted on output*/
	__u32		recv_async_map; /* 1 bit means that given control 
					   character is ignored on input*/
	__u32		bytes_sent;	/* Bytes sent on frame	*/
	__u32		bytes_rcvd;	/* Bytes recvd on frame	*/

	/* Async transmission information */
	struct sk_buff	*tpkt;		/* frame currently being sent	*/
	int		tpkt_pos;	/* how much of it we've done	*/
	__u16		tfcs;		/* FCS so far for it		*/
	unsigned char	*optr;		/* where we're up to in sending */
	unsigned char	*olim;		/* points past last valid char	*/

	/* Async reception information */
	struct sk_buff	*rpkt;		/* frame currently being rcvd	*/
	__u16		rfcs;		/* FCS so far of rpkt		*/

	/* Queues for select() functionality */
	struct wait_queue *read_wait;	/* queue for reading processes	*/

	/* info for detecting idle channels */
	unsigned long	last_xmit;	/* time of last transmission	*/
	unsigned long	last_recv;	/* time last packet received    */

	/* Statistic information */
	struct pppstat	stats;		/* statistic information	*/

	/* PPP compression protocol information */
	struct	compressor *sc_xcomp;	/* transmit compressor */
	void	*sc_xc_state;		/* transmit compressor state */
	struct	compressor *sc_rcomp;	/* receive decompressor */
	void	*sc_rc_state;		/* receive decompressor state */

	enum	NPmode sc_npmode[NUM_NP]; /* what to do with each NP */
	int	 sc_xfer;		/* PID of reserved PPP table */
	char	name[8];		/* space for unit name */
	struct device	dev;		/* net device structure */
	struct enet_statistics estats;	/* more detailed stats */

	/* tty output buffer */
	unsigned char	obuf[OBUFSIZE];	/* buffer for characters to send */
};

#define PPP_MAGIC	0x5002
#define PPP_VERSION	"2.3.11"
