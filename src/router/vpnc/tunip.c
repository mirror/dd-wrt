/* IPSec ESP and AH support.
   Copyright (c) 1999      Pierre Beyssac
   Copyright (C) 2002      Geoffrey Keating
   Copyright (C) 2003-2007 Maurice Massar
   Copyright (C) 2004      Tomas Mraz
   Copyright (C) 2005      Michael Tilstra
   Copyright (C) 2006      Daniel Roethlisberger
   Copyright (C) 2007      Paolo Zarpellon (tap+Cygwin support)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: tunip.c 371 2008-11-19 20:55:28Z Joerg Mayer $
*/

/* borrowed from pipsecd (-; */

/*-
 * Copyright (c) 1999 Pierre Beyssac
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#ifndef __SKYOS__
#include <netinet/ip_icmp.h>
#endif
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>
#include <sys/select.h>
#include <signal.h>

#ifdef __CYGWIN__
#include <pthread.h>
#endif

#if !defined(__sun__) && !defined(__SKYOS__)
#include <err.h>
#endif

#include <gcrypt.h>
#include "sysdep.h"
#include "config.h"
#include "vpnc.h"

#include "tunip.h"

#ifndef MAX
#define MAX(a,b)	((a)>(b)?(a):(b))
#endif

#ifndef FD_COPY
#define FD_COPY(f, t)	((void)memcpy((t), (f), sizeof(*(f))))
#endif

/* A real ESP header (RFC 2406) */
typedef struct esp_encap_header {
	uint32_t spi; /* security parameters index */
	uint32_t seq_id; /* sequence id (unimplemented) */
	/* variable-length payload data + padding */
	/* unsigned char next_header */
	/* optional auth data */
} __attribute__((packed)) esp_encap_header_t;

struct encap_method {
	int fixed_header_size;
	
	int  (*recv)      (struct sa_block *s, unsigned char *buf, unsigned int bufsize);
	void (*send_peer) (struct sa_block *s, unsigned char *buf, unsigned int bufsize);
	int  (*recv_peer) (struct sa_block *s);
};

/* Yuck! Global variables... */

#define MAX_HEADER 72
#define MAX_PACKET 4096
int volatile do_kill;
static uint8_t global_buffer_rx[MAX_HEADER + MAX_PACKET + ETH_HLEN];
static uint8_t global_buffer_tx[MAX_HEADER + MAX_PACKET + ETH_HLEN];

/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
static u_short in_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
	sum += (sum >> 16); /* add carry */
	answer = ~sum; /* truncate to 16 bits */
	return (answer);
}

/*
 * Decapsulate from a raw IP packet
 */
static int encap_rawip_recv(struct sa_block *s, unsigned char *buf, unsigned int bufsize)
{
	ssize_t r;
	struct ip *p = (struct ip *)buf;
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	
	r = recvfrom(s->esp_fd, buf, bufsize, 0, (struct sockaddr *)&from, &fromlen);
	if (r == -1) {
		syslog(LOG_ERR, "recvfrom: %m");
		return -1;
	}
	if (from.sin_addr.s_addr != s->dst.s_addr) {
		syslog(LOG_ALERT, "packet from unknown host %s", inet_ntoa(from.sin_addr));
		return -1;
	}
	if (r < (p->ip_hl << 2) + s->ipsec.em->fixed_header_size) {
		syslog(LOG_ALERT, "packet too short. got %d, expected %d", r, (p->ip_hl << 2) + s->ipsec.em->fixed_header_size);
		return -1;
	}

#ifdef NEED_IPLEN_FIX
	p->ip_len = r;
#else
	p->ip_len = ntohs(r);
#endif

	s->ipsec.rx.buf = buf;
	s->ipsec.rx.buflen = r;
	s->ipsec.rx.bufpayload = (p->ip_hl << 2);
	s->ipsec.rx.bufsize = bufsize;
	return r;
}

/*
 * Decapsulate from an UDP packet
 */
static int encap_udp_recv(struct sa_block *s, unsigned char *buf, unsigned int bufsize)
{
	ssize_t r;

	r = recv(s->esp_fd, buf, bufsize, 0);
	if (r == -1) {
		syslog(LOG_ERR, "recvfrom: %m");
		return -1;
	}
	if (s->ipsec.natt_active_mode == NATT_ACTIVE_DRAFT_OLD && r > 8) {
		r -= 8;
		memmove(buf, buf + 8, r);
	}
	if( r == 1 && *buf == 0xff )
	{
		DEBUGTOP(1, printf("UDP NAT keepalive packet received\n"));
		return -1;
	}
	if (r < s->ipsec.em->fixed_header_size) {
		syslog(LOG_ALERT, "packet too short from %s. got %d, expected %d",
			inet_ntoa(s->dst), r, s->ipsec.em->fixed_header_size);
		return -1;
	}

	s->ipsec.rx.buf = buf;
	s->ipsec.rx.buflen = r;
	s->ipsec.rx.bufpayload = 0;
	s->ipsec.rx.bufsize = bufsize;
	return r;
}

/*
 * Decapsulate packet
 */
static int encap_any_decap(struct sa_block *s)
{
	s->ipsec.rx.buflen -= s->ipsec.rx.bufpayload + s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size;
	s->ipsec.rx.buf    += s->ipsec.rx.bufpayload + s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size;
	if (s->ipsec.rx.buflen == 0)
		return 0;
	return 1;
}

/*
 * Send decapsulated packet to tunnel device
 */
static int tun_send_ip(struct sa_block *s)
{
	int sent, len;
	uint8_t *start;
	
	start = s->ipsec.rx.buf;
	len   = s->ipsec.rx.buflen;
	
	if (opt_if_mode == IF_MODE_TAP) {
#ifndef __sun__
		/*
		 * Add ethernet header before s->ipsec.rx.buf where
		 * at least ETH_HLEN bytes should be available.
		 */
		struct ether_header *eth_hdr = (struct ether_header *) (s->ipsec.rx.buf - ETH_HLEN);
		
		memcpy(eth_hdr->ether_dhost, s->tun_hwaddr, ETH_ALEN);
		memcpy(eth_hdr->ether_shost, s->tun_hwaddr, ETH_ALEN);
		
		/* Use a different MAC as source */
		eth_hdr->ether_shost[0] ^= 0x80; /* toggle some visible bit */
		eth_hdr->ether_type = htons(ETHERTYPE_IP);
		
		start = (uint8_t *) eth_hdr;
		len += ETH_HLEN;
#endif
	}
	
	sent = tun_write(s->tun_fd, start, len);
	if (sent != len)
		syslog(LOG_ERR, "truncated in: %d -> %d\n", len, sent);
	hex_dump("Tx pkt", start, len, NULL);
	return 1;
}

/*
 * Compute HMAC for an arbitrary stream of bytes
 */
static int hmac_compute(int md_algo,
	const unsigned char *data, unsigned int data_size,
	unsigned char *digest, unsigned char do_store,
	const unsigned char *secret, unsigned short secret_size)
{
	gcry_md_hd_t md_ctx;
	int ret;
	unsigned char *hmac_digest;
	unsigned int hmac_len;

	/* See RFC 2104 */
	gcry_md_open(&md_ctx, md_algo, GCRY_MD_FLAG_HMAC);
	assert(md_ctx != NULL);
	ret = gcry_md_setkey(md_ctx, secret, secret_size);
	assert(ret == 0);
	gcry_md_write(md_ctx, data, data_size);
	gcry_md_final(md_ctx);
	hmac_digest = gcry_md_read(md_ctx, 0);
	hmac_len = 12; /*gcry_md_get_algo_dlen(md_algo); see RFC .. only use 96 bit */

	if (do_store) {
		memcpy(digest, hmac_digest, hmac_len);
		ret = 0;
	} else
		ret = memcmp(digest, hmac_digest, hmac_len);

	gcry_md_close(md_ctx);
	return ret;
}

/*
 * Encapsulate a packet in ESP
 */
static void encap_esp_encapsulate(struct sa_block *s)
{
	esp_encap_header_t *eh;
	unsigned char *iv, *cleartext;
	size_t i, padding, pad_blksz;
	unsigned int cleartextlen;

	/*
	 * Add padding as necessary
	 *
	 * done: this should be checked, RFC 2406 section 2.4 is quite
	 *      obscure on that point.
	 * seems fine
	 */
	pad_blksz = s->ipsec.blk_len;
	while (pad_blksz & 3) /* must be multiple of 4 */
		pad_blksz <<= 1;
	padding = pad_blksz - ((s->ipsec.tx.buflen + 2 - s->ipsec.tx.var_header_size - s->ipsec.tx.bufpayload) % pad_blksz);
	DEBUG(3, printf("sending packet: len = %d, padding = %lu\n", s->ipsec.tx.buflen, (unsigned long)padding));
	if (padding == pad_blksz)
		padding = 0;

	for (i = 1; i <= padding; i++) {
		s->ipsec.tx.buf[s->ipsec.tx.buflen] = i;
		s->ipsec.tx.buflen++;
	}

	/* Add trailing padlen and next_header */
	s->ipsec.tx.buf[s->ipsec.tx.buflen++] = padding;
	s->ipsec.tx.buf[s->ipsec.tx.buflen++] = IPPROTO_IPIP;

	cleartext = s->ipsec.tx.buf + s->ipsec.tx.var_header_size + s->ipsec.tx.bufpayload;
	cleartextlen = s->ipsec.tx.buflen - s->ipsec.tx.var_header_size - s->ipsec.tx.bufpayload;

	eh = (esp_encap_header_t *) (s->ipsec.tx.buf + s->ipsec.tx.bufpayload);
	eh->spi = s->ipsec.tx.spi;
	eh->seq_id = htonl(s->ipsec.tx.seq_id++);

	/* Copy initialization vector in packet */
	iv = (unsigned char *)(eh + 1);
	gcry_create_nonce(iv, s->ipsec.iv_len);
	hex_dump("iv", iv, s->ipsec.iv_len, NULL);

	hex_dump("sending ESP packet (before crypt)", s->ipsec.tx.buf, s->ipsec.tx.buflen, NULL);

	if (s->ipsec.cry_algo) {
		gcry_cipher_setiv(s->ipsec.tx.cry_ctx, iv, s->ipsec.iv_len);
		gcry_cipher_encrypt(s->ipsec.tx.cry_ctx, cleartext, cleartextlen, NULL, 0);
	}

	hex_dump("sending ESP packet (after crypt)", s->ipsec.tx.buf, s->ipsec.tx.buflen, NULL);

	/* Handle optional authentication field */
	if (s->ipsec.md_algo) {
		hmac_compute(s->ipsec.md_algo,
			s->ipsec.tx.buf + s->ipsec.tx.bufpayload,
			s->ipsec.tx.var_header_size + cleartextlen,
			s->ipsec.tx.buf + s->ipsec.tx.bufpayload
			+ s->ipsec.tx.var_header_size + cleartextlen,
			1, s->ipsec.tx.key_md, s->ipsec.md_len);
		s->ipsec.tx.buflen += 12; /*gcry_md_get_algo_dlen(md_algo); see RFC .. only use 96 bit */
		hex_dump("sending ESP packet (after ah)", s->ipsec.tx.buf, s->ipsec.tx.buflen, NULL);
	}
}

/*
 * Encapsulate a packet in IP ESP and send to the peer.
 * "buf" should have exactly MAX_HEADER free bytes at its beginning
 * to account for encapsulation data (not counted in "size").
 */
static void encap_esp_send_peer(struct sa_block *s, unsigned char *buf, unsigned int bufsize)
{
	ssize_t sent;
	struct ip *tip, ip;
	struct sockaddr_in dstaddr;

	buf += MAX_HEADER;

	/* Keep a pointer to the old IP header */
	tip = (struct ip *)buf;

	s->ipsec.tx.buf = buf;
	s->ipsec.tx.buflen = bufsize;

	/* Prepend our encapsulation header and new IP header */
	s->ipsec.tx.var_header_size = (s->ipsec.em->fixed_header_size + s->ipsec.iv_len);

	s->ipsec.tx.buf -= sizeof(struct ip) + s->ipsec.tx.var_header_size;
	s->ipsec.tx.buflen += sizeof(struct ip) + s->ipsec.tx.var_header_size;

	s->ipsec.tx.bufpayload = sizeof(struct ip);

	/* Fill non-mutable fields */
	ip.ip_v = IPVERSION;
	ip.ip_hl = 5;
	/*gcry_md_get_algo_dlen(md_algo); see RFC .. only use 96 bit */
	ip.ip_id = htons(s->ipsec.ip_id++);
	ip.ip_p = IPPROTO_ESP;
	ip.ip_src = s->src;
	ip.ip_dst = s->dst;

	/* Fill mutable fields */
	ip.ip_tos = (bufsize < sizeof(struct ip)) ? 0 : tip->ip_tos;
	ip.ip_off = 0;
	ip.ip_ttl = IPDEFTTL;
	ip.ip_sum = 0;

	encap_esp_encapsulate(s);

	ip.ip_len = s->ipsec.tx.buflen;
#ifdef NEED_IPLEN_FIX
	ip.ip_len = htons(ip.ip_len);
#endif
	ip.ip_sum = in_cksum((u_short *) s->ipsec.tx.buf, sizeof(struct ip));

	memcpy(s->ipsec.tx.buf, &ip, sizeof ip);

	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr = s->dst;
	dstaddr.sin_port = 0;
	sent = sendto(s->esp_fd, s->ipsec.tx.buf, s->ipsec.tx.buflen, 0, (struct sockaddr *)&dstaddr, sizeof(struct sockaddr_in));
	if (sent == -1) {
		syslog(LOG_ERR, "esp sendto: %m");
		return;
	}
	if (sent != s->ipsec.tx.buflen)
		syslog(LOG_ALERT, "esp truncated out (%lld out of %d)", (long long)sent, s->ipsec.tx.buflen);
}

/*
 * Encapsulate a packet in UDP ESP and send to the peer.
 * "buf" should have exactly MAX_HEADER free bytes at its beginning
 * to account for encapsulation data (not counted in "size").
 */
static void encap_udp_send_peer(struct sa_block *s, unsigned char *buf, unsigned int bufsize)
{
	ssize_t sent;
	
	buf += MAX_HEADER;
	
	s->ipsec.tx.buf = buf;
	s->ipsec.tx.buflen = bufsize;
	
	/* Prepend our encapsulation header and new IP header */
	s->ipsec.tx.var_header_size = (s->ipsec.em->fixed_header_size + s->ipsec.iv_len);
	
	s->ipsec.tx.buf -= s->ipsec.tx.var_header_size;
	s->ipsec.tx.buflen += s->ipsec.tx.var_header_size;
	
	s->ipsec.tx.bufpayload = 0;
	
	encap_esp_encapsulate(s);
	
	if (s->ipsec.natt_active_mode == NATT_ACTIVE_DRAFT_OLD) {
		s->ipsec.tx.buf -= 8;
		s->ipsec.tx.buflen += 8;
		memset(s->ipsec.tx.buf, 0, 8);
	}
	
	sent = send(s->esp_fd, s->ipsec.tx.buf, s->ipsec.tx.buflen, 0);
	if (sent == -1) {
		syslog(LOG_ERR, "udp sendto: %m");
		return;
	}
	if (sent != s->ipsec.tx.buflen)
		syslog(LOG_ALERT, "udp truncated out (%lld out of %d)",
			(long long)sent, s->ipsec.tx.buflen);
}

static int encap_esp_recv_peer(struct sa_block *s)
{
	int len, i;
	size_t blksz;
	unsigned char padlen, next_header;
	unsigned char *pad;
	unsigned char *iv;
	struct esp_encap_header *eh;

	eh = (struct esp_encap_header *)(s->ipsec.rx.buf + s->ipsec.rx.bufpayload);
	s->ipsec.rx.var_header_size = s->ipsec.iv_len;
	iv = s->ipsec.rx.buf + s->ipsec.rx.bufpayload + s->ipsec.em->fixed_header_size;

	len = s->ipsec.rx.buflen - s->ipsec.rx.bufpayload - s->ipsec.em->fixed_header_size - s->ipsec.rx.var_header_size;

	if (len < 0) {
		syslog(LOG_ALERT, "Packet too short");
		return -1;
	}

	/* Handle optional authentication field */
	if (s->ipsec.md_algo) {
		len -= 12; /*gcry_md_get_algo_dlen(peer->local_sa->md_algo); */
		s->ipsec.rx.buflen -= 12;
		if (hmac_compute(s->ipsec.md_algo,
				s->ipsec.rx.buf + s->ipsec.rx.bufpayload,
				s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size + len,
				s->ipsec.rx.buf + s->ipsec.rx.bufpayload
				+ s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size + len,
				0,
				s->ipsec.rx.key_md,
				s->ipsec.md_len) != 0) {
			syslog(LOG_ALERT, "HMAC mismatch in ESP mode");
			return -1;
		}
	}

	blksz = s->ipsec.blk_len;
	if ((len % blksz) != 0) {
		syslog(LOG_ALERT,
			"payload len %d not a multiple of algorithm block size %lu", len,
			(unsigned long)blksz);
		return -1;
	}
	
	hex_dump("receiving ESP packet (before decrypt)",
		&s->ipsec.rx.buf[s->ipsec.rx.bufpayload + s->ipsec.em->fixed_header_size +
			 s->ipsec.rx.var_header_size], len, NULL);

	if (s->ipsec.cry_algo) {
		unsigned char *data;

		data = (s->ipsec.rx.buf + s->ipsec.rx.bufpayload
			+ s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size);
		gcry_cipher_setiv(s->ipsec.rx.cry_ctx, iv, s->ipsec.iv_len);
		gcry_cipher_decrypt(s->ipsec.rx.cry_ctx, data, len, NULL, 0);
	}

	hex_dump("receiving ESP packet (after decrypt)",
		&s->ipsec.rx.buf[s->ipsec.rx.bufpayload + s->ipsec.em->fixed_header_size +
			s->ipsec.rx.var_header_size], len, NULL);
	
	padlen = s->ipsec.rx.buf[s->ipsec.rx.bufpayload
		+ s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size + len - 2];
	next_header = s->ipsec.rx.buf[s->ipsec.rx.bufpayload
		+ s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size + len - 1];

	if (padlen + 2 > len) {
		syslog(LOG_ALERT, "Inconsistent padlen");
		return -1;
	}
	if (next_header != IPPROTO_IPIP) {
		syslog(LOG_ALERT, "Inconsistent next_header %d", next_header);
		return -1;
	}
	DEBUG(3, printf("pad len: %d, next_header: %d\n", padlen, next_header));

	len -= padlen + 2;
	s->ipsec.rx.buflen -= padlen + 2;

	/* Check padding */
	pad = s->ipsec.rx.buf + s->ipsec.rx.bufpayload
		+ s->ipsec.em->fixed_header_size + s->ipsec.rx.var_header_size + len;
	for (i = 1; i <= padlen; i++) {
		if (*pad != i) {
			syslog(LOG_ALERT, "Bad padding");
			return -1;
		}
		pad++;
	}

	return 0;
}

static void encap_esp_new(struct encap_method *encap)
{
	encap->recv = encap_rawip_recv;
	encap->send_peer = encap_esp_send_peer;
	encap->recv_peer = encap_esp_recv_peer;
	encap->fixed_header_size = sizeof(esp_encap_header_t);
}

static void encap_udp_new(struct encap_method *encap)
{
	encap->recv = encap_udp_recv;
	encap->send_peer = encap_udp_send_peer;
	encap->recv_peer = encap_esp_recv_peer;
	encap->fixed_header_size = sizeof(esp_encap_header_t);
}

/*
 * Process ARP 
 * Return 1 if packet has been processed, 0 otherwise
 */
static int process_arp(struct sa_block *s, uint8_t *frame)
{
#ifndef __sun__
	int frame_size;
	uint8_t tmp[4];
	struct ether_header *eth = (struct ether_header *) frame;
	struct ether_arp *arp = (struct ether_arp *) (frame + ETH_HLEN);
	
	if (ntohs(eth->ether_type) != ETHERTYPE_ARP) {
		return 0;
	}
	
	if (ntohs(arp->arp_hrd) != ARPHRD_ETHER ||
		ntohs(arp->arp_pro) != 0x800 ||
		arp->arp_hln != ETH_ALEN ||
		arp->arp_pln != 4 ||
		ntohs(arp->arp_op) != ARPOP_REQUEST ||
		!memcmp(arp->arp_spa, arp->arp_tpa, 4) ||
		memcmp(eth->ether_shost, s->tun_hwaddr, ETH_ALEN) ||
		!memcmp(arp->arp_tpa, s->our_address, 4)) {
		/* whatever .. just drop it */
		return 1;
	}
	
	/* send arp reply */
	
	memcpy(eth->ether_dhost, s->tun_hwaddr, ETH_ALEN);
	eth->ether_shost[0] ^= 0x80; /* Use a different MAC as source */
	
	memcpy(tmp, arp->arp_spa, 4);
	memcpy(arp->arp_spa, arp->arp_tpa, 4);
	memcpy(arp->arp_tpa, tmp, 4);
	
	memcpy(arp->arp_tha, s->tun_hwaddr, ETH_ALEN);
	arp->arp_sha[0] ^= 0x80; /* Use a different MAC as source */
	
	arp->arp_op = htons(ARPOP_REPLY);
	
	frame_size = ETH_HLEN + sizeof(struct ether_arp);
	tun_write(s->tun_fd, frame, frame_size);
	hex_dump("ARP reply", frame, frame_size, NULL);
	
	return 1;
#else
	s = 0;
	frame = 0;
	return 0;
#endif
}

/*
 * Process non-IP packets
 * Return 1 if packet has been processed, 0 otherwise
 */
static int process_non_ip(struct sa_block *s, uint8_t *frame)
{
	struct ether_header *eth = (struct ether_header *) frame;
	
	s = NULL; /* unused */
	
	if (ntohs(eth->ether_type) != ETHERTYPE_IP) {
		/* drop non-ip traffic */
		return 1;
	}
	
	return 0;
}

static void process_tun(struct sa_block *s)
{
	int pack;
	int size = MAX_PACKET;
	uint8_t *start = global_buffer_rx + MAX_HEADER;
	
	if (opt_if_mode == IF_MODE_TAP) {
		/* Make sure IP packet starts at buf + MAX_HEADER */
		start -= ETH_HLEN;
		size += ETH_HLEN;
	}
	
	/* Receive a packet from the tunnel interface */
	pack = tun_read(s->tun_fd, start, size);
	
	hex_dump("Rx pkt", start, pack, NULL);
	
	if (opt_if_mode == IF_MODE_TAP) {
		if (process_arp(s, start)) {
			return;
		}
		if (process_non_ip(s, start)) {
			return;
		}
		pack -= ETH_HLEN;
	}
	
	if (pack == -1) {
		syslog(LOG_ERR, "read: %m");
		return;
	}
	
	/* Don't access the contents of the buffer other than byte aligned.
	 * 12: Offset of ip source address in ip header,
	 *  4: Length of IP address */
	if (!memcmp(global_buffer_rx + MAX_HEADER + 12, &s->dst.s_addr, 4)) {
		syslog(LOG_ALERT, "routing loop to %s",
			inet_ntoa(s->dst));
		return;
	}
	
	/* Encapsulate and send to the other end of the tunnel */
	s->ipsec.life.tx += pack;
	s->ipsec.em->send_peer(s, global_buffer_rx, pack);
}

static void process_socket(struct sa_block *s)
{
	/* Receive a packet from a socket */
	int pack;
	uint8_t *start = global_buffer_tx;
	esp_encap_header_t *eh;

	if (opt_if_mode == IF_MODE_TAP) {
		start += ETH_HLEN;
	}
	
	pack = s->ipsec.em->recv(s, start, MAX_HEADER + MAX_PACKET);
	if (pack == -1)
		return;
	
	eh = (esp_encap_header_t *) (s->ipsec.rx.buf + s->ipsec.rx.bufpayload);
	if (eh->spi == 0) {
		process_late_ike(s, s->ipsec.rx.buf + s->ipsec.rx.bufpayload + 4 /* SPI-size */,
			s->ipsec.rx.buflen - s->ipsec.rx.bufpayload - 4);
		return;
	} else if (eh->spi != s->ipsec.rx.spi) {
		syslog(LOG_NOTICE, "unknown spi %#08x from peer", ntohl(eh->spi));
		return;
	}
	
	/* Check auth digest and/or decrypt */
	if (s->ipsec.em->recv_peer(s) != 0)
		return;
	
	if (encap_any_decap(s) == 0) {
		syslog(LOG_DEBUG, "received update probe from peer");
	} else {
		/* Send the decapsulated packet to the tunnel interface */
		s->ipsec.life.rx += s->ipsec.rx.buflen;
		tun_send_ip(s);
	}
}

#if defined(__CYGWIN__)
static void *tun_thread (void *arg)
{
	struct sa_block *s = (struct sa_block *) arg;
	
	while (!do_kill) {
		process_tun(s);
	}
	return NULL;
}
#endif

static void vpnc_main_loop(struct sa_block *s)
{
	fd_set rfds, refds;
	int nfds=0;
	int enable_keepalives;
	int timed_mode;
	ssize_t len;
	struct timeval select_timeout;
	struct timeval normal_timeout;
	time_t next_ike_keepalive=0;
	time_t next_ike_dpd=0;
#if defined(__CYGWIN__)
	pthread_t tid;
#endif
	
	/* non-esp marker, nat keepalive payload (0xFF) */
	uint8_t keepalive_v2[5] = { 0x00, 0x00, 0x00, 0x00, 0xFF };
	uint8_t keepalive_v1[1] = { 0xFF };
	uint8_t *keepalive;
	size_t keepalive_size;
	
	if (s->ipsec.natt_active_mode == NATT_ACTIVE_DRAFT_OLD) {
		keepalive = keepalive_v1;
		keepalive_size = sizeof(keepalive_v1);
	} else { /* active_mode is either RFC or CISCO_UDP */
		keepalive = keepalive_v2;
		keepalive_size = sizeof(keepalive_v2);
	}
	
	/* send keepalives if UDP encapsulation is enabled */
	enable_keepalives = (s->ipsec.encap_mode != IPSEC_ENCAP_TUNNEL);
	
	/* regular wakeups if keepalives on ike or dpd active */
	timed_mode = ((enable_keepalives && s->ike_fd != s->esp_fd) || s->ike.do_dpd);
	
	FD_ZERO(&rfds);
	
#if !defined(__CYGWIN__)
	FD_SET(s->tun_fd, &rfds);
	nfds = MAX(nfds, s->tun_fd +1);
#endif
	
	FD_SET(s->esp_fd, &rfds);
	nfds = MAX(nfds, s->esp_fd +1);
	
	if (s->ike_fd != s->esp_fd) {
		FD_SET(s->ike_fd, &rfds);
		nfds = MAX(nfds, s->ike_fd +1);
	}
	
#if defined(__CYGWIN__)
	if (pthread_create(&tid, NULL, tun_thread, s)) {
	        syslog(LOG_ERR, "Cannot create tun thread!\n");
		return;
	}
#endif
	
	normal_timeout.tv_sec = 86400;
	normal_timeout.tv_usec = 0;
	
	if (s->ike.do_dpd) {
		/* send initial dpd request */
		next_ike_dpd = time(NULL) + s->ike.dpd_idle;
		dpd_ike(s);
		normal_timeout.tv_sec = s->ike.dpd_idle;
		normal_timeout.tv_usec = 0;
	}
	
	if (enable_keepalives) {
		normal_timeout.tv_sec = 9;
		normal_timeout.tv_usec = 500000;

		if (s->ike_fd != s->esp_fd) {
			/* send initial nat ike keepalive packet */
			next_ike_keepalive = time(NULL) + 9;
			keepalive_ike(s);
		}
	}
	
	select_timeout = normal_timeout;
	
	while (!do_kill) {
		int presult;
		
		do {
			struct timeval *tvp = NULL;
			FD_COPY(&rfds, &refds);
			if (s->ike.do_dpd || enable_keepalives)
				tvp = &select_timeout;
			presult = select(nfds, &refds, NULL, NULL, tvp);
			if (presult == 0 && (s->ike.do_dpd || enable_keepalives)) {
				/* reset to max timeout */
				select_timeout = normal_timeout;
				if (enable_keepalives) {
					if (s->ike_fd != s->esp_fd) {
						/* send nat ike keepalive packet */
						next_ike_keepalive = time(NULL) + 9;
						keepalive_ike(s);
					}
					/* send nat keepalive packet */
					if (send(s->esp_fd, keepalive, keepalive_size, 0) == -1) {
						syslog(LOG_ERR, "keepalive sendto: %m");
					}
				}
				if (s->ike.do_dpd) {
					time_t now = time(NULL);
					if (s->ike.dpd_seqno != s->ike.dpd_seqno_ack) {
						/* Wake up more often for dpd attempts */
						select_timeout.tv_sec = 5;
						select_timeout.tv_usec = 0;
						dpd_ike(s);
						next_ike_dpd = now + s->ike.dpd_idle;
					}
					else if (now >= next_ike_dpd) {
						dpd_ike(s);
						next_ike_dpd = now + s->ike.dpd_idle;
					}
				}
			}
			DEBUG(2,printf("lifetime status: %ld of %u seconds used, %u|%u of %u kbytes used\n",
				time(NULL) - s->ipsec.life.start,
				s->ipsec.life.seconds,
				s->ipsec.life.rx/1024,
				s->ipsec.life.tx/1024,
				s->ipsec.life.kbytes));
		} while ((presult == 0 || (presult == -1 && errno == EINTR)) && !do_kill);
		if (presult == -1) {
			syslog(LOG_ERR, "select: %m");
			continue;
		}
		
#if !defined(__CYGWIN__)
		if (FD_ISSET(s->tun_fd, &refds)) {
			process_tun(s);
		}
#endif
		
		if (FD_ISSET(s->esp_fd, &refds) ) {
			process_socket(s);
		}
		
		if (s->ike_fd != s->esp_fd && FD_ISSET(s->ike_fd, &refds) ) {
			DEBUG(3,printf("received something on ike fd..\n"));
			len = recv(s->ike_fd, global_buffer_tx, MAX_HEADER + MAX_PACKET, 0);
			process_late_ike(s, global_buffer_tx, len);
		}

		if (timed_mode) {
			time_t now = time(NULL);
			time_t next_up = now + 86400;
			if (enable_keepalives) {
				/* never wait more than 9 seconds for a UDP keepalive */
				next_up = now + 9;
				if (s->ike_fd != s->esp_fd) {
					if (now >= next_ike_keepalive) {
						/* send nat ike keepalive packet now */
						next_ike_keepalive = now + 9;
						keepalive_ike(s);
						select_timeout = normal_timeout;
					}
					if (next_ike_keepalive < next_up)
						next_up = next_ike_keepalive;
				}
			}
			if (s->ike.do_dpd) {
				if (s->ike.dpd_seqno != s->ike.dpd_seqno_ack) {
					dpd_ike(s);
					next_ike_dpd = now + s->ike.dpd_idle;
					if (now + 5 < next_up)
						next_up = now + 5;
				}
				else if (now >= next_ike_dpd) {
					dpd_ike(s);
					next_ike_dpd = now + s->ike.dpd_idle;
				}
				if (next_ike_dpd < next_up)
					next_up = next_ike_dpd;
			}
			/* Reduce timeout so next activity happens on schedule */
			select_timeout.tv_sec = next_up - now;
			select_timeout.tv_usec = 0;
		}

	}
	
	switch (do_kill) {
		case -2:
			syslog(LOG_NOTICE, "connection terminated by dead peer detection");
			break;
		case -1:
			syslog(LOG_NOTICE, "connection terminated by peer");
			break;
		default:
			syslog(LOG_NOTICE, "terminated by signal: %d", do_kill);
			break;
	}
}

static void killit(int signum)
{
	do_kill = signum;
}

static void write_pidfile(const char *pidfile)
{
	FILE *pf;
	
	if (pidfile == NULL || pidfile[0] == '\0')
		return;
	
	pf = fopen(pidfile, "w");
	if (pf == NULL) {
		syslog(LOG_WARNING, "can't open pidfile %s for writing", pidfile);
		return;
	}
	
	fprintf(pf, "%d\n", (int)getpid());
	fclose(pf);
}

void vpnc_doit(struct sa_block *s)
{
	struct sigaction act;
	struct encap_method meth;
	
	const char *pidfile = config[CONFIG_PID_FILE];
	
	switch (s->ipsec.encap_mode) {
		case IPSEC_ENCAP_TUNNEL:
			encap_esp_new(&meth);
			gcry_create_nonce(&s->ipsec.ip_id, sizeof(uint16_t));
			break;
		case IPSEC_ENCAP_UDP_TUNNEL:
		case IPSEC_ENCAP_UDP_TUNNEL_OLD:
			encap_udp_new(&meth);
			break;
		default:
			abort();
	}
	s->ipsec.em = &meth;
	
	s->ipsec.rx.key_cry = s->ipsec.rx.key;
	hex_dump("rx.key_cry", s->ipsec.rx.key_cry, s->ipsec.key_len, NULL);
	
	s->ipsec.rx.key_md = s->ipsec.rx.key + s->ipsec.key_len;
	hex_dump("rx.key_md", s->ipsec.rx.key_md, s->ipsec.md_len, NULL);
	
	if (s->ipsec.cry_algo) {
		gcry_cipher_open(&s->ipsec.rx.cry_ctx, s->ipsec.cry_algo, GCRY_CIPHER_MODE_CBC, 0);
		gcry_cipher_setkey(s->ipsec.rx.cry_ctx, s->ipsec.rx.key_cry, s->ipsec.key_len);
	} else {
		s->ipsec.rx.cry_ctx = NULL;
	}
	
	s->ipsec.tx.key_cry = s->ipsec.tx.key;
	hex_dump("tx.key_cry", s->ipsec.tx.key_cry, s->ipsec.key_len, NULL);
	
	s->ipsec.tx.key_md = s->ipsec.tx.key + s->ipsec.key_len;
	hex_dump("tx.key_md", s->ipsec.tx.key_md, s->ipsec.md_len, NULL);
	
	if (s->ipsec.cry_algo) {
		gcry_cipher_open(&s->ipsec.tx.cry_ctx, s->ipsec.cry_algo, GCRY_CIPHER_MODE_CBC, 0);
		gcry_cipher_setkey(s->ipsec.tx.cry_ctx, s->ipsec.tx.key_cry, s->ipsec.key_len);
	} else {
		s->ipsec.tx.cry_ctx = NULL;
	}
	
	DEBUG(2, printf("remote -> local spi: %#08x\n", ntohl(s->ipsec.rx.spi)));
	DEBUG(2, printf("local -> remote spi: %#08x\n", ntohl(s->ipsec.tx.spi)));
	
	do_kill = 0;
	
	sigaction(SIGHUP, NULL, &act);
	if (act.sa_handler == SIG_DFL)
		signal(SIGHUP, killit);
	
	signal(SIGINT, killit);
	signal(SIGTERM, killit);
	
	chdir("/");
	
	if (!opt_nd) {
		pid_t pid;
		if ((pid = fork()) < 0) {
			fprintf(stderr, "Warning, could not fork the child process!\n");
		} else if (pid == 0) {
			close(0); open("/dev/null", O_RDONLY, 0666);
			close(1); open("/dev/null", O_WRONLY, 0666);
			close(2); open("/dev/null", O_WRONLY, 0666);
			setsid();
		} else {
			printf("VPNC started in background (pid: %d)...\n", (int)pid);
			exit(0);
		}
	} else {
		printf("VPNC started in foreground...\n");
	}
	openlog("vpnc", LOG_PID | LOG_PERROR, LOG_DAEMON);
	write_pidfile(pidfile);
	
	vpnc_main_loop(s);
	
	if (pidfile)
		unlink(pidfile); /* ignore errors */
}
