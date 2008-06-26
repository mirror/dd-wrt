/*
 * mod_tls - An RFC2228 SSL/TLS module for ProFTPD
 *
 * Copyright (c) 2000-2002 Peter 'Luna' Runestig <peter@runestig.com>
 * Copyright (c) 2002-2007 TJ Saunders <tj@castaglia.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modifi-
 * cation, are permitted provided that the following conditions are met:
 *
 *    o Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    o Redistributions in binary form must reproduce the above copyright no-
 *      tice, this list of conditions and the following disclaimer in the do-
 *      cumentation and/or other materials provided with the distribution.
 *
 *    o The names of the contributors may not be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LI-
 * ABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEV-
 * ER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABI-
 * LITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  --- DO NOT DELETE BELOW THIS LINE ----
 *  $Libraries: -lssl -lcrypto$
 */

#include "conf.h"
#include "privs.h"

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#if OPENSSL_VERSION_NUMBER > 0x000907000L
# include <openssl/engine.h>
#endif

#include <signal.h>
#include <sys/resource.h>

#ifdef HAVE_MLOCK
# include <sys/mman.h>
#endif

#define MOD_TLS_VERSION		"mod_tls/2.1.2"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001021001 
# error "ProFTPD 1.2.10rc1 or later required"
#endif

extern session_t session;
extern xaset_t *server_list;

/* DH parameters */
static unsigned char dh512_p[] = {
  0xC0,0xC5,0x23,0x8D,0x3A,0xB3,0xA3,0x63,0x57,0xC0,0xD3,0xFE,
  0xD4,0xC2,0x8F,0x17,0x0E,0x7A,0xDB,0x8E,0x3B,0xB6,0xA5,0xC2,
  0x60,0x7D,0xE7,0x03,0xCC,0xA3,0x10,0xCC,0x82,0x39,0x3C,0x68,
  0xA0,0x82,0x9C,0x7A,0x4A,0x96,0x8C,0xB0,0x1A,0xB4,0xB8,0xA0,
  0x9E,0x64,0x9D,0x40,0x77,0x8A,0x9C,0x97,0x96,0x69,0x3D,0xCA,
  0xA8,0x25,0xAE,0xAB,
};

static unsigned char dh512_g[] = {
  0x02,
};

static DH *get_dh512(void) {
  DH *dh;

  if ((dh = DH_new()) == NULL)
    return NULL;

  dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), NULL);
  dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), NULL);

  if ((dh->p == NULL) || (dh->g == NULL))
    return NULL;

  return dh;
}

/*    
-----BEGIN DH PARAMETERS-----
MEYCQQDAxSONOrOjY1fA0/7Uwo8XDnrbjju2pcJgfecDzKMQzII5PGiggpx6SpaM
sBq0uKCeZJ1Ad4qcl5ZpPcqoJa6rAgEC
-----END DH PARAMETERS-----
*/

static unsigned char dh768_p[] = {
  0xB3,0x95,0x74,0xCE,0x0B,0xFD,0xAB,0xC3,0x53,0x9B,0x0B,0xFD,
  0x6E,0xB2,0x64,0x64,0x02,0xDD,0xFF,0x2E,0x77,0xEB,0x0D,0x6C,
  0xCE,0x04,0x2C,0x8E,0x5A,0xA7,0x96,0x45,0x54,0xA6,0x2F,0xBC,
  0xF9,0x77,0x1C,0x50,0x66,0x8E,0x48,0xA8,0x34,0xF0,0x81,0xDD,
  0x5B,0x5A,0xD4,0xA6,0x13,0x89,0x60,0x46,0x05,0x65,0x57,0x2C,
  0x1E,0x94,0x57,0x3C,0x3E,0x38,0xA6,0xFE,0x7B,0x03,0x7D,0x16,
  0x46,0xF6,0xB3,0x21,0x3C,0x44,0xF1,0xF1,0x90,0xCE,0x40,0x93,
  0x4B,0xE6,0xD6,0x0E,0x20,0x85,0xDA,0x9B,0x3F,0x5C,0x1F,0xDB,
};

static unsigned char dh768_g[] = {
  0x02,
};

static DH *get_dh768(void) {
  DH *dh;

  if ((dh = DH_new()) == NULL)
    return NULL;

  dh->p = BN_bin2bn(dh768_p, sizeof(dh768_p), NULL);
  dh->g = BN_bin2bn(dh768_g, sizeof(dh768_g), NULL);

  if ((dh->p == NULL) || (dh->g == NULL))
    return NULL;

  return dh;
}

/*
-----BEGIN DH PARAMETERS-----
MGYCYQCzlXTOC/2rw1ObC/1usmRkAt3/LnfrDWzOBCyOWqeWRVSmL7z5dxxQZo5I
qDTwgd1bWtSmE4lgRgVlVywelFc8Pjim/nsDfRZG9rMhPETx8ZDOQJNL5tYOIIXa
mz9cH9sCAQI=
-----END DH PARAMETERS-----
*/

static unsigned char dh1024_p[] = {
  0xC1,0xD8,0x9C,0x90,0xB1,0x58,0x7C,0xE1,0x56,0x70,0xD7,0x61,
  0x6C,0x00,0xE6,0xE7,0x99,0x04,0x9F,0x86,0xD9,0xB4,0x11,0x09,
  0x23,0x18,0xAA,0x19,0xCA,0x49,0x7C,0xA8,0x9D,0xF7,0x43,0x3A,
  0xAF,0xC3,0x1F,0x0E,0xAE,0xBB,0xF2,0xEA,0x5B,0x62,0xA1,0x5F,
  0x7C,0x26,0xA8,0xB4,0x5D,0x2A,0x25,0xAB,0x88,0x70,0x27,0x06,
  0xD0,0xF5,0x01,0xD9,0x6A,0x1F,0x48,0x2D,0x9C,0xEC,0xFE,0xA8,
  0x45,0x97,0x1D,0xC0,0x8A,0xFF,0xE5,0xE1,0x79,0xDF,0x85,0x31,
  0xFC,0x58,0x91,0x35,0xE8,0xC7,0xDA,0x55,0x7B,0xAA,0xDD,0xC2,
  0x0A,0x94,0x34,0xF7,0xB4,0x4A,0x91,0x3B,0x1E,0x16,0x89,0x2A,
  0x04,0x47,0x5D,0xE9,0x42,0x47,0x5E,0x30,0x61,0xE8,0x42,0xC1,
  0x23,0xC7,0x97,0x78,0x63,0x36,0x9D,0x3B,
};

static unsigned char dh1024_g[]={
  0x02,
};

static DH *get_dh1024(void) {
  DH *dh;

  if ((dh = DH_new()) == NULL)
    return NULL;

  dh->p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), NULL);
  dh->g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), NULL);

  if ((dh->p == NULL) || (dh->g == NULL))
    return NULL;

  return(dh);
}
/*
-----BEGIN DH PARAMETERS-----
MIGHAoGBAMHYnJCxWHzhVnDXYWwA5ueZBJ+G2bQRCSMYqhnKSXyonfdDOq/DHw6u
u/LqW2KhX3wmqLRdKiWriHAnBtD1AdlqH0gtnOz+qEWXHcCK/+Xhed+FMfxYkTXo
x9pVe6rdwgqUNPe0SpE7HhaJKgRHXelCR14wYehCwSPHl3hjNp07AgEC
-----END DH PARAMETERS-----
*/

static unsigned char dh1536_p[] = {
  0xDA,0x68,0x25,0x7F,0x9D,0xB5,0x3F,0x42,0x05,0xBC,0x79,0x65,
  0x6F,0x19,0x6A,0x6F,0x70,0x11,0x91,0xF2,0x08,0x48,0x2B,0xE2,
  0x0C,0x15,0xD9,0x31,0xE7,0x3A,0x50,0x32,0x9F,0xFB,0xD6,0x56,
  0xFA,0xB4,0xA9,0x5F,0x22,0x17,0x52,0x72,0x2C,0xE3,0x5D,0xA1,
  0xA8,0xEF,0x16,0x42,0x35,0xC6,0xD9,0x64,0xC1,0xB3,0xB3,0x4C,
  0x09,0x90,0xF4,0x49,0xEF,0xDE,0x64,0x99,0xFF,0x3C,0x37,0x0A,
  0x91,0xA4,0x9E,0x38,0x27,0xF2,0x96,0x13,0x1E,0x15,0xA2,0x52,
  0xF1,0x54,0x0C,0xED,0x5C,0x38,0xC4,0xEC,0xFF,0xE2,0xFA,0x0A,
  0x41,0xBB,0x48,0x5D,0xD3,0x54,0xA1,0xEB,0xBD,0x1F,0x68,0xED,
  0x2A,0x49,0x7F,0x68,0x52,0xB3,0xA0,0x77,0x3E,0x19,0xFB,0x44,
  0xCD,0x4B,0x21,0x3E,0x3B,0xBA,0xF6,0xA2,0x36,0x37,0xE5,0xFA,
  0x95,0xB0,0x7D,0x7B,0x58,0x96,0xC4,0xC9,0xC0,0xCF,0xD9,0x3F,
  0xA3,0x42,0x0B,0xD7,0xBE,0x1A,0xA8,0xB5,0x57,0x58,0xF4,0x04,
  0x97,0x54,0xB0,0x59,0x23,0x5F,0x98,0x09,0x90,0xC0,0x49,0x85,
  0x40,0x23,0x2D,0x21,0x3E,0xB0,0x07,0x06,0x07,0x32,0xFB,0xB9,
  0x91,0x40,0x92,0x09,0xED,0x07,0x80,0x05,0x14,0x5B,0xC1,0x9B,
};

static unsigned char dh1536_g[] = {
  0x02,
};

static DH *get_dh1536(void) {
  DH *dh;

  if ((dh = DH_new()) == NULL)
    return NULL;

  dh->p = BN_bin2bn(dh1536_p, sizeof(dh1536_p), NULL);
  dh->g = BN_bin2bn(dh1536_g, sizeof(dh1536_g), NULL);

  if ((dh->p == NULL) || (dh->g == NULL))
    return NULL;

  return dh;
}

/*
-----BEGIN DH PARAMETERS-----
MIHHAoHBANpoJX+dtT9CBbx5ZW8Zam9wEZHyCEgr4gwV2THnOlAyn/vWVvq0qV8i
F1JyLONdoajvFkI1xtlkwbOzTAmQ9Env3mSZ/zw3CpGknjgn8pYTHhWiUvFUDO1c
OMTs/+L6CkG7SF3TVKHrvR9o7SpJf2hSs6B3Phn7RM1LIT47uvaiNjfl+pWwfXtY
lsTJwM/ZP6NCC9e+Gqi1V1j0BJdUsFkjX5gJkMBJhUAjLSE+sAcGBzL7uZFAkgnt
B4AFFFvBmwIBAg==
-----END DH PARAMETERS-----
*/

static unsigned char dh2048_p[] = {
  0xD0,0xE6,0xFF,0x1F,0x39,0xE0,0xCC,0x85,0xAC,0xA4,0xE6,0xDD,
  0x06,0xE5,0x2D,0xBF,0xEA,0x64,0x2E,0xC7,0x99,0x8A,0x0F,0xCB,
  0x3C,0x9D,0xEE,0xAC,0x61,0xFF,0x69,0x31,0x71,0xFE,0x2F,0x7B,
  0x65,0x95,0xA0,0xA4,0x59,0xB8,0xE3,0x66,0x5B,0x3F,0xD8,0x42,
  0x99,0x4F,0x09,0x44,0xC5,0x8D,0x8B,0x5D,0x16,0xAA,0x05,0x6E,
  0x8B,0x11,0x59,0x1F,0xD7,0x11,0x84,0x87,0x4D,0xBE,0xBB,0xBA,
  0x9A,0xF0,0xC3,0xE2,0x0E,0xB8,0x0F,0xFD,0x08,0xB1,0x48,0x98,
  0xDE,0x89,0xDA,0x00,0x15,0x04,0xA4,0x51,0xBE,0x5B,0x60,0x0A,
  0x0E,0x20,0xAC,0xC5,0x83,0x5D,0xC4,0x0F,0xA3,0x8E,0x11,0x66,
  0x2C,0xD3,0x61,0x5F,0x16,0x83,0xAA,0xCF,0x52,0x9C,0x7D,0x75,
  0xEA,0xCA,0x67,0xA3,0xAB,0x58,0x9F,0x67,0x17,0xA0,0x54,0x3A,
  0x2B,0xCA,0xB5,0x03,0x7E,0x50,0xBD,0x99,0x1E,0xEF,0xB2,0x8F,
  0xB4,0xFB,0xD2,0x2D,0x6A,0xA9,0xA2,0xC0,0xD4,0xD2,0x68,0x6C,
  0x21,0x71,0x78,0x75,0x82,0x4C,0xD8,0xE8,0x2C,0x0B,0xC9,0x3F,
  0xF6,0xF0,0x64,0xD9,0x6E,0x76,0xCB,0xBB,0x99,0xFB,0xBC,0x15,
  0x54,0x7B,0x7F,0x97,0x36,0x8F,0x0B,0x1C,0xFF,0xDD,0x28,0x99,
  0xE5,0x3A,0xAD,0xCD,0x84,0xAB,0xA1,0xEF,0xB2,0x21,0xEA,0xD6,
  0x49,0x22,0x6A,0x30,0x6A,0x63,0x2E,0x52,0x79,0xCF,0xBC,0xC2,
  0xB6,0x2E,0xA5,0x5D,0xB3,0xDA,0xC2,0xDD,0x02,0xEA,0x26,0x2F,
  0x3B,0x0A,0x12,0xBB,0xA2,0xEF,0x2B,0xFA,0xCC,0x25,0x63,0x1B,
  0xC3,0x00,0x18,0x8F,0x36,0xB7,0x30,0x5A,0x55,0x1A,0xE0,0x12,
  0xA1,0xD2,0x9C,0x93,
};

static unsigned char dh2048_g[] = {
  0x02,
};

static DH *get_dh2048(void) {
  DH *dh;

  if ((dh = DH_new()) == NULL)
    return NULL;

  dh->p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), NULL);
  dh->g = BN_bin2bn(dh2048_g, sizeof(dh2048_g), NULL);

  if ((dh->p == NULL) || (dh->g == NULL))
    return NULL;

  return dh;
}

/*
-----BEGIN DH PARAMETERS-----
MIIBCAKCAQEA0Ob/HzngzIWspObdBuUtv+pkLseZig/LPJ3urGH/aTFx/i97ZZWg
pFm442ZbP9hCmU8JRMWNi10WqgVuixFZH9cRhIdNvru6mvDD4g64D/0IsUiY3ona
ABUEpFG+W2AKDiCsxYNdxA+jjhFmLNNhXxaDqs9SnH116spno6tYn2cXoFQ6K8q1
A35QvZke77KPtPvSLWqposDU0mhsIXF4dYJM2OgsC8k/9vBk2W52y7uZ+7wVVHt/
lzaPCxz/3SiZ5TqtzYSroe+yIerWSSJqMGpjLlJ5z7zCti6lXbPawt0C6iYvOwoS
u6LvK/rMJWMbwwAYjza3MFpVGuASodKckwIBAg==
-----END DH PARAMETERS-----
*/

/* ASN1_BIT_STRING_cmp was renamed in 0.9.5 */
#if OPENSSL_VERSION_NUMBER < 0x00905100L
# define M_ASN1_BIT_STRING_cmp ASN1_BIT_STRING_cmp
#endif

/* From src/dirtree.c */
extern int ServerUseReverseDNS;

module tls_module;

typedef struct tls_pkey_obj {
  struct tls_pkey_obj *next;

  size_t pkeysz;

  char *rsa_pkey;
  void *rsa_pkey_ptr;

  char *dsa_pkey;
  void *dsa_pkey_ptr;

  unsigned int flags;

  server_rec *server;

} tls_pkey_t;

#define TLS_PKEY_USE_RSA		0x0100
#define TLS_PKEY_USE_DSA		0x0200

static tls_pkey_t *tls_pkey_list = NULL;
static unsigned int tls_npkeys = 0;

#define TLS_DEFAULT_CIPHER_SUITE	"ALL:!ADH"
#define TLS_DEFAULT_PROTOCOL		"SSLv23"

/* Module variables */
#if OPENSSL_VERSION_NUMBER > 0x000907000L
static const char *tls_crypto_device = NULL;
#endif
static unsigned char tls_engine = FALSE;
static unsigned long tls_flags = 0UL, tls_opts = 0UL;
static tls_pkey_t *tls_pkey = NULL;
static int tls_logfd = -1;
static char *tls_logname = NULL;

static char *tls_passphrase_provider = NULL;
#define TLS_PASSPHRASE_TIMEOUT		10
#define TLS_PASSPHRASE_FL_RSA_KEY	0x0001
#define TLS_PASSPHRASE_FL_DSA_KEY	0x0002

static char *tls_protocol = TLS_DEFAULT_PROTOCOL;
static unsigned char tls_required_on_auth = FALSE;
static unsigned char tls_required_on_ctrl = FALSE;
static unsigned char tls_required_on_data = FALSE;
static unsigned char *tls_authenticated = NULL;

/* mod_tls session flags */
#define	TLS_SESS_ON_CTRL		0x0001
#define TLS_SESS_ON_DATA		0x0002
#define TLS_SESS_PBSZ_OK		0x0004
#define TLS_SESS_TLS_REQUIRED		0x0010
#define TLS_SESS_VERIFY_CLIENT		0x0020
#define TLS_SESS_NO_PASSWD_NEEDED	0x0040
#define TLS_SESS_NEED_DATA_PROT		0x0100
#define TLS_SESS_CTRL_RENEGOTIATING	0x0200
#define TLS_SESS_DATA_RENEGOTIATING	0x0400
#define TLS_SESS_HAVE_CCC		0x0800

/* mod_tls option flags */
#define TLS_OPT_NO_CERT_REQUEST		0x0001
#define TLS_OPT_VERIFY_CERT_FQDN	0x0002
#define TLS_OPT_VERIFY_CERT_IP_ADDR	0x0004
#define TLS_OPT_ALLOW_DOT_LOGIN		0x0008
#define TLS_OPT_EXPORT_CERT_DATA	0x0010
#define TLS_OPT_STD_ENV_VARS		0x0020
#define TLS_OPT_ALLOW_PER_USER		0x0040

/* mod_tls cleanup flags */
#define TLS_CLEANUP_FL_SESS_INIT	0x0001

static char *tls_cipher_suite = NULL;
static char *tls_crl_file = NULL, *tls_crl_path = NULL;
static char *tls_dhparam_file = NULL;
static char *tls_dsa_cert_file = NULL, *tls_dsa_key_file = NULL;
static char *tls_rsa_cert_file = NULL, *tls_rsa_key_file = NULL;
static char *tls_rand_file = NULL;

/* Timeout given for TLS handshakes.  The default is 5 minutes. */
static unsigned int tls_handshake_timeout = 300;
static unsigned char tls_handshake_timed_out = FALSE;
static int tls_handshake_timer_id = -1;

/* Note: 9 is the default OpenSSL depth. */
static int tls_verify_depth = 9;

#if OPENSSL_VERSION_NUMBER > 0x000907000L
/* Renegotiate control channel on TLS sessions after 4 hours, by default. */
static int tls_ctrl_renegotiate_timeout = 14400;

/* Renegotiate data channel on TLS sessions after 1 gigabyte, by default. */
static off_t tls_data_renegotiate_limit = 1024 * 1024 * 1024;

/* Timeout given for renegotiations to occur before the TLS session is
 * shutdown.  The default is 30 seconds.
 */
static int tls_renegotiate_timeout = 30;

/* Is client acceptance of a requested renegotiation required? */
static unsigned char tls_renegotiate_required = TRUE;
#endif

static pr_netio_t *tls_ctrl_netio = NULL;
static pr_netio_stream_t *tls_ctrl_rd_nstrm = NULL;
static pr_netio_stream_t *tls_ctrl_wr_nstrm = NULL;

static pr_netio_t *tls_data_netio = NULL;
static pr_netio_stream_t *tls_data_rd_nstrm = NULL;
static pr_netio_stream_t *tls_data_wr_nstrm = NULL;

/* OpenSSL variables */
static SSL *ctrl_ssl = NULL;
static SSL_CTX *ssl_ctx = NULL;
static X509_STORE *tls_crl_store = NULL;
static DH *tls_tmp_dh = NULL;
static RSA *tls_tmp_rsa = NULL;

/* SSL/TLS support functions */
static void tls_closelog(void);
static void tls_end_sess(SSL *, int, int);
static void tls_fatal_error(int, int);
static const char *tls_get_errors(void);
static char *tls_get_page(size_t, void **);
static size_t tls_get_pagesz(void);
static char *tls_get_subj_name(void);

static int tls_log(const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 1, 2)));
#else
       ;
#endif

static int tls_openlog(void);
static RSA *tls_rsa_cb(SSL *, int, int);
static int tls_seed_prng(void);
static void tls_setup_environ(SSL *);
static int tls_verify_cb(int, X509_STORE_CTX *);
static int tls_verify_crl(int, X509_STORE_CTX *);
static char *tls_x509_name_oneline(X509_NAME *);

static unsigned char tls_check_client_cert(SSL *ssl, conn_t *conn) {
  X509 *cert = NULL;
  STACK_OF(GENERAL_NAME) *sk_alt_names;
  unsigned char ok = FALSE, have_dns_ext = FALSE, have_ipaddr_ext = FALSE;

  /* Only perform these more stringent checks if asked to verify clients. */
  if (!(tls_flags & TLS_SESS_VERIFY_CLIENT))
    return TRUE;

  /* Only perform these checks is configured to do so. */
  if (!(tls_opts & TLS_OPT_VERIFY_CERT_FQDN) &&
      !(tls_opts & TLS_OPT_VERIFY_CERT_IP_ADDR))
    return TRUE;

  /* First, check the subjectAltName X509v3 extensions, as is proper, for
   * the IP address and FQDN.  If enough people clamor for backward
   * compatibility, I'll amend this to check commonName later.  Otherwise,
   * for now, only look in the extensions.
   */

  /* Note: this should _never_ return NULL in this case. */
  cert = SSL_get_peer_certificate(ssl);

  sk_alt_names = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
  if (sk_alt_names) {
    register unsigned int i;
    int nnames = sk_GENERAL_NAME_num(sk_alt_names);

    for (i = 0; i < nnames; i++) {
      GENERAL_NAME *name = sk_GENERAL_NAME_value(sk_alt_names, i);

      /* Only interested in the DNS and IP address types right now. */
      switch (name->type) {
        case GEN_DNS:
          if (tls_opts & TLS_OPT_VERIFY_CERT_FQDN) {
            const char *cert_dns_name = (const char *) name->d.ia5->data;
            have_dns_ext = TRUE;

            if (strcmp(cert_dns_name, conn->remote_name) != 0) {
              tls_log("client cert dNSName value '%s' != client FQDN '%s'",
                cert_dns_name, conn->remote_name);

              GENERAL_NAME_free(name);
              sk_GENERAL_NAME_free(sk_alt_names);
              X509_free(cert);
              return FALSE;
            }

            tls_log("%s", "client cert dNSName matches client FQDN");
            ok = TRUE;
            continue;
          }
          break;

        case GEN_IPADD:
          if (tls_opts & TLS_OPT_VERIFY_CERT_IP_ADDR) {
            char cert_ipstr[INET_ADDRSTRLEN] = {'\0'};
            const char *cert_ipaddr = (const char *) name->d.ia5->data;

            /* Note: OpenSSL doesn't support IPv6 addresses in the
             * ipAddress name yet.
             */
            sprintf(cert_ipstr, "%u.%u.%u.%u", cert_ipaddr[0],
              cert_ipaddr[1], cert_ipaddr[2], cert_ipaddr[3]);
            have_ipaddr_ext = TRUE;

            if (strcmp(cert_ipstr, pr_netaddr_get_ipstr(conn->remote_addr))) {
              tls_log("client cert iPAddress value '%s' != client IP '%s'",
                cert_ipstr, pr_netaddr_get_ipstr(conn->remote_addr));

              GENERAL_NAME_free(name);
              sk_GENERAL_NAME_free(sk_alt_names);
              X509_free(cert);
              return FALSE;
            }

            tls_log("%s", "client cert iPAddress matches client IP");
            ok = TRUE;
            continue;
          }
          break;

        default:
          break;
      }

      GENERAL_NAME_free(name);
    } 

    sk_GENERAL_NAME_free(sk_alt_names);
  }

  if ((tls_opts & TLS_OPT_VERIFY_CERT_FQDN) && !have_dns_ext)
    tls_log("%s", "client cert missing required X509v3 subjectAltName dNSName");

  if ((tls_opts & TLS_OPT_VERIFY_CERT_IP_ADDR) && !have_ipaddr_ext)
    tls_log("%s", "client cert missing required X509v3 subjectAltName iPAddress");

  X509_free(cert);

  if (!ok)
    return FALSE;

  return TRUE;
}

struct tls_pkey_data {
  server_rec *s;
  int flags;
  char *buf;
  size_t buflen;
  const char *prompt;
};

static void tls_prepare_provider_fds(int stdout_fd, int stderr_fd) {
  unsigned long nfiles = 0;
  register unsigned int i = 0;
  struct rlimit rlim;

  if (stdout_fd != STDOUT_FILENO) {
    if (dup2(stdout_fd, STDOUT_FILENO) < 0)
      tls_log("error duping fd %d to stdout: %s", stdout_fd, strerror(errno));

    close(stdout_fd);
  }

  if (stderr_fd != STDERR_FILENO) {
    if (dup2(stderr_fd, STDERR_FILENO) < 0)
      tls_log("error duping fd %d to stderr: %s", stderr_fd, strerror(errno));

    close(stderr_fd);
  }

  /* Make sure not to pass on open file descriptors. For stdout and stderr,
   * we dup some pipes, so that we can capture what the command may write
   * to stdout or stderr.  The stderr output will be logged to the TLSLog.
   *
   * First, use getrlimit() to obtain the maximum number of open files
   * for this process -- then close that number.
   */
#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
# if defined(RLIMIT_NOFILE)
  if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
# elif defined(RLIMIT_OFILE)
  if (getrlimit(RLIMIT_OFILE, &rlim) < 0) {
# endif
    tls_log("getrlimit error: %s", strerror(errno));

    /* Pick some arbitrary high number. */
    nfiles = 1024;

  } else
    nfiles = rlim.rlim_max;
#else /* no RLIMIT_NOFILE or RLIMIT_OFILE */
   nfiles = 1024;
#endif

  /* Close the "non-standard" file descriptors. */
  for (i = 3; i < nfiles; i++)
    (void) close(i);

  return;
}

static void tls_prepare_provider_pipes(int *stdout_pipe, int *stderr_pipe) {
  if (pipe(stdout_pipe) < 0) {
    tls_log("error opening stdout pipe: %s", strerror(errno));
    stdout_pipe[0] = -1;
    stdout_pipe[1] = STDOUT_FILENO;

  } else {
    if (fcntl(stdout_pipe[0], F_SETFD, FD_CLOEXEC) < 0)
      tls_log("error setting close-on-exec flag on stdout pipe read fd: %s",
        strerror(errno));

    if (fcntl(stdout_pipe[1], F_SETFD, 0) < 0)
      tls_log("error setting close-on-exec flag on stdout pipe write fd: %s",
        strerror(errno));
  }

  if (pipe(stderr_pipe) < 0) {
    tls_log("error opening stderr pipe: %s", strerror(errno));
    stderr_pipe[0] = -1;
    stderr_pipe[1] = STDERR_FILENO;

  } else {
    if (fcntl(stderr_pipe[0], F_SETFD, FD_CLOEXEC) < 0)
      tls_log("error setting close-on-exec flag on stderr pipe read fd: %s",
        strerror(errno));

    if (fcntl(stderr_pipe[1], F_SETFD, 0) < 0)
      tls_log("error setting close-on-exec flag on stderr pipe write fd: %s",
        strerror(errno));
  }
}

static int tls_exec_passphrase_provider(server_rec *s, char *buf, int buflen,
    int flags) {
  pid_t pid;
  int status;
  int stdout_pipe[2], stderr_pipe[2];

  struct sigaction sa_ignore, sa_intr, sa_quit;
  sigset_t set_chldmask, set_save;

  /* Prepare signal dispositions. */
  sa_ignore.sa_handler = SIG_IGN;
  sigemptyset(&sa_ignore.sa_mask);
  sa_ignore.sa_flags = 0;

  if (sigaction(SIGINT, &sa_ignore, &sa_intr) < 0)
    return -1;

  if (sigaction(SIGQUIT, &sa_ignore, &sa_quit) < 0)
    return -1;

  sigemptyset(&set_chldmask);
  sigaddset(&set_chldmask, SIGCHLD);

  if (sigprocmask(SIG_BLOCK, &set_chldmask, &set_save) < 0)
    return -1;

  tls_prepare_provider_pipes(stdout_pipe, stderr_pipe);

  pid = fork();
  if (pid < 0) {
    pr_log_pri(PR_LOG_ERR, MOD_TLS_VERSION ": error: unable to fork: %s",
      strerror(errno));
    status = -1;

  } else if (pid == 0) {
    char nbuf[32] = {'\0'};
    pool *tmp_pool;
    char *stdin_argv[4];

    /* Child process */

    /* Note: there is no need to clean up this temporary pool, as we've
     * forked.  If the exec call succeeds, this child process will exit
     * normally, and its process space recovered by the OS.  If the exec
     * call fails, we still exit, and the process space is recovered by
     * the OS.  Either way, the memory will be cleaned up without need for
     * us to do it explicitly (unless one wanted to be pedantic about it,
     * of course).
     */
    tmp_pool = make_sub_pool(s->pool);

    /* Restore previous signal actions. */
    sigaction(SIGINT, &sa_intr, NULL);
    sigaction(SIGQUIT, &sa_quit, NULL);
    sigprocmask(SIG_SETMASK, &set_save, NULL);

    stdin_argv[0] = pstrdup(tmp_pool, tls_passphrase_provider);

    snprintf(nbuf, sizeof(buf)-1, "%u", s->ServerPort);
    nbuf[sizeof(nbuf)-1] = '\0';
    stdin_argv[1] = pstrcat(tmp_pool, s->ServerName, ":", nbuf, NULL);

    stdin_argv[2] = pstrdup(tmp_pool, flags & TLS_PASSPHRASE_FL_RSA_KEY ?
      "RSA" : "DSA");
    stdin_argv[3] = NULL;

    PRIVS_ROOT

    pr_log_debug(DEBUG6, MOD_TLS_VERSION
      ": executing '%s' with uid %lu (euid %lu), gid %lu (egid %lu)",
      tls_passphrase_provider,
      (unsigned long) getuid(), (unsigned long) geteuid(),
      (unsigned long) getgid(), (unsigned long) getegid());

    /* Prepare the file descriptors that the process will inherit. */
    tls_prepare_provider_fds(stdout_pipe[1], stderr_pipe[1]);

    errno = 0;
    execv(tls_passphrase_provider, stdin_argv);

    /* Since all previous file descriptors (including those for log files)
     * have been closed, and root privs have been revoked, there's little
     * chance of directing a message of execv() failure to proftpd's log
     * files.  execv() only returns if there's an error; the only way we
     * can signal this to the waiting parent process is to exit with a
     * non-zero value (the value of errno will do nicely).
     */

    exit(errno);

  } else {
    int res;
    int maxfd, fds, send_sigterm = 1;
    fd_set readfds;
    time_t start_time = time(NULL);
    struct timeval tv;

    /* Parent process */

    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    maxfd = (stderr_pipe[0] > stdout_pipe[0]) ?
      stderr_pipe[0] : stdout_pipe[0];

    res = waitpid(pid, &status, WNOHANG);
    while (res <= 0) {
      if (res < 0) {
        if (errno != EINTR) {
          pr_log_debug(DEBUG2, MOD_TLS_VERSION
            ": passphrase provider error: unable to wait for pid %u: %s",
            (unsigned int) pid, strerror(errno));
          status = -1;
          break;

        } else
          pr_signals_handle();
      }

      /* Check the time elapsed since we started. */
      if ((time(NULL) - start_time) > TLS_PASSPHRASE_TIMEOUT) {

        /* Send TERM, the first time, to be polite. */
        if (send_sigterm) {
          send_sigterm = 0;
          pr_log_debug(DEBUG6, MOD_TLS_VERSION
            ": '%s' has exceeded the timeout (%lu seconds), sending "
            "SIGTERM (signal %d)", tls_passphrase_provider,
            (unsigned long) TLS_PASSPHRASE_TIMEOUT, SIGTERM);
          kill(pid, SIGTERM);

        } else {
          /* The child is still around?  Terminate with extreme prejudice. */
          pr_log_debug(DEBUG6, MOD_TLS_VERSION
            ": '%s' has exceeded the timeout (%lu seconds), sending "
            "SIGKILL (signal %d)", tls_passphrase_provider,
            (unsigned long) TLS_PASSPHRASE_TIMEOUT, SIGKILL);
          kill(pid, SIGKILL);
        }
      }

      /* Select on the pipe read fds, to see if the child has anything
       * to tell us.
       */
      FD_ZERO(&readfds);

      FD_SET(stdout_pipe[0], &readfds);
      FD_SET(stderr_pipe[0], &readfds);

      /* Note: this delay should be configurable somehow. */
      tv.tv_sec = 2L;
      tv.tv_usec = 0L;

      fds = select(maxfd + 1, &readfds, NULL, NULL, &tv);

      if (fds == -1 &&
          errno == EINTR)
        pr_signals_handle();

      if (fds > 0) {
        /* The child sent us something.  How thoughtful. */

        if (FD_ISSET(stdout_pipe[0], &readfds)) {
          res = read(stdout_pipe[0], buf, buflen);
          if (res > 0) {
              while (res && (buf[res-1] == '\r' || buf[res-1] == '\n'))
                res--;
              buf[res] = '\0';

          } else if (res < 0){
            pr_log_debug(DEBUG2, MOD_TLS_VERSION
              ": error reading stdout from '%s': %s",
              tls_passphrase_provider, strerror(errno));
          }
        }

        if (FD_ISSET(stderr_pipe[0], &readfds)) {
          int stderrlen;
          char stderrbuf[PIPE_BUF];

          memset(stderrbuf, '\0', sizeof(stderrbuf));
          stderrlen = read(stderr_pipe[0], stderrbuf, sizeof(stderrbuf)-1);
          if (stderrlen > 0) {
            while (stderrlen &&
                   (stderrbuf[stderrlen-1] == '\r' ||
                    stderrbuf[stderrlen-1] == '\n'))
              stderrlen--;
            stderrbuf[stderrlen] = '\0';

            pr_log_debug(DEBUG5, MOD_TLS_VERSION
              ": stderr from '%s': %s", tls_passphrase_provider,
              stderrbuf);

          } else if (res < 0) {
            pr_log_debug(DEBUG2, MOD_TLS_VERSION
              ": error reading stderr from '%s': %s",
              tls_passphrase_provider, strerror(errno));
          }
        }
      }

      res = waitpid(pid, &status, WNOHANG);
    }
  }

  /* Restore the previous signal actions. */
  if (sigaction(SIGINT, &sa_intr, NULL) < 0)
    return -1;

  if (sigaction(SIGQUIT, &sa_quit, NULL) < 0)
    return -1; 

  if (sigprocmask(SIG_SETMASK, &set_save, NULL) < 0)
    return -1;

  if (WIFSIGNALED(status)) {
    pr_log_debug(DEBUG2, MOD_TLS_VERSION
      ": '%s' died from signal %d", tls_passphrase_provider,
      WTERMSIG(status));
    errno = EPERM;
    return -1;
  }

  return 0;
}

static int tls_passphrase_cb(char *buf, int buflen, int rwflag, void *d) {
  static int need_banner = TRUE;
  struct tls_pkey_data *pdata = d;

  if (!tls_passphrase_provider) {
    register unsigned int attempt;
    int pwlen = 0;

    tls_log("requesting passphrase from admin");

    /* Similar to Apache's mod_ssl, we want to be nice, and display an
     * informative message to the proftpd admin, telling them for what
     * server they are being requested to provide a passphrase.  
     */

    if (need_banner) {
      fprintf(stderr, "\nPlease provide passphrases for these encrypted certificate keys:\n");
      need_banner = FALSE;
    }

    /* You get three attempts at entering the passphrase correctly. */
    for (attempt = 0; attempt < 3; attempt++) {
      int res;

      /* Always handle signals in a loop. */
      pr_signals_handle();

      res = EVP_read_pw_string(buf, buflen, pdata->prompt, TRUE);

      /* A return value of zero from EVP_read_pw_string() means success; -1
       * means a system error occurred, and 1 means user interaction problems.
       */
      if (res != 0) {
         fprintf(stderr, "\nPassphrases do not match.  Please try again.\n");
         continue;
      }

      pwlen = strlen(buf);
      if (pwlen < 1)
        fprintf(stderr, "Error: passphrase must be at least one character\n");

      else {
        sstrncpy(pdata->buf, buf, pdata->buflen);
        return pwlen;
      }
    }

  } else {
    tls_log("requesting passphrase from '%s'", tls_passphrase_provider);

    if (tls_exec_passphrase_provider(pdata->s, buf, buflen, pdata->flags) < 0) {
      tls_log("error obtaining passphrase from '%s': %s",
        tls_passphrase_provider, strerror(errno));

    } else {
      sstrncpy(pdata->buf, buf, pdata->buflen);
      return strlen(buf);
    }
  }

#if OPENSSL_VERSION_NUMBER < 0x00908001
  PEMerr(PEM_F_DEF_CALLBACK, PEM_R_PROBLEMS_GETTING_PASSWORD);
#else
  PEMerr(PEM_F_PEM_DEF_CALLBACK, PEM_R_PROBLEMS_GETTING_PASSWORD);
#endif

  pr_memscrub(buf, buflen);
  return -1;
}

static int tls_get_passphrase(server_rec *s, const char *path,
    const char *prompt, char *buf, size_t buflen, int flags) {
  FILE *keyf;
  EVP_PKEY *pkey = NULL;
  int prompt_fd = -1;
  struct tls_pkey_data pdata;
  register unsigned int attempt;

  /* Open an fp on the cert file. */
  PRIVS_ROOT
  keyf = fopen(path, "r");
  PRIVS_RELINQUISH

  if (!keyf) {
    SYSerr(SYS_F_FOPEN, errno);
    return -1;
  }

  pdata.s = s;
  pdata.flags = flags;
  pdata.buf = buf;
  pdata.buflen = buflen;
  pdata.prompt = prompt;

  /* Reconnect stderr to the term because proftpd connects stderr, earlier,
   * to the general stderr logfile.
   */
  prompt_fd = open("/dev/null", O_WRONLY);
  if (prompt_fd == -1)
    /* This is an arbitrary, meaningless placeholder number. */
    prompt_fd = 76;

  dup2(STDERR_FILENO, prompt_fd);
  dup2(STDOUT_FILENO, STDERR_FILENO);

  /* The user gets three tries to enter the correct passphrase. */
  for (attempt = 0; attempt < 3; attempt++) {

    /* Always handle signals in a loop. */
    pr_signals_handle();

    pkey = PEM_read_PrivateKey(keyf, NULL, tls_passphrase_cb, &pdata);
    if (pkey)
      break;

    fseek(keyf, 0, SEEK_SET);
    ERR_clear_error();
    fprintf(stderr, "\nWrong passphrase for this key.  Please try again.\n");
  }
  fclose(keyf);

  /* Restore the normal stderr logging. */
  dup2(prompt_fd, STDERR_FILENO);
  close(prompt_fd);

  if (pkey == NULL)
    return -1;

#ifdef HAVE_MLOCK
   PRIVS_ROOT
   if (mlock(buf, buflen) < 0) 
     pr_log_debug(DEBUG1, MOD_TLS_VERSION
       ": error locking passphrase into memory: %s", strerror(errno));
   else
     pr_log_debug(DEBUG1, MOD_TLS_VERSION ": passphrase locked into memory");
   PRIVS_RELINQUISH
#endif

  EVP_PKEY_free(pkey);
  return 0;
}

static int tls_handshake_timeout_cb(CALLBACK_FRAME) {
  tls_handshake_timed_out = TRUE;
  return 0;
}

static tls_pkey_t *tls_lookup_pkey(void) {
  tls_pkey_t *k, *pkey = NULL;

  for (k = tls_pkey_list; k; k = k->next) {

    /* If this pkey matches the current server_rec, mark it and move on. */
    if (k->server == main_server) {

#ifdef HAVE_MLOCK
      /* mlock() the passphrase memory areas again; page locks are not
       * inherited across forks.
       */
      PRIVS_ROOT
      if (k->rsa_pkey)
        if (mlock(k->rsa_pkey, k->pkeysz) < 0)
          tls_log("error locking passphrase into memory: %s", strerror(errno));

      if (k->dsa_pkey)
        if (mlock(k->dsa_pkey, k->pkeysz) < 0)
          tls_log("error locking passphrase into memory: %s", strerror(errno));
      PRIVS_RELINQUISH
#endif /* HAVE_MLOCK */

      pkey = k;
      continue;
    }

    /* Otherwise, scrub the passphrase's memory areas. */
    if (k->rsa_pkey) {
      pr_memscrub(k->rsa_pkey, k->pkeysz);
      free(k->rsa_pkey_ptr);
      k->rsa_pkey = k->rsa_pkey_ptr = NULL;
    }

    if (k->dsa_pkey) {
      pr_memscrub(k->dsa_pkey, k->pkeysz);
      free(k->dsa_pkey_ptr);
      k->dsa_pkey = k->dsa_pkey_ptr = NULL;
    }
  }

  return pkey;
}

static int tls_pkey_cb(char *buf, int buflen, int rwflag, void *data) {
  tls_pkey_t *k;

  if (!data)
    return 0;

  k = (tls_pkey_t *) data;

  if ((k->flags & TLS_PKEY_USE_RSA) && k->rsa_pkey) {
    strncpy(buf, k->rsa_pkey, buflen);
    buf[buflen - 1] = '\0';
    return strlen(buf);
  }

  if ((k->flags & TLS_PKEY_USE_DSA) && k->dsa_pkey) {
    strncpy(buf, k->dsa_pkey, buflen);
    buf[buflen - 1] = '\0';
    return strlen(buf);
  }

  return 0;
}

static void tls_scrub_pkeys(void) {
  tls_pkey_t *k;

  /* Scrub and free all passphrases in memory. */
  if (tls_pkey_list) {
    pr_log_debug(DEBUG5, MOD_TLS_VERSION
      ": scrubbing %u %s from memory",
      tls_npkeys, tls_npkeys != 1 ? "passphrases" : "passphrase");

  } else
    return;

  for (k = tls_pkey_list; k; k = k->next) {
    if (k->rsa_pkey) {
      pr_memscrub(k->rsa_pkey, k->pkeysz);
      free(k->rsa_pkey_ptr);
      k->rsa_pkey = k->rsa_pkey_ptr = NULL;
    }

    if (k->dsa_pkey) {
      pr_memscrub(k->dsa_pkey, k->pkeysz);
      free(k->dsa_pkey_ptr);
      k->dsa_pkey = k->dsa_pkey_ptr = NULL;
    }
  }

  tls_pkey_list = NULL;
  tls_npkeys = 0;
}

#if OPENSSL_VERSION_NUMBER > 0x000907000L
static int tls_renegotiate_timeout_cb(CALLBACK_FRAME) {
  if ((tls_flags & TLS_SESS_ON_CTRL) &&
      (tls_flags & TLS_SESS_CTRL_RENEGOTIATING)) {

    if (!SSL_renegotiate_pending(ctrl_ssl)) {
      tls_log("%s", "control channel TLS session renegotiated");
      tls_flags &= ~TLS_SESS_CTRL_RENEGOTIATING;

    } else if (tls_renegotiate_required) {
      tls_log("%s", "requested TLS renegotiation timed out on control channel");
      tls_log("%s", "shutting down control channel TLS session");
      tls_end_sess(ctrl_ssl, PR_NETIO_STRM_CTRL, TRUE);
      tls_ctrl_rd_nstrm->strm_data = tls_ctrl_wr_nstrm->strm_data =
        ctrl_ssl = NULL;
    }
  }

  if ((tls_flags & TLS_SESS_ON_DATA) &&
      (tls_flags & TLS_SESS_DATA_RENEGOTIATING)) {

    if (!SSL_renegotiate_pending((SSL *) tls_data_wr_nstrm->strm_data)) {
      tls_log("%s", "data channel TLS session renegotiated");
      tls_flags &= ~TLS_SESS_DATA_RENEGOTIATING;

    } else if (tls_renegotiate_required) {
      tls_log("%s", "requested TLS renegotiation timed out on data channel");
      tls_log("%s", "shutting down data channel TLS session");
      tls_end_sess((SSL *) tls_data_wr_nstrm->strm_data, PR_NETIO_STRM_DATA,
        TRUE);
      tls_data_rd_nstrm->strm_data = tls_data_wr_nstrm->strm_data = NULL;
    }
  }

  return 0;
}

static int tls_ctrl_renegotiate_cb(CALLBACK_FRAME) {
  if (tls_flags & TLS_SESS_ON_CTRL) {
    tls_log("%s", "requesting TLS renegotiation on control channel");
    SSL_renegotiate(ctrl_ssl);
    /* SSL_do_handshake(ctrl_ssl); */

    pr_timer_add(tls_renegotiate_timeout, 0, &tls_module,
      tls_renegotiate_timeout_cb);

    tls_flags |= TLS_SESS_CTRL_RENEGOTIATING;

    /* Restart the timer. */
    return 1;
  }

  return 0;
}
#endif

static DH *tls_dh_cb(SSL *ssl, int is_export, int keylength) {
  FILE *fp = NULL;

  if (tls_tmp_dh)
    return tls_tmp_dh;

  if (tls_dhparam_file) {
    fp = fopen(tls_dhparam_file, "r");
    if (fp) {
      tls_tmp_dh = PEM_read_DHparams(fp, NULL, NULL, NULL);
      fclose(fp);

      if (tls_tmp_dh)
        return tls_tmp_dh;

    } else
      pr_log_debug(DEBUG3, MOD_TLS_VERSION
        ": unable to open TLSDHParamFile '%s': %s", tls_dhparam_file,
          strerror(errno));
  }

  switch (keylength) {
    case 512:
      return (tls_tmp_dh = get_dh512());

    case 768:
      return (tls_tmp_dh = get_dh768());

     case 1024:
       return (tls_tmp_dh = get_dh1024());

     case 1536:
       return (tls_tmp_dh = get_dh1536());

     case 2048:
       return (tls_tmp_dh = get_dh2048());

     default:
       return (tls_tmp_dh = get_dh1024());
  }

  return NULL;
}

/* Post 0.9.7a, RSA blinding is turned on by default, so there is no need to
 * do this manually.
 */
#if OPENSSL_VERSION_NUMBER < 0x0090702fL
static void tls_blinding_on(SSL *ssl) {
  EVP_PKEY *pkey = NULL;
  RSA *rsa = NULL;

  /* RSA keys are subject to timing attacks.  To attempt to make such
   * attacks harder, use RSA blinding.
   */

  pkey = SSL_get_privatekey(ssl);

  if (pkey)
    rsa = EVP_PKEY_get1_RSA(pkey);

  if (rsa) {
    if (RSA_blinding_on(rsa, NULL) != 1)
      tls_log("error setting RSA blinding: %s",
        ERR_error_string(ERR_get_error(), NULL));
    else
      tls_log("set RSA blinding on");

    /* Now, "free" the RSA pointer, to properly decrement the reference
     * counter.
     */
    RSA_free(rsa);

  } else {

    /* The administrator may have configured DSA keys rather than RSA keys.
     * In this case, there is nothing to do.
     */
  }

  return;
}
#endif

static int tls_init_ctxt(void) {
  SSL_load_error_strings();
  SSL_library_init();

#ifdef ZLIB
   {
     COMP_METHOD *cm = COMP_zlib();
     if (cm != NULL && cm->type != NID_undef) {
        SSL_COMP_add_compression_method(0xe0, cm); /* Eric Young's ZLIB ID */
     }
   }
#endif /* ZLIB */

  ssl_ctx = SSL_CTX_new(SSLv23_server_method());
  if (ssl_ctx == NULL) {
    tls_log("error: SSL_CTX_new(): %s", tls_get_errors());
    return -1;
  }

#if OPENSSL_VERSION_NUMBER > 0x000906000L
  /* The SSL_MODE_AUTO_RETRY mode was added in 0.9.6. */
  SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);
#endif

  /* Make sure that SSLv2 communications are disabled entirely.  If using
   * OpenSSL-0.9.7 or greater, prevent session resumptions on renegotiations
   * as well (more secure).
   */
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2|
    SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION|SSL_OP_SINGLE_DH_USE);
#else
  SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2|SSL_OP_SINGLE_DH_USE);
#endif

  /* Set up session caching. */
  SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_SERVER);
  SSL_CTX_set_session_id_context(ssl_ctx, (const unsigned char *) "1", 1);

  SSL_CTX_set_tmp_dh_callback(ssl_ctx, tls_dh_cb);

  if (tls_seed_prng())
    tls_log("%s", "unable to properly seed PRNG");

  /* Add the commands handled by this module to the HELP list. */
  pr_help_add(C_AUTH, "<sp> base64-data", TRUE);
  pr_help_add(C_PBSZ, "<sp> protection buffer size", TRUE);
  pr_help_add(C_PROT, "<sp> protection code", TRUE);

  return 0;
}

static int tls_init_server(void) {
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  config_rec *c = NULL;
#endif
  char *tls_ca_cert = NULL, *tls_ca_path = NULL;

  if (strcasecmp(tls_protocol, "SSLv23") == 0)
    /* This is the default, so there is no need to do anything. */
    ;

  else if (strcasecmp(tls_protocol, "SSLv3") == 0)
    SSL_CTX_set_ssl_version(ssl_ctx, SSLv3_server_method());

  else if (strcasecmp(tls_protocol, "TLSv1") == 0)
    SSL_CTX_set_ssl_version(ssl_ctx, TLSv1_server_method());

  tls_ca_cert = get_param_ptr(main_server->conf, "TLSCACertificateFile", FALSE);
  tls_ca_path = get_param_ptr(main_server->conf, "TLSCACertificatePath", FALSE);

  if (tls_ca_cert || tls_ca_path) {

    /* Set the locations used for verifying certificates. */
    PRIVS_ROOT
    if (SSL_CTX_load_verify_locations(ssl_ctx, tls_ca_cert, tls_ca_path) != 1) {
      PRIVS_RELINQUISH
      tls_log("unable to set CA verification using file '%s' or "
        "directory '%s': %s", tls_ca_cert ? tls_ca_cert : "(none)",
        tls_ca_path ? tls_ca_path : "(none)",
        ERR_error_string(ERR_get_error(), NULL));
      return -1;
    }
    PRIVS_RELINQUISH

  } else {

    /* Default to using locations set in the OpenSSL config file.
     */

    tls_log("%s", "using default OpenSSL verification locations "
      "(see $SSL_CERT_DIR environment variable)");

    if (SSL_CTX_set_default_verify_paths(ssl_ctx) != 1)
      tls_log("error setting default verification locations: %s",
          ERR_error_string(ERR_get_error(), NULL));
  }

  if (!(tls_opts & TLS_OPT_NO_CERT_REQUEST)) {
    int verify_mode = SSL_VERIFY_PEER;
    char *tls_ca_chain = NULL;

    /* If we are verifying client, make sure the client sends a cert;
     * the protocol allows for the client to disregard a request for
     * its cert by the server.
     */
    if (tls_flags & TLS_SESS_VERIFY_CLIENT)
      verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

    SSL_CTX_set_verify(ssl_ctx, verify_mode, tls_verify_cb);

    /* Note: we add one to the configured depth purposefully.  As noted
     * in the OpenSSL man pages, the verification process will silently
     * stop at the configured depth, and the error messages ensuing will
     * be that of an incomplete certificate chain, rather than the
     * "chain too long" error that might be expected.  To log the "chain
     * too long" condition, we add one to the configured depth, and catch,
     * in the verify callback, the exceeding of the actual depth.
     */
    SSL_CTX_set_verify_depth(ssl_ctx, tls_verify_depth + 1);

    /* Do not forget to configure the certs that the server will send to
     * the client when requesting a client cert.  Use the configured
     * TLSCertificateChainFile, if present; otherwise, construct the list
     * from all the certs in the TLSCACertificatePath.
     */
 
    tls_ca_chain = get_param_ptr(main_server->conf, "TLSCertificateChainFile",
      FALSE);
    if (tls_ca_chain) {
      if (SSL_CTX_use_certificate_chain_file(ssl_ctx, tls_ca_chain) != 1) {
        tls_log("unable to use certificate chain '%s': %s", tls_ca_chain,
          tls_get_errors());
      }
    } 

    if (tls_ca_cert) {
      FILE *cacertf = NULL;

      PRIVS_ROOT
      cacertf = fopen(tls_ca_cert, "r");
      PRIVS_RELINQUISH

      if (cacertf) {
        X509 *x509 = PEM_read_X509(cacertf, NULL, NULL, NULL);

        if (x509) {
          if (SSL_CTX_add_client_CA(ssl_ctx, x509) != 1) {
            tls_log("error adding '%s' to client CA list: %s", tls_ca_cert,
              tls_get_errors());
          }

        } else {
          tls_log("unable to read certificate in '%s': %s", tls_ca_cert,
            tls_get_errors());
        }

        fclose(cacertf);

      } else {
        tls_log("unable to open '%s': %s", tls_ca_cert, strerror(errno));
      }
    }

    if (!tls_ca_chain &&
        tls_ca_path) {
      DIR *cacertdir = NULL;

      PRIVS_ROOT
      cacertdir = opendir(tls_ca_path);
      PRIVS_RELINQUISH

      if (cacertdir) {
        struct dirent *cadent = NULL;
        pool *tmp_pool = make_sub_pool(permanent_pool);

        while ((cadent = readdir(cacertdir)) != NULL) {
          FILE *cacertf = NULL;
          char *cacertname = pdircat(tmp_pool, tls_ca_path, cadent->d_name,
             NULL);

          pr_signals_handle();

          /* Skip dot directories. */
          if (is_dotdir(cacertname)) {
            continue;
          }

          PRIVS_ROOT
          cacertf = fopen(cacertname, "r");
          PRIVS_RELINQUISH

          if (cacertf) {
            X509 *x509 = PEM_read_X509(cacertf, NULL, NULL, NULL);

            if (x509) {
              if (SSL_CTX_add_client_CA(ssl_ctx, x509) != 1) {
                tls_log("error adding '%s' to client CA list: %s", cacertname,
                  tls_get_errors());
              }

            } else {
              tls_log("unable to read '%s': %s", cacertname, tls_get_errors());
            }

            fclose(cacertf);

          } else {
            tls_log("unable to open '%s': %s", cacertname, strerror(errno));
          }
        }
        destroy_pool(tmp_pool);
        closedir(cacertdir);
 
      } else {
        tls_log("unable to add CAs in '%s': %s", tls_ca_path,
          strerror(errno));
      }
    }
  }

  /* Assume that, if no separate key files are configured, the keys are
   * in the same file as the corresponding certificate.
   */
  if (!tls_rsa_key_file)
     tls_rsa_key_file = tls_rsa_cert_file;

  if (!tls_dsa_key_file)
     tls_dsa_key_file = tls_dsa_cert_file;

  PRIVS_ROOT
  if (tls_rsa_cert_file) {
    int res = SSL_CTX_use_certificate_file(ssl_ctx, tls_rsa_cert_file,
      X509_FILETYPE_PEM);

    if (res <= 0) {
      PRIVS_RELINQUISH

      tls_log("error loading TLSRSACertificateFile '%s': %s", tls_rsa_cert_file,
        tls_get_errors());
      return -1;
    }

    SSL_CTX_set_tmp_rsa_callback(ssl_ctx, tls_rsa_cb);
  }

  if (tls_rsa_key_file) {
    int res;

    if (tls_pkey) {
      tls_pkey->flags |= TLS_PKEY_USE_RSA;
      tls_pkey->flags &= ~TLS_PKEY_USE_DSA;
    }

    res = SSL_CTX_use_PrivateKey_file(ssl_ctx, tls_rsa_key_file,
      X509_FILETYPE_PEM);

    if (res <= 0) {
      PRIVS_RELINQUISH

      tls_log("error loading TLSRSACertificateKeyFile '%s': %s",
        tls_rsa_key_file, tls_get_errors());
      return -1;
    }
  }

  if (tls_dsa_cert_file) {
    int res = SSL_CTX_use_certificate_file(ssl_ctx, tls_dsa_cert_file,
      X509_FILETYPE_PEM);

    if (res <= 0) {
      PRIVS_RELINQUISH

      tls_log("error loading TLSDSACertificateFile '%s' %s", tls_dsa_cert_file,
        tls_get_errors());
      return -1;
    }
  }

  if (tls_dsa_key_file) {
    int res;

    if (tls_pkey) {
      tls_pkey->flags &= ~TLS_PKEY_USE_RSA;
      tls_pkey->flags |= TLS_PKEY_USE_DSA;
    }

    res = SSL_CTX_use_PrivateKey_file(ssl_ctx, tls_dsa_key_file,
      X509_FILETYPE_PEM);

    if (res <= 0) {
      PRIVS_RELINQUISH

      tls_log("error loading TLSDSACertificateKeyFile '%s': %s",
        tls_dsa_key_file, tls_get_errors());
      return -1;
    }
  }
  PRIVS_RELINQUISH

  /* Set up the CRL. */
  if (tls_crl_file || tls_crl_path) {
    tls_crl_store = X509_STORE_new();
    if (tls_crl_store == NULL) {
      tls_log("error creating CRL store: %s", tls_get_errors());
      return -1;
    }

    if (X509_STORE_load_locations(tls_crl_store, tls_crl_file,
        tls_crl_path) == 0) {

      if (tls_crl_file && !tls_crl_path) {
        tls_log("error loading TLSCARevocationFile '%s': %s",
          tls_crl_file, tls_get_errors());

      } else if (!tls_crl_file && tls_crl_path) {
        tls_log("error loading TLSCARevocationPath '%s': %s",
          tls_crl_path, tls_get_errors());

      } else {
        tls_log("error loading TLSCARevocationFile '%s', "
          "TLSCARevocationPath '%s': %s", tls_crl_file, tls_crl_path,
          tls_get_errors());
      }
    }
  }

  SSL_CTX_set_cipher_list(ssl_ctx, tls_cipher_suite);

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  /* Lookup/process any configured TLSRenegotiate parameters. */
  c = find_config(main_server->conf, CONF_PARAM, "TLSRenegotiate", FALSE);
  if (c) {
    if (c->argc == 0) {
      /* Disable all server-side requested renegotiations; clients can
       * still request renegotiations.
       */
      tls_ctrl_renegotiate_timeout = 0;
      tls_data_renegotiate_limit = 0;
      tls_renegotiate_timeout = 0;
      tls_renegotiate_required = FALSE;

    } else {
      int ctrl_timeout = *((int *) c->argv[0]);
      off_t data_limit = *((off_t *) c->argv[1]);
      int renegotiate_timeout = *((int *) c->argv[2]);
      unsigned char renegotiate_required = *((unsigned char *) c->argv[3]);

      if (data_limit)
        tls_data_renegotiate_limit = data_limit;
    
      if (renegotiate_timeout)
        tls_renegotiate_timeout = renegotiate_timeout;

      tls_renegotiate_required = renegotiate_required;
  
      /* Set any control channel renegotiation timers, if need be. */
      pr_timer_add(ctrl_timeout ? ctrl_timeout : tls_ctrl_renegotiate_timeout,
        0, &tls_module, tls_ctrl_renegotiate_cb);
    }
  }

  /* Lookup and handle any requested crypto accelerators/drivers. */
  c = find_config(main_server->conf, CONF_PARAM, "TLSCryptoDevice", FALSE);
  if (c) {
    if (strcasecmp((const char *) c->argv[0], "NONE") == 0) {
      tls_crypto_device = NULL;

    } else if (strcasecmp((const char *) c->argv[0], "ALL") == 0) {
      /* Load all ENGINE implementations bundled with OpenSSL. */
      ENGINE_load_builtin_engines();
      ENGINE_register_all_complete();

      tls_crypto_device = c->argv[0];
      tls_log("enabled all builtin crypto devices");

    } else {
      ENGINE *e;

      /* Load all ENGINE implementations bundled with OpenSSL. */
      ENGINE_load_builtin_engines();

      e = ENGINE_by_id(c->argv[0]);
      if (!e) {
        /* The requested driver is not available. */
        tls_log("TLSCryptoDevice '%s' is not available",
          (const char *) c->argv[0]);
        return 0;
      }

      if (!ENGINE_init(e)) {
        /* The requested driver could not be initialized. */
        tls_log("unable to initialize TLSCryptoDevice '%s': %s",
          (const char *) c->argv[0], tls_get_errors());

        ENGINE_free(e);
        return 0;
      }

      if (!ENGINE_set_default(e, ENGINE_METHOD_ALL)) {
        /* The requested driver could not be used as the default for
         * some odd reason.
         */
        tls_log("unable to register TLSCryptoDevice '%s' as the default: %s",
          (const char *) c->argv[0], tls_get_errors());

        ENGINE_finish(e);
        ENGINE_free(e);
        return 0;
      }

      ENGINE_finish(e);
      ENGINE_free(e);

      tls_crypto_device = c->argv[0];
      tls_log("using TLSCryptoDevice '%s'", tls_crypto_device);
    }
  }
#endif

  return 0;
}

static int tls_accept(conn_t *conn, unsigned char on_data) {
  int res = 0;
  char *subj = NULL;
  static unsigned char logged_data = FALSE;
  SSL *ssl = NULL;

  if (!ssl_ctx) {
    tls_log("%s", "unable to start session: null SSL_CTX");
    return -1;
  }

  ssl = SSL_new(ssl_ctx);
  if (ssl == NULL) {
    tls_log("error: unable to start session: %s",
      ERR_error_string(ERR_get_error(), NULL));
    return -2;
  }

  /* This works with either rfd or wfd (I hope) */
  SSL_set_fd(ssl, conn->rfd);

  /* If configured, set a timer for the handshake. */
  if (tls_handshake_timeout)
    tls_handshake_timer_id = pr_timer_add(tls_handshake_timeout, -1,
      &tls_module, tls_handshake_timeout_cb);

  retry:
  pr_signals_handle();
  res = SSL_accept(ssl);
  if (res < 1) {
    const char *msg = "unable to accept TLS connection";
    int errcode = SSL_get_error(ssl, res);

    pr_signals_handle();

    if (tls_handshake_timed_out) {
      tls_log("TLS negotiation timed out (%u seconds)", tls_handshake_timeout);
      tls_end_sess(ssl, on_data ? PR_NETIO_STRM_DATA : PR_NETIO_STRM_CTRL,
        TRUE);
      return -4;
    }

    switch (errcode) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        goto retry;

      case SSL_ERROR_ZERO_RETURN:
        tls_log("%s: TLS connection closed", msg);
        break;

      case SSL_ERROR_WANT_X509_LOOKUP:
        tls_log("%s: needs X509 lookup", msg);
        break;

      case SSL_ERROR_SYSCALL: {
        /* Check to see if the OpenSSL error queue has info about this. */
        int xerrcode = ERR_get_error();
    
        if (xerrcode == 0) {
          /* The OpenSSL error queue doesn't have any more info, so we'll
           * examine the SSL_accept() return value itself.
           */

          if (res == 0) {
            /* EOF */
            tls_log("%s: received EOF that violates protocol", msg);

          } else if (res == -1) {
            /* Check errno */
            tls_log("%s: %s", msg, strerror(errno));
          }

        } else
          tls_log("%s: %s", msg, tls_get_errors());

        break;
      }

      case SSL_ERROR_SSL:
        tls_log("%s: %s", msg, tls_get_errors());
        break;
    }

    tls_end_sess(ssl, on_data ? PR_NETIO_STRM_DATA : PR_NETIO_STRM_CTRL,
      TRUE);
    return -3;
  }

  /* Disable the handshake timer. */
  pr_timer_remove(tls_handshake_timer_id, &tls_module);

  /* Stash the SSL object in the pointers of the correct NetIO streams. */
  if (conn == session.c) {
    ctrl_ssl = ssl;
    tls_ctrl_rd_nstrm->strm_data = tls_ctrl_wr_nstrm->strm_data = (void *) ssl;

  } else if (conn == session.d)
    tls_data_rd_nstrm->strm_data = tls_data_wr_nstrm->strm_data = (void *) ssl;

  /* TLS handshake on the control channel... */
  if (!on_data) {
    tls_log("%s connection accepted, using cipher %s (%d bits)",
      SSL_get_cipher_version(ssl), SSL_get_cipher_name(ssl),
      SSL_get_cipher_bits(ssl, NULL));

    subj = tls_get_subj_name();
    if (subj)
      tls_log("Client: %s", subj);

    if (!(tls_opts & TLS_OPT_NO_CERT_REQUEST)) {

      /* NOTE: should probably use SSL_get_verify_result() as a last
       * sanity check.
       */

      /* Now we can go on with our post-handshake, application level
       * requirement checks.
       */
      if (!tls_check_client_cert(ssl, conn))
        return -1;
    }

    /* Setup the TLS environment variables, if requested. */
    tls_setup_environ(ssl);

  /* TLS handshake on the data channel... */
  } else {

    /* Only be verbose with the first TLS data connection, otherwise there
     * might be too much noise.
     */
    if (!logged_data) {
      tls_log("%s data connection accepted, using cipher %s (%d bits)",
        SSL_get_cipher_version(ssl), SSL_get_cipher_name(ssl),
        SSL_get_cipher_bits(ssl, NULL));
      logged_data = TRUE;
    }
  }

  return 0;
}

static void tls_cleanup(int flags) {

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  if (tls_crypto_device) {
    ENGINE_cleanup();
    tls_crypto_device = NULL;
  }
#endif

  if (tls_crl_store) {
    X509_STORE_free(tls_crl_store);
    tls_crl_store = NULL;
  }

  if (ssl_ctx) {
    SSL_CTX_free(ssl_ctx);
    ssl_ctx = NULL;
  }

  if (tls_tmp_dh) {
    DH_free(tls_tmp_dh);
    tls_tmp_dh = NULL;
  }

  if (tls_tmp_rsa) {
    RSA_free(tls_tmp_rsa);
    tls_tmp_rsa = NULL;
  }

  if (!(flags & TLS_CLEANUP_FL_SESS_INIT)) {
    ERR_free_strings();
    ERR_remove_state(0);
    EVP_cleanup();

  } else {
    /* Only call EVP_cleanup() et al if other OpenSSL-using modules are not
     * present.  If we called EVP_cleanup() here during session
     * initialization, and other modules want to use OpenSSL, we may
     * be depriving those modules of OpenSSL functionality.
     *
     * At the moment, the modules known to use OpenSSL are mod_ldap
     * and mod_sql.
     */
    if (pr_module_get("mod_ldap.c") == NULL &&
        pr_module_get("mod_sql.c") == NULL) {
      ERR_free_strings();
      ERR_remove_state(0);
      EVP_cleanup();
    }
  }
}

static void tls_end_sess(SSL *ssl, int strms, int use_shutdown) {
  int res;
  int shutdown_state;

  if (!ssl)
    return;

  res = SSL_shutdown(ssl);
  if (res == 0) {
    if (use_shutdown) {
      /* Try calling SSL_shutdown() again.  First, though, send a TCP FIN
       * to trigger the remote end's close_notify SSL message, via shutdown().
       */
      if (strms & PR_NETIO_STRM_CTRL) {
        pr_netio_shutdown(session.c->outstrm, 1);

        if (session.c->instrm != session.c->outstrm)
          pr_netio_shutdown(session.c->instrm, 1);
      }

      if (strms & PR_NETIO_STRM_DATA) {
        pr_netio_shutdown(session.d->outstrm, 1);

        if (session.d->instrm != session.d->outstrm)
          pr_netio_shutdown(session.d->instrm, 1);
      }
    }

    shutdown_state = SSL_get_shutdown(ssl);

    /* Now call SSL_shutdown() again, but only if necessary. */
    res = 1;
    if (!(shutdown_state & SSL_RECEIVED_SHUTDOWN)) {
      res = SSL_shutdown(ssl);
    }

    if (res == 0) {
      int err = SSL_get_error(ssl, res);

      switch (err) {
        case SSL_ERROR_WANT_READ:
          tls_log("SSL_shutdown error: WANT_READ");
          pr_log_debug(DEBUG0, MOD_TLS_VERSION
            ": SSL_shutdown error: WANT_READ");
          break;

        case SSL_ERROR_WANT_WRITE:
          tls_log("SSL_shutdown error: WANT_WRITE");
          pr_log_debug(DEBUG0, MOD_TLS_VERSION
            ": SSL_shutdown error: WANT_WRITE");
          break;

        case SSL_ERROR_ZERO_RETURN:
          tls_log("SSL_shutdown error: ZERO_RETURN");
          pr_log_debug(DEBUG0, MOD_TLS_VERSION
            ": SSL_shutdown error: ZERO_RETURN");
          break;

        case SSL_ERROR_SYSCALL:
          if (errno != 0 &&
              errno != EOF &&
              errno != EBADF &&
              errno != EPIPE) {
            tls_log("SSL_shutdown syscall error: %s", strerror(errno));
            pr_log_debug(DEBUG0, MOD_TLS_VERSION
              ": SSL_shutdown syscall error: %s", strerror(errno));
          }
          break;

        default:
          tls_log("SSL_shutdown error [%d]: %s", err, tls_get_errors());
          pr_log_debug(DEBUG0, MOD_TLS_VERSION
            ": SSL_shutdown error [%d]: %s", err, tls_get_errors());
          break;
      }
    }

  } else if (res < 0) {
    int err = SSL_get_error(ssl, res);

    switch (err) {
      case SSL_ERROR_ZERO_RETURN:
        /* Clean shutdown, nothing we need to do. */
        break;

      default:
        tls_fatal_error(err, __LINE__);
        break;
    }
  }

  SSL_free(ssl);
}

static const char *tls_get_errors(void) {
  unsigned int count = 0;
  unsigned long e = ERR_get_error();
  BIO *bio = NULL;
  char *data = NULL;
  long datalen;
  const char *str = "(unknown)";

  /* Use ERR_print_errors() and a memory BIO to build up a string with
   * all of the error messages from the error queue.
   */

  if (e)
    bio = BIO_new(BIO_s_mem());

  while (e) {
    pr_signals_handle();
    BIO_printf(bio, "\n  (%u) %s", ++count, ERR_error_string(e, NULL));
    e = ERR_get_error(); 
  }

  datalen = BIO_get_mem_data(bio, &data);
  if (data) {
    data[datalen] = '\0';
    str = pstrdup(main_server->pool, data);
  }

  if (bio)
    BIO_free(bio);

  return str;
}

/* Return a page-aligned pointer to memory of at least the given size.
 */
static char *tls_get_page(size_t sz, void **ptr) {
  void *d;
  long pagesz = tls_get_pagesz(), p;

  d = malloc(sz + (pagesz-1));
  if (d == NULL) {
    pr_log_pri(PR_LOG_ERR, "out of memory!");
    exit(1);
  }

  *ptr = d;

  p = ((long) d + (pagesz-1)) &~ (pagesz-1);

  return ((char *) p);
}

/* Return the size of a page on this architecture.
 */
static size_t tls_get_pagesz(void) {
  long pagesz;

#if defined(_SC_PAGESIZE)
  pagesz = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
  pagesz = sysconf(_SC_PAGE_SIZE);
#else
  /* Default to using OpenSSL's defined buffer size for PEM files. */
  pagesz = PEM_BUFSIZE;
#endif /* !_SC_PAGESIZE and !_SC_PAGE_SIZE */

  return pagesz;
}

static char *tls_get_subj_name(void) {
  X509 *cert = SSL_get_peer_certificate(ctrl_ssl);

  if (cert) {
    char *name = tls_x509_name_oneline(X509_get_subject_name(cert));
    X509_free(cert);
    return name;
  }

  return NULL;
}

static void tls_fatal_error(int error, int lineno) {

  switch (error) {
    case SSL_ERROR_NONE:
      return;

    case SSL_ERROR_SSL:
      tls_log("panic: SSL_ERROR_SSL, line %d: %s", lineno, tls_get_errors());
      break;

    case SSL_ERROR_WANT_READ:
      tls_log("panic: SSL_ERROR_WANT_READ, line %d", lineno);
      break;

    case SSL_ERROR_WANT_WRITE:
      tls_log("panic: SSL_ERROR_WANT_WRITE, line %d", lineno);
      break;

    case SSL_ERROR_WANT_X509_LOOKUP:
      tls_log("panic: SSL_ERROR_WANT_X509_LOOKUP, line %d", lineno);
      break;

    case SSL_ERROR_SYSCALL: {
      int xerrcode = ERR_get_error();

      if (errno == ECONNRESET)
        return;

      /* Check to see if the OpenSSL error queue has info about this. */
      if (xerrcode == 0) {
        /* The OpenSSL error queue doesn't have any more info, so we'll
         * examine the error value itself.
         */

        if (errno == EOF)
          tls_log("panic: SSL_ERROR_SYSCALL, line %d: "
            "EOF that violates protocol", lineno);

        else
          /* Check errno */
          tls_log("panic: SSL_ERROR_SYSCALL, line %d: %s", lineno,
            strerror(errno));

      } else
        tls_log("panic: SSL_ERROR_SYSCALL, line %d: %s", lineno,
          tls_get_errors());

      break;
    }

    case SSL_ERROR_ZERO_RETURN:
      tls_log("panic: SSL_ERROR_ZERO_RETURN, line %d", lineno);
      break;

    case SSL_ERROR_WANT_CONNECT:
      tls_log("panic: SSL_ERROR_WANT_CONNECT, line %d", lineno);
      break;

    default:
      tls_log("panic: SSL_ERROR %d, line %d", error, lineno);
      break;
  }

  tls_log("%s", "unexpected OpenSSL error, disconnecting");
  pr_log_pri(PR_LOG_ERR, "%s", MOD_TLS_VERSION
    ": unexpected OpenSSL error, disconnecting");

  end_login(1);
}

/* This function checks if the client's cert is in the ~/.tlslogin file
 * of the "user".
 */
static unsigned char tls_dotlogin_allow(const char *user) {
  char buf[512] = {'\0'}, *home = NULL;
  FILE *fp = NULL;
  X509 *client_cert = NULL, *file_cert = NULL;
  struct passwd *pwd = NULL;
  pool *tmp_pool = NULL;
  unsigned char allow_user = FALSE;

  if (!(tls_flags & TLS_SESS_ON_CTRL) ||
      !ctrl_ssl ||
      !user)
    return FALSE;

  tmp_pool = make_sub_pool(permanent_pool);

  PRIVS_ROOT
  pwd = pr_auth_getpwnam(tmp_pool, user);
  PRIVS_RELINQUISH

  if (!pwd) {
    destroy_pool(tmp_pool);
    return FALSE;
  }

  /* Handle the case where the user's home directory is a symlink. */
  PRIVS_USER
  home = dir_realpath(tmp_pool, pwd->pw_dir);
  PRIVS_RELINQUISH

  snprintf(buf, sizeof(buf), "%s/.tlslogin", home ? home : pwd->pw_dir);
  buf[sizeof(buf)-1] = '\0';

  /* No need for the temporary pool any more. */
  destroy_pool(tmp_pool);
  tmp_pool = NULL;

  PRIVS_ROOT
  fp = fopen(buf, "r");
  PRIVS_RELINQUISH

  if (!fp) {
    tls_log(".tlslogin check: unable to open '%s': %s", buf, strerror(errno));
    return FALSE;
  }

  client_cert = SSL_get_peer_certificate(ctrl_ssl);
  if (!client_cert) {
    fclose(fp);
    return FALSE;
  }

  while ((file_cert = PEM_read_X509(fp, NULL, NULL, NULL))) {
    if (!M_ASN1_BIT_STRING_cmp(client_cert->signature, file_cert->signature))
      allow_user = TRUE;

    X509_free(file_cert);
    if (allow_user)
      break;
  }

  X509_free(client_cert);
  fclose(fp);

  return allow_user;
}

/* This is unused...for now. */
#if 0
static char *tls_cert_to_user(pool *cert_pool, X509 *cert) {
  if (!cert_pool || !cert)
    return FALSE;

  /* NOTE: insert cert->user translation code here.  Possibly add
   * TLSOptions that affect this mapping process.
   */

  return NULL;
}
#endif

static int tls_readmore(int rfd) {
  fd_set rfds;
  struct timeval tv;

  FD_ZERO(&rfds);
  FD_SET(rfd, &rfds);

  /* Use a timeout of 15 seconds */
  tv.tv_sec = 15;
  tv.tv_usec = 0;

  return select(rfd + 1, &rfds, NULL, NULL, &tv);
}

static int tls_writemore(int wfd) {
  fd_set wfds;
  struct timeval tv;

  FD_ZERO(&wfds);
  FD_SET(wfd, &wfds);

  /* Use a timeout of 15 seconds */
  tv.tv_sec = 15;
  tv.tv_usec = 0;

  return select(wfd + 1, NULL, &wfds, NULL, &tv);
}

static ssize_t tls_read(SSL *ssl, void *buf, size_t len) {
  ssize_t count;

  retry:
  pr_signals_handle();
  count = SSL_read(ssl, buf, len);
  if (count < 0) {
    int err = SSL_get_error(ssl, count);

    /* read(2) returns only the generic error number -1 */
    count = -1;

    switch (err) {
      case SSL_ERROR_WANT_READ:
        /* OpenSSL needs more data from the wire to finish the current block,
         * so we wait a little while for it.
         */
        if ((err = tls_readmore(SSL_get_fd(ssl))) > 0)
          goto retry;

        else if (err == 0)
          /* Still missing data after timeout. Simulate an EINTR and return.
           */
          errno = EINTR;

          /* If err < 0, i.e. some error from the select(), everything is
           * already in place; errno is properly set and this function
           * returns -1.
           */
          break;

      case SSL_ERROR_WANT_WRITE:
        /* OpenSSL needs to write more data to the wire to finish the current
         * block, so we wait a little while for it.
         */
        if ((err = tls_writemore(SSL_get_fd(ssl))) > 0)
          goto retry;

        else if (err == 0)
          /* Still missing data after timeout. Simulate an EINTR and return.
           */
          errno = EINTR;

          /* If err < 0, i.e. some error from the select(), everything is
           * already in place; errno is properly set and this function
           * returns -1.
           */
          break;

      case SSL_ERROR_ZERO_RETURN:
        tls_log("read EOF from client");
        break;

      default:
        tls_fatal_error(err, __LINE__);
        break;
    }
  }

  return count;
}

static RSA *tls_rsa_cb(SSL *ssl, int is_export, int keylength) {
  if (tls_tmp_rsa)
    return tls_tmp_rsa;

  tls_tmp_rsa = RSA_generate_key(keylength, RSA_F4, NULL, NULL);
  return tls_tmp_rsa;
}

static int tls_seed_prng(void) {
  char stackdata[1024];
  static char rand_file[300];
  FILE *fp = NULL;
  
  /* Lookup any configured TLSRandomSeed. */
  tls_rand_file = get_param_ptr(main_server->conf, "TLSRandomSeed", FALSE);

#if OPENSSL_VERSION_NUMBER >= 0x00905100L
  if (RAND_status())

    /* PRNG already well-seeded. */
    return 0;
#endif

  /* If the device '/dev/urandom' is present, OpenSSL uses it by default.
   * Check if it's present, else we have to make random data ourselves.
   */
  fp = fopen("/dev/urandom", "r");
  if (fp) {
    fclose(fp);
    return 0;
  }

  if (!tls_rand_file) {
    /* The ftpd's random file is (openssl-dir)/.rnd */
    snprintf(rand_file, sizeof(rand_file), "%s/.rnd",
      X509_get_default_cert_area());
    rand_file[sizeof(rand_file)-1] = '\0';
    tls_rand_file = rand_file;
  }

  if (!RAND_load_file(tls_rand_file, 1024)) {
    /* No random file found, create new seed. */
    unsigned int c = time(NULL);
    RAND_seed(&c, sizeof(c));
    c = getpid();
    RAND_seed(&c, sizeof(c));
    RAND_seed(stackdata, sizeof(stackdata));
  }

#if OPENSSL_VERSION_NUMBER >= 0x00905100L
  if (!RAND_status()) {
     /* PRNG still badly seeded. */
     return -1;
  }
#endif

  return 0;
}

/* Note: these mappings should probably be added to the mod_tls docs.
 */

static void tls_setup_cert_ext_environ(const char *env_prefix, X509 *cert) {

  /* NOTE: in the future, add ways of adding subjectAltName (and other
   * extensions?) to the environment.
   */

#if 0
  int nexts = 0;

  nexts = X509_get_ext_count(cert);
  if (nexts > 0) {
    register unsigned int i = 0;

    for (i = 0; i < nexts; i++) {
      X509_EXTENSION *ext = X509_get_ext(cert, i);
      const char *extstr = OBJ_nid2sn(OBJ_obj2nid(
        X509_EXTENSION_get_object(ext)));
    }
  }
#endif

  return;
}

/* Note: these mappings should probably be added to the mod_tls docs.
 *
 *   Name                    Short Name    NID
 *   ----                    ----------    ---
 *   countryName             C             NID_countryName
 *   commonName              CN            NID_commonName
 *   description             D             NID_description
 *   givenName               G             NID_givenName
 *   initials                I             NID_initials
 *   localityName            L             NID_localityName
 *   organizationName        O             NID_organizationName
 *   organizationalUnitName  OU            NID_organizationalUnitName
 *   stateOrProvinceName     ST            NID_stateOrProvinceName
 *   surname                 S             NID_surname
 *   title                   T             NID_title
 *   uniqueIdentifer         UID           NID_x500UniqueIdentifier
 *                                         (or NID_uniqueIdentifier, depending
 *                                         on OpenSSL version)
 *   email                   Email         NID_pkcs9_emailAddress
 */

static void tls_setup_cert_dn_environ(const char *env_prefix, X509_NAME *name) {
  register unsigned int i = 0;
  char *k, *v;

  for (i = 0; i < sk_X509_NAME_ENTRY_num(name->entries); i++) {
    X509_NAME_ENTRY *entry = sk_X509_NAME_ENTRY_value(name->entries, i);
    int nid = OBJ_obj2nid(entry->object);

    switch (nid) {
      case NID_countryName:
        k = pstrcat(main_server->pool, env_prefix, "C", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_commonName:
        k = pstrcat(main_server->pool, env_prefix, "CN", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_description:
        k = pstrcat(main_server->pool, env_prefix, "D", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_givenName:
        k = pstrcat(main_server->pool, env_prefix, "G", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_initials:
        k = pstrcat(main_server->pool, env_prefix, "I", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_localityName:
        k = pstrcat(main_server->pool, env_prefix, "L", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_organizationName:
        k = pstrcat(main_server->pool, env_prefix, "O", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_organizationalUnitName:
        k = pstrcat(main_server->pool, env_prefix, "OU", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_stateOrProvinceName:
        k = pstrcat(main_server->pool, env_prefix, "ST", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_surname:
        k = pstrcat(main_server->pool, env_prefix, "S", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_title:
        k = pstrcat(main_server->pool, env_prefix, "T", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

#if OPENSSL_VERSION_NUMBER >= 0x00907000L
      case NID_x500UniqueIdentifier:
#else
      case NID_uniqueIdentifier:
#endif
        k = pstrcat(main_server->pool, env_prefix, "UID", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      case NID_pkcs9_emailAddress:
        k = pstrcat(main_server->pool, env_prefix, "Email", NULL);
        v = pstrndup(main_server->pool, (const char *) entry->value->data,
          entry->value->length);
        pr_env_set(main_server->pool, k, v);
        break;

      default:
        break;
    }
  }
}

static void tls_setup_cert_environ(const char *env_prefix, X509 *cert) {
  char *data = NULL, *k, *v;
  long datalen = 0;
  BIO *bio = NULL;

  if (tls_opts & TLS_OPT_STD_ENV_VARS) {
    char buf[80] = {'\0'};
    ASN1_INTEGER *serial = X509_get_serialNumber(cert);

    sprintf(buf, "%lu", X509_get_version(cert) + 1);
    buf[sizeof(buf)-1] = '\0';

    k = pstrcat(main_server->pool, env_prefix, "M_VERSION", NULL);
    v = pstrdup(main_server->pool, buf);
    pr_env_set(main_server->pool, k, v);

    if (serial->length < 4) {
      memset(buf, '\0', sizeof(buf));
      sprintf(buf, "%lu", ASN1_INTEGER_get(serial));
      buf[sizeof(buf)-1] = '\0';

      k = pstrcat(main_server->pool, env_prefix, "M_SERIAL", NULL);
      v = pstrdup(main_server->pool, buf);
      pr_env_set(main_server->pool, k, v);

    } else {

      /* NOTE: actually, the number is printable, I'm just being lazy. This
       * case is much harder to deal with, and not really worth the effort.
       */
      tls_log("%s", "certificate serial number not printable");
    }

    k = pstrcat(main_server->pool, env_prefix, "S_DN", NULL);
    v = pstrdup(main_server->pool,
      tls_x509_name_oneline(X509_get_subject_name(cert)));
    pr_env_set(main_server->pool, k, v);

    tls_setup_cert_dn_environ(pstrcat(main_server->pool, env_prefix, "S_DN_",
      NULL), X509_get_subject_name(cert));

    k = pstrcat(main_server->pool, env_prefix, "I_DN", NULL);
    v = pstrdup(main_server->pool,
      tls_x509_name_oneline(X509_get_issuer_name(cert)));
    pr_env_set(main_server->pool, k, v);

    tls_setup_cert_dn_environ(pstrcat(main_server->pool, env_prefix, "I_DN_",
      NULL), X509_get_issuer_name(cert));

    tls_setup_cert_ext_environ(pstrcat(main_server->pool, env_prefix, "EXT_",
      NULL), cert);

    bio = BIO_new(BIO_s_mem());
    ASN1_TIME_print(bio, X509_get_notBefore(cert));
    datalen = BIO_get_mem_data(bio, &data);
    data[datalen] = '\0';

    k = pstrcat(main_server->pool, env_prefix, "V_START", NULL);
    v = pstrdup(main_server->pool, data);
    pr_env_set(main_server->pool, k, v);

    BIO_free(bio);

    bio = BIO_new(BIO_s_mem());
    ASN1_TIME_print(bio, X509_get_notAfter(cert));
    datalen = BIO_get_mem_data(bio, &data);
    data[datalen] = '\0';

    k = pstrcat(main_server->pool, env_prefix, "V_END", NULL);
    v = pstrdup(main_server->pool, data);
    pr_env_set(main_server->pool, k, v);

    BIO_free(bio);

    bio = BIO_new(BIO_s_mem());
    i2a_ASN1_OBJECT(bio, cert->cert_info->signature->algorithm);
    datalen = BIO_get_mem_data(bio, &data);
    data[datalen] = '\0';

    k = pstrcat(main_server->pool, env_prefix, "A_SIG", NULL);
    v = pstrdup(main_server->pool, data);
    pr_env_set(main_server->pool, k, v);

    BIO_free(bio);

    bio = BIO_new(BIO_s_mem());
    i2a_ASN1_OBJECT(bio, cert->cert_info->key->algor->algorithm);
    datalen = BIO_get_mem_data(bio, &data);
    data[datalen] = '\0';

    k = pstrcat(main_server->pool, env_prefix, "A_KEY", NULL);
    v = pstrdup(main_server->pool, data);
    pr_env_set(main_server->pool, k, v);

    BIO_free(bio);
  }

  bio = BIO_new(BIO_s_mem());
  PEM_write_bio_X509(bio, cert);
  datalen = BIO_get_mem_data(bio, &data);
  data[datalen] = '\0';

  k = pstrcat(main_server->pool, env_prefix, "CERT", NULL);
  v = pstrdup(main_server->pool, data);
  pr_env_set(main_server->pool, k, v);

  BIO_free(bio);
}

static void tls_setup_environ(SSL *ssl) {
  X509 *cert = NULL;
  STACK_OF(X509) *sk_cert_chain = NULL;
  char *k, *v;

  if (!(tls_opts & TLS_OPT_EXPORT_CERT_DATA) &&
      !(tls_opts & TLS_OPT_STD_ENV_VARS))
    return;

  if (tls_opts & TLS_OPT_STD_ENV_VARS) {
    SSL_CIPHER *cipher = NULL;
    SSL_SESSION *ssl_session = NULL;

    k = pstrdup(main_server->pool, "FTPS");
    v = pstrdup(main_server->pool, "1");
    pr_env_set(main_server->pool, k, v);

    k = pstrdup(main_server->pool, "TLS_PROTOCOL");
    v = pstrdup(main_server->pool, SSL_get_cipher_version(ssl));
    pr_env_set(main_server->pool, k, v);

    /* Process the SSL session-related environ variable. */
    ssl_session = SSL_get_session(ssl);
    if (ssl_session) {
      char buf[SSL_MAX_SSL_SESSION_ID_LENGTH*2+1] = {'\0'};
      register unsigned int i = 0;

      /* Have to obtain a stringified session ID the hard way. */
      for (i = 0; i < ssl_session->session_id_length; i++)
        sprintf(&(buf[i*2]), "%02X", ssl_session->session_id[i]);
      buf[sizeof(buf)-1] = '\0';

      k = pstrdup(main_server->pool, "TLS_SESSION_ID");
      v = pstrdup(main_server->pool, buf);
      pr_env_set(main_server->pool, k, v);
    }

    /* Process the SSL cipher-related environ variables. */
    cipher = SSL_get_current_cipher(ssl);
    if (cipher) {
      char buf[10] = {'\0'};
      int cipher_bits_used = 0, cipher_bits_possible = 0;

      k = pstrdup(main_server->pool, "TLS_CIPHER");
      v = pstrdup(main_server->pool, SSL_CIPHER_get_name(cipher));
      pr_env_set(main_server->pool, k, v);

      cipher_bits_used = SSL_CIPHER_get_bits(cipher, &cipher_bits_possible);

      if (cipher_bits_used < 56) {
        k = pstrdup(main_server->pool, "TLS_CIPHER_EXPORT");
        v = pstrdup(main_server->pool, "1");
        pr_env_set(main_server->pool, k, v);
      }

      memset(buf, '\0', sizeof(buf));
      snprintf(buf, sizeof(buf), "%d", cipher_bits_possible);
      buf[sizeof(buf)-1] = '\0';

      k = pstrdup(main_server->pool, "TLS_CIPHER_KEYSIZE_POSSIBLE");
      v = pstrdup(main_server->pool, buf);
      pr_env_set(main_server->pool, k, v);

      memset(buf, '\0', sizeof(buf));
      snprintf(buf, sizeof(buf), "%d", cipher_bits_used);
      buf[sizeof(buf)-1] = '\0';

      k = pstrdup(main_server->pool, "TLS_CIPHER_KEYSIZE_USED");
      v = pstrdup(main_server->pool, buf);
      pr_env_set(main_server->pool, k, v);
    }

    k = pstrdup(main_server->pool, "TLS_LIBRARY_VERSION");
    v = pstrdup(main_server->pool, OPENSSL_VERSION_TEXT);
    pr_env_set(main_server->pool, k, v);
  }

  sk_cert_chain = SSL_get_peer_cert_chain(ssl);
  if (sk_cert_chain) {
    char *data = NULL;
    long datalen = 0;
    register unsigned int i = 0;
    BIO *bio = NULL;

    /* Adding TLS_CLIENT_CERT_CHAIN environ variables. */
    for (i = 0; i < sk_X509_num(sk_cert_chain); i++) {
      size_t klen = 256;
      k = pcalloc(main_server->pool, klen);
      snprintf(k, klen - 1, "%s%u", "TLS_CLIENT_CERT_CHAIN", i + 1);

      bio = BIO_new(BIO_s_mem());
      PEM_write_bio_X509(bio, sk_X509_value(sk_cert_chain, i));
      datalen = BIO_get_mem_data(bio, &data);
      data[datalen] = '\0';

      v = pstrdup(main_server->pool, data);

      pr_env_set(main_server->pool, k, v);

      BIO_free(bio);
    } 
  }

  /* Note: SSL_get_certificate() does NOT increment a reference counter,
   * so we do not call X509_free() on it.
   */
  cert = SSL_get_certificate(ssl);
  if (cert) {
    tls_setup_cert_environ("TLS_SERVER_", cert);

  } else
    tls_log("unable to set server certificate environ variables");

  cert = SSL_get_peer_certificate(ssl);
  if (cert) {
    tls_setup_cert_environ("TLS_CLIENT_", cert);
    X509_free(cert);

  } else
    tls_log("unable to set client certificate environ variables");

  return;
}

static int tls_verify_cb(int ok, X509_STORE_CTX *ctx) {

  /* TODO: Make up my mind on what to accept or not.*/

  /* We can configure the server to skip the peer's cert verification */
  if (!(tls_flags & TLS_SESS_VERIFY_CLIENT))
     return 1;

  ok = tls_verify_crl(ok, ctx);

  if (!ok) {
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
    int depth = X509_STORE_CTX_get_error_depth(ctx);

    tls_log("error: unable to verify certificate at depth %d", depth);
    tls_log("error: cert subject: %s", tls_x509_name_oneline(
      X509_get_subject_name(cert)));
    tls_log("error: cert issuer: %s", tls_x509_name_oneline(
      X509_get_issuer_name(cert)));

    /* Catch a too long certificate chain here. */
    if (depth > tls_verify_depth)
      X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);

    switch (ctx->error) {
      case X509_V_ERR_CERT_CHAIN_TOO_LONG:
      case X509_V_ERR_CERT_HAS_EXPIRED:
      case X509_V_ERR_CERT_REVOKED:
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
      case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        tls_log("client certificate failed verification: %s",
          X509_verify_cert_error_string(ctx->error));
        ok = 0;
        break;

      case X509_V_ERR_INVALID_PURPOSE: {
        register unsigned int i;
        int count = X509_PURPOSE_get_count();

        tls_log("client certificate failed verification: %s",
          X509_verify_cert_error_string(ctx->error));

        for (i = 0; i < count; i++) {
          X509_PURPOSE *purp = X509_PURPOSE_get0(i);
          tls_log("  purpose #%d: %s", i+1, X509_PURPOSE_get0_name(purp));
        }

        ok = 0;
        break;
      }

      case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        /* XXX this is strange. we get this error for certain clients
         * (i.e. Jeff Altman's kftp) when all is ok. I think it's because the
         * client is actually sending the whole CA cert. This must be figured
         * out, but we let it pass for now. If the CA cert isn't available
         * locally, we will fail anyway.
         */
        tls_log("%s", X509_verify_cert_error_string(ctx->error));
        ok = 1;
        break;

      default:
        tls_log("error verifying client certificate: [%d] %s",
          ctx->error, X509_verify_cert_error_string(ctx->error));
        ok = 0;
        break;
    }
  }

  return ok;
}

/* This routine is (very much!) based on the work by Ralf S. Engelschall
 * <rse@engelshall.com>.  Comments by Ralf.
 */
static int tls_verify_crl(int ok, X509_STORE_CTX *ctx) {
  X509_OBJECT obj;
  X509_NAME *subject = NULL, *issuer = NULL;
  X509 *xs = NULL;
  X509_CRL *crl = NULL;
  X509_REVOKED *revoked = NULL;
  X509_STORE_CTX store_ctx;
  int n, rc;
  register unsigned int i = 0;

  /* Unless a revocation store for CRLs was created we cannot do any
   * CRL-based verification, of course.
   */
  if (!tls_crl_store) {
    return ok;
  }

  tls_log("CRL store present, checking client certificate against configured "
    "CRLs");

  /* Determine certificate ingredients in advance.
   */
  xs = X509_STORE_CTX_get_current_cert(ctx);
  subject = X509_get_subject_name(xs);
  issuer = X509_get_issuer_name(xs);

  /* OpenSSL provides the general mechanism to deal with CRLs but does not
   * use them automatically when verifying certificates, so we do it
   * explicitly here. We will check the CRL for the currently checked
   * certificate, if there is such a CRL in the store.
   *
   * We come through this procedure for each certificate in the certificate
   * chain, starting with the root-CA's certificate. At each step we've to
   * both verify the signature on the CRL (to make sure it's a valid CRL)
   * and its revocation list (to make sure the current certificate isn't
   * revoked).  But because to check the signature on the CRL we need the
   * public key of the issuing CA certificate (which was already processed
   * one round before), we've a little problem. But we can both solve it and
   * at the same time optimize the processing by using the following
   * verification scheme (idea and code snippets borrowed from the GLOBUS
   * project):
   *
   * 1. We'll check the signature of a CRL in each step when we find a CRL
   *    through the _subject_ name of the current certificate. This CRL
   *    itself will be needed the first time in the next round, of course.
   *    But we do the signature processing one round before this where the
   *    public key of the CA is available.
   *
   * 2. We'll check the revocation list of a CRL in each step when
   *    we find a CRL through the _issuer_ name of the current certificate.
   *    This CRLs signature was then already verified one round before.
   *
   * This verification scheme allows a CA to revoke its own certificate as
   * well, of course.
   */

  /* Try to retrieve a CRL corresponding to the _subject_ of
   * the current certificate in order to verify its integrity.
   */
  memset(&obj, 0, sizeof(obj));
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  if (X509_STORE_CTX_init(&store_ctx, tls_crl_store, NULL, NULL) <= 0) {
    tls_log("error initializing CRL store context: %s", tls_get_errors());
    return ok;
  }
#else
  X509_STORE_CTX_init(&store_ctx, tls_crl_store, NULL, NULL);
#endif

  rc = X509_STORE_get_by_subject(&store_ctx, X509_LU_CRL, subject, &obj);
  X509_STORE_CTX_cleanup(&store_ctx);
  crl = obj.data.crl;

  if (rc > 0 &&
      crl != NULL) {
    EVP_PKEY *pubkey;
    char buf[512];
    int len;
    BIO *b = BIO_new(BIO_s_mem());

    BIO_printf(b, "CA CRL: Issuer: ");
    X509_NAME_print(b, issuer, 0);

    BIO_printf(b, ", lastUpdate: ");
    ASN1_UTCTIME_print(b, crl->crl->lastUpdate);

    BIO_printf(b, ", nextUpdate: ");
    ASN1_UTCTIME_print(b, crl->crl->nextUpdate);

    len = BIO_read(b, buf, sizeof(buf) - 1);
    buf[strlen(buf)-1] = '\0';

    if (len >= sizeof(buf)) {
      len = sizeof(buf)-1;
    }
    buf[len] = '\0';

    BIO_free(b);

    tls_log("%s", buf);

    pubkey = X509_get_pubkey(xs);

    /* Verify the signature on this CRL */
    if (X509_CRL_verify(crl, pubkey) <= 0) {
      tls_log("invalid signature on CRL: %s", tls_get_errors());
      if (pubkey)
        EVP_PKEY_free(pubkey);

      X509_STORE_CTX_set_error(ctx, X509_V_ERR_CRL_SIGNATURE_FAILURE);
      X509_OBJECT_free_contents(&obj);
      return 0;
    }

    if (pubkey)
      EVP_PKEY_free(pubkey);

    /* Check date of CRL to make sure it's not expired */
    i = X509_cmp_current_time(X509_CRL_get_nextUpdate(crl));
    if (i == 0) {
      tls_log("CRL has invalid nextUpdate field: %s", tls_get_errors());
      X509_STORE_CTX_set_error(ctx, X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD);
      X509_OBJECT_free_contents(&obj);
      return 0;
    }

    if (i < 0) {
      tls_log("%s", "CRL is expired, revoking all certificates until an "
        "updated CRL is obtained");
      X509_STORE_CTX_set_error(ctx, X509_V_ERR_CRL_HAS_EXPIRED);
      X509_OBJECT_free_contents(&obj);
      return 0;
    }

    X509_OBJECT_free_contents(&obj);
  }

  /* Try to retrieve a CRL corresponding to the _issuer_ of
   * the current certificate in order to check for revocation.
   */
  memset(&obj, 0, sizeof(obj));
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  if (X509_STORE_CTX_init(&store_ctx, tls_crl_store, NULL, NULL) == 0) {
    tls_log("error initializing CRL store context: %s", tls_get_errors());
    return ok;
  }
#else
  X509_STORE_CTX_init(&store_ctx, tls_crl_store, NULL, NULL);
#endif

  rc = X509_STORE_get_by_subject(&store_ctx, X509_LU_CRL, issuer, &obj);
  X509_STORE_CTX_cleanup(&store_ctx);
  crl = obj.data.crl;

  if (rc > 0 &&
      crl != NULL) {

    /* Check if the current certificate is revoked by this CRL */
    n = sk_X509_REVOKED_num(X509_CRL_get_REVOKED(crl));

    for (i = 0; i < n; i++) {
      ASN1_INTEGER *sn;

      revoked = sk_X509_REVOKED_value(X509_CRL_get_REVOKED(crl), i);
      sn = revoked->serialNumber;

      if (ASN1_INTEGER_cmp(sn, X509_get_serialNumber(xs)) == 0) {
        long serial = ASN1_INTEGER_get(sn);
        char *cp = tls_x509_name_oneline(issuer);

        tls_log("certificate with serial %ld (0x%lX) revoked per CRL from "
          "issuer '%s'", serial, serial, cp ? cp : "(ERROR)");

        X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_REVOKED);
        X509_OBJECT_free_contents(&obj);
        return 0;
      }
    }

    X509_OBJECT_free_contents(&obj);
  }

  return ok;
}

static ssize_t tls_write(SSL *ssl, const void *buf, size_t len) {
  ssize_t count;

  count = SSL_write(ssl, buf, len);

  if (count < 0) {
    int err = SSL_get_error(ssl, count);

    /* write(2) returns only the generic error number -1 */
    count = -1;

    switch (err) {
      case SSL_ERROR_WANT_WRITE:
        /* Simulate an EINTR in case OpenSSL wants to write more. */
        errno = EINTR;
        break;

      default:
        tls_fatal_error(err, __LINE__);
        break;
    }
  }

  return count;
}

static char *tls_x509_name_oneline(X509_NAME *x509_name) {
  static char buf[1024] = {'\0'};

  /* If we are using OpenSSL 0.9.6 or newer, we want to use X509_NAME_print_ex()
   * instead of X509_NAME_oneline().
   */

#if OPENSSL_VERSION_NUMBER < 0x000906000L
  memset(&buf, '\0', sizeof(buf));
  return X509_NAME_oneline(x509_name, buf, sizeof(buf));
#else

  /* Sigh...do it the hard way. */
  BIO *mem = BIO_new(BIO_s_mem());
  char *data = NULL;
  long datalen = 0;
  int ok;
   
  ok = X509_NAME_print_ex(mem, x509_name, 0, XN_FLAG_ONELINE);
  if (ok) {
    datalen = BIO_get_mem_data(mem, &data);

    if (data) {
      memset(&buf, '\0', sizeof(buf));

      if (datalen >= sizeof(buf)) {
        datalen = sizeof(buf)-1;
      }

      memcpy(buf, data, datalen);

      buf[datalen] = '\0';
      buf[sizeof(buf)-1] = '\0';

      BIO_free(mem);
      return buf;
    }
  }

  BIO_free(mem);
  return NULL;
#endif /* OPENSSL_VERSION_NUMBER >= 0x000906000 */
}

/* NetIO callbacks
 */

static void tls_netio_abort_cb(pr_netio_stream_t *nstrm) {
  nstrm->strm_flags |= PR_NETIO_SESS_ABORT;
}

static int tls_netio_close_cb(pr_netio_stream_t *nstrm) {
  int res = 0;

  if (nstrm->strm_data) {

    if (nstrm->strm_type == PR_NETIO_STRM_CTRL &&
        nstrm->strm_mode == PR_NETIO_IO_WR) {
      tls_end_sess((SSL *) nstrm->strm_data, nstrm->strm_type, TRUE);
      tls_ctrl_rd_nstrm->strm_data = tls_ctrl_wr_nstrm->strm_data =
        nstrm->strm_data = NULL;
      tls_ctrl_netio = NULL;
      tls_flags &= ~TLS_SESS_ON_CTRL;
    }

    if (nstrm->strm_type == PR_NETIO_STRM_DATA &&
        nstrm->strm_mode == PR_NETIO_IO_WR) {
      tls_end_sess((SSL *) nstrm->strm_data, nstrm->strm_type, TRUE);
      tls_data_rd_nstrm->strm_data = tls_data_wr_nstrm->strm_data =
        nstrm->strm_data = NULL;
      tls_data_netio = NULL;
      tls_flags &= ~TLS_SESS_ON_DATA;
    }
  }

  res = close(nstrm->strm_fd);
  nstrm->strm_fd = -1;

  return res;
}

static pr_netio_stream_t *tls_netio_open_cb(pr_netio_stream_t *nstrm, int fd,
    int mode) {
  nstrm->strm_fd = fd;
  nstrm->strm_mode = mode;

  /* Cache a pointer to this stream. */
  if (nstrm->strm_type == PR_NETIO_STRM_CTRL) {
    if (nstrm->strm_mode == PR_NETIO_IO_RD)
      tls_ctrl_rd_nstrm = nstrm;

    if (nstrm->strm_mode == PR_NETIO_IO_WR)
      tls_ctrl_wr_nstrm = nstrm;

  } else if (nstrm->strm_type == PR_NETIO_STRM_DATA) {
    if (nstrm->strm_mode == PR_NETIO_IO_RD)
      tls_data_rd_nstrm = nstrm;

    if (nstrm->strm_mode == PR_NETIO_IO_WR)
      tls_data_wr_nstrm = nstrm;

    /* Note: from the FTP-TLS Draft 9.2:
     * 
     *  It is quite reasonable for the server to insist that the data
     *  connection uses a TLS cached session.  This might be a cache of a
     *  previous data connection or of the control connection.  If this is
     *  the reason for the the refusal to allow the data transfer then the
     *  '522' reply should indicate this.
     * 
     * and, from 10.4:
     *   
     *   If a server needs to have the connection protected then it will
     *   reply to the STOR/RETR/NLST/... command with a '522' indicating
     *   that the current state of the data connection protection level is
     *   not sufficient for that data transfer at that time.
     *
     * This points out the need for a module to be able to influence
     * command response codes in a more flexible manner...
     */
  }

  return nstrm;
}

static int tls_netio_poll_cb(pr_netio_stream_t *nstrm) {
  fd_set rfds, wfds;
  struct timeval tval;

  FD_ZERO(&rfds);
  FD_ZERO(&wfds);

  if (nstrm->strm_mode == PR_NETIO_IO_RD)
    FD_SET(nstrm->strm_fd, &rfds);

  else
    FD_SET(nstrm->strm_fd, &wfds);

  tval.tv_sec = (nstrm->strm_flags & PR_NETIO_SESS_INTR) ?
    nstrm->strm_interval : 10;
  tval.tv_usec = 0;

  return select(nstrm->strm_fd + 1, &rfds, &wfds, NULL, &tval);
}

static int tls_netio_postopen_cb(pr_netio_stream_t *nstrm) {

  /* If this is a data stream, and it's for writing, and TLS is required,
   * then do a TLS handshake.
   */

  if (nstrm->strm_type == PR_NETIO_STRM_DATA &&
      nstrm->strm_mode == PR_NETIO_IO_WR) {

    /* Enforce the "data" part of TLSRequired, if configured. */
    if (tls_required_on_data ||
        (tls_flags & TLS_SESS_NEED_DATA_PROT)) {
      X509 *ctrl_cert = NULL, *data_cert = NULL;

      tls_log("%s", "starting TLS negotiation on data connection");
      if (tls_accept(session.d, TRUE) < 0) {
        tls_log("%s", "unable to open data connection: TLS negotiation failed");
        session.d->xerrno = EPERM;
        return -1;
      }

      /* Make sure that the certificate used, if any, for this data channel
       * handshake is the same as that used for the control channel handshake.
       * This may be too strict of a requirement, though.
       */
      ctrl_cert = SSL_get_peer_certificate(ctrl_ssl);
      data_cert = SSL_get_peer_certificate((SSL *) nstrm->strm_data);

      if (ctrl_cert && data_cert) {
        if (X509_cmp(ctrl_cert, data_cert)) {
          X509_free(ctrl_cert);
          X509_free(data_cert);

          /* Properly shutdown the SSL session. */
          tls_end_sess((SSL *) nstrm->strm_data, nstrm->strm_type, TRUE);

          tls_data_rd_nstrm->strm_data = tls_data_wr_nstrm->strm_data =
            nstrm->strm_data = NULL;

          tls_log("%s", "unable to open data connection: control/data "
            "certificate mismatch");

          session.d->xerrno = EPERM;
          return -1;
        }
      }

#if OPENSSL_VERSION_NUMBER < 0x0090702fL
      /* Make sure blinding is turned on. (For some reason, this only seems
       * to be allowed on SSL objects, not on SSL_CTX objects.  Bummer).
       */
      tls_blinding_on((SSL *) nstrm->strm_data);
#endif

      if (ctrl_cert)
        X509_free(ctrl_cert);

      if (data_cert)
        X509_free(data_cert);

      tls_flags |= TLS_SESS_ON_DATA;
    }
  }

  return 0;
}

static int tls_netio_read_cb(pr_netio_stream_t *nstrm, char *buf,
    size_t buflen) {

  if (nstrm->strm_data)
    return tls_read((SSL *) nstrm->strm_data, buf, buflen);

  return read(nstrm->strm_fd, buf, buflen);
}

static pr_netio_stream_t *tls_netio_reopen_cb(pr_netio_stream_t *nstrm, int fd,
    int mode) {

  if (nstrm->strm_fd != -1)
    close(nstrm->strm_fd);

  nstrm->strm_fd = fd;
  nstrm->strm_mode = mode;

  /* NOTE: a no-op? */
  return nstrm;
}

static int tls_netio_shutdown_cb(pr_netio_stream_t *nstrm, int how) {
  return shutdown(nstrm->strm_fd, how);
}

static int tls_netio_write_cb(pr_netio_stream_t *nstrm, char *buf,
    size_t buflen) {

  if (nstrm->strm_data) {

#if OPENSSL_VERSION_NUMBER > 0x000907000L
    if (tls_data_renegotiate_limit &&
        session.xfer.total_bytes >= tls_data_renegotiate_limit) {
      tls_log("%s", "requesting TLS renegotiation on data channel");
      SSL_renegotiate((SSL *) nstrm->strm_data);
      /* SSL_do_handshake((SSL *) nstrm->strm_data); */

      pr_timer_add(tls_renegotiate_timeout, 0, &tls_module,
        tls_renegotiate_timeout_cb);

      tls_flags |= TLS_SESS_DATA_RENEGOTIATING;
    }
#endif

    return tls_write((SSL *) nstrm->strm_data, buf, buflen);
  }

  return write(nstrm->strm_fd, buf, buflen);
}

static void tls_netio_install_ctrl(void) {
  pr_netio_t *netio = tls_ctrl_netio ? tls_ctrl_netio :
    (tls_ctrl_netio = pr_alloc_netio(session.pool ? session.pool :
    permanent_pool));

  netio->abort = tls_netio_abort_cb;
  netio->close = tls_netio_close_cb;
  netio->open = tls_netio_open_cb;
  netio->poll = tls_netio_poll_cb;
  netio->postopen = tls_netio_postopen_cb;
  netio->read = tls_netio_read_cb;
  netio->reopen = tls_netio_reopen_cb;
  netio->shutdown = tls_netio_shutdown_cb;
  netio->write = tls_netio_write_cb;

  pr_unregister_netio(PR_NETIO_STRM_CTRL);

  if (pr_register_netio(netio, PR_NETIO_STRM_CTRL) < 0)
    pr_log_pri(PR_LOG_INFO, MOD_TLS_VERSION ": error registering netio: %s",
      strerror(errno));
}

static void tls_netio_install_data(void) {
  pr_netio_t *netio = tls_data_netio ? tls_data_netio :
    (tls_data_netio = pr_alloc_netio(session.pool ? session.pool :
    permanent_pool));

  netio->abort = tls_netio_abort_cb;
  netio->close = tls_netio_close_cb;
  netio->open = tls_netio_open_cb;
  netio->poll = tls_netio_poll_cb;
  netio->postopen = tls_netio_postopen_cb;
  netio->read = tls_netio_read_cb;
  netio->reopen = tls_netio_reopen_cb;
  netio->shutdown = tls_netio_shutdown_cb;
  netio->write = tls_netio_write_cb;

  pr_unregister_netio(PR_NETIO_STRM_DATA);

  if (pr_register_netio(netio, PR_NETIO_STRM_DATA) < 0)
    pr_log_pri(PR_LOG_INFO, MOD_TLS_VERSION ": error registering netio: %s",
      strerror(errno));
}

/* Logging functions
 */

static void tls_closelog(void) {

  /* Sanity check */
  if (tls_logfd != -1) {
    close(tls_logfd);
    tls_logfd = -1;
    tls_logname = NULL;
  }

  return;
}

static int tls_log(const char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  time_t timestamp = time(NULL);
  struct tm *t = NULL;
  va_list msg;

  /* Sanity check */
  if (!tls_logname)
    return 0;

  t = pr_localtime(NULL, &timestamp);

  /* Prepend the timestamp */
  strftime(buf, sizeof(buf), "%b %d %H:%M:%S ", t);
  buf[sizeof(buf)-1] = '\0';

  /* Prepend a small header */
  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), MOD_TLS_VERSION
    "[%u]: ", (unsigned int) getpid());
  buf[sizeof(buf)-1] = '\0';

  /* Affix the message */
  va_start(msg, fmt);
  vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, msg);
  va_end(msg);

  buf[sizeof(buf)-1] = '\0';
  buf[strlen(buf)] = '\n';

  if (write(tls_logfd, buf, strlen(buf)) < 0)
    return -1;

  return 0;
}

static int tls_openlog(void) {
  int res = 0;

  /* Sanity checks */
  tls_logname = get_param_ptr(main_server->conf, "TLSLog", FALSE);
  if (tls_logname == NULL)
    return 0;

  if (strcasecmp(tls_logname, "none") == 0) {
    tls_logname = NULL;
    return 0;
  }

  pr_signals_block();
  PRIVS_ROOT
  res = pr_log_openfile(tls_logname, &tls_logfd, 0600);
  PRIVS_RELINQUISH
  pr_signals_unblock();

  return res;
}

/* Authentication handlers
 */

/* This function does the main authentication work, and is called in the
 * normal course of events:
 *
 *   cmd->argv[0]: user name
 *   cmd->argv[1]: cleartext password
 */
MODRET tls_authenticate(cmd_rec *cmd) {
  if (!tls_engine)
    return PR_DECLINED(cmd);

  /* Possible authentication combinations:
   *
   *  TLS handshake + passwd (default)
   *  TLS handshake + .tlslogin (passwd ignored)
   */

  if ((tls_flags & TLS_SESS_ON_CTRL) &&
      (tls_opts & TLS_OPT_ALLOW_DOT_LOGIN)) {

    if (tls_dotlogin_allow(cmd->argv[0])) {
      tls_log("TLS/X509 .tlslogin check successful for user '%s'",
       cmd->argv[0]);
      pr_log_auth(PR_LOG_NOTICE, "USER %s: TLS/X509 .tlslogin authentication "
        "successful", cmd->argv[0]);
      session.auth_mech = "mod_tls.c";
      return mod_create_data(cmd, (void *) PR_AUTH_RFC2228_OK);

    } else
      tls_log("TLS/X509 .tlslogin check failed for user '%s'",
        cmd->argv[0]);
  }

  return PR_DECLINED(cmd);
}

/* This function is called only when UserPassword is involved, used to
 * override the configured password for a user.
 *
 *  cmd->argv[0]: hashed password (from proftpd.conf)
 *  cmd->argv[1]: user name
 *  cmd->argv[2]: cleartext password
 */
MODRET tls_auth_check(cmd_rec *cmd) {
  if (!tls_engine)
    return PR_DECLINED(cmd);

  /* Possible authentication combinations:
   *
   *  TLS handshake + passwd (default)
   *  TLS handshake + .tlslogin (passwd ignored)
   */

  if ((tls_flags & TLS_SESS_ON_CTRL) &&
      (tls_opts & TLS_OPT_ALLOW_DOT_LOGIN)) {

    if (tls_dotlogin_allow(cmd->argv[1])) {
      tls_log("TLS/X509 .tlslogin check successful for user '%s'",
       cmd->argv[0]);
      pr_log_auth(PR_LOG_NOTICE, "USER %s: TLS/X509 .tlslogin authentication "
        "successful", cmd->argv[1]);
      session.auth_mech = "mod_tls.c";
      return mod_create_data(cmd, (void *) PR_AUTH_RFC2228_OK);

    } else
      tls_log("TLS/X509 .tlslogin check failed for user '%s'",
        cmd->argv[1]);
  }

  return PR_DECLINED(cmd);
}

/* Command handlers
 */

MODRET tls_any(cmd_rec *cmd) {
  if (!tls_engine)
    return PR_DECLINED(cmd);

  /* NOTE: possibly add checks of commands here in order to support the
   * ability of having TLSRequired in per-directory configurations.  This
   * would mean watching for directory change commands, file transfer
   * commands, and doing a context check in order to appropriately set
   * the value of tls_required_on_data.
   */

  /* Some commands need not be hindered. */
  if (strcmp(cmd->argv[0], C_SYST) == 0 ||
      strcmp(cmd->argv[0], C_AUTH) == 0 ||
      strcmp(cmd->argv[0], C_QUIT) == 0)
    return PR_DECLINED(cmd);

  if (tls_required_on_auth &&
      !(tls_flags & TLS_SESS_ON_CTRL)) {
    if (strcmp(cmd->argv[0], C_USER) == 0 ||
        strcmp(cmd->argv[0], C_PASS) == 0 ||
        strcmp(cmd->argv[0], C_ACCT) == 0) {
      pr_response_add_err(R_550, "SSL/TLS required on the control channel");
      return PR_ERROR(cmd);
    }
  }

  if (tls_required_on_ctrl &&
      !(tls_flags & TLS_SESS_ON_CTRL)) {

    if (!(tls_opts & TLS_OPT_ALLOW_PER_USER)) {
      tls_log("SSL/TLS required but absent on control channel, "
        "denying %s command", cmd->argv[0]);
      pr_response_add_err(R_550, "SSL/TLS required on the control channel");
      return PR_ERROR(cmd);

    } else {

      if (tls_authenticated &&
          *tls_authenticated == TRUE) {
        tls_log("SSL/TLS required but absent on control channel, "
          "denying %s command", cmd->argv[0]);
        pr_response_add_err(R_550, "SSL/TLS required on the control channel");
        return PR_ERROR(cmd);
      }
    }
  }

  if (tls_required_on_data &&
      !(tls_flags & TLS_SESS_NEED_DATA_PROT)) {
    if (strcmp(cmd->argv[0], C_APPE) == 0 ||
        strcmp(cmd->argv[0], C_LIST) == 0 ||
        strcmp(cmd->argv[0], C_NLST) == 0 ||
        strcmp(cmd->argv[0], C_RETR) == 0 ||
        strcmp(cmd->argv[0], C_STOR) == 0 ||
        strcmp(cmd->argv[0], C_STOU) == 0) {
      tls_log("SSL/TLS required but absent on data channel, "
        "denying %s command", cmd->argv[0]);
      pr_response_add_err(R_550, "SSL/TLS required on the data channel");
      return PR_ERROR(cmd);
    }
  }

  return PR_DECLINED(cmd);
}

MODRET tls_auth(cmd_rec *cmd) {
  register unsigned int i = 0;

  if (!tls_engine)
    return PR_DECLINED(cmd);

  /* NOTE: need to make sure that AUTH cannot be used after USER has been
   * issued/processed (not without a REIN, anyway).
   */

  if (cmd->argc < 2) {
    pr_response_add_err(R_504, "AUTH requires at least one argument");
    return PR_ERROR(cmd);
  }

  if (tls_flags & TLS_SESS_HAVE_CCC) {
    tls_log("Unwilling to accept AUTH after CCC for this session");
    pr_response_add_err(R_534, "Unwilling to accept security parameters");
    return PR_ERROR(cmd);
  }

  /* Convert the parameter to upper case */
  for (i = 0; i < strlen(cmd->argv[1]); i++)
    (cmd->argv[1])[i] = toupper((cmd->argv[1])[i]);

  if (strcmp(cmd->argv[1], "TLS") == 0 ||
      strcmp(cmd->argv[1], "TLS-C") == 0) {
    pr_response_send(R_234, "AUTH %s successful", cmd->argv[1]);

    tls_log("%s", "TLS/TLS-C requested, starting TLS handshake");
    if (tls_accept(session.c, FALSE) < 0) {
      tls_log("%s", "TLS/TLS-C negotiation failed on control channel");

      if (tls_required_on_ctrl)
        end_login(1);

      /* If we reach this point, the debug logging may show gibberish
       * commands from the client.  In reality, this gibberish is probably
       * more encrypted data from the client.
       */
      pr_response_add_err(R_550, "TLS handshake failed");
      return PR_ERROR(cmd);
    }

#if OPENSSL_VERSION_NUMBER < 0x0090702fL
    /* Make sure blinding is turned on. (For some reason, this only seems
     * to be allowed on SSL objects, not on SSL_CTX objects.  Bummer).
     */
    tls_blinding_on(ctrl_ssl);
#endif

     tls_flags |= TLS_SESS_ON_CTRL;

  } else if (strcmp(cmd->argv[1], "SSL") == 0 ||
     strcmp(cmd->argv[1], "TLS-P") == 0) {
    pr_response_send(R_234, "AUTH %s successful", cmd->argv[1]);

    tls_log("%s", "SSL/TLS-P requested, starting TLS handshake");
    if (tls_accept(session.c, FALSE) < 0) {
      tls_log("%s", "SSL/TLS-P negotiation failed on control channel");

      if (tls_required_on_ctrl)
        end_login(1);

      /* If we reach this point, the debug logging may show gibberish
       * commands from the client.  In reality, this gibberish is probably
       * more encrypted data from the client.
       */
      pr_response_add_err(R_550, "TLS handshake failed");
      return PR_ERROR(cmd);
    }

#if OPENSSL_VERSION_NUMBER < 0x0090702fL
    /* Make sure blinding is turned on. (For some reason, this only seems
     * to be allowed on SSL objects, not on SSL_CTX objects.  Bummer).
     */
    tls_blinding_on(ctrl_ssl);
#endif

    tls_flags |= TLS_SESS_ON_CTRL;
    tls_flags |= TLS_SESS_NEED_DATA_PROT;

  } else {
    tls_log("AUTH %s unsupported, declining", cmd->argv[1]);

    /* Allow other RFC2228 modules a chance a handling this command. */
    return PR_DECLINED(cmd);
  }

  session.rfc2228_mech = "TLS";
  return PR_HANDLED(cmd);
}

MODRET tls_ccc(cmd_rec *cmd) {

  if (!tls_engine ||
      !session.rfc2228_mech ||
      strcmp(session.rfc2228_mech, "TLS") != 0)
    return PR_DECLINED(cmd);

  if (!(tls_flags & TLS_SESS_ON_CTRL)) {
    pr_response_add_err(R_533,
      "CCC not allowed on insecure control connection");
    return PR_ERROR(cmd);
  }

  if (tls_required_on_ctrl) {
    pr_response_add_err(R_534, "Unwilling to accept security parameters");
    tls_log("%s: unwilling to accept security parameters: "
      "TLSRequired setting does not allow for unprotected control channel",
      cmd->argv[0]);
    return PR_ERROR(cmd);
  }

  /* Check for <Limit> restrictions. */
  if (!dir_check(cmd->tmp_pool, C_CCC, G_NONE, session.cwd, NULL)) {
    pr_response_add_err(R_534, "Unwilling to accept security parameters");
    tls_log("%s: unwilling to accept security parameters", cmd->argv[0]);
    return PR_ERROR(cmd);
  }

  tls_log("received CCC, clearing control channel protection");

  /* Send the OK response asynchronously; the spec dictates that the
   * response be sent prior to performing the SSL session shutdown.
   */
  pr_response_send_async(R_200, "Clearing control channel protection");

  /* Close the SSL session, but only one the control channel.
   * The data channel, if protected, should remain so.
   */

  tls_end_sess(ctrl_ssl, PR_NETIO_STRM_CTRL, FALSE);
  ctrl_ssl = tls_ctrl_rd_nstrm->strm_data = tls_ctrl_wr_nstrm->strm_data = NULL;

  /* Remove our NetIO for the control channel. */
  pr_unregister_netio(PR_NETIO_STRM_CTRL);

  tls_flags &= ~TLS_SESS_ON_CTRL;
  tls_flags |= TLS_SESS_HAVE_CCC;

  return PR_HANDLED(cmd);
}

MODRET tls_pbsz(cmd_rec *cmd) {

  if (!tls_engine ||
      !session.rfc2228_mech ||
      strcmp(session.rfc2228_mech, "TLS") != 0)
    return PR_DECLINED(cmd);

  CHECK_CMD_ARGS(cmd, 2);

  if (!(tls_flags & TLS_SESS_ON_CTRL)) {
    pr_response_add_err(R_503,
      "PBSZ not allowed on insecure control connection");
    return PR_ERROR(cmd);
  }

  /* We expect "PBSZ 0" */
  if (strcmp(cmd->argv[1], "0") == 0)
    pr_response_add(R_200, "PBSZ 0 successful");
  else
    pr_response_add(R_200, "PBSZ=0 successful");

  tls_flags |= TLS_SESS_PBSZ_OK;
  return PR_HANDLED(cmd);
}

MODRET tls_post_pass(cmd_rec *cmd) {

  if (!tls_engine)
    return PR_DECLINED(cmd);

  if (!(tls_opts & TLS_OPT_ALLOW_PER_USER))
    return PR_DECLINED(cmd);

  tls_authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);

  if (tls_authenticated &&
      *tls_authenticated == TRUE) {
    config_rec *c;

    c = find_config(TOPLEVEL_CONF, CONF_PARAM, "TLSRequired", FALSE);
    if (c) {

      /* Lookup the TLSRequired directive again in this context (which could be
       * <Anonymous>, for example, or modified by mod_ifsession).
       */

      tls_required_on_ctrl = *((unsigned char *) c->argv[0]);
      tls_required_on_data = *((unsigned char *) c->argv[1]);
      tls_required_on_auth = *((unsigned char *) c->argv[2]);
    }
  }

  return PR_DECLINED(cmd);
}

MODRET tls_prot(cmd_rec *cmd) {

  if (!tls_engine ||
      !session.rfc2228_mech ||
      strcmp(session.rfc2228_mech, "TLS") != 0)
    return PR_DECLINED(cmd);

  CHECK_CMD_ARGS(cmd, 2);

  if (!(tls_flags & TLS_SESS_ON_CTRL) &&
      !(tls_flags & TLS_SESS_HAVE_CCC)) {
    pr_response_add_err(R_503,
      "PROT not allowed on insecure control connection");
    return PR_ERROR(cmd);
  }

  if (!(tls_flags & TLS_SESS_PBSZ_OK)) {
    pr_response_add_err(R_503, "You must issue the PBSZ command prior to PROT");
    return PR_ERROR(cmd);
  }

  /* Only PROT C or PROT P is valid with respect to SSL/TLS. */
  if (strcmp(cmd->argv[1], "C") == 0) {
    char *mesg = "Protection set to Clear";

    if (!tls_required_on_data) {

      /* Only accept this if SSL/TLS is not required, by policy, on data
       * connections.
       */
      tls_flags &= ~TLS_SESS_NEED_DATA_PROT;
      pr_response_add(R_200, "%s", mesg);
      tls_log("%s", mesg);

    } else {
      pr_response_add_err(R_534, "Unwilling to accept security parameters");
      tls_log("%s: TLSRequired requires protection for data transfers",
        cmd->argv[0]);
      tls_log("%s: unwilling to accept security parameter (%s)", cmd->argv[0],
        cmd->argv[1]);
      return PR_ERROR(cmd);
    }

  } else if (strcmp(cmd->argv[1], "P") == 0) {
    char *mesg = "Protection set to Private";

    tls_flags |= TLS_SESS_NEED_DATA_PROT;
    pr_response_add(R_200, "%s", mesg);
    tls_log("%s", mesg);

  } else if (strcmp(cmd->argv[1], "S") == 0 ||
             strcmp(cmd->argv[1], "E") == 0) {
    pr_response_add_err(R_536, "PROT %s unsupported", cmd->argv[1]);

    /* By the time the logic reaches this point, there must have been
     * an SSL/TLS session negotiated; other AUTH mechanisms will handle
     * things differently, and when they do, the logic of this handler
     * would not reach this point.  This means that it would not be impolite
     * to return ERROR here, rather than DECLINED: it shows that mod_tls
     * is handling the security mechanism, and that this module does not
     * allow for the unsupported PROT levels.
     */
    return PR_ERROR(cmd);

  } else {
    pr_response_add_err(R_504, "PROT %s unsupported", cmd->argv[1]);
    return PR_ERROR(cmd);
  }

  return PR_HANDLED(cmd);
}

/* Configuration handlers
 */

/* usage: TLSCACertificateFile file */
MODRET set_tlscacertfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1]))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSCACertificatePath path */
MODRET set_tlscacertpath(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

   if (!dir_exists(cmd->argv[1]))
    CONF_ERROR(cmd, "parameter must be a directory path");

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");
 
  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]); 
  return PR_HANDLED(cmd);
}

/* usage: TLSCARevocationFile file */
MODRET set_tlscacrlfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1]))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSCARevocationPath path */
MODRET set_tlscacrlpath(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

   if (!dir_exists(cmd->argv[1]))
    CONF_ERROR(cmd, "parameter must be a directory path");

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSCertificateChainFile file */
MODRET set_tlscertchain(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1]))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSCipherSuite string */
MODRET set_tlsciphersuite(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSCryptoDevice driver|"ALL"|"NONE" */
MODRET set_tlscryptodevice(cmd_rec *cmd) {
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);

#else /* OpenSSL is too old for ENGINE support */
  CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "The ", cmd->argv[0],
    "directive cannot be used on the system, as the OpenSSL version is too old",
    NULL));
#endif
}

/* usage: TLSDHParamFile file */
MODRET set_tlsdhparamfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1]))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSDSACertificateFile file */
MODRET set_tlsdsacertfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1])) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));
  }

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSDSACertificateKeyFile file */
MODRET set_tlsdsakeyfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1])) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));
  }

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSEngine on|off */
MODRET set_tlsengine(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((bool = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* usage: TLSLog file */
MODRET set_tlslog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSOptions opt1 opt2 ... */
MODRET set_tlsoptions(cmd_rec *cmd) {
  config_rec *c = NULL;
  register unsigned int i = 0;
  unsigned long opts = 0UL;

  if (cmd->argc-1 == 0)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);

  for (i = 1; i < cmd->argc; i++) {
    if (strcmp(cmd->argv[i], "AllowDotLogin") == 0)
      opts |= TLS_OPT_ALLOW_DOT_LOGIN;

    else if (strcmp(cmd->argv[i], "AllowPerUser") == 0)
      opts |= TLS_OPT_ALLOW_PER_USER;

    else if (strcmp(cmd->argv[i], "ExportCertData") == 0)
      opts |= TLS_OPT_EXPORT_CERT_DATA;

    else if (strcmp(cmd->argv[i], "NoCertRequest") == 0)
      opts |= TLS_OPT_NO_CERT_REQUEST;

    else if (strcmp(cmd->argv[i], "StdEnvVars") == 0)
      opts |= TLS_OPT_STD_ENV_VARS;

    else if (strcmp(cmd->argv[i], "dNSNameRequired") == 0)
      opts |= TLS_OPT_VERIFY_CERT_FQDN;
 
    else if (strcmp(cmd->argv[i], "iPAddressRequired") == 0)
      opts |= TLS_OPT_VERIFY_CERT_IP_ADDR;

    else
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown TLSOption: '",
        cmd->argv[i], "'", NULL));
  }

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[0]) = opts;

  return PR_HANDLED(cmd);
}

/* usage: TLSPassPhraseProvider path */
MODRET set_tlspassphraseprovider(cmd_rec *cmd) {
  struct stat st;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "must be a full path: '",
      cmd->argv[1], "'", NULL));

  if (stat(cmd->argv[1], &st) < 0)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error checking '",
      cmd->argv[1], "': ", strerror(errno), NULL));

  if (!S_ISREG(st.st_mode))
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to use '",
      cmd->argv[1], ": Not a regular file", NULL));

  tls_passphrase_provider = pstrdup(permanent_pool, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSProtocol protocol */
MODRET set_tlsprotocol(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (strcasecmp(cmd->argv[1], "SSLv23") == 0)
    tls_protocol = "SSLv23";

  else if (strcasecmp(cmd->argv[1], "SSLv3") == 0)
    tls_protocol = "SSLv3";

  else if (strcasecmp(cmd->argv[1], "TLSv1") == 0)
    tls_protocol = "TLSv1";

  else
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown protocol: '",
      cmd->argv[1], "'", NULL));

  return PR_HANDLED(cmd);
}

/* usage: TLSRandomSeed file */
MODRET set_tlsrandseed(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  /* NOTE: not yet implemented/used */
  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSRenegotiate [ctrl nsecs] [data nbytes] */
MODRET set_tlsrenegotiate(cmd_rec *cmd) {
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  register unsigned int i = 0;
  config_rec *c = NULL;

  if (cmd->argc-1 < 1 || cmd->argc-1 > 8)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "none") == 0) {
    add_config_param(cmd->argv[0], 0);
    return PR_HANDLED(cmd);
  }

  c = add_config_param(cmd->argv[0], 4, NULL, NULL, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = 0;
  c->argv[1] = pcalloc(c->pool, sizeof(off_t));
  *((off_t *) c->argv[1]) = 0;
  c->argv[2] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[2]) = 0;
  c->argv[3] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[3]) = TRUE;

  for (i = 1; i < cmd->argc;) {
    if (strcmp(cmd->argv[i], "ctrl") == 0) {
      int secs = atoi(cmd->argv[i+1]);

      if (secs > 0)
        *((int *) c->argv[0]) = secs;

      else
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": ", 
          cmd->argv[i], " must be greater than zero: '", cmd->argv[i+1], "'",
          NULL));

      i += 2;

    } else if (strcmp(cmd->argv[i], "data") == 0) {
      char *tmp = NULL;
      unsigned long kbytes = strtoul(cmd->argv[i+1], &tmp, 10);

      if (!(tmp && *tmp))
        *((off_t *) c->argv[1]) = (off_t) kbytes * 1024;

      else
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": ",
          cmd->argv[i], " must be greater than zero: '", cmd->argv[i+1], "'",
          NULL));

      i += 2;

    } else if (strcmp(cmd->argv[i], "required") == 0) {
      int bool = get_boolean(cmd, i+1);

      if (bool != -1)
        *((unsigned char *) c->argv[3]) = bool;

      else
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": ",
          cmd->argv[i], " must be a Boolean value: '", cmd->argv[i+1], "'",
          NULL));

      i += 2;

    } else if (strcmp(cmd->argv[i], "timeout") == 0) {
      int secs = atoi(cmd->argv[i+1]);
      
      if (secs > 0)
        *((int *) c->argv[2]) = secs;

      else
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": ", 
          cmd->argv[i], " must be greater than zero: '", cmd->argv[i+1], "'",
          NULL));

      i += 2;
    }
  }

  return PR_HANDLED(cmd);
#else
  CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, " requires OpenSSL-0.9.7 or greater",
    NULL));
#endif
}

/* usage: TLSRequired on|off|both|ctrl|control|data|auth|auth+data */
MODRET set_tlsrequired(cmd_rec *cmd) {
  int bool = -1;
  unsigned char on_auth = FALSE, on_ctrl = FALSE, on_data = FALSE;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  bool = get_boolean(cmd, 1);
  if (bool == -1) {
    if (strcmp(cmd->argv[1], "control") == 0 ||
        strcmp(cmd->argv[1], "ctrl") == 0) {
      on_auth = TRUE;
      on_ctrl = TRUE;

    } else if (strcmp(cmd->argv[1], "data") == 0) {
      on_data = TRUE;

    } else if (strcmp(cmd->argv[1], "both") == 0 ||
               strcmp(cmd->argv[1], "ctrl+data") == 0) {
      on_auth = TRUE;
      on_ctrl = TRUE;
      on_data = TRUE;

    } else if (strcmp(cmd->argv[1], "auth") == 0) {
      on_auth = TRUE;

    } else if (strcmp(cmd->argv[1], "auth+data") == 0) {
      on_auth = TRUE;
      on_data = TRUE;

    } else
      CONF_ERROR(cmd, "bad parameter");

  } else {
    if (bool == TRUE) {
      on_auth = TRUE;
      on_ctrl = TRUE;
      on_data = TRUE;
    }
  }

  c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = on_ctrl;
  c->argv[1] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[1]) = on_data;
  c->argv[2] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[2]) = on_auth;

  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: TLSRSACertificateFile file */
MODRET set_tlsrsacertfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1])) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));
  }

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSRSACertificateKeyFile file */
MODRET set_tlsrsakeyfile(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (!file_exists(cmd->argv[1])) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", cmd->argv[1],
      "' does not exist", NULL));
  }

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "parameter must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: TLSTimeoutHandshake <secs> */
MODRET set_tlstimeouthandshake(cmd_rec *cmd) {
  int timeout = -1;
  config_rec *c = NULL;
  char *tmp = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  timeout = (int) strtol(cmd->argv[1], &tmp, 10);

  if ((tmp && *tmp) || timeout < 0 || timeout > 65535)
    CONF_ERROR(cmd, "timeout value must be between 0 and 65535");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

/* usage: TLSVerifyClient on|off */
MODRET set_tlsverifyclient(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((bool = get_boolean(cmd, 1)) == -1)
     CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* usage: TLSVerifyDepth depth */
MODRET set_tlsverifydepth(cmd_rec *cmd) {
  int depth = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((depth = atoi(cmd->argv[1])) < 0)
    CONF_ERROR(cmd, "depth must be zero or greater");
 
  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = depth;
 
  return PR_HANDLED(cmd);
}

/* Event handlers
 */

#if defined(PR_SHARED_MODULE)
static void tls_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_tls.c", (const char *) event_data) == 0) {
    /* Unregister ourselves from all events. */
    pr_event_unregister(&tls_module, NULL, NULL);

    /* Cleanup the OpenSSL stuff. */
    tls_cleanup(0);

    /* Unregister our NetIO handler for the control channel. */
    pr_unregister_netio(PR_NETIO_STRM_CTRL);
  }
}
#endif /* PR_SHARED_MODULE */

static void tls_postparse_ev(const void *event_data, void *user_data) {
  server_rec *s = NULL;
  char buf[256];

  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    config_rec *rsa = NULL, *dsa = NULL;
    tls_pkey_t *k = NULL;

    /* Find any TLS{D,R}SACertificateKeyFile directives.  If they aren't
     * present, look for TLS{D,R}SACertificateFile directives.
     */
    rsa = find_config(s->conf, CONF_PARAM, "TLSRSACertificateKeyFile", FALSE);
    if (!rsa)
      rsa = find_config(s->conf, CONF_PARAM, "TLSRSACertificateFile", FALSE);

    dsa = find_config(s->conf, CONF_PARAM, "TLSDSACertificateKeyFile", FALSE);
    if (!dsa)
      dsa = find_config(s->conf, CONF_PARAM, "TLSDSACertificateFile", FALSE);

    if (!rsa && !dsa)
      continue;

    k = pcalloc(s->pool, sizeof(tls_pkey_t));
    k->pkeysz = PEM_BUFSIZE;
    k->server = s;

    if (rsa) {
      snprintf(buf, sizeof(buf)-1, "RSA key for the %s#%d (%s) server: ",
        pr_netaddr_get_ipstr(s->addr), s->ServerPort, s->ServerName);
      buf[sizeof(buf)-1] = '\0';

      k->rsa_pkey = tls_get_page(PEM_BUFSIZE, &k->rsa_pkey_ptr);
      if (k->rsa_pkey == NULL) {
        pr_log_pri(PR_LOG_ERR, "out of memory!");
        exit(1);
      }

      if (tls_get_passphrase(s, rsa->argv[0], buf, k->rsa_pkey, k->pkeysz,
          TLS_PASSPHRASE_FL_RSA_KEY) < 0) {
        tls_log("error reading RSA passphrase: %s",
          ERR_error_string(ERR_get_error(), NULL));

        pr_log_pri(PR_LOG_ERR, MOD_TLS_VERSION ": unable to use "
          "RSA certificate key in '%s', exiting", (char *) rsa->argv[0]);
        end_login(1);
      }
    }

    if (dsa) {
      snprintf(buf, sizeof(buf)-1, "DSA key for the %s#%d (%s) server: ",
        pr_netaddr_get_ipstr(s->addr), s->ServerPort, s->ServerName);
      buf[sizeof(buf)-1] = '\0';

      k->dsa_pkey = tls_get_page(PEM_BUFSIZE, &k->dsa_pkey_ptr);
      if (k->dsa_pkey == NULL) {
        pr_log_pri(PR_LOG_ERR, "out of memory!");
        exit(1);
      }

      if (tls_get_passphrase(s, dsa->argv[0], buf, k->dsa_pkey, k->pkeysz,
          TLS_PASSPHRASE_FL_DSA_KEY) < 0) {
        tls_log("error reading DSA passphrase: %s",
          ERR_error_string(ERR_get_error(), NULL));

        pr_log_pri(PR_LOG_ERR, MOD_TLS_VERSION ": unable to use "
          "DSA certificate key '%s', exiting", (char *) dsa->argv[0]);
        end_login(1);
      }
    }

    k->next = tls_pkey_list;
    tls_pkey_list = k;
    tls_npkeys++;
  }

  return;
}

/* Daemon PID */
extern pid_t mpid;

static void tls_daemon_exit_ev(const void *event_data, void *user_data) {
  if (mpid == getpid())
    tls_scrub_pkeys();
}

static void tls_restart_ev(const void *event_data, void *user_data) {
  tls_scrub_pkeys();
  tls_closelog();
}

static void tls_sess_exit_ev(const void *event_data, void *user_data) {

  /* OpenSSL cleanup */
  tls_cleanup(0);

  /* Write out a new RandomSeed file, for use later */
  if (tls_rand_file)
    RAND_write_file(tls_rand_file);

  /* Done with the NetIO objects */
  if (tls_ctrl_netio) {
    pr_unregister_netio(PR_NETIO_STRM_CTRL);
    destroy_pool(tls_ctrl_netio->pool);
    tls_ctrl_netio = NULL;
  }

  if (tls_data_netio) {
    pr_unregister_netio(PR_NETIO_STRM_DATA);
    destroy_pool(tls_data_netio->pool);
    tls_data_netio = NULL;
  }

  if (mpid != getpid())
    tls_scrub_pkeys();

  tls_closelog();
  return;
}

/* Initialization routines
 */

static int tls_init(void) {
  int res = 0;

  /* Make sure that the OpenSSL headers used match the version of the
   * OpenSSL library used.
   */
  if (SSLeay() != OPENSSL_VERSION_NUMBER) {
    pr_log_pri(PR_LOG_ERR, MOD_TLS_VERSION
      ": compiled using OpenSSL version '%s' headers, but linked to "
      "OpenSSL version '%s' library", OPENSSL_VERSION_TEXT,
      SSLeay_version(SSLEAY_VERSION));
    tls_log("compiled using OpenSSL version '%s' headers, but linked to "
      "OpenSSL version '%s' library", OPENSSL_VERSION_TEXT,
      SSLeay_version(SSLEAY_VERSION));
    return -1;
  }

  /* Install our control channel NetIO handlers.  This is done here
   * specifically because we need to cache a pointer to the nstrm that
   * is passed to the open callback().  Ideally we'd only install our
   * custom NetIO handlers if the appropriate AUTH command was given.
   * But by then, the open() callback will have already been called, and
   * we will not have a chance to get that nstrm pointer.
   */
  tls_netio_install_ctrl();

  /* Initialize the OpenSSL context. */
  res = tls_init_ctxt();

  pr_log_debug(DEBUG2, MOD_TLS_VERSION ": using " OPENSSL_VERSION_TEXT);

  pr_event_register(&tls_module, "core.exit", tls_daemon_exit_ev, NULL);
#if defined(PR_SHARED_MODULE)
  pr_event_register(&tls_module, "core.module-unload", tls_mod_unload_ev, NULL);
#endif /* PR_SHARED_MODULE */
  pr_event_register(&tls_module, "core.postparse", tls_postparse_ev, NULL);
  pr_event_register(&tls_module, "core.restart", tls_restart_ev, NULL);

  return 0;
}

static int tls_sess_init(void) {
  int res = 0;
  unsigned char *tmp = NULL;
  unsigned long *opts = NULL;
  config_rec *c = NULL;

  /* First, check to see whether mod_tls is even enabled. */
  if ((tmp = get_param_ptr(main_server->conf, "TLSEngine",
      FALSE)) != NULL && *tmp == TRUE)
    tls_engine = TRUE;

  else {

    /* No need for this modules's control channel NetIO handlers
     * anymore.
     */
    pr_unregister_netio(PR_NETIO_STRM_CTRL);

    /* No need for all the OpenSSL stuff in this process space, either.
     */
    tls_cleanup(TLS_CLEANUP_FL_SESS_INIT);
    tls_scrub_pkeys();

    return 0;
  }

  tls_cipher_suite = get_param_ptr(main_server->conf, "TLSCipherSuite",
    FALSE);
  if (tls_cipher_suite == NULL)
    tls_cipher_suite = TLS_DEFAULT_CIPHER_SUITE;

  tls_crl_file = get_param_ptr(main_server->conf,
    "TLSCARevocationFile", FALSE);
  tls_crl_path = get_param_ptr(main_server->conf, 
    "TLSCARevocationPath", FALSE);

  tls_dhparam_file = get_param_ptr(main_server->conf,
    "TLSDHParamFile", FALSE);

  tls_dsa_cert_file = get_param_ptr(main_server->conf,
    "TLSDSACertificateFile", FALSE);
  tls_dsa_key_file = get_param_ptr(main_server->conf,
    "TLSDSACertificateKeyFile", FALSE);

  tls_rsa_cert_file = get_param_ptr(main_server->conf,
    "TLSRSACertificateFile", FALSE);
  tls_rsa_key_file = get_param_ptr(main_server->conf,
    "TLSRSACertificateKeyFile", FALSE);

  if ((opts = get_param_ptr(main_server->conf, "TLSOptions", FALSE)) != NULL)
    tls_opts = *opts;

  if ((tmp = get_param_ptr(main_server->conf, "TLSVerifyClient",
      FALSE)) != NULL && *tmp == TRUE) {
    int *depth = NULL;
    tls_flags |= TLS_SESS_VERIFY_CLIENT;

    if ((depth = get_param_ptr(main_server->conf, "TLSVerifyDepth",
        FALSE)) != NULL)
      tls_verify_depth = *depth;
  }

  c = find_config(main_server->conf, CONF_PARAM, "TLSRequired", FALSE);
  if (c) {
    tls_required_on_ctrl = *((unsigned char *) c->argv[0]);
    tls_required_on_data = *((unsigned char *) c->argv[1]);
    tls_required_on_auth = *((unsigned char *) c->argv[2]);
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "TLSTimeoutHandshake",
      FALSE)))
    tls_handshake_timeout = *((unsigned int *) c->argv[0]);

  /* Open the TLSLog, if configured */
  res = tls_openlog();
  if (res < 0) {
    if (res == -1)
      pr_log_pri(PR_LOG_NOTICE, MOD_TLS_VERSION
        ": notice: unable to open TLSLog: %s", strerror(errno));

    else if (res == PR_LOG_WRITABLE_DIR)
      pr_log_pri(PR_LOG_NOTICE, MOD_TLS_VERSION
        ": notice: unable to open TLSLog: parent directory is world writable");

    else if (res == PR_LOG_SYMLINK)
      pr_log_pri(PR_LOG_NOTICE, MOD_TLS_VERSION
        ": notice: unable to open TLSLog: cannot log to a symbolic link");
  }

  /* If UseReverseDNS is set to off, disable TLS_OPT_VERIFY_CERT_FQDN. */
  if ((tls_opts & TLS_OPT_VERIFY_CERT_FQDN) && !ServerUseReverseDNS) {
    tls_opts &= ~TLS_OPT_VERIFY_CERT_FQDN;
    tls_log("%s", "reverse DNS off, disabling TLSOption dNSNameRequired");
  }

  /* Install our data channel NetIO handlers. */
  tls_netio_install_data();

  pr_event_register(&tls_module, "core.exit", tls_sess_exit_ev, NULL);

  /* Check to see if a passphrase has been entered for this server. */
  tls_pkey = tls_lookup_pkey();
  if (tls_pkey != NULL) {
    SSL_CTX_set_default_passwd_cb(ssl_ctx, tls_pkey_cb);
    SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx, (void *) tls_pkey);
  }
  
  /* NOTE: fail session init if TLS server init fails (e.g. res < 0)? */
  /* Initialize the OpenSSL context for this server's configuration. */
  res = tls_init_server();

  /* Add the additional features implemented by this module into the
   * list, to be displayed in response to a FEAT command.
   */
  pr_feat_add("AUTH TLS");
  pr_feat_add("PBSZ");
  pr_feat_add("PROT");

  return 0;
}

/* Module API tables
 */

static conftable tls_conftab[] = {
  { "TLSCACertificateFile",	set_tlscacertfile,	NULL },
  { "TLSCACertificatePath",	set_tlscacertpath,	NULL },
  { "TLSCARevocationFile",      set_tlscacrlfile,       NULL }, 
  { "TLSCARevocationPath",      set_tlscacrlpath,       NULL }, 
  { "TLSCertificateChainFile",	set_tlscertchain,	NULL },
  { "TLSCipherSuite",		set_tlsciphersuite,	NULL },
  { "TLSCryptoDevice",		set_tlscryptodevice,	NULL },
  { "TLSDHParamFile",		set_tlsdhparamfile,	NULL },
  { "TLSDSACertificateFile",	set_tlsdsacertfile,	NULL },
  { "TLSDSACertificateKeyFile",	set_tlsdsakeyfile,	NULL },
  { "TLSEngine",		set_tlsengine,		NULL },
  { "TLSLog",			set_tlslog,		NULL },
  { "TLSOptions",		set_tlsoptions,		NULL },
  { "TLSPassPhraseProvider",	set_tlspassphraseprovider, NULL },
  { "TLSProtocol",		set_tlsprotocol,	NULL },
  { "TLSRandomSeed",		set_tlsrandseed,	NULL },
  { "TLSRenegotiate",		set_tlsrenegotiate,	NULL },
  { "TLSRequired",		set_tlsrequired,	NULL },
  { "TLSRSACertificateFile",	set_tlsrsacertfile,	NULL },
  { "TLSRSACertificateKeyFile",	set_tlsrsakeyfile,	NULL },
  { "TLSTimeoutHandshake",	set_tlstimeouthandshake,NULL },
  { "TLSVerifyClient",		set_tlsverifyclient,	NULL },
  { "TLSVerifyDepth",		set_tlsverifydepth,	NULL },
  { NULL , NULL, NULL}
};

static cmdtable tls_cmdtab[] = {
  { PRE_CMD,	C_ANY,	G_NONE,	tls_any,	FALSE,	FALSE },
  { CMD,	C_AUTH,	G_NONE,	tls_auth,	FALSE,	FALSE,	CL_SEC },
  { CMD,	C_CCC,	G_NONE,	tls_ccc,	FALSE,	FALSE,	CL_SEC },
  { CMD,	C_PBSZ,	G_NONE,	tls_pbsz,	FALSE,	FALSE,	CL_SEC },
  { CMD,	C_PROT,	G_NONE,	tls_prot,	FALSE,	FALSE,	CL_SEC },
  { POST_CMD,	C_PASS,	G_NONE,	tls_post_pass,	FALSE,	FALSE,	CL_SEC },
  { 0,	NULL }
};

static authtable tls_authtab[] = {
  { 0, "auth",			tls_authenticate	},
  { 0, "check",			tls_auth_check		},
  { 0, "requires_pass",		tls_authenticate	},
  { 0, NULL }
};

module tls_module = {

  /* Always NULL */
    NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "tls",

  /* Module configuration handler table */
  tls_conftab,

  /* Module command handler table */
  tls_cmdtab,

  /* Module authentication handler table */
  tls_authtab,

  /* Module initialization */
  tls_init,

  /* Session initialization */
  tls_sess_init,

  /* Module version */
  MOD_TLS_VERSION
};

