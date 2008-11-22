/* IPSec VPN client compatible with Cisco equipment.
   Copyright (C) 2002      Geoffrey Keating
   Copyright (C) 2003-2005 Maurice Massar
   Copyright (C) 2004      Tomas Mraz
   Copyright (C) 2004      Martin von Gagern

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

   $Id: vpnc.c 371 2008-11-19 20:55:28Z Joerg Mayer $
*/

#define _GNU_SOURCE
#include <assert.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>

#include <gcrypt.h>

#ifdef OPENSSL_GPL_VIOLATION
/* OpenSSL */
#include <openssl/x509.h>
#include <openssl/err.h>
#endif /* OPENSSL_GPL_VIOLATION */

#include "sysdep.h"
#include "config.h"
#include "isakmp-pkt.h"
#include "math_group.h"
#include "dh.h"
#include "vpnc.h"
#include "tunip.h"
#include "supp.h"

#if defined(__CYGWIN__)
	GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif

#define ISAKMP_PORT (500)
#define ISAKMP_PORT_NATT (4500)

const unsigned char VID_XAUTH[] = { /* "draft-ietf-ipsra-isakmp-xauth-06.txt"/8 */
	0x09, 0x00, 0x26, 0x89, 0xDF, 0xD6, 0xB7, 0x12
};
const unsigned char VID_DPD[] = { /* Dead Peer Detection, RFC 3706 */
	0xAF, 0xCA, 0xD7, 0x13, 0x68, 0xA1, 0xF1, 0xC9,
	0x6B, 0x86, 0x96, 0xFC, 0x77, 0x57, 0x01, 0x00
};
const unsigned char VID_UNITY[] = { /* "CISCO-UNITY"/14 + major + minor */
	0x12, 0xF5, 0xF2, 0x8C, 0x45, 0x71, 0x68, 0xA9,
	0x70, 0x2D, 0x9F, 0xE2, 0x74, 0xCC, 0x01, 0x00
};
const unsigned char VID_UNKNOWN[] = {
	0x12, 0x6E, 0x1F, 0x57, 0x72, 0x91, 0x15, 0x3B,
	0x20, 0x48, 0x5F, 0x7F, 0x15, 0x5B, 0x4B, 0xC8
};
const unsigned char VID_NATT_00[] = { /* "draft-ietf-ipsec-nat-t-ike-00" */
	0x44, 0x85, 0x15, 0x2d, 0x18, 0xb6, 0xbb, 0xcd,
	0x0b, 0xe8, 0xa8, 0x46, 0x95, 0x79, 0xdd, 0xcc
};
const unsigned char VID_NATT_01[] = { /* "draft-ietf-ipsec-nat-t-ike-01" */
	0x16, 0xf6, 0xca, 0x16, 0xe4, 0xa4, 0x06, 0x6d,
	0x83, 0x82, 0x1a, 0x0f, 0x0a, 0xea, 0xa8, 0x62
};
const unsigned char VID_NATT_02[] = { /* "draft-ietf-ipsec-nat-t-ike-02" */
	0xcd, 0x60, 0x46, 0x43, 0x35, 0xdf, 0x21, 0xf8,
	0x7c, 0xfd, 0xb2, 0xfc, 0x68, 0xb6, 0xa4, 0x48
};
const unsigned char VID_NATT_02N[] = { /* "draft-ietf-ipsec-nat-t-ike-02\n" */
	0x90, 0xCB, 0x80, 0x91, 0x3E, 0xBB, 0x69, 0x6E,
	0x08, 0x63, 0x81, 0xB5, 0xEC, 0x42, 0x7B, 0x1F
};
const unsigned char VID_NATT_RFC[] = { /* "RFC 3947" */
	0x4A, 0x13, 0x1C, 0x81, 0x07, 0x03, 0x58, 0x45,
	0x5C, 0x57, 0x28, 0xF2, 0x0E, 0x95, 0x45, 0x2F
};
const unsigned char VID_DWR[] = { /* DWR: Delete with reason */
	0x2D, 0x79, 0x22, 0xC6, 0xB3, 0x01, 0xD9, 0xB0,
	0xE1, 0x34, 0x27, 0x39, 0xE9, 0xCF, 0xBB, 0xD5
};
/* Cisco Unknown1:
 *const unsigned char VID_CISCO_UNKNOWN_1[] = {
 *	1f07f70eaa6514d3b0fa96542a500407
 *};
 */

const unsigned char VID_CISCO_FRAG[] = { /* "FRAGMENTATION" */
	0x40, 0x48, 0xB7, 0xD5, 0x6E, 0xBC, 0xE8, 0x85,
	0x25, 0xE7, 0xDE, 0x7F, 0x00, 0xD6, 0xC2, 0xD3,
	0x80, 0x00, 0x00, 0x00
};
const unsigned char VID_NETSCREEN_15[] = { /* netscreen 15 */
	0x16, 0x6f, 0x93, 0x2d, 0x55, 0xeb, 0x64, 0xd8,
	0xe4, 0xdf, 0x4f, 0xd3, 0x7e, 0x23, 0x13, 0xf0,
	0xd0, 0xfd, 0x84, 0x51, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};
const unsigned char VID_HEARTBEAT_NOTIFY[] = { /* Heartbeat Notify */
	0x48, 0x65, 0x61, 0x72, 0x74, 0x42, 0x65, 0x61,
	0x74, 0x5f, 0x4e, 0x6f, 0x74, 0x69, 0x66, 0x79,
	0x38, 0x6b, 0x01, 0x00
};
const unsigned char VID_NORTEL_CONT[] = { /* BNES: Bay Networks Enterprise Switch + version/id of some kind */
	0x42, 0x4e, 0x45, 0x53, 0x00, 0x00, 0x00, 0x0a
};

const unsigned char FW_UNKNOWN_TYPEINFO[] = {
	0x80, 0x01, 0x00, 0x01, 0x80, 0x02, 0x00, 0x01,
	0x80, 0x03, 0x00, 0x02
};

struct vid_element {
	const unsigned char* valueptr;
	const uint16_t length;
	const char* descr;
};

const struct vid_element vid_list[] = {
	{ VID_XAUTH,		sizeof(VID_XAUTH),	"Xauth" },	
	{ VID_DPD,		sizeof(VID_DPD),	"DPD" },	
	{ VID_UNITY,		sizeof(VID_UNITY),	"Cisco Unity" },	
	{ VID_NATT_00,		sizeof(VID_NATT_00),	"Nat-T 00" },	
	{ VID_NATT_01,		sizeof(VID_NATT_01),	"Nat-T 01" },	
	{ VID_NATT_02,		sizeof(VID_NATT_02),	"Nat-T 02" },	
	{ VID_NATT_02N,		sizeof(VID_NATT_02N),	"Nat-T 02N" },	
	{ VID_NATT_RFC,		sizeof(VID_NATT_RFC),	"Nat-T RFC" },	
	{ VID_DWR,		sizeof(VID_DWR),	"Delete With Reason" },	
	{ VID_CISCO_FRAG,	sizeof(VID_CISCO_FRAG),	"Cisco Fragmentation" },
	{ VID_NETSCREEN_15,	sizeof(VID_NETSCREEN_15),	"Netscreen 15" },
	{ VID_NORTEL_CONT,	sizeof(VID_NORTEL_CONT),	"Nortel Contivity" },
	{ VID_HEARTBEAT_NOTIFY,	sizeof(VID_HEARTBEAT_NOTIFY),	"Heartbeat Notify" },

	{ NULL, 0, NULL }
};

/* What are DWR-Code and DWR-Text ? */

static uint8_t r_packet[8192];
static ssize_t r_length;

void print_vid(const unsigned char *vid, uint16_t len) {
	
	int vid_index = 0;

	if (opt_debug < 2)
		return;

	while (vid_list[vid_index].length) {
		if (len == vid_list[vid_index].length &&
			memcmp(vid_list[vid_index].valueptr, vid, len) == 0) {
			printf("   (%s)\n", vid_list[vid_index].descr);
			return;
		}
		vid_index++;
	}
	printf("   (unknown)\n");
}

static __inline__ int min(int a, int b)
{
	return (a < b) ? a : b;
}

static void addenv(const void *name, const char *value)
{
	char *strbuf = NULL, *oldval;

	oldval = getenv(name);
	if (oldval != NULL) {
		strbuf = xallocc(strlen(oldval) + 1 + strlen(value) + 1);
		strcat(strbuf, oldval);
		strcat(strbuf, " ");
		strcat(strbuf, value);
	}

	setenv(name, strbuf ? strbuf : value, 1);

	free(strbuf);
}

static void addenv_ipv4(const void *name, uint8_t * data)
{
	addenv(name, inet_ntoa(*((struct in_addr *)data)));
}

static int make_socket(struct sa_block *s, uint16_t src_port, uint16_t dst_port)
{
	int sock;
	struct sockaddr_in name;
	socklen_t len = sizeof(name);

	/* create the socket */
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		error(1, errno, "making socket");

#ifdef FD_CLOEXEC
	/* do not pass socket to vpnc-script, etc. */
	fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif

	/* give the socket a name */
	name.sin_family = AF_INET;
	name.sin_addr = s->opt_src_ip;
	name.sin_port = htons(src_port);
	if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
		error(1, errno, "Error binding to source port. Try '--local-port 0'\nFailed to bind to %s:%d", inet_ntoa(s->opt_src_ip), src_port);

	/* connect the socket */
	name.sin_family = AF_INET;
	name.sin_addr = s->dst;
	name.sin_port = htons(dst_port);
	if (connect(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
		error(1, errno, "connecting to port %d", ntohs(dst_port));

	/* who am I */
	if (getsockname(sock, (struct sockaddr *)&name, &len) < 0)
		error(1, errno, "reading local address from socket %d", sock);
	s->src = name.sin_addr;
	
	return sock;
}

static void cleanup(struct sa_block *s) {
	if (s->ike_fd != 0) {
		close(s->ike_fd);
		s->ike_fd = 0;
	}
	if (s->esp_fd != 0) {
		close(s->esp_fd);
		s->esp_fd = 0;
	}
	if (s->ike.resend_hash) {
		free(s->ike.resend_hash);
		s->ike.resend_hash = NULL;
	}
	if (s->ike.skeyid_d) {
		free(s->ike.skeyid_d);
		s->ike.skeyid_d = NULL;
	}
	if (s->ike.skeyid_a) {
		free(s->ike.skeyid_a);
		s->ike.skeyid_a = NULL;
	}
	if (s->ike.initial_iv) {
		free(s->ike.initial_iv);
		s->ike.initial_iv = NULL;
	}
	if (s->ike.current_iv) {
		free(s->ike.current_iv);
		s->ike.current_iv = NULL;
	}
	if (s->ike.key) {
		free(s->ike.key);
		s->ike.key = NULL;
	}
	if (s->ipsec.rx.key) {
		free(s->ipsec.rx.key);
		s->ipsec.rx.key = NULL;
	}
	if (s->ipsec.tx.key) {
		free(s->ipsec.tx.key);
		s->ipsec.tx.key = NULL;
	}
	if (s->ipsec.rx.cry_ctx) {
		gcry_cipher_close(s->ipsec.rx.cry_ctx);
		s->ipsec.rx.cry_ctx = NULL;
	}
	if (s->ipsec.tx.cry_ctx) {
		gcry_cipher_close(s->ipsec.tx.cry_ctx);
		s->ipsec.tx.cry_ctx = NULL;
	}
}

static void init_sockaddr(struct in_addr *dst, const char *hostname)
{
	struct hostent *hostinfo;

	if (inet_aton(hostname, dst) == 0) {
		hostinfo = gethostbyname(hostname);
		if (hostinfo == NULL)
			error(1, 0, "unknown host `%s'\n", hostname);
		*dst = *(struct in_addr *)hostinfo->h_addr;
	}
}

static void init_netaddr(struct in_addr *net, const char *string)
{
	char *p;

	if ((p = strchr(string, '/')) != NULL) {
		char *host = xallocc(p - string + 1);
		memcpy(host, string, p - string + 1);
		host[p - string] = '\0';
		init_sockaddr((struct in_addr *)net, host);
		free(host);
		if (strchr(p + 1, '.') != NULL)
			init_sockaddr(net + 1, p + 1);
		else {
			int bits = atoi(p + 1);
			unsigned long mask = (1 << bits) - 1;
			memcpy((char *)(net + 1), (char *)&mask, 4);
		}
	} else {
		memset((char *)net, 0, 8);
	}
}

static void setup_tunnel(struct sa_block *s)
{
	setenv("reason", "pre-init", 1);
	system(config[CONFIG_SCRIPT]);
	
	if (config[CONFIG_IF_NAME])
		memcpy(s->tun_name, config[CONFIG_IF_NAME], strlen(config[CONFIG_IF_NAME]));

	s->tun_fd = tun_open(s->tun_name, opt_if_mode);
	DEBUG(2, printf("using interface %s\n", s->tun_name));
	setenv("TUNDEV", s->tun_name, 1);

	if (s->tun_fd == -1)
		error(1, errno, "can't initialise tunnel interface");
#ifdef FD_CLOEXEC
	/* do not pass socket to vpnc-script, etc. */
	fcntl(s->tun_fd, F_SETFD, FD_CLOEXEC);
#endif
	
	if (opt_if_mode == IF_MODE_TAP) {
		if (tun_get_hwaddr(s->tun_fd, s->tun_name, s->tun_hwaddr) < 0) {
			error(1, errno, "can't get tunnel HW address");
		}
		hex_dump("interface HW addr", s->tun_hwaddr, ETH_ALEN, NULL);
	}
}

static void config_tunnel(struct sa_block *s)
{
	setenv("VPNGATEWAY", inet_ntoa(s->dst), 1);
	setenv("reason", "connect", 1);
	system(config[CONFIG_SCRIPT]);
}

static void close_tunnel(struct sa_block *s)
{
	setenv("reason", "disconnect", 1);
	system(config[CONFIG_SCRIPT]);
	tun_close(s->tun_fd, s->tun_name);
}

static int recv_ignore_dup(struct sa_block *s, void *recvbuf, size_t recvbufsize)
{
	uint8_t *resend_check_hash;
	int recvsize, hash_len;

	recvsize = recv(s->ike_fd, recvbuf, recvbufsize, 0);
	if (recvsize < 0)
		error(1, errno, "receiving packet");
	if ((unsigned int)recvsize > recvbufsize)
		error(1, errno, "received packet too large for buffer");

	/* skip (not only) NAT-T draft-0 keepalives */
	if ( /* (s->ipsec.natt_active_mode == NATT_ACTIVE_DRAFT_OLD) && */
	    (recvsize == 1) && (*((u_char *)(recvbuf)) == 0xff))
	{
		if ((s->ipsec.natt_active_mode != NATT_ACTIVE_DRAFT_OLD))
		{
			DEBUG(2, printf("Received UDP NAT-Keepalive bug nat active mode incorrect: %d\n", s->ipsec.natt_active_mode));
		}
		return -1;
	}
	
	hash_len = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
	resend_check_hash = malloc(hash_len);
	gcry_md_hash_buffer(GCRY_MD_SHA1, resend_check_hash, recvbuf, recvsize);
	if (s->ike.resend_hash && memcmp(s->ike.resend_hash, resend_check_hash, hash_len) == 0) {
		free(resend_check_hash);
		return -1;
	}
	if (!s->ike.resend_hash) {
		s->ike.resend_hash = resend_check_hash;
	} else {
		memcpy(s->ike.resend_hash, resend_check_hash, hash_len);
		free(resend_check_hash);
	}
	
	return recvsize;
}

/* Send TOSEND of size SENDSIZE to the socket.  Then wait for a new packet,
   resending TOSEND on timeout, and ignoring duplicate packets; the
   new packet is put in RECVBUF of size RECVBUFSIZE and the actual size
   of the new packet is returned.  */

static ssize_t sendrecv(struct sa_block *s, void *recvbuf, size_t recvbufsize, void *tosend, size_t sendsize, int sendonly)
{
	struct pollfd pfd;
	int tries = 0;
	int recvsize = -1;
	time_t start = time(NULL);
	time_t end = 0;
	void *realtosend;

	pfd.fd = s->ike_fd;
	pfd.events = POLLIN;
	tries = 0;

	if ((s->ipsec.natt_active_mode == NATT_ACTIVE_RFC) && (tosend != NULL)) {
		DEBUG(2, printf("NAT-T mode, adding non-esp marker\n"));
		realtosend = xallocc(sendsize+4);
		memmove((char*)realtosend+4, tosend, sendsize);
		sendsize += 4;
	} else {
		realtosend = tosend;
	}

	for (;;) {
		int pollresult;

		if (realtosend != NULL)
			if (write(s->ike_fd, realtosend, sendsize) != (int)sendsize)
				error(1, errno, "can't send packet");
		if (sendonly)
			break;
		
		do {
			pollresult = poll(&pfd, 1, s->ike.timeout << tries);
		} while (pollresult == -1 && errno == EINTR);
		
		if (pollresult == -1)
			error(1, errno, "can't poll socket");
		if (pollresult != 0) {
			recvsize = recv_ignore_dup(s, recvbuf, recvbufsize);
			end = time(NULL);
			if (recvsize != -1)
				break;
			continue;
		}
		
		if (tries > 2)
			error(1, 0, "no response from target");
		tries++;
	}

	if (realtosend != tosend)
		free(realtosend);

	if (sendonly)
		return 0;

	if ((s->ipsec.natt_active_mode == NATT_ACTIVE_RFC)&&(recvsize > 4)) {
		recvsize -= 4; /* 4 bytes non-esp marker */
		memmove(recvbuf, (char *)recvbuf+4, recvsize);
	}

	DEBUGTOP(3, printf("\n receiving: <========================\n"));

	/* Wait at least 2s for a response or 4 times the time it took
	 * last time.  */
	if (start >= end)
		s->ike.timeout = 2000;
	else
		s->ike.timeout = 4000 * (end - start);

	return recvsize;
}

static int isakmp_crypt(struct sa_block *s, uint8_t * block, size_t blocklen, int enc)
{
	unsigned char *new_iv, *iv = NULL;
	int info_ex;
	gcry_cipher_hd_t cry_ctx;

	if (blocklen < ISAKMP_PAYLOAD_O || ((blocklen - ISAKMP_PAYLOAD_O) % s->ike.ivlen != 0))
		abort();

	if (!enc && (memcmp(block + ISAKMP_I_COOKIE_O, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH) != 0
		|| memcmp(block + ISAKMP_R_COOKIE_O, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH) != 0)) {
		DEBUG(2, printf("got paket with wrong cookies\n"));
		return ISAKMP_N_INVALID_COOKIE;
	}
	
	info_ex = block[ISAKMP_EXCHANGE_TYPE_O] == ISAKMP_EXCHANGE_INFORMATIONAL;
	
	if (memcmp(block + ISAKMP_MESSAGE_ID_O, s->ike.current_iv_msgid, 4) != 0) {
		gcry_md_hd_t md_ctx;

		gcry_md_open(&md_ctx, s->ike.md_algo, 0);
		gcry_md_write(md_ctx, s->ike.initial_iv, s->ike.ivlen);
		gcry_md_write(md_ctx, block + ISAKMP_MESSAGE_ID_O, 4);
		gcry_md_final(md_ctx);
		if (info_ex) {
			iv = xallocc(s->ike.ivlen);
			memcpy(iv, gcry_md_read(md_ctx, 0), s->ike.ivlen);
		} else {
			memcpy(s->ike.current_iv, gcry_md_read(md_ctx, 0), s->ike.ivlen);
			memcpy(s->ike.current_iv_msgid, block + ISAKMP_MESSAGE_ID_O, 4);
		}
		gcry_md_close(md_ctx);
	} else if (info_ex) {
		abort();
	}
	
	if (!info_ex) {
		iv = s->ike.current_iv;
	}

	new_iv = xallocc(s->ike.ivlen);
	gcry_cipher_open(&cry_ctx, s->ike.cry_algo, GCRY_CIPHER_MODE_CBC, 0);
	gcry_cipher_setkey(cry_ctx, s->ike.key, s->ike.keylen);
	gcry_cipher_setiv(cry_ctx, iv, s->ike.ivlen);
	if (!enc) {
		memcpy(new_iv, block + blocklen - s->ike.ivlen, s->ike.ivlen);
		gcry_cipher_decrypt(cry_ctx, block + ISAKMP_PAYLOAD_O, blocklen - ISAKMP_PAYLOAD_O,
			NULL, 0);
		if (!info_ex)
			memcpy(s->ike.current_iv, new_iv, s->ike.ivlen);
	} else {
		gcry_cipher_encrypt(cry_ctx, block + ISAKMP_PAYLOAD_O, blocklen - ISAKMP_PAYLOAD_O,
			NULL, 0);
		if (!info_ex)
			memcpy(s->ike.current_iv, block + blocklen - s->ike.ivlen, s->ike.ivlen);
	}
	gcry_cipher_close(cry_ctx);
	
	free(new_iv);
	if (info_ex)
		free(iv);
	
	return 0;
}

static uint16_t unpack_verify_phase2(struct sa_block *s, uint8_t * r_packet,
	size_t r_length, struct isakmp_packet **r_p, const uint8_t * nonce, size_t nonce_size)
{
	struct isakmp_packet *r;
	int reject = 0;
	
	*r_p = NULL;

	/* Some users report "payload ... not padded..." errors. It seems that they
	 * are harmless, so ignore and fix the symptom
	 */
	if (r_length < ISAKMP_PAYLOAD_O ||
	    ((r_length - ISAKMP_PAYLOAD_O) % s->ike.ivlen != 0)) {
		DEBUG(2, printf("payload too short or not padded: len=%lld, min=%d (ivlen=%lld)\n",
			(long long)r_length, ISAKMP_PAYLOAD_O, (long long)s->ike.ivlen));
		hex_dump("Payload", r_packet, r_length, NULL);
		if (r_length < ISAKMP_PAYLOAD_O ) {
			return ISAKMP_N_UNEQUAL_PAYLOAD_LENGTHS;
		}
		r_length -= (r_length - ISAKMP_PAYLOAD_O) % s->ike.ivlen;
	}

	reject = isakmp_crypt(s, r_packet, r_length, 0);
	if (reject != 0)
		return reject;

	s->ike.life.rx += r_length;
	
	{
		r = parse_isakmp_packet(r_packet, r_length, &reject);
		if (reject != 0) {
			if (r) free_isakmp_packet(r);
			return reject;
		}
	}

	/* Verify the basic stuff.  */
	if (r->flags != ISAKMP_FLAG_E) {
		free_isakmp_packet(r);
		return ISAKMP_N_INVALID_FLAGS;
	}

	{
		size_t sz, spos;
		gcry_md_hd_t hm;
		unsigned char *expected_hash;
		struct isakmp_payload *h = r->payload;

		if (h == NULL || h->type != ISAKMP_PAYLOAD_HASH || h->u.hash.length != s->ike.md_len) {
			free_isakmp_packet(r);
			return ISAKMP_N_INVALID_HASH_INFORMATION;
		}

		spos = (ISAKMP_PAYLOAD_O + (r_packet[ISAKMP_PAYLOAD_O + 2] << 8)
			+ r_packet[ISAKMP_PAYLOAD_O + 3]);

		/* Compute the real length based on the payload lengths.  */
		for (sz = spos; r_packet[sz] != 0; sz += r_packet[sz + 2] << 8 | r_packet[sz + 3]) ;
		sz += r_packet[sz + 2] << 8 | r_packet[sz + 3];

		gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
		gcry_md_setkey(hm, s->ike.skeyid_a, s->ike.md_len);
		gcry_md_write(hm, r_packet + ISAKMP_MESSAGE_ID_O, 4);
		if (nonce)
			gcry_md_write(hm, nonce, nonce_size);
		gcry_md_write(hm, r_packet + spos, sz - spos);
		gcry_md_final(hm);
		expected_hash = gcry_md_read(hm, 0);

		DEBUG(3,printf("hashlen: %lu\n", (unsigned long)s->ike.md_len));
		DEBUG(3,printf("u.hash.length: %d\n", h->u.hash.length));
		hex_dump("expected_hash", expected_hash, s->ike.md_len, NULL);
		hex_dump("h->u.hash.data", h->u.hash.data, s->ike.md_len, NULL);

		reject = 0;
		if (memcmp(h->u.hash.data, expected_hash, s->ike.md_len) != 0)
			reject = ISAKMP_N_AUTHENTICATION_FAILED;
		gcry_md_close(hm);
#if 0
		if (reject != 0) {
			free_isakmp_packet(r);
			return reject;
		}
#endif
	}
	*r_p = r;
	return 0;
}

static void
phase2_authpacket(struct sa_block *s, struct isakmp_payload *pl,
	uint8_t exchange_type, uint32_t msgid,
	uint8_t ** p_flat, size_t * p_size,
	uint8_t * nonce_i, int ni_len, uint8_t * nonce_r, int nr_len)
{
	struct isakmp_packet *p;
	uint8_t *pl_flat;
	size_t pl_size;
	gcry_md_hd_t hm;
	uint8_t msgid_sent[4];

	/* Build up the packet.  */
	p = new_isakmp_packet();
	memcpy(p->i_cookie, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
	memcpy(p->r_cookie, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
	p->flags = ISAKMP_FLAG_E;
	p->isakmp_version = ISAKMP_VERSION;
	p->exchange_type = exchange_type;
	p->message_id = msgid;
	p->payload = new_isakmp_payload(ISAKMP_PAYLOAD_HASH);
	p->payload->next = pl;
	p->payload->u.hash.length = s->ike.md_len;
	p->payload->u.hash.data = xallocc(s->ike.md_len);

	/* Set the MAC.  */
	gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
	gcry_md_setkey(hm, s->ike.skeyid_a, s->ike.md_len);

	if (pl == NULL) {
		DEBUG(3, printf("authing NULL package!\n"));
		gcry_md_write(hm, "" /* \0 */ , 1);
	}

	msgid_sent[0] = msgid >> 24;
	msgid_sent[1] = msgid >> 16;
	msgid_sent[2] = msgid >> 8;
	msgid_sent[3] = msgid;
	gcry_md_write(hm, msgid_sent, sizeof(msgid_sent));

	if (nonce_i != NULL)
		gcry_md_write(hm, nonce_i, ni_len);

	if (nonce_r != NULL)
		gcry_md_write(hm, nonce_r, nr_len);

	if (pl != NULL) {
		flatten_isakmp_payloads(pl, &pl_flat, &pl_size);
		gcry_md_write(hm, pl_flat, pl_size);
		memset(pl_flat, 0, pl_size);
		free(pl_flat);
	}

	gcry_md_final(hm);
	memcpy(p->payload->u.hash.data, gcry_md_read(hm, 0), s->ike.md_len);
	gcry_md_close(hm);

	flatten_isakmp_packet(p, p_flat, p_size, s->ike.ivlen);
	free_isakmp_packet(p);
}

static void sendrecv_phase2(struct sa_block *s, struct isakmp_payload *pl,
	uint8_t exchange_type, uint32_t msgid, int sendonly,
	uint8_t ** save_p_flat, size_t * save_p_size,
	uint8_t * nonce_i, int ni_len, uint8_t * nonce_r, int nr_len)
{
	uint8_t *p_flat;
	size_t p_size;
	ssize_t recvlen;

	if ((save_p_flat == NULL) || (*save_p_flat == NULL)) {
		phase2_authpacket(s, pl, exchange_type, msgid, &p_flat, &p_size,
			nonce_i, ni_len, nonce_r, nr_len);
		isakmp_crypt(s, p_flat, p_size, 1);
	} else {
		p_flat = *save_p_flat;
		p_size = *save_p_size;
	}

	s->ike.life.tx += p_size;

	recvlen = sendrecv(s, r_packet, sizeof(r_packet), p_flat, p_size, sendonly);
	if (sendonly == 0)
		r_length = recvlen;
	
	if (save_p_flat == NULL) {
		free(p_flat);
	} else {
		*save_p_flat = p_flat;
		*save_p_size = p_size;
	}
}

static void send_phase2_late(struct sa_block *s, struct isakmp_payload *pl,
	uint8_t exchange_type, uint32_t msgid)
{
	struct isakmp_packet *p;
	uint8_t *p_flat;
	size_t p_size;
	ssize_t recvlen;

	/* Build up the packet.  */
	p = new_isakmp_packet();
	memcpy(p->i_cookie, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
	memcpy(p->r_cookie, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
	p->flags = ISAKMP_FLAG_E;
	p->isakmp_version = ISAKMP_VERSION;
	p->exchange_type = exchange_type;
	p->message_id = msgid;
	p->payload = pl;

	flatten_isakmp_packet(p, &p_flat, &p_size, s->ike.ivlen);
	free_isakmp_packet(p);
	isakmp_crypt(s, p_flat, p_size, 1);

	s->ike.life.tx += p_size;

	recvlen = sendrecv(s, NULL, 0, p_flat, p_size, 1);
	free(p_flat);
}

void keepalive_ike(struct sa_block *s)
{
	uint32_t msgid;

	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	send_phase2_late(s, NULL, ISAKMP_EXCHANGE_INFORMATIONAL, msgid);
}

static void send_dpd(struct sa_block *s, int isack, uint32_t seqno)
{
	struct isakmp_payload *pl;
	uint32_t msgid;
	
	pl = new_isakmp_payload(ISAKMP_PAYLOAD_N);
	pl->u.n.doi = ISAKMP_DOI_IPSEC;
	pl->u.n.protocol = ISAKMP_IPSEC_PROTO_ISAKMP;
	pl->u.n.type = isack ? ISAKMP_N_R_U_THERE_ACK : ISAKMP_N_R_U_THERE;
	pl->u.n.spi_length = 2 * ISAKMP_COOKIE_LENGTH;
	pl->u.n.spi = xallocc(2 * ISAKMP_COOKIE_LENGTH);
	memcpy(pl->u.n.spi + ISAKMP_COOKIE_LENGTH * 0, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
	memcpy(pl->u.n.spi + ISAKMP_COOKIE_LENGTH * 1, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
	pl->u.n.data_length = 4;
	pl->u.n.data = xallocc(4);
	memcpy(pl->u.n.data, &seqno, 4);
	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	/* 2007-09-06 JKU/ZID: Sonicwall drops non hashed r_u_there-requests */
	sendrecv_phase2(s, pl, ISAKMP_EXCHANGE_INFORMATIONAL, msgid,
		1 , NULL, NULL, NULL, 0, NULL, 0);
}

void dpd_ike(struct sa_block *s)
{
	if (!s->ike.do_dpd)
		return;
	
	if (s->ike.dpd_seqno == s->ike.dpd_seqno_ack) {
		/* Increase the sequence number, reset the attempts to 6, record
		** the current time and send a dpd request
		*/
		s->ike.dpd_attempts = 6;
		s->ike.dpd_sent = time(NULL);
		++s->ike.dpd_seqno;
		send_dpd(s, 0, s->ike.dpd_seqno);
	} else {
		/* Our last dpd request has not yet been acked.  If it's been
		** less than 5 seconds since we sent it do nothing.  Otherwise
		** decrement dpd_attempts.  If dpd_attempts is 0 dpd fails and we
		** terminate otherwise we send it again with the same sequence
		** number and record current time.
		*/
		time_t now = time(NULL);
		if (now < s->ike.dpd_sent + 5)
			return;
		if (--s->ike.dpd_attempts == 0) {
			DEBUG(2, printf("dead peer detected, terminating\n"));
			do_kill = -2;
			return;
		}
		s->ike.dpd_sent = now;
		send_dpd(s, 0, s->ike.dpd_seqno);
	}
}

static void phase2_fatal(struct sa_block *s, const char *msg, int id)
{
	struct isakmp_payload *pl;
	uint32_t msgid;

	DEBUG(1, printf("\n\n---!!!!!!!!! entering phase2_fatal !!!!!!!!!---\n\n\n"));
	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	pl = new_isakmp_payload(ISAKMP_PAYLOAD_N);
	pl->u.n.doi = ISAKMP_DOI_IPSEC;
	pl->u.n.protocol = ISAKMP_IPSEC_PROTO_ISAKMP;
	pl->u.n.type = id;
	sendrecv_phase2(s, pl, ISAKMP_EXCHANGE_INFORMATIONAL, msgid, 1, 0, 0, 0, 0, 0, 0);

	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	pl = new_isakmp_payload(ISAKMP_PAYLOAD_D);
	pl->u.d.doi = ISAKMP_DOI_IPSEC;
	pl->u.d.protocol = ISAKMP_IPSEC_PROTO_ISAKMP;
	pl->u.d.spi_length = 2 * ISAKMP_COOKIE_LENGTH;
	pl->u.d.num_spi = 1;
	pl->u.d.spi = xallocc(1 * sizeof(uint8_t *));
	pl->u.d.spi[0] = xallocc(2 * ISAKMP_COOKIE_LENGTH);
	memcpy(pl->u.d.spi[0] + ISAKMP_COOKIE_LENGTH * 0, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
	memcpy(pl->u.d.spi[0] + ISAKMP_COOKIE_LENGTH * 1, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
	sendrecv_phase2(s, pl, ISAKMP_EXCHANGE_INFORMATIONAL, msgid, 1, 0, 0, 0, 0, 0, 0);

	error(1, 0, msg, val_to_string(id, isakmp_notify_enum_array), id);
}

static uint8_t *gen_keymat(struct sa_block *s,
	uint8_t protocol, uint32_t spi,
	const uint8_t * dh_shared, size_t dh_size,
	const uint8_t * ni_data, size_t ni_size, const uint8_t * nr_data, size_t nr_size)
{
	gcry_md_hd_t hm;
	uint8_t *block;
	int i;
	int blksz;
	int cnt;

	blksz = s->ipsec.md_len + s->ipsec.key_len;
	cnt = (blksz + s->ike.md_len - 1) / s->ike.md_len;
	block = xallocc(cnt * s->ike.md_len);
	DEBUG(3, printf("generating %d bytes keymat (cnt=%d)\n", blksz, cnt));
	if (cnt < 1)
		abort();

	for (i = 0; i < cnt; i++) {
		gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
		gcry_md_setkey(hm, s->ike.skeyid_d, s->ike.md_len);
		if (i != 0)
			gcry_md_write(hm, block + (i - 1) * s->ike.md_len, s->ike.md_len);
		if (dh_shared != NULL)
			gcry_md_write(hm, dh_shared, dh_size);
		gcry_md_write(hm, &protocol, 1);
		gcry_md_write(hm, (uint8_t *) & spi, sizeof(spi));
		gcry_md_write(hm, ni_data, ni_size);
		gcry_md_write(hm, nr_data, nr_size);
		gcry_md_final(hm);
		memcpy(block + i * s->ike.md_len, gcry_md_read(hm, 0), s->ike.md_len);
		gcry_md_close(hm);
	}
	return block;
}

static int mask_to_masklen(struct in_addr mask)
{ 
	int len;
	uint32_t addr;
	
	addr = ntohl(mask.s_addr);
	for (len = 0; addr; addr <<= 1, len++)
		;
	return len;
}

static int do_config_to_env(struct sa_block *s, struct isakmp_attribute *a)
{
	int i;
	int reject = 0;
	int seen_address = 0;
	char *strbuf, *strbuf2;
	
	unsetenv("CISCO_BANNER");
	unsetenv("CISCO_DEF_DOMAIN");
	unsetenv("CISCO_SPLIT_INC");
	unsetenv("INTERNAL_IP4_NBNS");
	unsetenv("INTERNAL_IP4_DNS");
	unsetenv("INTERNAL_IP4_NETMASK");
	unsetenv("INTERNAL_IP4_ADDRESS");

	for (; a && reject == 0; a = a->next)
		switch (a->type) {
		case ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_ADDRESS:
			if (a->af != isakmp_attr_lots || a->u.lots.length != 4)
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			else {
				addenv_ipv4("INTERNAL_IP4_ADDRESS", a->u.lots.data);
				memcpy(s->our_address, a->u.lots.data, 4);
			}
			seen_address = 1;
			break;

		case ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_NETMASK:
			if (a->af == isakmp_attr_lots && a->u.lots.length == 0) {
				DEBUG(2, printf("ignoring zero length netmask\n"));
				continue;
			}
			if (a->af != isakmp_attr_lots || a->u.lots.length != 4)
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			else {
				uint32_t netaddr = ((struct in_addr *)(s->our_address))->s_addr & ((struct in_addr *)(a->u.lots.data))->s_addr;
				addenv_ipv4("INTERNAL_IP4_NETMASK", a->u.lots.data);
				asprintf(&strbuf, "%d", mask_to_masklen(*((struct in_addr *)a->u.lots.data)));
				setenv("INTERNAL_IP4_NETMASKLEN", strbuf, 1);
				free(strbuf);
				addenv_ipv4("INTERNAL_IP4_NETADDR",  (uint8_t *)&netaddr);
			}
			break;

		case ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_DNS:
			if (a->af != isakmp_attr_lots || a->u.lots.length != 4)
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			else
				addenv_ipv4("INTERNAL_IP4_DNS", a->u.lots.data);
			break;

		case ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_NBNS:
			if (a->af != isakmp_attr_lots || a->u.lots.length != 4)
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			else
				addenv_ipv4("INTERNAL_IP4_NBNS", a->u.lots.data);
			break;

		case ISAKMP_MODECFG_ATTRIB_CISCO_DEF_DOMAIN:
			if (a->af != isakmp_attr_lots) {
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
				break;
			}
			strbuf = xallocc(a->u.lots.length + 1);
			memcpy(strbuf, a->u.lots.data, a->u.lots.length);
			addenv("CISCO_DEF_DOMAIN", strbuf);
			free(strbuf);
			break;

		case ISAKMP_MODECFG_ATTRIB_CISCO_BANNER:
			if (a->af != isakmp_attr_lots) {
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
				break;
			}
			strbuf = xallocc(a->u.lots.length + 1);
			memcpy(strbuf, a->u.lots.data, a->u.lots.length);
			addenv("CISCO_BANNER", strbuf);
			free(strbuf);
			DEBUG(1, printf("Banner: "));
			DEBUG(1, fwrite(a->u.lots.data, a->u.lots.length, 1, stdout));
			DEBUG(1, printf("\n"));
			break;

		case ISAKMP_MODECFG_ATTRIB_APPLICATION_VERSION:
			DEBUG(2, printf("Remote Application Version: "));
			DEBUG(2, fwrite(a->u.lots.data, a->u.lots.length, 1, stdout));
			DEBUG(2, printf("\n"));
			break;

		case ISAKMP_MODECFG_ATTRIB_CISCO_DO_PFS:
			if (a->af != isakmp_attr_16)
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			else {
				s->ipsec.do_pfs = a->u.attr_16;
				DEBUG(2, printf("got pfs setting: %d\n", s->ipsec.do_pfs));
			}
			break;

		case ISAKMP_MODECFG_ATTRIB_CISCO_UDP_ENCAP_PORT:
			if (a->af != isakmp_attr_16)
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			else {
				s->ipsec.peer_udpencap_port = a->u.attr_16;
				DEBUG(2, printf("got peer udp encapsulation port: %hu\n", s->ipsec.peer_udpencap_port));
			}
			break;

		case ISAKMP_MODECFG_ATTRIB_CISCO_SPLIT_INC:
			if (a->af != isakmp_attr_acl) {
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
				break;
			}
			
			DEBUG(2, printf("got %d acls for split include\n", a->u.acl.count));
			asprintf(&strbuf, "%d", a->u.acl.count);
			setenv("CISCO_SPLIT_INC", strbuf, 1);
			free(strbuf);
			
			for (i = 0; i < a->u.acl.count; i++) {
				DEBUG(2, printf("acl %d: ", i));
				/* NOTE: inet_ntoa returns one static buffer */
				
				asprintf(&strbuf, "CISCO_SPLIT_INC_%d_ADDR", i);
				asprintf(&strbuf2, "%s", inet_ntoa(a->u.acl.acl_ent[i].addr));
				DEBUG(2, printf("addr: %s/", strbuf2));
				setenv(strbuf, strbuf2, 1);
				free(strbuf); free(strbuf2);
				
				asprintf(&strbuf, "CISCO_SPLIT_INC_%d_MASK", i);
				asprintf(&strbuf2, "%s", inet_ntoa(a->u.acl.acl_ent[i].mask));
				DEBUG(2, printf("%s ", strbuf2));
				setenv(strbuf, strbuf2, 1);
				free(strbuf); free(strbuf2);
				
				/* this is just here because ip route does not accept netmasks */
				asprintf(&strbuf, "CISCO_SPLIT_INC_%d_MASKLEN", i);
				asprintf(&strbuf2, "%d", mask_to_masklen(a->u.acl.acl_ent[i].mask));
				DEBUG(2, printf("(%s), ", strbuf2));
				setenv(strbuf, strbuf2, 1);
				free(strbuf); free(strbuf2);
				
				asprintf(&strbuf, "CISCO_SPLIT_INC_%d_PROTOCOL", i);
				asprintf(&strbuf2, "%hu", a->u.acl.acl_ent[i].protocol);
				DEBUG(2, printf("protocol: %s, ", strbuf2));
				setenv(strbuf, strbuf2, 1);
				free(strbuf); free(strbuf2);
				
				asprintf(&strbuf, "CISCO_SPLIT_INC_%d_SPORT", i);
				asprintf(&strbuf2, "%hu", a->u.acl.acl_ent[i].sport);
				DEBUG(2, printf("sport: %s, ", strbuf2));
				setenv(strbuf, strbuf2, 1);
				free(strbuf); free(strbuf2);
				
				asprintf(&strbuf, "CISCO_SPLIT_INC_%d_DPORT", i);
				asprintf(&strbuf2, "%hu", a->u.acl.acl_ent[i].dport);
				DEBUG(2, printf("dport: %s\n", strbuf2));
				setenv(strbuf, strbuf2, 1);
				free(strbuf); free(strbuf2);
			}
			break;
			
		case ISAKMP_MODECFG_ATTRIB_CISCO_SAVE_PW:
			DEBUG(2, printf("got save password setting: %d\n", a->u.attr_16));
			break;
			
		default:
			DEBUG(2, printf("unknown attribute %d / 0x%X\n", a->type, a->type));
			break;
		}

	if (reject == 0 && !seen_address)
		reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
	
	return reject;
}

/* * */

static struct isakmp_attribute *make_transform_ike(int dh_group, int crypt, int hash, int keylen, int auth)
{
	struct isakmp_attribute *a = NULL;

	a = new_isakmp_attribute(IKE_ATTRIB_LIFE_DURATION, a);
	a->af = isakmp_attr_lots;
	a->u.lots.length = 4;
	a->u.lots.data = xallocc(a->u.lots.length);
	*((uint32_t *) a->u.lots.data) = htonl(2147483);
	a = new_isakmp_attribute_16(IKE_ATTRIB_LIFE_TYPE, IKE_LIFE_TYPE_SECONDS, a);
	a = new_isakmp_attribute_16(IKE_ATTRIB_AUTH_METHOD, auth, a);
	a = new_isakmp_attribute_16(IKE_ATTRIB_GROUP_DESC, dh_group, a);
	a = new_isakmp_attribute_16(IKE_ATTRIB_HASH, hash, a);
	a = new_isakmp_attribute_16(IKE_ATTRIB_ENC, crypt, a);
	if (keylen != 0)
		a = new_isakmp_attribute_16(IKE_ATTRIB_KEY_LENGTH, keylen, a);
	return a;
}

static struct isakmp_payload *make_our_sa_ike(void)
{
	struct isakmp_payload *r = new_isakmp_payload(ISAKMP_PAYLOAD_SA);
	struct isakmp_payload *t = NULL, *tn;
	struct isakmp_attribute *a;
	int dh_grp = get_dh_group_ike()->ike_sa_id;
	unsigned int auth, crypt, hash, keylen;
	int i;

	r->u.sa.doi = ISAKMP_DOI_IPSEC;
	r->u.sa.situation = ISAKMP_IPSEC_SIT_IDENTITY_ONLY;
	r->u.sa.proposals = new_isakmp_payload(ISAKMP_PAYLOAD_P);
	r->u.sa.proposals->u.p.prot_id = ISAKMP_IPSEC_PROTO_ISAKMP;
	for (auth = 0; supp_auth[auth].name != NULL; auth++) {
		if (opt_auth_mode == AUTH_MODE_CERT) {
			if ((supp_auth[auth].ike_sa_id != IKE_AUTH_RSA_SIG) &&
				(supp_auth[auth].ike_sa_id != IKE_AUTH_DSS))
				continue;
		} else if (opt_auth_mode == AUTH_MODE_HYBRID) {
			if ((supp_auth[auth].ike_sa_id != IKE_AUTH_HybridInitRSA) &&
				(supp_auth[auth].ike_sa_id != IKE_AUTH_HybridInitDSS)) 
				continue;
		} else {
			if (supp_auth[auth].ike_sa_id == IKE_AUTH_HybridInitRSA ||
				supp_auth[auth].ike_sa_id == IKE_AUTH_HybridInitDSS ||
				supp_auth[auth].ike_sa_id == IKE_AUTH_RSA_SIG ||
				supp_auth[auth].ike_sa_id == IKE_AUTH_DSS)
				continue;
		}
		for (crypt = 0; supp_crypt[crypt].name != NULL; crypt++) {
			keylen = supp_crypt[crypt].keylen;
			for (hash = 0; supp_hash[hash].name != NULL; hash++) {
				tn = t;
				t = new_isakmp_payload(ISAKMP_PAYLOAD_T);
				t->u.t.id = ISAKMP_IPSEC_KEY_IKE;
				a = make_transform_ike(dh_grp, supp_crypt[crypt].ike_sa_id,
					supp_hash[hash].ike_sa_id, keylen, supp_auth[auth].ike_sa_id);
				t->u.t.attributes = a;
				t->next = tn;
			}
		}
	}
	for (i = 0, tn = t; tn; tn = tn->next)
		tn->u.t.number = i++;
	r->u.sa.proposals->u.p.transforms = t;
	return r;
}

static void lifetime_ike_process(struct sa_block *s, struct isakmp_attribute *a)
{
	uint32_t value;
	
	assert(a != NULL);
	assert(a->type == IKE_ATTRIB_LIFE_TYPE);
	assert(a->af == isakmp_attr_16);
	assert(a->u.attr_16 == IKE_LIFE_TYPE_SECONDS || a->u.attr_16 == IKE_LIFE_TYPE_K);
	assert(a->next != NULL);
	assert(a->next->type == IKE_ATTRIB_LIFE_DURATION);
	
	if (a->next->af == isakmp_attr_16)
		value = a->next->u.attr_16;
	else if (a->next->af == isakmp_attr_lots && a->next->u.lots.length == 4)
		value = ntohl(*((uint32_t *) a->next->u.lots.data));
	else
		assert(0);
	
	DEBUG(2, printf("got ike lifetime attributes: %d %s\n", value,
		(a->u.attr_16 == IKE_LIFE_TYPE_SECONDS) ? "seconds" : "kilobyte"));
	
	if (a->u.attr_16 == IKE_LIFE_TYPE_SECONDS)
		s->ike.life.seconds = value;
	else
		s->ike.life.kbytes = value;
}

static void lifetime_ipsec_process(struct sa_block *s, struct isakmp_attribute *a)
{
	uint32_t value;
	
	assert(a != NULL);
	assert(a->type == ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE);
	assert(a->af == isakmp_attr_16);
	assert(a->u.attr_16 == IPSEC_LIFE_SECONDS || a->u.attr_16 == IPSEC_LIFE_K);
	assert(a->next != NULL);
	assert(a->next->type == ISAKMP_IPSEC_ATTRIB_SA_LIFE_DURATION);
	
	if (a->next->af == isakmp_attr_16)
		value = a->next->u.attr_16;
	else if (a->next->af == isakmp_attr_lots && a->next->u.lots.length == 4)
		value = ntohl(*((uint32_t *) a->next->u.lots.data));
	else
		assert(0);
	
	DEBUG(2, printf("got ipsec lifetime attributes: %d %s\n", value,
		(a->u.attr_16 == IPSEC_LIFE_SECONDS) ? "seconds" : "kilobyte"));
	
	if (a->u.attr_16 == IPSEC_LIFE_SECONDS)
		s->ipsec.life.seconds = value;
	else
		s->ipsec.life.kbytes = value;
	
	/* FIXME: for notice-payloads: write a seperate function to handle them */
	/* bug: this may process lifetime-attributes of SAs twice but to no consequence */
	if (a->next->next != NULL && a->next->next->type == ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE)
		lifetime_ipsec_process(s, a->next->next);
}

static void do_phase1_am(const char *key_id, const char *shared_key, struct sa_block *s)
{
	unsigned char i_nonce[20];
	struct group *dh_grp;
	unsigned char *dh_public;
	unsigned char *returned_hash;
	unsigned char *psk_hash;

	struct isakmp_packet *p1;
	struct isakmp_packet *r;
	int seen_natt_vid = 0, seen_natd = 0, seen_natd_them = 0, seen_natd_us = 0, natd_type = 0;
	unsigned char *natd_us = NULL, *natd_them = NULL;
	int natt_draft = -1;
	unsigned char *dh_shared_secret;
	
	DEBUGTOP(2, printf("S4.1 create_nonce\n"));
	gcry_create_nonce(s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
	s->ike.life.start = time(NULL);
	s->ipsec.do_pfs = -1;
	if (s->ike.i_cookie[0] == 0)
		s->ike.i_cookie[0] = 1;
	hex_dump("i_cookie", s->ike.i_cookie, ISAKMP_COOKIE_LENGTH, NULL);
	gcry_create_nonce(i_nonce, sizeof(i_nonce));
	hex_dump("i_nonce", i_nonce, sizeof(i_nonce), NULL);
	DEBUGTOP(2, printf("S4.2 dh setup\n"));
	/* Set up the Diffie-Hellman stuff.  */
	{
		dh_grp = group_get(get_dh_group_ike()->my_id);
		dh_public = xallocc(dh_getlen(dh_grp));
		dh_create_exchange(dh_grp, dh_public);
		hex_dump("dh_public", dh_public, dh_getlen(dh_grp), NULL);
	}

	DEBUGTOP(2, printf("S4.3 AM packet_1\n"));
	/* Create the first packet.  */
	{
		struct isakmp_payload *l;
		uint8_t *pkt;
		size_t pkt_len;

		p1 = new_isakmp_packet();
		memcpy(p1->i_cookie, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
		p1->isakmp_version = ISAKMP_VERSION;
		p1->exchange_type = ISAKMP_EXCHANGE_AGGRESSIVE;
		p1->payload = l = make_our_sa_ike();
		l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_KE, dh_public, dh_getlen(dh_grp));
		l->next->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_NONCE,
			i_nonce, sizeof(i_nonce));
		l = l->next->next;
		l->next = new_isakmp_payload(ISAKMP_PAYLOAD_ID);
		l = l->next;
		if (opt_vendor == VENDOR_CISCO)
			l->u.id.type = ISAKMP_IPSEC_ID_KEY_ID;
		else
			l->u.id.type = ISAKMP_IPSEC_ID_USER_FQDN;
		l->u.id.protocol = IPPROTO_UDP;
		l->u.id.port = ISAKMP_PORT; /* this must be 500, see rfc2407, 4.6.2 */
		l->u.id.length = strlen(key_id);
		l->u.id.data = xallocc(l->u.id.length);
		memcpy(l->u.id.data, key_id, strlen(key_id));
		l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			VID_XAUTH, sizeof(VID_XAUTH));
		l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			VID_UNITY, sizeof(VID_UNITY));
		if ((opt_natt_mode == NATT_NORMAL) || (opt_natt_mode == NATT_FORCE)) {
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				VID_NATT_RFC, sizeof(VID_NATT_RFC));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				VID_NATT_02N, sizeof(VID_NATT_02N));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				VID_NATT_02, sizeof(VID_NATT_02));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				VID_NATT_01, sizeof(VID_NATT_01));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				VID_NATT_00, sizeof(VID_NATT_00));
		}
		s->ike.dpd_idle = atoi(config[CONFIG_DPD_IDLE]);
		if (s->ike.dpd_idle != 0) {
			if (s->ike.dpd_idle < 10)
				s->ike.dpd_idle = 10;
			if (s->ike.dpd_idle > 86400)
				s->ike.dpd_idle = 86400;
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				VID_DPD, sizeof(VID_DPD));
		}
		flatten_isakmp_packet(p1, &pkt, &pkt_len, 0);

		/* Now, send that packet and receive a new one.  */
		r_length = sendrecv(s, r_packet, sizeof(r_packet), pkt, pkt_len, 0);
		free(pkt);
	}
	DEBUGTOP(2, printf("S4.4 AM_packet2\n"));
	/* Decode the recieved packet.  */
	{
		int reject;
		struct isakmp_payload *rp;
		struct isakmp_payload *nonce = NULL;
		struct isakmp_payload *ke = NULL;
		struct isakmp_payload *hash = NULL;
		struct isakmp_payload *last_cert = NULL;
		struct isakmp_payload *sig = NULL;
		struct isakmp_payload *idp = NULL;
		int seen_sa = 0, seen_xauth_vid = 0;
		unsigned char *psk_skeyid;
		unsigned char *skeyid;
		gcry_md_hd_t skeyid_ctx;

#ifdef OPENSSL_GPL_VIOLATION
		X509 *current_cert;
		/* structure to store the certificate chain */
		STACK_OF(X509) *cert_stack = sk_X509_new_null();
#endif /* OPENSSL_GPL_VIOLATION */

		reject = 0;
		r = parse_isakmp_packet(r_packet, r_length, &reject);

		/* Verify the correctness of the recieved packet.  */
		if (reject == 0 && memcmp(r->i_cookie, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH) != 0)
			reject = ISAKMP_N_INVALID_COOKIE;
		if (reject == 0)
			memcpy(s->ike.r_cookie, r->r_cookie, ISAKMP_COOKIE_LENGTH);
		if (reject == 0 && r->exchange_type != ISAKMP_EXCHANGE_AGGRESSIVE)
			reject = ISAKMP_N_INVALID_EXCHANGE_TYPE;
		if (reject == 0 && r->flags != 0)
			reject = ISAKMP_N_INVALID_FLAGS;
		if (reject == 0 && r->message_id != 0)
			reject = ISAKMP_N_INVALID_MESSAGE_ID;
		if (reject != 0)
			error(1, 0, "response was invalid [1]: %s(%d)", val_to_string(reject, isakmp_notify_enum_array), reject);
		for (rp = r->payload; rp && reject == 0; rp = rp->next)
			switch (rp->type) {
			case ISAKMP_PAYLOAD_SA:
				if (reject == 0 && rp->u.sa.doi != ISAKMP_DOI_IPSEC)
					reject = ISAKMP_N_DOI_NOT_SUPPORTED;
				if (reject == 0 &&
					rp->u.sa.situation != ISAKMP_IPSEC_SIT_IDENTITY_ONLY)
					reject = ISAKMP_N_SITUATION_NOT_SUPPORTED;
				if (reject == 0 &&
					(rp->u.sa.proposals == NULL
						|| rp->u.sa.proposals->next != NULL))
					reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
				if (reject == 0 &&
					rp->u.sa.proposals->u.p.prot_id !=
					ISAKMP_IPSEC_PROTO_ISAKMP)
					reject = ISAKMP_N_INVALID_PROTOCOL_ID;
				if (reject == 0 && rp->u.sa.proposals->u.p.spi_size != 0)
					reject = ISAKMP_N_INVALID_SPI;
				if (reject == 0 &&
					(rp->u.sa.proposals->u.p.transforms == NULL
						|| rp->u.sa.proposals->u.p.transforms->next !=
						NULL))
					reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
				if (reject == 0 &&
					(rp->u.sa.proposals->u.p.transforms->u.t.id
						!= ISAKMP_IPSEC_KEY_IKE))
					reject = ISAKMP_N_INVALID_TRANSFORM_ID;
				if (reject == 0) {
					struct isakmp_attribute *a
						=
						rp->u.sa.proposals->u.p.transforms->u.t.attributes;
					int seen_enc = 0, seen_hash = 0, seen_auth = 0;
					int seen_group = 0, seen_keylen = 0;
					for (; a && reject == 0; a = a->next)
						switch (a->type) {
						case IKE_ATTRIB_GROUP_DESC:
							if (a->af == isakmp_attr_16 &&
								a->u.attr_16 ==
								get_dh_group_ike()->ike_sa_id)
								seen_group = 1;
							else
								reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
							break;
						case IKE_ATTRIB_AUTH_METHOD:
							if (a->af == isakmp_attr_16)
								seen_auth = a->u.attr_16;
							else
								reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
							break;
						case IKE_ATTRIB_HASH:
							if (a->af == isakmp_attr_16)
								seen_hash = a->u.attr_16;
							else
								reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
							break;
						case IKE_ATTRIB_ENC:
							if (a->af == isakmp_attr_16)
								seen_enc = a->u.attr_16;
							else
								reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
							break;
						case IKE_ATTRIB_KEY_LENGTH:
							if (a->af == isakmp_attr_16)
								seen_keylen = a->u.attr_16;
							else
								reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
							break;
						case IKE_ATTRIB_LIFE_TYPE:
							/* lifetime duration MUST follow lifetype attribute */
							if (a->next->type == IKE_ATTRIB_LIFE_DURATION) {
								lifetime_ike_process(s, a);
							} else
								reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
							break;
						case IKE_ATTRIB_LIFE_DURATION:
							/* already processed above in IKE_ATTRIB_LIFE_TYPE: */
							break;
						default:
							DEBUG(1, printf
								("unknown attribute %d, arborting..\n",
									a->type));
							reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
							break;
						}
					if (!seen_group || !seen_auth || !seen_hash || !seen_enc)
						reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;

					if (get_algo(SUPP_ALGO_AUTH, SUPP_ALGO_IKE_SA, seen_auth,
							NULL, 0) == NULL)
						reject = ISAKMP_N_NO_PROPOSAL_CHOSEN;
					if (get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IKE_SA, seen_hash,
							NULL, 0) == NULL)
						reject = ISAKMP_N_NO_PROPOSAL_CHOSEN;
					if (get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IKE_SA, seen_enc,
							NULL, seen_keylen) == NULL)
						reject = ISAKMP_N_NO_PROPOSAL_CHOSEN;

					if (reject == 0) {
						seen_sa = 1;
						s->ike.auth_algo = seen_auth;
						s->ike.cry_algo =
							get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IKE_SA,
							seen_enc, NULL, seen_keylen)->my_id;
						s->ike.md_algo =
							get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IKE_SA,
							seen_hash, NULL, 0)->my_id;
						s->ike.md_len = gcry_md_get_algo_dlen(s->ike.md_algo);
						DEBUG(1, printf("IKE SA selected %s-%s-%s\n",
								get_algo(SUPP_ALGO_AUTH,
									SUPP_ALGO_IKE_SA, seen_auth,
									NULL, 0)->name,
								get_algo(SUPP_ALGO_CRYPT,
									SUPP_ALGO_IKE_SA, seen_enc,
									NULL, seen_keylen)->name,
								get_algo(SUPP_ALGO_HASH,
									SUPP_ALGO_IKE_SA, seen_hash,
									NULL, 0)->name));
						if (s->ike.cry_algo == GCRY_CIPHER_DES && !opt_1des) {
							error(1, 0, "peer selected (single) DES as \"encryption\" method.\n"
								"This algorithm is considered too weak today\n"
								"If your vpn concentrator admin still insists on using DES\n"
								"use the \"--enable-1des\" option.\n");
						}
					}
				}
				break;

			case ISAKMP_PAYLOAD_ID:
				idp = rp;
				break;
			case ISAKMP_PAYLOAD_KE:
				ke = rp;
				break;
			case ISAKMP_PAYLOAD_NONCE:
				nonce = rp;
				break;
			case ISAKMP_PAYLOAD_HASH:
				hash = rp;
				break;
			case ISAKMP_PAYLOAD_CERT:
				last_cert = rp;
				if (last_cert->u.cert.encoding == ISAKMP_CERT_X509_SIG) {
#ifdef OPENSSL_GPL_VIOLATION
					/* convert the certificate to an openssl-X509 structure and push it onto the chain stack */
					current_cert = d2i_X509(NULL, (const unsigned char **)&last_cert->u.cert.data, last_cert->u.cert.length);
					sk_X509_push(cert_stack, current_cert);
					last_cert->u.cert.data -= last_cert->u.cert.length; /* 'rewind' the pointer */
#endif /* OPENSSL_GPL_VIOLATION */
				}
				break;
			case ISAKMP_PAYLOAD_SIG:
				sig = rp;
				break;
			case ISAKMP_PAYLOAD_VID:
				if (rp->u.vid.length == sizeof(VID_XAUTH)
					&& memcmp(rp->u.vid.data, VID_XAUTH,
						sizeof(VID_XAUTH)) == 0) {
					seen_xauth_vid = 1;
				} else if (rp->u.vid.length == sizeof(VID_NATT_RFC)
					&& memcmp(rp->u.vid.data, VID_NATT_RFC,
						sizeof(VID_NATT_RFC)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 2;
					DEBUG(2, printf("peer is NAT-T capable (RFC 3947)\n"));
				} else if (rp->u.vid.length == sizeof(VID_NATT_02N)
					&& memcmp(rp->u.vid.data, VID_NATT_02N,
						sizeof(VID_NATT_02N)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 2;
					DEBUG(2, printf("peer is NAT-T capable (draft-02)\\n\n")); /* sic! */
				} else if (rp->u.vid.length == sizeof(VID_NATT_02)
					&& memcmp(rp->u.vid.data, VID_NATT_02,
						sizeof(VID_NATT_02)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 2;
					DEBUG(2, printf("peer is NAT-T capable (draft-02)\n"));
				} else if (rp->u.vid.length == sizeof(VID_NATT_01)
					&& memcmp(rp->u.vid.data, VID_NATT_01,
						sizeof(VID_NATT_01)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 1;
					DEBUG(2, printf("peer is NAT-T capable (draft-01)\n"));
				} else if (rp->u.vid.length == sizeof(VID_NATT_00)
					&& memcmp(rp->u.vid.data, VID_NATT_00,
						sizeof(VID_NATT_00)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 0) natt_draft = 0;
					DEBUG(2, printf("peer is NAT-T capable (draft-00)\n"));
				} else if (rp->u.vid.length == sizeof(VID_DPD)
					&& memcmp(rp->u.vid.data, VID_DPD,
						sizeof(VID_DPD)) == 0) {
					if (s->ike.dpd_idle != 0) {
						gcry_create_nonce(&s->ike.dpd_seqno, sizeof(s->ike.dpd_seqno));
						s->ike.dpd_seqno &= 0x7FFFFFFF;
						s->ike.dpd_seqno_ack = s->ike.dpd_seqno;
						s->ike.do_dpd = 1;
						DEBUG(2, printf("peer is DPD capable (RFC3706)\n"));
					} else {
						DEBUG(2, printf("ignoring that peer is DPD capable (RFC3706)\n"));
					}
				} else if (rp->u.vid.length == sizeof(VID_NETSCREEN_15)
					&& memcmp(rp->u.vid.data, VID_NETSCREEN_15,
						sizeof(VID_NETSCREEN_15)) == 0) {
					DEBUG(2, printf("peer is using ScreenOS 5.3, 5.4 or 6.0\n"));
				} else if (rp->u.vid.length == sizeof(VID_HEARTBEAT_NOTIFY)
					&& memcmp(rp->u.vid.data, VID_HEARTBEAT_NOTIFY,
						sizeof(VID_HEARTBEAT_NOTIFY)) == 0) {
					DEBUG(2, printf("peer sent Heartbeat Notify payload\n"));
				} else {
					hex_dump("unknown ISAKMP_PAYLOAD_VID",
						rp->u.vid.data, rp->u.vid.length, NULL);
				}
				break;
			case ISAKMP_PAYLOAD_NAT_D_OLD:
			case ISAKMP_PAYLOAD_NAT_D:
				natd_type = rp->type;
				DEBUG(2, printf("peer is using type %d%s for NAT-Discovery payloads\n",
					natd_type, val_to_string(natd_type, isakmp_payload_enum_array)));
				if (!seen_sa /*|| !seen_natt_vid*/) {
					reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;
				} else if (opt_natt_mode == NATT_NONE) {
					;
				} else if (rp->u.natd.length != s->ike.md_len) {
					reject = ISAKMP_N_PAYLOAD_MALFORMED;
				} else if (seen_natd == 0) {
					gcry_md_hd_t hm;
					uint16_t n_dst_port = htons(s->ike.dst_port);
					
					natd_us = xallocc(s->ike.md_len);
					natd_them = xallocc(s->ike.md_len);
					memcpy(natd_us, rp->u.natd.data, s->ike.md_len);
					gcry_md_open(&hm, s->ike.md_algo, 0);
					gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
					gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
					gcry_md_write(hm, &s->dst, sizeof(struct in_addr));
					gcry_md_write(hm, &n_dst_port, sizeof(uint16_t));
					gcry_md_final(hm);
					memcpy(natd_them, gcry_md_read(hm, 0), s->ike.md_len);
					gcry_md_close(hm);
					seen_natd = 1;
				} else {
					if (memcmp(natd_them, rp->u.natd.data, s->ike.md_len) == 0)
						seen_natd_them = 1;
				}
				break;
			default:
				DEBUG(1, printf("rejecting invalid payload type %d\n", rp->type));
				reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;
				break;
			}

		if (reject == 0) {
			gcry_cipher_algo_info(s->ike.cry_algo, GCRYCTL_GET_BLKLEN, NULL, &(s->ike.ivlen));
			gcry_cipher_algo_info(s->ike.cry_algo, GCRYCTL_GET_KEYLEN, NULL, &(s->ike.keylen));
		}

		if (reject == 0 && (ke == NULL || ke->u.ke.length != dh_getlen(dh_grp)))
			reject = ISAKMP_N_INVALID_KEY_INFORMATION;
		if (reject == 0 && nonce == NULL)
			reject = ISAKMP_N_INVALID_HASH_INFORMATION;
		if (reject != 0)
			error(1, 0, "response was invalid [2]: %s(%d)", val_to_string(reject, isakmp_notify_enum_array), reject);
		if (reject == 0 && idp == NULL)
			reject = ISAKMP_N_INVALID_ID_INFORMATION;
  
		/* Decide if signature or hash is expected (sig only if vpnc is initiator of hybrid-auth */
		if (reject == 0 && opt_auth_mode == AUTH_MODE_PSK && (hash == NULL || hash->u.hash.length != s->ike.md_len))
			reject = ISAKMP_N_INVALID_HASH_INFORMATION;
		if (reject == 0 && sig == NULL &&
			(opt_auth_mode == AUTH_MODE_CERT ||
			 opt_auth_mode == AUTH_MODE_HYBRID))
			reject = ISAKMP_N_INVALID_SIGNATURE;
		if (reject != 0)
			error(1, 0, "response was invalid [3]: %s(%d)", val_to_string(reject, isakmp_notify_enum_array), reject);

		/* Determine the shared secret.  */
		dh_shared_secret = xallocc(dh_getlen(dh_grp));
		dh_create_shared(dh_grp, dh_shared_secret, ke->u.ke.data);
		hex_dump("dh_shared_secret", dh_shared_secret, dh_getlen(dh_grp), NULL);
		/* Generate SKEYID.  */
		{
			gcry_md_open(&skeyid_ctx, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(skeyid_ctx, shared_key, strlen(shared_key));
			gcry_md_write(skeyid_ctx, i_nonce, sizeof(i_nonce));
			gcry_md_write(skeyid_ctx, nonce->u.nonce.data, nonce->u.nonce.length);
			gcry_md_final(skeyid_ctx);
			psk_skeyid = xallocc(s->ike.md_len);
			memcpy(psk_skeyid, gcry_md_read(skeyid_ctx, 0), s->ike.md_len);
			if (opt_debug < 99)
				DEBUG(3, printf("(not dumping psk hash)\n"));
			else
				hex_dump("psk_skeyid", psk_skeyid, s->ike.md_len, NULL);
			free(psk_skeyid);
			gcry_md_close(skeyid_ctx);
			DEBUG(99, printf("shared-key: %s\n",shared_key));
			
			/* SKEYID - psk only */
			if (s->ike.auth_algo == IKE_AUTH_PRESHARED ||
				s->ike.auth_algo == IKE_AUTH_XAUTHInitPreShared ||
				s->ike.auth_algo == IKE_AUTH_XAUTHRespPreShared) {
				gcry_md_open(&skeyid_ctx, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
				gcry_md_setkey(skeyid_ctx, shared_key, strlen(shared_key));
				gcry_md_write(skeyid_ctx, i_nonce, sizeof(i_nonce));
				gcry_md_write(skeyid_ctx, nonce->u.nonce.data, nonce->u.nonce.length);
				gcry_md_final(skeyid_ctx);
			} else if (s->ike.auth_algo == IKE_AUTH_DSS ||
				s->ike.auth_algo == IKE_AUTH_RSA_SIG ||
				s->ike.auth_algo == IKE_AUTH_ECDSA_SIG ||
				s->ike.auth_algo == IKE_AUTH_HybridInitRSA ||
				s->ike.auth_algo == IKE_AUTH_HybridRespRSA || 
				s->ike.auth_algo == IKE_AUTH_HybridInitDSS ||
				s->ike.auth_algo == IKE_AUTH_HybridRespDSS ||
				s->ike.auth_algo == IKE_AUTH_XAUTHInitDSS ||
				s->ike.auth_algo == IKE_AUTH_XAUTHRespDSS ||
				s->ike.auth_algo == IKE_AUTH_XAUTHInitRSA ||
				s->ike.auth_algo == IKE_AUTH_XAUTHRespRSA) {
				unsigned char *key;
				int key_len;
				key_len = sizeof(i_nonce) + nonce->u.nonce.length;
				key = xallocc(key_len);
				memcpy(key, i_nonce, sizeof(i_nonce));
				memcpy(key + sizeof(i_nonce), nonce->u.nonce.data, nonce->u.nonce.length);
				gcry_md_open(&skeyid_ctx, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
				gcry_md_setkey(skeyid_ctx, key, key_len);
				gcry_md_write(skeyid_ctx, dh_shared_secret, dh_getlen(dh_grp));
				gcry_md_final(skeyid_ctx);
			} else
				error(1, 0, "SKEYID could not be computed: %s", "the selected authentication method is not supported");
			skeyid = gcry_md_read(skeyid_ctx, 0);
			hex_dump("skeyid", skeyid, s->ike.md_len, NULL);
		}

		/* Verify the hash.  */
		{
			gcry_md_hd_t hm;
			unsigned char *expected_hash;
			uint8_t *sa_f, *idi_f, *idp_f;
			size_t sa_size, idi_size, idp_size;
			struct isakmp_payload *sa, *idi;

			sa = p1->payload;
			for (idi = sa; idi->type != ISAKMP_PAYLOAD_ID; idi = idi->next) ;
			flatten_isakmp_payload(sa, &sa_f, &sa_size);
			flatten_isakmp_payload(idi, &idi_f, &idi_size);
			flatten_isakmp_payload(idp, &idp_f, &idp_size);

			gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(hm, skeyid, s->ike.md_len);
			gcry_md_write(hm, ke->u.ke.data, ke->u.ke.length);
			gcry_md_write(hm, dh_public, dh_getlen(dh_grp));
			gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, sa_f + 4, sa_size - 4);
			gcry_md_write(hm, idp_f + 4, idp_size - 4);
			gcry_md_final(hm);
			expected_hash = gcry_md_read(hm, 0);
			hex_dump("expected hash", expected_hash, s->ike.md_len, NULL);

			if (opt_auth_mode == AUTH_MODE_PSK) {
				if (memcmp(expected_hash, hash->u.hash.data, s->ike.md_len) != 0)
					error(2, 0, "hash comparison failed: %s(%d)\ncheck group password!",
						val_to_string(ISAKMP_N_AUTHENTICATION_FAILED, isakmp_notify_enum_array),
						ISAKMP_N_AUTHENTICATION_FAILED);
				hex_dump("received hash", hash->u.hash.data, hash->u.hash.length, NULL);
			} else if (opt_auth_mode == AUTH_MODE_CERT ||
				opt_auth_mode == AUTH_MODE_HYBRID) {
#ifdef OPENSSL_GPL_VIOLATION
				
				/* BEGIN - check the signature using OpenSSL */
			
				X509 		*x509;
				EVP_PKEY 	*pkey;
				RSA 		*rsa;
				X509_STORE 	*store;
				/* X509_LOOKUP	*lookup; */
				X509_STORE_CTX	*verify_ctx;
				unsigned char	*rec_hash;
				int		decr_size;
	
			  	hex_dump("received signature", sig->u.sig.data, sig->u.sig.length, NULL);
				OpenSSL_add_all_ciphers();
				OpenSSL_add_all_digests();
				OpenSSL_add_all_algorithms();
	
				ERR_load_crypto_strings();
	
				hex_dump("last cert", last_cert->u.cert.data, last_cert->u.cert.length, NULL);
				x509 = d2i_X509(NULL, (const unsigned char **)&last_cert->u.cert.data, last_cert->u.cert.length);
				if (x509 == NULL) {
					ERR_print_errors_fp (stderr);
					error(1, 0, "x509 error\n");
				}
				DEBUG(3, printf("Subject name hash: %08lx\n",X509_subject_name_hash(x509)));
	
				/* BEGIN - verify certificate chain */
				/* create the cert store */
				if (!(store = X509_STORE_new())) {
					error(1, 0, "Error creating X509_STORE object\n");
				}
				/* load the CA certificates */
				if (X509_STORE_load_locations (store, config[CONFIG_CA_FILE], config[CONFIG_CA_DIR]) != 1) {
					error(1, 0, "Error loading the CA file or directory\n");
				}
				if (X509_STORE_set_default_paths (store) != 1) {
					error(1, 0, "Error loading the system-wide CA certificates\n");
				}

#if 0
				/* check CRLs */
				/* add the corresponding CRL for each CA in the chain to the lookup */
#define CRL_FILE "root-ca-crl.crl.pem"

				if (!(lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file()))) {
					error(1, 0, "Error creating X509 lookup object.\n");
				}
				if (X509_load_crl_file(lookup, CRL_FILE, X509_FILETYPE_PEM) != 1) {
					ERR_print_errors_fp(stderr);
					error(1, 0, "Error reading CRL file\n");
				}
				X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif /* 0 */
				/* create a verification context and initialize it */
				if (!(verify_ctx = X509_STORE_CTX_new ())) {
					error(1, 0, "Error creating X509_STORE_CTX object\n");
				}
				/* X509_STORE_CTX_init did not return an error condition
				in prior versions */
				if (X509_STORE_CTX_init (verify_ctx, store, x509, cert_stack) != 1)
					printf("Error intializing verification context\n");
	
				/* verify the certificate */
				if (X509_verify_cert(verify_ctx) != 1) {
					ERR_print_errors_fp(stderr);
					error(2, 0, "Error verifying the certificate-chain\n");
				} else
					DEBUG(3, printf("Certificate-chain verified correctly!\n"));
	
				/* END   - verify certificate chain */
	
	
				/* BEGIN - Signature Verification */
				pkey = X509_get_pubkey(x509);
				if (pkey == NULL) {
					ERR_print_errors_fp (stderr);
					exit (1);
				}
	
				rsa = EVP_PKEY_get1_RSA(pkey);
				if (rsa == NULL) {
					ERR_print_errors_fp (stderr);
					exit (1);
				}
				rec_hash = xallocc(s->ike.md_len);
				decr_size = RSA_public_decrypt(sig->u.sig.length, sig->u.sig.data, rec_hash, rsa, RSA_PKCS1_PADDING);
	
				if (decr_size != (int) s->ike.md_len) {
					printf("Decrypted-Size: %d\n",decr_size);
					hex_dump("    decr_hash", rec_hash, decr_size, NULL);
					hex_dump("expected hash", expected_hash, s->ike.md_len, NULL);
				
					error(2, 0, "The hash-value, which was decrypted from the received signature, and the expected hash-value differ in size.\n");
				} else {
					if (memcmp(rec_hash, expected_hash, decr_size) != 0) {
						printf("Decrypted-Size: %d\n",decr_size);
						hex_dump("    decr_hash", rec_hash, decr_size, NULL);
						hex_dump("expected hash", expected_hash, s->ike.md_len, NULL);
	
						error(2, 0, "The hash-value, which was decrypted from the received signature, and the expected hash-value differ.\n");
					} else {
						DEBUG(3, printf("Signature MATCH!!\n"));
					}
				}
				/* END - Signature Verification */
	
				EVP_PKEY_free(pkey);
				free(rec_hash);
	
				/* END   - check the signature using OpenSSL */
#endif /* OPENSSL_GPL_VIOLATION */
			}

			gcry_md_close(hm);

			gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(hm, skeyid, s->ike.md_len);
			gcry_md_write(hm, dh_public, dh_getlen(dh_grp));
			gcry_md_write(hm, ke->u.ke.data, ke->u.ke.length);
			gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, sa_f + 4, sa_size - 4);
			gcry_md_write(hm, idi_f + 4, idi_size - 4);
			gcry_md_final(hm);
			returned_hash = xallocc(s->ike.md_len);
			memcpy(returned_hash, gcry_md_read(hm, 0), s->ike.md_len);
			gcry_md_close(hm);
			hex_dump("returned_hash", returned_hash, s->ike.md_len, NULL);

			/* PRESHARED_KEY_HASH */
			gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(hm, skeyid, s->ike.md_len);
			gcry_md_write(hm, shared_key, strlen(shared_key));
			gcry_md_final(hm);
			psk_hash = xallocc(s->ike.md_len);
			memcpy(psk_hash, gcry_md_read(hm, 0), s->ike.md_len);
			gcry_md_close(hm);
			hex_dump("psk_hash", psk_hash, s->ike.md_len, NULL);
			/* End PRESHARED_KEY_HASH */

			free(sa_f);
			free(idi_f);
			free(idp_f);
		}

		/* Determine all the SKEYID_x keys.  */
		{
			gcry_md_hd_t hm;
			int i;
			static const unsigned char c012[3] = { 0, 1, 2 };
			unsigned char *skeyid_e;
			unsigned char *dh_shared_secret;

			/* Determine the shared secret.  */
			dh_shared_secret = xallocc(dh_getlen(dh_grp));
			dh_create_shared(dh_grp, dh_shared_secret, ke->u.ke.data);
			hex_dump("dh_shared_secret", dh_shared_secret, dh_getlen(dh_grp), NULL);

			gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(hm, skeyid, s->ike.md_len);
			gcry_md_write(hm, dh_shared_secret, dh_getlen(dh_grp));
			gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, c012 + 0, 1);
			gcry_md_final(hm);
			if (s->ike.skeyid_d) free(s->ike.skeyid_d);
			s->ike.skeyid_d = xallocc(s->ike.md_len);
			memcpy(s->ike.skeyid_d, gcry_md_read(hm, 0), s->ike.md_len);
			gcry_md_close(hm);
			hex_dump("skeyid_d", s->ike.skeyid_d, s->ike.md_len, NULL);

			gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(hm, skeyid, s->ike.md_len);
			gcry_md_write(hm, s->ike.skeyid_d, s->ike.md_len);
			gcry_md_write(hm, dh_shared_secret, dh_getlen(dh_grp));
			gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, c012 + 1, 1);
			gcry_md_final(hm);
			if (s->ike.skeyid_a) free(s->ike.skeyid_a);
			s->ike.skeyid_a = xallocc(s->ike.md_len);
			memcpy(s->ike.skeyid_a, gcry_md_read(hm, 0), s->ike.md_len);
			gcry_md_close(hm);
			hex_dump("skeyid_a", s->ike.skeyid_a, s->ike.md_len, NULL);

			gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(hm, skeyid, s->ike.md_len);
			gcry_md_write(hm, s->ike.skeyid_a, s->ike.md_len);
			gcry_md_write(hm, dh_shared_secret, dh_getlen(dh_grp));
			gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
			gcry_md_write(hm, c012 + 2, 1);
			gcry_md_final(hm);
			skeyid_e = xallocc(s->ike.md_len);
			memcpy(skeyid_e, gcry_md_read(hm, 0), s->ike.md_len);
			gcry_md_close(hm);
			hex_dump("skeyid_e", skeyid_e, s->ike.md_len, NULL);

			memset(dh_shared_secret, 0, sizeof(dh_shared_secret));
			free(dh_shared_secret);

			/* Determine the IKE encryption key.  */
			if (s->ike.key) free(s->ike.key);
			s->ike.key = xallocc(s->ike.keylen);

			if (s->ike.keylen > s->ike.md_len) {
				for (i = 0; i * s->ike.md_len < s->ike.keylen; i++) {
					gcry_md_open(&hm, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
					gcry_md_setkey(hm, skeyid_e, s->ike.md_len);
					if (i == 0)
						gcry_md_write(hm, "" /* &'\0' */ , 1);
					else
						gcry_md_write(hm, s->ike.key + (i - 1) * s->ike.md_len,
							s->ike.md_len);
					gcry_md_final(hm);
					memcpy(s->ike.key + i * s->ike.md_len, gcry_md_read(hm, 0),
						min(s->ike.md_len, s->ike.keylen - i * s->ike.md_len));
					gcry_md_close(hm);
				}
			} else { /* keylen <= md_len */
				memcpy(s->ike.key, skeyid_e, s->ike.keylen);
			}
			hex_dump("enc-key", s->ike.key, s->ike.keylen, NULL);

			memset(skeyid_e, 0, s->ike.md_len);
			free(skeyid_e);
		}

		/* Determine the initial IV.  */
		{
			gcry_md_hd_t hm;

			assert(s->ike.ivlen <= s->ike.md_len);
			gcry_md_open(&hm, s->ike.md_algo, 0);
			gcry_md_write(hm, dh_public, dh_getlen(dh_grp));
			gcry_md_write(hm, ke->u.ke.data, ke->u.ke.length);
			gcry_md_final(hm);
			if (s->ike.current_iv) free(s->ike.current_iv);
			s->ike.current_iv = xallocc(s->ike.ivlen);
			memcpy(s->ike.current_iv, gcry_md_read(hm, 0), s->ike.ivlen);
			gcry_md_close(hm);
			hex_dump("current_iv", s->ike.current_iv, s->ike.ivlen, NULL);
			memset(s->ike.current_iv_msgid, 0, 4);
		}

		gcry_md_close(skeyid_ctx);
	}

	DEBUGTOP(2, printf("S4.5 AM_packet3\n"));
	/* Send final phase 1 packet.  */
	{
		struct isakmp_packet *p2;
		uint8_t *p2kt;
		size_t p2kt_len;
		struct isakmp_payload *pl;
#if 0 /* cert support */
#ifdef OPENSSL_GPL_VIOLATION
		struct isakmp_payload *last_cert = NULL;
		struct isakmp_payload *sig = NULL;


		X509 *current_cert;
		/* structure to store the certificate chain */
		STACK_OF(X509) *cert_stack = sk_X509_new_null();
#endif /* OPENSSL_GPL_VIOLATION */
#endif /* 0 */

		p2 = new_isakmp_packet();
		memcpy(p2->i_cookie, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
		memcpy(p2->r_cookie, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
		p2->flags = ISAKMP_FLAG_E;
		p2->isakmp_version = ISAKMP_VERSION;
		p2->exchange_type = ISAKMP_EXCHANGE_AGGRESSIVE;
	/* XXX CERT Add id(?), cert and sig here in case of cert auth */
		p2->payload = new_isakmp_data_payload(ISAKMP_PAYLOAD_HASH,
			returned_hash, s->ike.md_len);
		p2->payload->next = pl = new_isakmp_payload(ISAKMP_PAYLOAD_N);
		pl->u.n.doi = ISAKMP_DOI_IPSEC;
		pl->u.n.protocol = ISAKMP_IPSEC_PROTO_ISAKMP;
		pl->u.n.type = ISAKMP_N_IPSEC_INITIAL_CONTACT;
		pl->u.n.spi_length = 2 * ISAKMP_COOKIE_LENGTH;
		pl->u.n.spi = xallocc(2 * ISAKMP_COOKIE_LENGTH);
		memcpy(pl->u.n.spi + ISAKMP_COOKIE_LENGTH * 0, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
		memcpy(pl->u.n.spi + ISAKMP_COOKIE_LENGTH * 1, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);

		/* send PSK-hash if hybrid authentication is negotiated */
		if (s->ike.auth_algo == IKE_AUTH_HybridInitRSA ||
			s->ike.auth_algo == IKE_AUTH_HybridInitDSS) {
			/* Notify - PRESHARED_KEY_HASH */
			pl = pl->next = new_isakmp_payload(ISAKMP_PAYLOAD_N);
			pl->u.n.doi = ISAKMP_DOI_IPSEC;
			pl->u.n.protocol = ISAKMP_IPSEC_PROTO_ISAKMP;
			/* Notify Message - Type: PRESHARED_KEY_HASH */
			pl->u.n.type =  ISAKMP_N_CISCO_PRESHARED_KEY_HASH;
			pl->u.n.spi_length = 2 * ISAKMP_COOKIE_LENGTH;
			pl->u.n.spi = xallocc(2 * ISAKMP_COOKIE_LENGTH);
			memcpy(pl->u.n.spi + ISAKMP_COOKIE_LENGTH * 0,
				s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
			memcpy(pl->u.n.spi + ISAKMP_COOKIE_LENGTH * 1,
				s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
			pl->u.n.data_length = s->ike.md_len;
			pl->u.n.data = xallocc(pl->u.n.data_length);
			memcpy(pl->u.n.data, psk_hash, pl->u.n.data_length);
			/* End Notify - PRESHARED_KEY_HASH */
		}
		pl = pl->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			VID_UNKNOWN, sizeof(VID_UNKNOWN));
		pl = pl->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			VID_UNITY, sizeof(VID_UNITY));

		/* include NAT traversal discovery payloads */
		if (seen_natt_vid) {
			assert(natd_type != 0);
			pl = pl->next = new_isakmp_data_payload(natd_type,
				natd_them, s->ike.md_len);
			/* this could be repeated fo any known outbound interfaces */
			{
				gcry_md_hd_t hm;
				uint16_t n_src_port = htons(s->ike.src_port);
				
				gcry_md_open(&hm, s->ike.md_algo, 0);
				gcry_md_write(hm, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
				gcry_md_write(hm, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
				gcry_md_write(hm, &s->src, sizeof(struct in_addr));
				gcry_md_write(hm, &n_src_port, sizeof(uint16_t));
				gcry_md_final(hm);
				pl = pl->next = new_isakmp_data_payload(natd_type,
					gcry_md_read(hm, 0), s->ike.md_len);
				if (opt_natt_mode == NATT_FORCE) /* force detection of "this end behind NAT" */
					pl->u.ke.data[0] ^= 1; /* by flipping a bit in the nat-detection-hash */
				if (seen_natd && memcmp(natd_us, pl->u.ke.data, s->ike.md_len) == 0)
					seen_natd_us = 1;
				gcry_md_close(hm);
			}
			if (seen_natd) {
				free(natd_us);
				free(natd_them);
			}
			/* if there is a NAT, change to port 4500 and select UDP encap */
			if (!seen_natd_us || !seen_natd_them) {
				DEBUG(1, printf("NAT status: this end behind NAT? %s -- remote end behind NAT? %s\n",
					seen_natd_us ? "no" : "YES", seen_natd_them ? "no" : "YES"));
				switch (natd_type) {
					case ISAKMP_PAYLOAD_NAT_D:
						s->ipsec.encap_mode = IPSEC_ENCAP_UDP_TUNNEL;
						break;
					case ISAKMP_PAYLOAD_NAT_D_OLD:
						s->ipsec.encap_mode = IPSEC_ENCAP_UDP_TUNNEL_OLD;
						break;
					default:
						abort();
				}
				if (natt_draft >= 2) {
					s->ipsec.natt_active_mode = NATT_ACTIVE_RFC;
					close(s->ike_fd);
					if (s->ike.src_port == ISAKMP_PORT)
						s->ike.src_port = ISAKMP_PORT_NATT;
					s->ike_fd = make_socket(s, s->ike.src_port, s->ike.dst_port = ISAKMP_PORT_NATT);
				} else {
					s->ipsec.natt_active_mode = NATT_ACTIVE_DRAFT_OLD;
				}
			} else {
				DEBUG(1, printf("NAT status: NAT-T VID seen, no NAT device detected\n"));
			}
		} else {
			DEBUG(1, printf("NAT status: no NAT-T VID seen\n"));
		}

		
		flatten_isakmp_packet(p2, &p2kt, &p2kt_len, s->ike.ivlen);
		free_isakmp_packet(p2);
		isakmp_crypt(s, p2kt, p2kt_len, 1);

		if (s->ike.initial_iv) free(s->ike.initial_iv);
		s->ike.initial_iv = xallocc(s->ike.ivlen);
		memcpy(s->ike.initial_iv, s->ike.current_iv, s->ike.ivlen);
		hex_dump("initial_iv", s->ike.initial_iv, s->ike.ivlen, NULL);

		/* Now, send that packet and receive a new one.  */
		r_length = sendrecv(s, r_packet, sizeof(r_packet), p2kt, p2kt_len, 0);
		free(p2kt);
	}
	DEBUGTOP(2, printf("S4.6 cleanup\n"));

	free_isakmp_packet(p1);
	/* This seems to cause a duplicate free of some data:
	 * *** glibc detected *** vpnc-connect: free(): invalid pointer: 0x09d63ba5
	 * See also: http://bugs.gentoo.org/show_bug.cgi?id=229003
	 */
#if 0
	free_isakmp_packet(r);
#endif
	free(returned_hash);
	free(dh_public);
	free(dh_shared_secret);
	free(psk_hash);
	group_free(dh_grp);
}

static int do_phase2_notice_check(struct sa_block *s, struct isakmp_packet **r_p)
{
	int reject = 0;
	struct isakmp_packet *r;
	
	while (1) {
		reject = unpack_verify_phase2(s, r_packet, r_length, r_p, NULL, 0);
		if (reject == ISAKMP_N_INVALID_COOKIE) {
			r_length = sendrecv(s, r_packet, sizeof(r_packet), NULL, 0, 0);
			continue;
		}
		if (*r_p == NULL) {
			assert(reject != 0);
			return reject;
		}
		r = *r_p;
		
		/* check for notices */
		if (r->exchange_type == ISAKMP_EXCHANGE_INFORMATIONAL &&
			r->payload->next != NULL) {
			if (r->payload->next->type == ISAKMP_PAYLOAD_N) {
				if (r->payload->next->u.n.type == ISAKMP_N_CISCO_LOAD_BALANCE) {
					/* load balancing notice ==> restart with new gw */
					if (r->payload->next->u.n.data_length != 4)
						error(1, 0, "malformed loadbalance target");
					s->dst = *(struct in_addr *)r->payload->next->u.n.data;
					s->ike.dst_port = ISAKMP_PORT;
					s->ipsec.encap_mode = IPSEC_ENCAP_TUNNEL;
					s->ipsec.natt_active_mode = NATT_ACTIVE_NONE;
					if (s->ike.src_port == ISAKMP_PORT_NATT)
						s->ike.src_port = ISAKMP_PORT;
					close(s->ike_fd);
					s->ike_fd = make_socket(s, s->ike.src_port, s->ike.dst_port);
					DEBUG(2, printf("got cisco loadbalancing notice, diverting to %s\n",
							inet_ntoa(s->dst)));
					return -1;
				} else if (r->payload->next->u.n.type == ISAKMP_N_IPSEC_RESPONDER_LIFETIME) {
					if (r->payload->next->u.n.protocol == ISAKMP_IPSEC_PROTO_ISAKMP)
						lifetime_ike_process(s, r->payload->next->u.n.attributes);
					else if (r->payload->next->u.n.protocol == ISAKMP_IPSEC_PROTO_IPSEC_ESP)
						lifetime_ipsec_process(s, r->payload->next->u.n.attributes);
					else
						DEBUG(2, printf("got unknown lifetime notice, ignoring..\n"));
					r_length = sendrecv(s, r_packet, sizeof(r_packet), NULL, 0, 0);
					continue;
				} else if (r->payload->next->u.n.type == ISAKMP_N_IPSEC_INITIAL_CONTACT) {
					/* why in hell do we get this?? */
					DEBUG(2, printf("got initial contact notice, ignoring..\n"));
					r_length = sendrecv(s, r_packet, sizeof(r_packet), NULL, 0, 0);
					continue;
				} else {
					/* whatever */
					printf("received notice of type %s(%d), giving up\n",
						val_to_string(r->payload->next->u.n.type, isakmp_notify_enum_array),
						r->payload->next->u.n.type);
					return reject;
				}
			}
			if (r->payload->next->type == ISAKMP_PAYLOAD_D) {
				/* delete notice ==> ignore */
				DEBUG(2, printf("got delete for old connection, ignoring..\n"));
				r_length = sendrecv(s, r_packet, sizeof(r_packet), NULL, 0, 0);
				continue;
			}
		}
		
		break;
	}
	return reject;
}

static int do_phase2_xauth(struct sa_block *s)
{
	struct isakmp_packet *r = NULL;
	int loopcount;
	int reject;
	int passwd_used = 0;

	DEBUGTOP(2, printf("S5.1 xauth_start\n"));
	/* This can go around for a while.  */
	for (loopcount = 0;; loopcount++) {
		struct isakmp_payload *rp;
		struct isakmp_attribute *a, *ap, *reply_attr;
		char ntop_buf[32];
		int seen_answer = 0;

		DEBUGTOP(2, printf("S5.2 notice_check\n"));
		
		/* recv and check for notices */
		if (r) free_isakmp_packet(r);
		r = NULL;
		reject = do_phase2_notice_check(s, &r);
		if (reject == -1) {
			if (r) free_isakmp_packet(r);
			return 1;
		}
		
		DEBUGTOP(2, printf("S5.3 type-is-xauth check\n"));
		/* Check the transaction type is OK.  */
		if (reject == 0 && r->exchange_type != ISAKMP_EXCHANGE_MODECFG_TRANSACTION)
			reject = ISAKMP_N_INVALID_EXCHANGE_TYPE;

		/* After the hash, expect an attribute block.  */
		if (reject == 0
			&& (r->payload->next == NULL
				|| r->payload->next->next != NULL
				|| r->payload->next->type != ISAKMP_PAYLOAD_MODECFG_ATTR))
			reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;

		if (reject == 0 && r->payload->next->u.modecfg.type == ISAKMP_MODECFG_CFG_SET)
			break;
		if (reject == 0 && r->payload->next->u.modecfg.type != ISAKMP_MODECFG_CFG_REQUEST)
			reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;

		if (reject != 0)
			phase2_fatal(s, "expected xauth packet; rejected: %s(%d)", reject);

		DEBUGTOP(2, printf("S5.4 xauth type check\n"));
		a = r->payload->next->u.modecfg.attributes;
		/* First, print any messages, and verify that we understand the
		 * conversation. This looks for any place were input is
		 * required - in these cases, we need to print the prompt
		 * regardless of whether the user requested interactive mode
		 * or not. */
		for (ap = a; ap && seen_answer == 0; ap = ap->next)
			if (ap->type == ISAKMP_XAUTH_06_ATTRIB_ANSWER
			    || ap->type == ISAKMP_XAUTH_06_ATTRIB_NEXT_PIN
			    /* || ap->type == ISAKMP_XAUTH_06_ATTRIB_PASSCODE */)
				seen_answer = 1;

		for (ap = a; ap && reject == 0; ap = ap->next)
			switch (ap->type) {
			case ISAKMP_XAUTH_06_ATTRIB_TYPE:
				if (ap->af != isakmp_attr_16 || ap->u.attr_16 != 0)
					reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
				break;
			case ISAKMP_XAUTH_06_ATTRIB_USER_NAME:
			case ISAKMP_XAUTH_06_ATTRIB_USER_PASSWORD:
			case ISAKMP_XAUTH_06_ATTRIB_PASSCODE:
			case ISAKMP_XAUTH_06_ATTRIB_DOMAIN:
			case ISAKMP_XAUTH_06_ATTRIB_ANSWER:
			case ISAKMP_XAUTH_06_ATTRIB_NEXT_PIN:
			case ISAKMP_XAUTH_ATTRIB_CISCOEXT_VENDOR:
				break;
			case ISAKMP_XAUTH_06_ATTRIB_MESSAGE:
				if (opt_debug || seen_answer || config[CONFIG_XAUTH_INTERACTIVE]) {
					if (ap->af == isakmp_attr_16)
						printf("%c%c\n", ap->u.attr_16 >> 8, ap->u.attr_16);
					else
						printf("%.*s%s", ap->u.lots.length, ap->u.lots.data,
							((ap->u.lots.data
									&& ap->u.lots.data[ap->u.
										lots.length - 1] !=
									'\n')
								? "\n" : ""));
				}
				break;
			default:
				reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			}
		if (reject != 0)
			phase2_fatal(s, "xauth packet unsupported: %s(%d)", reject);

		DEBUGTOP(2, printf("S5.5 do xauth authentication\n"));
		inet_ntop(AF_INET, &s->dst, ntop_buf, sizeof(ntop_buf));

		/* Collect data from the user.  */
		reply_attr = NULL;
		for (ap = a; ap && reject == 0; ap = ap->next)
			switch (ap->type) {
			case ISAKMP_XAUTH_06_ATTRIB_DOMAIN:
				{
					struct isakmp_attribute *na;
					na = new_isakmp_attribute(ap->type, reply_attr);
					reply_attr = na;
					if (!config[CONFIG_DOMAIN])
						error(1, 0,
							"server requested domain, but none set (use \"Domain ...\" in config or --domain");
					na->u.lots.length = strlen(config[CONFIG_DOMAIN]);
					na->u.lots.data = xallocc(na->u.lots.length);
					memcpy(na->u.lots.data, config[CONFIG_DOMAIN],
						na->u.lots.length);
					break;
				}
			case ISAKMP_XAUTH_06_ATTRIB_USER_NAME:
				{
					struct isakmp_attribute *na;
					na = new_isakmp_attribute(ap->type, reply_attr);
					reply_attr = na;
					na->u.lots.length = strlen(config[CONFIG_XAUTH_USERNAME]);
					na->u.lots.data = xallocc(na->u.lots.length);
					memcpy(na->u.lots.data, config[CONFIG_XAUTH_USERNAME],
						na->u.lots.length);
					break;
				}
			case ISAKMP_XAUTH_06_ATTRIB_ANSWER:
			case ISAKMP_XAUTH_06_ATTRIB_USER_PASSWORD:
			case ISAKMP_XAUTH_06_ATTRIB_PASSCODE:
			case ISAKMP_XAUTH_06_ATTRIB_NEXT_PIN:
				if (passwd_used && config[CONFIG_NON_INTERACTIVE]) {
					reject = ISAKMP_N_AUTHENTICATION_FAILED;
					phase2_fatal(s, "noninteractive can't reuse password", reject);
					error(2, 0, "authentication unsuccessful");
				} else if (seen_answer || passwd_used || config[CONFIG_XAUTH_INTERACTIVE]) {
					char *pass, *prompt = NULL;
					struct isakmp_attribute *na;

					asprintf(&prompt, "%s for VPN %s@%s: ",
						(ap->type == ISAKMP_XAUTH_06_ATTRIB_ANSWER) ?
						"Answer" :
						(ap->type == ISAKMP_XAUTH_06_ATTRIB_USER_PASSWORD) ?
						"Password" : "Passcode",
						config[CONFIG_XAUTH_USERNAME], ntop_buf);
					pass = getpass(prompt);
					free(prompt);

					na = new_isakmp_attribute(ap->type, reply_attr);
					reply_attr = na;
					na->u.lots.length = strlen(pass);
					na->u.lots.data = xallocc(na->u.lots.length);
					memcpy(na->u.lots.data, pass, na->u.lots.length);
					memset(pass, 0, na->u.lots.length);
				} else {
					struct isakmp_attribute *na;
					na = new_isakmp_attribute(ap->type, reply_attr);
					reply_attr = na;
					na->u.lots.length = strlen(config[CONFIG_XAUTH_PASSWORD]);
					na->u.lots.data = xallocc(na->u.lots.length);
					memcpy(na->u.lots.data, config[CONFIG_XAUTH_PASSWORD],
						na->u.lots.length);
					passwd_used = 1; /* Provide canned password at most once */
				}
				break;
			default:
				;
			}

		/* Send the response.  */
		rp = new_isakmp_payload(ISAKMP_PAYLOAD_MODECFG_ATTR);
		rp->u.modecfg.type = ISAKMP_MODECFG_CFG_REPLY;
		rp->u.modecfg.id = r->payload->next->u.modecfg.id;
		rp->u.modecfg.attributes = reply_attr;
		sendrecv_phase2(s, rp, ISAKMP_EXCHANGE_MODECFG_TRANSACTION,
			r->message_id, 0, 0, 0, 0, 0, 0, 0);

	}
	
	if ((opt_vendor == VENDOR_NETSCREEN) &&
		(r->payload->next->u.modecfg.type == ISAKMP_MODECFG_CFG_SET)) {
		struct isakmp_attribute *a = r->payload->next->u.modecfg.attributes;
		
		DEBUGTOP(2, printf("S5.5.1 do netscreen modecfg extra\n"));
		
		do_config_to_env(s, a);
		
		for (; a; a = a->next)
			if(a->af == isakmp_attr_lots)
				a->u.lots.length = 0;

		r->payload->next->u.modecfg.type = ISAKMP_MODECFG_CFG_ACK;
		sendrecv_phase2(s, r->payload->next,
			ISAKMP_EXCHANGE_MODECFG_TRANSACTION,
			r->message_id, 0, 0, 0, 0, 0, 0, 0);
		
		reject = do_phase2_notice_check(s, &r);
		if (reject == -1) {
			free_isakmp_packet(r);
			return 1;
		}
	}
	
	DEBUGTOP(2, printf("S5.6 process xauth response\n"));
	{
		/* The final SET should have just one attribute.  */
		struct isakmp_attribute *a = r->payload->next->u.modecfg.attributes;
		uint16_t set_result = 1;

		if (a == NULL
			|| a->type != ISAKMP_XAUTH_06_ATTRIB_STATUS
			|| a->af != isakmp_attr_16 || a->next != NULL) {
			reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;
			phase2_fatal(s, "xauth SET response rejected: %s(%d)", reject);
		} else {
			set_result = a->u.attr_16;
		}

		/* ACK the SET.  */
		r->payload->next->u.modecfg.type = ISAKMP_MODECFG_CFG_ACK;
		sendrecv_phase2(s, r->payload->next, ISAKMP_EXCHANGE_MODECFG_TRANSACTION,
			r->message_id, 1, 0, 0, 0, 0, 0, 0);
		r->payload->next = NULL;
		free_isakmp_packet(r);

		if (set_result == 0)
			error(2, 0, "authentication unsuccessful");
	}
	DEBUGTOP(2, printf("S5.7 xauth done\n"));
	return 0;
}

static int do_phase2_config(struct sa_block *s)
{
	struct isakmp_payload *rp;
	struct isakmp_attribute *a;
	struct isakmp_packet *r;
	struct utsname uts;
	uint32_t msgid;
	int reject;
	
	uname(&uts);

	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	if (msgid == 0)
		msgid = 1;

	rp = new_isakmp_payload(ISAKMP_PAYLOAD_MODECFG_ATTR);
	rp->u.modecfg.type = ISAKMP_MODECFG_CFG_REQUEST;
	rp->u.modecfg.id = 20;
	a = NULL;

	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_APPLICATION_VERSION, a);
	a->u.lots.length = strlen(config[CONFIG_VERSION]);
	a->u.lots.data = xallocc(a->u.lots.length);
	memcpy(a->u.lots.data, config[CONFIG_VERSION], a->u.lots.length);

	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_DDNS_HOSTNAME, a);
	a->u.lots.length = strlen(uts.nodename);
	a->u.lots.data = xallocc(a->u.lots.length);
	memcpy(a->u.lots.data, uts.nodename, a->u.lots.length);

	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_SPLIT_INC, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_SAVE_PW, a);
	
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_BANNER, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_DO_PFS, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_FW_TYPE, a);
	a->u.lots.length = sizeof(FW_UNKNOWN_TYPEINFO);
	a->u.lots.data = xallocc(a->u.lots.length);
	memcpy(a->u.lots.data, FW_UNKNOWN_TYPEINFO, a->u.lots.length);
	if (opt_natt_mode == NATT_CISCO_UDP)
		a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_UDP_ENCAP_PORT, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_DEF_DOMAIN, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_NBNS, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_DNS, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_NETMASK, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_ADDRESS, a);

	rp->u.modecfg.attributes = a;
	DEBUGTOP(2, printf("S6.1 phase2_config send modecfg\n"));
	sendrecv_phase2(s, rp, ISAKMP_EXCHANGE_MODECFG_TRANSACTION, msgid, 0, 0, 0, 0, 0, 0, 0);

	DEBUGTOP(2, printf("S6.2 phase2_config receive modecfg\n"));
	/* recv and check for notices */
	reject = do_phase2_notice_check(s, &r);
	if (reject == -1) {
		if (r) free_isakmp_packet(r);
		return 1;
	}
		
	/* Check the transaction type & message ID are OK.  */
	if (reject == 0 && r->message_id != msgid)
		reject = ISAKMP_N_INVALID_MESSAGE_ID;
	if (reject == 0 && r->exchange_type != ISAKMP_EXCHANGE_MODECFG_TRANSACTION)
		reject = ISAKMP_N_INVALID_EXCHANGE_TYPE;

	/* After the hash, expect an attribute block.  */
	if (reject == 0
		&& (r->payload->next == NULL
			|| r->payload->next->next != NULL
			|| r->payload->next->type != ISAKMP_PAYLOAD_MODECFG_ATTR
#if 0
			|| r->payload->next->u.modecfg.id != 20
#endif
			|| r->payload->next->u.modecfg.type != ISAKMP_MODECFG_CFG_REPLY))
		reject = ISAKMP_N_PAYLOAD_MALFORMED;

	if (reject != 0)
		phase2_fatal(s, "configuration response rejected: %s(%d)", reject);

	if (reject == 0)
		reject = do_config_to_env(s, r->payload->next->u.modecfg.attributes);
	
	if (reject != 0)
		phase2_fatal(s, "configuration response rejected: %s(%d)", reject);

	DEBUG(1, printf("got address %s\n", getenv("INTERNAL_IP4_ADDRESS")));
	free_isakmp_packet(r);
	return 0;
}

static struct isakmp_attribute *make_transform_ipsec(struct sa_block *s, int dh_group, int hash, int keylen)
{
	struct isakmp_attribute *a = NULL;

	a = new_isakmp_attribute(ISAKMP_IPSEC_ATTRIB_SA_LIFE_DURATION, a);
	a->af = isakmp_attr_lots;
	a->u.lots.length = 4;
	a->u.lots.data = xallocc(a->u.lots.length);
	*((uint32_t *) a->u.lots.data) = htonl(2147483);
	a = new_isakmp_attribute_16(ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE, IPSEC_LIFE_SECONDS, a);

	if (dh_group)
		a = new_isakmp_attribute_16(ISAKMP_IPSEC_ATTRIB_GROUP_DESC, dh_group, a);
	a = new_isakmp_attribute_16(ISAKMP_IPSEC_ATTRIB_AUTH_ALG, hash, a);
	a = new_isakmp_attribute_16(ISAKMP_IPSEC_ATTRIB_ENCAP_MODE, s->ipsec.encap_mode, a);
	if (keylen != 0)
		a = new_isakmp_attribute_16(ISAKMP_IPSEC_ATTRIB_KEY_LENGTH, keylen, a);

	return a;
}

static struct isakmp_payload *make_our_sa_ipsec(struct sa_block *s)
{
	struct isakmp_payload *r = new_isakmp_payload(ISAKMP_PAYLOAD_SA);
	struct isakmp_payload *p = NULL, *pn;
	struct isakmp_attribute *a;
	int dh_grp = get_dh_group_ipsec(s->ipsec.do_pfs)->ipsec_sa_id;
	unsigned int crypt, hash, keylen;
	int i;

	r = new_isakmp_payload(ISAKMP_PAYLOAD_SA);
	r->u.sa.doi = ISAKMP_DOI_IPSEC;
	r->u.sa.situation = ISAKMP_IPSEC_SIT_IDENTITY_ONLY;
	r->u.sa.proposals = new_isakmp_payload(ISAKMP_PAYLOAD_P);
	r->u.sa.proposals->u.p.spi_size = 4;
	r->u.sa.proposals->u.p.spi = xallocc(4);
	/* The sadb_sa_spi field is already in network order.  */
	memcpy(r->u.sa.proposals->u.p.spi, &s->ipsec.rx.spi, 4);
	r->u.sa.proposals->u.p.prot_id = ISAKMP_IPSEC_PROTO_IPSEC_ESP;
	for (crypt = 0; supp_crypt[crypt].name != NULL; crypt++) {
		keylen = supp_crypt[crypt].keylen;
		for (hash = 0; supp_hash[hash].name != NULL; hash++) {
			pn = p;
			p = new_isakmp_payload(ISAKMP_PAYLOAD_P);
			p->u.p.spi_size = 4;
			p->u.p.spi = xallocc(4);
			/* The sadb_sa_spi field is already in network order.  */
			memcpy(p->u.p.spi, &s->ipsec.rx.spi, 4);
			p->u.p.prot_id = ISAKMP_IPSEC_PROTO_IPSEC_ESP;
			p->u.p.transforms = new_isakmp_payload(ISAKMP_PAYLOAD_T);
			p->u.p.transforms->u.t.id = supp_crypt[crypt].ipsec_sa_id;
			a = make_transform_ipsec(s, dh_grp, supp_hash[hash].ipsec_sa_id, keylen);
			p->u.p.transforms->u.t.attributes = a;
			p->next = pn;
		}
	}
	for (i = 0, pn = p; pn; pn = pn->next)
		pn->u.p.number = i++;
	r->u.sa.proposals = p;
	return r;
}

static void do_phase2_qm(struct sa_block *s)
{
	struct isakmp_payload *rp, *us, *ke = NULL, *them, *nonce_r = NULL;
	struct isakmp_packet *r;
	struct group *dh_grp = NULL;
	uint32_t msgid;
	int reject;
	uint8_t *p_flat = NULL, *realiv = NULL, realiv_msgid[4];
	size_t p_size = 0;
	uint8_t nonce_i[20], *dh_public = NULL;
	int i;

	DEBUGTOP(2, printf("S7.1 QM_packet1\n"));
	/* Set up the Diffie-Hellman stuff.  */
	if (get_dh_group_ipsec(s->ipsec.do_pfs)->my_id) {
		dh_grp = group_get(get_dh_group_ipsec(s->ipsec.do_pfs)->my_id);
		DEBUG(3, printf("len = %d\n", dh_getlen(dh_grp)));
		dh_public = xallocc(dh_getlen(dh_grp));
		dh_create_exchange(dh_grp, dh_public);
		hex_dump("dh_public", dh_public, dh_getlen(dh_grp), NULL);
	}

	gcry_create_nonce((uint8_t *) & s->ipsec.rx.spi, sizeof(s->ipsec.rx.spi));
	rp = make_our_sa_ipsec(s); /* FIXME: LEAK: allocated memory never freed */
	gcry_create_nonce((uint8_t *) nonce_i, sizeof(nonce_i));
	rp->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_NONCE, nonce_i, sizeof(nonce_i));

	us = new_isakmp_payload(ISAKMP_PAYLOAD_ID);
	us->u.id.type = ISAKMP_IPSEC_ID_IPV4_ADDR;
	us->u.id.length = 4;
	us->u.id.data = xallocc(4);
	memcpy(us->u.id.data, s->our_address, sizeof(struct in_addr));
	them = new_isakmp_payload(ISAKMP_PAYLOAD_ID);
	them->u.id.type = ISAKMP_IPSEC_ID_IPV4_ADDR_SUBNET;
	them->u.id.length = 8;
	them->u.id.data = xallocc(8);
	init_netaddr((struct in_addr *)them->u.id.data,
		     config[CONFIG_IPSEC_TARGET_NETWORK]);
	us->next = them;
	s->ipsec.life.start = time(NULL);

	if (!dh_grp) {
		rp->next->next = us;
	} else {
		rp->next->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_KE,
			dh_public, dh_getlen(dh_grp));
		rp->next->next->next = us;
	}

	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	if (msgid == 0)
		msgid = 1;

	for (i = 0; i < 4; i++) {
		DEBUGTOP(2, printf("S7.2 QM_packet2 send_receive\n"));
		sendrecv_phase2(s, rp, ISAKMP_EXCHANGE_IKE_QUICK,
			msgid, 0, &p_flat, &p_size, 0, 0, 0, 0);

		if (realiv == NULL) {
			realiv = xallocc(s->ike.ivlen);
			memcpy(realiv, s->ike.current_iv, s->ike.ivlen);
			memcpy(realiv_msgid, s->ike.current_iv_msgid, 4);
		}

		DEBUGTOP(2, printf("S7.3 QM_packet2 validate type\n"));
		reject = unpack_verify_phase2(s, r_packet, r_length, &r, nonce_i, sizeof(nonce_i)); /* FIXME: LEAK */

		if (((reject == 0) || (reject == ISAKMP_N_AUTHENTICATION_FAILED))
			&& r->exchange_type == ISAKMP_EXCHANGE_INFORMATIONAL) {
			DEBUGTOP(2, printf("S7.4 process and skip lifetime notice\n"));
			/* handle notify responder-lifetime */
			/* (broken hash => ignore AUTHENTICATION_FAILED) */
			if (reject == 0 && r->payload->next->type != ISAKMP_PAYLOAD_N)
				reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;

			if (reject == 0
				&& r->payload->next->u.n.type == ISAKMP_N_IPSEC_RESPONDER_LIFETIME) {
				if (r->payload->next->u.n.protocol == ISAKMP_IPSEC_PROTO_ISAKMP)
					lifetime_ike_process(s, r->payload->next->u.n.attributes);
				else if (r->payload->next->u.n.protocol == ISAKMP_IPSEC_PROTO_IPSEC_ESP)
					lifetime_ipsec_process(s, r->payload->next->u.n.attributes);
				else
					DEBUG(2, printf("got unknown lifetime notice, ignoring..\n"));
				memcpy(s->ike.current_iv, realiv, s->ike.ivlen);
				memcpy(s->ike.current_iv_msgid, realiv_msgid, 4);
				continue;
			}
		}

		/* Check the transaction type & message ID are OK.  */
		if (reject == 0 && r->message_id != msgid)
			reject = ISAKMP_N_INVALID_MESSAGE_ID;

		if (reject == 0 && r->exchange_type != ISAKMP_EXCHANGE_IKE_QUICK)
			reject = ISAKMP_N_INVALID_EXCHANGE_TYPE;

		/* The SA payload must be second.  */
		if (reject == 0 && r->payload->next->type != ISAKMP_PAYLOAD_SA)
			reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;

		free(p_flat);
		free(realiv);

		break;
	}

	DEBUGTOP(2, printf("S7.5 QM_packet2 check reject offer\n"));
	if (reject != 0)
		phase2_fatal(s, "quick mode response rejected: %s(%d)\n"
			"this means the concentrator did not like what we had to offer.\n"
			"Possible reasons are:\n"
			"  * concentrator configured to require a firewall\n"
			"     this locks out even Cisco clients on any platform expect windows\n"
			"     which is an obvious security improvment. There is no workaround (yet).\n"
			"  * concentrator configured to require IP compression\n"
			"     this is not yet supported by vpnc.\n"
			"     Note: the Cisco Concentrator Documentation recommends against using\n"
			"     compression, expect on low-bandwith (read: ISDN) links, because it\n"
			"     uses much CPU-resources on the concentrator\n",
			reject);

	DEBUGTOP(2, printf("S7.6 QM_packet2 check and process proposal\n"));
	for (rp = r->payload->next; rp && reject == 0; rp = rp->next)
		switch (rp->type) {
		case ISAKMP_PAYLOAD_SA:
			if (reject == 0 && rp->u.sa.doi != ISAKMP_DOI_IPSEC)
				reject = ISAKMP_N_DOI_NOT_SUPPORTED;
			if (reject == 0 && rp->u.sa.situation != ISAKMP_IPSEC_SIT_IDENTITY_ONLY)
				reject = ISAKMP_N_SITUATION_NOT_SUPPORTED;
			if (reject == 0 &&
				(rp->u.sa.proposals == NULL || rp->u.sa.proposals->next != NULL))
				reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			if (reject == 0 &&
				rp->u.sa.proposals->u.p.prot_id != ISAKMP_IPSEC_PROTO_IPSEC_ESP)
				reject = ISAKMP_N_INVALID_PROTOCOL_ID;
			if (reject == 0 && rp->u.sa.proposals->u.p.spi_size != 4)
				reject = ISAKMP_N_INVALID_SPI;
			if (reject == 0 &&
				(rp->u.sa.proposals->u.p.transforms == NULL
					|| rp->u.sa.proposals->u.p.transforms->next != NULL))
				reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			if (reject == 0) {
				struct isakmp_attribute *a
					= rp->u.sa.proposals->u.p.transforms->u.t.attributes;
				int seen_enc = rp->u.sa.proposals->u.p.transforms->u.t.id;
				int seen_auth = 0, seen_encap = 0, seen_group = 0, seen_keylen = 0;

				memcpy(&s->ipsec.tx.spi, rp->u.sa.proposals->u.p.spi, 4);

				for (; a && reject == 0; a = a->next)
					switch (a->type) {
					case ISAKMP_IPSEC_ATTRIB_AUTH_ALG:
						if (a->af == isakmp_attr_16)
							seen_auth = a->u.attr_16;
						else
							reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
						break;
					case ISAKMP_IPSEC_ATTRIB_ENCAP_MODE:
						if (a->af == isakmp_attr_16 &&
							a->u.attr_16 == s->ipsec.encap_mode)
							seen_encap = 1;
						else
							reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
						break;
					case ISAKMP_IPSEC_ATTRIB_GROUP_DESC:
						if (dh_grp &&
							a->af == isakmp_attr_16 &&
							a->u.attr_16 ==
							get_dh_group_ipsec(s->ipsec.do_pfs)->ipsec_sa_id)
							seen_group = 1;
						else
							reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
						break;
					case ISAKMP_IPSEC_ATTRIB_KEY_LENGTH:
						if (a->af == isakmp_attr_16)
							seen_keylen = a->u.attr_16;
						else
							reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
						break;
					case ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE:
						/* lifetime duration MUST follow lifetype attribute */
						if (a->next->type == ISAKMP_IPSEC_ATTRIB_SA_LIFE_DURATION) {
							lifetime_ipsec_process(s, a);
						} else
							reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
						break;
					case ISAKMP_IPSEC_ATTRIB_SA_LIFE_DURATION:
						/* already processed above in ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE: */
						break;
					default:
						reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
						break;
					}
				if (reject == 0 && (!seen_auth || !seen_encap ||
						(dh_grp && !seen_group)))
					reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;

				if (reject == 0
					&& get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IPSEC_SA, seen_auth,
						NULL, 0) == NULL)
					reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
				if (reject == 0
					&& get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IPSEC_SA, seen_enc,
						NULL, seen_keylen) == NULL)
					reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;

				if (reject == 0) {
					s->ipsec.cry_algo =
						get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IPSEC_SA,
						seen_enc, NULL, seen_keylen)->my_id;
					s->ipsec.md_algo =
						get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IPSEC_SA,
						seen_auth, NULL, 0)->my_id;
					if (s->ipsec.cry_algo) {
						gcry_cipher_algo_info(s->ipsec.cry_algo, GCRYCTL_GET_KEYLEN, NULL, &(s->ipsec.key_len));
						gcry_cipher_algo_info(s->ipsec.cry_algo, GCRYCTL_GET_BLKLEN, NULL, &(s->ipsec.blk_len));
						s->ipsec.iv_len = s->ipsec.blk_len;
					} else {
						s->ipsec.key_len = 0;
						s->ipsec.iv_len = 0;
						s->ipsec.blk_len = 8; /* seems to be this without encryption... */
					}
					s->ipsec.md_len = gcry_md_get_algo_dlen(s->ipsec.md_algo);
					DEBUG(1, printf("IPSEC SA selected %s-%s\n",
							get_algo(SUPP_ALGO_CRYPT,
								SUPP_ALGO_IPSEC_SA, seen_enc, NULL,
								seen_keylen)->name,
							get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IPSEC_SA,
								seen_auth, NULL, 0)->name));
					if (s->ipsec.cry_algo == GCRY_CIPHER_DES && !opt_1des) {
						error(1, 0, "peer selected (single) DES as \"encrytion\" method.\n"
							"This algorithm is considered to weak today\n"
							"If your vpn concentrator admin still insists on using DES\n"
							"use the \"--enable-1des\" option.\n");
					} else if (s->ipsec.cry_algo == GCRY_CIPHER_NONE && !opt_no_encryption) {
						error(1, 0, "peer selected NULL as \"encrytion\" method.\n"
							"This is _no_ encryption at all.\n"
							"Your traffic is still protected against modification with %s\n"
							"If your vpn concentrator admin still insists on not using encryption\n"
							"use the \"--enable-no-encryption\" option.\n",
							get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IPSEC_SA, seen_auth, NULL, 0)->name);
					}
				}
			}
			break;

		case ISAKMP_PAYLOAD_N:
			if (reject == 0 && rp->u.n.type == ISAKMP_N_IPSEC_RESPONDER_LIFETIME) {
				if (rp->u.n.protocol == ISAKMP_IPSEC_PROTO_ISAKMP)
					lifetime_ike_process(s, rp->u.n.attributes);
				else if (rp->u.n.protocol == ISAKMP_IPSEC_PROTO_IPSEC_ESP)
					lifetime_ipsec_process(s, rp->u.n.attributes);
				else
					DEBUG(2, printf("got unknown lifetime notice, ignoring..\n"));
			}
			break;
		case ISAKMP_PAYLOAD_ID:
			/* FIXME: Parse payload ID and add route-env in case of ipv4_prefix */
			break;
		case ISAKMP_PAYLOAD_KE:
			ke = rp;
			break;
		case ISAKMP_PAYLOAD_NONCE:
			nonce_r = rp;
			break;

		default:
			reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;
			break;
		}

	if (reject == 0 && nonce_r == NULL)
		reject = ISAKMP_N_INVALID_HASH_INFORMATION;
	if (reject == 0 && dh_grp && (ke == NULL || ke->u.ke.length != dh_getlen(dh_grp)))
		reject = ISAKMP_N_INVALID_KEY_INFORMATION;
	if (reject != 0)
		phase2_fatal(s, "quick mode response rejected [2]: %s(%d)", reject);

	/* send final packet */
	sendrecv_phase2(s, NULL, ISAKMP_EXCHANGE_IKE_QUICK,
		msgid, 1, 0, 0, nonce_i, sizeof(nonce_i),
		nonce_r->u.nonce.data, nonce_r->u.nonce.length);

	DEBUGTOP(2, printf("S7.7 QM_packet3 sent\n"));

	DEBUGTOP(2, printf("S7.8 setup ipsec tunnel\n"));
	{
		unsigned char *dh_shared_secret = NULL;

		if (dh_grp) {
			/* Determine the shared secret.  */
			dh_shared_secret = xallocc(dh_getlen(dh_grp));
			dh_create_shared(dh_grp, dh_shared_secret, ke->u.ke.data);
			hex_dump("dh_shared_secret", dh_shared_secret, dh_getlen(dh_grp), NULL);
		}
		
		s->ipsec.rx.key = gen_keymat(s, ISAKMP_IPSEC_PROTO_IPSEC_ESP, s->ipsec.rx.spi,
			dh_shared_secret, dh_grp ? dh_getlen(dh_grp) : 0,
			nonce_i, sizeof(nonce_i), nonce_r->u.nonce.data, nonce_r->u.nonce.length);
		
		s->ipsec.tx.key = gen_keymat(s, ISAKMP_IPSEC_PROTO_IPSEC_ESP, s->ipsec.tx.spi,
			dh_shared_secret, dh_grp ? dh_getlen(dh_grp) : 0,
			nonce_i, sizeof(nonce_i), nonce_r->u.nonce.data, nonce_r->u.nonce.length);
		
		if (dh_grp)
			group_free(dh_grp);
		free(dh_shared_secret);
		free_isakmp_packet(r);
		
		if ((opt_natt_mode == NATT_CISCO_UDP) && s->ipsec.peer_udpencap_port) {
			s->esp_fd = make_socket(s, opt_udpencapport, s->ipsec.peer_udpencap_port);
			s->ipsec.encap_mode = IPSEC_ENCAP_UDP_TUNNEL;
			s->ipsec.natt_active_mode = NATT_ACTIVE_CISCO_UDP;
		} else if (s->ipsec.encap_mode != IPSEC_ENCAP_TUNNEL) {
			s->esp_fd = s->ike_fd;
		} else {
#ifdef IP_HDRINCL
			int hincl = 1;
#endif
		
			s->esp_fd = socket(PF_INET, SOCK_RAW, IPPROTO_ESP);
			if (s->esp_fd == -1) {
				close_tunnel(s);
				error(1, errno, "Couldn't open socket of ESP. Maybe something registered ESP already.\nPlease try '--natt-mode force-natt' or disable whatever is using ESP.\nsocket(PF_INET, SOCK_RAW, IPPROTO_ESP)");
			}
#ifdef IP_HDRINCL
			if (setsockopt(s->esp_fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl)) == -1) {
				close_tunnel(s);
				error(1, errno, "setsockopt(esp_fd, IPPROTO_IP, IP_HDRINCL, 1)");
			}
#endif
		}
		
		s->ipsec.rx.seq_id = s->ipsec.tx.seq_id = 1;
	}
	free(dh_public);
}

static void send_delete_ipsec(struct sa_block *s)
{
	/* 2007-08-31 JKU/ZID: Sonicwall doesn't like the chained
	 * request but wants them split. Cisco does fine with it. */
	DEBUGTOP(2, printf("S7.10 send ipsec termination message\n"));
	{
		struct isakmp_payload *d_ipsec;
		uint8_t del_msgid;

		gcry_create_nonce((uint8_t *) & del_msgid, sizeof(del_msgid));
		d_ipsec = new_isakmp_payload(ISAKMP_PAYLOAD_D);
		d_ipsec->u.d.doi = ISAKMP_DOI_IPSEC;
		d_ipsec->u.d.protocol = ISAKMP_IPSEC_PROTO_IPSEC_ESP;
		d_ipsec->u.d.spi_length = 4;
		d_ipsec->u.d.num_spi = 2;
		d_ipsec->u.d.spi = xallocc(2 * sizeof(uint8_t *));
		d_ipsec->u.d.spi[0] = xallocc(d_ipsec->u.d.spi_length);
		memcpy(d_ipsec->u.d.spi[0], &s->ipsec.rx.spi, 4);
		d_ipsec->u.d.spi[1] = xallocc(d_ipsec->u.d.spi_length);
		memcpy(d_ipsec->u.d.spi[1], &s->ipsec.tx.spi, 4);
		sendrecv_phase2(s, d_ipsec, ISAKMP_EXCHANGE_INFORMATIONAL,
			del_msgid, 1, NULL, NULL,
			NULL, 0, NULL, 0);
	}
}

static void send_delete_isakmp(struct sa_block *s)
{
	DEBUGTOP(2, printf("S7.11 send isakmp termination message\n"));
	{
		struct isakmp_payload *d_isakmp;
		uint8_t del_msgid;

		gcry_create_nonce((uint8_t *) & del_msgid, sizeof(del_msgid));
		d_isakmp = new_isakmp_payload(ISAKMP_PAYLOAD_D);
		d_isakmp->u.d.doi = ISAKMP_DOI_IPSEC;
		d_isakmp->u.d.protocol = ISAKMP_IPSEC_PROTO_ISAKMP;
		d_isakmp->u.d.spi_length = 2 * ISAKMP_COOKIE_LENGTH;
		d_isakmp->u.d.num_spi = 1;
		d_isakmp->u.d.spi = xallocc(1 * sizeof(uint8_t *));
		d_isakmp->u.d.spi[0] = xallocc(2 * ISAKMP_COOKIE_LENGTH);
		memcpy(d_isakmp->u.d.spi[0] + ISAKMP_COOKIE_LENGTH * 0, s->ike.i_cookie,
			ISAKMP_COOKIE_LENGTH);
		memcpy(d_isakmp->u.d.spi[0] + ISAKMP_COOKIE_LENGTH * 1, s->ike.r_cookie,
			ISAKMP_COOKIE_LENGTH);
		sendrecv_phase2(s, d_isakmp, ISAKMP_EXCHANGE_INFORMATIONAL,
			del_msgid, 1, NULL, NULL,
			NULL, 0, NULL, 0);
	}
}

static int do_rekey(struct sa_block *s, struct isakmp_packet *r)
{
	struct isakmp_payload *rp, *ke = NULL, *nonce_i = NULL;
	struct isakmp_attribute *a;
	int seen_enc;
	int seen_auth = 0, seen_encap = 0, seen_group = 0, seen_keylen = 0;
	int nonce_i_copy_len;
	struct group *dh_grp = NULL;
	uint8_t nonce_r[20], *dh_public = NULL, *nonce_i_copy = NULL;
	unsigned char *dh_shared_secret = NULL;
	
	if (get_dh_group_ipsec(s->ipsec.do_pfs)->my_id) {
		dh_grp = group_get(get_dh_group_ipsec(s->ipsec.do_pfs)->my_id);
		DEBUG(3, printf("len = %d\n", dh_getlen(dh_grp)));
		dh_public = xallocc(dh_getlen(dh_grp));
		dh_create_exchange(dh_grp, dh_public);
		hex_dump("dh_public", dh_public, dh_getlen(dh_grp), NULL);
	}
	
	rp = r->payload->next;
	/* rp->type == ISAKMP_PAYLOAD_SA, verified by caller */
	
	if (rp->u.sa.doi != ISAKMP_DOI_IPSEC)
		return ISAKMP_N_DOI_NOT_SUPPORTED;
	if (rp->u.sa.situation != ISAKMP_IPSEC_SIT_IDENTITY_ONLY)
		return ISAKMP_N_SITUATION_NOT_SUPPORTED;
	if (rp->u.sa.proposals == NULL)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	if (rp->u.sa.proposals->u.p.prot_id != ISAKMP_IPSEC_PROTO_IPSEC_ESP)
		return ISAKMP_N_INVALID_PROTOCOL_ID;
	if (rp->u.sa.proposals->u.p.spi_size != 4)
		return ISAKMP_N_INVALID_SPI;
	if (rp->u.sa.proposals->u.p.transforms == NULL || rp->u.sa.proposals->u.p.transforms->next != NULL)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	
	seen_enc = rp->u.sa.proposals->u.p.transforms->u.t.id;
	
	memcpy(&s->ipsec.tx.spi, rp->u.sa.proposals->u.p.spi, 4);
	
	for (a = rp->u.sa.proposals->u.p.transforms->u.t.attributes; a; a = a->next)
		switch (a->type) {
		case ISAKMP_IPSEC_ATTRIB_AUTH_ALG:
			if (a->af == isakmp_attr_16)
				seen_auth = a->u.attr_16;
			else
				return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			break;
		case ISAKMP_IPSEC_ATTRIB_ENCAP_MODE:
			if (a->af == isakmp_attr_16 &&
				a->u.attr_16 == (
					(s->ipsec.natt_active_mode != NATT_ACTIVE_CISCO_UDP) ?
					s->ipsec.encap_mode :
					IPSEC_ENCAP_TUNNEL /* cisco-udp claims to use encap tunnel... */
				))
				seen_encap = 1;
			else
				return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			break;
		case ISAKMP_IPSEC_ATTRIB_GROUP_DESC:
			if (dh_grp && a->af == isakmp_attr_16 &&
				a->u.attr_16 == get_dh_group_ipsec(s->ipsec.do_pfs)->ipsec_sa_id)
				seen_group = 1;
			else
				return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			break;
		case ISAKMP_IPSEC_ATTRIB_KEY_LENGTH:
			if (a->af == isakmp_attr_16)
				seen_keylen = a->u.attr_16;
			else
				return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			break;
		case ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE:
			/* lifetime duration MUST follow lifetype attribute */
			if (a->next->type == ISAKMP_IPSEC_ATTRIB_SA_LIFE_DURATION) {
				lifetime_ipsec_process(s, a);
			} else
				return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			break;
		case ISAKMP_IPSEC_ATTRIB_SA_LIFE_DURATION:
			/* already processed above in ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE: */
			break;
		default:
			return ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
			break;
		}
	if (!seen_auth || !seen_encap || (dh_grp && !seen_group))
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	
	/* FIXME: Current code has a limitation that will cause problems if
	 * different algorithms are negotiated during re-keying
	 */
	if ((get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IPSEC_SA, seen_auth, NULL, 0) == NULL) || 
	    (get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IPSEC_SA, seen_enc, NULL, seen_keylen) == NULL)) {
		printf("\nFIXME: vpnc doesn't support change of algorightms during rekeying\n");
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	}
	
	/* we don't want to change ciphers during rekeying */
	if (s->ipsec.cry_algo != get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IPSEC_SA, seen_enc,  NULL, seen_keylen)->my_id)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	if (s->ipsec.md_algo  != get_algo(SUPP_ALGO_HASH,  SUPP_ALGO_IPSEC_SA, seen_auth, NULL, 0)->my_id)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	
	for (rp = rp->next; rp; rp = rp->next)
		switch (rp->type) {
		case ISAKMP_PAYLOAD_ID:
			/* FIXME: Parse payload ID and add route-env in case of ipv4_prefix */
			break;
		case ISAKMP_PAYLOAD_KE:
			ke = rp;
			break;
		case ISAKMP_PAYLOAD_NONCE:
			nonce_i = rp;
			break;
		default:
			return ISAKMP_N_INVALID_PAYLOAD_TYPE;
			break;
		}
	
	if ((dh_grp && ke == NULL) || nonce_i == NULL)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	
	DEBUG(3, printf("everything fine so far...\n"));
	gcry_create_nonce((uint8_t *) nonce_r, sizeof(nonce_r));
	gcry_create_nonce((uint8_t *) & s->ipsec.rx.spi, sizeof(s->ipsec.rx.spi));
	
	if (dh_grp) {
		/* Determine the shared secret.  */
		dh_shared_secret = xallocc(dh_getlen(dh_grp));
		dh_create_shared(dh_grp, dh_shared_secret, ke->u.ke.data);
		hex_dump("dh_shared_secret", dh_shared_secret, dh_getlen(dh_grp), NULL);
	}
	
	free(s->ipsec.rx.key);
	free(s->ipsec.tx.key);
		
	s->ipsec.rx.key = gen_keymat(s, ISAKMP_IPSEC_PROTO_IPSEC_ESP, s->ipsec.rx.spi,
		dh_shared_secret, dh_grp ? dh_getlen(dh_grp) : 0,
		nonce_i->u.nonce.data, nonce_i->u.nonce.length, nonce_r, sizeof(nonce_r));
	
	s->ipsec.tx.key = gen_keymat(s, ISAKMP_IPSEC_PROTO_IPSEC_ESP, s->ipsec.tx.spi,
		dh_shared_secret, dh_grp ? dh_getlen(dh_grp) : 0,
		nonce_i->u.nonce.data, nonce_i->u.nonce.length, nonce_r, sizeof(nonce_r));
	
	s->ipsec.rx.key_cry = s->ipsec.rx.key;
	s->ipsec.rx.key_md  = s->ipsec.rx.key + s->ipsec.key_len;
	s->ipsec.tx.key_cry = s->ipsec.tx.key;
	s->ipsec.tx.key_md  = s->ipsec.tx.key + s->ipsec.key_len;
	
	nonce_i_copy_len = nonce_i->u.nonce.length;
	nonce_i_copy = xallocc(nonce_i_copy_len);
	memcpy(nonce_i_copy, nonce_i->u.nonce.data, nonce_i_copy_len);
	
	s->ipsec.rx.seq_id = s->ipsec.tx.seq_id = 1;
	s->ipsec.life.start = time(NULL);
	s->ipsec.life.tx = 0;
	s->ipsec.life.rx = 0;
	
	if (s->ipsec.cry_algo) {
		gcry_cipher_setkey(s->ipsec.rx.cry_ctx, s->ipsec.rx.key_cry, s->ipsec.key_len);
		gcry_cipher_setkey(s->ipsec.tx.cry_ctx, s->ipsec.tx.key_cry, s->ipsec.key_len);
	}
	
	/* use request as template and just exchange some values */
	/* this overwrites data in nonce_i, ke! */
	rp = r->payload->next;
	/* SA, change the SPI */
	memcpy(rp->u.sa.proposals->u.p.spi, &s->ipsec.rx.spi, 4);
	
	for (rp = rp->next; rp; rp = rp->next)
		switch (rp->type) {
		case ISAKMP_PAYLOAD_ID:
			break;
		case ISAKMP_PAYLOAD_KE:
			memcpy(rp->u.ke.data, dh_public, dh_getlen(dh_grp));
			break;
		case ISAKMP_PAYLOAD_NONCE:
			memcpy(rp->u.nonce.data, nonce_r, sizeof(nonce_r));
			break;
		default:
			assert(0);
			break;
		}
	
	sendrecv_phase2(s, r->payload->next, ISAKMP_EXCHANGE_IKE_QUICK,
		r->message_id, 0, 0, 0, nonce_i_copy, nonce_i_copy_len, 0,0);
	unpack_verify_phase2(s, r_packet, r_length, &r, NULL, 0);
	free(nonce_i_copy);
	/* don't care about answer ... */
	
	return 0;
}

void process_late_ike(struct sa_block *s, uint8_t *r_packet, ssize_t r_length)
{
	int reject;
	struct isakmp_packet *r;
	struct isakmp_payload *rp;
	
	DEBUG(2,printf("got late ike paket: %zd bytes\n", r_length));
	/* we should ignore resent pakets here.
	 * unpack_verify_phase2 will fail to decode them probably */
	reject = unpack_verify_phase2(s, r_packet, r_length, &r, NULL, 0);
	
	/* just ignore broken stuff for now */
	if (reject != 0) {
		if (r) free_isakmp_packet(r);
		return;
	}
	
	/* everything must be encrypted by now */
	if (r->payload == NULL || r->payload->type != ISAKMP_PAYLOAD_HASH) {
		free_isakmp_packet(r);
		return;
	}
	
	/* empty packet? well, nothing to see here */
	if (r->payload->next == NULL) {
		free_isakmp_packet(r);
		return;
	}
	
	/* do we get an SA proposal for rekeying? */
	if (r->exchange_type == ISAKMP_EXCHANGE_IKE_QUICK &&
		r->payload->next->type == ISAKMP_PAYLOAD_SA) {
		reject = do_rekey(s, r);
		DEBUG(3, printf("do_rekey returned: %d\n", reject));
		/* FIXME: LEAK but will create segfault for double free */
		/* free_isakmp_packet(r); */
		return;
	}

	if (r->exchange_type == ISAKMP_EXCHANGE_INFORMATIONAL) {
		/* Search for notify payloads */
		for (rp = r->payload->next; rp; rp = rp->next) {
			if (rp->type != ISAKMP_PAYLOAD_N)
				continue;
			/* did we get a DPD request or ACK? */
			if (rp->u.n.protocol != ISAKMP_IPSEC_PROTO_ISAKMP) {
				DEBUG(2, printf("got non isakmp-notify, ignoring...\n"));
				continue;
			}
			if (rp->u.n.type == ISAKMP_N_R_U_THERE) {
				uint32_t seq;
				if (rp->u.n.data_length != 4) {
					DEBUG(2, printf("ignoring bad data length R-U-THERE request\n"));
					continue;
				}
				memcpy(&seq, rp->u.n.data, 4);
				send_dpd(s, 1, seq);
				DEBUG(2, printf("got r-u-there request sent ack\n"));
				continue;
			} else if (rp->u.n.type == ISAKMP_N_R_U_THERE_ACK) {
				uint32_t seqack;
				if (rp->u.n.data_length != 4) {
					DEBUG(2, printf("ignoring bad data length R-U-THERE-ACK\n"));
					continue;
				}
				memcpy(&seqack, rp->u.n.data, 4);
				if (seqack == s->ike.dpd_seqno) {
					s->ike.dpd_seqno_ack = seqack;
				} else {
					DEBUG(2, printf("ignoring r-u-there ack %u (expecting %u)\n", seqack, s->ike.dpd_seqno));
					continue;
				}
				DEBUG(2, printf("got r-u-there ack\n"));
			}
		}
	}
	
	/* check if our isakmp sa gets deleted */
	for (rp = r->payload->next; rp; rp = rp->next) {
		/* search for delete payloads */
		if (rp->type != ISAKMP_PAYLOAD_D)
			continue;
		if (rp->u.d.protocol == ISAKMP_IPSEC_PROTO_IPSEC_ESP) {
			/* RFC2408, 5.15:
			 * Process the Delete payload and take appropriate action, according
			 * to local security policy.  As described above, one appropriate
			 * action SHOULD include cleaning up the local SA database.
			 */
			/* FIXME: any cleanup needed??? */

			free_isakmp_packet(r);
			do_phase2_qm(s);
			return;
		}
		/* skip ipsec-esp delete */
		if (rp->u.d.protocol != ISAKMP_IPSEC_PROTO_ISAKMP) {
			DEBUG(2, printf("got non isakmp-delete, ignoring...\n"));
			continue;
		};
		
		/*
		 * RFC 2408, 3.15 Delete Payload
		 * it is not stated that the SPI field of a delete
		 * payload can be ignored, because it is given in
		 * the headers, but I assume so. In other cases
		 * RFC 2408 (notifications) states this.
		 */
		do_kill = -1;
		DEBUG(2, printf("got isakmp-delete, terminating...\n"));
		free_isakmp_packet(r);
		return;
	}
	
	free_isakmp_packet(r);
	return;
}

int main(int argc, char **argv)
{
	int do_load_balance;
	const uint8_t hex_test[] = { 0, 1, 2, 3 };
	struct sa_block oursa[1];
	struct sa_block *s = oursa;

	test_pack_unpack();
#if defined(__CYGWIN__)
	gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
#endif
	gcry_check_version("1.1.90");
	gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);
	group_init();
	
	memset(s, 0, sizeof(*s));
	s->ipsec.encap_mode = IPSEC_ENCAP_TUNNEL;
	s->ike.timeout = 1000; /* 1 second */

	do_config(argc, argv);
	
	DEBUG(1, printf("\nvpnc version " VERSION "\n"));
	hex_dump("hex_test", hex_test, sizeof(hex_test), NULL);

	DEBUGTOP(2, printf("S1 init_sockaddr\n"));
	init_sockaddr(&s->dst, config[CONFIG_IPSEC_GATEWAY]);
	init_sockaddr(&s->opt_src_ip, config[CONFIG_LOCAL_ADDR]);
	DEBUGTOP(2, printf("S2 make_socket\n"));
	s->ike.src_port = atoi(config[CONFIG_LOCAL_PORT]);
	s->ike.dst_port = ISAKMP_PORT;
	s->ike_fd = make_socket(s, s->ike.src_port, s->ike.dst_port);
	DEBUGTOP(2, printf("S3 setup_tunnel\n"));
	setup_tunnel(s);

	do_load_balance = 0;
	do {
		DEBUGTOP(2, printf("S4 do_phase1_am\n"));
		do_phase1_am(config[CONFIG_IPSEC_ID], config[CONFIG_IPSEC_SECRET], s);
		DEBUGTOP(2, printf("S5 do_phase2_xauth\n"));
		/* FIXME: Create and use a generic function in supp.[hc] */
		if (s->ike.auth_algo >= IKE_AUTH_HybridInitRSA)
			do_load_balance = do_phase2_xauth(s);
		DEBUGTOP(2, printf("S6 do_phase2_config\n"));
		if ((opt_vendor == VENDOR_CISCO) && (do_load_balance == 0))
			do_load_balance = do_phase2_config(s);
	} while (do_load_balance);
	DEBUGTOP(2, printf("S7 setup_link (phase 2 + main_loop)\n"));
	DEBUGTOP(2, printf("S7.0 run interface setup script\n"));
	config_tunnel(s);
	do_phase2_qm(s);
	DEBUGTOP(2, printf("S7.9 main loop (receive and transmit ipsec packets)\n"));
	vpnc_doit(s);

	/* Tear down phase 2 and 1 tunnels */
	send_delete_ipsec(s);
	send_delete_isakmp(s);

	/* Cleanup routing */
	DEBUGTOP(2, printf("S8 close_tunnel\n"));
	close_tunnel(s);

	/* Free resources */
	DEBUGTOP(2, printf("S9 cleanup\n"));
	cleanup(s);

	return 0;
}
