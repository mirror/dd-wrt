/*
 * eccert - minimal ECDSA P-256 key / CSR / X.509 certificate tool
 *          for space-constrained embedded routers.
 *
 * Depends only on libc (no stdio needed) and one ECC backend:
 *   -DECCERT_P256M : p256-m (Apache-2.0 OR GPL-2.0+), constant-time, smallest
 *   default        : micro-ecc (BSD-2-Clause)
 *
 *   eccert genkey [-out key.pem]
 *   eccert pub    -key key.pem [-out pub.pem]
 *   eccert req    -key key.pem [-subj /CN=..] [-san ..] [-out csr.pem]
 *   eccert cert   (-key key.pem | -csr req.pem) [-subj /CN=..] [-san ..]
 *                 [-days N] [-ca [-pathlen N]] [-cakey ca.key -cacert ca.pem]
 *                 [-out cert.pem]
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <poll.h>

#ifndef ECCERT_MIN_EPOCH            /* 2026-01-01T00:00:00Z: fallback if RTC/NTP not set yet */
#define ECCERT_MIN_EPOCH 1767225600LL
#endif

typedef uint8_t u8;
#define MAXBUF 4096

/* ------------------------------------------------------------------ ECC backend */

static int rng(u8 *dst, unsigned n);
#ifdef ECCERT_P256M
#include "p256-m.h"
/* p256-m (Mbed TLS copy) takes its randomness from psa_generate_random() */
int psa_generate_random(uint8_t *out, size_t n) { return rng(out, (unsigned)n) ? 0 : -1; }
#define ec_init()
#define ec_make_key(pub, priv)      (p256_gen_keypair(priv, pub) == P256_SUCCESS)
#define ec_pub_from_priv(priv, pub) (p256_public_from_private(pub, priv) == P256_SUCCESS)
#define ec_verify(pub, h, sig)      (p256_ecdsa_verify(sig, pub, h, 32) == P256_SUCCESS)
#define ec_valid_pub(pub)           (p256_validate_pubkey(pub) == P256_SUCCESS)
#else
#include "uECC.h"
#define ec_init()                   uECC_set_rng(rng)
#define ec_make_key(pub, priv)      uECC_make_key(pub, priv, uECC_secp256r1())
#define ec_pub_from_priv(priv, pub) uECC_compute_public_key(priv, pub, uECC_secp256r1())
#define ec_verify(pub, h, sig)      uECC_verify(pub, h, 32, sig, uECC_secp256r1())
#define ec_valid_pub(pub)           uECC_valid_public_key(pub, uECC_secp256r1())
#endif

/* ------------------------------------------------------------------ util */

static void put(int fd, const char *s) { if (s) (void)!write(fd, s, strlen(s)); }
static void die(const char *a, const char *b)
{
	put(2, "eccert: "); put(2, a); if (b) { put(2, ": "); put(2, b); } put(2, "\n");
	_exit(1);
}
static void warn(const char *a) { put(2, "eccert: warning: "); put(2, a); put(2, "\n"); }

/* RFC 6979 nonce stream: while active, rng() serves the ECDSA nonce candidates */
static struct { int on, first; u8 K[32], V[32]; } det;
static void det_next(u8 out[32]);

static int rng(u8 *dst, unsigned n)
{
	if (det.on) {
		if (n != 32) return 0;
		det_next(dst);
		return 1;
	}
#if defined(SYS_getrandom) && !defined(ECCERT_NO_GETRANDOM)
	/* getrandom() blocks until the kernel CRNG is seeded (first boot!);
	   try non-blocking first only to be able to tell the user why we hang */
	static int nb = 1;
	while (n) {
		long r = syscall(SYS_getrandom, dst, n, nb ? 1 /* GRND_NONBLOCK */ : 0);
		if (r < 0) {
			if (errno == EINTR) continue;
			if (errno == EAGAIN && nb) { warn("waiting for kernel entropy pool"); nb = 0; continue; }
			break;                       /* ENOSYS (kernel < 3.17), seccomp, ... */
		}
		dst += r; n -= (unsigned)r;
	}
	if (!n) return 1;
#endif
	/* /dev/urandom on old kernels does not block before the pool is seeded:
	   wait until /dev/random is readable first (as libsodium does) */
	static int seeded;
	if (!seeded) {
		int rfd = open("/dev/random", O_RDONLY);
		if (rfd >= 0) {
			struct pollfd p = { rfd, POLLIN, 0 };
			if (poll(&p, 1, 0) == 0) warn("waiting for kernel entropy pool");
			while (poll(&p, 1, -1) < 0 && errno == EINTR) ;
			close(rfd);
		}
		seeded = 1;
	}
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) return 0;
	while (n) {
		ssize_t r = read(fd, dst, n);
		if (r < 0 && errno == EINTR) continue;
		if (r <= 0) { close(fd); return 0; }
		dst += r; n -= (unsigned)r;
	}
	close(fd);
	return 1;
}

static size_t readfile(const char *fn, u8 *buf, size_t max)
{
	int fd = open(fn, O_RDONLY);
	if (fd < 0) die("cannot open", fn);
	size_t n = 0; ssize_t r;
	while (n < max && (r = read(fd, buf + n, max - n)) > 0) n += r;
	close(fd);
	if (n == max) die("file too large", fn);
	buf[n] = 0;
	return n;
}

static void writeout(const char *fn, const char *data, size_t n, int secret)
{
	int fd = 1;
	if (fn) {
		if (secret) unlink(fn);
		fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, secret ? 0600 : 0644);
		if (fd < 0) die("cannot create", fn);
	}
	while (n) { ssize_t r = write(fd, data, n); if (r <= 0) die("write failed", fn); data += r; n -= r; }
	if (fn && close(fd)) die("write failed", fn);
}

/* ---------------------------------------------------------------- sha256 */

typedef struct { uint32_t h[8]; uint64_t len; u8 buf[64]; unsigned bl; } sha256;

static const uint32_t K256[64] = {
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

#define ROR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
static void sha_block(sha256 *c, const u8 *p)
{
	uint32_t w[64], s[8], t1, t2; int i;
	for (i = 0; i < 16; i++) w[i] = (uint32_t)p[4*i]<<24 | p[4*i+1]<<16 | p[4*i+2]<<8 | p[4*i+3];
	for (; i < 64; i++)
		w[i] = (ROR(w[i-2],17) ^ ROR(w[i-2],19) ^ (w[i-2] >> 10)) + w[i-7]
		     + (ROR(w[i-15],7) ^ ROR(w[i-15],18) ^ (w[i-15] >> 3)) + w[i-16];
	memcpy(s, c->h, 32);
	for (i = 0; i < 64; i++) {
		t1 = s[7] + (ROR(s[4],6) ^ ROR(s[4],11) ^ ROR(s[4],25)) + ((s[4] & s[5]) ^ (~s[4] & s[6])) + K256[i] + w[i];
		t2 = (ROR(s[0],2) ^ ROR(s[0],13) ^ ROR(s[0],22)) + ((s[0] & s[1]) ^ (s[0] & s[2]) ^ (s[1] & s[2]));
		memmove(s + 1, s, 28); s[4] += t1; s[0] = t1 + t2;
	}
	for (i = 0; i < 8; i++) c->h[i] += s[i];
}
static void sha_init(sha256 *c)
{
	static const uint32_t iv[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
	                               0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
	memcpy(c->h, iv, 32); c->len = 0; c->bl = 0;
}
static void sha_update(sha256 *c, const u8 *p, size_t n)
{
	c->len += n;
	while (n--) { c->buf[c->bl++] = *p++; if (c->bl == 64) { sha_block(c, c->buf); c->bl = 0; } }
}
static void sha_final(sha256 *c, u8 out[32])
{
	uint64_t bits = c->len * 8; u8 pad = 0x80; int i;
	sha_update(c, &pad, 1); pad = 0;
	while (c->bl != 56) sha_update(c, &pad, 1);
	for (i = 7; i >= 0; i--) { pad = (u8)(bits >> (8 * i)); sha_update(c, &pad, 1); }
	for (i = 0; i < 32; i++) out[i] = (u8)(c->h[i / 4] >> (24 - 8 * (i % 4)));
}
static void sha256_once(const u8 *p, size_t n, u8 out[32])
{
	sha256 c; sha_init(&c); sha_update(&c, p, n); sha_final(&c, out);
}

/* ---------------------------------------------------------------- RFC 6979 */

/* HMAC-SHA256 with 32-byte key over  a || [sep] || [b] || [c]  (b, c: 32 bytes) */
static void hmac(const u8 key[32], const u8 *a, size_t al, int sep, const u8 *b, const u8 *c, u8 out[32])
{
	u8 pad[64], t[32]; sha256 s; int i;
	for (i = 0; i < 64; i++) pad[i] = (i < 32 ? key[i] : 0) ^ 0x36;
	sha_init(&s); sha_update(&s, pad, 64); sha_update(&s, a, al);
	if (sep >= 0) { u8 x = (u8)sep; sha_update(&s, &x, 1); }
	if (b) sha_update(&s, b, 32);
	if (c) sha_update(&s, c, 32);
	sha_final(&s, t);
	for (i = 0; i < 64; i++) pad[i] ^= 0x36 ^ 0x5c;
	sha_init(&s); sha_update(&s, pad, 64); sha_update(&s, t, 32); sha_final(&s, out);
	memset(pad, 0, sizeof pad); memset(&s, 0, sizeof s);
}

static const u8 N256[32] = {
	0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xbc,0xe6,0xfa,0xad,0xa7,0x17,0x9e,0x84,0xf3,0xb9,0xca,0xc2,0xfc,0x63,0x25,0x51};

/* RFC 6979 3.2 b-f for P-256/SHA-256 (qlen = hlen = 256) */
static void det_init(const u8 priv[32], const u8 h[32])
{
	u8 h1[32]; int i, br = 0;
	memcpy(h1, h, 32);
	if (memcmp(h1, N256, 32) >= 0)                 /* bits2octets: h mod n */
		for (i = 31; i >= 0; i--) { int d = h1[i] - N256[i] - br; br = d < 0; h1[i] = (u8)d; }
	memset(det.V, 1, 32); memset(det.K, 0, 32);
	hmac(det.K, det.V, 32, 0, priv, h1, det.K); hmac(det.K, det.V, 32, -1, 0, 0, det.V);
	hmac(det.K, det.V, 32, 1, priv, h1, det.K); hmac(det.K, det.V, 32, -1, 0, 0, det.V);
	det.first = 1;
}

/* RFC 6979 3.2 h: next candidate T (caller checks 1 <= k < n and asks again if not) */
static void det_next(u8 out[32])
{
	if (!det.first) { hmac(det.K, det.V, 32, 0, 0, 0, det.K); hmac(det.K, det.V, 32, -1, 0, 0, det.V); }
	det.first = 0;
	hmac(det.K, det.V, 32, -1, 0, 0, det.V);
	memcpy(out, det.V, 32);
}

/* deterministic ECDSA (RFC 6979): the nonce no longer depends on the RNG */
#ifdef ECCERT_P256M
static int ec_sign(const u8 priv[32], const u8 h[32], u8 sig[64])
{
	/* p256-m draws its nonce as one 32-byte big-endian value from the RNG and
	   retries if it is not in [1, n-1] - exactly the RFC 6979 candidate loop */
	det_init(priv, h); det.on = 1;
	int ok = p256_ecdsa_sign(sig, priv, h, 32) == P256_SUCCESS;
	memset(&det, 0, sizeof det);
	return ok;
}
#else
/* micro-ecc's own uECC_sign_deterministic() is not RFC 6979 conformant (copies the
   big-endian candidate into little-endian words, does not reduce h mod n; upstream
   issue #139), so feed the RFC 6979 candidates to the exported uECC_sign_with_k() */
int uECC_sign_with_k(const uint8_t *private_key, const uint8_t *message_hash, unsigned hash_size,
                     const uint8_t *k, uint8_t *signature, uECC_Curve curve);
static int ec_sign(const u8 priv[32], const u8 h[32], u8 sig[64])
{
	u8 k[32]; int i, ok = 0;
	det_init(priv, h);
	for (i = 0; i < 16 && !ok; i++) {      /* uECC rejects k outside [1, n-1] -> next candidate */
		det_next(k);
		ok = uECC_sign_with_k(priv, h, 32, k, sig, uECC_secp256r1());
	}
	memset(k, 0, sizeof k); memset(&det, 0, sizeof det);
	return ok;
}
#endif

/* ---------------------------------------------------------------- base64 / PEM */

static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static size_t pem_encode(const char *label, const u8 *d, size_t n, char *out)
{
	char *o = out; size_t i; int col = 0;
	o = stpcpy(o, "-----BEGIN "); o = stpcpy(o, label); o = stpcpy(o, "-----\n");
	for (i = 0; i < n; i += 3) {
		uint32_t v = d[i] << 16 | (i + 1 < n ? d[i+1] << 8 : 0) | (i + 2 < n ? d[i+2] : 0);
		*o++ = b64[v >> 18]; *o++ = b64[(v >> 12) & 63];
		*o++ = i + 1 < n ? b64[(v >> 6) & 63] : '=';
		*o++ = i + 2 < n ? b64[v & 63] : '=';
		if ((col += 4) == 64) { *o++ = '\n'; col = 0; }
	}
	if (col) *o++ = '\n';
	o = stpcpy(o, "-----END "); o = stpcpy(o, label); o = stpcpy(o, "-----\n");
	return o - out;
}

/* decodes the first PEM block with the given label (NULL: any). returns DER length */
static size_t pem_decode(const char *txt, const char *label, u8 *out, char *found)
{
	const char *p = txt, *e;
	for (;;) {
		p = strstr(p, "-----BEGIN ");
		if (!p) return 0;
		p += 11;
		e = strstr(p, "-----");
		if (!e) return 0;
		if (!label || ((size_t)(e - p) == strlen(label) && !memcmp(p, label, e - p))) break;
	}
	if (found) { size_t l = e - p < 63 ? e - p : 63; memcpy(found, p, l); found[l] = 0; }
	p = e + 5;
	size_t n = 0; uint32_t acc = 0; int bits = 0;
	for (; *p && *p != '-'; p++) {
		const char *q = strchr(b64, *p);
		if (!q || !*p) continue;
		acc = acc << 6 | (uint32_t)(q - b64); bits += 6;
		if (bits >= 8) { bits -= 8; out[n++] = (u8)(acc >> bits); }
	}
	return n;
}

/* ---------------------------------------------------------------- DER writer */

typedef struct { u8 d[MAXBUF]; size_t n; } buf;

static void b_add(buf *b, const void *p, size_t n)
{
	if (b->n + n > MAXBUF) die("output too large", 0);
	memcpy(b->d + b->n, p, n); b->n += n;
}
static void b_byte(buf *b, u8 c) { b_add(b, &c, 1); }

/* turn everything from 'start' to the end into the content of a TLV */
static void b_wrap(buf *b, size_t start, u8 tag)
{
	size_t len = b->n - start, hl = len < 0x80 ? 2 : len < 0x100 ? 3 : 4;
	if (b->n + hl > MAXBUF) die("output too large", 0);
	memmove(b->d + start + hl, b->d + start, len);
	u8 *h = b->d + start;
	*h++ = tag;
	if (len >= 0x100) { *h++ = 0x82; *h++ = (u8)(len >> 8); }
	else if (len >= 0x80) *h++ = 0x81;
	*h = (u8)len;
	b->n += hl;
}
static void b_tlv(buf *b, u8 tag, const void *p, size_t n)
{
	size_t s = b->n; b_add(b, p, n); b_wrap(b, s, tag);
}
static void b_uint(buf *b, const u8 *p, size_t n)   /* unsigned big-endian INTEGER */
{
	while (n > 1 && !*p) p++, n--;
	size_t s = b->n;
	if (*p & 0x80) b_byte(b, 0);
	b_add(b, p, n); b_wrap(b, s, 0x02);
}

static const u8 OID_ECPUB[]  = {0x06,0x07,0x2a,0x86,0x48,0xce,0x3d,0x02,0x01};
static const u8 OID_P256[]   = {0x06,0x08,0x2a,0x86,0x48,0xce,0x3d,0x03,0x01,0x07};
static const u8 ALG_ECDSA256[] = {0x30,0x0a,0x06,0x08,0x2a,0x86,0x48,0xce,0x3d,0x04,0x03,0x02};

static void b_spki(buf *b, const u8 pub[64])
{
	size_t s = b->n, a;
	a = b->n; b_add(b, OID_ECPUB, sizeof OID_ECPUB); b_add(b, OID_P256, sizeof OID_P256); b_wrap(b, a, 0x30);
	a = b->n; b_byte(b, 0); b_byte(b, 4); b_add(b, pub, 64); b_wrap(b, a, 0x03);
	b_wrap(b, s, 0x30);
}

/* signs the TLV in tbs and wraps tbs || alg || sig into SEQUENCE */
static void b_sign(buf *b, const u8 priv[32])
{
	u8 h[32], sig[64];
	sha256_once(b->d, b->n, h);
	if (!ec_sign(priv, h, sig)) die("signing failed", 0);
	b_add(b, ALG_ECDSA256, sizeof ALG_ECDSA256);
	size_t s = b->n;
	b_byte(b, 0);
	size_t q = b->n;
	b_uint(b, sig, 32); b_uint(b, sig + 32, 32); b_wrap(b, q, 0x30);
	b_wrap(b, s, 0x03);
	b_wrap(b, 0, 0x30);
}

/* ---------------------------------------------------------------- DER reader */

typedef struct { const u8 *p, *end; } rd;
typedef struct { u8 tag; const u8 *raw, *c; size_t len, rawlen; } tlv;

static int r_next(rd *r, tlv *t)
{
	const u8 *p = r->p;
	if (r->end - p < 2) return 0;
	t->raw = p; t->tag = *p++;
	size_t l = *p++;
	if (l & 0x80) {
		int k = l & 0x7f;
		if (!k || k > 3 || r->end - p < k) return 0;
		for (l = 0; k--; ) l = l << 8 | *p++;
	}
	if ((size_t)(r->end - p) < l) return 0;
	t->c = p; t->len = l; r->p = p + l; t->rawlen = r->p - t->raw;
	return 1;
}
static int r_expect(rd *r, tlv *t, u8 tag) { return r_next(r, t) && t->tag == tag; }
static rd r_in(const tlv *t) { rd r = { t->c, t->c + t->len }; return r; }
static int oid_is(const tlv *t, const u8 *full) { return t->rawlen == (size_t)full[1] + 2 && !memcmp(t->raw, full, t->rawlen); }

/* SubjectPublicKeyInfo -> 64 byte X||Y (P-256 only) */
static int parse_spki(const tlv *spki, u8 pub[64])
{
	rd r = r_in(spki), a; tlv alg, o, bs;
	if (!r_expect(&r, &alg, 0x30) || !r_expect(&r, &bs, 0x03)) return 0;
	a = r_in(&alg);
	if (!r_next(&a, &o) || !oid_is(&o, OID_ECPUB) || !r_next(&a, &o) || !oid_is(&o, OID_P256)) return 0;
	if (bs.len != 66 || bs.c[0] || bs.c[1] != 4) return 0;
	memcpy(pub, bs.c + 2, 64);
	return ec_valid_pub(pub);
}

/* ---------------------------------------------------------------- keys */

static void load_key(const char *fn, u8 priv[32], u8 pub[64])
{
	static char txt[MAXBUF + 1]; static u8 der[MAXBUF]; char label[64];
	readfile(fn, (u8 *)txt, sizeof txt - 1);
	size_t n = pem_decode(txt, 0, der, label);
	int pk8 = !strcmp(label, "PRIVATE KEY");
	if (!n || (!pk8 && strcmp(label, "EC PRIVATE KEY"))) die("no EC private key in", fn);
	rd r = { der, der + n }, s; tlv t, o;
	if (!r_expect(&r, &t, 0x30)) goto bad;
	s = r_in(&t);
	if (pk8) {   /* PKCS#8: version, AlgorithmIdentifier, OCTET STRING { SEC1 } */
		if (!r_expect(&s, &t, 0x02) || !r_expect(&s, &t, 0x30)) goto bad;
		rd a = r_in(&t);
		if (!r_next(&a, &o) || !oid_is(&o, OID_ECPUB) || !r_next(&a, &o) || !oid_is(&o, OID_P256))
			die("key is not P-256", fn);
		if (!r_expect(&s, &t, 0x04)) goto bad;
		s = r_in(&t);
		if (!r_expect(&s, &t, 0x30)) goto bad;
		s = r_in(&t);
	}
	/* SEC1: version 1, privateKey OCTET STRING, [0] params, [1] pub */
	if (!r_expect(&s, &t, 0x02) || !r_expect(&s, &t, 0x04) || t.len > 32) goto bad;
	memset(priv, 0, 32); memcpy(priv + 32 - t.len, t.c, t.len);
	if (r_next(&s, &o) && o.tag == 0xa0) {
		rd p = r_in(&o);
		if (!r_next(&p, &o) || !oid_is(&o, OID_P256)) die("key is not P-256", fn);
	}
	if (!ec_pub_from_priv(priv, pub)) goto bad;
	return;
bad:
	die("malformed private key", fn);
}

/* ---------------------------------------------------------------- names */

static const struct { const char *k; u8 oid; u8 str; } attrs[] = {
	{"C", 6, 0x13}, {"ST", 8, 0x0c}, {"L", 7, 0x0c}, {"O", 10, 0x0c}, {"OU", 11, 0x0c}, {"CN", 3, 0x0c},
};

/* "/C=DE/O=Foo/CN=router" -> Name */
static void b_name(buf *b, const char *subj)
{
	size_t s = b->n;
	const char *p = subj;
	while (*p) {
		if (*p == '/') { p++; continue; }
		const char *eq = strchr(p, '='), *end = strchr(p, '/');
		if (!end) end = p + strlen(p);
		if (!eq || eq > end) die("bad -subj", subj);
		unsigned i;
		for (i = 0; i < sizeof attrs / sizeof *attrs; i++)
			if (strlen(attrs[i].k) == (size_t)(eq - p) && !strncmp(p, attrs[i].k, eq - p)) break;
		if (i == sizeof attrs / sizeof *attrs) die("unsupported -subj attribute (C ST L O OU CN)", subj);
		size_t set = b->n, seq = b->n;
		u8 oid[] = {0x06, 0x03, 0x55, 0x04, attrs[i].oid};
		b_add(b, oid, 5);
		b_tlv(b, attrs[i].str, eq + 1, end - eq - 1);
		b_wrap(b, seq, 0x30); b_wrap(b, set, 0x31);
		p = end;
	}
	b_wrap(b, s, 0x30);
}

/* extract CN from DER Name (for default SAN) */
static int name_cn(const u8 *name, size_t n, char *cn, size_t max)
{
	rd r = { name, name + n }, s; tlv t, set, seq, o, v;
	static const u8 CN[] = {0x06, 0x03, 0x55, 0x04, 0x03};
	if (!r_expect(&r, &t, 0x30)) return 0;
	r = r_in(&t);
	int found = 0;
	while (r_next(&r, &set)) {
		s = r_in(&set);
		while (r_next(&s, &seq)) {
			rd q = r_in(&seq);
			if (r_next(&q, &o) && oid_is(&o, CN) && r_next(&q, &v) && v.len < max) {
				memcpy(cn, v.c, v.len); cn[v.len] = 0; found = 1;
			}
		}
	}
	return found;
}

/* ---------------------------------------------------------------- extensions */

static void b_ext(buf *b, u8 oid, int critical, const u8 *val, size_t n)
{
	static const u8 t[] = {0x01, 0x01, 0xff};
	size_t s = b->n;
	u8 o[] = {0x06, 0x03, 0x55, 0x1d, oid};
	b_add(b, o, 5);
	if (critical) b_add(b, t, 3);
	b_tlv(b, 0x04, val, n);
	b_wrap(b, s, 0x30);
}

/* SubjectAltName from "DNS:a,IP:1.2.3.4,..." ; returns 0 if nothing added */
static int b_san(buf *ext, const char *san)
{
	buf v; v.n = 0;
	char item[256];
	const char *p = san;
	while (*p) {
		const char *e = strchr(p, ','); if (!e) e = p + strlen(p);
		size_t l = e - p;
		if (l >= sizeof item) die("SAN entry too long", 0);
		memcpy(item, p, l); item[l] = 0;
		u8 ip[16];
		if (!strncmp(item, "DNS:", 4))       b_tlv(&v, 0x82, item + 4, l - 4);
		else if (!strncmp(item, "IP:", 3)) {
			if (inet_pton(AF_INET, item + 3, ip) == 1)       b_tlv(&v, 0x87, ip, 4);
			else if (inet_pton(AF_INET6, item + 3, ip) == 1) b_tlv(&v, 0x87, ip, 16);
			else die("bad IP in SAN", item);
		}
		else if (!strncmp(item, "email:", 6)) b_tlv(&v, 0x81, item + 6, l - 6);
		else die("SAN must be DNS:, IP: or email:", item);
		p = *e ? e + 1 : e;
	}
	if (!v.n) return 0;
	b_wrap(&v, 0, 0x30);
	b_ext(ext, 0x11, 0, v.d, v.n);
	return 1;
}

static void auto_san(char *dst, const char *cn)
{
	u8 ip[16];
	int isip = inet_pton(AF_INET, cn, ip) == 1 || inet_pton(AF_INET6, cn, ip) == 1;
	strcpy(dst, isip ? "IP:" : "DNS:"); strcat(dst, cn);
}

/* key identifier: leftmost 160 bits of SHA-256(subjectPublicKey) (RFC 7093 method 1) */
static void keyid(const u8 pub[64], u8 id[20])
{
	u8 k[65], h[32]; k[0] = 4; memcpy(k + 1, pub, 64);
	sha256_once(k, 65, h); memcpy(id, h, 20);
}

/* ---------------------------------------------------------------- time */

static void civil(int64_t days, int *y, int *m, int *d)     /* H. Hinnant, days since 1970-01-01 */
{
	days += 719468;
	int64_t era = (days >= 0 ? days : days - 146096) / 146097;
	unsigned doe = (unsigned)(days - era * 146097);
	unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
	unsigned doy = doe - (365*yoe + yoe/4 - yoe/100), mp = (5*doy + 2)/153;
	*d = doy - (153*mp + 2)/5 + 1;
	*m = mp < 10 ? mp + 3 : mp - 9;
	*y = (int)(yoe + era * 400) + (*m <= 2);
}

static void dig(char **o, unsigned v, int n) { while (n--) { (*o)[n] = '0' + v % 10; v /= 10; } }

static void b_time(buf *b, int64_t t)
{
	char s[16], *o = s; int y, m, d;
	if (t < 0) { b_tlv(b, 0x18, "99991231235959Z", 15); return; }  /* "no expiry" (RFC 5280) */
	civil(t / 86400, &y, &m, &d);
	if (y > 9999) { b_tlv(b, 0x18, "99991231235959Z", 15); return; }
	int64_t sec = t % 86400;
	int gen = y < 1950 || y > 2049;
	if (gen) { dig(&o, y, 4); o += 4; } else { dig(&o, y % 100, 2); o += 2; }
	dig(&o, m, 2); o += 2; dig(&o, d, 2); o += 2;
	dig(&o, sec / 3600, 2); o += 2; dig(&o, sec / 60 % 60, 2); o += 2; dig(&o, sec % 60, 2); o += 2;
	*o++ = 'Z';
	b_tlv(b, gen ? 0x18 : 0x17, s, o - s);
}

/* ---------------------------------------------------------------- commands */

static struct {
	const char *out, *key, *csr, *subj, *san, *cakey, *cacert;
	long days; int ca; int pathlen;
} opt = { .days = 3650, .pathlen = -1 };

static char defsubj[300];

static const char *subject(void)
{
	if (opt.subj) return opt.subj;
	char h[256] = "router";
	gethostname(h, sizeof h - 1); h[255] = 0;
	strcpy(defsubj, "/CN="); strcat(defsubj, h);
	return defsubj;
}

static void emit(const char *label, const u8 *d, size_t n, int secret)
{
	static char pem[MAXBUF * 2];
	writeout(opt.out, pem, pem_encode(label, d, n, pem), secret);
}

static void cmd_genkey(void)
{
	u8 priv[32], pub[64];
	if (!ec_make_key(pub, priv)) die("key generation failed (no entropy?)", 0);
	static buf b; b.n = 0;
	static const u8 v1[] = {0x02, 0x01, 0x01};
	b_add(&b, v1, 3);
	b_tlv(&b, 0x04, priv, 32);
	b_tlv(&b, 0xa0, OID_P256, sizeof OID_P256);
	size_t s = b.n; b_byte(&b, 0); b_byte(&b, 4); b_add(&b, pub, 64); b_wrap(&b, s, 0x03); b_wrap(&b, s, 0xa1);
	b_wrap(&b, 0, 0x30);
	emit("EC PRIVATE KEY", b.d, b.n, 1);
	memset(priv, 0, 32); memset(&b, 0, sizeof b);
}

static void cmd_pub(void)
{
	u8 priv[32], pub[64]; static buf b;
	load_key(opt.key, priv, pub);
	b_spki(&b, pub);
	emit("PUBLIC KEY", b.d, b.n, 0);
}

static void cmd_req(void)
{
	u8 priv[32], pub[64]; static buf b, ext;
	load_key(opt.key, priv, pub);
	static const u8 v0[] = {0x02, 0x01, 0x00};
	static const u8 EXTREQ[] = {0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x0e};
	b_add(&b, v0, 3);
	size_t ns = b.n; b_name(&b, subject());
	b_spki(&b, pub);
	size_t at = b.n;
	char cn[256], san[300];
	if (!opt.san && name_cn(b.d + ns, at - ns, cn, sizeof cn)) { auto_san(san, cn); opt.san = san; }
	if (opt.san && b_san(&ext, opt.san)) {
		b_add(&b, EXTREQ, sizeof EXTREQ);
		size_t s = b.n; b_add(&b, ext.d, ext.n); b_wrap(&b, s, 0x30); b_wrap(&b, s, 0x31);
		b_wrap(&b, at, 0x30);
	}
	b_wrap(&b, at, 0xa0);
	b_wrap(&b, 0, 0x30);
	b_sign(&b, priv);
	emit("CERTIFICATE REQUEST", b.d, b.n, 0);
}

/* parse a CSR: verify its signature, return subject and public key */
static void load_csr(const char *fn, const u8 **subj, size_t *sl, u8 pub[64],
                     const u8 **san, size_t *sanl)
{
	static const u8 EXTREQ[] = {0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x0e};
	static const u8 SAN[] = {0x06, 0x03, 0x55, 0x1d, 0x11};
	static char txt[MAXBUF + 1]; static u8 der[MAXBUF];
	readfile(fn, (u8 *)txt, sizeof txt - 1);
	size_t n = pem_decode(txt, "CERTIFICATE REQUEST", der, 0);
	if (!n) n = pem_decode(txt, "NEW CERTIFICATE REQUEST", der, 0);
	rd r = { der, der + n }, s; tlv t, cri, alg, sig, x;
	if (!r_expect(&r, &t, 0x30)) goto bad;
	r = r_in(&t);
	if (!r_expect(&r, &cri, 0x30) || !r_expect(&r, &alg, 0x30) || !r_expect(&r, &sig, 0x03)) goto bad;
	if (alg.rawlen != sizeof ALG_ECDSA256 || memcmp(alg.raw, ALG_ECDSA256, alg.rawlen))
		die("CSR is not ecdsa-with-SHA256", fn);
	s = r_in(&cri);
	if (!r_expect(&s, &x, 0x02) || !r_expect(&s, &x, 0x30)) goto bad;
	*subj = x.raw; *sl = x.rawlen;
	if (!r_expect(&s, &x, 0x30) || !parse_spki(&x, pub)) die("CSR key is not a valid P-256 key", fn);
	/* attributes [0] -> extensionRequest -> subjectAltName (copied verbatim) */
	*sanl = 0;
	if (r_expect(&s, &x, 0xa0)) {
		rd a = r_in(&x); tlv at, o, set, exts, ext;
		while (r_expect(&a, &at, 0x30)) {
			rd q = r_in(&at);
			if (!r_next(&q, &o) || !oid_is(&o, EXTREQ) || !r_expect(&q, &set, 0x31)) continue;
			rd e = r_in(&set);
			if (!r_expect(&e, &exts, 0x30)) continue;
			e = r_in(&exts);
			while (r_expect(&e, &ext, 0x30)) {
				rd z = r_in(&ext);
				if (r_next(&z, &o) && oid_is(&o, SAN)) { *san = ext.raw; *sanl = ext.rawlen; }
			}
		}
	}
	/* verify proof of possession */
	u8 h[32], rs[64] = {0};
	if (!sig.len || sig.c[0]) goto bad;
	rd q = { sig.c + 1, sig.c + sig.len };
	if (!r_expect(&q, &t, 0x30)) goto bad;
	q = r_in(&t);
	for (int i = 0; i < 2; i++) {
		if (!r_expect(&q, &x, 0x02)) goto bad;
		const u8 *p = x.c; size_t l = x.len;
		while (l > 32 && !*p) p++, l--;
		if (l > 32) goto bad;
		memcpy(rs + 32 * i + 32 - l, p, l);
	}
	sha256_once(cri.raw, cri.rawlen, h);
	if (!ec_verify(pub, h, rs)) die("CSR signature invalid", fn);
	return;
bad:
	die("malformed CSR", fn);
}

/* parse CA cert: subject (-> issuer), public key, subjectKeyIdentifier */
static void load_cacert(const char *fn, const u8 **subj, size_t *sl, u8 pub[64], u8 *ski, size_t *skil)
{
	static char txt[MAXBUF * 2 + 1]; static u8 der[MAXBUF * 2];
	readfile(fn, (u8 *)txt, sizeof txt - 1);
	size_t n = pem_decode(txt, "CERTIFICATE", der, 0);
	rd r = { der, der + n }; tlv t, x;
	static const u8 SKI[] = {0x06, 0x03, 0x55, 0x1d, 0x0e};
	*skil = 0;
	if (!r_expect(&r, &t, 0x30)) goto bad;
	r = r_in(&t);
	if (!r_expect(&r, &t, 0x30)) goto bad;
	r = r_in(&t);
	if (!r_next(&r, &x)) goto bad;
	if (x.tag == 0xa0 && !r_next(&r, &x)) goto bad;               /* version, serial */
	if (!r_expect(&r, &x, 0x30) || !r_expect(&r, &x, 0x30)) goto bad; /* sigalg, issuer */
	if (!r_expect(&r, &x, 0x30)) goto bad;                         /* validity */
	if (!r_expect(&r, &x, 0x30)) goto bad;                         /* subject */
	*subj = x.raw; *sl = x.rawlen;
	if (!r_expect(&r, &x, 0x30) || !parse_spki(&x, pub)) die("CA certificate key is not P-256", fn);
	while (r_next(&r, &x)) {
		if (x.tag != 0xa3) continue;
		rd e = r_in(&x); tlv seq, ext, o;
		if (!r_expect(&e, &seq, 0x30)) break;
		e = r_in(&seq);
		while (r_next(&e, &ext)) {
			rd q = r_in(&ext);
			if (!r_next(&q, &o) || !oid_is(&o, SKI)) continue;
			while (r_next(&q, &o) && o.tag != 0x04) ;
			rd v = r_in(&o); tlv id;
			if (o.tag == 0x04 && r_expect(&v, &id, 0x04) && id.len <= 32) { memcpy(ski, id.c, id.len); *skil = id.len; }
		}
	}
	return;
bad:
	die("malformed CA certificate", fn);
}

static void cmd_cert(void)
{
	u8 priv[32], pub[64], capriv[32], capub[64], capub2[64], ski[32], id[20], serial[16];
	size_t skil = 0, csl = 0, isl = 0, csrsanl = 0;
	const u8 *csrsubj = 0, *issuer = 0, *csrsan = 0;
	static buf b, ext, nm;
	static char cn[256], san[300];
	int self = !opt.cakey;

	if (!!opt.cakey != !!opt.cacert) die("-cakey and -cacert must be used together", 0);
	if (!!opt.key == !!opt.csr) die("need exactly one of -key or -csr", 0);
	if (opt.csr && self) die("-csr requires -cakey/-cacert", 0);
	if (opt.key) load_key(opt.key, priv, pub);
	else load_csr(opt.csr, &csrsubj, &csl, pub, &csrsan, &csrsanl);
	if (!self) {
		load_key(opt.cakey, capriv, capub);
		load_cacert(opt.cacert, &issuer, &isl, capub2, ski, &skil);
		if (memcmp(capub, capub2, 64)) die("-cakey does not match -cacert", 0);
	}

	/* subject name */
	if (csrsubj && !opt.subj) b_add(&nm, csrsubj, csl);
	else b_name(&nm, subject());
	if (self) { issuer = nm.d; isl = nm.n; }

	int64_t now = time(0);
	if (now < ECCERT_MIN_EPOCH) { warn("system clock not set, using build epoch as notBefore"); now = ECCERT_MIN_EPOCH; }
	now -= 300;   /* 5 min clock-skew tolerance */
	if (!rng(serial, 16)) die("no entropy", 0);
	serial[0] = (serial[0] & 0x7f) | 0x40;

	static const u8 v3[] = {0xa0, 0x03, 0x02, 0x01, 0x02};
	b_add(&b, v3, 5);
	b_uint(&b, serial, 16);
	b_add(&b, ALG_ECDSA256, sizeof ALG_ECDSA256);
	b_add(&b, issuer, isl);
	size_t s = b.n;
	b_time(&b, now);
	b_time(&b, opt.days <= 0 ? -1 : now + (int64_t)opt.days * 86400);
	b_wrap(&b, s, 0x30);
	b_add(&b, nm.d, nm.n);
	b_spki(&b, pub);

	/* extensions */
	if (opt.ca) {
		u8 bc[8] = {0x30, 0x03, 0x01, 0x01, 0xff}; size_t bl = 5;
		if (opt.pathlen >= 0) { bc[1] = 6; bc[5] = 2; bc[6] = 1; bc[7] = (u8)opt.pathlen; bl = 8; }
		b_ext(&ext, 0x13, 1, bc, bl);
		static const u8 ku[] = {0x03, 0x02, 0x01, 0x86};     /* digitalSignature keyCertSign cRLSign */
		b_ext(&ext, 0x0f, 1, ku, 4);
	} else {
		static const u8 bc[] = {0x30, 0x00};
		b_ext(&ext, 0x13, 1, bc, 2);
		static const u8 ku[] = {0x03, 0x02, 0x07, 0x80};     /* digitalSignature */
		b_ext(&ext, 0x0f, 1, ku, 4);
		static const u8 eku[] = {0x30, 0x14,
			0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01,   /* serverAuth */
			0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02};  /* clientAuth */
		b_ext(&ext, 0x25, 0, eku, sizeof eku);
		if (!opt.san && !csrsanl && name_cn(nm.d, nm.n, cn, sizeof cn)) { auto_san(san, cn); opt.san = san; }
	}
	if (opt.san) b_san(&ext, opt.san);
	else if (csrsanl) b_add(&ext, csrsan, csrsanl);
	keyid(pub, id);
	{ u8 v[22] = {0x04, 20}; memcpy(v + 2, id, 20); b_ext(&ext, 0x0e, 0, v, 22); }
	if (!self) {
		if (!skil) { keyid(capub, ski); skil = 20; }
		u8 v[36] = {0x30, (u8)(skil + 2), 0x80, (u8)skil}; memcpy(v + 4, ski, skil);
		b_ext(&ext, 0x23, 0, v, skil + 4);
	}
	s = b.n;
	b_add(&b, ext.d, ext.n); b_wrap(&b, s, 0x30); b_wrap(&b, s, 0xa3);
	b_wrap(&b, 0, 0x30);

	b_sign(&b, self ? priv : capriv);
	memset(priv, 0, 32); memset(capriv, 0, 32);
	emit("CERTIFICATE", b.d, b.n, 0);
}

/* ---------------------------------------------------------------- main */

static void usage(void)
{
	put(2,
	"usage: eccert genkey [-out key.pem]\n"
	"       eccert pub  -key key.pem [-out pub.pem]\n"
	"       eccert req  -key key.pem [-subj /CN=..] [-san ..] [-out csr.pem]\n"
	"       eccert cert (-key key.pem | -csr req.pem) [-subj /C=DE/O=..CN=..]\n"
	"                   [-san DNS:x,IP:1.2.3.4] [-days N (0=never)] [-ca [-pathlen N]]\n"
	"                   [-cakey ca.key -cacert ca.pem] [-out cert.pem]\n"
	"ECDSA P-256 / SHA-256 only. Without -cakey the certificate is self-signed.\n");
	_exit(2);
}

static long num(const char *s)
{
	long v = 0; int neg = *s == '-'; if (neg) s++;
	if (!*s) usage();
	for (; *s; s++) { if (*s < '0' || *s > '9') usage(); v = v * 10 + (*s - '0'); }
	return neg ? -v : v;
}

int main(int argc, char **argv)
{
	if (argc < 2) usage();
	ec_init();
	for (int i = 2; i < argc; i++) {
		const char *a = argv[i], *v = i + 1 < argc ? argv[i + 1] : 0;
		const char **sp = !strcmp(a, "-out") ? &opt.out : !strcmp(a, "-key") ? &opt.key :
			!strcmp(a, "-csr") ? &opt.csr : !strcmp(a, "-subj") ? &opt.subj :
			!strcmp(a, "-san") ? &opt.san : !strcmp(a, "-cakey") ? &opt.cakey :
			!strcmp(a, "-cacert") ? &opt.cacert : 0;
		if (sp) { if (!v) usage(); *sp = v; i++; }
		else if (!strcmp(a, "-days") && v) { opt.days = num(v); i++; }
		else if (!strcmp(a, "-pathlen") && v) { opt.pathlen = (int)num(v); i++; }
		else if (!strcmp(a, "-ca")) opt.ca = 1;
		else usage();
	}
	const char *c = argv[1];
	if (!strcmp(c, "genkey")) cmd_genkey();
	else if (!strcmp(c, "pub") && opt.key) cmd_pub();
	else if (!strcmp(c, "req") && opt.key) cmd_req();
	else if (!strcmp(c, "cert")) cmd_cert();
	else usage();
	return 0;
}
