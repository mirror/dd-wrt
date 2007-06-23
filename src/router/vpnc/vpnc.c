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

   $Id: vpnc.c 147 2007-02-19 20:49:52Z Maurice Massar $
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

#include "sysdep.h"
#include "config.h"
#include "isakmp-pkt.h"
#include "math_group.h"
#include "dh.h"
#include "vpnc.h"
#include "tunip.h"
#include "supp.h"

#define ISAKMP_PORT (500)

static int timeout = 1000; /* 1 second */
static uint8_t *resend_hash = NULL;

static uint8_t r_packet[2048];
static ssize_t r_length;

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

	if (strbuf)
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
	size_t len = sizeof(name);

	/* create the socket */
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		error(1, errno, "making socket");

	/* give the socket a name */
	name.sin_family = AF_INET;
	name.sin_addr = s->opt_src_ip;
	name.sin_port = htons(src_port);
	if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
		error(1, errno, "binding to %s:%d", inet_ntoa(s->opt_src_ip), ntohs(src_port));

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

static int recv_ignore_dup(struct sa_block *s, void *recvbuf, size_t recvbufsize)
{
	uint8_t *resend_check_hash;
	int recvsize, hash_len;

	recvsize = recv(s->ike_fd, recvbuf, recvbufsize, 0);
	if (recvsize == -1)
		error(1, errno, "receiving packet");
	
	/* skip NAT-T draft-0 keepalives */
	if ((s->ipsec.natt_active_mode == NATT_ACTIVE_DRAFT_OLD) &&
		(recvsize == 1) && (*((u_char *)(recvbuf)) == 0xff))
		return -1;
	
	hash_len = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
	resend_check_hash = malloc(hash_len);
	gcry_md_hash_buffer(GCRY_MD_SHA1, resend_check_hash, recvbuf, recvsize);
	if (resend_hash && memcmp(resend_hash, resend_check_hash, hash_len) == 0) {
		free(resend_check_hash);
		return -1;
	}
	if (!resend_hash) {
		resend_hash = resend_check_hash;
	} else {
		memcpy(resend_hash, resend_check_hash, hash_len);
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
		memcpy(realtosend+4, tosend, sendsize);
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
			pollresult = poll(&pfd, 1, timeout << tries);
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
		memmove(recvbuf, recvbuf+4, recvsize);
	}

	/* Wait at least 2s for a response or 4 times the time it took
	 * last time.  */
	if (start == end)
		timeout = 2000;
	else
		timeout = 4000 * (end - start);

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

	if (r_length < ISAKMP_PAYLOAD_O || ((r_length - ISAKMP_PAYLOAD_O) % s->ike.ivlen != 0)) {
		DEBUG(2, printf("payload too short or not padded: len=%lld, min=%d (ivlen=%lld)\n",
			(long long)r_length, ISAKMP_PAYLOAD_O, (long long)s->ike.ivlen));
		return ISAKMP_N_UNEQUAL_PAYLOAD_LENGTHS;
	}

	reject = isakmp_crypt(s, r_packet, r_length, 0);
	if (reject != 0)
		return reject;

	s->ike.life.rx += r_length;
	
	{
		r = parse_isakmp_packet(r_packet, r_length, &reject);
		if (reject != 0)
			return reject;
	}

	/* Verify the basic stuff.  */
	if (r->flags != ISAKMP_FLAG_E)
		return ISAKMP_N_INVALID_FLAGS;

	{
		size_t sz, spos;
		gcry_md_hd_t hm;
		unsigned char *expected_hash;
		struct isakmp_payload *h = r->payload;

		if (h == NULL || h->type != ISAKMP_PAYLOAD_HASH || h->u.hash.length != s->ike.md_len)
			return ISAKMP_N_INVALID_HASH_INFORMATION;

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

		if (opt_debug >= 3) {
			printf("hashlen: %lu\n", (unsigned long)s->ike.md_len);
			printf("u.hash.length: %d\n", h->u.hash.length);
			hex_dump("expected_hash", expected_hash, s->ike.md_len, NULL);
			hex_dump("h->u.hash.data", h->u.hash.data, s->ike.md_len, NULL);
		}

		reject = 0;
		if (memcmp(h->u.hash.data, expected_hash, s->ike.md_len) != 0)
			reject = ISAKMP_N_AUTHENTICATION_FAILED;
		gcry_md_close(hm);
#if 0
		if (reject != 0)
			return reject;
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
		flatten_isakmp_payload(pl, &pl_flat, &pl_size);
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

void keepalive_ike(struct sa_block *s)
{
	uint32_t msgid;

	gcry_create_nonce((uint8_t *) & msgid, sizeof(msgid));
	sendrecv_phase2(s, NULL, ISAKMP_EXCHANGE_INFORMATIONAL, msgid, 1, 0, 0, 0, 0, 0, 0);
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
			else
				addenv_ipv4("INTERNAL_IP4_NETMASK", a->u.lots.data);
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
				
				{ /* this is just here because ip route does not accept netmasks */
					int len;
					uint32_t addr;
					
					for (len = 0, addr = ntohl(a->u.acl.acl_ent[i].mask.s_addr);
						addr; addr <<= 1, len++)
						; /* do nothing */
					
					asprintf(&strbuf, "CISCO_SPLIT_INC_%d_MASKLEN", i);
					asprintf(&strbuf2, "%d", len);
					DEBUG(2, printf("(%s), ", strbuf2));
					setenv(strbuf, strbuf2, 1);
					free(strbuf); free(strbuf2);
				}
				
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
	a = new_isakmp_attribute_16(IKE_ATTRIB_GROUP_DESC, dh_group, a);
	a = new_isakmp_attribute_16(IKE_ATTRIB_AUTH_METHOD, auth, a);
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
}

static void do_phase_1(const char *key_id, const char *shared_key, struct sa_block *s)
{
	unsigned char i_nonce[20];
	struct group *dh_grp;
	unsigned char *dh_public;
	unsigned char *returned_hash;
	static const uint8_t xauth_vid[] = XAUTH_VENDOR_ID;
	static const uint8_t unity_vid[] = UNITY_VENDOR_ID;
	static const uint8_t unknown_vid[] = UNKNOWN_VENDOR_ID;
	/* NAT traversal */
	static const uint8_t natt_vid_00[] = NATT_VENDOR_ID_00;
	static const uint8_t natt_vid_01[] = NATT_VENDOR_ID_01;
	static const uint8_t natt_vid_02[] = NATT_VENDOR_ID_02;
	static const uint8_t natt_vid_02n[] = NATT_VENDOR_ID_02n;
	static const uint8_t natt_vid_rfc[] = NATT_VENDOR_ID_RFC;
#if 0
	static const uint8_t dpd_vid[] = DPD_VENDOR_ID; /* dead peer detection */
	static const uint8_t my_vid[] = {
		0x35, 0x53, 0x07, 0x6c, 0x4f, 0x65, 0x12, 0x68, 0x02, 0x82, 0xf2, 0x15,
		0x8a, 0xa8, 0xa0, 0x9e
	};
#endif

	struct isakmp_packet *p1;
	int seen_natt_vid = 0, seen_natd = 0, seen_natd_them = 0, seen_natd_us = 0, natd_type = 0;
	unsigned char *natd_us = NULL, *natd_them = NULL;
	int natt_draft = -1;
	
	DEBUG(2, printf("S4.1\n"));
	gcry_create_nonce(s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
	s->ike.life.start = time(NULL);
	s->ipsec.do_pfs = -1;
	if (s->ike.i_cookie[0] == 0)
		s->ike.i_cookie[0] = 1;
	hex_dump("i_cookie", s->ike.i_cookie, ISAKMP_COOKIE_LENGTH, NULL);
	gcry_create_nonce(i_nonce, sizeof(i_nonce));
	hex_dump("i_nonce", i_nonce, sizeof(i_nonce), NULL);
	DEBUG(2, printf("S4.2\n"));
	/* Set up the Diffie-Hellman stuff.  */
	{
		dh_grp = group_get(get_dh_group_ike()->my_id);
		dh_public = xallocc(dh_getlen(dh_grp));
		dh_create_exchange(dh_grp, dh_public);
		hex_dump("dh_public", dh_public, dh_getlen(dh_grp), NULL);
	}

	DEBUG(2, printf("S4.3\n"));
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
		l->u.id.port = 500; /* this must be 500, not local_port */
		l->u.id.length = strlen(key_id);
		l->u.id.data = xallocc(l->u.id.length);
		memcpy(l->u.id.data, key_id, strlen(key_id));
		l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			xauth_vid, sizeof(xauth_vid));
		l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			unity_vid, sizeof(unity_vid));
		if ((opt_natt_mode == NATT_NORMAL) || (opt_natt_mode == NATT_FORCE)) {
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				natt_vid_rfc, sizeof(natt_vid_rfc));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				natt_vid_02n, sizeof(natt_vid_02n));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				natt_vid_02, sizeof(natt_vid_02));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				natt_vid_01, sizeof(natt_vid_01));
			l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
				natt_vid_00, sizeof(natt_vid_00));
		}
#if 0
		l = l->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			dpd_vid, sizeof(dpd_vid));
#endif
		flatten_isakmp_packet(p1, &pkt, &pkt_len, 0);

		/* Now, send that packet and receive a new one.  */
		r_length = sendrecv(s, r_packet, sizeof(r_packet), pkt, pkt_len, 0);
		free(pkt);
	}
	DEBUG(2, printf("S4.4\n"));
	/* Decode the recieved packet.  */
	{
		struct isakmp_packet *r;
		int reject;
		struct isakmp_payload *rp;
		struct isakmp_payload *nonce = NULL;
		struct isakmp_payload *ke = NULL;
		struct isakmp_payload *hash = NULL;
		struct isakmp_payload *idp = NULL;
		int seen_sa = 0, seen_xauth_vid = 0;
		unsigned char *skeyid;
		gcry_md_hd_t skeyid_ctx;

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
							error(1, 0, "peer selected (single) DES as \"encrytion\" method.\n"
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
			case ISAKMP_PAYLOAD_VID:
				if (rp->u.vid.length == sizeof(xauth_vid)
					&& memcmp(rp->u.vid.data, xauth_vid,
						sizeof(xauth_vid)) == 0) {
					seen_xauth_vid = 1;
				} else if (rp->u.vid.length == sizeof(natt_vid_rfc)
					&& memcmp(rp->u.vid.data, natt_vid_rfc,
						sizeof(natt_vid_rfc)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 2;
					DEBUG(2, printf("peer is NAT-T capable (RFC 3947)\n"));
				} else if (rp->u.vid.length == sizeof(natt_vid_02n)
					&& memcmp(rp->u.vid.data, natt_vid_02n,
						sizeof(natt_vid_02n)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 2;
					DEBUG(2, printf("peer is NAT-T capable (draft-02)\n\n"));
				} else if (rp->u.vid.length == sizeof(natt_vid_02)
					&& memcmp(rp->u.vid.data, natt_vid_02,
						sizeof(natt_vid_02)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 2;
					DEBUG(2, printf("peer is NAT-T capable (draft-02)\n"));
				} else if (rp->u.vid.length == sizeof(natt_vid_01)
					&& memcmp(rp->u.vid.data, natt_vid_01,
						sizeof(natt_vid_01)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 1) natt_draft = 1;
					DEBUG(2, printf("peer is NAT-T capable (draft-01)\n"));
				} else if (rp->u.vid.length == sizeof(natt_vid_00)
					&& memcmp(rp->u.vid.data, natt_vid_00,
						sizeof(natt_vid_00)) == 0) {
					seen_natt_vid = 1;
					if (natt_draft < 0) natt_draft = 0;
					DEBUG(2, printf("peer is NAT-T capable (draft-00)\n"));
				} else {
					hex_dump("unknown ISAKMP_PAYLOAD_VID: ",
						rp->u.vid.data, rp->u.vid.length, NULL);
				}
				break;
			case ISAKMP_PAYLOAD_NAT_D_OLD:
			case ISAKMP_PAYLOAD_NAT_D:
				natd_type = rp->type;
				DEBUG(2, printf("peer is using type %d for NAT-Discovery payloads\n", natd_type));
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
		if (reject == 0 && (hash == NULL || hash->u.hash.length != s->ike.md_len))
			reject = ISAKMP_N_INVALID_HASH_INFORMATION;
		if (reject != 0)
			error(1, 0, "response was invalid [3]: %s(%d)", val_to_string(reject, isakmp_notify_enum_array), reject);

		/* Generate SKEYID.  */
		{
			gcry_md_open(&skeyid_ctx, s->ike.md_algo, GCRY_MD_FLAG_HMAC);
			gcry_md_setkey(skeyid_ctx, shared_key, strlen(shared_key));
			gcry_md_write(skeyid_ctx, i_nonce, sizeof(i_nonce));
			gcry_md_write(skeyid_ctx, nonce->u.nonce.data, nonce->u.nonce.length);
			gcry_md_final(skeyid_ctx);
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
			sa->next = NULL;
			idi->next = NULL;
			idp->next = NULL;
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

			if (memcmp(expected_hash, hash->u.hash.data, s->ike.md_len) != 0) {
				error(1, 0, "hash comparison failed: %s(%d)\ncheck group password!",
					val_to_string(ISAKMP_N_AUTHENTICATION_FAILED, isakmp_notify_enum_array),
					ISAKMP_N_AUTHENTICATION_FAILED);
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

			free(sa_f);
			free(idi);
			free(idp);
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

			/* Determine the IKE encryption key.  */
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
		}

		/* Determine the initial IV.  */
		{
			gcry_md_hd_t hm;

			assert(s->ike.ivlen <= s->ike.md_len);
			gcry_md_open(&hm, s->ike.md_algo, 0);
			gcry_md_write(hm, dh_public, dh_getlen(dh_grp));
			gcry_md_write(hm, ke->u.ke.data, ke->u.ke.length);
			gcry_md_final(hm);
			s->ike.current_iv = xallocc(s->ike.ivlen);
			memcpy(s->ike.current_iv, gcry_md_read(hm, 0), s->ike.ivlen);
			gcry_md_close(hm);
			hex_dump("current_iv", s->ike.current_iv, s->ike.ivlen, NULL);
			memset(s->ike.current_iv_msgid, 0, 4);
		}

		gcry_md_close(skeyid_ctx);
	}

	DEBUG(2, printf("S4.5\n"));
	/* Send final phase 1 packet.  */
	{
		struct isakmp_packet *p2;
		uint8_t *p2kt;
		size_t p2kt_len;
		struct isakmp_payload *pl;

		p2 = new_isakmp_packet();
		memcpy(p2->i_cookie, s->ike.i_cookie, ISAKMP_COOKIE_LENGTH);
		memcpy(p2->r_cookie, s->ike.r_cookie, ISAKMP_COOKIE_LENGTH);
		p2->flags = ISAKMP_FLAG_E;
		p2->isakmp_version = ISAKMP_VERSION;
		p2->exchange_type = ISAKMP_EXCHANGE_AGGRESSIVE;
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
		pl = pl->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			unknown_vid, sizeof(unknown_vid));
		pl = pl->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_VID,
			unity_vid, sizeof(unity_vid));

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
					if (s->ike.src_port == 500)
						s->ike.src_port = 4500;
					s->ike_fd = make_socket(s, s->ike.src_port, s->ike.dst_port = 4500);
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

		s->ike.initial_iv = xallocc(s->ike.ivlen);
		memcpy(s->ike.initial_iv, s->ike.current_iv, s->ike.ivlen);
		hex_dump("initial_iv", s->ike.initial_iv, s->ike.ivlen, NULL);

		/* Now, send that packet and receive a new one.  */
		r_length = sendrecv(s, r_packet, sizeof(r_packet), p2kt, p2kt_len, 0);
		free(p2kt);
	}
	DEBUG(2, printf("S4.6\n"));

	free_isakmp_packet(p1);
	free(returned_hash);
	free(dh_public);
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
					if (s->ike.src_port == 4500)
						s->ike.src_port = 500;
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

static int do_phase_2_xauth(struct sa_block *s)
{
	struct isakmp_packet *r;
	int loopcount;
	int reject;
	int passwd_used = 0;

	DEBUG(2, printf("S5.1\n"));
	/* This can go around for a while.  */
	for (loopcount = 0;; loopcount++) {
		struct isakmp_payload *rp;
		struct isakmp_attribute *a, *ap, *reply_attr;
		char ntop_buf[32];
		int seen_answer = 0;

		DEBUG(2, printf("S5.2\n"));
		
		/* recv and check for notices */
		reject = do_phase2_notice_check(s, &r);
		if (reject == -1)
			return 1;
		
		DEBUG(2, printf("S5.3\n"));
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

		DEBUG(2, printf("S5.4\n"));
		a = r->payload->next->u.modecfg.attributes;
		/* First, print any messages, and verify that we understand the
		 * conversation.  */
		for (ap = a; ap && seen_answer == 0; ap = ap->next)
			if (ap->type == ISAKMP_XAUTH_ATTRIB_ANSWER)
				seen_answer = 1;

		for (ap = a; ap && reject == 0; ap = ap->next)
			switch (ap->type) {
			case ISAKMP_XAUTH_ATTRIB_TYPE:
				if (ap->af != isakmp_attr_16 || ap->u.attr_16 != 0)
					reject = ISAKMP_N_ATTRIBUTES_NOT_SUPPORTED;
				break;
			case ISAKMP_XAUTH_ATTRIB_USER_NAME:
			case ISAKMP_XAUTH_ATTRIB_USER_PASSWORD:
			case ISAKMP_XAUTH_ATTRIB_PASSCODE:
			case ISAKMP_XAUTH_ATTRIB_DOMAIN:
			case ISAKMP_XAUTH_ATTRIB_ANSWER:
			case ISAKMP_XAUTH_ATTRIB_CISCOEXT_VENDOR:
				break;
			case ISAKMP_XAUTH_ATTRIB_MESSAGE:
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
		DEBUG(2, printf("S5.5\n"));
		if (reject != 0)
			phase2_fatal(s, "xauth packet unsupported: %s(%d)", reject);

		inet_ntop(AF_INET, &s->dst, ntop_buf, sizeof(ntop_buf));

		/* Collect data from the user.  */
		reply_attr = NULL;
		for (ap = a; ap && reject == 0; ap = ap->next)
			switch (ap->type) {
			case ISAKMP_XAUTH_ATTRIB_DOMAIN:
				{
					struct isakmp_attribute *na;
					na = new_isakmp_attribute(ap->type, reply_attr);
					reply_attr = na;
					if (!config[CONFIG_DOMAIN] || strlen(config[CONFIG_DOMAIN]) == 0)
						error(1, 0,
							"server requested domain, but none set (use \"Domain ...\" in config or --domain");
					na->u.lots.length = strlen(config[CONFIG_DOMAIN]);
					na->u.lots.data = xallocc(na->u.lots.length);
					memcpy(na->u.lots.data, config[CONFIG_DOMAIN],
						na->u.lots.length);
					break;
				}
			case ISAKMP_XAUTH_ATTRIB_USER_NAME:
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
			case ISAKMP_XAUTH_ATTRIB_ANSWER:
			case ISAKMP_XAUTH_ATTRIB_USER_PASSWORD:
			case ISAKMP_XAUTH_ATTRIB_PASSCODE:
				if (passwd_used && config[CONFIG_NON_INTERACTIVE]) {
					reject = ISAKMP_N_AUTHENTICATION_FAILED;
					phase2_fatal(s, "noninteractive can't reuse password", reject);
					error(2, 0, "authentication unsuccessful");
				} else if (seen_answer || passwd_used || config[CONFIG_XAUTH_INTERACTIVE]) {
					char *pass, *prompt = NULL;
					struct isakmp_attribute *na;

					asprintf(&prompt, "%s for VPN %s@%s: ",
						(ap->type == ISAKMP_XAUTH_ATTRIB_ANSWER) ?
						"Answer" :
						(ap->type == ISAKMP_XAUTH_ATTRIB_USER_PASSWORD) ?
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
		
		DEBUG(2, printf("S5.5.1\n"));
		
		do_config_to_env(s, a);
		
		for (; a; a = a->next)
			if(a->af == isakmp_attr_lots)
				a->u.lots.length = 0;

		r->payload->next->u.modecfg.type = ISAKMP_MODECFG_CFG_ACK;
		sendrecv_phase2(s, r->payload->next,
			ISAKMP_EXCHANGE_MODECFG_TRANSACTION,
			r->message_id, 0, 0, 0, 0, 0, 0, 0);
		
		reject = do_phase2_notice_check(s, &r);
		if (reject == -1)
			return 1;
	}
	
	DEBUG(2, printf("S5.6\n"));
	{
		/* The final SET should have just one attribute.  */
		struct isakmp_attribute *a = r->payload->next->u.modecfg.attributes;
		uint16_t set_result = 1;

		if (a == NULL
			|| a->type != ISAKMP_XAUTH_ATTRIB_STATUS
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
	DEBUG(2, printf("S5.7\n"));
	return 0;
}

static int do_phase_2_config(struct sa_block *s)
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
	if (opt_natt_mode == NATT_CISCO_UDP)
		a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_UDP_ENCAP_PORT, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_CISCO_DEF_DOMAIN, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_NBNS, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_DNS, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_NETMASK, a);
	a = new_isakmp_attribute(ISAKMP_MODECFG_ATTRIB_INTERNAL_IP4_ADDRESS, a);

	rp->u.modecfg.attributes = a;
	sendrecv_phase2(s, rp, ISAKMP_EXCHANGE_MODECFG_TRANSACTION, msgid, 0, 0, 0, 0, 0, 0, 0);

	/* recv and check for notices */
	reject = do_phase2_notice_check(s, &r);
	if (reject == -1)
		return 1;
		
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

static void setup_link(struct sa_block *s)
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

	DEBUG(2, printf("S7.1\n"));
	/* Set up the Diffie-Hellman stuff.  */
	if (get_dh_group_ipsec(s->ipsec.do_pfs)->my_id) {
		dh_grp = group_get(get_dh_group_ipsec(s->ipsec.do_pfs)->my_id);
		DEBUG(3, printf("len = %d\n", dh_getlen(dh_grp)));
		dh_public = xallocc(dh_getlen(dh_grp));
		dh_create_exchange(dh_grp, dh_public);
		hex_dump("dh_public", dh_public, dh_getlen(dh_grp), NULL);
	}

	gcry_create_nonce((uint8_t *) & s->ipsec.rx.spi, sizeof(s->ipsec.rx.spi));
	rp = make_our_sa_ipsec(s);
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
	memset(them->u.id.data, 0, 8);
	us->next = them;
	s->ipsec.life.start = time(NULL);

	if (!dh_grp) {
		rp->next->next = us;
	} else {
		rp->next->next = new_isakmp_data_payload(ISAKMP_PAYLOAD_KE,
			dh_public, dh_getlen(dh_grp));
		rp->next->next->next = us;
	}

	gcry_create_nonce((uint8_t *) & msgid, sizeof(&msgid));
	if (msgid == 0)
		msgid = 1;

	DEBUG(2, printf("S7.2\n"));
	for (i = 0; i < 4; i++) {
		sendrecv_phase2(s, rp, ISAKMP_EXCHANGE_IKE_QUICK,
			msgid, 0, &p_flat, &p_size, 0, 0, 0, 0);

		if (realiv == NULL) {
			realiv = xallocc(s->ike.ivlen);
			memcpy(realiv, s->ike.current_iv, s->ike.ivlen);
			memcpy(realiv_msgid, s->ike.current_iv_msgid, 4);
		}

		DEBUG(2, printf("S7.3\n"));
		reject = unpack_verify_phase2(s, r_packet, r_length, &r, nonce_i, sizeof(nonce_i));

		DEBUG(2, printf("S7.4\n"));
		if (((reject == 0) || (reject == ISAKMP_N_AUTHENTICATION_FAILED))
			&& r->exchange_type == ISAKMP_EXCHANGE_INFORMATIONAL) {
			/* handle notifie responder-lifetime */
			/* (broken hash => ignore AUTHENTICATION_FAILED) */
			if (reject == 0 && r->payload->next->type != ISAKMP_PAYLOAD_N)
				reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;

			if (reject == 0
				&& r->payload->next->u.n.type ==
				ISAKMP_N_IPSEC_RESPONDER_LIFETIME) {
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

		if (p_flat)
			free(p_flat);
		if (realiv)
			free(realiv);
		break;
	}

	DEBUG(2, printf("S7.5\n"));
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

	DEBUG(2, printf("S7.6\n"));
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

	DEBUG(2, printf("S7.7\n"));

	/* Set up the interface here so it's ready when our acknowledgement
	 * arrives.  */
	config_tunnel(s);
	DEBUG(2, printf("S7.8\n"));
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
				error(1, errno, "socket(PF_INET, SOCK_RAW, IPPROTO_ESP)");
			}
#ifdef IP_HDRINCL
			if (setsockopt(s->esp_fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl)) == -1) {
				error(1, errno, "setsockopt(esp_fd, IPPROTO_IP, IP_HDRINCL, 1)");
			}
#endif
		}
		
		s->ipsec.rx.seq_id = s->ipsec.tx.seq_id = 1;
		DEBUG(2, printf("S7.9\n"));
		vpnc_doit(s);
	}
	
	DEBUG(2, printf("S7.10\n"));
	/* finished, send the delete message */
	{
		struct isakmp_payload *d_isakmp, *d_ipsec;
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
		d_ipsec = new_isakmp_payload(ISAKMP_PAYLOAD_D);
		d_ipsec->next = d_isakmp;
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
	if (rp->u.sa.proposals == NULL || rp->u.sa.proposals->next != NULL) /* rekeying should only have one proposal */
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
	
	if (get_algo(SUPP_ALGO_HASH, SUPP_ALGO_IPSEC_SA, seen_auth, NULL, 0) == NULL)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	if (get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IPSEC_SA, seen_enc, NULL, seen_keylen) == NULL)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	
	/* we don't want to change ciphers during rekeying */
	if (s->ipsec.cry_algo != get_algo(SUPP_ALGO_CRYPT, SUPP_ALGO_IPSEC_SA, seen_enc,  NULL, seen_keylen)->my_id)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	if (s->ipsec.md_algo  != get_algo(SUPP_ALGO_HASH,  SUPP_ALGO_IPSEC_SA, seen_auth, NULL, 0)->my_id)
		return ISAKMP_N_BAD_PROPOSAL_SYNTAX;
	
	for (rp = rp->next; rp; rp = rp->next)
		switch (rp->type) {
		case ISAKMP_PAYLOAD_ID:
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
	
	DEBUG(2,printf("got late ike paket: %d bytes\n", r_length));
	/* we should ignore resend pakets here.
	 * unpack_verify_phase2 will fail to decode them probably */
	reject = unpack_verify_phase2(s, r_packet, r_length, &r, NULL, 0);
	
	/* just ignore broken stuff for now */
	if (reject != 0)
		return;
	
	/* everything must be encrypted by now */
	if (r->payload == NULL || r->payload->type != ISAKMP_PAYLOAD_HASH)
		return;
	
	/* empty packet? well, nothing to see here */
	if (r->payload->next == NULL)
		return;
	
	/* do we get an SA proposal for rekeying? */
	if (r->exchange_type == ISAKMP_EXCHANGE_IKE_QUICK &&
		r->payload->next->type == ISAKMP_PAYLOAD_SA) {
		reject = do_rekey(s, r);
		DEBUG(3, printf("do_rekey returned: %d\n", reject));
		return;
	}
	
	/* check if our isakmp sa gets deleted */
	for (rp = r->payload->next; rp; rp = rp->next) {
		/* search for delete payloads */
		if (rp->type != ISAKMP_PAYLOAD_D)
			continue;
		
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
		return;
	}
	
	return;
}

int main(int argc, char **argv)
{
	int do_load_balance;
	const uint8_t hex_test[] = { 0, 1, 2, 3 };
	struct sa_block oursa[1];
	struct sa_block *s = oursa;

	test_pack_unpack();
	gcry_check_version("1.1.90");
	gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);
	group_init();
	memset(s, 0, sizeof(*s));
	s->ipsec.encap_mode = IPSEC_ENCAP_TUNNEL;

	do_config(argc, argv);
	
	hex_dump("hex_test", hex_test, sizeof(hex_test), NULL);

	DEBUG(1, printf("vpnc version " VERSION "\n"));
	DEBUG(2, printf("S1\n"));
	init_sockaddr(&s->dst, config[CONFIG_IPSEC_GATEWAY]);
	DEBUG(2, printf("S2\n"));
	s->ike.src_port = atoi(config[CONFIG_LOCAL_PORT]);
	s->ike.dst_port = ISAKMP_PORT;
	s->ike_fd = make_socket(s, s->ike.src_port, s->ike.dst_port);
	DEBUG(2, printf("S3\n"));
	setup_tunnel(s);

	do_load_balance = 0;
	do {
		DEBUG(2, printf("S4\n"));
		do_phase_1(config[CONFIG_IPSEC_ID], config[CONFIG_IPSEC_SECRET], s);
		DEBUG(2, printf("S5\n"));
		if (s->ike.auth_algo == IKE_AUTH_XAUTHInitPreShared)
			do_load_balance = do_phase_2_xauth(s);
		DEBUG(2, printf("S6\n"));
		if ((opt_vendor != VENDOR_NETSCREEN) && (do_load_balance == 0))
			do_load_balance = do_phase_2_config(s);
	} while (do_load_balance);
	DEBUG(2, printf("S7\n"));
	setup_link(s);
	DEBUG(2, printf("S8\n"));
	setenv("reason", "disconnect", 1);
	system(config[CONFIG_SCRIPT]);

	return 0;
}
