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

/* $Id: tunip.c,v 1.53 1999/09/21 22:20:40 beyssac Exp $ */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <ctype.h>

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

#include <openssl/blowfish.h>
#include <openssl/cast.h>
#include <openssl/des.h>
#include <openssl/idea.h>

#include "defs.h"

#define _PATH_CONF		"/tmp/ipsec/pipsecd.conf"
#define _PATH_STARTUP		"/tmp/ipsec/startup"
#define _PATH_DEV_RANDOM	"/dev/urandom"

#ifdef USE_ETHERTAP
/* Use ethertap device under Linux */
struct ethtap_header {
    unsigned short padding;
    unsigned char dst[6];
    unsigned char src[6];
    unsigned short type;
};

struct ethtap_header ethtap;

#define MAX_LINKHEADER	(sizeof(struct ethtap_header))
#else
#define MAX_LINKHEADER	0
#endif

#define max(a,b)	((a)>(b)?(a):(b))

#define MAX_HEADER	max(64,MAX_LINKHEADER)
#define MAX_SECRET	64
#define MAX_PACKET	4096
#define MAX_PEERS	16

#define UDP_PORT	2001

#ifdef USE_SYSTEM_HASH
#define MD5_Init		MD5Init
#define MD5_Update		MD5Update
#define MD5_Final		MD5Final
#define SHA1_Init		SHA1Init
#define SHA1_Update		SHA1Update
#define SHA1_Final		SHA1Final
#define RIPEMD160_Init		RIPEMD160Init
#define RIPEMD160_Update	RIPEMD160Update
#define RIPEMD160_Final		RIPEMD160Final
#endif

unsigned char buf[MAX_HEADER+MAX_PACKET];

typedef union {
    MD5_CTX md5;
    SHA_CTX sha1;
    RIPEMD160_CTX rmd160;
} hash_CTX;

#define MAX_HASH_CTX	sizeof(hash_CTX)
#define MAX_HASH_DIGEST	20

typedef struct hash_method {
    struct hash_method *next;
    char *name;
    unsigned char size;
    void (*hash_Init)(hash_CTX *context);
    void (*hash_Update)(hash_CTX *context, const unsigned char *data,
		       unsigned int len);
    void (*hash_Final)(unsigned char *digest, hash_CTX *context);
} hash_method_t;

#define MAX_IV_SIZE	8

typedef union {
    BF_KEY bf;
    des_key_schedule des;
    struct {
	des_key_schedule k1;
	des_key_schedule k2;
	des_key_schedule k3;
    } des3;
    CAST_KEY cast;
    IDEA_KEY_SCHEDULE idea;
} crypt_key;

typedef struct crypt_method {
    struct crypt_method *next;
    char *name;
    unsigned char iv_size, block_size;
    void (*encrypt)(unsigned char *iv, crypt_key *ek,
		    unsigned char *t, unsigned int len);
    void (*decrypt)(unsigned char *iv, crypt_key *dk,
		    unsigned char *ct, unsigned int len);
    int (*setekey)(unsigned char *b, unsigned int len, crypt_key *k);
    int (*setdkey)(unsigned char *b, unsigned int len, crypt_key *k);
} crypt_method_t;

struct sa_desc {
    struct sa_desc *next;

    struct sockaddr_in init;	/* initial and fallback remote address */
    struct sockaddr_in dest;	/* current remote address */
    struct sockaddr_in source;	/* local socket address we send packets from */
    unsigned char use_fallback;	/* use initial address as fallback? */
    unsigned char use_dest;	/* is dest address known yet? */

    unsigned long spi;		/* security parameters index */
    unsigned long seq_id;	/* for replay protection (not implemented) */

    /* Encryption key */
    unsigned char enc_secret[MAX_SECRET];
    unsigned int enc_secret_size;
    /* Preprocessed encryption key */
    crypt_key enc_key;
    /* Encryption method to use, or NULL */
    crypt_method_t *cm;

    /* Authentication secret */
    unsigned char auth_secret[MAX_SECRET];
    unsigned int auth_secret_size;
    /* Authentication method to use, or NULL */
    hash_method_t *hm;

    /* Encapsulation method to use to send packets */
    struct encap_method *em;

    /* flags */
    unsigned char local, no_iv;
    /* timeout counters */
    time_t last_packet_sent, last_packet_recv, last_checkifaddr;
};

struct tun_method {
    unsigned int link_header_size;
    unsigned char *link_header;
};

struct peer_desc {
    struct sa_desc *local_sa, *remote_sa;
    int tun_fd;		/* file descriptor for associated tunnel device */
    struct tun_method *tm;
};

/* Size of sent hash (doesn't have to be the full hash) */
#define HMAC_SIZE	12

/* UDP encap header. Currently tries to mimic IPSEC's AH header */
typedef struct udp_encap_header {
    unsigned long spi;			/* security parameters index */
    unsigned long seq_id;		/* sequence id (unimplemented) */
    unsigned char hmac[HMAC_SIZE];	/* HMAC */
} udp_encap_header_t;

/* ICMP "custom AH" encap header */
typedef struct icmp_encap_header {
  /* ICMP fields */
    unsigned char icmp_type;
    unsigned char icmp_code;
    unsigned short icmp_cksum;
  /* Our custom fields */
    unsigned long spi;			/* security parameters index */
    unsigned long seq_id;		/* sequence id (unimplemented) */
    unsigned char hmac[HMAC_SIZE];	/* HMAC */
} icmp_encap_header_t;

/* A real AH header (RFC 2402) */
typedef struct ah_encap_header {
    unsigned char next_header;
    unsigned char payload_len;
    unsigned short reserved;
    unsigned long spi;			/* security parameters index */
    unsigned long seq_id;		/* sequence id (unimplemented) */
    unsigned char hmac[HMAC_SIZE];	/* HMAC */
} ah_encap_header_t;

/* A real ESP header (RFC 2406) */
typedef struct esp_encap_header {
    unsigned long spi;			/* security parameters index */
    unsigned long seq_id;		/* sequence id (unimplemented) */
    /* variable-length payload data + padding */
    /* unsigned char next_header */
    /* optional auth data */
} esp_encap_header_t;

struct encap_method {
    int fd;			/* file descriptor for relevant socket */
    unsigned char *name;

    unsigned int fixed_header_size;

    /* Description of the packet being processed */
    unsigned char *buf;
    unsigned int bufsize, buflen, bufpayload, var_header_size;
    struct sockaddr_in from; int fromlen;

    int (*recv)(struct encap_method *encap,
		unsigned char *buf, unsigned int bufsize,
		struct sockaddr_in *from);
    int (*hmac_compute)(struct encap_method *encap,
			unsigned char do_store,
			struct in_addr *src_ip, hash_method_t *hm,
			unsigned char *secret, unsigned short secret_size);
    struct peer_desc *(*peer_find)(struct encap_method *encap);
    void (*send_peer)(struct encap_method *encap,
		      struct peer_desc *peer,
		      unsigned char *buf, unsigned int bufsize);
    int (*recv_peer)(struct encap_method *encap,
		     struct in_addr *src_ip,
		     struct peer_desc *peer);
};

/* Forward decl */
int hmac_udp_compute(struct encap_method *encap,
		     unsigned char do_store,
		     struct in_addr *src_ip, hash_method_t *hm,
		     unsigned char *secret, unsigned short secret_size);
int hmac_rawip_compute(struct encap_method *encap,
		       unsigned char do_store,
		       struct in_addr *src_ip, hash_method_t *hm,
		       unsigned char *secret, unsigned short secret_size);
int hmac_icmp_compute(struct encap_method *encap,
		      unsigned char do_store,
		      struct in_addr *src_ip, hash_method_t *hm,
		      unsigned char *secret, unsigned short secret_size);
void encap_udp_send_peer(struct encap_method *encap,
			 struct peer_desc *peer,
			 unsigned char *buf, unsigned int bufsize);
void encap_ah_send_peer(struct encap_method *encap,
			struct peer_desc *peer,
			unsigned char *buf, unsigned int bufsize);
void encap_esp_send_peer(struct encap_method *encap,
			 struct peer_desc *peer,
			 unsigned char *buf, unsigned int bufsize);
void encap_icmp_send_peer(struct encap_method *encap,
			  struct peer_desc *peer,
			  unsigned char *buf, unsigned int bufsize);
struct peer_desc *peer_find(unsigned long spi, struct encap_method *encap);
struct sa_desc *find_local_sa(unsigned long spi, struct encap_method *encap);
struct sa_desc *find_remote_sa(unsigned long spi, struct encap_method *encap);
int encap_hmac_recv_peer(struct encap_method *encap,
			 struct in_addr *src_ip,
			 struct peer_desc *peer);
int encap_esp_recv_peer(struct encap_method *encap,
			struct in_addr *src_ip,
			struct peer_desc *peer);
void blowfish_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			  unsigned char *t, unsigned int len);
void blowfish_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			  unsigned char *ct, unsigned int len);
int blowfish_setkey(unsigned char *b, unsigned int len, crypt_key *k);
void cast_cbc_encrypt(unsigned char *iv, crypt_key *ek,
		      unsigned char *t, unsigned int len);
void cast_cbc_decrypt(unsigned char *iv, crypt_key *dk,
		      unsigned char *ct, unsigned int len);
int cast_setkey(unsigned char *b, unsigned int len, crypt_key *k);
void my_idea_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			 unsigned char *t, unsigned int len);
void my_idea_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			 unsigned char *ct, unsigned int len);
int my_idea_set_encrypt_key(unsigned char *b, unsigned int len, crypt_key *k);
int my_idea_set_decrypt_key(unsigned char *b, unsigned int len, crypt_key *k);
void my_des_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			unsigned char *t, unsigned int len);
void my_des_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			unsigned char *ct, unsigned int len);
int my_des_setkey(unsigned char *b, unsigned int len, crypt_key *k);
void my_des3_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			 unsigned char *t, unsigned int len);
void my_des3_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			 unsigned char *ct, unsigned int len);
int my_des3_setkey(unsigned char *b, unsigned int len, crypt_key *k);
void null_encrypt(unsigned char *iv, crypt_key *ek,
		  unsigned char *t, unsigned int len);
void null_decrypt(unsigned char *iv, crypt_key *dk,
		  unsigned char *ct, unsigned int len);
int null_setkey(unsigned char *b, unsigned int len, crypt_key *k);

/* Yuck! Global variables... */

struct peer_desc peers[MAX_PEERS];

struct tun_method tm_tun;
#ifdef USE_ETHERTAP
struct tun_method tm_tap;
#endif

unsigned short peer_num = 0;
unsigned short sa_num = 0;
unsigned short ip_id;

/* Descriptors for predefined encapsulation methods */
#define ENCAP_IPAH	0
#define ENCAP_UDP	1
#define ENCAP_IPESP	2
#define ENCAP_ICMP	3
#define ENCAP_MAX	4
struct encap_method encap_meth[ENCAP_MAX];

/* Security associations lists */
struct sa_desc *local_sa_list = NULL;
struct sa_desc *remote_sa_list = NULL;

/* Interval for keepalive probes */
int keepalive_recv = 600;
int keepalive_send = 200;

int check_ifaddr = 60;

/* IP protocol numbers */
int ipproto_esp = 50;
int ipproto_ah = 51;

hash_method_t hash_md5 = {
    NULL,
    "hmac-md5-96", 16,
    MD5_Init, MD5_Update, MD5_Final
};
hash_method_t hash_sha1 = {
    &hash_md5,
    "hmac-sha1-96", 20,
    SHA1_Init, SHA1_Update, SHA1_Final
};
hash_method_t hash_ripemd160 = {
    &hash_sha1,
    "hmac-rmd160-96", 20,
    RIPEMD160_Init, RIPEMD160_Update, RIPEMD160_Final
};

hash_method_t *hash_list = &hash_ripemd160;

crypt_method_t crypt_idea = {
    NULL,
    "idea_cbc", 8, 8,
    my_idea_cbc_encrypt, my_idea_cbc_decrypt,
    my_idea_set_encrypt_key, my_idea_set_decrypt_key
};
crypt_method_t crypt_cast = {
    &crypt_idea,
    "cast_cbc", 8, 8,
    cast_cbc_encrypt, cast_cbc_decrypt,
    cast_setkey, cast_setkey
};
crypt_method_t crypt_des3 = {
    &crypt_cast,
    "des3_cbc", 8, 8,
    my_des3_cbc_encrypt, my_des3_cbc_decrypt,
    my_des3_setkey, my_des3_setkey
};
crypt_method_t crypt_des = {
    &crypt_des3,
    "des_cbc", 8, 8,
    my_des_cbc_encrypt, my_des_cbc_decrypt,
    my_des_setkey, my_des_setkey
};
crypt_method_t crypt_bf = {
    &crypt_des,
    "blowfish_cbc", 8, 8,
    blowfish_cbc_encrypt, blowfish_cbc_decrypt,
    blowfish_setkey, blowfish_setkey
};
crypt_method_t crypt_null8 = {
    &crypt_bf,
    "null8", 8, 8,
    null_encrypt, null_decrypt,
    null_setkey, null_setkey
};
crypt_method_t crypt_null = {
    &crypt_null8,
    "null", 0, 1,
    null_encrypt, null_decrypt,
    null_setkey, null_setkey
};

crypt_method_t *crypt_list = &crypt_null;

unsigned char global_iv[MAX_IV_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8 };

#define encap_get_fd(e)	((e)->fd)
#define encap_recv(e,b,bs,f) \
	((e)->recv((e),(b),(bs),(f)))
#define encap_hmac_compute(e,s,hm,se,ss) \
	((e)->hmac_compute((e),1,(s),(hm),(se),(ss)))
#define encap_hmac_cmp(e,s,hm,se,ss) \
	((e)->hmac_compute((e),0,(s),(hm),(se),(ss)))
#define encap_peer_find(e) \
	((e)->peer_find((e)))
#define encap_send_peer(e,p,b,bs) \
	((e)->send_peer((e),(p),(b),(bs)))
#define encap_recv_peer(e,s,p) \
	((e)->recv_peer((e),(s),(p)))

/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
u_short
in_cksum(addr, len)
	u_short *addr;
	int len;
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
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

/*
 * Find an encap method by name
 */
struct encap_method *find_encap(unsigned char *name)
{
    int i;
    for (i = 0; i < ENCAP_MAX; i++)
        if (strcmp(encap_meth[i].name, name) == 0)
	    return encap_meth+i;
    return NULL;
}

/*
 * Find a crypt method by name
 */
crypt_method_t *find_crypt(unsigned char *name)
{
    crypt_method_t *cp;
    for (cp = crypt_list; cp; cp = cp->next)
        if (strcmp(cp->name, name) == 0)
	    return cp;
    return NULL;
}

/*
 * Find a hash method by name
 */
hash_method_t *find_hash(unsigned char *name)
{
    hash_method_t *hp;
    for (hp = hash_list; hp; hp = hp->next)
        if (strcmp(hp->name, name) == 0)
	    return hp;
    return NULL;
}

/*
 * Decapsulate from a UDP packet
 */
int encap_udp_recv(struct encap_method *encap,
		   unsigned char *buf, unsigned int bufsize,
		   struct sockaddr_in *from)
{
    int r;

    encap->fromlen = sizeof(encap->from);

    r = recvfrom(encap->fd, buf, bufsize, 0,
		 (struct sockaddr *)&encap->from, &encap->fromlen);
    if (r == -1) {
	syslog(LOG_ERR, "recvfrom: %m");
	return -1;
    }
    if (r < encap->fixed_header_size) {
        syslog(LOG_ALERT, "packet too short from %s",
		inet_ntoa(encap->from.sin_addr));
	return -1;
    }
    encap->buf = buf;
    encap->bufsize = bufsize;
    encap->buflen = r;
    encap->bufpayload = 0;
    *from = encap->from;
    return r;
}

/*
 * Decapsulate from a raw IP packet
 */
int encap_rawip_recv(struct encap_method *encap,
		     unsigned char *buf, unsigned int bufsize,
		     struct sockaddr_in *from)
{
    int r;
    struct ip *p = (struct ip *)buf;

    encap->fromlen = sizeof(encap->from);

    r = recvfrom(encap->fd, buf, bufsize, 0,
		 (struct sockaddr *)&encap->from, &encap->fromlen);
    if (r == -1) {
	syslog(LOG_ERR, "recvfrom: %m");
	return -1;
    }
    if (r < (p->ip_hl << 2)+encap->fixed_header_size) {
        syslog(LOG_ALERT, "packet too short from %s",
		inet_ntoa(encap->from.sin_addr));
	return -1;
    }

#if 0
    printf("raw got %d bytes\n", r);
    for (i = 0; i < r; i++) {
	printf(" %02x", buf[i]);
	if ((i & 15) == 15) printf("\n");
    }
    printf("\n");
#endif

#ifdef NEED_IPID_SWAP
    p->ip_id = htons(p->ip_id);
#endif
#ifdef NEED_IPLEN_FIX
    p->ip_len = r;
#else
    p->ip_len = ntohs(p->ip_len);
#endif

    encap->buf = buf;
    encap->buflen = r;
    encap->bufpayload = (p->ip_hl << 2);
    encap->bufsize = bufsize;
    *from = encap->from;
    return r;
}

/*
 * Decapsulate from a ICMP packet
 */
int encap_icmp_recv(struct encap_method *encap,
		    unsigned char *buf, unsigned int bufsize,
		    struct sockaddr_in *from)
{
    int r;
    struct ip *p = (struct ip *)buf;
    struct icmp *ic;
    encap->fromlen = sizeof(encap->from);

    r = recvfrom(encap->fd, buf, bufsize, 0,
		    (struct sockaddr *)&encap->from, &encap->fromlen);
    if (r == -1) {
	syslog(LOG_ERR, "recvfrom: %m");
	return -1;
    }

    /*
     * Apparently, we only get packets with a correct ICMP checksum from
     * the kernel.
     */

    if (r < (p->ip_hl << 2)+sizeof(struct icmp))
        /* Don't log (can be a reply to a ping, none of our business) */
	return -1;

    ic = (struct icmp *)(buf + (p->ip_hl << 2));
    if (ic->icmp_type != ICMP_ECHOREPLY)
        return -1;
    if (ic->icmp_code != ipproto_ah) /* arbitrary value */
        return -1;

    if (r < (p->ip_hl << 2)+encap->fixed_header_size) {
        syslog(LOG_ALERT, "packet too short from %s",
		inet_ntoa(encap->from.sin_addr));
	return -1;
    }

#ifdef NEED_IPID_SWAP
    p->ip_id = htons(p->ip_id);
#endif
#ifdef NEED_IPLEN_FIX
    p->ip_len = r;
#else
    p->ip_len = ntohs(p->ip_len);
#endif

    encap->buf = buf;
    encap->buflen = r;
    encap->bufpayload = (p->ip_hl << 2);
    encap->bufsize = bufsize;
    *from = encap->from;
    return r;
}

struct peer_desc *encap_udp_peer_find(struct encap_method *encap)
{
    udp_encap_header_t *eh;
    eh = (udp_encap_header_t *)encap->buf;
    return peer_find(ntohl(eh->spi), encap);
}

struct peer_desc *encap_ah_peer_find(struct encap_method *encap)
{
    ah_encap_header_t *eh;
    eh = (ah_encap_header_t *)(encap->buf + encap->bufpayload);
    return peer_find(ntohl(eh->spi), encap);
}

struct peer_desc *encap_esp_peer_find(struct encap_method *encap)
{
    esp_encap_header_t *eh;
    eh = (esp_encap_header_t *)(encap->buf + encap->bufpayload);
    return peer_find(ntohl(eh->spi), encap);
}

struct peer_desc *encap_icmp_peer_find(struct encap_method *encap)
{
    icmp_encap_header_t *eh;
    eh = (icmp_encap_header_t *)(encap->buf + encap->bufpayload);
    return peer_find(ntohl(eh->spi), encap);
}

/*
 * Decapsulate packet
 */
int encap_any_decap(struct encap_method *encap, int fd)
{
    encap->buflen
      -= encap->bufpayload+encap->fixed_header_size+encap->var_header_size;
    encap->buf
      += encap->bufpayload+encap->fixed_header_size+encap->var_header_size;
    if (encap->buflen == 0)
        return 0;
    return 1;
}

void tun_new(struct tun_method *this,
	     unsigned link_header_size, unsigned char *link_header)
{
    this->link_header_size = link_header_size;
    this->link_header = link_header;
}

/*
 * Send decapsulated packet to tunnel device
 */
int tun_send_ip(struct tun_method *this, struct encap_method *encap, int fd)
{
    int sent;

    if (this->link_header_size) {
        encap->buflen += this->link_header_size;
        encap->buf -= this->link_header_size;
        memcpy(encap->buf, this->link_header, this->link_header_size);
    }
    sent = write(fd, encap->buf, encap->buflen);
    if (sent != encap->buflen)
        syslog(LOG_ERR, "truncated in: %d -> %d\n", encap->buflen, sent);
    return 1;
}

/*
 * Initialize encap_method structures for each method
 */
int encap_udp_new(struct encap_method *encap, unsigned short port)
{
    int i;
    struct sockaddr_in source;

    encap->fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (encap->fd == -1) {
	perror("socket(SOCK_DGRAM)");
	return -1;
    }

    memset(&source, 0, sizeof source);
    source.sin_addr.s_addr = INADDR_ANY;
    source.sin_port = htons(port);
    source.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
    source.sin_len = sizeof(&source);
#endif

    i = bind(encap->fd, (struct sockaddr *)&source, sizeof(source));
    if (i == -1) {
	syslog(LOG_ERR, "bind: %m");
	return -1;
    }
    encap->name = "udp";
    encap->recv = encap_udp_recv;
    encap->hmac_compute = hmac_udp_compute;
    encap->peer_find = encap_udp_peer_find;
    encap->send_peer = encap_udp_send_peer;
    encap->recv_peer = encap_hmac_recv_peer;
    encap->fixed_header_size = sizeof(udp_encap_header_t);
    encap->var_header_size = 0;
    return 0;
}

int encap_ah_new(struct encap_method *encap, unsigned char proto)
{
#ifdef IP_HDRINCL
    int hincl = 1;
#endif

    encap->fd = socket(PF_INET, SOCK_RAW, proto);

    if (encap->fd == -1) {
	perror("socket(SOCK_RAW)");
	return -1;
    }
#ifdef IP_HDRINCL
    if (setsockopt(encap->fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl))
	== -1) {
        perror("setsockopt(IP_HDRINCL)");
	close(encap->fd);
	return -1;
    }
#endif
    encap->name = "ipah";
    encap->recv = encap_rawip_recv;
    encap->hmac_compute = hmac_rawip_compute;
    encap->peer_find = encap_ah_peer_find;
    encap->send_peer = encap_ah_send_peer;
    encap->recv_peer = encap_hmac_recv_peer;
    encap->fixed_header_size = sizeof(ah_encap_header_t);
    encap->var_header_size = 0;
    return 0;
}

int encap_esp_new(struct encap_method *encap, unsigned char proto)
{
#ifdef IP_HDRINCL
    int hincl = 1;
#endif

    encap->fd = socket(PF_INET, SOCK_RAW, proto);

    if (encap->fd == -1) {
	perror("socket(SOCK_RAW)");
	return -1;
    }
#ifdef IP_HDRINCL
    if (setsockopt(encap->fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl))
	== -1) {
        perror("setsockopt(IP_HDRINCL)");
	close(encap->fd);
	return -1;
    }
#endif
    encap->name = "ipesp";
    encap->recv = encap_rawip_recv;
    encap->hmac_compute = NULL;
    encap->peer_find = encap_esp_peer_find;
    encap->send_peer = encap_esp_send_peer;
    encap->recv_peer = encap_esp_recv_peer;
    encap->fixed_header_size = sizeof(esp_encap_header_t);
    encap->var_header_size = 0;
    return 0;
}

int encap_icmp_new(struct encap_method *encap, unsigned char proto)
{
#ifdef IP_HDRINCL
    int hincl = 1;
#endif

    encap->fd = socket(PF_INET, SOCK_RAW, proto);

    if (encap->fd == -1) {
	perror("socket(SOCK_RAW)");
	return -1;
    }
#ifdef IP_HDRINCL
    if (setsockopt(encap->fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl))
	== -1) {
        perror("setsockopt(IP_HDRINCL)");
	close(encap->fd);
	return -1;
    }
#endif
    encap->name = "icmp";
    encap->recv = encap_icmp_recv;
    encap->hmac_compute = hmac_icmp_compute;
    encap->peer_find = encap_icmp_peer_find;
    encap->send_peer = encap_icmp_send_peer;
    encap->recv_peer = encap_hmac_recv_peer;
    encap->fixed_header_size = sizeof(icmp_encap_header_t);
    encap->var_header_size = 0;
    return 0;
}

/*
 * This is a hack to retrieve which local IP address the system would use
 * as a source when sending packets to a given destination.
 */
int find_local_addr(struct sockaddr_in *dest, struct sockaddr_in *source)
{
    int addrlen;
    struct sockaddr_in dest_socket;
    int fd;

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
	syslog(LOG_ERR, "socket: %m");
	return -1;
    }

    memset(&dest_socket, 0, sizeof(dest_socket));

    dest_socket.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
    dest_socket.sin_len = sizeof(dest_socket);
#endif
    dest_socket.sin_addr = dest->sin_addr;
    dest_socket.sin_port = htons(4444);

    if (connect(fd,
		(struct sockaddr *)&dest_socket, sizeof(dest_socket)) == -1) {
	syslog(LOG_ERR, "connect: %m");
	close(fd);
	return -1;
    }

    addrlen = sizeof(*source);

    if (getsockname(fd, (struct sockaddr *)source, &addrlen) == -1) {
	syslog(LOG_ERR, "getsockname: %m");
	close(fd);
	return -1;
    }
    close(fd);
    return 0;
}

/*
 * Retrieve and possibly update our local address to a given remote SA.
 * Return 1 if changed, 0 if not, -1 if error.
 */
int update_sa_addr(struct sa_desc *p)
{
    struct sockaddr_in new_addr;

    if (find_local_addr(&p->dest, &new_addr) == -1) {
	syslog(LOG_ALERT,
		"can't find a local address for packets to %s",
		inet_ntoa(p->dest.sin_addr));
	return -1;
    }
    if (new_addr.sin_addr.s_addr != p->source.sin_addr.s_addr) {
	char addr1[16];
	p->source.sin_addr = new_addr.sin_addr;
	strcpy(addr1, inet_ntoa(p->dest.sin_addr));
	syslog(LOG_NOTICE,
		"local address for %s is %s", addr1,
		inet_ntoa(p->source.sin_addr));
	return 1;
    }
    return 0;
}

/*
 * Parse a secret hex string.
 * Bug: the string should include an even number of digits.
 */
unsigned int parse_secret(unsigned char *secret, unsigned char *result)
{
    unsigned char *p = result;
    unsigned char *c = secret;
    unsigned char hex, hex2;

    while (*c && p < result + MAX_SECRET) {
	if (!isxdigit(*c)) {
	    syslog(LOG_ERR, "illegal hex character '%c'", c);
	    return 0;
	}
	hex = *c - '0';
	if (*c >= 'a')
	    hex = *c - ('a'-10);
	else if (*c >= 'A')
	    hex = *c - ('A'-10);
	c++;
	if (!*c)
	    break;
	if (!isxdigit(*c)) {
	    syslog(LOG_ERR, "illegal hex character '%c'", c);
	    return 0;
	}
	hex2 = *c - '0';
	if (*c >= 'a')
	    hex2 = *c - ('a'-10);
	else if (*c >= 'A')
	    hex2 = *c - ('A'-10);
	c++;
	hex = hex*16 + hex2;
	*p++ = hex;
    }
    return p-result;
}

/*
 * Parse a configuration file
 */
void config_read(FILE *cf)
{
    struct hostent *he;
    struct sockaddr_in *dest, *init;
    unsigned char line[4096];
    unsigned int lineno;
    time_t t;

    lineno = 0;
    t = time(NULL);

    while (fgets(line, sizeof line, cf) != NULL) {
	unsigned char *lp = line;
	unsigned char *arg, *arg2;
	line[strlen(line)-1] = '\0';

	lineno++;

	do arg = strsep(&lp, " \t"); while (arg && *arg == '\0');
	if (arg == NULL || arg[0] == '#')
	    continue;

	do arg2 = strsep(&lp, " \t"); while (arg2 && *arg2 == '\0');
	if (arg2 == NULL) {
	    syslog(LOG_ALERT, "line %d too short", lineno);
	    continue;
	}

	if (strcmp(arg, "sa") == 0) {
	    int local;
	    struct sa_desc *sa;
	    struct encap_method *em;
	    em = find_encap(arg2);
	    if (em == NULL) {
		syslog(LOG_ALERT, "Unknown encap format %s on line %d",
			arg2, lineno);
		continue;
	    }

	    sa = (struct sa_desc *)malloc(sizeof(struct sa_desc));

	    sa->em = em;

	    sa->hm = NULL;
	    sa->cm = NULL;
	    sa->enc_secret_size = 0;
	    sa->auth_secret_size = 0;
	    sa->spi = 0;
	    sa->seq_id = 0;

	    sa->last_packet_recv = t;
	    sa->last_packet_sent = t;
	    sa->last_checkifaddr = t;

	    memset(&sa->init, 0, sizeof(sa->init));
	    memset(&sa->dest, 0, sizeof(sa->dest));
	    sa->use_fallback = 0;
	    sa->use_dest = 0;
	    sa->no_iv = 0;
	    local = 1;

	    for (;;) {
		do arg = strsep(&lp, " \t"); while (arg && *arg == '\0');
		if (arg == NULL)
		    break;
		arg2 = strchr(arg, '=');
		if (arg2) {
		    *arg2 = '\0';
		    arg2++;
		} else
		    arg2 = "";
		if (strcmp(arg, "enc") == 0) {
		    sa->cm = find_crypt(arg2);
		    if (sa->cm == NULL) {
			syslog(LOG_ERR, "Unknown crypt method %s on line %d",
				arg2, lineno);
			continue;
		    }
		} else if (strcmp(arg, "noiv") == 0) {
		    sa->no_iv = 1;
		} else if (strcmp(arg, "dest") == 0) {
		    he = gethostbyname(arg2);
		    if (he == NULL) {
			syslog(LOG_ALERT, "Unknown host %s on line %d",
				arg2, lineno);
			continue;
		    }
		    if (he->h_addrtype != AF_INET
			|| he->h_length != sizeof(init->sin_addr.s_addr)) {
			syslog(LOG_ALERT,
				"Address type mismatch for host %s on line %d",
				arg2, lineno);
			continue;
		    }

		    memset(&sa->init, 0, sizeof(sa->init));
		    memcpy((caddr_t)&sa->init.sin_addr, he->h_addr,
			   sizeof(sa->init.sin_addr));
		    sa->init.sin_port = htons(UDP_PORT);
		    sa->init.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
		    sa->init.sin_len = sizeof(dest);
#endif
		    sa->dest = sa->init;
		    if (update_sa_addr(sa) == -1)
			continue;
		    sa->use_fallback = 1;
		    sa->use_dest = 1;
		    local = 0;
		} else if (strcmp(arg, "ekey") == 0) {
		    sa->enc_secret_size = parse_secret(arg2, sa->enc_secret);
		    if (sa->enc_secret_size == 0) {
			syslog(LOG_ALERT, "No crypt key on line %d", lineno);
			continue;
		    }
		} else if (strcmp(arg, "akey") == 0) {
		    sa->auth_secret_size = parse_secret(arg2, sa->auth_secret);
		    if (sa->auth_secret_size == 0) {
			syslog(LOG_ALERT, "No auth key on line %d", lineno);
			continue;
		    }
		} else if (strcmp(arg, "spi") == 0) {
		    sa->spi = atoi(arg2);
		} else if (strcmp(arg, "auth") == 0) {
		    sa->hm = find_hash(arg2);
		    if (sa->hm == NULL) {
			syslog(LOG_ALERT, "Unknown auth method %s on line %d",
				arg2, lineno);
			continue;
		    }
		} else {
		    syslog(LOG_ALERT, "Unknown keyword %s on line %d",
			    arg, lineno);
		}
	    }
	    if (sa->cm) {
	        int rsk;
	        if (local)
		    rsk = sa->cm->setdkey(sa->enc_secret,
					  sa->enc_secret_size,
					  &sa->enc_key);
		else
		    rsk = sa->cm->setekey(sa->enc_secret,
					  sa->enc_secret_size,
					  &sa->enc_key);
		if (rsk != 0) {
		      syslog(LOG_ALERT, "Invalid crypt key on line %d",
			      lineno);
		      free(sa);
		      continue;
		}
	    }
	    if (local) {
		sa->next = local_sa_list;
		local_sa_list = sa;
	    } else {
		sa->next = remote_sa_list;
		remote_sa_list = sa;
	    }
	} else if (strcmp(arg, "if") == 0) {
	    int fd;
	    struct sa_desc *local_sa, *remote_sa;
	    struct peer_desc *peer;

	    fd = open(arg2, O_RDWR);
	    if (fd == -1) {
		perror(arg);
		continue;
	    }

	    local_sa = NULL;
	    remote_sa = NULL;

	    peer = peers + peer_num;
	    peer->tun_fd = fd;
	    peer->local_sa = NULL;
	    peer->remote_sa = NULL;
	    peer->tm = &tm_tun;

	    for (;;) {
		do arg = strsep(&lp, " \t"); while (arg && *arg == '\0');
		if (arg == NULL)
		    break;
		arg2 = strchr(arg, '=');
		if (arg2) {
		    *arg2 = '\0';
		    arg2++;
		} else
		    arg2 = "";
		if (strcmp(arg, "local_spi") == 0) {
		    unsigned long spi = atoi(arg2);
		    local_sa = find_local_sa(spi, NULL);
		    if (local_sa == NULL) {
			syslog(LOG_ALERT, "Unknown local SPI %ld on line %d",
				spi, lineno);
			continue;
		    }
		} else if (strcmp(arg, "remote_spi") == 0) {
		    unsigned long spi = atoi(arg2);
		    remote_sa = find_remote_sa(spi, NULL);
		    if (remote_sa == NULL) {
			syslog(LOG_ALERT, "Unknown remote SPI %ld on line %d",
				spi, lineno);
			continue;
		    }
#ifdef USE_ETHERTAP
		} else if (strcmp(arg, "tap") == 0) {
		    peer->tm = &tm_tap;
		    continue;
#endif
		} else {
		    syslog(LOG_ALERT, "Unknown keyword %s on line %d",
			    arg, lineno);
		}
	    }
	    if (!local_sa || !remote_sa) {
		syslog(LOG_ALERT, "Local or remote SPI missing on line %d",
			lineno);
		if (local_sa)
		    free(local_sa);
		if (remote_sa)
		    free(remote_sa);
		continue;
	    }
	    peer->local_sa = local_sa;
	    peer->remote_sa = remote_sa;
	    peer_num++;
	} else if (strcmp(arg, "keepalive_recv") == 0) {
	    keepalive_recv = atoi(arg2);
	} else if (strcmp(arg, "keepalive_send") == 0) {
	    keepalive_send = atoi(arg2);
	} else if (strcmp(arg, "checkifaddr") == 0) {
	    check_ifaddr = atoi(arg2);
	} else {
	    syslog(LOG_ALERT, "unrecognized syntax on line %d", lineno);
	}
    }
}

/*
 * Find the peer record associated with a given local SPI.
 */
struct peer_desc *peer_find(unsigned long spi, struct encap_method *encap)
{
    unsigned short i;
    for (i = 0; i < peer_num; i++) {
	if (peers[i].local_sa->spi == spi
	    && peers[i].local_sa->em == encap)
	    return peers+i;
    }
    syslog(LOG_ALERT, "unknown spi %ld", spi);
    return NULL;
}

/*
 * Find the SA record for a given local SPI.
 */
struct sa_desc *find_local_sa(unsigned long spi, struct encap_method *encap)
{
    struct sa_desc *sap;
    for (sap = local_sa_list; sap; sap = sap->next)
	if (sap->spi == spi && (encap == NULL || sap->em == encap))
	    return sap;
    syslog(LOG_ALERT, "Unknown local SPI %ld", spi);
    return NULL;
}

/*
 * Find the SA record for a given remote SPI.
 */
struct sa_desc *find_remote_sa(unsigned long spi, struct encap_method *encap)
{
    struct sa_desc *sap;
    for (sap = remote_sa_list; sap; sap = sap->next)
	if (sap->spi == spi && (encap == NULL || sap->em == encap))
	    return sap;
    syslog(LOG_ALERT, "Unknown remote SPI %ld", spi);
    return NULL;
}

/*
 * Compute HMAC for an arbitrary stream of bytes
 */
int hmac_compute(hash_method_t *hm,
		 unsigned char *data, unsigned int data_size,
		 unsigned char *digest, unsigned char do_store,
		 unsigned char *secret, unsigned short secret_size)
{
    unsigned char hmac_digest[MAX_HASH_DIGEST];
    int i;
    hash_CTX hash_ctx;

    /* See RFC 2104 */

    /* XXX: this assumes sizeof(long) == 4 */
    unsigned long k_ipad[16];
    unsigned long k_opad[16];

    /* Prepare key pads */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, secret, secret_size);
    memcpy(k_opad, secret, secret_size);

    for (i = 0; i < 16; i++) {
	k_ipad[i] ^= 0x36363636;
	k_opad[i] ^= 0x5c5c5c5c;
    }

    /* 1st pass */
    hm->hash_Init(&hash_ctx);
    hm->hash_Update(&hash_ctx, (char *)k_ipad, 64);
    hm->hash_Update(&hash_ctx, data, data_size);
    hm->hash_Final(hmac_digest, &hash_ctx);

    /* 2nd pass */
    hm->hash_Init(&hash_ctx);
    hm->hash_Update(&hash_ctx, (char *)k_opad, 64);
    hm->hash_Update(&hash_ctx, hmac_digest, hm->size);
    hm->hash_Final(hmac_digest, &hash_ctx);

    if (do_store) {
        memcpy(digest, hmac_digest, HMAC_SIZE);
        return 0;
    } else
        return memcmp(digest, hmac_digest, HMAC_SIZE);
}

void init_global_iv()
{
    struct random_pool {
	unsigned char rand[MAX_IV_SIZE];
	struct timeval t;
	int pid;
	int ppid;
	int fd;
    } r;
    MD5_CTX ctx;
    unsigned char md5[16];

    gettimeofday(&r.t, NULL);
    r.pid = getpid();
    r.ppid = getppid();
    r.fd = open(_PATH_DEV_RANDOM, O_RDONLY);
    if (r.fd != -1) {
	read(r.fd, r.rand, sizeof(r.rand));
	close(r.fd);
    }

    MD5_Init(&ctx);
    MD5_Update(&ctx, (char *)&r, sizeof r);
    MD5_Final(md5, &ctx);

    memcpy(global_iv, md5, sizeof(global_iv));
}

/*
 * Compute HMAC for UDP encapsulation.
 * XXX: not really HMAC yet...
 */
int hmac_udp_compute(struct encap_method *encap,
		     unsigned char do_store,
		     struct in_addr *src_ip, hash_method_t *hm,
		     unsigned char *secret, unsigned short secret_size)
{
    hash_CTX hash_ctx;
    unsigned char hmac_digest[MAX_HASH_DIGEST];
    udp_encap_header_t *eh = (udp_encap_header_t *)encap->buf;

    hm->hash_Init(&hash_ctx);
    hm->hash_Update(&hash_ctx, secret, secret_size);
    hm->hash_Update(&hash_ctx, (unsigned char *)src_ip, sizeof(*src_ip));
    hm->hash_Update(&hash_ctx, (unsigned char *)&eh->spi, sizeof(eh->spi));
    hm->hash_Update(&hash_ctx,
		    (unsigned char *)&eh->seq_id, sizeof(eh->seq_id));
    hm->hash_Update(&hash_ctx,
	      encap->buf+encap->fixed_header_size,
	      encap->buflen-encap->fixed_header_size);
    hm->hash_Update(&hash_ctx, ":::", 3);
    hm->hash_Final(hmac_digest, &hash_ctx);

    if (do_store) {
        memcpy(eh->hmac, hmac_digest, HMAC_SIZE);
        return 0;
    } else
        return memcmp(eh->hmac, hmac_digest, HMAC_SIZE);
}

/*
 * Compute HMAC for ICMP encapsulation.
 * XXX: could be merged with hmac_rawip_compute
 */
int hmac_icmp_compute(struct encap_method *encap,
		      unsigned char do_store,
		      struct in_addr *src_ip, hash_method_t *hm,
		      unsigned char *secret, unsigned short secret_size)
{
    unsigned char hmac_save[HMAC_SIZE];
    icmp_encap_header_t *eh;
    struct ip *ip;
    struct icmp *icmp;
    int retval;

    unsigned char sip_ttl, sip_tos;
    unsigned short sip_sum, sip_off, sicmp_sum;

    ip = (struct ip *)encap->buf;
    icmp = (struct icmp *)(ip + 1);
    eh = (icmp_encap_header_t *)icmp;

    /* Save, then zero-out mutable fields before computing */
    sip_ttl = ip->ip_ttl;
    sip_sum = ip->ip_sum;
    sip_tos = ip->ip_tos;
    sip_off = ip->ip_off;
    sicmp_sum = icmp->icmp_cksum;
    if (!do_store)
        memcpy(hmac_save, eh->hmac, sizeof(eh->hmac));
    ip->ip_ttl = 0;
    ip->ip_sum = 0;
    ip->ip_tos = 0;
    ip->ip_off = 0;
    ip->ip_len = htons(ip->ip_len);
    memset(eh->hmac, 0, sizeof(eh->hmac));
    icmp->icmp_cksum = 0;

#if 0
    printf("computing HMAC on:\n");
    for (i = 0; i < encap->buflen; i++) {
	printf(" %02x", encap->buf[i]);
	if ((i & 15) == 15) printf("\n");
    }
    printf("\n");
#endif

    retval = hmac_compute(hm, encap->buf, encap->buflen,
			  do_store?eh->hmac:hmac_save,
			  do_store, secret, secret_size);

    /* Restore mutable fields */
    ip->ip_ttl = sip_ttl;
    ip->ip_sum = sip_sum;
    ip->ip_tos = sip_tos;
    ip->ip_off = sip_off;
    ip->ip_len = ntohs(ip->ip_len);
    icmp->icmp_cksum = sicmp_sum;

    if (!do_store)
        memcpy(eh->hmac, hmac_save, sizeof(eh->hmac));

    return retval;
}

/*
 * Compute HMAC for a IP_AH packet.
 */
int hmac_rawip_compute(struct encap_method *encap,
		       unsigned char do_store,
		       struct in_addr *src_ip, hash_method_t *hm,
		       unsigned char *secret, unsigned short secret_size)
{
    unsigned char hmac_save[HMAC_SIZE];
    ah_encap_header_t *eh;
    struct ip *ip;
    int retval;

    unsigned char sip_ttl, sip_tos;
    unsigned short sip_sum, sip_off;

    eh = (ah_encap_header_t *)(encap->buf + encap->bufpayload);
    ip = (struct ip *)encap->buf;

    /* Save, then zero-out mutable fields before computing */
    sip_ttl = ip->ip_ttl;
    sip_sum = ip->ip_sum;
    sip_tos = ip->ip_tos;
    sip_off = ip->ip_off;
    if (!do_store)
        memcpy(hmac_save, eh->hmac, sizeof(eh->hmac));
    ip->ip_ttl = 0;
    ip->ip_sum = 0;
    ip->ip_tos = 0;
    ip->ip_off = 0;
    ip->ip_len = htons(ip->ip_len);
    memset(eh->hmac, 0, sizeof(eh->hmac));

#if 0
    printf("computing HMAC on:\n");
    for (i = 0; i < encap->buflen; i++) {
	printf(" %02x", encap->buf[i]);
	if ((i & 15) == 15) printf("\n");
    }
    printf("\n");
#endif

    retval = hmac_compute(hm, encap->buf, encap->buflen,
			  do_store?eh->hmac:hmac_save,
			  do_store, secret, secret_size);

    /* Restore mutable fields */
    ip->ip_ttl = sip_ttl;
    ip->ip_sum = sip_sum;
    ip->ip_tos = sip_tos;
    ip->ip_off = sip_off;
    ip->ip_len = ntohs(ip->ip_len);

    if (!do_store)
        memcpy(eh->hmac, hmac_save, sizeof(eh->hmac));

    return retval;
}

/*
 * Encapsulate a packet in UDP and send to the peer.
 * "buf" should have exactly MAX_HEADER free bytes at its beginning
 * to account for encapsulation data (not counted in "size").
 */
void encap_udp_send_peer(struct encap_method *encap,
			 struct peer_desc *peer,
			 unsigned char *buf, unsigned int bufsize)
{
    int sent;
    udp_encap_header_t *eh;

    /* Prepend our encapsulation header */
    encap->buf = buf + MAX_HEADER - encap->fixed_header_size;
    encap->buflen = bufsize + encap->fixed_header_size;

    eh = (udp_encap_header_t *)encap->buf;
    eh->spi = ntohl(peer->remote_sa->spi);
    eh->seq_id = htonl(++peer->remote_sa->seq_id);

    hmac_udp_compute(encap, 1,
		     &peer->remote_sa->source.sin_addr, peer->remote_sa->hm,
		     peer->remote_sa->auth_secret,
		     peer->remote_sa->auth_secret_size);

    sent = sendto(encap->fd, encap->buf, encap->buflen, 0,
		  (struct sockaddr *)&peer->remote_sa->dest,
		  sizeof(peer->remote_sa->dest));
    if (sent == -1) {
	syslog(LOG_ERR, "sendto: %m");
	return;
    }
    if (sent != encap->buflen)
	syslog(LOG_ALERT, "truncated out (%d out of %d)",
		sent, encap->buflen);
}

/*
 * Encapsulate a packet in IP AH and send to the peer.
 * "buf" should have exactly MAX_HEADER free bytes at its beginning
 * to account for encapsulation data (not counted in "size").
 */
void encap_ah_send_peer(struct encap_method *encap,
			struct peer_desc *peer,
			unsigned char *buf, unsigned int bufsize)
{
    int sent;
    ah_encap_header_t *eh;
    struct ip *tip, *ip;

    buf += MAX_HEADER;

    /* Keep a pointer to the old IP header */
    tip = (struct ip *)buf;

    encap->buf = buf;
    encap->buflen = bufsize;

    /* Prepend our encapsulation header and new IP header */
    encap->buf -= encap->fixed_header_size + sizeof(struct ip);
    encap->buflen += encap->fixed_header_size + sizeof(struct ip);

    encap->bufpayload = sizeof(struct ip);

    eh = (ah_encap_header_t *)(encap->buf + encap->bufpayload);
    ip = (struct ip *)(encap->buf);

    eh->spi = htonl(peer->remote_sa->spi);
    eh->seq_id = htonl(++peer->remote_sa->seq_id);
    eh->next_header = IPPROTO_IPIP;
    eh->payload_len = 4;
    eh->reserved = 0;

    /* Fill non-mutable fields */
    ip->ip_v = IPVERSION;
    ip->ip_hl = 5;
    ip->ip_len = encap->buflen;
    ip->ip_id = htons(ip_id++);
    ip->ip_p = ipproto_ah;
    ip->ip_src = peer->remote_sa->source.sin_addr;
    ip->ip_dst = peer->remote_sa->dest.sin_addr;

    hmac_rawip_compute(encap, 1,
		       &peer->remote_sa->source.sin_addr,
		       peer->remote_sa->hm,
		       peer->remote_sa->auth_secret,
		       peer->remote_sa->auth_secret_size);

    /* Fill mutable fields */
    ip->ip_tos = (bufsize < sizeof(struct ip)) ? 0 : tip->ip_tos;
    ip->ip_off = 0;
    ip->ip_ttl = IPDEFTTL;
    ip->ip_sum = 0;

    sent = sendto(encap->fd, encap->buf, encap->buflen, 0,
		  (struct sockaddr *)&peer->remote_sa->dest,
		  sizeof(peer->remote_sa->dest));
    if (sent == -1) {
	syslog(LOG_ERR, "sendto: %m");
	return;
    }
    if (sent != encap->buflen)
	syslog(LOG_ALERT, "truncated out (%d out of %d)",
		sent, encap->buflen);
}

/*
 * Encapsulate a packet in IP ESP and send to the peer.
 * "buf" should have exactly MAX_HEADER free bytes at its beginning
 * to account for encapsulation data (not counted in "size").
 */
void encap_esp_send_peer(struct encap_method *encap,
			 struct peer_desc *peer,
			 unsigned char *buf, unsigned int bufsize)
{
    int sent;
    esp_encap_header_t *eh;
    struct ip *tip, *ip;
    unsigned char *iv, *cleartext;
    int i, padding;
    unsigned int cleartextlen;
    unsigned char impl_iv[MAX_IV_SIZE];

    buf += MAX_HEADER;

    /* Keep a pointer to the old IP header */
    tip = (struct ip *)buf;

    encap->buf = buf;
    encap->buflen = bufsize;

    /*
     * Add padding as necessary
     *
     * XXX: this should be checked, RFC 2406 section 2.4 is quite
     *	    obscure on that point.
     */
    padding = peer->remote_sa->cm->block_size
      - (encap->buflen+2)%peer->remote_sa->cm->block_size;
    if (padding == peer->remote_sa->cm->block_size)
        padding = 0;

    if ((encap->buflen+2+padding) & 3)
        /*
	 * More padding needed to align padlen and next_header on
	 * the rightmost 2 bytes of a long.
	 */
        padding += 4 - ((encap->buflen+2+padding) & 3);

    for (i = 1; i <= padding; i++) {
        encap->buf[encap->buflen] = i;
	encap->buflen++;
    }

    /* Add trailing padlen and next_header */
    encap->buf[encap->buflen++] = padding;
    encap->buf[encap->buflen++] = IPPROTO_IPIP;

    cleartext = buf;
    cleartextlen = encap->buflen;

    /* Prepend our encapsulation header and new IP header */
    if (peer->remote_sa->no_iv)
	encap->var_header_size = encap->fixed_header_size;
    else
	encap->var_header_size = encap->fixed_header_size
				 + peer->remote_sa->cm->iv_size;

    encap->buf -= sizeof(struct ip) + encap->var_header_size;
    encap->buflen += sizeof(struct ip) + encap->var_header_size;

    encap->bufpayload = sizeof(struct ip);

    eh = (esp_encap_header_t *)(encap->buf + encap->bufpayload);
    ip = (struct ip *)(encap->buf);
    eh->spi = htonl(peer->remote_sa->spi);
    eh->seq_id = htonl(++peer->remote_sa->seq_id);

    if (peer->remote_sa->no_iv) {
	/* Implicit IV (OpenBSD compatibility mode) */
	iv = impl_iv;
	memcpy(iv, &eh->seq_id, sizeof(eh->seq_id));
	iv[4] = ~iv[0];
	iv[5] = ~iv[1];
	iv[6] = ~iv[2];
	iv[7] = ~iv[3];
    } else {
	/* Copy initialization vector in packet */
	iv = (unsigned char *)(eh + 1);
	memcpy(iv, global_iv, peer->remote_sa->cm->iv_size);
    }

    /* Fill non-mutable fields */
    ip->ip_v = IPVERSION;
    ip->ip_hl = 5;
    ip->ip_len = encap->buflen + (peer->remote_sa->hm?HMAC_SIZE:0);
    ip->ip_id = htons(ip_id++);
    ip->ip_p = ipproto_esp;
    ip->ip_src = peer->remote_sa->source.sin_addr;
    ip->ip_dst = peer->remote_sa->dest.sin_addr;

    /* Fill mutable fields */
    ip->ip_tos = (bufsize < sizeof(struct ip)) ? 0 : tip->ip_tos;
    ip->ip_off = 0;
    ip->ip_ttl = IPDEFTTL;
    ip->ip_sum = 0;

#if 0
    printf("sending ESP packet (before crypt %d):\n", cleartextlen);
    for (i = 0; i < encap->buflen; i++)
        printf(" %02x", encap->buf[i]);
    printf("\n");
#endif

    peer->remote_sa->cm->encrypt(global_iv, &peer->remote_sa->enc_key,
				 cleartext, cleartextlen);

#if 0
    printf("sending ESP packet (after crypt %d):\n", cleartextlen);
    for (i = 0; i < encap->buflen; i++)
        printf(" %02x", encap->buf[i]);
    printf("\n");
#endif

    /* Handle optional authentication field */
    if (peer->remote_sa->hm) {
#if 0
	printf("sending ESP packet (before ah %d):\n", encap->buflen);
	for (i = 0; i < encap->buflen; i++)
	    printf(" %02x", encap->buf[i]);
	printf("\n");
#endif
	hmac_compute(peer->remote_sa->hm,
		     encap->buf+encap->bufpayload,
		     encap->var_header_size+cleartextlen,
		     encap->buf+encap->bufpayload
		     +encap->var_header_size+cleartextlen,
		     1,
		     peer->remote_sa->auth_secret,
		     peer->remote_sa->auth_secret_size);
	encap->buflen += HMAC_SIZE;
#if 0
	printf("sending ESP packet (after ah %d):\n", encap->buflen);
	for (i = 0; i < encap->buflen; i++)
	    printf(" %02x", encap->buf[i]);
	printf("\n");
#endif
    }

    sent = sendto(encap->fd, encap->buf, encap->buflen, 0,
		  (struct sockaddr *)&peer->remote_sa->dest,
		  sizeof(peer->remote_sa->dest));
    if (sent == -1) {
	syslog(LOG_ERR, "sendto: %m");
	return;
    }
    if (sent != encap->buflen)
	syslog(LOG_ALERT, "truncated out (%d out of %d)",
		sent, encap->buflen);
}

/*
 * Encapsulate a packet in ICMP and send to the peer.
 * "buf" should have exactly MAX_HEADER free bytes at its beginning
 * to account for encapsulation data (not counted in "size").
 */
void encap_icmp_send_peer(struct encap_method *encap,
			  struct peer_desc *peer,
			  unsigned char *buf, unsigned int bufsize)
{
    int sent;
    icmp_encap_header_t *eh;
    struct ip *tip, *ip;

    buf += MAX_HEADER;

    /* Keep a pointer to the old IP header */
    tip = (struct ip *)buf;

    encap->buf = buf;
    encap->buflen = bufsize;

    /* Prepend our encapsulation header and new IP header */
    encap->buf -= encap->fixed_header_size + sizeof(struct ip);
    encap->buflen += encap->fixed_header_size + sizeof(struct ip);

    encap->bufpayload = sizeof(struct ip);

    eh = (icmp_encap_header_t *)(encap->buf + encap->bufpayload);
    ip = (struct ip *)(encap->buf);

    /* Fill non-mutable fields */
    ip->ip_v = IPVERSION;
    ip->ip_hl = 5;
    ip->ip_len = encap->buflen;
    ip->ip_id = htons(ip_id++);
    ip->ip_p = IPPROTO_ICMP;
    ip->ip_src = peer->remote_sa->source.sin_addr;
    ip->ip_dst = peer->remote_sa->dest.sin_addr;

    eh->icmp_type = ICMP_ECHOREPLY;
    eh->icmp_code = ipproto_ah; /* arbitrary */
    eh->icmp_cksum = 0;
    eh->spi = htonl(peer->remote_sa->spi);
    eh->seq_id = htonl(++peer->remote_sa->seq_id);

    hmac_icmp_compute(encap, 1,
		      &peer->remote_sa->source.sin_addr,
		      peer->remote_sa->hm,
		      peer->remote_sa->auth_secret,
		      peer->remote_sa->auth_secret_size);

    /* Fill mutable fields */
    ip->ip_tos = (bufsize < sizeof(struct ip)) ? 0 : tip->ip_tos;
    ip->ip_off = 0;
    ip->ip_ttl = IPDEFTTL;
    ip->ip_sum = 0;

    /* Fill ICMP checksum last */
    eh->icmp_cksum = in_cksum(encap->buf+sizeof(struct ip),
			      encap->buflen-sizeof(struct ip));

    sent = sendto(encap->fd, encap->buf, encap->buflen, 0,
		  (struct sockaddr *)&peer->remote_sa->dest,
		  sizeof(peer->remote_sa->dest));
    if (sent == -1) {
	syslog(LOG_ERR, "sendto: %m");
	return;
    }
    if (sent != encap->buflen)
	syslog(LOG_ALERT, "truncated out (%d out of %d)",
		sent, encap->buflen);
}

int encap_hmac_recv_peer(struct encap_method *encap,
			 struct in_addr *src_ip,
			 struct peer_desc *peer)
{
    if (encap_hmac_cmp(encap, src_ip,
		       peer->local_sa->hm,
		       peer->local_sa->auth_secret,
		       peer->local_sa->auth_secret_size) != 0) {
	syslog(LOG_ALERT, "HMAC mismatch from %s",
		inet_ntoa(*src_ip));
	return -1;
    }
    return 0;
}

int encap_esp_recv_peer(struct encap_method *encap,
			struct in_addr *src_ip,
			struct peer_desc *peer)
{
    unsigned int len, i;
    unsigned char padlen, next_header;
    unsigned char *pad;
    unsigned char impl_iv[MAX_IV_SIZE], *iv;
    struct esp_encap_header *eh;

    eh = (struct esp_encap_header *)(encap->buf + encap->bufpayload);
    if (peer->local_sa->no_iv) {
	encap->var_header_size = 0;
	iv = impl_iv;
	memcpy(iv, &eh->seq_id, sizeof(eh->seq_id));
	iv[4] = ~iv[0];
	iv[5] = ~iv[1];
	iv[6] = ~iv[2];
	iv[7] = ~iv[3];
    } else {
	encap->var_header_size = peer->local_sa->cm->iv_size;
	iv = encap->buf + encap->bufpayload + encap->fixed_header_size;
    }

    len = encap->buflen - encap->bufpayload
        - encap->fixed_header_size - encap->var_header_size;

    if (len < 0) {
        syslog(LOG_ALERT, "Packet too short");
        return -1;
    }

    /* Handle optional authentication field */
    if (peer->local_sa->hm) {
	len -= HMAC_SIZE;
	if (hmac_compute(peer->local_sa->hm,
			 encap->buf+encap->bufpayload,
			 encap->fixed_header_size+encap->var_header_size+len,
			 encap->buf+encap->bufpayload
			 +encap->fixed_header_size+encap->var_header_size+len,
			 0,
			 peer->local_sa->auth_secret,
			 peer->local_sa->auth_secret_size) != 0) {
	    syslog(LOG_ALERT, "HMAC mismatch in ESP mode");
	    return -1;
	}
    }

    if ((len % peer->local_sa->cm->block_size) != 0) {
        syslog(LOG_ALERT,
		"payload len %d not a multiple of algorithm block size %d",
		len, peer->local_sa->cm->block_size);
	return -1;
    }

#if 0
    printf("receiving ESP packet (before decrypt):\n");
    for (i = 0; i < len; i++)
        printf(" %02x", encap->buf[encap->bufpayload
				  +encap->fixed_header_size
				  +encap->var_header_size+i]);
    printf("\n");
#endif

    peer->local_sa->cm->decrypt(iv,
				&peer->local_sa->enc_key,
				encap->buf+encap->bufpayload
				+encap->fixed_header_size
				+encap->var_header_size,
				len);

#if 0
    printf("receiving ESP packet (after decrypt %d):\n", len);
    for (i = 0; i < len; i++)
        printf(" %02x", encap->buf[encap->bufpayload
				  +encap->fixed_header_size
				  +encap->var_header_size+i]);
    printf("\n");
#endif

    padlen = encap->buf[encap->bufpayload
		       +encap->fixed_header_size
		       +encap->var_header_size+len-2];
    next_header = encap->buf[encap->bufpayload
			    +encap->fixed_header_size
			    +encap->var_header_size+len-1];

    if (padlen+2 > len) {
        syslog(LOG_ALERT, "Inconsistent padlen");
	return -1;
    }
    if (next_header != IPPROTO_IPIP) {
        syslog(LOG_ALERT, "Inconsistent next_header %d", next_header);
	return -1;
    }

#if 0
    printf("pad len: %d, next_header: %d\n", padlen, next_header);
#endif
    len -= padlen + 2;

    /* Check padding */
    pad = encap->buf + encap->bufpayload
        + encap->fixed_header_size
        + encap->var_header_size + len;
    for (i = 1; i <= padlen; i++) {
        if (*pad != i) {
	    syslog(LOG_ALERT, "Bad padding");
	    return -1;
        }
	pad++;
    }

    return 0;
}

void blowfish_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			  unsigned char *t, unsigned int len)
{
    BF_cbc_encrypt(t, t, len, &ek->bf, iv, BF_ENCRYPT);
}

void blowfish_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			  unsigned char *ct, unsigned int len)
{
    BF_cbc_encrypt(ct, ct, len, &dk->bf, iv, BF_DECRYPT);
}

int blowfish_setkey(unsigned char *b, unsigned int len, crypt_key *k)
{
    BF_set_key(&k->bf, len, b);
    return 0;
}

void cast_cbc_encrypt(unsigned char *iv, crypt_key *ek,
		      unsigned char *t, unsigned int len)
{
    CAST_cbc_encrypt(t, t, len, &ek->cast, iv, CAST_ENCRYPT);
}

void cast_cbc_decrypt(unsigned char *iv, crypt_key *dk,
		      unsigned char *ct, unsigned int len)
{
    CAST_cbc_encrypt(ct, ct, len, &dk->cast, iv, CAST_DECRYPT);
}

int cast_setkey(unsigned char *b, unsigned int len, crypt_key *k)
{
    if (len != CAST_KEY_LENGTH)
	return -1;
    CAST_set_key(&k->cast, len, b);
    return 0;
}

void my_idea_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			 unsigned char *t, unsigned int len)
{
    idea_cbc_encrypt(t, t, len, &ek->idea, iv, IDEA_ENCRYPT);
}

void my_idea_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			 unsigned char *ct, unsigned int len)
{
    idea_cbc_encrypt(ct, ct, len, &dk->idea, iv, IDEA_DECRYPT);
}

int my_idea_set_encrypt_key(unsigned char *b, unsigned int len, crypt_key *k)
{
    if (len != IDEA_KEY_LENGTH)
	return -1;
    idea_set_encrypt_key(b, &k->idea);
    return 0;
}

int my_idea_set_decrypt_key(unsigned char *b, unsigned int len, crypt_key *k)
{
    if (len != IDEA_KEY_LENGTH)
	return -1;
    idea_set_encrypt_key(b, &k->idea);
    idea_set_decrypt_key(&k->idea, &k->idea);
    return 0;
}

void my_des_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			unsigned char *t, unsigned int len)
{
    des_cbc_encrypt(t, t, len, ek->des, iv, DES_ENCRYPT);
}

void my_des_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			unsigned char *ct, unsigned int len)
{
#if 0
    int i;
    printf("%d bytes to decrypt\n", len);
    for (i = 0; i < len; i++) printf(" %02x", ct[i]);
    printf("\n");
#endif
    des_cbc_encrypt(ct, ct, len, dk->des, iv, DES_DECRYPT);
#if 0
    printf("%d bytes after decrypt\n", len);
    for (i = 0; i < len; i++) printf(" %02x", ct[i]);
    printf("\n");
#endif
}

int my_des_setkey(unsigned char *b, unsigned int len, crypt_key *k)
{
    if (len == 8)
	return des_set_key(b, k->des);
    return -1;
}

void my_des3_cbc_encrypt(unsigned char *iv, crypt_key *ek,
			 unsigned char *t, unsigned int len)
{
    des_ede3_cbc_encrypt(t, t, len,
			 ek->des3.k1, ek->des3.k2, ek->des3.k3,
			 iv, DES_ENCRYPT);
}

void my_des3_cbc_decrypt(unsigned char *iv, crypt_key *dk,
			 unsigned char *ct, unsigned int len)
{
    des_ede3_cbc_encrypt(ct, ct, len,
			 dk->des3.k1, dk->des3.k2, dk->des3.k3,
			 iv, DES_DECRYPT);
}

int my_des3_setkey(unsigned char *b, unsigned int len, crypt_key *k)
{
    if (len != 24)
	return -1;

    if (des_set_key(b, k->des3.k1) != 0)
	return -1;
    if (des_set_key(b+8, k->des3.k2) != 0)
	return -1;
    if (des_set_key(b+16, k->des3.k3) != 0)
	return -1;

    return 0;
}

void null_encrypt(unsigned char *iv, crypt_key *ek,
		  unsigned char *t, unsigned int len)
{
    return;
}

void null_decrypt(unsigned char *iv, crypt_key *dk,
		      unsigned char *ct, unsigned int len)
{
    return;
}

int null_setkey(unsigned char *b, unsigned int len, crypt_key *k)
{
    return 0;
}

int main(int argc, char **argv)
{
    time_t t;
    fd_set fds;
    int pack, i;
    struct sockaddr_in from;
    struct stat sb;

    FILE *f;

    openlog ("pipsecd", LOG_PID, LOG_DAEMON);
    syslog (LOG_NOTICE, "pipsecd starting");

    init_global_iv();

    tun_new(&tm_tun, NULL, 0);

#ifdef USE_ETHERTAP
    memset(&ethtap, 0, sizeof(ethtap));
    ethtap.type = htons(0x0800);
    tun_new(&tm_tap, &ethtap, sizeof(ethtap));
#endif

    if (encap_udp_new(&encap_meth[ENCAP_UDP], UDP_PORT) == -1)
	exit(1);
    if (encap_ah_new(&encap_meth[ENCAP_IPAH], ipproto_ah) == -1)
	exit(1);
    if (encap_esp_new(&encap_meth[ENCAP_IPESP], ipproto_esp) == -1)
	exit(1);
    if (encap_icmp_new(&encap_meth[ENCAP_ICMP], IPPROTO_ICMP) == -1)
	exit(1);

    f = fopen(_PATH_CONF, "r");
    if (f == NULL) {
	perror("configuration file");
	exit(1);
    }

    config_read(f);
    fclose(f);

    /* Execute startup script, if any */
    if (stat(_PATH_STARTUP, &sb) == 0 && (sb.st_mode & 0400))
	system(_PATH_STARTUP);

    /* Send a probe to every peer on startup */
    for (i = 0; i < peer_num; i++)
	encap_send_peer(peers[i].remote_sa->em, peers+i, buf, 0);

    FD_ZERO(&fds);

    for (;;) {
	struct timeval tv;

	for (i = 0; i < ENCAP_MAX; i++)
	    FD_SET(encap_get_fd(&encap_meth[i]), &fds);

	for (i = 0; i < peer_num; i++)
	    FD_SET(peers[i].tun_fd, &fds);

	do {
	    tv.tv_usec = 0;
	    tv.tv_sec = keepalive_send;
	    i = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
	    if (i == -1 && errno != EINTR)
		syslog(LOG_ERR, "select: %m");
	} while (i < 0);

	t = time(NULL);

	for (i = 0; i < peer_num; i++) {

	    /* Handle checks on local interface addresses */
	    if (t - peers[i].remote_sa->last_checkifaddr >= check_ifaddr) {
		if (update_sa_addr(peers[i].remote_sa) == 1)
		    /* Address changed, send a probe immediately */
		    encap_send_peer(peers[i].remote_sa->em,
				    peers+i, buf, 0);
		peers[i].remote_sa->last_checkifaddr = t;
	    }

	    /* Handle timeout on packets sent to this peer */
	    if (t - peers[i].remote_sa->last_packet_sent >= keepalive_send) {
		/*
		 * Send an empty packet to the other end of the tunnel,
		 * just to say we're still here and let him update its
		 * idea of our IP address.
		 */
		syslog(LOG_DEBUG, "sending update probe to peer %d", i);
		encap_send_peer(peers[i].remote_sa->em, peers+i, buf, 0);

		/* Update sent packet timeout */
		peers[i].remote_sa->last_packet_sent = t;
	    }

	    /* Handle timeout on packets received from this peer */
	    if (t - peers[i].remote_sa->last_packet_recv >= keepalive_recv) {
		/* Back to fallback address, if any */
		if (peers[i].remote_sa->use_fallback) {
		    syslog(LOG_NOTICE,
			   "using fallback address for peer %d", i);
		    peers[i].remote_sa->dest = peers[i].remote_sa->init;
		}
		/* Check our local interface address for change */
		update_sa_addr(peers[i].remote_sa);
		/*
		 * Send an empty packet to the other end of the tunnel,
		 * just to say we're still here and let him update its
		 * idea of our IP address.
		 */
		syslog(LOG_DEBUG, "sending update probe to peer %d", i);
		encap_send_peer(peers[i].remote_sa->em, peers+i, buf, 0);

		/* Update received packet timeout */
		peers[i].remote_sa->last_packet_recv = t;
	    }

	    if (FD_ISSET(peers[i].tun_fd, &fds)) {

		/* Receive a packet from the tunnel interface */
		pack = read(peers[i].tun_fd,
			    buf+MAX_HEADER-peers[i].tm->link_header_size,
			    MAX_PACKET+peers[i].tm->link_header_size);
		if (pack == -1) {
		    syslog(LOG_ERR, "read: %m");
		    continue;
		}

		if (peers[i].remote_sa->use_dest == 0) {
		    syslog(LOG_NOTICE,
			   "peer %d hasn't a known address yet", i);
		    continue;
		}

		if (((struct ip *)(buf+MAX_HEADER))->ip_dst.s_addr
		    == peers[i].remote_sa->dest.sin_addr.s_addr) {
		    syslog(LOG_ALERT, "routing loop to %s",
			    inet_ntoa(peers[i].remote_sa->dest.sin_addr));
		    continue;
		}

		/* Encapsulate and send to the other end of the tunnel */
		encap_send_peer(peers[i].remote_sa->em, peers+i, buf, pack);

		/* Update sent packet timeout */
		peers[i].remote_sa->last_packet_sent = t;
	    }
	}

	for (i = 0; i < ENCAP_MAX; i++) {
	    if (FD_ISSET(encap_get_fd(&encap_meth[i]), &fds)) {

		/* Receive a packet from a socket */

		struct peer_desc *peer;
		struct encap_method *em;

	        em = &encap_meth[i];

		pack = encap_recv(em, buf, MAX_HEADER+MAX_PACKET, &from);
		if (pack == -1)
		    continue;

		peer = encap_peer_find(em);
		if (peer == NULL) {
		    syslog(LOG_NOTICE, "unknown spi from %s",
			    inet_ntoa(from.sin_addr));
		    continue;
		}

		/* Check auth digest and/or decrypt */
		if (encap_recv_peer(em, &from.sin_addr, peer) != 0)
		    continue;

		/* Check origin IP; update our copy if need be */
		if (peer->remote_sa->use_dest == 0
		    || from.sin_addr.s_addr
		    != peer->remote_sa->dest.sin_addr.s_addr) {
		    /* remote end changed address */
		    char addr1[16];
		    strcpy(addr1, inet_ntoa(peer->remote_sa->dest.sin_addr));
		    syslog(LOG_NOTICE,
			   "spi %ld: remote address changed from %s to %s",
			   peer->remote_sa->spi,
			   addr1,
			   inet_ntoa(from.sin_addr));
		    peer->remote_sa->dest.sin_addr.s_addr
		      = from.sin_addr.s_addr;
		    peer->remote_sa->use_dest = 1;
		    update_sa_addr(peer->remote_sa);
		}
		/* Update received packet timeout */
		peer->remote_sa->last_packet_recv = t;

		if (encap_any_decap(em, peer->tun_fd) == 0)
		    syslog(LOG_DEBUG, "received update probe from peer %d",
			    peer - peers);
		else
		    /* Send the decapsulated packet to the tunnel interface */
		    tun_send_ip(peer->tm, em, peer->tun_fd);
	    }
	}
    }
}
