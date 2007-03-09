//==========================================================================
//
//      net/tcp.c
//
//      Stand-alone TCP networking support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <net/net.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>

#define MAX_TCP_SEGMENT (ETH_MAX_PKTLEN - (sizeof(eth_header_t) + sizeof(ip_header_t)))
#define MAX_TCP_DATA    (MAX_TCP_SEGMENT - sizeof(tcp_header_t))


/* sequence number comparison macros */
#define SEQ_LT(a,b) ((int)((a)-(b)) < 0)
#define SEQ_LE(a,b) ((int)((a)-(b)) <= 0)
#define SEQ_GT(a,b) ((int)((a)-(b)) > 0)
#define SEQ_GE(a,b) ((int)((a)-(b)) >= 0)

/* Set a timer which will send an RST and abort a connection. */
static timer_t abort_timer;

static void do_retrans(void *p);
static void do_close(void *p);

#ifdef BSP_LOG
static char *
flags_to_str(octet f)
{
    static char str[7], *p;

    p = str;

    if (f & TCP_FLAG_FIN)
	*p++ = 'F';
    if (f & TCP_FLAG_SYN)
	*p++ = 'S';
    if (f & TCP_FLAG_RST)
	*p++ = 'R';
    if (f & TCP_FLAG_PSH)
	*p++ = 'P';
    if (f & TCP_FLAG_ACK)
	*p++ = 'A';
    if (f & TCP_FLAG_URG)
	*p++ = 'U';
    *p = '\0';
    return str;
}
#endif

/*
 * A major assumption is that only a very small number of sockets will
 * active, so a simple linear search of those sockets is acceptible.
 */
static tcp_socket_t *tcp_list;

/*
 * Format and send an outgoing segment.
 */
static void
tcp_send(tcp_socket_t *s, int flags, int resend)
{
    tcp_header_t *tcp;
    ip_header_t  *ip;
    pktbuf_t     *pkt = &s->pkt;
    unsigned short cksum;
    dword         tcp_magic;
    int           tcp_magic_size = sizeof(tcp_magic);

    ip = pkt->ip_hdr;
    tcp = pkt->tcp_hdr;

    if (flags & TCP_FLAG_SYN) {
	/* If SYN, assume no data and send MSS option in tcp header */
	pkt->pkt_bytes = sizeof(tcp_header_t) + 4;
	tcp->hdr_len = 6;
        tcp_magic = htonl(0x02040000 | MAX_TCP_DATA);
	memcpy((unsigned char *)(tcp+1), &tcp_magic, tcp_magic_size);
	s->data_bytes = 0;
    } else {
	pkt->pkt_bytes = s->data_bytes + sizeof(tcp_header_t);
	tcp->hdr_len = 5;
    }

    /* tcp header */
    tcp->reserved = 0;
    tcp->seqnum = htonl(s->seq);
    tcp->acknum = htonl(s->ack);
    tcp->checksum = 0;

    if (!resend) {
	tcp->src_port = htons(s->our_port);
	tcp->dest_port = htons(s->his_port);
	tcp->flags = flags;
	/* always set PUSH flag if sending data */
	if (s->data_bytes)
	    tcp->flags |= TCP_FLAG_PSH;
	tcp->window = htons(MAX_TCP_DATA);
	tcp->urgent = 0;

	/* fill in some pseudo-header fields */
	memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
	memcpy(ip->destination, s->his_addr.ip_addr, sizeof(ip_addr_t));
	ip->protocol = IP_PROTO_TCP;
    }

    /* another pseudo-header field */
    ip->length = htons(pkt->pkt_bytes);

    /* compute tcp checksum */
    cksum = __sum((word *)tcp, pkt->pkt_bytes, __pseudo_sum(ip));
    tcp->checksum = htons(cksum);

    __ip_send(pkt, IP_PROTO_TCP, &s->his_addr);

    // HACK!  If this delay is not present, then if the target system sends
    // back data (not just an ACK), then somehow we miss it :-(
    CYGACC_CALL_IF_DELAY_US(2*1000);

    BSPLOG(bsp_log("tcp_send: state[%d] flags[%s] ack[%x] data[%d].\n",
		   s->state, flags_to_str(tcp->flags), s->ack, s->data_bytes));

    if (s->state == _TIME_WAIT) {
        // If 'reuse' is set on socket, close after 1 second, otherwise 2 minutes
        __timer_set(&s->timer, s->reuse ? 1000 : 120000, do_close, s);
    }
    else if ((tcp->flags & (TCP_FLAG_FIN | TCP_FLAG_SYN)) || s->data_bytes)
	__timer_set(&s->timer, 1000, do_retrans, s);
}

static pktbuf_t ack_pkt;
static word     ack_buf[ETH_MIN_PKTLEN/sizeof(word)];

/*
 * Send an ack.
 */
static void
send_ack(tcp_socket_t *s)
{
    tcp_header_t *tcp;
    ip_header_t  *ip;
    unsigned short cksum;

    ack_pkt.buf = ack_buf;
    ack_pkt.bufsize = sizeof(ack_buf);
    ack_pkt.ip_hdr = ip = (ip_header_t *)ack_buf;
    ack_pkt.tcp_hdr = tcp = (tcp_header_t *)(ip + 1);
    ack_pkt.pkt_bytes = sizeof(tcp_header_t);

    /* tcp header */
    tcp->hdr_len = 5;
    tcp->reserved = 0;
    tcp->seqnum = htonl(s->seq);
    tcp->acknum = htonl(s->ack);
    tcp->checksum = 0;

    tcp->src_port = htons(s->our_port);
    tcp->dest_port = htons(s->his_port);
    tcp->flags = TCP_FLAG_ACK;

    tcp->window = htons(MAX_TCP_DATA);
    tcp->urgent = 0;

    /* fill in some pseudo-header fields */
    memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(ip->destination, s->his_addr.ip_addr, sizeof(ip_addr_t));
    ip->protocol = IP_PROTO_TCP;

    /* another pseudo-header field */
    ip->length = htons(sizeof(tcp_header_t));

    /* compute tcp checksum */
    cksum = __sum((word *)tcp, sizeof(*tcp), __pseudo_sum(ip));
    tcp->checksum = htons(cksum);

    __ip_send(&ack_pkt, IP_PROTO_TCP, &s->his_addr);
}


/*
 * Send a reset for a bogus incoming segment.
 */
static void
send_reset(pktbuf_t *pkt, ip_route_t *r)
{
    ip_header_t   *ip = pkt->ip_hdr;
    tcp_header_t  *tcp = pkt->tcp_hdr;
    dword         seq, ack;
    word          src, dest;
    word          cksum;

    seq = ntohl(tcp->acknum);
    ack = ntohl(tcp->seqnum);
    src = ntohs(tcp->dest_port);
    dest = ntohs(tcp->src_port);

    tcp = (tcp_header_t *)(ip + 1);
    pkt->pkt_bytes = sizeof(tcp_header_t);
    
    /* tcp header */
    tcp->hdr_len = 5;
    tcp->reserved = 0;
    tcp->seqnum = htonl(seq);
    tcp->acknum = htonl(ack);
    tcp->window = htons(1024);
    tcp->urgent = 0;
    tcp->checksum = 0;
    tcp->src_port = htons(src);
    tcp->dest_port = htons(dest);
    tcp->flags = TCP_FLAG_RST | TCP_FLAG_ACK;

    /* fill in some pseudo-header fields */
    memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(ip->destination, r->ip_addr, sizeof(ip_addr_t));
    ip->protocol = IP_PROTO_TCP;
    ip->length = htons(pkt->pkt_bytes);

    /* compute tcp checksum */
    cksum = __sum((word *)tcp, pkt->pkt_bytes, __pseudo_sum(ip));
    tcp->checksum = htons(cksum);

    __ip_send(pkt, IP_PROTO_TCP, r);
}



/*
 * Remove given socket from socket list.
 */
static void
unlink_socket(tcp_socket_t *s)
{
    tcp_socket_t *prev, *tp;

    for (prev = NULL, tp = tcp_list; tp; prev = tp, tp = tp->next)
	if (tp == s) {
	    BSPLOG(bsp_log("unlink tcp socket.\n"));
	    if (prev)
		prev->next = s->next;
	    else
		tcp_list = s->next;
	}
}

/*
 * Retransmit last packet.
 */
static void
do_retrans(void *p)
{
    BSPLOG(bsp_log("tcp do_retrans.\n"));
    tcp_send((tcp_socket_t *)p, 0, 1);
}


static void
do_close(void *p)
{
    BSPLOG(bsp_log("tcp do_close.\n"));
    /* close connection */
    ((tcp_socket_t *)p)->state = _CLOSED;
    unlink_socket(p);
}


static void
free_rxlist(tcp_socket_t *s)
{
    pktbuf_t *p;

    BSPLOG(bsp_log("tcp free_rxlist.\n"));

    while ((p = s->rxlist) != NULL) {
	s->rxlist = p->next;
	__pktbuf_free(p);
    }
}


/*
 * Handle a conection reset.
 */
static void
do_reset(tcp_socket_t *s)
{
    /* close connection */
    s->state = _CLOSED;
    __timer_cancel(&s->timer);
    free_rxlist(s);
    unlink_socket(s);
}


/*
 * Extract data from incoming tcp segment.
 * Returns true if packet is queued on rxlist, false otherwise.
 */
static int
handle_data(tcp_socket_t *s, pktbuf_t *pkt)
{
    tcp_header_t  *tcp = pkt->tcp_hdr;
    unsigned int  diff, seq;
    int           data_len;
    char          *data_ptr;
    pktbuf_t      *p;

    data_len = pkt->pkt_bytes - (tcp->hdr_len << 2);
    data_ptr = ((char *)tcp)  + (tcp->hdr_len << 2);

    seq = ntohl(tcp->seqnum);

    BSPLOG(bsp_log("tcp data: seq[%x] len[%d].\n", seq, data_len));

    if (SEQ_LE(seq, s->ack)) {
	/*
	 * Figure difference between which byte we're expecting and which byte
	 * is sent first. Adjust data length and data pointer accordingly.
	 */
	diff = s->ack - seq;
	data_len -= diff;
	data_ptr += diff;

	if (data_len > 0) {
	    /* queue the new data */
	    s->ack += data_len;
	    pkt->next = NULL;
	    if ((p = s->rxlist) != NULL) {
		while (p->next)
		    p = p->next;
		p->next = pkt;
		BSPLOG(bsp_log("tcp data: Add pkt[%x] len[%d].\n",
			       pkt, data_len));
	    } else {
		s->rxlist = pkt;
		s->rxcnt = data_len;
		s->rxptr = data_ptr;
		BSPLOG(bsp_log("tcp data: pkt[%x] len[%d].\n",
			       pkt, data_len));
	    }
	    return 1;
	}
    }
    return 0;
}


static void
handle_ack(tcp_socket_t *s, pktbuf_t *pkt)
{
    tcp_header_t *tcp = pkt->tcp_hdr;
    dword        ack;
    int          advance;
    char         *dp;

    /* process ack value in packet */
    ack = ntohl(tcp->acknum);

    BSPLOG(bsp_log("Rcvd tcp ACK %x\n", ack));

    if (SEQ_GT(ack, s->seq)) {
	__timer_cancel(&s->timer);
	advance = ack - s->seq;
	if (advance > s->data_bytes)
	    advance = s->data_bytes;

	BSPLOG(bsp_log("seq advance %d", advance));

	if (advance > 0) {
	    s->seq += advance;
	    s->data_bytes -= advance;
	    if (s->data_bytes) {
		/* other end ack'd only part of the pkt */
		BSPLOG(bsp_log(" %d bytes left", s->data_bytes));
		dp = (char *)(s->pkt.tcp_hdr + 1);
		memcpy(dp, dp + advance, s->data_bytes);
	    }
	}
    }
    BSPLOG(bsp_log("\n"));
}


/*
 * Handle incoming TCP packets.
 */
void
__tcp_handler(pktbuf_t *pkt, ip_route_t *r)
{
    tcp_header_t *tcp = pkt->tcp_hdr;
    ip_header_t  *ip = pkt->ip_hdr;
    tcp_socket_t *prev,*s;
    dword        ack;
    int          queued = 0;

    /* set length for pseudo sum calculation */
    ip->length = htons(pkt->pkt_bytes);

    if (__sum((word *)tcp, pkt->pkt_bytes, __pseudo_sum(ip)) == 0) {
	for (prev = NULL, s = tcp_list; s; prev = s, s = s->next) {
	    if (s->our_port == ntohs(tcp->dest_port)) {
		if (s->his_port == 0)
		    break;
		if (s->his_port == ntohs(tcp->src_port) &&
		    !memcmp(r->ip_addr, s->his_addr.ip_addr, sizeof(ip_addr_t)))
		    break;
	    }
	}

	if (s) {
	    /* found the socket this packet belongs to */
	    
	    /* refresh his ethernet address */
	    memcpy(s->his_addr.enet_addr, r->enet_addr, sizeof(enet_addr_t));

	    if (s->state != _SYN_RCVD && tcp->flags & TCP_FLAG_RST) {
		BSPLOG(bsp_log("TCP_FLAG_RST rcvd\n"));
		do_reset(s);
		__pktbuf_free(pkt);
		return;
	    }

	    switch (s->state) {

	      case _SYN_SENT:
		/* active open not supported */
                  if (tcp->flags != (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                      do_reset(s);
                      __pktbuf_free(pkt);
                      return;
                  }
                  s->state = _ESTABLISHED;
                  s->ack = ntohl(tcp->seqnum) + 1;
                  s->seq = ntohl(tcp->acknum);
		  __timer_cancel(&s->timer);
                  send_ack(s);
		break;

	      case _LISTEN:
		if (tcp->flags & TCP_FLAG_SYN) {
		    s->state = _SYN_RCVD;
		    s->ack = ntohl(tcp->seqnum) + 1;
		    s->his_port = ntohs(tcp->src_port);
		    memcpy(s->his_addr.ip_addr, r->ip_addr, sizeof(ip_addr_t));
		    s->data_bytes = 0;

		    BSPLOG(bsp_log("SYN from %d.%d.%d.%d:%d (seq %x)\n",
			       s->his_addr.ip_addr[0],s->his_addr.ip_addr[1],
			       s->his_addr.ip_addr[2],s->his_addr.ip_addr[3],
			       s->his_port, ntohl(tcp->seqnum)));

		    tcp_send(s, TCP_FLAG_SYN | TCP_FLAG_ACK, 0);
		}
		else
		    send_reset(pkt, r);
		break;

	      case _SYN_RCVD:
		BSPLOG(bsp_log("_SYN_RCVD timer cancel.\n"));
		__timer_cancel(&s->timer);

		/* go back to _LISTEN state if reset */
		if (tcp->flags & TCP_FLAG_RST) {
		    s->state = _LISTEN;

		    BSPLOG(bsp_log("_SYN_RCVD --> _LISTEN\n"));

		} else if (tcp->flags & TCP_FLAG_SYN) {
		    /* apparently our SYN/ACK was lost? */
		    tcp_send(s, 0, 1);

		    BSPLOG(bsp_log("retransmitting SYN/ACK\n"));

		} else if ((tcp->flags & TCP_FLAG_ACK) &&
			   ntohl(tcp->acknum) == (s->seq + 1)) {
		    /* we've established the connection */
		    s->state = _ESTABLISHED;
		    s->seq++;

		    BSPLOG(bsp_log("ACK received - connection established\n"));
		}
		break;

	      case _ESTABLISHED:
	      case _CLOSE_WAIT:
		ack = s->ack;  /* save original ack */
		if (tcp->flags & TCP_FLAG_ACK)
		    handle_ack(s, pkt);

		queued = handle_data(s, pkt);

		if ((tcp->flags & TCP_FLAG_FIN) &&
		    ntohl(tcp->seqnum) == s->ack) {

		    BSPLOG(bsp_log("FIN received - going to _CLOSE_WAIT\n"));

		    s->ack++;
		    s->state = _CLOSE_WAIT;
		}
		/*
		 * Send an ack if neccessary.
		 */
		if (s->ack != ack || pkt->pkt_bytes > (tcp->hdr_len << 2))
		    send_ack(s);
		break;

	      case _LAST_ACK:
		if (tcp->flags & TCP_FLAG_ACK) {
		    handle_ack(s, pkt);
		    if (ntohl(tcp->acknum) == (s->seq + 1)) {
			BSPLOG(bsp_log("_LAST_ACK --> _CLOSED\n"));
			s->state = _CLOSED;
			unlink_socket(s);
		    }
		}
		break;

	      case _FIN_WAIT_1:
		if (tcp->flags & TCP_FLAG_ACK) {
		    handle_ack(s, pkt);
		    if (ntohl(tcp->acknum) == (s->seq + 1)) {
			/* got ACK for FIN packet */
			s->seq++;
			if (tcp->flags & TCP_FLAG_FIN) {
			    BSPLOG(bsp_log("_FIN_WAIT_1 --> _TIME_WAIT\n"));
			    s->ack++;
			    s->state = _TIME_WAIT;
			    send_ack(s);
			} else {
			    s->state = _FIN_WAIT_2;
			    BSPLOG(bsp_log("_FIN_WAIT_1 --> _FIN_WAIT_2\n"));
			}
                        break; /* All done for now */
		    }
		}
                /* At this point, no ACK for FIN has been seen, so check for
                   simultaneous close */
		if (tcp->flags & TCP_FLAG_FIN) {
		    BSPLOG(bsp_log("_FIN_WAIT_1 --> _CLOSING\n"));
		    __timer_cancel(&s->timer);
		    s->ack++;
		    s->state = _CLOSING;
                    /* FIN is resent so the timeout and retry for this packet
                       will also take care of timeout and resend of the
                       previously sent FIN (which got us to FIN_WAIT_1). While
                       not technically correct, resending FIN only causes a
                       duplicate FIN (same sequence number) which should be
                       ignored by the other end. */
		    tcp_send(s, TCP_FLAG_FIN | TCP_FLAG_ACK, 0);
		}
		break;

	      case _FIN_WAIT_2:
		queued = handle_data(s, pkt);
		if (tcp->flags & TCP_FLAG_FIN) {
		    BSPLOG(bsp_log("_FIN_WAIT_2 --> _TIME_WAIT\n"));
		    s->ack++;
		    s->state = _TIME_WAIT;
		    send_ack(s);
		}
		break;

	      case _CLOSING:
		if (tcp->flags & TCP_FLAG_ACK) {
		    handle_ack(s, pkt);
		    if (ntohl(tcp->acknum) == (s->seq + 1)) {
			/* got ACK for FIN packet */
			BSPLOG(bsp_log("_CLOSING --> _TIME_WAIT\n"));
			__timer_cancel(&s->timer);
			s->state = _TIME_WAIT;
		    }
		}
		break;

	      case _TIME_WAIT:
		BSPLOG(bsp_log("_TIME_WAIT resend.\n"));
		if (tcp->flags & TCP_FLAG_FIN)
		    tcp_send(s, 0, 1); /* just resend ack */
		break;
	    }
	} else {
	    BSPLOG(bsp_log("Unexpected segment from: %d.%d.%d.%d:%d\n",
			   r->ip_addr[0], r->ip_addr[1], r->ip_addr[3],
			   r->ip_addr[4], ntohs(tcp->src_port)));
	    send_reset(pkt, r);
	}
    }
    if (!queued)
	__pktbuf_free(pkt);
}


void
__tcp_poll(void)
{
    __enet_poll();
    MS_TICKS_DELAY();
    __timer_poll();
}


int
__tcp_listen(tcp_socket_t *s, word port)
{
    BSPLOG(bsp_log("tcp_listen: s[%p] port[%x]\n", s, port));

    memset(s, 0, sizeof(tcp_socket_t));
    s->state    = _LISTEN;
    s->our_port = port;
    s->pkt.buf = (word *)s->pktbuf;
    s->pkt.bufsize = ETH_MAX_PKTLEN;
    s->pkt.ip_hdr  = (ip_header_t *)s->pkt.buf;
    s->pkt.tcp_hdr = (tcp_header_t *)(s->pkt.ip_hdr + 1);

    s->next = tcp_list;

#if 0
    /* limit to one open socket at a time */
    if (s->next) {
	BSPLOG(bsp_log("tcp_listen: recursion error\n"));
	BSPLOG(while(1));
    }
#endif
    
    tcp_list = s;

    return 0;
}

/*
 * SO_REUSEADDR, no 2MSL.
 */
void
__tcp_so_reuseaddr(tcp_socket_t *s)
{
//    BSPLOG(bsp_log("__tcp_so_reuseaddr.\n"));
    s->reuse = 0x01;
}

/*
 * Block while waiting for all data to be transmitted.
 */
void
__tcp_drain(tcp_socket_t *s)
{
//    BSPLOG(bsp_log("__tcp_drain.\n"));
    while (s->state != _CLOSED && s->data_bytes)
	__tcp_poll();
//    BSPLOG(bsp_log("__tcp_drain done.\n"));
}


/*
 * Close the tcp connection.
 */
static void
do_abort(void *s)
{
    BSPLOG(bsp_log("do_abort: send RST\n"));
    tcp_send((tcp_socket_t *)s, TCP_FLAG_ACK | TCP_FLAG_RST, 0);
    __timer_cancel(&abort_timer);
    ((tcp_socket_t *)s)->state = _CLOSED;
    free_rxlist((tcp_socket_t *)s);
    unlink_socket((tcp_socket_t *)s);
}

void
__tcp_abort(tcp_socket_t *s, unsigned long delay)
{
  __timer_set(&abort_timer, delay, do_abort, s);
}

/*
 * Close the tcp connection.
 */
void
__tcp_close(tcp_socket_t *s)
{
    __tcp_drain(s);
    if (s->state == _ESTABLISHED || s->state == _SYN_RCVD) {
	BSPLOG(bsp_log("__tcp_close: going to _FIN_WAIT_1\n"));
	s->state = _FIN_WAIT_1;
	tcp_send(s, TCP_FLAG_ACK | TCP_FLAG_FIN, 0);
    } else if (s->state == _CLOSE_WAIT) {

	BSPLOG(bsp_log("__tcp_close: going to _LAST_ACK\n"));

	s->state = _LAST_ACK;
	tcp_send(s, TCP_FLAG_ACK | TCP_FLAG_FIN, 0);
    }
    free_rxlist(s);
}


/*
 * Wait for connection to be fully closed.
 */
void
__tcp_close_wait(tcp_socket_t *s)
{
    BSPLOG(bsp_log("__tcp_close_wait.\n"));
    while (s->state != _CLOSED)
	__tcp_poll();
    BSPLOG(bsp_log("__tcp_close_wait done.\n"));
}


/*
 * Read up to 'len' bytes without blocking.
 */
int
__tcp_read(tcp_socket_t *s, char *buf, int len)
{
    int          nread;
    pktbuf_t     *pkt;
    tcp_header_t *tcp;

    if (len <= 0 || s->rxcnt == 0)
	return 0;

    if (s->state != _ESTABLISHED && s->rxcnt == 0)
	return -1;

    nread = 0;
    while (len) {
	if (len < s->rxcnt) {
	    memcpy(buf, s->rxptr, len);
	    BSPLOG(bsp_log("tcp_read: read %d bytes.\n", len));
	    s->rxptr += len;
	    s->rxcnt -= len;
	    nread    += len;

	    BSPLOG(bsp_log("tcp_read: %d bytes left in rxlist head.\n",
		       s->rxcnt));

	    break;
	} else {
	    memcpy(buf, s->rxptr, s->rxcnt);
	    BSPLOG(bsp_log("tcp_read: read %d bytes. pkt[%x] freed.\n",
			   s->rxcnt, s->rxlist));
	    nread += s->rxcnt;
	    buf   += s->rxcnt;
	    len   -= s->rxcnt;

	    /* setup for next packet in list */
	    pkt = s->rxlist;
	    s->rxlist = pkt->next;
	    __pktbuf_free(pkt);

	    if ((pkt = s->rxlist) != NULL) {
		tcp = pkt->tcp_hdr;
		s->rxcnt = pkt->pkt_bytes - (tcp->hdr_len << 2);
		s->rxptr = ((char *)tcp)  + (tcp->hdr_len << 2);

		BSPLOG(bsp_log("tcp_read: next pkt[%x] has %d bytes.\n",
			   s->rxlist, s->rxcnt));
	    } else {

		BSPLOG(bsp_log("tcp_read: no more data.\n"));

		s->rxcnt = 0;
		break;
	    }
	}
    }
    return nread;
}


/*
 * Write up to 'len' bytes without blocking
 */
int
__tcp_write(tcp_socket_t *s, char *buf, int len)
{
    tcp_header_t *tcp = s->pkt.tcp_hdr;

    if (len <= 0)
	return 0;

    if (s->state != _ESTABLISHED && s->state != _CLOSE_WAIT)
	return -1;

    if (s->data_bytes)
	return 0;

    if (len > MAX_TCP_DATA)
	len = MAX_TCP_DATA;

    memcpy(tcp + 1, buf, len);
    s->data_bytes = len;

    tcp_send(s, TCP_FLAG_ACK, 0);

    return len;
}

/*
 * Write 'len' bytes from 'buf', blocking until sent.
 * If connection collapses, return -1
 */
int
__tcp_write_block(tcp_socket_t *s, char *buf, int len)
{
    int total = 0;
    int n;

    while (len) {
        if (s->state == _CLOSE_WAIT) {
            // This connection is tring to close
            // This connection is breaking
            if (s->data_bytes == 0 && s->rxcnt == 0)
                __tcp_close(s);
        }
        if (s->state == _CLOSED) {
            // The connection is gone!
            return -1;
        }
        n = __tcp_write(s, buf, len);
        if (n > 0) {
            len -= n;
            buf += n;
        }
        __tcp_poll();
    }
    __tcp_drain(s);
    return total;
}

/*
 * Establish a new [outgoing] connection, with a timeout.
 */
int 
__tcp_open(tcp_socket_t *s, struct sockaddr_in *host, 
           word port, int timeout, int *err)
{
    // Fill in socket details
    memset(s, 0, sizeof(tcp_socket_t));
    s->state = _SYN_SENT;
    s->our_port = port;
    s->his_port = host->sin_port;
    s->pkt.buf = (word *)s->pktbuf;
    s->pkt.bufsize = ETH_MAX_PKTLEN;
    s->pkt.ip_hdr  = (ip_header_t *)s->pkt.buf;
    s->pkt.tcp_hdr = (tcp_header_t *)(s->pkt.ip_hdr + 1);
    s->seq = (port << 16) | 0xDE77;
    s->ack = 0;
    if (__arp_lookup((ip_addr_t *)&host->sin_addr, &s->his_addr) < 0) {
        diag_printf("%s: Can't find address of server\n", __FUNCTION__);
        return -1;
    }
    s->next = tcp_list;
    tcp_list = s;

    // Send off the SYN packet to open the connection
    tcp_send(s, TCP_FLAG_SYN, 0);
    // Wait for connection to establish
    while (s->state != _ESTABLISHED) {
        if (s->state == _CLOSED) {
            diag_printf("TCP open - host closed connection\n");
            return -1;
        }
        if (--timeout <= 0) {
            diag_printf("TCP open - connection timed out\n");
            return -1;
        }
        MS_TICKS_DELAY();
        __tcp_poll();
    }
    return 0;
}


